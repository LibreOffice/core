/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef __ATK_WRAPPER_HXX__
#define __ATK_WRAPPER_HXX__

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

    ::com::sun::star::accessibility::XAccessible               *mpAccessible;
    ::com::sun::star::accessibility::XAccessibleContext        *mpContext;
    ::com::sun::star::accessibility::XAccessibleAction         *mpAction;
    ::com::sun::star::accessibility::XAccessibleComponent      *mpComponent;
    ::com::sun::star::accessibility::XAccessibleEditableText   *mpEditableText;
    ::com::sun::star::accessibility::XAccessibleHypertext      *mpHypertext;
    ::com::sun::star::accessibility::XAccessibleImage          *mpImage;
    ::com::sun::star::accessibility::XAccessibleMultiLineText  *mpMultiLineText;
    ::com::sun::star::accessibility::XAccessibleSelection      *mpSelection;
    ::com::sun::star::accessibility::XAccessibleTable          *mpTable;
    ::com::sun::star::accessibility::XAccessibleText           *mpText;
    ::com::sun::star::accessibility::XAccessibleTextMarkup     *mpTextMarkup;
    ::com::sun::star::accessibility::XAccessibleTextAttributes *mpTextAttributes;
    ::com::sun::star::accessibility::XAccessibleValue          *mpValue;

    AtkObject *child_about_to_be_removed;
    gint       index_of_child_about_to_be_removed;
//    ::rtl::OString * m_pKeyBindings
};

struct _AtkObjectWrapperClass
{
    AtkObjectClass aParentClass;
};

GType                  atk_object_wrapper_get_type (void) G_GNUC_CONST;
AtkObject *            atk_object_wrapper_ref(
    const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >& rxAccessible,
    bool create = true );

AtkObject *            atk_object_wrapper_new(
    const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >& rxAccessible,
    AtkObject* parent = NULL );

void                   atk_object_wrapper_add_child(AtkObjectWrapper* wrapper, AtkObject *child, gint index);
void                   atk_object_wrapper_remove_child(AtkObjectWrapper* wrapper, AtkObject *child, gint index);

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
OUStringToGChar(const rtl::OUString& rString )
{
    rtl::OString aUtf8 = rtl::OUStringToOString( rString, RTL_TEXTENCODING_UTF8 );
    return g_strdup( aUtf8.getStr() );
}

#define OUStringToConstGChar( string ) rtl::OUStringToOString( string, RTL_TEXTENCODING_UTF8 ).getStr()

#endif /* __ATK_WRAPPER_HXX__ */
