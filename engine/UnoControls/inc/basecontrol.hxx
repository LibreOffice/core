/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#pragma once

#include <com/sun/star/awt/XPaintListener.hpp>
#include <com/sun/star/awt/XWindowListener.hpp>
#include <com/sun/star/awt/XControl.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/awt/XView.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <cppuhelper/compbase.hxx>
#include <cppuhelper/basemutex.hxx>
#include <rtl/ref.hxx>

namespace com::sun::star::uno { class XComponentContext; }
namespace com::sun::star::awt { struct WindowEvent; }
namespace unocontrols { class OMRCListenerMultiplexerHelper; }

namespace unocontrols {

class BaseControl : public cppu::BaseMutex,
                    public cppu::WeakComponentImplHelper<cpo::lang::XServiceInfo,
                                                         cpo::awt::XPaintListener,
                                                         cpo::awt::XWindowListener,
                                                         cpo::awt::XView,
                                                         cpo::awt::XWindow,
                                                         cpo::awt::XControl>
{
public:
    BaseControl( const cpo::uno::Reference< cpo::uno::XComponentContext >& rxContext );

    virtual ~BaseControl() override;

    //  XServiceInfo

    virtual bool SAL_CALL supportsService(
        const OUString& sServiceName
    ) override;

    virtual OUString SAL_CALL getImplementationName() override;

    virtual cpo::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    //  XComponent

    virtual void SAL_CALL dispose() override;

    virtual void SAL_CALL addEventListener(
        const cpo::uno::Reference< cpo::lang::XEventListener >& xListener
    ) override;

    virtual void SAL_CALL removeEventListener(
        const cpo::uno::Reference< cpo::lang::XEventListener >& xListener
    ) override;

    //  XControl

    virtual void SAL_CALL createPeer(
        const cpo::uno::Reference< cpo::awt::XToolkit >& xToolkit,
        const cpo::uno::Reference< cpo::awt::XWindowPeer >& xParent
    ) override;

    virtual void SAL_CALL setContext(
        const cpo::uno::Reference< cpo::uno::XInterface >& xContext
    ) override;

    virtual bool SAL_CALL setModel(
        const cpo::uno::Reference< cpo::awt::XControlModel >& xModel
    ) override = 0;

    virtual void SAL_CALL setDesignMode( bool bOn ) override;

    virtual cpo::uno::Reference< cpo::uno::XInterface > SAL_CALL getContext() override;

    virtual cpo::uno::Reference< cpo::awt::XControlModel > SAL_CALL getModel() override = 0;

    virtual cpo::uno::Reference< cpo::awt::XWindowPeer > SAL_CALL getPeer() override;

    virtual cpo::uno::Reference< cpo::awt::XView > SAL_CALL getView() override;

    virtual bool SAL_CALL isDesignMode() override;

    virtual bool SAL_CALL isTransparent() override;

    //  XWindow

    virtual void SAL_CALL setPosSize(   sal_Int32   nX      ,
                                        sal_Int32   nY      ,
                                        sal_Int32   nWidth  ,
                                        sal_Int32   nHeight ,
                                        sal_Int16   nFlags  ) override;

    virtual void SAL_CALL setVisible( bool bVisible ) override;

    virtual void SAL_CALL setEnable( bool bEnable ) override;

    virtual void SAL_CALL setFocus() override;

    virtual cpo::awt::Rectangle SAL_CALL getPosSize() override;

    virtual void SAL_CALL addWindowListener(
        const cpo::uno::Reference< cpo::awt::XWindowListener >& xListener
    ) override;

    virtual void SAL_CALL addFocusListener(
        const cpo::uno::Reference< cpo::awt::XFocusListener >& xListener
    ) override;

    virtual void SAL_CALL addKeyListener(
        const cpo::uno::Reference< cpo::awt::XKeyListener >& xListener ) override;

    virtual void SAL_CALL addMouseListener(
        const cpo::uno::Reference< cpo::awt::XMouseListener >& xListener
    ) override;

    virtual void SAL_CALL addMouseMotionListener(
        const cpo::uno::Reference< cpo::awt::XMouseMotionListener >& xListener
    ) override;

    virtual void SAL_CALL addPaintListener(
        const cpo::uno::Reference< cpo::awt::XPaintListener >& xListener
    ) override;

    virtual void SAL_CALL removeWindowListener(
        const cpo::uno::Reference< cpo::awt::XWindowListener >& xListener
    ) override;

    virtual void SAL_CALL removeFocusListener(
        const cpo::uno::Reference< cpo::awt::XFocusListener >& xListener
    ) override;

    virtual void SAL_CALL removeKeyListener(
        const cpo::uno::Reference< cpo::awt::XKeyListener >& xListener
    ) override;

    virtual void SAL_CALL removeMouseListener(
        const cpo::uno::Reference< cpo::awt::XMouseListener >& xListener
    ) override;

    virtual void SAL_CALL removeMouseMotionListener(
        const cpo::uno::Reference< cpo::awt::XMouseMotionListener >& xListener
    ) override;

    virtual void SAL_CALL removePaintListener(
        const cpo::uno::Reference< cpo::awt::XPaintListener >& xListener
    ) override;

    //  XView

    virtual void SAL_CALL draw( sal_Int32   nX  ,
                                sal_Int32   nY  ) override;

    virtual bool SAL_CALL setGraphics(
        const cpo::uno::Reference< cpo::awt::XGraphics >& xDevice
    ) override;

    virtual void SAL_CALL setZoom(  float   fZoomX  ,
                                    float   fZoomY  ) override;

    virtual cpo::uno::Reference< cpo::awt::XGraphics > SAL_CALL getGraphics() override;

    virtual cpo::awt::Size SAL_CALL getSize() override;

    //  cpo::lang::XEventListener

    virtual void SAL_CALL disposing(
        const cpo::lang::EventObject& rSource
    ) override;

    //  XPaintListener

    virtual void SAL_CALL windowPaint(
        const cpo::awt::PaintEvent& rEvent
    ) override;

    //  XWindowListener

    virtual void SAL_CALL windowResized( const cpo::awt::WindowEvent& aEvent ) override;
    virtual void SAL_CALL windowMoved( const cpo::awt::WindowEvent& aEvent ) override;
    virtual void SAL_CALL windowShown( const cpo::lang::EventObject& aEvent ) override;
    virtual void SAL_CALL windowHidden( const cpo::lang::EventObject& aEvent ) override;

protected:
    using WeakComponentImplHelperBase::disposing;

    const cpo::uno::Reference< cpo::uno::XComponentContext >& impl_getComponentContext() const { return m_xComponentContext;}

    const cpo::uno::Reference< cpo::awt::XWindow >& impl_getPeerWindow() const { return m_xPeerWindow;}

    const cpo::uno::Reference< cpo::awt::XGraphics >& impl_getGraphicsPeer() const { return m_xGraphicsPeer;}

    sal_Int32 impl_getWidth() const { return m_nWidth;}

    sal_Int32 impl_getHeight() const { return m_nHeight;}

    virtual cpo::awt::WindowDescriptor impl_getWindowDescriptor(
        const cpo::uno::Reference< cpo::awt::XWindowPeer >& xParentPeer
    );

    virtual void impl_paint(        sal_Int32               nX          ,
                                    sal_Int32               nY          ,
                            const   cpo::uno::Reference< cpo::awt::XGraphics >&   xGraphics   );

    virtual void impl_recalcLayout( const cpo::awt::WindowEvent& aEvent );

private:
    OMRCListenerMultiplexerHelper* impl_getMultiplexer();

    cpo::uno::Reference< cpo::uno::XComponentContext >        m_xComponentContext;
    rtl::Reference<OMRCListenerMultiplexerHelper>             m_xMultiplexer;   // multiplex events
    cpo::uno::Reference< cpo::uno::XInterface >               m_xContext;
    cpo::uno::Reference< cpo::awt::XWindowPeer >              m_xPeer;
    cpo::uno::Reference< cpo::awt::XWindow >                  m_xPeerWindow;
    cpo::uno::Reference< cpo::awt::XGraphics >                m_xGraphicsView;   // graphics for cpo::awt::XView-operations
    cpo::uno::Reference< cpo::awt::XGraphics >                m_xGraphicsPeer;   // graphics for painting on a peer
    sal_Int32                                       m_nX;   // Position ...
    sal_Int32                                       m_nY;
    sal_Int32                                       m_nWidth;   // ... and size of window
    sal_Int32                                       m_nHeight;
    bool                                        m_bVisible;   // Some state flags
    bool                                        m_bInDesignMode;
    bool                                        m_bEnable;
};

}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
