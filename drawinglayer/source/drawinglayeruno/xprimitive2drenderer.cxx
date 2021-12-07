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

#include <sal/config.h>

#include <com/sun/star/graphic/XPrimitive2DRenderer.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <cppuhelper/implbase2.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <comphelper/sequence.hxx>
#include <drawinglayer/geometry/viewinformation2d.hxx>
#include <basegfx/numeric/ftools.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/canvastools.hxx>
#include <com/sun/star/geometry/RealRectangle2D.hpp>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <drawinglayer/primitive2d/transformprimitive2d.hxx>

#include <drawinglayer/converters.hxx>
#include <comphelper/sequenceashashmap.hxx>

using namespace ::com::sun::star;


namespace drawinglayer::unorenderer
{
        namespace {

        class XPrimitive2DRenderer:
            public cppu::WeakAggImplHelper2<
                css::graphic::XPrimitive2DRenderer, css::lang::XServiceInfo>
        {
        public:
            XPrimitive2DRenderer();

            XPrimitive2DRenderer(const XPrimitive2DRenderer&) = delete;
            const XPrimitive2DRenderer& operator=(const XPrimitive2DRenderer&) = delete;

            // XPrimitive2DRenderer
            virtual uno::Reference< rendering::XBitmap > SAL_CALL rasterize(
                const uno::Sequence< uno::Reference< graphic::XPrimitive2D > >& Primitive2DSequence,
                const uno::Sequence< beans::PropertyValue >& aViewInformationSequence,
                ::sal_uInt32 DPI_X,
                ::sal_uInt32 DPI_Y,
                const css::geometry::RealRectangle2D& Range,
                ::sal_uInt32 MaximumQuadraticPixels) override;

            // XServiceInfo
            virtual OUString SAL_CALL getImplementationName() override;
            virtual sal_Bool SAL_CALL supportsService(const OUString&) override;
            virtual uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;
        };

        }

        XPrimitive2DRenderer::XPrimitive2DRenderer()
        {
        }

        uno::Reference< rendering::XBitmap > XPrimitive2DRenderer::rasterize(
            const uno::Sequence< uno::Reference< graphic::XPrimitive2D > >& aPrimitive2DSequence,
            const uno::Sequence< beans::PropertyValue >& aViewInformationSequence,
            ::sal_uInt32 DPI_X,
            ::sal_uInt32 DPI_Y,
            const css::geometry::RealRectangle2D& Range,
            ::sal_uInt32 MaximumQuadraticPixels)
        {
            o3tl::Length eRangeUnit = o3tl::Length::mm100;
            comphelper::SequenceAsHashMap aViewInformationMap(aViewInformationSequence);
            auto it = aViewInformationMap.find("RangeUnit");
            if (it != aViewInformationMap.end())
            {
                sal_Int32 nVal{};
                it->second >>= nVal;
                eRangeUnit = static_cast<o3tl::Length>(nVal);
            }

            uno::Reference< rendering::XBitmap > XBitmap;

            if(aPrimitive2DSequence.hasElements())
            {
                const basegfx::B2DRange aRange(Range.X1, Range.Y1, Range.X2, Range.Y2);
                const double fWidth(aRange.getWidth());
                const double fHeight(aRange.getHeight());

                if(basegfx::fTools::more(fWidth, 0.0) && basegfx::fTools::more(fHeight, 0.0))
                {
                    if(0 == DPI_X)
                    {
                        DPI_X = 75;
                    }

                    if(0 == DPI_Y)
                    {
                        DPI_Y = 75;
                    }

                    if(0 == MaximumQuadraticPixels)
                    {
                        MaximumQuadraticPixels = 500000;
                    }

                    const auto aViewInformation2D = geometry::createViewInformation2D(aViewInformationSequence);
                    const sal_uInt32 nDiscreteWidth(basegfx::fround(o3tl::convert(fWidth, eRangeUnit, o3tl::Length::in) * DPI_X));
                    const sal_uInt32 nDiscreteHeight(basegfx::fround(o3tl::convert(fHeight, eRangeUnit, o3tl::Length::in) * DPI_Y));

                    basegfx::B2DHomMatrix aEmbedding(
                        basegfx::utils::createTranslateB2DHomMatrix(
                            -aRange.getMinX(),
                            -aRange.getMinY()));

                    aEmbedding.scale(
                        nDiscreteWidth / fWidth,
                        nDiscreteHeight / fHeight);

                    const primitive2d::Primitive2DReference xEmbedRef(
                        new primitive2d::TransformPrimitive2D(
                            aEmbedding,
                            aPrimitive2DSequence));
                    primitive2d::Primitive2DContainer xEmbedSeq { xEmbedRef };

                    BitmapEx aBitmapEx(
                        convertToBitmapEx(
                            std::move(xEmbedSeq),
                            aViewInformation2D,
                            nDiscreteWidth,
                            nDiscreteHeight,
                            MaximumQuadraticPixels));

                    if(!aBitmapEx.IsEmpty())
                    {
                        aBitmapEx.SetPrefMapMode(MapMode(MapUnit::Map100thMM));
                        aBitmapEx.SetPrefSize(Size(basegfx::fround(fWidth), basegfx::fround(fHeight)));
                        XBitmap = vcl::unotools::xBitmapFromBitmapEx(aBitmapEx);
                    }
                }
            }

            return XBitmap;
        }

        OUString SAL_CALL XPrimitive2DRenderer::getImplementationName()
        {
            return "drawinglayer::unorenderer::XPrimitive2DRenderer";
        }

        sal_Bool SAL_CALL XPrimitive2DRenderer::supportsService(const OUString& rServiceName)
        {
            return cppu::supportsService(this, rServiceName);
        }

        uno::Sequence< OUString > SAL_CALL XPrimitive2DRenderer::getSupportedServiceNames()
        {
            return { "com.sun.star.graphic.Primitive2DTools" };
        }

} // end of namespace


extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
drawinglayer_XPrimitive2DRenderer(
    css::uno::XComponentContext* , css::uno::Sequence<css::uno::Any> const& )
{
    return cppu::acquire(new drawinglayer::unorenderer::XPrimitive2DRenderer());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
