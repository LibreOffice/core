/*************************************************************************
 *
 *  $RCSfile: statusbar.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2004-11-26 16:19:49 $
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
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef __FRAMEWORK_UIELEMENT_STATUSBAR_HXX_
#include <uielement/statusbar.hxx>
#endif

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________

#include <vcl/svapp.hxx>

namespace framework
{

FrameworkStatusBar::FrameworkStatusBar(
    Window*           pParent,
    WinBits           nWinBits ) :
    StatusBar( pParent, nWinBits ),
    m_bShow( sal_False ),
    m_bLock( sal_False ),
    m_pMgr( NULL )
{
    // enable a thin border to better separate from the vertical docking areas
    SetTopBorder();
    // set optimal size
    SetOutputSizePixel( CalcWindowSizePixel() );
}

FrameworkStatusBar::~FrameworkStatusBar()
{
}

void FrameworkStatusBar::SetStatusBarManager( StatusBarManager* pStatusBarManager )
{
    vos::OGuard aSolarMutexGuard( Application::GetSolarMutex() );
    m_pMgr = pStatusBarManager;
}

void FrameworkStatusBar::UserDraw(const UserDrawEvent& rUDEvt)
{
    if ( m_pMgr )
        m_pMgr->UserDraw( rUDEvt );
}

void FrameworkStatusBar::Command( const CommandEvent& rEvt )
{
    if ( m_pMgr )
        m_pMgr->Command( rEvt );
}

void FrameworkStatusBar::StateChanged( StateChangedType nType )
{
    if ( m_pMgr )
        m_pMgr->StateChanged( nType );
}

void FrameworkStatusBar::DataChanged( const DataChangedEvent& rDCEvt )
{
    if ( m_pMgr )
        m_pMgr->DataChanged( rDCEvt );
}

}
