/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: transparencygroupaction.cxx,v $
 * $Revision: 1.11 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_cppcanvas.hxx"

#include <tools/gen.hxx>

#include <canvas/debug.hxx>
#include <canvas/verbosetrace.hxx>
#include <canvas/canvastools.hxx>

#include <rtl/logfile.hxx>

#include <com/sun/star/rendering/XBitmap.hpp>

#include <rtl/math.hxx>

#include <vcl/metaact.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/canvastools.hxx>
#include <vcl/svapp.hxx>
#include <vcl/outdev.hxx>
#include <vcl/virdev.hxx>
#include <vcl/virdev.hxx>
#include <vcl/gdimtf.hxx>
#include <vcl/gradient.hxx>

#include <canvas/canvastools.hxx>

#include <basegfx/range/b2drange.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/vector/b2dsize.hxx>
#include <basegfx/numeric/ftools.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/tuple/b2dtuple.hxx>
#include <basegfx/tools/canvastools.hxx>

#include <boost/utility.hpp>

#include "transparencygroupaction.hxx"
#include "outdevstate.hxx"
#include "mtftools.hxx"
#include "cppcanvas/vclfactory.hxx"


using namespace ::com::sun::star;

namespace cppcanvas
{
    namespace internal
    {
        // free support functions
        // ======================
        namespace
        {
            class TransparencyGroupAction : public Action, private ::boost::noncopyable
            {
            public:
                /** Create new transparency group action.

                    @param rGroupMtf
                    Metafile that groups all actions to be rendered
                    transparent

                    @param rParms
                    Render parameters

                    @param rDstPoint
                    Left, top edge of destination, in current state
                    coordinate system

                    @param rDstSize
                    Size of the transparency group object, in current
                    state coordinate system.

                    @param nAlpha
                    Alpha value, must be in the range [0,1]
                */
                TransparencyGroupAction( MtfAutoPtr&                    rGroupMtf,
                                         const Renderer::Parameters&    rParms,
                                         const ::basegfx::B2DPoint&     rDstPoint,
                                         const ::basegfx::B2DVector&    rDstSize,
                                         double                         nAlpha,
                                         const CanvasSharedPtr&         rCanvas,
                                         const OutDevState&             rState );

                /** Create new transparency group action.

                    @param rGroupMtf
                    Metafile that groups all actions to be rendered
                    transparent.

                    @param rAlphaGradient
                    VCL gradient, to be rendered into the action's alpha
                    channel.

                    @param rParms
                    Render parameters

                    @param rDstPoint
                    Left, top edge of destination, in current state
                    coordinate system

                    @param rDstSize
                    Size of the transparency group object, in current
                    state coordinate system.
                */
                TransparencyGroupAction( MtfAutoPtr&                    rGroupMtf,
                                         GradientAutoPtr&               rAlphaGradient,
                                         const Renderer::Parameters&    rParms,
                                         const ::basegfx::B2DPoint&     rDstPoint,
                                         const ::basegfx::B2DVector&    rDstSize,
                                         const CanvasSharedPtr&         rCanvas,
                                         const OutDevState&             rState );

                virtual bool render( const ::basegfx::B2DHomMatrix& rTransformation ) const;
                virtual bool render( const ::basegfx::B2DHomMatrix& rTransformation,
                                     const Subset&                  rSubset ) const;

                virtual ::basegfx::B2DRange getBounds( const ::basegfx::B2DHomMatrix& rTransformation ) const;
                virtual ::basegfx::B2DRange getBounds( const ::basegfx::B2DHomMatrix&   rTransformation,
                                                       const Subset&                    rSubset ) const;

                virtual sal_Int32 getActionCount() const;

            private:
                MtfAutoPtr                                          mpGroupMtf;
                GradientAutoPtr                                     mpAlphaGradient;

                const Renderer::Parameters                          maParms;

                const ::basegfx::B2DSize                            maDstSize;

                mutable uno::Reference< rendering::XBitmap >        mxBufferBitmap; // contains last rendered version
                mutable ::basegfx::B2DHomMatrix                     maLastTransformation; // contains last active transformation
                mutable Subset                                      maLastSubset; // contains last effective subset

                // transformation for
                // mxBufferBitmap content
                CanvasSharedPtr                                     mpCanvas;
                rendering::RenderState                              maState;
                const double                                        mnAlpha;
            };


            /** Setup transformation such that the next render call is
                moved rPoint away, and scaled according to the ratio
                given by src and dst size.
            */
            void implSetupTransform( rendering::RenderState&    rRenderState,
                                     const ::basegfx::B2DPoint& rDstPoint   )
            {
                ::basegfx::B2DHomMatrix aLocalTransformation;

                aLocalTransformation.translate( rDstPoint.getX(),
                                                rDstPoint.getY() );
                ::canvas::tools::appendToRenderState( rRenderState,
                                                      aLocalTransformation );
            }

            TransparencyGroupAction::TransparencyGroupAction( MtfAutoPtr&                   rGroupMtf,
                                                              const Renderer::Parameters&   rParms,
                                                              const ::basegfx::B2DPoint&    rDstPoint,
                                                              const ::basegfx::B2DVector&   rDstSize,
                                                              double                        nAlpha,
                                                              const CanvasSharedPtr&        rCanvas,
                                                              const OutDevState&            rState ) :
                mpGroupMtf( rGroupMtf ),
                mpAlphaGradient(),
                maParms( rParms ),
                maDstSize( rDstSize ),
                mxBufferBitmap(),
                maLastTransformation(),
                mpCanvas( rCanvas ),
                maState(),
                mnAlpha( nAlpha )
            {
                tools::initRenderState(maState,rState);
                implSetupTransform( maState, rDstPoint );

                // correct clip (which is relative to original transform)
                tools::modifyClip( maState,
                                   rState,
                                   rCanvas,
                                   rDstPoint,
                                   NULL,
                                   NULL );

                maLastSubset.mnSubsetBegin = 0;
                maLastSubset.mnSubsetEnd = -1;
            }

            TransparencyGroupAction::TransparencyGroupAction( MtfAutoPtr&                   rGroupMtf,
                                                              GradientAutoPtr&              rAlphaGradient,
                                                              const Renderer::Parameters&   rParms,
                                                              const ::basegfx::B2DPoint&    rDstPoint,
                                                              const ::basegfx::B2DVector&   rDstSize,
                                                              const CanvasSharedPtr&        rCanvas,
                                                              const OutDevState&            rState ) :
                mpGroupMtf( rGroupMtf ),
                mpAlphaGradient( rAlphaGradient ),
                maParms( rParms ),
                maDstSize( rDstSize ),
                mxBufferBitmap(),
                maLastTransformation(),
                mpCanvas( rCanvas ),
                maState(),
                mnAlpha( 1.0 )
            {
                tools::initRenderState(maState,rState);
                implSetupTransform( maState, rDstPoint );

                // correct clip (which is relative to original transform)
                tools::modifyClip( maState,
                                   rState,
                                   rCanvas,
                                   rDstPoint,
                                   NULL,
                                   NULL );

                maLastSubset.mnSubsetBegin = 0;
                maLastSubset.mnSubsetEnd = -1;
            }

            // TODO(P3): The whole float transparency handling is a mess,
            // this should be refactored. What's more, the old idea of
            // having only internal 'metaactions', and not the original
            // GDIMetaFile now looks a lot less attractive. Try to move
            // into the direction of having a direct GDIMetaFile2XCanvas
            // renderer, and maybe a separate metafile XCanvas
            // implementation.
            bool TransparencyGroupAction::render( const ::basegfx::B2DHomMatrix&    rTransformation,
                                                  const Subset&                     rSubset ) const
            {
                RTL_LOGFILE_CONTEXT( aLog, "::cppcanvas::internal::TransparencyGroupAction::render()" );
                RTL_LOGFILE_CONTEXT_TRACE1( aLog, "::cppcanvas::internal::TransparencyGroupAction: 0x%X", this );

                // determine overall transformation matrix (render, view,
                // and passed transformation)
                ::basegfx::B2DHomMatrix aTransform;
                ::canvas::tools::getRenderStateTransform( aTransform, maState );
                aTransform = rTransformation * aTransform;

                ::basegfx::B2DHomMatrix aTotalTransform;
                ::canvas::tools::getViewStateTransform( aTotalTransform, mpCanvas->getViewState() );
                aTotalTransform = aTotalTransform * aTransform;

                // since pure translational changes to the transformation
                // does not matter, remove them before comparing
                aTotalTransform.set( 0, 2, 0.0 );
                aTotalTransform.set( 1, 2, 0.0 );

                // if there's no buffer bitmap, or as soon as the
                // total transformation changes, we've got to
                // re-render the bitmap
                if( !mxBufferBitmap.is() ||
                    aTotalTransform != maLastTransformation ||
                    rSubset.mnSubsetBegin != maLastSubset.mnSubsetBegin ||
                    rSubset.mnSubsetEnd != maLastSubset.mnSubsetEnd )
                {
                    DBG_TESTSOLARMUTEX();

                    // determine total scaling factor of the
                    // transformation matrix - need to make the bitmap
                    // large enough
                    ::basegfx::B2DTuple aScale;
                    ::basegfx::B2DTuple aTranslate;
                    double              nRotate;
                    double              nShearX;
                    if( !aTotalTransform.decompose( aScale,
                                                    aTranslate,
                                                    nRotate,
                                                    nShearX ) )
                    {
                        OSL_ENSURE( false,
                                    "TransparencyGroupAction::render(): non-decomposable transformation" );
                        return false;
                    }

                    // output size of metafile
                    ::Size aOutputSizePixel( ::basegfx::fround( aScale.getX() * maDstSize.getX() ),
                                             ::basegfx::fround( aScale.getY() * maDstSize.getY() ) );

                    // pixel size of cache bitmap: round up to nearest int
                    ::Size aBitmapSizePixel( static_cast<sal_Int32>( aScale.getX() * maDstSize.getX() )+1,
                                             static_cast<sal_Int32>( aScale.getY() * maDstSize.getY() )+1 );

                    ::Point aEmptyPoint;

                    // render our content into an appropriately sized
                    // VirtualDevice with alpha channel
                    VirtualDevice aVDev(
                        *::Application::GetDefaultDevice(), 0, 0 );
                    aVDev.SetOutputSizePixel( aBitmapSizePixel );
                    aVDev.SetMapMode();

                    if( rSubset.mnSubsetBegin != 0 ||
                        rSubset.mnSubsetEnd != -1 )
                    {
                        // true subset - extract referenced
                        // metaactions from mpGroupMtf
                        GDIMetaFile aMtf;
                        MetaAction* pCurrAct;
                        int         nCurrActionIndex;

                        // extract subset actions
                        for( nCurrActionIndex=0,
                                 pCurrAct=mpGroupMtf->FirstAction();
                             pCurrAct;
                             ++nCurrActionIndex, pCurrAct = mpGroupMtf->NextAction() )
                        {
                            switch( pCurrAct->GetType() )
                            {
                                case META_PUSH_ACTION:
                                case META_POP_ACTION:
                                case META_CLIPREGION_ACTION:
                                case META_ISECTRECTCLIPREGION_ACTION:
                                case META_ISECTREGIONCLIPREGION_ACTION:
                                case META_MOVECLIPREGION_ACTION:
                                case META_LINECOLOR_ACTION:
                                case META_FILLCOLOR_ACTION:
                                case META_TEXTCOLOR_ACTION:
                                case META_TEXTFILLCOLOR_ACTION:
                                case META_TEXTLINECOLOR_ACTION:
                                case META_TEXTALIGN_ACTION:
                                case META_FONT_ACTION:
                                case META_RASTEROP_ACTION:
                                case META_REFPOINT_ACTION:
                                case META_LAYOUTMODE_ACTION:
                                    // state-changing action - copy as-is
                                    aMtf.AddAction( pCurrAct->Clone() );
                                    break;

                                case META_GRADIENT_ACTION:
                                case META_HATCH_ACTION:
                                case META_EPS_ACTION:
                                case META_COMMENT_ACTION:
                                case META_POINT_ACTION:
                                case META_PIXEL_ACTION:
                                case META_LINE_ACTION:
                                case META_RECT_ACTION:
                                case META_ROUNDRECT_ACTION:
                                case META_ELLIPSE_ACTION:
                                case META_ARC_ACTION:
                                case META_PIE_ACTION:
                                case META_CHORD_ACTION:
                                case META_POLYLINE_ACTION:
                                case META_POLYGON_ACTION:
                                case META_POLYPOLYGON_ACTION:
                                case META_BMP_ACTION:
                                case META_BMPSCALE_ACTION:
                                case META_BMPSCALEPART_ACTION:
                                case META_BMPEX_ACTION:
                                case META_BMPEXSCALE_ACTION:
                                case META_BMPEXSCALEPART_ACTION:
                                case META_MASK_ACTION:
                                case META_MASKSCALE_ACTION:
                                case META_MASKSCALEPART_ACTION:
                                case META_GRADIENTEX_ACTION:
                                case META_WALLPAPER_ACTION:
                                case META_TRANSPARENT_ACTION:
                                case META_FLOATTRANSPARENT_ACTION:
                                case META_TEXT_ACTION:
                                case META_TEXTARRAY_ACTION:
                                case META_TEXTLINE_ACTION:
                                case META_TEXTRECT_ACTION:
                                case META_STRETCHTEXT_ACTION:
                                    // output-generating action - only
                                    // copy, if we're within the
                                    // requested subset
                                    if( rSubset.mnSubsetBegin <= nCurrActionIndex &&
                                        rSubset.mnSubsetEnd > nCurrActionIndex )
                                    {
                                        aMtf.AddAction( pCurrAct->Clone() );
                                    }
                                    break;

                                default:
                                    OSL_ENSURE( false,
                                                "Unknown meta action type encountered" );
                                    break;
                            }
                        }

                        aVDev.DrawTransparent( aMtf,
                                               aEmptyPoint,
                                               aOutputSizePixel,
                                               *mpAlphaGradient );
                    }
                    else
                    {
                        // no subsetting - render whole mtf
                        aVDev.DrawTransparent( *mpGroupMtf,
                                               aEmptyPoint,
                                               aOutputSizePixel,
                                               *mpAlphaGradient );
                    }


                    // update buffered bitmap and transformation
                    BitmapSharedPtr aBmp( VCLFactory::getInstance().createBitmap(
                                              mpCanvas,
                                              aVDev.GetBitmapEx(
                                                  aEmptyPoint,
                                                  aBitmapSizePixel ) ) );
                    mxBufferBitmap = aBmp->getUNOBitmap();
                    maLastTransformation = aTotalTransform;
                    maLastSubset = rSubset;
                }

                // determine target transformation (we can't simply pass
                // aTotalTransform as assembled above, since we must take
                // the canvas' view state as is, it might contain clipping
                // (which, in turn, is relative to the view
                // transformation))

                // given that aTotalTransform is the identity
                // transformation, we could simply render our bitmap
                // as-is. Now, since the mxBufferBitmap content already
                // accounts for scale changes in the overall
                // transformation, we must factor this out
                // before. Generally, the transformation matrix should be
                // structured like this:
                // Translation*Rotation*Shear*Scale. Thus, to neutralize
                // the contained scaling, we've got to right-multiply with
                // the inverse.
                ::basegfx::B2ISize aBmpSize(
                    ::basegfx::unotools::b2ISizeFromIntegerSize2D( mxBufferBitmap->getSize() ) );

                ::basegfx::B2DHomMatrix aScaleCorrection;
                aScaleCorrection.scale( (double)maDstSize.getX() / aBmpSize.getX(),
                                        (double)maDstSize.getY() / aBmpSize.getY() );
                aTransform = aTransform * aScaleCorrection;

                rendering::RenderState aLocalState( maState );
                ::canvas::tools::setRenderStateTransform(aLocalState, aTransform);

#ifdef SPECIAL_DEBUG
                aLocalState.Clip.clear();
                aLocalState.DeviceColor =
                    ::vcl::unotools::colorToDoubleSequence( mpCanvas->getUNOCanvas()->getDevice(),
                                                            ::Color( 0x80FF0000 ) );

                if( maState.Clip.is() )
                    mpCanvas->getUNOCanvas()->fillPolyPolygon( maState.Clip,
                                                               mpCanvas->getViewState(),
                                                               aLocalState );

                aLocalState.DeviceColor = maState.DeviceColor;
#endif

                if( ::rtl::math::approxEqual(mnAlpha, 1.0) )
                {
                    // no further alpha changes necessary -> draw directly
                    mpCanvas->getUNOCanvas()->drawBitmap( mxBufferBitmap,
                                                          mpCanvas->getViewState(),
                                                          aLocalState );
                }
                else
                {
                    // add alpha modulation value to DeviceColor
                    ::canvas::tools::setDeviceColor( aLocalState,
                                                     1.0, 1.0, 1.0, mnAlpha );

                    mpCanvas->getUNOCanvas()->drawBitmapModulated( mxBufferBitmap,
                                                                   mpCanvas->getViewState(),
                                                                   aLocalState );
                }

                return true;
            }

            // TODO(P3): The whole float transparency handling is a mess,
            // this should be refactored. What's more, the old idea of
            // having only internal 'metaactions', and not the original
            // GDIMetaFile now looks a lot less attractive. Try to move
            // into the direction of having a direct GDIMetaFile2XCanvas
            // renderer, and maybe a separate metafile XCanvas
            // implementation.
            bool TransparencyGroupAction::render( const ::basegfx::B2DHomMatrix& rTransformation ) const
            {
                Subset aSubset;

                aSubset.mnSubsetBegin = 0;
                aSubset.mnSubsetEnd   = -1;

                return render( rTransformation, aSubset );
            }

            ::basegfx::B2DRange TransparencyGroupAction::getBounds( const ::basegfx::B2DHomMatrix&  rTransformation ) const
            {
                rendering::RenderState aLocalState( maState );
                ::canvas::tools::prependToRenderState(aLocalState, rTransformation);

                return tools::calcDevicePixelBounds(
                    ::basegfx::B2DRange( 0,0,
                                         maDstSize.getX(),
                                         maDstSize.getY() ),
                    mpCanvas->getViewState(),
                    aLocalState );
            }

            ::basegfx::B2DRange TransparencyGroupAction::getBounds( const ::basegfx::B2DHomMatrix&  rTransformation,
                                                                    const Subset&                   rSubset ) const
            {
                // TODO(F3): Currently, the bounds for
                // TransparencyGroupAction subsets equal those of the
                // full set, although this action is able to render
                // true subsets.

                // polygon only contains a single action, empty bounds
                // if subset requests different range
                if( rSubset.mnSubsetBegin != 0 ||
                    rSubset.mnSubsetEnd != 1 )
                    return ::basegfx::B2DRange();

                return getBounds( rTransformation );
            }

            sal_Int32 TransparencyGroupAction::getActionCount() const
            {
                return mpGroupMtf.get() ? mpGroupMtf->GetActionCount() : 0;
            }

        }

        ActionSharedPtr TransparencyGroupActionFactory::createTransparencyGroupAction( MtfAutoPtr&                  rGroupMtf,
                                                                                       const Renderer::Parameters&  rParms,
                                                                                       const ::basegfx::B2DPoint&   rDstPoint,
                                                                                       const ::basegfx::B2DVector&  rDstSize,
                                                                                       double                       nAlpha,
                                                                                       const CanvasSharedPtr&       rCanvas,
                                                                                       const OutDevState&           rState )
        {
            return ActionSharedPtr( new TransparencyGroupAction(rGroupMtf,
                                                                rParms,
                                                                rDstPoint,
                                                                rDstSize,
                                                                nAlpha,
                                                                rCanvas,
                                                                rState ) );
        }

        ActionSharedPtr TransparencyGroupActionFactory::createTransparencyGroupAction( MtfAutoPtr&                  rGroupMtf,
                                                                                       GradientAutoPtr&             rAlphaGradient,
                                                                                       const Renderer::Parameters&  rParms,
                                                                                       const ::basegfx::B2DPoint&   rDstPoint,
                                                                                       const ::basegfx::B2DVector&  rDstSize,
                                                                                       const CanvasSharedPtr&       rCanvas,
                                                                                       const OutDevState&           rState )
        {
            return ActionSharedPtr( new TransparencyGroupAction(rGroupMtf,
                                                                rAlphaGradient,
                                                                rParms,
                                                                rDstPoint,
                                                                rDstSize,
                                                                rCanvas,
                                                                rState ) );
        }

    }
}
