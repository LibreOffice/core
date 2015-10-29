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

#include "basecontrol.hxx"

#include <com/sun/star/awt/XDevice.hpp>
#include <com/sun/star/awt/XDisplayBitmap.hpp>
#include <com/sun/star/awt/DeviceInfo.hpp>
#include <com/sun/star/awt/WindowAttribute.hpp>
#include <com/sun/star/awt/PosSize.hpp>
#include <com/sun/star/awt/Toolkit.hpp>
#include <comphelper/processfactory.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <cppuhelper/typeprovider.hxx>

//  namespaces

using namespace ::cppu;
using namespace ::osl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::awt;

namespace unocontrols{

#define DEFAULT_PMULTIPLEXER                NULL
#define DEFAULT_X                           0
#define DEFAULT_Y                           0
#define DEFAULT_WIDTH                       100
#define DEFAULT_HEIGHT                      100
#define DEFAULT_VISIBLE                     false
#define DEFAULT_INDESIGNMODE                false
#define DEFAULT_ENABLE                      true

//  construct/destruct

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

//  XInterface

Any SAL_CALL BaseControl::queryInterface( const Type& rType ) throw( RuntimeException, std::exception )
{
    Any aReturn;
    if ( m_xDelegator.is() )
    {
        // If an delegator exist, forward question to his queryInterface.
        // Delegator will ask his own queryAggregation!
        aReturn = m_xDelegator->queryInterface( rType );
    }
    else
    {
        // If an delegator unknown, forward question to own queryAggregation.
        aReturn = queryAggregation( rType );
    }

    return aReturn;
}

//  XInterface

void SAL_CALL BaseControl::acquire() throw()
{
    // Attention:
    //  Don't use mutex or guard in this method!!! Is a method of XInterface.

    // Forward to baseclass
    OComponentHelper::acquire();
}

//  XInterface

void SAL_CALL BaseControl::release() throw()
{
    // Attention:
    //  Don't use mutex or guard in this method!!! Is a method of XInterface.

    // Forward to baseclass
    OComponentHelper::release();
}

//  XTypeProvider

Sequence< Type > SAL_CALL BaseControl::getTypes() throw( RuntimeException, std::exception )
{
    // Optimize this method !
    // We initialize a static variable only one time. And we don't must use a mutex at every call!
    // For the first call; pTypeCollection is NULL - for the second call pTypeCollection is different from NULL!
    static OTypeCollection* pTypeCollection = NULL;

    if ( pTypeCollection == NULL )
    {
        // Ready for multithreading; get global mutex for first call of this method only! see before
        MutexGuard aGuard( Mutex::getGlobalMutex() );

        // Control these pointer again ... it can be, that another instance will be faster then these!
        if ( pTypeCollection == NULL )
        {
            // Create a static typecollection ...
            static OTypeCollection aTypeCollection( cppu::UnoType<XPaintListener>::get(),
                                                    cppu::UnoType<XWindowListener>::get(),
                                                    cppu::UnoType<XView>::get(),
                                                    cppu::UnoType<XWindow>::get(),
                                                    cppu::UnoType<XServiceInfo>::get(),
                                                    cppu::UnoType<XControl>::get(),
                                                    OComponentHelper::getTypes()
                                                  );

            // ... and set his address to static pointer!
            pTypeCollection = &aTypeCollection;
        }
    }

    return pTypeCollection->getTypes();
}

//  XTypeProvider

Sequence< sal_Int8 > SAL_CALL BaseControl::getImplementationId() throw( RuntimeException, std::exception )
{
    return css::uno::Sequence<sal_Int8>();
}

//  XAggregation

void SAL_CALL BaseControl::setDelegator( const Reference< XInterface >& xDel ) throw( RuntimeException, std::exception )
{
    // Ready for multithreading
    MutexGuard aGuard( m_aMutex );
    m_xDelegator = xDel;
}

//  XAggregation

Any SAL_CALL BaseControl::queryAggregation( const Type& aType ) throw( RuntimeException, std::exception )
{
    // Ask for my own supported interfaces ...
    // Attention: XTypeProvider and XInterface are supported by OComponentHelper!
    Any aReturn ( ::cppu::queryInterface(   aType                                   ,
                                               static_cast< XPaintListener*> ( this )   ,
                                               static_cast< XWindowListener*> ( this )  ,
                                               static_cast< XView*          > ( this )  ,
                                               static_cast< XWindow*        > ( this )  ,
                                               static_cast< XServiceInfo*   > ( this )  ,
                                               static_cast< XControl*       > ( this )
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
        return OComponentHelper::queryAggregation( aType );
    }
}

//  XServiceInfo

OUString SAL_CALL BaseControl::getImplementationName() throw( RuntimeException, std::exception )
{
    return impl_getStaticImplementationName();
}

//  XServiceInfo

sal_Bool SAL_CALL BaseControl::supportsService( const OUString& sServiceName ) throw( RuntimeException, std::exception )
{
    return cppu::supportsService(this, sServiceName);
}

//  XServiceInfo

Sequence< OUString > SAL_CALL BaseControl::getSupportedServiceNames() throw( RuntimeException, std::exception )
{
    return impl_getStaticSupportedServiceNames();
}

//  XComponent

void SAL_CALL BaseControl::dispose() throw( RuntimeException, std::exception )
{
    // Ready for multithreading
    MutexGuard aGuard( m_aMutex );

    if ( m_pMultiplexer != NULL )
    {
        // to all other paint, focus, etc.
        m_pMultiplexer->disposeAndClear();
    }

    // set the service manager to disposed
    OComponentHelper::dispose();

    // release context and peer
    m_xContext.clear();
    impl_releasePeer();

    // release view
    if ( m_xGraphicsView.is() )
    {
        m_xGraphicsView.clear();
    }
}

//  XComponent

void SAL_CALL BaseControl::addEventListener( const Reference< XEventListener >& xListener ) throw( RuntimeException, std::exception )
{
    // Ready for multithreading
    MutexGuard aGuard( m_aMutex );
    OComponentHelper::addEventListener( xListener );
}

//  XComponent

void SAL_CALL BaseControl::removeEventListener( const Reference< XEventListener >& xListener ) throw( RuntimeException, std::exception )
{
    // Ready for multithreading
    MutexGuard aGuard( m_aMutex );
    OComponentHelper::removeEventListener( xListener );
}

//  XControl

void SAL_CALL BaseControl::createPeer(  const   Reference< XToolkit >&      xToolkit    ,
                                        const   Reference< XWindowPeer >&   xParentPeer ) throw( RuntimeException, std::exception )
{
    // Ready for multithreading
    MutexGuard aGuard( m_aMutex );

    if ( !m_xPeer.is() )
    {
        // use method "BaseControl::getWindowDescriptor()" fot change window attributes !!!
        WindowDescriptor* pDescriptor = impl_getWindowDescriptor( xParentPeer );

        if ( m_bVisible )
        {
            pDescriptor->WindowAttributes |= WindowAttribute::SHOW;
        }

        // very slow under remote conditions!
        // create the window on the server
        Reference< XToolkit > xLocalToolkit = xToolkit;
        if ( !xLocalToolkit.is() )
        {
            // but first create well known toolkit, if it not exist
            xLocalToolkit.set( Toolkit::create(m_xComponentContext), UNO_QUERY_THROW );
        }
        m_xPeer         = xLocalToolkit->createWindow( *pDescriptor );
        m_xPeerWindow.set( m_xPeer, UNO_QUERY );

        // don't forget to release the memory!
        delete pDescriptor;

        if ( m_xPeerWindow.is() )
        {
            if ( m_pMultiplexer != NULL )
            {
                m_pMultiplexer->setPeer( m_xPeerWindow );
            }

            // create new referenz to xgraphics for painting on a peer
            // and add a paint listener
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

//  XControl

void SAL_CALL BaseControl::setContext( const Reference< XInterface >& xContext ) throw( RuntimeException, std::exception )
{
    // Ready for multithreading
    MutexGuard aGuard( m_aMutex );
    m_xContext = xContext;
}

//  XControl

void SAL_CALL BaseControl::setDesignMode( sal_Bool bOn ) throw( RuntimeException, std::exception )
{
    // Ready for multithreading
    MutexGuard aGuard( m_aMutex );
    m_bInDesignMode = bOn;
}

//  XControl

Reference< XInterface > SAL_CALL BaseControl::getContext() throw( RuntimeException, std::exception )
{
    // Ready for multithreading
    MutexGuard aGuard( m_aMutex );
    return m_xContext;
}

//  XControl

Reference< XWindowPeer > SAL_CALL BaseControl::getPeer() throw( RuntimeException, std::exception )
{
    // Ready for multithreading
    MutexGuard aGuard( m_aMutex );
    return m_xPeer;
}

//  XControl

Reference< XView > SAL_CALL BaseControl::getView() throw( RuntimeException, std::exception )
{
    // Ready for multithreading
    MutexGuard aGuard( m_aMutex );
    return Reference< XView >( static_cast<OWeakObject*>(this), UNO_QUERY );
}

//  XControl

sal_Bool SAL_CALL BaseControl::isDesignMode() throw( RuntimeException, std::exception )
{
    // Ready for multithreading
    MutexGuard aGuard( m_aMutex );
    return m_bInDesignMode;
}

//  XControl

sal_Bool SAL_CALL BaseControl::isTransparent() throw( RuntimeException, std::exception )
{
    return false;
}

//  XWindow

void SAL_CALL BaseControl::setPosSize(  sal_Int32   nX      ,
                                        sal_Int32   nY      ,
                                        sal_Int32   nWidth  ,
                                        sal_Int32   nHeight ,
                                        sal_Int16   nFlags  ) throw( RuntimeException, std::exception )
{
    // - change size and position of window and save the values

    // Ready for multithreading
    MutexGuard aGuard( m_aMutex );

    bool bChanged = false;

    if ( nFlags & PosSize::X )
    {
        bChanged |= m_nX != nX, m_nX = nX;
    }

    if ( nFlags & PosSize::Y )
    {
        bChanged |= m_nY != nY, m_nY = nY;
    }

    if ( nFlags & PosSize::WIDTH )
    {
        bChanged |= m_nWidth != nWidth, m_nWidth  = nWidth;
    }

    if ( nFlags & PosSize::HEIGHT )
    {
        bChanged |= m_nHeight != nHeight, m_nHeight = nHeight;
    }

    if ( bChanged && m_xPeerWindow.is() )
    {
        m_xPeerWindow->setPosSize( m_nX, m_nY, m_nWidth, m_nHeight, nFlags );
    }
}

//  XWindow

void SAL_CALL BaseControl::setVisible( sal_Bool bVisible ) throw( RuntimeException, std::exception )
{
    // Ready for multithreading
    MutexGuard aGuard( m_aMutex );

    // Set new state of flag
    m_bVisible = bVisible;

    if ( m_xPeerWindow.is() )
    {
        // Set it also on peerwindow
        m_xPeerWindow->setVisible( m_bVisible );
    }
}

//  XWindow

void SAL_CALL BaseControl::setEnable( sal_Bool bEnable ) throw( RuntimeException, std::exception )
{
    // Ready for multithreading
    MutexGuard aGuard( m_aMutex );

    // Set new state of flag
    m_bEnable = bEnable;

    if ( m_xPeerWindow.is() )
    {
        // Set it also on peerwindow
        m_xPeerWindow->setEnable( m_bEnable );
    }
}

//  XWindow

void SAL_CALL BaseControl::setFocus() throw( RuntimeException, std::exception )
{
    // Ready for multithreading
    MutexGuard aGuard( m_aMutex );

    if ( m_xPeerWindow.is() )
    {
        m_xPeerWindow->setFocus();
    }
}

//  XWindow

Rectangle SAL_CALL BaseControl::getPosSize() throw( RuntimeException, std::exception )
{
    // Ready for multithreading
    MutexGuard aGuard( m_aMutex );
    return Rectangle( m_nX, m_nY , m_nWidth, m_nHeight );
}

//  XWindow

void SAL_CALL BaseControl::addWindowListener( const Reference< XWindowListener >& xListener ) throw( RuntimeException, std::exception )
{
    impl_getMultiplexer()->advise( cppu::UnoType<XWindowListener>::get(), xListener );
}

//  XWindow

void SAL_CALL BaseControl::addFocusListener( const Reference< XFocusListener >& xListener ) throw( RuntimeException, std::exception )
{
    impl_getMultiplexer()->advise( cppu::UnoType<XFocusListener>::get(), xListener );
}

//  XWindow

void SAL_CALL BaseControl::addKeyListener( const Reference< XKeyListener >& xListener ) throw( RuntimeException, std::exception )
{
    impl_getMultiplexer()->advise( cppu::UnoType<XKeyListener>::get(), xListener );
}

//  XWindow

void SAL_CALL BaseControl::addMouseListener( const Reference< XMouseListener >& xListener ) throw( RuntimeException, std::exception )
{
    impl_getMultiplexer()->advise( cppu::UnoType<XMouseListener>::get(), xListener );
}

//  XWindow

void SAL_CALL BaseControl::addMouseMotionListener( const Reference< XMouseMotionListener >& xListener ) throw( RuntimeException, std::exception )
{
    impl_getMultiplexer()->advise( cppu::UnoType<XMouseMotionListener>::get(), xListener );
}

//  XWindow

void SAL_CALL BaseControl::addPaintListener( const Reference< XPaintListener >& xListener ) throw( RuntimeException, std::exception )
{
    impl_getMultiplexer()->advise( cppu::UnoType<XPaintListener>::get(), xListener );
}

//  XWindow

void SAL_CALL BaseControl::removeWindowListener( const Reference< XWindowListener >& xListener ) throw( RuntimeException, std::exception )
{
    impl_getMultiplexer()->unadvise( cppu::UnoType<XWindowListener>::get(), xListener );
}

//  XWindow

void SAL_CALL BaseControl::removeFocusListener( const Reference< XFocusListener >& xListener ) throw( RuntimeException, std::exception )
{
    impl_getMultiplexer()->unadvise( cppu::UnoType<XFocusListener>::get(), xListener );
}

//  XWindow

void SAL_CALL BaseControl::removeKeyListener( const Reference< XKeyListener >& xListener ) throw( RuntimeException, std::exception )
{
    impl_getMultiplexer()->unadvise( cppu::UnoType<XKeyListener>::get(), xListener );
}

//  XWindow

void SAL_CALL BaseControl::removeMouseListener( const Reference< XMouseListener >& xListener ) throw( RuntimeException, std::exception )
{
    impl_getMultiplexer()->unadvise( cppu::UnoType<XMouseListener>::get(), xListener );
}

//  XWindow

void  SAL_CALL BaseControl::removeMouseMotionListener( const Reference< XMouseMotionListener >& xListener ) throw( RuntimeException, std::exception )
{
    impl_getMultiplexer()->unadvise( cppu::UnoType<XMouseMotionListener>::get(), xListener );
}

//  XWindow

void SAL_CALL BaseControl::removePaintListener( const Reference< XPaintListener >& xListener ) throw( RuntimeException, std::exception )
{
    impl_getMultiplexer()->unadvise( cppu::UnoType<XPaintListener>::get(), xListener );
}

//  XView

void SAL_CALL BaseControl::draw(    sal_Int32   nX  ,
                                    sal_Int32   nY  ) throw( RuntimeException, std::exception )
{
    // Ready for multithreading
    MutexGuard aGuard( m_aMutex );

    // - paint to an view
    // - use the method "paint()"
    // - see also "windowPaint()"
    impl_paint( nX, nY, m_xGraphicsView );
}

//  XView

sal_Bool SAL_CALL BaseControl::setGraphics( const Reference< XGraphics >& xDevice ) throw( RuntimeException, std::exception )
{
    // - set the graphics for an view
    // - in this class exist 2 graphics-member ... one for peer[_xGraphicsPeer] and one for view[_xGraphicsView]
    // - they are used by "windowPaint() and draw()", forwarded to "paint ()"
    bool bReturn = false;
    if ( xDevice.is() )
    {
        // Ready for multithreading
        MutexGuard aGuard( m_aMutex );

        m_xGraphicsView = xDevice;
        bReturn         = true;
    }

    return bReturn;
}

//  XView

void SAL_CALL BaseControl::setZoom( float   /*fZoomX*/  ,
                                    float   /*fZoomY*/  ) throw( RuntimeException, std::exception )
{
    // Not implemented yet
}

//  XView

Reference< XGraphics > SAL_CALL BaseControl::getGraphics() throw( RuntimeException, std::exception )
{
    // Ready for multithreading
    MutexGuard aGuard( m_aMutex );
    return m_xGraphicsView;
}

//  XView

Size SAL_CALL BaseControl::getSize() throw( RuntimeException, std::exception )
{
    // Ready for multithreading
    MutexGuard aGuard( m_aMutex );
    return Size( m_nWidth, m_nHeight );
}

//  XEventListener

void SAL_CALL BaseControl::disposing( const EventObject& /*aSource*/ ) throw( RuntimeException, std::exception )
{
    // Ready for multithreading
    MutexGuard aGuard( m_aMutex );

    // - release ALL references
    // - it must be !!!
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

//  XPaintListener

void SAL_CALL BaseControl::windowPaint( const PaintEvent& /*aEvent*/ ) throw( RuntimeException, std::exception )
{
    // Ready for multithreading
    MutexGuard aGuard( m_aMutex );

    // - repaint the peer
    // - use the method "paint ()" for painting on a peer and a print device !!!
    // - see also "draw ()"
    impl_paint( 0, 0, m_xGraphicsPeer );
}

//  XWindowListener

void SAL_CALL BaseControl::windowResized( const WindowEvent& aEvent ) throw( RuntimeException, std::exception )
{
    // Ready for multithreading
    MutexGuard aGuard( m_aMutex );

    m_nWidth    =   aEvent.Width;
    m_nHeight   =   aEvent.Height;
    WindowEvent aMappedEvent = aEvent;
    aMappedEvent.X = 0;
    aMappedEvent.Y = 0;
    impl_recalcLayout( aMappedEvent );
}

//  XWindowListener

void SAL_CALL BaseControl::windowMoved( const WindowEvent& aEvent ) throw( RuntimeException, std::exception )
{
    // Ready for multithreading
    MutexGuard aGuard( m_aMutex );

    m_nWidth    =   aEvent.Width;
    m_nHeight   =   aEvent.Height;
    WindowEvent aMappedEvent = aEvent;
    aMappedEvent.X = 0;
    aMappedEvent.Y = 0;
    impl_recalcLayout( aMappedEvent );
}

//  XWindowListener

void SAL_CALL BaseControl::windowShown( const EventObject& /*aEvent*/ ) throw( RuntimeException, std::exception )
{
}

//  XWindowListener

void SAL_CALL BaseControl::windowHidden( const EventObject& /*aEvent*/ ) throw( RuntimeException, std::exception )
{
}

//  impl but public method to register service in DLL
//  (In this BASE-implementation not implemented! Overwrite it in derived classes.)

const Sequence< OUString > BaseControl::impl_getStaticSupportedServiceNames()
{
    return Sequence< OUString >();
}

//  impl but public method to register service in DLL
//  (In this BASE-implementation not implemented! Overwrite it in derived classes.)

const OUString BaseControl::impl_getStaticImplementationName()
{
    return OUString();
}

//  protected method

WindowDescriptor* BaseControl::impl_getWindowDescriptor( const Reference< XWindowPeer >& xParentPeer )
{
    // - used from "createPeer()" to set the values of an ::com::sun::star::awt::WindowDescriptor !!!
    // - if you will change the descriptor-values, you must override this virtuell function
    // - the caller must release the memory for this dynamical descriptor !!!

    WindowDescriptor* pDescriptor = new WindowDescriptor;

    pDescriptor->Type               = WindowClass_SIMPLE;
    pDescriptor->WindowServiceName  = "window";
    pDescriptor->ParentIndex        = -1;
    pDescriptor->Parent             = xParentPeer;
    pDescriptor->Bounds             = getPosSize ();
    pDescriptor->WindowAttributes   = 0;

    return pDescriptor;
}

//  protected method

void BaseControl::impl_paint(           sal_Int32               /*nX*/          ,
                                        sal_Int32               /*nY*/          ,
                                const   Reference< XGraphics >& /*xGraphics*/   )
{
    // - one paint method for peer AND view !!!
    //   (see also => "windowPaint()" and "draw()")
    // - not used in this implementation, but its not necessary to make it pure virtual !!!
}

//  protected method

void BaseControl::impl_recalcLayout( const WindowEvent& /*aEvent*/ )
{
    // We need as virtual function to support automatically resizing of derived controls!
    // But we make it not pure virtual because it's not necessary for all derived classes!
}

//  protected method


//  private method

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
            // take changes on multiplexer
            m_pMultiplexer->setPeer( Reference< XWindow >() );
        }
    }
}

//  private method

OMRCListenerMultiplexerHelper* BaseControl::impl_getMultiplexer()
{
    if ( m_pMultiplexer == NULL )
    {
        m_pMultiplexer = new OMRCListenerMultiplexerHelper( static_cast<XWindow*>(this), m_xPeerWindow );
        m_xMultiplexer.set( static_cast<OWeakObject*>(m_pMultiplexer), UNO_QUERY );
    }

    return m_pMultiplexer;
}

} // namespace unocontrols

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
