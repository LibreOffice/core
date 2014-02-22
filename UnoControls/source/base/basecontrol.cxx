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

#include "basecontrol.hxx"

#include <com/sun/star/awt/XDevice.hpp>
#include <com/sun/star/awt/XDisplayBitmap.hpp>
#include <com/sun/star/awt/DeviceInfo.hpp>
#include <com/sun/star/awt/WindowAttribute.hpp>
#include <com/sun/star/awt/PosSize.hpp>
#include <com/sun/star/awt/Toolkit.hpp>
#include <comphelper/processfactory.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <cppuhelper/typeprovider.hxx>





using namespace ::cppu                  ;
using namespace ::osl                   ;
using namespace ::rtl                   ;
using namespace ::com::sun::star::uno   ;
using namespace ::com::sun::star::lang  ;
using namespace ::com::sun::star::awt   ;

namespace unocontrols{

#define DEFAULT_PMULTIPLEXER                NULL
#define DEFAULT_X                           0
#define DEFAULT_Y                           0
#define DEFAULT_WIDTH                       100
#define DEFAULT_HEIGHT                      100
#define DEFAULT_VISIBLE                     false
#define DEFAULT_INDESIGNMODE                false
#define DEFAULT_ENABLE                      true





BaseControl::BaseControl( const Reference< XComponentContext >& rxContext )
    : IMPL_MutexContainer       (                       )
    , OComponentHelper          ( m_aMutex              )
    , m_xComponentContext       ( rxContext              )
    , m_pMultiplexer            ( DEFAULT_PMULTIPLEXER  )
    , m_nX                      ( DEFAULT_X             )
    , m_nY                      ( DEFAULT_Y             )
    , m_nWidth                  ( DEFAULT_WIDTH         )
    , m_nHeight                 ( DEFAULT_HEIGHT        )
    , m_bVisible                ( DEFAULT_VISIBLE       )
    , m_bInDesignMode           ( DEFAULT_INDESIGNMODE  )
    , m_bEnable                 ( DEFAULT_ENABLE        )
{
}

BaseControl::~BaseControl()
{
}





Any SAL_CALL BaseControl::queryInterface( const Type& rType ) throw( RuntimeException )
{
    Any aReturn ;
    if ( m_xDelegator.is() )
    {
        
        
        aReturn = m_xDelegator->queryInterface( rType );
    }
    else
    {
        
        aReturn = queryAggregation( rType );
    }

    return aReturn ;
}





void SAL_CALL BaseControl::acquire() throw()
{
    
    

    
    OComponentHelper::acquire();
}





void SAL_CALL BaseControl::release() throw()
{
    
    

    
    OComponentHelper::release();
}





Sequence< Type > SAL_CALL BaseControl::getTypes() throw( RuntimeException )
{
    
    
    
    static OTypeCollection* pTypeCollection = NULL ;

    if ( pTypeCollection == NULL )
    {
        
        MutexGuard aGuard( Mutex::getGlobalMutex() );

        
        if ( pTypeCollection == NULL )
        {
            
            static OTypeCollection aTypeCollection( ::getCppuType(( const Reference< XPaintListener >*)NULL ) ,
                                                    ::getCppuType(( const Reference< XWindowListener>*)NULL ) ,
                                                    ::getCppuType(( const Reference< XView          >*)NULL ) ,
                                                    ::getCppuType(( const Reference< XWindow        >*)NULL ) ,
                                                    ::getCppuType(( const Reference< XServiceInfo   >*)NULL ) ,
                                                    ::getCppuType(( const Reference< XControl       >*)NULL ) ,
                                                    OComponentHelper::getTypes()
                                                  );

            
            pTypeCollection = &aTypeCollection ;
        }
    }

    return pTypeCollection->getTypes();
}





Sequence< sal_Int8 > SAL_CALL BaseControl::getImplementationId() throw( RuntimeException )
{
    
    

    
    
    
    static OImplementationId* pID = NULL ;

    if ( pID == NULL )
    {
        
        MutexGuard aGuard( Mutex::getGlobalMutex() );

        
        if ( pID == NULL )
        {
            
            static OImplementationId aID( false );
            
            pID = &aID ;
        }
    }

    return pID->getImplementationId();
}





void SAL_CALL BaseControl::setDelegator( const Reference< XInterface >& xDel ) throw( RuntimeException )
{
    
    MutexGuard aGuard( m_aMutex );
    m_xDelegator = xDel;
}





Any SAL_CALL BaseControl::queryAggregation( const Type& aType ) throw( RuntimeException )
{
    
    
    Any aReturn ( ::cppu::queryInterface(   aType                                   ,
                                               static_cast< XPaintListener*> ( this )   ,
                                               static_cast< XWindowListener*> ( this )  ,
                                               static_cast< XView*          > ( this )  ,
                                               static_cast< XWindow*        > ( this )  ,
                                               static_cast< XServiceInfo*   > ( this )  ,
                                               static_cast< XControl*       > ( this )
                                        )
                );

    
    if ( aReturn.hasValue() )
    {
        
        return aReturn ;
    }
    else
    {
        
        return OComponentHelper::queryAggregation( aType );
    }
}





OUString SAL_CALL BaseControl::getImplementationName() throw( RuntimeException )
{
    return impl_getStaticImplementationName();
}





sal_Bool SAL_CALL BaseControl::supportsService( const OUString& sServiceName ) throw( RuntimeException )
{
    return cppu::supportsService(this, sServiceName);
}





Sequence< OUString > SAL_CALL BaseControl::getSupportedServiceNames() throw( RuntimeException )
{
    return impl_getStaticSupportedServiceNames();
}





void SAL_CALL BaseControl::dispose() throw( RuntimeException )
{
    
    MutexGuard aGuard( m_aMutex );

    if ( m_pMultiplexer != NULL )
    {
        
        m_pMultiplexer->disposeAndClear();
    }

    
    OComponentHelper::dispose();

    
    m_xContext.clear();
    impl_releasePeer();

    
    if ( m_xGraphicsView.is() )
    {
        m_xGraphicsView.clear();
    }
}





void SAL_CALL BaseControl::addEventListener( const Reference< XEventListener >& xListener ) throw( RuntimeException )
{
    
    MutexGuard aGuard( m_aMutex );
    OComponentHelper::addEventListener( xListener );
}





void SAL_CALL BaseControl::removeEventListener( const Reference< XEventListener >& xListener ) throw( RuntimeException )
{
    
    MutexGuard aGuard( m_aMutex );
    OComponentHelper::removeEventListener( xListener );
}





void SAL_CALL BaseControl::createPeer(  const   Reference< XToolkit >&      xToolkit    ,
                                        const   Reference< XWindowPeer >&   xParentPeer ) throw( RuntimeException )
{
    
    MutexGuard aGuard( m_aMutex );

    if ( !m_xPeer.is() )
    {
        
        WindowDescriptor* pDescriptor = impl_getWindowDescriptor( xParentPeer );

        if ( m_bVisible )
        {
            pDescriptor->WindowAttributes |= WindowAttribute::SHOW ;
        }

        
        
        Reference< XToolkit > xLocalToolkit = xToolkit ;
        if ( !xLocalToolkit.is() )
        {
            
            xLocalToolkit = Reference< XToolkit > ( Toolkit::create(m_xComponentContext), UNO_QUERY_THROW );
        }
        m_xPeer         = xLocalToolkit->createWindow( *pDescriptor );
        m_xPeerWindow   = Reference< XWindow >( m_xPeer, UNO_QUERY );

        
        delete pDescriptor ;

        if ( m_xPeerWindow.is() )
        {
            if ( m_pMultiplexer != NULL )
            {
                m_pMultiplexer->setPeer( m_xPeerWindow );
            }

            
            
            Reference< XDevice > xDevice( m_xPeerWindow, UNO_QUERY );

            if ( xDevice.is() )
            {
                m_xGraphicsPeer = xDevice->createGraphics();
            }

            if ( m_xGraphicsPeer.is() )
            {
                addPaintListener( this );
                addWindowListener( this );
            }

            m_xPeerWindow->setPosSize(  m_nX, m_nY, m_nWidth, m_nHeight, PosSize::POSSIZE   );
            m_xPeerWindow->setEnable(   m_bEnable                                           );
            m_xPeerWindow->setVisible(  m_bVisible && !m_bInDesignMode                      );
        }
    }
}





void SAL_CALL BaseControl::setContext( const Reference< XInterface >& xContext ) throw( RuntimeException )
{
    
    MutexGuard aGuard( m_aMutex );
    m_xContext = xContext ;
}





void SAL_CALL BaseControl::setDesignMode( sal_Bool bOn ) throw( RuntimeException )
{
    
    MutexGuard aGuard( m_aMutex );
    m_bInDesignMode = bOn ;
}





Reference< XInterface > SAL_CALL BaseControl::getContext() throw( RuntimeException )
{
    
    MutexGuard aGuard( m_aMutex );
    return m_xContext ;
}





Reference< XWindowPeer > SAL_CALL BaseControl::getPeer() throw( RuntimeException )
{
    
    MutexGuard aGuard( m_aMutex );
    return m_xPeer ;
}





Reference< XView > SAL_CALL BaseControl::getView() throw( RuntimeException )
{
    
    MutexGuard aGuard( m_aMutex );
    return Reference< XView >( (OWeakObject*)this, UNO_QUERY );
}





sal_Bool SAL_CALL BaseControl::isDesignMode() throw( RuntimeException )
{
    
    MutexGuard aGuard( m_aMutex );
    return m_bInDesignMode ;
}





sal_Bool SAL_CALL BaseControl::isTransparent() throw( RuntimeException )
{
    return sal_False ;
}





void SAL_CALL BaseControl::setPosSize(  sal_Int32   nX      ,
                                        sal_Int32   nY      ,
                                        sal_Int32   nWidth  ,
                                        sal_Int32   nHeight ,
                                        sal_Int16   nFlags  ) throw( RuntimeException )
{
    

    
    MutexGuard aGuard( m_aMutex );

    bool bChanged = false ;

    if ( nFlags & PosSize::X )
    {
        bChanged |= m_nX != nX, m_nX = nX ;
    }

    if ( nFlags & PosSize::Y )
    {
        bChanged |= m_nY != nY, m_nY = nY ;
    }

    if ( nFlags & PosSize::WIDTH )
    {
        bChanged |= m_nWidth != nWidth, m_nWidth  = nWidth ;
    }

    if ( nFlags & PosSize::HEIGHT )
    {
        bChanged |= m_nHeight != nHeight, m_nHeight = nHeight  ;
    }

    if ( bChanged && m_xPeerWindow.is() )
    {
        m_xPeerWindow->setPosSize( m_nX, m_nY, m_nWidth, m_nHeight, nFlags );
    }
}





void SAL_CALL BaseControl::setVisible( sal_Bool bVisible ) throw( RuntimeException )
{
    
    MutexGuard aGuard( m_aMutex );

    
    m_bVisible = bVisible ;

    if ( m_xPeerWindow.is() )
    {
        
        m_xPeerWindow->setVisible( m_bVisible );
    }
}





void SAL_CALL BaseControl::setEnable( sal_Bool bEnable ) throw( RuntimeException )
{
    
    MutexGuard aGuard( m_aMutex );

    
    m_bEnable = bEnable ;

    if ( m_xPeerWindow.is() )
    {
        
        m_xPeerWindow->setEnable( m_bEnable );
    }
}





void SAL_CALL BaseControl::setFocus() throw( RuntimeException )
{
    
    MutexGuard aGuard( m_aMutex );

    if ( m_xPeerWindow.is() )
    {
        m_xPeerWindow->setFocus();
    }
}





Rectangle SAL_CALL BaseControl::getPosSize() throw( RuntimeException )
{
    
    MutexGuard aGuard( m_aMutex );
    return Rectangle( m_nX, m_nY , m_nWidth, m_nHeight );
}





void SAL_CALL BaseControl::addWindowListener( const Reference< XWindowListener >& xListener ) throw( RuntimeException )
{
    impl_getMultiplexer()->advise( ::getCppuType(( const Reference< XWindowListener >*)0), xListener );
}





void SAL_CALL BaseControl::addFocusListener( const Reference< XFocusListener >& xListener ) throw( RuntimeException )
{
    impl_getMultiplexer()->advise( ::getCppuType(( const Reference< XFocusListener >*)0), xListener );
}





void SAL_CALL BaseControl::addKeyListener( const Reference< XKeyListener >& xListener ) throw( RuntimeException )
{
    impl_getMultiplexer()->advise( ::getCppuType(( const Reference< XKeyListener >*)0), xListener );
}





void SAL_CALL BaseControl::addMouseListener( const Reference< XMouseListener >& xListener ) throw( RuntimeException )
{
    impl_getMultiplexer()->advise( ::getCppuType(( const Reference< XMouseListener >*)0), xListener );
}





void SAL_CALL BaseControl::addMouseMotionListener( const Reference< XMouseMotionListener >& xListener ) throw( RuntimeException )
{
    impl_getMultiplexer()->advise( ::getCppuType(( const Reference< XMouseMotionListener >*)0), xListener );
}





void SAL_CALL BaseControl::addPaintListener( const Reference< XPaintListener >& xListener ) throw( RuntimeException )
{
    impl_getMultiplexer()->advise( ::getCppuType(( const Reference< XPaintListener >*)0), xListener );
}





void SAL_CALL BaseControl::removeWindowListener( const Reference< XWindowListener >& xListener ) throw( RuntimeException )
{
    impl_getMultiplexer()->unadvise( ::getCppuType(( const Reference< XWindowListener >*)0), xListener );
}





void SAL_CALL BaseControl::removeFocusListener( const Reference< XFocusListener >& xListener ) throw( RuntimeException )
{
    impl_getMultiplexer()->unadvise( ::getCppuType(( const Reference< XFocusListener >*)0), xListener );
}





void SAL_CALL BaseControl::removeKeyListener( const Reference< XKeyListener >& xListener ) throw( RuntimeException )
{
    impl_getMultiplexer()->unadvise( ::getCppuType(( const Reference< XKeyListener >*)0), xListener );
}





void SAL_CALL BaseControl::removeMouseListener( const Reference< XMouseListener >& xListener ) throw( RuntimeException )
{
    impl_getMultiplexer()->unadvise( ::getCppuType(( const Reference< XMouseListener >*)0), xListener );
}





void  SAL_CALL BaseControl::removeMouseMotionListener( const Reference< XMouseMotionListener >& xListener ) throw( RuntimeException )
{
    impl_getMultiplexer()->unadvise( ::getCppuType(( const Reference< XMouseMotionListener >*)0), xListener );
}





void SAL_CALL BaseControl::removePaintListener( const Reference< XPaintListener >& xListener ) throw( RuntimeException )
{
    impl_getMultiplexer()->unadvise( ::getCppuType(( const Reference< XPaintListener >*)0), xListener );
}





void SAL_CALL BaseControl::draw(    sal_Int32   nX  ,
                                    sal_Int32   nY  ) throw( RuntimeException )
{
    
    MutexGuard aGuard( m_aMutex );

    
    
    
    impl_paint( nX, nY, m_xGraphicsView );
}





sal_Bool SAL_CALL BaseControl::setGraphics( const Reference< XGraphics >& xDevice ) throw( RuntimeException )
{
    
    
    
    bool bReturn = false ;
    if ( xDevice.is() )
    {
        
        MutexGuard aGuard( m_aMutex );

        m_xGraphicsView = xDevice   ;
        bReturn         = true  ;
    }

    return bReturn ;
}





void SAL_CALL BaseControl::setZoom( float   /*fZoomX*/  ,
                                    float   /*fZoomY*/  ) throw( RuntimeException )
{
    
}





Reference< XGraphics > SAL_CALL BaseControl::getGraphics() throw( RuntimeException )
{
    
    MutexGuard aGuard( m_aMutex );
    return m_xGraphicsView ;
}





Size SAL_CALL BaseControl::getSize() throw( RuntimeException )
{
    
    MutexGuard aGuard( m_aMutex );
    return Size( m_nWidth, m_nHeight );
}





void SAL_CALL BaseControl::disposing( const EventObject& /*aSource*/ ) throw( RuntimeException )
{
    
    MutexGuard aGuard( m_aMutex );

    
    
    if ( m_xGraphicsPeer.is() )
    {
        removePaintListener( this );
        removeWindowListener( this );
        m_xGraphicsPeer.clear();
    }

    if ( m_xGraphicsView.is() )
    {
        m_xGraphicsView.clear();
    }
}





void SAL_CALL BaseControl::windowPaint( const PaintEvent& /*aEvent*/ ) throw( RuntimeException )
{
    
    MutexGuard aGuard( m_aMutex );

    
    
    
    impl_paint( 0, 0, m_xGraphicsPeer );
}





void SAL_CALL BaseControl::windowResized( const WindowEvent& aEvent ) throw( RuntimeException )
{
    
    MutexGuard aGuard( m_aMutex );

    m_nWidth    =   aEvent.Width    ;
    m_nHeight   =   aEvent.Height   ;
    WindowEvent aMappedEvent = aEvent;
    aMappedEvent.X = 0;
    aMappedEvent.Y = 0;
    impl_recalcLayout( aMappedEvent );
}





void SAL_CALL BaseControl::windowMoved( const WindowEvent& aEvent ) throw( RuntimeException )
{
    
    MutexGuard aGuard( m_aMutex );

    m_nWidth    =   aEvent.Width    ;
    m_nHeight   =   aEvent.Height   ;
    WindowEvent aMappedEvent = aEvent;
    aMappedEvent.X = 0;
    aMappedEvent.Y = 0;
    impl_recalcLayout( aMappedEvent );
}





void SAL_CALL BaseControl::windowShown( const EventObject& /*aEvent*/ ) throw( RuntimeException )
{
}





void SAL_CALL BaseControl::windowHidden( const EventObject& /*aEvent*/ ) throw( RuntimeException )
{
}






const Sequence< OUString > BaseControl::impl_getStaticSupportedServiceNames()
{
    return Sequence< OUString >();
}






const OUString BaseControl::impl_getStaticImplementationName()
{
    return OUString();
}





const Reference< XComponentContext > BaseControl::impl_getComponentContext()
{
    return m_xComponentContext ;
}





const Reference< XWindow > BaseControl::impl_getPeerWindow()
{
    return m_xPeerWindow ;
}





const Reference< XGraphics > BaseControl::impl_getGraphicsPeer()
{
    return m_xGraphicsPeer ;
}





const sal_Int32& BaseControl::impl_getWidth()
{
    return m_nWidth ;
}





const sal_Int32& BaseControl::impl_getHeight()
{
    return m_nHeight ;
}





WindowDescriptor* BaseControl::impl_getWindowDescriptor( const Reference< XWindowPeer >& xParentPeer )
{
    
    
    

    WindowDescriptor* pDescriptor = new WindowDescriptor ;

    pDescriptor->Type               = WindowClass_SIMPLE                    ;
    pDescriptor->WindowServiceName  = "window"                              ;
    pDescriptor->ParentIndex        = -1                                    ;
    pDescriptor->Parent             = xParentPeer                           ;
    pDescriptor->Bounds             = getPosSize ()                         ;
    pDescriptor->WindowAttributes   = 0                                     ;

    return pDescriptor ;
}





void BaseControl::impl_paint(           sal_Int32               /*nX*/          ,
                                        sal_Int32               /*nY*/          ,
                                const   Reference< XGraphics >& /*xGraphics*/   )
{
    
    
    
}





void BaseControl::impl_recalcLayout( const WindowEvent& /*aEvent*/ )
{
    
    
}





Reference< XInterface > BaseControl::impl_getDelegator()
{
    return m_xDelegator ;
}





void BaseControl::impl_releasePeer()
{
    if ( m_xPeer.is() )
    {
        if ( m_xGraphicsPeer.is() )
        {
            removePaintListener( this );
            removeWindowListener( this );
            m_xGraphicsPeer.clear();
        }

        m_xPeer->dispose();
        m_xPeerWindow.clear();
        m_xPeer.clear();

        if ( m_pMultiplexer != NULL )
        {
            
            m_pMultiplexer->setPeer( Reference< XWindow >() );
        }
    }
}





OMRCListenerMultiplexerHelper* BaseControl::impl_getMultiplexer()
{
    if ( m_pMultiplexer == NULL )
    {
        m_pMultiplexer = new OMRCListenerMultiplexerHelper( (XWindow*)this, m_xPeerWindow );
        m_xMultiplexer = Reference< XInterface >( (OWeakObject*)m_pMultiplexer, UNO_QUERY );
    }

    return m_pMultiplexer ;
}

} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
