/*************************************************************************
 *
 *  $RCSfile: syswin.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obr $ $Date: 2001-02-05 09:45:05 $
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

#define _SV_SYSWIN_CXX

#ifndef _DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef REMOTE_APPSERVER
#ifndef _SV_SVSYS_HXX
#include <svsys.h>
#endif
#ifndef _SV_SALFRAME_HXX
#include <salframe.hxx>
#endif
#endif

#ifndef _SV_SVDATA_HXX
#include <svdata.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <svapp.hxx>
#endif
#ifndef _SV_BRDWIN_HXX
#include <brdwin.hxx>
#endif
#ifndef _SV_MENU_HXX
#include <menu.hxx>
#endif
#ifndef _SV_ACCESS_HXX
#include <access.hxx>
#endif
#ifndef _SV_WINDOW_H
#include <window.h>
#endif
#ifndef _SV_BRDWIN_HXX
#include <brdwin.hxx>
#endif
#ifndef _SV_SYSWIN_HXX
#include <syswin.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <svapp.hxx>
#endif

#include <unowrap.hxx>
#include <dndevdis.hxx>

#ifdef REMOTE_APPSERVER
#include "rmwindow.hxx"
#endif

#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif
#ifndef _COM_SUN_STAR_DATATRANSFER_DND_XDRAGSOURCE_HPP_
#include <com/sun/star/datatransfer/dnd/XDragSource.hpp>
#endif
#ifndef _COM_SUN_STAR_DATATRANSFER_DND_XDROPTARGETFACTORY_HPP_
#include <com/sun/star/datatransfer/dnd/XDropTargetFactory.hpp>
#endif

#pragma hdrstop
using namespace rtl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::datatransfer::dnd;

// =======================================================================

SystemWindow::SystemWindow( WindowType nType ) :
    Window( nType )
{
    mbSysWin            = TRUE;
    mnActivateMode      = ACTIVATE_MODE_GRABFOCUS;

    mpMenuBar           = NULL;
    mbPined             = FALSE;
    mbRollUp            = FALSE;
    mbRollFunc          = FALSE;
    mbDockBtn           = FALSE;
    mbHideBtn           = FALSE;
    mnMenuBarMode       = MENUBAR_MODE_NORMAL;

    try {
#ifdef REMOTE_APPSERVER
    //  Reference< XMultiServiceFactory > xFactory;

#else

        Reference< XMultiServiceFactory > xFactory = ::comphelper::getProcessServiceFactory();
        if ( xFactory.is() )
        {
            Sequence< sal_Int8 > windowId( 4 );

            /*
             * IMHO this belongs in the platform dependend part of vcl, but the vcl guys say it
             * is better to implement it here to reduce dependencies on the other ports like Mac etc.
             */

            const SystemEnvData * pEnvData = GetSystemData();
            if( pEnvData )
            {
                Reference< XInterface > xInstance;

#if defined WNT
                xInstance = xFactory->createInstance( OUString::createFromAscii( "com.sun.star.datatransfer.dnd.OleDragAndDrop" ) );
                * ( reinterpret_cast < HWND * > ( windowId.getArray() ) ) = pEnvData->hWnd;
#elif defined UNX
                xInstance = xFactory->createInstance( OUString::createFromAscii( "com.sun.star.datatransfer.dnd.X11DragAndDrop" ) );
                * ( reinterpret_cast < long * > ( windowId.getArray() ) ) = pEnvData->aWindow;
#endif

                // FIXME: add service names for macos etc. here

                // remember drag source
                mxDragSource = Reference< XDragSource >( xInstance, UNO_QUERY );

                // create drop target
                Reference< XDropTargetFactory > xDropTargetFactory( xInstance, UNO_QUERY );
                if( xDropTargetFactory.is() )
                {
                    mxDropTarget = xDropTargetFactory->createDropTarget( windowId );
                }
            }
        }
#endif

        if( mxDropTarget.is() )
            mxDropTarget->addDropTargetListener( new DNDEventDispatcher( this ) );
    }

    // createInstance can throw any exception
    catch( Exception exc )
    {
        // release all instances
        mxDropTarget.clear();
        mxDragSource.clear();
    }
}

// -----------------------------------------------------------------------

long SystemWindow::Notify( NotifyEvent& rNEvt )
{
    // Abfangen von KeyEvents fuer Menu-Steuerung
    if ( rNEvt.GetType() == EVENT_KEYINPUT )
    {
        MenuBar* pMBar = mpMenuBar;
        if ( !pMBar && ( GetType() == WINDOW_FLOATINGWINDOW ) )
        {
            SystemWindow* pW = (SystemWindow*)ImplGetFrameWindow()->ImplGetWindow();
            if ( pW )
                pMBar = pW->GetMenuBar();
        }
        if ( pMBar && pMBar->ImplHandleKeyEvent( *rNEvt.GetKeyEvent(), FALSE ) )
            return TRUE;
    }

    return Window::Notify( rNEvt );
}

// -----------------------------------------------------------------------

BOOL SystemWindow::Close()
{
    // shutdown drag and drop for this window
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent > xComponent( mxDropTarget, ::com::sun::star::uno::UNO_QUERY );

    // DNDEventDispatcher does not hold a reference of the DropTarget,
    // so it's ok if it does not support XComponent
    if( xComponent.is() )
        xComponent->dispose();

    if ( mxWindowPeer.is() )
    {
        // #76482# This window can be destroyed in WindowEvent_Close.
        // => Don't use members after calling WindowEvent_Close
        BOOL bCreatedWithToolkit = IsCreatedWithToolkit();
        Application::GetUnoWrapper()->WindowEvent_Close( this );
        if ( bCreatedWithToolkit )
            return FALSE;
    }

    Hide();

    // Ist es das Applikationsfenster, dann beende die Applikation
    if ( Application::GetAppWindow() == (const WorkWindow*)this )
        GetpApp()->Quit();

    return TRUE;
}

// -----------------------------------------------------------------------

void SystemWindow::TitleButtonClick( USHORT )
{
}

// -----------------------------------------------------------------------

void SystemWindow::Pin()
{
}

// -----------------------------------------------------------------------

void SystemWindow::Roll()
{
}

// -----------------------------------------------------------------------

void SystemWindow::Resizing( Size& )
{
}

// -----------------------------------------------------------------------

void SystemWindow::SetZLevel( BYTE nLevel )
{
    Window* pWindow = this;
    while ( pWindow->mpBorderWindow )
        pWindow = pWindow->mpBorderWindow;
    if ( pWindow->mbOverlapWin && !pWindow->mbFrame )
    {
        BYTE nOldLevel = pWindow->mpOverlapData->mnTopLevel;
        pWindow->mpOverlapData->mnTopLevel = nLevel;
        // Wenn der neue Level groesser als der alte ist, schieben
        // wir das Fenster nach hinten
        if ( !IsReallyVisible() && (nLevel > nOldLevel) && pWindow->mpNext )
        {
            // Fenster aus der Liste entfernen
            if ( pWindow->mpPrev )
                pWindow->mpPrev->mpNext = pWindow->mpNext;
            else
                pWindow->mpOverlapWindow->mpFirstOverlap = pWindow->mpNext;
            pWindow->mpNext->mpPrev = pWindow->mpPrev;
            pWindow->mpNext = NULL;
            // und Fenster wieder in die Liste am Ende eintragen
            pWindow->mpPrev = pWindow->mpOverlapWindow->mpLastOverlap;
            pWindow->mpOverlapWindow->mpLastOverlap = pWindow;
            pWindow->mpPrev->mpNext = pWindow;
        }
    }
}

// -----------------------------------------------------------------------

BYTE SystemWindow::GetZLevel() const
{
    const Window* pWindow = this;
    while ( pWindow->mpBorderWindow )
        pWindow = pWindow->mpBorderWindow;
    if ( pWindow->mpOverlapData )
        return pWindow->mpOverlapData->mnTopLevel;
    else
        return FALSE;
}

// -----------------------------------------------------------------------

void SystemWindow::EnableSaveBackground( BOOL bSave )
{
    Window* pWindow = this;
    while ( pWindow->mpBorderWindow )
        pWindow = pWindow->mpBorderWindow;
    if ( pWindow->mbOverlapWin && !pWindow->mbFrame )
    {
        pWindow->mpOverlapData->mbSaveBack = bSave;
        if ( !bSave )
            pWindow->ImplDeleteOverlapBackground();
    }
}

// -----------------------------------------------------------------------

BOOL SystemWindow::IsSaveBackgroundEnabled() const
{
    const Window* pWindow = this;
    while ( pWindow->mpBorderWindow )
        pWindow = pWindow->mpBorderWindow;
    if ( pWindow->mpOverlapData )
        return pWindow->mpOverlapData->mbSaveBack;
    else
        return FALSE;
}

// -----------------------------------------------------------------------

void SystemWindow::ShowTitleButton( USHORT nButton, BOOL bVisible )
{
    if ( nButton == TITLE_BUTTON_DOCKING )
    {
        if ( mbDockBtn != bVisible )
        {
            mbDockBtn = bVisible;
            if ( mpBorderWindow )
                ((ImplBorderWindow*)mpBorderWindow)->SetDockButton( bVisible );
        }
    }
    else /* if ( nButton == TITLE_BUTTON_HIDE ) */
    {
        if ( mbHideBtn != bVisible )
        {
            mbHideBtn = bVisible;
            if ( mpBorderWindow )
                ((ImplBorderWindow*)mpBorderWindow)->SetHideButton( bVisible );
        }
    }
}

// -----------------------------------------------------------------------

BOOL SystemWindow::IsTitleButtonVisible( USHORT nButton ) const
{
    if ( nButton == TITLE_BUTTON_DOCKING )
        return mbDockBtn;
    else /* if ( nButton == TITLE_BUTTON_HIDE ) */
        return mbHideBtn;
}

// -----------------------------------------------------------------------

void SystemWindow::SetPin( BOOL bPin )
{
    if ( bPin != mbPined )
    {
        mbPined = bPin;
        if ( mpBorderWindow )
            ((ImplBorderWindow*)mpBorderWindow)->SetPin( bPin );
    }
}

// -----------------------------------------------------------------------

void SystemWindow::RollUp()
{
    if ( !mbRollUp )
    {
        maOrgSize = GetOutputSizePixel();
        mbRollFunc = TRUE;
        Size aSize = maRollUpOutSize;
        if ( !aSize.Width() )
            aSize.Width() = GetOutputSizePixel().Width();
        mbRollUp = TRUE;
        if ( mpBorderWindow )
            ((ImplBorderWindow*)mpBorderWindow)->SetRollUp( TRUE, aSize );
        else
            SetOutputSizePixel( aSize );
        mbRollFunc = FALSE;
    }
}

// -----------------------------------------------------------------------

void SystemWindow::RollDown()
{
    if ( mbRollUp )
    {
        mbRollUp = FALSE;
        if ( mpBorderWindow )
            ((ImplBorderWindow*)mpBorderWindow)->SetRollUp( FALSE, maOrgSize );
        else
            SetOutputSizePixel( maOrgSize );
    }
}

// -----------------------------------------------------------------------

void SystemWindow::SetMinOutputSizePixel( const Size& rSize )
{
    maMinOutSize = rSize;
    if ( mpBorderWindow )
    {
        ((ImplBorderWindow*)mpBorderWindow)->SetMinOutputSize( rSize.Width(), rSize.Height() );
        if ( mpBorderWindow->mbFrame )
            mpBorderWindow->mpFrame->SetMinClientSize( rSize.Width(), rSize.Height() );
    }
    else if ( mbFrame )
        mpFrame->SetMinClientSize( rSize.Width(), rSize.Height() );
}

// -----------------------------------------------------------------------

Size SystemWindow::GetResizeOutputSizePixel() const
{
    Size aSize = GetOutputSizePixel();
    if ( aSize.Width() < maMinOutSize.Width() )
        aSize.Width() = maMinOutSize.Width();
    if ( aSize.Height() < maMinOutSize.Height() )
        aSize.Height() = maMinOutSize.Height();
    return aSize;
}

// -----------------------------------------------------------------------

void SystemWindow::SetMenuBar( MenuBar* pMenuBar )
{
    if ( mpMenuBar != pMenuBar )
    {
        MenuBar* pOldMenuBar = mpMenuBar;
        Window*  pOldWindow;
        mpMenuBar = pMenuBar;

        if ( mpBorderWindow && (mpBorderWindow->GetType() == WINDOW_BORDERWINDOW) )
        {
            if ( pOldMenuBar )
                pOldWindow = pOldMenuBar->ImplGetWindow();
            else
                pOldWindow = NULL;
            if ( pMenuBar )
            {
                DBG_ASSERT( !pMenuBar->pWindow, "SystemWindow::SetMenuBar() - MenuBars can only set in one SystemWindow at time" );
                ((ImplBorderWindow*)mpBorderWindow)->SetMenuBarWindow( MenuBar::ImplCreate( mpBorderWindow, pOldWindow, pMenuBar ) );
            }
            else
                ((ImplBorderWindow*)mpBorderWindow)->SetMenuBarWindow( NULL );
            ImplToBottomChild();
            if ( pOldMenuBar )
                MenuBar::ImplDestroy( pOldMenuBar, pMenuBar == 0 );
        }

        Application::GenerateAccessEvent( ACCESS_EVENT_MENUBAR );
    }
}

// -----------------------------------------------------------------------

void SystemWindow::SetMenuBarMode( USHORT nMode )
{
    if ( mnMenuBarMode != nMode )
    {
        mnMenuBarMode = nMode;
        if ( mpBorderWindow && (mpBorderWindow->GetType() == WINDOW_BORDERWINDOW) )
        {
            if ( nMode == MENUBAR_MODE_HIDE )
                ((ImplBorderWindow*)mpBorderWindow)->SetMenuBarMode( TRUE );
            else
                ((ImplBorderWindow*)mpBorderWindow)->SetMenuBarMode( FALSE );
        }
    }
}
