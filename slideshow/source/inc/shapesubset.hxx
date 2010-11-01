/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef INCLUDED_SLIDESHOW_SHAPESUBSET_HXX
#define INCLUDED_SLIDESHOW_SHAPESUBSET_HXX

#include "attributableshape.hxx"
#include "subsettableshapemanager.hxx"
#include "doctreenode.hxx"

#include <boost/shared_ptr.hpp>

namespace slideshow
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
            subsetting (and de-subsetting) needs the
            SubsettableShapeManager. And holding that at the DrawShape
            creates one heck of a circular reference.
         */
        class ShapeSubset
        {
        public:
            /** Create a subset directly from a Shape.

                @param rOriginalShape
                Original shape to subset

                @param rTreeNode
                Subset this object should represent

                @param rShapeManager
                Manager object, where subsets are
                registered/unregistered
             */
            ShapeSubset( const AttributableShapeSharedPtr&       rOriginalShape,
                         const DocTreeNode&                      rTreeNode,
                         const SubsettableShapeManagerSharedPtr& rSubsetManager );

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
            ShapeSubset( const AttributableShapeSharedPtr&       rOriginalShape,
                         const SubsettableShapeManagerSharedPtr& rShapeManager );

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

            AttributableShapeSharedPtr       mpOriginalShape;
            AttributableShapeSharedPtr       mpSubsetShape;
            DocTreeNode                      maTreeNode;
            SubsettableShapeManagerSharedPtr mpShapeManager;
        };
    }
}

#endif /* INCLUDED_SLIDESHOW_SHAPESUBSET_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
