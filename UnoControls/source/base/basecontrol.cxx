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

#include <basecontrol.hxx>
#include <multiplexer.hxx>

#include <com/sun/star/awt/XDevice.hpp>
#include <com/sun/star/awt/WindowAttribute.hpp>
#include <com/sun/star/awt/PosSize.hpp>
#include <com/sun/star/awt/Toolkit.hpp>
#include <cppuhelper/supportsservice.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <cppuhelper/typeprovider.hxx>

//  namespaces

using namespace ::cppu;
using namespace ::osl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::awt;

namespace unocontrols {

constexpr sal_Int32 DEFAULT_X = 0;
constexpr sal_Int32 DEFAULT_Y = 0;
constexpr sal_Int32 DEFAULT_WIDTH = 100;
constexpr sal_Int32 DEFAULT_HEIGHT = 100;
constexpr bool DEFAULT_VISIBLE = false;
constexpr bool DEFAULT_INDESIGNMODE = false;
constexpr bool DEFAULT_ENABLE = true;

//  construct/destruct

BaseControl::BaseControl( const Reference< XComponentContext >& rxContext )
    : WeakComponentImplHelper   ( m_aMutex              )
    , m_xComponentContext       ( rxContext              )
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

//  XServiceInfo

OUString SAL_CALL BaseControl::getImplementationName()
{
    return OUString();
}

//  XServiceInfo

sal_Bool SAL_CALL BaseControl::supportsService( const OUString& sServiceName )
{
    return cppu::supportsService(this, sServiceName);
}

//  XServiceInfo

Sequence< OUString > SAL_CALL BaseControl::getSupportedServiceNames()
{
    return Sequence< OUString >();
}

//  XComponent

void SAL_CALL BaseControl::dispose()
{
    // Ready for multithreading
    MutexGuard aGuard( m_aMutex );

    if ( m_xMultiplexer.is() )
    {
        // to all other paint, focus, etc.
        m_xMultiplexer->disposeAndClear();
        m_xMultiplexer.clear();
    }

    // set the service manager to disposed
    WeakComponentImplHelper::dispose();

    // release context and peer
    m_xContext.clear();
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
    }

    // release view
    if ( m_xGraphicsView.is() )
    {
        m_xGraphicsView.clear();
    }
}

//  XComponent

void SAL_CALL BaseControl::addEventListener( const Reference< XEventListener >& xListener )
{
    // Ready for multithreading
    MutexGuard aGuard( m_aMutex );
    WeakComponentImplHelper::addEventListener( xListener );
}

//  XComponent

void SAL_CALL BaseControl::removeEventListener( const Reference< XEventListener >& xListener )
{
    // Ready for multithreading
    MutexGuard aGuard( m_aMutex );
    WeakComponentImplHelper::removeEventListener( xListener );
}

//  XControl

void SAL_CALL BaseControl::createPeer(  const   Reference< XToolkit >&      xToolkit    ,
                                        const   Reference< XWindowPeer >&   xParentPeer )
{
    // Ready for multithreading
    MutexGuard aGuard( m_aMutex );

    if ( m_xPeer.is() )
        return;

    // use method "BaseControl::getWindowDescriptor()" to change window attributes!
    WindowDescriptor aDescriptor = impl_getWindowDescriptor( xParentPeer );

    if ( m_bVisible )
    {
        aDescriptor.WindowAttributes |= WindowAttribute::SHOW;
    }

    // very slow under remote conditions!
    // create the window on the server
    Reference< XToolkit > xLocalToolkit = xToolkit;
    if ( !xLocalToolkit.is() )
    {
        // but first create well known toolkit, if it not exist
        xLocalToolkit.set( Toolkit::create(m_xComponentContext), UNO_QUERY_THROW );
    }
    m_xPeer         = xLocalToolkit->createWindow( aDescriptor );
    m_xPeerWindow.set( m_xPeer, UNO_QUERY );

    if ( !m_xPeerWindow.is() )
        return;

    if ( m_xMultiplexer.is() )
    {
        m_xMultiplexer->setPeer( m_xPeerWindow );
    }

    // create new reference to xgraphics for painting on a peer
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

//  XControl

void SAL_CALL BaseControl::setContext( const Reference< XInterface >& xContext )
{
    // Ready for multithreading
    MutexGuard aGuard( m_aMutex );
    m_xContext = xContext;
}

//  XControl

void SAL_CALL BaseControl::setDesignMode( sal_Bool bOn )
{
    // Ready for multithreading
    MutexGuard aGuard( m_aMutex );
    m_bInDesignMode = bOn;
}

//  XControl

Reference< XInterface > SAL_CALL BaseControl::getContext()
{
    // Ready for multithreading
    MutexGuard aGuard( m_aMutex );
    return m_xContext;
}

//  XControl

Reference< XWindowPeer > SAL_CALL BaseControl::getPeer()
{
    // Ready for multithreading
    MutexGuard aGuard( m_aMutex );
    return m_xPeer;
}

//  XControl

Reference< XView > SAL_CALL BaseControl::getView()
{
    // Ready for multithreading
    MutexGuard aGuard( m_aMutex );
    return this;
}

//  XControl

sal_Bool SAL_CALL BaseControl::isDesignMode()
{
    // Ready for multithreading
    MutexGuard aGuard( m_aMutex );
    return m_bInDesignMode;
}

//  XControl

sal_Bool SAL_CALL BaseControl::isTransparent()
{
    return false;
}

//  XWindow

void SAL_CALL BaseControl::setPosSize(  sal_Int32   nX      ,
                                        sal_Int32   nY      ,
                                        sal_Int32   nWidth  ,
                                        sal_Int32   nHeight ,
                                        sal_Int16   nFlags  )
{
    // - change size and position of window and save the values

    // Ready for multithreading
    MutexGuard aGuard( m_aMutex );

    bool bChanged = false;

    if ( nFlags & PosSize::X )
    {
        bChanged |= m_nX != nX;
        m_nX = nX;
    }

    if ( nFlags & PosSize::Y )
    {
        bChanged |= m_nY != nY;
        m_nY = nY;
    }

    if ( nFlags & PosSize::WIDTH )
    {
        bChanged |= m_nWidth != nWidth;
        m_nWidth  = nWidth;
    }

    if ( nFlags & PosSize::HEIGHT )
    {
        bChanged |= m_nHeight != nHeight;
        m_nHeight = nHeight;
    }

    if ( bChanged && m_xPeerWindow.is() )
    {
        m_xPeerWindow->setPosSize( m_nX, m_nY, m_nWidth, m_nHeight, nFlags );
    }
}

//  XWindow

void SAL_CALL BaseControl::setVisible( sal_Bool bVisible )
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

void SAL_CALL BaseControl::setEnable( sal_Bool bEnable )
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

void SAL_CALL BaseControl::setFocus()
{
    // Ready for multithreading
    MutexGuard aGuard( m_aMutex );

    if ( m_xPeerWindow.is() )
    {
        m_xPeerWindow->setFocus();
    }
}

//  XWindow

Rectangle SAL_CALL BaseControl::getPosSize()
{
    // Ready for multithreading
    MutexGuard aGuard( m_aMutex );
    return Rectangle( m_nX, m_nY , m_nWidth, m_nHeight );
}

//  XWindow

void SAL_CALL BaseControl::addWindowListener( const Reference< XWindowListener >& xListener )
{
    impl_getMultiplexer()->advise(xListener);
}

//  XWindow

void SAL_CALL BaseControl::addFocusListener( const Reference< XFocusListener >& xListener )
{
    impl_getMultiplexer()->advise(xListener);
}

//  XWindow

void SAL_CALL BaseControl::addKeyListener( const Reference< XKeyListener >& xListener )
{
    impl_getMultiplexer()->advise(xListener);
}

//  XWindow

void SAL_CALL BaseControl::addMouseListener( const Reference< XMouseListener >& xListener )
{
    impl_getMultiplexer()->advise(xListener);
}

//  XWindow

void SAL_CALL BaseControl::addMouseMotionListener( const Reference< XMouseMotionListener >& xListener )
{
    impl_getMultiplexer()->advise(xListener);
}

//  XWindow

void SAL_CALL BaseControl::addPaintListener( const Reference< XPaintListener >& xListener )
{
    impl_getMultiplexer()->advise(xListener);
}

//  XWindow

void SAL_CALL BaseControl::removeWindowListener( const Reference< XWindowListener >& xListener )
{
    impl_getMultiplexer()->unadvise(xListener);
}

//  XWindow

void SAL_CALL BaseControl::removeFocusListener( const Reference< XFocusListener >& xListener )
{
    impl_getMultiplexer()->unadvise(xListener);
}

//  XWindow

void SAL_CALL BaseControl::removeKeyListener( const Reference< XKeyListener >& xListener )
{
    impl_getMultiplexer()->unadvise(xListener);
}

//  XWindow

void SAL_CALL BaseControl::removeMouseListener( const Reference< XMouseListener >& xListener )
{
    impl_getMultiplexer()->unadvise(xListener);
}

//  XWindow

void  SAL_CALL BaseControl::removeMouseMotionListener( const Reference< XMouseMotionListener >& xListener )
{
    impl_getMultiplexer()->unadvise(xListener);
}

//  XWindow

void SAL_CALL BaseControl::removePaintListener( const Reference< XPaintListener >& xListener )
{
    impl_getMultiplexer()->unadvise(xListener);
}

//  XView

void SAL_CALL BaseControl::draw(    sal_Int32   nX  ,
                                    sal_Int32   nY  )
{
    // Ready for multithreading
    MutexGuard aGuard( m_aMutex );

    // - paint to a view
    // - use the method "paint()"
    // - see also "windowPaint()"
    impl_paint( nX, nY, m_xGraphicsView );
}

//  XView

sal_Bool SAL_CALL BaseControl::setGraphics( const Reference< XGraphics >& xDevice )
{
    // - set the graphics for a view
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
                                    float   /*fZoomY*/  )
{
    // Not implemented yet
}

//  XView

Reference< XGraphics > SAL_CALL BaseControl::getGraphics()
{
    // Ready for multithreading
    MutexGuard aGuard( m_aMutex );
    return m_xGraphicsView;
}

//  XView

Size SAL_CALL BaseControl::getSize()
{
    // Ready for multithreading
    MutexGuard aGuard( m_aMutex );
    return Size( m_nWidth, m_nHeight );
}

//  XEventListener

void SAL_CALL BaseControl::disposing( const EventObject& /*aSource*/ )
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

void SAL_CALL BaseControl::windowPaint( const PaintEvent& /*aEvent*/ )
{
    // Ready for multithreading
    MutexGuard aGuard( m_aMutex );

    // - repaint the peer
    // - use the method "paint ()" for painting on a peer and a print device !!!
    // - see also "draw ()"
    impl_paint( 0, 0, m_xGraphicsPeer );
}

//  XWindowListener

void SAL_CALL BaseControl::windowResized( const WindowEvent& aEvent )
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

void SAL_CALL BaseControl::windowMoved( const WindowEvent& aEvent )
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

void SAL_CALL BaseControl::windowShown( const EventObject& /*aEvent*/ )
{
}

//  XWindowListener

void SAL_CALL BaseControl::windowHidden( const EventObject& /*aEvent*/ )
{
}

//  protected method

WindowDescriptor BaseControl::impl_getWindowDescriptor( const Reference< XWindowPeer >& xParentPeer )
{
    // - used from "createPeer()" to set the values of a css::awt::WindowDescriptor !!!
    // - if you will change the descriptor-values, you must override this virtual function
    // - the caller must release the memory for this dynamical descriptor !!!

    WindowDescriptor aDescriptor;

    aDescriptor.Type               = WindowClass_SIMPLE;
    aDescriptor.WindowServiceName  = "window";
    aDescriptor.ParentIndex        = -1;
    aDescriptor.Parent             = xParentPeer;
    aDescriptor.Bounds             = getPosSize ();
    aDescriptor.WindowAttributes   = 0;

    return aDescriptor;
}

//  protected method

void BaseControl::impl_paint(           sal_Int32               /*nX*/          ,
                                        sal_Int32               /*nY*/          ,
                                const   Reference< XGraphics >& /*xGraphics*/   )
{
    // - one paint method for peer AND view !!!
    //   (see also => "windowPaint()" and "draw()")
    // - not used in this implementation, but it's not necessary to make it pure virtual !!!
}

//  protected method

void BaseControl::impl_recalcLayout( const WindowEvent& /*aEvent*/ )
{
    // We need as virtual function to support automatically resizing of derived controls!
    // But we make it not pure virtual because it's not necessary for all derived classes!
}

//  private method

OMRCListenerMultiplexerHelper* BaseControl::impl_getMultiplexer()
{
    if ( !m_xMultiplexer.is() )
    {
        m_xMultiplexer = new OMRCListenerMultiplexerHelper( static_cast<XWindow*>(this), m_xPeerWindow );
    }

    return m_xMultiplexer.get();
}

} // namespace unocontrols

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
