/*************************************************************************
 *
 *  $RCSfile: regpathhelper.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: jsc $ $Date: 2000-11-06 12:37:11 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include <stdio.h>

#ifndef _OSL_SECURITY_H_
#include <osl/security.h>
#endif
#ifndef _OSL_FILE_HXX_
#include <osl/file.hxx>
#endif
#ifndef _VOS_PROCESS_HXX_
#include <vos/process.hxx>
#endif
#ifndef _VOS_PROFILE_HXX_
#include <vos/profile.hxx>
#endif
#ifndef _OSL_FILE_HXX_
#include <osl/file.hxx>
#endif
#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif


#if defined ( UNX ) || defined( MAC )
#include <limits.h>
#define _MAX_PATH PATH_MAX
#endif

using namespace vos;
using namespace osl;
using namespace rtl;

#define PATH_DELEMITTER '/'

#define USER_REGISTRY_NAME_ENV      "STAR_USER_REGISTRY"
#define SYSTEM_REGISTRY_NAME_ENV    "STAR_REGISTRY"

#define SVERSION_NAME           "sversion"
#define REGISTRY_SECTION        "Registry"
#define REGISTRY_VERSION_KEY    "UserRegistry 6.0"
#define REGISTRY_LOCAL_NAME     "user60.rdb"
#define REGISTRY_SYSTEM_NAME    "applicat.rdb"

#define LOCALREGISTRY_DEFAULTLOCATION       "?^"
#define LOCALREGISTRY_PORTALLOCATION        "?#"

#define SVERSION_LOCATION       "?^"
#define SVERSION_FALLBACK       "?$"

namespace comphelper
{

static OUString getDefaultLocalRegistry()
{
    OUString uBuffer, userRegistryName;

    OStartupInfo startInfo;
    sal_uInt32 nArgs = startInfo.getCommandArgCount();
    sal_Bool bIsPortalUser = sal_False, bFindProfile = sal_False;
    OUString sArg;
    while( nArgs > 0 )
      {
        if ( !startInfo.getCommandArg(--nArgs, sArg) )
          {
            if ( sArg.indexOf(OUString::createFromAscii("-userid")) == 0 )
            {
                  bIsPortalUser = sal_True;
                  break;
            }
          }
     }

    if ( bIsPortalUser )
       {
        bFindProfile = OProfile::getProfileName(uBuffer,
                               OUString::createFromAscii(REGISTRY_LOCAL_NAME),
                               OUString::createFromAscii(LOCALREGISTRY_PORTALLOCATION));

        if (bFindProfile)
        {
            sal_Int32 tokenCount = uBuffer.getTokenCount('/');
            OUString sSeperator(RTL_CONSTASCII_USTRINGPARAM("/"));
            OUString sPath(RTL_CONSTASCII_USTRINGPARAM("//"));
            FileBase::RC retRC = FileBase::E_None;

            sPath += uBuffer.getToken(2, '/');
            for (sal_Int32 i = 3; i < tokenCount - 1; i++)
            {
                sPath += sSeperator;
                sPath += uBuffer.getToken(i, '/');

                retRC = Directory::create(sPath);
                if ( retRC != FileBase::E_None && retRC != FileBase::E_EXIST)
                {
                    return OUString();
                }
            }
        }
    } else
       {
        bFindProfile = OProfile::getProfileName(uBuffer,
                               OUString::createFromAscii(REGISTRY_LOCAL_NAME),
                               OUString::createFromAscii(LOCALREGISTRY_DEFAULTLOCATION));
    }
    if ( bFindProfile )
    {
        FileBase::getSystemPathFromNormalizedPath(uBuffer, userRegistryName);
    }

    return userRegistryName;
}


OUString getPathToUserRegistry()
{
    OUString    uBuffer;
    OUString    userRegistryName;
    sal_Char    buffer[_MAX_PATH]= "";
    FILE        *f=NULL;
    sal_Bool    findProfile = sal_False;

    // search the environment STAR_USER_REGISTRY
    OString sBuffer( getenv(USER_REGISTRY_NAME_ENV) );
    if ( sBuffer.getLength() > 0 )
    {
        f = fopen( sBuffer.getStr(), "r" );

        if (f != NULL)
        {
            fclose(f);
            userRegistryName = OStringToOUString( sBuffer, RTL_TEXTENCODING_ASCII_US);
            return userRegistryName;
        }
    }

    // search for entry in sversion.ini in config directory
    if ( OProfile::getProfileName( uBuffer,
                                   OUString( RTL_CONSTASCII_USTRINGPARAM(SVERSION_NAME) ),
                                   OUString( RTL_CONSTASCII_USTRINGPARAM(SVERSION_LOCATION) )) )
    {
        OProfile profile;

        if ( profile.open(uBuffer) )
        {
            findProfile = sal_True;

            OString userRegistryKey( REGISTRY_VERSION_KEY );
            userRegistryKey += "/";
            userRegistryKey += OString::valueOf((sal_Int32)SUPD);

            if ( profile.readString(REGISTRY_SECTION, userRegistryKey.getStr(), buffer, _MAX_PATH, "") )
            {
                if ( buffer[0] == '\0' )
                {
                    if ( profile.readString(REGISTRY_SECTION, REGISTRY_VERSION_KEY, buffer, _MAX_PATH, "") )
                        sBuffer = buffer;
                } else
                {
                    sBuffer = buffer;
                }
            }
        }

        if ( sBuffer.getLength() > 0 )
        {
            f = fopen( sBuffer.getStr(), "r");

            if (f != NULL)
            {
                fclose(f);
                userRegistryName = OStringToOUString(sBuffer, RTL_TEXTENCODING_ASCII_US);
                return userRegistryName;
            }
        }
    }
    // search for entry in sversion.ini in system directory
    if ( !findProfile &&
         OProfile::getProfileName( uBuffer,
                                    OUString::createFromAscii(SVERSION_NAME),
                                    OUString::createFromAscii(SVERSION_FALLBACK) ))
    {
        OProfile profile;

        if ( profile.open(uBuffer) )
        {
            OString userRegistryKey(REGISTRY_VERSION_KEY);
            userRegistryKey += OString::valueOf(PATH_DELEMITTER);
            userRegistryKey += OString::valueOf((sal_Int32)SUPD);

            if ( profile.readString(REGISTRY_SECTION, userRegistryKey.getStr(), buffer, _MAX_PATH, "") )
            {
                if (buffer[0] == '\0')
                {
                    if (profile.readString(REGISTRY_SECTION, REGISTRY_VERSION_KEY, buffer, _MAX_PATH, ""))
                        sBuffer = buffer;
                } else
                {
                    sBuffer = buffer;
                }
            }
        }

        if ( sBuffer.getLength() > 0 )
        {
            f = fopen( sBuffer.getStr(), "r" );

            if (f != NULL)
            {
                fclose(f);
                userRegistryName = OStringToOUString(sBuffer, RTL_TEXTENCODING_ASCII_US);
                return userRegistryName;
            }
        }
    }

    // default security of logged in user
    OSecurity sec;
    if ( sec.getConfigDir(uBuffer) && uBuffer.getLength() )
    {
        // search without dot
        OUString normalizedPath;
        if (!FileBase::getSystemPathFromNormalizedPath(uBuffer, normalizedPath))
        {
            sBuffer = OUStringToOString(normalizedPath, RTL_TEXTENCODING_ASCII_US);
            if ( sBuffer.getLength() > 0 )
                sBuffer += OString::valueOf(PATH_DELEMITTER);
            sBuffer += REGISTRY_LOCAL_NAME;

            userRegistryName = OStringToOUString( sBuffer, RTL_TEXTENCODING_ASCII_US);
            f = fopen(sBuffer.getStr(), "r");

            if (f == NULL)
            {
                // search with dot
                sBuffer = OUStringToOString(normalizedPath, RTL_TEXTENCODING_ASCII_US);
                if ( sBuffer.getLength() > 0 )
                    sBuffer += OString::valueOf(PATH_DELEMITTER);

                sBuffer += ".";
                sBuffer += REGISTRY_LOCAL_NAME;

                userRegistryName = OStringToOUString( sBuffer, RTL_TEXTENCODING_ASCII_US);
                f = fopen( sBuffer.getStr(), "r" );

                if (f == NULL)
                {
                    userRegistryName = OUString();
                }
            }

            if (f != NULL)
            {
                fclose(f);
            }
        }
    }

    if ( !userRegistryName.getLength() )
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
    if( OStartupInfo::E_None == OStartupInfo().getExecutableFile(uBuffer) )
    {
        sal_uInt32  lastIndex = uBuffer.lastIndexOf(PATH_DELEMITTER);
        if (lastIndex > 0)
        {
            uBuffer = uBuffer.copy(0, lastIndex + 1);
        }

        uBuffer += registryBaseName;

        if (!FileBase::getSystemPathFromNormalizedPath(uBuffer, systemRegistryName))
        {
            OString tmpStr( OUStringToOString(systemRegistryName, RTL_TEXTENCODING_ASCII_US) );
            f = fopen( tmpStr.getStr(), "r" );
        }
    }

    if (f == NULL)
    {
        // search the environment STAR_REGISTRY
        OString tmpStr( getenv(SYSTEM_REGISTRY_NAME_ENV) );
        if ( tmpStr.getLength() > 0 )
        {
            f = fopen(tmpStr.getStr(), "r");

            if (f != NULL)
            {
                fclose(f);
                systemRegistryName = OStringToOUString( tmpStr, RTL_TEXTENCODING_ASCII_US);
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

