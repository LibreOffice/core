/*************************************************************************
 *
 *  $RCSfile: implrenderer.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: thb $ $Date: 2004-03-18 10:41:04 $
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

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif
#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

#ifndef _COMPHELPER_SEQUENCE_HXX_
#include <comphelper/sequence.hxx>
#endif

#include <cppcanvas/canvas.hxx>

#ifndef _VCL_CANVASTOOLS_HXX
#include <vcl/canvastools.hxx>
#endif
#ifndef _BGFX_TOOLS_CANVASTOOLS_HXX
#include <basegfx/tools/canvastools.hxx>
#endif
#ifndef _CANVAS_CANVASTOOLS_HXX
#include <canvas/canvastools.hxx>
#endif

#include "implrenderer.hxx"
#include "tools.hxx"
#include "outdevstate.hxx"

#include "action.hxx"
#include "bitmapaction.hxx"
#include "lineaction.hxx"
#include "pointaction.hxx"
#include "polypolyaction.hxx"
#include "textaction.hxx"

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

#ifndef _SV_GDIMTF_HXX
#include <vcl/gdimtf.hxx>
#endif

#ifndef _SV_METAACT_HXX
#include <vcl/metaact.hxx>
#endif

#ifndef _SV_VIRDEV_HXX
#include <vcl/virdev.hxx>
#endif

#ifndef _TL_POLY_HXX
#include <tools/poly.hxx>
#endif

#include "outdevstate.hxx"


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
            maActions.push_back(
                MtfAction(
                    ActionSharedPtr(
                        new internal::PolyPolyAction( rPolyPoly, rCanvas, getState( rStates ) ) ),
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

        void ImplRenderer::createGradientAction( const Rectangle&       rRect,
                                                 const Gradient&        rGradient,
                                                 VirtualDevice&         rVDev,
                                                 const CanvasSharedPtr& rCanvas,
                                                 VectorOfOutDevStates&  rStates      )
        {
            DBG_TESTSOLARMUTEX();

            // TODO: Use native canvas gradients here (saves a lot of UNO calls)
            GDIMetaFile aTmpMtf;

            rVDev.AddGradientActions( rRect,
                                      rGradient,
                                      aTmpMtf );

            pushState( rStates );
            createActions( rCanvas, rVDev, aTmpMtf, rStates );
            popState( rStates );
        }

        bool ImplRenderer::createActions( const CanvasSharedPtr&    rCanvas,
                                          VirtualDevice&            rVDev,
                                          GDIMetaFile&              rMtf,
                                          VectorOfOutDevStates&     rStates )
        {
            /* TODO:
               =====

               - Float-Transparency (skipped for prototype
               - bitmap fillings (do that via comments)
               - gradient fillings (do that via comments)

               - think about mapping. _If_ we do everything in logical
                    coordinates (which would solve the probs for stroke
                 widths and and text offsets), then we would have to
                 recalc scaling for every drawing operation. This is
                 because the outdev map mode might change at any time.

             */

            // Loop over every metaaction
            // ==========================
            MetaAction* pCurrAct;
            int nCurrActionIndex;

            // TODO: think about caching
            for( nCurrActionIndex=0, pCurrAct=rMtf.FirstAction();
                 pCurrAct;
                 ++nCurrActionIndex, pCurrAct = rMtf.NextAction() )
            {
                // execute every action, to keep VDev state up-to-date
                // (currently used only for the map mode, and for
                // line/fill color when processing a
                // META_TRANSPARENT_ACTION)
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
                    case META_ISECTRECTCLIPREGION_ACTION:
                    case META_ISECTREGIONCLIPREGION_ACTION:
                    case META_MOVECLIPREGION_ACTION:
                        // TODO: NYI
                        break;

                    case META_LINECOLOR_ACTION:
                        setStateColor( static_cast<MetaLineColorAction*>(pCurrAct),
                                       getState( rStates ).isLineColorSet,
                                       getState( rStates ).lineColor,
                                       rCanvas );
                        break;

                    case META_FILLCOLOR_ACTION:
                        setStateColor( static_cast<MetaFillColorAction*>(pCurrAct),
                                       getState( rStates ).isFillColorSet,
                                       getState( rStates ).fillColor,
                                       rCanvas );
                        break;

                    case META_TEXTCOLOR_ACTION:
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
                    break;

                    case META_TEXTFILLCOLOR_ACTION:
                        setStateColor( static_cast<MetaTextFillColorAction*>(pCurrAct),
                                       getState( rStates ).isTextFillColorSet,
                                       getState( rStates ).textFillColor,
                                       rCanvas );
                        break;

                    case META_TEXTLINECOLOR_ACTION:
                        setStateColor( static_cast<MetaTextLineColorAction*>(pCurrAct),
                                       getState( rStates ).isTextLineColorSet,
                                       getState( rStates ).textLineColor,
                                       rCanvas );
                        break;

                    case META_TEXTALIGN_ACTION:
                        // TODO: NYI
                        break;

                    case META_FONT_ACTION:
                    {
                        // TODO: For now, only dummy implementation
                        rendering::FontRequest aFontRequest;
                        const ::Font& rFont( static_cast<MetaFontAction*>(pCurrAct)->GetFont() );

                        aFontRequest.FamilyName = rFont.GetName();
                        aFontRequest.StyleName = rFont.GetStyleName();

                        // TODO: use correct scale direction, font
                        // height might be width or anything else
                        const Size aSize( 0, rFont.GetHeight() );
                        aFontRequest.CellSize = rVDev.LogicToPixel( aSize ).Height();

                        const short nFontAngle( rFont.GetOrientation() );

                        // setup state-local text transformation,
                        // should the font be rotated
                        if( nFontAngle != 0 )
                        {
                            // VCL font does not access system structs here
                            const double rAngle( nFontAngle * (F_PI / 1800.0) );

                            // reset transform
                            getState( rStates ).fontTransform.identity();

                            // rotate by given angle
                            getState( rStates ).fontTransform.rotate( -rAngle );
                        }

                        getState( rStates ).xFont = rCanvas->getUNOCanvas()->queryFont( aFontRequest );
                    }
                    break;

                    case META_RASTEROP_ACTION:
                        // TODO: NYI
                        break;

                    case META_REFPOINT_ACTION:
                        // TODO: NYI
                        break;

                    case META_LAYOUTMODE_ACTION:
                    {
                        // TODO: A lot is missing here
                        switch( static_cast<MetaLayoutModeAction*>(pCurrAct)->GetLayoutMode() )
                        {
                            case TEXT_LAYOUT_BIDI_RTL:
                            case TEXT_LAYOUT_TEXTORIGIN_RIGHT:
                                getState( rStates ).textDirection = rendering::TextDirection::RIGHT_TO_LEFT;
                                break;

                            case TEXT_LAYOUT_BIDI_LTR:
                            case TEXT_LAYOUT_BIDI_STRONG:
                            case TEXT_LAYOUT_TEXTORIGIN_LEFT:
                            case TEXT_LAYOUT_COMPLEX_DISABLED:
                            case TEXT_LAYOUT_ENABLE_LIGATURES:
                            case TEXT_LAYOUT_SUBSTITUTE_DIGITS:
                                getState( rStates ).textDirection = rendering::TextDirection::LEFT_TO_RIGHT;
                                break;
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
                        createGradientAction( pGradAct->GetRect(),
                                              pGradAct->GetGradient(),
                                              rVDev,
                                              rCanvas,
                                              rStates );
                    }
                    break;

                    case META_HATCH_ACTION:
                    {
                        // TODO: use native Canvas hatches here
                        GDIMetaFile aTmpMtf;

                        rVDev.AddHatchActions( static_cast<MetaHatchAction*>(pCurrAct)->GetPolyPolygon(),
                                               static_cast<MetaHatchAction*>(pCurrAct)->GetHatch(),
                                               aTmpMtf );
                        createActions( rCanvas, rVDev, aTmpMtf, rStates );
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
                                           rStates );

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
                                                pushState( rStates );

                                                // TODO: Hack! If
                                                // current state
                                                // already contains a
                                                // clipping, we'll
                                                // overwrite it here!
                                                getState( rStates ).xClipPoly = ::vcl::unotools::xPolyPolygonFromPolyPolygon( rCanvas->getUNOCanvas()->getDevice(),
                                                                                                                              rVDev.LogicToPixel( pGradAction->GetPolyPolygon() ) );

                                                createGradientAction( pGradAction->GetPolyPolygon().GetBoundRect(),
                                                                      pGradAction->GetGradient(),
                                                                      rVDev,
                                                                      rCanvas,
                                                                      rStates );

                                                popState( rStates );
                                            }
                                        }
                                        break;
                                }
                            }
                        }

                        // Handle drawing layer strokes
                        else if( pAct->GetComment().Equals( "XPATHSTROKE_SEQ_BEGIN" ) )
                        {
                            // TODO: Later
#if 0
                            const BYTE* pData = pAct->GetData();
                            if ( pData )
                            {
                                SvMemoryStream  aMemStm( (void*)pData, pA->GetDataSize(), STREAM_READ );

                                SvtGraphicStroke aStroke;
                                aMemStm >> aStroke;

                                // TODO: respect exceptions, like
                                // start/end arrows and joins not
                                // displayable via Canvas

                                // TODO: use correct scale direction, stroke
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
                            // TODO: Later
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
                        maActions.push_back(
                            MtfAction(
                                ActionSharedPtr(
                                    new internal::PointAction(
                                        rVDev.LogicToPixel( static_cast<MetaPointAction*>(pCurrAct)->GetPoint() ),
                                        rCanvas,
                                        getState( rStates ) ) ),
                                nCurrActionIndex ) );
                    }
                    break;

                    case META_PIXEL_ACTION:
                    {
                        maActions.push_back(
                            MtfAction(
                                ActionSharedPtr(
                                    new internal::PointAction(
                                        rVDev.LogicToPixel(
                                            static_cast<MetaPixelAction*>(pCurrAct)->GetPoint() ),
                                        rCanvas,
                                        getState( rStates ),
                                        static_cast<MetaPixelAction*>(pCurrAct)->GetColor() ) ),
                                nCurrActionIndex ) );
                    }
                    break;

                    case META_LINE_ACTION:
                    {
                        maActions.push_back(
                            MtfAction(
                                ActionSharedPtr(
                                    new internal::LineAction(
                                        rVDev.LogicToPixel( static_cast<MetaLineAction*>(pCurrAct)->GetStartPoint() ),
                                        rVDev.LogicToPixel( static_cast<MetaLineAction*>(pCurrAct)->GetEndPoint() ),
                                        rCanvas,
                                        getState( rStates ) ) ),
                                nCurrActionIndex ) );
                    }
                    break;

                    case META_RECT_ACTION:
                        createFillAndStroke( ::PolyPolygon( ::Polygon( rVDev.LogicToPixel( static_cast<MetaRectAction*>(pCurrAct)->GetRect() ) ) ),
                                             rCanvas, nCurrActionIndex,
                                             rStates );
                        break;

                    case META_ROUNDRECT_ACTION:
                        createFillAndStroke( rVDev.LogicToPixel( Polygon( static_cast<MetaRoundRectAction*>(pCurrAct)->GetRect(),
                                                                          static_cast<MetaRoundRectAction*>(pCurrAct)->GetHorzRound(),
                                                                          static_cast<MetaRoundRectAction*>(pCurrAct)->GetVertRound() ) ),
                                             rCanvas, nCurrActionIndex,
                                             rStates );
                        break;

                    case META_ELLIPSE_ACTION:
                    {
                        const Rectangle& rRect = static_cast<MetaEllipseAction*>(pCurrAct)->GetRect();
                        createFillAndStroke( rVDev.LogicToPixel( Polygon( rRect.Center(),
                                                                          rRect.GetWidth() >> 1,
                                                                          rRect.GetHeight() >> 1 ) ),
                                             rCanvas, nCurrActionIndex,
                                             rStates );
                        break;
                    }

                    case META_ARC_ACTION:
                        createFillAndStroke( rVDev.LogicToPixel( Polygon( static_cast<MetaArcAction*>(pCurrAct)->GetRect(),
                                                                          static_cast<MetaArcAction*>(pCurrAct)->GetStartPoint(),
                                                                          static_cast<MetaArcAction*>(pCurrAct)->GetEndPoint(), POLY_ARC ) ),
                                             rCanvas, nCurrActionIndex,
                                             rStates );
                        break;

                    case META_PIE_ACTION:
                        createFillAndStroke( rVDev.LogicToPixel( Polygon( static_cast<MetaPieAction*>(pCurrAct)->GetRect(),
                                                                          static_cast<MetaPieAction*>(pCurrAct)->GetStartPoint(),
                                                                          static_cast<MetaPieAction*>(pCurrAct)->GetEndPoint(), POLY_PIE ) ),
                                             rCanvas, nCurrActionIndex,
                                             rStates );
                        break;

                    case META_CHORD_ACTION:
                        createFillAndStroke( rVDev.LogicToPixel( Polygon( static_cast<MetaChordAction*>(pCurrAct)->GetRect(),
                                                                          static_cast<MetaChordAction*>(pCurrAct)->GetStartPoint(),
                                                                          static_cast<MetaChordAction*>(pCurrAct)->GetEndPoint(), POLY_CHORD ) ),
                                             rCanvas, nCurrActionIndex,
                                             rStates );
                        break;

                    case META_POLYLINE_ACTION:
                    {
                        maActions.push_back(
                            MtfAction(
                                ActionSharedPtr(
                                    new internal::PolyPolyAction(
                                        rVDev.LogicToPixel( static_cast<MetaPolyLineAction*>(pCurrAct)->GetPolygon() ),
                                        rCanvas,
                                        getState( rStates ),
                                        internal::PolyPolyAction::strokeOnly ) ),
                                nCurrActionIndex ) );
                    }
                    break;

                    case META_POLYGON_ACTION:
                        createFillAndStroke( rVDev.LogicToPixel( static_cast<MetaPolygonAction*>(pCurrAct)->GetPolygon() ),
                                             rCanvas, nCurrActionIndex,
                                             rStates );
                        break;

                    case META_POLYPOLYGON_ACTION:
                        createFillAndStroke( rVDev.LogicToPixel( static_cast<MetaPolyPolygonAction*>(pCurrAct)->GetPolyPolygon() ),
                                             rCanvas, nCurrActionIndex,
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
                                nCurrActionIndex ) );
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
                                nCurrActionIndex ) );
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
                                nCurrActionIndex ) );
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
                                nCurrActionIndex ) );
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
                                nCurrActionIndex ) );
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
                                nCurrActionIndex ) );
                    }
                    break;

                    case META_MASK_ACTION:
                    {
                        MetaMaskAction* pAct = static_cast<MetaMaskAction*>(pCurrAct);

                        // TODO: masking NYI. Further members: mask color
                        maActions.push_back(
                            MtfAction(
                                ActionSharedPtr(
                                    new internal::BitmapAction(
                                        pAct->GetBitmap(),
                                        rVDev.LogicToPixel( pAct->GetPoint() ),
                                        rCanvas,
                                        getState( rStates ) ) ),
                                nCurrActionIndex ) );
                    }
                    break;

                    case META_MASKSCALE_ACTION:
                    {
                        MetaMaskScaleAction* pAct = static_cast<MetaMaskScaleAction*>(pCurrAct);

                        // TODO: masking NYI. Further members: mask color
                        maActions.push_back(
                            MtfAction(
                                ActionSharedPtr(
                                    new internal::BitmapAction(
                                        pAct->GetBitmap(),
                                        rVDev.LogicToPixel( pAct->GetPoint() ),
                                        rVDev.LogicToPixel( pAct->GetSize() ),
                                        rCanvas,
                                        getState( rStates ) ) ),
                                nCurrActionIndex ) );
                    }
                    break;

                    case META_MASKSCALEPART_ACTION:
                    {
                        MetaMaskScalePartAction* pAct = static_cast<MetaMaskScalePartAction*>(pCurrAct);

                        // TODO: masking NYI. Further members: mask color
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
                                nCurrActionIndex ) );
                    }
                    break;

                    case META_GRADIENTEX_ACTION:
                        // TODO: use native Canvas gradients here
                        // action is ignored here, because redundant to META_GRADIENT_ACTION
                        break;

                    case META_WALLPAPER_ACTION:
                        // TODO: NYI
                        break;

                    case META_TRANSPARENT_ACTION:
                    {
                        MetaTransparentAction* pAct = static_cast<MetaTransparentAction*>(pCurrAct);

                        maActions.push_back(
                            MtfAction(
                                ActionSharedPtr(
                                    new internal::PolyPolyAction(
                                        rVDev.LogicToPixel( pAct->GetPolyPolygon() ),
                                        rCanvas,
                                        getState( rStates ),
                                        pAct->GetTransparence() ) ),
                                nCurrActionIndex ) );
                            }
                    break;

                    case META_FLOATTRANSPARENT_ACTION:
                        // TODO: NYI. This has to be rendered into a separate bitmap canvas
                        break;

                    case META_TEXT_ACTION:
                    {
                        MetaTextAction* pAct = static_cast<MetaTextAction*>(pCurrAct);

                        maActions.push_back(
                            MtfAction(
                                ActionSharedPtr(
                                    new internal::TextAction(
                                        rVDev.LogicToPixel(pAct->GetPoint()),
                                        pAct->GetText(),
                                        pAct->GetIndex(),
                                        pAct->GetLen(),
                                        rCanvas,
                                        getState( rStates ) ) ),
                                nCurrActionIndex ) );
                    }
                    break;

                    case META_TEXTARRAY_ACTION:
                    {
                        MetaTextArrayAction* pAct = static_cast<MetaTextArrayAction*>(pCurrAct);

                        uno::Sequence< double > offsets( ::comphelper::arrayToSequence<long int, double>( pAct->GetDXArray(),
                                                                                                          pAct->GetLen() ) );

                        // convert offsets to physical

                        // TODO: use correct scale direction, text advancement
                        // might be horizontal, vertical, or anything else
                        int i;
                        for( i=0; i<offsets.getLength(); ++i )
                        {
                            const Size aSize( static_cast<long>( offsets[i] + .5 ), 0 );
                            offsets[i] = rVDev.LogicToPixel( aSize ).Width();
                        }

                        maActions.push_back(
                            MtfAction(
                                ActionSharedPtr(
                                    new internal::TextAction(
                                        rVDev.LogicToPixel(pAct->GetPoint()),
                                        pAct->GetText(),
                                        pAct->GetIndex(),
                                        pAct->GetLen(),
                                        offsets,
                                        rCanvas,
                                        getState( rStates ) ) ),
                                nCurrActionIndex ) );
                    }
                    break;

                    case META_TEXTRECT_ACTION:
                    case META_STRETCHTEXT_ACTION:
                    case META_TEXTLINE_ACTION:
                        // TODO: NYI
                        DBG_ERROR("META_TEXT* not yet supported");
                        break;

                    default:
                        break;
                }
            }

            return true;
        }

        ImplRenderer::ImplRenderer( const CanvasSharedPtr&  rCanvas,
                                    const GDIMetaFile&      rMtf ) :
            CanvasGraphicHelper( rCanvas ),
            maActions()
        {
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

            const Size aMtfSizePix( aVDev.LogicToPixel( rMtf.GetPrefSize(),
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

                createActions( rCanvas,
                               aVDev,
                               const_cast<GDIMetaFile&>(rMtf), // HACK:
                                                               // we're
                                                               // changing
                                                               // the
                                                               // current
                                                               // action
                                                               // in
                                                               // createActions!
                               aStateStack );
            }
        }

        ImplRenderer::ImplRenderer( const CanvasSharedPtr&  rCanvas,
                                    const BitmapEx&         rBmpEx ) :
            CanvasGraphicHelper( rCanvas ),
            maActions()
        {
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
                ActionRenderer() :
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
                    mbRet &= rAction.mpAction->render();
                }

            private:
                bool                    mbRet;
            };
        }

        bool ImplRenderer::drawSubset( int  startIndex,
                                       int  endIndex ) const
        {
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

            // render subset of actions
            return ::std::for_each( aIterBegin, aIterEnd, ActionRenderer() ).result();
        }

        bool ImplRenderer::draw() const
        {
            return ::std::for_each( maActions.begin(), maActions.end(), ActionRenderer() ).result();
        }
    }
}
