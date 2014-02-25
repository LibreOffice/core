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


#include "SlideRenderer.hxx"
#include "sdpage.hxx"
#include <toolkit/helper/vclunohelper.hxx>
#include <com/sun/star/rendering/XBitmapCanvas.hpp>
#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>
#include <cppcanvas/vclfactory.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

namespace sd { namespace presenter {

//===== Service ===============================================================

Reference<XInterface> SAL_CALL SlideRenderer_createInstance (
    const Reference<XComponentContext>& rxContext)
{
    return Reference<XInterface>(static_cast<XWeak*>(new SlideRenderer(rxContext)));
}




OUString SlideRenderer_getImplementationName (void) throw(RuntimeException)
{
    return OUString("com.sun.star.comp.Draw.SlideRenderer");
}




Sequence<OUString> SAL_CALL SlideRenderer_getSupportedServiceNames (void)
    throw (RuntimeException)
{
    static const OUString sServiceName("com.sun.star.drawing.SlideRenderer");
    return Sequence<OUString>(&sServiceName, 1);
}




//===== SlideRenderer ==========================================================

SlideRenderer::SlideRenderer (const Reference<XComponentContext>& rxContext)
    : SlideRendererInterfaceBase(m_aMutex),
      maPreviewRenderer()
{
    (void)rxContext;
}




SlideRenderer::~SlideRenderer (void)
{
}




void SAL_CALL SlideRenderer::disposing (void)
{
}




//----- XInitialization -------------------------------------------------------

void SAL_CALL SlideRenderer::initialize (const Sequence<Any>& rArguments)
    throw (Exception, RuntimeException, std::exception)
{
    ThrowIfDisposed();

    if (rArguments.getLength() != 0)
    {
        throw RuntimeException("SlideRenderer: invalid number of arguments",
                static_cast<XWeak*>(this));
    }
}




//----- XSlideRenderer --------------------------------------------------------

Reference<awt::XBitmap> SlideRenderer::createPreview (
    const Reference<drawing::XDrawPage>& rxSlide,
    const awt::Size& rMaximalSize,
    sal_Int16 nSuperSampleFactor)
    throw (css::uno::RuntimeException, std::exception)
{
    ThrowIfDisposed();
    SolarMutexGuard aGuard;

    return VCLUnoHelper::CreateBitmap(
        CreatePreview(rxSlide, rMaximalSize, nSuperSampleFactor));
}




Reference<rendering::XBitmap> SlideRenderer::createPreviewForCanvas (
    const Reference<drawing::XDrawPage>& rxSlide,
    const awt::Size& rMaximalSize,
    sal_Int16 nSuperSampleFactor,
    const Reference<rendering::XCanvas>& rxCanvas)
    throw (css::uno::RuntimeException, std::exception)
{
    ThrowIfDisposed();
    SolarMutexGuard aGuard;

    cppcanvas::CanvasSharedPtr pCanvas (
        cppcanvas::VCLFactory::getInstance().createCanvas(rxCanvas));
    if (pCanvas.get() != NULL)
        return cppcanvas::VCLFactory::getInstance().createBitmap(
            pCanvas,
            CreatePreview(rxSlide, rMaximalSize, nSuperSampleFactor))->getUNOBitmap();
    else
        return NULL;
}




awt::Size SAL_CALL SlideRenderer::calculatePreviewSize (
    double nSlideAspectRatio,
    const awt::Size& rMaximalSize)
    throw (css::uno::RuntimeException, std::exception)
{
    if (rMaximalSize.Width <= 0
        || rMaximalSize.Height <= 0
        || nSlideAspectRatio <= 0)
    {
        return awt::Size(0,0);
    }

    const double nWindowAspectRatio (double(rMaximalSize.Width) / double(rMaximalSize.Height));
    if (nSlideAspectRatio < nWindowAspectRatio)
        return awt::Size(
            sal::static_int_cast<sal_Int32>(rMaximalSize.Height * nSlideAspectRatio),
            rMaximalSize.Height);
    else
        return awt::Size(
            rMaximalSize.Width,
            sal::static_int_cast<sal_Int32>(rMaximalSize.Width / nSlideAspectRatio));
}






BitmapEx SlideRenderer::CreatePreview (
    const Reference<drawing::XDrawPage>& rxSlide,
    const awt::Size& rMaximalSize,
    sal_Int16 nSuperSampleFactor)
    throw (css::uno::RuntimeException)
{
    const SdPage* pPage = SdPage::getImplementation(rxSlide);
    if (pPage == NULL)
        throw lang::IllegalArgumentException("SlideRenderer::createPreview() called with invalid slide",
            static_cast<XWeak*>(this),
            0);

    // Determine the size of the current slide and its aspect ratio.
    Size aPageSize = pPage->GetSize();
    if (aPageSize.Height() <= 0)
        throw lang::IllegalArgumentException("SlideRenderer::createPreview() called with invalid size",
            static_cast<XWeak*>(this),
            1);

    // Compare with the aspect ratio of the window (which rMaximalSize
    // assumed to be) and calculate the size of the preview so that it
    // a) will have the aspect ratio of the page and
    // b) will be as large as possible.
    awt::Size aPreviewSize (calculatePreviewSize(
        double(aPageSize.Width()) / double(aPageSize.Height()),
        rMaximalSize));
    if (aPreviewSize.Width <= 0 || aPreviewSize.Height <= 0)
        return BitmapEx();

    // Make sure that the super sample factor has a sane value.
    sal_Int16 nFactor (nSuperSampleFactor);
    if (nFactor < 1)
        nFactor = 1;
    else if (nFactor > 10)
        nFactor = 10;

    // Create the preview.  When the super sample factor n is greater than 1
    // then a preview is created in size (n*width, n*height) and then scaled
    // down to (width, height).  This is a poor mans antialiasing for the
    // time being.  When we have true antialiasing support this workaround
    // can be removed.
    const Image aPreview = maPreviewRenderer.RenderPage (
        pPage,
        Size(aPreviewSize.Width*nFactor, aPreviewSize.Height*nFactor),
        OUString());
    if (nFactor == 1)
        return aPreview.GetBitmapEx();
    else
    {
        BitmapEx aScaledPreview = aPreview.GetBitmapEx();
        aScaledPreview.Scale(
            Size(aPreviewSize.Width,aPreviewSize.Height),
            BMP_SCALE_BESTQUALITY);
        return aScaledPreview;
    }
}




void SlideRenderer::ThrowIfDisposed (void)
    throw (::com::sun::star::lang::DisposedException)
{
    if (SlideRendererInterfaceBase::rBHelper.bDisposed || SlideRendererInterfaceBase::rBHelper.bInDispose)
    {
        throw lang::DisposedException ("SlideRenderer object has already been disposed",
            static_cast<uno::XWeak*>(this));
    }
}


} } // end of namespace ::sd::presenter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
