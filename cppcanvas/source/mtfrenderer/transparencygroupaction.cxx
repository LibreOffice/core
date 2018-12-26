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

#include <utility>

#include <tools/gen.hxx>
#include <tools/debug.hxx>

#include <canvas/canvastools.hxx>

#include <com/sun/star/rendering/XBitmap.hpp>
#include <com/sun/star/rendering/XCanvas.hpp>

#include <rtl/math.hxx>

#include <vcl/metaact.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/canvastools.hxx>
#include <vcl/svapp.hxx>
#include <vcl/outdev.hxx>
#include <vcl/virdev.hxx>
#include <vcl/gdimtf.hxx>
#include <vcl/gradient.hxx>

#include <basegfx/range/b2drange.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/vector/b2dsize.hxx>
#include <basegfx/numeric/ftools.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/tuple/b2dtuple.hxx>
#include <basegfx/utils/canvastools.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <sal/log.hxx>

#include "transparencygroupaction.hxx"
#include <outdevstate.hxx>
#include "mtftools.hxx"
#include <cppcanvas/vclfactory.hxx>


using namespace ::com::sun::star;

namespace cppcanvas
{
    namespace internal
    {
        // free support functions
        // ======================
        namespace
        {
            class TransparencyGroupAction : public Action
            {
            public:
                /** Create new transparency group action.

                    @param rGroupMtf
                    Metafile that groups all actions to be rendered
                    transparent.

                    @param rAlphaGradient
                    VCL gradient, to be rendered into the action's alpha
                    channel.

                    @param rDstPoint
                    Left, top edge of destination, in current state
                    coordinate system

                    @param rDstSize
                    Size of the transparency group object, in current
                    state coordinate system.
                */
                TransparencyGroupAction( MtfAutoPtr&&                   rGroupMtf,
                                         GradientAutoPtr&&              rAlphaGradient,
                                         const ::basegfx::B2DPoint&     rDstPoint,
                                         const ::basegfx::B2DVector&    rDstSize,
                                         const CanvasSharedPtr&         rCanvas,
                                         const OutDevState&             rState );

                TransparencyGroupAction(const TransparencyGroupAction&) = delete;
                const TransparencyGroupAction& operator=(const TransparencyGroupAction&) = delete;

                virtual bool render( const ::basegfx::B2DHomMatrix& rTransformation ) const override;
                virtual bool renderSubset( const ::basegfx::B2DHomMatrix& rTransformation,
                                           const Subset&                  rSubset ) const override;

                virtual ::basegfx::B2DRange getBounds( const ::basegfx::B2DHomMatrix& rTransformation ) const override;
                virtual ::basegfx::B2DRange getBounds( const ::basegfx::B2DHomMatrix&   rTransformation,
                                                       const Subset&                    rSubset ) const override;

                virtual sal_Int32 getActionCount() const override;

            private:
                MtfAutoPtr                                          mpGroupMtf;
                GradientAutoPtr                                     mpAlphaGradient;

                const ::basegfx::B2DSize                            maDstSize;

                mutable uno::Reference< rendering::XBitmap >        mxBufferBitmap; // contains last rendered version
                mutable ::basegfx::B2DHomMatrix                     maLastTransformation; // contains last active transformation
                mutable Subset                                      maLastSubset; // contains last effective subset

                // transformation for
                // mxBufferBitmap content
                CanvasSharedPtr                                     mpCanvas;
                rendering::RenderState                              maState;
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

            TransparencyGroupAction::TransparencyGroupAction( MtfAutoPtr&&                  rGroupMtf,
                                                              GradientAutoPtr&&             rAlphaGradient,
                                                              const ::basegfx::B2DPoint&    rDstPoint,
                                                              const ::basegfx::B2DVector&   rDstSize,
                                                              const CanvasSharedPtr&        rCanvas,
                                                              const OutDevState&            rState ) :
                mpGroupMtf( std::move(rGroupMtf) ),
                mpAlphaGradient( std::move(rAlphaGradient) ),
                maDstSize( rDstSize ),
                mxBufferBitmap(),
                maLastTransformation(),
                mpCanvas( rCanvas ),
                maState()
            {
                tools::initRenderState(maState,rState);
                implSetupTransform( maState, rDstPoint );

                // correct clip (which is relative to original transform)
                tools::modifyClip( maState,
                                   rState,
                                   rCanvas,
                                   rDstPoint,
                                   nullptr,
                                   nullptr );

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
            bool TransparencyGroupAction::renderSubset( const ::basegfx::B2DHomMatrix&    rTransformation,
                                                        const Subset&                     rSubset ) const
            {
                SAL_INFO( "cppcanvas.emf", "::cppcanvas::internal::TransparencyGroupAction::renderSubset()" );
                SAL_INFO( "cppcanvas.emf", "::cppcanvas::internal::TransparencyGroupAction: 0x" << std::hex << this );

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
                    SAL_WARN( "cppcanvas.emf", "TransparencyGroupAction::renderSubset(): non-decomposable transformation" );
                    return false;
                }

                // if there's no buffer bitmap, or as soon as the
                // total transformation changes, we've got to
                // re-render the bitmap
                if( !mxBufferBitmap.is() ||
                    aTotalTransform != maLastTransformation ||
                    rSubset.mnSubsetBegin != maLastSubset.mnSubsetBegin ||
                    rSubset.mnSubsetEnd != maLastSubset.mnSubsetEnd )
                {
                    DBG_TESTSOLARMUTEX();

                    // output size of metafile
                    ::Size aOutputSizePixel( ::basegfx::fround( aScale.getX() * maDstSize.getX() ),
                                             ::basegfx::fround( aScale.getY() * maDstSize.getY() ) );

                    // pixel size of cache bitmap: round up to nearest int
                    ::Size aBitmapSizePixel( static_cast<sal_Int32>( aScale.getX() * maDstSize.getX() )+1,
                                             static_cast<sal_Int32>( aScale.getY() * maDstSize.getY() )+1 );

                    ::Point aEmptyPoint;

                    // render our content into an appropriately sized
                    // VirtualDevice with alpha channel
                    ScopedVclPtrInstance<VirtualDevice> aVDev(
                        *::Application::GetDefaultDevice(), DeviceFormat::DEFAULT, DeviceFormat::DEFAULT );
                    aVDev->SetOutputSizePixel( aBitmapSizePixel );
                    aVDev->SetMapMode();

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
                                case MetaActionType::PUSH:
                                case MetaActionType::POP:
                                case MetaActionType::CLIPREGION:
                                case MetaActionType::ISECTRECTCLIPREGION:
                                case MetaActionType::ISECTREGIONCLIPREGION:
                                case MetaActionType::MOVECLIPREGION:
                                case MetaActionType::LINECOLOR:
                                case MetaActionType::FILLCOLOR:
                                case MetaActionType::TEXTCOLOR:
                                case MetaActionType::TEXTFILLCOLOR:
                                case MetaActionType::TEXTLINECOLOR:
                                case MetaActionType::TEXTALIGN:
                                case MetaActionType::FONT:
                                case MetaActionType::RASTEROP:
                                case MetaActionType::REFPOINT:
                                case MetaActionType::LAYOUTMODE:
                                    // state-changing action - copy as-is
                                    aMtf.AddAction( pCurrAct->Clone() );
                                    break;

                                case MetaActionType::GRADIENT:
                                case MetaActionType::HATCH:
                                case MetaActionType::EPS:
                                case MetaActionType::COMMENT:
                                case MetaActionType::POINT:
                                case MetaActionType::PIXEL:
                                case MetaActionType::LINE:
                                case MetaActionType::RECT:
                                case MetaActionType::ROUNDRECT:
                                case MetaActionType::ELLIPSE:
                                case MetaActionType::ARC:
                                case MetaActionType::PIE:
                                case MetaActionType::CHORD:
                                case MetaActionType::POLYLINE:
                                case MetaActionType::POLYGON:
                                case MetaActionType::POLYPOLYGON:
                                case MetaActionType::BMP:
                                case MetaActionType::BMPSCALE:
                                case MetaActionType::BMPSCALEPART:
                                case MetaActionType::BMPEX:
                                case MetaActionType::BMPEXSCALE:
                                case MetaActionType::BMPEXSCALEPART:
                                case MetaActionType::MASK:
                                case MetaActionType::MASKSCALE:
                                case MetaActionType::MASKSCALEPART:
                                case MetaActionType::GRADIENTEX:
                                case MetaActionType::WALLPAPER:
                                case MetaActionType::Transparent:
                                case MetaActionType::FLOATTRANSPARENT:
                                case MetaActionType::TEXT:
                                case MetaActionType::TEXTARRAY:
                                case MetaActionType::TEXTLINE:
                                case MetaActionType::TEXTRECT:
                                case MetaActionType::STRETCHTEXT:
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
                                    SAL_WARN( "cppcanvas.emf", "Unknown meta action type encountered" );
                                    break;
                            }
                        }

                        aVDev->DrawTransparent( aMtf,
                                               aEmptyPoint,
                                               aOutputSizePixel,
                                               *mpAlphaGradient );
                    }
                    else
                    {
                        // no subsetting - render whole mtf
                        aVDev->DrawTransparent( *mpGroupMtf,
                                               aEmptyPoint,
                                               aOutputSizePixel,
                                               *mpAlphaGradient );
                    }


                    // update buffered bitmap and transformation
                    BitmapSharedPtr aBmp( VCLFactory::createBitmap(
                                              mpCanvas,
                                              aVDev->GetBitmapEx(
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
                ::basegfx::B2DHomMatrix aScaleCorrection;
                aScaleCorrection.scale( 1/aScale.getX(), 1/aScale.getY() );
                aTransform = aTransform * aScaleCorrection;

                rendering::RenderState aLocalState( maState );
                ::canvas::tools::setRenderStateTransform(aLocalState, aTransform);

                if(aLocalState.Clip.is())
                {
                    // tdf#95709
                    // Adjust renderstate clip to modified scale from above
                    ::basegfx::B2DPolyPolygon aClip = ::basegfx::unotools::b2DPolyPolygonFromXPolyPolygon2D(aLocalState.Clip);
                    aClip.transform(basegfx::utils::createScaleB2DHomMatrix(aScale));
                    aLocalState.Clip = ::basegfx::unotools::xPolyPolygonFromB2DPolyPolygon(mpCanvas->getUNOCanvas()->getDevice(), aClip);
                }

#if OSL_DEBUG_LEVEL > 2
                aLocalState.Clip.clear();
                aLocalState.DeviceColor =
                    vcl::unotools::colorToDoubleSequence(
                        ::Color( 0x80FF0000 ),
                        mpCanvas->getUNOCanvas()->getDevice()->getDeviceColorSpace() );

                if( maState.Clip.is() )
                    mpCanvas->getUNOCanvas()->fillPolyPolygon( maState.Clip,
                                                               mpCanvas->getViewState(),
                                                               aLocalState );

                aLocalState.DeviceColor = maState.DeviceColor;
#endif

                // no further alpha changes necessary -> draw directly
                mpCanvas->getUNOCanvas()->drawBitmap( mxBufferBitmap,
                                                      mpCanvas->getViewState(),
                                                      aLocalState );
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

                return renderSubset( rTransformation, aSubset );
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
                return mpGroupMtf.get() ? mpGroupMtf->GetActionSize() : 0;
            }

        }

        std::shared_ptr<Action> TransparencyGroupActionFactory::createTransparencyGroupAction( MtfAutoPtr&&                 rGroupMtf,
                                                                                       GradientAutoPtr&&            rAlphaGradient,
                                                                                       const ::basegfx::B2DPoint&   rDstPoint,
                                                                                       const ::basegfx::B2DVector&  rDstSize,
                                                                                       const CanvasSharedPtr&       rCanvas,
                                                                                       const OutDevState&           rState )
        {
            return std::shared_ptr<Action>( new TransparencyGroupAction(std::move(rGroupMtf),
                                                                std::move(rAlphaGradient),
                                                                rDstPoint,
                                                                rDstSize,
                                                                rCanvas,
                                                                rState ) );
        }

    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
