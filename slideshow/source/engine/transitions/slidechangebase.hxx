/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: slidechangebase.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 20:58:22 $
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
#include "cppuhelper/implbase1.hxx"
#include "com/sun/star/util/XModifyListener.hpp"
#include "com/sun/star/presentation/XSlideShowView.hpp"
#include "boost/utility.hpp" // for boost::noncopyable
#include "boost/optional.hpp"
#include "boost/bind.hpp"

namespace presentation {
namespace internal {

/** Base class for all slide change effects.

    This class provides the basic sprite and view handling
    functionality.  Derived classes should normally only need to
    implement the perform() method.
*/
class SlideChangeBase : public SlideChangeAnimation,
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

    void renderBitmap(
        SlideBitmapSharedPtr const & pSlideBitmap,
        ::cppcanvas::CanvasSharedPtr const & pCanvas );

    /** Loop over each View, and call func with that
     */
    template <typename FuncT>
    FuncT for_each_view( FuncT func ) {
        return ::std::for_each( maViews.begin(), maViews.end(), func );
    }

    /** Loop over each View's canvas, and call func with that
     */
    template <typename FuncT>
    FuncT for_each_canvas( FuncT func ) {
        // at least gcc 3.4.1 cannot cope with the below:
//         ::std::for_each(
//             maViews.begin(), maViews.end(),
//             ::boost::bind( ::boost::ref(func),
//                            ::boost::bind( &ViewLayer::getCanvas, _1 ) ) );
        const UnoViewVector::const_iterator iEnd( maViews.end() );
        for ( UnoViewVector::const_iterator iPos( maViews.begin() );
              iPos != iEnd; ++iPos )
        {
            const ::cppcanvas::CanvasSharedPtr pCanvas( (*iPos)->getCanvas() );
            func( pCanvas );
        }
        return func;
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
    mutable ::osl::Mutex maMutex;

    UnoViewVector maViews;
    UnoViewSharedPtr findUnoView(
        ::com::sun::star::uno::Reference<
        ::com::sun::star::presentation::XSlideShowView> const & xSlideShowView )
        const;

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

    void notifyViewChange( ::com::sun::star::lang::EventObject const & evt );

    class ModifyListener
        : public ::cppu::WeakImplHelper1<
              ::com::sun::star::util::XModifyListener >,
          private ::boost::noncopyable
    {
    public:
        // XModifyListener
        virtual void SAL_CALL modified(
            ::com::sun::star::lang::EventObject const & evt )
            throw (::com::sun::star::uno::RuntimeException);
        // XEventListener
        virtual void SAL_CALL disposing(
            ::com::sun::star::lang::EventObject const & evt )
            throw (::com::sun::star::uno::RuntimeException);

        ModifyListener( SlideChangeBase * pSlideChangeBase )
            : m_pSlideChangeBase(pSlideChangeBase)
            {}

    private:
        // TODO: think about boost::shared_ptr<> when
        // enable_shared_ptr_from_this is available
        SlideChangeBase * m_pSlideChangeBase;
        friend class SlideChangeBase;
    };

    ::rtl::Reference<ModifyListener> mxModifyListener;
    ::rtl::Reference<ModifyListener> const & getModifyListener() {
        if (! mxModifyListener.is()) // late init:
            mxModifyListener = new ModifyListener(this);
        return mxModifyListener;
    }
    friend class ModifyListener;
};

} // namespace internal
} // namespace presentation

#endif
