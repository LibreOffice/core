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

#include <com/sun/star/awt/XAdjustmentListener.hpp>
#include <com/sun/star/awt/XActionListener.hpp>
#include <com/sun/star/awt/XTextListener.hpp>
#include <com/sun/star/awt/XSpinListener.hpp>
#include <com/sun/star/awt/XItemListener.hpp>
#include <com/sun/star/awt/XVclContainerListener.hpp>
#include <com/sun/star/awt/PosSize.hpp>

#include <plugin/plctrl.hxx>
#include <vcl/syschild.hxx>
#include <toolkit/helper/vclunohelper.hxx>

PluginControl_Impl::PluginControl_Impl()
    : _pMultiplexer( NULL )
    , _nX( 0 )
    , _nY( 0 )
    , _nWidth( 100 )
    , _nHeight( 100 )
    , _nFlags( css::awt::PosSize::POSSIZE )
    , _bVisible(false)
    , _bInDesignMode(false)
    , _bEnable(true)
    , _pSysChild(NULL)
{
}

PluginControl_Impl::~PluginControl_Impl()
{
}

MRCListenerMultiplexerHelper* PluginControl_Impl::getMultiplexer()
{
    if( ! _pMultiplexer )
        _pMultiplexer = new MRCListenerMultiplexerHelper( this, _xPeerWindow );
    return _pMultiplexer;
}


void PluginControl_Impl::addEventListener( const Reference< css::lang::XEventListener > & l )
    throw( RuntimeException, std::exception )
{
    _aDisposeListeners.push_back( l );
}

//---- css::lang::XComponent ----------------------------------------------------------------------------------
void PluginControl_Impl::removeEventListener( const Reference< css::lang::XEventListener > & l )
    throw( RuntimeException, std::exception )
{
    _aDisposeListeners.remove( l );
}

//---- css::lang::XComponent ----------------------------------------------------------------------------------
void PluginControl_Impl::dispose()
    throw( RuntimeException, std::exception )
{
    // send disposing events
    css::lang::EventObject aEvt;
    if( getMultiplexer() )
        getMultiplexer()->disposeAndClear();

    // release context
    _xContext.clear();
    releasePeer();
}


//---- css::awt::XWindow -------------------------------------------------------------------------------------
void PluginControl_Impl::setPosSize( sal_Int32 nX_, sal_Int32 nY_, sal_Int32 nWidth_, sal_Int32 nHeight_, sal_Int16 nFlags )
    throw( RuntimeException, std::exception )
{
    _nX         = nX_ >=0 ? nX_ : 0;
    _nY         = nY_ >=0 ? nY_ : 0;
    _nWidth     = nWidth_ >=0 ? nWidth_ : 0;
    _nHeight    = nHeight_ >=0 ? nHeight_ : 0;
    _nFlags     = nFlags;

    if (_xPeerWindow.is())
        _xPeerWindow->setPosSize( _nX, _nY, _nWidth, _nHeight, nFlags );
}

//---- css::awt::XWindow -------------------------------------------------------------------------------------
css::awt::Rectangle PluginControl_Impl::getPosSize()
    throw( RuntimeException, std::exception )
{
    return _xPeerWindow->getPosSize();
}

//---- css::awt::XWindow -------------------------------------------------------------------------------------
void PluginControl_Impl::setVisible( sal_Bool bVisible )
    throw( RuntimeException, std::exception )
{
    _bVisible = bVisible;
    if (_xPeerWindow.is())
        _xPeerWindow->setVisible( _bVisible && !_bInDesignMode );
}

//---- css::awt::XWindow -------------------------------------------------------------------------------------
void PluginControl_Impl::setEnable( sal_Bool bEnable )
    throw( RuntimeException, std::exception )
{
    _bEnable = bEnable;
    if (_xPeerWindow.is())
        _xPeerWindow->setEnable( _bEnable );
}

//---- css::awt::XWindow -------------------------------------------------------------------------------------
void PluginControl_Impl::setFocus() throw( RuntimeException, std::exception )
{
    if (_xPeerWindow.is())
        _xPeerWindow->setFocus();
}



void PluginControl_Impl::releasePeer()
{
    if (_xPeer.is())
    {
        _xParentWindow->removeFocusListener( this );
        _xPeerWindow->dispose();
        _pSysChild      = NULL;
        _xPeerWindow.clear();
        _xPeer.clear();
        getMultiplexer()->setPeer( Reference< css::awt::XWindow > () );
    }
}

//---- css::awt::XControl ------------------------------------------------------------------------------------
void PluginControl_Impl::createPeer( const Reference< css::awt::XToolkit > & /*xToolkit*/, const Reference< css::awt::XWindowPeer >  & xParentPeer )
    throw( RuntimeException, std::exception )
{
    if (_xPeer.is())
    {
        OSL_FAIL( "### Peer is already set!" );
        return;
    }

    _xParentPeer = xParentPeer;
    _xParentWindow.set( xParentPeer, UNO_QUERY );
    DBG_ASSERT( _xParentWindow.is(), "### no parent peer window!" );

    vcl::Window* pImpl = VCLUnoHelper::GetWindow( xParentPeer );
    if (pImpl)
    {
        _pSysChild = VclPtr<SystemChildWindow>::Create( pImpl, WB_CLIPCHILDREN );
        if (pImpl->HasFocus())
            _pSysChild->GrabFocus();

        // get peer
        _xPeer.set( _pSysChild->GetComponentInterface() );
        _xPeerWindow.set( _xPeer, UNO_QUERY );
        // !_BOTH_ MUST BE VALID!
        DBG_ASSERT( (_xPeer.is() && _xPeerWindow.is()), "### no peer!" );

        _xParentWindow->addFocusListener( this );
        _xPeerWindow->setPosSize( _nX, _nY, _nWidth, _nHeight, _nFlags );
        _xPeerWindow->setEnable( _bEnable );
        _xPeerWindow->setVisible( _bVisible && !_bInDesignMode );
    }
    else
    {
        OSL_FAIL( "### cannot get implementation of parent peer!" );
    }

    getMultiplexer()->setPeer( _xPeerWindow );
}

//---- css::awt::XControl ------------------------------------------------------------------------------------
void PluginControl_Impl::setDesignMode( sal_Bool bOn )
    throw( RuntimeException, std::exception )
{
    _bInDesignMode = bOn;
    if (_xPeerWindow.is())
        _xPeerWindow->setVisible( _bVisible && !_bInDesignMode );
}

//---- css::awt::XWindow -------------------------------------------------------------------------------------
void PluginControl_Impl::addPaintListener( const Reference< css::awt::XPaintListener > & l )
    throw( RuntimeException, std::exception )
{
    getMultiplexer()->advise( cppu::UnoType<css::awt::XPaintListener>::get(), l );
}

//---- css::awt::XWindow -------------------------------------------------------------------------------------
void PluginControl_Impl::removePaintListener( const Reference< css::awt::XPaintListener > & l )
    throw( RuntimeException, std::exception )
{
    getMultiplexer()->unadvise( cppu::UnoType<css::awt::XPaintListener>::get(), l );
}

//---- css::awt::XWindow -------------------------------------------------------------------------------------
void PluginControl_Impl::addWindowListener( const Reference< css::awt::XWindowListener > & l )
    throw( RuntimeException, std::exception )
{
    getMultiplexer()->advise( cppu::UnoType<css::awt::XWindowListener>::get(), l );
}

//---- css::awt::XWindow -------------------------------------------------------------------------------------
void PluginControl_Impl::removeWindowListener( const Reference< css::awt::XWindowListener > & l )
    throw( RuntimeException, std::exception )
{
    getMultiplexer()->unadvise( cppu::UnoType<css::awt::XWindowListener>::get(), l );
}

//---- css::awt::XWindow -------------------------------------------------------------------------------------
void PluginControl_Impl::addFocusListener( const Reference< css::awt::XFocusListener > & l )
    throw( RuntimeException, std::exception )
{
    getMultiplexer()->advise( cppu::UnoType<css::awt::XFocusListener>::get(), l );
}

//---- css::awt::XWindow -------------------------------------------------------------------------------------
void PluginControl_Impl::removeFocusListener( const Reference< css::awt::XFocusListener > & l )
    throw( RuntimeException, std::exception )
{
    getMultiplexer()->unadvise( cppu::UnoType<css::awt::XFocusListener>::get(), l );
}

//---- css::awt::XWindow -------------------------------------------------------------------------------------
void PluginControl_Impl::addKeyListener( const Reference< css::awt::XKeyListener > & l )
    throw( RuntimeException, std::exception )
{
    getMultiplexer()->advise( cppu::UnoType<css::awt::XKeyListener>::get(), l );
}

//---- css::awt::XWindow -------------------------------------------------------------------------------------
void PluginControl_Impl::removeKeyListener( const Reference< css::awt::XKeyListener > & l )
    throw( RuntimeException, std::exception )
{
    getMultiplexer()->unadvise( cppu::UnoType<css::awt::XKeyListener>::get(), l );
}

//---- css::awt::XWindow -------------------------------------------------------------------------------------
void PluginControl_Impl::addMouseListener( const Reference< css::awt::XMouseListener > & l )
    throw( RuntimeException, std::exception )
{
    getMultiplexer()->advise( cppu::UnoType<css::awt::XMouseListener>::get(), l );
}

//---- css::awt::XWindow -------------------------------------------------------------------------------------
void PluginControl_Impl::removeMouseListener( const Reference< css::awt::XMouseListener > & l )
    throw( RuntimeException, std::exception )
{
    getMultiplexer()->unadvise( cppu::UnoType<css::awt::XMouseListener>::get(), l );
}

//---- css::awt::XWindow -------------------------------------------------------------------------------------
void PluginControl_Impl::addMouseMotionListener( const Reference< css::awt::XMouseMotionListener > & l )
    throw( RuntimeException, std::exception )
{
    getMultiplexer()->advise( cppu::UnoType<css::awt::XMouseMotionListener>::get(), l );
}

//---- css::awt::XWindow -------------------------------------------------------------------------------------
void PluginControl_Impl::removeMouseMotionListener( const Reference< css::awt::XMouseMotionListener > & l )
    throw( RuntimeException, std::exception )
{
    getMultiplexer()->unadvise( cppu::UnoType<css::awt::XMouseMotionListener>::get(), l );
}


//---- css::awt::XView ---------------------------------------------------------------------------------------
void PluginControl_Impl::draw( sal_Int32 /*x*/, sal_Int32 /*y*/ )
    throw( RuntimeException, std::exception )
{
    // has to be done by further implementation of control
}

//---- css::awt::XView ---------------------------------------------------------------------------------------
void PluginControl_Impl::setZoom( float /*ZoomX*/, float /*ZoomY*/ )
    throw( RuntimeException, std::exception )
{
    // has to be done by further implementation of control
}

//---- css::lang::XEventListener ------------------------------------------------------------------------------
void PluginControl_Impl::disposing( const css::lang::EventObject & /*rSource*/ )
    throw( RuntimeException, std::exception )
{
}
//---- css::awt::XFocusListener ------------------------------------------------------------------------------
void PluginControl_Impl::focusGained( const css::awt::FocusEvent & /*rEvt*/ )
    throw( RuntimeException, std::exception )
{
    if (_xPeerWindow.is())
        _xPeerWindow->setFocus();
}
//---- css::awt::XFocusListener ------------------------------------------------------------------------------
void PluginControl_Impl::focusLost( const css::awt::FocusEvent & /*rEvt*/ )
    throw( RuntimeException, std::exception )
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
