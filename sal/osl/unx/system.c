/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: system.c,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: kz $ $Date: 2006-07-19 09:39:37 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#include "system.h"

#ifdef MACOSX
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/errno.h>
#include <string.h>
#endif

#ifdef NO_PTHREAD_RTL

static pthread_mutex_t getrtl_mutex = PTHREAD_MUTEX_INITIALIZER;

/* struct passwd differs on some platforms */
#if defined NETBSD || defined MACOSX
#include <pwd.h>
#include <sys/types.h>

/* [ed] 9/1/02 On OS 10.2 and higher, the OS headers define this function slightly differently.
   A fourth argument is expected, a struct passwd **.  We still want to make our own definition,
   however, so when we run on 10.1 we can still locate the symbol in our own libraries.  So
   if we're building on 10.2, simply change the prototype to match the expected system
   prototype and provide the duplicate symbol.
   */
#ifdef MACOSX
#ifdef BUILD_OS_APPLEOSX
#if (BUILD_OS_MAJOR >= 10) && (BUILD_OS_MINOR >= 2)
int getpwnam_r(const char* name, struct passwd* s, char* buffer, size_t size, struct passwd **ignore )
#else /* BUILD_OS_MAJOR && BUILD_OS_MINOR */
/* previous versions of MacOS X...10.0/1.  Use old prototype */
struct passwd *getpwnam_r(const char* name, struct passwd* s, char* buffer, int size )
#endif /* BUILD_OS_MAJOR && BUILD_OS_MINOR */
#else /* BUILD_OS_APPLE_OSX */
/* configure didn't take, or we're building on darwin.  Fallback on old prototype */
struct passwd *getpwnam_r(const char* name, struct passwd* s, char* buffer, int size )
#endif /* BUILD_OS_APPLEOSX */
#else /* MACOSX */
struct passwd *getpwnam_r(const char* name, struct passwd* s, char* buffer, int size )
#endif /* MACOSX */
{
      struct passwd* res;

      pthread_mutex_lock(&getrtl_mutex);

      if ( res = getpwnam(name) )
      {
        int nname, npasswd, nclass, ngecos, ndir;

        nname= strlen(res->pw_name)+1;
        npasswd= strlen(res->pw_passwd)+1;
        nclass= strlen(res->pw_class)+1;
        ngecos= strlen(res->pw_gecos)+1;
        ndir= strlen(res->pw_dir)+1;

        if (nname+npasswd+nclass+ngecos
                +ndir+strlen(res->pw_shell) < size)
        {
            memcpy(s, res, sizeof(struct passwd));

            strcpy(buffer, res->pw_name);
              s->pw_name = buffer;
            buffer += nname;

            strcpy(buffer, res->pw_passwd);
              s->pw_passwd = buffer;
            buffer += npasswd;

            strcpy(buffer, res->pw_class);
              s->pw_class = buffer;
            buffer += nclass;

            strcpy(buffer, res->pw_gecos);
              s->pw_gecos = buffer;
            buffer += ngecos;

            strcpy(buffer, res->pw_dir);
              s->pw_dir = buffer;
            buffer += ndir;

            strcpy(buffer, res->pw_shell);
              s->pw_shell = buffer;

               res = s;
        }
        else
            res = 0;
    }

    pthread_mutex_unlock(&getrtl_mutex);

#ifdef MACOSX
#ifdef BUILD_OS_APPLEOSX
#if (BUILD_OS_MAJOR >= 10) && (BUILD_OS_MINOR >= 2)
    return((int)res);
#else
    return(res);
#endif /* BUILD_OS_MAJOR && BUILD_OS_MINOR */
#else /* BUILD_OS_APPLEOSX */
        return(res);
#endif /* BUILD_OS_APPLEOSX */
#else /* MACOSX */
        return(res);
#endif /* MACOSX */
}

#if defined(NETBSD) || defined(MACOSX)
int getpwuid_r(uid_t uid, struct passwd *pwd, char *buffer,
           size_t buflen, struct passwd **result)
{
  struct passwd* res;

  pthread_mutex_lock(&getrtl_mutex);

  if ( res = getpwuid(uid) )
  {
    size_t pw_name, pw_passwd, pw_class, pw_gecos, pw_dir, pw_shell;

    pw_name = strlen(res->pw_name)+1;
    pw_passwd = strlen(res->pw_passwd)+1;
    pw_class = strlen(res->pw_class)+1;
    pw_gecos = strlen(res->pw_gecos)+1;
    pw_dir = strlen(res->pw_dir)+1;
    pw_shell = strlen(res->pw_shell)+1;

    if (pw_name+pw_passwd+pw_class+pw_gecos
                                 +pw_dir+pw_shell < buflen)
    {
      memcpy(pwd, res, sizeof(struct passwd));

      strncpy(buffer, res->pw_name, pw_name);
      pwd->pw_name = buffer;
      buffer += pw_name;

      strncpy(buffer, res->pw_passwd, pw_passwd);
      pwd->pw_passwd = buffer;
      buffer += pw_passwd;

      strncpy(buffer, res->pw_class, pw_class);
      pwd->pw_class = buffer;
      buffer += pw_class;

      strncpy(buffer, res->pw_gecos, pw_gecos);
      pwd->pw_gecos = buffer;
      buffer += pw_gecos;

      strncpy(buffer, res->pw_dir, pw_dir);
      pwd->pw_dir = buffer;
      buffer += pw_dir;

      strncpy(buffer, res->pw_shell, pw_shell);
      pwd->pw_shell = buffer;
      buffer += pw_shell;

      *result = pwd ;
      res = 0 ;

    } else {

      res = ENOMEM ;

    }

  } else {

    res = errno ;

  }

  pthread_mutex_unlock(&getrtl_mutex);

  return res;
}
#endif

struct tm *localtime_r(const time_t *timep, struct tm *buffer)
{
    struct tm* res;

      pthread_mutex_lock(&getrtl_mutex);

    if (res = localtime(timep))
    {
        memcpy(buffer, res, sizeof(struct tm));
        res = buffer;
    }

    pthread_mutex_unlock(&getrtl_mutex);

    return res;
}

struct tm *gmtime_r(const time_t *timep, struct tm *buffer)
{
    struct tm* res;

      pthread_mutex_lock(&getrtl_mutex);

    if (res = gmtime(timep))
    {
        memcpy(buffer, res, sizeof(struct tm));
        res = buffer;
    }

    pthread_mutex_unlock(&getrtl_mutex);

    return res;
}
#endif  /* defined NETBSD || defined MACOSX */

#ifdef SCO
#include <pwd.h>
#include <shadow.h>
#include <sys/types.h>

struct spwd *getspnam_r(const char *name, struct spwd* s, char* buffer, int size )
{
      struct spwd* res;

      pthread_mutex_lock(&getrtl_mutex);

      if ( res = getspnam(name) )
      {
        int nnamp;

        nnamp = strlen(res->sp_namp)+1;

        if (nnamp+strlen(res->sp_pwdp) < size) {
            memcpy(s, res, sizeof(struct spwd));

            strcpy(buffer, res->sp_namp);
              s->sp_namp = buffer;
            buffer += nnamp;

            strcpy(buffer, res->sp_pwdp);
              s->sp_pwdp = buffer;

               res = s;
        }
        else
            res = 0;
    }

      pthread_mutex_unlock(&getrtl_mutex);

      return res;
}

struct passwd *getpwnam_r(const char* name, struct passwd* s, char* buffer, int size )
{
      struct passwd* res;

      pthread_mutex_lock(&getrtl_mutex);

      if ( res = getpwnam(name) )
      {
        int nname, npasswd, nage;
        int ncomment, ngecos, ndir;

        nname= strlen(res->pw_name)+1;
        npasswd= strlen(res->pw_passwd)+1;
        nage= strlen(res->pw_age)+1;
        ncomment= strlen(res->pw_comment)+1;
        ngecos= strlen(res->pw_gecos)+1;
        ndir= strlen(res->pw_dir)+1;

        if (nname+npasswd+nage+ncomment+ngecos+ndir
                +strlen(res->pw_shell) < size)
        {
            memcpy(s, res, sizeof(struct passwd));

            strcpy(buffer, res->pw_name);
              s->pw_name = buffer;
            buffer += nname;

            strcpy(buffer, res->pw_passwd);
              s->pw_passwd = buffer;
            buffer += npasswd;

            strcpy(buffer, res->pw_age);
              s->pw_age = buffer;
            buffer += nage;

            strcpy(buffer, res->pw_comment);
              s->pw_comment = buffer;
            buffer += ncomment;

            strcpy(buffer, res->pw_gecos);
              s->pw_gecos = buffer;
            buffer += ngecos;

            strcpy(buffer, res->pw_dir);
              s->pw_dir = buffer;
            buffer += ndir;

            strcpy(buffer, res->pw_shell);
              s->pw_shell = buffer;

               res = s;
        }
        else
            res = 0;
    }

    pthread_mutex_unlock(&getrtl_mutex);

      return res;
}
#endif /* defined SCO */

#if !defined(FREEBSD) || (__FreeBSD_version < 700015)

extern int h_errno;

struct hostent *gethostbyname_r(const char *name, struct hostent *result,
                                char *buffer, int buflen, int *h_errnop)
{
    /* buffer layout:   name\0
     *                  array_of_pointer_to_aliases
     *                  NULL
     *                  alias1\0...aliasn\0
     *                  array_of_pointer_to_addresses
     *                  NULL
     *                  addr1addr2addr3...addrn
     */
      struct hostent* res;

      pthread_mutex_lock(&getrtl_mutex);

      if ( res = gethostbyname(name) )
      {
        int nname, naliases, naddr_list, naliasesdata, ncntaddr_list, n;
        char **p, **parray, *data;

        /* Check buffer size before copying, we want to leave the
         * buffers unmodified in case something goes wrong.
         *
         * Is this required?
         */

        nname= strlen(res->h_name)+1;

        naliases = naddr_list = naliasesdata = 0;

        for ( p = res->h_aliases; *p != NULL; p++) {
            naliases++;
            naliasesdata += strlen(*p)+1;
        }

        for ( p = res->h_addr_list; *p != NULL; p++)
            naddr_list++;

        if ( nname
             + (naliases+1)*sizeof(char*) + naliasesdata
             + (naddr_list+1)*sizeof(char*) + naddr_list*res->h_length
             <= buflen )
        {
            memcpy(result, res, sizeof(struct hostent));

            strcpy(buffer, res->h_name);
              result->h_name = buffer;
            buffer += nname;

            result->h_aliases = buffer;
            parray = (char**)buffer;
            data = buffer + (naliases+1)*sizeof(char*);
            for ( p = res->h_aliases; *p != NULL; p++) {
                n = strlen(*p)+1;
                *parray++ = data;
                memcpy(data, *p, n);
                data += n;
            }
            *parray = NULL;
            buffer = data;

            result->h_addr_list = buffer;
            parray = (char**)buffer;
            data = buffer + (naddr_list+1)*sizeof(char*);
            for ( p = res->h_addr_list; *p != NULL; p++) {
                *parray++ = data;
                memcpy(data, *p, res->h_length);
                data += res->h_length;
            }
            *parray = NULL;

               res = result;
        }
        else
        {
            errno = ERANGE;
            res = NULL;
        }
    }
    else
    {
        *h_errnop = h_errno;
    }

    pthread_mutex_unlock(&getrtl_mutex);

      return res;
}
#endif /* !defined(FREEBSD) || (__FreeBSD_version < 700015) */

#if defined(MACOSX)
/*
 * This section works around calls that are missing or broken
 * in MacOS X 10.1.x and earlier.
 */

/* MacOS X doesn't have readdir_r() standard, plus readdir() isn't threadsafe. */

/*******************************************************************************/
int readdir_r( DIR *dirp, struct dirent *entry, struct dirent **result )
{
    struct dirent* pDirEntry;
    int nRet;
    int nSavedErrno;

    pthread_mutex_lock(&getrtl_mutex);

    nSavedErrno = errno;
    errno = 0;
    pDirEntry = readdir(dirp);

    if ( pDirEntry ) {
        memcpy(entry, pDirEntry, sizeof(struct dirent));
        *result = entry;
        errno = nSavedErrno;
        nRet = 0;
    }
    else {
        if ( errno ) {
            nRet = errno; /* can be EBADF */
        }
        else {
            *result = NULL;
            nRet = 0;
            /* errno must not be changed if reaching end of dir */
            errno = nSavedErrno;
        }
    }

    pthread_mutex_unlock(&getrtl_mutex);

    return nRet;
}

/* No reentrant asctime() either... */

/*******************************************************************************/
char *asctime_r( const struct tm *tm, char *buffer )
{
    char        *asctimeBuffer;

    pthread_mutex_lock(&getrtl_mutex);

    asctimeBuffer = asctime( tm );
    /* Simply hope we don't have a buffer overflow... */
    if ( asctimeBuffer )
        strcpy( buffer, asctimeBuffer );
    else
        buffer = NULL;

    pthread_mutex_unlock(&getrtl_mutex);
    return( buffer );
}

/*
 * Return the system version.  Currently, we only differentiate between
 * Darwin and OS X.
 */
void macxp_getSystemVersion( unsigned int *isDarwin, unsigned int *majorVersion, unsigned int *minorVersion, unsigned int *minorMinorVersion )
{
    int         err;
    struct stat status;

    /* Check for the presence of /usr/bin/osascript, which is
     * believed to be OS X only.
     */
     err = stat( "/usr/bin/osascript", &status );
     if ( err == 0 )
        *isDarwin = 0;      /* OS X system detected */
    else
        *isDarwin = 1;      /* Darwin system detected */

    *majorVersion = 0;
    *minorVersion = 0;
    *minorMinorVersion = 0;
}

/*
 * Add support for resolving Mac native alias files (not the same as unix alias files)
 * returns 0 on success.
 */
int macxp_resolveAlias(char *path, int buflen)
{
  FSRef aFSRef;
  OSStatus nErr;
  Boolean bFolder;
  Boolean bAliased;
  char *unprocessedPath = path;

  if ( *unprocessedPath == '/' )
    unprocessedPath++;

  int nRet = 0;
  while ( !nRet && unprocessedPath && *unprocessedPath )
    {
      unprocessedPath = strchr( unprocessedPath, '/' );
      if ( unprocessedPath )
    *unprocessedPath = '\0';

      nErr = noErr;
      bFolder = FALSE;
      bAliased = FALSE;
      if ( FSPathMakeRef( (const UInt8 *)path, &aFSRef, 0 ) == noErr )
    {
      nErr = FSResolveAliasFileWithMountFlags( &aFSRef, TRUE, &bFolder, &bAliased, kResolveAliasFileNoUI );
      if ( nErr == nsvErr )
        {
          errno = ENOENT;
          nRet = -1;
        }
      else if ( nErr == noErr && bAliased )
        {
          char tmpPath[ PATH_MAX ];
          if ( FSRefMakePath( &aFSRef, (UInt8 *)tmpPath, PATH_MAX ) == noErr )
        {
          int nLen = strlen( tmpPath ) + ( unprocessedPath ? strlen( unprocessedPath + 1 ) + 1 : 0 );
          if ( nLen < buflen && nLen < PATH_MAX )
            {
              if ( unprocessedPath )
            {
              int nTmpPathLen = strlen( tmpPath );
              strcat( tmpPath, "/" );
              strcat( tmpPath, unprocessedPath + 1 );
              strcpy( path, tmpPath);
              unprocessedPath = path + nTmpPathLen;
            }
              else if ( !unprocessedPath )
            {
              strcpy( path, tmpPath);
            }
            }
          else
            {
              errno = ENAMETOOLONG;
              nRet = -1;
            }
        }
        }
    }

      if ( unprocessedPath )
    *unprocessedPath++ = '/';
    }

  return nRet;
}

#endif  /* defined MACOSX */

#endif /* NO_PTHREAD_RTL */

#if (defined (LINUX) && (GLIBC >= 2))
/* The linux kernel thread implemention, always return the pid of the
   thread subprocess and not of the main process. So we save the main
   pid at startup
*/

// Directly from libc.so.6, obviously missing from some unistd.h:
extern __pid_t __getpid(void);

static pid_t pid = -1;

static void savePid(void) __attribute__((constructor));

static void savePid(void)
{
    if (pid == -1)
        pid = __getpid();
}

pid_t getpid(void)
{
    if (pid == -1)
        savePid();

    return (pid);
}
#endif /*  (defined (LINUX) && (GLIBC >= 2)) */

#ifdef NO_PTHREAD_SEMAPHORES
int sem_init(sem_t* sem, int pshared, unsigned int value)
{
    pthread_mutex_init(&sem->mutex, PTHREAD_MUTEXATTR_DEFAULT);
    pthread_cond_init(&sem->increased, PTHREAD_CONDATTR_DEFAULT);

    sem->value = (int)value;
    return 0;
}

int sem_destroy(sem_t* sem)
{
    pthread_mutex_destroy(&sem->mutex);
    pthread_cond_destroy(&sem->increased);
    sem->value = 0;
    return 0;
}

int sem_wait(sem_t* sem)
{
    pthread_mutex_lock(&sem->mutex);

    while (sem->value <= 0)
    {
        pthread_cond_wait(&sem->increased, &sem->mutex);
    }

    sem->value--;
    pthread_mutex_unlock(&sem->mutex);

    return 0;
}

int sem_trywait(sem_t* sem)
{
    int result = 0;

    pthread_mutex_lock(&sem->mutex);

    if (sem->value > 0)
    {
        sem->value--;
    }
    else
    {
        errno = EAGAIN;
        result = -1;
    }

    pthread_mutex_unlock(&sem->mutex);

    return result;
}

int sem_post(sem_t* sem)
{
    pthread_mutex_lock(&sem->mutex);

    sem->value++;

    pthread_mutex_unlock(&sem->mutex);

    pthread_cond_signal(&sem->increased);

    return 0;
}
#endif

#if defined(FREEBSD)
char *fcvt(double value, int ndigit, int *decpt, int *sign)
{
  static char ret[256];
  char buf[256],zahl[256],format[256]="%";
  char *v1,*v2;

  if (value==0.0) value=1e-30;

  if (value<0.0) *sign=1; else *sign=0;

  if (value<1.0)
  {
    *decpt=(int)log10(value);
    value*=pow(10.0,1-*decpt);
    ndigit+=*decpt-1;
    if (ndigit<0) ndigit=0;
  }
  else
  {
    *decpt=(int)log10(value)+1;
  }

  sprintf(zahl,"%d",ndigit);
  strcat(format,zahl);
  strcat(format,".");
  strcat(format,zahl);
  strcat(format,"f");

  sprintf(buf,format,value);

  if (ndigit!=0)
  {
    v1=strtok(buf,".");
    v2=strtok(NULL,".");
    strcpy(ret,v1);
    strcat(ret,v2);
  }
  else
  {
    strcpy(ret,buf);
  }

  return(ret);
}

#endif
