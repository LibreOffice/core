/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include "system.h"

#ifdef NO_PTHREAD_RTL

static pthread_mutex_t getrtl_mutex = PTHREAD_MUTEX_INITIALIZER;

/* struct passwd differs on some platforms */
#if defined NETBSD && (__NetBSD_Version__ < 299001000)
struct passwd *getpwnam_r(const char* name, struct passwd* s, char* buffer, int size )
{
      struct passwd* res;

      pthread_mutex_lock(&getrtl_mutex);

      if ( (res = getpwnam(name)) )
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

        return(res);
}

int getpwuid_r(uid_t uid, struct passwd *pwd, char *buffer,
           size_t buflen, struct passwd **result)
{
  struct passwd* res;
  int retval = 0;

  pthread_mutex_lock(&getrtl_mutex);

  if ( (res = getpwuid(uid)) )
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
      retval = 0 ;

    }
    else
        retval =  ENOMEM;
  }
  else
      retval = errno ;

  pthread_mutex_unlock(&getrtl_mutex);

  return retval;
}

struct tm *localtime_r(const time_t *timep, struct tm *buffer)
{
    struct tm* res;

      pthread_mutex_lock(&getrtl_mutex);

    if ( (res = localtime(timep)))
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

    if ( (res = gmtime(timep)) )
    {
        memcpy(buffer, res, sizeof(struct tm));
        res = buffer;
    }

    pthread_mutex_unlock(&getrtl_mutex);

    return res;
}
#endif  /* defined NETBSD */

#if !defined(FREEBSD) || (__FreeBSD_version < 601103)

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

      if ( (res = gethostbyname(name)) )
      {
        int nname, naliases, naddr_list, naliasesdata, n;
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

            parray = (char**)buffer;
            result->h_aliases = parray;
            data = buffer + (naliases+1)*sizeof(char*);
            for ( p = res->h_aliases; *p != NULL; p++) {
                n = strlen(*p)+1;
                *parray++ = data;
                memcpy(data, *p, n);
                data += n;
            }
            *parray = NULL;
            buffer = data;
            parray = (char**)buffer;
            result->h_addr_list = parray;
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
#endif /* !defined(FREEBSD) || (__FreeBSD_version < 601103) */

#if defined(MACOSX)
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
    (void)pshared;
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
