/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: shapesubset.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 21:19:03 $
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

#ifndef _SLIDESHOW_SHAPESUBSET_HXX
#define _SLIDESHOW_SHAPESUBSET_HXX

#include <attributableshape.hxx>
#include <layermanager.hxx>
#include <doctreenode.hxx>

#ifndef BOOST_SHARED_PTR_HPP_INCLUDED
#include <boost/shared_ptr.hpp>
#endif

namespace presentation
{
    namespace internal
    {
        class ShapeSubset;
        typedef ::boost::shared_ptr< ShapeSubset > ShapeSubsetSharedPtr;

        /* Definition of ShapeSubset class */

        /** Subset RAII wrapper for shapes.

            This class wraps the plain Shape with a wrapper for subset
            functionality. Subsetting can be turned on and off. Note
            that the reason to have shape subsetting RAII implemented
            separately (i.e. not within the DrawShape) was that
            subsetting (and de-subsetting) needs the LayerManager. And
            holding that at the DrawShape creates one heck of a
            circular reference.
         */
        class ShapeSubset
        {
        public:
            /** Create a subset directly from a Shape.

                @param rOriginalShape
                Original shape to subset

                @param rTreeNode
                Subset this object should represent

                @param rLayerManager
                Manager object, where subsets are
                registered/unregistered
             */
            ShapeSubset( const AttributableShapeSharedPtr&  rOriginalShape,
                         const DocTreeNode&                 rTreeNode,
                         const LayerManagerSharedPtr&       rLayerManager );

            /** Create a subset from another subset.

                Note: if you want this subset to subtract from the
                passed subset reference (and not from the original,
                unsubsetted shape), the passed subset must be enabled
                (enableSubsetShape() must have been called)

                @param rOriginalSubset
                Original subset, which to subset again.

                @param rTreeNode
                Subset of the original subset
             */
            ShapeSubset( const ShapeSubsetSharedPtr&        rOriginalSubset,
                         const DocTreeNode&                 rTreeNode );

            /** Create full set for the given shape.

                @param rOriginalShape
                Original shape, which will be represented as a whole
                by this object
             */
            ShapeSubset( const AttributableShapeSharedPtr&  rOriginalShape,
                         const LayerManagerSharedPtr&       rLayerManager );

            ~ShapeSubset();

            /** Get the actual subset shape.

                If the subset is currently revoked, this method
                returns the original shape.
             */
            AttributableShapeSharedPtr  getSubsetShape() const;

            /** Enable the subset shape.

                This method enables the subset. That means, on
                successful completion of this method, the original
                shape will cease to show the subset range, and
                getSubsetShape() will return a valid shape.

                @return true, if subsetting was successfully enabled.
             */
            bool            enableSubsetShape();

            /** Disable the subset shape.

                This method revokes the subset from the original
                shape. That means, the original shape will again show
                the hidden range.
             */
            void            disableSubsetShape();

            /** Query whether this subset actually is none, but
                contains the whole original shape's content
             */
            bool isFullSet() const;

            /** Query subset this object represents
             */
            DocTreeNode getSubset() const;

        private:
            // default copy/assignment are okay
            //ShapeSubset(const ShapeSubset&);
            //ShapeSubset& operator=( const ShapeSubset& );

            AttributableShapeSharedPtr  mpOriginalShape;
            AttributableShapeSharedPtr  mpSubsetShape;
            DocTreeNode                 maTreeNode;
            LayerManagerSharedPtr       mpLayerManager;
        };
    }
}

#endif /* _SLIDESHOW_SHAPESUBSET_HXX */
