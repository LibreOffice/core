/*************************************************************************
 *
 *  $RCSfile: slidetransitionfactory.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2005-01-21 17:08:43 $
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

#include <basegfx/numeric/ftools.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>

#include <canvas/debug.hxx>
#include <cppcanvas/basegfxfactory.hxx>

#include <com/sun/star/animations/TransitionType.hpp>
#include <com/sun/star/animations/TransitionSubType.hpp>

#include <unoview.hxx>
#include <transitionfactory.hxx>
#include <transitiontools.hxx>
#include <parametricpolypolygonfactory.hxx>
#include <animationfactory.hxx>
#include <clippingfunctor.hxx>
#include <combtransition.hxx>

#ifndef BOOST_BIND_HPP_INCLUDED
#include <boost/bind.hpp>
#endif


using namespace ::com::sun::star;

namespace presentation {
namespace internal {


/***************************************************
 ***                                             ***
 ***          Slide Transition Effects           ***
 ***                                             ***
 ***************************************************/

namespace
{
    // helper methods
    // =============================================

    void fillPage( const ::cppcanvas::CanvasSharedPtr& rDestinationCanvas,
                   const ::basegfx::B2DSize&           rPageSizePixel,
                   const RGBColor&                     rFillColor )
    {
        // need to render without any transformation (we
        // assume rPageSizePixel to represent device units)
        ::cppcanvas::CanvasSharedPtr pDevicePixelCanvas( rDestinationCanvas->clone() );
        pDevicePixelCanvas->setTransformation( ::basegfx::B2DHomMatrix() );

        // TODO(F2): Properly respect clip here. Might have to be transformed, too.
        const ::basegfx::B2DHomMatrix   aViewTransform( rDestinationCanvas->getTransformation() );
        const ::basegfx::B2DPoint       aOutputPosPixel( aViewTransform * ::basegfx::B2DPoint() );

        const ::basegfx::B2DPolygon aPoly( ::basegfx::tools::createPolygonFromRect(
                                               ::basegfx::B2DRectangle(aOutputPosPixel.getX(),
                                                                       aOutputPosPixel.getY(),
                                                                       aOutputPosPixel.getX() + rPageSizePixel.getX(),
                                                                       aOutputPosPixel.getY() + rPageSizePixel.getY() ) ) );

        ::cppcanvas::PolyPolygonSharedPtr pPolyPoly(
            ::cppcanvas::BaseGfxFactory::getInstance().createPolyPolygon( pDevicePixelCanvas,
                                                                          aPoly ) );

        if( pPolyPoly.get() )
        {
            pPolyPoly->setRGBAFillColor( rFillColor.getIntegerColor() );
            pPolyPoly->draw();
        }
    }

    /** Base class for all slide change effects.

        This class provides the basic sprite and view handling
        functionality. Derived classes should normally only need to
        implement the perform() method.
     */
    class SlideChangeBase : public SlideChangeAnimation
    {
    public:
        /** Create a new SlideChanger, for the given leaving and
            entering slide bitmaps

            @param rLeavingBitmap
            Bitmap that contains the slide image of the leaving
            slide. Can be NULL.

            @param rEnteringBitmap
            Bitmap that contains the slide image of the entering slide

            @param rSoundPlayer
            Sound to play, while transition is running. Use NULL
            for no sound.
        */
        SlideChangeBase( const SlideBitmapSharedPtr& rLeavingBitmap,
                         const SlideBitmapSharedPtr& rEnteringBitmap,
                         const SoundPlayerSharedPtr& rSoundPlayer );

        // NumberAnimation
        virtual bool operator()( double x );
        virtual double getUnderlyingValue() const;

        // Animation
        virtual void start( const AnimatableShapeSharedPtr&,
                            const ShapeAttributeLayerSharedPtr& );
        virtual void end();

        // SlideChangeAnimation
        virtual void addView( const ViewSharedPtr& rView );
        virtual bool removeView( const ViewSharedPtr& rView );

    protected:
        /** Query the size of the bitmaps in device pixel
         */
        ::basegfx::B2DSize getBitmapSize() const;

    private:
        /** Called on derived classes to implement actual slide change.

            This method is called with the sprite of the slide coming 'in'

            @param rSprite
            Current sprite to operate on. This is the sprite of the
            'entering' slide

            @param x
            Current parameter value
         */
        virtual void performIn( const ::cppcanvas::CustomSpriteSharedPtr&   rSprite,
                                const ::cppcanvas::CanvasSharedPtr&         rDestinationCanvas,
                                double                                      t ) = 0;

        /** Called on derived classes to implement actual slide change.

            This method is called with the sprite of the slide moving 'out'

            @param rSprite
            Current sprite to operate on. This is the sprite of the
            'leaving' slide

            @param x
            Current parameter value
         */
        virtual void performOut( const ::cppcanvas::CustomSpriteSharedPtr&  rSprite,
                                 const ::cppcanvas::CanvasSharedPtr&        rDestinationCanvas,
                                 double                                     t ) = 0;

        ViewVector                                              maViews;
        SlideBitmapSharedPtr                                    mpLeavingBitmap;
        SlideBitmapSharedPtr                                    mpEnteringBitmap;

        SoundPlayerSharedPtr                                    mpSoundPlayer;

        typedef ::std::vector< ::cppcanvas::CustomSpriteSharedPtr > SpriteVector;
        SpriteVector                                            maOutSprites;
        SpriteVector                                            maInSprites;
        bool                                                    mbSpritesVisible;
    };

    SlideChangeBase::SlideChangeBase( const SlideBitmapSharedPtr& rLeavingBitmap,
                                      const SlideBitmapSharedPtr& rEnteringBitmap,
                                      const SoundPlayerSharedPtr& rSoundPlayer ) :
        maViews(),
        mpLeavingBitmap( rLeavingBitmap ),
        mpEnteringBitmap( rEnteringBitmap ),
        mpSoundPlayer( rSoundPlayer ),
        maOutSprites(),
        maInSprites(),
        mbSpritesVisible(false)
    {
        ENSURE_AND_THROW(
            rEnteringBitmap.get(),
            "SlideChangeBase::SlideChangeBase(): Invalid entering bitmap" );
    }

    ::basegfx::B2DSize SlideChangeBase::getBitmapSize() const
    {
        return ::basegfx::B2DSize(
            ::basegfx::B2DTuple( mpEnteringBitmap->getSize() ) );
    }

    void SlideChangeBase::start( const AnimatableShapeSharedPtr&,
                                 const ShapeAttributeLayerSharedPtr& )
    {
        if( maInSprites.empty() )
        {
            // TODO(P2): change to bitmapsprite once that's working

            // create leaving slide sprites
            // ============================

            if( mpLeavingBitmap.get() )
            {
                // create a sprite for every entry in mrViews
                ::std::transform( maViews.begin(),
                                  maViews.end(),
                                  ::std::back_insert_iterator<SpriteVector>(maOutSprites),
                                  ::boost::bind( &ViewLayer::createSprite,
                                                 _1,
                                                 ::basegfx::B2DSize(
                                                     ::basegfx::B2DTuple(
                                                         mpLeavingBitmap->getSize() ) ) ) );

                // set sprite alpha to 1.0 for every sprite
                ::std::for_each( maOutSprites.begin(),
                                 maOutSprites.end(),
                                 ::boost::bind( &::cppcanvas::Sprite::setAlpha,
                                                _1,
                                                1.0) );   // default is 0.0, which seems to be
                                                          // a bad idea when viewing content...
            }

            // create entering slide sprites
            // =============================

            // create a sprite for every entry in mrViews
            ::std::transform( maViews.begin(),
                              maViews.end(),
                              ::std::back_insert_iterator<SpriteVector>(maInSprites),
                              ::boost::bind( &ViewLayer::createSprite,
                                             _1,
                                             getBitmapSize() ) );

            // set sprite alpha to 1.0 for every sprite
            ::std::for_each( maInSprites.begin(),
                             maInSprites.end(),
                             ::boost::bind( &::cppcanvas::Sprite::setAlpha,
                                            _1,
                                            1.0) );   // default is 0.0, which seems to be
                                                      // a bad idea when viewing content...

            // start accompanying sound effect, if any
            if( mpSoundPlayer.get() )
                mpSoundPlayer->startPlayback();
        }
    }

    void SlideChangeBase::end()
    {
        // end accompanying sound effect, if any
        if( mpSoundPlayer.get() )
            mpSoundPlayer->stopPlayback();

        if (mpEnteringBitmap.get() != 0)
        {
            // draw fully entered bitmap:
            ViewVector::const_iterator iPos( maViews.begin() );
            const ViewVector::const_iterator iEnd( maViews.end() );
            for ( ; iPos != iEnd; ++iPos )
            {
                const cppcanvas::CanvasSharedPtr pCanvas(
                    (*iPos)->getCanvas() );
                // need to render without any transformation (we
                // assume device units):
                const basegfx::B2DHomMatrix viewTransform(
                    pCanvas->getTransformation() );
                const basegfx::B2DPoint posPixel(
                    viewTransform * basegfx::B2DPoint() );
                const cppcanvas::CanvasSharedPtr pDevicePixelCanvas(
                    pCanvas->clone() );
                basegfx::B2DHomMatrix transform;
                transform.translate( posPixel.getX(), posPixel.getY() );
                pDevicePixelCanvas->setTransformation( transform );
                mpEnteringBitmap->draw( pDevicePixelCanvas );
            }
            // TODO: Slide::show() initial Sliderendering may be obsolete now
        }

        mbSpritesVisible = false;

        // drop all references
        mpLeavingBitmap.reset();
        mpEnteringBitmap.reset();
        maOutSprites.clear();
        maInSprites.clear();

        maViews.clear();
    }

    bool SlideChangeBase::operator()( double nValue )
    {
        if( maInSprites.empty() ||
            mpEnteringBitmap.get() == NULL )
        {
            return false;
        }

        ENSURE_AND_RETURN( maViews.size() == maInSprites.size(),
                           "SlideChangeBase::operator(): Mismatching sprite/view numbers" );

        bool bSpritesVisible( mbSpritesVisible );

        for( ::std::size_t i=0, nEntries=maInSprites.size(); i<nEntries; ++i )
        {
            // calc sprite offsets. The enter/leaving bitmaps are only
            // as large as the actual slides. For scaled-down
            // presentations, we have to move the left, top edge of
            // those bitmaps to the actual position, governed by the
            // given view transform. The aSpritePosPixel local
            // variable is already in device coordinate space
            // (i.e. pixel).

            ::cppcanvas::CanvasSharedPtr        pCanvas( maViews[i]->getCanvas() );
            ::cppcanvas::CustomSpriteSharedPtr  pInSprite( maInSprites[i] );
            ::cppcanvas::CustomSpriteSharedPtr  pOutSprite;

            if( !maOutSprites.empty() )
                pOutSprite = maOutSprites[i];

            // TODO(F2): Properly respect clip here. Might have to be transformed, too.
            const ::basegfx::B2DHomMatrix   aViewTransform( pCanvas->getTransformation() );
            const ::basegfx::B2DPoint       aSpritePosPixel( aViewTransform * ::basegfx::B2DPoint() );

            // move sprite to final output position, in
            // device coordinates
            if( pOutSprite.get() )
                pOutSprite->movePixel( aSpritePosPixel );
            pInSprite->movePixel( aSpritePosPixel );

            if( !mbSpritesVisible )
            {
                if( pOutSprite.get() )
                {
                    // only render once: clipping is done
                    // exclusively with the sprite
                    const ::cppcanvas::CanvasSharedPtr pOutContentCanvas( pOutSprite->getContentCanvas() );

                    if( pOutContentCanvas.get() )
                    {
                        // TODO(Q2): Use basegfx bitmaps here
                        // TODO(F1): SlideBitmap is not fully portable between different canvases!

                        // render the content
                        mpLeavingBitmap->draw( pOutContentCanvas );
                    }
                }

                // only render once: clipping is done
                // exclusively with the sprite
                const ::cppcanvas::CanvasSharedPtr pInContentCanvas( pInSprite->getContentCanvas() );

                if( pInContentCanvas.get() )
                {
                    // TODO(Q2): Use basegfx bitmaps here
                    // TODO(F1): SlideBitmap is not fully portable between different canvases!

                    // render the content
                    mpEnteringBitmap->draw( pInContentCanvas );
                }
            }

            // first, call perform method for 'leaving' slide, if it
            // was specified. After that, call perform on 'entering'
            // slide mandatorily.
            if( pOutSprite.get() )
                performOut( pOutSprite, pCanvas, nValue );

            performIn( pInSprite, pCanvas, nValue );

            // finishing deeds for first run.
            if( !mbSpritesVisible )
            {
                // enable sprites
                if( pOutSprite.get() )
                    pOutSprite->show();
                pInSprite->show();
                bSpritesVisible = true;
            }
        } // for_each( sprite )

        mbSpritesVisible = bSpritesVisible;

        return true;
    }

    double SlideChangeBase::getUnderlyingValue() const
    {
        return 0.0;     // though this should be used in concert with
                        // ActivitiesFactory::createSimpleActivity, better
                        // explicitely name our start value.
                        // Permissible range for operator() above is [0,1]
    }

    void SlideChangeBase::addView( const ViewSharedPtr& rView )
    {
        // TODO(Q2): Try to use UnoViewContainer here, and only create
        // new sprites here (when new views are added during animation
        // playback).
        maViews.push_back( rView );
    }

    bool SlideChangeBase::removeView( const ViewSharedPtr& rView )
    {
        // remove locally
        const ViewVector::iterator aEnd( maViews.end() );
        ViewVector::iterator aIter;
        if( (aIter=::std::remove( maViews.begin(),
                                  aEnd,
                                  rView)) == aEnd )
        {
            // view seemingly was not added, failed
            return false;
        }

        // actually erase from container
        maViews.erase( aIter, aEnd );

        return true;
    }


    class ClippedSlideChange : public SlideChangeBase
    {
    public:
        /** Create a new SlideChanger, for the given leaving and
            entering slide bitmaps, which applies the given clip
            polygon.

            @param rEnteringBitmap
            Bitmap that contains the slide image of the entering slide

            @param rSlideSize
            Slide side in user coordinate space
        */
        ClippedSlideChange( const SlideBitmapSharedPtr&             rEnteringBitmap,
                            const ParametricPolyPolygonSharedPtr&   rPolygon,
                            const TransitionInfo&                   rTransitionInfo,
                            const ::basegfx::B2DSize&               rSlideSize,
                            bool                                    bDirectionForward,
                            const SoundPlayerSharedPtr&             rSoundPlayer ) :
            SlideChangeBase( SlideBitmapSharedPtr(), // leaving bitmap
                                                     // is empty,
                                                     // we're
                                                     // leveraging the
                                                     // fact that the
                                                     // old slide is
                                                     // still
                                                     // displayed in
                                                     // the background.
                             rEnteringBitmap,
                             rSoundPlayer ),
            maClippingFunctor( rPolygon,
                               rTransitionInfo,
                               bDirectionForward,
                               true ),
            maSlideSize( rSlideSize )
        {
        }

        virtual void performIn( const ::cppcanvas::CustomSpriteSharedPtr&   rSprite,
                                const ::cppcanvas::CanvasSharedPtr&         rDestinationCanvas,
                                double                                      t )
        {
            rSprite->setClip(
                maClippingFunctor( t,
                                   maSlideSize ) );
        }

        virtual void performOut( const ::cppcanvas::CustomSpriteSharedPtr&  rSprite,
                                 const ::cppcanvas::CanvasSharedPtr&        rDestinationCanvas,
                                 double                                     t )
        {
            // not needed here
        }

    private:
        ClippingFunctor             maClippingFunctor;
        const ::basegfx::B2DSize    maSlideSize;
    };


    class FadingSlideChange : public SlideChangeBase
    {
    public:
        /** Create a new SlideChanger, for the given leaving and
            entering slide bitmaps, which applies a fade effect.

            @param rEnteringBitmap
            Bitmap that contains the slide image of the entering slide
        */
        FadingSlideChange( const SlideBitmapSharedPtr&                      rLeavingBitmap,
                           const SlideBitmapSharedPtr&                      rEnteringBitmap,
                           const ::comphelper::OptionalValue< RGBColor >&   rFadeColor,
                           const SoundPlayerSharedPtr&                      rSoundPlayer ) :
            SlideChangeBase( rLeavingBitmap,
                             rEnteringBitmap,
                             rSoundPlayer ),
            maFadeColor( rFadeColor ),
            mbFirstTurn( true )
        {
        }

        virtual void performIn( const ::cppcanvas::CustomSpriteSharedPtr&   rSprite,
                                const ::cppcanvas::CanvasSharedPtr&         rDestinationCanvas,
                                double                                      t )
        {
            ENSURE_AND_THROW( rSprite.get(),
                              "FadingSlideChange::performIn(): Invalid sprite" );

            if( maFadeColor.isValid() )
                // After half of the active time, fade in new slide
                rSprite->setAlpha( t > 0.5 ? 2.0*(t-0.5) : 0.0 );
            else
                // Fade in new slide over full active time
                rSprite->setAlpha( t );
        }

        virtual void performOut( const ::cppcanvas::CustomSpriteSharedPtr&  rSprite,
                                 const ::cppcanvas::CanvasSharedPtr&        rDestinationCanvas,
                                 double                                     t )
        {
            ENSURE_AND_THROW( rSprite.get(),
                              "FadingSlideChange::performOut(): Invalid sprite" );
            ENSURE_AND_THROW( rDestinationCanvas.get(),
                              "FadingSlideChange::performOut(): Invalid dest canvas" );

            // only needed for color fades
            if( maFadeColor.isValid() )
            {
                if( mbFirstTurn )
                {
                    mbFirstTurn = false;

                    // clear page to given fade color. 'Leaving' slide is
                    // painted atop of that, but slowly fading out.
                    fillPage( rDestinationCanvas,
                              getBitmapSize(),
                              maFadeColor.getValue() );
                }

                // Until half of the active time, fade out old
                // slide. After half of the active time, old slide
                // will be invisible.
                rSprite->setAlpha( t > 0.5 ? 0.0 : 2.0*t );
            }
        }

    private:
        const ::comphelper::OptionalValue< RGBColor >   maFadeColor;
        bool                                            mbFirstTurn;
    };

    /** To avoid ternary operator in initializer below (Solaris
     * compiler problems)
     */
    SlideBitmapSharedPtr selectLeavingBitmap( bool                          bDontUseBitmap,
                                              const SlideBitmapSharedPtr&   rBitmap )
    {
        if( bDontUseBitmap )
            return SlideBitmapSharedPtr();
        else
            return rBitmap;
    }


    class MovingSlideChange : public SlideChangeBase
    {
    public:
        /** Create a new SlideChanger, for the given entering slide
            bitmaps, which performes a moving slide change effect

            @param rLeavingBitmap
            Bitmap that contains the slide image of the leaving slide

            @param rEnteringBitmap
            Bitmap that contains the slide image of the entering slide

            @param rLeavingDirection
            Direction vector. The move is performed along this
            direction vector, starting at a position where the leaving
            slide is fully visible, and ending at a position where the
            leaving slide is just not visible. The vector must have
            unit length.

            @param rEnteringDirection
            Direction vector. The move is performed along this
            direction vector, starting at a position where the
            entering slide is just not visible, and ending at the
            final slide position. The vector must have unit length.
        */
        MovingSlideChange( const SlideBitmapSharedPtr& rLeavingBitmap,
                           const SlideBitmapSharedPtr& rEnteringBitmap,
                           const ::basegfx::B2DVector& rLeavingDirection,
                           const ::basegfx::B2DVector& rEnteringDirection,
                           bool                        bDirectionForward,
                           const SoundPlayerSharedPtr& rSoundPlayer ) :
            // Optimization: when leaving bitmap is given, but it does
            // not move, don't pass on to SlideChangeBase: we simply
            // paint it once at startup.
            SlideChangeBase(
                selectLeavingBitmap(
                    rLeavingDirection.equalZero(),
                    rLeavingBitmap ),
                rEnteringBitmap,
                rSoundPlayer ),
            maBitmapSize( getBitmapSize() ),
            mpLeavingBitmap( rLeavingBitmap ),
            // TODO(F1): calc correct length of direction
            // vector. Directions not strictly horizontal or vertical
            // must travel a longer distance.
            maLeavingDirection( maBitmapSize *
                                rLeavingDirection ),
            // TODO(F1): calc correct length of direction
            // vector. Directions not strictly horizontal or vertical
            // must travel a longer distance.
            maEnteringDirection( maBitmapSize *
                                 rEnteringDirection ),
            mbDirectionForward( bDirectionForward ),
            mbFirstPerformCall( true )
        {
        }

        void renderLeavingBitmap( const ::cppcanvas::CanvasSharedPtr& rDestinationCanvas )
        {
            if( mbFirstPerformCall )
            {
                mbFirstPerformCall = false;

                if( maLeavingDirection.equalZero() &&
                    mpLeavingBitmap.get() )
                {
                    // leaving bitmap given, which is static (and
                    // therefore not passed to SlideChangeBase): paint
                    // once on first performIn callstartup
                    const ::basegfx::B2DHomMatrix   aViewTransform( rDestinationCanvas->getTransformation() );
                    const ::basegfx::B2DPoint       aOutPosPixel( aViewTransform * ::basegfx::B2DPoint() );

                    // setup a canvas with device coordinate space,
                    // the slide bitmap already has the correct
                    // dimension.
                    ::cppcanvas::CanvasSharedPtr pDevicePixelCanvas( rDestinationCanvas->clone() );
                    pDevicePixelCanvas->setTransformation( ::basegfx::B2DHomMatrix() );

                    // render at given output position
                    mpLeavingBitmap->move( aOutPosPixel );
                    mpLeavingBitmap->draw( pDevicePixelCanvas );
                }
            }

        }

        virtual void performIn( const ::cppcanvas::CustomSpriteSharedPtr&   rSprite,
                                const ::cppcanvas::CanvasSharedPtr&         rDestinationCanvas,
                                double                                      t )
        {
            ENSURE_AND_THROW( rSprite.get(),
                              "MovingSlideChange::performIn(): Invalid sprite" );
            ENSURE_AND_THROW( rDestinationCanvas.get(),
                              "MovingSlideChange::performIn(): Invalid dest canvas" );

            renderLeavingBitmap( rDestinationCanvas );

            // TODO(F1): This does not account for non-translational
            // transformations! If the canvas is rotated, we still
            // move the sprite unrotated (which might or might not
            // produce the intended effect).
            const ::basegfx::B2DHomMatrix   aViewTransform( rDestinationCanvas->getTransformation() );
            const ::basegfx::B2DPoint       aPageOrigin( aViewTransform * ::basegfx::B2DPoint() );

            t = mbDirectionForward ? t : 1.0 - t;

            // move sprite
            rSprite->movePixel( aPageOrigin + (t-1.0)*maEnteringDirection );
        }

        virtual void performOut( const ::cppcanvas::CustomSpriteSharedPtr&  rSprite,
                                 const ::cppcanvas::CanvasSharedPtr&        rDestinationCanvas,
                                 double                                     t )
        {
            ENSURE_AND_THROW( rSprite.get(),
                              "MovingSlideChange::performOut(): Invalid sprite" );
            ENSURE_AND_THROW( rDestinationCanvas.get(),
                              "MovingSlideChange::performOut(): Invalid dest canvas" );

            renderLeavingBitmap( rDestinationCanvas );

            // TODO(F1): This does not account for non-translational
            // transformations! If the canvas is rotated, we still
            // move the sprite unrotated (which might or might not
            // produce the intended effect).
            const ::basegfx::B2DHomMatrix   aViewTransform( rDestinationCanvas->getTransformation() );
            const ::basegfx::B2DPoint       aPageOrigin( aViewTransform * ::basegfx::B2DPoint() );

            t = mbDirectionForward ? t : 1.0 - t;

            // move sprite
            rSprite->movePixel( aPageOrigin + t*maLeavingDirection );
        }

    private:
        const ::basegfx::B2DSize    maBitmapSize;

        /// Local copy, in case it's not passed on to base class
        SlideBitmapSharedPtr        mpLeavingBitmap;

        /// Direction vector for leaving slide, already scaled up to full slide size
        const ::basegfx::B2DVector  maLeavingDirection;

        /// Direction vector for entering slide, already scaled up to full slide size
        const ::basegfx::B2DVector  maEnteringDirection;

        /// When false, t parameter sweep is inverted
        const bool                  mbDirectionForward;

        bool                        mbFirstPerformCall;
    };

    ::cppcanvas::CanvasSharedPtr getCanvasFromView( const UnoViewContainer& rViews )
    {
        ENSURE_AND_THROW( !rViews.empty(),
                          "getCanvasFromView(): Empty view container!" );

        // TODO(F2): Generalize to multiple, multi-device views
        return (*rViews.begin())->getCanvas();
    }

    /** Retrieve bitmap for given slide.

        If the slide ptr is invalid, a black bitmap is created.
     */
    SlideBitmapSharedPtr retrieveSlideBitmap( const SlideSharedPtr&     rSlide,
                                              const UnoViewContainer&   rViews,
                                              const ::basegfx::B2DSize  rSlideSize )
    {
        if( rSlide.get() )
        {
            // TODO(F2): Generalize to multiple, multi-device views
            return rSlide->getCurrentSlideBitmap( *rViews.begin() );
        }
        else
        {
            ::cppcanvas::CanvasSharedPtr pCanvas( getCanvasFromView( rViews ) );

            // convert slide size to device coordinate system (i.e. pixel)
            const ::basegfx::B2DHomMatrix aViewTransform( pCanvas->getTransformation() );
            const ::basegfx::B2DSize aFloatBmpSize( aViewTransform * rSlideSize );

            // create a fully black bitmap (black is the default
            // background, used when no slides are present)
            const ::basegfx::B2ISize aBmpSize(
                ::basegfx::fround( aFloatBmpSize.getX() ),
                ::basegfx::fround( aFloatBmpSize.getY() ) );

            // create a bitmap of appropriate size
            ::cppcanvas::BitmapSharedPtr pBitmap(
                ::cppcanvas::BaseGfxFactory::getInstance().createBitmap(
                    pCanvas,
                    aBmpSize ) );

            ENSURE_AND_THROW( pBitmap.get(),
                              "retrieveLeavingBitmap(): Cannot create page bitmap" );

            ::cppcanvas::BitmapCanvasSharedPtr pBitmapCanvas( pBitmap->getBitmapCanvas() );

            ENSURE_AND_THROW( pBitmapCanvas.get(),
                              "retrieveLeavingBitmap(): Cannot create page bitmap canvas" );

            // set transformation to identitiy (->device pixel)
            pBitmapCanvas->setTransformation( ::basegfx::B2DHomMatrix() );

            // fill the bounds rectangle in white
            ::basegfx::B2DPolygon aPoly;
            aPoly.append( ::basegfx::B2DPoint() );
            aPoly.append( ::basegfx::B2DPoint(rSlideSize.getX(), 0) );
            aPoly.append( ::basegfx::B2DPoint(rSlideSize.getX(), rSlideSize.getY()) );
            aPoly.append( ::basegfx::B2DPoint(0, rSlideSize.getY()) );
            aPoly.setClosed(true);

            ::cppcanvas::PolyPolygonSharedPtr pPolyPoly(
                ::cppcanvas::BaseGfxFactory::getInstance().createPolyPolygon( pBitmapCanvas,
                                                                              aPoly ) );

            if( pPolyPoly.get() )
            {
                pPolyPoly->setRGBAFillColor( 0x00000000U );
                pPolyPoly->draw();
            }

            return SlideBitmapSharedPtr( new SlideBitmap( pBitmap ) );
        }
    }

    SlideChangeAnimationSharedPtr createPushWipeTransition(
        const SlideSharedPtr&                           rLeavingSlide,
        const SlideSharedPtr&                           rEnteringSlide,
        const UnoViewContainer&                         rViews,
        const ::basegfx::B2DSize&                       rSlideSize,
        sal_Int16                                       nTransitionType,
        sal_Int16                                       nTransitionSubType,
        bool                                            bTransitionDirection,
        const SoundPlayerSharedPtr&                     rSoundPlayer )
    {
        SlideBitmapSharedPtr pLeavingBitmap;

        SlideBitmapSharedPtr pEnteringBitmap(
            retrieveSlideBitmap( rEnteringSlide,
                                 rViews,
                                 rSlideSize ) );

        if( rLeavingSlide.get() )
        {
            // only get leaving bitmap, if we've an
            // actual slide to move out here. We
            // _don't_ need a fake black background
            // bitmap, neither for push nor for comb
            // wipes.
            pLeavingBitmap = retrieveSlideBitmap( rLeavingSlide,
                                                  rViews,
                                                  rSlideSize );
        }

        // setup direction vector
        bool bComb( false );
        ::basegfx::B2DVector aDirection;
        switch( nTransitionSubType )
        {
            default:
                OSL_ENSURE( false,
                            "createPushWipeTransition(): Unexpected transition "
                            "subtype for animations::TransitionType::PUSHWIPE transitions" );
                return SlideChangeAnimationSharedPtr();

            case animations::TransitionSubType::FROMTOP:
                aDirection = ::basegfx::B2DVector( 0.0, 1.0 );
                break;

            case animations::TransitionSubType::FROMBOTTOM:
                aDirection = ::basegfx::B2DVector( 0.0, -1.0 );
                break;

            case animations::TransitionSubType::FROMLEFT:
                aDirection = ::basegfx::B2DVector( 1.0, 0.0 );
                break;

            case animations::TransitionSubType::FROMRIGHT:
                aDirection = ::basegfx::B2DVector( -1.0, 0.0 );
                break;

            case animations::TransitionSubType::FROMBOTTOMRIGHT:
                aDirection = ::basegfx::B2DVector( -1.0, -1.0 );
                break;

            case animations::TransitionSubType::FROMBOTTOMLEFT:
                aDirection = ::basegfx::B2DVector( 1.0, -1.0 );
                break;

            case animations::TransitionSubType::FROMTOPRIGHT:
                aDirection = ::basegfx::B2DVector( -1.0, 1.0 );
                break;

            case animations::TransitionSubType::FROMTOPLEFT:
                aDirection = ::basegfx::B2DVector( 1.0, 1.0 );
                break;

            case animations::TransitionSubType::COMBHORIZONTAL:
                aDirection = ::basegfx::B2DVector( 1.0, 0.0 );
                bComb = true;
                break;

            case animations::TransitionSubType::COMBVERTICAL:
                aDirection = ::basegfx::B2DVector( 0.0, 1.0 );
                bComb = true;
                break;
        }

        if( bComb )
        {
            return SlideChangeAnimationSharedPtr(
                new CombTransition( pLeavingBitmap,
                                    pEnteringBitmap,
                                    aDirection,
                                    24,
                                    rSoundPlayer ) ); // comb with 12 stripes
        }
        else
        {
            return SlideChangeAnimationSharedPtr(
                new MovingSlideChange( pLeavingBitmap,
                                       pEnteringBitmap,
                                       aDirection,
                                       aDirection,
                                       true,
                                       rSoundPlayer ) );
        }
    }

    SlideChangeAnimationSharedPtr createSlideWipeTransition(
        const SlideSharedPtr&                           rLeavingSlide,
        const SlideSharedPtr&                           rEnteringSlide,
        const UnoViewContainer&                         rViews,
        const ::basegfx::B2DSize&                       rSlideSize,
        sal_Int16                                       nTransitionType,
        sal_Int16                                       nTransitionSubType,
        bool                                            bTransitionDirection,
        const SoundPlayerSharedPtr&                     rSoundPlayer )
    {
        // setup 'in' direction vector
        ::basegfx::B2DVector aInDirection;
        switch( nTransitionSubType )
        {
            default:
                OSL_ENSURE( false,
                            "createSlideWipeTransition(): Unexpected transition "
                            "subtype for animations::TransitionType::SLIDEWIPE transitions" );
                return SlideChangeAnimationSharedPtr();

            case animations::TransitionSubType::FROMTOP:
                aInDirection = ::basegfx::B2DVector( 0.0, 1.0 );
                break;

            case animations::TransitionSubType::FROMRIGHT:
                aInDirection = ::basegfx::B2DVector( -1.0, 0.0 );
                break;

            case animations::TransitionSubType::FROMLEFT:
                aInDirection = ::basegfx::B2DVector( 1.0, 0.0 );
                break;

            case animations::TransitionSubType::FROMBOTTOM:
                aInDirection = ::basegfx::B2DVector( 0.0, -1.0 );
                break;

            case animations::TransitionSubType::FROMBOTTOMRIGHT:
                aInDirection = ::basegfx::B2DVector( -1.0, -1.0 );
                break;

            case animations::TransitionSubType::FROMBOTTOMLEFT:
                aInDirection = ::basegfx::B2DVector( 1.0, -1.0 );
                break;

            case animations::TransitionSubType::FROMTOPRIGHT:
                aInDirection = ::basegfx::B2DVector( -1.0, 1.0 );
                break;

            case animations::TransitionSubType::FROMTOPLEFT:
                aInDirection = ::basegfx::B2DVector( 1.0, 1.0 );
                break;
        }

        if( bTransitionDirection )
        {
            // normal, 'forward' slide wipe effect. Since the old
            // content is still on screen (and does not move), we can
            // pass the 'leaving' bitmap empty.
            // =======================================================

            return SlideChangeAnimationSharedPtr(
                new MovingSlideChange( SlideBitmapSharedPtr(),
                                       retrieveSlideBitmap( rEnteringSlide,
                                                            rViews,
                                                            rSlideSize ),
                                       ::basegfx::B2DVector(),
                                       aInDirection,
                                       true,
                                       rSoundPlayer ) );
        }
        else
        {
            // 'reversed' slide wipe effect. Reverse for slide wipes
            // means, that the new slide is in the back, statically,
            // and the old one is moving off in the foreground. Since
            // the old content is moving, we need a bitmap for it in
            // any case.
            // =======================================================

            return SlideChangeAnimationSharedPtr(
                new MovingSlideChange( retrieveSlideBitmap( rEnteringSlide,
                                                            rViews,
                                                            rSlideSize ),
                                       retrieveSlideBitmap( rLeavingSlide,
                                                            rViews,
                                                            rSlideSize ),
                                       ::basegfx::B2DVector(),
                                       // flip direction, to match
                                       // 'reverse mode' for parameter sweep
                                       -1.0*aInDirection,
                                       false,
                                       rSoundPlayer ) );
        }
    }

} // anon namespace


SlideChangeAnimationSharedPtr TransitionFactory::createSlideTransition(
    const SlideSharedPtr&                           rLeavingSlide,
    const SlideSharedPtr&                           rEnteringSlide,
    const UnoViewContainer&                         rViews,
    const ::basegfx::B2DSize&                       rSlideSize,
    sal_Int16                                       nTransitionType,
    sal_Int16                                       nTransitionSubType,
    bool                                            bTransitionDirection,
    const ::comphelper::OptionalValue< RGBColor >&  rTransitionFadeColor,
    const SoundPlayerSharedPtr&                     rSoundPlayer            )
{
    ENSURE_AND_THROW(
        rEnteringSlide.get(),
        "TransitionFactory::createSlideTransition(): Invalid entering slide" );

    const TransitionInfo* pTransitionInfo(
        getTransitionInfo( nTransitionType, nTransitionSubType ) );

    ::cppcanvas::CanvasSharedPtr pCanvas( getCanvasFromView(rViews) );

    if( pTransitionInfo != NULL )
    {
        switch( pTransitionInfo->meTransitionClass )
        {
            default:
            case TransitionInfo::TRANSITION_INVALID:
                OSL_TRACE(
                    "TransitionFactory::createSlideTransition(): Invalid type/subtype (%d/%d) "
                    "combination encountered.",
                    nTransitionType,
                    nTransitionSubType );
                return SlideChangeAnimationSharedPtr();


            case TransitionInfo::TRANSITION_CLIP_POLYPOLYGON:
            {
                // generate parametric poly-polygon
                ParametricPolyPolygonSharedPtr pPoly(
                    ParametricPolyPolygonFactory::createClipPolyPolygon(
                        nTransitionType, nTransitionSubType ) );

                SlideBitmapSharedPtr pEnteringBitmap(
                    retrieveSlideBitmap( rEnteringSlide,
                                         rViews,
                                         rSlideSize ) );

                // create a clip transition from that
                return SlideChangeAnimationSharedPtr(
                    new ClippedSlideChange( pEnteringBitmap,
                                            pPoly,
                                            *pTransitionInfo,
                                            rSlideSize,
                                            bTransitionDirection,
                                            rSoundPlayer ) );
            }
            break;

            case TransitionInfo::TRANSITION_SPECIAL:
            {
                switch( nTransitionType )
                {
                    default:
                        OSL_ENSURE( false,
                                    "TransitionFactory::createSlideTransition(): Unexpected transition "
                                    "type for TRANSITION_SPECIAL transitions" );
                        return SlideChangeAnimationSharedPtr();

                    case animations::TransitionType::RANDOM:
                    {
                        // select randomly one of the effects from the
                        // TransitionFactoryTable

                        const TransitionInfo* pRandomTransitionInfo( getRandomTransitionInfo() );

                        ENSURE_AND_THROW( pRandomTransitionInfo != NULL,
                                          "TransitionFactory::createSlideTransition(): Got invalid random transition info" );

                        ENSURE_AND_THROW( pRandomTransitionInfo->mnTransitionType != animations::TransitionType::RANDOM,
                                          "TransitionFactory::createSlideTransition(): Got random again for random input!" );

                        // and recurse
                        return createSlideTransition( rLeavingSlide,
                                                      rEnteringSlide,
                                                      rViews,
                                                      rSlideSize,
                                                      pRandomTransitionInfo->mnTransitionType,
                                                      pRandomTransitionInfo->mnTransitionSubType,
                                                      bTransitionDirection,
                                                      rTransitionFadeColor,
                                                      rSoundPlayer );
                    }

                    case animations::TransitionType::PUSHWIPE:
                    {
                        return createPushWipeTransition( rLeavingSlide,
                                                         rEnteringSlide,
                                                         rViews,
                                                         rSlideSize,
                                                         nTransitionType,
                                                         nTransitionSubType,
                                                         bTransitionDirection,
                                                         rSoundPlayer );
                    }

                    case animations::TransitionType::SLIDEWIPE:
                    {
                        return createSlideWipeTransition( rLeavingSlide,
                                                          rEnteringSlide,
                                                          rViews,
                                                          rSlideSize,
                                                          nTransitionType,
                                                          nTransitionSubType,
                                                          bTransitionDirection,
                                                          rSoundPlayer );
                    }

                    case animations::TransitionType::FADE:
                    {
                        SlideBitmapSharedPtr pLeavingBitmap;

                        SlideBitmapSharedPtr pEnteringBitmap(
                            retrieveSlideBitmap( rEnteringSlide,
                                                 rViews,
                                                 rSlideSize ) );

                        if( rLeavingSlide.get() &&
                            rTransitionFadeColor.isValid() )
                        {
                            // only generate second bitmap, if fade
                            // effect really needs it.
                            pLeavingBitmap = retrieveSlideBitmap( rLeavingSlide,
                                                                  rViews,
                                                                  rSlideSize );
                        }

                        return SlideChangeAnimationSharedPtr(
                            new FadingSlideChange( pLeavingBitmap,
                                                   pEnteringBitmap,
                                                   rTransitionFadeColor,
                                                   rSoundPlayer ) );
                    }
                }
            }
            break;
        }
    }

    // No animation generated, maybe no table entry for given
    // transition?
    OSL_TRACE(
        "TransitionFactory::createSlideTransition(): Unknown type/subtype (%d/%d) "
        "combination encountered",
        nTransitionType,
        nTransitionSubType );
    OSL_ENSURE(
        false,
        "TransitionFactory::createSlideTransition(): Unknown type/subtype "
        "combination encountered" );

    return SlideChangeAnimationSharedPtr();
}

}
}
