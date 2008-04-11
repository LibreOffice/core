/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: atklistener.cxx,v $
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

#include <com/sun/star/accessibility/TextSegment.hpp>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/AccessibleTableModelChange.hpp>
#include <com/sun/star/accessibility/AccessibleTableModelChangeType.hpp>
#include <com/sun/star/accessibility/XAccessibleEventBroadcaster.hpp>

#include "atklistener.hxx"
#include "atkwrapper.hxx"

#include <rtl/ref.hxx>
#include <stdio.h>

using namespace com::sun::star;


#define CSTRING_FROM_ANY(i) rtl::OUStringToOString( i.get< rtl::OUString >(), RTL_TEXTENCODING_UTF8 ).getStr()

AtkListener::AtkListener( AtkObjectWrapper* pWrapper )
{
    if( pWrapper )
    {
        mpAccessible = ATK_OBJECT( g_object_ref( pWrapper ) );
        updateChildList( pWrapper->mpContext );
    }
}

AtkListener::~AtkListener()
{
}

/*****************************************************************************/

AtkStateType mapState( const uno::Any &rAny )
{
    sal_Int16 nState = accessibility::AccessibleStateType::INVALID;
    rAny >>= nState;
    return mapAtkState( nState );
}

/*****************************************************************************/

// XEventListener implementation
void AtkListener::disposing( const lang::EventObject& ) throw (uno::RuntimeException)
{
    if( mpAccessible )
    {
        // Release all interface references to avoid shutdown problems with
        // global mutex
        atk_object_wrapper_dispose( ATK_OBJECT_WRAPPER( mpAccessible ) );

        // This is an equivalent to a state change to DEFUNC(T).
        atk_object_notify_state_change( mpAccessible, ATK_STATE_DEFUNCT, TRUE );

        if( atk_get_focus_object() == mpAccessible )
            atk_focus_tracker_notify( NULL );

        // Release the wrapper object so that it can vanish ..
        g_object_unref( mpAccessible );
        mpAccessible = NULL;
    }
}

/*****************************************************************************/

static AtkObject *getObjFromAny( const uno::Any &rAny )
{
    uno::Reference< accessibility::XAccessible > xAccessible;
    rAny >>= xAccessible;
    return xAccessible.is() ? atk_object_wrapper_ref( xAccessible ) : NULL;
}

/*****************************************************************************/

// Updates the child list held to provide the old IndexInParent on children_changed::remove
void AtkListener::updateChildList(accessibility::XAccessibleContext* pContext)
{
     m_aChildList.clear();

     uno::Reference< accessibility::XAccessibleStateSet > xStateSet = pContext->getAccessibleStateSet();
     if( xStateSet.is()
         && !xStateSet->contains(accessibility::AccessibleStateType::DEFUNC)
         && !xStateSet->contains(accessibility::AccessibleStateType::MANAGES_DESCENDANTS) )
     {
         sal_Int32 nChildren = pContext->getAccessibleChildCount();
         m_aChildList.resize(nChildren);
         for(sal_Int32 n = 0; n < nChildren; n++)
         {
             m_aChildList[n] = pContext->getAccessibleChild(n);
             OSL_ASSERT(m_aChildList[n].is());
         }
     }
}

/*****************************************************************************/

void AtkListener::handleChildAdded(
    const uno::Reference< accessibility::XAccessibleContext >& rxParent,
    const uno::Reference< accessibility::XAccessible>& rxAccessible)
{
    AtkObject * pChild = atk_object_wrapper_ref( rxAccessible );

    if( pChild )
    {
        atk_object_set_parent( pChild, mpAccessible );
        updateChildList(rxParent.get());
        g_signal_emit_by_name( mpAccessible, "children_changed::add",
            atk_object_get_index_in_parent( pChild ), pChild, NULL );
        g_object_unref( pChild );
    }
}

/*****************************************************************************/

void AtkListener::handleChildRemoved(
    const uno::Reference< accessibility::XAccessibleContext >& rxParent,
    const uno::Reference< accessibility::XAccessible>& rxChild)
{
    sal_Int32 nIndex = -1;

    // Locate the child in the children list
    size_t n, nmax = m_aChildList.size();
    for( n = 0; n < nmax; ++n )
    {
        if( rxChild == m_aChildList[n] )
        {
            nIndex = n;
            break;
        }
    }

    // FIXME: two problems here:
    // a) we get child-removed events for objects that are no real childs
    //    in the accessibility hierarchy
    // b) spi_atk_bridge_signal_listener ignores the given parameters
    //    for children_changed events and always asks the parent for the
    //    0. child, which breaks somehow on vanishing list boxes.
    // Ignoring "remove" events for objects not in the m_aChildList
    // for now.
    if( nIndex >= 0 )
    {
        updateChildList(rxParent.get());

        AtkObject * pChild = atk_object_wrapper_ref( rxChild, false );
        if( pChild )
        {
            atk_object_set_parent( pChild, atk_get_root() );
            g_signal_emit_by_name( mpAccessible, "children_changed::remove", nIndex, pChild, NULL );
            g_object_unref( pChild );
        }
    }
}

/*****************************************************************************/

void AtkListener::handleInvalidateChildren(
    const uno::Reference< accessibility::XAccessibleContext >& rxParent)
{
    // Send notifications for all previous children
    size_t n = m_aChildList.size();
    while( n-- > 0 )
    {
        if( m_aChildList[n].is() )
        {
            AtkObject * pChild = atk_object_wrapper_ref( m_aChildList[n], false );
            if( pChild )
            {
                atk_object_set_parent( pChild, atk_get_root() );
                g_signal_emit_by_name( mpAccessible, "children_changed::remove", n, pChild, NULL );
                g_object_unref( pChild );
            }
        }
    }

    updateChildList(rxParent.get());

    // Send notifications for all new children
    size_t nmax = m_aChildList.size();
    for( n = 0; n < nmax; ++n )
    {
        if( m_aChildList[n].is() )
        {
            AtkObject * pChild = atk_object_wrapper_ref( m_aChildList[n] );

            if( pChild )
            {
                atk_object_set_parent( pChild, mpAccessible );
                g_signal_emit_by_name( mpAccessible, "children_changed::add", n, pChild, NULL );
                g_object_unref( pChild );
            }
        }
    }
}

/*****************************************************************************/

static uno::Reference< accessibility::XAccessibleContext >
getAccessibleContextFromSource( const uno::Reference< uno::XInterface >& rxSource )
{
    uno::Reference< accessibility::XAccessibleContext > xContext(rxSource, uno::UNO_QUERY);
    if( ! xContext.is() )
    {
         g_warning( "ERROR: Event source does not implement XAccessibleContext" );

         // Second try - query for XAccessible, which should give us access to
         // XAccessibleContext.
         uno::Reference< accessibility::XAccessible > xAccessible(rxSource, uno::UNO_QUERY);
         if( xAccessible.is() )
             xContext = xAccessible->getAccessibleContext();
    }

    return xContext;
}

/*****************************************************************************/

// XAccessibleEventListener
void AtkListener::notifyEvent( const accessibility::AccessibleEventObject& aEvent ) throw( uno::RuntimeException )
{
    if( !mpAccessible )
        return;

    switch( aEvent.EventId )
    {
    // AtkObject signals:
        // Hierarchy signals
        case accessibility::AccessibleEventId::CHILD:
        {
            uno::Reference< accessibility::XAccessibleContext > xParent;
            uno::Reference< accessibility::XAccessible > xChild;

            xParent = getAccessibleContextFromSource(aEvent.Source);
            g_return_if_fail( xParent.is() );

            if( aEvent.OldValue >>= xChild )
                handleChildRemoved(xParent, xChild);

            if( aEvent.NewValue >>= xChild )
                handleChildAdded(xParent, xChild);
        }
            break;

        case accessibility::AccessibleEventId::INVALIDATE_ALL_CHILDREN:
        {
            uno::Reference< accessibility::XAccessibleContext > xParent;

            xParent = getAccessibleContextFromSource(aEvent.Source);
            g_return_if_fail( xParent.is() );

            handleInvalidateChildren(xParent);
        }
            break;

        case accessibility::AccessibleEventId::NAME_CHANGED:
            g_object_notify( G_OBJECT( mpAccessible ), "accessible-name" );
            break;

        case accessibility::AccessibleEventId::DESCRIPTION_CHANGED:
            g_object_notify( G_OBJECT( mpAccessible ), "accessible-description" );
            break;

        case accessibility::AccessibleEventId::STATE_CHANGED:
        {
            AtkStateType eOldState = mapState( aEvent.OldValue );
            AtkStateType eNewState = mapState( aEvent.NewValue );

            gboolean bState = eNewState != ATK_STATE_INVALID;
            AtkStateType eRealState = bState ? eNewState : eOldState;

            atk_object_notify_state_change( mpAccessible, eRealState, bState );
            break;
        }

        case accessibility::AccessibleEventId::BOUNDRECT_CHANGED:

#ifdef HAS_ATKRECTANGLE
            if( ATK_IS_COMPONENT( mpAccessible ) )
            {
                AtkRectangle rect;

                atk_component_get_extents( ATK_COMPONENT( mpAccessible ),
                                           &rect.x,
                                           &rect.y,
                                           &rect.width,
                                           &rect.height,
                                           ATK_XY_SCREEN );

                g_signal_emit_by_name( mpAccessible, "bounds_changed", &rect );
            }
            else
                g_warning( "bounds_changed event for object not implementing AtkComponent\n");
#endif

            break;

        case accessibility::AccessibleEventId::VISIBLE_DATA_CHANGED:
            g_signal_emit_by_name( mpAccessible, "visible-data-changed" );
            break;

        case accessibility::AccessibleEventId::ACTIVE_DESCENDANT_CHANGED:
        {
            AtkObject *pChild = getObjFromAny( aEvent.NewValue );
            if( pChild )
            {
                g_signal_emit_by_name( mpAccessible, "active-descendant-changed", pChild );
                g_object_unref( pChild );
            }
            break;
        }

        // AtkAction signals ...
        case accessibility::AccessibleEventId::ACTION_CHANGED:
            g_signal_emit_by_name( G_OBJECT( mpAccessible ), "property_change::accessible-actions");
            break;

        // AtkText
        case accessibility::AccessibleEventId::CARET_CHANGED:
        {
            sal_Int32 nPos=0;
            aEvent.NewValue >>= nPos;
            g_signal_emit_by_name( mpAccessible, "text_caret_moved", nPos );
            break;
        }
        case accessibility::AccessibleEventId::TEXT_CHANGED:
        {
            // TESTME: and remove this comment:
            // cf. comphelper/source/misc/accessibletexthelper.cxx (implInitTextChangedEvent)
            accessibility::TextSegment aDeletedText;
            accessibility::TextSegment aInsertedText;

            // TODO: when GNOME starts to send "update" kind of events, change
            // we need to re-think this implementation as well
            if( aEvent.OldValue >>= aDeletedText )
            {
                /* Remember the text segment here to be able to return removed text in get_text().
                 * This is clearly a hack to be used until appropriate API exists in atk to pass
                 * the string value directly or we find a compelling reason to start caching the
                 * UTF-8 converted strings in the atk wrapper object.
                 */

                g_object_set_data( G_OBJECT(mpAccessible), "ooo::text_changed::delete", &aDeletedText);

                g_signal_emit_by_name( mpAccessible, "text_changed::delete",
                                       (gint) aDeletedText.SegmentStart,
                                       (gint)( aDeletedText.SegmentEnd - aDeletedText.SegmentStart ) );

                g_object_steal_data( G_OBJECT(mpAccessible), "ooo::text_changed::delete" );
            }

            if( aEvent.NewValue >>= aInsertedText )
                g_signal_emit_by_name( mpAccessible, "text_changed::insert",
                                       (gint) aInsertedText.SegmentStart,
                                       (gint)( aInsertedText.SegmentEnd - aInsertedText.SegmentStart ) );
            break;
        }

        case accessibility::AccessibleEventId::TEXT_SELECTION_CHANGED:
        {
            g_signal_emit_by_name( mpAccessible, "text-selection-changed" );
            break;
        }

        case accessibility::AccessibleEventId::TEXT_ATTRIBUTE_CHANGED:
            g_signal_emit_by_name( mpAccessible, "text-attributes-changed" );
            break;

        // AtkValue
        case accessibility::AccessibleEventId::VALUE_CHANGED:
            g_object_notify( G_OBJECT( mpAccessible ), "accessible-value" );
            break;

        case accessibility::AccessibleEventId::CONTENT_FLOWS_FROM_RELATION_CHANGED:
        case accessibility::AccessibleEventId::CONTENT_FLOWS_TO_RELATION_CHANGED:
        case accessibility::AccessibleEventId::CONTROLLED_BY_RELATION_CHANGED:
        case accessibility::AccessibleEventId::CONTROLLER_FOR_RELATION_CHANGED:
        case accessibility::AccessibleEventId::LABEL_FOR_RELATION_CHANGED:
        case accessibility::AccessibleEventId::LABELED_BY_RELATION_CHANGED:
        case accessibility::AccessibleEventId::MEMBER_OF_RELATION_CHANGED:
        case accessibility::AccessibleEventId::SUB_WINDOW_OF_RELATION_CHANGED:
            // FIXME: ask Bill how Atk copes with this little lot ...
            break;

        // AtkTable
        case accessibility::AccessibleEventId::TABLE_MODEL_CHANGED:
        {
            accessibility::AccessibleTableModelChange aChange;
            aEvent.NewValue >>= aChange;

            sal_Int32 nRowsChanged = aChange.LastRow - aChange.FirstRow + 1;
            sal_Int32 nColumnsChanged = aChange.LastColumn - aChange.FirstColumn + 1;

            static const struct {
                    const char *row;
                    const char *col;
            } aSignalNames[] =
            {
                { NULL, NULL }, // dummy
                { "row_inserted", "column_inserted" }, // INSERT = 1
                { "row_deleted", "column_deleted" } // DELETE = 2
            };
            switch( aChange.Type )
            {
            case accessibility::AccessibleTableModelChangeType::INSERT:
            case accessibility::AccessibleTableModelChangeType::DELETE:
                if( nRowsChanged > 0 )
                    g_signal_emit_by_name( G_OBJECT( mpAccessible ),
                                           aSignalNames[aChange.Type].row,
                                           aChange.FirstRow, nRowsChanged );
                if( nColumnsChanged > 0 )
                    g_signal_emit_by_name( G_OBJECT( mpAccessible ),
                                           aSignalNames[aChange.Type].col,
                                           aChange.FirstColumn, nColumnsChanged );
                break;

            case accessibility::AccessibleTableModelChangeType::UPDATE:
                // This is not really a model change, is it ?
                break;
            default:
                g_warning( "TESTME: unusual table model change %d\n", aChange.Type );
                break;
            }
            g_signal_emit_by_name( G_OBJECT( mpAccessible ), "model-changed" );
            break;
        }

        case accessibility::AccessibleEventId::TABLE_COLUMN_HEADER_CHANGED:
            g_signal_emit_by_name( G_OBJECT( mpAccessible ), "property_change::accessible-table-column-header");
            break;

        case accessibility::AccessibleEventId::TABLE_CAPTION_CHANGED:
            g_signal_emit_by_name( G_OBJECT( mpAccessible ), "property_change::accessible-table-caption");
            break;

        case accessibility::AccessibleEventId::TABLE_COLUMN_DESCRIPTION_CHANGED:
            g_signal_emit_by_name( G_OBJECT( mpAccessible ), "property_change::accessible-table-column-description");
            break;

        case accessibility::AccessibleEventId::TABLE_ROW_DESCRIPTION_CHANGED:
            g_signal_emit_by_name( G_OBJECT( mpAccessible ), "property_change::accessible-table-row-description");
            break;

        case accessibility::AccessibleEventId::TABLE_ROW_HEADER_CHANGED:
            g_signal_emit_by_name( G_OBJECT( mpAccessible ), "property_change::accessible-table-row-header");
            break;

        case accessibility::AccessibleEventId::TABLE_SUMMARY_CHANGED:
            g_signal_emit_by_name( G_OBJECT( mpAccessible ), "property_change::accessible-table-summary");
            break;

        case accessibility::AccessibleEventId::SELECTION_CHANGED:
            g_signal_emit_by_name( G_OBJECT( mpAccessible ), "selection_changed");
            break;

        case accessibility::AccessibleEventId::HYPERTEXT_CHANGED:
            g_signal_emit_by_name( G_OBJECT( mpAccessible ), "property_change::accessible-hypertext-offset");
            break;

    default:
            g_warning( "Unknown event notification %d", aEvent.EventId );
            break;
    }
}
