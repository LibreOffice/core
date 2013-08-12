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
#include <com/sun/star/accessibility/XAccessibleTextMarkup.hpp>
#include <com/sun/star/accessibility/XAccessibleTextAttributes.hpp>
#include <com/sun/star/accessibility/XAccessibleValue.hpp>
#include <com/sun/star/accessibility/XAccessibleAction.hpp>
#include <com/sun/star/accessibility/XAccessibleContext.hpp>
#include <com/sun/star/accessibility/XAccessibleComponent.hpp>
#include <com/sun/star/accessibility/XAccessibleEventBroadcaster.hpp>
#include <com/sun/star/accessibility/XAccessibleMultiLineText.hpp>
#include <com/sun/star/accessibility/XAccessibleStateSet.hpp>
#include <com/sun/star/accessibility/XAccessibleRelationSet.hpp>
#include <com/sun/star/accessibility/XAccessibleTable.hpp>
#include <com/sun/star/accessibility/XAccessibleEditableText.hpp>
#include <com/sun/star/accessibility/XAccessibleImage.hpp>
#include <com/sun/star/accessibility/XAccessibleHyperlink.hpp>
#include <com/sun/star/accessibility/XAccessibleHypertext.hpp>
#include <com/sun/star/accessibility/XAccessibleSelection.hpp>
#include <com/sun/star/awt/XExtendedToolkit.hpp>
#include <com/sun/star/awt/XTopWindow.hpp>
#include <com/sun/star/awt/XTopWindowListener.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/beans/Property.hpp>

#include <rtl/ref.hxx>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/queryinterface.hxx>

#include "atkwrapper.hxx"
#include "atkregistry.hxx"
#include "atklistener.hxx"

#ifdef ENABLE_TRACING
#include <stdio.h>
#endif

#include <string.h>

using namespace ::com::sun::star;

static GObjectClass *parent_class = NULL;

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


AtkStateType mapAtkState( sal_Int16 nState )
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
        // a spelling error ...
        case accessibility::AccessibleStateType::DEFUNC:
            type = ATK_STATE_DEFUNCT; break;
        case accessibility::AccessibleStateType::MULTI_SELECTABLE:
            type = ATK_STATE_MULTISELECTABLE; break;
    default:
        break;
    }

    return type;
}

static inline AtkRole registerRole( const gchar * name )
{
    AtkRole ret = atk_role_for_name( name );
    if( ATK_ROLE_INVALID == ret )
        ret = atk_role_register( name );

    return ret;
}

static AtkRole mapToAtkRole( sal_Int16 nRole )
{
    AtkRole role = ATK_ROLE_UNKNOWN;

    static AtkRole roleMap[] = {
        ATK_ROLE_UNKNOWN,
        ATK_ROLE_ALERT,
        ATK_ROLE_COLUMN_HEADER,
        ATK_ROLE_CANVAS,
        ATK_ROLE_CHECK_BOX,
        ATK_ROLE_CHECK_MENU_ITEM,
        ATK_ROLE_COLOR_CHOOSER,
        ATK_ROLE_COMBO_BOX,
        ATK_ROLE_DATE_EDITOR,
        ATK_ROLE_DESKTOP_ICON,
        ATK_ROLE_DESKTOP_FRAME,   // ? pane
        ATK_ROLE_DIRECTORY_PANE,
        ATK_ROLE_DIALOG,
        ATK_ROLE_UNKNOWN,         // DOCUMENT - registered below
        ATK_ROLE_UNKNOWN,         // EMBEDDED_OBJECT - registered below
        ATK_ROLE_UNKNOWN,         // END_NOTE - registered below
        ATK_ROLE_FILE_CHOOSER,
        ATK_ROLE_FILLER,
        ATK_ROLE_FONT_CHOOSER,
        ATK_ROLE_FOOTER,
        ATK_ROLE_TEXT,            // FOOTNOTE - registered below
        ATK_ROLE_FRAME,
        ATK_ROLE_GLASS_PANE,
        ATK_ROLE_IMAGE,           // GRAPHIC
        ATK_ROLE_UNKNOWN,         // GROUP_BOX - registered below
        ATK_ROLE_HEADER,
        ATK_ROLE_PARAGRAPH,       // HEADING - registered below
        ATK_ROLE_TEXT,            // HYPER_LINK - registered below
        ATK_ROLE_ICON,
        ATK_ROLE_INTERNAL_FRAME,
        ATK_ROLE_LABEL,
        ATK_ROLE_LAYERED_PANE,
        ATK_ROLE_LIST,
        ATK_ROLE_LIST_ITEM,
        ATK_ROLE_MENU,
        ATK_ROLE_MENU_BAR,
        ATK_ROLE_MENU_ITEM,
        ATK_ROLE_OPTION_PANE,
        ATK_ROLE_PAGE_TAB,
        ATK_ROLE_PAGE_TAB_LIST,
        ATK_ROLE_PANEL,
        ATK_ROLE_PARAGRAPH,
        ATK_ROLE_PASSWORD_TEXT,
        ATK_ROLE_POPUP_MENU,
        ATK_ROLE_PUSH_BUTTON,
        ATK_ROLE_PROGRESS_BAR,
        ATK_ROLE_RADIO_BUTTON,
        ATK_ROLE_RADIO_MENU_ITEM,
        ATK_ROLE_ROW_HEADER,
        ATK_ROLE_ROOT_PANE,
        ATK_ROLE_SCROLL_BAR,
        ATK_ROLE_SCROLL_PANE,
        ATK_ROLE_UNKNOWN,        // SHAPE - registered below
        ATK_ROLE_SEPARATOR,
        ATK_ROLE_SLIDER,
        ATK_ROLE_SPIN_BUTTON,    // SPIN_BOX ?
        ATK_ROLE_SPLIT_PANE,
        ATK_ROLE_STATUSBAR,
        ATK_ROLE_TABLE,
        ATK_ROLE_TABLE_CELL,
        ATK_ROLE_TEXT,
        ATK_ROLE_INTERNAL_FRAME, // TEXT_FRAME - registered below
        ATK_ROLE_TOGGLE_BUTTON,
        ATK_ROLE_TOOL_BAR,
        ATK_ROLE_TOOL_TIP,
        ATK_ROLE_TREE,
        ATK_ROLE_VIEWPORT,
        ATK_ROLE_WINDOW,
        ATK_ROLE_PUSH_BUTTON,   // BUTTON_DROPDOWN
        ATK_ROLE_PUSH_BUTTON,   // BUTTON_MENU
        ATK_ROLE_UNKNOWN,       // CAPTION - registered below
        ATK_ROLE_UNKNOWN,       // CHART - registered below
        ATK_ROLE_UNKNOWN,       // EDIT_BAR - registered below
        ATK_ROLE_UNKNOWN,       // FORM - registered below
        ATK_ROLE_UNKNOWN,       // IMAGE_MAP - registered below
        ATK_ROLE_UNKNOWN,       // NOTE - registered below
        ATK_ROLE_UNKNOWN,       // PAGE - registered below
        ATK_ROLE_RULER,
        ATK_ROLE_UNKNOWN,       // SECTION - registered below
        ATK_ROLE_UNKNOWN,       // TREE_ITEM - registered below
        ATK_ROLE_TREE_TABLE,
        ATK_ROLE_SCROLL_PANE,   // COMMENT - mapped to atk_role_scroll_pane
        ATK_ROLE_UNKNOWN        // COMMENT_END - mapped to atk_role_unknown
    };

    static bool initialized = false;

    if( ! initialized )
    {
        // re-use strings from ATK library
        roleMap[accessibility::AccessibleRole::EDIT_BAR] = registerRole("edit bar");
        roleMap[accessibility::AccessibleRole::EMBEDDED_OBJECT] = registerRole("embedded component");
        roleMap[accessibility::AccessibleRole::CHART] = registerRole("chart");
        roleMap[accessibility::AccessibleRole::CAPTION] = registerRole("caption");
        roleMap[accessibility::AccessibleRole::DOCUMENT] = registerRole("document frame");
        roleMap[accessibility::AccessibleRole::HEADING] = registerRole("heading");
        roleMap[accessibility::AccessibleRole::PAGE] = registerRole("page");
        roleMap[accessibility::AccessibleRole::SECTION] = registerRole("section");
        roleMap[accessibility::AccessibleRole::FORM] = registerRole("form");

        // these don't exist in ATK yet
        roleMap[accessibility::AccessibleRole::END_NOTE] = registerRole("end note");
        roleMap[accessibility::AccessibleRole::FOOTNOTE] = registerRole("foot note");
        roleMap[accessibility::AccessibleRole::GROUP_BOX] = registerRole("group box");
        roleMap[accessibility::AccessibleRole::HYPER_LINK] = registerRole("hyper link");
        roleMap[accessibility::AccessibleRole::SHAPE] = registerRole("shape");
        roleMap[accessibility::AccessibleRole::TEXT_FRAME] = registerRole("text frame");
        roleMap[accessibility::AccessibleRole::IMAGE_MAP] = registerRole("image map");
        roleMap[accessibility::AccessibleRole::NOTE] = registerRole("note");
        roleMap[accessibility::AccessibleRole::TREE_ITEM] = registerRole("tree item");

        initialized = true;
    }

    static const sal_Int32 nMapSize = SAL_N_ELEMENTS(roleMap);
    if( 0 <= nRole &&  nMapSize > nRole )
        role = roleMap[nRole];

    return role;
}


/*****************************************************************************/

extern "C" {

/*****************************************************************************/

static G_CONST_RETURN gchar*
wrapper_get_name( AtkObject *atk_obj )
{
    AtkObjectWrapper *obj = ATK_OBJECT_WRAPPER (atk_obj);

    if( obj->mpContext )
    {
        uno::Reference< accessibility::XAccessibleContext > xContext(obj->mpContext);
        try {
            OString aName =
                OUStringToOString(
                    xContext->getAccessibleName(),
                    RTL_TEXTENCODING_UTF8);

            int nCmp = atk_obj->name ? rtl_str_compare( atk_obj->name, aName.getStr() ) : -1;
            if( nCmp != 0 )
            {
                if( atk_obj->name )
                    g_free(atk_obj->name);
                atk_obj->name = g_strdup(aName.getStr());
            }
        }
        catch(const uno::Exception& e) {
            g_warning( "Exception in getAccessibleName()" );
        }
    }

    return ATK_OBJECT_CLASS (parent_class)->get_name(atk_obj);
}

/*****************************************************************************/

static G_CONST_RETURN gchar*
wrapper_get_description( AtkObject *atk_obj )
{
    AtkObjectWrapper *obj = ATK_OBJECT_WRAPPER (atk_obj);

    if( obj->mpContext )
    {
        uno::Reference< accessibility::XAccessibleContext > xContext(obj->mpContext);
        try {
            OString aDescription =
                OUStringToOString(
                    xContext->getAccessibleDescription(),
                    RTL_TEXTENCODING_UTF8);

            g_free(atk_obj->description);
            atk_obj->description = g_strdup(aDescription.getStr());
        }
        catch(const uno::Exception& e) {
            g_warning( "Exception in getAccessibleDescription()" );
        }
    }

    return ATK_OBJECT_CLASS (parent_class)->get_description(atk_obj);

}

/*****************************************************************************/

static gint
wrapper_get_n_children( AtkObject *atk_obj )
{
    AtkObjectWrapper *obj = ATK_OBJECT_WRAPPER (atk_obj);
    gint n = 0;

    if( obj->mpContext )
    {
        uno::Reference< accessibility::XAccessibleContext > xContext(obj->mpContext);
        try {
            n = xContext->getAccessibleChildCount();
        }
        catch(const uno::Exception& e) {
            OSL_FAIL("Exception in getAccessibleChildCount()" );
        }
    }

    return n;
}

/*****************************************************************************/

static AtkObject *
wrapper_ref_child( AtkObject *atk_obj,
                   gint       i )
{
    AtkObjectWrapper *obj = ATK_OBJECT_WRAPPER (atk_obj);
    AtkObject* child = NULL;

    // see comments above atk_object_wrapper_remove_child
    if( -1 < i && obj->index_of_child_about_to_be_removed == i )
    {
        g_object_ref( obj->child_about_to_be_removed );
        return obj->child_about_to_be_removed;
    }

    if( obj->mpContext )
    {
        uno::Reference< accessibility::XAccessibleContext > xContext(obj->mpContext);
        try {
            uno::Reference< accessibility::XAccessible > xAccessible =
                xContext->getAccessibleChild( i );

            child = atk_object_wrapper_ref( xAccessible );
        }
        catch(const uno::Exception& e) {
            OSL_FAIL("Exception in getAccessibleChild");
        }
    }

    return child;
}

/*****************************************************************************/

static gint
wrapper_get_index_in_parent( AtkObject *atk_obj )
{
    AtkObjectWrapper *obj = ATK_OBJECT_WRAPPER (atk_obj);
    gint i = -1;

    if( obj->mpContext )
    {
        uno::Reference< accessibility::XAccessibleContext > xContext(obj->mpContext);
        try {
            i = xContext->getAccessibleIndexInParent();

#ifdef ENABLE_TRACING
            fprintf(stderr, "%p->getAccessibleIndexInParent() returned: %u\n",
                obj->mpAccessible, i);
#endif
        }
        catch(const uno::Exception& e) {
            g_warning( "Exception in getAccessibleIndexInParent()" );
        }
    }
    return i;
}

/*****************************************************************************/

static AtkRelationSet *
wrapper_ref_relation_set( AtkObject *atk_obj )
{
    AtkObjectWrapper *obj = ATK_OBJECT_WRAPPER (atk_obj);
    AtkRelationSet *pSet = atk_relation_set_new();

    if( obj->mpContext )
    {
        uno::Reference< accessibility::XAccessibleContext > xContext(obj->mpContext);
        try {
            uno::Reference< accessibility::XAccessibleRelationSet > xRelationSet(
                    xContext->getAccessibleRelationSet()
            );

            sal_Int32 nRelations = xRelationSet.is() ? xRelationSet->getRelationCount() : 0;
            for( sal_Int32 n = 0; n < nRelations; n++ )
            {
                accessibility::AccessibleRelation aRelation = xRelationSet->getRelation( n );
                sal_uInt32 nTargetCount = aRelation.TargetSet.getLength();
                AtkObject **pTargets = (AtkObject **) alloca( nTargetCount * sizeof(AtkObject *) );

                for( sal_uInt32 i = 0; i < nTargetCount; i++ )
                {
                    uno::Reference< accessibility::XAccessible > xAccessible(
                            aRelation.TargetSet[i], uno::UNO_QUERY );
                    pTargets[i] = atk_object_wrapper_ref( xAccessible );
                }

                AtkRelation *pRel =
                    atk_relation_new(
                        pTargets, nTargetCount,
                        mapRelationType( aRelation.RelationType )
                    );
                atk_relation_set_add( pSet, pRel );
                g_object_unref( G_OBJECT( pRel ) );
            }
        }
        catch(const uno::Exception &e) {
            g_object_unref( G_OBJECT( pSet ) );
            pSet = NULL;
        }
    }

    return pSet;
}

static AtkStateSet *
wrapper_ref_state_set( AtkObject *atk_obj )
{
    AtkObjectWrapper *obj = ATK_OBJECT_WRAPPER (atk_obj);
    AtkStateSet *pSet = atk_state_set_new();

    if( obj->mpContext )
    {
        uno::Reference< accessibility::XAccessibleContext > xContext(obj->mpContext);
        try {
            uno::Reference< accessibility::XAccessibleStateSet > xStateSet(
                xContext->getAccessibleStateSet());

            if( xStateSet.is() )
            {
                uno::Sequence< sal_Int16 > aStates = xStateSet->getStates();

                for( sal_Int32 n = 0; n < aStates.getLength(); n++ )
                    atk_state_set_add_state( pSet, mapAtkState( aStates[n] ) );

                // We need to emulate FOCUS state for menus, menu-items etc.
                if( atk_obj == atk_get_focus_object() )
                    atk_state_set_add_state( pSet, ATK_STATE_FOCUSED );
/* FIXME - should we do this ?
                else
                    atk_state_set_remove_state( pSet, ATK_STATE_FOCUSED );
*/
            }
        }

        catch(const uno::Exception &e) {
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

    if( pWrap->mpAccessible )
    {
        ooo_wrapper_registry_remove( pWrap->mpAccessible );
        pWrap->mpAccessible->release();
        pWrap->mpAccessible = NULL;
    }

    atk_object_wrapper_dispose( pWrap );

    parent_class->finalize( obj );
}

static void
atk_object_wrapper_class_init (AtkObjectWrapperClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS( klass );
  AtkObjectClass *atk_class = ATK_OBJECT_CLASS( klass );

  parent_class = (GObjectClass *) g_type_class_peek_parent (klass);

  // GObject methods
  gobject_class->finalize = atk_object_wrapper_finalize;

  // AtkObject methods
  atk_class->get_name = wrapper_get_name;
  atk_class->get_description = wrapper_get_description;
  atk_class->get_n_children = wrapper_get_n_children;
  atk_class->ref_child = wrapper_ref_child;
  atk_class->get_index_in_parent = wrapper_get_index_in_parent;
  atk_class->ref_relation_set = wrapper_ref_relation_set;
  atk_class->ref_state_set = wrapper_ref_state_set;
}

static void
atk_object_wrapper_init (AtkObjectWrapper      *wrapper,
                         AtkObjectWrapperClass)
{
   wrapper->mpAction = NULL;
   wrapper->mpComponent = NULL;
   wrapper->mpEditableText = NULL;
   wrapper->mpHypertext = NULL;
   wrapper->mpImage = NULL;
   wrapper->mpSelection = NULL;
   wrapper->mpTable = NULL;
   wrapper->mpText = NULL;
   wrapper->mpValue = NULL;
}

} // extern "C"

GType
atk_object_wrapper_get_type (void)
{
  static GType type = 0;

  if (!type)
    {
      static const GTypeInfo typeInfo =
      {
        sizeof (AtkObjectWrapperClass),
        (GBaseInitFunc) NULL,
        (GBaseFinalizeFunc) NULL,
        (GClassInitFunc) atk_object_wrapper_class_init,
        (GClassFinalizeFunc) NULL,
        NULL,
        sizeof (AtkObjectWrapper),
        0,
        (GInstanceInitFunc) atk_object_wrapper_init,
        NULL
      } ;
      type = g_type_register_static (ATK_TYPE_OBJECT,
                                     "OOoAtkObj",
                                     &typeInfo, (GTypeFlags)0) ;
    }
  return type;
}

static bool
isOfType( uno::XInterface *pInterface, const uno::Type & rType )
{
    g_return_val_if_fail( pInterface != NULL, false );

    bool bIs = false;
    try {
        uno::Any aRet = pInterface->queryInterface( rType );

        bIs = ( ( typelib_TypeClass_INTERFACE == aRet.pType->eTypeClass ) &&
                ( aRet.pReserved != NULL ) );
    } catch( const uno::Exception &e) { }

    return bIs;
}

extern "C" {
typedef  GType (* GetGIfaceType ) (void);
}
const struct {
        const char          *name;
        GInterfaceInitFunc   aInit;
        GetGIfaceType        aGetGIfaceType;
        const uno::Type &  (*aGetUnoType) (void *);
} aTypeTable[] = {
// re-location heaven:
    {
        "Comp", (GInterfaceInitFunc) componentIfaceInit,
        atk_component_get_type,
        accessibility::XAccessibleComponent::static_type
    },
    {
        "Act",  (GInterfaceInitFunc) actionIfaceInit,
        atk_action_get_type,
        accessibility::XAccessibleAction::static_type
    },
    {
        "Txt",  (GInterfaceInitFunc) textIfaceInit,
        atk_text_get_type,
        accessibility::XAccessibleText::static_type
    },
    {
        "Val",  (GInterfaceInitFunc) valueIfaceInit,
        atk_value_get_type,
        accessibility::XAccessibleValue::static_type
    },
    {
        "Tab",  (GInterfaceInitFunc) tableIfaceInit,
        atk_table_get_type,
        accessibility::XAccessibleTable::static_type
    },
    {
        "Edt",  (GInterfaceInitFunc) editableTextIfaceInit,
        atk_editable_text_get_type,
        accessibility::XAccessibleEditableText::static_type
    },
    {
        "Img",  (GInterfaceInitFunc) imageIfaceInit,
        atk_image_get_type,
        accessibility::XAccessibleImage::static_type
    },
    {
        "Hyp",  (GInterfaceInitFunc) hypertextIfaceInit,
        atk_hypertext_get_type,
        accessibility::XAccessibleHypertext::static_type
    },
    {
        "Sel",  (GInterfaceInitFunc) selectionIfaceInit,
        atk_selection_get_type,
        accessibility::XAccessibleSelection::static_type
    }
    // AtkDocument is a nastily broken interface (so far)
    //  we could impl. get_document_type perhaps though.
};

const int aTypeTableSize = G_N_ELEMENTS( aTypeTable );

static GType
ensureTypeFor( uno::XInterface *pAccessible )
{
    int i;
    int bTypes[ aTypeTableSize ] = { 0, };
    OString aTypeName( "OOoAtkObj" );

    for( i = 0; i < aTypeTableSize; i++ )
    {
        if( isOfType( pAccessible, aTypeTable[i].aGetUnoType(0) ) )
        {
            aTypeName += aTypeTable[i].name;
            bTypes[i] = TRUE;
        }
    }

    GType nType = g_type_from_name( aTypeName.getStr() );
    if( nType == G_TYPE_INVALID )
    {
        GTypeInfo aTypeInfo = {
            sizeof( AtkObjectWrapperClass ),
            NULL, NULL, NULL, NULL, NULL,
            sizeof( AtkObjectWrapper ),
            0, NULL, NULL
        } ;
        nType = g_type_register_static( ATK_TYPE_OBJECT_WRAPPER,
                                        aTypeName.getStr(), &aTypeInfo,
                                        (GTypeFlags)0 ) ;

        for( int j = 0; j < aTypeTableSize; j++ )
            if( bTypes[j] )
            {
                GInterfaceInfo aIfaceInfo = { NULL, NULL, NULL };
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
    g_return_val_if_fail( rxAccessible.get() != NULL, NULL );

    AtkObject *obj = ooo_wrapper_registry_get(rxAccessible);
    if( obj )
    {
        g_object_ref( obj );
        return obj;
    }

    if( create )
        return atk_object_wrapper_new( rxAccessible );

    return NULL;
}


AtkObject *
atk_object_wrapper_new( const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >& rxAccessible,
                        AtkObject* parent )
{
    g_return_val_if_fail( rxAccessible.get() != NULL, NULL );

    AtkObjectWrapper *pWrap = NULL;

    try {
        uno::Reference< accessibility::XAccessibleContext > xContext(rxAccessible->getAccessibleContext());

        g_return_val_if_fail( xContext.get() != NULL, NULL );

        GType nType = ensureTypeFor( xContext.get() );
        gpointer obj = g_object_new( nType, NULL);

        pWrap = ATK_OBJECT_WRAPPER( obj );
        pWrap->mpAccessible = rxAccessible.get();
        rxAccessible->acquire();

        pWrap->index_of_child_about_to_be_removed = -1;
        pWrap->child_about_to_be_removed = NULL;

        xContext->acquire();
        pWrap->mpContext = xContext.get();

        AtkObject* atk_obj = ATK_OBJECT(pWrap);
        atk_obj->role = mapToAtkRole( xContext->getAccessibleRole() );
        atk_obj->accessible_parent = parent;

        ooo_wrapper_registry_add( rxAccessible, atk_obj );

        if( parent )
            g_object_ref( atk_obj->accessible_parent );
        else
        {
            /* gail_focus_tracker remembers the focused object at the first
             * parent in the hierachy that is a Gtk+ widget, but at the time the
             * event gets processed (at idle), it may be too late to create the
             * hierachy, so doing it now ..
             */
            uno::Reference< accessibility::XAccessible > xParent( xContext->getAccessibleParent() );

            if( xParent.is() )
                atk_obj->accessible_parent = atk_object_wrapper_ref( xParent );
        }

        // Attach a listener to the UNO object if it's not TRANSIENT
        uno::Reference< accessibility::XAccessibleStateSet > xStateSet( xContext->getAccessibleStateSet() );
        if( xStateSet.is() && ! xStateSet->contains( accessibility::AccessibleStateType::TRANSIENT ) )
        {
            uno::Reference< accessibility::XAccessibleEventBroadcaster > xBroadcaster(xContext, uno::UNO_QUERY);
            if( xBroadcaster.is() )
                xBroadcaster->addAccessibleEventListener( static_cast< accessibility::XAccessibleEventListener * > ( new AtkListener(pWrap) ) );
        else
                OSL_ASSERT( false );
        }

        return ATK_OBJECT( pWrap );
    }
    catch (const uno::Exception &e)
    {
        if( pWrap )
            g_object_unref( pWrap );

        return NULL;
    }
}


/*****************************************************************************/

void atk_object_wrapper_add_child(AtkObjectWrapper* wrapper, AtkObject *child, gint index)
{
    AtkObject *atk_obj = ATK_OBJECT( wrapper );

    atk_object_set_parent( child, atk_obj );
    g_signal_emit_by_name( atk_obj, "children_changed::add", index, child, NULL );
}

/*****************************************************************************/

void atk_object_wrapper_remove_child(AtkObjectWrapper* wrapper, AtkObject *child, gint index)
{
    /*
     * the atk-bridge GTK+ module get's back to the event source to ref the child just
     * vanishing, so we keep this reference because the semantic on OOo side is different.
     */
    wrapper->child_about_to_be_removed = child;
    wrapper->index_of_child_about_to_be_removed = index;

    g_signal_emit_by_name( ATK_OBJECT( wrapper ), "children_changed::remove", index, child, NULL );

    wrapper->index_of_child_about_to_be_removed = -1;
    wrapper->child_about_to_be_removed = NULL;
}

/*****************************************************************************/

#define RELEASE(i) if( i ) { i->release(); i = NULL; }

void atk_object_wrapper_dispose(AtkObjectWrapper* wrapper)
{
    RELEASE( wrapper->mpContext )
    RELEASE( wrapper->mpAction )
    RELEASE( wrapper->mpComponent )
    RELEASE( wrapper->mpEditableText )
    RELEASE( wrapper->mpHypertext )
    RELEASE( wrapper->mpImage )
    RELEASE( wrapper->mpSelection )
    RELEASE( wrapper->mpMultiLineText )
    RELEASE( wrapper->mpTable )
    RELEASE( wrapper->mpText )
    RELEASE( wrapper->mpTextMarkup )
    RELEASE( wrapper->mpTextAttributes )
    RELEASE( wrapper->mpValue )
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
