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

//  macros

#define MULTIPLEX( INTERFACE, METHOD, EVENTTYP, EVENT )                                                                             \
                                                                                                                                    \
    /* First get all interfaces from container with right type.*/                                                                   \
    OInterfaceContainerHelper* pContainer = m_aListenerHolder.getContainer( cppu::UnoType<INTERFACE>::get() );  \
    /* Do the follow only, if elements in container exist.*/                                                                        \
    if( !pContainer )                                                                                                        \
        return;                                                                                                                               \
    OInterfaceIteratorHelper aIterator( *pContainer );                                                                          \
    EVENTTYP aLocalEvent = EVENT;                                                                                               \
    /* Remark: The control is the event source not the peer.*/                                                                  \
    /*         We must change the source of the event.      */                                                                  \
    aLocalEvent.Source = m_xControl;                                                                                           \
    /* Is the control not destroyed? */                                                                                         \
    if( !aLocalEvent.Source )                                                                                               \
        return;                                                                                                                           \
    if( !aIterator.hasMoreElements() )                                                                                       \
        return;                                                                                                             \
    INTERFACE * pListener = static_cast<INTERFACE *>(aIterator.next());                                                 \
    try                                                                                                                 \
    {                                                                                                                   \
        pListener->METHOD( aLocalEvent );                                                                               \
    }                                                                                                                   \
    catch(const RuntimeException& )                                                                                     \
    {                                                                                                                   \
        /* Ignore all system exceptions from the listener! */                                                           \
    }

//  construct/destruct

OMRCListenerMultiplexerHelper::OMRCListenerMultiplexerHelper(   const   Reference< XWindow >&   xControl    ,
                                                                const   Reference< XWindow >&   xPeer       )
    : m_xPeer           ( xPeer     )
    , m_xControl        ( xControl  )
    , m_aListenerHolder ( m_aMutex  )
{
}

OMRCListenerMultiplexerHelper::OMRCListenerMultiplexerHelper( const OMRCListenerMultiplexerHelper& aCopyInstance )
    : XFocusListener()
    , XWindowListener()
    , XKeyListener()
    , XMouseListener()
    , XMouseMotionListener()
    , XPaintListener()
    , XTopWindowListener()
    , OWeakObject(aCopyInstance)
    , m_aListenerHolder ( m_aMutex  )
{
}

OMRCListenerMultiplexerHelper::~OMRCListenerMultiplexerHelper()
{
}

//  XInterface

Any SAL_CALL OMRCListenerMultiplexerHelper::queryInterface( const Type& rType )
{
    // Attention:
    //  Don't use mutex or guard in this method!!! Is a method of XInterface.

    // Ask for my own supported interfaces ...
    // Attention: XTypeProvider and XInterface are supported by OComponentHelper!
    Any aReturn ( ::cppu::queryInterface(   rType                                           ,
                                            static_cast< XWindowListener*       > ( this )  ,
                                            static_cast< XKeyListener*          > ( this )  ,
                                            static_cast< XFocusListener*        > ( this )  ,
                                            static_cast< XMouseListener*        > ( this )  ,
                                            static_cast< XMouseMotionListener*  > ( this )  ,
                                            static_cast< XPaintListener*        > ( this )  ,
                                            static_cast< XTopWindowListener*    > ( this )  ,
                                            static_cast< XTopWindowListener*    > ( this )
                                        )
                );

    // If searched interface supported by this class ...
    if ( aReturn.hasValue() )
    {
        // ... return this information.
        return aReturn;
    }
    else
    {
        // Else; ... ask baseclass for interfaces!
        return OWeakObject::queryInterface( rType );
    }
}

//  XInterface

void SAL_CALL OMRCListenerMultiplexerHelper::acquire() noexcept
{
    // Attention:
    //  Don't use mutex or guard in this method!!! Is a method of XInterface.

    // Forward to baseclass
    OWeakObject::acquire();
}

//  XInterface

void SAL_CALL OMRCListenerMultiplexerHelper::release() noexcept
{
    // Attention:
    //  Don't use mutex or guard in this method!!! Is a method of XInterface.

    // Forward to baseclass
    OWeakObject::release();
}

//  container method

void OMRCListenerMultiplexerHelper::setPeer( const Reference< XWindow >& xPeer )
{
    MutexGuard aGuard( m_aMutex );
    if( m_xPeer == xPeer )
        return;

    if( m_xPeer.is() )
    {
        // get all types from the listener added to the peer
        const Sequence< Type >    aContainedTypes = m_aListenerHolder.getContainedTypes();
        // loop over all listener types and remove the listeners from the peer
        for( const auto& rContainedType : aContainedTypes )
            impl_unadviseFromPeer( m_xPeer, rContainedType );
    }
    m_xPeer = xPeer;
    if( m_xPeer.is() )
    {
        // get all types from the listener added to the peer
        const Sequence< Type >    aContainedTypes = m_aListenerHolder.getContainedTypes();
        // loop over all listener types and add the listeners to the peer
        for( const auto& rContainedType : aContainedTypes )
            impl_adviseToPeer( m_xPeer, rContainedType );
    }
}

//  container method

void OMRCListenerMultiplexerHelper::disposeAndClear()
{
    EventObject aEvent;
    aEvent.Source = m_xControl;
    m_aListenerHolder.disposeAndClear( aEvent );
}

//  container method

void OMRCListenerMultiplexerHelper::advise( const   Type&                       aType       ,
                                            const   Reference< XInterface >&    xListener   )
{
    MutexGuard aGuard( m_aMutex );
    if( m_aListenerHolder.addInterface( aType, xListener ) == 1 )
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
                                                const   Reference< XInterface >&    xListener   )
{
    MutexGuard aGuard( m_aMutex );
    if( m_aListenerHolder.removeInterface( aType, xListener ) == 0 )
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
    MutexGuard aGuard( m_aMutex );
    // peer is disposed, clear the reference
    m_xPeer.clear();
}

//  XFcousListener

void OMRCListenerMultiplexerHelper::focusGained(const FocusEvent& aEvent )
{
    MULTIPLEX( XFocusListener, focusGained, FocusEvent, aEvent )
}

//  XFcousListener

void OMRCListenerMultiplexerHelper::focusLost(const FocusEvent& aEvent )
{
    MULTIPLEX( XFocusListener, focusLost, FocusEvent, aEvent )
}

//  XWindowListener

void OMRCListenerMultiplexerHelper::windowResized(const WindowEvent& aEvent )
{
    MULTIPLEX( XWindowListener, windowResized, WindowEvent, aEvent )
}

//  XWindowListener

void OMRCListenerMultiplexerHelper::windowMoved(const WindowEvent& aEvent )
{
    MULTIPLEX( XWindowListener, windowMoved, WindowEvent, aEvent )
}

//  XWindowListener

void OMRCListenerMultiplexerHelper::windowShown(const EventObject& aEvent )
{
    MULTIPLEX( XWindowListener, windowShown, EventObject, aEvent )
}

//  XWindowListener

void OMRCListenerMultiplexerHelper::windowHidden(const EventObject& aEvent )
{
    MULTIPLEX( XWindowListener, windowHidden, EventObject, aEvent )
}

//  XKeyListener

void OMRCListenerMultiplexerHelper::keyPressed(const KeyEvent& aEvent)
{
    MULTIPLEX( XKeyListener, keyPressed, KeyEvent, aEvent )
}

//  XKeyListener

void OMRCListenerMultiplexerHelper::keyReleased(const KeyEvent& aEvent)
{
    MULTIPLEX( XKeyListener, keyReleased, KeyEvent, aEvent )
}

//  XMouseListener

void OMRCListenerMultiplexerHelper::mousePressed(const MouseEvent& aEvent)
{
    MULTIPLEX( XMouseListener, mousePressed, MouseEvent, aEvent )
}

//  XMouseListener

void OMRCListenerMultiplexerHelper::mouseReleased(const MouseEvent& aEvent)
{
    MULTIPLEX( XMouseListener, mouseReleased, MouseEvent, aEvent )
}

//  XMouseListener

void OMRCListenerMultiplexerHelper::mouseEntered(const MouseEvent& aEvent)
{
    MULTIPLEX( XMouseListener, mouseEntered, MouseEvent, aEvent )
}

//  XMouseListener

void OMRCListenerMultiplexerHelper::mouseExited(const MouseEvent& aEvent)
{
    MULTIPLEX( XMouseListener, mouseExited, MouseEvent, aEvent )
}

//  XMouseMotionListener

void OMRCListenerMultiplexerHelper::mouseDragged(const MouseEvent& aEvent)
{
    MULTIPLEX( XMouseMotionListener, mouseDragged, MouseEvent, aEvent )
}

//  XMouseMotionListener

void OMRCListenerMultiplexerHelper::mouseMoved(const MouseEvent& aEvent)
{
    MULTIPLEX( XMouseMotionListener, mouseMoved, MouseEvent, aEvent )
}

//  XPaintListener

void OMRCListenerMultiplexerHelper::windowPaint(const PaintEvent& aEvent)
{
    MULTIPLEX( XPaintListener, windowPaint, PaintEvent, aEvent )
}

//  XTopWindowListener

void OMRCListenerMultiplexerHelper::windowOpened(const EventObject& aEvent)
{
    MULTIPLEX( XTopWindowListener, windowOpened, EventObject, aEvent )
}

//  XTopWindowListener

void OMRCListenerMultiplexerHelper::windowClosing( const EventObject& aEvent )
{
    MULTIPLEX( XTopWindowListener, windowClosing, EventObject, aEvent )
}

//  XTopWindowListener

void OMRCListenerMultiplexerHelper::windowClosed( const EventObject& aEvent )
{
    MULTIPLEX( XTopWindowListener, windowClosed, EventObject, aEvent )
}

//  XTopWindowListener

void OMRCListenerMultiplexerHelper::windowMinimized( const EventObject& aEvent )
{
    MULTIPLEX( XTopWindowListener, windowMinimized, EventObject, aEvent )
}

//  XTopWindowListener

void OMRCListenerMultiplexerHelper::windowNormalized( const EventObject& aEvent )
{
    MULTIPLEX( XTopWindowListener, windowNormalized, EventObject, aEvent )
}

//  XTopWindowListener

void OMRCListenerMultiplexerHelper::windowActivated( const EventObject& aEvent )
{
    MULTIPLEX( XTopWindowListener, windowActivated, EventObject, aEvent )
}

//  XTopWindowListener

void OMRCListenerMultiplexerHelper::windowDeactivated( const EventObject& aEvent )
{
    MULTIPLEX( XTopWindowListener, windowDeactivated, EventObject, aEvent )
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
