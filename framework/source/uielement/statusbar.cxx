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

#include <uielement/statusbar.hxx>

#include <vcl/svapp.hxx>

namespace framework
{

FrameworkStatusBar::FrameworkStatusBar(
    Window*           pParent,
    WinBits           nWinBits ) :
    StatusBar( pParent, nWinBits ),
    m_pMgr( NULL )
{
    // set optimal size
    SetOutputSizePixel( CalcWindowSizePixel() );
}

FrameworkStatusBar::~FrameworkStatusBar()
{
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
