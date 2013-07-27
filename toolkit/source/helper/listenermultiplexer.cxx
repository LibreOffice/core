/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
//#include "precompiled_toolkit.hxx"
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
IMPL_LISTENERMULTIPLEXER_LISTENERMETHOD( MenuListenerMultiplexer, ::com::sun::star::awt::XMenuListener, itemHighlighted, ::com::sun::star::awt::MenuEvent )
IMPL_LISTENERMULTIPLEXER_LISTENERMETHOD( MenuListenerMultiplexer, ::com::sun::star::awt::XMenuListener, itemSelected, ::com::sun::star::awt::MenuEvent )
IMPL_LISTENERMULTIPLEXER_LISTENERMETHOD( MenuListenerMultiplexer, ::com::sun::star::awt::XMenuListener, itemActivated, ::com::sun::star::awt::MenuEvent )
IMPL_LISTENERMULTIPLEXER_LISTENERMETHOD( MenuListenerMultiplexer, ::com::sun::star::awt::XMenuListener, itemDeactivated, ::com::sun::star::awt::MenuEvent )

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
