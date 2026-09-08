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

namespace cpo::uno { class XComponentContext; }
namespace com::sun::star::awt { struct WindowEvent; }
namespace unocontrols { class OMRCListenerMultiplexerHelper; }

namespace unocontrols {

class BaseControl : public cppu::BaseMutex,
                    public cppu::WeakComponentImplHelper<css::lang::XServiceInfo,
                                                         css::awt::XPaintListener,
                                                         css::awt::XWindowListener,
                                                         css::awt::XView,
                                                         css::awt::XWindow,
                                                         css::awt::XControl>
{
public:
    BaseControl( const cpo::uno::Reference< cpo::uno::XComponentContext >& rxContext );

    virtual ~BaseControl() override;

    //  XServiceInfo

    virtual bool supportsService(
        const OUString& sServiceName
    ) override;

    virtual OUString getImplementationName() override;

    virtual cpo::uno::Sequence< OUString > getSupportedServiceNames() override;

    //  XComponent

    virtual void dispose() override;

    virtual void addEventListener(
        const cpo::uno::Reference< css::lang::XEventListener >& xListener
    ) override;

    virtual void removeEventListener(
        const cpo::uno::Reference< css::lang::XEventListener >& xListener
    ) override;

    //  XControl

    virtual void createPeer(
        const cpo::uno::Reference< css::awt::XToolkit >& xToolkit,
        const cpo::uno::Reference< css::awt::XWindowPeer >& xParent
    ) override;

    virtual void setContext(
        const cpo::uno::Reference< cpo::uno::XInterface >& xContext
    ) override;

    virtual bool setModel(
        const cpo::uno::Reference< css::awt::XControlModel >& xModel
    ) override = 0;

    virtual void setDesignMode( bool bOn ) override;

    virtual cpo::uno::Reference< cpo::uno::XInterface > getContext() override;

    virtual cpo::uno::Reference< css::awt::XControlModel > getModel() override = 0;

    virtual cpo::uno::Reference< css::awt::XWindowPeer > getPeer() override;

    virtual cpo::uno::Reference< css::awt::XView > getView() override;

    virtual bool isDesignMode() override;

    virtual bool isTransparent() override;

    //  XWindow

    virtual void setPosSize(   sal_Int32   nX      ,
                                        sal_Int32   nY      ,
                                        sal_Int32   nWidth  ,
                                        sal_Int32   nHeight ,
                                        sal_Int16   nFlags  ) override;

    virtual void setVisible( bool bVisible ) override;

    virtual void setEnable( bool bEnable ) override;

    virtual void setFocus() override;

    virtual css::awt::Rectangle getPosSize() override;

    virtual void addWindowListener(
        const cpo::uno::Reference< css::awt::XWindowListener >& xListener
    ) override;

    virtual void addFocusListener(
        const cpo::uno::Reference< css::awt::XFocusListener >& xListener
    ) override;

    virtual void addKeyListener(
        const cpo::uno::Reference< css::awt::XKeyListener >& xListener ) override;

    virtual void addMouseListener(
        const cpo::uno::Reference< css::awt::XMouseListener >& xListener
    ) override;

    virtual void addMouseMotionListener(
        const cpo::uno::Reference< css::awt::XMouseMotionListener >& xListener
    ) override;

    virtual void addPaintListener(
        const cpo::uno::Reference< css::awt::XPaintListener >& xListener
    ) override;

    virtual void removeWindowListener(
        const cpo::uno::Reference< css::awt::XWindowListener >& xListener
    ) override;

    virtual void removeFocusListener(
        const cpo::uno::Reference< css::awt::XFocusListener >& xListener
    ) override;

    virtual void removeKeyListener(
        const cpo::uno::Reference< css::awt::XKeyListener >& xListener
    ) override;

    virtual void removeMouseListener(
        const cpo::uno::Reference< css::awt::XMouseListener >& xListener
    ) override;

    virtual void removeMouseMotionListener(
        const cpo::uno::Reference< css::awt::XMouseMotionListener >& xListener
    ) override;

    virtual void removePaintListener(
        const cpo::uno::Reference< css::awt::XPaintListener >& xListener
    ) override;

    //  XView

    virtual void draw( sal_Int32   nX  ,
                                sal_Int32   nY  ) override;

    virtual bool setGraphics(
        const cpo::uno::Reference< css::awt::XGraphics >& xDevice
    ) override;

    virtual void setZoom(  float   fZoomX  ,
                                    float   fZoomY  ) override;

    virtual cpo::uno::Reference< css::awt::XGraphics > getGraphics() override;

    virtual css::awt::Size getSize() override;

    //  css::lang::XEventListener

    virtual void disposing(
        const css::lang::EventObject& rSource
    ) override;

    //  XPaintListener

    virtual void windowPaint(
        const css::awt::PaintEvent& rEvent
    ) override;

    //  XWindowListener

    virtual void windowResized( const css::awt::WindowEvent& aEvent ) override;
    virtual void windowMoved( const css::awt::WindowEvent& aEvent ) override;
    virtual void windowShown( const css::lang::EventObject& aEvent ) override;
    virtual void windowHidden( const css::lang::EventObject& aEvent ) override;

protected:
    using WeakComponentImplHelperBase::disposing;

    const cpo::uno::Reference< cpo::uno::XComponentContext >& impl_getComponentContext() const { return m_xComponentContext;}

    const cpo::uno::Reference< css::awt::XWindow >& impl_getPeerWindow() const { return m_xPeerWindow;}

    const cpo::uno::Reference< css::awt::XGraphics >& impl_getGraphicsPeer() const { return m_xGraphicsPeer;}

    sal_Int32 impl_getWidth() const { return m_nWidth;}

    sal_Int32 impl_getHeight() const { return m_nHeight;}

    virtual css::awt::WindowDescriptor impl_getWindowDescriptor(
        const cpo::uno::Reference< css::awt::XWindowPeer >& xParentPeer
    );

    virtual void impl_paint(        sal_Int32               nX          ,
                                    sal_Int32               nY          ,
                            const   cpo::uno::Reference< css::awt::XGraphics >&   xGraphics   );

    virtual void impl_recalcLayout( const css::awt::WindowEvent& aEvent );

private:
    OMRCListenerMultiplexerHelper* impl_getMultiplexer();

    cpo::uno::Reference< cpo::uno::XComponentContext >        m_xComponentContext;
    rtl::Reference<OMRCListenerMultiplexerHelper>             m_xMultiplexer;   // multiplex events
    cpo::uno::Reference< cpo::uno::XInterface >               m_xContext;
    cpo::uno::Reference< css::awt::XWindowPeer >              m_xPeer;
    cpo::uno::Reference< css::awt::XWindow >                  m_xPeerWindow;
    cpo::uno::Reference< css::awt::XGraphics >                m_xGraphicsView;   // graphics for css::awt::XView-operations
    cpo::uno::Reference< css::awt::XGraphics >                m_xGraphicsPeer;   // graphics for painting on a peer
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
