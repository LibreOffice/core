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
#include "precompiled_svx.hxx"

// include ---------------------------------------------------------------
#include <tools/pstm.hxx>
#include <svl/intitem.hxx>
#include <sfx2/objsh.hxx>

#include <svx/svxids.hrc>

#define _SVX_SRCHDLG_CXX // damit private-Methoden vom SrchDlg bekannt sind



#include "srchctrl.hxx"
#include "svx/srchdlg.hxx"
#include <svl/srchitem.hxx>

// class SvxSearchFamilyControllerItem -----------------------------------

SvxSearchController::SvxSearchController
(
    sal_uInt16 _nId,
    SfxBindings& rBind,
    SvxSearchDialog& rDlg
) :
    SfxControllerItem( _nId, rBind ),

    rSrchDlg( rDlg )
{
}

// -----------------------------------------------------------------------

void SvxSearchController::StateChanged( sal_uInt16 nSID, SfxItemState eState,
                                        const SfxPoolItem* pState )
{
    if ( SFX_ITEM_AVAILABLE == eState )
    {
        if ( SID_STYLE_FAMILY1 <= nSID && nSID <= SID_STYLE_FAMILY4 )
        {
            SfxObjectShell* pShell = SfxObjectShell::Current();

            if ( pShell && pShell->GetStyleSheetPool() )
                rSrchDlg.TemplatesChanged_Impl( *pShell->GetStyleSheetPool() );
        }
        else if ( SID_SEARCH_OPTIONS == nSID )
        {
            DBG_ASSERT( pState->ISA(SfxUInt16Item), "wrong item type" );
            sal_uInt16 nFlags = (sal_uInt16)( (SfxUInt16Item*)pState )->GetValue();
            rSrchDlg.EnableControls_Impl( nFlags );
        }
        else if ( SID_SEARCH_ITEM == nSID )
        {
            DBG_ASSERT( pState->ISA(SvxSearchItem), "wrong item type" );
            rSrchDlg.SetItem_Impl( (const SvxSearchItem*)pState );
        }
    }
    else if ( SID_SEARCH_OPTIONS == nSID || SID_SEARCH_ITEM == nSID )
        rSrchDlg.EnableControls_Impl( 0 );
}


