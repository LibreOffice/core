/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: accresmgr.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2008-02-25 16:02:07 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_accessibility.hxx"

#ifndef ACCESSIBILITY_HELPER_TKARESMGR_HXX
#include <accessibility/helper/accresmgr.hxx>
#endif

#ifndef _TOOLS_SIMPLERESMGR_HXX
#include <tools/simplerm.hxx>
#endif

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif


// -----------------------------------------------------------------------------
// TkResMgr
// -----------------------------------------------------------------------------

SimpleResMgr* TkResMgr::m_pImpl = NULL;

// -----------------------------------------------------------------------------

TkResMgr::EnsureDelete::~EnsureDelete()
{
    delete TkResMgr::m_pImpl;
}

// -----------------------------------------------------------------------------

void TkResMgr::ensureImplExists()
{
    if (m_pImpl)
        return;

    ::com::sun::star::lang::Locale aLocale = Application::GetSettings().GetUILocale();

    ByteString sResMgrName( "tk" );

    m_pImpl = SimpleResMgr::Create( sResMgrName.GetBuffer(), aLocale );

    if (m_pImpl)
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
    if ( m_pImpl )
        sReturn = m_pImpl->ReadString( nResId );

    return sReturn;
}

// -----------------------------------------------------------------------------
