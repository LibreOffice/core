/*************************************************************************
 *
 *  $RCSfile: doctreenode.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-11-26 19:14:50 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _SLIDESHOW_DOCTREENODE_HXX
#define _SLIDESHOW_DOCTREENODE_HXX

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#include <attributableshape.hxx>

#include <vector>

/* Definition of DocTreeNode class */

namespace presentation
{
    namespace internal
    {

        /** This class represents kind of a DOM tree for shape text

            In order to animate subsets of shape text, we need
            information about the logical and formatting structure of
            that text (lines, paragraphs, words etc.). This is
            represented in a tree structure, with DocTreeNodes as the
            nodes.

            This class has nothing to do with the Draw document tree.
         */
        class DocTreeNode
        {
        public:
            typedef ::std::vector< DocTreeNode > VectorOfDocTreeNodes;

            /// Type of text entity represented by this node
            enum NodeType
            {
                NODETYPE_INVALID=0,

                /// This node contains a full shape
                NODETYPE_FORMATTING_SHAPE=1,
                /// This node contains a line
                NODETYPE_FORMATTING_LINE=2,

                /// This node contains a full shape
                NODETYPE_LOGICAL_SHAPE=128,
                /// This node contains a paragraph
                NODETYPE_LOGICAL_PARAGRAPH=129,
                /// This node contains a sentence
                NODETYPE_LOGICAL_SENTENCE=130,
                /// This node contains a word
                NODETYPE_LOGICAL_WORD=131,
                /// This node contains a character
                NODETYPE_LOGICAL_CHARACTER_CELL=132
            };

            // classificators for above text entity types
            static bool isLogicalNodeType( NodeType eType ) { return eType > 127; }
            static bool isFormattingNodeType( NodeType eType ) { return eType > 0 && eType < 128; }

            /** Create DocTreeNode from shape subset.

                This method creates a DocTreeNode from a shape, a
                given node type an a running index into the shape's
                DocTreeNodes of the given type.

                @param rShape
                Shape to extract the DocTreeNode from

                @param nNodeIndex
                Starting with 0, every DocTreeNode of the shape that
                has type eNodeType is indexed. The DocTreeNode whose
                index equals nNodeIndex will be returned.

                @param eNodeType
                Type of the node to return

                @return the DocTreeNode found, or the empty
                DocTreeNode, if nothing was found.
             */
            static DocTreeNode createFromShape( const AttributableShapeSharedPtr& rShape,
                                                sal_Int16                         nNodeIndex,
                                                DocTreeNode::NodeType             eNodeType );

            /** Create empty tree node
             */
            DocTreeNode() :
                maChildren(),
                mnStartIndex(-1),
                mnEndIndex(-1),
                meType(NODETYPE_INVALID)
            {
            }

            /** Create tree node from start and end index.

                Create a tree node for the given range and type.

                @param nStartIndex
                Start index

                @param nEndIndex
                End index (exclusive)

                @param eType
                Node type
             */
            DocTreeNode( sal_Int32 nStartIndex, sal_Int32 nEndIndex, NodeType eType ) :
                maChildren(),
                mnStartIndex(nStartIndex),
                mnEndIndex(nEndIndex),
                meType(eType)
            {
            }

            bool                isEmpty() const;

            sal_Int32           getStartIndex() const { return mnStartIndex; }
            sal_Int32           getEndIndex() const { return mnEndIndex; }
            void                setStartIndex( sal_Int32 nIndex ) { mnStartIndex = nIndex; }
            void                setEndIndex( sal_Int32 nIndex ) { mnEndIndex = nIndex; }

            NodeType            getType() const;

            sal_Int32           getNumberOfChildren() const;
            const DocTreeNode&  getChild( sal_Int32 nIndex ) const;
            void                setChild( const DocTreeNode& rChild, sal_Int32 nIndex );
            void                appendChild( const DocTreeNode& rChild );

        private:

            // TODO(P2): convert to shared_ptr here

            // TODO(P3): create children lazily
            VectorOfDocTreeNodes    maChildren;
            sal_Int32               mnStartIndex;
            sal_Int32               mnEndIndex;
            NodeType                meType;

        };
    }
}

#endif /* _SLIDESHOW_DOCTREENODE_HXX */
