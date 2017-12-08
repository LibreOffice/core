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

#include <cppuhelper/supportsservice.hxx>

/**************************************************************************
                                TODO
 **************************************************************************

 *************************************************************************/

#include <osl/file.hxx>
#include <rtl/ref.hxx>
#include <com/sun/star/util/theMacroExpander.hpp>
#include <comphelper/fileurl.hxx>

#include "officeinstallationdirectories.hxx"

using namespace com::sun::star;

static bool makeCanonicalFileURL( OUString & rURL )
{
    OSL_ENSURE(comphelper::isFileUrl(rURL), "File URL expected!");

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
                    if ( !aNormalizedURL.endsWith("/") )
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

namespace comphelper {

OfficeInstallationDirectories::OfficeInstallationDirectories(
        const uno::Reference< uno::XComponentContext > & xCtx )
: m_aOfficeBrandDirMacro( "$(brandbaseurl)" ),
  m_aUserDirMacro( "$(userdataurl)" ),
  m_xCtx( xCtx ),
  m_pOfficeBrandDir( nullptr ),
  m_pUserDir( nullptr )
{
}


// virtual
OfficeInstallationDirectories::~OfficeInstallationDirectories()
{
}


// util::XOfficeInstallationDirectories


// virtual
OUString SAL_CALL
OfficeInstallationDirectories::getOfficeInstallationDirectoryURL()
{
    initDirs();
    return *m_pOfficeBrandDir;
}


// virtual
OUString SAL_CALL
OfficeInstallationDirectories::getOfficeUserDataDirectoryURL()
{
    initDirs();
    return *m_pUserDir;
}



// virtual
OUString SAL_CALL
OfficeInstallationDirectories::makeRelocatableURL( const OUString& URL )
{
    if ( !URL.isEmpty() )
    {
        initDirs();

        OUString aCanonicalURL( URL );
        makeCanonicalFileURL( aCanonicalURL );

        sal_Int32 nIndex = aCanonicalURL.indexOf( *m_pOfficeBrandDir );
        if ( nIndex  != -1 )
        {
            return
                aCanonicalURL.replaceAt( nIndex,
                                         m_pOfficeBrandDir->getLength(),
                                         m_aOfficeBrandDirMacro );
        }
        else
        {
            nIndex = aCanonicalURL.indexOf( *m_pUserDir );
            if ( nIndex  != -1 )
            {
                return
                    aCanonicalURL.replaceAt( nIndex,
                                             m_pUserDir->getLength(),
                                             m_aUserDirMacro );
            }
        }
    }
    return URL;
}


// virtual
OUString SAL_CALL
OfficeInstallationDirectories::makeAbsoluteURL( const OUString& URL )
{
    if ( !URL.isEmpty() )
    {
        sal_Int32 nIndex = URL.indexOf( m_aOfficeBrandDirMacro );
        if ( nIndex != -1 )
        {
            initDirs();

            return
                URL.replaceAt( nIndex,
                               m_aOfficeBrandDirMacro.getLength(),
                               *m_pOfficeBrandDir );
        }
        else
        {
            nIndex = URL.indexOf( m_aUserDirMacro );
            if ( nIndex != -1 )
            {
                initDirs();

                return
                    URL.replaceAt( nIndex,
                                   m_aUserDirMacro.getLength(),
                                   *m_pUserDir );
            }
        }
    }
    return URL;
}


// lang::XServiceInfo


// virtual
OUString SAL_CALL
OfficeInstallationDirectories::getImplementationName()
{
    return OUString("com.sun.star.comp.util.OfficeInstallationDirectories");
}

// virtual
sal_Bool SAL_CALL
OfficeInstallationDirectories::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService(this, ServiceName);
}

// virtual
uno::Sequence< OUString > SAL_CALL
OfficeInstallationDirectories::getSupportedServiceNames()
{
    return { "com.sun.star.util.OfficeInstallationDirectories" };
}

void OfficeInstallationDirectories::initDirs()
{
    if ( m_pOfficeBrandDir == nullptr )
    {
        osl::MutexGuard aGuard( m_aMutex );
        if ( m_pOfficeBrandDir == nullptr )
        {
            m_pOfficeBrandDir.reset( new OUString );
            m_pUserDir.reset( new OUString );

            uno::Reference< util::XMacroExpander > xExpander = util::theMacroExpander::get(m_xCtx);

            *m_pOfficeBrandDir = xExpander->expandMacros( "$BRAND_BASE_DIR" );

            OSL_ENSURE( !m_pOfficeBrandDir->isEmpty(),
                        "Unable to obtain office brand installation directory!" );

            makeCanonicalFileURL( *m_pOfficeBrandDir );

            *m_pUserDir =
                xExpander->expandMacros(
                    "${$BRAND_BASE_DIR/" LIBO_ETC_FOLDER "/" SAL_CONFIGFILE( "bootstrap" ) ":UserInstallation}" );

            OSL_ENSURE( !m_pUserDir->isEmpty(),
                        "Unable to obtain office user data directory!" );

            makeCanonicalFileURL( *m_pUserDir );
        }
    }
}

}

namespace {

struct Instance {
    explicit Instance(
        css::uno::Reference<css::uno::XComponentContext> const & context):
        instance(static_cast<cppu::OWeakObject *>(
            new comphelper::OfficeInstallationDirectories(context)))
    {}

    rtl::Reference<css::uno::XInterface> instance;
};

struct Singleton:
    public rtl::StaticWithArg<
        Instance, css::uno::Reference<css::uno::XComponentContext>, Singleton>
{};

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_comp_util_OfficeInstallationDirectories(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(static_cast<cppu::OWeakObject *>(
                Singleton::get(context).instance.get()));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
