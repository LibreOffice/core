/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: SlideRenderer.cxx,v $
 *
 * $Revision: 1.3 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include "precompiled_sd.hxx"

#include "SlideRenderer.hxx"
#include "sdpage.hxx"
#include <toolkit/helper/vclunohelper.hxx>
#include <vos/mutex.hxx>
#include <vcl/svapp.hxx>
#include <cppcanvas/vclfactory.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using ::rtl::OUString;

namespace sd { namespace presenter {

//===== Service ===============================================================

Reference<XInterface> SAL_CALL SlideRenderer_createInstance (
    const Reference<XComponentContext>& rxContext)
{
    return Reference<XInterface>(static_cast<XWeak*>(new SlideRenderer(rxContext)));
}




::rtl::OUString SlideRenderer_getImplementationName (void) throw(RuntimeException)
{
    return OUString::createFromAscii("com.sun.star.comp.Draw.SlideRenderer");
}




Sequence<rtl::OUString> SAL_CALL SlideRenderer_getSupportedServiceNames (void)
    throw (RuntimeException)
{
    static const ::rtl::OUString sServiceName(
        ::rtl::OUString::createFromAscii("com.sun.star.drawing.SlideRenderer"));
    return Sequence<rtl::OUString>(&sServiceName, 1);
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
    throw (Exception, RuntimeException)
{
    ThrowIfDisposed();

    if (rArguments.getLength() == 0)
    {
        try
        {
        }
        catch (RuntimeException&)
        {
            throw;
        }
    }
    else
    {
        throw RuntimeException(
            OUString::createFromAscii("SlideRenderer: invalid number of arguments"),
                static_cast<XWeak*>(this));
    }
}




//----- XSlideRenderer --------------------------------------------------------

Reference<awt::XBitmap> SlideRenderer::createPreview (
    const Reference<drawing::XDrawPage>& rxSlide,
    const awt::Size& rMaximalSize,
    sal_Int16 nSuperSampleFactor)
    throw (css::uno::RuntimeException)
{
    ThrowIfDisposed();
    ::vos::OGuard aGuard (Application::GetSolarMutex());

    return VCLUnoHelper::CreateBitmap(
        CreatePreview(rxSlide, rMaximalSize, nSuperSampleFactor));
}




Reference<rendering::XBitmap> SlideRenderer::createPreviewForCanvas (
    const Reference<drawing::XDrawPage>& rxSlide,
    const awt::Size& rMaximalSize,
    sal_Int16 nSuperSampleFactor,
    const Reference<rendering::XCanvas>& rxCanvas)
    throw (css::uno::RuntimeException)
{
    ThrowIfDisposed();
    ::vos::OGuard aGuard (Application::GetSolarMutex());

    cppcanvas::BitmapCanvasSharedPtr pCanvas (cppcanvas::VCLFactory::getInstance().createCanvas(
        Reference<rendering::XBitmapCanvas>(rxCanvas, UNO_QUERY)));
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
    throw (css::uno::RuntimeException)
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




//-----------------------------------------------------------------------------

BitmapEx SlideRenderer::CreatePreview (
    const Reference<drawing::XDrawPage>& rxSlide,
    const awt::Size& rMaximalSize,
    sal_Int16 nSuperSampleFactor)
    throw (css::uno::RuntimeException)
{
    const SdPage* pPage = SdPage::getImplementation(rxSlide);
    if (pPage == NULL)
        throw lang::IllegalArgumentException(
            OUString::createFromAscii("SlideRenderer::createPreview() called with invalid slide"),
            static_cast<XWeak*>(this),
            0);

    // Determine the size of the current slide and its aspect ratio.
    Size aPageSize = pPage->GetSize();
    if (aPageSize.Height() <= 0)
        throw lang::IllegalArgumentException(
            OUString::createFromAscii("SlideRenderer::createPreview() called with invalid size"),
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
        ::rtl::OUString());
    if (nFactor == 1)
        return aPreview.GetBitmapEx();
    else
    {
        BitmapEx aScaledPreview = aPreview.GetBitmapEx();
        aScaledPreview.Scale(
            Size(aPreviewSize.Width,aPreviewSize.Height),
            BMP_SCALE_INTERPOLATE);
        return aScaledPreview;
    }
}




void SlideRenderer::ThrowIfDisposed (void)
    throw (::com::sun::star::lang::DisposedException)
{
    if (SlideRendererInterfaceBase::rBHelper.bDisposed || SlideRendererInterfaceBase::rBHelper.bInDispose)
    {
        throw lang::DisposedException (
            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                "SlideRenderer object has already been disposed")),
            static_cast<uno::XWeak*>(this));
    }
}


} } // end of namespace ::sd::presenter

