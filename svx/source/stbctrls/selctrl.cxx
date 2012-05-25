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

#include <tools/shl.hxx>
#include <vcl/menu.hxx>
#include <vcl/status.hxx>
#include <svl/intitem.hxx>
#include <sfx2/dispatch.hxx>
#include <tools/urlobj.hxx>

#include <svx/selctrl.hxx>
#include <svx/dialmgr.hxx>
#include <svx/dialogs.hrc>

#include "stbctrls.h"

SFX_IMPL_STATUSBAR_CONTROL(SvxSelectionModeControl, SfxUInt16Item);

/// Popup menu to select the selection type
class SelectionTypePopup : public PopupMenu
{
public:
    SelectionTypePopup( sal_uInt16 nCurrent );
};

/// Item id's cannot start from 0, so we need to convert
static sal_uInt16 id_to_state( sal_uInt16 nId )
{
    switch ( nId )
    {
        default: // fall through
        case SELECTION_STANDARD: return 0;
        case SELECTION_EXTENDED: return 1;
        case SELECTION_ADDED:    return 2;
        case SELECTION_BLOCK:    return 3;
    }
}

/// Item id's cannot start from 0, so we need to convert
static sal_uInt16 state_to_id( sal_uInt16 nState )
{
    switch ( nState )
    {
        default: // fall through
        case 0: return SELECTION_STANDARD;
        case 1: return SELECTION_EXTENDED;
        case 2: return SELECTION_ADDED;
        case 3: return SELECTION_BLOCK;
    }
}

SelectionTypePopup::SelectionTypePopup( sal_uInt16 nCurrent )
    : PopupMenu( ResId( RID_SVXMENU_SELECTION, DIALOG_MGR() ) )
{
    CheckItem( state_to_id( nCurrent ), true );
}

// class SvxSelectionModeControl -----------------------------------------

SvxSelectionModeControl::SvxSelectionModeControl( sal_uInt16 _nSlotId,
                                                  sal_uInt16 _nId,
                                                  StatusBar& rStb ) :
    SfxStatusBarControl( _nSlotId, _nId, rStb ),
    mnState( 0 ),
    maImage( SVX_RES( RID_SVXBMP_SELECTION ) )
{
    GetStatusBar().SetItemText( GetId(), String() );
}

// -----------------------------------------------------------------------

void SvxSelectionModeControl::StateChanged( sal_uInt16, SfxItemState eState,
                                            const SfxPoolItem* pState )
{
    if ( SFX_ITEM_AVAILABLE == eState )
    {
        DBG_ASSERT( pState->ISA( SfxUInt16Item ), "invalid item type" );
        SfxUInt16Item* pItem = (SfxUInt16Item*)pState;
        mnState = pItem->GetValue();

        SelectionTypePopup aPop( mnState );
        GetStatusBar().SetQuickHelpText( GetId(), aPop.GetItemText( state_to_id( mnState ) ) );
    }
}

// -----------------------------------------------------------------------

sal_Bool SvxSelectionModeControl::MouseButtonDown( const MouseEvent& rEvt )
{
    CaptureMouse();
    SelectionTypePopup aPop( mnState );
    StatusBar& rStatusbar = GetStatusBar();

    if ( aPop.Execute( &rStatusbar, rEvt.GetPosPixel() ) )
    {
        sal_uInt16 nNewState = id_to_state( aPop.GetCurItemId() );
        if ( nNewState != mnState )
        {
            mnState = nNewState;

            ::com::sun::star::uno::Any a;
            SfxUInt16Item aState( GetSlotId(), mnState );
            INetURLObject aObj( m_aCommandURL );

            ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > aArgs( 1 );
            aArgs[0].Name  = aObj.GetURLPath();
            aState.QueryValue( a );
            aArgs[0].Value = a;

            execute( aArgs );
        }
    }
    ReleaseMouse();

    return sal_True;
}

// -----------------------------------------------------------------------

void SvxSelectionModeControl::Paint( const UserDrawEvent& rUsrEvt )
{
    const Rectangle aControlRect = getControlRect();
    OutputDevice* pDev = rUsrEvt.GetDevice();
    Rectangle aRect = rUsrEvt.GetRect();

    Size aImgSize( maImage.GetSizePixel() );

    Point aPos( aRect.Left() + ( aControlRect.GetWidth() - aImgSize.Width() ) / 2,
            aRect.Top() + ( aControlRect.GetHeight() - aImgSize.Height() ) / 2 );

    pDev->DrawImage( aPos, maImage );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
