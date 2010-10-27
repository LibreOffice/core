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

#ifndef INCLUDED_SLIDESHOW_DOCTREENODE_HXX
#define INCLUDED_SLIDESHOW_DOCTREENODE_HXX

#include <sal/types.h>
#include <vector>


/* Definition of DocTreeNode class */

namespace slideshow
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

            void                reset()
            {
                mnStartIndex = -1;
                mnEndIndex   = -1;
                meType = NODETYPE_INVALID;
            }

        private:
            sal_Int32   mnStartIndex;
            sal_Int32   mnEndIndex;
            NodeType    meType;

        };

        typedef ::std::vector< DocTreeNode > VectorOfDocTreeNodes;
    }
}

#endif /* INCLUDED_SLIDESHOW_DOCTREENODE_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
