#define FUSE_USE_VERSION 26

#include "tagfs_common.h"
#include "tagfs_db.h"
#include "tagfs_debug.h"

#include <errno.h>
#include <fuse.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

static int tagfs_getattr(const char *path, struct stat *buf)
{
	int retstat = 0;

	printf("%s is a... ", path);

	if (valid_path_to_file(path)) {
		printf("File!\n");
		retstat = stat(get_file_location(path), buf);
	}
	else if(valid_path_to_tag(path)) {
		printf("Directory!\n");
		buf->st_mode = S_IFDIR | 0755;
		buf->st_nlink = 2;
	}
	else {
		printf("No idea!\n");
		return -ENOENT;
	}

	return retstat;

//	int res = 0;
//
//	memset(stbuf, 0, sizeof(struct stat));
//	if (valid_path_to_tag(path)) {
//		stbuf->st_mode = S_IFDIR | 0755;
//		stbuf->st_nlink = 2;
//	}
//	else {
//		stbuf->st_mode = S_IFREG | 0444;
//		stbuf->st_nlink = 1;
//	}
//
//	return res;
}

static int tagfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi)
{
//	printf("readdir: %s\n", path);
	char **file_array = NULL;
	char **tag_array = NULL;
	int i = 0;
	int num_files = db_files_from_restricted_query(path, &file_array);
	int num_tags = db_tags_from_query(path, &tag_array);

	filler(buf, ".", NULL, 0);
	filler(buf, "..", NULL, 0);

	for(i = 0; i < num_files; i++) {
//		DEBUG(DL_NORMAL, "Adding file: %s", file_array[i]);
		filler(buf, file_array[i], NULL, 0);
	}

	for(i = 0; i < num_tags; i++) {
		filler(buf, tag_array[i], NULL, 0);
	}

	free_char_ptr_array(&file_array, num_files);
	free_char_ptr_array(&tag_array, num_tags);

	return 0;
}

int tagfs_unlink(const char *path) {
	printf("%s\n", path);
	db_delete_file(path);

	return 0;
}

int tagfs_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
	int retstat = 0;

	int fd = open(get_file_location(path), O_RDONLY);
	retstat = pread(fd, buf, size, offset);

	return retstat;
}

//int tagfs_opendir(const char *path, struct fuse_file_info *fi) {
//	return 0;
//}

static struct fuse_operations tagfs_oper = {
	.getattr = tagfs_getattr,
	.readdir = tagfs_readdir,
	.unlink = tagfs_unlink,
	.read = tagfs_read,
};

int main(int argc, char *argv[])
{
	debug_init();

//	db_delete_file("/zouch");
//	return 0;
//	printf("%s\n", valid_path_to_tag("/") == true ? "true" : "false");
//	return 0;
	return fuse_main(argc, argv, &tagfs_oper, NULL);
}
