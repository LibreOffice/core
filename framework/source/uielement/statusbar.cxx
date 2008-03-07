/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: statusbar.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-07 14:32:23 $
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
#include "precompiled_framework.hxx"

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
    m_pMgr( NULL ),
    m_bShow( sal_False ),
    m_bLock( sal_False )
{
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
    StatusBar::DataChanged( rDCEvt );
    if ( m_pMgr )
        m_pMgr->DataChanged( rDCEvt );
}

void FrameworkStatusBar::MouseMove( const MouseEvent& rMEvt )
{
    StatusBar::MouseMove( rMEvt );
    if ( m_pMgr )
        m_pMgr->MouseMove( rMEvt );
}

void FrameworkStatusBar::MouseButtonDown( const MouseEvent& rMEvt )
{
    StatusBar::MouseButtonDown( rMEvt );
    if ( m_pMgr )
        m_pMgr->MouseButtonDown( rMEvt );
}

void FrameworkStatusBar::MouseButtonUp( const MouseEvent& rMEvt )
{
    StatusBar::MouseButtonUp( rMEvt );
    if ( m_pMgr )
        m_pMgr->MouseButtonUp( rMEvt );
}

}
