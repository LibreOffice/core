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
#ifndef _STATUS_HXX //autogen
#include <vcl/status.hxx>
#endif
#include <svl/eitem.hxx>
#include <sfx2/app.hxx>
#include <sfx2/dispatch.hxx>

#define _SVX_INSCTRL_CXX

#include <svx/dialogs.hrc>

#include "svx/insctrl.hxx"
#include <svx/dialmgr.hxx>

#define PAINT_OFFSET    5

SFX_IMPL_STATUSBAR_CONTROL(SvxInsertStatusBarControl, SfxBoolItem);

// class SvxInsertStatusBarControl ---------------------------------------

SvxInsertStatusBarControl::SvxInsertStatusBarControl( sal_uInt16 _nSlotId,
                                                      sal_uInt16 _nId,
                                                      StatusBar& rStb ) :

    SfxStatusBarControl( _nSlotId, _nId, rStb ),
    bInsert( sal_True )
{
}

// -----------------------------------------------------------------------

SvxInsertStatusBarControl::~SvxInsertStatusBarControl()
{
}

// -----------------------------------------------------------------------

void SvxInsertStatusBarControl::StateChanged( sal_uInt16 , SfxItemState eState,
                                              const SfxPoolItem* pState )
{
    if ( SFX_ITEM_AVAILABLE != eState )
        GetStatusBar().SetItemText( GetId(), String() );
    else
    {
        DBG_ASSERT( pState->ISA( SfxBoolItem ), "invalid item type" );
        SfxBoolItem* pItem = (SfxBoolItem*)pState;
        bInsert = pItem->GetValue();
        DrawItemText_Impl();
    }
}

// -----------------------------------------------------------------------

void SvxInsertStatusBarControl::Click()
{
    if ( !GetStatusBar().GetItemText( GetId() ).Len() )
        return;
    bInsert = !bInsert;
    SfxBoolItem aIns( GetSlotId(), bInsert );

    ::com::sun::star::uno::Any a;
    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > aArgs( 1 );
    aArgs[0].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "InsertMode" ));
    aIns.QueryValue( a );
    aArgs[0].Value = a;

    execute( aArgs );
}

// -----------------------------------------------------------------------

void SvxInsertStatusBarControl::Paint( const UserDrawEvent& )
{
    DrawItemText_Impl();
}

// -----------------------------------------------------------------------

void SvxInsertStatusBarControl::DrawItemText_Impl()
{
    sal_uInt16 _nId = RID_SVXSTR_OVERWRITE_TEXT;

    if ( bInsert )
        _nId = RID_SVXSTR_INSERT_TEXT;
    GetStatusBar().SetItemText( GetId(), SVX_RESSTR( _nId ) );
}

sal_uIntPtr SvxInsertStatusBarControl::GetDefItemWidth(const StatusBar& rStb)
{
    long nWidth1 =  rStb.GetTextWidth(SVX_RESSTR(RID_SVXSTR_OVERWRITE_TEXT));
    long nWidth2 =  rStb.GetTextWidth(SVX_RESSTR(RID_SVXSTR_INSERT_TEXT));

    if(nWidth1<nWidth2)
        nWidth1=nWidth2;

    return nWidth1+PAINT_OFFSET;
}


