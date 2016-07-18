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

#ifndef INCLUDED_SLIDESHOW_SOURCE_INC_SHAPESUBSET_HXX
#define INCLUDED_SLIDESHOW_SOURCE_INC_SHAPESUBSET_HXX

#include "attributableshape.hxx"
#include "subsettableshapemanager.hxx"
#include "doctreenode.hxx"

#include <memory>

namespace slideshow
{
    namespace internal
    {
        class ShapeSubset;
        typedef ::std::shared_ptr< ShapeSubset > ShapeSubsetSharedPtr;

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
            AttributableShapeSharedPtr const &  getSubsetShape() const;

            /** Enable the subset shape.

                This method enables the subset. That means, on
                successful completion of this method, the original
                shape will cease to show the subset range, and
                getSubsetShape() will return a valid shape.
             */
            void            enableSubsetShape();

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
            const DocTreeNode& getSubset() const;

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

#endif // INCLUDED_SLIDESHOW_SOURCE_INC_SHAPESUBSET_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
