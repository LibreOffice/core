/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_comphelper.hxx"

#include "comphelper_module.hxx"

/**************************************************************************
                                TODO
 **************************************************************************

 *************************************************************************/

#include "osl/file.hxx"
#include "com/sun/star/beans/XPropertySet.hpp"
#include "com/sun/star/util/XMacroExpander.hpp"

#include "officeinstallationdirectories.hxx"

using namespace com::sun::star;

using namespace comphelper;

//=========================================================================
// helpers
//=========================================================================

//=========================================================================
static bool makeCanonicalFileURL( rtl::OUString & rURL )
{
    OSL_ENSURE( rURL.matchAsciiL( "file:", sizeof( "file:" ) - 1 , 0 ) ,
                "File URL expected!" );

    rtl::OUString aNormalizedURL;
    if ( osl::FileBase::getAbsoluteFileURL( rtl::OUString(),
                                            rURL,
                                            aNormalizedURL )
            == osl::DirectoryItem::E_None )
    {
        osl::DirectoryItem aDirItem;
        if ( osl::DirectoryItem::get( aNormalizedURL, aDirItem )
                == osl::DirectoryItem::E_None )
        {
            osl::FileStatus aFileStatus( FileStatusMask_FileURL );

            if ( aDirItem.getFileStatus( aFileStatus )
                    == osl::DirectoryItem::E_None )
            {
                aNormalizedURL = aFileStatus.getFileURL();

                if ( aNormalizedURL.getLength() > 0 )
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
: m_aOfficeBrandDirMacro( RTL_CONSTASCII_USTRINGPARAM( "$(brandbaseurl)" ) ),
  m_aOfficeBaseDirMacro( RTL_CONSTASCII_USTRINGPARAM( "$(baseinsturl)" ) ),
  m_aUserDirMacro( RTL_CONSTASCII_USTRINGPARAM( "$(userdataurl)" ) ),
  m_xCtx( xCtx ),
  m_pOfficeBrandDir( 0 ),
  m_pOfficeBaseDir( 0 ),
  m_pUserDir( 0 )
{
}

//=========================================================================
// virtual
OfficeInstallationDirectories::~OfficeInstallationDirectories()
{
    delete m_pOfficeBrandDir;
    delete m_pOfficeBaseDir;
    delete m_pUserDir;
}

//=========================================================================
// util::XOfficeInstallationDirectories
//=========================================================================

// virtual
rtl::OUString SAL_CALL
OfficeInstallationDirectories::getOfficeInstallationDirectoryURL()
    throw ( uno::RuntimeException )
{
    initDirs();
    return rtl::OUString( *m_pOfficeBrandDir );
}

//=========================================================================
// virtual
rtl::OUString SAL_CALL
OfficeInstallationDirectories::getOfficeUserDataDirectoryURL()
    throw ( uno::RuntimeException )
{
    initDirs();
    return rtl::OUString( *m_pUserDir );
}


//=========================================================================
// virtual
rtl::OUString SAL_CALL
OfficeInstallationDirectories::makeRelocatableURL( const rtl::OUString& URL )
    throw ( uno::RuntimeException )
{
    if ( URL.getLength() > 0 )
    {
        initDirs();

        rtl::OUString aCanonicalURL( URL );
        makeCanonicalFileURL( aCanonicalURL );

        sal_Int32 nIndex = aCanonicalURL.indexOf( *m_pOfficeBrandDir );
        if ( nIndex  != -1 )
        {
            return rtl::OUString(
                URL.replaceAt( nIndex,
                               m_pOfficeBrandDir->getLength(),
                               m_aOfficeBrandDirMacro ) );
        }
        else
        {
            nIndex = aCanonicalURL.indexOf( *m_pOfficeBaseDir );
            if ( nIndex  != -1 )
            {
                return rtl::OUString(
                    URL.replaceAt( nIndex,
                                   m_pOfficeBaseDir->getLength(),
                                   m_aOfficeBaseDirMacro ) );
            }
            else
            {
                nIndex = aCanonicalURL.indexOf( *m_pUserDir );
                if ( nIndex  != -1 )
                {
                    return rtl::OUString(
                        URL.replaceAt( nIndex,
                                       m_pUserDir->getLength(),
                                       m_aUserDirMacro ) );
                }
            }
        }
    }
    return rtl::OUString( URL );
}

//=========================================================================
// virtual
rtl::OUString SAL_CALL
OfficeInstallationDirectories::makeAbsoluteURL( const rtl::OUString& URL )
    throw ( uno::RuntimeException )
{
    if ( URL.getLength() > 0 )
    {
        sal_Int32 nIndex = URL.indexOf( m_aOfficeBrandDirMacro );
        if ( nIndex != -1 )
        {
            initDirs();

            return rtl::OUString(
                URL.replaceAt( nIndex,
                               m_aOfficeBrandDirMacro.getLength(),
                               *m_pOfficeBrandDir ) );
        }
        else
        {
            nIndex = URL.indexOf( m_aOfficeBaseDirMacro );
            if ( nIndex != -1 )
            {
                initDirs();

                return rtl::OUString(
                    URL.replaceAt( nIndex,
                                   m_aOfficeBaseDirMacro.getLength(),
                                   *m_pOfficeBaseDir ) );
            }
            else
            {
                nIndex = URL.indexOf( m_aUserDirMacro );
                if ( nIndex != -1 )
                {
                    initDirs();

                    return rtl::OUString(
                        URL.replaceAt( nIndex,
                                       m_aUserDirMacro.getLength(),
                                       *m_pUserDir ) );
                }
            }
        }
    }
    return rtl::OUString( URL );
}

//=========================================================================
// lang::XServiceInfo
//=========================================================================

// virtual
rtl::OUString SAL_CALL
OfficeInstallationDirectories::getImplementationName()
    throw ( uno::RuntimeException )
{
    return getImplementationName_static();
}

//=========================================================================
// virtual
sal_Bool SAL_CALL
OfficeInstallationDirectories::supportsService( const rtl::OUString& ServiceName )
    throw ( uno::RuntimeException )
{
    const uno::Sequence< rtl::OUString > & aNames
        = getSupportedServiceNames();
    const rtl::OUString * p = aNames.getConstArray();
    for ( sal_Int32 nPos = 0; nPos < aNames.getLength(); nPos++ )
    {
        if ( p[ nPos ].equals( ServiceName ) )
            return sal_True;
    }
    return sal_False;

}

//=========================================================================
// virtual
uno::Sequence< ::rtl::OUString > SAL_CALL
OfficeInstallationDirectories::getSupportedServiceNames()
    throw ( uno::RuntimeException )
{
    return getSupportedServiceNames_static();
}

//=========================================================================
// static
rtl::OUString SAL_CALL
OfficeInstallationDirectories::getImplementationName_static()
{
    return rtl::OUString(
        RTL_CONSTASCII_USTRINGPARAM(
            "com.sun.star.comp.util.OfficeInstallationDirectories" ) );
}

//=========================================================================
// static
uno::Sequence< ::rtl::OUString > SAL_CALL
OfficeInstallationDirectories::getSupportedServiceNames_static()
{
    const rtl::OUString aServiceName(
        RTL_CONSTASCII_USTRINGPARAM(
            "com.sun.star.util.OfficeInstallationDirectories" ) );
    return uno::Sequence< rtl::OUString >( &aServiceName, 1 );
}

//=========================================================================
// static
rtl::OUString SAL_CALL OfficeInstallationDirectories::getSingletonName_static()
{
    return rtl::OUString(
        RTL_CONSTASCII_USTRINGPARAM(
            "com.sun.star.util.theOfficeInstallationDirectories" ) );
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
            m_pOfficeBrandDir = new rtl::OUString;
            m_pOfficeBaseDir  = new rtl::OUString;
            m_pUserDir        = new rtl::OUString;

            uno::Reference< util::XMacroExpander > xExpander;

            m_xCtx->getValueByName(
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                    "/singletons/com.sun.star.util.theMacroExpander" ) ) )
            >>= xExpander;

            OSL_ENSURE( xExpander.is(),
                        "Unable to obtain macro expander singleton!" );

            if ( xExpander.is() )
            {
                *m_pOfficeBrandDir =
                    xExpander->expandMacros(
                         rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "$BRAND_BASE_DIR" ) ) );

                OSL_ENSURE( m_pOfficeBrandDir->getLength() > 0,
                            "Unable to obtain office brand installation directory!" );

                makeCanonicalFileURL( *m_pOfficeBrandDir );

                *m_pOfficeBaseDir =
                    xExpander->expandMacros(
                        rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                            "${$BRAND_BASE_DIR/program/" SAL_CONFIGFILE( "bootstrap" ) ":BaseInstallation}" ) ) );

                OSL_ENSURE( m_pOfficeBaseDir->getLength() > 0,
                            "Unable to obtain office base installation directory!" );

                makeCanonicalFileURL( *m_pOfficeBaseDir );

                *m_pUserDir =
                    xExpander->expandMacros(
                        rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                            "${$BRAND_BASE_DIR/program/" SAL_CONFIGFILE( "bootstrap" ) ":UserInstallation}" ) ) );

                OSL_ENSURE( m_pUserDir->getLength() > 0,
                            "Unable to obtain office user data directory!" );

                makeCanonicalFileURL( *m_pUserDir );
            }
        }
    }
}

void createRegistryInfo_OfficeInstallationDirectories()
{
    static ::comphelper::module::OSingletonRegistration< OfficeInstallationDirectories > aAutoRegistration;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
