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

#include <svl/solar.hrc>
#include <svx/svxids.hrc>

#include "idetemp.hxx"
#include "tbxctl.hxx"

#include <sfx2/dispatch.hxx>
#include <sfx2/imagemgr.hxx>
#include <sfx2/viewfrm.hxx>
#include <svl/aeitem.hxx>
#include <vcl/toolbox.hxx>

namespace basctl
{

using namespace ::com::sun::star::uno;

namespace
{
    OUString aSubToolBarResName( "private:resource/toolbar/insertcontrolsbar" );
}

SFX_IMPL_TOOLBOX_CONTROL( TbxControls, SfxAllEnumItem )

/*************************************************************************
|*
|* Toolbox Class
|*
\************************************************************************/

TbxControls::TbxControls( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx ) :
        SfxToolBoxControl( nSlotId, nId, rTbx )
{
    nLastSlot = USHRT_MAX;

    rTbx.SetItemBits( nId, TIB_DROPDOWN | rTbx.GetItemBits( nId ) );
    rTbx.Invalidate();
}

/*************************************************************************
|*
|* If you want to create a pop-up window
|*
\************************************************************************/
SfxPopupWindowType TbxControls::GetPopupWindowType() const
{
    if( nLastSlot == USHRT_MAX )
        return(SFX_POPUPWINDOW_ONCLICK);
    return(SFX_POPUPWINDOW_ONTIMEOUT);
}

void TbxControls::StateChanged( sal_uInt16 nSID, SfxItemState eState,
  const SfxPoolItem* pState )
{
    if( pState )
    {
        if (SfxAllEnumItem const* pItem = dynamic_cast<SfxAllEnumItem const*>(pState))
        {
            sal_uInt16 nTemp = 0;
            sal_uInt16 nLastEnum = pItem->GetValue();
            switch (nLastEnum)
            {
                case SVX_SNAP_PUSHBUTTON:       nTemp = SID_INSERT_PUSHBUTTON; break;
                case SVX_SNAP_CHECKBOX:         nTemp = SID_INSERT_CHECKBOX; break;
                case SVX_SNAP_RADIOBUTTON:      nTemp = SID_INSERT_RADIOBUTTON; break;
                case SVX_SNAP_SPINBUTTON:       nTemp = SID_INSERT_SPINBUTTON; break;
                case SVX_SNAP_FIXEDTEXT:        nTemp = SID_INSERT_FIXEDTEXT; break;
                case SVX_SNAP_GROUPBOX:         nTemp = SID_INSERT_GROUPBOX; break;
                case SVX_SNAP_LISTBOX:          nTemp = SID_INSERT_LISTBOX; break;
                case SVX_SNAP_COMBOBOX:         nTemp = SID_INSERT_COMBOBOX; break;
                case SVX_SNAP_EDIT:             nTemp = SID_INSERT_EDIT; break;
                case SVX_SNAP_HSCROLLBAR:       nTemp = SID_INSERT_HSCROLLBAR; break;
                case SVX_SNAP_VSCROLLBAR:       nTemp = SID_INSERT_VSCROLLBAR; break;
                case SVX_SNAP_PREVIEW:          nTemp = SID_INSERT_PREVIEW; break;
                case SVX_SNAP_URLBUTTON:        nTemp = SID_INSERT_URLBUTTON; break;
                case SVX_SNAP_IMAGECONTROL:     nTemp = SID_INSERT_IMAGECONTROL; break;
                case SVX_SNAP_PROGRESSBAR:      nTemp = SID_INSERT_PROGRESSBAR; break;
                case SVX_SNAP_HFIXEDLINE:       nTemp = SID_INSERT_HFIXEDLINE; break;
                case SVX_SNAP_VFIXEDLINE:       nTemp = SID_INSERT_VFIXEDLINE; break;
                case SVX_SNAP_DATEFIELD:        nTemp = SID_INSERT_DATEFIELD; break;
                case SVX_SNAP_TIMEFIELD:        nTemp = SID_INSERT_TIMEFIELD; break;
                case SVX_SNAP_NUMERICFIELD:     nTemp = SID_INSERT_NUMERICFIELD; break;
                case SVX_SNAP_CURRENCYFIELD:    nTemp = SID_INSERT_CURRENCYFIELD; break;
                case SVX_SNAP_FORMATTEDFIELD:   nTemp = SID_INSERT_FORMATTEDFIELD; break;
                case SVX_SNAP_PATTERNFIELD:     nTemp = SID_INSERT_PATTERNFIELD; break;
                case SVX_SNAP_FILECONTROL:      nTemp = SID_INSERT_FILECONTROL; break;
                case SVX_SNAP_TREECONTROL:      nTemp = SID_INSERT_TREECONTROL; break;
            }
            if( nTemp )
            {
                OUString aSlotURL( "slot:" );
                aSlotURL += OUString::valueOf( sal_Int32( nTemp ));
                Image aImage = GetImage( m_xFrame,
                                         aSlotURL,
                                         hasBigImages()
                                       );
                ToolBox& rBox = GetToolBox();
                rBox.SetItemImage(GetId(), aImage);
                nLastSlot = nLastEnum;
            }
        }
    }
    SfxToolBoxControl::StateChanged( nSID, eState,pState );
}

void TbxControls::Select( sal_uInt16 nModifier )
{
    (void)nModifier;
    SfxAllEnumItem aItem( SID_CHOOSE_CONTROLS, nLastSlot );
    SfxViewFrame* pCurFrame = SfxViewFrame::Current();
    DBG_ASSERT( pCurFrame != NULL, "No current view frame!" );
    SfxDispatcher* pDispatcher = pCurFrame ? pCurFrame->GetDispatcher() : NULL;
    if( pDispatcher )
    {
        pDispatcher->Execute( SID_CHOOSE_CONTROLS, SFX_CALLMODE_SYNCHRON, &aItem, 0L );
    }
}

/*************************************************************************
|*
|* the window is created here
|* position of the toolbox can be requested with GetToolBox()
|* rItemRect are the screen coordinates
|*
\************************************************************************/
SfxPopupWindow* TbxControls::CreatePopupWindow()
{
    if ( GetSlotId() == SID_CHOOSE_CONTROLS )
        createAndPositionSubToolBar( aSubToolBarResName );

    return(0);
}


} // namespace basctl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
