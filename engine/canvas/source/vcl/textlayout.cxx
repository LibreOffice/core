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

#include <comphelper/diagnose_ex.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/numeric/ftools.hxx>
#include <basegfx/utils/canvastools.hxx>
#include <com/sun/star/rendering/CompositeOperation.hpp>
#include <com/sun/star/rendering/RenderState.hpp>
#include <com/sun/star/rendering/TextDirection.hpp>
#include <com/sun/star/rendering/ViewState.hpp>
#include <comphelper/sequence.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <utility>
#include <vcl/kernarray.hxx>
#include <vcl/metric.hxx>
#include <vcl/virdev.hxx>

#include <canvas/canvastools.hxx>

#include "textlayout.hxx"

#include <memory>

using namespace ::com::sun::star;

namespace vclcanvas
{
    namespace
    {
        void setupLayoutMode( OutputDevice& rOutDev,
                              sal_Int8      nTextDirection )
        {
            // TODO(P3): avoid if already correctly set
            vcl::text::ComplexTextLayoutFlags nLayoutMode = vcl::text::ComplexTextLayoutFlags::Default;
            switch( nTextDirection )
            {
                case rendering::TextDirection::WEAK_LEFT_TO_RIGHT:
                    break;
                case rendering::TextDirection::STRONG_LEFT_TO_RIGHT:
                    nLayoutMode = vcl::text::ComplexTextLayoutFlags::BiDiStrong;
                    break;
                case rendering::TextDirection::WEAK_RIGHT_TO_LEFT:
                    nLayoutMode = vcl::text::ComplexTextLayoutFlags::BiDiRtl;
                    break;
                case rendering::TextDirection::STRONG_RIGHT_TO_LEFT:
                    nLayoutMode = vcl::text::ComplexTextLayoutFlags::BiDiRtl | vcl::text::ComplexTextLayoutFlags::BiDiStrong;
                    break;
                default:
                    break;
            }

            // set calculated layout mode. Origin is always the left edge,
            // as required at the API spec
            rOutDev.SetLayoutMode( nLayoutMode | vcl::text::ComplexTextLayoutFlags::TextOriginLeft );
        }
    }

    TextLayout::TextLayout( rendering::StringContext                   aText,
                            sal_Int8                                   nDirection,
                            CanvasFont::Reference                      rFont,
                            uno::Reference<rendering::XGraphicDevice>  xDevice,
                            OutDevProviderSharedPtr                    xOutDev ) :
        maText(std::move( aText )),
        mpFont(std::move( rFont )),
        mxDevice(std::move( xDevice )),
        mpOutDevProvider(std::move( xOutDev )),
        mnTextDirection( nDirection )
    {}

    void TextLayout::disposing(std::unique_lock<std::mutex>& rGuard)
    {
        rGuard.unlock();
        {
            SolarMutexGuard aGuard;
            mpOutDevProvider.reset();
            mxDevice.clear();
            mpFont.clear();
        }
        rGuard.lock();
    }

    // XTextLayout

    cpo::uno::Sequence< double > TextLayout::queryLogicalAdvancements(  )
    {
        SolarMutexGuard aGuard;

        return maLogicalAdvancements;
    }

    void TextLayout::applyLogicalAdvancements( const cpo::uno::Sequence< double >& aAdvancements )
    {
        SolarMutexGuard aGuard;

        ENSURE_ARG_OR_THROW( aAdvancements.getLength() == maText.Length,
                         "TextLayout::applyLogicalAdvancements(): mismatching number of advancements" );

        maLogicalAdvancements = aAdvancements;
    }

    cpo::uno::Sequence< bool > TextLayout::queryKashidaPositions(  )
    {
        SolarMutexGuard aGuard;

        return maKashidaPositions;
    }

    void TextLayout::applyKashidaPositions( const cpo::uno::Sequence< bool >& aPositions )
    {
        SolarMutexGuard aGuard;

        ENSURE_ARG_OR_THROW( !aPositions.hasElements() || aPositions.getLength() == maText.Length,
                         "TextLayout::applyKashidaPositions(): mismatching number of positions" );

        maKashidaPositions = aPositions;
    }

    geometry::RealRectangle2D TextLayout::queryTextBounds(  )
    {
        SolarMutexGuard aGuard;

        if( !mpOutDevProvider )
            return geometry::RealRectangle2D();

        OutputDevice& rOutDev = mpOutDevProvider->getOutDev();

        ScopedVclPtrInstance< VirtualDevice > pVDev( rOutDev );
        pVDev->SetFont( mpFont->getVCLFont() );

        // need metrics for Y offset, the XCanvas always renders
        // relative to baseline
        const ::FontMetric aMetric( pVDev->GetFontMetric() );

        setupLayoutMode( *pVDev, mnTextDirection );

        const sal_Int32 nAboveBaseline( -aMetric.GetAscent() );
        const sal_Int32 nBelowBaseline( aMetric.GetDescent() );

        if( maLogicalAdvancements.hasElements() )
        {
            return geometry::RealRectangle2D( 0, nAboveBaseline,
                                              maLogicalAdvancements[ maLogicalAdvancements.getLength()-1 ],
                                              nBelowBaseline );
        }
        else
        {
            return geometry::RealRectangle2D( 0, nAboveBaseline,
                                              pVDev->GetTextWidth(
                                                  maText.Text,
                                                  ::canvastools::numeric_cast<sal_uInt16>(maText.StartPosition),
                                                  ::canvastools::numeric_cast<sal_uInt16>(maText.Length) ),
                                              nBelowBaseline );
        }
    }

    sal_Int8 TextLayout::getMainTextDirection(  )
    {
        return mnTextDirection;
    }

    uno::Reference< rendering::XCanvasFont > TextLayout::getFont(  )
    {
        SolarMutexGuard aGuard;

        return mpFont;
    }

    rendering::StringContext TextLayout::getText(  )
    {
        return maText;
    }

    void TextLayout::draw( OutputDevice&                 rOutDev,
                           const Point&                  rOutpos,
                           const rendering::ViewState&   viewState,
                           const rendering::RenderState& renderState ) const
    {
        SolarMutexGuard aGuard;

#ifdef _WIN32
        // tdf#147999
        // On Windows we get the wrong font width for fallback fonts unless we setup again here.
        vcl::Font aFont(rOutDev.GetFont());
        vclcanvastools::setupFontWidth(mpFont->getFontMatrix(), aFont, rOutDev);
        rOutDev.SetFont(aFont);
#endif

        setupLayoutMode( rOutDev, mnTextDirection );

        if( maLogicalAdvancements.hasElements() )
        {
            // TODO(P2): cache that
            KernArray aOffsets(setupTextOffsets(maLogicalAdvancements, viewState, renderState));
            std::span<const bool> aKashidaArray(maKashidaPositions.getConstArray(), maKashidaPositions.getLength());

            // TODO(F3): ensure correct length and termination for DX
            // array (last entry _must_ contain the overall width)

            rOutDev.DrawTextArray( rOutpos,
                                   maText.Text,
                                   aOffsets,
                                   aKashidaArray,
                                   ::canvastools::numeric_cast<sal_uInt16>(maText.StartPosition),
                                   ::canvastools::numeric_cast<sal_uInt16>(maText.Length) );
        }
        else
        {
            rOutDev.DrawText( rOutpos,
                              maText.Text,
                              ::canvastools::numeric_cast<sal_uInt16>(maText.StartPosition),
                              ::canvastools::numeric_cast<sal_uInt16>(maText.Length) );
        }
    }

    namespace
    {
        class OffsetTransformer
        {
        public:
            explicit OffsetTransformer( ::basegfx::B2DHomMatrix aMat ) :
                maMatrix(std::move( aMat ))
            {
            }

            sal_Int32 operator()( const double& rOffset )
            {
                // This is an optimization of the normal rMat*[x,0]
                // transformation of the advancement vector (in x
                // direction), followed by a length calculation of the
                // resulting vector: advancement' =
                // ||rMat*[x,0]||. Since advancements are vectors, we
                // can ignore translational components, thus if [x,0],
                // it follows that rMat*[x,0]=[x',0] holds. Thus, we
                // just have to calc the transformation of the x
                // component.

                // TODO(F2): Handle non-horizontal advancements!
                return ::basegfx::fround( hypot(maMatrix.get(0,0)*rOffset,
                                                maMatrix.get(1,0)*rOffset) );
            }

        private:
            ::basegfx::B2DHomMatrix maMatrix;
        };
    }

    KernArray TextLayout::setupTextOffsets(
                                       const cpo::uno::Sequence< double >&   inputOffsets,
                                       const rendering::ViewState&      viewState,
                                       const rendering::RenderState&    renderState     ) const
    {
        ::basegfx::B2DHomMatrix aMatrix;

        ::canvastools::mergeViewAndRenderTransform(aMatrix,
                                                     viewState,
                                                     renderState);

        // fill integer offsets
        KernArray outputOffsets;
        OffsetTransformer aTransform(aMatrix);
        std::for_each(inputOffsets.begin(), inputOffsets.end(),
                      [&outputOffsets, &aTransform](double n) {outputOffsets.push_back(aTransform(n)); } );
        return outputOffsets;
    }

    OUString TextLayout::getImplementationName()
    {
        return u"VCLCanvas::TextLayout"_ustr;
    }

    bool TextLayout::supportsService( const OUString& ServiceName )
    {
        return cppu::supportsService( this, ServiceName );
    }

    cpo::uno::Sequence< OUString > TextLayout::getSupportedServiceNames()
    {
        return { u"com.sun.star.rendering.TextLayout"_ustr };
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
