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

#pragma once

#include <sal/config.h>

#include <string_view>

#include <atk/atk.h>
#include <gtk/gtk.h>
#include <gtk/gtk-a11y.h>
#include <com/sun/star/accessibility/XAccessible.hpp>

extern "C" {

namespace com::sun::star::accessibility {
    class XAccessibleAction;
    class XAccessibleComponent;
    class XAccessibleEditableText;
    class XAccessibleHypertext;
    class XAccessibleImage;
    class XAccessibleMultiLineText;
    class XAccessibleSelection;
    class XAccessibleTable;
    class XAccessibleTableSelection;
    class XAccessibleText;
    class XAccessibleTextMarkup;
    class XAccessibleTextAttributes;
    class XAccessibleValue;
}

struct AtkObjectWrapper
{
    GtkWidgetAccessible aParent;

    AtkObject* mpOrig;  //if we're a GtkDrawingArea acting as a custom LibreOffice widget, this is the toolkit default impl
    AtkObject* mpSysObjChild; //if we're a container for a sysobj, then this is the sysobj native gtk AtkObject

    css::uno::Reference<css::accessibility::XAccessible> mpAccessible;
    css::uno::Reference<css::accessibility::XAccessibleContext> mpContext;
    css::uno::Reference<css::accessibility::XAccessibleAction> mpAction;
    css::uno::Reference<css::accessibility::XAccessibleComponent> mpComponent;
    css::uno::Reference<css::accessibility::XAccessibleEditableText>
        mpEditableText;
    css::uno::Reference<css::accessibility::XAccessibleHypertext> mpHypertext;
    css::uno::Reference<css::accessibility::XAccessibleImage> mpImage;
    css::uno::Reference<css::accessibility::XAccessibleMultiLineText>
        mpMultiLineText;
    css::uno::Reference<css::accessibility::XAccessibleSelection> mpSelection;
    css::uno::Reference<css::accessibility::XAccessibleTable> mpTable;
    css::uno::Reference<css::accessibility::XAccessibleTableSelection> mpTableSelection;
    css::uno::Reference<css::accessibility::XAccessibleText> mpText;
    css::uno::Reference<css::accessibility::XAccessibleTextMarkup> mpTextMarkup;
    css::uno::Reference<css::accessibility::XAccessibleTextAttributes>
        mpTextAttributes;
    css::uno::Reference<css::accessibility::XAccessibleValue> mpValue;

    AtkObject *child_about_to_be_removed;
    gint       index_of_child_about_to_be_removed;
//    OString * m_pKeyBindings
};

struct AtkObjectWrapperClass
{
    GtkWidgetAccessibleClass aParentClass;
};

GType                  atk_object_wrapper_get_type() G_GNUC_CONST;
AtkObject *            atk_object_wrapper_ref(
    const css::uno::Reference< css::accessibility::XAccessible >& rxAccessible,
    bool create = true );

AtkObject *            atk_object_wrapper_new(
    const css::uno::Reference< css::accessibility::XAccessible >& rxAccessible,
    AtkObject* parent = nullptr, AtkObject* orig = nullptr );

void                   atk_object_wrapper_add_child(AtkObjectWrapper* wrapper, AtkObject *child, gint index);
void                   atk_object_wrapper_remove_child(AtkObjectWrapper* wrapper, AtkObject *child, gint index);
void                   atk_object_wrapper_set_role(AtkObjectWrapper* wrapper, sal_Int16 role, sal_Int64 nStates);

void                   atk_object_wrapper_dispose(AtkObjectWrapper* wrapper);

AtkStateType mapAtkState( sal_Int64 nState );

AtkRelation*           atk_object_wrapper_relation_new(const css::accessibility::AccessibleRelation& rRelation);

void                   actionIfaceInit(gpointer iface_, gpointer);
void                   componentIfaceInit(gpointer iface_, gpointer);
void                   editableTextIfaceInit(gpointer iface_, gpointer);
void                   hypertextIfaceInit(gpointer iface_, gpointer);
void                   imageIfaceInit(gpointer iface_, gpointer);
void                   selectionIfaceInit(gpointer iface_, gpointer);
void                   tableIfaceInit(gpointer iface_, gpointer);
void                   tablecellIfaceInit(gpointer iface_, gpointer);
void                   textIfaceInit(gpointer iface_, gpointer);
void                   valueIfaceInit(gpointer iface_, gpointer);

} // extern "C"

#define ATK_TYPE_OBJECT_WRAPPER atk_object_wrapper_get_type()
#define ATK_OBJECT_WRAPPER(obj) \
    (G_TYPE_CHECK_INSTANCE_CAST ((obj), ATK_TYPE_OBJECT_WRAPPER, AtkObjectWrapper))
#define ATK_IS_OBJECT_WRAPPER(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), ATK_TYPE_OBJECT_WRAPPER))

static inline gchar *
OUStringToGChar(std::u16string_view rString )
{
    OString aUtf8 = OUStringToOString( rString, RTL_TEXTENCODING_UTF8 );
    return g_strdup( aUtf8.getStr() );
}

#define OUStringToConstGChar( string ) OUStringToOString( string, RTL_TEXTENCODING_UTF8 ).getStr()

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
