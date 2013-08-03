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
   * kernel[0]: Sandbox: soffice(57342) deny file-read-data /Users
   * kernel[0]: Sandbox: soffice(57342) deny file-read-data /Users/tml
   *
   * etc. It is quite unlikely anyway, I hope, that anything except
   * the last component of a path name would be a bookmark.
   */
  char *unprocessedPath = path + strlen(path) - 1;
#else
  char *unprocessedPath = path;
#endif

  if ( *unprocessedPath == '/' )
      unprocessedPath++;

  int nRet = 0;
  while ( !nRet && unprocessedPath && *unprocessedPath )
  {
      unprocessedPath = strchr( unprocessedPath, '/' );
      if ( unprocessedPath )
          *unprocessedPath = '\0';

      CFStringRef cfpath = CFStringCreateWithCString( NULL, path, kCFStringEncodingUTF8 );
      CFURLRef cfurl = CFURLCreateWithFileSystemPath( NULL, cfpath, kCFURLPOSIXPathStyle, false );
      CFRelease( cfpath );
      CFErrorRef cferror = NULL;
      CFDataRef cfbookmark = CFURLCreateBookmarkDataFromFile( NULL, cfurl, &cferror );
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

      if ( unprocessedPath )
          *unprocessedPath++ = '/';
  }

  return nRet;
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
