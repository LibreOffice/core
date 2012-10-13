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

#ifndef _TOOLKIT_HELPER_LISTENERMULTIPLEXER_HXX_
#define _TOOLKIT_HELPER_LISTENERMULTIPLEXER_HXX_

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
#include <com/sun/star/util/VetoException.hpp>
#include <cppuhelper/weak.hxx>
#include <cppuhelper/interfacecontainer.hxx>
#include <osl/mutex.hxx>
#include <toolkit/helper/mutexhelper.hxx>
#include <toolkit/helper/macros.hxx>
#include <com/sun/star/awt/grid/XGridSelectionListener.hpp>
#include <com/sun/star/awt/tab/XTabPageContainerListener.hpp>
//  ----------------------------------------------------
//  class ListenerMultiplexerBase
//  ----------------------------------------------------

class TOOLKIT_DLLPUBLIC ListenerMultiplexerBase : public MutexHelper,
                                public ::cppu::OInterfaceContainerHelper,
                                public ::com::sun::star::uno::XInterface
{
private:
    ::cppu::OWeakObject&    mrContext;

protected:
    ::cppu::OWeakObject&    GetContext() { return mrContext; }

public:
    ListenerMultiplexerBase( ::cppu::OWeakObject& rSource );
    virtual ~ListenerMultiplexerBase();

    // ::com::sun::star::uno::XInterface
    ::com::sun::star::uno::Any  SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
    void                        SAL_CALL acquire() throw()  { mrContext.acquire(); }
    void                        SAL_CALL release() throw()  { mrContext.release(); }
};


//  ----------------------------------------------------
//  class EventListenerMultiplexer
//  ----------------------------------------------------
DECL_LISTENERMULTIPLEXER_START( EventListenerMultiplexer, ::com::sun::star::lang::XEventListener )
DECL_LISTENERMULTIPLEXER_END

//  ----------------------------------------------------
//  class FocusListenerMultiplexer
//  ----------------------------------------------------
DECL_LISTENERMULTIPLEXER_START( FocusListenerMultiplexer, ::com::sun::star::awt::XFocusListener )
    void SAL_CALL focusGained( const ::com::sun::star::awt::FocusEvent& e ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL focusLost( const ::com::sun::star::awt::FocusEvent& e ) throw(::com::sun::star::uno::RuntimeException);
DECL_LISTENERMULTIPLEXER_END


//  ----------------------------------------------------
//  class WindowListenerMultiplexer
//  ----------------------------------------------------
DECL_LISTENERMULTIPLEXER_START( WindowListenerMultiplexer, ::com::sun::star::awt::XWindowListener )
    void SAL_CALL windowResized( const ::com::sun::star::awt::WindowEvent& e ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL windowMoved( const ::com::sun::star::awt::WindowEvent& e ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL windowShown( const ::com::sun::star::lang::EventObject& e ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL windowHidden( const ::com::sun::star::lang::EventObject& e ) throw(::com::sun::star::uno::RuntimeException);
DECL_LISTENERMULTIPLEXER_END



//  ----------------------------------------------------
//  class VclContainerListenerMultiplexer
//  ----------------------------------------------------
DECL_LISTENERMULTIPLEXER_START( VclContainerListenerMultiplexer, ::com::sun::star::awt::XVclContainerListener )
    void SAL_CALL windowAdded( const ::com::sun::star::awt::VclContainerEvent& e ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL windowRemoved( const ::com::sun::star::awt::VclContainerEvent& e ) throw(::com::sun::star::uno::RuntimeException);
DECL_LISTENERMULTIPLEXER_END

//  ----------------------------------------------------
//  class KeyListenerMultiplexer
//  ----------------------------------------------------
DECL_LISTENERMULTIPLEXER_START( KeyListenerMultiplexer, ::com::sun::star::awt::XKeyListener )
    void SAL_CALL keyPressed( const ::com::sun::star::awt::KeyEvent& e ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL keyReleased( const ::com::sun::star::awt::KeyEvent& e ) throw(::com::sun::star::uno::RuntimeException);
DECL_LISTENERMULTIPLEXER_END

//  ----------------------------------------------------
//  class MouseListenerMultiplexer
//  ----------------------------------------------------
DECL_LISTENERMULTIPLEXER_START_DLLPUB( MouseListenerMultiplexer, ::com::sun::star::awt::XMouseListener )
    void SAL_CALL mousePressed( const ::com::sun::star::awt::MouseEvent& e ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL mouseReleased( const ::com::sun::star::awt::MouseEvent& e ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL mouseEntered( const ::com::sun::star::awt::MouseEvent& e ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL mouseExited( const ::com::sun::star::awt::MouseEvent& e ) throw(::com::sun::star::uno::RuntimeException);
DECL_LISTENERMULTIPLEXER_END

//  ----------------------------------------------------
//  class MouseMotionListenerMultiplexer
//  ----------------------------------------------------
DECL_LISTENERMULTIPLEXER_START( MouseMotionListenerMultiplexer, ::com::sun::star::awt::XMouseMotionListener )
    void SAL_CALL mouseDragged( const ::com::sun::star::awt::MouseEvent& e ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL mouseMoved( const ::com::sun::star::awt::MouseEvent& e ) throw(::com::sun::star::uno::RuntimeException);
DECL_LISTENERMULTIPLEXER_END

//  ----------------------------------------------------
//  class PaintListenerMultiplexer
//  ----------------------------------------------------
DECL_LISTENERMULTIPLEXER_START( PaintListenerMultiplexer, ::com::sun::star::awt::XPaintListener )
    void SAL_CALL windowPaint( const ::com::sun::star::awt::PaintEvent& e ) throw(::com::sun::star::uno::RuntimeException);
DECL_LISTENERMULTIPLEXER_END

//  ----------------------------------------------------
//  class TopWindowListenerMultiplexer
//  ----------------------------------------------------
DECL_LISTENERMULTIPLEXER_START( TopWindowListenerMultiplexer, ::com::sun::star::awt::XTopWindowListener )
    void SAL_CALL windowOpened( const ::com::sun::star::lang::EventObject& e ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL windowClosing( const ::com::sun::star::lang::EventObject& e ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL windowClosed( const ::com::sun::star::lang::EventObject& e ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL windowMinimized( const ::com::sun::star::lang::EventObject& e ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL windowNormalized( const ::com::sun::star::lang::EventObject& e ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL windowActivated( const ::com::sun::star::lang::EventObject& e ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL windowDeactivated( const ::com::sun::star::lang::EventObject& e ) throw(::com::sun::star::uno::RuntimeException);
DECL_LISTENERMULTIPLEXER_END

//  ----------------------------------------------------
//  class TextListenerMultiplexer
//  ----------------------------------------------------
DECL_LISTENERMULTIPLEXER_START_DLLPUB( TextListenerMultiplexer, ::com::sun::star::awt::XTextListener )
    void SAL_CALL textChanged( const ::com::sun::star::awt::TextEvent& rEvent ) throw(::com::sun::star::uno::RuntimeException);
DECL_LISTENERMULTIPLEXER_END

//  ----------------------------------------------------
//  class ActionListenerMultiplexer
//  ----------------------------------------------------
DECL_LISTENERMULTIPLEXER_START_DLLPUB( ActionListenerMultiplexer, ::com::sun::star::awt::XActionListener )
    void SAL_CALL actionPerformed( const ::com::sun::star::awt::ActionEvent& rEvent ) throw(::com::sun::star::uno::RuntimeException);
DECL_LISTENERMULTIPLEXER_END

//  ----------------------------------------------------
//  class ItemListenerMultiplexer
//  ----------------------------------------------------
DECL_LISTENERMULTIPLEXER_START_DLLPUB( ItemListenerMultiplexer, ::com::sun::star::awt::XItemListener )
    void SAL_CALL itemStateChanged( const ::com::sun::star::awt::ItemEvent& rEvent ) throw(::com::sun::star::uno::RuntimeException);
DECL_LISTENERMULTIPLEXER_END

//  ----------------------------------------------------
//  class TabListenerMultiplexer
//  ----------------------------------------------------
DECL_LISTENERMULTIPLEXER_START_DLLPUB( TabListenerMultiplexer, ::com::sun::star::awt::XTabListener )
    void SAL_CALL inserted( ::sal_Int32 ID ) throw (::com::sun::star::uno::RuntimeException);
    void SAL_CALL removed( ::sal_Int32 ID ) throw (::com::sun::star::uno::RuntimeException);
    void SAL_CALL changed( ::sal_Int32 ID, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue >& Properties ) throw (::com::sun::star::uno::RuntimeException);
    void SAL_CALL activated( ::sal_Int32 ID ) throw (::com::sun::star::uno::RuntimeException);
    void SAL_CALL deactivated( ::sal_Int32 ID ) throw (::com::sun::star::uno::RuntimeException);
DECL_LISTENERMULTIPLEXER_END

//  ----------------------------------------------------
//  class ContainerListenerMultiplexer
//  ----------------------------------------------------
DECL_LISTENERMULTIPLEXER_START( ContainerListenerMultiplexer, ::com::sun::star::container::XContainerListener )
    void SAL_CALL elementInserted( const ::com::sun::star::container::ContainerEvent& Event ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL elementRemoved( const ::com::sun::star::container::ContainerEvent& Event ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL elementReplaced( const ::com::sun::star::container::ContainerEvent& Event ) throw(::com::sun::star::uno::RuntimeException);
DECL_LISTENERMULTIPLEXER_END

//  ----------------------------------------------------
//  class SpinListenerMultiplexer
//  ----------------------------------------------------
DECL_LISTENERMULTIPLEXER_START_DLLPUB( SpinListenerMultiplexer, ::com::sun::star::awt::XSpinListener )
    void SAL_CALL up( const ::com::sun::star::awt::SpinEvent& rEvent ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL down( const ::com::sun::star::awt::SpinEvent& rEvent ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL first( const ::com::sun::star::awt::SpinEvent& rEvent ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL last( const ::com::sun::star::awt::SpinEvent& rEvent ) throw(::com::sun::star::uno::RuntimeException);
DECL_LISTENERMULTIPLEXER_END

//  ----------------------------------------------------
//  class AdjustmentListenerMultiplexer
//  ----------------------------------------------------
DECL_LISTENERMULTIPLEXER_START( AdjustmentListenerMultiplexer, ::com::sun::star::awt::XAdjustmentListener )
    void SAL_CALL adjustmentValueChanged( const ::com::sun::star::awt::AdjustmentEvent& rEvent ) throw(::com::sun::star::uno::RuntimeException);
DECL_LISTENERMULTIPLEXER_END

//  ----------------------------------------------------
//  class MenuListenerMultiplexer
//  ----------------------------------------------------
DECL_LISTENERMULTIPLEXER_START( MenuListenerMultiplexer, ::com::sun::star::awt::XMenuListener )
    void SAL_CALL highlight( const ::com::sun::star::awt::MenuEvent& rEvent ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL select( const ::com::sun::star::awt::MenuEvent& rEvent ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL activate( const ::com::sun::star::awt::MenuEvent& rEvent ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL deactivate( const ::com::sun::star::awt::MenuEvent& rEvent ) throw(::com::sun::star::uno::RuntimeException);
DECL_LISTENERMULTIPLEXER_END

//  ----------------------------------------------------
//  class TreeSelectionListenerMultiplexer
//  ----------------------------------------------------
DECL_LISTENERMULTIPLEXER_START_DLLPUB( TreeSelectionListenerMultiplexer, ::com::sun::star::view::XSelectionChangeListener )
    virtual void SAL_CALL selectionChanged( const ::com::sun::star::lang::EventObject& aEvent ) throw (::com::sun::star::uno::RuntimeException);
DECL_LISTENERMULTIPLEXER_END

//  ----------------------------------------------------
//  class TreeExpansionListenerMultiplexer
//  ----------------------------------------------------
DECL_LISTENERMULTIPLEXER_START_DLLPUB( TreeExpansionListenerMultiplexer, ::com::sun::star::awt::tree::XTreeExpansionListener )
    virtual void SAL_CALL requestChildNodes( const ::com::sun::star::awt::tree::TreeExpansionEvent& aEvent ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL treeExpanding( const ::com::sun::star::awt::tree::TreeExpansionEvent& aEvent ) throw (::com::sun::star::awt::tree::ExpandVetoException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL treeCollapsing( const ::com::sun::star::awt::tree::TreeExpansionEvent& aEvent ) throw (::com::sun::star::awt::tree::ExpandVetoException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL treeExpanded( const ::com::sun::star::awt::tree::TreeExpansionEvent& aEvent ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL treeCollapsed( const ::com::sun::star::awt::tree::TreeExpansionEvent& aEvent ) throw (::com::sun::star::uno::RuntimeException);
DECL_LISTENERMULTIPLEXER_END

//  ----------------------------------------------------
//  class TreeEditListenerMultiplexer
//  ----------------------------------------------------
DECL_LISTENERMULTIPLEXER_START_DLLPUB( TreeEditListenerMultiplexer, ::com::sun::star::awt::tree::XTreeEditListener )
    virtual void SAL_CALL nodeEditing( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::tree::XTreeNode >& Node ) throw (::com::sun::star::uno::RuntimeException,::com::sun::star::util::VetoException);
    virtual void SAL_CALL nodeEdited( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::tree::XTreeNode >& Node, const ::rtl::OUString& NewText ) throw (::com::sun::star::uno::RuntimeException);
DECL_LISTENERMULTIPLEXER_END

//  ----------------------------------------------------
//  class SelectionListenerMultiplexer
//  ----------------------------------------------------
DECL_LISTENERMULTIPLEXER_START_DLLPUB( SelectionListenerMultiplexer, ::com::sun::star::awt::grid::XGridSelectionListener )
    void SAL_CALL selectionChanged( const ::com::sun::star::awt::grid::GridSelectionEvent& aEvent ) throw (::com::sun::star::uno::RuntimeException);
DECL_LISTENERMULTIPLEXER_END

//  ----------------------------------------------------
//  class TabPageListenerMultiplexer
//  ----------------------------------------------------
DECL_LISTENERMULTIPLEXER_START_DLLPUB( TabPageListenerMultiplexer, ::com::sun::star::awt::tab::XTabPageContainerListener )
    void SAL_CALL tabPageActivated( const ::com::sun::star::awt::tab::TabPageActivatedEvent& aEvent ) throw (::com::sun::star::uno::RuntimeException);
DECL_LISTENERMULTIPLEXER_END

#endif // _TOOLKIT_HELPER_LISTENERMULTIPLEXER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
