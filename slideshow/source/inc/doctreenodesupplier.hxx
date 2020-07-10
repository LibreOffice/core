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

#ifndef INCLUDED_SLIDESHOW_SOURCE_INC_DOCTREENODESUPPLIER_HXX
#define INCLUDED_SLIDESHOW_SOURCE_INC_DOCTREENODESUPPLIER_HXX

#include "doctreenode.hxx"


/* Definition of DocTreeNodeSupplier interface */

namespace slideshow::internal
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

        protected:
            ~DocTreeNodeSupplier() {}
        };

}

#endif // INCLUDED_SLIDESHOW_SOURCE_INC_DOCTREENODESUPPLIER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
