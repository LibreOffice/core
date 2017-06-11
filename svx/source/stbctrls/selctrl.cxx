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

#include <vcl/builder.hxx>
#include <vcl/menu.hxx>
#include <vcl/status.hxx>
#include <svl/intitem.hxx>
#include <sfx2/dispatch.hxx>
#include <tools/urlobj.hxx>

#include <svx/selctrl.hxx>
#include <svx/dialmgr.hxx>
#include <svx/strings.hrc>

#include "stbctrls.h"
#include "bitmaps.hlst"

SFX_IMPL_STATUSBAR_CONTROL(SvxSelectionModeControl, SfxUInt16Item);

/// Popup menu to select the selection type
class SelectionTypePopup
{
    VclBuilder        m_aBuilder;
    VclPtr<PopupMenu> m_xMenu;
    static sal_uInt16 id_to_state(const OString& rIdent);
    sal_uInt16 state_to_id(sal_uInt16 nState) const;
public:
    explicit SelectionTypePopup(sal_uInt16 nCurrent);
    OUString GetItemTextForState(sal_uInt16 nState) { return m_xMenu->GetItemText(state_to_id(nState)); }
    sal_uInt16 GetState() const { return id_to_state(m_xMenu->GetCurItemIdent()); }
    sal_uInt16 Execute(vcl::Window* pWindow, const Point& rPopupPos) { return m_xMenu->Execute(pWindow, rPopupPos); }
};

sal_uInt16 SelectionTypePopup::id_to_state(const OString& rIdent)
{
    if (rIdent == "block")
        return 3;
    else if (rIdent == "adding")
        return 2;
    else if (rIdent == "extending")
        return 1;
    else // fall through
        return 0;
}

sal_uInt16 SelectionTypePopup::state_to_id(sal_uInt16 nState) const
{
    switch (nState)
    {
        default: // fall through
        case 0: return m_xMenu->GetItemId("standard");
        case 1: return m_xMenu->GetItemId("extending");
        case 2: return m_xMenu->GetItemId("adding");
        case 3: return m_xMenu->GetItemId("block");
    }
}

SelectionTypePopup::SelectionTypePopup(sal_uInt16 nCurrent)
    : m_aBuilder(nullptr, VclBuilderContainer::getUIRootDir(), "svx/ui/selectionmenu.ui", "")
    , m_xMenu(m_aBuilder.get_menu("menu"))
{
    m_xMenu->CheckItem(state_to_id(nCurrent));
}

SvxSelectionModeControl::SvxSelectionModeControl( sal_uInt16 _nSlotId,
                                                  sal_uInt16 _nId,
                                                  StatusBar& rStb ) :
    SfxStatusBarControl( _nSlotId, _nId, rStb ),
    mnState( 0 ),
    maImage(BitmapEx(RID_SVXBMP_SELECTION))
{
    GetStatusBar().SetItemText( GetId(), "" );
}

void SvxSelectionModeControl::StateChanged( sal_uInt16, SfxItemState eState,
                                            const SfxPoolItem* pState )
{
    if ( SfxItemState::DEFAULT == eState )
    {
        DBG_ASSERT( dynamic_cast< const SfxUInt16Item* >(pState) !=  nullptr, "invalid item type" );
        const SfxUInt16Item* pItem = static_cast<const SfxUInt16Item*>(pState);
        mnState = pItem->GetValue();

        SelectionTypePopup aPop(mnState);
        GetStatusBar().SetQuickHelpText(GetId(), aPop.GetItemTextForState(mnState));
    }
}

bool SvxSelectionModeControl::MouseButtonDown( const MouseEvent& rEvt )
{
    SelectionTypePopup aPop(mnState);
    StatusBar& rStatusbar = GetStatusBar();

    if (aPop.Execute(&rStatusbar, rEvt.GetPosPixel()))
    {
        sal_uInt16 nNewState = aPop.GetState();
        if ( nNewState != mnState )
        {
            mnState = nNewState;

            css::uno::Any a;
            SfxUInt16Item aState( GetSlotId(), mnState );
            INetURLObject aObj( m_aCommandURL );

            css::uno::Sequence< css::beans::PropertyValue > aArgs( 1 );
            aArgs[0].Name  = aObj.GetURLPath();
            aState.QueryValue( a );
            aArgs[0].Value = a;

            execute( aArgs );
        }
    }

    return true;
}


void SvxSelectionModeControl::Paint( const UserDrawEvent& rUsrEvt )
{
    const tools::Rectangle aControlRect = getControlRect();
    vcl::RenderContext* pDev = rUsrEvt.GetRenderContext();
    tools::Rectangle aRect = rUsrEvt.GetRect();

    Size aImgSize( maImage.GetSizePixel() );

    Point aPos( aRect.Left() + ( aControlRect.GetWidth() - aImgSize.Width() ) / 2,
            aRect.Top() + ( aControlRect.GetHeight() - aImgSize.Height() ) / 2 );

    pDev->DrawImage( aPos, maImage );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
