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
#include <vcl/window.hxx>
#include "atkwrapper.hxx"
#include "atkfactory.hxx"
#include "atkregistry.hxx"

using namespace ::com::sun::star;

extern "C" {

/*
 *  Instances of this dummy object class are returned whenever we have to
 *  create an AtkObject, but can't touch the OOo object anymore since it
 *  is already disposed.
 */

static AtkStateSet *
noop_wrapper_ref_state_set( AtkObject * )
{
    AtkStateSet *state_set = atk_state_set_new();
    atk_state_set_add_state( state_set, ATK_STATE_DEFUNCT );
    return state_set;
}

static void
atk_noop_object_wrapper_class_init(AtkNoOpObjectClass *klass)
{
    AtkObjectClass *atk_class = ATK_OBJECT_CLASS( klass );
    atk_class->ref_state_set = noop_wrapper_ref_state_set;
}

static GType
atk_noop_object_wrapper_get_type(void)
{
    static GType type = 0;

    if (!type)
    {
        static const GTypeInfo typeInfo =
        {
            sizeof (AtkNoOpObjectClass),
            (GBaseInitFunc) NULL,
            (GBaseFinalizeFunc) NULL,
            (GClassInitFunc) atk_noop_object_wrapper_class_init,
            (GClassFinalizeFunc) NULL,
            NULL,
            sizeof (AtkObjectWrapper),
            0,
            (GInstanceInitFunc) NULL,
            NULL
        } ;

        type = g_type_register_static (ATK_TYPE_OBJECT, "OOoAtkNoOpObj", &typeInfo, (GTypeFlags)0) ;
  }
  return type;
}

AtkObject*
atk_noop_object_wrapper_new()
{
  AtkObject *accessible;

  accessible = (AtkObject *) g_object_new (atk_noop_object_wrapper_get_type(), NULL);
  g_return_val_if_fail (accessible != NULL, NULL);

  accessible->role = ATK_ROLE_INVALID;
  accessible->layer = ATK_LAYER_INVALID;

  return accessible;
}

/*
 * The wrapper factory
 */

static GType
wrapper_factory_get_accessible_type(void)
{
  return atk_object_wrapper_get_type();
}

static AtkObject*
wrapper_factory_create_accessible( GObject *obj )
{
    GtkWidget* parent_widget = gtk_widget_get_parent( GTK_WIDGET( obj ) );

    // gail_container_real_remove_gtk tries to re-instanciate an accessible
    // for a widget that is about to vanish ..
    if( ! parent_widget )
        return atk_noop_object_wrapper_new();

    GtkSalFrame* pFrame = GtkSalFrame::getFromWindow( GTK_WINDOW( parent_widget ) );
    g_return_val_if_fail( pFrame != NULL, NULL );

    vcl::Window* pFrameWindow = pFrame->GetWindow();
    if( pFrameWindow )
    {
        vcl::Window* pWindow = pFrameWindow;

        // skip accessible objects already exposed by the frame objects
        if( WINDOW_BORDERWINDOW == pWindow->GetType() )
            pWindow = pFrameWindow->GetAccessibleChildWindow(0);

        if( pWindow )
        {
             uno::Reference< accessibility::XAccessible > xAccessible = pWindow->GetAccessible(true);
            if( xAccessible.is() )
            {
                AtkObject *accessible = ooo_wrapper_registry_get( xAccessible );

                if( accessible )
                    g_object_ref( G_OBJECT(accessible) );
                else
                    accessible = atk_object_wrapper_new( xAccessible, gtk_widget_get_accessible(parent_widget) );

                return accessible;
            }
        }
    }

    return NULL;
}

static void
wrapper_factory_class_init( AtkObjectFactoryClass *klass )
{
  klass->create_accessible   = wrapper_factory_create_accessible;
  klass->get_accessible_type = wrapper_factory_get_accessible_type;
}

GType
wrapper_factory_get_type (void)
{
  static GType t = 0;

  if (!t) {
    static const GTypeInfo tinfo =
    {
      sizeof (AtkObjectFactoryClass),
      NULL, NULL, (GClassInitFunc) wrapper_factory_class_init,
      NULL, NULL, sizeof (AtkObjectFactory), 0, NULL, NULL
    };

    t = g_type_register_static (
        ATK_TYPE_OBJECT_FACTORY, "OOoAtkObjectWrapperFactory",
        &tinfo, (GTypeFlags) 0);
  }

  return t;
}

} // extern C

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
