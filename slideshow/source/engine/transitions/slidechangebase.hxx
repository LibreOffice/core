/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: slidechangebase.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2005-10-11 08:46:17 $
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

#include "unoview.hxx"
#include "slidechangeanimation.hxx"
#include "slide.hxx"
#include "soundplayer.hxx"
#include "rtl/ref.hxx"
#include "osl/mutex.hxx"
#include "cppuhelper/compbase1.hxx"
#include "comphelper/broadcasthelper.hxx"
#include "com/sun/star/util/XModifyListener.hpp"
#include "com/sun/star/presentation/XSlideShowView.hpp"
#include "boost/utility.hpp" // for boost::noncopyable
#include "boost/optional.hpp"
#include "boost/bind.hpp"

namespace presentation {
namespace internal {

typedef ::cppu::WeakComponentImplHelper1<
    ::com::sun::star::util::XModifyListener > UnoBaseT;

/** Base class for all slide change effects.

    This class provides the basic sprite and view handling
    functionality.  Derived classes should normally only need to
    implement the perform() method.
*/
class SlideChangeBase : private ::comphelper::OBaseMutex,
                        public SlideChangeAnimation,
                        public UnoBaseT,
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

    // SlideChangeAnimation
    virtual void addView( UnoViewSharedPtr const & pView );
    virtual bool removeView( UnoViewSharedPtr const & pView );

protected:
    virtual ~SlideChangeBase();

    /** Create a new SlideChanger, for the given leaving and
        entering slides.
    */
    SlideChangeBase(
        ::boost::optional<SlideSharedPtr> const & leavingSlide,
        const SlideSharedPtr& pEnteringSlide,
        const SoundPlayerSharedPtr& pSoundPlayer,
        bool bCreateLeavingSprites = true, bool bCreateEnteringSprites = true );

    SlideBitmapSharedPtr getLeavingBitmap() const;
    SlideBitmapSharedPtr getEnteringBitmap() const;

    SlideBitmapSharedPtr SlideChangeBase::createBitmap(
        ::boost::optional<SlideSharedPtr> const & rSlide_ ) const;

    /// Query the size of the bitmaps in device pixel
    ::basegfx::B2ISize getEnteringSizePixel(
        UnoViewSharedPtr const & pView ) const;
    /// Query the XDrawPage's size
    ::basegfx::B2DSize getEnteringSize() const;

    void renderBitmap( SlideBitmapSharedPtr const & pSlideBitmap,
                       ::cppcanvas::CanvasSharedPtr const & pCanvas );

    /** Loop over each View, and call func with that
     */
    template <typename FuncT>
    void for_each_view( FuncT const& func ) const {
        ::std::for_each( maViews.begin(), maViews.end(), func );
    }

    /** Loop over each View's canvas, and call func with that
     */
    template <typename FuncT>
    void for_each_canvas( FuncT const& func ) const {
        UnoViewVector::const_iterator const iEnd( maViews.end() );
        for ( UnoViewVector::const_iterator iPos( maViews.begin() );
              iPos != iEnd; ++iPos )
        {
            ::cppcanvas::CanvasSharedPtr const pCanvas( (*iPos)->getCanvas() );
            func( pCanvas );
        }
    }

    /** Called on derived classes to implement actual slide change.

        This method is called with the sprite of the slide coming 'in'

        @param rSprite
        Current sprite to operate on. This is the sprite of the
        'entering' slide

        @param x
        Current parameter value
    */
    virtual void performIn(
        const ::cppcanvas::CustomSpriteSharedPtr&   rSprite,
        UnoViewSharedPtr const &                    pView,
        const ::cppcanvas::CanvasSharedPtr&         rDestinationCanvas,
        double                                      t );

    /** Called on derived classes to implement actual slide change.

        This method is called with the sprite of the slide moving 'out'

        @param rSprite
        Current sprite to operate on. This is the sprite of the
        'leaving' slide

        @param x
        Current parameter value
    */
    virtual void performOut(
        const ::cppcanvas::CustomSpriteSharedPtr&  rSprite,
        UnoViewSharedPtr const &                   pView,
        const ::cppcanvas::CanvasSharedPtr&        rDestinationCanvas,
        double                                     t );

private:
    // XModifyListener
    virtual void SAL_CALL modified(
        ::com::sun::star::lang::EventObject const& evt )
        throw (::com::sun::star::uno::RuntimeException);
    // XEventListener
    virtual void SAL_CALL disposing(
        ::com::sun::star::lang::EventObject const& evt )
        throw (::com::sun::star::uno::RuntimeException);

    /// WeakComponentImplHelperBase:
    virtual void SAL_CALL disposing();

private:

    // view mangement:
    UnoViewVector maViews;
    UnoViewSharedPtr findUnoView(
        ::com::sun::star::uno::Reference<
        ::com::sun::star::presentation::XSlideShowView> const & xSlideShowView )
        const;
    bool removeView_( UnoViewSharedPtr const& pView,
                      bool bDisposedView = false );
    void removeTransformationChangedListenerFrom(UnoViewSharedPtr const& pView);

    SoundPlayerSharedPtr mpSoundPlayer;

    ::boost::optional<SlideSharedPtr> mLeavingSlide;
    SlideSharedPtr mpEnteringSlide;
    mutable SlideBitmapSharedPtr mpLeavingBitmap;
    mutable SlideBitmapSharedPtr mpEnteringBitmap;

    typedef ::std::vector< ::cppcanvas::CustomSpriteSharedPtr > SpriteVector;
    SpriteVector maOutSprites;
    SpriteVector maInSprites;
    const bool mbCreateLeavingSprites;
    const bool mbCreateEnteringSprites;
    bool mbSpritesVisible;

    ::cppcanvas::CustomSpriteSharedPtr createSprite(
        UnoViewSharedPtr const & pView,
        ::basegfx::B2DSize const & rSpriteSize ) const;
    void addSprites( UnoViewSharedPtr const & pView );
};

} // namespace internal
} // namespace presentation

#endif
