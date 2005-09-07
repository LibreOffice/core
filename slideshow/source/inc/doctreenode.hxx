/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: doctreenode.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 21:09:14 $
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

#ifndef _SLIDESHOW_DOCTREENODE_HXX
#define _SLIDESHOW_DOCTREENODE_HXX

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#include <vector>


/* Definition of DocTreeNode class */

namespace presentation
{
    namespace internal
    {

        /** This class represents kind of a DOM tree node for shape
            text

            In order to animate subsets of shape text, we need
            information about the logical and formatting structure of
            that text (lines, paragraphs, words etc.). This is
            represented in a tree structure, with DocTreeNodes as the
            nodes. Instances of this class can be queried from the
            DocTreeNodeSupplier interface.

            This class has nothing to do with the Draw document tree.
         */
        class DocTreeNode
        {
        public:
            /// Type of shape entity represented by this node
            enum NodeType
            {
                NODETYPE_INVALID=0,

                /// This node represents a full shape
                NODETYPE_FORMATTING_SHAPE=1,
                /// This node represents a line
                NODETYPE_FORMATTING_LINE=2,

                /// This node represents a full shape
                NODETYPE_LOGICAL_SHAPE=128,
                /// This node represents a paragraph
                NODETYPE_LOGICAL_PARAGRAPH=129,
                /// This node represents a sentence
                NODETYPE_LOGICAL_SENTENCE=130,
                /// This node represents a word
                NODETYPE_LOGICAL_WORD=131,
                /// This node represents a character
                NODETYPE_LOGICAL_CHARACTER_CELL=132
            };

            // classificators for above text entity types
            static bool isLogicalNodeType( NodeType eType ) { return eType > 127; }
            static bool isFormattingNodeType( NodeType eType ) { return eType > 0 && eType < 128; }

            /** Create empty tree node
             */
            DocTreeNode() :
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
            DocTreeNode( sal_Int32 nStartIndex,
                         sal_Int32 nEndIndex,
                         NodeType  eType ) :
                mnStartIndex(nStartIndex),
                mnEndIndex(nEndIndex),
                meType(eType)
            {
            }

            bool                isEmpty() const { return mnStartIndex == mnEndIndex; }

            sal_Int32           getStartIndex() const { return mnStartIndex; }
            sal_Int32           getEndIndex() const { return mnEndIndex; }
            void                setStartIndex( sal_Int32 nIndex ) { mnStartIndex = nIndex; }
            void                setEndIndex( sal_Int32 nIndex ) { mnEndIndex = nIndex; }

            NodeType            getType() const { return meType; }

        private:
            sal_Int32   mnStartIndex;
            sal_Int32   mnEndIndex;
            NodeType    meType;

        };

        typedef ::std::vector< DocTreeNode > VectorOfDocTreeNodes;
    }
}

#endif /* _SLIDESHOW_DOCTREENODE_HXX */
