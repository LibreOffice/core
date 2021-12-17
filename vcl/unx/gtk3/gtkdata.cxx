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

#include <stdio.h>
#include <stdlib.h>
#if defined(FREEBSD) || defined(NETBSD)
#include <sys/types.h>
#include <sys/time.h>
#endif
#include <unx/gtk/gtkbackend.hxx>
#include <unx/gtk/gtkdata.hxx>
#include <unx/gtk/gtkinst.hxx>
#include <unx/gtk/gtkframe.hxx>
#include <bitmaps.hlst>
#include <cursor_hotspots.hxx>
#include <o3tl/safeint.hxx>
#include <o3tl/string_view.hxx>
#include <osl/thread.h>
#include <osl/process.h>

#include <vcl/svapp.hxx>
#include <sal/log.hxx>

#include <chrono>

using namespace vcl_sal;

/***************************************************************
 * class GtkSalDisplay                                         *
 ***************************************************************/

GtkSalDisplay::GtkSalDisplay( GdkDisplay* pDisplay ) :
            m_pSys( GtkSalSystem::GetSingleton() ),
            m_pGdkDisplay( pDisplay ),
            m_bStartupCompleted( false )
{
    for(GdkCursor* & rpCsr : m_aCursors)
        rpCsr = nullptr;

    if ( getenv( "SAL_IGNOREXERRORS" ) )
        GetGenericUnixSalData()->ErrorTrapPush(); // and leak the trap

    gtk_widget_set_default_direction(AllSettings::GetLayoutRTL() ? GTK_TEXT_DIR_RTL : GTK_TEXT_DIR_LTR);
}

GtkSalDisplay::~GtkSalDisplay()
{
#if !GTK_CHECK_VERSION(4, 0, 0)
    if( !m_bStartupCompleted )
        gdk_notify_startup_complete();

    for(GdkCursor* & rpCsr : m_aCursors)
        if( rpCsr )
            gdk_cursor_unref( rpCsr );
#endif
}

#if GTK_CHECK_VERSION(4, 0, 0)

static void signalMonitorsChanged(GListModel*, gpointer data)
{
    GtkSalDisplay* pDisp = static_cast<GtkSalDisplay*>(data);
    pDisp->emitDisplayChanged();
}

#else

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
#endif

GdkCursor* GtkSalDisplay::getFromSvg(OUString const & name, int nXHot, int nYHot)
{
    GdkPixbuf* pPixBuf = load_icon_by_name(name);
    assert(pPixBuf && "missing image?");
    if (!pPixBuf)
        return nullptr;

#if !GTK_CHECK_VERSION(4, 0, 0)
    guint nDefaultCursorSize = gdk_display_get_default_cursor_size( m_pGdkDisplay );
    int nPixWidth = gdk_pixbuf_get_width(pPixBuf);
    int nPixHeight = gdk_pixbuf_get_height(pPixBuf);
    double fScalefactor = static_cast<double>(nDefaultCursorSize) / std::max(nPixWidth, nPixHeight);
    GdkPixbuf* pScaledPixBuf = gdk_pixbuf_scale_simple(pPixBuf,
                                                       nPixWidth * fScalefactor,
                                                       nPixHeight * fScalefactor,
                                                       GDK_INTERP_HYPER);
    g_object_unref(pPixBuf);
    return gdk_cursor_new_from_pixbuf(m_pGdkDisplay, pScaledPixBuf,
                                      nXHot * fScalefactor, nYHot * fScalefactor);
#else
    GdkTexture* pTexture = gdk_texture_new_for_pixbuf(pPixBuf);
    g_object_unref(pPixBuf);
    return gdk_cursor_new_from_texture(pTexture, nXHot, nYHot, nullptr);
#endif
}

#define MAKE_CURSOR( vcl_name, name, name2 ) \
    case vcl_name: \
        pCursor = getFromSvg(name2, name##curs_x_hot, name##curs_y_hot); \
        break

#if !GTK_CHECK_VERSION(4, 0, 0)
#define MAP_BUILTIN( vcl_name, gdk3_name, css_name ) \
    case vcl_name: \
        pCursor = gdk_cursor_new_for_display( m_pGdkDisplay, gdk3_name ); \
        break
#else
#define MAP_BUILTIN( vcl_name, gdk3_name, css_name ) \
    case vcl_name: \
        pCursor = gdk_cursor_new_from_name(css_name, nullptr); \
        break
#endif

GdkCursor *GtkSalDisplay::getCursor( PointerStyle ePointerStyle )
{
    if ( !m_aCursors[ ePointerStyle ] )
    {
        GdkCursor *pCursor = nullptr;

        switch( ePointerStyle )
        {
            MAP_BUILTIN( PointerStyle::Arrow, GDK_LEFT_PTR, "default" );
            MAP_BUILTIN( PointerStyle::Text, GDK_XTERM, "text" );
            MAP_BUILTIN( PointerStyle::Help, GDK_QUESTION_ARROW, "help" );
            MAP_BUILTIN( PointerStyle::Cross, GDK_CROSSHAIR, "crosshair" );
            MAP_BUILTIN( PointerStyle::Wait, GDK_WATCH, "wait" );

            MAP_BUILTIN( PointerStyle::NSize, GDK_SB_V_DOUBLE_ARROW, "n-resize" );
            MAP_BUILTIN( PointerStyle::SSize, GDK_SB_V_DOUBLE_ARROW, "s-resize" );
            MAP_BUILTIN( PointerStyle::WSize, GDK_SB_H_DOUBLE_ARROW, "w-resize" );
            MAP_BUILTIN( PointerStyle::ESize, GDK_SB_H_DOUBLE_ARROW, "e-resize" );

            MAP_BUILTIN( PointerStyle::NWSize, GDK_TOP_LEFT_CORNER, "nw-resize" );
            MAP_BUILTIN( PointerStyle::NESize, GDK_TOP_RIGHT_CORNER, "ne-resize" );
            MAP_BUILTIN( PointerStyle::SWSize, GDK_BOTTOM_LEFT_CORNER, "sw-resize" );
            MAP_BUILTIN( PointerStyle::SESize, GDK_BOTTOM_RIGHT_CORNER, "se-resize" );

            MAP_BUILTIN( PointerStyle::WindowNSize, GDK_TOP_SIDE, "n-resize" );
            MAP_BUILTIN( PointerStyle::WindowSSize, GDK_BOTTOM_SIDE, "s-resize" );
            MAP_BUILTIN( PointerStyle::WindowWSize, GDK_LEFT_SIDE, "w-resize" );
            MAP_BUILTIN( PointerStyle::WindowESize, GDK_RIGHT_SIDE, "e-resize" );

            MAP_BUILTIN( PointerStyle::WindowNWSize, GDK_TOP_LEFT_CORNER, "nw-resize" );
            MAP_BUILTIN( PointerStyle::WindowNESize, GDK_TOP_RIGHT_CORNER, "ne-resize" );
            MAP_BUILTIN( PointerStyle::WindowSWSize, GDK_BOTTOM_LEFT_CORNER, "sw-resize" );
            MAP_BUILTIN( PointerStyle::WindowSESize, GDK_BOTTOM_RIGHT_CORNER, "se-resize" );

            MAP_BUILTIN( PointerStyle::HSizeBar, GDK_SB_H_DOUBLE_ARROW, "col-resize" );
            MAP_BUILTIN( PointerStyle::VSizeBar, GDK_SB_V_DOUBLE_ARROW, "row-resize" );

            MAP_BUILTIN( PointerStyle::RefHand, GDK_HAND2, "grab" );
            MAP_BUILTIN( PointerStyle::Hand, GDK_HAND2, "grab" );

#if !GTK_CHECK_VERSION(4, 0, 0)
            MAP_BUILTIN( PointerStyle::Pen, GDK_PENCIL, "" );
#else
            MAKE_CURSOR( PointerStyle::Pen, pen_, RID_CURSOR_PEN );
#endif

            MAP_BUILTIN( PointerStyle::HSplit, GDK_SB_H_DOUBLE_ARROW, "col-resize" );
            MAP_BUILTIN( PointerStyle::VSplit, GDK_SB_V_DOUBLE_ARROW, "row-resize" );

            MAP_BUILTIN( PointerStyle::Move, GDK_FLEUR, "move" );

#if !GTK_CHECK_VERSION(4, 0, 0)
            MAKE_CURSOR( PointerStyle::Null, null, RID_CURSOR_NULL );
#else
            MAP_BUILTIN( PointerStyle::Null, 0, "none" );
#endif

            MAKE_CURSOR( PointerStyle::Magnify, magnify_, RID_CURSOR_MAGNIFY );
            MAKE_CURSOR( PointerStyle::Fill, fill_, RID_CURSOR_FILL );
            MAKE_CURSOR( PointerStyle::MoveData, movedata_, RID_CURSOR_MOVE_DATA );
            MAKE_CURSOR( PointerStyle::CopyData, copydata_, RID_CURSOR_COPY_DATA );
            MAKE_CURSOR( PointerStyle::MoveFile, movefile_, RID_CURSOR_MOVE_FILE );
            MAKE_CURSOR( PointerStyle::CopyFile, copyfile_, RID_CURSOR_COPY_FILE );
            MAKE_CURSOR( PointerStyle::MoveFiles, movefiles_, RID_CURSOR_MOVE_FILES );
            MAKE_CURSOR( PointerStyle::CopyFiles, copyfiles_, RID_CURSOR_COPY_FILES );

#if !GTK_CHECK_VERSION(4, 0, 0)
            MAKE_CURSOR( PointerStyle::NotAllowed, nodrop_, RID_CURSOR_NOT_ALLOWED );
#else
            MAP_BUILTIN( PointerStyle::NotAllowed, 0, "not-allowed" );
#endif

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
            MAKE_CURSOR( PointerStyle::FatCross, fatcross_, RID_CURSOR_FATCROSS );

        default:
            SAL_WARN( "vcl.gtk", "pointer " << static_cast<int>(ePointerStyle) << "not implemented" );
            break;
        }
        if( !pCursor )
        {
#if !GTK_CHECK_VERSION(4, 0, 0)
            pCursor = gdk_cursor_new_for_display( m_pGdkDisplay, GDK_LEFT_PTR );
#else
            pCursor = gdk_cursor_new_from_name("normal", nullptr);
#endif
        }

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
            static_cast<GtkSalFrame*>(m_pCapture)->grabPointer( false, false, false );
        m_pCapture = nullptr;
        return 0;
    }

    if( m_pCapture )
    {
        if( pFrame == m_pCapture )
            return 1;
        static_cast<GtkSalFrame*>(m_pCapture)->grabPointer( false, false, false );
    }

    m_pCapture = pFrame;
    pFrame->grabPointer( true, false, false );
    return 1;
}

/**********************************************************************
 * class GtkSalData                                                   *
 **********************************************************************/

GtkSalData::GtkSalData( SalInstance *pInstance )
    : GenericUnixSalData(pInstance)
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
            bool wasOneEvent = true;
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
             * workaround: timeout of 1 second an emergency exit
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

static GtkStyleProvider* CreateStyleProvider()
{
    /*
       set a provider to:

       1) allow certain widgets to have no padding

       1.a) little close button in menubar to close back to start-center
       1.b) and small buttons in view->data sources (button.small-button)
       1.c.1) gtk3 small toolbar button in infobars (toolbar.small-button button)
       1.c.2) gtk4 small toolbar button in infobars (box.small-button button)
       1.d) comboboxes in the data browser for tdf#137695 (box#combobox button.small-button,
          which would instead be combobox button.small-button if we didn't replace GtkComboBox,
          see GtkInstanceComboBox for an explanation for why we do that)
       1.e) entry in the data browser for tdf#137695 (entry.small-button)
       1.f) spinbutton in the data browser tdf#141633 (spinbutton.small-button)

       2) hide the unwanted active tab in an 'overflow' notebook of double-decker notebooks.
          (tdf#122623) it's nigh impossible to have a GtkNotebook without an active (checked) tab,
          so theme the unwanted tab into invisibility
    */
    GtkCssProvider* pStyleProvider = gtk_css_provider_new();
    static const gchar data[] =
      "button.small-button, toolbar.small-button button, box.small-button button, "
      "combobox.small-button *.combo, box#combobox.small-button *.combo, entry.small-button, "
      "spinbutton.small-button, spinbutton.small-button entry, spinbutton.small-button button { "
      "padding: 0; margin-left: 0; margin-right: 0; margin-top: 0; margin-bottom: 0;"
      "border-width: 0; min-height: 0; min-width: 0; }"
      "notebook.overflow > header.top > tabs > tab:checked { "
      "box-shadow: none; padding: 0 0 0 0; margin: 0 0 0 0;"
      "border-image: none; border-image-width: 0 0 0 0;"
      "background-image: none; background-color: transparent;"
      "border-radius: 0 0 0 0; border-width: 0 0 0 0;"
      "border-style: none; border-color: transparent;"
      "opacity: 0; min-height: 0; min-width: 0; }";
    css_provider_load_from_data(pStyleProvider, data, -1);
    return GTK_STYLE_PROVIDER(pStyleProvider);
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
#if GTK_CHECK_VERSION(4, 0, 0)
    gtk_init_check();
#else
    gtk_init_check( &nParams, &pCmdLineAry );
#endif

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

    ErrorTrapPush();

#if defined(GDK_WINDOWING_X11)
    if (DLSYM_GDK_IS_X11_DISPLAY(pGdkDisp))
        aOrigXIOErrorHandler = XSetIOErrorHandler(XIOErrorHdl);
#endif

    GtkSalDisplay *pDisplay = new GtkSalDisplay( pGdkDisp );
    SetDisplay( pDisplay );

#if GTK_CHECK_VERSION(4, 0, 0)
    pDisplay->emitDisplayChanged();
    GListModel *pMonitors = gdk_display_get_monitors(pGdkDisp);
    g_signal_connect(pMonitors, "items-changed", G_CALLBACK(signalMonitorsChanged), pDisplay);

    gtk_style_context_add_provider_for_display(pGdkDisp, CreateStyleProvider(),
            GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
#else
    int nScreens = gdk_display_get_n_screens( pGdkDisp );
    for( int n = 0; n < nScreens; n++ )
    {
        GdkScreen *pScreen = gdk_display_get_screen( pGdkDisp, n );
        if (!pScreen)
            continue;

        pDisplay->screenSizeChanged( pScreen );
        pDisplay->monitorsChanged( pScreen );
        // add signal handler to notify screen size changes
        g_signal_connect( G_OBJECT(pScreen), "size-changed",
                          G_CALLBACK(signalScreenSizeChanged), pDisplay );
        g_signal_connect( G_OBJECT(pScreen), "monitors-changed",
                          G_CALLBACK(signalMonitorsChanged), pDisplay );

        gtk_style_context_add_provider_for_screen(pScreen, CreateStyleProvider(),
            GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    }
#endif
}

void GtkSalData::ErrorTrapPush()
{
#if GTK_CHECK_VERSION(4, 0, 0)
# if defined(GDK_WINDOWING_X11)
    GdkDisplay* pGdkDisp = gdk_display_get_default();
    if (DLSYM_GDK_IS_X11_DISPLAY(pGdkDisp))
        gdk_x11_display_error_trap_push(pGdkDisp);
# endif
#else
    gdk_error_trap_push();
#endif
}

bool GtkSalData::ErrorTrapPop( bool bIgnoreError )
{
#if GTK_CHECK_VERSION(4, 0, 0)
# if defined(GDK_WINDOWING_X11)
    GdkDisplay* pGdkDisp = gdk_display_get_default();
    if (DLSYM_GDK_IS_X11_DISPLAY(pGdkDisp))
    {
        if (bIgnoreError)
        {
            gdk_x11_display_error_trap_pop_ignored(pGdkDisp); // faster
            return false;
        }
        return gdk_x11_display_error_trap_pop(pGdkDisp) != 0;
    }
# endif
    return false;
#else
    if (bIgnoreError)
    {
        gdk_error_trap_pop_ignored (); // faster
        return false;
    }
    return gdk_error_trap_pop () != 0;
#endif
}

#if !GLIB_CHECK_VERSION(2,32,0)
#define G_SOURCE_REMOVE FALSE
#endif

extern "C" {

    struct SalGtkTimeoutSource {
        GSource      aParent;
        gint64       aFireTime;
        GtkSalTimer *pInstance;
    };

    static void sal_gtk_timeout_defer( SalGtkTimeoutSource *pTSource )
    {
        pTSource->aFireTime = g_get_real_time() + pTSource->pInstance->m_nTimeoutMS * 1000;
    }

    static gboolean sal_gtk_timeout_expired( SalGtkTimeoutSource *pTSource,
                                             gint *nTimeoutMS, gint64 const pTimeNow )
    {
        gint64 nDeltaUSec = pTSource->aFireTime - pTimeNow;
        if( nDeltaUSec < 0 )
        {
            *nTimeoutMS = 0;
            return true;
        }
        // if the clock changes backwards we need to cope ...
        if( o3tl::make_unsigned(nDeltaUSec) > 1000000 + ( pTSource->pInstance->m_nTimeoutMS / 1000 ) )
        {
            sal_gtk_timeout_defer( pTSource );
            return true;
        }
        *nTimeoutMS = MIN( G_MAXINT,  (nDeltaUSec + 999) / 1000 );
        return *nTimeoutMS == 0;
    }

    static gboolean sal_gtk_timeout_prepare( GSource *pSource, gint *nTimeoutMS )
    {
        SalGtkTimeoutSource *pTSource = reinterpret_cast<SalGtkTimeoutSource *>(pSource);

        gint64 aTimeNow;
        aTimeNow = g_get_real_time();

        return sal_gtk_timeout_expired( pTSource, nTimeoutMS, aTimeNow );
    }

    static gboolean sal_gtk_timeout_check( GSource *pSource )
    {
        SalGtkTimeoutSource *pTSource = reinterpret_cast<SalGtkTimeoutSource *>(pSource);

        gint64 aTimeNow;
        aTimeNow = g_get_real_time();

        return ( pTSource->aFireTime < aTimeNow );
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
  g_source_set_can_recurse( pSource, true );
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
    gint64 aTimeNow;
    aTimeNow = g_get_real_time();
    return !!sal_gtk_timeout_expired( m_pTimeout, &nDummy, aTimeNow);
}

void GtkSalTimer::Start( sal_uInt64 nMS )
{
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
        return true;
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
        g_source_set_can_recurse (m_pUserEvent, true);
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
        if (pEnvData->GetWindowHandle(pSalFrame) == hWindow)
            return GTK_WIDGET(pEnvData->pWidget);
    }
    return nullptr;
}

void GtkSalDisplay::deregisterFrame( SalFrame* pFrame )
{
    if( m_pCapture == pFrame )
    {
        static_cast<GtkSalFrame*>(m_pCapture)->grabPointer( false, false, false );
        m_pCapture = nullptr;
    }
    SalGenericDisplay::deregisterFrame( pFrame );
}

namespace {

struct ButtonOrder
{
    std::string_view m_aType;
    int m_nPriority;
};

}

int getButtonPriority(std::string_view rType)
{
    static const size_t N_TYPES = 8;
    static const ButtonOrder aDiscardCancelSave[N_TYPES] =
    {
        { "discard", 0 },
        { "cancel", 1 },
        { "close", 1 },
        { "no", 2 },
        { "open", 3 },
        { "save", 3 },
        { "yes", 3 },
        { "ok", 3 }
    };

    static const ButtonOrder aSaveDiscardCancel[N_TYPES] =
    {
        { "open", 0 },
        { "save", 0 },
        { "yes", 0 },
        { "ok", 0 },
        { "discard", 1 },
        { "no", 1 },
        { "cancel", 2 },
        { "close", 2 }
    };

    const ButtonOrder* pOrder = &aDiscardCancelSave[0];

    const OUString &rEnv = Application::GetDesktopEnvironment();

    if (rEnv.equalsIgnoreAsciiCase("windows") ||
        rEnv.equalsIgnoreAsciiCase("tde") ||
        rEnv.startsWithIgnoreAsciiCase("kde"))
    {
        pOrder = &aSaveDiscardCancel[0];
    }

    for (size_t i = 0; i < N_TYPES; ++i, ++pOrder)
    {
        if (rType == pOrder->m_aType)
            return pOrder->m_nPriority;
    }

    return -1;
}

void container_remove(GtkWidget* pContainer, GtkWidget* pChild)
{
#if !GTK_CHECK_VERSION(4, 0, 0)
    gtk_container_remove(GTK_CONTAINER(pContainer), pChild);
#else
    assert(GTK_IS_BOX(pContainer) || GTK_IS_GRID(pContainer) || GTK_IS_POPOVER(pContainer) ||
           GTK_IS_FIXED(pContainer) || GTK_IS_WINDOW(pContainer));
    if (GTK_IS_BOX(pContainer))
        gtk_box_remove(GTK_BOX(pContainer), pChild);
    else if (GTK_IS_GRID(pContainer))
        gtk_grid_remove(GTK_GRID(pContainer), pChild);
    else if (GTK_IS_POPOVER(pContainer))
        gtk_popover_set_child(GTK_POPOVER(pContainer), nullptr);
    else if (GTK_IS_WINDOW(pContainer))
        gtk_window_set_child(GTK_WINDOW(pContainer), nullptr);
    else if (GTK_IS_FIXED(pContainer))
        gtk_fixed_remove(GTK_FIXED(pContainer), pChild);
#endif
}

void container_add(GtkWidget* pContainer, GtkWidget* pChild)
{
#if !GTK_CHECK_VERSION(4, 0, 0)
    gtk_container_add(GTK_CONTAINER(pContainer), pChild);
#else
    assert(GTK_IS_BOX(pContainer) || GTK_IS_GRID(pContainer) || GTK_IS_POPOVER(pContainer) ||
           GTK_IS_FIXED(pContainer) || GTK_IS_WINDOW(pContainer));
    if (GTK_IS_BOX(pContainer))
        gtk_box_append(GTK_BOX(pContainer), pChild);
    else if (GTK_IS_GRID(pContainer))
        gtk_grid_attach(GTK_GRID(pContainer), pChild, 0, 0, 1, 1);
    else if (GTK_IS_POPOVER(pContainer))
        gtk_popover_set_child(GTK_POPOVER(pContainer), pChild);
    else if (GTK_IS_WINDOW(pContainer))
        gtk_window_set_child(GTK_WINDOW(pContainer), pChild);
    else if (GTK_IS_FIXED(pContainer))
        gtk_fixed_put(GTK_FIXED(pContainer), pChild, 0, 0);
#endif
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
