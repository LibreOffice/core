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

#include <config_wasm_strip.h>
#include <ChartWindow.hxx>
#include <ChartController.hxx>
#include <helpids.h>
#include <uiobject.hxx>

#include <vcl/help.hxx>
#include <vcl/settings.hxx>

#include <sfx2/ipclient.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/lokhelper.hxx>
#include <comphelper/lok.hxx>

using namespace ::com::sun::star;

namespace
{
::tools::Rectangle lcl_AWTRectToVCLRect( const css::awt::Rectangle & rAWTRect )
{
    ::tools::Rectangle aResult(rAWTRect.X, rAWTRect.Y);
    aResult.setWidth( rAWTRect.Width );
    aResult.setHeight( rAWTRect.Height );
    return aResult;
}
} // anonymous namespace

namespace chart
{

ChartWindow::ChartWindow( ChartController* pController, vcl::Window* pParent, WinBits nStyle )
        : Window(pParent, nStyle)
        , m_pWindowController( pController )
        , m_bInPaint(false)
        , m_pViewShellWindow( nullptr )
{
    set_id(u"chart_window"_ustr);
    SetHelpId( HID_SCH_WIN_DOCUMENT );
    SetMapMode( MapMode(MapUnit::Map100thMM) );
    adjustHighContrastMode();
    // chart does not depend on exact pixel painting => enable antialiased drawing
    GetOutDev()->SetAntialiasing( AntialiasingFlags::Enable | GetOutDev()->GetAntialiasing() );
    EnableRTL( false );
    if( pParent )
        pParent->EnableRTL( false );// #i96215# necessary for a correct position of the context menu in rtl mode
}

ChartWindow::~ChartWindow()
{
    disposeOnce();
}

void ChartWindow::dispose()
{
    m_pWindowController = nullptr;
    m_pViewShellWindow.clear();
    vcl::Window::dispose();
}

void ChartWindow::PrePaint(vcl::RenderContext& )
{
    // forward VCLs PrePaint window event to DrawingLayer
    if (m_pWindowController)
    {
       m_pWindowController->PrePaint();
    }
}

void ChartWindow::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect)
{
    if (comphelper::LibreOfficeKit::isActive() && !rRenderContext.IsVirtual())
        return;

    m_bInPaint = true;
    if (m_pWindowController)
    {
        m_pWindowController->execute_Paint(rRenderContext, rRect);
    }
    else
    {
        Window::Paint(rRenderContext, rRect);
    }
    m_bInPaint = false;
}

void ChartWindow::MouseButtonDown(const MouseEvent& rMEvt)
{
    if( m_pWindowController )
        m_pWindowController->execute_MouseButtonDown(rMEvt);
    else
        Window::MouseButtonDown(rMEvt);
}

void ChartWindow::MouseMove( const MouseEvent& rMEvt )
{
    if( m_pWindowController )
        m_pWindowController->execute_MouseMove( rMEvt );
    else
        Window::MouseMove( rMEvt );
}

void ChartWindow::Tracking( const TrackingEvent& rTEvt )
{
    if( !m_pWindowController )
        Window::Tracking( rTEvt );
}

void ChartWindow::MouseButtonUp( const MouseEvent& rMEvt )
{
    if( m_pWindowController )
        m_pWindowController->execute_MouseButtonUp( rMEvt );
    else
        Window::MouseButtonUp( rMEvt );
}

void ChartWindow::Resize()
{
    if( m_pWindowController )
        m_pWindowController->execute_Resize();
    else
        Window::Resize();
}

void ChartWindow::Activate()
{
    if( !m_pWindowController )
        Window::Activate();
}
void ChartWindow::Deactivate()
{
    if( !m_pWindowController )
        Window::Deactivate();
}
void ChartWindow::GetFocus()
{
    if( !m_pWindowController )
        Window::GetFocus();
}
void ChartWindow::LoseFocus()
{
    if( !m_pWindowController )
        Window::LoseFocus();
}

void ChartWindow::Command( const CommandEvent& rCEvt )
{
    if( m_pWindowController )
        m_pWindowController->execute_Command( rCEvt );
    else
        Window::Command( rCEvt );
}

void ChartWindow::KeyInput( const KeyEvent& rKEvt )
{
    if( m_pWindowController )
    {
        if( !m_pWindowController->execute_KeyInput(rKEvt) )
            Window::KeyInput(rKEvt);
    }
    else
        Window::KeyInput( rKEvt );
}

uno::Reference< css::accessibility::XAccessible > ChartWindow::CreateAccessible()
{
#if !ENABLE_WASM_STRIP_ACCESSIBILITY
    if( m_pWindowController )
        return m_pWindowController->CreateAccessible();
    else
        return Window::CreateAccessible();
#else
    return uno::Reference< css::accessibility::XAccessible >();
#endif
}

void ChartWindow::DataChanged( const DataChangedEvent& rDCEvt )
{
    vcl::Window::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DataChangedEventType::SETTINGS) &&
         (rDCEvt.GetFlags() & AllSettingsFlags::STYLE) )
    {
        adjustHighContrastMode();
    }
}

void ChartWindow::RequestHelp( const HelpEvent& rHEvt )
{
    bool bHelpHandled = false;
    if( ( rHEvt.GetMode() & HelpEventMode::QUICK ) &&
        m_pWindowController )
    {
        Point aLogicHitPos = PixelToLogic( GetPointerPosPixel());
        OUString aQuickHelpText;
        awt::Rectangle aHelpRect;
        bool bIsBalloonHelp( Help::IsBalloonHelpEnabled() );
        bHelpHandled = m_pWindowController->requestQuickHelp( aLogicHitPos, bIsBalloonHelp, aQuickHelpText, aHelpRect );

        if( bHelpHandled )
        {
            tools::Rectangle aPixelRect(LogicToPixel(lcl_AWTRectToVCLRect(aHelpRect)));
            tools::Rectangle aScreenRect(OutputToScreenPixel(aPixelRect.TopLeft()),
                                         OutputToScreenPixel(aPixelRect.BottomRight()));

            if( bIsBalloonHelp )
                Help::ShowBalloon(this, rHEvt.GetMousePosPixel(), aScreenRect, aQuickHelpText);
            else
                Help::ShowQuickHelp(this, aScreenRect, aQuickHelpText);
        }
    }

    if( !bHelpHandled )
        vcl::Window::RequestHelp( rHEvt );
}

void ChartWindow::adjustHighContrastMode()
{
    static const DrawModeFlags nContrastMode =
        DrawModeFlags::SettingsLine | DrawModeFlags::SettingsFill |
        DrawModeFlags::SettingsText | DrawModeFlags::SettingsGradient;

    bool bUseContrast = GetSettings().GetStyleSettings().GetHighContrastMode();
    GetOutDev()->SetDrawMode( bUseContrast ? nContrastMode : DrawModeFlags::Default );
}

void ChartWindow::ForceInvalidate()
{
    vcl::Window::Invalidate();
}
void ChartWindow::ImplInvalidate( const vcl::Region* rRegion, InvalidateFlags nFlags )
{
    if( m_bInPaint ) // #i101928# superfluous paint calls while entering and editing charts"
        return;
    vcl::Window::ImplInvalidate( rRegion, nFlags );
}

void ChartWindow::LogicInvalidate(const tools::Rectangle* pRectangle)
{
    SfxViewShell* pCurrentShell = SfxViewShell::Current();
    if ( nullptr == pCurrentShell )
        return;
    tools::Rectangle aResultRectangle;
    if (!pRectangle)
    {
        // we have to invalidate the whole chart area not the whole document
        aResultRectangle = GetBoundingBox();
    }
    else
    {
        tools::Rectangle aRectangle(*pRectangle);
        // When dragging shapes the map mode is disabled.
        if (IsMapModeEnabled())
        {
            if (GetMapMode().GetMapUnit() == MapUnit::Map100thMM)
            {
                aRectangle = o3tl::convert(aRectangle, o3tl::Length::mm100, o3tl::Length::twip);
            }
        }
        else
        {
            aRectangle = PixelToLogic(aRectangle, MapMode(MapUnit::MapTwip));
        }

        vcl::Window* pEditWin = GetParentEditWin();
        if (pEditWin)
        {
            MapMode aCWMapMode = GetMapMode();
            constexpr auto p = o3tl::getConversionMulDiv(o3tl::Length::px, o3tl::Length::twip);
            const auto& scaleX = aCWMapMode.GetScaleX();
            const auto& scaleY = aCWMapMode.GetScaleY();
            const auto nXNum = p.first * scaleX.GetDenominator();
            const auto nXDen = p.second * scaleX.GetNumerator();
            const auto nYNum = p.first * scaleY.GetDenominator();
            const auto nYDen = p.second * scaleY.GetNumerator();

            if (!IsMapModeEnabled())
            {
                aRectangle = aRectangle.scale(scaleX.GetDenominator(), scaleX.GetNumerator(),
                                              scaleY.GetDenominator(), scaleY.GetNumerator());
            }

            Point aOffset = this->GetOffsetPixelFrom(*pEditWin).scale(nXNum, nXDen, nYNum, nYDen);

            aRectangle = tools::Rectangle(aRectangle.TopLeft() + aOffset, aRectangle.GetSize());
        }

        aResultRectangle = aRectangle;
    }
    SfxLokHelper::notifyInvalidation(pCurrentShell, &aResultRectangle);
}

FactoryFunction ChartWindow::GetUITestFactory() const
{
    return ChartWindowUIObject::create;
}

ChartController* ChartWindow::GetController()
{
    return m_pWindowController;
}

vcl::Window* ChartWindow::GetParentEditWin()
{
    if (m_pViewShellWindow)
        return m_pViewShellWindow.get();

    // So, you are thinking, why do not invoke pCurrentShell->GetWindow() ?
    // Because in Impress the parent edit win is not view shell window.
    SfxViewShell* pCurrentShell = SfxViewShell::Current();
    if( pCurrentShell )
    {
        SfxInPlaceClient* pIPClient = pCurrentShell->GetIPClient();
        if (pIPClient)
        {
            vcl::Window* pRootWin = pIPClient->GetEditWin();
            if(pRootWin && pRootWin->IsAncestorOf(*this))
            {
                m_pViewShellWindow = pRootWin;
                return m_pViewShellWindow.get();
            }
        }
    }
    return nullptr;
}

tools::Rectangle ChartWindow::GetBoundingBox()
{
    tools::Rectangle aBBox;

    vcl::Window* pRootWin = GetParentEditWin();
    if (pRootWin)
    {
        // In all cases, the following code fragment
        // returns the chart bounding box in twips.
        MapMode aCWMapMode = GetMapMode();
        constexpr auto p = o3tl::getConversionMulDiv(o3tl::Length::px, o3tl::Length::twip);
        const auto& scaleX = aCWMapMode.GetScaleX();
        const auto& scaleY = aCWMapMode.GetScaleY();
        const auto nXNum = p.first * scaleX.GetDenominator();
        const auto nXDen = p.second * scaleX.GetNumerator();
        const auto nYNum = p.first * scaleY.GetDenominator();
        const auto nYDen = p.second * scaleY.GetNumerator();

        Point aOffset = GetOffsetPixelFrom(*pRootWin);
        aOffset.setX( o3tl::convert(aOffset.X(), nXNum, nXDen) );
        aOffset.setY( o3tl::convert(aOffset.Y(), nYNum, nYDen) );
        Size aSize = GetSizePixel();
        aSize.setWidth( o3tl::convert(aSize.Width(), nXNum, nXDen) );
        aSize.setHeight( o3tl::convert(aSize.Height(), nYNum, nYDen) );
        aBBox = tools::Rectangle(aOffset, aSize);
    }
    return aBBox;
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
