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
#include <sal/log.hxx>

#include <math.h>

#include <com/sun/star/rendering/TextDirection.hpp>
#include <canvas/canvastools.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/numeric/ftools.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <utility>
#include <vcl/kernarray.hxx>
#include <vcl/metric.hxx>
#include <vcl/virdev.hxx>

#include "cairo_textlayout.hxx"

using namespace ::cairo;
using namespace ::com::sun::star;

namespace cairocanvas
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

    TextLayout::TextLayout( rendering::StringContext      aText,
                            sal_Int8                      nDirection,
                            sal_Int64                     /*nRandomSeed*/,
                            CanvasFont::Reference         rFont,
                            SurfaceProviderRef            rRefDevice ) :
        maText(std::move( aText )),
        mpFont(std::move( rFont )),
        mpRefDevice(std::move( rRefDevice )),
        mnTextDirection( nDirection )
    {
    }

    TextLayout::~TextLayout()
    {
    }

    void TextLayout::disposing(std::unique_lock<std::mutex>& /*rGuard*/)
    {
        mpFont.clear();
        mpRefDevice.clear();
    }

    // XTextLayout
    uno::Sequence< uno::Reference< rendering::XPolyPolygon2D > > SAL_CALL TextLayout::queryTextShapes(  )
    {
        // TODO
        return uno::Sequence< uno::Reference< rendering::XPolyPolygon2D > >();
    }

    uno::Sequence< geometry::RealRectangle2D > SAL_CALL TextLayout::queryInkMeasures(  )
    {
        // TODO
        return uno::Sequence< geometry::RealRectangle2D >();
    }

    uno::Sequence< geometry::RealRectangle2D > SAL_CALL TextLayout::queryMeasures(  )
    {
        // TODO
        return uno::Sequence< geometry::RealRectangle2D >();
    }

    uno::Sequence< double > SAL_CALL TextLayout::queryLogicalAdvancements(  )
    {
        std::unique_lock aGuard( m_aMutex );

        return maLogicalAdvancements;
    }

    void SAL_CALL TextLayout::applyLogicalAdvancements( const uno::Sequence< double >& aAdvancements )
    {
        std::unique_lock aGuard( m_aMutex );

        if( aAdvancements.getLength() != maText.Length )
        {
            SAL_WARN("canvas.cairo", "TextLayout::applyLogicalAdvancements(): mismatching number of advancements" );
            throw lang::IllegalArgumentException(u"mismatching number of advancements"_ustr, getXWeak(), 1);
        }

        maLogicalAdvancements = aAdvancements;
    }

    uno::Sequence< sal_Bool > SAL_CALL TextLayout::queryKashidaPositions(  )
    {
        std::unique_lock aGuard( m_aMutex );

        return maKashidaPositions;
    }

    void SAL_CALL TextLayout::applyKashidaPositions( const uno::Sequence< sal_Bool >& aPositions )
    {
        std::unique_lock aGuard( m_aMutex );

        if( aPositions.hasElements() && aPositions.getLength() != maText.Length )
        {
            SAL_WARN("canvas.cairo", "TextLayout::applyKashidaPositions(): mismatching number of positions" );
            throw lang::IllegalArgumentException(u"mismatching number of positions"_ustr, getXWeak(), 1);
        }

        maKashidaPositions = aPositions;
    }

    geometry::RealRectangle2D SAL_CALL TextLayout::queryTextBounds(  )
    {
        std::unique_lock aGuard( m_aMutex );

        OutputDevice* pOutDev = mpRefDevice->getOutputDevice();
        if( !pOutDev )
            return geometry::RealRectangle2D();

        ScopedVclPtrInstance< VirtualDevice > pVDev( *pOutDev );
        pVDev->SetFont( mpFont->getVCLFont() );

        // need metrics for Y offset, the XCanvas always renders
        // relative to baseline
        const ::FontMetric& aMetric( pVDev->GetFontMetric() );

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
                                                  ::canvas::tools::numeric_cast<sal_uInt16>(maText.StartPosition),
                                                  ::canvas::tools::numeric_cast<sal_uInt16>(maText.Length) ),
                                              nBelowBaseline );
        }
    }

    double SAL_CALL TextLayout::justify( double /*nSize*/ )
    {
        // TODO
        return 0.0;
    }

    double SAL_CALL TextLayout::combinedJustify( const uno::Sequence< uno::Reference< rendering::XTextLayout > >& /*aNextLayouts*/,
                                                 double /*nSize*/ )
    {
        // TODO
        return 0.0;
    }

    rendering::TextHit SAL_CALL TextLayout::getTextHit( const geometry::RealPoint2D& /*aHitPoint*/ )
    {
        // TODO
        return rendering::TextHit();
    }

    rendering::Caret SAL_CALL TextLayout::getCaret( sal_Int32 /*nInsertionIndex*/,
                                                    sal_Bool /*bExcludeLigatures*/ )
    {
        // TODO
        return rendering::Caret();
    }

    sal_Int32 SAL_CALL TextLayout::getNextInsertionIndex( sal_Int32 /*nStartIndex*/,
                                                          sal_Int32 /*nCaretAdvancement*/,
                                                          sal_Bool /*bExcludeLigatures*/ )
    {
        // TODO
        return 0;
    }

    uno::Reference< rendering::XPolyPolygon2D > SAL_CALL TextLayout::queryVisualHighlighting( sal_Int32 /*nStartIndex*/,
                                                                                              sal_Int32 /*nEndIndex*/ )
    {
        // TODO
        return uno::Reference< rendering::XPolyPolygon2D >();
    }

    uno::Reference< rendering::XPolyPolygon2D > SAL_CALL TextLayout::queryLogicalHighlighting( sal_Int32 /*nStartIndex*/,
                                                                                               sal_Int32 /*nEndIndex*/ )
    {
        // TODO
        return uno::Reference< rendering::XPolyPolygon2D >();
    }

    double SAL_CALL TextLayout::getBaselineOffset(  )
    {
        // TODO
        return 0.0;
    }

    sal_Int8 SAL_CALL TextLayout::getMainTextDirection(  )
    {
        return mnTextDirection;
    }

    uno::Reference< rendering::XCanvasFont > SAL_CALL TextLayout::getFont(  )
    {
        std::unique_lock aGuard( m_aMutex );

        return mpFont;
    }

    rendering::StringContext SAL_CALL TextLayout::getText(  )
    {
        return maText;
    }

  /**
   * TextLayout::draw
   *
   * Cairo-based text rendering. Draw text directly on the cairo surface with cairo fonts.
   * Avoid using VCL VirtualDevices for that, bypassing VCL DrawText functions, when possible
   *
   * Note: some text effects are not rendered due to lacking generic canvas or cairo canvas
   *       implementation. See issues 92657, 92658, 92659, 92660, 97529
   **/
    void TextLayout::draw( OutputDevice&                 rOutDev,
                           const Point&                  rOutpos,
                           const rendering::ViewState&   viewState,
                           const rendering::RenderState& renderState ) const
    {
        std::unique_lock aGuard( m_aMutex );
        setupLayoutMode( rOutDev, mnTextDirection );

        if (maLogicalAdvancements.hasElements())
        {
            KernArray aOffsets(setupTextOffsets(maLogicalAdvancements, viewState, renderState));
            std::span<const sal_Bool> aKashidaArray(maKashidaPositions.getConstArray(), maKashidaPositions.getLength());

            rOutDev.DrawTextArray( rOutpos, maText.Text, aOffsets, aKashidaArray,
                                   ::canvas::tools::numeric_cast<sal_uInt16>(maText.StartPosition),
                                   ::canvas::tools::numeric_cast<sal_uInt16>(maText.Length) );
        }
        else
        {
            rOutDev.DrawText( rOutpos, maText.Text,
                              ::canvas::tools::numeric_cast<sal_uInt16>(maText.StartPosition),
                              ::canvas::tools::numeric_cast<sal_uInt16>(maText.Length) );
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
                                       const uno::Sequence< double >&   inputOffsets,
                                       const rendering::ViewState&      viewState,
                                       const rendering::RenderState&    renderState     ) const
    {
        ::basegfx::B2DHomMatrix aMatrix;

        ::canvas::tools::mergeViewAndRenderTransform(aMatrix,
                                                     viewState,
                                                     renderState);

        // fill integer offsets
        KernArray outputOffsets;
        OffsetTransformer aTransform(aMatrix);
        std::for_each(inputOffsets.begin(), inputOffsets.end(),
                      [&outputOffsets, &aTransform](double n) {outputOffsets.push_back(aTransform(n)); } );
        return outputOffsets;
    }

    OUString SAL_CALL TextLayout::getImplementationName()
    {
        return u"CairoCanvas::TextLayout"_ustr;
    }

    sal_Bool SAL_CALL TextLayout::supportsService( const OUString& ServiceName )
    {
        return cppu::supportsService( this, ServiceName );
    }

    uno::Sequence< OUString > SAL_CALL TextLayout::getSupportedServiceNames()
    {
        return { u"com.sun.star.rendering.TextLayout"_ustr };
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
