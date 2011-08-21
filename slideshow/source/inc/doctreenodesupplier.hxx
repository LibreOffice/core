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

#ifndef INCLUDED_SLIDESHOW_DOCTREENODESUPPLIER_HXX
#define INCLUDED_SLIDESHOW_DOCTREENODESUPPLIER_HXX

#include "doctreenode.hxx"


/* Definition of DocTreeNodeSupplier interface */

namespace slideshow
{
    namespace internal
    {
        /** Interface to retrieve DocTreeNodes from subsettable
            shapes.

            Shapes which implement the AttributableShape interface
            also provides this interface, providing methods to
            retrieve specific DocTreeNode objects from the shape. The
            methods mainly distinguish various ways on how to specify
            the actual DocTreeNode to return.

            If a requested DocTreeNode is not available when one of
            the methods below is called, an empty DocTreeNode will be
            returned (the predicate DocTreeNode::isEmpty() will return
            true). If, on the other hand, the shape cannot determine,
            for internal reasons, the internal tree node structure,
            all those methods will throw an
            ShapeLoadFailedException. This is, in fact, a delayed error
            that could also have been reported during shape
            construction, but might be postponed until the missing
            information is actually requested.
         */
        class DocTreeNodeSupplier
        {
        public:
            /** Query number of tree nodes of the given type this
                shape contains.

                The value returned by this method minus one is the
                maximum value permissible at the getTreeNode()
                method, for the given node type.

                @throws ShapeLoadFailedException, if tree node structure
                cannot be determined.
             */
            virtual sal_Int32 getNumberOfTreeNodes( DocTreeNode::NodeType eNodeType ) const = 0; // throw ShapeLoadFailedException;

            /** Create DocTreeNode from shape.

                This method creates a DocTreeNode from a shape, a
                given node type and a running index into the shape's
                DocTreeNodes of the given type.

                @param nNodeIndex
                Starting with 0, every DocTreeNode of the shape that
                has type eNodeType is indexed. The DocTreeNode whose
                index equals nNodeIndex will be returned.

                @param eNodeType
                Type of the node to return

                @return the DocTreeNode found, or the empty
                DocTreeNode, if nothing was found.

                @throws ShapeLoadFailedException, if tree node structure
                cannot be determined.
            */
            virtual DocTreeNode getTreeNode( sal_Int32              nNodeIndex,
                                             DocTreeNode::NodeType  eNodeType ) const = 0; // throw ShapeLoadFailedException;

            /** Query number of tree nodes of the given type this
                subset contains.

                The value returned by this method minus one is the
                maximum value permissible at the
                getSubsetTreeNode() method, for the given node
                type.

                @param rParentNode
                The parent node, below which the number of tree nodes
                of the given type shall be counted.

                @param eNodeType
                Node type to count.

                @throws ShapeLoadFailedException, if tree node structure
                cannot be determined.
             */
            virtual sal_Int32 getNumberOfSubsetTreeNodes( const DocTreeNode&    rParentNode,
                                                          DocTreeNode::NodeType eNodeType ) const = 0; // throw ShapeLoadFailedException;

            /** Create DocTreeNode from shape subset.

                This method creates a DocTreeNode from a shape, a
                parent tree node, a given node type and a running
                index into the shape's DocTreeNodes of the given type.

                @param rParentNode
                Parent node, below which the tree node with the given
                type shall be selected.

                @param nNodeIndex
                Starting with 0, every DocTreeNode of the shape that
                has type eNodeType is indexed. The DocTreeNode whose
                index equals nNodeIndex will be returned.

                @param eNodeType
                Type of the node to return

                @return the DocTreeNode found, or the empty
                DocTreeNode, if nothing was found.

                @throws ShapeLoadFailedException, if tree node structure
                cannot be determined.
            */
            virtual DocTreeNode getSubsetTreeNode( const DocTreeNode&       rParentNode,
                                                   sal_Int32                nNodeIndex,
                                                   DocTreeNode::NodeType    eNodeType ) const = 0; // throw ShapeLoadFailedException;
        };

    }
}

#endif /* INCLUDED_SLIDESHOW_DOCTREENODESUPPLIER_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
