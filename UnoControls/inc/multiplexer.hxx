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

#ifndef INCLUDED_UNOCONTROLS_INC_MULTIPLEXER_HXX
#define INCLUDED_UNOCONTROLS_INC_MULTIPLEXER_HXX

#include <com/sun/star/awt/XKeyListener.hpp>
#include <com/sun/star/awt/XPaintListener.hpp>
#include <com/sun/star/awt/KeyEvent.hpp>
#include <com/sun/star/awt/KeyModifier.hpp>
#include <com/sun/star/awt/XMouseMotionListener.hpp>
#include <com/sun/star/awt/FocusEvent.hpp>
#include <com/sun/star/awt/XWindowListener.hpp>
#include <com/sun/star/awt/XActivateListener.hpp>
#include <com/sun/star/awt/MouseEvent.hpp>
#include <com/sun/star/awt/XTopWindowListener.hpp>
#include <com/sun/star/awt/PaintEvent.hpp>
#include <com/sun/star/awt/InputEvent.hpp>
#include <com/sun/star/awt/KeyGroup.hpp>
#include <com/sun/star/awt/Key.hpp>
#include <com/sun/star/awt/WindowEvent.hpp>
#include <com/sun/star/awt/XMouseListener.hpp>
#include <com/sun/star/awt/KeyFunction.hpp>
#include <com/sun/star/awt/FocusChangeReason.hpp>
#include <com/sun/star/awt/MouseButton.hpp>
#include <com/sun/star/awt/XFocusListener.hpp>
#include <com/sun/star/awt/XTopWindow.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/awt/PosSize.hpp>
#include <cppuhelper/weak.hxx>
#include <cppuhelper/weakref.hxx>
#include <cppuhelper/interfacecontainer.hxx>

//  "namespaces"

namespace unocontrols{

//  class

class OMRCListenerMultiplexerHelper : public ::com::sun::star::awt::XFocusListener
                                    , public ::com::sun::star::awt::XWindowListener
                                    , public ::com::sun::star::awt::XKeyListener
                                    , public ::com::sun::star::awt::XMouseListener
                                    , public ::com::sun::star::awt::XMouseMotionListener
                                    , public ::com::sun::star::awt::XPaintListener
                                    , public ::com::sun::star::awt::XTopWindowListener
                                    , public ::cppu::OWeakObject
{

//  public methods

public:

    //  construct/destruct

    /**_______________________________________________________________________________________________________
        @short      constructor
        @descr      Create a Multiplexer of XWindowEvents.
        @param      rControl    The control. All listeners think that this is the original broadcaster.
        @param      rPeer       The peer from which the original events are dispatched. Null is allowed.
    */

    OMRCListenerMultiplexerHelper(  const   ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >& xControl    ,
                                    const   ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >& xPeer       );

    /**_______________________________________________________________________________________________________
        @short      copy-constructor
        @descr
        @param      rCopyInstance   C++-Reference to instance to make copy from.
    */

    OMRCListenerMultiplexerHelper( const OMRCListenerMultiplexerHelper& aCopyInstance );

    /**_______________________________________________________________________________________________________
        @short      destructor
    */

    virtual ~OMRCListenerMultiplexerHelper();

    //  XInterface

    /**_______________________________________________________________________________________________________
        @short      give answer, if interface is supported
        @descr      The interfaces are searched by type.

        @seealso    XInterface

        @param      "rType" is the type of searched interface.

        @return     Any     information about found interface

        @onerror    A RuntimeException is thrown.
    */

    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& aType )
        throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    /**_______________________________________________________________________________________________________
        @short      increment refcount
        @seealso    XInterface
        @seealso    release()
        @onerror    A RuntimeException is thrown.
    */

    virtual void SAL_CALL acquire() throw() override;

    /**_______________________________________________________________________________________________________
        @short      decrement refcount
        @seealso    XInterface
        @seealso    acquire()
        @onerror    A RuntimeException is thrown.
    */

    virtual void SAL_CALL release() throw() override;

    //  operator

    /**_______________________________________________________________________________________________________
    */

    operator ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >() const { return (static_cast<OWeakObject*>(const_cast<OMRCListenerMultiplexerHelper *>(this)));}

    /**_______________________________________________________________________________________________________
    */

    OMRCListenerMultiplexerHelper& operator= ( const OMRCListenerMultiplexerHelper& aCopyInstance );

    //  container methods

    /**_______________________________________________________________________________________________________
        @short      Remove all listeners from the previous set peer and add the needed listeners to rPeer.
        @param      rPeer       The peer from which the original events are dispatched. Null is allowed.
    */

    void setPeer( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >& xPeer );

    /**_______________________________________________________________________________________________________
        @short      Remove all listeners and send a disposing message.
    */

    void disposeAndClear();

    /**_______________________________________________________________________________________________________
        @short      Add the specified listener to the source.
    */

    void advise(    const   ::com::sun::star::uno::Type&                              aType       ,
                    const   ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >&  xListener   );

    /**_______________________________________________________________________________________________________
        @short      Remove the specified listener from the source.
    */

    void unadvise(  const   ::com::sun::star::uno::Type&                              aType       ,
                    const   ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >&  xListener   );

    //  XEventListener

    /**_______________________________________________________________________________________________________
    */

    virtual void SAL_CALL disposing(const ::com::sun::star::lang::EventObject& aSource)
        throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    //  XFocusListener

    /**_______________________________________________________________________________________________________
    */

    virtual void SAL_CALL focusGained(const ::com::sun::star::awt::FocusEvent& aEvent )
        throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    /**_______________________________________________________________________________________________________
    */

    virtual void SAL_CALL focusLost(const ::com::sun::star::awt::FocusEvent& aEvent )
        throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    //  XWindowListener

    /**_______________________________________________________________________________________________________
    */

    virtual void SAL_CALL windowResized(const ::com::sun::star::awt::WindowEvent& aEvent )
        throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    /**_______________________________________________________________________________________________________
    */

    virtual void SAL_CALL windowMoved(const ::com::sun::star::awt::WindowEvent& aEvent )
        throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    /**_______________________________________________________________________________________________________
    */

    virtual void SAL_CALL windowShown(const ::com::sun::star::lang::EventObject& aEvent )
        throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    /**_______________________________________________________________________________________________________
    */

    virtual void SAL_CALL windowHidden(const ::com::sun::star::lang::EventObject& aEvent )
        throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    //  XKeyListener

    /**_______________________________________________________________________________________________________
    */

    virtual void SAL_CALL keyPressed( const ::com::sun::star::awt::KeyEvent& aEvent )
        throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    /**_______________________________________________________________________________________________________
    */

    virtual void SAL_CALL keyReleased( const ::com::sun::star::awt::KeyEvent& aEvent )
        throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    //  XMouseListener

    /**_______________________________________________________________________________________________________
    */

    virtual void SAL_CALL mousePressed(const ::com::sun::star::awt::MouseEvent& aEvent )
        throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    /**_______________________________________________________________________________________________________
    */

    virtual void SAL_CALL mouseReleased(const ::com::sun::star::awt::MouseEvent& aEvent )
        throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    /**_______________________________________________________________________________________________________
    */

    virtual void SAL_CALL mouseEntered(const ::com::sun::star::awt::MouseEvent& aEvent )
        throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    /**_______________________________________________________________________________________________________
    */

    virtual void SAL_CALL mouseExited(const ::com::sun::star::awt::MouseEvent& aEvent )
        throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    //  XMouseMotionListener

    /**_______________________________________________________________________________________________________
    */

    virtual void SAL_CALL mouseDragged(const ::com::sun::star::awt::MouseEvent& aEvent )
        throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    /**_______________________________________________________________________________________________________
    */

    virtual void SAL_CALL mouseMoved(const ::com::sun::star::awt::MouseEvent& aEvent )
        throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    //  XPaintListener

    /**_______________________________________________________________________________________________________
    */

    virtual void SAL_CALL windowPaint(const ::com::sun::star::awt::PaintEvent& aEvent )
        throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    //  XTopWindowListener

    /**_______________________________________________________________________________________________________
    */

    virtual void SAL_CALL windowOpened( const ::com::sun::star::lang::EventObject& aEvent )
        throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    /**_______________________________________________________________________________________________________
    */

    virtual void SAL_CALL windowClosing( const ::com::sun::star::lang::EventObject& aEvent )
        throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    /**_______________________________________________________________________________________________________
    */

    virtual void SAL_CALL windowClosed( const ::com::sun::star::lang::EventObject& aEvent )
        throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    /**_______________________________________________________________________________________________________
    */

    virtual void SAL_CALL windowMinimized( const ::com::sun::star::lang::EventObject& aEvent )
        throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    /**_______________________________________________________________________________________________________
    */

    virtual void SAL_CALL windowNormalized( const ::com::sun::star::lang::EventObject& aEvent )
        throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    /**_______________________________________________________________________________________________________
    */

    virtual void SAL_CALL windowActivated( const ::com::sun::star::lang::EventObject& aEvent )
        throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    /**_______________________________________________________________________________________________________
    */

    virtual void SAL_CALL windowDeactivated( const ::com::sun::star::lang::EventObject& aEvent )
        throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

//  protected methods

protected:

    /**_______________________________________________________________________________________________________
        @short      Remove the listener from the peer.
        @param      xPeer   The peer from which the listener is removed.
        @param      rType   The listener type, which specify the type of the listener.
    */

    void impl_adviseToPeer( const   ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >& xPeer   ,
                            const   ::com::sun::star::uno::Type&                          aType   );

    /**_______________________________________________________________________________________________________
        @short      Add the listener to the peer.
        @param      xPeer   The peer to which the listener is added.
        @param      rType   The listener type, which specify the type of the listener.
    */

    void impl_unadviseFromPeer( const   ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >& xPeer   ,
                                const   ::com::sun::star::uno::Type&                          aType   );

//  private variables

private:

    ::osl::Mutex                                m_aMutex;
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >      m_xPeer;   /// The source of the events. Normally this is the peer object.
    ::com::sun::star::uno::WeakReference< ::com::sun::star::awt::XWindow >  m_xControl;
    ::cppu::OMultiTypeInterfaceContainerHelper  m_aListenerHolder;

};  // class OMRCListenerMultiplexerHelper

}   // namespace unocontrols

#endif // INCLUDED_UNOCONTROLS_INC_MULTIPLEXER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
