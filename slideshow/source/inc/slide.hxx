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

#ifndef INCLUDED_SLIDESHOW_SOURCE_INC_SLIDE_HXX
#define INCLUDED_SLIDESHOW_SOURCE_INC_SLIDE_HXX

#include "shapemanager.hxx"
#include "subsettableshapemanager.hxx"
#include "unoviewcontainer.hxx"
#include "slidebitmap.hxx"
#include "shapemaps.hxx"

#include <memory>

namespace com {  namespace sun { namespace star {
    namespace drawing {
        class XDrawPage;
        class XDrawPagesSupplier;
    }
    namespace uno {
        class XComponentContext;
    }
    namespace animations {
        class XAnimationNode;
} } } }

namespace basegfx
{
    class B2IVector;
}

/* Definition of Slide interface */

namespace slideshow
{
    namespace internal
    {
        class RGBColor;
        class ScreenUpdater;
        typedef ::std::vector< ::cppcanvas::PolyPolygonSharedPtr> PolyPolygonVector;
        class Slide
        {
        public:
            // Showing


            /** Prepares to show slide.

                Call this method to reduce the timeout show(), and
                getInitialSlideBitmap() need to complete. If
                prefetch() is not called explicitly, the named
                methods will call it implicitly.
             */
            virtual void prefetch() = 0;

            /** Shows the slide on all registered views

                After this call, the slide will render itself to the
                views, and start its animations.

                @param bSlideBackgoundPainted
                When true, the initial slide content on the background
                layer is already rendered (e.g. from a previous slide
                transition). When false, Slide renders initial content of
                slide.
             */
            virtual void show( bool bSlideBackgoundPainted ) = 0;

            /** Force-ends the slide

                After this call, the slide has stopped all animations,
                and ceased rendering/visualization on all views.
             */
            virtual void hide() = 0;


            // Queries


            /** Query the size of this slide in user coordinates

                This value is retrieved from the XDrawPage properties.
             */
            virtual basegfx::B2ISize getSlideSize() const = 0;

            /// Gets the underlying API page
            virtual css::uno::Reference< css::drawing::XDrawPage > getXDrawPage() const = 0;

            /// Gets the animation node.
            virtual css::uno::Reference< css::animations::XAnimationNode > getXAnimationNode() const = 0;

            ///Gets the slide Polygons
            virtual PolyPolygonVector getPolygons() = 0;

            ///Draw the slide Polygons
            virtual void drawPolygons() const = 0;

            ///Check if paint overlay is already active
            virtual bool isPaintOverlayActive() const = 0;

            virtual void enablePaintOverlay() = 0;
            virtual void disablePaintOverlay() = 0;

            virtual void update_settings( bool bUserPaintEnabled, RGBColor const& aUserPaintColor, double dUserPaintStrokeWidth ) = 0;

            // Slide bitmaps


            /** Request bitmap for current slide appearance.

                The bitmap returned by this method depends on the
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
                that the view must have been added to this slide
                before via viewAdded().
             */
            virtual SlideBitmapSharedPtr
                getCurrentSlideBitmap( const UnoViewSharedPtr& rView ) const = 0;

        protected:
            ~Slide() {}
        };

        typedef ::std::shared_ptr< Slide > SlideSharedPtr;

        class EventQueue;
        class CursorManager;
        class MediaFileManager;
        class EventMultiplexer;
        class ActivitiesQueue;
        class UserEventQueue;
        class RGBColor;

        /** Construct from XDrawPage

            The Slide object generally works in XDrawPage model
            coordinates, that is, the page will have the width and
            height as specified in the XDrawPage's property
            set. The top, left corner of the page will be rendered
            at (0,0) in the given canvas' view coordinate system.

            Does not render anything initially

            @param xDrawPage
            Page to display on this slide

            @param xRootNode
            Root of the SMIL animation tree. Used to animate the slide.

            @param rEventQueue
            EventQueue. Used to post events.

            @param rActivitiesQueue
            ActivitiesQueue. Used to run animations.

            @param rEventMultiplexer
            Event source

            @param rUserEventQueue
            UserEeventQueue
        */
        SlideSharedPtr createSlide( const css::uno::Reference< css::drawing::XDrawPage >&          xDrawPage,
                                    const css::uno::Reference< css::drawing::XDrawPagesSupplier >& xDrawPages,
                                    const css::uno::Reference< css::animations::XAnimationNode >&  xRootNode,
                                    EventQueue&                                         rEventQueue,
                                    EventMultiplexer&                                   rEventMultiplexer,
                                    ScreenUpdater&                                      rScreenUpdater,
                                    ActivitiesQueue&                                    rActivitiesQueue,
                                    UserEventQueue&                                     rUserEventQueue,
                                    CursorManager&                                      rCursorManager,
                                    MediaFileManager&                                      rMediaFileManager,
                                    const UnoViewContainer&                             rViewContainer,
                                    const css::uno::Reference< css::uno::XComponentContext >&      xContext,
                                    const ShapeEventListenerMap&                        rShapeListenerMap,
                                    const ShapeCursorMap&                               rShapeCursorMap,
                                    const PolyPolygonVector&                            rPolyPolygonVector,
                                    RGBColor const&                                     aUserPaintColor,
                                    double                                              dUserPaintStrokeWidth,
                                    bool                                                bUserPaintEnabled,
                                    bool                                                bIntrinsicAnimationsAllowed,
                                    bool                                                bDisableAnimationZOrder );
    }
}

#endif // INCLUDED_SLIDESHOW_SOURCE_INC_SLIDE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
