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

#ifndef INCLUDED_VCL_UNX_GTK_A11Y_ATKWRAPPER_HXX
#define INCLUDED_VCL_UNX_GTK_A11Y_ATKWRAPPER_HXX

#include <atk/atk.h>
#include <com/sun/star/accessibility/XAccessible.hpp>

extern "C" {

typedef struct _AtkObjectWrapper      AtkObjectWrapper;
typedef struct _AtkObjectWrapperClass AtkObjectWrapperClass;

namespace com { namespace sun { namespace star { namespace accessibility {
    class XAccessibleAction;
    class XAccessibleComponent;
    class XAccessibleEditableText;
    class XAccessibleHypertext;
    class XAccessibleImage;
    class XAccessibleMultiLineText;
    class XAccessibleSelection;
    class XAccessibleTable;
    class XAccessibleText;
    class XAccessibleTextMarkup;
    class XAccessibleTextAttributes;
    class XAccessibleValue;
} } } }

struct _AtkObjectWrapper
{
    AtkObject aParent;

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
    css::uno::Reference<css::accessibility::XAccessibleText> mpText;
    css::uno::Reference<css::accessibility::XAccessibleTextMarkup> mpTextMarkup;
    css::uno::Reference<css::accessibility::XAccessibleTextAttributes>
        mpTextAttributes;
    css::uno::Reference<css::accessibility::XAccessibleValue> mpValue;

    AtkObject *child_about_to_be_removed;
    gint       index_of_child_about_to_be_removed;
//    OString * m_pKeyBindings
};

struct _AtkObjectWrapperClass
{
    AtkObjectClass aParentClass;
};

GType                  atk_object_wrapper_get_type() G_GNUC_CONST;
AtkObject *            atk_object_wrapper_ref(
    const css::uno::Reference< css::accessibility::XAccessible >& rxAccessible,
    bool create = true );

AtkObject *            atk_object_wrapper_new(
    const css::uno::Reference< css::accessibility::XAccessible >& rxAccessible,
    AtkObject* parent = nullptr );

void                   atk_object_wrapper_add_child(AtkObjectWrapper* wrapper, AtkObject *child, gint index);
void                   atk_object_wrapper_remove_child(AtkObjectWrapper* wrapper, AtkObject *child, gint index);
void                   atk_object_wrapper_set_role(AtkObjectWrapper* wrapper, sal_Int16 role);

void                   atk_object_wrapper_dispose(AtkObjectWrapper* wrapper);

AtkStateType mapAtkState( sal_Int16 nState );

void                   actionIfaceInit(AtkActionIface *iface);
void                   componentIfaceInit(AtkComponentIface *iface);
void                   editableTextIfaceInit(AtkEditableTextIface *iface);
void                   hypertextIfaceInit(AtkHypertextIface *iface);
void                   imageIfaceInit(AtkImageIface *iface);
void                   selectionIfaceInit(AtkSelectionIface *iface);
void                   tableIfaceInit(AtkTableIface *iface);
void                   textIfaceInit(AtkTextIface *iface);
void                   valueIfaceInit(AtkValueIface *iface);

} // extern "C"

#define ATK_TYPE_OBJECT_WRAPPER atk_object_wrapper_get_type()
#define ATK_OBJECT_WRAPPER(obj) \
    (G_TYPE_CHECK_INSTANCE_CAST ((obj), ATK_TYPE_OBJECT_WRAPPER, AtkObjectWrapper))

static inline gchar *
OUStringToGChar(const OUString& rString )
{
    OString aUtf8 = OUStringToOString( rString, RTL_TEXTENCODING_UTF8 );
    return g_strdup( aUtf8.getStr() );
}

#define OUStringToConstGChar( string ) OUStringToOString( string, RTL_TEXTENCODING_UTF8 ).getStr()

#endif // INCLUDED_VCL_UNX_GTK_A11Y_ATKWRAPPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
