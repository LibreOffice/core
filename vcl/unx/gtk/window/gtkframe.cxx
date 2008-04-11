/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: gtkframe.cxx,v $
 * $Revision: 1.79 $
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

#include <plugins/gtk/gtkframe.hxx>
#include <plugins/gtk/gtkdata.hxx>
#include <plugins/gtk/gtkinst.hxx>
#include <plugins/gtk/gtkgdi.hxx>
#include <vcl/keycodes.hxx>
#include <wmadaptor.hxx>
#include <sm.hxx>
#include <salbmp.h>
#include <salprn.h>
#include <vcl/floatwin.hxx>
#include <salprn.h>
#include <vcl/svapp.hxx>
#include <vcl/window.hxx>

#include <prex.h>
#include <X11/Xatom.h>
#include <postx.h>

#include <dlfcn.h>
#include <vcl/salbtype.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/impbmp.hxx>
#include <vcl/svids.hrc>

#include <algorithm>

#if OSL_DEBUG_LEVEL > 1
#include <cstdio>
#endif

#include <com/sun/star/accessibility/XAccessibleContext.hpp>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/XAccessibleStateSet.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/XAccessibleEditableText.hpp>

#ifdef ENABLE_DBUS
#include <dbus/dbus-glib.h>

#define GSS_DBUS_SERVICE        "org.gnome.ScreenSaver"
#define GSS_DBUS_PATH           "/org/gnome/ScreenSaver"
#define GSS_DBUS_INTERFACE      "org.gnome.ScreenSaver"
#endif

using namespace com::sun::star;

int GtkSalFrame::m_nFloats = 0;

static USHORT GetKeyModCode( guint state )
{
    USHORT nCode = 0;
    if( (state & GDK_SHIFT_MASK) )
        nCode |= KEY_SHIFT;
    if( (state & GDK_CONTROL_MASK)
#ifdef MACOSX
     || (state & GDK_MOD2_MASK) // map Meta (aka Command key) to Ctrl
#endif
    )
        nCode |= KEY_MOD1;
    if( (state & GDK_MOD1_MASK) )
    {
        nCode |= KEY_MOD2;
#ifdef MACOSX
        if( ! (nCode & KEY_MOD1) )
            nCode |= KEY_MOD3;
#endif
    }
    return nCode;
}

static USHORT GetMouseModCode( guint state )
{
    USHORT nCode = GetKeyModCode( state );
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
    else if( keyval >= GDK_KP_0 && keyval <= GDK_KP_9 )
        nCode = KEY_0 + (keyval-GDK_KP_0);
    else if( keyval >= GDK_A && keyval <= GDK_Z )
        nCode = KEY_A + (keyval-GDK_A );
    else if( keyval >= GDK_a && keyval <= GDK_z )
        nCode = KEY_A + (keyval-GDK_a );
    else if( keyval >= GDK_F1 && keyval <= GDK_F26 )
    {
        if( GetX11SalData()->GetDisplay()->IsNumLockFromXS() )
        {
            nCode = KEY_F1 + (keyval-GDK_F1);
        }
        else
        {
            switch( keyval )
            {
                // - - - - - Sun keyboard, see vcl/unx/source/app/saldisp.cxx
                case GDK_L2:
                    if( GetX11SalData()->GetDisplay()->GetServerVendor() == vendor_sun )
                        nCode = KEY_REPEAT;
                    else
                        nCode = KEY_F12;
                    break;
                case GDK_L3:            nCode = KEY_PROPERTIES; break;
                case GDK_L4:            nCode = KEY_UNDO;       break;
                case GDK_L6:            nCode = KEY_COPY;       break; // KEY_F16
                case GDK_L8:            nCode = KEY_PASTE;      break; // KEY_F18
                case GDK_L10:           nCode = KEY_CUT;        break; // KEY_F20
                default:
                    nCode = KEY_F1 + (keyval-GDK_F1);           break;
            }
        }
    }
    else
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
            case GDK_KP_Begin:
            case GDK_KP_Home:
            case GDK_Begin:
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
            case GDK_KP_Insert:
            case GDK_Insert:        nCode = KEY_INSERT;     break;
            case GDK_KP_Delete:
            case GDK_Delete:        nCode = KEY_DELETE;     break;
            case GDK_plus:
            case GDK_KP_Add:        nCode = KEY_ADD;        break;
            case GDK_minus:
            case GDK_KP_Subtract:   nCode = KEY_SUBTRACT;   break;
            case GDK_asterisk:
            case GDK_KP_Multiply:   nCode = KEY_MULTIPLY;   break;
            case GDK_slash:
            case GDK_KP_Divide:     nCode = KEY_DIVIDE;     break;
            case GDK_period:
            case GDK_decimalpoint:  nCode = KEY_POINT;      break;
            case GDK_comma:         nCode = KEY_COMMA;      break;
            case GDK_less:          nCode = KEY_LESS;       break;
            case GDK_greater:       nCode = KEY_GREATER;    break;
            case GDK_KP_Equal:
            case GDK_equal:         nCode = KEY_EQUAL;      break;
            case GDK_Find:          nCode = KEY_FIND;       break;
            case GDK_Menu:          nCode = KEY_CONTEXTMENU;break;
            case GDK_Help:          nCode = KEY_HELP;       break;
            case GDK_Undo:          nCode = KEY_UNDO;       break;
            case GDK_Redo:          nCode = KEY_REPEAT;     break;
            case GDK_KP_Decimal:
            case GDK_KP_Separator:  nCode = KEY_DECIMAL;    break;
            case GDK_asciitilde:    nCode = KEY_TILDE;      break;
            case GDK_leftsinglequotemark:
            case GDK_quoteleft: nCode = KEY_QUOTELEFT;      break;
            // some special cases, also see saldisp.cxx
            // - - - - - - - - - - - - -  Apollo - - - - - - - - - - - - - 0x1000
            case 0x1000FF02: // apXK_Copy
                nCode = KEY_COPY;
                break;
            case 0x1000FF03: // apXK_Cut
                nCode = KEY_CUT;
                break;
            case 0x1000FF04: // apXK_Paste
                nCode = KEY_PASTE;
                break;
            case 0x1000FF14: // apXK_Repeat
                nCode = KEY_REPEAT;
                break;
            // Exit, Save
            // - - - - - - - - - - - - - - D E C - - - - - - - - - - - - - 0x1000
            case 0x1000FF00:
                nCode = KEY_DELETE;
                break;
            // - - - - - - - - - - - - - -  H P  - - - - - - - - - - - - - 0x1000
            case 0x1000FF73: // hpXK_DeleteChar
                nCode = KEY_DELETE;
                break;
            case 0x1000FF74: // hpXK_BackTab
            case 0x1000FF75: // hpXK_KP_BackTab
                nCode = KEY_TAB;
                break;
            // - - - - - - - - - - - - - - I B M - - - - - - - - - - - - -
            // - - - - - - - - - - - - - - O S F - - - - - - - - - - - - - 0x1004
            case 0x1004FF02: // osfXK_Copy
                nCode = KEY_COPY;
                break;
            case 0x1004FF03: // osfXK_Cut
                nCode = KEY_CUT;
                break;
            case 0x1004FF04: // osfXK_Paste
                nCode = KEY_PASTE;
                break;
            case 0x1004FF07: // osfXK_BackTab
                nCode = KEY_TAB;
                break;
            case 0x1004FF08: // osfXK_BackSpace
                nCode = KEY_BACKSPACE;
                break;
            case 0x1004FF1B: // osfXK_Escape
                nCode = KEY_ESCAPE;
                break;
            // Up, Down, Left, Right, PageUp, PageDown
            // - - - - - - - - - - - - - - S C O - - - - - - - - - - - - -
            // - - - - - - - - - - - - - - S G I - - - - - - - - - - - - - 0x1007
            // - - - - - - - - - - - - - - S N I - - - - - - - - - - - - -
            // - - - - - - - - - - - - - - S U N - - - - - - - - - - - - - 0x1005
            case 0x1005FF10: // SunXK_F36
                nCode = KEY_F11;
                break;
            case 0x1005FF11: // SunXK_F37
                nCode = KEY_F12;
                break;
            case 0x1005FF70: // SunXK_Props
                nCode = KEY_PROPERTIES;
                break;
            case 0x1005FF71: // SunXK_Front
                nCode = KEY_FRONT;
                break;
            case 0x1005FF72: // SunXK_Copy
                nCode = KEY_COPY;
                break;
            case 0x1005FF73: // SunXK_Open
                nCode = KEY_OPEN;
                break;
            case 0x1005FF74: // SunXK_Paste
                nCode = KEY_PASTE;
                break;
            case 0x1005FF75: // SunXK_Cut
                nCode = KEY_CUT;
                break;
        }
    }

    return nCode;
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

void GtkSalFrame::doKeyCallback( guint state,
                                 guint keyval,
                                 guint16 hardware_keycode,
                                 guint8 /*group*/,
                                 guint32 time,
                                 sal_Unicode aOrigCode,
                                 bool bDown,
                                 bool bSendRelease
                                 )
{
    SalKeyEvent aEvent;

    aEvent.mnTime           = time;
    aEvent.mnCharCode       = aOrigCode;
    aEvent.mnRepeat         = 0;

    vcl::DeletionListener aDel( this );
    /* #i42122# translate all keys with Ctrl and/or Alt to group 0
    *  else shortcuts (e.g. Ctrl-o) will not work but be inserted by
    *  the application
    */
    /* #i52338# do this for all keys that the independent part has no key code for
    */
    aEvent.mnCode = GetKeyCode( keyval );
    if( aEvent.mnCode == 0 )
    {
        // check other mapping
        gint eff_group, level;
        GdkModifierType consumed;
        guint updated_keyval = 0;
        // use gdk_keymap_get_default instead of NULL;
        // workaround a crahs fixed in gtk 2.4
        if( gdk_keymap_translate_keyboard_state( gdk_keymap_get_default(),
                                                 hardware_keycode,
                                                 (GdkModifierType)0,
                                                 0,
                                                 &updated_keyval,
                                                 &eff_group,
                                                 &level,
                                                 &consumed ) )
        {
            aEvent.mnCode   = GetKeyCode( updated_keyval );
        }
    }
    aEvent.mnCode   |= GetKeyModCode( state );

    if( bDown )
    {
        bool bHandled = CallCallback( SALEVENT_KEYINPUT, &aEvent );
        // #i46889# copy AlternatKeyCode handling from generic plugin
        if( ! bHandled )
        {
            KeyAlternate aAlternate = GetAlternateKeyCode( aEvent.mnCode );
            if( aAlternate.nKeyCode )
            {
                aEvent.mnCode = aAlternate.nKeyCode;
                if( aAlternate.nCharCode )
                    aEvent.mnCharCode = aAlternate.nCharCode;
                bHandled = CallCallback( SALEVENT_KEYINPUT, &aEvent );
            }
        }
        if( bSendRelease && ! aDel.isDeleted() )
        {
            CallCallback( SALEVENT_KEYUP, &aEvent );
        }
    }
    else
        CallCallback( SALEVENT_KEYUP, &aEvent );
}

GtkSalFrame::GraphicsHolder::~GraphicsHolder()
{
    delete pGraphics;
}

GtkSalFrame::GtkSalFrame( SalFrame* pParent, ULONG nStyle )
{
    m_nScreen = getDisplay()->GetDefaultScreenNumber();
    getDisplay()->registerFrame( this );
    m_bDefaultPos       = true;
    m_bDefaultSize      = ( (nStyle & SAL_FRAME_STYLE_SIZEABLE) && ! pParent );
    m_bWindowIsGtkPlug  = false;
    Init( pParent, nStyle );
}

GtkSalFrame::GtkSalFrame( SystemParentData* pSysData )
{
    m_nScreen = getDisplay()->GetDefaultScreenNumber();
    getDisplay()->registerFrame( this );
    getDisplay()->setHaveSystemChildFrame();
    m_bDefaultPos       = true;
    m_bDefaultSize      = true;
    Init( pSysData );
}

GtkSalFrame::~GtkSalFrame()
{
    if( m_pParent )
        m_pParent->m_aChildren.remove( this );

    getDisplay()->deregisterFrame( this );

    if( m_pRegion )
        gdk_region_destroy( m_pRegion );

    if( m_hBackgroundPixmap )
    {
        XSetWindowBackgroundPixmap( getDisplay()->GetDisplay(),
                                    GDK_WINDOW_XWINDOW(m_pWindow->window),
                                    None );
        XFreePixmap( getDisplay()->GetDisplay(), m_hBackgroundPixmap );
    }

    if( m_pIMHandler )
        delete m_pIMHandler;

    if( m_pFixedContainer )
        gtk_widget_destroy( GTK_WIDGET(m_pFixedContainer) );
    if( m_pWindow )
    {
        g_object_set_data( G_OBJECT( m_pWindow ), "SalFrame", NULL );
        gtk_widget_destroy( m_pWindow );
    }
    if( m_pForeignParent )
        g_object_unref( G_OBJECT(m_pForeignParent) );
    if( m_pForeignTopLevel )
        g_object_unref(G_OBJECT( m_pForeignTopLevel) );
}

void GtkSalFrame::moveWindow( long nX, long nY )
{
    if( isChild( false, true ) )
    {
        if( m_pParent )
            gtk_fixed_move( m_pParent->getFixedContainer(),
                            m_pWindow,
                            nX - m_pParent->maGeometry.nX, nY - m_pParent->maGeometry.nY );
    }
    else
        gtk_window_move( GTK_WINDOW(m_pWindow), nX, nY );
}

void GtkSalFrame::resizeWindow( long nWidth, long nHeight )
{
    if( isChild( false, true ) )
        gtk_widget_set_size_request( m_pWindow, nWidth, nHeight );
    else if( ! isChild( true, false ) )
        gtk_window_resize( GTK_WINDOW(m_pWindow), nWidth, nHeight );
}

/*
 * Always use a sub-class of GtkFixed we can tag for a11y. This allows us to
 * utilize GAIL for the toplevel window and toolkit implementation incl.
 * key event listener support ..
 */

GType
ooo_fixed_get_type()
{
    static GType type = 0;

    if (!type) {
        static const GTypeInfo tinfo =
        {
            sizeof (GtkFixedClass),
            (GBaseInitFunc) NULL,      /* base init */
            (GBaseFinalizeFunc) NULL,  /* base finalize */
            (GClassInitFunc) NULL,     /* class init */
            (GClassFinalizeFunc) NULL, /* class finalize */
            NULL,                      /* class data */
            sizeof (GtkFixed),         /* instance size */
            0,                         /* nb preallocs */
            (GInstanceInitFunc) NULL,  /* instance init */
            NULL                       /* value table */
        };

        type = g_type_register_static( GTK_TYPE_FIXED, "OOoFixed",
                                       &tinfo, (GTypeFlags) 0);
    }

    return type;
}

void GtkSalFrame::updateScreenNumber()
{
    if( getDisplay()->IsXinerama() && getDisplay()->GetXineramaScreens().size() > 1 )
    {
        Point aPoint( maGeometry.nX, maGeometry.nY );
        const std::vector<Rectangle>& rScreenRects( getDisplay()->GetXineramaScreens() );
        size_t nScreens = rScreenRects.size();
        for( size_t i = 0; i < nScreens; i++ )
        {
            if( rScreenRects[i].IsInside( aPoint ) )
            {
                maGeometry.nScreenNumber = static_cast<unsigned int>(i);
                break;
            }
        }
    }
    else
        maGeometry.nScreenNumber = static_cast<unsigned int>(m_nScreen);
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
    g_signal_connect( G_OBJECT(m_pWindow), "destroy", G_CALLBACK(signalDestroy), this );

    // init members
    m_pCurrentCursor    = NULL;
    m_nKeyModifiers     = 0;
    m_bSingleAltPress   = false;
    m_bFullscreen       = false;
    m_nState            = GDK_WINDOW_STATE_WITHDRAWN;
    m_nVisibility       = GDK_VISIBILITY_FULLY_OBSCURED;
    m_bSendModChangeOnRelease = false;
    m_pIMHandler        = NULL;
    m_hBackgroundPixmap = None;
    m_nSavedScreenSaverTimeout = 0;
    m_nGSSCookie = 0;
    m_nExtStyle         = 0;
    m_pRegion           = NULL;
    m_ePointerStyle     = 0xffff;

    gtk_widget_set_app_paintable( m_pWindow, TRUE );
    gtk_widget_set_double_buffered( m_pWindow, FALSE );
    gtk_widget_set_redraw_on_allocate( m_pWindow, FALSE );
    gtk_widget_add_events( m_pWindow,
                           GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK |
                           GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK |
                           GDK_VISIBILITY_NOTIFY_MASK
                           );

    // add the fixed container child,
    // fixed is needed since we have to position plugin windows
    m_pFixedContainer = GTK_FIXED(g_object_new( ooo_fixed_get_type(), NULL ));
    gtk_container_add( GTK_CONTAINER(m_pWindow), GTK_WIDGET(m_pFixedContainer) );

    // show the widgets
    gtk_widget_show( GTK_WIDGET(m_pFixedContainer) );

    // realize the window, we need an XWindow id
    gtk_widget_realize( m_pWindow );

    //system data
    SalDisplay* pDisp = GetX11SalData()->GetDisplay();
    m_aSystemData.pDisplay      = pDisp->GetDisplay();
    m_aSystemData.aWindow       = GDK_WINDOW_XWINDOW(m_pWindow->window);
    m_aSystemData.pSalFrame     = this;
    m_aSystemData.pWidget       = m_pWindow;
    m_aSystemData.pVisual       = pDisp->GetVisual( m_nScreen ).GetVisual();
    m_aSystemData.nDepth        = pDisp->GetVisual( m_nScreen ).GetDepth();
    m_aSystemData.aColormap     = pDisp->GetColormap( m_nScreen ).GetXColormap();
    m_aSystemData.pAppContext   = NULL;
    m_aSystemData.aShellWindow  = m_aSystemData.aWindow;
    m_aSystemData.pShellWidget  = m_aSystemData.pWidget;


    // fake an initial geometry, gets updated via configure event or SetPosSize
    if( m_bDefaultPos || m_bDefaultSize )
    {
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
    else
    {
        resizeWindow( maGeometry.nWidth, maGeometry.nHeight );
        moveWindow( maGeometry.nX, maGeometry.nY );
    }
    updateScreenNumber();

    SetIcon(1);
    m_nWorkArea = pDisp->getWMAdaptor()->getCurrentWorkArea();

    /* #i64117# gtk sets a nice background pixmap
    *  but we actually don't really want that, so save
    *  some time on the Xserver as well as prevent
    *  some paint issues
    */
    XSetWindowBackgroundPixmap( getDisplay()->GetDisplay(),
                                GDK_WINDOW_XWINDOW(m_pWindow->window),
                                m_hBackgroundPixmap );
}

/*  Sadly gtk_window_set_accept_focus exists only since gtk 2.4
 *  for achieving the same effect we will remove the WM_TAKE_FOCUS
 *  protocol from the window and set the input hint to false.
 *  But gtk_window_set_accept_focus needs to be called before
 *  window realization whereas the removal obviously can only happen
 *  after realization.
 */

extern "C" {
    typedef void(*setAcceptFn)( GtkWindow*, gboolean );
    static setAcceptFn p_gtk_window_set_accept_focus = NULL;
    static bool bGetAcceptFocusFn = true;
}

static void lcl_set_accept_focus( GtkWindow* pWindow, gboolean bAccept, bool bBeforeRealize )
{
    if( bGetAcceptFocusFn )
    {
        bGetAcceptFocusFn = false;
        OUString aSym( RTL_CONSTASCII_USTRINGPARAM( "gtk_window_set_accept_focus" ) );
        p_gtk_window_set_accept_focus = (setAcceptFn)osl_getFunctionSymbol( GetSalData()->m_pPlugin, aSym.pData );
    }
    if( p_gtk_window_set_accept_focus && bBeforeRealize )
        p_gtk_window_set_accept_focus( pWindow, bAccept );
    else if( ! bBeforeRealize )
    {
        Display* pDisplay = GetX11SalData()->GetDisplay()->GetDisplay();
        XLIB_Window aWindow = GDK_WINDOW_XWINDOW( GTK_WIDGET(pWindow)->window );
        XWMHints* pHints = XGetWMHints( pDisplay, aWindow );
        if( ! pHints )
        {
            pHints = XAllocWMHints();
            pHints->flags = 0;
        }
        pHints->flags |= InputHint;
        pHints->input = bAccept ? True : False;
        XSetWMHints( pDisplay, aWindow, pHints );
        XFree( pHints );

        /*  remove WM_TAKE_FOCUS protocol; this would usually be the
         *  right thing, but gtk handles it internally whereas we
         *  want to handle it ourselves (as to sometimes not get
         *  the focus)
         */
        Atom* pProtocols = NULL;
        int nProtocols = 0;
        XGetWMProtocols( pDisplay,
                         aWindow,
                         &pProtocols, &nProtocols );
        if( pProtocols )
        {
            bool bSet = false;
            Atom nTakeFocus = XInternAtom( pDisplay, "WM_TAKE_FOCUS", True );
            if( nTakeFocus )
            {
                for( int i = 0; i < nProtocols; i++ )
                {
                    if( pProtocols[i] == nTakeFocus )
                    {
                        for( int n = i; n < nProtocols-1; n++ )
                            pProtocols[n] = pProtocols[n+1];
                        nProtocols--;
                        i--;
                        bSet = true;
                    }
                }
            }
            if( bSet )
                XSetWMProtocols( pDisplay, aWindow, pProtocols, nProtocols );
            XFree( pProtocols );
        }
    }
}

GtkSalFrame *GtkSalFrame::getFromWindow( GtkWindow *pWindow )
{
    return (GtkSalFrame *) g_object_get_data( G_OBJECT( pWindow ), "SalFrame" );
}

AtkRole
GtkSalFrame::GetAtkRole( GtkWindow* window )
{
    static AtkRole aDefaultRole = ATK_ROLE_INVALID;

    // Special role for sub-menu and combo-box popups that are exposed directly
    // by their parents already.
    if( aDefaultRole == ATK_ROLE_INVALID )
        aDefaultRole = atk_role_register( "redundant object" );

    AtkRole role = ATK_ROLE_INVALID;

    GtkSalFrame * pFrame = getFromWindow( window );
    if( pFrame )
    {
        role = aDefaultRole;

        Window *pWindow = pFrame->GetWindow();
        if( pWindow )
        {
            // Determine the appropriate role for the GtkWindow
            switch( pWindow->GetAccessibleRole() )
            {
                case accessibility::AccessibleRole::ALERT:
                    role = ATK_ROLE_ALERT;
                    break;

                case accessibility::AccessibleRole::DIALOG:
                    role = ATK_ROLE_DIALOG;
                    break;

                case accessibility::AccessibleRole::FRAME:
                    role = ATK_ROLE_FRAME;
                    break;

                // Ignore window objects for sub-menus, which are exposed
                // as children of their parent menu
                case accessibility::AccessibleRole::WINDOW:
                {
                    Window *pChild = pWindow->GetChild( 0 );
                    if( pChild )
                    {
                        uno::Reference< accessibility::XAccessible > xAccessible( pChild->GetAccessible( true ) );
                        if( xAccessible.is() )
                            role = ATK_ROLE_WINDOW;
                    }
                }
                break;

                default:
                {
                    Window *pChild = pWindow->GetChild( 0 );
                    if( pChild )
                    {
                        if( WINDOW_HELPTEXTWINDOW == pChild->GetType() )
                        {
                            role = ATK_ROLE_TOOL_TIP;
                            pChild->SetAccessibleRole( accessibility::AccessibleRole::LABEL );
                        }
                    }
                    break;
                }
            }
        }
    }

    return role;
}

void GtkSalFrame::Init( SalFrame* pParent, ULONG nStyle )
{
    if( nStyle & SAL_FRAME_STYLE_DEFAULT ) // ensure default style
    {
        nStyle |= SAL_FRAME_STYLE_MOVEABLE | SAL_FRAME_STYLE_SIZEABLE | SAL_FRAME_STYLE_CLOSEABLE;
        nStyle &= ~SAL_FRAME_STYLE_FLOAT;
    }

    m_pParent = static_cast<GtkSalFrame*>(pParent);
    m_pForeignParent = NULL;
    m_aForeignParentWindow = None;
    m_pForeignTopLevel = NULL;
    m_aForeignTopLevelWindow = None;
    m_nStyle = nStyle;

    if( nStyle & SAL_FRAME_STYLE_SYSTEMCHILD )
    {
        m_pWindow = gtk_event_box_new();
        if( m_pParent )
        {
            // insert into container
            gtk_fixed_put( m_pParent->getFixedContainer(),
                           m_pWindow, 0, 0 );

        }
    }
    else
        m_pWindow = gtk_widget_new( GTK_TYPE_WINDOW, "type", ((nStyle & SAL_FRAME_STYLE_FLOAT) && ! (nStyle & SAL_FRAME_STYLE_OWNERDRAWDECORATION)) ? GTK_WINDOW_POPUP : GTK_WINDOW_TOPLEVEL, "visible", FALSE, NULL );
    g_object_set_data( G_OBJECT( m_pWindow ), "SalFrame", this );

    // force wm class hint
    m_nExtStyle = ~0;
    SetExtendedFrameStyle( 0 );

    if( m_pParent && m_pParent->m_pWindow && ! isChild() )
        gtk_window_set_screen( GTK_WINDOW(m_pWindow), gtk_window_get_screen( GTK_WINDOW(m_pParent->m_pWindow) ) );

    // set window type
    bool bDecoHandling =
        ! isChild() &&
        ( ! (nStyle & SAL_FRAME_STYLE_FLOAT) ||
          (nStyle & SAL_FRAME_STYLE_OWNERDRAWDECORATION) );

    if( bDecoHandling )
    {
        bool bNoDecor = ! (nStyle & (SAL_FRAME_STYLE_MOVEABLE | SAL_FRAME_STYLE_SIZEABLE | SAL_FRAME_STYLE_CLOSEABLE ) );
        GdkWindowTypeHint eType = GDK_WINDOW_TYPE_HINT_NORMAL;
        if( (nStyle & SAL_FRAME_STYLE_DIALOG) && m_pParent != 0 )
            eType = GDK_WINDOW_TYPE_HINT_DIALOG;
        if( (nStyle & SAL_FRAME_STYLE_INTRO) )
        {
            gtk_window_set_role( GTK_WINDOW(m_pWindow), "splashscreen" );
            eType = GDK_WINDOW_TYPE_HINT_SPLASHSCREEN;
        }
        else if( (nStyle & SAL_FRAME_STYLE_TOOLWINDOW ) )
        {
            eType = GDK_WINDOW_TYPE_HINT_UTILITY;
            gtk_window_set_skip_taskbar_hint( GTK_WINDOW(m_pWindow), true );
        }
        else if( (nStyle & SAL_FRAME_STYLE_OWNERDRAWDECORATION) )
        {
            eType = GDK_WINDOW_TYPE_HINT_TOOLBAR;
            lcl_set_accept_focus( GTK_WINDOW(m_pWindow), FALSE, true );
            bNoDecor = true;
        }
        if( (nStyle & SAL_FRAME_STYLE_PARTIAL_FULLSCREEN ) )
        {
            eType = GDK_WINDOW_TYPE_HINT_DOCK;
            gtk_window_set_keep_above( GTK_WINDOW(m_pWindow), true );
        }

        gtk_window_set_type_hint( GTK_WINDOW(m_pWindow), eType );
        if( bNoDecor )
            gtk_window_set_decorated( GTK_WINDOW(m_pWindow), FALSE );
        gtk_window_set_gravity( GTK_WINDOW(m_pWindow), GDK_GRAVITY_STATIC );
        if( m_pParent && ! (m_pParent->m_nStyle & SAL_FRAME_STYLE_PLUG) )
            gtk_window_set_transient_for( GTK_WINDOW(m_pWindow), GTK_WINDOW(m_pParent->m_pWindow) );
    }
    else if( (nStyle & SAL_FRAME_STYLE_FLOAT) )
    {
        gtk_window_set_type_hint( GTK_WINDOW(m_pWindow), GDK_WINDOW_TYPE_HINT_UTILITY );
    }
    if( m_pParent )
        m_pParent->m_aChildren.push_back( this );

    InitCommon();

    if( bDecoHandling )
    {
        gtk_window_set_resizable( GTK_WINDOW(m_pWindow), (nStyle & SAL_FRAME_STYLE_SIZEABLE) ? TRUE : FALSE );
        if( (nStyle & SAL_FRAME_STYLE_OWNERDRAWDECORATION) )
            lcl_set_accept_focus( GTK_WINDOW(m_pWindow), FALSE, false );
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
        pChildren = NULL;
        nChildren = 0;
        aParent = aRoot = None;
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
    m_aForeignParentWindow = (GdkNativeWindow)pSysData->aWindow;
    m_pForeignParent = NULL;
    m_aForeignTopLevelWindow = findTopLevelSystemWindow( (GdkNativeWindow)pSysData->aWindow );
    m_pForeignTopLevel = gdk_window_foreign_new_for_display( getGdkDisplay(), m_aForeignTopLevelWindow );
    gdk_window_set_events( m_pForeignTopLevel, GDK_STRUCTURE_MASK );

    if( pSysData->nSize > sizeof(pSysData->nSize)+sizeof(pSysData->aWindow) && pSysData->bXEmbedSupport )
    {
        m_pWindow = gtk_plug_new( pSysData->aWindow );
        m_bWindowIsGtkPlug  = true;
        GTK_WIDGET_SET_FLAGS( m_pWindow, GTK_CAN_FOCUS | GTK_SENSITIVE | GTK_CAN_DEFAULT );
        gtk_widget_set_sensitive( m_pWindow, true );
    }
    else
    {
        m_pWindow = gtk_window_new( GTK_WINDOW_POPUP );
        m_bWindowIsGtkPlug  = false;
    }
    m_nStyle = SAL_FRAME_STYLE_PLUG;
    InitCommon();

    m_pForeignParent = gdk_window_foreign_new_for_display( getGdkDisplay(), m_aForeignParentWindow );
    gdk_window_set_events( m_pForeignParent, GDK_STRUCTURE_MASK );
    int x_ret, y_ret;
    unsigned int w, h, bw, d;
    XLIB_Window aRoot;
    XGetGeometry( getDisplay()->GetDisplay(), pSysData->aWindow,
                  &aRoot, &x_ret, &y_ret, &w, &h, &bw, &d );
    maGeometry.nWidth   = w;
    maGeometry.nHeight  = h;
    gtk_window_resize( GTK_WINDOW(m_pWindow), w, h );
    gtk_window_move( GTK_WINDOW(m_pWindow), 0, 0 );
    if( ! m_bWindowIsGtkPlug )
    {
        XReparentWindow( getDisplay()->GetDisplay(),
                         GDK_WINDOW_XWINDOW(m_pWindow->window),
                         (XLIB_Window)pSysData->aWindow,
                         0, 0 );
    }
}

void GtkSalFrame::askForXEmbedFocus( sal_Int32 i_nTimeCode )
{
    XEvent aEvent;

    rtl_zeroMemory( &aEvent, sizeof(aEvent) );
    aEvent.xclient.window = m_aForeignParentWindow;
    aEvent.xclient.type = ClientMessage;
    aEvent.xclient.message_type = getDisplay()->getWMAdaptor()->getAtom( vcl_sal::WMAdaptor::XEMBED );
    aEvent.xclient.format = 32;
    aEvent.xclient.data.l[0] = i_nTimeCode ? i_nTimeCode : CurrentTime;
    aEvent.xclient.data.l[1] = 3; // XEMBED_REQUEST_FOCUS
    aEvent.xclient.data.l[2] = 0;
    aEvent.xclient.data.l[3] = 0;
    aEvent.xclient.data.l[4] = 0;

    getDisplay()->GetXLib()->PushXErrorLevel( true );
    XSendEvent( getDisplay()->GetDisplay(),
                m_aForeignParentWindow,
                False, NoEventMask, &aEvent );
    XSync( getDisplay()->GetDisplay(), False );
    getDisplay()->GetXLib()->PopXErrorLevel();
}

void GtkSalFrame::SetExtendedFrameStyle( SalExtStyle nStyle )
{
    if( nStyle != m_nExtStyle && ! isChild() )
    {
        m_nExtStyle = nStyle;
        if( GTK_WIDGET_REALIZED( m_pWindow ) )
        {
            XClassHint* pClass = XAllocClassHint();
            rtl::OString aResHint = X11SalData::getFrameResName( m_nExtStyle );
            pClass->res_name  = const_cast<char*>(aResHint.getStr());
            pClass->res_class = const_cast<char*>(X11SalData::getFrameClassName());
            XSetClassHint( getDisplay()->GetDisplay(),
                           GDK_WINDOW_XWINDOW(m_pWindow->window),
                           pClass );
            XFree( pClass );
        }
        else
            gtk_window_set_wmclass( GTK_WINDOW(m_pWindow),
                                    X11SalData::getFrameResName( m_nExtStyle ),
                                    X11SalData::getFrameClassName() );
    }
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
                    m_aGraphics[i].pGraphics = new GtkSalGraphics( m_pWindow );
                    m_aGraphics[i].pGraphics->Init( this, GDK_WINDOW_XWINDOW(m_pWindow->window), m_nScreen );
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
    m_aTitle = rTitle;
    if( m_pWindow && ! isChild() )
        gtk_window_set_title( GTK_WINDOW(m_pWindow), rtl::OUStringToOString( rTitle, RTL_TEXTENCODING_UTF8 ).getStr() );
}

static inline BYTE *
getRow( BitmapBuffer *pBuffer, ULONG nRow )
{
    if( BMP_SCANLINE_ADJUSTMENT( pBuffer->mnFormat ) == BMP_FORMAT_TOP_DOWN )
        return pBuffer->mpBits + nRow * pBuffer->mnScanlineSize;
    else
        return pBuffer->mpBits + ( pBuffer->mnHeight - nRow - 1 ) * pBuffer->mnScanlineSize;
}

static GdkPixbuf *
bitmapToPixbuf( SalBitmap *pSalBitmap, SalBitmap *pSalAlpha )
{
    g_return_val_if_fail( pSalBitmap != NULL, NULL );
    g_return_val_if_fail( pSalAlpha != NULL, NULL );

    BitmapBuffer *pBitmap = pSalBitmap->AcquireBuffer( TRUE );
    g_return_val_if_fail( pBitmap != NULL, NULL );
    g_return_val_if_fail( pBitmap->mnBitCount == 24, NULL );

    BitmapBuffer *pAlpha = pSalAlpha->AcquireBuffer( TRUE );
    g_return_val_if_fail( pAlpha != NULL, NULL );
    g_return_val_if_fail( pAlpha->mnBitCount == 8, NULL );

    Size aSize = pSalBitmap->GetSize();
    g_return_val_if_fail( pSalAlpha->GetSize() == aSize, NULL );

    int nX, nY;
    guchar *pPixbufData = (guchar *)g_malloc (4 * aSize.Width() * aSize.Height() );
    guchar *pDestData = pPixbufData;

    for( nY = 0; nY < pBitmap->mnHeight; nY++ )
    {
        BYTE *pData = getRow( pBitmap, nY );
        BYTE *pAlphaData = getRow( pAlpha, nY );

        for( nX = 0; nX < pBitmap->mnWidth; nX++ )
        {
            if( pBitmap->mnFormat == BMP_FORMAT_24BIT_TC_BGR )
            {
                pDestData[2] = *pData++;
                pDestData[1] = *pData++;
                pDestData[0] = *pData++;
            }
            else // BMP_FORMAT_24BIT_TC_RGB
            {
                pDestData[0] = *pData++;
                pDestData[1] = *pData++;
                pDestData[2] = *pData++;
            }
            pDestData += 3;
            *pDestData++ = 255 - *pAlphaData++;
        }
    }

    pSalBitmap->ReleaseBuffer( pBitmap, TRUE );
    pSalAlpha->ReleaseBuffer( pAlpha, TRUE );

    return gdk_pixbuf_new_from_data( pPixbufData,
                                     GDK_COLORSPACE_RGB, TRUE, 8,
                                     aSize.Width(), aSize.Height(),
                                     aSize.Width() * 4,
                                     (GdkPixbufDestroyNotify) g_free,
                                     NULL );
}

void GtkSalFrame::SetIcon( USHORT nIcon )
{
    if( (m_nStyle & (SAL_FRAME_STYLE_PLUG|SAL_FRAME_STYLE_SYSTEMCHILD|SAL_FRAME_STYLE_FLOAT|SAL_FRAME_STYLE_INTRO|SAL_FRAME_STYLE_OWNERDRAWDECORATION))
        || ! m_pWindow )
        return;

    if( !ImplGetResMgr() )
        return;

    GdkPixbuf *pBuf;
    GList *pIcons = NULL;

    USHORT nOffsets[2] = { SV_ICON_SMALL_START, SV_ICON_LARGE_START };
    USHORT nIndex;

    // Use high contrast icons where appropriate
    if( Application::GetSettings().GetStyleSettings().GetFaceColor().IsDark() )
    {
        nOffsets[0] = SV_ICON_LARGE_HC_START;
        nOffsets[1] = SV_ICON_SMALL_HC_START;
    }

    for( nIndex = 0; nIndex < sizeof(nOffsets)/ sizeof(USHORT); nIndex++ )
    {
        // #i44723# workaround gcc temporary problem
        ResId aResId( nOffsets[nIndex] + nIcon, *ImplGetResMgr() );
        BitmapEx aIcon( aResId );

        // #i81083# convert to 24bit/8bit alpha bitmap
        Bitmap aBmp = aIcon.GetBitmap();
        if( aBmp.GetBitCount() != 24 || ! aIcon.IsAlpha() )
        {
            if( aBmp.GetBitCount() != 24 )
                aBmp.Convert( BMP_CONVERSION_24BIT );
            AlphaMask aMask;
            if( ! aIcon.IsAlpha() )
            {
                switch( aIcon.GetTransparentType() )
                {
                    case TRANSPARENT_NONE:
                    {
                        BYTE nTrans = 0;
                        aMask = AlphaMask( aBmp.GetSizePixel(), &nTrans );
                    }
                    break;
                    case TRANSPARENT_COLOR:
                        aMask = AlphaMask( aBmp.CreateMask( aIcon.GetTransparentColor() ) );
                    break;
                    case TRANSPARENT_BITMAP:
                        aMask = AlphaMask( aIcon.GetMask() );
                    break;
                    default:
                        DBG_ERROR( "unhandled transparent type" );
                    break;
                }
            }
            else
                aMask = aIcon.GetAlpha();
            aIcon = BitmapEx( aBmp, aMask );
        }

        ImpBitmap *pIconImpBitmap = aIcon.ImplGetBitmapImpBitmap();
        ImpBitmap *pIconImpMask   = aIcon.ImplGetMaskImpBitmap();


        if( pIconImpBitmap && pIconImpMask )
        {
            SalBitmap *pIconBitmap =
                pIconImpBitmap->ImplGetSalBitmap();
            SalBitmap *pIconMask =
                pIconImpMask->ImplGetSalBitmap();

            if( ( pBuf = bitmapToPixbuf( pIconBitmap, pIconMask ) ) )
                pIcons = g_list_prepend( pIcons, pBuf );
        }
    }

    gtk_window_set_icon_list( GTK_WINDOW(m_pWindow), pIcons );

    g_list_foreach( pIcons, (GFunc) g_object_unref, NULL );
    g_list_free( pIcons );
}

void GtkSalFrame::SetMenu( SalMenu* )
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

        Size aScreenSize = GetX11SalData()->GetDisplay()->GetScreenSize( m_nScreen );
        nScreenWidth        = aScreenSize.Width();
        nScreenHeight       = aScreenSize.Height();
        if( GetX11SalData()->GetDisplay()->IsXinerama() )
        {
            // get xinerama screen we are on
            // if there is a parent, use its center for screen determination
            // else use the pointer
            GdkScreen* pScreen;
            gint x, y;
            GdkModifierType aMask;
            gdk_display_get_pointer( getGdkDisplay(), &pScreen, &x, &y, &aMask );

            const std::vector< Rectangle >& rScreens = GetX11SalData()->GetDisplay()->GetXineramaScreens();
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
    Size aScreenSize = GetX11SalData()->GetDisplay()->GetScreenSize( m_nScreen );
    long w = aScreenSize.Width();
    long h = aScreenSize.Height();

    // fill in holy default values brought to us by product management
    if( aScreenSize.Width() >= 800 )
        w = 785;
    if( aScreenSize.Width() >= 1024 )
        w = 920;

    if( aScreenSize.Height() >= 600 )
        h = 550;
    if( aScreenSize.Height() >= 768 )
        h = 630;
    if( aScreenSize.Height() >= 1024 )
        h = 875;

    return Size( w, h );
}

void GtkSalFrame::SetDefaultSize()
{
    Size aDefSize = calcDefaultSize();

    SetPosSize( 0, 0, aDefSize.Width(), aDefSize.Height(),
                SAL_FRAME_POSSIZE_WIDTH | SAL_FRAME_POSSIZE_HEIGHT );

    if( (m_nStyle & SAL_FRAME_STYLE_DEFAULT) && m_pWindow )
        gtk_window_maximize( GTK_WINDOW(m_pWindow) );
}

static void initClientId()
{
    static bool bOnce = false;
    if( ! bOnce )
    {
        bOnce = true;
        const ByteString& rID = SessionManagerClient::getSessionID();
        if( rID.Len() > 0 )
            gdk_set_sm_client_id(rID.GetBuffer());
    }
}

void GtkSalFrame::Show( BOOL bVisible, BOOL /*bNoActivate*/ )
{
    if( m_pWindow )
    {
        if( m_pParent && (m_pParent->m_nStyle & SAL_FRAME_STYLE_PARTIAL_FULLSCREEN) )
            gtk_window_set_keep_above( GTK_WINDOW(m_pWindow), bVisible );
        if( bVisible )
        {
            SessionManagerClient::open(); // will simply return after the first time
            initClientId();
            getDisplay()->startupNotificationCompleted();

            if( m_bDefaultPos )
                Center();
            if( m_bDefaultSize )
                SetDefaultSize();
            setMinMaxSize();

            // #i45160# switch to desktop where a dialog with parent will appear
            if( m_pParent && m_pParent->m_nWorkArea != m_nWorkArea )
                getDisplay()->getWMAdaptor()->switchToWorkArea( m_pParent->m_nWorkArea );

            if( isFloatGrabWindow() &&
                m_pParent &&
                m_nFloats == 0 &&
                ! getDisplay()->GetCaptureFrame() )
            {
                /* #i63086#
                 * outsmart Metacity's "focus:mouse" mode
                 * which insists on taking the focus from the document
                 * to the new float. Grab focus to parent frame BEFORE
                 * showing the float (cannot grab it to the float
                 * before show).
                 */
                 m_pParent->grabPointer( TRUE, TRUE );
            }
            gtk_widget_show( m_pWindow );
            if( isFloatGrabWindow() )
            {
                m_nFloats++;
                if( ! getDisplay()->GetCaptureFrame() && m_nFloats == 1 )
                    grabPointer( TRUE, TRUE );
                // #i44068# reset parent's IM context
                if( m_pParent )
                    m_pParent->EndExtTextInput(0);
            }
            if( m_bWindowIsGtkPlug )
                askForXEmbedFocus( 0 );
        }
        else
        {
            if( isFloatGrabWindow() )
            {
                m_nFloats--;
                if( ! getDisplay()->GetCaptureFrame() && m_nFloats == 0)
                    grabPointer( FALSE );
            }
            gtk_widget_hide( m_pWindow );
            if( m_pIMHandler )
                m_pIMHandler->focusChanged( false );
        }
        CallCallback( SALEVENT_RESIZE, NULL );
    }
}

void GtkSalFrame::Enable( BOOL /*bEnable*/ )
{
    // Not implemented by X11SalFrame either
}

void GtkSalFrame::setMinMaxSize()
{
/*  FIXME: for yet unknown reasons the reported size is a little smaller
 *  than the max size hint; one would guess that this was due to the border
 *  sizes of the widgets involved (GtkWindow and GtkFixed), but setting the
 *  their border to 0 (which is the default anyway) does not change the
 *  behaviour. Until the reason is known we'll add some pixels here.
 */
#define CONTAINER_ADJUSTMENT 6

    /*  #i34504# metacity (and possibly others) do not treat
     *  _NET_WM_STATE_FULLSCREEN and max_width/heigth independently;
     *  whether they should is undefined. So don't set the max size hint
     *  for a full screen window.
    */
    if( m_pWindow && ! isChild() )
    {
        GdkGeometry aGeo;
        int aHints = 0;
        if( m_nStyle & SAL_FRAME_STYLE_SIZEABLE )
        {
            if( m_aMinSize.Width() && m_aMinSize.Height() )
            {
                aGeo.min_width  = m_aMinSize.Width()+CONTAINER_ADJUSTMENT;
                aGeo.min_height = m_aMinSize.Height()+CONTAINER_ADJUSTMENT;
                aHints |= GDK_HINT_MIN_SIZE;
            }
            if( m_aMaxSize.Width() && m_aMaxSize.Height() && ! m_bFullscreen )
            {
                aGeo.max_width  = m_aMaxSize.Width()+CONTAINER_ADJUSTMENT;
                aGeo.max_height = m_aMaxSize.Height()+CONTAINER_ADJUSTMENT;
                aHints |= GDK_HINT_MAX_SIZE;
            }
        }
        else
        {
            aGeo.min_width = maGeometry.nWidth;
            aGeo.min_height = maGeometry.nHeight;
            aHints |= GDK_HINT_MIN_SIZE;
            if( ! m_bFullscreen )
            {
                aGeo.max_width = maGeometry.nWidth;
                aGeo.max_height = maGeometry.nHeight;
                aHints |= GDK_HINT_MAX_SIZE;
            }
        }
        if( aHints )
            gtk_window_set_geometry_hints( GTK_WINDOW(m_pWindow),
                                           NULL,
                                           &aGeo,
                                           GdkWindowHints( aHints ) );
    }
}

void GtkSalFrame::SetMaxClientSize( long nWidth, long nHeight )
{
    if( ! isChild() )
    {
        m_aMaxSize = Size( nWidth, nHeight );
        // Show does a setMinMaxSize
        if( GTK_WIDGET_MAPPED( m_pWindow ) )
            setMinMaxSize();
    }
}
void GtkSalFrame::SetMinClientSize( long nWidth, long nHeight )
{
    if( ! isChild() )
    {
        m_aMinSize = Size( nWidth, nHeight );
        if( m_pWindow )
        {
            gtk_widget_set_size_request( m_pWindow, nWidth, nHeight );
            // Show does a setMinMaxSize
            if( GTK_WIDGET_MAPPED( m_pWindow ) )
                setMinMaxSize();
        }
    }
}

void GtkSalFrame::SetPosSize( long nX, long nY, long nWidth, long nHeight, USHORT nFlags )
{
    if( !m_pWindow || isChild( true, false ) )
        return;

    bool bSized = false, bMoved = false;

    if( (nFlags & ( SAL_FRAME_POSSIZE_WIDTH | SAL_FRAME_POSSIZE_HEIGHT )) &&
        (nWidth > 0 && nHeight > 0 ) // sometimes stupid things happen
            )
    {
        m_bDefaultSize = false;

        if( (unsigned long)nWidth != maGeometry.nWidth || (unsigned long)nHeight != maGeometry.nHeight )
            bSized = true;
        maGeometry.nWidth   = nWidth;
        maGeometry.nHeight  = nHeight;

        if( isChild( false, true ) )
            gtk_widget_set_size_request( m_pWindow, nWidth, nHeight );
        else
            gtk_window_resize( GTK_WINDOW(m_pWindow), nWidth, nHeight );
        setMinMaxSize();
    }
    else if( m_bDefaultSize )
        SetDefaultSize();

    m_bDefaultSize = false;

    if( nFlags & ( SAL_FRAME_POSSIZE_X | SAL_FRAME_POSSIZE_Y ) )
    {
        if( m_pParent )
        {
            if( Application::GetSettings().GetLayoutRTL() )
                nX = m_pParent->maGeometry.nWidth-maGeometry.nWidth-1-nX;
            nX += m_pParent->maGeometry.nX;
            nY += m_pParent->maGeometry.nY;
        }

        // adjust position to avoid off screen windows
        // but allow toolbars to be positioned partly off screen by the user
        Size aScreenSize = GetX11SalData()->GetDisplay()->GetScreenSize( m_nScreen );
        if( ! (m_nStyle & SAL_FRAME_STYLE_OWNERDRAWDECORATION) )
        {
            if( nX < (long)maGeometry.nLeftDecoration )
                nX = maGeometry.nLeftDecoration;
            if( nY < (long)maGeometry.nTopDecoration )
                nY = maGeometry.nTopDecoration;
            if( (nX + (long)maGeometry.nWidth + (long)maGeometry.nRightDecoration) > (long)aScreenSize.Width() )
                nX = aScreenSize.Width() - maGeometry.nWidth - maGeometry.nRightDecoration;
            if( (nY + (long)maGeometry.nHeight + (long)maGeometry.nBottomDecoration) > (long)aScreenSize.Height() )
                nY = aScreenSize.Height() - maGeometry.nHeight - maGeometry.nBottomDecoration;
        }
        else
        {
            if( nX + (long)maGeometry.nWidth < 10 )
                nX = 10 - (long)maGeometry.nWidth;
            if( nY + (long)maGeometry.nHeight < 10 )
                nY = 10 - (long)maGeometry.nHeight;
            if( nX > (long)aScreenSize.Width() - 10 )
                nX = (long)aScreenSize.Width() - 10;
            if( nY > (long)aScreenSize.Height() - 10 )
                nY = (long)aScreenSize.Height() - 10;
        }

        if( nX != maGeometry.nX || nY != maGeometry.nY )
            bMoved = true;
        maGeometry.nX = nX;
        maGeometry.nY = nY;

        m_bDefaultPos = false;

        moveWindow( maGeometry.nX, maGeometry.nY );

        updateScreenNumber();
    }
    else if( m_bDefaultPos )
        Center();

    m_bDefaultPos = false;

    if( bSized && ! bMoved )
        CallCallback( SALEVENT_RESIZE, NULL );
    else if( bMoved && ! bSized )
        CallCallback( SALEVENT_MOVE, NULL );
    else if( bMoved && bSized )
        CallCallback( SALEVENT_MOVERESIZE, NULL );
}

void GtkSalFrame::GetClientSize( long& rWidth, long& rHeight )
{
    if( m_pWindow && !(m_nState & GDK_WINDOW_STATE_ICONIFIED) )
    {
        rWidth = maGeometry.nWidth;
        rHeight = maGeometry.nHeight;
    }
    else
        rWidth = rHeight = 0;
}

void GtkSalFrame::GetWorkArea( Rectangle& rRect )
{
    rRect = GetX11SalData()->GetDisplay()->getWMAdaptor()->getWorkArea( 0 );
}

SalFrame* GtkSalFrame::GetParent() const
{
    return m_pParent;
}

void GtkSalFrame::SetWindowState( const SalFrameState* pState )
{
    if( ! m_pWindow || ! pState || isChild( true, false ) )
        return;

    const ULONG nMaxGeometryMask =
        SAL_FRAMESTATE_MASK_X | SAL_FRAMESTATE_MASK_Y |
        SAL_FRAMESTATE_MASK_WIDTH | SAL_FRAMESTATE_MASK_HEIGHT |
        SAL_FRAMESTATE_MASK_MAXIMIZED_X | SAL_FRAMESTATE_MASK_MAXIMIZED_Y |
        SAL_FRAMESTATE_MASK_MAXIMIZED_WIDTH | SAL_FRAMESTATE_MASK_MAXIMIZED_HEIGHT;

    if( (pState->mnMask & SAL_FRAMESTATE_MASK_STATE) &&
        (pState->mnState & SAL_FRAMESTATE_MAXIMIZED) &&
        (pState->mnMask & nMaxGeometryMask) == nMaxGeometryMask )
    {
        resizeWindow( pState->mnWidth, pState->mnHeight );
        moveWindow( pState->mnX, pState->mnY );
        m_bDefaultPos = m_bDefaultSize = false;

        maGeometry.nX       = pState->mnMaximizedX;
        maGeometry.nY       = pState->mnMaximizedY;
        maGeometry.nWidth   = pState->mnMaximizedWidth;
        maGeometry.nHeight  = pState->mnMaximizedHeight;
        updateScreenNumber();

        m_nState = GdkWindowState( m_nState | GDK_WINDOW_STATE_MAXIMIZED );
        m_aRestorePosSize = Rectangle( Point( pState->mnX, pState->mnY ),
                                       Size( pState->mnWidth, pState->mnHeight ) );
    }
    else if( pState->mnMask & (SAL_FRAMESTATE_MASK_X | SAL_FRAMESTATE_MASK_Y |
                               SAL_FRAMESTATE_MASK_WIDTH | SAL_FRAMESTATE_MASK_HEIGHT ) )
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
    if( pState->mnMask & SAL_FRAMESTATE_MASK_STATE && ! isChild() )
    {
        if( pState->mnState & SAL_FRAMESTATE_MAXIMIZED )
            gtk_window_maximize( GTK_WINDOW(m_pWindow) );
        else
            gtk_window_unmaximize( GTK_WINDOW(m_pWindow) );
        /* #i42379# there is no rollup state in GDK; and rolled up windows are
        *  (probably depending on the WM) reported as iconified. If we iconify a
        *  window here that was e.g. a dialog, then it will be unmapped but still
        *  not be displayed in the task list, so it's an iconified window that
        *  the user cannot get out of this state. So do not set the iconified state
        *  on windows with a parent (that is transient frames) since these tend
        *  to not be represented in an icon task list.
        */
        if( (pState->mnState & SAL_FRAMESTATE_MINIMIZED)
            && ! m_pParent )
            gtk_window_iconify( GTK_WINDOW(m_pWindow) );
        else
            gtk_window_deiconify( GTK_WINDOW(m_pWindow) );
    }
}

BOOL GtkSalFrame::GetWindowState( SalFrameState* pState )
{
    pState->mnState = SAL_FRAMESTATE_NORMAL;
    pState->mnMask  = SAL_FRAMESTATE_MASK_STATE;
    // rollup ? gtk 2.2 does not seem to support the shaded state
    if( (m_nState & GDK_WINDOW_STATE_ICONIFIED) )
        pState->mnState |= SAL_FRAMESTATE_MINIMIZED;
    if( m_nState & GDK_WINDOW_STATE_MAXIMIZED )
    {
        pState->mnState |= SAL_FRAMESTATE_MAXIMIZED;
        pState->mnX                 = m_aRestorePosSize.Left();
        pState->mnY                 = m_aRestorePosSize.Top();
        pState->mnWidth             = m_aRestorePosSize.GetWidth();
        pState->mnHeight            = m_aRestorePosSize.GetHeight();
        pState->mnMaximizedX        = maGeometry.nX;
        pState->mnMaximizedY        = maGeometry.nY;
        pState->mnMaximizedWidth    = maGeometry.nWidth;
        pState->mnMaximizedHeight   = maGeometry.nHeight;
        pState->mnMask  |= SAL_FRAMESTATE_MASK_MAXIMIZED_X          |
                           SAL_FRAMESTATE_MASK_MAXIMIZED_Y          |
                           SAL_FRAMESTATE_MASK_MAXIMIZED_WIDTH      |
                           SAL_FRAMESTATE_MASK_MAXIMIZED_HEIGHT;
    }
    else
    {

        pState->mnX         = maGeometry.nX;
        pState->mnY         = maGeometry.nY;
        pState->mnWidth     = maGeometry.nWidth;
        pState->mnHeight    = maGeometry.nHeight;
    }
    pState->mnMask  |= SAL_FRAMESTATE_MASK_X            |
                       SAL_FRAMESTATE_MASK_Y            |
                       SAL_FRAMESTATE_MASK_WIDTH        |
                       SAL_FRAMESTATE_MASK_HEIGHT;

    return TRUE;
}

void GtkSalFrame::moveToScreen( int nScreen )
{
    if( isChild() )
        return;

    if( nScreen < 0 || nScreen >= gdk_display_get_n_screens( getGdkDisplay() ) )
        nScreen = m_nScreen;
    if( nScreen == m_nScreen )
        return;

    GdkScreen* pScreen = gdk_display_get_screen( getGdkDisplay(), nScreen );
    if( pScreen )
    {
        m_nScreen = nScreen;
        gtk_window_set_screen( GTK_WINDOW(m_pWindow), pScreen );
        // realize the window, we need an XWindow id
        gtk_widget_realize( m_pWindow );
        // update system data
        GtkSalDisplay* pDisp = getDisplay();
        m_aSystemData.aWindow       = GDK_WINDOW_XWINDOW(m_pWindow->window);
        m_aSystemData.pVisual       = pDisp->GetVisual( m_nScreen ).GetVisual();
        m_aSystemData.nDepth        = pDisp->GetVisual( m_nScreen ).GetDepth();
        m_aSystemData.aColormap     = pDisp->GetColormap( m_nScreen ).GetXColormap();
        m_aSystemData.pAppContext   = NULL;
        m_aSystemData.aShellWindow  = m_aSystemData.aWindow;
        // update graphics if necessary
        for( unsigned int i = 0; i < sizeof(m_aGraphics)/sizeof(m_aGraphics[0]); i++ )
        {
            if( m_aGraphics[i].bInUse )
                m_aGraphics[i].pGraphics->SetDrawable( GDK_WINDOW_XWINDOW(m_pWindow->window), m_nScreen );
        }
        updateScreenNumber();
    }

    if( m_pParent && m_pParent->m_nScreen != m_nScreen )
        SetParent( NULL );
    std::list< GtkSalFrame* > aChildren = m_aChildren;
    for( std::list< GtkSalFrame* >::iterator it = aChildren.begin(); it != aChildren.end(); ++it )
        (*it)->moveToScreen( m_nScreen );

    // FIXME: SalObjects
}

void GtkSalFrame::SetScreenNumber( unsigned int nNewScreen )
{
    if( nNewScreen == maGeometry.nScreenNumber )
        return;

    if( m_pWindow && ! isChild() )
    {
        GtkSalDisplay* pDisp = getDisplay();
        if( pDisp->IsXinerama() && pDisp->GetXineramaScreens().size() > 1 )
        {
            if( nNewScreen >= pDisp->GetXineramaScreens().size() )
                return;

            Rectangle aOldScreenRect( pDisp->GetXineramaScreens()[maGeometry.nScreenNumber] );
            Rectangle aNewScreenRect( pDisp->GetXineramaScreens()[nNewScreen] );
            bool bVisible = GTK_WIDGET_MAPPED(m_pWindow);
            if( bVisible )
                Show( FALSE );
            maGeometry.nX = aNewScreenRect.Left() + (maGeometry.nX - aOldScreenRect.Left());
            maGeometry.nY = aNewScreenRect.Top() + (maGeometry.nY - aOldScreenRect.Top());
            createNewWindow( None, false, m_nScreen );
            gtk_window_move( GTK_WINDOW(m_pWindow), maGeometry.nX, maGeometry.nY );
            if( bVisible )
                Show( TRUE );
            maGeometry.nScreenNumber = nNewScreen;
        }
        else if( sal_Int32(nNewScreen) < pDisp->GetScreenCount() )
        {
            moveToScreen( (int)nNewScreen );
            maGeometry.nScreenNumber = nNewScreen;
            gtk_window_move( GTK_WINDOW(m_pWindow), maGeometry.nX, maGeometry.nY );
        }
    }
}

void GtkSalFrame::ShowFullScreen( BOOL bFullScreen, sal_Int32 nScreen )
{
    if( m_pWindow && ! isChild() )
    {
        GtkSalDisplay* pDisp = getDisplay();
        // xinerama ?
        if( pDisp->IsXinerama() && pDisp->GetXineramaScreens().size() > 1 )
        {
            if( bFullScreen )
            {
                m_aRestorePosSize = Rectangle( Point( maGeometry.nX, maGeometry.nY ),
                                               Size( maGeometry.nWidth, maGeometry.nHeight ) );
                // workaround different window managers have different opinions about
                // _NET_WM_STATE_FULLSCREEN (Metacity <-> KWin)
                bool bVisible = GTK_WIDGET_MAPPED(m_pWindow);
                if( bVisible )
                    Show( FALSE );
                m_nStyle |= SAL_FRAME_STYLE_PARTIAL_FULLSCREEN;
                createNewWindow( None, false, m_nScreen );
                Rectangle aNewPosSize;
                if( nScreen < 0 || nScreen >= static_cast<int>(pDisp->GetXineramaScreens().size()) )
                    aNewPosSize = Rectangle( Point( 0, 0 ), pDisp->GetScreenSize(m_nScreen) );
                else
                    aNewPosSize = pDisp->GetXineramaScreens()[ nScreen ];
                gtk_window_resize( GTK_WINDOW(m_pWindow),
                                   maGeometry.nWidth = aNewPosSize.GetWidth(),
                                   maGeometry.nHeight = aNewPosSize.GetHeight() );
                gtk_window_move( GTK_WINDOW(m_pWindow),
                                 maGeometry.nX = aNewPosSize.Left(),
                                 maGeometry.nY = aNewPosSize.Top() );
                if( bVisible )
                    Show( TRUE );
            }
            else
            {
                bool bVisible = GTK_WIDGET_MAPPED(m_pWindow);
                if( bVisible )
                    Show( FALSE );
                m_nStyle &= ~SAL_FRAME_STYLE_PARTIAL_FULLSCREEN;
                createNewWindow( None, false, m_nScreen );
                if( ! m_aRestorePosSize.IsEmpty() )
                {
                    gtk_window_resize( GTK_WINDOW(m_pWindow),
                                       maGeometry.nWidth = m_aRestorePosSize.GetWidth(),
                                       maGeometry.nHeight = m_aRestorePosSize.GetHeight() );
                    gtk_window_move( GTK_WINDOW(m_pWindow),
                                     maGeometry.nX = m_aRestorePosSize.Left(),
                                     maGeometry.nY = m_aRestorePosSize.Top() );
                    m_aRestorePosSize = Rectangle();
                }
                if( bVisible )
                    Show( TRUE );
            }
        }
        else
        {
            if( bFullScreen )
            {
                if( !(m_nStyle & SAL_FRAME_STYLE_SIZEABLE) )
                    gtk_window_set_resizable( GTK_WINDOW(m_pWindow), TRUE );
                gtk_window_fullscreen( GTK_WINDOW(m_pWindow) );
                moveToScreen( nScreen );
                Size aScreenSize = pDisp->GetScreenSize( m_nScreen );
                maGeometry.nX       = 0;
                maGeometry.nY       = 0;
                maGeometry.nWidth   = aScreenSize.Width();
                maGeometry.nHeight  = aScreenSize.Height();
            }
            else
            {
                gtk_window_unfullscreen( GTK_WINDOW(m_pWindow) );
                if( !(m_nStyle & SAL_FRAME_STYLE_SIZEABLE) )
                    gtk_window_set_resizable( GTK_WINDOW(m_pWindow), FALSE );
                moveToScreen( nScreen );
            }
        }
        m_bDefaultPos = m_bDefaultSize = false;
        updateScreenNumber();
        CallCallback( SALEVENT_MOVERESIZE, NULL );
    }
    m_bFullscreen = bFullScreen;
}

/* definitions from xautolock.c (pl15) */
#define XAUTOLOCK_DISABLE 1
#define XAUTOLOCK_ENABLE  2

void GtkSalFrame::setAutoLock( bool bLock )
{
    if( isChild() )
        return;

    GdkScreen  *pScreen = gtk_window_get_screen( GTK_WINDOW(m_pWindow) );
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

#ifdef ENABLE_DBUS
/** cookie is returned as an unsigned integer */
static guint
dbus_inhibit_gss (const gchar *appname,
                  const gchar *reason)
{
        gboolean         res;
        guint            cookie;
        GError          *error = NULL;
        DBusGProxy      *proxy = NULL;
        DBusGConnection *session_connection = NULL;

        /* get the DBUS session connection */
        session_connection = dbus_g_bus_get (DBUS_BUS_SESSION, &error);
        if (error != NULL) {
                g_warning ("DBUS cannot connect : %s", error->message);
                g_error_free (error);
                return -1;
        }

        /* get the proxy with gnome-screensaver */
        proxy = dbus_g_proxy_new_for_name (session_connection,
                                           GSS_DBUS_SERVICE,
                                           GSS_DBUS_PATH,
                                           GSS_DBUS_INTERFACE);
        if (proxy == NULL) {
                g_warning ("Could not get DBUS proxy: %s", GSS_DBUS_SERVICE);
                return -1;
        }

        res = dbus_g_proxy_call (proxy,
                                 "Inhibit", &error,
                                 G_TYPE_STRING, appname,
                                 G_TYPE_STRING, reason,
                                 G_TYPE_INVALID,
                                 G_TYPE_UINT, &cookie,
                                 G_TYPE_INVALID);

        /* check the return value */
        if (! res) {
                cookie = -1;
                g_warning ("Inhibit method failed");
        }

        /* check the error value */
        if (error != NULL) {
                g_warning ("Inhibit problem : %s", error->message);
                g_error_free (error);
                cookie = -1;
        }

        g_object_unref (G_OBJECT (proxy));
        return cookie;
}

static void
dbus_uninhibit_gss (guint cookie)
{
        gboolean         res;
        GError          *error = NULL;
        DBusGProxy      *proxy = NULL;
        DBusGConnection *session_connection = NULL;

        /* cookies have to be positive as unsigned */
        if (cookie < 0) {
                g_warning ("Invalid cookie");
                return;
        }

        /* get the DBUS session connection */
        session_connection = dbus_g_bus_get (DBUS_BUS_SESSION, &error);
        if (error) {
                g_warning ("DBUS cannot connect : %s", error->message);
                g_error_free (error);
                return;
        }

        /* get the proxy with gnome-screensaver */
        proxy = dbus_g_proxy_new_for_name (session_connection,
                                           GSS_DBUS_SERVICE,
                                           GSS_DBUS_PATH,
                                           GSS_DBUS_INTERFACE);
        if (proxy == NULL) {
                g_warning ("Could not get DBUS proxy: %s", GSS_DBUS_SERVICE);
                return;
        }

        res = dbus_g_proxy_call (proxy,
                                 "UnInhibit",
                                 &error,
                                 G_TYPE_UINT, cookie,
                                 G_TYPE_INVALID,
                                 G_TYPE_INVALID);

        /* check the return value */
        if (! res) {
                g_warning ("UnInhibit method failed");
        }

        /* check the error value */
        if (error != NULL) {
                g_warning ("Inhibit problem : %s", error->message);
                g_error_free (error);
                cookie = -1;
        }
        g_object_unref (G_OBJECT (proxy));
}
#endif

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
#ifdef ENABLE_DBUS
        m_nGSSCookie = dbus_inhibit_gss(g_get_application_name(), "presentation");
#endif
    }
    else
    {
        if( m_nSavedScreenSaverTimeout )
            XSetScreenSaver( pDisplay, m_nSavedScreenSaverTimeout,
                             nInterval, bPreferBlanking,
                             bAllowExposures );
        m_nSavedScreenSaverTimeout = 0;
#ifdef ENABLE_DBUS
        dbus_uninhibit_gss(m_nGSSCookie);
#endif
    }
}

void GtkSalFrame::SetAlwaysOnTop( BOOL /*bOnTop*/ )
{
}

void GtkSalFrame::ToTop( USHORT nFlags )
{
    if( m_pWindow )
    {
        if( isChild( false, true ) )
            gtk_widget_grab_focus( m_pWindow );
        else if( GTK_WIDGET_MAPPED( m_pWindow ) )
        {
            if( ! (nFlags & SAL_FRAME_TOTOP_GRABFOCUS_ONLY) )
                gtk_window_present( GTK_WINDOW(m_pWindow) );
            else
                gdk_window_focus( m_pWindow->window, GDK_CURRENT_TIME );
            /*  need to do an XSetInputFocus here because
             *  gdk_window_focus will ask a EWMH compliant WM to put the focus
             *  to our window - which it of course won't since our input hint
             *  is set to false.
             */
            if( (m_nStyle & SAL_FRAME_STYLE_OWNERDRAWDECORATION) )
                XSetInputFocus( getDisplay()->GetDisplay(), GDK_WINDOW_XWINDOW( m_pWindow->window ), RevertToParent, CurrentTime );
        }
        else
        {
            if( nFlags & SAL_FRAME_TOTOP_RESTOREWHENMIN )
                gtk_window_present( GTK_WINDOW(m_pWindow) );
        }
    }
}

void GtkSalFrame::SetPointer( PointerStyle ePointerStyle )
{
    if( m_pWindow && ePointerStyle != m_ePointerStyle )
    {
        m_ePointerStyle = ePointerStyle;
        GdkCursor *pCursor = getDisplay()->getCursor( ePointerStyle );
        gdk_window_set_cursor( m_pWindow->window, pCursor );
        m_pCurrentCursor = pCursor;

        // #i80791# use grabPointer the same way as CaptureMouse, respective float grab
        if( getDisplay()->MouseCaptured( this ) )
            grabPointer( TRUE, FALSE );
        else if( m_nFloats > 0 )
            grabPointer( TRUE, TRUE );
    }
}

void GtkSalFrame::grabPointer( BOOL bGrab, BOOL bOwnerEvents )
{
    if( m_pWindow )
    {
        if( bGrab )
        {
            bool bUseGdkGrab = true;
            if( getDisplay()->getHaveSystemChildFrame() )
            {
                const std::list< SalFrame* >& rFrames = getDisplay()->getFrames();
                for( std::list< SalFrame* >::const_iterator it = rFrames.begin(); it != rFrames.end(); ++it )
                {
                    const GtkSalFrame* pFrame = static_cast< const GtkSalFrame* >(*it);
                    if( pFrame->m_bWindowIsGtkPlug )
                    {
                        bUseGdkGrab = false;
                        break;
                    }
                }
            }
            if( bUseGdkGrab )
            {
                const int nMask = ( GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK );

                gdk_pointer_grab( m_pWindow->window, bOwnerEvents,
                                  (GdkEventMask) nMask, NULL, m_pCurrentCursor,
                                  GDK_CURRENT_TIME );
            }
            else
            {
                // FIXME: for some unknown reason gdk_pointer_grab does not
                // really produce owner events for GtkPlug windows
                // the cause is yet unknown
                //
                // this is of course a bad hack, especially as we cannot
                // set the right cursor this way
                XGrabPointer( getDisplay()->GetDisplay(),
                              GDK_WINDOW_XWINDOW( m_pWindow->window),
                              bOwnerEvents,
                              PointerMotionMask | ButtonPressMask | ButtonReleaseMask,
                              GrabModeAsync,
                              GrabModeAsync,
                              None,
                              None,
                              CurrentTime
                              );

            }
        }
        else
        {
            // Two GdkDisplays may be open
            gdk_display_pointer_ungrab( getGdkDisplay(), GDK_CURRENT_TIME);
        }
    }
}

void GtkSalFrame::CaptureMouse( BOOL bCapture )
{
    getDisplay()->CaptureMouse( bCapture ? this : NULL );
}

void GtkSalFrame::SetPointerPos( long nX, long nY )
{
    GtkSalFrame* pFrame = this;
    while( pFrame && pFrame->isChild( false, true ) )
        pFrame = pFrame->m_pParent;
    if( ! pFrame )
        return;

    GdkScreen *pScreen = gtk_window_get_screen( GTK_WINDOW(pFrame->m_pWindow) );
    GdkDisplay *pDisplay = gdk_screen_get_display( pScreen );

    /* #87921# when the application tries to center the mouse in the dialog the
     * window isn't mapped already. So use coordinates relative to the root window.
     */
    unsigned int nWindowLeft = maGeometry.nX + nX;
    unsigned int nWindowTop  = maGeometry.nY + nY;

    XWarpPointer( GDK_DISPLAY_XDISPLAY (pDisplay), None,
                  GDK_WINDOW_XID (gdk_screen_get_root_window( pScreen ) ),
                  0, 0, 0, 0, nWindowLeft, nWindowTop);
    // #i38648# ask for the next motion hint
    gint x, y;
    GdkModifierType mask;
    gdk_window_get_pointer( pFrame->m_pWindow->window, &x, &y, &mask );
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
    return static_cast<GtkSalDisplay*>(GetX11SalData()->GetDisplay())->GetGdkDisplay();
}

GtkSalDisplay *GtkSalFrame::getDisplay()
{
    return static_cast<GtkSalDisplay*>(GetX11SalData()->GetDisplay());
}

SalFrame::SalPointerState GtkSalFrame::GetPointerState()
{
    SalPointerState aState;
    GdkScreen* pScreen;
    gint x, y;
    GdkModifierType aMask;
    gdk_display_get_pointer( getGdkDisplay(), &pScreen, &x, &y, &aMask );
    aState.maPos = Point( x - maGeometry.nX, y - maGeometry.nY );
    aState.mnState = GetMouseModCode( aMask );
    return aState;
}

void GtkSalFrame::SetInputContext( SalInputContext* pContext )
{
    if( ! pContext )
        return;

    if( ! (pContext->mnOptions & SAL_INPUTCONTEXT_TEXT) )
        return;

    // create a new im context
    if( ! m_pIMHandler )
        m_pIMHandler = new IMHandler( this );
    m_pIMHandler->setInputContext( pContext );
}

void GtkSalFrame::EndExtTextInput( USHORT nFlags )
{
    if( m_pIMHandler )
        m_pIMHandler->endExtTextInput( nFlags );
}

BOOL GtkSalFrame::MapUnicodeToKeyCode( sal_Unicode , LanguageType , KeyCode& )
{
    // not supported yet
    return FALSE;
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
    GdkWindow *pWin = m_pWindow->window;
    if( pBmp->SnapShot( GDK_DISPLAY_XDISPLAY( getGdkDisplay() ),
                        GDK_WINDOW_XID( pWin ) ) )
        return pBmp;
    else
        delete pBmp;

    return NULL;
}

void GtkSalFrame::UpdateSettings( AllSettings& rSettings )
{
    if( ! m_pWindow )
        return;

    GtkSalGraphics* pGraphics = static_cast<GtkSalGraphics*>(m_aGraphics[0].pGraphics);
    bool bFreeGraphics = false;
    if( ! pGraphics )
    {
        pGraphics = static_cast<GtkSalGraphics*>(GetGraphics());
        bFreeGraphics = true;
    }

    pGraphics->updateSettings( rSettings );

    if( bFreeGraphics )
        ReleaseGraphics( pGraphics );
}

void GtkSalFrame::Beep( SoundType eType )
{
    switch( eType )
    {
        case SOUND_DEFAULT:
        case SOUND_ERROR:
            gdk_display_beep( getGdkDisplay() );
            break;
        default:
            break;
    }
}

const SystemEnvData* GtkSalFrame::GetSystemData() const
{
    return &m_aSystemData;
}

void GtkSalFrame::SetParent( SalFrame* pNewParent )
{
    if( m_pParent )
        m_pParent->m_aChildren.remove( this );
    m_pParent = static_cast<GtkSalFrame*>(pNewParent);
    if( m_pParent )
        m_pParent->m_aChildren.push_back( this );
    if( ! isChild() )
        gtk_window_set_transient_for( GTK_WINDOW(m_pWindow),
                                      (m_pParent && ! m_pParent->isChild(true,false)) ? GTK_WINDOW(m_pParent->m_pWindow) : NULL
                                     );
}

void GtkSalFrame::createNewWindow( XLIB_Window aNewParent, bool bXEmbed, int nScreen )
{
    bool bWasVisible = GTK_WIDGET_MAPPED(m_pWindow);
    if( bWasVisible )
        Show( FALSE );

    if( nScreen < 0 || nScreen >= getDisplay()->GetScreenCount() )
        nScreen = m_nScreen;

    SystemParentData aParentData;
    aParentData.aWindow = aNewParent;
    aParentData.bXEmbedSupport = bXEmbed;
    if( aNewParent == None )
    {
        aNewParent = getDisplay()->GetRootWindow(nScreen);
        aParentData.aWindow = None;
        aParentData.bXEmbedSupport = false;
    }
    else
    {
        // is new parent a root window ?
        Display* pDisp = getDisplay()->GetDisplay();
        int nScreens = getDisplay()->GetScreenCount();
        for( int i = 0; i < nScreens; i++ )
        {
            if( aNewParent == RootWindow( pDisp, i ) )
            {
                nScreen = i;
                aParentData.aWindow = None;
                aParentData.bXEmbedSupport = false;
                break;
            }
        }
    }

    // first deinit frame
    if( m_pIMHandler )
    {
        delete m_pIMHandler;
        m_pIMHandler = NULL;
    }
    if( m_pRegion )
        gdk_region_destroy( m_pRegion );
    if( m_pFixedContainer )
        gtk_widget_destroy( GTK_WIDGET(m_pFixedContainer) );
    if( m_pWindow )
        gtk_widget_destroy( m_pWindow );
    if( m_pForeignParent )
        g_object_unref( G_OBJECT(m_pForeignParent) );
    if( m_pForeignTopLevel )
        g_object_unref( G_OBJECT(m_pForeignTopLevel) );

    // init new window
    m_bDefaultPos = m_bDefaultSize = false;
    if( aParentData.aWindow != None )
    {
        m_nStyle |= SAL_FRAME_STYLE_PLUG;
        Init( &aParentData );
    }
    else
    {
        m_nStyle &= ~SAL_FRAME_STYLE_PLUG;
        Init( (m_pParent && m_pParent->m_nScreen == m_nScreen) ? m_pParent : NULL, m_nStyle );
    }

    // update graphics
    for( unsigned int i = 0; i < sizeof(m_aGraphics)/sizeof(m_aGraphics[0]); i++ )
    {
        if( m_aGraphics[i].bInUse )
        {
            m_aGraphics[i].pGraphics->SetDrawable( GDK_WINDOW_XWINDOW(m_pWindow->window), m_nScreen );
            m_aGraphics[i].pGraphics->SetWindow( m_pWindow );
        }
    }

    if( m_aTitle.Len() )
        SetTitle( m_aTitle );

    if( bWasVisible )
        Show( TRUE );

    std::list< GtkSalFrame* > aChildren = m_aChildren;
    m_aChildren.clear();
    for( std::list< GtkSalFrame* >::iterator it = aChildren.begin(); it != aChildren.end(); ++it )
        (*it)->createNewWindow( None, false, m_nScreen );

    // FIXME: SalObjects
}

bool GtkSalFrame::SetPluginParent( SystemParentData* pSysParent )
{
    if( pSysParent ) // this may be the first system child frame now
        getDisplay()->setHaveSystemChildFrame();
    createNewWindow( pSysParent->aWindow, (pSysParent->nSize > sizeof(long)) ? pSysParent->bXEmbedSupport : false, m_nScreen );
    return true;
}

void GtkSalFrame::ResetClipRegion()
{
    if( m_pWindow )
        gdk_window_shape_combine_region( m_pWindow->window, NULL, 0, 0 );
}

void GtkSalFrame::BeginSetClipRegion( ULONG )
{
    if( m_pRegion )
        gdk_region_destroy( m_pRegion );
    m_pRegion = gdk_region_new();
}

void GtkSalFrame::UnionClipRegion( long nX, long nY, long nWidth, long nHeight )
{
    if( m_pRegion )
    {
        GdkRectangle aRect;
        aRect.x         = nX;
        aRect.y         = nY;
        aRect.width     = nWidth;
        aRect.height    = nHeight;

        gdk_region_union_with_rect( m_pRegion, &aRect );
    }
}

void GtkSalFrame::EndSetClipRegion()
{
    if( m_pWindow && m_pRegion )
        gdk_window_shape_combine_region( m_pWindow->window, m_pRegion, 0, 0 );
}

bool GtkSalFrame::Dispatch( const XEvent* pEvent )
{
    bool bContinueDispatch = true;

    if( pEvent->type == PropertyNotify )
    {
        vcl_sal::WMAdaptor* pAdaptor = getDisplay()->getWMAdaptor();
        Atom nDesktopAtom = pAdaptor->getAtom( vcl_sal::WMAdaptor::NET_WM_DESKTOP );
        if( pEvent->xproperty.atom == nDesktopAtom &&
            pEvent->xproperty.state == PropertyNewValue )
        {
            m_nWorkArea = pAdaptor->getWindowWorkArea( GDK_WINDOW_XWINDOW( m_pWindow->window) );
        }
    }
    else if( pEvent->type == ConfigureNotify )
    {
        if( m_pForeignParent && pEvent->xconfigure.window == m_aForeignParentWindow )
        {
            bContinueDispatch = false;
            gtk_window_resize( GTK_WINDOW(m_pWindow), pEvent->xconfigure.width, pEvent->xconfigure.height );
            if( ( sal::static_int_cast< int >(maGeometry.nWidth) !=
                  pEvent->xconfigure.width ) ||
                ( sal::static_int_cast< int >(maGeometry.nHeight) !=
                  pEvent->xconfigure.height ) )
            {
                maGeometry.nWidth  = pEvent->xconfigure.width;
                maGeometry.nHeight = pEvent->xconfigure.height;
                setMinMaxSize();
                getDisplay()->SendInternalEvent( this, NULL, SALEVENT_RESIZE );
            }
        }
        else if( m_pForeignTopLevel && pEvent->xconfigure.window == m_aForeignTopLevelWindow )
        {
            bContinueDispatch = false;
            // update position
            int x = 0, y = 0;
            XLIB_Window aChild;
            XTranslateCoordinates( getDisplay()->GetDisplay(),
                                   GDK_WINDOW_XWINDOW( m_pWindow->window),
                                   getDisplay()->GetRootWindow( getDisplay()->GetDefaultScreenNumber() ),
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
    }
    else if( pEvent->type == ClientMessage &&
             pEvent->xclient.message_type == getDisplay()->getWMAdaptor()->getAtom( vcl_sal::WMAdaptor::XEMBED ) &&
             pEvent->xclient.window == GDK_WINDOW_XWINDOW(m_pWindow->window) &&
             m_bWindowIsGtkPlug
             )
    {
        // FIXME: this should not be necessary, GtkPlug should do this
        // transparently for us
        if( pEvent->xclient.data.l[1] == 1 || // XEMBED_WINDOW_ACTIVATE
            pEvent->xclient.data.l[1] == 2    // XEMBED_WINDOW_DEACTIVATE
        )
        {
            GdkEventFocus aEvent;
            aEvent.type = GDK_FOCUS_CHANGE;
            aEvent.window = m_pWindow->window;
            aEvent.send_event = TRUE;
            aEvent.in = (pEvent->xclient.data.l[1] == 1);
            signalFocus( m_pWindow, &aEvent, this );
        }
    }

    return bContinueDispatch;
}

void GtkSalFrame::SetBackgroundBitmap( SalBitmap* pBitmap )
{
    if( m_hBackgroundPixmap )
    {
        XSetWindowBackgroundPixmap( getDisplay()->GetDisplay(),
                                    GDK_WINDOW_XWINDOW(m_pWindow->window),
                                    None );
        XFreePixmap( getDisplay()->GetDisplay(), m_hBackgroundPixmap );
        m_hBackgroundPixmap = None;
    }
    if( pBitmap )
    {
        X11SalBitmap* pBM = static_cast<X11SalBitmap*>(pBitmap);
        Size aSize = pBM->GetSize();
        if( aSize.Width() && aSize.Height() )
        {
            m_hBackgroundPixmap =
                XCreatePixmap( getDisplay()->GetDisplay(),
                               GDK_WINDOW_XWINDOW(m_pWindow->window),
                               aSize.Width(),
                               aSize.Height(),
                               getDisplay()->GetVisual(m_nScreen).GetDepth() );
            if( m_hBackgroundPixmap )
            {
                SalTwoRect aTwoRect;
                aTwoRect.mnSrcX = aTwoRect.mnSrcY = aTwoRect.mnDestX = aTwoRect.mnDestY = 0;
                aTwoRect.mnSrcWidth = aTwoRect.mnDestWidth = aSize.Width();
                aTwoRect.mnSrcHeight = aTwoRect.mnDestHeight = aSize.Height();
                pBM->ImplDraw( m_hBackgroundPixmap,
                               m_nScreen,
                               getDisplay()->GetVisual(m_nScreen).GetDepth(),
                               aTwoRect,
                               getDisplay()->GetCopyGC(m_nScreen) );
                XSetWindowBackgroundPixmap( getDisplay()->GetDisplay(),
                                            GDK_WINDOW_XWINDOW(m_pWindow->window),
                                            m_hBackgroundPixmap );
            }
        }
    }
}

gboolean GtkSalFrame::signalButton( GtkWidget*, GdkEventButton* pEvent, gpointer frame )
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
        default: return FALSE;
    }
    aEvent.mnTime   = pEvent->time;
    aEvent.mnX      = (long)pEvent->x_root - pThis->maGeometry.nX;
    aEvent.mnY      = (long)pEvent->y_root - pThis->maGeometry.nY;
    aEvent.mnCode   = GetMouseModCode( pEvent->state );

    bool bClosePopups = false;
    if( pEvent->type == GDK_BUTTON_PRESS &&
        (pThis->m_nStyle & SAL_FRAME_STYLE_OWNERDRAWDECORATION) == 0
        )
    {
        if( m_nFloats > 0 )
        {
            // close popups if user clicks outside our application
            gint x, y;
            bClosePopups = (gdk_display_get_window_at_pointer( pThis->getGdkDisplay(), &x, &y ) == NULL);
        }
        /*  #i30306# release implicit pointer grab if no popups are open; else
         *  Drag cannot grab the pointer and will fail.
         */
        if( m_nFloats < 1 || bClosePopups )
            gdk_display_pointer_ungrab( pThis->getGdkDisplay(), GDK_CURRENT_TIME );
    }

    GTK_YIELD_GRAB();

    if( pThis->m_bWindowIsGtkPlug &&
        pEvent->type == GDK_BUTTON_PRESS &&
        pEvent->button == 1 )
    {
        pThis->askForXEmbedFocus( pEvent->time );
    }

    // --- RTL --- (mirror mouse pos)
    if( Application::GetSettings().GetLayoutRTL() )
        aEvent.mnX = pThis->maGeometry.nWidth-1-aEvent.mnX;

    vcl::DeletionListener aDel( pThis );

    pThis->CallCallback( nEventType, &aEvent );

    if( ! aDel.isDeleted() )
    {
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

        if( ! aDel.isDeleted() )
        {
            int frame_x = (int)(pEvent->x_root - pEvent->x);
            int frame_y = (int)(pEvent->y_root - pEvent->y);
            if( frame_x != pThis->maGeometry.nX || frame_y != pThis->maGeometry.nY )
            {
                pThis->maGeometry.nX = frame_x;
                pThis->maGeometry.nY = frame_y;
                pThis->CallCallback( SALEVENT_MOVE, NULL );
            }
        }
    }

    return FALSE;
}

gboolean GtkSalFrame::signalScroll( GtkWidget*, GdkEvent* pEvent, gpointer frame )
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
    aEvent.mnCode           = GetMouseModCode( pSEvent->state );
    aEvent.mbHorz           = (pSEvent->direction == GDK_SCROLL_LEFT || pSEvent->direction == GDK_SCROLL_RIGHT);

    GTK_YIELD_GRAB();

    // --- RTL --- (mirror mouse pos)
    if( Application::GetSettings().GetLayoutRTL() )
        aEvent.mnX = pThis->maGeometry.nWidth-1-aEvent.mnX;

    pThis->CallCallback( SALEVENT_WHEELMOUSE, &aEvent );

    return FALSE;
}

gboolean GtkSalFrame::signalMotion( GtkWidget*, GdkEventMotion* pEvent, gpointer frame )
{
    GtkSalFrame* pThis = (GtkSalFrame*)frame;

    SalMouseEvent aEvent;
    aEvent.mnTime   = pEvent->time;
    aEvent.mnX      = (long)pEvent->x_root - pThis->maGeometry.nX;
    aEvent.mnY      = (long)pEvent->y_root - pThis->maGeometry.nY;
    aEvent.mnCode   = GetMouseModCode( pEvent->state );
    aEvent.mnButton = 0;


    GTK_YIELD_GRAB();

    // --- RTL --- (mirror mouse pos)
    if( Application::GetSettings().GetLayoutRTL() )
        aEvent.mnX = pThis->maGeometry.nWidth-1-aEvent.mnX;

    vcl::DeletionListener aDel( pThis );

    pThis->CallCallback( SALEVENT_MOUSEMOVE, &aEvent );

    if( ! aDel.isDeleted() )
    {
        int frame_x = (int)(pEvent->x_root - pEvent->x);
        int frame_y = (int)(pEvent->y_root - pEvent->y);
        if( frame_x != pThis->maGeometry.nX || frame_y != pThis->maGeometry.nY )
        {
            pThis->maGeometry.nX = frame_x;
            pThis->maGeometry.nY = frame_y;
            pThis->CallCallback( SALEVENT_MOVE, NULL );
        }

        if( ! aDel.isDeleted() )
        {
            // ask for the next hint
            gint x, y;
            GdkModifierType mask;
            gdk_window_get_pointer( GTK_WIDGET(pThis->m_pWindow)->window, &x, &y, &mask );
        }
    }

    return TRUE;
}

gboolean GtkSalFrame::signalCrossing( GtkWidget*, GdkEventCrossing* pEvent, gpointer frame )
{
    GtkSalFrame* pThis = (GtkSalFrame*)frame;
    SalMouseEvent aEvent;
    aEvent.mnTime   = pEvent->time;
    aEvent.mnX      = (long)pEvent->x_root - pThis->maGeometry.nX;
    aEvent.mnY      = (long)pEvent->y_root - pThis->maGeometry.nY;
    aEvent.mnCode   = GetMouseModCode( pEvent->state );
    aEvent.mnButton = 0;

    GTK_YIELD_GRAB();
    pThis->CallCallback( (pEvent->type == GDK_ENTER_NOTIFY) ? SALEVENT_MOUSEMOVE : SALEVENT_MOUSELEAVE, &aEvent );

    return TRUE;
}


gboolean GtkSalFrame::signalExpose( GtkWidget*, GdkEventExpose* pEvent, gpointer frame )
{
    GtkSalFrame* pThis = (GtkSalFrame*)frame;

    struct SalPaintEvent aEvent( pEvent->area.x, pEvent->area.y, pEvent->area.width, pEvent->area.height );

    GTK_YIELD_GRAB();
    pThis->CallCallback( SALEVENT_PAINT, &aEvent );

    return FALSE;
}

gboolean GtkSalFrame::signalFocus( GtkWidget*, GdkEventFocus* pEvent, gpointer frame )
{
    GtkSalFrame* pThis = (GtkSalFrame*)frame;

    GTK_YIELD_GRAB();

    // check if printers have changed (analogous to salframe focus handler)
    vcl_sal::PrinterUpdate::update();

    if( !pEvent->in )
    {
        pThis->m_nKeyModifiers = 0;
        pThis->m_bSingleAltPress = false;
        pThis->m_bSendModChangeOnRelease = false;
    }

    if( pThis->m_pIMHandler )
        pThis->m_pIMHandler->focusChanged( pEvent->in );

    // ask for changed printers like generic implementation
    if( pEvent->in )
        if( static_cast< X11SalInstance* >(GetSalData()->m_pInstance)->isPrinterInit() )
            vcl_sal::PrinterUpdate::update();

    // FIXME: find out who the hell steals the focus from our frame
    // while we have the pointer grabbed, this should not come from
    // the window manager. Is this an event that was still queued ?
    // The focus does not seem to get set inside our process
    //
    // in the meantime do not propagate focus get/lose if floats are open
    if( m_nFloats == 0 )
        pThis->CallCallback( pEvent->in ? SALEVENT_GETFOCUS : SALEVENT_LOSEFOCUS, NULL );

    return FALSE;
}

gboolean GtkSalFrame::signalMap( GtkWidget*, GdkEvent*, gpointer frame )
{
    GtkSalFrame* pThis = (GtkSalFrame*)frame;

    GTK_YIELD_GRAB();

    if( GetX11SalData()->isTestTool() )
    {
        /* #i76541# testtool needs the focus to be in a new document
        *  however e.g. metacity does not necessarily put the focus into
        *  a newly shown window. An extra little hint seems to help here.
        *  however we don't want to interfere with the normal user experience
        *  so this is done when running in testtool only
        */
        if( ! pThis->m_pParent && (pThis->m_nStyle & SAL_FRAME_STYLE_MOVEABLE) != 0 )
            XSetInputFocus( pThis->getDisplay()->GetDisplay(),
                            GDK_WINDOW_XWINDOW( GTK_WIDGET(pThis->m_pWindow)->window),
                            RevertToParent, CurrentTime );
    }

    pThis->CallCallback( SALEVENT_RESIZE, NULL );

    return FALSE;
}

gboolean GtkSalFrame::signalUnmap( GtkWidget*, GdkEvent*, gpointer frame )
{
    GtkSalFrame* pThis = (GtkSalFrame*)frame;

    GTK_YIELD_GRAB();
    pThis->CallCallback( SALEVENT_RESIZE, NULL );

    return FALSE;
}

gboolean GtkSalFrame::signalConfigure( GtkWidget*, GdkEventConfigure* pEvent, gpointer frame )
{
    GtkSalFrame* pThis = (GtkSalFrame*)frame;

    bool bMoved = false, bSized = false;
    int x = pEvent->x, y = pEvent->y;

    /*  HACK: during sizing/moving a toolbar pThis->maGeometry is actually
     *  already exact; even worse: due to the asynchronicity of configure
     *  events the borderwindow which would evaluate this event
     *  would size/move based on wrong data if we would actually evaluate
     *  this event. So let's swallow it; this is also a performance
     *  improvement as one can omit the synchronous XTranslateCoordinates
     *  call below.
     */
    if( (pThis->m_nStyle & SAL_FRAME_STYLE_OWNERDRAWDECORATION) &&
        pThis->getDisplay()->GetCaptureFrame() == pThis )
        return FALSE;


    // in child case the coordinates are not root coordinates,
    // need to transform

    /* #i31785# sadly one cannot really trust the x,y members of the event;
     * they are e.g. not set correctly on maximize/demaximize; this rather
     * sounds like a bug in gtk we have to workaround.
     */
    XLIB_Window aChild;
    XTranslateCoordinates( pThis->getDisplay()->GetDisplay(),
                           GDK_WINDOW_XWINDOW(GTK_WIDGET(pThis->m_pWindow)->window),
                           pThis->getDisplay()->GetRootWindow( pThis->getDisplay()->GetDefaultScreenNumber() ),
                           0, 0,
                           &x, &y,
                           &aChild );

    if( x != pThis->maGeometry.nX || y != pThis->maGeometry.nY )
        bMoved = true;
    if( pEvent->width != (int)pThis->maGeometry.nWidth || pEvent->height != (int)pThis->maGeometry.nHeight )
        bSized = true;

    pThis->maGeometry.nX        = x;
    pThis->maGeometry.nY        = y;
    pThis->maGeometry.nWidth    = pEvent->width;
    pThis->maGeometry.nHeight   = pEvent->height;

    // update decoration hints
    if( ! (pThis->m_nStyle & SAL_FRAME_STYLE_PLUG) )
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
    pThis->updateScreenNumber();
    if( bMoved && bSized )
        pThis->CallCallback( SALEVENT_MOVERESIZE, NULL );
    else if( bMoved )
        pThis->CallCallback( SALEVENT_MOVE, NULL );
    else if( bSized )
        pThis->CallCallback( SALEVENT_RESIZE, NULL );

    return FALSE;
}

gboolean GtkSalFrame::signalKey( GtkWidget*, GdkEventKey* pEvent, gpointer frame )
{
    GtkSalFrame* pThis = (GtkSalFrame*)frame;

    vcl::DeletionListener aDel( pThis );

    if( pThis->m_pIMHandler )
    {
        if( pThis->m_pIMHandler->handleKeyEvent( pEvent ) )
        {
            pThis->m_bSingleAltPress = false;
            return TRUE;
        }
    }
    GTK_YIELD_GRAB();

    // handle modifiers
    if( pEvent->keyval == GDK_Shift_L || pEvent->keyval == GDK_Shift_R ||
        pEvent->keyval == GDK_Control_L || pEvent->keyval == GDK_Control_R ||
        pEvent->keyval == GDK_Alt_L || pEvent->keyval == GDK_Alt_R ||
        pEvent->keyval == GDK_Meta_L || pEvent->keyval == GDK_Meta_R )
    {
        SalKeyModEvent aModEvt;

        USHORT nModCode = GetKeyModCode( pEvent->state );

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
#ifdef MACOSX
            case GDK_Meta_L:   // map Meta (aka Command key) to Ctrl
#endif
            case GDK_Control_L:
                nExtModMask = MODKEY_LMOD1;
                nModMask = KEY_MOD1;
                break;
#ifdef MACOSX
            case GDK_Meta_R:   // map Meta (aka Command key) to Ctrl
#endif
            case GDK_Control_R:
                nExtModMask = MODKEY_RMOD1;
                nModMask = KEY_MOD1;
                break;
            case GDK_Alt_L:
                nExtModMask = MODKEY_LMOD2;
#ifdef MACOSX
                nModMask = KEY_MOD3;
#else
                nModMask = KEY_MOD2;
#endif
                break;
            case GDK_Alt_R:
                nExtModMask = MODKEY_RMOD2;
#ifdef MACOSX
                nModMask = KEY_MOD2 | (pEvent->type == GDK_KEY_RELEASE ? KEY_MOD3 : 0);
#else
                nModMask = KEY_MOD2;
#endif
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

        if( ! aDel.isDeleted() )
        {
            // emulate KEY_MENU
            if( ( pEvent->keyval == GDK_Alt_L || pEvent->keyval == GDK_Alt_R ) &&
                ( nModCode & ~(KEY_MOD3|KEY_MOD2)) == 0 )
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
                    if( ! aDel.isDeleted() )
                    {
                        pThis->CallCallback( SALEVENT_KEYUP, &aKeyEvt );
                        pThis->m_bSingleAltPress = false;
                    }
                }
            }
            else
                pThis->m_bSingleAltPress = false;
        }
    }
    else
    {
        pThis->doKeyCallback( pEvent->state,
                              pEvent->keyval,
                              pEvent->hardware_keycode,
                              pEvent->group,
                              pEvent->time,
                              sal_Unicode(gdk_keyval_to_unicode( pEvent->keyval )),
                              (pEvent->type == GDK_KEY_PRESS),
                              false );
        if( ! aDel.isDeleted() )
        {
            pThis->m_bSendModChangeOnRelease = false;
            pThis->m_bSingleAltPress = false;
        }
    }

    if( !aDel.isDeleted() && pThis->m_pIMHandler )
        pThis->m_pIMHandler->updateIMSpotLocation();

    return TRUE;
}

gboolean GtkSalFrame::signalDelete( GtkWidget*, GdkEvent*, gpointer frame )
{
    GtkSalFrame* pThis = (GtkSalFrame*)frame;

    GTK_YIELD_GRAB();
    pThis->CallCallback( SALEVENT_CLOSE, NULL );

    return TRUE;
}

void GtkSalFrame::signalStyleSet( GtkWidget*, GtkStyle* pPrevious, gpointer frame )
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

    /* #i64117# gtk sets a nice background pixmap
    *  but we actually don't really want that, so save
    *  some time on the Xserver as well as prevent
    *  some paint issues
    */
    GdkWindow* pWin = GTK_WIDGET(pThis->getWindow())->window;
    if( pWin )
    {
        XLIB_Window aWin = GDK_WINDOW_XWINDOW(pWin);
        if( aWin != None )
            XSetWindowBackgroundPixmap( pThis->getDisplay()->GetDisplay(),
                                        aWin,
                                        pThis->m_hBackgroundPixmap );
    }

    if( ! pThis->m_pParent )
    {
        // signalize theme changed for NWF caches
        // FIXME: should be called only once for a style change
        GtkSalGraphics::bThemeChanged = TRUE;
    }
}

gboolean GtkSalFrame::signalState( GtkWidget*, GdkEvent* pEvent, gpointer frame )
{
    GtkSalFrame* pThis = (GtkSalFrame*)frame;
    if( (pThis->m_nState & GDK_WINDOW_STATE_ICONIFIED) != (pEvent->window_state.new_window_state & GDK_WINDOW_STATE_ICONIFIED ) )
        pThis->getDisplay()->SendInternalEvent( pThis, NULL, SALEVENT_RESIZE );

    if(   (pEvent->window_state.new_window_state & GDK_WINDOW_STATE_MAXIMIZED) &&
        ! (pThis->m_nState & GDK_WINDOW_STATE_MAXIMIZED) )
    {
        pThis->m_aRestorePosSize =
            Rectangle( Point( pThis->maGeometry.nX, pThis->maGeometry.nY ),
                       Size( pThis->maGeometry.nWidth, pThis->maGeometry.nHeight ) );
    }
    pThis->m_nState = pEvent->window_state.new_window_state;

    return FALSE;
}

gboolean GtkSalFrame::signalVisibility( GtkWidget*, GdkEventVisibility* pEvent, gpointer frame )
{
    GtkSalFrame* pThis = (GtkSalFrame*)frame;
    pThis->m_nVisibility = pEvent->state;

    return FALSE;
}

void GtkSalFrame::signalDestroy( GtkObject* pObj, gpointer frame )
{
    GtkSalFrame* pThis = (GtkSalFrame*)frame;
    if( GTK_WIDGET( pObj ) == pThis->m_pWindow )
    {
        pThis->m_pFixedContainer = NULL;
        pThis->m_pWindow = NULL;
    }
}

// ----------------------------------------------------------------------
// GtkSalFrame::IMHandler
// ----------------------------------------------------------------------

GtkSalFrame::IMHandler::IMHandler( GtkSalFrame* pFrame )
: m_pFrame(pFrame),
  m_nPrevKeyPresses( 0 ),
  m_pIMContext( NULL ),
  m_bFocused( true )
{
    m_aInputEvent.mpTextAttr = NULL;
    createIMContext();
}

GtkSalFrame::IMHandler::~IMHandler()
{
    // cancel an eventual event posted to begin preedit again
    m_pFrame->getDisplay()->CancelInternalEvent( m_pFrame, &m_aInputEvent, SALEVENT_EXTTEXTINPUT );
    deleteIMContext();
}

void GtkSalFrame::IMHandler::createIMContext()
{
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
        g_signal_connect( m_pIMContext, "preedit_start",
                          G_CALLBACK (signalIMPreeditStart), this );
        g_signal_connect( m_pIMContext, "preedit_end",
                          G_CALLBACK (signalIMPreeditEnd), this );

        m_pFrame->getDisplay()->GetXLib()->PushXErrorLevel( true );
        gtk_im_context_set_client_window( m_pIMContext, GTK_WIDGET(m_pFrame->m_pWindow)->window );
        gtk_im_context_focus_in( m_pIMContext );
        m_pFrame->getDisplay()->GetXLib()->PopXErrorLevel();
        m_bFocused = true;
   }
}

void GtkSalFrame::IMHandler::deleteIMContext()
{
    if( m_pIMContext )
    {
        // first give IC a chance to deinitialize
        m_pFrame->getDisplay()->GetXLib()->PushXErrorLevel( true );
        gtk_im_context_set_client_window( m_pIMContext, NULL );
        m_pFrame->getDisplay()->GetXLib()->PopXErrorLevel();
        // destroy old IC
        g_object_unref( m_pIMContext );
        m_pIMContext = NULL;
    }
}

void GtkSalFrame::IMHandler::doCallEndExtTextInput()
{
    m_aInputEvent.mpTextAttr = NULL;
    m_pFrame->CallCallback( SALEVENT_ENDEXTTEXTINPUT, NULL );
}

void GtkSalFrame::IMHandler::updateIMSpotLocation()
{
    SalExtTextInputPosEvent aPosEvent;
    m_pFrame->CallCallback( SALEVENT_EXTTEXTINPUTPOS, (void*)&aPosEvent );
    GdkRectangle aArea;
    aArea.x = aPosEvent.mnX;
    aArea.y = aPosEvent.mnY;
    aArea.width = aPosEvent.mnWidth;
    aArea.height = aPosEvent.mnHeight;
    gtk_im_context_set_cursor_location( m_pIMContext, &aArea );
}

void GtkSalFrame::IMHandler::setInputContext( SalInputContext* )
{
}

void GtkSalFrame::IMHandler::sendEmptyCommit()
{
    vcl::DeletionListener aDel( m_pFrame );

    SalExtTextInputEvent aEmptyEv;
    aEmptyEv.mnTime             = 0;
    aEmptyEv.mpTextAttr         = 0;
    aEmptyEv.maText             = String();
    aEmptyEv.mnCursorPos        = 0;
    aEmptyEv.mnCursorFlags      = 0;
    aEmptyEv.mnDeltaStart       = 0;
    aEmptyEv.mbOnlyCursor       = False;
    m_pFrame->CallCallback( SALEVENT_EXTTEXTINPUT, (void*)&aEmptyEv );
    if( ! aDel.isDeleted() )
        m_pFrame->CallCallback( SALEVENT_ENDEXTTEXTINPUT, NULL );
}

void GtkSalFrame::IMHandler::endExtTextInput( USHORT /*nFlags*/ )
{
    if( m_aInputEvent.mpTextAttr )
    {
        vcl::DeletionListener aDel( m_pFrame );
        // delete preedit in sal (commit an empty string)
        sendEmptyCommit();
        if( ! aDel.isDeleted() )
        {
            // mark previous preedit state again (will e.g. be sent at focus gain)
            m_aInputEvent.mpTextAttr = &m_aInputFlags[0];
            if( m_bFocused )
            {
                // begin preedit again
                m_pFrame->getDisplay()->SendInternalEvent( m_pFrame, &m_aInputEvent, SALEVENT_EXTTEXTINPUT );
            }
        }
    }
}

void GtkSalFrame::IMHandler::focusChanged( bool bFocusIn )
{
    m_bFocused = bFocusIn;
    if( bFocusIn )
    {
        m_pFrame->getDisplay()->GetXLib()->PushXErrorLevel( true );
        gtk_im_context_focus_in( m_pIMContext );
        m_pFrame->getDisplay()->GetXLib()->PopXErrorLevel();
        if( m_aInputEvent.mpTextAttr )
        {
            sendEmptyCommit();
            // begin preedit again
            m_pFrame->getDisplay()->SendInternalEvent( m_pFrame, &m_aInputEvent, SALEVENT_EXTTEXTINPUT );
        }
    }
    else
    {
        m_pFrame->getDisplay()->GetXLib()->PushXErrorLevel( true );
        gtk_im_context_focus_out( m_pIMContext );
        m_pFrame->getDisplay()->GetXLib()->PopXErrorLevel();
        // cancel an eventual event posted to begin preedit again
        m_pFrame->getDisplay()->CancelInternalEvent( m_pFrame, &m_aInputEvent, SALEVENT_EXTTEXTINPUT );
    }
}

bool GtkSalFrame::IMHandler::handleKeyEvent( GdkEventKey* pEvent )
{
    vcl::DeletionListener aDel( m_pFrame );

    if( pEvent->type == GDK_KEY_PRESS )
    {
        // Add this key press event to the list of previous key presses
        // to which we compare key release events.  If a later key release
        // event has a matching key press event in this list, we swallow
        // the key release because some GTK Input Methods don't swallow it
        // for us.
        m_aPrevKeyPresses.push_back( PreviousKeyPress(pEvent) );
        m_nPrevKeyPresses++;

        // Also pop off the earliest key press event if there are more than 10
        // already.
        while (m_nPrevKeyPresses > 10)
        {
            m_aPrevKeyPresses.pop_front();
            m_nPrevKeyPresses--;
        }

        GObject* pRef = G_OBJECT( g_object_ref( G_OBJECT( m_pIMContext ) ) );

        // #i51353# update spot location on every key input since we cannot
        // know which key may activate a preedit choice window
        updateIMSpotLocation();
        if( aDel.isDeleted() )
            return true;

        gboolean bResult = gtk_im_context_filter_keypress( m_pIMContext, pEvent );
        g_object_unref( pRef );

        if( aDel.isDeleted() )
            return true;

        if( bResult )
            return true;
        else
        {
            DBG_ASSERT( m_nPrevKeyPresses > 0, "key press has vanished !" );
            if( ! m_aPrevKeyPresses.empty() ) // sanity check
            {
                // event was not swallowed, do not filter a following
                // key release event
                // note: this relies on gtk_im_context_filter_keypress
                // returning without calling a handler (in the "not swallowed"
                // case ) which might change the previous key press list so
                // we would pop the wrong event here
                m_aPrevKeyPresses.pop_back();
                m_nPrevKeyPresses--;
            }
        }
    }

    // Determine if we got an earlier key press event corresponding to this key release
    if (pEvent->type == GDK_KEY_RELEASE)
    {
        GObject* pRef = G_OBJECT( g_object_ref( G_OBJECT( m_pIMContext ) ) );
        gboolean bResult = gtk_im_context_filter_keypress( m_pIMContext, pEvent );
        g_object_unref( pRef );

        if( aDel.isDeleted() )
            return true;

        std::list<PreviousKeyPress>::iterator    iter     = m_aPrevKeyPresses.begin();
        std::list<PreviousKeyPress>::iterator    iter_end = m_aPrevKeyPresses.end();
        while (iter != iter_end)
        {
            // If we found a corresponding previous key press event, swallow the release
            // and remove the earlier key press from our list
            if (*iter == pEvent)
            {
                m_aPrevKeyPresses.erase(iter);
                m_nPrevKeyPresses--;
                return true;
            }
            ++iter;
        }

        if( bResult )
            return true;
    }

    return false;
}

/* FIXME:
* #122282# still more hacking: some IMEs never start a preedit but simply commit
* in this case we cannot commit a single character. Workaround: do not do the
* single key hack for enter or space if the unicode commited does not match
*/

static bool checkSingleKeyCommitHack( guint keyval, sal_Unicode cCode )
{
    bool bRet = true;
    switch( keyval )
    {
        case GDK_KP_Enter:
        case GDK_Return:
            if( cCode != '\n' && cCode != '\r' )
                bRet = false;
            break;
        case GDK_space:
        case GDK_KP_Space:
            if( cCode != ' ' )
                bRet = false;
            break;
        default:
            break;
    }
    return bRet;
}

#ifdef SOLARIS
#define CONTEXT_ARG pContext
#else
#define CONTEXT_ARG EMPTYARG
#endif
void GtkSalFrame::IMHandler::signalIMCommit( GtkIMContext* CONTEXT_ARG, gchar* pText, gpointer im_handler )
{
    GtkSalFrame::IMHandler* pThis = (GtkSalFrame::IMHandler*)im_handler;

    vcl::DeletionListener aDel( pThis->m_pFrame );
    // open a block that will end the GTK_YIELD_GRAB before calling preedit changed again
    {
        GTK_YIELD_GRAB();

        bool bWasPreedit = (pThis->m_aInputEvent.mpTextAttr != 0);

        pThis->m_aInputEvent.mnTime             = 0;
        pThis->m_aInputEvent.mpTextAttr         = 0;
        pThis->m_aInputEvent.maText             = String( pText, RTL_TEXTENCODING_UTF8 );
        pThis->m_aInputEvent.mnCursorPos        = pThis->m_aInputEvent.maText.Len();
        pThis->m_aInputEvent.mnCursorFlags      = 0;
        pThis->m_aInputEvent.mnDeltaStart       = 0;
        pThis->m_aInputEvent.mbOnlyCursor       = False;

        pThis->m_aInputFlags.clear();

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
        bool bSingleCommit = false;
        if( ! bWasPreedit
            && pThis->m_aInputEvent.maText.Len() == 1
            && ! pThis->m_aPrevKeyPresses.empty()
            )
        {
            const PreviousKeyPress& rKP = pThis->m_aPrevKeyPresses.back();
            sal_Unicode aOrigCode = pThis->m_aInputEvent.maText.GetChar(0);

            if( checkSingleKeyCommitHack( rKP.keyval, aOrigCode ) )
            {
                pThis->m_pFrame->doKeyCallback( rKP.state, rKP.keyval, rKP.hardware_keycode, rKP.group, rKP.time, aOrigCode, true, true );
                bSingleCommit = true;
            }
        }

        if( ! bSingleCommit )
        {
            pThis->m_pFrame->CallCallback( SALEVENT_EXTTEXTINPUT, (void*)&pThis->m_aInputEvent);
            if( ! aDel.isDeleted() )
                pThis->doCallEndExtTextInput();
        }
        if( ! aDel.isDeleted() )
        {
            // reset input event
            pThis->m_aInputEvent.maText = String();
            pThis->m_aInputEvent.mnCursorPos = 0;
            pThis->updateIMSpotLocation();
        }
    }
    #ifdef SOLARIS
    // #i51356# workaround a solaris IIIMP bug
    // in case of partial commits the preedit changed signal
    // and commit signal come in wrong order
    if( ! aDel.isDeleted() )
        signalIMPreeditChanged( pContext, im_handler );
    #endif
}

void GtkSalFrame::IMHandler::signalIMPreeditChanged( GtkIMContext*, gpointer im_handler )
{
    GtkSalFrame::IMHandler* pThis = (GtkSalFrame::IMHandler*)im_handler;

    char*           pText           = NULL;
    PangoAttrList*  pAttrs          = NULL;
    gint            nCursorPos      = 0;

    gtk_im_context_get_preedit_string( pThis->m_pIMContext,
                                       &pText,
                                       &pAttrs,
                                       &nCursorPos );
    if( pText && ! *pText ) // empty string
    {
        // change from nothing to nothing -> do not start preedit
        // e.g. this will activate input into a calc cell without
        // user input
        if( pThis->m_aInputEvent.maText.Len() == 0 )
        {
            g_free( pText );
            return;
        }
    }

    bool bEndPreedit = (!pText || !*pText) && pThis->m_aInputEvent.mpTextAttr != NULL;
    pThis->m_aInputEvent.mnTime             = 0;
    pThis->m_aInputEvent.maText             = String( pText, RTL_TEXTENCODING_UTF8 );
    pThis->m_aInputEvent.mnCursorPos        = nCursorPos;
    pThis->m_aInputEvent.mnCursorFlags      = 0;
    pThis->m_aInputEvent.mnDeltaStart       = 0;
    pThis->m_aInputEvent.mbOnlyCursor       = False;

    pThis->m_aInputFlags = std::vector<USHORT>( std::max( 1, (int)pThis->m_aInputEvent.maText.Len() ), 0 );

    PangoAttrIterator   *iter       = pango_attr_list_get_iterator (pAttrs);
    do
    {
        GSList *attr_list = NULL;
        GSList *tmp_list = NULL;
        gint start, end;
        guint sal_attr = 0;

        pango_attr_iterator_range (iter, &start, &end);
        if (end == G_MAXINT)
            end = pText ? strlen (pText) : 0;
        if (end == start)
            continue;

        start = g_utf8_pointer_to_offset (pText, pText + start);
        end = g_utf8_pointer_to_offset (pText, pText + end);

        tmp_list = attr_list = pango_attr_iterator_get_attrs (iter);
        while (tmp_list)
        {
            PangoAttribute *pango_attr = (PangoAttribute *)(tmp_list->data);

            switch (pango_attr->klass->type)
            {
                case PANGO_ATTR_BACKGROUND:
                    sal_attr |= (SAL_EXTTEXTINPUT_ATTR_HIGHLIGHT | SAL_EXTTEXTINPUT_CURSOR_INVISIBLE);
                    break;
                case PANGO_ATTR_UNDERLINE:
                    sal_attr |= SAL_EXTTEXTINPUT_ATTR_UNDERLINE;
                    break;
                case PANGO_ATTR_STRIKETHROUGH:
                    sal_attr |= SAL_EXTTEXTINPUT_ATTR_REDTEXT;
                    break;
                default:
                    break;
            }
            pango_attribute_destroy (pango_attr);
            tmp_list = tmp_list->next;
        }
        if (sal_attr == 0)
            sal_attr |= SAL_EXTTEXTINPUT_ATTR_UNDERLINE;
        g_slist_free (attr_list);

        // Set the sal attributes on our text
        for (int i = start; i < end; i++)
            pThis->m_aInputFlags[i] |= sal_attr;
    } while (pango_attr_iterator_next (iter));

    pThis->m_aInputEvent.mpTextAttr         = &pThis->m_aInputFlags[0];

    g_free( pText );
    pango_attr_list_unref( pAttrs );

    GTK_YIELD_GRAB();

    vcl::DeletionListener aDel( pThis->m_pFrame );

    pThis->m_pFrame->CallCallback( SALEVENT_EXTTEXTINPUT, (void*)&pThis->m_aInputEvent);
    if( bEndPreedit && ! aDel.isDeleted() )
        pThis->doCallEndExtTextInput();
    if( ! aDel.isDeleted() )
        pThis->updateIMSpotLocation();
}

void GtkSalFrame::IMHandler::signalIMPreeditStart( GtkIMContext*, gpointer /*im_handler*/ )
{
}

void GtkSalFrame::IMHandler::signalIMPreeditEnd( GtkIMContext*, gpointer im_handler )
{
    GtkSalFrame::IMHandler* pThis = (GtkSalFrame::IMHandler*)im_handler;
    GTK_YIELD_GRAB();

    vcl::DeletionListener aDel( pThis->m_pFrame );
    pThis->doCallEndExtTextInput();
    if( ! aDel.isDeleted() )
        pThis->updateIMSpotLocation();
}

uno::Reference<accessibility::XAccessibleEditableText>
    FindFocus(uno::Reference< accessibility::XAccessibleContext > xContext)
{
    if (!xContext.is())
        uno::Reference< accessibility::XAccessibleEditableText >();

    uno::Reference<accessibility::XAccessibleStateSet> xState = xContext->getAccessibleStateSet();
    if (xState.is())
    {
        if (xState->contains(accessibility::AccessibleStateType::FOCUSED))
            return uno::Reference<accessibility::XAccessibleEditableText>(xContext, uno::UNO_QUERY);
    }

    for (sal_Int32 i = 0; i < xContext->getAccessibleChildCount(); ++i)
    {
        uno::Reference< accessibility::XAccessible > xChild = xContext->getAccessibleChild(i);
        if (!xChild.is())
            continue;
            uno::Reference< accessibility::XAccessibleContext > xChildContext = xChild->getAccessibleContext();
        if (!xChildContext.is())
            continue;
        uno::Reference< accessibility::XAccessibleEditableText > xText = FindFocus(xChildContext);
        if (xText.is())
            return xText;
    }
    return uno::Reference< accessibility::XAccessibleEditableText >();
}

uno::Reference<accessibility::XAccessibleEditableText> lcl_GetxText()
{
    uno::Reference<accessibility::XAccessibleEditableText> xText;
    Window* pFocusWin = ImplGetSVData()->maWinData.mpFocusWin;
    if (!pFocusWin)
    return xText;

    uno::Reference< accessibility::XAccessible > xAccessible( pFocusWin->GetAccessible( true ) );
    if (xAccessible.is())
        xText = FindFocus(xAccessible->getAccessibleContext());
    return xText;
}

gboolean GtkSalFrame::IMHandler::signalIMRetrieveSurrounding( GtkIMContext* pContext, gpointer /*im_handler*/ )
{
    uno::Reference<accessibility::XAccessibleEditableText> xText = lcl_GetxText();

    if (xText.is())
    {
        sal_uInt32 nPosition = xText->getCaretPosition();
        rtl::OUString sAllText = xText->getText();
        if (!sAllText.getLength())
            return FALSE;
    rtl::OString sUTF = rtl::OUStringToOString(sAllText, RTL_TEXTENCODING_UTF8);
    rtl::OUString sCursorText(sAllText, nPosition);
    gtk_im_context_set_surrounding(pContext, sUTF.getStr(), sUTF.getLength(),
        rtl::OUStringToOString(sCursorText, RTL_TEXTENCODING_UTF8).getLength());
    return TRUE;
    }

    return FALSE;
}

gboolean GtkSalFrame::IMHandler::signalIMDeleteSurrounding( GtkIMContext*, gint offset, gint nchars,
    gpointer /*im_handler*/ )
{
    uno::Reference<accessibility::XAccessibleEditableText> xText = lcl_GetxText();

    if (xText.is())
    {
        sal_uInt32 nPosition = xText->getCaretPosition();
    xText->deleteText(nPosition + offset, nPosition + offset + nchars);
    return TRUE;
    }

    return FALSE;
}
