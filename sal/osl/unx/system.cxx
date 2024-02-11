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

#include "system.hxx"

#ifdef NO_PTHREAD_RTL

#if defined(MACOSX)

#include <config_features.h>

#include <premac.h>
#include <Foundation/Foundation.h>
#include <postmac.h>

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

      // tdf#155710 handle conversion failures due to non-UTF8 strings
      // Windows and Linux paths can be passed as parameters to this function
      // and those paths may not always be UTF8 encoded like macOS paths.
      CFStringRef cfpath = CFStringCreateWithCString( nullptr, path, kCFStringEncodingUTF8 );
      CFErrorRef cferror = nullptr;
      CFDataRef cfbookmark = nullptr;
      if (cfpath)
      {
          CFURLRef cfurl = CFURLCreateWithFileSystemPath( nullptr, cfpath, kCFURLPOSIXPathStyle, false );
          CFRelease( cfpath );
          cfbookmark = CFURLCreateBookmarkDataFromFile( nullptr, cfurl, &cferror );
          CFRelease( cfurl );
      }

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
          CFURLRef cfurl = CFURLCreateByResolvingBookmarkData( nullptr, cfbookmark, kCFBookmarkResolutionWithoutUIMask,
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
//java. Quite possibly also required/helpful for s390x and maybe some
//others. Without it the dlopen of libjvm.so will fail with __data_start
//not found
extern int __data_start[] __attribute__((weak));
extern int data_start[] __attribute__((weak));
extern int _end[] __attribute__((weak));
static void *dummy[] __attribute__((used)) = {__data_start, data_start, _end};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
