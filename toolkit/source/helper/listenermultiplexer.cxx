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

#include <toolkit/helper/listenermultiplexer.hxx>
#include <com/sun/star/lang/DisposedException.hpp>
#include <cppuhelper/queryinterface.hxx>
#include <tools/diagnose_ex.h>

//  class ListenerMultiplexerBase

ListenerMultiplexerBase::ListenerMultiplexerBase( ::cppu::OWeakObject& rSource )
    : ::comphelper::OInterfaceContainerHelper2( GetMutex() ), mrContext( rSource )
{
}

ListenerMultiplexerBase::~ListenerMultiplexerBase()
{
}

// css::uno::XInterface
css::uno::Any ListenerMultiplexerBase::queryInterface( const css::uno::Type & rType )
{
    return ::cppu::queryInterface( rType, static_cast< css::uno::XInterface* >(this) );
}


//  class EventListenerMultiplexer

EventListenerMultiplexer::EventListenerMultiplexer( ::cppu::OWeakObject& rSource )
    : ListenerMultiplexerBase( rSource )
{
}

void SAL_CALL EventListenerMultiplexer::acquire() throw ()
{
    return ListenerMultiplexerBase::acquire();
}
void SAL_CALL EventListenerMultiplexer::release() throw ()
{
    return ListenerMultiplexerBase::release();
}

// css::uno::XInterface
css::uno::Any EventListenerMultiplexer::queryInterface( const css::uno::Type & rType )
{
    css::uno::Any aRet = ::cppu::queryInterface( rType,
                                        static_cast< css::lang::XEventListener* >(this) );
    return (aRet.hasValue() ? aRet : ListenerMultiplexerBase::queryInterface( rType ));
}

// css::lang::XEventListener
void EventListenerMultiplexer::disposing( const css::lang::EventObject& )
{
}


//  class FocusListenerMultiplexer

IMPL_LISTENERMULTIPLEXER_BASEMETHODS( FocusListenerMultiplexer, css::awt::XFocusListener )
IMPL_LISTENERMULTIPLEXER_LISTENERMETHOD( FocusListenerMultiplexer, css::awt::XFocusListener, focusGained, css::awt::FocusEvent )
IMPL_LISTENERMULTIPLEXER_LISTENERMETHOD( FocusListenerMultiplexer, css::awt::XFocusListener, focusLost, css::awt::FocusEvent )


//  class WindowListenerMultiplexer

IMPL_LISTENERMULTIPLEXER_BASEMETHODS( WindowListenerMultiplexer, css::awt::XWindowListener )
IMPL_LISTENERMULTIPLEXER_LISTENERMETHOD( WindowListenerMultiplexer, css::awt::XWindowListener, windowResized, css::awt::WindowEvent )
IMPL_LISTENERMULTIPLEXER_LISTENERMETHOD( WindowListenerMultiplexer, css::awt::XWindowListener, windowMoved, css::awt::WindowEvent )
IMPL_LISTENERMULTIPLEXER_LISTENERMETHOD( WindowListenerMultiplexer, css::awt::XWindowListener, windowShown, css::lang::EventObject )
IMPL_LISTENERMULTIPLEXER_LISTENERMETHOD( WindowListenerMultiplexer, css::awt::XWindowListener, windowHidden, css::lang::EventObject )


//  class VclContainerListenerMultiplexer

IMPL_LISTENERMULTIPLEXER_BASEMETHODS( VclContainerListenerMultiplexer, css::awt::XVclContainerListener )
IMPL_LISTENERMULTIPLEXER_LISTENERMETHOD( VclContainerListenerMultiplexer, css::awt::XVclContainerListener, windowAdded, css::awt::VclContainerEvent )
IMPL_LISTENERMULTIPLEXER_LISTENERMETHOD( VclContainerListenerMultiplexer, css::awt::XVclContainerListener, windowRemoved, css::awt::VclContainerEvent )


//  class KeyListenerMultiplexer

IMPL_LISTENERMULTIPLEXER_BASEMETHODS( KeyListenerMultiplexer, css::awt::XKeyListener )
IMPL_LISTENERMULTIPLEXER_LISTENERMETHOD( KeyListenerMultiplexer, css::awt::XKeyListener, keyPressed, css::awt::KeyEvent )
IMPL_LISTENERMULTIPLEXER_LISTENERMETHOD( KeyListenerMultiplexer, css::awt::XKeyListener, keyReleased, css::awt::KeyEvent )


//  class MouseListenerMultiplexer

IMPL_LISTENERMULTIPLEXER_BASEMETHODS( MouseListenerMultiplexer, css::awt::XMouseListener )
IMPL_LISTENERMULTIPLEXER_LISTENERMETHOD( MouseListenerMultiplexer, css::awt::XMouseListener, mousePressed, css::awt::MouseEvent )
IMPL_LISTENERMULTIPLEXER_LISTENERMETHOD( MouseListenerMultiplexer, css::awt::XMouseListener, mouseReleased, css::awt::MouseEvent )
IMPL_LISTENERMULTIPLEXER_LISTENERMETHOD( MouseListenerMultiplexer, css::awt::XMouseListener, mouseEntered, css::awt::MouseEvent )
IMPL_LISTENERMULTIPLEXER_LISTENERMETHOD( MouseListenerMultiplexer, css::awt::XMouseListener, mouseExited, css::awt::MouseEvent )


//  class MouseMotionListenerMultiplexer

IMPL_LISTENERMULTIPLEXER_BASEMETHODS( MouseMotionListenerMultiplexer, css::awt::XMouseMotionListener )
IMPL_LISTENERMULTIPLEXER_LISTENERMETHOD( MouseMotionListenerMultiplexer, css::awt::XMouseMotionListener, mouseDragged, css::awt::MouseEvent )
IMPL_LISTENERMULTIPLEXER_LISTENERMETHOD( MouseMotionListenerMultiplexer, css::awt::XMouseMotionListener, mouseMoved, css::awt::MouseEvent )


//  class PaintListenerMultiplexer

IMPL_LISTENERMULTIPLEXER_BASEMETHODS( PaintListenerMultiplexer, css::awt::XPaintListener )
IMPL_LISTENERMULTIPLEXER_LISTENERMETHOD( PaintListenerMultiplexer, css::awt::XPaintListener, windowPaint, css::awt::PaintEvent )


//  class TopWindowListenerMultiplexer

IMPL_LISTENERMULTIPLEXER_BASEMETHODS( TopWindowListenerMultiplexer, css::awt::XTopWindowListener )
IMPL_LISTENERMULTIPLEXER_LISTENERMETHOD( TopWindowListenerMultiplexer, css::awt::XTopWindowListener, windowOpened, css::lang::EventObject )
IMPL_LISTENERMULTIPLEXER_LISTENERMETHOD( TopWindowListenerMultiplexer, css::awt::XTopWindowListener, windowClosing, css::lang::EventObject )
IMPL_LISTENERMULTIPLEXER_LISTENERMETHOD( TopWindowListenerMultiplexer, css::awt::XTopWindowListener, windowClosed, css::lang::EventObject )
IMPL_LISTENERMULTIPLEXER_LISTENERMETHOD( TopWindowListenerMultiplexer, css::awt::XTopWindowListener, windowMinimized, css::lang::EventObject )
IMPL_LISTENERMULTIPLEXER_LISTENERMETHOD( TopWindowListenerMultiplexer, css::awt::XTopWindowListener, windowNormalized, css::lang::EventObject )
IMPL_LISTENERMULTIPLEXER_LISTENERMETHOD( TopWindowListenerMultiplexer, css::awt::XTopWindowListener, windowActivated, css::lang::EventObject )
IMPL_LISTENERMULTIPLEXER_LISTENERMETHOD( TopWindowListenerMultiplexer, css::awt::XTopWindowListener, windowDeactivated, css::lang::EventObject )


//  class TextListenerMultiplexer

IMPL_LISTENERMULTIPLEXER_BASEMETHODS( TextListenerMultiplexer, css::awt::XTextListener )
IMPL_LISTENERMULTIPLEXER_LISTENERMETHOD( TextListenerMultiplexer, css::awt::XTextListener, textChanged, css::awt::TextEvent )


//  class ActionListenerMultiplexer

IMPL_LISTENERMULTIPLEXER_BASEMETHODS( ActionListenerMultiplexer, css::awt::XActionListener )
IMPL_LISTENERMULTIPLEXER_LISTENERMETHOD( ActionListenerMultiplexer, css::awt::XActionListener, actionPerformed, css::awt::ActionEvent )


//  class ItemListenerMultiplexer

IMPL_LISTENERMULTIPLEXER_BASEMETHODS( ItemListenerMultiplexer, css::awt::XItemListener )
IMPL_LISTENERMULTIPLEXER_LISTENERMETHOD( ItemListenerMultiplexer, css::awt::XItemListener, itemStateChanged, css::awt::ItemEvent )


//  class TabListenerMultiplexer

IMPL_LISTENERMULTIPLEXER_BASEMETHODS( TabListenerMultiplexer, css::awt::XTabListener )

void TabListenerMultiplexer::inserted( sal_Int32 evt )
IMPL_TABLISTENERMULTIPLEXER_LISTENERMETHOD_BODY_1PARAM( TabListenerMultiplexer, css::awt::XTabListener, inserted, ::sal_Int32 )

void TabListenerMultiplexer::removed( sal_Int32 evt )
IMPL_TABLISTENERMULTIPLEXER_LISTENERMETHOD_BODY_1PARAM( TabListenerMultiplexer, css::awt::XTabListener, removed, ::sal_Int32 )

void TabListenerMultiplexer::changed( sal_Int32 evt, const css::uno::Sequence< css::beans::NamedValue >& evt2 )
{
    sal_Int32 aMulti( evt );
    ::comphelper::OInterfaceIteratorHelper2 aIt( *this );
    while( aIt.hasMoreElements() )
    {
        css::uno::Reference< css::awt::XTabListener > xListener(
            static_cast< css::awt::XTabListener* >( aIt.next() ) );
        try
        {
            xListener->changed( aMulti, evt2 );
        }
        catch(const css::lang::DisposedException& e)
        {
            OSL_ENSURE( e.Context.is(), "caught DisposedException with empty Context field" );
            if ( e.Context == xListener || !e.Context.is() )
                aIt.remove();
        }
        catch(const css::uno::RuntimeException&)
        {
            DISPLAY_EXCEPTION( TabListenerMultiplexer, changed )
        }
    }
}


void TabListenerMultiplexer::activated( sal_Int32 evt )
IMPL_TABLISTENERMULTIPLEXER_LISTENERMETHOD_BODY_1PARAM( TabListenerMultiplexer, css::awt::XTabListener, activated, ::sal_Int32 )

void TabListenerMultiplexer::deactivated( sal_Int32 evt )
IMPL_TABLISTENERMULTIPLEXER_LISTENERMETHOD_BODY_1PARAM( TabListenerMultiplexer, css::awt::XTabListener, deactivated, ::sal_Int32 )


//  class ContainerListenerMultiplexer

IMPL_LISTENERMULTIPLEXER_BASEMETHODS( ContainerListenerMultiplexer, css::container::XContainerListener )
IMPL_LISTENERMULTIPLEXER_LISTENERMETHOD( ContainerListenerMultiplexer, css::container::XContainerListener, elementInserted, css::container::ContainerEvent )
IMPL_LISTENERMULTIPLEXER_LISTENERMETHOD( ContainerListenerMultiplexer, css::container::XContainerListener, elementRemoved, css::container::ContainerEvent )
IMPL_LISTENERMULTIPLEXER_LISTENERMETHOD( ContainerListenerMultiplexer, css::container::XContainerListener, elementReplaced, css::container::ContainerEvent )


//  class SpinListenerMultiplexer

IMPL_LISTENERMULTIPLEXER_BASEMETHODS( SpinListenerMultiplexer, css::awt::XSpinListener )
IMPL_LISTENERMULTIPLEXER_LISTENERMETHOD( SpinListenerMultiplexer, css::awt::XSpinListener, up, css::awt::SpinEvent )
IMPL_LISTENERMULTIPLEXER_LISTENERMETHOD( SpinListenerMultiplexer, css::awt::XSpinListener, down, css::awt::SpinEvent )
IMPL_LISTENERMULTIPLEXER_LISTENERMETHOD( SpinListenerMultiplexer, css::awt::XSpinListener, first, css::awt::SpinEvent )
IMPL_LISTENERMULTIPLEXER_LISTENERMETHOD( SpinListenerMultiplexer, css::awt::XSpinListener, last, css::awt::SpinEvent )


//  class AdjustmentListenerMultiplexer

IMPL_LISTENERMULTIPLEXER_BASEMETHODS( AdjustmentListenerMultiplexer, css::awt::XAdjustmentListener )
IMPL_LISTENERMULTIPLEXER_LISTENERMETHOD( AdjustmentListenerMultiplexer, css::awt::XAdjustmentListener, adjustmentValueChanged, css::awt::AdjustmentEvent )


//  class MenuListenerMultiplexer

IMPL_LISTENERMULTIPLEXER_BASEMETHODS( MenuListenerMultiplexer, css::awt::XMenuListener )
IMPL_LISTENERMULTIPLEXER_LISTENERMETHOD( MenuListenerMultiplexer, css::awt::XMenuListener, itemHighlighted, css::awt::MenuEvent )
IMPL_LISTENERMULTIPLEXER_LISTENERMETHOD( MenuListenerMultiplexer, css::awt::XMenuListener, itemSelected, css::awt::MenuEvent )
IMPL_LISTENERMULTIPLEXER_LISTENERMETHOD( MenuListenerMultiplexer, css::awt::XMenuListener, itemActivated, css::awt::MenuEvent )
IMPL_LISTENERMULTIPLEXER_LISTENERMETHOD( MenuListenerMultiplexer, css::awt::XMenuListener, itemDeactivated, css::awt::MenuEvent )


//  class TreeSelectionListenerMultiplexer

IMPL_LISTENERMULTIPLEXER_BASEMETHODS( TreeSelectionListenerMultiplexer, css::view::XSelectionChangeListener )
IMPL_LISTENERMULTIPLEXER_LISTENERMETHOD( TreeSelectionListenerMultiplexer, css::view::XSelectionChangeListener, selectionChanged,  css::lang::EventObject )


//  class TreeSelectionListenerMultiplexer

IMPL_LISTENERMULTIPLEXER_BASEMETHODS( TreeExpansionListenerMultiplexer, css::awt::tree::XTreeExpansionListener )
IMPL_LISTENERMULTIPLEXER_LISTENERMETHOD( TreeExpansionListenerMultiplexer, css::awt::tree::XTreeExpansionListener, requestChildNodes, css::awt::tree::TreeExpansionEvent )
IMPL_LISTENERMULTIPLEXER_LISTENERMETHOD_EXCEPTION( TreeExpansionListenerMultiplexer, css::awt::tree::XTreeExpansionListener, treeExpanding, css::awt::tree::TreeExpansionEvent, css::awt::tree::ExpandVetoException )
IMPL_LISTENERMULTIPLEXER_LISTENERMETHOD_EXCEPTION( TreeExpansionListenerMultiplexer, css::awt::tree::XTreeExpansionListener, treeCollapsing, css::awt::tree::TreeExpansionEvent, css::awt::tree::ExpandVetoException )
IMPL_LISTENERMULTIPLEXER_LISTENERMETHOD( TreeExpansionListenerMultiplexer, css::awt::tree::XTreeExpansionListener, treeExpanded, css::awt::tree::TreeExpansionEvent )
IMPL_LISTENERMULTIPLEXER_LISTENERMETHOD( TreeExpansionListenerMultiplexer, css::awt::tree::XTreeExpansionListener, treeCollapsed, css::awt::tree::TreeExpansionEvent )


//  class TreeEditListenerMultiplexer

IMPL_LISTENERMULTIPLEXER_BASEMETHODS( TreeEditListenerMultiplexer, css::awt::tree::XTreeEditListener )


//  class SelectionListenerMultiplexer

IMPL_LISTENERMULTIPLEXER_BASEMETHODS( SelectionListenerMultiplexer, css::awt::grid::XGridSelectionListener )
IMPL_LISTENERMULTIPLEXER_LISTENERMETHOD( SelectionListenerMultiplexer, css::awt::grid::XGridSelectionListener, selectionChanged, css::awt::grid::GridSelectionEvent )


//  class SelectionListenerMultiplexer

IMPL_LISTENERMULTIPLEXER_BASEMETHODS( TabPageListenerMultiplexer, css::awt::tab::XTabPageContainerListener )
IMPL_LISTENERMULTIPLEXER_LISTENERMETHOD( TabPageListenerMultiplexer, css::awt::tab::XTabPageContainerListener, tabPageActivated, css::awt::tab::TabPageActivatedEvent )
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
