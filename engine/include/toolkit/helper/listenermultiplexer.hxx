/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#pragma once

#include <config_options.h>
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
#include <cppuhelper/queryinterface.hxx>
#include <cppuhelper/weak.hxx>
#include <comphelper/interfacecontainer4.hxx>
#include <toolkit/helper/macros.hxx>
#include <com/sun/star/awt/grid/XGridSelectionListener.hpp>
#include <com/sun/star/awt/tab/XTabPageContainerListener.hpp>

//  class ListenerMultiplexerBase

template <class ListenerT>
class UNLESS_MERGELIBS(TOOLKIT_DLLPUBLIC) ListenerMultiplexerBase :
                                public css::uno::XInterface
{
private:
    ::cppu::OWeakObject&    mrContext;
protected:
    mutable std::mutex m_aMutex;
    ::comphelper::OInterfaceContainerHelper4<ListenerT> maListeners;

    ::cppu::OWeakObject&    GetContext() { return mrContext; }

public:
    ListenerMultiplexerBase( ::cppu::OWeakObject& rSource )
        : mrContext(rSource)
    {
    }

    virtual ~ListenerMultiplexerBase()
    {
    }

    // css::uno::XInterface
    cpo::uno::Any  queryInterface(const cpo::uno::Type & rType) override
    {
        return ::cppu::queryInterface(rType, static_cast<css::uno::XInterface*>(this));
    }

    void                        acquire() noexcept override  { mrContext.acquire(); }
    void                        release() noexcept override  { mrContext.release(); }

    void addInterface( const css::uno::Reference<ListenerT>& l)
    {
        std::unique_lock g(m_aMutex);
        maListeners.addInterface(g, l);
    }

    void removeInterface( const css::uno::Reference<ListenerT>& l)
    {
        std::unique_lock g(m_aMutex);
        maListeners.removeInterface(g, l);
    }

    void disposeAndClear(const css::lang::EventObject& rDisposeEvent)
    {
        std::unique_lock g(m_aMutex);
        maListeners.disposeAndClear(g, rDisposeEvent);
    }

    void disposeAndClear(std::unique_lock<std::mutex>& rGuard, const css::lang::EventObject& rDisposeEvent)
    {
        maListeners.disposeAndClear(rGuard, rDisposeEvent);
    }

    sal_Int32 getLength() const
    {
        std::unique_lock g(m_aMutex);
        return maListeners.getLength(g);
    }

    template <typename EventT>
    inline void notifyEach(void (ListenerT::*NotificationMethod)(const EventT&),
                           const EventT& Event) const
    {
        std::unique_lock g(m_aMutex);
        return maListeners.notifyEach(g, NotificationMethod, Event);
    }
};

//  class EventListenerMultiplexer

DECL_LISTENERMULTIPLEXER_START( EventListenerMultiplexer, css::lang::XEventListener )
DECL_LISTENERMULTIPLEXER_END


//  class FocusListenerMultiplexer

DECL_LISTENERMULTIPLEXER_START_DLLPUB( FocusListenerMultiplexer, css::awt::XFocusListener )
    void focusGained( const css::awt::FocusEvent& e ) override;
    void focusLost( const css::awt::FocusEvent& e ) override;
DECL_LISTENERMULTIPLEXER_END


//  class WindowListenerMultiplexer

DECL_LISTENERMULTIPLEXER_START_DLLPUB( WindowListenerMultiplexer, css::awt::XWindowListener )
    void windowResized( const css::awt::WindowEvent& e ) override;
    void windowMoved( const css::awt::WindowEvent& e ) override;
    void windowShown( const css::lang::EventObject& e ) override;
    void windowHidden( const css::lang::EventObject& e ) override;
DECL_LISTENERMULTIPLEXER_END


//  class VclContainerListenerMultiplexer

DECL_LISTENERMULTIPLEXER_START( VclContainerListenerMultiplexer, css::awt::XVclContainerListener )
    void windowAdded( const css::awt::VclContainerEvent& e ) override;
    void windowRemoved( const css::awt::VclContainerEvent& e ) override;
DECL_LISTENERMULTIPLEXER_END


//  class KeyListenerMultiplexer

DECL_LISTENERMULTIPLEXER_START_DLLPUB( KeyListenerMultiplexer, css::awt::XKeyListener )
    void keyPressed( const css::awt::KeyEvent& e ) override;
    void keyReleased( const css::awt::KeyEvent& e ) override;
DECL_LISTENERMULTIPLEXER_END


//  class MouseListenerMultiplexer

DECL_LISTENERMULTIPLEXER_START_DLLPUB( MouseListenerMultiplexer, css::awt::XMouseListener )
    void mousePressed( const css::awt::MouseEvent& e ) override;
    void mouseReleased( const css::awt::MouseEvent& e ) override;
    void mouseEntered( const css::awt::MouseEvent& e ) override;
    void mouseExited( const css::awt::MouseEvent& e ) override;
DECL_LISTENERMULTIPLEXER_END


//  class MouseMotionListenerMultiplexer

DECL_LISTENERMULTIPLEXER_START_DLLPUB( MouseMotionListenerMultiplexer, css::awt::XMouseMotionListener )
    void mouseDragged( const css::awt::MouseEvent& e ) override;
    void mouseMoved( const css::awt::MouseEvent& e ) override;
DECL_LISTENERMULTIPLEXER_END


//  class PaintListenerMultiplexer

DECL_LISTENERMULTIPLEXER_START_DLLPUB( PaintListenerMultiplexer, css::awt::XPaintListener )
    void windowPaint( const css::awt::PaintEvent& e ) override;
DECL_LISTENERMULTIPLEXER_END


//  class TopWindowListenerMultiplexer

DECL_LISTENERMULTIPLEXER_START( TopWindowListenerMultiplexer, css::awt::XTopWindowListener )
    void windowOpened( const css::lang::EventObject& e ) override;
    void windowClosing( const css::lang::EventObject& e ) override;
    void windowClosed( const css::lang::EventObject& e ) override;
    void windowMinimized( const css::lang::EventObject& e ) override;
    void windowNormalized( const css::lang::EventObject& e ) override;
    void windowActivated( const css::lang::EventObject& e ) override;
    void windowDeactivated( const css::lang::EventObject& e ) override;
DECL_LISTENERMULTIPLEXER_END


//  class TextListenerMultiplexer

DECL_LISTENERMULTIPLEXER_START_DLLPUB( TextListenerMultiplexer, css::awt::XTextListener )
    void textChanged( const css::awt::TextEvent& rEvent ) override;
DECL_LISTENERMULTIPLEXER_END


//  class ActionListenerMultiplexer

DECL_LISTENERMULTIPLEXER_START( ActionListenerMultiplexer, css::awt::XActionListener )
    void actionPerformed( const css::awt::ActionEvent& rEvent ) override;
DECL_LISTENERMULTIPLEXER_END


//  class ItemListenerMultiplexer

DECL_LISTENERMULTIPLEXER_START( ItemListenerMultiplexer, css::awt::XItemListener )
    void itemStateChanged( const css::awt::ItemEvent& rEvent ) override;
DECL_LISTENERMULTIPLEXER_END


//  class TabListenerMultiplexer

DECL_LISTENERMULTIPLEXER_START( TabListenerMultiplexer, css::awt::XTabListener )
    void inserted( ::sal_Int32 ID ) override;
    void removed( ::sal_Int32 ID ) override;
    void changed( ::sal_Int32 ID, const cpo::uno::Sequence< css::beans::NamedValue >& Properties ) override;
    void activated( ::sal_Int32 ID ) override;
    void deactivated( ::sal_Int32 ID ) override;
DECL_LISTENERMULTIPLEXER_END


//  class ContainerListenerMultiplexer

DECL_LISTENERMULTIPLEXER_START( ContainerListenerMultiplexer, css::container::XContainerListener )
    void elementInserted( const css::container::ContainerEvent& Event ) override;
    void elementRemoved( const css::container::ContainerEvent& Event ) override;
    void elementReplaced( const css::container::ContainerEvent& Event ) override;
DECL_LISTENERMULTIPLEXER_END


//  class SpinListenerMultiplexer

DECL_LISTENERMULTIPLEXER_START_DLLPUB( SpinListenerMultiplexer, css::awt::XSpinListener )
    void up( const css::awt::SpinEvent& rEvent ) override;
    void down( const css::awt::SpinEvent& rEvent ) override;
    void first( const css::awt::SpinEvent& rEvent ) override;
    void last( const css::awt::SpinEvent& rEvent ) override;
DECL_LISTENERMULTIPLEXER_END


//  class AdjustmentListenerMultiplexer

DECL_LISTENERMULTIPLEXER_START( AdjustmentListenerMultiplexer, css::awt::XAdjustmentListener )
    void adjustmentValueChanged( const css::awt::AdjustmentEvent& rEvent ) override;
DECL_LISTENERMULTIPLEXER_END


//  class MenuListenerMultiplexer

DECL_LISTENERMULTIPLEXER_START( MenuListenerMultiplexer, css::awt::XMenuListener )
    void itemHighlighted( const css::awt::MenuEvent& rEvent ) override;
    void itemSelected( const css::awt::MenuEvent& rEvent ) override;
    void itemActivated( const css::awt::MenuEvent& rEvent ) override;
    void itemDeactivated( const css::awt::MenuEvent& rEvent ) override;
DECL_LISTENERMULTIPLEXER_END


//  class TreeSelectionListenerMultiplexer

DECL_LISTENERMULTIPLEXER_START( TreeSelectionListenerMultiplexer, css::view::XSelectionChangeListener )
    virtual void selectionChanged( const css::lang::EventObject& aEvent ) override;
DECL_LISTENERMULTIPLEXER_END


//  class TreeExpansionListenerMultiplexer

DECL_LISTENERMULTIPLEXER_START( TreeExpansionListenerMultiplexer, css::awt::tree::XTreeExpansionListener )
    virtual void requestChildNodes( const css::awt::tree::TreeExpansionEvent& aEvent ) override;
    virtual void treeExpanding( const css::awt::tree::TreeExpansionEvent& aEvent ) override;
    virtual void treeCollapsing( const css::awt::tree::TreeExpansionEvent& aEvent ) override;
    virtual void treeExpanded( const css::awt::tree::TreeExpansionEvent& aEvent ) override;
    virtual void treeCollapsed( const css::awt::tree::TreeExpansionEvent& aEvent ) override;
DECL_LISTENERMULTIPLEXER_END


//  class TreeEditListenerMultiplexer

DECL_LISTENERMULTIPLEXER_START( TreeEditListenerMultiplexer, css::awt::tree::XTreeEditListener )
    virtual void nodeEditing( const css::uno::Reference< css::awt::tree::XTreeNode >& Node ) override;
    virtual void nodeEdited( const css::uno::Reference< css::awt::tree::XTreeNode >& Node, const OUString& NewText ) override;
DECL_LISTENERMULTIPLEXER_END


//  class SelectionListenerMultiplexer

DECL_LISTENERMULTIPLEXER_START_DLLPUB( SelectionListenerMultiplexer, css::awt::grid::XGridSelectionListener )
    void selectionChanged( const css::awt::grid::GridSelectionEvent& aEvent ) override;
DECL_LISTENERMULTIPLEXER_END


//  class TabPageListenerMultiplexer

DECL_LISTENERMULTIPLEXER_START( TabPageListenerMultiplexer, css::awt::tab::XTabPageContainerListener )
    void tabPageActivated( const css::awt::tab::TabPageActivatedEvent& aEvent ) override;
DECL_LISTENERMULTIPLEXER_END

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
