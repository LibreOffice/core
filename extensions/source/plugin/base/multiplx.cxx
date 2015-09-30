/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include <osl/diagnose.h>
#include <plugin/multiplx.hxx>


//  class MRCListenerMultiplexerHelper

MRCListenerMultiplexerHelper::MRCListenerMultiplexerHelper
(
      const Reference< css::awt::XWindow >  & rControl
    , const Reference< css::awt::XWindow >  & rPeer
)
    : xPeer( rPeer )
    , xControl( Reference< css::awt::XControl >( rControl, UNO_QUERY ) )
    , aListenerHolder( aMutex )
{
}


void MRCListenerMultiplexerHelper::setPeer( const Reference< css::awt::XWindow >  & rPeer )
{
    ::osl::Guard< ::osl::Mutex > aGuard( aMutex );
    if( xPeer != rPeer )
    {
        if( xPeer.is() )
        {
            // get all uiks from the listener added to the peer
            Sequence<Type> aContainedTypes = aListenerHolder.getContainedTypes();
            const Type* pArray = aContainedTypes.getConstArray();
            sal_Int32 nCount = aContainedTypes.getLength();
            // loop over all listener types and remove the listeners from the peer
            for( sal_Int32 i = 0; i < nCount; i++ )
                unadviseFromPeer( xPeer, pArray[i] );
        }
        xPeer = rPeer;
        if( xPeer.is() )
        {
            // get all uiks from the listener added to the peer
            Sequence<Type> aContainedTypes = aListenerHolder.getContainedTypes();
            const Type * pArray = aContainedTypes.getConstArray();
            sal_Int32 nCount = aContainedTypes.getLength();
            // loop over all listener types and add the listeners to the peer
            for( sal_Int32 i = 0; i < nCount; i++ )
                adviseToPeer( xPeer, pArray[i] );
        }
    }
}

// MRCListenerMultiplexerHelper
void MRCListenerMultiplexerHelper::disposeAndClear()
{
    css::lang::EventObject aEvt;
    aEvt.Source = xControl;
    aListenerHolder.disposeAndClear( aEvt );
}

// MRCListenerMultiplexerHelper
void MRCListenerMultiplexerHelper::adviseToPeer( const Reference< css::awt::XWindow >  & rPeer, const Type & type )
{
    // add a listener to the source (peer)
    if( type == cppu::UnoType<css::awt::XWindowListener>::get())
        rPeer->addWindowListener( this );
    else if( type == cppu::UnoType<css::awt::XKeyListener>::get())
        rPeer->addKeyListener( this );
    else if( type == cppu::UnoType<css::awt::XFocusListener>::get())
        rPeer->addFocusListener( this );
    else if( type == cppu::UnoType<css::awt::XMouseListener>::get())
        rPeer->addMouseListener( this );
    else if( type == cppu::UnoType<css::awt::XMouseMotionListener>::get())
        rPeer->addMouseMotionListener( this );
    else if( type == cppu::UnoType<css::awt::XPaintListener>::get())
        rPeer->addPaintListener( this );
    else if( type == cppu::UnoType<css::awt::XTopWindowListener>::get())
    {
        Reference< css::awt::XTopWindow >  xTop( rPeer, UNO_QUERY );
        if( xTop.is() )
            xTop->addTopWindowListener( this );
    }
    else
    {
        OSL_FAIL( "unknown listener" );
    }
}

// MRCListenerMultiplexerHelper
void MRCListenerMultiplexerHelper::unadviseFromPeer( const Reference< css::awt::XWindow >  & rPeer, const Type & type )
{
    // the last listener is removed, remove the listener from the source (peer)
    if( type == cppu::UnoType<css::awt::XWindowListener>::get())
        rPeer->removeWindowListener( this );
    else if( type == cppu::UnoType<css::awt::XKeyListener>::get())
        rPeer->removeKeyListener( this );
    else if( type == cppu::UnoType<css::awt::XFocusListener>::get())
        rPeer->removeFocusListener( this );
    else if( type == cppu::UnoType<css::awt::XMouseListener>::get())
        rPeer->removeMouseListener( this );
    else if( type == cppu::UnoType<css::awt::XMouseMotionListener>::get())
        rPeer->removeMouseMotionListener( this );
    else if( type == cppu::UnoType<css::awt::XPaintListener>::get())
        rPeer->removePaintListener( this );
    else if( type == cppu::UnoType<css::awt::XTopWindowListener>::get())
    {
        Reference< css::awt::XTopWindow >  xTop( rPeer, UNO_QUERY );
        if( xTop.is() )
            xTop->removeTopWindowListener( this );
    }
    else
    {
        OSL_FAIL( "unknown listener" );
    }
}

// MRCListenerMultiplexerHelper
void MRCListenerMultiplexerHelper::advise( const Type & type, const Reference< XInterface > & listener)
{
    ::osl::Guard< ::osl::Mutex > aGuard( aMutex );
    if( 1 == aListenerHolder.addInterface( type, listener ) )
    {
        // the first listener is added
        if( xPeer.is() )
            adviseToPeer( xPeer, type );
    }
}

// MRCListenerMultiplexerHelper
void MRCListenerMultiplexerHelper::unadvise(const Type & type, const Reference< XInterface > & listener)
{
    ::osl::Guard< ::osl::Mutex > aGuard( aMutex );
    ::cppu::OInterfaceContainerHelper * pCont = aListenerHolder.getContainer( type );
    if( pCont )
    {
        if( 0 == pCont->removeInterface( listener ) && xPeer.is() )
            // the last listener is removed
            unadviseFromPeer( xPeer, type );
    }
}

// css::lang::XEventListener
void MRCListenerMultiplexerHelper::disposing(const css::lang::EventObject& ) throw(std::exception)
{
    ::osl::Guard< ::osl::Mutex > aGuard( aMutex );
    // peer is disposed, clear the reference
    xPeer = Reference< css::awt::XWindow > ();
}

#define MULTIPLEX( InterfaceName, MethodName, EventName )                   \
::cppu::OInterfaceContainerHelper * pCont;                                          \
pCont = aListenerHolder.getContainer( cppu::UnoType< InterfaceName >::get());        \
if( pCont )                                                                 \
{                                                                           \
    ::cppu::OInterfaceIteratorHelper    aIt( *pCont );                              \
    EventName aEvt = e;                                                     \
    /* Remark: The control is the event source not the peer. We must change */  \
    /* the source of the event */                                               \
    aEvt.Source = xControl;\
    /*.is the control not destroyed */                                      \
    if( aEvt.Source.is() )                                                  \
    {                                                                       \
        if( aIt.hasMoreElements() )                                         \
        {                                                                   \
            InterfaceName * pListener = static_cast<InterfaceName *>(aIt.next()); \
            try                                                             \
            {                                                               \
                pListener->MethodName( aEvt );                              \
            }                                                               \
            catch(const RuntimeException&)                                  \
            {                                                               \
                /* ignore all usr system exceptions from the listener */    \
            }                                                               \
        }                                                                   \
    }                                                                       \
}

// css::awt::XFocusListener
void MRCListenerMultiplexerHelper::focusGained(const css::awt::FocusEvent& e) throw(std::exception)
{
    MULTIPLEX( css::awt::XFocusListener, focusGained, css::awt::FocusEvent )
}

// css::awt::XFocusListener
void MRCListenerMultiplexerHelper::focusLost(const css::awt::FocusEvent& e) throw(std::exception)
{
    MULTIPLEX( css::awt::XFocusListener, focusLost, css::awt::FocusEvent )
}

// css::awt::XWindowListener
void MRCListenerMultiplexerHelper::windowResized(const css::awt::WindowEvent& e) throw(std::exception)
{
    MULTIPLEX( css::awt::XWindowListener, windowResized, css::awt::WindowEvent )
}

// css::awt::XWindowListener
void MRCListenerMultiplexerHelper::windowMoved(const css::awt::WindowEvent& e) throw(std::exception)
{
    MULTIPLEX( css::awt::XWindowListener, windowMoved, css::awt::WindowEvent )
}

// css::awt::XWindowListener
void MRCListenerMultiplexerHelper::windowShown(const css::lang::EventObject& e) throw(std::exception)
{
    MULTIPLEX( css::awt::XWindowListener, windowShown, css::lang::EventObject )
}

// css::awt::XWindowListener
void MRCListenerMultiplexerHelper::windowHidden(const css::lang::EventObject& e) throw(std::exception)
{
    MULTIPLEX( css::awt::XWindowListener, windowHidden, css::lang::EventObject )
}

// css::awt::XKeyListener
void MRCListenerMultiplexerHelper::keyPressed(const css::awt::KeyEvent& e) throw(std::exception)
{
    MULTIPLEX( css::awt::XKeyListener, keyPressed, css::awt::KeyEvent )
}

// css::awt::XKeyListener
void MRCListenerMultiplexerHelper::keyReleased(const css::awt::KeyEvent& e) throw(std::exception)
{
    MULTIPLEX( css::awt::XKeyListener, keyReleased, css::awt::KeyEvent )
}

// css::awt::XMouseListener
void MRCListenerMultiplexerHelper::mousePressed(const css::awt::MouseEvent& e) throw(std::exception)
{
    MULTIPLEX( css::awt::XMouseListener, mousePressed, css::awt::MouseEvent )
}

// css::awt::XMouseListener
void MRCListenerMultiplexerHelper::mouseReleased(const css::awt::MouseEvent& e) throw(std::exception)
{
    MULTIPLEX( css::awt::XMouseListener, mouseReleased, css::awt::MouseEvent )
}

// css::awt::XMouseListener
void MRCListenerMultiplexerHelper::mouseEntered(const css::awt::MouseEvent& e) throw(std::exception)
{
    MULTIPLEX( css::awt::XMouseListener, mouseEntered, css::awt::MouseEvent )
}

// css::awt::XMouseListener
void MRCListenerMultiplexerHelper::mouseExited(const css::awt::MouseEvent& e) throw(std::exception)
{
    MULTIPLEX( css::awt::XMouseListener, mouseExited, css::awt::MouseEvent )
}

// css::awt::XMouseMotionListener
void MRCListenerMultiplexerHelper::mouseDragged(const css::awt::MouseEvent& e) throw(std::exception)
{
    MULTIPLEX( css::awt::XMouseMotionListener, mouseDragged, css::awt::MouseEvent )
}

// css::awt::XMouseMotionListener
void MRCListenerMultiplexerHelper::mouseMoved(const css::awt::MouseEvent& e) throw(std::exception)
{
    MULTIPLEX( css::awt::XMouseMotionListener, mouseMoved, css::awt::MouseEvent )
}

// css::awt::XPaintListener
void MRCListenerMultiplexerHelper::windowPaint(const css::awt::PaintEvent& e) throw(std::exception)
{
    MULTIPLEX( css::awt::XPaintListener, windowPaint, css::awt::PaintEvent )
}

// css::awt::XTopWindowListener
void MRCListenerMultiplexerHelper::windowOpened(const css::lang::EventObject& e) throw(std::exception)
{
    MULTIPLEX( css::awt::XTopWindowListener, windowOpened, css::lang::EventObject )
}

// css::awt::XTopWindowListener
void MRCListenerMultiplexerHelper::windowClosing( const css::lang::EventObject& e ) throw(std::exception)
{
    MULTIPLEX( css::awt::XTopWindowListener, windowClosing, css::lang::EventObject )
}

// css::awt::XTopWindowListener
void MRCListenerMultiplexerHelper::windowClosed( const css::lang::EventObject& e ) throw(std::exception)
{
    MULTIPLEX( css::awt::XTopWindowListener, windowClosed, css::lang::EventObject )
}

// css::awt::XTopWindowListener
void MRCListenerMultiplexerHelper::windowMinimized( const css::lang::EventObject& e ) throw(std::exception)
{
    MULTIPLEX( css::awt::XTopWindowListener, windowMinimized, css::lang::EventObject )
}

// css::awt::XTopWindowListener
void MRCListenerMultiplexerHelper::windowNormalized( const css::lang::EventObject& e ) throw(std::exception)
{
    MULTIPLEX( css::awt::XTopWindowListener, windowNormalized, css::lang::EventObject )
}

// css::awt::XTopWindowListener
void MRCListenerMultiplexerHelper::windowActivated( const css::lang::EventObject& e ) throw(std::exception)
{
    MULTIPLEX( css::awt::XTopWindowListener, windowActivated, css::lang::EventObject )
}

// css::awt::XTopWindowListener
void MRCListenerMultiplexerHelper::windowDeactivated( const css::lang::EventObject& e ) throw(std::exception)
{
    MULTIPLEX( css::awt::XTopWindowListener, windowDeactivated, css::lang::EventObject )
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
