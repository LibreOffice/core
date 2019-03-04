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

#ifndef INCLUDED_SLIDESHOW_SOURCE_INC_SHAPE_HXX
#define INCLUDED_SLIDESHOW_SOURCE_INC_SHAPE_HXX

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>

#include <basegfx/range/b2drectangle.hxx>

#include "viewlayer.hxx"

#include <memory>
#include <set>
#include <vector>

namespace basegfx {
    class B2DRange;
}

namespace slideshow
{
    namespace internal
    {
        // forward declaration necessary, because methods use ShapeSharedPtr
        class Shape;

        typedef ::std::shared_ptr< Shape > ShapeSharedPtr;

        /** Represents a slide's shape object.

            This interface represents the view-independent aspects of a
            slide's shape, providing bound rect, underlying XShape and
            basic paint methods.
         */
        class Shape
        {
        public:
            Shape() = default;
            virtual ~Shape() {}
            Shape(const Shape&) = delete;
            Shape& operator=(const Shape&) = delete;

            /** Get the associated XShape of this shape.

                @return the associated XShape. If this method returns
                an empty reference, this object might be one of the
                special-purpose shapes of a slide, which have no
                direct corresponding XShape (the background comes to
                mind here).
             */
            virtual css::uno::Reference< css::drawing::XShape > getXShape() const = 0;


            // View layer methods


            /** Add a new view layer.

                This method adds a new view layer, this shape shall
                show itself on.

                @param rNewLayer
                New layer to show on

                @param bRedrawLayer
                Redraw shape on given layer
             */
            virtual void addViewLayer( const ViewLayerSharedPtr&    rNewLayer,
                                       bool                         bRedrawLayer ) = 0;

            /** Withdraw the shape from a view layer

                This method removes the shape from the given view
                layer.

                @return true, if the shape was successfully removed
             */
            virtual bool removeViewLayer( const ViewLayerSharedPtr& rNewLayer ) = 0;

            /** Withdraw all view layers at once

                This method will be faster than repeated
                removeViewLayer() calls.
             */
            virtual void clearAllViewLayers() = 0;

            // render methods


            /** Update the shape

                This method updates the Shape on all registered view
                layers, but only if shape content has actually
                changed.

                @return whether the update finished successfully.
            */
            virtual bool update() const = 0;

            /** Render the shape.

                This method renders the shape on all registered view
                layers, regardless of whether shape content has
                changed or not.

                @return whether the rendering finished successfully.
            */
            virtual bool render() const = 0;

            /** Query whether shape content changed

                This method returns true, if shape content changed
                since the last rendering (i.e. the shape needs an
                update to reflect that changed content on the views).
             */
            virtual bool isContentChanged() const = 0;


            // Shape attributes


            /** Get the current shape position and size.

                This method yields the currently effective shape
                bounds (which might change over time, for animated
                shapes). Please note that possibly shape rotations
                from its original document state must not be taken
                into account here: if you need the screen bounding
                box, use getUpdateArea() instead. Note further that
                shape rotations, which are already contained in the
                shape as displayed in the original document
                <em>are</em> included herein (we currently take the
                shape as-is from the document, assuming a rotation
                angle of 0).
             */
            virtual ::basegfx::B2DRectangle getBounds() const = 0;

            /** Get the DOM position and size of the shape.

                This method yields the underlying DOM shape bounds,
                i.e. the original shape bounds from the document
                model. This value is <em>always</em> unaffected by any
                animation activity. Note that shape rotations, which
                are already contained in the shape as displayed in the
                original document are already included herein (we
                currently take the shape as-is from the document,
                assuming a rotation angle of 0).
             */
            virtual ::basegfx::B2DRectangle getDomBounds() const = 0;

            /** Get the current shape update area.

                This method yields the currently effective update area
                for the shape, i.e. the area that needs to be updated,
                should the shape be painted. Normally, this will be
                the (possibly rotated and sheared) area returned by
                getBounds().
             */
            virtual ::basegfx::B2DRectangle getUpdateArea() const = 0;

            /** Query whether the shape is visible at all.

                @return true, if this shape is visible, false
                otherwise.
             */
            virtual bool isVisible() const = 0;

            /** Get the shape priority.

                The shape priority defines the relative order of the
                shapes on the slide.

                @return the priority. Will be in the [0,+infty) range.
             */
            virtual double getPriority() const = 0;

            /** Query whether the Shape is currently detached from the
                background.

                This method checks whether the Shape is currently
                detached from the slide background, i.e. whether shape
                updates affect the underlying slide background or
                not. A shape that returns true here must not alter
                slide content in any way when called render() or
                update() (this is normally achieved by making this
                shape a sprite).
             */
            virtual bool isBackgroundDetached() const = 0;

            // Misc


            /** Functor struct, for shape ordering

                This defines a strict weak ordering of shapes, primary
                sort key is the shape priority, and secondary sort key
                the object ptr value. Most typical use is for
                associative containers holding shapes (and which also
                have to maintain something like a paint order).
             */
            struct lessThanShape
            {
                // make functor adaptable (to boost::bind)
                typedef bool result_type;

                // since the ZOrder property on the XShape has somewhat
                // peculiar attributes (it's basically the index of the shapes
                // in the drawing layer's SdrObjList - which means, it starts
                // from 0 for children of group objects), we cannot use it to determine
                // drawing order. Thus, we rely on importer-provided order values here,
                // which is basically a running counter during shape import (i.e. denotes
                // the order of shape import). This is the correct order, at least for the
                // current drawing core.

                // If, someday, the above proposition is no longer true, one directly use
                // the shape's ZOrder property

                static bool compare(const Shape* pLHS, const Shape* pRHS)
                {
                    const double nPrioL( pLHS->getPriority() );
                    const double nPrioR( pRHS->getPriority() );

                    // if prios are equal, tie-break on ptr value
                    return nPrioL == nPrioR ? pLHS < pRHS : nPrioL < nPrioR;
                }

                bool operator()(const ShapeSharedPtr& rLHS, const ShapeSharedPtr& rRHS) const
                {
                    return compare(rLHS.get(),rRHS.get());
                }

            };
        };

        /** A set which contains all shapes in an ordered fashion.
         */
        typedef ::std::set< ShapeSharedPtr, Shape::lessThanShape >  ShapeSet;
    }
}

#endif // INCLUDED_SLIDESHOW_SOURCE_INC_SHAPE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
