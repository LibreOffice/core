/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: tkresmgr.cxx,v $
 * $Revision: 1.8 $
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
#include "precompiled_toolkit.hxx"
#include <toolkit/helper/tkresmgr.hxx>
#include <tools/simplerm.hxx>
#ifndef  _TOOLS_RESMGR_HXX_
#include <tools/resmgr.hxx>
#endif


#include <vcl/svapp.hxx>


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

    ByteString sResMgrName( "tk" );

    m_pSimpleResMgr = SimpleResMgr::Create( sResMgrName.GetBuffer(), aLocale );
    m_pResMgr = ResMgr::CreateResMgr( sResMgrName.GetBuffer() );

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
