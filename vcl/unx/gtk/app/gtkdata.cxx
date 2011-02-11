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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"

#define _SV_SALDATA_CXX

// -=-= #includes =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

#include <unistd.h>
#include <fcntl.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>
#include <poll.h>
#if defined(FREEBSD) || defined(NETBSD)
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#endif
#include <plugins/gtk/gtkdata.hxx>
#include <plugins/gtk/gtkinst.hxx>
#include <plugins/gtk/gtkframe.hxx>
#include <salobj.h>
#include <osl/thread.h>
#include <osl/process.h>

#include <osl/conditn.h>
#include <tools/debug.hxx>
#include "i18n_im.hxx"
#include "i18n_xkb.hxx"
#include <wmadaptor.hxx>

#include "../../unx/source/inc/salcursors.h"

#include <vcl/svapp.hxx>

using namespace rtl;
using namespace vcl_sal;

/***************************************************************************
 * class GtkDisplay                                                        *
 ***************************************************************************/

GtkSalDisplay::GtkSalDisplay( GdkDisplay* pDisplay )
            : SalDisplay( gdk_x11_display_get_xdisplay( pDisplay ) ),
              m_pGdkDisplay( pDisplay ),
              m_bStartupCompleted( false )
{
    m_bUseRandRWrapper = false; // use gdk signal instead
    for(int i = 0; i < POINTER_COUNT; i++)
        m_aCursors[ i ] = NULL;
    Init ();
}

GtkSalDisplay::~GtkSalDisplay()
{
    if( !m_bStartupCompleted )
        gdk_notify_startup_complete();
    doDestruct();

    for(int i = 0; i < POINTER_COUNT; i++)
        if( m_aCursors[ i ] )
            gdk_cursor_unref( m_aCursors[ i ] );

    pDisp_ = NULL;
}

void GtkSalDisplay::deregisterFrame( SalFrame* pFrame )
{
    if( m_pCapture == pFrame )
    {
        static_cast<GtkSalFrame*>(m_pCapture)->grabPointer( FALSE );
        m_pCapture = NULL;
    }
    SalDisplay::deregisterFrame( pFrame );
}

extern "C" {
GdkFilterReturn call_filterGdkEvent( GdkXEvent* sys_event,
                                     GdkEvent* event,
                                     gpointer data )
{
    return GtkSalDisplay::filterGdkEvent( sys_event, event, data );
}

void signalKeysChanged( GdkKeymap*, gpointer data )
{
    GtkSalDisplay* pDisp = (GtkSalDisplay*)data;
    pDisp->GetKeyboardName(true);
}

void signalScreenSizeChanged( GdkScreen* pScreen, gpointer data )
{
    GtkSalDisplay* pDisp = (GtkSalDisplay*)data;
    pDisp->screenSizeChanged( pScreen );
}

void signalMonitorsChanged( GdkScreen* pScreen, gpointer data )
{
    GtkSalDisplay* pDisp = (GtkSalDisplay*)data;
    pDisp->monitorsChanged( pScreen );
}

}

GdkFilterReturn GtkSalDisplay::filterGdkEvent( GdkXEvent* sys_event,
                                               GdkEvent*,
                                               gpointer data )
{
    GdkFilterReturn aFilterReturn = GDK_FILTER_CONTINUE;

    XEvent *pEvent = (XEvent *)sys_event;
    GtkSalDisplay *pDisplay = (GtkSalDisplay *)data;

    // dispatch all XEvents to event callback
    if( GetSalData()->m_pInstance->
        CallEventCallback( pEvent, sizeof( XEvent ) ) )
        aFilterReturn = GDK_FILTER_REMOVE;

    GTK_YIELD_GRAB();

    if (pDisplay->GetDisplay() == pEvent->xany.display )
    {
        // #i53471# gtk has no callback mechanism that lets us be notified
        // when settings (as in XSETTING and opposed to styles) are changed.
        // so we need to listen for corresponding property notifications here
        // these should be rare enough so that we can assume that the settings
        // actually change when a corresponding PropertyNotify occurs
        if( pEvent->type == PropertyNotify &&
            pEvent->xproperty.atom == pDisplay->getWMAdaptor()->getAtom( WMAdaptor::XSETTINGS ) &&
            ! pDisplay->m_aFrames.empty()
           )
        {
            pDisplay->SendInternalEvent( pDisplay->m_aFrames.front(), NULL, SALEVENT_SETTINGSCHANGED );
        }
        // let's see if one of our frames wants to swallow these events
        // get the frame
        for( std::list< SalFrame* >::const_iterator it = pDisplay->m_aFrames.begin();
                 it != pDisplay->m_aFrames.end(); ++it )
        {
            GtkSalFrame* pFrame = static_cast<GtkSalFrame*>(*it);
            if( (GdkNativeWindow)pFrame->GetSystemData()->aWindow == pEvent->xany.window ||
                ( pFrame->getForeignParent() && pFrame->getForeignParentWindow() == pEvent->xany.window ) ||
                ( pFrame->getForeignTopLevel() && pFrame->getForeignTopLevelWindow() == pEvent->xany.window )
                )
            {
                if( ! pFrame->Dispatch( pEvent ) )
                    aFilterReturn = GDK_FILTER_REMOVE;
                break;
            }
        }
        X11SalObject::Dispatch( pEvent );
    }

    return aFilterReturn;
}

void GtkSalDisplay::screenSizeChanged( GdkScreen* pScreen )
{
    if( pScreen )
    {
        int nScreen = gdk_screen_get_number( pScreen );
        if( nScreen < static_cast<int>(m_aScreens.size()) )
        {
            ScreenData& rSD = const_cast<ScreenData&>(m_aScreens[nScreen]);
            if( rSD.m_bInit )
            {
                rSD.m_aSize = Size( gdk_screen_get_width( pScreen ),
                                    gdk_screen_get_height( pScreen ) );
                if( ! m_aFrames.empty() )
                    m_aFrames.front()->CallCallback( SALEVENT_DISPLAYCHANGED, 0 );
            }
        }
        else
        {
            DBG_ERROR( "unknown screen changed size" );
        }
    }
}

void GtkSalDisplay::monitorsChanged( GdkScreen* pScreen )
{
    if( pScreen )
    {
        if( gdk_display_get_n_screens(m_pGdkDisplay) == 1 )
        {
            int nScreen = gdk_screen_get_number( pScreen );
            if( nScreen == m_nDefaultScreen ) //To-Do, make m_aXineramaScreens a per-screen thing ?
            {
                gint nMonitors = gdk_screen_get_n_monitors(pScreen);
                m_aXineramaScreens = std::vector<Rectangle>();
                m_aXineramaScreenIndexMap = std::vector<int>(nMonitors);
                for (gint i = 0; i < nMonitors; ++i)
                {
                    GdkRectangle dest;
                    gdk_screen_get_monitor_geometry(pScreen, i, &dest);
                    addXineramaScreenUnique( i, dest.x, dest.y, dest.width, dest.height );
                }
                m_bXinerama = m_aXineramaScreens.size() > 1;
                if( ! m_aFrames.empty() )
                    m_aFrames.front()->CallCallback( SALEVENT_DISPLAYCHANGED, 0 );
            }
            else
            {
                DBG_ERROR( "monitors for non-default screen changed, extend-me" );
            }
        }
    }
}

extern "C"
{
    typedef gint(* screen_get_primary_monitor)(GdkScreen *screen);
}

int GtkSalDisplay::GetDefaultMonitorNumber() const
{
    GdkScreen* pScreen = gdk_display_get_screen( m_pGdkDisplay, m_nDefaultScreen );
#if GTK_CHECK_VERSION(2,20,0)
    return m_aXineramaScreenIndexMap[gdk_screen_get_primary_monitor(pScreen)];
#else
    static screen_get_primary_monitor sym_gdk_screen_get_primary_monitor =
        (screen_get_primary_monitor)osl_getAsciiFunctionSymbol( GetSalData()->m_pPlugin, "gdk_screen_get_primary_monitor" );
    if (sym_gdk_screen_get_primary_monitor)
        return m_aXineramaScreenIndexMap[sym_gdk_screen_get_primary_monitor(pScreen)];
#if GTK_CHECK_VERSION(2,14,0)
    //gdk_screen_get_primary_monitor unavailable, take the first laptop monitor
    //as the default
    gint nMonitors = gdk_screen_get_n_monitors(pScreen);
    for (gint i = 0; i < nMonitors; ++i)
    {
        if (g_ascii_strncasecmp (gdk_screen_get_monitor_plug_name(pScreen, i), "LVDS", 4) == 0)
            return m_aXineramaScreenIndexMap[i];
    }
#endif
    return 0;
#endif
}

void GtkSalDisplay::initScreen( int nScreen ) const
{
    if( nScreen < 0 || nScreen >= static_cast<int>(m_aScreens.size()) )
        nScreen = m_nDefaultScreen;
    ScreenData& rSD = const_cast<ScreenData&>(m_aScreens[nScreen]);
    if( rSD.m_bInit )
        return;

    // choose visual for screen
    SalDisplay::initScreen( nScreen );
    // now set a gdk default colormap matching the chosen visual to the screen
    GdkVisual* pVis = gdkx_visual_get( rSD.m_aVisual.visualid );
    GdkScreen* pScreen = gdk_display_get_screen( m_pGdkDisplay, nScreen );
    if( pVis )
    {
        GdkColormap* pDefCol = gdk_screen_get_default_colormap( pScreen );
        GdkVisual* pDefVis = gdk_colormap_get_visual( pDefCol );
        if( pDefVis != pVis )
        {
           pDefCol = gdk_x11_colormap_foreign_new( pVis, rSD.m_aColormap.GetXColormap() );
           gdk_screen_set_default_colormap( pScreen, pDefCol );
           #if OSL_DEBUG_LEVEL > 1
           fprintf( stderr, "set new gdk color map for screen %d\n", nScreen );
           #endif
        }
    }
    #if OSL_DEBUG_LEVEL > 1
    else
        fprintf( stderr, "not GdkVisual for visual id %d\n", (int)rSD.m_aVisual.visualid );
    #endif
}

long GtkSalDisplay::Dispatch( XEvent* pEvent )
{
    if( GetDisplay() == pEvent->xany.display )
    {
        // let's see if one of our frames wants to swallow these events
        // get the child frame
        for( std::list< SalFrame* >::const_iterator it = m_aFrames.begin();
             it != m_aFrames.end(); ++it )
        {
            if( (GdkNativeWindow)(*it)->GetSystemData()->aWindow == pEvent->xany.window )
                return static_cast<GtkSalFrame*>(*it)->Dispatch( pEvent );
        }
    }

    return GDK_FILTER_CONTINUE;
}

GdkCursor* GtkSalDisplay::getFromXPM( const unsigned char *pBitmap,
                                      const unsigned char *pMask,
                                      int nWidth, int nHeight,
                                      int nXHot, int nYHot )
{
    GdkScreen *pScreen = gdk_display_get_default_screen( m_pGdkDisplay );
    GdkDrawable *pDrawable = GDK_DRAWABLE( gdk_screen_get_root_window (pScreen) );
    GdkBitmap *pBitmapPix = gdk_bitmap_create_from_data
            ( pDrawable, reinterpret_cast<const char*>(pBitmap), nWidth, nHeight );
    GdkBitmap *pMaskPix = gdk_bitmap_create_from_data
            ( pDrawable, reinterpret_cast<const char*>(pMask), nWidth, nHeight );
    GdkColormap *pColormap = gdk_drawable_get_colormap( pDrawable );

    GdkColor aWhite = { 0, 0xffff, 0xffff, 0xffff };
    GdkColor aBlack = { 0, 0, 0, 0 };

    gdk_colormap_alloc_color( pColormap, &aBlack, FALSE, TRUE);
    gdk_colormap_alloc_color( pColormap, &aWhite, FALSE, TRUE);

    return gdk_cursor_new_from_pixmap
            ( pBitmapPix, pMaskPix,
              &aBlack, &aWhite, nXHot, nYHot);
}

#define MAKE_CURSOR( vcl_name, name ) \
    case vcl_name: \
        pCursor = getFromXPM( name##curs##_bits, name##mask##_bits, \
                              name##curs_width, name##curs_height, \
                              name##curs_x_hot, name##curs_y_hot ); \
        break
#define MAP_BUILTIN( vcl_name, gdk_name ) \
        case vcl_name: \
            pCursor = gdk_cursor_new_for_display( m_pGdkDisplay, gdk_name ); \
            break

GdkCursor *GtkSalDisplay::getCursor( PointerStyle ePointerStyle )
{
    if( ePointerStyle > POINTER_COUNT )
        return NULL;

    if ( !m_aCursors[ ePointerStyle ] )
    {
        GdkCursor *pCursor = NULL;

        switch( ePointerStyle )
        {
            MAP_BUILTIN( POINTER_ARROW, GDK_LEFT_PTR );
            MAP_BUILTIN( POINTER_TEXT, GDK_XTERM );
            MAP_BUILTIN( POINTER_HELP, GDK_QUESTION_ARROW );
            MAP_BUILTIN( POINTER_CROSS, GDK_CROSSHAIR );
            MAP_BUILTIN( POINTER_WAIT, GDK_WATCH );

            MAP_BUILTIN( POINTER_NSIZE, GDK_SB_V_DOUBLE_ARROW );
            MAP_BUILTIN( POINTER_SSIZE, GDK_SB_V_DOUBLE_ARROW );
            MAP_BUILTIN( POINTER_WSIZE, GDK_SB_H_DOUBLE_ARROW );
            MAP_BUILTIN( POINTER_ESIZE, GDK_SB_H_DOUBLE_ARROW );

            MAP_BUILTIN( POINTER_NWSIZE, GDK_TOP_LEFT_CORNER );
            MAP_BUILTIN( POINTER_NESIZE, GDK_TOP_RIGHT_CORNER );
            MAP_BUILTIN( POINTER_SWSIZE, GDK_BOTTOM_LEFT_CORNER );
            MAP_BUILTIN( POINTER_SESIZE, GDK_BOTTOM_RIGHT_CORNER );

            MAP_BUILTIN( POINTER_WINDOW_NSIZE, GDK_TOP_SIDE );
            MAP_BUILTIN( POINTER_WINDOW_SSIZE, GDK_BOTTOM_SIDE );
            MAP_BUILTIN( POINTER_WINDOW_WSIZE, GDK_LEFT_SIDE );
            MAP_BUILTIN( POINTER_WINDOW_ESIZE, GDK_RIGHT_SIDE );

            MAP_BUILTIN( POINTER_WINDOW_NWSIZE, GDK_TOP_LEFT_CORNER );
            MAP_BUILTIN( POINTER_WINDOW_NESIZE, GDK_TOP_RIGHT_CORNER );
            MAP_BUILTIN( POINTER_WINDOW_SWSIZE, GDK_BOTTOM_LEFT_CORNER );
            MAP_BUILTIN( POINTER_WINDOW_SESIZE, GDK_BOTTOM_RIGHT_CORNER );

            MAP_BUILTIN( POINTER_HSIZEBAR, GDK_SB_H_DOUBLE_ARROW );
            MAP_BUILTIN( POINTER_VSIZEBAR, GDK_SB_V_DOUBLE_ARROW );

            MAP_BUILTIN( POINTER_REFHAND, GDK_HAND2 );
            MAP_BUILTIN( POINTER_HAND, GDK_HAND2 );
            MAP_BUILTIN( POINTER_PEN, GDK_PENCIL );

            MAP_BUILTIN( POINTER_HSPLIT, GDK_SB_H_DOUBLE_ARROW );
            MAP_BUILTIN( POINTER_VSPLIT, GDK_SB_V_DOUBLE_ARROW );

            MAP_BUILTIN( POINTER_MOVE, GDK_FLEUR );

            MAKE_CURSOR( POINTER_NULL, null );
            MAKE_CURSOR( POINTER_MAGNIFY, magnify_ );
            MAKE_CURSOR( POINTER_FILL, fill_ );
            MAKE_CURSOR( POINTER_MOVEDATA, movedata_ );
            MAKE_CURSOR( POINTER_COPYDATA, copydata_ );
            MAKE_CURSOR( POINTER_MOVEFILE, movefile_ );
            MAKE_CURSOR( POINTER_COPYFILE, copyfile_ );
            MAKE_CURSOR( POINTER_MOVEFILES, movefiles_ );
            MAKE_CURSOR( POINTER_COPYFILES, copyfiles_ );
            MAKE_CURSOR( POINTER_NOTALLOWED, nodrop_ );
            MAKE_CURSOR( POINTER_ROTATE, rotate_ );
            MAKE_CURSOR( POINTER_HSHEAR, hshear_ );
            MAKE_CURSOR( POINTER_VSHEAR, vshear_ );
            MAKE_CURSOR( POINTER_DRAW_LINE, drawline_ );
            MAKE_CURSOR( POINTER_DRAW_RECT, drawrect_ );
            MAKE_CURSOR( POINTER_DRAW_POLYGON, drawpolygon_ );
            MAKE_CURSOR( POINTER_DRAW_BEZIER, drawbezier_ );
            MAKE_CURSOR( POINTER_DRAW_ARC, drawarc_ );
            MAKE_CURSOR( POINTER_DRAW_PIE, drawpie_ );
            MAKE_CURSOR( POINTER_DRAW_CIRCLECUT, drawcirclecut_ );
            MAKE_CURSOR( POINTER_DRAW_ELLIPSE, drawellipse_ );
            MAKE_CURSOR( POINTER_DRAW_CONNECT, drawconnect_ );
            MAKE_CURSOR( POINTER_DRAW_TEXT, drawtext_ );
            MAKE_CURSOR( POINTER_MIRROR, mirror_ );
            MAKE_CURSOR( POINTER_CROOK, crook_ );
            MAKE_CURSOR( POINTER_CROP, crop_ );
            MAKE_CURSOR( POINTER_MOVEPOINT, movepoint_ );
            MAKE_CURSOR( POINTER_MOVEBEZIERWEIGHT, movebezierweight_ );
            MAKE_CURSOR( POINTER_DRAW_FREEHAND, drawfreehand_ );
            MAKE_CURSOR( POINTER_DRAW_CAPTION, drawcaption_ );
            MAKE_CURSOR( POINTER_LINKDATA, linkdata_ );
            MAKE_CURSOR( POINTER_MOVEDATALINK, movedlnk_ );
            MAKE_CURSOR( POINTER_COPYDATALINK, copydlnk_ );
            MAKE_CURSOR( POINTER_LINKFILE, linkfile_ );
            MAKE_CURSOR( POINTER_MOVEFILELINK, moveflnk_ );
            MAKE_CURSOR( POINTER_COPYFILELINK, copyflnk_ );
            MAKE_CURSOR( POINTER_CHART, chart_ );
            MAKE_CURSOR( POINTER_DETECTIVE, detective_ );
            MAKE_CURSOR( POINTER_PIVOT_COL, pivotcol_ );
            MAKE_CURSOR( POINTER_PIVOT_ROW, pivotrow_ );
            MAKE_CURSOR( POINTER_PIVOT_FIELD, pivotfld_ );
            MAKE_CURSOR( POINTER_PIVOT_DELETE, pivotdel_ );
            MAKE_CURSOR( POINTER_CHAIN, chain_ );
            MAKE_CURSOR( POINTER_CHAIN_NOTALLOWED, chainnot_ );
            MAKE_CURSOR( POINTER_TIMEEVENT_MOVE, timemove_ );
            MAKE_CURSOR( POINTER_TIMEEVENT_SIZE, timesize_ );
            MAKE_CURSOR( POINTER_AUTOSCROLL_N, asn_ );
            MAKE_CURSOR( POINTER_AUTOSCROLL_S, ass_ );
            MAKE_CURSOR( POINTER_AUTOSCROLL_W, asw_ );
            MAKE_CURSOR( POINTER_AUTOSCROLL_E, ase_ );
            MAKE_CURSOR( POINTER_AUTOSCROLL_NW, asnw_ );
            MAKE_CURSOR( POINTER_AUTOSCROLL_NE, asne_ );
            MAKE_CURSOR( POINTER_AUTOSCROLL_SW, assw_ );
            MAKE_CURSOR( POINTER_AUTOSCROLL_SE, asse_ );
            MAKE_CURSOR( POINTER_AUTOSCROLL_NS, asns_ );
            MAKE_CURSOR( POINTER_AUTOSCROLL_WE, aswe_ );
            MAKE_CURSOR( POINTER_AUTOSCROLL_NSWE, asnswe_ );
            MAKE_CURSOR( POINTER_AIRBRUSH, airbrush_ );
            MAKE_CURSOR( POINTER_TEXT_VERTICAL, vertcurs_ );

            // --> FME 2004-07-30 #i32329# Enhanced table selection
            MAKE_CURSOR( POINTER_TAB_SELECT_S, tblsels_ );
            MAKE_CURSOR( POINTER_TAB_SELECT_E, tblsele_ );
            MAKE_CURSOR( POINTER_TAB_SELECT_SE, tblselse_ );
            MAKE_CURSOR( POINTER_TAB_SELECT_W, tblselw_ );
            MAKE_CURSOR( POINTER_TAB_SELECT_SW, tblselsw_ );
            // <--

            // --> FME 2004-08-16 #i20119# Paintbrush tool
            MAKE_CURSOR( POINTER_PAINTBRUSH, paintbrush_ );
            // <--

        default:
            fprintf( stderr, "pointer %d not implemented", ePointerStyle );
            break;
        }
        if( !pCursor )
            pCursor = gdk_cursor_new_for_display( m_pGdkDisplay, GDK_LEFT_PTR );

        m_aCursors[ ePointerStyle ] = pCursor;
    }

    return m_aCursors[ ePointerStyle ];
}

int GtkSalDisplay::CaptureMouse( SalFrame* pSFrame )
{
    GtkSalFrame* pFrame = static_cast<GtkSalFrame*>(pSFrame);

    if( !pFrame )
    {
        if( m_pCapture )
            static_cast<GtkSalFrame*>(m_pCapture)->grabPointer( FALSE );
        m_pCapture = NULL;
        return 0;
    }

    if( m_pCapture )
    {
        if( pFrame == m_pCapture )
            return 1;
        static_cast<GtkSalFrame*>(m_pCapture)->grabPointer( FALSE );
    }

    m_pCapture = pFrame;
    static_cast<GtkSalFrame*>(pFrame)->grabPointer( TRUE );
    return 1;
}

/***************************************************************************
 * class GtkXLib                                                           *
 ***************************************************************************/

class GtkXLib : public SalXLib
{
    GtkSalDisplay       *m_pGtkSalDisplay;
    std::list<GSource *> m_aSources;
    GSource             *m_pTimeout;
    GSource             *m_pUserEvent;
    oslMutex             m_aDispatchMutex;
    oslCondition         m_aDispatchCondition;
    XIOErrorHandler      m_aOrigGTKXIOErrorHandler;

public:
    static gboolean      timeoutFn(gpointer data);
    static gboolean      userEventFn(gpointer data);

    GtkXLib();
    virtual ~GtkXLib();

    virtual void    Init();
    virtual void    Yield( bool bWait, bool bHandleAllCurrentEvents );
    virtual void    Insert( int fd, void* data,
                            YieldFunc   pending,
                            YieldFunc   queued,
                            YieldFunc   handle );
    virtual void    Remove( int fd );

    virtual void    StartTimer( ULONG nMS );
    virtual void    StopTimer();
    virtual void    Wakeup();
    virtual void    PostUserEvent();
};

GtkXLib::GtkXLib()
{
#if OSL_DEBUG_LEVEL > 1
    fprintf( stderr, "GtkXLib::GtkXLib()\n" );
#endif
    m_pGtkSalDisplay = NULL;
    m_pTimeout = NULL;
    m_nTimeoutMS = 0;
    m_pUserEvent = NULL;
    m_aDispatchCondition = osl_createCondition();
    m_aDispatchMutex = osl_createMutex();
    m_aOrigGTKXIOErrorHandler = NULL;
}

GtkXLib::~GtkXLib()
{
#if OSL_DEBUG_LEVEL > 1
    fprintf( stderr, "GtkXLib::~GtkXLib()\n" );
#endif
    StopTimer();
     // sanity check: at this point nobody should be yielding, but wake them
     // up anyway before the condition they're waiting on gets destroyed.
    osl_setCondition( m_aDispatchCondition );
    osl_destroyCondition( m_aDispatchCondition );
    osl_destroyMutex( m_aDispatchMutex );

    PopXErrorLevel();
    XSetIOErrorHandler (m_aOrigGTKXIOErrorHandler);
}

void GtkXLib::Init()
{
    int i;
#if OSL_DEBUG_LEVEL > 1
    fprintf( stderr, "GtkXLib::Init()\n" );
#endif
    XrmInitialize();

    gtk_set_locale();

    /*
     * open connection to X11 Display
     * try in this order:
     *  o  -display command line parameter,
     *  o  $DISPLAY environment variable
     *  o  default display
     */

    GdkDisplay *pGdkDisp = NULL;

    // is there a -display command line parameter?
    rtl_TextEncoding aEnc = osl_getThreadTextEncoding();
    int nParams = osl_getCommandArgCount();
    rtl::OString aDisplay;
    rtl::OUString aParam, aBin;
    char** pCmdLineAry = new char*[ nParams+1 ];
    osl_getExecutableFile( &aParam.pData );
    osl_getSystemPathFromFileURL( aParam.pData, &aBin.pData );
    pCmdLineAry[0] = g_strdup( OUStringToOString( aBin, aEnc ).getStr() );
    for (i=0; i<nParams; i++)
    {
        osl_getCommandArg(i, &aParam.pData );
        OString aBParam( OUStringToOString( aParam, aEnc ) );

        if( aParam.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "-display" ) ) || aParam.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "--display" ) ) )
        {
            pCmdLineAry[i+1] = g_strdup( "--display" );
            osl_getCommandArg(i+1, &aParam.pData );
            aDisplay = rtl::OUStringToOString( aParam, aEnc );
        }
        else
            pCmdLineAry[i+1] = g_strdup( aBParam.getStr() );
    }
    // add executable
    nParams++;

    g_set_application_name(X11SalData::getFrameClassName());

    // Set consistant name of the root accessible
    rtl::OUString aAppName = Application::GetAppName();
    if( aAppName.getLength() > 0 )
    {
        rtl::OString aPrgName = rtl::OUStringToOString(aAppName, aEnc);
        g_set_prgname(aPrgName);
    }

    // init gtk/gdk
    gtk_init_check( &nParams, &pCmdLineAry );

    //gtk_init_check sets XError/XIOError handlers, we want our own one
    m_aOrigGTKXIOErrorHandler = XSetIOErrorHandler ( (XIOErrorHandler)X11SalData::XIOErrorHdl );
    PushXErrorLevel( !!getenv( "SAL_IGNOREXERRORS" ) );

    for (i = 0; i < nParams; i++ )
        g_free( pCmdLineAry[i] );
    delete [] pCmdLineAry;

#if OSL_DEBUG_LEVEL > 1
    if (g_getenv ("SAL_DEBUG_UPDATES"))
        gdk_window_set_debug_updates (TRUE);
#endif

    pGdkDisp = gdk_display_get_default();
    if ( !pGdkDisp )
    {
        rtl::OUString aProgramFileURL;
        osl_getExecutableFile( &aProgramFileURL.pData );
        rtl::OUString aProgramSystemPath;
        osl_getSystemPathFromFileURL (aProgramFileURL.pData, &aProgramSystemPath.pData);
        rtl::OString  aProgramName = rtl::OUStringToOString(
                                            aProgramSystemPath,
                                            osl_getThreadTextEncoding() );
        fprintf( stderr, "%s X11 error: Can't open display: %s\n",
                aProgramName.getStr(), aDisplay.getStr());
        fprintf( stderr, "   Set DISPLAY environment variable, use -display option\n");
        fprintf( stderr, "   or check permissions of your X-Server\n");
        fprintf( stderr, "   (See \"man X\" resp. \"man xhost\" for details)\n");
        fflush( stderr );
        exit(0);
    }

    /*
     * if a -display switch was used, we need
     * to set the environment accoringly since
     * the clipboard build another connection
     * to the xserver using $DISPLAY
     */
    rtl::OUString envVar(RTL_CONSTASCII_USTRINGPARAM("DISPLAY"));
    const gchar *name = gdk_display_get_name( pGdkDisp );
    rtl::OUString envValue(name, strlen(name), aEnc);
    osl_setEnvironment(envVar.pData, envValue.pData);

    Display *pDisp = gdk_x11_display_get_xdisplay( pGdkDisp );

    m_pGtkSalDisplay = new GtkSalDisplay( pGdkDisp );

    gdk_window_add_filter( NULL, call_filterGdkEvent, m_pGtkSalDisplay );

    PushXErrorLevel( true );
    SalI18N_KeyboardExtension *pKbdExtension = new SalI18N_KeyboardExtension( pDisp );
    XSync( pDisp, False );

    pKbdExtension->UseExtension( ! HasXErrorOccurred() );
    PopXErrorLevel();

    m_pGtkSalDisplay->SetKbdExtension( pKbdExtension );

    g_signal_connect( G_OBJECT(gdk_keymap_get_default()), "keys_changed", G_CALLBACK(signalKeysChanged), m_pGtkSalDisplay );

    // add signal handler to notify screen size changes
    int nScreens = gdk_display_get_n_screens( pGdkDisp );
    for( int n = 0; n < nScreens; n++ )
    {
        GdkScreen *pScreen = gdk_display_get_screen( pGdkDisp, n );
        if( pScreen )
        {
            g_signal_connect( G_OBJECT(pScreen), "size-changed", G_CALLBACK(signalScreenSizeChanged), m_pGtkSalDisplay );
            if( ! gtk_check_version( 2, 14, 0 ) ) // monitors-changed came in with 2.14, avoid an assertion
                g_signal_connect( G_OBJECT(pScreen), "monitors-changed", G_CALLBACK(signalMonitorsChanged), m_pGtkSalDisplay );
        }
    }
}

extern "C"
{
    gboolean call_timeoutFn(gpointer data)
    {
        return GtkXLib::timeoutFn(data);
    }
}

gboolean GtkXLib::timeoutFn(gpointer data)
{
    SalData *pSalData = GetSalData();
    GtkXLib *pThis = (GtkXLib *) data;

    pSalData->m_pInstance->GetYieldMutex()->acquire();

    if( pThis->m_pTimeout )
    {
        g_source_unref (pThis->m_pTimeout);
        pThis->m_pTimeout = NULL;
    }

    // Auto-restart immediately
    pThis->StartTimer( pThis->m_nTimeoutMS );

    GetX11SalData()->Timeout();

    pSalData->m_pInstance->GetYieldMutex()->release();

    return FALSE;
}

void GtkXLib::StartTimer( ULONG nMS )
{
    m_nTimeoutMS = nMS; // for restarting

    if (m_pTimeout)
    {
        g_source_destroy (m_pTimeout);
        g_source_unref (m_pTimeout);
    }

    m_pTimeout = g_timeout_source_new (m_nTimeoutMS);
    // #i36226# timers should be executed with lower priority
    // than XEvents like in generic plugin
    g_source_set_priority( m_pTimeout, G_PRIORITY_LOW );
    g_source_set_can_recurse (m_pTimeout, TRUE);
    g_source_set_callback (m_pTimeout, call_timeoutFn,
                           (gpointer) this, NULL);
    g_source_attach (m_pTimeout, g_main_context_default ());

    SalXLib::StartTimer( nMS );
}

void GtkXLib::StopTimer()
{
    SalXLib::StopTimer();

    if (m_pTimeout)
    {
        g_source_destroy (m_pTimeout);
        g_source_unref (m_pTimeout);
        m_pTimeout = NULL;
    }
}

extern "C"
{
    gboolean call_userEventFn( gpointer data )
    {
        return GtkXLib::userEventFn( data );
    }
}

gboolean GtkXLib::userEventFn(gpointer data)
{
    gboolean bContinue;
    GtkXLib *pThis = (GtkXLib *) data;
    SalData *pSalData = GetSalData();

    pSalData->m_pInstance->GetYieldMutex()->acquire();
    pThis->m_pGtkSalDisplay->EventGuardAcquire();

    if( !pThis->m_pGtkSalDisplay->HasMoreEvents() )
    {
        if( pThis->m_pUserEvent )
        {
            g_source_unref (pThis->m_pUserEvent);
            pThis->m_pUserEvent = NULL;
        }
        bContinue = FALSE;
    }
    else
        bContinue = TRUE;

    pThis->m_pGtkSalDisplay->EventGuardRelease();

    pThis->m_pGtkSalDisplay->DispatchInternalEvent();

    pSalData->m_pInstance->GetYieldMutex()->release();

    return bContinue;
}

// hEventGuard_ held during this invocation
void GtkXLib::PostUserEvent()
{
    if( !m_pUserEvent ) // not pending anyway
    {
        m_pUserEvent = g_idle_source_new();
        g_source_set_priority( m_pUserEvent, G_PRIORITY_HIGH );
        g_source_set_can_recurse (m_pUserEvent, TRUE);
        g_source_set_callback (m_pUserEvent, call_userEventFn,
                               (gpointer) this, NULL);
        g_source_attach (m_pUserEvent, g_main_context_default ());
    }
    Wakeup();
}

void GtkXLib::Wakeup()
{
    g_main_context_wakeup( g_main_context_default () );
}

void GtkXLib::Yield( bool bWait, bool bHandleAllCurrentEvents )
{
    /* #i33212# only enter g_main_context_iteration in one thread at any one
     * time, else one of them potentially will never end as long as there is
     * another thread in in there. Having only one yieldin thread actually dispatch
     * fits the vcl event model (see e.g. the generic plugin).
     */

    bool bDispatchThread = false;
    gboolean wasEvent = FALSE;
    {
        // release YieldMutex (and re-acquire at block end)
        YieldMutexReleaser aReleaser;
        if( osl_tryToAcquireMutex( m_aDispatchMutex ) )
            bDispatchThread = true;
        else if( ! bWait )
            return; // someone else is waiting already, return


        if( bDispatchThread )
        {
            int nMaxEvents = bHandleAllCurrentEvents ? 100 : 1;
            gboolean wasOneEvent = TRUE;
            while( nMaxEvents-- && wasOneEvent )
            {
                wasOneEvent = g_main_context_iteration( NULL, FALSE );
                if( wasOneEvent )
                    wasEvent = TRUE;
            }
            if( bWait && ! wasEvent )
                wasEvent = g_main_context_iteration( NULL, TRUE );
        }
        else if( bWait )
           {
            /* #i41693# in case the dispatch thread hangs in join
             * for this thread the condition will never be set
             * workaround: timeout of 1 second a emergency exit
             */
            // we are the dispatch thread
            osl_resetCondition( m_aDispatchCondition );
            TimeValue aValue = { 1, 0 };
            osl_waitCondition( m_aDispatchCondition, &aValue );
        }
    }

    if( bDispatchThread )
    {
        osl_releaseMutex( m_aDispatchMutex );
        if( wasEvent )
            osl_setCondition( m_aDispatchCondition ); // trigger non dispatch thread yields
    }
}

extern "C" {

typedef struct {
    GSource       source;

    GPollFD       pollfd;
    GIOCondition  condition;

    YieldFunc     pending;
    YieldFunc     handle;
    gpointer      user_data;
} SalWatch;

static gboolean
sal_source_prepare (GSource *source,
                    gint    *timeout)
{
    SalWatch *watch = (SalWatch *)source;

    *timeout = -1;

    if (watch->pending &&
        watch->pending (watch->pollfd.fd, watch->user_data)) {
        watch->pollfd.revents |= watch->condition;
        return TRUE;
    }

    return FALSE;
}

static gboolean
sal_source_check (GSource *source)
{
    SalWatch *watch = (SalWatch *)source;

    return watch->pollfd.revents & watch->condition;
}

static gboolean
sal_source_dispatch (GSource    *source,
                     GSourceFunc,
                     gpointer)
{
    SalData *pSalData = GetSalData();
    SalWatch *watch = (SalWatch *) source;

    pSalData->m_pInstance->GetYieldMutex()->acquire();

    watch->handle (watch->pollfd.fd, watch->user_data);

    pSalData->m_pInstance->GetYieldMutex()->release();

    return TRUE;
}

static void
sal_source_finalize (GSource*)
{
}

static GSourceFuncs sal_source_watch_funcs = {
    sal_source_prepare,
    sal_source_check,
    sal_source_dispatch,
    sal_source_finalize,
    NULL,
    NULL
};

static GSource *
sal_source_create_watch (int           fd,
                         GIOCondition  condition,
                         YieldFunc     pending,
                         YieldFunc     handle,
                         gpointer      user_data)
{
    GSource      *source;
    SalWatch     *watch;
    GMainContext *context = g_main_context_default ();

    source = g_source_new (&sal_source_watch_funcs,
                   sizeof (SalWatch));
    watch = (SalWatch *) source;

    watch->pollfd.fd     = fd;
    watch->pollfd.events = condition;
    watch->condition = condition;
    watch->pending   = pending;
    watch->handle    = handle;
    watch->user_data = user_data;

    g_source_set_can_recurse (source, TRUE);
    g_source_add_poll (source, &watch->pollfd);
    g_source_attach (source, context);

    return source;
}

} // extern "C"

void GtkXLib::Insert( int       nFD,
              void     *data,
              YieldFunc pending,
              YieldFunc,
              YieldFunc handle )
{
    GSource *source = sal_source_create_watch
        ( nFD, (GIOCondition) ((G_IO_IN|G_IO_PRI) |
                       (G_IO_ERR|G_IO_HUP|G_IO_NVAL)),
          pending, handle, data );
    m_aSources.push_back( source );
}

void GtkXLib::Remove( int nFD )
{
    ::std::list< GSource * >::iterator it;

    for (it = m_aSources.begin(); it != m_aSources.end(); ++it)
    {
        SalWatch *watch = (SalWatch *) *it;

        if (watch->pollfd.fd == nFD)
        {
            m_aSources.erase( it );

            g_source_destroy ((GSource *)watch);
            g_source_unref   ((GSource *)watch);
            return;
        }
    }
}

/**********************************************************************
 * class GtkData                                                      *
 **********************************************************************/

GtkData::~GtkData()
{
}

void GtkData::Init()
{
    pXLib_ = new GtkXLib();
    pXLib_->Init();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
