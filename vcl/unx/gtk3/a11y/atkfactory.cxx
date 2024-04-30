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
    GtkWidget* pEventBox = gtk_widget_get_parent(GTK_WIDGET(obj));
    assert(pEventBox);
    if (!pEventBox)
        return nullptr;

    GtkWidget* pTopLevelGrid = gtk_widget_get_parent(pEventBox);
    assert(pTopLevelGrid);
    if (!pTopLevelGrid)
        return nullptr;

    GtkWidget* pTopLevel = gtk_widget_get_parent(pTopLevelGrid);
    assert(pTopLevel);
    if (!pTopLevel)
        return nullptr;

    GtkSalFrame* pFrame = GtkSalFrame::getFromWindow(pTopLevel);
    assert(pFrame);
    if (!pFrame)
        return nullptr;

    vcl::Window* pFrameWindow = pFrame->GetWindow();
    assert(pFrameWindow);
    if( pFrameWindow )
    {
        vcl::Window* pWindow = pFrameWindow;

        // skip accessible objects already exposed by the frame objects
        if( WindowType::BORDERWINDOW == pWindow->GetType() )
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
                    accessible = atk_object_wrapper_new(xAccessible, gtk_widget_get_accessible(pEventBox));

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
wrapper_factory_class_init( gpointer klass_, gpointer )
{
  auto const klass = static_cast<AtkObjectFactoryClass *>(klass_);
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
      nullptr, nullptr, wrapper_factory_class_init,
      nullptr, nullptr, sizeof (AtkObjectFactory), 0, nullptr, nullptr
    };

    t = g_type_register_static (
        ATK_TYPE_OBJECT_FACTORY, "OOoAtkObjectWrapperFactory",
        &tinfo, GTypeFlags(0));
  }

  return t;
}

} // extern C

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
