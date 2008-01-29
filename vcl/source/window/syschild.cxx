/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: syschild.cxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: vg $ $Date: 2008-01-29 08:37:44 $
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
#include "precompiled_vcl.hxx"

#ifndef _SV_SVSYS_HXX
#include <svsys.h>
#endif
#ifndef _SV_SALINST_HXX
#include <vcl/salinst.hxx>
#endif
#ifndef _SV_SALFRAME_HXX
#include <vcl/salframe.hxx>
#endif
#include <vcl/window.hxx>
#ifndef _SV_SALOBJ_HXX
#include <vcl/salobj.hxx>
#endif

#ifndef _SV_RC_H
#include <tools/rc.h>
#endif
#ifndef _SV_SVDATA_HXX
#include <vcl/svdata.hxx>
#endif
#ifndef _SV_WIDNOW_H
#include <vcl/window.h>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _SV_SYSCHILD_HXX
#include <vcl/syschild.hxx>
#endif



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
