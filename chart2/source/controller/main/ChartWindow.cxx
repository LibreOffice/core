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

#include "ChartWindow.hxx"
#include "ChartController.hxx"
#include "HelpIds.hrc"

#include <vcl/help.hxx>
#include <vcl/openglwin.hxx>
#include <vcl/settings.hxx>

#include <com/sun/star/chart2/X3DChartWindowProvider.hpp>

using namespace ::com::sun::star;

namespace
{
::Rectangle lcl_AWTRectToVCLRect( const ::com::sun::star::awt::Rectangle & rAWTRect )
{
    ::Rectangle aResult;
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
        , m_pOpenGLWindow(new OpenGLWindow(this))
{
    this->SetHelpId( HID_SCH_WIN_DOCUMENT );
    this->SetMapMode( MapMode(MAP_100TH_MM) );
    adjustHighContrastMode();
    // chart does not depend on exact pixel painting => enable antialiased drawing
    SetAntialiasing( ANTIALIASING_ENABLE_B2DDRAW | GetAntialiasing() );
    EnableRTL( false );
    if( pParent )
        pParent->EnableRTL( false );// #i96215# necessary for a correct position of the context menu in rtl mode

    if( m_pOpenGLWindow )
    {
        m_pOpenGLWindow->Show();
        uno::Reference< chart2::X3DChartWindowProvider > x3DWindowProvider(pController->getModel(), uno::UNO_QUERY_THROW);
        sal_uInt64 nWindowPtr = reinterpret_cast<sal_uInt64>(m_pOpenGLWindow);
        x3DWindowProvider->setWindow(nWindowPtr);
        x3DWindowProvider->update();
    }
}

ChartWindow::~ChartWindow()
{
    if (m_pWindowController && m_pWindowController->getModel().is())
    {
        uno::Reference< chart2::X3DChartWindowProvider > x3DWindowProvider(m_pWindowController->getModel(), uno::UNO_QUERY_THROW);
        x3DWindowProvider->setWindow(0);
        x3DWindowProvider->update();
    }
    delete m_pOpenGLWindow;
}

void ChartWindow::clear()
{
    m_pWindowController=0;
    this->ReleaseMouse();
}

void ChartWindow::PrePaint()
{
    // forward VCLs PrePaint window event to DrawingLayer
    if( m_pWindowController )
    {
        m_pWindowController->PrePaint();
    }
}

void ChartWindow::Paint( const Rectangle& rRect )
{
    m_bInPaint = true;
    if (m_pOpenGLWindow && m_pOpenGLWindow->IsVisible())
    {
        m_pOpenGLWindow->Paint(rRect);
    }
    else if (m_pWindowController)
    {
        m_pWindowController->execute_Paint(rRect);
    }
    else
    {
        Window::Paint( rRect );
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
    if( m_pWindowController )
        m_pWindowController->execute_Tracking( rTEvt );
    else
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

    if( m_pOpenGLWindow )
        m_pOpenGLWindow->SetSizePixel(GetSizePixel());
}

void ChartWindow::Activate()
{
    if( m_pWindowController )
        m_pWindowController->execute_Activate();
    else
        Window::Activate();
}
void ChartWindow::Deactivate()
{
    if( m_pWindowController )
        m_pWindowController->execute_Deactivate();
    else
        Window::Deactivate();
}
void ChartWindow::GetFocus()
{
    if( m_pWindowController )
        m_pWindowController->execute_GetFocus();
    else
        Window::GetFocus();
}
void ChartWindow::LoseFocus()
{
    if( m_pWindowController )
        m_pWindowController->execute_LoseFocus();
    else
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

    if ( (rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
         (rDCEvt.GetFlags() & SETTINGS_STYLE) )
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

void ChartWindow::adjustHighContrastMode()
{
    static const sal_Int32 nContrastMode =
        DRAWMODE_SETTINGSLINE | DRAWMODE_SETTINGSFILL |
        DRAWMODE_SETTINGSTEXT | DRAWMODE_SETTINGSGRADIENT;

    bool bUseContrast = GetSettings().GetStyleSettings().GetHighContrastMode();
    SetDrawMode( bUseContrast ? nContrastMode : DRAWMODE_DEFAULT );
}

void ChartWindow::ForceInvalidate()
{
    vcl::Window::Invalidate();
    if(m_pOpenGLWindow)
    {
        m_pOpenGLWindow->Invalidate();
    }
}
void ChartWindow::Invalidate( sal_uInt16 nFlags )
{
    if( m_bInPaint ) // #i101928# superfluous paint calls while entering and editing charts"
        return;
    vcl::Window::Invalidate( nFlags );
    if(m_pOpenGLWindow)
    {
        m_pOpenGLWindow->Invalidate( nFlags );
    }
}
void ChartWindow::Invalidate( const Rectangle& rRect, sal_uInt16 nFlags )
{
    if( m_bInPaint ) // #i101928# superfluous paint calls while entering and editing charts"
        return;
    vcl::Window::Invalidate( rRect, nFlags );
    if(m_pOpenGLWindow)
    {
        m_pOpenGLWindow->Invalidate( rRect, nFlags );
    }
}
void ChartWindow::Invalidate( const vcl::Region& rRegion, sal_uInt16 nFlags )
{
    if( m_bInPaint ) // #i101928# superfluous paint calls while entering and editing charts"
        return;
    vcl::Window::Invalidate( rRegion, nFlags );
    if(m_pOpenGLWindow)
    {
        m_pOpenGLWindow->Invalidate( rRegion, nFlags );
    }
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
