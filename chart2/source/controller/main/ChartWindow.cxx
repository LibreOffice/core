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

#define TWIPS_PER_PIXEL 15

using namespace ::com::sun::star;

namespace
{
::tools::Rectangle lcl_AWTRectToVCLRect( const css::awt::Rectangle & rAWTRect )
{
    ::tools::Rectangle aResult;
    aResult.setX( rAWTRect.X );
    aResult.setY( rAWTRect.Y );
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
    set_id("chart_window");
    SetHelpId( HID_SCH_WIN_DOCUMENT );
    SetMapMode( MapMode(MapUnit::Map100thMM) );
    adjustHighContrastMode();
    // chart does not depend on exact pixel painting => enable antialiased drawing
    SetAntialiasing( AntialiasingFlags::Enable | GetAntialiasing() );
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
    if( m_pWindowController )
        return m_pWindowController->CreateAccessible();
    else
        return Window::CreateAccessible();
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
//         Point aLogicHitPos = PixelToLogic( rHEvt.GetMousePosPixel()); // old chart: GetPointerPosPixel()
        Point aLogicHitPos = PixelToLogic( GetPointerPosPixel());
        OUString aQuickHelpText;
        awt::Rectangle aHelpRect;
        bool bIsBalloonHelp( Help::IsBalloonHelpEnabled() );
        bHelpHandled = m_pWindowController->requestQuickHelp( aLogicHitPos, bIsBalloonHelp, aQuickHelpText, aHelpRect );

        if( bHelpHandled )
        {
            if( bIsBalloonHelp )
                Help::ShowBalloon(
                    this, rHEvt.GetMousePosPixel(), lcl_AWTRectToVCLRect( aHelpRect ), aQuickHelpText );
            else
                Help::ShowQuickHelp(
                    this, lcl_AWTRectToVCLRect( aHelpRect ), aQuickHelpText );
        }
    }

    if( !bHelpHandled )
        vcl::Window::RequestHelp( rHEvt );
}

void ChartWindow::LogicMouseButtonDown(const MouseEvent& rEvent)
{
    MouseButtonDown(rEvent);
}

void ChartWindow::LogicMouseButtonUp(const MouseEvent& rEvent)
{
    MouseButtonUp(rEvent);
}

void ChartWindow::LogicMouseMove(const MouseEvent& rEvent)
{
    MouseMove(rEvent);
}

void ChartWindow::adjustHighContrastMode()
{
    static const DrawModeFlags nContrastMode =
        DrawModeFlags::SettingsLine | DrawModeFlags::SettingsFill |
        DrawModeFlags::SettingsText | DrawModeFlags::SettingsGradient;

    bool bUseContrast = GetSettings().GetStyleSettings().GetHighContrastMode();
    SetDrawMode( bUseContrast ? nContrastMode : DrawModeFlags::Default );
}

void ChartWindow::ForceInvalidate()
{
    vcl::Window::Invalidate();
}
void ChartWindow::Invalidate( InvalidateFlags nFlags )
{
    if( m_bInPaint ) // #i101928# superfluous paint calls while entering and editing charts"
        return;
    vcl::Window::Invalidate( nFlags );
}
void ChartWindow::Invalidate( const tools::Rectangle& rRect, InvalidateFlags nFlags )
{
    if( m_bInPaint ) // #i101928# superfluous paint calls while entering and editing charts"
        return;
    vcl::Window::Invalidate( rRect, nFlags );
}
void ChartWindow::Invalidate( const vcl::Region& rRegion, InvalidateFlags nFlags )
{
    if( m_bInPaint ) // #i101928# superfluous paint calls while entering and editing charts"
        return;
    vcl::Window::Invalidate( rRegion, nFlags );
}

void ChartWindow::LogicInvalidate(const tools::Rectangle* pRectangle)
{
    SfxViewShell* pCurrentShell = SfxViewShell::Current();
    if ( nullptr == pCurrentShell )
        return;
    OString sRectangle;
    if (!pRectangle)
    {
        // we have to invalidate the whole chart area not the whole document
        sRectangle = GetBoundingBox().toString();
    }
    else
    {
        tools::Rectangle aRectangle(*pRectangle);
        // When dragging shapes the map mode is disabled.
        if (IsMapModeEnabled())
        {
            if (GetMapMode().GetMapUnit() == MapUnit::Map100thMM)
                aRectangle = OutputDevice::LogicToLogic(aRectangle, MapMode(MapUnit::Map100thMM), MapMode(MapUnit::MapTwip));
        }
        else
        {
            aRectangle = PixelToLogic(aRectangle, MapMode(MapUnit::MapTwip));
        }

        vcl::Window* pEditWin = GetParentEditWin();
        if (pEditWin)
        {
            MapMode aCWMapMode = GetMapMode();
            double fXScale( aCWMapMode.GetScaleX() );
            double fYScale( aCWMapMode.GetScaleY() );

            if (!IsMapModeEnabled())
            {
                aRectangle.SetLeft( aRectangle.Left() / fXScale );
                aRectangle.SetRight( aRectangle.Right() / fXScale );
                aRectangle.SetTop( aRectangle.Top() / fYScale );
                aRectangle.SetBottom( aRectangle.Bottom() / fYScale );
            }

            Point aOffset = this->GetOffsetPixelFrom(*pEditWin);
            aOffset.setX( aOffset.X() * (TWIPS_PER_PIXEL / fXScale) );
            aOffset.setY( aOffset.Y() * (TWIPS_PER_PIXEL / fYScale) );

            aRectangle = tools::Rectangle(aRectangle.TopLeft() + aOffset, aRectangle.GetSize());
        }

        sRectangle = aRectangle.toString();
    }
    SfxLokHelper::notifyInvalidation(pCurrentShell, sRectangle);
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
        double fXScale( aCWMapMode.GetScaleX() );
        double fYScale( aCWMapMode.GetScaleY() );
        Point aOffset = GetOffsetPixelFrom(*pRootWin);
        aOffset.setX( aOffset.X() * (TWIPS_PER_PIXEL / fXScale) );
        aOffset.setY( aOffset.Y() * (TWIPS_PER_PIXEL / fYScale) );
        Size aSize = GetSizePixel();
        aSize.setWidth( aSize.Width() * (TWIPS_PER_PIXEL / fXScale) );
        aSize.setHeight( aSize.Height() * (TWIPS_PER_PIXEL / fYScale) );
        aBBox = tools::Rectangle(aOffset, aSize);
    }
    return aBBox;
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
