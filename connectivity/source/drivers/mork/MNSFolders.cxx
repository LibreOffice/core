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

#include <MNSFolders.hxx>

#ifdef UNIX
#include <sys/types.h>
#include <strings.h>
#include <string.h>
#endif // End UNIX

#ifdef WNT
#include "pre_include_windows.h"
#include <windows.h>
#include <stdlib.h>
#include <shlobj.h>
#include <objidl.h>
#include "post_include_windows.h"
#endif // End WNT
#include <osl/security.hxx>
#include <osl/file.hxx>
#include <osl/thread.h>

using namespace ::com::sun::star::mozilla;

namespace
{
    // -------------------------------------------------------------------
    static ::rtl::OUString lcl_getUserDataDirectory()
    {
        ::osl::Security   aSecurity;
        ::rtl::OUString   aConfigPath;

    #if defined(XP_WIN) || defined(MACOSX)
        aSecurity.getConfigDir( aConfigPath );
    #else
        //This is to find the dir under which .mozilla/.thunderbird is created.
        //mozilla doesn't honour XDG_CONFIG_HOME, so raw home dir required here
        //not xdg-config dir
        aSecurity.getHomeDir( aConfigPath );
    #endif

        return aConfigPath + ::rtl::OUString("/");
    }

    // -------------------------------------------------------------------
    const size_t NB_PRODUCTS = 3;
    const size_t NB_CANDIDATES = 4;

    static const char* DefaultProductDir[NB_PRODUCTS][NB_CANDIDATES] =
    {
    #if defined(XP_WIN)
        { "Mozilla/SeaMonkey/", NULL, NULL, NULL },
        { "Mozilla/Firefox/", NULL, NULL, NULL },
        { "Thunderbird/", "Mozilla/Thunderbird/", NULL, NULL }
    #elif(MACOSX)
        { "../Mozilla/SeaMonkey/", NULL, NULL, NULL },
        { "Firefox/", NULL, NULL, NULL },
        { "../Thunderbird/", NULL, NULL, NULL }
    #else
        { ".mozilla/seamonkey/", NULL, NULL, NULL },
        { ".mozilla/firefox/", NULL, NULL, NULL },
        { ".thunderbird/", ".mozilla-thunderbird/", ".mozilla/thunderbird/", ".icedove" }
    #endif
    };

    static const char* ProductRootEnvironmentVariable[NB_PRODUCTS] =
    {
        "MOZILLA_PROFILE_ROOT",
        "MOZILLA_FIREFOX_PROFILE_ROOT",
        "MOZILLA_THUNDERBIRD_PROFILE_ROOT",
    };

    // -------------------------------------------------------------------
    static ::rtl::OUString lcl_guessProfileRoot( MozillaProductType _product )
    {
        size_t productIndex = _product - 1;

        static ::rtl::OUString s_productDirectories[NB_PRODUCTS];

        if ( s_productDirectories[ productIndex ].isEmpty() )
        {
            ::rtl::OUString sProductPath;

            // check whether we have an anevironment variable which helps us
            const char* pProfileByEnv = getenv( ProductRootEnvironmentVariable[ productIndex ] );
            if ( pProfileByEnv )
            {
                sProductPath = ::rtl::OUString( pProfileByEnv, rtl_str_getLength( pProfileByEnv ), osl_getThreadTextEncoding() );
                // asume that this is fine, no further checks
            }
            else
            {
                ::rtl::OUString sProductDirCandidate;
                const char* pProfileRegistry = "profiles.ini";

                // check all possible candidates
                for ( size_t i=0; i<NB_CANDIDATES; ++i )
                {
                    if ( NULL == DefaultProductDir[ productIndex ][ i ] )
                        break;

                    sProductDirCandidate = lcl_getUserDataDirectory() +
                        ::rtl::OUString::createFromAscii( DefaultProductDir[ productIndex ][ i ] );

                    // check existence
                    ::osl::DirectoryItem aRegistryItem;
                    ::osl::FileBase::RC result = ::osl::DirectoryItem::get( sProductDirCandidate + ::rtl::OUString::createFromAscii( pProfileRegistry ), aRegistryItem );
                    if ( result == ::osl::FileBase::E_None  )
                    {
                        ::osl::FileStatus aStatus( osl_FileStatus_Mask_Validate );
                        result = aRegistryItem.getFileStatus( aStatus );
                        if ( result == ::osl::FileBase::E_None  )
                        {
                            // the registry file exists
                            break;
                        }
                    }
                }

                ::osl::FileBase::getSystemPathFromFileURL( sProductDirCandidate, sProductPath );
            }

            s_productDirectories[ productIndex ] = sProductPath;
        }

        return s_productDirectories[ productIndex ];
    }
}

// -----------------------------------------------------------------------
::rtl::OUString getRegistryDir(MozillaProductType product)
{
    if (product == MozillaProductType_Default)
        return ::rtl::OUString();

    return lcl_guessProfileRoot( product );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
