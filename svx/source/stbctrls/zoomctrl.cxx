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
#include <vcl/svapp.hxx>
#include <vcl/status.hxx>
#include <vcl/menu.hxx>
#include <vcl/settings.hxx>
#include <sfx2/dispatch.hxx>
#include <tools/urlobj.hxx>

#include <svx/dialogs.hrc>

#include <svx/zoomctrl.hxx>
#include <svx/zoomslideritem.hxx>
#include <sfx2/zoomitem.hxx>
#include "stbctrls.h"
#include <svx/dialmgr.hxx>
#include "modctrl_internal.hxx"

SFX_IMPL_STATUSBAR_CONTROL(SvxZoomStatusBarControl,SvxZoomItem);

class ZoomPopup_Impl : public PopupMenu
{
public:
    ZoomPopup_Impl( sal_uInt16 nZ, SvxZoomEnableFlags nValueSet );

    sal_uInt16          GetZoom() const { return nZoom; }
    sal_uInt16          GetCurId() const { return nCurId; }

private:
    sal_uInt16          nZoom;
    sal_uInt16          nCurId;

    virtual void    Select() override;
};



ZoomPopup_Impl::ZoomPopup_Impl( sal_uInt16 nZ, SvxZoomEnableFlags nValueSet )
    : PopupMenu(ResId(RID_SVXMNU_ZOOM, DIALOG_MGR()))
    , nZoom(nZ)
    , nCurId(0)
{
    if ( !(SvxZoomEnableFlags::N50 & nValueSet) )
            EnableItem( ZOOM_50, false );
    if ( !(SvxZoomEnableFlags::N100 & nValueSet) )
            EnableItem( ZOOM_100, false );
    if ( !(SvxZoomEnableFlags::N150 & nValueSet) )
            EnableItem( ZOOM_150, false );
    if ( !(SvxZoomEnableFlags::N200 & nValueSet) )
            EnableItem( ZOOM_200, false );
    if ( !(SvxZoomEnableFlags::OPTIMAL & nValueSet) )
            EnableItem( ZOOM_OPTIMAL, false );
    if ( !(SvxZoomEnableFlags::WHOLEPAGE & nValueSet) )
            EnableItem( ZOOM_WHOLE_PAGE, false );
    if ( !(SvxZoomEnableFlags::PAGEWIDTH & nValueSet) )
            EnableItem( ZOOM_PAGE_WIDTH, false );
}



void ZoomPopup_Impl::Select()
{
    nCurId = GetCurItemId();

    switch ( nCurId )
    {
        case ZOOM_200:          nZoom = 200; break;
        case ZOOM_150:          nZoom = 150; break;
        case ZOOM_100:          nZoom = 100; break;
        case ZOOM_75:           nZoom =  75; break;
        case ZOOM_50:           nZoom =  50; break;

        case ZOOM_OPTIMAL:
        case ZOOM_PAGE_WIDTH:
        case ZOOM_WHOLE_PAGE:   nZoom = 0; break;

    }
}

SvxZoomStatusBarControl::SvxZoomStatusBarControl( sal_uInt16 _nSlotId,
                                                  sal_uInt16 _nId,
                                                  StatusBar& rStb ) :

    SfxStatusBarControl( _nSlotId, _nId, rStb ),
    nZoom( 100 ),
    nValueSet( SvxZoomEnableFlags::ALL )
{
    GetStatusBar().SetQuickHelpText(GetId(), SVX_RESSTR(RID_SVXSTR_ZOOMTOOL_HINT));
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
        ZoomPopup_Impl aPop( nZoom, nValueSet );
        StatusBar& rStatusbar = GetStatusBar();

        if ( aPop.Execute( &rStatusbar, rCEvt.GetMousePosPixel() ) && ( nZoom != aPop.GetZoom() || !nZoom ) )
        {
            nZoom = aPop.GetZoom();
            SvxZoomItem aZoom( SvxZoomType::PERCENT, nZoom, GetId() );

            switch( aPop.GetCurId() )
            {
            case ZOOM_OPTIMAL:      aZoom.SetType( SvxZoomType::OPTIMAL ); break;
            case ZOOM_PAGE_WIDTH:   aZoom.SetType( SvxZoomType::PAGEWIDTH ); break;
            case ZOOM_WHOLE_PAGE:   aZoom.SetType( SvxZoomType::WHOLEPAGE ); break;
            }

            ::com::sun::star::uno::Any a;
            INetURLObject aObj( m_aCommandURL );

            ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > aArgs( 1 );
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
    , maImage(SVX_RES(RID_SVXBMP_ZOOM_PAGE))
{
    GetStatusBar().SetQuickHelpText(GetId(), SVX_RESSTR(RID_SVXSTR_FIT_SLIDE));
}

void SvxZoomPageStatusBarControl::Paint(const UserDrawEvent& rUsrEvt)
{
    vcl::RenderContext* pDev = rUsrEvt.GetRenderContext();
    Rectangle aRect = rUsrEvt.GetRect();
    Point aPt = centerImage(aRect, maImage);
    pDev->DrawImage(aPt, maImage);
}

bool SvxZoomPageStatusBarControl::MouseButtonDown(const MouseEvent&)
{
    SvxZoomItem aZoom( SvxZoomType::WHOLEPAGE, 0, GetId() );

    ::com::sun::star::uno::Any a;
    INetURLObject aObj( m_aCommandURL );

    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > aArgs( 1 );
    aArgs[0].Name  = aObj.GetURLPath();
    aZoom.QueryValue( a );
    aArgs[0].Value = a;

    execute( aArgs );

    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
