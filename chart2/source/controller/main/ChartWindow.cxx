/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"
#include "ChartWindow.hxx"
#include "ChartController.hxx"
#include "HelpIds.hrc"

#include <vcl/help.hxx>

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


//.............................................................................
namespace chart
{
//.............................................................................

ChartWindow::ChartWindow( WindowController* pWindowController, Window* pParent, WinBits nStyle )
        : Window(pParent, nStyle)
        , m_pWindowController( pWindowController )
        , m_bInPaint(false)
{
    this->SetHelpId( HID_SCH_WIN_DOCUMENT );
    this->SetMapMode( MapMode(MAP_100TH_MM) );
    adjustHighContrastMode();
    // chart does not depend on exact pixel painting => enable antialiased drawing
    SetAntialiasing( ANTIALIASING_ENABLE_B2DDRAW | GetAntialiasing() );
    EnableRTL( sal_False );
    if( pParent )
        pParent->EnableRTL( sal_False );// #i96215# necessary for a correct position of the context menu in rtl mode
}

ChartWindow::~ChartWindow()
{
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
    if( m_pWindowController )
        m_pWindowController->execute_Paint( rRect );
    else
        Window::Paint( rRect );
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

uno::Reference< accessibility::XAccessible > ChartWindow::CreateAccessible()
{
    if( m_pWindowController )
        return m_pWindowController->CreateAccessible();
    else
        return Window::CreateAccessible();
}

void ChartWindow::DataChanged( const DataChangedEvent& rDCEvt )
{
    ::Window::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
         (rDCEvt.GetFlags() & SETTINGS_STYLE) )
    {
        adjustHighContrastMode();
    }
}

void ChartWindow::RequestHelp( const HelpEvent& rHEvt )
{
    bool bHelpHandled = false;
    if( ( rHEvt.GetMode() & HELPMODE_QUICK ) &&
        m_pWindowController )
    {
//         Point aLogicHitPos = PixelToLogic( rHEvt.GetMousePosPixel()); // old chart: GetPointerPosPixel()
        Point aLogicHitPos = PixelToLogic( GetPointerPosPixel());
        ::rtl::OUString aQuickHelpText;
        awt::Rectangle aHelpRect;
        bool bIsBalloonHelp( Help::IsBalloonHelpEnabled() );
        bHelpHandled = m_pWindowController->requestQuickHelp( aLogicHitPos, bIsBalloonHelp, aQuickHelpText, aHelpRect );

        if( bHelpHandled )
        {
            if( bIsBalloonHelp )
                Help::ShowBalloon(
                    this, rHEvt.GetMousePosPixel(), lcl_AWTRectToVCLRect( aHelpRect ), String( aQuickHelpText ));
            else
                Help::ShowQuickHelp(
                    this, lcl_AWTRectToVCLRect( aHelpRect ), String( aQuickHelpText ));
        }
    }

    if( !bHelpHandled )
        ::Window::RequestHelp( rHEvt );
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
    ::Window::Invalidate();
}
void ChartWindow::Invalidate( sal_uInt16 nFlags )
{
    if( m_bInPaint ) // #i101928# superfluous paint calls while entering and editing charts"
        return;
    ::Window::Invalidate( nFlags );
}
void ChartWindow::Invalidate( const Rectangle& rRect, sal_uInt16 nFlags )
{
    if( m_bInPaint ) // #i101928# superfluous paint calls while entering and editing charts"
        return;
    ::Window::Invalidate( rRect, nFlags );
}
void ChartWindow::Invalidate( const Region& rRegion, sal_uInt16 nFlags )
{
    if( m_bInPaint ) // #i101928# superfluous paint calls while entering and editing charts"
        return;
    ::Window::Invalidate( rRegion, nFlags );
}

//.............................................................................
} //namespace chart
//.............................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
