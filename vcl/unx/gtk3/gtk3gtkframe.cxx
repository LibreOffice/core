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
#include <unx/gtk/gtksalmenu.hxx>
#include <unx/gtk/hudawareness.h>
#include <vcl/help.hxx>
#include <vcl/keycodes.hxx>
#include <vcl/layout.hxx>
#include <unx/wmadaptor.hxx>
#include <unx/sm.hxx>
#include <unx/salbmp.h>
#include <unx/genprn.h>
#include <unx/geninst.h>
#include <headless/svpgdi.hxx>
#include <o3tl/runtimetooustring.hxx>
#include <osl/file.hxx>
#include <rtl/bootstrap.hxx>
#include <rtl/process.h>
#include <vcl/floatwin.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>
#include <vcl/window.hxx>
#include <vcl/settings.hxx>
#include <cppuhelper/exc_hlp.hxx>

#include <config_gio.h>

#include <gtk/gtk.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#if defined(GDK_WINDOWING_X11)
#   include <gdk/gdkx.h>
#endif
#if defined(GDK_WINDOWING_WAYLAND)
#   include <gdk/gdkwayland.h>
#endif

#include <dlfcn.h>
#include <vcl/salbtype.hxx>
#include <window.h>
#include <strings.hrc>
#include <bitmaps.hlst>
#include <sal/macros.h>

#include <basegfx/range/b2ibox.hxx>
#include <basegfx/vector/b2ivector.hxx>

#include <algorithm>
#include <glib/gprintf.h>

#if OSL_DEBUG_LEVEL > 1
#  include <cstdio>
#endif

#include <i18nlangtag/mslangid.hxx>

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

#include <config_folders.h>

#define IS_WIDGET_REALIZED gtk_widget_get_realized
#define IS_WIDGET_MAPPED   gtk_widget_get_mapped

#ifndef GDK_IS_X11_DISPLAY
#define GDK_IS_X11_DISPLAY(foo) (true)
#endif


using namespace com::sun::star;

int GtkSalFrame::m_nFloats = 0;

static GDBusConnection* pSessionBus = nullptr;

sal_uInt16 GtkSalFrame::GetKeyModCode( guint state )
{
    sal_uInt16 nCode = 0;
    if( state & GDK_SHIFT_MASK )
        nCode |= KEY_SHIFT;
    if( state & GDK_CONTROL_MASK )
        nCode |= KEY_MOD1;
    if( state & GDK_MOD1_MASK )
        nCode |= KEY_MOD2;
    if( state & GDK_SUPER_MASK )
        nCode |= KEY_MOD3;
    return nCode;
}

sal_uInt16 GtkSalFrame::GetMouseModCode( guint state )
{
    sal_uInt16 nCode = GetKeyModCode( state );
    if( state & GDK_BUTTON1_MASK )
        nCode |= MOUSE_LEFT;
    if( state & GDK_BUTTON2_MASK )
        nCode |= MOUSE_MIDDLE;
    if( state & GDK_BUTTON3_MASK )
        nCode |= MOUSE_RIGHT;

    return nCode;
}

sal_uInt16 GtkSalFrame::GetKeyCode(guint keyval)
{
    sal_uInt16 nCode = 0;
    if( keyval >= GDK_KEY_0 && keyval <= GDK_KEY_9 )
        nCode = KEY_0 + (keyval-GDK_KEY_0);
    else if( keyval >= GDK_KEY_KP_0 && keyval <= GDK_KEY_KP_9 )
        nCode = KEY_0 + (keyval-GDK_KEY_KP_0);
    else if( keyval >= GDK_KEY_A && keyval <= GDK_KEY_Z )
        nCode = KEY_A + (keyval-GDK_KEY_A );
    else if( keyval >= GDK_KEY_a && keyval <= GDK_KEY_z )
        nCode = KEY_A + (keyval-GDK_KEY_a );
    else if( keyval >= GDK_KEY_F1 && keyval <= GDK_KEY_F26 )
    // KEY_F26 is the last function key known to keycodes.hxx
    {
        switch( keyval )
        {
            // - - - - - Sun keyboard, see vcl/unx/source/app/saldisp.cxx
            // althopugh GDK_KEY_F1 ... GDK_KEY_L10 are known to
            // gdk/gdkkeysyms.h, they are unlikely to be generated
            // except possibly by Solaris systems
            // this whole section needs review
            case GDK_KEY_L2:
                    nCode = KEY_F12;
                break;
            case GDK_KEY_L3:            nCode = KEY_PROPERTIES; break;
            case GDK_KEY_L4:            nCode = KEY_UNDO;       break;
            case GDK_KEY_L6:            nCode = KEY_COPY;       break; // KEY_F16
            case GDK_KEY_L8:            nCode = KEY_PASTE;      break; // KEY_F18
            case GDK_KEY_L10:           nCode = KEY_CUT;        break; // KEY_F20
            default:
                nCode = KEY_F1 + (keyval-GDK_KEY_F1);           break;
        }
    }
    else
    {
        switch( keyval )
        {
            case GDK_KEY_KP_Down:
            case GDK_KEY_Down:          nCode = KEY_DOWN;       break;
            case GDK_KEY_KP_Up:
            case GDK_KEY_Up:            nCode = KEY_UP;         break;
            case GDK_KEY_KP_Left:
            case GDK_KEY_Left:          nCode = KEY_LEFT;       break;
            case GDK_KEY_KP_Right:
            case GDK_KEY_Right:         nCode = KEY_RIGHT;      break;
            case GDK_KEY_KP_Begin:
            case GDK_KEY_KP_Home:
            case GDK_KEY_Begin:
            case GDK_KEY_Home:          nCode = KEY_HOME;       break;
            case GDK_KEY_KP_End:
            case GDK_KEY_End:           nCode = KEY_END;        break;
            case GDK_KEY_KP_Page_Up:
            case GDK_KEY_Page_Up:       nCode = KEY_PAGEUP;     break;
            case GDK_KEY_KP_Page_Down:
            case GDK_KEY_Page_Down:     nCode = KEY_PAGEDOWN;   break;
            case GDK_KEY_KP_Enter:
            case GDK_KEY_Return:        nCode = KEY_RETURN;     break;
            case GDK_KEY_Escape:        nCode = KEY_ESCAPE;     break;
            case GDK_KEY_ISO_Left_Tab:
            case GDK_KEY_KP_Tab:
            case GDK_KEY_Tab:           nCode = KEY_TAB;        break;
            case GDK_KEY_BackSpace:     nCode = KEY_BACKSPACE;  break;
            case GDK_KEY_KP_Space:
            case GDK_KEY_space:         nCode = KEY_SPACE;      break;
            case GDK_KEY_KP_Insert:
            case GDK_KEY_Insert:        nCode = KEY_INSERT;     break;
            case GDK_KEY_KP_Delete:
            case GDK_KEY_Delete:        nCode = KEY_DELETE;     break;
            case GDK_KEY_plus:
            case GDK_KEY_KP_Add:        nCode = KEY_ADD;        break;
            case GDK_KEY_minus:
            case GDK_KEY_KP_Subtract:   nCode = KEY_SUBTRACT;   break;
            case GDK_KEY_asterisk:
            case GDK_KEY_KP_Multiply:   nCode = KEY_MULTIPLY;   break;
            case GDK_KEY_slash:
            case GDK_KEY_KP_Divide:     nCode = KEY_DIVIDE;     break;
            case GDK_KEY_period:        nCode = KEY_POINT;      break;
            case GDK_KEY_decimalpoint:  nCode = KEY_POINT;      break;
            case GDK_KEY_comma:         nCode = KEY_COMMA;      break;
            case GDK_KEY_less:          nCode = KEY_LESS;       break;
            case GDK_KEY_greater:       nCode = KEY_GREATER;    break;
            case GDK_KEY_KP_Equal:
            case GDK_KEY_equal:         nCode = KEY_EQUAL;      break;
            case GDK_KEY_Find:          nCode = KEY_FIND;       break;
            case GDK_KEY_Menu:          nCode = KEY_CONTEXTMENU;break;
            case GDK_KEY_Help:          nCode = KEY_HELP;       break;
            case GDK_KEY_Undo:          nCode = KEY_UNDO;       break;
            case GDK_KEY_Redo:          nCode = KEY_REPEAT;     break;
            // on a sun keyboard this actually is usually SunXK_Stop = 0x0000FF69 (XK_Cancel),
            // but VCL doesn't have a key definition for that
            case GDK_KEY_Cancel:        nCode = KEY_F11;        break;
            case GDK_KEY_KP_Decimal:
            case GDK_KEY_KP_Separator:  nCode = KEY_DECIMAL;    break;
            case GDK_KEY_asciitilde:    nCode = KEY_TILDE;      break;
            case GDK_KEY_leftsinglequotemark:
            case GDK_KEY_quoteleft:    nCode = KEY_QUOTELEFT;    break;
            case GDK_KEY_bracketleft:  nCode = KEY_BRACKETLEFT;  break;
            case GDK_KEY_bracketright: nCode = KEY_BRACKETRIGHT; break;
            case GDK_KEY_semicolon:    nCode = KEY_SEMICOLON;    break;
            case GDK_KEY_quoteright:   nCode = KEY_QUOTERIGHT;   break;
            // some special cases, also see saldisp.cxx
            // - - - - - - - - - - - - -  Apollo - - - - - - - - - - - - - 0x1000
            // These can be found in ap_keysym.h
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
            // These can be found in DECkeysym.h
            case 0x1000FF00:
                nCode = KEY_DELETE;
                break;
            // - - - - - - - - - - - - - -  H P  - - - - - - - - - - - - - 0x1000
            // These can be found in HPkeysym.h
            case 0x1000FF73: // hpXK_DeleteChar
                nCode = KEY_DELETE;
                break;
            case 0x1000FF74: // hpXK_BackTab
            case 0x1000FF75: // hpXK_KP_BackTab
                nCode = KEY_TAB;
                break;
            // - - - - - - - - - - - - - - I B M - - - - - - - - - - - - -
            // - - - - - - - - - - - - - - O S F - - - - - - - - - - - - - 0x1004
            // These also can be found in HPkeysym.h
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
            // These can be found in Sunkeysym.h
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
            // - - - - - - - - - - - - - X F 8 6 - - - - - - - - - - - - - 0x1008
            // These can be found in XF86keysym.h
            // but more importantly they are also available GTK/Gdk version 3
            // and hence are already provided in gdk/gdkkeysyms.h, and hence
            // in gdk/gdk.h
            case GDK_KEY_Copy:          nCode = KEY_COPY;  break; // 0x1008ff57
            case GDK_KEY_Cut:           nCode = KEY_CUT;   break; // 0x1008ff58
            case GDK_KEY_Open:          nCode = KEY_OPEN;  break; // 0x1008ff6b
            case GDK_KEY_Paste:         nCode = KEY_PASTE; break; // 0x1008ff6d
        }
    }

    return nCode;
}

guint GtkSalFrame::GetKeyValFor(GdkKeymap* pKeyMap, guint16 hardware_keycode, guint8 group)
{
    guint updated_keyval = 0;
    gdk_keymap_translate_keyboard_state(pKeyMap, hardware_keycode,
        GdkModifierType(0), group, &updated_keyval, nullptr, nullptr, nullptr);
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

#if OSL_DEBUG_LEVEL > 0
static bool dumpframes = false;
#endif

bool GtkSalFrame::doKeyCallback( guint state,
                                 guint keyval,
                                 guint16 hardware_keycode,
                                 guint8 group,
                                 sal_Unicode aOrigCode,
                                 bool bDown,
                                 bool bSendRelease
                                 )
{
    SalKeyEvent aEvent;

    aEvent.mnCharCode       = aOrigCode;
    aEvent.mnRepeat         = 0;

    vcl::DeletionListener aDel( this );

#if OSL_DEBUG_LEVEL > 0
    const char* pKeyDebug = getenv("VCL_GTK3_PAINTDEBUG");

    if (pKeyDebug && *pKeyDebug == '1')
    {
        if (bDown)
        {
            // shift-zero forces a re-draw and event is swallowed
            if (keyval == GDK_KEY_0)
            {
                fprintf( stderr, "force widget_queue_draw\n");
                gtk_widget_queue_draw(GTK_WIDGET(m_pFixedContainer));
                return false;
            }
            else if (keyval == GDK_KEY_1)
            {
                fprintf( stderr, "force repaint all\n");
                TriggerPaintEvent();
                return false;
            }
            else if (keyval == GDK_KEY_2)
            {
                dumpframes = !dumpframes;
                fprintf(stderr, "toggle dump frames to %d\n", dumpframes);
                return false;
            }
        }
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
        if (gdk_keymap_get_entries_for_keyval(keymap, GDK_KEY_A, &keys, &n_keys))
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

    bool bStopProcessingKey;
    if (bDown)
    {
        bStopProcessingKey = CallCallbackExc(SalEvent::KeyInput, &aEvent);
        // #i46889# copy AlternateKeyCode handling from generic plugin
        if (!bStopProcessingKey)
        {
            KeyAlternate aAlternate = GetAlternateKeyCode( aEvent.mnCode );
            if( aAlternate.nKeyCode )
            {
                aEvent.mnCode = aAlternate.nKeyCode;
                if( aAlternate.nCharCode )
                    aEvent.mnCharCode = aAlternate.nCharCode;
                bStopProcessingKey = CallCallbackExc(SalEvent::KeyInput, &aEvent);
            }
        }
        if( bSendRelease && ! aDel.isDeleted() )
        {
            CallCallbackExc(SalEvent::KeyUp, &aEvent);
        }
    }
    else
        bStopProcessingKey = CallCallbackExc(SalEvent::KeyUp, &aEvent);
    return bStopProcessingKey;
}

GtkSalFrame::GtkSalFrame( SalFrame* pParent, SalFrameStyleFlags nStyle )
    : m_nXScreen( getDisplay()->GetDefaultXScreen() )
    , m_pHeaderBar(nullptr)
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
    , m_pHeaderBar(nullptr)
    , m_pGraphics(nullptr)
    , m_bGraphics(false)
{
    getDisplay()->registerFrame( this );
    // permanently ignore errors from our unruly children ...
    GetGenericUnixSalData()->ErrorTrapPush();
    m_bDefaultPos       = true;
    m_bDefaultSize      = true;
    Init( pSysData );
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
        g_free(strval);

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
        sal_uIntPtr windowId = pSalFrame->GetNativeWindowHandle(pSalFrame->getWindow());
        gchar* aDBusWindowPath = g_strdup_printf( "/org/libreoffice/window/%lu", windowId );
        gchar* aDBusMenubarPath = g_strdup_printf( "/org/libreoffice/window/%lu/menus/menubar", windowId );

        // Set window properties.
        g_object_set_data_full( G_OBJECT( gdkWindow ), "g-lo-menubar", pMenuModel, ObjectDestroyedNotify );
        g_object_set_data_full( G_OBJECT( gdkWindow ), "g-lo-action-group", pActionGroup, ObjectDestroyedNotify );

        GdkDisplay *pDisplay = GtkSalFrame::getGdkDisplay();
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
        if (!bDesktopIsUnity)
        {
            GMenu *menu = g_menu_new ();
            GMenuItem* item;

            GMenu *firstsubmenu = g_menu_new ();

            OString sNew(OUStringToOString(VclResId(SV_BUTTONTEXT_NEW),
                RTL_TEXTENCODING_UTF8).replaceFirst("~", "_"));

            item = g_menu_item_new(sNew.getStr(), "app.New");
            g_menu_append_item( firstsubmenu, item );
            g_object_unref(item);

            g_menu_append_section( menu, nullptr, G_MENU_MODEL(firstsubmenu));
            g_object_unref(firstsubmenu);

            GMenu *secondsubmenu = g_menu_new ();

            OString sPreferences(OUStringToOString(VclResId(SV_STDTEXT_PREFERENCES),
                RTL_TEXTENCODING_UTF8).replaceFirst("~", "_"));

            item = g_menu_item_new(sPreferences.getStr(), "app.OptionsTreeDialog");
            g_menu_append_item( secondsubmenu, item );
            g_object_unref(item);

            g_menu_append_section( menu, nullptr, G_MENU_MODEL(secondsubmenu));
            g_object_unref(secondsubmenu);

            GMenu *thirdsubmenu = g_menu_new ();

            OString sHelp(OUStringToOString(VclResId(SV_BUTTONTEXT_HELP),
                RTL_TEXTENCODING_UTF8).replaceFirst("~", "_"));

            item = g_menu_item_new(sHelp.getStr(), "app.HelpIndex");
            g_menu_append_item( thirdsubmenu, item );
            g_object_unref(item);

            OString sAbout(OUStringToOString(VclResId(SV_STDTEXT_ABOUT),
                RTL_TEXTENCODING_UTF8).replaceFirst("~", "_"));

            item = g_menu_item_new(sAbout.getStr(), "app.About");
            g_menu_append_item( thirdsubmenu, item );
            g_object_unref(item);

            OString sQuit(OUStringToOString(VclResId(SV_MENU_MAC_QUITAPP),
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
        pGtkSalMenu->EnableUnity(true);
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

void GtkSalFrame::EnsureAppMenuWatch()
{
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
    m_aSmoothScrollIdle.Stop();
    m_aSmoothScrollIdle.ClearInvokeHandler();

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

    if (m_pParent)
    {
        m_pParent->m_aChildren.remove( this );
    }

    getDisplay()->deregisterFrame( this );

    if( m_pRegion )
    {
        cairo_region_destroy( m_pRegion );
    }

    m_pIMHandler.reset();

    //tdf#108705 remove grabs on event widget before
    //destroying event widget
    while (m_nGrabLevel)
        removeGrabLevel();

    GtkWidget *pEventWidget = getMouseEventWidget();
    for (auto handler_id : m_aMouseSignalIds)
        g_signal_handler_disconnect(G_OBJECT(pEventWidget), handler_id);
    if( m_pFixedContainer )
        gtk_widget_destroy( GTK_WIDGET( m_pFixedContainer ) );
    if( m_pEventBox )
        gtk_widget_destroy( GTK_WIDGET(m_pEventBox) );
    if( m_pTopLevelGrid )
        gtk_widget_destroy( GTK_WIDGET(m_pTopLevelGrid) );
    {
        SolarMutexGuard aGuard;

        if(m_nWatcherId)
            g_bus_unwatch_name(m_nWatcherId);

        if( m_pWindow )
        {
            g_object_set_data( G_OBJECT( m_pWindow ), "SalFrame", nullptr );

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
            gtk_widget_destroy( m_pWindow );
        }
    }
    if( m_pForeignParent )
        g_object_unref( G_OBJECT( m_pForeignParent ) );
    if( m_pForeignTopLevel )
        g_object_unref( G_OBJECT( m_pForeignTopLevel) );

    m_pGraphics.reset();

    if (m_pSurface)
        cairo_surface_destroy(m_pSurface);
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
    gtk_widget_set_size_request(GTK_WIDGET(m_pFixedContainer), nWidth, nHeight );
}

void GtkSalFrame::window_resize(long nWidth, long nHeight)
{
    m_nWidthRequest = nWidth;
    m_nHeightRequest = nHeight;
    gtk_window_set_default_size(GTK_WINDOW(m_pWindow), nWidth, nHeight);
    if (gtk_widget_get_visible(m_pWindow))
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
                                       &tinfo, GTypeFlags(0));
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

static void damaged(void *handle,
                    sal_Int32 nExtentsX, sal_Int32 nExtentsY,
                    sal_Int32 nExtentsWidth, sal_Int32 nExtentsHeight)
{
    GtkSalFrame* pThis = static_cast<GtkSalFrame*>(handle);
    pThis->damaged(nExtentsX, nExtentsY, nExtentsWidth, nExtentsHeight);
}

void GtkSalFrame::InitCommon()
{
    m_pSurface = nullptr;
    m_nGrabLevel = 0;
    m_bSalObjectSetPosSize = false;

    m_aDamageHandler.handle = this;
    m_aDamageHandler.damaged = ::damaged;

    m_aSmoothScrollIdle.SetInvokeHandler(LINK(this, GtkSalFrame, AsyncScroll));

    m_pTopLevelGrid = GTK_GRID(gtk_grid_new());
    gtk_container_add(GTK_CONTAINER(m_pWindow), GTK_WIDGET(m_pTopLevelGrid));

    m_pEventBox = GTK_EVENT_BOX(gtk_event_box_new());
    gtk_widget_add_events( GTK_WIDGET(m_pEventBox),
                           GDK_ALL_EVENTS_MASK );
    gtk_widget_set_vexpand(GTK_WIDGET(m_pEventBox), true);
    gtk_widget_set_hexpand(GTK_WIDGET(m_pEventBox), true);
    gtk_grid_attach(m_pTopLevelGrid, GTK_WIDGET(m_pEventBox), 0, 0, 1, 1);

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
    gtk_drag_dest_set(GTK_WIDGET(pEventWidget), GtkDestDefaults(0), nullptr, 0, GdkDragAction(0));
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
    g_signal_connect( G_OBJECT(m_pFixedContainer), "realize", G_CALLBACK(signalRealize), this );
    g_signal_connect( G_OBJECT(m_pFixedContainer), "size-allocate", G_CALLBACK(sizeAllocated), this );

    GtkGesture *pSwipe = gtk_gesture_swipe_new(pEventWidget);
    g_signal_connect(pSwipe, "swipe", G_CALLBACK(gestureSwipe), this);
    gtk_event_controller_set_propagation_phase(GTK_EVENT_CONTROLLER (pSwipe), GTK_PHASE_TARGET);
    g_object_weak_ref(G_OBJECT(pEventWidget), reinterpret_cast<GWeakNotify>(g_object_unref), pSwipe);

    GtkGesture *pLongPress = gtk_gesture_long_press_new(pEventWidget);
    g_signal_connect(pLongPress, "pressed", G_CALLBACK(gestureLongPress), this);
    gtk_event_controller_set_propagation_phase(GTK_EVENT_CONTROLLER (pLongPress), GTK_PHASE_TARGET);
    g_object_weak_ref(G_OBJECT(pEventWidget), reinterpret_cast<GWeakNotify>(g_object_unref), pLongPress);

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
    m_nKeyModifiers     = ModKeyFlags::NONE;
    m_bFullscreen       = false;
    m_bSpanMonitorsWhenFullscreen = false;
    m_nState            = GDK_WINDOW_STATE_WITHDRAWN;
    m_pIMHandler        = nullptr;
    m_pRegion           = nullptr;
    m_pDropTarget       = nullptr;
    m_pDragSource       = nullptr;
    m_bInDrag           = false;
    m_pFormatConversionRequest = nullptr;
    m_bGeometryIsProvisional = false;
    m_ePointerStyle     = static_cast<PointerStyle>(0xffff);
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
    gtk_widget_show_all(GTK_WIDGET(m_pTopLevelGrid));

    // realize the window, we need an XWindow id
    gtk_widget_realize( m_pWindow );

    //system data
    m_aSystemData.nSize         = sizeof( SystemEnvData );
    m_aSystemData.aWindow       = GetNativeWindowHandle(m_pWindow);
    m_aSystemData.aShellWindow  = reinterpret_cast<sal_IntPtr>(this);
    m_aSystemData.pSalFrame     = this;
    m_aSystemData.pWidget       = m_pWindow;
    m_aSystemData.nScreen       = m_nXScreen.getXScreen();
    m_aSystemData.pToolkit      = "gtk3";
    GdkScreen* pScreen = gtk_window_get_screen(GTK_WINDOW(m_pWindow));
    GdkVisual* pVisual = gdk_screen_get_system_visual(pScreen);

#if defined(GDK_WINDOWING_X11)
    GdkDisplay *pDisplay = getGdkDisplay();
    if (GDK_IS_X11_DISPLAY(pDisplay))
    {
        m_aSystemData.pDisplay = gdk_x11_display_get_xdisplay(pDisplay);
        m_aSystemData.pVisual = gdk_x11_visual_get_xvisual(pVisual);
    }
#endif

    m_bGraphics = false;
    m_pGraphics = nullptr;

    m_nFloatFlags = FloatWinPopupFlags::NONE;
    m_bFloatPositioned = false;

    m_nWidthRequest = 0;
    m_nHeightRequest = 0;

    // fake an initial geometry, gets updated via configure event or SetPosSize
    if (m_bDefaultPos || m_bDefaultSize)
    {
        Size aDefSize = calcDefaultSize();
        maGeometry.nX                   = -1;
        maGeometry.nY                   = -1;
        maGeometry.nWidth               = aDefSize.Width();
        maGeometry.nHeight              = aDefSize.Height();
        maGeometry.nTopDecoration       = 0;
        maGeometry.nBottomDecoration    = 0;
        maGeometry.nLeftDecoration      = 0;
        maGeometry.nRightDecoration     = 0;
    }
    updateScreenNumber();

    SetIcon(SV_ICON_ID_OFFICE);
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
        m_pWindow = gtk_window_new(eWinType);
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
        gtk_window_group_add_window(gtk_window_get_group(GTK_WINDOW(m_pParent->m_pWindow)), GTK_WINDOW(m_pWindow));
    }
    else
    {
        gtk_window_group_add_window(gtk_window_group_new(), GTK_WINDOW(m_pWindow));
        g_object_unref(gtk_window_get_group(GTK_WINDOW(m_pWindow)));
    }

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
        if( nStyle & SalFrameStyleFlags::INTRO )
        {
            gtk_window_set_role( GTK_WINDOW(m_pWindow), "splashscreen" );
            eType = GDK_WINDOW_TYPE_HINT_SPLASHSCREEN;
        }
        else if( nStyle & SalFrameStyleFlags::TOOLWINDOW )
        {
            eType = GDK_WINDOW_TYPE_HINT_DIALOG;
            gtk_window_set_skip_taskbar_hint( GTK_WINDOW(m_pWindow), true );
        }
        else if( nStyle & SalFrameStyleFlags::OWNERDRAWDECORATION )
        {
            eType = GDK_WINDOW_TYPE_HINT_TOOLBAR;
            gtk_window_set_focus_on_map(GTK_WINDOW(m_pWindow), false);
            gtk_window_set_decorated(GTK_WINDOW(m_pWindow), false);
        }
        gtk_window_set_type_hint( GTK_WINDOW(m_pWindow), eType );
        gtk_window_set_gravity( GTK_WINDOW(m_pWindow), GDK_GRAVITY_STATIC );
        gtk_window_set_resizable( GTK_WINDOW(m_pWindow), bool(nStyle & SalFrameStyleFlags::SIZEABLE) );

#if defined(GDK_WINDOWING_WAYLAND)
        //rhbz#1392145 under wayland/csd if we've overridden the default widget direction in order to set LibreOffice's
        //UI to the configured ui language but the system ui locale is a different text direction, then the toplevel
        //built-in close button of the titlebar follows the overridden direction rather than continue in the same
        //direction as every other titlebar on the user's desktop. So if they don't match set an explicit
        //header bar with the desired 'outside' direction
        if ((eType == GDK_WINDOW_TYPE_HINT_NORMAL || eType == GDK_WINDOW_TYPE_HINT_DIALOG) && GDK_IS_WAYLAND_DISPLAY(GtkSalFrame::getGdkDisplay()))
        {
            const bool bDesktopIsRTL = MsLangId::isRightToLeft(MsLangId::getSystemUILanguage());
            const bool bAppIsRTL = gtk_widget_get_default_direction() == GTK_TEXT_DIR_RTL;
            if (bDesktopIsRTL != bAppIsRTL)
            {
                m_pHeaderBar = GTK_HEADER_BAR(gtk_header_bar_new());
                gtk_widget_set_direction(GTK_WIDGET(m_pHeaderBar), bDesktopIsRTL ? GTK_TEXT_DIR_RTL : GTK_TEXT_DIR_LTR);
                gtk_header_bar_set_show_close_button(m_pHeaderBar, true);
                gtk_window_set_titlebar(GTK_WINDOW(m_pWindow), GTK_WIDGET(m_pHeaderBar));
                gtk_widget_show(GTK_WIDGET(m_pHeaderBar));
            }
        }
#endif
    }
    else if( nStyle & SalFrameStyleFlags::FLOAT )
        gtk_window_set_type_hint( GTK_WINDOW(m_pWindow), GDK_WINDOW_TYPE_HINT_POPUP_MENU );

    InitCommon();

    if( eWinType == GTK_WINDOW_TOPLEVEL )
    {
        // Enable DBus native menu if available.
        ensure_dbus_setup( this );

    }
}

GdkNativeWindow GtkSalFrame::findTopLevelSystemWindow( GdkNativeWindow )
{
    //FIXME: no findToplevelSystemWindow
    return 0;
}

void GtkSalFrame::Init( SystemParentData* pSysData )
{
    m_pParent = nullptr;
    m_aForeignParentWindow = pSysData->aWindow;
    m_pForeignParent = nullptr;
    m_aForeignTopLevelWindow = findTopLevelSystemWindow(pSysData->aWindow);
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

void GtkSalFrame::SetExtendedFrameStyle(SalExtStyle)
{
}

SalGraphics* GtkSalFrame::AcquireGraphics()
{
    if( m_bGraphics )
        return nullptr;

    if( !m_pGraphics )
    {
        m_pGraphics.reset( new GtkSalGraphics( this, m_pWindow ) );
        if (!m_pSurface)
        {
            AllocateFrame();
            TriggerPaintEvent();
        }
        m_pGraphics->setSurface(m_pSurface, m_aFrameSize);
    }
    m_bGraphics = true;
    return m_pGraphics.get();
}

void GtkSalFrame::ReleaseGraphics( SalGraphics* pGraphics )
{
    (void) pGraphics;
    assert( pGraphics == m_pGraphics.get() );
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
    {
        OString sTitle(OUStringToOString(rTitle, RTL_TEXTENCODING_UTF8));
        gtk_window_set_title(GTK_WINDOW(m_pWindow), sTitle.getStr());
        if (m_pHeaderBar)
            gtk_header_bar_set_title(m_pHeaderBar, sTitle.getStr());
    }
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
    g_free (appicon);
}

void GtkSalFrame::SetMenu( SalMenu* pSalMenu )
{
    m_pSalMenu = static_cast<GtkSalMenu*>(pSalMenu);
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
    Size aScreenSize(getDisplay()->GetScreenSize(GetDisplayScreen()));
    int scale = gtk_widget_get_scale_factor(m_pWindow);
    aScreenSize.setWidth( aScreenSize.Width() / scale );
    aScreenSize.setHeight( aScreenSize.Height() / scale );
    return bestmaxFrameSizeForScreenSize(aScreenSize);
}

void GtkSalFrame::SetDefaultSize()
{
    Size aDefSize = calcDefaultSize();

    SetPosSize( 0, 0, aDefSize.Width(), aDefSize.Height(),
                SAL_FRAME_POSSIZE_WIDTH | SAL_FRAME_POSSIZE_HEIGHT );

    if( (m_nStyle & SalFrameStyleFlags::DEFAULT) && m_pWindow )
        gtk_window_maximize( GTK_WINDOW(m_pWindow) );
}

void GtkSalFrame::Show( bool bVisible, bool /*bNoActivate*/ )
{
    if( m_pWindow )
    {
        if( bVisible )
        {
            getDisplay()->startupNotificationCompleted();

            if( m_bDefaultPos )
                Center();
            if( m_bDefaultSize )
                SetDefaultSize();
            setMinMaxSize();

            if (isFloatGrabWindow() && !getDisplay()->GetCaptureFrame())
            {
                m_pParent->grabPointer(true, true);
                m_pParent->addGrabLevel();
            }

#if defined(GDK_WINDOWING_WAYLAND)
            //rhbz#1334915, gnome#779143, tdf#100158
            //gtk under wayland lacks a way to change the app_id
            //of a window, so brute force everything as a
            //startcenter when initially shown to at least get
            //the default LibreOffice icon and not the broken
            //app icon
            if (GDK_IS_WAYLAND_DISPLAY(getGdkDisplay()))
            {
                OString sOrigName(g_get_prgname());
                g_set_prgname("libreoffice-startcenter");
                gtk_widget_show(m_pWindow);
                g_set_prgname(sOrigName.getStr());
            }
            else
            {
                gtk_widget_show(m_pWindow);
            }
#else
            gtk_widget_show(m_pWindow);
#endif

            if( isFloatGrabWindow() )
            {
                m_nFloats++;
                if (!getDisplay()->GetCaptureFrame())
                {
                    grabPointer(true, true);
                    addGrabLevel();
                }
                // #i44068# reset parent's IM context
                if( m_pParent )
                    m_pParent->EndExtTextInput(EndExtTextInputFlags::NONE);
            }
        }
        else
        {
            if( isFloatGrabWindow() )
            {
                m_nFloats--;
                if (!getDisplay()->GetCaptureFrame())
                {
                    removeGrabLevel();
                    grabPointer(false);
                    m_pParent->removeGrabLevel();
                    m_pParent->grabPointer(false);
                }
            }
            gtk_widget_hide( m_pWindow );
            if( m_pIMHandler )
                m_pIMHandler->focusChanged( false );
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

void GtkSalFrame::AllocateFrame()
{
    basegfx::B2IVector aFrameSize( maGeometry.nWidth, maGeometry.nHeight );
    if (!m_pSurface || m_aFrameSize.getX() != aFrameSize.getX() ||
                       m_aFrameSize.getY() != aFrameSize.getY() )
    {
        if( aFrameSize.getX() == 0 )
            aFrameSize.setX( 1 );
        if( aFrameSize.getY() == 0 )
            aFrameSize.setY( 1 );

        if (m_pSurface)
            cairo_surface_destroy(m_pSurface);

        m_pSurface = gdk_window_create_similar_surface(widget_get_window(m_pWindow),
                                                       CAIRO_CONTENT_COLOR_ALPHA,
                                                       aFrameSize.getX(),
                                                       aFrameSize.getY());
        m_aFrameSize = aFrameSize;

        cairo_surface_set_user_data(m_pSurface, SvpSalGraphics::getDamageKey(), &m_aDamageHandler, nullptr);
        SAL_INFO("vcl.gtk3", "allocated Frame size of " << maGeometry.nWidth << " x " << maGeometry.nHeight);

        if (m_pGraphics)
            m_pGraphics->setSurface(m_pSurface, m_aFrameSize);
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

        maGeometry.nX = nX;
        maGeometry.nY = nY;
        m_bGeometryIsProvisional = true;

        m_bDefaultPos = false;

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

void GtkSalFrame::GetWorkArea( tools::Rectangle& rRect )
{
    GdkScreen  *pScreen = gtk_window_get_screen(GTK_WINDOW(m_pWindow));
    tools::Rectangle aRetRect;
    int max = gdk_screen_get_n_monitors (pScreen);
    for (int i = 0; i < max; ++i)
    {
        GdkRectangle aRect;
        gdk_screen_get_monitor_workarea(pScreen, i, &aRect);
        tools::Rectangle aMonitorRect(aRect.x, aRect.y, aRect.x+aRect.width, aRect.y+aRect.height);
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

        updateScreenNumber();

        m_nState = GdkWindowState( m_nState | GDK_WINDOW_STATE_MAXIMIZED );
        m_aRestorePosSize = tools::Rectangle( Point( pState->mnX, pState->mnY ),
                                       Size( pState->mnWidth, pState->mnHeight ) );
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

    tools::Rectangle GetPosAndSize(GtkWindow *pWindow)
    {
        long nX, nY, nWidth, nHeight;
        GetPosAndSize(pWindow, nX, nY, nWidth, nHeight);
        return tools::Rectangle(nX, nY, nX + nWidth, nY + nHeight);
    }
}

bool GtkSalFrame::GetWindowState( SalFrameState* pState )
{
    pState->mnState = WindowStateState::Normal;
    pState->mnMask  = WindowStateMask::State;
    // rollup ? gtk 2.2 does not seem to support the shaded state
    if( m_nState & GDK_WINDOW_STATE_ICONIFIED )
        pState->mnState |= WindowStateState::Minimized;
    if( m_nState & GDK_WINDOW_STATE_MAXIMIZED )
    {
        pState->mnState |= WindowStateState::Maximized;
        pState->mnX                 = m_aRestorePosSize.Left();
        pState->mnY                 = m_aRestorePosSize.Top();
        pState->mnWidth             = m_aRestorePosSize.GetWidth();
        pState->mnHeight            = m_aRestorePosSize.GetHeight();
        GetPosAndSize(GTK_WINDOW(m_pWindow), pState->mnMaximizedX, pState->mnMaximizedY,
                                             pState->mnMaximizedWidth, pState->mnMaximizedHeight);
        pState->mnMask  |= WindowStateMask::MaximizedX          |
                           WindowStateMask::MaximizedY          |
                           WindowStateMask::MaximizedWidth      |
                           WindowStateMask::MaximizedHeight;
    }
    else
    {
        GetPosAndSize(GTK_WINDOW(m_pWindow), pState->mnX, pState->mnY,
                                             pState->mnWidth, pState->mnHeight);
    }
    pState->mnMask  |= WindowStateMask::X            |
                       WindowStateMask::Y            |
                       WindowStateMask::Width        |
                       WindowStateMask::Height;

    return true;
}

void GtkSalFrame::SetScreen( unsigned int nNewScreen, SetType eType, tools::Rectangle const *pSize )
{
    if( !m_pWindow )
        return;

    if (maGeometry.nDisplayScreenNumber == nNewScreen && eType == SetType::RetainSize)
        return;

    int nX = maGeometry.nX, nY = maGeometry.nY,
        nWidth = maGeometry.nWidth, nHeight = maGeometry.nHeight;
    GdkScreen *pScreen = nullptr;
    GdkRectangle aNewMonitor;

    bool bSpanAllScreens = nNewScreen == static_cast<unsigned int>(-1);
    m_bSpanMonitorsWhenFullscreen = bSpanAllScreens && getDisplay()->getSystem()->GetDisplayScreenCount() > 1;
    gint nMonitor = -1;
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

    if( eType == SetType::Fullscreen )
    {
        nX = aNewMonitor.x;
        nY = aNewMonitor.y;
        nWidth = aNewMonitor.width;
        nHeight = aNewMonitor.height;
        m_nStyle |= SalFrameStyleFlags::PARTIAL_FULLSCREEN;
        bResize = true;

        // #i110881# for the benefit of compiz set a max size here
        // else setting to fullscreen fails for unknown reasons
        m_aMaxSize.setWidth( aNewMonitor.width );
        m_aMaxSize.setHeight( aNewMonitor.height );
    }

    if( pSize && eType == SetType::UnFullscreen )
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

    gdk_window_set_fullscreen_mode( widget_get_window(m_pWindow), m_bSpanMonitorsWhenFullscreen
        ? GDK_FULLSCREEN_ON_ALL_MONITORS : GDK_FULLSCREEN_ON_CURRENT_MONITOR );

    GtkWidget* pMenuBarContainerWidget = m_pSalMenu ? m_pSalMenu->GetMenuBarContainerWidget() : nullptr;
    if( eType == SetType::Fullscreen )
    {
        if (pMenuBarContainerWidget)
            gtk_widget_hide(pMenuBarContainerWidget);
        if (m_bSpanMonitorsWhenFullscreen)
            gtk_window_fullscreen(GTK_WINDOW(m_pWindow));
        else
        {
            gtk_window_fullscreen_on_monitor(GTK_WINDOW(m_pWindow), pScreen, nMonitor);
        }

    }
    else if( eType == SetType::UnFullscreen )
    {
        if (pMenuBarContainerWidget)
            gtk_widget_show(pMenuBarContainerWidget);
        gtk_window_unfullscreen( GTK_WINDOW( m_pWindow ) );
    }

    if( eType == SetType::UnFullscreen &&
        !(m_nStyle & SalFrameStyleFlags::SIZEABLE) )
        gtk_window_set_resizable( GTK_WINDOW( m_pWindow ), FALSE );

    // FIXME: we should really let gtk+ handle our widget hierarchy ...
    if( m_pParent && gtk_widget_get_screen( m_pParent->m_pWindow ) != pScreen )
        SetParent( nullptr );
    std::list< GtkSalFrame* > aChildren = m_aChildren;
    for (auto const& child : aChildren)
        child->SetScreen( nNewScreen, SetType::RetainSize );

    m_bDefaultPos = m_bDefaultSize = false;
    updateScreenNumber();

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

        for (auto const& child : m_aChildren)
            child->SetApplicationID(rWMClass);
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
        SetScreen( nScreen, SetType::Fullscreen );
    }
    else
    {
        SetScreen( nScreen, SetType::UnFullscreen,
                   !m_aRestorePosSize.IsEmpty() ? &m_aRestorePosSize : nullptr );
        m_aRestorePosSize = tools::Rectangle();
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

static guint32 nLastUserInputTime = GDK_CURRENT_TIME;

guint32 GtkSalFrame::GetLastInputEventTime()
{
    return nLastUserInputTime;
}

void GtkSalFrame::UpdateLastInputEventTime(guint32 nUserInputTime)
{
    //gtk3 can generate a synthetic crossing event with a useless 0
    //(GDK_CURRENT_TIME) timestamp on showing a menu from the main
    //menubar, which is unhelpful, so ignore the 0 timestamps
    if (nUserInputTime == GDK_CURRENT_TIME)
        return;
    nLastUserInputTime = nUserInputTime;
}

void GtkSalFrame::ToTop( SalFrameToTop nFlags )
{
    if( m_pWindow )
    {
        if( isChild( false ) )
            gtk_widget_grab_focus( m_pWindow );
        else if( IS_WIDGET_MAPPED( m_pWindow ) )
        {
            if (!(nFlags & SalFrameToTop::GrabFocusOnly))
                gtk_window_present_with_time(GTK_WINDOW(m_pWindow), GetLastInputEventTime());
            else
                gdk_window_focus(widget_get_window(m_pWindow), GetLastInputEventTime());
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

#if GTK_CHECK_VERSION(3, 20, 0)
    if (gtk_check_version(3, 20, 0) == nullptr)
    {
        GdkSeat* pSeat = gdk_display_get_default_seat(getGdkDisplay());
        if (bGrab)
        {
            gdk_seat_grab(pSeat, widget_get_window(getMouseEventWidget()), GDK_SEAT_CAPABILITY_ALL_POINTING,
                          bOwnerEvents, nullptr, nullptr, nullptr, nullptr);
        }
        else
        {
            gdk_seat_ungrab(pSeat);
        }
        return;
    }
#endif

    //else older gtk3
    const int nMask = (GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK);

    GdkDeviceManager* pDeviceManager = gdk_display_get_device_manager(getGdkDisplay());
    GdkDevice* pPointer = gdk_device_manager_get_client_pointer(pDeviceManager);
    if (bGrab)
    {
        gdk_device_grab(pPointer, widget_get_window(getMouseEventWidget()), GDK_OWNERSHIP_NONE,
                        bOwnerEvents, GdkEventMask(nMask), m_pCurrentCursor, gtk_get_current_event_time());
    }
    else
    {
        gdk_device_ungrab(pPointer, gtk_get_current_event_time());
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

void GtkSalFrame::KeyCodeToGdkKey(const vcl::KeyCode& rKeyCode,
    guint* pGdkKeyCode, GdkModifierType *pGdkModifiers)
{
    if ( pGdkKeyCode == nullptr || pGdkModifiers == nullptr )
        return;

    // Get GDK key modifiers
    GdkModifierType nModifiers = GdkModifierType(0);

    if ( rKeyCode.IsShift() )
        nModifiers = static_cast<GdkModifierType>( nModifiers | GDK_SHIFT_MASK );

    if ( rKeyCode.IsMod1() )
        nModifiers = static_cast<GdkModifierType>( nModifiers | GDK_CONTROL_MASK );

    if ( rKeyCode.IsMod2() )
        nModifiers = static_cast<GdkModifierType>( nModifiers | GDK_MOD1_MASK );

    *pGdkModifiers = nModifiers;

    // Get GDK keycode.
    guint nKeyCode = 0;

    guint nCode = rKeyCode.GetCode();

    if ( nCode >= KEY_0 && nCode <= KEY_9 )
        nKeyCode = ( nCode - KEY_0 ) + GDK_KEY_0;
    else if ( nCode >= KEY_A && nCode <= KEY_Z )
        nKeyCode = ( nCode - KEY_A ) + GDK_KEY_A;
    else if ( nCode >= KEY_F1 && nCode <= KEY_F26 )
        nKeyCode = ( nCode - KEY_F1 ) + GDK_KEY_F1;
    else
    {
        switch (nCode)
        {
            case KEY_DOWN:          nKeyCode = GDK_KEY_Down;            break;
            case KEY_UP:            nKeyCode = GDK_KEY_Up;              break;
            case KEY_LEFT:          nKeyCode = GDK_KEY_Left;            break;
            case KEY_RIGHT:         nKeyCode = GDK_KEY_Right;           break;
            case KEY_HOME:          nKeyCode = GDK_KEY_Home;            break;
            case KEY_END:           nKeyCode = GDK_KEY_End;             break;
            case KEY_PAGEUP:        nKeyCode = GDK_KEY_Page_Up;         break;
            case KEY_PAGEDOWN:      nKeyCode = GDK_KEY_Page_Down;       break;
            case KEY_RETURN:        nKeyCode = GDK_KEY_Return;          break;
            case KEY_ESCAPE:        nKeyCode = GDK_KEY_Escape;          break;
            case KEY_TAB:           nKeyCode = GDK_KEY_Tab;             break;
            case KEY_BACKSPACE:     nKeyCode = GDK_KEY_BackSpace;       break;
            case KEY_SPACE:         nKeyCode = GDK_KEY_space;           break;
            case KEY_INSERT:        nKeyCode = GDK_KEY_Insert;          break;
            case KEY_DELETE:        nKeyCode = GDK_KEY_Delete;          break;
            case KEY_ADD:           nKeyCode = GDK_KEY_plus;            break;
            case KEY_SUBTRACT:      nKeyCode = GDK_KEY_minus;           break;
            case KEY_MULTIPLY:      nKeyCode = GDK_KEY_asterisk;        break;
            case KEY_DIVIDE:        nKeyCode = GDK_KEY_slash;           break;
            case KEY_POINT:         nKeyCode = GDK_KEY_period;          break;
            case KEY_COMMA:         nKeyCode = GDK_KEY_comma;           break;
            case KEY_LESS:          nKeyCode = GDK_KEY_less;            break;
            case KEY_GREATER:       nKeyCode = GDK_KEY_greater;         break;
            case KEY_EQUAL:         nKeyCode = GDK_KEY_equal;           break;
            case KEY_FIND:          nKeyCode = GDK_KEY_Find;            break;
            case KEY_CONTEXTMENU:   nKeyCode = GDK_KEY_Menu;            break;
            case KEY_HELP:          nKeyCode = GDK_KEY_Help;            break;
            case KEY_UNDO:          nKeyCode = GDK_KEY_Undo;            break;
            case KEY_REPEAT:        nKeyCode = GDK_KEY_Redo;            break;
            case KEY_DECIMAL:       nKeyCode = GDK_KEY_KP_Decimal;      break;
            case KEY_TILDE:         nKeyCode = GDK_KEY_asciitilde;      break;
            case KEY_QUOTELEFT:     nKeyCode = GDK_KEY_quoteleft;       break;
            case KEY_BRACKETLEFT:   nKeyCode = GDK_KEY_bracketleft;     break;
            case KEY_BRACKETRIGHT:  nKeyCode = GDK_KEY_bracketright;    break;
            case KEY_SEMICOLON:     nKeyCode = GDK_KEY_semicolon;       break;
            case KEY_QUOTERIGHT:    nKeyCode = GDK_KEY_quoteright;      break;

            // Special cases
            case KEY_COPY:          nKeyCode = GDK_KEY_Copy;            break;
            case KEY_CUT:           nKeyCode = GDK_KEY_Cut;             break;
            case KEY_PASTE:         nKeyCode = GDK_KEY_Paste;           break;
            case KEY_OPEN:          nKeyCode = GDK_KEY_Open;            break;
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
    if (gdk_keymap_get_scroll_lock_state(pKeyMap))
        nState |= KeyIndicatorState::SCROLLLOCK;

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
        m_pIMHandler.reset( new IMHandler( this ) );
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

    GtkSalGraphics* pGraphics = m_pGraphics.get();
    bool bFreeGraphics = false;
    if( ! pGraphics )
    {
        pGraphics = static_cast<GtkSalGraphics*>(AcquireGraphics());
        if ( !pGraphics )
        {
            SAL_WARN("vcl.gtk3", "Could not get graphics - unable to update settings");
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
    if (m_pParent)
    {
        gtk_window_group_remove_window(gtk_window_get_group(GTK_WINDOW(m_pParent->m_pWindow)), GTK_WINDOW(m_pWindow));
        m_pParent->m_aChildren.remove(this);
    }
    m_pParent = static_cast<GtkSalFrame*>(pNewParent);
    if (m_pParent)
    {
        m_pParent->m_aChildren.push_back(this);
        gtk_window_group_add_window(gtk_window_get_group(GTK_WINDOW(m_pParent->m_pWindow)), GTK_WINDOW(m_pWindow));
    }
    if( ! isChild() )
        gtk_window_set_transient_for( GTK_WINDOW(m_pWindow),
                                      (m_pParent && ! m_pParent->isChild(true,false)) ? GTK_WINDOW(m_pParent->m_pWindow) : nullptr
                                     );
}

bool GtkSalFrame::SetPluginParent( SystemParentData* )
{
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

void GtkSalFrame::PositionByToolkit(const tools::Rectangle& rRect, FloatWinPopupFlags nFlags)
{
    if (ImplGetSVData()->maNWFData.mbCanDetermineWindowPosition)
        return;

    m_aFloatRect = rRect;
    m_nFloatFlags = nFlags;
    m_bFloatPositioned = true;
}

void GtkSalFrame::SetModal(bool bModal)
{
    if (!m_pWindow)
        return;
    gtk_window_set_modal(GTK_WINDOW(m_pWindow), bModal);
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

bool GtkSalFrame::ShowTooltip(const OUString& rHelpText, const tools::Rectangle& rHelpArea)
{
    m_aTooltip = rHelpText;
    m_aHelpArea = rHelpArea;
    gtk_widget_trigger_tooltip_query(getMouseEventWidget());
    return true;
}

namespace
{
    void set_pointing_to(GtkPopover *pPopOver, vcl::Window* pParent, const tools::Rectangle& rHelpArea, const SalFrameGeometry& rGeometry)
    {
        GdkRectangle aRect;
        aRect.x = FloatingWindow::ImplConvertToAbsPos(pParent, rHelpArea).Left() - rGeometry.nX;
        aRect.y = rHelpArea.Top();
        aRect.width = 1;
        aRect.height = 1;

        GtkPositionType ePos = gtk_popover_get_position(pPopOver);
        switch (ePos)
        {
            case GTK_POS_BOTTOM:
            case GTK_POS_TOP:
                aRect.width = rHelpArea.GetWidth();
                break;
            case GTK_POS_RIGHT:
            case GTK_POS_LEFT:
                aRect.height = rHelpArea.GetHeight();
                break;
        }

        gtk_popover_set_pointing_to(pPopOver, &aRect);
    }
}

void* GtkSalFrame::ShowPopover(const OUString& rHelpText, vcl::Window* pParent, const tools::Rectangle& rHelpArea, QuickHelpFlags nFlags)
{
    GtkWidget *pWidget = gtk_popover_new(getMouseEventWidget());
    OString sUTF = OUStringToOString(rHelpText, RTL_TEXTENCODING_UTF8);
    GtkWidget *pLabel =  gtk_label_new(sUTF.getStr());
    gtk_container_add(GTK_CONTAINER(pWidget), pLabel);

    if (nFlags & QuickHelpFlags::Top)
        gtk_popover_set_position(GTK_POPOVER(pWidget), GTK_POS_BOTTOM);
    else if (nFlags & QuickHelpFlags::Bottom)
        gtk_popover_set_position(GTK_POPOVER(pWidget), GTK_POS_TOP);
    else if (nFlags & QuickHelpFlags::Left)
        gtk_popover_set_position(GTK_POPOVER(pWidget), GTK_POS_RIGHT);
    else if (nFlags & QuickHelpFlags::Right)
        gtk_popover_set_position(GTK_POPOVER(pWidget), GTK_POS_LEFT);

    set_pointing_to(GTK_POPOVER(pWidget), pParent, rHelpArea, maGeometry);

    gtk_popover_set_modal(GTK_POPOVER(pWidget), false);

    gtk_widget_show_all(pWidget);

    return pWidget;
}

bool GtkSalFrame::UpdatePopover(void* nId, const OUString& rHelpText, vcl::Window* pParent, const tools::Rectangle& rHelpArea)
{
    GtkWidget *pWidget = static_cast<GtkWidget*>(nId);

    set_pointing_to(GTK_POPOVER(pWidget), pParent, rHelpArea, maGeometry);

    GtkWidget *pLabel = gtk_bin_get_child(GTK_BIN(pWidget));
    OString sUTF = OUStringToOString(rHelpText, RTL_TEXTENCODING_UTF8);
    gtk_label_set_text(GTK_LABEL(pLabel), sUTF.getStr());

    return true;
}

bool GtkSalFrame::HidePopover(void* nId)
{
    GtkWidget *pWidget = static_cast<GtkWidget*>(nId);
    gtk_widget_destroy(pWidget);
    return true;
}

void GtkSalFrame::addGrabLevel()
{
    if (m_nGrabLevel == 0)
        gtk_grab_add(getMouseEventWidget());
    ++m_nGrabLevel;
}

void GtkSalFrame::removeGrabLevel()
{
    if (m_nGrabLevel > 0)
    {
        --m_nGrabLevel;
        if (m_nGrabLevel == 0)
            gtk_grab_remove(getMouseEventWidget());
    }
}

void GtkSalFrame::closePopup()
{
    if (!m_nFloats)
        return;
    ImplSVData* pSVData = ImplGetSVData();
    if (!pSVData->maWinData.mpFirstFloat)
        return;
    if (pSVData->maWinData.mpFirstFloat->ImplGetFrame() != this)
        return;
    pSVData->maWinData.mpFirstFloat->EndPopupMode(FloatWinPopupEndFlags::Cancel | FloatWinPopupEndFlags::CloseAll);
}

namespace
{
    //tdf#117981 translate embedded video window mouse events to parent coordinates
    void translate_coords(GdkWindow* pSourceWindow, GtkWidget* pTargetWidget, int& rEventX, int& rEventY)
    {
        gpointer user_data=nullptr;
        gdk_window_get_user_data(pSourceWindow, &user_data);
        GtkWidget* pRealEventWidget = static_cast<GtkWidget*>(user_data);
        if (pRealEventWidget)
        {
            gtk_widget_translate_coordinates(pRealEventWidget, pTargetWidget, rEventX, rEventY, &rEventX, &rEventY);
        }
    }
}

gboolean GtkSalFrame::signalButton( GtkWidget*, GdkEventButton* pEvent, gpointer frame )
{
    UpdateLastInputEventTime(pEvent->time);

    GtkSalFrame* pThis = static_cast<GtkSalFrame*>(frame);
    GtkWidget* pEventWidget = pThis->getMouseEventWidget();
    bool bDifferentEventWindow = pEvent->window != widget_get_window(pEventWidget);

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

    vcl::DeletionListener aDel( pThis );

    if (pThis->isFloatGrabWindow())
    {
        //rhbz#1505379 if the window that got the event isn't our one, or there's none
        //of our windows under the mouse then close this popup window
        if (bDifferentEventWindow ||
            gdk_device_get_window_at_position(pEvent->device, nullptr, nullptr) == nullptr)
        {
            if (pEvent->type == GDK_BUTTON_PRESS)
                pThis->closePopup();
            else if (pEvent->type == GDK_BUTTON_RELEASE)
                return true;
        }
    }

    int nEventX = pEvent->x;
    int nEventY = pEvent->y;

    if (bDifferentEventWindow)
        translate_coords(pEvent->window, pEventWidget, nEventX, nEventY);

    if (!aDel.isDeleted())
    {
        int frame_x = static_cast<int>(pEvent->x_root - nEventX);
        int frame_y = static_cast<int>(pEvent->y_root - nEventY);
        if (pThis->m_bGeometryIsProvisional || frame_x != pThis->maGeometry.nX || frame_y != pThis->maGeometry.nY)
        {
            pThis->m_bGeometryIsProvisional = false;
            pThis->maGeometry.nX = frame_x;
            pThis->maGeometry.nY = frame_y;
            ImplSVData* pSVData = ImplGetSVData();
            if (pSVData->maNWFData.mbCanDetermineWindowPosition)
                pThis->CallCallbackExc(SalEvent::Move, nullptr);
        }
    }

    if (!aDel.isDeleted())
    {
        aEvent.mnTime   = pEvent->time;
        aEvent.mnX      = static_cast<long>(pEvent->x_root) - pThis->maGeometry.nX;
        aEvent.mnY      = static_cast<long>(pEvent->y_root) - pThis->maGeometry.nY;
        aEvent.mnCode   = GetMouseModCode( pEvent->state );

        if( AllSettings::GetLayoutRTL() )
            aEvent.mnX = pThis->maGeometry.nWidth-1-aEvent.mnX;

        pThis->CallCallbackExc( nEventType, &aEvent );
    }

    return true;
}

void GtkSalFrame::LaunchAsyncScroll(GdkEvent const * pEvent)
{
    //if we don't match previous pending states, flush that queue now
    if (!m_aPendingScrollEvents.empty() && pEvent->scroll.state != m_aPendingScrollEvents.back()->scroll.state)
    {
        m_aSmoothScrollIdle.Stop();
        m_aSmoothScrollIdle.Invoke();
        assert(m_aPendingScrollEvents.empty());
    }
    //add scroll event to queue
    m_aPendingScrollEvents.push_back(gdk_event_copy(pEvent));
    if (!m_aSmoothScrollIdle.IsActive())
        m_aSmoothScrollIdle.Start();
}

IMPL_LINK_NOARG(GtkSalFrame, AsyncScroll, Timer *, void)
{
    assert(!m_aPendingScrollEvents.empty());

    SalWheelMouseEvent aEvent;

    GdkEvent* pEvent = m_aPendingScrollEvents.back();

    aEvent.mnTime = pEvent->scroll.time;
    aEvent.mnX = static_cast<sal_uLong>(pEvent->scroll.x);
    // --- RTL --- (mirror mouse pos)
    if (AllSettings::GetLayoutRTL())
        aEvent.mnX = maGeometry.nWidth - 1 - aEvent.mnX;
    aEvent.mnY = static_cast<sal_uLong>(pEvent->scroll.y);
    aEvent.mnCode = GetMouseModCode( pEvent->scroll.state );

    double delta_x(0.0), delta_y(0.0);
    for (auto pSubEvent : m_aPendingScrollEvents)
    {
        delta_x += pSubEvent->scroll.delta_x;
        delta_y += pSubEvent->scroll.delta_y;
        gdk_event_free(pSubEvent);
    }
    m_aPendingScrollEvents.clear();

    // rhbz#1344042 "Traditionally" in gtk3 we tool a single up/down event as
    // equating to 3 scroll lines and a delta of 120. So scale the delta here
    // by 120 where a single mouse wheel click is an incoming delta_x of 1
    // and divide that by 40 to get the number of scroll lines
    if (delta_x != 0.0)
    {
        aEvent.mnDelta = -delta_x * 120;
        aEvent.mnNotchDelta = aEvent.mnDelta < 0 ? -1 : +1;
        if (aEvent.mnDelta == 0)
            aEvent.mnDelta = aEvent.mnNotchDelta;
        aEvent.mbHorz = true;
        aEvent.mnScrollLines = std::abs(aEvent.mnDelta) / 40.0;
        CallCallbackExc(SalEvent::WheelMouse, &aEvent);
    }

    if (delta_y != 0.0)
    {
        aEvent.mnDelta = -delta_y * 120;
        aEvent.mnNotchDelta = aEvent.mnDelta < 0 ? -1 : +1;
        if (aEvent.mnDelta == 0)
            aEvent.mnDelta = aEvent.mnNotchDelta;
        aEvent.mbHorz = false;
        aEvent.mnScrollLines = std::abs(aEvent.mnDelta) / 40.0;
        CallCallbackExc(SalEvent::WheelMouse, &aEvent);
    }
}

gboolean GtkSalFrame::signalScroll(GtkWidget*, GdkEvent* pInEvent, gpointer frame)
{
    GdkEventScroll& rEvent = pInEvent->scroll;

    UpdateLastInputEventTime(rEvent.time);

    GtkSalFrame* pThis = static_cast<GtkSalFrame*>(frame);

    if (rEvent.direction == GDK_SCROLL_SMOOTH)
    {
        pThis->LaunchAsyncScroll(pInEvent);
        return true;
    }

    //if we have smooth scrolling previous pending states, flush that queue now
    if (!pThis->m_aPendingScrollEvents.empty())
    {
        pThis->m_aSmoothScrollIdle.Stop();
        pThis->m_aSmoothScrollIdle.Invoke();
        assert(pThis->m_aPendingScrollEvents.empty());
    }

    SalWheelMouseEvent aEvent;

    aEvent.mnTime = rEvent.time;
    aEvent.mnX = static_cast<sal_uLong>(rEvent.x);
    // --- RTL --- (mirror mouse pos)
    if (AllSettings::GetLayoutRTL())
        aEvent.mnX = pThis->maGeometry.nWidth - 1 - aEvent.mnX;
    aEvent.mnY = static_cast<sal_uLong>(rEvent.y);
    aEvent.mnCode = GetMouseModCode(rEvent.state);

    switch (rEvent.direction)
    {
        case GDK_SCROLL_UP:
            aEvent.mnDelta = 120;
            aEvent.mnNotchDelta = 1;
            aEvent.mnScrollLines = 3;
            aEvent.mbHorz = false;
            pThis->CallCallbackExc(SalEvent::WheelMouse, &aEvent);
            break;

        case GDK_SCROLL_DOWN:
            aEvent.mnDelta = -120;
            aEvent.mnNotchDelta = -1;
            aEvent.mnScrollLines = 3;
            aEvent.mbHorz = false;
            pThis->CallCallbackExc(SalEvent::WheelMouse, &aEvent);
            break;

        case GDK_SCROLL_LEFT:
            aEvent.mnDelta = 120;
            aEvent.mnNotchDelta = 1;
            aEvent.mnScrollLines = 3;
            aEvent.mbHorz = true;
            pThis->CallCallbackExc(SalEvent::WheelMouse, &aEvent);
            break;

        case GDK_SCROLL_RIGHT:
            aEvent.mnDelta = -120;
            aEvent.mnNotchDelta = -1;
            aEvent.mnScrollLines = 3;
            aEvent.mbHorz = true;
            pThis->CallCallbackExc(SalEvent::WheelMouse, &aEvent);
            break;
        default:
            break;
    }

    return true;
}

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

        pThis->CallCallbackExc(SalEvent::Swipe, &aEvent);
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

        pThis->CallCallbackExc(SalEvent::LongPress, &aEvent);
    }
}

gboolean GtkSalFrame::signalMotion( GtkWidget*, GdkEventMotion* pEvent, gpointer frame )
{
    UpdateLastInputEventTime(pEvent->time);

    GtkSalFrame* pThis = static_cast<GtkSalFrame*>(frame);
    GtkWidget* pEventWidget = pThis->getMouseEventWidget();
    bool bDifferentEventWindow = pEvent->window != widget_get_window(pEventWidget);

    //If a menu, e.g. font name dropdown, is open, then under wayland moving the
    //mouse in the top left corner of the toplevel window in a
    //0,0,float-width,float-height area generates motion events which are
    //delivered to the dropdown
    if (pThis->isFloatGrabWindow() && bDifferentEventWindow)
        return true;

    vcl::DeletionListener aDel( pThis );

    int nEventX = pEvent->x;
    int nEventY = pEvent->y;

    if (bDifferentEventWindow)
        translate_coords(pEvent->window, pEventWidget, nEventX, nEventY);

    int frame_x = static_cast<int>(pEvent->x_root - nEventX);
    int frame_y = static_cast<int>(pEvent->y_root - nEventY);

    if (pThis->m_bGeometryIsProvisional || frame_x != pThis->maGeometry.nX || frame_y != pThis->maGeometry.nY)
    {
        pThis->m_bGeometryIsProvisional = false;
        pThis->maGeometry.nX = frame_x;
        pThis->maGeometry.nY = frame_y;
        ImplSVData* pSVData = ImplGetSVData();
        if (pSVData->maNWFData.mbCanDetermineWindowPosition)
            pThis->CallCallbackExc(SalEvent::Move, nullptr);
    }

    if (!aDel.isDeleted())
    {
        SalMouseEvent aEvent;
        aEvent.mnTime   = pEvent->time;
        aEvent.mnX      = static_cast<long>(pEvent->x_root) - pThis->maGeometry.nX;
        aEvent.mnY      = static_cast<long>(pEvent->y_root) - pThis->maGeometry.nY;
        aEvent.mnCode   = GetMouseModCode( pEvent->state );
        aEvent.mnButton = 0;

        if( AllSettings::GetLayoutRTL() )
            aEvent.mnX = pThis->maGeometry.nWidth-1-aEvent.mnX;

        pThis->CallCallbackExc( SalEvent::MouseMove, &aEvent );
    }

    if (!aDel.isDeleted())
    {
        // ask for the next hint
        gint x, y;
        GdkModifierType mask;
        gdk_window_get_pointer( widget_get_window(GTK_WIDGET(pThis->m_pWindow)), &x, &y, &mask );
    }

    return true;
}

gboolean GtkSalFrame::signalCrossing( GtkWidget*, GdkEventCrossing* pEvent, gpointer frame )
{
    UpdateLastInputEventTime(pEvent->time);

    GtkSalFrame* pThis = static_cast<GtkSalFrame*>(frame);
    SalMouseEvent aEvent;
    aEvent.mnTime   = pEvent->time;
    aEvent.mnX      = static_cast<long>(pEvent->x_root) - pThis->maGeometry.nX;
    aEvent.mnY      = static_cast<long>(pEvent->y_root) - pThis->maGeometry.nY;
    aEvent.mnCode   = GetMouseModCode( pEvent->state );
    aEvent.mnButton = 0;

    if (AllSettings::GetLayoutRTL())
        aEvent.mnX = pThis->maGeometry.nWidth-1-aEvent.mnX;

    pThis->CallCallbackExc( (pEvent->type == GDK_ENTER_NOTIFY) ? SalEvent::MouseMove : SalEvent::MouseLeave, &aEvent );

    return true;
}

cairo_t* GtkSalFrame::getCairoContext() const
{
    cairo_t* cr = cairo_create(m_pSurface);
    assert(cr);
    return cr;
}

void GtkSalFrame::damaged(sal_Int32 nExtentsX, sal_Int32 nExtentsY,
                          sal_Int32 nExtentsWidth, sal_Int32 nExtentsHeight) const
{
#if OSL_DEBUG_LEVEL > 0
    if (dumpframes)
    {
        static int frame;
        OString tmp("/tmp/frame" + OString::number(frame++) + ".png");
        cairo_t* cr = getCairoContext();
        cairo_surface_write_to_png(cairo_get_target(cr), tmp.getStr());
        cairo_destroy(cr);
    }
#endif

    gtk_widget_queue_draw_area(GTK_WIDGET(m_pFixedContainer),
                               nExtentsX, nExtentsY,
                               nExtentsWidth, nExtentsHeight);
}

// blit our backing cairo surface to the target cairo context
gboolean GtkSalFrame::signalDraw(GtkWidget*, cairo_t *cr, gpointer frame)
{
    GtkSalFrame* pThis = static_cast<GtkSalFrame*>(frame);

    cairo_set_source_surface(cr, pThis->m_pSurface, 0, 0);
    cairo_paint(cr);

    return false;
}

void GtkSalFrame::sizeAllocated(GtkWidget* pWidget, GdkRectangle *pAllocation, gpointer frame)
{
    GtkSalFrame* pThis = static_cast<GtkSalFrame*>(frame);
    pThis->maGeometry.nWidth = pAllocation->width;
    pThis->maGeometry.nHeight = pAllocation->height;
    bool bRealized = gtk_widget_get_realized(pWidget);
    if (bRealized)
        pThis->AllocateFrame();
    pThis->CallCallbackExc( SalEvent::Resize, nullptr );
    if (bRealized && !pThis->m_bSalObjectSetPosSize)
        pThis->TriggerPaintEvent();
}

#if GTK_CHECK_VERSION(3,23,0)
namespace {

void swapDirection(GdkGravity& gravity)
{
    if (gravity == GDK_GRAVITY_NORTH_WEST)
        gravity = GDK_GRAVITY_NORTH_EAST;
    else if (gravity == GDK_GRAVITY_NORTH_EAST)
        gravity = GDK_GRAVITY_NORTH_WEST;
    else if (gravity == GDK_GRAVITY_SOUTH_WEST)
        gravity = GDK_GRAVITY_SOUTH_EAST;
    else if (gravity == GDK_GRAVITY_SOUTH_EAST)
        gravity = GDK_GRAVITY_SOUTH_WEST;
}

}
#endif

void GtkSalFrame::signalRealize(GtkWidget*, gpointer frame)
{
    GtkSalFrame* pThis = static_cast<GtkSalFrame*>(frame);
    pThis->AllocateFrame();
    if (pThis->m_bSalObjectSetPosSize)
        return;
    pThis->TriggerPaintEvent();

#if GTK_CHECK_VERSION(3,23,0)
    if (gtk_check_version(3, 23, 0) == nullptr && pThis->m_bFloatPositioned)
    {
        GdkGravity rect_anchor = GDK_GRAVITY_SOUTH_WEST, menu_anchor = GDK_GRAVITY_NORTH_WEST;

        if (pThis->m_nFloatFlags & FloatWinPopupFlags::Left)
        {
            rect_anchor = GDK_GRAVITY_NORTH_WEST;
            menu_anchor = GDK_GRAVITY_NORTH_EAST;
        }
        else if (pThis->m_nFloatFlags & FloatWinPopupFlags::Up)
        {
            rect_anchor = GDK_GRAVITY_NORTH_WEST;
            menu_anchor = GDK_GRAVITY_SOUTH_WEST;
        }
        else if (pThis->m_nFloatFlags & FloatWinPopupFlags::Right)
        {
            rect_anchor = GDK_GRAVITY_NORTH_EAST;
        }

        VclPtr<vcl::Window> pVclParent = pThis->GetWindow()->GetParent();
        if (pVclParent->HasMirroredGraphics() && pVclParent->IsRTLEnabled())
        {
            swapDirection(rect_anchor);
            swapDirection(menu_anchor);
        }

        tools::Rectangle aFloatRect = FloatingWindow::ImplConvertToAbsPos(pVclParent, pThis->m_aFloatRect);
        if (gdk_window_get_window_type(widget_get_window(pThis->m_pParent->m_pWindow)) != GDK_WINDOW_TOPLEVEL)
            aFloatRect.Move(-pThis->m_pParent->maGeometry.nX, -pThis->m_pParent->maGeometry.nY);

        GdkRectangle rect {static_cast<int>(aFloatRect.Left()), static_cast<int>(aFloatRect.Top()),
                           static_cast<int>(aFloatRect.GetWidth()), static_cast<int>(aFloatRect.GetHeight())};

        GdkWindow* gdkWindow = widget_get_window(pThis->m_pWindow);
        gdk_window_move_to_rect(gdkWindow, &rect, rect_anchor, menu_anchor, GDK_ANCHOR_FLIP, 0, 0);
    }
#endif
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
    if (pThis->m_bGeometryIsProvisional || x != pThis->maGeometry.nX || y != pThis->maGeometry.nY )
    {
        bMoved = true;
        pThis->m_bGeometryIsProvisional = false;
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
    {
        ImplSVData* pSVData = ImplGetSVData();
        if (pSVData->maNWFData.mbCanDetermineWindowPosition)
            pThis->CallCallbackExc(SalEvent::Move, nullptr);
    }

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
    CallCallbackExc(SalEvent::Paint, &aPaintEvt);
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
        pThis->m_nKeyModifiers = ModKeyFlags::NONE;

    if( pThis->m_pIMHandler )
        pThis->m_pIMHandler->focusChanged( pEvent->in != 0 );

    // ask for changed printers like generic implementation
    if( pEvent->in && pSalInstance->isPrinterInit() )
        pSalInstance->updatePrinterUpdate();

    // FIXME: find out who the hell steals the focus from our frame
    // while we have the pointer grabbed, this should not come from
    // the window manager. Is this an event that was still queued ?
    // The focus does not seem to get set inside our process

    // in the meantime do not propagate focus get/lose if floats are open
    if( m_nFloats == 0 )
        pThis->CallCallbackExc( pEvent->in ? SalEvent::GetFocus : SalEvent::LoseFocus, nullptr );

    return false;
}

gboolean GtkSalFrame::signalMap(GtkWidget *, GdkEvent*, gpointer frame)
{
    GtkSalFrame* pThis = static_cast<GtkSalFrame*>(frame);

    pThis->CallCallbackExc( SalEvent::Resize, nullptr );
    pThis->TriggerPaintEvent();

    return false;
}

gboolean GtkSalFrame::signalUnmap( GtkWidget*, GdkEvent*, gpointer frame )
{
    GtkSalFrame* pThis = static_cast<GtkSalFrame*>(frame);

    pThis->CallCallbackExc( SalEvent::Resize, nullptr );

    if (pThis->m_bFloatPositioned)
    {
        // Unrealize is needed for cases where we reuse the same popup
        // (e.g. the font name control), making the realize signal fire
        // again on next show.
        gtk_widget_unrealize(pThis->m_pWindow);
        pThis->m_bFloatPositioned = false;
    }

    return false;
}

gboolean GtkSalFrame::signalKey(GtkWidget* pWidget, GdkEventKey* pEvent, gpointer frame)
{
    UpdateLastInputEventTime(pEvent->time);

    GtkSalFrame* pThis = static_cast<GtkSalFrame*>(frame);

    if (pThis->isFloatGrabWindow())
        return signalKey(pWidget, pEvent, pThis->m_pParent);

    vcl::DeletionListener aDel( pThis );

    if( pThis->m_pIMHandler )
    {
        if( pThis->m_pIMHandler->handleKeyEvent( pEvent ) )
            return true;
    }

    bool bStopProcessingKey = false;

    // handle modifiers
    if( pEvent->keyval == GDK_KEY_Shift_L || pEvent->keyval == GDK_KEY_Shift_R ||
        pEvent->keyval == GDK_KEY_Control_L || pEvent->keyval == GDK_KEY_Control_R ||
        pEvent->keyval == GDK_KEY_Alt_L || pEvent->keyval == GDK_KEY_Alt_R ||
        pEvent->keyval == GDK_KEY_Meta_L || pEvent->keyval == GDK_KEY_Meta_R ||
        pEvent->keyval == GDK_KEY_Super_L || pEvent->keyval == GDK_KEY_Super_R )
    {
        sal_uInt16 nModCode = GetKeyModCode( pEvent->state );
        ModKeyFlags nExtModMask = ModKeyFlags::NONE;
        sal_uInt16 nModMask = 0;
        // pressing just the ctrl key leads to a keysym of XK_Control but
        // the event state does not contain ControlMask. In the release
        // event it's the other way round: it does contain the Control mask.
        // The modifier mode therefore has to be adapted manually.
        switch( pEvent->keyval )
        {
            case GDK_KEY_Control_L:
                nExtModMask = ModKeyFlags::LeftMod1;
                nModMask = KEY_MOD1;
                break;
            case GDK_KEY_Control_R:
                nExtModMask = ModKeyFlags::RightMod1;
                nModMask = KEY_MOD1;
                break;
            case GDK_KEY_Alt_L:
                nExtModMask = ModKeyFlags::LeftMod2;
                nModMask = KEY_MOD2;
                break;
            case GDK_KEY_Alt_R:
                nExtModMask = ModKeyFlags::RightMod2;
                nModMask = KEY_MOD2;
                break;
            case GDK_KEY_Shift_L:
                nExtModMask = ModKeyFlags::LeftShift;
                nModMask = KEY_SHIFT;
                break;
            case GDK_KEY_Shift_R:
                nExtModMask = ModKeyFlags::RightShift;
                nModMask = KEY_SHIFT;
                break;
            // Map Meta/Super to MOD3 modifier on all Unix systems
            // except Mac OS X
            case GDK_KEY_Meta_L:
            case GDK_KEY_Super_L:
                nExtModMask = ModKeyFlags::LeftMod3;
                nModMask = KEY_MOD3;
                break;
            case GDK_KEY_Meta_R:
            case GDK_KEY_Super_R:
                nExtModMask = ModKeyFlags::RightMod3;
                nModMask = KEY_MOD3;
                break;
        }

        SalKeyModEvent aModEvt;
        aModEvt.mbDown = pEvent->type == GDK_KEY_PRESS;
        aModEvt.mnCode = nModCode;

        if( pEvent->type == GDK_KEY_RELEASE )
        {
            aModEvt.mnModKeyCode = pThis->m_nKeyModifiers;
            nModCode &= ~nModMask;
            pThis->m_nKeyModifiers &= ~nExtModMask;
        }
        else
        {
            nModCode |= nModMask;
            pThis->m_nKeyModifiers |= nExtModMask;
            aModEvt.mnModKeyCode = pThis->m_nKeyModifiers;
        }

        pThis->CallCallbackExc( SalEvent::KeyModChange, &aModEvt );
    }
    else
    {
        bStopProcessingKey = pThis->doKeyCallback(pEvent->state,
                                                  pEvent->keyval,
                                                  pEvent->hardware_keycode,
                                                  pEvent->group,
                                                  sal_Unicode(gdk_keyval_to_unicode( pEvent->keyval )),
                                                  (pEvent->type == GDK_KEY_PRESS),
                                                  false);
        if( ! aDel.isDeleted() )
            pThis->m_nKeyModifiers = ModKeyFlags::NONE;
    }

    if( !aDel.isDeleted() && pThis->m_pIMHandler )
        pThis->m_pIMHandler->updateIMSpotLocation();

    return bStopProcessingKey;
}

gboolean GtkSalFrame::signalDelete( GtkWidget*, GdkEvent*, gpointer frame )
{
    GtkSalFrame* pThis = static_cast<GtkSalFrame*>(frame);

    //If we went into the backdrop we disabled the toplevel window, if we
    //receive a delete here, re-enable so we can process it
    bool bBackDrop = (gtk_widget_get_state_flags(GTK_WIDGET(pThis->m_pWindow)) & GTK_STATE_FLAG_BACKDROP);
    if (bBackDrop)
        pThis->GetWindow()->Enable();

    pThis->CallCallbackExc( SalEvent::Close, nullptr );

    return true;
}

void GtkSalFrame::signalStyleUpdated(GtkWidget*, gpointer frame)
{
    GtkSalFrame* pThis = static_cast<GtkSalFrame*>(frame);

    // note: settings changed for multiple frames is avoided in winproc.cxx ImplHandleSettings
    GtkSalFrame::getDisplay()->SendInternalEvent( pThis, nullptr, SalEvent::SettingsChanged );

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
        pInstance->ResetLastSeenCairoFontOptions(pCurrentCairoFontOptions);
        GtkSalFrame::getDisplay()->SendInternalEvent( pThis, nullptr, SalEvent::FontChanged );
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

    if ((pEvent->window_state.new_window_state & GDK_WINDOW_STATE_MAXIMIZED) &&
        !(pThis->m_nState & GDK_WINDOW_STATE_MAXIMIZED))
    {
        pThis->m_aRestorePosSize = GetPosAndSize(GTK_WINDOW(pThis->m_pWindow));
    }

    if ((pEvent->window_state.new_window_state & GDK_WINDOW_STATE_WITHDRAWN) &&
        !(pThis->m_nState & GDK_WINDOW_STATE_WITHDRAWN))
    {
        if (pThis->isFloatGrabWindow())
            pThis->closePopup();
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

gboolean GtkSalFrame::signalVisibility( GtkWidget*, GdkEventVisibility* /*pEvent*/, gpointer /*frame*/ )
{
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

namespace
{
    GdkDragAction getPreferredDragAction(sal_Int8 dragOperation)
    {
        GdkDragAction eAct(static_cast<GdkDragAction>(0));

        if (dragOperation & css::datatransfer::dnd::DNDConstants::ACTION_MOVE)
            eAct = GDK_ACTION_MOVE;
        else if (dragOperation & css::datatransfer::dnd::DNDConstants::ACTION_COPY)
            eAct = GDK_ACTION_COPY;
        else if (dragOperation & css::datatransfer::dnd::DNDConstants::ACTION_LINK)
            eAct = GDK_ACTION_LINK;

        return eAct;
    }
}

static bool g_DropSuccessSet = false;
static bool g_DropSuccess = false;

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
    virtual void SAL_CALL acceptDrop(sal_Int8 dragOperation) override
    {
        gdk_drag_status(m_pContext, getPreferredDragAction(dragOperation), m_nTime);
    }

    virtual void SAL_CALL rejectDrop() override
    {
        gdk_drag_status(m_pContext, static_cast<GdkDragAction>(0), m_nTime);
    }

    virtual void SAL_CALL dropComplete(sal_Bool bSuccess) override
    {
        gtk_drag_finish(m_pContext, bSuccess, false, m_nTime);
        if (GtkDragSource::g_ActiveDragSource)
        {
            g_DropSuccessSet = true;
            g_DropSuccess = bSuccess;
        }
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

    virtual css::uno::Any SAL_CALL getTransferData(const css::datatransfer::DataFlavor& rFlavor) override
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
    // ACTION_DEFAULT is documented as...
    // 'This means the user did not press any key during the Drag and Drop operation
    // and the action that was combined with ACTION_DEFAULT is the system default action'
    // in tdf#107031 writer won't insert a link when a heading is dragged from the
    // navigator unless this is set. Its unclear really what ACTION_DEFAULT means,
    // there is a deprecated 'GDK_ACTION_DEFAULT Means nothing, and should not be used'
    // possible equivalent in gtk.
    // So (tdf#109227) set ACTION_DEFAULT if no modifier key is held down
    GdkModifierType mask;
    gdk_window_get_pointer(widget_get_window(pWidget), nullptr, nullptr, &mask);
    if (!(mask & (GDK_CONTROL_MASK | GDK_SHIFT_MASK)))
        aEvent.DropAction |= css::datatransfer::dnd::DNDConstants::ACTION_DEFAULT;
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

    virtual void SAL_CALL acceptDrag(sal_Int8 dragOperation) override
    {
        gdk_drag_status(m_pContext, getPreferredDragAction(dragOperation), m_nTime);
    }

    virtual void SAL_CALL rejectDrag() override
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
    sal_Int8 nSourceActions = GdkToVcl(gdk_drag_context_get_actions(context));
    GdkModifierType mask;
    gdk_window_get_pointer(widget_get_window(pWidget), nullptr, nullptr, &mask);

    // tdf#109227 if a modifier is held down, default to the matching
    // action for that modifier combo, otherwise pick the preferred
    // default from the possible source actions
    sal_Int8 nNewDropAction = css::datatransfer::dnd::DNDConstants::ACTION_MOVE;
    if ((mask & GDK_SHIFT_MASK) && !(mask & GDK_CONTROL_MASK))
        nNewDropAction = css::datatransfer::dnd::DNDConstants::ACTION_MOVE;
    else if ((mask & GDK_CONTROL_MASK) && !(mask & GDK_SHIFT_MASK))
        nNewDropAction = css::datatransfer::dnd::DNDConstants::ACTION_COPY;
    else if ((mask & GDK_SHIFT_MASK) && (mask & GDK_CONTROL_MASK) )
        nNewDropAction = css::datatransfer::dnd::DNDConstants::ACTION_LINK;
    nNewDropAction &= nSourceActions;

    GdkDragAction eAction;
    if (!(mask & (GDK_CONTROL_MASK | GDK_SHIFT_MASK)) && !nNewDropAction)
        eAction = getPreferredDragAction(nSourceActions);
    else
        eAction = getPreferredDragAction(nNewDropAction);

    gdk_drag_status(context, eAction, time);
    aEvent.Context = pContext;
    aEvent.LocationX = x;
    aEvent.LocationY = y;
    //under wayland at least, the action selected by gdk_drag_status on the
    //context is not immediately available via gdk_drag_context_get_selected_action
    //so here we set the DropAction from what we selected on the context, not
    //what the context says is selected
    aEvent.DropAction = GdkToVcl(eAction);
    aEvent.SourceActions = nSourceActions;

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
}

void GtkSalFrame::signalDestroy( GtkWidget* pObj, gpointer frame )
{
    GtkSalFrame* pThis = static_cast<GtkSalFrame*>(frame);
    if( pObj == pThis->m_pWindow )
    {
        pThis->m_pFixedContainer = nullptr;
        pThis->m_pEventBox = nullptr;
        pThis->m_pTopLevelGrid = nullptr;
        pThis->m_pWindow = nullptr;
        pThis->m_xFrameWeld.reset();
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
    if(  m_pIMContext )
        return;

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

    GetGenericUnixSalData()->ErrorTrapPush();
    gtk_im_context_set_client_window(m_pIMContext, widget_get_window(m_pFrame->getMouseEventWidget()));
    gtk_im_context_focus_in( m_pIMContext );
    GetGenericUnixSalData()->ErrorTrapPop();
    m_bFocused = true;

}

void GtkSalFrame::IMHandler::deleteIMContext()
{
    if( m_pIMContext )
    {
        // first give IC a chance to deinitialize
        GetGenericUnixSalData()->ErrorTrapPush();
        gtk_im_context_set_client_window( m_pIMContext, nullptr );
        GetGenericUnixSalData()->ErrorTrapPop();
        // destroy old IC
        g_object_unref( m_pIMContext );
        m_pIMContext = nullptr;
    }
}

void GtkSalFrame::IMHandler::doCallEndExtTextInput()
{
    m_aInputEvent.mpTextAttr = nullptr;
    m_pFrame->CallCallbackExc( SalEvent::EndExtTextInput, nullptr );
}

void GtkSalFrame::IMHandler::updateIMSpotLocation()
{
    SalExtTextInputPosEvent aPosEvent;
    m_pFrame->CallCallbackExc( SalEvent::ExtTextInputPos, static_cast<void*>(&aPosEvent) );
    GdkRectangle aArea;
    aArea.x = aPosEvent.mnX;
    aArea.y = aPosEvent.mnY;
    aArea.width = aPosEvent.mnWidth;
    aArea.height = aPosEvent.mnHeight;
    GetGenericUnixSalData()->ErrorTrapPush();
    gtk_im_context_set_cursor_location( m_pIMContext, &aArea );
    GetGenericUnixSalData()->ErrorTrapPop();
}

void GtkSalFrame::IMHandler::sendEmptyCommit()
{
    vcl::DeletionListener aDel( m_pFrame );

    SalExtTextInputEvent aEmptyEv;
    aEmptyEv.mpTextAttr         = nullptr;
    aEmptyEv.maText.clear();
    aEmptyEv.mnCursorPos        = 0;
    aEmptyEv.mnCursorFlags      = 0;
    m_pFrame->CallCallbackExc( SalEvent::ExtTextInput, static_cast<void*>(&aEmptyEv) );
    if( ! aDel.isDeleted() )
        m_pFrame->CallCallbackExc( SalEvent::EndExtTextInput, nullptr );
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
        GetGenericUnixSalData()->ErrorTrapPush();
        gtk_im_context_focus_in( m_pIMContext );
        GetGenericUnixSalData()->ErrorTrapPop();
        if( m_aInputEvent.mpTextAttr )
        {
            sendEmptyCommit();
            // begin preedit again
            GtkSalFrame::getDisplay()->SendInternalEvent( m_pFrame, &m_aInputEvent, SalEvent::ExtTextInput );
        }
    }
    else
    {
        GetGenericUnixSalData()->ErrorTrapPush();
        gtk_im_context_focus_out( m_pIMContext );
        GetGenericUnixSalData()->ErrorTrapPop();
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
        m_aPrevKeyPresses.emplace_back(pEvent );
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
            SAL_WARN_IF( m_nPrevKeyPresses <= 0, "vcl.gtk3", "key press has vanished !" );
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
        case GDK_KEY_KP_Enter:
        case GDK_KEY_Return:
            if( cCode != '\n' && cCode != '\r' )
                bRet = false;
            break;
        case GDK_KEY_space:
        case GDK_KEY_KP_Space:
            if( cCode != ' ' )
                bRet = false;
            break;
        default:
            break;
    }
    return bRet;
}

void GtkSalFrame::IMHandler::signalIMCommit( GtkIMContext* /*pContext*/, gchar* pText, gpointer im_handler )
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
                pThis->m_pFrame->doKeyCallback( rKP.state, rKP.keyval, rKP.hardware_keycode, rKP.group, aOrigCode, true, true );
                bSingleCommit = true;
            }
        }
        if( ! bSingleCommit )
        {
            pThis->m_pFrame->CallCallbackExc( SalEvent::ExtTextInput, static_cast<void*>(&pThis->m_aInputEvent));
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

    pThis->m_aInputFlags = std::vector<ExtTextInputAttr>( std::max( 1, static_cast<int>(pThis->m_aInputEvent.maText.getLength()) ), ExtTextInputAttr::NONE );

    PangoAttrIterator *iter = pango_attr_list_get_iterator(pAttrs);
    do
    {
        GSList *attr_list = nullptr;
        GSList *tmp_list = nullptr;
        gint start, end;
        ExtTextInputAttr sal_attr = ExtTextInputAttr::NONE;

        pango_attr_iterator_range (iter, &start, &end);
        if (start == G_MAXINT || end == G_MAXINT)
        {
            auto len = pText ? g_utf8_strlen(pText, -1) : 0;
            if (end == G_MAXINT)
                end = len;
            if (start == G_MAXINT)
                start = len;
        }
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
                "vcl.gtk3", "pango attrib out of range. Broken range: "
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

    pThis->m_pFrame->CallCallbackExc( SalEvent::ExtTextInput, static_cast<void*>(&pThis->m_aInputEvent));
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
    FindFocus(uno::Reference< accessibility::XAccessibleContext > const & xContext)
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
            if (xState->contains(accessibility::AccessibleStateType::MANAGES_DESCENDANTS))
                return uno::Reference< accessibility::XAccessibleEditableText >();
        }
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
        SAL_WARN( "vcl.gtk3", "Exception in getting input method surrounding text: " << e);
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
    tools::Rectangle aRect = m_pSys->GetDisplayScreenPosSizePixel( nDisplayScreen );
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
                              const css::uno::Reference<css::datatransfer::dnd::XDragSourceListener>& rListener)
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
        g_DropSuccessSet = false;
        g_DropSuccess = false;

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

    GdkDragContext *pContext = gtk_drag_begin_with_coordinates(getMouseEventWidget(),
                                                               pTargetList,
                                                               sourceActions,
                                                               nButton,
                                                               &aFakeEvent,
                                                               nDragOriginX,
                                                               nDragOriginY);

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
        // an internal drop can accept the drop but fail with dropComplete( false )
        // this is different than the GTK API
        if (g_DropSuccessSet)
            aEv.DropSuccess = g_DropSuccess;
        else
            aEv.DropSuccess = true;
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

bool GtkSalFrame::CallCallbackExc(SalEvent nEvent, const void* pEvent) const
{
    bool nRet = false;
    try
    {
        nRet = CallCallback(nEvent, pEvent);
    }
    catch (const css::uno::Exception&)
    {
        auto e = cppu::getCaughtException();
        GtkSalData *pSalData = static_cast<GtkSalData*>(GetSalData());
        pSalData->setException(e);
    }
    catch (std::exception & e)
    {
        static_cast<GtkSalData *>(GetSalData())->setException(
            css::uno::Any(
                css::uno::RuntimeException(
                    "wrapped std::exception "
                    + o3tl::runtimeToOUString(e.what()))));
    }
    catch (...)
    {
        static_cast<GtkSalData *>(GetSalData())->setException(
            css::uno::Any(
                css::uno::RuntimeException("wrapped unknown exception")));
    }
    return nRet;
}

void GtkSalFrame::nopaint_container_resize_children(GtkContainer *pContainer)
{
    m_bSalObjectSetPosSize = true;
    gtk_container_resize_children(pContainer);
    m_bSalObjectSetPosSize = false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
