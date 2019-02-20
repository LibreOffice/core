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

#include "system.hxx"

#ifdef NO_PTHREAD_RTL

/* struct passwd differs on some platforms */

#if defined(MACOSX) || defined(IOS) || defined(OPENBSD) || defined(NETBSD) || defined(HAIKU)

//No mutex needed on macOS, gethostbyname is thread safe

#if defined(MACOSX)

#define RTL_MUTEX_LOCK
#define RTL_MUTEX_UNLOCK

#include <premac.h>
#include <Foundation/Foundation.h>
#include <postmac.h>

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
        int nname, naliases, naddr_list, naliasesdata;
        char **p, **parray, *data;

        /* Check buffer size before copying, we want to leave the
         * buffers unmodified in case something goes wrong.
         *
         * Is this required?
         */

        nname= strlen(res->h_name)+1;

        naliases = naddr_list = naliasesdata = 0;

        for ( p = res->h_aliases; *p != nullptr; p++) {
            naliases++;
            naliasesdata += strlen(*p)+1;
        }

        for ( p = res->h_addr_list; *p != nullptr; p++)
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

            parray = reinterpret_cast<char**>(buffer);
            result->h_aliases = parray;
            data = buffer + (naliases+1)*sizeof(char*);
            for ( p = res->h_aliases; *p != nullptr; p++) {
                int n = strlen(*p)+1;
                *parray++ = data;
                memcpy(data, *p, n);
                data += n;
            }
            *parray = nullptr;
            buffer = data;
            parray = reinterpret_cast<char**>(buffer);
            result->h_addr_list = parray;
            data = buffer + (naddr_list+1)*sizeof(char*);
            for ( p = res->h_addr_list; *p != nullptr; p++) {
                *parray++ = data;
                memcpy(data, *p, res->h_length);
                data += res->h_length;
            }
            *parray = nullptr;

            res = result;
        }
        else
        {
            errno = ERANGE;
            res = nullptr;
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
  CFStringRef cfpath;
  CFURLRef cfurl;
  CFErrorRef cferror;
  CFDataRef cfbookmark;

  // Don't even try anything for files inside the app bundle. Just a
  // waste of time.

  static const char * const appBundle = [[[NSBundle mainBundle] bundlePath] UTF8String];

  const size_t appBundleLen = strlen(appBundle);
  if (strncmp(path, appBundle, appBundleLen) == 0 && path[appBundleLen] == '/')
      return 0;

  char *unprocessedPath = path;

  if ( *unprocessedPath == '/' )
      unprocessedPath++;

  int nRet = 0;
  while ( !nRet && unprocessedPath && *unprocessedPath )
  {
      unprocessedPath = strchr( unprocessedPath, '/' );
      if ( unprocessedPath )
          *unprocessedPath = '\0';

      cfpath = CFStringCreateWithCString( nullptr, path, kCFStringEncodingUTF8 );
      cfurl = CFURLCreateWithFileSystemPath( nullptr, cfpath, kCFURLPOSIXPathStyle, false );
      CFRelease( cfpath );
      cferror = nullptr;
      cfbookmark = CFURLCreateBookmarkDataFromFile( nullptr, cfurl, &cferror );
      CFRelease( cfurl );

      if ( cfbookmark == nullptr )
      {
          if(cferror)
          {
              CFRelease( cferror );
          }
      }
      else
      {
          Boolean isStale;
          cfurl = CFURLCreateByResolvingBookmarkData( nullptr, cfbookmark, kCFBookmarkResolutionWithoutUIMask,
                                                      nullptr, nullptr, &isStale, &cferror );
          CFRelease( cfbookmark );
          if ( cfurl == nullptr )
          {
              CFRelease( cferror );
          }
          else
          {
              cfpath = CFURLCopyFileSystemPath( cfurl, kCFURLPOSIXPathStyle );
              CFRelease( cfurl );
              if ( cfpath != nullptr )
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

      if ( unprocessedPath )
          *unprocessedPath++ = '/';
  }

  return nRet;
#endif
}

#endif  /* defined MACOSX */

#endif /* NO_PTHREAD_RTL */

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
