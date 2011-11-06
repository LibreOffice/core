/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_framework.hxx"
#include <uielement/statusbar.hxx>

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
