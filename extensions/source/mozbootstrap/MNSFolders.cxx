/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_extensions.hxx"
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

        aSecurity.getConfigDir( aConfigPath );
        return aConfigPath + ::rtl::OUString::createFromAscii( "/" );
    }

    // -------------------------------------------------------------------
    static const char* DefaultProductDir[3][3] =
    {
    #if defined(XP_WIN)
        { "Mozilla/SeaMonkey/", NULL, NULL },
        { "Mozilla/Firefox/", NULL, NULL },
        { "Thunderbird/", "Mozilla/Thunderbird/", NULL }
    #elif(MACOSX)
        { "../Mozilla/SeaMonkey/", NULL, NULL },
        { "Firefox/", NULL, NULL },
        { "../Thunderbird/", NULL, NULL }
    #else
        { ".mozilla/seamonkey/", NULL, NULL },
        { ".mozilla/firefox/", NULL, NULL },
        { ".thunderbird/", ".mozilla-thunderbird/", ".mozilla/thunderbird/" }
    #endif
    };

    static const char* ProductRootEnvironmentVariable[3] =
    {
        "MOZILLA_PROFILE_ROOT",
        "MOZILLA_FIREFOX_PROFILE_ROOT",
        "MOZILLA_THUNDERBIRD_PROFILE_ROOT",
    };

    // -------------------------------------------------------------------
    static ::rtl::OUString lcl_guessProfileRoot( MozillaProductType _product )
    {
        size_t productIndex = _product - 1;

        static ::rtl::OUString s_productDirectories[3];

        if ( !s_productDirectories[ productIndex ].getLength() )
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
                for ( size_t i=0; i<3; ++i )
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
                        ::osl::FileStatus aStatus( FileStatusMask_Validate );
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
