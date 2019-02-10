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

#ifdef AIX
#define _LINUX_SOURCE_COMPAT
#include <sys/timer.h>
#undef _LINUX_SOURCE_COMPAT
#endif

#include <com/sun/star/accessibility/XAccessibleContext.hpp>
#include <com/sun/star/accessibility/XAccessibleEventBroadcaster.hpp>
#include <com/sun/star/accessibility/XAccessibleSelection.hpp>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/XAccessibleText.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/weakref.hxx>
#include <rtl/ref.hxx>
#include <sal/log.hxx>

#include <vcl/svapp.hxx>
#include <vcl/window.hxx>
#include <vcl/menu.hxx>
#include <vcl/toolbox.hxx>

#include <unx/gtk/gtkdata.hxx>
#include "atkwrapper.hxx"
#include "atkutil.hxx"

#include <gtk/gtk.h>
#include <config_version.h>

#include <cassert>
#include <set>

using namespace ::com::sun::star;

namespace
{
    struct theNextFocusObject :
        public rtl::Static< uno::WeakReference< accessibility::XAccessible >, theNextFocusObject>
    {
    };
}

static guint focus_notify_handler = 0;

/*****************************************************************************/

extern "C" {

static gboolean
atk_wrapper_focus_idle_handler (gpointer data)
{
    SolarMutexGuard aGuard;

    focus_notify_handler = 0;

    uno::Reference< accessibility::XAccessible > xAccessible = theNextFocusObject::get();
    if( xAccessible.get() == static_cast < accessibility::XAccessible * > (data) )
    {
        AtkObject *atk_obj = xAccessible.is() ? atk_object_wrapper_ref( xAccessible ) : nullptr;
        // Gail does not notify focus changes to NULL, so do we ..
        if( atk_obj )
        {
            SAL_WNODEPRECATED_DECLARATIONS_PUSH
            atk_focus_tracker_notify(atk_obj);
            SAL_WNODEPRECATED_DECLARATIONS_POP
            // #i93269#
            // emit text_caret_moved event for <XAccessibleText> object,
            // if cursor is inside the <XAccessibleText> object.
            // also emit state-changed:focused event under the same condition.
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
                            atk_object_notify_state_change( atk_obj, ATK_STATE_FOCUSED, TRUE );
                            g_signal_emit_by_name( atk_obj, "text_caret_moved", caretPos );
                        }
                    }
                }
            }
            g_object_unref(atk_obj);
        }
    }

    return false;
}

} // extern "C"

/*****************************************************************************/

static void
atk_wrapper_focus_tracker_notify_when_idle( const uno::Reference< accessibility::XAccessible > &xAccessible )
{
    if( focus_notify_handler )
        g_source_remove(focus_notify_handler);

    theNextFocusObject::get() = xAccessible;

    focus_notify_handler = g_idle_add (atk_wrapper_focus_idle_handler, xAccessible.get());
}

/*****************************************************************************/

class DocumentFocusListener :
    public ::cppu::WeakImplHelper< accessibility::XAccessibleEventListener >
{

    std::set< uno::Reference< uno::XInterface > > m_aRefList;

public:
    /// @throws lang::IndexOutOfBoundsException
    /// @throws uno::RuntimeException
    void attachRecursive(
        const uno::Reference< accessibility::XAccessible >& xAccessible
    );

    /// @throws lang::IndexOutOfBoundsException
    /// @throws uno::RuntimeException
    void attachRecursive(
        const uno::Reference< accessibility::XAccessible >& xAccessible,
        const uno::Reference< accessibility::XAccessibleContext >& xContext
    );

    /// @throws lang::IndexOutOfBoundsException
    /// @throws uno::RuntimeException
    void attachRecursive(
        const uno::Reference< accessibility::XAccessible >& xAccessible,
        const uno::Reference< accessibility::XAccessibleContext >& xContext,
        const uno::Reference< accessibility::XAccessibleStateSet >& xStateSet
    );

    /// @throws lang::IndexOutOfBoundsException
    /// @throws uno::RuntimeException
    void detachRecursive(
        const uno::Reference< accessibility::XAccessible >& xAccessible
    );

    /// @throws lang::IndexOutOfBoundsException
    /// @throws uno::RuntimeException
    void detachRecursive(
        const uno::Reference< accessibility::XAccessibleContext >& xContext
    );

    /// @throws lang::IndexOutOfBoundsException
    /// @throws uno::RuntimeException
    void detachRecursive(
        const uno::Reference< accessibility::XAccessibleContext >& xContext,
        const uno::Reference< accessibility::XAccessibleStateSet >& xStateSet
    );

    /// @throws lang::IndexOutOfBoundsException
    /// @throws uno::RuntimeException
    static uno::Reference< accessibility::XAccessible > getAccessible(const lang::EventObject& aEvent );

    // XEventListener
    virtual void SAL_CALL disposing( const lang::EventObject& Source ) override;

    // XAccessibleEventListener
    virtual void SAL_CALL notifyEvent( const accessibility::AccessibleEventObject& aEvent ) override;
};

/*****************************************************************************/

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
                sal_Int16 nState = accessibility::AccessibleStateType::INVALID;
                aEvent.NewValue >>= nState;

                if( accessibility::AccessibleStateType::FOCUSED == nState )
                    atk_wrapper_focus_tracker_notify_when_idle( getAccessible(aEvent) );

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
                SAL_INFO("vcl.a11y", "Invalidate all children called");
                break;

            default:
                break;
        }
    }
    catch( const lang::IndexOutOfBoundsException& e )
    {
        g_warning("Focused object has invalid index in parent");
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
    uno::Reference< accessibility::XAccessibleStateSet > xStateSet =
        xContext->getAccessibleStateSet();

    if( xStateSet.is() )
        attachRecursive(xAccessible, xContext, xStateSet);
}

/*****************************************************************************/

void DocumentFocusListener::attachRecursive(
    const uno::Reference< accessibility::XAccessible >& xAccessible,
    const uno::Reference< accessibility::XAccessibleContext >& xContext,
    const uno::Reference< accessibility::XAccessibleStateSet >& xStateSet
)
{
    if( xStateSet->contains(accessibility::AccessibleStateType::FOCUSED ) )
        atk_wrapper_focus_tracker_notify_when_idle( xAccessible );

    uno::Reference< accessibility::XAccessibleEventBroadcaster > xBroadcaster =
        uno::Reference< accessibility::XAccessibleEventBroadcaster >(xContext, uno::UNO_QUERY);

    if (!xBroadcaster.is())
        return;

    // If not already done, add the broadcaster to the list and attach as listener.
    const uno::Reference< uno::XInterface >& xInterface = xBroadcaster;
    if( m_aRefList.insert(xInterface).second )
    {
        xBroadcaster->addAccessibleEventListener(static_cast< accessibility::XAccessibleEventListener *>(this));

        if( ! xStateSet->contains(accessibility::AccessibleStateType::MANAGES_DESCENDANTS ) )
        {
            sal_Int32 n, nmax = xContext->getAccessibleChildCount();
            for( n = 0; n < nmax; n++ )
            {
                uno::Reference< accessibility::XAccessible > xChild( xContext->getAccessibleChild( n ) );

                if( xChild.is() )
                    attachRecursive(xChild);
            }
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
    uno::Reference< accessibility::XAccessibleStateSet > xStateSet =
        xContext->getAccessibleStateSet();

    if( xStateSet.is() )
        detachRecursive(xContext, xStateSet);
}

/*****************************************************************************/

void DocumentFocusListener::detachRecursive(
    const uno::Reference< accessibility::XAccessibleContext >& xContext,
    const uno::Reference< accessibility::XAccessibleStateSet >& xStateSet
)
{
    uno::Reference< accessibility::XAccessibleEventBroadcaster > xBroadcaster =
        uno::Reference< accessibility::XAccessibleEventBroadcaster >(xContext, uno::UNO_QUERY);

    if( xBroadcaster.is() && 0 < m_aRefList.erase(xBroadcaster) )
    {
        xBroadcaster->removeAccessibleEventListener(static_cast< accessibility::XAccessibleEventListener *>(this));

        if( ! xStateSet->contains(accessibility::AccessibleStateType::MANAGES_DESCENDANTS ) )
        {
            sal_Int32 n, nmax = xContext->getAccessibleChildCount();
            for( n = 0; n < nmax; n++ )
            {
                uno::Reference< accessibility::XAccessible > xChild( xContext->getAccessibleChild( n ) );

                if( xChild.is() )
                    detachRecursive(xChild);
            }
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
        atk_wrapper_focus_tracker_notify_when_idle( xSelection->getSelectedAccessibleChild(0) );
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
        atk_wrapper_focus_tracker_notify_when_idle( xContext->getAccessibleChild( nPos ) );
            //TODO: ToolBox::ImplToolItems::size_type -> sal_Int32
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

/*****************************************************************************/

namespace {

struct WindowList {
    ~WindowList() { assert(list.empty()); };
        // needs to be empty already on DeInitVCL, but at least check it's empty
        // on exit

    std::set< VclPtr<vcl::Window> > list;
};

WindowList g_aWindowList;

}

DocumentFocusListener & GtkSalData::GetDocumentFocusListener()
{
    if (!m_pDocumentFocusListener)
    {
        m_pDocumentFocusListener = new DocumentFocusListener;
        m_xDocumentFocusListener.set(m_pDocumentFocusListener);
    }
    return *m_pDocumentFocusListener;
}

static void handle_get_focus(::VclWindowEvent const * pEvent)
{
    GtkSalData *const pSalData(GetGtkSalData());
    assert(pSalData);

    DocumentFocusListener & rDocumentFocusListener(pSalData->GetDocumentFocusListener());

    vcl::Window *pWindow = pEvent->GetWindow();

    // The menu bar is handled through VclEventId::MenuHighlightED
    if( ! pWindow || !pWindow->IsReallyVisible() || pWindow->GetType() == WindowType::MENUBARWINDOW )
        return;

    // ToolBoxes are handled through VclEventId::ToolboxHighlight
    if( pWindow->GetType() == WindowType::TOOLBOX )
        return;

    if( pWindow->GetType() == WindowType::TABCONTROL )
    {
        handle_tabpage_activated( pWindow );
        return;
    }

    uno::Reference< accessibility::XAccessible > xAccessible =
        pWindow->GetAccessible();

    if( ! xAccessible.is() )
        return;

    uno::Reference< accessibility::XAccessibleContext > xContext =
        xAccessible->getAccessibleContext();

    if( ! xContext.is() )
        return;

    uno::Reference< accessibility::XAccessibleStateSet > xStateSet =
        xContext->getAccessibleStateSet();

    if( ! xStateSet.is() )
        return;

/* the UNO ToolBox wrapper does not (yet?) support XAccessibleSelection, so we
 * need to add listeners to the children instead of re-using the tabpage stuff
 */
    if( xStateSet->contains(accessibility::AccessibleStateType::FOCUSED) &&
        ( pWindow->GetType() != WindowType::TREELISTBOX ) )
    {
        atk_wrapper_focus_tracker_notify_when_idle( xAccessible );
    }
    else
    {
        if( g_aWindowList.list.find(pWindow) == g_aWindowList.list.end() )
        {
            g_aWindowList.list.insert(pWindow);
            try
            {
                rDocumentFocusListener.attachRecursive(xAccessible, xContext, xStateSet);
            }
            catch (const uno::Exception&)
            {
                g_warning( "Exception caught processing focus events" );
            }
        }
    }
}

/*****************************************************************************/

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
                    atk_wrapper_focus_tracker_notify_when_idle( xContext->getAccessibleChild( nPos ) );
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
        case VclEventId::WindowShow:
            break;
        case VclEventId::WindowHide:
            break;
        case VclEventId::WindowClose:
            break;
        case VclEventId::WindowGetFocus:
            handle_get_focus(static_cast< ::VclWindowEvent const * >(&rEvent));
            break;
        case VclEventId::WindowLoseFocus:
            break;
        case VclEventId::WindowMinimize:
            break;
        case VclEventId::WindowNormalize:
            break;
        case VclEventId::WindowKeyInput:
        case VclEventId::WindowKeyUp:
        case VclEventId::WindowCommand:
        case VclEventId::WindowMouseMove:
            break;

        case VclEventId::MenuHighlight:
            if (const VclMenuEvent* pMenuEvent = dynamic_cast<const VclMenuEvent*>(&rEvent))
            {
                handle_menu_highlighted(pMenuEvent);
            }
            else if (const VclAccessibleEvent* pAccEvent = dynamic_cast<const VclAccessibleEvent*>(&rEvent))
            {
                const uno::Reference< accessibility::XAccessible >& xAccessible = pAccEvent->GetAccessible();
                if (xAccessible.is())
                    atk_wrapper_focus_tracker_notify_when_idle(xAccessible);
            }
            break;

        case VclEventId::ToolboxHighlight:
            handle_toolbox_highlight(static_cast< ::VclWindowEvent const * >(&rEvent)->GetWindow());
            break;

        case VclEventId::ToolboxButtonStateChanged:
            handle_toolbox_buttonchange(static_cast< ::VclWindowEvent const * >(&rEvent));
            break;

        case VclEventId::ObjectDying:
            g_aWindowList.list.erase( static_cast< ::VclWindowEvent const * >(&rEvent)->GetWindow() );
            [[fallthrough]];
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
        g_warning("Focused object has invalid index in parent");
    }
}

static Link<VclSimpleEvent&,void> g_aEventListenerLink( nullptr, WindowEventHandler );

/*****************************************************************************/

extern "C" {

static G_CONST_RETURN gchar *
ooo_atk_util_get_toolkit_name()
{
    return "VCL";
}

/*****************************************************************************/

static G_CONST_RETURN gchar *
ooo_atk_util_get_toolkit_version()
{
    return LIBO_VERSION_DOTTED;
}

/*****************************************************************************/

/*
 * GObject inheritance
 */

static void
ooo_atk_util_class_init (AtkUtilClass *)
{
    AtkUtilClass *atk_class;
    gpointer data;

    data = g_type_class_peek (ATK_TYPE_UTIL);
    atk_class = ATK_UTIL_CLASS (data);

    atk_class->get_toolkit_name = ooo_atk_util_get_toolkit_name;
    atk_class->get_toolkit_version = ooo_atk_util_get_toolkit_version;

    ooo_atk_util_ensure_event_listener();
}

} // extern "C"

void ooo_atk_util_ensure_event_listener()
{
    static bool bInited;
    if (!bInited)
    {
        Application::AddEventListener( g_aEventListenerLink );
        bInited = true;
    }
}

GType
ooo_atk_util_get_type()
{
    static GType type = 0;

    if (!type)
    {
        GType parent_type = g_type_from_name( "GailUtil" );

        if( ! parent_type )
        {
            g_warning( "Unknown type: GailUtil" );
            parent_type = ATK_TYPE_UTIL;
        }

        GTypeQuery type_query;
        g_type_query( parent_type, &type_query );

        static const GTypeInfo typeInfo =
        {
            static_cast<guint16>(type_query.class_size),
            nullptr,
            nullptr,
            reinterpret_cast<GClassInitFunc>(ooo_atk_util_class_init),
            nullptr,
            nullptr,
            static_cast<guint16>(type_query.instance_size),
            0,
            nullptr,
            nullptr
        } ;

        type = g_type_register_static (parent_type, "OOoUtil", &typeInfo, GTypeFlags(0)) ;
    }

    return type;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
