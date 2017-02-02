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

#include <tools/diagnose_ex.h>
#include <canvas/canvastools.hxx>

#include "eventqueue.hxx"
#include "eventmultiplexer.hxx"
#include "slideview.hxx"
#include "delayevent.hxx"
#include "unoview.hxx"

#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/compbase.hxx>
#include <cppuhelper/implementationentry.hxx>
#include <cppuhelper/interfacecontainer.h>
#include <comphelper/make_shared_from_uno.hxx>

#include <cppcanvas/spritecanvas.hxx>
#include <cppcanvas/customsprite.hxx>
#include <cppcanvas/vclfactory.hxx>
#include <cppcanvas/basegfxfactory.hxx>

#include <basegfx/range/b1drange.hxx>
#include <basegfx/range/b2drange.hxx>
#include <basegfx/range/b2irange.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/tools/canvastools.hxx>
#include <basegfx/polygon/b2dpolygonclipper.hxx>
#include <basegfx/polygon/b2dpolypolygoncutter.hxx>

#include <com/sun/star/presentation/XSlideShow.hpp>
#include <com/sun/star/rendering/CompositeOperation.hpp>

#include <memory>
#include <vector>
#include <iterator>
#include <algorithm>

using namespace com::sun::star;

namespace slideshow {
namespace internal {

namespace {

/** Sprite entry, to store sprite plus priority

    The operator<() defines a strict weak ordering of sprites, sort
    key is the sprite priority.
 */
struct SpriteEntry
{
    SpriteEntry( const cppcanvas::CustomSpriteSharedPtr& rSprite,
                 double                                  nPrio ) :
        mpSprite( rSprite ),
        mnPriority( nPrio )
    {
    }

    bool operator<(const SpriteEntry& rRHS) const
    {
        return mnPriority < rRHS.mnPriority;
    }

    std::weak_ptr< cppcanvas::CustomSprite > mpSprite;
    double                                     mnPriority;
};

typedef std::vector< SpriteEntry > SpriteVector;


/** Create a clip polygon for slide views

    @param rClip
    Clip to set (can be empty)

    @param rCanvas
    Canvas to create the clip polygon for

    @param rUserSize
    The size of the view. Note that the returned clip will
    <em>always</em> clip to at least the rect defined herein.

    @return the view clip polygon, in view coordinates, which is
    guaranteed to at least clip to the view size.
 */
basegfx::B2DPolyPolygon createClipPolygon( const basegfx::B2DPolyPolygon&    rClip,
                                           const cppcanvas::CanvasSharedPtr& /*rCanvas*/,
                                           const basegfx::B2DSize&           rUserSize )
{
    // setup canvas clipping
    // =====================

    // AW: Simplified
    const basegfx::B2DRange aClipRange(0, 0, rUserSize.getX(), rUserSize.getY());

    if(rClip.count())
    {
        return basegfx::tools::clipPolyPolygonOnRange(rClip, aClipRange, true, false);
    }
    else
    {
        return basegfx::B2DPolyPolygon(basegfx::tools::createPolygonFromRect(aClipRange));
    }
}

/** Prepare given clip polygon to be stored as the current clip

    Note that this is separate from createClipPolygon(), to allow
    SlideView implementations to store this intermediate result
    (createClipPolygon() has to be called every time the view size
    changes)
 */
basegfx::B2DPolyPolygon prepareClip( const basegfx::B2DPolyPolygon& rClip )
{
    basegfx::B2DPolyPolygon aClip( rClip );

    // TODO(P2): unnecessary, once XCanvas is correctly handling this
    // AW: Should be no longer necessary; tools are now bezier-safe
    if( aClip.areControlPointsUsed() )
        aClip = basegfx::tools::adaptiveSubdivideByAngle( aClip );

    // normalize polygon, preparation for clipping
    // in updateCanvas()
    aClip = basegfx::tools::correctOrientations(aClip);
    aClip = basegfx::tools::solveCrossovers(aClip);
    aClip = basegfx::tools::stripNeutralPolygons(aClip);
    aClip = basegfx::tools::stripDispensablePolygons(aClip);

    return aClip;
}


void clearRect( ::cppcanvas::CanvasSharedPtr const& pCanvas,
                basegfx::B2IRange const&            rArea )
{
    // convert clip polygon to device coordinate system
    ::basegfx::B2DPolyPolygon const* pClipPoly( pCanvas->getClip() );
    if( pClipPoly )
    {
        ::basegfx::B2DPolyPolygon aClipPoly( *pClipPoly );
        aClipPoly.transform( pCanvas->getTransformation() );
        pCanvas->setClip( aClipPoly );
    }

    // set transformation to identitiy (->device pixel)
    pCanvas->setTransformation( ::basegfx::B2DHomMatrix() );

    // #i42440# Fill the _full_ background in
    // black. Since we had to extend the bitmap by one
    // pixel, and the bitmap is initialized white,
    // depending on the slide content a one pixel wide
    // line will show to the bottom and the right.
    const ::basegfx::B2DPolygon aPoly(
        ::basegfx::tools::createPolygonFromRect(
            basegfx::B2DRange(rArea)));

    ::cppcanvas::PolyPolygonSharedPtr pPolyPoly(
        ::cppcanvas::BaseGfxFactory::createPolyPolygon( pCanvas, aPoly ) );

    if( pPolyPoly )
    {
        pPolyPoly->setCompositeOp( css::rendering::CompositeOperation::SOURCE );
        pPolyPoly->setRGBAFillColor( 0xFFFFFF00U );
        pPolyPoly->draw();
    }

#if defined(DBG_UTIL)
    ::cppcanvas::CanvasSharedPtr pCliplessCanvas( pCanvas->clone() );
    pCliplessCanvas->setClip();

    if( pCanvas->getClip() )
    {
        ::cppcanvas::PolyPolygonSharedPtr pPolyPoly2(
            ::cppcanvas::BaseGfxFactory::createPolyPolygon( pCliplessCanvas, aPoly ));
        if( pPolyPoly2 )
        {
            pPolyPoly2->setRGBALineColor( 0x008000FFU );
            pPolyPoly2->draw();
        }
    }
#endif
}

/** Get bounds in pixel

    @param rLayerBounds
    Bound rect, in user space coordinates

    @param rTransformation
    User space to device pixel transformation

    @return the layer bounds in pixel, extended by one pixel to the
    right and bottom
 */
basegfx::B2IRange getLayerBoundsPixel( basegfx::B2DRange const&     rLayerBounds,
                                       basegfx::B2DHomMatrix const& rTransformation )
{
    ::basegfx::B2DRange aTmpRect;
    ::canvas::tools::calcTransformedRectBounds( aTmpRect,
                                                rLayerBounds,
                                                rTransformation );

    if( aTmpRect.isEmpty() )
        return ::basegfx::B2IRange();

    // #i42440# Returned layer size is one pixel too small, as
    // rendering happens one pixel to the right and below the
    // actual bound rect.
    return ::basegfx::B2IRange( ::basegfx::fround(aTmpRect.getMinX()),
                                ::basegfx::fround(aTmpRect.getMinY()),
                                ::basegfx::fround(aTmpRect.getMaxX()) + 1,
                                ::basegfx::fround(aTmpRect.getMaxY()) + 1 );
}


/** Container class for sprites issued by a ViewLayer

    This class handles the sprite prioritization issues, that are
    needed for layer sprites (e.g. the need to re-prioritize sprites
    when the layer changes prio).
 */
class LayerSpriteContainer
{
    /** Max fill level of maSprites, before we try to prune it from
        deceased sprites
    */
    enum{ SPRITE_ULLAGE=256 };

    /** All sprites that have been issued by this container (pruned
        from time to time, for invalid references). This vector is
        kept sorted with increasing sprite priority.
    */
    SpriteVector       maSprites;

    /// Priority of this layer, relative to other view layers
    basegfx::B1DRange  maLayerPrioRange;

    double getSpritePriority( std::size_t nSpriteNum ) const
    {
        // divide the available layer range equally between all
        // sprites, assign upper bound of individual sprite range as
        // sprite prio (the layer itself gets assigned the lower bound
        // of sprite 0's individual range):

        // | layer 0                    | layer 1                    | ...
        // |    sprite 0 |    sprite 1  |    sprite 0 |    sprite 1  | ...
        return maLayerPrioRange.getMinimum() + maLayerPrioRange.getRange()*(nSpriteNum+1)/(maSprites.size()+1);
    }

    /** Rescan sprite vector, and remove deceased sprites (and reset
        sprite prio)

        @param aBegin
        Iterator to the first entry to rescan
     */
    void updateSprites()
    {
        SpriteVector aValidSprites;

        // check all sprites for validity and set new priority
        for( const auto& rSprite : maSprites )
        {
            cppcanvas::CustomSpriteSharedPtr pCurrSprite( rSprite.mpSprite.lock() );

            if( pCurrSprite )
            {
                // only copy still valid sprites over to the refreshed
                // sprite vector.
                aValidSprites.push_back( rSprite );

                pCurrSprite->setPriority(
                    getSpritePriority( aValidSprites.size()-1 ));
            }
        }

        // replace sprite list with pruned one
        maSprites.swap( aValidSprites );
    }

public:
    LayerSpriteContainer() :
        maSprites(),
        maLayerPrioRange()
    {
    }

    const basegfx::B1DRange& getLayerPriority() const
    {
        return maLayerPrioRange;
    }

    void setLayerPriority( const basegfx::B1DRange& rRange )
    {
        if( rRange != maLayerPrioRange )
        {
            maLayerPrioRange = rRange;

            // prune and recalc sprite prios
            updateSprites();
        }
    }

    void addSprite( const cppcanvas::CustomSpriteSharedPtr& pSprite,
                    double                                  nPriority )
    {
        if( !pSprite )
            return;

        SpriteEntry aEntry( pSprite,nPriority );

        // insert new sprite, such that vector stays sorted
        SpriteVector::iterator aInsertPos(
            maSprites.insert(
                std::lower_bound( maSprites.begin(),
                                  maSprites.end(),
                                  aEntry ),
                aEntry ));

        const std::size_t nNumSprites( maSprites.size() );
        if( nNumSprites > SPRITE_ULLAGE ||
            maSprites.end() - aInsertPos > 1 )
        {
            // updateSprites() also updates all sprite prios
            updateSprites();
        }
        else
        {
            // added sprite to the end, and not too many sprites in
            // vector - perform optimized update (only need to set
            // prio). This basically caters for the common case of
            // iterated character animations, which generate lots of
            // sprites, all added to the end.
            pSprite->setPriority(
                getSpritePriority( nNumSprites-1 ));
        }
    }

    void clear()
    {
        maSprites.clear();
    }
};


/** This class provides layers for a slide view

    Layers are used to render animations with the correct z order -
    because sprites are always in front of the static canvas
    background, shapes that must appear <em<before</em> an animation
    must also be displayed as a sprite.

    Each layer has a priority assigned to it (valid range [0,1]), which
    also affects all sprites created for this specific layer - i.e. if
    the layer priority changes, the sprites change z order together
    with their parent.
 */
class SlideViewLayer : public ViewLayer
{
    /// Smart container for all sprites issued by this layer
    mutable LayerSpriteContainer             maSpriteContainer;

    /// Bounds of this layer in user space coordinates
    basegfx::B2DRange                        maLayerBounds;

    /// Bounds of this layer in device pixel
    mutable basegfx::B2IRange                maLayerBoundsPixel;

    /// Current clip polygon in user coordinates
    basegfx::B2DPolyPolygon                  maClip;

    /// Current size of the view in user coordinates
    basegfx::B2DSize                         maUserSize;

    /// Current overall view transformation
    basegfx::B2DHomMatrix                    maTransformation;

    /// 'parent' canvas, this viewlayer is associated with
    const cppcanvas::SpriteCanvasSharedPtr   mpSpriteCanvas;

    /** output surface (necessarily a sprite, won't otherwise be able
        to display anything <em>before</em> other sprites)
    */
    mutable cppcanvas::CustomSpriteSharedPtr mpSprite;

    /// actual output canvas retrieved from a sprite
    mutable cppcanvas::CanvasSharedPtr       mpOutputCanvas;

    /// ptr back to owning view. needed for isOnView() method
    View const* const                        mpParentView;

public:
    /** Create a new layer

        @param pCanvas
        Sprite canvas to create the layer on

        @param rTransform
        Initial overall canvas transformation

        @param rLayerBounds
        Initial layer bounds, in view coordinate system
     */
    SlideViewLayer( const cppcanvas::SpriteCanvasSharedPtr& pCanvas,
                    const basegfx::B2DHomMatrix&            rTransform,
                    const basegfx::B2DRange&                rLayerBounds,
                    const basegfx::B2DSize&                 rUserSize,
                    View const* const                       pParentView) :
        maSpriteContainer(),
        maLayerBounds(rLayerBounds),
        maLayerBoundsPixel(),
        maClip(),
        maUserSize(rUserSize),
        maTransformation(rTransform),
        mpSpriteCanvas(pCanvas),
        mpSprite(),
        mpOutputCanvas(),
        mpParentView(pParentView)
    {
    }

    SlideViewLayer(const SlideViewLayer&) = delete;
    SlideViewLayer& operator=(const SlideViewLayer&) = delete;

    void updateView( const basegfx::B2DHomMatrix& rMatrix,
                     const basegfx::B2DSize&      rUserSize )
    {
        maTransformation = rMatrix;
        maUserSize = rUserSize;

        // limit layer bounds to visible screen
        maLayerBounds.intersect( basegfx::B2DRange(0.0,
                                                   0.0,
                                                   maUserSize.getX(),
                                                   maUserSize.getY()) );

        basegfx::B2IRange const& rNewLayerPixel(
            getLayerBoundsPixel(maLayerBounds,
                                maTransformation) );
        if( rNewLayerPixel != maLayerBoundsPixel )
        {
            // re-gen sprite with new size
            mpOutputCanvas.reset();
            mpSprite.reset();
        }
    }

    virtual css::geometry::IntegerSize2D getTranslationOffset() const override
    {
        basegfx::B2DRectangle aTmpRect;
        canvas::tools::calcTransformedRectBounds( aTmpRect,
                                                  maLayerBounds,
                                                  maTransformation );
        geometry::IntegerSize2D offset(0, 0);

        // Add translation according to the origin of aTmpRect.  Ignore the
        // translation when aTmpRect was not properly initialized.
        if ( ! aTmpRect.isEmpty())
        {
            offset.Width  = basegfx::fround(aTmpRect.getMinX());
            offset.Height = basegfx::fround(aTmpRect.getMinY());
        }
        return offset;
    }

private:
    // ViewLayer interface


    virtual cppcanvas::CustomSpriteSharedPtr createSprite(
        const ::basegfx::B2DSize& rSpriteSizePixel,
        double                    nPriority ) const override
    {
        cppcanvas::CustomSpriteSharedPtr pSprite(
            mpSpriteCanvas->createCustomSprite( rSpriteSizePixel ) );

        maSpriteContainer.addSprite( pSprite,
                                     nPriority );

        return pSprite;
    }

    virtual void setPriority( const basegfx::B1DRange& rRange ) override
    {
        OSL_ENSURE( !rRange.isEmpty() &&
                    rRange.getMinimum() >= 1.0,
                    "SlideViewLayer::setPriority(): prio MUST be larger than 1.0 (because "
                    "the background layer already lies there)" );

        maSpriteContainer.setLayerPriority( rRange );

        if( mpSprite )
            mpSprite->setPriority( rRange.getMinimum() );
    }

    virtual basegfx::B2DHomMatrix getTransformation() const override
    {
        // Offset given transformation by left, top border of given
        // range (after transformation through given transformation)
        basegfx::B2DRectangle aTmpRect;
        canvas::tools::calcTransformedRectBounds( aTmpRect,
                                                  maLayerBounds,
                                                  maTransformation );

        basegfx::B2DHomMatrix aMatrix( maTransformation );

        // Add translation according to the origin of aTmpRect.  Ignore the
        // translation when aTmpRect was not properly initialized.
        if ( ! aTmpRect.isEmpty())
        {
            aMatrix.translate( -basegfx::fround(aTmpRect.getMinX()),
                               -basegfx::fround(aTmpRect.getMinY()) );
        }

        return aMatrix;
    }

    virtual basegfx::B2DHomMatrix getSpriteTransformation() const override
    {
        return maTransformation;
    }

    virtual void clear() const override
    {
        // grab canvas - that also lazy-initializes maLayerBoundsPixel
        cppcanvas::CanvasSharedPtr pCanvas=getCanvas()->clone();

        // clear whole canvas
        const basegfx::B2I64Tuple& rSpriteSize(maLayerBoundsPixel.getRange());
        clearRect(pCanvas,
                  basegfx::B2IRange(0,0,rSpriteSize.getX(),rSpriteSize.getY()));
    }

    virtual void clearAll() const override
    {
        // grab canvas - that also lazy-initializes maLayerBoundsPixel
        ::cppcanvas::CanvasSharedPtr pCanvas( getCanvas()->clone() );

        // clear layer clip, to clear whole area
        pCanvas->setClip();

        // clear whole canvas
        const basegfx::B2I64Tuple& rSpriteSize(maLayerBoundsPixel.getRange());
        clearRect(pCanvas,
                  basegfx::B2IRange(0,0,rSpriteSize.getX(),rSpriteSize.getY()));
    }

    virtual bool isOnView(std::shared_ptr<View> const& rView) const override
    {
        return rView.get() == mpParentView;
    }

    virtual cppcanvas::CanvasSharedPtr getCanvas() const override
    {
        if( !mpOutputCanvas )
        {
            if( !mpSprite )
            {
                maLayerBoundsPixel = getLayerBoundsPixel(maLayerBounds,
                                                         maTransformation);

                // HACK: ensure at least 1x1 pixel size. clients might
                // need an actual canvas (e.g. for bound rect
                // calculations) without rendering anything. Better
                // solution: introduce something like a reference
                // canvas for ViewLayers, which is always available.
                if( maLayerBoundsPixel.isEmpty() )
                    maLayerBoundsPixel = basegfx::B2IRange(0,0,1,1);

                const basegfx::B2I64Tuple& rSpriteSize(maLayerBoundsPixel.getRange());
                mpSprite = mpSpriteCanvas->createCustomSprite(
                    basegfx::B2DVector(sal::static_int_cast<sal_Int32>(rSpriteSize.getX()),
                                       sal::static_int_cast<sal_Int32>(rSpriteSize.getY())) );

                mpSprite->setPriority(
                    maSpriteContainer.getLayerPriority().getMinimum() );

#if defined(DBG_UTIL)
                mpSprite->movePixel(
                    basegfx::B2DPoint(maLayerBoundsPixel.getMinimum()) +
                    basegfx::B2DPoint(10,10) );

                mpSprite->setAlpha(0.5);
#else
                mpSprite->movePixel(
                    basegfx::B2DPoint(maLayerBoundsPixel.getMinimum()) );

                mpSprite->setAlpha(1.0);
#endif
                mpSprite->show();
            }

            ENSURE_OR_THROW( mpSprite,
                              "SlideViewLayer::getCanvas(): no layer sprite" );

            mpOutputCanvas = mpSprite->getContentCanvas();

            ENSURE_OR_THROW( mpOutputCanvas,
                              "SlideViewLayer::getCanvas(): sprite doesn't yield a canvas" );

            // new canvas retrieved - setup transformation and clip
            mpOutputCanvas->setTransformation( getTransformation() );
            mpOutputCanvas->setClip(
                createClipPolygon( maClip,
                                   mpOutputCanvas,
                                   maUserSize ));
        }

        return mpOutputCanvas;
    }

    virtual void setClip( const basegfx::B2DPolyPolygon& rClip ) override
    {
        basegfx::B2DPolyPolygon aNewClip = prepareClip( rClip );

        if( aNewClip != maClip )
        {
            maClip = aNewClip;

            if(mpOutputCanvas )
                mpOutputCanvas->setClip(
                    createClipPolygon( maClip,
                                       mpOutputCanvas,
                                       maUserSize ));
        }
    }

    virtual bool resize( const ::basegfx::B2DRange& rArea ) override
    {
        const bool bRet( maLayerBounds != rArea );
        maLayerBounds = rArea;
        updateView( maTransformation,
                    maUserSize );

        return bRet;
    }
};


typedef cppu::WeakComponentImplHelper<
      css::util::XModifyListener,
      css::awt::XPaintListener> SlideViewBase;

/** SlideView class

    This class implements the View interface, encapsulating
    <em>one</em> view a slideshow is displayed on.
 */
class SlideView : private cppu::BaseMutex,
                  public SlideViewBase,
                  public UnoView
{
public:
    SlideView( const uno::Reference<presentation::XSlideShowView>& xView,
               EventQueue&                                         rEventQueue,
               EventMultiplexer&                                   rEventMultiplexer );
    void updateCanvas();

private:
    // View:
    virtual ViewLayerSharedPtr createViewLayer( const basegfx::B2DRange& rLayerBounds ) const override;
    virtual bool updateScreen() const override;
    virtual bool paintScreen() const override;
    virtual void setViewSize( const ::basegfx::B2DSize& ) override;
    virtual void setCursorShape( sal_Int16 nPointerShape ) override;

    // ViewLayer interface
    virtual bool isOnView(std::shared_ptr<View> const& rView) const override;
    virtual void clear() const override;
    virtual void clearAll() const override;
    virtual cppcanvas::CanvasSharedPtr getCanvas() const override;
    virtual cppcanvas::CustomSpriteSharedPtr createSprite( const ::basegfx::B2DSize& rSpriteSizePixel,
                                                           double                    nPriority ) const override;
    virtual void setPriority( const basegfx::B1DRange& rRange ) override;
    virtual geometry::IntegerSize2D getTranslationOffset() const override;
    virtual ::basegfx::B2DHomMatrix getTransformation() const override;
    virtual basegfx::B2DHomMatrix getSpriteTransformation() const override;
    virtual void setClip( const ::basegfx::B2DPolyPolygon& rClip ) override;
    virtual bool resize( const ::basegfx::B2DRange& rArea ) override;

    // UnoView:
    virtual void _dispose() override;
    virtual uno::Reference<presentation::XSlideShowView> getUnoView()const override;
    virtual void setIsSoundEnabled (const bool bValue) override;
    virtual bool isSoundEnabled() const override;

    // XEventListener:
    virtual void SAL_CALL disposing( lang::EventObject const& evt ) override;
    // XModifyListener:
    virtual void SAL_CALL modified( const lang::EventObject& aEvent ) override;
    // XPaintListener:
    virtual void SAL_CALL windowPaint( const awt::PaintEvent& e ) override;

    // WeakComponentImplHelperBase:
    virtual void SAL_CALL disposing() override;

    void updateClip();

private:
    typedef std::vector< std::weak_ptr<SlideViewLayer> > ViewLayerVector;

    /// Prune viewlayers from deceased ones, optionally update them
    void pruneLayers( bool bWithViewLayerUpdate=false ) const;

    /** Max fill level of maViewLayers, before we try to prune it from
        deceased layers
    */
    enum{ LAYER_ULLAGE=8 };

    uno::Reference<presentation::XSlideShowView>              mxView;
    cppcanvas::SpriteCanvasSharedPtr                          mpCanvas;

    EventMultiplexer&                                         mrEventMultiplexer;
    EventQueue&                                               mrEventQueue;

    mutable LayerSpriteContainer                              maSprites;
    mutable ViewLayerVector                                   maViewLayers;

    basegfx::B2DPolyPolygon                                   maClip;

    basegfx::B2DHomMatrix                                     maViewTransform;
    basegfx::B2DSize                                          maUserSize;
    bool mbIsSoundEnabled;
};


SlideView::SlideView( const uno::Reference<presentation::XSlideShowView>& xView,
                      EventQueue&                                         rEventQueue,
                      EventMultiplexer&                                   rEventMultiplexer ) :
    SlideViewBase( m_aMutex ),
    mxView( xView ),
    mpCanvas(),
    mrEventMultiplexer( rEventMultiplexer ),
    mrEventQueue( rEventQueue ),
    maSprites(),
    maViewLayers(),
    maClip(),
    maViewTransform(),
    maUserSize( 1.0, 1.0 ), // default size: one-by-one rectangle
    mbIsSoundEnabled(true)
{
    // take care not constructing any UNO references to this _inside_
    // ctor, shift that code to createSlideView()!
    ENSURE_OR_THROW( mxView.is(),
                      "SlideView::SlideView(): Invalid view" );

    mpCanvas = cppcanvas::VCLFactory::createSpriteCanvas(
        xView->getCanvas() );
    ENSURE_OR_THROW( mpCanvas,
                      "Could not create cppcanvas" );

    geometry::AffineMatrix2D aViewTransform(
        xView->getTransformation() );

    if( basegfx::fTools::equalZero(
            basegfx::B2DVector(aViewTransform.m00,
                               aViewTransform.m10).getLength()) ||
        basegfx::fTools::equalZero(
            basegfx::B2DVector(aViewTransform.m01,
                               aViewTransform.m11).getLength()) )
    {
        OSL_FAIL( "SlideView::SlideView(): Singular matrix!" );

        canvas::tools::setIdentityAffineMatrix2D(aViewTransform);
    }

    basegfx::unotools::homMatrixFromAffineMatrix(
        maViewTransform, aViewTransform );

    // once and forever: set fixed prio to this 'layer' (we're always
    // the background layer)
    maSprites.setLayerPriority( basegfx::B1DRange(0.0,1.0) );
}

void SlideView::disposing()
{
    osl::MutexGuard aGuard( m_aMutex );

    maViewLayers.clear();
    maSprites.clear();
    mpCanvas.reset();

    // additionally, also de-register from XSlideShowView
    if (mxView.is())
    {
        mxView->removeTransformationChangedListener( this );
        mxView->removePaintListener( this );
        mxView.clear();
    }
}

ViewLayerSharedPtr SlideView::createViewLayer( const basegfx::B2DRange& rLayerBounds ) const
{
    osl::MutexGuard aGuard( m_aMutex );

    ENSURE_OR_THROW( mpCanvas,
                      "SlideView::createViewLayer(): Disposed" );

    const std::size_t nNumLayers( maViewLayers.size() );

    // avoid filling up layer vector with lots of deceased layer weak
    // ptrs
    if( nNumLayers > LAYER_ULLAGE )
        pruneLayers();

    std::shared_ptr<SlideViewLayer> xViewLayer( new SlideViewLayer(mpCanvas,
                                                                     getTransformation(),
                                                                     rLayerBounds,
                                                                     maUserSize,
                                                                     this) );
    maViewLayers.push_back(xViewLayer);

    return xViewLayer;
}

bool SlideView::updateScreen() const
{
    osl::MutexGuard aGuard( m_aMutex );

    ENSURE_OR_RETURN_FALSE( mpCanvas.get(),
                       "SlideView::updateScreen(): Disposed" );

    return mpCanvas->updateScreen( false );
}

bool SlideView::paintScreen() const
{
    osl::MutexGuard aGuard( m_aMutex );

    ENSURE_OR_RETURN_FALSE( mpCanvas.get(),
                       "SlideView::paintScreen(): Disposed" );

    return mpCanvas->updateScreen( true );
}

void SlideView::clear() const
{
    osl::MutexGuard aGuard( m_aMutex );

    OSL_ENSURE( mxView.is() && mpCanvas,
                "SlideView::clear(): Disposed" );
    if( !mxView.is() || !mpCanvas )
        return;

    // keep layer clip
    clearRect(getCanvas()->clone(),
              getLayerBoundsPixel(
                  basegfx::B2DRange(0,0,
                                    maUserSize.getX(),
                                    maUserSize.getY()),
                  getTransformation()));
}

void SlideView::clearAll() const
{
    osl::MutexGuard aGuard( m_aMutex );

    OSL_ENSURE( mxView.is() && mpCanvas,
                "SlideView::clear(): Disposed" );
    if( !mxView.is() || !mpCanvas )
        return;

    mpCanvas->clear(); // this is unnecessary, strictly speaking. but
                       // it makes the SlideView behave exactly like a
                       // sprite-based SlideViewLayer, because those
                       // are created from scratch after a resize

    // clear whole view
    mxView->clear();
}

void SlideView::setViewSize( const basegfx::B2DSize& rSize )
{
    osl::MutexGuard aGuard( m_aMutex );

    maUserSize = rSize;
    updateCanvas();
}

void SlideView::setCursorShape( sal_Int16 nPointerShape )
{
    osl::MutexGuard const guard( m_aMutex );

    if (mxView.is())
        mxView->setMouseCursor( nPointerShape );
}

bool SlideView::isOnView(std::shared_ptr<View> const& rView) const
{
    return rView.get() == this;
}

cppcanvas::CanvasSharedPtr SlideView::getCanvas() const
{
    osl::MutexGuard aGuard( m_aMutex );

    ENSURE_OR_THROW( mpCanvas,
                      "SlideView::getCanvas(): Disposed" );

    return mpCanvas;
}

cppcanvas::CustomSpriteSharedPtr SlideView::createSprite(
    const basegfx::B2DSize& rSpriteSizePixel,
    double                  nPriority ) const
{
    osl::MutexGuard aGuard( m_aMutex );

    ENSURE_OR_THROW( mpCanvas, "SlideView::createSprite(): Disposed" );

    cppcanvas::CustomSpriteSharedPtr pSprite(
        mpCanvas->createCustomSprite( rSpriteSizePixel ) );

    maSprites.addSprite( pSprite,
                         nPriority );

    return pSprite;
}

void SlideView::setPriority( const basegfx::B1DRange& /*rRange*/ )
{
    osl::MutexGuard aGuard( m_aMutex );

    OSL_FAIL( "SlideView::setPriority() is a NOOP for slide view - "
                "content will always be shown in the background" );
}

basegfx::B2DHomMatrix SlideView::getTransformation() const
{
    osl::MutexGuard aGuard( m_aMutex );

    basegfx::B2DHomMatrix aMatrix;
    aMatrix.scale( 1.0/maUserSize.getX(), 1.0/maUserSize.getY() );

    return maViewTransform * aMatrix;
}

geometry::IntegerSize2D SlideView::getTranslationOffset() const
{
    return mxView->getTranslationOffset();
}

basegfx::B2DHomMatrix SlideView::getSpriteTransformation() const
{
    return getTransformation();
}

void SlideView::setClip( const basegfx::B2DPolyPolygon& rClip )
{
    osl::MutexGuard aGuard( m_aMutex );

    basegfx::B2DPolyPolygon aNewClip = prepareClip( rClip );

    if( aNewClip != maClip )
    {
        maClip = aNewClip;

        updateClip();
    }
}

bool SlideView::resize( const ::basegfx::B2DRange& /*rArea*/ )
{
    osl::MutexGuard aGuard( m_aMutex );

    OSL_FAIL( "SlideView::resize(): ignored for the View, can't change size "
                "effectively, anyway" );

    return false;
}

uno::Reference<presentation::XSlideShowView> SlideView::getUnoView() const
{
    osl::MutexGuard aGuard( m_aMutex );
    return mxView;
}

void SlideView::setIsSoundEnabled (const bool bValue)
{
    mbIsSoundEnabled = bValue;
}

bool SlideView::isSoundEnabled() const
{
    return mbIsSoundEnabled;
}

void SlideView::_dispose()
{
    dispose();
}

// XEventListener
void SlideView::disposing( lang::EventObject const& evt )
{
    (void)evt;

    // no deregistration necessary anymore, XView has left:
    osl::MutexGuard const guard( m_aMutex );

    if (mxView.is())
    {
        OSL_ASSERT( evt.Source == mxView );
        mxView.clear();
    }

    dispose();
}

// silly wrapper to check that event handlers don't touch dead SlideView
struct WeakRefWrapper
{
    SlideView & m_rObj;
    uno::WeakReference<uno::XInterface> const m_wObj;
    std::function<void (SlideView&)> const m_func;

    WeakRefWrapper(SlideView & rObj, std::function<void (SlideView&)> const& func)
        : m_rObj(rObj)
        , m_wObj(static_cast<::cppu::OWeakObject*>(&rObj))
        , m_func(func)
    {
    }

    void operator()()
    {
        uno::Reference<uno::XInterface> const xObj(m_wObj);
        if (xObj.is())
        {
            m_func(m_rObj);
        }
    }
};

// XModifyListener
void SlideView::modified( const lang::EventObject& /*aEvent*/ )
{
    osl::MutexGuard const guard( m_aMutex );

    OSL_ENSURE( mxView.is(), "SlideView::modified(): "
                "Disposed, but event received from XSlideShowView?!");

    if( !mxView.is() )
        return;

    geometry::AffineMatrix2D aViewTransform(
        mxView->getTransformation() );

    if( basegfx::fTools::equalZero(
            basegfx::B2DVector(aViewTransform.m00,
                               aViewTransform.m10).getLength()) ||
        basegfx::fTools::equalZero(
            basegfx::B2DVector(aViewTransform.m01,
                               aViewTransform.m11).getLength()) )
    {
        OSL_FAIL( "SlideView::modified(): Singular matrix!" );

        canvas::tools::setIdentityAffineMatrix2D(aViewTransform);
    }

    // view transformation really changed?
    basegfx::B2DHomMatrix aNewTransform;
    basegfx::unotools::homMatrixFromAffineMatrix(
        aNewTransform,
        aViewTransform );

    if( aNewTransform == maViewTransform )
        return; // No change, nothing to do

    maViewTransform = aNewTransform;

    updateCanvas();

    // notify view change. Don't call EventMultiplexer directly, this
    // might not be the main thread!
    mrEventQueue.addEvent(
        makeEvent( WeakRefWrapper(*this,
            [] (SlideView & rThis) { rThis.mrEventMultiplexer.notifyViewChanged(rThis.mxView); }),
                   "EventMultiplexer::notifyViewChanged"));
}

// XPaintListener
void SlideView::windowPaint( const awt::PaintEvent& /*e*/ )
{
    osl::MutexGuard aGuard( m_aMutex );

    OSL_ENSURE( mxView.is() && mpCanvas, "Disposed, but event received?!" );

    // notify view clobbering. Don't call EventMultiplexer directly,
    // this might not be the main thread!
    mrEventQueue.addEvent(
        makeEvent( WeakRefWrapper(*this,
            [] (SlideView & rThis) { rThis.mrEventMultiplexer.notifyViewClobbered(rThis.mxView); }),
                   "EventMultiplexer::notifyViewClobbered") );
}

void SlideView::updateCanvas()
{
    OSL_ENSURE( mpCanvas,
                "SlideView::updateCanvasTransform(): Disposed" );

    if( !mpCanvas || !mxView.is())
        return;

    clearAll();
    mpCanvas->setTransformation( getTransformation() );
    mpCanvas->setClip(
        createClipPolygon( maClip,
                           mpCanvas,
                           maUserSize ));

    // forward update to viewlayers
    pruneLayers( true );
}

void SlideView::updateClip()
{
    OSL_ENSURE( mpCanvas,
                "SlideView::updateClip(): Disposed" );

    if( !mpCanvas )
        return;

    mpCanvas->setClip(
        createClipPolygon( maClip,
                           mpCanvas,
                           maUserSize ));

    pruneLayers();
}

void SlideView::pruneLayers( bool bWithViewLayerUpdate ) const
{
    ViewLayerVector aValidLayers;

    const basegfx::B2DHomMatrix& rCurrTransform(
        getTransformation() );

    // check all layers for validity, and retain only the live ones
    for( const auto& rView : maViewLayers )
    {
        std::shared_ptr< SlideViewLayer > xCurrLayer( rView.lock() );

        if ( xCurrLayer )
        {
            aValidLayers.push_back( xCurrLayer );

            if( bWithViewLayerUpdate )
                xCurrLayer->updateView( rCurrTransform,
                                        maUserSize );
        }
    }

    // replace layer list with pruned one
    maViewLayers.swap( aValidLayers );
}

} // anonymous namespace

UnoViewSharedPtr createSlideView( uno::Reference< presentation::XSlideShowView> const& xView,
                                  EventQueue&                                          rEventQueue,
                                  EventMultiplexer&                                    rEventMultiplexer )
{
    std::shared_ptr<SlideView> const that(
        comphelper::make_shared_from_UNO(
            new SlideView(xView,
                          rEventQueue,
                          rEventMultiplexer)));

    // register listeners with XSlideShowView
    xView->addTransformationChangedListener( that.get() );
    xView->addPaintListener( that.get() );

    // set new transformation
    that->updateCanvas();

    return that;
}

} // namespace internal
} // namespace slideshow

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
