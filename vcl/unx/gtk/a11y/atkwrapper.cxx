/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: atkwrapper.cxx,v $
 * $Revision: 1.8 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"

#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Type.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleRelation.hpp>
#include <com/sun/star/accessibility/AccessibleRelationType.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/accessibility/XAccessibleText.hpp>
#include <com/sun/star/accessibility/XAccessibleTextAttributes.hpp>
#include <com/sun/star/accessibility/XAccessibleValue.hpp>
#include <com/sun/star/accessibility/XAccessibleAction.hpp>
#include <com/sun/star/accessibility/XAccessibleContext.hpp>
#include <com/sun/star/accessibility/XAccessibleComponent.hpp>
#include <com/sun/star/accessibility/XAccessibleEventBroadcaster.hpp>
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

static G_CONST_RETURN gchar *
getAsConst( rtl::OUString rString )
{
    static const int nMax = 10;
    static rtl::OString aUgly[nMax];
    static int nIdx = 0;
    nIdx = (nIdx + 1) % nMax;
    aUgly[nIdx] = rtl::OUStringToOString( rString, RTL_TEXTENCODING_UTF8 );
    return aUgly[ nIdx ];
}

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

        default:
            break;
    }
#if 0
  ATK_RELATION_NODE_CHILD_OF,
  ATK_RELATION_EMBEDS,
  ATK_RELATION_EMBEDDED_BY,
  ATK_RELATION_POPUP_FOR,
#endif
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

static AtkRole mapToAtkRole( sal_Int16 nRole )
{
    AtkRole role = ATK_ROLE_UNKNOWN;

    switch( nRole )
    {
#define MAP(a,b) \
        case a: role = b; break

        MAP( ::accessibility::AccessibleRole::UNKNOWN, ATK_ROLE_UNKNOWN );
        MAP( ::accessibility::AccessibleRole::ALERT, ATK_ROLE_ALERT );
        MAP( ::accessibility::AccessibleRole::COLUMN_HEADER, ATK_ROLE_COLUMN_HEADER );
        MAP( ::accessibility::AccessibleRole::CANVAS, ATK_ROLE_CANVAS );
        MAP( ::accessibility::AccessibleRole::CHECK_BOX, ATK_ROLE_CHECK_BOX );
        MAP( ::accessibility::AccessibleRole::CHECK_MENU_ITEM, ATK_ROLE_CHECK_MENU_ITEM );
        MAP( ::accessibility::AccessibleRole::COLOR_CHOOSER, ATK_ROLE_COLOR_CHOOSER );
        MAP( ::accessibility::AccessibleRole::COMBO_BOX, ATK_ROLE_COMBO_BOX );
        MAP( ::accessibility::AccessibleRole::DATE_EDITOR, ATK_ROLE_DATE_EDITOR );
        MAP( ::accessibility::AccessibleRole::DESKTOP_ICON, ATK_ROLE_DESKTOP_ICON );
        MAP( ::accessibility::AccessibleRole::DESKTOP_PANE, ATK_ROLE_DESKTOP_FRAME ); // ? pane
        MAP( ::accessibility::AccessibleRole::DIRECTORY_PANE, ATK_ROLE_DIRECTORY_PANE );
        MAP( ::accessibility::AccessibleRole::DIALOG, ATK_ROLE_DIALOG );
        MAP( ::accessibility::AccessibleRole::DOCUMENT, ATK_ROLE_UNKNOWN ); // - FIXME -
        MAP( ::accessibility::AccessibleRole::EMBEDDED_OBJECT, ATK_ROLE_UNKNOWN ); // - FIXME -
        MAP( ::accessibility::AccessibleRole::END_NOTE, ATK_ROLE_UNKNOWN ); // - FIXME -
        MAP( ::accessibility::AccessibleRole::FILE_CHOOSER, ATK_ROLE_FILE_CHOOSER );
        MAP( ::accessibility::AccessibleRole::FILLER, ATK_ROLE_FILLER );
        MAP( ::accessibility::AccessibleRole::FONT_CHOOSER, ATK_ROLE_FONT_CHOOSER );
        MAP( ::accessibility::AccessibleRole::FOOTER, ATK_ROLE_FOOTER );
        MAP( ::accessibility::AccessibleRole::FOOTNOTE, ATK_ROLE_TEXT ); // - FIXME -
        MAP( ::accessibility::AccessibleRole::FRAME, ATK_ROLE_FRAME );
        MAP( ::accessibility::AccessibleRole::GLASS_PANE, ATK_ROLE_GLASS_PANE );
        MAP( ::accessibility::AccessibleRole::GRAPHIC, ATK_ROLE_UNKNOWN ); // - FIXME -
        MAP( ::accessibility::AccessibleRole::GROUP_BOX, ATK_ROLE_UNKNOWN ); // - FIXME -
        MAP( ::accessibility::AccessibleRole::HEADER, ATK_ROLE_HEADER );
        MAP( ::accessibility::AccessibleRole::HEADING, ATK_ROLE_HEADER ); // - FIXME -
        MAP( ::accessibility::AccessibleRole::HYPER_LINK, ATK_ROLE_TEXT ); // - FIXME -
        MAP( ::accessibility::AccessibleRole::ICON, ATK_ROLE_ICON );
        MAP( ::accessibility::AccessibleRole::INTERNAL_FRAME, ATK_ROLE_INTERNAL_FRAME );
        MAP( ::accessibility::AccessibleRole::LABEL, ATK_ROLE_LABEL );
        MAP( ::accessibility::AccessibleRole::LAYERED_PANE, ATK_ROLE_LAYERED_PANE );
        MAP( ::accessibility::AccessibleRole::LIST, ATK_ROLE_LIST );
        MAP( ::accessibility::AccessibleRole::LIST_ITEM, ATK_ROLE_LIST_ITEM );
        MAP( ::accessibility::AccessibleRole::MENU, ATK_ROLE_MENU );
        MAP( ::accessibility::AccessibleRole::MENU_BAR, ATK_ROLE_MENU_BAR );
        MAP( ::accessibility::AccessibleRole::MENU_ITEM, ATK_ROLE_MENU_ITEM );
        MAP( ::accessibility::AccessibleRole::OPTION_PANE, ATK_ROLE_OPTION_PANE );
        MAP( ::accessibility::AccessibleRole::PAGE_TAB, ATK_ROLE_PAGE_TAB );
        MAP( ::accessibility::AccessibleRole::PAGE_TAB_LIST, ATK_ROLE_PAGE_TAB_LIST );
        MAP( ::accessibility::AccessibleRole::PANEL, ATK_ROLE_PANEL );
        MAP( ::accessibility::AccessibleRole::PARAGRAPH, ATK_ROLE_PARAGRAPH );
        MAP( ::accessibility::AccessibleRole::PASSWORD_TEXT, ATK_ROLE_PASSWORD_TEXT );
        MAP( ::accessibility::AccessibleRole::POPUP_MENU, ATK_ROLE_POPUP_MENU );
        MAP( ::accessibility::AccessibleRole::PUSH_BUTTON, ATK_ROLE_PUSH_BUTTON );
        MAP( ::accessibility::AccessibleRole::PROGRESS_BAR, ATK_ROLE_PROGRESS_BAR );
        MAP( ::accessibility::AccessibleRole::RADIO_BUTTON, ATK_ROLE_RADIO_BUTTON );
        MAP( ::accessibility::AccessibleRole::RADIO_MENU_ITEM, ATK_ROLE_RADIO_MENU_ITEM );
        MAP( ::accessibility::AccessibleRole::ROW_HEADER, ATK_ROLE_ROW_HEADER );
        MAP( ::accessibility::AccessibleRole::ROOT_PANE, ATK_ROLE_ROOT_PANE );
        MAP( ::accessibility::AccessibleRole::SCROLL_BAR, ATK_ROLE_SCROLL_BAR );
        MAP( ::accessibility::AccessibleRole::SCROLL_PANE, ATK_ROLE_SCROLL_PANE );
        MAP( ::accessibility::AccessibleRole::SHAPE, ATK_ROLE_UNKNOWN ); // - FIXME -
        MAP( ::accessibility::AccessibleRole::SEPARATOR, ATK_ROLE_SEPARATOR );
        MAP( ::accessibility::AccessibleRole::SLIDER, ATK_ROLE_SLIDER );
        MAP( ::accessibility::AccessibleRole::SPIN_BOX, ATK_ROLE_SPIN_BUTTON ); // ?
        MAP( ::accessibility::AccessibleRole::SPLIT_PANE, ATK_ROLE_SPLIT_PANE );
        MAP( ::accessibility::AccessibleRole::STATUS_BAR, ATK_ROLE_STATUSBAR );
        MAP( ::accessibility::AccessibleRole::TABLE, ATK_ROLE_TABLE );
        MAP( ::accessibility::AccessibleRole::TABLE_CELL, ATK_ROLE_TABLE_CELL );
        MAP( ::accessibility::AccessibleRole::TEXT, ATK_ROLE_TEXT );
        MAP( ::accessibility::AccessibleRole::TEXT_FRAME, ATK_ROLE_UNKNOWN ); // - FIXME -
        MAP( ::accessibility::AccessibleRole::TOGGLE_BUTTON, ATK_ROLE_TOGGLE_BUTTON );
        MAP( ::accessibility::AccessibleRole::TOOL_BAR, ATK_ROLE_TOOL_BAR );
        MAP( ::accessibility::AccessibleRole::TOOL_TIP, ATK_ROLE_TOOL_TIP );
        MAP( ::accessibility::AccessibleRole::TREE, ATK_ROLE_TREE );
        MAP( ::accessibility::AccessibleRole::VIEW_PORT, ATK_ROLE_VIEWPORT );
        MAP( ::accessibility::AccessibleRole::WINDOW, ATK_ROLE_WINDOW );
#undef MAP
        default:
            break;
    }
/* Roles unimplemented by OO.o / a11y:
  ATK_ROLE_INVALID,      ATK_ROLE_ACCEL_LABEL,    ATK_ROLE_ANIMATION,
  ATK_ROLE_ARROW,        ATK_ROLE_CALENDAR,       ATK_ROLE_DIAL,
  ATK_ROLE_DRAWING_AREA, ATK_ROLE_HTML_CONTAINER, ATK_ROLE_IMAGE,
  ATK_ROLE_TABLE_COLUMN_HEADER, ATK_ROLE_TABLE_ROW_HEADER, ATK_ROLE_TEAR_OFF_MENU_ITEM,
  ATK_ROLE_TERMINAL,            ATK_ROLE_TREE_TABLE,       ATK_ROLE_RULER,
  ATK_ROLE_APPLICATION,         ATK_ROLE_AUTOCOMPLETE,     ATK_ROLE_EDITBAR */

    return role;
}


static accessibility::XAccessibleContext*
        getContext( AtkObject* obj ) throw (uno::RuntimeException)
{
    AtkObjectWrapper *pWrap = ATK_OBJECT_WRAPPER( obj );

    if( pWrap )
    {
        if( !pWrap->mpContext && pWrap->mpContext )
        {
            uno::Any any = pWrap->mpContext->queryInterface( accessibility::XAccessibleContext::static_type(NULL) );
            pWrap->mpContext = reinterpret_cast< accessibility::XAccessibleContext * > (any.pReserved);
            pWrap->mpContext->acquire();
        }

        return pWrap->mpContext;
    }

    return NULL;
}

/*****************************************************************************/

extern "C" {

/*****************************************************************************/

static G_CONST_RETURN gchar*
wrapper_get_name( AtkObject *pObject )
{
    G_CONST_RETURN gchar* name;

    name = ATK_OBJECT_CLASS (parent_class)->get_name( pObject );
    if (name == NULL)
    {
        try {
            accessibility::XAccessibleContext* pContext = getContext( pObject );
            if( pContext )
                name = getAsConst( pContext->getAccessibleName() );
        }
        catch(const uno::Exception& e) {
            g_warning( "Exception in getAccessibleName()" );
        }
    }

    return name;
}

/*****************************************************************************/

static G_CONST_RETURN gchar*
wrapper_get_description( AtkObject *pObject )
{
    G_CONST_RETURN gchar* desc;

    desc = ATK_OBJECT_CLASS (parent_class)->get_description( pObject );
    if (desc == NULL)
    {
        try {
            accessibility::XAccessibleContext* pContext = getContext( pObject );
            if( pContext )
                desc = getAsConst( pContext->getAccessibleDescription() );
        }
        catch(const uno::Exception& e) {
            g_warning( "Exception in getAccessibleDescription()" );
        }
    }

    return desc;
}

/*****************************************************************************/

static gint
wrapper_get_n_children( AtkObject *pObject )
{
    try {
        accessibility::XAccessibleContext* pContext = getContext( pObject );
        if( pContext )
            return pContext->getAccessibleChildCount();
    }
    catch(const uno::Exception& e) {
        g_warning( "Exception in getAccessibleChildCount()" );
    }

    return 0;
}

/*****************************************************************************/

static AtkObject *
wrapper_ref_child( AtkObject *pObject,
                   gint       i )
{
    try {
        accessibility::XAccessibleContext* pContext = getContext( pObject );
        if( pContext )
        {
//    fprintf(stderr, "wrapper_ref_child %d of (%d)\n", i,
//        getContext( pObject )->getAccessibleChildCount());
            if( i >= 0 && i <  getContext( pObject )->getAccessibleChildCount())
            {
                uno::Reference< accessibility::XAccessible > xAccessible =
                    pContext->getAccessibleChild( i );

                AtkObject* child = atk_object_wrapper_ref( xAccessible );
                return child;
            }
        }
    }
    catch(const uno::Exception& e) {
        g_warning( "Exception in getAccessibleChildCount()" );
    }

    return NULL;
}

static gint
wrapper_get_index_in_parent( AtkObject *pObject )
{
    try {
        accessibility::XAccessibleContext* pContext = getContext( pObject );

#ifdef ENABLE_TRACING
        if( pContext )
            fprintf(stderr, "%p->getAccessibleIndexInParent() returned: %u\n",
                ATK_OBJECT_WRAPPER( pObject )->mpAccessible,
                pContext->getAccessibleIndexInParent());
#endif
        if( pContext )
            return pContext->getAccessibleIndexInParent();
    }
    catch(const uno::Exception& e) {
        g_warning( "Exception in getAccessibleIndexInParent()" );
    }
    return -1;
}

static AtkRelationSet *
wrapper_ref_relation_set( AtkObject *pObject )
{
    AtkRelationSet *pSet = atk_relation_set_new();
    try {
        accessibility::XAccessibleContext* pContext = getContext( pObject );
        if( pContext )
        {
            uno::Reference< accessibility::XAccessibleRelationSet > xRelationSet;

            xRelationSet = pContext->getAccessibleRelationSet();

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

                AtkRelation *pRel = atk_relation_new
                    ( pTargets, nTargetCount,
                          mapRelationType( aRelation.RelationType ) );
                atk_relation_set_add( pSet, pRel );
                g_object_unref( G_OBJECT( pRel ) );
            }
        }
    }
    catch(const uno::Exception &e)
    {
        g_object_unref( G_OBJECT( pSet ) );
        pSet = NULL;
    }

    return pSet;
}

static AtkStateSet *
wrapper_ref_state_set( AtkObject *pObject )
{
    AtkStateSet *pSet = atk_state_set_new();
    try {
        accessibility::XAccessibleContext* pContext = getContext( pObject );
        if( pContext )
        {
            uno::Reference< accessibility::XAccessibleStateSet > xStateSet =
                pContext->getAccessibleStateSet();

            if( xStateSet.is() )
            {
                uno::Sequence< sal_Int16 > aStates = xStateSet->getStates();

                for( sal_Int32 n = 0; n < aStates.getLength(); n++ )
                    atk_state_set_add_state( pSet, mapAtkState( aStates[n] ) );

                // We need to emulate FOCUS state for menus, menu-items etc.
                if( pObject == atk_get_focus_object() )
                    atk_state_set_add_state( pSet, ATK_STATE_FOCUSED );
/* FIXME - should we do this ?
                else
                    atk_state_set_remove_state( pSet, ATK_STATE_FOCUSED );
*/
            }
        }
    }

    catch(const uno::Exception &e)
    {
        g_warning( "Exception in wrapper_ref_state_set" );
        atk_state_set_add_state( pSet, ATK_STATE_DEFUNCT );
    }

    return pSet;
}

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
    rtl::OString aTypeName( "OOoAtkObj" );

    for( i = 0; i < aTypeTableSize; i++ )
    {
        if( isOfType( pAccessible, aTypeTable[i].aGetUnoType(0) ) )
        {
            aTypeName += aTypeTable[i].name;
            bTypes[i] = TRUE;
        }
//      g_message( "Accessible %p has type '%s' (%d)",
//                 pAccessible, aTypeTable[i].name, bTypes[i] );
    }

    GType nType = g_type_from_name( aTypeName );
    if( nType == G_TYPE_INVALID )
    {
        GTypeInfo aTypeInfo = {
            sizeof( AtkObjectWrapperClass ),
            NULL, NULL, NULL, NULL, NULL,
            sizeof( AtkObjectWrapper ),
            0, NULL, NULL
        } ;
        nType = g_type_register_static( ATK_TYPE_OBJECT_WRAPPER,
                                        aTypeName, &aTypeInfo, (GTypeFlags)0 ) ;

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

            /* The top-level objects should never be of this class */
            OSL_ASSERT( xParent.is() );

            if( xParent.is() )
                atk_obj->accessible_parent = atk_object_wrapper_ref( xParent );
        }

        // Attach a listener to the UNO object if it's not TRANSIENT
        uno::Reference< accessibility::XAccessibleStateSet > xStateSet( xContext->getAccessibleStateSet() );
        if( xStateSet.is() && ! xStateSet->contains( accessibility::AccessibleStateType::TRANSIENT ) )
        {
            uno::Reference< accessibility::XAccessibleEventBroadcaster > xBroadcaster(xContext, uno::UNO_QUERY);
            if( xBroadcaster.is() )
                xBroadcaster->addEventListener( static_cast< accessibility::XAccessibleEventListener * > ( new AtkListener(pWrap) ) );
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
    RELEASE( wrapper->mpTable )
    RELEASE( wrapper->mpText )
    RELEASE( wrapper->mpTextAttributes )
    RELEASE( wrapper->mpValue )
}
