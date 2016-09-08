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
#include <unx/gtk/gtksalmenu.hxx>
#include <unx/salobj.h>
#include <unx/geninst.h>
#include <osl/thread.h>
#include <osl/process.h>

#include "unx/i18n_im.hxx"
#include "unx/i18n_xkb.hxx"
#include <unx/wmadaptor.hxx>

#include "unx/x11_cursors/salcursors.h"

#include <vcl/svapp.hxx>

#ifdef GDK_WINDOWING_X11
#  include <gdk/gdkx.h>
#endif

using namespace vcl_sal;

/***************************************************************
 * class GtkSalDisplay                                         *
 ***************************************************************/
extern "C" {
GdkFilterReturn call_filterGdkEvent( GdkXEvent* sys_event,
                                     GdkEvent* event,
                                     gpointer data )
{
    GtkSalDisplay *pDisplay = static_cast<GtkSalDisplay *>(data);
    return pDisplay->filterGdkEvent( sys_event, event );
}
}

GtkSalDisplay::GtkSalDisplay( GdkDisplay* pDisplay ) :
            m_pSys( GtkSalSystem::GetSingleton() ),
            m_pGdkDisplay( pDisplay ),
            m_bStartupCompleted( false )
{
    for(GdkCursor* & rpCsr : m_aCursors)
        rpCsr = nullptr;

    // FIXME: unify this with SalInst's filter too ?
    gdk_window_add_filter( nullptr, call_filterGdkEvent, this );

    if ( getenv( "SAL_IGNOREXERRORS" ) )
        GetGenericData()->ErrorTrapPush(); // and leak the trap

    m_bX11Display = GDK_IS_X11_DISPLAY( m_pGdkDisplay );

#if GTK_CHECK_VERSION(3,10,0)
#ifdef GDK_WINDOWING_X11
    if (m_bX11Display)
    {
        if (!getenv("GDK_SCALE"))
        {
            gdk_x11_display_set_window_scale(m_pGdkDisplay, 1);
        }
    }
#endif
#endif

    gtk_widget_set_default_direction(AllSettings::GetLayoutRTL() ? GTK_TEXT_DIR_RTL : GTK_TEXT_DIR_LTR);
}

GtkSalDisplay::~GtkSalDisplay()
{
    gdk_window_remove_filter( nullptr, call_filterGdkEvent, this );

    if( !m_bStartupCompleted )
        gdk_notify_startup_complete();

    for(GdkCursor* & rpCsr : m_aCursors)
        if( rpCsr )
            gdk_cursor_unref( rpCsr );
}

extern "C" {

void signalScreenSizeChanged( GdkScreen* pScreen, gpointer data )
{
    GtkSalDisplay* pDisp = static_cast<GtkSalDisplay*>(data);
    pDisp->screenSizeChanged( pScreen );
}

void signalMonitorsChanged( GdkScreen* pScreen, gpointer data )
{
    GtkSalDisplay* pDisp = static_cast<GtkSalDisplay*>(data);
    pDisp->monitorsChanged( pScreen );
}

}

GdkFilterReturn GtkSalDisplay::filterGdkEvent( GdkXEvent* sys_event,
                                               GdkEvent* )
{
    (void) this; // loplugin:staticmethods
    (void) sys_event;
    //FIXME: implement filterGdkEvent ...
    return GDK_FILTER_CONTINUE;
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

GdkCursor* GtkSalDisplay::getFromXBM( const unsigned char *pBitmap,
                                      const unsigned char *pMask,
                                      int nWidth, int nHeight,
                                      int nXHot, int nYHot )
{
    int cairo_stride = cairo_format_stride_for_width(CAIRO_FORMAT_A1, nWidth);

    unsigned char *pPaddedXBM = ensurePaddedForCairo(pBitmap, nWidth, nHeight, cairo_stride);
    cairo_surface_t *source = cairo_image_surface_create_for_data(
        pPaddedXBM,
        CAIRO_FORMAT_A1, nWidth, nHeight,
        cairo_stride);

    unsigned char *pPaddedMaskXBM = ensurePaddedForCairo(pMask, nWidth, nHeight, cairo_stride);
    cairo_surface_t *mask = cairo_image_surface_create_for_data(
        pPaddedMaskXBM,
        CAIRO_FORMAT_A1, nWidth, nHeight,
        cairo_stride);

    cairo_surface_t *s = cairo_surface_create_similar(source, CAIRO_CONTENT_ALPHA, nWidth, nHeight);
    cairo_t *cr = cairo_create(s);
    cairo_set_source_surface(cr, source, 0, 0);
    cairo_mask_surface(cr, mask, 0, 0);
    cairo_destroy(cr);

#if GTK_CHECK_VERSION(3,10,0)
    GdkCursor *cursor = gdk_cursor_new_from_surface(m_pGdkDisplay, s, nXHot, nYHot);
#else
    GdkPixbuf *pixbuf = gdk_pixbuf_get_from_surface(s, 0, 0, nWidth, nHeight);
    GdkCursor *cursor = gdk_cursor_new_from_pixbuf(m_pGdkDisplay, pixbuf, nXHot, nYHot);
    g_object_unref(pixbuf);
#endif

    cairo_surface_destroy(s);
    cairo_surface_destroy(mask);
    cairo_surface_destroy(source);

    if (pPaddedMaskXBM != pMask)
        delete [] pPaddedMaskXBM;

    if (pPaddedXBM != pBitmap)
        delete [] pPaddedXBM;

    return cursor;
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
    if ( !m_aCursors[ ePointerStyle ] )
    {
        GdkCursor *pCursor = nullptr;

        switch( ePointerStyle )
        {
            MAP_BUILTIN( PointerStyle::Arrow, GDK_LEFT_PTR );
            MAP_BUILTIN( PointerStyle::Text, GDK_XTERM );
            MAP_BUILTIN( PointerStyle::Help, GDK_QUESTION_ARROW );
            MAP_BUILTIN( PointerStyle::Cross, GDK_CROSSHAIR );
            MAP_BUILTIN( PointerStyle::Wait, GDK_WATCH );

            MAP_BUILTIN( PointerStyle::NSize, GDK_SB_V_DOUBLE_ARROW );
            MAP_BUILTIN( PointerStyle::SSize, GDK_SB_V_DOUBLE_ARROW );
            MAP_BUILTIN( PointerStyle::WSize, GDK_SB_H_DOUBLE_ARROW );
            MAP_BUILTIN( PointerStyle::ESize, GDK_SB_H_DOUBLE_ARROW );

            MAP_BUILTIN( PointerStyle::NWSize, GDK_TOP_LEFT_CORNER );
            MAP_BUILTIN( PointerStyle::NESize, GDK_TOP_RIGHT_CORNER );
            MAP_BUILTIN( PointerStyle::SWSize, GDK_BOTTOM_LEFT_CORNER );
            MAP_BUILTIN( PointerStyle::SESize, GDK_BOTTOM_RIGHT_CORNER );

            MAP_BUILTIN( PointerStyle::WindowNSize, GDK_TOP_SIDE );
            MAP_BUILTIN( PointerStyle::WindowSSize, GDK_BOTTOM_SIDE );
            MAP_BUILTIN( PointerStyle::WindowWSize, GDK_LEFT_SIDE );
            MAP_BUILTIN( PointerStyle::WindowESize, GDK_RIGHT_SIDE );

            MAP_BUILTIN( PointerStyle::WindowNWSize, GDK_TOP_LEFT_CORNER );
            MAP_BUILTIN( PointerStyle::WindowNESize, GDK_TOP_RIGHT_CORNER );
            MAP_BUILTIN( PointerStyle::WindowSWSize, GDK_BOTTOM_LEFT_CORNER );
            MAP_BUILTIN( PointerStyle::WindowSESize, GDK_BOTTOM_RIGHT_CORNER );

            MAP_BUILTIN( PointerStyle::HSizeBar, GDK_SB_H_DOUBLE_ARROW );
            MAP_BUILTIN( PointerStyle::VSizeBar, GDK_SB_V_DOUBLE_ARROW );

            MAP_BUILTIN( PointerStyle::RefHand, GDK_HAND2 );
            MAP_BUILTIN( PointerStyle::Hand, GDK_HAND2 );
            MAP_BUILTIN( PointerStyle::Pen, GDK_PENCIL );

            MAP_BUILTIN( PointerStyle::HSplit, GDK_SB_H_DOUBLE_ARROW );
            MAP_BUILTIN( PointerStyle::VSplit, GDK_SB_V_DOUBLE_ARROW );

            MAP_BUILTIN( PointerStyle::Move, GDK_FLEUR );

            MAKE_CURSOR( PointerStyle::Null, null );
            MAKE_CURSOR( PointerStyle::Magnify, magnify_ );
            MAKE_CURSOR( PointerStyle::Fill, fill_ );
            MAKE_CURSOR( PointerStyle::MoveData, movedata_ );
            MAKE_CURSOR( PointerStyle::CopyData, copydata_ );
            MAKE_CURSOR( PointerStyle::MoveFile, movefile_ );
            MAKE_CURSOR( PointerStyle::CopyFile, copyfile_ );
            MAKE_CURSOR( PointerStyle::MoveFiles, movefiles_ );
            MAKE_CURSOR( PointerStyle::CopyFiles, copyfiles_ );
            MAKE_CURSOR( PointerStyle::NotAllowed, nodrop_ );
            MAKE_CURSOR( PointerStyle::Rotate, rotate_ );
            MAKE_CURSOR( PointerStyle::HShear, hshear_ );
            MAKE_CURSOR( PointerStyle::VShear, vshear_ );
            MAKE_CURSOR( PointerStyle::DrawLine, drawline_ );
            MAKE_CURSOR( PointerStyle::DrawRect, drawrect_ );
            MAKE_CURSOR( PointerStyle::DrawPolygon, drawpolygon_ );
            MAKE_CURSOR( PointerStyle::DrawBezier, drawbezier_ );
            MAKE_CURSOR( PointerStyle::DrawArc, drawarc_ );
            MAKE_CURSOR( PointerStyle::DrawPie, drawpie_ );
            MAKE_CURSOR( PointerStyle::DrawCircleCut, drawcirclecut_ );
            MAKE_CURSOR( PointerStyle::DrawEllipse, drawellipse_ );
            MAKE_CURSOR( PointerStyle::DrawConnect, drawconnect_ );
            MAKE_CURSOR( PointerStyle::DrawText, drawtext_ );
            MAKE_CURSOR( PointerStyle::Mirror, mirror_ );
            MAKE_CURSOR( PointerStyle::Crook, crook_ );
            MAKE_CURSOR( PointerStyle::Crop, crop_ );
            MAKE_CURSOR( PointerStyle::MovePoint, movepoint_ );
            MAKE_CURSOR( PointerStyle::MoveBezierWeight, movebezierweight_ );
            MAKE_CURSOR( PointerStyle::DrawFreehand, drawfreehand_ );
            MAKE_CURSOR( PointerStyle::DrawCaption, drawcaption_ );
            MAKE_CURSOR( PointerStyle::LinkData, linkdata_ );
            MAKE_CURSOR( PointerStyle::MoveDataLink, movedlnk_ );
            MAKE_CURSOR( PointerStyle::CopyDataLink, copydlnk_ );
            MAKE_CURSOR( PointerStyle::LinkFile, linkfile_ );
            MAKE_CURSOR( PointerStyle::MoveFileLink, moveflnk_ );
            MAKE_CURSOR( PointerStyle::CopyFileLink, copyflnk_ );
            MAKE_CURSOR( PointerStyle::Chart, chart_ );
            MAKE_CURSOR( PointerStyle::Detective, detective_ );
            MAKE_CURSOR( PointerStyle::PivotCol, pivotcol_ );
            MAKE_CURSOR( PointerStyle::PivotRow, pivotrow_ );
            MAKE_CURSOR( PointerStyle::PivotField, pivotfld_ );
            MAKE_CURSOR( PointerStyle::PivotDelete, pivotdel_ );
            MAKE_CURSOR( PointerStyle::Chain, chain_ );
            MAKE_CURSOR( PointerStyle::ChainNotAllowed, chainnot_ );
            MAKE_CURSOR( PointerStyle::TimeEventMove, timemove_ );
            MAKE_CURSOR( PointerStyle::TimeEventSize, timesize_ );
            MAKE_CURSOR( PointerStyle::AutoScrollN, asn_ );
            MAKE_CURSOR( PointerStyle::AutoScrollS, ass_ );
            MAKE_CURSOR( PointerStyle::AutoScrollW, asw_ );
            MAKE_CURSOR( PointerStyle::AutoScrollE, ase_ );
            MAKE_CURSOR( PointerStyle::AutoScrollNW, asnw_ );
            MAKE_CURSOR( PointerStyle::AutoScrollNE, asne_ );
            MAKE_CURSOR( PointerStyle::AutoScrollSW, assw_ );
            MAKE_CURSOR( PointerStyle::AutoScrollSE, asse_ );
            MAKE_CURSOR( PointerStyle::AutoScrollNS, asns_ );
            MAKE_CURSOR( PointerStyle::AutoScrollWE, aswe_ );
            MAKE_CURSOR( PointerStyle::AutoScrollNSWE, asnswe_ );
            MAKE_CURSOR( PointerStyle::Airbrush, airbrush_ );
            MAKE_CURSOR( PointerStyle::TextVertical, vertcurs_ );

            // #i32329#
            MAKE_CURSOR( PointerStyle::TabSelectS, tblsels_ );
            MAKE_CURSOR( PointerStyle::TabSelectE, tblsele_ );
            MAKE_CURSOR( PointerStyle::TabSelectSE, tblselse_ );
            MAKE_CURSOR( PointerStyle::TabSelectW, tblselw_ );
            MAKE_CURSOR( PointerStyle::TabSelectSW, tblselsw_ );

            // #i20119#
            MAKE_CURSOR( PointerStyle::Paintbrush, paintbrush_ );

            MAKE_CURSOR( PointerStyle::HideWhitespace, hidewhitespace_ );
            MAKE_CURSOR( PointerStyle::ShowWhitespace, showwhitespace_ );

        default:
            SAL_WARN( "vcl.gtk", "pointer " << static_cast<int>(ePointerStyle) << "not implemented" );
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
        m_pCapture = nullptr;
        return 0;
    }

    if( m_pCapture )
    {
        if( pFrame == m_pCapture )
            return 1;
        static_cast<GtkSalFrame*>(m_pCapture)->grabPointer( FALSE );
    }

    m_pCapture = pFrame;
    pFrame->grabPointer( TRUE );
    return 1;
}

/**********************************************************************
 * class GtkData                                                      *
 **********************************************************************/

GtkData::GtkData( SalInstance *pInstance )
    : SalGenericData( SAL_DATA_GTK3, pInstance )
    , m_aDispatchMutex()
    , blockIdleTimeout( false )
{
    m_pUserEvent = nullptr;
    m_aDispatchCondition = osl_createCondition();
}

#if defined(GDK_WINDOWING_X11)
XIOErrorHandler aOrigXIOErrorHandler = nullptr;

extern "C" {

static int XIOErrorHdl(Display *)
{
    fprintf(stderr, "X IO Error\n");
    _exit(1);
        // avoid crashes in unrelated threads that still run while atexit
        // handlers are in progress
}

}
#endif

GtkData::~GtkData()
{
    Yield( true, true );
    g_warning ("TESTME: We used to have a stop-timer here, but the central code should do this");

     // sanity check: at this point nobody should be yielding, but wake them
     // up anyway before the condition they're waiting on gets destroyed.
    osl_setCondition( m_aDispatchCondition );

    osl::MutexGuard g( m_aDispatchMutex );
    if (m_pUserEvent)
    {
        g_source_destroy (m_pUserEvent);
        g_source_unref (m_pUserEvent);
        m_pUserEvent = nullptr;
    }
    osl_destroyCondition( m_aDispatchCondition );
#if defined(GDK_WINDOWING_X11)
    if (GDK_IS_X11_DISPLAY(gdk_display_get_default()))
        XSetIOErrorHandler(aOrigXIOErrorHandler);
#endif
}

void GtkData::Dispose()
{
    deInitNWF();
}

/// Allows events to be processed, returns true if we processed an event.
bool GtkData::Yield( bool bWait, bool bHandleAllCurrentEvents )
{
    blockIdleTimeout = !bWait;

    /* #i33212# only enter g_main_context_iteration in one thread at any one
     * time, else one of them potentially will never end as long as there is
     * another thread in there. Having only one yielding thread actually dispatch
     * fits the vcl event model (see e.g. the generic plugin).
     */
    bool bDispatchThread = false;
    bool bWasEvent = false;
    {
        // release YieldMutex (and re-acquire at block end)
        SalYieldMutexReleaser aReleaser;
        if( m_aDispatchMutex.tryToAcquire() )
            bDispatchThread = true;
        else if( ! bWait )
        {
            blockIdleTimeout = false;
            return false; // someone else is waiting already, return
        }

        if( bDispatchThread )
        {
            int nMaxEvents = bHandleAllCurrentEvents ? 100 : 1;
            gboolean wasOneEvent = TRUE;
            while( nMaxEvents-- && wasOneEvent )
            {
                wasOneEvent = g_main_context_iteration( nullptr, bWait && !bWasEvent );
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
        m_aDispatchMutex.release();
        if( bWasEvent )
            osl_setCondition( m_aDispatchCondition ); // trigger non dispatch thread yields
    }
    blockIdleTimeout = false;

    return bWasEvent;
}

void GtkData::Init()
{
    SAL_INFO( "vcl.gtk", "GtkMainloop::Init()" );

    /*
     * open connection to X11 Display
     * try in this order:
     *  o  -display command line parameter,
     *  o  $DISPLAY environment variable
     *  o  default display
     */

    GdkDisplay *pGdkDisp = nullptr;

    // is there a -display command line parameter?
    rtl_TextEncoding aEnc = osl_getThreadTextEncoding();
    int nParams = osl_getCommandArgCount();
    OString aDisplay;
    OUString aParam, aBin;
    char** pCmdLineAry = new char*[ nParams+1 ];
    osl_getExecutableFile( &aParam.pData );
    osl_getSystemPathFromFileURL( aParam.pData, &aBin.pData );
    pCmdLineAry[0] = g_strdup( OUStringToOString( aBin, aEnc ).getStr() );
    for (int i = 0; i < nParams; ++i)
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

    // Set consistent name of the root accessible
    OUString aAppName = Application::GetAppName();
    if( !aAppName.isEmpty() )
    {
        OString aPrgName = OUStringToOString(aAppName, aEnc);
        g_set_prgname(aPrgName.getStr());
    }

    // init gtk/gdk
    gtk_init_check( &nParams, &pCmdLineAry );
    gdk_error_trap_push();

    for (int i = 0; i < nParams; ++i)
        g_free( pCmdLineAry[i] );
    delete [] pCmdLineAry;

#if OSL_DEBUG_LEVEL > 1
    if (g_getenv("SAL_DEBUG_UPDATES"))
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

#if defined(GDK_WINDOWING_X11)
    if (GDK_IS_X11_DISPLAY(pGdkDisp))
        aOrigXIOErrorHandler = XSetIOErrorHandler(XIOErrorHdl);
#endif

    GtkSalDisplay *pDisplay = new GtkSalDisplay( pGdkDisp );
    SetDisplay( pDisplay );

    //FIXME: unwind keyboard extension bits

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
    if (bIgnoreError)
    {
        gdk_error_trap_pop_ignored (); // faster
        return false;
    }
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
        SalGtkTimeoutSource *pTSource = reinterpret_cast<SalGtkTimeoutSource *>(pSource);

        GTimeVal aTimeNow;
        g_get_current_time( &aTimeNow );

        return sal_gtk_timeout_expired( pTSource, nTimeoutMS, &aTimeNow );
    }

    static gboolean sal_gtk_timeout_check( GSource *pSource )
    {
        SalGtkTimeoutSource *pTSource = reinterpret_cast<SalGtkTimeoutSource *>(pSource);

        GTimeVal aTimeNow;
        g_get_current_time( &aTimeNow );

        return ( pTSource->aFireTime.tv_sec < aTimeNow.tv_sec ||
                 ( pTSource->aFireTime.tv_sec == aTimeNow.tv_sec &&
                   pTSource->aFireTime.tv_usec < aTimeNow.tv_usec ) );
    }

    static gboolean sal_gtk_timeout_dispatch( GSource *pSource, GSourceFunc, gpointer )
    {
        SalGtkTimeoutSource *pTSource = reinterpret_cast<SalGtkTimeoutSource *>(pSource);

        if( !pTSource->pInstance )
            return FALSE;

        GtkData *pSalData = static_cast< GtkData* >( GetSalData());

        osl::Guard< comphelper::SolarMutex > aGuard( pSalData->m_pInstance->GetYieldMutex() );

        sal_gtk_timeout_defer( pTSource );

        ImplSVData* pSVData = ImplGetSVData();
        if( pSVData->mpSalTimer )
        {
            // TODO: context_pending should be probably checked too, but it causes locking assertion failures
            bool idle = !pSalData->BlockIdleTimeout() && /*!g_main_context_pending( NULL ) &&*/ !gdk_events_pending();
            pSVData->mpSalTimer->CallCallback( idle );
        }

        return TRUE;
    }

    static GSourceFuncs sal_gtk_timeout_funcs =
    {
        sal_gtk_timeout_prepare,
        sal_gtk_timeout_check,
        sal_gtk_timeout_dispatch,
        nullptr, nullptr, nullptr
    };
}

static SalGtkTimeoutSource *
create_sal_gtk_timeout( GtkSalTimer *pTimer )
{
  GSource *pSource = g_source_new( &sal_gtk_timeout_funcs, sizeof( SalGtkTimeoutSource ) );
  SalGtkTimeoutSource *pTSource = reinterpret_cast<SalGtkTimeoutSource *>(pSource);
  pTSource->pInstance = pTimer;

  // #i36226# timers should be executed with lower priority
  // than XEvents like in generic plugin
  g_source_set_priority( pSource, G_PRIORITY_LOW );
  g_source_set_can_recurse( pSource, TRUE );
  g_source_set_callback( pSource,
                         /* unused dummy */ g_idle_remove_by_data,
                         nullptr, nullptr );
  g_source_attach( pSource, g_main_context_default() );
#ifdef DBG_UTIL
  g_source_set_name( pSource, "VCL timeout source" );
#endif

  sal_gtk_timeout_defer( pTSource );

  return pTSource;
}

GtkSalTimer::GtkSalTimer()
    : m_pTimeout(nullptr)
    , m_nTimeoutMS(0)
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
    // glib is not 64bit safe in this regard.
    assert( nMS <= G_MAXINT );
    m_nTimeoutMS = nMS; // for restarting
    Stop(); // FIXME: ideally re-use an existing m_pTimeout
    m_pTimeout = create_sal_gtk_timeout( this );
}

void GtkSalTimer::Stop()
{
    if( m_pTimeout )
    {
        g_source_destroy( &m_pTimeout->aParent );
        g_source_unref( &m_pTimeout->aParent );
        m_pTimeout = nullptr;
    }
}

gboolean GtkData::userEventFn( gpointer data )
{
    gboolean bContinue = FALSE;
    GtkData *pThis = static_cast<GtkData *>(data);
    SalGenericData *pData = GetGenericData();
    osl::Guard< comphelper::SolarMutex > aGuard( pData->m_pInstance->GetYieldMutex() );
    const SalGenericDisplay *pDisplay = pData->GetDisplay();
    if (pDisplay)
    {
        OSL_ASSERT(static_cast<const SalGenericDisplay *>(pThis->GetGtkDisplay()) == pDisplay);
        {
            osl::MutexGuard g (pThis->GetGtkDisplay()->getEventGuardMutex());

            if( !pThis->GetGtkDisplay()->HasUserEvents() )
            {
                if( pThis->m_pUserEvent )
                {
                    g_source_unref (pThis->m_pUserEvent);
                    pThis->m_pUserEvent = nullptr;
                }
                bContinue = FALSE;
            }
            else
                bContinue = TRUE;
        }
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
        g_main_context_wakeup (nullptr); // really needed ?
    else // nothing pending anyway
    {
        m_pUserEvent = g_idle_source_new();
        g_source_set_priority (m_pUserEvent, G_PRIORITY_HIGH);
        g_source_set_can_recurse (m_pUserEvent, TRUE);
        g_source_set_callback (m_pUserEvent, call_userEventFn,
                               static_cast<gpointer>(this), nullptr);
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
        m_pCapture = nullptr;
    }
    SalGenericDisplay::deregisterFrame( pFrame );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
