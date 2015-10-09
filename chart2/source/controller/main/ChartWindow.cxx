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
        , m_pOpenGLWindow(VclPtr<OpenGLWindow>::Create(this))
{
    this->SetHelpId( HID_SCH_WIN_DOCUMENT );
    this->SetMapMode( MapMode(MAP_100TH_MM) );
    adjustHighContrastMode();
    // chart does not depend on exact pixel painting => enable antialiased drawing
    SetAntialiasing( AntialiasingFlags::EnableB2dDraw | GetAntialiasing() );
    EnableRTL( false );
    if( pParent )
        pParent->EnableRTL( false );// #i96215# necessary for a correct position of the context menu in rtl mode

    if( m_pOpenGLWindow )
    {
        m_pOpenGLWindow->Show();
        uno::Reference< chart2::X3DChartWindowProvider > x3DWindowProvider(pController->getModel(), uno::UNO_QUERY_THROW);
        sal_uInt64 nWindowPtr = reinterpret_cast<sal_uInt64>(m_pOpenGLWindow.get());
        x3DWindowProvider->setWindow(nWindowPtr);
        uno::Reference<util::XUpdatable> const xUpdatable(x3DWindowProvider,
                uno::UNO_QUERY_THROW);
        xUpdatable->update();
    }
}

ChartWindow::~ChartWindow()
{
    disposeOnce();
}

void ChartWindow::dispose()
{
    if (m_pWindowController && m_pWindowController->getModel().is())
    {
        uno::Reference< chart2::X3DChartWindowProvider > x3DWindowProvider(m_pWindowController->getModel(), uno::UNO_QUERY_THROW);
        x3DWindowProvider->setWindow(0);
        uno::Reference<util::XUpdatable> const xUpdatable(x3DWindowProvider,
                uno::UNO_QUERY_THROW);
        xUpdatable->update();
    }
    m_pOpenGLWindow.disposeAndClear();
    vcl::Window::dispose();
}

void ChartWindow::clear()
{
    m_pWindowController=0;
    this->ReleaseMouse();
}

void ChartWindow::PrePaint(vcl::RenderContext& rRenderContext)
{
    // forward VCLs PrePaint window event to DrawingLayer
    if (m_pWindowController)
    {
       m_pWindowController->PrePaint(rRenderContext);
    }
}

void ChartWindow::Paint(vcl::RenderContext& rRenderContext, const Rectangle& rRect)
{
    m_bInPaint = true;
    if (m_pOpenGLWindow && m_pOpenGLWindow->IsVisible())
    {
        m_pOpenGLWindow->Paint(rRenderContext, rRect);
    }
    else if (m_pWindowController)
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

    if( m_pOpenGLWindow )
        m_pOpenGLWindow->SetSizePixel(GetSizePixel());
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
    if(m_pOpenGLWindow)
    {
        m_pOpenGLWindow->Invalidate();
    }
}
void ChartWindow::Invalidate( InvalidateFlags nFlags )
{
    if( m_bInPaint ) // #i101928# superfluous paint calls while entering and editing charts"
        return;
    vcl::Window::Invalidate( nFlags );
    if(m_pOpenGLWindow)
    {
        m_pOpenGLWindow->Invalidate( nFlags );
    }
}
void ChartWindow::Invalidate( const Rectangle& rRect, InvalidateFlags nFlags )
{
    if( m_bInPaint ) // #i101928# superfluous paint calls while entering and editing charts"
        return;
    vcl::Window::Invalidate( rRect, nFlags );
    if(m_pOpenGLWindow)
    {
        m_pOpenGLWindow->Invalidate( rRect, nFlags );
    }
}
void ChartWindow::Invalidate( const vcl::Region& rRegion, InvalidateFlags nFlags )
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
