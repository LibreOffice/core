/*
    getcwd -- get pathname of current working directory

    public-domain implementation

    last edit:  03-Nov-1990 Gwyn@BRL.MIL

    complies with the following standards:
        IEEE Std 1003.1-1988
        SVID Issue 3
        X/Open Portability Guide Issue 2 (when "XPG2" is defined)
        X/Open Portability Guide Issue 3

    This implementation of getcwd() can be used to replace the UNIX
    System V library routine (which uses popen() to capture the output of
    the "pwd" command).  Once that is done, "pwd" can be reimplemented as
    just puts(getcwd((char*)0,0)), assuming "XPG2" is defined below.

    This implementation depends on every directory having entries for
    "." and "..".  It also depends on the internals of the <dirent.h>
    data structures to some degree.

    I considered using chdir() to ascend the hierarchy, followed by a
    final chdir() to the path being returned by getcwd() to restore the
    location, but decided that error recovery was too difficult that way.
    The algorithm I settled on was inspired by my rewrite of the "pwd"
    utility, combined with the dotdots[] array trick from the SVR2 shell.
*/
#define XPG2    /* define to support obsolete XPG2-mandated feature */


#include    <sys/types.h>
#include    <sys/stat.h>

#ifdef M_XENIX
# include        <sys/ndir.h>
# define dirent direct
#else
# include        <dirent.h>
#endif

#include    <errno.h>
#include    <string.h>

typedef char    *pointer;       /* (void *) if you have it */

extern void free();
extern pointer  malloc();
extern int  fstat(), stat();

extern int  errno;          /* normally done by <errno.h> */

#ifndef NULL
#define NULL    0           /* amorphous null pointer constant */
#endif

#ifndef NAME_MAX
#define NAME_MAX    255     /* maximum directory entry size */
#endif


char    *
getcwd( buf, size )         /* returns pointer to CWD pathname */
    char        *buf;       /* where to put name (NULL to malloc) */
    int     size;       /* size of buf[] or malloc()ed memory */
    {
    static char dotdots[] =
"../../../../../../../../../../../../../../../../../../../../../../../../../..";
    char        *dotdot;    /* -> dotdots[.], right to left */
    DIR     *dirp;      /* -> parent directory stream */
    struct dirent   *dir;       /* -> directory entry */
    struct stat stat1,
            stat2;      /* info from stat() */
    struct stat *d = &stat1;    /* -> info about "." */
    struct stat *dd = &stat2;   /* -> info about ".." */
    register char   *buffer;    /* local copy of buf, or malloc()ed */
    char        *bufend;    /* -> buffer[size] */
    register char   *endp;      /* -> end of reversed string */
    register char   *dname;     /* entry name ("" for root) */
    int     serrno = errno; /* save entry errno */

    if ( buf != NULL && size <= 0
#ifndef XPG2
      || buf == NULL
#endif
       )    {
        errno = EINVAL;     /* invalid argument */
        return NULL;
        }

    buffer = buf;
#ifdef XPG2
    if ( buf == NULL        /* wants us to malloc() the string */
      && (buffer = (char *) malloc( (unsigned) size )) == NULL
    /* XXX -- actually should probably not pay attention to "size" arg */
       )    {
        errno = ENOMEM;     /* cannot malloc() specified size */
        return NULL;
        }
#endif

    if ( stat( ".", dd ) != 0 ) /* prime the pump */
        goto error;     /* errno already set */

    endp = buffer;          /* initially, empty string */
    bufend = &buffer[size];

    for ( dotdot = &dotdots[sizeof dotdots]; dotdot != dotdots; )
        {
        dotdot -= 3;        /* include one more "/.." section */
                    /* (first time is actually "..") */

        /* swap stat() info buffers */
        {
        register struct stat    *temp = d;

        d = dd;         /* new current dir is old parent dir */
        dd = temp;
        }

        if ( (dirp = opendir( dotdot )) == NULL )   /* new parent */
            goto error; /* errno already set */

        if ( fstat( dirp->dd_fd, dd ) != 0 )
            {
            serrno = errno; /* set by fstat() */
            (void)closedir( dirp );
            errno = serrno; /* in case closedir() clobbered it */
            goto error;
            }

        if ( d->st_dev == dd->st_dev )
            {       /* not crossing a mount point */
            if ( d->st_ino == dd->st_ino )
                {   /* root directory */
                dname = "";
                goto append;
                }

            do
                if ( (dir = readdir( dirp )) == NULL )
                    {
                    (void)closedir( dirp );
                    errno = ENOENT; /* missing entry */
                    goto error;
                    }
            while ( dir->d_ino != d->st_ino );
            }
        else    {       /* crossing a mount point */
            struct stat t;  /* info re. test entry */
            char        name[sizeof dotdots + 1 + NAME_MAX];

            (void)strcpy( name, dotdot );
            dname = &name[strlen( name )];
            *dname++ = '/';

            do  {
                if ( (dir = readdir( dirp )) == NULL )
                    {
                    (void)closedir( dirp );
                    errno = ENOENT; /* missing entry */
                    goto error;
                    }

                (void)strcpy( dname, dir->d_name );
                /* must fit if NAME_MAX is not a lie */
                }
            while ( stat( name, &t ) != 0
                 || t.st_ino != d->st_ino
                 || t.st_dev != d->st_dev
                  );
            }

        dname = dir->d_name;

        /* append "/" and reversed dname string onto buffer */
    append:
        if ( endp != buffer /* avoid trailing / in final name */
          || dname[0] == '\0'   /* but allow "/" when CWD is root */
           )
            *endp++ = '/';

        {
        register char   *app;   /* traverses dname string */

        for ( app = dname; *app != '\0'; ++app )
            ;

        if ( app - dname >= bufend - endp )
            {
            (void)closedir( dirp );
            errno = ERANGE; /* won't fit allotted space */
            goto error;
            }

        while ( app != dname )
            *endp++ = *--app;
        }

        (void)closedir( dirp );

        if ( dname[0] == '\0' ) /* reached root; wrap it up */
            {
            register char   *startp;    /* -> buffer[.] */

            *endp = '\0';   /* plant null terminator */

            /* straighten out reversed pathname string */
            for ( startp = buffer; --endp > startp; ++startp )
                {
                char    temp = *endp;

                *endp = *startp;
                *startp = temp;
                }

            errno = serrno; /* restore entry errno */
            /* XXX -- if buf==NULL, realloc here? */
            return buffer;
            }
        }

    errno = ENOMEM;         /* actually, algorithm failure */

    error:
    if ( buf == NULL )
        free( (pointer)buffer );

    return NULL;
    }

