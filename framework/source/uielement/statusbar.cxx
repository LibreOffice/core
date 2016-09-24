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

#include <uielement/statusbar.hxx>

#include <vcl/svapp.hxx>

namespace framework
{

FrameworkStatusBar::FrameworkStatusBar(
    vcl::Window*           pParent,
    WinBits           nWinBits ) :
    StatusBar( pParent, nWinBits ),
    m_pMgr( nullptr )
{
    // set optimal size
    SetOutputSizePixel( CalcWindowSizePixel() );
}

void FrameworkStatusBar::SetStatusBarManager( StatusBarManager* pStatusBarManager )
{
    SolarMutexGuard aSolarMutexGuard;
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

void FrameworkStatusBar::StateChanged( StateChangedType )
{
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
