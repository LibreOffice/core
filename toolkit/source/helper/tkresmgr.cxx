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
