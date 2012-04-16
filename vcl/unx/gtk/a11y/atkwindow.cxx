/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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


#include <unx/gtk/gtkframe.hxx>
#include <vcl/svapp.hxx>
#include <vcl/window.hxx>
#include "vcl/popupmenuwindow.hxx"

#include "atkwindow.hxx"
#include "atkwrapper.hxx"
#include "atkregistry.hxx"

#include <com/sun/star/accessibility/AccessibleRole.hpp>

using namespace ::com::sun::star::accessibility;
using namespace ::com::sun::star::uno;

extern "C" {

static void (* window_real_initialize) (AtkObject *obj, gpointer data) = NULL;
static void (* window_real_finalize) (GObject *obj) = NULL;

static void
init_from_window( AtkObject *accessible, Window *pWindow )
{
    static AtkRole aDefaultRole = ATK_ROLE_INVALID;

    // Special role for sub-menu and combo-box popups that are exposed directly
    // by their parents already.
    if( aDefaultRole == ATK_ROLE_INVALID )
        aDefaultRole = atk_role_register( "redundant object" );

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
            sal_uInt16 type = WINDOW_WINDOW;
            bool parentIsMenuFloatingWindow = false;

            Window *pParent = pWindow->GetParent();
            if( pParent ) {
                type = pParent->GetType();
                parentIsMenuFloatingWindow = ( TRUE == pParent->IsMenuFloatingWindow() );
            }

            if( (WINDOW_LISTBOX != type) && (WINDOW_COMBOBOX != type) &&
                (WINDOW_MENUBARWINDOW != type) && ! parentIsMenuFloatingWindow )
            {
                role = ATK_ROLE_WINDOW;
            }
        }
        break;

        default:
        {
            Window *pChild = pWindow->GetWindow(WINDOW_FIRSTCHILD);
            if( pChild )
            {
                if( WINDOW_HELPTEXTWINDOW == pChild->GetType() )
                {
                    role = ATK_ROLE_TOOL_TIP;
                    pChild->SetAccessibleRole( AccessibleRole::LABEL );
                    accessible->name = g_strdup( rtl::OUStringToOString( pChild->GetText(), RTL_TEXTENCODING_UTF8 ).getStr() );
                }
                else if ( pWindow->GetType() == WINDOW_BORDERWINDOW && pChild->GetType() == WINDOW_FLOATINGWINDOW )
                {
                    PopupMenuFloatingWindow* p = dynamic_cast<PopupMenuFloatingWindow*>(pChild);
                    if (p && p->IsPopupMenu() && p->GetMenuStackLevel() == 0)
                    {
                        // This is a top-level menu popup.  Register it.
                        role = ATK_ROLE_POPUP_MENU;
                        pChild->SetAccessibleRole( AccessibleRole::POPUP_MENU );
                        accessible->name = g_strdup( rtl::OUStringToOString( pChild->GetText(), RTL_TEXTENCODING_UTF8 ).getStr() );
                    }
                }
            }
            break;
        }
    }

    accessible->role = role;
}

/*****************************************************************************/

static gint
ooo_window_wrapper_clear_focus(gpointer)
{
    SolarMutexGuard aGuard;
    atk_focus_tracker_notify( NULL );
    return FALSE;
}

/*****************************************************************************/

static gboolean
ooo_window_wrapper_real_focus_gtk (GtkWidget *, GdkEventFocus *)
{
    g_idle_add( ooo_window_wrapper_clear_focus, NULL );
    return FALSE;
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
isChildPopupMenu(Window* pWindow)
{
    Window* pChild = pWindow->GetAccessibleChildWindow(0);
    if (!pChild)
        return false;

    if (WINDOW_FLOATINGWINDOW != pChild->GetType())
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
        Window *pWindow = pFrame->GetWindow();
        if( pWindow )
        {
            init_from_window( obj, pWindow );

            Reference< XAccessible > xAccessible( pWindow->GetAccessible(true) );

            /* We need the wrapper object for the top-level XAccessible to be
             * in the wrapper registry when atk traverses the hierachy up on
             * focus events
             */
            if( WINDOW_BORDERWINDOW == pWindow->GetType() )
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
                            NULL);

    if( obj->role == ATK_ROLE_TOOL_TIP )
    {
        g_signal_connect_after( GTK_WIDGET( data ), "map-event",
                                G_CALLBACK (ooo_tooltip_map),
                                NULL);
        g_signal_connect_after( GTK_WIDGET( data ), "unmap-event",
                                G_CALLBACK (ooo_tooltip_unmap),
                                NULL);
    }
}

/*****************************************************************************/

static void
ooo_window_wrapper_real_finalize (GObject *obj)
{
    ooo_wrapper_registry_remove( (XAccessible *) g_object_get_data( obj, "ooo:atk-wrapper-key" ));
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
ooo_window_wrapper_get_type (void)
{
    static GType type = 0;

    if (!type)
    {
        GType parent_type = g_type_from_name( "GailWindow" );

        if( ! parent_type )
        {
            g_warning( "Unknown type: GailWindow" );
            parent_type = ATK_TYPE_OBJECT;
        }

        GTypeQuery type_query;
        g_type_query( parent_type, &type_query );

        static const GTypeInfo typeInfo =
        {
            static_cast<guint16>(type_query.class_size),
            (GBaseInitFunc) NULL,
            (GBaseFinalizeFunc) NULL,
            (GClassInitFunc) ooo_window_wrapper_class_init,
            (GClassFinalizeFunc) NULL,
            NULL,
            static_cast<guint16>(type_query.instance_size),
            0,
            (GInstanceInitFunc) NULL,
            NULL
        } ;

        type = g_type_register_static (parent_type, "OOoWindowAtkObject", &typeInfo, (GTypeFlags)0) ;
    }

    return type;
}

void restore_gail_window_vtable (void)
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
