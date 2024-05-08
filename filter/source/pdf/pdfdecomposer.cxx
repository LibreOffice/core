/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <vector>

#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <drawinglayer/primitive2d/bitmapprimitive2d.hxx>
#include <drawinglayer/primitive2d/Primitive2DContainer.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/pdfread.hxx>
#include <vcl/svapp.hxx>
#include <vcl/outdev.hxx>
#include <vcl/BinaryDataContainer.hxx>
#include <vcl/BinaryDataContainerTools.hxx>
#include <toolkit/helper/vclunohelper.hxx>

#include <com/sun/star/graphic/XPdfDecomposer.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/util/XBinaryDataContainer.hpp>

using namespace css;

namespace
{
/// Class to convert the PDF data into a XPrimitive2D (containing only a bitmap).
class XPdfDecomposer : public ::cppu::WeakImplHelper<graphic::XPdfDecomposer, lang::XServiceInfo>
{
public:
    explicit XPdfDecomposer(uno::Reference<uno::XComponentContext> const& context);
    XPdfDecomposer(const XPdfDecomposer&) = delete;
    XPdfDecomposer& operator=(const XPdfDecomposer&) = delete;

    // XPdfDecomposer
    uno::Sequence<uno::Reference<graphic::XPrimitive2D>> SAL_CALL
    getDecomposition(const uno::Reference<util::XBinaryDataContainer>& xDataContainer,
                     const uno::Sequence<beans::PropertyValue>& xDecompositionParameters) override;

    // XServiceInfo
    OUString SAL_CALL getImplementationName() override;
    sal_Bool SAL_CALL supportsService(const OUString&) override;
    uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;
};

XPdfDecomposer::XPdfDecomposer(uno::Reference<uno::XComponentContext> const&) {}

uno::Sequence<uno::Reference<graphic::XPrimitive2D>> SAL_CALL
XPdfDecomposer::getDecomposition(const uno::Reference<util::XBinaryDataContainer>& xDataContainer,
                                 const uno::Sequence<beans::PropertyValue>& xParameters)
{
    sal_Int32 nPageIndex = -1;

    for (const beans::PropertyValue& rProperty : xParameters)
    {
        if (rProperty.Name == "PageIndex")
        {
            rProperty.Value >>= nPageIndex;
            break;
        }
    }

    if (nPageIndex < 0)
        nPageIndex = 0;

    BinaryDataContainer aDataContainer = vcl::convertUnoBinaryDataContainer(xDataContainer);

    std::vector<BitmapEx> aBitmaps;
    int rv = vcl::RenderPDFBitmaps(aDataContainer.getData(), aDataContainer.getSize(), aBitmaps,
                                   nPageIndex, 1);
    if (rv == 0)
        return {}; // happens if we do not have PDFium

    BitmapEx aReplacement(aBitmaps[0]);

    // short form for scale and translate transformation
    const Size aBitmapSize(aReplacement.GetSizePixel());
    // ImpGraphic::getPrefMapMode() requires mm100 for bitmaps rendered from vector graphic data.
    const Size aMM100(
        Application::GetDefaultDevice()->PixelToLogic(aBitmapSize, MapMode(MapUnit::Map100thMM)));
    const basegfx::B2DHomMatrix aBitmapTransform(basegfx::utils::createScaleTranslateB2DHomMatrix(
        aMM100.getWidth(), aMM100.getHeight(), 0, 0));

    // create primitive
    return drawinglayer::primitive2d::Primitive2DContainer{
        new drawinglayer::primitive2d::BitmapPrimitive2D(aReplacement, aBitmapTransform)
    }
        .toSequence();
}

OUString SAL_CALL XPdfDecomposer::getImplementationName()
{
    return u"com.sun.star.comp.PDF.PDFDecomposer"_ustr;
}

sal_Bool SAL_CALL XPdfDecomposer::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence<OUString> SAL_CALL XPdfDecomposer::getSupportedServiceNames()
{
    return { u"com.sun.star.graphic.PdfTools"_ustr };
}
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
filter_PdfDecomposer_get_implementation(css::uno::XComponentContext* context,
                                        css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new XPdfDecomposer(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
