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

template <> constexpr inline auto Add<css::awt::XFocusListener> = &css::awt::XWindow::addFocusListener;
template <> constexpr inline auto Remove<css::awt::XFocusListener> = &css::awt::XWindow::removeFocusListener;

template <> constexpr inline auto Add<css::awt::XWindowListener> = &css::awt::XWindow::addWindowListener;
template <> constexpr inline auto Remove<css::awt::XWindowListener> = &css::awt::XWindow::removeWindowListener;

template <> constexpr inline auto Add<css::awt::XKeyListener> = &css::awt::XWindow::addKeyListener;
template <> constexpr inline auto Remove<css::awt::XKeyListener> = &css::awt::XWindow::removeKeyListener;

template <> constexpr inline auto Add<css::awt::XMouseListener> = &css::awt::XWindow::addMouseListener;
template <> constexpr inline auto Remove<css::awt::XMouseListener> = &css::awt::XWindow::removeMouseListener;

template <> constexpr inline auto Add<css::awt::XMouseMotionListener> = &css::awt::XWindow::addMouseMotionListener;
template <> constexpr inline auto Remove<css::awt::XMouseMotionListener> = &css::awt::XWindow::removeMouseMotionListener;

template <> constexpr inline auto Add<css::awt::XPaintListener> = &css::awt::XWindow::addPaintListener;
template <> constexpr inline auto Remove<css::awt::XPaintListener> = &css::awt::XWindow::removePaintListener;

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
    void notifyPeer(const css::uno::Reference<css::awt::XWindow>& peer,
                    void (SAL_CALL css::awt::XWindow::*func)(const css::uno::Reference<Ifc1>&))
    {
        if (peer)
            (peer.get()->*func)(this);
    }

    template <class Ifc1>
    void add(std::unique_lock<std::mutex>& guard, const css::uno::Reference<Ifc1>& listener,
             const css::uno::Reference<css::awt::XWindow>& peer)
    {
        assert(listener);
        if (Listeners<Ifc1>::list.addInterface(guard, listener) == 1)
        {
            // the first listener is added
            notifyPeer(peer, Add<Ifc1>);
        }
    }

    template <class Ifc1>
    void remove(std::unique_lock<std::mutex>& guard, const css::uno::Reference<Ifc1>& listener,
                const css::uno::Reference<css::awt::XWindow>& peer)
    {
        if (Listeners<Ifc1>::list.removeInterface(guard, listener) == 0)
        {
            // the last listener is removed
            notifyPeer(peer, Remove<Ifc1>);
        }
    }
};

class OMRCListenerMultiplexerHelper final : public ContainersHolder< css::awt::XFocusListener
                                                                   , css::awt::XWindowListener
                                                                   , css::awt::XKeyListener
                                                                   , css::awt::XMouseListener
                                                                   , css::awt::XMouseMotionListener
                                                                   , css::awt::XPaintListener >
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

    virtual ~OMRCListenerMultiplexerHelper() override;

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

    template <class Interface> void advise(const css::uno::Reference<Interface>& xListener)
    {
        std::unique_lock aGuard(m_aMutex);
        add(aGuard, xListener, m_xPeer);
    }

    /**
        @short      Remove the specified listener from the source.
    */

    template <class Interface> void unadvise(const css::uno::Reference<Interface>& xListener)
    {
        std::unique_lock aGuard(m_aMutex);
        remove(aGuard, xListener, m_xPeer);
    }

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

private:
    template <class Interface, typename Event>
    void Multiplex(void (SAL_CALL Interface::*method)(const Event&), const Event& event);

//  private variables

private:
    css::uno::Reference< css::awt::XWindow >      m_xPeer;   /// The source of the events. Normally this is the peer object.
    css::uno::WeakReference< css::awt::XWindow >  m_xControl;
};

}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
