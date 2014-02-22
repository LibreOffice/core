/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include "multiplexer.hxx"

#include <osl/diagnose.h>

using namespace ::cppu                  ;
using namespace ::osl                   ;
using namespace ::com::sun::star::uno   ;
using namespace ::com::sun::star::awt   ;
using namespace ::com::sun::star::lang  ;

namespace unocontrols{





#define MULTIPLEX( INTERFACE, METHOD, EVENTTYP, EVENT )                                                                             \
                                                                                                                                    \
    /* First get all interfaces from container with right type.*/                                                                   \
    OInterfaceContainerHelper* pContainer = m_aListenerHolder.getContainer( ::getCppuType((const Reference< INTERFACE >*)0) );  \
    /* Do the follow only, if elements in container exist.*/                                                                        \
    if( pContainer != NULL )                                                                                                        \
    {                                                                                                                               \
        OInterfaceIteratorHelper aIterator( *pContainer );                                                                          \
        EVENTTYP aLocalEvent = EVENT;                                                                                               \
        /* Remark: The control is the event source not the peer.*/                                                                  \
        /*         We must change the source of the event.      */                                                                  \
        aLocalEvent.Source = m_xControl ;                                                                                           \
        /* Is the control not destroyed? */                                                                                         \
        if( aLocalEvent.Source.is() )                                                                                               \
        {                                                                                                                           \
            if( aIterator.hasMoreElements() )                                                                                       \
            {                                                                                                                       \
                INTERFACE * pListener = (INTERFACE *)aIterator.next();                                                          \
                try                                                                                                                 \
                {                                                                                                                   \
                    pListener->METHOD( aLocalEvent );                                                                               \
                }                                                                                                                   \
                catch(const RuntimeException& )                                                                                     \
                {                                                                                                                   \
                    /* Ignore all system exceptions from the listener! */                                                           \
                }                                                                                                                   \
            }                                                                                                                       \
        }                                                                                                                           \
    }





OMRCListenerMultiplexerHelper::OMRCListenerMultiplexerHelper(   const   Reference< XWindow >&   xControl    ,
                                                                const   Reference< XWindow >&   xPeer       )
    : m_xPeer           ( xPeer     )
    , m_xControl        ( xControl  )
    , m_aListenerHolder ( m_aMutex  )
{
}

OMRCListenerMultiplexerHelper::OMRCListenerMultiplexerHelper( const OMRCListenerMultiplexerHelper& /*aCopyInstance*/ )
    : XFocusListener()
    , XWindowListener()
    , XKeyListener()
    , XMouseListener()
    , XMouseMotionListener()
    , XPaintListener()
    , XTopWindowListener()
    , OWeakObject()
    , m_aListenerHolder ( m_aMutex  )
{
}

OMRCListenerMultiplexerHelper::~OMRCListenerMultiplexerHelper()
{
}





Any SAL_CALL OMRCListenerMultiplexerHelper::queryInterface( const Type& rType ) throw( RuntimeException )
{
    
    

    
    
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

    
    if ( aReturn.hasValue() )
    {
        
        return aReturn ;
    }
    else
    {
        
        return OWeakObject::queryInterface( rType );
    }
}





void SAL_CALL OMRCListenerMultiplexerHelper::acquire() throw()
{
    
    

    
    OWeakObject::acquire();
}





void SAL_CALL OMRCListenerMultiplexerHelper::release() throw()
{
    
    

    
    OWeakObject::release();
}





OMRCListenerMultiplexerHelper::operator Reference< XInterface >() const
{
    return ((OWeakObject*)this) ;
}





void OMRCListenerMultiplexerHelper::setPeer( const Reference< XWindow >& xPeer )
{
    MutexGuard aGuard( m_aMutex );
    if( m_xPeer != xPeer )
    {
        if( m_xPeer.is() )
        {
            
            Sequence< Type >    aContainedTypes = m_aListenerHolder.getContainedTypes();
            const Type*         pArray          = aContainedTypes.getConstArray();
            sal_Int32           nCount          = aContainedTypes.getLength();
            
            for( sal_Int32 i=0; i<nCount; i++ )
                impl_unadviseFromPeer( m_xPeer, pArray[i] );
        }
        m_xPeer = xPeer;
        if( m_xPeer.is() )
        {
            
            Sequence< Type >    aContainedTypes = m_aListenerHolder.getContainedTypes();
            const Type*         pArray          = aContainedTypes.getConstArray();
            sal_Int32           nCount          = aContainedTypes.getLength();
            
            for( sal_Int32 i = 0; i < nCount; i++ )
                impl_adviseToPeer( m_xPeer, pArray[i] );
        }
    }
}





void OMRCListenerMultiplexerHelper::disposeAndClear()
{
    EventObject aEvent ;
    aEvent.Source = m_xControl ;
    m_aListenerHolder.disposeAndClear( aEvent );
}





void OMRCListenerMultiplexerHelper::advise( const   Type&                       aType       ,
                                            const   Reference< XInterface >&    xListener   )
{
    MutexGuard aGuard( m_aMutex );
    if( m_aListenerHolder.addInterface( aType, xListener ) == 1 )
    {
        
        if( m_xPeer.is() )
        {
            impl_adviseToPeer( m_xPeer, aType );
        }
    }
}





void OMRCListenerMultiplexerHelper::unadvise(   const   Type&                       aType       ,
                                                const   Reference< XInterface >&    xListener   )
{
    MutexGuard aGuard( m_aMutex );
    if( m_aListenerHolder.removeInterface( aType, xListener ) == 0 )
    {
        
        if ( m_xPeer.is() )
        {
            impl_unadviseFromPeer( m_xPeer, aType );
        }
    }
}





void SAL_CALL OMRCListenerMultiplexerHelper::disposing( const EventObject& /*aSource*/ ) throw( RuntimeException )
{
    MutexGuard aGuard( m_aMutex );
    
    m_xPeer.clear();
}





void OMRCListenerMultiplexerHelper::focusGained(const FocusEvent& aEvent ) throw( RuntimeException )
{
    MULTIPLEX( XFocusListener, focusGained, FocusEvent, aEvent )
}





void OMRCListenerMultiplexerHelper::focusLost(const FocusEvent& aEvent ) throw( RuntimeException )
{
    MULTIPLEX( XFocusListener, focusLost, FocusEvent, aEvent )
}





void OMRCListenerMultiplexerHelper::windowResized(const WindowEvent& aEvent ) throw( RuntimeException )
{
    MULTIPLEX( XWindowListener, windowResized, WindowEvent, aEvent )
}





void OMRCListenerMultiplexerHelper::windowMoved(const WindowEvent& aEvent ) throw( RuntimeException )
{
    MULTIPLEX( XWindowListener, windowMoved, WindowEvent, aEvent )
}





void OMRCListenerMultiplexerHelper::windowShown(const EventObject& aEvent ) throw( RuntimeException )
{
    MULTIPLEX( XWindowListener, windowShown, EventObject, aEvent )
}





void OMRCListenerMultiplexerHelper::windowHidden(const EventObject& aEvent ) throw( RuntimeException )
{
    MULTIPLEX( XWindowListener, windowHidden, EventObject, aEvent )
}





void OMRCListenerMultiplexerHelper::keyPressed(const KeyEvent& aEvent) throw( RuntimeException )
{
    MULTIPLEX( XKeyListener, keyPressed, KeyEvent, aEvent )
}





void OMRCListenerMultiplexerHelper::keyReleased(const KeyEvent& aEvent) throw( RuntimeException )
{
    MULTIPLEX( XKeyListener, keyReleased, KeyEvent, aEvent )
}





void OMRCListenerMultiplexerHelper::mousePressed(const MouseEvent& aEvent) throw( RuntimeException )
{
    MULTIPLEX( XMouseListener, mousePressed, MouseEvent, aEvent )
}





void OMRCListenerMultiplexerHelper::mouseReleased(const MouseEvent& aEvent) throw( RuntimeException )
{
    MULTIPLEX( XMouseListener, mouseReleased, MouseEvent, aEvent )
}





void OMRCListenerMultiplexerHelper::mouseEntered(const MouseEvent& aEvent) throw( RuntimeException )
{
    MULTIPLEX( XMouseListener, mouseEntered, MouseEvent, aEvent )
}





void OMRCListenerMultiplexerHelper::mouseExited(const MouseEvent& aEvent) throw( RuntimeException )
{
    MULTIPLEX( XMouseListener, mouseExited, MouseEvent, aEvent )
}





void OMRCListenerMultiplexerHelper::mouseDragged(const MouseEvent& aEvent) throw( RuntimeException )
{
    MULTIPLEX( XMouseMotionListener, mouseDragged, MouseEvent, aEvent )
}





void OMRCListenerMultiplexerHelper::mouseMoved(const MouseEvent& aEvent) throw( RuntimeException )
{
    MULTIPLEX( XMouseMotionListener, mouseMoved, MouseEvent, aEvent )
}





void OMRCListenerMultiplexerHelper::windowPaint(const PaintEvent& aEvent) throw( RuntimeException )
{
    MULTIPLEX( XPaintListener, windowPaint, PaintEvent, aEvent )
}





void OMRCListenerMultiplexerHelper::windowOpened(const EventObject& aEvent) throw( RuntimeException )
{
    MULTIPLEX( XTopWindowListener, windowOpened, EventObject, aEvent )
}





void OMRCListenerMultiplexerHelper::windowClosing( const EventObject& aEvent ) throw( RuntimeException )
{
    MULTIPLEX( XTopWindowListener, windowClosing, EventObject, aEvent )
}





void OMRCListenerMultiplexerHelper::windowClosed( const EventObject& aEvent ) throw( RuntimeException )
{
    MULTIPLEX( XTopWindowListener, windowClosed, EventObject, aEvent )
}





void OMRCListenerMultiplexerHelper::windowMinimized( const EventObject& aEvent ) throw( RuntimeException )
{
    MULTIPLEX( XTopWindowListener, windowMinimized, EventObject, aEvent )
}





void OMRCListenerMultiplexerHelper::windowNormalized( const EventObject& aEvent ) throw( RuntimeException )
{
    MULTIPLEX( XTopWindowListener, windowNormalized, EventObject, aEvent )
}





void OMRCListenerMultiplexerHelper::windowActivated( const EventObject& aEvent ) throw( RuntimeException )
{
    MULTIPLEX( XTopWindowListener, windowActivated, EventObject, aEvent )
}





void OMRCListenerMultiplexerHelper::windowDeactivated( const EventObject& aEvent ) throw( RuntimeException )
{
    MULTIPLEX( XTopWindowListener, windowDeactivated, EventObject, aEvent )
}





void OMRCListenerMultiplexerHelper::impl_adviseToPeer(  const   Reference< XWindow >&   xPeer   ,
                                                        const   Type&                   aType   )
{
    
    if( aType == ::getCppuType((const Reference< XWindowListener >*)0) )
        xPeer->addWindowListener( this );
    else if( aType == ::getCppuType((const Reference< XKeyListener >*)0) )
        xPeer->addKeyListener( this );
    else if( aType == ::getCppuType((const Reference< XFocusListener >*)0) )
        xPeer->addFocusListener( this );
    else if( aType == ::getCppuType((const Reference< XMouseListener >*)0) )
        xPeer->addMouseListener( this );
    else if( aType == ::getCppuType((const Reference< XMouseMotionListener >*)0) )
        xPeer->addMouseMotionListener( this );
    else if( aType == ::getCppuType((const Reference< XPaintListener >*)0) )
        xPeer->addPaintListener( this );
    else if( aType == ::getCppuType((const Reference< XTopWindowListener >*)0) )
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





void OMRCListenerMultiplexerHelper::impl_unadviseFromPeer(  const   Reference< XWindow >&   xPeer   ,
                                                            const   Type&                   aType   )
{
    
    if( aType == ::getCppuType((const Reference< XWindowListener >*)0) )
        xPeer->removeWindowListener( this );
    else if( aType == ::getCppuType((const Reference< XKeyListener >*)0) )
        xPeer->removeKeyListener( this );
    else if( aType == ::getCppuType((const Reference< XFocusListener >*)0) )
        xPeer->removeFocusListener( this );
    else if( aType == ::getCppuType((const Reference< XMouseListener >*)0) )
        xPeer->removeMouseListener( this );
    else if( aType == ::getCppuType((const Reference< XMouseMotionListener >*)0) )
        xPeer->removeMouseMotionListener( this );
    else if( aType == ::getCppuType((const Reference< XPaintListener >*)0) )
        xPeer->removePaintListener( this );
    else if( aType == ::getCppuType((const Reference< XTopWindowListener >*)0) )
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

} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
