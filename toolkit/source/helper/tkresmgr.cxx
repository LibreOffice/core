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

#include <toolkit/helper/tkresmgr.hxx>
#include <tools/simplerm.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/componentcontext.hxx>
#include <comphelper/namedvaluecollection.hxx>
#include <com/sun/star/graphic/GraphicProvider.hpp>
#include <com/sun/star/graphic/XGraphicProvider.hpp>
#include <tools/resmgr.hxx>
#include <tools/diagnose_ex.h>

#include <vcl/svapp.hxx>

using ::com::sun::star::uno::Reference;
using ::com::sun::star::graphic::XGraphic;
using ::com::sun::star::graphic::XGraphicProvider;
using namespace ::com::sun::star;
// -----------------------------------------------------------------------------
// TkResMgr
// -----------------------------------------------------------------------------

SimpleResMgr*   TkResMgr::m_pSimpleResMgr = NULL;
ResMgr*         TkResMgr::m_pResMgr = NULL;

// -----------------------------------------------------------------------------

TkResMgr::EnsureDelete::~EnsureDelete()
{
    delete TkResMgr::m_pSimpleResMgr;
//    delete TkResMgr::m_pResMgr;
}

// -----------------------------------------------------------------------------

void TkResMgr::ensureImplExists()
{
    if (m_pSimpleResMgr)
        return;

    ::com::sun::star::lang::Locale aLocale = Application::GetSettings().GetUILocale();

    m_pSimpleResMgr = SimpleResMgr::Create( "tk", aLocale );
    m_pResMgr = ResMgr::CreateResMgr( "tk" );

    if (m_pSimpleResMgr)
    {
        // now that we have a impl class, make sure it's deleted on unloading the library
        static TkResMgr::EnsureDelete s_aDeleteTheImplClass;
    }
}

// -----------------------------------------------------------------------------
::rtl::OUString TkResMgr::loadString( sal_uInt16 nResId )
{
    ::rtl::OUString sReturn;

    ensureImplExists();
    if ( m_pSimpleResMgr )
        sReturn = m_pSimpleResMgr->ReadString( nResId );

    return sReturn;
}

// -----------------------------------------------------------------------------
Image TkResMgr::loadImage( sal_uInt16 nResId )
{
    Image aReturn;

    ensureImplExists();
    if ( m_pResMgr )
        aReturn = Image( ResId( nResId, *m_pResMgr ) );

    return aReturn;
}

// -----------------------------------------------------------------------------
Image TkResMgr::getImageFromURL( const ::rtl::OUString& i_rImageURL )
{
    if ( i_rImageURL.isEmpty() )
        return Image();

    try
    {
        Reference< uno::XComponentContext > xContext( ::comphelper::getProcessComponentContext() );
        Reference< XGraphicProvider > xProvider( graphic::GraphicProvider::create(xContext) );
        ::comphelper::NamedValueCollection aMediaProperties;
        aMediaProperties.put( "URL", i_rImageURL );
        Reference< XGraphic > xGraphic = xProvider->queryGraphic( aMediaProperties.getPropertyValues() );
        return Image( xGraphic );
    }
    catch( const uno::Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
    return Image();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
