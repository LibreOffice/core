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


#include <osl/diagnose.hxx>
#include <tools/diagnose_ex.h>
#include <sal/log.hxx>
#include <canvas/canvastools.hxx>
#include <cppcanvas/basegfxfactory.hxx>

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/numeric/ftools.hxx>

#include <com/sun/star/awt/SystemPointer.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/drawing/XMasterPageTarget.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/presentation/ParagraphTarget.hpp>
#include <com/sun/star/presentation/EffectNodeType.hpp>
#include <com/sun/star/drawing/TextAnimationKind.hpp>

#include <cppuhelper/exc_hlp.hxx>
#include <comphelper/anytostring.hxx>

#include <slide.hxx>
#include <slideshowcontext.hxx>
#include "slideanimations.hxx"
#include <doctreenode.hxx>
#include <screenupdater.hxx>
#include <cursormanager.hxx>
#include <shapeimporter.hxx>
#include <slideshowexceptions.hxx>
#include <eventqueue.hxx>
#include <activitiesqueue.hxx>
#include "layermanager.hxx"
#include "shapemanagerimpl.hxx"
#include <usereventqueue.hxx>
#include "userpaintoverlay.hxx"
#include <event.hxx>
#include "targetpropertiescreator.hxx"
#include <tools.hxx>

#include <iterator>
#include <functional>
#include <iostream>

using namespace ::com::sun::star;


namespace slideshow
{
namespace internal
{
namespace
{

class SlideImpl : public Slide,
                  public CursorManager,
                  public ViewEventHandler,
                  public ::osl::DebugBase<SlideImpl>
{
public:
    SlideImpl( const uno::Reference<drawing::XDrawPage>&         xDrawPage,
               const uno::Reference<drawing::XDrawPagesSupplier>&    xDrawPages,
               const uno::Reference<animations::XAnimationNode>& xRootNode,
               EventQueue&                                       rEventQueue,
               EventMultiplexer&                                 rEventMultiplexer,
               ScreenUpdater&                                    rScreenUpdater,
               ActivitiesQueue&                                  rActivitiesQueue,
               UserEventQueue&                                   rUserEventQueue,
               CursorManager&                                    rCursorManager,
               MediaFileManager&                                 rMediaFileManager,
               const UnoViewContainer&                           rViewContainer,
               const uno::Reference<uno::XComponentContext>&     xContext,
               const ShapeEventListenerMap&                      rShapeListenerMap,
               const ShapeCursorMap&                             rShapeCursorMap,
               const PolyPolygonVector&                          rPolyPolygonVector,
               RGBColor const&                                   rUserPaintColor,
               double                                            dUserPaintStrokeWidth,
               bool                                              bUserPaintEnabled,
               bool                                              bIntrinsicAnimationsAllowed,
               bool                                              bDisableAnimationZOrder );

    virtual ~SlideImpl() override;


    // Slide interface


    virtual void prefetch() override;
    virtual void show( bool ) override;
    virtual void hide() override;

    virtual basegfx::B2ISize getSlideSize() const override;
    virtual uno::Reference<drawing::XDrawPage > getXDrawPage() const override;
    virtual uno::Reference<animations::XAnimationNode> getXAnimationNode() const override;
    virtual PolyPolygonVector getPolygons() override;
    virtual void drawPolygons() const override;
    virtual bool isPaintOverlayActive() const override;
    virtual void enablePaintOverlay() override;
    virtual void disablePaintOverlay() override;
    virtual void update_settings( bool bUserPaintEnabled, RGBColor const& aUserPaintColor, double dUserPaintStrokeWidth ) override;


    // TODO(F2): Rework SlideBitmap to no longer be based on XBitmap,
    // but on canvas-independent basegfx bitmaps
    virtual SlideBitmapSharedPtr getCurrentSlideBitmap( const UnoViewSharedPtr& rView ) const override;


private:
    // ViewEventHandler
    virtual void viewAdded( const UnoViewSharedPtr& rView ) override;
    virtual void viewRemoved( const UnoViewSharedPtr& rView ) override;
    virtual void viewChanged( const UnoViewSharedPtr& rView ) override;
    virtual void viewsChanged() override;

    // CursorManager
    virtual bool requestCursor( sal_Int16 nCursorShape ) override;
    virtual void resetCursor() override;

    void activatePaintOverlay();
    void deactivatePaintOverlay();

    /** Query whether the slide has animations at all

        If the slide doesn't have animations, show() displays
        only static content. If an event is registered with
        registerSlideEndEvent(), this event will be
        immediately activated at the end of the show() method.

        @return true, if this slide has animations, false
        otherwise
    */
    bool isAnimated();

    /// Set all Shapes to their initial attributes for slideshow
    bool applyInitialShapeAttributes( const css::uno::Reference< css::animations::XAnimationNode >& xRootAnimationNode );

    /// Set shapes to attributes corresponding to initial or final state of slide
    void applyShapeAttributes(
        const css::uno::Reference< css::animations::XAnimationNode >& xRootAnimationNode,
        bool bInitial) const;

    /// Renders current slide content to bitmap
    SlideBitmapSharedPtr createCurrentSlideBitmap(
        const UnoViewSharedPtr& rView,
        ::basegfx::B2ISize const & rSlideSize ) const;

    /// Prefetch all shapes (not the animations)
    bool loadShapes();

    /// Retrieve slide size from XDrawPage
    basegfx::B2ISize getSlideSizeImpl() const;

    /// Prefetch show, but don't call applyInitialShapeAttributes()
    bool implPrefetchShow();

    /// Add Polygons to the member maPolygons
    void addPolygons(const PolyPolygonVector& rPolygons);

    // Types
    // =====

    enum SlideAnimationState
    {
        CONSTRUCTING_STATE=0,
        INITIAL_STATE=1,
        SHOWING_STATE=2,
        FINAL_STATE=3,
        SlideAnimationState_NUM_ENTRIES=4
    };

    typedef std::vector< SlideBitmapSharedPtr > VectorOfSlideBitmaps;
    /** Vector of slide bitmaps.

        Since the bitmap content is sensitive to animation
        effects, we have an inner vector containing a distinct
        bitmap for each of the SlideAnimationStates.
    */
    typedef ::std::vector< std::pair< UnoViewSharedPtr,
                                      VectorOfSlideBitmaps > > VectorOfVectorOfSlideBitmaps;


    // Member variables
    // ================

    /// The page model object
    uno::Reference< drawing::XDrawPage >                mxDrawPage;
    uno::Reference< drawing::XDrawPagesSupplier >       mxDrawPagesSupplier;
    uno::Reference< animations::XAnimationNode >        mxRootNode;

    LayerManagerSharedPtr                               mpLayerManager;
    std::shared_ptr<ShapeManagerImpl>                 mpShapeManager;
    std::shared_ptr<SubsettableShapeManager>          mpSubsettableShapeManager;

    /// Contains common objects needed throughout the slideshow
    SlideShowContext                                    maContext;

    /// parent cursor manager
    CursorManager&                                      mrCursorManager;

    /// Handles the animation and event generation for us
    SlideAnimations                                     maAnimations;
    PolyPolygonVector                                   maPolygons;

    RGBColor                                            maUserPaintColor;
    double                                              mdUserPaintStrokeWidth;
    UserPaintOverlaySharedPtr                           mpPaintOverlay;

    /// Bitmaps with slide content at various states
    mutable VectorOfVectorOfSlideBitmaps                maSlideBitmaps;

    SlideAnimationState                                 meAnimationState;

    const basegfx::B2ISize                              maSlideSize;

    sal_Int16                                           mnCurrentCursor;

    /// True, when intrinsic shape animations are allowed
    bool const                                          mbIntrinsicAnimationsAllowed;

    /// True, when user paint overlay is enabled
    bool                                                mbUserPaintOverlayEnabled;

    /// True, if initial load of all page shapes succeeded
    bool                                                mbShapesLoaded;

    /// True, if initial load of all animation info succeeded
    bool                                                mbShowLoaded;

    /** True, if this slide is not static.

        If this slide has animated content, this variable will
        be true, and false otherwise.
    */
    bool                                                mbHaveAnimations;

    /** True, if this slide has a main animation sequence.

        If this slide has animation content, which in turn has
        a main animation sequence (which must be fully run
        before EventMultiplexer::notifySlideAnimationsEnd() is
        called), this member is true.
    */
    bool                                                mbMainSequenceFound;

    /// When true, show() was called. Slide hidden otherwise.
    bool                                                mbActive;

    /// When true, enablePaintOverlay was called and mbUserPaintOverlay = true
    bool                                                mbPaintOverlayActive;

    /// When true, final state attributes are already applied to shapes
    bool                                                mbFinalStateApplied;
};


void slideRenderer( SlideImpl const * pSlide, const UnoViewSharedPtr& rView )
{
    // fully clear view content to background color
    rView->clearAll();

    SlideBitmapSharedPtr         pBitmap( pSlide->getCurrentSlideBitmap( rView ) );
    ::cppcanvas::CanvasSharedPtr pCanvas( rView->getCanvas() );

    const ::basegfx::B2DHomMatrix   aViewTransform( rView->getTransformation() );
    const ::basegfx::B2DPoint       aOutPosPixel( aViewTransform * ::basegfx::B2DPoint() );

    // setup a canvas with device coordinate space, the slide
    // bitmap already has the correct dimension.
    ::cppcanvas::CanvasSharedPtr pDevicePixelCanvas( pCanvas->clone() );
    pDevicePixelCanvas->setTransformation( ::basegfx::B2DHomMatrix() );

    // render at given output position
    pBitmap->move( aOutPosPixel );

    // clear clip (might have been changed, e.g. from comb
    // transition)
    pBitmap->clip( ::basegfx::B2DPolyPolygon() );
    pBitmap->draw( pDevicePixelCanvas );
}


SlideImpl::SlideImpl( const uno::Reference< drawing::XDrawPage >&           xDrawPage,
                      const uno::Reference<drawing::XDrawPagesSupplier>&    xDrawPages,
                      const uno::Reference< animations::XAnimationNode >&   xRootNode,
                      EventQueue&                                           rEventQueue,
                      EventMultiplexer&                                     rEventMultiplexer,
                      ScreenUpdater&                                        rScreenUpdater,
                      ActivitiesQueue&                                      rActivitiesQueue,
                      UserEventQueue&                                       rUserEventQueue,
                      CursorManager&                                        rCursorManager,
                      MediaFileManager&                                     rMediaFileManager,
                      const UnoViewContainer&                               rViewContainer,
                      const uno::Reference< uno::XComponentContext >&       xComponentContext,
                      const ShapeEventListenerMap&                          rShapeListenerMap,
                      const ShapeCursorMap&                                 rShapeCursorMap,
                      const PolyPolygonVector&                              rPolyPolygonVector,
                      RGBColor const&                                       aUserPaintColor,
                      double                                                dUserPaintStrokeWidth,
                      bool                                                  bUserPaintEnabled,
                      bool                                                  bIntrinsicAnimationsAllowed,
                      bool                                                  bDisableAnimationZOrder ) :
    mxDrawPage( xDrawPage ),
    mxDrawPagesSupplier( xDrawPages ),
    mxRootNode( xRootNode ),
    mpLayerManager( new LayerManager(
                        rViewContainer,
                        bDisableAnimationZOrder) ),
    mpShapeManager( new ShapeManagerImpl(
                        rEventMultiplexer,
                        mpLayerManager,
                        rCursorManager,
                        rShapeListenerMap,
                        rShapeCursorMap)),
    mpSubsettableShapeManager( mpShapeManager ),
    maContext( mpSubsettableShapeManager,
               rEventQueue,
               rEventMultiplexer,
               rScreenUpdater,
               rActivitiesQueue,
               rUserEventQueue,
               *this,
               rMediaFileManager,
               rViewContainer,
               xComponentContext ),
    mrCursorManager( rCursorManager ),
    maAnimations( maContext,
                  basegfx::B2DSize( getSlideSizeImpl() ) ),
    maPolygons(rPolyPolygonVector),
    maUserPaintColor(aUserPaintColor),
    mdUserPaintStrokeWidth(dUserPaintStrokeWidth),
    mpPaintOverlay(),
    maSlideBitmaps(),
    meAnimationState( CONSTRUCTING_STATE ),
    maSlideSize(getSlideSizeImpl()),
    mnCurrentCursor( awt::SystemPointer::ARROW ),
    mbIntrinsicAnimationsAllowed( bIntrinsicAnimationsAllowed ),
    mbUserPaintOverlayEnabled(bUserPaintEnabled),
    mbShapesLoaded( false ),
    mbShowLoaded( false ),
    mbHaveAnimations( false ),
    mbMainSequenceFound( false ),
    mbActive( false ),
    mbPaintOverlayActive( false ),
    mbFinalStateApplied( false )
{
    // clone already existing views for slide bitmaps
    for( const auto& rView : rViewContainer )
        viewAdded( rView );

    // register screen update (LayerManager needs to signal pending
    // updates)
    maContext.mrScreenUpdater.addViewUpdate(mpShapeManager);
}

void SlideImpl::update_settings( bool bUserPaintEnabled, RGBColor const& aUserPaintColor, double dUserPaintStrokeWidth )
{
    maUserPaintColor = aUserPaintColor;
    mdUserPaintStrokeWidth = dUserPaintStrokeWidth;
    mbUserPaintOverlayEnabled = bUserPaintEnabled;
}

SlideImpl::~SlideImpl()
{
    if( mpShapeManager )
    {
        maContext.mrScreenUpdater.removeViewUpdate(mpShapeManager);
        mpShapeManager->dispose();

        // TODO(Q3): Make sure LayerManager (and thus Shapes) dies
        // first, because SlideShowContext has SubsettableShapeManager
        // as reference member.
        mpLayerManager.reset();
    }
}

void SlideImpl::prefetch()
{
    if( !mxRootNode.is() )
        return;

    applyInitialShapeAttributes(mxRootNode);
}

void SlideImpl::show( bool bSlideBackgoundPainted )
{
    if( mbActive )
        return; // already active

    if( !mpShapeManager || !mpLayerManager )
        return; // disposed

    // set initial shape attributes (e.g. hide shapes that have
    // 'appear' effect set)
    if( !applyInitialShapeAttributes(mxRootNode) )
        return;

    // activate and take over view - clears view, if necessary
    mbActive = true;
    requestCursor( mnCurrentCursor );

    // enable shape management & event broadcasting for shapes of this
    // slide. Also enables LayerManager to record updates. Currently,
    // never let LayerManager render initial slide content, use
    // buffered slide bitmaps instead.
    mpShapeManager->activate();


    // render slide to screen, if requested
    if( !bSlideBackgoundPainted )
    {
        for( const auto& rContext : maContext.mrViewContainer )
            slideRenderer( this, rContext );

        maContext.mrScreenUpdater.notifyUpdate();
    }


    // fire up animations
    const bool bIsAnimated( isAnimated() );
    if( bIsAnimated )
        maAnimations.start(); // feeds initial events into queue

    // NOTE: this looks slightly weird, but is indeed correct:
    // as isAnimated() might return false, _although_ there is
    // a main sequence (because the animation nodes don't
    // contain any executable effects), we gotta check both
    // conditions here.
    if( !bIsAnimated || !mbMainSequenceFound )
    {
        // manually trigger a slide animation end event (we don't have
        // animations at all, or we don't have a main animation
        // sequence, but if we had, it'd end now). Note that having
        // animations alone does not matter here, as only main
        // sequence animations prevents showing the next slide on
        // nextEvent().
        maContext.mrEventMultiplexer.notifySlideAnimationsEnd();
    }

    // enable shape-intrinsic animations (drawing layer animations or
    // GIF animations)
    if( mbIntrinsicAnimationsAllowed )
        mpSubsettableShapeManager->notifyIntrinsicAnimationsEnabled();

    // enable paint overlay, if maUserPaintColor is valid
    activatePaintOverlay();


    // from now on, animations might be showing
    meAnimationState = SHOWING_STATE;
}

void SlideImpl::hide()
{
    if( !mbActive || !mpShapeManager )
        return; // already hidden/disposed


    // from now on, all animations are stopped
    meAnimationState = FINAL_STATE;


    // disable user paint overlay under all circumstances,
    // this slide now ceases to be active.
    deactivatePaintOverlay();


    // switch off all shape-intrinsic animations.
    mpSubsettableShapeManager->notifyIntrinsicAnimationsDisabled();

    // force-end all SMIL animations, too
    maAnimations.end();


    // disable shape management & event broadcasting for shapes of this
    // slide. Also disables LayerManager.
    mpShapeManager->deactivate();

    // vanish from view
    resetCursor();
    mbActive = false;
}

basegfx::B2ISize SlideImpl::getSlideSize() const
{
    return maSlideSize;
}

uno::Reference<drawing::XDrawPage > SlideImpl::getXDrawPage() const
{
    return mxDrawPage;
}

uno::Reference<animations::XAnimationNode> SlideImpl::getXAnimationNode() const
{
    return mxRootNode;
}

PolyPolygonVector SlideImpl::getPolygons()
{
    if(mbPaintOverlayActive)
        maPolygons = mpPaintOverlay->getPolygons();
    return maPolygons;
}

SlideBitmapSharedPtr SlideImpl::getCurrentSlideBitmap( const UnoViewSharedPtr& rView ) const
{
    // search corresponding entry in maSlideBitmaps (which
    // contains the views as the key)
    VectorOfVectorOfSlideBitmaps::iterator       aIter;
    const VectorOfVectorOfSlideBitmaps::iterator aEnd( maSlideBitmaps.end() );
    if( (aIter=std::find_if( maSlideBitmaps.begin(),
                             aEnd,
                             [&rView]
                             ( const VectorOfVectorOfSlideBitmaps::value_type& cp )
                             { return rView == cp.first; } ) ) == aEnd )
    {
        // corresponding view not found - maybe view was not
        // added to Slide?
        ENSURE_OR_THROW( false,
                          "SlideImpl::getInitialSlideBitmap(): view does not "
                          "match any of the added ones" );
    }

    // ensure that the show is loaded
    if( !mbShowLoaded )
    {
        // only prefetch and init shapes when not done already
        // (otherwise, at least applyInitialShapeAttributes() will be
        // called twice for initial slide rendering). Furthermore,
        // applyInitialShapeAttributes() _always_ performs
        // initializations, which would be highly unwanted during a
        // running show. OTOH, a slide whose mbShowLoaded is false is
        // guaranteed not be running a show.

        // set initial shape attributes (e.g. hide 'appear' effect
        // shapes)
        if( !const_cast<SlideImpl*>(this)->applyInitialShapeAttributes( mxRootNode ) )
            ENSURE_OR_THROW(false,
                             "SlideImpl::getCurrentSlideBitmap(): Cannot "
                             "apply initial attributes");
    }

    SlideBitmapSharedPtr&     rBitmap( aIter->second.at( meAnimationState ));
    const ::basegfx::B2ISize& rSlideSize(
        getSlideSizePixel( ::basegfx::B2DSize( getSlideSize() ),
                           rView ));

    // is the bitmap valid (actually existent, and of correct
    // size)?
    if( !rBitmap || rBitmap->getSize() != rSlideSize )
    {
        // no bitmap there yet, or wrong size - create one
        rBitmap = createCurrentSlideBitmap(rView, rSlideSize);
    }

    return rBitmap;
}


// private methods


void SlideImpl::viewAdded( const UnoViewSharedPtr& rView )
{
    maSlideBitmaps.emplace_back( rView,
                        VectorOfSlideBitmaps(SlideAnimationState_NUM_ENTRIES) );

    if( mpLayerManager )
        mpLayerManager->viewAdded( rView );
}

void SlideImpl::viewRemoved( const UnoViewSharedPtr& rView )
{
    if( mpLayerManager )
        mpLayerManager->viewRemoved( rView );

    const VectorOfVectorOfSlideBitmaps::iterator aEnd( maSlideBitmaps.end() );
    maSlideBitmaps.erase(
        std::remove_if( maSlideBitmaps.begin(),
                        aEnd,
                        [&rView]
                        ( const VectorOfVectorOfSlideBitmaps::value_type& cp )
                        { return rView == cp.first; } ),
                        aEnd );
}

void SlideImpl::viewChanged( const UnoViewSharedPtr& rView )
{
    // nothing to do for the Slide - getCurrentSlideBitmap() lazily
    // handles bitmap resizes
    if( mbActive && mpLayerManager )
        mpLayerManager->viewChanged(rView);
}

void SlideImpl::viewsChanged()
{
    // nothing to do for the Slide - getCurrentSlideBitmap() lazily
    // handles bitmap resizes
    if( mbActive && mpLayerManager )
        mpLayerManager->viewsChanged();
}

bool SlideImpl::requestCursor( sal_Int16 nCursorShape )
{
    mnCurrentCursor = nCursorShape;
    return mrCursorManager.requestCursor(mnCurrentCursor);
}

void SlideImpl::resetCursor()
{
    mnCurrentCursor = awt::SystemPointer::ARROW;
    mrCursorManager.resetCursor();
}

bool SlideImpl::isAnimated()
{
    // prefetch, but don't apply initial shape attributes
    if( !implPrefetchShow() )
        return false;

    return mbHaveAnimations && maAnimations.isAnimated();
}

SlideBitmapSharedPtr SlideImpl::createCurrentSlideBitmap( const UnoViewSharedPtr&   rView,
                                                          const ::basegfx::B2ISize& rBmpSize ) const
{
    ENSURE_OR_THROW( rView && rView->getCanvas(),
                      "SlideImpl::createCurrentSlideBitmap(): Invalid view" );
    ENSURE_OR_THROW( mpLayerManager,
                      "SlideImpl::createCurrentSlideBitmap(): Invalid layer manager" );
    ENSURE_OR_THROW( mbShowLoaded,
                      "SlideImpl::createCurrentSlideBitmap(): No show loaded" );

    // tdf#96083 ensure end state settings are applied to shapes once when bitmap gets re-rendered
    // in that state
    if(!mbFinalStateApplied && FINAL_STATE == meAnimationState && mxRootNode.is())
    {
        const_cast< SlideImpl* >(this)->mbFinalStateApplied = true;
        applyShapeAttributes(mxRootNode, false);
    }

    ::cppcanvas::CanvasSharedPtr pCanvas( rView->getCanvas() );

    // create a bitmap of appropriate size
    ::cppcanvas::BitmapSharedPtr pBitmap(
        ::cppcanvas::BaseGfxFactory::createBitmap(
            pCanvas,
            rBmpSize ) );

    ENSURE_OR_THROW( pBitmap,
                      "SlideImpl::createCurrentSlideBitmap(): Cannot create page bitmap" );

    ::cppcanvas::BitmapCanvasSharedPtr pBitmapCanvas( pBitmap->getBitmapCanvas() );

    ENSURE_OR_THROW( pBitmapCanvas,
                      "SlideImpl::createCurrentSlideBitmap(): Cannot create page bitmap canvas" );

    // apply linear part of destination canvas transformation (linear means in this context:
    // transformation without any translational components)
    ::basegfx::B2DHomMatrix aLinearTransform( rView->getTransformation() );
    aLinearTransform.set( 0, 2, 0.0 );
    aLinearTransform.set( 1, 2, 0.0 );
    pBitmapCanvas->setTransformation( aLinearTransform );

    // output all shapes to bitmap
    initSlideBackground( pBitmapCanvas, rBmpSize );
    mpLayerManager->renderTo( pBitmapCanvas );

    return std::make_shared<SlideBitmap>( pBitmap );
}

class MainSequenceSearcher
{
public:
    MainSequenceSearcher()
    {
        maSearchKey.Name = "node-type";
        maSearchKey.Value <<= presentation::EffectNodeType::MAIN_SEQUENCE;
    }

    void operator()( const uno::Reference< animations::XAnimationNode >& xChildNode )
    {
        uno::Sequence< beans::NamedValue > aUserData( xChildNode->getUserData() );

        if( findNamedValue( aUserData, maSearchKey ) )
        {
            maMainSequence = xChildNode;
        }
    }

    const uno::Reference< animations::XAnimationNode >& getMainSequence() const
    {
        return maMainSequence;
    }

private:
    beans::NamedValue                               maSearchKey;
    uno::Reference< animations::XAnimationNode >    maMainSequence;
};

bool SlideImpl::implPrefetchShow()
{
    if( mbShowLoaded )
        return true;

    ENSURE_OR_RETURN_FALSE( mxDrawPage.is(),
                       "SlideImpl::implPrefetchShow(): Invalid draw page" );
    ENSURE_OR_RETURN_FALSE( mpLayerManager,
                       "SlideImpl::implPrefetchShow(): Invalid layer manager" );

    // fetch desired page content
    // ==========================

    if( !loadShapes() )
        return false;

    // New animations framework: import the shape effect info
    // ======================================================

    try
    {
        if( mxRootNode.is() )
        {
            if( !maAnimations.importAnimations( mxRootNode ) )
            {
                OSL_FAIL( "SlideImpl::implPrefetchShow(): have animation nodes, "
                            "but import animations failed." );

                // could not import animation framework,
                // _although_ some animation nodes are there -
                // this is an error (not finding animations at
                // all is okay - might be a static slide)
                return false;
            }

            // now check whether we've got a main sequence (if
            // not, we must manually call
            // EventMultiplexer::notifySlideAnimationsEnd()
            // above, as e.g. interactive sequences alone
            // don't block nextEvent() from issuing the next
            // slide)
            MainSequenceSearcher aSearcher;
            if( for_each_childNode( mxRootNode, aSearcher ) )
                mbMainSequenceFound = aSearcher.getMainSequence().is();

            // import successfully done
            mbHaveAnimations = true;
        }
    }
    catch( uno::RuntimeException& )
    {
        throw;
    }
    catch( uno::Exception& )
    {
        SAL_WARN( "slideshow", exceptionToString(cppu::getCaughtException()) );
        // TODO(E2): Error handling. For now, bail out
    }

    mbShowLoaded = true;

    return true;
}

void SlideImpl::enablePaintOverlay()
{
    if( !mbUserPaintOverlayEnabled || !mbPaintOverlayActive )
    {
        mbUserPaintOverlayEnabled = true;
        activatePaintOverlay();
    }
}

void SlideImpl::disablePaintOverlay()
{
}

void SlideImpl::activatePaintOverlay()
{
    if( mbUserPaintOverlayEnabled || !maPolygons.empty() )
    {
        mpPaintOverlay = UserPaintOverlay::create( maUserPaintColor,
                                                   mdUserPaintStrokeWidth,
                                                   maContext,
                                                   maPolygons,
                                                   mbUserPaintOverlayEnabled );
        mbPaintOverlayActive = true;
    }
}

void SlideImpl::drawPolygons() const
{
    if( mpPaintOverlay  )
        mpPaintOverlay->drawPolygons();
}

void SlideImpl::addPolygons(const PolyPolygonVector& rPolygons)
{
    for (const auto& rxPolygon : rPolygons)
        maPolygons.push_back(rxPolygon);
}

bool SlideImpl::isPaintOverlayActive() const
{
    return mbPaintOverlayActive;
}

void SlideImpl::deactivatePaintOverlay()
{
    if(mbPaintOverlayActive)
        maPolygons = mpPaintOverlay->getPolygons();

    mpPaintOverlay.reset();
    mbPaintOverlayActive = false;
}

void SlideImpl::applyShapeAttributes(
    const css::uno::Reference< css::animations::XAnimationNode >& xRootAnimationNode,
    bool bInitial) const
{
    uno::Sequence< animations::TargetProperties > aProps(
        TargetPropertiesCreator::createTargetProperties( xRootAnimationNode, bInitial ) );

    // apply extracted values to our shapes
    const ::std::size_t nSize( aProps.getLength() );
    for( ::std::size_t i=0; i<nSize; ++i )
    {
        sal_Int16                         nParaIndex( -1 );
        uno::Reference< drawing::XShape > xShape( aProps[i].Target,
                                                  uno::UNO_QUERY );

        if( !xShape.is() )
        {
            // not a shape target. Maybe a ParagraphTarget?
            presentation::ParagraphTarget aParaTarget;

            if( aProps[i].Target >>= aParaTarget )
            {
                // yep, ParagraphTarget found - extract shape
                // and index
                xShape = aParaTarget.Shape;
                nParaIndex = aParaTarget.Paragraph;
            }
        }

        if( xShape.is() )
        {
            ShapeSharedPtr pShape( mpLayerManager->lookupShape( xShape ) );

            if( !pShape )
            {
                OSL_FAIL( "SlideImpl::applyInitialShapeAttributes(): no shape found for given target" );
                continue;
            }

            AttributableShapeSharedPtr pAttrShape(
                ::std::dynamic_pointer_cast< AttributableShape >( pShape ) );

            if( !pAttrShape )
            {
                OSL_FAIL( "SlideImpl::applyInitialShapeAttributes(): shape found does not "
                            "implement AttributableShape interface" );
                continue;
            }

            if( nParaIndex != -1 )
            {
                // our target is a paragraph subset, thus look
                // this up first.
                const DocTreeNodeSupplier& rNodeSupplier( pAttrShape->getTreeNodeSupplier() );

                if( rNodeSupplier.getNumberOfTreeNodes(
                        DocTreeNode::NodeType::LogicalParagraph ) <= nParaIndex )
                {
                    OSL_FAIL( "SlideImpl::applyInitialShapeAttributes(): shape found does not "
                                "provide a subset for requested paragraph index" );
                    continue;
                }

                pAttrShape = pAttrShape->getSubset(
                    rNodeSupplier.getTreeNode(
                        nParaIndex,
                        DocTreeNode::NodeType::LogicalParagraph ) );

                if( !pAttrShape )
                {
                    OSL_FAIL( "SlideImpl::applyInitialShapeAttributes(): shape found does not "
                                "provide a subset for requested paragraph index" );
                    continue;
                }
            }

            const uno::Sequence< beans::NamedValue >& rShapeProps( aProps[i].Properties );
            const ::std::size_t nShapePropSize( rShapeProps.getLength() );
            for( ::std::size_t j=0; j<nShapePropSize; ++j )
            {
                bool bVisible=false;
                if( rShapeProps[j].Name.equalsIgnoreAsciiCase("visibility") &&
                    extractValue( bVisible,
                                  rShapeProps[j].Value,
                                  pShape,
                                  ::basegfx::B2DSize( getSlideSize() ) ))
                {
                    pAttrShape->setVisibility( bVisible );
                }
                else
                {
                    OSL_FAIL( "SlideImpl::applyInitialShapeAttributes(): Unexpected "
                                "(and unimplemented) property encountered" );
                }
            }
        }
    }
}

bool SlideImpl::applyInitialShapeAttributes(
    const uno::Reference< animations::XAnimationNode >& xRootAnimationNode )
{
    if( !implPrefetchShow() )
        return false;

    if( !xRootAnimationNode.is() )
    {
        meAnimationState = INITIAL_STATE;

        return true; // no animations - no attributes to apply -
                     // succeeded
    }

    applyShapeAttributes(xRootAnimationNode, true);

    meAnimationState = INITIAL_STATE;

    return true;
}

bool SlideImpl::loadShapes()
{
    if( mbShapesLoaded )
        return true;

    ENSURE_OR_RETURN_FALSE( mxDrawPage.is(),
                       "SlideImpl::loadShapes(): Invalid draw page" );
    ENSURE_OR_RETURN_FALSE( mpLayerManager,
                       "SlideImpl::loadShapes(): Invalid layer manager" );

    // fetch desired page content
    // ==========================

    // also take master page content
    uno::Reference< drawing::XDrawPage > xMasterPage;
    uno::Reference< drawing::XShapes >   xMasterPageShapes;
    sal_Int32                            nCurrCount(0);

    uno::Reference< drawing::XMasterPageTarget > xMasterPageTarget( mxDrawPage,
                                                                    uno::UNO_QUERY );
    if( xMasterPageTarget.is() )
    {
        xMasterPage = xMasterPageTarget->getMasterPage();
        xMasterPageShapes.set( xMasterPage,
                               uno::UNO_QUERY );

        if( xMasterPage.is() && xMasterPageShapes.is() )
        {
            // TODO(P2): maybe cache master pages here (or treat the
            // masterpage as a single metafile. At least currently,
            // masterpages do not contain animation effects)
            try
            {
                // load the masterpage shapes

                ShapeImporter aMPShapesFunctor( xMasterPage,
                                                mxDrawPage,
                                                mxDrawPagesSupplier,
                                                maContext,
                                                0, /* shape num starts at 0 */
                                                true );

                mpLayerManager->addShape(
                    aMPShapesFunctor.importBackgroundShape() );

                while( !aMPShapesFunctor.isImportDone() )
                {
                    ShapeSharedPtr const& rShape(
                        aMPShapesFunctor.importShape() );
                    if( rShape )
                        mpLayerManager->addShape( rShape );
                }
                addPolygons(aMPShapesFunctor.getPolygons());

                nCurrCount = static_cast<sal_Int32>(aMPShapesFunctor.getImportedShapesCount());
            }
            catch( uno::RuntimeException& )
            {
                throw;
            }
            catch( ShapeLoadFailedException& )
            {
                // TODO(E2): Error handling. For now, bail out
                OSL_FAIL( "SlideImpl::loadShapes(): caught ShapeLoadFailedException" );
                return false;

            }
            catch( uno::Exception& )
            {
                SAL_WARN( "slideshow", exceptionToString( cppu::getCaughtException() ) );
                return false;
            }
        }
    }

    try
    {
        // load the normal page shapes


        ShapeImporter aShapesFunctor( mxDrawPage,
                                      mxDrawPage,
                                      mxDrawPagesSupplier,
                                      maContext,
                                      nCurrCount,
                                      false );

        while( !aShapesFunctor.isImportDone() )
        {
            ShapeSharedPtr const& rShape(
                aShapesFunctor.importShape() );
            if( rShape )
                mpLayerManager->addShape( rShape );
        }
        addPolygons(aShapesFunctor.getPolygons());
    }
    catch( uno::RuntimeException& )
    {
        throw;
    }
    catch( ShapeLoadFailedException& )
    {
        // TODO(E2): Error handling. For now, bail out
        OSL_FAIL( "SlideImpl::loadShapes(): caught ShapeLoadFailedException" );
        return false;
    }
    catch( uno::Exception& )
    {
        SAL_WARN( "slideshow", exceptionToString( cppu::getCaughtException() ) );
        return false;
    }

    mbShapesLoaded = true;

    return true;
}

basegfx::B2ISize SlideImpl::getSlideSizeImpl() const
{
    uno::Reference< beans::XPropertySet > xPropSet(
        mxDrawPage, uno::UNO_QUERY_THROW );

    sal_Int32 nDocWidth = 0;
    sal_Int32 nDocHeight = 0;
    xPropSet->getPropertyValue("Width") >>= nDocWidth;
    xPropSet->getPropertyValue("Height") >>= nDocHeight;

    return basegfx::B2ISize( nDocWidth, nDocHeight );
}

} // namespace


SlideSharedPtr createSlide( const uno::Reference< drawing::XDrawPage >&         xDrawPage,
                            const uno::Reference<drawing::XDrawPagesSupplier>&  xDrawPages,
                            const uno::Reference< animations::XAnimationNode >& xRootNode,
                            EventQueue&                                         rEventQueue,
                            EventMultiplexer&                                   rEventMultiplexer,
                            ScreenUpdater&                                      rScreenUpdater,
                            ActivitiesQueue&                                    rActivitiesQueue,
                            UserEventQueue&                                     rUserEventQueue,
                            CursorManager&                                      rCursorManager,
                            MediaFileManager&                                   rMediaFileManager,
                            const UnoViewContainer&                             rViewContainer,
                            const uno::Reference< uno::XComponentContext >&     xComponentContext,
                            const ShapeEventListenerMap&                        rShapeListenerMap,
                            const ShapeCursorMap&                               rShapeCursorMap,
                            const PolyPolygonVector&                            rPolyPolygonVector,
                            RGBColor const&                                     rUserPaintColor,
                            double                                              dUserPaintStrokeWidth,
                            bool                                                bUserPaintEnabled,
                            bool                                                bIntrinsicAnimationsAllowed,
                            bool                                                bDisableAnimationZOrder )
{
    std::shared_ptr<SlideImpl> pRet( new SlideImpl( xDrawPage, xDrawPages, xRootNode, rEventQueue,
                                                      rEventMultiplexer, rScreenUpdater,
                                                      rActivitiesQueue, rUserEventQueue,
                                                      rCursorManager, rMediaFileManager, rViewContainer,
                                                      xComponentContext, rShapeListenerMap,
                                                      rShapeCursorMap, rPolyPolygonVector, rUserPaintColor,
                                                      dUserPaintStrokeWidth, bUserPaintEnabled,
                                                      bIntrinsicAnimationsAllowed,
                                                      bDisableAnimationZOrder ));

    rEventMultiplexer.addViewHandler( pRet );

    return pRet;
}

} // namespace internal
} // namespace slideshow

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
