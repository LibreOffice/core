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

#include <sal/config.h>

#include <string_view>

#include <vcl/event.hxx>
#include <vcl/status.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weldutils.hxx>
#include <svl/intitem.hxx>
#include <tools/urlobj.hxx>

#include <svx/selctrl.hxx>

#include <bitmaps.hlst>

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>

SFX_IMPL_STATUSBAR_CONTROL(SvxSelectionModeControl, SfxUInt16Item);

namespace {

/// Popup menu to select the selection type
class SelectionTypePopup
{
    weld::Window* m_pPopupParent;
    std::unique_ptr<weld::Builder> m_xBuilder;
    std::unique_ptr<weld::Menu> m_xMenu;
    static OString state_to_id(sal_uInt16 nState);
public:
    SelectionTypePopup(weld::Window* pPopupParent, sal_uInt16 nCurrent);
    OUString GetItemTextForState(sal_uInt16 nState) { return m_xMenu->get_label(state_to_id(nState)); }
    OString popup_at_rect(const tools::Rectangle& rRect)
    {
        return m_xMenu->popup_at_rect(m_pPopupParent, rRect);
    }
    void HideSelectionType(const OString& rIdent)
    {
        m_xMenu->remove(rIdent);
    }
    static sal_uInt16 id_to_state(std::string_view ident);
};

}

sal_uInt16 SelectionTypePopup::id_to_state(std::string_view ident)
{
    if (ident == "block")
        return 3;
    else if (ident == "adding")
        return 2;
    else if (ident == "extending")
        return 1;
    else // fall through
        return 0;
}

OString SelectionTypePopup::state_to_id(sal_uInt16 nState)
{
    switch (nState)
    {
        default: // fall through
        case 0: return "standard";
        case 1: return "extending";
        case 2: return "adding";
        case 3: return "block";
    }
}

SelectionTypePopup::SelectionTypePopup(weld::Window* pPopupParent, sal_uInt16 nCurrent)
    : m_pPopupParent(pPopupParent)
    , m_xBuilder(Application::CreateBuilder(m_pPopupParent, "svx/ui/selectionmenu.ui"))
    , m_xMenu(m_xBuilder->weld_menu("menu"))
{
    m_xMenu->set_active(state_to_id(nCurrent), true);
}

SvxSelectionModeControl::SvxSelectionModeControl( sal_uInt16 _nSlotId,
                                                  sal_uInt16 _nId,
                                                  StatusBar& rStb ) :
    SfxStatusBarControl( _nSlotId, _nId, rStb ),
    mnState( 0 ),
    maImage(StockImage::Yes, RID_SVXBMP_SELECTION)
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

        SelectionTypePopup aPop(GetStatusBar().GetFrameWeld(), mnState);
        GetStatusBar().SetQuickHelpText(GetId(), aPop.GetItemTextForState(mnState));
    }
}

bool SvxSelectionModeControl::MouseButtonDown( const MouseEvent& rEvt )
{
    if (!rEvt.IsMiddle())
        return false;

    ::tools::Rectangle aRect(rEvt.GetPosPixel(), Size(1, 1));
    weld::Window* pPopupParent = weld::GetPopupParent(GetStatusBar(), aRect);
    SelectionTypePopup aPop(pPopupParent, mnState);

    // Check if Calc is opened; if true, hide block selection mode tdf#122280
    const css::uno::Reference < css::frame::XModel > xModel = m_xFrame->getController()->getModel();
    css::uno::Reference< css::lang::XServiceInfo > xServices( xModel, css::uno::UNO_QUERY );
    if ( xServices.is() )
    {
        bool bSpecModeCalc = xServices->supportsService("com.sun.star.sheet.SpreadsheetDocument");
        if (bSpecModeCalc)
            aPop.HideSelectionType("block");
    }

    OString sIdent = aPop.popup_at_rect(aRect);
    if (!sIdent.isEmpty())
    {
        sal_uInt16 nNewState = SelectionTypePopup::id_to_state(sIdent);
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
