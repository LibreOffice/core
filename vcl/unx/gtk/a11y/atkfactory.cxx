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
atk_noop_object_wrapper_get_type()
{
    static GType type = 0;

    if (!type)
    {
        static const GTypeInfo typeInfo =
        {
            sizeof (AtkNoOpObjectClass),
            nullptr,
            nullptr,
            reinterpret_cast<GClassInitFunc>(atk_noop_object_wrapper_class_init),
            nullptr,
            nullptr,
            sizeof (AtkObjectWrapper),
            0,
            nullptr,
            nullptr
        } ;

        type = g_type_register_static (ATK_TYPE_OBJECT, "OOoAtkNoOpObj", &typeInfo, (GTypeFlags)0) ;
  }
  return type;
}

AtkObject*
atk_noop_object_wrapper_new()
{
  AtkObject *accessible;

  accessible = static_cast<AtkObject *>(g_object_new (atk_noop_object_wrapper_get_type(), nullptr));
  g_return_val_if_fail (accessible != nullptr, nullptr);

  accessible->role = ATK_ROLE_INVALID;
  accessible->layer = ATK_LAYER_INVALID;

  return accessible;
}

/*
 * The wrapper factory
 */

static GType
wrapper_factory_get_accessible_type()
{
  return atk_object_wrapper_get_type();
}

static AtkObject*
wrapper_factory_create_accessible( GObject *obj )
{
#if GTK_CHECK_VERSION(3,0,0)
    GtkWidget* pEventBox = gtk_widget_get_parent(GTK_WIDGET(obj));

    // gail_container_real_remove_gtk tries to re-instanciate an accessible
    // for a widget that is about to vanish ..
    if (!pEventBox)
        return atk_noop_object_wrapper_new();

    GtkWidget* pTopLevelGrid = gtk_widget_get_parent(pEventBox);
    if (!pTopLevelGrid)
        return atk_noop_object_wrapper_new();

    GtkWidget* pTopLevel = gtk_widget_get_parent(pTopLevelGrid);
    if (!pTopLevel)
        return atk_noop_object_wrapper_new();
#else
    GtkWidget* pTopLevel = gtk_widget_get_parent(GTK_WIDGET(obj));

    // gail_container_real_remove_gtk tries to re-instanciate an accessible
    // for a widget that is about to vanish ..
    if (!pTopLevel)
        return atk_noop_object_wrapper_new();
#endif

    GtkSalFrame* pFrame = GtkSalFrame::getFromWindow(GTK_WINDOW(pTopLevel));
    g_return_val_if_fail( pFrame != nullptr, nullptr );

    vcl::Window* pFrameWindow = pFrame->GetWindow();
    if( pFrameWindow )
    {
        vcl::Window* pWindow = pFrameWindow;

        // skip accessible objects already exposed by the frame objects
        if( WINDOW_BORDERWINDOW == pWindow->GetType() )
            pWindow = pFrameWindow->GetAccessibleChildWindow(0);

        if( pWindow )
        {
             uno::Reference< accessibility::XAccessible > xAccessible = pWindow->GetAccessible();
            if( xAccessible.is() )
            {
                AtkObject *accessible = ooo_wrapper_registry_get( xAccessible );

                if( accessible )
                    g_object_ref( G_OBJECT(accessible) );
                else
                    accessible = atk_object_wrapper_new( xAccessible, gtk_widget_get_accessible(pTopLevel) );

                return accessible;
            }
        }
    }

    return nullptr;
}

AtkObject* ooo_fixed_get_accessible(GtkWidget *obj)
{
    return wrapper_factory_create_accessible(G_OBJECT(obj));
}

static void
wrapper_factory_class_init( AtkObjectFactoryClass *klass )
{
  klass->create_accessible   = wrapper_factory_create_accessible;
  klass->get_accessible_type = wrapper_factory_get_accessible_type;
}

GType
wrapper_factory_get_type()
{
  static GType t = 0;

  if (!t) {
    static const GTypeInfo tinfo =
    {
      sizeof (AtkObjectFactoryClass),
      nullptr, nullptr, reinterpret_cast<GClassInitFunc>(wrapper_factory_class_init),
      nullptr, nullptr, sizeof (AtkObjectFactory), 0, nullptr, nullptr
    };

    t = g_type_register_static (
        ATK_TYPE_OBJECT_FACTORY, "OOoAtkObjectWrapperFactory",
        &tinfo, (GTypeFlags) 0);
  }

  return t;
}

} // extern C

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
