/*************************************************************************
 *
 *  $RCSfile: salframe.cxx,v $
 *
 *  $Revision: 1.164 $
 *
 *  last change: $Author: kz $ $Date: 2003-08-25 13:58:00 $
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
 *  Contributor(s): Juergen Keil
 *
 *
 ************************************************************************/

#define _SV_SALFRAME_CXX

// -=-= #includes =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#if !(defined FREEBSD || defined NETBSD || defined MACOSX)
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
#ifndef _USE_PRINT_EXTENSION_
#ifndef _PSPRINT_PRINTERINFOMANAGER_HXX_
#include <psprint/printerinfomanager.hxx>
#endif
#ifndef _SV_SALPRN_HXX
#include <salprn.hxx>
#endif
#endif
#ifndef _SV_FLOATWIN_HXX
#include <floatwin.hxx>
#endif
#ifndef _SV_SALLAYOUT_HXX
#include <sallayout.hxx>
#endif
#include <svapp.hxx>

#ifndef _SV_SALBMP_HXX
#include <salbmp.hxx>
#endif

#ifndef _SAL_I18N_INPUTCONTEXT_HXX
#include <i18n_ic.hxx>
#endif

#ifndef _SAL_I18N_KEYSYM_HXX
#include <i18n_keysym.hxx>
#endif

#ifndef _SAL_I18N_STATUS_HXX
#include <i18n_status.hxx>
#endif

#ifndef _RTL_BOOTSTRAP_HXX
#include <rtl/bootstrap.hxx>
#endif
#ifndef _OSL_PROCESS_H
#include <osl/process.h>
#endif

#include <algorithm>

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

static XLIB_Window  hPresentationWindow = None, hPresFocusWindow = None;
static ::std::list< XLIB_Window > aPresentationReparentList;
static int          nVisibleFloats      = 0;

SalFrame* SalFrameData::s_pSaveYourselfFrame = NULL;

// -=-= C++ statics =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static long sal_CallbackDummy( void*, SalFrame*, USHORT, const void* )
{ return 0; }

static void doReparentPresentationDialogues( SalDisplay* pDisplay )
{
    BOOL bIgnore = pDisplay->GetXLib()->GetIgnoreXErrors();
    while( aPresentationReparentList.begin() != aPresentationReparentList.end() )
    {
        pDisplay->GetXLib()->SetIgnoreXErrors( TRUE );
        int x, y;
        XLIB_Window aRoot;
        unsigned int w, h, bw, d;
        pDisplay->GetXLib()->SetIgnoreXErrors( TRUE );
        XGetGeometry( pDisplay->GetDisplay(),
                      aPresentationReparentList.front(),
                      &aRoot,
                      &x, &y, &w, &h, &bw, &d );
        pDisplay->GetXLib()->SetIgnoreXErrors( TRUE );
        XTranslateCoordinates( pDisplay->GetDisplay(),
                               hPresentationWindow,
                               pDisplay->GetRootWindow(),
                               x, y,
                               &x, &y,
                               &aRoot );
        pDisplay->GetXLib()->SetIgnoreXErrors( TRUE );
        XReparentWindow( pDisplay->GetDisplay(),
                         aPresentationReparentList.front(),
                         pDisplay->GetRootWindow(),
                         x, y );
        aPresentationReparentList.pop_front();
    }
    if( hPresFocusWindow )
        XSetInputFocus( pDisplay->GetDisplay(), hPresFocusWindow, PointerRoot, CurrentTime );
    XSync( pDisplay->GetDisplay(), False );
    pDisplay->GetXLib()->SetIgnoreXErrors( bIgnore );
}

static char* getFrameResName()
{
    /*  according to ICCCM:
     *  first search command line for -name parameter
     *  then try REOURCE_NAME environment variable
     *  then use argv[0] stripped by directories
     */
    static char pResName[256] = "";
    if( !*pResName )
    {
        int nArgs = osl_getCommandArgCount();
        for( int n = 0; n < nArgs-1; n++ )
        {
            ::rtl::OUString aArg;
            if( ! osl_getCommandArg( n, &aArg.pData ) &&
                aArg.equalsIgnoreAsciiCaseAscii( "-name" ) &&
                ! osl_getCommandArg( n+1, &aArg.pData ) )
            {
                strncpy( pResName, ::rtl::OUStringToOString( aArg, osl_getThreadTextEncoding() ).getStr(), sizeof(pResName)-1 );
                break;
            }
        }
        if( !*pResName )
        {
            const char* pEnv = getenv( "RESOURCE_NAME" );
            if( pEnv && *pEnv )
                strncpy( pResName, pEnv, sizeof(pResName)-1 );
        }
        if( !*pResName )
            strcpy( pResName, "VCLSalFrame" );
    }
    return pResName;
}

static char* getFrameClassName()
{
    static char pClassName[256] = "";
    if( !*pClassName )
    {
        ::rtl::OUString aIni, aProduct;
        osl_getExecutableFile( &aIni.pData );
        aIni = aIni.copy( 0, aIni.lastIndexOf( SAL_PATHDELIMITER )+1 );
        aIni += ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( SAL_CONFIGFILE( "bootstrap" ) ) );
        ::rtl::Bootstrap aBootstrap( aIni );
        aBootstrap.getFrom( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ProductKey" ) ), aProduct );

        if( aProduct.getLength() )
        {
            strncpy( pClassName, ::rtl::OUStringToOString( aProduct, osl_getThreadTextEncoding() ).getStr(), sizeof( pClassName )-1 );
        }
        else
            strcpy( pClassName, "VCLSalFrame" );
    }
    return pClassName;
}

// -=-= SalInstance =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
SalFrame *SalInstance::CreateFrame( SalFrame *pParent,
                                          ULONG nSalFrameStyle )
{
    SalFrame *pFrame = new SalFrame;

    pFrame->maFrameData.mpParent = pParent;
    if( pParent )
        pParent->maFrameData.maChildren.push_back( pFrame );
    pFrame->maFrameData.Init( nSalFrameStyle );

    // initialize system settings update
    DtIntegrator* pIntegrator = DtIntegrator::CreateDtIntegrator( pFrame );
    pIntegrator->Acquire();

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
    m_pFrame        = pFrame;
    m_pVDev         = NULL;

    nPenPixel_      = GetPixel( nPenColor_ );
    nTextPixel_     = GetPixel( nTextColor_ );
    nBrushPixel_    = GetPixel( nBrushColor_ );
}

// -=-= SalFrame / SalFrameData =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

bool SalFrameData::IsOverrideRedirect() const
{
    return
        ((nStyle_ & SAL_FRAME_STYLE_INTRO) && !pDisplay_->getWMAdaptor()->supportsSplash())
        ||
        (!( nStyle_ & ~SAL_FRAME_STYLE_DEFAULT ) && !pDisplay_->getWMAdaptor()->supportsFullScreen())
        ;
}

bool SalFrameData::IsFloatGrabWindow() const
{
    static const char* pDisableGrab = getenv( "SAL_DISABLE_FLOATGRAB" );

    return
        ( ( !pDisableGrab || !*pDisableGrab ) &&
          ((nStyle_ & SAL_FRAME_STYLE_FLOAT) && !(nStyle_ & SAL_FRAME_STYLE_TOOLTIP)) );
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

void SalFrameData::Init( ULONG nSalFrameStyle, SystemParentData* pParentData )
{
    nStyle_     = nSalFrameStyle;
    XWMHints Hints;
    Hints.flags = InputHint;
    Hints.input = True;

    int x = 0, y = 0;
    unsigned int w = 500, h = 500;
    XSetWindowAttributes Attributes;

    int nAttrMask =   CWBorderPixel
                    | CWBackPixmap
                    | CWColormap
                    | CWOverrideRedirect
                    | CWEventMask
                    ;
    Attributes.border_pixel             = 0;
    Attributes.background_pixmap        = None;
    Attributes.colormap                 = GetDisplay()->GetColormap().GetXColormap();
    Attributes.override_redirect        = False;
    Attributes.event_mask               = CLIENT_EVENTS;

    SalVisual* pVis = GetDisplay()->GetVisual();
    XLIB_Window aFrameParent = pParentData ? pParentData->aWindow : GetDisplay()->GetRootWindow();

    if( nSalFrameStyle & SAL_FRAME_STYLE_FLOAT )
    {
        w = 10;
        h = 10;
        Attributes.override_redirect = True;

    }
    else if( pParentData )
    {
        // plugin parent may be killed unexpectedly by
        // plugging process; ignore XErrors in that case
        GetDisplay()->GetXLib()->SetIgnoreXErrors( TRUE );

        nStyle_ |= SAL_FRAME_STYLE_CHILD;
        Attributes.override_redirect = True;

        int x_ret, y_ret;
        unsigned int bw, d;
        XLIB_Window aRoot, aParent;

        XGetGeometry( GetXDisplay(), pParentData->aWindow,
                      &aRoot, &x_ret, &y_ret, &w, &h, &bw, &d );
        mhForeignParent = pParentData->aWindow;

        mhShellWindow = aParent = mhForeignParent;
        XLIB_Window* pChildren;
        unsigned int nChildren;
        bool bBreak = false;
        do
        {
            XQueryTree( GetDisplay()->GetDisplay(), mhShellWindow,
                        &aRoot, &aParent, &pChildren, &nChildren );
            XFree( pChildren );
            if( aParent != aRoot )
                mhShellWindow = aParent;
            int nCount = 0;
            Atom* pProps = XListProperties( GetDisplay()->GetDisplay(),
                                            mhShellWindow,
                                            &nCount );
            for( int i = 0; i < nCount && ! bBreak; ++i )
                bBreak = (pProps[i] == XA_WM_HINTS);
            if( pProps )
                XFree( pProps );
        } while( aParent != aRoot && ! bBreak );

        // check if this is really one of our own frames
        // do not change the input mask in that case
        SalFrame* pFrame = GetSalData()->pFirstFrame_;
        while( pFrame && mhForeignParent != pFrame->maFrameData.GetWindow() )
            pFrame = pFrame->maFrameData.pNextFrame_;

        if( ! pFrame )
        {
            XSelectInput( GetDisplay()->GetDisplay(), mhForeignParent, StructureNotifyMask | FocusChangeMask );
            XSelectInput( GetDisplay()->GetDisplay(), mhShellWindow, StructureNotifyMask | FocusChangeMask );
        }
    }
    else
    {
        const Size& rScreenSize( pDisplay_->GetScreenSize() );
        w = rScreenSize.Width();
        h = rScreenSize.Height();
        if( nSalFrameStyle & SAL_FRAME_STYLE_SIZEABLE &&
            nSalFrameStyle & SAL_FRAME_STYLE_MOVEABLE )
        {
            // fill in holy default values brought to us by product management
            if( rScreenSize.Width() >= 800 )
                w = 785;
            if( rScreenSize.Width() >= 1024 )
                w = 920;

            if( rScreenSize.Height() >= 600 )
                h = 550;
            if( rScreenSize.Height() >= 768 )
                h = 630;
            if( rScreenSize.Height() >= 1024 )
                h = 875;
        }
        if( ! mpParent )
        {
            // find the last document window (if any)
            SalFrame* pFrame = pNextFrame_;
            while( pFrame &&
                   ( pFrame->maFrameData.mpParent
                     || pFrame->maFrameData.mbFullScreen
                     || ! ( pFrame->maFrameData.nStyle_ & SAL_FRAME_STYLE_SIZEABLE )
                     || ! pFrame->GetUnmirroredGeometry().nWidth
                     || ! pFrame->GetUnmirroredGeometry().nHeight
                     )
                   )
                pFrame = pFrame->maFrameData.pNextFrame_;
            if( pFrame )
            {
                // set a document position and size
                // the first frame gets positioned by the window manager
                const SalFrameGeometry& rGeom( pFrame->GetUnmirroredGeometry() );
                x = rGeom.nX;
                y = rGeom.nY;
                w = rGeom.nWidth;
                h = rGeom.nHeight;
                if( x+(int)w+40 <= (int)pDisplay_->GetScreenSize().Width() &&
                    y+(int)h+40 <= (int)pDisplay_->GetScreenSize().Height()
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
            else if( GetDisplay()->IsXinerama() )
            {
                // place frame on same screen as mouse pointer
                XLIB_Window aRoot, aChild;
                int root_x = 0, root_y = 0, lx, ly;
                unsigned int mask;
                XQueryPointer( GetXDisplay(),
                               GetDisplay()->GetRootWindow(),
                               &aRoot, &aChild,
                               &root_x, &root_y, &lx, &ly, &mask );
                const std::vector< Rectangle >& rScreens = GetDisplay()->GetXineramaScreens();
                for( int i = 0; i < rScreens.size(); i++ )
                    if( rScreens[i].IsInside( Point( root_x, root_y ) ) )
                    {
                        x = rScreens[i].Left();
                        y = rScreens[i].Top();
                        break;
                    }
            }
        }
        Attributes.win_gravity = pDisplay_->getWMAdaptor()->getInitWinGravity();
        nAttrMask |= CWWinGravity;
        if( mpParent )
        {
            Attributes.save_under = True;
            nAttrMask |= CWSaveUnder;
        }
        if( IsOverrideRedirect() )
            Attributes.override_redirect = True;
        // default icon
        if( SelectAppIconPixmap( pDisplay_, mpParent ? mpParent->maFrameData.mnIconID : 1, 32,
                                 Hints.icon_pixmap, Hints.icon_mask ))
        {
            Hints.flags     |= IconPixmapHint;
            if( Hints.icon_mask )
                Hints.flags |= IconMaskHint;
        }

        Hints.flags         |= WindowGroupHint;
        SalFrame* pFrame = pFrame_;
        while( pFrame->maFrameData.mpParent )
            pFrame = pFrame->maFrameData.mpParent;
        Hints.window_group  = pFrame->maFrameData.GetShellWindow();
    }

    mhWindow = XCreateWindow( GetXDisplay(),
                               aFrameParent,
                               x, y,
                               w, h,
                               0,
                               pVis->GetDepth(),
                               InputOutput,
                               pVis->GetVisual(),
                               nAttrMask,
                               &Attributes );
    mhShellWindow = pParentData ? mhShellWindow : mhWindow;

    pFrame_->maGeometry.nX      = x;
    pFrame_->maGeometry.nY      = y;
    pFrame_->maGeometry.nWidth  = w;
    pFrame_->maGeometry.nHeight = h;

    XSync( GetXDisplay(), False );

    if( ! pParentData && ! (nSalFrameStyle & (SAL_FRAME_STYLE_FLOAT|SAL_FRAME_STYLE_CHILD)) )
    {
        XSetWMHints( GetXDisplay(), mhWindow, &Hints );
        // WM Protocols && internals
        Atom a[4];
        int  n = 0;
        a[n++] = pDisplay_->getWMAdaptor()->getAtom( WMAdaptor::WM_DELETE_WINDOW );
        if( ! s_pSaveYourselfFrame && ! mpParent)
        {
            // at all times have only one frame with SaveYourself
            a[n++] = pDisplay_->getWMAdaptor()->getAtom( WMAdaptor::WM_SAVE_YOURSELF );
            s_pSaveYourselfFrame = pFrame_;
        }
        XSetWMProtocols( GetXDisplay(), GetShellWindow(), a, n );

        XClassHint* pClass = XAllocClassHint();
        pClass->res_name  = getFrameResName();
        pClass->res_class = getFrameClassName();
        XSetClassHint( GetXDisplay(), GetShellWindow(), pClass );
        XFree( pClass );

        XSizeHints* pHints = XAllocSizeHints();
        pHints->flags       = PWinGravity;
        pHints->win_gravity = GetDisplay()->getWMAdaptor()->getPositionWinGravity();
        XSetWMNormalHints( GetXDisplay(),
                           GetShellWindow(),
                           pHints );
        XFree (pHints);

        // set client leader
        XLIB_Window aLeader = GetDisplay()->GetDrawable();
        XChangeProperty( GetXDisplay(),
                         mhWindow,
                         pDisplay_->getWMAdaptor()->getAtom( WMAdaptor::WM_CLIENT_LEADER),
                         XA_WINDOW,
                         32,
                         PropModeReplace,
                         (unsigned char*)&aLeader,
                         1
                         );
#define DECOFLAGS (SAL_FRAME_STYLE_MOVEABLE | SAL_FRAME_STYLE_SIZEABLE | SAL_FRAME_STYLE_CLOSEABLE)
        int nDecoFlags = WMAdaptor::decoration_All;
        if( (nStyle_ & DECOFLAGS ) != DECOFLAGS || (nStyle_ & SAL_FRAME_STYLE_TOOLWINDOW) )
        {
            if( nStyle_ & (SAL_FRAME_STYLE_MOVEABLE | SAL_FRAME_STYLE_SIZEABLE | SAL_FRAME_STYLE_CLOSEABLE ) )
                // if any decoration, then show a border
                nDecoFlags = WMAdaptor::decoration_Border;
            else
                nDecoFlags = 0;

            if( ! mpParent && (nStyle_ & DECOFLAGS) )
                // don't add a min button if window should be decorationless
                nDecoFlags |= WMAdaptor::decoration_MinimizeBtn;
            if( nStyle_ & SAL_FRAME_STYLE_CLOSEABLE )
                nDecoFlags |= WMAdaptor::decoration_CloseBtn;
            if( nStyle_ & SAL_FRAME_STYLE_SIZEABLE )
            {
                nDecoFlags |= WMAdaptor::decoration_Resize;
                if( ! (nStyle_ & SAL_FRAME_STYLE_TOOLWINDOW) )
                    nDecoFlags |= WMAdaptor::decoration_MaximizeBtn;
            }
            if( nStyle_ & SAL_FRAME_STYLE_MOVEABLE )
                nDecoFlags |= WMAdaptor::decoration_Title;
        }

        WMAdaptor::WMWindowType eType = WMAdaptor::windowType_Normal;
        if( nStyle_ & SAL_FRAME_STYLE_INTRO )
            eType = WMAdaptor::windowType_Splash;
        if( mpParent && hPresentationWindow == None )
            eType = WMAdaptor::windowType_ModelessDialogue;
        if( nStyle_ & SAL_FRAME_STYLE_TOOLWINDOW )
            eType = WMAdaptor::windowType_Utility;

        GetDisplay()->getWMAdaptor()->
            setFrameTypeAndDecoration( pFrame_,
                                       eType,
                                       nDecoFlags,
                                       hPresentationWindow ? NULL : mpParent );

        if( nStyle_ & SAL_FRAME_STYLE_DEFAULT )
        pDisplay_->getWMAdaptor()->maximizeFrame( pFrame_, true, true );
    }

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
    mbTransientForRoot          = false;

    pProc_                      = sal_CallbackDummy;
    pInst_                      = (void*)ILLEGAL_POINTER;

    pDisplay_                   = pSalData->GetCurDisp();
    mhWindow                    = None;
    mhShellWindow               = None;
    mhStackingWindow            = None;
    mhForeignParent             = None;

    pGraphics_                  = NULL;
    pFreeGraphics_              = NULL;

    hCursor_                    = None;
    nCaptured_                  = 0;

     nReleaseTime_              = 0;
    nKeyCode_                   = 0;
    nKeyState_                  = 0;
    nCompose_                   = -1;
    mbKeyMenu                   = false;
    mbSendExtKeyModChange       = false;
    mnExtKeyMod                 = 0;

    nShowState_                 = SHOWSTATE_UNKNOWN;
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
    mbInShow                    = FALSE;

    nScreenSaversTimeout_       = 0;

    mpInputContext              = NULL;
    mbDeleteInputContext        = false;
    mbInputFocus                = False;

    maResizeTimer.SetTimeoutHdl( LINK( this, SalFrameData, HandleResizeTimer ) );
    maResizeTimer.SetTimeout( 50 );

    maAlwaysOnTopRaiseTimer.SetTimeoutHdl( LINK( this, SalFrameData, HandleAlwaysOnTopRaise ) );
    maAlwaysOnTopRaiseTimer.SetTimeout( 100 );

    mpDeleteData                = NULL;

    meWindowType                = WMAdaptor::windowType_Normal;
    mnDecorationFlags           = WMAdaptor::decoration_All;
    mbMaximizedVert             = false;
    mbMaximizedHorz             = false;
    mbShaded                    = false;
    mbFullScreen                = false;
    mbMoved                     = false;
    mbSized                     = false;

    mnIconID                    = 0; // ICON_DEFAULT
}

SalFrame::SalFrame() : maFrameData( this )
{
    memset( &maGeometry, 0, sizeof(maGeometry) );
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

void SalFrameData::passOnSaveYourSelf()
{
    if( pFrame_ == s_pSaveYourselfFrame )
    {
        SalData* pSalData = GetSalData();

        // pass on SaveYourself
        SalFrame* pFrame = pSalData->pFirstFrame_;
        while( pFrame && ( ( pFrame->maFrameData.nStyle_ & (SAL_FRAME_STYLE_FLOAT|SAL_FRAME_STYLE_CHILD) ) || pFrame->maFrameData.mpParent  ) )
            pFrame = pFrame->maFrameData.GetNextFrame();
        s_pSaveYourselfFrame = pFrame;
        if( pFrame )
        {
            Atom a[4];
            int  n = 0;
            a[n++] = pDisplay_->getWMAdaptor()->getAtom( WMAdaptor::WM_DELETE_WINDOW );
            a[n++] = pDisplay_->getWMAdaptor()->getAtom( WMAdaptor::WM_SAVE_YOURSELF );
            XSetWMProtocols( GetXDisplay(), pFrame->maFrameData.GetShellWindow(), a, n );
        }
    }
}

inline SalFrameData::~SalFrameData()
{
    if( mpInputContext )
    {
        mpInputContext->UnsetICFocus( pFrame_ );
        mpInputContext->Unmap( pFrame_ );
        if( mbDeleteInputContext )
            delete mpInputContext;
    }

    if( GetWindow() == hPresentationWindow )
    {
        hPresentationWindow = None;
        doReparentPresentationDialogues( GetDisplay() );
    }

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

    XDestroyWindow( GetXDisplay(), mhWindow );

    SalData* pSalData = GetSalData();

    if( pFrame_ == pSalData->pFirstFrame_ )
        pSalData->pFirstFrame_ = GetNextFrame();
    else
    {
        SalFrameData *pTemp = &pSalData->pFirstFrame_->maFrameData;
        while( pTemp->GetNextFrame() != pFrame_ )
            pTemp = &pTemp->GetNextFrame()->maFrameData;

        pTemp->pNextFrame_ = GetNextFrame();
    }
    /*
     *  check if there is only the status frame is left
     *  if so, free it
     */
    SalFrame* pStatusFrame = I18NStatus::get().getStatusFrame();
    if( pStatusFrame
        && pSalData->pFirstFrame_ == pStatusFrame
        && pSalData->pFirstFrame_->maFrameData.GetNextFrame() == NULL )
        ::vcl::I18NStatus::free();

    passOnSaveYourSelf();
}

SalFrame::~SalFrame()
{
    if( maFrameData.mhStackingWindow )
        aPresentationReparentList.remove( maFrameData.mhStackingWindow );
    // aus papis child liste entfernen
    if( maFrameData.mpParent )
        maFrameData.mpParent->maFrameData.maChildren.remove( this );
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
    pFrame->maFrameData.maSystemChildData.pWidget       = NULL;
    pFrame->maFrameData.maSystemChildData.pVisual       = _GetDisplay()->GetVisual()->GetVisual();
    pFrame->maFrameData.maSystemChildData.nDepth        = _GetDisplay()->GetVisual()->GetDepth();
    pFrame->maFrameData.maSystemChildData.aColormap     = _GetDisplay()->GetColormap().GetXColormap();
    pFrame->maFrameData.maSystemChildData.pAppContext   = NULL;
    pFrame->maFrameData.maSystemChildData.aShellWindow  = pFrame->maFrameData.GetShellWindow();
    pFrame->maFrameData.maSystemChildData.pShellWidget  = NULL;
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
        pGraphics_ = new SalGraphicsLayout;
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
    if ( !( maFrameData.nStyle_ & (SAL_FRAME_STYLE_CHILD|SAL_FRAME_STYLE_FLOAT) ) )
    {
        maFrameData.mnIconID = nIcon;

        XIconSize *pIconSize = NULL;
        int nSizes = 0;
        int iconSize = 32;
        if ( XGetIconSizes( _GetXDisplay(), _GetDisplay()->GetRootWindow(), &pIconSize, &nSizes ) )
        {
#if OSL_DEBUG_LEVEL > 1
            fprintf(stderr, "SalFrame::SetIcon(): found %d IconSizes:\n", nSizes);
#endif

            const int ourLargestIconSize = 48;
            bool bFoundIconSize = false;

            int i;
            for( i=0; i<nSizes; i++)
            {
               // select largest supported icon

               // Note: olwm/olvwm reports a huge max icon size of
               // 160x160 pixels; always choosing the max as the
               // preferred icon size is apparently wrong under olvwm
               // - so we keep the safe default |iconSize| when we see
               // unreasonable large max icon sizes (> twice of our
               // largest available icon) reported by XGetIconSizes.
                if( pIconSize[i].max_width > iconSize
                    && pIconSize[i].max_width <= 2*ourLargestIconSize )
                {
                    iconSize = pIconSize[i].max_width;
                    bFoundIconSize = true;
                }
                iconSize = pIconSize[i].max_width;

#if OSL_DEBUG_LEVEL > 1
                fprintf(stderr, "min: %d, %d\nmax: %d, %d\ninc: %d, %d\n\n",
                        pIconSize[i].min_width, pIconSize[i].min_height,
                        pIconSize[i].max_width, pIconSize[i].max_height,
                        pIconSize[i].width_inc, pIconSize[i].height_inc);
#endif
            }

            if ( !bFoundIconSize )
            {
               // Unless someone has fixed olwm/olvwm, we have rejected
               // the max icon size from |XGetIconSizes()|.  Provide a
               // better icon size default value, in case our window manager
               // is olwm/olvwm.
               const String& rWM( maFrameData.pDisplay_->getWMAdaptor()->getWindowManagerName() );

               if ( rWM.EqualsAscii( "Olwm" ) )
                   iconSize = 48;
            }

            XFree( pIconSize );
        }
        else
        {
            const String& rWM( maFrameData.pDisplay_->getWMAdaptor()->getWindowManagerName() );
            if( rWM.EqualsAscii( "KWin" ) )         // assume KDE is running
                iconSize = 16;
            static bool bGnomeIconSize = false;
            static bool bGnomeChecked = false;
            if( ! bGnomeChecked )
            {
                bGnomeChecked=true;
                int nCount = 0;
                Atom* pProps = XListProperties( _GetXDisplay(),
                                                _GetDisplay()->GetRootWindow(),
                                                &nCount );
                for( int i = 0; i < nCount && !bGnomeIconSize; i++ )
                 {
                    char* pName = XGetAtomName( _GetXDisplay(), pProps[i] );
                    if( !strcmp( pName, "GNOME_PANEL_DESKTOP_AREA" ) )
                        bGnomeIconSize = true;
                    if( pName )
                        XFree( pName );
                 }
            }
            if( bGnomeIconSize )
                iconSize = 20;
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
    if( maFrameData.GetShellWindow() )
    {
        XSizeHints* pHints = XAllocSizeHints();
        long nSupplied = 0;
        XGetWMNormalHints( _GetXDisplay(),
                           maFrameData.GetShellWindow(),
                           pHints,
                           &nSupplied
                           );
        pHints->min_width   = nWidth;
        pHints->min_height  = nHeight;
        pHints->flags |= PMinSize;
        XSetWMNormalHints( _GetXDisplay(),
                           maFrameData.GetShellWindow(),
                           pHints );
        XFree( pHints );
    }
}

// Show + Pos (x,y,z) + Size (width,height)
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void SalFrame::Show( BOOL bVisible, BOOL /*bNoActivate*/ )
{
    if( ( bVisible && maFrameData.bMapped_ )
        || ( !bVisible && !maFrameData.bMapped_ ) )
        return;

    maFrameData.bMapped_   = bVisible;
    maFrameData.bViewable_ = bVisible;
    if( bVisible )
    {
        SessionManagerClient::open(); // will simply return after the first time

        maFrameData.mbInShow = TRUE;
        if( ! (maFrameData.nStyle_ & SAL_FRAME_STYLE_INTRO) )
        {
            SalFrame* pFrame = GetSalData()->pFirstFrame_;
            while( pFrame )
            {
                // look for intro bit map; if present, hide it
                if( pFrame->maFrameData.nStyle_ & SAL_FRAME_STYLE_INTRO )
                    pFrame->Show( FALSE );
                pFrame = pFrame->maFrameData.pNextFrame_;
            }
        }

        /*
         *  #95097#
         *  Actually this is rather exotic and currently happens only in conjunction
         *  with the basic dialogue editor,
         *  which shows a frame and instantly hides it again. After that the
         *  editor window is shown and the WM takes this as an opportunity
         *  to show our hidden transient frame also. So Show( FALSE ) must
         *  withdraw the frame AND delete the WM_TRANSIENT_FOR property.
         *  In case the frame is shown again, the transient hint must be restored here.
         */
        if( ! ( maFrameData.nStyle_ & ( SAL_FRAME_STYLE_FLOAT | SAL_FRAME_STYLE_CHILD ) )
            && ! maFrameData.IsOverrideRedirect()
            && maFrameData.mpParent
            )
        {
            _GetDisplay()->getWMAdaptor()->changeReferenceFrame( this, maFrameData.mpParent );
        }

        // actually map the window
        if( maFrameData.GetWindow() != maFrameData.GetShellWindow() )
        {
            XMapWindow( _GetXDisplay(), maFrameData.GetShellWindow() );
            XSelectInput( _GetXDisplay(), maFrameData.GetShellWindow(), CLIENT_EVENTS );
        }
        if( maFrameData.nStyle_ & SAL_FRAME_STYLE_FLOAT )
            XMapRaised( _GetXDisplay(), maFrameData.GetWindow() );
        else
            XMapWindow( _GetXDisplay(), maFrameData.GetWindow() );
        XSelectInput( _GetXDisplay(), maFrameData.GetWindow(), CLIENT_EVENTS );

        if( maGeometry.nWidth > 0
            && maGeometry.nHeight > 0
            && (   maFrameData.nWidth_  != (int)maGeometry.nWidth
                || maFrameData.nHeight_ != (int)maGeometry.nHeight ) )
        {
            maFrameData.nWidth_  = maGeometry.nWidth;
            maFrameData.nHeight_ = maGeometry.nHeight;
        }

        XSync( _GetXDisplay(), False );

        if( maFrameData.IsFloatGrabWindow() )
        {
            /*
             *  #95453#
             *  Sawfish and twm can be switched to enter-exit focus behaviour. In this case
             *  we must grab the pointer else the dumb WM will put the focus to the
             *  override-redirect float window. The application window will be deactivated
             *  which causes that the floats are destroyed, so the user can never click on
             *  a menu because it vanishes as soon as he enters it.
             */
            nVisibleFloats++;
            if( nVisibleFloats == 1 && ! _GetDisplay()->GetCaptureFrame() )
            {
                XGrabPointer( _GetXDisplay(),
                              maFrameData.GetWindow(),
                              True,
                              PointerMotionMask | ButtonPressMask | ButtonReleaseMask,
                              GrabModeAsync,
                              GrabModeAsync,
                              None,
                              maFrameData.mpParent ? maFrameData.mpParent->maFrameData.GetCursor() : None,
                              CurrentTime
                              );
            }
        }
        maFrameData.Call( SALEVENT_RESIZE, NULL );

        /*
         *  sometimes a message box/dialogue is brought up when a frame is not mapped
         *  the corresponding TRANSIENT_FOR hint is then set to the root window
         *  so that the dialogue shows in all cases. Correct it here if the
         *  frame is shown afterwards.
         */
        if( ! ( maFrameData.nStyle_ & ( SAL_FRAME_STYLE_FLOAT | SAL_FRAME_STYLE_CHILD ) )
            && ! maFrameData.IsOverrideRedirect() )
        {
            for( ::std::list< SalFrame* >::const_iterator it = maFrameData.maChildren.begin();
                 it != maFrameData.maChildren.end(); ++it )
            {
                if( (*it)->maFrameData.mbTransientForRoot )
                    _GetDisplay()->getWMAdaptor()->changeReferenceFrame( *it, this );
            }
        }
        /*
         *  leave SHOWSTATE_UNKNOWN as this indicates first mapping
         *  and is only reset int HandleSizeEvent
         */
        if( maFrameData.nShowState_ != SHOWSTATE_UNKNOWN )
            maFrameData.nShowState_ = SHOWSTATE_NORMAL;

        /*
         *  #98107# plugged windows don't necessarily get the
         *  focus on show because the parent may already be mapped
         *  and have the focus. So try to set the focus
         *  to the child on Show(TRUE)
         */
        if( maFrameData.nStyle_ & SAL_FRAME_STYLE_CHILD )
            XSetInputFocus( _GetXDisplay(),
                            maFrameData.GetWindow(),
                            RevertToParent,
                            CurrentTime );

        if( maFrameData.mpParent )
        {
            // push this frame so it will be in front of its siblings
            // only necessary for insane transient behaviour of Dtwm/olwm
            maFrameData.mpParent->maFrameData.maChildren.remove( this );
            maFrameData.mpParent->maFrameData.maChildren.push_front(this);
        }
    }
    else
    {
        if( maFrameData.getInputContext() )
            maFrameData.getInputContext()->Unmap( this );

        if( maFrameData.mpParent )
            XDeleteProperty( _GetXDisplay(), maFrameData.GetShellWindow(), _GetDisplay()->getWMAdaptor()->getAtom( WMAdaptor::WM_TRANSIENT_FOR ) );
        XWithdrawWindow( _GetXDisplay(), maFrameData.GetWindow(), _GetDisplay()->GetScreenNumber() );
        maFrameData.nShowState_ = SHOWSTATE_HIDDEN;
        if( maFrameData.IsFloatGrabWindow() && nVisibleFloats )
        {
            nVisibleFloats--;
            if( nVisibleFloats == 0  && ! _GetDisplay()->GetCaptureFrame() )
                XUngrabPointer( _GetXDisplay(),
                                CurrentTime );
        }
    }
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void SalFrame::ToTop( USHORT nFlags )
{
    if( ( nFlags & SAL_FRAME_TOTOP_RESTOREWHENMIN )
        && ! ( maFrameData.nStyle_ & SAL_FRAME_STYLE_FLOAT )
        && maFrameData.nShowState_ != SHOWSTATE_HIDDEN
        && maFrameData.nShowState_ != SHOWSTATE_UNKNOWN
        )
    {
        if( maFrameData.GetWindow() != maFrameData.GetShellWindow() )
            XMapWindow( _GetXDisplay(), maFrameData.GetShellWindow() );
        XMapWindow( _GetXDisplay(), maFrameData.GetWindow() );
    }

    if( ! (nFlags & SAL_FRAME_TOTOP_GRABFOCUS_ONLY) )
    {
        XRaiseWindow( _GetXDisplay(), maFrameData.GetShellWindow() );
        if( ! _GetDisplay()->getWMAdaptor()->isTransientBehaviourAsExpected() )
            for( std::list< SalFrame* >::const_iterator it = maFrameData.maChildren.begin();
                 it != maFrameData.maChildren.end(); ++it )
                (*it)->ToTop( nFlags & ~SAL_FRAME_TOTOP_GRABFOCUS );
    }

    if( ( ( nFlags & SAL_FRAME_TOTOP_GRABFOCUS ) || ( nFlags & SAL_FRAME_TOTOP_GRABFOCUS_ONLY ) )
        && maFrameData.bMapped_ )
        XSetInputFocus( _GetXDisplay(), maFrameData.GetShellWindow(), RevertToParent, CurrentTime );
}
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void SalFrame::GetWorkArea( Rectangle& rWorkArea )
{
    rWorkArea = maFrameData.pDisplay_->getWMAdaptor()->getWorkArea( 0 );
}
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void SalFrame::GetClientSize( long &rWidth, long &rHeight )
{
    if( ! maFrameData.bViewable_  )
    {
        rWidth = rHeight = 0;
        return;
    }

    rWidth  = maGeometry.nWidth;
    rHeight = maGeometry.nHeight;

    if( !rWidth || !rHeight )
    {
        XWindowAttributes aAttrib;

        XGetWindowAttributes( _GetXDisplay(), maFrameData.GetShellWindow(), &aAttrib );

        rWidth  = aAttrib.width;
        rHeight = aAttrib.height;
    }
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

void SalFrameData::SetWindowGravity (int nGravity) const
{
    XSizeHints* pHint = XAllocSizeHints();
    long        nFlag;

    XGetWMNormalHints (GetXDisplay(), GetShellWindow(), pHint, &nFlag);
    pHint->flags       |= PWinGravity;
    pHint->win_gravity  = nGravity;

    XSetWMNormalHints (GetXDisplay(), GetShellWindow(), pHint);
    XSync (GetXDisplay(), False);

    XFree (pHint);
}

void SalFrameData::Center( )
{
    int             nX, nY, nScreenWidth, nScreenHeight;
    int             nRealScreenWidth, nRealScreenHeight;
    int             nScreenX = 0, nScreenY = 0;

    nScreenWidth        = GetDisplay()->GetScreenSize().Width();
    nScreenHeight       = GetDisplay()->GetScreenSize().Height();
    nRealScreenWidth    = nScreenWidth;
    nRealScreenHeight   = nScreenHeight;

    if( GetDisplay()->IsXinerama() )
    {
        // get xinerama screen we are on
        // if there is a parent, use its center for screen determination
        // else use the pointer
        XLIB_Window aRoot, aChild;
        int root_x, root_y, x, y;
        unsigned int mask;
        if( mpParent )
        {
            root_x = mpParent->maGeometry.nX + mpParent->maGeometry.nWidth/2;
            root_y = mpParent->maGeometry.nY + mpParent->maGeometry.nHeight/2;
        }
        else
            XQueryPointer( GetXDisplay(),
                           GetShellWindow(),
                           &aRoot, &aChild,
                           &root_x, &root_y,
                           &x, &y,
                           &mask );
        const std::vector< Rectangle >& rScreens = GetDisplay()->GetXineramaScreens();
        for( int i = 0; i < rScreens.size(); i++ )
            if( rScreens[i].IsInside( Point( root_x, root_y ) ) )
            {
                nScreenX            = rScreens[i].Left();
                nScreenY            = rScreens[i].Top();
                nRealScreenWidth    = rScreens[i].GetWidth();
                nRealScreenHeight   = rScreens[i].GetHeight();
                break;
            }
    }

    if( mpParent )
    {
        SalFrame* pFrame = mpParent;
        while( pFrame->maFrameData.mpParent )
            pFrame = pFrame->maFrameData.mpParent;
        if( pFrame->maGeometry.nWidth < 1  || pFrame->maGeometry.nHeight < 1 )
        {
            Rectangle aRect;
            pFrame->maFrameData.GetPosSize( aRect );
            pFrame->maGeometry.nX       = aRect.Left();
            pFrame->maGeometry.nY       = aRect.Top();
            pFrame->maGeometry.nWidth   = aRect.GetWidth();
            pFrame->maGeometry.nHeight  = aRect.GetHeight();
        }

        if( pFrame->maFrameData.nStyle_ & SAL_FRAME_STYLE_CHILD )
        {
            XLIB_Window aRoot;
            unsigned int bw, depth;
            XGetGeometry( GetXDisplay(),
                          pFrame->maFrameData.GetShellWindow(),
                          &aRoot,
                          &nScreenX, &nScreenY,
                          (unsigned int*)&nScreenWidth,
                          (unsigned int*)&nScreenHeight,
                          &bw, &depth );
        }
        else
        {
            nScreenX        = pFrame->maGeometry.nX;
            nScreenY        = pFrame->maGeometry.nY;
            nScreenWidth    = pFrame->maGeometry.nWidth;
            nScreenHeight   = pFrame->maGeometry.nHeight;
        }
    }

    if( mpParent && mpParent->maFrameData.nShowState_ == SHOWSTATE_NORMAL )
    {
        if( pFrame_->maGeometry.nWidth >= mpParent->maGeometry.nWidth &&
            pFrame_->maGeometry.nHeight >= mpParent->maGeometry.nHeight )
        {
            nX = nScreenX + 40;
            nY = nScreenY + 40;
        }
        else
        {
            // center the window relative to the top level frame
            nX = (nScreenWidth  - (int)pFrame_->maGeometry.nWidth ) / 2 + nScreenX;
            nY = (nScreenHeight - (int)pFrame_->maGeometry.nHeight) / 2 + nScreenY;
        }
    }
    else
    {
        // center the window relative to screen
        nX = (nRealScreenWidth  - (int)pFrame_->maGeometry.nWidth ) / 2 + nScreenX;
        nY = (nRealScreenHeight - (int)pFrame_->maGeometry.nHeight) / 2 + nScreenY;
    }
    nX = nX < 0 ? 0 : nX;
    nY = nY < 0 ? 0 : nY;

    bDefaultPosition_ = False;
    if( mpParent )
    {
        nX -= mpParent->maGeometry.nX;
        nY -= mpParent->maGeometry.nY;
    }

    Point aPoint(nX, nY);
    SetPosSize( Rectangle( aPoint, Size( pFrame_->maGeometry.nWidth, pFrame_->maGeometry.nHeight ) ) );
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void SalFrame::SetPosSize( long nX, long nY, long nWidth, long nHeight, USHORT nFlags )
{
    if( maFrameData.nStyle_ & SAL_FRAME_STYLE_CHILD )
        return;

    // relative positioning in SalFrameData::SetPosSize
    Rectangle aPosSize( Point( maGeometry.nX, maGeometry.nY ), Size( maGeometry.nWidth, maGeometry.nHeight ) );
    aPosSize.Justify();

    if( ! ( nFlags & SAL_FRAME_POSSIZE_X ) )
    {
        nX = aPosSize.Left();
        if( maFrameData.mpParent )
            nX -= maFrameData.mpParent->maFrameData.pFrame_->maGeometry.nX;
    }
    if( ! ( nFlags & SAL_FRAME_POSSIZE_Y ) )
    {
        nY = aPosSize.Top();
        if( maFrameData.mpParent )
            nY -= maFrameData.mpParent->maFrameData.pFrame_->maGeometry.nY;
    }
    if( ! ( nFlags & SAL_FRAME_POSSIZE_WIDTH ) )
        nWidth = aPosSize.GetWidth();
    if( ! ( nFlags & SAL_FRAME_POSSIZE_HEIGHT ) )
        nHeight = aPosSize.GetHeight();

    aPosSize = Rectangle( Point( nX, nY ), Size( nWidth, nHeight ) );

    if( ! ( nFlags & ( SAL_FRAME_POSSIZE_X | SAL_FRAME_POSSIZE_Y ) ) )
    {
        if( maFrameData.bDefaultPosition_ )
        {
            maGeometry.nWidth = aPosSize.GetWidth();
            maGeometry.nHeight = aPosSize.GetHeight();
            maFrameData.Center();
        }
        else
            maFrameData.SetSize( Size( nWidth, nHeight ) );
    }
    else
        maFrameData.SetPosSize( aPosSize );
    maFrameData.bDefaultPosition_ = False;
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void SalFrame::SetAlwaysOnTop( BOOL bOnTop )
{
    if( ! maFrameData.IsOverrideRedirect() )
    {
        maFrameData.bAlwaysOnTop_ = bOnTop;
        maFrameData.pDisplay_->getWMAdaptor()->enableAlwaysOnTop( this, bOnTop );
    }
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

        const Size& rScreenSize( maFrameData.pDisplay_->GetScreenSize() );
        const WMAdaptor *pWM = _GetDisplay()->getWMAdaptor();
        int nGravity = pWM->getPositionWinGravity();

        if( pState->mnMask & ( SAL_FRAMESTATE_MASK_HEIGHT | SAL_FRAMESTATE_MASK_WIDTH )
            && aPosSize.GetWidth() <= rScreenSize.Width()
            && aPosSize.GetHeight() <= rScreenSize.Height() )
        {
            SalFrameGeometry aGeom = maGeometry;

            if( ! (maFrameData.nStyle_ & ( SAL_FRAME_STYLE_FLOAT | SAL_FRAME_STYLE_CHILD ) ) &&
               maFrameData.mpParent &&
                aGeom.nLeftDecoration == 0 &&
                aGeom.nTopDecoration == 0 )
            {
                aGeom = maFrameData.mpParent->maGeometry;
                if( aGeom.nLeftDecoration == 0 &&
                    aGeom.nTopDecoration == 0 )
                {
                    aGeom.nLeftDecoration = 5;
                    aGeom.nTopDecoration = 20;
                    aGeom.nRightDecoration = 5;
                    aGeom.nBottomDecoration = 5;
                }
            }

            // adjust position so that frame fits onto screen
            if( aPosSize.Right()+(long)aGeom.nRightDecoration > rScreenSize.Width()-1 )
                aPosSize.Move( (long)rScreenSize.Width() - (long)aPosSize.Right() - (long)aGeom.nRightDecoration, 0 );
            if( aPosSize.Bottom()+(long)aGeom.nBottomDecoration > rScreenSize.Height()-1 )
                aPosSize.Move( 0, (long)rScreenSize.Height() - (long)aPosSize.Bottom() - (long)aGeom.nBottomDecoration );
            if( aPosSize.Left() < (long)aGeom.nLeftDecoration )
                aPosSize.Move( (long)aGeom.nLeftDecoration - (long)aPosSize.Left(), 0 );
            if( aPosSize.Top() < (long)aGeom.nTopDecoration )
                aPosSize.Move( 0, (long)aGeom.nTopDecoration - (long)aPosSize.Top() );
        }

         // resize with new args
         if (pWM->supportsICCCMPos())
         {
             if( maFrameData.mpParent )
                 aPosSize.Move( -maFrameData.mpParent->maGeometry.nX,
                                -maFrameData.mpParent->maGeometry.nY );
             maFrameData.SetPosSize( aPosSize );
             maFrameData.bDefaultPosition_ = False;
         }
         else
             SetPosSize( 0, 0, aPosSize.GetWidth(), aPosSize.GetHeight(), SAL_FRAME_POSSIZE_WIDTH | SAL_FRAME_POSSIZE_HEIGHT );
    }

    // request for status change
    if (pState->mnMask & SAL_FRAMESTATE_MASK_STATE)
    {
        if (pState->mnState & SAL_FRAMESTATE_MAXIMIZED)
        {
            maFrameData.nShowState_ = SHOWSTATE_NORMAL;
            if( ! (pState->mnMask & (SAL_FRAMESTATE_MAXIMIZED_HORZ|SAL_FRAMESTATE_MAXIMIZED_VERT) ) )
                maFrameData.Maximize();
            else
            {
                bool bHorz = (pState->mnState & SAL_FRAMESTATE_MAXIMIZED_HORZ) ? true : false;
                bool bVert = (pState->mnState & SAL_FRAMESTATE_MAXIMIZED_VERT) ? true : false;
                _GetDisplay()->getWMAdaptor()->maximizeFrame( this, bHorz, bVert );
            }
        }
        else if( maFrameData.mbMaximizedHorz || maFrameData.mbMaximizedVert )
            _GetDisplay()->getWMAdaptor()->maximizeFrame( this, false, false );

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
            _GetDisplay()->getWMAdaptor()->shade( this, true );
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
    if (! maFrameData.maRestorePosSize.IsEmpty() )
    {
        aPosSize = maFrameData.maRestorePosSize;
        pState->mnState |= SAL_FRAMESTATE_MAXIMIZED;
    }
    else
    {
        maFrameData.GetPosSize( aPosSize );
    }
    if( maFrameData.mbMaximizedHorz )
        pState->mnState |= SAL_FRAMESTATE_MAXIMIZED_HORZ;
    if( maFrameData.mbMaximizedVert )
        pState->mnState |= SAL_FRAMESTATE_MAXIMIZED_VERT;
    if( maFrameData.mbShaded )
        pState->mnState |= SAL_FRAMESTATE_ROLLUP;

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
    Display* pDisplay = _GetXDisplay();

    // make sure the frame has been reparented and all paint timer have been
    // expired
    maFrameData.maResizeTimer.Stop();
    if ( maFrameData.mbMoved || maFrameData.mbSized )
    {
        maFrameData.HandleResizeTimer (&maFrameData.maResizeTimer);
    }

    do
    {
        XSync(pDisplay, False);
        Application::Reschedule ();
    }
    while (XPending(pDisplay));
    usleep (50000);
    do
    {
        XSync(pDisplay, False);
        Application::Reschedule ();
    }
    while (XPending(pDisplay));

    // get the most outer window, usually the window manager decoration
    Drawable hWindow = None;
    if (maFrameData.IsOverrideRedirect())
        hWindow = _GetDrawable();
    else
    if (hPresentationWindow != None)
        hWindow = hPresentationWindow;
    else
        hWindow = maFrameData.GetStackingWindow();

    // query the contents of the window
    if (hWindow != None)
    {
        SalBitmap *pBmp = new SalBitmap;
        if (pBmp->SnapShot (pDisplay, hWindow))
            return pBmp;
        else
            delete pBmp;
    }

    return NULL;
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void SalFrameData::GetPosSize( Rectangle &rPosSize )
{
    if( pFrame_->maGeometry.nWidth < 1 || pFrame_->maGeometry.nHeight < 1 )
    {
        long w = pDisplay_->GetScreenSize().Width()  - pFrame_->maGeometry.nLeftDecoration - pFrame_->maGeometry.nRightDecoration;
        long h = pDisplay_->GetScreenSize().Height() - pFrame_->maGeometry.nTopDecoration - pFrame_->maGeometry.nBottomDecoration;

        rPosSize = Rectangle( Point( pFrame_->maGeometry.nX, pFrame_->maGeometry.nY ), Size( w, h ) );
    }
    else
        rPosSize = Rectangle( Point( pFrame_->maGeometry.nX, pFrame_->maGeometry.nY ),
                              Size( pFrame_->maGeometry.nWidth, pFrame_->maGeometry.nHeight ) );
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void SalFrameData::SetSize( const Size &rSize )
{
    if( rSize.Width() > 0 && rSize.Height() > 0 )
    {
         if( ! ( nStyle_ & SAL_FRAME_STYLE_SIZEABLE )
            && ! ( nStyle_ & SAL_FRAME_STYLE_CHILD )
            && ! ( nStyle_ & SAL_FRAME_STYLE_FLOAT ) )
         {
            XSizeHints* pHints = XAllocSizeHints();
            long nSupplied = 0;
            XGetWMNormalHints( GetXDisplay(),
                               GetShellWindow(),
                               pHints,
                               &nSupplied
                               );
            pHints->min_width   = rSize.Width();
            pHints->min_height  = rSize.Height();
            pHints->max_width   = rSize.Width();
            pHints->max_height  = rSize.Height();
            pHints->flags |= PMinSize | PMaxSize;
            XSetWMNormalHints( GetXDisplay(),
                               GetShellWindow(),
                               pHints );
            XFree( pHints );
         }
        XResizeWindow( GetXDisplay(), GetShellWindow(), rSize.Width(), rSize.Height() );
        if( GetWindow() != GetShellWindow() )
            XMoveResizeWindow( GetXDisplay(), GetWindow(), 0, 0, rSize.Width(), rSize.Height() );

        pFrame_->maGeometry.nWidth  = rSize.Width();
        pFrame_->maGeometry.nHeight = rSize.Height();

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

    if( !values.width || !values.height )
        return;

     if( mpParent )
     {
        // --- RTL --- (mirror window pos)
        if( Application::GetSettings().GetLayoutRTL() )
            values.x = mpParent->maGeometry.nWidth-values.width-1-values.x;

         XLIB_Window aChild;
         // coordinates are relative to parent, so translate to root coordinates
         XTranslateCoordinates( GetDisplay()->GetDisplay(),
                                mpParent->maFrameData.GetWindow(),
                                GetDisplay()->GetRootWindow(),
                                values.x, values.y,
                                &values.x, &values.y,
                                & aChild );
     }

    bool bMoved = false;
    bool bSized = false;
    if( values.x != pFrame_->maGeometry.nX || values.y != pFrame_->maGeometry.nY )
        bMoved = true;
    if( values.width != (int)pFrame_->maGeometry.nWidth || values.height != (int)pFrame_->maGeometry.nHeight )
        bSized = true;

    if( ! ( nStyle_ & ( SAL_FRAME_STYLE_CHILD | SAL_FRAME_STYLE_FLOAT ) )
        && !(pDisplay_->GetProperties() & PROPERTY_SUPPORT_WM_ClientPos) )
    {
        values.x    -= pFrame_->maGeometry.nLeftDecoration;
        values.y    -= pFrame_->maGeometry.nTopDecoration;
    }

    if( ( ! ( nStyle_ & SAL_FRAME_STYLE_CHILD )
          && ! ( nStyle_ & SAL_FRAME_STYLE_FLOAT ) )
        && ( nShowState_ == SHOWSTATE_UNKNOWN ||
             nShowState_ == SHOWSTATE_HIDDEN ||
             ! ( nStyle_ & SAL_FRAME_STYLE_SIZEABLE )
             )
        )
    {
        XSizeHints* pHints = XAllocSizeHints();
        long nSupplied = 0;
        XGetWMNormalHints( GetXDisplay(),
                           GetShellWindow(),
                           pHints,
                           &nSupplied
                           );
        if( ! ( nStyle_ & SAL_FRAME_STYLE_SIZEABLE ) )
        {
            pHints->min_width   = rPosSize.GetWidth();
            pHints->min_height  = rPosSize.GetHeight();
            pHints->max_width   = rPosSize.GetWidth();
            pHints->max_height  = rPosSize.GetHeight();
            pHints->flags |= PMinSize | PMaxSize;
        }
        if( nShowState_ == SHOWSTATE_UNKNOWN || nShowState_ == SHOWSTATE_HIDDEN )
        {
            pHints->flags |= PPosition | PWinGravity;
            pHints->x           = values.x;
            pHints->y           = values.y;
            pHints->win_gravity = pDisplay_->getWMAdaptor()->getPositionWinGravity();
        }
        XSetWMNormalHints( GetXDisplay(),
                           GetShellWindow(),
                           pHints );
        XFree( pHints );
    }

    XMoveResizeWindow( GetXDisplay(), GetShellWindow(), values.x, values.y, values.width, values.height );
    if( GetShellWindow() != GetWindow() )
        XMoveResizeWindow( GetXDisplay(), GetWindow(), 0, 0, values.width, values.height );

    pFrame_->maGeometry.nX      = values.x;
    pFrame_->maGeometry.nY      = values.y;
    pFrame_->maGeometry.nWidth  = values.width;
    pFrame_->maGeometry.nHeight = values.height;
    if( bSized && ! bMoved )
        Call ( SALEVENT_RESIZE, NULL );
    else if( bMoved && ! bSized )
        Call ( SALEVENT_MOVE, NULL );
    else
        Call ( SALEVENT_MOVERESIZE, NULL );

    // allow the external status window to reposition
    if (mbInputFocus && mpInputContext != NULL)
        mpInputContext->SetICFocus ( pFrame_ );
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void SalFrameData::Minimize()
{
    if( SHOWSTATE_UNKNOWN == nShowState_ || SHOWSTATE_HIDDEN == nShowState_ )
    {
        stderr0( "SalFrameData::Minimize on withdrawn window\n" );
        return;
    }

    if( XIconifyWindow( GetXDisplay(),
                        GetShellWindow(),
                        pDisplay_->GetScreenNumber() ) )
        nShowState_ = SHOWSTATE_MINIMIZED;
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void SalFrameData::Maximize()
{
    if( SHOWSTATE_MINIMIZED == nShowState_ )
    {
        XMapWindow( GetXDisplay(), GetShellWindow() );
        nShowState_ = SHOWSTATE_NORMAL;
    }

    pDisplay_->getWMAdaptor()->maximizeFrame( pFrame_, true, true );
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void SalFrameData::Restore()
{
    if( SHOWSTATE_UNKNOWN == nShowState_ || SHOWSTATE_HIDDEN == nShowState_ )
    {
        stderr0( "SalFrameData::Restore on withdrawn window\n" );
        return;
    }

    if( SHOWSTATE_MINIMIZED == nShowState_ )
    {
        XMapWindow( GetXDisplay(), GetShellWindow() );
        nShowState_ = SHOWSTATE_NORMAL;
    }

    pDisplay_->getWMAdaptor()->maximizeFrame( pFrame_, false, false );
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

void SalFrame::ShowFullScreen( BOOL bFullScreen )
{
    if( maFrameData.mbFullScreen == (bool)bFullScreen )
        return;

    maFrameData.pDisplay_->getWMAdaptor()->showFullScreen( this, bFullScreen );
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
    I18NStatus::get().show( !bStart, I18NStatus::presentation );
    if ( bStart )
        MessageToXAutoLock( _GetXDisplay(), XAUTOLOCK_DISABLE );
    else
        MessageToXAutoLock( _GetXDisplay(), XAUTOLOCK_ENABLE );

    if( ! bStart && hPresentationWindow != None )
        doReparentPresentationDialogues( _GetDisplay() );
    hPresentationWindow = (bStart && maFrameData.IsOverrideRedirect() ) ? maFrameData.GetWindow() : None;
    if( bStart || maFrameData.nScreenSaversTimeout_ )
    {
        if( hPresentationWindow )
        {
            /*  #i10559# workaround for WindowMaker: try to restore
             *  current focus after presentation window is gone
             */
            int revert_to = 0;
            XGetInputFocus( _GetXDisplay(), &hPresFocusWindow, &revert_to );
        }
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
    XDefineCursor( GetXDisplay(), GetWindow(), hCursor_ );

    if( IsCaptured() || nVisibleFloats > 0 )
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
     */
    unsigned int nWindowLeft = maGeometry.nX + nX;
    unsigned int nWindowTop  = maGeometry.nY + nY;

    XWarpPointer( _GetXDisplay(), None, maFrameData.pDisplay_->GetRootWindow(),
                  0, 0, 0, 0, nWindowLeft, nWindowTop);
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
    if( ! ( (maFrameData.nStyle_ & SAL_FRAME_STYLE_CHILD ) ||
            (maFrameData.nStyle_ & SAL_FRAME_STYLE_FLOAT ) ) )
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
    {
        if( maFrameData.mpInputContext )
            maFrameData.mpInputContext->Unmap( this );
        return;
    }

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
    else
        maFrameData.mpInputContext->Map( this );
      return;
}

// -----------------------------------------------------------------------

void SalFrame::EndExtTextInput( USHORT nFlags )
{
    if (maFrameData.mpInputContext != NULL)
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

LanguageType SalFrame::GetInputLanguage()
{
    // could be improved by checking unicode ranges of the last input
    return LANGUAGE_DONTKNOW;
}

// Settings
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

inline Color getColorFromLong( long nColor )
{
    return Color( (nColor & 0xff), (nColor & 0xff00)>>8, (nColor & 0xff0000)>>16);
}

void SalFrame::UpdateSettings( AllSettings& rSettings )
{

    DtIntegrator* pIntegrator = DtIntegrator::CreateDtIntegrator( this );
#if OSL_DEBUG_LEVEL > 1
    fprintf( stderr, "DtIntegrator: %d\n", pIntegrator ? pIntegrator->GetDtType() : -1 );
#endif
    if( pIntegrator )
        pIntegrator->GetSystemLook( rSettings );
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

void SalFrame::SetParent( SalFrame* pNewParent )
{
    if( maFrameData.mpParent != pNewParent )
    {
        if( maFrameData.mpParent )
            maFrameData.mpParent->maFrameData.maChildren.remove( this );

        maFrameData.mpParent = pNewParent;
        pNewParent->maFrameData.maChildren.push_back( this );
        maFrameData.GetDisplay()->getWMAdaptor()->changeReferenceFrame( this, pNewParent );
    }
}

SalFrame* SalFrame::GetParent() const
{
    return maFrameData.mpParent;
}

bool SalFrameData::SetPluginParent( SystemParentData* pNewParent )
{
    // plugin parent may be killed unexpectedly by
    // plugging process; ignore XErrors in that case
    GetDisplay()->GetXLib()->SetIgnoreXErrors( TRUE );

    // first deinit frame
    if( mpInputContext )
    {
        mpInputContext->UnsetICFocus( pFrame_ );
        mpInputContext->Unmap( pFrame_ );
    }
    if( GetWindow() == hPresentationWindow )
    {
        hPresentationWindow = None;
        doReparentPresentationDialogues( GetDisplay() );
    }
    XDestroyWindow( GetXDisplay(), mhWindow );
    mhWindow = None;

    passOnSaveYourSelf();

    // now init with new parent again
    Init( nStyle_ | SAL_FRAME_STYLE_CHILD, pNewParent );

    // update graphics if necessary
    if( pGraphics_ )
        pGraphics_->maGraphicsData.SetDrawable( mhWindow );

    return true;
}

bool SalFrame::SetPluginParent( SystemParentData* pNewParent )
{
    return maFrameData.SetPluginParent( pNewParent );
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

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

ULONG SalFrame::GetCurrentModButtons()
{
    XLIB_Window aRoot, aChild;
    int rx, ry, wx, wy;
    unsigned int nMask = 0;
    XQueryPointer( maFrameData.GetXDisplay(),
                   maFrameData.GetShellWindow(),
                   &aRoot,
                   &aChild,
                   &rx, &ry,
                   &wx, &wy,
                   &nMask
                   );
    return sal_GetCode( nMask );
}

long SalFrameData::HandleMouseEvent( XEvent *pEvent )
{
    SalMouseEvent       aMouseEvt;
    USHORT              nEvent = 0;
    bool                bClosePopups = false;

    if( nVisibleFloats && pEvent->type == EnterNotify )
        return 0;

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
        /*
         *  #89075# #89335#
         *
         *  some WMs (and/or) applications  have a passive grab on
         *  mouse buttons (XGrabButton). This leads to enter/leave notifies
         *  with mouse buttons pressed in the state mask before the actual
         *  ButtonPress event gets dispatched. But EnterNotify
         *  is reported in vcl as MouseMove event. Some office code
         *  decides that a pressed button in a MouseMove belongs to
         *  a drag operation which leads to doing things differently.
         *
         *  #95901#
         *  ignore Enter/LeaveNotify resulting from grabs so that
         *  help windows do not disappear just after appearing
         *
         *  hopefully this workaround will not break anything.
         */
        if( pEvent->xcrossing.mode == NotifyGrab || pEvent->xcrossing.mode == NotifyUngrab  )
            return 0;

        aMouseEvt.mnX       = pEvent->xcrossing.x;
        aMouseEvt.mnY       = pEvent->xcrossing.y;
        aMouseEvt.mnTime    = pEvent->xcrossing.time;
        aMouseEvt.mnCode    = sal_GetCode( pEvent->xcrossing.state );
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
        if( nVisibleFloats > 0 && mpParent )
        {
            XLIB_Cursor aCursor = mpParent->maFrameData.GetCursor();
            if( pEvent->xmotion.x >= 0 && pEvent->xmotion.x < (int)pFrame_->maGeometry.nWidth &&
                pEvent->xmotion.y >= 0 && pEvent->xmotion.y < (int)pFrame_->maGeometry.nHeight )
                aCursor = None;

            XChangeActivePointerGrab( GetXDisplay(),
                                      PointerMotionMask|ButtonPressMask|ButtonReleaseMask,
                                      aCursor,
                                      CurrentTime );
        }
    }
    else
    {
        // let mouse events reach the correct window
        if( nVisibleFloats < 1 )
            XUngrabPointer( GetXDisplay(), CurrentTime );
        else if( pEvent->type == ButtonPress )
        {
            // see if the user clicks outside all of the floats
            // if yes release the grab
            bool bInside = false;
             for( SalFrame* pFrame = GetSalData()->pFirstFrame_; pFrame; pFrame = pFrame->maFrameData.pNextFrame_ )
            {
                if( pFrame->maFrameData.IsFloatGrabWindow()                                     &&
                    pFrame->maFrameData.bMapped_                                                &&
                    pEvent->xbutton.x_root >= pFrame->maGeometry.nX                             &&
                    pEvent->xbutton.x_root < pFrame->maGeometry.nX + (int)pFrame->maGeometry.nWidth &&
                    pEvent->xbutton.y_root >= pFrame->maGeometry.nY                             &&
                    pEvent->xbutton.y_root < pFrame->maGeometry.nY + (int)pFrame->maGeometry.nHeight )
                {
                    bInside = true;
                    break;
                }
            }
            if( ! bInside )
            {
                // need not take care of the XUngrabPointer in Show( FALSE )
                // because XUngrabPointer does not produce errors if pointer
                // is not grabbed
                XUngrabPointer( GetXDisplay(), CurrentTime );
                bClosePopups = true;

                /*  #i15246# only close popups if pointer is outside all our frames
                 *  cannot use our own geometry data here because stacking
                 *  is unknown (the above case implicitly assumes
                 *  that floats are on top which should be true)
                 */
                XLIB_Window aRoot, aChild;
                int root_x, root_y, win_x, win_y;
                unsigned int mask_return;
                if( XQueryPointer( GetXDisplay(), GetDisplay()->GetRootWindow(),
                                   &aRoot, &aChild,
                                   &root_x, &root_y,
                                   &win_x, &win_y,
                                   &mask_return )
                    && aChild // pointer may not be in any child
                    )
                {
                    for( SalFrame* pFrame = GetSalData()->pFirstFrame_; pFrame; pFrame = pFrame->maFrameData.pNextFrame_ )
                    {
                        if( ! pFrame->maFrameData.IsFloatGrabWindow()
                            && ( pFrame->maFrameData.GetWindow() == aChild ||
                                 pFrame->maFrameData.GetShellWindow() == aChild ||
                                 pFrame->maFrameData.GetStackingWindow() == aChild )
                            )
                        {
                            bClosePopups = false;
                            break;
                        }
                    }
                }
            }
        }

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
            if( pEvent->type == ButtonRelease )
                return 0;

            static ULONG        nLines = 0;
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

            // --- RTL --- (mirror mouse pos)
            if( Application::GetSettings().GetLayoutRTL() )
                aWheelEvt.mnX = nWidth_-1-aWheelEvt.mnX;
            return Call( nEvent, &aWheelEvt );
        }
    }

    int nRet = 0;
    if( nEvent == SALEVENT_MOUSELEAVE
        || ( aMouseEvt.mnX <  nWidth_  && aMouseEvt.mnX >  -1 &&
             aMouseEvt.mnY <  nHeight_ && aMouseEvt.mnY >  -1 )
        || pDisplay_->MouseCaptured( this )
        )
    {
        // --- RTL --- (mirror mouse pos)
        if( Application::GetSettings().GetLayoutRTL() )
            aMouseEvt.mnX = nWidth_-1-aMouseEvt.mnX;
        nRet = Call( nEvent, &aMouseEvt );
    }

    if( bClosePopups )
    {
        /*  #108213# close popups after dispatching the event outside the popup;
         *  applications do weird things.
         */
        ImplSVData* pSVData = ImplGetSVData();
        if ( pSVData->maWinData.mpFirstFloat )
        {
            static const char* pEnv = getenv( "SAL_FLOATWIN_NOAPPFOCUSCLOSE" );
            if ( !(pSVData->maWinData.mpFirstFloat->GetPopupModeFlags() & FLOATWIN_POPUPMODE_NOAPPFOCUSCLOSE) && !(pEnv && *pEnv) )
                pSVData->maWinData.mpFirstFloat->EndPopupMode( FLOATWIN_POPUPMODEEND_CANCEL | FLOATWIN_POPUPMODEEND_CLOSEALL );
        }
    }

    return nRet;
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

    SalKeyEvent aKeyEvt;
    USHORT      nKeyCode;
    USHORT nModCode = 0;
    char        aDummy;

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
        aModEvt.mnModKeyCode = 0;
        if( pEvent->type == XLIB_KeyPress && mnExtKeyMod == 0 )
            mbSendExtKeyModChange = true;
        else if( pEvent->type == KeyRelease && mbSendExtKeyModChange )
        {
            aModEvt.mnModKeyCode = mnExtKeyMod;
            mnExtKeyMod = 0;
        }

        // pressing just the ctrl key leads to a keysym of XK_Control but
        // the event state does not contain ControlMask. In the release
        // event its the other way round: it does contain the Control mask.
        // The modifier mode therefore has to be adapted manually.
        USHORT nExtModMask = 0;
        USHORT nModMask = 0;
        switch( nKeySym )
        {
            case XK_Control_L:
                nExtModMask = MODKEY_LMOD1;
                nModMask = KEY_MOD1;
                break;
            case XK_Control_R:
                nExtModMask = MODKEY_RMOD1;
                nModMask = KEY_MOD1;
                break;
            case XK_Alt_L:
                nExtModMask = MODKEY_LMOD2;
                nModMask = KEY_MOD2 | (pEvent->type==KeyRelease ? KEY_CONTROLMOD : 0);
                break;
            case XK_Alt_R:
                nExtModMask = MODKEY_RMOD2;
                nModMask = KEY_MOD2 | (pEvent->type==KeyRelease ? KEY_CONTROLMOD : 0);
                break;
            case XK_Shift_L:
                nExtModMask = MODKEY_LSHIFT;
                nModMask = KEY_SHIFT;
                break;
            case XK_Shift_R:
                nExtModMask = MODKEY_RSHIFT;
                nModMask = KEY_SHIFT;
                break;
        }
        if( pEvent->type == KeyRelease )
        {
            nModCode &= ~nModMask;
            mnExtKeyMod &= ~nExtModMask;
        }
        else
        {
            nModCode |= nModMask;
            mnExtKeyMod |= nExtModMask;
        }

        aModEvt.mnCode = nModCode;
        aModEvt.mnTime = pEvent->time;

        int nRet = Call( SALEVENT_KEYMODCHANGE, &aModEvt );

        // emulate KEY_MENU
        if ( ( (nKeySym == XK_Alt_L) || (nKeySym == XK_Alt_R) ) &&
             ( (nModCode & ~(KEY_CONTROLMOD|KEY_MOD2)) == 0 ) )
        {
            if( pEvent->type == XLIB_KeyPress )
                mbKeyMenu = true;
            else if( mbKeyMenu )
            {
                // simulate KEY_MENU
                aKeyEvt.mnCode     = KEY_MENU | nModCode;
                aKeyEvt.mnRepeat   = 0;
                aKeyEvt.mnTime     = pEvent->time;
                aKeyEvt.mnCharCode = 0;
                nRet = Call( SALEVENT_KEYINPUT, &aKeyEvt );
                nRet = Call( SALEVENT_KEYUP, &aKeyEvt );
            }
        }
        else
            mbKeyMenu = false;
        return nRet;
    }

    mbSendExtKeyModChange = mbKeyMenu = false;

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

    rtl_TextEncoding nEncoding;

    if (mpInputContext != NULL && mpInputContext->IsMultiLingual() )
        nEncoding = RTL_TEXTENCODING_UTF8;
    else
        nEncoding = osl_getThreadTextEncoding();

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
    // #107739# ReflectionX in Windows mode changes focus while mouse is grabbed
    if( nVisibleFloats > 0 && GetDisplay()->getWMAdaptor()->getWindowManagerName().EqualsAscii( "ReflectionX Windows" ) )
        return 1;

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
        {
            /*
             *  do not unset the IC focuse here because would kill
             *  a lookup choice windows that might have the focus now
             *      mpInputContext->UnsetICFocus( pFrame_ );
             */
            I18NStatus::get().show( false, I18NStatus::focus );
        }
    }


    if ( pEvent->mode == NotifyNormal || pEvent->mode == NotifyWhileGrabbed ||
         ( ( nStyle_ & SAL_FRAME_STYLE_CHILD ) && pEvent->window == GetShellWindow() )
         )
    {
        if( hPresentationWindow != None && hPresentationWindow != GetShellWindow() )
            return 0;

        if( FocusIn == pEvent->type )
        {
#ifndef _USE_PRINT_EXTENSION_
            if( GetSalData()->pFirstInstance_->maInstData.mbPrinterInit )
                vcl_sal::PrinterUpdate::update();
#endif
            mbInputFocus = True;
            ImplSVData* pSVData = ImplGetSVData();



            long nRet = Call( SALEVENT_GETFOCUS,  0 );
            if ((mpParent != NULL && nStyle_ == 0)
                && pSVData->maWinData.mpFirstFloat )
            {
                ULONG nMode = pSVData->maWinData.mpFirstFloat->GetPopupModeFlags();
                pSVData->maWinData.mpFirstFloat->SetPopupModeFlags(
                                        nMode & ~(FLOATWIN_POPUPMODE_NOAPPFOCUSCLOSE));
            }
            return nRet;
        }
        else
        {
            mbInputFocus = False;
            mbSendExtKeyModChange = mbKeyMenu = false;
            mnExtKeyMod = 0;
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

    if( IsOverrideRedirect() && mbFullScreen &&
        aPresentationReparentList.begin() == aPresentationReparentList.end() )
        // we are in fullscreen mode -> override redirect
         // focus is possibly lost, so reget it
         XSetInputFocus( GetXDisplay(), GetShellWindow(), RevertToNone, CurrentTime );

    // width and height are extents, so they are of by one for rectangle
    maPaintRegion.Union( Rectangle( Point(aRect.x, aRect.y), Size(aRect.width+1, aRect.height+1) ) );

    if( nCount || maResizeTimer.IsActive() ) // wait for last expose rectangle
        return 1;

    SalPaintEvent aPEvt;

    aPEvt.mnBoundX          = maPaintRegion.Left();
    aPEvt.mnBoundY          = maPaintRegion.Top();
    aPEvt.mnBoundWidth      = maPaintRegion.GetWidth();
    aPEvt.mnBoundHeight     = maPaintRegion.GetHeight();

    // --- RTL --- (mirror paint rect)
    if( Application::GetSettings().GetLayoutRTL() )
        aPEvt.mnBoundX = nWidth_-aPEvt.mnBoundWidth-aPEvt.mnBoundX;

     Call( SALEVENT_PAINT, &aPEvt );
    maPaintRegion = Rectangle();

    return 1;
}

void SalFrameData::RestackChildren( XLIB_Window* pTopLevelWindows, int nTopLevelWindows )
{
    if( maChildren.begin() != maChildren.end() )
    {
        int nWindow = nTopLevelWindows;
        while( nWindow-- )
            if( pTopLevelWindows[nWindow] == GetStackingWindow() )
                break;
        if( nWindow < 0 )
            return;

        ::std::list< SalFrame* >::const_iterator it;
        for( it = maChildren.begin(); it != maChildren.end(); ++it )
        {
            SalFrameData* pData = &(*it)->maFrameData;
            if( pData->bMapped_ )
            {
                int nChild = nWindow;
                while( nChild-- )
                {
                    if( pTopLevelWindows[nChild] == pData->GetStackingWindow() )
                    {
                        // if a child is behind its parent, place it above the
                        // parent (for insane WMs like Dtwm and olwm)
                        XWindowChanges aCfg;
                        aCfg.sibling    = GetStackingWindow();
                        aCfg.stack_mode = Above;
                        XConfigureWindow( GetXDisplay(), pData->GetStackingWindow(), CWSibling|CWStackMode, &aCfg );
                        break;
                    }
                }
            }
        }
        for( it = maChildren.begin(); it != maChildren.end(); ++it )
        {
            SalFrameData* pData = &(*it)->maFrameData;
            pData->RestackChildren( pTopLevelWindows, nTopLevelWindows );
        }
    }
}

void SalFrameData::RestackChildren()
{
    if( ! GetDisplay()->getWMAdaptor()->isTransientBehaviourAsExpected()
        && maChildren.begin() != maChildren.end() )
    {
        XLIB_Window aRoot, aParent, *pChildren = NULL;
        unsigned int nChildren;
        if( XQueryTree( GetXDisplay(),
                        GetDisplay()->GetRootWindow(),
                        &aRoot,
                        &aParent,
                        &pChildren,
                        &nChildren ) )
        {
            RestackChildren( pChildren, nChildren );
            XFree( pChildren );
        }
    }
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
long SalFrameData::HandleSizeEvent( XConfigureEvent *pEvent )
{
    if (   pEvent->window != GetShellWindow()
           && pEvent->window != GetWindow()
           && pEvent->window != GetForeignParent()
           && pEvent->window != GetStackingWindow()
           )
    {
        // could be as well a sys-child window (aka SalObject)
        return 1;
    }

     // ignore for now
     if( nStyle_ & SAL_FRAME_STYLE_FLOAT )
         return 1;

    if( ( nStyle_ & SAL_FRAME_STYLE_CHILD ) && pEvent->window == GetShellWindow() )
    {
        // just update the children's positions
        RestackChildren();
        return 1;
    }

    if( pEvent->window == GetForeignParent() )
        XResizeWindow( GetXDisplay(),
                       GetWindow(),
                       pEvent->width,
                       pEvent->height );

    XLIB_Window hDummy;
    XTranslateCoordinates( GetXDisplay(),
                           GetWindow(),
                           pDisplay_->GetRootWindow(),
                           0, 0,
                           &pEvent->x, &pEvent->y,
                           &hDummy );

    if( pEvent->window == GetStackingWindow() )
    {
        if( pFrame_->maGeometry.nX != pEvent->x || pFrame_->maGeometry.nY != pEvent->y )
        {
            pFrame_->maGeometry.nX = pEvent->x;
            pFrame_->maGeometry.nY = pEvent->y;
            Call( SALEVENT_MOVE, NULL );
        }
        return 1;
    }

    // check size hints in first time SalFrame::Show
    if( SHOWSTATE_UNKNOWN == nShowState_ && bMapped_ )
        nShowState_ = SHOWSTATE_NORMAL;

    nWidth_     = pEvent->width;
    nHeight_    = pEvent->height;

    if( !mbMoved )
        mbMoved = ( pEvent->x != pFrame_->maGeometry.nX || pEvent->y != pFrame_->maGeometry.nY );
    if( !mbSized )
        mbSized = ( pEvent->width != pFrame_->maGeometry.nWidth || pEvent->height != pFrame_->maGeometry.nHeight );

    maResizeTimer.Start();

    pFrame_->maGeometry.nX      = pEvent->x;
    pFrame_->maGeometry.nY      = pEvent->y;
    pFrame_->maGeometry.nWidth  = pEvent->width;
    pFrame_->maGeometry.nHeight = pEvent->height;

    return 1;
}

IMPL_LINK( SalFrameData, HandleAlwaysOnTopRaise, void*, pDummy )
{
    if( bMapped_ )
        pFrame_->ToTop( 0 );
    return 0;
}

IMPL_LINK( SalFrameData, HandleResizeTimer, void*, pDummy )
{
    // update children's position
    RestackChildren();

    if( mbSized && ! mbMoved )
        Call ( SALEVENT_RESIZE, NULL );
    else if( mbMoved && ! mbSized )
        Call ( SALEVENT_MOVE, NULL );
    else if( mbMoved && mbSized )
        Call ( SALEVENT_MOVERESIZE, NULL );

    mbMoved = mbSized = false;

    SalPaintEvent aPEvt;

    aPEvt.mnBoundX          = maPaintRegion.Left();
    aPEvt.mnBoundY          = maPaintRegion.Top();
    aPEvt.mnBoundWidth      = maPaintRegion.GetWidth();
    aPEvt.mnBoundHeight     = maPaintRegion.GetHeight();

    if( Application::GetSettings().GetLayoutRTL() )
        aPEvt.mnBoundX = nWidth_-aPEvt.mnBoundWidth-aPEvt.mnBoundX;

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
    unsigned int    nChildren;
    BOOL            bNone = pDisplay_->GetProperties()
                            & PROPERTY_SUPPORT_WM_Parent_Pixmap_None;
    BOOL            bAccessParentWindow = ! (pDisplay_->GetProperties()
                            & PROPERTY_FEATURE_TrustedSolaris);

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
    Children = NULL;
        XQueryTree( pDisplay,
                    hWM_Parent,
                    &hRoot,
                    &hDummy,
                    &Children,
                    &nChildren );
        /* #107048# this sometimes happens if a Show(TRUE) is
         *  immediately followed by Show(FALSE) (which is braindead anyway)
         */
        if(  hDummy == hWM_Parent )
            hDummy = hRoot;
        if( hDummy != hRoot )
        {
            hWM_Parent = hDummy;
            if( bAccessParentWindow && bNone )
                XSetWindowBackgroundPixmap( pDisplay, hWM_Parent, None );
        }
        if( Children )
            XFree( Children );
    } while( hDummy != hRoot );

    if( GetStackingWindow() == None
        && hWM_Parent != hPresentationWindow
        && hWM_Parent != GetShellWindow()
        && ( ! pDisableStackingCheck || ! *pDisableStackingCheck )
        )
    {
        mhStackingWindow = hWM_Parent;
        if (bAccessParentWindow)
            XSelectInput( pDisplay, GetStackingWindow(), StructureNotifyMask );
    }

    if(     hWM_Parent == pDisplay_->GetRootWindow()
            ||  hWM_Parent == GetForeignParent()
            ||  pEvent->parent == pDisplay_->GetRootWindow()
            || ( nStyle_ & SAL_FRAME_STYLE_FLOAT ) )
    {
        // Reparenting before Destroy
        aPresentationReparentList.remove( GetStackingWindow() );
        mhStackingWindow = None;
        return 0;
    }

    /*
     *  evil hack to show decorated windows on top
     *  of override redirect presentation windows:
     *  reparent the window manager window to the presentation window
     *  does not work with non-reparenting WMs
     *  in future this should not be necessary anymore with
     *  _NET_WM_STATE_FULLSCREEN available
     */
    if( hPresentationWindow != None
        && hPresentationWindow != GetWindow()
        && GetStackingWindow() != None
        && GetStackingWindow() != GetDisplay()->GetRootWindow()
        )
    {
        int x = 0, y = 0;
        XLIB_Window aChild;
        XTranslateCoordinates( GetXDisplay(),
                               GetStackingWindow(),
                               GetDisplay()->GetRootWindow(),
                               0, 0,
                               &x, &y,
                               &aChild
                               );
        XReparentWindow( GetXDisplay(),
                         GetStackingWindow(),
                         hPresentationWindow,
                         x, y
                         );
        aPresentationReparentList.push_back( GetStackingWindow() );
    }

#ifdef NC_EVENTS
    XQueryTree( pDisplay,
                hWM_Parent,
                &hRoot,
                &hDummy,
                &Children,
                &nChildren );

    for( n = 0; n < nChildren; n++ )
        if( Children[n] != hShell_ && Children[n] != pEvent->parent )
            XSelectInput( pDisplay, Children[n], NC_EVENTS );
    XFree( Children );
#endif

    int nLeft = 0, nTop = 0;
    XTranslateCoordinates( GetXDisplay(),
                           GetShellWindow(),
                           hWM_Parent,
                           0, 0,
                           &nLeft,
                           &nTop,
                           &hDummy );
    pFrame_->maGeometry.nLeftDecoration = nLeft > 0 ? nLeft-1 : 0;
    pFrame_->maGeometry.nTopDecoration  = nTop  > 0 ? nTop-1  : 0;

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
    pFrame_->maGeometry.nRightDecoration    = wp - w - pFrame_->maGeometry.nLeftDecoration;
    pFrame_->maGeometry.nBottomDecoration   = hp - h - pFrame_->maGeometry.nTopDecoration;
    /*
     *  note: this works because hWM_Parent is direct child of root,
     *  not necessarily parent of GetShellWindow()
     */
    pFrame_->maGeometry.nX      = xp + pFrame_->maGeometry.nLeftDecoration;
    pFrame_->maGeometry.nY      = yp + pFrame_->maGeometry.nTopDecoration;
    bool bResized = w != pFrame_->maGeometry.nWidth || h != pFrame_->maGeometry.nHeight;
    pFrame_->maGeometry.nWidth  = w;
    pFrame_->maGeometry.nHeight = h;

    // limit width and height if we are too large: #47757
    // olwm and fvwm need this, it doesnt harm the rest

    int nScreenWidth  = pDisplay_->GetScreenSize().Width();
    int nScreenHeight = pDisplay_->GetScreenSize().Height();
    int nFrameWidth   = pFrame_->maGeometry.nWidth + pFrame_->maGeometry.nLeftDecoration + pFrame_->maGeometry.nRightDecoration;
    int nFrameHeight  = pFrame_->maGeometry.nHeight + pFrame_->maGeometry.nTopDecoration  + pFrame_->maGeometry.nBottomDecoration;

    if ((nFrameWidth > nScreenWidth) || (nFrameHeight > nScreenHeight))
    {
        Size aSize(pFrame_->maGeometry.nWidth, pFrame_->maGeometry.nHeight);

        if (nFrameWidth  > nScreenWidth)
            aSize.Width()  = nScreenWidth  - pFrame_->maGeometry.nRightDecoration - pFrame_->maGeometry.nLeftDecoration;
        if (nFrameHeight > nScreenHeight)
            aSize.Height() = nScreenHeight - pFrame_->maGeometry.nBottomDecoration - pFrame_->maGeometry.nTopDecoration;

        SetSize (aSize);
    }
    else if( bResized )
        Call( SALEVENT_RESIZE, NULL );

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
                                 GetShellWindow(),
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
    else if( pEvent->message_type == rWMAdaptor.getAtom( WMAdaptor::WM_PROTOCOLS )
             && ! ( nStyle_ & SAL_FRAME_STYLE_CHILD )
             && ! ( nStyle_ & SAL_FRAME_STYLE_FLOAT )
             )
    {
        if( (Atom)pEvent->data.l[0] == rWMAdaptor.getAtom( WMAdaptor::WM_DELETE_WINDOW ) )
        {
            Close();
            return 1;
        }
        else if( (Atom)pEvent->data.l[0] == rWMAdaptor.getAtom( WMAdaptor::WM_SAVE_YOURSELF ) )
        {
            if( pFrame_ == s_pSaveYourselfFrame && ! rWMAdaptor.getWindowManagerName().EqualsAscii( "Dtwm" ) )
            {
                ByteString aExec( SessionManagerClient::getExecName(), osl_getThreadTextEncoding() );
                char* argv[2];
                argv[0] = "/bin/sh";
                argv[1] = const_cast<char*>(aExec.GetBuffer());
#if OSL_DEBUG_LEVEL > 1
                fprintf( stderr, "SaveYourself request, setting command: %s %s\n", argv[0], argv[1] );
#endif
                XSetCommand( GetXDisplay(), GetShellWindow(), argv, 2 );
            }
            else // can only happen in race between WM and window closing
                XChangeProperty( GetXDisplay(), GetShellWindow(), rWMAdaptor.getAtom( WMAdaptor::WM_COMMAND ), XA_STRING, 8, PropModeReplace, (unsigned char*)"", 0 );

            if( rWMAdaptor.getWindowManagerName().EqualsAscii( "Dtwm" ) )
            {
                // save open documents; would be good for non Dtwm, too,
                // but there is no real Shutdown message in the ancient
                // SM protocol; on Dtwm SaveYourself really means Shutdown, too.
                Application::EnableDialogCancel( TRUE );
                ApplicationEvent aEvent( String( RTL_CONSTASCII_USTRINGPARAM( "SessionManager" ) ),
                                         ApplicationAddress(),
                                         ByteString( APPEVENT_SAVEDOCUMENTS_STRING ),
                                         String( RTL_CONSTASCII_USTRINGPARAM( "All" ) ) );

                if( GetpApp() )
                    GetpApp()->AppEvent( aEvent );

#if OSL_DEBUG_LEVEL > 1
                fprintf( stderr, "sending ShutDown message\n" );
#endif
                ShutDown();
            }
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

    if( pEvent->xany.window == GetShellWindow() || pEvent->xany.window == GetWindow() )
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
                    if( nShowState_ == SHOWSTATE_HIDDEN )
                    {
                        /*
                         *  #95097# workaround for (at least) KWin 2.2.2
                         *  which will map windows that were once transient
                         *  even if they are withdrawn when the respective
                         *  document is mapped.
                         */
                        XUnmapWindow( GetXDisplay(), GetShellWindow() );
                        break;
                    }
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
                        maPaintRegion.Union( Rectangle( Point( 0, 0 ), Size( pFrame_->maGeometry.nWidth, pFrame_->maGeometry.nHeight ) ) );
                        if( ! maResizeTimer.IsActive() )
                        {
                            mbSized = mbMoved = true;
                            maResizeTimer.Start();
                        }
                    }

                    /*  #99570# another workaround for sawfish: if a transient window for the same parent is shown
                     *  sawfish does not set the focus to it. Applies only for click to focus mode.
                     */
                    if( ! (nStyle_ & SAL_FRAME_STYLE_FLOAT ) && mbInShow && GetDisplay()->getWMAdaptor()->getWindowManagerName().EqualsAscii( "Sawfish" ) )
                    {
                        // #101775# don't set the focus into the IME status window
                        // since this will lead to a parent loose-focus, close status,
                        // reget focus, open status, .... flicker loop
                        if ( (I18NStatus::get().getStatusFrame() != pFrame_) )
                            XSetInputFocus( GetXDisplay(), GetShellWindow(), RevertToParent, CurrentTime );
                    }

                    /*
                     *  sometimes a message box/dialogue is brought up when a frame is not mapped
                     *  the corresponding TRANSIENT_FOR hint is then set to the root window
                     *  so that the dialogue shows in all cases. Correct it here if the
                     *  frame is shown afterwards.
                     */
                    if( ! ( nStyle_ & ( SAL_FRAME_STYLE_FLOAT | SAL_FRAME_STYLE_CHILD ) )
                        && ! IsOverrideRedirect() )
                    {
                        for( ::std::list< SalFrame* >::const_iterator it = maChildren.begin();
                             it != maChildren.end(); ++it )
                        {
                            if( (*it)->maFrameData.mbTransientForRoot )
                                pDisplay_->getWMAdaptor()->changeReferenceFrame( *it, pFrame_ );
                        }
                    }

                    if( hPresentationWindow != None && GetShellWindow() == hPresentationWindow )
                        XSetInputFocus( GetXDisplay(), GetShellWindow(), RevertToParent, CurrentTime );
                    RestackChildren();
                    mbInShow = FALSE;
                }
                break;

            case UnmapNotify:
                if( pEvent->xunmap.window == GetShellWindow() )
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
                if( pEvent->xconfigure.window == GetShellWindow()
                    || pEvent->xconfigure.window == GetWindow() )
                    nRet = HandleSizeEvent( &pEvent->xconfigure );
                break;

            case VisibilityNotify:
                nVisibility_ = pEvent->xvisibility.state;
                nRet = TRUE;
                if( bAlwaysOnTop_
                    && bMapped_
                    && ! GetDisplay()->getWMAdaptor()->isAlwaysOnTopOK()
                    && nVisibility_ != VisibilityUnobscured )
                    maAlwaysOnTopRaiseTimer.Start();
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
                    nRet = pDisplay_->getWMAdaptor()->handlePropertyNotify( pFrame_, &pEvent->xproperty );
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
             case FocusIn:
             case FocusOut:
                if( ( nStyle_ & SAL_FRAME_STYLE_CHILD )
                    && ( pEvent->xfocus.window == GetShellWindow()
                         || pEvent->xfocus.window == GetForeignParent() )
                    )
                {
                    nRet = HandleFocusEvent( &pEvent->xfocus );
                }
                 break;

            case ConfigureNotify:
                if( pEvent->xconfigure.window == GetForeignParent() ||
                    pEvent->xconfigure.window == GetShellWindow() )
                    nRet = HandleSizeEvent( &pEvent->xconfigure );

                if( pEvent->xconfigure.window == GetStackingWindow() )
                    nRet = HandleSizeEvent( &pEvent->xconfigure );

                RestackChildren();
                break;
        }
    }

    return nRet;
}

