#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dlfcn.h>

#ifdef _cplusplus
extern "C" {
#endif

int   chown  (const char *path, uid_t owner, gid_t group) {return 0;}
int   lchown (const char *path, uid_t owner, gid_t group) {return 0;}
int   fchown (int fildes, uid_t owner, gid_t group)       {return 0;}

uid_t getuid  (void) {return 0;}
#ifdef __notdef__
uid_t geteuid (void) {return 0;}
gid_t getgid  (void) {return 0;}
gid_t getegid (void) {return 0;}
#endif

int   setuid  (uid_t p)  {return 0;}
int   setgid  (gid_t p)  {return 0;}

int fstat(int fildes, struct stat *buf)
{
    int ret = 0;
    static int (*p_fstat) (int fildes, struct stat *buf) = NULL;
    if (p_fstat == NULL)
        p_fstat = (int (*)(int fildes, struct stat *buf))
            dlsym (RTLD_NEXT, "fstat");
    ret = (*p_fstat)(fildes, buf);
    if (buf != NULL)
    {
        buf->st_uid = 0; /* root */
        buf->st_gid = 2; /* bin */
    }

    return ret;
}

#ifdef _cplusplus
}
#endif

