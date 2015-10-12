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

#ifndef INCLUDED_SD_SOURCE_UI_PRESENTER_SLIDERENDERER_HXX
#define INCLUDED_SD_SOURCE_UI_PRESENTER_SLIDERENDERER_HXX

#include "PreviewRenderer.hxx"
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/drawing/XSlideRenderer.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/rendering/XSpriteCanvas.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/compbase.hxx>
#include <boost/noncopyable.hpp>

namespace sd { namespace presenter {

namespace {
    typedef ::cppu::WeakComponentImplHelper <
        css::drawing::XSlideRenderer,
        css::lang::XInitialization,
        css::lang::XServiceInfo
    > SlideRendererInterfaceBase;
}

/** Render single slides into bitmaps.
*/
class SlideRenderer
    : private ::boost::noncopyable,
      protected ::cppu::BaseMutex,
      public SlideRendererInterfaceBase
{
public:
    explicit SlideRenderer (const css::uno::Reference<css::uno::XComponentContext>& rxContext);
    virtual ~SlideRenderer();
    virtual void SAL_CALL disposing() override;

    // XInitialization

    virtual void SAL_CALL initialize (const css::uno::Sequence<css::uno::Any>& rArguments)
        throw (css::uno::Exception, css::uno::RuntimeException, std::exception) override;

    OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) override;

    sal_Bool SAL_CALL supportsService(OUString const & ServiceName)
        throw (css::uno::RuntimeException, std::exception) override;

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames()
        throw (css::uno::RuntimeException, std::exception) override;

    // XSlideRenderer

    virtual css::uno::Reference<css::awt::XBitmap> SAL_CALL createPreview (
        const css::uno::Reference<css::drawing::XDrawPage>& rxSlide,
        const css::awt::Size& rMaximumPreviewPixelSize,
        sal_Int16 nSuperSampleFactor)
        throw (css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Reference<css::rendering::XBitmap> SAL_CALL createPreviewForCanvas (
        const css::uno::Reference<css::drawing::XDrawPage>& rxSlide,
        const css::awt::Size& rMaximumPreviewPixelSize,
        sal_Int16 nSuperSampleFactor,
        const css::uno::Reference<css::rendering::XCanvas>& rxCanvas)
        throw (css::uno::RuntimeException, std::exception) override;

    virtual css::awt::Size SAL_CALL calculatePreviewSize (
        double nSlideAspectRatio,
        const css::awt::Size& rMaximumPreviewPixelSize)
        throw (css::uno::RuntimeException, std::exception) override;

private:
    PreviewRenderer maPreviewRenderer;

    BitmapEx CreatePreview (
        const css::uno::Reference<css::drawing::XDrawPage>& rxSlide,
        const css::awt::Size& rMaximumPreviewPixelSize,
        sal_Int16 nSuperSampleFactor)
        throw (css::uno::RuntimeException, std::exception);

    /** This method throws a DisposedException when the object has already been
        disposed.
    */
    void ThrowIfDisposed() throw (css::lang::DisposedException);
};

} } // end of namespace ::sd::presenter

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
