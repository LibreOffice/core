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

#ifndef INCLUDED_TOOLKIT_HELPER_LISTENERMULTIPLEXER_HXX
#define INCLUDED_TOOLKIT_HELPER_LISTENERMULTIPLEXER_HXX

#include <toolkit/dllapi.h>
#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/awt/XFocusListener.hpp>
#include <com/sun/star/awt/XWindowListener.hpp>
#include <com/sun/star/awt/XVclContainerListener.hpp>
#include <com/sun/star/awt/XKeyListener.hpp>
#include <com/sun/star/awt/XMouseListener.hpp>
#include <com/sun/star/awt/XMouseMotionListener.hpp>
#include <com/sun/star/awt/XPaintListener.hpp>
#include <com/sun/star/awt/XTopWindowListener.hpp>
#include <com/sun/star/awt/XTextListener.hpp>
#include <com/sun/star/awt/XActionListener.hpp>
#include <com/sun/star/awt/XItemListener.hpp>
#include <com/sun/star/awt/XTabListener.hpp>
#include <com/sun/star/container/XContainerListener.hpp>
#include <com/sun/star/awt/XSpinListener.hpp>
#include <com/sun/star/awt/XAdjustmentListener.hpp>
#include <com/sun/star/awt/XMenuListener.hpp>
#include <com/sun/star/awt/tree/XTreeExpansionListener.hpp>
#include <com/sun/star/awt/tree/XTreeEditListener.hpp>
#include <com/sun/star/view/XSelectionChangeListener.hpp>
#include <cppuhelper/weak.hxx>
#include <comphelper/interfacecontainer2.hxx>
#include <toolkit/helper/mutexhelper.hxx>
#include <toolkit/helper/macros.hxx>
#include <com/sun/star/awt/grid/XGridSelectionListener.hpp>
#include <com/sun/star/awt/tab/XTabPageContainerListener.hpp>

//  class ListenerMultiplexerBase


class TOOLKIT_DLLPUBLIC ListenerMultiplexerBase : public MutexHelper,
                                public ::comphelper::OInterfaceContainerHelper2,
                                public css::uno::XInterface
{
private:
    ::cppu::OWeakObject&    mrContext;

protected:
    ::cppu::OWeakObject&    GetContext() { return mrContext; }

public:
    ListenerMultiplexerBase( ::cppu::OWeakObject& rSource );
    virtual ~ListenerMultiplexerBase();

    // css::uno::XInterface
    css::uno::Any  SAL_CALL queryInterface( const css::uno::Type & rType ) override;
    void                        SAL_CALL acquire() throw() override  { mrContext.acquire(); }
    void                        SAL_CALL release() throw() override  { mrContext.release(); }
};


//  class EventListenerMultiplexer

DECL_LISTENERMULTIPLEXER_START( EventListenerMultiplexer, css::lang::XEventListener )
DECL_LISTENERMULTIPLEXER_END


//  class FocusListenerMultiplexer

DECL_LISTENERMULTIPLEXER_START_DLLPUB( FocusListenerMultiplexer, css::awt::XFocusListener )
    void SAL_CALL focusGained( const css::awt::FocusEvent& e ) override;
    void SAL_CALL focusLost( const css::awt::FocusEvent& e ) override;
DECL_LISTENERMULTIPLEXER_END


//  class WindowListenerMultiplexer

DECL_LISTENERMULTIPLEXER_START_DLLPUB( WindowListenerMultiplexer, css::awt::XWindowListener )
    void SAL_CALL windowResized( const css::awt::WindowEvent& e ) override;
    void SAL_CALL windowMoved( const css::awt::WindowEvent& e ) override;
    void SAL_CALL windowShown( const css::lang::EventObject& e ) override;
    void SAL_CALL windowHidden( const css::lang::EventObject& e ) override;
DECL_LISTENERMULTIPLEXER_END


//  class VclContainerListenerMultiplexer

DECL_LISTENERMULTIPLEXER_START( VclContainerListenerMultiplexer, css::awt::XVclContainerListener )
    void SAL_CALL windowAdded( const css::awt::VclContainerEvent& e ) override;
    void SAL_CALL windowRemoved( const css::awt::VclContainerEvent& e ) override;
DECL_LISTENERMULTIPLEXER_END


//  class KeyListenerMultiplexer

DECL_LISTENERMULTIPLEXER_START_DLLPUB( KeyListenerMultiplexer, css::awt::XKeyListener )
    void SAL_CALL keyPressed( const css::awt::KeyEvent& e ) override;
    void SAL_CALL keyReleased( const css::awt::KeyEvent& e ) override;
DECL_LISTENERMULTIPLEXER_END


//  class MouseListenerMultiplexer

DECL_LISTENERMULTIPLEXER_START_DLLPUB( MouseListenerMultiplexer, css::awt::XMouseListener )
    void SAL_CALL mousePressed( const css::awt::MouseEvent& e ) override;
    void SAL_CALL mouseReleased( const css::awt::MouseEvent& e ) override;
    void SAL_CALL mouseEntered( const css::awt::MouseEvent& e ) override;
    void SAL_CALL mouseExited( const css::awt::MouseEvent& e ) override;
DECL_LISTENERMULTIPLEXER_END


//  class MouseMotionListenerMultiplexer

DECL_LISTENERMULTIPLEXER_START_DLLPUB( MouseMotionListenerMultiplexer, css::awt::XMouseMotionListener )
    void SAL_CALL mouseDragged( const css::awt::MouseEvent& e ) override;
    void SAL_CALL mouseMoved( const css::awt::MouseEvent& e ) override;
DECL_LISTENERMULTIPLEXER_END


//  class PaintListenerMultiplexer

DECL_LISTENERMULTIPLEXER_START_DLLPUB( PaintListenerMultiplexer, css::awt::XPaintListener )
    void SAL_CALL windowPaint( const css::awt::PaintEvent& e ) override;
DECL_LISTENERMULTIPLEXER_END


//  class TopWindowListenerMultiplexer

DECL_LISTENERMULTIPLEXER_START( TopWindowListenerMultiplexer, css::awt::XTopWindowListener )
    void SAL_CALL windowOpened( const css::lang::EventObject& e ) override;
    void SAL_CALL windowClosing( const css::lang::EventObject& e ) override;
    void SAL_CALL windowClosed( const css::lang::EventObject& e ) override;
    void SAL_CALL windowMinimized( const css::lang::EventObject& e ) override;
    void SAL_CALL windowNormalized( const css::lang::EventObject& e ) override;
    void SAL_CALL windowActivated( const css::lang::EventObject& e ) override;
    void SAL_CALL windowDeactivated( const css::lang::EventObject& e ) override;
DECL_LISTENERMULTIPLEXER_END


//  class TextListenerMultiplexer

DECL_LISTENERMULTIPLEXER_START_DLLPUB( TextListenerMultiplexer, css::awt::XTextListener )
    void SAL_CALL textChanged( const css::awt::TextEvent& rEvent ) override;
DECL_LISTENERMULTIPLEXER_END


//  class ActionListenerMultiplexer

DECL_LISTENERMULTIPLEXER_START_DLLPUB( ActionListenerMultiplexer, css::awt::XActionListener )
    void SAL_CALL actionPerformed( const css::awt::ActionEvent& rEvent ) override;
DECL_LISTENERMULTIPLEXER_END


//  class ItemListenerMultiplexer

DECL_LISTENERMULTIPLEXER_START_DLLPUB( ItemListenerMultiplexer, css::awt::XItemListener )
    void SAL_CALL itemStateChanged( const css::awt::ItemEvent& rEvent ) override;
DECL_LISTENERMULTIPLEXER_END


//  class TabListenerMultiplexer

DECL_LISTENERMULTIPLEXER_START_DLLPUB( TabListenerMultiplexer, css::awt::XTabListener )
    void SAL_CALL inserted( ::sal_Int32 ID ) override;
    void SAL_CALL removed( ::sal_Int32 ID ) override;
    void SAL_CALL changed( ::sal_Int32 ID, const css::uno::Sequence< css::beans::NamedValue >& Properties ) override;
    void SAL_CALL activated( ::sal_Int32 ID ) override;
    void SAL_CALL deactivated( ::sal_Int32 ID ) override;
DECL_LISTENERMULTIPLEXER_END


//  class ContainerListenerMultiplexer

DECL_LISTENERMULTIPLEXER_START( ContainerListenerMultiplexer, css::container::XContainerListener )
    void SAL_CALL elementInserted( const css::container::ContainerEvent& Event ) override;
    void SAL_CALL elementRemoved( const css::container::ContainerEvent& Event ) override;
    void SAL_CALL elementReplaced( const css::container::ContainerEvent& Event ) override;
DECL_LISTENERMULTIPLEXER_END


//  class SpinListenerMultiplexer

DECL_LISTENERMULTIPLEXER_START_DLLPUB( SpinListenerMultiplexer, css::awt::XSpinListener )
    void SAL_CALL up( const css::awt::SpinEvent& rEvent ) override;
    void SAL_CALL down( const css::awt::SpinEvent& rEvent ) override;
    void SAL_CALL first( const css::awt::SpinEvent& rEvent ) override;
    void SAL_CALL last( const css::awt::SpinEvent& rEvent ) override;
DECL_LISTENERMULTIPLEXER_END


//  class AdjustmentListenerMultiplexer

DECL_LISTENERMULTIPLEXER_START( AdjustmentListenerMultiplexer, css::awt::XAdjustmentListener )
    void SAL_CALL adjustmentValueChanged( const css::awt::AdjustmentEvent& rEvent ) override;
DECL_LISTENERMULTIPLEXER_END


//  class MenuListenerMultiplexer

DECL_LISTENERMULTIPLEXER_START( MenuListenerMultiplexer, css::awt::XMenuListener )
    void SAL_CALL itemHighlighted( const css::awt::MenuEvent& rEvent ) override;
    void SAL_CALL itemSelected( const css::awt::MenuEvent& rEvent ) override;
    void SAL_CALL itemActivated( const css::awt::MenuEvent& rEvent ) override;
    void SAL_CALL itemDeactivated( const css::awt::MenuEvent& rEvent ) override;
DECL_LISTENERMULTIPLEXER_END


//  class TreeSelectionListenerMultiplexer

DECL_LISTENERMULTIPLEXER_START_DLLPUB( TreeSelectionListenerMultiplexer, css::view::XSelectionChangeListener )
    virtual void SAL_CALL selectionChanged( const css::lang::EventObject& aEvent ) override;
DECL_LISTENERMULTIPLEXER_END


//  class TreeExpansionListenerMultiplexer

DECL_LISTENERMULTIPLEXER_START_DLLPUB( TreeExpansionListenerMultiplexer, css::awt::tree::XTreeExpansionListener )
    virtual void SAL_CALL requestChildNodes( const css::awt::tree::TreeExpansionEvent& aEvent ) override;
    virtual void SAL_CALL treeExpanding( const css::awt::tree::TreeExpansionEvent& aEvent ) override;
    virtual void SAL_CALL treeCollapsing( const css::awt::tree::TreeExpansionEvent& aEvent ) override;
    virtual void SAL_CALL treeExpanded( const css::awt::tree::TreeExpansionEvent& aEvent ) override;
    virtual void SAL_CALL treeCollapsed( const css::awt::tree::TreeExpansionEvent& aEvent ) override;
DECL_LISTENERMULTIPLEXER_END


//  class TreeEditListenerMultiplexer

DECL_LISTENERMULTIPLEXER_START_DLLPUB( TreeEditListenerMultiplexer, css::awt::tree::XTreeEditListener )
    virtual void SAL_CALL nodeEditing( const css::uno::Reference< css::awt::tree::XTreeNode >& Node ) override;
    virtual void SAL_CALL nodeEdited( const css::uno::Reference< css::awt::tree::XTreeNode >& Node, const OUString& NewText ) override;
DECL_LISTENERMULTIPLEXER_END


//  class SelectionListenerMultiplexer

DECL_LISTENERMULTIPLEXER_START_DLLPUB( SelectionListenerMultiplexer, css::awt::grid::XGridSelectionListener )
    void SAL_CALL selectionChanged( const css::awt::grid::GridSelectionEvent& aEvent ) override;
DECL_LISTENERMULTIPLEXER_END


//  class TabPageListenerMultiplexer

DECL_LISTENERMULTIPLEXER_START_DLLPUB( TabPageListenerMultiplexer, css::awt::tab::XTabPageContainerListener )
    void SAL_CALL tabPageActivated( const css::awt::tab::TabPageActivatedEvent& aEvent ) override;
DECL_LISTENERMULTIPLEXER_END

#endif // INCLUDED_TOOLKIT_HELPER_LISTENERMULTIPLEXER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
