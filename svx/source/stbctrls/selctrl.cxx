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
#include "precompiled_svx.hxx"

// include ---------------------------------------------------------------
#include <tools/shl.hxx>
#include <vcl/status.hxx>
#include <svl/intitem.hxx>
#include <sfx2/dispatch.hxx>
#include <tools/urlobj.hxx>

#define _SVX_SELCTRL_CXX

#include "selctrl.hxx"
#include <svx/dialmgr.hxx>

#include <svx/dialogs.hrc>

#define PAINT_OFFSET    5

SFX_IMPL_STATUSBAR_CONTROL(SvxSelectionModeControl, SfxUInt16Item);

// class SvxSelectionModeControl -----------------------------------------

SvxSelectionModeControl::SvxSelectionModeControl( USHORT _nSlotId,
                                                  USHORT _nId,
                                                  StatusBar& rStb ) :
    SfxStatusBarControl( _nSlotId, _nId, rStb ),
    nState( 0 )
{
}

// -----------------------------------------------------------------------

void SvxSelectionModeControl::StateChanged( USHORT, SfxItemState eState,
                                            const SfxPoolItem* pState )
{
    if ( SFX_ITEM_AVAILABLE != eState )
        GetStatusBar().SetItemText( GetId(), String() );
    else
    {
        DBG_ASSERT( pState->ISA( SfxUInt16Item ), "invalid item type" );
        SfxUInt16Item* pItem = (SfxUInt16Item*)pState;
        nState = pItem->GetValue();
        DrawItemText_Impl();
    }
}

// -----------------------------------------------------------------------

void SvxSelectionModeControl::Click()
{
    if ( !GetStatusBar().GetItemText( GetId() ).Len() )
        return;
    nState++;
    if ( nState > 3 )
        nState = 0;

    ::com::sun::star::uno::Any a;
    SfxUInt16Item aState( GetSlotId(), nState );
    INetURLObject aObj( m_aCommandURL );

    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > aArgs( 1 );
    aArgs[0].Name  = aObj.GetURLPath();
    aState.QueryValue( a );
    aArgs[0].Value = a;

    execute( aArgs );
}

// -----------------------------------------------------------------------

void SvxSelectionModeControl::Paint( const UserDrawEvent& )
{
    DrawItemText_Impl();
}

// -----------------------------------------------------------------------

void SvxSelectionModeControl::DrawItemText_Impl()
{
    String sTxt;
    USHORT _nId = 0;

    switch ( nState )
    {
        case 0:
            _nId = RID_SVXSTR_SELMODE_STD;
            break;
        case 1:
            _nId = RID_SVXSTR_SELMODE_ER;
            break;
        case 2:
            _nId = RID_SVXSTR_SELMODE_ERG;
            break;
        case 3:
            _nId = RID_SVXSTR_SELMODE_BLK;
            break;
        default: OSL_FAIL( "invalid selection mode!" );
    }

    if ( _nId )
        sTxt = SVX_RESSTR( _nId );
    GetStatusBar().SetItemText( GetId(), sTxt );
}

ULONG SvxSelectionModeControl::GetDefItemWidth(const StatusBar& rStb)
{
    long nWidth1 =  rStb.GetTextWidth(SVX_RESSTR(RID_SVXSTR_SELMODE_STD));
    long nWidth2 =  rStb.GetTextWidth(SVX_RESSTR(RID_SVXSTR_SELMODE_ER));
    long nWidth3 =  rStb.GetTextWidth(SVX_RESSTR(RID_SVXSTR_SELMODE_ERG));
    long nWidth4 =  rStb.GetTextWidth(SVX_RESSTR(RID_SVXSTR_SELMODE_BLK));

    if(nWidth1<nWidth2)
        nWidth1=nWidth2;

    if(nWidth1<nWidth3)
        nWidth1=nWidth3;

    if(nWidth1<nWidth4)
        nWidth1=nWidth4;

    return nWidth1+PAINT_OFFSET;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
