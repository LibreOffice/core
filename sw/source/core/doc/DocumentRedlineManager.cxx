
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
#include <DocumentRedlineManager.hxx>
#include <frmfmt.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <IDocumentState.hxx>
#include <redline.hxx>
#include <UndoRedline.hxx>
#include <docary.hxx>
#include <ndtxt.hxx>
#include <comcore.hrc>
#include <swmodule.hxx>
#include <editsh.hxx>
#include <vcl/layout.hxx>

using namespace com::sun::star;

#ifdef DBG_UTIL

    #define _ERROR_PREFIX "redline table corrupted: "

    namespace
    {
        // helper function for lcl_CheckRedline
        // 1. make sure that pPos->nContent points into pPos->nNode
        // 2. check that position is valid and doesn't point after text
        static void lcl_CheckPosition( const SwPosition* pPos )
        {
            assert(dynamic_cast<SwIndexReg*>(&pPos->nNode.GetNode())
                    == pPos->nContent.GetIdxReg());

            SwTextNode* pTextNode = pPos->nNode.GetNode().GetTextNode();
            if( pTextNode == nullptr )
            {
                assert(pPos->nContent == 0);
            }
            else
            {
                assert(pPos->nContent >= 0 && pPos->nContent <= pTextNode->Len());
            }
        }

        static void lcl_CheckPam( const SwPaM* pPam )
        {
            assert(pPam);
            lcl_CheckPosition( pPam->GetPoint() );
            lcl_CheckPosition( pPam->GetMark() );
        }

        // check validity of the redline table. Checks redline bounds, and make
        // sure the redlines are sorted and non-overlapping.
        static void lcl_CheckRedline( IDocumentRedlineAccess& redlineAccess )
        {
            const SwRedlineTable& rTable = redlineAccess.GetRedlineTable();

            // verify valid redline positions
            for( size_t i = 0; i < rTable.size(); ++i )
                lcl_CheckPam( rTable[ i ] );

            for( size_t j = 0; j < rTable.size(); ++j )
            {
                // check for empty redlines
                OSL_ENSURE( ( *(rTable[j]->GetPoint()) != *(rTable[j]->GetMark()) ) ||
                            ( rTable[j]->GetContentIdx() != nullptr ),
                            _ERROR_PREFIX "empty redline" );
             }

            // verify proper redline sorting
            for( size_t n = 1; n < rTable.size(); ++n )
            {
                const SwRangeRedline* pPrev = rTable[ n-1 ];
                const SwRangeRedline* pCurrent = rTable[ n ];

                // check redline sorting
                SAL_WARN_IF( *pPrev->Start() > *pCurrent->Start(), "sw",
                             _ERROR_PREFIX "not sorted correctly" );

                // check for overlapping redlines
                SAL_WARN_IF( *pPrev->End() > *pCurrent->Start(), "sw",
                             _ERROR_PREFIX "overlapping redlines" );
            }

            assert(std::is_sorted(rTable.begin(), rTable.end(), CompareSwRedlineTable()));
        }
    }

    #define _CHECK_REDLINE( pDoc ) lcl_CheckRedline( pDoc );

#else

    #define _CHECK_REDLINE( pDoc )

#endif

namespace
{
    static inline bool IsPrevPos( const SwPosition & rPos1, const SwPosition & rPos2 )
    {
        const SwContentNode* pCNd;
        return 0 == rPos2.nContent.GetIndex() &&
               rPos2.nNode.GetIndex() - 1 == rPos1.nNode.GetIndex() &&
               nullptr != ( pCNd = rPos1.nNode.GetNode().GetContentNode() ) &&
               rPos1.nContent.GetIndex() == pCNd->Len();
    }

    static bool lcl_AcceptRedline( SwRedlineTable& rArr, sal_uInt16& rPos,
                            bool bCallDelete,
                            const SwPosition* pSttRng = nullptr,
                            const SwPosition* pEndRng = nullptr )
    {
        bool bRet = true;
        SwRangeRedline* pRedl = rArr[ rPos ];
        SwPosition *pRStt = nullptr, *pREnd = nullptr;
        SwComparePosition eCmp = POS_OUTSIDE;
        if( pSttRng && pEndRng )
        {
            pRStt = pRedl->Start();
            pREnd = pRedl->End();
            eCmp = ComparePosition( *pSttRng, *pEndRng, *pRStt, *pREnd );
        }

        pRedl->InvalidateRange();

        switch( pRedl->GetType() )
        {
        case nsRedlineType_t::REDLINE_INSERT:
        case nsRedlineType_t::REDLINE_FORMAT:
            {
                bool bCheck = false, bReplace = false;
                switch( eCmp )
                {
                case POS_INSIDE:
                    if( *pSttRng == *pRStt )
                        pRedl->SetStart( *pEndRng, pRStt );
                    else
                    {
                        if( *pEndRng != *pREnd )
                        {
                            // split up
                            SwRangeRedline* pNew = new SwRangeRedline( *pRedl );
                            pNew->SetStart( *pEndRng );
                            rArr.Insert( pNew ); ++rPos;
                        }
                        pRedl->SetEnd( *pSttRng, pREnd );
                        bCheck = true;
                    }
                    break;

                case POS_OVERLAP_BEFORE:
                    pRedl->SetStart( *pEndRng, pRStt );
                    bReplace = true;
                    break;

                case POS_OVERLAP_BEHIND:
                    pRedl->SetEnd( *pSttRng, pREnd );
                    bCheck = true;
                    break;

                case POS_OUTSIDE:
                case POS_EQUAL:
                    rArr.DeleteAndDestroy( rPos-- );
                    break;

                default:
                    bRet = false;
                }

                if( bReplace || ( bCheck && !pRedl->HasValidRange() ))
                {
                    // re-insert
                    rArr.Remove( pRedl );
                    rArr.Insert( pRedl );
                }
            }
            break;
        case nsRedlineType_t::REDLINE_DELETE:
            {
                SwDoc& rDoc = *pRedl->GetDoc();
                const SwPosition *pDelStt = nullptr, *pDelEnd = nullptr;
                bool bDelRedl = false;
                switch( eCmp )
                {
                case POS_INSIDE:
                    if( bCallDelete )
                    {
                        pDelStt = pSttRng;
                        pDelEnd = pEndRng;
                    }
                    break;

                case POS_OVERLAP_BEFORE:
                    if( bCallDelete )
                    {
                        pDelStt = pRStt;
                        pDelEnd = pEndRng;
                    }
                    break;
                case POS_OVERLAP_BEHIND:
                    if( bCallDelete )
                    {
                        pDelStt = pREnd;
                        pDelEnd = pSttRng;
                    }
                    break;

                case POS_OUTSIDE:
                case POS_EQUAL:
                    {
                        rArr.Remove( rPos-- );
                        bDelRedl = true;
                        if( bCallDelete )
                        {
                            pDelStt = pRedl->Start();
                            pDelEnd = pRedl->End();
                        }
                    }
                    break;
                default:
                    bRet = false;
                }

                if( pDelStt && pDelEnd )
                {
                    SwPaM aPam( *pDelStt, *pDelEnd );
                    SwContentNode* pCSttNd = pDelStt->nNode.GetNode().GetContentNode();
                    SwContentNode* pCEndNd = pDelEnd->nNode.GetNode().GetContentNode();

                    if( bDelRedl )
                        delete pRedl;

                    RedlineMode_t eOld = rDoc.getIDocumentRedlineAccess().GetRedlineMode();
                    rDoc.getIDocumentRedlineAccess().SetRedlineMode_intern( (RedlineMode_t)(eOld & ~(nsRedlineMode_t::REDLINE_ON | nsRedlineMode_t::REDLINE_IGNORE)));

                    if( pCSttNd && pCEndNd )
                        rDoc.getIDocumentContentOperations().DeleteAndJoin( aPam );
                    else
                    {
                        rDoc.getIDocumentContentOperations().DeleteRange( aPam );

                        if( pCSttNd && !pCEndNd )
                        {
                            aPam.GetBound().nContent.Assign( nullptr, 0 );
                            aPam.GetBound( false ).nContent.Assign( nullptr, 0 );
                            aPam.DeleteMark();
                            rDoc.getIDocumentContentOperations().DelFullPara( aPam );
                        }
                    }
                    rDoc.getIDocumentRedlineAccess().SetRedlineMode_intern( eOld );
                }
                else if( bDelRedl )
                    delete pRedl;
            }
            break;

        case nsRedlineType_t::REDLINE_FMTCOLL:
            rArr.DeleteAndDestroy( rPos-- );
            break;

        default:
            bRet = false;
        }
        return bRet;
    }

    static bool lcl_RejectRedline( SwRedlineTable& rArr, sal_uInt16& rPos,
                            bool bCallDelete,
                            const SwPosition* pSttRng = nullptr,
                            const SwPosition* pEndRng = nullptr )
    {
        bool bRet = true;
        SwRangeRedline* pRedl = rArr[ rPos ];
        SwPosition *pRStt = nullptr, *pREnd = nullptr;
        SwComparePosition eCmp = POS_OUTSIDE;
        if( pSttRng && pEndRng )
        {
            pRStt = pRedl->Start();
            pREnd = pRedl->End();
            eCmp = ComparePosition( *pSttRng, *pEndRng, *pRStt, *pREnd );
        }

        pRedl->InvalidateRange();

        switch( pRedl->GetType() )
        {
        case nsRedlineType_t::REDLINE_INSERT:
            {
                SwDoc& rDoc = *pRedl->GetDoc();
                const SwPosition *pDelStt = nullptr, *pDelEnd = nullptr;
                bool bDelRedl = false;
                switch( eCmp )
                {
                case POS_INSIDE:
                    if( bCallDelete )
                    {
                        pDelStt = pSttRng;
                        pDelEnd = pEndRng;
                    }
                    break;

                case POS_OVERLAP_BEFORE:
                    if( bCallDelete )
                    {
                        pDelStt = pRStt;
                        pDelEnd = pEndRng;
                    }
                    break;
                case POS_OVERLAP_BEHIND:
                    if( bCallDelete )
                    {
                        pDelStt = pREnd;
                        pDelEnd = pSttRng;
                    }
                    break;
                case POS_OUTSIDE:
                case POS_EQUAL:
                    {
                        // delete the range again
                        rArr.Remove( rPos-- );
                        bDelRedl = true;
                        if( bCallDelete )
                        {
                            pDelStt = pRedl->Start();
                            pDelEnd = pRedl->End();
                        }
                    }
                    break;

                default:
                    bRet = false;
                }
                if( pDelStt && pDelEnd )
                {
                    SwPaM aPam( *pDelStt, *pDelEnd );

                    SwContentNode* pCSttNd = pDelStt->nNode.GetNode().GetContentNode();
                    SwContentNode* pCEndNd = pDelEnd->nNode.GetNode().GetContentNode();

                    if( bDelRedl )
                        delete pRedl;

                    RedlineMode_t eOld = rDoc.getIDocumentRedlineAccess().GetRedlineMode();
                    rDoc.getIDocumentRedlineAccess().SetRedlineMode_intern( (RedlineMode_t)(eOld & ~(nsRedlineMode_t::REDLINE_ON | nsRedlineMode_t::REDLINE_IGNORE)));

                    if( pCSttNd && pCEndNd )
                        rDoc.getIDocumentContentOperations().DeleteAndJoin( aPam );
                    else
                    {
                        rDoc.getIDocumentContentOperations().DeleteRange( aPam );

                        if( pCSttNd && !pCEndNd )
                        {
                            aPam.GetBound().nContent.Assign( nullptr, 0 );
                            aPam.GetBound( false ).nContent.Assign( nullptr, 0 );
                            aPam.DeleteMark();
                            rDoc.getIDocumentContentOperations().DelFullPara( aPam );
                        }
                    }
                    rDoc.getIDocumentRedlineAccess().SetRedlineMode_intern( eOld );
                }
                else if( bDelRedl )
                    delete pRedl;
            }
            break;
        case nsRedlineType_t::REDLINE_DELETE:
            {
                SwRangeRedline* pNew = nullptr;
                bool bCheck = false, bReplace = false;

                switch( eCmp )
                {
                case POS_INSIDE:
                    {
                        if( 1 < pRedl->GetStackCount() )
                        {
                            pNew = new SwRangeRedline( *pRedl );
                            pNew->PopData();
                        }
                        if( *pSttRng == *pRStt )
                        {
                            pRedl->SetStart( *pEndRng, pRStt );
                            bReplace = true;
                            if( pNew )
                                pNew->SetEnd( *pEndRng );
                        }
                        else
                        {
                            if( *pEndRng != *pREnd )
                            {
                                // split up
                                SwRangeRedline* pCpy = new SwRangeRedline( *pRedl );
                                pCpy->SetStart( *pEndRng );
                                rArr.Insert( pCpy ); ++rPos;
                                if( pNew )
                                    pNew->SetEnd( *pEndRng );
                            }

                            pRedl->SetEnd( *pSttRng, pREnd );
                            bCheck = true;
                            if( pNew )
                                pNew->SetStart( *pSttRng );
                        }
                    }
                    break;

                case POS_OVERLAP_BEFORE:
                    if( 1 < pRedl->GetStackCount() )
                    {
                        pNew = new SwRangeRedline( *pRedl );
                        pNew->PopData();
                    }
                    pRedl->SetStart( *pEndRng, pRStt );
                    bReplace = true;
                    if( pNew )
                        pNew->SetEnd( *pEndRng );
                    break;

                case POS_OVERLAP_BEHIND:
                    if( 1 < pRedl->GetStackCount() )
                    {
                        pNew = new SwRangeRedline( *pRedl );
                        pNew->PopData();
                    }
                    pRedl->SetEnd( *pSttRng, pREnd );
                    bCheck = true;
                    if( pNew )
                        pNew->SetStart( *pSttRng );
                    break;

                case POS_OUTSIDE:
                case POS_EQUAL:
                    if( !pRedl->PopData() )
                        // deleting the RedlineObject is enough
                        rArr.DeleteAndDestroy( rPos-- );
                    break;

                default:
                    bRet = false;
                }

                if( pNew )
                {
                    rArr.Insert( pNew ); ++rPos;
                }

                if( bReplace || ( bCheck && !pRedl->HasValidRange() ))
                {
                    // re-insert
                    rArr.Remove( pRedl );
                    rArr.Insert( pRedl );
                }
            }
            break;

        case nsRedlineType_t::REDLINE_FORMAT:
        case nsRedlineType_t::REDLINE_FMTCOLL:
            {
                if( pRedl->GetExtraData() )
                    pRedl->GetExtraData()->Reject( *pRedl );
                rArr.DeleteAndDestroy( rPos-- );
            }
            break;

        default:
            bRet = false;
        }
        return bRet;
    }

    typedef bool (*Fn_AcceptReject)( SwRedlineTable& rArr, sal_uInt16& rPos,
                            bool bCallDelete,
                            const SwPosition* pSttRng,
                            const SwPosition* pEndRng);


    static int lcl_AcceptRejectRedl( Fn_AcceptReject fn_AcceptReject,
                                SwRedlineTable& rArr, bool bCallDelete,
                                const SwPaM& rPam)
    {
        sal_uInt16 n = 0;
        int nCount = 0;

        const SwPosition* pStt = rPam.Start(),
                        * pEnd = pStt == rPam.GetPoint() ? rPam.GetMark()
                                                         : rPam.GetPoint();
        const SwRangeRedline* pFnd = rArr.FindAtPosition( *pStt, n );
        if( pFnd &&     // Is new a part of it?
            ( *pFnd->Start() != *pStt || *pFnd->End() > *pEnd ))
        {
            // Only revoke the partial selection
            if( (*fn_AcceptReject)( rArr, n, bCallDelete, pStt, pEnd ))
                nCount++;
            ++n;
        }

        for( ; n < rArr.size(); ++n )
        {
            SwRangeRedline* pTmp = rArr[ n ];
            if( pTmp->HasMark() && pTmp->IsVisible() )
            {
                if( *pTmp->End() <= *pEnd )
                {
                    if( (*fn_AcceptReject)( rArr, n, bCallDelete, nullptr, nullptr ))
                        nCount++;
                }
                else
                {
                    if( *pTmp->Start() < *pEnd )
                    {
                        // Only revoke the partial selection
                        if( (*fn_AcceptReject)( rArr, n, bCallDelete, pStt, pEnd ))
                            nCount++;
                    }
                    break;
                }
            }
        }
        return nCount;
    }

    static void lcl_AdjustRedlineRange( SwPaM& rPam )
    {
        // The Selection is only in the ContentSection. If there are Redlines
        // to Non-ContentNodes before or after that, then the Selections
        // expand to them.
        SwPosition* pStt = rPam.Start(),
                  * pEnd = pStt == rPam.GetPoint() ? rPam.GetMark()
                                                   : rPam.GetPoint();
        SwDoc* pDoc = rPam.GetDoc();
        if( !pStt->nContent.GetIndex() &&
            !pDoc->GetNodes()[ pStt->nNode.GetIndex() - 1 ]->IsContentNode() )
        {
            const SwRangeRedline* pRedl = pDoc->getIDocumentRedlineAccess().GetRedline( *pStt, nullptr );
            if( pRedl )
            {
                const SwPosition* pRStt = pRedl->Start();
                if( !pRStt->nContent.GetIndex() && pRStt->nNode.GetIndex() ==
                    pStt->nNode.GetIndex() - 1 )
                    *pStt = *pRStt;
            }
        }
        if( pEnd->nNode.GetNode().IsContentNode() &&
            !pDoc->GetNodes()[ pEnd->nNode.GetIndex() + 1 ]->IsContentNode() &&
            pEnd->nContent.GetIndex() == pEnd->nNode.GetNode().GetContentNode()->Len()    )
        {
            const SwRangeRedline* pRedl = pDoc->getIDocumentRedlineAccess().GetRedline( *pEnd, nullptr );
            if( pRedl )
            {
                const SwPosition* pREnd = pRedl->End();
                if( !pREnd->nContent.GetIndex() && pREnd->nNode.GetIndex() ==
                    pEnd->nNode.GetIndex() + 1 )
                    *pEnd = *pREnd;
            }
        }
    }
}

namespace sw
{

DocumentRedlineManager::DocumentRedlineManager( SwDoc& i_rSwdoc ) : m_rDoc( i_rSwdoc ),
                                                                    meRedlineMode((RedlineMode_t)(nsRedlineMode_t::REDLINE_SHOW_INSERT | nsRedlineMode_t::REDLINE_SHOW_DELETE)),
                                                                    mpRedlineTable( new SwRedlineTable ),
                                                                    mpExtraRedlineTable ( new SwExtraRedlineTable ),
                                                                    mpAutoFormatRedlnComment( nullptr ),
                                                                    mbIsRedlineMove(false),
                                                                    mbReadlineChecked(false),
                                                                    mnAutoFormatRedlnCommentNo( 0 )
{
}

RedlineMode_t DocumentRedlineManager::GetRedlineMode() const
{
    return meRedlineMode;
}

void DocumentRedlineManager::SetRedlineMode( RedlineMode_t eMode )
{
    if( meRedlineMode != eMode )
    {
        if( (nsRedlineMode_t::REDLINE_SHOW_MASK & meRedlineMode) != (nsRedlineMode_t::REDLINE_SHOW_MASK & eMode)
            || 0 == (nsRedlineMode_t::REDLINE_SHOW_MASK & eMode) )
        {
            bool bSaveInXMLImportFlag = m_rDoc.IsInXMLImport();
            m_rDoc.SetInXMLImport( false );
            // and then hide/display everything
            void (SwRangeRedline::*pFnc)(sal_uInt16, size_t) = nullptr;

            switch( nsRedlineMode_t::REDLINE_SHOW_MASK & eMode )
            {
            case nsRedlineMode_t::REDLINE_SHOW_INSERT | nsRedlineMode_t::REDLINE_SHOW_DELETE :
                pFnc = &SwRangeRedline::Show;
                break;
            case nsRedlineMode_t::REDLINE_SHOW_INSERT:
                pFnc = &SwRangeRedline::Hide;
                break;
            case nsRedlineMode_t::REDLINE_SHOW_DELETE:
                pFnc = &SwRangeRedline::ShowOriginal;
                break;

            default:
                pFnc = &SwRangeRedline::Hide;
                eMode = (RedlineMode_t)(eMode | nsRedlineMode_t::REDLINE_SHOW_INSERT);
                break;
            }

            CheckAnchoredFlyConsistency(m_rDoc);
            _CHECK_REDLINE( *this )

            if (pFnc)
            {
                for (sal_uInt16 nLoop = 1; nLoop <= 2; ++nLoop)
                    for (size_t i = 0; i < mpRedlineTable->size(); ++i)
                        ((*mpRedlineTable)[i]->*pFnc)(nLoop, i);

                //SwRangeRedline::MoveFromSection routinely changes
                //the keys that mpRedlineTable is sorted by
                mpRedlineTable->Resort();
            }

            CheckAnchoredFlyConsistency(m_rDoc);
            _CHECK_REDLINE( *this )
            m_rDoc.SetInXMLImport( bSaveInXMLImportFlag );
        }
        meRedlineMode = eMode;
        m_rDoc.getIDocumentState().SetModified();
    }

    // #TODO - add 'SwExtraRedlineTable' also ?
}

bool DocumentRedlineManager::IsRedlineOn() const
{
    return IDocumentRedlineAccess::IsRedlineOn(meRedlineMode);
}

bool DocumentRedlineManager::IsIgnoreRedline() const
{
    return (nsRedlineMode_t::REDLINE_IGNORE & meRedlineMode);
}

void DocumentRedlineManager::SetRedlineMode_intern(RedlineMode_t eMode)
{
    meRedlineMode = eMode;
}

const SwRedlineTable& DocumentRedlineManager::GetRedlineTable() const
{
    return *mpRedlineTable;
}

SwRedlineTable& DocumentRedlineManager::GetRedlineTable()
{
    return *mpRedlineTable;
}

const SwExtraRedlineTable& DocumentRedlineManager::GetExtraRedlineTable() const
{
    return *mpExtraRedlineTable;
}

SwExtraRedlineTable& DocumentRedlineManager::GetExtraRedlineTable()
{
    return *mpExtraRedlineTable;
}

bool DocumentRedlineManager::HasExtraRedlineTable() const
{
    return mpExtraRedlineTable != nullptr;
}

bool DocumentRedlineManager::IsInRedlines(const SwNode & rNode) const
{
    SwPosition aPos(rNode);
    SwNode & rEndOfRedlines = m_rDoc.GetNodes().GetEndOfRedlines();
    SwPaM aPam(SwPosition(*rEndOfRedlines.StartOfSectionNode()),
               SwPosition(rEndOfRedlines));

    return aPam.ContainsPosition(aPos);
}

bool DocumentRedlineManager::IsRedlineMove() const
{
    return mbIsRedlineMove;
}

void DocumentRedlineManager::SetRedlineMove(bool bFlag)
{
    mbIsRedlineMove = bFlag;
}

/*
Text means Text not "polluted" by Redlines.

Behaviour of Insert-Redline:
    - in the Text                       - insert Redline Object
    - in InsertRedline (own)            - ignore, existing is extended
    - in InsertRedline (others)         - split up InsertRedline and
                                          insert Redline Object
    - in DeleteRedline                  - split up DeleteRedline or
                                          move at the end/beginning

Behaviour of Delete-Redline:
    - in the Text                       - insert Redline Object
    - in DeleteRedline (own/others)     - ignore
    - in InsertRedline (own)            - ignore, but delete character
    - in InsertRedline (others)         - split up InsertRedline and
                                          insert Redline Object
    - Text and own Insert overlap       - delete Text in the own Insert,
                                          extend in the other Text
                                          (up to the Insert!)
    - Text and other Insert overlap     - insert Redline Object, the
                                          other Insert is overlapped by
                                          the Delete
*/
bool DocumentRedlineManager::AppendRedline( SwRangeRedline* pNewRedl, bool bCallDelete )
{
    bool bMerged = false;
    _CHECK_REDLINE( *this )

    if (IsRedlineOn() && !IsShowOriginal(meRedlineMode))
    {
        pNewRedl->InvalidateRange();

        if( m_rDoc.IsAutoFormatRedline() )
        {
            pNewRedl->SetAutoFormatFlag();
            if( mpAutoFormatRedlnComment && !mpAutoFormatRedlnComment->isEmpty() )
            {
                pNewRedl->SetComment( *mpAutoFormatRedlnComment );
                pNewRedl->SetSeqNo( mnAutoFormatRedlnCommentNo );
            }
        }

        SwPosition* pStt = pNewRedl->Start(),
                  * pEnd = pStt == pNewRedl->GetPoint() ? pNewRedl->GetMark()
                                                        : pNewRedl->GetPoint();
        {
            SwTextNode* pTextNode = pStt->nNode.GetNode().GetTextNode();
            if( pTextNode == nullptr )
            {
                if( pStt->nContent > 0 )
                {
                    OSL_ENSURE( false, "Redline start: non-text-node with content" );
                    pStt->nContent = 0;
                }
            }
            else
            {
                if( pStt->nContent > pTextNode->Len() )
                {
                    OSL_ENSURE( false, "Redline start: index after text" );
                    pStt->nContent = pTextNode->Len();
                }
            }
            pTextNode = pEnd->nNode.GetNode().GetTextNode();
            if( pTextNode == nullptr )
            {
                if( pEnd->nContent > 0 )
                {
                    OSL_ENSURE( false, "Redline end: non-text-node with content" );
                    pEnd->nContent = 0;
                }
            }
            else
            {
                if( pEnd->nContent > pTextNode->Len() )
                {
                    OSL_ENSURE( false, "Redline end: index after text" );
                    pEnd->nContent = pTextNode->Len();
                }
            }
        }
        if( ( *pStt == *pEnd ) &&
            ( pNewRedl->GetContentIdx() == nullptr ) )
        {   // Do not insert empty redlines
            delete pNewRedl;
            return false;
        }
        bool bCompress = false;
        sal_uInt16 n = 0;
        // look up the first Redline for the starting position
        if( !GetRedline( *pStt, &n ) && n )
            --n;
        bool bDec = false;

        for( ; pNewRedl && n < mpRedlineTable->size(); bDec ? n : ++n )
        {
            bDec = false;

            SwRangeRedline* pRedl = (*mpRedlineTable)[ n ];
            SwPosition* pRStt = pRedl->Start(),
                      * pREnd = pRStt == pRedl->GetPoint() ? pRedl->GetMark()
                                                           : pRedl->GetPoint();

            // #i8518# remove empty redlines while we're at it
            if( ( *pRStt == *pREnd ) &&
                ( pRedl->GetContentIdx() == nullptr ) )
            {
                mpRedlineTable->DeleteAndDestroy(n);
                continue;
            }

            SwComparePosition eCmpPos = ComparePosition( *pStt, *pEnd, *pRStt, *pREnd );

            switch( pNewRedl->GetType() )
            {
            case nsRedlineType_t::REDLINE_INSERT:
                switch( pRedl->GetType() )
                {
                case nsRedlineType_t::REDLINE_INSERT:
                    if( pRedl->IsOwnRedline( *pNewRedl ) )
                    {
                        bool bDelete = false;

                        // Merge if applicable?
                        if( (( POS_BEHIND == eCmpPos &&
                               IsPrevPos( *pREnd, *pStt ) ) ||
                             ( POS_COLLIDE_START == eCmpPos ) ||
                             ( POS_OVERLAP_BEHIND == eCmpPos ) ) &&
                            pRedl->CanCombine( *pNewRedl ) &&
                            ( n+1 >= (sal_uInt16)mpRedlineTable->size() ||
                             ( *(*mpRedlineTable)[ n+1 ]->Start() >= *pEnd &&
                             *(*mpRedlineTable)[ n+1 ]->Start() != *pREnd ) ) )
                        {
                            pRedl->SetEnd( *pEnd, pREnd );
                            if( !pRedl->HasValidRange() )
                            {
                                // re-insert
                                mpRedlineTable->Remove( n );
                                mpRedlineTable->Insert( pRedl );
                            }

                            bMerged = true;
                            bDelete = true;
                        }
                        else if( (( POS_BEFORE == eCmpPos &&
                                    IsPrevPos( *pEnd, *pRStt ) ) ||
                                   ( POS_COLLIDE_END == eCmpPos ) ||
                                  ( POS_OVERLAP_BEFORE == eCmpPos ) ) &&
                            pRedl->CanCombine( *pNewRedl ) &&
                            ( !n ||
                             *(*mpRedlineTable)[ n-1 ]->End() != *pRStt ))
                        {
                            pRedl->SetStart( *pStt, pRStt );
                            // re-insert
                            mpRedlineTable->Remove( n );
                            mpRedlineTable->Insert( pRedl );

                            bMerged = true;
                            bDelete = true;
                        }
                        else if ( POS_OUTSIDE == eCmpPos )
                        {
                            // own insert-over-insert redlines:
                            // just scrap the inside ones
                            mpRedlineTable->DeleteAndDestroy( n );
                            bDec = true;
                        }
                        else if( POS_OVERLAP_BEHIND == eCmpPos )
                        {
                            *pStt = *pREnd;
                            if( ( *pStt == *pEnd ) &&
                                ( pNewRedl->GetContentIdx() == nullptr ) )
                                bDelete = true;
                        }
                        else if( POS_OVERLAP_BEFORE == eCmpPos )
                        {
                            *pEnd = *pRStt;
                            if( ( *pStt == *pEnd ) &&
                                ( pNewRedl->GetContentIdx() == nullptr ) )
                                bDelete = true;
                        }
                        else if( POS_INSIDE == eCmpPos || POS_EQUAL == eCmpPos)
                            bDelete = true;

                        if( bDelete )
                        {
                            delete pNewRedl;
                            pNewRedl = nullptr;
                            bCompress = true;
                        }
                    }
                    else if( POS_INSIDE == eCmpPos )
                    {
                        // split up
                        if( *pEnd != *pREnd )
                        {
                            SwRangeRedline* pCpy = new SwRangeRedline( *pRedl );
                            pCpy->SetStart( *pEnd );
                            mpRedlineTable->Insert( pCpy );
                        }
                        pRedl->SetEnd( *pStt, pREnd );
                        if( ( *pStt == *pRStt ) &&
                            ( pRedl->GetContentIdx() == nullptr ) )
                        {
                            mpRedlineTable->DeleteAndDestroy( n );
                            bDec = true;
                        }
                        else if( !pRedl->HasValidRange() )
                        {
                            // re-insert
                            mpRedlineTable->Remove( n );
                            mpRedlineTable->Insert( pRedl );
                        }
                    }
                    else if ( POS_OUTSIDE == eCmpPos )
                    {
                        // handle overlapping redlines in broken documents

                        // split up the new redline, since it covers the
                        // existing redline. Insert the first part, and
                        // progress with the remainder as usual
                        SwRangeRedline* pSplit = new SwRangeRedline( *pNewRedl );
                        pSplit->SetEnd( *pRStt );
                        pNewRedl->SetStart( *pREnd );
                        mpRedlineTable->Insert( pSplit );
                        if( *pStt == *pEnd && pNewRedl->GetContentIdx() == nullptr )
                        {
                            delete pNewRedl;
                            pNewRedl = nullptr;
                            bCompress = true;
                        }
                    }
                    else if ( POS_OVERLAP_BEHIND == eCmpPos )
                    {
                        // handle overlapping redlines in broken documents
                        pNewRedl->SetStart( *pREnd );
                    }
                    else if ( POS_OVERLAP_BEFORE == eCmpPos )
                    {
                        // handle overlapping redlines in broken documents
                        *pEnd = *pRStt;
                        if( ( *pStt == *pEnd ) &&
                            ( pNewRedl->GetContentIdx() == nullptr ) )
                        {
                            delete pNewRedl;
                            pNewRedl = nullptr;
                            bCompress = true;
                        }
                    }
                    break;
                case nsRedlineType_t::REDLINE_DELETE:
                    if( POS_INSIDE == eCmpPos )
                    {
                        // split up
                        if( *pEnd != *pREnd )
                        {
                            SwRangeRedline* pCpy = new SwRangeRedline( *pRedl );
                            pCpy->SetStart( *pEnd );
                            mpRedlineTable->Insert( pCpy );
                        }
                        pRedl->SetEnd( *pStt, pREnd );
                        if( ( *pStt == *pRStt ) &&
                            ( pRedl->GetContentIdx() == nullptr ) )
                        {
                            mpRedlineTable->DeleteAndDestroy( n );
                            bDec = true;
                        }
                        else if( !pRedl->HasValidRange() )
                        {
                            // re-insert
                            mpRedlineTable->Remove( n );
                            mpRedlineTable->Insert( pRedl, n );
                        }
                    }
                    else if ( POS_OUTSIDE == eCmpPos )
                    {
                        // handle overlapping redlines in broken documents

                        // split up the new redline, since it covers the
                        // existing redline. Insert the first part, and
                        // progress with the remainder as usual
                        SwRangeRedline* pSplit = new SwRangeRedline( *pNewRedl );
                        pSplit->SetEnd( *pRStt );
                        pNewRedl->SetStart( *pREnd );
                        mpRedlineTable->Insert( pSplit );
                        if( *pStt == *pEnd && pNewRedl->GetContentIdx() == nullptr )
                        {
                            delete pNewRedl;
                            pNewRedl = nullptr;
                            bCompress = true;
                        }
                    }
                    else if ( POS_EQUAL == eCmpPos )
                    {
                        // handle identical redlines in broken documents
                        // delete old (delete) redline
                        mpRedlineTable->DeleteAndDestroy( n );
                        bDec = true;
                    }
                    else if ( POS_OVERLAP_BEHIND == eCmpPos )
                    {   // Another workaround for broken redlines
                        pNewRedl->SetStart( *pREnd );
                    }
                    break;
                case nsRedlineType_t::REDLINE_FORMAT:
                    switch( eCmpPos )
                    {
                    case POS_OVERLAP_BEFORE:
                        pRedl->SetStart( *pEnd, pRStt );
                        // re-insert
                        mpRedlineTable->Remove( n );
                        mpRedlineTable->Insert( pRedl, n );
                        bDec = true;
                        break;

                    case POS_OVERLAP_BEHIND:
                        pRedl->SetEnd( *pStt, pREnd );
                        if( *pStt == *pRStt && pRedl->GetContentIdx() == nullptr )
                        {
                            mpRedlineTable->DeleteAndDestroy( n );
                            bDec = true;
                        }
                        break;

                    case POS_EQUAL:
                    case POS_OUTSIDE:
                        // Overlaps the current one completely or has the
                        // same dimension, delete the old one
                        mpRedlineTable->DeleteAndDestroy( n );
                        bDec = true;
                        break;

                    case POS_INSIDE:
                        // Overlaps the current one completely,
                        // split or shorten the new one
                        if( *pEnd != *pREnd )
                        {
                            if( *pEnd != *pRStt )
                            {
                                SwRangeRedline* pNew = new SwRangeRedline( *pRedl );
                                pNew->SetStart( *pEnd );
                                pRedl->SetEnd( *pStt, pREnd );
                                if( *pStt == *pRStt && pRedl->GetContentIdx() == nullptr )
                                    mpRedlineTable->DeleteAndDestroy( n );
                                AppendRedline( pNew, bCallDelete );
                                n = 0;      // re-initialize
                                bDec = true;
                            }
                        }
                        else
                            pRedl->SetEnd( *pStt, pREnd );
                        break;
                    default:
                        break;
                    }
                    break;
                default:
                    break;
                }
                break;

            case nsRedlineType_t::REDLINE_DELETE:
                switch( pRedl->GetType() )
                {
                case nsRedlineType_t::REDLINE_DELETE:
                    switch( eCmpPos )
                    {
                    case POS_OUTSIDE:
                        {
                            // Overlaps the current one completely,
                            // split the new one
                            if( *pEnd != *pREnd )
                            {
                                SwRangeRedline* pNew = new SwRangeRedline( *pNewRedl );
                                pNew->SetStart( *pREnd );
                                pNewRedl->SetEnd( *pRStt, pEnd );
                                AppendRedline( pNew, bCallDelete );
                                n = 0;      // re-initialize
                                bDec = true;
                            }
                            else
                                pNewRedl->SetEnd( *pRStt, pEnd );
                        }
                        break;

                    case POS_INSIDE:
                    case POS_EQUAL:
                        delete pNewRedl;
                        pNewRedl = nullptr;
                        bCompress = true;
                        break;

                    case POS_OVERLAP_BEFORE:
                    case POS_OVERLAP_BEHIND:
                        if( pRedl->IsOwnRedline( *pNewRedl ) &&
                            pRedl->CanCombine( *pNewRedl ))
                        {
                            // If that's the case we can merge it, meaning
                            // the new one covers this well
                            if( POS_OVERLAP_BEHIND == eCmpPos )
                                pNewRedl->SetStart( *pRStt, pStt );
                            else
                                pNewRedl->SetEnd( *pREnd, pEnd );
                            mpRedlineTable->DeleteAndDestroy( n );
                            bDec = true;
                        }
                        else if( POS_OVERLAP_BEHIND == eCmpPos )
                            pNewRedl->SetStart( *pREnd, pStt );
                        else
                            pNewRedl->SetEnd( *pRStt, pEnd );
                        break;

                    case POS_COLLIDE_START:
                    case POS_COLLIDE_END:
                        if( pRedl->IsOwnRedline( *pNewRedl ) &&
                            pRedl->CanCombine( *pNewRedl ) )
                        {
                            if( IsHideChanges( meRedlineMode ))
                            {
                                // Before we can merge, we make it visible!
                                // We insert temporarily so that pNew is
                                // also dealt with when moving the indices.
                                mpRedlineTable->Insert(pNewRedl);
                                pRedl->Show(0, mpRedlineTable->GetPos(pRedl));
                                mpRedlineTable->Remove( pNewRedl );
                                pRStt = pRedl->Start();
                                pREnd = pRedl->End();
                            }

                            // If that's the case we can merge it, meaning
                            // the new one covers this well
                            if( POS_COLLIDE_START == eCmpPos )
                                pNewRedl->SetStart( *pRStt, pStt );
                            else
                                pNewRedl->SetEnd( *pREnd, pEnd );

                            // delete current (below), and restart process with
                            // previous
                            sal_uInt16 nToBeDeleted = n;
                            bDec = true;

                            if( *(pNewRedl->Start()) <= *pREnd )
                            {
                                // Whoooah, we just extended the new 'redline'
                                // beyond previous redlines, so better start
                                // again. Of course this is not supposed to
                                // happen, and in an ideal world it doesn't,
                                // but unfortunately this code is buggy and
                                // totally rotten so it does happen and we
                                // better fix it.
                                n = 0;
                                bDec = true;
                            }

                            mpRedlineTable->DeleteAndDestroy( nToBeDeleted );
                        }
                        break;
                    default:
                        break;
                    }
                    break;

                case nsRedlineType_t::REDLINE_INSERT:
                {
                    // b62341295: Do not throw away redlines
                    // even if they are not allowed to be combined
                    RedlineMode_t eOld = meRedlineMode;
                    if( !( eOld & nsRedlineMode_t::REDLINE_DONTCOMBINE_REDLINES ) &&
                        pRedl->IsOwnRedline( *pNewRedl ) )
                    {

              // Set to NONE, so that the Delete::Redo merges the Redline data correctly!
              // The ShowMode needs to be retained!
              meRedlineMode = (RedlineMode_t)(eOld & ~(nsRedlineMode_t::REDLINE_ON | nsRedlineMode_t::REDLINE_IGNORE));
                        switch( eCmpPos )
                        {
                        case POS_EQUAL:
                            bCompress = true;
                            mpRedlineTable->DeleteAndDestroy( n );
                            bDec = true;
                            // no break!

                        case POS_INSIDE:
                            if( bCallDelete )
                            {
                              meRedlineMode = (RedlineMode_t)(meRedlineMode | nsRedlineMode_t::REDLINE_IGNOREDELETE_REDLINES);

                                // DeleteAndJoin does not yield the
                                // desired result if there is no paragraph to
                                // join with, i.e. at the end of the document.
                                // For this case, we completely delete the
                                // paragraphs (if, of course, we also start on
                                // a paragraph boundary).
                                if( (pStt->nContent == 0) &&
                                    pEnd->nNode.GetNode().IsEndNode() )
                                {
                                    pEnd->nNode--;
                                    pEnd->nContent.Assign(
                                        pEnd->nNode.GetNode().GetTextNode(), 0);
                                    m_rDoc.getIDocumentContentOperations().DelFullPara( *pNewRedl );
                                }
                                else
                                    m_rDoc.getIDocumentContentOperations().DeleteAndJoin( *pNewRedl );

                                bCompress = true;
                            }
                            delete pNewRedl;
                            pNewRedl = nullptr;
                            break;

                        case POS_OUTSIDE:
                            {
                                mpRedlineTable->Remove( n );
                                bDec = true;
                                // We insert temporarily so that pNew is
                                // also dealt with when moving the indices.
                                if( bCallDelete )
                                {
                                    mpRedlineTable->Insert( pNewRedl );
                                    m_rDoc.getIDocumentContentOperations().DeleteAndJoin( *pRedl );
                                    if( !mpRedlineTable->Remove( pNewRedl ) )
                                        pNewRedl = nullptr;
                                }
                                delete pRedl;
                            }
                            break;

                        case POS_OVERLAP_BEFORE:
                            {
                                SwPaM aPam( *pRStt, *pEnd );

                                if( *pEnd == *pREnd )
                                    mpRedlineTable->DeleteAndDestroy( n );
                                else
                                {
                                    pRedl->SetStart( *pEnd, pRStt );
                                    // re-insert
                                    mpRedlineTable->Remove( n );
                                    mpRedlineTable->Insert( pRedl, n );
                                }

                                if( bCallDelete )
                                {
                                    // We insert temporarily so that pNew is
                                    // also dealt with when moving the indices.
                                    mpRedlineTable->Insert( pNewRedl );
                                    m_rDoc.getIDocumentContentOperations().DeleteAndJoin( aPam );
                                    if( !mpRedlineTable->Remove( pNewRedl ) )
                                        pNewRedl = nullptr;
                                    n = 0;      // re-initialize
                                }
                                bDec = true;
                            }
                            break;

                        case POS_OVERLAP_BEHIND:
                            {
                                SwPaM aPam( *pStt, *pREnd );

                                if( *pStt == *pRStt )
                                {
                                    mpRedlineTable->DeleteAndDestroy( n );
                                    bDec = true;
                                }
                                else
                                    pRedl->SetEnd( *pStt, pREnd );

                                if( bCallDelete )
                                {
                                    // We insert temporarily so that pNew is
                                    // also dealt with when moving the indices.
                                    mpRedlineTable->Insert( pNewRedl );
                                    m_rDoc.getIDocumentContentOperations().DeleteAndJoin( aPam );
                                    if( !mpRedlineTable->Remove( pNewRedl ) )
                                        pNewRedl = nullptr;
                                    n = 0;      // re-initialize
                                    bDec = true;
                                }
                            }
                            break;
                        default:
                            break;
                        }

                        meRedlineMode = eOld;
                    }
                    else
                    {
                        // it may be necessary to split the existing redline in
                        // two. In this case, pRedl will be changed to cover
                        // only part of its former range, and pNew will cover
                        // the remainder.
                        SwRangeRedline* pNew = nullptr;

                        switch( eCmpPos )
                        {
                        case POS_EQUAL:
                            {
                                pRedl->PushData( *pNewRedl );
                                delete pNewRedl;
                                pNewRedl = nullptr;
                                if( IsHideChanges( meRedlineMode ))
                                {
                                    pRedl->Hide(0, mpRedlineTable->GetPos(pRedl));
                                }
                                bCompress = true;
                            }
                            break;

                        case POS_INSIDE:
                            {
                                if( *pRStt == *pStt )
                                {
                                    // #i97421#
                                    // redline w/out extent loops
                                    if (*pStt != *pEnd)
                                    {
                                        pNewRedl->PushData( *pRedl, false );
                                        pRedl->SetStart( *pEnd, pRStt );
                                        // re-insert
                                        mpRedlineTable->Remove( n );
                                        mpRedlineTable->Insert( pRedl, n );
                                        bDec = true;
                                    }
                                }
                                else
                                {
                                    pNewRedl->PushData( *pRedl, false );
                                    if( *pREnd != *pEnd )
                                    {
                                        pNew = new SwRangeRedline( *pRedl );
                                        pNew->SetStart( *pEnd );
                                    }
                                    pRedl->SetEnd( *pStt, pREnd );
                                    if( !pRedl->HasValidRange() )
                                    {
                                        // re-insert
                                        mpRedlineTable->Remove( n );
                                        mpRedlineTable->Insert( pRedl, n );
                                    }
                                }
                            }
                            break;

                        case POS_OUTSIDE:
                            {
                                pRedl->PushData( *pNewRedl );
                                if( *pEnd == *pREnd )
                                    pNewRedl->SetEnd( *pRStt, pEnd );
                                else
                                {
                                    pNew = new SwRangeRedline( *pNewRedl );
                                    pNew->SetEnd( *pRStt );
                                    pNewRedl->SetStart( *pREnd, pStt );
                                }
                                bCompress = true;
                            }
                            break;

                        case POS_OVERLAP_BEFORE:
                            {
                                if( *pEnd == *pREnd )
                                {
                                    pRedl->PushData( *pNewRedl );
                                    pNewRedl->SetEnd( *pRStt, pEnd );
                                    if( IsHideChanges( meRedlineMode ))
                                    {
                                        mpRedlineTable->Insert(pNewRedl);
                                        pRedl->Hide(0, mpRedlineTable->GetPos(pRedl));
                                        mpRedlineTable->Remove( pNewRedl );
                                    }
                                }
                                else
                                {
                                    pNew = new SwRangeRedline( *pRedl );
                                    pNew->PushData( *pNewRedl );
                                    pNew->SetEnd( *pEnd );
                                    pNewRedl->SetEnd( *pRStt, pEnd );
                                    pRedl->SetStart( *pNew->End(), pRStt ) ;
                                    // re-insert
                                    mpRedlineTable->Remove( n );
                                    mpRedlineTable->Insert( pRedl );
                                    bDec = true;
                                }
                            }
                            break;

                        case POS_OVERLAP_BEHIND:
                            {
                                if( *pStt == *pRStt )
                                {
                                    pRedl->PushData( *pNewRedl );
                                    pNewRedl->SetStart( *pREnd, pStt );
                                    if( IsHideChanges( meRedlineMode ))
                                    {
                                        mpRedlineTable->Insert( pNewRedl );
                                        pRedl->Hide(0, mpRedlineTable->GetPos(pRedl));
                                        mpRedlineTable->Remove( pNewRedl );
                                    }
                                }
                                else
                                {
                                    pNew = new SwRangeRedline( *pRedl );
                                    pNew->PushData( *pNewRedl );
                                    pNew->SetStart( *pStt );
                                    pNewRedl->SetStart( *pREnd, pStt );
                                    pRedl->SetEnd( *pNew->Start(), pREnd );
                                    if( !pRedl->HasValidRange() )
                                    {
                                        // re-insert
                                        mpRedlineTable->Remove( n );
                                        mpRedlineTable->Insert( pRedl );
                                    }
                                }
                            }
                            break;
                        default:
                            break;
                        }

                        // insert the pNew part (if it exists)
                        if( pNew )
                        {
                            mpRedlineTable->Insert( pNew );

                            // pNew must be deleted if Insert() wasn't
                            // successful. But that can't happen, since pNew is
                            // part of the original pRedl redline.
                            // OSL_ENSURE( bRet, "Can't insert existing redline?" );

                            // restart (now with pRedl being split up)
                            n = 0;
                            bDec = true;
                        }
                    }
                }
                break;

                case nsRedlineType_t::REDLINE_FORMAT:
                    switch( eCmpPos )
                    {
                    case POS_OVERLAP_BEFORE:
                        pRedl->SetStart( *pEnd, pRStt );
                        // re-insert
                        mpRedlineTable->Remove( n );
                        mpRedlineTable->Insert( pRedl, n );
                        bDec = true;
                        break;

                    case POS_OVERLAP_BEHIND:
                        pRedl->SetEnd( *pStt, pREnd );
                        break;

                    case POS_EQUAL:
                    case POS_OUTSIDE:
                        // Overlaps the current one completely or has the
                        // same dimension, delete the old one
                        mpRedlineTable->DeleteAndDestroy( n );
                        bDec = true;
                        break;

                    case POS_INSIDE:
                        // Overlaps the current one completely,
                        // split or shorten the new one
                        if( *pEnd != *pREnd )
                        {
                            if( *pEnd != *pRStt )
                            {
                                SwRangeRedline* pNew = new SwRangeRedline( *pRedl );
                                pNew->SetStart( *pEnd );
                                pRedl->SetEnd( *pStt, pREnd );
                                if( ( *pStt == *pRStt ) &&
                                    ( pRedl->GetContentIdx() == nullptr ) )
                                    mpRedlineTable->DeleteAndDestroy( n );
                                AppendRedline( pNew, bCallDelete );
                                n = 0;      // re-initialize
                                bDec = true;
                            }
                        }
                        else
                            pRedl->SetEnd( *pStt, pREnd );
                        break;
                    default:
                        break;
                    }
                    break;
                default:
                    break;
                }
                break;

            case nsRedlineType_t::REDLINE_FORMAT:
                switch( pRedl->GetType() )
                {
                case nsRedlineType_t::REDLINE_INSERT:
                case nsRedlineType_t::REDLINE_DELETE:
                    switch( eCmpPos )
                    {
                    case POS_OVERLAP_BEFORE:
                        pNewRedl->SetEnd( *pRStt, pEnd );
                        break;

                    case POS_OVERLAP_BEHIND:
                        pNewRedl->SetStart( *pREnd, pStt );
                        break;

                    case POS_EQUAL:
                    case POS_INSIDE:
                        delete pNewRedl;
                        pNewRedl = nullptr;
                        break;

                    case POS_OUTSIDE:
                        // Overlaps the current one completely,
                        // split or shorten the new one
                        if( *pEnd != *pREnd )
                        {
                            if( *pEnd != *pRStt )
                            {
                                SwRangeRedline* pNew = new SwRangeRedline( *pNewRedl );
                                pNew->SetStart( *pREnd );
                                pNewRedl->SetEnd( *pRStt, pEnd );
                                AppendRedline( pNew, bCallDelete );
                                n = 0;      // re-initialize
                                bDec = true;
                            }
                        }
                        else
                            pNewRedl->SetEnd( *pRStt, pEnd );
                        break;
                    default:
                        break;
                    }
                    break;
                case nsRedlineType_t::REDLINE_FORMAT:
                    switch( eCmpPos )
                    {
                    case POS_OUTSIDE:
                    case POS_EQUAL:
                        {
                            // Overlaps the current one completely or has the
                            // same dimension, delete the old one
                            mpRedlineTable->DeleteAndDestroy( n );
                            bDec = true;
                        }
                        break;

                    case POS_INSIDE:
                        if( pRedl->IsOwnRedline( *pNewRedl ) &&
                            pRedl->CanCombine( *pNewRedl ))
                        {
                            // own one can be ignored completely
                            delete pNewRedl;
                            pNewRedl = nullptr;
                        }
                        else if( *pREnd == *pEnd )
                            // or else only shorten the current one
                            pRedl->SetEnd( *pStt, pREnd );
                        else if( *pRStt == *pStt )
                        {
                            // or else only shorten the current one
                            pRedl->SetStart( *pEnd, pRStt );
                            // re-insert
                            mpRedlineTable->Remove( n );
                            mpRedlineTable->Insert( pRedl, n );
                            bDec = true;
                        }
                        else
                        {
                            // If it lies completely within the current one
                            // we need to split it
                            SwRangeRedline* pNew = new SwRangeRedline( *pRedl );
                            pNew->SetStart( *pEnd );
                            pRedl->SetEnd( *pStt, pREnd );
                            AppendRedline( pNew, bCallDelete );
                            n = 0;      // re-initialize
                            bDec = true;
                        }
                        break;

                    case POS_OVERLAP_BEFORE:
                    case POS_OVERLAP_BEHIND:
                        if( pRedl->IsOwnRedline( *pNewRedl ) &&
                            pRedl->CanCombine( *pNewRedl ))
                        {
                            // If that's the case we can merge it, meaning
                            // the new one covers this well
                            if( POS_OVERLAP_BEHIND == eCmpPos )
                                pNewRedl->SetStart( *pRStt, pStt );
                            else
                                pNewRedl->SetEnd( *pREnd, pEnd );
                            mpRedlineTable->DeleteAndDestroy( n );
                            bDec = false;
                        }
                        else if( POS_OVERLAP_BEHIND == eCmpPos )
                            pNewRedl->SetStart( *pREnd, pStt );
                        else
                            pNewRedl->SetEnd( *pRStt, pEnd );
                        break;

                    case POS_COLLIDE_END:
                        if( pRedl->IsOwnRedline( *pNewRedl ) &&
                            pRedl->CanCombine( *pNewRedl ) && n &&
                            *(*mpRedlineTable)[ n-1 ]->End() < *pStt )
                        {
                            // If that's the case we can merge it, meaning
                            // the new one covers this well
                            pNewRedl->SetEnd( *pREnd, pEnd );
                            mpRedlineTable->DeleteAndDestroy( n );
                            bDec = true;
                        }
                        break;
                    case POS_COLLIDE_START:
                        if( pRedl->IsOwnRedline( *pNewRedl ) &&
                            pRedl->CanCombine( *pNewRedl ) &&
                            n+1 < (sal_uInt16)mpRedlineTable->size() &&
                            *(*mpRedlineTable)[ n+1 ]->Start() < *pEnd )
                        {
                            // If that's the case we can merge it, meaning
                            // the new one covers this well
                            pNewRedl->SetStart( *pRStt, pStt );
                            mpRedlineTable->DeleteAndDestroy( n );
                            bDec = true;
                        }
                        break;
                    default:
                        break;
                    }
                    break;
                default:
                    break;
                }
                break;

            case nsRedlineType_t::REDLINE_FMTCOLL:
                // How should we behave here?
                // insert as is
                break;
            default:
                break;
            }
        }

        if( pNewRedl )
        {
            if( ( *pStt == *pEnd ) &&
                ( pNewRedl->GetContentIdx() == nullptr ) )
            {   // Do not insert empty redlines
                delete pNewRedl;
                pNewRedl = nullptr;
            }
            else
                mpRedlineTable->Insert( pNewRedl );
        }

        if( bCompress )
            CompressRedlines();
    }
    else
    {
        if( bCallDelete && nsRedlineType_t::REDLINE_DELETE == pNewRedl->GetType() )
        {
            RedlineMode_t eOld = meRedlineMode;
            // Set to NONE, so that the Delete::Redo merges the Redline data correctly!
            // The ShowMode needs to be retained!
            meRedlineMode = (RedlineMode_t)(eOld & ~(nsRedlineMode_t::REDLINE_ON | nsRedlineMode_t::REDLINE_IGNORE));
            m_rDoc.getIDocumentContentOperations().DeleteAndJoin( *pNewRedl );
            meRedlineMode = eOld;
        }
        delete pNewRedl;
        pNewRedl = nullptr;
    }
    _CHECK_REDLINE( *this )

    return ( nullptr != pNewRedl ) || bMerged;
}

bool DocumentRedlineManager::AppendTableRowRedline( SwTableRowRedline* pNewRedl, bool bCallDelete )
{
    (void)bCallDelete;

    // #TODO - equivalent for 'SwTableRowRedline'
    bool bMerged = false;
    /*
    _CHECK_REDLINE( this )
    */

    if (IsRedlineOn() && !IsShowOriginal(meRedlineMode))
    {
        // #TODO - equivalent for 'SwTableRowRedline'
        /*
        pNewRedl->InvalidateRange();
        */

        // Make equivalent of 'AppendRedline' checks inside here too

        mpExtraRedlineTable->Insert( pNewRedl );
    }
    else
    {
        // TO DO - equivalent for 'SwTableRowRedline'
        /*
        if( bCallDelete && nsRedlineType_t::REDLINE_DELETE == pNewRedl->GetType() )
        {
            RedlineMode_t eOld = meRedlineMode;
            // Set to NONE, so that the Delete::Redo merges the Redline data correctly!
            // The ShowMode needs to be retained!
            meRedlineMode = (RedlineMode_t)(eOld & ~(nsRedlineMode_t::REDLINE_ON | nsRedlineMode_t::REDLINE_IGNORE));
            DeleteAndJoin( *pNewRedl );
            meRedlineMode = eOld;
        }
        delete pNewRedl, pNewRedl = 0;
        */
    }
    // #TODO - equivalent for 'SwTableRowRedline'
    /*
    _CHECK_REDLINE( this )
    */

    return ( nullptr != pNewRedl ) || bMerged;
}

bool DocumentRedlineManager::AppendTableCellRedline( SwTableCellRedline* pNewRedl, bool bCallDelete )
{
    (void)bCallDelete;

    // #TODO - equivalent for 'SwTableCellRedline'
    bool bMerged = false;
    /*
    _CHECK_REDLINE( this )
    */

    if (IsRedlineOn() && !IsShowOriginal(meRedlineMode))
    {
        // #TODO - equivalent for 'SwTableCellRedline'
        /*
        pNewRedl->InvalidateRange();
        */

        // Make equivalent of 'AppendRedline' checks inside here too

        mpExtraRedlineTable->Insert( pNewRedl );
    }
    else
    {
        // TO DO - equivalent for 'SwTableCellRedline'
        /*
        if( bCallDelete && nsRedlineType_t::REDLINE_DELETE == pNewRedl->GetType() )
        {
            RedlineMode_t eOld = meRedlineMode;
            // Set to NONE, so that the Delete::Redo merges the Redline data correctly!
            // The ShowMode needs to be retained!
            meRedlineMode = (RedlineMode_t)(eOld & ~(nsRedlineMode_t::REDLINE_ON | nsRedlineMode_t::REDLINE_IGNORE));
            DeleteAndJoin( *pNewRedl );
            meRedlineMode = eOld;
        }
        delete pNewRedl, pNewRedl = 0;
        */
    }
    // #TODO - equivalent for 'SwTableCellRedline'
    /*
    _CHECK_REDLINE( this )
    */

    return ( nullptr != pNewRedl ) || bMerged;
}

void DocumentRedlineManager::CompressRedlines()
{
    _CHECK_REDLINE( *this )

    void (SwRangeRedline::*pFnc)(sal_uInt16, size_t) = nullptr;
    switch( nsRedlineMode_t::REDLINE_SHOW_MASK & meRedlineMode )
    {
    case nsRedlineMode_t::REDLINE_SHOW_INSERT | nsRedlineMode_t::REDLINE_SHOW_DELETE:
        pFnc = &SwRangeRedline::Show;
        break;
    case nsRedlineMode_t::REDLINE_SHOW_INSERT:
        pFnc = &SwRangeRedline::Hide;
        break;
    }

    // Try to merge identical ones
    for( size_t n = 1; n < mpRedlineTable->size(); ++n )
    {
        SwRangeRedline* pPrev = (*mpRedlineTable)[ n-1 ],
                    * pCur = (*mpRedlineTable)[ n ];
        const SwPosition* pPrevStt = pPrev->Start(),
                        * pPrevEnd = pPrevStt == pPrev->GetPoint()
                            ? pPrev->GetMark() : pPrev->GetPoint();
        const SwPosition* pCurStt = pCur->Start(),
                        * pCurEnd = pCurStt == pCur->GetPoint()
                            ? pCur->GetMark() : pCur->GetPoint();
        if( *pPrevEnd == *pCurStt && pPrev->CanCombine( *pCur ) &&
            pPrevStt->nNode.GetNode().StartOfSectionNode() ==
            pCurEnd->nNode.GetNode().StartOfSectionNode() &&
            !pCurEnd->nNode.GetNode().StartOfSectionNode()->IsTableNode() )
        {
            // we then can merge them
            size_t nPrevIndex = n-1;
            pPrev->Show(0, nPrevIndex);
            pCur->Show(0, n);

            pPrev->SetEnd( *pCur->End() );
            mpRedlineTable->DeleteAndDestroy( n );
            --n;
            if( pFnc )
                (pPrev->*pFnc)(0, nPrevIndex);
        }
    }
    _CHECK_REDLINE( *this )

    // #TODO - add 'SwExtraRedlineTable' also ?
}

bool DocumentRedlineManager::SplitRedline( const SwPaM& rRange )
{
    bool bChg = false;
    sal_uInt16 n = 0;
    const SwPosition* pStt = rRange.Start();
    const SwPosition* pEnd = rRange.End();
    GetRedline( *pStt, &n );
    for ( ; n < mpRedlineTable->size(); ++n)
    {
        SwRangeRedline * pRedline = (*mpRedlineTable)[ n ];
        SwPosition *const pRedlineStart = pRedline->Start();
        SwPosition *const pRedlineEnd = pRedline->End();
        if (*pRedlineStart <= *pStt && *pStt <= *pRedlineEnd &&
            *pRedlineStart <= *pEnd && *pEnd <= *pRedlineEnd)
        {
            bChg = true;
            int nn = 0;
            if (*pStt == *pRedlineStart)
                nn += 1;
            if (*pEnd == *pRedlineEnd)
                nn += 2;

            SwRangeRedline* pNew = nullptr;
            switch( nn )
            {
            case 0:
                pNew = new SwRangeRedline( *pRedline );
                pRedline->SetEnd( *pStt, pRedlineEnd );
                pNew->SetStart( *pEnd );
                break;

            case 1:
                *pRedlineStart = *pEnd;
                break;

            case 2:
                *pRedlineEnd = *pStt;
                break;

            case 3:
                pRedline->InvalidateRange();
                mpRedlineTable->DeleteAndDestroy( n-- );
                pRedline = nullptr;
                break;
            }
            if (pRedline && !pRedline->HasValidRange())
            {
                // re-insert
                mpRedlineTable->Remove( n );
                mpRedlineTable->Insert( pRedline, n );
            }
            if( pNew )
                mpRedlineTable->Insert( pNew, n );
        }
        else if (*pEnd < *pRedlineStart)
            break;
    }
    return bChg;

    // #TODO - add 'SwExtraRedlineTable' also ?
}

bool DocumentRedlineManager::DeleteRedline( const SwPaM& rRange, bool bSaveInUndo,
                            sal_uInt16 nDelType )
{
    if( nsRedlineMode_t::REDLINE_IGNOREDELETE_REDLINES & meRedlineMode ||
        !rRange.HasMark() || *rRange.GetMark() == *rRange.GetPoint() )
        return false;

    bool bChg = false;

    if (bSaveInUndo && m_rDoc.GetIDocumentUndoRedo().DoesUndo())
    {
        SwUndoRedline* pUndo = new SwUndoRedline( UNDO_REDLINE, rRange );
        if( pUndo->GetRedlSaveCount() )
        {
            m_rDoc.GetIDocumentUndoRedo().AppendUndo(pUndo);
        }
        else
            delete pUndo;
    }

    const SwPosition* pStt = rRange.Start(),
                    * pEnd = pStt == rRange.GetPoint() ? rRange.GetMark()
                                                       : rRange.GetPoint();
    sal_uInt16 n = 0;
    GetRedline( *pStt, &n );
    for( ; n < mpRedlineTable->size() ; ++n )
    {
        SwRangeRedline* pRedl = (*mpRedlineTable)[ n ];
        if( USHRT_MAX != nDelType && nDelType != pRedl->GetType() )
            continue;

        SwPosition* pRStt = pRedl->Start(),
                  * pREnd = pRStt == pRedl->GetPoint() ? pRedl->GetMark()
                                                       : pRedl->GetPoint();
        switch( ComparePosition( *pStt, *pEnd, *pRStt, *pREnd ) )
        {
        case POS_EQUAL:
        case POS_OUTSIDE:
            pRedl->InvalidateRange();
            mpRedlineTable->DeleteAndDestroy( n-- );
            bChg = true;
            break;

        case POS_OVERLAP_BEFORE:
                pRedl->InvalidateRange();
                pRedl->SetStart( *pEnd, pRStt );
                // re-insert
                mpRedlineTable->Remove( n );
                mpRedlineTable->Insert( pRedl );
                --n;
            break;

        case POS_OVERLAP_BEHIND:
                pRedl->InvalidateRange();
                pRedl->SetEnd( *pStt, pREnd );
                if( !pRedl->HasValidRange() )
                {
                    // re-insert
                    mpRedlineTable->Remove( n );
                    mpRedlineTable->Insert( pRedl );
                    --n;
                }
            break;

        case POS_INSIDE:
            {
                // this one needs to be splitted
                pRedl->InvalidateRange();
                if( *pRStt == *pStt )
                {
                    pRedl->SetStart( *pEnd, pRStt );
                    // re-insert
                    mpRedlineTable->Remove( n );
                    mpRedlineTable->Insert( pRedl );
                    --n;
                }
                else
                {
                    SwRangeRedline* pCpy;
                    if( *pREnd != *pEnd )
                    {
                        pCpy = new SwRangeRedline( *pRedl );
                        pCpy->SetStart( *pEnd );
                    }
                    else
                        pCpy = nullptr;
                    pRedl->SetEnd( *pStt, pREnd );
                    if( !pRedl->HasValidRange() )
                    {
                        // re-insert
                        mpRedlineTable->Remove( n );
                        mpRedlineTable->Insert( pRedl );
                        --n;
                    }
                    if( pCpy )
                        mpRedlineTable->Insert( pCpy );
                }
            }
            break;

        case POS_COLLIDE_END:
        case POS_BEFORE:
            n = mpRedlineTable->size();
            break;
        default:
            break;
        }
    }

    if( bChg )
        m_rDoc.getIDocumentState().SetModified();

    return bChg;

    // #TODO - add 'SwExtraRedlineTable' also ?
}

bool DocumentRedlineManager::DeleteRedline( const SwStartNode& rNode, bool bSaveInUndo,
                            sal_uInt16 nDelType )
{
    SwPaM aTemp(*rNode.EndOfSectionNode(), rNode);
    return DeleteRedline(aTemp, bSaveInUndo, nDelType);
}

sal_uInt16 DocumentRedlineManager::GetRedlinePos( const SwNode& rNd, sal_uInt16 nType ) const
{
    const sal_uLong nNdIdx = rNd.GetIndex();
    for( size_t n = 0; n < mpRedlineTable->size() ; ++n )
    {
        const SwRangeRedline* pTmp = (*mpRedlineTable)[ n ];
        sal_uLong nPt = pTmp->GetPoint()->nNode.GetIndex(),
              nMk = pTmp->GetMark()->nNode.GetIndex();
        if( nPt < nMk ) { long nTmp = nMk; nMk = nPt; nPt = nTmp; }

        if( ( USHRT_MAX == nType || nType == pTmp->GetType()) &&
            nMk <= nNdIdx && nNdIdx <= nPt )
            return n;

        if( nMk > nNdIdx )
            break;
    }
    return USHRT_MAX;

    // #TODO - add 'SwExtraRedlineTable' also ?
}

const SwRangeRedline* DocumentRedlineManager::GetRedline( const SwPosition& rPos,
                                    sal_uInt16* pFndPos ) const
{
    sal_uInt16 nO = mpRedlineTable->size(), nM, nU = 0;
    if( nO > 0 )
    {
        nO--;
        while( nU <= nO )
        {
            nM = nU + ( nO - nU ) / 2;
            const SwRangeRedline* pRedl = (*mpRedlineTable)[ nM ];
            const SwPosition* pStt = pRedl->Start();
            const SwPosition* pEnd = pStt == pRedl->GetPoint()
                                        ? pRedl->GetMark()
                                        : pRedl->GetPoint();
            if( pEnd == pStt
                    ? *pStt == rPos
                    : ( *pStt <= rPos && rPos < *pEnd ) )
            {
                while( nM && rPos == *(*mpRedlineTable)[ nM - 1 ]->End() &&
                    rPos == *(*mpRedlineTable)[ nM - 1 ]->Start() )
                {
                    --nM;
                    pRedl = (*mpRedlineTable)[ nM ];
                }
                // if there are format and insert changes in the same position
                // show insert change first.
                // since the redlines are sorted by position, only check the redline
                // before and after the current redline
                if( nsRedlineType_t::REDLINE_FORMAT == pRedl->GetType() )
                {
                    if( nM && rPos >= *(*mpRedlineTable)[ nM - 1 ]->Start() &&
                        rPos <= *(*mpRedlineTable)[ nM - 1 ]->End() &&
                        ( nsRedlineType_t::REDLINE_INSERT == (*mpRedlineTable)[ nM - 1 ]->GetType() ) )
                    {
                        --nM;
                        pRedl = (*mpRedlineTable)[ nM ];
                    }
                    else if( ( nM + 1 ) <= nO && rPos >= *(*mpRedlineTable)[ nM + 1 ]->Start() &&
                        rPos <= *(*mpRedlineTable)[ nM + 1 ]->End() &&
                        ( nsRedlineType_t::REDLINE_INSERT == (*mpRedlineTable)[ nM + 1 ]->GetType() ) )
                    {
                        ++nM;
                        pRedl = (*mpRedlineTable)[ nM ];
                    }
                }

                if( pFndPos )
                    *pFndPos = nM;
                return pRedl;
            }
            else if( *pEnd <= rPos )
                nU = nM + 1;
            else if( nM == 0 )
            {
                if( pFndPos )
                    *pFndPos = nU;
                return nullptr;
            }
            else
                nO = nM - 1;
        }
    }
    if( pFndPos )
        *pFndPos = nU;
    return nullptr;

    // #TODO - add 'SwExtraRedlineTable' also ?
}

bool DocumentRedlineManager::AcceptRedline( sal_uInt16 nPos, bool bCallDelete )
{
    bool bRet = false;

    // Switch to visible in any case
    if( (nsRedlineMode_t::REDLINE_SHOW_INSERT | nsRedlineMode_t::REDLINE_SHOW_DELETE) !=
        (nsRedlineMode_t::REDLINE_SHOW_MASK & meRedlineMode) )
      SetRedlineMode( (RedlineMode_t)(nsRedlineMode_t::REDLINE_SHOW_INSERT | nsRedlineMode_t::REDLINE_SHOW_DELETE | meRedlineMode));

    SwRangeRedline* pTmp = (*mpRedlineTable)[ nPos ];
    if( pTmp->HasMark() && pTmp->IsVisible() )
    {
        if (m_rDoc.GetIDocumentUndoRedo().DoesUndo())
        {
            SwRewriter aRewriter;

            aRewriter.AddRule(UndoArg1, pTmp->GetDescr());
            m_rDoc.GetIDocumentUndoRedo().StartUndo(UNDO_ACCEPT_REDLINE, &aRewriter);
        }

        int nLoopCnt = 2;
        sal_uInt16 nSeqNo = pTmp->GetSeqNo();

        do {

            if (m_rDoc.GetIDocumentUndoRedo().DoesUndo())
            {
                SwUndo *const pUndo( new SwUndoAcceptRedline(*pTmp) );
                m_rDoc.GetIDocumentUndoRedo().AppendUndo(pUndo);
            }

            bRet |= lcl_AcceptRedline( *mpRedlineTable, nPos, bCallDelete );

            if( nSeqNo )
            {
                if( USHRT_MAX == nPos )
                    nPos = 0;
                sal_uInt16 nFndPos = 2 == nLoopCnt
                                    ? mpRedlineTable->FindNextSeqNo( nSeqNo, nPos )
                                    : mpRedlineTable->FindPrevSeqNo( nSeqNo, nPos );
                if( USHRT_MAX != nFndPos || ( 0 != ( --nLoopCnt ) &&
                    USHRT_MAX != ( nFndPos =
                        mpRedlineTable->FindPrevSeqNo( nSeqNo, nPos ))) )
                    pTmp = (*mpRedlineTable)[ nPos = nFndPos ];
                else
                    nLoopCnt = 0;
            }
            else
                nLoopCnt = 0;

        } while( nLoopCnt );

        if( bRet )
        {
            CompressRedlines();
            m_rDoc.getIDocumentState().SetModified();
        }

        if (m_rDoc.GetIDocumentUndoRedo().DoesUndo())
        {
            m_rDoc.GetIDocumentUndoRedo().EndUndo(UNDO_END, nullptr);
        }
    }
    return bRet;

    // #TODO - add 'SwExtraRedlineTable' also ?
}

bool DocumentRedlineManager::AcceptRedline( const SwPaM& rPam, bool bCallDelete )
{
    // Switch to visible in any case
    if( (nsRedlineMode_t::REDLINE_SHOW_INSERT | nsRedlineMode_t::REDLINE_SHOW_DELETE) !=
        (nsRedlineMode_t::REDLINE_SHOW_MASK & meRedlineMode) )
      SetRedlineMode( (RedlineMode_t)(nsRedlineMode_t::REDLINE_SHOW_INSERT | nsRedlineMode_t::REDLINE_SHOW_DELETE | meRedlineMode));

    // The Selection is only in the ContentSection. If there are Redlines
    // to Non-ContentNodes before or after that, then the Selections
    // expand to them.
    SwPaM aPam( *rPam.GetMark(), *rPam.GetPoint() );
    lcl_AdjustRedlineRange( aPam );

    if (m_rDoc.GetIDocumentUndoRedo().DoesUndo())
    {
        m_rDoc.GetIDocumentUndoRedo().StartUndo( UNDO_ACCEPT_REDLINE, nullptr );
        m_rDoc.GetIDocumentUndoRedo().AppendUndo( new SwUndoAcceptRedline( aPam ));
    }

    int nRet = lcl_AcceptRejectRedl( lcl_AcceptRedline, *mpRedlineTable,
                                     bCallDelete, aPam );
    if( nRet > 0 )
    {
        CompressRedlines();
        m_rDoc.getIDocumentState().SetModified();
    }
    if (m_rDoc.GetIDocumentUndoRedo().DoesUndo())
    {
        OUString aTmpStr;

        {
            SwRewriter aRewriter;
            aRewriter.AddRule(UndoArg1, OUString::number(nRet));
            aTmpStr = aRewriter.Apply(OUString(SW_RES(STR_N_REDLINES)));
        }

        SwRewriter aRewriter;
        aRewriter.AddRule(UndoArg1, aTmpStr);

        m_rDoc.GetIDocumentUndoRedo().EndUndo( UNDO_ACCEPT_REDLINE, &aRewriter );
    }
    return nRet != 0;

    // #TODO - add 'SwExtraRedlineTable' also ?
}

bool DocumentRedlineManager::RejectRedline( sal_uInt16 nPos, bool bCallDelete )
{
    bool bRet = false;

    // Switch to visible in any case
    if( (nsRedlineMode_t::REDLINE_SHOW_INSERT | nsRedlineMode_t::REDLINE_SHOW_DELETE) !=
        (nsRedlineMode_t::REDLINE_SHOW_MASK & meRedlineMode) )
      SetRedlineMode( (RedlineMode_t)(nsRedlineMode_t::REDLINE_SHOW_INSERT | nsRedlineMode_t::REDLINE_SHOW_DELETE | meRedlineMode));

    SwRangeRedline* pTmp = (*mpRedlineTable)[ nPos ];
    if( pTmp->HasMark() && pTmp->IsVisible() )
    {
        if (m_rDoc.GetIDocumentUndoRedo().DoesUndo())
        {
            SwRewriter aRewriter;

            aRewriter.AddRule(UndoArg1, pTmp->GetDescr());
            m_rDoc.GetIDocumentUndoRedo().StartUndo(UNDO_REJECT_REDLINE, &aRewriter);
        }

        int nLoopCnt = 2;
        sal_uInt16 nSeqNo = pTmp->GetSeqNo();

        do {

            if (m_rDoc.GetIDocumentUndoRedo().DoesUndo())
            {
                SwUndo *const pUndo( new SwUndoRejectRedline( *pTmp ) );
                m_rDoc.GetIDocumentUndoRedo().AppendUndo(pUndo);
            }

            bRet |= lcl_RejectRedline( *mpRedlineTable, nPos, bCallDelete );

            if( nSeqNo )
            {
                if( USHRT_MAX == nPos )
                    nPos = 0;
                sal_uInt16 nFndPos = 2 == nLoopCnt
                                    ? mpRedlineTable->FindNextSeqNo( nSeqNo, nPos )
                                    : mpRedlineTable->FindPrevSeqNo( nSeqNo, nPos );
                if( USHRT_MAX != nFndPos || ( 0 != ( --nLoopCnt ) &&
                    USHRT_MAX != ( nFndPos =
                            mpRedlineTable->FindPrevSeqNo( nSeqNo, nPos ))) )
                    pTmp = (*mpRedlineTable)[ nPos = nFndPos ];
                else
                    nLoopCnt = 0;
            }
            else
                nLoopCnt = 0;

        } while( nLoopCnt );

        if( bRet )
        {
            CompressRedlines();
            m_rDoc.getIDocumentState().SetModified();
        }

        if (m_rDoc.GetIDocumentUndoRedo().DoesUndo())
        {
            m_rDoc.GetIDocumentUndoRedo().EndUndo(UNDO_END, nullptr);
        }
    }
    return bRet;

    // #TODO - add 'SwExtraRedlineTable' also ?
}

bool DocumentRedlineManager::RejectRedline( const SwPaM& rPam, bool bCallDelete )
{
    // Switch to visible in any case
    if( (nsRedlineMode_t::REDLINE_SHOW_INSERT | nsRedlineMode_t::REDLINE_SHOW_DELETE) !=
        (nsRedlineMode_t::REDLINE_SHOW_MASK & meRedlineMode) )
      SetRedlineMode((RedlineMode_t)(nsRedlineMode_t::REDLINE_SHOW_INSERT | nsRedlineMode_t::REDLINE_SHOW_DELETE | meRedlineMode));

    // The Selection is only in the ContentSection. If there are Redlines
    // to Non-ContentNodes before or after that, then the Selections
    // expand to them.
    SwPaM aPam( *rPam.GetMark(), *rPam.GetPoint() );
    lcl_AdjustRedlineRange( aPam );

    if (m_rDoc.GetIDocumentUndoRedo().DoesUndo())
    {
        m_rDoc.GetIDocumentUndoRedo().StartUndo( UNDO_REJECT_REDLINE, nullptr );
        m_rDoc.GetIDocumentUndoRedo().AppendUndo( new SwUndoRejectRedline(aPam) );
    }

    int nRet = lcl_AcceptRejectRedl( lcl_RejectRedline, *mpRedlineTable,
                                        bCallDelete, aPam );
    if( nRet > 0 )
    {
        CompressRedlines();
        m_rDoc.getIDocumentState().SetModified();
    }
    if (m_rDoc.GetIDocumentUndoRedo().DoesUndo())
    {
        OUString aTmpStr;

        {
            SwRewriter aRewriter;
            aRewriter.AddRule(UndoArg1, OUString::number(nRet));
            aTmpStr = aRewriter.Apply(OUString(SW_RES(STR_N_REDLINES)));
        }

        SwRewriter aRewriter;
        aRewriter.AddRule(UndoArg1, aTmpStr);

        m_rDoc.GetIDocumentUndoRedo().EndUndo( UNDO_REJECT_REDLINE, &aRewriter );
    }

    return nRet != 0;

    // #TODO - add 'SwExtraRedlineTable' also ?
}

const SwRangeRedline* DocumentRedlineManager::SelNextRedline( SwPaM& rPam ) const
{
    rPam.DeleteMark();
    rPam.SetMark();

    SwPosition& rSttPos = *rPam.GetPoint();
    SwPosition aSavePos( rSttPos );
    bool bRestart;

    // If the starting position points to the last valid ContentNode,
    // we take the next Redline in any case.
    sal_uInt16 n = 0;
    const SwRangeRedline* pFnd = GetRedlineTable().FindAtPosition( rSttPos, n );
    if( pFnd )
    {
        const SwPosition* pEnd = pFnd->End();
        if( !pEnd->nNode.GetNode().IsContentNode() )
        {
            SwNodeIndex aTmp( pEnd->nNode );
            SwContentNode* pCNd = SwNodes::GoPrevSection( &aTmp );
            if( !pCNd || ( aTmp == rSttPos.nNode &&
                pCNd->Len() == rSttPos.nContent.GetIndex() ))
                pFnd = nullptr;
        }
        if( pFnd )
            rSttPos = *pFnd->End();
    }

    do {
        bRestart = false;

        for( ; !pFnd && n < mpRedlineTable->size(); ++n )
        {
            pFnd = (*mpRedlineTable)[ n ];
            if( pFnd->HasMark() && pFnd->IsVisible() )
            {
                *rPam.GetMark() = *pFnd->Start();
                rSttPos = *pFnd->End();
                break;
            }
            else
                pFnd = nullptr;
        }

        if( pFnd )
        {
            // Merge all of the same type and author that are
            // consecutive into one Selection.
            const SwPosition* pPrevEnd = pFnd->End();
            while( ++n < mpRedlineTable->size() )
            {
                const SwRangeRedline* pTmp = (*mpRedlineTable)[ n ];
                if( pTmp->HasMark() && pTmp->IsVisible() )
                {
                    const SwPosition *pRStt;
                    if( pFnd->GetType() == pTmp->GetType() &&
                        pFnd->GetAuthor() == pTmp->GetAuthor() &&
                        ( *pPrevEnd == *( pRStt = pTmp->Start() ) ||
                          IsPrevPos( *pPrevEnd, *pRStt )) )
                    {
                        pPrevEnd = pTmp->End();
                        rSttPos = *pPrevEnd;
                    }
                    else
                        break;
                }
            }
        }

        if( pFnd )
        {
            const SwRangeRedline* pSaveFnd = pFnd;

            SwContentNode* pCNd;
            SwNodeIndex* pIdx = &rPam.GetMark()->nNode;
            if( !pIdx->GetNode().IsContentNode() &&
                nullptr != ( pCNd = m_rDoc.GetNodes().GoNextSection( pIdx )) )
            {
                if( *pIdx <= rPam.GetPoint()->nNode )
                    rPam.GetMark()->nContent.Assign( pCNd, 0 );
                else
                    pFnd = nullptr;
            }

            if( pFnd )
            {
                pIdx = &rPam.GetPoint()->nNode;
                if( !pIdx->GetNode().IsContentNode() &&
                    nullptr != ( pCNd = SwNodes::GoPrevSection( pIdx )) )
                {
                    if( *pIdx >= rPam.GetMark()->nNode )
                        rPam.GetPoint()->nContent.Assign( pCNd, pCNd->Len() );
                    else
                        pFnd = nullptr;
                }
            }

            if( !pFnd || *rPam.GetMark() == *rPam.GetPoint() )
            {
                if( n < mpRedlineTable->size() )
                {
                    bRestart = true;
                    *rPam.GetPoint() = *pSaveFnd->End();
                }
                else
                {
                    rPam.DeleteMark();
                    *rPam.GetPoint() = aSavePos;
                }
                pFnd = nullptr;
            }
        }
    } while( bRestart );

    return pFnd;

    // #TODO - add 'SwExtraRedlineTable' also ?
}

const SwRangeRedline* DocumentRedlineManager::SelPrevRedline( SwPaM& rPam ) const
{
    rPam.DeleteMark();
    rPam.SetMark();

    SwPosition& rSttPos = *rPam.GetPoint();
    SwPosition aSavePos( rSttPos );
    bool bRestart;

    // If the starting position points to the last valid ContentNode,
    // we take the previous Redline in any case.
    sal_uInt16 n = 0;
    const SwRangeRedline* pFnd = GetRedlineTable().FindAtPosition( rSttPos, n, false );
    if( pFnd )
    {
        const SwPosition* pStt = pFnd->Start();
        if( !pStt->nNode.GetNode().IsContentNode() )
        {
            SwNodeIndex aTmp( pStt->nNode );
            SwContentNode* pCNd = m_rDoc.GetNodes().GoNextSection( &aTmp );
            if( !pCNd || ( aTmp == rSttPos.nNode &&
                !rSttPos.nContent.GetIndex() ))
                pFnd = nullptr;
        }
        if( pFnd )
            rSttPos = *pFnd->Start();
    }

    do {
        bRestart = false;

        while( !pFnd && 0 < n )
        {
            pFnd = (*mpRedlineTable)[ --n ];
            if( pFnd->HasMark() && pFnd->IsVisible() )
            {
                *rPam.GetMark() = *pFnd->End();
                rSttPos = *pFnd->Start();
            }
            else
                pFnd = nullptr;
        }

        if( pFnd )
        {
            // Merge all of the same type and author that are
            // consecutive into one Selection.
            const SwPosition* pNextStt = pFnd->Start();
            while( 0 < n )
            {
                const SwRangeRedline* pTmp = (*mpRedlineTable)[ --n ];
                if( pTmp->HasMark() && pTmp->IsVisible() )
                {
                    const SwPosition *pREnd;
                    if( pFnd->GetType() == pTmp->GetType() &&
                        pFnd->GetAuthor() == pTmp->GetAuthor() &&
                        ( *pNextStt == *( pREnd = pTmp->End() ) ||
                          IsPrevPos( *pREnd, *pNextStt )) )
                    {
                        pNextStt = pTmp->Start();
                        rSttPos = *pNextStt;
                    }
                    else
                    {
                        ++n;
                        break;
                    }
                }
            }
        }

        if( pFnd )
        {
            const SwRangeRedline* pSaveFnd = pFnd;

            SwContentNode* pCNd;
            SwNodeIndex* pIdx = &rPam.GetMark()->nNode;
            if( !pIdx->GetNode().IsContentNode() &&
                nullptr != ( pCNd = SwNodes::GoPrevSection( pIdx )) )
            {
                if( *pIdx >= rPam.GetPoint()->nNode )
                    rPam.GetMark()->nContent.Assign( pCNd, pCNd->Len() );
                else
                    pFnd = nullptr;
            }

            if( pFnd )
            {
                pIdx = &rPam.GetPoint()->nNode;
                if( !pIdx->GetNode().IsContentNode() &&
                    nullptr != ( pCNd = m_rDoc.GetNodes().GoNextSection( pIdx )) )
                {
                    if( *pIdx <= rPam.GetMark()->nNode )
                        rPam.GetPoint()->nContent.Assign( pCNd, 0 );
                    else
                        pFnd = nullptr;
                }
            }

            if( !pFnd || *rPam.GetMark() == *rPam.GetPoint() )
            {
                if( n )
                {
                    bRestart = true;
                    *rPam.GetPoint() = *pSaveFnd->Start();
                }
                else
                {
                    rPam.DeleteMark();
                    *rPam.GetPoint() = aSavePos;
                }
                pFnd = nullptr;
            }
        }
    } while( bRestart );

    return pFnd;

    // #TODO - add 'SwExtraRedlineTable' also ?
}

// Set comment at the Redline
bool DocumentRedlineManager::SetRedlineComment( const SwPaM& rPaM, const OUString& rS )
{
    bool bRet = false;
    const SwPosition* pStt = rPaM.Start(),
                    * pEnd = pStt == rPaM.GetPoint() ? rPaM.GetMark()
                                                     : rPaM.GetPoint();
    sal_uInt16 n = 0;
    if( GetRedlineTable().FindAtPosition( *pStt, n ) )
    {
        for( ; n < mpRedlineTable->size(); ++n )
        {
            bRet = true;
            SwRangeRedline* pTmp = (*mpRedlineTable)[ n ];
            if( pStt != pEnd && *pTmp->Start() > *pEnd )
                break;

            pTmp->SetComment( rS );
            if( *pTmp->End() >= *pEnd )
                break;
        }
    }
    if( bRet )
        m_rDoc.getIDocumentState().SetModified();

    return bRet;

    // #TODO - add 'SwExtraRedlineTable' also ?
}

// Create a new author if necessary
sal_uInt16 DocumentRedlineManager::GetRedlineAuthor()
{
    return SW_MOD()->GetRedlineAuthor();
}

/// Insert new author into the Table for the Readers etc.
sal_uInt16 DocumentRedlineManager::InsertRedlineAuthor( const OUString& rNew )
{
    return SW_MOD()->InsertRedlineAuthor(rNew);
}

void DocumentRedlineManager::UpdateRedlineAttr()
{
    const SwRedlineTable& rTable = GetRedlineTable();
    for( size_t n = 0; n < rTable.size(); ++n )
    {
        SwRangeRedline* pRedl = rTable[ n ];
        if( pRedl->IsVisible() )
            pRedl->InvalidateRange();
    }

    // #TODO - add 'SwExtraRedlineTable' also ?
}

const uno::Sequence <sal_Int8>& DocumentRedlineManager::GetRedlinePassword() const
{
    return maRedlinePasswd;
}

void DocumentRedlineManager::SetRedlinePassword(
            /*[in]*/const uno::Sequence <sal_Int8>& rNewPassword)
{
    maRedlinePasswd = rNewPassword;
    m_rDoc.getIDocumentState().SetModified();
}

/// Set comment text for the Redline, which is inserted later on via
/// AppendRedline. Is used by Autoformat.
/// A null pointer resets the mode. The pointer is not copied, so it
/// needs to stay valid!
void DocumentRedlineManager::SetAutoFormatRedlineComment( const OUString* pText, sal_uInt16 nSeqNo )
{
    m_rDoc.SetAutoFormatRedline( nullptr != pText );
    if( pText )
    {
        if( !mpAutoFormatRedlnComment )
            mpAutoFormatRedlnComment = new OUString( *pText );
        else
            *mpAutoFormatRedlnComment = *pText;
    }
    else
    {
        delete mpAutoFormatRedlnComment;
        mpAutoFormatRedlnComment = nullptr;
    }

    mnAutoFormatRedlnCommentNo = nSeqNo;
}

#define MAX_REDLINE_COUNT   250

void DocumentRedlineManager::checkRedlining(RedlineMode_t& _rReadlineMode)
{
    const SwRedlineTable& rRedlineTable = GetRedlineTable();
    SwEditShell* pEditShell = m_rDoc.GetEditShell();
    vcl::Window* pParent = pEditShell ? pEditShell->GetWin() : nullptr;
    if ( pParent && !mbReadlineChecked && rRedlineTable.size() > MAX_REDLINE_COUNT
        && !((_rReadlineMode & nsRedlineMode_t::REDLINE_SHOW_DELETE) == nsRedlineMode_t::REDLINE_SHOW_DELETE) )
    {
        ScopedVclPtrInstance< MessageDialog > aQuery(pParent, "QueryShowChangesDialog", "modules/swriter/ui/queryshowchangesdialog.ui");
        sal_uInt16 nResult = aQuery->Execute();
        mbReadlineChecked = true;
        if ( nResult == RET_YES )
        {
            sal_Int32 nMode = (sal_Int32)_rReadlineMode;
            nMode |= nsRedlineMode_t::REDLINE_SHOW_INSERT | nsRedlineMode_t::REDLINE_SHOW_DELETE;
            _rReadlineMode = (RedlineMode_t)nMode;
        }
    }
}

DocumentRedlineManager::~DocumentRedlineManager()
{
    delete mpRedlineTable; mpRedlineTable = nullptr;
    delete mpExtraRedlineTable; mpExtraRedlineTable = nullptr;
    delete mpAutoFormatRedlnComment; mpAutoFormatRedlnComment = nullptr;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
