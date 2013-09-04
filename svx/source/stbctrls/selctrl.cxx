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
    GetStatusBar().SetItemText( GetId(), "" );
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
