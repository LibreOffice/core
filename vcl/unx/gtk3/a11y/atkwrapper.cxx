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

#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Type.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleRelation.hpp>
#include <com/sun/star/accessibility/AccessibleRelationType.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/accessibility/XAccessibleText.hpp>
#include <com/sun/star/accessibility/XAccessibleValue.hpp>
#include <com/sun/star/accessibility/XAccessibleAction.hpp>
#include <com/sun/star/accessibility/XAccessibleContext.hpp>
#include <com/sun/star/accessibility/XAccessibleContext2.hpp>
#include <com/sun/star/accessibility/XAccessibleComponent.hpp>
#include <com/sun/star/accessibility/XAccessibleEventBroadcaster.hpp>
#include <com/sun/star/accessibility/XAccessibleRelationSet.hpp>
#include <com/sun/star/accessibility/XAccessibleTable.hpp>
#include <com/sun/star/accessibility/XAccessibleEditableText.hpp>
#include <com/sun/star/accessibility/XAccessibleExtendedAttributes.hpp>
#include <com/sun/star/accessibility/XAccessibleImage.hpp>
#include <com/sun/star/accessibility/XAccessibleHypertext.hpp>
#include <com/sun/star/accessibility/XAccessibleSelection.hpp>
#include <com/sun/star/awt/XWindow.hpp>

#include <rtl/strbuf.hxx>
#include <osl/diagnose.h>
#include <comphelper/diagnose_ex.hxx>
#include <vcl/syschild.hxx>
#include <vcl/sysdata.hxx>
#include <vcl/svapp.hxx>
#include <vcl/toolkit/unowrap.hxx>

#include "atkwrapper.hxx"
#include "atkregistry.hxx"
#include "atklistener.hxx"
#include "atktextattributes.hxx"

#include <vector>
#include <dlfcn.h>

using namespace ::com::sun::star;

static GObjectClass *parent_class = nullptr;

static AtkRelationType mapRelationType( sal_Int16 nRelation )
{
    AtkRelationType type = ATK_RELATION_NULL;

    switch( nRelation )
    {
        case accessibility::AccessibleRelationType::CONTENT_FLOWS_FROM:
            type = ATK_RELATION_FLOWS_FROM;
            break;

        case accessibility::AccessibleRelationType::CONTENT_FLOWS_TO:
            type = ATK_RELATION_FLOWS_TO;
            break;

        case accessibility::AccessibleRelationType::CONTROLLED_BY:
            type = ATK_RELATION_CONTROLLED_BY;
            break;

        case accessibility::AccessibleRelationType::CONTROLLER_FOR:
            type = ATK_RELATION_CONTROLLER_FOR;
            break;

        case accessibility::AccessibleRelationType::LABEL_FOR:
            type = ATK_RELATION_LABEL_FOR;
            break;

        case accessibility::AccessibleRelationType::LABELED_BY:
            type = ATK_RELATION_LABELLED_BY;
            break;

        case accessibility::AccessibleRelationType::MEMBER_OF:
            type = ATK_RELATION_MEMBER_OF;
            break;

        case accessibility::AccessibleRelationType::SUB_WINDOW_OF:
            type = ATK_RELATION_SUBWINDOW_OF;
            break;

        case accessibility::AccessibleRelationType::NODE_CHILD_OF:
            type = ATK_RELATION_NODE_CHILD_OF;
            break;

        default:
            break;
    }

    return type;
}

AtkStateType mapAtkState( sal_Int64 nState )
{
    AtkStateType type = ATK_STATE_INVALID;

    // A perfect / complete mapping ...
    switch( nState )
    {
#define MAP_DIRECT( a ) \
        case accessibility::AccessibleStateType::a: \
            type = ATK_STATE_##a; break

        MAP_DIRECT( INVALID );
        MAP_DIRECT( ACTIVE );
        MAP_DIRECT( ARMED );
        MAP_DIRECT( BUSY );
        MAP_DIRECT( CHECKABLE );
        MAP_DIRECT( CHECKED );
        MAP_DIRECT( EDITABLE );
        MAP_DIRECT( ENABLED );
        MAP_DIRECT( EXPANDABLE );
        MAP_DIRECT( EXPANDED );
        MAP_DIRECT( FOCUSABLE );
        MAP_DIRECT( FOCUSED );
        MAP_DIRECT( HORIZONTAL );
        MAP_DIRECT( ICONIFIED );
        MAP_DIRECT( INDETERMINATE );
        MAP_DIRECT( MANAGES_DESCENDANTS );
        MAP_DIRECT( MODAL );
        MAP_DIRECT( MULTI_LINE );
        MAP_DIRECT( OPAQUE );
        MAP_DIRECT( PRESSED );
        MAP_DIRECT( RESIZABLE );
        MAP_DIRECT( SELECTABLE );
        MAP_DIRECT( SELECTED );
        MAP_DIRECT( SENSITIVE );
        MAP_DIRECT( SHOWING );
        MAP_DIRECT( SINGLE_LINE );
        MAP_DIRECT( STALE );
        MAP_DIRECT( TRANSIENT );
        MAP_DIRECT( VERTICAL );
        MAP_DIRECT( VISIBLE );
        MAP_DIRECT( DEFAULT );
        // a spelling error ...
        case accessibility::AccessibleStateType::DEFUNC:
            type = ATK_STATE_DEFUNCT; break;
        case accessibility::AccessibleStateType::MULTI_SELECTABLE:
            type = ATK_STATE_MULTISELECTABLE; break;
    default:
        //Mis-use ATK_STATE_LAST_DEFINED to check if a state is unmapped
        //NOTE! Do not report it
        type = ATK_STATE_LAST_DEFINED;
        break;
    }

    return type;
}

static AtkRole mapToAtkRole(sal_Int16 nRole, sal_Int64 nStates)
{
    switch (nRole)
    {
        case accessibility::AccessibleRole::UNKNOWN:
            return ATK_ROLE_UNKNOWN;
        case accessibility::AccessibleRole::ALERT:
            return ATK_ROLE_ALERT;
        case accessibility::AccessibleRole::BLOCK_QUOTE:
            return ATK_ROLE_BLOCK_QUOTE;
        case accessibility::AccessibleRole::COLUMN_HEADER:
            return ATK_ROLE_COLUMN_HEADER;
        case accessibility::AccessibleRole::CANVAS:
            return ATK_ROLE_CANVAS;
        case accessibility::AccessibleRole::CHECK_BOX:
            return ATK_ROLE_CHECK_BOX;
        case accessibility::AccessibleRole::CHECK_MENU_ITEM:
            return ATK_ROLE_CHECK_MENU_ITEM;
        case accessibility::AccessibleRole::COLOR_CHOOSER:
            return ATK_ROLE_COLOR_CHOOSER;
        case accessibility::AccessibleRole::COMBO_BOX:
            return ATK_ROLE_COMBO_BOX;
        case accessibility::AccessibleRole::DATE_EDITOR:
            return ATK_ROLE_DATE_EDITOR;
        case accessibility::AccessibleRole::DESKTOP_ICON:
            return ATK_ROLE_DESKTOP_ICON;
        case accessibility::AccessibleRole::DESKTOP_PANE:
            return ATK_ROLE_DESKTOP_FRAME;
        case accessibility::AccessibleRole::DIRECTORY_PANE:
            return ATK_ROLE_DIRECTORY_PANE;
        case accessibility::AccessibleRole::DIALOG:
            return ATK_ROLE_DIALOG;
        case accessibility::AccessibleRole::DOCUMENT:
            return ATK_ROLE_DOCUMENT_FRAME;
        case accessibility::AccessibleRole::EMBEDDED_OBJECT:
            return ATK_ROLE_EMBEDDED;
        case accessibility::AccessibleRole::END_NOTE:
            return ATK_ROLE_FOOTNOTE;
        case accessibility::AccessibleRole::FILE_CHOOSER:
            return ATK_ROLE_FILE_CHOOSER;
        case accessibility::AccessibleRole::FILLER:
            return ATK_ROLE_FILLER;
        case accessibility::AccessibleRole::FONT_CHOOSER:
            return ATK_ROLE_FONT_CHOOSER;
        case accessibility::AccessibleRole::FOOTER:
            return ATK_ROLE_FOOTER;
        case accessibility::AccessibleRole::FOOTNOTE:
            return ATK_ROLE_FOOTNOTE;
        case accessibility::AccessibleRole::FRAME:
            return ATK_ROLE_FRAME;
        case accessibility::AccessibleRole::GLASS_PANE:
            return ATK_ROLE_GLASS_PANE;
        case accessibility::AccessibleRole::GRAPHIC:
            return ATK_ROLE_IMAGE;
        case accessibility::AccessibleRole::GROUP_BOX:
            return ATK_ROLE_GROUPING;
        case accessibility::AccessibleRole::HEADER:
            return ATK_ROLE_HEADER;
        case accessibility::AccessibleRole::HEADING:
            return ATK_ROLE_HEADING;
        case accessibility::AccessibleRole::HYPER_LINK:
            return ATK_ROLE_LINK;
        case accessibility::AccessibleRole::ICON:
            return ATK_ROLE_ICON;
        case accessibility::AccessibleRole::INTERNAL_FRAME:
            return ATK_ROLE_INTERNAL_FRAME;
        case accessibility::AccessibleRole::LABEL:
            return ATK_ROLE_LABEL;
        case accessibility::AccessibleRole::LAYERED_PANE:
            return ATK_ROLE_LAYERED_PANE;
        case accessibility::AccessibleRole::LIST:
            return ATK_ROLE_LIST;
        case accessibility::AccessibleRole::LIST_ITEM:
            return ATK_ROLE_LIST_ITEM;
        case accessibility::AccessibleRole::MENU:
            return ATK_ROLE_MENU;
        case accessibility::AccessibleRole::MENU_BAR:
            return ATK_ROLE_MENU_BAR;
        case accessibility::AccessibleRole::MENU_ITEM:
            return ATK_ROLE_MENU_ITEM;
        case accessibility::AccessibleRole::OPTION_PANE:
            return ATK_ROLE_OPTION_PANE;
        case accessibility::AccessibleRole::PAGE_TAB:
            return ATK_ROLE_PAGE_TAB;
        case accessibility::AccessibleRole::PAGE_TAB_LIST:
            return ATK_ROLE_PAGE_TAB_LIST;
        case accessibility::AccessibleRole::PANEL:
            return ATK_ROLE_PANEL;
        case accessibility::AccessibleRole::PARAGRAPH:
            return ATK_ROLE_PARAGRAPH;
        case accessibility::AccessibleRole::PASSWORD_TEXT:
            return ATK_ROLE_PASSWORD_TEXT;
        case accessibility::AccessibleRole::POPUP_MENU:
            return ATK_ROLE_POPUP_MENU;
        case accessibility::AccessibleRole::PUSH_BUTTON:
            return ATK_ROLE_PUSH_BUTTON;
        case accessibility::AccessibleRole::PROGRESS_BAR:
            return ATK_ROLE_PROGRESS_BAR;
        case accessibility::AccessibleRole::RADIO_BUTTON:
            return ATK_ROLE_RADIO_BUTTON;
        case accessibility::AccessibleRole::RADIO_MENU_ITEM:
            return ATK_ROLE_RADIO_MENU_ITEM;
        case accessibility::AccessibleRole::ROW_HEADER:
            return ATK_ROLE_ROW_HEADER;
        case accessibility::AccessibleRole::ROOT_PANE:
            return ATK_ROLE_ROOT_PANE;
        case accessibility::AccessibleRole::SCROLL_BAR:
            return ATK_ROLE_SCROLL_BAR;
        case accessibility::AccessibleRole::SCROLL_PANE:
            return ATK_ROLE_SCROLL_PANE;
        case accessibility::AccessibleRole::SHAPE:
            return ATK_ROLE_PANEL;
        case accessibility::AccessibleRole::SEPARATOR:
            return ATK_ROLE_SEPARATOR;
        case accessibility::AccessibleRole::SLIDER:
            return ATK_ROLE_SLIDER;
        case accessibility::AccessibleRole::SPIN_BOX:
            return ATK_ROLE_SPIN_BUTTON;
        case accessibility::AccessibleRole::SPLIT_PANE:
            return ATK_ROLE_SPLIT_PANE;
        case accessibility::AccessibleRole::STATUS_BAR:
            return ATK_ROLE_STATUSBAR;
        case accessibility::AccessibleRole::TABLE:
            return ATK_ROLE_TABLE;
        case accessibility::AccessibleRole::TABLE_CELL:
            return ATK_ROLE_TABLE_CELL;
        case accessibility::AccessibleRole::TEXT:
            return ATK_ROLE_TEXT;
        case accessibility::AccessibleRole::TEXT_FRAME:
            return ATK_ROLE_PANEL;
        case accessibility::AccessibleRole::TOGGLE_BUTTON:
            return ATK_ROLE_TOGGLE_BUTTON;
        case accessibility::AccessibleRole::TOOL_BAR:
            return ATK_ROLE_TOOL_BAR;
        case accessibility::AccessibleRole::TOOL_TIP:
            return ATK_ROLE_TOOL_TIP;
        case accessibility::AccessibleRole::TREE:
            return ATK_ROLE_TREE;
        case accessibility::AccessibleRole::VIEW_PORT:
            return ATK_ROLE_VIEWPORT;
        case accessibility::AccessibleRole::WINDOW:
            return ATK_ROLE_WINDOW;
        case accessibility::AccessibleRole::BUTTON_DROPDOWN:
        {
            if (nStates & css::accessibility::AccessibleStateType::CHECKABLE)
                return ATK_ROLE_TOGGLE_BUTTON;
            return ATK_ROLE_PUSH_BUTTON;
        }
        case accessibility::AccessibleRole::BUTTON_MENU:
#if ATK_CHECK_VERSION(2, 46, 0)
            return ATK_ROLE_PUSH_BUTTON_MENU;
#else
            return ATK_ROLE_PUSH_BUTTON;
#endif
        case accessibility::AccessibleRole::CAPTION:
            return ATK_ROLE_CAPTION;
        case accessibility::AccessibleRole::CHART:
            return ATK_ROLE_CHART;
        case accessibility::AccessibleRole::EDIT_BAR:
            return ATK_ROLE_EDITBAR;
        case accessibility::AccessibleRole::FORM:
            return ATK_ROLE_FORM;
        case accessibility::AccessibleRole::IMAGE_MAP:
            return ATK_ROLE_IMAGE_MAP;
        case accessibility::AccessibleRole::NOTE:
            return ATK_ROLE_COMMENT;
        case accessibility::AccessibleRole::PAGE:
            return ATK_ROLE_PAGE;
        case accessibility::AccessibleRole::RULER:
            return ATK_ROLE_RULER;
        case accessibility::AccessibleRole::SECTION:
            return ATK_ROLE_SECTION;
        case accessibility::AccessibleRole::TREE_ITEM:
            return ATK_ROLE_TREE_ITEM;
        case accessibility::AccessibleRole::TREE_TABLE:
            return ATK_ROLE_TREE_TABLE;
        case accessibility::AccessibleRole::COMMENT:
            return ATK_ROLE_COMMENT;
        case accessibility::AccessibleRole::COMMENT_END:
            return ATK_ROLE_UNKNOWN;
        case accessibility::AccessibleRole::DOCUMENT_PRESENTATION:
            return ATK_ROLE_DOCUMENT_PRESENTATION;
        case accessibility::AccessibleRole::DOCUMENT_SPREADSHEET:
            return ATK_ROLE_DOCUMENT_SPREADSHEET;
        case accessibility::AccessibleRole::DOCUMENT_TEXT:
            return ATK_ROLE_DOCUMENT_TEXT;
        case accessibility::AccessibleRole::STATIC:
            return ATK_ROLE_STATIC;
        case accessibility::AccessibleRole::NOTIFICATION:
            return ATK_ROLE_NOTIFICATION;
        default:
            SAL_WARN("vcl.gtk", "Unmapped accessible role: " << nRole);
            return ATK_ROLE_UNKNOWN;
    }
}

/*****************************************************************************/

extern "C" {

/*****************************************************************************/

static const gchar*
wrapper_get_name( AtkObject *atk_obj )
{
    AtkObjectWrapper *obj = ATK_OBJECT_WRAPPER (atk_obj);

    if( obj->mpContext.is() )
    {
        try {
            OString aName =
                OUStringToOString(
                    obj->mpContext->getAccessibleName(),
                    RTL_TEXTENCODING_UTF8);

            int nCmp = atk_obj->name ? rtl_str_compare( atk_obj->name, aName.getStr() ) : -1;
            if( nCmp != 0 )
            {
                if( atk_obj->name )
                    g_free(atk_obj->name);
                atk_obj->name = g_strdup(aName.getStr());

                return atk_obj->name;
            }
        }
        catch(const uno::Exception&) {
            g_warning( "Exception in getAccessibleName()" );
        }
    }

    return ATK_OBJECT_CLASS (parent_class)->get_name(atk_obj);
}

/*****************************************************************************/

static const gchar*
wrapper_get_description( AtkObject *atk_obj )
{
    AtkObjectWrapper *obj = ATK_OBJECT_WRAPPER (atk_obj);

    if( obj->mpContext.is() )
    {
        try {
            OString aDescription =
                OUStringToOString(
                    obj->mpContext->getAccessibleDescription(),
                    RTL_TEXTENCODING_UTF8);

            g_free(atk_obj->description);
            atk_obj->description = g_strdup(aDescription.getStr());

            return atk_obj->description;
        }
        catch(const uno::Exception&) {
            g_warning( "Exception in getAccessibleDescription()" );
        }
    }

    return ATK_OBJECT_CLASS (parent_class)->get_description(atk_obj);

}

/*****************************************************************************/

static AtkAttributeSet *
wrapper_get_attributes( AtkObject *atk_obj )
{
    AtkObjectWrapper *obj = ATK_OBJECT_WRAPPER( atk_obj );
    AtkAttributeSet *pSet = nullptr;

    try
    {
        uno::Reference< accessibility::XAccessibleExtendedAttributes >
            xExtendedAttrs( obj->mpContext, uno::UNO_QUERY );
        if( xExtendedAttrs.is() )
            pSet = attribute_set_new_from_extended_attributes( xExtendedAttrs );
    }
    catch(const uno::Exception&)
    {
        g_warning( "Exception in getAccessibleAttributes()" );
    }

    return pSet;
}

/*****************************************************************************/

static gint
wrapper_get_n_children( AtkObject *atk_obj )
{
    AtkObjectWrapper *obj = ATK_OBJECT_WRAPPER (atk_obj);

    if (obj->mpSysObjChild)
        return 1;

    gint n = 0;

    if( obj->mpContext.is() )
    {
        try {
            sal_Int64 nChildCount = obj->mpContext->getAccessibleChildCount();
            if (nChildCount > std::numeric_limits<gint>::max())
            {
                SAL_WARN("vcl.gtk", "wrapper_get_n_children: Child count exceeds maximum gint value, "
                                    "returning max gint.");
                nChildCount = std::numeric_limits<gint>::max();
            }
            n = nChildCount;
        }
        catch(const uno::Exception&) {
            TOOLS_WARN_EXCEPTION( "vcl", "Exception" );
        }
    }

    return n;
}

/*****************************************************************************/

static AtkObject *
wrapper_ref_child( AtkObject *atk_obj,
                   gint       i )
{
    SolarMutexGuard aGuard;

    AtkObjectWrapper *obj = ATK_OBJECT_WRAPPER (atk_obj);

    if (obj->mpSysObjChild)
    {
        g_object_ref(obj->mpSysObjChild);
        return obj->mpSysObjChild;
    }

    AtkObject* child = nullptr;

    // see comments above atk_object_wrapper_remove_child
    if( -1 < i && obj->index_of_child_about_to_be_removed == i )
    {
        g_object_ref( obj->child_about_to_be_removed );
        return obj->child_about_to_be_removed;
    }

    if( obj->mpContext.is() )
    {
        try {
            uno::Reference< accessibility::XAccessible > xAccessible =
                obj->mpContext->getAccessibleChild( i );

            child = atk_object_wrapper_ref( xAccessible );
        }
        catch(const uno::Exception&) {
            TOOLS_WARN_EXCEPTION( "vcl", "getAccessibleChild");
        }
    }

    return child;
}

/*****************************************************************************/

static gint
wrapper_get_index_in_parent( AtkObject *atk_obj )
{
    SolarMutexGuard aGuard;

    AtkObjectWrapper *obj = ATK_OBJECT_WRAPPER (atk_obj);

    //if we're a native GtkDrawingArea with custom a11y, use the default toolkit a11y
    if (obj->mpOrig)
        return atk_object_get_index_in_parent(obj->mpOrig);

    gint i = -1;

    if( obj->mpContext.is() )
    {
        try {
            sal_Int64 nIndex = obj->mpContext->getAccessibleIndexInParent();
            if (nIndex > std::numeric_limits<gint>::max())
            {
                // use -2 when the child index is too large to fit into 32 bit to neither use the
                // valid index of another child nor -1, which would e.g. make Orca interpret the
                // object as being a zombie
                SAL_WARN("vcl.gtk", "wrapper_get_index_in_parent: Child index exceeds maximum gint value, "
                                    "returning -2.");
                nIndex = -2;
            }
            i = nIndex;
        }
        catch(const uno::Exception&) {
            g_warning( "Exception in getAccessibleIndexInParent()" );
        }
    }
    return i;
}

/*****************************************************************************/

AtkRelation*
atk_object_wrapper_relation_new(const accessibility::AccessibleRelation& rRelation)
{
    sal_uInt32 nTargetCount = rRelation.TargetSet.getLength();

    std::vector<AtkObject*> aTargets;

    for (const auto& rTarget : rRelation.TargetSet)
    {
        uno::Reference< accessibility::XAccessible > xAccessible( rTarget, uno::UNO_QUERY );
        aTargets.push_back(atk_object_wrapper_ref(xAccessible));
    }

    AtkRelation *pRel =
        atk_relation_new(
            aTargets.data(), nTargetCount,
            mapRelationType( rRelation.RelationType )
        );

    return pRel;
}

static AtkRelationSet *
wrapper_ref_relation_set( AtkObject *atk_obj )
{
    AtkObjectWrapper *obj = ATK_OBJECT_WRAPPER (atk_obj);

    //if we're a native GtkDrawingArea with custom a11y, use the default toolkit relation set impl
    if (obj->mpOrig)
        return atk_object_ref_relation_set(obj->mpOrig);

    AtkRelationSet *pSet = atk_relation_set_new();

    if( obj->mpContext.is() )
    {
        try {
            uno::Reference< accessibility::XAccessibleRelationSet > xRelationSet(
                    obj->mpContext->getAccessibleRelationSet()
            );

            sal_Int32 nRelations = xRelationSet.is() ? xRelationSet->getRelationCount() : 0;
            for( sal_Int32 n = 0; n < nRelations; n++ )
            {
                AtkRelation *pRel = atk_object_wrapper_relation_new(xRelationSet->getRelation(n));
                atk_relation_set_add(pSet, pRel);
                g_object_unref(pRel);
            }
        }
        catch(const uno::Exception &) {
            g_object_unref( G_OBJECT( pSet ) );
            pSet = nullptr;
        }
    }

    return pSet;
}

static AtkStateSet *
wrapper_ref_state_set( AtkObject *atk_obj )
{
    AtkObjectWrapper *obj = ATK_OBJECT_WRAPPER (atk_obj);
    AtkStateSet *pSet = atk_state_set_new();

    if( obj->mpContext.is() )
    {
        try {
            sal_Int64 nStateSet = obj->mpContext->getAccessibleStateSet();

            if( nStateSet )
            {
                for (int i=0; i<63; ++i)
                {
                    // ATK_STATE_LAST_DEFINED is used to check if the state
                    // is unmapped, do not report it to Atk
                    sal_Int64 nState = sal_Int64(1) << i;
                    if ( (nStateSet & nState) && mapAtkState( nState ) != ATK_STATE_LAST_DEFINED )
                        atk_state_set_add_state( pSet, mapAtkState( nState ) );
                }

                // We need to emulate FOCUS state for menus, menu-items etc.
                if( atk_obj == atk_get_focus_object() )
                    atk_state_set_add_state( pSet, ATK_STATE_FOCUSED );
/* FIXME - should we do this ?
                else
                    atk_state_set_remove_state( pSet, ATK_STATE_FOCUSED );
*/
            }
        }

        catch(const uno::Exception &) {
            g_warning( "Exception in wrapper_ref_state_set" );
            atk_state_set_add_state( pSet, ATK_STATE_DEFUNCT );
        }
    }
    else
        atk_state_set_add_state( pSet, ATK_STATE_DEFUNCT );

    return pSet;
}

/*****************************************************************************/

static void
atk_object_wrapper_finalize (GObject *obj)
{
    AtkObjectWrapper *pWrap = ATK_OBJECT_WRAPPER (obj);

    if( pWrap->mpAccessible.is() )
    {
        ooo_wrapper_registry_remove( pWrap->mpAccessible );
        SolarMutexGuard aGuard;
        pWrap->mpAccessible.clear();
    }

    atk_object_wrapper_dispose( pWrap );

    parent_class->finalize( obj );
}

static void
atk_object_wrapper_class_init (gpointer klass_, gpointer)
{
  auto const klass = static_cast<AtkObjectWrapperClass *>(klass_);
  GObjectClass *gobject_class = G_OBJECT_CLASS( klass );
  AtkObjectClass *atk_class = ATK_OBJECT_CLASS( klass );

  parent_class = static_cast<GObjectClass *>(g_type_class_peek_parent (klass));

  // GObject methods
  gobject_class->finalize = atk_object_wrapper_finalize;

  // AtkObject methods
  atk_class->get_name = wrapper_get_name;
  atk_class->get_description = wrapper_get_description;
  atk_class->get_attributes = wrapper_get_attributes;
  atk_class->get_n_children = wrapper_get_n_children;
  atk_class->ref_child = wrapper_ref_child;
  atk_class->get_index_in_parent = wrapper_get_index_in_parent;
  atk_class->ref_relation_set = wrapper_ref_relation_set;
  atk_class->ref_state_set = wrapper_ref_state_set;

  AtkObjectClass* orig_atk_klass = static_cast<AtkObjectClass*>(g_type_class_ref(ATK_TYPE_OBJECT));
  // tdf#150496 we want to inherit from GtkAccessible because gtk assumes it can cast to GtkAccessible
  // but we want the original behaviour we got from atk_object_real_get_parent when we inherited
  // from AtkObject
  atk_class->get_parent = orig_atk_klass->get_parent;
  // and likewise for focus_event
  atk_class->focus_event = orig_atk_klass->focus_event;
  g_type_class_unref(orig_atk_klass);
}

static void
atk_object_wrapper_init (AtkObjectWrapper      *wrapper,
                         AtkObjectWrapperClass*)
{
   wrapper->mpAction = nullptr;
   wrapper->mpComponent = nullptr;
   wrapper->mpEditableText = nullptr;
   wrapper->mpHypertext = nullptr;
   wrapper->mpImage = nullptr;
   wrapper->mpSelection = nullptr;
   wrapper->mpTable = nullptr;
   wrapper->mpTableSelection = nullptr;
   wrapper->mpText = nullptr;
   wrapper->mpValue = nullptr;
}

} // extern "C"

GType
atk_object_wrapper_get_type()
{
  static GType type = 0;

  if (!type)
  {
      static const GTypeInfo typeInfo =
      {
        sizeof (AtkObjectWrapperClass),
        nullptr,
        nullptr,
        atk_object_wrapper_class_init,
        nullptr,
        nullptr,
        sizeof (AtkObjectWrapper),
        0,
        reinterpret_cast<GInstanceInitFunc>(atk_object_wrapper_init),
        nullptr
      } ;
      type = g_type_register_static (GTK_TYPE_WIDGET_ACCESSIBLE,
                                     "OOoAtkObj",
                                     &typeInfo, GTypeFlags(0)) ;
  }
  return type;
}

static bool
isOfType( uno::XInterface *pInterface, const uno::Type & rType )
{
    g_return_val_if_fail( pInterface != nullptr, false );

    bool bIs = false;
    try {
        uno::Any aRet = pInterface->queryInterface( rType );

        bIs = ( ( typelib_TypeClass_INTERFACE == aRet.pType->eTypeClass ) &&
                ( aRet.pReserved != nullptr ) );
    } catch( const uno::Exception &) { }

    return bIs;
}

// Whether AtkTableCell can be supported for the interface.
// Returns true if the corresponding XAccessible has role TABLE_CELL
// and an XAccessibleTable as parent.
static bool isTableCell(uno::XInterface* pInterface)
{
    g_return_val_if_fail(pInterface != nullptr, false);

    try {
        auto aType = cppu::UnoType<accessibility::XAccessible>::get().getTypeLibType();
        uno::Any aAcc = pInterface->queryInterface(aType);

        css::uno::Reference<css::accessibility::XAccessible> xAcc;
        aAcc >>= xAcc;
        if (!xAcc.is())
            return false;

        css::uno::Reference<css::accessibility::XAccessibleContext> xContext = xAcc->getAccessibleContext();
        if (!xContext.is() || !(xContext->getAccessibleRole() == accessibility::AccessibleRole::TABLE_CELL))
            return false;

        css::uno::Reference<css::accessibility::XAccessible> xParent = xContext->getAccessibleParent();
        if (!xParent.is())
            return false;
        css::uno::Reference<css::accessibility::XAccessibleContext> xParentContext = xParent->getAccessibleContext();
        if (!xParentContext.is())
            return false;

        css::uno::Reference<css::accessibility::XAccessibleTable> xTable(xParentContext, uno::UNO_QUERY);
        return xTable.is();
    }
    catch(const uno::Exception &)
    {
        g_warning("Exception in isTableCell()");
    }

    return false;
}

extern "C" {
typedef  GType (* GetGIfaceType ) ();
}
const struct {
        const char          *name;
        GInterfaceInitFunc const   aInit;
        GetGIfaceType const        aGetGIfaceType;
        const uno::Type &  (*aGetUnoType) ();
} aTypeTable[] = {
// re-location heaven:
    {
        "Comp", componentIfaceInit,
        atk_component_get_type,
        cppu::UnoType<accessibility::XAccessibleComponent>::get
    },
    {
        "Act",  actionIfaceInit,
        atk_action_get_type,
        cppu::UnoType<accessibility::XAccessibleAction>::get
    },
    {
        "Txt",  textIfaceInit,
        atk_text_get_type,
        cppu::UnoType<accessibility::XAccessibleText>::get
    },
    {
        "Val",  valueIfaceInit,
        atk_value_get_type,
        cppu::UnoType<accessibility::XAccessibleValue>::get
    },
    {
        "Tab",  tableIfaceInit,
        atk_table_get_type,
        cppu::UnoType<accessibility::XAccessibleTable>::get
    },
    {
        "Cell",  tablecellIfaceInit,
        atk_table_cell_get_type,
        // there is no UNO a11y interface for table cells, so this case is handled separately below
        nullptr
    },
    {
        "Edt",  editableTextIfaceInit,
        atk_editable_text_get_type,
        cppu::UnoType<accessibility::XAccessibleEditableText>::get
    },
    {
        "Img",  imageIfaceInit,
        atk_image_get_type,
        cppu::UnoType<accessibility::XAccessibleImage>::get
    },
    {
        "Hyp",  hypertextIfaceInit,
        atk_hypertext_get_type,
        cppu::UnoType<accessibility::XAccessibleHypertext>::get
    },
    {
        "Sel",  selectionIfaceInit,
        atk_selection_get_type,
        cppu::UnoType<accessibility::XAccessibleSelection>::get
    }
    // AtkDocument is a nastily broken interface (so far)
    //  we could impl. get_document_type perhaps though.
};

const int aTypeTableSize = G_N_ELEMENTS( aTypeTable );

static GType
ensureTypeFor( uno::XInterface *pAccessible )
{
    int i;
    bool bTypes[ aTypeTableSize ] = { false, };
    OStringBuffer aTypeNameBuf( "OOoAtkObj" );

    for( i = 0; i < aTypeTableSize; i++ )
    {
        if(!g_strcmp0(aTypeTable[i].name, "Cell"))
        {
            // there is no UNO interface for table cells, but AtkTableCell can be supported
            // for table cells via the methods of the parent that is a table
            if (isTableCell(pAccessible))
            {
                aTypeNameBuf.append(aTypeTable[i].name);
                bTypes[i] = true;
            }
        }
        else if (isOfType( pAccessible, aTypeTable[i].aGetUnoType() ) )
        {
            aTypeNameBuf.append(aTypeTable[i].name);
            bTypes[i] = true;
        }
    }

    OString aTypeName = aTypeNameBuf.makeStringAndClear();
    GType nType = g_type_from_name( aTypeName.getStr() );
    if( nType == G_TYPE_INVALID )
    {
        GTypeInfo aTypeInfo = {
            sizeof( AtkObjectWrapperClass ),
            nullptr, nullptr, nullptr, nullptr, nullptr,
            sizeof( AtkObjectWrapper ),
            0, nullptr, nullptr
        } ;
        nType = g_type_register_static( ATK_TYPE_OBJECT_WRAPPER,
                                        aTypeName.getStr(), &aTypeInfo,
                                        GTypeFlags(0) ) ;

        for( int j = 0; j < aTypeTableSize; j++ )
            if( bTypes[j] )
            {
                GInterfaceInfo aIfaceInfo = { nullptr, nullptr, nullptr };
                aIfaceInfo.interface_init = aTypeTable[j].aInit;
                g_type_add_interface_static (nType, aTypeTable[j].aGetGIfaceType(),
                                             &aIfaceInfo);
            }
    }
    return nType;
}

AtkObject *
atk_object_wrapper_ref( const uno::Reference< accessibility::XAccessible > &rxAccessible, bool create )
{
    g_return_val_if_fail( bool(rxAccessible), nullptr );

    AtkObject *obj = ooo_wrapper_registry_get(rxAccessible);
    if( obj )
    {
        g_object_ref( obj );
        return obj;
    }

    if( create )
        return atk_object_wrapper_new( rxAccessible );

    return nullptr;
}

AtkObject *
atk_object_wrapper_new( const css::uno::Reference< css::accessibility::XAccessible >& rxAccessible,
                        AtkObject* parent, AtkObject* orig )
{
    g_return_val_if_fail( bool(rxAccessible), nullptr );

    AtkObjectWrapper *pWrap = nullptr;

    try {
        uno::Reference< accessibility::XAccessibleContext > xContext(rxAccessible->getAccessibleContext());

        g_return_val_if_fail( bool(xContext), nullptr );

        GType nType = ensureTypeFor( xContext.get() );
        gpointer obj = g_object_new( nType, nullptr);

        pWrap = ATK_OBJECT_WRAPPER( obj );
        pWrap->mpAccessible = rxAccessible;

        pWrap->index_of_child_about_to_be_removed = -1;
        pWrap->child_about_to_be_removed = nullptr;

        pWrap->mpContext = xContext;
        pWrap->mpOrig = orig;

        AtkObject* atk_obj = ATK_OBJECT(pWrap);
        atk_obj->role = mapToAtkRole(xContext->getAccessibleRole(), xContext->getAccessibleStateSet());
        atk_obj->accessible_parent = parent;

        ooo_wrapper_registry_add( rxAccessible, atk_obj );

        if( parent )
            g_object_ref( atk_obj->accessible_parent );
        else
        {
            /* gail_focus_tracker remembers the focused object at the first
             * parent in the hierarchy that is a Gtk+ widget, but at the time the
             * event gets processed (at idle), it may be too late to create the
             * hierarchy, so doing it now ..
             */
            uno::Reference< accessibility::XAccessible > xParent( xContext->getAccessibleParent() );

            if( xParent.is() )
                atk_obj->accessible_parent = atk_object_wrapper_ref( xParent );
        }

        // Attach a listener to the UNO object if it's not TRANSIENT
        sal_Int64 nStateSet( xContext->getAccessibleStateSet() );
        if( ! (nStateSet & accessibility::AccessibleStateType::TRANSIENT ) )
        {
            uno::Reference< accessibility::XAccessibleEventBroadcaster > xBroadcaster(xContext, uno::UNO_QUERY);
            if( xBroadcaster.is() )
            {
                uno::Reference<accessibility::XAccessibleEventListener> xListener(new AtkListener(pWrap));
                xBroadcaster->addAccessibleEventListener(xListener);
            }
            else
                OSL_ASSERT( false );
        }

        static auto func = reinterpret_cast<void(*)(AtkObject*, const gchar*)>(dlsym(nullptr, "atk_object_set_accessible_id"));
        if (func)
        {
            css::uno::Reference<css::accessibility::XAccessibleContext2> xContext2(xContext, css::uno::UNO_QUERY);
            if( xContext2.is() )
            {
                OString aId = OUStringToOString( xContext2->getAccessibleId(), RTL_TEXTENCODING_UTF8);
                (*func)(atk_obj, aId.getStr());
            }
        }

        // tdf#141197 if we have a sysobj child then include that in the hierarchy
        if (UnoWrapperBase* pWrapper = UnoWrapperBase::GetUnoWrapper())
        {
            css::uno::Reference<css::awt::XWindow> xAWTWindow(rxAccessible, css::uno::UNO_QUERY);
            VclPtr<vcl::Window> xWindow = pWrapper->GetWindow(xAWTWindow);
            if (xWindow && xWindow->GetType() == WindowType::SYSTEMCHILDWINDOW)
            {
                const SystemEnvData* pEnvData = static_cast<SystemChildWindow*>(xWindow.get())->GetSystemData();
                if (GtkWidget *pSysObj = pEnvData ? static_cast<GtkWidget*>(pEnvData->pWidget) : nullptr)
                    pWrap->mpSysObjChild = gtk_widget_get_accessible(pSysObj);
            }
        }

        return ATK_OBJECT( pWrap );
    }
    catch (const uno::Exception &)
    {
        if( pWrap )
            g_object_unref( pWrap );

        return nullptr;
    }
}

/*****************************************************************************/

void atk_object_wrapper_add_child(AtkObjectWrapper* wrapper, AtkObject *child, gint index)
{
    AtkObject *atk_obj = ATK_OBJECT( wrapper );

    atk_object_set_parent( child, atk_obj );
    g_signal_emit_by_name( atk_obj, "children_changed::add", index, child, nullptr );
}

/*****************************************************************************/

void atk_object_wrapper_remove_child(AtkObjectWrapper* wrapper, AtkObject *child, gint index)
{
    /*
     * the atk-bridge GTK+ module gets back to the event source to ref the child just
     * vanishing, so we keep this reference because the semantic on OOo side is different.
     */
    wrapper->child_about_to_be_removed = child;
    wrapper->index_of_child_about_to_be_removed = index;

    g_signal_emit_by_name( ATK_OBJECT( wrapper ), "children_changed::remove", index, child, nullptr );

    wrapper->index_of_child_about_to_be_removed = -1;
    wrapper->child_about_to_be_removed = nullptr;
}

/*****************************************************************************/

void atk_object_wrapper_set_role(AtkObjectWrapper* wrapper, sal_Int16 role, sal_Int64 nStates)
{
    AtkObject *atk_obj = ATK_OBJECT( wrapper );
    atk_object_set_role(atk_obj, mapToAtkRole(role, nStates));
}

/*****************************************************************************/

void atk_object_wrapper_dispose(AtkObjectWrapper* wrapper)
{
    wrapper->mpContext.clear();
    wrapper->mpAction.clear();
    wrapper->mpComponent.clear();
    wrapper->mpEditableText.clear();
    wrapper->mpHypertext.clear();
    wrapper->mpImage.clear();
    wrapper->mpSelection.clear();
    wrapper->mpMultiLineText.clear();
    wrapper->mpTable.clear();
    wrapper->mpTableSelection.clear();
    wrapper->mpText.clear();
    wrapper->mpTextMarkup.clear();
    wrapper->mpTextAttributes.clear();
    wrapper->mpValue.clear();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
