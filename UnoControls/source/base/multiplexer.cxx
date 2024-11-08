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

#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/awt/XTopWindow.hpp>

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
    /* First get all interfaces from container with right type.*/
    auto* pContainer = m_aListenerHolder.getContainer(aGuard, cppu::UnoType<Interface>::get());
    /* Do the follow only, if elements in container exist.*/
    if (!pContainer)
        return;
    comphelper::OInterfaceIteratorHelper4 aIterator(aGuard, *pContainer);
    Event aLocalEvent = event;
    /* Remark: The control is the event source not the peer.*/
    /*         We must change the source of the event.      */
    aLocalEvent.Source = m_xControl;
    aGuard.unlock();
    /* Is the control not destroyed? */
    if (!aLocalEvent.Source)
        return;
    if (!aIterator.hasMoreElements())
        return;
    auto* pListener = aIterator.next().get();
    assert(dynamic_cast<Interface*>(pListener));
    try
    {
        (static_cast<Interface*>(pListener)->*method)(aLocalEvent);
    }
    catch (const RuntimeException&)
    {
        /* Ignore all system exceptions from the listener! */
    }
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
        // get all types from the listener added to the peer
        const std::vector< Type > aContainedTypes = m_aListenerHolder.getContainedTypes(aGuard);
        // loop over all listener types and remove the listeners from the peer
        for( const auto& rContainedType : aContainedTypes )
            impl_unadviseFromPeer( m_xPeer, rContainedType );
    }
    m_xPeer = xPeer;
    if( m_xPeer.is() )
    {
        // get all types from the listener added to the peer
        const std::vector< Type > aContainedTypes = m_aListenerHolder.getContainedTypes(aGuard);
        // loop over all listener types and add the listeners to the peer
        for( const auto& rContainedType : aContainedTypes )
            impl_adviseToPeer( m_xPeer, rContainedType );
    }
}

//  container method

void OMRCListenerMultiplexerHelper::disposeAndClear()
{
    std::unique_lock aGuard(m_aMutex);
    EventObject aEvent;
    aEvent.Source = m_xControl;
    m_aListenerHolder.disposeAndClear(aGuard, aEvent);
}

//  container method

void OMRCListenerMultiplexerHelper::advise( const   Type&                       aType       ,
                                            const   Reference< XEventListener >&    xListener   )
{
    assert(xListener && xListener->queryInterface(aType).getValue());
    std::unique_lock aGuard(m_aMutex);
    if (m_aListenerHolder.addInterface(aGuard, aType, xListener) == 1)
    {
        // the first listener is added
        if( m_xPeer.is() )
        {
            impl_adviseToPeer( m_xPeer, aType );
        }
    }
}

//  container method

void OMRCListenerMultiplexerHelper::unadvise(   const   Type&                       aType       ,
                                                const   Reference< XEventListener >&    xListener   )
{
    std::unique_lock aGuard(m_aMutex);
    if (m_aListenerHolder.removeInterface(aGuard, aType, xListener) == 0)
    {
        // the last listener is removed
        if ( m_xPeer.is() )
        {
            impl_unadviseFromPeer( m_xPeer, aType );
        }
    }
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

//  XTopWindowListener

void OMRCListenerMultiplexerHelper::windowOpened(const EventObject& aEvent)
{
    Multiplex(&XTopWindowListener::windowOpened, aEvent);
}

//  XTopWindowListener

void OMRCListenerMultiplexerHelper::windowClosing( const EventObject& aEvent )
{
    Multiplex(&XTopWindowListener::windowClosing, aEvent);
}

//  XTopWindowListener

void OMRCListenerMultiplexerHelper::windowClosed( const EventObject& aEvent )
{
    Multiplex(&XTopWindowListener::windowClosed, aEvent);
}

//  XTopWindowListener

void OMRCListenerMultiplexerHelper::windowMinimized( const EventObject& aEvent )
{
    Multiplex(&XTopWindowListener::windowMinimized, aEvent);
}

//  XTopWindowListener

void OMRCListenerMultiplexerHelper::windowNormalized( const EventObject& aEvent )
{
    Multiplex(&XTopWindowListener::windowNormalized, aEvent);
}

//  XTopWindowListener

void OMRCListenerMultiplexerHelper::windowActivated( const EventObject& aEvent )
{
    Multiplex(&XTopWindowListener::windowActivated, aEvent);
}

//  XTopWindowListener

void OMRCListenerMultiplexerHelper::windowDeactivated( const EventObject& aEvent )
{
    Multiplex(&XTopWindowListener::windowDeactivated, aEvent);
}

//  protected method

void OMRCListenerMultiplexerHelper::impl_adviseToPeer(  const   Reference< XWindow >&   xPeer   ,
                                                        const   Type&                   aType   )
{
    // add a listener to the source (peer)
    if( aType == cppu::UnoType<XWindowListener>::get())
        xPeer->addWindowListener( this );
    else if( aType == cppu::UnoType<XKeyListener>::get())
        xPeer->addKeyListener( this );
    else if( aType == cppu::UnoType<XFocusListener>::get())
        xPeer->addFocusListener( this );
    else if( aType == cppu::UnoType<XMouseListener>::get())
        xPeer->addMouseListener( this );
    else if( aType == cppu::UnoType<XMouseMotionListener>::get())
        xPeer->addMouseMotionListener( this );
    else if( aType == cppu::UnoType<XPaintListener>::get())
        xPeer->addPaintListener( this );
    else if( aType == cppu::UnoType<XTopWindowListener>::get())
    {
        Reference< XTopWindow > xTop( xPeer, UNO_QUERY );
        if( xTop.is() )
            xTop->addTopWindowListener( this );
    }
    else
    {
        OSL_FAIL( "unknown listener" );
    }
}

//  protected method

void OMRCListenerMultiplexerHelper::impl_unadviseFromPeer(  const   Reference< XWindow >&   xPeer   ,
                                                            const   Type&                   aType   )
{
    // the last listener is removed, remove the listener from the source (peer)
    if( aType == cppu::UnoType<XWindowListener>::get())
        xPeer->removeWindowListener( this );
    else if( aType == cppu::UnoType<XKeyListener>::get())
        xPeer->removeKeyListener( this );
    else if( aType == cppu::UnoType<XFocusListener>::get())
        xPeer->removeFocusListener( this );
    else if( aType == cppu::UnoType<XMouseListener>::get())
        xPeer->removeMouseListener( this );
    else if( aType == cppu::UnoType<XMouseMotionListener>::get())
        xPeer->removeMouseMotionListener( this );
    else if( aType == cppu::UnoType<XPaintListener>::get())
        xPeer->removePaintListener( this );
    else if( aType == cppu::UnoType<XTopWindowListener>::get())
    {
        Reference< XTopWindow >  xTop( xPeer, UNO_QUERY );
        if( xTop.is() )
            xTop->removeTopWindowListener( this );
    }
    else
    {
        OSL_FAIL( "unknown listener" );
    }
}

} // namespace unocontrols

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
