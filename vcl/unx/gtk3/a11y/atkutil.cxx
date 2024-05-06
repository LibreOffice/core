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

#include <com/sun/star/accessibility/XAccessibleEventBroadcaster.hpp>
#include <com/sun/star/accessibility/XAccessibleSelection.hpp>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/XAccessibleText.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/weakref.hxx>
#include <sal/log.hxx>

#include <vcl/svapp.hxx>
#include <vcl/window.hxx>
#include <vcl/menu.hxx>
#include <vcl/toolbox.hxx>

#include <unx/gtk/gtkdata.hxx>
#include "atkwrapper.hxx"
#include "atkutil.hxx"

#include <cassert>
#include <set>

using namespace ::com::sun::star;

static void
atk_wrapper_notify_focus_change(const uno::Reference<accessibility::XAccessible>& xAccessible)
{
    AtkObject *atk_obj = xAccessible.is() ? atk_object_wrapper_ref( xAccessible ) : nullptr;
    // Gail does not notify focus changes to NULL, so do we ..
    if( atk_obj )
    {
        atk_object_notify_state_change(atk_obj, ATK_STATE_FOCUSED, true);
        // #i93269#
        // emit text_caret_moved event for <XAccessibleText> object,
        // if cursor is inside the <XAccessibleText> object.
        {
            AtkObjectWrapper* wrapper_obj = ATK_OBJECT_WRAPPER (atk_obj);
            if( wrapper_obj && !wrapper_obj->mpText.is() )
            {
                wrapper_obj->mpText.set(wrapper_obj->mpContext, css::uno::UNO_QUERY);
                if ( wrapper_obj->mpText.is() )
                {
                    gint caretPos = -1;

                    try {
                        caretPos = wrapper_obj->mpText->getCaretPosition();
                    }
                    catch(const uno::Exception&) {
                        g_warning( "Exception in getCaretPosition()" );
                    }

                    if ( caretPos != -1 )
                    {
                        g_signal_emit_by_name( atk_obj, "text_caret_moved", caretPos );
                    }
                }
            }
        }
        g_object_unref(atk_obj);
    }
}

void DocumentFocusListener::disposing( const lang::EventObject& aEvent )
{

    // Unref the object here, but do not remove as listener since the object
    // might no longer be in a state that safely allows this.
    if( aEvent.Source.is() )
        m_aRefList.erase(aEvent.Source);

}

/*****************************************************************************/

void DocumentFocusListener::notifyEvent( const accessibility::AccessibleEventObject& aEvent )
{
    try {
        switch( aEvent.EventId )
        {
            case accessibility::AccessibleEventId::STATE_CHANGED:
            {
                sal_Int64 nState = accessibility::AccessibleStateType::INVALID;
                aEvent.NewValue >>= nState;

                if( accessibility::AccessibleStateType::FOCUSED == nState )
                    atk_wrapper_notify_focus_change(getAccessible(aEvent));

                break;
            }

            case accessibility::AccessibleEventId::CHILD:
            {
                uno::Reference< accessibility::XAccessible > xChild;
                if( (aEvent.OldValue >>= xChild) && xChild.is() )
                    detachRecursive(xChild);

                if( (aEvent.NewValue >>= xChild) && xChild.is() )
                    attachRecursive(xChild);

                break;
            }

            case accessibility::AccessibleEventId::INVALIDATE_ALL_CHILDREN:
            {
                if (uno::Reference< accessibility::XAccessible > xAcc = getAccessible(aEvent))
                    detachRecursive(xAcc);
                break;
            }

            default:
                break;
        }
    }
    catch( const lang::IndexOutOfBoundsException& )
    {
        g_warning("DocumentFocusListener: Focused object has invalid index in parent");
    }
}

/*****************************************************************************/

uno::Reference< accessibility::XAccessible > DocumentFocusListener::getAccessible(const lang::EventObject& aEvent )
{
    uno::Reference< accessibility::XAccessible > xAccessible(aEvent.Source, uno::UNO_QUERY);

    if( xAccessible.is() )
        return xAccessible;

    uno::Reference< accessibility::XAccessibleContext > xContext(aEvent.Source, uno::UNO_QUERY);

    if( xContext.is() )
    {
        uno::Reference< accessibility::XAccessible > xParent( xContext->getAccessibleParent() );
        if( xParent.is() )
        {
            uno::Reference< accessibility::XAccessibleContext > xParentContext( xParent->getAccessibleContext() );
            if( xParentContext.is() )
            {
                return xParentContext->getAccessibleChild( xContext->getAccessibleIndexInParent() );
            }
        }
    }

    return uno::Reference< accessibility::XAccessible >();
}

/*****************************************************************************/

void DocumentFocusListener::attachRecursive(
    const uno::Reference< accessibility::XAccessible >& xAccessible
)
{
    uno::Reference< accessibility::XAccessibleContext > xContext =
        xAccessible->getAccessibleContext();

    if( xContext.is() )
        attachRecursive(xAccessible, xContext);
}

/*****************************************************************************/

void DocumentFocusListener::attachRecursive(
    const uno::Reference< accessibility::XAccessible >& xAccessible,
    const uno::Reference< accessibility::XAccessibleContext >& xContext
)
{
    sal_Int64 nStateSet = xContext->getAccessibleStateSet();
    attachRecursive(xAccessible, xContext, nStateSet);
}

/*****************************************************************************/

void DocumentFocusListener::attachRecursive(
    const uno::Reference< accessibility::XAccessible >& xAccessible,
    const uno::Reference< accessibility::XAccessibleContext >& xContext,
    sal_Int64 nStateSet
)
{
    if( nStateSet & accessibility::AccessibleStateType::FOCUSED )
        atk_wrapper_notify_focus_change(xAccessible);

    uno::Reference< accessibility::XAccessibleEventBroadcaster > xBroadcaster(xContext, uno::UNO_QUERY);

    if (!xBroadcaster.is())
        return;

    // If not already done, add the broadcaster to the list and attach as listener.
    const uno::Reference< uno::XInterface >& xInterface = xBroadcaster;
    if( !m_aRefList.insert(xInterface).second )
        return;

    xBroadcaster->addAccessibleEventListener(static_cast< accessibility::XAccessibleEventListener *>(this));

    if( ! (nStateSet & accessibility::AccessibleStateType::MANAGES_DESCENDANTS) )
    {
        sal_Int64 n, nmax = xContext->getAccessibleChildCount();
        for( n = 0; n < nmax; n++ )
        {
            uno::Reference< accessibility::XAccessible > xChild( xContext->getAccessibleChild( n ) );

            if( xChild.is() )
                attachRecursive(xChild);
        }
    }
}

/*****************************************************************************/

void DocumentFocusListener::detachRecursive(
    const uno::Reference< accessibility::XAccessible >& xAccessible
)
{
    uno::Reference< accessibility::XAccessibleContext > xContext =
        xAccessible->getAccessibleContext();

    if( xContext.is() )
        detachRecursive(xContext);
}

/*****************************************************************************/

void DocumentFocusListener::detachRecursive(
    const uno::Reference< accessibility::XAccessibleContext >& xContext
)
{
    sal_Int64 nStateSet = xContext->getAccessibleStateSet();

    detachRecursive(xContext, nStateSet);
}

/*****************************************************************************/

void DocumentFocusListener::detachRecursive(
    const uno::Reference< accessibility::XAccessibleContext >& xContext,
    sal_Int64 nStateSet
)
{
    uno::Reference< accessibility::XAccessibleEventBroadcaster > xBroadcaster(xContext, uno::UNO_QUERY);

    if( !xBroadcaster.is() || 0 >= m_aRefList.erase(xBroadcaster) )
        return;

    xBroadcaster->removeAccessibleEventListener(static_cast< accessibility::XAccessibleEventListener *>(this));

    if( ! (nStateSet & accessibility::AccessibleStateType::MANAGES_DESCENDANTS) )
    {
        sal_Int64 n, nmax = xContext->getAccessibleChildCount();
        for( n = 0; n < nmax; n++ )
        {
            uno::Reference< accessibility::XAccessible > xChild( xContext->getAccessibleChild( n ) );

            if( xChild.is() )
                detachRecursive(xChild);
        }
    }
}

/*****************************************************************************/

/*
 * page tabs in gtk are widgets, so we need to simulate focus events for those
 */

static void handle_tabpage_activated(vcl::Window *pWindow)
{
    uno::Reference< accessibility::XAccessible > xAccessible =
        pWindow->GetAccessible();

    if( ! xAccessible.is() )
        return;

    uno::Reference< accessibility::XAccessibleSelection > xSelection(
        xAccessible->getAccessibleContext(), uno::UNO_QUERY);

    if( xSelection.is() )
        atk_wrapper_notify_focus_change(xSelection->getSelectedAccessibleChild(0));
}

/*****************************************************************************/

/*
 * toolbar items in gtk are widgets, so we need to simulate focus events for those
 */

static void notify_toolbox_item_focus(ToolBox *pToolBox)
{
    uno::Reference< accessibility::XAccessible > xAccessible =
        pToolBox->GetAccessible();

    if( ! xAccessible.is() )
        return;

    uno::Reference< accessibility::XAccessibleContext > xContext =
        xAccessible->getAccessibleContext();

    if( ! xContext.is() )
        return;

    ToolBox::ImplToolItems::size_type nPos = pToolBox->GetItemPos( pToolBox->GetHighlightItemId() );
    if( nPos != ToolBox::ITEM_NOTFOUND )
        atk_wrapper_notify_focus_change(xContext->getAccessibleChild(nPos));
}

static void handle_toolbox_highlight(vcl::Window *pWindow)
{
    ToolBox *pToolBox = static_cast <ToolBox *> (pWindow);

    // Make sure either the toolbox or its parent toolbox has the focus
    if ( ! pToolBox->HasFocus() )
    {
        ToolBox* pToolBoxParent = dynamic_cast< ToolBox* >( pToolBox->GetParent() );
        if ( ! pToolBoxParent || ! pToolBoxParent->HasFocus() )
            return;
    }

    notify_toolbox_item_focus(pToolBox);
}

static void handle_toolbox_highlightoff(vcl::Window const *pWindow)
{
    ToolBox* pToolBoxParent = dynamic_cast< ToolBox* >( pWindow->GetParent() );

    // Notify when leaving sub toolboxes
    if( pToolBoxParent && pToolBoxParent->HasFocus() )
        notify_toolbox_item_focus( pToolBoxParent );
}

/*****************************************************************************/

static void create_wrapper_for_child(
    const uno::Reference< accessibility::XAccessibleContext >& xContext,
    sal_Int32 index)
{
    if( xContext.is() )
    {
        uno::Reference< accessibility::XAccessible > xChild(xContext->getAccessibleChild(index));
        if( xChild.is() )
        {
            // create the wrapper object - it will survive the unref unless it is a transient object
            g_object_unref( atk_object_wrapper_ref( xChild ) );
        }
    }
}

/*****************************************************************************/

static void handle_toolbox_buttonchange(VclWindowEvent const *pEvent)
{
    vcl::Window* pWindow = pEvent->GetWindow();
    sal_Int32 index = static_cast<sal_Int32>(reinterpret_cast<sal_IntPtr>(pEvent->GetData()));

    if( pWindow && pWindow->IsReallyVisible() )
    {
        uno::Reference< accessibility::XAccessible > xAccessible(pWindow->GetAccessible());
        if( xAccessible.is() )
        {
            create_wrapper_for_child(xAccessible->getAccessibleContext(), index);
        }
    }
}

rtl::Reference<DocumentFocusListener> GtkSalData::GetDocumentFocusListener()
{
    rtl::Reference<DocumentFocusListener> xDFL = m_xDocumentFocusListener.get();
    if (!xDFL)
    {
        xDFL = new DocumentFocusListener;
        m_xDocumentFocusListener = xDFL.get();
    }
    return xDFL;
}

static void handle_menu_highlighted(::VclMenuEvent const * pEvent)
{
    try
    {
        Menu* pMenu = pEvent->GetMenu();
        sal_uInt16 nPos = pEvent->GetItemPos();

        if( pMenu &&  nPos != 0xFFFF)
        {
            uno::Reference< accessibility::XAccessible > xAccessible ( pMenu->GetAccessible() );

            if( xAccessible.is() )
            {
                uno::Reference< accessibility::XAccessibleContext > xContext ( xAccessible->getAccessibleContext() );

                if( xContext.is() )
                    atk_wrapper_notify_focus_change(xContext->getAccessibleChild(nPos));
            }
        }
    }
    catch (const uno::Exception&)
    {
        g_warning( "Exception caught processing menu highlight events" );
    }
}

/*****************************************************************************/

static void WindowEventHandler(void *, VclSimpleEvent& rEvent)
{
    try
    {
        switch (rEvent.GetId())
        {
        case VclEventId::MenuHighlight:
            if (const VclMenuEvent* pMenuEvent = dynamic_cast<const VclMenuEvent*>(&rEvent))
            {
                handle_menu_highlighted(pMenuEvent);
            }
            break;

        case VclEventId::ToolboxHighlight:
            handle_toolbox_highlight(static_cast< ::VclWindowEvent const * >(&rEvent)->GetWindow());
            break;

        case VclEventId::ToolboxButtonStateChanged:
            handle_toolbox_buttonchange(static_cast< ::VclWindowEvent const * >(&rEvent));
            break;

        case VclEventId::ToolboxHighlightOff:
            handle_toolbox_highlightoff(static_cast< ::VclWindowEvent const * >(&rEvent)->GetWindow());
            break;

        case VclEventId::TabpageActivate:
            handle_tabpage_activated(static_cast< ::VclWindowEvent const * >(&rEvent)->GetWindow());
            break;

        case VclEventId::ComboboxSetText:
            // This looks quite strange to me. Stumbled over this when fixing #i104290#.
            // This kicked in when leaving the combobox in the toolbar, after that the events worked.
            // I guess this was a try to work around missing combobox events, which didn't do the full job, and shouldn't be necessary anymore.
            // Fix for #i104290# was done in toolkit/source/awt/vclxaccessiblecomponent, FOCUSED state for compound controls in general.
            // create_wrapper_for_children(static_cast< ::VclWindowEvent const * >(pEvent)->GetWindow());
            break;

        default:
            break;
        }
    }
    catch (const lang::IndexOutOfBoundsException&)
    {
        g_warning("WindowEventHandler: Focused object has invalid index in parent");
    }
}

static Link<VclSimpleEvent&,void> g_aEventListenerLink( nullptr, WindowEventHandler );

/*****************************************************************************/

void ooo_atk_util_ensure_event_listener()
{
    static bool bInited;
    if (!bInited)
    {
        Application::AddEventListener( g_aEventListenerLink );
        bInited = true;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
