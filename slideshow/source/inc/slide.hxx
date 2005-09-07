/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: slide.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 21:19:23 $
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

#ifndef _SLIDESHOW_SLIDE_HXX
#define _SLIDESHOW_SLIDE_HXX

#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HXX_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XDRAWPAGE_HPP_
#include <com/sun/star/drawing/XDrawPage.hpp>
#endif

#ifndef BOOST_SHARED_PTR_HPP_INCLUDED
#include <boost/shared_ptr.hpp>
#endif

#ifndef _CPPCANVAS_SPRITECANVAS_HXX
#include <cppcanvas/spritecanvas.hxx>
#endif

#ifndef _BGFX_VECTOR_B2ISIZE_HXX
#include <basegfx/vector/b2isize.hxx>
#endif

#include <vector>
#include <utility>

#include <unoview.hxx>
#include <event.hxx>
#include <layermanager.hxx>
#include <userpaintoverlay.hxx>
#include <slideanimations.hxx>
#include <slidebitmap.hxx>
#include <shapeeventbroadcaster.hxx>


/* Definition of Slide class */

namespace presentation
{
    namespace internal
    {
        class EventQueue;
        class ActivitiesQueue;
        class UserEventQueue;

        class Slide
        {
        public:
            /** Construct from XDrawPage

                Prefetches shapes from draw page, thus might take some
                time. The Slide object generally works in XDrawPage
                model coordinates, that is, the page will have the
                width and height as specified in the XDrawPage's
                property set. The top, left corner of the page will be
                rendered at (0,0) in the given canvas' view coordinate
                system.

                @param xDrawPage
                XDrawPage to fetch the content from

                @param nSlideIndex
                Index number of this slide
             */
            Slide( const ::com::sun::star::uno::Reference<
                       ::com::sun::star::drawing::XDrawPage >&          xDrawPage,
                   const ::com::sun::star::uno::Reference<
                       ::com::sun::star::animations::XAnimationNode >&  xRootNode,
                   sal_Int32                                            nSlideIndex,
                   EventQueue&                                          rEventQueue,
                   ActivitiesQueue&                                     rActivitiesQueue,
                   EventMultiplexer&                                    rEventMultiplexer,
                   UserEventQueue&                                      rUserEventQueue,
                   const ::com::sun::star::uno::Reference<
                               ::com::sun::star::uno::XComponentContext >& xContext );

            ~Slide();

            /** Add a view to this slide
             */
            void addView( const UnoViewSharedPtr& rView );

            /** Remove a previously added a view from this slide

                @return true, if this view was successfully removed, false
                otherwise (e.g. if this view wasn't added in the first place)
            */
            bool removeView( const UnoViewSharedPtr& rView );

            /** Add the given listener for the given shape.

                This method implements the addShapeEventListener
                method of the XSlideShow interface. The given listener
                is called, whenever a user clicks on the shape. If the
                shape is not contained in this slide, this method does
                nothing.
             */
            void addShapeEventListener( const ::com::sun::star::uno::Reference<
                                                ::com::sun::star::presentation::XShapeEventListener >&  xListener,
                                        const ::com::sun::star::uno::Reference<
                                                ::com::sun::star::drawing::XShape >&                    xShape );

            /** Revoke the given listener for the given shape.

                This method implements the removeShapeEventListener
                method of the XSlideShow interface.
             */
            void removeShapeEventListener( const ::com::sun::star::uno::Reference<
                                                       ::com::sun::star::presentation::XShapeEventListener >&   xListener,
                                           const ::com::sun::star::uno::Reference<
                                                       ::com::sun::star::drawing::XShape >&                     xShape );

            /** Control intrinsic animation behaviour

                @param bImageAnimationsAllowed
                When true, GIF and drawing layer animations will be
                shown. When false, those shapes have no intrinsic
                animation.
             */
            void setImageAnimationsAllowed( bool bImageAnimationsAllowed );

            /** Set the mouse cursor for a given shape.

                This method implements the setShapeCursor method of
                the XSlideShow interface. Whenever the mouse hovers
                over the given shape, the specified mouse cursor will
                be shown. To reset the mouse cursor to the default,
                specify awt::SystemPointer::ARROW.
             */
            void setShapeCursor( const ::com::sun::star::uno::Reference<
                                         ::com::sun::star::drawing::XShape >&   xShape,
                                 sal_Int16                                      nPointerShape );

            /** Prepares to show slide.

                Call this method to reduce the timeout show(), and
                getInitialSlideBitmap() need to complete. If
                prefetchShow() is not called explicitely, the named
                methods will call it implicitely.
             */
            bool prefetchShow();

            /** Paint the slide on all registered views, without any effects
             */
            bool paint();

            /** Shows the slide on all registered views
             */
            bool show();

            /** Force-end the slide show.
             */
            void end();

            /** Query whether this slide is currently showing.

                @return true, if this slide is currently showing
                (i.e. show() was called more often than end()).
             */
            bool isShowing() const;

            /** Query whether the slide has animations at all

                If the slide doesn't have animations, show() is
                equivalent to paint(). If an event is registered with
                registerSlideEndEvent(), this event will be
                immediately activated at the end of the show() method.

                @return true, if this slide has animations, false
                otherwise
             */
            bool isAnimated();

            /** Request bitmap for current slide appearance.

                The bitmap returned by this method is dependent on the
                current state of the slide and the contained
                animations. A newly generated slide will return the
                initial slide content here (e.g. with all 'appear'
                effect shapes invisible), a slide whose effects are
                currently running will return a bitmap corresponding
                to the current position on the animation timeline, and
                a slide whose effects have all been run will generate
                a bitmap with the final slide appearance (e.g. with
                all 'hide' effect shapes invisible).

                @param rView
                View to retrieve bitmap for (note that the bitmap will
                have device-pixel equivalence to the content that
                would have been rendered onto the given view). Note
                that the view must have been added to this slide via
                addView() before, otherwise, this method will throw an
                exception.
             */
            // TODO(F2): Rework SlideBitmap to no longer be based on XBitmap,
            // but on canvas-independent basegfx bitmaps
            SlideBitmapSharedPtr getCurrentSlideBitmap( const UnoViewSharedPtr& rView );

            /** Query the slide, whether next slide should appear
                automatically.

                @return true, if next slide should be displayed
                automatically. When false is returned, the user is
                required to click to have the next slide shown.
             */
            bool hasAutomaticNextSlide() const;

            /** Query the slide for the timeout, until the next slide
                is automatically shown.

                This value is undefined, should
                hasAutomaticNextSlide() return false.

                @return a timeout in seconds, for which to delay the
                display of the next slide.
             */
            double getAutomaticNextSlideTimeout() const;

            /** Control the user paint mode.

                The user paint mode lets the user paint on this slide
                with the given color, when pressing down the
                mousebutton.
             */
            void setUserPaintColor( const ::comphelper::OptionalValue< RGBColor >& rColor );

            /// Query the XDrawPage's size
            basegfx::B2ISize getSlideSize() const;

            /// Get size of the slide in device coordinates for given view
            ::basegfx::B2ISize getSlideSizePixel(
                UnoViewSharedPtr const & pView ) const;

        private:
            // default: disabled copy/assignment
            Slide(const Slide&);
            Slide& operator=( const Slide& );

            void enablePaintOverlay();
            void disablePaintOverlay();

            /// Set all Shapes to their initial attributes for slideshow
            bool applyInitialShapeAttributes( const ::com::sun::star::uno::Reference<
                                                  ::com::sun::star::animations::XAnimationNode >& xRootAnimationNode );

            /// Renders current slide content to bitmap
            SlideBitmapSharedPtr createCurrentSlideBitmap(
                const UnoViewSharedPtr& rView,
                ::basegfx::B2ISize const & rSlideSize );

            /// Prefetch all shapes (not the animations)
            bool prefetchShapes();

            /// Prefetch show, but don't call applyInitialShapeAttributes()
            bool implPrefetchShow();

            /// Query the rectangle covered by the slide
            ::basegfx::B2DRectangle getSlideRect() const;

            /// Start GIF and other intrinsic shape animations
            void endIntrinsicAnimations();

            /// End GIF and other intrinsic shape animations
            void startIntrinsicAnimations();


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

            /** Vector of slide bitmaps.

                Since the bitmap content is sensitive to animation
                effects, we have an inner vector containing a distinct
                bitmap for each of the SlideAnimationStates.
             */
            typedef ::std::vector< ::std::vector< SlideBitmapSharedPtr > > VectorOfSlideBitmaps;


            // Member variables
            // ================

            /// The page model object
            ::com::sun::star::uno::Reference<
                ::com::sun::star::drawing::XDrawPage >          mxDrawPage;
            ::com::sun::star::uno::Reference<
                ::com::sun::star::animations::XAnimationNode >  mxRootNode;

            /// Contains common objects needed throughout the slideshow
            SlideShowContext                                    maContext;

            ShapeEventBroadcasterSharedPtr                      mpEventBroadcaster;

            /// Handles the animation and event generation for us
            SlideAnimations                                     maAnimations;

            /// All added views
            UnoViewVector                                       maViews;

            ::comphelper::OptionalValue< RGBColor >             maUserPaintColor;
            UserPaintOverlaySharedPtr                           mpPaintOverlay;

            /// Bitmap with initial slide content
            VectorOfSlideBitmaps                                maSlideBitmaps;

            /// Timeout for automatic next slide display
            double                                              mnNextSlideTimeout;

            SlideAnimationState                                 meAnimationState;

            /// True, when intrinsic shape animations are allowed
            bool                                                mbImageAnimationsAllowed;

            /// True, if initial load of all page shapes succeeded
            bool                                                mbShapesLoaded;

            /// True, if initial load of all animation info succeeded
            bool                                                mbShowLoaded;

            /** True, if this slide is not static.

                If this slide has animated content, this variable wiil
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

            /** When true, next slide should display without further
                user interaction.
             */
            bool                                                mbHasAutomaticNextSlide;
        };

        typedef ::boost::shared_ptr< ::presentation::internal::Slide > SlideSharedPtr;

    }
}

#endif /* _SLIDESHOW_SLIDE_HXX */
