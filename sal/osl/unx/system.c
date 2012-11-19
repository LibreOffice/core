/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "system.h"

#ifdef NO_PTHREAD_RTL

/* struct passwd differs on some platforms */

#if defined(MACOSX) || defined(IOS) || defined(OPENBSD) || defined(NETBSD)

//No mutex needed on Mac OS X, gethostbyname is thread safe

#if defined(MACOSX)

#define RTL_MUTEX_LOCK
#define RTL_MUTEX_UNLOCK

#else //defined(MACOSX)

static pthread_mutex_t getrtl_mutex = PTHREAD_MUTEX_INITIALIZER;

#define RTL_MUTEX_LOCK pthread_mutex_lock(&getrtl_mutex);
#define RTL_MUTEX_UNLOCK pthread_mutex_unlock(&getrtl_mutex);

#endif //defined(MACOSX)

extern int h_errno;

struct hostent *gethostbyname_r(const char *name, struct hostent *result,
                                char *buffer, size_t buflen, int *h_errnop)
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

      RTL_MUTEX_LOCK

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

    RTL_MUTEX_UNLOCK

    return res;
}
#endif // OSX || IOS || OPENBSD || NETBSD

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

#if defined(LINUX) && defined (__GLIBC__) && __GLIBC__ == 2 && __GLIBC_MINOR__ < 4
/* The linux kernel 2.4 getpid implemention always return the pid of the
   thread subprocess and not of the main process, the NPTL implementation
   with a Linux kernel 2.6 kernel return the pid. So when possibly
   there is the wrong implementation of getpid, we save the pid at startup.
   FIXME: when our Linux base-line is above:
        + Linux kernel version 2.6 or higher; -> clone() for NTPL
        + glibc2 version 2.4 or higher; -> No longer LinuxThreads, only NPTL
   Then we get a working getpid() and can remove this hack.
   FIXME: getppid is also wrong in this situation
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
#endif /*  defined LINUX */

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
