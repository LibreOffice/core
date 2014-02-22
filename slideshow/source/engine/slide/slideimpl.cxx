/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */


#include <osl/diagnose.hxx>
#include <canvas/debug.hxx>
#include <tools/diagnose_ex.h>
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
#include <com/sun/star/animations/XAnimationNodeSupplier.hpp>
#include <com/sun/star/animations/XTargetPropertiesCreator.hpp>
#include <com/sun/star/drawing/TextAnimationKind.hpp>

#include <animations/animationnodehelper.hxx>

#include <cppuhelper/exc_hlp.hxx>
#include <comphelper/anytostring.hxx>

#include "slide.hxx"
#include "slideshowcontext.hxx"
#include "slideanimations.hxx"
#include "doctreenode.hxx"
#include "screenupdater.hxx"
#include "cursormanager.hxx"
#include "shapeimporter.hxx"
#include "slideshowexceptions.hxx"
#include "eventqueue.hxx"
#include "activitiesqueue.hxx"
#include "layermanager.hxx"
#include "shapemanagerimpl.hxx"
#include "usereventqueue.hxx"
#include "userpaintoverlay.hxx"
#include "event.hxx"
#include "tools.hxx"

#include <o3tl/compat_functional.hxx>

#include <boost/bind.hpp>
#include <iterator>
#include <algorithm>
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

    ~SlideImpl();


    
    

    virtual void dispose();


    
    

    virtual bool prefetch();
    virtual bool show( bool );
    virtual void hide();

    virtual basegfx::B2ISize getSlideSize() const;
    virtual uno::Reference<drawing::XDrawPage > getXDrawPage() const;
    virtual uno::Reference<animations::XAnimationNode> getXAnimationNode() const;
    virtual PolyPolygonVector getPolygons();
    virtual void drawPolygons() const;
    virtual bool isPaintOverlayActive() const;
    virtual void enablePaintOverlay();
    virtual void disablePaintOverlay();
    virtual void update_settings( bool bUserPaintEnabled, RGBColor const& aUserPaintColor, double dUserPaintStrokeWidth );


    
    
    virtual SlideBitmapSharedPtr getCurrentSlideBitmap( const UnoViewSharedPtr& rView ) const;


private:
    
    virtual void viewAdded( const UnoViewSharedPtr& rView );
    virtual void viewRemoved( const UnoViewSharedPtr& rView );
    virtual void viewChanged( const UnoViewSharedPtr& rView );
    virtual void viewsChanged();

    
    virtual bool requestCursor( sal_Int16 nCursorShape );
    virtual void resetCursor();

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

    
    bool applyInitialShapeAttributes( const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::animations::XAnimationNode >& xRootAnimationNode );

    
    SlideBitmapSharedPtr createCurrentSlideBitmap(
        const UnoViewSharedPtr& rView,
        ::basegfx::B2ISize const & rSlideSize ) const;

    
    bool loadShapes();

    
    basegfx::B2ISize getSlideSizeImpl() const;

    
    bool implPrefetchShow();

    
    ::basegfx::B2DRectangle getSlideRect() const;

    
    void endIntrinsicAnimations();

    
    void startIntrinsicAnimations();

    
    void addPolygons(PolyPolygonVector aPolygons);

    
    

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


    
    

    
    uno::Reference< drawing::XDrawPage >                mxDrawPage;
    uno::Reference< drawing::XDrawPagesSupplier >       mxDrawPagesSupplier;
    uno::Reference< animations::XAnimationNode >        mxRootNode;

    LayerManagerSharedPtr                               mpLayerManager;
    boost::shared_ptr<ShapeManagerImpl>                 mpShapeManager;
    boost::shared_ptr<SubsettableShapeManager>          mpSubsettableShapeManager;

    
    SlideShowContext                                    maContext;

    
    CursorManager&                                      mrCursorManager;

    
    SlideAnimations                                     maAnimations;
    PolyPolygonVector                                   maPolygons;

    RGBColor                                            maUserPaintColor;
    double                                              mdUserPaintStrokeWidth;
    UserPaintOverlaySharedPtr                           mpPaintOverlay;

    
    mutable VectorOfVectorOfSlideBitmaps                maSlideBitmaps;

    SlideAnimationState                                 meAnimationState;

    const basegfx::B2ISize                              maSlideSize;

    sal_Int16                                           mnCurrentCursor;

    
    bool                                                mbIntrinsicAnimationsAllowed;

    
    bool                                                mbUserPaintOverlayEnabled;

    
    bool                                                mbShapesLoaded;

    
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

    
    bool                                                mbActive;

    
    bool                                                mbPaintOverlayActive;
};





class SlideRenderer
{
public:
    explicit SlideRenderer( SlideImpl& rSlide ) :
        mrSlide( rSlide )
    {
    }

    void operator()( const UnoViewSharedPtr& rView )
    {
        
        rView->clearAll();

        SlideBitmapSharedPtr         pBitmap( mrSlide.getCurrentSlideBitmap( rView ) );
        ::cppcanvas::CanvasSharedPtr pCanvas( rView->getCanvas() );

        const ::basegfx::B2DHomMatrix   aViewTransform( rView->getTransformation() );
        const ::basegfx::B2DPoint       aOutPosPixel( aViewTransform * ::basegfx::B2DPoint() );

        
        
        ::cppcanvas::CanvasSharedPtr pDevicePixelCanvas( pCanvas->clone() );
        pDevicePixelCanvas->setTransformation( ::basegfx::B2DHomMatrix() );

        
        pBitmap->move( aOutPosPixel );

        
        
        pBitmap->clip( ::basegfx::B2DPolyPolygon() );
        pBitmap->draw( pDevicePixelCanvas );
    }

private:
    SlideImpl& mrSlide;
};





SlideImpl::SlideImpl( const uno::Reference< drawing::XDrawPage >&           xDrawPage,
                      const uno::Reference<drawing::XDrawPagesSupplier>&    xDrawPages,
                      const uno::Reference< animations::XAnimationNode >&   xRootNode,
                      EventQueue&                                           rEventQueue,
                      EventMultiplexer&                                     rEventMultiplexer,
                      ScreenUpdater&                                        rScreenUpdater,
                      ActivitiesQueue&                                      rActivitiesQueue,
                      UserEventQueue&                                       rUserEventQueue,
                      CursorManager&                                        rCursorManager,
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
                        getSlideRect(),
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
    mbPaintOverlayActive( false )
{
    
    std::for_each( rViewContainer.begin(),
                   rViewContainer.end(),
                   boost::bind( &SlideImpl::viewAdded,
                                this,
                                _1 ));

    
    
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

        
        
        
        mpLayerManager.reset();
    }
}

void SlideImpl::dispose()
{
    maSlideBitmaps.clear();
    mpPaintOverlay.reset();
    maAnimations.dispose();
    maContext.dispose();

    if( mpShapeManager )
    {
        maContext.mrScreenUpdater.removeViewUpdate(mpShapeManager);
        mpShapeManager->dispose();
    }

    
    
    
    mpLayerManager.reset();
    mpSubsettableShapeManager.reset();
    mpShapeManager.reset();
    mxRootNode.clear();
    mxDrawPage.clear();
    mxDrawPagesSupplier.clear();
}

bool SlideImpl::prefetch()
{
    if( !mxRootNode.is() )
        return false;

    return applyInitialShapeAttributes(mxRootNode);
}

bool SlideImpl::show( bool bSlideBackgoundPainted )
{
    

    if( mbActive )
        return true; 

    if( !mpShapeManager || !mpLayerManager )
        return false; 

    

    
    
    if( !applyInitialShapeAttributes(mxRootNode) )
        return false;

    

    
    mbActive = true;
    requestCursor( mnCurrentCursor );

    
    
    
    
    mpShapeManager->activate( true );

    

    
    if( !bSlideBackgoundPainted )
    {
        std::for_each(maContext.mrViewContainer.begin(),
                      maContext.mrViewContainer.end(),
                      boost::mem_fn(&View::clearAll));

        std::for_each( maContext.mrViewContainer.begin(),
                       maContext.mrViewContainer.end(),
                       SlideRenderer(*this) );
        maContext.mrScreenUpdater.notifyUpdate();
    }

    

    
    const bool bIsAnimated( isAnimated() );
    if( bIsAnimated )
        maAnimations.start(); 

    
    
    
    
    
    if( !bIsAnimated || !mbMainSequenceFound )
    {
        
        
        
        
        
        
        maContext.mrEventMultiplexer.notifySlideAnimationsEnd();
    }

    
    
    if( mbIntrinsicAnimationsAllowed )
        startIntrinsicAnimations();

    

    
    activatePaintOverlay();

    

    
    meAnimationState = SHOWING_STATE;

    return true;
}

void SlideImpl::hide()
{
    if( !mbActive || !mpShapeManager )
        return; 

    

    
    meAnimationState = FINAL_STATE;

    

    
    
    deactivatePaintOverlay();

    

    
    endIntrinsicAnimations();

    
    maAnimations.end();

    

    
    
    mpShapeManager->deactivate();

    
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
    
    
    VectorOfVectorOfSlideBitmaps::iterator       aIter;
    const VectorOfVectorOfSlideBitmaps::iterator aEnd( maSlideBitmaps.end() );
    if( (aIter=std::find_if( maSlideBitmaps.begin(),
                             aEnd,
                             boost::bind(
                                 std::equal_to<UnoViewSharedPtr>(),
                                 rView,
                                 
                                 boost::bind(
                                     o3tl::select1st<VectorOfVectorOfSlideBitmaps::value_type>(),
                                     _1 )))) == aEnd )
    {
        
        
        ENSURE_OR_THROW( false,
                          "SlideImpl::getInitialSlideBitmap(): view does not "
                          "match any of the added ones" );
    }

    
    if( !mbShowLoaded )
    {
        
        
        
        
        
        
        

        
        
        if( !const_cast<SlideImpl*>(this)->applyInitialShapeAttributes( mxRootNode ) )
            ENSURE_OR_THROW(false,
                             "SlideImpl::getCurrentSlideBitmap(): Cannot "
                             "apply initial attributes");
    }

    SlideBitmapSharedPtr&     rBitmap( aIter->second.at( meAnimationState ));
    const ::basegfx::B2ISize& rSlideSize(
        getSlideSizePixel( ::basegfx::B2DSize( getSlideSize() ),
                           rView ));

    
    
    if( !rBitmap || rBitmap->getSize() != rSlideSize )
    {
        
        rBitmap = createCurrentSlideBitmap(rView, rSlideSize);
    }

    return rBitmap;
}






void SlideImpl::viewAdded( const UnoViewSharedPtr& rView )
{
    maSlideBitmaps.push_back(
        std::make_pair( rView,
                        VectorOfSlideBitmaps(SlideAnimationState_NUM_ENTRIES) ));

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
                        boost::bind(
                            std::equal_to<UnoViewSharedPtr>(),
                            rView,
                            
                            boost::bind(
                                o3tl::select1st<VectorOfVectorOfSlideBitmaps::value_type>(),
                                _1 ))),
        aEnd );
}

void SlideImpl::viewChanged( const UnoViewSharedPtr& rView )
{
    
    
    if( mbActive && mpLayerManager )
        mpLayerManager->viewChanged(rView);
}

void SlideImpl::viewsChanged()
{
    
    
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

    ::cppcanvas::CanvasSharedPtr pCanvas( rView->getCanvas() );

    
    ::cppcanvas::BitmapSharedPtr pBitmap(
        ::cppcanvas::BaseGfxFactory::getInstance().createBitmap(
            pCanvas,
            rBmpSize ) );

    ENSURE_OR_THROW( pBitmap,
                      "SlideImpl::createCurrentSlideBitmap(): Cannot create page bitmap" );

    ::cppcanvas::BitmapCanvasSharedPtr pBitmapCanvas( pBitmap->getBitmapCanvas() );

    ENSURE_OR_THROW( pBitmapCanvas,
                      "SlideImpl::createCurrentSlideBitmap(): Cannot create page bitmap canvas" );

    
    
    ::basegfx::B2DHomMatrix aLinearTransform( rView->getTransformation() );
    aLinearTransform.set( 0, 2, 0.0 );
    aLinearTransform.set( 1, 2, 0.0 );
    pBitmapCanvas->setTransformation( aLinearTransform );

    
    initSlideBackground( pBitmapCanvas, rBmpSize );
    mpLayerManager->renderTo( pBitmapCanvas );

    return SlideBitmapSharedPtr( new SlideBitmap( pBitmap ) );
}

namespace
{
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

        uno::Reference< animations::XAnimationNode > getMainSequence() const
        {
            return maMainSequence;
        }

    private:
        beans::NamedValue                               maSearchKey;
        uno::Reference< animations::XAnimationNode >    maMainSequence;
    };
}

bool SlideImpl::implPrefetchShow()
{
    if( mbShowLoaded )
        return true;

    ENSURE_OR_RETURN_FALSE( mxDrawPage.is(),
                       "SlideImpl::implPrefetchShow(): Invalid draw page" );
    ENSURE_OR_RETURN_FALSE( mpLayerManager,
                       "SlideImpl::implPrefetchShow(): Invalid layer manager" );

    
    

    if( !loadShapes() )
        return false;

    
    

    try
    {
        if( mxRootNode.is() )
        {
            if( !maAnimations.importAnimations( mxRootNode ) )
            {
                OSL_FAIL( "SlideImpl::implPrefetchShow(): have animation nodes, "
                            "but import animations failed." );

                
                
                
                
                return false;
            }

            
            
            
            
            
            
            MainSequenceSearcher aSearcher;
            if( ::anim::for_each_childNode( mxRootNode, aSearcher ) )
                mbMainSequenceFound = aSearcher.getMainSequence().is();

            
            mbHaveAnimations = true;
        }
    }
    catch( uno::RuntimeException& )
    {
        throw;
    }
    catch( uno::Exception& )
    {
        OSL_FAIL(
            OUStringToOString(
                comphelper::anyToString(cppu::getCaughtException()),
                RTL_TEXTENCODING_UTF8).getStr() );
        
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

void SlideImpl::addPolygons(PolyPolygonVector aPolygons)
{
    if(!aPolygons.empty())
    {
        for( PolyPolygonVector::iterator aIter=aPolygons.begin(),
                 aEnd=aPolygons.end();
             aIter!=aEnd;
             ++aIter )
        {
            maPolygons.push_back(*aIter);
        }
    }
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

::basegfx::B2DRectangle SlideImpl::getSlideRect() const
{
    const basegfx::B2ISize slideSize( getSlideSizeImpl() );
    return ::basegfx::B2DRectangle(0.0,0.0,
                                   slideSize.getX(),
                                   slideSize.getY());
}

void SlideImpl::endIntrinsicAnimations()
{
    mpSubsettableShapeManager->notifyIntrinsicAnimationsDisabled();
}

void SlideImpl::startIntrinsicAnimations()
{
    mpSubsettableShapeManager->notifyIntrinsicAnimationsEnabled();
}

bool SlideImpl::applyInitialShapeAttributes(
    const uno::Reference< animations::XAnimationNode >& xRootAnimationNode )
{
    if( !implPrefetchShow() )
        return false;

    if( !xRootAnimationNode.is() )
    {
        meAnimationState = INITIAL_STATE;

        return true; 
                     
    }

    uno::Reference< animations::XTargetPropertiesCreator > xPropsCreator;

    try
    {
        ENSURE_OR_RETURN_FALSE( maContext.mxComponentContext.is(),
                           "SlideImpl::applyInitialShapeAttributes(): Invalid component context" );

        uno::Reference<lang::XMultiComponentFactory> xFac(
            maContext.mxComponentContext->getServiceManager() );

        xPropsCreator.set(
            xFac->createInstanceWithContext(
                OUString(
                                     "com.sun.star.animations.TargetPropertiesCreator"),
                maContext.mxComponentContext ),
            uno::UNO_QUERY_THROW );
    }
    catch( uno::RuntimeException& )
    {
        throw;
    }
    catch( uno::Exception& )
    {
        OSL_FAIL(
            OUStringToOString(
                comphelper::anyToString(cppu::getCaughtException()),
                RTL_TEXTENCODING_UTF8).getStr() );

        
        
        
        ENSURE_OR_RETURN_FALSE( false,
                           "SlideImpl::applyInitialShapeAttributes(): "
                           "couldn't create TargetPropertiesCreator." );
    }

    uno::Sequence< animations::TargetProperties > aProps(
        xPropsCreator->createInitialTargetProperties( xRootAnimationNode ) );

    
    const ::std::size_t nSize( aProps.getLength() );
    for( ::std::size_t i=0; i<nSize; ++i )
    {
        sal_Int16                         nParaIndex( -1 );
        uno::Reference< drawing::XShape > xShape( aProps[i].Target,
                                                  uno::UNO_QUERY );

        if( !xShape.is() )
        {
            
            presentation::ParagraphTarget aParaTarget;

            if( (aProps[i].Target >>= aParaTarget) )
            {
                
                
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
                ::boost::dynamic_pointer_cast< AttributableShape >( pShape ) );

            if( !pAttrShape )
            {
                OSL_FAIL( "SlideImpl::applyInitialShapeAttributes(): shape found does not "
                            "implement AttributableShape interface" );
                continue;
            }

            if( nParaIndex != -1 )
            {
                
                
                const DocTreeNodeSupplier& rNodeSupplier( pAttrShape->getTreeNodeSupplier() );

                if( rNodeSupplier.getNumberOfTreeNodes(
                        DocTreeNode::NODETYPE_LOGICAL_PARAGRAPH ) <= nParaIndex )
                {
                    OSL_FAIL( "SlideImpl::applyInitialShapeAttributes(): shape found does not "
                                "provide a subset for requested paragraph index" );
                    continue;
                }

                pAttrShape = pAttrShape->getSubset(
                    rNodeSupplier.getTreeNode(
                        nParaIndex,
                        DocTreeNode::NODETYPE_LOGICAL_PARAGRAPH ) );

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
            
            
            
            try
            {
                
                
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
                
                OSL_FAIL( "SlideImpl::loadShapes(): caught ShapeLoadFailedException" );
                return false;

            }
            catch( uno::Exception& )
            {
                OSL_FAIL( OUStringToOString(
                                comphelper::anyToString( cppu::getCaughtException() ),
                                RTL_TEXTENCODING_UTF8 ).getStr() );

                return false;
            }
        }
    }

    try
    {
        
        

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
        
        OSL_FAIL( "SlideImpl::loadShapes(): caught ShapeLoadFailedException" );
        return false;
    }
    catch( uno::Exception& )
    {
        OSL_FAIL( OUStringToOString(
                        comphelper::anyToString( cppu::getCaughtException() ),
                        RTL_TEXTENCODING_UTF8 ).getStr() );

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

} 


SlideSharedPtr createSlide( const uno::Reference< drawing::XDrawPage >&         xDrawPage,
                            const uno::Reference<drawing::XDrawPagesSupplier>&  xDrawPages,
                            const uno::Reference< animations::XAnimationNode >& xRootNode,
                            EventQueue&                                         rEventQueue,
                            EventMultiplexer&                                   rEventMultiplexer,
                            ScreenUpdater&                                      rScreenUpdater,
                            ActivitiesQueue&                                    rActivitiesQueue,
                            UserEventQueue&                                     rUserEventQueue,
                            CursorManager&                                      rCursorManager,
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
    boost::shared_ptr<SlideImpl> pRet( new SlideImpl( xDrawPage, xDrawPages, xRootNode, rEventQueue,
                                                      rEventMultiplexer, rScreenUpdater,
                                                      rActivitiesQueue, rUserEventQueue,
                                                      rCursorManager, rViewContainer,
                                                      xComponentContext, rShapeListenerMap,
                                                      rShapeCursorMap, rPolyPolygonVector, rUserPaintColor,
                                                      dUserPaintStrokeWidth, bUserPaintEnabled,
                                                      bIntrinsicAnimationsAllowed,
                                                      bDisableAnimationZOrder ));

    rEventMultiplexer.addViewHandler( pRet );

    return pRet;
}

} 
} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
