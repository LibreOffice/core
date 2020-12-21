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

#include <i18nutil/unicode.hxx>
#include <vcl/commandevent.hxx>
#include <vcl/event.hxx>
#include <vcl/svapp.hxx>
#include <vcl/status.hxx>
#include <vcl/weldutils.hxx>
#include <vcl/settings.hxx>
#include <tools/urlobj.hxx>
#include <sal/log.hxx>

#include <svx/strings.hrc>

#include <svx/zoomctrl.hxx>
#include <sfx2/zoomitem.hxx>
#include <svx/dialmgr.hxx>
#include "modctrl_internal.hxx"
#include <bitmaps.hlst>

#include <com/sun/star/beans/PropertyValue.hpp>

SFX_IMPL_STATUSBAR_CONTROL(SvxZoomStatusBarControl,SvxZoomItem);

namespace {

class ZoomPopup_Impl
{
public:
    ZoomPopup_Impl(weld::Window* pPopupParent, sal_uInt16 nZ, SvxZoomEnableFlags nValueSet);

    sal_uInt16 GetZoom(std::string_view ident);

    OString popup_at_rect(const tools::Rectangle& rRect)
    {
        return m_xMenu->popup_at_rect(m_pPopupParent, rRect);
    }

private:
    weld::Window* m_pPopupParent;
    std::unique_ptr<weld::Builder> m_xBuilder;
    std::unique_ptr<weld::Menu> m_xMenu;
    sal_uInt16 nZoom;
};

}

ZoomPopup_Impl::ZoomPopup_Impl(weld::Window* pPopupParent, sal_uInt16 nZ, SvxZoomEnableFlags nValueSet)
    : m_pPopupParent(pPopupParent)
    , m_xBuilder(Application::CreateBuilder(m_pPopupParent, "svx/ui/zoommenu.ui"))
    , m_xMenu(m_xBuilder->weld_menu("menu"))
    , nZoom(nZ)
{
    if ( !(SvxZoomEnableFlags::N50 & nValueSet) )
        m_xMenu->set_sensitive("50", false);
    if ( !(SvxZoomEnableFlags::N100 & nValueSet) )
        m_xMenu->set_sensitive("100", false);
    if ( !(SvxZoomEnableFlags::N150 & nValueSet) )
        m_xMenu->set_sensitive("150", false);
    if ( !(SvxZoomEnableFlags::N200 & nValueSet) )
        m_xMenu->set_sensitive("200", false);
    if ( !(SvxZoomEnableFlags::OPTIMAL & nValueSet) )
        m_xMenu->set_sensitive("optimal", false);
    if ( !(SvxZoomEnableFlags::WHOLEPAGE & nValueSet) )
        m_xMenu->set_sensitive("page", false);
    if ( !(SvxZoomEnableFlags::PAGEWIDTH & nValueSet) )
        m_xMenu->set_sensitive("width", false);
}

sal_uInt16 ZoomPopup_Impl::GetZoom(std::string_view ident)
{
    sal_uInt16 nRet = nZoom;

    if (ident == "200")
        nRet = 200;
    else if (ident == "150")
        nRet = 150;
    else if (ident == "100")
        nRet = 100;
    else if (ident == "75")
        nRet =  75;
    else if (ident == "50")
        nRet =  50;
    else if (ident == "optimal" || ident == "width" || ident == "page")
        nRet = 0;

    return nRet;
}

SvxZoomStatusBarControl::SvxZoomStatusBarControl( sal_uInt16 _nSlotId,
                                                  sal_uInt16 _nId,
                                                  StatusBar& rStb ) :

    SfxStatusBarControl( _nSlotId, _nId, rStb ),
    nZoom( 100 ),
    nValueSet( SvxZoomEnableFlags::ALL )
{
    GetStatusBar().SetQuickHelpText(GetId(), SvxResId(RID_SVXSTR_ZOOMTOOL_HINT));
    ImplUpdateItemText();
}

void SvxZoomStatusBarControl::StateChanged( sal_uInt16, SfxItemState eState,
                                            const SfxPoolItem* pState )
{
    if( SfxItemState::DEFAULT != eState )
    {
        GetStatusBar().SetItemText( GetId(), "" );
        nValueSet = SvxZoomEnableFlags::NONE;
    }
    else if ( auto pItem = dynamic_cast< const SfxUInt16Item* >(pState) )
    {
        nZoom = pItem->GetValue();
        ImplUpdateItemText();

        if ( auto pZoomItem = dynamic_cast<const SvxZoomItem*>(pState) )
        {
            nValueSet = pZoomItem->GetValueSet();
        }
        else
        {
            SAL_INFO( "svx", "use SfxZoomItem for SID_ATTR_ZOOM" );
            nValueSet = SvxZoomEnableFlags::ALL;
        }
    }
}

void SvxZoomStatusBarControl::ImplUpdateItemText()
{
    // workaround - don't bother updating when we don't have a real zoom value
    if (nZoom)
    {
        OUString aStr(unicode::formatPercent(nZoom, Application::GetSettings().GetUILanguageTag()));
        GetStatusBar().SetItemText( GetId(), aStr );
    }
}

void SvxZoomStatusBarControl::Paint( const UserDrawEvent& )
{
}

void SvxZoomStatusBarControl::Command( const CommandEvent& rCEvt )
{
    if ( CommandEventId::ContextMenu == rCEvt.GetCommand() && bool(nValueSet) )
    {
        ::tools::Rectangle aRect(rCEvt.GetMousePosPixel(), Size(1, 1));
        weld::Window* pPopupParent = weld::GetPopupParent(GetStatusBar(), aRect);
        ZoomPopup_Impl aPop(pPopupParent, nZoom, nValueSet);

        OString sIdent = aPop.popup_at_rect(aRect);
        if (!sIdent.isEmpty() && (nZoom != aPop.GetZoom(sIdent) || !nZoom))
        {
            nZoom = aPop.GetZoom(sIdent);
            ImplUpdateItemText();
            SvxZoomItem aZoom(SvxZoomType::PERCENT, nZoom, GetId());

            if (sIdent == "optimal")
                aZoom.SetType(SvxZoomType::OPTIMAL);
            else if (sIdent == "width")
                aZoom.SetType(SvxZoomType::PAGEWIDTH);
            else if (sIdent == "page")
                aZoom.SetType(SvxZoomType::WHOLEPAGE);

            css::uno::Any a;
            INetURLObject aObj( m_aCommandURL );

            css::uno::Sequence< css::beans::PropertyValue > aArgs( 1 );
            aArgs[0].Name  = aObj.GetURLPath();
            aZoom.QueryValue( a );
            aArgs[0].Value = a;

            execute( aArgs );
        }
    }
    else
        SfxStatusBarControl::Command( rCEvt );
}

SFX_IMPL_STATUSBAR_CONTROL(SvxZoomPageStatusBarControl,SfxVoidItem);

SvxZoomPageStatusBarControl::SvxZoomPageStatusBarControl(sal_uInt16 _nSlotId,
    sal_uInt16 _nId, StatusBar& rStb)
    : SfxStatusBarControl(_nSlotId, _nId, rStb)
    , maImage(StockImage::Yes, RID_SVXBMP_ZOOM_PAGE)
{
    GetStatusBar().SetQuickHelpText(GetId(), SvxResId(RID_SVXSTR_FIT_SLIDE));
}

void SvxZoomPageStatusBarControl::Paint(const UserDrawEvent& rUsrEvt)
{
    vcl::RenderContext* pDev = rUsrEvt.GetRenderContext();
    tools::Rectangle aRect = rUsrEvt.GetRect();
    Point aPt = centerImage(aRect, maImage);
    pDev->DrawImage(aPt, maImage);
}

bool SvxZoomPageStatusBarControl::MouseButtonDown(const MouseEvent&)
{
    SvxZoomItem aZoom( SvxZoomType::WHOLEPAGE, 0, GetId() );

    css::uno::Any a;
    INetURLObject aObj( m_aCommandURL );

    css::uno::Sequence< css::beans::PropertyValue > aArgs( 1 );
    aArgs[0].Name  = aObj.GetURLPath();
    aZoom.QueryValue( a );
    aArgs[0].Value = a;

    execute( aArgs );

    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
