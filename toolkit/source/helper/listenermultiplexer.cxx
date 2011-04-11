/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#include "precompiled_toolkit.hxx"
#include <toolkit/helper/listenermultiplexer.hxx>
#include <com/sun/star/lang/DisposedException.hpp>

//  ----------------------------------------------------
//  class ListenerMultiplexerBase
//  ----------------------------------------------------
ListenerMultiplexerBase::ListenerMultiplexerBase( ::cppu::OWeakObject& rSource )
    : ::cppu::OInterfaceContainerHelper( GetMutex() ), mrContext( rSource )
{
}

ListenerMultiplexerBase::~ListenerMultiplexerBase()
{
}

// ::com::sun::star::uno::XInterface
::com::sun::star::uno::Any ListenerMultiplexerBase::queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException)
{
    return ::cppu::queryInterface( rType, SAL_STATIC_CAST( ::com::sun::star::uno::XInterface*, this ) );
}


//  ----------------------------------------------------
//  class EventListenerMultiplexer
//  ----------------------------------------------------
EventListenerMultiplexer::EventListenerMultiplexer( ::cppu::OWeakObject& rSource )
    : ListenerMultiplexerBase( rSource )
{
}

// ::com::sun::star::uno::XInterface
::com::sun::star::uno::Any EventListenerMultiplexer::queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aRet = ::cppu::queryInterface( rType,
                                        SAL_STATIC_CAST( ::com::sun::star::lang::XEventListener*, this ) );
    return (aRet.hasValue() ? aRet : ListenerMultiplexerBase::queryInterface( rType ));
}

// ::com::sun::star::lang::XEventListener
void EventListenerMultiplexer::disposing( const ::com::sun::star::lang::EventObject& ) throw(::com::sun::star::uno::RuntimeException)
{
}

//  ----------------------------------------------------
//  class FocusListenerMultiplexer
//  ----------------------------------------------------
IMPL_LISTENERMULTIPLEXER_BASEMETHODS( FocusListenerMultiplexer, ::com::sun::star::awt::XFocusListener )
IMPL_LISTENERMULTIPLEXER_LISTENERMETHOD( FocusListenerMultiplexer, ::com::sun::star::awt::XFocusListener, focusGained, ::com::sun::star::awt::FocusEvent )
IMPL_LISTENERMULTIPLEXER_LISTENERMETHOD( FocusListenerMultiplexer, ::com::sun::star::awt::XFocusListener, focusLost, ::com::sun::star::awt::FocusEvent )

//  ----------------------------------------------------
//  class WindowListenerMultiplexer
//  ----------------------------------------------------
IMPL_LISTENERMULTIPLEXER_BASEMETHODS( WindowListenerMultiplexer, ::com::sun::star::awt::XWindowListener )
IMPL_LISTENERMULTIPLEXER_LISTENERMETHOD( WindowListenerMultiplexer, ::com::sun::star::awt::XWindowListener, windowResized, ::com::sun::star::awt::WindowEvent )
IMPL_LISTENERMULTIPLEXER_LISTENERMETHOD( WindowListenerMultiplexer, ::com::sun::star::awt::XWindowListener, windowMoved, ::com::sun::star::awt::WindowEvent )
IMPL_LISTENERMULTIPLEXER_LISTENERMETHOD( WindowListenerMultiplexer, ::com::sun::star::awt::XWindowListener, windowShown, ::com::sun::star::lang::EventObject )
IMPL_LISTENERMULTIPLEXER_LISTENERMETHOD( WindowListenerMultiplexer, ::com::sun::star::awt::XWindowListener, windowHidden, ::com::sun::star::lang::EventObject )

//  ----------------------------------------------------
//  class VclContainerListenerMultiplexer
//  ----------------------------------------------------
IMPL_LISTENERMULTIPLEXER_BASEMETHODS( VclContainerListenerMultiplexer, ::com::sun::star::awt::XVclContainerListener )
IMPL_LISTENERMULTIPLEXER_LISTENERMETHOD( VclContainerListenerMultiplexer, ::com::sun::star::awt::XVclContainerListener, windowAdded, ::com::sun::star::awt::VclContainerEvent )
IMPL_LISTENERMULTIPLEXER_LISTENERMETHOD( VclContainerListenerMultiplexer, ::com::sun::star::awt::XVclContainerListener, windowRemoved, ::com::sun::star::awt::VclContainerEvent )

//  ----------------------------------------------------
//  class KeyListenerMultiplexer
//  ----------------------------------------------------
IMPL_LISTENERMULTIPLEXER_BASEMETHODS( KeyListenerMultiplexer, ::com::sun::star::awt::XKeyListener )
IMPL_LISTENERMULTIPLEXER_LISTENERMETHOD( KeyListenerMultiplexer, ::com::sun::star::awt::XKeyListener, keyPressed, ::com::sun::star::awt::KeyEvent )
IMPL_LISTENERMULTIPLEXER_LISTENERMETHOD( KeyListenerMultiplexer, ::com::sun::star::awt::XKeyListener, keyReleased, ::com::sun::star::awt::KeyEvent )

//  ----------------------------------------------------
//  class MouseListenerMultiplexer
//  ----------------------------------------------------
IMPL_LISTENERMULTIPLEXER_BASEMETHODS( MouseListenerMultiplexer, ::com::sun::star::awt::XMouseListener )
IMPL_LISTENERMULTIPLEXER_LISTENERMETHOD( MouseListenerMultiplexer, ::com::sun::star::awt::XMouseListener, mousePressed, ::com::sun::star::awt::MouseEvent )
IMPL_LISTENERMULTIPLEXER_LISTENERMETHOD( MouseListenerMultiplexer, ::com::sun::star::awt::XMouseListener, mouseReleased, ::com::sun::star::awt::MouseEvent )
IMPL_LISTENERMULTIPLEXER_LISTENERMETHOD( MouseListenerMultiplexer, ::com::sun::star::awt::XMouseListener, mouseEntered, ::com::sun::star::awt::MouseEvent )
IMPL_LISTENERMULTIPLEXER_LISTENERMETHOD( MouseListenerMultiplexer, ::com::sun::star::awt::XMouseListener, mouseExited, ::com::sun::star::awt::MouseEvent )

//  ----------------------------------------------------
//  class MouseMotionListenerMultiplexer
//  ----------------------------------------------------
IMPL_LISTENERMULTIPLEXER_BASEMETHODS( MouseMotionListenerMultiplexer, ::com::sun::star::awt::XMouseMotionListener )
IMPL_LISTENERMULTIPLEXER_LISTENERMETHOD( MouseMotionListenerMultiplexer, ::com::sun::star::awt::XMouseMotionListener, mouseDragged, ::com::sun::star::awt::MouseEvent )
IMPL_LISTENERMULTIPLEXER_LISTENERMETHOD( MouseMotionListenerMultiplexer, ::com::sun::star::awt::XMouseMotionListener, mouseMoved, ::com::sun::star::awt::MouseEvent )

//  ----------------------------------------------------
//  class PaintListenerMultiplexer
//  ----------------------------------------------------
IMPL_LISTENERMULTIPLEXER_BASEMETHODS( PaintListenerMultiplexer, ::com::sun::star::awt::XPaintListener )
IMPL_LISTENERMULTIPLEXER_LISTENERMETHOD( PaintListenerMultiplexer, ::com::sun::star::awt::XPaintListener, windowPaint, ::com::sun::star::awt::PaintEvent )

//  ----------------------------------------------------
//  class TopWindowListenerMultiplexer
//  ----------------------------------------------------
IMPL_LISTENERMULTIPLEXER_BASEMETHODS( TopWindowListenerMultiplexer, ::com::sun::star::awt::XTopWindowListener )
IMPL_LISTENERMULTIPLEXER_LISTENERMETHOD( TopWindowListenerMultiplexer, ::com::sun::star::awt::XTopWindowListener, windowOpened, ::com::sun::star::lang::EventObject )
IMPL_LISTENERMULTIPLEXER_LISTENERMETHOD( TopWindowListenerMultiplexer, ::com::sun::star::awt::XTopWindowListener, windowClosing, ::com::sun::star::lang::EventObject )
IMPL_LISTENERMULTIPLEXER_LISTENERMETHOD( TopWindowListenerMultiplexer, ::com::sun::star::awt::XTopWindowListener, windowClosed, ::com::sun::star::lang::EventObject )
IMPL_LISTENERMULTIPLEXER_LISTENERMETHOD( TopWindowListenerMultiplexer, ::com::sun::star::awt::XTopWindowListener, windowMinimized, ::com::sun::star::lang::EventObject )
IMPL_LISTENERMULTIPLEXER_LISTENERMETHOD( TopWindowListenerMultiplexer, ::com::sun::star::awt::XTopWindowListener, windowNormalized, ::com::sun::star::lang::EventObject )
IMPL_LISTENERMULTIPLEXER_LISTENERMETHOD( TopWindowListenerMultiplexer, ::com::sun::star::awt::XTopWindowListener, windowActivated, ::com::sun::star::lang::EventObject )
IMPL_LISTENERMULTIPLEXER_LISTENERMETHOD( TopWindowListenerMultiplexer, ::com::sun::star::awt::XTopWindowListener, windowDeactivated, ::com::sun::star::lang::EventObject )

//  ----------------------------------------------------
//  class TextListenerMultiplexer
//  ----------------------------------------------------
IMPL_LISTENERMULTIPLEXER_BASEMETHODS( TextListenerMultiplexer, ::com::sun::star::awt::XTextListener )
IMPL_LISTENERMULTIPLEXER_LISTENERMETHOD( TextListenerMultiplexer, ::com::sun::star::awt::XTextListener, textChanged, ::com::sun::star::awt::TextEvent )

//  ----------------------------------------------------
//  class ActionListenerMultiplexer
//  ----------------------------------------------------
IMPL_LISTENERMULTIPLEXER_BASEMETHODS( ActionListenerMultiplexer, ::com::sun::star::awt::XActionListener )
IMPL_LISTENERMULTIPLEXER_LISTENERMETHOD( ActionListenerMultiplexer, ::com::sun::star::awt::XActionListener, actionPerformed, ::com::sun::star::awt::ActionEvent )

//  ----------------------------------------------------
//  class ItemListenerMultiplexer
//  ----------------------------------------------------
IMPL_LISTENERMULTIPLEXER_BASEMETHODS( ItemListenerMultiplexer, ::com::sun::star::awt::XItemListener )
IMPL_LISTENERMULTIPLEXER_LISTENERMETHOD( ItemListenerMultiplexer, ::com::sun::star::awt::XItemListener, itemStateChanged, ::com::sun::star::awt::ItemEvent )

//  ----------------------------------------------------
//  class TabListenerMultiplexer
//  ----------------------------------------------------
IMPL_LISTENERMULTIPLEXER_BASEMETHODS( TabListenerMultiplexer, ::com::sun::star::awt::XTabListener )
void TabListenerMultiplexer::inserted( sal_Int32 evt ) throw(::com::sun::star::uno::RuntimeException)
IMPL_TABLISTENERMULTIPLEXER_LISTENERMETHOD_BODY_1PARAM( TabListenerMultiplexer, ::com::sun::star::awt::XTabListener, inserted, ::sal_Int32 )
void TabListenerMultiplexer::removed( sal_Int32 evt ) throw(::com::sun::star::uno::RuntimeException)
IMPL_TABLISTENERMULTIPLEXER_LISTENERMETHOD_BODY_1PARAM( TabListenerMultiplexer, ::com::sun::star::awt::XTabListener, removed, ::sal_Int32 )
void TabListenerMultiplexer::changed( sal_Int32 evt, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue >& evt2 ) throw(::com::sun::star::uno::RuntimeException)
IMPL_TABLISTENERMULTIPLEXER_LISTENERMETHOD_BODY_2PARAM( TabListenerMultiplexer, ::com::sun::star::awt::XTabListener, changed, ::sal_Int32, ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue > )
void TabListenerMultiplexer::activated( sal_Int32 evt ) throw(::com::sun::star::uno::RuntimeException)
IMPL_TABLISTENERMULTIPLEXER_LISTENERMETHOD_BODY_1PARAM( TabListenerMultiplexer, ::com::sun::star::awt::XTabListener, activated, ::sal_Int32 )
void TabListenerMultiplexer::deactivated( sal_Int32 evt ) throw(::com::sun::star::uno::RuntimeException)
IMPL_TABLISTENERMULTIPLEXER_LISTENERMETHOD_BODY_1PARAM( TabListenerMultiplexer, ::com::sun::star::awt::XTabListener, deactivated, ::sal_Int32 )

//  ----------------------------------------------------
//  class ContainerListenerMultiplexer
//  ----------------------------------------------------
IMPL_LISTENERMULTIPLEXER_BASEMETHODS( ContainerListenerMultiplexer, ::com::sun::star::container::XContainerListener )
IMPL_LISTENERMULTIPLEXER_LISTENERMETHOD( ContainerListenerMultiplexer, ::com::sun::star::container::XContainerListener, elementInserted, ::com::sun::star::container::ContainerEvent )
IMPL_LISTENERMULTIPLEXER_LISTENERMETHOD( ContainerListenerMultiplexer, ::com::sun::star::container::XContainerListener, elementRemoved, ::com::sun::star::container::ContainerEvent )
IMPL_LISTENERMULTIPLEXER_LISTENERMETHOD( ContainerListenerMultiplexer, ::com::sun::star::container::XContainerListener, elementReplaced, ::com::sun::star::container::ContainerEvent )

//  ----------------------------------------------------
//  class SpinListenerMultiplexer
//  ----------------------------------------------------
IMPL_LISTENERMULTIPLEXER_BASEMETHODS( SpinListenerMultiplexer, ::com::sun::star::awt::XSpinListener )
IMPL_LISTENERMULTIPLEXER_LISTENERMETHOD( SpinListenerMultiplexer, ::com::sun::star::awt::XSpinListener, up, ::com::sun::star::awt::SpinEvent )
IMPL_LISTENERMULTIPLEXER_LISTENERMETHOD( SpinListenerMultiplexer, ::com::sun::star::awt::XSpinListener, down, ::com::sun::star::awt::SpinEvent )
IMPL_LISTENERMULTIPLEXER_LISTENERMETHOD( SpinListenerMultiplexer, ::com::sun::star::awt::XSpinListener, first, ::com::sun::star::awt::SpinEvent )
IMPL_LISTENERMULTIPLEXER_LISTENERMETHOD( SpinListenerMultiplexer, ::com::sun::star::awt::XSpinListener, last, ::com::sun::star::awt::SpinEvent )

//  ----------------------------------------------------
//  class AdjustmentListenerMultiplexer
//  ----------------------------------------------------
IMPL_LISTENERMULTIPLEXER_BASEMETHODS( AdjustmentListenerMultiplexer, ::com::sun::star::awt::XAdjustmentListener )
IMPL_LISTENERMULTIPLEXER_LISTENERMETHOD( AdjustmentListenerMultiplexer, ::com::sun::star::awt::XAdjustmentListener, adjustmentValueChanged, ::com::sun::star::awt::AdjustmentEvent )

//  ----------------------------------------------------
//  class MenuListenerMultiplexer
//  ----------------------------------------------------
IMPL_LISTENERMULTIPLEXER_BASEMETHODS( MenuListenerMultiplexer, ::com::sun::star::awt::XMenuListener )
IMPL_LISTENERMULTIPLEXER_LISTENERMETHOD( MenuListenerMultiplexer, ::com::sun::star::awt::XMenuListener, highlight, ::com::sun::star::awt::MenuEvent )
IMPL_LISTENERMULTIPLEXER_LISTENERMETHOD( MenuListenerMultiplexer, ::com::sun::star::awt::XMenuListener, select, ::com::sun::star::awt::MenuEvent )
IMPL_LISTENERMULTIPLEXER_LISTENERMETHOD( MenuListenerMultiplexer, ::com::sun::star::awt::XMenuListener, activate, ::com::sun::star::awt::MenuEvent )
IMPL_LISTENERMULTIPLEXER_LISTENERMETHOD( MenuListenerMultiplexer, ::com::sun::star::awt::XMenuListener, deactivate, ::com::sun::star::awt::MenuEvent )

//  ----------------------------------------------------
//  class TreeSelectionListenerMultiplexer
//  ----------------------------------------------------
IMPL_LISTENERMULTIPLEXER_BASEMETHODS( TreeSelectionListenerMultiplexer, ::com::sun::star::view::XSelectionChangeListener )
IMPL_LISTENERMULTIPLEXER_LISTENERMETHOD( TreeSelectionListenerMultiplexer, ::com::sun::star::view::XSelectionChangeListener, selectionChanged,  ::com::sun::star::lang::EventObject )

//  ----------------------------------------------------
//  class TreeSelectionListenerMultiplexer
//  ----------------------------------------------------
IMPL_LISTENERMULTIPLEXER_BASEMETHODS( TreeExpansionListenerMultiplexer, ::com::sun::star::awt::tree::XTreeExpansionListener )
IMPL_LISTENERMULTIPLEXER_LISTENERMETHOD( TreeExpansionListenerMultiplexer, ::com::sun::star::awt::tree::XTreeExpansionListener, requestChildNodes, ::com::sun::star::awt::tree::TreeExpansionEvent )
IMPL_LISTENERMULTIPLEXER_LISTENERMETHOD_EXCEPTION( TreeExpansionListenerMultiplexer, ::com::sun::star::awt::tree::XTreeExpansionListener, treeExpanding, ::com::sun::star::awt::tree::TreeExpansionEvent, ::com::sun::star::awt::tree::ExpandVetoException )
IMPL_LISTENERMULTIPLEXER_LISTENERMETHOD_EXCEPTION( TreeExpansionListenerMultiplexer, ::com::sun::star::awt::tree::XTreeExpansionListener, treeCollapsing, ::com::sun::star::awt::tree::TreeExpansionEvent, ::com::sun::star::awt::tree::ExpandVetoException )
IMPL_LISTENERMULTIPLEXER_LISTENERMETHOD( TreeExpansionListenerMultiplexer, ::com::sun::star::awt::tree::XTreeExpansionListener, treeExpanded, ::com::sun::star::awt::tree::TreeExpansionEvent )
IMPL_LISTENERMULTIPLEXER_LISTENERMETHOD( TreeExpansionListenerMultiplexer, ::com::sun::star::awt::tree::XTreeExpansionListener, treeCollapsed, ::com::sun::star::awt::tree::TreeExpansionEvent )

//  ----------------------------------------------------
//  class TreeEditListenerMultiplexer
//  ----------------------------------------------------
IMPL_LISTENERMULTIPLEXER_BASEMETHODS( TreeEditListenerMultiplexer, ::com::sun::star::awt::tree::XTreeEditListener )

//  ----------------------------------------------------
//  class SelectionListenerMultiplexer
//  ----------------------------------------------------
IMPL_LISTENERMULTIPLEXER_BASEMETHODS( SelectionListenerMultiplexer, ::com::sun::star::awt::grid::XGridSelectionListener )
IMPL_LISTENERMULTIPLEXER_LISTENERMETHOD( SelectionListenerMultiplexer, ::com::sun::star::awt::grid::XGridSelectionListener, selectionChanged, ::com::sun::star::awt::grid::GridSelectionEvent )

//  ----------------------------------------------------
//  class SelectionListenerMultiplexer
//  ----------------------------------------------------
IMPL_LISTENERMULTIPLEXER_BASEMETHODS( TabPageListenerMultiplexer, ::com::sun::star::awt::tab::XTabPageContainerListener )
IMPL_LISTENERMULTIPLEXER_LISTENERMETHOD( TabPageListenerMultiplexer, ::com::sun::star::awt::tab::XTabPageContainerListener, tabPageActivated, ::com::sun::star::awt::tab::TabPageActivatedEvent )
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */