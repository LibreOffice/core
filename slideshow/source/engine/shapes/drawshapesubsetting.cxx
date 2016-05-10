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


#include <tools/diagnose_ex.h>

#include <rtl/math.hxx>

#include <vcl/metaact.hxx>
#include <vcl/gdimtf.hxx>
#include <basegfx/numeric/ftools.hxx>

#include "drawshapesubsetting.hxx"
#include "drawshape.hxx"

#include <algorithm>
#include <functional>
#include <limits>

using namespace ::com::sun::star;


namespace slideshow
{
    namespace internal
    {


        // Private methods


        void DrawShapeSubsetting::ensureInitializedNodeTree() const
        {
            ENSURE_OR_THROW( mpMtf,
                              "DrawShapeSubsetting::ensureInitializedNodeTree(): Invalid mtf" );

            if( mbNodeTreeInitialized )
                return; // done, already initialized.

            // init doctree vector
            maActionClassVector.clear();
            maActionClassVector.reserve( mpMtf->GetActionSize() );

            // search metafile for text output
            MetaAction* pCurrAct;

            sal_Int32 nActionIndex(0);
            sal_Int32 nLastTextActionIndex(0);
            for( pCurrAct = mpMtf->FirstAction(); pCurrAct; pCurrAct = mpMtf->NextAction() )
            {
                // check for one of our special text doctree comments
                switch( pCurrAct->GetType() )
                {
                    case MetaActionType::COMMENT:
                    {
                        MetaCommentAction* pAct = static_cast<MetaCommentAction*>(pCurrAct);

                        // skip comment if not a special XTEXT... comment
                        if( pAct->GetComment().matchIgnoreAsciiCase( OString("XTEXT") ) )
                        {
                            // fill classification vector with NOOPs,
                            // then insert corresponding classes at
                            // the given index
                            maActionClassVector.resize( nActionIndex+1, CLASS_NOOP );

                            if( pAct->GetComment().equalsIgnoreAsciiCase("XTEXT_EOC") )
                            {
                                // special, because can happen
                                // in-between of portions - set
                                // character-end classificator at
                                // given index (relative to last text
                                // action).
                                const sal_Int32 nIndex( nLastTextActionIndex + pAct->GetValue() );

                                ENSURE_OR_THROW( static_cast< ::std::size_t >(nIndex) < maActionClassVector.size(),
                                                  "DrawShapeSubsetting::ensureInitializedNodeTree(): sentence index out of range" );

                                maActionClassVector[ nIndex ] = CLASS_CHARACTER_CELL_END;
                            }
                            else if( pAct->GetComment().equalsIgnoreAsciiCase("XTEXT_EOW") )
                            {
                                // special, because can happen
                                // in-between of portions - set
                                // word-end classificator at given
                                // index (relative to last text
                                // action).
                                const sal_Int32 nIndex( nLastTextActionIndex + pAct->GetValue() );

                                ENSURE_OR_THROW( static_cast< ::std::size_t >(nIndex) < maActionClassVector.size(),
                                                  "DrawShapeSubsetting::ensureInitializedNodeTree(): sentence index out of range" );

                                maActionClassVector[ nIndex ] = CLASS_WORD_END;
                            }
                            else if( pAct->GetComment().equalsIgnoreAsciiCase("XTEXT_EOS") )
                            {
                                // special, because can happen
                                // in-between of portions - set
                                // sentence-end classificator at given
                                // index (relative to last text
                                // action).
                                const sal_Int32 nIndex( nLastTextActionIndex + pAct->GetValue() );

                                ENSURE_OR_THROW( static_cast< ::std::size_t >(nIndex) < maActionClassVector.size(),
                                                  "DrawShapeSubsetting::ensureInitializedNodeTree(): sentence index out of range" );

                                maActionClassVector[ nIndex ] = CLASS_SENTENCE_END;
                            }
                            else if( pAct->GetComment().equalsIgnoreAsciiCase("XTEXT_EOL") )
                            {
                                maActionClassVector[ nActionIndex ] = CLASS_LINE_END;
                            }
                            else if( pAct->GetComment().equalsIgnoreAsciiCase("XTEXT_EOP") )
                            {
                                maActionClassVector[ nActionIndex ] = CLASS_PARAGRAPH_END;
                            }
                            else if( pAct->GetComment().equalsIgnoreAsciiCase("XTEXT_PAINTSHAPE_END") )
                            {
                                maActionClassVector[ nActionIndex ] = CLASS_SHAPE_END;
                            }
                            else if( pAct->GetComment().equalsIgnoreAsciiCase("XTEXT_PAINTSHAPE_BEGIN") )
                            {
                                maActionClassVector[ nActionIndex ] = CLASS_SHAPE_START;
                            }
                        }
                        SAL_INFO(
                            "slideshow",
                            "Shape text structure: " << pAct->GetComment()
                                << " at action #" << nActionIndex);
                        ++nActionIndex;
                        break;
                    }
                    case MetaActionType::TEXT:
                    case MetaActionType::TEXTARRAY:
                    case MetaActionType::STRETCHTEXT:
                        nLastTextActionIndex = nActionIndex;
                        SAL_INFO("slideshow.verbose", "Shape text \"" <<
                                 (static_cast<MetaTextAction*>(pCurrAct))->GetText() <<
                                 "\" at action #" << nActionIndex );
                        SAL_FALLTHROUGH;
                    default:
                        // comment action and all actions not
                        // explicitly handled here:
                        nActionIndex += getNextActionOffset(pCurrAct);
                        break;
                }
            }

            mbNodeTreeInitialized = true;
        }

        void DrawShapeSubsetting::updateSubsetBounds( const SubsetEntry& rSubsetEntry )
        {
            // TODO(F1): This removes too much from non-contiguous subsets
            mnMinSubsetActionIndex = ::std::min(
                mnMinSubsetActionIndex,
                rSubsetEntry.mnStartActionIndex );
            mnMaxSubsetActionIndex = ::std::max(
                mnMaxSubsetActionIndex,
                rSubsetEntry.mnEndActionIndex );
        }

        void DrawShapeSubsetting::updateSubsets()
        {
            maCurrentSubsets.clear();

            if( !maSubsetShapes.empty() )
            {
                if( maSubset.isEmpty() )
                {
                    // non-subsetted node, with some child subsets
                    // that subtract from it
                    maCurrentSubsets.push_back( DocTreeNode( 0,
                                                             mnMinSubsetActionIndex,
                                                             DocTreeNode::NODETYPE_INVALID ) );
                    maCurrentSubsets.push_back( DocTreeNode( mnMaxSubsetActionIndex,
                                                             maActionClassVector.size(),
                                                             DocTreeNode::NODETYPE_INVALID ) );
                }
                else
                {
                    // subsetted node, from which some further child
                    // subsets subtract content
                    maCurrentSubsets.push_back( DocTreeNode( maSubset.getStartIndex(),
                                                             mnMinSubsetActionIndex,
                                                             DocTreeNode::NODETYPE_INVALID ) );
                    maCurrentSubsets.push_back( DocTreeNode( mnMaxSubsetActionIndex,
                                                             maSubset.getEndIndex(),
                                                             DocTreeNode::NODETYPE_INVALID ) );
                }
            }
            else
            {
                // no further child subsets, simply add our subset (if any)
                if( !maSubset.isEmpty() )
                {
                    // subsetted node, without any subset children
                    maCurrentSubsets.push_back( maSubset );
                }
            }
        }


        // Public methods


        DrawShapeSubsetting::DrawShapeSubsetting() :
            maActionClassVector(),
            mpMtf(),
            maSubset(),
            maSubsetShapes(),
            mnMinSubsetActionIndex( SAL_MAX_INT32 ),
            mnMaxSubsetActionIndex(0),
            maCurrentSubsets(),
            mbNodeTreeInitialized( false )
        {
        }

        DrawShapeSubsetting::DrawShapeSubsetting( const DocTreeNode&            rShapeSubset,
                                                  const GDIMetaFileSharedPtr&   rMtf ) :
            maActionClassVector(),
            mpMtf( rMtf ),
            maSubset( rShapeSubset ),
            maSubsetShapes(),
            mnMinSubsetActionIndex( SAL_MAX_INT32 ),
            mnMaxSubsetActionIndex(0),
            maCurrentSubsets(),
            mbNodeTreeInitialized( false )
        {
            ENSURE_OR_THROW( mpMtf,
                              "DrawShapeSubsetting::DrawShapeSubsetting(): Invalid metafile" );

            initCurrentSubsets();
        }

        void DrawShapeSubsetting::reset()
        {
            maActionClassVector.clear();
            mpMtf.reset();
            maSubset.reset();
            maSubsetShapes.clear();
            mnMinSubsetActionIndex = SAL_MAX_INT32;
            mnMaxSubsetActionIndex = 0;
            maCurrentSubsets.clear();
            mbNodeTreeInitialized = false;
        }

        void DrawShapeSubsetting::reset( const ::boost::shared_ptr< GDIMetaFile >& rMtf )
        {
            reset();
            mpMtf = rMtf;

            initCurrentSubsets();
        }

        void DrawShapeSubsetting::initCurrentSubsets()
        {
            // only add subset to vector, if it's not empty - that's
            // because the vector's content is later literally used
            // for e.g. painting.
            if( !maSubset.isEmpty() )
                maCurrentSubsets.push_back( maSubset );
        }

        DocTreeNode DrawShapeSubsetting::getSubsetNode() const
        {
            return maSubset;
        }

        AttributableShapeSharedPtr DrawShapeSubsetting::getSubsetShape( const DocTreeNode& rTreeNode ) const
        {
            SAL_INFO( "slideshow", "::presentation::internal::DrawShapeSubsetting::getSubsetShape()" );

            // subset shape already created for this DocTreeNode?
            SubsetEntry aEntry;

            aEntry.mnStartActionIndex   = rTreeNode.getStartIndex();
            aEntry.mnEndActionIndex     = rTreeNode.getEndIndex();

            ShapeSet::const_iterator aIter;
            if( (aIter=maSubsetShapes.find( aEntry )) != maSubsetShapes.end() )
            {
                // already created, return found entry
                return aIter->mpShape;
            }

            return AttributableShapeSharedPtr();
        }

        void DrawShapeSubsetting::addSubsetShape( const AttributableShapeSharedPtr& rShape )
        {
            SAL_INFO( "slideshow", "::presentation::internal::DrawShapeSubsetting::addSubsetShape()" );

            // subset shape already created for this DocTreeNode?
            SubsetEntry aEntry;
            const DocTreeNode& rEffectiveSubset( rShape->getSubsetNode() );

            aEntry.mnStartActionIndex   = rEffectiveSubset.getStartIndex();
            aEntry.mnEndActionIndex     = rEffectiveSubset.getEndIndex();

            ShapeSet::const_iterator aIter;
            if( (aIter=maSubsetShapes.find( aEntry )) != maSubsetShapes.end() )
            {
                // already created, increment use count and return

                // safe cast, since set order does not depend on
                // mnSubsetQueriedCount
                const_cast<SubsetEntry&>(*aIter).mnSubsetQueriedCount++;
            }
            else
            {
                // not yet created, init entry
                aEntry.mnSubsetQueriedCount = 1;
                aEntry.mpShape = rShape;

                maSubsetShapes.insert( aEntry );

                // update cached subset borders
                updateSubsetBounds( aEntry );
                updateSubsets();
            }
        }

        bool DrawShapeSubsetting::revokeSubsetShape( const AttributableShapeSharedPtr& rShape )
        {
            SAL_INFO( "slideshow", "::presentation::internal::DrawShapeSubsetting::revokeSubsetShape()" );

            // lookup subset shape
            SubsetEntry aEntry;
            const DocTreeNode& rEffectiveSubset( rShape->getSubsetNode() );

            aEntry.mnStartActionIndex   = rEffectiveSubset.getStartIndex();
            aEntry.mnEndActionIndex     = rEffectiveSubset.getEndIndex();

            ShapeSet::iterator aIter;
            if( (aIter=maSubsetShapes.find( aEntry )) == maSubsetShapes.end() )
                return false; // not found, subset was never queried

            // last client of the subset revoking?
            if( aIter->mnSubsetQueriedCount > 1 )
            {
                // no, still clients out there. Just decrement use count
                // safe cast, since order does not depend on mnSubsetQueriedCount
                const_cast<SubsetEntry&>(*aIter).mnSubsetQueriedCount--;

                SAL_INFO(
                    "slideshow",
                    "Subset summary: shape " << this << ", "
                        << maSubsetShapes.size()
                        << " open subsets, revoked subset has refcount "
                        << aIter->mnSubsetQueriedCount);

                return false; // not the last client
            }

            SAL_INFO(
                "slideshow",
                "Subset summary: shape " << this << ", "
                    << maSubsetShapes.size()
                    << " open subsets, cleared subset has range ["
                    << aEntry.mnStartActionIndex << ","
                    << aEntry.mnEndActionIndex << "]");

            // yes, remove from set
            maSubsetShapes.erase( aIter );


            // update currently active subset for _our_ shape (the
            // part of this shape that is visible, i.e. not displayed
            // in subset shapes)


            // init bounds
            mnMinSubsetActionIndex = SAL_MAX_INT32;
            mnMaxSubsetActionIndex = 0;

            // TODO(P2): This is quite expensive, when
            // after every subset effect end, we have to scan
            // the whole shape set

            // determine new subset range
            for( const auto& rSubsetShape : maSubsetShapes )
                updateSubsetBounds( rSubsetShape );

            updateSubsets();

            return true;
        }

        namespace
        {
            /** Iterate over all action classification entries in the
                given range, pass each element range found to the
                given functor.

                This method extracts, for each of the different action
                classifications, the count and the ranges for each of
                them, and calls the provided functor with that
                information.

                @tpl FunctorT
                This is the functor's operator() calling signature,
                with eCurrElemClassification denoting the current
                classification type the functor is called for,
                nCurrElemCount the running total of elements visited
                for the given class (starting from 0), and
                rCurrElemBegin/rCurrElemEnd the range of the current
                element (i.e. the iterators from the start to the end
                of this element).
                <pre>
                bool operator()( IndexClassificator                              eCurrElemClassification
                                 sal_Int32                                       nCurrElemCount,
                                 const IndexClassificatorVector::const_iterator& rCurrElemBegin,
                                 const IndexClassificatorVector::const_iterator& rCurrElemEnd );
                </pre>
                If the functor returns false, iteration over the
                shapes is immediately stopped.

                @param io_pFunctor
                This functor is called for every shape found.

                @param rBegin
                Start of range to iterate over

                @param rEnd
                End of range to iterate over

                @return the number of shapes found in the metafile
             */
            template< typename FunctorT > void iterateActionClassifications(
                FunctorT&                                                            io_rFunctor,
                const DrawShapeSubsetting::IndexClassificatorVector::const_iterator& rBegin,
                const DrawShapeSubsetting::IndexClassificatorVector::const_iterator& rEnd )
            {
                sal_Int32 nCurrShapeCount( 0 );
                sal_Int32 nCurrParaCount( 0 );
                sal_Int32 nCurrLineCount( 0 );
                sal_Int32 nCurrSentenceCount( 0 );
                sal_Int32 nCurrWordCount( 0 );
                sal_Int32 nCurrCharCount( 0 );

                DrawShapeSubsetting::IndexClassificatorVector::const_iterator aLastShapeStart(rBegin);
                DrawShapeSubsetting::IndexClassificatorVector::const_iterator aLastParaStart(rBegin);
                DrawShapeSubsetting::IndexClassificatorVector::const_iterator aLastLineStart(rBegin);
                DrawShapeSubsetting::IndexClassificatorVector::const_iterator aLastSentenceStart(rBegin);
                DrawShapeSubsetting::IndexClassificatorVector::const_iterator aLastWordStart(rBegin);
                DrawShapeSubsetting::IndexClassificatorVector::const_iterator aLastCharStart(rBegin);

                DrawShapeSubsetting::IndexClassificatorVector::const_iterator aNext;
                DrawShapeSubsetting::IndexClassificatorVector::const_iterator aCurr( rBegin );
                while( aCurr != rEnd )
                {
                    // aNext will hold an iterator to the next element
                    // (or the past-the-end iterator, if aCurr
                    // references the last element). Used to pass a
                    // valid half-open range to the functors.
                    aNext = aCurr;
                    ++aNext;

                    switch( *aCurr )
                    {
                        default:
                            ENSURE_OR_THROW( false,
                                              "Unexpected type in iterateDocShapes()" );
                        case DrawShapeSubsetting::CLASS_NOOP:
                            // ignore NOOP actions
                            break;

                        case DrawShapeSubsetting::CLASS_SHAPE_START:
                            // regardless of ending action
                            // classifications before: a new shape
                            // always also starts contained elements
                            // anew
                            aLastShapeStart    =
                            aLastParaStart     =
                            aLastLineStart     =
                            aLastSentenceStart =
                            aLastWordStart     =
                            aLastCharStart     = aCurr;
                            break;

                        case DrawShapeSubsetting::CLASS_SHAPE_END:
                            if( !io_rFunctor( DrawShapeSubsetting::CLASS_SHAPE_END,
                                              nCurrShapeCount,
                                              aLastShapeStart,
                                              aNext ) )
                            {
                                return;
                            }

                            ++nCurrShapeCount;
                            SAL_FALLTHROUGH; // shape end also ends lines
                        case DrawShapeSubsetting::CLASS_PARAGRAPH_END:
                            if( !io_rFunctor( DrawShapeSubsetting::CLASS_PARAGRAPH_END,
                                              nCurrParaCount,
                                              aLastParaStart,
                                              aNext ) )
                            {
                                return;
                            }

                            ++nCurrParaCount;
                            aLastParaStart = aNext;
                            SAL_FALLTHROUGH; // para end also ends line
                        case DrawShapeSubsetting::CLASS_LINE_END:
                            if( !io_rFunctor( DrawShapeSubsetting::CLASS_LINE_END,
                                              nCurrLineCount,
                                              aLastLineStart,
                                              aNext ) )
                            {
                                return;
                            }

                            ++nCurrLineCount;
                            aLastLineStart = aNext;

                            if( *aCurr == DrawShapeSubsetting::CLASS_LINE_END )
                            {
                                // DON'T fall through here, as a line
                                // does NOT end neither a sentence,
                                // nor a word. OTOH, all parent
                                // structures (paragraph and shape),
                                // which itself fall through to this
                                // code, DO end word, sentence and
                                // character cell.

                                // TODO(F1): Maybe a line should end a
                                // character cell, OTOH?
                                break;
                            }
                            SAL_FALLTHROUGH;
                        case DrawShapeSubsetting::CLASS_SENTENCE_END:
                            if( !io_rFunctor( DrawShapeSubsetting::CLASS_SENTENCE_END,
                                              nCurrSentenceCount,
                                              aLastSentenceStart,
                                              aNext ) )
                            {
                                return;
                            }

                            ++nCurrSentenceCount;
                            aLastSentenceStart = aNext;
                            SAL_FALLTHROUGH;
                        case DrawShapeSubsetting::CLASS_WORD_END:
                            if( !io_rFunctor( DrawShapeSubsetting::CLASS_WORD_END,
                                              nCurrWordCount,
                                              aLastWordStart,
                                              aNext ) )
                            {
                                return;
                            }

                            ++nCurrWordCount;
                            aLastWordStart = aNext;
                            SAL_FALLTHROUGH;
                        case DrawShapeSubsetting::CLASS_CHARACTER_CELL_END:
                            if( !io_rFunctor( DrawShapeSubsetting::CLASS_CHARACTER_CELL_END,
                                              nCurrCharCount,
                                              aLastCharStart,
                                              aNext ) )
                            {
                                return;
                            }

                            ++nCurrCharCount;
                            aLastCharStart = aNext;
                            break;
                    }

                    aCurr = aNext;
                }
            }

            DrawShapeSubsetting::IndexClassificator mapDocTreeNode( DocTreeNode::NodeType eNodeType )
            {
                switch( eNodeType )
                {
                    case DocTreeNode::NODETYPE_INVALID:
                        // FALLTHROUGH intended
                    default:
                        SAL_WARN( "slideshow", "DrawShapeSubsetting::mapDocTreeNode(): unexpected node type");
                        return DrawShapeSubsetting::CLASS_NOOP;

                    case DocTreeNode::NODETYPE_LOGICAL_SHAPE:
                        // FALLTHROUGH intended
                    case DocTreeNode::NODETYPE_FORMATTING_SHAPE:
                        return DrawShapeSubsetting::CLASS_SHAPE_END;

                    case DocTreeNode::NODETYPE_FORMATTING_LINE:
                        return DrawShapeSubsetting::CLASS_LINE_END;

                    case DocTreeNode::NODETYPE_LOGICAL_PARAGRAPH:
                        return DrawShapeSubsetting::CLASS_PARAGRAPH_END;

                    case DocTreeNode::NODETYPE_LOGICAL_SENTENCE:
                        return DrawShapeSubsetting::CLASS_SENTENCE_END;

                    case DocTreeNode::NODETYPE_LOGICAL_WORD:
                        return DrawShapeSubsetting::CLASS_WORD_END;

                    case DocTreeNode::NODETYPE_LOGICAL_CHARACTER_CELL:
                        return DrawShapeSubsetting::CLASS_CHARACTER_CELL_END;
                };
            }

            /// Counts number of class occurrences
            class CountClassFunctor
            {
            public:
                explicit CountClassFunctor( DrawShapeSubsetting::IndexClassificator eClass ) :
                    meClass( eClass ),
                    mnCurrCount(0)
                {
                }

                bool operator()( DrawShapeSubsetting::IndexClassificator                                eCurrElemClassification,
                                 sal_Int32                                                              /*nCurrElemCount*/,
                                 const DrawShapeSubsetting::IndexClassificatorVector::const_iterator&   /*rCurrElemBegin*/,
                                 const DrawShapeSubsetting::IndexClassificatorVector::const_iterator&   /*rCurrElemEnd*/ )
                {
                    if( eCurrElemClassification == meClass )
                        ++mnCurrCount;

                    return true; // never stop, count all occurrences
                }

                sal_Int32 getCount() const
                {
                    return mnCurrCount;
                }

            private:
                DrawShapeSubsetting::IndexClassificator meClass;
                sal_Int32                               mnCurrCount;
            };
        }

        sal_Int32 DrawShapeSubsetting::implGetNumberOfTreeNodes( const DrawShapeSubsetting::IndexClassificatorVector::const_iterator& rBegin,
                                                                 const DrawShapeSubsetting::IndexClassificatorVector::const_iterator& rEnd,
                                                                 DocTreeNode::NodeType                                                eNodeType )
        {
            const IndexClassificator eRequestedClass(
                mapDocTreeNode( eNodeType ) );

            // create a counting functor for the requested class of
            // actions
            CountClassFunctor aFunctor( eRequestedClass );

            // count all occurrences in the given range
            iterateActionClassifications( aFunctor, rBegin, rEnd );

            return aFunctor.getCount();
        }

        sal_Int32 DrawShapeSubsetting::getNumberOfTreeNodes( DocTreeNode::NodeType eNodeType ) const
        {
            ensureInitializedNodeTree();

            return implGetNumberOfTreeNodes( maActionClassVector.begin(),
                                             maActionClassVector.end(),
                                             eNodeType );
        }

        namespace
        {
            /** This functor finds the nth occurrence of a given
                action class.

                The operator() compares the given index value with the
                requested index, as given on the functor's
                constructor. Then, the operator() returns false,
                denoting that the requested action is found.
             */
            class FindNthElementFunctor
            {
            public:
                FindNthElementFunctor( sal_Int32                                                      nNodeIndex,
                                       DrawShapeSubsetting::IndexClassificatorVector::const_iterator& rLastBegin,
                                       DrawShapeSubsetting::IndexClassificatorVector::const_iterator& rLastEnd,
                                       DrawShapeSubsetting::IndexClassificator                        eClass ) :
                    mnNodeIndex( nNodeIndex ),
                    mrLastBegin( rLastBegin ),
                    mrLastEnd( rLastEnd ),
                    meClass( eClass )
                {
                }

                bool operator()( DrawShapeSubsetting::IndexClassificator                                eCurrElemClassification,
                                 sal_Int32                                                              nCurrElemCount,
                                 const DrawShapeSubsetting::IndexClassificatorVector::const_iterator&   rCurrElemBegin,
                                 const DrawShapeSubsetting::IndexClassificatorVector::const_iterator&   rCurrElemEnd )
                {
                    if( eCurrElemClassification == meClass &&
                        nCurrElemCount == mnNodeIndex )
                    {
                        mrLastBegin = rCurrElemBegin;
                        mrLastEnd = rCurrElemEnd;

                        return false; // abort iteration, we've
                                      // already found what we've been
                                      // looking for
                    }

                    return true; // keep on truckin'
                }

            private:
                sal_Int32                                                       mnNodeIndex;
                DrawShapeSubsetting::IndexClassificatorVector::const_iterator&  mrLastBegin;
                DrawShapeSubsetting::IndexClassificatorVector::const_iterator&  mrLastEnd;
                DrawShapeSubsetting::IndexClassificator                         meClass;
            };

            DocTreeNode makeTreeNode( const DrawShapeSubsetting::IndexClassificatorVector::const_iterator& rBegin,
                                      const DrawShapeSubsetting::IndexClassificatorVector::const_iterator& rStart,
                                      const DrawShapeSubsetting::IndexClassificatorVector::const_iterator& rEnd,
                                      DocTreeNode::NodeType                                                eNodeType )
            {
                return DocTreeNode( ::std::distance(rBegin,
                                                    rStart),
                                    ::std::distance(rBegin,
                                                    rEnd),
                                    eNodeType );
            }
        }

        DocTreeNode DrawShapeSubsetting::implGetTreeNode( const IndexClassificatorVector::const_iterator&   rBegin,
                                                          const IndexClassificatorVector::const_iterator&   rEnd,
                                                          sal_Int32                                         nNodeIndex,
                                                          DocTreeNode::NodeType                             eNodeType ) const
        {
            const IndexClassificator eRequestedClass(
                mapDocTreeNode( eNodeType ) );

            DrawShapeSubsetting::IndexClassificatorVector::const_iterator aLastBegin(rEnd);
            DrawShapeSubsetting::IndexClassificatorVector::const_iterator aLastEnd(rEnd);

            // create a nth element functor for the requested class of
            // actions, and nNodeIndex as the target index
            FindNthElementFunctor aFunctor( nNodeIndex,
                                            aLastBegin,
                                            aLastEnd,
                                            eRequestedClass );

            // find given index in the given range
            iterateActionClassifications( aFunctor, rBegin, rEnd );

            return makeTreeNode( maActionClassVector.begin(),
                                 aLastBegin, aLastEnd,
                                 eNodeType );
        }

        DocTreeNode DrawShapeSubsetting::getTreeNode( sal_Int32             nNodeIndex,
                                                      DocTreeNode::NodeType eNodeType ) const
        {
            ensureInitializedNodeTree();

            return implGetTreeNode( maActionClassVector.begin(),
                                    maActionClassVector.end(),
                                    nNodeIndex,
                                    eNodeType );
        }

        sal_Int32 DrawShapeSubsetting::getNumberOfSubsetTreeNodes( const DocTreeNode&       rParentNode,
                                                                   DocTreeNode::NodeType    eNodeType ) const
        {
            ensureInitializedNodeTree();

            // convert from vector indices to vector iterators
            const DrawShapeSubsetting::IndexClassificatorVector::const_iterator aBegin( maActionClassVector.begin() );
            const DrawShapeSubsetting::IndexClassificatorVector::const_iterator aParentBegin( aBegin + rParentNode.getStartIndex() );
            const DrawShapeSubsetting::IndexClassificatorVector::const_iterator aParentEnd( aBegin + rParentNode.getEndIndex() );

            return implGetNumberOfTreeNodes( aParentBegin,
                                             aParentEnd,
                                             eNodeType );
        }

        DocTreeNode DrawShapeSubsetting::getSubsetTreeNode( const DocTreeNode&      rParentNode,
                                                            sal_Int32               nNodeIndex,
                                                            DocTreeNode::NodeType   eNodeType ) const
        {
            ensureInitializedNodeTree();

            // convert from vector indices to vector iterators
            const DrawShapeSubsetting::IndexClassificatorVector::const_iterator aBegin( maActionClassVector.begin() );
            const DrawShapeSubsetting::IndexClassificatorVector::const_iterator aParentBegin( aBegin + rParentNode.getStartIndex() );
            const DrawShapeSubsetting::IndexClassificatorVector::const_iterator aParentEnd( aBegin + rParentNode.getEndIndex() );

            return implGetTreeNode( aParentBegin,
                                    aParentEnd,
                                    nNodeIndex,
                                    eNodeType );
        }


    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
