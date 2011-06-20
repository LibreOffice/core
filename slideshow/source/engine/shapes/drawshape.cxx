/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
#include "precompiled_slideshow.hxx"

// must be first
#include <canvas/debug.hxx>
#include <tools/diagnose_ex.h>
#include <canvas/verbosetrace.hxx>

#include <rtl/logfile.hxx>
#include <osl/diagnose.hxx>
#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/awt/FontWeight.hpp>
#include <comphelper/anytostring.hxx>
#include <cppuhelper/exc_hlp.hxx>

#include <vcl/metaact.hxx>
#include <vcl/gdimtf.hxx>
#include <vcl/wrkwin.hxx>

#include <basegfx/numeric/ftools.hxx>
#include <basegfx/range/rangeexpander.hxx>

#include <rtl/math.hxx>

#include <com/sun/star/drawing/TextAnimationKind.hpp>

#include <vcl/svapp.hxx>
#include <vcl/window.hxx>
#include <tools/stream.hxx>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/datatransfer/XTransferable.hpp>

#include <comphelper/scopeguard.hxx>
#include <canvas/canvastools.hxx>

#include <cmath> // for trigonometry and fabs
#include <algorithm>
#include <functional>
#include <limits>

#include "drawshapesubsetting.hxx"
#include "drawshape.hxx"
#include "eventqueue.hxx"
#include "wakeupevent.hxx"
#include "subsettableshapemanager.hxx"
#include "intrinsicanimationactivity.hxx"
#include "slideshowexceptions.hxx"
#include "tools.hxx"
#include "gdimtftools.hxx"
#include "drawinglayeranimation.hxx"

#include <boost/bind.hpp>
#include <math.h>

using namespace ::com::sun::star;


namespace slideshow
{
    namespace internal
    {
        //#i75867# poor quality of ole's alternative view with 3D scenes and zoomfactors besides 100%
        //metafiles are resolution dependent when bitmaps are contained with is the case for 3D scenes for example
        //in addition a chart has resolution dependent content as it might skip points that are not visible for a given resolution (this is done for performance reasons)
        bool local_getMetafileForChart( const uno::Reference< lang::XComponent >&     xSource,
                  const uno::Reference< drawing::XDrawPage >&     xContainingPage,
                  GDIMetaFile&                                    rMtf )
        {
            //get the chart model
            uno::Reference< beans::XPropertySet > xPropSet( xSource, uno::UNO_QUERY );
            uno::Reference< frame::XModel > xChartModel;
            getPropertyValue( xChartModel, xPropSet, OUSTR("Model"));
            uno::Reference< lang::XMultiServiceFactory > xFact( xChartModel, uno::UNO_QUERY );
            OSL_ENSURE( xFact.is(), "Chart cannot be painted pretty!\n" );
            if(!xFact.is())
                return false;

            //get the chart view
            uno::Reference< datatransfer::XTransferable > xChartViewTransferable(
                xFact->createInstance( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.chart2.ChartView" ) ) ), uno::UNO_QUERY );
            uno::Reference< beans::XPropertySet > xChartViewProp( xChartViewTransferable, uno::UNO_QUERY );
            OSL_ENSURE( xChartViewProp.is(), "Chart cannot be painted pretty!\n" );
            if( !xChartViewProp.is() )
                return false;

            //estimate zoom and resolution (this is only a workaround, correct would be to know and use the exact zoom and resoltion during slideshow display)
            sal_Int32 nScaleXNumerator = 100;//zoom factor -> exact values are important for the quality of the created bitmap especially for 3D charts
            sal_Int32 nScaleYNumerator = 100;
            sal_Int32 nScaleXDenominator = 100;
            sal_Int32 nScaleYDenominator = 100;
            awt::Size aPixelPerChart( 1000, 1000 );//when data points happen to be on the same pixel as their predecessor no shape is created to safe performance

            Window* pActiveTopWindow( Application::GetActiveTopWindow() );
            WorkWindow* pWorkWindow( dynamic_cast<WorkWindow*>(pActiveTopWindow));
            if( pWorkWindow && pWorkWindow->IsPresentationMode() )
            {
                Size aPixScreenSize( pActiveTopWindow->GetOutputSizePixel() );
                aPixelPerChart = awt::Size( aPixScreenSize.getWidth(), aPixScreenSize.getHeight() );//this is still to much (but costs only seldom performance), correct would be pixel per chart object

                uno::Reference< beans::XPropertySet > xPageProp( xContainingPage, uno::UNO_QUERY );
                sal_Int32 nLogicPageWidth=1;
                sal_Int32 nLogicPageHeight=1;
                if( getPropertyValue( nLogicPageWidth, xPageProp, OUSTR("Width")) &&
                    getPropertyValue( nLogicPageHeight, xPageProp, OUSTR("Height")) )
                {
                    Size aLogicScreenSize( pActiveTopWindow->PixelToLogic( aPixScreenSize, MAP_100TH_MM ) );
                    nScaleXNumerator = aLogicScreenSize.getWidth();
                    nScaleYNumerator = aLogicScreenSize.getHeight();
                    nScaleXDenominator = nLogicPageWidth;
                    nScaleYDenominator = nLogicPageHeight;
                }
            }
            else
            {
                long nMaxPixWidth = 0;
                long nMaxPixHeight = 0;
                unsigned int nScreenCount( Application::GetScreenCount() );
                for( unsigned int nScreen=0; nScreen<nScreenCount; nScreen++ )
                {
                    Rectangle aCurScreenRect( Application::GetScreenPosSizePixel( nScreen ) );
                    if( aCurScreenRect.GetWidth() > nMaxPixWidth )
                        nMaxPixWidth = aCurScreenRect.GetWidth();
                    if( aCurScreenRect.GetHeight() > nMaxPixHeight )
                        nMaxPixHeight = aCurScreenRect.GetHeight();
                }
                if(nMaxPixWidth>1 && nMaxPixHeight>1)
                    aPixelPerChart = awt::Size( nMaxPixWidth, nMaxPixHeight );//this is still to much (but costs only seldom performance), correct would be pixel per chart object
            }

            try
            {
                uno::Sequence< beans::PropertyValue > aZoomFactors(4);
                aZoomFactors[0].Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ScaleXNumerator") );
                aZoomFactors[0].Value = uno::makeAny( nScaleXNumerator );
                aZoomFactors[1].Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ScaleXDenominator") );
                aZoomFactors[1].Value = uno::makeAny( nScaleXDenominator );
                aZoomFactors[2].Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ScaleYNumerator") );
                aZoomFactors[2].Value = uno::makeAny( nScaleYNumerator );
                aZoomFactors[3].Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ScaleYDenominator") );
                aZoomFactors[3].Value = uno::makeAny( nScaleYDenominator );

                xChartViewProp->setPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ZoomFactors") ), uno::makeAny( aZoomFactors ));
                xChartViewProp->setPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Resolution") ), uno::makeAny( aPixelPerChart ));
            }
            catch (uno::Exception &)
            {
                OSL_FAIL( rtl::OUStringToOString(
                                comphelper::anyToString(
                                    cppu::getCaughtException() ),
                                RTL_TEXTENCODING_UTF8 ).getStr() );
            }

            //get a metafile from the prepared chart view
            datatransfer::DataFlavor aDataFlavor(
                    ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("application/x-openoffice-gdimetafile;windows_formatname=\"GDIMetaFile\"") ),
                    ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "GDIMetaFile" ) ),
                    ::getCppuType( (const uno::Sequence< sal_Int8 >*) 0 ) );
            uno::Any aData( xChartViewTransferable->getTransferData( aDataFlavor ) );
            uno::Sequence< sal_Int8 > aSeq;
            if( aData >>= aSeq )
            {
                ::std::auto_ptr< SvMemoryStream > pSrcStm( new SvMemoryStream( (char*) aSeq.getConstArray(), aSeq.getLength(), STREAM_WRITE | STREAM_TRUNC ) );
                *(pSrcStm.get() ) >> rMtf;
                return true;
            }
            return false;
        }

        //same as getMetafile with an exception for charts
        //for charts a metafile with a higher resolution is created, because charts have resolution dependent content
        bool local_getMetaFile_WithSpecialChartHandling( const uno::Reference< lang::XComponent >&    xSource,
                  const uno::Reference< drawing::XDrawPage >&     xContainingPage,
                  GDIMetaFile&                                    rMtf,
                  int                                             mtfLoadFlags,
                  const uno::Reference< uno::XComponentContext >& rxContext )
        {
            uno::Reference<beans::XPropertySet> xProp( xSource, uno::UNO_QUERY );
            rtl::OUString sCLSID;
            getPropertyValue( sCLSID, xProp, OUSTR("CLSID"));
            if( sCLSID.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("12DCAE26-281F-416F-a234-c3086127382e")) && local_getMetafileForChart( xSource, xContainingPage, rMtf ) )
                return true;
            return getMetaFile( xSource, xContainingPage, rMtf, mtfLoadFlags, rxContext );
        }


        //////////////////////////////////////////////////////////////////////
        //
        // Private methods
        //
        //////////////////////////////////////////////////////////////////////

        GDIMetaFileSharedPtr DrawShape::forceScrollTextMetaFile()
        {
            if ((mnCurrMtfLoadFlags & MTF_LOAD_SCROLL_TEXT_MTF) != MTF_LOAD_SCROLL_TEXT_MTF)
            {
                // reload with added flags:
                mpCurrMtf.reset( new GDIMetaFile );
                mnCurrMtfLoadFlags |= MTF_LOAD_SCROLL_TEXT_MTF;
                local_getMetaFile_WithSpecialChartHandling(
                    uno::Reference<lang::XComponent>(mxShape, uno::UNO_QUERY),
                    mxPage, *mpCurrMtf, mnCurrMtfLoadFlags,
                    mxComponentContext );

                // TODO(F1): Currently, the scroll metafile will
                // never contain any verbose text comments. Thus,
                // can only display the full mtf content, no
                // subsets.
                maSubsetting.reset( mpCurrMtf );

                // adapt maBounds. the requested scroll text metafile
                // will typically have dimension different from the
                // actual shape
                ::basegfx::B2DRectangle aScrollRect, aPaintRect;
                ENSURE_OR_THROW( getRectanglesFromScrollMtf( aScrollRect,
                                                              aPaintRect,
                                                              mpCurrMtf ),
                                  "DrawShape::forceScrollTextMetaFile(): Could "
                                  "not extract scroll anim rectangles from mtf" );

                // take the larger one of the two rectangles (that
                // should be the bound rect of the retrieved
                // metafile)
                if( aScrollRect.isInside( aPaintRect ) )
                    maBounds = aScrollRect;
                else
                    maBounds = aPaintRect;
            }
            return mpCurrMtf;
        }

        void DrawShape::updateStateIds() const
        {
            // Update the states, we've just redrawn or created a new
            // attribute layer.
            if( mpAttributeLayer )
            {
                mnAttributeTransformationState = mpAttributeLayer->getTransformationState();
                mnAttributeClipState = mpAttributeLayer->getClipState();
                mnAttributeAlphaState = mpAttributeLayer->getAlphaState();
                mnAttributePositionState = mpAttributeLayer->getPositionState();
                mnAttributeContentState = mpAttributeLayer->getContentState();
                mnAttributeVisibilityState = mpAttributeLayer->getVisibilityState();
            }
        }

        void DrawShape::ensureVerboseMtfComments() const
        {
            // TODO(F1): Text effects don't currently work for drawing
            // layer animations.

            // only touch mpCurrMtf, if we're not a DrawingLayer
            // animation.
            if( (mnCurrMtfLoadFlags & MTF_LOAD_VERBOSE_COMMENTS) == 0 &&
                maAnimationFrames.empty() )
            {
                ENSURE_OR_THROW( !maSubsetting.hasSubsetShapes(),
                                  "DrawShape::ensureVerboseMtfComments(): reloading the metafile "
                                  "with active child subsets will wreak havoc on the view!" );
                ENSURE_OR_THROW( maSubsetting.getSubsetNode().isEmpty(),
                                  "DrawShape::ensureVerboseMtfComments(): reloading the metafile "
                                  "for an ALREADY SUBSETTED shape is not possible!" );

                // re-fetch metafile with comments
                // note that, in case of shapes without text, the new
                // metafile might still not provide any useful
                // subsetting information!
                mpCurrMtf.reset( new GDIMetaFile );
                mnCurrMtfLoadFlags |= MTF_LOAD_VERBOSE_COMMENTS;
                local_getMetaFile_WithSpecialChartHandling(
                    uno::Reference<lang::XComponent>(mxShape, uno::UNO_QUERY),
                    mxPage, *mpCurrMtf, mnCurrMtfLoadFlags,
                    mxComponentContext );

                maSubsetting.reset( maSubsetting.getSubsetNode(),
                                    mpCurrMtf );
            }
        }

        ViewShape::RenderArgs DrawShape::getViewRenderArgs() const
        {
            return ViewShape::RenderArgs(
                maBounds,
                getUpdateArea(),
                getBounds(),
                getActualUnitShapeBounds(),
                mpAttributeLayer,
                maSubsetting.getActiveSubsets(),
                mnPriority);
        }

        bool DrawShape::implRender( int nUpdateFlags ) const
        {
            RTL_LOGFILE_CONTEXT( aLog, "::presentation::internal::DrawShape::implRender()" );
            RTL_LOGFILE_CONTEXT_TRACE1( aLog, "::presentation::internal::DrawShape: 0x%X", this );

            // will perform the update now, clear update-enforcing
            // flags
            mbForceUpdate = false;
            mbAttributeLayerRevoked = false;

            ENSURE_OR_RETURN_FALSE( !maViewShapes.empty(),
                               "DrawShape::implRender(): render called on DrawShape without views" );

            if( maBounds.isEmpty() )
            {
                // zero-sized shapes are effectively invisible,
                // thus, we save us the rendering...
                return true;
            }

            // redraw all view shapes, by calling their update() method
            if( ::std::count_if( maViewShapes.begin(),
                                 maViewShapes.end(),
                                 ::boost::bind<bool>(
                                     ::boost::mem_fn( &ViewShape::update ), // though _theoretically_,
                                                                             // bind should eat this even
                                                                             // with _1 being a shared_ptr,
                                                                             // it does _not_ for MSVC without
                                                                             // the extra mem_fn. WTF.
                                     _1,
                                     ::boost::cref( mpCurrMtf ),
                                     ::boost::cref(
                                         getViewRenderArgs() ),
                                     nUpdateFlags,
                                     isVisible() ) )
                != static_cast<ViewShapeVector::difference_type>(maViewShapes.size()) )
            {
                // at least one of the ViewShape::update() calls did return
                // false - update failed on at least one ViewLayer
                return false;
            }

            // successfully redrawn - update state IDs to detect next changes
            updateStateIds();

            return true;
        }

        int DrawShape::getUpdateFlags() const
        {
            // default: update nothing, unless ShapeAttributeStack
            // tells us below, or if the attribute layer was revoked
            int nUpdateFlags(ViewShape::NONE);

            // possibly the whole shape content changed
            if( mbAttributeLayerRevoked )
                nUpdateFlags = ViewShape::CONTENT;


            // determine what has to be updated
            // --------------------------------

            // do we have an attribute layer?
            if( mpAttributeLayer )
            {
                // Prevent nUpdateFlags to be modified when the shape is not
                // visible, except when it just was hidden.
                if (mpAttributeLayer->getVisibility()
                    || mpAttributeLayer->getVisibilityState() != mnAttributeVisibilityState )
                {
                    if (mpAttributeLayer->getVisibilityState() != mnAttributeVisibilityState )
                    {
                        // Change of the visibility state is mapped to
                        // content change because when the visibility
                        // changes then usually a sprite is shown or hidden
                        // and the background under has to be painted once.
                        nUpdateFlags |= ViewShape::CONTENT;
                    }

                    // TODO(P1): This can be done without conditional branching.
                    // See HAKMEM.
                    if( mpAttributeLayer->getPositionState() != mnAttributePositionState )
                    {
                        nUpdateFlags |= ViewShape::POSITION;
                    }
                    if( mpAttributeLayer->getAlphaState() != mnAttributeAlphaState )
                    {
                        nUpdateFlags |= ViewShape::ALPHA;
                    }
                    if( mpAttributeLayer->getClipState() != mnAttributeClipState )
                    {
                        nUpdateFlags |= ViewShape::CLIP;
                    }
                    if( mpAttributeLayer->getTransformationState() != mnAttributeTransformationState )
                    {
                        nUpdateFlags |= ViewShape::TRANSFORMATION;
                    }
                    if( mpAttributeLayer->getContentState() != mnAttributeContentState )
                    {
                        nUpdateFlags |= ViewShape::CONTENT;
                    }
                }
            }

            return nUpdateFlags;
        }

        ::basegfx::B2DRectangle DrawShape::getActualUnitShapeBounds() const
        {
            ENSURE_OR_THROW( !maViewShapes.empty(),
                              "DrawShape::getActualUnitShapeBounds(): called on DrawShape without views" );

            const VectorOfDocTreeNodes& rSubsets(
                maSubsetting.getActiveSubsets() );

            const ::basegfx::B2DRectangle aDefaultBounds( 0.0,0.0,1.0,1.0 );

            // perform the cheapest check first
            if( rSubsets.empty() )
            {
                // if subset contains the whole shape, no need to call
                // the somewhat expensive bound calculation, since as
                // long as the subset is empty, this branch will be
                // taken.
                return aDefaultBounds;
            }
            else
            {
                OSL_ENSURE( rSubsets.size() != 1 ||
                            !rSubsets.front().isEmpty(),
                            "DrawShape::getActualUnitShapeBounds() expects a "
                            "_non-empty_ subset vector for a subsetted shape!" );

                // are the cached bounds still valid?
                if( !maCurrentShapeUnitBounds )
                {
                    // no, (re)generate them
                    // =====================

                    // setup cached values to defaults (might fail to
                    // retrieve true bounds below)
                    maCurrentShapeUnitBounds.reset( aDefaultBounds );

                    // TODO(P2): the subset of the master shape (that from
                    // which the subsets are subtracted) changes
                    // relatively often (every time a subset shape is
                    // added or removed). Maybe we should exclude it here,
                    // always assuming full bounds?

                    ::cppcanvas::CanvasSharedPtr pDestinationCanvas(
                        maViewShapes.front()->getViewLayer()->getCanvas() );

                    // TODO(Q2): Although this _is_ currently
                    // view-agnostic, it might not stay like
                    // that. Maybe this method should again be moved
                    // to the ViewShape
                    ::cppcanvas::RendererSharedPtr pRenderer(
                        maViewShapes.front()->getRenderer(
                            pDestinationCanvas, mpCurrMtf, mpAttributeLayer ) );

                    // If we cannot not prefetch, be defensive and assume
                    // full shape size
                    if( pRenderer )
                    {
                        // temporarily, switch total transformation to identity
                        // (need the bounds in the [0,1]x[0,1] unit coordinate
                        // system.
                        ::basegfx::B2DHomMatrix      aEmptyTransformation;

                        ::basegfx::B2DHomMatrix      aOldTransform( pDestinationCanvas->getTransformation() );
                        pDestinationCanvas->setTransformation( aEmptyTransformation );
                        pRenderer->setTransformation( aEmptyTransformation );

                        // restore old transformation when leaving the scope
                        const ::comphelper::ScopeGuard aGuard(
                            boost::bind( &::cppcanvas::Canvas::setTransformation,
                                         pDestinationCanvas, aOldTransform ) );


                        // retrieve bounds for subset of whole metafile
                        // --------------------------------------------

                        ::basegfx::B2DRange aTotalBounds;

                        // cannot use ::boost::bind, ::basegfx::B2DRange::expand()
                        // is overloaded.
                        VectorOfDocTreeNodes::const_iterator        aCurr( rSubsets.begin() );
                        const VectorOfDocTreeNodes::const_iterator  aEnd( rSubsets.end() );
                        while( aCurr != aEnd )
                        {
                            aTotalBounds.expand( pRenderer->getSubsetArea(
                                                     aCurr->getStartIndex(),
                                                     aCurr->getEndIndex() )  );
                            ++aCurr;
                        }

                        OSL_ENSURE( aTotalBounds.getMinX() >= -0.1 &&
                                    aTotalBounds.getMinY() >= -0.1 &&
                                    aTotalBounds.getMaxX() <= 1.1 &&
                                    aTotalBounds.getMaxY() <= 1.1,
                                    "DrawShape::getActualUnitShapeBounds(): bounds noticeably larger than original shape - clipping!" );

                        // really make sure no shape appears larger than its
                        // original bounds (there _are_ some pathologic cases,
                        // especially when imported from PPT, that have
                        // e.g. obscenely large polygon bounds)
                        aTotalBounds.intersect(
                            ::basegfx::B2DRange( 0.0, 0.0,
                                                 1.0, 1.0 ));

                        maCurrentShapeUnitBounds.reset( aTotalBounds );
                    }
                }

                return *maCurrentShapeUnitBounds;
            }
        }

        DrawShape::DrawShape( const uno::Reference< drawing::XShape >&      xShape,
                              const uno::Reference< drawing::XDrawPage >&   xContainingPage,
                              double                                        nPrio,
                              bool                                          bForeignSource,
                              const SlideShowContext&                       rContext ) :
            mxShape( xShape ),
            mxPage( xContainingPage ),
            maAnimationFrames(), // empty, we don't have no intrinsic animation
            mnCurrFrame(0),
            mpCurrMtf(),
            mnCurrMtfLoadFlags( bForeignSource
                                ? MTF_LOAD_FOREIGN_SOURCE : MTF_LOAD_NONE ),
            maCurrentShapeUnitBounds(),
            mnPriority( nPrio ), // TODO(F1): When ZOrder someday becomes usable: make this ( getAPIShapePrio( xShape ) ),
            maBounds( getAPIShapeBounds( xShape ) ),
            mpAttributeLayer(),
            mpIntrinsicAnimationActivity(),
            mnAttributeTransformationState(0),
            mnAttributeClipState(0),
            mnAttributeAlphaState(0),
            mnAttributePositionState(0),
            mnAttributeContentState(0),
            mnAttributeVisibilityState(0),
            maViewShapes(),
            mxComponentContext( rContext.mxComponentContext ),
            maHyperlinkIndices(),
            maHyperlinkRegions(),
            maSubsetting(),
            mnIsAnimatedCount(0),
            mnAnimationLoopCount(0),
            meCycleMode(CYCLE_LOOP),
            mbIsVisible( true ),
            mbForceUpdate( false ),
            mbAttributeLayerRevoked( false ),
            mbDrawingLayerAnim( false )
        {
            ENSURE_OR_THROW( mxShape.is(), "DrawShape::DrawShape(): Invalid XShape" );
            ENSURE_OR_THROW( mxPage.is(), "DrawShape::DrawShape(): Invalid containing page" );

            // check for drawing layer animations:
            drawing::TextAnimationKind eKind = drawing::TextAnimationKind_NONE;
            uno::Reference<beans::XPropertySet> xPropSet( mxShape,
                                                          uno::UNO_QUERY );
            if( xPropSet.is() )
                getPropertyValue( eKind, xPropSet,
                                  OUSTR("TextAnimationKind") );
            mbDrawingLayerAnim = (eKind != drawing::TextAnimationKind_NONE);

            // must NOT be called from within initializer list, uses
            // state from mnCurrMtfLoadFlags!
            mpCurrMtf.reset( new GDIMetaFile );
            local_getMetaFile_WithSpecialChartHandling(
                uno::Reference<lang::XComponent>(xShape, uno::UNO_QUERY),
                xContainingPage, *mpCurrMtf, mnCurrMtfLoadFlags,
                mxComponentContext );
            ENSURE_OR_THROW( mpCurrMtf,
                              "DrawShape::DrawShape(): Invalid metafile" );
            maSubsetting.reset( mpCurrMtf );

            prepareHyperlinkIndices();
        }

        DrawShape::DrawShape( const uno::Reference< drawing::XShape >&      xShape,
                              const uno::Reference< drawing::XDrawPage >&   xContainingPage,
                              double                                        nPrio,
                              const Graphic&                                rGraphic,
                              const SlideShowContext&                       rContext ) :
            mxShape( xShape ),
            mxPage( xContainingPage ),
            maAnimationFrames(),
            mnCurrFrame(0),
            mpCurrMtf(),
            mnCurrMtfLoadFlags( MTF_LOAD_NONE ),
            maCurrentShapeUnitBounds(),
            mnPriority( nPrio ), // TODO(F1): When ZOrder someday becomes usable: make this ( getAPIShapePrio( xShape ) ),
            maBounds( getAPIShapeBounds( xShape ) ),
            mpAttributeLayer(),
            mpIntrinsicAnimationActivity(),
            mnAttributeTransformationState(0),
            mnAttributeClipState(0),
            mnAttributeAlphaState(0),
            mnAttributePositionState(0),
            mnAttributeContentState(0),
            mnAttributeVisibilityState(0),
            maViewShapes(),
            mxComponentContext( rContext.mxComponentContext ),
            maHyperlinkIndices(),
            maHyperlinkRegions(),
            maSubsetting(),
            mnIsAnimatedCount(0),
            mnAnimationLoopCount(0),
            meCycleMode(CYCLE_LOOP),
            mbIsVisible( true ),
            mbForceUpdate( false ),
            mbAttributeLayerRevoked( false ),
            mbDrawingLayerAnim( false )
        {
            ENSURE_OR_THROW( rGraphic.IsAnimated(),
                              "DrawShape::DrawShape(): Graphic is no animation" );

            getAnimationFromGraphic( maAnimationFrames,
                                     mnAnimationLoopCount,
                                     meCycleMode,
                                     rGraphic );

            ENSURE_OR_THROW( !maAnimationFrames.empty() &&
                              maAnimationFrames.front().mpMtf,
                              "DrawShape::DrawShape(): " );
            mpCurrMtf = maAnimationFrames.front().mpMtf;

            ENSURE_OR_THROW( mxShape.is(), "DrawShape::DrawShape(): Invalid XShape" );
            ENSURE_OR_THROW( mxPage.is(), "DrawShape::DrawShape(): Invalid containing page" );
            ENSURE_OR_THROW( mpCurrMtf, "DrawShape::DrawShape(): Invalid metafile" );
        }

        DrawShape::DrawShape( const DrawShape&      rSrc,
                              const DocTreeNode&    rTreeNode,
                              double                nPrio ) :
            mxShape( rSrc.mxShape ),
            mxPage( rSrc.mxPage ),
            maAnimationFrames(), // don't copy animations for subsets,
                                 // only the current frame!
            mnCurrFrame(0),
            mpCurrMtf( rSrc.mpCurrMtf ),
            mnCurrMtfLoadFlags( rSrc.mnCurrMtfLoadFlags ),
            maCurrentShapeUnitBounds(),
            mnPriority( nPrio ),
            maBounds( rSrc.maBounds ),
            mpAttributeLayer(),
            mpIntrinsicAnimationActivity(),
            mnAttributeTransformationState(0),
            mnAttributeClipState(0),
            mnAttributeAlphaState(0),
            mnAttributePositionState(0),
            mnAttributeContentState(0),
            mnAttributeVisibilityState(0),
            maViewShapes(),
            mxComponentContext( rSrc.mxComponentContext ),
            maHyperlinkIndices(),
            maHyperlinkRegions(),
            maSubsetting( rTreeNode, mpCurrMtf ),
            mnIsAnimatedCount(0),
            mnAnimationLoopCount(0),
            meCycleMode(CYCLE_LOOP),
            mbIsVisible( rSrc.mbIsVisible ),
            mbForceUpdate( false ),
            mbAttributeLayerRevoked( false ),
            mbDrawingLayerAnim( false )
        {
            ENSURE_OR_THROW( mxShape.is(), "DrawShape::DrawShape(): Invalid XShape" );
            ENSURE_OR_THROW( mpCurrMtf, "DrawShape::DrawShape(): Invalid metafile" );

            // xxx todo: currently not implemented for subsetted shapes;
            //           would mean modifying set of hyperlink regions when
            //           subsetting text portions. N.B.: there's already an
            //           issue for this #i72828#
        }

        //////////////////////////////////////////////////////////////////////
        //
        // Public methods
        //
        //////////////////////////////////////////////////////////////////////

        DrawShapeSharedPtr DrawShape::create(
            const uno::Reference< drawing::XShape >&    xShape,
            const uno::Reference< drawing::XDrawPage >& xContainingPage,
            double                                      nPrio,
            bool                                        bForeignSource,
            const SlideShowContext&                     rContext )
        {
            DrawShapeSharedPtr pShape( new DrawShape(xShape,
                                                     xContainingPage,
                                                     nPrio,
                                                     bForeignSource,
                                                     rContext) );

            if( pShape->hasIntrinsicAnimation() )
            {
                OSL_ASSERT( pShape->maAnimationFrames.empty() );
                if( pShape->getNumberOfTreeNodes(
                        DocTreeNode::NODETYPE_LOGICAL_PARAGRAPH) > 0 )
                {
                    pShape->mpIntrinsicAnimationActivity =
                        createDrawingLayerAnimActivity(
                            rContext,
                            pShape);
                }
            }

            if( pShape->hasHyperlinks() )
                rContext.mpSubsettableShapeManager->addHyperlinkArea( pShape );

            return pShape;
        }

        DrawShapeSharedPtr DrawShape::create(
            const uno::Reference< drawing::XShape >&    xShape,
            const uno::Reference< drawing::XDrawPage >& xContainingPage,
            double                                      nPrio,
            const Graphic&                              rGraphic,
            const SlideShowContext&                     rContext )
        {
            DrawShapeSharedPtr pShape( new DrawShape(xShape,
                                                     xContainingPage,
                                                     nPrio,
                                                     rGraphic,
                                                     rContext) );

            if( pShape->hasIntrinsicAnimation() )
            {
                OSL_ASSERT( !pShape->maAnimationFrames.empty() );

                std::vector<double> aTimeout;
                std::transform(
                    pShape->maAnimationFrames.begin(),
                    pShape->maAnimationFrames.end(),
                    std::back_insert_iterator< std::vector<double> >( aTimeout ),
                    boost::mem_fn(&MtfAnimationFrame::getDuration) );

                WakeupEventSharedPtr pWakeupEvent(
                    new WakeupEvent( rContext.mrEventQueue.getTimer(),
                                     rContext.mrActivitiesQueue ) );

                ActivitySharedPtr pActivity =
                    createIntrinsicAnimationActivity(
                        rContext,
                        pShape,
                        pWakeupEvent,
                        aTimeout,
                        pShape->mnAnimationLoopCount,
                        pShape->meCycleMode);

                pWakeupEvent->setActivity( pActivity );
                pShape->mpIntrinsicAnimationActivity = pActivity;
            }

            OSL_ENSURE( !pShape->hasHyperlinks(),
                        "DrawShape::create(): graphic-only shapes must not have hyperlinks!" );

            return pShape;
        }

        DrawShape::~DrawShape()
        {
            try
            {
                // dispose intrinsic animation activity, else, it will
                // linger forever
                ActivitySharedPtr pActivity( mpIntrinsicAnimationActivity.lock() );
                if( pActivity )
                    pActivity->dispose();
            }
            catch (uno::Exception &)
            {
                OSL_FAIL( rtl::OUStringToOString(
                                comphelper::anyToString(
                                    cppu::getCaughtException() ),
                                RTL_TEXTENCODING_UTF8 ).getStr() );
            }
        }

        uno::Reference< drawing::XShape > DrawShape::getXShape() const
        {
            return mxShape;
        }

        void DrawShape::addViewLayer( const ViewLayerSharedPtr& rNewLayer,
                                      bool                      bRedrawLayer )
        {
            ViewShapeVector::iterator aEnd( maViewShapes.end() );

            // already added?
            if( ::std::find_if( maViewShapes.begin(),
                                aEnd,
                                ::boost::bind<bool>(
                                    ::std::equal_to< ViewLayerSharedPtr >(),
                                    ::boost::bind( &ViewShape::getViewLayer,
                                                   _1 ),
                                    ::boost::cref( rNewLayer ) ) ) != aEnd )
            {
                // yes, nothing to do
                return;
            }

            ViewShapeSharedPtr pNewShape( new ViewShape( rNewLayer ) );

            maViewShapes.push_back( pNewShape );

            // pass on animation state
            if( mnIsAnimatedCount )
            {
                for( int i=0; i<mnIsAnimatedCount; ++i )
                    pNewShape->enterAnimationMode();
            }

            // render the Shape on the newly added ViewLayer
            if( bRedrawLayer )
            {
                pNewShape->update( mpCurrMtf,
                                   getViewRenderArgs(),
                                   ViewShape::FORCE,
                                   isVisible() );
            }
        }

        bool DrawShape::removeViewLayer( const ViewLayerSharedPtr& rLayer )
        {
            const ViewShapeVector::iterator aEnd( maViewShapes.end() );

            OSL_ENSURE( ::std::count_if(maViewShapes.begin(),
                                        aEnd,
                                        ::boost::bind<bool>(
                                            ::std::equal_to< ViewLayerSharedPtr >(),
                                            ::boost::bind( &ViewShape::getViewLayer,
                                                           _1 ),
                                            ::boost::cref( rLayer ) ) ) < 2,
                        "DrawShape::removeViewLayer(): Duplicate ViewLayer entries!" );

            ViewShapeVector::iterator aIter;

            if( (aIter=::std::remove_if( maViewShapes.begin(),
                                         aEnd,
                                         ::boost::bind<bool>(
                                             ::std::equal_to< ViewLayerSharedPtr >(),
                                             ::boost::bind( &ViewShape::getViewLayer,
                                                            _1 ),
                                             ::boost::cref( rLayer ) ) )) == aEnd )
            {
                // view layer seemingly was not added, failed
                return false;
            }

            // actually erase from container
            maViewShapes.erase( aIter, aEnd );

            return true;
        }

        bool DrawShape::clearAllViewLayers()
        {
            maViewShapes.clear();
            return true;
        }

        bool DrawShape::update() const
        {
            if( mbForceUpdate )
            {
                return render();
            }
            else
            {
                return implRender( getUpdateFlags() );
            }
        }

        bool DrawShape::render() const
        {
            // force redraw. Have to also pass on the update flags,
            // because e.g. content update (regeneration of the
            // metafile renderer) is normally not performed. A simple
            // ViewShape::FORCE would only paint the metafile in its
            // old state.
            return implRender( ViewShape::FORCE | getUpdateFlags() );
        }

        bool DrawShape::isContentChanged() const
        {
            return mbForceUpdate ?
                true :
                getUpdateFlags() != ViewShape::NONE;
        }


        ::basegfx::B2DRectangle DrawShape::getBounds() const
        {
            // little optimization: for non-modified shapes, we don't
            // create an ShapeAttributeStack, and therefore also don't
            // have to check it.
            return getShapePosSize( maBounds,
                                    mpAttributeLayer );
        }

        ::basegfx::B2DRectangle DrawShape::getDomBounds() const
        {
            return maBounds;
        }

        namespace
        {
            /** Functor expanding AA border for each passed ViewShape

                Could not use ::boost::bind here, since
                B2DRange::expand is overloaded (which yields one or
                the other template type deduction ambiguous)
             */
            class Expander
            {
            public:
                Expander( ::basegfx::B2DSize& rBounds ) :
                    mrBounds( rBounds )
                {
                }

                void operator()( const ViewShapeSharedPtr& rShape ) const
                {
                    const ::basegfx::B2DSize& rShapeBorder( rShape->getAntialiasingBorder() );

                    mrBounds.setX(
                        ::std::max(
                            rShapeBorder.getX(),
                            mrBounds.getX() ) );
                    mrBounds.setY(
                        ::std::max(
                            rShapeBorder.getY(),
                            mrBounds.getY() ) );
                }

            private:
                ::basegfx::B2DSize& mrBounds;
            };
        }

        ::basegfx::B2DRectangle DrawShape::getUpdateArea() const
        {
            ::basegfx::B2DRectangle aBounds;

            // an already empty shape bound need no further
            // treatment. In fact, any changes applied below would
            // actually remove the special empty state, thus, don't
            // change!
            if( !maBounds.isEmpty() )
            {
                basegfx::B2DRectangle aUnitBounds(0.0,0.0,1.0,1.0);

                if( !maViewShapes.empty() )
                    aUnitBounds = getActualUnitShapeBounds();

                if( !aUnitBounds.isEmpty() )
                {
                    if( mpAttributeLayer )
                    {
                        // calc actual shape area (in user coordinate
                        // space) from the transformation as given by the
                        // shape attribute layer
                        aBounds = getShapeUpdateArea( aUnitBounds,
                                                      getShapeTransformation( getBounds(),
                                                                              mpAttributeLayer ),
                                                      mpAttributeLayer );
                    }
                    else
                    {
                        // no attribute layer, thus, the true shape bounds
                        // can be directly derived from the XShape bound
                        // attribute
                        aBounds = getShapeUpdateArea( aUnitBounds,
                                                      maBounds );
                    }

                    if( !maViewShapes.empty() )
                    {
                        // determine border needed for antialiasing the shape
                        ::basegfx::B2DSize aAABorder(0.0,0.0);

                        // for every view, get AA border and 'expand' aAABorder
                        // appropriately.
                        ::std::for_each( maViewShapes.begin(),
                                         maViewShapes.end(),
                                         Expander( aAABorder ) );

                        // add calculated AA border to aBounds
                        aBounds = ::basegfx::B2DRectangle( aBounds.getMinX() - aAABorder.getX(),
                                                           aBounds.getMinY() - aAABorder.getY(),
                                                           aBounds.getMaxX() + aAABorder.getX(),
                                                           aBounds.getMaxY() + aAABorder.getY() );
                    }
                }
            }

            return aBounds;
        }

        bool DrawShape::isVisible() const
        {
            bool bIsVisible( mbIsVisible );

            if( mpAttributeLayer )
            {
                // check whether visibility and alpha are not default
                // (mpAttributeLayer->isVisibilityValid() returns true
                // then): bVisible becomes true, if shape visibility
                // is on and alpha is not 0.0 (fully transparent)
                if( mpAttributeLayer->isVisibilityValid() )
                    bIsVisible = mpAttributeLayer->getVisibility();

                // only touch bIsVisible, if the shape is still
                // visible - if getVisibility already made us
                // invisible, no alpha value will make us appear
                // again.
                if( bIsVisible && mpAttributeLayer->isAlphaValid() )
                    bIsVisible = !::basegfx::fTools::equalZero( mpAttributeLayer->getAlpha() );
            }

            return bIsVisible;
        }

        double DrawShape::getPriority() const
        {
            return mnPriority;
        }

        bool DrawShape::isBackgroundDetached() const
        {
            return mnIsAnimatedCount > 0;
        }

        bool DrawShape::hasIntrinsicAnimation() const
        {
            return (!maAnimationFrames.empty() || mbDrawingLayerAnim);
        }

        bool DrawShape::setIntrinsicAnimationFrame( ::std::size_t nCurrFrame )
        {
            ENSURE_OR_RETURN_FALSE( nCurrFrame < maAnimationFrames.size(),
                               "DrawShape::setIntrinsicAnimationFrame(): frame index out of bounds" );

            if( mnCurrFrame != nCurrFrame )
            {
                mnCurrFrame   = nCurrFrame;
                mpCurrMtf     = maAnimationFrames[ mnCurrFrame ].mpMtf;
                mbForceUpdate = true;
            }

            return true;
        }

        // hyperlink support
        void DrawShape::prepareHyperlinkIndices() const
        {
            if ( !maHyperlinkIndices.empty())
            {
                maHyperlinkIndices.clear();
                maHyperlinkRegions.clear();
            }

            sal_Int32 nIndex = 0;
            for ( MetaAction * pCurrAct = mpCurrMtf->FirstAction();
                  pCurrAct != 0; pCurrAct = mpCurrMtf->NextAction() )
            {
                if (pCurrAct->GetType() == META_COMMENT_ACTION) {
                    MetaCommentAction * pAct =
                        static_cast<MetaCommentAction *>(pCurrAct);
                    // skip comment if not a special XTEXT comment
                    if (pAct->GetComment().CompareIgnoreCaseToAscii(
                            RTL_CONSTASCII_STRINGPARAM("FIELD_SEQ_BEGIN") ) ==
                        COMPARE_EQUAL &&
                        // e.g. date field doesn't have data!
                        // currently assuming that only url field, this is
                        // somehow fragile! xxx todo if possible
                        pAct->GetData() != 0 &&
                        pAct->GetDataSize() > 0)
                    {
                        if (!maHyperlinkIndices.empty() &&
                            maHyperlinkIndices.back().second == -1) {
                            OSL_FAIL( "### pending FIELD_SEQ_END!" );
                            maHyperlinkIndices.pop_back();
                            maHyperlinkRegions.pop_back();
                        }
                        maHyperlinkIndices.push_back(
                            HyperlinkIndexPair( nIndex + 1,
                                                -1 /* to be filled below */ ) );
                        maHyperlinkRegions.push_back(
                            HyperlinkRegion(
                                basegfx::B2DRectangle(),
                                rtl::OUString(
                                    reinterpret_cast<sal_Unicode const*>(
                                        pAct->GetData()),
                                    pAct->GetDataSize() / sizeof(sal_Unicode) )
                                ) );
                    }
                    else if (pAct->GetComment().CompareIgnoreCaseToAscii(
                                 RTL_CONSTASCII_STRINGPARAM("FIELD_SEQ_END")) ==
                             COMPARE_EQUAL &&
                             // pending end is expected:
                             !maHyperlinkIndices.empty() &&
                             maHyperlinkIndices.back().second == -1)
                    {
                        maHyperlinkIndices.back().second = nIndex;
                    }
                    ++nIndex;
                }
                else
                    nIndex += getNextActionOffset(pCurrAct);
            }
            if (!maHyperlinkIndices.empty() &&
                maHyperlinkIndices.back().second == -1) {
                OSL_FAIL( "### pending FIELD_SEQ_END!" );
                maHyperlinkIndices.pop_back();
                maHyperlinkRegions.pop_back();
            }
            OSL_ASSERT( maHyperlinkIndices.size() == maHyperlinkRegions.size());
        }

        bool DrawShape::hasHyperlinks() const
        {
            return ! maHyperlinkRegions.empty();
        }

        HyperlinkArea::HyperlinkRegions DrawShape::getHyperlinkRegions() const
        {
            OSL_ASSERT( !maViewShapes.empty() );

            if( !isVisible() )
                return HyperlinkArea::HyperlinkRegions();

            // late init, determine regions:
            if( !maHyperlinkRegions.empty() &&
                !maViewShapes.empty() &&
                // region already inited?
                maHyperlinkRegions.front().first.getWidth() == 0 &&
                maHyperlinkRegions.front().first.getHeight() == 0 &&
                maHyperlinkRegions.size() == maHyperlinkIndices.size() )
            {
                // TODO(Q2): Although this _is_ currently
                // view-agnostic, it might not stay like that.
                ViewShapeSharedPtr const& pViewShape = maViewShapes.front();
                cppcanvas::CanvasSharedPtr const pCanvas(
                    pViewShape->getViewLayer()->getCanvas() );

                // reuse Renderer of first view shape:
                cppcanvas::RendererSharedPtr const pRenderer(
                    pViewShape->getRenderer(
                        pCanvas, mpCurrMtf, mpAttributeLayer ) );

                OSL_ASSERT( pRenderer );

                if (pRenderer)
                {
                    basegfx::B2DHomMatrix const aOldTransform(
                        pCanvas->getTransformation() );
                    basegfx::B2DHomMatrix aTransform;
                    pCanvas->setTransformation( aTransform /* empty */ );

                    comphelper::ScopeGuard const resetOldTransformation(
                        boost::bind( &cppcanvas::Canvas::setTransformation,
                                     pCanvas.get(),
                                     boost::cref(aOldTransform) ));

                    aTransform.scale( maBounds.getWidth(),
                                      maBounds.getHeight() );
                    pRenderer->setTransformation( aTransform );
                    pRenderer->setClip();

                    for( std::size_t pos = maHyperlinkRegions.size(); pos--; )
                    {
                        // get region:
                        HyperlinkIndexPair const& rIndices = maHyperlinkIndices[pos];
                        basegfx::B2DRectangle const region(
                            pRenderer->getSubsetArea( rIndices.first,
                                                      rIndices.second ));
                        maHyperlinkRegions[pos].first = region;
                    }
                }
            }

            // shift shape-relative hyperlink regions to
            // slide-absolute position

            HyperlinkRegions aTranslatedRegions;
            const basegfx::B2DPoint& rOffset(getBounds().getMinimum());
            HyperlinkRegions::const_iterator       aIter( maHyperlinkRegions.begin() );
            HyperlinkRegions::const_iterator const aEnd ( maHyperlinkRegions.end() );
            while( aIter != aEnd )
            {
                basegfx::B2DRange const& relRegion( aIter->first );
                aTranslatedRegions.push_back(
                    std::make_pair(
                        basegfx::B2DRange(
                            relRegion.getMinimum() + rOffset,
                            relRegion.getMaximum() + rOffset),
                        aIter->second) );
                ++aIter;
            }

            return aTranslatedRegions;
        }

        double DrawShape::getHyperlinkPriority() const
        {
            return getPriority();
        }


        // AnimatableShape methods
        // ======================================================

        void DrawShape::enterAnimationMode()
        {
            OSL_ENSURE( !maViewShapes.empty(),
                        "DrawShape::enterAnimationMode(): called on DrawShape without views" );

            if( mnIsAnimatedCount == 0 )
            {
                // notify all ViewShapes, by calling their enterAnimationMode method.
                // We're now entering animation mode
                ::std::for_each( maViewShapes.begin(),
                                 maViewShapes.end(),
                                 ::boost::mem_fn( &ViewShape::enterAnimationMode ) );
            }

            ++mnIsAnimatedCount;
        }

        void DrawShape::leaveAnimationMode()
        {
            OSL_ENSURE( !maViewShapes.empty(),
                        "DrawShape::leaveAnimationMode(): called on DrawShape without views" );

            --mnIsAnimatedCount;

            if( mnIsAnimatedCount == 0 )
            {
                // notify all ViewShapes, by calling their leaveAnimationMode method.
                // we're now leaving animation mode
                ::std::for_each( maViewShapes.begin(),
                                 maViewShapes.end(),
                                 ::boost::mem_fn( &ViewShape::leaveAnimationMode ) );
            }
        }


        // AttributableShape methods
        // ======================================================

        ShapeAttributeLayerSharedPtr DrawShape::createAttributeLayer()
        {
            // create new layer, with last as its new child
            mpAttributeLayer.reset( new ShapeAttributeLayer( mpAttributeLayer ) );

            // Update the local state ids to reflect those of the new layer.
            updateStateIds();

            return mpAttributeLayer;
        }

        bool DrawShape::revokeAttributeLayer( const ShapeAttributeLayerSharedPtr& rLayer )
        {
            if( !mpAttributeLayer )
                return false; // no layers

            if( mpAttributeLayer == rLayer )
            {
                // it's the toplevel layer
                mpAttributeLayer = mpAttributeLayer->getChildLayer();

                // force content redraw, all state variables have
                // possibly changed
                mbAttributeLayerRevoked = true;

                return true;
            }
            else
            {
                // pass on to the layer, to try its children
                return mpAttributeLayer->revokeChildLayer( rLayer );
            }
        }

        ShapeAttributeLayerSharedPtr DrawShape::getTopmostAttributeLayer() const
        {
            return mpAttributeLayer;
        }

        void DrawShape::setVisibility( bool bVisible )
        {
            if( mbIsVisible != bVisible )
            {
                mbIsVisible = bVisible;
                mbForceUpdate = true;
            }
        }

        const DocTreeNodeSupplier& DrawShape::getTreeNodeSupplier() const
        {
            return *this;
        }

        DocTreeNodeSupplier& DrawShape::getTreeNodeSupplier()
        {
            return *this;
        }

        DocTreeNode DrawShape::getSubsetNode() const
        {
            ensureVerboseMtfComments();

            // forward to delegate
            return maSubsetting.getSubsetNode();
        }

        AttributableShapeSharedPtr DrawShape::getSubset( const DocTreeNode& rTreeNode ) const
        {
            ENSURE_OR_THROW( (mnCurrMtfLoadFlags & MTF_LOAD_VERBOSE_COMMENTS) != 0,
                              "DrawShape::getSubset(): subset query on shape with apparently no subsets" );

            // forward to delegate
            return maSubsetting.getSubsetShape( rTreeNode );
        }

        bool DrawShape::createSubset( AttributableShapeSharedPtr&   o_rSubset,
                                      const DocTreeNode&            rTreeNode )
        {
            ENSURE_OR_THROW( (mnCurrMtfLoadFlags & MTF_LOAD_VERBOSE_COMMENTS) != 0,
                              "DrawShape::createSubset(): subset query on shape with apparently no subsets" );

            // subset shape already created for this DocTreeNode?
            AttributableShapeSharedPtr pSubset( maSubsetting.getSubsetShape( rTreeNode ) );

            // when true, this method has created a new subset
            // DrawShape
            bool bNewlyCreated( false );

            if( pSubset )
            {
                o_rSubset = pSubset;

                // reusing existing subset
            }
            else
            {
                // not yet created, init entry
                o_rSubset.reset( new DrawShape( *this,
                                                rTreeNode,
                                                // TODO(Q3): That's a
                                                // hack. We assume
                                                // that start and end
                                                // index will always
                                                // be less than 65535
                                                mnPriority +
                                                rTreeNode.getStartIndex()/double(SAL_MAX_INT16) ));

                bNewlyCreated = true; // subset newly created
            }

            // always register shape at DrawShapeSubsetting, to keep
            // refcount up-to-date
            maSubsetting.addSubsetShape( o_rSubset );

            // flush bounds cache
            maCurrentShapeUnitBounds.reset();

            return bNewlyCreated;
        }

        bool DrawShape::revokeSubset( const AttributableShapeSharedPtr& rShape )
        {
            ENSURE_OR_THROW( (mnCurrMtfLoadFlags & MTF_LOAD_VERBOSE_COMMENTS) != 0,
                              "DrawShape::createSubset(): subset query on shape with apparently no subsets" );

            // flush bounds cache
            maCurrentShapeUnitBounds.reset();

            // forward to delegate
            if( maSubsetting.revokeSubsetShape( rShape ) )
            {
                // force redraw, our content has possibly changed (as
                // one of the subsets now display within our shape
                // again).
                mbForceUpdate = true;

                // #i47428# TEMP FIX: synchronize visibility of subset
                // with parent.

                // TODO(F3): Remove here, and implement
                // TEXT_ONLY/BACKGROUND_ONLY with the proverbial
                // additional level of indirection: create a
                // persistent subset, containing all text/only the
                // background respectively. From _that_ object,
                // generate the temporary character subset shapes.
                const ShapeAttributeLayerSharedPtr& rAttrLayer(
                    rShape->getTopmostAttributeLayer() );
                if( rAttrLayer &&
                    rAttrLayer->isVisibilityValid() &&
                    rAttrLayer->getVisibility() != isVisible() )
                {
                    const bool bVisibility( rAttrLayer->getVisibility() );

                    // visibilities differ - adjust ours, then
                    if( mpAttributeLayer )
                        mpAttributeLayer->setVisibility( bVisibility );
                    else
                        mbIsVisible = bVisibility;
                }

                // END TEMP FIX

                return true;
            }

            return false;
        }

        sal_Int32 DrawShape::getNumberOfTreeNodes( DocTreeNode::NodeType eNodeType ) const // throw ShapeLoadFailedException
        {
            ensureVerboseMtfComments();

            return maSubsetting.getNumberOfTreeNodes( eNodeType );
        }

        DocTreeNode DrawShape::getTreeNode( sal_Int32               nNodeIndex,
                                            DocTreeNode::NodeType   eNodeType ) const // throw ShapeLoadFailedException
        {
            ensureVerboseMtfComments();

            if ( hasHyperlinks())
            {
                prepareHyperlinkIndices();
            }

            return maSubsetting.getTreeNode( nNodeIndex, eNodeType );
        }

        sal_Int32 DrawShape::getNumberOfSubsetTreeNodes ( const DocTreeNode&    rParentNode,
                                                          DocTreeNode::NodeType eNodeType ) const // throw ShapeLoadFailedException
        {
            ensureVerboseMtfComments();

            return maSubsetting.getNumberOfSubsetTreeNodes( rParentNode, eNodeType );
        }

        DocTreeNode DrawShape::getSubsetTreeNode( const DocTreeNode&    rParentNode,
                                                  sal_Int32             nNodeIndex,
                                                  DocTreeNode::NodeType eNodeType ) const // throw ShapeLoadFailedException
        {
            ensureVerboseMtfComments();

            return maSubsetting.getSubsetTreeNode( rParentNode, nNodeIndex, eNodeType );
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
