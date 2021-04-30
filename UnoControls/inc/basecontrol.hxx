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

#pragma once

#include <com/sun/star/awt/XPaintListener.hpp>
#include <com/sun/star/awt/XWindowListener.hpp>
#include <com/sun/star/awt/XControl.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/awt/XView.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <osl/mutex.hxx>
#include <cppuhelper/component.hxx>
#include <rtl/ref.hxx>

namespace com::sun::star::uno { class XComponentContext; }
namespace com::sun::star::awt { class XFocusListener; }
namespace com::sun::star::awt { class XMouseListener; }
namespace com::sun::star::awt { class XMouseMotionListener; }
namespace com::sun::star::awt { struct PaintEvent; }
namespace com::sun::star::awt { struct WindowEvent; }
namespace unocontrols { class OMRCListenerMultiplexerHelper; }

namespace unocontrols {

struct IMPL_MutexContainer
{
    // Is necessary to initialize "BaseControl" and make this class thread-safe.
    ::osl::Mutex m_aMutex;
};

class BaseControl   : public css::lang::XServiceInfo
                    , public css::awt::XPaintListener
                    , public css::awt::XWindowListener
                    , public css::awt::XView
                    , public css::awt::XWindow
                    , public css::awt::XControl
                    , public IMPL_MutexContainer
                    , public ::cppu::OComponentHelper
{
public:
    BaseControl( const css::uno::Reference< css::uno::XComponentContext >& rxContext );

    virtual ~BaseControl() override;

    //  XInterface

    /**
        @short      give answer, if interface is supported
        @descr      The interfaces are searched by type.

        @seealso    XInterface

        @param      "rType" is the type of searched interface.

        @return     Any     information about found interface

        @onerror    A RuntimeException is thrown.
    */

    virtual css::uno::Any SAL_CALL queryInterface(
        const css::uno::Type& aType
    ) override;

    /**
        @short      increment refcount
        @seealso    XInterface
        @seealso    release()
        @onerror    A RuntimeException is thrown.
    */

    virtual void SAL_CALL acquire() noexcept override;

    /**
        @short      decrement refcount
        @seealso    XInterface
        @seealso    acquire()
        @onerror    A RuntimeException is thrown.
    */

    virtual void SAL_CALL release() noexcept override;

    //  XTypeProvider

    /**
        @short      get information about supported interfaces
        @seealso    XTypeProvider
        @return     Sequence of types of all supported interfaces

        @onerror    A RuntimeException is thrown.
    */

    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes() override;

    /**
        @short      get implementation id
        @descr      This ID is necessary for UNO-caching. If there no ID, cache is disabled.
                    Another way, cache is enabled.

        @seealso    XTypeProvider
        @return     ID as Sequence of byte

        @onerror    A RuntimeException is thrown.
    */

    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() override;

    //  XAggregation

    virtual void SAL_CALL setDelegator(
        const css::uno::Reference< css::uno::XInterface >& xDelegator
    ) override;

    virtual css::uno::Any SAL_CALL queryAggregation(
        const css::uno::Type& aType
    ) override;

    //  XServiceInfo

    virtual sal_Bool SAL_CALL supportsService(
        const OUString& sServiceName
    ) override;

    virtual OUString SAL_CALL getImplementationName() override;

    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    //  XComponent

    virtual void SAL_CALL dispose() override;

    virtual void SAL_CALL addEventListener(
        const css::uno::Reference< css::lang::XEventListener >& xListener
    ) override;

    virtual void SAL_CALL removeEventListener(
        const css::uno::Reference< css::lang::XEventListener >& xListener
    ) override;

    //  XControl

    virtual void SAL_CALL createPeer(
        const css::uno::Reference< css::awt::XToolkit >& xToolkit,
        const css::uno::Reference< css::awt::XWindowPeer >& xParent
    ) override;

    virtual void SAL_CALL setContext(
        const css::uno::Reference< css::uno::XInterface >& xContext
    ) override;

    virtual sal_Bool SAL_CALL setModel(
        const css::uno::Reference< css::awt::XControlModel >& xModel
    ) override = 0;

    virtual void SAL_CALL setDesignMode( sal_Bool bOn ) override;

    virtual css::uno::Reference< css::uno::XInterface > SAL_CALL getContext() override;

    virtual css::uno::Reference< css::awt::XControlModel > SAL_CALL getModel() override = 0;

    virtual css::uno::Reference< css::awt::XWindowPeer > SAL_CALL getPeer() override;

    virtual css::uno::Reference< css::awt::XView > SAL_CALL getView() override;

    virtual sal_Bool SAL_CALL isDesignMode() override;

    virtual sal_Bool SAL_CALL isTransparent() override;

    //  XWindow

    virtual void SAL_CALL setPosSize(   sal_Int32   nX      ,
                                        sal_Int32   nY      ,
                                        sal_Int32   nWidth  ,
                                        sal_Int32   nHeight ,
                                        sal_Int16   nFlags  ) override;

    virtual void SAL_CALL setVisible( sal_Bool bVisible ) override;

    virtual void SAL_CALL setEnable( sal_Bool bEnable ) override;

    virtual void SAL_CALL setFocus() override;

    virtual css::awt::Rectangle SAL_CALL getPosSize() override;

    virtual void SAL_CALL addWindowListener(
        const css::uno::Reference< css::awt::XWindowListener >& xListener
    ) override;

    virtual void SAL_CALL addFocusListener(
        const css::uno::Reference< css::awt::XFocusListener >& xListener
    ) override;

    virtual void SAL_CALL addKeyListener(
        const css::uno::Reference< css::awt::XKeyListener >& xListener ) override;

    virtual void SAL_CALL addMouseListener(
        const css::uno::Reference< css::awt::XMouseListener >& xListener
    ) override;

    virtual void SAL_CALL addMouseMotionListener(
        const css::uno::Reference< css::awt::XMouseMotionListener >& xListener
    ) override;

    virtual void SAL_CALL addPaintListener(
        const css::uno::Reference< css::awt::XPaintListener >& xListener
    ) override;

    virtual void SAL_CALL removeWindowListener(
        const css::uno::Reference< css::awt::XWindowListener >& xListener
    ) override;

    virtual void SAL_CALL removeFocusListener(
        const css::uno::Reference< css::awt::XFocusListener >& xListener
    ) override;

    virtual void SAL_CALL removeKeyListener(
        const css::uno::Reference< css::awt::XKeyListener >& xListener
    ) override;

    virtual void SAL_CALL removeMouseListener(
        const css::uno::Reference< css::awt::XMouseListener >& xListener
    ) override;

    virtual void SAL_CALL removeMouseMotionListener(
        const css::uno::Reference< css::awt::XMouseMotionListener >& xListener
    ) override;

    virtual void SAL_CALL removePaintListener(
        const css::uno::Reference< css::awt::XPaintListener >& xListener
    ) override;

    //  XView

    virtual void SAL_CALL draw( sal_Int32   nX  ,
                                sal_Int32   nY  ) override;

    virtual sal_Bool SAL_CALL setGraphics(
        const css::uno::Reference< css::awt::XGraphics >& xDevice
    ) override;

    virtual void SAL_CALL setZoom(  float   fZoomX  ,
                                    float   fZoomY  ) override;

    virtual css::uno::Reference< css::awt::XGraphics > SAL_CALL getGraphics() override;

    virtual css::awt::Size SAL_CALL getSize() override;

    //  css::lang::XEventListener

    virtual void SAL_CALL disposing(
        const css::lang::EventObject& rSource
    ) override;

    //  XPaintListener

    virtual void SAL_CALL windowPaint(
        const css::awt::PaintEvent& rEvent
    ) override;

    //  XWindowListener

    virtual void SAL_CALL windowResized( const css::awt::WindowEvent& aEvent ) override;
    virtual void SAL_CALL windowMoved( const css::awt::WindowEvent& aEvent ) override;
    virtual void SAL_CALL windowShown( const css::lang::EventObject& aEvent ) override;
    virtual void SAL_CALL windowHidden( const css::lang::EventObject& aEvent ) override;

protected:
    using OComponentHelper::disposing;

    const css::uno::Reference< css::uno::XComponentContext >& impl_getComponentContext() const { return m_xComponentContext;}

    const css::uno::Reference< css::awt::XWindow >& impl_getPeerWindow() const { return m_xPeerWindow;}

    const css::uno::Reference< css::awt::XGraphics >& impl_getGraphicsPeer() const { return m_xGraphicsPeer;}

    sal_Int32 impl_getWidth() const { return m_nWidth;}

    sal_Int32 impl_getHeight() const { return m_nHeight;}

    virtual css::awt::WindowDescriptor impl_getWindowDescriptor(
        const css::uno::Reference< css::awt::XWindowPeer >& xParentPeer
    );

    virtual void impl_paint(        sal_Int32               nX          ,
                                    sal_Int32               nY          ,
                            const   css::uno::Reference< css::awt::XGraphics >&   xGraphics   );

    virtual void impl_recalcLayout( const css::awt::WindowEvent& aEvent );

    const css::uno::Reference< css::uno::XInterface >& impl_getDelegator() const { return m_xDelegator;}

private:
    OMRCListenerMultiplexerHelper* impl_getMultiplexer();

    css::uno::Reference< css::uno::XComponentContext >        m_xComponentContext;
    css::uno::Reference< css::uno::XInterface >               m_xDelegator;
    rtl::Reference<OMRCListenerMultiplexerHelper>             m_xMultiplexer;   // multiplex events
    css::uno::Reference< css::uno::XInterface >               m_xContext;
    css::uno::Reference< css::awt::XWindowPeer >              m_xPeer;
    css::uno::Reference< css::awt::XWindow >                  m_xPeerWindow;
    css::uno::Reference< css::awt::XGraphics >                m_xGraphicsView;   // graphics for css::awt::XView-operations
    css::uno::Reference< css::awt::XGraphics >                m_xGraphicsPeer;   // graphics for painting on a peer
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
