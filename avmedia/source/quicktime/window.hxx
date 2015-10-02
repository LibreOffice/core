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

#ifndef INCLUDED_AVMEDIA_SOURCE_QUICKTIME_WINDOW_HXX
#define INCLUDED_AVMEDIA_SOURCE_QUICKTIME_WINDOW_HXX

#include "quicktimecommon.hxx"
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/interfacecontainer.h>

#include "com/sun/star/media/XPlayerWindow.hpp"

namespace avmedia { namespace quicktime {

class Player;

class Window : public ::cppu::WeakImplHelper< css::media::XPlayerWindow,
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
    virtual void SAL_CALL update( ) override;
    virtual sal_Bool SAL_CALL setZoomLevel( css::media::ZoomLevel ) override;
    virtual css::media::ZoomLevel SAL_CALL getZoomLevel( ) override;
    virtual void SAL_CALL setPointerType( sal_Int32 nPointerType ) override;

    // XWindow
    virtual void SAL_CALL setPosSize( sal_Int32 , sal_Int32 , sal_Int32 Width, sal_Int32 Height, sal_Int16 Flags ) override;
    virtual css::awt::Rectangle SAL_CALL getPosSize( ) override;
    virtual void SAL_CALL setVisible( sal_Bool ) override;
    virtual void SAL_CALL setEnable( sal_Bool ) override;
    virtual void SAL_CALL setFocus( ) override;
    virtual void SAL_CALL addWindowListener( const css::uno::Reference< css::awt::XWindowListener >& xListener ) override;
    virtual void SAL_CALL removeWindowListener( const css::uno::Reference< css::awt::XWindowListener >& xListener ) override;
    virtual void SAL_CALL addFocusListener( const css::uno::Reference< css::awt::XFocusListener >& xListener ) override;
    virtual void SAL_CALL removeFocusListener( const css::uno::Reference< css::awt::XFocusListener >& xListener ) override;
    virtual void SAL_CALL addKeyListener( const css::uno::Reference< css::awt::XKeyListener >& xListener ) override;
    virtual void SAL_CALL removeKeyListener( const css::uno::Reference< css::awt::XKeyListener >& xListener ) override;
    virtual void SAL_CALL addMouseListener( const css::uno::Reference< css::awt::XMouseListener >& xListener ) override;
    virtual void SAL_CALL removeMouseListener( const css::uno::Reference< css::awt::XMouseListener >& xListener ) override;
    virtual void SAL_CALL addMouseMotionListener( const css::uno::Reference< css::awt::XMouseMotionListener >& xListener ) override;
    virtual void SAL_CALL removeMouseMotionListener( const css::uno::Reference< css::awt::XMouseMotionListener >& xListener ) override;
    virtual void SAL_CALL addPaintListener( const css::uno::Reference< css::awt::XPaintListener >& xListener ) override;
    virtual void SAL_CALL removePaintListener( const css::uno::Reference< css::awt::XPaintListener >& xListener ) override;

    // XComponent
    virtual void SAL_CALL dispose( ) override;
    virtual void SAL_CALL addEventListener( const css::uno::Reference< css::lang::XEventListener >& xListener ) override;
    virtual void SAL_CALL removeEventListener( const css::uno::Reference< css::lang::XEventListener >& aListener ) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName( ) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames( ) override;

private:

    css::uno::Reference< css::lang::XMultiServiceFactory > mxMgr;

    ::osl::Mutex                                maMutex;
    ::cppu::OMultiTypeInterfaceContainerHelper  maListeners;
    css::media::ZoomLevel                       meZoomLevel;
    Player&                                     mrPlayer;
    int                                         mnPointerType;

    NSView*                                     mpParentView; // parent view for our own private movie view
    QTMovieView*                                mpMovieView;  // the view containing the movie object, output target and controller

    void                                        ImplLayoutVideoWindow();
};

} // namespace quicktime
} // namespace avmedia

#endif // INCLUDED_AVMEDIA_SOURCE_QUICKTIME_WINDOW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
