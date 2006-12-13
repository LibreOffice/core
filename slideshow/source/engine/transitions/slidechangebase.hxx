/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: slidechangebase.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-13 15:45:39 $
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

#if ! defined(INCLUDED_PRESENTATION_INTERNAL_TRANSITIONS_SLIDECHANGEBASE_HXX)
#define INCLUDED_PRESENTATION_INTERNAL_TRANSITIONS_SLIDECHANGEBASE_HXX

#include <osl/mutex.hxx>

#include "unoview.hxx"
#include "vieweventhandler.hxx"
#include "numberanimation.hxx"
#include "slide.hxx"
#include "soundplayer.hxx"

#include <boost/enable_shared_from_this.hpp>
#include <boost/utility.hpp>
#include <boost/optional.hpp>

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
    virtual void start( const AnimatableShapeSharedPtr&,
                        const ShapeAttributeLayerSharedPtr& );
    virtual void end();

    // ViewEventHandler
    virtual void viewAdded( const UnoViewSharedPtr& rView );
    virtual void viewRemoved( const UnoViewSharedPtr& rView );
    virtual void viewChanged( const UnoViewSharedPtr& rView );

protected:
    /** Create a new SlideChanger, for the given leaving and
        entering slides.
    */
    SlideChangeBase(
        ::boost::optional<SlideSharedPtr> const & leavingSlide,
        const SlideSharedPtr&                     pEnteringSlide,
        const SoundPlayerSharedPtr&               pSoundPlayer,
        const UnoViewContainer&                   rViewContainer,
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
        UnoViewSharedPtr                    mpView;
        /// outgoing slide sprite
        cppcanvas::CustomSpriteSharedPtr    mpOutSprite;
        /// incoming slide sprite
        cppcanvas::CustomSpriteSharedPtr    mpInSprite;
        /// outgoing slide bitmap
        mutable SlideBitmapSharedPtr        mpLeavingBitmap;
        /// incoming slide bitmap
        mutable SlideBitmapSharedPtr        mpEnteringBitmap;

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

    ::basegfx::B2ISize getEnteringSizePixel( const UnoViewSharedPtr& pView ) const;

    void renderBitmap( SlideBitmapSharedPtr const & pSlideBitmap,
                       ::cppcanvas::CanvasSharedPtr const & pCanvas );

    /** Called on derived classes to implement actual slide change.

        This method is called with the sprite of the slide coming 'in'

        @param rSprite
        Current sprite to operate on. This is the sprite of the
        'entering' slide

        @param t
        Current parameter value
    */
    virtual void performIn(
        const ::cppcanvas::CustomSpriteSharedPtr&   rSprite,
        const ViewEntry&                            rViewEntry,
        const ::cppcanvas::CanvasSharedPtr&         rDestinationCanvas,
        double                                      t );

    /** Called on derived classes to implement actual slide change.

        This method is called with the sprite of the slide moving 'out'

        @param rSprite
        Current sprite to operate on. This is the sprite of the
        'leaving' slide

        @param t
        Current parameter value
    */
    virtual void performOut(
        const ::cppcanvas::CustomSpriteSharedPtr&  rSprite,
        const ViewEntry&                           rViewEntry,
        const ::cppcanvas::CanvasSharedPtr&        rDestinationCanvas,
        double                                     t );

private:

    ::cppcanvas::CustomSpriteSharedPtr createSprite(
        UnoViewSharedPtr const &   pView,
        ::basegfx::B2DSize const & rSpriteSize,
        double                     nPrio ) const;

    void addSprites( ViewEntry& rEntry );

    ViewsVecT::iterator lookupView( UnoViewSharedPtr const & pView );
    ViewsVecT::const_iterator lookupView( UnoViewSharedPtr const & pView ) const;

    SoundPlayerSharedPtr                mpSoundPlayer;

    const UnoViewContainer&             mrViewContainer;
    EventMultiplexer&                   mrEventMultiplexer;

    ::boost::optional<SlideSharedPtr>   mLeavingSlide;
    SlideSharedPtr                      mpEnteringSlide;

    ViewsVecT                           maViewData;
    const bool                          mbCreateLeavingSprites;
    const bool                          mbCreateEnteringSprites;
    bool                                mbSpritesVisible;
};

} // namespace internal
} // namespace presentation

#endif
