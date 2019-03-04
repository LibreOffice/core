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

#ifndef INCLUDED_SLIDESHOW_SOURCE_ENGINE_SHAPES_DRAWSHAPESUBSETTING_HXX
#define INCLUDED_SLIDESHOW_SOURCE_ENGINE_SHAPES_DRAWSHAPESUBSETTING_HXX

#include <doctreenode.hxx>
#include <attributableshape.hxx>


class GDIMetaFile;
typedef ::std::shared_ptr< GDIMetaFile > GDIMetaFileSharedPtr;

namespace slideshow
{
    namespace internal
    {
        /** This class encapsulates the subsetting aspects of a
            DrawShape.
         */
        class DrawShapeSubsetting
        {
        public:
            /** Create empty shape subset handling.

                This method creates a subset handler which contains no
                subset information. All methods will return default
                values.

                @param rMtf
                Metafile to retrieve subset info from (must have been
                generated with verbose text comments switched on).
             */
            DrawShapeSubsetting();

            /** Create new shape subset handling.

                @param rShapeSubset
                The subset this object represents (can be empty, then
                denoting 'represents a whole shape')

                @param rMtf
                Metafile to retrieve subset info from (must have been
                generated with verbose text comments switched on).
             */
            DrawShapeSubsetting( const DocTreeNode&     rShapeSubset,
                                 GDIMetaFileSharedPtr   rMtf );

            /// Forbid copy construction
            DrawShapeSubsetting(const DrawShapeSubsetting&) = delete;

            /// Forbid copy assignment
            DrawShapeSubsetting& operator=(const DrawShapeSubsetting&) = delete;

            /** Reset metafile.

                Use this method to completely reset the
                ShapeSubsetting, with a new metafile. Note that any
                information previously set will be lost, including
                added subset shapes!

                @param rMtf
                Metafile to retrieve subset info from (must have been
                generated with verbose text comments switched on).
             */
            void reset( const ::std::shared_ptr< GDIMetaFile >&   rMtf );

            // Shape subsetting methods


            /// Return subset node for this shape
            const DocTreeNode&          getSubsetNode       () const;

            /// Get subset shape for given node, if any
            AttributableShapeSharedPtr  getSubsetShape      ( const DocTreeNode& rTreeNode ) const;

            /// Add child subset shape (or increase use count, if already existent)
            void                        addSubsetShape      ( const AttributableShapeSharedPtr& rShape );

            /** Revoke subset shape

                This method revokes a subset shape, decrementing the
                use count for this subset by one. If the use count
                reaches zero (i.e. when the number of addSubsetShape()
                matches the number of revokeSubsetShape() calls for
                the same subset), the subset entry is removed from the
                internal list, and subsequent getSubsetShape() calls
                will return the empty pointer for this subset.

                @return true, if the subset shape was physically
                removed from the list (false is returned, when nothing
                was removed, either because only the use count was
                decremented, or there was no such subset found, in the
                first place).
             */
            bool                        revokeSubsetShape   ( const AttributableShapeSharedPtr& rShape );


            // Doc tree methods


            /// Return overall number of nodes for given type
            sal_Int32   getNumberOfTreeNodes        ( DocTreeNode::NodeType eNodeType ) const;

            /// Return tree node of given index and given type
            DocTreeNode getTreeNode                 ( sal_Int32             nNodeIndex,
                                                      DocTreeNode::NodeType eNodeType ) const;

            /// Return number of nodes of given type, below parent node
            sal_Int32   getNumberOfSubsetTreeNodes  ( const DocTreeNode&    rParentNode,
                                                      DocTreeNode::NodeType eNodeType ) const;

            /// Return tree node of given index and given type, relative to parent node
            DocTreeNode getSubsetTreeNode           ( const DocTreeNode&    rParentNode,
                                                      sal_Int32             nNodeIndex,
                                                      DocTreeNode::NodeType eNodeType ) const;

            // Helper


            /** Return a vector of currently active subsets.

                Needed when rendering a shape, this method provides a
                vector of subsets currently visible (the range as
                returned by getEffectiveSubset(), minus the parts that
                are currently hidden, because displayed by child
                shapes).
             */
            const VectorOfDocTreeNodes& getActiveSubsets() const {  return maCurrentSubsets; }

            /** This enum classifies each action index in the
                metafile.

                Of interest are, of course, the places where
                structural shape and/or text elements end. The
                remainder of the action gets classified as 'noop'
             */
            enum IndexClassificator
            {
                CLASS_NOOP,
                CLASS_SHAPE_START,
                CLASS_SHAPE_END,

                CLASS_LINE_END,
                CLASS_PARAGRAPH_END,
                CLASS_SENTENCE_END,
                CLASS_WORD_END,
                CLASS_CHARACTER_CELL_END
            };

            typedef ::std::vector< IndexClassificator > IndexClassificatorVector;

        private:
            /** Entry for subset shape

                This struct contains data for every subset shape
                generated. Note that for a given start/end action
                index combination, only one subset instance is
                generated (and reused for subsequent queries).
             */
            struct SubsetEntry
            {
                AttributableShapeSharedPtr  mpShape;
                sal_Int32                   mnStartActionIndex;
                sal_Int32                   mnEndActionIndex;

                /// Number of times this subset was queried, and not yet revoked
                int                         mnSubsetQueriedCount;

                sal_Int32 getHashValue() const
                {
                    // TODO(Q3): That's a hack. We assume that start
                    // index will always be less than 65535 (if this
                    // assumption is violated, hash map performance
                    // will degrade severely)
                    return mnStartActionIndex*SAL_MAX_INT16 + mnEndActionIndex;
                }

                /// The shape set is ordered according to this method
                bool operator<(const SubsetEntry& rOther) const
                {
                    return getHashValue() < rOther.getHashValue();
                }

            };

            typedef ::std::set< SubsetEntry >       ShapeSet;

            void ensureInitializedNodeTree() const;
            void excludeSubset(sal_Int32 nExcludedStart, sal_Int32 nExcludedEnd);
            void updateSubsets();
            void initCurrentSubsets();
            void reset();

            static sal_Int32   implGetNumberOfTreeNodes( const IndexClassificatorVector::const_iterator&   rBegin,
                                                  const IndexClassificatorVector::const_iterator&   rEnd,
                                                  DocTreeNode::NodeType                             eNodeType );
            DocTreeNode implGetTreeNode( const IndexClassificatorVector::const_iterator&    rBegin,
                                         const IndexClassificatorVector::const_iterator&    rEnd,
                                         sal_Int32                                          nNodeIndex,
                                         DocTreeNode::NodeType                              eNodeType ) const;

            mutable IndexClassificatorVector    maActionClassVector;

            /// Metafile to retrieve subset info from
            ::std::shared_ptr< GDIMetaFile >  mpMtf;

            /// Subset of the metafile represented by this object
            DocTreeNode                         maSubset;

            /// the list of subset shapes spawned from this one.
            ShapeSet                            maSubsetShapes;

            /** Current number of subsets to render (calculated from
                maSubset and mnMin/MaxSubsetActionIndex).

                Note that this is generally _not_ equivalent to
                maSubset, as it excludes all active subset children!
             */
            mutable VectorOfDocTreeNodes        maCurrentSubsets;

            /// Whether the shape's doc tree has been initialized successfully, or not
            mutable bool                        mbNodeTreeInitialized;
        };

    }
}

#endif // INCLUDED_SLIDESHOW_SOURCE_ENGINE_SHAPES_DRAWSHAPESUBSETTING_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
