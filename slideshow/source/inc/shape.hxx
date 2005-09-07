/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: shape.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 21:17:37 $
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

#ifndef _SLIDESHOW_SHAPE_HXX
#define _SLIDESHOW_SHAPE_HXX

#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XSHAPE_HPP_
#include <com/sun/star/drawing/XShape.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XDRAWPAGE_HPP_
#include <com/sun/star/drawing/XDrawPage.hpp>
#endif

#ifndef _BGFX_RANGE_B2DRECTANGLE_HXX
#include <basegfx/range/b2drectangle.hxx>
#endif

#ifndef BOOST_SHARED_PTR_HPP_INCLUDED
#include <boost/shared_ptr.hpp>
#endif

#include <viewlayer.hxx>


namespace presentation
{
    namespace internal
    {
        // forward declaration necessary, because methods use ShapeSharedPtr
        class Shape;

        typedef ::boost::shared_ptr< Shape > ShapeSharedPtr;

        /** Represents a slide's shape object.

            This interface represents the view-independent aspects of a
            slide's shape, providing bound rect, underlying XShape and
            basic paint methods.
         */
        class Shape
        {
        public:
            virtual ~Shape() {}

            /** Get the associated XShape of this shape.

                @return the associated XShape. If this method returns
                an empty reference, this object might be one of the
                special-purpose shapes of a slide, which have no
                direct corresponding XShape (the background comes to
                mind here).
             */
            virtual ::com::sun::star::uno::Reference<
                ::com::sun::star::drawing::XShape > getXShape() const = 0;


            // View layer methods
            //------------------------------------------------------------------

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


            // render methods
            //------------------------------------------------------------------

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

            /** Query whether an update is necessary.

                This method returns true, if shape content changed
                since the last rendering (i.e. the shape needs an
                update to reflect that changed content on the views).
             */
            virtual bool isUpdateNecessary() const = 0;


            // Shape attributes
            //------------------------------------------------------------------

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
            virtual ::basegfx::B2DRectangle getPosSize() const = 0;

            /** Get the current shape update area.

                This method yields the currently effective update area
                for the shape, i.e. the area that needs to be updated,
                should the shape be painted. Normally, this will be
                the (possibly rotated and sheared) area returned by
                getPosSize().
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

                @return the priority. Will be in the [0,1] range.
             */
            virtual double getPriority() const = 0;

            /** Query whether the Shape is currently detached from the
                background.

                This method checks whether the Shape is currently
                detached from the slide background, i.e. whether shape
                updates affect the underlying slide background or
                not. A shape that returnes true here must not alter
                slide content in any way when called render() or
                update() (this is normally achieved by making this
                shape a sprite).
             */
            virtual bool isBackgroundDetached() const = 0;

            // TODO(Q1): Maybe the hasIntrinsicAnimation() method is
            // superfluous. Depending on the RTTI implementation, that
            // might do exactly the same.

            /** Query whether shape has intrinsic animation.

                This method must return true, if the shape has an
                intrinsic animation (e.g. drawing layer animation, or
                animated GIF), which needs external update triggers.

                @return true, if this shape is intrinsically animated.

                @see IntrinsicAnimation interface
             */
            virtual bool hasIntrinsicAnimation() const = 0;


            // Misc
            //------------------------------------------------------------------

            /** Functor struct, for shape ordering

                This defines a strict weak ordering of shapes, primary
                sort key is the shape priority, and secondy sort key
                the object ptr value. Most typical use is for
                associative containers holding shapes (and which also
                have to maintain something like a paint order).
             */
            struct lessThanShape
            {
                // since the ZOrder property on the XShape has somewhat
                // peculiar attributes (it's basically the index of the shapes
                // in the drawing layer's SdrObjList - which means, it starts
                // from 0 for children of group objects), we cannot use it to determine
                // drawing order. Thus, we rely on importer-provided order values here,
                // which is basically a running counter during shape import (i.e. denotes
                // the order of shape import). This is the correct order, at least for the
                // current drawing core.
                //
                // If, someday, the above proposition is no longer true, one directly use
                // the shape's ZOrder property
                //
                inline bool operator()(const ShapeSharedPtr& rLHS, const ShapeSharedPtr& rRHS) const
                {
                    const double nPrioL( rLHS->getPriority() );
                    const double nPrioR( rRHS->getPriority() );

                    // if prios are equal, tie-break on ptr value
                    return nPrioL == nPrioR ? rLHS.get() < rRHS.get() : nPrioL < nPrioR;
                }
            };
        };

        typedef ::boost::shared_ptr< Shape > ShapeSharedPtr;

    }
}

#endif /* _SLIDESHOW_SHAPE_HXX */
