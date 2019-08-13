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
#include <vcl/svapp.hxx>
#include <vcl/window.hxx>
#include <vcl/popupmenuwindow.hxx>
#include <sal/log.hxx>

#include "atkwindow.hxx"
#include "atkwrapper.hxx"
#include "atkregistry.hxx"

#include <com/sun/star/accessibility/AccessibleRole.hpp>

using namespace ::com::sun::star::accessibility;
using namespace ::com::sun::star::uno;

extern "C" {

static void (* window_real_initialize) (AtkObject *obj, gpointer data) = nullptr;
static void (* window_real_finalize) (GObject *obj) = nullptr;

static void
init_from_window( AtkObject *accessible, vcl::Window const *pWindow )
{
    static AtkRole aDefaultRole = ATK_ROLE_INVALID;

    // Special role for sub-menu and combo-box popups that are exposed directly
    // by their parents already.
    if( aDefaultRole == ATK_ROLE_INVALID )
    {
        SAL_WNODEPRECATED_DECLARATIONS_PUSH
        aDefaultRole = atk_role_register( "redundant object" );
        SAL_WNODEPRECATED_DECLARATIONS_POP
    }

    AtkRole role = aDefaultRole;

    // Determine the appropriate role for the GtkWindow
    switch( pWindow->GetAccessibleRole() )
    {
        case AccessibleRole::ALERT:
            role = ATK_ROLE_ALERT;
            break;

        case AccessibleRole::DIALOG:
            role = ATK_ROLE_DIALOG;
            break;

        case AccessibleRole::FRAME:
            role = ATK_ROLE_FRAME;
            break;

        /* Ignore window objects for sub-menus, combo- and list boxes,
         *  which are exposed as children of their parents.
         */
        case AccessibleRole::WINDOW:
        {
            WindowType type = WindowType::WINDOW;
            bool parentIsMenuFloatingWindow = false;

            vcl::Window *pParent = pWindow->GetParent();
            if( pParent ) {
                type = pParent->GetType();
                parentIsMenuFloatingWindow = pParent->IsMenuFloatingWindow();
            }

            if( (WindowType::LISTBOX != type) && (WindowType::COMBOBOX != type) &&
                (WindowType::MENUBARWINDOW != type) && ! parentIsMenuFloatingWindow )
            {
                role = ATK_ROLE_WINDOW;
            }
        }
        break;

        default:
        {
            vcl::Window *pChild = pWindow->GetWindow(GetWindowType::FirstChild);
            if( pChild )
            {
                if( WindowType::HELPTEXTWINDOW == pChild->GetType() )
                {
                    role = ATK_ROLE_TOOL_TIP;
                    pChild->SetAccessibleRole( AccessibleRole::LABEL );
                    accessible->name = g_strdup( OUStringToOString( pChild->GetText(), RTL_TEXTENCODING_UTF8 ).getStr() );
                }
                else if ( pWindow->GetType() == WindowType::BORDERWINDOW && pChild->GetType() == WindowType::FLOATINGWINDOW )
                {
                    PopupMenuFloatingWindow* p = dynamic_cast<PopupMenuFloatingWindow*>(pChild);
                    if (p && p->IsPopupMenu() && p->GetMenuStackLevel() == 0)
                    {
                        // This is a top-level menu popup.  Register it.
                        role = ATK_ROLE_POPUP_MENU;
                        pChild->SetAccessibleRole( AccessibleRole::POPUP_MENU );
                        accessible->name = g_strdup( OUStringToOString( pChild->GetText(), RTL_TEXTENCODING_UTF8 ).getStr() );
                    }
                }
            }
            break;
        }
    }

    accessible->role = role;
}

/*****************************************************************************/

static gboolean
ooo_window_wrapper_clear_focus(gpointer)
{
    SolarMutexGuard aGuard;
    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    atk_focus_tracker_notify( nullptr );
    SAL_WNODEPRECATED_DECLARATIONS_POP
    return false;
}

/*****************************************************************************/

static gboolean
ooo_window_wrapper_real_focus_gtk (GtkWidget *, GdkEventFocus *)
{
    g_idle_add( ooo_window_wrapper_clear_focus, nullptr );
    return false;
}

static gboolean ooo_tooltip_map( GtkWidget* pToolTip, gpointer )
{
    AtkObject* pAccessible = gtk_widget_get_accessible( pToolTip );
    if( pAccessible )
        atk_object_notify_state_change( pAccessible, ATK_STATE_SHOWING, TRUE );
    return FALSE;
}

static gboolean ooo_tooltip_unmap( GtkWidget* pToolTip, gpointer )
{
    AtkObject* pAccessible = gtk_widget_get_accessible( pToolTip );
    if( pAccessible )
        atk_object_notify_state_change( pAccessible, ATK_STATE_SHOWING, FALSE );
    return FALSE;
}

/*****************************************************************************/

static bool
isChildPopupMenu(vcl::Window* pWindow)
{
    vcl::Window* pChild = pWindow->GetAccessibleChildWindow(0);
    if (!pChild)
        return false;

    if (WindowType::FLOATINGWINDOW != pChild->GetType())
        return false;

    PopupMenuFloatingWindow* p = dynamic_cast<PopupMenuFloatingWindow*>(pChild);
    if (!p)
        return false;

    return p->IsPopupMenu();
}

static void
ooo_window_wrapper_real_initialize(AtkObject *obj, gpointer data)
{
    window_real_initialize(obj, data);

    GtkSalFrame *pFrame = GtkSalFrame::getFromWindow( GTK_WINDOW( data ) );
    if( pFrame )
    {
        vcl::Window *pWindow = pFrame->GetWindow();
        if( pWindow )
        {
            init_from_window( obj, pWindow );

            Reference< XAccessible > xAccessible( pWindow->GetAccessible() );

            /* We need the wrapper object for the top-level XAccessible to be
             * in the wrapper registry when atk traverses the hierarchy up on
             * focus events
             */
            if( WindowType::BORDERWINDOW == pWindow->GetType() )
            {
                if ( isChildPopupMenu(pWindow) )
                {
                    AtkObject *child = atk_object_wrapper_new( xAccessible, obj );
                    ooo_wrapper_registry_add( xAccessible, child );
                }
                else
                {
                    ooo_wrapper_registry_add( xAccessible, obj );
                    g_object_set_data( G_OBJECT(obj), "ooo:atk-wrapper-key", xAccessible.get() );
                }
            }
            else
            {
                AtkObject *child = atk_object_wrapper_new( xAccessible, obj );
                child->role = ATK_ROLE_FILLER;
                if( (ATK_ROLE_DIALOG == obj->role) || (ATK_ROLE_ALERT == obj->role) )
                    child->role = ATK_ROLE_OPTION_PANE;
                ooo_wrapper_registry_add( xAccessible, child );
            }
        }
    }

    g_signal_connect_after( GTK_WIDGET( data ), "focus-out-event",
                            G_CALLBACK (ooo_window_wrapper_real_focus_gtk),
                            nullptr);

    if( obj->role == ATK_ROLE_TOOL_TIP )
    {
        g_signal_connect_after( GTK_WIDGET( data ), "map-event",
                                G_CALLBACK (ooo_tooltip_map),
                                nullptr);
        g_signal_connect_after( GTK_WIDGET( data ), "unmap-event",
                                G_CALLBACK (ooo_tooltip_unmap),
                                nullptr);
    }
}

/*****************************************************************************/

static void
ooo_window_wrapper_real_finalize (GObject *obj)
{
    ooo_wrapper_registry_remove( static_cast<XAccessible *>(g_object_get_data( obj, "ooo:atk-wrapper-key" )));
    window_real_finalize( obj );
}

/*****************************************************************************/

static void
ooo_window_wrapper_class_init (AtkObjectClass *klass, gpointer)
{
    AtkObjectClass *atk_class;
    GObjectClass *gobject_class;
    gpointer data;

    /*
     * Patch the gobject vtable of GailWindow to refer to our instance of
     * "initialize".
     */

    data = g_type_class_peek_parent( klass );
    atk_class = ATK_OBJECT_CLASS (data);

    window_real_initialize = atk_class->initialize;
    atk_class->initialize = ooo_window_wrapper_real_initialize;

    gobject_class = G_OBJECT_CLASS (data);

    window_real_finalize = gobject_class->finalize;
    gobject_class->finalize = ooo_window_wrapper_real_finalize;
}

} // extern "C"

/*****************************************************************************/

GType
ooo_window_wrapper_get_type()
{
    static GType type = 0;

    if (!type)
    {
        GType parent_type = g_type_from_name( "GailWindow" );

        if( ! parent_type )
        {
            SAL_INFO("vcl.a11y", "Unknown type: GailWindow");
            parent_type = ATK_TYPE_OBJECT;
        }

        GTypeQuery type_query;
        g_type_query( parent_type, &type_query );

        static const GTypeInfo typeInfo =
        {
            static_cast<guint16>(type_query.class_size),
            nullptr,
            nullptr,
            reinterpret_cast<GClassInitFunc>(ooo_window_wrapper_class_init),
            nullptr,
            nullptr,
            static_cast<guint16>(type_query.instance_size),
            0,
            nullptr,
            nullptr
        } ;

        type = g_type_register_static (parent_type, "OOoWindowAtkObject", &typeInfo, GTypeFlags(0)) ;
    }

    return type;
}

void restore_gail_window_vtable()
{
    AtkObjectClass *atk_class;
    gpointer data;

    GType type = g_type_from_name( "GailWindow" );

    if( type == G_TYPE_INVALID )
        return;

    data = g_type_class_peek( type );
    atk_class = ATK_OBJECT_CLASS (data);

    atk_class->initialize = window_real_initialize;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
