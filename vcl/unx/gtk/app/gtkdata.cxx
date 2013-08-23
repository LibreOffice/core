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
#endif
#include <unx/gtk/gtkdata.hxx>
#include <unx/gtk/gtkinst.hxx>
#include <unx/gtk/gtkframe.hxx>
#include <unx/salobj.h>
#include <generic/geninst.h>
#include <osl/thread.h>
#include <osl/process.h>

#include "unx/i18n_im.hxx"
#include "unx/i18n_xkb.hxx"
#include <unx/wmadaptor.hxx>

#include "unx/x11_cursors/salcursors.h"

#include <vcl/svapp.hxx>

using namespace vcl_sal;


/***************************************************************
 * class GtkSalDisplay                                         *
 ***************************************************************/
extern "C" {
GdkFilterReturn call_filterGdkEvent( GdkXEvent* sys_event,
                                     GdkEvent* event,
                                     gpointer data )
{
    GtkSalDisplay *pDisplay = (GtkSalDisplay *)data;
    return pDisplay->filterGdkEvent( sys_event, event );
}
}

GtkSalDisplay::GtkSalDisplay( GdkDisplay* pDisplay ) :
#if !GTK_CHECK_VERSION(3,0,0)
            SalDisplay( gdk_x11_display_get_xdisplay( pDisplay ) ),
#endif
            m_pSys( GtkSalSystem::GetSingleton() ),
            m_pGdkDisplay( pDisplay ),
            m_bStartupCompleted( false )
{
    for(int i = 0; i < POINTER_COUNT; i++)
        m_aCursors[ i ] = NULL;
#if !GTK_CHECK_VERSION(3,0,0)
    m_bUseRandRWrapper = false; // use gdk signal instead
    Init ();
#endif

    // FIXME: unify this with SalInst's filter too ?
    gdk_window_add_filter( NULL, call_filterGdkEvent, this );

    if ( getenv( "SAL_IGNOREXERRORS" ) )
        GetGenericData()->ErrorTrapPush(); // and leak the trap

#if GTK_CHECK_VERSION(3,0,0)
    m_bX11Display = GDK_IS_X11_DISPLAY( m_pGdkDisplay );
#else
    m_bX11Display = true;
#endif
}

GtkSalDisplay::~GtkSalDisplay()
{
    gdk_window_remove_filter( NULL, call_filterGdkEvent, this );

    if( !m_bStartupCompleted )
        gdk_notify_startup_complete();

#if !GTK_CHECK_VERSION(3,0,0)
    doDestruct();
    pDisp_ = NULL;
#endif

    for(int i = 0; i < POINTER_COUNT; i++)
        if( m_aCursors[ i ] )
            gdk_cursor_unref( m_aCursors[ i ] );
}

extern "C" {

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
                                               GdkEvent* )
{
#if !GTK_CHECK_VERSION(3,0,0)
    GdkFilterReturn aFilterReturn = GDK_FILTER_CONTINUE;
    XEvent *pEvent = (XEvent *)sys_event;

    // dispatch all XEvents to event callback
    if( GetSalData()->m_pInstance->
        CallEventCallback( pEvent, sizeof( XEvent ) ) )
        aFilterReturn = GDK_FILTER_REMOVE;

    if (GetDisplay() == pEvent->xany.display )
    {
        // #i53471# gtk has no callback mechanism that lets us be notified
        // when settings (as in XSETTING and opposed to styles) are changed.
        // so we need to listen for corresponding property notifications here
        // these should be rare enough so that we can assume that the settings
        // actually change when a corresponding PropertyNotify occurs
        if( pEvent->type == PropertyNotify &&
            pEvent->xproperty.atom == getWMAdaptor()->getAtom( WMAdaptor::XSETTINGS ) &&
            ! m_aFrames.empty()
           )
        {
            SendInternalEvent( m_aFrames.front(), NULL, SALEVENT_SETTINGSCHANGED );
        }
        // let's see if one of our frames wants to swallow these events
        // get the frame
        for( std::list< SalFrame* >::const_iterator it = m_aFrames.begin();
                 it != m_aFrames.end(); ++it )
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
#else
    (void) sys_event;
#warning FIXME: implement filterGdkEvent ...
    return GDK_FILTER_CONTINUE;
#endif
}

void GtkSalDisplay::screenSizeChanged( GdkScreen* pScreen )
{
    m_pSys->countScreenMonitors();
    if (pScreen)
        emitDisplayChanged();
}

void GtkSalDisplay::monitorsChanged( GdkScreen* pScreen )
{
    m_pSys->countScreenMonitors();
    if (pScreen)
        emitDisplayChanged();
}

#if !GTK_CHECK_VERSION(3,0,0)
SalDisplay::ScreenData *
GtkSalDisplay::initScreen( SalX11Screen nXScreen ) const
{
    // choose visual for screen
    ScreenData *pSD;
    if (!(pSD = SalDisplay::initScreen( nXScreen )))
        return NULL;

    // now set a gdk default colormap matching the chosen visual to the screen
    GdkScreen* pScreen = gdk_display_get_screen( m_pGdkDisplay, nXScreen.getXScreen() );
//  should really use this:
//  GdkVisual* pVis = gdk_x11_screen_lookup_visual_get( screen, pSD->m_aVisual.visualid );
//  and not this:
    GdkVisual* pVis = gdkx_visual_get( pSD->m_aVisual.visualid );
    if( pVis )
    {
        GdkColormap* pDefCol = gdk_screen_get_default_colormap( pScreen );
        GdkVisual* pDefVis = gdk_colormap_get_visual( pDefCol );
        if( pDefVis != pVis )
        {
           pDefCol = gdk_x11_colormap_foreign_new( pVis, pSD->m_aColormap.GetXColormap() );
           gdk_screen_set_default_colormap( pScreen, pDefCol );
           SAL_INFO( "vcl.gtk", "set new gdk color map for screen " << nXScreen.getXScreen() );
        }
    }
    else
        SAL_INFO( "vcl.gtk", "not GdkVisual for visual id " << pSD->m_aVisual.visualid );

    return pSD;
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
#endif

#if GTK_CHECK_VERSION(3,0,0)
namespace
{
    //cairo annoyingly won't take raw xbm data unless it fits
    //the required cairo stride
    unsigned char* ensurePaddedForCairo(const unsigned char *pXBM,
        int nWidth, int nHeight, int nStride)
    {
        unsigned char *pPaddedXBM = const_cast<unsigned char*>(pXBM);

        int bytes_per_row = (nWidth + 7) / 8;

        if (nStride != bytes_per_row)
        {
            pPaddedXBM = new unsigned char[nStride * nHeight];
            for (int row = 0; row < nHeight; ++row)
            {
                memcpy(pPaddedXBM + (nStride * row),
                    pXBM + (bytes_per_row * row), bytes_per_row);
                memset(pPaddedXBM + (nStride * row) + bytes_per_row,
                    0, nStride - bytes_per_row);
            }
        }

        return pPaddedXBM;
    }
}
#endif

GdkCursor* GtkSalDisplay::getFromXBM( const unsigned char *pBitmap,
                                      const unsigned char *pMask,
                                      int nWidth, int nHeight,
                                      int nXHot, int nYHot )
{
#if GTK_CHECK_VERSION(3,0,0)
    int cairo_stride = cairo_format_stride_for_width(CAIRO_FORMAT_A1, nWidth);

    unsigned char *pPaddedXBM = ensurePaddedForCairo(pBitmap, nWidth, nHeight, cairo_stride);
    cairo_surface_t *s = cairo_image_surface_create_for_data(
        pPaddedXBM,
        CAIRO_FORMAT_A1, nWidth, nHeight,
        cairo_stride);

    cairo_t *cr = cairo_create(s);
    unsigned char *pPaddedMaskXBM = ensurePaddedForCairo(pMask, nWidth, nHeight, cairo_stride);
    cairo_surface_t *mask = cairo_image_surface_create_for_data(
        pPaddedMaskXBM,
        CAIRO_FORMAT_A1, nWidth, nHeight,
        cairo_stride);
    cairo_mask_surface(cr, mask, 0, 0);
    cairo_destroy(cr);
    cairo_surface_destroy(mask);
    if (pPaddedMaskXBM != pMask)
        delete [] pPaddedMaskXBM;

    GdkPixbuf *pixbuf = gdk_pixbuf_get_from_surface(s, 0, 0, nWidth, nHeight);
    cairo_surface_destroy(s);
    if (pPaddedXBM != pBitmap)
        delete [] pPaddedXBM;

    GdkCursor *cursor = gdk_cursor_new_from_pixbuf(m_pGdkDisplay, pixbuf, nXHot, nYHot);
    g_object_unref(pixbuf);

    return cursor;
#else
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
#endif
}

#define MAKE_CURSOR( vcl_name, name ) \
    case vcl_name: \
        pCursor = getFromXBM( name##curs##_bits, name##mask##_bits, \
                              name##curs_width, name##curs_height, \
                              name##curs_x_hot, name##curs_y_hot ); \
        break
#define MAP_BUILTIN( vcl_name, gdk_name ) \
        case vcl_name: \
            pCursor = gdk_cursor_new_for_display( m_pGdkDisplay, gdk_name ); \
            break

GdkCursor *GtkSalDisplay::getCursor( PointerStyle ePointerStyle )
{
    if( ePointerStyle >= POINTER_COUNT )
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

            // #i32329#
            MAKE_CURSOR( POINTER_TAB_SELECT_S, tblsels_ );
            MAKE_CURSOR( POINTER_TAB_SELECT_E, tblsele_ );
            MAKE_CURSOR( POINTER_TAB_SELECT_SE, tblselse_ );
            MAKE_CURSOR( POINTER_TAB_SELECT_W, tblselw_ );
            MAKE_CURSOR( POINTER_TAB_SELECT_SW, tblselsw_ );

            // #i20119#
            MAKE_CURSOR( POINTER_PAINTBRUSH, paintbrush_ );

        default:
            SAL_WARN( "vcl.gtk", "pointer " << ePointerStyle << "not implemented" );
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


/**********************************************************************
 * class GtkData                                                      *
 **********************************************************************/

GtkData::GtkData( SalInstance *pInstance )
#if GTK_CHECK_VERSION(3,0,0)
    : SalGenericData( SAL_DATA_GTK3, pInstance )
#else
    : SalGenericData( SAL_DATA_GTK, pInstance )
#endif
{
    m_pUserEvent = NULL;
    m_aDispatchMutex = osl_createMutex();
    m_aDispatchCondition = osl_createCondition();
}

XIOErrorHandler aOrigXIOErrorHandler = NULL;

int XIOErrorHdl(Display *)
{
    fprintf(stderr, "X IO Error\n");
    _exit(1);
        // avoid crashes in unrelated threads that still run while atexit
        // handlers are in progress
}

GtkData::~GtkData()
{
    Yield( true, true );
    g_warning ("TESTME: We used to have a stop-timer here, but the central code should do this");

     // sanity check: at this point nobody should be yielding, but wake them
     // up anyway before the condition they're waiting on gets destroyed.
    osl_setCondition( m_aDispatchCondition );

    osl_acquireMutex( m_aDispatchMutex );
    if (m_pUserEvent)
    {
        g_source_destroy (m_pUserEvent);
        g_source_unref (m_pUserEvent);
        m_pUserEvent = NULL;
    }
    osl_destroyCondition( m_aDispatchCondition );
    osl_releaseMutex( m_aDispatchMutex );
    osl_destroyMutex( m_aDispatchMutex );
    XSetIOErrorHandler(aOrigXIOErrorHandler);
}

void GtkData::Dispose()
{
    deInitNWF();
}

void GtkData::Yield( bool bWait, bool bHandleAllCurrentEvents )
{
    /* #i33212# only enter g_main_context_iteration in one thread at any one
     * time, else one of them potentially will never end as long as there is
     * another thread in in there. Having only one yieldin thread actually dispatch
     * fits the vcl event model (see e.g. the generic plugin).
     */
    bool bDispatchThread = false;
    bool bWasEvent = false;
    {
        // release YieldMutex (and re-acquire at block end)
        SalYieldMutexReleaser aReleaser;
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
                wasOneEvent = g_main_context_iteration( NULL, bWait & !bWasEvent );
                if( wasOneEvent )
                    bWasEvent = true;
            }
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
        if( bWasEvent )
            osl_setCondition( m_aDispatchCondition ); // trigger non dispatch thread yields
    }
}

void GtkData::Init()
{
    int i;
    SAL_INFO( "vcl.gtk", "GtkMainloop::Init()" );
    XrmInitialize();

#if !GTK_CHECK_VERSION(3,0,0)
    gtk_set_locale();
#endif

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
    OString aDisplay;
    OUString aParam, aBin;
    char** pCmdLineAry = new char*[ nParams+1 ];
    osl_getExecutableFile( &aParam.pData );
    osl_getSystemPathFromFileURL( aParam.pData, &aBin.pData );
    pCmdLineAry[0] = g_strdup( OUStringToOString( aBin, aEnc ).getStr() );
    for (i=0; i<nParams; i++)
    {
        osl_getCommandArg(i, &aParam.pData );
        OString aBParam( OUStringToOString( aParam, aEnc ) );

        if( aParam == "-display" || aParam == "--display" )
        {
            pCmdLineAry[i+1] = g_strdup( "--display" );
            osl_getCommandArg(i+1, &aParam.pData );
            aDisplay = OUStringToOString( aParam, aEnc );
        }
        else
            pCmdLineAry[i+1] = g_strdup( aBParam.getStr() );
    }
    // add executable
    nParams++;

    g_set_application_name(SalGenericSystem::getFrameClassName());

    // Set consistant name of the root accessible
    OUString aAppName = Application::GetAppName();
    if( !aAppName.isEmpty() )
    {
        OString aPrgName = OUStringToOString(aAppName, aEnc);
        g_set_prgname(aPrgName.getStr());
    }

    // init gtk/gdk
    gtk_init_check( &nParams, &pCmdLineAry );
    gdk_error_trap_push();
    aOrigXIOErrorHandler = XSetIOErrorHandler(XIOErrorHdl);

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
        OUString aProgramFileURL;
        osl_getExecutableFile( &aProgramFileURL.pData );
        OUString aProgramSystemPath;
        osl_getSystemPathFromFileURL (aProgramFileURL.pData, &aProgramSystemPath.pData);
        OString  aProgramName = OUStringToOString(
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
    OUString envVar("DISPLAY");
    const gchar *name = gdk_display_get_name( pGdkDisp );
    OUString envValue(name, strlen(name), aEnc);
    osl_setEnvironment(envVar.pData, envValue.pData);

    GtkSalDisplay *pDisplay = new GtkSalDisplay( pGdkDisp );
    SetDisplay( pDisplay );

#if !GTK_CHECK_VERSION(3,0,0)
    Display *pDisp = gdk_x11_display_get_xdisplay( pGdkDisp );

    gdk_error_trap_push();
    SalI18N_KeyboardExtension *pKbdExtension = new SalI18N_KeyboardExtension( pDisp );
    bool bErrorOccured = gdk_error_trap_pop() != 0;
    gdk_error_trap_push();
    pKbdExtension->UseExtension( bErrorOccured );
    gdk_error_trap_pop();
    GetGtkDisplay()->SetKbdExtension( pKbdExtension );
#else
#  warning unwind keyboard extension bits
#endif

    // add signal handler to notify screen size changes
    int nScreens = gdk_display_get_n_screens( pGdkDisp );
    for( int n = 0; n < nScreens; n++ )
    {
        GdkScreen *pScreen = gdk_display_get_screen( pGdkDisp, n );
        if( pScreen )
        {
            pDisplay->screenSizeChanged( pScreen );
            pDisplay->monitorsChanged( pScreen );
            g_signal_connect( G_OBJECT(pScreen), "size-changed",
                              G_CALLBACK(signalScreenSizeChanged), pDisplay );
            if( ! gtk_check_version( 2, 14, 0 ) ) // monitors-changed came in with 2.14, avoid an assertion
                g_signal_connect( G_OBJECT(pScreen), "monitors-changed",
                                  G_CALLBACK(signalMonitorsChanged), GetGtkDisplay() );
        }
    }
}

void GtkData::ErrorTrapPush()
{
    gdk_error_trap_push ();
}

bool GtkData::ErrorTrapPop( bool bIgnoreError )
{
#if GTK_CHECK_VERSION(3,0,0)
    if( bIgnoreError )
    {
        gdk_error_trap_pop_ignored (); // faster
        return false;
    }
#else
    (void) bIgnoreError;
#endif
    return gdk_error_trap_pop () != 0;
}

extern "C" {

    struct SalGtkTimeoutSource {
        GSource      aParent;
        GTimeVal     aFireTime;
        GtkSalTimer *pInstance;
    };

    static void sal_gtk_timeout_defer( SalGtkTimeoutSource *pTSource )
    {
        g_get_current_time( &pTSource->aFireTime );
        g_time_val_add( &pTSource->aFireTime, pTSource->pInstance->m_nTimeoutMS * 1000 );
    }

    static gboolean sal_gtk_timeout_expired( SalGtkTimeoutSource *pTSource,
                                             gint *nTimeoutMS, GTimeVal *pTimeNow )
    {
        glong nDeltaSec = pTSource->aFireTime.tv_sec - pTimeNow->tv_sec;
        glong nDeltaUSec = pTSource->aFireTime.tv_usec - pTimeNow->tv_usec;
        if( nDeltaSec < 0 || ( nDeltaSec == 0 && nDeltaUSec < 0) )
        {
            *nTimeoutMS = 0;
            return TRUE;
        }
        if( nDeltaUSec < 0 )
        {
            nDeltaUSec += 1000000;
            nDeltaSec -= 1;
        }
        // if the clock changes backwards we need to cope ...
        if( (unsigned long) nDeltaSec > 1 + ( pTSource->pInstance->m_nTimeoutMS / 1000 ) )
        {
            sal_gtk_timeout_defer( pTSource );
            return TRUE;
        }

        *nTimeoutMS = MIN( G_MAXINT, ( nDeltaSec * 1000 + (nDeltaUSec + 999) / 1000 ) );

        return *nTimeoutMS == 0;
    }

    static gboolean sal_gtk_timeout_prepare( GSource *pSource, gint *nTimeoutMS )
    {
        SalGtkTimeoutSource *pTSource = (SalGtkTimeoutSource *)pSource;

        GTimeVal aTimeNow;
        g_get_current_time( &aTimeNow );

        return sal_gtk_timeout_expired( pTSource, nTimeoutMS, &aTimeNow );
    }

    static gboolean sal_gtk_timeout_check( GSource *pSource )
    {
        SalGtkTimeoutSource *pTSource = (SalGtkTimeoutSource *)pSource;

        GTimeVal aTimeNow;
        g_get_current_time( &aTimeNow );

        return ( pTSource->aFireTime.tv_sec < aTimeNow.tv_sec ||
                 ( pTSource->aFireTime.tv_sec == aTimeNow.tv_sec &&
                   pTSource->aFireTime.tv_usec < aTimeNow.tv_usec ) );
    }

    static gboolean sal_gtk_timeout_dispatch( GSource *pSource, GSourceFunc, gpointer )
    {
        SalGtkTimeoutSource *pTSource = (SalGtkTimeoutSource *)pSource;

        if( !pTSource->pInstance )
            return FALSE;

        SalData *pSalData = GetSalData();

        osl::Guard< comphelper::SolarMutex > aGuard( pSalData->m_pInstance->GetYieldMutex() );

        sal_gtk_timeout_defer( pTSource );

        ImplSVData* pSVData = ImplGetSVData();
        if( pSVData->mpSalTimer )
            pSVData->mpSalTimer->CallCallback();

        return TRUE;
    }

    static GSourceFuncs sal_gtk_timeout_funcs =
    {
        sal_gtk_timeout_prepare,
        sal_gtk_timeout_check,
        sal_gtk_timeout_dispatch,
        NULL, NULL, NULL
    };
}

static SalGtkTimeoutSource *
create_sal_gtk_timeout( GtkSalTimer *pTimer )
{
  GSource *pSource = g_source_new( &sal_gtk_timeout_funcs, sizeof( SalGtkTimeoutSource ) );
  SalGtkTimeoutSource *pTSource = (SalGtkTimeoutSource *)pSource;
  pTSource->pInstance = pTimer;

  // #i36226# timers should be executed with lower priority
  // than XEvents like in generic plugin
  g_source_set_priority( pSource, G_PRIORITY_LOW );
  g_source_set_can_recurse( pSource, TRUE );
  g_source_set_callback( pSource,
                         /* unused dummy */ g_idle_remove_by_data,
                         NULL, NULL );
  g_source_attach( pSource, g_main_context_default() );

  sal_gtk_timeout_defer( pTSource );

  return pTSource;
}

GtkSalTimer::GtkSalTimer()
    : m_pTimeout( NULL )
{
}

GtkSalTimer::~GtkSalTimer()
{
    GtkInstance *pInstance = static_cast<GtkInstance *>(GetSalData()->m_pInstance);
    pInstance->RemoveTimer( this );
    Stop();
}

bool GtkSalTimer::Expired()
{
    if( !m_pTimeout )
        return false;

    gint nDummy = 0;
    GTimeVal aTimeNow;
    g_get_current_time( &aTimeNow );
    return !!sal_gtk_timeout_expired( m_pTimeout, &nDummy, &aTimeNow);
}

void GtkSalTimer::Start( sal_uLong nMS )
{
    m_nTimeoutMS = nMS; // for restarting
    Stop(); // FIXME: ideally re-use an existing m_pTimeout
    m_pTimeout = create_sal_gtk_timeout( this );
}

void GtkSalTimer::Stop()
{
    if( m_pTimeout )
    {
        g_source_destroy( (GSource *)m_pTimeout );
        g_source_unref( (GSource *)m_pTimeout );
        m_pTimeout = NULL;
    }
}

gboolean GtkData::userEventFn( gpointer data )
{
    gboolean bContinue = FALSE;
    GtkData *pThis = (GtkData *) data;
    SalGenericData *pData = GetGenericData();
    osl::Guard< comphelper::SolarMutex > aGuard( pData->m_pInstance->GetYieldMutex() );
    const SalGenericDisplay *pDisplay = pData->GetDisplay();
    if (pDisplay)
    {
        OSL_ASSERT(static_cast<const SalGenericDisplay *>(pThis->GetGtkDisplay()) == pDisplay);
        pThis->GetGtkDisplay()->EventGuardAcquire();

        if( !pThis->GetGtkDisplay()->HasUserEvents() )
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

        pThis->GetGtkDisplay()->EventGuardRelease();

        pThis->GetGtkDisplay()->DispatchInternalEvent();
    }

    return bContinue;
}

extern "C" {
    static gboolean call_userEventFn( void *data )
    {
        SolarMutexGuard aGuard;
        return GtkData::userEventFn( data );
    }
}

// hEventGuard_ held during this invocation
void GtkData::PostUserEvent()
{
    if (m_pUserEvent)
        g_main_context_wakeup (NULL); // really needed ?
    else // nothing pending anyway
    {
        m_pUserEvent = g_idle_source_new();
        g_source_set_priority (m_pUserEvent, G_PRIORITY_HIGH);
        g_source_set_can_recurse (m_pUserEvent, TRUE);
        g_source_set_callback (m_pUserEvent, call_userEventFn,
                               (gpointer) this, NULL);
        g_source_attach (m_pUserEvent, g_main_context_default ());
    }
}

void GtkSalDisplay::PostUserEvent()
{
    GetGtkSalData()->PostUserEvent();
}

void GtkSalDisplay::deregisterFrame( SalFrame* pFrame )
{
    if( m_pCapture == pFrame )
    {
        static_cast<GtkSalFrame*>(m_pCapture)->grabPointer( FALSE );
        m_pCapture = NULL;
    }
    SalGenericDisplay::deregisterFrame( pFrame );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
