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
#include <com/sun/star/util/theMacroExpander.hpp>
#include <comphelper/fileurl.hxx>
#include <utility>

#include "officeinstallationdirectories.hxx"

using namespace com::sun::star;

static bool makeCanonicalFileURL( OUString & rURL )
{
    OSL_ENSURE(comphelper::isFileUrl(rURL), "File URL expected!");

    OUString aNormalizedURL;
    if ( osl::FileBase::getAbsoluteFileURL( OUString(),
                                            rURL,
                                            aNormalizedURL )
            != osl::DirectoryItem::E_None )
        return false;

    osl::DirectoryItem aDirItem;
    if ( osl::DirectoryItem::get( aNormalizedURL, aDirItem )
            != osl::DirectoryItem::E_None )
        return false;

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
    return false;
}

namespace comphelper {

constexpr OUString g_aOfficeBrandDirMacro(u"$(brandbaseurl)"_ustr);
constexpr OUString g_aUserDirMacro(u"$(userdataurl)"_ustr);

OfficeInstallationDirectories::OfficeInstallationDirectories(
        uno::Reference< uno::XComponentContext > xCtx )
: m_xCtx(std::move( xCtx ))
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
    return *m_xOfficeBrandDir;
}


// virtual
OUString SAL_CALL
OfficeInstallationDirectories::getOfficeUserDataDirectoryURL()
{
    initDirs();
    return *m_xUserDir;
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

        sal_Int32 nIndex = aCanonicalURL.indexOf( *m_xOfficeBrandDir );
        if ( nIndex  != -1 )
        {
            return
                aCanonicalURL.replaceAt( nIndex,
                                         m_xOfficeBrandDir->getLength(),
                                         g_aOfficeBrandDirMacro );
        }
        else
        {
            nIndex = aCanonicalURL.indexOf( *m_xUserDir );
            if ( nIndex  != -1 )
            {
                return
                    aCanonicalURL.replaceAt( nIndex,
                                             m_xUserDir->getLength(),
                                             g_aUserDirMacro );
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
        sal_Int32 nIndex = URL.indexOf( g_aOfficeBrandDirMacro );
        if ( nIndex != -1 )
        {
            initDirs();

            return
                URL.replaceAt( nIndex,
                               g_aOfficeBrandDirMacro.getLength(),
                               *m_xOfficeBrandDir );
        }
        else
        {
            nIndex = URL.indexOf( g_aUserDirMacro );
            if ( nIndex != -1 )
            {
                initDirs();

                return
                    URL.replaceAt( nIndex,
                                   g_aUserDirMacro.getLength(),
                                   *m_xUserDir );
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
    return u"com.sun.star.comp.util.OfficeInstallationDirectories"_ustr;
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
    return { u"com.sun.star.util.OfficeInstallationDirectories"_ustr };
}

void OfficeInstallationDirectories::initDirs()
{
    if ( m_xOfficeBrandDir)
        return;

    std::unique_lock aGuard( m_aMutex );
    if ( m_xOfficeBrandDir )
        return;

    uno::Reference< util::XMacroExpander > xExpander = util::theMacroExpander::get(m_xCtx);

    m_xOfficeBrandDir = xExpander->expandMacros( u"$BRAND_BASE_DIR"_ustr );

    OSL_ENSURE( !m_xOfficeBrandDir->isEmpty(),
                "Unable to obtain office brand installation directory!" );

    makeCanonicalFileURL( *m_xOfficeBrandDir );

    m_xUserDir =
        xExpander->expandMacros(
            u"${$BRAND_BASE_DIR/" LIBO_ETC_FOLDER "/" SAL_CONFIGFILE( "bootstrap" ) ":UserInstallation}"_ustr );

    OSL_ENSURE( !m_xUserDir->isEmpty(),
                "Unable to obtain office user data directory!" );

    makeCanonicalFileURL( *m_xUserDir );
}

}


extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_comp_util_OfficeInstallationDirectories(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(
            new comphelper::OfficeInstallationDirectories(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
