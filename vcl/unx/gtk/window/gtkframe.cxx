/*************************************************************************
 *
 *  $RCSfile: gtkframe.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-23 10:05:32 $
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

#include <plugins/gtk/gtkframe.hxx>
#include <plugins/gtk/gtkdata.hxx>
#include <plugins/gtk/gtkinst.hxx>
#include <plugins/gtk/gtkgdi.hxx>
#include <pspgraphics.h>
#include <keycodes.hxx>
#include <wmadaptor.hxx>
#include <salbmp.h>
#include <floatwin.hxx>

#include <prex.h>
#include <X11/Xatom.h>
#include <postx.h>

#include <dlfcn.h>
#include <soicon.hxx>

#if OSL_DEBUG_LEVEL > 1
#include <cstdio>
#endif

int GtkSalFrame::m_nFloats = 0;

static USHORT GetModCode( guint state )
{
    USHORT nCode = 0;
    if( (state & GDK_SHIFT_MASK) )
        nCode |= KEY_SHIFT;
    if( (state & GDK_CONTROL_MASK) )
        nCode |= KEY_MOD1;
    if( (state & GDK_MOD1_MASK) )
    {
        nCode |= KEY_MOD2;
        if( (state & GDK_MOD1_MASK) )
            nCode |= KEY_CONTROLMOD;
    }
    if( (state & GDK_BUTTON1_MASK) )
        nCode |= MOUSE_LEFT;
    if( (state & GDK_BUTTON2_MASK) )
        nCode |= MOUSE_MIDDLE;
    if( (state & GDK_BUTTON3_MASK) )
        nCode |= MOUSE_RIGHT;
    return nCode;
}

static USHORT GetKeyCode( guint keyval )
{
    USHORT nCode = 0;
    if( keyval >= GDK_0 && keyval <= GDK_9 )
        nCode = KEY_0 + (keyval-GDK_0);
    else if( keyval >= GDK_A && keyval <= GDK_Z )
        nCode = KEY_A + (keyval-GDK_A );
    else if( keyval >= GDK_a && keyval <= GDK_z )
        nCode = KEY_A + (keyval-GDK_a );
    else if( keyval >= GDK_F1 && keyval <= GDK_F26 )
        nCode = KEY_F1 + (keyval-GDK_F1);

    {
        switch( keyval )
        {
            case GDK_KP_Down:
            case GDK_Down:          nCode = KEY_DOWN;       break;
            case GDK_KP_Up:
            case GDK_Up:            nCode = KEY_UP;         break;
            case GDK_KP_Left:
            case GDK_Left:          nCode = KEY_LEFT;       break;
            case GDK_KP_Right:
            case GDK_Right:         nCode = KEY_RIGHT;      break;
            case GDK_KP_Home:
            case GDK_Home:          nCode = KEY_HOME;       break;
            case GDK_KP_End:
            case GDK_End:           nCode = KEY_END;        break;
            case GDK_KP_Page_Up:
            case GDK_Page_Up:       nCode = KEY_PAGEUP;     break;
            case GDK_KP_Page_Down:
            case GDK_Page_Down:     nCode = KEY_PAGEDOWN;   break;
            case GDK_KP_Enter:
            case GDK_Return:        nCode = KEY_RETURN;     break;
            case GDK_Escape:        nCode = KEY_ESCAPE;     break;
            case GDK_ISO_Left_Tab:
            case GDK_KP_Tab:
            case GDK_Tab:           nCode = KEY_TAB;        break;
            case GDK_BackSpace:     nCode = KEY_BACKSPACE;  break;
            case GDK_KP_Space:
            case GDK_space:         nCode = KEY_SPACE;      break;
            case GDK_Insert:        nCode = KEY_INSERT;     break;
            case GDK_Delete:        nCode = KEY_DELETE;     break;
            case GDK_KP_Add:        nCode = KEY_ADD;        break;
            case GDK_KP_Subtract:   nCode = KEY_SUBTRACT;   break;
            case GDK_KP_Multiply:   nCode = KEY_MULTIPLY;   break;
            case GDK_KP_Divide:     nCode = KEY_DIVIDE;     break;
            case GDK_period:
            case GDK_decimalpoint:  nCode = KEY_POINT;      break;
            case GDK_comma:         nCode = KEY_COMMA;      break;
            case GDK_less:          nCode = KEY_LESS;       break;
            case GDK_greater:       nCode = KEY_GREATER;    break;
            case GDK_equal:         nCode = KEY_EQUAL;      break;
            case GDK_Find:          nCode = KEY_FIND;       break;
            case GDK_Menu:          nCode = KEY_MENU;       break;
            case GDK_Help:          nCode = KEY_HELP;       break;
            case GDK_Undo:          nCode = KEY_UNDO;       break;
        }
    }
    return nCode;
}

static USHORT GetKeyCodeFromUnicode( sal_Unicode cCode )
{
    USHORT nRet = 0;

    if( cCode >= '0' && cCode <= '9' )
        nRet = KEY_0 + (cCode - '0');
    else if( cCode >= 'a' && cCode <= 'z' )
        nRet = KEY_A + (cCode -'a' );
    else if( cCode >= 'A' && cCode <= 'Z' )
        nRet = KEY_A + (cCode -'A' );
    else if( cCode == ' ' )
        nRet = KEY_SPACE;
    else if( cCode == '+' )
        nRet = KEY_ADD;
    else if( cCode == '-' )
        nRet = KEY_SUBTRACT;
    else if( cCode == '*' )
        nRet = KEY_MULTIPLY;
    else if( cCode == '/' )
        nRet = KEY_DIVIDE;
    else if( cCode == '.' )
        nRet = KEY_POINT;
    else if( cCode == ',' )
        nRet = KEY_COMMA;
    else if( cCode == '<' )
        nRet = KEY_LESS;
    else if( cCode == '>' )
        nRet = KEY_GREATER;
    else if( cCode == '=' )
        nRet = KEY_EQUAL;

    return nRet;
}

GtkSalFrame::GraphicsHolder::~GraphicsHolder()
{
    delete pGraphics;
}

GtkSalFrame::GtkSalFrame( SalFrame* pParent, ULONG nStyle )
{
    getDisplay()->registerFrame( this );
    Init( pParent, nStyle );
}

GtkSalFrame::GtkSalFrame( SystemParentData* pSysData )
{
    getDisplay()->registerFrame( this );
    getDisplay()->setHaveSystemChildFrame();
    Init( pSysData );
}

GtkSalFrame::~GtkSalFrame()
{
    getDisplay()->deregisterFrame( this );
    if( m_pIMContext )
    {
        gtk_im_context_reset( m_pIMContext );
        gtk_im_context_set_client_window( m_pIMContext, NULL );
        g_object_unref( m_pIMContext );
    }
    if( m_pFixedContainer )
        gtk_widget_destroy( GTK_WIDGET(m_pFixedContainer) );
    if( m_pWindow )
        gtk_widget_destroy( GTK_WIDGET(m_pWindow) );
    if( m_pForeignParent )
        gdk_window_destroy( m_pForeignParent );
    if( m_pForeignTopLevel )
        gdk_window_destroy( m_pForeignTopLevel );
}

void GtkSalFrame::InitCommon()
{
    // connect signals
    g_signal_connect( G_OBJECT(m_pWindow), "style-set", G_CALLBACK(signalStyleSet), this );
    g_signal_connect( G_OBJECT(m_pWindow), "button-press-event", G_CALLBACK(signalButton), this );
    g_signal_connect( G_OBJECT(m_pWindow), "button-release-event", G_CALLBACK(signalButton), this );
    g_signal_connect( G_OBJECT(m_pWindow), "expose-event", G_CALLBACK(signalExpose), this );
    g_signal_connect( G_OBJECT(m_pWindow), "focus-in-event", G_CALLBACK(signalFocus), this );
    g_signal_connect( G_OBJECT(m_pWindow), "focus-out-event", G_CALLBACK(signalFocus), this );
    g_signal_connect( G_OBJECT(m_pWindow), "map-event", G_CALLBACK(signalMap), this );
    g_signal_connect( G_OBJECT(m_pWindow), "unmap-event", G_CALLBACK(signalUnmap), this );
    g_signal_connect( G_OBJECT(m_pWindow), "configure-event", G_CALLBACK(signalConfigure), this );
    g_signal_connect( G_OBJECT(m_pWindow), "motion-notify-event", G_CALLBACK(signalMotion), this );
    g_signal_connect( G_OBJECT(m_pWindow), "key-press-event", G_CALLBACK(signalKey), this );
    g_signal_connect( G_OBJECT(m_pWindow), "key-release-event", G_CALLBACK(signalKey), this );
    g_signal_connect( G_OBJECT(m_pWindow), "delete-event", G_CALLBACK(signalDelete), this );
    g_signal_connect( G_OBJECT(m_pWindow), "window-state-event", G_CALLBACK(signalState), this );
    g_signal_connect( G_OBJECT(m_pWindow), "scroll-event", G_CALLBACK(signalScroll), this );
    g_signal_connect( G_OBJECT(m_pWindow), "leave-notify-event", G_CALLBACK(signalCrossing), this );
    g_signal_connect( G_OBJECT(m_pWindow), "enter-notify-event", G_CALLBACK(signalCrossing), this );
    g_signal_connect( G_OBJECT(m_pWindow), "visibility-notify-event", G_CALLBACK(signalVisibility), this );

    // init members
    m_pCurrentCursor    = NULL;
    m_nKeyModifiers     = 0;
    m_bSingleAltPress   = false;
    m_bResizeable       = true;
    m_bDefaultPos       = true;
    m_bDefaultSize      = ( (m_nStyle & SAL_FRAME_STYLE_SIZEABLE) && ! m_pParent );
    m_nState            = GDK_WINDOW_STATE_WITHDRAWN;
    m_nVisibility       = GDK_VISIBILITY_FULLY_OBSCURED;
    m_bSendModChangeOnRelease = false;
    m_pIMContext        = NULL;
    m_bWasPreedit       = false;
    // delete graphics if InitCommon is called not from destructor
    for( int i = 0; i < nMaxGraphics; i++ )
    {
        m_aGraphics[i].bInUse = false;
        delete m_aGraphics[i].pGraphics;
        m_aGraphics[i].pGraphics = NULL;
    }

    gtk_widget_set_app_paintable( GTK_WIDGET(m_pWindow), TRUE );
    gtk_widget_set_double_buffered( GTK_WIDGET(m_pWindow), FALSE );
    gtk_widget_set_redraw_on_allocate( GTK_WIDGET(m_pWindow), FALSE );

    // realize the window, we need an XWindow id
    gtk_widget_realize( GTK_WIDGET(m_pWindow) );

    // add the fixed container child,
    // fixed is needed since we have to position plugin windows
    m_pFixedContainer = GTK_FIXED(gtk_fixed_new());
    gtk_container_add( GTK_CONTAINER(m_pWindow), GTK_WIDGET(m_pFixedContainer) );
    gtk_widget_show( GTK_WIDGET(m_pFixedContainer) );

    //system data
    SalDisplay* pDisp = GetSalData()->GetDisplay();
    m_aSystemData.pDisplay      = pDisp->GetDisplay();
    m_aSystemData.aWindow       = GDK_WINDOW_XWINDOW(GTK_WIDGET(m_pWindow)->window);
    m_aSystemData.pSalFrame     = this;
    m_aSystemData.pWidget       = GTK_WIDGET(m_pWindow);
    m_aSystemData.pVisual       = pDisp->GetVisual()->GetVisual();
    m_aSystemData.nDepth        = pDisp->GetVisual()->GetDepth();
    m_aSystemData.aColormap     = pDisp->GetColormap().GetXColormap();
    m_aSystemData.pAppContext   = NULL;
    m_aSystemData.aShellWindow  = m_aSystemData.aWindow;
    m_aSystemData.pShellWidget  = m_aSystemData.pWidget;

    gtk_widget_add_events( GTK_WIDGET(m_pWindow),
                           GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK |
                           GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK |
                           GDK_VISIBILITY_NOTIFY_MASK
                           );

    // fake an initial geometry, gets updated via configure event or SetPosSize
    Size aDefSize = calcDefaultSize();
    maGeometry.nX                   = -1;
    maGeometry.nY                   = -1;
    maGeometry.nWidth               = aDefSize.Width();
    maGeometry.nHeight              = aDefSize.Height();
    if( m_pParent )
    {
        // approximation
        maGeometry.nTopDecoration       = m_pParent->maGeometry.nTopDecoration;
        maGeometry.nBottomDecoration    = m_pParent->maGeometry.nBottomDecoration;
        maGeometry.nLeftDecoration      = m_pParent->maGeometry.nLeftDecoration;
        maGeometry.nRightDecoration     = m_pParent->maGeometry.nRightDecoration;
    }
    else
    {
        maGeometry.nTopDecoration       = 0;
        maGeometry.nBottomDecoration    = 0;
        maGeometry.nLeftDecoration      = 0;
        maGeometry.nRightDecoration     = 0;
    }
}

void GtkSalFrame::Init( SalFrame* pParent, ULONG nStyle )
{
    if( nStyle & SAL_FRAME_STYLE_DEFAULT ) // ensure default style
    {
        nStyle |= SAL_FRAME_STYLE_MOVEABLE | SAL_FRAME_STYLE_SIZEABLE | SAL_FRAME_STYLE_CLOSEABLE;
        nStyle &= ~SAL_FRAME_STYLE_FLOAT;
    }

    m_pWindow = GTK_WINDOW( gtk_widget_new( GTK_TYPE_WINDOW, "type", (nStyle & SAL_FRAME_STYLE_FLOAT) ? GTK_WINDOW_POPUP : GTK_WINDOW_TOPLEVEL, "visible", FALSE, NULL ) );
    m_pParent = static_cast<GtkSalFrame*>(pParent);
    m_pForeignParent = NULL;
    m_pForeignTopLevel = NULL;
    m_nStyle = nStyle;

    if( m_pParent && m_pParent->m_pWindow && ! (m_pParent->m_nStyle & SAL_FRAME_STYLE_CHILD) )
        gtk_window_set_screen( m_pWindow, gtk_window_get_screen( m_pParent->m_pWindow ) );

    InitCommon();

    if( ! (nStyle & (SAL_FRAME_STYLE_FLOAT|SAL_FRAME_STYLE_CHILD)) )
    {
        m_bResizeable = (nStyle & SAL_FRAME_STYLE_SIZEABLE) != 0;
        gtk_window_set_resizable( m_pWindow, m_bResizeable ? TRUE : FALSE );
        gtk_window_set_gravity( m_pWindow, GDK_GRAVITY_STATIC );
        if( (nStyle & SAL_FRAME_STYLE_INTRO) )
            gtk_window_set_type_hint( m_pWindow, GDK_WINDOW_TYPE_HINT_SPLASHSCREEN );
        else if( (nStyle & SAL_FRAME_STYLE_TOOLWINDOW ) )
            gtk_window_set_type_hint( m_pWindow, GDK_WINDOW_TYPE_HINT_UTILITY );
        if( ! (nStyle & (SAL_FRAME_STYLE_MOVEABLE | SAL_FRAME_STYLE_SIZEABLE | SAL_FRAME_STYLE_CLOSEABLE ) ) )
            gtk_window_set_decorated( m_pWindow, FALSE );
        if( m_pParent && ! (m_pParent->m_nStyle & SAL_FRAME_STYLE_CHILD) )
            gtk_window_set_transient_for( m_pWindow, m_pParent->m_pWindow );
    }
}

GdkNativeWindow GtkSalFrame::findTopLevelSystemWindow( GdkNativeWindow aWindow )
{
    XLIB_Window aRoot, aParent;
    XLIB_Window* pChildren;
    unsigned int nChildren;
    bool bBreak = false;
    do
    {
        XQueryTree( getDisplay()->GetDisplay(), aWindow,
                    &aRoot, &aParent, &pChildren, &nChildren );
        XFree( pChildren );
        if( aParent != aRoot )
            aWindow = aParent;
        int nCount = 0;
        Atom* pProps = XListProperties( getDisplay()->GetDisplay(),
                                        aWindow,
                                        &nCount );
        for( int i = 0; i < nCount && ! bBreak; ++i )
            bBreak = (pProps[i] == XA_WM_HINTS);
        if( pProps )
            XFree( pProps );
    } while( aParent != aRoot && ! bBreak );

    return aWindow;
}

void GtkSalFrame::Init( SystemParentData* pSysData )
{
    m_pParent = NULL;
    m_pForeignParent = gdk_window_foreign_new_for_display( getGdkDisplay(), (GdkNativeWindow)pSysData->aWindow );
    gdk_window_set_events( m_pForeignParent, GDK_STRUCTURE_MASK );
    m_pForeignTopLevel = gdk_window_foreign_new_for_display( getGdkDisplay(), findTopLevelSystemWindow( (GdkNativeWindow)pSysData->aWindow ) );
    gdk_window_set_events( m_pForeignTopLevel, GDK_STRUCTURE_MASK );
    m_pWindow = GTK_WINDOW(gtk_window_new( GTK_WINDOW_POPUP ));
    m_nStyle = SAL_FRAME_STYLE_CHILD;
    InitCommon();
    XReparentWindow( getDisplay()->GetDisplay(),
                     GDK_WINDOW_XWINDOW(GTK_WIDGET(m_pWindow)->window),
                     (XLIB_Window)pSysData->aWindow,
                     0, 0 );
}

SalGraphics* GtkSalFrame::GetGraphics()
{
    if( m_pWindow )
    {
        for( int i = 0; i < nMaxGraphics; i++ )
        {
            if( ! m_aGraphics[i].bInUse )
            {
                m_aGraphics[i].bInUse = true;
                if( ! m_aGraphics[i].pGraphics )
                {
                    m_aGraphics[i].pGraphics = new GtkSalGraphics(GTK_WIDGET( m_pWindow ));
                    m_aGraphics[i].pGraphics->Init( this, GDK_WINDOW_XWINDOW(GTK_WIDGET(m_pWindow)->window) );
                }
                return m_aGraphics[i].pGraphics;
            }
        }
    }

    return NULL;
}

void GtkSalFrame::ReleaseGraphics( SalGraphics* pGraphics )
{
    for( int i = 0; i < nMaxGraphics; i++ )
    {
        if( m_aGraphics[i].pGraphics == pGraphics )
        {
            m_aGraphics[i].bInUse = false;
            break;
        }
    }
}

BOOL GtkSalFrame::PostEvent( void* pData )
{
    getDisplay()->SendInternalEvent( this, pData );
    return TRUE;
}

void GtkSalFrame::SetTitle( const String& rTitle )
{
    if( m_pWindow && !(m_nStyle & SAL_FRAME_STYLE_CHILD))
        gtk_window_set_title( m_pWindow, rtl::OUStringToOString( rTitle, RTL_TEXTENCODING_UTF8 ).getStr() );
}

void GtkSalFrame::SetIcon( USHORT nIcon )
{
    if( m_nStyle & SAL_FRAME_STYLE_CHILD )
        return;

    GList *pPixbufs = NULL;

    // pl: evil ? don't be scared so easily :-)
    { // Begin this unutterably evil; we need to pass a nice handle down
        VCL_CUSTOM_ICON_FN *pCustomIcon = 0;
        char *pSymbol = g_strdup_printf ("%s%d", VCL_CUSTOM_ICON_BASE, nIcon );
        void *pAppHdl = dlopen( NULL, RTLD_LAZY );
        if ( ( pCustomIcon = ( VCL_CUSTOM_ICON_FN* ) dlsym( pAppHdl, pSymbol ) ) )
        {
            char **pIcons[4] = { NULL, NULL, NULL, NULL };
            pCustomIcon( pIcons[0], pIcons[1], pIcons[2], pIcons[3] );
            for( int i = 0; i < 4; i++)
            {
                if( pIcons[i] )
                {
                    GdkPixbuf *pPixbuf = gdk_pixbuf_new_from_xpm_data( (const char **) pIcons[i] );
                    pPixbufs = g_list_prepend( pPixbufs, pPixbuf );
                }
            }
        }
        g_free( pSymbol );
        dlclose( pAppHdl );
    } // End evilness

    gtk_window_set_icon_list( m_pWindow, pPixbufs );

    g_list_foreach( pPixbufs, (GFunc) g_object_unref, NULL );
    g_list_free( pPixbufs );
}

void GtkSalFrame::SetMenu( SalMenu* pSalMenu )
{
}

void GtkSalFrame::DrawMenuBar()
{
}

void GtkSalFrame::Center()
{
    long nX, nY;

    if( m_pParent )
    {
        nX = ((long)m_pParent->maGeometry.nWidth - (long)maGeometry.nWidth)/2;
        nY = ((long)m_pParent->maGeometry.nHeight - (long)maGeometry.nHeight)/2;

    }
    else
    {
        long    nScreenWidth, nScreenHeight;
        long    nScreenX = 0, nScreenY = 0;

        nScreenWidth        = GetSalData()->GetDisplay()->GetScreenSize().Width();
        nScreenHeight       = GetSalData()->GetDisplay()->GetScreenSize().Height();
        if( GetSalData()->GetDisplay()->IsXinerama() )
        {
            // get xinerama screen we are on
            // if there is a parent, use its center for screen determination
            // else use the pointer
            GdkScreen* pScreen;
            gint x, y;
            GdkModifierType aMask;
            gdk_display_get_pointer( getGdkDisplay(), &pScreen, &x, &y, &aMask );

            const std::vector< Rectangle >& rScreens = GetSalData()->GetDisplay()->GetXineramaScreens();
            for( unsigned int i = 0; i < rScreens.size(); i++ )
                if( rScreens[i].IsInside( Point( x, y ) ) )
                {
                    nScreenX            = rScreens[i].Left();
                    nScreenY            = rScreens[i].Top();
                    nScreenWidth        = rScreens[i].GetWidth();
                    nScreenHeight       = rScreens[i].GetHeight();
                    break;
                }
        }
        nX = nScreenX + (nScreenWidth - (long)maGeometry.nWidth)/2;
        nY = nScreenY + (nScreenHeight - (long)maGeometry.nHeight)/2;
    }
    SetPosSize( nX, nY, 0, 0, SAL_FRAME_POSSIZE_X | SAL_FRAME_POSSIZE_Y );
}

Size GtkSalFrame::calcDefaultSize()
{
    const Size& rScreenSize( getDisplay()->GetScreenSize() );
    long w = rScreenSize.Width();
    long h = rScreenSize.Height();

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

    return Size( w, h );
}

void GtkSalFrame::SetDefaultSize()
{
    Size aDefSize = calcDefaultSize();

    SetPosSize( 0, 0, aDefSize.Width(), aDefSize.Height(),
                SAL_FRAME_POSSIZE_WIDTH | SAL_FRAME_POSSIZE_HEIGHT );

    if( m_nStyle & SAL_FRAME_STYLE_DEFAULT )
        gtk_window_maximize( m_pWindow );
}

void GtkSalFrame::Show( BOOL bVisible, BOOL bNoActivate )
{
    if( m_pWindow )
    {
        if( bVisible )
        {
            if( m_bDefaultPos )
                Center();
            if( m_bDefaultSize )
                SetDefaultSize();

            gtk_widget_show( GTK_WIDGET(m_pWindow) );
            if( isFloatGrabWindow() )
            {
                m_nFloats++;
                if( ! getDisplay()->GetCaptureFrame() && m_nFloats == 1 )
                    grabPointer( TRUE, TRUE );
            }
        }
        else
        {
            if( isFloatGrabWindow() )
            {
                m_nFloats--;
                if( ! getDisplay()->GetCaptureFrame() && m_nFloats == 0)
                    grabPointer( FALSE );
            }
            gtk_widget_hide( GTK_WIDGET(m_pWindow) );
            if( m_pIMContext )
            {
                gtk_im_context_focus_out( m_pIMContext );
                gtk_im_context_reset( m_pIMContext );
            }
        }
    }
}

void GtkSalFrame::Enable( BOOL bEnable )
{
    // Not implemented by X11SalFrame either
}

void GtkSalFrame::SetMinClientSize( long nWidth, long nHeight )
{
    if( m_pWindow )
        gtk_widget_set_size_request( GTK_WIDGET(m_pWindow), nWidth, nHeight );
}

void GtkSalFrame::SetPosSize( long nX, long nY, long nWidth, long nHeight, USHORT nFlags )
{
    if( !m_pWindow || (m_nStyle & SAL_FRAME_STYLE_CHILD) )
        return;

    if( (nFlags & ( SAL_FRAME_POSSIZE_WIDTH | SAL_FRAME_POSSIZE_HEIGHT )) &&
        (nWidth > 0 && nHeight > 0 ) // sometimes stupid things happen
            )
    {
#if OSL_DEBUG_LEVEL > 1
        if( nWidth < 2 || nWidth > 2000 || nHeight < 2 || nHeight > 2000 )
        {
            fprintf( stderr, "Discarding bad size: %ld, %ld\n", nWidth, nHeight );
            return;
        }
#endif
        m_bDefaultSize = false;
        gtk_window_resize( m_pWindow, nWidth, nHeight );
        if( ! m_bResizeable )
        {
            GdkGeometry aGeo;
            aGeo.min_width = aGeo.max_width = nWidth;
            aGeo.min_height = aGeo.max_height = nHeight;
            gtk_window_set_geometry_hints( m_pWindow,
                                           NULL,
                                           &aGeo,
                                           (GdkWindowHints)( GDK_HINT_MIN_SIZE | GDK_HINT_MAX_SIZE ) );
        }

        maGeometry.nWidth   = nWidth;
        maGeometry.nHeight  = nHeight;
    }
    else if( m_bDefaultSize )
        SetDefaultSize();

    m_bDefaultSize = false;

    if( nFlags & ( SAL_FRAME_POSSIZE_X | SAL_FRAME_POSSIZE_Y ) )
    {
#if OSL_DEBUG_LEVEL > 1
        if( std::abs( nX ) > 2000 || std::abs( nY ) > 2000 )
        {
            fprintf( stderr, "Discarding bad pos: %ld, %ld\n", nX, nY );
            return;
        }
#endif
        if( m_pParent )
        {
            nX += m_pParent->maGeometry.nX;
            nY += m_pParent->maGeometry.nY;
        }

        // adjust position to avoid off screen windows
        Size aScreenSize = GetSalData()->GetDisplay()->GetScreenSize();
        if( nX < (long)maGeometry.nLeftDecoration )
            nX = maGeometry.nLeftDecoration;
        if( nY < (long)maGeometry.nTopDecoration )
            nY = maGeometry.nTopDecoration;
        if( (nX + (long)maGeometry.nWidth + (long)maGeometry.nRightDecoration) > (long)aScreenSize.Width() )
            nX = aScreenSize.Width() - maGeometry.nWidth - maGeometry.nRightDecoration;
        if( (nY + (long)maGeometry.nHeight + (long)maGeometry.nBottomDecoration) > (long)aScreenSize.Height() )
            nY = aScreenSize.Height() - maGeometry.nHeight - maGeometry.nBottomDecoration;

        m_bDefaultPos = false;
        gtk_window_move( m_pWindow, nX, nY );
        maGeometry.nX = nX;
        maGeometry.nY = nY;
    }
    else if( m_bDefaultPos )
        Center();

    m_bDefaultPos = false;
}

void GtkSalFrame::GetClientSize( long& rWidth, long& rHeight )
{
    if( GTK_WIDGET_MAPPED( GTK_WIDGET(m_pWindow) ) )
    {
        rWidth = maGeometry.nWidth;
        rHeight = maGeometry.nHeight;
    }
    else
        rWidth = rHeight = 0;
}

void GtkSalFrame::GetWorkArea( Rectangle& rRect )
{
    rRect = GetSalData()->GetDisplay()->getWMAdaptor()->getWorkArea( 0 );
}

SalFrame* GtkSalFrame::GetParent() const
{
    return m_pParent;
}

void GtkSalFrame::SetWindowState( const SalFrameState* pState )
{
    if( ! m_pWindow || ! pState || (m_nStyle & SAL_FRAME_STYLE_CHILD) )
        return;

    if( pState->mnMask & (SAL_FRAMESTATE_MASK_X | SAL_FRAMESTATE_MASK_Y | SAL_FRAMESTATE_MASK_WIDTH | SAL_FRAMESTATE_MASK_HEIGHT ) )
    {
        USHORT nPosSizeFlags = 0;
        long nX         = pState->mnX - (m_pParent ? m_pParent->maGeometry.nX : 0);
        long nY         = pState->mnY - (m_pParent ? m_pParent->maGeometry.nY : 0);
        long nWidth     = pState->mnWidth;
        long nHeight    = pState->mnHeight;
        if( pState->mnMask & SAL_FRAMESTATE_MASK_X )
            nPosSizeFlags |= SAL_FRAME_POSSIZE_X;
        else
            nX = maGeometry.nX - (m_pParent ? m_pParent->maGeometry.nX : 0);
        if( pState->mnMask & SAL_FRAMESTATE_MASK_Y )
            nPosSizeFlags |= SAL_FRAME_POSSIZE_Y;
        else
            nY = maGeometry.nY - (m_pParent ? m_pParent->maGeometry.nY : 0);
        if( pState->mnMask & SAL_FRAMESTATE_MASK_WIDTH )
            nPosSizeFlags |= SAL_FRAME_POSSIZE_WIDTH;
        else
            nWidth = maGeometry.nWidth;
        if( pState->mnMask & SAL_FRAMESTATE_MASK_HEIGHT )
            nPosSizeFlags |= SAL_FRAME_POSSIZE_HEIGHT;
        else
            nHeight = maGeometry.nHeight;
        SetPosSize( nX, nY, pState->mnWidth, pState->mnHeight, nPosSizeFlags );
    }
    if( pState->mnMask & SAL_FRAMESTATE_MASK_STATE )
    {
        if( pState->mnState & SAL_FRAMESTATE_MAXIMIZED )
            gtk_window_maximize( m_pWindow );
        if( pState->mnState & SAL_FRAMESTATE_MINIMIZED )
            gtk_window_iconify( m_pWindow );
    }
}

BOOL GtkSalFrame::GetWindowState( SalFrameState* pState )
{
    pState->mnState = SAL_FRAMESTATE_NORMAL;
    if( (m_nState & GDK_WINDOW_STATE_ICONIFIED) )
        pState->mnState |= SAL_FRAMESTATE_MINIMIZED;
    if( m_nState & GDK_WINDOW_STATE_MAXIMIZED )
        pState->mnState |= SAL_FRAMESTATE_MAXIMIZED;
    // rollup ? gtk 2.2 does not seem to support the shaded state

    pState->mnX         = maGeometry.nX;
    pState->mnY         = maGeometry.nY;
    pState->mnWidth     = maGeometry.nWidth;
    pState->mnHeight    = maGeometry.nHeight;
    pState->mnMask      =
        SAL_FRAMESTATE_MASK_X           |
        SAL_FRAMESTATE_MASK_Y           |
        SAL_FRAMESTATE_MASK_WIDTH       |
        SAL_FRAMESTATE_MASK_HEIGHT      |
        SAL_FRAMESTATE_MASK_STATE;

#if OSL_DEBUG_LEVEL > 1
    if( std::abs( maGeometry.nX ) > 2000 || std::abs( maGeometry.nY ) > 2000 )
    {
        fprintf( stderr, "bad pos in GetWindowState: %d, %d\n", maGeometry.nX, maGeometry.nY );
        abort();
    }
    if( maGeometry.nWidth < 2 || maGeometry.nWidth > 2000 || maGeometry.nHeight < 2 || maGeometry.nHeight > 2000 )
    {
        fprintf( stderr, "bad size in GetWindowState: %d, %d\n", maGeometry.nWidth, maGeometry.nHeight );
        abort();
    }
#endif

    return TRUE;
}

void GtkSalFrame::ShowFullScreen( BOOL bFullScreen )
{
    if( m_pWindow && ! (m_nStyle & SAL_FRAME_STYLE_CHILD) )
    {
        if( bFullScreen )
        {
            if( ! m_bResizeable )
                gtk_window_set_resizable( m_pWindow, TRUE );
            gtk_window_fullscreen( m_pWindow );
        }
        else
        {
            gtk_window_unfullscreen( m_pWindow );
            if( ! m_bResizeable )
                gtk_window_set_resizable( m_pWindow, FALSE );
        }
    }
}

/* definitions from xautolock.c (pl15) */
#define XAUTOLOCK_DISABLE 1
#define XAUTOLOCK_ENABLE  2

void GtkSalFrame::setAutoLock( bool bLock )
{
    GdkScreen  *pScreen = gtk_window_get_screen( m_pWindow );
    GdkDisplay *pDisplay = gdk_screen_get_display( pScreen );
    GdkWindow  *pRootWin = gdk_screen_get_root_window( pScreen );

    Atom nAtom = XInternAtom( GDK_DISPLAY_XDISPLAY( pDisplay ),
                              "XAUTOLOCK_MESSAGE", False );

    int nMessage = bLock ? XAUTOLOCK_ENABLE : XAUTOLOCK_DISABLE;

    XChangeProperty( GDK_DISPLAY_XDISPLAY( pDisplay ),
                     GDK_WINDOW_XID( pRootWin ),
                     nAtom, XA_INTEGER,
                     8, PropModeReplace,
                     (unsigned char*)&nMessage,
                     sizeof( nMessage ) );
}

void GtkSalFrame::StartPresentation( BOOL bStart )
{
    Display *pDisplay = GDK_DISPLAY_XDISPLAY( getGdkDisplay() );

    setAutoLock( !bStart );

    int nTimeout, nInterval, bPreferBlanking, bAllowExposures;

    XGetScreenSaver( pDisplay, &nTimeout, &nInterval,
                     &bPreferBlanking, &bAllowExposures );
    if( bStart )
    {
        if ( nTimeout )
        {
            m_nSavedScreenSaverTimeout = nTimeout;
            XResetScreenSaver( pDisplay );
            XSetScreenSaver( pDisplay, 0, nInterval,
                             bPreferBlanking, bAllowExposures );
        }
    }
    else
    {
        if( m_nSavedScreenSaverTimeout )
            XSetScreenSaver( pDisplay, m_nSavedScreenSaverTimeout,
                             nInterval, bPreferBlanking,
                             bAllowExposures );
        m_nSavedScreenSaverTimeout = 0;
    }
}

void GtkSalFrame::SetAlwaysOnTop( BOOL bOnTop )
{
}

void GtkSalFrame::ToTop( USHORT nFlags )
{
    if( m_pWindow )
    {
        if( GTK_WIDGET_MAPPED( GTK_WIDGET(m_pWindow ) ) )
        {
            if( ! SAL_FRAME_TOTOP_GRABFOCUS_ONLY )
                gtk_window_present( m_pWindow );
            else
                gdk_window_focus( GTK_WIDGET(m_pWindow)->window, GDK_CURRENT_TIME );
        }
        else
        {
            if( nFlags & SAL_FRAME_TOTOP_RESTOREWHENMIN )
                gtk_window_present( m_pWindow );
        }
    }
}

void GtkSalFrame::SetPointer( PointerStyle ePointerStyle )
{
    GdkCursor *pCursor = getDisplay()->getCursor( ePointerStyle );
    gdk_window_set_cursor( GTK_WIDGET(m_pWindow)->window, pCursor );
    m_pCurrentCursor = pCursor;
}

void GtkSalFrame::grabPointer( BOOL bGrab, BOOL bOwnerEvents )
{
    int nMask = ( GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK );
    GdkGrabStatus nStatus;

    if( bGrab )
        nStatus = gdk_pointer_grab
                ( GTK_WIDGET(m_pWindow)->window, bOwnerEvents,
                  (GdkEventMask) nMask, NULL, m_pCurrentCursor,
                  GDK_CURRENT_TIME );
    else
    {
        // Two GdkDisplays may be open
        gdk_display_pointer_ungrab( getGdkDisplay(), GDK_CURRENT_TIME);
    }
}

void GtkSalFrame::CaptureMouse( BOOL bCapture )
{
    getDisplay()->CaptureMouse( bCapture ? this : NULL );
}

void GtkSalFrame::SetPointerPos( long nX, long nY )
{
    GdkScreen *pScreen = gtk_window_get_screen( m_pWindow );
    GdkDisplay *pDisplay = gdk_screen_get_display( pScreen );

    /* #87921# when the application tries to center the mouse in the dialog the
     * window isn't mapped already. So use coordinates relative to the root window.
     */
    unsigned int nWindowLeft = maGeometry.nX + nX;
    unsigned int nWindowTop  = maGeometry.nY + nY;

    XWarpPointer( GDK_DISPLAY_XDISPLAY (pDisplay), None,
                  GDK_WINDOW_XID (gdk_screen_get_root_window( pScreen ) ),
                  0, 0, 0, 0, nWindowLeft, nWindowTop);
}

void GtkSalFrame::Flush()
{
#ifdef HAVE_A_RECENT_GTK
    gdk_display_flush( getGdkDisplay() );
#else
    XFlush (GDK_DISPLAY_XDISPLAY (getGdkDisplay()));
#endif
}

void GtkSalFrame::Sync()
{
    gdk_display_sync( getGdkDisplay() );
}

String GtkSalFrame::GetSymbolKeyName( const String&, USHORT nKeyCode )
{
  return getDisplay()->GetKeyName( nKeyCode );
}

String GtkSalFrame::GetKeyName( USHORT nKeyCode )
{
    return getDisplay()->GetKeyName( nKeyCode );
}

GdkDisplay *GtkSalFrame::getGdkDisplay()
{
    return static_cast<GtkSalDisplay*>(GetSalData()->GetDisplay())->GetGdkDisplay();
}

GtkSalDisplay *GtkSalFrame::getDisplay()
{
    return static_cast<GtkSalDisplay*>(GetSalData()->GetDisplay());
}

SalFrame::SalPointerState GtkSalFrame::GetPointerState()
{
    SalPointerState aState;
    GdkScreen* pScreen;
    gint x, y;
    GdkModifierType aMask;
    gdk_display_get_pointer( getGdkDisplay(), &pScreen, &x, &y, &aMask );
    aState.maPos = Point( x - maGeometry.nX, y - maGeometry.nY );
    aState.mnState = GetModCode( aMask );
    return aState;
}

void GtkSalFrame::SetInputContext( SalInputContext* pContext )
{
    if( ! pContext )
        return;

    if( ! (pContext->mnOptions & SAL_INPUTCONTEXT_TEXT) )
    {
        if( m_pIMContext )
            gtk_im_context_focus_out( m_pIMContext );
        return;
    }

    // create a new im context
    if( ! m_pIMContext )
    {
        m_pIMContext = gtk_im_multicontext_new ();
        g_signal_connect( m_pIMContext, "commit",
                          G_CALLBACK (signalIMCommit), this );
        g_signal_connect( m_pIMContext, "preedit_changed",
                          G_CALLBACK (signalIMPreeditChanged), this );
        g_signal_connect( m_pIMContext, "retrieve_surrounding",
                          G_CALLBACK (signalIMRetrieveSurrounding), this );
        g_signal_connect( m_pIMContext, "delete_surrounding",
                          G_CALLBACK (signalIMDeleteSurrounding), this );

        gtk_im_context_set_client_window( m_pIMContext, GTK_WIDGET(m_pWindow)->window );
    }
}
void GtkSalFrame::EndExtTextInput( USHORT nFlags )
{
    if( m_pIMContext )
        gtk_im_context_reset( m_pIMContext );
    m_bWasPreedit = false;
}

LanguageType GtkSalFrame::GetInputLanguage()
{
    return LANGUAGE_DONTKNOW;
}

SalBitmap* GtkSalFrame::SnapShot()
{
    if( !m_pWindow )
        return NULL;

    X11SalBitmap *pBmp = new X11SalBitmap;
    GdkWindow *pWin = GTK_WIDGET(m_pWindow)->window;
    if( pBmp->SnapShot( GDK_DISPLAY_XDISPLAY( getGdkDisplay() ),
                        GDK_WINDOW_XID( pWin ) ) )
        return pBmp;
    else
        delete pBmp;

    return NULL;
}

static inline Color getColor( const GdkColor& rCol )
{
    return Color( rCol.red >> 8, rCol.green >> 8, rCol.blue >> 8 );
}

#if OSL_DEBUG_LEVEL > 1

void printColor( const char* name, const GdkColor& rCol )
{
    fprintf( stderr, "   %s = 0x%2x 0x%2x 0x%2x\n",
             name,
             rCol.red >> 8, rCol.green >> 8, rCol.blue >> 8 );
}

void printStyleColors( GtkStyle* pStyle )
{
    static const char* pStates[] = { "NORMAL", "ACTIVE", "PRELIGHT", "SELECTED", "INSENSITIVE" };

    for( int i = 0; i < 5; i++ )
    {
        fprintf( stderr, "state %s colors:\n", pStates[i] );
        printColor( "bg     ", pStyle->bg[i] );
        printColor( "fg     ", pStyle->fg[i] );
        printColor( "light  ", pStyle->light[i] );
        printColor( "dark   ", pStyle->dark[i] );
        printColor( "mid    ", pStyle->mid[i] );
        printColor( "text   ", pStyle->text[i] );
        printColor( "base   ", pStyle->base[i] );
        printColor( "text_aa", pStyle->text_aa[i] );
    }
}
#endif

void GtkSalFrame::UpdateSettings( AllSettings& rSettings )
{
    if( ! m_pWindow )
        return;

    StyleSettings aStyleSet = rSettings.GetStyleSettings();

    gtk_widget_ensure_style( GTK_WIDGET(m_pWindow) );
    GtkStyle* pStyle = gtk_widget_get_style( GTK_WIDGET(m_pWindow) );

#if OSL_DEBUG_LEVEL > 2
    printStyleColors( pStyle );
#endif

    // text colors
    Color aTextColor = getColor( pStyle->text[GTK_STATE_NORMAL] );
    aStyleSet.SetDialogTextColor( aTextColor );
    aStyleSet.SetMenuTextColor( aTextColor );
    aStyleSet.SetButtonTextColor( aTextColor );
    aStyleSet.SetRadioCheckTextColor( aTextColor );
    aStyleSet.SetGroupTextColor( aTextColor );
    aStyleSet.SetLabelTextColor( aTextColor );
    aStyleSet.SetInfoTextColor( aTextColor );
    aStyleSet.SetWindowTextColor( aTextColor );
    aStyleSet.SetFieldTextColor( aTextColor );

    // background colors
    Color aBackColor = getColor( pStyle->bg[GTK_STATE_NORMAL] );
    Color aBackFieldColor = getColor( pStyle->base[ GTK_STATE_NORMAL ] );
    aStyleSet.Set3DColors( aBackColor );
    aStyleSet.SetFaceColor( aBackColor );
    aStyleSet.SetDialogColor( aBackColor );
    aStyleSet.SetMenuColor( aBackColor );
    aStyleSet.SetMenuBarColor( aBackColor );
    aStyleSet.SetFieldColor( aBackFieldColor );
    aStyleSet.SetWindowColor( aBackFieldColor );
    // ancient wisdom tells us a mystic algorithm how to set checked color
    if( aBackColor == COL_LIGHTGRAY )
        aStyleSet.SetCheckedColor( Color( 0xCC, 0xCC, 0xCC ) );
    else
    {
        Color aColor2 = aStyleSet.GetLightColor();
        Color aCheck( (BYTE)(((USHORT)aBackColor.GetRed()+(USHORT)aColor2.GetRed())/2),
                      (BYTE)(((USHORT)aBackColor.GetGreen()+(USHORT)aColor2.GetGreen())/2),
                      (BYTE)(((USHORT)aBackColor.GetBlue()+(USHORT)aColor2.GetBlue())/2)
                      );
        aStyleSet.SetCheckedColor( aCheck );
    }

    // highlighting colors
    Color aHighlightColor = getColor( pStyle->base[GTK_STATE_SELECTED] );
    Color aHighlightTextColor = getColor( pStyle->text[GTK_STATE_SELECTED] );
    aStyleSet.SetHighlightColor( aHighlightColor );
    aStyleSet.SetHighlightTextColor( aHighlightTextColor );

    // menu highlighting
    aHighlightColor = getColor( pStyle->bg[ GTK_STATE_SELECTED ] );
    aStyleSet.SetMenuHighlightColor( aHighlightColor );
    aHighlightTextColor = getColor( pStyle->text[ GTK_STATE_PRELIGHT ] );
    aStyleSet.SetMenuHighlightTextColor( aHighlightTextColor );

    // UI font
    OString aFamily     = pango_font_description_get_family( pStyle->font_desc );
    int nPixelHeight    = pango_font_description_get_size( pStyle->font_desc )/PANGO_SCALE;
    PangoStyle  eStyle  = pango_font_description_get_style( pStyle->font_desc );
    PangoWeight eWeight = pango_font_description_get_weight( pStyle->font_desc );
    PangoStretch eStretch = pango_font_description_get_stretch( pStyle->font_desc );

    psp::FastPrintFontInfo aInfo;
    // set family name
    aInfo.m_aFamilyName = OStringToOUString( aFamily, RTL_TEXTENCODING_UTF8 );
    // set italic
    switch( eStyle )
    {
        case PANGO_STYLE_NORMAL:    aInfo.m_eItalic = psp::italic::Upright;break;
        case PANGO_STYLE_ITALIC:    aInfo.m_eItalic = psp::italic::Italic;break;
        case PANGO_STYLE_OBLIQUE:   aInfo.m_eItalic = psp::italic::Oblique;break;
    }
    // set weight
    if( eWeight <= PANGO_WEIGHT_ULTRALIGHT )
        aInfo.m_eWeight = psp::weight::UltraLight;
    else if( eWeight <= PANGO_WEIGHT_LIGHT )
        aInfo.m_eWeight = psp::weight::Light;
    else if( eWeight <= PANGO_WEIGHT_NORMAL )
        aInfo.m_eWeight = psp::weight::Normal;
    else if( eWeight <= PANGO_WEIGHT_BOLD )
        aInfo.m_eWeight = psp::weight::Bold;
    else
        aInfo.m_eWeight = psp::weight::UltraBold;
    // set width
    switch( eStretch )
    {
        case PANGO_STRETCH_ULTRA_CONDENSED: aInfo.m_eWidth = psp::width::UltraCondensed;break;
        case PANGO_STRETCH_EXTRA_CONDENSED: aInfo.m_eWidth = psp::width::ExtraCondensed;break;
        case PANGO_STRETCH_CONDENSED:       aInfo.m_eWidth = psp::width::Condensed;break;
        case PANGO_STRETCH_SEMI_CONDENSED:  aInfo.m_eWidth = psp::width::SemiCondensed;break;
        case PANGO_STRETCH_NORMAL:          aInfo.m_eWidth = psp::width::Normal;break;
        case PANGO_STRETCH_SEMI_EXPANDED:   aInfo.m_eWidth = psp::width::SemiExpanded;break;
        case PANGO_STRETCH_EXPANDED:        aInfo.m_eWidth = psp::width::Expanded;break;
        case PANGO_STRETCH_EXTRA_EXPANDED:  aInfo.m_eWidth = psp::width::ExtraExpanded;break;
        case PANGO_STRETCH_ULTRA_EXPANDED:  aInfo.m_eWidth = psp::width::UltraExpanded;break;
    }

#if OSL_DEBUG_LEVEL > 1
    fprintf( stderr, "font name BEFORE system match: \"%s\"\n", aFamily.getStr() );
#endif

    // match font to e.g. resolve "Sans"
    psp::PrintFontManager::get().matchFont( aInfo );

#if OSL_DEBUG_LEVEL > 1
    fprintf( stderr, "font match %s, name AFTER: \"%s\"\n",
             aInfo.m_nID != 0 ? "succeeded" : "failed",
             OUStringToOString( aInfo.m_aFamilyName, RTL_TEXTENCODING_ISO_8859_1 ).getStr() );
#endif

    sal_Int32 nDPIX, nDPIY;
    sal_Int32 nDispDPIY = getDisplay()->GetResolution().B();
    getDisplay()->GetScreenFontResolution( nDPIX, nDPIY );
    int nHeight = nPixelHeight * nDispDPIY / nDPIY;
    // allow for rounding in back conversion (at SetFont)
    while( (nHeight * nDPIY / nDispDPIY) > nPixelHeight )
        nHeight--;
    while( (nHeight * nDPIY / nDispDPIY) < nPixelHeight )
        nHeight++;

    Font aFont( aInfo.m_aFamilyName, Size( 0, nHeight ) );
    if( aInfo.m_eWeight != psp::weight::Unknown )
        aFont.SetWeight( PspGraphics::ToFontWeight( aInfo.m_eWeight ) );
    if( aInfo.m_eWidth != psp::width::Unknown )
        aFont.SetWidthType( PspGraphics::ToFontWidth( aInfo.m_eWidth ) );
    if( aInfo.m_eItalic != psp::italic::Unknown )
        aFont.SetItalic( PspGraphics::ToFontItalic( aInfo.m_eItalic ) );
    if( aInfo.m_ePitch != psp::pitch::Unknown )
        aFont.SetPitch( PspGraphics::ToFontPitch( aInfo.m_ePitch ) );

    aStyleSet.SetAppFont( aFont );
    aStyleSet.SetHelpFont( aFont );
    aStyleSet.SetTitleFont( aFont );
    aStyleSet.SetFloatTitleFont( aFont );
    aStyleSet.SetMenuFont( aFont );
    aStyleSet.SetToolFont( aFont );
    aStyleSet.SetLabelFont( aFont );
    aStyleSet.SetInfoFont( aFont );
    aStyleSet.SetRadioCheckFont( aFont );
    aStyleSet.SetPushButtonFont( aFont );
    aStyleSet.SetFieldFont( aFont );
    aStyleSet.SetIconFont( aFont );
    aStyleSet.SetGroupFont( aFont );

//  FIXME: need some way of fetching toolbar icon size.
//  aStyleSet.SetToolbarIconSize( STYLE_TOOLBAR_ICONSIZE_SMALL );

    rSettings.SetStyleSettings( aStyleSet );
}

void GtkSalFrame::Beep( SoundType eType )
{
    gdk_display_beep( getGdkDisplay() );
}

const SystemEnvData* GtkSalFrame::GetSystemData() const
{
    return &m_aSystemData;
}

void GtkSalFrame::SetParent( SalFrame* pNewParent )
{
    m_pParent = static_cast<GtkSalFrame*>(pNewParent);
    if( ! (m_nStyle & SAL_FRAME_STYLE_CHILD) )
        gtk_window_set_transient_for( m_pWindow,
                                      (m_pParent && !(m_pParent->m_nStyle & SAL_FRAME_STYLE_CHILD)) ? m_pParent->m_pWindow : NULL
                                     );
}

bool GtkSalFrame::SetPluginParent( SystemParentData* pSysParent )
{
    gtk_widget_destroy( GTK_WIDGET(m_pWindow) );
    if( m_pForeignParent )
        gdk_window_destroy( m_pForeignParent );
    if( m_pForeignTopLevel )
        gdk_window_destroy( m_pForeignTopLevel );
    Init( pSysParent );
    return true;
}

bool GtkSalFrame::Dispatch( const XEvent* pEvent )
{
    bool bContinueDispatch = true;

    if( m_pForeignParent &&
        pEvent->type == ConfigureNotify &&
        pEvent->xconfigure.window == GDK_WINDOW_XWINDOW(m_pForeignParent)
        )
    {
        bContinueDispatch = false;
        gtk_window_resize( m_pWindow, pEvent->xconfigure.width, pEvent->xconfigure.height );
    }
    else if( m_pForeignTopLevel &&
             pEvent->type == ConfigureNotify &&
             pEvent->xconfigure.window == GDK_WINDOW_XWINDOW(m_pForeignTopLevel)
             )
    {
        bContinueDispatch = false;
        // update position
        int x = 0, y = 0;
        XLIB_Window aChild;
          XTranslateCoordinates( getDisplay()->GetDisplay(),
                               GDK_WINDOW_XWINDOW(GTK_WIDGET(m_pWindow)->window),
                                getDisplay()->GetRootWindow(),
                                0, 0,
                                &x, &y,
                                &aChild );
        if( x != maGeometry.nX || y != maGeometry.nY )
        {
            maGeometry.nX = x;
            maGeometry.nY = y;
            getDisplay()->SendInternalEvent( this, NULL, SALEVENT_MOVE );
        }
    }

    return bContinueDispatch;
}

gboolean GtkSalFrame::signalButton( GtkWidget* pWidget, GdkEventButton* pEvent, gpointer frame )
{
    GtkSalFrame* pThis = (GtkSalFrame*)frame;
    SalMouseEvent aEvent;
    USHORT nEventType = 0;
    switch( pEvent->type )
    {
        case GDK_BUTTON_PRESS:
            nEventType = SALEVENT_MOUSEBUTTONDOWN;
            break;
        case GDK_BUTTON_RELEASE:
            nEventType = SALEVENT_MOUSEBUTTONUP;
            break;
        default:
            return FALSE;
    }
    switch( pEvent->button )
    {
        case 1: aEvent.mnButton = MOUSE_LEFT;   break;
        case 2: aEvent.mnButton = MOUSE_MIDDLE; break;
        case 3: aEvent.mnButton = MOUSE_RIGHT;  break;
        default: return FALSE;              break;
    }
    aEvent.mnTime   = pEvent->time;
    aEvent.mnX      = (long)pEvent->x;
    aEvent.mnY      = (long)pEvent->y;
    aEvent.mnCode   = GetModCode( pEvent->state );

    bool bClosePopups = false;
    if( pEvent->type == GDK_BUTTON_PRESS && m_nFloats > 0 )
    {
        // close popups if user clicks outside our application
        gint x, y;
        bClosePopups = (gdk_display_get_window_at_pointer( pThis->getGdkDisplay(), &x, &y ) == NULL);
        gdk_display_pointer_ungrab( pThis->getGdkDisplay(), GDK_CURRENT_TIME );
    }

    GTK_YIELD_GRAB();
    pThis->CallCallback( nEventType, &aEvent );

    if( bClosePopups )
    {
        ImplSVData* pSVData = ImplGetSVData();
        if ( pSVData->maWinData.mpFirstFloat )
        {
            static const char* pEnv = getenv( "SAL_FLOATWIN_NOAPPFOCUSCLOSE" );
            if ( !(pSVData->maWinData.mpFirstFloat->GetPopupModeFlags() & FLOATWIN_POPUPMODE_NOAPPFOCUSCLOSE) && !(pEnv && *pEnv) )
                pSVData->maWinData.mpFirstFloat->EndPopupMode( FLOATWIN_POPUPMODEEND_CANCEL | FLOATWIN_POPUPMODEEND_CLOSEALL );
        }
    }

    return FALSE;
}

gboolean GtkSalFrame::signalScroll( GtkWidget* pWidget, GdkEvent* pEvent, gpointer frame )
{
    GtkSalFrame* pThis = (GtkSalFrame*)frame;
    GdkEventScroll* pSEvent = (GdkEventScroll*)pEvent;

    static ULONG        nLines = 0;
    if( ! nLines )
    {
        char* pEnv = getenv( "SAL_WHEELLINES" );
        nLines = pEnv ? atoi( pEnv ) : 3;
        if( nLines > 10 )
            nLines = SAL_WHEELMOUSE_EVENT_PAGESCROLL;
    }

    bool bNeg = (pSEvent->direction == GDK_SCROLL_DOWN || pSEvent->direction == GDK_SCROLL_RIGHT );
    SalWheelMouseEvent aEvent;
    aEvent.mnTime           = pSEvent->time;
    aEvent.mnX              = (ULONG)pSEvent->x;
    aEvent.mnY              = (ULONG)pSEvent->y;
    aEvent.mnDelta          = bNeg ? -120 : 120;
    aEvent.mnNotchDelta     = bNeg ? -1 : 1;
    aEvent.mnScrollLines    = nLines;
    aEvent.mnCode           = GetModCode( pSEvent->state );
    aEvent.mbHorz           = (pSEvent->direction == GDK_SCROLL_LEFT || pSEvent->direction == GDK_SCROLL_RIGHT);

    GTK_YIELD_GRAB();
    pThis->CallCallback( SALEVENT_WHEELMOUSE, &aEvent );

    return FALSE;
}

gboolean GtkSalFrame::signalMotion( GtkWidget* pWidget, GdkEventMotion* pEvent, gpointer frame )
{
    GtkSalFrame* pThis = (GtkSalFrame*)frame;
    SalMouseEvent aEvent;
    aEvent.mnTime   = pEvent->time;
    aEvent.mnX      = (long)pEvent->x;
    aEvent.mnY      = (long)pEvent->y;
    aEvent.mnCode   = GetModCode( pEvent->state );
    aEvent.mnButton = 0;

    GTK_YIELD_GRAB();
    pThis->CallCallback( SALEVENT_MOUSEMOVE, &aEvent );

    // ask for the next hint
    gint x, y;
    GdkModifierType mask;
    gdk_window_get_pointer( GTK_WIDGET(pThis->m_pWindow)->window, &x, &y, &mask );

    return TRUE;
}

gboolean GtkSalFrame::signalCrossing( GtkWidget* pWidget, GdkEventCrossing* pEvent, gpointer frame )
{
    GtkSalFrame* pThis = (GtkSalFrame*)frame;
    SalMouseEvent aEvent;
    aEvent.mnTime   = pEvent->time;
    aEvent.mnX      = (long)pEvent->x;
    aEvent.mnY      = (long)pEvent->y;
    aEvent.mnCode   = GetModCode( pEvent->state );
    aEvent.mnButton = 0;

    GTK_YIELD_GRAB();
    pThis->CallCallback( (pEvent->type == GDK_ENTER_NOTIFY) ? SALEVENT_MOUSEMOVE : SALEVENT_MOUSELEAVE, &aEvent );

    return TRUE;
}


gboolean GtkSalFrame::signalExpose( GtkWidget* pWidget, GdkEventExpose* pEvent, gpointer frame )
{
    GtkSalFrame* pThis = (GtkSalFrame*)frame;

    struct SalPaintEvent aEvent;
    aEvent.mnBoundX         = pEvent->area.x;
    aEvent.mnBoundY         = pEvent->area.y;
    aEvent.mnBoundWidth     = pEvent->area.width;
    aEvent.mnBoundHeight    = pEvent->area.height;

    GTK_YIELD_GRAB();
    pThis->CallCallback( SALEVENT_PAINT, &aEvent );

    return FALSE;
}

gboolean GtkSalFrame::signalFocus( GtkWidget* pWidget, GdkEventFocus* pEvent, gpointer frame )
{
    GtkSalFrame* pThis = (GtkSalFrame*)frame;

    GTK_YIELD_GRAB();

    if( !pEvent->in )
    {
        pThis->m_nKeyModifiers = 0;
        pThis->m_bSingleAltPress = false;
        pThis->m_bSendModChangeOnRelease = false;
        pThis->m_bWasPreedit = false;
    }

    if( pThis->m_pIMContext )
    {
        if( pEvent->in )
        {
            gtk_im_context_focus_in( pThis->m_pIMContext );
            gtk_im_context_reset( pThis->m_pIMContext );
        }
        else
        {
            gtk_im_context_focus_out( pThis->m_pIMContext );
            pThis->CallCallback( SALEVENT_ENDEXTTEXTINPUT, NULL );
            gtk_im_context_reset( pThis->m_pIMContext );
        }
    }

    // FIXME: find out who the hell steals the focus from our frame
    // if we are plugged and a float opens; why does the same not
    // happen unplugged ? Is the plugging application (SimpleViewer in SDK)
    // somehow responsible ? The focus does not seem to get set inside our process
    //
    // in the meantime do not propagate focus get/lose on a plugged window
    // if floats are open
    if( ! ((pThis->m_nStyle & SAL_FRAME_STYLE_CHILD) && m_nFloats > 0 ) )
        pThis->CallCallback( pEvent->in ? SALEVENT_GETFOCUS : SALEVENT_LOSEFOCUS, NULL );

    return FALSE;
}

gboolean GtkSalFrame::signalMap( GtkWidget* pWidget, GdkEvent* pEvent, gpointer frame )
{
    GtkSalFrame* pThis = (GtkSalFrame*)frame;

    GTK_YIELD_GRAB();
    pThis->CallCallback( SALEVENT_RESIZE, NULL );

    return FALSE;
}

gboolean GtkSalFrame::signalUnmap( GtkWidget* pWidget, GdkEvent* pEvent, gpointer frame )
{
    GtkSalFrame* pThis = (GtkSalFrame*)frame;

    GTK_YIELD_GRAB();
    pThis->CallCallback( SALEVENT_RESIZE, NULL );

    return FALSE;
}

gboolean GtkSalFrame::signalConfigure( GtkWidget* pWidget, GdkEventConfigure* pEvent, gpointer frame )
{
    GtkSalFrame* pThis = (GtkSalFrame*)frame;

    bool bMoved = false, bSized = false;
    int x = pEvent->x, y = pEvent->y;

    if( (pThis->m_nStyle & SAL_FRAME_STYLE_CHILD) )
    {
        // in child case the coordinates are not root coordinates,
        // need to transform
        XLIB_Window aChild;
          XTranslateCoordinates( pThis->getDisplay()->GetDisplay(),
                               GDK_WINDOW_XWINDOW(GTK_WIDGET(pThis->m_pWindow)->window),
                                pThis->getDisplay()->GetRootWindow(),
                                0, 0,
                                &x, &y,
                                &aChild );
    }


    if( x != pThis->maGeometry.nX || y != pThis->maGeometry.nY )
        bMoved = true;
    if( pEvent->width != (int)pThis->maGeometry.nWidth || pEvent->height != (int)pThis->maGeometry.nHeight )
        bSized = true;

    pThis->maGeometry.nX        = x;
    pThis->maGeometry.nY        = y;
    pThis->maGeometry.nWidth    = pEvent->width;
    pThis->maGeometry.nHeight   = pEvent->height;

    // update decoration hints
    if( ! (pThis->m_nStyle & SAL_FRAME_STYLE_CHILD) )
    {
        GdkRectangle aRect;
        gdk_window_get_frame_extents( GTK_WIDGET(pThis->m_pWindow)->window, &aRect );
        pThis->maGeometry.nTopDecoration    = y - aRect.y;
        pThis->maGeometry.nBottomDecoration = aRect.y + aRect.height - y - pEvent->height;
        pThis->maGeometry.nLeftDecoration   = x - aRect.x;
        pThis->maGeometry.nRightDecoration  = aRect.x + aRect.width - x - pEvent->width;
    }
    else
    {
        pThis->maGeometry.nTopDecoration =
            pThis->maGeometry.nBottomDecoration =
            pThis->maGeometry.nLeftDecoration =
            pThis->maGeometry.nRightDecoration = 0;
    }

    GTK_YIELD_GRAB();
    if( bMoved && bSized )
        pThis->CallCallback( SALEVENT_MOVERESIZE, NULL );
    else if( bMoved )
        pThis->CallCallback( SALEVENT_MOVE, NULL );
    else if( bSized )
        pThis->CallCallback( SALEVENT_RESIZE, NULL );

    return FALSE;
}

gboolean GtkSalFrame::signalKey( GtkWidget* pWidget, GdkEventKey* pEvent, gpointer frame )
{
    GtkSalFrame* pThis = (GtkSalFrame*)frame;

    static GdkEventKey aLastPressEvent = { GDK_KEY_PRESS, NULL, 0, 0, 0, 0, 0, NULL, 0, 0 };

    if( pThis->m_pIMContext )
    {
        if( gtk_im_context_filter_keypress( pThis->m_pIMContext, pEvent ) )
        {
            aLastPressEvent = *pEvent;
            return TRUE;
        }
        else
            if( pEvent->type == GDK_KEY_PRESS )
                aLastPressEvent.window = NULL;
    }

    // swallow key release events if according keypress was filtered
    if( pEvent->type == GDK_KEY_RELEASE                         &&
        pEvent->window      == aLastPressEvent.window           &&
        pEvent->send_event  == aLastPressEvent.send_event       &&
        pEvent->state       == aLastPressEvent.state            &&
        pEvent->keyval      == aLastPressEvent.keyval )
    {
        aLastPressEvent.window = NULL;
        return TRUE;
    }

    GTK_YIELD_GRAB();

    // handle modifiers
    if( pEvent->keyval == GDK_Shift_L || pEvent->keyval == GDK_Shift_R ||
        pEvent->keyval == GDK_Control_L || pEvent->keyval == GDK_Control_R ||
        pEvent->keyval == GDK_Alt_L || pEvent->keyval == GDK_Alt_R ||
        pEvent->keyval == GDK_Meta_L || pEvent->keyval == GDK_Meta_R )
    {
        SalKeyModEvent aModEvt;

        USHORT nModCode = GetModCode( pEvent->state );

        aModEvt.mnModKeyCode = 0; // emit no MODKEYCHANGE events
        if( pEvent->type == GDK_KEY_PRESS && !pThis->m_nKeyModifiers )
            pThis->m_bSendModChangeOnRelease = true;

        else if( pEvent->type == GDK_KEY_RELEASE &&
                 pThis->m_bSendModChangeOnRelease )
        {
            aModEvt.mnModKeyCode = pThis->m_nKeyModifiers;
            pThis->m_nKeyModifiers = 0;
        }

        USHORT nExtModMask = 0;
        USHORT nModMask = 0;
        // pressing just the ctrl key leads to a keysym of XK_Control but
        // the event state does not contain ControlMask. In the release
        // event its the other way round: it does contain the Control mask.
        // The modifier mode therefore has to be adapted manually.
        switch( pEvent->keyval )
        {
            case GDK_Control_L:
                nExtModMask = MODKEY_LMOD1;
                nModMask = KEY_MOD1;
                break;
            case GDK_Control_R:
                nExtModMask = MODKEY_RMOD1;
                nModMask = KEY_MOD1;
                break;
            case GDK_Alt_L:
                nExtModMask = MODKEY_LMOD2;
                nModMask = KEY_MOD2 | (pEvent->type == GDK_KEY_RELEASE ? KEY_CONTROLMOD : 0);
                break;
            case GDK_Alt_R:
                nExtModMask = MODKEY_RMOD2;
                nModMask = KEY_MOD2 | (pEvent->type == GDK_KEY_RELEASE ? KEY_CONTROLMOD : 0);
                break;
            case GDK_Shift_L:
                nExtModMask = MODKEY_LSHIFT;
                nModMask = KEY_SHIFT;
                break;
            case GDK_Shift_R:
                nExtModMask = MODKEY_RSHIFT;
                nModMask = KEY_SHIFT;
                break;
        }
        if( pEvent->type == GDK_KEY_RELEASE )
        {
            nModCode &= ~nModMask;
            pThis->m_nKeyModifiers &= ~nExtModMask;
        }
        else
        {
            nModCode |= nModMask;
            pThis->m_nKeyModifiers |= nExtModMask;
        }

        aModEvt.mnCode = nModCode;
        aModEvt.mnTime = pEvent->time;

        pThis->CallCallback( SALEVENT_KEYMODCHANGE, &aModEvt );

        // emulate KEY_MENU
        if( ( pEvent->keyval == GDK_Alt_L || pEvent->keyval == GDK_Alt_R ) &&
            ( nModCode & ~(KEY_CONTROLMOD|KEY_MOD2)) == 0 )
        {
            if( pEvent->type == GDK_KEY_PRESS )
                pThis->m_bSingleAltPress = true;

            else if( pThis->m_bSingleAltPress )
            {
                SalKeyEvent aKeyEvt;

                aKeyEvt.mnCode     = KEY_MENU | nModCode;
                aKeyEvt.mnRepeat   = 0;
                aKeyEvt.mnTime     = pEvent->time;
                aKeyEvt.mnCharCode = 0;

                // simulate KEY_MENU
                pThis->CallCallback( SALEVENT_KEYINPUT, &aKeyEvt );
                pThis->CallCallback( SALEVENT_KEYUP, &aKeyEvt );
                pThis->m_bSingleAltPress = false;
            }
        }
        else
            pThis->m_bSingleAltPress = false;
    }
    else
    {
        SalKeyEvent aEvent;

        aEvent.mnTime           = pEvent->time;
        aEvent.mnCode           = GetKeyCode( pEvent->keyval ) | GetModCode( pEvent->state );
        aEvent.mnCharCode       = (USHORT)gdk_keyval_to_unicode( pEvent->keyval );
        aEvent.mnRepeat         = 0;

        pThis->CallCallback( (pEvent->type == GDK_KEY_PRESS) ? SALEVENT_KEYINPUT : SALEVENT_KEYUP, &aEvent );

        pThis->m_bSendModChangeOnRelease = false;
        pThis->m_bSingleAltPress = false;
    }

    return TRUE;
}

gboolean GtkSalFrame::signalDelete( GtkWidget* pWidget, GdkEvent* pEvent, gpointer frame )
{
    GtkSalFrame* pThis = (GtkSalFrame*)frame;

    GTK_YIELD_GRAB();
    pThis->CallCallback( SALEVENT_CLOSE, NULL );

    return TRUE;
}

void GtkSalFrame::signalStyleSet( GtkWidget* pWidget, GtkStyle* pPrevious, gpointer frame )
{
    GtkSalFrame* pThis = (GtkSalFrame*)frame;

    // every frame gets an initial style set on creation
    // do not post these as the whole application tends to
    // redraw itself to adjust to the new style
    // where there IS no new style resulting in tremendous unnecessary flickering
    if( pPrevious != NULL )
        // signalStyleSet does NOT usually have the gdk lock
        // so post user event to safely dispatch the SALEVENT_SETTINGSCHANGED
        // note: settings changed for multiple frames is avoided in winproc.cxx ImplHandleSettings
        pThis->getDisplay()->SendInternalEvent( pThis, NULL, SALEVENT_SETTINGSCHANGED );

    if( ! pThis->m_pParent )
    {
        // signalize theme changed for NWF caches
        // FIXME: should be called only once for a style change
        GtkSalGraphics::bThemeChanged = TRUE;
    }
}

gboolean GtkSalFrame::signalState( GtkWidget* pWidget, GdkEvent* pEvent, gpointer frame )
{
    GtkSalFrame* pThis = (GtkSalFrame*)frame;
    pThis->m_nState = pEvent->window_state.new_window_state;

    return FALSE;
}

gboolean GtkSalFrame::signalVisibility( GtkWidget* pWidget, GdkEventVisibility* pEvent, gpointer frame )
{
    GtkSalFrame* pThis = (GtkSalFrame*)frame;
    pThis->m_nVisibility = pEvent->state;

    return FALSE;
}

void GtkSalFrame::signalIMCommit( GtkIMContext* pContext, gchar* pText, gpointer frame )
{
    GtkSalFrame* pThis = (GtkSalFrame*)frame;

    SalExtTextInputEvent aTextEvent;

    aTextEvent.mnTime           = 0;
    aTextEvent.mpTextAttr       = 0;
    aTextEvent.maText           = String( pText, RTL_TEXTENCODING_UTF8 );
    aTextEvent.mnCursorPos      = aTextEvent.maText.Len();
    aTextEvent.mnCursorFlags    = 0;
    aTextEvent.mnDeltaStart     = 0;
    aTextEvent.mbOnlyCursor     = False;

    GTK_YIELD_GRAB();

    /* necessary HACK: all keyboard input comes in here as soon as a IMContext is set
     *  which is logical and consequent. But since even simple input like
     *  <space> comes through the commit signal instead of signalKey
     *  and all kinds of windows only implement KeyInput (e.g. PushButtons,
     *  RadioButtons and a lot of other Controls), will send a single
     *  KeyInput/KeyUp sequence instead of an ExtText event if there
     *  never was a preedit and the text is only one character.
     *
     *  In this case there the last ExtText event must have been
     *  SALEVENT_ENDEXTTEXTINPUT, either because of a regular commit
     *  or because there never was a preedit.
     */
    if( ! pThis->m_bWasPreedit && aTextEvent.maText.Len() == 1 )
    {
        USHORT nCharCode = GetKeyCodeFromUnicode( aTextEvent.maText.GetChar( 0 ) );
        SalKeyEvent aEvent;

        aEvent.mnTime           = 0;
        aEvent.mnCode           = nCharCode;
        aEvent.mnCharCode       = aTextEvent.maText.GetChar(0);
        aEvent.mnRepeat         = 0;

        pThis->m_bWasPreedit = false;
        pThis->CallCallback( SALEVENT_KEYINPUT, &aEvent );
        pThis->CallCallback( SALEVENT_KEYUP, &aEvent );
        return;
    }

    pThis->m_bWasPreedit = false;
    pThis->CallCallback( SALEVENT_EXTTEXTINPUT, (void*)&aTextEvent);
    pThis->CallCallback( SALEVENT_ENDEXTTEXTINPUT, (void*)NULL );
}

void GtkSalFrame::signalIMPreeditChanged( GtkIMContext* pContext, gpointer frame )
{
    GtkSalFrame* pThis = (GtkSalFrame*)frame;

    char*           pText           = NULL;
    PangoAttrList*  pAttribs        = NULL;
    gint            nCursorPos      = 0;

    gtk_im_context_get_preedit_string( pThis->m_pIMContext,
                                       &pText,
                                       &pAttribs,
                                       &nCursorPos );
    SalExtTextInputEvent aTextEvent;

    aTextEvent.mnTime           = 0;
    aTextEvent.maText           = String( pText, RTL_TEXTENCODING_UTF8 );
    aTextEvent.mnCursorPos      = nCursorPos;
    aTextEvent.mnCursorFlags    = 0;
    aTextEvent.mnDeltaStart     = 0;
    aTextEvent.mbOnlyCursor     = False;

    USHORT* pSalAttribs = new USHORT[ aTextEvent.maText.Len() ];
    // FIXME: more sophisticated attributes
    for( int i = 0; i < aTextEvent.maText.Len(); ++i )
        pSalAttribs[i] = SAL_EXTTEXTINPUT_ATTR_UNDERLINE;

    aTextEvent.mpTextAttr       = pSalAttribs;

    g_free( pText );
    pango_attr_list_unref( pAttribs );

    GTK_YIELD_GRAB();
    pThis->m_bWasPreedit = true;
    pThis->CallCallback( SALEVENT_EXTTEXTINPUT, (void*)&aTextEvent);

    delete [] pSalAttribs;
}

void GtkSalFrame::signalIMPreeditStart( GtkIMContext* pContext, gpointer frame )
{
//    GtkSalFrame* pThis = (GtkSalFrame*)frame;
}

void GtkSalFrame::signalIMPreeditEnd( GtkIMContext* pContext, gpointer frame )
{
//    GtkSalFrame* pThis = (GtkSalFrame*)frame;
}

gboolean GtkSalFrame::signalIMRetrieveSurrounding( GtkIMContext* pContext, gpointer frame )
{
//    GtkSalFrame* pThis = (GtkSalFrame*)frame;

    return FALSE;
}

gboolean GtkSalFrame::signalIMDeleteSurrounding( GtkIMContext* pContext, gint arg1, gint arg2, gpointer frame )
{
//    GtkSalFrame* pThis = (GtkSalFrame*)frame;

    return FALSE;
}
