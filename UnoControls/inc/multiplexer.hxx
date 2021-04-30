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

#pragma once

#include <com/sun/star/awt/XKeyListener.hpp>
#include <com/sun/star/awt/XPaintListener.hpp>
#include <com/sun/star/awt/XMouseMotionListener.hpp>
#include <com/sun/star/awt/XWindowListener.hpp>
#include <com/sun/star/awt/XTopWindowListener.hpp>
#include <com/sun/star/awt/XMouseListener.hpp>
#include <com/sun/star/awt/XFocusListener.hpp>
#include <cppuhelper/weak.hxx>
#include <cppuhelper/weakref.hxx>
#include <cppuhelper/interfacecontainer.hxx>

namespace com::sun::star::awt { class XWindow; }
namespace com::sun::star::awt { struct KeyEvent; }
namespace com::sun::star::awt { struct MouseEvent; }
namespace com::sun::star::awt { struct PaintEvent; }
namespace com::sun::star::awt { struct WindowEvent; }

namespace unocontrols {

class OMRCListenerMultiplexerHelper final : public css::awt::XFocusListener
                                    , public css::awt::XWindowListener
                                    , public css::awt::XKeyListener
                                    , public css::awt::XMouseListener
                                    , public css::awt::XMouseMotionListener
                                    , public css::awt::XPaintListener
                                    , public css::awt::XTopWindowListener
                                    , public ::cppu::OWeakObject
{
public:

    /**
        @short      constructor
        @descr      Create a Multiplexer of XWindowEvents.
        @param      rControl    The control. All listeners think that this is the original broadcaster.
        @param      rPeer       The peer from which the original events are dispatched. Null is allowed.
    */

    OMRCListenerMultiplexerHelper(  const   css::uno::Reference< css::awt::XWindow >& xControl    ,
                                    const   css::uno::Reference< css::awt::XWindow >& xPeer       );

    /**
        @short      copy-constructor
        @descr
        @param      rCopyInstance   C++-Reference to instance to make copy from.
    */

    OMRCListenerMultiplexerHelper( const OMRCListenerMultiplexerHelper& aCopyInstance );

    virtual ~OMRCListenerMultiplexerHelper() override;

    //  XInterface

    /**
        @short      give answer, if interface is supported
        @descr      The interfaces are searched by type.

        @seealso    XInterface

        @param      "rType" is the type of searched interface.

        @return     Any     information about found interface

        @onerror    A RuntimeException is thrown.
    */

    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type& aType ) override;

    /**
        @short      increment refcount
        @seealso    XInterface
        @seealso    release()
        @onerror    A RuntimeException is thrown.
    */

    virtual void SAL_CALL acquire() noexcept override;

    /**
        @short      decrement refcount
        @seealso    XInterface
        @seealso    acquire()
        @onerror    A RuntimeException is thrown.
    */

    virtual void SAL_CALL release() noexcept override;

    OMRCListenerMultiplexerHelper& operator= ( const OMRCListenerMultiplexerHelper& aCopyInstance );

    //  container methods

    /**
        @short      Remove all listeners from the previous set peer and add the needed listeners to rPeer.
        @param      rPeer       The peer from which the original events are dispatched. Null is allowed.
    */

    void setPeer( const css::uno::Reference< css::awt::XWindow >& xPeer );

    /**
        @short      Remove all listeners and send a disposing message.
    */

    void disposeAndClear();

    /**
        @short      Add the specified listener to the source.
    */

    void advise(    const   css::uno::Type&                              aType       ,
                    const   css::uno::Reference< css::uno::XInterface >&  xListener   );

    /**
        @short      Remove the specified listener from the source.
    */

    void unadvise(  const   css::uno::Type&                              aType       ,
                    const   css::uno::Reference< css::uno::XInterface >&  xListener   );

    //  XEventListener

    virtual void SAL_CALL disposing(const css::lang::EventObject& aSource) override;

    //  XFocusListener

    virtual void SAL_CALL focusGained(const css::awt::FocusEvent& aEvent ) override;

    virtual void SAL_CALL focusLost(const css::awt::FocusEvent& aEvent ) override;

    //  XWindowListener

    virtual void SAL_CALL windowResized(const css::awt::WindowEvent& aEvent ) override;

    virtual void SAL_CALL windowMoved(const css::awt::WindowEvent& aEvent ) override;

    virtual void SAL_CALL windowShown(const css::lang::EventObject& aEvent ) override;

    virtual void SAL_CALL windowHidden(const css::lang::EventObject& aEvent ) override;

    //  XKeyListener

    virtual void SAL_CALL keyPressed( const css::awt::KeyEvent& aEvent ) override;

    virtual void SAL_CALL keyReleased( const css::awt::KeyEvent& aEvent ) override;

    //  XMouseListener

    virtual void SAL_CALL mousePressed(const css::awt::MouseEvent& aEvent ) override;

    virtual void SAL_CALL mouseReleased(const css::awt::MouseEvent& aEvent ) override;

    virtual void SAL_CALL mouseEntered(const css::awt::MouseEvent& aEvent ) override;

    virtual void SAL_CALL mouseExited(const css::awt::MouseEvent& aEvent ) override;

    //  XMouseMotionListener

    virtual void SAL_CALL mouseDragged(const css::awt::MouseEvent& aEvent ) override;

    virtual void SAL_CALL mouseMoved(const css::awt::MouseEvent& aEvent ) override;

    //  XPaintListener

    virtual void SAL_CALL windowPaint(const css::awt::PaintEvent& aEvent ) override;

    //  XTopWindowListener

    virtual void SAL_CALL windowOpened( const css::lang::EventObject& aEvent ) override;

    virtual void SAL_CALL windowClosing( const css::lang::EventObject& aEvent ) override;

    virtual void SAL_CALL windowClosed( const css::lang::EventObject& aEvent ) override;

    virtual void SAL_CALL windowMinimized( const css::lang::EventObject& aEvent ) override;

    virtual void SAL_CALL windowNormalized( const css::lang::EventObject& aEvent ) override;

    virtual void SAL_CALL windowActivated( const css::lang::EventObject& aEvent ) override;

    virtual void SAL_CALL windowDeactivated( const css::lang::EventObject& aEvent ) override;

private:

    /**
        @short      Remove the listener from the peer.
        @param      xPeer   The peer from which the listener is removed.
        @param      rType   The listener type, which specify the type of the listener.
    */

    void impl_adviseToPeer( const   css::uno::Reference< css::awt::XWindow >& xPeer   ,
                            const   css::uno::Type&                          aType   );

    /**
        @short      Add the listener to the peer.
        @param      xPeer   The peer to which the listener is added.
        @param      rType   The listener type, which specify the type of the listener.
    */

    void impl_unadviseFromPeer( const   css::uno::Reference< css::awt::XWindow >& xPeer   ,
                                const   css::uno::Type&                          aType   );

//  private variables

private:
    ::osl::Mutex                                m_aMutex;
    css::uno::Reference< css::awt::XWindow >      m_xPeer;   /// The source of the events. Normally this is the peer object.
    css::uno::WeakReference< css::awt::XWindow >  m_xControl;
    ::cppu::OMultiTypeInterfaceContainerHelper  m_aListenerHolder;

};

}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
