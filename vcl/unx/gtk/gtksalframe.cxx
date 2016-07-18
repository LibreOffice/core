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
#include <unx/genprn.h>
#include <unx/geninst.h>
#include <headless/svpgdi.hxx>
#include <osl/file.hxx>
#include <rtl/bootstrap.hxx>
#include <rtl/process.h>
#include <vcl/floatwin.hxx>
#include <vcl/svapp.hxx>
#include <vcl/window.hxx>
#include <vcl/settings.hxx>
#include <vcl/opengl/OpenGLHelper.hxx>

#include <config_gio.h>

#include <unx/x11/xlimits.hxx>
#if ENABLE_DBUS && ENABLE_GIO
#  include <unx/gtk/gtksalmenu.hxx>
#endif
#if defined ENABLE_GMENU_INTEGRATION // defined in gtksalmenu.hxx above
#  include <unx/gtk/hudawareness.h>
#endif

#include <gtk/gtk.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <gdk/gdkx.h>

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

#include <comphelper/processfactory.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <com/sun/star/accessibility/XAccessibleContext.hpp>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/XAccessibleStateSet.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/XAccessibleEditableText.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/ModuleManager.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/util/URLTransformer.hpp>

#include <config_folders.h>

#define IS_WIDGET_REALIZED GTK_WIDGET_REALIZED
#define IS_WIDGET_MAPPED   GTK_WIDGET_MAPPED

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
        if( GetGtkSalData()->GetGtkDisplay()->IsNumLockFromXS() )
        {
            nCode = KEY_F1 + (keyval-GDK_F1);
        }
        else
        {
            switch( keyval )
            {
                // - - - - - Sun keyboard, see vcl/unx/source/app/saldisp.cxx
                case GDK_L2:
                    if( GetGtkSalData()->GetGtkDisplay()->GetServerVendor() == vendor_sun )
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
        bool bHandled = CallCallback( SalEvent::KeyInput, &aEvent );
        // #i46889# copy AlternateKeyCode handling from generic plugin
        if( ! bHandled )
        {
            KeyAlternate aAlternate = GetAlternateKeyCode( aEvent.mnCode );
            if( aAlternate.nKeyCode )
            {
                aEvent.mnCode = aAlternate.nKeyCode;
                if( aAlternate.nCharCode )
                    aEvent.mnCharCode = aAlternate.nCharCode;
                CallCallback( SalEvent::KeyInput, &aEvent );
            }
        }
        if( bSendRelease && ! aDel.isDeleted() )
        {
            CallCallback( SalEvent::KeyUp, &aEvent );
        }
    }
    else
        CallCallback( SalEvent::KeyUp, &aEvent );
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
    Init( pSysData );
}

#ifdef ENABLE_GMENU_INTEGRATION

static void
gdk_x11_window_set_utf8_property  (GdkWindow *window,
                                   const gchar *name,
                                   const gchar *value)
{
  GdkDisplay* display = gdk_window_get_display (window);

  if (value != nullptr)
    {
      XChangeProperty (GDK_DISPLAY_XDISPLAY (display),
                       GDK_WINDOW_XID (window),
                       gdk_x11_get_xatom_by_name_for_display (display, name),
                       gdk_x11_get_xatom_by_name_for_display (display, "UTF8_STRING"), 8,
                       PropModeReplace, reinterpret_cast<guchar const *>(value), strlen (value));
    }
  else
    {
      XDeleteProperty (GDK_DISPLAY_XDISPLAY (display),
                       GDK_WINDOW_XID (window),
                       gdk_x11_get_xatom_by_name_for_display (display, name));
    }
}

// AppMenu watch functions.

static void ObjectDestroyedNotify( gpointer data )
{
    if ( data ) {
        g_object_unref( data );
    }
}

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
    g_object_get(action, "name", &strval, nullptr);
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
        GActionGroup* pActionGroup = reinterpret_cast<GActionGroup*>(g_lo_action_group_new());

        // Generate menu paths.
        ::Window windowId = GDK_WINDOW_XID( gdkWindow );
        gchar* aDBusWindowPath = g_strdup_printf( "/org/libreoffice/window/%lu", windowId );
        gchar* aDBusMenubarPath = g_strdup_printf( "/org/libreoffice/window/%lu/menus/menubar", windowId );

        // Set window properties.
        g_object_set_data_full( G_OBJECT( gdkWindow ), "g-lo-menubar", pMenuModel, ObjectDestroyedNotify );
        g_object_set_data_full( G_OBJECT( gdkWindow ), "g-lo-action-group", pActionGroup, ObjectDestroyedNotify );

        gdk_x11_window_set_utf8_property( gdkWindow, "_GTK_APPLICATION_ID", "org.libreoffice" );
        gdk_x11_window_set_utf8_property( gdkWindow, "_GTK_UNIQUE_BUS_NAME", g_dbus_connection_get_unique_name( pSessionBus ) );
        gdk_x11_window_set_utf8_property( gdkWindow, "_GTK_APPLICATION_OBJECT_PATH", "/org/libreoffice" );
        gdk_x11_window_set_utf8_property( gdkWindow, "_GTK_WINDOW_OBJECT_PATH", aDBusWindowPath );
        gdk_x11_window_set_utf8_property( gdkWindow, "_GTK_MENUBAR_OBJECT_PATH", aDBusMenubarPath );

        // Publish the menu model and the action group.
        SAL_INFO("vcl.unity", "exporting menu model at " << pMenuModel << " for window " << windowId);
        pSalFrame->m_nMenuExportId = g_dbus_connection_export_menu_model (pSessionBus, aDBusMenubarPath, pMenuModel, nullptr);
        SAL_INFO("vcl.unity", "exporting action group at " << pActionGroup << " for window " << windowId);
        pSalFrame->m_nActionGroupExportId = g_dbus_connection_export_action_group( pSessionBus, aDBusWindowPath, pActionGroup, nullptr);
        pSalFrame->m_nHudAwarenessId = hud_awareness_register( pSessionBus, aDBusMenubarPath, hud_activated, pSalFrame, nullptr, nullptr );

        // fdo#70885 we don't want app menu under Unity
        bool bDesktopIsUnity = (SalGetDesktopEnvironment() == "UNITY");

        if (!bDesktopIsUnity)
            gdk_x11_window_set_utf8_property( gdkWindow, "_GTK_APP_MENU_OBJECT_PATH", "/org/libreoffice/menus/appmenu" );

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
#if GLIB_CHECK_VERSION(2,38,0) // g_simple_action_group_add_entries is deprecated since 2.38
            g_action_map_add_action_entries (G_ACTION_MAP (group), app_entries, G_N_ELEMENTS (app_entries), nullptr);
#else
            g_simple_action_group_add_entries (group, app_entries, G_N_ELEMENTS (app_entries), NULL);
#endif
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
        pGtkSalMenu->EnableUnity(true);
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
        pGtkSalMenu->EnableUnity(false);
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
                                                       this,
                                                       nullptr );
    }
#else
    (void) this; // loplugin:staticmethods
#endif
}

void GtkSalFrame::InvalidateGraphics()
{
    if( m_pGraphics )
    {
        m_pGraphics->DeInit();
        m_pGraphics->SetWindow(nullptr);
        delete m_pGraphics;
        m_pGraphics = nullptr;
        m_bGraphics = false;
    }
}

GtkSalFrame::~GtkSalFrame()
{
    InvalidateGraphics();

    if( m_pParent )
        m_pParent->m_aChildren.remove( this );

    getDisplay()->deregisterFrame( this );

    if( m_pRegion )
    {
        gdk_region_destroy( m_pRegion );
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

void GtkSalFrame::widget_set_size_request(long nWidth, long nHeight)
{
    gint nOrigwidth, nOrigheight;
    gtk_window_get_size(GTK_WINDOW(m_pWindow), &nOrigwidth, &nOrigheight);
    if (nWidth > nOrigwidth || nHeight > nOrigheight)
    {
        m_bPaintsBlocked = true;
    }
    gtk_widget_set_size_request(m_pWindow, nWidth, nHeight );
}

void GtkSalFrame::window_resize(long nWidth, long nHeight)
{
    gint nOrigwidth, nOrigheight;
    gtk_window_get_size(GTK_WINDOW(m_pWindow), &nOrigwidth, &nOrigheight);
    if (nWidth > nOrigwidth || nHeight > nOrigheight)
    {
        m_bPaintsBlocked = true;
    }
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
            nullptr,     /* class init */
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
    return m_pWindow;
}

void GtkSalFrame::InitCommon()
{
    m_pEventBox = nullptr;
    // add the fixed container child,
    // fixed is needed since we have to position plugin windows
    m_pFixedContainer = GTK_FIXED(g_object_new( ooo_fixed_get_type(), nullptr ));
    gtk_container_add( GTK_CONTAINER(m_pWindow), GTK_WIDGET(m_pFixedContainer) );

    GtkWidget *pEventWidget = getMouseEventWidget();

    gtk_widget_set_app_paintable(GTK_WIDGET(m_pFixedContainer), true);
    gtk_widget_set_double_buffered(GTK_WIDGET(m_pFixedContainer), false);
    gtk_widget_set_redraw_on_allocate(GTK_WIDGET(m_pFixedContainer), false);


    // connect signals
    g_signal_connect( G_OBJECT(m_pWindow), "style-set", G_CALLBACK(signalStyleSet), this );
    m_aMouseSignalIds.push_back(g_signal_connect( G_OBJECT(pEventWidget), "button-press-event", G_CALLBACK(signalButton), this ));
    m_aMouseSignalIds.push_back(g_signal_connect( G_OBJECT(pEventWidget), "motion-notify-event", G_CALLBACK(signalMotion), this ));
    m_aMouseSignalIds.push_back(g_signal_connect( G_OBJECT(pEventWidget), "button-release-event", G_CALLBACK(signalButton), this ));
    g_signal_connect( G_OBJECT(m_pFixedContainer), "expose-event", G_CALLBACK(signalExpose), this );
    g_signal_connect( G_OBJECT(m_pWindow), "focus-in-event", G_CALLBACK(signalFocus), this );
    g_signal_connect( G_OBJECT(m_pWindow), "focus-out-event", G_CALLBACK(signalFocus), this );
    g_signal_connect( G_OBJECT(m_pWindow), "map-event", G_CALLBACK(signalMap), this );
    g_signal_connect( G_OBJECT(m_pWindow), "unmap-event", G_CALLBACK(signalUnmap), this );
    g_signal_connect( G_OBJECT(m_pWindow), "configure-event", G_CALLBACK(signalConfigure), this );
    g_signal_connect( G_OBJECT(m_pWindow), "key-press-event", G_CALLBACK(signalKey), this );
    g_signal_connect( G_OBJECT(m_pWindow), "key-release-event", G_CALLBACK(signalKey), this );
    g_signal_connect( G_OBJECT(m_pWindow), "delete-event", G_CALLBACK(signalDelete), this );
    g_signal_connect( G_OBJECT(m_pWindow), "window-state-event", G_CALLBACK(signalWindowState), this );
    g_signal_connect( G_OBJECT(m_pWindow), "scroll-event", G_CALLBACK(signalScroll), this );
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
    m_nExtStyle         = 0;
    m_pRegion           = nullptr;
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
    gtk_widget_show_all( GTK_WIDGET(m_pFixedContainer) );

    // realize the window, we need an XWindow id
    gtk_widget_realize( m_pWindow );

    //system data
    m_aSystemData.nSize         = sizeof( SystemEnvData );
    GtkSalDisplay* pDisp = GetGtkSalData()->GetGtkDisplay();
    m_aSystemData.pDisplay      = pDisp->GetDisplay();
    m_aSystemData.pVisual       = pDisp->GetVisual( m_nXScreen ).GetVisual();
    m_aSystemData.nDepth        = pDisp->GetVisual( m_nXScreen ).GetDepth();
    m_aSystemData.aColormap     = pDisp->GetColormap( m_nXScreen ).GetXColormap();
    m_aSystemData.aWindow       = widget_get_xid(m_pWindow);
    m_aSystemData.aShellWindow  = m_aSystemData.aWindow;
    m_aSystemData.pSalFrame     = this;
    m_aSystemData.pWidget       = m_pWindow;
    m_aSystemData.nScreen       = m_nXScreen.getXScreen();
    m_aSystemData.pAppContext   = nullptr;
    m_aSystemData.pShellWidget  = m_aSystemData.pWidget;
    m_aSystemData.pToolkit      = "gtk2";

    m_bGraphics = false;
    m_pGraphics = nullptr;

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
                                widget_get_xid(m_pWindow),
                                None );
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
    static setAcceptFn p_gtk_window_set_accept_focus = nullptr;
    static bool bGetAcceptFocusFn = true;

    typedef void(*setUserTimeFn)( GdkWindow*, guint32 );
    static setUserTimeFn p_gdk_x11_window_set_user_time = nullptr;
    static bool bGetSetUserTimeFn = true;
}

static void lcl_set_accept_focus( GtkWindow* pWindow, gboolean bAccept, bool bBeforeRealize )
{
    if( bGetAcceptFocusFn )
    {
        bGetAcceptFocusFn = false;
        p_gtk_window_set_accept_focus = reinterpret_cast<setAcceptFn>(osl_getAsciiFunctionSymbol( GetSalData()->m_pPlugin, "gtk_window_set_accept_focus" ));
    }
    if( p_gtk_window_set_accept_focus && bBeforeRealize )
        p_gtk_window_set_accept_focus( pWindow, bAccept );
    else if( ! bBeforeRealize )
    {
        Display* pDisplay = GetGtkSalData()->GetGtkDisplay()->GetDisplay();
        ::Window aWindow = widget_get_xid(GTK_WIDGET(pWindow));
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

        if (GetGtkSalData()->GetGtkDisplay()->getWMAdaptor()->getWindowManagerName() == "compiz")
            return;

        /*  remove WM_TAKE_FOCUS protocol; this would usually be the
         *  right thing, but gtk handles it internally whereas we
         *  want to handle it ourselves (as to sometimes not get
         *  the focus)
         */
        Atom* pProtocols = nullptr;
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

static void lcl_set_user_time( GtkWindow* i_pWindow, guint32 i_nTime )
{
    if( bGetSetUserTimeFn )
    {
        bGetSetUserTimeFn = false;
        p_gdk_x11_window_set_user_time = reinterpret_cast<setUserTimeFn>(osl_getAsciiFunctionSymbol( GetSalData()->m_pPlugin, "gdk_x11_window_set_user_time" ));
    }
    bool bSet = false;
    if( p_gdk_x11_window_set_user_time )
    {
        GdkWindow* pWin = widget_get_window(GTK_WIDGET(i_pWindow));
        if( pWin ) // only if the window is realized.
        {
            p_gdk_x11_window_set_user_time( pWin, i_nTime );
            bSet = true;
        }
    }
    if( !bSet )
    {
        Display* pDisplay = GetGtkSalData()->GetGtkDisplay()->GetDisplay();
        Atom nUserTime = XInternAtom( pDisplay, "_NET_WM_USER_TIME", True );
        if( nUserTime )
        {
            XChangeProperty( pDisplay, widget_get_xid(GTK_WIDGET(i_pWindow)),
                             nUserTime, XA_CARDINAL, 32,
                             PropModeReplace, reinterpret_cast<unsigned char*>(&i_nTime), 1 );
        }
    }
};

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
    if (!isChild())
    {
        if (m_pParent)
            m_sWMClass = m_pParent->m_sWMClass;
        updateWMClass();
    }

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
            lcl_set_accept_focus( GTK_WINDOW(m_pWindow), false, true );
        }
        if( (nStyle & SalFrameStyleFlags::PARTIAL_FULLSCREEN )
            && getDisplay()->getWMAdaptor()->isLegacyPartialFullscreen() )
        {
            eType = GDK_WINDOW_TYPE_HINT_TOOLBAR;
            gtk_window_set_keep_above( GTK_WINDOW(m_pWindow), true );
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

        guint32 nUserTime = 0;
        if( (nStyle & (SalFrameStyleFlags::OWNERDRAWDECORATION|SalFrameStyleFlags::TOOLWINDOW)) == SalFrameStyleFlags::NONE )
        {
            nUserTime = gdk_x11_get_server_time(GTK_WIDGET (m_pWindow)->window);
        }
        lcl_set_user_time(GTK_WINDOW(m_pWindow), nUserTime);
    }

    if( bDecoHandling )
    {
        gtk_window_set_resizable( GTK_WINDOW(m_pWindow), bool(nStyle & SalFrameStyleFlags::SIZEABLE) );
        if( ( (nStyle & (SalFrameStyleFlags::OWNERDRAWDECORATION)) ) )
            lcl_set_accept_focus( GTK_WINDOW(m_pWindow), false, false );
    }
}

GdkNativeWindow GtkSalFrame::findTopLevelSystemWindow( GdkNativeWindow aWindow )
{
    ::Window aRoot, aParent;
    ::Window* pChildren;
    unsigned int nChildren;
    bool bBreak = false;
    do
    {
        pChildren = nullptr;
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
    m_pParent = nullptr;
    m_aForeignParentWindow = (GdkNativeWindow)pSysData->aWindow;
    m_pForeignParent = nullptr;
    m_aForeignTopLevelWindow = findTopLevelSystemWindow( (GdkNativeWindow)pSysData->aWindow );
    m_pForeignTopLevel = gdk_window_foreign_new_for_display( getGdkDisplay(), m_aForeignTopLevelWindow );
    gdk_window_set_events( m_pForeignTopLevel, GDK_STRUCTURE_MASK );

    if( pSysData->nSize > sizeof(pSysData->nSize)+sizeof(pSysData->aWindow) && pSysData->bXEmbedSupport )
    {
        m_pWindow = gtk_plug_new( pSysData->aWindow );
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

    int x_ret, y_ret;
    unsigned int w, h, bw, d;
    ::Window aRoot;
    XGetGeometry( getDisplay()->GetDisplay(), pSysData->aWindow,
                  &aRoot, &x_ret, &y_ret, &w, &h, &bw, &d );
    maGeometry.nWidth   = w;
    maGeometry.nHeight  = h;
    window_resize(w, h);
    gtk_window_move( GTK_WINDOW(m_pWindow), 0, 0 );
    if( ! m_bWindowIsGtkPlug )
    {
        XReparentWindow( getDisplay()->GetDisplay(),
                         widget_get_xid(m_pWindow),
                         (::Window)pSysData->aWindow,
                         0, 0 );
    }
}

void GtkSalFrame::askForXEmbedFocus( sal_Int32 i_nTimeCode )
{
    XEvent aEvent;

    memset( &aEvent, 0, sizeof(aEvent) );
    aEvent.xclient.window = m_aForeignParentWindow;
    aEvent.xclient.type = ClientMessage;
    aEvent.xclient.message_type = getDisplay()->getWMAdaptor()->getAtom( vcl_sal::WMAdaptor::XEMBED );
    aEvent.xclient.format = 32;
    aEvent.xclient.data.l[0] = i_nTimeCode ? i_nTimeCode : CurrentTime;
    aEvent.xclient.data.l[1] = 3; // XEMBED_REQUEST_FOCUS
    aEvent.xclient.data.l[2] = 0;
    aEvent.xclient.data.l[3] = 0;
    aEvent.xclient.data.l[4] = 0;

    GetGenericData()->ErrorTrapPush();
    XSendEvent( getDisplay()->GetDisplay(),
                m_aForeignParentWindow,
                False, NoEventMask, &aEvent );
    GetGenericData()->ErrorTrapPop();
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
        m_pGraphics = new GtkSalGraphics( this, m_pWindow, m_nXScreen );
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
        appicon = g_strdup ("libreoffice-startcenter");

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
    long nX, nY;

    if( m_pParent )
    {
        nX = ((long)m_pParent->maGeometry.nWidth - (long)maGeometry.nWidth)/2;
        nY = ((long)m_pParent->maGeometry.nHeight - (long)maGeometry.nHeight)/2;
    }
    else
    {
        GdkScreen *pScreen = nullptr;
        gint px, py;
        GdkModifierType nMask;
        gdk_display_get_pointer( getGdkDisplay(), &pScreen, &px, &py, &nMask );
        if( !pScreen )
            pScreen = gtk_widget_get_screen( m_pWindow );

        gint nMonitor;
        nMonitor = gdk_screen_get_monitor_at_point( pScreen, px, py );

        GdkRectangle aMonitor;
        gdk_screen_get_monitor_geometry( pScreen, nMonitor, &aMonitor );

        nX = aMonitor.x + (aMonitor.width - (long)maGeometry.nWidth)/2;
        nY = aMonitor.y + (aMonitor.height - (long)maGeometry.nHeight)/2;
    }
    SetPosSize( nX, nY, 0, 0, SAL_FRAME_POSSIZE_X | SAL_FRAME_POSSIZE_Y );
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
    static bool bOnce = false;
    if (!bOnce)
    {
        bOnce = true;
        const OString& rID = SessionManagerClient::getSessionID();
        if (!rID.isEmpty())
            gdk_set_sm_client_id(rID.getStr());
    }
}

void GtkSalFrame::Show( bool bVisible, bool bNoActivate )
{
    if( m_pWindow )
    {
        if( m_pParent && (m_pParent->m_nStyle & SalFrameStyleFlags::PARTIAL_FULLSCREEN)
            && getDisplay()->getWMAdaptor()->isLegacyPartialFullscreen() )
            gtk_window_set_keep_above( GTK_WINDOW(m_pWindow), bVisible );
        if( bVisible )
        {
            initClientId();
            getDisplay()->startupNotificationCompleted();

            if( m_bDefaultPos )
                Center();
            if( m_bDefaultSize )
                SetDefaultSize();
            setMinMaxSize();

            // #i45160# switch to desktop where a dialog with parent will appear
            if( m_pParent && m_pParent->m_nWorkArea != m_nWorkArea && IS_WIDGET_MAPPED(m_pParent->m_pWindow) )
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
                 m_pParent->grabPointer( true, true );
            }

            guint32 nUserTime = 0;
            if( ! bNoActivate && !(m_nStyle & (SalFrameStyleFlags::OWNERDRAWDECORATION|SalFrameStyleFlags::TOOLWINDOW)) )
                nUserTime = gdk_x11_get_server_time(GTK_WIDGET (m_pWindow)->window);

            //For these floating windows we don't want the main window to lose focus, and metacity has...
            // metacity-2.24.0/src/core/window.c

            //  if ((focus_window != NULL) && XSERVER_TIME_IS_BEFORE (compare, focus_window->net_wm_user_time))
            //      "compare" window focus prevented by other activity

            //  where "compare" is this window

            //  which leads to...

            // /* This happens for error dialogs or alerts; these need to remain on
            // * top, but it would be confusing to have its ancestor remain
            // * focused.
            // */
            // if (meta_window_is_ancestor_of_transient (focus_window, window))
            //          "The focus window %s is an ancestor of the newly mapped "
            //         "window %s which isn't being focused.  Unfocusing the "
            //          "ancestor.\n",

            // i.e. having a time < that of the toplevel frame means that the toplevel frame gets unfocused.
            // awesome.
            if( nUserTime == 0 )
            {
                nUserTime = gdk_x11_get_server_time(GTK_WIDGET (m_pWindow)->window);
            }
            lcl_set_user_time(GTK_WINDOW(m_pWindow), nUserTime );

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
        CallCallback( SalEvent::Resize, nullptr );
        TriggerPaintEvent();
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
            if( ! m_bFullscreen )
            {
                aGeo.min_width = maGeometry.nWidth;
                aGeo.min_height = maGeometry.nHeight;
                aHints |= GDK_HINT_MIN_SIZE;

                aGeo.max_width = maGeometry.nWidth;
                aGeo.max_height = maGeometry.nHeight;
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
        // Show does a setMinMaxSize
        if( IS_WIDGET_MAPPED( m_pWindow ) )
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
            widget_set_size_request(nWidth, nHeight );
            // Show does a setMinMaxSize
            if( IS_WIDGET_MAPPED( m_pWindow ) )
                setMinMaxSize();
        }
    }
}

void GtkSalFrame::AllocateFrame()
{
}

void GtkSalFrame::SetPosSize( long nX, long nY, long nWidth, long nHeight, sal_uInt16 nFlags )
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
                nX = m_pParent->maGeometry.nWidth-maGeometry.nWidth-1-nX;
            nX += m_pParent->maGeometry.nX;
            nY += m_pParent->maGeometry.nY;
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

    if( bSized )
        AllocateFrame();

    if( bSized && ! bMoved )
        CallCallback( SalEvent::Resize, nullptr );
    else if( bMoved && ! bSized )
        CallCallback( SalEvent::Move, nullptr );
    else if( bMoved && bSized )
        CallCallback( SalEvent::MoveResize, nullptr );

    if (bSized)
        TriggerPaintEvent();
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
    rRect = GetGtkSalData()->GetGtkDisplay()->getWMAdaptor()->getWorkArea( 0 );
}

SalFrame* GtkSalFrame::GetParent() const
{
    return m_pParent;
}

void GtkSalFrame::SetWindowState( const SalFrameState* pState )
{
    if( ! m_pWindow || ! pState || isChild( true, false ) )
        return;

    const WindowStateMask nMaxGeometryMask =
        WindowStateMask::X | WindowStateMask::Y |
        WindowStateMask::Width | WindowStateMask::Height |
        WindowStateMask::MaximizedX | WindowStateMask::MaximizedY |
        WindowStateMask::MaximizedWidth | WindowStateMask::MaximizedHeight;

    if( (pState->mnMask & WindowStateMask::State) &&
        ! ( m_nState & GDK_WINDOW_STATE_MAXIMIZED ) &&
        (pState->mnState & WindowStateState::Maximized) &&
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
        CallCallback( SalEvent::Resize, nullptr );
    }
    else if( pState->mnMask & (WindowStateMask::X | WindowStateMask::Y |
                               WindowStateMask::Width | WindowStateMask::Height ) )
    {
        sal_uInt16 nPosSizeFlags = 0;
        long nX         = pState->mnX - (m_pParent ? m_pParent->maGeometry.nX : 0);
        long nY         = pState->mnY - (m_pParent ? m_pParent->maGeometry.nY : 0);
        if( pState->mnMask & WindowStateMask::X )
            nPosSizeFlags |= SAL_FRAME_POSSIZE_X;
        else
            nX = maGeometry.nX - (m_pParent ? m_pParent->maGeometry.nX : 0);
        if( pState->mnMask & WindowStateMask::Y )
            nPosSizeFlags |= SAL_FRAME_POSSIZE_Y;
        else
            nY = maGeometry.nY - (m_pParent ? m_pParent->maGeometry.nY : 0);
        if( pState->mnMask & WindowStateMask::Width )
            nPosSizeFlags |= SAL_FRAME_POSSIZE_WIDTH;
        if( pState->mnMask & WindowStateMask::Height )
            nPosSizeFlags |= SAL_FRAME_POSSIZE_HEIGHT;
        SetPosSize( nX, nY, pState->mnWidth, pState->mnHeight, nPosSizeFlags );
    }
    if( pState->mnMask & WindowStateMask::State && ! isChild() )
    {
        if( pState->mnState & WindowStateState::Maximized )
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
        if( (pState->mnState & WindowStateState::Minimized)
            && ! m_pParent )
            gtk_window_iconify( GTK_WINDOW(m_pWindow) );
        else
            gtk_window_deiconify( GTK_WINDOW(m_pWindow) );
    }
    TriggerPaintEvent();
}

bool GtkSalFrame::GetWindowState( SalFrameState* pState )
{
    pState->mnState = WindowStateState::Normal;
    pState->mnMask  = WindowStateMask::State;
    // rollup ? gtk 2.2 does not seem to support the shaded state
    if( (m_nState & GDK_WINDOW_STATE_ICONIFIED) )
        pState->mnState |= WindowStateState::Minimized;
    if( m_nState & GDK_WINDOW_STATE_MAXIMIZED )
    {
        pState->mnState |= WindowStateState::Maximized;
        pState->mnX                 = m_aRestorePosSize.Left();
        pState->mnY                 = m_aRestorePosSize.Top();
        pState->mnWidth             = m_aRestorePosSize.GetWidth();
        pState->mnHeight            = m_aRestorePosSize.GetHeight();
        pState->mnMaximizedX        = maGeometry.nX;
        pState->mnMaximizedY        = maGeometry.nY;
        pState->mnMaximizedWidth    = maGeometry.nWidth;
        pState->mnMaximizedHeight   = maGeometry.nHeight;
        pState->mnMask  |= WindowStateMask::MaximizedX          |
                           WindowStateMask::MaximizedY          |
                           WindowStateMask::MaximizedWidth      |
                           WindowStateMask::MaximizedHeight;
    }
    else
    {
        pState->mnX         = maGeometry.nX;
        pState->mnY         = maGeometry.nY;
        pState->mnWidth     = maGeometry.nWidth;
        pState->mnHeight    = maGeometry.nHeight;
    }
    pState->mnMask  |= WindowStateMask::X            |
                       WindowStateMask::Y            |
                       WindowStateMask::Width        |
                       WindowStateMask::Height;

    return true;
}

void GtkSalFrame::SetScreen( unsigned int nNewScreen, SetType eType, Rectangle *pSize )
{
    if( !m_pWindow )
        return;

    if (maGeometry.nDisplayScreenNumber == nNewScreen && eType == SetType::RetainSize)
        return;

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

        maGeometry.nX = aNewMonitor.x + maGeometry.nX - aOldMonitor.x;
        maGeometry.nY = aNewMonitor.y + maGeometry.nY - aOldMonitor.y;
    }

    bool bResize = false;
    bool bVisible = IS_WIDGET_MAPPED( m_pWindow );
    if( bVisible )
        Show( false );

    if( eType == SetType::Fullscreen )
    {
        maGeometry.nX = aNewMonitor.x;
        maGeometry.nY = aNewMonitor.y;
        maGeometry.nWidth = aNewMonitor.width;
        maGeometry.nHeight = aNewMonitor.height;
        m_nStyle |= SalFrameStyleFlags::PARTIAL_FULLSCREEN;
        bResize = true;

        // #i110881# for the benefit of compiz set a max size here
        // else setting to fullscreen fails for unknown reasons
        m_aMaxSize.Width() = aNewMonitor.width;
        m_aMaxSize.Height() = aNewMonitor.height;
    }

    if( pSize && eType == SetType::UnFullscreen )
    {
        maGeometry.nX = pSize->Left();
        maGeometry.nY = pSize->Top();
        maGeometry.nWidth = pSize->GetWidth();
        maGeometry.nHeight = pSize->GetHeight();
        m_nStyle &= ~SalFrameStyleFlags::PARTIAL_FULLSCREEN;
        bResize = true;
    }

    if (bResize)
    {
        // temporarily re-sizeable
        if( !(m_nStyle & SalFrameStyleFlags::SIZEABLE) )
            gtk_window_set_resizable( GTK_WINDOW(m_pWindow), TRUE );
        window_resize(maGeometry.nWidth, maGeometry.nHeight);
        //I wonder if we should instead leave maGeometry alone and rely on
        //configure-event to trigger signalConfigure and set it there
        AllocateFrame();
        TriggerPaintEvent();
    }

    gtk_window_move( GTK_WINDOW( m_pWindow ), maGeometry.nX, maGeometry.nY );

    // _NET_WM_STATE_FULLSCREEN (Metacity <-> KWin)
    if( ! getDisplay()->getWMAdaptor()->isLegacyPartialFullscreen() )
    {
        if( eType == SetType::Fullscreen )
            gtk_window_fullscreen( GTK_WINDOW( m_pWindow ) );
        else if( eType == SetType::UnFullscreen )
            gtk_window_unfullscreen( GTK_WINDOW( m_pWindow ) );
    }

    if( eType == SetType::UnFullscreen &&
        !(m_nStyle & SalFrameStyleFlags::SIZEABLE) )
        gtk_window_set_resizable( GTK_WINDOW( m_pWindow ), FALSE );

    // FIXME: we should really let gtk+ handle our widget hierarchy ...
    if( m_pParent && gtk_widget_get_screen( m_pParent->m_pWindow ) != pScreen )
        SetParent( nullptr );
    std::list< GtkSalFrame* > aChildren = m_aChildren;
    for( std::list< GtkSalFrame* >::iterator it = aChildren.begin(); it != aChildren.end(); ++it )
        (*it)->SetScreen( nNewScreen, SetType::RetainSize );

    m_bDefaultPos = m_bDefaultSize = false;
    updateScreenNumber();
    CallCallback( SalEvent::MoveResize, nullptr );

    if( bVisible )
        Show( true );
}

void GtkSalFrame::SetScreenNumber( unsigned int nNewScreen )
{
    SetScreen( nNewScreen, SetType::RetainSize );
}

void GtkSalFrame::updateWMClass()
{
    OString aResClass = OUStringToOString(m_sWMClass, RTL_TEXTENCODING_ASCII_US);
    const char *pResClass = !aResClass.isEmpty() ? aResClass.getStr() :
                                                    SalGenericSystem::getFrameClassName();
    Display *display;

    if (!getDisplay()->IsX11Display())
        return;

    display = getDisplay()->GetDisplay();

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
        m_aRestorePosSize = Rectangle( Point( maGeometry.nX, maGeometry.nY ),
                                       Size( maGeometry.nWidth, maGeometry.nHeight ) );
        SetScreen( nScreen, SetType::Fullscreen );
    }
    else
    {
        SetScreen( nScreen, SetType::UnFullscreen,
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

void GtkSalFrame::ToTop( SalFrameToTop nFlags )
{
    if( m_pWindow )
    {
        if( isChild( false ) )
            gtk_widget_grab_focus( m_pWindow );
        else if( IS_WIDGET_MAPPED( m_pWindow ) )
        {
            if( ! (nFlags & SalFrameToTop::GrabFocusOnly) )
                gtk_window_present( GTK_WINDOW(m_pWindow) );
            else
            {
                guint32 nUserTime = gdk_x11_get_server_time(GTK_WIDGET (m_pWindow)->window);
                gdk_window_focus( widget_get_window(m_pWindow), nUserTime );
            }
            /*  need to do an XSetInputFocus here because
             *  gdk_window_focus will ask a EWMH compliant WM to put the focus
             *  to our window - which it of course won't since our input hint
             *  is set to false.
             */
            if (m_nStyle & SalFrameStyleFlags::OWNERDRAWDECORATION)
            {
                // sad but true: this can cause an XError, we need to catch that
                // to do this we need to synchronize with the XServer
                GetGenericData()->ErrorTrapPush();
                XSetInputFocus( getDisplay()->GetDisplay(), widget_get_xid(m_pWindow), RevertToParent, CurrentTime );
                // fdo#46687 - an XSync should not be necessary - but for some reason it is.
                XSync( getDisplay()->GetDisplay(), False );
                GetGenericData()->ErrorTrapPop();
            }
        }
        else
        {
            if( nFlags & SalFrameToTop::RestoreWhenMin )
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

    if( bGrab )
    {
        bool bUseGdkGrab = true;
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
        if( bUseGdkGrab )
        {
            gdk_pointer_grab( widget_get_window( m_pWindow ), bOwnerEvents,
                              (GdkEventMask) nMask, nullptr, m_pCurrentCursor,
                              GDK_CURRENT_TIME );
        }
        else
        {
            // FIXME: for some unknown reason gdk_pointer_grab does not
            // really produce owner events for GtkPlug windows
            // the cause is yet unknown

            // this is of course a bad hack, especially as we cannot
            // set the right cursor this way
            XGrabPointer( getDisplay()->GetDisplay(),
                          widget_get_xid( m_pWindow ),
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

void GtkSalFrame::grabKeyboard( bool bGrab )
{
    static const char* pEnv = getenv("SAL_NO_MOUSEGRABS"); // let's not introduce a special var for this
    if (pEnv && *pEnv)
        return;

    if (!m_pWindow)
        return;

    if( bGrab )
    {
        gdk_keyboard_grab(widget_get_window(m_pWindow), true,
                          GDK_CURRENT_TIME);
    }
    else
    {
        gdk_keyboard_ungrab(GDK_CURRENT_TIME);
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

    XWarpPointer( GDK_DISPLAY_XDISPLAY (pDisplay), None,
                  GDK_WINDOW_XID (gdk_screen_get_root_window( pScreen ) ),
                  0, 0, 0, 0, nWindowLeft, nWindowTop);
    // #i38648# ask for the next motion hint
    gint x, y;
    GdkModifierType mask;
    gdk_window_get_pointer( widget_get_window(pFrame->m_pWindow) , &x, &y, &mask );
}

void GtkSalFrame::Flush()
{
    XFlush (GDK_DISPLAY_XDISPLAY (getGdkDisplay()));
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
    return getDisplay()->GetKeyName( nKeyCode );
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
    return GetGtkSalData()->GetGtkDisplay()->GetIndicatorState();
}

void GtkSalFrame::SimulateKeyPress( sal_uInt16 nKeyCode )
{
    GetGtkSalData()->GetGtkDisplay()->SimulateKeyPress(nKeyCode);
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

void GtkSalFrame::createNewWindow( ::Window aNewParent, bool bXEmbed, SalX11Screen nXScreen )
{
    bool bWasVisible = m_pWindow && IS_WIDGET_MAPPED(m_pWindow);
    if( bWasVisible )
        Show( false );

    if( (int)nXScreen.getXScreen() >= getDisplay()->GetXScreenCount() )
        nXScreen = m_nXScreen;

    SystemParentData aParentData;
    aParentData.nSize = sizeof(SystemParentData);
    aParentData.aWindow = aNewParent;
    aParentData.bXEmbedSupport = bXEmbed;
    if( aNewParent == None )
    {
        aParentData.aWindow = None;
        aParentData.bXEmbedSupport = false;
    }
    else
    {
        // is new parent a root window ?
        Display* pDisp = getDisplay()->GetDisplay();
        int nScreens = getDisplay()->GetXScreenCount();
        for( int i = 0; i < nScreens; i++ )
        {
            if( aNewParent == RootWindow( pDisp, i ) )
            {
                nXScreen = SalX11Screen( i );
                aParentData.aWindow = None;
                aParentData.bXEmbedSupport = false;
                break;
            }
        }
    }

    // free xrender resources
    if( m_pGraphics )
        m_pGraphics->SetDrawable( None, m_nXScreen );

    // first deinit frame
    if( m_pIMHandler )
    {
        delete m_pIMHandler;
        m_pIMHandler = nullptr;
    }
    if( m_pRegion )
    {
        gdk_region_destroy( m_pRegion );
    }

    GtkWidget *pEventWidget = getMouseEventWidget();
    for (auto handler_id : m_aMouseSignalIds)
        g_signal_handler_disconnect(G_OBJECT(pEventWidget), handler_id);
    if( m_pFixedContainer )
        gtk_widget_destroy( GTK_WIDGET(m_pFixedContainer) );
    if( m_pEventBox )
        gtk_widget_destroy( GTK_WIDGET(m_pEventBox) );
    if( m_pWindow )
        gtk_widget_destroy( m_pWindow );
    if( m_pForeignParent )
        g_object_unref( G_OBJECT( m_pForeignParent ) );
    if( m_pForeignTopLevel )
        g_object_unref( G_OBJECT( m_pForeignTopLevel ) );

    // init new window
    m_bDefaultPos = m_bDefaultSize = false;
    if( aParentData.aWindow != None )
    {
        m_nStyle |= SalFrameStyleFlags::PLUG;
        Init( &aParentData );
    }
    else
    {
        m_nStyle &= ~SalFrameStyleFlags::PLUG;
        Init( (m_pParent && m_pParent->m_nXScreen == m_nXScreen) ? m_pParent : nullptr, m_nStyle );
    }

    // update graphics
    if( m_pGraphics )
    {
        m_pGraphics->SetDrawable( widget_get_xid(m_pWindow), m_nXScreen );
        m_pGraphics->SetWindow( m_pWindow );
    }

    if( ! m_aTitle.isEmpty() )
        SetTitle( m_aTitle );

    if( bWasVisible )
        Show( true );

    std::list< GtkSalFrame* > aChildren = m_aChildren;
    m_aChildren.clear();
    for( std::list< GtkSalFrame* >::iterator it = aChildren.begin(); it != aChildren.end(); ++it )
        (*it)->createNewWindow( None, false, m_nXScreen );

    // FIXME: SalObjects
}

bool GtkSalFrame::SetPluginParent( SystemParentData* pSysParent )
{
    GetGenericData()->ErrorTrapPush(); // permanantly ignore unruly children's errors
    createNewWindow( pSysParent->aWindow, (pSysParent->nSize > sizeof(long)) && pSysParent->bXEmbedSupport, m_nXScreen );
    return true;
}

void GtkSalFrame::ResetClipRegion()
{
    if( m_pWindow )
        gdk_window_shape_combine_region( widget_get_window( m_pWindow ), nullptr, 0, 0 );
}

void GtkSalFrame::BeginSetClipRegion( sal_uLong )
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
        gdk_window_shape_combine_region( widget_get_window(m_pWindow), m_pRegion, 0, 0 );
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
            m_nWorkArea = pAdaptor->getWindowWorkArea( widget_get_xid(m_pWindow) );
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
                getDisplay()->SendInternalEvent( this, nullptr, SalEvent::Resize );
            }
        }
        else if( m_pForeignTopLevel && pEvent->xconfigure.window == m_aForeignTopLevelWindow )
        {
            bContinueDispatch = false;
            // update position
            int x = 0, y = 0;
            ::Window aChild;
            XTranslateCoordinates( getDisplay()->GetDisplay(),
                                   widget_get_xid(m_pWindow),
                                   getDisplay()->GetRootWindow( getDisplay()->GetDefaultXScreen() ),
                                   0, 0,
                                   &x, &y,
                                   &aChild );
            if( x != maGeometry.nX || y != maGeometry.nY )
            {
                maGeometry.nX = x;
                maGeometry.nY = y;
                getDisplay()->SendInternalEvent( this, nullptr, SalEvent::Move );
            }
        }
    }
    else if( pEvent->type == ClientMessage &&
             pEvent->xclient.message_type == getDisplay()->getWMAdaptor()->getAtom( vcl_sal::WMAdaptor::XEMBED ) &&
             pEvent->xclient.window == widget_get_xid(m_pWindow) &&
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
            aEvent.window = widget_get_window( m_pWindow );
            aEvent.send_event = gint8(TRUE);
            aEvent.in = gint16(pEvent->xclient.data.l[1] == 1);
            signalFocus( m_pWindow, &aEvent, this );
        }
    }

    return bContinueDispatch;
}

gboolean GtkSalFrame::signalButton( GtkWidget*, GdkEventButton* pEvent, gpointer frame )
{
    GtkSalFrame* pThis = static_cast<GtkSalFrame*>(frame);

    SalMouseEvent aEvent;
    SalEvent nEventType = SalEvent::NONE;
    switch( pEvent->type )
    {
        case GDK_BUTTON_PRESS:
            nEventType = SalEvent::MouseButtonDown;
            break;
        case GDK_BUTTON_RELEASE:
            nEventType = SalEvent::MouseButtonUp;
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
                if (!(pSVData->maWinData.mpFirstFloat->GetPopupModeFlags() & FloatWinPopupFlags::NoAppFocusClose))
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
                pThis->CallCallback( SalEvent::Move, nullptr );
            }
        }
    }

    return true;
}

gboolean GtkSalFrame::signalScroll( GtkWidget*, GdkEventScroll* pEvent, gpointer frame )
{
    GtkSalFrame* pThis = static_cast<GtkSalFrame*>(frame);

    static sal_uLong        nLines = 0;
    if( ! nLines )
    {
        char* pEnv = getenv( "SAL_WHEELLINES" );
        nLines = pEnv ? atoi( pEnv ) : 3;
        if( nLines > 10 )
            nLines = SAL_WHEELMOUSE_EVENT_PAGESCROLL;
    }

    bool bNeg = (pEvent->direction == GDK_SCROLL_DOWN || pEvent->direction == GDK_SCROLL_RIGHT );
    SalWheelMouseEvent aEvent;
    aEvent.mnTime           = pEvent->time;
    aEvent.mnX              = (sal_uLong)pEvent->x;
    aEvent.mnY              = (sal_uLong)pEvent->y;
    aEvent.mnDelta          = bNeg ? -120 : 120;
    aEvent.mnNotchDelta     = bNeg ? -1 : 1;
    aEvent.mnScrollLines    = nLines;
    aEvent.mnCode           = GetMouseModCode( pEvent->state );
    aEvent.mbHorz           = (pEvent->direction == GDK_SCROLL_LEFT || pEvent->direction == GDK_SCROLL_RIGHT);

    // --- RTL --- (mirror mouse pos)
    if( AllSettings::GetLayoutRTL() )
        aEvent.mnX = pThis->maGeometry.nWidth-1-aEvent.mnX;

    pThis->CallCallback( SalEvent::WheelMouse, &aEvent );

    return true;
}

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

    pThis->CallCallback( SalEvent::MouseMove, &aEvent );

    if( ! aDel.isDeleted() )
    {
        int frame_x = (int)(pEvent->x_root - pEvent->x);
        int frame_y = (int)(pEvent->y_root - pEvent->y);
        if( frame_x != pThis->maGeometry.nX || frame_y != pThis->maGeometry.nY )
        {
            pThis->maGeometry.nX = frame_x;
            pThis->maGeometry.nY = frame_y;
            pThis->CallCallback( SalEvent::Move, nullptr );
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

    pThis->CallCallback( (pEvent->type == GDK_ENTER_NOTIFY) ? SalEvent::MouseMove : SalEvent::MouseLeave, &aEvent );

    return true;
}

gboolean GtkSalFrame::signalExpose( GtkWidget*, GdkEventExpose* pEvent, gpointer frame )
{
    GtkSalFrame* pThis = static_cast<GtkSalFrame*>(frame);
    pThis->m_bPaintsBlocked = false;

    struct SalPaintEvent aEvent( pEvent->area.x, pEvent->area.y, pEvent->area.width, pEvent->area.height, OpenGLHelper::isVCLOpenGLEnabled() );

    pThis->CallCallback( SalEvent::Paint, &aEvent );

    return false;
}

gboolean GtkSalFrame::signalConfigure( GtkWidget*, GdkEventConfigure* pEvent, gpointer frame )
{
    GtkSalFrame* pThis = static_cast<GtkSalFrame*>(frame);
    pThis->m_bPaintsBlocked = false;

    bool bMoved = false, bSized = false;
    int x = pEvent->x, y = pEvent->y;

    /*  HACK: during sizing/moving a toolbar pThis->maGeometry is actually
     *  already exact; even worse: due to the asynchronicity of configure
     *  events the borderwindow which would evaluate this event
     *  would size/move based on wrong data if we would actually evaluate
     *  this event. So let's swallow it.
     */
    if( (pThis->m_nStyle & SalFrameStyleFlags::OWNERDRAWDECORATION) &&
        GtkSalFrame::getDisplay()->GetCaptureFrame() == pThis )
        return false;

    /* #i31785# claims we cannot trust the x,y members of the event;
     * they are e.g. not set correctly on maximize/demaximize;
     * yet the gdkdisplay-x11.c code handling configure_events has
     * done this XTranslateCoordinates work since the day ~zero.
     */
    if( x != pThis->maGeometry.nX || y != pThis->maGeometry.nY )
    {
        bMoved = true;
        pThis->maGeometry.nX        = x;
        pThis->maGeometry.nY        = y;
    }
    /* #i86302#
     * for non sizeable windows we set the min and max hint for the window manager to
     * achieve correct sizing. However this is asynchronous and e.g. on Compiz
     * it sometimes happens that the window gets resized to another size (some default)
     * if we update the size here, subsequent setMinMaxSize will use this wrong size
     * - which is not good since the window manager will now size the window back to this
     * wrong size at some point.
     */
    if( pThis->m_bFullscreen || (pThis->m_nStyle & (SalFrameStyleFlags::SIZEABLE | SalFrameStyleFlags::PLUG)) == SalFrameStyleFlags::SIZEABLE )
    {
        if( pEvent->width != (int)pThis->maGeometry.nWidth || pEvent->height != (int)pThis->maGeometry.nHeight )
        {
            bSized = true;
            pThis->maGeometry.nWidth    = pEvent->width;
            pThis->maGeometry.nHeight   = pEvent->height;
        }
    }

    // update decoration hints
    if( ! (pThis->m_nStyle & SalFrameStyleFlags::PLUG) )
    {
        GdkRectangle aRect;
        gdk_window_get_frame_extents( widget_get_window(GTK_WIDGET(pThis->m_pWindow)), &aRect );
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

    pThis->updateScreenNumber();
    if( bSized )
        pThis->AllocateFrame();

    if( bMoved && bSized )
        pThis->CallCallback( SalEvent::MoveResize, nullptr );
    else if( bMoved )
        pThis->CallCallback( SalEvent::Move, nullptr );
    else if( bSized )
        pThis->CallCallback( SalEvent::Resize, nullptr );

    if (bSized)
        pThis->TriggerPaintEvent();
    return false;
}

void GtkSalFrame::TriggerPaintEvent()
{
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
        pThis->CallCallback( pEvent->in ? SalEvent::GetFocus : SalEvent::LoseFocus, nullptr );

    return false;
}

static OString getDisplayString()
{
    int nParams = rtl_getAppCommandArgCount();
    OUString aParam;
    for( int i = 0; i < nParams; i++ )
    {
        rtl_getAppCommandArg( i, &aParam.pData );
        if( i < nParams-1 && (aParam == "-display" || aParam == "--display" ) )
        {
            rtl_getAppCommandArg( i+1, &aParam.pData );
            return OUStringToOString( aParam, osl_getThreadTextEncoding() );
        }
    }
    return OString();
}

gboolean GtkSalFrame::signalMap( GtkWidget *pWidget, GdkEvent*, gpointer frame )
{
    GtkSalFrame* pThis = static_cast<GtkSalFrame*>(frame);

    //Spawn off a helper program that will attempt to set this fullscreen
    //window to span all displays.
    if (pThis->m_bFullscreen && pThis->m_bSpanMonitorsWhenFullscreen)
    {
        GdkWindow* gdkwin = widget_get_window(pThis->m_pWindow);
        if (gdkwin)
        {
            OUString sProgramURL( "$BRAND_BASE_DIR/" LIBO_LIBEXEC_FOLDER "/xid-fullscreen-on-all-monitors");
            rtl::Bootstrap::expandMacros(sProgramURL);
            OUString sProgram;
            if (osl::FileBase::getSystemPathFromFileURL(sProgramURL, sProgram) == osl::File::E_None)
            {
                OString sFinalProgram(OUStringToOString(sProgram, osl_getThreadTextEncoding())
                    + " " + OString::number((int)GDK_WINDOW_XID(gdkwin)));
                OString sDisplay(getDisplayString());
                if (!sDisplay.isEmpty())
                {
                    sFinalProgram += "--display " + sDisplay;
                }
                int returnValue = system(sFinalProgram.getStr());
                (void)returnValue;
            }
        }
    }

    bool bSetFocus = pThis->m_bSetFocusOnMap;
    pThis->m_bSetFocusOnMap = false;

    if( bSetFocus )
    {
        GetGenericData()->ErrorTrapPush();
        XSetInputFocus( GtkSalFrame::getDisplay()->GetDisplay(),
                        widget_get_xid(pWidget),
                        RevertToParent, CurrentTime );
        XSync( GtkSalFrame::getDisplay()->GetDisplay(), False );
        GetGenericData()->ErrorTrapPop();
    }

    pThis->CallCallback( SalEvent::Resize, nullptr );
    pThis->TriggerPaintEvent();

    return false;
}

gboolean GtkSalFrame::signalUnmap( GtkWidget*, GdkEvent*, gpointer frame )
{
    GtkSalFrame* pThis = static_cast<GtkSalFrame*>(frame);

    pThis->CallCallback( SalEvent::Resize, nullptr );

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

        pThis->CallCallback( SalEvent::KeyModChange, &aModEvt );

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

    pThis->CallCallback( SalEvent::Close, nullptr );

    return true;
}

void GtkSalFrame::signalStyleSet( GtkWidget*, GtkStyle* pPrevious, gpointer frame )
{
    GtkSalFrame* pThis = static_cast<GtkSalFrame*>(frame);

    // every frame gets an initial style set on creation
    // do not post these as the whole application tends to
    // redraw itself to adjust to the new style
    // where there IS no new style resulting in tremendous unnecessary flickering
    if( pPrevious != nullptr )
    {
        // signalStyleSet does NOT usually have the gdk lock
        // so post user event to safely dispatch the SalEvent::SettingsChanged
        // note: settings changed for multiple frames is avoided in winproc.cxx ImplHandleSettings
        GtkSalFrame::getDisplay()->SendInternalEvent( pThis, nullptr, SalEvent::SettingsChanged );
        GtkSalFrame::getDisplay()->SendInternalEvent( pThis, nullptr, SalEvent::FontChanged );
    }

    /* #i64117# gtk sets a nice background pixmap
    *  but we actually don't really want that, so save
    *  some time on the Xserver as well as prevent
    *  some paint issues
    */
    GdkWindow* pWin = widget_get_window(GTK_WIDGET(pThis->getWindow()));
    if( pWin )
    {
        ::Window aWin = GDK_WINDOW_XWINDOW(pWin);
        if( aWin != None )
            XSetWindowBackgroundPixmap( GtkSalFrame::getDisplay()->GetDisplay(),
                                        aWin,
                                        None );
    }
    if( ! pThis->m_pParent )
    {
        // signalize theme changed for NWF caches
        // FIXME: should be called only once for a style change
        GtkSalGraphics::bThemeChanged = true;
    }
}

gboolean GtkSalFrame::signalWindowState( GtkWidget*, GdkEvent* pEvent, gpointer frame )
{
    GtkSalFrame* pThis = static_cast<GtkSalFrame*>(frame);
    if( (pThis->m_nState & GDK_WINDOW_STATE_ICONIFIED) != (pEvent->window_state.new_window_state & GDK_WINDOW_STATE_ICONIFIED ) )
    {
        GtkSalFrame::getDisplay()->SendInternalEvent( pThis, nullptr, SalEvent::Resize );
        pThis->TriggerPaintEvent();
    }

    if(   (pEvent->window_state.new_window_state & GDK_WINDOW_STATE_MAXIMIZED) &&
        ! (pThis->m_nState & GDK_WINDOW_STATE_MAXIMIZED) )
    {
        pThis->m_aRestorePosSize =
            Rectangle( Point( pThis->maGeometry.nX, pThis->maGeometry.nY ),
                       Size( pThis->maGeometry.nWidth, pThis->maGeometry.nHeight ) );
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
    GtkSalFrame::getDisplay()->CancelInternalEvent( m_pFrame, &m_aInputEvent, SalEvent::ExtTextInput );
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
    m_pFrame->CallCallback( SalEvent::EndExtTextInput, nullptr );
}

void GtkSalFrame::IMHandler::updateIMSpotLocation()
{
    SalExtTextInputPosEvent aPosEvent;
    m_pFrame->CallCallback( SalEvent::ExtTextInputPos, static_cast<void*>(&aPosEvent) );
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
    aEmptyEv.mpTextAttr         = nullptr;
    aEmptyEv.maText.clear();
    aEmptyEv.mnCursorPos        = 0;
    aEmptyEv.mnCursorFlags      = 0;
    m_pFrame->CallCallback( SalEvent::ExtTextInput, static_cast<void*>(&aEmptyEv) );
    if( ! aDel.isDeleted() )
        m_pFrame->CallCallback( SalEvent::EndExtTextInput, nullptr );
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
                GtkSalFrame::getDisplay()->SendInternalEvent( m_pFrame, &m_aInputEvent, SalEvent::ExtTextInput );
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
            GtkSalFrame::getDisplay()->SendInternalEvent( m_pFrame, &m_aInputEvent, SalEvent::ExtTextInput );
        }
    }
    else
    {
        GetGenericData()->ErrorTrapPush();
        gtk_im_context_focus_out( m_pIMContext );
        GetGenericData()->ErrorTrapPop();
        // cancel an eventual event posted to begin preedit again
        GtkSalFrame::getDisplay()->CancelInternalEvent( m_pFrame, &m_aInputEvent, SalEvent::ExtTextInput );
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
            SAL_WARN_IF( m_nPrevKeyPresses <= 0, "vcl", "key press has vanished !" );
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

        pThis->m_aInputEvent.mpTextAttr         = nullptr;
        pThis->m_aInputEvent.maText             = OUString( pText, strlen(pText), RTL_TEXTENCODING_UTF8 );
        pThis->m_aInputEvent.mnCursorPos        = pThis->m_aInputEvent.maText.getLength();
        pThis->m_aInputEvent.mnCursorFlags      = 0;

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
         *  SalEvent::EndExtTextInput, either because of a regular commit
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
            pThis->m_pFrame->CallCallback( SalEvent::ExtTextInput, static_cast<void*>(&pThis->m_aInputEvent));
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
    pThis->m_aInputEvent.maText             = pText ? OUString( pText, strlen(pText), RTL_TEXTENCODING_UTF8 ) : OUString();
    pThis->m_aInputEvent.mnCursorPos        = nCursorPos;
    pThis->m_aInputEvent.mnCursorFlags      = 0;

    pThis->m_aInputFlags = std::vector<ExtTextInputAttr>( std::max( 1, (int)pThis->m_aInputEvent.maText.getLength() ), ExtTextInputAttr::NONE );

    PangoAttrIterator *iter = pango_attr_list_get_iterator(pAttrs);
    do
    {
        GSList *attr_list = nullptr;
        GSList *tmp_list = nullptr;
        gint start, end;
        ExtTextInputAttr sal_attr = ExtTextInputAttr::NONE;

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
                    sal_attr |= ExtTextInputAttr::Highlight;
                    pThis->m_aInputEvent.mnCursorFlags |= EXTTEXTINPUT_CURSOR_INVISIBLE;
                    break;
                case PANGO_ATTR_UNDERLINE:
                    sal_attr |= ExtTextInputAttr::Underline;
                    break;
                case PANGO_ATTR_STRIKETHROUGH:
                    sal_attr |= ExtTextInputAttr::RedText;
                    break;
                default:
                    break;
            }
            pango_attribute_destroy (pango_attr);
            tmp_list = tmp_list->next;
        }
        if (sal_attr == ExtTextInputAttr::NONE)
            sal_attr |= ExtTextInputAttr::Underline;
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

    pThis->m_pFrame->CallCallback( SalEvent::ExtTextInput, static_cast<void*>(&pThis->m_aInputEvent));
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
    FindFocus(const uno::Reference< accessibility::XAccessibleContext >& xContext)
{
    if (!xContext.is())
        return uno::Reference< accessibility::XAccessibleEditableText >();

    uno::Reference<accessibility::XAccessibleStateSet> xState = xContext->getAccessibleStateSet();
    if (xState.is())
    {
        if (xState->contains(accessibility::AccessibleStateType::FOCUSED))
        {
            uno::Reference< accessibility::XAccessibleEditableText > xText =
                uno::Reference<accessibility::XAccessibleEditableText>(xContext, uno::UNO_QUERY);
            if (xText.is())
                return xText;
        }
        if (xState->contains(accessibility::AccessibleStateType::MANAGES_DESCENDANTS))
            return uno::Reference< accessibility::XAccessibleEditableText >();
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

sal_uIntPtr GtkSalFrame::GetNativeWindowHandle()
{
    return widget_get_xid(m_pWindow);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
