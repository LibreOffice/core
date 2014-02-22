/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */



#include <canvas/debug.hxx>
#include <tools/diagnose_ex.h>
#include <canvas/verbosetrace.hxx>

#include <rtl/math.hxx>

#include <vcl/metaact.hxx>
#include <vcl/gdimtf.hxx>
#include <basegfx/numeric/ftools.hxx>

#include "drawshapesubsetting.hxx"
#include "drawshape.hxx"

#include <boost/bind.hpp>

#include <algorithm>
#include <functional>
#include <limits>

using namespace ::com::sun::star;


namespace slideshow
{
    namespace internal
    {

        
        //
        
        //
        

        void DrawShapeSubsetting::ensureInitializedNodeTree() const
        {
            ENSURE_OR_THROW( mpMtf,
                              "DrawShapeSubsetting::ensureInitializedNodeTree(): Invalid mtf" );

            if( mbNodeTreeInitialized )
                return; 

            
            maActionClassVector.clear();
            maActionClassVector.reserve( mpMtf->GetActionSize() );

            
            MetaAction* pCurrAct;

            sal_Int32 nActionIndex(0);
            sal_Int32 nLastTextActionIndex(0);
            for( pCurrAct = mpMtf->FirstAction(); pCurrAct; pCurrAct = mpMtf->NextAction() )
            {
                
                switch( pCurrAct->GetType() )
                {
                    case META_COMMENT_ACTION:
                    {
                        MetaCommentAction* pAct = static_cast<MetaCommentAction*>(pCurrAct);

                        
                        if( pAct->GetComment().matchIgnoreAsciiCase( OString("XTEXT"), 0 ) )
                        {
                            
                            
                            
                            maActionClassVector.resize( nActionIndex+1, CLASS_NOOP );

                            if( pAct->GetComment().equalsIgnoreAsciiCase("XTEXT_EOC") )
                            {
                                
                                
                                
                                
                                
                                const sal_Int32 nIndex( nLastTextActionIndex + pAct->GetValue() );

                                ENSURE_OR_THROW( static_cast< ::std::size_t >(nIndex) < maActionClassVector.size(),
                                                  "DrawShapeSubsetting::ensureInitializedNodeTree(): sentence index out of range" );

                                maActionClassVector[ nIndex ] = CLASS_CHARACTER_CELL_END;
                            }
                            else if( pAct->GetComment().equalsIgnoreAsciiCase("XTEXT_EOW") )
                            {
                                
                                
                                
                                
                                
                                const sal_Int32 nIndex( nLastTextActionIndex + pAct->GetValue() );

                                ENSURE_OR_THROW( static_cast< ::std::size_t >(nIndex) < maActionClassVector.size(),
                                                  "DrawShapeSubsetting::ensureInitializedNodeTree(): sentence index out of range" );

                                maActionClassVector[ nIndex ] = CLASS_WORD_END;
                            }
                            else if( pAct->GetComment().equalsIgnoreAsciiCase("XTEXT_EOS") )
                            {
                                
                                
                                
                                
                                
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
                        VERBOSE_TRACE( "Shape text structure: %s at action #%d",
                                       pAct->GetComment().getStr(), nActionIndex );
                        ++nActionIndex;
                        break;
                    }
                    case META_TEXT_ACTION:
                    case META_TEXTARRAY_ACTION:
                    case META_STRETCHTEXT_ACTION:
                        nLastTextActionIndex = nActionIndex;
#if OSL_DEBUG_LEVEL > 1
                        {
                            MetaTextAction* pText = static_cast<MetaTextAction*>(pCurrAct);
                            VERBOSE_TRACE( "Shape text \"%s\" at action #%d",
                                           OUStringToOString(pText->GetText(),
                                                      RTL_TEXTENCODING_ISO_8859_1).getStr(),
                                           nActionIndex );
                        }
#endif
                        
                    default:
                        
                        
                        nActionIndex += getNextActionOffset(pCurrAct);
                        break;
                }
            }

            mbNodeTreeInitialized = true;
        }

        void DrawShapeSubsetting::updateSubsetBounds( const SubsetEntry& rSubsetEntry )
        {
            
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
                    
                    
                    maCurrentSubsets.push_back( DocTreeNode( 0,
                                                             mnMinSubsetActionIndex,
                                                             DocTreeNode::NODETYPE_INVALID ) );
                    maCurrentSubsets.push_back( DocTreeNode( mnMaxSubsetActionIndex,
                                                             maActionClassVector.size(),
                                                             DocTreeNode::NODETYPE_INVALID ) );
                }
                else
                {
                    
                    
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
                
                if( !maSubset.isEmpty() )
                {
                    
                    maCurrentSubsets.push_back( maSubset );
                }
            }
        }

        
        //
        
        //
        

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

            
            SubsetEntry aEntry;

            aEntry.mnStartActionIndex   = rTreeNode.getStartIndex();
            aEntry.mnEndActionIndex     = rTreeNode.getEndIndex();

            ShapeSet::const_iterator aIter;
            if( (aIter=maSubsetShapes.find( aEntry )) != maSubsetShapes.end() )
            {
                
                return aIter->mpShape;
            }

            return AttributableShapeSharedPtr();
        }

        void DrawShapeSubsetting::addSubsetShape( const AttributableShapeSharedPtr& rShape )
        {
            SAL_INFO( "slideshow", "::presentation::internal::DrawShapeSubsetting::addSubsetShape()" );

            
            SubsetEntry aEntry;
            const DocTreeNode& rEffectiveSubset( rShape->getSubsetNode() );

            aEntry.mnStartActionIndex   = rEffectiveSubset.getStartIndex();
            aEntry.mnEndActionIndex     = rEffectiveSubset.getEndIndex();

            ShapeSet::const_iterator aIter;
            if( (aIter=maSubsetShapes.find( aEntry )) != maSubsetShapes.end() )
            {
                

                
                
                const_cast<SubsetEntry&>(*aIter).mnSubsetQueriedCount++;
            }
            else
            {
                
                aEntry.mnSubsetQueriedCount = 1;
                aEntry.mpShape = rShape;

                maSubsetShapes.insert( aEntry );

                
                updateSubsetBounds( aEntry );
                updateSubsets();
            }
        }

        bool DrawShapeSubsetting::revokeSubsetShape( const AttributableShapeSharedPtr& rShape )
        {
            SAL_INFO( "slideshow", "::presentation::internal::DrawShapeSubsetting::revokeSubsetShape()" );

            
            SubsetEntry aEntry;
            const DocTreeNode& rEffectiveSubset( rShape->getSubsetNode() );

            aEntry.mnStartActionIndex   = rEffectiveSubset.getStartIndex();
            aEntry.mnEndActionIndex     = rEffectiveSubset.getEndIndex();

            ShapeSet::iterator aIter;
            if( (aIter=maSubsetShapes.find( aEntry )) == maSubsetShapes.end() )
                return false; 

            
            if( aIter->mnSubsetQueriedCount > 1 )
            {
                
                
                const_cast<SubsetEntry&>(*aIter).mnSubsetQueriedCount--;

                VERBOSE_TRACE( "Subset summary: shape 0x%X, %d open subsets, revoked subset has refcount %d",
                               this,
                               maSubsetShapes.size(),
                               aIter->mnSubsetQueriedCount );

                return false; 
            }

            VERBOSE_TRACE( "Subset summary: shape 0x%X, %d open subsets, cleared subset has range [%d,%d]",
                           this,
                           maSubsetShapes.size(),
                           aEntry.mnStartActionIndex,
                           aEntry.mnEndActionIndex );

            
            maSubsetShapes.erase( aIter );


            
            
            
            

            
            mnMinSubsetActionIndex = SAL_MAX_INT32;
            mnMaxSubsetActionIndex = 0;

            
            
            

            
            ::std::for_each( maSubsetShapes.begin(),
                             maSubsetShapes.end(),
                             ::boost::bind(&DrawShapeSubsetting::updateSubsetBounds,
                                           this,
                                           _1 ) );

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
                    
                    
                    
                    
                    aNext = aCurr;
                    ++aNext;

                    switch( *aCurr )
                    {
                        default:
                            ENSURE_OR_THROW( false,
                                              "Unexpected type in iterateDocShapes()" );
                        case DrawShapeSubsetting::CLASS_NOOP:
                            
                            break;

                        case DrawShapeSubsetting::CLASS_SHAPE_START:
                            
                            
                            
                            
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
                                
                                
                                
                                
                                
                                
                                

                                
                                
                                break;
                            }
                            
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
                        
                    default:
                        SAL_WARN( "slideshow", "DrawShapeSubsetting::mapDocTreeNode(): unexpected node type");
                        return DrawShapeSubsetting::CLASS_NOOP;

                    case DocTreeNode::NODETYPE_LOGICAL_SHAPE:
                        
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

            
            class CountClassFunctor
            {
            public:
                CountClassFunctor( DrawShapeSubsetting::IndexClassificator eClass ) :
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

                    return true; 
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
                                                                 DocTreeNode::NodeType                                                eNodeType ) const
        {
            const IndexClassificator eRequestedClass(
                mapDocTreeNode( eNodeType ) );

            
            
            CountClassFunctor aFunctor( eRequestedClass );

            
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

                        return false; 
                                      
                                      
                    }

                    return true; 
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

            
            
            FindNthElementFunctor aFunctor( nNodeIndex,
                                            aLastBegin,
                                            aLastEnd,
                                            eRequestedClass );

            
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

            
            const DrawShapeSubsetting::IndexClassificatorVector::const_iterator aBegin( maActionClassVector.begin() );
            const DrawShapeSubsetting::IndexClassificatorVector::const_iterator aParentBegin( aBegin + rParentNode.getStartIndex() );
            const DrawShapeSubsetting::IndexClassificatorVector::const_iterator aParentEnd( aBegin + rParentNode.getEndIndex() );

            return implGetTreeNode( aParentBegin,
                                    aParentEnd,
                                    nNodeIndex,
                                    eNodeType );
        }

        const VectorOfDocTreeNodes& DrawShapeSubsetting::getActiveSubsets() const
        {
            return maCurrentSubsets;
        }

    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
