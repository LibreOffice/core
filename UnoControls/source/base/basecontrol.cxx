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
#include <cppuhelper/typeprovider.hxx>

//____________________________________________________________________________________________________________
//  namespaces
//____________________________________________________________________________________________________________

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
#define DEFAULT_VISIBLE                     sal_False
#define DEFAULT_INDESIGNMODE                sal_False
#define DEFAULT_ENABLE                      sal_True

//____________________________________________________________________________________________________________
//  construct/destruct
//____________________________________________________________________________________________________________

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

//____________________________________________________________________________________________________________
//  XInterface
//____________________________________________________________________________________________________________

Any SAL_CALL BaseControl::queryInterface( const Type& rType ) throw( RuntimeException )
{
    Any aReturn ;
    if ( m_xDelegator.is() == sal_True )
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

    return aReturn ;
}

//____________________________________________________________________________________________________________
//  XInterface
//____________________________________________________________________________________________________________

void SAL_CALL BaseControl::acquire() throw()
{
    // Attention:
    //  Don't use mutex or guard in this method!!! Is a method of XInterface.

    // Forward to baseclass
    OComponentHelper::acquire();
}

//____________________________________________________________________________________________________________
//  XInterface
//____________________________________________________________________________________________________________

void SAL_CALL BaseControl::release() throw()
{
    // Attention:
    //  Don't use mutex or guard in this method!!! Is a method of XInterface.

    // Forward to baseclass
    OComponentHelper::release();
}

//____________________________________________________________________________________________________________
//  XTypeProvider
//____________________________________________________________________________________________________________

Sequence< Type > SAL_CALL BaseControl::getTypes() throw( RuntimeException )
{
    // Optimize this method !
    // We initialize a static variable only one time. And we don't must use a mutex at every call!
    // For the first call; pTypeCollection is NULL - for the second call pTypeCollection is different from NULL!
    static OTypeCollection* pTypeCollection = NULL ;

    if ( pTypeCollection == NULL )
    {
        // Ready for multithreading; get global mutex for first call of this method only! see before
        MutexGuard aGuard( Mutex::getGlobalMutex() );

        // Control these pointer again ... it can be, that another instance will be faster then these!
        if ( pTypeCollection == NULL )
        {
            // Create a static typecollection ...
            static OTypeCollection aTypeCollection( ::getCppuType(( const Reference< XPaintListener >*)NULL ) ,
                                                    ::getCppuType(( const Reference< XWindowListener>*)NULL ) ,
                                                    ::getCppuType(( const Reference< XView          >*)NULL ) ,
                                                    ::getCppuType(( const Reference< XWindow        >*)NULL ) ,
                                                    ::getCppuType(( const Reference< XServiceInfo   >*)NULL ) ,
                                                    ::getCppuType(( const Reference< XControl       >*)NULL ) ,
                                                    OComponentHelper::getTypes()
                                                  );

            // ... and set his address to static pointer!
            pTypeCollection = &aTypeCollection ;
        }
    }

    return pTypeCollection->getTypes();
}

//____________________________________________________________________________________________________________
//  XTypeProvider
//____________________________________________________________________________________________________________

Sequence< sal_Int8 > SAL_CALL BaseControl::getImplementationId() throw( RuntimeException )
{
    // Create one Id for all instances of this class.
    // Use ethernet address to do this! (sal_True)

    // Optimize this method
    // We initialize a static variable only one time. And we don't must use a mutex at every call!
    // For the first call; pID is NULL - for the second call pID is different from NULL!
    static OImplementationId* pID = NULL ;

    if ( pID == NULL )
    {
        // Ready for multithreading; get global mutex for first call of this method only! see before
        MutexGuard aGuard( Mutex::getGlobalMutex() );

        // Control these pointer again ... it can be, that another instance will be faster then these!
        if ( pID == NULL )
        {
            // Create a new static ID ...
            static OImplementationId aID( sal_False );
            // ... and set his address to static pointer!
            pID = &aID ;
        }
    }

    return pID->getImplementationId();
}

//____________________________________________________________________________________________________________
//  XAggregation
//____________________________________________________________________________________________________________

void SAL_CALL BaseControl::setDelegator( const Reference< XInterface >& xDel ) throw( RuntimeException )
{
    // Ready for multithreading
    MutexGuard aGuard( m_aMutex );
    m_xDelegator = xDel;
}

//____________________________________________________________________________________________________________
//  XAggregation
//____________________________________________________________________________________________________________

Any SAL_CALL BaseControl::queryAggregation( const Type& aType ) throw( RuntimeException )
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
    if ( aReturn.hasValue() == sal_True )
    {
        // ... return this information.
        return aReturn ;
    }
    else
    {
        // Else; ... ask baseclass for interfaces!
        return OComponentHelper::queryAggregation( aType );
    }
}

//____________________________________________________________________________________________________________
//  XServiceInfo
//____________________________________________________________________________________________________________

OUString SAL_CALL BaseControl::getImplementationName() throw( RuntimeException )
{
    return impl_getStaticImplementationName();
}

//____________________________________________________________________________________________________________
//  XServiceInfo
//____________________________________________________________________________________________________________

sal_Bool SAL_CALL BaseControl::supportsService( const OUString& sServiceName ) throw( RuntimeException )
{
    return cppu::supportsService(this, sServiceName);
}

//____________________________________________________________________________________________________________
//  XServiceInfo
//____________________________________________________________________________________________________________

Sequence< OUString > SAL_CALL BaseControl::getSupportedServiceNames() throw( RuntimeException )
{
    return impl_getStaticSupportedServiceNames();
}

//____________________________________________________________________________________________________________
//  XComponent
//____________________________________________________________________________________________________________

void SAL_CALL BaseControl::dispose() throw( RuntimeException )
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
    m_xContext = Reference< XInterface >();
    impl_releasePeer();

    // release view
    if ( m_xGraphicsView.is() == sal_True )
    {
        m_xGraphicsView = Reference< XGraphics >();
    }
}

//____________________________________________________________________________________________________________
//  XComponent
//____________________________________________________________________________________________________________

void SAL_CALL BaseControl::addEventListener( const Reference< XEventListener >& xListener ) throw( RuntimeException )
{
    // Ready for multithreading
    MutexGuard aGuard( m_aMutex );
    OComponentHelper::addEventListener( xListener );
}

//____________________________________________________________________________________________________________
//  XComponent
//____________________________________________________________________________________________________________

void SAL_CALL BaseControl::removeEventListener( const Reference< XEventListener >& xListener ) throw( RuntimeException )
{
    // Ready for multithreading
    MutexGuard aGuard( m_aMutex );
    OComponentHelper::removeEventListener( xListener );
}

//____________________________________________________________________________________________________________
//  XControl
//____________________________________________________________________________________________________________

void SAL_CALL BaseControl::createPeer(  const   Reference< XToolkit >&      xToolkit    ,
                                        const   Reference< XWindowPeer >&   xParentPeer ) throw( RuntimeException )
{
    // Ready for multithreading
    MutexGuard aGuard( m_aMutex );

    if ( m_xPeer.is() == sal_False )
    {
        // use method "BaseControl::getWindowDescriptor()" fot change window attributes !!!
        WindowDescriptor* pDescriptor = impl_getWindowDescriptor( xParentPeer );

        if ( m_bVisible == sal_True )
        {
            pDescriptor->WindowAttributes |= WindowAttribute::SHOW ;
        }

        // very slow under remote conditions!
        // create the window on the server
        Reference< XToolkit > xLocalToolkit = xToolkit ;
        if ( xLocalToolkit.is() == sal_False )
        {
            // but first create well known toolkit, if it not exist
            xLocalToolkit = Reference< XToolkit > ( Toolkit::create(m_xComponentContext), UNO_QUERY_THROW );
        }
        m_xPeer         = xLocalToolkit->createWindow( *pDescriptor );
        m_xPeerWindow   = Reference< XWindow >( m_xPeer, UNO_QUERY );

        // don't forget to release the memory!
        delete pDescriptor ;

        if ( m_xPeerWindow.is() == sal_True )
        {
            if ( m_pMultiplexer != NULL )
            {
                m_pMultiplexer->setPeer( m_xPeerWindow );
            }

            // create new referenz to xgraphics for painting on a peer
            // and add a paint listener
            Reference< XDevice > xDevice( m_xPeerWindow, UNO_QUERY );

            if ( xDevice.is() == sal_True )
            {
                m_xGraphicsPeer = xDevice->createGraphics();
            }

            if ( m_xGraphicsPeer.is() == sal_True )
            {
                addPaintListener( this );
                addWindowListener( this );
            }

            // PosSize_POSSIZE defined in <stardiv/uno/awt/window.hxx>
            m_xPeerWindow->setPosSize(  m_nX, m_nY, m_nWidth, m_nHeight, PosSize::POSSIZE   );
            m_xPeerWindow->setEnable(   m_bEnable                                           );
            m_xPeerWindow->setVisible(  m_bVisible && !m_bInDesignMode                      );
        }
    }
}

//____________________________________________________________________________________________________________
//  XControl
//____________________________________________________________________________________________________________

void SAL_CALL BaseControl::setContext( const Reference< XInterface >& xContext ) throw( RuntimeException )
{
    // Ready for multithreading
    MutexGuard aGuard( m_aMutex );
    m_xContext = xContext ;
}

//____________________________________________________________________________________________________________
//  XControl
//____________________________________________________________________________________________________________

void SAL_CALL BaseControl::setDesignMode( sal_Bool bOn ) throw( RuntimeException )
{
    // Ready for multithreading
    MutexGuard aGuard( m_aMutex );
    m_bInDesignMode = bOn ;
}

//____________________________________________________________________________________________________________
//  XControl
//____________________________________________________________________________________________________________

Reference< XInterface > SAL_CALL BaseControl::getContext() throw( RuntimeException )
{
    // Ready for multithreading
    MutexGuard aGuard( m_aMutex );
    return m_xContext ;
}

//____________________________________________________________________________________________________________
//  XControl
//____________________________________________________________________________________________________________

Reference< XWindowPeer > SAL_CALL BaseControl::getPeer() throw( RuntimeException )
{
    // Ready for multithreading
    MutexGuard aGuard( m_aMutex );
    return m_xPeer ;
}

//____________________________________________________________________________________________________________
//  XControl
//____________________________________________________________________________________________________________

Reference< XView > SAL_CALL BaseControl::getView() throw( RuntimeException )
{
    // Ready for multithreading
    MutexGuard aGuard( m_aMutex );
    return Reference< XView >( (OWeakObject*)this, UNO_QUERY );
}

//____________________________________________________________________________________________________________
//  XControl
//____________________________________________________________________________________________________________

sal_Bool SAL_CALL BaseControl::isDesignMode() throw( RuntimeException )
{
    // Ready for multithreading
    MutexGuard aGuard( m_aMutex );
    return m_bInDesignMode ;
}

//____________________________________________________________________________________________________________
//  XControl
//____________________________________________________________________________________________________________

sal_Bool SAL_CALL BaseControl::isTransparent() throw( RuntimeException )
{
    return sal_False ;
}

//____________________________________________________________________________________________________________
//  XWindow
//____________________________________________________________________________________________________________

void SAL_CALL BaseControl::setPosSize(  sal_Int32   nX      ,
                                        sal_Int32   nY      ,
                                        sal_Int32   nWidth  ,
                                        sal_Int32   nHeight ,
                                        sal_Int16   nFlags  ) throw( RuntimeException )
{
    // - change size and position of window and save the values
    // - "nFlags" declared in <stardiv/uno/awt/window.hxx> ("#define PosSize_X .....")

    // Ready for multithreading
    MutexGuard aGuard( m_aMutex );

    sal_Bool bChanged = sal_False ;

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

//____________________________________________________________________________________________________________
//  XWindow
//____________________________________________________________________________________________________________

void SAL_CALL BaseControl::setVisible( sal_Bool bVisible ) throw( RuntimeException )
{
    // Ready for multithreading
    MutexGuard aGuard( m_aMutex );

    // Set new state of flag
    m_bVisible = bVisible ;

    if ( m_xPeerWindow.is() == sal_True )
    {
        // Set it also on peerwindow
        m_xPeerWindow->setVisible( m_bVisible );
    }
}

//____________________________________________________________________________________________________________
//  XWindow
//____________________________________________________________________________________________________________

void SAL_CALL BaseControl::setEnable( sal_Bool bEnable ) throw( RuntimeException )
{
    // Ready for multithreading
    MutexGuard aGuard( m_aMutex );

    // Set new state of flag
    m_bEnable = bEnable ;

    if ( m_xPeerWindow.is() == sal_True )
    {
        // Set it also on peerwindow
        m_xPeerWindow->setEnable( m_bEnable );
    }
}

//____________________________________________________________________________________________________________
//  XWindow
//____________________________________________________________________________________________________________

void SAL_CALL BaseControl::setFocus() throw( RuntimeException )
{
    // Ready for multithreading
    MutexGuard aGuard( m_aMutex );

    if ( m_xPeerWindow.is() == sal_True )
    {
        m_xPeerWindow->setFocus();
    }
}

//____________________________________________________________________________________________________________
//  XWindow
//____________________________________________________________________________________________________________

Rectangle SAL_CALL BaseControl::getPosSize() throw( RuntimeException )
{
    // Ready for multithreading
    MutexGuard aGuard( m_aMutex );
    return Rectangle( m_nX, m_nY , m_nWidth, m_nHeight );
}

//____________________________________________________________________________________________________________
//  XWindow
//____________________________________________________________________________________________________________

void SAL_CALL BaseControl::addWindowListener( const Reference< XWindowListener >& xListener ) throw( RuntimeException )
{
    impl_getMultiplexer()->advise( ::getCppuType(( const Reference< XWindowListener >*)0), xListener );
}

//____________________________________________________________________________________________________________
//  XWindow
//____________________________________________________________________________________________________________

void SAL_CALL BaseControl::addFocusListener( const Reference< XFocusListener >& xListener ) throw( RuntimeException )
{
    impl_getMultiplexer()->advise( ::getCppuType(( const Reference< XFocusListener >*)0), xListener );
}

//____________________________________________________________________________________________________________
//  XWindow
//____________________________________________________________________________________________________________

void SAL_CALL BaseControl::addKeyListener( const Reference< XKeyListener >& xListener ) throw( RuntimeException )
{
    impl_getMultiplexer()->advise( ::getCppuType(( const Reference< XKeyListener >*)0), xListener );
}

//____________________________________________________________________________________________________________
//  XWindow
//____________________________________________________________________________________________________________

void SAL_CALL BaseControl::addMouseListener( const Reference< XMouseListener >& xListener ) throw( RuntimeException )
{
    impl_getMultiplexer()->advise( ::getCppuType(( const Reference< XMouseListener >*)0), xListener );
}

//____________________________________________________________________________________________________________
//  XWindow
//____________________________________________________________________________________________________________

void SAL_CALL BaseControl::addMouseMotionListener( const Reference< XMouseMotionListener >& xListener ) throw( RuntimeException )
{
    impl_getMultiplexer()->advise( ::getCppuType(( const Reference< XMouseMotionListener >*)0), xListener );
}

//____________________________________________________________________________________________________________
//  XWindow
//____________________________________________________________________________________________________________

void SAL_CALL BaseControl::addPaintListener( const Reference< XPaintListener >& xListener ) throw( RuntimeException )
{
    impl_getMultiplexer()->advise( ::getCppuType(( const Reference< XPaintListener >*)0), xListener );
}

//____________________________________________________________________________________________________________
//  XWindow
//____________________________________________________________________________________________________________

void SAL_CALL BaseControl::removeWindowListener( const Reference< XWindowListener >& xListener ) throw( RuntimeException )
{
    impl_getMultiplexer()->unadvise( ::getCppuType(( const Reference< XWindowListener >*)0), xListener );
}

//____________________________________________________________________________________________________________
//  XWindow
//____________________________________________________________________________________________________________

void SAL_CALL BaseControl::removeFocusListener( const Reference< XFocusListener >& xListener ) throw( RuntimeException )
{
    impl_getMultiplexer()->unadvise( ::getCppuType(( const Reference< XFocusListener >*)0), xListener );
}

//____________________________________________________________________________________________________________
//  XWindow
//____________________________________________________________________________________________________________

void SAL_CALL BaseControl::removeKeyListener( const Reference< XKeyListener >& xListener ) throw( RuntimeException )
{
    impl_getMultiplexer()->unadvise( ::getCppuType(( const Reference< XKeyListener >*)0), xListener );
}

//____________________________________________________________________________________________________________
//  XWindow
//____________________________________________________________________________________________________________

void SAL_CALL BaseControl::removeMouseListener( const Reference< XMouseListener >& xListener ) throw( RuntimeException )
{
    impl_getMultiplexer()->unadvise( ::getCppuType(( const Reference< XMouseListener >*)0), xListener );
}

//____________________________________________________________________________________________________________
//  XWindow
//____________________________________________________________________________________________________________

void  SAL_CALL BaseControl::removeMouseMotionListener( const Reference< XMouseMotionListener >& xListener ) throw( RuntimeException )
{
    impl_getMultiplexer()->unadvise( ::getCppuType(( const Reference< XMouseMotionListener >*)0), xListener );
}

//____________________________________________________________________________________________________________
//  XWindow
//____________________________________________________________________________________________________________

void SAL_CALL BaseControl::removePaintListener( const Reference< XPaintListener >& xListener ) throw( RuntimeException )
{
    impl_getMultiplexer()->unadvise( ::getCppuType(( const Reference< XPaintListener >*)0), xListener );
}

//____________________________________________________________________________________________________________
//  XView
//____________________________________________________________________________________________________________

void SAL_CALL BaseControl::draw(    sal_Int32   nX  ,
                                    sal_Int32   nY  ) throw( RuntimeException )
{
    // Ready for multithreading
    MutexGuard aGuard( m_aMutex );

    // - paint to an view
    // - use the method "paint()"
    // - see also "windowPaint()"
    impl_paint( nX, nY, m_xGraphicsView );
}

//____________________________________________________________________________________________________________
//  XView
//____________________________________________________________________________________________________________

sal_Bool SAL_CALL BaseControl::setGraphics( const Reference< XGraphics >& xDevice ) throw( RuntimeException )
{
    // - set the graphics for an view
    // - in this class exist 2 graphics-member ... one for peer[_xGraphicsPeer] and one for view[_xGraphicsView]
    // - they are used by "windowPaint() and draw()", forwarded to "paint ()"
    sal_Bool bReturn = sal_False ;
    if ( xDevice.is() == sal_True )
    {
        // Ready for multithreading
        MutexGuard aGuard( m_aMutex );

        m_xGraphicsView = xDevice   ;
        bReturn         = sal_True  ;
    }

    return bReturn ;
}

//____________________________________________________________________________________________________________
//  XView
//____________________________________________________________________________________________________________

void SAL_CALL BaseControl::setZoom( float   /*fZoomX*/  ,
                                    float   /*fZoomY*/  ) throw( RuntimeException )
{
    // Not implemented yet
}

//____________________________________________________________________________________________________________
//  XView
//____________________________________________________________________________________________________________

Reference< XGraphics > SAL_CALL BaseControl::getGraphics() throw( RuntimeException )
{
    // Ready for multithreading
    MutexGuard aGuard( m_aMutex );
    return m_xGraphicsView ;
}

//____________________________________________________________________________________________________________
//  XView
//____________________________________________________________________________________________________________

Size SAL_CALL BaseControl::getSize() throw( RuntimeException )
{
    // Ready for multithreading
    MutexGuard aGuard( m_aMutex );
    return Size( m_nWidth, m_nHeight );
}

//____________________________________________________________________________________________________________
//  XEventListener
//____________________________________________________________________________________________________________

void SAL_CALL BaseControl::disposing( const EventObject& /*aSource*/ ) throw( RuntimeException )
{
    // Ready for multithreading
    MutexGuard aGuard( m_aMutex );

    // - release ALL references
    // - it must be !!!
    if ( m_xGraphicsPeer.is() == sal_True )
    {
        removePaintListener( this );
        removeWindowListener( this );
        m_xGraphicsPeer = Reference< XGraphics >();
    }

    if ( m_xGraphicsView.is() == sal_True )
    {
        m_xGraphicsView = Reference< XGraphics >();
    }
}

//____________________________________________________________________________________________________________
//  XPaintListener
//____________________________________________________________________________________________________________

void SAL_CALL BaseControl::windowPaint( const PaintEvent& /*aEvent*/ ) throw( RuntimeException )
{
    // Ready for multithreading
    MutexGuard aGuard( m_aMutex );

    // - repaint the peer
    // - use the method "paint ()" for painting on a peer and a print device !!!
    // - see also "draw ()"
    impl_paint( 0, 0, m_xGraphicsPeer );
}

//____________________________________________________________________________________________________________
//  XWindowListener
//____________________________________________________________________________________________________________

void SAL_CALL BaseControl::windowResized( const WindowEvent& aEvent ) throw( RuntimeException )
{
    // Ready for multithreading
    MutexGuard aGuard( m_aMutex );

    m_nWidth    =   aEvent.Width    ;
    m_nHeight   =   aEvent.Height   ;
    WindowEvent aMappedEvent = aEvent;
    aMappedEvent.X = 0;
    aMappedEvent.Y = 0;
    impl_recalcLayout( aMappedEvent );
}

//____________________________________________________________________________________________________________
//  XWindowListener
//____________________________________________________________________________________________________________

void SAL_CALL BaseControl::windowMoved( const WindowEvent& aEvent ) throw( RuntimeException )
{
    // Ready for multithreading
    MutexGuard aGuard( m_aMutex );

    m_nWidth    =   aEvent.Width    ;
    m_nHeight   =   aEvent.Height   ;
    WindowEvent aMappedEvent = aEvent;
    aMappedEvent.X = 0;
    aMappedEvent.Y = 0;
    impl_recalcLayout( aMappedEvent );
}

//____________________________________________________________________________________________________________
//  XWindowListener
//____________________________________________________________________________________________________________

void SAL_CALL BaseControl::windowShown( const EventObject& /*aEvent*/ ) throw( RuntimeException )
{
}

//____________________________________________________________________________________________________________
//  XWindowListener
//____________________________________________________________________________________________________________

void SAL_CALL BaseControl::windowHidden( const EventObject& /*aEvent*/ ) throw( RuntimeException )
{
}

//____________________________________________________________________________________________________________
//  impl but public method to register service in DLL
//  (In this BASE-implementation not implemented! Overwrite it in derived classes.)
//____________________________________________________________________________________________________________

const Sequence< OUString > BaseControl::impl_getStaticSupportedServiceNames()
{
    return Sequence< OUString >();
}

//____________________________________________________________________________________________________________
//  impl but public method to register service in DLL
//  (In this BASE-implementation not implemented! Overwrite it in derived classes.)
//____________________________________________________________________________________________________________

const OUString BaseControl::impl_getStaticImplementationName()
{
    return OUString();
}

//____________________________________________________________________________________________________________
//  protected method
//____________________________________________________________________________________________________________

const Reference< XComponentContext > BaseControl::impl_getComponentContext()
{
    return m_xComponentContext ;
}

//____________________________________________________________________________________________________________
//  protected method
//____________________________________________________________________________________________________________

const Reference< XWindow > BaseControl::impl_getPeerWindow()
{
    return m_xPeerWindow ;
}

//____________________________________________________________________________________________________________
//  protected method
//____________________________________________________________________________________________________________

const Reference< XGraphics > BaseControl::impl_getGraphicsPeer()
{
    return m_xGraphicsPeer ;
}

//____________________________________________________________________________________________________________
//  protected method
//____________________________________________________________________________________________________________

const sal_Int32& BaseControl::impl_getWidth()
{
    return m_nWidth ;
}

//____________________________________________________________________________________________________________
//  protected method
//____________________________________________________________________________________________________________

const sal_Int32& BaseControl::impl_getHeight()
{
    return m_nHeight ;
}

//____________________________________________________________________________________________________________
//  protected method
//____________________________________________________________________________________________________________

WindowDescriptor* BaseControl::impl_getWindowDescriptor( const Reference< XWindowPeer >& xParentPeer )
{
    // - used from "createPeer()" to set the values of an ::com::sun::star::awt::WindowDescriptor !!!
    // - if you will change the descriptor-values, you must override this virtuell function
    // - the caller must release the memory for this dynamical descriptor !!!

    WindowDescriptor* pDescriptor = new WindowDescriptor ;

    pDescriptor->Type               = WindowClass_SIMPLE                    ;
    pDescriptor->WindowServiceName  = "window"                              ;
    pDescriptor->ParentIndex        = -1                                    ;
    pDescriptor->Parent             = xParentPeer                           ;
    pDescriptor->Bounds             = getPosSize ()                         ;
    pDescriptor->WindowAttributes   = 0                                     ;

    return pDescriptor ;
}

//____________________________________________________________________________________________________________
//  protected method
//____________________________________________________________________________________________________________

void BaseControl::impl_paint(           sal_Int32               /*nX*/          ,
                                        sal_Int32               /*nY*/          ,
                                const   Reference< XGraphics >& /*xGraphics*/   )
{
    // - one paint method for peer AND view !!!
    //   (see also => "windowPaint()" and "draw()")
    // - not used in this implementation, but its not necessary to make it pure virtual !!!
}

//____________________________________________________________________________________________________________
//  protected method
//____________________________________________________________________________________________________________

void BaseControl::impl_recalcLayout( const WindowEvent& /*aEvent*/ )
{
    // We need as virtual function to support automaticly resizing of derived controls!
    // But we make it not pure virtual because it's not neccessary for all derived classes!
}

//____________________________________________________________________________________________________________
//  protected method
//____________________________________________________________________________________________________________

Reference< XInterface > BaseControl::impl_getDelegator()
{
    return m_xDelegator ;
}

//____________________________________________________________________________________________________________
//  private method
//____________________________________________________________________________________________________________

void BaseControl::impl_releasePeer()
{
    if ( m_xPeer.is() == sal_True )
    {
        if ( m_xGraphicsPeer.is() == sal_True )
        {
            removePaintListener( this );
            removeWindowListener( this );
            m_xGraphicsPeer = Reference< XGraphics >();
        }

        m_xPeer->dispose();
        m_xPeerWindow   = Reference< XWindow >();
        m_xPeer         = Reference< XWindowPeer >();

        if ( m_pMultiplexer != NULL )
        {
            // take changes on multiplexer
            m_pMultiplexer->setPeer( Reference< XWindow >() );
        }
    }
}

//____________________________________________________________________________________________________________
//  private method
//____________________________________________________________________________________________________________

OMRCListenerMultiplexerHelper* BaseControl::impl_getMultiplexer()
{
    if ( m_pMultiplexer == NULL )
    {
        m_pMultiplexer = new OMRCListenerMultiplexerHelper( (XWindow*)this, m_xPeerWindow );
        m_xMultiplexer = Reference< XInterface >( (OWeakObject*)m_pMultiplexer, UNO_QUERY );
    }

    return m_pMultiplexer ;
}

} // namespace unocontrols

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
