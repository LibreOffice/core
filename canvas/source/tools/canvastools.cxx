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

#include <limits>

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <basegfx/numeric/ftools.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/point/b2ipoint.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/range/b2drange.hxx>
#include <basegfx/range/b2drectangle.hxx>
#include <basegfx/range/b2irange.hxx>
#include <basegfx/tools/canvastools.hxx>
#include <basegfx/vector/b2ivector.hxx>
#include <com/sun/star/awt/Rectangle.hpp>
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
#include <rtl/instance.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <tools/diagnose_ex.h>
#include <vcl/canvastools.hxx>
#include <vcl/window.hxx>

#include <canvas/canvastools.hxx>


using namespace ::com::sun::star;

namespace canvas
{
    namespace tools
    {
        geometry::RealSize2D createInfiniteSize2D()
        {
            return geometry::RealSize2D(
                ::std::numeric_limits<double>::infinity(),
                ::std::numeric_limits<double>::infinity() );
        }

        rendering::RenderState& initRenderState( rendering::RenderState& renderState )
        {
            // setup identity transform
            setIdentityAffineMatrix2D( renderState.AffineTransform );
            renderState.Clip.clear();
            renderState.DeviceColor = uno::Sequence< double >();
            renderState.CompositeOperation = rendering::CompositeOperation::OVER;

            return renderState;
        }

        rendering::ViewState& initViewState( rendering::ViewState& viewState )
        {
            // setup identity transform
            setIdentityAffineMatrix2D( viewState.AffineTransform );
            viewState.Clip.clear();

            return viewState;
        }

        ::basegfx::B2DHomMatrix& getViewStateTransform( ::basegfx::B2DHomMatrix&    transform,
                                                        const rendering::ViewState& viewState )
        {
            return ::basegfx::unotools::homMatrixFromAffineMatrix( transform, viewState.AffineTransform );
        }

        rendering::ViewState& setViewStateTransform( rendering::ViewState&          viewState,
                                                     const ::basegfx::B2DHomMatrix& transform )
        {
            ::basegfx::unotools::affineMatrixFromHomMatrix( viewState.AffineTransform, transform );

            return viewState;
        }

        ::basegfx::B2DHomMatrix& getRenderStateTransform( ::basegfx::B2DHomMatrix&      transform,
                                                          const rendering::RenderState& renderState )
        {
            return ::basegfx::unotools::homMatrixFromAffineMatrix( transform, renderState.AffineTransform );
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
            ::basegfx::B2DHomMatrix transform;

            getRenderStateTransform( transform, renderState );
            return setRenderStateTransform( renderState, transform * rTransform );
        }

        rendering::RenderState& prependToRenderState( rendering::RenderState&           renderState,
                                                      const ::basegfx::B2DHomMatrix&    rTransform )
        {
            ::basegfx::B2DHomMatrix transform;

            getRenderStateTransform( transform, renderState );
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
                uno::Sequence< sal_Int8 >  maComponentTags;
                uno::Sequence< sal_Int32 > maBitCounts;

                virtual ::sal_Int8 SAL_CALL getType(  ) override
                {
                    return rendering::ColorSpaceType::RGB;
                }
                virtual uno::Sequence< ::sal_Int8 > SAL_CALL getComponentTags(  ) override
                {
                    return maComponentTags;
                }
                virtual ::sal_Int8 SAL_CALL getRenderingIntent(  ) override
                {
                    return rendering::RenderingIntent::PERCEPTUAL;
                }
                virtual uno::Sequence< beans::PropertyValue > SAL_CALL getProperties(  ) override
                {
                    return uno::Sequence< beans::PropertyValue >();
                }
                virtual uno::Sequence< double > SAL_CALL convertColorSpace( const uno::Sequence< double >& deviceColor,
                                                                            const uno::Reference< rendering::XColorSpace >& targetColorSpace ) override
                {
                    // TODO(P3): if we know anything about target
                    // colorspace, this can be greatly sped up
                    uno::Sequence<rendering::ARGBColor> aIntermediate(
                        convertToARGB(deviceColor));
                    return targetColorSpace->convertFromARGB(aIntermediate);
                }
                virtual uno::Sequence< rendering::RGBColor > SAL_CALL convertToRGB( const uno::Sequence< double >& deviceColor ) override
                {
                    const double*  pIn( deviceColor.getConstArray() );
                    const std::size_t nLen( deviceColor.getLength() );
                    ENSURE_ARG_OR_THROW2(nLen%4==0,
                                         "number of channels no multiple of 4",
                                         static_cast<rendering::XColorSpace*>(this), 0);

                    uno::Sequence< rendering::RGBColor > aRes(nLen/4);
                    rendering::RGBColor* pOut( aRes.getArray() );
                    for( std::size_t i=0; i<nLen; i+=4 )
                    {
                        *pOut++ = rendering::RGBColor(pIn[0],pIn[1],pIn[2]);
                        pIn += 4;
                    }
                    return aRes;
                }
                virtual uno::Sequence< rendering::ARGBColor > SAL_CALL convertToARGB( const uno::Sequence< double >& deviceColor ) override
                {
                    SAL_WARN_IF(deviceColor.getLength() == 0, "canvas", "empty deviceColor argument");
                    const double*  pIn( deviceColor.getConstArray() );
                    const std::size_t nLen( deviceColor.getLength() );
                    ENSURE_ARG_OR_THROW2(nLen%4==0,
                                         "number of channels no multiple of 4",
                                         static_cast<rendering::XColorSpace*>(this), 0);

                    uno::Sequence< rendering::ARGBColor > aRes(nLen/4);
                    rendering::ARGBColor* pOut( aRes.getArray() );
                    for( std::size_t i=0; i<nLen; i+=4 )
                    {
                        *pOut++ = rendering::ARGBColor(pIn[3],pIn[0],pIn[1],pIn[2]);
                        pIn += 4;
                    }
                    return aRes;
                }
                virtual uno::Sequence< rendering::ARGBColor > SAL_CALL convertToPARGB( const uno::Sequence< double >& deviceColor ) override
                {
                    const double*  pIn( deviceColor.getConstArray() );
                    const std::size_t nLen( deviceColor.getLength() );
                    ENSURE_ARG_OR_THROW2(nLen%4==0,
                                         "number of channels no multiple of 4",
                                         static_cast<rendering::XColorSpace*>(this), 0);

                    uno::Sequence< rendering::ARGBColor > aRes(nLen/4);
                    rendering::ARGBColor* pOut( aRes.getArray() );
                    for( std::size_t i=0; i<nLen; i+=4 )
                    {
                        *pOut++ = rendering::ARGBColor(pIn[3],pIn[3]*pIn[0],pIn[3]*pIn[1],pIn[3]*pIn[2]);
                        pIn += 4;
                    }
                    return aRes;
                }
                virtual uno::Sequence< double > SAL_CALL convertFromRGB( const uno::Sequence< rendering::RGBColor >& rgbColor ) override
                {
                    const rendering::RGBColor* pIn( rgbColor.getConstArray() );
                    const std::size_t             nLen( rgbColor.getLength() );

                    uno::Sequence< double > aRes(nLen*4);
                    double* pColors=aRes.getArray();
                    for( std::size_t i=0; i<nLen; ++i )
                    {
                        *pColors++ = pIn->Red;
                        *pColors++ = pIn->Green;
                        *pColors++ = pIn->Blue;
                        *pColors++ = 1.0;
                        ++pIn;
                    }
                    return aRes;
                }
                virtual uno::Sequence< double > SAL_CALL convertFromARGB( const uno::Sequence< rendering::ARGBColor >& rgbColor ) override
                {
                    const rendering::ARGBColor* pIn( rgbColor.getConstArray() );
                    const std::size_t              nLen( rgbColor.getLength() );

                    uno::Sequence< double > aRes(nLen*4);
                    double* pColors=aRes.getArray();
                    for( std::size_t i=0; i<nLen; ++i )
                    {
                        *pColors++ = pIn->Red;
                        *pColors++ = pIn->Green;
                        *pColors++ = pIn->Blue;
                        *pColors++ = pIn->Alpha;
                        ++pIn;
                    }
                    return aRes;
                }
                virtual uno::Sequence< double > SAL_CALL convertFromPARGB( const uno::Sequence< rendering::ARGBColor >& rgbColor ) override
                {
                    const rendering::ARGBColor* pIn( rgbColor.getConstArray() );
                    const std::size_t              nLen( rgbColor.getLength() );

                    uno::Sequence< double > aRes(nLen*4);
                    double* pColors=aRes.getArray();
                    for( std::size_t i=0; i<nLen; ++i )
                    {
                        *pColors++ = pIn->Red/pIn->Alpha;
                        *pColors++ = pIn->Green/pIn->Alpha;
                        *pColors++ = pIn->Blue/pIn->Alpha;
                        *pColors++ = pIn->Alpha;
                        ++pIn;
                    }
                    return aRes;
                }

                // XIntegerBitmapColorSpace
                virtual ::sal_Int32 SAL_CALL getBitsPerPixel(  ) override
                {
                    return 32;
                }
                virtual uno::Sequence< ::sal_Int32 > SAL_CALL getComponentBitCounts(  ) override
                {
                    return maBitCounts;
                }
                virtual ::sal_Int8 SAL_CALL getEndianness(  ) override
                {
                    return util::Endianness::LITTLE;
                }
                virtual uno::Sequence<double> SAL_CALL convertFromIntegerColorSpace( const uno::Sequence< ::sal_Int8 >& deviceColor,
                                                                                     const uno::Reference< rendering::XColorSpace >& targetColorSpace ) override
                {
                    if( dynamic_cast<StandardColorSpace*>(targetColorSpace.get()) )
                    {
                        const sal_Int8* pIn( deviceColor.getConstArray() );
                        const std::size_t  nLen( deviceColor.getLength() );
                        ENSURE_ARG_OR_THROW2(nLen%4==0,
                                             "number of channels no multiple of 4",
                                             static_cast<rendering::XColorSpace*>(this), 0);

                        uno::Sequence<double> aRes(nLen);
                        double* pOut( aRes.getArray() );
                        for( std::size_t i=0; i<nLen; i+=4 )
                        {
                            *pOut++ = vcl::unotools::toDoubleColor(*pIn++);
                            *pOut++ = vcl::unotools::toDoubleColor(*pIn++);
                            *pOut++ = vcl::unotools::toDoubleColor(*pIn++);
                            *pOut++ = vcl::unotools::toDoubleColor(*pIn++);
                        }
                        return aRes;
                    }
                    else
                    {
                        // TODO(P3): if we know anything about target
                        // colorspace, this can be greatly sped up
                        uno::Sequence<rendering::ARGBColor> aIntermediate(
                            convertIntegerToARGB(deviceColor));
                        return targetColorSpace->convertFromARGB(aIntermediate);
                    }
                }
                virtual uno::Sequence< ::sal_Int8 > SAL_CALL convertToIntegerColorSpace( const uno::Sequence< ::sal_Int8 >& deviceColor,
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
                        uno::Sequence<rendering::ARGBColor> aIntermediate(
                            convertIntegerToARGB(deviceColor));
                        return targetColorSpace->convertIntegerFromARGB(aIntermediate);
                    }
                }
                virtual uno::Sequence< rendering::RGBColor > SAL_CALL convertIntegerToRGB( const uno::Sequence< ::sal_Int8 >& deviceColor ) override
                {
                    const sal_Int8* pIn( deviceColor.getConstArray() );
                    const std::size_t  nLen( deviceColor.getLength() );
                    ENSURE_ARG_OR_THROW2(nLen%4==0,
                                         "number of channels no multiple of 4",
                                         static_cast<rendering::XColorSpace*>(this), 0);

                    uno::Sequence< rendering::RGBColor > aRes(nLen/4);
                    rendering::RGBColor* pOut( aRes.getArray() );
                    for( std::size_t i=0; i<nLen; i+=4 )
                    {
                        *pOut++ = rendering::RGBColor(
                            vcl::unotools::toDoubleColor(pIn[0]),
                            vcl::unotools::toDoubleColor(pIn[1]),
                            vcl::unotools::toDoubleColor(pIn[2]));
                        pIn += 4;
                    }
                    return aRes;
                }

                virtual uno::Sequence< rendering::ARGBColor > SAL_CALL convertIntegerToARGB( const uno::Sequence< ::sal_Int8 >& deviceColor ) override
                {
                    const sal_Int8* pIn( deviceColor.getConstArray() );
                    const std::size_t  nLen( deviceColor.getLength() );
                    ENSURE_ARG_OR_THROW2(nLen%4==0,
                                         "number of channels no multiple of 4",
                                         static_cast<rendering::XColorSpace*>(this), 0);

                    uno::Sequence< rendering::ARGBColor > aRes(nLen/4);
                    rendering::ARGBColor* pOut( aRes.getArray() );
                    for( std::size_t i=0; i<nLen; i+=4 )
                    {
                        *pOut++ = rendering::ARGBColor(
                            vcl::unotools::toDoubleColor(pIn[3]),
                            vcl::unotools::toDoubleColor(pIn[0]),
                            vcl::unotools::toDoubleColor(pIn[1]),
                            vcl::unotools::toDoubleColor(pIn[2]));
                        pIn += 4;
                    }
                    return aRes;
                }

                virtual uno::Sequence< rendering::ARGBColor > SAL_CALL convertIntegerToPARGB( const uno::Sequence< ::sal_Int8 >& deviceColor ) override
                {
                    const sal_Int8* pIn( deviceColor.getConstArray() );
                    const std::size_t  nLen( deviceColor.getLength() );
                    ENSURE_ARG_OR_THROW2(nLen%4==0,
                                         "number of channels no multiple of 4",
                                         static_cast<rendering::XColorSpace*>(this), 0);

                    uno::Sequence< rendering::ARGBColor > aRes(nLen/4);
                    rendering::ARGBColor* pOut( aRes.getArray() );
                    for( std::size_t i=0; i<nLen; i+=4 )
                    {
                        const sal_Int8 nAlpha( pIn[3] );
                        *pOut++ = rendering::ARGBColor(
                            vcl::unotools::toDoubleColor(nAlpha),
                            vcl::unotools::toDoubleColor(nAlpha*pIn[0]),
                            vcl::unotools::toDoubleColor(nAlpha*pIn[1]),
                            vcl::unotools::toDoubleColor(nAlpha*pIn[2]));
                        pIn += 4;
                    }
                    return aRes;
                }

                virtual uno::Sequence< ::sal_Int8 > SAL_CALL convertIntegerFromRGB( const uno::Sequence< rendering::RGBColor >& rgbColor ) override
                {
                    const rendering::RGBColor* pIn( rgbColor.getConstArray() );
                    const std::size_t             nLen( rgbColor.getLength() );

                    uno::Sequence< sal_Int8 > aRes(nLen*4);
                    sal_Int8* pColors=aRes.getArray();
                    for( std::size_t i=0; i<nLen; ++i )
                    {
                        *pColors++ = vcl::unotools::toByteColor(pIn->Red);
                        *pColors++ = vcl::unotools::toByteColor(pIn->Green);
                        *pColors++ = vcl::unotools::toByteColor(pIn->Blue);
                        *pColors++ = 0;
                        ++pIn;
                    }
                    return aRes;
                }

                virtual uno::Sequence< ::sal_Int8 > SAL_CALL convertIntegerFromARGB( const uno::Sequence< rendering::ARGBColor >& rgbColor ) override
                {
                    const rendering::ARGBColor* pIn( rgbColor.getConstArray() );
                    const std::size_t              nLen( rgbColor.getLength() );

                    uno::Sequence< sal_Int8 > aRes(nLen*4);
                    sal_Int8* pColors=aRes.getArray();
                    for( std::size_t i=0; i<nLen; ++i )
                    {
                        *pColors++ = vcl::unotools::toByteColor(pIn->Red);
                        *pColors++ = vcl::unotools::toByteColor(pIn->Green);
                        *pColors++ = vcl::unotools::toByteColor(pIn->Blue);
                        *pColors++ = vcl::unotools::toByteColor(pIn->Alpha);
                        ++pIn;
                    }
                    return aRes;
                }

                virtual uno::Sequence< ::sal_Int8 > SAL_CALL convertIntegerFromPARGB( const uno::Sequence< rendering::ARGBColor >& rgbColor ) override
                {
                    const rendering::ARGBColor* pIn( rgbColor.getConstArray() );
                    const std::size_t              nLen( rgbColor.getLength() );

                    uno::Sequence< sal_Int8 > aRes(nLen*4);
                    sal_Int8* pColors=aRes.getArray();
                    for( std::size_t i=0; i<nLen; ++i )
                    {
                        *pColors++ = vcl::unotools::toByteColor(pIn->Red/pIn->Alpha);
                        *pColors++ = vcl::unotools::toByteColor(pIn->Green/pIn->Alpha);
                        *pColors++ = vcl::unotools::toByteColor(pIn->Blue/pIn->Alpha);
                        *pColors++ = vcl::unotools::toByteColor(pIn->Alpha);
                        ++pIn;
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
                uno::Sequence< sal_Int8 >  maComponentTags;
                uno::Sequence< sal_Int32 > maBitCounts;

                virtual ::sal_Int8 SAL_CALL getType(  ) override
                {
                    return rendering::ColorSpaceType::RGB;
                }
                virtual uno::Sequence< ::sal_Int8 > SAL_CALL getComponentTags(  ) override
                {
                    return maComponentTags;
                }
                virtual ::sal_Int8 SAL_CALL getRenderingIntent(  ) override
                {
                    return rendering::RenderingIntent::PERCEPTUAL;
                }
                virtual uno::Sequence< beans::PropertyValue > SAL_CALL getProperties(  ) override
                {
                    return uno::Sequence< beans::PropertyValue >();
                }
                virtual uno::Sequence< double > SAL_CALL convertColorSpace( const uno::Sequence< double >& deviceColor,
                                                                            const uno::Reference< rendering::XColorSpace >& targetColorSpace ) override
                {
                    // TODO(P3): if we know anything about target
                    // colorspace, this can be greatly sped up
                    uno::Sequence<rendering::ARGBColor> aIntermediate(
                        convertToARGB(deviceColor));
                    return targetColorSpace->convertFromARGB(aIntermediate);
                }
                virtual uno::Sequence< rendering::RGBColor > SAL_CALL convertToRGB( const uno::Sequence< double >& deviceColor ) override
                {
                    const double*  pIn( deviceColor.getConstArray() );
                    const std::size_t nLen( deviceColor.getLength() );
                    ENSURE_ARG_OR_THROW2(nLen%4==0,
                                         "number of channels no multiple of 4",
                                         static_cast<rendering::XColorSpace*>(this), 0);

                    uno::Sequence< rendering::RGBColor > aRes(nLen/4);
                    rendering::RGBColor* pOut( aRes.getArray() );
                    for( std::size_t i=0; i<nLen; i+=4 )
                    {
                        *pOut++ = rendering::RGBColor(pIn[0],pIn[1],pIn[2]);
                        pIn += 4;
                    }
                    return aRes;
                }
                virtual uno::Sequence< rendering::ARGBColor > SAL_CALL convertToARGB( const uno::Sequence< double >& deviceColor ) override
                {
                    const double*  pIn( deviceColor.getConstArray() );
                    const std::size_t nLen( deviceColor.getLength() );
                    ENSURE_ARG_OR_THROW2(nLen%4==0,
                                         "number of channels no multiple of 4",
                                         static_cast<rendering::XColorSpace*>(this), 0);

                    uno::Sequence< rendering::ARGBColor > aRes(nLen/4);
                    rendering::ARGBColor* pOut( aRes.getArray() );
                    for( std::size_t i=0; i<nLen; i+=4 )
                    {
                        *pOut++ = rendering::ARGBColor(1.0,pIn[0],pIn[1],pIn[2]);
                        pIn += 4;
                    }
                    return aRes;
                }
                virtual uno::Sequence< rendering::ARGBColor > SAL_CALL convertToPARGB( const uno::Sequence< double >& deviceColor ) override
                {
                    const double*  pIn( deviceColor.getConstArray() );
                    const std::size_t nLen( deviceColor.getLength() );
                    ENSURE_ARG_OR_THROW2(nLen%4==0,
                                         "number of channels no multiple of 4",
                                         static_cast<rendering::XColorSpace*>(this), 0);

                    uno::Sequence< rendering::ARGBColor > aRes(nLen/4);
                    rendering::ARGBColor* pOut( aRes.getArray() );
                    for( std::size_t i=0; i<nLen; i+=4 )
                    {
                        *pOut++ = rendering::ARGBColor(1.0,pIn[0],pIn[1],pIn[2]);
                        pIn += 4;
                    }
                    return aRes;
                }
                virtual uno::Sequence< double > SAL_CALL convertFromRGB( const uno::Sequence< rendering::RGBColor >& rgbColor ) override
                {
                    const rendering::RGBColor* pIn( rgbColor.getConstArray() );
                    const std::size_t             nLen( rgbColor.getLength() );

                    uno::Sequence< double > aRes(nLen*4);
                    double* pColors=aRes.getArray();
                    for( std::size_t i=0; i<nLen; ++i )
                    {
                        *pColors++ = pIn->Red;
                        *pColors++ = pIn->Green;
                        *pColors++ = pIn->Blue;
                        *pColors++ = 1.0; // the value does not matter
                        ++pIn;
                    }
                    return aRes;
                }
                virtual uno::Sequence< double > SAL_CALL convertFromARGB( const uno::Sequence< rendering::ARGBColor >& rgbColor ) override
                {
                    const rendering::ARGBColor* pIn( rgbColor.getConstArray() );
                    const std::size_t              nLen( rgbColor.getLength() );

                    uno::Sequence< double > aRes(nLen*4);
                    double* pColors=aRes.getArray();
                    for( std::size_t i=0; i<nLen; ++i )
                    {
                        *pColors++ = pIn->Red;
                        *pColors++ = pIn->Green;
                        *pColors++ = pIn->Blue;
                        *pColors++ = 1.0; // the value does not matter
                        ++pIn;
                    }
                    return aRes;
                }
                virtual uno::Sequence< double > SAL_CALL convertFromPARGB( const uno::Sequence< rendering::ARGBColor >& rgbColor ) override
                {
                    const rendering::ARGBColor* pIn( rgbColor.getConstArray() );
                    const std::size_t              nLen( rgbColor.getLength() );

                    uno::Sequence< double > aRes(nLen*4);
                    double* pColors=aRes.getArray();
                    for( std::size_t i=0; i<nLen; ++i )
                    {
                        *pColors++ = pIn->Red/pIn->Alpha;
                        *pColors++ = pIn->Green/pIn->Alpha;
                        *pColors++ = pIn->Blue/pIn->Alpha;
                        *pColors++ = 1.0; // the value does not matter
                        ++pIn;
                    }
                    return aRes;
                }

                // XIntegerBitmapColorSpace
                virtual ::sal_Int32 SAL_CALL getBitsPerPixel(  ) override
                {
                    return 32;
                }
                virtual uno::Sequence< ::sal_Int32 > SAL_CALL getComponentBitCounts(  ) override
                {
                    return maBitCounts;
                }
                virtual ::sal_Int8 SAL_CALL getEndianness(  ) override
                {
                    return util::Endianness::LITTLE;
                }
                virtual uno::Sequence<double> SAL_CALL convertFromIntegerColorSpace( const uno::Sequence< ::sal_Int8 >& deviceColor,
                                                                                     const uno::Reference< rendering::XColorSpace >& targetColorSpace ) override
                {
                    if( dynamic_cast<StandardNoAlphaColorSpace*>(targetColorSpace.get()) )
                    {
                        const sal_Int8* pIn( deviceColor.getConstArray() );
                        const std::size_t  nLen( deviceColor.getLength() );
                        ENSURE_ARG_OR_THROW2(nLen%4==0,
                                             "number of channels no multiple of 4",
                                             static_cast<rendering::XColorSpace*>(this), 0);

                        uno::Sequence<double> aRes(nLen);
                        double* pOut( aRes.getArray() );
                        for( std::size_t i=0; i<nLen; i+=4 )
                        {
                            *pOut++ = vcl::unotools::toDoubleColor(*pIn++);
                            *pOut++ = vcl::unotools::toDoubleColor(*pIn++);
                            *pOut++ = vcl::unotools::toDoubleColor(*pIn++);
                            *pOut++ = 1.0;
                        }
                        return aRes;
                    }
                    else
                    {
                        // TODO(P3): if we know anything about target
                        // colorspace, this can be greatly sped up
                        uno::Sequence<rendering::ARGBColor> aIntermediate(
                            convertIntegerToARGB(deviceColor));
                        return targetColorSpace->convertFromARGB(aIntermediate);
                    }
                }
                virtual uno::Sequence< ::sal_Int8 > SAL_CALL convertToIntegerColorSpace( const uno::Sequence< ::sal_Int8 >& deviceColor,
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
                        uno::Sequence<rendering::ARGBColor> aIntermediate(
                            convertIntegerToARGB(deviceColor));
                        return targetColorSpace->convertIntegerFromARGB(aIntermediate);
                    }
                }
                virtual uno::Sequence< rendering::RGBColor > SAL_CALL convertIntegerToRGB( const uno::Sequence< ::sal_Int8 >& deviceColor ) override
                {
                    const sal_Int8* pIn( deviceColor.getConstArray() );
                    const std::size_t  nLen( deviceColor.getLength() );
                    ENSURE_ARG_OR_THROW2(nLen%4==0,
                                         "number of channels no multiple of 4",
                                         static_cast<rendering::XColorSpace*>(this), 0);

                    uno::Sequence< rendering::RGBColor > aRes(nLen/4);
                    rendering::RGBColor* pOut( aRes.getArray() );
                    for( std::size_t i=0; i<nLen; i+=4 )
                    {
                        *pOut++ = rendering::RGBColor(
                            vcl::unotools::toDoubleColor(pIn[0]),
                            vcl::unotools::toDoubleColor(pIn[1]),
                            vcl::unotools::toDoubleColor(pIn[2]));
                        pIn += 4;
                    }
                    return aRes;
                }

                virtual uno::Sequence< rendering::ARGBColor > SAL_CALL convertIntegerToARGB( const uno::Sequence< ::sal_Int8 >& deviceColor ) override
                {
                    const sal_Int8* pIn( deviceColor.getConstArray() );
                    const std::size_t  nLen( deviceColor.getLength() );
                    ENSURE_ARG_OR_THROW2(nLen%4==0,
                                         "number of channels no multiple of 4",
                                         static_cast<rendering::XColorSpace*>(this), 0);

                    uno::Sequence< rendering::ARGBColor > aRes(nLen/4);
                    rendering::ARGBColor* pOut( aRes.getArray() );
                    for( std::size_t i=0; i<nLen; i+=4 )
                    {
                        *pOut++ = rendering::ARGBColor(
                            1.0,
                            vcl::unotools::toDoubleColor(pIn[0]),
                            vcl::unotools::toDoubleColor(pIn[1]),
                            vcl::unotools::toDoubleColor(pIn[2]));
                        pIn += 4;
                    }
                    return aRes;
                }

                virtual uno::Sequence< rendering::ARGBColor > SAL_CALL convertIntegerToPARGB( const uno::Sequence< ::sal_Int8 >& deviceColor ) override
                {
                    const sal_Int8* pIn( deviceColor.getConstArray() );
                    const std::size_t  nLen( deviceColor.getLength() );
                    ENSURE_ARG_OR_THROW2(nLen%4==0,
                                         "number of channels no multiple of 4",
                                         static_cast<rendering::XColorSpace*>(this), 0);

                    uno::Sequence< rendering::ARGBColor > aRes(nLen/4);
                    rendering::ARGBColor* pOut( aRes.getArray() );
                    for( std::size_t i=0; i<nLen; i+=4 )
                    {
                        *pOut++ = rendering::ARGBColor(
                            1.0,
                            vcl::unotools::toDoubleColor(pIn[0]),
                            vcl::unotools::toDoubleColor(pIn[1]),
                            vcl::unotools::toDoubleColor(pIn[2]));
                        pIn += 4;
                    }
                    return aRes;
                }

                virtual uno::Sequence< ::sal_Int8 > SAL_CALL convertIntegerFromRGB( const uno::Sequence< rendering::RGBColor >& rgbColor ) override
                {
                    const rendering::RGBColor* pIn( rgbColor.getConstArray() );
                    const std::size_t             nLen( rgbColor.getLength() );

                    uno::Sequence< sal_Int8 > aRes(nLen*4);
                    sal_Int8* pColors=aRes.getArray();
                    for( std::size_t i=0; i<nLen; ++i )
                    {
                        *pColors++ = vcl::unotools::toByteColor(pIn->Red);
                        *pColors++ = vcl::unotools::toByteColor(pIn->Green);
                        *pColors++ = vcl::unotools::toByteColor(pIn->Blue);
                        *pColors++ = 1.0;
                        ++pIn;
                    }
                    return aRes;
                }

                virtual uno::Sequence< ::sal_Int8 > SAL_CALL convertIntegerFromARGB( const uno::Sequence< rendering::ARGBColor >& rgbColor ) override
                {
                    const rendering::ARGBColor* pIn( rgbColor.getConstArray() );
                    const std::size_t              nLen( rgbColor.getLength() );

                    uno::Sequence< sal_Int8 > aRes(nLen*4);
                    sal_Int8* pColors=aRes.getArray();
                    for( std::size_t i=0; i<nLen; ++i )
                    {
                        *pColors++ = vcl::unotools::toByteColor(pIn->Red);
                        *pColors++ = vcl::unotools::toByteColor(pIn->Green);
                        *pColors++ = vcl::unotools::toByteColor(pIn->Blue);
                        *pColors++ = -1;
                        ++pIn;
                    }
                    return aRes;
                }

                virtual uno::Sequence< ::sal_Int8 > SAL_CALL convertIntegerFromPARGB( const uno::Sequence< rendering::ARGBColor >& rgbColor ) override
                {
                    const rendering::ARGBColor* pIn( rgbColor.getConstArray() );
                    const std::size_t              nLen( rgbColor.getLength() );

                    uno::Sequence< sal_Int8 > aRes(nLen*4);
                    sal_Int8* pColors=aRes.getArray();
                    for( std::size_t i=0; i<nLen; ++i )
                    {
                        *pColors++ = vcl::unotools::toByteColor(pIn->Red/pIn->Alpha);
                        *pColors++ = vcl::unotools::toByteColor(pIn->Green/pIn->Alpha);
                        *pColors++ = vcl::unotools::toByteColor(pIn->Blue/pIn->Alpha);
                        *pColors++ = -1;
                        ++pIn;
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

            struct StandardColorSpaceHolder : public rtl::StaticWithInit<uno::Reference<rendering::XIntegerBitmapColorSpace>,
                                                                         StandardColorSpaceHolder>
            {
                uno::Reference<rendering::XIntegerBitmapColorSpace> operator()()
                {
                    return new StandardColorSpace();
                }
            };

            struct StandardNoAlphaColorSpaceHolder : public rtl::StaticWithInit<uno::Reference<rendering::XIntegerBitmapColorSpace>,
                                                                         StandardNoAlphaColorSpaceHolder>
            {
                uno::Reference<rendering::XIntegerBitmapColorSpace> operator()()
                {
                    return new StandardNoAlphaColorSpace();
                }
            };
        }

        uno::Reference<rendering::XIntegerBitmapColorSpace> getStdColorSpace()
        {
            return StandardColorSpaceHolder::get();
        }

        uno::Reference<rendering::XIntegerBitmapColorSpace> getStdColorSpaceWithoutAlpha()
        {
            return StandardNoAlphaColorSpaceHolder::get();
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

        ::Color stdIntSequenceToColor( const uno::Sequence<sal_Int8>& rColor )
        {
#ifdef OSL_BIGENDIAN
            const sal_Int8* pCols( rColor.getConstArray() );
            return ::Color( pCols[3], pCols[0], pCols[1], pCols[2] );
#else
            return ::Color( *reinterpret_cast< const ::ColorData* >(rColor.getConstArray()) );
#endif
        }

        uno::Sequence<sal_Int8> colorToStdIntSequence( const ::Color& rColor )
        {
            uno::Sequence<sal_Int8> aRet(4);
            sal_Int8* pCols( aRet.getArray() );
#ifdef OSL_BIGENDIAN
            pCols[0] = rColor.GetRed();
            pCols[1] = rColor.GetGreen();
            pCols[2] = rColor.GetBlue();
            pCols[3] = 255-rColor.GetTransparency();
#else
            *reinterpret_cast<sal_Int32*>(pCols) = rColor.GetColor();
#endif
            return aRet;
        }

        // Create a corrected view transformation out of the give one,
        // which ensures that the rectangle given by (0,0) and
        // rSpriteSize is mapped with its left,top corner to (0,0)
        // again. This is required to properly render sprite
        // animations to buffer bitmaps.
        ::basegfx::B2DHomMatrix& calcRectToOriginTransform( ::basegfx::B2DHomMatrix&            o_transform,
                                                            const ::basegfx::B2DRange&          i_srcRect,
                                                            const ::basegfx::B2DHomMatrix&      i_transformation )
        {
            if( i_srcRect.isEmpty() )
                return o_transform=i_transformation;

            // transform by given transformation
            ::basegfx::B2DRectangle aTransformedRect;

            calcTransformedRectBounds( aTransformedRect,
                                       i_srcRect,
                                       i_transformation );

            // now move resulting left,top point of bounds to (0,0)
            const basegfx::B2DHomMatrix aCorrectedTransform(basegfx::tools::createTranslateB2DHomMatrix(
                -aTransformedRect.getMinX(), -aTransformedRect.getMinY()));

            // prepend to original transformation
            o_transform = aCorrectedTransform * i_transformation;

            return o_transform;
        }

        ::basegfx::B2DRange& calcTransformedRectBounds( ::basegfx::B2DRange&            outRect,
                                                        const ::basegfx::B2DRange&      inRect,
                                                        const ::basegfx::B2DHomMatrix&  transformation )
        {
            outRect.reset();

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

        bool isInside( const ::basegfx::B2DRange&       rContainedRect,
                       const ::basegfx::B2DRange&       rTransformRect,
                       const ::basegfx::B2DHomMatrix&   rTransformation )
        {
            if( rContainedRect.isEmpty() || rTransformRect.isEmpty() )
                return false;

            ::basegfx::B2DPolygon aPoly(
                ::basegfx::tools::createPolygonFromRect( rTransformRect ) );
            aPoly.transform( rTransformation );

            return ::basegfx::tools::isInside( aPoly,
                                               ::basegfx::tools::createPolygonFromRect(
                                                   rContainedRect ),
                                               true );
        }

        namespace
        {
            bool clipAreaImpl( ::basegfx::B2IRange*       o_pDestArea,
                               ::basegfx::B2IRange&       io_rSourceArea,
                               ::basegfx::B2IPoint&       io_rDestPoint,
                               const ::basegfx::B2IRange& rSourceBounds,
                               const ::basegfx::B2IRange& rDestBounds )
            {
                const ::basegfx::B2IPoint aSourceTopLeft(
                    io_rSourceArea.getMinimum() );

                ::basegfx::B2IRange aLocalSourceArea( io_rSourceArea );

                // clip source area (which must be inside rSourceBounds)
                aLocalSourceArea.intersect( rSourceBounds );

                if( aLocalSourceArea.isEmpty() )
                    return false;

                // calc relative new source area points (relative to orig
                // source area)
                const ::basegfx::B2IVector aUpperLeftOffset(
                    aLocalSourceArea.getMinimum()-aSourceTopLeft );
                const ::basegfx::B2IVector aLowerRightOffset(
                    aLocalSourceArea.getMaximum()-aSourceTopLeft );

                ::basegfx::B2IRange aLocalDestArea( io_rDestPoint + aUpperLeftOffset,
                                                    io_rDestPoint + aLowerRightOffset );

                // clip dest area (which must be inside rDestBounds)
                aLocalDestArea.intersect( rDestBounds );

                if( aLocalDestArea.isEmpty() )
                    return false;

                // calc relative new dest area points (relative to orig
                // source area)
                const ::basegfx::B2IVector aDestUpperLeftOffset(
                    aLocalDestArea.getMinimum()-io_rDestPoint );
                const ::basegfx::B2IVector aDestLowerRightOffset(
                    aLocalDestArea.getMaximum()-io_rDestPoint );

                io_rSourceArea = ::basegfx::B2IRange( aSourceTopLeft + aDestUpperLeftOffset,
                                                      aSourceTopLeft + aDestLowerRightOffset );
                io_rDestPoint  = aLocalDestArea.getMinimum();

                if( o_pDestArea )
                    *o_pDestArea = aLocalDestArea;

                return true;
            }
        }

        bool clipScrollArea( ::basegfx::B2IRange&                  io_rSourceArea,
                             ::basegfx::B2IPoint&                  io_rDestPoint,
                             ::std::vector< ::basegfx::B2IRange >& o_ClippedAreas,
                             const ::basegfx::B2IRange&            rBounds )
        {
            ::basegfx::B2IRange aResultingDestArea;

            // compute full destination area (to determine uninitialized
            // areas below)
            const ::basegfx::B2I64Tuple& rRange( io_rSourceArea.getRange() );
            ::basegfx::B2IRange aInputDestArea( io_rDestPoint.getX(),
                                                io_rDestPoint.getY(),
                                                (io_rDestPoint.getX()
                                                 + static_cast<sal_Int32>(rRange.getX())),
                                                (io_rDestPoint.getY()
                                                 + static_cast<sal_Int32>(rRange.getY())) );
            // limit to output area (no point updating outside of it)
            aInputDestArea.intersect( rBounds );

            // clip to rBounds
            if( !clipAreaImpl( &aResultingDestArea,
                               io_rSourceArea,
                               io_rDestPoint,
                               rBounds,
                               rBounds ) )
                return false;

            // finally, compute all areas clipped off the total
            // destination area.
            ::basegfx::computeSetDifference( o_ClippedAreas,
                                             aInputDestArea,
                                             aResultingDestArea );

            return true;
        }

        ::basegfx::B2IRange spritePixelAreaFromB2DRange( const ::basegfx::B2DRange& rRange )
        {
            if( rRange.isEmpty() )
                return ::basegfx::B2IRange();

            const ::basegfx::B2IPoint aTopLeft( ::basegfx::fround( rRange.getMinX() ),
                                                ::basegfx::fround( rRange.getMinY() ) );
            return ::basegfx::B2IRange( aTopLeft,
                                        aTopLeft + ::basegfx::B2IPoint(
                                            ::basegfx::fround( rRange.getWidth() ),
                                            ::basegfx::fround( rRange.getHeight() ) ) );
        }

        uno::Sequence< uno::Any >& getDeviceInfo( const uno::Reference< rendering::XCanvas >& i_rxCanvas,
                                                  uno::Sequence< uno::Any >&                  o_rxParams )
        {
            o_rxParams.realloc( 0 );

            if( i_rxCanvas.is() )
            {
                try
                {
                    uno::Reference< rendering::XGraphicDevice > xDevice( i_rxCanvas->getDevice(),
                                                                         uno::UNO_QUERY_THROW );

                    uno::Reference< lang::XServiceInfo >  xServiceInfo( xDevice,
                                                                        uno::UNO_QUERY_THROW );
                    uno::Reference< beans::XPropertySet > xPropSet( xDevice,
                                                                    uno::UNO_QUERY_THROW );

                    o_rxParams.realloc( 2 );

                    o_rxParams[ 0 ] <<= xServiceInfo->getImplementationName();
                    o_rxParams[ 1 ] = xPropSet->getPropertyValue( "DeviceHandle" );
                }
                catch( const uno::Exception& )
                {
                    // ignore, but return empty sequence
                }
            }

            return o_rxParams;
        }

        awt::Rectangle getAbsoluteWindowRect( const awt::Rectangle&                  rRect,
                                              const uno::Reference< awt::XWindow2 >& xWin  )
        {
            awt::Rectangle aRetVal( rRect );

            VclPtr<vcl::Window> pWindow = VCLUnoHelper::GetWindow(xWin);
            if( pWindow )
            {
                ::Point aPoint( aRetVal.X,
                                aRetVal.Y );

                aPoint = pWindow->OutputToScreenPixel( aPoint );

                aRetVal.X = aPoint.X();
                aRetVal.Y = aPoint.Y();
            }

            return aRetVal;
        }

        ::basegfx::B2DPolyPolygon getBoundMarksPolyPolygon( const ::basegfx::B2DRange& rRange )
        {
            ::basegfx::B2DPolyPolygon aPolyPoly;
            ::basegfx::B2DPolygon     aPoly;

            const double nX0( rRange.getMinX() );
            const double nY0( rRange.getMinY() );
            const double nX1( rRange.getMaxX() );
            const double nY1( rRange.getMaxY() );

            aPoly.append( ::basegfx::B2DPoint( nX0+4,
                                               nY0 ) );
            aPoly.append( ::basegfx::B2DPoint( nX0,
                                               nY0 ) );
            aPoly.append( ::basegfx::B2DPoint( nX0,
                                               nY0+4 ) );
            aPolyPoly.append( aPoly ); aPoly.clear();

            aPoly.append( ::basegfx::B2DPoint( nX1-4,
                                               nY0 ) );
            aPoly.append( ::basegfx::B2DPoint( nX1,
                                               nY0 ) );
            aPoly.append( ::basegfx::B2DPoint( nX1,
                                               nY0+4 ) );
            aPolyPoly.append( aPoly ); aPoly.clear();

            aPoly.append( ::basegfx::B2DPoint( nX0+4,
                                               nY1 ) );
            aPoly.append( ::basegfx::B2DPoint( nX0,
                                               nY1 ) );
            aPoly.append( ::basegfx::B2DPoint( nX0,
                                               nY1-4 ) );
            aPolyPoly.append( aPoly ); aPoly.clear();

            aPoly.append( ::basegfx::B2DPoint( nX1-4,
                                               nY1 ) );
            aPoly.append( ::basegfx::B2DPoint( nX1,
                                               nY1 ) );
            aPoly.append( ::basegfx::B2DPoint( nX1,
                                               nY1-4 ) );
            aPolyPoly.append( aPoly );

            return aPolyPoly;
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
            ::canvas::tools::mergeViewAndRenderTransform(aMatrix,
                                                         viewState,
                                                         renderState);
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
                    ::std::max(
                        ::basegfx::B2DVector(aRightBottom-aLeftTop).getLength(),
                        ::basegfx::B2DVector(aRightTop-aLeftBottom).getLength() ) + 1.0 ) );

            // typical number for pixel of the same color (strip size)
            const int nStripSize( nGradientSize < 50 ? 2 : 4 );

            // use at least three steps, and at utmost the number of color
            // steps
            return ::std::max( 3,
                               ::std::min(
                                   nGradientSize / nStripSize,
                                   nColorSteps ) );
        }

        void clipOutDev(const rendering::ViewState& viewState,
                        const rendering::RenderState& renderState,
                        OutputDevice& rOutDev,
                        OutputDevice* p2ndOutDev)
        {
            // accumulate non-empty clips into one region
            vcl::Region aClipRegion(true);

            if( viewState.Clip.is() )
            {
                ::basegfx::B2DPolyPolygon aClipPoly(
                    ::basegfx::unotools::b2DPolyPolygonFromXPolyPolygon2D(viewState.Clip) );

                if( aClipPoly.count() )
                {
                    // setup non-empty clipping
                    ::basegfx::B2DHomMatrix aMatrix;
                    aClipPoly.transform(
                        ::basegfx::unotools::homMatrixFromAffineMatrix( aMatrix,
                                                                        viewState.AffineTransform ) );

                    aClipRegion = vcl::Region::GetRegionFromPolyPolygon( ::tools::PolyPolygon( aClipPoly ) );
                }
                else
                {
                    // clip polygon is empty
                    aClipRegion.SetEmpty();
                }
            }

            if( renderState.Clip.is() )
            {
                ::basegfx::B2DPolyPolygon aClipPoly(
                    ::basegfx::unotools::b2DPolyPolygonFromXPolyPolygon2D(renderState.Clip) );

                ::basegfx::B2DHomMatrix aMatrix;
                aClipPoly.transform(
                    ::canvas::tools::mergeViewAndRenderTransform( aMatrix,
                                                                  viewState,
                                                                  renderState ) );

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
            {
                rOutDev.SetClipRegion();

                if( p2ndOutDev )
                    p2ndOutDev->SetClipRegion();
            }
            else
            {
                rOutDev.SetClipRegion( aClipRegion );

                if( p2ndOutDev )
                    p2ndOutDev->SetClipRegion( aClipRegion );
            }
        }
    } // namespace tools

} // namespace canvas

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
