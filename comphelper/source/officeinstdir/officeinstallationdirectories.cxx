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

#include <config_folders.h>

#include "comphelper_module.hxx"

/**************************************************************************
                                TODO
 **************************************************************************

 *************************************************************************/

#include "osl/file.hxx"
#include "com/sun/star/beans/XPropertySet.hpp"
#include "com/sun/star/util/theMacroExpander.hpp"

#include "officeinstallationdirectories.hxx"

using namespace com::sun::star;

using namespace comphelper;

//=========================================================================
// helpers
//=========================================================================

//=========================================================================
static bool makeCanonicalFileURL( OUString & rURL )
{
    OSL_ENSURE( rURL.matchAsciiL( "file:", sizeof( "file:" ) - 1 , 0 ) ,
                "File URL expected!" );

    OUString aNormalizedURL;
    if ( osl::FileBase::getAbsoluteFileURL( OUString(),
                                            rURL,
                                            aNormalizedURL )
            == osl::DirectoryItem::E_None )
    {
        osl::DirectoryItem aDirItem;
        if ( osl::DirectoryItem::get( aNormalizedURL, aDirItem )
                == osl::DirectoryItem::E_None )
        {
            osl::FileStatus aFileStatus( osl_FileStatus_Mask_FileURL );

            if ( aDirItem.getFileStatus( aFileStatus )
                    == osl::DirectoryItem::E_None )
            {
                aNormalizedURL = aFileStatus.getFileURL();

                if ( !aNormalizedURL.isEmpty() )
                {
                    if ( aNormalizedURL
                            .getStr()[ aNormalizedURL.getLength() - 1 ]
                                != sal_Unicode( '/' ) )
                        rURL = aNormalizedURL;
                    else
                        rURL = aNormalizedURL
                                .copy( 0, aNormalizedURL.getLength() - 1 );

                    return true;
                }
            }
        }
    }
    return false;
}

//=========================================================================
//=========================================================================
//
// OfficeInstallationDirectories Implementation.
//
//=========================================================================
//=========================================================================

OfficeInstallationDirectories::OfficeInstallationDirectories(
        const uno::Reference< uno::XComponentContext > & xCtx )
: m_aOfficeBrandDirMacro( "$(brandbaseurl)" ),
  m_aOfficeBaseDirMacro( "$(baseinsturl)" ),
  m_aUserDirMacro( "$(userdataurl)" ),
  m_xCtx( xCtx ),
  m_pOfficeBrandDir( 0 ),
  m_pUserDir( 0 )
{
}

//=========================================================================
// virtual
OfficeInstallationDirectories::~OfficeInstallationDirectories()
{
    delete m_pOfficeBrandDir;
    delete m_pUserDir;
}

//=========================================================================
// util::XOfficeInstallationDirectories
//=========================================================================

// virtual
OUString SAL_CALL
OfficeInstallationDirectories::getOfficeInstallationDirectoryURL()
    throw ( uno::RuntimeException )
{
    initDirs();
    return OUString( *m_pOfficeBrandDir );
}

//=========================================================================
// virtual
OUString SAL_CALL
OfficeInstallationDirectories::getOfficeUserDataDirectoryURL()
    throw ( uno::RuntimeException )
{
    initDirs();
    return OUString( *m_pUserDir );
}


//=========================================================================
// virtual
OUString SAL_CALL
OfficeInstallationDirectories::makeRelocatableURL( const OUString& URL )
    throw ( uno::RuntimeException )
{
    if ( !URL.isEmpty() )
    {
        initDirs();

        OUString aCanonicalURL( URL );
        makeCanonicalFileURL( aCanonicalURL );

        sal_Int32 nIndex = aCanonicalURL.indexOf( *m_pOfficeBrandDir );
        if ( nIndex  != -1 )
        {
            return OUString(
                aCanonicalURL.replaceAt( nIndex,
                                         m_pOfficeBrandDir->getLength(),
                                         m_aOfficeBrandDirMacro ) );
        }
        else
        {
            nIndex = aCanonicalURL.indexOf( *m_pUserDir );
            if ( nIndex  != -1 )
            {
                return OUString(
                    aCanonicalURL.replaceAt( nIndex,
                                             m_pUserDir->getLength(),
                                             m_aUserDirMacro ) );
            }
        }
    }
    return OUString( URL );
}

//=========================================================================
// virtual
OUString SAL_CALL
OfficeInstallationDirectories::makeAbsoluteURL( const OUString& URL )
    throw ( uno::RuntimeException )
{
    if ( !URL.isEmpty() )
    {
        sal_Int32 nIndex = URL.indexOf( m_aOfficeBrandDirMacro );
        if ( nIndex != -1 )
        {
            initDirs();

            return OUString(
                URL.replaceAt( nIndex,
                               m_aOfficeBrandDirMacro.getLength(),
                               *m_pOfficeBrandDir ) );
        }
        else
        {
            nIndex = URL.indexOf( m_aUserDirMacro );
            if ( nIndex != -1 )
            {
                initDirs();

                return OUString(
                    URL.replaceAt( nIndex,
                                   m_aUserDirMacro.getLength(),
                                   *m_pUserDir ) );
            }
        }
    }
    return OUString( URL );
}

//=========================================================================
// lang::XServiceInfo
//=========================================================================

// virtual
OUString SAL_CALL
OfficeInstallationDirectories::getImplementationName()
    throw ( uno::RuntimeException )
{
    return getImplementationName_static();
}

//=========================================================================
// virtual
sal_Bool SAL_CALL
OfficeInstallationDirectories::supportsService( const OUString& ServiceName )
    throw ( uno::RuntimeException )
{
    const uno::Sequence< OUString > & aNames
        = getSupportedServiceNames();
    const OUString * p = aNames.getConstArray();
    for ( sal_Int32 nPos = 0; nPos < aNames.getLength(); nPos++ )
    {
        if ( p[ nPos ].equals( ServiceName ) )
            return sal_True;
    }
    return sal_False;

}

//=========================================================================
// virtual
uno::Sequence< OUString > SAL_CALL
OfficeInstallationDirectories::getSupportedServiceNames()
    throw ( uno::RuntimeException )
{
    return getSupportedServiceNames_static();
}

//=========================================================================
// static
OUString SAL_CALL
OfficeInstallationDirectories::getImplementationName_static()
{
    return OUString("com.sun.star.comp.util.OfficeInstallationDirectories");
}

//=========================================================================
// static
uno::Sequence< OUString > SAL_CALL
OfficeInstallationDirectories::getSupportedServiceNames_static()
{
    const OUString aServiceName("com.sun.star.util.OfficeInstallationDirectories");
    return uno::Sequence< OUString >( &aServiceName, 1 );
}

//=========================================================================
// static
OUString SAL_CALL OfficeInstallationDirectories::getSingletonName_static()
{
    return OUString("com.sun.star.util.theOfficeInstallationDirectories");
}

//=========================================================================
// static
uno::Reference< uno::XInterface > SAL_CALL
OfficeInstallationDirectories::Create(
        const uno::Reference< uno::XComponentContext > & rxContext )
{
    return static_cast< cppu::OWeakObject * >(
        new OfficeInstallationDirectories( rxContext ) );
}

//=========================================================================
// non-UNO
//=========================================================================

void OfficeInstallationDirectories::initDirs()
{
    if ( m_pOfficeBrandDir == 0 )
    {
        osl::MutexGuard aGuard( m_aMutex );
        if ( m_pOfficeBrandDir == 0 )
        {
            m_pOfficeBrandDir = new OUString;
            m_pUserDir        = new OUString;

            uno::Reference< util::XMacroExpander > xExpander = util::theMacroExpander::get(m_xCtx);

            *m_pOfficeBrandDir =
                xExpander->expandMacros(
                     OUString( "$BRAND_BASE_DIR" ) );

            OSL_ENSURE( !m_pOfficeBrandDir->isEmpty(),
                        "Unable to obtain office brand installation directory!" );

            makeCanonicalFileURL( *m_pOfficeBrandDir );

            *m_pUserDir =
                xExpander->expandMacros(
                    OUString("${$BRAND_BASE_DIR/" LIBO_ETC_FOLDER "/" SAL_CONFIGFILE( "bootstrap" ) ":UserInstallation}" ) );

            OSL_ENSURE( !m_pUserDir->isEmpty(),
                        "Unable to obtain office user data directory!" );

            makeCanonicalFileURL( *m_pUserDir );
        }
    }
}

void createRegistryInfo_OfficeInstallationDirectories()
{
    static ::comphelper::module::OSingletonRegistration< OfficeInstallationDirectories > aAutoRegistration;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
