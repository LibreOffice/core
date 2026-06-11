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

#include <com/sun/star/awt/XKeyListener.hpp>
#include <com/sun/star/awt/XPaintListener.hpp>
#include <com/sun/star/awt/XMouseMotionListener.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/awt/XWindowListener.hpp>
#include <com/sun/star/awt/XMouseListener.hpp>
#include <com/sun/star/awt/XFocusListener.hpp>
#include <comphelper/compbase.hxx>
#include <comphelper/interfacecontainer4.hxx>
#include <cppuhelper/weakref.hxx>

#include <type_traits>

namespace unocontrols {

template <class Listener> extern int Add; // dummy
template <class Listener> extern int Remove; // dummy

template <> constexpr inline auto Add<cpo::awt::XFocusListener> = &cpo::awt::XWindow::addFocusListener;
template <> constexpr inline auto Remove<cpo::awt::XFocusListener> = &cpo::awt::XWindow::removeFocusListener;

template <> constexpr inline auto Add<cpo::awt::XWindowListener> = &cpo::awt::XWindow::addWindowListener;
template <> constexpr inline auto Remove<cpo::awt::XWindowListener> = &cpo::awt::XWindow::removeWindowListener;

template <> constexpr inline auto Add<cpo::awt::XKeyListener> = &cpo::awt::XWindow::addKeyListener;
template <> constexpr inline auto Remove<cpo::awt::XKeyListener> = &cpo::awt::XWindow::removeKeyListener;

template <> constexpr inline auto Add<cpo::awt::XMouseListener> = &cpo::awt::XWindow::addMouseListener;
template <> constexpr inline auto Remove<cpo::awt::XMouseListener> = &cpo::awt::XWindow::removeMouseListener;

template <> constexpr inline auto Add<cpo::awt::XMouseMotionListener> = &cpo::awt::XWindow::addMouseMotionListener;
template <> constexpr inline auto Remove<cpo::awt::XMouseMotionListener> = &cpo::awt::XWindow::removeMouseMotionListener;

template <> constexpr inline auto Add<cpo::awt::XPaintListener> = &cpo::awt::XWindow::addPaintListener;
template <> constexpr inline auto Remove<cpo::awt::XPaintListener> = &cpo::awt::XWindow::removePaintListener;

template <class Ifc> class Listeners
{
protected:
    comphelper::OInterfaceContainerHelper4<Ifc> list;
};

template <class... Ifc>
class ContainersHolder : public comphelper::WeakImplHelper<Ifc...>, public Listeners<Ifc>...
{
protected:
    template <typename F> void for_each_container(F f) { (..., f(Listeners<Ifc>::list)); }

    template <class Ifc1>
    void notifyPeer(const cpo::uno::Reference<cpo::awt::XWindow>& peer,
                    void (SAL_CALL cpo::awt::XWindow::*func)(const cpo::uno::Reference<Ifc1>&))
    {
        if (peer)
            (peer.get()->*func)(this);
    }

    template <class Ifc1>
    void add(std::unique_lock<std::mutex>& guard, const cpo::uno::Reference<Ifc1>& listener,
             const cpo::uno::Reference<cpo::awt::XWindow>& peer)
    {
        assert(listener);
        if (Listeners<Ifc1>::list.addInterface(guard, listener) == 1)
        {
            // the first listener is added
            notifyPeer(peer, Add<Ifc1>);
        }
    }

    template <class Ifc1>
    void remove(std::unique_lock<std::mutex>& guard, const cpo::uno::Reference<Ifc1>& listener,
                const cpo::uno::Reference<cpo::awt::XWindow>& peer)
    {
        if (Listeners<Ifc1>::list.removeInterface(guard, listener) == 0)
        {
            // the last listener is removed
            notifyPeer(peer, Remove<Ifc1>);
        }
    }
};

class OMRCListenerMultiplexerHelper final : public ContainersHolder< cpo::awt::XFocusListener
                                                                   , cpo::awt::XWindowListener
                                                                   , cpo::awt::XKeyListener
                                                                   , cpo::awt::XMouseListener
                                                                   , cpo::awt::XMouseMotionListener
                                                                   , cpo::awt::XPaintListener >
{
public:

    /**
        @short      constructor
        @descr      Create a Multiplexer of XWindowEvents.
        @param      rControl    The control. All listeners think that this is the original broadcaster.
        @param      rPeer       The peer from which the original events are dispatched. Null is allowed.
    */

    OMRCListenerMultiplexerHelper(  const   cpo::uno::Reference< cpo::awt::XWindow >& xControl    ,
                                    const   cpo::uno::Reference< cpo::awt::XWindow >& xPeer       );

    virtual ~OMRCListenerMultiplexerHelper() override;

    OMRCListenerMultiplexerHelper& operator= ( const OMRCListenerMultiplexerHelper& aCopyInstance );

    //  container methods

    /**
        @short      Remove all listeners from the previous set peer and add the needed listeners to rPeer.
        @param      rPeer       The peer from which the original events are dispatched. Null is allowed.
    */

    void setPeer( const cpo::uno::Reference< cpo::awt::XWindow >& xPeer );

    /**
        @short      Remove all listeners and send a disposing message.
    */

    void disposeAndClear();

    /**
        @short      Add the specified listener to the source.
    */

    template <class Interface> void advise(const cpo::uno::Reference<Interface>& xListener)
    {
        std::unique_lock aGuard(m_aMutex);
        add(aGuard, xListener, m_xPeer);
    }

    /**
        @short      Remove the specified listener from the source.
    */

    template <class Interface> void unadvise(const cpo::uno::Reference<Interface>& xListener)
    {
        std::unique_lock aGuard(m_aMutex);
        remove(aGuard, xListener, m_xPeer);
    }

    //  XEventListener

    virtual void SAL_CALL disposing(const cpo::lang::EventObject& aSource) override;

    //  XFocusListener

    virtual void SAL_CALL focusGained(const cpo::awt::FocusEvent& aEvent ) override;

    virtual void SAL_CALL focusLost(const cpo::awt::FocusEvent& aEvent ) override;

    //  XWindowListener

    virtual void SAL_CALL windowResized(const cpo::awt::WindowEvent& aEvent ) override;

    virtual void SAL_CALL windowMoved(const cpo::awt::WindowEvent& aEvent ) override;

    virtual void SAL_CALL windowShown(const cpo::lang::EventObject& aEvent ) override;

    virtual void SAL_CALL windowHidden(const cpo::lang::EventObject& aEvent ) override;

    //  XKeyListener

    virtual void SAL_CALL keyPressed( const cpo::awt::KeyEvent& aEvent ) override;

    virtual void SAL_CALL keyReleased( const cpo::awt::KeyEvent& aEvent ) override;

    //  XMouseListener

    virtual void SAL_CALL mousePressed(const cpo::awt::MouseEvent& aEvent ) override;

    virtual void SAL_CALL mouseReleased(const cpo::awt::MouseEvent& aEvent ) override;

    virtual void SAL_CALL mouseEntered(const cpo::awt::MouseEvent& aEvent ) override;

    virtual void SAL_CALL mouseExited(const cpo::awt::MouseEvent& aEvent ) override;

    //  XMouseMotionListener

    virtual void SAL_CALL mouseDragged(const cpo::awt::MouseEvent& aEvent ) override;

    virtual void SAL_CALL mouseMoved(const cpo::awt::MouseEvent& aEvent ) override;

    //  XPaintListener

    virtual void SAL_CALL windowPaint(const cpo::awt::PaintEvent& aEvent ) override;

private:
    template <class Interface, typename Event>
    void Multiplex(void (SAL_CALL Interface::*method)(const Event&), const Event& event);

//  private variables

private:
    cpo::uno::Reference< cpo::awt::XWindow >      m_xPeer;   /// The source of the events. Normally this is the peer object.
    cpo::uno::WeakReference< cpo::awt::XWindow >  m_xControl;
};

}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
