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

#ifndef INCLUDED_SLIDESHOW_TRANSITIONS_SLIDECHANGEBASE_HXX
#define INCLUDED_SLIDESHOW_TRANSITIONS_SLIDECHANGEBASE_HXX

#include <osl/mutex.hxx>

#include "unoview.hxx"
#include "vieweventhandler.hxx"
#include "numberanimation.hxx"
#include "slide.hxx"
#include "screenupdater.hxx"
#include "soundplayer.hxx"

#include <boost/enable_shared_from_this.hpp>
#include <boost/noncopyable.hpp>
#include <boost/optional.hpp>

namespace cppcanvas
{
    class Canvas;
    class CustomSprite;
}

namespace slideshow {
namespace internal {

/** Base class for all slide change effects.

    This class provides the basic sprite and view handling
    functionality.  Derived classes should normally only need to
    implement the perform() method.
*/
class SlideChangeBase : public ViewEventHandler,
                        public NumberAnimation,
                        public boost::enable_shared_from_this<SlideChangeBase>,
                        private ::boost::noncopyable
{
public:
    // NumberAnimation
    virtual bool operator()( double x );
    virtual double getUnderlyingValue() const;

    // Animation
    virtual void prefetch( const AnimatableShapeSharedPtr&,
                           const ShapeAttributeLayerSharedPtr& );
    virtual void start( const AnimatableShapeSharedPtr&,
                        const ShapeAttributeLayerSharedPtr& );
    virtual void end();

    // ViewEventHandler
    virtual void viewAdded( const UnoViewSharedPtr& rView );
    virtual void viewRemoved( const UnoViewSharedPtr& rView );
    virtual void viewChanged( const UnoViewSharedPtr& rView );
    virtual void viewsChanged();

protected:
    /** Create a new SlideChanger, for the given leaving and
        entering slides.
    */
    SlideChangeBase(
        ::boost::optional<SlideSharedPtr> const & leavingSlide,
        const SlideSharedPtr&                     pEnteringSlide,
        const SoundPlayerSharedPtr&               pSoundPlayer,
        const UnoViewContainer&                   rViewContainer,
        ScreenUpdater&                            rScreenUpdater,
        EventMultiplexer&                         rEventMultiplexer,
        bool                                      bCreateLeavingSprites = true,
        bool                                      bCreateEnteringSprites = true );

    /// Info on a per-view basis
    struct ViewEntry
    {
        ViewEntry() {}

        explicit ViewEntry( const UnoViewSharedPtr& rView ) :
            mpView( rView )
        {
        }

        /// The view this entry is for
        UnoViewSharedPtr                              mpView;
        /// outgoing slide sprite
        boost::shared_ptr<cppcanvas::CustomSprite>    mpOutSprite;
        /// incoming slide sprite
        boost::shared_ptr<cppcanvas::CustomSprite>    mpInSprite;
        /// outgoing slide bitmap
        mutable SlideBitmapSharedPtr                  mpLeavingBitmap;
        /// incoming slide bitmap
        mutable SlideBitmapSharedPtr                  mpEnteringBitmap;

        // for algo access
        const UnoViewSharedPtr& getView() const { return mpView; }
    };

    typedef ::std::vector<ViewEntry> ViewsVecT;

    ViewsVecT::const_iterator beginViews() { return maViewData.begin(); }
    ViewsVecT::const_iterator endViews() { return maViewData.end(); }

    SlideBitmapSharedPtr getLeavingBitmap( const ViewEntry& rViewEntry ) const;
    SlideBitmapSharedPtr getEnteringBitmap( const ViewEntry& rViewEntry ) const;

    SlideBitmapSharedPtr createBitmap( const UnoViewSharedPtr&                pView,
                                       const boost::optional<SlideSharedPtr>& rSlide_ ) const;

    ::basegfx::B2ISize getEnteringSlideSizePixel( const UnoViewSharedPtr& pView ) const;

    void renderBitmap( SlideBitmapSharedPtr const&                 pSlideBitmap,
                       boost::shared_ptr<cppcanvas::Canvas> const& pCanvas );

    /** Called on derived classes to perform actions before first run.

        This typically involves rendering of the initial slide content.

        @param rViewEntry the view entry

        @param rDestinationCanvas the canvas to render on
     */
    virtual void prepareForRun(
            const ViewEntry& rViewEntry,
            const cppcanvas::CanvasSharedPtr& rDestinationCanvas );

    /** Called on derived classes to implement actual slide change.

        This method is called with the sprite of the slide coming 'in'

        @param rSprite
        Current sprite to operate on. This is the sprite of the
        'entering' slide

        @param t
        Current parameter value
    */
    virtual void performIn(
        const boost::shared_ptr<cppcanvas::CustomSprite>&   rSprite,
        const ViewEntry&                                    rViewEntry,
        const boost::shared_ptr<cppcanvas::Canvas>&         rDestinationCanvas,
        double                                              t );

    /** Called on derived classes to implement actual slide change.

        This method is called with the sprite of the slide moving 'out'

        @param rSprite
        Current sprite to operate on. This is the sprite of the
        'leaving' slide

        @param t
        Current parameter value
    */
    virtual void performOut(
        const boost::shared_ptr<cppcanvas::CustomSprite>& rSprite,
        const ViewEntry&                                  rViewEntry,
        const boost::shared_ptr<cppcanvas::Canvas>&       rDestinationCanvas,
        double                                            t );

    ScreenUpdater& getScreenUpdater() const { return mrScreenUpdater; }

private:

    boost::shared_ptr<cppcanvas::CustomSprite> createSprite(
        UnoViewSharedPtr const &   pView,
        ::basegfx::B2DSize const & rSpriteSize,
        double                     nPrio ) const;

    void addSprites( ViewEntry& rEntry );
    void clearViewEntry( ViewEntry& rEntry );

    ViewsVecT::iterator lookupView( UnoViewSharedPtr const & pView );
    ViewsVecT::const_iterator lookupView( UnoViewSharedPtr const & pView ) const;

    SoundPlayerSharedPtr                mpSoundPlayer;

    EventMultiplexer&                   mrEventMultiplexer;
    ScreenUpdater&                      mrScreenUpdater;

    ::boost::optional<SlideSharedPtr>   maLeavingSlide;
    SlideSharedPtr                      mpEnteringSlide;

    ViewsVecT                           maViewData;
    const UnoViewContainer&             mrViewContainer;

    const bool                          mbCreateLeavingSprites;
    const bool                          mbCreateEnteringSprites;
    bool                                mbSpritesVisible;
    bool                                mbFinished;
    bool                                mbPrefetched;
};

} // namespace internal
} // namespace presentation

#endif /* INCLUDED_SLIDESHOW_TRANSITIONS_SLIDECHANGEBASE_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
