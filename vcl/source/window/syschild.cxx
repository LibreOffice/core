/*************************************************************************
 *
 *  $RCSfile: syschild.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:05:40 $
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

#define _SV_SYSCHILD_CXX

#ifndef REMOTE_APPSERVER
#ifndef _SV_SVSYS_HXX
#include <svsys.h>
#endif
#ifndef _SV_SALINST_HXX
#include <salinst.hxx>
#endif
#ifndef _SV_SALFRAME_HXX
#include <salframe.hxx>
#endif
#include <window.hxx>
#ifndef _SV_SALOBJ_HXX
#include <salobj.hxx>
#endif
#endif

#ifndef _SV_RC_H
#include <rc.h>
#endif
#ifndef _SV_SVDATA_HXX
#include <svdata.hxx>
#endif
#ifndef _SV_WIDNOW_H
#include <window.h>
#endif
#ifndef _SV_SVAPP_HXX
#include <svapp.hxx>
#endif
#ifndef _SV_SYSCHILD_HXX
#include <syschild.hxx>
#endif

#pragma hdrstop

// =======================================================================

#ifndef REMOTE_APPSERVER

long ImplSysChildProc( void* pInst, SalObject* /* pObject */,
                       USHORT nEvent, const void* /* pEvent */ )
{
    SystemChildWindow* pWindow = (SystemChildWindow*)pInst;
    long nRet = 0;

    switch ( nEvent )
    {
        case SALOBJ_EVENT_GETFOCUS:
            // Focus holen und zwar so, das alle Handler gerufen
            // werden, als ob dieses Fenster den Focus bekommt,
            // ohne das der Frame den Focus wieder klaut
            pWindow->ImplGetFrameData()->mbSysObjFocus = TRUE;
            pWindow->ImplGetFrameData()->mbInSysObjToTopHdl = TRUE;
            pWindow->ToTop( TOTOP_NOGRABFOCUS );
            pWindow->ImplGetFrameData()->mbInSysObjToTopHdl = FALSE;
            pWindow->ImplGetFrameData()->mbInSysObjFocusHdl = TRUE;
            pWindow->GrabFocus();
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
            pWindow->GrabFocus();
            pWindow->ImplGetFrameData()->mbInSysObjToTopHdl = FALSE;
            break;
    }

    return nRet;
}

#endif

// =======================================================================

void SystemChildWindow::ImplInit( Window* pParent, WinBits nStyle )
{
#ifndef REMOTE_APPSERVER
    mpSysObj = ImplGetSVData()->mpDefInst->CreateObject( pParent->ImplGetFrame() );
#endif

    Window::ImplInit( pParent, nStyle, NULL );

#ifndef REMOTE_APPSERVER
    // Wenn es ein richtiges SysChild ist, dann painten wir auch nicht
    if ( GetSystemData() )
    {
        mpSysObj->SetCallback( this, ImplSysChildProc );
        SetParentClipMode( PARENTCLIPMODE_CLIP );
        SetBackground();
    }
#endif
}

// -----------------------------------------------------------------------

SystemChildWindow::SystemChildWindow( Window* pParent, WinBits nStyle ) :
    Window( WINDOW_SYSTEMCHILDWINDOW )
{
    ImplInit( pParent, nStyle );
}

// -----------------------------------------------------------------------

SystemChildWindow::SystemChildWindow( Window* pParent, const ResId& rResId ) :
    Window( WINDOW_SYSTEMCHILDWINDOW )
{
    rResId.SetRT( RSC_WINDOW );
    WinBits nStyle = ImplInitRes( rResId );
    ImplInit( pParent, nStyle );
    ImplLoadRes( rResId );

    if ( !(nStyle & WB_HIDE) )
        Show();
}

// -----------------------------------------------------------------------

SystemChildWindow::~SystemChildWindow()
{
#ifndef REMOTE_APPSERVER
    Hide();
    if ( mpSysObj )
    {
        ImplGetSVData()->mpDefInst->DestroyObject( mpSysObj );
        mpSysObj = NULL;
    }
#endif
}

// -----------------------------------------------------------------------

const SystemEnvData* SystemChildWindow::GetSystemData() const
{
#ifndef REMOTE_APPSERVER
    if ( mpSysObj )
        return mpSysObj->GetSystemData();
    else
        return NULL;
#else
    return NULL;
#endif
}

