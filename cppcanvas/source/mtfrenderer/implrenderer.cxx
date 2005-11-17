/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: implrenderer.cxx,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: obo $ $Date: 2005-11-17 16:14:06 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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

#ifndef _COM_SUN_STAR_RENDERING_XGRAPHICDEVICE_HPP_
#include <com/sun/star/rendering/XGraphicDevice.hpp>
#endif
#ifndef _COM_SUN_STAR_RENDERING_TEXTURINGMODE_HPP_
#include <com/sun/star/rendering/TexturingMode.hpp>
#endif
#ifndef _COM_SUN_STAR_RENDERING_XPARAMETRICPOLYPOLYGON2DFACTORY_HPP_
#include <com/sun/star/rendering/XParametricPolyPolygon2DFactory.hpp>
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
#include <vcl/salbtype.hxx>

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
#include <iterator>

#include <boost/scoped_array.hpp>

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif

#ifndef _COM_SUN_STAR_GEOMETRY_REALPOINT2D_HPP__
#include <com/sun/star/geometry/RealPoint2D.hpp>
#endif
#ifndef _COM_SUN_STAR_RENDERING_VIEWSTATE_HPP__
#include <com/sun/star/rendering/ViewState.hpp>
#endif
#ifndef _COM_SUN_STAR_RENDERING_RENDERSTATE_HPP__
#include <com/sun/star/rendering/RenderState.hpp>
#endif
#ifndef _COM_SUN_STAR_RENDERING_XCANVASFONT_HPP__
#include <com/sun/star/rendering/XCanvasFont.hpp>
#endif
#ifndef _COM_SUN_STAR_RENDERING_XPOLYPOLYGON2D_HPP__
#include <com/sun/star/rendering/XPolyPolygon2D.hpp>
#endif
#ifndef _COM_SUN_STAR_RENDERING_XCANVAS_HPP__
#include <com/sun/star/rendering/XCanvas.hpp>
#endif
#ifndef _COM_SUN_STAR_RENDERING_PATHCAPTYPE_HPP__
#include <com/sun/star/rendering/PathCapType.hpp>
#endif
#ifndef _COM_SUN_STAR_RENDERING_PATHJOINTYPE_HPP__
#include <com/sun/star/rendering/PathJoinType.hpp>
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

#ifndef _SVTOOLS_GRAPHICTOOLS_HXX_
#include <svtools/graphictools.hxx>
#endif

#include "mtftools.hxx"
#include "outdevstate.hxx"


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

    void setupStrokeAttributes( rendering::StrokeAttributes& o_rStrokeAttributes,
                                const VirtualDevice&         rVDev,
                                const LineInfo&              rLineInfo              )
    {
        const Size aWidth( rLineInfo.GetWidth(), 0 );
        o_rStrokeAttributes.StrokeWidth =
            rVDev.LogicToPixel( aWidth ).Width();

        // setup reasonable defaults
        o_rStrokeAttributes.MiterLimit   = 1.0;
        o_rStrokeAttributes.StartCapType = rendering::PathCapType::BUTT;
        o_rStrokeAttributes.EndCapType   = rendering::PathCapType::BUTT;
        o_rStrokeAttributes.JoinType     = rendering::PathJoinType::MITER;

        if( LINE_DASH == rLineInfo.GetStyle() )
        {
            // interpret dash info only if explicitely enabled as
            // style
            const Size aDistance( rLineInfo.GetDistance(), 0 );
            const sal_Int32 nDistance( rVDev.LogicToPixel( aDistance ).Width() );

            const Size aDashLen( rLineInfo.GetDashLen(), 0 );
            const sal_Int32 nDashLen( rVDev.LogicToPixel( aDashLen ).Width() );

            const Size aDotLen( rLineInfo.GetDotLen(), 0 );
            const sal_Int32 nDotLen( rVDev.LogicToPixel( aDotLen ).Width() );

            const sal_Int32 nNumArryEntries( 2*rLineInfo.GetDashCount() +
                                             2*rLineInfo.GetDotCount() );

            o_rStrokeAttributes.DashArray.realloc( nNumArryEntries );
            double* pDashArray = o_rStrokeAttributes.DashArray.getArray();


            // iteratively fill dash array, first with dashs, then
            // with dots.
            // ===================================================

            sal_Int32 nCurrEntry=0;

            for( sal_Int32 i=0; i<rLineInfo.GetDashCount(); ++i )
            {
                pDashArray[nCurrEntry++] = nDashLen;
                pDashArray[nCurrEntry++] = nDistance;
            }
            for( sal_Int32 i=0; i<rLineInfo.GetDotCount(); ++i )
            {
                pDashArray[nCurrEntry++] = nDotLen;
                pDashArray[nCurrEntry++] = nDistance;
            }
        }
    }


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

    const ::cppcanvas::internal::OutDevState& getState( const ::cppcanvas::internal::VectorOfOutDevStates& rStates )
    {
        return rStates.back();
    }

    void pushState( ::cppcanvas::internal::VectorOfOutDevStates& rStates,
                    USHORT nFlags                                           )
    {
        rStates.push_back( getState( rStates ) );
        getState( rStates ).pushFlags = nFlags;
    }

    void popState( ::cppcanvas::internal::VectorOfOutDevStates& rStates )
    {
        if( getState( rStates ).pushFlags != PUSH_ALL )
        {
            // a state is pushed which is incomplete, i.e. does not
            // restore everything to the previous stack level when
            // popped.
            // That means, we take the old state, and restore every
            // OutDevState member whose flag is set, from the new to the
            // old state. Then the new state gets overwritten by the
            // calculated state

            // preset to-be-calculated new state with old state
            ::cppcanvas::internal::OutDevState aCalculatedNewState( getState( rStates ) );

            // selectively copy to-be-restored content over saved old
            // state
            rStates.pop_back();

            const ::cppcanvas::internal::OutDevState& rNewState( getState( rStates ) );

            if( (aCalculatedNewState.pushFlags & PUSH_LINECOLOR) )
            {
                aCalculatedNewState.lineColor      = rNewState.lineColor;
                aCalculatedNewState.isLineColorSet = rNewState.isLineColorSet;
            }

            if( (aCalculatedNewState.pushFlags & PUSH_FILLCOLOR) )
            {
                aCalculatedNewState.fillColor      = rNewState.fillColor;
                aCalculatedNewState.isFillColorSet = rNewState.isFillColorSet;
            }

            if( (aCalculatedNewState.pushFlags & PUSH_FONT) )
            {
                aCalculatedNewState.xFont                   = rNewState.xFont;
                aCalculatedNewState.fontRotation            = rNewState.fontRotation;
                aCalculatedNewState.textReliefStyle         = rNewState.textReliefStyle;
                aCalculatedNewState.textUnderlineStyle      = rNewState.textUnderlineStyle;
                aCalculatedNewState.textStrikeoutStyle      = rNewState.textStrikeoutStyle;
                aCalculatedNewState.textEmphasisMarkStyle   = rNewState.textEmphasisMarkStyle;
                aCalculatedNewState.isTextEffectShadowSet   = rNewState.isTextEffectShadowSet;
                aCalculatedNewState.isTextWordUnderlineSet  = rNewState.isTextWordUnderlineSet;
                aCalculatedNewState.isTextOutlineModeSet    = rNewState.isTextOutlineModeSet;
            }

            if( (aCalculatedNewState.pushFlags & PUSH_TEXTCOLOR) )
            {
                aCalculatedNewState.textColor = rNewState.textColor;
            }

            // is handled by state tracking VDev
            // if( (aCalculatedNewState.pushFlags & PUSH_MAPMODE) )
            // {
            // }

            if( (aCalculatedNewState.pushFlags & PUSH_CLIPREGION) )
            {
                aCalculatedNewState.clip        = rNewState.clip;
                aCalculatedNewState.clipRect    = rNewState.clipRect;
                aCalculatedNewState.xClipPoly   = rNewState.xClipPoly;
            }

            // TODO(F2): Raster ops NYI
            // if( (aCalculatedNewState.pushFlags & PUSH_RASTEROP) )
            // {
            // }

            if( (aCalculatedNewState.pushFlags & PUSH_TEXTFILLCOLOR) )
            {
                aCalculatedNewState.textFillColor      = rNewState.textFillColor;
                aCalculatedNewState.isTextFillColorSet = rNewState.isTextFillColorSet;
            }

            if( (aCalculatedNewState.pushFlags & PUSH_TEXTALIGN) )
            {
                aCalculatedNewState.textReferencePoint = rNewState.textReferencePoint;
            }

            // TODO(F1): Refpoint handling NYI
            // if( (aCalculatedNewState.pushFlags & PUSH_REFPOINT) )
            // {
            // }

            if( (aCalculatedNewState.pushFlags & PUSH_TEXTLINECOLOR) )
            {
                aCalculatedNewState.textLineColor      = rNewState.textLineColor;
                aCalculatedNewState.isTextLineColorSet = rNewState.isTextLineColorSet;
            }

            if( (aCalculatedNewState.pushFlags & PUSH_TEXTLAYOUTMODE) )
            {
                aCalculatedNewState.textAlignment = rNewState.textAlignment;
                aCalculatedNewState.textDirection = rNewState.textDirection;
            }

            // TODO(F2): Text language handling NYI
            // if( (aCalculatedNewState.pushFlags & PUSH_TEXTLANGUAGE) )
            // {
            // }

            // always copy push mode
            aCalculatedNewState.pushFlags = rNewState.pushFlags;

            // flush to stack
            getState( rStates ) = aCalculatedNewState;
        }
        else
        {
            rStates.pop_back();
        }
    }

    /** Create masked BitmapEx, where the white areas of rBitmap are
        transparent, and the other appear in rMaskColor.
     */
    BitmapEx createMaskBmpEx( const Bitmap&  rBitmap,
                              const ::Color& rMaskColor )
    {
        const ::Color aWhite( COL_WHITE );
        BitmapPalette aBiLevelPalette(2);
        aBiLevelPalette[0] = aWhite;
        aBiLevelPalette[1] = rMaskColor;

        Bitmap aMask( rBitmap.CreateMask( aWhite ));
        Bitmap aSolid( rBitmap.GetSizePixel(),
                       1,
                       &aBiLevelPalette );
        aSolid.Erase( rMaskColor );

        return BitmapEx( aSolid, aMask );
    }
}


namespace cppcanvas
{
    namespace internal
    {
        bool ImplRenderer::createFillAndStroke( const ::PolyPolygon&        rPolyPoly,
                                                const CanvasSharedPtr&      rCanvas,
                                                sal_Int32&                  rActionIndex,
                                                const VectorOfOutDevStates& rStates )
        {
            const OutDevState& rState( getState( rStates ) );
            if( (!rState.isLineColorSet &&
                 !rState.isFillColorSet) ||
                (rState.lineColor.getLength() == 0 &&
                 rState.fillColor.getLength() == 0) )
            {
                return false;
            }

            ActionSharedPtr pPolyAction(
                internal::PolyPolyActionFactory::createPolyPolyAction(
                    rPolyPoly, rCanvas, rState ) );

            if( pPolyAction )
            {
                maActions.push_back(
                    MtfAction(
                        pPolyAction,
                        rActionIndex ) );

                rActionIndex += pPolyAction->getActionCount()-1;
            }

            return true;
        }

        void ImplRenderer::skipContent( GDIMetaFile& rMtf,
                                        const char*  pCommentString,
                                        sal_Int32&   io_rCurrActionIndex ) const
        {
            ENSURE_AND_THROW( pCommentString,
                              "ImplRenderer::skipContent(): NULL string given" );

            MetaAction* pCurrAct;
            while( (pCurrAct=rMtf.NextAction()) )
            {
                // increment action index, we've skipped an action.
                ++io_rCurrActionIndex;

                if( pCurrAct->GetType() == META_COMMENT_ACTION &&
                    static_cast<MetaCommentAction*>(pCurrAct)->GetComment().CompareIgnoreCaseToAscii(
                        pCommentString ) == COMPARE_EQUAL )
                {
                    // requested comment found, done
                    return;
                }
            }

            // EOF
            return;
        }

        bool ImplRenderer::isActionContained( GDIMetaFile& rMtf,
                                              const char*  pCommentString,
                                              USHORT       nType ) const
        {
            ENSURE_AND_THROW( pCommentString,
                              "ImplRenderer::isActionContained(): NULL string given" );

            bool bRet( false );

            // at least _one_ call to GDIMetaFile::NextAction() is
            // executed
            ULONG nPos( 1 );

            MetaAction* pCurrAct;
            while( (pCurrAct=rMtf.NextAction()) )
            {
                if( pCurrAct->GetType() == nType )
                {
                    bRet = true; // action type found
                    break;
                }

                if( pCurrAct->GetType() == META_COMMENT_ACTION &&
                    static_cast<MetaCommentAction*>(pCurrAct)->GetComment().CompareIgnoreCaseToAscii(
                        pCommentString ) == COMPARE_EQUAL )
                {
                    // delimiting end comment found, done
                    bRet = false; // not yet found
                    break;
                }

                ++nPos;
            }

            // rewind metafile to previous position (this method must
            // not change the current metaaction)
            while( nPos-- )
                rMtf.WindPrev();

            if( !pCurrAct )
            {
                // EOF, and not yet found
                bRet = false;
            }

            return bRet;
        }

        void ImplRenderer::createGradientAction( const ::PolyPolygon&           rPoly,
                                                 const ::Gradient&              rGradient,
                                                 ::VirtualDevice&               rVDev,
                                                 const CanvasSharedPtr&         rCanvas,
                                                 VectorOfOutDevStates&          rStates,
                                                 const Parameters&              rParms,
                                                 sal_Int32&                     io_rCurrActionIndex,
                                                 bool                           bIsPolygonRectangle,
                                                 bool                           bSubsettableActions )
        {
            DBG_TESTSOLARMUTEX();

            ::PolyPolygon aDevicePoly( rVDev.LogicToPixel( rPoly ) );

            // decide, whether this gradient can be rendered natively
            // by the canvas, or must be emulated via VCL gradient
            // action extraction.
            const USHORT nSteps( rGradient.GetSteps() );

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
                                    aTexture.Gradient = xFactory->createLinearHorizontalGradient( aStartColor,
                                                                                                  aEndColor );
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

                                    // enlarge gradient to match bound rect diagonal
                                    aTextureTransformation.translate( -0.5, -0.5 );
                                    const double nScale( hypot(aBounds.GetWidth(), aBounds.GetHeight()) / nScaleX );
                                    aTextureTransformation.scale( nScale, nScale );
                                    aTextureTransformation.translate( 0.5, 0.5 );

                                    aTexture.Gradient = xFactory->createEllipticalGradient( aEndColor,
                                                                                            aStartColor,
                                                                                            geometry::RealRectangle2D(0.0,0.0,
                                                                                                                    1.0,1.0) );
                                }
                                break;

                                case GRADIENT_ELLIPTICAL:
                                {
                                    // enlarge gradient slightly
                                    aTextureTransformation.translate( -0.5, -0.5 );
                                    const double nSqrt2( sqrt(2.0) );
                                    aTextureTransformation.scale( nSqrt2,nSqrt2 );
                                    aTextureTransformation.translate( 0.5, 0.5 );

                                    aTexture.Gradient = xFactory->createEllipticalGradient( aEndColor,
                                                                                            aStartColor,
                                                                                            geometry::RealRectangle2D( aBounds.Left(),
                                                                                                                       aBounds.Top(),
                                                                                                                       aBounds.Right(),
                                                                                                                       aBounds.Bottom() ) );
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

                    // As the texture coordinate space is relative to
                    // the polygon coordinate space (NOT to the
                    // polygon itself), move gradient to the start of
                    // the actual polygon. If we skip this, the
                    // gradient will always display at the origin, and
                    // not within the polygon bound (which might be
                    // miles away from the origin).
                    aTextureTransformation.translate( aBounds.Left(),
                                                      aBounds.Top() );

                    ::basegfx::unotools::affineMatrixFromHomMatrix( aTexture.AffineTransform,
                                                                    aTextureTransformation );

                    ActionSharedPtr pPolyAction(
                        internal::PolyPolyActionFactory::createPolyPolyAction(
                            aDevicePoly,
                            rCanvas,
                            getState( rStates ),
                            aTexture ) );

                    if( pPolyAction )
                    {
                        maActions.push_back(
                            MtfAction(
                                pPolyAction,
                                io_rCurrActionIndex ) );

                        io_rCurrActionIndex += pPolyAction->getActionCount()-1;
                    }

                    // done, using native gradients
                    return;
                }
            }

            // cannot currently use native canvas gradients, as a
            // finite step size is given (this funny feature is not
            // supported by the XCanvas API)
            pushState( rStates, PUSH_ALL );

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

            createActions( rCanvas, rVDev, aTmpMtf, rStates,
                           rParms, bSubsettableActions,
                           io_rCurrActionIndex );

            popState( rStates );
        }

        uno::Reference< rendering::XCanvasFont > ImplRenderer::createFont( double&                  o_rFontRotation,
                                                                           const ::Font&            rFont,
                                                                           const CanvasSharedPtr&   rCanvas,
                                                                           const ::VirtualDevice&   rVDev,
                                                                           const Parameters&        rParms ) const
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

            // setup state-local text transformation,
            // if the font be rotated
            const short nFontAngle( rFont.GetOrientation() );
            if( nFontAngle != 0 )
            {
                // set to unity transform rotated by font angle
                const double nAngle( nFontAngle * (F_PI / 1800.0) );
                o_rFontRotation = -nAngle;
            }
            else
            {
                o_rFontRotation = 0.0;
            }

            geometry::Matrix2D aFontMatrix;
            ::canvas::tools::setIdentityMatrix2D( aFontMatrix );

            // TODO(F2): use correct scale direction, font
            // height might be width or anything else
            const ::Size rFontSizeLog( rFont.GetSize() );
            const sal_Int32 nFontWidthLog = rFontSizeLog.Width();
            if( nFontWidthLog != 0 )
            {
                ::Font aTestFont = rFont;
                aTestFont.SetWidth( 0 );
                sal_Int32 nNormalWidth = rVDev.GetFontMetric( aTestFont ).GetWidth();
                if( nNormalWidth != nFontWidthLog )
                    if( nNormalWidth )
                        aFontMatrix.m00 = (double)nFontWidthLog / nNormalWidth;
            }

            // #i52608# apply map mode scale also to font matrix - an
            // anisotrophic mapmode must be reflected in an
            // anisotrophic font matrix scale.
            const ::Size& rScale10000(
                rVDev.LogicToPixel( Size(100000L,
                                         100000L) ) );
            const sal_Int32 nWidth( rScale10000.Width() );
            const sal_Int32 nHeight( rScale10000.Height() );
            if( nWidth != nHeight )
            {
                // note: no reason to check for division by zero, we
                // always have the value closer (or equal) to zero as
                // the nominator.
                if( abs(nWidth) < abs(nHeight) )
                    aFontMatrix.m00 *= (double)nWidth / nHeight;
                else
                    aFontMatrix.m11 *= (double)nHeight / nWidth;
            }

            aFontRequest.CellSize = rVDev.LogicToPixel( rFontSizeLog ).Height();

            return rCanvas->getUNOCanvas()->createFont( aFontRequest,
                                                        uno::Sequence< beans::PropertyValue >(),
                                                        aFontMatrix );
        }

        // create text effects such as shadow/relief/embossed
        void ImplRenderer::createTextAction( const ::Point&         rStartPoint,
                                             const String           rString,
                                             int                    nIndex,
                                             int                    nLength,
                                             const sal_Int32*       pCharWidths,
                                             ::VirtualDevice&       rVDev,
                                             const CanvasSharedPtr& rCanvas,
                                             VectorOfOutDevStates&  rStates,
                                             const Parameters&      rParms,
                                             bool                   bSubsettableActions,
                                             sal_Int32&             io_rCurrActionIndex )
        {
            ENSURE_AND_THROW( nIndex >= 0 && nLength <= rString.Len() + nIndex,
                              "ImplRenderer::createTextWithEffectsAction(): Invalid text index" );

            if( !nLength )
                return; // zero-length text, no visible output

            const OutDevState& rState( getState( rStates ) );

            // TODO(F2): implement all text effects
            if( rState.textAlignment );             // TODO(F2): NYI

            ::Color aShadowColor( COL_AUTO );
            ::Color aReliefColor( COL_AUTO );
            ::Size  aShadowOffset;
            ::Size  aReliefOffset;

            if( rState.isTextEffectShadowSet )
            {
                // calculate shadow offset (similar to outdev3.cxx)
                // TODO(F3): better match with outdev3.cxx
                sal_Int32 nShadowOffset = static_cast<sal_Int32>(1.5 + ((rVDev.GetFont().GetHeight()-24.0)/24.0));
                if( nShadowOffset < 1 )
                    nShadowOffset = 1;

                aShadowOffset.setWidth( nShadowOffset );
                aShadowOffset.setHeight( nShadowOffset );

                // determine shadow color (from outdev3.cxx)
                ::Color aTextColor = ::vcl::unotools::sequenceToColor(
                    rCanvas->getUNOCanvas()->getDevice(), rState.textColor );
                bool bIsDark = (aTextColor.GetColor() == COL_BLACK)
                    || (aTextColor.GetLuminance() < 8);

                aShadowColor = bIsDark ? COL_LIGHTGRAY : COL_BLACK;
                aShadowColor.SetTransparency( aTextColor.GetTransparency() );
            }

            if( rState.textReliefStyle )
            {
                // calculate relief offset (similar to outdev3.cxx)
                sal_Int32 nReliefOffset = rVDev.PixelToLogic( Size( 1, 1 ) ).Height();
                nReliefOffset += nReliefOffset/2;
                if( nReliefOffset < 1 )
                    nReliefOffset = 1;

                if( rState.textReliefStyle == RELIEF_ENGRAVED )
                    nReliefOffset = -nReliefOffset;

                aReliefOffset.setWidth( nReliefOffset );
                aReliefOffset.setHeight( nReliefOffset );

                // determine relief color (from outdev3.cxx)
                ::Color aTextColor = ::vcl::unotools::sequenceToColor(
                    rCanvas->getUNOCanvas()->getDevice(),
                    rState.textColor );

                aReliefColor = ::Color( COL_LIGHTGRAY );

                // we don't have a automatic color, so black is always
                // drawn on white (literally copied from
                // vcl/source/gdi/outdev3.cxx)
                if( aTextColor.GetColor() == COL_BLACK )
                {
                    aTextColor = ::Color( COL_WHITE );
                    getState( rStates ).textColor =
                        ::vcl::unotools::colorToDoubleSequence( rCanvas->getUNOCanvas()->getDevice(),
                                                                aTextColor );
                }

                if( aTextColor.GetColor() == COL_WHITE )
                    aReliefColor = ::Color( COL_BLACK );
                aReliefColor.SetTransparency( aTextColor.GetTransparency() );
            }

            // create the actual text action
            ActionSharedPtr pTextAction(
                TextActionFactory::createTextAction(
                    rStartPoint,
                    aReliefOffset,
                    aReliefColor,
                    aShadowOffset,
                    aShadowColor,
                    rString,
                    nIndex,
                    nLength,
                    pCharWidths,
                    rVDev,
                    rCanvas,
                    rState,
                    rParms,
                    bSubsettableActions ) );

            if( pTextAction )
            {
                maActions.push_back(
                    MtfAction(
                        pTextAction,
                        io_rCurrActionIndex ) );

                io_rCurrActionIndex += pTextAction->getActionCount()-1;
            }
        }

        void ImplRenderer::updateClipping( VectorOfOutDevStates&            rStates,
                                           const ::basegfx::B2DPolyPolygon& rClipPoly,
                                           const CanvasSharedPtr&           rCanvas,
                                           bool                             bIntersect )
        {
            ::cppcanvas::internal::OutDevState& rState( getState( rStates ) );
            ::basegfx::B2DPolyPolygon aClipPoly( rClipPoly );

            const bool bEmptyClipRect( rState.clipRect.IsEmpty() );
            const bool bEmptyClipPoly( rState.clip.count() == 0 );

            ENSURE_AND_THROW( bEmptyClipPoly || bEmptyClipRect,
                              "ImplRenderer::updateClipping(): Clip rect and polygon are both set!" );

            if( !bIntersect ||
                (bEmptyClipRect && bEmptyClipPoly) )
            {
                rState.clip = rClipPoly;
            }
            else
            {
                if( !bEmptyClipRect )
                {
                    // TODO(P3): Use Liang-Barsky polygon clip here,
                    // after all, one object is just a rectangle!

                    // convert rect to polygon beforehand, must revert
                    // to general polygon clipping here.
                    rState.clip = ::basegfx::B2DPolyPolygon(
                        ::basegfx::tools::createPolygonFromRect(
                            // #121100# VCL rectangular clips always
                            // include one more pixel to the right
                            // and the bottom
                            ::basegfx::B2DRectangle( rState.clipRect.Left(),
                                                     rState.clipRect.Top(),
                                                     rState.clipRect.Right()+1,
                                                     rState.clipRect.Bottom()+1 ) ) );
                }

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

            // by now, our clip resides in the OutDevState::clip
            // poly-polygon.
            rState.clipRect.SetEmpty();

            if( rState.clip.count() == 0 )
            {
                if( rState.clipRect.IsEmpty() )
                {
                    rState.xClipPoly.clear();
                }
                else
                {
                    rState.xClipPoly = ::basegfx::unotools::xPolyPolygonFromB2DPolyPolygon(
                        rCanvas->getUNOCanvas()->getDevice(),
                        ::basegfx::B2DPolyPolygon(
                            ::basegfx::tools::createPolygonFromRect(
                                // #121100# VCL rectangular clips
                                // always include one more pixel to
                                // the right and the bottom
                                ::basegfx::B2DRectangle( rState.clipRect.Left(),
                                                         rState.clipRect.Top(),
                                                         rState.clipRect.Right()+1,
                                                         rState.clipRect.Bottom()+1 ) ) ) );
                }
            }
            else
            {
                rState.xClipPoly = ::basegfx::unotools::xPolyPolygonFromB2DPolyPolygon(
                    rCanvas->getUNOCanvas()->getDevice(),
                    rState.clip );
            }
        }

        void ImplRenderer::updateClipping( VectorOfOutDevStates&    rStates,
                                           const ::Rectangle&       rClipRect,
                                           const CanvasSharedPtr&   rCanvas,
                                           bool                     bIntersect )
        {
            ::cppcanvas::internal::OutDevState& rState( getState( rStates ) );

            const bool bEmptyClipRect( rState.clipRect.IsEmpty() );
            const bool bEmptyClipPoly( rState.clip.count() == 0 );

            ENSURE_AND_THROW( bEmptyClipPoly || bEmptyClipRect,
                              "ImplRenderer::updateClipping(): Clip rect and polygon are both set!" );

            if( !bIntersect ||
                (bEmptyClipRect && bEmptyClipPoly) )
            {
                rState.clipRect = rClipRect;
                rState.clip.clear();
            }
            else if( bEmptyClipPoly )
            {
                rState.clipRect.Intersection( rClipRect );
                rState.clip.clear();
            }
            else
            {
                // TODO(P3): Handle a fourth case here, when all clip
                // polygons are rectangular, once B2DMultiRange's
                // sweep line implementation is done.

                // general case: convert to polygon and clip
                // -----------------------------------------

                // convert rect to polygon beforehand, must revert
                // to general polygon clipping here.
                ::basegfx::B2DPolyPolygon aClipPoly(
                    ::basegfx::tools::createPolygonFromRect(
                        ::basegfx::B2DRectangle( rClipRect.Left(),
                                                 rClipRect.Top(),
                                                 rClipRect.Right(),
                                                 rClipRect.Bottom() ) ) );

                rState.clipRect.SetEmpty();
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
                if( rState.clipRect.IsEmpty() )
                {
                    rState.xClipPoly.clear();
                }
                else
                {
                    rState.xClipPoly = ::basegfx::unotools::xPolyPolygonFromB2DPolyPolygon(
                        rCanvas->getUNOCanvas()->getDevice(),
                        ::basegfx::B2DPolyPolygon(
                            ::basegfx::tools::createPolygonFromRect(
                                // #121100# VCL rectangular clips
                                // always include one more pixel to
                                // the right and the bottom
                                ::basegfx::B2DRectangle( rState.clipRect.Left(),
                                                         rState.clipRect.Top(),
                                                         rState.clipRect.Right()+1,
                                                         rState.clipRect.Bottom()+1 ) ) ) );
                }
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
                                          bool                      bSubsettableActions,
                                          sal_Int32&                io_rCurrActionIndex )
        {
            /* TODO(P2): interpret mtf-comments
               ================================

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
                 pCurrAct = rMtf.NextAction() )
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
                    {
                        MetaPushAction* pPushAction = static_cast<MetaPushAction*>(pCurrAct);
                        pushState( rStates,
                                   pPushAction->GetFlags() );
                    }
                    break;

                    case META_POP_ACTION:
                        popState( rStates );
                        break;

                    case META_MAPMODE_ACTION:
                        // Currently ignored, only affects rVDev (for
                        // which it _is_ executed)
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
                                    aClipRect,
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
                            aClipRect,
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
                                aClipRect,
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
                    {
                        ::cppcanvas::internal::OutDevState& rState = getState( rStates );
                        const TextAlign eTextAlign( static_cast<MetaTextAlignAction*>(pCurrAct)->GetTextAlign() );

                        rState.textReferencePoint = eTextAlign;
                    }
                    break;

                    case META_FONT_ACTION:
                    {
                        ::cppcanvas::internal::OutDevState& rState = getState( rStates );
                        const ::Font& rFont( static_cast<MetaFontAction*>(pCurrAct)->GetFont() );

                        rState.xFont = createFont( rState.fontRotation,
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

                    case META_TEXTLANGUAGE_ACTION:
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
                                              true,
                                              bSubsettableActions );
                    }
                    break;

                    case META_HATCH_ACTION:
                    {
                        // TODO(F2): use native Canvas hatches here
                        GDIMetaFile aTmpMtf;

                        rVDev.AddHatchActions( static_cast<MetaHatchAction*>(pCurrAct)->GetPolyPolygon(),
                                               static_cast<MetaHatchAction*>(pCurrAct)->GetHatch(),
                                               aTmpMtf );
                        createActions( rCanvas, rVDev, aTmpMtf, rStates,
                                       rParms, bSubsettableActions,
                                       io_rCurrActionIndex );
                    }
                    break;

                    case META_EPS_ACTION:
                    {
                        MetaEPSAction*      pAct = static_cast<MetaEPSAction*>(pCurrAct);
                        const GDIMetaFile&  rSubstitute = pAct->GetSubstitute();

                        const Size aMtfSize( rSubstitute.GetPrefSize() );
                        const Size aMtfSizePixPre( rVDev.LogicToPixel( aMtfSize,
                                                                       rSubstitute.GetPrefMapMode() ) );

                        // #i44110# correct null-sized output - there
                        // are metafiles which have zero size in at
                        // least one dimension
                        const Size aMtfSizePix( ::std::max( aMtfSizePixPre.Width(), 1L ),
                                                ::std::max( aMtfSizePixPre.Height(), 1L ) );

                        const Point aEmptyPt;
                        const Point aMtfOriginPix( rVDev.LogicToPixel( aEmptyPt,
                                                                       rSubstitute.GetPrefMapMode() ) );

                        // Setup local transform, such that the
                        // metafile renders itself into the given
                        // output rectangle
                        pushState( rStates, PUSH_ALL );

                        const ::Point& rPos( rVDev.LogicToPixel( pAct->GetPoint() ) );
                        const ::Size&  rSize( rVDev.LogicToPixel( pAct->GetSize() ) );

                        getState( rStates ).transform.translate( rPos.X(),
                                                                 rPos.Y() );
                        getState( rStates ).transform.scale( (double)rSize.Width() / aMtfSizePix.Width(),
                                                             (double)rSize.Height() / aMtfSizePix.Height() );

                        rVDev.Push();
                        rVDev.SetMapMode( rSubstitute.GetPrefMapMode() );

                        createActions( rCanvas, rVDev,
                                       const_cast<GDIMetaFile&>(pAct->GetSubstitute()),
                                       rStates, rParms,
                                       bSubsettableActions,
                                       io_rCurrActionIndex );

                        rVDev.Pop();
                        popState( rStates );
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
                                                                      false,
                                                                      bSubsettableActions );
                                            }
                                        }
                                        break;
                                }
                            }
                        }
                        // TODO(P2): Handle drawing layer strokes, via
                        // XPATHSTROKE_SEQ_BEGIN comment

                        // Handle drawing layer fills
                        else if( pAct->GetComment().Equals( "XPATHFILL_SEQ_BEGIN" ) )
                        {
                            const BYTE* pData = pAct->GetData();
                            if ( pData )
                            {
                                SvMemoryStream  aMemStm( (void*)pData, pAct->GetDataSize(), STREAM_READ );

                                SvtGraphicFill aFill;
                                aMemStm >> aFill;

                                // TODO(P2): Also handle gradients and
                                // hatches like this

                                // only evaluate comment for pure
                                // bitmap fills. If a transparency
                                // gradient is involved (denoted by
                                // the FloatTransparent action), take
                                // the normal meta actions.
                                if( aFill.getFillType() == SvtGraphicFill::fillTexture &&
                                    !isActionContained( rMtf,
                                                       "XPATHFILL_SEQ_END",
                                                        META_FLOATTRANSPARENT_ACTION ) )
                                {
                                    rendering::Texture aTexture;

                                    // TODO(F1): the SvtGraphicFill
                                    // can also transport metafiles
                                    // here, handle that case, too
                                    Graphic aGraphic;
                                    aFill.getGraphic( aGraphic );

                                    BitmapEx     aBmpEx( aGraphic.GetBitmapEx() );
                                    const ::Size aBmpSize( aBmpEx.GetSizePixel() );

                                    ::SvtGraphicFill::Transform aTransform;
                                    aFill.getTransform( aTransform );

                                    ::basegfx::B2DHomMatrix aMatrix;

                                    // convert to basegfx matrix
                                    aMatrix.set(0,0, aTransform.matrix[ 0 ] );
                                    aMatrix.set(0,1, aTransform.matrix[ 1 ] );
                                    aMatrix.set(0,2, aTransform.matrix[ 2 ] );
                                    aMatrix.set(1,0, aTransform.matrix[ 3 ] );
                                    aMatrix.set(1,1, aTransform.matrix[ 4 ] );
                                    aMatrix.set(1,2, aTransform.matrix[ 5 ] );

                                    ::basegfx::B2DHomMatrix aScale;
                                    aScale.scale( aBmpSize.Width(),
                                                  aBmpSize.Height() );

                                    // post-multiply with the bitmap
                                    // size (XCanvas' texture assumes
                                    // the given bitmap to be
                                    // normalized to [0,1]x[0,1]
                                    // rectangle)
                                    aMatrix = aMatrix * aScale;

                                    // pre-multiply with the
                                    // logic-to-pixel scale factor
                                    // (the metafile comment works in
                                    // logical coordinates).
                                    ::basegfx::B2DHomMatrix aLogic2PixelTransform;
                                    aMatrix *= tools::calcLogic2PixelLinearTransform( aLogic2PixelTransform,
                                                                                      rVDev );

                                    ::basegfx::unotools::affineMatrixFromHomMatrix(
                                        aTexture.AffineTransform,
                                        aMatrix );

                                    aTexture.Alpha = 1.0 - aFill.getTransparency();
                                    aTexture.Bitmap =
                                        ::vcl::unotools::xBitmapFromBitmapEx(
                                            rCanvas->getUNOCanvas()->getDevice(),
                                            aBmpEx );
                                    aTexture.RepeatModeX = rendering::TexturingMode::REPEAT;
                                    aTexture.RepeatModeY = rendering::TexturingMode::REPEAT;

                                    ::PolyPolygon aPath;
                                    aFill.getPath( aPath );

                                    ActionSharedPtr pPolyAction(
                                        internal::PolyPolyActionFactory::createPolyPolyAction(
                                            rVDev.LogicToPixel( aPath ),
                                            rCanvas,
                                            getState( rStates ),
                                            aTexture ) );

                                    if( pPolyAction )
                                    {
                                        maActions.push_back(
                                            MtfAction(
                                                pPolyAction,
                                                io_rCurrActionIndex ) );

                                        io_rCurrActionIndex += pPolyAction->getActionCount()-1;
                                    }

                                    // skip broken-down render output
                                    skipContent( rMtf,
                                                 "XPATHFILL_SEQ_END",
                                                 io_rCurrActionIndex );
                                }
                            }
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
                            ActionSharedPtr pPointAction(
                                internal::PointActionFactory::createPointAction(
                                    rVDev.LogicToPixel( static_cast<MetaPointAction*>(pCurrAct)->GetPoint() ),
                                    rCanvas,
                                    rState ) );

                            if( pPointAction )
                            {
                                maActions.push_back(
                                    MtfAction(
                                        pPointAction,
                                        io_rCurrActionIndex ) );

                                io_rCurrActionIndex += pPointAction->getActionCount()-1;
                            }
                        }
                    }
                    break;

                    case META_PIXEL_ACTION:
                    {
                        const OutDevState& rState( getState( rStates ) );
                        if( rState.lineColor.getLength() )
                        {
                            ActionSharedPtr pPointAction(
                                internal::PointActionFactory::createPointAction(
                                    rVDev.LogicToPixel(
                                        static_cast<MetaPixelAction*>(pCurrAct)->GetPoint() ),
                                    rCanvas,
                                    rState,
                                    static_cast<MetaPixelAction*>(pCurrAct)->GetColor() ) );

                            if( pPointAction )
                            {
                                maActions.push_back(
                                    MtfAction(
                                        pPointAction,
                                        io_rCurrActionIndex ) );

                                io_rCurrActionIndex += pPointAction->getActionCount()-1;
                            }
                        }
                    }
                    break;

                    case META_LINE_ACTION:
                    {
                        const OutDevState& rState( getState( rStates ) );
                        if( rState.lineColor.getLength() )
                        {
                            MetaLineAction* pLineAct = static_cast<MetaLineAction*>(pCurrAct);

                            const LineInfo& rLineInfo( pLineAct->GetLineInfo() );

                            ::Point aPoints[2];
                            aPoints[0] = rVDev.LogicToPixel( pLineAct->GetStartPoint() );
                            aPoints[1] = rVDev.LogicToPixel( pLineAct->GetEndPoint() );

                            ActionSharedPtr pLineAction;

                            if( rLineInfo.IsDefault() )
                            {
                                // plain hair line
                                pLineAction =
                                    internal::LineActionFactory::createLineAction(
                                        aPoints[0],
                                        aPoints[1],
                                        rCanvas,
                                        rState );

                                if( pLineAction )
                                {
                                    maActions.push_back(
                                        MtfAction(
                                            pLineAction,
                                            io_rCurrActionIndex ) );

                                    io_rCurrActionIndex += pLineAction->getActionCount()-1;
                                }
                            }
                            else if( LINE_NONE != rLineInfo.GetStyle() )
                            {
                                // 'thick' line
                                rendering::StrokeAttributes aStrokeAttributes;

                                setupStrokeAttributes( aStrokeAttributes,
                                                       rVDev,
                                                       rLineInfo );

                                // XCanvas can only stroke polygons,
                                // not simple lines - thus, handle
                                // this case via the polypolygon
                                // action
                                pLineAction =
                                    internal::PolyPolyActionFactory::createPolyPolyAction(
                                        ::PolyPolygon(
                                            ::Polygon( 2, aPoints ) ),
                                        rCanvas, rState, aStrokeAttributes );

                                if( pLineAction )
                                {
                                    maActions.push_back(
                                        MtfAction(
                                            pLineAction,
                                            io_rCurrActionIndex ) );

                                    io_rCurrActionIndex += pLineAction->getActionCount()-1;
                                }
                            }
                            // else: line style is default
                            // (i.e. invisible), don't generate action
                        }
                    }
                    break;

                    case META_RECT_ACTION:
                    {
                        Rectangle aPixelRect(
                            rVDev.LogicToPixel( static_cast<MetaRectAction*>(pCurrAct)->GetRect() ) );

                        // #121100# OutputDevice::DrawRect() fills
                        // rectangles Apple-like, i.e. with one
                        // additional pixel to the right and bottom.
                        aPixelRect.setWidth( aPixelRect.getWidth()+1 );
                        aPixelRect.setHeight( aPixelRect.getHeight()+1 );

                        createFillAndStroke( ::PolyPolygon(
                                                 ::Polygon( aPixelRect ) ),
                                              rCanvas, io_rCurrActionIndex,
                                              rStates );
                        break;
                    }

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
                            MetaPolyLineAction* pPolyLineAct = static_cast<MetaPolyLineAction*>(pCurrAct);

                            const LineInfo& rLineInfo( pPolyLineAct->GetLineInfo() );
                            ::Polygon       aPoly( rVDev.LogicToPixel(
                                                       pPolyLineAct->GetPolygon() ) );

                            ActionSharedPtr pLineAction;

                            if( rLineInfo.IsDefault() )
                            {
                                // plain hair line polygon
                                pLineAction =
                                    internal::PolyPolyActionFactory::createLinePolyPolyAction(
                                        aPoly,
                                        rCanvas,
                                        rState );

                                if( pLineAction )
                                {
                                    maActions.push_back(
                                        MtfAction(
                                            pLineAction,
                                            io_rCurrActionIndex ) );

                                    io_rCurrActionIndex += pLineAction->getActionCount()-1;
                                }
                            }
                            else if( LINE_NONE != rLineInfo.GetStyle() )
                            {
                                // 'thick' line polygon
                                rendering::StrokeAttributes aStrokeAttributes;

                                setupStrokeAttributes( aStrokeAttributes,
                                                       rVDev,
                                                       rLineInfo );

                                pLineAction =
                                    internal::PolyPolyActionFactory::createPolyPolyAction(
                                        aPoly,
                                        rCanvas,
                                        rState,
                                        aStrokeAttributes ) ;

                                if( pLineAction )
                                {
                                    maActions.push_back(
                                        MtfAction(
                                            pLineAction,
                                            io_rCurrActionIndex ) );

                                    io_rCurrActionIndex += pLineAction->getActionCount()-1;
                                }
                            }
                            // else: line style is default
                            // (i.e. invisible), don't generate action
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

                        ActionSharedPtr pBmpAction(
                                internal::BitmapActionFactory::createBitmapAction(
                                    pAct->GetBitmap(),
                                    rVDev.LogicToPixel( pAct->GetPoint() ),
                                    rCanvas,
                                    getState( rStates ) ) );

                        if( pBmpAction )
                        {
                            maActions.push_back(
                                MtfAction(
                                    pBmpAction,
                                    io_rCurrActionIndex ) );

                            io_rCurrActionIndex += pBmpAction->getActionCount()-1;
                        }
                    }
                    break;

                    case META_BMPSCALE_ACTION:
                    {
                        MetaBmpScaleAction* pAct = static_cast<MetaBmpScaleAction*>(pCurrAct);

                        ActionSharedPtr pBmpAction(
                                internal::BitmapActionFactory::createBitmapAction(
                                    pAct->GetBitmap(),
                                    rVDev.LogicToPixel( pAct->GetPoint() ),
                                    rVDev.LogicToPixel( pAct->GetSize() ),
                                    rCanvas,
                                    getState( rStates ) ) );

                        if( pBmpAction )
                        {
                            maActions.push_back(
                                MtfAction(
                                    pBmpAction,
                                    io_rCurrActionIndex ) );

                            io_rCurrActionIndex += pBmpAction->getActionCount()-1;
                        }
                    }
                    break;

                    case META_BMPSCALEPART_ACTION:
                    {
                        MetaBmpScalePartAction* pAct = static_cast<MetaBmpScalePartAction*>(pCurrAct);

                        // crop bitmap to given source rectangle (no
                        // need to copy and convert the whole bitmap)
                        Bitmap aBmp( pAct->GetBitmap() );
                        const Rectangle aCropRect( pAct->GetSrcPoint(),
                                                    pAct->GetSrcSize() );
                        aBmp.Crop( aCropRect );

                        ActionSharedPtr pBmpAction(
                                internal::BitmapActionFactory::createBitmapAction(
                                    aBmp,
                                    rVDev.LogicToPixel( pAct->GetDestPoint() ),
                                    rVDev.LogicToPixel( pAct->GetDestSize() ),
                                    rCanvas,
                                    getState( rStates ) ) );

                        if( pBmpAction )
                        {
                            maActions.push_back(
                                MtfAction(
                                    pBmpAction,
                                    io_rCurrActionIndex ) );

                            io_rCurrActionIndex += pBmpAction->getActionCount()-1;
                        }
                    }
                    break;

                    case META_BMPEX_ACTION:
                    {
                        MetaBmpExAction* pAct = static_cast<MetaBmpExAction*>(pCurrAct);

                        ActionSharedPtr pBmpAction(
                                internal::BitmapActionFactory::createBitmapAction(
                                    pAct->GetBitmapEx(),
                                    rVDev.LogicToPixel( pAct->GetPoint() ),
                                    rCanvas,
                                    getState( rStates ) ) );

                        if( pBmpAction )
                        {
                            maActions.push_back(
                                MtfAction(
                                    pBmpAction,
                                    io_rCurrActionIndex ) );

                            io_rCurrActionIndex += pBmpAction->getActionCount()-1;
                        }
                    }
                    break;

                    case META_BMPEXSCALE_ACTION:
                    {
                        MetaBmpExScaleAction* pAct = static_cast<MetaBmpExScaleAction*>(pCurrAct);

                        ActionSharedPtr pBmpAction(
                                internal::BitmapActionFactory::createBitmapAction(
                                    pAct->GetBitmapEx(),
                                    rVDev.LogicToPixel( pAct->GetPoint() ),
                                    rVDev.LogicToPixel( pAct->GetSize() ),
                                    rCanvas,
                                    getState( rStates ) ) );

                        if( pBmpAction )
                        {
                            maActions.push_back(
                                MtfAction(
                                    pBmpAction,
                                    io_rCurrActionIndex ) );

                            io_rCurrActionIndex += pBmpAction->getActionCount()-1;
                        }
                    }
                    break;

                    case META_BMPEXSCALEPART_ACTION:
                    {
                        MetaBmpExScalePartAction* pAct = static_cast<MetaBmpExScalePartAction*>(pCurrAct);

                        // crop bitmap to given source rectangle (no
                        // need to copy and convert the whole bitmap)
                        BitmapEx aBmp( pAct->GetBitmapEx() );
                        const Rectangle aCropRect( pAct->GetSrcPoint(),
                                                   pAct->GetSrcSize() );
                        aBmp.Crop( aCropRect );

                        ActionSharedPtr pBmpAction(
                            internal::BitmapActionFactory::createBitmapAction(
                                aBmp,
                                rVDev.LogicToPixel( pAct->GetDestPoint() ),
                                rVDev.LogicToPixel( pAct->GetDestSize() ),
                                rCanvas,
                                getState( rStates ) ) );

                        if( pBmpAction )
                        {
                            maActions.push_back(
                                MtfAction(
                                    pBmpAction,
                                    io_rCurrActionIndex ) );

                            io_rCurrActionIndex += pBmpAction->getActionCount()-1;
                        }
                    }
                    break;

                    case META_MASK_ACTION:
                    {
                        MetaMaskAction* pAct = static_cast<MetaMaskAction*>(pCurrAct);

                        // create masked BitmapEx right here, as the
                        // canvas does not provide equivalent
                        // functionality
                        BitmapEx aBmp( createMaskBmpEx( pAct->GetBitmap(),
                                                        pAct->GetColor() ));

                        ActionSharedPtr pBmpAction(
                            internal::BitmapActionFactory::createBitmapAction(
                                aBmp,
                                rVDev.LogicToPixel( pAct->GetPoint() ),
                                rCanvas,
                                getState( rStates ) ) );

                        if( pBmpAction )
                        {
                            maActions.push_back(
                                MtfAction(
                                    pBmpAction,
                                    io_rCurrActionIndex ) );

                            io_rCurrActionIndex += pBmpAction->getActionCount()-1;
                        }
                    }
                    break;

                    case META_MASKSCALE_ACTION:
                    {
                        MetaMaskScaleAction* pAct = static_cast<MetaMaskScaleAction*>(pCurrAct);

                        // create masked BitmapEx right here, as the
                        // canvas does not provide equivalent
                        // functionality
                        BitmapEx aBmp( createMaskBmpEx( pAct->GetBitmap(),
                                                        pAct->GetColor() ));

                        ActionSharedPtr pBmpAction(
                            internal::BitmapActionFactory::createBitmapAction(
                                aBmp,
                                rVDev.LogicToPixel( pAct->GetPoint() ),
                                rVDev.LogicToPixel( pAct->GetSize() ),
                                rCanvas,
                                getState( rStates ) ) );

                        if( pBmpAction )
                        {
                            maActions.push_back(
                                MtfAction(
                                    pBmpAction,
                                    io_rCurrActionIndex ) );

                            io_rCurrActionIndex += pBmpAction->getActionCount()-1;
                        }
                    }
                    break;

                    case META_MASKSCALEPART_ACTION:
                    {
                        MetaMaskScalePartAction* pAct = static_cast<MetaMaskScalePartAction*>(pCurrAct);

                        // create masked BitmapEx right here, as the
                        // canvas does not provide equivalent
                        // functionality
                        BitmapEx aBmp( createMaskBmpEx( pAct->GetBitmap(),
                                                        pAct->GetColor() ));

                        // crop bitmap to given source rectangle (no
                        // need to copy and convert the whole bitmap)
                        const Rectangle aCropRect( pAct->GetSrcPoint(),
                                                   pAct->GetSrcSize() );
                        aBmp.Crop( aCropRect );

                        ActionSharedPtr pBmpAction(
                            internal::BitmapActionFactory::createBitmapAction(
                                aBmp,
                                rVDev.LogicToPixel( pAct->GetDestPoint() ),
                                rVDev.LogicToPixel( pAct->GetDestSize() ),
                                rCanvas,
                                getState( rStates ) ) );

                        if( pBmpAction )
                        {
                            maActions.push_back(
                                MtfAction(
                                    pBmpAction,
                                    io_rCurrActionIndex ) );

                            io_rCurrActionIndex += pBmpAction->getActionCount()-1;
                        }
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

                            ActionSharedPtr pPolyAction(
                                internal::PolyPolyActionFactory::createPolyPolyAction(
                                    rVDev.LogicToPixel( pAct->GetPolyPolygon() ),
                                    rCanvas,
                                    rState,
                                    pAct->GetTransparence() ) );

                            if( pPolyAction )
                            {
                                maActions.push_back(
                                    MtfAction(
                                        pPolyAction,
                                        io_rCurrActionIndex ) );

                                io_rCurrActionIndex += pPolyAction->getActionCount()-1;
                            }
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

                        ActionSharedPtr pFloatTransAction(
                            internal::TransparencyGroupActionFactory::createTransparencyGroupAction(
                                pMtf,
                                pGradient,
                                rParms,
                                rVDev.LogicToPixel( pAct->GetPoint() ),
                                rVDev.LogicToPixel( pAct->GetSize() ),
                                rCanvas,
                                getState( rStates ) ) );

                        if( pFloatTransAction )
                        {
                            maActions.push_back(
                                MtfAction(
                                    pFloatTransAction,
                                    io_rCurrActionIndex ) );

                            io_rCurrActionIndex += pFloatTransAction->getActionCount()-1;
                        }
                    }
                    break;

                    case META_TEXT_ACTION:
                    {
                        MetaTextAction* pAct = static_cast<MetaTextAction*>(pCurrAct);

                        createTextAction(
                            pAct->GetPoint(),
                            pAct->GetText(),
                            pAct->GetIndex(),
                            pAct->GetLen() == (USHORT)STRING_LEN ? pAct->GetText().Len() - pAct->GetIndex() : pAct->GetLen(),
                            NULL,
                            rVDev,
                            rCanvas,
                            rStates,
                            rParms,
                            bSubsettableActions,
                            io_rCurrActionIndex );
                    }
                    break;

                    case META_TEXTARRAY_ACTION:
                    {
                        MetaTextArrayAction* pAct = static_cast<MetaTextArrayAction*>(pCurrAct);

                        createTextAction(
                            pAct->GetPoint(),
                            pAct->GetText(),
                            pAct->GetIndex(),
                            pAct->GetLen() == (USHORT)STRING_LEN ? pAct->GetText().Len() - pAct->GetIndex() : pAct->GetLen(),
                            pAct->GetDXArray(),
                            rVDev,
                            rCanvas,
                            rStates,
                            rParms,
                            bSubsettableActions,
                            io_rCurrActionIndex );
                    }
                    break;

                    case META_TEXTLINE_ACTION:
                    {
                        MetaTextLineAction* pAct = static_cast<MetaTextLineAction*>(pCurrAct);

                        const OutDevState&  rState( getState( rStates ) );
                        const ::Size        aBaselineOffset( tools::getBaselineOffset( rState,
                                                                                       rVDev ) );
                        const ::Point       aStartPoint( pAct->GetStartPoint() );
                        const ::Size        aSize( rVDev.LogicToPixel(
                                                       ::Size( pAct->GetWidth(),
                                                               0 ) ) );

                        ActionSharedPtr pPolyAction(
                            PolyPolyActionFactory::createPolyPolyAction(
                                ::PolyPolygon(
                                    tools::createTextLinesPolyPolygon(
                                        ::vcl::unotools::b2DPointFromPoint(
                                            rVDev.LogicToPixel(
                                                ::Point(
                                                    aStartPoint.X() + aBaselineOffset.Width(),
                                                    aStartPoint.Y() + aBaselineOffset.Height() ) ) ),
                                        aSize.Width(),
                                        tools::createTextLineInfo( rVDev,
                                                                   rState ) ) ),
                                rCanvas,
                                rState ) );

                        if( pPolyAction.get() )
                        {
                            maActions.push_back(
                                MtfAction(
                                    pPolyAction,
                                    io_rCurrActionIndex ) );

                            io_rCurrActionIndex += pPolyAction->getActionCount()-1;
                        }
                    }
                    break;

                    case META_TEXTRECT_ACTION:
                    {
                        MetaTextRectAction* pAct = static_cast<MetaTextRectAction*>(pCurrAct);

                        pushState( rStates, PUSH_ALL );

                        // use the VDev to break up the text rect
                        // action into readily formatted lines
                        GDIMetaFile aTmpMtf;
                        rVDev.AddTextRectActions( pAct->GetRect(),
                                                  pAct->GetText(),
                                                  pAct->GetStyle(),
                                                  aTmpMtf );

                        createActions( rCanvas, rVDev, aTmpMtf, rStates,
                                       rParms, bSubsettableActions,
                                       io_rCurrActionIndex );

                        popState( rStates );

                        break;
                    }

                    case META_STRETCHTEXT_ACTION:
                    {
                        MetaStretchTextAction* pAct = static_cast<MetaStretchTextAction*>(pCurrAct);

                        const USHORT nLen( pAct->GetLen() == (USHORT)STRING_LEN ?
                                           pAct->GetText().Len() - pAct->GetIndex() : pAct->GetLen() );

                        // have to fit the text into the given
                        // width. This is achieved by internally
                        // generating a DX array, and uniformly
                        // distributing the excess/insufficient width
                        // to every logical character.
                        ::boost::scoped_array< sal_Int32 > pDXArray( new sal_Int32[nLen] );

                        rVDev.GetTextArray( pAct->GetText(), pDXArray.get(),
                                            pAct->GetIndex(), pAct->GetLen() );

                        const sal_Int32 nWidthDifference( pAct->GetWidth() - pDXArray[ nLen-1 ] );

                        // Last entry of pDXArray contains total width of the text
                        sal_Int32* p=pDXArray.get();
                        for( USHORT i=1; i<=nLen; ++i )
                        {
                            // calc ratio for every array entry, to
                            // distribute rounding errors 'evenly'
                            // across the characters. Note that each
                            // entry represents the 'end' position of
                            // the corresponding character, thus, we
                            // let i run from 1 to nLen.
                            *p++ += (sal_Int32)i*nWidthDifference/nLen;
                        }

                        createTextAction(
                            pAct->GetPoint(),
                            pAct->GetText(),
                            pAct->GetIndex(),
                            pAct->GetLen() == (USHORT)STRING_LEN ? pAct->GetText().Len() - pAct->GetIndex() : pAct->GetLen(),
                            pDXArray.get(),
                            rVDev,
                            rCanvas,
                            rStates,
                            rParms,
                            bSubsettableActions,
                            io_rCurrActionIndex );
                    }
                    break;

                    default:
                        OSL_ENSURE( false,
                                    "Unknown meta action type encountered" );
                        break;
                }

                // increment action index (each mtf action counts _at
                // least_ one. Some count for more, therefore,
                // io_rCurrActionIndex is sometimes incremented by
                // pAct->getActionCount()-1 above, the -1 being the
                // correction for the unconditional increment here).
                ++io_rCurrActionIndex;
            }

            return true;
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

                void operator()( const ::cppcanvas::internal::ImplRenderer::MtfAction&  rAction,
                                 const Action::Subset&                                  rSubset )
                {
                    // ANDing the result. We want to fail if at least
                    // one action failed.
                    mbRet &= rAction.mpAction->render( maTransformation,
                                                       rSubset );
                }

            private:
                ::basegfx::B2DHomMatrix maTransformation;
                bool                    mbRet;
            };

            class AreaQuery
            {
            public:
                AreaQuery( const ::basegfx::B2DHomMatrix& rTransformation ) :
                    maTransformation( rTransformation ),
                    maBounds()
                {
                }

                bool result()
                {
                    return true; // nothing can fail here
                }

                void operator()( const ::cppcanvas::internal::ImplRenderer::MtfAction& rAction )
                {
                    maBounds.expand( rAction.mpAction->getBounds( maTransformation ) );
                }

                void operator()( const ::cppcanvas::internal::ImplRenderer::MtfAction&  rAction,
                                 const Action::Subset&                                  rSubset )
                {
                    maBounds.expand( rAction.mpAction->getBounds( maTransformation,
                                                                  rSubset ) );
                }

                ::basegfx::B2DRange getBounds() const
                {
                    return maBounds;
                }

            private:
                ::basegfx::B2DHomMatrix maTransformation;
                ::basegfx::B2DRange     maBounds;
            };

            // Doing that via inline class. Compilers tend to not inline free
            // functions.
            struct UpperBoundActionIndexComparator
            {
                bool operator()( const ::cppcanvas::internal::ImplRenderer::MtfAction& rLHS,
                                 const ::cppcanvas::internal::ImplRenderer::MtfAction& rRHS )
                {
                    const sal_Int32 nLHSCount( rLHS.mpAction ?
                                               rLHS.mpAction->getActionCount() : 0 );
                    const sal_Int32 nRHSCount( rRHS.mpAction ?
                                               rRHS.mpAction->getActionCount() : 0 );

                    // compare end of action range, to have an action selected
                    // by lower_bound even if the requested index points in
                    // the middle of the action's range
                    return rLHS.mnOrigIndex + nLHSCount < rRHS.mnOrigIndex + nRHSCount;
                }
            };

            /** Algorithm to apply given functor to a subset range

                @tpl Functor

                Functor to call for each element of the subset
                range. Must provide the following method signatures:
                bool result() (returning false if operation failed)

             */
            template< typename Functor > bool
                forSubsetRange( Functor&                                            rFunctor,
                                ImplRenderer::ActionVector::const_iterator          aRangeBegin,
                                ImplRenderer::ActionVector::const_iterator          aRangeEnd,
                                sal_Int32                                           nStartIndex,
                                sal_Int32                                           nEndIndex,
                                const ImplRenderer::ActionVector::const_iterator&   rEnd )
            {
                if( aRangeBegin == aRangeEnd )
                {
                    // only a single action. Setup subset, and call functor
                    Action::Subset aSubset;
                    aSubset.mnSubsetBegin = ::std::max( 0L,
                                                        nStartIndex - aRangeBegin->mnOrigIndex );
                    aSubset.mnSubsetEnd   = ::std::min( aRangeBegin->mpAction->getActionCount(),
                                                        nEndIndex - aRangeBegin->mnOrigIndex );

                    ENSURE_AND_RETURN( aSubset.mnSubsetBegin >= 0 && aSubset.mnSubsetEnd >= 0,
                                       "ImplRenderer::forSubsetRange(): Invalid indices" );

                    rFunctor( *aRangeBegin, aSubset );
                }
                else
                {
                    // more than one action.

                    // render partial first, full intermediate, and
                    // partial last action
                    Action::Subset aSubset;
                    aSubset.mnSubsetBegin = ::std::max( 0L,
                                                        nStartIndex - aRangeBegin->mnOrigIndex );
                    aSubset.mnSubsetEnd   = aRangeBegin->mpAction->getActionCount();

                    ENSURE_AND_RETURN( aSubset.mnSubsetBegin >= 0 && aSubset.mnSubsetEnd >= 0,
                                       "ImplRenderer::forSubsetRange(): Invalid indices" );

                    rFunctor( *aRangeBegin, aSubset );

                    // first action rendered, skip to next
                    ++aRangeBegin;

                    // render full middle actions
                    while( aRangeBegin != aRangeEnd )
                        rFunctor( *aRangeBegin++ );

                    if( aRangeEnd == rEnd ||
                        aRangeEnd->mnOrigIndex > nEndIndex )
                    {
                        // aRangeEnd denotes end of action vector,
                        //
                        // or
                        //
                        // nEndIndex references something _after_
                        // aRangeBegin, but _before_ aRangeEnd
                        //
                        // either way: no partial action left
                        return rFunctor.result();
                    }

                    aSubset.mnSubsetBegin = 0;
                    aSubset.mnSubsetEnd   = nEndIndex - aRangeEnd->mnOrigIndex;

                    ENSURE_AND_RETURN( aSubset.mnSubsetBegin >= 0 && aSubset.mnSubsetEnd >= 0,
                                       "ImplRenderer::forSubsetRange(): Invalid indices" );

                    rFunctor( *aRangeEnd, aSubset );
                }

                return rFunctor.result();
            }
        }

        bool ImplRenderer::getSubsetIndices( sal_Int32&                     io_rStartIndex,
                                             sal_Int32&                     io_rEndIndex,
                                             ActionVector::const_iterator&  o_rRangeBegin,
                                             ActionVector::const_iterator&  o_rRangeEnd ) const
        {
            ENSURE_AND_RETURN( io_rStartIndex<=io_rEndIndex,
                               "ImplRenderer::getSubsetIndices(): invalid action range" );

            ENSURE_AND_RETURN( !maActions.empty(),
                               "ImplRenderer::getSubsetIndices(): no actions to render" );

            const sal_Int32 nMinActionIndex( maActions.front().mnOrigIndex );
            const sal_Int32 nMaxActionIndex( maActions.back().mnOrigIndex +
                                             maActions.back().mpAction->getActionCount() );

            // clip given range to permissible values (there might be
            // ranges before and behind the valid indices)
            io_rStartIndex = ::std::max( nMinActionIndex,
                                         io_rStartIndex );
            io_rEndIndex = ::std::min( nMaxActionIndex,
                                       io_rEndIndex );

            if( io_rStartIndex == io_rEndIndex ||
                io_rStartIndex > io_rEndIndex )
            {
                // empty range, don't render anything. The second
                // condition e.g. happens if the requested range lies
                // fully before or behind the valid action indices.
                return false;
            }


            const ActionVector::const_iterator aBegin( maActions.begin() );
            const ActionVector::const_iterator aEnd( maActions.end() );


            // find start and end action
            // =========================
            o_rRangeBegin = ::std::lower_bound( aBegin, aEnd,
                                                MtfAction( ActionSharedPtr(), io_rStartIndex ),
                                                UpperBoundActionIndexComparator() );
            o_rRangeEnd   = ::std::lower_bound( aBegin, aEnd,
                                                MtfAction( ActionSharedPtr(), io_rEndIndex ),
                                                UpperBoundActionIndexComparator() );
            return true;
        }


        // Public methods
        // ====================================================================

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
            const Size aMtfSizePixPre( aVDev.LogicToPixel( aMtfSize,
                                                           rMtf.GetPrefMapMode() ) );
            const Point aEmptyPt;
            const Point aMtfOriginPix( aVDev.LogicToPixel( aEmptyPt ) );

            // #i44110# correct null-sized output - there are shapes
            // which have zero size in at least one dimension
            const Size aMtfSizePix( ::std::max( aMtfSizePixPre.Width(), 1L ),
                                    ::std::max( aMtfSizePixPre.Height(), 1L ) );

            // init state stack
            clearStateStack( aStateStack );

            // Setup local state, such that the metafile renders
            // itself into a one-by-one square at the origin for
            // identity view and render transformations
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

                rState.xFont = createFont( rState.fontRotation,
                                           ::Font(), // default font
                                           rCanvas,
                                           aVDev,
                                           rParams );
            }

            sal_Int32 nCurrActions(0);
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
                           true, // TODO(P1): make subsettability configurable
                           nCurrActions );
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
                    BitmapActionFactory::createBitmapAction(
                        rBmpEx,
                        Point(),
                        rCanvas,
                        aState),
                    0 ) );
        }

        ImplRenderer::~ImplRenderer()
        {
        }

        bool ImplRenderer::drawSubset( sal_Int32    nStartIndex,
                                       sal_Int32    nEndIndex ) const
        {
            RTL_LOGFILE_CONTEXT( aLog, "::cppcanvas::internal::ImplRenderer::drawSubset()" );

            ActionVector::const_iterator aRangeBegin;
            ActionVector::const_iterator aRangeEnd;

            if( !getSubsetIndices( nStartIndex, nEndIndex,
                                   aRangeBegin, aRangeEnd ) )
                return true; // nothing to render (but _that_ was successful)

            // now, aRangeBegin references the action in which the
            // subset rendering must start, and aRangeEnd references
            // the action in which the subset rendering must end (it
            // might also end right at the start of the referenced
            // action, such that zero of that action needs to be
            // rendered).


            // render subset of actions
            // ========================

            ::basegfx::B2DHomMatrix aMatrix;
            ::canvas::tools::getRenderStateTransform( aMatrix,
                                                      getRenderState() );

            ActionRenderer aRenderer( aMatrix );

            return forSubsetRange( aRenderer,
                                   aRangeBegin,
                                   aRangeEnd,
                                   nStartIndex,
                                   nEndIndex,
                                   maActions.end() );
        }

        ::basegfx::B2DRange ImplRenderer::getSubsetArea( sal_Int32  nStartIndex,
                                                         sal_Int32  nEndIndex ) const
        {
            RTL_LOGFILE_CONTEXT( aLog, "::cppcanvas::internal::ImplRenderer::getSubsetArea()" );

            ActionVector::const_iterator aRangeBegin;
            ActionVector::const_iterator aRangeEnd;

            if( !getSubsetIndices( nStartIndex, nEndIndex,
                                   aRangeBegin, aRangeEnd ) )
                return ::basegfx::B2DRange(); // nothing to render -> empty range

            // now, aRangeBegin references the action in which the
            // subset querying must start, and aRangeEnd references
            // the action in which the subset querying must end (it
            // might also end right at the start of the referenced
            // action, such that zero of that action needs to be
            // queried).


            // query bounds for subset of actions
            // ==================================

            ::basegfx::B2DHomMatrix aMatrix;
            ::canvas::tools::getRenderStateTransform( aMatrix,
                                                      getRenderState() );

            AreaQuery aQuery( aMatrix );
            forSubsetRange( aQuery,
                            aRangeBegin,
                            aRangeEnd,
                            nStartIndex,
                            nEndIndex,
                            maActions.end() );

            return aQuery.getBounds();
        }

        bool ImplRenderer::draw() const
        {
            RTL_LOGFILE_CONTEXT( aLog, "::cppcanvas::internal::ImplRenderer::draw()" );

            ::basegfx::B2DHomMatrix aMatrix;
            ::canvas::tools::getRenderStateTransform( aMatrix,
                                                      getRenderState() );

            return ::std::for_each( maActions.begin(), maActions.end(), ActionRenderer( aMatrix ) ).result();
        }
    }
}
