/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ChartWindow.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 00:33:51 $
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
#include "ChartWindow.hxx"
#include "ChartController.hxx"

using namespace ::com::sun::star;

//.............................................................................
namespace chart
{
//.............................................................................

ChartWindow::ChartWindow( WindowController* pWindowController, Window* pParent, WinBits nStyle )
        : Window(pParent, nStyle)
        , m_pWindowController( pWindowController )
{
    this->SetMapMode( MapMode(MAP_100TH_MM) );
}

ChartWindow::~ChartWindow()
{
}

void ChartWindow::Paint( const Rectangle& rRect )
{
    m_pWindowController->execute_Paint( rRect );
}

void ChartWindow::MouseButtonDown(const MouseEvent& rMEvt)
{
    m_pWindowController->execute_MouseButtonDown(rMEvt);
}

void ChartWindow::MouseMove( const MouseEvent& rMEvt )
{
    m_pWindowController->execute_MouseMove( rMEvt );
}

void ChartWindow::Tracking( const TrackingEvent& rTEvt )
{
    m_pWindowController->execute_Tracking( rTEvt );
}

void ChartWindow::MouseButtonUp( const MouseEvent& rMEvt )
{
    m_pWindowController->execute_MouseButtonUp( rMEvt );
}

void ChartWindow::Resize()
{
    m_pWindowController->execute_Resize();
}

void ChartWindow::Activate()
{
    m_pWindowController->execute_Activate();
}
void ChartWindow::Deactivate()
{
    m_pWindowController->execute_Deactivate();
}
void ChartWindow::GetFocus()
{
    m_pWindowController->execute_GetFocus();
}
void ChartWindow::LoseFocus()
{
    m_pWindowController->execute_LoseFocus();
}

void ChartWindow::Command( const CommandEvent& rCEvt )
{
    m_pWindowController->execute_Command( rCEvt );
}

void ChartWindow::KeyInput( const KeyEvent& rKEvt )
{
    if( !m_pWindowController->execute_KeyInput(rKEvt) )
        Window::KeyInput(rKEvt);
}

//.............................................................................
} //namespace chart
//.............................................................................
