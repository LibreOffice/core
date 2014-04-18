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

#ifndef INCLUDED_SLIDESHOW_SOURCE_INC_DOCTREENODE_HXX
#define INCLUDED_SLIDESHOW_SOURCE_INC_DOCTREENODE_HXX

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

#endif // INCLUDED_SLIDESHOW_SOURCE_INC_DOCTREENODE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
