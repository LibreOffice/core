/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: atkwindow.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-29 16:20:01 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"

#include <plugins/gtk/gtkframe.hxx>
#include <vcl/window.hxx>

#include "atkwindow.hxx"
#include "atkregistry.hxx"

using namespace ::com::sun::star::accessibility;
using namespace ::com::sun::star::uno;

extern "C" {

static void (* window_real_initialize) (AtkObject *obj, gpointer data) = NULL;
static void (* window_real_finalize) (GObject *obj) = NULL;
static G_CONST_RETURN gchar* (* window_real_get_name) (AtkObject *accessible) = NULL;

static gint
ooo_window_wrapper_clear_focus(gpointer)
{
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

/*****************************************************************************/

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
            Reference< XAccessible > xAccessible( pWindow->GetAccessible(true) );
            ooo_wrapper_registry_add( xAccessible, obj );
            g_object_set_data( G_OBJECT(obj), "ooo:registry-key", xAccessible.get() );
        }
    }

    /* GetAtkRole returns ATK_ROLE_INVALID for all non VCL windows, i.e.
     * native Gtk+ file picker etc.
     */
    AtkRole newRole = GtkSalFrame::GetAtkRole( GTK_WINDOW( data ) );
    if( newRole != ATK_ROLE_INVALID )
        obj->role = newRole;

    if( obj->role == ATK_ROLE_TOOL_TIP )
    {
        /* HACK: Avoid endless loop when get_name is called from
         * gail_window_new() context, which leads to the code path
         * showing up in wrapper_factory_create_accessible with no
         * accessible assigned to the GtkWindow yet.
         */
        g_object_set_data( G_OBJECT( data ), "ooo:tooltip-accessible", obj );
    }

    g_signal_connect_after( GTK_WIDGET( data ), "focus-out-event",
                            G_CALLBACK (ooo_window_wrapper_real_focus_gtk),
                            NULL);
}

/*****************************************************************************/

static G_CONST_RETURN gchar*
ooo_window_wrapper_real_get_name(AtkObject *accessible)
{
    G_CONST_RETURN gchar* name = NULL;

    if( accessible->role == ATK_ROLE_TOOL_TIP )
    {
        AtkObject *child = atk_object_ref_accessible_child(accessible, 0);
        if( child )
        {
            name = atk_object_get_name(child);
            g_object_unref(child);
        }

        return name;
    }

    return window_real_get_name(accessible);
}

/*****************************************************************************/

static void
ooo_window_wrapper_real_finalize (GObject *obj)
{
    ooo_wrapper_registry_remove( (XAccessible *) g_object_get_data( obj, "ooo:registry-key" ));
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
     * "initialize" and "get_name".
     */

    data = g_type_class_peek_parent( klass );
    atk_class = ATK_OBJECT_CLASS (data);

    window_real_initialize = atk_class->initialize;
    atk_class->initialize = ooo_window_wrapper_real_initialize;

    window_real_get_name = atk_class->get_name;
    atk_class->get_name = ooo_window_wrapper_real_get_name;

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
            type_query.class_size,
            (GBaseInitFunc) NULL,
            (GBaseFinalizeFunc) NULL,
            (GClassInitFunc) ooo_window_wrapper_class_init,
            (GClassFinalizeFunc) NULL,
            NULL,
            type_query.instance_size,
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
    atk_class->get_name = window_real_get_name;
}

