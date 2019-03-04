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
#include <unx/gtk/gtkbackend.hxx>
#include <unx/gtk/gtkdata.hxx>
#include <unx/gtk/gtkinst.hxx>
#include <unx/gtk/gtkframe.hxx>
#include <unx/gtk/gtksalmenu.hxx>
#include <unx/salobj.h>
#include <unx/geninst.h>
#include <bitmaps.hlst>
#include <osl/thread.h>
#include <osl/process.h>

#include <unx/i18n_im.hxx>
#include <unx/i18n_xkb.hxx>
#include <unx/wmadaptor.hxx>

#include <vcl/svapp.hxx>
#include <vcl/BitmapTools.hxx>
#include <sal/log.hxx>

#ifdef GDK_WINDOWING_X11
#  include <gdk/gdkx.h>
#endif

#include <cppuhelper/exc_hlp.hxx>
#include <chrono>


#define nodrop_curs_x_hot 9
#define nodrop_curs_y_hot 9
#define magnify_curs_x_hot 12
#define magnify_curs_y_hot 13
#define rotate_curs_x_hot 15
#define rotate_curs_y_hot 15
#define hshear_curs_x_hot 15
#define hshear_curs_y_hot 15
#define vshear_curs_x_hot 15
#define vshear_curs_y_hot 15
#define drawline_curs_x_hot 7
#define drawline_curs_y_hot 7
#define drawrect_curs_x_hot 7
#define drawrect_curs_y_hot 7
#define drawpolygon_curs_x_hot 7
#define drawpolygon_curs_y_hot 7
#define drawbezier_curs_x_hot 7
#define drawbezier_curs_y_hot 7
#define drawarc_curs_x_hot 7
#define drawarc_curs_y_hot 7
#define drawpie_curs_x_hot 7
#define drawpie_curs_y_hot 7
#define drawcirclecut_curs_x_hot 7
#define drawcirclecut_curs_y_hot 7
#define drawellipse_curs_x_hot 7
#define drawellipse_curs_y_hot 7
#define drawconnect_curs_x_hot 7
#define drawconnect_curs_y_hot 7
#define drawtext_curs_x_hot 8
#define drawtext_curs_y_hot 8
#define mirror_curs_x_hot 14
#define mirror_curs_y_hot 12
#define crook_curs_x_hot 15
#define crook_curs_y_hot 14
#define crop_curs_x_hot 9
#define crop_curs_y_hot 9
#define movepoint_curs_x_hot 0
#define movepoint_curs_y_hot 0
#define movebezierweight_curs_x_hot 0
#define movebezierweight_curs_y_hot 0
#define drawfreehand_curs_x_hot 8
#define drawfreehand_curs_y_hot 8
#define drawcaption_curs_x_hot 8
#define drawcaption_curs_y_hot 8
#define movedata_curs_x_hot 1
#define movedata_curs_y_hot 1
#define copydata_curs_x_hot 1
#define copydata_curs_y_hot 1
#define linkdata_curs_x_hot 1
#define linkdata_curs_y_hot 1
#define movedlnk_curs_x_hot 1
#define movedlnk_curs_y_hot 1
#define copydlnk_curs_x_hot 1
#define copydlnk_curs_y_hot 1
#define movefile_curs_x_hot 9
#define movefile_curs_y_hot 9
#define copyfile_curs_x_hot 9
#define copyfile_curs_y_hot 9
#define linkfile_curs_x_hot 9
#define linkfile_curs_y_hot 9
#define moveflnk_curs_x_hot 9
#define moveflnk_curs_y_hot 9
#define copyflnk_curs_x_hot 9
#define copyflnk_curs_y_hot 9
#define movefiles_curs_x_hot 8
#define movefiles_curs_y_hot 9
#define copyfiles_curs_x_hot 8
#define copyfiles_curs_y_hot 9

#define chart_curs_x_hot 15
#define chart_curs_y_hot 16
#define detective_curs_x_hot 12
#define detective_curs_y_hot 13
#define pivotcol_curs_x_hot 7
#define pivotcol_curs_y_hot 5
#define pivotfld_curs_x_hot 8
#define pivotfld_curs_y_hot 7
#define pivotrow_curs_x_hot 8
#define pivotrow_curs_y_hot 7
#define pivotdel_curs_x_hot 9
#define pivotdel_curs_y_hot 8

#define chain_curs_x_hot 0
#define chain_curs_y_hot 2
#define chainnot_curs_x_hot 2
#define chainnot_curs_y_hot 2

#define ase_curs_x_hot 19
#define ase_curs_y_hot 16
#define asn_curs_x_hot 16
#define asn_curs_y_hot 12
#define asne_curs_x_hot 21
#define asne_curs_y_hot 10
#define asns_curs_x_hot 15
#define asns_curs_y_hot 15
#define asnswe_curs_x_hot 15
#define asnswe_curs_y_hot 15
#define asnw_curs_x_hot 10
#define asnw_curs_y_hot 10
#define ass_curs_x_hot 15
#define ass_curs_y_hot 19
#define asse_curs_x_hot 21
#define asse_curs_y_hot 21
#define assw_curs_x_hot 10
#define assw_curs_y_hot 21
#define asw_curs_x_hot 12
#define asw_curs_y_hot 15
#define aswe_curs_x_hot 15
#define aswe_curs_y_hot 15
#define nullcurs_x_hot 2
#define nullcurs_y_hot 2

#define fill_curs_x_hot 10
#define fill_curs_y_hot 22
#define vertcurs_curs_x_hot 8
#define vertcurs_curs_y_hot 8
#define tblsele_curs_x_hot 14
#define tblsele_curs_y_hot 8
#define tblsels_curs_x_hot 7
#define tblsels_curs_y_hot 14
#define tblselse_curs_x_hot 14
#define tblselse_curs_y_hot 14
#define tblselw_curs_x_hot 1
#define tblselw_curs_y_hot 8
#define tblselsw_curs_x_hot 1
#define tblselsw_curs_y_hot 14
#define hidewhitespace_curs_x_hot 0
#define hidewhitespace_curs_y_hot 10
#define showwhitespace_curs_x_hot 0
#define showwhitespace_curs_y_hot 10

using namespace vcl_sal;

/***************************************************************
 * class GtkSalDisplay                                         *
 ***************************************************************/
extern "C" {
static GdkFilterReturn call_filterGdkEvent( GdkXEvent* sys_event,
                                     GdkEvent* /*event*/,
                                     gpointer data )
{
    GtkSalDisplay *pDisplay = static_cast<GtkSalDisplay *>(data);
    return pDisplay->filterGdkEvent( sys_event );
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
        GetGenericUnixSalData()->ErrorTrapPush(); // and leak the trap

    m_bX11Display = DLSYM_GDK_IS_X11_DISPLAY( m_pGdkDisplay );

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

static void signalScreenSizeChanged( GdkScreen* pScreen, gpointer data )
{
    GtkSalDisplay* pDisp = static_cast<GtkSalDisplay*>(data);
    pDisp->screenSizeChanged( pScreen );
}

static void signalMonitorsChanged( GdkScreen* pScreen, gpointer data )
{
    GtkSalDisplay* pDisp = static_cast<GtkSalDisplay*>(data);
    pDisp->monitorsChanged( pScreen );
}

}

GdkFilterReturn GtkSalDisplay::filterGdkEvent( GdkXEvent* )
{
    (void) this; // loplugin:staticmethods
    //FIXME: implement filterGdkEvent ...
    return GDK_FILTER_CONTINUE;
}

void GtkSalDisplay::screenSizeChanged( GdkScreen const * pScreen )
{
    m_pSys->countScreenMonitors();
    if (pScreen)
        emitDisplayChanged();
}

void GtkSalDisplay::monitorsChanged( GdkScreen const * pScreen )
{
    m_pSys->countScreenMonitors();
    if (pScreen)
        emitDisplayChanged();
}

GdkCursor* GtkSalDisplay::getFromSvg(OUString const & name, int nXHot, int nYHot)
{
    GdkPixbuf* pPixBuf = load_icon_by_name(name);
    assert(pPixBuf && "missing image?");
    if (!pPixBuf)
        return nullptr;
    guint nDefaultCursorSize = gdk_display_get_default_cursor_size( m_pGdkDisplay );
    GdkPixbuf* pScaledPixBuf = gdk_pixbuf_scale_simple(pPixBuf, nDefaultCursorSize, nDefaultCursorSize, GDK_INTERP_HYPER);
    g_object_unref(pPixBuf);
    GdkCursor* pCursor = gdk_cursor_new_from_pixbuf( m_pGdkDisplay, pScaledPixBuf, nXHot, nYHot);
    return pCursor;
}

#define MAKE_CURSOR( vcl_name, name, name2 ) \
    case vcl_name: \
        pCursor = getFromSvg(name2, name##curs_x_hot, name##curs_y_hot); \
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

            MAKE_CURSOR( PointerStyle::Null, null, RID_CURSOR_NULL );
            MAKE_CURSOR( PointerStyle::Magnify, magnify_, RID_CURSOR_MAGNIFY );
            MAKE_CURSOR( PointerStyle::Fill, fill_, RID_CURSOR_FILL );
            MAKE_CURSOR( PointerStyle::MoveData, movedata_, RID_CURSOR_MOVE_DATA );
            MAKE_CURSOR( PointerStyle::CopyData, copydata_, RID_CURSOR_COPY_DATA );
            MAKE_CURSOR( PointerStyle::MoveFile, movefile_, RID_CURSOR_MOVE_FILE );
            MAKE_CURSOR( PointerStyle::CopyFile, copyfile_, RID_CURSOR_COPY_FILE );
            MAKE_CURSOR( PointerStyle::MoveFiles, movefiles_, RID_CURSOR_MOVE_FILES );
            MAKE_CURSOR( PointerStyle::CopyFiles, copyfiles_, RID_CURSOR_COPY_FILES );
            MAKE_CURSOR( PointerStyle::NotAllowed, nodrop_, RID_CURSOR_NOT_ALLOWED );
            MAKE_CURSOR( PointerStyle::Rotate, rotate_, RID_CURSOR_ROTATE );
            MAKE_CURSOR( PointerStyle::HShear, hshear_, RID_CURSOR_H_SHEAR );
            MAKE_CURSOR( PointerStyle::VShear, vshear_, RID_CURSOR_V_SHEAR );
            MAKE_CURSOR( PointerStyle::DrawLine, drawline_, RID_CURSOR_DRAW_LINE );
            MAKE_CURSOR( PointerStyle::DrawRect, drawrect_, RID_CURSOR_DRAW_RECT );
            MAKE_CURSOR( PointerStyle::DrawPolygon, drawpolygon_, RID_CURSOR_DRAW_POLYGON );
            MAKE_CURSOR( PointerStyle::DrawBezier, drawbezier_, RID_CURSOR_DRAW_BEZIER );
            MAKE_CURSOR( PointerStyle::DrawArc, drawarc_, RID_CURSOR_DRAW_ARC );
            MAKE_CURSOR( PointerStyle::DrawPie, drawpie_, RID_CURSOR_DRAW_PIE );
            MAKE_CURSOR( PointerStyle::DrawCircleCut, drawcirclecut_, RID_CURSOR_DRAW_CIRCLE_CUT );
            MAKE_CURSOR( PointerStyle::DrawEllipse, drawellipse_, RID_CURSOR_DRAW_ELLIPSE );
            MAKE_CURSOR( PointerStyle::DrawConnect, drawconnect_, RID_CURSOR_DRAW_CONNECT );
            MAKE_CURSOR( PointerStyle::DrawText, drawtext_, RID_CURSOR_DRAW_TEXT );
            MAKE_CURSOR( PointerStyle::Mirror, mirror_, RID_CURSOR_MIRROR );
            MAKE_CURSOR( PointerStyle::Crook, crook_, RID_CURSOR_CROOK );
            MAKE_CURSOR( PointerStyle::Crop, crop_, RID_CURSOR_CROP );
            MAKE_CURSOR( PointerStyle::MovePoint, movepoint_, RID_CURSOR_MOVE_POINT );
            MAKE_CURSOR( PointerStyle::MoveBezierWeight, movebezierweight_, RID_CURSOR_MOVE_BEZIER_WEIGHT );
            MAKE_CURSOR( PointerStyle::DrawFreehand, drawfreehand_, RID_CURSOR_DRAW_FREEHAND );
            MAKE_CURSOR( PointerStyle::DrawCaption, drawcaption_, RID_CURSOR_DRAW_CAPTION );
            MAKE_CURSOR( PointerStyle::LinkData, linkdata_, RID_CURSOR_LINK_DATA );
            MAKE_CURSOR( PointerStyle::MoveDataLink, movedlnk_, RID_CURSOR_MOVE_DATA_LINK );
            MAKE_CURSOR( PointerStyle::CopyDataLink, copydlnk_, RID_CURSOR_COPY_DATA_LINK );
            MAKE_CURSOR( PointerStyle::LinkFile, linkfile_, RID_CURSOR_LINK_FILE );
            MAKE_CURSOR( PointerStyle::MoveFileLink, moveflnk_, RID_CURSOR_MOVE_FILE_LINK );
            MAKE_CURSOR( PointerStyle::CopyFileLink, copyflnk_, RID_CURSOR_COPY_FILE_LINK );
            MAKE_CURSOR( PointerStyle::Chart, chart_, RID_CURSOR_CHART );
            MAKE_CURSOR( PointerStyle::Detective, detective_, RID_CURSOR_DETECTIVE );
            MAKE_CURSOR( PointerStyle::PivotCol, pivotcol_, RID_CURSOR_PIVOT_COLUMN );
            MAKE_CURSOR( PointerStyle::PivotRow, pivotrow_, RID_CURSOR_PIVOT_ROW );
            MAKE_CURSOR( PointerStyle::PivotField, pivotfld_, RID_CURSOR_PIVOT_FIELD );
            MAKE_CURSOR( PointerStyle::PivotDelete, pivotdel_, RID_CURSOR_PIVOT_DELETE );
            MAKE_CURSOR( PointerStyle::Chain, chain_, RID_CURSOR_CHAIN );
            MAKE_CURSOR( PointerStyle::ChainNotAllowed, chainnot_, RID_CURSOR_CHAIN_NOT_ALLOWED );
            MAKE_CURSOR( PointerStyle::AutoScrollN, asn_, RID_CURSOR_AUTOSCROLL_N );
            MAKE_CURSOR( PointerStyle::AutoScrollS, ass_, RID_CURSOR_AUTOSCROLL_S );
            MAKE_CURSOR( PointerStyle::AutoScrollW, asw_, RID_CURSOR_AUTOSCROLL_W );
            MAKE_CURSOR( PointerStyle::AutoScrollE, ase_, RID_CURSOR_AUTOSCROLL_E );
            MAKE_CURSOR( PointerStyle::AutoScrollNW, asnw_, RID_CURSOR_AUTOSCROLL_NW );
            MAKE_CURSOR( PointerStyle::AutoScrollNE, asne_, RID_CURSOR_AUTOSCROLL_NE );
            MAKE_CURSOR( PointerStyle::AutoScrollSW, assw_, RID_CURSOR_AUTOSCROLL_SW );
            MAKE_CURSOR( PointerStyle::AutoScrollSE, asse_, RID_CURSOR_AUTOSCROLL_SE );
            MAKE_CURSOR( PointerStyle::AutoScrollNS, asns_, RID_CURSOR_AUTOSCROLL_NS );
            MAKE_CURSOR( PointerStyle::AutoScrollWE, aswe_, RID_CURSOR_AUTOSCROLL_WE );
            MAKE_CURSOR( PointerStyle::AutoScrollNSWE, asnswe_, RID_CURSOR_AUTOSCROLL_NSWE );
            MAKE_CURSOR( PointerStyle::TextVertical, vertcurs_, RID_CURSOR_TEXT_VERTICAL );

            // #i32329#
            MAKE_CURSOR( PointerStyle::TabSelectS, tblsels_, RID_CURSOR_TAB_SELECT_S );
            MAKE_CURSOR( PointerStyle::TabSelectE, tblsele_, RID_CURSOR_TAB_SELECT_E );
            MAKE_CURSOR( PointerStyle::TabSelectSE, tblselse_, RID_CURSOR_TAB_SELECT_SE );
            MAKE_CURSOR( PointerStyle::TabSelectW, tblselw_, RID_CURSOR_TAB_SELECT_W );
            MAKE_CURSOR( PointerStyle::TabSelectSW, tblselsw_, RID_CURSOR_TAB_SELECT_SW );

            MAKE_CURSOR( PointerStyle::HideWhitespace, hidewhitespace_, RID_CURSOR_HIDE_WHITESPACE );
            MAKE_CURSOR( PointerStyle::ShowWhitespace, showwhitespace_, RID_CURSOR_SHOW_WHITESPACE );

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
 * class GtkSalData                                                   *
 **********************************************************************/

GtkSalData::GtkSalData( SalInstance *pInstance )
    : GenericUnixSalData( SAL_DATA_GTK3, pInstance )
    , m_aDispatchMutex()
    , m_aDispatchCondition()
    , m_pDocumentFocusListener(nullptr)
{
    m_pUserEvent = nullptr;
}

#if defined(GDK_WINDOWING_X11)
static XIOErrorHandler aOrigXIOErrorHandler = nullptr;

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

GtkSalData::~GtkSalData()
{
    Yield( true, true );
    g_warning ("TESTME: We used to have a stop-timer here, but the central code should do this");

    // sanity check: at this point nobody should be yielding, but wake them
    // up anyway before the condition they're waiting on gets destroyed.
    m_aDispatchCondition.set();

    osl::MutexGuard g( m_aDispatchMutex );
    if (m_pUserEvent)
    {
        g_source_destroy (m_pUserEvent);
        g_source_unref (m_pUserEvent);
        m_pUserEvent = nullptr;
    }
#if defined(GDK_WINDOWING_X11)
    if (DLSYM_GDK_IS_X11_DISPLAY(gdk_display_get_default()))
        XSetIOErrorHandler(aOrigXIOErrorHandler);
#endif
}

void GtkSalData::Dispose()
{
    deInitNWF();
}

/// Allows events to be processed, returns true if we processed an event.
bool GtkSalData::Yield( bool bWait, bool bHandleAllCurrentEvents )
{
    /* #i33212# only enter g_main_context_iteration in one thread at any one
     * time, else one of them potentially will never end as long as there is
     * another thread in there. Having only one yielding thread actually dispatch
     * fits the vcl event model (see e.g. the generic plugin).
     */
    bool bDispatchThread = false;
    bool bWasEvent = false;
    {
        // release YieldMutex (and re-acquire at block end)
        SolarMutexReleaser aReleaser;
        if( m_aDispatchMutex.tryToAcquire() )
            bDispatchThread = true;
        else if( ! bWait )
        {
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
            if (m_aException)
                std::rethrow_exception(m_aException);
        }
        else if( bWait )
        {
            /* #i41693# in case the dispatch thread hangs in join
             * for this thread the condition will never be set
             * workaround: timeout of 1 second a emergency exit
             */
            // we are the dispatch thread
            m_aDispatchCondition.reset();
            m_aDispatchCondition.wait(std::chrono::seconds(1));
        }
    }

    if( bDispatchThread )
    {
        m_aDispatchMutex.release();
        if( bWasEvent )
            m_aDispatchCondition.set(); // trigger non dispatch thread yields
    }

    return bWasEvent;
}

void GtkSalData::Init()
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
    if (DLSYM_GDK_IS_X11_DISPLAY(pGdkDisp))
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

void GtkSalData::ErrorTrapPush()
{
    gdk_error_trap_push ();
}

bool GtkSalData::ErrorTrapPop( bool bIgnoreError )
{
    if (bIgnoreError)
    {
        gdk_error_trap_pop_ignored (); // faster
        return false;
    }
    return gdk_error_trap_pop () != 0;
}

#if !GLIB_CHECK_VERSION(2,32,0)
#define G_SOURCE_REMOVE FALSE
#endif

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
                                             gint *nTimeoutMS, GTimeVal const *pTimeNow )
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
        if( static_cast<unsigned long>(nDeltaSec) > 1 + ( pTSource->pInstance->m_nTimeoutMS / 1000 ) )
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

        SolarMutexGuard aGuard;

        sal_gtk_timeout_defer( pTSource );

        ImplSVData* pSVData = ImplGetSVData();
        if( pSVData->maSchedCtx.mpSalTimer )
            pSVData->maSchedCtx.mpSalTimer->CallCallback();

        return G_SOURCE_REMOVE;
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
    pInstance->RemoveTimer();
    Stop();
}

bool GtkSalTimer::Expired()
{
    if( !m_pTimeout || g_source_is_destroyed( &m_pTimeout->aParent ) )
        return false;

    gint nDummy = 0;
    GTimeVal aTimeNow;
    g_get_current_time( &aTimeNow );
    return !!sal_gtk_timeout_expired( m_pTimeout, &nDummy, &aTimeNow);
}

void GtkSalTimer::Start( sal_uInt64 nMS )
{
    // glib is not 64bit safe in this regard.
    assert( nMS <= G_MAXINT );
    if ( nMS > G_MAXINT )
        nMS = G_MAXINT;
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

extern "C" {
    static gboolean call_userEventFn( void *data )
    {
        SolarMutexGuard aGuard;
        const SalGenericDisplay *pDisplay = GetGenericUnixSalData()->GetDisplay();
        if ( pDisplay )
        {
            GtkSalDisplay *pThisDisplay = static_cast<GtkSalData *>(data)->GetGtkDisplay();
            assert(static_cast<const SalGenericDisplay *>(pThisDisplay) == pDisplay);
            pThisDisplay->DispatchInternalEvent();
        }
        return TRUE;
    }
}

void GtkSalData::TriggerUserEventProcessing()
{
    if (m_pUserEvent)
        g_main_context_wakeup (nullptr); // really needed ?
    else // nothing pending anyway
    {
        m_pUserEvent = g_idle_source_new();
        // tdf#110737 set user-events to a lower priority than system redraw
        // events, which is G_PRIORITY_HIGH_IDLE + 20, so presentations
        // queue-redraw has a chance to be fulfilled
        g_source_set_priority (m_pUserEvent,  G_PRIORITY_HIGH_IDLE + 30);
        g_source_set_can_recurse (m_pUserEvent, TRUE);
        g_source_set_callback (m_pUserEvent, call_userEventFn,
                               static_cast<gpointer>(this), nullptr);
        g_source_attach (m_pUserEvent, g_main_context_default ());
    }
}

void GtkSalData::TriggerAllUserEventsProcessed()
{
    assert( m_pUserEvent );
    g_source_destroy( m_pUserEvent );
    g_source_unref( m_pUserEvent );
    m_pUserEvent = nullptr;
}

void GtkSalDisplay::TriggerUserEventProcessing()
{
    GetGtkSalData()->TriggerUserEventProcessing();
}

void GtkSalDisplay::TriggerAllUserEventsProcessed()
{
    GetGtkSalData()->TriggerAllUserEventsProcessed();
}

GtkWidget* GtkSalDisplay::findGtkWidgetForNativeHandle(sal_uIntPtr hWindow) const
{
    for (auto pSalFrame : m_aFrames )
    {
        const SystemEnvData* pEnvData = pSalFrame->GetSystemData();
        if (pEnvData->aWindow == hWindow)
            return GTK_WIDGET(pEnvData->pWidget);
    }
    return nullptr;
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
