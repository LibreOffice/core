/*************************************************************************
 *
 *  $RCSfile: salframe.cxx,v $
 *
 *  $Revision: 1.69 $
 *
 *  last change: $Author: pl $ $Date: 2001-08-24 10:22:30 $
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

#define _SV_SALFRAME_CXX

// -=-= #includes =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#if !(defined FREEBSD || defined NETBSD)
#include <alloca.h>
#endif

#include <prex.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>
#include <FWS.hxx>
#include <postx.h>

#include <salunx.h>

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _SV_SALDATA_HXX
#include <saldata.hxx>
#endif
#ifndef _SV_SALDISP_HXX
#include <saldisp.hxx>
#endif
#ifndef _SV_SALINST_HXX
#include <salinst.hxx>
#endif
#ifndef _SV_SALGDI_HXX
#include <salgdi.hxx>
#endif
#ifndef _SV_SALFRAME_HXX
#include <salframe.hxx>
#endif
#ifndef _SV_KEYCOES_HXX
#include <keycodes.hxx>
#endif
#ifndef _SV_SOICON_HXX
#include <soicon.hxx>
#endif
#ifndef _SV_SYSDATA_HXX
#include <sysdata.hxx>
#endif
#ifndef _SV_DTINT_HXX
#include <dtint.hxx>
#endif
#ifndef _VCL_SM_HXX
#include <sm.hxx>
#endif
#ifndef _SV_SETTINGS_HXX
#include <settings.hxx>
#endif
#ifndef _VCL_WMADAPTOR_HXX_
#include <wmadaptor.hxx>
#endif
#ifdef USE_PSPRINT
#ifndef _PSPRINT_PRINTERINFOMANAGER_HXX_
#include <psprint/printerinfomanager.hxx>
#endif
#endif

#include <svapp.hxx>

#ifndef _SAL_I18N_INPUTCONTEXT_HXX
#include <i18n_ic.hxx>
#endif

#ifndef _SAL_I18N_KEYSYM_HXX
#include <i18n_keysym.hxx>
#endif

#ifndef _SAL_I18N_STATUS_HXX
#include <i18n_status.hxx>
#endif

using namespace vcl_sal;
using namespace vcl;

// -=-= #defines -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#define CLIENT_EVENTS           StructureNotifyMask \
                                | SubstructureNotifyMask \
                                | KeyPressMask \
                                | KeyReleaseMask \
                                | ButtonPressMask \
                                | ButtonReleaseMask \
                                | PointerMotionMask \
                                | EnterWindowMask \
                                | LeaveWindowMask \
                                | FocusChangeMask \
                                | ExposureMask \
                                | VisibilityChangeMask \
                                | PropertyChangeMask \
                                | ColormapChangeMask

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#define _GetDrawable()      maFrameData.GetDrawable()
#define _GetDisplay()       maFrameData.pDisplay_
#define _GetXDisplay()      maFrameData.GetXDisplay()
#define _GetColormap()      maFrameData.GetColormap()
#define _GetPaintRegion()   maFrameData.GetPaintRegion()
#define _IsMapped()         maFrameData.bMapped_

static XLIB_Window  hPresentationWindow = None;
static SalFrame*    pIntroBitmap = NULL;
static bool     bWasIntroBitmap = false;

// -=-= C++ statics =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static long sal_CallbackDummy( void*, SalFrame*, USHORT, const void* )
{ return 0; }

// -=-= SalInstance =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
SalFrame *SalInstance::CreateFrame( SalFrame *pParent,
                                          ULONG nSalFrameStyle )
{
    SalFrame *pFrame = new SalFrame;

    pFrame->maFrameData.mpParent = pParent;
    if( pParent )
        pParent->maFrameData.maChildren.Insert( pFrame );
    pFrame->maFrameData.Init( nSalFrameStyle );

    return pFrame;
}

SalFrame* SalInstance::CreateChildFrame( SystemParentData* pParentData, ULONG nStyle )
{
    SalFrame* pFrame = new SalFrame;
    pFrame->maFrameData.mpParent = NULL;
    pFrame->maFrameData.Init( nStyle, pParentData );

    return pFrame;
}

void SalInstance::DestroyFrame( SalFrame* pFrame )
{
    delete pFrame;
}

// -=-= SalGraphics / SalGraphicsData =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void SalGraphicsData::Init( SalFrame *pFrame )
{
    xColormap_      = &pFrame->_GetColormap();
    hDrawable_      = pFrame->_GetDrawable();

    bWindow_        = TRUE;

    nPenPixel_      = GetPixel( nPenColor_ );
    nTextPixel_     = GetPixel( nTextColor_ );
    nBrushPixel_    = GetPixel( nBrushColor_ );
}

// -=-= SalFrame / SalFrameData =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

bool SalFrameData::IsOverrideRedirect() const
{ return  ! ( nStyle_ & ~SAL_FRAME_STYLE_DEFAULT ); }

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

void SalFrameData::Init( ULONG nSalFrameStyle, SystemParentData* pParentData )
{
    nStyle_     = nSalFrameStyle;

    XWMHints Hints;
    Hints.flags = InputHint;
    Hints.input = True;
    hStackingWindow_ = None;

    if( nSalFrameStyle & SAL_FRAME_STYLE_DEFAULT )
    {
        hShell_     = pDisplay_->GetShellWidget();
        hComposite_ = pDisplay_->GetWidget();

        XSelectInput( GetXDisplay(), XtWindow( hShell_ ), CLIENT_EVENTS );
        XSelectInput( GetXDisplay(), XtWindow( hComposite_ ), CLIENT_EVENTS );

        Hints.flags |= InputHint;
        Hints.input       = True;
        // default icon
        if( SelectAppIconPixmap( pDisplay_, 1, 32,
                Hints.icon_pixmap, Hints.icon_mask ))
        {
             Hints.flags     |= IconPixmapHint;
            if( Hints.icon_mask )
                Hints.flags |= IconMaskHint;
        }
    }
    else if( nSalFrameStyle & SAL_FRAME_STYLE_FLOAT )
    {
        Arg aArgs[10];
        int nArgs = 0;

        SalVisual* pVis = GetDisplay()->GetVisual();
        XtSetArg( aArgs[nArgs], XtNvisual, pVis->GetVisual() ); nArgs++;
        XtSetArg( aArgs[nArgs], XtNdepth, pVis->GetDepth() );   nArgs++;
        XtSetArg( aArgs[nArgs], XtNcolormap,
                  GetDisplay()->GetColormap().GetXColormap() ); nArgs++;
        XtSetArg( aArgs[nArgs], XtNwidth, 10 );                 nArgs++;
        XtSetArg( aArgs[nArgs], XtNheight, 10 );                nArgs++;

        hShell_ = XtAppCreateShell( NULL, NULL,
                                    overrideShellWidgetClass,
                                    pDisplay_->GetDisplay(),
                                    aArgs, nArgs );

        XtSetMappedWhenManaged( hShell_, FALSE );
        XtRealizeWidget( hShell_ );

        hComposite_     = XtVaCreateManagedWidget(
            "ShellComposite",
            SAL_COMPOSITE_WIDGET,
            hShell_,
            NULL );
        XtRealizeWidget( hComposite_ );
#ifdef DEBUG
        fprintf( stderr, "created new FLOAT style shell\n" );
#endif
    }
    else if( pParentData )
    {
        nStyle_ |= SAL_FRAME_STYLE_CHILD;

        int x_ret, y_ret;
        unsigned int w, h, bw, d;
        XLIB_Window aRoot;
        XLIB_Window aParent;

        XGetGeometry( GetDisplay()->GetDisplay(), pParentData->aWindow,
                      &aRoot, &x_ret, &y_ret, &w, &h, &bw, &d );

        Arg aArgs[10];
        int nArgs = 0;

        SalVisual* pVis = GetDisplay()->GetVisual();
        XtSetArg( aArgs[nArgs], XtNvisual, pVis->GetVisual() ); nArgs++;
        XtSetArg( aArgs[nArgs], XtNdepth, pVis->GetDepth() );   nArgs++;
        XtSetArg( aArgs[nArgs], XtNcolormap,
                  GetDisplay()->GetColormap().GetXColormap() ); nArgs++;
        XtSetArg( aArgs[nArgs], XtNwidth, w );                  nArgs++;
        XtSetArg( aArgs[nArgs], XtNheight, h );                 nArgs++;

        hShell_ = XtAppCreateShell( NULL, NULL,
                                    overrideShellWidgetClass,
                                    pDisplay_->GetDisplay(),
                                    aArgs, nArgs );

        XtSetMappedWhenManaged( hShell_, FALSE );
        XtRealizeWidget( hShell_ );

        XEvent aEvent;
        XReparentWindow( GetDisplay()->GetDisplay(), XtWindow( hShell_ ),
                         pParentData->aWindow, 0, 0 );
        GetDisplay()->GetXLib()->SetIgnoreXErrors( TRUE ); // hack for plugin
        XSync( GetDisplay()->GetDisplay(), False );

        while( ! XCheckTypedWindowEvent( GetDisplay()->GetDisplay(),
                                         XtWindow( hShell_ ),
                                         ReparentNotify,
                                         &aEvent ) )
        {
            usleep(10000);
        }

        hComposite_     = XtVaCreateManagedWidget(
            "ShellComposite",
            SAL_COMPOSITE_WIDGET,
            hShell_,
            NULL );
        XtRealizeWidget( hComposite_ );

        hForeignParent_ = pParentData->aWindow;
        // get foreign top level window
        // we need the ConfigureNotifies of this window
        // to update the positions of this frame's children of
        // type SAL_FRAME_STYLE_FLOAT
        aParent = hForeignParent_;
        hForeignTopLevelWindow_ = hForeignParent_;
        XLIB_Window* pChildren;
        unsigned int nChildren;
        do
        {
            XQueryTree( GetDisplay()->GetDisplay(), hForeignTopLevelWindow_,
                        &aRoot, &aParent, &pChildren, &nChildren );
            XFree( pChildren );
            if( aParent != aRoot )
                hForeignTopLevelWindow_ = aParent;
        } while( aParent != aRoot );

        // check if this is really one of our own frames
        // do not change the input mask in that case
        SalFrame* pFrame = GetSalData()->pFirstFrame_;
        while( pFrame &&
               hForeignParent_ != pFrame->maFrameData.GetWindow() &&
               hForeignParent_ != pFrame->maFrameData.GetShellWindow() )
            pFrame = pFrame->maFrameData.pNextFrame_;

        if( ! pFrame )
        {
            XSelectInput( GetDisplay()->GetDisplay(), hForeignParent_, StructureNotifyMask );
            XSelectInput( GetDisplay()->GetDisplay(), hForeignTopLevelWindow_, StructureNotifyMask );
        }

        SetPosSize( Rectangle( Point( 0, 0 ), Size( w, h ) ) );
    }
    else
    {
        SalVisual *pVisual = pDisplay_->GetVisual();

         int w = 500;
         int h = 400;
        int x = -1;
        int y = -1;
         if( pDisplay_->GetProperties() & PROPERTY_FEATURE_Maximize )
         {
             w = pDisplay_->GetScreenSize().Width();
             h = pDisplay_->GetScreenSize().Height();
         }
        if( nSalFrameStyle & SAL_FRAME_STYLE_SIZEABLE &&
            nSalFrameStyle & SAL_FRAME_STYLE_MOVEABLE )
        {
            w = pDisplay_->GetScreenSize().Width()*2/3;
            h = pDisplay_->GetScreenSize().Height()*2/3;
        }
        if( ! mpParent )
        {
            // find the last document window (if any)
            SalFrame* pFrame = pNextFrame_;
            while( pFrame &&
                   ( pFrame->maFrameData.mpParent
                     || pFrame->maFrameData.IsOverrideRedirect()
                     || ! ( pFrame->maFrameData.nStyle_ & SAL_FRAME_STYLE_SIZEABLE )
                     || ! pFrame->maFrameData.aPosSize_.GetWidth()
                     || ! pFrame->maFrameData.aPosSize_.GetHeight()
                     )
                   )
                pFrame = pFrame->maFrameData.pNextFrame_;
            if( pFrame )
            {
                // set a document position and size
                // the first frame gets positioned by the window manager
                x = pFrame->maFrameData.aPosSize_.Left();
                y = pFrame->maFrameData.aPosSize_.Top();
                w = pFrame->maFrameData.aPosSize_.GetWidth();
                h = pFrame->maFrameData.aPosSize_.GetHeight();
                if( x+w+40 <= pDisplay_->GetScreenSize().Width() &&
                    y+h+40 <= pDisplay_->GetScreenSize().Height()
                    )
                {
                    y += 40;
                    x += 40;
                }
                else
                {
                    x = 10; // leave some space for decoration
                    y = 20;
                }
            }
        }

        Arg aArgs[16];
        int nArgs=0;
        SalVisual* pVis = GetDisplay()->GetVisual();
        XtSetArg( aArgs[nArgs], XtNvisual, pVis->GetVisual() );     nArgs++;
        XtSetArg( aArgs[nArgs], XtNdepth, pVis->GetDepth() );       nArgs++;
        XtSetArg( aArgs[nArgs], XtNcolormap,
                  GetDisplay()->GetColormap().GetXColormap() );     nArgs++;
        if( x >= 0 && y >= 0 )
        {
            XtSetArg( aArgs[nArgs], XtNx, x );                      nArgs++;
            XtSetArg( aArgs[nArgs], XtNy, y );                      nArgs++;
        }
        XtSetArg( aArgs[nArgs], XtNwidth, w );                      nArgs++;
        XtSetArg( aArgs[nArgs], XtNheight, h );                     nArgs++;
        XtSetArg( aArgs[nArgs], XtNallowShellResize, True );        nArgs++;
        XtSetArg( aArgs[nArgs], XtNwinGravity, NorthWestGravity );  nArgs++;
        XtSetArg( aArgs[nArgs], XtNwaitForWm, False );              nArgs++;
        if( mpParent )
        {
            XtSetArg( aArgs[nArgs], XtNsaveUnder, True );           nArgs++;
        }

#ifdef DEBUG
        fprintf( stderr, "nStyle = 0x%x\n", nStyle_ );
#endif
        // look if any frame is in full screen mode
        // in this case make the frame override redirect so it can show
        // above the full screen frame
           if( IsOverrideRedirect() || hPresentationWindow != None )
           {
               XtSetArg( aArgs[nArgs], XtNoverrideRedirect, True ); nArgs++;
           }

        hShell_ = XtAppCreateShell( "", "VCLSalFrame",
                                    applicationShellWidgetClass,
                                    GetXDisplay(),
                                    aArgs, nArgs );

        // X-Window erzeugen
        XtSetMappedWhenManaged( hShell_, FALSE );
        XtRealizeWidget( hShell_ );

        hComposite_     = XtVaCreateManagedWidget(
            "ShellComposite",
            SAL_COMPOSITE_WIDGET,
            hShell_,
            NULL );
        XtRealizeWidget( hComposite_ );

        // default icon
        if( SelectAppIconPixmap( pDisplay_, mpParent ? mpParent->maFrameData.mnIconID : 1, 32,
                                 Hints.icon_pixmap, Hints.icon_mask ))
        {
            Hints.flags      |= IconPixmapHint;
            if( Hints.icon_mask )
                Hints.flags |= IconMaskHint;
        }

        Hints.flags        |= WindowGroupHint;
        Hints.window_group  = mpParent ? mpParent->maFrameData.GetShellWindow() : pDisplay_->GetShellWindow();
        if( x < 0 || y < 0 )
            SetSize( Size( w, h ) );
        else
            SetPosSize( Rectangle( Point( x, y ), Size( w, h ) ) );
    }
    if( hShell_ )
        XSetWindowBackgroundPixmap( pDisplay_->GetDisplay(), XtWindow( hShell_ ), None );
    if( hComposite_ )
        XSetWindowBackgroundPixmap( pDisplay_->GetDisplay(), XtWindow( hComposite_ ), None );

    if( ! ( (nStyle_ & SAL_FRAME_STYLE_CHILD)  && pParentData ) )
    {
        XSetWMHints( GetXDisplay(), XtWindow( hShell_ ), &Hints );


        // WM Protocols && internals
        Atom a[4];
        int  n = 0;

        a[n++] = pDisplay_->getWMAdaptor()->getAtom( WMAdaptor::WM_DELETE_WINDOW );
        a[n++] = pDisplay_->getWMAdaptor()->getAtom( WMAdaptor::WM_SAVE_YOURSELF );

        XSetWMProtocols( GetXDisplay(), XtWindow( hShell_ ), a, n );
    }

    XSync( GetXDisplay(), False );

    int nDecoFlags = WMAdaptor::decoration_All;
    if( nStyle_ & (SAL_FRAME_STYLE_MOVEABLE | SAL_FRAME_STYLE_SIZEABLE | SAL_FRAME_STYLE_CLOSEABLE)
        != (SAL_FRAME_STYLE_MOVEABLE | SAL_FRAME_STYLE_SIZEABLE | SAL_FRAME_STYLE_CLOSEABLE) )
    {
        nDecoFlags = WMAdaptor::decoration_Border;
        if( ! mpParent )
            nDecoFlags |= WMAdaptor::decoration_MinimizeBtn;
        if( nStyle_ & SAL_FRAME_STYLE_CLOSEABLE )
            nDecoFlags |= WMAdaptor::decoration_CloseBtn;
        if( nStyle_ & SAL_FRAME_STYLE_SIZEABLE )
            nDecoFlags |= WMAdaptor::decoration_MaximizeBtn | WMAdaptor::decoration_Resize;
        if( nStyle_ & SAL_FRAME_STYLE_MOVEABLE )
            nDecoFlags |= WMAdaptor::decoration_Title;
    }

    pDisplay_->getWMAdaptor()->
        setFrameTypeAndDecoration(
                                  pFrame_,
                                  mpParent ?
                                  WMAdaptor::windowType_ModelessDialogue
                                  : WMAdaptor::windowType_Normal,
                                  nDecoFlags,
                                  mpParent );

    // Pointer
    pFrame_->SetPointer( POINTER_ARROW );

}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
inline SalFrameData::SalFrameData( SalFrame *pFrame )
{
    SalData* pSalData = GetSalData();

    // insert frame in framelist
    pNextFrame_                 = pSalData->pFirstFrame_;
    pSalData->pFirstFrame_      = pFrame;
    pFrame_                     = pFrame;

    pProc_                      = sal_CallbackDummy;
    pInst_                      = (void*)ILLEGAL_POINTER;

    pDisplay_                   = pSalData->GetCurDisp();
    hShell_                     = NULL;
    hComposite_                 = NULL;
    hForeignParent_             = None;
    hForeignTopLevelWindow_     = None;

    pGraphics_                  = NULL;
    pFreeGraphics_              = NULL;

    hCursor_                    = None;
    nCaptured_                  = 0;

     nReleaseTime_              = 0;
    nKeyCode_                   = 0;
    nKeyState_                  = 0;
    nCompose_                   = -1;

    nShowState_                 = SHOWSTATE_UNKNOWN;
    nLeft_                      = 0;
    nTop_                       = 0;
    nRight_                     = 0;
    nBottom_                    = 0;
    nMaxWidth_                  = 0;
    nMaxHeight_                 = 0;
    nWidth_                     = 0;
    nHeight_                    = 0;
    nStyle_                     = 0;
    bAlwaysOnTop_               = FALSE;
    // #58928# fake to be mapped on startup, because the sclient may be
    // resized before mapping and the
    // SetPosSize / call(salevent_resize) / GetClientSize
    // stuff will not work in that (unmapped) case
    bViewable_                  = TRUE;
    bMapped_                    = FALSE;
    bDefaultPosition_           = TRUE;
    nVisibility_                = VisibilityFullyObscured;

    nScreenSaversTimeout_       = 0;

    mpInputContext              = NULL;
    mbDeleteInputContext        = false;
    mbInputFocus                = False;

    maResizeTimer.SetTimeoutHdl( LINK( this, SalFrameData, HandleResizeTimer ) );
    maResizeTimer.SetTimeout( 50 );

    mpDeleteData                = NULL;

    meWindowType                = WMAdaptor::windowType_Normal;
    mnDecorationFlags           = WMAdaptor::decoration_All;
    mbMaximizedVert             = false;
    mbMaximizedHorz             = false;

    mnIconID                    = 0; // ICON_DEFAULT
}

SalFrame::SalFrame() : maFrameData( this ) {}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
inline SalFrameData::~SalFrameData()
{
    if( GetWindow() == hPresentationWindow )
        hPresentationWindow = None;

    if( pIntroBitmap == pFrame_ )
        pIntroBitmap = NULL;

    NotifyDeleteData ();

    if( pGraphics_ )
    {
        stderr0( "SalFrameData::~SalFrameData pGraphics_\n" );
        pGraphics_->maGraphicsData.DeInit();
        delete pGraphics_;
    }

    if( pFreeGraphics_ )
    {
        pFreeGraphics_->maGraphicsData.DeInit();
        delete pFreeGraphics_;
    }

    if( mpInputContext && mbDeleteInputContext )
        delete mpInputContext;

    if( hShell_ != pDisplay_->GetWidget() )
        XtDestroyWidget( hShell_ );

    SalData* pSalData = GetSalData();

    if( mpInputContext )
        mpInputContext->Unmap( pFrame_ );
    if( pFrame_ == pSalData->pFirstFrame_ )
        pSalData->pFirstFrame_ = GetNextFrame();
    else
    {
        SalFrameData *pTemp = &pSalData->pFirstFrame_->maFrameData;
        while( pTemp->GetNextFrame() != pFrame_ )
            pTemp = &pTemp->GetNextFrame()->maFrameData;

        pTemp->pNextFrame_ = GetNextFrame();
    }
}

SalFrame::~SalFrame()
{
    // aus papis child liste entfernen
    if( maFrameData.mpParent )
        maFrameData.mpParent->maFrameData.maChildren.Remove( this );
    // einige kommen trotzdem immer noch durch
    XSelectInput( _GetXDisplay(), maFrameData.GetShellWindow(), 0 );
    XSelectInput( _GetXDisplay(), maFrameData.GetWindow(), 0 );

    ShowFullScreen( FALSE );

    if( _IsMapped() )
        Show( FALSE );
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// irgendwann auf Liste umstellen

const SystemChildData* SalFrame::GetSystemData() const
{
    SalFrame *pFrame = const_cast<SalFrame*>(this);
    pFrame->maFrameData.maSystemChildData.nSize         = sizeof( SystemChildData );
    pFrame->maFrameData.maSystemChildData.pDisplay      = _GetXDisplay();
    pFrame->maFrameData.maSystemChildData.aWindow       = pFrame->maFrameData.GetWindow();
    pFrame->maFrameData.maSystemChildData.pSalFrame     = pFrame;
    pFrame->maFrameData.maSystemChildData.pWidget       = pFrame->maFrameData.GetWidget();
    pFrame->maFrameData.maSystemChildData.pVisual       = _GetDisplay()->GetVisual()->GetVisual();
    pFrame->maFrameData.maSystemChildData.nDepth        = _GetDisplay()->GetVisual()->GetDepth();
    pFrame->maFrameData.maSystemChildData.aColormap     = _GetDisplay()->GetColormap().GetXColormap();
    pFrame->maFrameData.maSystemChildData.pAppContext   = _GetDisplay()->GetXLib()->GetAppContext();
    pFrame->maFrameData.maSystemChildData.aShellWindow  = pFrame->maFrameData.GetShellWindow();
    pFrame->maFrameData.maSystemChildData.pShellWidget  = pFrame->maFrameData.GetShellWidget();
    return &maFrameData.maSystemChildData;
}

SalGraphics *SalFrameData::GetGraphics()
{
    if( pGraphics_ )
        return NULL;

    if( pFreeGraphics_ )
    {
        pGraphics_      = pFreeGraphics_;
        pFreeGraphics_  = NULL;
    }
    else
    {
        pGraphics_ = new SalGraphics;
        pGraphics_->maGraphicsData.Init( pFrame_ );
    }

    return pGraphics_;
}

SalGraphics *SalFrame::GetGraphics()
{ return maFrameData.GetGraphics(); }

void SalFrame::ReleaseGraphics( SalGraphics *pGraphics )
{
    if( pGraphics != maFrameData.pGraphics_ )
    {
        stderr0( "SalFrame::ReleaseGraphics pGraphics!=pGraphics_" );
        return;
    }

    maFrameData.pFreeGraphics_  = pGraphics;
    maFrameData.pGraphics_      = NULL;
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

void SalFrame::Enable( BOOL bEnable )
{
    // NYI: enable/disable frame
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

void SalFrame::SetIcon( USHORT nIcon )
{
    if ( !( maFrameData.nStyle_ & SAL_FRAME_STYLE_CHILD )
            && !( maFrameData.nStyle_ & SAL_FRAME_STYLE_FLOAT ) )
    {
        maFrameData.mnIconID = nIcon;

        XIconSize *pIconSize;
        int nSizes;
        int iconSize = 32;
        if ( XGetIconSizes( _GetXDisplay(), maFrameData.GetShellWindow(), &pIconSize, &nSizes ) )
        {
#if defined DBG_UTIL || defined DEBUG
            fprintf(stderr, "SalFrame::SetIcon(): found %d IconSizes:\n", nSizes);
#endif
            int i;
            for( i=0; i<nSizes; i++)
            {
                // select largest supported icon
                if( pIconSize[i].max_width > iconSize )
                    iconSize = pIconSize[i].max_width;

#if defined DBG_UTIL || defined DEBUG
                fprintf(stderr, "min: %d, %d\nmax: %d, %d\ninc: %d, %d\n\n",
                        pIconSize[i].min_width, pIconSize[i].min_height,
                        pIconSize[i].max_width, pIconSize[i].max_height,
                        pIconSize[i].width_inc, pIconSize[i].height_inc);
#endif
            }
        }

        XWMHints Hints;
        Hints.flags = 0;
        XWMHints *pHints = XGetWMHints( _GetXDisplay(), maFrameData.GetShellWindow() );
        if( pHints )
        {
            memcpy(&Hints, pHints, sizeof( XWMHints ));
            XFree( pHints );
        }
        pHints = &Hints;

        BOOL bOk = SelectAppIconPixmap( maFrameData.GetDisplay(), nIcon, iconSize,
                pHints->icon_pixmap, pHints->icon_mask );
        if ( !bOk )
        {
            // load default icon (0)
            bOk = SelectAppIconPixmap( maFrameData.GetDisplay(), 0, iconSize,
                pHints->icon_pixmap, pHints->icon_mask );
        }
        if( bOk )
        {
            pHints->flags    |= IconPixmapHint;
            if( pHints->icon_mask )
                pHints->flags |= IconMaskHint;

            XSetWMHints( _GetXDisplay(), maFrameData.GetShellWindow(), pHints );
        }
    }
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

void SalFrame::SetMinClientSize( long nWidth, long nHeight )
{
    if( maFrameData.hShell_ )
    {
        Arg args[10];
        int n = 0;
        XtSetArg( args[n], XtNminWidth, nWidth );   n++;
        XtSetArg( args[n], XtNminHeight, nHeight ); n++;
        XtSetValues( maFrameData.hShell_, args, n );
    }
}

// Show + Pos (x,y,z) + Size (width,height)
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void SalFrame::Show( BOOL bVisible )
{
    if( ( bVisible && maFrameData.bMapped_ )
        || ( !bVisible && !maFrameData.bMapped_ ) )
        return;

    maFrameData.bMapped_   = bVisible;
    maFrameData.bViewable_ = bVisible;
    if( bVisible )
    {
        if( ! bWasIntroBitmap && maFrameData.IsOverrideRedirect() )
        {
            const Size& rScreenSize( maFrameData.pDisplay_->GetScreenSize() );
            if( maFrameData.aPosSize_.GetWidth() < rScreenSize.Width()-30 ||
                maFrameData.aPosSize_.GetHeight() < rScreenSize.Height()-30 )
            {
                bWasIntroBitmap = true;
                pIntroBitmap = this;
            }
        }
        // look for intro bit map; if present, hide it
        if( pIntroBitmap && pIntroBitmap != this )
            pIntroBitmap->Show( FALSE );

        if( maFrameData.nStyle_ & ( SAL_FRAME_STYLE_CHILD | SAL_FRAME_STYLE_FLOAT ) )
            XtPopup( maFrameData.hShell_, XtGrabNone );
        else
            XtMapWidget( maFrameData.hShell_ );
        // Manch ein WM verschluckt Key Events im Fullscreenmode ...
        XSelectInput( _GetXDisplay(), maFrameData.GetShellWindow(), CLIENT_EVENTS );
        XSelectInput( _GetXDisplay(), maFrameData.GetWindow(), CLIENT_EVENTS );

        if( !maFrameData.aPosSize_.IsEmpty()
            && (maFrameData.nWidth_ != maFrameData.aPosSize_.GetWidth()
                || maFrameData.nHeight_ != maFrameData.aPosSize_.GetHeight()) )
        {
            maFrameData.nWidth_  = maFrameData.aPosSize_.GetWidth();
            maFrameData.nHeight_ = maFrameData.aPosSize_.GetHeight();
        }

        XSync( _GetXDisplay(), False );
        maFrameData.Call( SALEVENT_RESIZE, NULL );
    }
    else
    {
        if( maFrameData.getInputContext() )
            maFrameData.getInputContext()->Unmap( this );

        if( maFrameData.nStyle_ & ( SAL_FRAME_STYLE_CHILD | SAL_FRAME_STYLE_FLOAT ) )
            XtPopdown( maFrameData.hShell_ );
        else
            XtUnmapWidget( maFrameData.hShell_ );
    }
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void SalFrame::ToTop( USHORT nFlags )
{
    int i;

    if( nFlags & SAL_FRAME_TOTOP_RESTOREWHENMIN )
        XtMapWidget( maFrameData.hShell_ );

    XRaiseWindow( _GetXDisplay(), maFrameData.GetShellWindow() );
    for( i=0; i < maFrameData.maChildren.Count(); i++ )
        maFrameData.maChildren.GetObject( i )->ToTop( nFlags );
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void SalFrame::GetClientSize( long &rWidth, long &rHeight )
{
    if( ! maFrameData.bViewable_  )
    {
        rWidth = rHeight = 0;
        return;
    }

    rWidth  = maFrameData.aPosSize_.GetWidth();
    rHeight = maFrameData.aPosSize_.GetHeight();

    if( !rWidth || !rHeight )
    {
        XWindowAttributes aAttrib;

        XGetWindowAttributes( _GetXDisplay(), maFrameData.GetShellWindow(), &aAttrib );

        rWidth  = aAttrib.width;
        rHeight = aAttrib.height;
    }
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void SalFrame::SetClientSize( long nWidth, long nHeight )
{
    if( maFrameData.nStyle_ & SAL_FRAME_STYLE_FLOAT )
    {
        maFrameData.SetPosSize( Rectangle(
            Point( maFrameData.aPosSize_.Left(), maFrameData.aPosSize_.Top() ),
            Size( nWidth, nHeight ) ) );
        return;
    }

    XLIB_Window     aDummy;
    int             nX, nY, nScreenWidth, nScreenHeight;
    int             nRealScreenWidth, nRealScreenHeight;
    int             nScreenX = 0, nScreenY = 0;

    nScreenWidth        = _GetDisplay()->GetScreenSize().Width();
    nScreenHeight       = _GetDisplay()->GetScreenSize().Height();
    nRealScreenWidth    = nScreenWidth;
    nRealScreenHeight   = nScreenHeight;

    if( maFrameData.mpParent )
    {
        SalFrame* pFrame = maFrameData.mpParent;
        while( pFrame->maFrameData.mpParent )
            pFrame = pFrame->maFrameData.mpParent;
        if( pFrame->maFrameData.aPosSize_.IsEmpty() )
            pFrame->maFrameData.GetPosSize( pFrame->maFrameData.aPosSize_ );

        if( pFrame->maFrameData.hForeignTopLevelWindow_ )
        {
            XLIB_Window aRoot;
            unsigned int bw, depth;
            XGetGeometry( maFrameData.GetXDisplay(),
                          pFrame->maFrameData.hForeignTopLevelWindow_,
                          &aRoot,
                          &nScreenX, &nScreenY,
                          (unsigned int*)&nScreenWidth,
                          (unsigned int*)&nScreenHeight,
                          &bw, &depth );
        }
        else
        {
            nScreenX        = pFrame->maFrameData.aPosSize_.Left();
            nScreenY        = pFrame->maFrameData.aPosSize_.Top();
            nScreenWidth    = pFrame->maFrameData.aPosSize_.GetWidth();
            nScreenHeight   = pFrame->maFrameData.aPosSize_.GetHeight();
        }
    }

    nX = maFrameData.aPosSize_.Left();
    nY = maFrameData.aPosSize_.Top();

    if ( maFrameData.bDefaultPosition_ )
    {
        SalFrameData* pParentData = maFrameData.mpParent ? &maFrameData.mpParent->maFrameData : NULL;
        if( pParentData && pParentData->nShowState_ == SHOWSTATE_NORMAL )
        {
            if( nWidth >= pParentData->aPosSize_.GetWidth() &&
                nHeight >= pParentData->aPosSize_.GetHeight() )
            {
                nX = pParentData->aPosSize_.Left()+40;
                nY = pParentData->aPosSize_.Top()+40;
            }
            else
            {
                // center the window relative to the top level frame
                nX = (nScreenWidth  - nWidth ) / 2 + nScreenX;
                nY = (nScreenHeight - nHeight) / 2 + nScreenY;
            }
        }
        else
        {
            // center the window relative to screen
            nX = (nRealScreenWidth  - nWidth ) / 2;
            nY = (nRealScreenHeight - nHeight) / 2;
        }
        nX = nX < 0 ? 0 : nX;
        nY = nY < 0 ? 0 : nY;

        if( maFrameData.nShowState_ == SHOWSTATE_NORMAL)
            maFrameData.bDefaultPosition_ = False;
    }

    // once centered, we leave the window where it is with new size
    // but only if it does not run out of screen (unless user placed it there)

    if( nWidth > maFrameData.aPosSize_.GetWidth() )
    {
        if ( nX + nWidth  > nRealScreenWidth  )
            nX = nRealScreenWidth  - nWidth;
        nX = nX < 0 ? 0 : nX;
    }
    if( nHeight > maFrameData.aPosSize_.GetHeight() )
    {
        if ( nY + nHeight > nRealScreenHeight )
            nY = nRealScreenHeight - nHeight;
        // guess: size of top window decoration is 20
        nY = nY < 20 ? 20 : nY;
    }

    Point aPoint ( nX, nY );
    Size  aSize  ( nWidth, nHeight );
    maFrameData.SetPosSize( Rectangle( aPoint, aSize ) );
}

#if 0
void SalFrame::SetClientPosSize( const Rectangle& rRect )
{
    if( maFrameData.nStyle_ & SAL_FRAME_STYLE_CHILD )
        return;

    maFrameData.SetPosSize( rRect );
}
#endif

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void SalFrame::SetAlwaysOnTop( BOOL bOnTop )
{
    // #74406# do not raise fullscreenwindow since it may override the
    // screenlocker
    // maFrameData.bAlwaysOnTop_ = bOnTop;
    if( bOnTop )
        XRaiseWindow( _GetXDisplay(), maFrameData.GetShellWindow() );
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

#define _FRAMESTATE_MASK_GEOMETRY \
     (SAL_FRAMESTATE_MASK_X     | SAL_FRAMESTATE_MASK_Y |   \
      SAL_FRAMESTATE_MASK_WIDTH | SAL_FRAMESTATE_MASK_HEIGHT)

void
SalFrame::SetWindowState( const SalFrameState *pState )
{
    if (pState == NULL)
        return;

    // demand correct positioning from the WM
    XSizeHints  hints;
    XSizeHints  supplied;

    Display*    pDisplay = _GetXDisplay();
    XLIB_Window hWindow  = maFrameData.GetShellWindow();
    if( XGetWMNormalHints (pDisplay, hWindow, &hints, (long*)&supplied ) )
    {
        hints.flags         |= PWinGravity;
        hints.win_gravity    = StaticGravity;
        XSetWMNormalHints (pDisplay, hWindow, &hints );
        XSync (pDisplay, False);
    }

    // Request for position or size change
    if (pState->mnMask & _FRAMESTATE_MASK_GEOMETRY)
    {
        Rectangle aPosSize;

        // initialize with current geometry
        if ((pState->mnMask & _FRAMESTATE_MASK_GEOMETRY) != _FRAMESTATE_MASK_GEOMETRY)
            maFrameData.GetPosSize (aPosSize);

        // change requested properties
        if (pState->mnMask & SAL_FRAMESTATE_MASK_X)
        {
            aPosSize.setX (pState->mnX);
        }
        if (pState->mnMask & SAL_FRAMESTATE_MASK_Y)
        {
            aPosSize.setY (pState->mnY);
        }
        if (pState->mnMask & SAL_FRAMESTATE_MASK_WIDTH)
        {
            long nWidth = pState->mnWidth > 0 ? pState->mnWidth  - 1 : 0;
            aPosSize.setWidth (nWidth);
        }
        if (pState->mnMask & SAL_FRAMESTATE_MASK_HEIGHT)
        {
            int nHeight = pState->mnHeight > 0 ? pState->mnHeight - 1 : 0;
            aPosSize.setHeight (nHeight);
        }

        // resize with new args
        if (maFrameData.pDisplay_->getWMAdaptor()->supportsICCCMPos())
        {
            maFrameData.SetPosSize( aPosSize );
        }
        else
        {
            SetClientSize (aPosSize.GetWidth(), aPosSize.GetHeight());
        }
    }

    // request for status change
    if (pState->mnMask & SAL_FRAMESTATE_MASK_STATE)
    {
        if (pState->mnState & SAL_FRAMESTATE_MAXIMIZED)
        {
            maFrameData.nShowState_ = SHOWSTATE_NORMAL;
            maFrameData.Maximize();
        }
        if (pState->mnState & SAL_FRAMESTATE_MINIMIZED)
        {
            if (maFrameData.nShowState_ == SHOWSTATE_UNKNOWN)
                maFrameData.nShowState_ = SHOWSTATE_NORMAL;
            maFrameData.Minimize();
        }
        if (pState->mnState & SAL_FRAMESTATE_NORMAL)
        {
            if (maFrameData.nShowState_ != SHOWSTATE_NORMAL)
                maFrameData.Restore();
        }
        if (pState->mnState & SAL_FRAMESTATE_ROLLUP)
        {
            /* not implemented */
        }
    }
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
BOOL SalFrame::GetWindowState( SalFrameState* pState )
{
    if( SHOWSTATE_MINIMIZED == maFrameData.nShowState_ )
        pState->mnState = SAL_FRAMESTATE_MINIMIZED;
    else
        pState->mnState = SAL_FRAMESTATE_NORMAL;

    Rectangle aPosSize;
    if (! maFrameData.aRestoreFullScreen_.IsEmpty() )
    {
        aPosSize = maFrameData.aRestoreFullScreen_;
        pState->mnState |= SAL_FRAMESTATE_MAXIMIZED;
    }
    else
    {
        maFrameData.GetPosSize( aPosSize );
    }

    pState->mnX      = aPosSize.Left();
    pState->mnY      = aPosSize.Top();
    pState->mnWidth  = aPosSize.GetWidth();
    pState->mnHeight = aPosSize.GetHeight();

    pState->mnMask   = _FRAMESTATE_MASK_GEOMETRY | SAL_FRAMESTATE_MASK_STATE;

    return TRUE;
}

// ----------------------------------------------------------------------------
// get a screenshot of the current frame including window manager decoration
SalBitmap*
SalFrame::SnapShot()
{
    return NULL;
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void SalFrameData::GetPosSize( Rectangle &rPosSize )
{
    if( aPosSize_.IsEmpty() )
    {
        long w = nMaxWidth_
                 ? nMaxWidth_
                 : pDisplay_->GetScreenSize().Width()  - nLeft_ - nRight_;
        long h = nMaxHeight_
                 ? nMaxHeight_
                 : pDisplay_->GetScreenSize().Height() - nTop_ - nBottom_;

        rPosSize = Rectangle( Point( nLeft_, nTop_ ), Size( w, h ) );
    }
    else
        rPosSize = aPosSize_;
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void SalFrameData::SetSize( const Size &rSize )
{
    XWindowChanges values;
    values.width    = rSize.Width();
    values.height   = rSize.Height();
    if (values.width > 0 && values.height > 0)
    {
         if( ! ( nStyle_ & SAL_FRAME_STYLE_SIZEABLE ) )
         {
             Arg args[10];
             int n = 0;
             XtSetArg( args[n], XtNminWidth, rSize.Width() );   n++;
             XtSetArg( args[n], XtNminHeight, rSize.Height() ); n++;
             XtSetArg( args[n], XtNmaxWidth, rSize.Width() );   n++;
             XtSetArg( args[n], XtNmaxHeight, rSize.Height() ); n++;
             XtSetValues( hShell_, args, n );
         }
        XResizeWindow( GetXDisplay(), GetShellWindow(), rSize.Width(), rSize.Height() );
        XMoveResizeWindow( GetXDisplay(), GetWindow(), 0, 0, rSize.Width(), rSize.Height() );

        if( ! ( nStyle_ & ( SAL_FRAME_STYLE_CHILD | SAL_FRAME_STYLE_FLOAT ) ) )
            MarkWindowAsGoodPositioned( XtWindow( hShell_ ) );

        aPosSize_.SetSize( rSize );

        // allow the external status window to reposition
        if (mbInputFocus && mpInputContext != NULL)
            mpInputContext->SetICFocus ( pFrame_ );
    }
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

void SalFrameData::SetPosSize( const Rectangle &rPosSize )
{
    XWindowChanges values;
    values.x        = rPosSize.Left();
    values.y        = rPosSize.Top();
    values.width    = rPosSize.GetWidth();
    values.height   = rPosSize.GetHeight();

    if ( !values.width || !values.height || aPosSize_ == rPosSize )
        return;
#ifdef DEBUG
    fprintf(stderr, "SalFrameData::SetPosSize( %dx%d+%d+%d )\n",
            rPosSize.GetWidth(), rPosSize.GetHeight(),
            rPosSize.Left(), rPosSize.Top() );
#endif

    if( ! ( nStyle_ & ( SAL_FRAME_STYLE_CHILD | SAL_FRAME_STYLE_FLOAT ) ) )
    {
        MarkWindowAsGoodPositioned( XtWindow( hShell_ ) );

        if( !(pDisplay_->GetProperties() & PROPERTY_SUPPORT_WM_ClientPos) )
        {
            values.x    -= nLeft_;
            values.y    -= nTop_;
        }
    }
     if( ( nStyle_ & SAL_FRAME_STYLE_FLOAT ) && mpParent )
     {
         XLIB_Window aChild;
         // coordinates are relative to parent, so translate to root coordinates
         XTranslateCoordinates( GetDisplay()->GetDisplay(),
                                mpParent->maFrameData.GetWindow(),
                                GetDisplay()->GetRootWindow(),
                                values.x, values.y,
                                &values.x, &values.y,
                                & aChild );
     }

    if( ! ( nStyle_ & SAL_FRAME_STYLE_SIZEABLE ) )
     {
         Arg args[10];
         int n = 0;
         XtSetArg( args[n], XtNminWidth, values.width );        n++;
         XtSetArg( args[n], XtNminHeight, values.height );  n++;
         XtSetArg( args[n], XtNmaxWidth, values.width );        n++;
         XtSetArg( args[n], XtNmaxHeight, values.height );  n++;
         XtSetValues( hShell_, args, n );
     }
    XMoveResizeWindow( GetXDisplay(), GetShellWindow(), values.x, values.y, values.width, values.height );
    XMoveResizeWindow( GetXDisplay(), GetWindow(), 0, 0, values.width, values.height );

    aPosSize_ = rPosSize;
    Call ( SALEVENT_RESIZE, NULL );

    // allow the external status window to reposition
    if (mbInputFocus && mpInputContext != NULL)
        mpInputContext->SetICFocus ( pFrame_ );
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void SalFrameData::Minimize()
{
    if( SHOWSTATE_UNKNOWN == nShowState_ )
    {
        stderr0( "SalFrameData::Minimize SHOWSTATE_UNKNOWN\n" );
        return;
    }

    if( XIconifyWindow( GetXDisplay(),
                        XtWindow( hShell_ ),
                        pDisplay_->GetScreenNumber() ) )
        nShowState_ = SHOWSTATE_MINIMIZED;
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void SalFrameData::Maximize()
{
    if( SHOWSTATE_MINIMIZED == nShowState_ )
    {
        XtMapWidget( hShell_ );
        nShowState_ = SHOWSTATE_NORMAL;
    }

    pDisplay_->getWMAdaptor()->maximizeFrame( pFrame_, true, true );
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void SalFrameData::Restore()
{
    if( SHOWSTATE_UNKNOWN == nShowState_ )
    {
        stderr0( "SalFrameData::Restore SHOWSTATE_UNKNOWN\n" );
        return;
    }

    if( SHOWSTATE_MINIMIZED == nShowState_ )
    {
        XtMapWidget( hShell_ );
        nShowState_ = SHOWSTATE_NORMAL;
    }

    pDisplay_->getWMAdaptor()->maximizeFrame( pFrame_, false, false );
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

void SalFrame::ShowFullScreen( BOOL bFullScreen )
{
    if( maFrameData.aRestoreFullScreen_.IsEmpty() == !bFullScreen )
        return;
    maFrameData.pDisplay_->getWMAdaptor()->maximizeFrame( this, bFullScreen, bFullScreen );
    if( maFrameData.IsOverrideRedirect()
        && WMSupportsFWS( maFrameData.GetXDisplay(), maFrameData.GetDisplay()->GetRootWindow()) )
    {
        AddFwsProtocols( maFrameData.GetXDisplay(), maFrameData.GetShellWindow() );
        RegisterFwsWindow( maFrameData.GetXDisplay(), maFrameData.GetShellWindow() );
    }
}

/* ---------------------------------------------------------------------
   the xautolock pseudo screen saver needs special treatment since it
   doesn't cooperate with XxxxScreenSaver settings
   ------------------------------------------------------------------- */

static Bool
IsRunningXAutoLock( Display *p_display, XLIB_Window a_window )
{
    const char *p_atomname = "XAUTOLOCK_SEMAPHORE_PID";
    Atom        a_pidatom;

    // xautolock interns this atom
    a_pidatom    = XInternAtom( p_display, p_atomname, True );
    if ( a_pidatom == None )
        return False;

    Atom          a_type;
    int           n_format;
    unsigned long n_items;
    unsigned long n_bytes_after;
    pid_t        *p_pid;
    pid_t         n_pid;
    // get pid of running xautolock
    XGetWindowProperty (p_display, a_window, a_pidatom, 0L, 2L, False,
            AnyPropertyType, &a_type, &n_format, &n_items, &n_bytes_after,
            (unsigned char**) &p_pid );
    n_pid = *p_pid;
    XFree( p_pid );

      if ( a_type == XA_INTEGER )
      {
        // check if xautolock pid points to a running process
        if ( kill(n_pid, 0) == -1 )
            return False;
        else
            return True;
    }

    return False;
}

/* definitions from xautolock.c (pl15) */
#define XAUTOLOCK_DISABLE 1
#define XAUTOLOCK_ENABLE  2

static Bool
MessageToXAutoLock( Display *p_display, int n_message )
{
    const char *p_atomname = "XAUTOLOCK_MESSAGE" ;
    Atom        a_messageatom;
    XLIB_Window a_rootwindow;

    a_rootwindow = RootWindowOfScreen( ScreenOfDisplay(p_display, 0) );
    if ( ! IsRunningXAutoLock(p_display, a_rootwindow) )
    {
        // remove any pending messages
        a_messageatom = XInternAtom( p_display, p_atomname, True );
        if ( a_messageatom != None )
            XDeleteProperty( p_display, a_rootwindow, a_messageatom );
        return False;
    }

    a_messageatom = XInternAtom( p_display, p_atomname, False );
    XChangeProperty (p_display, a_rootwindow, a_messageatom, XA_INTEGER,
            8, PropModeReplace, (unsigned char*)&n_message, sizeof(n_message) );

    return True;
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void SalFrame::StartPresentation( BOOL bStart )
{
    if ( bStart )
        MessageToXAutoLock( _GetXDisplay(), XAUTOLOCK_DISABLE );
    else
        MessageToXAutoLock( _GetXDisplay(), XAUTOLOCK_ENABLE );

    hPresentationWindow = bStart ? maFrameData.GetWindow() : None;
    if( bStart || maFrameData.nScreenSaversTimeout_ )
    {
        int timeout, interval, prefer_blanking, allow_exposures;
        XGetScreenSaver( _GetXDisplay(),
                         &timeout,
                         &interval,
                         &prefer_blanking,
                         &allow_exposures );
        if( !bStart )
        {
            XSetScreenSaver( _GetXDisplay(),
                             maFrameData.nScreenSaversTimeout_,
                             interval,
                             prefer_blanking,
                             allow_exposures );
            maFrameData.nScreenSaversTimeout_ = 0;
        }
        else if( timeout )
        {
            maFrameData.nScreenSaversTimeout_ = timeout;
            XResetScreenSaver( _GetXDisplay() );
            XSetScreenSaver( _GetXDisplay(),
                             0,
                             interval,
                             prefer_blanking,
                             allow_exposures );
        }
    }
}

// Pointer
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
inline void SalFrameData::SetPointer( PointerStyle ePointerStyle )
{
    hCursor_ = pDisplay_->GetPointer( ePointerStyle );
    XDefineCursor( GetXDisplay(), XtWindow( hComposite_ ), hCursor_ );

    if( IsCaptured() )
        XChangeActivePointerGrab( GetXDisplay(),
                        PointerMotionMask|ButtonPressMask|ButtonReleaseMask,
                        hCursor_,
                        CurrentTime );
}

void SalFrame::SetPointer( PointerStyle ePointerStyle )
{ maFrameData.SetPointer( ePointerStyle ); }

void SalFrame::CaptureMouse( BOOL bCapture )
{ maFrameData.CaptureMouse( bCapture ); }

void SalFrame::SetPointerPos(long nX, long nY)
{
    /* #87921# when the application tries to center the mouse in the dialog the
     * window isn't mapped already. So use coordinates relative to the root window.
     * Please note that the window isn't reparented yet and still will be moved out of
     * center */
    unsigned int nWindowLeft = maFrameData.aPosSize_.Left() + maFrameData.nLeft_;
    unsigned int nWindowTop  = maFrameData.aPosSize_.Top()  + maFrameData.nTop_;

    XWarpPointer( _GetXDisplay(), None, maFrameData.pDisplay_->GetRootWindow(),
            0, 0, 0, 0, nWindowLeft + nX, nWindowTop + nY);
}

// delay handling of extended text input
#if !defined(__synchronous_extinput__)
void
SalFrameData::PostExtTextEvent (sal_uInt16 nExtTextEventType, void *pExtTextEvent)
{
    XLIB_Window nFocusWindow = GetWindow();
    Atom        nEventAtom   = GetDisplay()->getWMAdaptor()->getAtom( WMAdaptor::SAL_EXTTEXTEVENT );

    sal_uInt32 pEventData[5];

#if __SIZEOFLONG > 4
    pEventData[0] = (sal_uInt32)((long)pExtTextEvent & 0xffffffff);
    pEventData[1] = (sal_uInt32)((long)pExtTextEvent >> 32);
#else
    pEventData[0] = (sal_uInt32)((long)pExtTextEvent);
    pEventData[1] = NULL;
#endif
    pEventData[2] = (sal_uInt32)nExtTextEventType;
    pEventData[3] = NULL;
    pEventData[4] = NULL;

    GetDisplay()->SendEvent (nEventAtom, pEventData, nFocusWindow);
}

void
SalFrameData::HandleExtTextEvent (XClientMessageEvent *pEvent)
{
    #if __SIZEOFLONG > 4
    void* pExtTextEvent = (void*)(  (pEvent->data.l[0] & 0xffffffff)
                                  | (pEvent->data.l[1] << 32) );
    #else
    void* pExtTextEvent = (void*)(pEvent->data.l[0]);
    #endif
    sal_uInt16 nExtTextEventType = sal_uInt16(pEvent->data.l[2]);

    Call(nExtTextEventType, pExtTextEvent);

    switch (nExtTextEventType)
    {
        case SALEVENT_ENDEXTTEXTINPUT:

            break;

        case SALEVENT_EXTTEXTINPUT:

            if (pExtTextEvent != NULL)
            {
                  SalExtTextInputEvent *pEvent = (SalExtTextInputEvent*)pExtTextEvent;

                if (pEvent->mpTextAttr != NULL)
                    free ((void*)pEvent->mpTextAttr);
                delete (pEvent);
            }
            break;

        default:

            fprintf(stderr, "SalFrameData::HandleExtTextEvent: invalid extended input\n");
    }
}
#endif /* defined(__synchronous_extinput__) */

// PostEvent
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
BOOL SalFrame::PostEvent( void *pData )
{
    _GetDisplay()->SendEvent( _GetDisplay()->getWMAdaptor()->getAtom( WMAdaptor::SAL_USEREVENT ),
                              pData,
                              maFrameData.GetWindow() );
    return TRUE;
}

// Title
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void SalFrame::SetTitle( const XubString& rTitle )
{
    _GetDisplay()->getWMAdaptor()->setWMName( this, rTitle );
}

// -----------------------------------------------------------------------

void SalFrame::Flush()
{
    XFlush( _GetDisplay()->GetDisplay() );
}

// -----------------------------------------------------------------------

void SalFrame::Sync()
{
    XSync( _GetDisplay()->GetDisplay(), False );
}

// Keyboard
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

// -----------------------------------------------------------------------

void SalFrame::SetInputContext( SalInputContext* pContext )
{
      if (pContext == NULL)
        return;

      // 1. We should create an input context for this frame
      //    only when SAL_INPUTCONTEXT_TEXT is set.

      if (!(pContext->mnOptions & SAL_INPUTCONTEXT_TEXT))
        return;

    // 2. We should use on-the-spot inputstyle
      //    only when SAL_INPUTCONTEXT_EXTTEXTINPUT is set.

      if (maFrameData.mpInputContext == NULL)
    {
        I18NStatus& rStatus( I18NStatus::get() );
        rStatus.setParent( this );
        maFrameData.mpInputContext = rStatus.getInputContext( maFrameData.mbDeleteInputContext );
        if (maFrameData.mpInputContext->UseContext())
        {
              maFrameData.mpInputContext->ExtendEventMask( maFrameData.GetShellWindow() );
              if (pContext->mnOptions & SAL_INPUTCONTEXT_CHANGELANGUAGE)
                maFrameData.mpInputContext->SetLanguage(pContext->meLanguage);
            if (maFrameData.mbInputFocus)
                maFrameData.mpInputContext->SetICFocus( this );
        }
      }
      return;
}

// -----------------------------------------------------------------------

void SalFrame::EndExtTextInput( USHORT nFlags )
{
      maFrameData.mpInputContext->EndExtTextInput( nFlags );
}

// -----------------------------------------------------------------------

XubString SalFrame::GetKeyName( USHORT nKeyCode )
{
    return _GetDisplay()->GetKeyName( nKeyCode );
}

XubString SalFrame::GetSymbolKeyName( const XubString&, USHORT nKeyCode )
{
  return GetKeyName( nKeyCode );
}

// Settings
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

inline Color getColorFromLong( long nColor )
{
    return Color( (nColor & 0xff), (nColor & 0xff00)>>8, (nColor & 0xff0000)>>16);
}

void SalFrame::UpdateSettings( AllSettings& rSettings )
{

    static SystemLookInfo aInfo;
    static BOOL bHaveInfo = FALSE;
    static BOOL bInit = FALSE;

    if( ! bInit )
    {
        bInit = TRUE;
        DtIntegrator* pIntegrator = DtIntegrator::CreateDtIntegrator( this );
        if( pIntegrator )
            bHaveInfo = pIntegrator->GetSystemLook( aInfo );
    }

    /*
     *  fill in mouse settings to paste on middle mouse click
     */
    MouseSettings aMouseSettings = rSettings.GetMouseSettings();
    aMouseSettings.SetMiddleButtonAction( MOUSE_MIDDLE_PASTESELECTION );
    rSettings.SetMouseSettings( aMouseSettings );

    if( bHaveInfo )
    {
        StyleSettings aStyleSettings = rSettings.GetStyleSettings();
        if( aInfo.windowActiveStart.GetColor() != COL_TRANSPARENT )
        {
            aStyleSettings.SetActiveColor( aInfo.windowActiveStart );
            if( aInfo.windowActiveEnd.GetColor() != COL_TRANSPARENT )
                aStyleSettings.SetActiveColor2( aInfo.windowActiveEnd );
        }
        if( aInfo.windowInactiveStart.GetColor() != COL_TRANSPARENT )
        {
            aStyleSettings.SetDeactiveColor( aInfo.windowInactiveStart );
            if( aInfo.windowInactiveEnd.GetColor() != COL_TRANSPARENT )
                aStyleSettings.SetDeactiveColor2( aInfo.windowInactiveEnd );
        }
        if( aInfo.activeBorder.GetColor() != COL_TRANSPARENT )
            aStyleSettings.SetActiveBorderColor( aInfo.activeBorder );
        if( aInfo.inactiveBorder.GetColor() != COL_TRANSPARENT )
            aStyleSettings.SetDeactiveBorderColor( aInfo.inactiveBorder );
        if( aInfo.activeForeground.GetColor() != COL_TRANSPARENT )
            aStyleSettings.SetActiveTextColor( aInfo.activeForeground );
        if( aInfo.inactiveForeground.GetColor() != COL_TRANSPARENT )
            aStyleSettings.SetDeactiveTextColor( aInfo.inactiveForeground );
        if( aInfo.selectForeground.GetColor() != COL_TRANSPARENT )
            aStyleSettings.SetHighlightTextColor( aInfo.selectForeground );
        if( aInfo.selectBackground.GetColor() != COL_TRANSPARENT )
            aStyleSettings.SetHighlightColor( aInfo.selectBackground );
        if( aInfo.foreground.GetColor() != COL_TRANSPARENT )
        {
            aStyleSettings.SetDialogTextColor( aInfo.foreground );
            aStyleSettings.SetMenuTextColor( aInfo.foreground );
            aStyleSettings.SetButtonTextColor( aInfo.foreground );
            aStyleSettings.SetRadioCheckTextColor( aInfo.foreground );
            aStyleSettings.SetGroupTextColor( aInfo.foreground );
            aStyleSettings.SetLabelTextColor( aInfo.foreground );
            aStyleSettings.SetInfoTextColor( aInfo.foreground );
        }
        if( aInfo.background.GetColor() != COL_TRANSPARENT )
        {
            aStyleSettings.Set3DColors( aInfo.background );
            aStyleSettings.SetFaceColor( aInfo.background );
            aStyleSettings.SetDialogColor( aInfo.background );
            aStyleSettings.SetMenuColor( aInfo.background );
            if ( aStyleSettings.GetFaceColor() == COL_LIGHTGRAY )
                aStyleSettings.SetCheckedColor( Color( 0xCC, 0xCC, 0xCC ) );
            else
            {
                // calculate Checked color
                Color   aColor2 = aStyleSettings.GetLightColor();
                BYTE    nRed    = (BYTE)(((USHORT)aInfo.background.GetRed()   + (USHORT)aColor2.GetRed())/2);
                BYTE    nGreen  = (BYTE)(((USHORT)aInfo.background.GetGreen() + (USHORT)aColor2.GetGreen())/2);
                BYTE    nBlue   = (BYTE)(((USHORT)aInfo.background.GetBlue()  + (USHORT)aColor2.GetBlue())/2);
                aStyleSettings.SetCheckedColor( Color( nRed, nGreen, nBlue ) );
            }
        }

        if( aInfo.windowFont.Len() )
        {
            Font aWindowFont = aStyleSettings.GetTitleFont();
            aWindowFont.SetName( aInfo.windowFont );
            aStyleSettings.SetTitleFont( aWindowFont );
        }

        rSettings.SetStyleSettings( aStyleSettings );
    }
}

// Sound
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void SalFrame::Beep( SoundType eSoundType ) // not fully suported
{ _GetDisplay()->Beep(); }

// Callback
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void SalFrame::SetCallback( void* pInst, SALFRAMEPROC pProc )
{
    maFrameData.pInst_ = pInst;
    maFrameData.pProc_ = pProc ? pProc : sal_CallbackDummy;
}

// Event Handling
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static USHORT sal_GetCode( int state )
{
    USHORT nCode = 0;

    if( state & Button1Mask )
        nCode |= MOUSE_LEFT;
    if( state & Button2Mask )
        nCode |= MOUSE_MIDDLE;
    if( state & Button3Mask )
        nCode |= MOUSE_RIGHT;

    if( state & ShiftMask )
        nCode |= KEY_SHIFT;
    if( state & ControlMask )
        nCode |= KEY_MOD1;
    if( state & Mod1Mask )
        nCode |= KEY_MOD2;

    return nCode;
}

long SalFrameData::HandleMouseEvent( XEvent *pEvent )
{
    SalMouseEvent       aMouseEvt;
    USHORT              nEvent;
    static ULONG        nLines = 0;

    // Solaris X86: clicking the right button on a two-button mouse
    // generates a button2 event not a button3 event
    if (pDisplay_->GetProperties() & PROPERTY_SUPPORT_3ButtonMouse )
    {
        switch (pEvent->type)
        {
            case EnterNotify:
            case LeaveNotify:
                if ( pEvent->xcrossing.state & Button2Mask )
                {
                    pEvent->xcrossing.state &= ~Button2Mask;
                    pEvent->xcrossing.state |=  Button3Mask;
                }
                break;

            case MotionNotify:
                if ( pEvent->xmotion.state & Button2Mask )
                {
                    pEvent->xmotion.state &= ~Button2Mask;
                    pEvent->xmotion.state |=  Button3Mask;
                }
                break;

            default:
                if ( Button2 == pEvent->xbutton.button )
                {
                    pEvent->xbutton.state &= ~Button2Mask;
                    pEvent->xbutton.state |=  Button3Mask;
                    pEvent->xbutton.button =  Button3;
                }
                break;
        }
    }


    if( LeaveNotify == pEvent->type || EnterNotify == pEvent->type )
    {
        aMouseEvt.mnX       = pEvent->xcrossing.x;
        aMouseEvt.mnY       = pEvent->xcrossing.y;
        aMouseEvt.mnTime    = pEvent->xcrossing.time;
        /*
         *  #89075# #89335# *sigh*
         *  commented out:
         *  aMouseEvt.mnCode    = sal_GetCode( pEvent->xcrossing.state );
         *
         *  some WMs (and/or) applications  have a passive grab on
         *  mouse buttons (XGrabButton). This leads to enter/leave notifies
         *  with mouse buttons pressed in the state mask before the actual
         *  ButtonPress event gets dispatched. But EnterNotify
         *  is reported in vcl as MouseMove event. Some office code
         *  decides that a pressed button in a MouseMove belongs to
         *  a drag operation which leads to doing things differently.
         *
         *  hopefully this workaround will not break anything.
         */
        aMouseEvt.mnCode    = 0;
        aMouseEvt.mnButton  = 0;

        nEvent              = LeaveNotify == pEvent->type
                              ? SALEVENT_MOUSELEAVE
                              : SALEVENT_MOUSEMOVE;
    }
    else if( pEvent->type == MotionNotify )
    {
        aMouseEvt.mnX       = pEvent->xmotion.x;
        aMouseEvt.mnY       = pEvent->xmotion.y;
        aMouseEvt.mnTime    = pEvent->xmotion.time;
        aMouseEvt.mnCode    = sal_GetCode( pEvent->xmotion.state );

        aMouseEvt.mnButton  = 0;

        nEvent              = SALEVENT_MOUSEMOVE;
    }
    else
    {
        // get input focus on SAL_FRAME_STYLE_CHILD windows
        // because the focus handling in this case (running as plugin)
        // is "a little tricky"
        // on some wm however, this leads to closing our menues before the menu event
        // was fired (#89867), so we cancel this now
          //if( nStyle_ & SAL_FRAME_STYLE_CHILD )
        //XSetInputFocus( GetDisplay()->GetDisplay(), GetWindow(), RevertToParent, CurrentTime );
        if( pEvent->xbutton.button == Button1 ||
            pEvent->xbutton.button == Button2 ||
            pEvent->xbutton.button == Button3 )
        {
            aMouseEvt.mnX       = pEvent->xbutton.x;
            aMouseEvt.mnY       = pEvent->xbutton.y;
            aMouseEvt.mnTime    = pEvent->xbutton.time;
            aMouseEvt.mnCode    = sal_GetCode( pEvent->xbutton.state );

            if( Button1 == pEvent->xbutton.button )
                aMouseEvt.mnButton  = MOUSE_LEFT;
            else if( Button2 == pEvent->xbutton.button )
                aMouseEvt.mnButton  = MOUSE_MIDDLE;
            else if( Button3 == pEvent->xbutton.button )
                aMouseEvt.mnButton  = MOUSE_RIGHT;

            nEvent              = ButtonPress == pEvent->type
                ? SALEVENT_MOUSEBUTTONDOWN
                : SALEVENT_MOUSEBUTTONUP;
        }
        else if( pEvent->xbutton.button == Button4 ||
            pEvent->xbutton.button == Button5 )
        {
            if( ! nLines )
            {
                char* pEnv = getenv( "SAL_WHEELLINES" );
                nLines = pEnv ? atoi( pEnv ) : 3;
                if( nLines > 10 )
                    nLines = SAL_WHEELMOUSE_EVENT_PAGESCROLL;
            }

            SalWheelMouseEvent  aWheelEvt;
            aWheelEvt.mnTime        = pEvent->xbutton.time;
            aWheelEvt.mnX           = pEvent->xbutton.x;
            aWheelEvt.mnY           = pEvent->xbutton.y;
            aWheelEvt.mnDelta       =
                pEvent->xbutton.button == Button4 ? 120 : -120;
            aWheelEvt.mnNotchDelta  =
                pEvent->xbutton.button == Button4 ? 1 : -1;
            aWheelEvt.mnScrollLines = nLines;
            aWheelEvt.mnCode        = sal_GetCode( pEvent->xbutton.state );
            aWheelEvt.mbHorz        = FALSE;

            nEvent = SALEVENT_WHEELMOUSE;

            return Call( nEvent, &aWheelEvt );
        }
    }

    if( nEvent == SALEVENT_MOUSELEAVE
        || ( aMouseEvt.mnX <  nWidth_  && aMouseEvt.mnX >  -1 &&
             aMouseEvt.mnY <  nHeight_ && aMouseEvt.mnY >  -1 )
        || pDisplay_->MouseCaptured( this ) )
        return Call( nEvent, &aMouseEvt );

#ifdef DBG_UTIL
    fprintf( stderr, "SalFrameData::HandleMouseEvent %d size=%d*%d event=%d.%d\n",
             pEvent->type, nWidth_, nHeight_, aMouseEvt.mnX, aMouseEvt.mnY );
#endif
    return 0;
}

//
// The eventhandler member functions may indirectly call their own destructor.
// So make sure to be notified of that case to not to touch any member in the
// rest of the eventhandler.
//
void
SalFrameData::RegisterDeleteData (SalFrameDelData *pData)
{
    pData->SetNext (mpDeleteData);
    mpDeleteData = pData;
}
void
SalFrameData::NotifyDeleteData ()
{
    for (SalFrameDelData* pData = mpDeleteData; pData != NULL; pData = pData->GetNext())
        pData->Delete();
}
void
SalFrameData::UnregisterDeleteData (SalFrameDelData *pData)
{
    if (mpDeleteData == pData)
    {
        mpDeleteData = pData->GetNext ();
    }
    else
    {
        SalFrameDelData* pList = mpDeleteData;
        while (pList->GetNext() != pData)
            pList = pList->GetNext ();
        pList->SetNext (pData->GetNext());
    }
}

// F10 means either KEY_F10 or KEY_MENU, which has to be decided
// in the independent part.
struct KeyAlternate
{
    USHORT          nKeyCode;
    sal_Unicode     nCharCode;
    KeyAlternate() : nKeyCode( 0 ), nCharCode( 0 ) {}
    KeyAlternate( USHORT nKey, sal_Unicode nChar = 0 ) : nKeyCode( nKey ), nCharCode( nChar ) {}
};

inline KeyAlternate
GetAlternateKeyCode( const USHORT nKeyCode )
{
    KeyAlternate aAlternate;

    switch( nKeyCode )
    {
        case KEY_F10: aAlternate = KeyAlternate( KEY_MENU );break;
        case KEY_F24: aAlternate = KeyAlternate( KEY_SUBTRACT, '-' );break;
    }

    return aAlternate;
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
long SalFrameData::HandleKeyEvent( XKeyEvent *pEvent )
{
    KeySym          nKeySym;
    int             nLen = 2048;
    unsigned char   *pPrintable = (unsigned char*)alloca( nLen );

    // singlebyte code composed by input method, the new default
    if (mpInputContext != NULL && mpInputContext->UseContext())
    {
        // returns a keysym as well as the pPrintable (in system encoding)
        // printable may be empty.
        Status nStatus;
        nKeySym = pDisplay_->GetKeySym( pEvent, pPrintable, &nLen,
                &nStatus, mpInputContext->GetContext() );
        if ( nStatus == XBufferOverflow )
        {
            nLen *= 2;
            pPrintable = (unsigned char*)alloca( nLen );
            nKeySym = pDisplay_->GetKeySym( pEvent, pPrintable, &nLen,
                    &nStatus, mpInputContext->GetContext() );
        }
    }
    else
    {
        // fallback, this should never ever be called
        Status nStatus = 0;
           nKeySym = pDisplay_->GetKeySym( pEvent, pPrintable, &nLen, &nStatus );
     }

    USHORT nModCode = 0;
    if( pEvent->state & ShiftMask )
        nModCode |= KEY_SHIFT;
    if( pEvent->state & ControlMask )
        nModCode |= KEY_MOD1;
    if( pEvent->state & Mod1Mask )
    {
        nModCode |= KEY_MOD2;
        if ( !(nModCode & KEY_MOD1) )
            nModCode |= KEY_CONTROLMOD;
    }

    if(     nKeySym == XK_Shift_L   || nKeySym == XK_Shift_R
        ||  nKeySym == XK_Control_L || nKeySym == XK_Control_R
        ||  nKeySym == XK_Alt_L     || nKeySym == XK_Alt_R
        ||  nKeySym == XK_Meta_L    || nKeySym == XK_Meta_R )
    {
        SalKeyModEvent aModEvt;

        // pressing just the ctrl key leads to a keysym of XK_Control but
        // the event state does not contain ControlMask. In the release
        // event its the other way round: it does contain the Control mask.
        // The modifier mode therefor has to be adapted manually.
        if (pEvent->type == KeyRelease)
        {
            if ( (nKeySym == XK_Control_L) || (nKeySym == XK_Control_R) )
                nModCode &= ~KEY_MOD1;
            if ( (nKeySym == XK_Shift_L)   || (nKeySym == XK_Shift_R) )
                nModCode &= ~KEY_SHIFT;
            if ( (nKeySym == XK_Alt_L)     || (nKeySym == XK_Alt_R) )
                nModCode &= ~KEY_MOD2;
        }
        else
        {
            if ( (nKeySym == XK_Control_L) || (nKeySym == XK_Control_R) )
                nModCode |= KEY_MOD1;
            if ( (nKeySym == XK_Shift_L)   || (nKeySym == XK_Shift_R) )
                nModCode |= KEY_SHIFT;
            if ( (nKeySym == XK_Alt_L)     || (nKeySym == XK_Alt_R) )
                nModCode |= KEY_MOD2;
        }

        aModEvt.mnCode = nModCode;
        aModEvt.mnTime = pEvent->time;

        return Call( SALEVENT_KEYMODCHANGE, &aModEvt );
    }

    SalKeyEvent aKeyEvt;
    USHORT      nKeyCode;
    char        aDummy;

    // try to figure out the vcl code for the keysym
    nKeyCode = pDisplay_->GetKeyCode( nKeySym, &aDummy );
    // try to figure out a printable if XmbLookupString returns only a keysym
    // and NOT a printable. Do not store it in pPrintable[0] since it is expected to
    // be in system encoding, not unicode.
    sal_Unicode nKeyString = 0x0;
    if (nLen == 0)
        nKeyString = KeysymToUnicode (nKeySym);
    // if we have nothing we give up
    if( !nKeyCode && !nLen && !nKeyString)
        return 0;

    rtl_TextEncoding nEncoding = osl_getThreadTextEncoding();
    sal_Unicode *pBuffer;
    sal_Unicode *pString;
    sal_Size     nBufferSize = nLen * 2;
    sal_Size     nSize;
    pBuffer = (sal_Unicode*) malloc( nBufferSize + 2 );
    pBuffer[ 0 ] = 0;
    if (nLen > 0 && nEncoding != RTL_TEXTENCODING_UNICODE)
    {
        // create text converter
        rtl_TextToUnicodeConverter aConverter =
                rtl_createTextToUnicodeConverter( nEncoding );
        rtl_TextToUnicodeContext aContext =
                 rtl_createTextToUnicodeContext( aConverter );

        sal_uInt32  nConversionInfo;
        sal_Size    nConvertedChars;

        // convert to single byte text stream
        nSize = rtl_convertTextToUnicode(
                aConverter, aContext,
                (char*)pPrintable, nLen,
                pBuffer, nBufferSize,
                  RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_IGNORE
                | RTL_TEXTTOUNICODE_FLAGS_INVALID_IGNORE,
                &nConversionInfo, &nConvertedChars );

        // destroy converter
        rtl_destroyTextToUnicodeContext( aConverter, aContext );
        rtl_destroyTextToUnicodeConverter( aConverter );

        pString = pBuffer;
    }
    else
    if (nLen > 0 /* nEncoding == RTL_TEXTENCODING_UNICODE */)
    {
        pString = (sal_Unicode*)pPrintable;
          nSize = nLen;
    }
    else
    /* if (nKeyString != 0) */
    {
        pString = &nKeyString;
        nSize = 1;
    }

    SalFrameDelData aDeleteWatch;
    RegisterDeleteData (&aDeleteWatch);

    if (   mpInputContext != NULL
        && mpInputContext->UseContext()
        && KeyRelease != pEvent->type
        && (   (nSize >  1)
            || (nSize >  0 && mpInputContext->IsPreeditMode())) )
    {
        mpInputContext->CommitKeyEvent(pString, nSize);
    }
    else
    // normal single character keyinput
    {
        aKeyEvt.mnCode     = nKeyCode | nModCode;
        aKeyEvt.mnRepeat   = 0;
        aKeyEvt.mnTime     = pEvent->time;
        aKeyEvt.mnCharCode = pString[ 0 ];

        if( KeyRelease == pEvent->type )
        {
            Call( SALEVENT_KEYUP, &aKeyEvt );
        }
        else
        {
            if ( ! Call(SALEVENT_KEYINPUT, &aKeyEvt) )
            {
                // independent layer doesnt want to handle key-event, so check
                // whether the keycode may have an alternate meaning
                KeyAlternate aAlternate = GetAlternateKeyCode( nKeyCode );
                if ( aAlternate.nKeyCode != 0 )
                {
                    aKeyEvt.mnCode = aAlternate.nKeyCode | nModCode;
                    if( aAlternate.nCharCode )
                        aKeyEvt.mnCharCode = aAlternate.nCharCode;
                    Call(SALEVENT_KEYINPUT, &aKeyEvt);
                }
                else
                if (pEvent->keycode != 0)
                {
                    // try to strip off modifiers, e.g. Ctrl-$ becomes Ctrl-Shift-4
                    nKeySym  = XKeycodeToKeysym (pDisplay_->GetDisplay(),
                                                 pEvent->keycode, 0);
                    nKeyCode = pDisplay_->GetKeyCode(nKeySym, &aDummy);
                    if ((nKeyCode != 0) && ((nKeyCode | nModCode) != aKeyEvt.mnCode))
                    {
                        aKeyEvt.mnCode = nKeyCode | nModCode;
                        Call(SALEVENT_KEYINPUT, &aKeyEvt);
                    }
                }
            }
        }
    }

      //
      // update the spot location for PreeditPosition IME style
      //
    if (! aDeleteWatch.IsDeleted())
    {
        if (mpInputContext != NULL && mpInputContext->UseContext())
            mpInputContext->UpdateSpotLocation();

        UnregisterDeleteData (&aDeleteWatch);
    }

    free (pBuffer);
    return True;
}


// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
long SalFrameData::HandleFocusEvent( XFocusChangeEvent *pEvent )
{
    /*  #55691# ignore focusout resulting from keyboard grabs
     *  we do not grab it and are not interested when
     *  someone else does CDE e.g. does a XGrabKey on arrow keys
     *  #73179# handle focus events with mode NotifyWhileGrabbed
     *  because with CDE alt-tab focus changing we do not get
     *  normal focus events
     *  #71791# cast focus event to the input context, otherwise the
     *  status window does not follow the application frame
     */

    if ( mpInputContext != NULL  )
    {
        if( FocusIn == pEvent->type )
            mpInputContext->SetICFocus( pFrame_ );
        else
            mpInputContext->UnsetICFocus( pFrame_ );
    }

    if ( pEvent->mode == NotifyNormal || pEvent->mode == NotifyWhileGrabbed )
    {
        if( FocusIn == pEvent->type )
        {
#ifdef USE_PSPRINT
            if( GetSalData()->pFirstInstance_->maInstData.mbPrinterInit )
            {
                ::psp::PrinterInfoManager& rManager( ::psp::PrinterInfoManager::get() );
                if( rManager.checkPrintersChanged() )
                {
                    SalFrame* pFrame = GetSalData()->pFirstFrame_;
                    while( pFrame )
                    {
                        pFrame->maFrameData.Call( SALEVENT_PRINTERCHANGED, NULL );
                        pFrame = pFrame->maFrameData.GetNextFrame();
                    }
                }
            }
#endif
            mbInputFocus = True;
            return Call( SALEVENT_GETFOCUS,  0 );
        }
        else
        {
            mbInputFocus = False;
            return Call( SALEVENT_LOSEFOCUS, 0 );
        }
    }

    return 0;
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
long SalFrameData::HandleExposeEvent( XEvent *pEvent )
{
    XRectangle  aRect;
    USHORT      nCount = 0;

    if( pEvent->type == Expose )
    {
        aRect.x         = pEvent->xexpose.x;
        aRect.y         = pEvent->xexpose.y;
        aRect.width     = pEvent->xexpose.width;
        aRect.height    = pEvent->xexpose.height;
        nCount          = pEvent->xexpose.count;
    }
    else if( pEvent->type == GraphicsExpose )
    {
        aRect.x         = pEvent->xgraphicsexpose.x;
        aRect.y         = pEvent->xgraphicsexpose.y;
        aRect.width     = pEvent->xgraphicsexpose.width;
        aRect.height    = pEvent->xgraphicsexpose.height;
        nCount          = pEvent->xgraphicsexpose.count;
    }

    if( IsOverrideRedirect() && ! aRestoreFullScreen_.IsEmpty() )
        // we are in fullscreen mode -> override redirect
         // focus is probably lost, so reget it
         XSetInputFocus( GetXDisplay(), XtWindow( hShell_ ), RevertToNone, CurrentTime );

    // width and height are extents, so they are of by one for rectangle
    maPaintRegion.Union( Rectangle( Point(aRect.x, aRect.y), Size(aRect.width+1, aRect.height+1) ) );

    if( nCount || maResizeTimer.IsActive() ) // wait for last expose rectangle
        return 1;

    SalPaintEvent aPEvt;

    aPEvt.mnBoundX          = maPaintRegion.Left();
    aPEvt.mnBoundY          = maPaintRegion.Top();
    aPEvt.mnBoundWidth      = maPaintRegion.GetWidth();
    aPEvt.mnBoundHeight     = maPaintRegion.GetHeight();

    Call( SALEVENT_PAINT, &aPEvt );
    maPaintRegion = Rectangle();

    return 1;
}

void SalFrameData::RepositionFloatChildren()
{
    // move SAL_FRAME_STYLE_FLOAT children to new position
    for( int nChild = 0; nChild < maChildren.Count(); nChild++ )
    {
        SalFrameData* pData = &maChildren.GetObject( nChild )->maFrameData;
        if( pData->nStyle_ & SAL_FRAME_STYLE_FLOAT )
        {
#ifdef DEBUG
            fprintf( stderr, "moving FLOAT child\n" );
#endif
            pData->SetPosSize( pData->aPosSize_ );
        }
    }
}

void SalFrameData::RepositionChildren()
{
    RepositionFloatChildren();
    int nChild;
    for( nChild = 0; nChild < maChildren.Count(); nChild++ )
    {
        SalFrameData* pData = &maChildren.GetObject( nChild )->maFrameData;
        if( pData->bMapped_ )
            XRaiseWindow( GetXDisplay(), pData->GetStackingWindow() ? pData->GetStackingWindow() : pData->GetShellWindow() );
    }
    for( nChild = 0; nChild < maChildren.Count(); nChild++ )
    {
        SalFrameData* pData = &maChildren.GetObject( nChild )->maFrameData;
        pData->RepositionChildren();
    }
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
long SalFrameData::HandleSizeEvent( XConfigureEvent *pEvent )
{
    if (   pEvent->window != XtWindow( hShell_ )
        && pEvent->window != XtWindow( hComposite_ )
        && pEvent->window != hForeignParent_
        && pEvent->window != hForeignTopLevelWindow_ )
    {
        // could be as well a sys-child window (aka SalObject)
        return 1;
    }

     // ignore for now
     if( nStyle_ & SAL_FRAME_STYLE_FLOAT )
         return 1;

    if( pEvent->window == hForeignTopLevelWindow_ )
    {
        // just update the children's positions
        RepositionChildren();
        return 1;
    }

    if( pEvent->window == hForeignParent_ )
    {
        Arg args[10];
           int n = 0;
        XtSetArg(args[n], XtNheight, pEvent->height);   n++;
        XtSetArg(args[n], XtNwidth,  pEvent->width);    n++;
        XtSetValues( hShell_, args, n );
    }
    if( SHOWSTATE_UNKNOWN == nShowState_ )
    {
        nShowState_ = SHOWSTATE_NORMAL;

        XSizeHints  hints;
        long        supplied;
        if( XGetWMNormalHints( pEvent->display,
                               pEvent->window,
                               &hints,
                               &supplied ) )
        {
            if( hints.flags & PMaxSize ) // supplied
            {
                nMaxWidth_  = hints.max_width;
                nMaxHeight_ = hints.max_height;
                DBG_ASSERT( nMaxWidth_ && nMaxHeight_, "!MaxWidth^!MaxHeight" )
            }
            hints.flags |= PWinGravity;
            hints.win_gravity = StaticGravity;
            XSetWMNormalHints( pEvent->display,
                               GetShellWindow(),
                               &hints );
            XSync( pEvent->display, False );
        }
    }

    XLIB_Window hDummy;
    XTranslateCoordinates( GetXDisplay(),
                           GetWindow(),
                           pDisplay_->GetRootWindow(),
                           0, 0,
                           &pEvent->x, &pEvent->y,
                           &hDummy );

    maResizeBuffer.SetPos( Point( pEvent->x, pEvent->y ) );
    maResizeBuffer.SetSize( Size( pEvent->width, pEvent->height ) );

    if( nWidth_ != pEvent->width || nHeight_ != pEvent->height )
    {
        nWidth_     = pEvent->width;
        nHeight_    = pEvent->height;

        if( pEvent->window != XtWindow( hComposite_ ) )
            XtResizeWidget(hComposite_, nWidth_, nHeight_, 0);

        maResizeTimer.Start();
    }
    return 1;
}

IMPL_LINK( SalFrameData, HandleResizeTimer, void*, pDummy )
{
    aPosSize_ = maResizeBuffer;
    // update children's position
    RepositionChildren();

    Call( SALEVENT_RESIZE, NULL );

    SalPaintEvent aPEvt;

    aPEvt.mnBoundX          = maPaintRegion.Left();
    aPEvt.mnBoundY          = maPaintRegion.Top();
    aPEvt.mnBoundWidth      = maPaintRegion.GetWidth();
    aPEvt.mnBoundHeight     = maPaintRegion.GetHeight();

    Call( SALEVENT_PAINT, &aPEvt );
    maPaintRegion = Rectangle();

    return 0;
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
long SalFrameData::HandleReparentEvent( XReparentEvent *pEvent )
{
    Display        *pDisplay   = pEvent->display;
    XLIB_Window     hWM_Parent;
    XLIB_Window     hRoot, *Children, hDummy;
    unsigned int    nChildren, n;
    BOOL            bNone = pDisplay_->GetProperties()
                            & PROPERTY_SUPPORT_WM_Parent_Pixmap_None;
    static const char* pDisableStackingCheck = getenv( "SAL_DISABLE_STACKING_CHECK" );

    /*
     *  #89186# don't rely on the new parent from the event.
     *  the event may be "out of date", that is the window manager
     *  window may not exist anymore. This can happen if someone
     *  shows a frame and hides it again quickly (not that that would
     *  be very sensible)
     */
    hWM_Parent = GetShellWindow();
    do
    {
        XQueryTree( pDisplay,
                    hWM_Parent,
                    &hRoot,
                    &hDummy,
                    &Children,
                    &nChildren );
        if( hDummy != hRoot )
        {
            hWM_Parent = hDummy;
            if( bNone )
                XSetWindowBackgroundPixmap( pDisplay, hWM_Parent, None );
        }
        if( Children )
            XFree( Children );
    } while( hDummy != hRoot );

    if( hStackingWindow_ == None && ( ! pDisableStackingCheck || ! *pDisableStackingCheck ) )
    {
        hStackingWindow_ = hWM_Parent;
        XSelectInput( pDisplay, hStackingWindow_, StructureNotifyMask );
    }

    if(     hWM_Parent == pDisplay_->GetRootWindow()
        ||  hWM_Parent == hForeignParent_
        ||  pEvent->parent == pDisplay_->GetRootWindow()
        || ( nStyle_ & SAL_FRAME_STYLE_FLOAT ) )
    {
        // Reparenting before Destroy
        hStackingWindow_ = None;
        return 0;
    }

#ifdef NC_EVENTS
    XQueryTree( pDisplay,
                hWM_Parent,
                &hRoot,
                &hDummy,
                &Children,
                &nChildren );

#ifdef DBG_UTIL
    if( hDummy != hRoot )
    {
        fprintf( stderr,
                 "SalFrameData::HandleReparentEvent hDummy!=hRoot"
                 " r=%ld d=%ld p=%ld n=%d\n",
                 hRoot, hDummy, hWM_Parent, nChildren );
        pDisplay_->PrintEvent( "HandleReparentEvent", (XEvent*)pEvent );
    }
#endif
    for( n = 0; n < nChildren; n++ )
        if( Children[n] != hShell_ && Children[n] != pEvent->parent )
            XSelectInput( pDisplay, Children[n], NC_EVENTS );
    XFree( Children );
#endif

    if( !XTranslateCoordinates( GetXDisplay(),
                                XtWindow( hShell_ ),
                                hWM_Parent,
                                0, 0,
                                &nLeft_, &nTop_,
                                &hDummy ) )
    {
        nLeft_ = nTop_ = 0;
    }

    /*
     *  decorations are not symmetric,
     *  so need real geometries here
     *  (this will fail with virtual roots ?)
     */
    int xp, yp, x, y;
    unsigned int wp, w, hp, h, bw, d;
    XGetGeometry( GetXDisplay(),
                  GetShellWindow(),
                  &hRoot,
                  &x, &y, &w, &h, &bw, &d );
    XGetGeometry( GetXDisplay(),
                  hWM_Parent,
                  &hRoot,
                  &xp, &yp, &wp, &hp, &bw, &d );
    nRight_     = wp - w - nLeft_;
    nBottom_    = hp - h - nTop_;

    XSizeHints  hints;
    XSizeHints  supplied;
    if( XGetWMNormalHints( pEvent->display,
                pEvent->window,
                &hints,
                (long*)&supplied ) )
    {
        if( hints.flags & PMaxSize ) // supplied
        {
            nMaxWidth_  = hints.max_width;
            nMaxHeight_ = hints.max_height;
            DBG_ASSERT( nMaxWidth_ && nMaxHeight_, "!MaxWidth^!MaxHeight" )
        }
        hints.flags |= PWinGravity;
        hints.win_gravity = StaticGravity;
        XSetWMNormalHints( pEvent->display,
                GetShellWindow(),
                &hints );
        XSync( pEvent->display, False );
    }

    if( (aPosSize_.IsEmpty() || WindowNeedGoodPosition( XtWindow( hShell_ ) ) )
        && pDisplay_->GetProperties() & PROPERTY_FEATURE_Maximize )
    {
        nShowState_ = SHOWSTATE_NORMAL;
        Maximize();
        aRestoreFullScreen_ = aPosSize_;
    }
    else
    {
        // limit width and height if we are too large: #47757
        // olwm and fvwm need this, it doesnt harm the rest

        int nScreenWidth  = pDisplay_->GetScreenSize().Width();
        int nScreenHeight = pDisplay_->GetScreenSize().Height();
        int nFrameWidth   = aPosSize_.GetWidth()  + nLeft_ + nRight_;
        int nFrameHeight  = aPosSize_.GetHeight() + nTop_  + nBottom_;

        if ((nFrameWidth > nScreenWidth) || (nFrameHeight > nScreenHeight))
        {
            Size aSize(aPosSize_.GetWidth(), aPosSize_.GetHeight());

            if (nFrameWidth  > nScreenWidth)
                aSize.Width()  = nScreenWidth  - nRight_  - nLeft_;
            if (nFrameHeight > nScreenHeight)
                aSize.Height() = nScreenHeight - nBottom_ - nTop_;

            SetSize (aSize);
        }
    }


    return 1;
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
long SalFrameData::HandleColormapEvent( XColormapEvent *pEvent )
{
    return 0;
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
long SalFrameData::HandleStateEvent( XPropertyEvent *pEvent )
{
    Atom          actual_type;
    int           actual_format;
    unsigned long nitems, bytes_after;
    unsigned char *prop = NULL;

    if( 0 != XGetWindowProperty( GetXDisplay(),
                                 XtWindow( hShell_ ),
                                 pEvent->atom,          // property
                                 0,                     // long_offset (32bit)
                                 2,                     // long_length (32bit)
                                 False,                 // delete
                                 pEvent->atom,          // req_type
                                 &actual_type,
                                 &actual_format,
                                 &nitems,
                                 &bytes_after,
                                 &prop )
        || ! prop
        )
        return 0;

    DBG_ASSERT( actual_type = pEvent->atom
                && 32 == actual_format
                &&  2 == nitems
                &&  0 == bytes_after, "HandleStateEvent" )

    if( *(unsigned long*)prop == NormalState )
        nShowState_ = SHOWSTATE_NORMAL;
    else if( *(unsigned long*)prop == IconicState )
        nShowState_ = SHOWSTATE_MINIMIZED;

    XFree( prop );
    return 1;
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
long SalFrameData::HandleClientMessage( XClientMessageEvent *pEvent )
{
    const WMAdaptor& rWMAdaptor( *pDisplay_->getWMAdaptor() );

    if( pEvent->message_type == rWMAdaptor.getAtom( WMAdaptor::SAL_USEREVENT ) )
    {
#if __SIZEOFLONG > 4
        void* pData = (void*)
            ( (pEvent->data.l[0] & 0xffffffff) | (pEvent->data.l[1] << 32) );
#else
        void* pData = (void*)(pEvent->data.l[0]);
#endif
        Call( SALEVENT_USEREVENT, pData );
        return 1;
    }
#if !defined(__synchronous_extinput__)
    if( pEvent->message_type == rWMAdaptor.getAtom( WMAdaptor::SAL_EXTTEXTEVENT ) )
    {
        HandleExtTextEvent (pEvent);
        return 1;
    }
#endif
    else if( pEvent->message_type == rWMAdaptor.getAtom( WMAdaptor::SAL_QUITEVENT ) )
    {
        stderr0( "SalFrameData::Dispatch Quit\n" );
        Close(); // ???
        return 1;
    }
    else if( pEvent->message_type == rWMAdaptor.getAtom( WMAdaptor::WM_PROTOCOLS ) )
    {
        if( pEvent->data.l[0] == rWMAdaptor.getAtom( WMAdaptor::WM_DELETE_WINDOW ) )
        {
            stderr0( "SalFrameData::Dispatch DeleteWindow\n" );
            Close();
            return 1;
        }
        else if( pEvent->data.l[0] == rWMAdaptor.getAtom( WMAdaptor::WM_SAVE_YOURSELF ) )
        {
            SalFrame* pLast = GetSalData()->pFirstFrame_;
            while( pLast->maFrameData.pNextFrame_ )
                pLast = pLast->maFrameData.pNextFrame_;
            if( pLast == pFrame_ )
            {
                ByteString aExec( SessionManagerClient::getExecName(), osl_getThreadTextEncoding() );
                char* argv[2];
                argv[0] = "/bin/sh";
                argv[1] = const_cast<char*>(aExec.GetBuffer());
                XSetCommand( GetXDisplay(), XtWindow( hShell_ ), argv, 2 );
            }
            else
                XDeleteProperty( GetXDisplay(), XtWindow( hShell_ ), rWMAdaptor.getAtom( WMAdaptor::WM_COMMAND ) );
        }
    }
    return 0;
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

Bool SalFrameData::checkKeyReleaseForRepeat( Display* pDisplay, XEvent* pCheck, XPointer pSalFrameData )
{
    SalFrameData* pThis = (SalFrameData*)pSalFrameData;
    return
        pCheck->type            == XLIB_KeyPress &&
        pCheck->xkey.state      == pThis->nKeyState_ &&
        pCheck->xkey.keycode    == pThis->nKeyCode_ &&
        pCheck->xkey.time       == pThis->nReleaseTime_  ? True : False;
}

long SalFrameData::Dispatch( XEvent *pEvent )
{
    long nRet = 0;

    if( -1 == nCaptured_ )
    {
        CaptureMouse( TRUE );
#ifdef DBG_UTIL
        if( -1 != nCaptured_ )
            pDisplay_->PrintEvent( "Captured", pEvent );
#endif
    }

    if( pEvent->xany.window == XtWindow( hShell_ ) || pEvent->xany.window == XtWindow( hComposite_ ) )
    {
        switch( pEvent->type )
        {
            case XLIB_KeyPress:
                nKeyCode_   = pEvent->xkey.keycode;
                nKeyState_  = pEvent->xkey.state;
                nRet        = HandleKeyEvent( &pEvent->xkey );
                break;

            case KeyRelease:
                if( -1 == nCompose_ )
                {
                    nReleaseTime_ = pEvent->xkey.time;
                    XEvent aEvent;
                    if( XCheckIfEvent( pEvent->xkey.display, &aEvent, checkKeyReleaseForRepeat, (XPointer)this ) )
                        XPutBackEvent( pEvent->xkey.display, &aEvent );
                    else
                        nRet        = HandleKeyEvent( &pEvent->xkey );
                }
            break;

            case ButtonPress:
                // #74406# if we loose the focus in presentation mode
                // there are good chances that we never get it back
                // since the WM ignores us
                 if( IsOverrideRedirect() )
                 {
                     XSetInputFocus( GetXDisplay(), GetShellWindow(),
                             RevertToNone, CurrentTime );
                 }

            case ButtonRelease:
            case MotionNotify:
            case EnterNotify:
            case LeaveNotify:
                nRet = HandleMouseEvent( pEvent );
                break;

            case FocusIn:
            case FocusOut:
                nRet = HandleFocusEvent( &pEvent->xfocus );
                break;

            case Expose:
            case GraphicsExpose:
                nRet = HandleExposeEvent( pEvent );
                break;

            case MapNotify:
                if( pEvent->xmap.window == GetShellWindow() )
                {
                    bMapped_   = TRUE;
                    bViewable_ = TRUE;
                    nRet = TRUE;
                    if ( mpInputContext != NULL )
                        mpInputContext->Map( pFrame_ );
                    Call( SALEVENT_RESIZE, NULL );
                    if( pDisplay_->GetServerVendor() == vendor_hummingbird )
                    {
                        /*
                         *  With Exceed sometimes there does not seem to be
                         *  an Expose after the MapNotify.
                         *  so start a paint via the timer here
                         *  to avoid duplicate paints
                         */
                        maPaintRegion.Union( Rectangle( Point( 0, 0 ), aPosSize_.GetSize() ) );
                        if( ! maResizeTimer.IsActive() )
                        {
                            maResizeBuffer = aPosSize_;
                            maResizeTimer.Start();
                        }
                    }
                }
                break;

            case UnmapNotify:
                if( pEvent->xunmap.window == XtWindow( hShell_ ) )
                {
                    bMapped_   = FALSE;
                    bViewable_ = FALSE;
                    nRet = TRUE;
                    if ( mpInputContext != NULL )
                        mpInputContext->Unmap( pFrame_ );
                    Call( SALEVENT_RESIZE, NULL );
                }
                break;

            case ConfigureNotify:
                if( pEvent->xconfigure.window == XtWindow( hShell_ )        ||
                    pEvent->xconfigure.window == XtWindow( hComposite_ ) )
                    nRet = HandleSizeEvent( &pEvent->xconfigure );
                break;

            case VisibilityNotify:
                nVisibility_ = pEvent->xvisibility.state;
                nRet = TRUE;
            break;

            case ReparentNotify:
                nRet = HandleReparentEvent( &pEvent->xreparent );
                break;

            case MappingNotify:
                if( MappingPointer != pEvent->xmapping.request )
                    nRet = Call( SALEVENT_KEYBOARDCHANGED, 0 );
                break;

            case ColormapNotify:
                nRet = HandleColormapEvent( &pEvent->xcolormap );
                break;

            case PropertyNotify:
            {
                if( pEvent->xproperty.atom == pDisplay_->getWMAdaptor()->getAtom( WMAdaptor::WM_STATE ) )
                    nRet = HandleStateEvent( &pEvent->xproperty );
                else
                    nRet = 1;
                break;
            }

            case ClientMessage:
                nRet = HandleClientMessage( &pEvent->xclient );
                break;
        }
    }
    else
    {
        switch( pEvent->type )
        {
#ifdef NC_EVENTS
            case XLIB_KeyPress:
            case KeyRelease:
                nRet = HandleNCKeyEvent()
                break;

            case ButtonPress:
            case ButtonRelease:
            case MotionNotify:
            case EnterNotify:
            case LeaveNotify:
                nRet = HandleNCMouseEvent( pEvent );
                break;

            case Expose:
                nRet = HandleNCExposeEvent( pEvent );
                break;
#endif
            case ConfigureNotify:
                if( pEvent->xconfigure.window == hForeignParent_ ||
                    pEvent->xconfigure.window == hForeignTopLevelWindow_ )
                    nRet = HandleSizeEvent( &pEvent->xconfigure );

                if( pEvent->xconfigure.window == hStackingWindow_ )
                {
                    // update position here as well as in HandleSizeEvent
                    // if the window is only moved this is the only
                    // chance to notice that.
                    XLIB_Window hDummy;
                    int nX, nY;
                    XTranslateCoordinates( GetXDisplay(),
                                           GetWindow(),
                                           pDisplay_->GetRootWindow(),
                                           0, 0,
                                           &nX, &nY,
                                           &hDummy );
                    aPosSize_.SetPos( Point( nX, nY ) );
                }
                RepositionChildren();
                break;
        }
    }

    // #74406# do not raise fullscreenwindow as it may override the
    // screenlocker
#if 0
    if( bAlwaysOnTop_
        && nVisibility_ != VisibilityUnobscured
        && pEvent->type != ConfigureNotify
        && pEvent->type != MotionNotify )
    {
        XRaiseWindow( GetXDisplay(), XtWindow( hShell_ ) );
    }
#endif


    return nRet;
}

