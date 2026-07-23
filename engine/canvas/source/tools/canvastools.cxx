/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <limits>

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <basegfx/numeric/ftools.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/point/b2ipoint.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/range/b2drange.hxx>
#include <basegfx/range/b2drectangle.hxx>
#include <basegfx/range/b2irange.hxx>
#include <basegfx/utils/canvastools.hxx>
#include <basegfx/vector/b2ivector.hxx>
#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/awt/XWindow2.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/geometry/AffineMatrix2D.hpp>
#include <com/sun/star/geometry/Matrix2D.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/rendering/ColorComponentTag.hpp>
#include <com/sun/star/rendering/ColorSpaceType.hpp>
#include <com/sun/star/rendering/CompositeOperation.hpp>
#include <com/sun/star/rendering/IntegerBitmapLayout.hpp>
#include <com/sun/star/rendering/RenderState.hpp>
#include <com/sun/star/rendering/RenderingIntent.hpp>
#include <com/sun/star/rendering/ViewState.hpp>
#include <com/sun/star/rendering/XCanvas.hpp>
#include <com/sun/star/rendering/XColorSpace.hpp>
#include <com/sun/star/rendering/XIntegerBitmapColorSpace.hpp>
#include <com/sun/star/util/Endianness.hpp>
#include <cppuhelper/implbase.hxx>
#include <sal/log.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <vcl/canvastools.hxx>
#include <vcl/window.hxx>

#include <canvas/canvastools.hxx>


using namespace ::com::sun::star;

namespace canvastools
{
        geometry::RealSize2D createInfiniteSize2D()
        {
            return geometry::RealSize2D(
                std::numeric_limits<double>::infinity(),
                std::numeric_limits<double>::infinity() );
        }

        rendering::RenderState& initRenderState( rendering::RenderState& renderState )
        {
            // setup identity transform
            setIdentityAffineMatrix2D( renderState.AffineTransform );
            renderState.Clip.clear();
            renderState.DeviceColor = cpo::uno::Sequence< double >();
            renderState.CompositeOperation = rendering::CompositeOperation::OVER;

            return renderState;
        }

        rendering::ViewState& initViewState( rendering::ViewState& viewState )
        {
            // setup identity transform
            setIdentityAffineMatrix2D( viewState.AffineTransform );

            return viewState;
        }

        ::basegfx::B2DHomMatrix getViewStateTransform( const rendering::ViewState& viewState )
        {
            ::basegfx::B2DHomMatrix aTransform;
            return ::basegfx::unotools::homMatrixFromAffineMatrix( aTransform, viewState.AffineTransform );
        }

        rendering::ViewState& setViewStateTransform( rendering::ViewState&          viewState,
                                                     const ::basegfx::B2DHomMatrix& transform )
        {
            ::basegfx::unotools::affineMatrixFromHomMatrix( viewState.AffineTransform, transform );

            return viewState;
        }

        ::basegfx::B2DHomMatrix getRenderStateTransform( const rendering::RenderState& renderState )
        {
            ::basegfx::B2DHomMatrix aTransform;
            return ::basegfx::unotools::homMatrixFromAffineMatrix( aTransform, renderState.AffineTransform );
        }

        rendering::RenderState& setRenderStateTransform( rendering::RenderState&        renderState,
                                                         const ::basegfx::B2DHomMatrix& transform )
        {
            ::basegfx::unotools::affineMatrixFromHomMatrix( renderState.AffineTransform, transform );

            return renderState;
        }

        rendering::RenderState& appendToRenderState( rendering::RenderState&        renderState,
                                                   const ::basegfx::B2DHomMatrix&   rTransform )
        {
            ::basegfx::B2DHomMatrix transform = getRenderStateTransform( renderState );
            return setRenderStateTransform( renderState, transform * rTransform );
        }

        rendering::RenderState& prependToRenderState( rendering::RenderState&           renderState,
                                                      const ::basegfx::B2DHomMatrix&    rTransform )
        {
            ::basegfx::B2DHomMatrix transform = getRenderStateTransform( renderState );
            return setRenderStateTransform( renderState, rTransform * transform );
        }

        ::basegfx::B2DHomMatrix& mergeViewAndRenderTransform( ::basegfx::B2DHomMatrix&      combinedTransform,
                                                              const rendering::ViewState&   viewState,
                                                              const rendering::RenderState& renderState )
        {
            ::basegfx::B2DHomMatrix viewTransform;

            ::basegfx::unotools::homMatrixFromAffineMatrix( combinedTransform, renderState.AffineTransform );
            ::basegfx::unotools::homMatrixFromAffineMatrix( viewTransform, viewState.AffineTransform );

            // this statement performs combinedTransform = viewTransform * combinedTransform
            combinedTransform *= viewTransform;

            return combinedTransform;
        }

        geometry::AffineMatrix2D& setIdentityAffineMatrix2D( geometry::AffineMatrix2D& matrix )
        {
            matrix.m00 = 1.0;
            matrix.m01 = 0.0;
            matrix.m02 = 0.0;
            matrix.m10 = 0.0;
            matrix.m11 = 1.0;
            matrix.m12 = 0.0;

            return matrix;
        }

        geometry::Matrix2D& setIdentityMatrix2D( geometry::Matrix2D& matrix )
        {
            matrix.m00 = 1.0;
            matrix.m01 = 0.0;
            matrix.m10 = 0.0;
            matrix.m11 = 1.0;

            return matrix;
        }

        namespace
        {
            class StandardColorSpace : public cppu::WeakImplHelper< css::rendering::XIntegerBitmapColorSpace >
            {
            private:
                cpo::uno::Sequence< sal_Int8 >  maComponentTags;
                cpo::uno::Sequence< sal_Int32 > maBitCounts;

                virtual ::sal_Int8 getType(  ) override
                {
                    return rendering::ColorSpaceType::RGB;
                }
                virtual cpo::uno::Sequence< ::sal_Int8 > getComponentTags(  ) override
                {
                    return maComponentTags;
                }
                virtual ::sal_Int8 getRenderingIntent(  ) override
                {
                    return rendering::RenderingIntent::PERCEPTUAL;
                }
                virtual cpo::uno::Sequence< beans::PropertyValue > getProperties(  ) override
                {
                    return cpo::uno::Sequence< beans::PropertyValue >();
                }
                virtual cpo::uno::Sequence< double > convertColorSpace( const cpo::uno::Sequence< double >& deviceColor,
                                                                            const uno::Reference< rendering::XColorSpace >& targetColorSpace ) override
                {
                    // TODO(P3): if we know anything about target
                    // colorspace, this can be greatly sped up
                    cpo::uno::Sequence<rendering::ARGBColor> aIntermediate(
                        convertToARGB(deviceColor));
                    return targetColorSpace->convertFromARGB(aIntermediate);
                }
                virtual cpo::uno::Sequence< rendering::RGBColor > convertToRGB( const cpo::uno::Sequence< double >& deviceColor ) override
                {
                    const sal_Int32 nLen(deviceColor.getLength());
                    ENSURE_ARG_OR_THROW2(nLen%4==0,
                                         "number of channels no multiple of 4",
                                         static_cast<rendering::XColorSpace*>(this), 0);

                    cpo::uno::Sequence< rendering::RGBColor > aRes(nLen/4);
                    rendering::RGBColor* pOut( aRes.getArray() );
                    for (sal_Int32 i = 0; i < nLen; i += 4)
                    {
                        *pOut++ = rendering::RGBColor(deviceColor[i], deviceColor[i+1], deviceColor[i+2]);
                    }
                    return aRes;
                }
                virtual cpo::uno::Sequence< rendering::ARGBColor > convertToARGB( const cpo::uno::Sequence< double >& deviceColor ) override
                {
                    SAL_WARN_IF(!deviceColor.hasElements(), "canvas", "empty deviceColor argument");
                    const sal_Int32 nLen(deviceColor.getLength());
                    ENSURE_ARG_OR_THROW2(nLen%4==0,
                                         "number of channels no multiple of 4",
                                         static_cast<rendering::XColorSpace*>(this), 0);

                    cpo::uno::Sequence< rendering::ARGBColor > aRes(nLen/4);
                    rendering::ARGBColor* pOut( aRes.getArray() );
                    for (sal_Int32 i = 0; i < nLen; i += 4)
                    {
                        *pOut++ = rendering::ARGBColor(deviceColor[i+3], deviceColor[i], deviceColor[i+1], deviceColor[i+2]);
                    }
                    return aRes;
                }
                virtual cpo::uno::Sequence< rendering::ARGBColor > convertToPARGB( const cpo::uno::Sequence< double >& deviceColor ) override
                {
                    const sal_Int32 nLen(deviceColor.getLength());
                    ENSURE_ARG_OR_THROW2(nLen%4==0,
                                         "number of channels no multiple of 4",
                                         static_cast<rendering::XColorSpace*>(this), 0);

                    cpo::uno::Sequence< rendering::ARGBColor > aRes(nLen/4);
                    rendering::ARGBColor* pOut( aRes.getArray() );
                    for (sal_Int32 i = 0; i < nLen; i += 4)
                    {
                        *pOut++ = rendering::ARGBColor(deviceColor[i+3],
                                                       deviceColor[i+3] * deviceColor[i],
                                                       deviceColor[i+3] * deviceColor[i+1],
                                                       deviceColor[i+3] * deviceColor[i+2]);
                    }
                    return aRes;
                }
                virtual cpo::uno::Sequence< double > convertFromRGB( const cpo::uno::Sequence< rendering::RGBColor >& rgbColor ) override
                {
                    cpo::uno::Sequence<double> aRes(rgbColor.getLength() * 4);
                    double* pColors=aRes.getArray();
                    for (auto& color : rgbColor)
                    {
                        *pColors++ = color.Red;
                        *pColors++ = color.Green;
                        *pColors++ = color.Blue;
                        *pColors++ = 1.0;
                    }
                    return aRes;
                }
                virtual cpo::uno::Sequence< double > convertFromARGB( const cpo::uno::Sequence< rendering::ARGBColor >& rgbColor ) override
                {
                    cpo::uno::Sequence<double> aRes(rgbColor.getLength() * 4);
                    double* pColors=aRes.getArray();
                    for (auto& color : rgbColor)
                    {
                        *pColors++ = color.Red;
                        *pColors++ = color.Green;
                        *pColors++ = color.Blue;
                        *pColors++ = color.Alpha;
                    }
                    return aRes;
                }
                virtual cpo::uno::Sequence< double > convertFromPARGB( const cpo::uno::Sequence< rendering::ARGBColor >& rgbColor ) override
                {
                    cpo::uno::Sequence<double> aRes(rgbColor.getLength() * 4);
                    double* pColors=aRes.getArray();
                    for (auto& color : rgbColor)
                    {
                        *pColors++ = color.Red / color.Alpha;
                        *pColors++ = color.Green / color.Alpha;
                        *pColors++ = color.Blue / color.Alpha;
                        *pColors++ = color.Alpha;
                    }
                    return aRes;
                }

                // XIntegerBitmapColorSpace
                virtual ::sal_Int32 getBitsPerPixel(  ) override
                {
                    return 32;
                }
                virtual cpo::uno::Sequence< ::sal_Int32 > getComponentBitCounts(  ) override
                {
                    return maBitCounts;
                }
                virtual ::sal_Int8 getEndianness(  ) override
                {
                    return util::Endianness::LITTLE;
                }
                virtual cpo::uno::Sequence<double> convertFromIntegerColorSpace( const cpo::uno::Sequence< ::sal_Int8 >& deviceColor,
                                                                                     const uno::Reference< rendering::XColorSpace >& targetColorSpace ) override
                {
                    if( dynamic_cast<StandardColorSpace*>(targetColorSpace.get()) )
                    {
                        const sal_Int32 nLen(deviceColor.getLength());
                        ENSURE_ARG_OR_THROW2(nLen%4==0,
                                             "number of channels no multiple of 4",
                                             static_cast<rendering::XColorSpace*>(this), 0);

                        cpo::uno::Sequence<double> aRes(nLen);
                        std::transform(deviceColor.begin(), deviceColor.end(), aRes.getArray(),
                                       [](auto c) { return vcl::unotools::toDoubleColor(c); });
                        return aRes;
                    }
                    else
                    {
                        // TODO(P3): if we know anything about target
                        // colorspace, this can be greatly sped up
                        cpo::uno::Sequence<rendering::ARGBColor> aIntermediate(
                            convertIntegerToARGB(deviceColor));
                        return targetColorSpace->convertFromARGB(aIntermediate);
                    }
                }
                virtual cpo::uno::Sequence< ::sal_Int8 > convertToIntegerColorSpace( const cpo::uno::Sequence< ::sal_Int8 >& deviceColor,
                                                                                         const uno::Reference< rendering::XIntegerBitmapColorSpace >& targetColorSpace ) override
                {
                    if( dynamic_cast<StandardColorSpace*>(targetColorSpace.get()) )
                    {
                        // it's us, so simply pass-through the data
                        return deviceColor;
                    }
                    else
                    {
                        // TODO(P3): if we know anything about target
                        // colorspace, this can be greatly sped up
                        cpo::uno::Sequence<rendering::ARGBColor> aIntermediate(
                            convertIntegerToARGB(deviceColor));
                        return targetColorSpace->convertIntegerFromARGB(aIntermediate);
                    }
                }
                virtual cpo::uno::Sequence< rendering::RGBColor > convertIntegerToRGB( const cpo::uno::Sequence< ::sal_Int8 >& deviceColor ) override
                {
                    const sal_Int32 nLen(deviceColor.getLength());
                    ENSURE_ARG_OR_THROW2(nLen%4==0,
                                         "number of channels no multiple of 4",
                                         static_cast<rendering::XColorSpace*>(this), 0);

                    cpo::uno::Sequence< rendering::RGBColor > aRes(nLen/4);
                    rendering::RGBColor* pOut( aRes.getArray() );
                    for (sal_Int32 i = 0; i < nLen; i += 4)
                    {
                        *pOut++ = rendering::RGBColor(
                            vcl::unotools::toDoubleColor(deviceColor[i]),
                            vcl::unotools::toDoubleColor(deviceColor[i+1]),
                            vcl::unotools::toDoubleColor(deviceColor[i+2]));
                    }
                    return aRes;
                }

                virtual cpo::uno::Sequence< rendering::ARGBColor > convertIntegerToARGB( const cpo::uno::Sequence< ::sal_Int8 >& deviceColor ) override
                {
                    const sal_Int32 nLen(deviceColor.getLength());
                    ENSURE_ARG_OR_THROW2(nLen%4==0,
                                         "number of channels no multiple of 4",
                                         static_cast<rendering::XColorSpace*>(this), 0);

                    cpo::uno::Sequence< rendering::ARGBColor > aRes(nLen/4);
                    rendering::ARGBColor* pOut( aRes.getArray() );
                    for (sal_Int32 i = 0; i < nLen; i += 4)
                    {
                        *pOut++ = rendering::ARGBColor(
                            vcl::unotools::toDoubleColor(deviceColor[i+3]),
                            vcl::unotools::toDoubleColor(deviceColor[i]),
                            vcl::unotools::toDoubleColor(deviceColor[i+1]),
                            vcl::unotools::toDoubleColor(deviceColor[i+2]));
                    }
                    return aRes;
                }

                virtual cpo::uno::Sequence< rendering::ARGBColor > convertIntegerToPARGB( const cpo::uno::Sequence< ::sal_Int8 >& deviceColor ) override
                {
                    const sal_Int32 nLen(deviceColor.getLength());
                    ENSURE_ARG_OR_THROW2(nLen%4==0,
                                         "number of channels no multiple of 4",
                                         static_cast<rendering::XColorSpace*>(this), 0);

                    cpo::uno::Sequence< rendering::ARGBColor > aRes(nLen/4);
                    rendering::ARGBColor* pOut( aRes.getArray() );
                    for (sal_Int32 i = 0; i < nLen; i += 4)
                    {
                        const sal_Int8 nAlpha(deviceColor[i+3]);
                        *pOut++ = rendering::ARGBColor(
                            vcl::unotools::toDoubleColor(nAlpha),
                            vcl::unotools::toDoubleColor(nAlpha * deviceColor[i]),
                            vcl::unotools::toDoubleColor(nAlpha * deviceColor[i+1]),
                            vcl::unotools::toDoubleColor(nAlpha * deviceColor[i+2]));
                    }
                    return aRes;
                }

                virtual cpo::uno::Sequence< ::sal_Int8 > convertIntegerFromRGB( const cpo::uno::Sequence< rendering::RGBColor >& rgbColor ) override
                {
                    cpo::uno::Sequence<sal_Int8> aRes(rgbColor.getLength() * 4);
                    sal_Int8* pColors=aRes.getArray();
                    for (auto& color : rgbColor)
                    {
                        *pColors++ = vcl::unotools::toByteColor(color.Red);
                        *pColors++ = vcl::unotools::toByteColor(color.Green);
                        *pColors++ = vcl::unotools::toByteColor(color.Blue);
                        *pColors++ = 0;
                    }
                    return aRes;
                }

                virtual cpo::uno::Sequence< ::sal_Int8 > convertIntegerFromARGB( const cpo::uno::Sequence< rendering::ARGBColor >& rgbColor ) override
                {
                    cpo::uno::Sequence<sal_Int8> aRes(rgbColor.getLength() * 4);
                    sal_Int8* pColors=aRes.getArray();
                    for (auto& color : rgbColor)
                    {
                        *pColors++ = vcl::unotools::toByteColor(color.Red);
                        *pColors++ = vcl::unotools::toByteColor(color.Green);
                        *pColors++ = vcl::unotools::toByteColor(color.Blue);
                        *pColors++ = vcl::unotools::toByteColor(color.Alpha);
                    }
                    return aRes;
                }

                virtual cpo::uno::Sequence< ::sal_Int8 > convertIntegerFromPARGB( const cpo::uno::Sequence< rendering::ARGBColor >& rgbColor ) override
                {
                    cpo::uno::Sequence<sal_Int8> aRes(rgbColor.getLength() * 4);
                    sal_Int8* pColors=aRes.getArray();
                    for (auto& color : rgbColor)
                    {
                        *pColors++ = vcl::unotools::toByteColor(color.Red / color.Alpha);
                        *pColors++ = vcl::unotools::toByteColor(color.Green / color.Alpha);
                        *pColors++ = vcl::unotools::toByteColor(color.Blue / color.Alpha);
                        *pColors++ = vcl::unotools::toByteColor(color.Alpha);
                    }
                    return aRes;
                }

            public:
                StandardColorSpace() :
                    maComponentTags(4),
                    maBitCounts(4)
                {
                    sal_Int8*  pTags = maComponentTags.getArray();
                    sal_Int32* pBitCounts = maBitCounts.getArray();
                    pTags[0] = rendering::ColorComponentTag::RGB_RED;
                    pTags[1] = rendering::ColorComponentTag::RGB_GREEN;
                    pTags[2] = rendering::ColorComponentTag::RGB_BLUE;
                    pTags[3] = rendering::ColorComponentTag::ALPHA;

                    pBitCounts[0] =
                    pBitCounts[1] =
                    pBitCounts[2] =
                    pBitCounts[3] = 8;
                }
            };

            class StandardNoAlphaColorSpace : public cppu::WeakImplHelper< css::rendering::XIntegerBitmapColorSpace >
            {
            private:
                cpo::uno::Sequence< sal_Int8 >  maComponentTags;
                cpo::uno::Sequence< sal_Int32 > maBitCounts;

                virtual ::sal_Int8 getType(  ) override
                {
                    return rendering::ColorSpaceType::RGB;
                }
                virtual cpo::uno::Sequence< ::sal_Int8 > getComponentTags(  ) override
                {
                    return maComponentTags;
                }
                virtual ::sal_Int8 getRenderingIntent(  ) override
                {
                    return rendering::RenderingIntent::PERCEPTUAL;
                }
                virtual cpo::uno::Sequence< beans::PropertyValue > getProperties(  ) override
                {
                    return cpo::uno::Sequence< beans::PropertyValue >();
                }
                virtual cpo::uno::Sequence< double > convertColorSpace( const cpo::uno::Sequence< double >& deviceColor,
                                                                            const uno::Reference< rendering::XColorSpace >& targetColorSpace ) override
                {
                    // TODO(P3): if we know anything about target
                    // colorspace, this can be greatly sped up
                    cpo::uno::Sequence<rendering::ARGBColor> aIntermediate(
                        convertToARGB(deviceColor));
                    return targetColorSpace->convertFromARGB(aIntermediate);
                }
                virtual cpo::uno::Sequence< rendering::RGBColor > convertToRGB( const cpo::uno::Sequence< double >& deviceColor ) override
                {
                    const sal_Int32 nLen(deviceColor.getLength());
                    ENSURE_ARG_OR_THROW2(nLen%4==0,
                                         "number of channels no multiple of 4",
                                         static_cast<rendering::XColorSpace*>(this), 0);

                    cpo::uno::Sequence< rendering::RGBColor > aRes(nLen/4);
                    rendering::RGBColor* pOut( aRes.getArray() );
                    for (sal_Int32 i = 0; i < nLen; i += 4)
                    {
                        *pOut++ = rendering::RGBColor(deviceColor[i], deviceColor[i+1], deviceColor[i+2]);
                    }
                    return aRes;
                }
                virtual cpo::uno::Sequence< rendering::ARGBColor > convertToARGB( const cpo::uno::Sequence< double >& deviceColor ) override
                {
                    const sal_Int32 nLen(deviceColor.getLength());
                    ENSURE_ARG_OR_THROW2(nLen%4==0,
                                         "number of channels no multiple of 4",
                                         static_cast<rendering::XColorSpace*>(this), 0);

                    cpo::uno::Sequence< rendering::ARGBColor > aRes(nLen/4);
                    rendering::ARGBColor* pOut( aRes.getArray() );
                    for (sal_Int32 i = 0; i < nLen; i += 4)
                    {
                        *pOut++ = rendering::ARGBColor(1.0, deviceColor[i], deviceColor[i+1], deviceColor[i+2]);
                    }
                    return aRes;
                }
                virtual cpo::uno::Sequence< rendering::ARGBColor > convertToPARGB( const cpo::uno::Sequence< double >& deviceColor ) override
                {
                    const sal_Int32 nLen(deviceColor.getLength());
                    ENSURE_ARG_OR_THROW2(nLen%4==0,
                                         "number of channels no multiple of 4",
                                         static_cast<rendering::XColorSpace*>(this), 0);

                    cpo::uno::Sequence< rendering::ARGBColor > aRes(nLen/4);
                    rendering::ARGBColor* pOut( aRes.getArray() );
                    for (sal_Int32 i = 0; i < nLen; i += 4)
                    {
                        *pOut++ = rendering::ARGBColor(1.0, deviceColor[i], deviceColor[i+1], deviceColor[i+2]);
                    }
                    return aRes;
                }
                virtual cpo::uno::Sequence< double > convertFromRGB( const cpo::uno::Sequence< rendering::RGBColor >& rgbColor ) override
                {
                    cpo::uno::Sequence<double> aRes(rgbColor.getLength() * 4);
                    double* pColors=aRes.getArray();
                    for (auto& color : rgbColor)
                    {
                        *pColors++ = color.Red;
                        *pColors++ = color.Green;
                        *pColors++ = color.Blue;
                        *pColors++ = 1.0; // the value does not matter
                    }
                    return aRes;
                }
                virtual cpo::uno::Sequence< double > convertFromARGB( const cpo::uno::Sequence< rendering::ARGBColor >& rgbColor ) override
                {
                    cpo::uno::Sequence<double> aRes(rgbColor.getLength() * 4);
                    double* pColors=aRes.getArray();
                    for (auto& color : rgbColor)
                    {
                        *pColors++ = color.Red;
                        *pColors++ = color.Green;
                        *pColors++ = color.Blue;
                        *pColors++ = 1.0; // the value does not matter
                    }
                    return aRes;
                }
                virtual cpo::uno::Sequence< double > convertFromPARGB( const cpo::uno::Sequence< rendering::ARGBColor >& rgbColor ) override
                {
                    cpo::uno::Sequence<double> aRes(rgbColor.getLength() * 4);
                    double* pColors=aRes.getArray();
                    for (auto& color : rgbColor)
                    {
                        *pColors++ = color.Red / color.Alpha;
                        *pColors++ = color.Green / color.Alpha;
                        *pColors++ = color.Blue / color.Alpha;
                        *pColors++ = 1.0; // the value does not matter
                    }
                    return aRes;
                }

                // XIntegerBitmapColorSpace
                virtual ::sal_Int32 getBitsPerPixel(  ) override
                {
                    return 32;
                }
                virtual cpo::uno::Sequence< ::sal_Int32 > getComponentBitCounts(  ) override
                {
                    return maBitCounts;
                }
                virtual ::sal_Int8 getEndianness(  ) override
                {
                    return util::Endianness::LITTLE;
                }
                virtual cpo::uno::Sequence<double> convertFromIntegerColorSpace( const cpo::uno::Sequence< ::sal_Int8 >& deviceColor,
                                                                                     const uno::Reference< rendering::XColorSpace >& targetColorSpace ) override
                {
                    if( dynamic_cast<StandardNoAlphaColorSpace*>(targetColorSpace.get()) )
                    {
                        const sal_Int32 nLen(deviceColor.getLength());
                        ENSURE_ARG_OR_THROW2(nLen%4==0,
                                             "number of channels no multiple of 4",
                                             static_cast<rendering::XColorSpace*>(this), 0);

                        cpo::uno::Sequence<double> aRes(nLen);
                        double* pOut( aRes.getArray() );
                        for (sal_Int32 i = 0; i < nLen; i += 4)
                        {
                            *pOut++ = vcl::unotools::toDoubleColor(deviceColor[i]);
                            *pOut++ = vcl::unotools::toDoubleColor(deviceColor[i+1]);
                            *pOut++ = vcl::unotools::toDoubleColor(deviceColor[i+2]);
                            *pOut++ = 1.0;
                        }
                        return aRes;
                    }
                    else
                    {
                        // TODO(P3): if we know anything about target
                        // colorspace, this can be greatly sped up
                        cpo::uno::Sequence<rendering::ARGBColor> aIntermediate(
                            convertIntegerToARGB(deviceColor));
                        return targetColorSpace->convertFromARGB(aIntermediate);
                    }
                }
                virtual cpo::uno::Sequence< ::sal_Int8 > convertToIntegerColorSpace( const cpo::uno::Sequence< ::sal_Int8 >& deviceColor,
                                                                                         const uno::Reference< rendering::XIntegerBitmapColorSpace >& targetColorSpace ) override
                {
                    if( dynamic_cast<StandardNoAlphaColorSpace*>(targetColorSpace.get()) )
                    {
                        // it's us, so simply pass-through the data
                        return deviceColor;
                    }
                    else
                    {
                        // TODO(P3): if we know anything about target
                        // colorspace, this can be greatly sped up
                        cpo::uno::Sequence<rendering::ARGBColor> aIntermediate(
                            convertIntegerToARGB(deviceColor));
                        return targetColorSpace->convertIntegerFromARGB(aIntermediate);
                    }
                }
                virtual cpo::uno::Sequence< rendering::RGBColor > convertIntegerToRGB( const cpo::uno::Sequence< ::sal_Int8 >& deviceColor ) override
                {
                    const sal_Int32 nLen(deviceColor.getLength());
                    ENSURE_ARG_OR_THROW2(nLen%4==0,
                                         "number of channels no multiple of 4",
                                         static_cast<rendering::XColorSpace*>(this), 0);

                    cpo::uno::Sequence< rendering::RGBColor > aRes(nLen/4);
                    rendering::RGBColor* pOut( aRes.getArray() );
                    for (sal_Int32 i = 0; i < nLen; i += 4)
                    {
                        *pOut++ = rendering::RGBColor(
                            vcl::unotools::toDoubleColor(deviceColor[i]),
                            vcl::unotools::toDoubleColor(deviceColor[i+1]),
                            vcl::unotools::toDoubleColor(deviceColor[i+2]));
                    }
                    return aRes;
                }

                virtual cpo::uno::Sequence< rendering::ARGBColor > convertIntegerToARGB( const cpo::uno::Sequence< ::sal_Int8 >& deviceColor ) override
                {
                    const sal_Int32 nLen(deviceColor.getLength());
                    ENSURE_ARG_OR_THROW2(nLen%4==0,
                                         "number of channels no multiple of 4",
                                         static_cast<rendering::XColorSpace*>(this), 0);

                    cpo::uno::Sequence< rendering::ARGBColor > aRes(nLen/4);
                    rendering::ARGBColor* pOut( aRes.getArray() );
                    for (sal_Int32 i = 0; i < nLen; i += 4)
                    {
                        *pOut++ = rendering::ARGBColor(
                            1.0,
                            vcl::unotools::toDoubleColor(deviceColor[i]),
                            vcl::unotools::toDoubleColor(deviceColor[i+1]),
                            vcl::unotools::toDoubleColor(deviceColor[i+2]));
                    }
                    return aRes;
                }

                virtual cpo::uno::Sequence< rendering::ARGBColor > convertIntegerToPARGB( const cpo::uno::Sequence< ::sal_Int8 >& deviceColor ) override
                {
                    const sal_Int32 nLen(deviceColor.getLength());
                    ENSURE_ARG_OR_THROW2(nLen%4==0,
                                         "number of channels no multiple of 4",
                                         static_cast<rendering::XColorSpace*>(this), 0);

                    cpo::uno::Sequence< rendering::ARGBColor > aRes(nLen/4);
                    rendering::ARGBColor* pOut( aRes.getArray() );
                    for (sal_Int32 i = 0; i < nLen; i += 4)
                    {
                        *pOut++ = rendering::ARGBColor(
                            1.0,
                            vcl::unotools::toDoubleColor(deviceColor[i]),
                            vcl::unotools::toDoubleColor(deviceColor[i+1]),
                            vcl::unotools::toDoubleColor(deviceColor[i+2]));
                    }
                    return aRes;
                }

                virtual cpo::uno::Sequence< ::sal_Int8 > convertIntegerFromRGB( const cpo::uno::Sequence< rendering::RGBColor >& rgbColor ) override
                {
                    cpo::uno::Sequence<sal_Int8> aRes(rgbColor.getLength() * 4);
                    sal_Int8* pColors=aRes.getArray();
                    for (auto& color : rgbColor)
                    {
                        *pColors++ = vcl::unotools::toByteColor(color.Red);
                        *pColors++ = vcl::unotools::toByteColor(color.Green);
                        *pColors++ = vcl::unotools::toByteColor(color.Blue);
                        *pColors++ = 1.0;
                    }
                    return aRes;
                }

                virtual cpo::uno::Sequence< ::sal_Int8 > convertIntegerFromARGB( const cpo::uno::Sequence< rendering::ARGBColor >& rgbColor ) override
                {
                    cpo::uno::Sequence<sal_Int8> aRes(rgbColor.getLength() * 4);
                    sal_Int8* pColors=aRes.getArray();
                    for (auto& color : rgbColor)
                    {
                        *pColors++ = vcl::unotools::toByteColor(color.Red);
                        *pColors++ = vcl::unotools::toByteColor(color.Green);
                        *pColors++ = vcl::unotools::toByteColor(color.Blue);
                        *pColors++ = -1;
                    }
                    return aRes;
                }

                virtual cpo::uno::Sequence< ::sal_Int8 > convertIntegerFromPARGB( const cpo::uno::Sequence< rendering::ARGBColor >& rgbColor ) override
                {
                    cpo::uno::Sequence<sal_Int8> aRes(rgbColor.getLength() * 4);
                    sal_Int8* pColors=aRes.getArray();
                    for (auto& color : rgbColor)
                    {
                        *pColors++ = vcl::unotools::toByteColor(color.Red / color.Alpha);
                        *pColors++ = vcl::unotools::toByteColor(color.Green / color.Alpha);
                        *pColors++ = vcl::unotools::toByteColor(color.Blue / color.Alpha);
                        *pColors++ = -1;
                    }
                    return aRes;
                }

            public:
                StandardNoAlphaColorSpace() :
                    maComponentTags(3),
                    maBitCounts(3)
                {
                    sal_Int8*  pTags = maComponentTags.getArray();
                    sal_Int32* pBitCounts = maBitCounts.getArray();
                    pTags[0] = rendering::ColorComponentTag::RGB_RED;
                    pTags[1] = rendering::ColorComponentTag::RGB_GREEN;
                    pTags[2] = rendering::ColorComponentTag::RGB_BLUE;

                    pBitCounts[0] =
                    pBitCounts[1] =
                    pBitCounts[2] = 8;
                }
            };

        }

        uno::Reference<rendering::XIntegerBitmapColorSpace> const & getStdColorSpace()
        {
            static uno::Reference<rendering::XIntegerBitmapColorSpace> SPACE = new StandardColorSpace();
            return SPACE;
        }

        uno::Reference<rendering::XIntegerBitmapColorSpace> const & getStdColorSpaceWithoutAlpha()
        {
            static uno::Reference<rendering::XIntegerBitmapColorSpace> SPACE = new StandardNoAlphaColorSpace();
            return SPACE;
        }

        rendering::IntegerBitmapLayout getStdMemoryLayout( const geometry::IntegerSize2D& rBmpSize )
        {
            rendering::IntegerBitmapLayout aLayout;

            aLayout.ScanLines = rBmpSize.Height;
            aLayout.ScanLineBytes = rBmpSize.Width*4;
            aLayout.ScanLineStride = aLayout.ScanLineBytes;
            aLayout.PlaneStride = 0;
            aLayout.ColorSpace = getStdColorSpace();
            aLayout.Palette.clear();
            aLayout.IsMsbFirst = false;

            return aLayout;
        }

        cpo::uno::Sequence<sal_Int8> colorToStdIntSequence( const ::Color& rColor )
        {
            cpo::uno::Sequence<sal_Int8> aRet(4);
            sal_Int8* pCols( aRet.getArray() );
#ifdef OSL_BIGENDIAN
            pCols[0] = rColor.GetRed();
            pCols[1] = rColor.GetGreen();
            pCols[2] = rColor.GetBlue();
            pCols[3] = rColor.GetAlpha();
#else
            *reinterpret_cast<sal_Int32*>(pCols) = sal_Int32(rColor);
#endif
            return aRet;
        }

        // Create a corrected view transformation out of the give one,
        // which ensures that the rectangle given by (0,0) and
        // rSpriteSize is mapped with its left,top corner to (0,0)
        // again. This is required to properly render sprite
        // animations to buffer bitmaps.
        ::basegfx::B2DHomMatrix calcRectToOriginTransform( const ::basegfx::B2DRange&          i_srcRect,
                                                           const ::basegfx::B2DHomMatrix&      i_transformation )
        {
            ::basegfx::B2DHomMatrix o_transform;
            if( i_srcRect.isEmpty() )
            {
                o_transform = i_transformation;
                return o_transform;
            }

            // transform by given transformation
            ::basegfx::B2DRectangle aTransformedRect = calcTransformedRectBounds(
                                       i_srcRect,
                                       i_transformation );

            // now move resulting left,top point of bounds to (0,0)
            const basegfx::B2DHomMatrix aCorrectedTransform(basegfx::utils::createTranslateB2DHomMatrix(
                -aTransformedRect.getMinX(), -aTransformedRect.getMinY()));

            // prepend to original transformation
            o_transform = aCorrectedTransform * i_transformation;

            return o_transform;
        }

        ::basegfx::B2DRange calcTransformedRectBounds( const ::basegfx::B2DRange&      inRect,
                                                        const ::basegfx::B2DHomMatrix&  transformation )
        {
            ::basegfx::B2DRange outRect;

            if( inRect.isEmpty() )
                return outRect;

            // transform all four extremal points of the rectangle,
            // take bounding rect of those.

            // transform left-top point
            outRect.expand( transformation * inRect.getMinimum() );

            // transform bottom-right point
            outRect.expand( transformation * inRect.getMaximum() );

            ::basegfx::B2DPoint aPoint;

            // transform top-right point
            aPoint.setX( inRect.getMaxX() );
            aPoint.setY( inRect.getMinY() );

            aPoint *= transformation;
            outRect.expand( aPoint );

            // transform bottom-left point
            aPoint.setX( inRect.getMinX() );
            aPoint.setY( inRect.getMaxY() );

            aPoint *= transformation;
            outRect.expand( aPoint );

            // over and out.
            return outRect;
        }

        int calcGradientStepCount( ::basegfx::B2DHomMatrix&      rTotalTransform,
                                   const rendering::ViewState&   viewState,
                                   const rendering::RenderState& renderState,
                                   const rendering::Texture&     texture,
                                   int                           nColorSteps )
        {
            // calculate overall texture transformation (directly from
            // texture to device space).
            ::basegfx::B2DHomMatrix aMatrix;

            rTotalTransform.identity();
            ::basegfx::unotools::homMatrixFromAffineMatrix( rTotalTransform,
                                                            texture.AffineTransform );
            mergeViewAndRenderTransform(aMatrix, viewState, renderState);
            rTotalTransform *= aMatrix; // prepend total view/render transformation

            // determine size of gradient in device coordinate system
            // (to e.g. determine sensible number of gradient steps)
            ::basegfx::B2DPoint aLeftTop( 0.0, 0.0 );
            ::basegfx::B2DPoint aLeftBottom( 0.0, 1.0 );
            ::basegfx::B2DPoint aRightTop( 1.0, 0.0 );
            ::basegfx::B2DPoint aRightBottom( 1.0, 1.0 );

            aLeftTop    *= rTotalTransform;
            aLeftBottom *= rTotalTransform;
            aRightTop   *= rTotalTransform;
            aRightBottom*= rTotalTransform;

            // longest line in gradient bound rect
            const int nGradientSize(
                static_cast<int>(
                    std::max(
                        ::basegfx::B2DVector(aRightBottom-aLeftTop).getLength(),
                        ::basegfx::B2DVector(aRightTop-aLeftBottom).getLength() ) + 1.0 ) );

            // typical number for pixel of the same color (strip size)
            const int nStripSize( nGradientSize < 50 ? 2 : 4 );

            // use at least three steps, and at utmost the number of color
            // steps
            return std::max( 3,
                               std::min(
                                   nGradientSize / nStripSize,
                                   nColorSteps ) );
        }

        void clipOutDev(const rendering::ViewState& viewState,
                        const rendering::RenderState& renderState,
                        OutputDevice& rOutDev)
        {
            // accumulate non-empty clips into one region
            vcl::Region aClipRegion(true);

            if( renderState.Clip.is() )
            {
                ::basegfx::B2DPolyPolygon aClipPoly(
                    ::basegfx::unotools::b2DPolyPolygonFromXPolyPolygon2D(renderState.Clip) );

                ::basegfx::B2DHomMatrix aMatrix;
                aClipPoly.transform(mergeViewAndRenderTransform(aMatrix, viewState, renderState));

                if( aClipPoly.count() )
                {
                    // setup non-empty clipping
                    vcl::Region aRegion = vcl::Region::GetRegionFromPolyPolygon( ::tools::PolyPolygon( aClipPoly ) );
                    aClipRegion.Intersect( aRegion );
                }
                else
                {
                    // clip polygon is empty
                    aClipRegion.SetEmpty();
                }
            }

            // setup accumulated clip region. Note that setting an
            // empty clip region denotes "clip everything" on the
            // OutputDevice (which is why we translate that into
            // SetClipRegion() here). When both view and render clip
            // are empty, aClipRegion remains default-constructed,
            // i.e. empty, too.
            if( aClipRegion.IsNull() )
                rOutDev.SetClipRegion();
            else
                rOutDev.SetClipRegion( aClipRegion );
        }

        void extractExtraFontProperties(const cpo::uno::Sequence<beans::PropertyValue>& rExtraFontProperties,
                        sal_uInt32 &rEmphasisMark)
        {
            for(const beans::PropertyValue& rPropVal : rExtraFontProperties)
            {
                if (rPropVal.Name == "EmphasisMark")
                    rPropVal.Value >>= rEmphasisMark;
            }
        }

} // namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
