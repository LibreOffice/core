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

#include <multiplexer.hxx>

#include <osl/diagnose.h>
#include <cppuhelper/queryinterface.hxx>

using namespace ::cppu;
using namespace ::osl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::lang;

namespace unocontrols {

template <class Interface, typename Event>
void OMRCListenerMultiplexerHelper::Multiplex(void (SAL_CALL Interface::*method)(const Event&),
                                              const Event& event)
{
    std::unique_lock aGuard(m_aMutex);
    Event aLocalEvent = event;
    /* Remark: The control is the event source not the peer.*/
    /*         We must change the source of the event.      */
    aLocalEvent.Source = m_xControl;
    /* Is the control not destroyed? */
    if (!aLocalEvent.Source)
        return;
    Listeners<Interface>::list.notifyEach(aGuard, method, aLocalEvent);
}

//  construct/destruct

OMRCListenerMultiplexerHelper::OMRCListenerMultiplexerHelper(   const   Reference< XWindow >&   xControl    ,
                                                                const   Reference< XWindow >&   xPeer       )
    : m_xPeer           ( xPeer     )
    , m_xControl        ( xControl  )
{
}

OMRCListenerMultiplexerHelper::~OMRCListenerMultiplexerHelper()
{
}

//  container method

void OMRCListenerMultiplexerHelper::setPeer( const Reference< XWindow >& xPeer )
{
    std::unique_lock aGuard(m_aMutex);
    if( m_xPeer == xPeer )
        return;

    if( m_xPeer.is() )
    {
        for_each_container(
            [this, &aGuard]<class Ifc>(const comphelper::OInterfaceContainerHelper4<Ifc>& c)
            {
                if (c.getLength(aGuard) > 0)
                    notifyPeer(m_xPeer, Remove<Ifc>);
            });
    }
    m_xPeer = xPeer;
    if( m_xPeer.is() )
    {
        for_each_container(
            [this, &aGuard]<class Ifc>(const comphelper::OInterfaceContainerHelper4<Ifc>& c)
            {
                if (c.getLength(aGuard) > 0)
                    notifyPeer(m_xPeer, Add<Ifc>);
            });
    }
}

//  container method

void OMRCListenerMultiplexerHelper::disposeAndClear()
{
    std::unique_lock aGuard(m_aMutex);
    EventObject aEvent;
    aEvent.Source = m_xControl;
    for_each_container([&aGuard, &aEvent](auto& c) { c.disposeAndClear(aGuard, aEvent); });
}

//  XEventListener

void SAL_CALL OMRCListenerMultiplexerHelper::disposing( const EventObject& /*aSource*/ )
{
    std::unique_lock aGuard(m_aMutex);
    // peer is disposed, clear the reference
    m_xPeer.clear();
}

//  XFcousListener

void OMRCListenerMultiplexerHelper::focusGained(const FocusEvent& aEvent )
{
    Multiplex(&XFocusListener::focusGained, aEvent);
}

//  XFcousListener

void OMRCListenerMultiplexerHelper::focusLost(const FocusEvent& aEvent )
{
    Multiplex(&XFocusListener::focusLost, aEvent);
}

//  XWindowListener

void OMRCListenerMultiplexerHelper::windowResized(const WindowEvent& aEvent )
{
    Multiplex(&XWindowListener::windowResized, aEvent);
}

//  XWindowListener

void OMRCListenerMultiplexerHelper::windowMoved(const WindowEvent& aEvent )
{
    Multiplex(&XWindowListener::windowMoved, aEvent);
}

//  XWindowListener

void OMRCListenerMultiplexerHelper::windowShown(const EventObject& aEvent )
{
    Multiplex(&XWindowListener::windowShown, aEvent);
}

//  XWindowListener

void OMRCListenerMultiplexerHelper::windowHidden(const EventObject& aEvent )
{
    Multiplex(&XWindowListener::windowHidden, aEvent);
}

//  XKeyListener

void OMRCListenerMultiplexerHelper::keyPressed(const KeyEvent& aEvent)
{
    Multiplex(&XKeyListener::keyPressed, aEvent);
}

//  XKeyListener

void OMRCListenerMultiplexerHelper::keyReleased(const KeyEvent& aEvent)
{
    Multiplex(&XKeyListener::keyReleased, aEvent);
}

//  XMouseListener

void OMRCListenerMultiplexerHelper::mousePressed(const MouseEvent& aEvent)
{
    Multiplex(&XMouseListener::mousePressed, aEvent);
}

//  XMouseListener

void OMRCListenerMultiplexerHelper::mouseReleased(const MouseEvent& aEvent)
{
    Multiplex(&XMouseListener::mouseReleased, aEvent);
}

//  XMouseListener

void OMRCListenerMultiplexerHelper::mouseEntered(const MouseEvent& aEvent)
{
    Multiplex(&XMouseListener::mouseEntered, aEvent);
}

//  XMouseListener

void OMRCListenerMultiplexerHelper::mouseExited(const MouseEvent& aEvent)
{
    Multiplex(&XMouseListener::mouseExited, aEvent);
}

//  XMouseMotionListener

void OMRCListenerMultiplexerHelper::mouseDragged(const MouseEvent& aEvent)
{
    Multiplex(&XMouseMotionListener::mouseDragged, aEvent);
}

//  XMouseMotionListener

void OMRCListenerMultiplexerHelper::mouseMoved(const MouseEvent& aEvent)
{
    Multiplex(&XMouseMotionListener::mouseMoved, aEvent);
}

//  XPaintListener

void OMRCListenerMultiplexerHelper::windowPaint(const PaintEvent& aEvent)
{
    Multiplex(&XPaintListener::windowPaint, aEvent);
}

} // namespace unocontrols

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
