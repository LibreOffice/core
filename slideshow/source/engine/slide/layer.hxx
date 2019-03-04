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

#ifndef INCLUDED_SLIDESHOW_SOURCE_ENGINE_SLIDE_LAYER_HXX
#define INCLUDED_SLIDESHOW_SOURCE_ENGINE_SLIDE_LAYER_HXX

#include <basegfx/range/b2dpolyrange.hxx>
#include <cppcanvas/spritecanvas.hxx>

#include <view.hxx>
#include <animatableshape.hxx>

#include <vector>
#include <memory>


namespace slideshow
{
    namespace internal
    {
        class LayerEndUpdate;
        class Layer;
        typedef ::std::shared_ptr< Layer >    LayerSharedPtr;
        typedef ::std::weak_ptr< Layer >      LayerWeakPtr;


        /* Definition of Layer class */

        /** This class represents one layer of output on a Slide.

            Layers group shapes for a certain depth region of a slide.

            Since slides have a notion of depth, i.e. shapes on it
            have a certain order in which they lie upon each other,
            this layering must be modelled. A prime example for this
            necessity are animations of shapes lying behind other
            shapes. Then, everything behind the animated shape will be
            in a background layer, the shape itself will be in an
            animation layer, and everything before it will be in a
            foreground layer (these layers are most preferably
            modelled as XSprite objects internally).

            @attention All methods of this class are only supposed to
            be called from the LayerManager. Normally, it shouldn't be
            possible to get hold of an instance of this class at all.
         */
        class Layer : public std::enable_shared_from_this<Layer>
        {
        public:
            typedef std::shared_ptr<LayerEndUpdate> EndUpdater;

            /// Forbid copy construction
            Layer(const Layer&) = delete;
            /// Forbid copy assignment
            Layer& operator=(const Layer&) = delete;

            /** Create background layer

                This method will create a layer without a ViewLayer,
                i.e. one that displays directly on the background.
             */
            static LayerSharedPtr createBackgroundLayer();

            /** Create non-background layer

                This method will create a layer in front of the
                background, to contain shapes that should appear in
                front of animated objects.
             */
            static LayerSharedPtr createLayer();


            /** Predicate, whether this layer is the special
                background layer

                This method is mostly useful for checking invariants.
             */
            bool isBackgroundLayer() const { return mbBackgroundLayer; }

            /** Add a view to this layer.

                If the view is already added, this method does not add
                it a second time, just returning the existing ViewLayer.

                @param rNewView
                New view to add to this layer.

                @return the newly generated ViewLayer for this View
             */
            ViewLayerSharedPtr addView( const ViewSharedPtr& rNewView );

            /** Remove a view

                This method removes the view from this Layer and all
                shapes included herein.

                @return the ViewLayer of the removed Layer, if
                any. Otherwise, NULL is returned.
             */
            ViewLayerSharedPtr removeView( const ViewSharedPtr& rView );

            /** Init shape with this layer's views

                @param rShape
                The shape, that will subsequently display on this
                layer's views
             */
            void setShapeViews( ShapeSharedPtr const& rShape ) const;


            /** Change layer priority range.

                The layer priority affects the position of the layer
                in the z direction (i.e. before/behind which other
                layers this one appears). The higher the prio, the
                further on top of the layer stack this one appears.

                @param rPrioRange
                The priority range of differing layers must not
                intersect
             */
            void setPriority( const ::basegfx::B1DRange& rPrioRange );

            /** Add an area that needs update

                @param rUpdateRange
                Area on this layer that needs update
             */
            void addUpdateRange( ::basegfx::B2DRange const& rUpdateRange );

            /** Whether any update ranges have been added

                @return true, if any non-empty addUpdateRange() calls
                have been made since the last render()/update() call.
             */
            bool isUpdatePending() const { return maUpdateAreas.count()!=0; }

            /** Update layer bound rect from shape bounds
             */
            void updateBounds( ShapeSharedPtr const& rShape );

            /** Commit collected layer bounds to ViewLayer

                Call this method when you're done adding new shapes to
                the layer.

                @return true, if layer needed a resize (which
                invalidates its content - you have to repaint all
                contained shapes!)
             */
            bool commitBounds();

            /** Clear all registered update ranges

                This method clears all update ranges that are
                registered at this layer.
             */
            void clearUpdateRanges();

            /** Clear whole layer content

                This method clears the whole layer content. As a
                byproduct, all update ranges are cleared as well. It
                makes no sense to maintain them any further, since
                they only serve for partial updates.
             */
            void clearContent();

            /** Init layer update.

                This method initializes a full layer update of the
                update area. When the last copy of the returned
                EndUpdater is destroyed, the Layer leaves update mode
                again.

                @return a update end RAII object.
            */
            EndUpdater beginUpdate();

            /** Finish layer update

                Resets clipping and transformation to normal values
             */
            void endUpdate();

            /** Check whether given shape is inside current update area.

                @return true, if the given shape is at least partially
                inside the current update area.
            */
            bool isInsideUpdateArea( ShapeSharedPtr const& rShape ) const;

        private:
            enum Dummy{ BackgroundLayer };

            /** Create background layer

                This constructor will create a layer without a
                ViewLayer, i.e. one that displays directly on the
                background.

                @param eFlag
                Dummy parameter, to disambiguate from normal layer
                constructor
             */
            explicit Layer( Dummy                    eFlag );

            /** Create non-background layer

                This constructor will create a layer in front of the
                background, to contain shapes that should appear in
                front of animated objects.
             */
            explicit Layer();

            struct ViewEntry
            {
                ViewEntry( const ViewSharedPtr&      rView,
                           const ViewLayerSharedPtr& rViewLayer ) :
                    mpView( rView ),
                    mpViewLayer( rViewLayer )
                {}

                ViewSharedPtr      mpView;
                ViewLayerSharedPtr mpViewLayer;

                // for generic algo access (which needs actual functions)
                const ViewSharedPtr&      getView() const { return mpView; }
                const ViewLayerSharedPtr& getViewLayer() const { return mpViewLayer; }
            };

            typedef ::std::vector< ViewEntry > ViewEntryVector;

            ViewEntryVector            maViewEntries;
            basegfx::B2DPolyRange      maUpdateAreas;
            basegfx::B2DRange          maBounds;
            basegfx::B2DRange          maNewBounds;
            bool                       mbBoundsDirty;     // true, if view layers need resize
            bool const                 mbBackgroundLayer; // true, if this
                                                          // layer is the
                                                          // special
                                                          // background layer
            bool                       mbClipSet; // true, if beginUpdate set a clip
        };

    }
}

#endif // INCLUDED_SLIDESHOW_SOURCE_ENGINE_SLIDE_LAYER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
