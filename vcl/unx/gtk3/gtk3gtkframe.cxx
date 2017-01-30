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

#include <unx/gtk/gtkframe.hxx>
#include <unx/gtk/gtkdata.hxx>
#include <unx/gtk/gtkinst.hxx>
#include <unx/gtk/gtkgdi.hxx>
#include <vcl/help.hxx>
#include <vcl/keycodes.hxx>
#include <vcl/layout.hxx>
#include <unx/wmadaptor.hxx>
#include <unx/sm.hxx>
#include <unx/salbmp.h>
#include <generic/genprn.h>
#include <generic/geninst.h>
#include <headless/svpgdi.hxx>
#include <osl/file.hxx>
#include <rtl/bootstrap.hxx>
#include <rtl/process.h>
#include <vcl/floatwin.hxx>
#include <vcl/svapp.hxx>
#include <vcl/window.hxx>
#include <vcl/settings.hxx>

#include <config_vclplug.h>

#if defined(ENABLE_DBUS) && ENABLE_GIO
#  include <unx/gtk/gtksalmenu.hxx>
#endif
#if defined ENABLE_GMENU_INTEGRATION // defined in gtksalmenu.hxx above
#  include <unx/gtk/hudawareness.h>
#endif

#include <gtk/gtk.h>
#include <prex.h>
#include <X11/Xatom.h>
#if defined(GDK_WINDOWING_X11)
#   include <gdk/gdkx.h>
#endif
#if defined(GDK_WINDOWING_WAYLAND)
#   include <gdk/gdkwayland.h>
#endif
#include <postx.h>

#include <dlfcn.h>
#include <vcl/salbtype.hxx>
#include <impbmp.hxx>
#include <svids.hrc>
#include <sal/macros.h>

#include <basegfx/range/b2ibox.hxx>
#include <basegfx/vector/b2ivector.hxx>

#include <algorithm>
#include <glib/gprintf.h>

#if OSL_DEBUG_LEVEL > 1
#  include <cstdio>
#endif

#include <cstdlib>
#include <cmath>

#include <comphelper/processfactory.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <com/sun/star/accessibility/XAccessibleContext.hpp>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/XAccessibleStateSet.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/XAccessibleEditableText.hpp>
#include <com/sun/star/awt/MouseButton.hpp>
#include <com/sun/star/datatransfer/dnd/DNDConstants.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/ModuleManager.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/util/URLTransformer.hpp>

#include <gdk/gdkkeysyms-compat.h>

#include <config_folders.h>

#define IS_WIDGET_REALIZED gtk_widget_get_realized
#define IS_WIDGET_MAPPED   gtk_widget_get_mapped

#ifndef GDK_IS_X11_DISPLAY
#define GDK_IS_X11_DISPLAY(foo) (true)
#endif


using namespace com::sun::star;

int GtkSalFrame::m_nFloats = 0;

#if defined ENABLE_GMENU_INTEGRATION
static GDBusConnection* pSessionBus = nullptr;
#endif

static sal_uInt16 GetKeyModCode( guint state )
{
    sal_uInt16 nCode = 0;
    if( (state & GDK_SHIFT_MASK) )
        nCode |= KEY_SHIFT;
    if( (state & GDK_CONTROL_MASK) )
        nCode |= KEY_MOD1;
    if( (state & GDK_MOD1_MASK) )
        nCode |= KEY_MOD2;

    // Map Meta/Super keys to MOD3 modifier on all Unix systems
    // except Mac OS X
    if ( (state & GDK_META_MASK ) || ( state & GDK_SUPER_MASK ) )
        nCode |= KEY_MOD3;
    return nCode;
}

static sal_uInt16 GetMouseModCode( guint state )
{
    sal_uInt16 nCode = GetKeyModCode( state );
    if( (state & GDK_BUTTON1_MASK) )
        nCode |= MOUSE_LEFT;
    if( (state & GDK_BUTTON2_MASK) )
        nCode |= MOUSE_MIDDLE;
    if( (state & GDK_BUTTON3_MASK) )
        nCode |= MOUSE_RIGHT;

    return nCode;
}

static sal_uInt16 GetKeyCode( guint keyval )
{
    sal_uInt16 nCode = 0;
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
        switch( keyval )
        {
            // - - - - - Sun keyboard, see vcl/unx/source/app/saldisp.cxx
            case GDK_L2:
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
            case GDK_period:        nCode = KEY_POINT;      break;
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
            case GDK_bracketleft:  nCode = KEY_BRACKETLEFT;  break;
            case GDK_bracketright: nCode = KEY_BRACKETRIGHT; break;
            case GDK_semicolon:    nCode = KEY_SEMICOLON;   break;
            case GDK_quoteright:   nCode = KEY_QUOTERIGHT;  break;
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

static guint GetKeyValFor(GdkKeymap* pKeyMap, guint16 hardware_keycode, guint8 group)
{
    guint updated_keyval = 0;
    gdk_keymap_translate_keyboard_state(pKeyMap, hardware_keycode,
        (GdkModifierType)0, group, &updated_keyval, nullptr, nullptr, nullptr);
    return updated_keyval;
}

// F10 means either KEY_F10 or KEY_MENU, which has to be decided
// in the independent part.
struct KeyAlternate
{
    sal_uInt16          nKeyCode;
    sal_Unicode     nCharCode;
    KeyAlternate() : nKeyCode( 0 ), nCharCode( 0 ) {}
    KeyAlternate( sal_uInt16 nKey, sal_Unicode nChar = 0 ) : nKeyCode( nKey ), nCharCode( nChar ) {}
};

inline KeyAlternate
GetAlternateKeyCode( const sal_uInt16 nKeyCode )
{
    KeyAlternate aAlternate;

    switch( nKeyCode )
    {
        case KEY_F10: aAlternate = KeyAlternate( KEY_MENU );break;
        case KEY_F24: aAlternate = KeyAlternate( KEY_SUBTRACT, '-' );break;
    }

    return aAlternate;
}

namespace {
/// Decouple SalFrame lifetime from damagetracker lifetime
struct DamageTracker : public basebmp::IBitmapDeviceDamageTracker
{
    DamageTracker(GtkSalFrame& rFrame) : m_rFrame(rFrame)
    {}

    virtual ~DamageTracker() {}

    virtual void damaged(const basegfx::B2IBox& rDamageRect) const override
    {
        m_rFrame.damaged(rDamageRect);
    }

    GtkSalFrame& m_rFrame;
};
}

static bool dumpframes = false;

void GtkSalFrame::doKeyCallback( guint state,
                                 guint keyval,
                                 guint16 hardware_keycode,
                                 guint8 group,
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

#if 0
    // shift-zero forces a re-draw and event is swallowed
    if (keyval == GDK_0)
    {
        fprintf( stderr, "force widget_queue_draw\n");
        gtk_widget_queue_draw(GTK_WIDGET(m_pFixedContainer));
        return;
    }
    else if (keyval == GDK_1)
    {
        fprintf( stderr, "force repaint all\n");
        TriggerPaintEvent();
        return;
    }
    else if (keyval == GDK_2)
    {
        dumpframes = !dumpframes;
        fprintf(stderr, "toggle dump frames to %d\n", dumpframes);
        return;
    }
#endif

    /*
     *  #i42122# translate all keys with Ctrl and/or Alt to group 0 else
     *  shortcuts (e.g. Ctrl-o) will not work but be inserted by the
     *  application
     *
     *  #i52338# do this for all keys that the independent part has no key code
     *  for
     *
     *  fdo#41169 rather than use group 0, detect if there is a group which can
     *  be used to input Latin text and use that if possible
     */
    aEvent.mnCode = GetKeyCode( keyval );
    if( aEvent.mnCode == 0 )
    {
        gint best_group = SAL_MAX_INT32;

        // Try and find Latin layout
        GdkKeymap* keymap = gdk_keymap_get_default();
        GdkKeymapKey *keys;
        gint n_keys;
        if (gdk_keymap_get_entries_for_keyval(keymap, GDK_A, &keys, &n_keys))
        {
            // Find the lowest group that supports Latin layout
            for (gint i = 0; i < n_keys; ++i)
            {
                if (keys[i].level != 0 && keys[i].level != 1)
                    continue;
                best_group = std::min(best_group, keys[i].group);
                if (best_group == 0)
                    break;
            }
            g_free(keys);
        }

        //Unavailable, go with original group then I suppose
        if (best_group == SAL_MAX_INT32)
            best_group = group;

        guint updated_keyval = GetKeyValFor(keymap, hardware_keycode, best_group);
        aEvent.mnCode = GetKeyCode(updated_keyval);
    }

    aEvent.mnCode   |= GetKeyModCode( state );

    if( bDown )
    {
        bool bHandled = CallCallback( SALEVENT_KEYINPUT, &aEvent );
        // #i46889# copy AlternateKeyCode handling from generic plugin
        if( ! bHandled )
        {
            KeyAlternate aAlternate = GetAlternateKeyCode( aEvent.mnCode );
            if( aAlternate.nKeyCode )
            {
                aEvent.mnCode = aAlternate.nKeyCode;
                if( aAlternate.nCharCode )
                    aEvent.mnCharCode = aAlternate.nCharCode;
                CallCallback( SALEVENT_KEYINPUT, &aEvent );
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

GtkSalFrame::GtkSalFrame( SalFrame* pParent, SalFrameStyleFlags nStyle )
    : m_nXScreen( getDisplay()->GetDefaultXScreen() )
    , m_pGraphics(nullptr)
    , m_bGraphics(false)
{
    getDisplay()->registerFrame( this );
    m_bDefaultPos       = true;
    m_bDefaultSize      = ( (nStyle & SalFrameStyleFlags::SIZEABLE) && ! pParent );
    m_bWindowIsGtkPlug  = false;
#if defined(ENABLE_DBUS) && ENABLE_GIO
    m_pLastSyncedDbusMenu = nullptr;
#endif
    Init( pParent, nStyle );
}

GtkSalFrame::GtkSalFrame( SystemParentData* pSysData )
    : m_nXScreen( getDisplay()->GetDefaultXScreen() )
    , m_pGraphics(nullptr)
    , m_bGraphics(false)
{
    getDisplay()->registerFrame( this );
    // permanently ignore errors from our unruly children ...
    GetGenericData()->ErrorTrapPush();
    m_bDefaultPos       = true;
    m_bDefaultSize      = true;
#if defined(ENABLE_DBUS) && ENABLE_GIO
    m_pLastSyncedDbusMenu = nullptr;
#endif
    Init( pSysData );
}

#ifdef ENABLE_GMENU_INTEGRATION

// AppMenu watch functions.

static void ObjectDestroyedNotify( gpointer data )
{
    if ( data ) {
        g_object_unref( data );
    }
}

#if defined(ENABLE_DBUS) && ENABLE_GIO
void GtkSalFrame::EnsureDbusMenuSynced()
{
    GtkSalMenu* pSalMenu = static_cast<GtkSalMenu*>(GetMenu());
    if(m_pLastSyncedDbusMenu != pSalMenu) {
        m_pLastSyncedDbusMenu = pSalMenu;
        static_cast<GtkSalMenu*>(pSalMenu)->Activate();
    }
}
#endif

static void hud_activated( gboolean hud_active, gpointer user_data )
{
    if ( hud_active )
    {
        SolarMutexGuard aGuard;
        GtkSalFrame* pSalFrame = static_cast< GtkSalFrame* >( user_data );
        GtkSalMenu* pSalMenu = reinterpret_cast< GtkSalMenu* >( pSalFrame->GetMenu() );

        if ( pSalMenu )
            pSalMenu->UpdateFull();
    }
}

static void activate_uno(GSimpleAction *action, GVariant*, gpointer)
{
    uno::Reference< css::uno::XComponentContext > xContext = ::comphelper::getProcessComponentContext();

    uno::Reference< css::frame::XDesktop2 > xDesktop = css::frame::Desktop::create( xContext );

    uno::Reference < css::frame::XFrame > xFrame(xDesktop->getActiveFrame());
    if (!xFrame.is())
        xFrame.set(xDesktop, uno::UNO_QUERY);

    if (!xFrame.is())
        return;

    uno::Reference< css::frame::XDispatchProvider > xDispatchProvider(xFrame, uno::UNO_QUERY);
    if (!xDispatchProvider.is())
        return;

    gchar *strval = nullptr;
    g_object_get(action, "name", &strval, NULL);
    if (!strval)
        return;

    if (strcmp(strval, "New") == 0)
    {
        uno::Reference<frame::XModuleManager2> xModuleManager(frame::ModuleManager::create(xContext));
        OUString aModuleId(xModuleManager->identify(xFrame));
        if (aModuleId.isEmpty())
            return;

        comphelper::SequenceAsHashMap lModuleDescription(xModuleManager->getByName(aModuleId));
        OUString sFactoryService;
        lModuleDescription[OUString("ooSetupFactoryEmptyDocumentURL")] >>= sFactoryService;
        if (sFactoryService.isEmpty())
            return;

        uno::Sequence < css::beans::PropertyValue > args(0);
        xDesktop->loadComponentFromURL(sFactoryService, "_blank", 0, args);
        return;
    }

    OUString sCommand(".uno:");
    sCommand += OUString(strval, strlen(strval), RTL_TEXTENCODING_UTF8);
    g_free(strval);

    css::util::URL aCommand;
    aCommand.Complete = sCommand;
    uno::Reference< css::util::XURLTransformer > xParser = css::util::URLTransformer::create(xContext);
    xParser->parseStrict(aCommand);

    uno::Reference< css::frame::XDispatch > xDisp = xDispatchProvider->queryDispatch(aCommand, OUString(), 0);

    if (!xDisp.is())
        return;

    xDisp->dispatch(aCommand, css::uno::Sequence< css::beans::PropertyValue >());
}

static const GActionEntry app_entries[] = {
  { "OptionsTreeDialog", activate_uno, nullptr, nullptr, nullptr, {0} },
  { "About", activate_uno, nullptr, nullptr, nullptr, {0} },
  { "HelpIndex", activate_uno, nullptr, nullptr, nullptr, {0} },
  { "Quit", activate_uno, nullptr, nullptr, nullptr, {0} },
  { "New", activate_uno, nullptr, nullptr, nullptr, {0} }
};

gboolean ensure_dbus_setup( gpointer data )
{
    GtkSalFrame* pSalFrame = static_cast< GtkSalFrame* >( data );
    GdkWindow* gdkWindow = widget_get_window( pSalFrame->getWindow() );

    if ( gdkWindow != nullptr && g_object_get_data( G_OBJECT( gdkWindow ), "g-lo-menubar" ) == nullptr )
    {
        // Get a DBus session connection.
        if(!pSessionBus)
            pSessionBus = g_bus_get_sync (G_BUS_TYPE_SESSION, nullptr, nullptr);
        if( !pSessionBus )
        {
            return FALSE;
        }

        // Create menu model and action group attached to this frame.
        GMenuModel* pMenuModel = G_MENU_MODEL( g_lo_menu_new() );
        GActionGroup* pActionGroup = reinterpret_cast<GActionGroup*>(g_lo_action_group_new( static_cast< gpointer >( pSalFrame ) ));

        // Generate menu paths.
        sal_uIntPtr windowId = pSalFrame->GetNativeWindowHandle(pSalFrame->getWindow());
        gchar* aDBusWindowPath = g_strdup_printf( "/org/libreoffice/window/%lu", windowId );
        gchar* aDBusMenubarPath = g_strdup_printf( "/org/libreoffice/window/%lu/menus/menubar", windowId );

        // Set window properties.
        g_object_set_data_full( G_OBJECT( gdkWindow ), "g-lo-menubar", pMenuModel, ObjectDestroyedNotify );
        g_object_set_data_full( G_OBJECT( gdkWindow ), "g-lo-action-group", pActionGroup, ObjectDestroyedNotify );

        GdkDisplay *pDisplay = pSalFrame->getGdkDisplay();
        // fdo#70885 we don't want app menu under Unity
        const bool bDesktopIsUnity = (SalGetDesktopEnvironment() == "UNITY");
#if defined(GDK_WINDOWING_X11)
        if (GDK_IS_X11_DISPLAY(pDisplay))
        {
            gdk_x11_window_set_utf8_property( gdkWindow, "_GTK_APPLICATION_ID", "org.libreoffice" );
            if (!bDesktopIsUnity)
                gdk_x11_window_set_utf8_property( gdkWindow, "_GTK_APP_MENU_OBJECT_PATH", "/org/libreoffice/menus/appmenu" );
            gdk_x11_window_set_utf8_property( gdkWindow, "_GTK_MENUBAR_OBJECT_PATH", aDBusMenubarPath );
            gdk_x11_window_set_utf8_property( gdkWindow, "_GTK_WINDOW_OBJECT_PATH", aDBusWindowPath );
            gdk_x11_window_set_utf8_property( gdkWindow, "_GTK_APPLICATION_OBJECT_PATH", "/org/libreoffice" );
            gdk_x11_window_set_utf8_property( gdkWindow, "_GTK_UNIQUE_BUS_NAME", g_dbus_connection_get_unique_name( pSessionBus ) );
        }
#endif
#if defined(GDK_WINDOWING_WAYLAND)
        if (GDK_IS_WAYLAND_DISPLAY(pDisplay))
        {
            gdk_wayland_window_set_dbus_properties_libgtk_only(gdkWindow, "org.libreoffice",
                                                               "/org/libreoffice/menus/appmenu",
                                                               !bDesktopIsUnity ? aDBusMenubarPath : nullptr,
                                                               aDBusWindowPath,
                                                               "/org/libreoffice",
                                                               g_dbus_connection_get_unique_name( pSessionBus ));
        }
#endif
        // Publish the menu model and the action group.
        SAL_INFO("vcl.unity", "exporting menu model at " << pMenuModel << " for window " << windowId);
        pSalFrame->m_nMenuExportId = g_dbus_connection_export_menu_model (pSessionBus, aDBusMenubarPath, pMenuModel, nullptr);
        SAL_INFO("vcl.unity", "exporting action group at " << pActionGroup << " for window " << windowId);
        pSalFrame->m_nActionGroupExportId = g_dbus_connection_export_action_group( pSessionBus, aDBusWindowPath, pActionGroup, nullptr);
        pSalFrame->m_nHudAwarenessId = hud_awareness_register( pSessionBus, aDBusMenubarPath, hud_activated, pSalFrame, nullptr, nullptr );

        //app menu, to-do translations, block normal menus when active, honor use appmenu settings
        ResMgr* pMgr = ImplGetResMgr();
        if( pMgr && !bDesktopIsUnity )
        {
            GMenu *menu = g_menu_new ();
            GMenuItem* item;

            GMenu *firstsubmenu = g_menu_new ();

            OString sNew(OUStringToOString(ResId(SV_BUTTONTEXT_NEW, *pMgr).toString(),
                RTL_TEXTENCODING_UTF8).replaceFirst("~", "_"));

            item = g_menu_item_new(sNew.getStr(), "app.New");
            g_menu_append_item( firstsubmenu, item );
            g_object_unref(item);

            g_menu_append_section( menu, nullptr, G_MENU_MODEL(firstsubmenu));
            g_object_unref(firstsubmenu);

            GMenu *secondsubmenu = g_menu_new ();

            OString sPreferences(OUStringToOString(ResId(SV_STDTEXT_PREFERENCES, *pMgr).toString(),
                RTL_TEXTENCODING_UTF8).replaceFirst("~", "_"));

            item = g_menu_item_new(sPreferences.getStr(), "app.OptionsTreeDialog");
            g_menu_append_item( secondsubmenu, item );
            g_object_unref(item);

            g_menu_append_section( menu, nullptr, G_MENU_MODEL(secondsubmenu));
            g_object_unref(secondsubmenu);

            GMenu *thirdsubmenu = g_menu_new ();

            OString sHelp(OUStringToOString(ResId(SV_BUTTONTEXT_HELP, *pMgr).toString(),
                RTL_TEXTENCODING_UTF8).replaceFirst("~", "_"));

            item = g_menu_item_new(sHelp.getStr(), "app.HelpIndex");
            g_menu_append_item( thirdsubmenu, item );
            g_object_unref(item);

            OString sAbout(OUStringToOString(ResId(SV_STDTEXT_ABOUT, *pMgr).toString(),
                RTL_TEXTENCODING_UTF8).replaceFirst("~", "_"));

            item = g_menu_item_new(sAbout.getStr(), "app.About");
            g_menu_append_item( thirdsubmenu, item );
            g_object_unref(item);

            OString sQuit(OUStringToOString(ResId(SV_MENU_MAC_QUITAPP, *pMgr).toString(),
                RTL_TEXTENCODING_UTF8).replaceFirst("~", "_"));

            item = g_menu_item_new(sQuit.getStr(), "app.Quit");
            g_menu_append_item( thirdsubmenu, item );
            g_object_unref(item);
            g_menu_append_section( menu, nullptr, G_MENU_MODEL(thirdsubmenu));
            g_object_unref(thirdsubmenu);

            GSimpleActionGroup *group = g_simple_action_group_new ();
            g_action_map_add_action_entries (G_ACTION_MAP (group), app_entries, G_N_ELEMENTS (app_entries), nullptr);
            GActionGroup* pAppActionGroup = G_ACTION_GROUP(group);

            pSalFrame->m_nAppActionGroupExportId = g_dbus_connection_export_action_group( pSessionBus, "/org/libreoffice", pAppActionGroup, nullptr);
            g_object_unref(pAppActionGroup);
            pSalFrame->m_nAppMenuExportId = g_dbus_connection_export_menu_model (pSessionBus, "/org/libreoffice/menus/appmenu", G_MENU_MODEL (menu), nullptr);
            g_object_unref(menu);
        }

        g_free( aDBusMenubarPath );
        g_free( aDBusWindowPath );
    }

    return FALSE;
}

void on_registrar_available( GDBusConnection * /*connection*/,
                             const gchar     * /*name*/,
                             const gchar     * /*name_owner*/,
                             gpointer         user_data )
{
    SolarMutexGuard aGuard;

    GtkSalFrame* pSalFrame = static_cast< GtkSalFrame* >( user_data );

    SalMenu* pSalMenu = pSalFrame->GetMenu();

    if ( pSalMenu != nullptr )
    {
        GtkSalMenu* pGtkSalMenu = static_cast<GtkSalMenu*>(pSalMenu);
        pGtkSalMenu->Display( true );
        pGtkSalMenu->UpdateFull();
    }
}

// This is called when the registrar becomes unavailable. It shows the menubar.
void on_registrar_unavailable( GDBusConnection * /*connection*/,
                               const gchar     * /*name*/,
                               gpointer         user_data )
{
    SolarMutexGuard aGuard;

    SAL_INFO("vcl.unity", "on_registrar_unavailable");

    //pSessionBus = NULL;
    GtkSalFrame* pSalFrame = static_cast< GtkSalFrame* >( user_data );

    SalMenu* pSalMenu = pSalFrame->GetMenu();

    if ( pSalMenu ) {
        GtkSalMenu* pGtkSalMenu = static_cast< GtkSalMenu* >( pSalMenu );
        pGtkSalMenu->Display( false );
    }
}
#endif

void GtkSalFrame::EnsureAppMenuWatch()
{
#ifdef ENABLE_GMENU_INTEGRATION
    if ( !m_nWatcherId )
    {
        // Get a DBus session connection.
        if ( pSessionBus == nullptr )
        {
            pSessionBus = g_bus_get_sync( G_BUS_TYPE_SESSION, nullptr, nullptr );

            if ( pSessionBus == nullptr )
                return;
        }

        // Publish the menu only if AppMenu registrar is available.
        m_nWatcherId = g_bus_watch_name_on_connection( pSessionBus,
                                                       "com.canonical.AppMenu.Registrar",
                                                       G_BUS_NAME_WATCHER_FLAGS_NONE,
                                                       on_registrar_available,
                                                       on_registrar_unavailable,
                                                       static_cast<GtkSalFrame*>(this),
                                                       nullptr );
    }

    //ensure_dbus_setup( this );
#else
    (void) this; // loplugin:staticmethods
#endif
}

void GtkSalFrame::InvalidateGraphics()
{
    if( m_pGraphics )
    {
        m_bGraphics = false;
    }
}

GtkSalFrame::~GtkSalFrame()
{
    if (m_pDropTarget)
    {
        m_pDropTarget->deinitialize();
        m_pDropTarget = nullptr;
    }

    if (m_pDragSource)
    {
        m_pDragSource->deinitialize();
        m_pDragSource= nullptr;
    }

    InvalidateGraphics();

    if( m_pParent )
        m_pParent->m_aChildren.remove( this );

    getDisplay()->deregisterFrame( this );

    if( m_pRegion )
    {
        cairo_region_destroy( m_pRegion );
    }

    delete m_pIMHandler;

    GtkWidget *pEventWidget = getMouseEventWidget();
    for (auto handler_id : m_aMouseSignalIds)
        g_signal_handler_disconnect(G_OBJECT(pEventWidget), handler_id);
    if( m_pFixedContainer )
        gtk_widget_destroy( GTK_WIDGET( m_pFixedContainer ) );
    if( m_pEventBox )
        gtk_widget_destroy( GTK_WIDGET(m_pEventBox) );
    {
        SolarMutexGuard aGuard;
#if defined ENABLE_GMENU_INTEGRATION
        if(m_nWatcherId)
            g_bus_unwatch_name(m_nWatcherId);
#endif
        if( m_pWindow )
        {
            g_object_set_data( G_OBJECT( m_pWindow ), "SalFrame", nullptr );

#if defined ENABLE_GMENU_INTEGRATION
            if ( pSessionBus )
            {
                if ( m_nHudAwarenessId )
                    hud_awareness_unregister( pSessionBus, m_nHudAwarenessId );
                if ( m_nMenuExportId )
                    g_dbus_connection_unexport_menu_model( pSessionBus, m_nMenuExportId );
                if ( m_nAppMenuExportId )
                    g_dbus_connection_unexport_menu_model( pSessionBus, m_nAppMenuExportId );
                if ( m_nActionGroupExportId )
                    g_dbus_connection_unexport_action_group( pSessionBus, m_nActionGroupExportId );
                if ( m_nAppActionGroupExportId )
                    g_dbus_connection_unexport_action_group( pSessionBus, m_nAppActionGroupExportId );
            }
#endif
            gtk_widget_destroy( m_pWindow );
        }
    }
    if( m_pForeignParent )
        g_object_unref( G_OBJECT( m_pForeignParent ) );
    if( m_pForeignTopLevel )
        g_object_unref( G_OBJECT( m_pForeignTopLevel) );

    delete m_pGraphics;
    m_pGraphics = nullptr;
}

void GtkSalFrame::moveWindow( long nX, long nY )
{
    if( isChild( false ) )
    {
        if( m_pParent )
            gtk_fixed_move( m_pParent->getFixedContainer(),
                            m_pWindow,
                            nX - m_pParent->maGeometry.nX, nY - m_pParent->maGeometry.nY );
    }
    else
        gtk_window_move( GTK_WINDOW(m_pWindow), nX, nY );
}

void GtkSalFrame::dragWindowTo(long nX, long nY)
{
    if (isChild(false))
        moveWindow(nX, nY);
    else
    {
#if defined(GDK_WINDOWING_WAYLAND)
        if (GDK_IS_WAYLAND_DISPLAY(getGdkDisplay()))
        {
            gtk_window_begin_move_drag(GTK_WINDOW(m_pWindow), 1, nX, nY, GDK_CURRENT_TIME);
            return;
        }
#endif
        gtk_window_move(GTK_WINDOW(m_pWindow), nX, nY);
    }
}

void GtkSalFrame::widget_set_size_request(long nWidth, long nHeight)
{
    gtk_widget_set_size_request(GTK_WIDGET(m_pFixedContainer), nWidth, nHeight );
}

void GtkSalFrame::window_resize(long nWidth, long nHeight)
{
    m_nWidthRequest = nWidth;
    m_nHeightRequest = nHeight;
    gtk_window_resize(GTK_WINDOW(m_pWindow), nWidth, nHeight);
}

void GtkSalFrame::resizeWindow( long nWidth, long nHeight )
{
    if( isChild( false ) )
    {
        widget_set_size_request(nWidth, nHeight);
    }
    else if( ! isChild( true, false ) )
        window_resize(nWidth, nHeight);
}

static void
ooo_fixed_class_init(GtkFixedClass *klass)
{
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);
    widget_class->get_accessible = ooo_fixed_get_accessible;
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
            nullptr,      /* base init */
            nullptr,  /* base finalize */
            reinterpret_cast<GClassInitFunc>(ooo_fixed_class_init), /* class init */
            nullptr, /* class finalize */
            nullptr,                      /* class data */
            sizeof (GtkFixed),         /* instance size */
            0,                         /* nb preallocs */
            nullptr,  /* instance init */
            nullptr                       /* value table */
        };

        type = g_type_register_static( GTK_TYPE_FIXED, "OOoFixed",
                                       &tinfo, (GTypeFlags) 0);
    }

    return type;
}

void GtkSalFrame::updateScreenNumber()
{
    int nScreen = 0;
    GdkScreen *pScreen = gtk_widget_get_screen( m_pWindow );
    if( pScreen )
        nScreen = getDisplay()->getSystem()->getScreenMonitorIdx( pScreen, maGeometry.nX, maGeometry.nY );
    maGeometry.nDisplayScreenNumber = nScreen;
}

GtkWidget *GtkSalFrame::getMouseEventWidget() const
{
    return GTK_WIDGET(m_pEventBox);
}

void GtkSalFrame::InitCommon()
{
    m_pEventBox = GTK_EVENT_BOX(gtk_event_box_new());
    gtk_widget_add_events( GTK_WIDGET(m_pEventBox),
                           GDK_ALL_EVENTS_MASK );
    gtk_container_add( GTK_CONTAINER(m_pWindow), GTK_WIDGET(m_pEventBox) );

    // add the fixed container child,
    // fixed is needed since we have to position plugin windows
    m_pFixedContainer = GTK_FIXED(g_object_new( ooo_fixed_get_type(), nullptr ));
    gtk_container_add( GTK_CONTAINER(m_pEventBox), GTK_WIDGET(m_pFixedContainer) );

    GtkWidget *pEventWidget = getMouseEventWidget();

    gtk_widget_set_app_paintable(GTK_WIDGET(m_pFixedContainer), true);
    /*non-X11 displays won't show anything at all without double-buffering
      enabled*/
    if (GDK_IS_X11_DISPLAY(getGdkDisplay()))
        gtk_widget_set_double_buffered(GTK_WIDGET(m_pFixedContainer), false);
    gtk_widget_set_redraw_on_allocate(GTK_WIDGET(m_pFixedContainer), false);


    // connect signals
    g_signal_connect( G_OBJECT(m_pWindow), "style-updated", G_CALLBACK(signalStyleUpdated), this );
    gtk_widget_set_has_tooltip(pEventWidget, true);
    m_aMouseSignalIds.push_back(g_signal_connect( G_OBJECT(pEventWidget), "query-tooltip", G_CALLBACK(signalTooltipQuery), this ));
    m_aMouseSignalIds.push_back(g_signal_connect( G_OBJECT(pEventWidget), "button-press-event", G_CALLBACK(signalButton), this ));
    m_aMouseSignalIds.push_back(g_signal_connect( G_OBJECT(pEventWidget), "motion-notify-event", G_CALLBACK(signalMotion), this ));
    m_aMouseSignalIds.push_back(g_signal_connect( G_OBJECT(pEventWidget), "button-release-event", G_CALLBACK(signalButton), this ));

    //Drop Target Stuff
    gtk_drag_dest_set(GTK_WIDGET(pEventWidget), (GtkDestDefaults)0, nullptr, 0, (GdkDragAction)0);
    gtk_drag_dest_set_track_motion(GTK_WIDGET(pEventWidget), true);
    m_aMouseSignalIds.push_back(g_signal_connect( G_OBJECT(pEventWidget), "drag-motion", G_CALLBACK(signalDragMotion), this ));
    m_aMouseSignalIds.push_back(g_signal_connect( G_OBJECT(pEventWidget), "drag-drop", G_CALLBACK(signalDragDrop), this ));
    m_aMouseSignalIds.push_back(g_signal_connect( G_OBJECT(pEventWidget), "drag-data-received", G_CALLBACK(signalDragDropReceived), this ));
    m_aMouseSignalIds.push_back(g_signal_connect( G_OBJECT(pEventWidget), "drag-leave", G_CALLBACK(signalDragLeave), this ));

    //Drag Source Stuff
    m_aMouseSignalIds.push_back(g_signal_connect( G_OBJECT(pEventWidget), "drag-end", G_CALLBACK(signalDragEnd), this ));
    m_aMouseSignalIds.push_back(g_signal_connect( G_OBJECT(pEventWidget), "drag-failed", G_CALLBACK(signalDragFailed), this ));
    m_aMouseSignalIds.push_back(g_signal_connect( G_OBJECT(pEventWidget), "drag-data-delete", G_CALLBACK(signalDragDelete), this ));
    m_aMouseSignalIds.push_back(g_signal_connect( G_OBJECT(pEventWidget), "drag-data-get", G_CALLBACK(signalDragDataGet), this ));
    m_aMouseSignalIds.push_back(g_signal_connect( G_OBJECT(pEventWidget), "scroll-event", G_CALLBACK(signalScroll), this ));

    g_signal_connect( G_OBJECT(m_pFixedContainer), "draw", G_CALLBACK(signalDraw), this );
    g_signal_connect( G_OBJECT(m_pFixedContainer), "size-allocate", G_CALLBACK(sizeAllocated), this );
#if GTK_CHECK_VERSION(3,14,0)
    GtkGesture *pSwipe = gtk_gesture_swipe_new(pEventWidget);
    g_signal_connect(pSwipe, "swipe", G_CALLBACK(gestureSwipe), this);
    gtk_event_controller_set_propagation_phase(GTK_EVENT_CONTROLLER (pSwipe), GTK_PHASE_TARGET);
    g_object_weak_ref(G_OBJECT(pEventWidget), reinterpret_cast<GWeakNotify>(g_object_unref), pSwipe);

    GtkGesture *pLongPress = gtk_gesture_long_press_new(pEventWidget);
    g_signal_connect(pLongPress, "pressed", G_CALLBACK(gestureLongPress), this);
    gtk_event_controller_set_propagation_phase(GTK_EVENT_CONTROLLER (pLongPress), GTK_PHASE_TARGET);
    g_object_weak_ref(G_OBJECT(pEventWidget), reinterpret_cast<GWeakNotify>(g_object_unref), pLongPress);

#endif

    g_signal_connect( G_OBJECT(m_pWindow), "focus-in-event", G_CALLBACK(signalFocus), this );
    g_signal_connect( G_OBJECT(m_pWindow), "focus-out-event", G_CALLBACK(signalFocus), this );
    g_signal_connect( G_OBJECT(m_pWindow), "map-event", G_CALLBACK(signalMap), this );
    g_signal_connect( G_OBJECT(m_pWindow), "unmap-event", G_CALLBACK(signalUnmap), this );
    g_signal_connect( G_OBJECT(m_pWindow), "configure-event", G_CALLBACK(signalConfigure), this );
    g_signal_connect( G_OBJECT(m_pWindow), "key-press-event", G_CALLBACK(signalKey), this );
    g_signal_connect( G_OBJECT(m_pWindow), "key-release-event", G_CALLBACK(signalKey), this );
    g_signal_connect( G_OBJECT(m_pWindow), "delete-event", G_CALLBACK(signalDelete), this );
    g_signal_connect( G_OBJECT(m_pWindow), "window-state-event", G_CALLBACK(signalWindowState), this );
    g_signal_connect( G_OBJECT(m_pWindow), "leave-notify-event", G_CALLBACK(signalCrossing), this );
    g_signal_connect( G_OBJECT(m_pWindow), "enter-notify-event", G_CALLBACK(signalCrossing), this );
    g_signal_connect( G_OBJECT(m_pWindow), "visibility-notify-event", G_CALLBACK(signalVisibility), this );
    g_signal_connect( G_OBJECT(m_pWindow), "destroy", G_CALLBACK(signalDestroy), this );

    // init members
    m_pCurrentCursor    = nullptr;
    m_nKeyModifiers     = 0;
    m_bFullscreen       = false;
    m_bSpanMonitorsWhenFullscreen = false;
    m_nState            = GDK_WINDOW_STATE_WITHDRAWN;
    m_nVisibility       = GDK_VISIBILITY_FULLY_OBSCURED;
    m_bSendModChangeOnRelease = false;
    m_pIMHandler        = nullptr;
    m_hBackgroundPixmap = None;
    m_nExtStyle         = 0;
    m_pRegion           = nullptr;
    m_pDropTarget       = nullptr;
    m_pDragSource       = nullptr;
    m_bInDrag           = false;
    m_pFormatConversionRequest = nullptr;
    m_ePointerStyle     = static_cast<PointerStyle>(0xffff);
    m_bSetFocusOnMap    = false;
    m_pSalMenu          = nullptr;
    m_nWatcherId        = 0;
    m_nMenuExportId     = 0;
    m_nAppMenuExportId  = 0;
    m_nActionGroupExportId = 0;
    m_nAppActionGroupExportId = 0;
    m_nHudAwarenessId   = 0;

    gtk_widget_add_events( m_pWindow,
                           GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK |
                           GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK |
                           GDK_VISIBILITY_NOTIFY_MASK | GDK_SCROLL_MASK
                           );

    // show the widgets
    gtk_widget_show_all( GTK_WIDGET(m_pEventBox) );

    // realize the window, we need an XWindow id
    gtk_widget_realize( m_pWindow );

    //system data
    m_aSystemData.nSize         = sizeof( SystemEnvData );
    m_aSystemData.aWindow       = GetNativeWindowHandle(m_pWindow);
    m_aSystemData.aShellWindow  = reinterpret_cast<long>(this);
    m_aSystemData.pSalFrame     = this;
    m_aSystemData.pWidget       = m_pWindow;
    m_aSystemData.nScreen       = m_nXScreen.getXScreen();
    m_aSystemData.pAppContext   = nullptr;
    m_aSystemData.pShellWidget  = m_aSystemData.pWidget;
    m_aSystemData.pToolkit      = "gtk3";

    m_bGraphics = false;
    m_pGraphics = nullptr;

    m_nWidthRequest = 0;
    m_nHeightRequest = 0;

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
    updateScreenNumber();

    SetIcon(1);
}

GtkSalFrame *GtkSalFrame::getFromWindow( GtkWindow *pWindow )
{
    return static_cast<GtkSalFrame *>(g_object_get_data( G_OBJECT( pWindow ), "SalFrame" ));
}

void GtkSalFrame::Init( SalFrame* pParent, SalFrameStyleFlags nStyle )
{
    if( nStyle & SalFrameStyleFlags::DEFAULT ) // ensure default style
    {
        nStyle |= SalFrameStyleFlags::MOVEABLE | SalFrameStyleFlags::SIZEABLE | SalFrameStyleFlags::CLOSEABLE;
        nStyle &= ~SalFrameStyleFlags::FLOAT;
    }

    m_pParent = static_cast<GtkSalFrame*>(pParent);
    m_pForeignParent = nullptr;
    m_aForeignParentWindow = None;
    m_pForeignTopLevel = nullptr;
    m_aForeignTopLevelWindow = None;
    m_nStyle = nStyle;

    GtkWindowType eWinType = (  (nStyle & SalFrameStyleFlags::FLOAT) &&
                              ! (nStyle & SalFrameStyleFlags::OWNERDRAWDECORATION)
                              )
        ? GTK_WINDOW_POPUP : GTK_WINDOW_TOPLEVEL;

    if( nStyle & SalFrameStyleFlags::SYSTEMCHILD )
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
    {
        m_pWindow = gtk_widget_new( GTK_TYPE_WINDOW, "type", eWinType,
                                    "visible", FALSE, nullptr );
    }
    g_object_set_data( G_OBJECT( m_pWindow ), "SalFrame", this );
    g_object_set_data( G_OBJECT( m_pWindow ), "libo-version", const_cast<char *>(LIBO_VERSION_DOTTED));

    // force wm class hint
    m_nExtStyle = ~0;
    if (m_pParent)
        m_sWMClass = m_pParent->m_sWMClass;
    SetExtendedFrameStyle( 0 );

    if( m_pParent && m_pParent->m_pWindow && ! isChild() )
        gtk_window_set_screen( GTK_WINDOW(m_pWindow), gtk_window_get_screen( GTK_WINDOW(m_pParent->m_pWindow) ) );

    if (m_pParent)
    {
        if (!(m_pParent->m_nStyle & SalFrameStyleFlags::PLUG))
            gtk_window_set_transient_for( GTK_WINDOW(m_pWindow), GTK_WINDOW(m_pParent->m_pWindow) );
        m_pParent->m_aChildren.push_back( this );
    }

    InitCommon();

    // set window type
    bool bDecoHandling =
        ! isChild() &&
        ( ! (nStyle & SalFrameStyleFlags::FLOAT) ||
          (nStyle & SalFrameStyleFlags::OWNERDRAWDECORATION) );

    if( bDecoHandling )
    {
        GdkWindowTypeHint eType = GDK_WINDOW_TYPE_HINT_NORMAL;
        if( (nStyle & SalFrameStyleFlags::DIALOG) && m_pParent != nullptr )
            eType = GDK_WINDOW_TYPE_HINT_DIALOG;
        if( (nStyle & SalFrameStyleFlags::INTRO) )
        {
            gtk_window_set_role( GTK_WINDOW(m_pWindow), "splashscreen" );
            eType = GDK_WINDOW_TYPE_HINT_SPLASHSCREEN;
        }
        else if( (nStyle & SalFrameStyleFlags::TOOLWINDOW ) )
        {
            eType = GDK_WINDOW_TYPE_HINT_UTILITY;
            gtk_window_set_skip_taskbar_hint( GTK_WINDOW(m_pWindow), true );
        }
        else if( (nStyle & SalFrameStyleFlags::OWNERDRAWDECORATION) )
        {
            eType = GDK_WINDOW_TYPE_HINT_TOOLBAR;
            gtk_window_set_accept_focus(GTK_WINDOW(m_pWindow), false);
            gtk_window_set_decorated(GTK_WINDOW(m_pWindow), false);
        }
        gtk_window_set_type_hint( GTK_WINDOW(m_pWindow), eType );
        gtk_window_set_gravity( GTK_WINDOW(m_pWindow), GDK_GRAVITY_STATIC );
    }
    else if( (nStyle & SalFrameStyleFlags::FLOAT) )
        gtk_window_set_type_hint( GTK_WINDOW(m_pWindow), GDK_WINDOW_TYPE_HINT_POPUP_MENU );

    if( eWinType == GTK_WINDOW_TOPLEVEL )
    {
#ifdef ENABLE_GMENU_INTEGRATION
        // Enable DBus native menu if available.
        ensure_dbus_setup( this );
#endif

    }

    if( bDecoHandling )
    {
        gtk_window_set_resizable( GTK_WINDOW(m_pWindow), bool(nStyle & SalFrameStyleFlags::SIZEABLE) );
        if( ( (nStyle & (SalFrameStyleFlags::OWNERDRAWDECORATION)) ) )
            gtk_window_set_accept_focus(GTK_WINDOW(m_pWindow), false);
    }
}

GdkNativeWindow GtkSalFrame::findTopLevelSystemWindow( GdkNativeWindow aWindow )
{
    (void)aWindow;
    //FIXME: no findToplevelSystemWindow
    return 0;
}

void GtkSalFrame::Init( SystemParentData* pSysData )
{
    m_pParent = nullptr;
    m_aForeignParentWindow = (GdkNativeWindow)pSysData->aWindow;
    m_pForeignParent = nullptr;
    m_aForeignTopLevelWindow = findTopLevelSystemWindow( (GdkNativeWindow)pSysData->aWindow );
    m_pForeignTopLevel = gdk_window_foreign_new_for_display( getGdkDisplay(), m_aForeignTopLevelWindow );
    gdk_window_set_events( m_pForeignTopLevel, GDK_STRUCTURE_MASK );

    if( pSysData->nSize > sizeof(pSysData->nSize)+sizeof(pSysData->aWindow) && pSysData->bXEmbedSupport )
    {
        m_pWindow = gtk_plug_new_for_display( getGdkDisplay(), pSysData->aWindow );
        m_bWindowIsGtkPlug  = true;
        widget_set_can_default( m_pWindow, true );
        widget_set_can_focus( m_pWindow, true );
        gtk_widget_set_sensitive( m_pWindow, true );
    }
    else
    {
        m_pWindow = gtk_window_new( GTK_WINDOW_POPUP );
        m_bWindowIsGtkPlug  = false;
    }
    m_nStyle = SalFrameStyleFlags::PLUG;
    InitCommon();

    m_pForeignParent = gdk_window_foreign_new_for_display( getGdkDisplay(), m_aForeignParentWindow );
    gdk_window_set_events( m_pForeignParent, GDK_STRUCTURE_MASK );

    //FIXME: Handling embedded windows, is going to be fun ...
}

void GtkSalFrame::askForXEmbedFocus( sal_Int32 i_nTimeCode )
{
    (void) this; // loplugin:staticmethods
    (void)i_nTimeCode;
    //FIXME: no askForXEmbedFocus for gtk3 yet
}

void GtkSalFrame::SetExtendedFrameStyle( SalExtStyle nStyle )
{
    if( nStyle != m_nExtStyle && ! isChild() )
    {
        m_nExtStyle = nStyle;
        updateWMClass();
    }
}

SalGraphics* GtkSalFrame::AcquireGraphics()
{
    if( m_bGraphics )
        return nullptr;

    if( !m_pGraphics )
    {
        m_pGraphics = new GtkSalGraphics( this, m_pWindow );
        if( !m_aFrame.get() )
        {
            AllocateFrame();
            TriggerPaintEvent();
        }
        m_pGraphics->setDevice( m_aFrame );
    }
    m_bGraphics = true;
    return m_pGraphics;
}

void GtkSalFrame::ReleaseGraphics( SalGraphics* pGraphics )
{
    (void) pGraphics;
    assert( pGraphics == m_pGraphics );
    m_bGraphics = false;
}

bool GtkSalFrame::PostEvent(ImplSVEvent* pData)
{
    getDisplay()->SendInternalEvent( this, pData );
    return true;
}

void GtkSalFrame::SetTitle( const OUString& rTitle )
{
    m_aTitle = rTitle;
    if( m_pWindow && ! isChild() )
        gtk_window_set_title( GTK_WINDOW(m_pWindow), OUStringToOString( rTitle, RTL_TEXTENCODING_UTF8 ).getStr() );
}

void GtkSalFrame::SetIcon( sal_uInt16 nIcon )
{
    if( (m_nStyle & (SalFrameStyleFlags::PLUG|SalFrameStyleFlags::SYSTEMCHILD|SalFrameStyleFlags::FLOAT|SalFrameStyleFlags::INTRO|SalFrameStyleFlags::OWNERDRAWDECORATION))
        || ! m_pWindow )
        return;

    gchar* appicon;

    if (nIcon == SV_ICON_ID_TEXT)
        appicon = g_strdup ("libreoffice-writer");
    else if (nIcon == SV_ICON_ID_SPREADSHEET)
        appicon = g_strdup ("libreoffice-calc");
    else if (nIcon == SV_ICON_ID_DRAWING)
        appicon = g_strdup ("libreoffice-draw");
    else if (nIcon == SV_ICON_ID_PRESENTATION)
        appicon = g_strdup ("libreoffice-impress");
    else if (nIcon == SV_ICON_ID_DATABASE)
        appicon = g_strdup ("libreoffice-base");
    else if (nIcon == SV_ICON_ID_FORMULA)
        appicon = g_strdup ("libreoffice-math");
    else
        appicon = g_strdup ("libreoffice-main");

    gtk_window_set_icon_name (GTK_WINDOW (m_pWindow), appicon);
}

void GtkSalFrame::SetMenu( SalMenu* pSalMenu )
{
//    if(m_pSalMenu)
//    {
//        static_cast<GtkSalMenu*>(m_pSalMenu)->DisconnectFrame();
//    }
    m_pSalMenu = pSalMenu;
}

SalMenu* GtkSalFrame::GetMenu()
{
    return m_pSalMenu;
}

void GtkSalFrame::DrawMenuBar()
{
}

void GtkSalFrame::Center()
{
    if (m_pParent)
        gtk_window_set_position(GTK_WINDOW(m_pWindow), GTK_WIN_POS_CENTER_ON_PARENT);
    else
        gtk_window_set_position(GTK_WINDOW(m_pWindow), GTK_WIN_POS_CENTER);
}

Size GtkSalFrame::calcDefaultSize()
{
    return bestmaxFrameSizeForScreenSize(getDisplay()->GetScreenSize(GetDisplayScreen()));
}

void GtkSalFrame::SetDefaultSize()
{
    Size aDefSize = calcDefaultSize();

    SetPosSize( 0, 0, aDefSize.Width(), aDefSize.Height(),
                SAL_FRAME_POSSIZE_WIDTH | SAL_FRAME_POSSIZE_HEIGHT );

    if( (m_nStyle & SalFrameStyleFlags::DEFAULT) && m_pWindow )
        gtk_window_maximize( GTK_WINDOW(m_pWindow) );
}

static void initClientId()
{
    // No session management support for gtk3+ - this is now legacy.
}

void GtkSalFrame::Show( bool bVisible, bool bNoActivate )
{
    if( m_pWindow )
    {
        if( bVisible )
        {
            initClientId();
            getDisplay()->startupNotificationCompleted();

            if( m_bDefaultPos )
                Center();
            if( m_bDefaultSize )
                SetDefaultSize();
            setMinMaxSize();

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
                 m_pParent->grabPointer( true, true );
            }

            if( ! bNoActivate && (m_nStyle & SalFrameStyleFlags::TOOLWINDOW) )
                m_bSetFocusOnMap = true;

            gtk_widget_show( m_pWindow );

            if( isFloatGrabWindow() )
            {
                m_nFloats++;
                if( ! getDisplay()->GetCaptureFrame() && m_nFloats == 1 )
                {
                    grabPointer(true, true);
                    GtkSalFrame *pKeyboardFrame = m_pParent ? m_pParent : this;
                    pKeyboardFrame->grabKeyboard(true);
                }
                // #i44068# reset parent's IM context
                if( m_pParent )
                    m_pParent->EndExtTextInput(EndExtTextInputFlags::NONE);
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
                {
                    GtkSalFrame *pKeyboardFrame = m_pParent ? m_pParent : this;
                    pKeyboardFrame->grabKeyboard(false);
                    grabPointer(false);
                }
            }
            gtk_widget_hide( m_pWindow );
            if( m_pIMHandler )
                m_pIMHandler->focusChanged( false );
            // flush here; there may be a very seldom race between
            // the display connection used for clipboard and our connection
            Flush();
        }
    }
}

void GtkSalFrame::setMinMaxSize()
{
    /*  #i34504# metacity (and possibly others) do not treat
     *  _NET_WM_STATE_FULLSCREEN and max_width/height independently;
     *  whether they should is undefined. So don't set the max size hint
     *  for a full screen window.
    */
    if( m_pWindow && ! isChild() )
    {
        GdkGeometry aGeo;
        int aHints = 0;
        if( m_nStyle & SalFrameStyleFlags::SIZEABLE )
        {
            if( m_aMinSize.Width() && m_aMinSize.Height() && ! m_bFullscreen )
            {
                aGeo.min_width  = m_aMinSize.Width();
                aGeo.min_height = m_aMinSize.Height();
                aHints |= GDK_HINT_MIN_SIZE;
            }
            if( m_aMaxSize.Width() && m_aMaxSize.Height() && ! m_bFullscreen )
            {
                aGeo.max_width  = m_aMaxSize.Width();
                aGeo.max_height = m_aMaxSize.Height();
                aHints |= GDK_HINT_MAX_SIZE;
            }
        }
        else
        {
            if (!m_bFullscreen && m_nWidthRequest && m_nHeightRequest)
            {
                aGeo.min_width = m_nWidthRequest;
                aGeo.min_height = m_nHeightRequest;
                aHints |= GDK_HINT_MIN_SIZE;

                aGeo.max_width = m_nWidthRequest;
                aGeo.max_height = m_nHeightRequest;
                aHints |= GDK_HINT_MAX_SIZE;
            }
        }

        if( m_bFullscreen && m_aMaxSize.Width() && m_aMaxSize.Height() )
        {
            aGeo.max_width = m_aMaxSize.Width();
            aGeo.max_height = m_aMaxSize.Height();
            aHints |= GDK_HINT_MAX_SIZE;
        }
        if( aHints )
        {
            gtk_window_set_geometry_hints( GTK_WINDOW(m_pWindow),
                                           nullptr,
                                           &aGeo,
                                           GdkWindowHints( aHints ) );
        }
    }
}

void GtkSalFrame::SetMaxClientSize( long nWidth, long nHeight )
{
    if( ! isChild() )
    {
        m_aMaxSize = Size( nWidth, nHeight );
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
            widget_set_size_request(nWidth, nHeight);
            setMinMaxSize();
        }
    }
}

// FIXME: we should really be an SvpSalFrame sub-class, and
// share their AllocateFrame !
void GtkSalFrame::AllocateFrame()
{
    basegfx::B2IVector aFrameSize( maGeometry.nWidth, maGeometry.nHeight );
    if( ! m_aFrame.get() || m_aFrame->getSize() != aFrameSize )
    {
        if( aFrameSize.getX() == 0 )
            aFrameSize.setX( 1 );
        if( aFrameSize.getY() == 0 )
            aFrameSize.setY( 1 );
        m_aFrame = basebmp::createBitmapDevice(aFrameSize, true, SVP_CAIRO_FORMAT);
        assert(cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, aFrameSize.getX()) ==
               m_aFrame->getScanlineStride());
        m_aFrame->setDamageTracker(
            basebmp::IBitmapDeviceDamageTrackerSharedPtr(new DamageTracker(*this)) );
        SAL_INFO("vcl.gtk3", "allocated m_aFrame size of " << maGeometry.nWidth << " x " << maGeometry.nHeight);

#if OSL_DEBUG_LEVEL > 0 // set background to orange
        m_aFrame->clear( basebmp::Color( 255, 127, 0 ) );
#endif

        if( m_pGraphics )
            m_pGraphics->setDevice( m_aFrame );
    }
}

void GtkSalFrame::SetPosSize( long nX, long nY, long nWidth, long nHeight, sal_uInt16 nFlags )
{
    if( !m_pWindow || isChild( true, false ) )
        return;

    if( (nFlags & ( SAL_FRAME_POSSIZE_WIDTH | SAL_FRAME_POSSIZE_HEIGHT )) &&
        (nWidth > 0 && nHeight > 0 ) // sometimes stupid things happen
            )
    {
        m_bDefaultSize = false;

        if( isChild( false ) )
            widget_set_size_request(nWidth, nHeight);
        else if( ! ( m_nState & GDK_WINDOW_STATE_MAXIMIZED ) )
            window_resize(nWidth, nHeight);

        setMinMaxSize();
    }
    else if( m_bDefaultSize )
        SetDefaultSize();

    m_bDefaultSize = false;

    if( nFlags & ( SAL_FRAME_POSSIZE_X | SAL_FRAME_POSSIZE_Y ) )
    {
        if( m_pParent )
        {
            if( AllSettings::GetLayoutRTL() )
                nX = m_pParent->maGeometry.nWidth-m_nWidthRequest-1-nX;
            nX += m_pParent->maGeometry.nX;
            nY += m_pParent->maGeometry.nY;
        }

        m_bDefaultPos = false;

        if (nFlags & SAL_FRAME_POSSIZE_BYDRAG)
            dragWindowTo(nX, nY);
        else
            moveWindow(nX, nY);

        updateScreenNumber();
    }
    else if( m_bDefaultPos )
        Center();

    m_bDefaultPos = false;
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
    GdkScreen  *pScreen = gtk_window_get_screen(GTK_WINDOW(m_pWindow));
    Rectangle aRetRect;
    int max = gdk_screen_get_n_monitors (pScreen);
    for (int i = 0; i < max; ++i)
    {
        GdkRectangle aRect;
        gdk_screen_get_monitor_workarea(pScreen, i, &aRect);
        Rectangle aMonitorRect(aRect.x, aRect.y, aRect.x+aRect.width, aRect.y+aRect.height);
        aRetRect.Union(aMonitorRect);
    }
    rRect = aRetRect;
}

SalFrame* GtkSalFrame::GetParent() const
{
    return m_pParent;
}

void GtkSalFrame::SetWindowState( const SalFrameState* pState )
{
    if( ! m_pWindow || ! pState || isChild( true, false ) )
        return;

    const sal_uLong nMaxGeometryMask =
        WINDOWSTATE_MASK_X | WINDOWSTATE_MASK_Y |
        WINDOWSTATE_MASK_WIDTH | WINDOWSTATE_MASK_HEIGHT |
        WINDOWSTATE_MASK_MAXIMIZED_X | WINDOWSTATE_MASK_MAXIMIZED_Y |
        WINDOWSTATE_MASK_MAXIMIZED_WIDTH | WINDOWSTATE_MASK_MAXIMIZED_HEIGHT;

    if( (pState->mnMask & WINDOWSTATE_MASK_STATE) &&
        ! ( m_nState & GDK_WINDOW_STATE_MAXIMIZED ) &&
        (pState->mnState & WINDOWSTATE_STATE_MAXIMIZED) &&
        (pState->mnMask & nMaxGeometryMask) == nMaxGeometryMask )
    {
        resizeWindow( pState->mnWidth, pState->mnHeight );
        moveWindow( pState->mnX, pState->mnY );
        m_bDefaultPos = m_bDefaultSize = false;

        updateScreenNumber();

        m_nState = GdkWindowState( m_nState | GDK_WINDOW_STATE_MAXIMIZED );
        m_aRestorePosSize = Rectangle( Point( pState->mnX, pState->mnY ),
                                       Size( pState->mnWidth, pState->mnHeight ) );
    }
    else if( pState->mnMask & (WINDOWSTATE_MASK_X | WINDOWSTATE_MASK_Y |
                               WINDOWSTATE_MASK_WIDTH | WINDOWSTATE_MASK_HEIGHT ) )
    {
        sal_uInt16 nPosSizeFlags = 0;
        long nX         = pState->mnX - (m_pParent ? m_pParent->maGeometry.nX : 0);
        long nY         = pState->mnY - (m_pParent ? m_pParent->maGeometry.nY : 0);
        if( pState->mnMask & WINDOWSTATE_MASK_X )
            nPosSizeFlags |= SAL_FRAME_POSSIZE_X;
        else
            nX = maGeometry.nX - (m_pParent ? m_pParent->maGeometry.nX : 0);
        if( pState->mnMask & WINDOWSTATE_MASK_Y )
            nPosSizeFlags |= SAL_FRAME_POSSIZE_Y;
        else
            nY = maGeometry.nY - (m_pParent ? m_pParent->maGeometry.nY : 0);
        if( pState->mnMask & WINDOWSTATE_MASK_WIDTH )
            nPosSizeFlags |= SAL_FRAME_POSSIZE_WIDTH;
        if( pState->mnMask & WINDOWSTATE_MASK_HEIGHT )
            nPosSizeFlags |= SAL_FRAME_POSSIZE_HEIGHT;
        SetPosSize( nX, nY, pState->mnWidth, pState->mnHeight, nPosSizeFlags );
    }
    if( pState->mnMask & WINDOWSTATE_MASK_STATE && ! isChild() )
    {
        if( pState->mnState & WINDOWSTATE_STATE_MAXIMIZED )
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
        if( (pState->mnState & WINDOWSTATE_STATE_MINIMIZED)
            && ! m_pParent )
            gtk_window_iconify( GTK_WINDOW(m_pWindow) );
        else
            gtk_window_deiconify( GTK_WINDOW(m_pWindow) );
    }
    TriggerPaintEvent();
}

namespace
{
    void GetPosAndSize(GtkWindow *pWindow, long& rX, long &rY, long &rWidth, long &rHeight)
    {
       gint root_x, root_y;
       gtk_window_get_position(GTK_WINDOW(pWindow), &root_x, &root_y);
       rX = root_x;
       rY = root_y;
       gint width, height;
       gtk_window_get_size(GTK_WINDOW(pWindow), &width, &height);
       rWidth = width;
       rHeight = height;
    }

    Rectangle GetPosAndSize(GtkWindow *pWindow)
    {
        long nX, nY, nWidth, nHeight;
        GetPosAndSize(pWindow, nX, nY, nWidth, nHeight);
        return Rectangle(nX, nY, nX + nWidth, nY + nHeight);
    }
}

bool GtkSalFrame::GetWindowState( SalFrameState* pState )
{
    pState->mnState = WINDOWSTATE_STATE_NORMAL;
    pState->mnMask  = WINDOWSTATE_MASK_STATE;
    // rollup ? gtk 2.2 does not seem to support the shaded state
    if( (m_nState & GDK_WINDOW_STATE_ICONIFIED) )
        pState->mnState |= WINDOWSTATE_STATE_MINIMIZED;
    if( m_nState & GDK_WINDOW_STATE_MAXIMIZED )
    {
        pState->mnState |= WINDOWSTATE_STATE_MAXIMIZED;
        pState->mnX                 = m_aRestorePosSize.Left();
        pState->mnY                 = m_aRestorePosSize.Top();
        pState->mnWidth             = m_aRestorePosSize.GetWidth();
        pState->mnHeight            = m_aRestorePosSize.GetHeight();
        GetPosAndSize(GTK_WINDOW(m_pWindow), pState->mnMaximizedX, pState->mnMaximizedY,
                                             pState->mnMaximizedWidth, pState->mnMaximizedHeight);
        pState->mnMask  |= WINDOWSTATE_MASK_MAXIMIZED_X          |
                           WINDOWSTATE_MASK_MAXIMIZED_Y          |
                           WINDOWSTATE_MASK_MAXIMIZED_WIDTH      |
                           WINDOWSTATE_MASK_MAXIMIZED_HEIGHT;
    }
    else
    {
        GetPosAndSize(GTK_WINDOW(m_pWindow), pState->mnX, pState->mnY,
                                             pState->mnWidth, pState->mnHeight);
    }
    pState->mnMask  |= WINDOWSTATE_MASK_X            |
                       WINDOWSTATE_MASK_Y            |
                       WINDOWSTATE_MASK_WIDTH        |
                       WINDOWSTATE_MASK_HEIGHT;

    return true;
}

typedef enum {
    SET_RETAIN_SIZE,
    SET_FULLSCREEN,
    SET_UN_FULLSCREEN
} SetType;

void GtkSalFrame::SetScreen( unsigned int nNewScreen, int eType, Rectangle *pSize )
{
    if( !m_pWindow )
        return;

    if (maGeometry.nDisplayScreenNumber == nNewScreen && eType == SET_RETAIN_SIZE)
        return;

    int nX = maGeometry.nX, nY = maGeometry.nY,
        nWidth = maGeometry.nWidth, nHeight = maGeometry.nHeight;
    GdkScreen *pScreen = nullptr;
    GdkRectangle aNewMonitor;

    bool bSpanAllScreens = nNewScreen == (unsigned int)-1;
    m_bSpanMonitorsWhenFullscreen = bSpanAllScreens && getDisplay()->getSystem()->GetDisplayScreenCount() > 1;

    if (m_bSpanMonitorsWhenFullscreen)   //span all screens
    {
        pScreen = gtk_widget_get_screen( m_pWindow );
        aNewMonitor.x = 0;
        aNewMonitor.y = 0;
        aNewMonitor.width = gdk_screen_get_width(pScreen);
        aNewMonitor.height = gdk_screen_get_height(pScreen);
    }
    else
    {
        gint nMonitor;
        bool bSameMonitor = false;

        if (!bSpanAllScreens)
        {
            pScreen = getDisplay()->getSystem()->getScreenMonitorFromIdx( nNewScreen, nMonitor );
            if (!pScreen)
            {
                g_warning ("Attempt to move GtkSalFrame to invalid screen %d => "
                           "fallback to current\n", nNewScreen);
            }
        }

        if (!pScreen)
        {
            pScreen = gtk_widget_get_screen( m_pWindow );
            bSameMonitor = true;
        }

        // Heavy lifting, need to move screen ...
        if( pScreen != gtk_widget_get_screen( m_pWindow ))
            gtk_window_set_screen( GTK_WINDOW( m_pWindow ), pScreen );

        gint nOldMonitor = gdk_screen_get_monitor_at_window(
                                pScreen, widget_get_window( m_pWindow ) );
        if (bSameMonitor)
            nMonitor = nOldMonitor;

    #if OSL_DEBUG_LEVEL > 1
        if( nMonitor == nOldMonitor )
            g_warning( "An apparently pointless SetScreen - should we elide it ?" );
    #endif

        GdkRectangle aOldMonitor;
        gdk_screen_get_monitor_geometry( pScreen, nOldMonitor, &aOldMonitor );
        gdk_screen_get_monitor_geometry( pScreen, nMonitor, &aNewMonitor );

        nX = aNewMonitor.x + nX - aOldMonitor.x;
        nY = aNewMonitor.y + nY - aOldMonitor.y;
    }

    bool bResize = false;
    bool bVisible = IS_WIDGET_MAPPED( m_pWindow );
    if( bVisible )
        Show( false );

    if( eType == SET_FULLSCREEN )
    {
        nX = aNewMonitor.x;
        nY = aNewMonitor.y;
        nWidth = aNewMonitor.width;
        nHeight = aNewMonitor.height;
        m_nStyle |= SalFrameStyleFlags::PARTIAL_FULLSCREEN;
        bResize = true;

        // #i110881# for the benefit of compiz set a max size here
        // else setting to fullscreen fails for unknown reasons
        m_aMaxSize.Width() = aNewMonitor.width;
        m_aMaxSize.Height() = aNewMonitor.height;
    }

    if( pSize && eType == SET_UN_FULLSCREEN )
    {
        nX = pSize->Left();
        nY = pSize->Top();
        nWidth = pSize->GetWidth();
        nHeight = pSize->GetHeight();
        m_nStyle &= ~SalFrameStyleFlags::PARTIAL_FULLSCREEN;
        bResize = true;
    }

    if (bResize)
    {
        // temporarily re-sizeable
        if( !(m_nStyle & SalFrameStyleFlags::SIZEABLE) )
            gtk_window_set_resizable( GTK_WINDOW(m_pWindow), TRUE );
        window_resize(nWidth, nHeight);
    }

    gtk_window_move(GTK_WINDOW(m_pWindow), nX, nY);

#if GTK_CHECK_VERSION(3,8,0)
    gdk_window_set_fullscreen_mode( widget_get_window(m_pWindow), m_bSpanMonitorsWhenFullscreen
        ? GDK_FULLSCREEN_ON_ALL_MONITORS : GDK_FULLSCREEN_ON_CURRENT_MONITOR );
#endif
    if( eType == SET_FULLSCREEN )
        gtk_window_fullscreen( GTK_WINDOW( m_pWindow ) );
    else if( eType == SET_UN_FULLSCREEN )
        gtk_window_unfullscreen( GTK_WINDOW( m_pWindow ) );

    if( eType == SET_UN_FULLSCREEN &&
        !(m_nStyle & SalFrameStyleFlags::SIZEABLE) )
        gtk_window_set_resizable( GTK_WINDOW( m_pWindow ), FALSE );

    // FIXME: we should really let gtk+ handle our widget hierarchy ...
    if( m_pParent && gtk_widget_get_screen( m_pParent->m_pWindow ) != pScreen )
        SetParent( nullptr );
    std::list< GtkSalFrame* > aChildren = m_aChildren;
    for( std::list< GtkSalFrame* >::iterator it = aChildren.begin(); it != aChildren.end(); ++it )
        (*it)->SetScreen( nNewScreen, SET_RETAIN_SIZE );

    m_bDefaultPos = m_bDefaultSize = false;
    updateScreenNumber();

    if( bVisible )
        Show( true );
}

void GtkSalFrame::SetScreenNumber( unsigned int nNewScreen )
{
    SetScreen( nNewScreen, SET_RETAIN_SIZE );
}

void GtkSalFrame::updateWMClass()
{
    OString aResClass = OUStringToOString(m_sWMClass, RTL_TEXTENCODING_ASCII_US);
    const char *pResClass = !aResClass.isEmpty() ? aResClass.getStr() :
                                                    SalGenericSystem::getFrameClassName();
    Display *display;

    if (!getDisplay()->IsX11Display())
        return;

    display = GDK_DISPLAY_XDISPLAY(getGdkDisplay());

    if( IS_WIDGET_REALIZED( m_pWindow ) )
    {
        XClassHint* pClass = XAllocClassHint();
        OString aResName = SalGenericSystem::getFrameResName();
        pClass->res_name  = const_cast<char*>(aResName.getStr());
        pClass->res_class = const_cast<char*>(pResClass);
        XSetClassHint( display,
                       widget_get_xid(m_pWindow),
                       pClass );
        XFree( pClass );
    }
}

void GtkSalFrame::SetApplicationID( const OUString &rWMClass )
{
    if( rWMClass != m_sWMClass && ! isChild() )
    {
        m_sWMClass = rWMClass;
        updateWMClass();

        for( std::list< GtkSalFrame* >::iterator it = m_aChildren.begin(); it != m_aChildren.end(); ++it )
            (*it)->SetApplicationID(rWMClass);
    }
}

void GtkSalFrame::ShowFullScreen( bool bFullScreen, sal_Int32 nScreen )
{
    m_bFullscreen = bFullScreen;

    if( !m_pWindow || isChild() )
        return;

    if( bFullScreen )
    {
        m_aRestorePosSize = GetPosAndSize(GTK_WINDOW(m_pWindow));
        SetScreen( nScreen, SET_FULLSCREEN );
    }
    else
    {
        SetScreen( nScreen, SET_UN_FULLSCREEN,
                   !m_aRestorePosSize.IsEmpty() ? &m_aRestorePosSize : nullptr );
        m_aRestorePosSize = Rectangle();
    }
}

void GtkSalFrame::StartPresentation( bool bStart )
{
    boost::optional<guint> aWindow;
    boost::optional<Display*> aDisplay;
    if( getDisplay()->IsX11Display() )
    {
        aWindow = widget_get_xid(m_pWindow);
        aDisplay = GDK_DISPLAY_XDISPLAY( getGdkDisplay() );
    }

    m_ScreenSaverInhibitor.inhibit( bStart,
                                    "presentation",
                                    getDisplay()->IsX11Display(),
                                    aWindow,
                                    aDisplay );
}

void GtkSalFrame::SetAlwaysOnTop( bool bOnTop )
{
    if( m_pWindow )
        gtk_window_set_keep_above( GTK_WINDOW( m_pWindow ), bOnTop );
}

void GtkSalFrame::ToTop( sal_uInt16 nFlags )
{
    if( m_pWindow )
    {
        if( isChild( false ) )
            gtk_widget_grab_focus( m_pWindow );
        else if( IS_WIDGET_MAPPED( m_pWindow ) )
        {
            if( ! (nFlags & SAL_FRAME_TOTOP_GRABFOCUS_ONLY) )
                gtk_window_present( GTK_WINDOW(m_pWindow) );
            else
            {
                guint32 nUserTime = GDK_CURRENT_TIME;
                gdk_window_focus( widget_get_window(m_pWindow), nUserTime );
            }
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
        gdk_window_set_cursor( widget_get_window(m_pWindow), pCursor );
        m_pCurrentCursor = pCursor;

        // #i80791# use grabPointer the same way as CaptureMouse, respective float grab
        if( getDisplay()->MouseCaptured( this ) )
            grabPointer( true );
        else if( m_nFloats > 0 )
            grabPointer( true, true );
    }
}

void GtkSalFrame::grabPointer( bool bGrab, bool bOwnerEvents )
{
    static const char* pEnv = getenv( "SAL_NO_MOUSEGRABS" );
    if (pEnv && *pEnv)
        return;

    if (!m_pWindow)
        return;

    const int nMask = (GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK);

    GdkDeviceManager* pDeviceManager = gdk_display_get_device_manager(getGdkDisplay());
    GdkDevice* pPointer = gdk_device_manager_get_client_pointer(pDeviceManager);
    if (bGrab)
        gdk_device_grab(pPointer, widget_get_window(getMouseEventWidget()), GDK_OWNERSHIP_NONE, bOwnerEvents, (GdkEventMask) nMask, m_pCurrentCursor, GDK_CURRENT_TIME);
    else
        gdk_device_ungrab(pPointer, GDK_CURRENT_TIME);
}

void GtkSalFrame::grabKeyboard( bool bGrab )
{
    static const char* pEnv = getenv("SAL_NO_MOUSEGRABS"); // let's not introduce a special var for this
    if (pEnv && *pEnv)
        return;

    if (!m_pWindow)
        return;

    GdkDeviceManager* pDeviceManager = gdk_display_get_device_manager(getGdkDisplay());
    GdkDevice* pPointer = gdk_device_manager_get_client_pointer(pDeviceManager);
    GdkDevice* pKeyboard = gdk_device_get_associated_device(pPointer);
    if (bGrab)
    {
        gdk_device_grab(pKeyboard, widget_get_window(m_pWindow), GDK_OWNERSHIP_NONE,
                        true, (GdkEventMask)(GDK_KEY_PRESS | GDK_KEY_RELEASE), nullptr, GDK_CURRENT_TIME);
    }
    else
    {
        gdk_device_ungrab(pKeyboard, GDK_CURRENT_TIME);
    }
}

void GtkSalFrame::CaptureMouse( bool bCapture )
{
    getDisplay()->CaptureMouse( bCapture ? this : nullptr );
}

void GtkSalFrame::SetPointerPos( long nX, long nY )
{
    GtkSalFrame* pFrame = this;
    while( pFrame && pFrame->isChild( false ) )
        pFrame = pFrame->m_pParent;
    if( ! pFrame )
        return;

    GdkScreen *pScreen = gtk_window_get_screen( GTK_WINDOW(pFrame->m_pWindow) );
    GdkDisplay *pDisplay = gdk_screen_get_display( pScreen );

    /* when the application tries to center the mouse in the dialog the
     * window isn't mapped already. So use coordinates relative to the root window.
     */
    unsigned int nWindowLeft = maGeometry.nX + nX;
    unsigned int nWindowTop  = maGeometry.nY + nY;

    GdkDeviceManager* pManager = gdk_display_get_device_manager(pDisplay);
    gdk_device_warp(gdk_device_manager_get_client_pointer(pManager), pScreen, nWindowLeft, nWindowTop);

    // #i38648# ask for the next motion hint
    gint x, y;
    GdkModifierType mask;
    gdk_window_get_pointer( widget_get_window(pFrame->m_pWindow) , &x, &y, &mask );
}

void GtkSalFrame::Flush()
{
    gdk_display_flush( getGdkDisplay() );
}

#ifndef GDK_Open
#define GDK_Open 0x1008ff6b
#endif
#ifndef GDK_Paste
#define GDK_Paste 0x1008ff6d
#endif
#ifndef GDK_Copy
#define GDK_Copy 0x1008ff57
#endif
#ifndef GDK_Cut
#define GDK_Cut 0x1008ff58
#endif

void GtkSalFrame::KeyCodeToGdkKey(const vcl::KeyCode& rKeyCode,
    guint* pGdkKeyCode, GdkModifierType *pGdkModifiers)
{
    if ( pGdkKeyCode == nullptr || pGdkModifiers == nullptr )
        return;

    // Get GDK key modifiers
    GdkModifierType nModifiers = (GdkModifierType) 0;

    if ( rKeyCode.IsShift() )
        nModifiers = (GdkModifierType) ( nModifiers | GDK_SHIFT_MASK );

    if ( rKeyCode.IsMod1() )
        nModifiers = (GdkModifierType) ( nModifiers | GDK_CONTROL_MASK );

    if ( rKeyCode.IsMod2() )
        nModifiers = (GdkModifierType) ( nModifiers | GDK_MOD1_MASK );

    *pGdkModifiers = nModifiers;

    // Get GDK keycode.
    guint nKeyCode = 0;

    guint nCode = rKeyCode.GetCode();

    if ( nCode >= KEY_0 && nCode <= KEY_9 )
        nKeyCode = ( nCode - KEY_0 ) + GDK_0;
    else if ( nCode >= KEY_A && nCode <= KEY_Z )
        nKeyCode = ( nCode - KEY_A ) + GDK_A;
    else if ( nCode >= KEY_F1 && nCode <= KEY_F26 )
        nKeyCode = ( nCode - KEY_F1 ) + GDK_F1;
    else
    {
        switch( nCode )
        {
        case KEY_DOWN:          nKeyCode = GDK_Down;            break;
        case KEY_UP:            nKeyCode = GDK_Up;              break;
        case KEY_LEFT:          nKeyCode = GDK_Left;            break;
        case KEY_RIGHT:         nKeyCode = GDK_Right;           break;
        case KEY_HOME:          nKeyCode = GDK_Home;            break;
        case KEY_END:           nKeyCode = GDK_End;             break;
        case KEY_PAGEUP:        nKeyCode = GDK_Page_Up;         break;
        case KEY_PAGEDOWN:      nKeyCode = GDK_Page_Down;       break;
        case KEY_RETURN:        nKeyCode = GDK_Return;          break;
        case KEY_ESCAPE:        nKeyCode = GDK_Escape;          break;
        case KEY_TAB:           nKeyCode = GDK_Tab;             break;
        case KEY_BACKSPACE:     nKeyCode = GDK_BackSpace;       break;
        case KEY_SPACE:         nKeyCode = GDK_space;           break;
        case KEY_INSERT:        nKeyCode = GDK_Insert;          break;
        case KEY_DELETE:        nKeyCode = GDK_Delete;          break;
        case KEY_ADD:           nKeyCode = GDK_plus;            break;
        case KEY_SUBTRACT:      nKeyCode = GDK_minus;           break;
        case KEY_MULTIPLY:      nKeyCode = GDK_asterisk;        break;
        case KEY_DIVIDE:        nKeyCode = GDK_slash;           break;
        case KEY_POINT:         nKeyCode = GDK_period;          break;
        case KEY_COMMA:         nKeyCode = GDK_comma;           break;
        case KEY_LESS:          nKeyCode = GDK_less;            break;
        case KEY_GREATER:       nKeyCode = GDK_greater;         break;
        case KEY_EQUAL:         nKeyCode = GDK_equal;           break;
        case KEY_FIND:          nKeyCode = GDK_Find;            break;
        case KEY_CONTEXTMENU:   nKeyCode = GDK_Menu;            break;
        case KEY_HELP:          nKeyCode = GDK_Help;            break;
        case KEY_UNDO:          nKeyCode = GDK_Undo;            break;
        case KEY_REPEAT:        nKeyCode = GDK_Redo;            break;
        case KEY_DECIMAL:       nKeyCode = GDK_KP_Decimal;      break;
        case KEY_TILDE:         nKeyCode = GDK_asciitilde;      break;
        case KEY_QUOTELEFT:     nKeyCode = GDK_quoteleft;       break;
        case KEY_BRACKETLEFT:   nKeyCode = GDK_bracketleft;     break;
        case KEY_BRACKETRIGHT:  nKeyCode = GDK_bracketright;    break;
        case KEY_SEMICOLON:     nKeyCode = GDK_semicolon;       break;
        case KEY_QUOTERIGHT:    nKeyCode = GDK_quoteright;      break;

        // Special cases
        case KEY_COPY:          nKeyCode = GDK_Copy;            break;
        case KEY_CUT:           nKeyCode = GDK_Cut;             break;
        case KEY_PASTE:         nKeyCode = GDK_Paste;           break;
        case KEY_OPEN:          nKeyCode = GDK_Open;            break;
        }
    }

    *pGdkKeyCode = nKeyCode;
}

OUString GtkSalFrame::GetKeyName( sal_uInt16 nKeyCode )
{
    guint nGtkKeyCode;
    GdkModifierType nGtkModifiers;
    KeyCodeToGdkKey(nKeyCode, &nGtkKeyCode, &nGtkModifiers );

    gchar* pName = gtk_accelerator_get_label(nGtkKeyCode, nGtkModifiers);
    OUString aRet(pName, rtl_str_getLength(pName), RTL_TEXTENCODING_UTF8);
    g_free(pName);
    return aRet;
}

GdkDisplay *GtkSalFrame::getGdkDisplay()
{
    return GetGtkSalData()->GetGdkDisplay();
}

GtkSalDisplay *GtkSalFrame::getDisplay()
{
    return GetGtkSalData()->GetGtkDisplay();
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

KeyIndicatorState GtkSalFrame::GetIndicatorState()
{
    KeyIndicatorState nState = KeyIndicatorState::NONE;

    GdkKeymap *pKeyMap = gdk_keymap_get_for_display(getGdkDisplay());

    if (gdk_keymap_get_caps_lock_state(pKeyMap))
        nState |= KeyIndicatorState::CAPSLOCK;
    if (gdk_keymap_get_num_lock_state(pKeyMap))
        nState |= KeyIndicatorState::NUMLOCK;
#if GTK_CHECK_VERSION(3,18,0)
    if (gdk_keymap_get_scroll_lock_state(pKeyMap))
        nState |= KeyIndicatorState::SCROLLLOCK;
#endif

    return nState;
}

void GtkSalFrame::SimulateKeyPress( sal_uInt16 nKeyCode )
{
    g_warning ("missing simulate keypress %d", nKeyCode);
}

void GtkSalFrame::SetInputContext( SalInputContext* pContext )
{
    if( ! pContext )
        return;

    if( ! (pContext->mnOptions & InputContextFlags::Text) )
        return;

    // create a new im context
    if( ! m_pIMHandler )
        m_pIMHandler = new IMHandler( this );
}

void GtkSalFrame::EndExtTextInput( EndExtTextInputFlags nFlags )
{
    if( m_pIMHandler )
        m_pIMHandler->endExtTextInput( nFlags );
}

bool GtkSalFrame::MapUnicodeToKeyCode( sal_Unicode , LanguageType , vcl::KeyCode& )
{
    // not supported yet
    return false;
}

LanguageType GtkSalFrame::GetInputLanguage()
{
    return LANGUAGE_DONTKNOW;
}

void GtkSalFrame::UpdateSettings( AllSettings& rSettings )
{
    if( ! m_pWindow )
        return;

    GtkSalGraphics* pGraphics = m_pGraphics;
    bool bFreeGraphics = false;
    if( ! pGraphics )
    {
        pGraphics = static_cast<GtkSalGraphics*>(AcquireGraphics());
        if ( !pGraphics )
        {
            SAL_WARN("vcl", "Could not get graphics - unable to update settings");
            return;
        }
        bFreeGraphics = true;
    }

    pGraphics->updateSettings( rSettings );

    if( bFreeGraphics )
        ReleaseGraphics( pGraphics );
}

void GtkSalFrame::Beep()
{
    gdk_display_beep( getGdkDisplay() );
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
                                      (m_pParent && ! m_pParent->isChild(true,false)) ? GTK_WINDOW(m_pParent->m_pWindow) : nullptr
                                     );
}

bool GtkSalFrame::SetPluginParent( SystemParentData* pSysParent )
{
    (void)pSysParent;
    //FIXME: no SetPluginParent impl. for gtk3
    return false;
}

void GtkSalFrame::ResetClipRegion()
{
    if( m_pWindow )
        gdk_window_shape_combine_region( widget_get_window( m_pWindow ), nullptr, 0, 0 );
}

void GtkSalFrame::BeginSetClipRegion( sal_uLong )
{
    if( m_pRegion )
        cairo_region_destroy( m_pRegion );
    m_pRegion = cairo_region_create();
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
        cairo_region_union_rectangle( m_pRegion, &aRect );
    }
}

void GtkSalFrame::EndSetClipRegion()
{
    if( m_pWindow && m_pRegion )
        gdk_window_shape_combine_region( widget_get_window(m_pWindow), m_pRegion, 0, 0 );
}

void GtkSalFrame::SetModal(bool bModal)
{
    if (!m_pWindow)
        return;
    gtk_window_set_modal(GTK_WINDOW(m_pWindow), bModal);
    if (bModal)
    {
        //gtk_window_set_modal bTrue adds a grab, so ungrab here. Quite
        //possibly we should alternatively call grab_add grab_ungrab on
        //show/hide of menus ?
        gtk_grab_remove(m_pWindow);
    }
}

gboolean GtkSalFrame::signalTooltipQuery(GtkWidget*, gint /*x*/, gint /*y*/,
                                     gboolean /*keyboard_mode*/, GtkTooltip *tooltip,
                                     gpointer frame)
{
    GtkSalFrame* pThis = static_cast<GtkSalFrame*>(frame);
    if (pThis->m_aTooltip.isEmpty())
        return false;
    gtk_tooltip_set_text(tooltip,
        OUStringToOString(pThis->m_aTooltip, RTL_TEXTENCODING_UTF8).getStr());
    GdkRectangle aHelpArea;
    aHelpArea.x = pThis->m_aHelpArea.Left();
    aHelpArea.y = pThis->m_aHelpArea.Top();
    aHelpArea.width = pThis->m_aHelpArea.GetWidth();
    aHelpArea.height = pThis->m_aHelpArea.GetHeight();
    if (AllSettings::GetLayoutRTL())
        aHelpArea.x = pThis->maGeometry.nWidth-aHelpArea.width-1-aHelpArea.x;
    gtk_tooltip_set_tip_area(tooltip, &aHelpArea);
    return true;
}

bool GtkSalFrame::ShowTooltip(const OUString& rHelpText, const Rectangle& rHelpArea)
{
    m_aTooltip = rHelpText;
    m_aHelpArea = rHelpArea;
    gtk_widget_trigger_tooltip_query(getMouseEventWidget());
    return true;
}

gboolean GtkSalFrame::signalButton( GtkWidget*, GdkEventButton* pEvent, gpointer frame )
{
    GtkSalFrame* pThis = static_cast<GtkSalFrame*>(frame);

    SalMouseEvent aEvent;
    sal_uInt16 nEventType = 0;
    switch( pEvent->type )
    {
        case GDK_BUTTON_PRESS:
            nEventType = SALEVENT_MOUSEBUTTONDOWN;
            break;
        case GDK_BUTTON_RELEASE:
            nEventType = SALEVENT_MOUSEBUTTONUP;
            break;
        default:
            return false;
    }
    switch( pEvent->button )
    {
        case 1: aEvent.mnButton = MOUSE_LEFT;   break;
        case 2: aEvent.mnButton = MOUSE_MIDDLE; break;
        case 3: aEvent.mnButton = MOUSE_RIGHT;  break;
        default: return false;
    }
    aEvent.mnTime   = pEvent->time;
    aEvent.mnX      = (long)pEvent->x_root - pThis->maGeometry.nX;
    aEvent.mnY      = (long)pEvent->y_root - pThis->maGeometry.nY;
    aEvent.mnCode   = GetMouseModCode( pEvent->state );

    bool bClosePopups = false;
    if( pEvent->type == GDK_BUTTON_PRESS &&
        !(pThis->m_nStyle & SalFrameStyleFlags::OWNERDRAWDECORATION)
        )
    {
        if( m_nFloats > 0 )
        {
            // close popups if user clicks outside our application
            gint x, y;
            bClosePopups = (gdk_display_get_window_at_pointer( GtkSalFrame::getGdkDisplay(), &x, &y ) == nullptr);
        }
        /*  #i30306# release implicit pointer grab if no popups are open; else
         *  Drag cannot grab the pointer and will fail.
         */
        if( m_nFloats < 1 || bClosePopups )
            gdk_display_pointer_ungrab( GtkSalFrame::getGdkDisplay(), GDK_CURRENT_TIME );
    }

    if( pThis->m_bWindowIsGtkPlug &&
        pEvent->type == GDK_BUTTON_PRESS &&
        pEvent->button == 1 )
    {
        pThis->askForXEmbedFocus( pEvent->time );
    }

    // --- RTL --- (mirror mouse pos)
    if( AllSettings::GetLayoutRTL() )
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
                if ( !(pSVData->maWinData.mpFirstFloat->GetPopupModeFlags() & FloatWinPopupFlags::NoAppFocusClose) && !(pEnv && *pEnv) )
                    pSVData->maWinData.mpFirstFloat->EndPopupMode( FloatWinPopupEndFlags::Cancel | FloatWinPopupEndFlags::CloseAll );
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
                pThis->CallCallback( SALEVENT_MOVE, nullptr );
            }
        }
    }

    return true;
}

gboolean GtkSalFrame::signalScroll( GtkWidget*, GdkEventScroll* pEvent, gpointer frame )
{
    GtkSalFrame* pThis = static_cast<GtkSalFrame*>(frame);

    SalWheelMouseEvent aEvent;

    aEvent.mnTime = pEvent->time;
    aEvent.mnX = (sal_uLong)pEvent->x;
    // --- RTL --- (mirror mouse pos)
    if (AllSettings::GetLayoutRTL())
        aEvent.mnX = pThis->maGeometry.nWidth - 1 - aEvent.mnX;
    aEvent.mnY = (sal_uLong)pEvent->y;
    aEvent.mnCode = GetMouseModCode( pEvent->state );

    switch (pEvent->direction)
    {
        case GDK_SCROLL_SMOOTH:
            // rhbz#1344042 "Traditionally" in gtk3 we tool a single up/down event as
            // equating to 3 scroll lines and a delta of 120. So scale the delta here
            // by 120 where a single mouse wheel click is an incoming delta_x of 1
            // and divide that by 40 to get the number of scrollines
            if (pEvent->delta_x != 0.0)
            {
                aEvent.mnDelta = -pEvent->delta_x * 120;
                aEvent.mnNotchDelta = aEvent.mnDelta < 0 ? -1 : +1;
                if (aEvent.mnDelta == 0)
                    aEvent.mnDelta = aEvent.mnNotchDelta;
                aEvent.mbHorz = true;
                aEvent.mnScrollLines = std::abs(aEvent.mnDelta) / 40;
                if (aEvent.mnScrollLines == 0)
                    aEvent.mnScrollLines = 1;

                pThis->CallCallback(SALEVENT_WHEELMOUSE, &aEvent);
            }

            if (pEvent->delta_y != 0.0)
            {
                aEvent.mnDelta = -pEvent->delta_y * 120;
                aEvent.mnNotchDelta = aEvent.mnDelta < 0 ? -1 : +1;
                if (aEvent.mnDelta == 0)
                    aEvent.mnDelta = aEvent.mnNotchDelta;
                aEvent.mbHorz = false;
                aEvent.mnScrollLines = std::abs(aEvent.mnDelta) / 40;
                if (aEvent.mnScrollLines == 0)
                    aEvent.mnScrollLines = 1;

                pThis->CallCallback(SALEVENT_WHEELMOUSE, &aEvent);
            }

            break;

        case GDK_SCROLL_UP:
            aEvent.mnDelta = 120;
            aEvent.mnNotchDelta = 1;
            aEvent.mnScrollLines = 3;
            aEvent.mbHorz = false;
            pThis->CallCallback(SALEVENT_WHEELMOUSE, &aEvent);
            break;

        case GDK_SCROLL_DOWN:
            aEvent.mnDelta = -120;
            aEvent.mnNotchDelta = -1;
            aEvent.mnScrollLines = 3;
            aEvent.mbHorz = false;
            pThis->CallCallback(SALEVENT_WHEELMOUSE, &aEvent);
            break;

        case GDK_SCROLL_LEFT:
            aEvent.mnDelta = 120;
            aEvent.mnNotchDelta = 1;
            aEvent.mnScrollLines = 3;
            aEvent.mbHorz = true;
            pThis->CallCallback(SALEVENT_WHEELMOUSE, &aEvent);
            break;

        case GDK_SCROLL_RIGHT:
            aEvent.mnDelta = -120;
            aEvent.mnNotchDelta = -1;
            aEvent.mnScrollLines = 3;
            aEvent.mbHorz = true;
            pThis->CallCallback(SALEVENT_WHEELMOUSE, &aEvent);
            break;
    }

    return true;
}

#if GTK_CHECK_VERSION(3,14,0)
void GtkSalFrame::gestureSwipe(GtkGestureSwipe* gesture, gdouble velocity_x, gdouble velocity_y, gpointer frame)
{
    gdouble x, y;
    GdkEventSequence *sequence = gtk_gesture_single_get_current_sequence(GTK_GESTURE_SINGLE(gesture));
    //I feel I want the first point of the sequence, not the last point which
    //the docs say this gives, but for the moment assume we start and end
    //within the same vcl window
    if (gtk_gesture_get_point(GTK_GESTURE(gesture), sequence, &x, &y))
    {
        GtkSalFrame* pThis = static_cast<GtkSalFrame*>(frame);

        SalSwipeEvent aEvent;
        aEvent.mnVelocityX = velocity_x;
        aEvent.mnVelocityY = velocity_y;
        aEvent.mnX = x;
        aEvent.mnY = y;

        pThis->CallCallback(SALEVENT_SWIPE, &aEvent);
    }
}

void GtkSalFrame::gestureLongPress(GtkGestureLongPress* gesture, gpointer frame)
{
    GtkSalFrame* pThis = static_cast<GtkSalFrame*>(frame);

    if(pThis)
    {
        SalLongPressEvent aEvent;

        gdouble x, y;
        GdkEventSequence *sequence = gtk_gesture_single_get_current_sequence(GTK_GESTURE_SINGLE(gesture));
        gtk_gesture_get_point(GTK_GESTURE(gesture), sequence, &x, &y);
        aEvent.mnX = x;
        aEvent.mnY = y;

        pThis->CallCallback(SALEVENT_LONGPRESS, &aEvent);
    }
}

#endif

gboolean GtkSalFrame::signalMotion( GtkWidget*, GdkEventMotion* pEvent, gpointer frame )
{
    GtkSalFrame* pThis = static_cast<GtkSalFrame*>(frame);

    SalMouseEvent aEvent;
    aEvent.mnTime   = pEvent->time;
    aEvent.mnX      = (long)pEvent->x_root - pThis->maGeometry.nX;
    aEvent.mnY      = (long)pEvent->y_root - pThis->maGeometry.nY;
    aEvent.mnCode   = GetMouseModCode( pEvent->state );
    aEvent.mnButton = 0;

    // --- RTL --- (mirror mouse pos)
    if( AllSettings::GetLayoutRTL() )
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
            pThis->CallCallback( SALEVENT_MOVE, nullptr );
        }

        if( ! aDel.isDeleted() )
        {
            // ask for the next hint
            gint x, y;
            GdkModifierType mask;
            gdk_window_get_pointer( widget_get_window(GTK_WIDGET(pThis->m_pWindow)), &x, &y, &mask );
        }
    }

    return true;
}

gboolean GtkSalFrame::signalCrossing( GtkWidget*, GdkEventCrossing* pEvent, gpointer frame )
{
    GtkSalFrame* pThis = static_cast<GtkSalFrame*>(frame);
    SalMouseEvent aEvent;
    aEvent.mnTime   = pEvent->time;
    aEvent.mnX      = (long)pEvent->x_root - pThis->maGeometry.nX;
    aEvent.mnY      = (long)pEvent->y_root - pThis->maGeometry.nY;
    aEvent.mnCode   = GetMouseModCode( pEvent->state );
    aEvent.mnButton = 0;

    pThis->CallCallback( (pEvent->type == GDK_ENTER_NOTIFY) ? SALEVENT_MOUSEMOVE : SALEVENT_MOUSELEAVE, &aEvent );

    return true;
}

cairo_t* GtkSalFrame::getCairoContext() const
{
    cairo_t* cr = SvpSalGraphics::createCairoContext(m_aFrame);
    assert(cr);
    return cr;
}

void GtkSalFrame::damaged (const basegfx::B2IBox& rDamageRect)
{
#if OSL_DEBUG_LEVEL > 1
    long long area = rDamageRect.getWidth() * rDamageRect.getHeight();
    if( area > 32 * 1024 )
    {
        fprintf( stderr, "bitmap damaged  %d %d (%dx%d) area %lld widget\n",
                  (int) rDamageRect.getMinX(),
                  (int) rDamageRect.getMinY(),
                  (int) rDamageRect.getWidth(),
                 (int) rDamageRect.getHeight(),
                 area );
    }
#endif

    if (dumpframes)
    {
        static int frame;
        OString tmp("/tmp/frame" + OString::number(frame++) + ".png");
        cairo_t* cr = getCairoContext();
        cairo_surface_write_to_png(cairo_get_target(cr), tmp.getStr());
        cairo_destroy(cr);
    }

    gtk_widget_queue_draw_area(GTK_WIDGET(m_pFixedContainer),
                               rDamageRect.getMinX(),
                               rDamageRect.getMinY(),
                               rDamageRect.getWidth(),
                               rDamageRect.getHeight());
}

// blit our backing basebmp buffer to the target cairo context cr
gboolean GtkSalFrame::signalDraw( GtkWidget*, cairo_t *cr, gpointer frame )
{
    GtkSalFrame* pThis = static_cast<GtkSalFrame*>(frame);

    cairo_save(cr);

    cairo_t* source = pThis->getCairoContext();
    cairo_surface_t *pSurface = cairo_get_target(source);

    cairo_set_operator( cr, CAIRO_OPERATOR_OVER );
    cairo_set_source_surface(cr, pSurface, 0, 0);
    cairo_paint(cr);

    cairo_destroy(source);

    cairo_restore(cr);

    cairo_surface_flush(cairo_get_target(cr));

    return false;
}

void GtkSalFrame::sizeAllocated(GtkWidget*, GdkRectangle *pAllocation, gpointer frame)
{
    GtkSalFrame* pThis = static_cast<GtkSalFrame*>(frame);
    pThis->maGeometry.nWidth = pAllocation->width;
    pThis->maGeometry.nHeight = pAllocation->height;
    pThis->AllocateFrame();
    pThis->CallCallback( SALEVENT_RESIZE, nullptr );
    pThis->TriggerPaintEvent();
}

gboolean GtkSalFrame::signalConfigure(GtkWidget*, GdkEventConfigure* pEvent, gpointer frame)
{
    GtkSalFrame* pThis = static_cast<GtkSalFrame*>(frame);

    bool bMoved = false;
    int x = pEvent->x, y = pEvent->y;

    /* #i31785# claims we cannot trust the x,y members of the event;
     * they are e.g. not set correctly on maximize/demaximize;
     * yet the gdkdisplay-x11.c code handling configure_events has
     * done this XTranslateCoordinates work since the day ~zero.
     */
    if( x != pThis->maGeometry.nX || y != pThis->maGeometry.nY )
    {
        bMoved = true;
        pThis->maGeometry.nX = x;
        pThis->maGeometry.nY = y;
    }

    // update decoration hints
    GdkRectangle aRect;
    gdk_window_get_frame_extents( widget_get_window(GTK_WIDGET(pThis->m_pWindow)), &aRect );
    pThis->maGeometry.nTopDecoration    = y - aRect.y;
    pThis->maGeometry.nBottomDecoration = aRect.y + aRect.height - y - pEvent->height;
    pThis->maGeometry.nLeftDecoration   = x - aRect.x;
    pThis->maGeometry.nRightDecoration  = aRect.x + aRect.width - x - pEvent->width;
    pThis->updateScreenNumber();

    if (bMoved)
        pThis->CallCallback(SALEVENT_MOVE, nullptr);

    return false;
}

void GtkSalFrame::TriggerPaintEvent()
{
    //Under gtk2 we can basically paint directly into the XWindow and on
    //additional "expose-event" events we can re-render the missing pieces
    //
    //Under gtk3 we have to keep our own buffer up to date and flush it into
    //the given cairo context on "draw". So we emit a paint event on
    //opportune resize trigger events to initially fill our backbuffer and then
    //keep it up to date with our direct paints and tell gtk those regions
    //have changed and then blit them into the provided cairo context when
    //we get the "draw"
    //
    //The other alternative was to always paint everything on "draw", but
    //that duplicates the amount of drawing and is hideously slow
    SAL_INFO("vcl.gtk3", "force painting" << 0 << "," << 0 << " " << maGeometry.nWidth << "x" << maGeometry.nHeight);
    SalPaintEvent aPaintEvt(0, 0, maGeometry.nWidth, maGeometry.nHeight, true);
    CallCallback(SALEVENT_PAINT, &aPaintEvt);
    gtk_widget_queue_draw(GTK_WIDGET(m_pFixedContainer));
}

gboolean GtkSalFrame::signalFocus( GtkWidget*, GdkEventFocus* pEvent, gpointer frame )
{
    GtkSalFrame* pThis = static_cast<GtkSalFrame*>(frame);

    SalGenericInstance *pSalInstance =
        static_cast< SalGenericInstance* >(GetSalData()->m_pInstance);

    // check if printers have changed (analogous to salframe focus handler)
    pSalInstance->updatePrinterUpdate();

    if( !pEvent->in )
    {
        pThis->m_nKeyModifiers = 0;
        pThis->m_bSendModChangeOnRelease = false;
    }

    if( pThis->m_pIMHandler )
        pThis->m_pIMHandler->focusChanged( pEvent->in );

    // ask for changed printers like generic implementation
    if( pEvent->in && pSalInstance->isPrinterInit() )
        pSalInstance->updatePrinterUpdate();

    // FIXME: find out who the hell steals the focus from our frame
    // while we have the pointer grabbed, this should not come from
    // the window manager. Is this an event that was still queued ?
    // The focus does not seem to get set inside our process

    // in the meantime do not propagate focus get/lose if floats are open
    if( m_nFloats == 0 )
        pThis->CallCallback( pEvent->in ? SALEVENT_GETFOCUS : SALEVENT_LOSEFOCUS, nullptr );

    return false;
}

gboolean GtkSalFrame::signalMap( GtkWidget *pWidget, GdkEvent*, gpointer frame )
{
    GtkSalFrame* pThis = static_cast<GtkSalFrame*>(frame);

    bool bSetFocus = pThis->m_bSetFocusOnMap;
    pThis->m_bSetFocusOnMap = false;

    (void)pWidget; (void)bSetFocus;
    //FIXME: no set input focus ...

    pThis->CallCallback( SALEVENT_RESIZE, nullptr );
    pThis->TriggerPaintEvent();

    return false;
}

gboolean GtkSalFrame::signalUnmap( GtkWidget*, GdkEvent*, gpointer frame )
{
    GtkSalFrame* pThis = static_cast<GtkSalFrame*>(frame);

    pThis->CallCallback( SALEVENT_RESIZE, nullptr );

    return false;
}

gboolean GtkSalFrame::signalKey( GtkWidget*, GdkEventKey* pEvent, gpointer frame )
{
    GtkSalFrame* pThis = static_cast<GtkSalFrame*>(frame);

    vcl::DeletionListener aDel( pThis );

    if( pThis->m_pIMHandler )
    {
        if( pThis->m_pIMHandler->handleKeyEvent( pEvent ) )
            return true;
    }

    // handle modifiers
    if( pEvent->keyval == GDK_Shift_L || pEvent->keyval == GDK_Shift_R ||
        pEvent->keyval == GDK_Control_L || pEvent->keyval == GDK_Control_R ||
        pEvent->keyval == GDK_Alt_L || pEvent->keyval == GDK_Alt_R ||
        pEvent->keyval == GDK_Meta_L || pEvent->keyval == GDK_Meta_R ||
        pEvent->keyval == GDK_Super_L || pEvent->keyval == GDK_Super_R )
    {
        SalKeyModEvent aModEvt;

        sal_uInt16 nModCode = GetKeyModCode( pEvent->state );

        aModEvt.mnModKeyCode = 0; // emit no MODKEYCHANGE events
        if( pEvent->type == GDK_KEY_PRESS && !pThis->m_nKeyModifiers )
            pThis->m_bSendModChangeOnRelease = true;

        else if( pEvent->type == GDK_KEY_RELEASE &&
                 pThis->m_bSendModChangeOnRelease )
        {
            aModEvt.mnModKeyCode = pThis->m_nKeyModifiers;
        }

        sal_uInt16 nExtModMask = 0;
        sal_uInt16 nModMask = 0;
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
                nModMask = KEY_MOD2;
                break;
            case GDK_Alt_R:
                nExtModMask = MODKEY_RMOD2;
                nModMask = KEY_MOD2;
                break;
            case GDK_Shift_L:
                nExtModMask = MODKEY_LSHIFT;
                nModMask = KEY_SHIFT;
                break;
            case GDK_Shift_R:
                nExtModMask = MODKEY_RSHIFT;
                nModMask = KEY_SHIFT;
                break;
            // Map Meta/Super to MOD3 modifier on all Unix systems
            // except Mac OS X
            case GDK_Meta_L:
            case GDK_Super_L:
                nExtModMask = MODKEY_LMOD3;
                nModMask = KEY_MOD3;
                break;
            case GDK_Meta_R:
            case GDK_Super_R:
                nExtModMask = MODKEY_RMOD3;
                nModMask = KEY_MOD3;
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
        aModEvt.mnModKeyCode = pThis->m_nKeyModifiers;

        pThis->CallCallback( SALEVENT_KEYMODCHANGE, &aModEvt );

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
            pThis->m_bSendModChangeOnRelease = false;
    }

    if( !aDel.isDeleted() && pThis->m_pIMHandler )
        pThis->m_pIMHandler->updateIMSpotLocation();

    return true;
}

gboolean GtkSalFrame::signalDelete( GtkWidget*, GdkEvent*, gpointer frame )
{
    GtkSalFrame* pThis = static_cast<GtkSalFrame*>(frame);

    //If we went into the backdrop we disabled the toplevel window, if we
    //receive a delete here, re-enable so we can process it
    bool bBackDrop = (gtk_widget_get_state_flags(GTK_WIDGET(pThis->m_pWindow)) & GTK_STATE_FLAG_BACKDROP);
    if (bBackDrop)
        pThis->GetWindow()->Enable();

    pThis->CallCallback( SALEVENT_CLOSE, nullptr );

    return true;
}

void GtkSalFrame::signalStyleUpdated(GtkWidget*, gpointer frame)
{
    GtkSalFrame* pThis = static_cast<GtkSalFrame*>(frame);

    // note: settings changed for multiple frames is avoided in winproc.cxx ImplHandleSettings
    GtkSalFrame::getDisplay()->SendInternalEvent( pThis, nullptr, SALEVENT_SETTINGSCHANGED );

    // fire off font-changed when the system cairo font hints change
    GtkInstance *pInstance = static_cast<GtkInstance*>(GetSalData()->m_pInstance);
    const cairo_font_options_t* pLastCairoFontOptions = pInstance->GetLastSeenCairoFontOptions();
    const cairo_font_options_t* pCurrentCairoFontOptions = gdk_screen_get_font_options(gdk_screen_get_default());
    bool bFontSettingsChanged = true;
    if (pLastCairoFontOptions && pCurrentCairoFontOptions)
        bFontSettingsChanged = !cairo_font_options_equal(pLastCairoFontOptions, pCurrentCairoFontOptions);
    else if (!pLastCairoFontOptions && !pCurrentCairoFontOptions)
        bFontSettingsChanged = false;
    if (bFontSettingsChanged)
    {
        pInstance->ResetLastSeenCairoFontOptions();
        GtkSalFrame::getDisplay()->SendInternalEvent( pThis, nullptr, SALEVENT_FONTCHANGED );
    }
}

gboolean GtkSalFrame::signalWindowState( GtkWidget*, GdkEvent* pEvent, gpointer frame )
{
    GtkSalFrame* pThis = static_cast<GtkSalFrame*>(frame);
    if( (pThis->m_nState & GDK_WINDOW_STATE_ICONIFIED) != (pEvent->window_state.new_window_state & GDK_WINDOW_STATE_ICONIFIED ) )
    {
        GtkSalFrame::getDisplay()->SendInternalEvent( pThis, nullptr, SALEVENT_RESIZE );
        pThis->TriggerPaintEvent();
    }

    if(   (pEvent->window_state.new_window_state & GDK_WINDOW_STATE_MAXIMIZED) &&
        ! (pThis->m_nState & GDK_WINDOW_STATE_MAXIMIZED) )
    {
        pThis->m_aRestorePosSize = GetPosAndSize(GTK_WINDOW(pThis->m_pWindow));
    }
    pThis->m_nState = pEvent->window_state.new_window_state;

    #if OSL_DEBUG_LEVEL > 1
    if( (pEvent->window_state.changed_mask & GDK_WINDOW_STATE_FULLSCREEN) )
    {
        fprintf( stderr, "window %p %s full screen state\n",
            pThis,
            (pEvent->window_state.new_window_state & GDK_WINDOW_STATE_FULLSCREEN) ? "enters" : "leaves");
    }
    #endif

    return false;
}

gboolean GtkSalFrame::signalVisibility( GtkWidget*, GdkEventVisibility* pEvent, gpointer frame )
{
    GtkSalFrame* pThis = static_cast<GtkSalFrame*>(frame);
    pThis->m_nVisibility = pEvent->state;
    return true;
}

namespace
{
    GdkDragAction VclToGdk(sal_Int8 dragOperation)
    {
        GdkDragAction eRet(static_cast<GdkDragAction>(0));
        if (dragOperation & css::datatransfer::dnd::DNDConstants::ACTION_COPY)
            eRet = static_cast<GdkDragAction>(eRet | GDK_ACTION_COPY);
        if (dragOperation & css::datatransfer::dnd::DNDConstants::ACTION_MOVE)
            eRet = static_cast<GdkDragAction>(eRet | GDK_ACTION_MOVE);
        if (dragOperation & css::datatransfer::dnd::DNDConstants::ACTION_LINK)
            eRet = static_cast<GdkDragAction>(eRet | GDK_ACTION_LINK);
        return eRet;
    }

    sal_Int8 GdkToVcl(GdkDragAction dragOperation)
    {
        sal_Int8 nRet(0);
        if (dragOperation & GDK_ACTION_COPY)
            nRet |= css::datatransfer::dnd::DNDConstants::ACTION_COPY;
        if (dragOperation & GDK_ACTION_MOVE)
            nRet |= css::datatransfer::dnd::DNDConstants::ACTION_MOVE;
        if (dragOperation & GDK_ACTION_LINK)
            nRet |= css::datatransfer::dnd::DNDConstants::ACTION_LINK;
        return nRet;
    }
}

class GtkDropTargetDropContext : public cppu::WeakImplHelper<css::datatransfer::dnd::XDropTargetDropContext>
{
    GdkDragContext *m_pContext;
    guint m_nTime;
public:
    GtkDropTargetDropContext(GdkDragContext *pContext, guint nTime)
        : m_pContext(pContext)
        , m_nTime(nTime)
    {
    }

    // XDropTargetDropContext
    virtual void SAL_CALL acceptDrop(sal_Int8 dragOperation) throw(std::exception) override
    {
        GdkDragAction eAct(static_cast<GdkDragAction>(0));

        if (dragOperation & css::datatransfer::dnd::DNDConstants::ACTION_MOVE)
            eAct = GDK_ACTION_MOVE;
        else if (dragOperation & css::datatransfer::dnd::DNDConstants::ACTION_COPY)
            eAct = GDK_ACTION_COPY;
        else if (dragOperation & css::datatransfer::dnd::DNDConstants::ACTION_LINK)
            eAct = GDK_ACTION_LINK;

        gdk_drag_status(m_pContext, eAct, m_nTime);
    }

    virtual void SAL_CALL rejectDrop() throw(std::exception) override
    {
        gdk_drag_status(m_pContext, static_cast<GdkDragAction>(0), m_nTime);
    }

    virtual void SAL_CALL dropComplete(sal_Bool bSuccess) throw(std::exception) override
    {
        gtk_drag_finish(m_pContext, bSuccess, false, m_nTime);
    }
};

class GtkDnDTransferable : public GtkTransferable
{
    GdkDragContext *m_pContext;
    guint m_nTime;
    GtkWidget *m_pWidget;
    GtkSalFrame *m_pFrame;
    GMainLoop *m_pLoop;
    GtkSelectionData *m_pData;
public:
    GtkDnDTransferable(GdkDragContext *pContext, guint nTime, GtkWidget *pWidget, GtkSalFrame *pFrame)
        : m_pContext(pContext)
        , m_nTime(nTime)
        , m_pWidget(pWidget)
        , m_pFrame(pFrame)
        , m_pLoop(nullptr)
        , m_pData(nullptr)
    {
    }

    virtual css::uno::Any SAL_CALL getTransferData(const css::datatransfer::DataFlavor& rFlavor)
        throw(css::datatransfer::UnsupportedFlavorException,
              css::io::IOException,
              css::uno::RuntimeException, std::exception) override
    {
        css::datatransfer::DataFlavor aFlavor(rFlavor);
        if (aFlavor.MimeType == "text/plain;charset=utf-16")
            aFlavor.MimeType = "text/plain;charset=utf-8";

        auto it = m_aMimeTypeToAtom.find(aFlavor.MimeType);
        if (it == m_aMimeTypeToAtom.end())
            return css::uno::Any();

        /* like gtk_clipboard_wait_for_contents run a sub loop
         * waiting for drag-data-received triggered from
         * gtk_drag_get_data
         */
        {
            m_pLoop = g_main_loop_new(nullptr, true);
            m_pFrame->SetFormatConversionRequest(this);

            gtk_drag_get_data(m_pWidget, m_pContext, it->second, m_nTime);

            if (g_main_loop_is_running(m_pLoop))
            {
                gdk_threads_leave();
                g_main_loop_run(m_pLoop);
                gdk_threads_enter();
            }

            g_main_loop_unref(m_pLoop);
            m_pLoop = nullptr;
            m_pFrame->SetFormatConversionRequest(nullptr);
        }

        css::uno::Any aRet;

        if (aFlavor.MimeType == "text/plain;charset=utf-8")
        {
            OUString aStr;
            gchar *pText = reinterpret_cast<gchar*>(gtk_selection_data_get_text(m_pData));
            if (pText)
                aStr = OUString(pText, rtl_str_getLength(pText), RTL_TEXTENCODING_UTF8);
            g_free(pText);
            aRet <<= aStr.replaceAll("\r\n", "\n");
        }
        else
        {
            gint length(0);
            const guchar *rawdata = gtk_selection_data_get_data_with_length(m_pData,
                                                                            &length);
            css::uno::Sequence<sal_Int8> aSeq(reinterpret_cast<const sal_Int8*>(rawdata), length);
            aRet <<= aSeq;
        }

        gtk_selection_data_free(m_pData);

        return aRet;
    }

    virtual std::vector<css::datatransfer::DataFlavor> getTransferDataFlavorsAsVector() override
    {
        std::vector<GdkAtom> targets;
        for (GList* l = gdk_drag_context_list_targets(m_pContext); l; l = l->next)
            targets.push_back(static_cast<GdkAtom>(l->data));
        return GtkTransferable::getTransferDataFlavorsAsVector(targets.data(), targets.size());
    }

    void LoopEnd(GtkSelectionData *pData)
    {
        m_pData = pData;
        g_main_loop_quit(m_pLoop);
    }
};

// For LibreOffice internal D&D we provide the Transferable without Gtk
// intermediaries as a shortcut, see tdf#100097 for how dbaccess depends on this
GtkDragSource* GtkDragSource::g_ActiveDragSource;

gboolean GtkSalFrame::signalDragDrop(GtkWidget* pWidget, GdkDragContext* context, gint x, gint y, guint time, gpointer frame)
{
    GtkSalFrame* pThis = static_cast<GtkSalFrame*>(frame);

    if (!pThis->m_pDropTarget)
        return false;

    css::datatransfer::dnd::DropTargetDropEvent aEvent;
    aEvent.Source = static_cast<css::datatransfer::dnd::XDropTarget*>(pThis->m_pDropTarget);
    aEvent.Context = new GtkDropTargetDropContext(context, time);
    aEvent.LocationX = x;
    aEvent.LocationY = y;
    aEvent.DropAction = GdkToVcl(gdk_drag_context_get_selected_action(context));
    aEvent.SourceActions = GdkToVcl(gdk_drag_context_get_actions(context));
    css::uno::Reference<css::datatransfer::XTransferable> xTransferable;
    // For LibreOffice internal D&D we provide the Transferable without Gtk
    // intermediaries as a shortcut, see tdf#100097 for how dbaccess depends on this
    if (GtkDragSource::g_ActiveDragSource)
        xTransferable = GtkDragSource::g_ActiveDragSource->GetTransferrable();
    else
        xTransferable = new GtkDnDTransferable(context, time, pWidget, pThis);
    aEvent.Transferable = xTransferable;

    pThis->m_pDropTarget->fire_drop(aEvent);

    return true;
}

class GtkDropTargetDragContext : public cppu::WeakImplHelper<css::datatransfer::dnd::XDropTargetDragContext>
{
    GdkDragContext *m_pContext;
    guint m_nTime;
public:
    GtkDropTargetDragContext(GdkDragContext *pContext, guint nTime)
        : m_pContext(pContext)
        , m_nTime(nTime)
    {
    }

    virtual void SAL_CALL acceptDrag(sal_Int8 dragOperation) throw(std::exception) override
    {
        GdkDragAction eAct(static_cast<GdkDragAction>(0));

        if (dragOperation & css::datatransfer::dnd::DNDConstants::ACTION_MOVE)
            eAct = GDK_ACTION_MOVE;
        else if (dragOperation & css::datatransfer::dnd::DNDConstants::ACTION_COPY)
            eAct = GDK_ACTION_COPY;
        else if (dragOperation & css::datatransfer::dnd::DNDConstants::ACTION_LINK)
            eAct = GDK_ACTION_LINK;

        gdk_drag_status(m_pContext, eAct, m_nTime);
    }

    virtual void SAL_CALL rejectDrag() throw(std::exception) override
    {
        gdk_drag_status(m_pContext, static_cast<GdkDragAction>(0), m_nTime);
    }
};

void GtkSalFrame::signalDragDropReceived(GtkWidget* /*pWidget*/, GdkDragContext * /*context*/, gint /*x*/, gint /*y*/, GtkSelectionData* data, guint /*ttype*/, guint /*time*/, gpointer frame)
{
    GtkSalFrame* pThis = static_cast<GtkSalFrame*>(frame);

    /*
     * If we get a drop, then we will call like gtk_clipboard_wait_for_contents
     * with a loop inside a loop to get the right format, so if this is the
     * case return to the outer loop here with a copy of the desired data
     *
     * don't look at me like that.
     */
    if (!pThis->m_pFormatConversionRequest)
        return;

    pThis->m_pFormatConversionRequest->LoopEnd(gtk_selection_data_copy(data));
}

gboolean GtkSalFrame::signalDragMotion(GtkWidget *pWidget, GdkDragContext *context, gint x, gint y, guint time, gpointer frame)
{
    GtkSalFrame* pThis = static_cast<GtkSalFrame*>(frame);

    if (!pThis->m_pDropTarget)
        return false;

    if (!pThis->m_bInDrag)
        gtk_drag_highlight(pWidget);

    css::datatransfer::dnd::DropTargetDragEnterEvent aEvent;
    aEvent.Source = static_cast<css::datatransfer::dnd::XDropTarget*>(pThis->m_pDropTarget);
    GtkDropTargetDragContext* pContext = new GtkDropTargetDragContext(context, time);
    //preliminary accept the Drag and select the preferred action, the fire_* will
    //inform the original caller of our choice and the callsite can decide
    //to overrule this choice. i.e. typically here we default to ACTION_MOVE
    pContext->acceptDrag(GdkToVcl(gdk_drag_context_get_actions(context)));
    aEvent.Context = pContext;
    aEvent.LocationX = x;
    aEvent.LocationY = y;
    aEvent.DropAction = GdkToVcl(gdk_drag_context_get_selected_action(context));
    aEvent.SourceActions = GdkToVcl(gdk_drag_context_get_actions(context));

    if (!pThis->m_bInDrag)
    {
        css::uno::Reference<css::datatransfer::XTransferable> xTransferable;
        // For LibreOffice internal D&D we provide the Transferable without Gtk
        // intermediaries as a shortcut, see tdf#100097 for how dbaccess depends on this
        if (GtkDragSource::g_ActiveDragSource)
            xTransferable = GtkDragSource::g_ActiveDragSource->GetTransferrable();
        else
            xTransferable = new GtkDnDTransferable(context, time, pWidget, pThis);
        css::uno::Sequence<css::datatransfer::DataFlavor> aFormats = xTransferable->getTransferDataFlavors();
        aEvent.SupportedDataFlavors = aFormats;
        pThis->m_pDropTarget->fire_dragEnter(aEvent);
        pThis->m_bInDrag = true;
    }
    else
    {
        pThis->m_pDropTarget->fire_dragOver(aEvent);
    }

    return true;
}

void GtkSalFrame::signalDragLeave(GtkWidget *pWidget, GdkDragContext * /*context*/, guint /*time*/, gpointer frame)
{
    GtkSalFrame* pThis = static_cast<GtkSalFrame*>(frame);
    if (!pThis->m_pDropTarget)
        return;
    pThis->m_bInDrag = false;
    gtk_drag_unhighlight(pWidget);

#if 0
    css::datatransfer::dnd::DropTargetEvent aEvent;
    aEvent.Source = static_cast<css::datatransfer::dnd::XDropTarget*>(pThis->m_pDropTarget);
    pThis->m_pDropTarget->fire_dragExit(aEvent);
#endif
}

void GtkSalFrame::signalDestroy( GtkWidget* pObj, gpointer frame )
{
    GtkSalFrame* pThis = static_cast<GtkSalFrame*>(frame);
    if( pObj == pThis->m_pWindow )
    {
        pThis->m_pFixedContainer = nullptr;
        pThis->m_pEventBox = nullptr;
        pThis->m_pWindow = nullptr;
        pThis->InvalidateGraphics();
    }
}

// GtkSalFrame::IMHandler

GtkSalFrame::IMHandler::IMHandler( GtkSalFrame* pFrame )
: m_pFrame(pFrame),
  m_nPrevKeyPresses( 0 ),
  m_pIMContext( nullptr ),
  m_bFocused( true ),
  m_bPreeditJustChanged( false )
{
    m_aInputEvent.mpTextAttr = nullptr;
    createIMContext();
}

GtkSalFrame::IMHandler::~IMHandler()
{
    // cancel an eventual event posted to begin preedit again
    GtkSalFrame::getDisplay()->CancelInternalEvent( m_pFrame, &m_aInputEvent, SALEVENT_EXTTEXTINPUT );
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

        GetGenericData()->ErrorTrapPush();
        gtk_im_context_set_client_window( m_pIMContext, widget_get_window(GTK_WIDGET(m_pFrame->m_pWindow)) );
        gtk_im_context_focus_in( m_pIMContext );
        GetGenericData()->ErrorTrapPop();
        m_bFocused = true;
   }
}

void GtkSalFrame::IMHandler::deleteIMContext()
{
    if( m_pIMContext )
    {
        // first give IC a chance to deinitialize
        GetGenericData()->ErrorTrapPush();
        gtk_im_context_set_client_window( m_pIMContext, nullptr );
        GetGenericData()->ErrorTrapPop();
        // destroy old IC
        g_object_unref( m_pIMContext );
        m_pIMContext = nullptr;
    }
}

void GtkSalFrame::IMHandler::doCallEndExtTextInput()
{
    m_aInputEvent.mpTextAttr = nullptr;
    m_pFrame->CallCallback( SALEVENT_ENDEXTTEXTINPUT, nullptr );
}

void GtkSalFrame::IMHandler::updateIMSpotLocation()
{
    SalExtTextInputPosEvent aPosEvent;
    m_pFrame->CallCallback( SALEVENT_EXTTEXTINPUTPOS, static_cast<void*>(&aPosEvent) );
    GdkRectangle aArea;
    aArea.x = aPosEvent.mnX;
    aArea.y = aPosEvent.mnY;
    aArea.width = aPosEvent.mnWidth;
    aArea.height = aPosEvent.mnHeight;
    GetGenericData()->ErrorTrapPush();
    gtk_im_context_set_cursor_location( m_pIMContext, &aArea );
    GetGenericData()->ErrorTrapPop();
}

void GtkSalFrame::IMHandler::sendEmptyCommit()
{
    vcl::DeletionListener aDel( m_pFrame );

    SalExtTextInputEvent aEmptyEv;
    aEmptyEv.mnTime             = 0;
    aEmptyEv.mpTextAttr         = nullptr;
    aEmptyEv.maText.clear();
    aEmptyEv.mnCursorPos        = 0;
    aEmptyEv.mnCursorFlags      = 0;
    aEmptyEv.mbOnlyCursor       = False;
    m_pFrame->CallCallback( SALEVENT_EXTTEXTINPUT, static_cast<void*>(&aEmptyEv) );
    if( ! aDel.isDeleted() )
        m_pFrame->CallCallback( SALEVENT_ENDEXTTEXTINPUT, nullptr );
}

void GtkSalFrame::IMHandler::endExtTextInput( EndExtTextInputFlags /*nFlags*/ )
{
    gtk_im_context_reset ( m_pIMContext );

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
                GtkSalFrame::getDisplay()->SendInternalEvent( m_pFrame, &m_aInputEvent, SALEVENT_EXTTEXTINPUT );
            }
        }
    }
}

void GtkSalFrame::IMHandler::focusChanged( bool bFocusIn )
{
    m_bFocused = bFocusIn;
    if( bFocusIn )
    {
        GetGenericData()->ErrorTrapPush();
        gtk_im_context_focus_in( m_pIMContext );
        GetGenericData()->ErrorTrapPop();
        if( m_aInputEvent.mpTextAttr )
        {
            sendEmptyCommit();
            // begin preedit again
            GtkSalFrame::getDisplay()->SendInternalEvent( m_pFrame, &m_aInputEvent, SALEVENT_EXTTEXTINPUT );
        }
    }
    else
    {
        GetGenericData()->ErrorTrapPush();
        gtk_im_context_focus_out( m_pIMContext );
        GetGenericData()->ErrorTrapPop();
        // cancel an eventual event posted to begin preedit again
        GtkSalFrame::getDisplay()->CancelInternalEvent( m_pFrame, &m_aInputEvent, SALEVENT_EXTTEXTINPUT );
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

        m_bPreeditJustChanged = false;

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

        m_bPreeditJustChanged = false;

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
* single key hack for enter or space if the unicode committed does not match
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

void GtkSalFrame::IMHandler::signalIMCommit( GtkIMContext* pContext, gchar* pText, gpointer im_handler )
{
    GtkSalFrame::IMHandler* pThis = static_cast<GtkSalFrame::IMHandler*>(im_handler);

    SolarMutexGuard aGuard;
    vcl::DeletionListener aDel( pThis->m_pFrame );
    {
        const bool bWasPreedit =
            (pThis->m_aInputEvent.mpTextAttr != nullptr) ||
            pThis->m_bPreeditJustChanged;

        pThis->m_aInputEvent.mnTime             = 0;
        pThis->m_aInputEvent.mpTextAttr         = nullptr;
        pThis->m_aInputEvent.maText             = OUString( pText, strlen(pText), RTL_TEXTENCODING_UTF8 );
        pThis->m_aInputEvent.mnCursorPos        = pThis->m_aInputEvent.maText.getLength();
        pThis->m_aInputEvent.mnCursorFlags      = 0;
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
            && pThis->m_aInputEvent.maText.getLength() == 1
            && ! pThis->m_aPrevKeyPresses.empty()
            )
        {
            const PreviousKeyPress& rKP = pThis->m_aPrevKeyPresses.back();
            sal_Unicode aOrigCode = pThis->m_aInputEvent.maText[0];

            if( checkSingleKeyCommitHack( rKP.keyval, aOrigCode ) )
            {
                pThis->m_pFrame->doKeyCallback( rKP.state, rKP.keyval, rKP.hardware_keycode, rKP.group, rKP.time, aOrigCode, true, true );
                bSingleCommit = true;
            }
        }
        if( ! bSingleCommit )
        {
            pThis->m_pFrame->CallCallback( SALEVENT_EXTTEXTINPUT, static_cast<void*>(&pThis->m_aInputEvent));
            if( ! aDel.isDeleted() )
                pThis->doCallEndExtTextInput();
        }
        if( ! aDel.isDeleted() )
        {
            // reset input event
            pThis->m_aInputEvent.maText.clear();
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
#else
    (void) pContext;
#endif
}

void GtkSalFrame::IMHandler::signalIMPreeditChanged( GtkIMContext*, gpointer im_handler )
{
    GtkSalFrame::IMHandler* pThis = static_cast<GtkSalFrame::IMHandler*>(im_handler);

    char*           pText           = nullptr;
    PangoAttrList*  pAttrs          = nullptr;
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
        if( pThis->m_aInputEvent.maText.getLength() == 0 )
        {
            g_free( pText );
            pango_attr_list_unref( pAttrs );
            return;
        }
    }

    pThis->m_bPreeditJustChanged = true;

    bool bEndPreedit = (!pText || !*pText) && pThis->m_aInputEvent.mpTextAttr != nullptr;
    pThis->m_aInputEvent.mnTime             = 0;
    pThis->m_aInputEvent.maText             = pText ? OUString( pText, strlen(pText), RTL_TEXTENCODING_UTF8 ) : OUString();
    pThis->m_aInputEvent.mnCursorPos        = nCursorPos;
    pThis->m_aInputEvent.mnCursorFlags      = 0;
    pThis->m_aInputEvent.mbOnlyCursor       = False;

    pThis->m_aInputFlags = std::vector<sal_uInt16>( std::max( 1, (int)pThis->m_aInputEvent.maText.getLength() ), 0 );

    PangoAttrIterator *iter = pango_attr_list_get_iterator(pAttrs);
    do
    {
        GSList *attr_list = nullptr;
        GSList *tmp_list = nullptr;
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
            PangoAttribute *pango_attr = static_cast<PangoAttribute *>(tmp_list->data);

            switch (pango_attr->klass->type)
            {
                case PANGO_ATTR_BACKGROUND:
                    sal_attr |= (EXTTEXTINPUT_ATTR_HIGHLIGHT | EXTTEXTINPUT_CURSOR_INVISIBLE);
                    break;
                case PANGO_ATTR_UNDERLINE:
                    sal_attr |= EXTTEXTINPUT_ATTR_UNDERLINE;
                    break;
                case PANGO_ATTR_STRIKETHROUGH:
                    sal_attr |= EXTTEXTINPUT_ATTR_REDTEXT;
                    break;
                default:
                    break;
            }
            pango_attribute_destroy (pango_attr);
            tmp_list = tmp_list->next;
        }
        if (sal_attr == 0)
            sal_attr |= EXTTEXTINPUT_ATTR_UNDERLINE;
        g_slist_free (attr_list);

        // Set the sal attributes on our text
        for (int i = start; i < end; ++i)
        {
            SAL_WARN_IF(i >= static_cast<int>(pThis->m_aInputFlags.size()),
                "vcl.gtk", "pango attrib out of range. Broken range: "
                << start << "," << end << " Legal range: 0,"
                << pThis->m_aInputFlags.size());
            if (i >= static_cast<int>(pThis->m_aInputFlags.size()))
                continue;
            pThis->m_aInputFlags[i] |= sal_attr;
        }
    } while (pango_attr_iterator_next (iter));
    pango_attr_iterator_destroy(iter);

    pThis->m_aInputEvent.mpTextAttr = &pThis->m_aInputFlags[0];

    g_free( pText );
    pango_attr_list_unref( pAttrs );

    SolarMutexGuard aGuard;
    vcl::DeletionListener aDel( pThis->m_pFrame );

    pThis->m_pFrame->CallCallback( SALEVENT_EXTTEXTINPUT, static_cast<void*>(&pThis->m_aInputEvent));
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
    GtkSalFrame::IMHandler* pThis = static_cast<GtkSalFrame::IMHandler*>(im_handler);

    pThis->m_bPreeditJustChanged = true;

    SolarMutexGuard aGuard;
    vcl::DeletionListener aDel( pThis->m_pFrame );
    pThis->doCallEndExtTextInput();
    if( ! aDel.isDeleted() )
        pThis->updateIMSpotLocation();
}

uno::Reference<accessibility::XAccessibleEditableText>
    FindFocus(uno::Reference< accessibility::XAccessibleContext > xContext)
{
    if (!xContext.is())
        return uno::Reference< accessibility::XAccessibleEditableText >();

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

static uno::Reference<accessibility::XAccessibleEditableText> lcl_GetxText(vcl::Window *pFocusWin)
{
    uno::Reference<accessibility::XAccessibleEditableText> xText;
    try
    {
        uno::Reference< accessibility::XAccessible > xAccessible( pFocusWin->GetAccessible() );
        if (xAccessible.is())
            xText = FindFocus(xAccessible->getAccessibleContext());
    }
    catch(const uno::Exception& e)
    {
        SAL_WARN( "vcl.gtk", "Exception in getting input method surrounding text: " << e.Message);
    }
    return xText;
}

gboolean GtkSalFrame::IMHandler::signalIMRetrieveSurrounding( GtkIMContext* pContext, gpointer /*im_handler*/ )
{
    vcl::Window *pFocusWin = Application::GetFocusWindow();
    if (!pFocusWin)
        return true;

    uno::Reference<accessibility::XAccessibleEditableText> xText = lcl_GetxText(pFocusWin);
    if (xText.is())
    {
        sal_Int32 nPosition = xText->getCaretPosition();
        OUString sAllText = xText->getText();
        OString sUTF = OUStringToOString(sAllText, RTL_TEXTENCODING_UTF8);
        OUString sCursorText(sAllText.copy(0, nPosition));
        gtk_im_context_set_surrounding(pContext, sUTF.getStr(), sUTF.getLength(),
            OUStringToOString(sCursorText, RTL_TEXTENCODING_UTF8).getLength());
        return true;
    }

    return false;
}

gboolean GtkSalFrame::IMHandler::signalIMDeleteSurrounding( GtkIMContext*, gint offset, gint nchars,
    gpointer /*im_handler*/ )
{
    vcl::Window *pFocusWin = Application::GetFocusWindow();
    if (!pFocusWin)
        return true;

    uno::Reference<accessibility::XAccessibleEditableText> xText = lcl_GetxText(pFocusWin);
    if (xText.is())
    {
        sal_Int32 nPosition = xText->getCaretPosition();
        // #i111768# range checking
        sal_Int32 nDeletePos = nPosition + offset;
        sal_Int32 nDeleteEnd = nDeletePos + nchars;
        if (nDeletePos < 0)
            nDeletePos = 0;
        if (nDeleteEnd < 0)
            nDeleteEnd = 0;
        if (nDeleteEnd > xText->getCharacterCount())
            nDeleteEnd = xText->getCharacterCount();

        xText->deleteText(nDeletePos, nDeleteEnd);
        //tdf91641 adjust cursor if deleted chars shift it forward (normal case)
        if (nDeletePos < nPosition)
        {
            if (nDeleteEnd <= nPosition)
                nPosition = nPosition - (nDeleteEnd - nDeletePos);
            else
                nPosition = nDeletePos;

            if (xText->getCharacterCount() >= nPosition)
                xText->setCaretPosition( nPosition );
        }
        return true;
    }

    return false;
}

Size GtkSalDisplay::GetScreenSize( int nDisplayScreen )
{
    Rectangle aRect = m_pSys->GetDisplayScreenPosSizePixel( nDisplayScreen );
    return Size( aRect.GetWidth(), aRect.GetHeight() );
}

sal_uIntPtr GtkSalFrame::GetNativeWindowHandle(GtkWidget *pWidget)
{
    (void) this;                // Silence loplugin:staticmethods
    GdkDisplay *pDisplay = getGdkDisplay();
    GdkWindow *pWindow = gtk_widget_get_window(pWidget);

#if defined(GDK_WINDOWING_X11)
    if (GDK_IS_X11_DISPLAY(pDisplay))
    {
        return GDK_WINDOW_XID(pWindow);
    }
#endif
#if defined(GDK_WINDOWING_WAYLAND)
    if (GDK_IS_WAYLAND_DISPLAY(pDisplay))
    {
        return reinterpret_cast<sal_uIntPtr>(gdk_wayland_window_get_wl_surface(pWindow));
    }
#endif
    return 0;
}

sal_uIntPtr GtkSalFrame::GetNativeWindowHandle()
{
    return GetNativeWindowHandle(m_pWindow);
}

void GtkDragSource::startDrag(const datatransfer::dnd::DragGestureEvent& rEvent,
                              sal_Int8 sourceActions, sal_Int32 /*cursor*/, sal_Int32 /*image*/,
                              const css::uno::Reference<css::datatransfer::XTransferable>& rTrans,
                              const css::uno::Reference<css::datatransfer::dnd::XDragSourceListener>& rListener) throw(std::exception)
{
    m_xListener = rListener;
    m_xTrans = rTrans;

    if (m_pFrame)
    {
        css::uno::Sequence<css::datatransfer::DataFlavor> aFormats = rTrans->getTransferDataFlavors();
        std::vector<GtkTargetEntry> aGtkTargets(m_aConversionHelper.FormatsToGtk(aFormats));
        GtkTargetList *pTargetList = gtk_target_list_new(aGtkTargets.data(), aGtkTargets.size());

        gint nDragButton = 1; // default to left button
        css::awt::MouseEvent aEvent;
        if (rEvent.Event >>= aEvent)
        {
            if (aEvent.Buttons & css::awt::MouseButton::LEFT )
                nDragButton = 1;
            else if (aEvent.Buttons & css::awt::MouseButton::RIGHT)
                nDragButton = 3;
            else if (aEvent.Buttons & css::awt::MouseButton::MIDDLE)
                nDragButton = 2;
        }

        // For LibreOffice internal D&D we provide the Transferable without Gtk
        // intermediaries as a shortcut, see tdf#100097 for how dbaccess depends on this
        g_ActiveDragSource = this;

        m_pFrame->startDrag(nDragButton, rEvent.DragOriginX, rEvent.DragOriginY,
                            VclToGdk(sourceActions), pTargetList);
        gtk_target_list_unref(pTargetList);
        for (auto &a : aGtkTargets)
            g_free(a.target);
    }
    else
        dragFailed();
}

void GtkSalFrame::startDrag(gint nButton, gint nDragOriginX, gint nDragOriginY,
                            GdkDragAction sourceActions, GtkTargetList* pTargetList)
{
    SolarMutexGuard aGuard;

    assert(m_pDragSource);

    GdkEvent aFakeEvent;
    memset(&aFakeEvent, 0, sizeof(GdkEvent));
    aFakeEvent.type = GDK_BUTTON_PRESS;
    aFakeEvent.button.window = widget_get_window(getMouseEventWidget());
    aFakeEvent.button.time = GDK_CURRENT_TIME;
    GdkDeviceManager* pDeviceManager = gdk_display_get_device_manager(getGdkDisplay());
    aFakeEvent.button.device = gdk_device_manager_get_client_pointer(pDeviceManager);

#if GTK_CHECK_VERSION(3,10,0)
    GdkDragContext *pContext = gtk_drag_begin_with_coordinates(getMouseEventWidget(),
                                                               pTargetList,
                                                               sourceActions,
                                                               nButton,
                                                               &aFakeEvent,
                                                               nDragOriginX,
                                                               nDragOriginY);
#else
    GdkDragContext *pContext = gtk_drag_begin(getMouseEventWidget(),
                                              pTargetList,
                                              sourceActions,
                                              nButton,
                                              &aFakeEvent);
    (void)nDragOriginX;
    (void)nDragOriginY;
#endif

    if (!pContext)
        m_pDragSource->dragFailed();
}

void GtkDragSource::dragFailed()
{
    if (m_xListener.is())
    {
        datatransfer::dnd::DragSourceDropEvent aEv;
        aEv.DropAction = datatransfer::dnd::DNDConstants::ACTION_NONE;
        aEv.DropSuccess = false;
        auto xListener = m_xListener;
        m_xListener.clear();
        xListener->dragDropEnd(aEv);
    }
}

gboolean GtkSalFrame::signalDragFailed(GtkWidget* /*widget*/, GdkDragContext* /*context*/, GtkDragResult /*result*/, gpointer frame)
{
    GtkSalFrame* pThis = static_cast<GtkSalFrame*>(frame);
    if (!pThis->m_pDragSource)
        return false;
    pThis->m_pDragSource->dragFailed();
    return false;
}

void GtkDragSource::dragDelete()
{
    if (m_xListener.is())
    {
        datatransfer::dnd::DragSourceDropEvent aEv;
        aEv.DropAction = datatransfer::dnd::DNDConstants::ACTION_MOVE;
        aEv.DropSuccess = true;
        auto xListener = m_xListener;
        m_xListener.clear();
        xListener->dragDropEnd(aEv);
    }
}

void GtkSalFrame::signalDragDelete(GtkWidget* /*widget*/, GdkDragContext* /*context*/, gpointer frame)
{
    GtkSalFrame* pThis = static_cast<GtkSalFrame*>(frame);
    if (!pThis->m_pDragSource)
        return;
    pThis->m_pDragSource->dragDelete();
}

void GtkDragSource::dragEnd(GdkDragContext* context)
{
    if (m_xListener.is())
    {
        datatransfer::dnd::DragSourceDropEvent aEv;
        aEv.DropAction = GdkToVcl(gdk_drag_context_get_selected_action(context));
        aEv.DropSuccess = gdk_drag_drop_succeeded(context);
        auto xListener = m_xListener;
        m_xListener.clear();
        xListener->dragDropEnd(aEv);
    }
    g_ActiveDragSource = nullptr;
}

void GtkSalFrame::signalDragEnd(GtkWidget* /*widget*/, GdkDragContext* context, gpointer frame)
{
    GtkSalFrame* pThis = static_cast<GtkSalFrame*>(frame);
    if (!pThis->m_pDragSource)
        return;
    pThis->m_pDragSource->dragEnd(context);
}

void GtkDragSource::dragDataGet(GtkSelectionData *data, guint info)
{
    m_aConversionHelper.setSelectionData(m_xTrans, data, info);
}

void GtkSalFrame::signalDragDataGet(GtkWidget* /*widget*/, GdkDragContext* /*context*/, GtkSelectionData *data, guint info,
                                    guint /*time*/, gpointer frame)
{
    GtkSalFrame* pThis = static_cast<GtkSalFrame*>(frame);
    if (!pThis->m_pDragSource)
        return;
    pThis->m_pDragSource->dragDataGet(data, info);
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
