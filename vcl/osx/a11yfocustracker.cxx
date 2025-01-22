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

#include <vcl/svapp.hxx>
#include <vcl/window.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/menu.hxx>

#include <osx/a11yfocustracker.hxx>

#include "documentfocuslistener.hxx"

#include <com/sun/star/accessibility/XAccessibleContext.hpp>
#include <com/sun/star/accessibility/XAccessibleSelection.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>

using namespace ::com::sun::star::accessibility;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;

AquaA11yFocusTracker& TheAquaA11yFocusTracker()
{
    static AquaA11yFocusTracker SINGLETON;
    return SINGLETON;
}

static vcl::Window *
getWindow(const ::VclSimpleEvent *pEvent)
{
    return static_cast< const ::VclWindowEvent *> (pEvent)->GetWindow();
}

// callback function for Application::addEventListener

void AquaA11yFocusTracker::WindowEventHandler(void * pThis, VclSimpleEvent& rEvent)
{
    AquaA11yFocusTracker *pFocusTracker = static_cast<AquaA11yFocusTracker *>(
        pThis);
    switch (rEvent.GetId())
    {
    case VclEventId::WindowPaint:
        pFocusTracker-> toolbox_open_floater( getWindow(&rEvent) );
        break;
    case VclEventId::WindowGetFocus:
        pFocusTracker->window_got_focus( getWindow(&rEvent) );
        break;
    case VclEventId::ObjectDying:
        pFocusTracker->m_aDocumentWindowList.erase( getWindow(&rEvent) );
        [[fallthrough]];
    case VclEventId::ToolboxHighlightOff:
        pFocusTracker->toolbox_highlight_off( getWindow(&rEvent) );
        break;
    case VclEventId::ToolboxHighlight:
        pFocusTracker->toolbox_highlight_on( getWindow(&rEvent) );
        break;
    case VclEventId::TabpageActivate:
        pFocusTracker->tabpage_activated( getWindow(&rEvent) );
        break;
    case VclEventId::MenuHighlight:
        // Inspired by code in WindowEventHandler in
        // vcl/unx/gtk/a11y/atkutil.cxx, find out what kind of event
        // it is to avoid blindly using a static_cast and crash,
        // fdo#47275.
        if( const VclMenuEvent* pMenuEvent = dynamic_cast < const VclMenuEvent* > (&rEvent) )
        {
            pFocusTracker->menu_highlighted( pMenuEvent );
        }
        break;
    default:
        break;
    }
}

AquaA11yFocusTracker::AquaA11yFocusTracker() :
    m_aWindowEventLink(LINK_NONMEMBER(this, WindowEventHandler)),
    m_xDocumentFocusListener(new DocumentFocusListener(*this))
{
    Application::AddEventListener(m_aWindowEventLink);
    window_got_focus(Application::GetFocusWindow());
}

AquaA11yFocusTracker::~AquaA11yFocusTracker() {}

void AquaA11yFocusTracker::setFocusedObject(const Reference< XAccessible >& xAccessible)
{
    if( xAccessible != m_xFocusedObject )
    {
        m_xFocusedObject = xAccessible;

        if( m_aFocusListener.is() )
            m_aFocusListener->focusedObjectChanged(xAccessible);
    }
}

void AquaA11yFocusTracker::notify_toolbox_item_focus(ToolBox *pToolBox)
{
    Reference< XAccessible > xAccessible( pToolBox->GetAccessible() );

    if( xAccessible.is() )
    {
        Reference< XAccessibleContext > xContext(xAccessible->getAccessibleContext());

        if( xContext.is() )
        {
            try {
                ToolBox::ImplToolItems::size_type nPos = pToolBox->GetItemPos( pToolBox->GetHighlightItemId() );
                if( nPos != ToolBox::ITEM_NOTFOUND )
                    setFocusedObject( xContext->getAccessibleChild( nPos ) );
                        //TODO: ToolBox::ImplToolItems::size_type -> sal_Int32!
            }
            catch (const IndexOutOfBoundsException&)
            {
                SAL_WARN("vcl", "Accessible object has invalid index in parent");
            }
        }
    }
}

void AquaA11yFocusTracker::toolbox_open_floater(vcl::Window *pWindow)
{
    bool bToolboxFound = false;
    bool bFloatingWindowFound = false;
    vcl::Window * pFloatingWindow = nullptr;
    while ( pWindow != nullptr ) {
        if ( pWindow->GetType() == WindowType::TOOLBOX ) {
            bToolboxFound = true;
        } else if ( pWindow->GetType() == WindowType::FLOATINGWINDOW ) {
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
            try {
                Reference < XAccessible > rxAccessibleChild = rxContext -> getAccessibleChild( 0 );
                if ( ! rxAccessibleChild.is() ) {
                    return;
                }
                setFocusedObject ( rxAccessibleChild );
            }
            catch (const IndexOutOfBoundsException&)
            {
                SAL_WARN("vcl", "No valid accessible objects in parent");
            }
        }
    }
}

void AquaA11yFocusTracker::toolbox_highlight_on(vcl::Window *pWindow)
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

void AquaA11yFocusTracker::toolbox_highlight_off(vcl::Window const *pWindow)
{
    ToolBox* pToolBoxParent = dynamic_cast< ToolBox * >( pWindow->GetParent() );

    // Notify when leaving sub toolboxes
    if( pToolBoxParent && pToolBoxParent->HasFocus() )
        notify_toolbox_item_focus( pToolBoxParent );
}

void AquaA11yFocusTracker::tabpage_activated(vcl::Window *pWindow)
{
    Reference< XAccessible > xAccessible( pWindow->GetAccessible() );

    if( xAccessible.is() )
    {
        Reference< XAccessibleSelection > xSelection(xAccessible->getAccessibleContext(), UNO_QUERY);

        if( xSelection.is() )
            setFocusedObject( xSelection->getSelectedAccessibleChild(0) );
    }
}

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

void AquaA11yFocusTracker::window_got_focus(vcl::Window *pWindow)
{
    // The menu bar is handled through VclEventId::MenuHighlightED
    if( ! pWindow || !pWindow->IsReallyVisible() || pWindow->GetType() == WindowType::MENUBARWINDOW )
        return;

    // ToolBoxes are handled through VclEventId::ToolboxHighlight
    if( pWindow->GetType() == WindowType::TOOLBOX )
        return;

    if( pWindow->GetType() == WindowType::TABCONTROL )
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

    sal_Int64 nStateSet = xContext->getAccessibleStateSet();

    if( ! nStateSet )
        return;

/* the UNO ToolBox wrapper does not (yet?) support XAccessibleSelection, so we
 * need to add listeners to the children instead of re-using the tabpage stuff
 */
    if( (nStateSet & AccessibleStateType::FOCUSED) && (pWindow->GetType() != WindowType::TREELISTBOX) )
    {
        setFocusedObject( xAccessible );
    }
    else
    {
        if( m_aDocumentWindowList.insert(pWindow).second )
            m_xDocumentFocusListener->attachRecursive(xAccessible, xContext, nStateSet);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
