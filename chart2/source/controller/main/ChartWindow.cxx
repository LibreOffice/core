/*************************************************************************
 *
 *  $RCSfile: ChartWindow.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: iha $ $Date: 2003-10-28 15:50:17 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2003 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
