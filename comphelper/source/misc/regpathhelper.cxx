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
#include "precompiled_comphelper.hxx"

#include <stdio.h>
#include <osl/file.hxx>
#include <osl/security.hxx>
#include <osl/thread.h>
#include <vos/process.hxx>
#include <rtl/textenc.h>
#include <rtl/uri.h>
#include <rtl/uri.hxx>

using namespace vos;
using namespace osl;
using namespace rtl;

#define PATH_DELEMITTER '/'

#define USER_REGISTRY_NAME_ENV      "STAR_USER_REGISTRY"
#define SYSTEM_REGISTRY_NAME_ENV    "STAR_REGISTRY"
#define REGISTRY_SYSTEM_NAME    "services.rdb"

#define REGISTRY_LOCAL_NAME     "user60.rdb"

#ifdef SAL_UNX
#define CONFIG_PATH_PREFIX      "."
#else
#define CONFIG_PATH_PREFIX      ""
#endif

namespace comphelper
{

/**
   @return sal_True, if the office is started in a portal
                     environment.
           sal_False, if the common office is started
 */
static sal_Bool retrievePortalUserDir( OUString *pDirectory )
{
    OStartupInfo startInfo;
    sal_uInt32 nArgs = startInfo.getCommandArgCount();
    sal_Bool bIsPortalUser = sal_False;
    OUString sArg;
    while( nArgs > 0 )
      {
        if ( !startInfo.getCommandArg(--nArgs, sArg) )
          {
            if ( sArg.indexOf(OUString::createFromAscii("-userid")) == 0 )
            {

                  bIsPortalUser = sal_True;
                sal_Int32 nStart = sArg.lastIndexOf( '[' );
                sal_Int32 nEnd   = sArg.lastIndexOf( ']' );
                if( -1 == nStart || -1 == nEnd || nEnd < nStart)
                {
                    *pDirectory = OUString();
                }
                else
                {
                    OUString aEncHome = sArg.copy( nStart + 1 , nEnd - nStart -1 );
                    *pDirectory = rtl::Uri::decode(aEncHome,
                                                   rtl_UriDecodeWithCharset,
                                                   RTL_TEXTENCODING_UTF8);
                }
                  break;
            }
          }
     }
    return bIsPortalUser;
}


static OUString getDefaultLocalRegistry()
{
    OUString uBuffer, userRegistryName;
    OUString portalUserDir;

    sal_Bool bIsPortalUser = retrievePortalUserDir( &portalUserDir );

    if ( bIsPortalUser )
       {
        if( !portalUserDir.isEmpty() )
        {
            FileBase::getFileURLFromSystemPath( portalUserDir , portalUserDir );
            userRegistryName = portalUserDir;
            userRegistryName += OUString( RTL_CONSTASCII_USTRINGPARAM(
                "/user/" REGISTRY_LOCAL_NAME ) );

            // Directory creation is probably necessary for bootstrapping a new
            // user in the portal environment (the ucb uses this function).
            // This should be solved differently, as
            // no one expects this function to create anything ...
            OUString sSeparator(RTL_CONSTASCII_USTRINGPARAM("/"));
            OUString sPath(RTL_CONSTASCII_USTRINGPARAM("file://"));
            FileBase::RC retRC = FileBase::E_None;

            sal_Int32 nIndex = 3;
            sPath += userRegistryName.getToken(2, '/', nIndex);
            while( nIndex != -1 )
            {
                sPath += sSeparator;
                sPath += userRegistryName.getToken(0, '/', nIndex);
                if( nIndex == -1 )
                    break;
                Directory aDir( sPath );
                if( aDir.open() == FileBase::E_NOENT )
                {
                    retRC = Directory::create(sPath);
                    if ( retRC != FileBase::E_None && retRC != FileBase::E_EXIST)
                    {
                        return OUString();
                    }
                }
            }
        }
    }
    else /* bIsPortalUser */
    {
        ::osl::Security aUserSecurity;
        aUserSecurity.getConfigDir( userRegistryName );
        userRegistryName += OUString( RTL_CONSTASCII_USTRINGPARAM(
            "/" CONFIG_PATH_PREFIX REGISTRY_LOCAL_NAME ) );
    }

    return userRegistryName;
}


OUString getPathToUserRegistry()
{
    OUString    userRegistryName;
    FILE        *f=NULL;

    // search the environment STAR_USER_REGISTRY
    OString sBuffer( getenv(USER_REGISTRY_NAME_ENV) );
    if ( !sBuffer.isEmpty() )
    {
        f = fopen( sBuffer.getStr(), "r" );

        if (f != NULL)
        {
            fclose(f);
            userRegistryName = OStringToOUString( sBuffer, osl_getThreadTextEncoding() );
        }
    }

    if ( userRegistryName.isEmpty() )
    {
        userRegistryName = getDefaultLocalRegistry();
    }

    return userRegistryName;
}

OUString getPathToSystemRegistry()
{
    OUString uBuffer;
    OUString registryBaseName( RTL_CONSTASCII_USTRINGPARAM(REGISTRY_SYSTEM_NAME) );
    OUString systemRegistryName;
    FILE     *f=NULL;

    // search in the directory of the executable
    OStartupInfo info;
    if( OStartupInfo::E_None == info.getExecutableFile(uBuffer) )
    {
        sal_uInt32  lastIndex = uBuffer.lastIndexOf(PATH_DELEMITTER);
        if (lastIndex > 0)
        {
            uBuffer = uBuffer.copy(0, lastIndex + 1);
        }

        uBuffer += registryBaseName;

        if (!FileBase::getSystemPathFromFileURL(uBuffer, systemRegistryName))
        {
            OString tmpStr( OUStringToOString(systemRegistryName, osl_getThreadTextEncoding()) );
            f = fopen( tmpStr.getStr(), "r" );
        }
    }

    if (f == NULL)
    {
        // search the environment STAR_REGISTRY
        OString tmpStr( getenv(SYSTEM_REGISTRY_NAME_ENV) );
        if ( !tmpStr.isEmpty() )
        {
            f = fopen(tmpStr.getStr(), "r");

            if (f != NULL)
            {
                fclose(f);
                systemRegistryName = OStringToOUString( tmpStr, osl_getThreadTextEncoding() );
            } else
            {
                systemRegistryName = OUString();
            }
        }
    } else
    {
        fclose(f);
    }

    return systemRegistryName;
}

}

