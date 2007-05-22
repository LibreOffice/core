/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ChartWindow.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 18:05:34 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
{
    this->SetSmartHelpId( SmartId( HID_SCH_WIN_DOCUMENT ) );
    this->SetMapMode( MapMode(MAP_100TH_MM) );
    adjustHighContrastMode();
}

ChartWindow::~ChartWindow()
{
}

void ChartWindow::clear()
{
    m_pWindowController=0;
    this->ReleaseMouse();
}

void ChartWindow::Paint( const Rectangle& rRect )
{
    if( m_pWindowController )
        m_pWindowController->execute_Paint( rRect );
    else
        Window::Paint( rRect );
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

//.............................................................................
} //namespace chart
//.............................................................................
