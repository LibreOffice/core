/*************************************************************************
 *
 *  $RCSfile: regpathhelper.cxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: jsc $ $Date: 2001-07-23 10:14:42 $
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

#ifndef _OSL_FILE_HXX_
#include <osl/file.hxx>
#endif
#ifndef _OSL_SECURITY_HXX_
#include <osl/security.hxx>
#endif
#ifndef _OSL_THREAD_H_
#include <osl/thread.h>
#endif
#ifndef _VOS_PROCESS_HXX_
#include <vos/process.hxx>
#endif
#ifndef _RTL_TEXTENC_H
#include <rtl/textenc.h>
#endif
#ifndef _RTL_URI_H_
#include <rtl/uri.h>
#endif
#ifndef _RTL_URI_HXX_
#include <rtl/uri.hxx>
#endif

using namespace vos;
using namespace osl;
using namespace rtl;

#define PATH_DELEMITTER '/'

#define USER_REGISTRY_NAME_ENV      "STAR_USER_REGISTRY"
#define SYSTEM_REGISTRY_NAME_ENV    "STAR_REGISTRY"
#define REGISTRY_SYSTEM_NAME    "applicat.rdb"

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
        if(  portalUserDir.getLength() )
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
    if ( sBuffer.getLength() > 0 )
    {
        f = fopen( sBuffer.getStr(), "r" );

        if (f != NULL)
        {
            fclose(f);
            userRegistryName = OStringToOUString( sBuffer, osl_getThreadTextEncoding() );
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
        if ( tmpStr.getLength() > 0 )
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

