/*************************************************************************
 *
 *  $RCSfile: gtkdata.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2004-02-20 08:53:25 $
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
#ifdef FREEBSD
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#endif

#ifndef _VCL_GTKDATA_HXX
#include <plugins/gtk/gtkdata.hxx>
#endif
#ifndef _VCL_GTKINST_HXX
#include <plugins/gtk/gtkinst.hxx>
#endif
#ifndef _VCL_GTKFRAME_HXX
#include <plugins/gtk/gtkframe.hxx>
#endif
#ifndef _OSL_THREAD_H_
#include <osl/thread.h>
#endif
#ifndef _OSL_PROCESS_H_
#include <osl/process.h>
#endif

#include <tools/debug.hxx>

#ifndef _SAL_I18N_INPUTMETHOD_HXX
#include "i18n_im.hxx"
#endif
#ifndef _SAL_I18N_XKBDEXTENSION_HXX
#include "i18n_xkb.hxx"
#endif

#include "../../unx/source/inc/salcursors.h"

using namespace rtl;

/***************************************************************************
 * class GtkDisplay                                                        *
 ***************************************************************************/

GtkSalDisplay::GtkSalDisplay( GdkDisplay* pDisplay, Visual* pVis, Colormap aCol )
            : m_pGdkDisplay( pDisplay ),
              SalDisplay( gdk_x11_display_get_xdisplay( pDisplay ), aCol )
{
    for(int i = 0; i < POINTER_COUNT; i++)
        m_aCursors[ i ] = NULL;
    Init ( aCol, pVis );
}

GtkSalDisplay::~GtkSalDisplay()
{
    doDestruct();

    for(int i = 0; i < POINTER_COUNT; i++)
        if( m_aCursors[ i ] )
            gdk_cursor_unref( m_aCursors[ i ] );

    pDisp_ = NULL;
}

void GtkSalDisplay::registerFrame( GtkSalFrame* pFrame )
{
    m_aFrames.push_back( pFrame );
}

void GtkSalDisplay::deregisterFrame( GtkSalFrame* pFrame )
{
    if( m_pCapture == pFrame )
    {
        static_cast<GtkSalFrame*>(m_pCapture)->grabPointer( FALSE );
        m_pCapture = NULL;
    }
    m_aFrames.remove( pFrame );
}

GdkFilterReturn GtkSalDisplay::filterGdkEvent( GdkXEvent* sys_event,
                                               GdkEvent* event,
                                               gpointer data )
{
    GTK_YIELD_GRAB();

    GdkFilterReturn aFilterReturn = GDK_FILTER_CONTINUE;

    XEvent *pEvent = (XEvent *)sys_event;
    GtkSalDisplay *pDisplay = (GtkSalDisplay *)data;

    if (pDisplay->GetDisplay() == pEvent->xany.display )
    {
        // let's see if one of our frames wants to swallow these events

        // get the child frame
        GtkSalFrame* pFrame = NULL;
        for( std::list< GtkSalFrame* >::const_iterator it = pDisplay->m_aFrames.begin();
                 it != pDisplay->m_aFrames.end() && ! pFrame; ++it )
        {
            if( (*it)->GetSystemData()->aWindow == pEvent->xany.window )
                pFrame = *it;
        }

        if( pFrame && !pFrame->Dispatch( pEvent ) )
            aFilterReturn = GDK_FILTER_REMOVE;
    }

    return aFilterReturn;
}

GdkCursor *GtkSalDisplay::getFromXPM( const char *pBitmap,
                                      const char *pMask,
                                      int nWidth, int nHeight,
                                      int nXHot, int nYHot )
{
    GdkScreen *pScreen = gdk_display_get_default_screen( m_pGdkDisplay );
    GdkDrawable *pDrawable = GDK_DRAWABLE( gdk_screen_get_root_window (pScreen) );
    GdkBitmap *pBitmapPix = gdk_bitmap_create_from_data
            ( pDrawable, pBitmap, nWidth, nHeight );
    GdkBitmap *pMaskPix = gdk_bitmap_create_from_data
            ( pDrawable, pMask, nWidth, nHeight );
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
        GdkCursor *pCursor;

        switch( ePointerStyle )
        {
            MAP_BUILTIN( POINTER_ARROW, GDK_LEFT_PTR );
            MAP_BUILTIN( POINTER_TEXT, GDK_XTERM );
            MAP_BUILTIN( POINTER_HELP, GDK_QUESTION_ARROW );
            MAP_BUILTIN( POINTER_CROSS, GDK_CROSSHAIR );

            MAP_BUILTIN( POINTER_NSIZE, GDK_SB_V_DOUBLE_ARROW );
            MAP_BUILTIN( POINTER_SSIZE, GDK_SB_V_DOUBLE_ARROW );
            MAP_BUILTIN( POINTER_WSIZE, GDK_SB_H_DOUBLE_ARROW );
            MAP_BUILTIN( POINTER_ESIZE, GDK_SB_H_DOUBLE_ARROW );

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

            MAP_BUILTIN( POINTER_REFHAND, GDK_HAND1 );
            MAP_BUILTIN( POINTER_HAND, GDK_HAND2 );
            MAP_BUILTIN( POINTER_PEN, GDK_PENCIL );

            MAKE_CURSOR( POINTER_NULL, null );
            MAKE_CURSOR( POINTER_WAIT, wait_ );
            MAKE_CURSOR( POINTER_NWSIZE, nwsesize_ );
            MAKE_CURSOR( POINTER_NESIZE, neswsize_ );
            MAKE_CURSOR( POINTER_SWSIZE, neswsize_ );
            MAKE_CURSOR( POINTER_SESIZE, nwsesize_ );
            MAKE_CURSOR( POINTER_HSPLIT, hsplit_ );
            MAKE_CURSOR( POINTER_VSPLIT, vsplit_ );
            MAKE_CURSOR( POINTER_MAGNIFY, magnify_ );
            MAKE_CURSOR( POINTER_FILL, fill_ );
            MAKE_CURSOR( POINTER_MOVE, move_ );
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
private:
    void updateTimeout (gint wait_ms);
public:
    GtkXLib();
    virtual ~GtkXLib();

    virtual void    Init();
    virtual void    Yield( BOOL );
    virtual void    Wakeup();
    virtual void    Insert( int fd, void* data,
                            YieldFunc   pending,
                            YieldFunc   queued,
                            YieldFunc   handle );
    virtual void    Remove( int fd );
};

GtkXLib::GtkXLib()
{
#if OSL_DEBUG_LEVEL > 1
    fprintf( stderr, "GtkXLib::GtkXLib()\n" );
#endif
    m_pGtkSalDisplay = NULL;
    m_pTimeout = NULL;
}

GtkXLib::~GtkXLib()
{
#if OSL_DEBUG_LEVEL > 1
    fprintf( stderr, "GtkXLib::~GtkXLib()\n" );
#endif
}

void GtkXLib::Init()
{
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
    for (int i=0; i<nParams; i++)
    {
        osl_getCommandArg(i, &aParam.pData );
        OString aBParam( OUStringToOString( aParam, aEnc ) );

        if( aParam.equalsAscii( "-display" ) || aParam.equalsAscii( "--display" ) )
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

    // init gtk/gdk
    gtk_init_check( &nParams, &pCmdLineAry );

    for (i = 0; i < nParams; i++ )
        g_free( pCmdLineAry[i] );
    delete pCmdLineAry;

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
    char *pPutEnvIsBroken = g_strdup_printf( "DISPLAY=%s",
                                             gdk_display_get_name( pGdkDisp ) );
    putenv( pPutEnvIsBroken );

    Display *pDisp = gdk_x11_display_get_xdisplay( pGdkDisp );
    XVisualInfo aVI;
    Colormap    aColMap;
    int         nScreen = DefaultScreen( pDisp );

    if( SalDisplay::BestVisual( pDisp, nScreen, aVI ) ) // DefaultVisual
        aColMap = DefaultColormap( pDisp, nScreen );
    else
        aColMap = XCreateColormap( pDisp,
                                   RootWindow( pDisp, nScreen ),
                                   aVI.visual,
                                   AllocNone );

    XSetIOErrorHandler    ( (XIOErrorHandler)SalData::XIOErrorHdl );
    XSetErrorHandler      ( (XErrorHandler)SalData::XErrorHdl );

    m_pGtkSalDisplay = new GtkSalDisplay( pGdkDisp, aVI.visual, aColMap );

    gdk_window_add_filter( NULL, GtkSalDisplay::filterGdkEvent, m_pGtkSalDisplay );

    sal_Bool bOldErrorSetting = GetIgnoreXErrors();
    SetIgnoreXErrors( True );
    SalI18N_KeyboardExtension *pKbdExtension = new SalI18N_KeyboardExtension( pDisp );
    XSync( pDisp, False );

    pKbdExtension->UseExtension( ! WasXError() );
    SetIgnoreXErrors( bOldErrorSetting );

    m_pGtkSalDisplay->SetKbdExtension( pKbdExtension );
}

static gboolean
dummy_fn (gpointer data)
{
    return TRUE;
}

void GtkXLib::updateTimeout (gint wait_ms)
{
    if (m_pTimeout)
    {
        g_source_destroy (m_pTimeout);
        g_source_unref (m_pTimeout);
    }

//  fprintf (stderr, "Add timeout of '%d'ms\n", wait_ms);
    m_pTimeout = g_timeout_source_new (wait_ms);
    g_source_set_can_recurse (m_pTimeout, TRUE);
    g_source_set_callback (m_pTimeout, dummy_fn, NULL, NULL);
    g_source_attach (m_pTimeout, g_main_context_default ());
}

void GtkXLib::Yield( BOOL bWait )
{
    // check for timeouts here if you want to make screenshots
    static char* p_prioritize_timer = getenv ("SAL_HIGHPRIORITY_REPAINT");
    if (p_prioritize_timer != NULL)
        CheckTimeout();

    // first, check for already queued events; dispatch one if exists
    if( m_pGtkSalDisplay->DispatchInternalEvent() )
        return;

    timeval thisTimeout = { 0 };
    gint wait_ms = 0;

    if (bWait) {
        if (Timeout_.tv_sec) {
            // determine remaining timeout.
            gettimeofday (&thisTimeout, 0);
            thisTimeout = Timeout_ - thisTimeout;
            wait_ms = (thisTimeout.tv_usec + 900)/1000 + thisTimeout.tv_sec * 1000;
            if (wait_ms <= 0)
                bWait = FALSE;
            else
                updateTimeout (wait_ms);
        }
    }

    {
        // release YieldMutex (and re-acquire at block end)
        YieldMutexReleaser aReleaser;
        if( !bWait )
            osl_yieldThread();
        g_main_context_iteration( NULL, bWait );
    }

    // dispatch eventual timeout
    CheckTimeout();
    // dispatch eventual user event
    m_pGtkSalDisplay->DispatchInternalEvent();
}

void GtkXLib::Wakeup()
{
    g_main_context_wakeup (NULL);
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
                     GSourceFunc callback,
                     gpointer    user_data)
{
    SalData *pSalData = GetSalData();
    SalWatch *watch = (SalWatch *) source;

    pSalData->pInstance_->GetYieldMutex()->acquire();

    watch->handle (watch->pollfd.fd, watch->user_data);

    pSalData->pInstance_->GetYieldMutex()->release();

    return TRUE;
}

static void
sal_source_finalize (GSource *source)
{
}

static GSourceFuncs sal_source_watch_funcs = {
    sal_source_prepare,
    sal_source_check,
    sal_source_dispatch,
    sal_source_finalize
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
              YieldFunc queued,
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
