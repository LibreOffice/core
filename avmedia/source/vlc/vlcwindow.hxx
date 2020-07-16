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

#include "vlccommon.hxx"
#include <cppuhelper/implbase.hxx>

namespace avmedia::vlc {
class VLCPlayer;

class VLCWindow : public ::cppu::WeakImplHelper< css::media::XPlayerWindow,
                                                 css::lang::XServiceInfo >
{
    VLCPlayer&            mPlayer;
    const intptr_t        mPrevWinID;
    css::media::ZoomLevel meZoomLevel;
    css::awt::Rectangle   mSize;
public:
    VLCWindow( VLCPlayer& player, const intptr_t prevWinID );
    virtual ~VLCWindow() override;

    void SAL_CALL update() override;
    sal_Bool SAL_CALL setZoomLevel( css::media::ZoomLevel ZoomLevel ) override;
    css::media::ZoomLevel SAL_CALL getZoomLevel() override;
    void SAL_CALL setPointerType( ::sal_Int32 SystemPointerType ) override;

    OUString SAL_CALL getImplementationName() override;
    sal_Bool SAL_CALL supportsService( const OUString& serviceName ) override;
    css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    void SAL_CALL dispose() override;
    void SAL_CALL addEventListener( const css::uno::Reference< css::lang::XEventListener >& xListener ) override;
    void SAL_CALL removeEventListener( const css::uno::Reference< css::lang::XEventListener >& aListener ) override;

    void SAL_CALL setPosSize( sal_Int32 X, sal_Int32 Y, sal_Int32 Width, sal_Int32 Height, sal_Int16 Flags ) override;
    css::awt::Rectangle SAL_CALL getPosSize() override;
    void SAL_CALL setVisible( sal_Bool Visible ) override;
    void SAL_CALL setEnable( sal_Bool Enable ) override;
    void SAL_CALL setFocus() override;
    void SAL_CALL addWindowListener( const css::uno::Reference< css::awt::XWindowListener >& xListener ) override;
    void SAL_CALL removeWindowListener( const css::uno::Reference< css::awt::XWindowListener >& xListener ) override;
    void SAL_CALL addFocusListener( const css::uno::Reference< css::awt::XFocusListener >& xListener ) override;
    void SAL_CALL removeFocusListener( const css::uno::Reference< css::awt::XFocusListener >& xListener ) override;
    void SAL_CALL addKeyListener( const css::uno::Reference< css::awt::XKeyListener >& xListener ) override;
    void SAL_CALL removeKeyListener( const css::uno::Reference< css::awt::XKeyListener >& xListener ) override;
    void SAL_CALL addMouseListener( const css::uno::Reference< css::awt::XMouseListener >& xListener ) override;
    void SAL_CALL removeMouseListener( const css::uno::Reference< css::awt::XMouseListener >& xListener ) override;
    void SAL_CALL addMouseMotionListener( const css::uno::Reference< css::awt::XMouseMotionListener >& xListener ) override;
    void SAL_CALL removeMouseMotionListener( const css::uno::Reference< css::awt::XMouseMotionListener >& xListener ) override;
    void SAL_CALL addPaintListener( const css::uno::Reference< css::awt::XPaintListener >& xListener ) override;
    void SAL_CALL removePaintListener( const css::uno::Reference< css::awt::XPaintListener >& xListener ) override;
};

}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
