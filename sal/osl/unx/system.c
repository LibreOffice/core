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

#include <unistd.h>
#include <string.h>

#include <config_features.h>

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
 * (what are "unix alias files"?)
 * returns 0 on success.
 */
int macxp_resolveAlias(char *path, int buflen)
{
#if HAVE_FEATURE_MACOSX_SANDBOX
  /* Avoid unnecessary messages in the system.log:
   *
   * soffice(57342) deny file-read-data /Users/tml/Documents/b.odt/..namedfork/rsrc
   * etc.
   *
   * Just don't bother with resolving aliases. I doubt its usefulness anyway.
   */
  (void) path;
  (void) buflen;
  return 0;
#else
#if MAC_OS_X_VERSION_MAX_ALLOWED < 1060
  FSRef aFSRef;
  OSStatus nErr;
  Boolean bFolder;
  Boolean bAliased;
#else
  CFStringRef cfpath;
  CFURLRef cfurl;
  CFErrorRef cferror;
  CFDataRef cfbookmark;
#endif

  char *unprocessedPath = path;

  if ( *unprocessedPath == '/' )
      unprocessedPath++;

  int nRet = 0;
  while ( !nRet && unprocessedPath && *unprocessedPath )
  {
      unprocessedPath = strchr( unprocessedPath, '/' );
      if ( unprocessedPath )
          *unprocessedPath = '\0';

#if MAC_OS_X_VERSION_MAX_ALLOWED < 1060
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
#else
      cfpath = CFStringCreateWithCString( NULL, path, kCFStringEncodingUTF8 );
      cfurl = CFURLCreateWithFileSystemPath( NULL, cfpath, kCFURLPOSIXPathStyle, false );
      CFRelease( cfpath );
      cferror = NULL;
      cfbookmark = CFURLCreateBookmarkDataFromFile( NULL, cfurl, &cferror );
      CFRelease( cfurl );

      if ( cfbookmark == NULL )
      {
          if(cferror)
          {
              CFRelease( cferror );
          }
      }
      else
      {
          Boolean isStale;
          cfurl = CFURLCreateByResolvingBookmarkData( NULL, cfbookmark, kCFBookmarkResolutionWithoutUIMask,
                                                      NULL, NULL, &isStale, &cferror );
          CFRelease( cfbookmark );
          if ( cfurl == NULL )
          {
              CFRelease( cferror );
          }
          else
          {
              cfpath = CFURLCopyFileSystemPath( cfurl, kCFURLPOSIXPathStyle );
              CFRelease( cfurl );
              if ( cfpath != NULL )
              {
                  char tmpPath[ PATH_MAX ];
                  if ( CFStringGetCString( cfpath, tmpPath, PATH_MAX, kCFStringEncodingUTF8 ) )
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
                              strcpy( path, tmpPath );
                          }
                      }
                      else
                      {
                          errno = ENAMETOOLONG;
                          nRet = -1;
                      }
                  }
                  CFRelease( cfpath );
              }
          }
      }
#endif

      if ( unprocessedPath )
          *unprocessedPath++ = '/';
  }

  return nRet;
#endif
}

#endif  /* defined MACOSX */

#endif /* NO_PTHREAD_RTL */

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

#if    ( defined(__GNUC__) && (defined(X86) || defined(X86_64)) )\
    || ( defined(SOLARIS) && defined(__i386) )

/* Safe default */
int osl_isSingleCPU = 0;

/* Determine if we are on a multiprocessor/multicore/HT x86/x64 system
 *
 * The lock prefix for atomic operations in osl_[inc|de]crementInterlockedCount()
 * comes with a cost and is especially expensive on pre HT x86 single processor
 * systems, where it isn't needed at all.
 *
 * This should be run as early as possible, thus it's placed in the init section
 */
#if defined(_SC_NPROCESSORS_CONF) /* i.e. MACOSX for Intel doesn't have this */
#if defined(__GNUC__)
void osl_interlockedCountCheckForSingleCPU(void)  __attribute__((constructor));
#endif

void osl_interlockedCountCheckForSingleCPU(void)
{
    /* In case sysconfig fails be on the safe side,
     * consider it a multiprocessor/multicore/HT system */
    if ( sysconf(_SC_NPROCESSORS_CONF) == 1 ) {
        osl_isSingleCPU = 1;
    }
}
#endif /* defined(_SC_NPROCESSORS_CONF) */
#endif

//might be useful on other platforms, but doesn't compiler under MACOSX anyway
#if defined(__GNUC__) && defined(LINUX)
//force the __data_start symbol to exist in any executables that link against
//libuno_sal so that dlopening of the libgcj provided libjvm.so on some
//platforms where it needs that symbol will succeed. e.g. Debian mips/lenny
//with gcc 4.3. With this in place the smoketest succeeds with libgcj provided
//java. Quite possibly also required/helpful for s390x/s390 and maybe some
//others. Without it the dlopen of libjvm.so will fail with __data_start
//not found
extern int __data_start[] __attribute__((weak));
extern int data_start[] __attribute__((weak));
extern int _end[] __attribute__((weak));
static void *dummy[] __attribute__((used)) = {__data_start, data_start, _end};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
