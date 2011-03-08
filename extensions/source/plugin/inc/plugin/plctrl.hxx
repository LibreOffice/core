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
#ifndef __PLCTRL_HXX
#define __PLCTRL_HXX

#include <tools/debug.hxx>

#include <cppuhelper/weak.hxx>
#include <plugin/multiplx.hxx>
#include <com/sun/star/beans/PropertyValues.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/PropertyState.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/beans/XFastPropertySet.hpp>
#include <com/sun/star/beans/XVetoableChangeListener.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/beans/XPropertyStateChangeListener.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/XPropertiesChangeListener.hpp>
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#include <com/sun/star/beans/XPropertyAccess.hpp>
#include <com/sun/star/beans/XPropertyContainer.hpp>
#include <com/sun/star/beans/PropertyStateChangeEvent.hpp>
#include <com/sun/star/beans/PropertyChangeEvent.hpp>
#include <com/sun/star/awt/XVclContainerPeer.hpp>
#include <com/sun/star/awt/XVclWindowPeer.hpp>
#include <com/sun/star/awt/XControlModel.hpp>
#include <com/sun/star/awt/XUnoControlContainer.hpp>
#include <com/sun/star/awt/XControlContainer.hpp>
#include <com/sun/star/awt/VclWindowPeerAttribute.hpp>
#include <com/sun/star/awt/XVclContainer.hpp>
#include <com/sun/star/awt/XControl.hpp>
#include <com/sun/star/awt/XTopWindow.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/awt/PosSize.hpp>

#include <cppuhelper/implbase4.hxx>

#include <list>

class SystemChildWindow;

//==================================================================================================
class PluginControl_Impl : public ::cppu::WeakAggImplHelper4<
      ::com::sun::star::awt::XControl,
      ::com::sun::star::awt::XWindow,
      ::com::sun::star::awt::XFocusListener,
      ::com::sun::star::awt::XView >
{
public:
    // ::com::sun::star::awt::XControl
    virtual void SAL_CALL setContext( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > & xContext ) throw( ::com::sun::star::uno::RuntimeException )
    { _xContext = xContext; }
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL getContext() throw( ::com::sun::star::uno::RuntimeException )
    { return _xContext; }

    virtual sal_Bool SAL_CALL setModel( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > & Model ) throw( ::com::sun::star::uno::RuntimeException ) = 0;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > SAL_CALL getModel() throw( ::com::sun::star::uno::RuntimeException ) = 0;

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XView > SAL_CALL getView() throw( ::com::sun::star::uno::RuntimeException )
    { return (::com::sun::star::awt::XView*)this; }

    virtual sal_Bool SAL_CALL isTransparent() throw( ::com::sun::star::uno::RuntimeException )
    { return sal_False; }

    virtual void SAL_CALL setDesignMode( sal_Bool bOn ) throw( ::com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL isDesignMode() throw( ::com::sun::star::uno::RuntimeException )
    { return _bInDesignMode; }

    virtual void SAL_CALL createPeer( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit > & xToolkit, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer > & Parent) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer > SAL_CALL getPeer() throw( ::com::sun::star::uno::RuntimeException )
    { return _xPeer; }

    // ::com::sun::star::awt::XWindow
    virtual void SAL_CALL setVisible( sal_Bool bVisible ) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL setEnable( sal_Bool bEnable ) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL setFocus(void) throw( ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL setPosSize( sal_Int32 nX_, sal_Int32 nY_, sal_Int32 nWidth_, sal_Int32 nHeight_, sal_Int16 nFlags ) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::awt::Rectangle SAL_CALL getPosSize(void) throw( ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL addWindowListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowListener > & l ) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL removeWindowListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowListener > & l ) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL addFocusListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XFocusListener > & l ) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL removeFocusListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XFocusListener > & l ) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL addKeyListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XKeyListener > & l ) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL removeKeyListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XKeyListener > & l ) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL addMouseListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMouseListener > & l ) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL removeMouseListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMouseListener > & l ) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL addMouseMotionListener( const Reference< ::com::sun::star::awt::XMouseMotionListener > & l ) throw( RuntimeException );
    virtual void SAL_CALL removeMouseMotionListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMouseMotionListener > & l ) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL addPaintListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XPaintListener > & l ) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL removePaintListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XPaintListener > & l ) throw( ::com::sun::star::uno::RuntimeException );

    // ::com::sun::star::lang::XEventListener
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject & rSource ) throw( ::com::sun::star::uno::RuntimeException );
    // ::com::sun::star::awt::XFocusListener
    virtual void SAL_CALL focusGained( const ::com::sun::star::awt::FocusEvent & rEvt ) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL focusLost( const ::com::sun::star::awt::FocusEvent & rEvt ) throw( ::com::sun::star::uno::RuntimeException );

    // ::com::sun::star::lang::XComponent
    virtual void SAL_CALL addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener > & l ) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener > & l ) throw( ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL dispose() throw( ::com::sun::star::uno::RuntimeException );

    // ::com::sun::star::awt::XView
    virtual sal_Bool SAL_CALL setGraphics( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XGraphics > & /*aDevice*/ ) throw( ::com::sun::star::uno::RuntimeException )
    { return sal_False; }
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XGraphics > SAL_CALL getGraphics(void) throw( ::com::sun::star::uno::RuntimeException )
    { return ::com::sun::star::uno::Reference< ::com::sun::star::awt::XGraphics > (); }

    virtual ::com::sun::star::awt::Size SAL_CALL getSize(void) throw( ::com::sun::star::uno::RuntimeException )
    { return ::com::sun::star::awt::Size(_nWidth, _nHeight); }

    virtual void SAL_CALL draw( sal_Int32 x, sal_Int32 y ) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL setZoom( float ZoomX, float ZoomY ) throw( ::com::sun::star::uno::RuntimeException );

public:
                                PluginControl_Impl();
    virtual                     ~PluginControl_Impl();

    MRCListenerMultiplexerHelper* getMultiplexer();

protected:
    void                        releasePeer();

protected:
    ::std::list< Reference< ::com::sun::star::lang::XEventListener > >  _aDisposeListeners;
    MRCListenerMultiplexerHelper*       _pMultiplexer;

    Reference< XInterface >                         _xContext;

    sal_Int32                               _nX;
    sal_Int32                               _nY;
    sal_Int32                               _nWidth;
    sal_Int32                               _nHeight;
    sal_Int16                               _nFlags;

    sal_Bool                                _bVisible;
    sal_Bool                                _bInDesignMode;
    sal_Bool                                _bEnable;

    SystemChildWindow*                  _pSysChild;
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >                      _xPeer;
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >                          _xPeerWindow;

    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >                          _xParentWindow;
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >                      _xParentPeer;
};

#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
