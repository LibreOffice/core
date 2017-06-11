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
#include <vcl/builder.hxx>
#include <vcl/svapp.hxx>
#include <vcl/status.hxx>
#include <vcl/menu.hxx>
#include <vcl/settings.hxx>
#include <sfx2/dispatch.hxx>
#include <tools/urlobj.hxx>

#include <svx/strings.hrc>

#include <svx/zoomctrl.hxx>
#include <svx/zoomslideritem.hxx>
#include <sfx2/zoomitem.hxx>
#include "stbctrls.h"
#include <svx/dialmgr.hxx>
#include "modctrl_internal.hxx"
#include "bitmaps.hlst"

SFX_IMPL_STATUSBAR_CONTROL(SvxZoomStatusBarControl,SvxZoomItem);

class ZoomPopup_Impl
{
public:
    ZoomPopup_Impl( sal_uInt16 nZ, SvxZoomEnableFlags nValueSet );

    sal_uInt16 GetZoom();
    OString    GetCurItemIdent() const { return m_xMenu->GetCurItemIdent(); }

    sal_uInt16 Execute(vcl::Window* pWindow, const Point& rPopupPos)
    {
        return m_xMenu->Execute(pWindow, rPopupPos);
    }

private:
    VclBuilder          m_aBuilder;
    VclPtr<PopupMenu>   m_xMenu;
    sal_uInt16          nZoom;
};

ZoomPopup_Impl::ZoomPopup_Impl( sal_uInt16 nZ, SvxZoomEnableFlags nValueSet )
    : m_aBuilder(nullptr, VclBuilderContainer::getUIRootDir(), "svx/ui/zoommenu.ui", "")
    , m_xMenu(m_aBuilder.get_menu("menu"))
    , nZoom(nZ)
{
    if ( !(SvxZoomEnableFlags::N50 & nValueSet) )
        m_xMenu->EnableItem("50", false);
    if ( !(SvxZoomEnableFlags::N100 & nValueSet) )
        m_xMenu->EnableItem("100", false);
    if ( !(SvxZoomEnableFlags::N150 & nValueSet) )
        m_xMenu->EnableItem("150", false);
    if ( !(SvxZoomEnableFlags::N200 & nValueSet) )
        m_xMenu->EnableItem("200", false);
    if ( !(SvxZoomEnableFlags::OPTIMAL & nValueSet) )
        m_xMenu->EnableItem("optimal", false);
    if ( !(SvxZoomEnableFlags::WHOLEPAGE & nValueSet) )
        m_xMenu->EnableItem("page", false);
    if ( !(SvxZoomEnableFlags::PAGEWIDTH & nValueSet) )
        m_xMenu->EnableItem("width", false);
}

sal_uInt16 ZoomPopup_Impl::GetZoom()
{
    OString sIdent = GetCurItemIdent();
    if (sIdent == "200")
        nZoom = 200;
    else if (sIdent == "150")
        nZoom = 150;
    else if (sIdent == "100")
        nZoom = 100;
    else if (sIdent == "75")
        nZoom =  75;
    else if (sIdent == "50")
        nZoom =  50;
    else if (sIdent == "optimal" || sIdent == "width" || sIdent == "page")
        nZoom = 0;

    return nZoom;
}

SvxZoomStatusBarControl::SvxZoomStatusBarControl( sal_uInt16 _nSlotId,
                                                  sal_uInt16 _nId,
                                                  StatusBar& rStb ) :

    SfxStatusBarControl( _nSlotId, _nId, rStb ),
    nZoom( 100 ),
    nValueSet( SvxZoomEnableFlags::ALL )
{
    GetStatusBar().SetQuickHelpText(GetId(), SvxResId(RID_SVXSTR_ZOOMTOOL_HINT));
}

void SvxZoomStatusBarControl::StateChanged( sal_uInt16, SfxItemState eState,
                                            const SfxPoolItem* pState )
{
    if( SfxItemState::DEFAULT != eState )
    {
        GetStatusBar().SetItemText( GetId(), "" );
        nValueSet = SvxZoomEnableFlags::NONE;
    }
    else if ( dynamic_cast< const SfxUInt16Item* >(pState) !=  nullptr )
    {
        const SfxUInt16Item* pItem = static_cast<const SfxUInt16Item*>(pState);
        nZoom = pItem->GetValue();

        OUString aStr(unicode::formatPercent(nZoom, Application::GetSettings().GetUILanguageTag()));
        GetStatusBar().SetItemText( GetId(), aStr );

        if ( dynamic_cast<const SvxZoomItem*>( pState) !=  nullptr )
        {
            nValueSet = static_cast<const SvxZoomItem*>(pState)->GetValueSet();
        }
        else
        {
            SAL_INFO( "svx", "use SfxZoomItem for SID_ATTR_ZOOM" );
            nValueSet = SvxZoomEnableFlags::ALL;
        }
    }
}

void SvxZoomStatusBarControl::Paint( const UserDrawEvent& )
{
    OUString aStr(unicode::formatPercent(nZoom, Application::GetSettings().GetUILanguageTag()));
    GetStatusBar().SetItemText( GetId(), aStr );
}

void SvxZoomStatusBarControl::Command( const CommandEvent& rCEvt )
{
    if ( CommandEventId::ContextMenu == rCEvt.GetCommand() && bool(nValueSet) )
    {
        ZoomPopup_Impl aPop(nZoom, nValueSet);
        StatusBar& rStatusbar = GetStatusBar();

        if (aPop.Execute(&rStatusbar, rCEvt.GetMousePosPixel()) && (nZoom != aPop.GetZoom() || !nZoom))
        {
            nZoom = aPop.GetZoom();
            SvxZoomItem aZoom(SvxZoomType::PERCENT, nZoom, GetId());

            OString sIdent = aPop.GetCurItemIdent();
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
    , maImage(BitmapEx(RID_SVXBMP_ZOOM_PAGE))
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
