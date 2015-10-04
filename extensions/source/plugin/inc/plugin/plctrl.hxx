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
#ifndef INCLUDED_EXTENSIONS_SOURCE_PLUGIN_INC_PLUGIN_PLCTRL_HXX
#define INCLUDED_EXTENSIONS_SOURCE_PLUGIN_INC_PLUGIN_PLCTRL_HXX

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

#include <cppuhelper/implbase.hxx>
#include <vcl/vclptr.hxx>

#include <list>

class SystemChildWindow;


class PluginControl_Impl : public ::cppu::WeakAggImplHelper<
      css::awt::XControl,
      css::awt::XWindow,
      css::awt::XFocusListener,
      css::awt::XView >
{
public:
    // css::awt::XControl
    virtual void SAL_CALL setContext( const css::uno::Reference< css::uno::XInterface > & xContext ) throw( css::uno::RuntimeException, std::exception ) override
    { _xContext = xContext; }
    virtual css::uno::Reference< css::uno::XInterface > SAL_CALL getContext() throw( css::uno::RuntimeException, std::exception ) override
    { return _xContext; }

    virtual sal_Bool SAL_CALL setModel( const css::uno::Reference< css::awt::XControlModel > & Model ) throw( css::uno::RuntimeException, std::exception ) override = 0;
    virtual css::uno::Reference< css::awt::XControlModel > SAL_CALL getModel() throw( css::uno::RuntimeException, std::exception ) override = 0;

    virtual css::uno::Reference< css::awt::XView > SAL_CALL getView() throw( css::uno::RuntimeException, std::exception ) override
    { return static_cast<css::awt::XView*>(this); }

    virtual sal_Bool SAL_CALL isTransparent() throw( css::uno::RuntimeException, std::exception ) override
    { return sal_False; }

    virtual void SAL_CALL setDesignMode( sal_Bool bOn ) throw( css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL isDesignMode() throw( css::uno::RuntimeException, std::exception ) override
    { return _bInDesignMode; }

    virtual void SAL_CALL createPeer( const css::uno::Reference< css::awt::XToolkit > & xToolkit, const css::uno::Reference< css::awt::XWindowPeer > & Parent) throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Reference< css::awt::XWindowPeer > SAL_CALL getPeer() throw( css::uno::RuntimeException, std::exception ) override
    { return _xPeer; }

    // css::awt::XWindow
    virtual void SAL_CALL setVisible( sal_Bool bVisible ) throw( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL setEnable( sal_Bool bEnable ) throw( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL setFocus() throw( css::uno::RuntimeException, std::exception ) override;

    virtual void SAL_CALL setPosSize( sal_Int32 nX_, sal_Int32 nY_, sal_Int32 nWidth_, sal_Int32 nHeight_, sal_Int16 nFlags ) throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::awt::Rectangle SAL_CALL getPosSize() throw( css::uno::RuntimeException, std::exception ) override;

    virtual void SAL_CALL addWindowListener( const css::uno::Reference< css::awt::XWindowListener > & l ) throw( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL removeWindowListener( const css::uno::Reference< css::awt::XWindowListener > & l ) throw( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL addFocusListener( const css::uno::Reference< css::awt::XFocusListener > & l ) throw( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL removeFocusListener( const css::uno::Reference< css::awt::XFocusListener > & l ) throw( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL addKeyListener( const css::uno::Reference< css::awt::XKeyListener > & l ) throw( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL removeKeyListener( const css::uno::Reference< css::awt::XKeyListener > & l ) throw( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL addMouseListener( const css::uno::Reference< css::awt::XMouseListener > & l ) throw( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL removeMouseListener( const css::uno::Reference< css::awt::XMouseListener > & l ) throw( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL addMouseMotionListener( const Reference< css::awt::XMouseMotionListener > & l ) throw( RuntimeException, std::exception ) override;
    virtual void SAL_CALL removeMouseMotionListener( const css::uno::Reference< css::awt::XMouseMotionListener > & l ) throw( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL addPaintListener( const css::uno::Reference< css::awt::XPaintListener > & l ) throw( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL removePaintListener( const css::uno::Reference< css::awt::XPaintListener > & l ) throw( css::uno::RuntimeException, std::exception ) override;

    // css::lang::XEventListener
    virtual void SAL_CALL disposing( const css::lang::EventObject & rSource ) throw( css::uno::RuntimeException, std::exception ) override;
    // css::awt::XFocusListener
    virtual void SAL_CALL focusGained( const css::awt::FocusEvent & rEvt ) throw( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL focusLost( const css::awt::FocusEvent & rEvt ) throw( css::uno::RuntimeException, std::exception ) override;

    // css::lang::XComponent
    virtual void SAL_CALL addEventListener( const css::uno::Reference< css::lang::XEventListener > & l ) throw( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL removeEventListener( const css::uno::Reference< css::lang::XEventListener > & l ) throw( css::uno::RuntimeException, std::exception ) override;

    virtual void SAL_CALL dispose() throw( css::uno::RuntimeException, std::exception ) override;

    // css::awt::XView
    virtual sal_Bool SAL_CALL setGraphics( const css::uno::Reference< css::awt::XGraphics > & /*aDevice*/ ) throw( css::uno::RuntimeException, std::exception ) override
    { return sal_False; }
    virtual css::uno::Reference< css::awt::XGraphics > SAL_CALL getGraphics() throw( css::uno::RuntimeException, std::exception ) override
    { return css::uno::Reference< css::awt::XGraphics > (); }

    virtual css::awt::Size SAL_CALL getSize() throw( css::uno::RuntimeException, std::exception ) override
    { return css::awt::Size(_nWidth, _nHeight); }

    virtual void SAL_CALL draw( sal_Int32 x, sal_Int32 y ) throw( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL setZoom( float ZoomX, float ZoomY ) throw( css::uno::RuntimeException, std::exception ) override;

public:
                                PluginControl_Impl();
    virtual                     ~PluginControl_Impl();

    MRCListenerMultiplexerHelper* getMultiplexer();

protected:
    void                        releasePeer();

protected:
    ::std::list< Reference< css::lang::XEventListener > >  _aDisposeListeners;
    MRCListenerMultiplexerHelper*           _pMultiplexer;

    Reference< XInterface >                 _xContext;

    sal_Int32                               _nX;
    sal_Int32                               _nY;
    sal_Int32                               _nWidth;
    sal_Int32                               _nHeight;
    sal_Int16                               _nFlags;

    bool                                    _bVisible;
    bool                                    _bInDesignMode;
    bool                                    _bEnable;

    VclPtr<SystemChildWindow>               _pSysChild;
    css::uno::Reference< css::awt::XWindowPeer >                      _xPeer;
    css::uno::Reference< css::awt::XWindow >                          _xPeerWindow;

    css::uno::Reference< css::awt::XWindow >                          _xParentWindow;
    css::uno::Reference< css::awt::XWindowPeer >                      _xParentPeer;
};

#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
