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

#ifndef INCLUDED_AVMEDIA_SOURCE_MACAVF_WINDOW_HXX
#define INCLUDED_AVMEDIA_SOURCE_MACAVF_WINDOW_HXX

#include "macavfcommon.hxx"
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/interfacecontainer.h>

#include "com/sun/star/media/XPlayerWindow.hdl"

// ---------------
// - MyMediaView -
// ---------------

@interface MyMediaView : NSView
@property (nonatomic, readonly, strong) AVPlayer* player;
@property (nonatomic, readonly, strong) AVPlayerLayer* playerLayer;
@property (nonatomic, retain) NSURL* videoURL;
- (void) play;
@end

namespace avmedia { namespace macavf {

// ---------------
// - Window -
// ---------------

class Player;

class Window
:   public MacAVObserverHandler
,   public ::cppu::WeakImplHelper< css::media::XPlayerWindow,
                                   css::lang::XServiceInfo >
{
public:

            Window( const css::uno::Reference< css::lang::XMultiServiceFactory >& i_rxMgr,
                    Player& i_rPlayer,
                    NSView* i_pParentView
                    );
    virtual ~Window();

    void    processGraphEvent();
    void    updatePointer();

    // XPlayerWindow
    virtual void SAL_CALL update(  ) throw (css::uno::RuntimeException) override;
    virtual sal_Bool SAL_CALL setZoomLevel( css::media::ZoomLevel ZoomLevel ) throw (css::uno::RuntimeException) override;
    virtual css::media::ZoomLevel SAL_CALL getZoomLevel(  ) throw (css::uno::RuntimeException) override;
    virtual void SAL_CALL setPointerType( sal_Int32 nPointerType ) throw (css::uno::RuntimeException) override;

    // XWindow
    virtual void SAL_CALL setPosSize( sal_Int32 X, sal_Int32 Y, sal_Int32 Width, sal_Int32 Height, sal_Int16 Flags ) throw (css::uno::RuntimeException) override;
    virtual css::awt::Rectangle SAL_CALL getPosSize(  ) throw (css::uno::RuntimeException) override;
    virtual void SAL_CALL setVisible( sal_Bool Visible ) throw (css::uno::RuntimeException) override;
    virtual void SAL_CALL setEnable( sal_Bool Enable ) throw (css::uno::RuntimeException) override;
    virtual void SAL_CALL setFocus(  ) throw (css::uno::RuntimeException) override;
    virtual void SAL_CALL addWindowListener( const css::uno::Reference< css::awt::XWindowListener >& xListener ) throw (css::uno::RuntimeException) override;
    virtual void SAL_CALL removeWindowListener( const css::uno::Reference< css::awt::XWindowListener >& xListener ) throw (css::uno::RuntimeException) override;
    virtual void SAL_CALL addFocusListener( const css::uno::Reference< css::awt::XFocusListener >& xListener ) throw (css::uno::RuntimeException) override;
    virtual void SAL_CALL removeFocusListener( const css::uno::Reference< css::awt::XFocusListener >& xListener ) throw (css::uno::RuntimeException) override;
    virtual void SAL_CALL addKeyListener( const css::uno::Reference< css::awt::XKeyListener >& xListener ) throw (css::uno::RuntimeException) override;
    virtual void SAL_CALL removeKeyListener( const css::uno::Reference< css::awt::XKeyListener >& xListener ) throw (css::uno::RuntimeException) override;
    virtual void SAL_CALL addMouseListener( const css::uno::Reference< css::awt::XMouseListener >& xListener ) throw (css::uno::RuntimeException) override;
    virtual void SAL_CALL removeMouseListener( const css::uno::Reference< css::awt::XMouseListener >& xListener ) throw (css::uno::RuntimeException) override;
    virtual void SAL_CALL addMouseMotionListener( const css::uno::Reference< css::awt::XMouseMotionListener >& xListener ) throw (css::uno::RuntimeException) override;
    virtual void SAL_CALL removeMouseMotionListener( const css::uno::Reference< css::awt::XMouseMotionListener >& xListener ) throw (css::uno::RuntimeException) override;
    virtual void SAL_CALL addPaintListener( const css::uno::Reference< css::awt::XPaintListener >& xListener ) throw (css::uno::RuntimeException) override;
    virtual void SAL_CALL removePaintListener( const css::uno::Reference< css::awt::XPaintListener >& xListener ) throw (css::uno::RuntimeException) override;

    // XComponent
    virtual void SAL_CALL dispose(  ) throw (css::uno::RuntimeException) override;
    virtual void SAL_CALL addEventListener( const css::uno::Reference< css::lang::XEventListener >& xListener ) throw (css::uno::RuntimeException) override;
    virtual void SAL_CALL removeEventListener( const css::uno::Reference< css::lang::XEventListener >& aListener ) throw (css::uno::RuntimeException) override;

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw (css::uno::RuntimeException) override;
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw (css::uno::RuntimeException) override;
    virtual css::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw (css::uno::RuntimeException) override;

    virtual bool handleObservation( NSString* pKeyPath ) override;

private:

    css::uno::Reference< css::lang::XMultiServiceFactory > mxMgr;

    ::osl::Mutex                                maMutex;
    ::cppu::OMultiTypeInterfaceContainerHelper  maListeners;
    css::media::ZoomLevel                       meZoomLevel;
    Player&                                     mrPlayer;
    int                                         mnPointerType;

    NSView*                                     mpView; // parent-view == movie-view
    AVPlayerLayer*                              mpPlayerLayer;

    void                                        ImplLayoutVideoWindow();
};

} // namespace macavf
} // namespace avmedia

#endif // INCLUDED_AVMEDIA_SOURCE_MACAVF_WINDOW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
