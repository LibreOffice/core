/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: syschild.cxx,v $
 * $Revision: 1.13 $
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
#include "precompiled_vcl.hxx"

#ifndef _SV_SVSYS_HXX
#include <svsys.h>
#endif
#include <vcl/salinst.hxx>
#include <vcl/salframe.hxx>
#include <vcl/window.hxx>
#include <vcl/salobj.hxx>

#ifndef _SV_RC_H
#include <tools/rc.h>
#endif
#include <vcl/svdata.hxx>
#ifndef _SV_WIDNOW_H
#include <vcl/window.h>
#endif
#include <vcl/svapp.hxx>
#include <vcl/syschild.hxx>



// =======================================================================

long ImplSysChildProc( void* pInst, SalObject* /* pObject */,
                       USHORT nEvent, const void* /* pEvent */ )
{
    SystemChildWindow* pWindow = (SystemChildWindow*)pInst;
    long nRet = 0;

    ImplDelData aDogTag( pWindow );
    switch ( nEvent )
    {
        case SALOBJ_EVENT_GETFOCUS:
            // Focus holen und zwar so, das alle Handler gerufen
            // werden, als ob dieses Fenster den Focus bekommt,
            // ohne das der Frame den Focus wieder klaut
            pWindow->ImplGetFrameData()->mbSysObjFocus = TRUE;
            pWindow->ImplGetFrameData()->mbInSysObjToTopHdl = TRUE;
            pWindow->ToTop( TOTOP_NOGRABFOCUS );
            if( aDogTag.IsDead() )
                break;
            pWindow->ImplGetFrameData()->mbInSysObjToTopHdl = FALSE;
            pWindow->ImplGetFrameData()->mbInSysObjFocusHdl = TRUE;
            pWindow->GrabFocus();
            if( aDogTag.IsDead() )
                break;
            pWindow->ImplGetFrameData()->mbInSysObjFocusHdl = FALSE;
            break;

        case SALOBJ_EVENT_LOSEFOCUS:
            // Hintenrum einen LoseFocus ausloesen, das der Status
            // der Fenster dem entsprechenden Activate-Status
            // entspricht
            pWindow->ImplGetFrameData()->mbSysObjFocus = FALSE;
            if ( !pWindow->ImplGetFrameData()->mnFocusId )
            {
                pWindow->ImplGetFrameData()->mbStartFocusState = TRUE;
                Application::PostUserEvent( pWindow->ImplGetFrameData()->mnFocusId, LINK( pWindow->ImplGetFrameWindow(), Window, ImplAsyncFocusHdl ) );
            }
            break;

        case SALOBJ_EVENT_TOTOP:
            pWindow->ImplGetFrameData()->mbInSysObjToTopHdl = TRUE;
            if ( !Application::GetFocusWindow() || pWindow->HasChildPathFocus() )
                pWindow->ToTop( TOTOP_NOGRABFOCUS );
            else
                pWindow->ToTop();
            if( aDogTag.IsDead() )
                break;
            pWindow->GrabFocus();
            if( aDogTag.IsDead() )
                break;
            pWindow->ImplGetFrameData()->mbInSysObjToTopHdl = FALSE;
            break;
    }

    return nRet;
}

// =======================================================================

void SystemChildWindow::ImplInitSysChild( Window* pParent, WinBits nStyle, SystemWindowData *pData, BOOL bShow )
{
    mpWindowImpl->mpSysObj = ImplGetSVData()->mpDefInst->CreateObject( pParent->ImplGetFrame(), pData, bShow );

    Window::ImplInit( pParent, nStyle, NULL );

    // Wenn es ein richtiges SysChild ist, dann painten wir auch nicht
    if ( GetSystemData() )
    {
        mpWindowImpl->mpSysObj->SetCallback( this, ImplSysChildProc );
        SetParentClipMode( PARENTCLIPMODE_CLIP );
        SetBackground();
    }
}

// -----------------------------------------------------------------------

SystemChildWindow::SystemChildWindow( Window* pParent, WinBits nStyle ) :
    Window( WINDOW_SYSTEMCHILDWINDOW )
{
    ImplInitSysChild( pParent, nStyle, NULL );
}

// -----------------------------------------------------------------------

SystemChildWindow::SystemChildWindow( Window* pParent, WinBits nStyle, SystemWindowData *pData, BOOL bShow ) :
    Window( WINDOW_SYSTEMCHILDWINDOW )
{
    ImplInitSysChild( pParent, nStyle, pData, bShow );
}

// -----------------------------------------------------------------------

SystemChildWindow::SystemChildWindow( Window* pParent, const ResId& rResId ) :
    Window( WINDOW_SYSTEMCHILDWINDOW )
{
    rResId.SetRT( RSC_WINDOW );
    WinBits nStyle = ImplInitRes( rResId );
    ImplInitSysChild( pParent, nStyle, NULL );
    ImplLoadRes( rResId );

    if ( !(nStyle & WB_HIDE) )
        Show();
}

// -----------------------------------------------------------------------

SystemChildWindow::~SystemChildWindow()
{
    Hide();
    if ( mpWindowImpl->mpSysObj )
    {
        ImplGetSVData()->mpDefInst->DestroyObject( mpWindowImpl->mpSysObj );
        mpWindowImpl->mpSysObj = NULL;
    }
}

// -----------------------------------------------------------------------

const SystemEnvData* SystemChildWindow::GetSystemData() const
{
    if ( mpWindowImpl->mpSysObj )
        return mpWindowImpl->mpSysObj->GetSystemData();
    else
        return NULL;
}

// -----------------------------------------------------------------------

void SystemChildWindow::EnableEraseBackground( BOOL bEnable )
{
    if ( mpWindowImpl->mpSysObj )
        mpWindowImpl->mpSysObj->EnableEraseBackground( bEnable );
}

BOOL SystemChildWindow::IsEraseBackgroundEnabled()
{
    if ( mpWindowImpl->mpSysObj )
        return mpWindowImpl->mpSysObj->IsEraseBackgroundEnabled();
    else
        return FALSE;
}
