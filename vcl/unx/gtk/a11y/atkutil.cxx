/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: atkutil.cxx,v $
 * $Revision: 1.9 $
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
#include <com/sun/star/accessibility/XAccessibleContext.hpp>
#include <com/sun/star/accessibility/XAccessibleEventBroadcaster.hpp>
#include <com/sun/star/accessibility/XAccessibleSelection.hpp>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <cppuhelper/implbase1.hxx>
#include <vos/mutex.hxx>
#include <rtl/ref.hxx>
#include <vcl/svapp.hxx>

#include <vcl/window.hxx>
#include <vcl/menu.hxx>
#include <vcl/toolbox.hxx>

#include "atkwrapper.hxx"
#include "atkutil.hxx"

#include <set>

// #define ENABLE_TRACING

#ifdef ENABLE_TRACING
#include <stdio.h>
#endif

using namespace ::com::sun::star;

static AtkObject *last_focused_object = NULL;

/*****************************************************************************/

extern "C" {

static gint
atk_wrapper_focus_idle_handler (gpointer data)
{
    vos::OGuard aGuard( Application::GetSolarMutex() );

#ifdef ENABLE_TRACING
    fprintf(stderr, "%s focus event for %p\n",
        (data == last_focused_object) ? "notifying" : "ignoring"  , data );
#endif

    if( data == last_focused_object )
        atk_focus_tracker_notify( ATK_OBJECT( data ) );

    if( data )
        g_object_unref( G_OBJECT(data) );

    return FALSE;
}

} // extern "C"

/*****************************************************************************/

static void
atk_wrapper_focus_tracker_notify_when_idle( const uno::Reference< accessibility::XAccessible > &rAccessible )
{
    AtkObject *accessible = NULL;

    if( rAccessible.is() )
        accessible = atk_object_wrapper_ref( rAccessible );

    last_focused_object = accessible;
    g_idle_add (atk_wrapper_focus_idle_handler, accessible);
}

/*****************************************************************************/

class DocumentFocusListener :
    public ::cppu::WeakImplHelper1< accessibility::XAccessibleEventListener >
{

    std::set< uno::Reference< uno::XInterface > > m_aRefList;

public:
    void attachRecursive(
        const uno::Reference< accessibility::XAccessible >& xAccessible
    ) throw (lang::IndexOutOfBoundsException, uno::RuntimeException);

    void attachRecursive(
        const uno::Reference< accessibility::XAccessible >& xAccessible,
        const uno::Reference< accessibility::XAccessibleContext >& xContext
    ) throw (lang::IndexOutOfBoundsException, uno::RuntimeException);

    void attachRecursive(
        const uno::Reference< accessibility::XAccessible >& xAccessible,
        const uno::Reference< accessibility::XAccessibleContext >& xContext,
        const uno::Reference< accessibility::XAccessibleStateSet >& xStateSet
    ) throw (lang::IndexOutOfBoundsException, uno::RuntimeException);

    void detachRecursive(
        const uno::Reference< accessibility::XAccessible >& xAccessible
    ) throw (lang::IndexOutOfBoundsException, uno::RuntimeException);

    void detachRecursive(
        const uno::Reference< accessibility::XAccessible >& xAccessible,
        const uno::Reference< accessibility::XAccessibleContext >& xContext
    ) throw (lang::IndexOutOfBoundsException, uno::RuntimeException);

    void detachRecursive(
        const uno::Reference< accessibility::XAccessible >& xAccessible,
        const uno::Reference< accessibility::XAccessibleContext >& xContext,
        const uno::Reference< accessibility::XAccessibleStateSet >& xStateSet
    ) throw (lang::IndexOutOfBoundsException, uno::RuntimeException);

    static uno::Reference< accessibility::XAccessible > getAccessible(const lang::EventObject& aEvent )
        throw (lang::IndexOutOfBoundsException, uno::RuntimeException);

    // XEventListener
    virtual void disposing( const lang::EventObject& Source ) throw (uno::RuntimeException);

    // XAccessibleEventListener
    virtual void notifyEvent( const accessibility::AccessibleEventObject& aEvent ) throw( uno::RuntimeException );
};

/*****************************************************************************/

void DocumentFocusListener::disposing( const lang::EventObject& aEvent )
    throw (uno::RuntimeException)
{
//    fprintf(stderr, "In DocumentFocusListener::disposing (%p)\n", this);
//    fprintf(stderr, "m_aRefList has %d entries\n", m_aRefList.size());

    // Unref the object here, but do not remove as listener since the object
    // might no longer be in a state that safely allows this.
    if( aEvent.Source.is() )
        m_aRefList.erase(aEvent.Source);

//    fprintf(stderr, "m_aRefList has %d entries\n", m_aRefList.size());

}

/*****************************************************************************/

void DocumentFocusListener::notifyEvent( const accessibility::AccessibleEventObject& aEvent )
    throw( uno::RuntimeException )
{
    switch( aEvent.EventId )
    {
        case accessibility::AccessibleEventId::STATE_CHANGED:
            try
            {
                sal_Int16 nState = accessibility::AccessibleStateType::INVALID;
                aEvent.NewValue >>= nState;

                if( accessibility::AccessibleStateType::FOCUSED == nState )
                    atk_wrapper_focus_tracker_notify_when_idle( getAccessible(aEvent) );
            }
            catch(lang::IndexOutOfBoundsException e)
            {
                g_warning("Focused object has invalid index in parent");
            }
            break;

        case accessibility::AccessibleEventId::CHILD:
        {
            uno::Reference< accessibility::XAccessible > xChild;
            if( (aEvent.OldValue >>= xChild) && xChild.is() )
                detachRecursive(xChild);

            if( (aEvent.NewValue >>= xChild) && xChild.is() )
                attachRecursive(xChild);
        }
            break;

        case accessibility::AccessibleEventId::INVALIDATE_ALL_CHILDREN:
/*        {
            uno::Reference< accessibility::XAccessible > xAccessible( getAccessible(aEvent) );
            detachRecursive(xAccessible);
            attachRecursive(xAccessible);
        }
*/
            g_warning( "Invalidate all children called\n" );
            break;
        default:
            break;
    }
}

/*****************************************************************************/

uno::Reference< accessibility::XAccessible > DocumentFocusListener::getAccessible(const lang::EventObject& aEvent )
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
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
) throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
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
)  throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
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
) throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
    if( xStateSet->contains(accessibility::AccessibleStateType::FOCUSED ) )
        atk_wrapper_focus_tracker_notify_when_idle( xAccessible );

    uno::Reference< accessibility::XAccessibleEventBroadcaster > xBroadcaster =
        uno::Reference< accessibility::XAccessibleEventBroadcaster >(xContext, uno::UNO_QUERY);

    // If not already done, add the broadcaster to the list and attach as listener.
    if( xBroadcaster.is() && m_aRefList.insert(xBroadcaster).second )
    {
        xBroadcaster->addEventListener(static_cast< accessibility::XAccessibleEventListener *>(this));

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
) throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
    uno::Reference< accessibility::XAccessibleContext > xContext =
        xAccessible->getAccessibleContext();

    if( xContext.is() )
        detachRecursive(xAccessible, xContext);
}

/*****************************************************************************/

void DocumentFocusListener::detachRecursive(
    const uno::Reference< accessibility::XAccessible >& xAccessible,
    const uno::Reference< accessibility::XAccessibleContext >& xContext
)  throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
    uno::Reference< accessibility::XAccessibleStateSet > xStateSet =
        xContext->getAccessibleStateSet();

    if( xStateSet.is() )
        detachRecursive(xAccessible, xContext, xStateSet);
}

/*****************************************************************************/

void DocumentFocusListener::detachRecursive(
    const uno::Reference< accessibility::XAccessible >&,
    const uno::Reference< accessibility::XAccessibleContext >& xContext,
    const uno::Reference< accessibility::XAccessibleStateSet >& xStateSet
) throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
    uno::Reference< accessibility::XAccessibleEventBroadcaster > xBroadcaster =
        uno::Reference< accessibility::XAccessibleEventBroadcaster >(xContext, uno::UNO_QUERY);

    if( xBroadcaster.is() && 0 < m_aRefList.erase(xBroadcaster) )
    {
        xBroadcaster->removeEventListener(static_cast< accessibility::XAccessibleEventListener *>(this));

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

static void handle_tabpage_activated(Window *pWindow)
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

    sal_Int32 nPos = pToolBox->GetItemPos( pToolBox->GetHighlightItemId() );
    if( nPos != TOOLBOX_ITEM_NOTFOUND )
        atk_wrapper_focus_tracker_notify_when_idle( xContext->getAccessibleChild( nPos ) );
}

static void handle_toolbox_highlight(Window *pWindow)
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

static void handle_toolbox_highlightoff(Window *pWindow)
{
    ToolBox *pToolBox = static_cast <ToolBox *> (pWindow);
    ToolBox* pToolBoxParent = dynamic_cast< ToolBox* >( pToolBox->GetParent() );

    // Notify when leaving sub toolboxes
    if( pToolBoxParent && pToolBoxParent->HasFocus() )
        notify_toolbox_item_focus( pToolBoxParent );
}

static void handle_toolbox_buttonchange(VclWindowEvent const *pEvent)
{
    Window* pWindow = pEvent->GetWindow();
    sal_Int32 index = (sal_Int32)(sal_IntPtr) pEvent->GetData();

    if( pWindow && pWindow->IsReallyVisible() )
    {
        uno::Reference< accessibility::XAccessible > xAccessible(pWindow->GetAccessible());
        if( xAccessible.is() )
        {
            uno::Reference< accessibility::XAccessibleContext > xContext(xAccessible->getAccessibleContext());
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
    }
}

/*****************************************************************************/

static std::set< Window * > g_aWindowList;

static void handle_get_focus(::VclWindowEvent const * pEvent)
{
    static rtl::Reference< DocumentFocusListener > aDocumentFocusListener =
        new DocumentFocusListener();

    Window *pWindow = pEvent->GetWindow();

    // The menu bar is handled through VCLEVENT_MENU_HIGHLIGHTED
    if( ! pWindow || !pWindow->IsReallyVisible() || pWindow->GetType() == WINDOW_MENUBARWINDOW )
        return;

    // ToolBoxes are handled through VCLEVENT_TOOLBOX_HIGHLIGHT
    if( pWindow->GetType() == WINDOW_TOOLBOX )
        return;

    if( pWindow->GetType() == WINDOW_TABCONTROL )
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
        ( pWindow->GetType() != WINDOW_TREELISTBOX ) )
    {
        atk_wrapper_focus_tracker_notify_when_idle( xAccessible );
    }
    else
    {
        if( g_aWindowList.find(pWindow) == g_aWindowList.end() )
        {
            g_aWindowList.insert(pWindow);
            aDocumentFocusListener->attachRecursive(xAccessible, xContext, xStateSet);
        }
#ifdef ENABLE_TRACING
        else
            fprintf(stderr, "Window %p already in the list\n", pWindow );
#endif
    }
}

/*****************************************************************************/

static void handle_menu_highlighted(::VclMenuEvent const * pEvent)
{
    try
    {
        Menu* pMenu = pEvent->GetMenu();
        USHORT nPos = pEvent->GetItemPos();

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
    catch( uno::Exception e )
    {
        g_warning( "Exception caught processing menu highlight events" );
    }
}

/*****************************************************************************/

long WindowEventHandler(void *, ::VclSimpleEvent const * pEvent)
{
    switch (pEvent->GetId())
    {
    case VCLEVENT_WINDOW_SHOW:
//        fprintf(stderr, "got VCLEVENT_WINDOW_SHOW for %p\n",
//            static_cast< ::VclWindowEvent const * >(pEvent)->GetWindow());
        break;
    case VCLEVENT_WINDOW_HIDE:
//        fprintf(stderr, "got VCLEVENT_WINDOW_HIDE for %p\n",
//            static_cast< ::VclWindowEvent const * >(pEvent)->GetWindow());
        break;
    case VCLEVENT_WINDOW_CLOSE:
//        fprintf(stderr, "got VCLEVENT_WINDOW_CLOSE for %p\n",
//            static_cast< ::VclWindowEvent const * >(pEvent)->GetWindow());
        break;
    case VCLEVENT_WINDOW_GETFOCUS:
        handle_get_focus(static_cast< ::VclWindowEvent const * >(pEvent));
        break;
    case VCLEVENT_WINDOW_LOSEFOCUS:
//        fprintf(stderr, "got VCLEVENT_WINDOW_LOSEFOCUS for %p\n",
//            static_cast< ::VclWindowEvent const * >(pEvent)->GetWindow());
        break;
    case VCLEVENT_WINDOW_MINIMIZE:
//        fprintf(stderr, "got VCLEVENT_WINDOW_MINIMIZE for %p\n",
//            static_cast< ::VclWindowEvent const * >(pEvent)->GetWindow());
        break;
    case VCLEVENT_WINDOW_NORMALIZE:
//        fprintf(stderr, "got VCLEVENT_WINDOW_NORMALIZE for %p\n",
//            static_cast< ::VclWindowEvent const * >(pEvent)->GetWindow());
        break;
    case VCLEVENT_WINDOW_KEYINPUT:
    case VCLEVENT_WINDOW_KEYUP:
    case VCLEVENT_WINDOW_COMMAND:
        break;
 /*
        fprintf(stderr, "got VCLEVENT_WINDOW_COMMAND (%d) for %p\n",
            static_cast< ::CommandEvent const * > (
                static_cast< ::VclWindowEvent const * >(pEvent)->GetData())->GetCommand(),
            static_cast< ::VclWindowEvent const * >(pEvent)->GetWindow());
 */
    case VCLEVENT_MENU_HIGHLIGHT:
        handle_menu_highlighted(static_cast< ::VclMenuEvent const * >(pEvent));
        break;

    case VCLEVENT_TOOLBOX_HIGHLIGHT:
        handle_toolbox_highlight(static_cast< ::VclWindowEvent const * >(pEvent)->GetWindow());
        break;

    case VCLEVENT_TOOLBOX_BUTTONSTATECHANGED:
        handle_toolbox_buttonchange(static_cast< ::VclWindowEvent const * >(pEvent));
        break;

    case VCLEVENT_OBJECT_DYING:
    case VCLEVENT_TOOLBOX_HIGHLIGHTOFF:
        handle_toolbox_highlightoff(static_cast< ::VclWindowEvent const * >(pEvent)->GetWindow());
        break;

    case VCLEVENT_TABPAGE_ACTIVATE:
        handle_tabpage_activated(static_cast< ::VclWindowEvent const * >(pEvent)->GetWindow());
        break;

    default:
        //fprintf(stderr, "got event %d \n", pEvent->GetId());
        break;
    }
    return 0;
}

static Link g_aEventListenerLink( NULL, (PSTUB) WindowEventHandler );

/*****************************************************************************/

extern "C" {

static G_CONST_RETURN gchar *
ooo_atk_util_get_toolkit_name (void)
{
    return "VCL";
}

/*****************************************************************************/

static G_CONST_RETURN gchar *
ooo_atk_util_get_toolkit_version (void)
{
    /*
     * Version is passed in as a -D flag when this file is
     * compiled.
     */

    return VERSION;
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

    Application::AddEventListener( g_aEventListenerLink );
}

} // extern "C"

/*****************************************************************************/

GType
ooo_atk_util_get_type (void)
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
            type_query.class_size,
            (GBaseInitFunc) NULL,
            (GBaseFinalizeFunc) NULL,
            (GClassInitFunc) ooo_atk_util_class_init,
            (GClassFinalizeFunc) NULL,
            NULL,
            type_query.instance_size,
            0,
            (GInstanceInitFunc) NULL,
            NULL
        } ;

        type = g_type_register_static (parent_type, "OOoUtil", &typeInfo, (GTypeFlags)0) ;
  }

  return type;
}


