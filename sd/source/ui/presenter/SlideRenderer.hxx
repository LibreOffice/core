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

#include <PreviewRenderer.hxx>
#include <com/sun/star/drawing/XSlideRenderer.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>

#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/compbase.hxx>

namespace com::sun::star::drawing { class XDrawPage; }

namespace sd::presenter {

typedef ::cppu::WeakComponentImplHelper <
    css::drawing::XSlideRenderer,
    css::lang::XInitialization,
    css::lang::XServiceInfo
> SlideRendererInterfaceBase;

/** Render single slides into bitmaps.
*/
class SlideRenderer
    : protected ::cppu::BaseMutex,
      public SlideRendererInterfaceBase
{
public:
    SlideRenderer ();
    virtual ~SlideRenderer() override;
    SlideRenderer(const SlideRenderer&) = delete;
    SlideRenderer& operator=(const SlideRenderer&) = delete;
    virtual void SAL_CALL disposing() override;

    // XInitialization

    virtual void SAL_CALL initialize (const css::uno::Sequence<css::uno::Any>& rArguments) override;

    OUString SAL_CALL getImplementationName() override;

    sal_Bool SAL_CALL supportsService(OUString const & ServiceName) override;

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;

    // XSlideRenderer

    virtual css::uno::Reference<css::awt::XBitmap> SAL_CALL createPreview (
        const css::uno::Reference<css::drawing::XDrawPage>& rxSlide,
        const css::awt::Size& rMaximumPreviewPixelSize,
        sal_Int16 nSuperSampleFactor) override;

    virtual css::uno::Reference<css::rendering::XBitmap> SAL_CALL createPreviewForCanvas (
        const css::uno::Reference<css::drawing::XDrawPage>& rxSlide,
        const css::awt::Size& rMaximumPreviewPixelSize,
        sal_Int16 nSuperSampleFactor,
        const css::uno::Reference<css::rendering::XCanvas>& rxCanvas) override;

    virtual css::awt::Size SAL_CALL calculatePreviewSize (
        double nSlideAspectRatio,
        const css::awt::Size& rMaximumPreviewPixelSize) override;

private:
    PreviewRenderer maPreviewRenderer;

    /// @throws css::uno::RuntimeException
    BitmapEx CreatePreview (
        const css::uno::Reference<css::drawing::XDrawPage>& rxSlide,
        const css::awt::Size& rMaximumPreviewPixelSize,
        sal_Int16 nSuperSampleFactor);

    /** @throws css::lang::DisposedException when the object has already been
        disposed.
    */
    void ThrowIfDisposed();
};

} // end of namespace ::sd::presenter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
