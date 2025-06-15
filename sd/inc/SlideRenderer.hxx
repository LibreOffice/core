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

#include "sddllapi.h"
#include <PreviewRenderer.hxx>
#include <comphelper/compbase.hxx>
#include <com/sun/star/awt/XBitmap.hpp>
#include <com/sun/star/rendering/XBitmap.hpp>

namespace com::sun::star::drawing { class XDrawPage; }

namespace sd::presenter {

typedef comphelper::WeakComponentImplHelper<> SlideRendererInterfaceBase;

/** Render single slides into bitmaps.
*/
class SD_DLLPUBLIC SlideRenderer final
    : public SlideRendererInterfaceBase
{
public:
    SlideRenderer ();
    virtual ~SlideRenderer() override;
    SlideRenderer(const SlideRenderer&) = delete;
    SlideRenderer& operator=(const SlideRenderer&) = delete;

    css::uno::Reference<css::awt::XBitmap> createPreview (
        const css::uno::Reference<css::drawing::XDrawPage>& rxSlide,
        const css::awt::Size& rMaximumPreviewPixelSize,
        sal_Int16 nSuperSampleFactor);

    css::uno::Reference<css::rendering::XBitmap> createPreviewForCanvas (
        const css::uno::Reference<css::drawing::XDrawPage>& rxSlide,
        const css::awt::Size& rMaximumPreviewPixelSize,
        sal_Int16 nSuperSampleFactor,
        const css::uno::Reference<css::rendering::XCanvas>& rxCanvas);

    static css::awt::Size calculatePreviewSize (
        double nSlideAspectRatio,
        const css::awt::Size& rMaximumPreviewPixelSize);

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
