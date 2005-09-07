/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: doctreenodesupplier.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 21:09:35 $
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

#ifndef _SLIDESHOW_DOCTREENODESUPPLIER_HXX
#define _SLIDESHOW_DOCTREENODESUPPLIER_HXX

#include <doctreenode.hxx>


/* Definition of DocTreeNodeSupplier interface */

namespace presentation
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
            ImportFailedException. This is, in fact, a delayed error
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

                @throws ImportFailedException, if tree node structure
                cannot be determined.
             */
            virtual sal_Int32 getNumberOfTreeNodes( DocTreeNode::NodeType eNodeType ) const = 0; // throw ImportFailedException;

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

                @throws ImportFailedException, if tree node structure
                cannot be determined.
            */
            virtual DocTreeNode getTreeNode( sal_Int32              nNodeIndex,
                                             DocTreeNode::NodeType  eNodeType ) const = 0; // throw ImportFailedException;

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

                @throws ImportFailedException, if tree node structure
                cannot be determined.
             */
            virtual sal_Int32 getNumberOfSubsetTreeNodes( const DocTreeNode&    rParentNode,
                                                          DocTreeNode::NodeType eNodeType ) const = 0; // throw ImportFailedException;

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

                @throws ImportFailedException, if tree node structure
                cannot be determined.
            */
            virtual DocTreeNode getSubsetTreeNode( const DocTreeNode&       rParentNode,
                                                   sal_Int32                nNodeIndex,
                                                   DocTreeNode::NodeType    eNodeType ) const = 0; // throw ImportFailedException;
        };

    }
}

#endif /* _SLIDESHOW_DOCTREENODESUPPLIER_HXX */
