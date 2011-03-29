/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#include "vcl/svapp.hxx"
#include "vcl/window.hxx"
#include "vcl/toolbox.hxx"
#include "vcl/menu.hxx"

#include "aqua/aqua11yfocustracker.hxx"

#include "documentfocuslistener.hxx"

#include <com/sun/star/accessibility/XAccessibleContext.hpp>
#include <com/sun/star/accessibility/XAccessibleSelection.hpp>
#include <com/sun/star/accessibility/XAccessibleStateSet.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/AccessibleRole.hpp>

using namespace ::com::sun::star::accessibility;
using namespace ::com::sun::star::uno;

//------------------------------------------------------------------------------

static inline Window *
getWindow(const ::VclSimpleEvent *pEvent)
{
    return static_cast< const ::VclWindowEvent *> (pEvent)->GetWindow();
}


//------------------------------------------------------------------------------

// callback function for Application::addEventListener

long AquaA11yFocusTracker::WindowEventHandler(AquaA11yFocusTracker *pFocusTracker, ::VclSimpleEvent const *pEvent)
{
    switch (pEvent->GetId())
    {
    case VCLEVENT_WINDOW_PAINT:
        pFocusTracker-> toolbox_open_floater( getWindow(pEvent) );
        break;
    case VCLEVENT_WINDOW_GETFOCUS:
        pFocusTracker->window_got_focus( getWindow(pEvent) );
        break;
    case VCLEVENT_OBJECT_DYING:
        pFocusTracker->m_aDocumentWindowList.erase( getWindow(pEvent) );
        // intentional pass through ..
    case VCLEVENT_TOOLBOX_HIGHLIGHTOFF:
        pFocusTracker->toolbox_highlight_off( getWindow(pEvent) );
        break;
    case VCLEVENT_TOOLBOX_HIGHLIGHT:
        pFocusTracker->toolbox_highlight_on( getWindow(pEvent) );
        break;
    case VCLEVENT_TABPAGE_ACTIVATE:
        pFocusTracker->tabpage_activated( getWindow(pEvent) );
        break;
    case VCLEVENT_MENU_HIGHLIGHT:
        pFocusTracker->menu_highlighted( static_cast < const VclMenuEvent * > (pEvent) );
        break;
    default:
        break;
    };

    return 0;
}

//------------------------------------------------------------------------------

AquaA11yFocusTracker::AquaA11yFocusTracker() :
    m_aWindowEventLink(this, (PSTUB) WindowEventHandler),
    m_xDocumentFocusListener(new DocumentFocusListener(*this))
{
    Application::AddEventListener(m_aWindowEventLink);
    window_got_focus(Application::GetFocusWindow());
}

//------------------------------------------------------------------------------

void AquaA11yFocusTracker::setFocusedObject(const Reference< XAccessible >& xAccessible)
{
    if( xAccessible != m_xFocusedObject )
    {
        m_xFocusedObject = xAccessible;

        if( m_aFocusListener.is() )
            m_aFocusListener->focusedObjectChanged(xAccessible);
    }
}

//------------------------------------------------------------------------------

void AquaA11yFocusTracker::notify_toolbox_item_focus(ToolBox *pToolBox)
{
    Reference< XAccessible > xAccessible( pToolBox->GetAccessible() );

    if( xAccessible.is() )
    {
        Reference< XAccessibleContext > xContext(xAccessible->getAccessibleContext());

        if( xContext.is() )
        {
            sal_Int32 nPos = pToolBox->GetItemPos( pToolBox->GetHighlightItemId() );
            if( nPos != TOOLBOX_ITEM_NOTFOUND )
                setFocusedObject( xContext->getAccessibleChild( nPos ) );
        }
    }
}

//------------------------------------------------------------------------------

void AquaA11yFocusTracker::toolbox_open_floater(Window *pWindow)
{
    bool bToolboxFound = false;
    bool bFloatingWindowFound = false;
    Window * pFloatingWindow = NULL;
    while ( pWindow != NULL ) {
        if ( pWindow->GetType() == WINDOW_TOOLBOX ) {
            bToolboxFound = true;
        } else if ( pWindow->GetType() == WINDOW_FLOATINGWINDOW ) {
            bFloatingWindowFound = true;
            pFloatingWindow = pWindow;
        }
        pWindow = pWindow->GetParent();
    }
    if ( bToolboxFound && bFloatingWindowFound ) {
        Reference < XAccessible > rxAccessible = pFloatingWindow -> GetAccessible();
        if ( ! rxAccessible.is() ) {
            return;
        }
        Reference < XAccessibleContext > rxContext = rxAccessible -> getAccessibleContext();
        if ( ! rxContext.is() ) {
            return;
        }
        if ( rxContext -> getAccessibleChildCount() > 0 ) {
            Reference < XAccessible > rxAccessibleChild = rxContext -> getAccessibleChild( 0 );
            if ( ! rxAccessibleChild.is() ) {
                return;
            }
            setFocusedObject ( rxAccessibleChild );
        }
    }
}

//------------------------------------------------------------------------------

void AquaA11yFocusTracker::toolbox_highlight_on(Window *pWindow)
{
    // Make sure either the toolbox or its parent toolbox has the focus
    if ( ! pWindow->HasFocus() )
    {
        ToolBox* pToolBoxParent = dynamic_cast< ToolBox * >( pWindow->GetParent() );
        if ( ! pToolBoxParent || ! pToolBoxParent->HasFocus() )
            return;
    }

    notify_toolbox_item_focus(static_cast <ToolBox *> (pWindow));
}

//------------------------------------------------------------------------------

void AquaA11yFocusTracker::toolbox_highlight_off(Window *pWindow)
{
    ToolBox* pToolBoxParent = dynamic_cast< ToolBox * >( pWindow->GetParent() );

    // Notify when leaving sub toolboxes
    if( pToolBoxParent && pToolBoxParent->HasFocus() )
        notify_toolbox_item_focus( pToolBoxParent );
}

//------------------------------------------------------------------------------

void AquaA11yFocusTracker::tabpage_activated(Window *pWindow)
{
    Reference< XAccessible > xAccessible( pWindow->GetAccessible() );

    if( xAccessible.is() )
    {
        Reference< XAccessibleSelection > xSelection(xAccessible->getAccessibleContext(), UNO_QUERY);

        if( xSelection.is() )
            setFocusedObject( xSelection->getSelectedAccessibleChild(0) );
    }
}

//------------------------------------------------------------------------------

void AquaA11yFocusTracker::menu_highlighted(const VclMenuEvent *pEvent)
{
    Menu * pMenu = pEvent->GetMenu();

    if( pMenu )
    {
        Reference< XAccessible > xAccessible( pMenu->GetAccessible() );

        if( xAccessible.is() )
            setFocusedObject( xAccessible );
    }
}

//------------------------------------------------------------------------------

void AquaA11yFocusTracker::window_got_focus(Window *pWindow)
{
    // The menu bar is handled through VCLEVENT_MENU_HIGHLIGHTED
    if( ! pWindow || !pWindow->IsReallyVisible() || pWindow->GetType() == WINDOW_MENUBARWINDOW )
        return;

    // ToolBoxes are handled through VCLEVENT_TOOLBOX_HIGHLIGHT
    if( pWindow->GetType() == WINDOW_TOOLBOX )
        return;

    if( pWindow->GetType() == WINDOW_TABCONTROL )
    {
        tabpage_activated( pWindow );
        return;
    }

    Reference< XAccessible > xAccessible(pWindow->GetAccessible());

    if( ! xAccessible.is() )
        return;

    Reference< XAccessibleContext > xContext = xAccessible->getAccessibleContext();

    if( ! xContext.is() )
        return;

    Reference< XAccessibleStateSet > xStateSet = xContext->getAccessibleStateSet();

    if( ! xStateSet.is() )
        return;

/* the UNO ToolBox wrapper does not (yet?) support XAccessibleSelection, so we
 * need to add listeners to the children instead of re-using the tabpage stuff
 */
    if( xStateSet->contains(AccessibleStateType::FOCUSED) && (pWindow->GetType() != WINDOW_TREELISTBOX) )
    {
        setFocusedObject( xAccessible );
    }
    else
    {
        if( m_aDocumentWindowList.find(pWindow) == m_aDocumentWindowList.end() )
        {
            m_aDocumentWindowList.insert(pWindow);
            m_xDocumentFocusListener->attachRecursive(xAccessible, xContext, xStateSet);
        }
#ifdef ENABLE_TRACING
        else
            fprintf(stderr, "Window %p already in the list\n", pWindow );
#endif
    }
}
