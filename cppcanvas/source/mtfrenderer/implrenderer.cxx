/*************************************************************************
 *
 *  $RCSfile: implrenderer.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2004-11-26 20:54:32 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include <canvas/debug.hxx>
#include <canvas/verbosetrace.hxx>

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif
#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

#ifndef _RTL_LOGFILE_HXX_
#include <rtl/logfile.hxx>
#endif

#ifndef _COMPHELPER_SEQUENCE_HXX_
#include <comphelper/sequence.hxx>
#endif

#include <cppcanvas/canvas.hxx>

#ifndef _DRAFTS_COM_SUN_STAR_RENDERING_XGRAPHICDEVICE_HPP_
#include <drafts/com/sun/star/rendering/XGraphicDevice.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_RENDERING_TEXTURINGMODE_HPP_
#include <drafts/com/sun/star/rendering/TexturingMode.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_RENDERING_XPARAMETRICPOLYPOLYGON2DFACTORY_HPP_
#include <drafts/com/sun/star/rendering/XParametricPolyPolygon2DFactory.hpp>
#endif

#ifndef _VCL_CANVASTOOLS_HXX
#include <vcl/canvastools.hxx>
#endif
#ifndef _BGFX_TOOLS_CANVASTOOLS_HXX
#include <basegfx/tools/canvastools.hxx>
#endif
#ifndef _BGFX_NUMERIC_FTOOLS_HXX
#include <basegfx/numeric/ftools.hxx>
#endif
#ifndef _BGFX_POLYGON_B2DPOLYPOLYGONTOOLS_HXX
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#endif
#ifndef _BGFX_POLYGON_B2DPOLYGONTOOLS_HXX
#include <basegfx/polygon/b2dpolygontools.hxx>
#endif
#ifndef _CANVAS_CANVASTOOLS_HXX
#include <canvas/canvastools.hxx>
#endif
#ifndef _VCL_CANVASTOOLS_HXX
#include <vcl/canvastools.hxx>
#endif

#include <implrenderer.hxx>
#include <tools.hxx>
#include <outdevstate.hxx>

#include <action.hxx>
#include <bitmapaction.hxx>
#include <lineaction.hxx>
#include <pointaction.hxx>
#include <polypolyaction.hxx>
#include <textaction.hxx>
#include <transparencygroupaction.hxx>

#include <vector>
#include <algorithm>

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif

#ifndef _DRAFTS_COM_SUN_STAR_GEOMETRY_REALPOINT2D_HPP__
#include <drafts/com/sun/star/geometry/RealPoint2D.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_RENDERING_VIEWSTATE_HPP__
#include <drafts/com/sun/star/rendering/ViewState.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_RENDERING_RENDERSTATE_HPP__
#include <drafts/com/sun/star/rendering/RenderState.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_RENDERING_XCANVASFONT_HPP__
#include <drafts/com/sun/star/rendering/XCanvasFont.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_RENDERING_XPOLYPOLYGON2D_HPP__
#include <drafts/com/sun/star/rendering/XPolyPolygon2D.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_RENDERING_XCANVAS_HPP__
#include <drafts/com/sun/star/rendering/XCanvas.hpp>
#endif

#ifndef _BGFX_MATRIX_B2DHOMMATRIX_HXX
#include <basegfx/matrix/b2dhommatrix.hxx>
#endif
#ifndef _BGFX_TUPLE_B2DTUPLE_HXX
#include <basegfx/tuple/b2dtuple.hxx>
#endif

#ifndef _SV_GDIMTF_HXX
#include <vcl/gdimtf.hxx>
#endif

#ifndef _SV_METAACT_HXX
#include <vcl/metaact.hxx>
#endif

#ifndef _SV_VIRDEV_HXX
#include <vcl/virdev.hxx>
#endif

#ifndef _SV_METRIC_HXX
#include <vcl/metric.hxx>
#endif

#ifndef _TL_POLY_HXX
#include <tools/poly.hxx>
#endif

#include <outdevstate.hxx>


using namespace ::drafts::com::sun::star;
using namespace ::com::sun::star;


// free support functions
// ======================
namespace
{
    template < class MetaActionType > void setStateColor( MetaActionType*                   pAct,
                                                          bool&                             rIsColorSet,
                                                          uno::Sequence< double >&          rColorSequence,
                                                          const cppcanvas::CanvasSharedPtr& rCanvas )
    {
        // set rIsColorSet and check for true at the same time
        if( (rIsColorSet=pAct->IsSetting()) )
        {
            ::Color aColor( pAct->GetColor() );

            // force alpha part of color to
            // opaque. transparent painting is done
            // explicitely via META_TRANSPARENT_ACTION
            aColor.SetTransparency(0);
            //aColor.SetTransparency(128);

            rColorSequence = ::vcl::unotools::colorToDoubleSequence( rCanvas->getUNOCanvas()->getDevice(),
                                                                     aColor );
        }
    }

    // Doing that via inline class. Compilers tend to not inline free
    // functions.
    class ActionIndexComparator
    {
    public:
        ActionIndexComparator() {}

        bool operator()( const ::cppcanvas::internal::ImplRenderer::MtfAction& rLHS,
                         const ::cppcanvas::internal::ImplRenderer::MtfAction& rRHS )
        {
            return rLHS.mnOrigIndex < rRHS.mnOrigIndex;
        }
    };

    // state stack manipulators
    // ------------------------
    void clearStateStack( ::cppcanvas::internal::VectorOfOutDevStates& rStates )
    {
        rStates.clear();
        const ::cppcanvas::internal::OutDevState aDefaultState;
        rStates.push_back( aDefaultState );
    }

    ::cppcanvas::internal::OutDevState& getState( ::cppcanvas::internal::VectorOfOutDevStates& rStates )
    {
        return rStates.back();
    }

    void pushState( ::cppcanvas::internal::VectorOfOutDevStates& rStates )
    {
        rStates.push_back( getState( rStates ) );
    }

    void popState( ::cppcanvas::internal::VectorOfOutDevStates& rStates )
    {
        rStates.pop_back();
    }

}


namespace cppcanvas
{
    namespace internal
    {
        bool ImplRenderer::createFillAndStroke( const ::PolyPolygon&    rPolyPoly,
                                                const CanvasSharedPtr&  rCanvas,
                                                int                     rActionIndex,
                                                VectorOfOutDevStates&   rStates )
        {
            const OutDevState& rState( getState( rStates ) );
            if( rState.lineColor.getLength() == 0 &&
                rState.fillColor.getLength() == 0 )
            {
                return false;
            }

            maActions.push_back(
                MtfAction(
                    ActionSharedPtr(
                        new internal::PolyPolyAction( rPolyPoly, rCanvas, rState ) ),
                    rActionIndex ) );

            return true;
        }

        void ImplRenderer::skipContent( GDIMetaFile& rMtf,
                                        const char&  rCommentString ) const
        {
            MetaAction* pCurrAct;
            while( (pCurrAct=rMtf.NextAction()) )
            {
                if( pCurrAct->GetType() == META_COMMENT_ACTION &&
                    static_cast<MetaCommentAction*>(pCurrAct)->GetComment().CompareIgnoreCaseToAscii( rCommentString ) == COMPARE_EQUAL )
                {
                    // requested comment found, done
                    return;
                }
            }

            // EOF
            return;
        }

        void ImplRenderer::createGradientAction( const ::PolyPolygon&   rPoly,
                                                 const ::Gradient&      rGradient,
                                                 ::VirtualDevice&       rVDev,
                                                 const CanvasSharedPtr& rCanvas,
                                                 VectorOfOutDevStates&  rStates,
                                                 const Parameters&      rParms,
                                                 int&                   io_rCurrActionIndex,
                                                 bool                   bIsPolygonRectangle )
        {
            DBG_TESTSOLARMUTEX();

            ::PolyPolygon aDevicePoly( rVDev.LogicToPixel( rPoly ) );

            // decide, whether this gradient can be rendered natively
            // by the canvas, or must be emulated via VCL gradient
            // action extraction.
            const USHORT nSteps( rGradient.GetSteps() );

#if 0
            if( // step count is infinite, can use native canvas
                // gradients here
                nSteps == 0 ||
                // step count is sufficiently high, such that no
                // discernible difference should be visible.
                nSteps > 64 )
            {
                uno::Reference< rendering::XParametricPolyPolygon2DFactory > xFactory(
                    rCanvas->getUNOCanvas()->getDevice()->getParametricPolyPolygonFactory() );

                if( xFactory.is() )
                {
                    ::basegfx::B2DHomMatrix aTextureTransformation;
                    rendering::Texture      aTexture;

                    aTexture.RepeatModeX = rendering::TexturingMode::CLAMP;
                    aTexture.RepeatModeY = rendering::TexturingMode::CLAMP;
                    aTexture.Alpha = 1.0;


                    // setup start/end color values
                    // ----------------------------

                    // scale color coefficients with gradient intensities
                    const USHORT nStartIntensity( rGradient.GetStartIntensity() );
                    ::Color aVCLStartColor( rGradient.GetStartColor() );
                    aVCLStartColor.SetRed( aVCLStartColor.GetRed() * nStartIntensity / 100 );
                    aVCLStartColor.SetGreen( aVCLStartColor.GetGreen() * nStartIntensity / 100 );
                    aVCLStartColor.SetBlue( aVCLStartColor.GetBlue() * nStartIntensity / 100 );

                    const USHORT nEndIntensity( rGradient.GetEndIntensity() );
                    ::Color aVCLEndColor( rGradient.GetEndColor() );
                    aVCLEndColor.SetRed( aVCLEndColor.GetRed() * nEndIntensity / 100 );
                    aVCLEndColor.SetGreen( aVCLEndColor.GetGreen() * nEndIntensity / 100 );
                    aVCLEndColor.SetBlue( aVCLEndColor.GetBlue() * nEndIntensity / 100 );

                    const uno::Sequence< double > aStartColor(
                        ::vcl::unotools::colorToDoubleSequence( rCanvas->getUNOCanvas()->getDevice(),
                                                                aVCLStartColor ) );
                    const uno::Sequence< double > aEndColor(
                        ::vcl::unotools::colorToDoubleSequence( rCanvas->getUNOCanvas()->getDevice(),
                                                                aVCLEndColor ) );

                    // Setup texture transformation
                    // ----------------------------

                    const Rectangle aBounds( aDevicePoly.GetBoundRect() );

                    // setup rotation angle. VCL rotates
                    // counter-clockwise, while canvas transformation
                    // rotates clockwise
                    double nRotation( -rGradient.GetAngle() * M_PI / 1800.0 );

                    switch( rGradient.GetStyle() )
                    {
                        case GRADIENT_LINEAR:
                            // FALLTHROUGH intended
                        case GRADIENT_AXIAL:
                        {
                            // standard orientation for VCL linear
                            // gradient is vertical, thus, rotate 90
                            // degrees
                            nRotation += M_PI/2.0;

                            const double nBorder(
                                ::basegfx::pruneScaleValue(
                                    (1.0 - rGradient.GetBorder() / 100.0) ) );

                            // shrink texture, to account for border
                            // (only in x direction, linear gradient
                            // is constant in y direction, anyway)
                            aTextureTransformation.scale( nBorder,
                                                          1.0 );

                            // linear gradients don't respect offsets
                            // (they are implicitely assumed to be
                            // 50%). linear gradients don't have
                            // border on both sides, only on the
                            // startColor side, axial gradients have
                            // border on both sides. As both gradients
                            // are invariant in y direction: leave y
                            // offset alone.
                            double nOffsetX( rGradient.GetBorder() / 200.0 );

                            // determine type of gradient (and necessary
                            // transformation matrix, should it be emulated by a
                            // generic gradient)
                            switch( rGradient.GetStyle() )
                            {
                                case GRADIENT_LINEAR:
                                    nOffsetX = rGradient.GetBorder() / 100.0;
                                    aTexture.Gradient = xFactory->createLinearHorizontalGradient( aEndColor,
                                                                                                  aStartColor );
                                    break;

                                case GRADIENT_AXIAL:
                                    aTexture.Gradient = xFactory->createAxialHorizontalGradient( aStartColor,
                                                                                                 aEndColor );
                                    break;
                            }

                            // apply border offset values
                            aTextureTransformation.translate( nOffsetX,
                                                              0.0 );

                            // rotate texture according to gradient rotation
                            aTextureTransformation.translate( -0.5, -0.5 );
                            aTextureTransformation.rotate( nRotation );

                            // to let the first strip of a rotated
                            // gradient start at the _edge_ of the
                            // bound rect (and not, due to rotation,
                            // slightly inside), slightly enlarge the
                            // gradient:
                            //
                            // y/2 sin(alpha) + x/2 cos(alpha)
                            //
                            // (values to change are not actual
                            // gradient scales, but original bound
                            // rect dimensions. Since we still want
                            // the border setting to apply after that,
                            // we multiply with that as above for
                            // nScaleX)
                            const double nScale(
                                ::basegfx::pruneScaleValue(
                                    fabs( aBounds.GetHeight()*sin(nRotation) ) +
                                    fabs( aBounds.GetWidth()*cos(nRotation) )));

                            aTextureTransformation.scale( nScale, nScale );

                            // translate back origin to center of
                            // primitive
                            aTextureTransformation.translate( 0.5*aBounds.GetWidth(),
                                                              0.5*aBounds.GetHeight() );
                        }
                        break;

                        case GRADIENT_RADIAL:
                            // FALLTHROUGH intended
                        case GRADIENT_ELLIPTICAL:
                            // FALLTHROUGH intended
                        case GRADIENT_SQUARE:
                            // FALLTHROUGH intended
                        case GRADIENT_RECT:
                        {
                            // determine scale factors for the gradient (must
                            // be scaled up from [0,1]x[0,1] rect to object
                            // bounds). Will potentially changed in switch
                            // statement below.
                            // Respect border value, while doing so, the VCL
                            // gradient's border will effectively shrink the
                            // resulting gradient.
                            double nScaleX( aBounds.GetWidth() * (1.0 - rGradient.GetBorder() / 100.0) );
                            double nScaleY( aBounds.GetHeight()* (1.0 - rGradient.GetBorder() / 100.0) );

                            // determine offset values. Since the border is
                            // divided half-by-half to both sides of the
                            // gradient, divide translation offset by an
                            // additional 2. Also respect offset here, but
                            // since VCL gradients have their center at [0,0]
                            // for zero offset, but canvas gradients have
                            // their top, left edge aligned with the
                            // primitive, and offset of 50% effectively must
                            // yield zero shift. Both values will potentially
                            // be adapted in switch statement below.
                            double nOffsetX( aBounds.GetWidth() *
                                             (2.0 * rGradient.GetOfsX() - 100.0 + rGradient.GetBorder()) / 200.0 );
                            double nOffsetY( aBounds.GetHeight() *
                                             (2.0 * rGradient.GetOfsY() - 100.0 + rGradient.GetBorder()) / 200.0 );

                            // determine type of gradient (and necessary
                            // transformation matrix, should it be emulated by a
                            // generic gradient)
                            switch( rGradient.GetStyle() )
                            {
                                case GRADIENT_RADIAL:
                                {
                                    // enlarge gradient slightly
                                    aTextureTransformation.translate( -0.5, -0.5 );
                                    const double nSqrt2( sqrt(2.0) );
                                    aTextureTransformation.scale( nSqrt2,nSqrt2 );
                                    aTextureTransformation.translate( 0.5, 0.5 );

                                    // create isotrophic scaling
                                    if( nScaleX > nScaleY )
                                    {
                                        nOffsetY -= (nScaleX - nScaleY) * 0.5;
                                        nScaleY = nScaleX;
                                    }
                                    else
                                    {
                                        nOffsetX -= (nScaleY - nScaleX) * 0.5;
                                        nScaleX = nScaleY;
                                    }

                                    aTexture.Gradient = xFactory->createCircularGradient( aEndColor,
                                                                                          aStartColor );
                                }
                                break;

                                case GRADIENT_ELLIPTICAL:
                                {
                                    // enlarge gradient slightly
                                    aTextureTransformation.translate( -0.5, -0.5 );
                                    const double nSqrt2( sqrt(2.0) );
                                    aTextureTransformation.scale( nSqrt2,nSqrt2 );
                                    aTextureTransformation.translate( 0.5, 0.5 );

                                    aTexture.Gradient = xFactory->createCircularGradient( aEndColor,
                                                                                          aStartColor );
                                }
                                break;

                                case GRADIENT_SQUARE:
                                    // create isotrophic scaling
                                    if( nScaleX > nScaleY )
                                    {
                                        nOffsetY -= (nScaleX - nScaleY) * 0.5;
                                        nScaleY = nScaleX;
                                    }
                                    else
                                    {
                                        nOffsetX -= (nScaleY - nScaleX) * 0.5;
                                        nScaleX = nScaleY;
                                    }

                                    aTexture.Gradient = xFactory->createRectangularGradient( aEndColor,
                                                                                             aStartColor,
                                                                                             geometry::RealRectangle2D(0.0,0.0,
                                                                                                                       1.0,1.0) );
                                    break;

                                case GRADIENT_RECT:
                                    aTexture.Gradient = xFactory->createRectangularGradient(
                                        aEndColor,
                                        aStartColor,
                                        geometry::RealRectangle2D( aBounds.Left(),
                                                                   aBounds.Top(),
                                                                   aBounds.Right(),
                                                                   aBounds.Bottom() ) );
                                    break;
                            }

                            nScaleX = ::basegfx::pruneScaleValue( nScaleX );
                            nScaleY = ::basegfx::pruneScaleValue( nScaleY );

                            aTextureTransformation.scale( nScaleX, nScaleY );

                            // rotate texture according to gradient rotation
                            aTextureTransformation.translate( -0.5*nScaleX, -0.5*nScaleY );
                            aTextureTransformation.rotate( nRotation );
                            aTextureTransformation.translate( 0.5*nScaleX, 0.5*nScaleY );

                            aTextureTransformation.translate( nOffsetX, nOffsetY );
                        }
                        break;

                        default:
                            ENSURE_AND_THROW( false,
                                              "ImplRenderer::createGradientAction(): Unexpected gradient type" );
                            break;
                    }

                    ::basegfx::unotools::affineMatrixFromHomMatrix( aTexture.AffineTransform,
                                                                    aTextureTransformation );

                    maActions.push_back(
                        MtfAction(
                            ActionSharedPtr(
                                new internal::PolyPolyAction( aDevicePoly,
                                                              rCanvas,
                                                              getState( rStates ),
                                                              aTexture ) ),
                            io_rCurrActionIndex ) );

                    // done, using native gradients
                    return;
                }
            }
#endif

            // cannot currently use native canvas gradients, as
            // step size is given
            pushState( rStates );

            if( !bIsPolygonRectangle )
            {
                // only clip, if given polygon is not a rectangle in
                // the first place (the gradient is always limited to
                // the given bound rect)
                updateClipping(
                    rStates,
                    aDevicePoly.getB2DPolyPolygon(),
                    rCanvas,
                    true );
            }

            GDIMetaFile aTmpMtf;
            rVDev.AddGradientActions( rPoly.GetBoundRect(),
                                      rGradient,
                                      aTmpMtf );

            createActions( rCanvas, rVDev, aTmpMtf, rStates, rParms, io_rCurrActionIndex );

            popState( rStates );
        }

        uno::Reference< rendering::XCanvasFont > ImplRenderer::createFont( ::basegfx::B2DHomMatrix&     o_rFontMatrix,
                                                                           const ::Font&                rFont,
                                                                           const CanvasSharedPtr&       rCanvas,
                                                                           const ::VirtualDevice&       rVDev,
                                                                           const Parameters&            rParms ) const
        {
            rendering::FontRequest aFontRequest;

            if( rParms.maFontName.isValid() )
                aFontRequest.FontDescription.FamilyName = rParms.maFontName.getValue();
            else
                aFontRequest.FontDescription.FamilyName = rFont.GetName();

            aFontRequest.FontDescription.StyleName = rFont.GetStyleName();

            aFontRequest.FontDescription.IsSymbolFont = (rFont.GetCharSet() == RTL_TEXTENCODING_SYMBOL) ? util::TriState_YES : util::TriState_NO;
            aFontRequest.FontDescription.IsVertical = rFont.IsVertical() ? util::TriState_YES : util::TriState_NO;

            // TODO(F2): improve vclenum->panose conversion
            aFontRequest.FontDescription.FontDescription.Weight =
                rParms.maFontWeight.isValid() ?
                rParms.maFontWeight.getValue() :
                ::canvas::tools::numeric_cast<sal_Int8>( ::basegfx::fround( rFont.GetWeight() ) );
            aFontRequest.FontDescription.FontDescription.Letterform =
                rParms.maFontLetterForm.isValid() ?
                rParms.maFontLetterForm.getValue() :
                (rFont.GetItalic() == ITALIC_NONE) ? 0 : 9;

            // TODO(F2): use correct scale direction, font
            // height might be width or anything else
            aFontRequest.CellSize = rVDev.LogicToPixel( rFont.GetSize() ).Height();

            // setup state-local text transformation,
            // if the font be rotated
            const short nFontAngle( rFont.GetOrientation() );
            if( nFontAngle != 0 )
            {
                // set to unity transform rotated by font angle
                const double rAngle( nFontAngle * (F_PI / 1800.0) );
                o_rFontMatrix.identity();
                o_rFontMatrix.rotate( -rAngle );
            }

            geometry::Matrix2D aFontMatrix;
            ::canvas::tools::setIdentityMatrix2D( aFontMatrix );

            // check if the font is stretched or squeezed
            long nFontWidth = rFont.GetSize().Width();
            if( nFontWidth != 0 )
            {
                ::Font aTestFont = rFont;
                aTestFont.SetWidth( 0 );
                int nNormalWidth = rVDev.GetFontMetric( aTestFont ).GetWidth();
                if( nNormalWidth != nFontWidth )
                    if( nNormalWidth )
                        aFontMatrix.m00 = (double)nFontWidth / nNormalWidth;
            }

            return rCanvas->getUNOCanvas()->createFont( aFontRequest,
                                                        uno::Sequence< beans::PropertyValue >(),
                                                        aFontMatrix );
        }

        // create text effects such as shadow/relief/embossed
        void ImplRenderer::createTextWithEffectsAction(
                const Point&            rStartPoint,
                const String            rString,
                int                     nIndex,
                int                     nLength,
                const long*             pCharWidths,
                VirtualDevice&          rVDev,
                const CanvasSharedPtr&  rCanvas,
                VectorOfOutDevStates&   rStates,
                const Parameters&       rParms,
                int                     nCurrActionIndex )
        {
            ENSURE_AND_THROW( nIndex >= 0 && nLength <= rString.Len() + nIndex,
                              "ImplRenderer::createTextWithEffectsAction(): Invalid text index" );

            ::cppcanvas::internal::OutDevState& rState = getState( rStates );

            // TODO(F2): implement all text effects
            if( rState.textAlignment );             // TODO(F2): NYI

            if( rState.isTextEffectShadowSet )
            {
                // calculate relief offset (similar to outdev3.cxx)
                // TODO(F3): better match with outdev3.cxx
                long nShadowOffset = static_cast<long>(1.5 + ((rVDev.GetFont().GetHeight()-24.0)/24.0));
                if( nShadowOffset < 1 )
                    nShadowOffset = 1;
                Point aShadowPoint( nShadowOffset, nShadowOffset );
                aShadowPoint += rStartPoint;

                // determine shadow color (from outdev3.cxx)
                ::Color aTextColor = ::vcl::unotools::sequenceToColor(
                    rCanvas->getUNOCanvas()->getDevice(), rState.textColor );
                bool bIsDark = (aTextColor.GetColor() == COL_BLACK)
                    || (aTextColor.GetLuminance() < 8);
                ::Color aShadowColor( bIsDark ? COL_LIGHTGRAY : COL_BLACK );
                aShadowColor.SetTransparency( aTextColor.GetTransparency() );

                // draw shadow text and restore original rState
                // TODO(P2): just restore textColor instead of push/pop
                pushState( rStates );
                // ::com::sun::star::uno::Sequence< double > origTextColor = rState.textColor;
                getState( rStates ).textColor = ::vcl::unotools::colorToDoubleSequence(
                    rCanvas->getUNOCanvas()->getDevice(), aShadowColor );
                createTextWithLinesAction(
                    aShadowPoint, rString, nIndex, nLength,
                    pCharWidths, rVDev, rCanvas, rStates, rParms, nCurrActionIndex );
                popState( rStates );
                // rState.textColor = origTextColor;
            }

            // draw the normal text
            createTextWithLinesAction(
                rStartPoint, rString, nIndex, nLength,
                pCharWidths, rVDev, rCanvas, rStates, rParms, nCurrActionIndex );

            if( rState.textReliefStyle )
            {
                // calculate relief offset (similar to outdev3.cxx)
                long nReliefOffset = rVDev.PixelToLogic( Size( 1, 1 ) ).Height();
                nReliefOffset += nReliefOffset/2;
                if( nReliefOffset < 1 )
                    nReliefOffset = 1;

                if( rState.textReliefStyle == RELIEF_ENGRAVED )
                    nReliefOffset = -nReliefOffset;
                Point aReliefPoint( nReliefOffset, nReliefOffset );
                aReliefPoint += rStartPoint;

                // determine relief color (from outdev3.cxx)
                ::Color aTextColor = ::vcl::unotools::sequenceToColor(
                    rCanvas->getUNOCanvas()->getDevice(), rState.textColor );
                ::Color aReliefColor( COL_LIGHTGRAY );
                if( aTextColor.GetColor() == COL_BLACK )
                    aReliefColor = ::Color( COL_WHITE );
                else if( aTextColor.GetColor() == COL_WHITE )
                    aReliefColor = ::Color( COL_BLACK );
                aReliefColor.SetTransparency( aTextColor.GetTransparency() );

                // draw relief text and restore original rState
                // TODO(P2): just restore textColor instead of push/pop
                pushState( rStates );
                // ::com::sun::star::uno::Sequence< double > origTextColor = rState.textColor;
                getState( rStates ).textColor = ::vcl::unotools::colorToDoubleSequence(
                    rCanvas->getUNOCanvas()->getDevice(), aReliefColor );
                createTextWithLinesAction(
                    aReliefPoint, rString, nIndex, nLength,
                    pCharWidths, rVDev, rCanvas, rStates, rParms, nCurrActionIndex );
                popState( rStates );
                // rState.textColor = origTextColor;
            }
        }


        // create draw actions for text and underlines/strikeouts/etc.
        void ImplRenderer::createTextWithLinesAction(
                const Point&            rStartPoint,
                const String            rString,
                int                     nIndex,
                int                     nLength,
                const long*             pCharWidths,
                VirtualDevice&          rVDev,
                const CanvasSharedPtr&  rCanvas,
                VectorOfOutDevStates&   rStates,
                const Parameters&       rParms,
                int                     nCurrActionIndex )
       {
            ::cppcanvas::internal::OutDevState& rState = getState( rStates );

            Point aStartPixel = rVDev.LogicToPixel( rStartPoint );
            internal::Action* pAction = NULL;

            if( rState.isTextOutlineModeSet )
            {
                ::PolyPolygon aVclPolyPolygon;
                if( rVDev.GetTextOutline( aVclPolyPolygon, rString,
                    nIndex, nIndex, nLength, TRUE, 0, pCharWidths ) )
                {
                   int nOutlineWidth = rVDev.GetFont().GetHeight() / 32;
                    if( nOutlineWidth <= 0 )
                        nOutlineWidth = 1;
                    // TODO(F3): rState.strokeWith = nOutlineWidth;
                    // Path stroking not yet functional for all canvas
                    // implementations
                    aVclPolyPolygon.Translate( rStartPoint ); //####???
                    aVclPolyPolygon = rVDev.LogicToPixel( aVclPolyPolygon );

                    // set outline color
                    rState.lineColor = rState.textColor;
                    rState.isLineColorSet = true;
                    // set the fill color to match the VCL fill color for outlined text
                    // TODO: in the current Canvas model setting a fixed color is beyond ugly
                    ColorSharedPtr pColor( rCanvas.get()->createColor() );
                    rState.fillColor = pColor->getDeviceColor( 0xFFFFFFFF ); // solid white
                    rState.isFillColorSet = true;
                    // NOTE: rState.strokeWidth is already set by the caller

                    // TODO(F3): use bezier polygon directly when it
                    // is implemented
                    ::PolyPolygon aNOTBEZIER;
                    aVclPolyPolygon.AdaptiveSubdivide( aNOTBEZIER );
                    pAction = new internal::PolyPolyAction( aNOTBEZIER, rCanvas, rState );
                }
            }

            // if no outline mode was requested or possible then draw the normal text
            if( !pAction )
            {
                if( !pCharWidths )
                    pAction = new internal::TextAction( aStartPixel, rString,
                        nIndex, nLength, rCanvas, rState,
                        rParms.maTextTransformation );
                else
                {
                    uno::Sequence< double > aCharWidthSeq( ::comphelper::arrayToSequence<double>( pCharWidths, nLength ) );
                    // convert character widths from logical units
                    for( int i = 0; i < nLength; ++i )
                    {
                        // TODO(F2): use correct scale direction
                        const Size aSize( ::basegfx::fround( pCharWidths[i] + .5 ), 0 );
                        aCharWidthSeq[i] = rVDev.LogicToPixel( aSize ).Width();
                    }

                    pAction = new internal::TextAction( aStartPixel, rString,
                        nIndex, nLength, aCharWidthSeq, rCanvas, rState,
                        rParms.maTextTransformation );
                }
            }

            maActions.push_back( MtfAction( ActionSharedPtr( pAction ), nCurrActionIndex ) );

            if( rState.textUnderlineStyle || rState.textStrikeoutStyle )
            {
                // TODO: split text lines on word breaks
                if( rState.isTextWordUnderlineSet );    // TODO: NYI

                long nTextWidth = rVDev.GetTextWidth( rString, nIndex, nLength );
                createJustTextLinesAction( rStartPoint, nTextWidth, rVDev,
                                           rCanvas, rStates, rParms, nCurrActionIndex );
            }

            // TODO: implement text emphasis mark styles
            if( rState.textEmphasisMarkStyle );     // TODO: NYI
       }


        // create line actions for text such as underline and strikeout
        void ImplRenderer::createJustTextLinesAction(
                const Point&            rLogicalStartPoint,
                long                    nLineWidth,
                VirtualDevice&          rVDev,
                const CanvasSharedPtr&  rCanvas,
                VectorOfOutDevStates&   rStates,
                const Parameters&       rParms,
                int                     nCurrActionIndex )
        {
            pushState( rStates );
            ::cppcanvas::internal::OutDevState& rState = getState( rStates );

            // initialize the color of the text lines
            if( !rState.isTextOutlineModeSet )
            {
                // normal text lines
                rState.fillColor = rState.textColor;
                // NOTE: strangely the text line color is ignored by vcl
                // color = rState.isTextLineColorSet ? rState.textLineColor : rState.textColor;
                rState.isFillColorSet = true;
                rState.isLineColorSet = false;
            }
#if 0
            // the line and fill colors are already set by the calling method
            // when we are in outline mode, because the text rendering already needed it
            else
            {
                // text lines for text using an outline font
                rState.lineColor = rState.textColor;
                rState.isLineColorSet = true;
                // TODO: rState.fillColor = Color( COL_WHITE );
                // TODO: rState.isFillColorSet = true;
                // NOTE: rState.strokeWidth is already set by the caller
            }
#endif

            // prepare text line position and size
            ::FontMetric aMetric = rVDev.GetFontMetric();
            long nLineHeight = (aMetric.GetDescent() + 2) / 4;
            Point aUnderlineOffset( 0, aMetric.GetDescent() / 2);

            // fill the polypolygon with all text lines
            ::PolyPolygon aTextLinePolyPoly;

            ::Polygon aPolygon(4);
            switch( rState.textUnderlineStyle )
            {
                case UNDERLINE_NONE:    // nothing to do
                case UNDERLINE_DONTKNOW:
                    break;
                case UNDERLINE_DASHDOT:       // TODO(F3): NYI
                case UNDERLINE_DASHDOTDOT:    // TODO(F3): NYI
                case UNDERLINE_SMALLWAVE:     // TODO(F3): NYI
                case UNDERLINE_WAVE:          // TODO(F3): NYI
                case UNDERLINE_DOUBLEWAVE:    // TODO(F3): NYI
                case UNDERLINE_BOLDDOTTED:    // TODO(F3): NYI
                case UNDERLINE_BOLDDASH:      // TODO(F3): NYI
                case UNDERLINE_BOLDLONGDASH:  // TODO(F3): NYI
                case UNDERLINE_BOLDDASHDOT:   // TODO(F3): NYI
                case UNDERLINE_BOLDDASHDOTDOT:// TODO(F3): NYI
                case UNDERLINE_BOLDWAVE:      // TODO(F3): NYI
                case UNDERLINE_SINGLE:
                {
                    aPolygon = Polygon( Rectangle( aUnderlineOffset, Size( nLineWidth, nLineHeight ) ) );
                    aTextLinePolyPoly.Insert( aPolygon );
                    break;
                }
                case UNDERLINE_BOLD:
                {
                    aPolygon = Polygon( Rectangle( aUnderlineOffset, Size( nLineWidth, 2*nLineHeight ) ) );
                    aTextLinePolyPoly.Insert( aPolygon );
                    break;
                }
                case UNDERLINE_DOUBLE:
                {
                    aPolygon = Polygon( Rectangle( aUnderlineOffset, Size( nLineWidth, nLineHeight ) ) );
                    aPolygon.Move( 0, -nLineHeight );
                    aTextLinePolyPoly.Insert( aPolygon );
                    aPolygon.Move( 0, +nLineHeight * 2 );
                    aTextLinePolyPoly.Insert( aPolygon );
                    break;
                }
                case UNDERLINE_DOTTED:
                {
                    aPolygon = Polygon( Rectangle( aUnderlineOffset, Size( nLineHeight, nLineHeight ) ) );
                    for( int x = 0; x < nLineWidth;)
                    {
                        aTextLinePolyPoly.Insert( aPolygon );
                        aPolygon.Move( 2 * nLineHeight, 0 );
                        x += 6 * nLineHeight ;
                    }
                    break;
                }
                case UNDERLINE_DASH:
                {
                    aPolygon = Polygon( Rectangle( aUnderlineOffset, Size( 3*nLineHeight, nLineHeight ) ) );
                    for( int x = 0; x < nLineWidth;)
                    {
                        aTextLinePolyPoly.Insert( aPolygon );
                        aPolygon.Move( 6 * nLineHeight, 0 );
                        x += 6 * nLineHeight ;
                    }
                    break;
                }
                case UNDERLINE_LONGDASH:
                {
                    aPolygon = Polygon( Rectangle( aUnderlineOffset, Size( 6*nLineHeight, nLineHeight ) ) );
                    for( int x = 0; x < nLineWidth;)
                    {
                        aTextLinePolyPoly.Insert( aPolygon );
                        aPolygon.Move( 12 * nLineHeight, 0 );
                        x += 12 * nLineHeight;
                    }
                    break;
                }
            }

            switch( rState.textStrikeoutStyle )
            {
                case STRIKEOUT_NONE:    // nothing to do
                case STRIKEOUT_DONTKNOW:
                    break;
                case STRIKEOUT_SLASH:   // TODO: we should handle this in the text layer
                case STRIKEOUT_X:
                    break;
                case STRIKEOUT_SINGLE:
                {
                    Point aOffset( 0, (aMetric.GetIntLeading() - aMetric.GetAscent()) / 3 );
                    Rectangle aRect( aOffset, Size( nLineWidth, nLineHeight ) );
                    aTextLinePolyPoly.Insert( ::Polygon( aRect ) );
                    break;
                }
                case STRIKEOUT_BOLD:
                {
                    Point aOffset( 0, (aMetric.GetIntLeading() - aMetric.GetAscent()) / 3 );
                    Rectangle aRect( aOffset, Size( nLineWidth, 2 * nLineHeight ) );
                    aTextLinePolyPoly.Insert( ::Polygon( aRect ) );
                    break;
                }
                case STRIKEOUT_DOUBLE:
                {
                    Point aOffset( 0, (aMetric.GetIntLeading() - aMetric.GetAscent()) / 3 );
                    aOffset.Y() -= nLineHeight;
                    Rectangle aRect( aOffset, Size( nLineWidth, nLineHeight ) );
                    aTextLinePolyPoly.Insert( ::Polygon( aRect ) );
                    aRect.Move( 0, 2 * nLineHeight );
                    aTextLinePolyPoly.Insert( ::Polygon( aRect ) );
                    break;
                }
            }

            if( !aTextLinePolyPoly.Count() )
                return;

            // prepare text line orientation
            basegfx::B2DTuple aTScale, aTTrans;
            double fRotate, fShearX;
            rState.fontTransform.decompose( aTScale, aTTrans, fRotate, fShearX );
            fRotate = 3600 - fRotate * (1800.0 / F_PI);

            // adjust to the target coordinate system
            aTextLinePolyPoly.Rotate( Point(0,0),
                                      ::canvas::tools::numeric_cast<USHORT>( ::basegfx::fround( fRotate ) ) );
            aTextLinePolyPoly.Translate( rLogicalStartPoint );
            aTextLinePolyPoly = rVDev.LogicToPixel( aTextLinePolyPoly );

           // create the underline + strikethrough line actions
            maActions.push_back( MtfAction( ActionSharedPtr(
                new internal::PolyPolyAction( aTextLinePolyPoly, rCanvas, rState ) ),
                nCurrActionIndex ) );

            popState( rStates );
        }

        void ImplRenderer::updateClipping( VectorOfOutDevStates&            rStates,
                                           const ::basegfx::B2DPolyPolygon& rClipPoly,
                                           const CanvasSharedPtr&           rCanvas,
                                           bool                             bIntersect )
        {
            ::cppcanvas::internal::OutDevState& rState( getState( rStates ) );
            ::basegfx::B2DPolyPolygon aClipPoly( rClipPoly );

            if( !bIntersect ||
                rState.clip.count() == 0 )
            {
                rState.clip = rClipPoly;
            }
            else
            {
                rState.clip = ::basegfx::tools::correctOrientations( rState.clip );
                aClipPoly = ::basegfx::tools::correctOrientations( aClipPoly );

                // intersect the two poly-polygons
                rState.clip = ::basegfx::tools::removeAllIntersections(rState.clip);
                rState.clip = ::basegfx::tools::removeNeutralPolygons(rState.clip, sal_True);
                aClipPoly = ::basegfx::tools::removeAllIntersections(aClipPoly);
                aClipPoly = ::basegfx::tools::removeNeutralPolygons(aClipPoly, sal_True);
                rState.clip.append(aClipPoly);
                rState.clip = ::basegfx::tools::removeAllIntersections(rState.clip);
                rState.clip = ::basegfx::tools::removeNeutralPolygons(rState.clip, sal_False);
            }

            if( rState.clip.count() == 0 )
            {
                rState.xClipPoly.clear();
            }
            else
            {
                rState.xClipPoly = ::basegfx::unotools::xPolyPolygonFromB2DPolyPolygon(
                    rCanvas->getUNOCanvas()->getDevice(),
                    rState.clip );
            }
        }

        bool ImplRenderer::createActions( const CanvasSharedPtr&    rCanvas,
                                          VirtualDevice&            rVDev,
                                          GDIMetaFile&              rMtf,
                                          VectorOfOutDevStates&     rStates,
                                          const Parameters&         rParms,
                                          int&                      io_rCurrActionIndex )
        {
            /* TODO(P2): interpret mtf-comments
               ================================

               - Float-Transparency (skipped for prototype)
               - bitmap fillings (do that via comments)
               - gradient fillings (do that via comments)

               - think about mapping. _If_ we do everything in logical
                    coordinates (which would solve the probs for stroke
                 widths and text offsets), then we would have to
                 recalc scaling for every drawing operation. This is
                 because the outdev map mode might change at any time.
                 Also keep in mind, that, although we've double precision
                 float arithmetic now, different offsets might still
                 generate different roundings (aka
                 'OutputDevice::SetPixelOffset())

             */

            // Loop over every metaaction
            // ==========================
            MetaAction* pCurrAct;

            // TODO(P1): think about caching
            for( pCurrAct=rMtf.FirstAction();
                 pCurrAct;
                 ++io_rCurrActionIndex, pCurrAct = rMtf.NextAction() )
            {
                // execute every action, to keep VDev state up-to-date
                // currently used only for
                // - the map mode
                // - the line/fill color when processing a META_TRANSPARENT_ACTION
                // - SetFont to process font metric specific actions
                pCurrAct->Execute( &rVDev );

                switch( pCurrAct->GetType() )
                {
                    // ------------------------------------------------------------

                    // In the first part of this monster-switch, we
                    // handle all state-changing meta actions. These
                    // are all handled locally.

                    // ------------------------------------------------------------

                    case META_PUSH_ACTION:
                        pushState( rStates );
                        break;

                    case META_POP_ACTION:
                        popState( rStates );
                        break;

                    // monitor clip regions, to assemble clip polygon on our own
                    case META_CLIPREGION_ACTION:
                    {
                        MetaClipRegionAction* pClipAction = static_cast<MetaClipRegionAction*>(pCurrAct);

                        if( !pClipAction->IsClipping() )
                        {
                            // clear clipping
                            getState( rStates ).clip.clear();
                        }
                        else
                        {
                            if( !pClipAction->GetRegion().HasPolyPolygon() )
                            {
                                VERBOSE_TRACE( "ImplRenderer::createActions(): non-polygonal clip "
                                               "region encountered, falling back to bounding box!" );

                                Rectangle aClipRect(
                                    rVDev.LogicToPixel(
                                        pClipAction->GetRegion().GetBoundRect() ) );

                                // intersect current clip with given rect
                                updateClipping(
                                    rStates,
                                    ::basegfx::B2DPolyPolygon(
                                        ::basegfx::tools::createPolygonFromRect(
                                            ::basegfx::B2DRectangle( aClipRect.Left(),
                                                                     aClipRect.Top(),
                                                                     aClipRect.Right(),
                                                                     aClipRect.Bottom() ) ) ),
                                    rCanvas,
                                    false );
                            }
                            else
                            {
                                // set new clip polygon (don't intersect
                                // with old one, just set it)
                                updateClipping(
                                    rStates,
                                    rVDev.LogicToPixel(
                                        pClipAction->GetRegion().GetPolyPolygon() ).getB2DPolyPolygon(),
                                    rCanvas,
                                    false );
                            }
                        }

                        break;
                    }

                    case META_ISECTRECTCLIPREGION_ACTION:
                    {
                        MetaISectRectClipRegionAction* pClipAction = static_cast<MetaISectRectClipRegionAction*>(pCurrAct);
                        Rectangle aClipRect(
                            rVDev.LogicToPixel( pClipAction->GetRect() ) );

                        // intersect current clip with given rect
                        updateClipping(
                            rStates,
                            ::basegfx::B2DPolyPolygon(
                                ::basegfx::tools::createPolygonFromRect(
                                    ::basegfx::B2DRectangle( aClipRect.Left(),
                                                             aClipRect.Top(),
                                                             aClipRect.Right(),
                                                             aClipRect.Bottom() ) ) ),
                            rCanvas,
                            true );

                        break;
                    }

                    case META_ISECTREGIONCLIPREGION_ACTION:
                    {
                        MetaISectRegionClipRegionAction* pClipAction = static_cast<MetaISectRegionClipRegionAction*>(pCurrAct);

                        if( !pClipAction->GetRegion().HasPolyPolygon() )
                        {
                            VERBOSE_TRACE( "ImplRenderer::createActions(): non-polygonal clip "
                                           "region encountered, falling back to bounding box!" );

                            Rectangle aClipRect(
                                rVDev.LogicToPixel( pClipAction->GetRegion().GetBoundRect() ) );

                            // intersect current clip with given rect
                            updateClipping(
                                rStates,
                                ::basegfx::B2DPolyPolygon(
                                    ::basegfx::tools::createPolygonFromRect(
                                        ::basegfx::B2DRectangle( aClipRect.Left(),
                                                                 aClipRect.Top(),
                                                                 aClipRect.Right(),
                                                                 aClipRect.Bottom() ) ) ),
                                rCanvas,
                                true );
                        }
                        else
                        {
                            // intersect current clip with given clip polygon
                            updateClipping(
                                rStates,
                                rVDev.LogicToPixel(
                                    pClipAction->GetRegion().GetPolyPolygon() ).getB2DPolyPolygon(),
                                rCanvas,
                                true );
                        }

                        break;
                    }

                    case META_MOVECLIPREGION_ACTION:
                        // TODO(F2): NYI
                        break;

                    case META_LINECOLOR_ACTION:
                        if( !rParms.maLineColor.isValid() )
                        {
                            setStateColor( static_cast<MetaLineColorAction*>(pCurrAct),
                                           getState( rStates ).isLineColorSet,
                                           getState( rStates ).lineColor,
                                           rCanvas );
                        }
                        break;

                    case META_FILLCOLOR_ACTION:
                        if( !rParms.maFillColor.isValid() )
                        {
                            setStateColor( static_cast<MetaFillColorAction*>(pCurrAct),
                                           getState( rStates ).isFillColorSet,
                                           getState( rStates ).fillColor,
                                           rCanvas );
                        }
                        break;

                    case META_TEXTCOLOR_ACTION:
                    {
                        if( !rParms.maTextColor.isValid() )
                        {
                            // Text color is set unconditionally, thus, no
                            // use of setStateColor here
                            ::Color aColor( static_cast<MetaTextColorAction*>(pCurrAct)->GetColor() );

                            // force alpha part of color to
                            // opaque. transparent painting is done
                            // explicitely via META_TRANSPARENT_ACTION
                            aColor.SetTransparency(0);

                            getState( rStates ).textColor =
                                ::vcl::unotools::colorToDoubleSequence( rCanvas->getUNOCanvas()->getDevice(),
                                                                        aColor );
                        }
                    }
                    break;

                    case META_TEXTFILLCOLOR_ACTION:
                        if( !rParms.maTextColor.isValid() )
                        {
                            setStateColor( static_cast<MetaTextFillColorAction*>(pCurrAct),
                                           getState( rStates ).isTextFillColorSet,
                                           getState( rStates ).textFillColor,
                                           rCanvas );
                        }
                        break;

                    case META_TEXTLINECOLOR_ACTION:
                        if( !rParms.maTextColor.isValid() )
                        {
                            setStateColor( static_cast<MetaTextLineColorAction*>(pCurrAct),
                                           getState( rStates ).isTextLineColorSet,
                                           getState( rStates ).textLineColor,
                                           rCanvas );
                        }
                        break;

                    case META_TEXTALIGN_ACTION:
                        // TODO(F2): NYI
                        break;

                    case META_FONT_ACTION:
                    {
                        ::cppcanvas::internal::OutDevState& rState = getState( rStates );
                        const ::Font& rFont( static_cast<MetaFontAction*>(pCurrAct)->GetFont() );

                        rState.xFont = createFont( rState.fontTransform,
                                                   rFont,
                                                   rCanvas,
                                                   rVDev,
                                                   rParms );

                        // TODO(Q2): define and use appropriate enumeration types
                        rState.textReliefStyle          = (sal_Int8)rFont.GetRelief();
                        rState.textUnderlineStyle       = rParms.maFontUnderline.isValid() ?
                            (rParms.maFontUnderline.getValue() ? UNDERLINE_SINGLE : UNDERLINE_NONE) :
                            (sal_Int8)rFont.GetUnderline();
                        rState.textStrikeoutStyle       = (sal_Int8)rFont.GetStrikeout();
                        rState.textEmphasisMarkStyle    = (sal_Int8)rFont.GetEmphasisMark();
                        rState.isTextEffectShadowSet    = (rFont.IsShadow() != FALSE);
                        rState.isTextWordUnderlineSet   = (rFont.IsWordLineMode() != FALSE);
                        rState.isTextOutlineModeSet     = (rFont.IsOutline() != FALSE);
                    }
                    break;

                    case META_RASTEROP_ACTION:
                        // TODO(F2): NYI
                        break;

                    case META_REFPOINT_ACTION:
                        // TODO(F2): NYI
                        break;

                    case META_LAYOUTMODE_ACTION:
                    {
                        // TODO(F2): A lot is missing here
                        int nLayoutMode = static_cast<MetaLayoutModeAction*>(pCurrAct)->GetLayoutMode();
                        ::cppcanvas::internal::OutDevState& rState = getState( rStates );
                        switch( nLayoutMode & (TEXT_LAYOUT_BIDI_RTL|TEXT_LAYOUT_BIDI_STRONG) )
                        {
                            case TEXT_LAYOUT_BIDI_LTR:
                                rState.textDirection = rendering::TextDirection::WEAK_LEFT_TO_RIGHT;
                                break;

                            case (TEXT_LAYOUT_BIDI_LTR | TEXT_LAYOUT_BIDI_STRONG):
                                rState.textDirection = rendering::TextDirection::STRONG_LEFT_TO_RIGHT;
                                break;

                            case TEXT_LAYOUT_BIDI_RTL:
                                rState.textDirection = rendering::TextDirection::WEAK_RIGHT_TO_LEFT;
                                break;

                            case (TEXT_LAYOUT_BIDI_RTL | TEXT_LAYOUT_BIDI_STRONG):
                                rState.textDirection = rendering::TextDirection::STRONG_RIGHT_TO_LEFT;
                                break;
                        }

                        rState.textAlignment = 0; // TODO(F2): rendering::TextAlignment::LEFT_ALIGNED;
                        if( (nLayoutMode & (TEXT_LAYOUT_BIDI_RTL | TEXT_LAYOUT_TEXTORIGIN_RIGHT) )
                            && !(nLayoutMode & TEXT_LAYOUT_TEXTORIGIN_LEFT ) )
                        {
                            rState.textAlignment = 1; // TODO(F2): rendering::TextAlignment::RIGHT_ALIGNED;
                        }
                    }
                    break;

                    // ------------------------------------------------------------

                    // In the second part of this monster-switch, we
                    // handle all recursing meta actions. These are the
                    // ones generating a metafile by themselves, which is
                    // then processed by recursively calling this method.

                    // ------------------------------------------------------------

                    case META_GRADIENT_ACTION:
                    {
                        MetaGradientAction* pGradAct = static_cast<MetaGradientAction*>(pCurrAct);
                        createGradientAction( ::Polygon( pGradAct->GetRect() ),
                                              pGradAct->GetGradient(),
                                              rVDev,
                                              rCanvas,
                                              rStates,
                                              rParms,
                                              io_rCurrActionIndex,
                                              true );
                    }
                    break;

                    case META_HATCH_ACTION:
                    {
                        // TODO(F2): use native Canvas hatches here
                        GDIMetaFile aTmpMtf;

                        rVDev.AddHatchActions( static_cast<MetaHatchAction*>(pCurrAct)->GetPolyPolygon(),
                                               static_cast<MetaHatchAction*>(pCurrAct)->GetHatch(),
                                               aTmpMtf );
                        createActions( rCanvas, rVDev, aTmpMtf, rStates, rParms, io_rCurrActionIndex );
                    }
                    break;

                    case META_EPS_ACTION:
                    {
                        MetaEPSAction*      pAct = static_cast<MetaEPSAction*>(pCurrAct);
                        const GDIMetaFile&  pSubstitute = pAct->GetSubstitute();

                        const Size aMtfSizePix( rVDev.LogicToPixel( pSubstitute.GetPrefSize(),
                                                                    pSubstitute.GetPrefMapMode() ) );

                        // skip null-sized output
                        if( aMtfSizePix.Width() != 0 &&
                            aMtfSizePix.Height() != 0 )
                        {
                            const Point aEmptyPt;
                            const Point aMtfOriginPix( rVDev.LogicToPixel( aEmptyPt,
                                                                           pSubstitute.GetPrefMapMode() ) );

                            // Setup local transform, such that the
                            // metafile renders itself into the given
                            // output rectangle
                            pushState( rStates );

                            getState( rStates ).transform.translate( -aMtfOriginPix.X(), -aMtfOriginPix.Y() );
                            getState( rStates ).transform.scale( 1.0 / aMtfSizePix.Width(),
                                                                 1.0 / aMtfSizePix.Height() );

                            createActions( rCanvas, rVDev,
                                           const_cast<GDIMetaFile&>(pAct->GetSubstitute()),
                                           rStates, rParms,
                                           io_rCurrActionIndex );

                            popState( rStates );
                        }
                    }
                    break;

                    // handle metafile comments, to retrieve
                    // meta-information for gradients, fills and
                    // strokes. May skip actions, and may recurse.
                    case META_COMMENT_ACTION:
                    {
                        MetaCommentAction* pAct = static_cast<MetaCommentAction*>(pCurrAct);

                        // Handle gradients
                        if ( pAct->GetComment().CompareIgnoreCaseToAscii( "XGRAD_SEQ_BEGIN" ) == COMPARE_EQUAL )
                        {
                            MetaGradientExAction* pGradAction = NULL;
                            bool bDone( false );
                            while( !bDone &&
                                   (pCurrAct=rMtf.NextAction()) )
                            {
                                switch( pCurrAct->GetType() )
                                {
                                    // extract gradient info
                                    case META_GRADIENTEX_ACTION:
                                        pGradAction = static_cast<MetaGradientExAction*>(pCurrAct);
                                        break;

                                    // skip broken-down rendering, output gradient when sequence is ended
                                    case META_COMMENT_ACTION:
                                        if( static_cast<MetaCommentAction*>(pCurrAct)->GetComment().CompareIgnoreCaseToAscii( "XGRAD_SEQ_END" ) == COMPARE_EQUAL )
                                        {
                                            bDone = true;

                                            if( pGradAction )
                                            {
                                                createGradientAction( pGradAction->GetPolyPolygon(),
                                                                      pGradAction->GetGradient(),
                                                                      rVDev,
                                                                      rCanvas,
                                                                      rStates,
                                                                      rParms,
                                                                      io_rCurrActionIndex,
                                                                      false );
                                            }
                                        }
                                        break;
                                }
                            }
                        }

                        // Handle drawing layer strokes
                        else if( pAct->GetComment().Equals( "XPATHSTROKE_SEQ_BEGIN" ) )
                        {
                            // TODO(F1): Later
#if 0
                            const BYTE* pData = pAct->GetData();
                            if ( pData )
                            {
                                SvMemoryStream  aMemStm( (void*)pData, pA->GetDataSize(), STREAM_READ );

                                SvtGraphicStroke aStroke;
                                aMemStm >> aStroke;

                                // TODO(F1): respect exceptions, like
                                // start/end arrows and joins not
                                // displayable via Canvas

                                // TODO(F1): use correct scale direction, stroke
                                // width might be height or anything else
                                const Size aSize( aStroke.getStrokeWidth(), 0 );

                                internal::StrokeAction aStrokeAction( rVDev.LogicToPixel( aStroke.getPath() ),
                                                                      aStroke.getTransparency(),
                                                                      rVDev.LogicToPixel( aSize ).Width(),
                                                                      aStroke.getJoinType(),
                                                                      aStroke.getDashArray(),
                                                                      aStroke.getMiterLimit(),
                                                                      aStroke.getCapType(),
                                                                      rCanvas,
                                                                      getState( rStates ) );

                                aStrokeAction.render( rViewState );

                                // skip broken-down render output
                                skipContent( rMtf, "XPATHSTROKE_SEQ_END" );
                            }
#endif
                        }

                        // Handle drawing layer fills
                        else if( pAct->GetComment().Equals( "XPATHFILL_SEQ_BEGIN" ) )
                        {
                            // TODO(F1): Later
#if 0
                            const BYTE* pData = pAct->GetData();
                            if ( pData )
                            {
                                SvMemoryStream  aMemStm( (void*)pData, pA->GetDataSize(), STREAM_READ );

                                SvtGraphicFill aFill;
                                aMemStm >> aFill;

                                switch( aFill.getType() )
                                {
                                    case SvtGraphicFill::fillSolid:
                                    {
                                        internal::SolidFillAction aFillAction( rVDev.LogicToPixel( aFill.getPath() ),
                                                                               aFill.getFillColor(),
                                                                               aFill.getTransparency(),
                                                                               aFill.getFillRule(),
                                                                               rCanvas,
                                                                               getState( rStates ) );
                                        aFillAction.render( rViewState );
                                    }
                                    break;

                                    case SvtGraphicFill::fillGradient:
                                    {
                                        internal::GradientFillAction aFillAction( rVDev.LogicToPixel( aFill.getPath() ),
                                                                                  aFill.getTransparency(),
                                                                                  aFill.getFillRule(),
                                                                                  aFill.getTransform(),
                                                                                  aFill.getGradientType(),
                                                                                  aFill.getGradient1stColor(),
                                                                                  aFill.getGradient2ndColor(),
                                                                                  aFill.getGradientStepCount(),
                                                                                  rCanvas,
                                                                                  getState( rStates ) );
                                        aFillAction.render( rViewState );
                                    }
                                    break;

                                    case SvtGraphicFill::fillHatch:
                                    {
                                        internal::HatchedFillAction aFillAction( rVDev.LogicToPixel( aFill.getPath() ),
                                                                                 aFill.getTransparency(),
                                                                                 aFill.getFillRule(),
                                                                                 aFill.getTransform(),
                                                                                 aFill.getHatchType(),
                                                                                 aFill.getHatchColor(),
                                                                                 rCanvas,
                                                                                 getState( rStates ) );
                                        aFillAction.render( rViewState );
                                    }
                                    break;

                                    case SvtGraphicFill::fillTexture:
                                    {
                                        internal::BitmapFillAction aFillAction( rVDev.LogicToPixel( aFill.getPath() ),
                                                                                aFill.getTransparency(),
                                                                                aFill.getFillRule(),
                                                                                aFill.getTransform(),
                                                                                aFill.getFillGraphic(),
                                                                                rCanvas,
                                                                                getState( rStates ) );
                                        aFillAction.render( rViewState );
                                    }
                                    break;
                                }

                                // skip broken-down render output
                                skipContent( rMtf, "XPATHFILL_SEQ_END" );
                            }
#endif
                        }
                    }
                    break;

                    // ------------------------------------------------------------

                    // In the third part of this monster-switch, we
                    // handle all 'acting' meta actions. These are all
                    // processed by constructing function objects for
                    // them, which will later ease caching.

                    // ------------------------------------------------------------

                    case META_POINT_ACTION:
                    {
                        const OutDevState& rState( getState( rStates ) );
                        if( rState.lineColor.getLength() )
                        {
                            maActions.push_back(
                                MtfAction(
                                    ActionSharedPtr(
                                        new internal::PointAction(
                                            rVDev.LogicToPixel( static_cast<MetaPointAction*>(pCurrAct)->GetPoint() ),
                                            rCanvas,
                                            rState ) ),
                                    io_rCurrActionIndex ) );
                        }
                    }
                    break;

                    case META_PIXEL_ACTION:
                    {
                        const OutDevState& rState( getState( rStates ) );
                        if( rState.lineColor.getLength() )
                        {
                            maActions.push_back(
                                MtfAction(
                                    ActionSharedPtr(
                                        new internal::PointAction(
                                            rVDev.LogicToPixel(
                                                static_cast<MetaPixelAction*>(pCurrAct)->GetPoint() ),
                                            rCanvas,
                                            rState,
                                            static_cast<MetaPixelAction*>(pCurrAct)->GetColor() ) ),
                                    io_rCurrActionIndex ) );
                        }
                    }
                    break;

                    case META_LINE_ACTION:
                    {
                        const OutDevState& rState( getState( rStates ) );
                        if( rState.lineColor.getLength() )
                        {
                            maActions.push_back(
                                MtfAction(
                                    ActionSharedPtr(
                                        new internal::LineAction(
                                            rVDev.LogicToPixel( static_cast<MetaLineAction*>(pCurrAct)->GetStartPoint() ),
                                            rVDev.LogicToPixel( static_cast<MetaLineAction*>(pCurrAct)->GetEndPoint() ),
                                            rCanvas,
                                            rState ) ),
                                    io_rCurrActionIndex ) );
                        }
                    }
                    break;

                    case META_RECT_ACTION:
                        createFillAndStroke( ::PolyPolygon( ::Polygon( rVDev.LogicToPixel( static_cast<MetaRectAction*>(pCurrAct)->GetRect() ) ) ),
                                             rCanvas, io_rCurrActionIndex,
                                             rStates );
                        break;

                    case META_ROUNDRECT_ACTION:
                        createFillAndStroke( rVDev.LogicToPixel( Polygon( static_cast<MetaRoundRectAction*>(pCurrAct)->GetRect(),
                                                                          static_cast<MetaRoundRectAction*>(pCurrAct)->GetHorzRound(),
                                                                          static_cast<MetaRoundRectAction*>(pCurrAct)->GetVertRound() ) ),
                                             rCanvas, io_rCurrActionIndex,
                                             rStates );
                        break;

                    case META_ELLIPSE_ACTION:
                    {
                        const Rectangle& rRect = static_cast<MetaEllipseAction*>(pCurrAct)->GetRect();
                        createFillAndStroke( rVDev.LogicToPixel( Polygon( rRect.Center(),
                                                                          rRect.GetWidth() >> 1,
                                                                          rRect.GetHeight() >> 1 ) ),
                                             rCanvas, io_rCurrActionIndex,
                                             rStates );
                        break;
                    }

                    case META_ARC_ACTION:
                        createFillAndStroke( rVDev.LogicToPixel( Polygon( static_cast<MetaArcAction*>(pCurrAct)->GetRect(),
                                                                          static_cast<MetaArcAction*>(pCurrAct)->GetStartPoint(),
                                                                          static_cast<MetaArcAction*>(pCurrAct)->GetEndPoint(), POLY_ARC ) ),
                                             rCanvas, io_rCurrActionIndex,
                                             rStates );
                        break;

                    case META_PIE_ACTION:
                        createFillAndStroke( rVDev.LogicToPixel( Polygon( static_cast<MetaPieAction*>(pCurrAct)->GetRect(),
                                                                          static_cast<MetaPieAction*>(pCurrAct)->GetStartPoint(),
                                                                          static_cast<MetaPieAction*>(pCurrAct)->GetEndPoint(), POLY_PIE ) ),
                                             rCanvas, io_rCurrActionIndex,
                                             rStates );
                        break;

                    case META_CHORD_ACTION:
                        createFillAndStroke( rVDev.LogicToPixel( Polygon( static_cast<MetaChordAction*>(pCurrAct)->GetRect(),
                                                                          static_cast<MetaChordAction*>(pCurrAct)->GetStartPoint(),
                                                                          static_cast<MetaChordAction*>(pCurrAct)->GetEndPoint(), POLY_CHORD ) ),
                                             rCanvas, io_rCurrActionIndex,
                                             rStates );
                        break;

                    case META_POLYLINE_ACTION:
                    {
                        const OutDevState& rState( getState( rStates ) );
                        if( rState.lineColor.getLength() ||
                            rState.fillColor.getLength() )
                        {
                            maActions.push_back(
                                MtfAction(
                                    ActionSharedPtr(
                                        new internal::PolyPolyAction(
                                            rVDev.LogicToPixel( static_cast<MetaPolyLineAction*>(pCurrAct)->GetPolygon() ),
                                            rCanvas,
                                            rState,
                                            internal::PolyPolyAction::strokeOnly ) ),
                                    io_rCurrActionIndex ) );
                        }
                    }
                    break;

                    case META_POLYGON_ACTION:
                        createFillAndStroke( rVDev.LogicToPixel( static_cast<MetaPolygonAction*>(pCurrAct)->GetPolygon() ),
                                             rCanvas, io_rCurrActionIndex,
                                             rStates );
                        break;

                    case META_POLYPOLYGON_ACTION:
                        createFillAndStroke( rVDev.LogicToPixel( static_cast<MetaPolyPolygonAction*>(pCurrAct)->GetPolyPolygon() ),
                                             rCanvas, io_rCurrActionIndex,
                                             rStates );
                        break;

                    case META_BMP_ACTION:
                    {
                        MetaBmpAction* pAct = static_cast<MetaBmpAction*>(pCurrAct);

                        maActions.push_back(
                            MtfAction(
                                ActionSharedPtr(
                                    new internal::BitmapAction(
                                        pAct->GetBitmap(),
                                        rVDev.LogicToPixel( pAct->GetPoint() ),
                                        rCanvas,
                                        getState( rStates ) ) ),
                                io_rCurrActionIndex ) );
                    }
                    break;

                    case META_BMPSCALE_ACTION:
                    {
                        MetaBmpScaleAction* pAct = static_cast<MetaBmpScaleAction*>(pCurrAct);

                        maActions.push_back(
                            MtfAction(
                                ActionSharedPtr(
                                    new internal::BitmapAction(
                                        pAct->GetBitmap(),
                                        rVDev.LogicToPixel( pAct->GetPoint() ),
                                        rVDev.LogicToPixel( pAct->GetSize() ),
                                        rCanvas,
                                        getState( rStates ) ) ),
                                io_rCurrActionIndex ) );
                    }
                    break;

                    case META_BMPSCALEPART_ACTION:
                    {
                        MetaBmpScalePartAction* pAct = static_cast<MetaBmpScalePartAction*>(pCurrAct);

                        maActions.push_back(
                            MtfAction(
                                ActionSharedPtr(
                                    new internal::BitmapAction(
                                        pAct->GetBitmap(),
                                        pAct->GetSrcPoint(),
                                        pAct->GetSrcSize(),
                                        rVDev.LogicToPixel( pAct->GetDestPoint() ),
                                        rVDev.LogicToPixel( pAct->GetDestSize() ),
                                        rCanvas,
                                        getState( rStates ) ) ),
                                io_rCurrActionIndex ) );
                    }
                    break;

                    case META_BMPEX_ACTION:
                    {
                        MetaBmpExAction* pAct = static_cast<MetaBmpExAction*>(pCurrAct);

                        maActions.push_back(
                            MtfAction(
                                ActionSharedPtr(
                                    new internal::BitmapAction(
                                        pAct->GetBitmapEx(),
                                        rVDev.LogicToPixel( pAct->GetPoint() ),
                                        rCanvas,
                                        getState( rStates ) ) ),
                                io_rCurrActionIndex ) );
                    }
                    break;

                    case META_BMPEXSCALE_ACTION:
                    {
                        MetaBmpExScaleAction* pAct = static_cast<MetaBmpExScaleAction*>(pCurrAct);

                        maActions.push_back(
                            MtfAction(
                                ActionSharedPtr(
                                    new internal::BitmapAction(
                                        pAct->GetBitmapEx(),
                                        rVDev.LogicToPixel( pAct->GetPoint() ),
                                        rVDev.LogicToPixel( pAct->GetSize() ),
                                        rCanvas,
                                        getState( rStates ) ) ),
                                io_rCurrActionIndex ) );
                    }
                    break;

                    case META_BMPEXSCALEPART_ACTION:
                    {
                        MetaBmpExScalePartAction* pAct = static_cast<MetaBmpExScalePartAction*>(pCurrAct);

                        maActions.push_back(
                            MtfAction(
                                ActionSharedPtr(
                                    new internal::BitmapAction(
                                        pAct->GetBitmapEx(),
                                        pAct->GetSrcPoint(),
                                        pAct->GetSrcSize(),
                                        rVDev.LogicToPixel( pAct->GetDestPoint() ),
                                        rVDev.LogicToPixel( pAct->GetDestSize() ),
                                        rCanvas,
                                        getState( rStates ) ) ),
                                io_rCurrActionIndex ) );
                    }
                    break;

                    case META_MASK_ACTION:
                    {
                        MetaMaskAction* pAct = static_cast<MetaMaskAction*>(pCurrAct);

                        // TODO(F2): masking NYI. Further members: mask color
                        maActions.push_back(
                            MtfAction(
                                ActionSharedPtr(
                                    new internal::BitmapAction(
                                        pAct->GetBitmap(),
                                        rVDev.LogicToPixel( pAct->GetPoint() ),
                                        rCanvas,
                                        getState( rStates ) ) ),
                                io_rCurrActionIndex ) );
                    }
                    break;

                    case META_MASKSCALE_ACTION:
                    {
                        MetaMaskScaleAction* pAct = static_cast<MetaMaskScaleAction*>(pCurrAct);

                        // TODO(F2): masking NYI. Further members: mask color
                        maActions.push_back(
                            MtfAction(
                                ActionSharedPtr(
                                    new internal::BitmapAction(
                                        pAct->GetBitmap(),
                                        rVDev.LogicToPixel( pAct->GetPoint() ),
                                        rVDev.LogicToPixel( pAct->GetSize() ),
                                        rCanvas,
                                        getState( rStates ) ) ),
                                io_rCurrActionIndex ) );
                    }
                    break;

                    case META_MASKSCALEPART_ACTION:
                    {
                        MetaMaskScalePartAction* pAct = static_cast<MetaMaskScalePartAction*>(pCurrAct);

                        // TODO(F2): masking NYI. Further members: mask color
                        maActions.push_back(
                            MtfAction(
                                ActionSharedPtr(
                                    new internal::BitmapAction(
                                        pAct->GetBitmap(),
                                        pAct->GetSrcPoint(),
                                        pAct->GetSrcSize(),
                                        rVDev.LogicToPixel( pAct->GetDestPoint() ),
                                        rVDev.LogicToPixel( pAct->GetDestSize() ),
                                        rCanvas,
                                        getState( rStates ) ) ),
                                io_rCurrActionIndex ) );
                    }
                    break;

                    case META_GRADIENTEX_ACTION:
                        // TODO(F1): use native Canvas gradients here
                        // action is ignored here, because redundant to META_GRADIENT_ACTION
                        break;

                    case META_WALLPAPER_ACTION:
                        // TODO(F2): NYI
                        break;

                    case META_TRANSPARENT_ACTION:
                    {
                        const OutDevState& rState( getState( rStates ) );
                        if( rState.lineColor.getLength() ||
                            rState.fillColor.getLength() )
                        {
                            MetaTransparentAction* pAct = static_cast<MetaTransparentAction*>(pCurrAct);

                            maActions.push_back(
                                MtfAction(
                                    ActionSharedPtr(
                                        new internal::PolyPolyAction(
                                            rVDev.LogicToPixel( pAct->GetPolyPolygon() ),
                                            rCanvas,
                                            rState,
                                            pAct->GetTransparence() ) ),
                                    io_rCurrActionIndex ) );
                        }
                    }
                    break;

                    case META_FLOATTRANSPARENT_ACTION:
                    {
                        MetaFloatTransparentAction* pAct = static_cast<MetaFloatTransparentAction*>(pCurrAct);

                        internal::MtfAutoPtr pMtf(
                            new ::GDIMetaFile( pAct->GetGDIMetaFile() ) );

                        // TODO(P2): Use native canvas gradients here (saves a lot of UNO calls)
                        internal::GradientAutoPtr pGradient(
                            new Gradient( pAct->GetGradient() ) );

                        DBG_TESTSOLARMUTEX();

                        maActions.push_back(
                            MtfAction(
                                ActionSharedPtr(
                                    new internal::TransparencyGroupAction(
                                        pMtf,
                                        pGradient,
                                        rParms,
                                        rVDev.LogicToPixel( pAct->GetPoint() ),
                                        rVDev.LogicToPixel( pAct->GetSize() ),
                                        rCanvas,
                                        getState( rStates ) ) ),
                                io_rCurrActionIndex ) );
                    }
                    break;

                    case META_TEXT_ACTION:
                    {
                        MetaTextAction* pAct = static_cast<MetaTextAction*>(pCurrAct);
                        createTextWithEffectsAction(
                            pAct->GetPoint(),
                            pAct->GetText(),
                            pAct->GetIndex(),
                            pAct->GetLen() == (USHORT)STRING_LEN ? pAct->GetText().Len() - pAct->GetIndex() : pAct->GetLen(),
                            NULL,
                            rVDev,
                            rCanvas,
                            rStates,
                            rParms,
                            io_rCurrActionIndex );
                    }
                    break;

                    case META_TEXTARRAY_ACTION:
                    {
                        MetaTextArrayAction* pAct = static_cast<MetaTextArrayAction*>(pCurrAct);
                        createTextWithEffectsAction(
                            pAct->GetPoint(),
                            pAct->GetText(),
                            pAct->GetIndex(),
                            pAct->GetLen() == (USHORT)STRING_LEN ? pAct->GetText().Len() - pAct->GetIndex() : pAct->GetLen(),
                            pAct->GetDXArray(),
                            rVDev,
                            rCanvas,
                            rStates,
                            rParms,
                            io_rCurrActionIndex );
                    }
                    break;

                    case META_TEXTLINE_ACTION:
                    {
                        MetaTextLineAction* pAct = static_cast<MetaTextLineAction*>(pCurrAct);
                        createJustTextLinesAction(
                            pAct->GetStartPoint(),
                            pAct->GetWidth(),
                            rVDev,
                            rCanvas,
                            rStates,
                            rParms,
                            io_rCurrActionIndex );
                    }
                    break;

                    case META_TEXTRECT_ACTION:
                        // TODO(F2): NYI
                        DBG_ERROR("META_TEXTRECT not yet supported");
                        break;

                    case META_STRETCHTEXT_ACTION:
                        // TODO(F2): NYI
                        DBG_ERROR("META_STRETCHTEXT not yet supported");
                        break;

                    default:
                        break;
                }
            }

            return true;
        }

        ImplRenderer::ImplRenderer( const CanvasSharedPtr&  rCanvas,
                                    const GDIMetaFile&      rMtf,
                                    const Parameters&       rParams ) :
            CanvasGraphicHelper( rCanvas ),
            maActions()
        {
            RTL_LOGFILE_CONTEXT( aLog, "::cppcanvas::internal::ImplRenderer::ImplRenderer(mtf)" );

            OSL_ENSURE( rCanvas.get() != NULL && rCanvas->getUNOCanvas().is(),
                        "ImplRenderer::ImplRenderer(): Invalid canvas" );
            OSL_ENSURE( rCanvas->getUNOCanvas()->getDevice().is(),
                        "ImplRenderer::ImplRenderer(): Invalid graphic device" );

            // make sure canvas and graphic device are valid; action
            // creation don't check that every time
            if( rCanvas.get() == NULL ||
                !rCanvas->getUNOCanvas().is() ||
                !rCanvas->getUNOCanvas()->getDevice().is() )
            {
                // leave actions empty
                return;
            }

            VectorOfOutDevStates    aStateStack;

            VirtualDevice aVDev;
            aVDev.EnableOutput( FALSE );

            // Setup VDev for state tracking and mapping
            // =========================================

            aVDev.SetMapMode( rMtf.GetPrefMapMode() );

            const Size aMtfSize( rMtf.GetPrefSize() );
            const Size aMtfSizePix( aVDev.LogicToPixel( aMtfSize,
                                                        rMtf.GetPrefMapMode() ) );
            const Point aEmptyPt;
            const Point aMtfOriginPix( aVDev.LogicToPixel( aEmptyPt ) );

            // skip null-sized output
            if( aMtfSizePix.Width() != 0 &&
                aMtfSizePix.Height() != 0 )
            {
                // init state stack
                clearStateStack( aStateStack );

                // Setup local state, such that the metafile renders
                // itself into a one-by-one square for identity view
                // and render transformations
                getState( aStateStack ).transform.translate( -aMtfOriginPix.X(), -aMtfOriginPix.Y() );
                getState( aStateStack ).transform.scale( 1.0 / aMtfSizePix.Width(),
                                                         1.0 / aMtfSizePix.Height() );

                ColorSharedPtr pColor( getCanvas()->createColor() );

                // setup default text color to black
                getState( aStateStack ).textColor =
                    getState( aStateStack ).textFillColor =
                    getState( aStateStack ).textLineColor = pColor->getDeviceColor( 0x000000FF );

                // apply overrides from the Parameters struct
                if( rParams.maFillColor.isValid() )
                {
                    getState( aStateStack ).isFillColorSet = true;
                    getState( aStateStack ).fillColor = pColor->getDeviceColor( rParams.maFillColor.getValue() );
                }
                if( rParams.maLineColor.isValid() )
                {
                    getState( aStateStack ).isLineColorSet = true;
                    getState( aStateStack ).lineColor = pColor->getDeviceColor( rParams.maLineColor.getValue() );
                }
                if( rParams.maTextColor.isValid() )
                {
                    getState( aStateStack ).isTextFillColorSet = true;
                    getState( aStateStack ).isTextLineColorSet = true;
                    getState( aStateStack ).textColor =
                        getState( aStateStack ).textFillColor =
                        getState( aStateStack ).textLineColor = pColor->getDeviceColor( rParams.maTextColor.getValue() );
                }
                if( rParams.maFontName.isValid() ||
                    rParams.maFontWeight.isValid() ||
                    rParams.maFontLetterForm.isValid() ||
                    rParams.maFontUnderline.isValid() )
                {
                    ::cppcanvas::internal::OutDevState& rState = getState( aStateStack );

                    rState.xFont = createFont( rState.fontTransform,
                                               ::Font(), // default font
                                               rCanvas,
                                               aVDev,
                                               rParams );
                }

                int nCurrActions(0);
                createActions( rCanvas,
                               aVDev,
                               const_cast<GDIMetaFile&>(rMtf), // HACK(Q2):
                                                               // we're
                                                               // changing
                                                               // the
                                                               // current
                                                               // action
                                                               // in
                                                               // createActions!
                               aStateStack,
                               rParams,
                               nCurrActions );
            }
        }

        ImplRenderer::ImplRenderer( const CanvasSharedPtr&  rCanvas,
                                    const BitmapEx&         rBmpEx,
                                    const Parameters&       rParams ) :
            CanvasGraphicHelper( rCanvas ),
            maActions()
        {
            RTL_LOGFILE_CONTEXT( aLog, "::cppcanvas::internal::ImplRenderer::ImplRenderer(bitmap)" );

            OSL_ENSURE( rCanvas.get() != NULL && rCanvas->getUNOCanvas().is(),
                        "ImplRenderer::ImplRenderer(): Invalid canvas" );
            OSL_ENSURE( rCanvas->getUNOCanvas()->getDevice().is(),
                        "ImplRenderer::ImplRenderer(): Invalid graphic device" );

            // make sure canvas and graphic device are valid; action
            // creation don't check that every time
            if( rCanvas.get() == NULL ||
                !rCanvas->getUNOCanvas().is() ||
                !rCanvas->getUNOCanvas()->getDevice().is() )
            {
                // leave actions empty
                return;
            }

            OutDevState aState;

            const Size aBmpSize( rBmpEx.GetSizePixel() );

            // Setup local state, such that the bitmap renders itself
            // into a one-by-one square for identity view and render
            // transformations
            aState.transform.scale( 1.0 / aBmpSize.Width(),
                                    1.0 / aBmpSize.Height() );

            // create a single action for the provided BitmapEx
            maActions.push_back(
                MtfAction(
                    ActionSharedPtr( new BitmapAction(rBmpEx,
                                                      Point(),
                                                      rCanvas,
                                                      aState) ),
                    0 ) );
        }

        ImplRenderer::~ImplRenderer()
        {
        }

        namespace
        {
            class ActionRenderer
            {
            public:
                ActionRenderer( const ::basegfx::B2DHomMatrix& rTransformation ) :
                    maTransformation( rTransformation ),
                    mbRet( true )
                {
                }

                bool result()
                {
                    return mbRet;
                }

                void operator()( const ::cppcanvas::internal::ImplRenderer::MtfAction& rAction )
                {
                    // ANDing the result. We want to fail if at least
                    // one action failed.
                    mbRet &= rAction.mpAction->render( maTransformation );
                }

            private:
                const ::basegfx::B2DHomMatrix   maTransformation;
                bool                            mbRet;
            };
        }

        bool ImplRenderer::drawSubset( int  startIndex,
                                       int  endIndex ) const
        {
            RTL_LOGFILE_CONTEXT( aLog, "::cppcanvas::internal::ImplRenderer::draw()" );

            OSL_ENSURE( startIndex<=endIndex,
                        "ImplRenderer::draw() invalid action range" );

            // find start and end action
            ActionVector::const_iterator aIterBegin( ::std::lower_bound( maActions.begin(),
                                                                         maActions.end(),
                                                                         MtfAction( ActionSharedPtr(), startIndex ),
                                                                         ActionIndexComparator() ) );
            ActionVector::const_iterator aIterEnd( ::std::lower_bound( maActions.begin(),
                                                                       maActions.end(),
                                                                       MtfAction( ActionSharedPtr(), endIndex ),
                                                                       ActionIndexComparator() ) );

            ::basegfx::B2DHomMatrix aMatrix;
            ::canvas::tools::getRenderStateTransform( aMatrix, maRenderState );

            // render subset of actions
            return ::std::for_each( aIterBegin, aIterEnd, ActionRenderer( aMatrix ) ).result();
        }

        bool ImplRenderer::draw() const
        {
            RTL_LOGFILE_CONTEXT( aLog, "::cppcanvas::internal::ImplRenderer::draw()" );

            ::basegfx::B2DHomMatrix aMatrix;
            ::canvas::tools::getRenderStateTransform( aMatrix, maRenderState );

            return ::std::for_each( maActions.begin(), maActions.end(), ActionRenderer( aMatrix ) ).result();
        }
    }
}
