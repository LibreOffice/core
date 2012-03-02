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



#include <hintids.hxx>
#include <tools/shl.hxx>
#include <svl/itemiter.hxx>
#include <sfx2/app.hxx>
#include <editeng/colritem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/crsditem.hxx>
#include <swmodule.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <docary.hxx>
#include <ndtxt.hxx>
#include <redline.hxx>
#include <swundo.hxx>
#include <UndoCore.hxx>
#include <UndoRedline.hxx>
#include <hints.hxx>
#include <pamtyp.hxx>
#include <poolfmt.hxx>
#include <viewsh.hxx>
#include <rootfrm.hxx>

#include <comcore.hrc>

using namespace com::sun::star;

TYPEINIT1(SwRedlineHint, SfxHint);

#ifdef DBG_UTIL

    #define _ERROR_PREFIX "redline table corrupted: "

    // helper function for lcl_CheckRedline
    // 1. make sure that pPos->nContent points into pPos->nNode
    //    (or into the 'special' no-content-node-IndexReg)
    // 2. check that position is valid and doesn't point behind text
    void lcl_CheckPosition( const SwPosition* pPos )
    {
        SwPosition aComparePos( *pPos );
        aComparePos.nContent.Assign(
            aComparePos.nNode.GetNode().GetCntntNode(), 0 );
        OSL_ENSURE( pPos->nContent.GetIdxReg() ==
                    aComparePos.nContent.GetIdxReg(),
                    _ERROR_PREFIX "illegal position" );

        SwTxtNode* pTxtNode = pPos->nNode.GetNode().GetTxtNode();
        if( pTxtNode == NULL )
        {
            OSL_ENSURE( pPos->nContent == 0,
                        _ERROR_PREFIX "non-text-node with content" );
        }
        else
        {
            OSL_ENSURE( pPos->nContent >= 0  &&
                        pPos->nContent <= pTxtNode->Len(),
                        _ERROR_PREFIX "index behind text" );
        }
    }

    void lcl_CheckPam( const SwPaM* pPam )
    {
        OSL_ENSURE( pPam != NULL, _ERROR_PREFIX "illegal argument" );
        lcl_CheckPosition( pPam->GetPoint() );
        lcl_CheckPosition( pPam->GetMark() );
    }

    // check validity of the redline table. Checks redline bounds, and make
    // sure the redlines are sorted and non-overlapping.
    void lcl_CheckRedline( const SwDoc* pDoc )
    {
        const SwRedlineTbl& rTbl = pDoc->GetRedlineTbl();

        // verify valid redline positions
        for( sal_uInt16 i = 0; i < rTbl.Count(); ++i )
            lcl_CheckPam( rTbl[ i ] );

        for( sal_uInt16 j = 0; j < rTbl.Count(); ++j )
        {
            // check for empty redlines
            OSL_ENSURE( ( *(rTbl[j]->GetPoint()) != *(rTbl[j]->GetMark()) ) ||
                        ( rTbl[j]->GetContentIdx() != NULL ),
                        _ERROR_PREFIX "empty redline" );
         }

        // verify proper redline sorting
        for( sal_uInt16 n = 1; n < rTbl.Count(); ++n )
        {
            const SwRedline* pPrev = rTbl[ n-1 ];
            const SwRedline* pCurrent = rTbl[ n ];

            // check redline sorting
            OSL_ENSURE( *pPrev->Start() <= *pCurrent->Start(),
                        _ERROR_PREFIX "not sorted correctly" );

            // check for overlapping redlines
            OSL_ENSURE( *pPrev->End() <= *pCurrent->Start(),
                        _ERROR_PREFIX "overlapping redlines" );
        }
    }

    #define _CHECK_REDLINE( pDoc ) lcl_CheckRedline( pDoc );

    void lcl_DebugRedline( const SwDoc* pDoc )
    {
        static sal_uInt16 nWatch = 0;
        const SwRedlineTbl& rTbl = pDoc->GetRedlineTbl();
        for( sal_uInt16 n = 0; n < rTbl.Count(); ++n )
        {
            sal_uInt16 nDummy = 0;
            const SwRedline* pCurrent = rTbl[ n ];
            const SwRedline* pNext = n+1 < rTbl.Count() ? rTbl[ n+1 ] : 0;
            if( pCurrent == pNext )
                ++nDummy;
            if( n == nWatch )
                ++nDummy; // Possible debugger breakpoint
        }
    }

    #define _DEBUG_REDLINE( pDoc ) lcl_DebugRedline( pDoc );


#else

    #define _CHECK_REDLINE( pDoc )
    #define _DEBUG_REDLINE( pDoc )

#endif

SV_IMPL_OP_PTRARR_SORT( _SwRedlineTbl, SwRedlinePtr )

inline bool IsPrevPos( const SwPosition rPos1, const SwPosition rPos2 )
{
    const SwCntntNode* pCNd;
    return 0 == rPos2.nContent.GetIndex() &&
            rPos2.nNode.GetIndex() - 1 == rPos1.nNode.GetIndex() &&
            0 != ( pCNd = rPos1.nNode.GetNode().GetCntntNode() )
                ? rPos1.nContent.GetIndex() == pCNd->Len()
                : false;
}

#if OSL_DEBUG_LEVEL > 0
bool CheckPosition( const SwPosition* pStt, const SwPosition* pEnd )
{
    int nError = 0;
    SwNode* pSttNode = &pStt->nNode.GetNode();
    SwNode* pEndNode = &pEnd->nNode.GetNode();
    SwNode* pSttTab = pSttNode->StartOfSectionNode()->FindTableNode();
    SwNode* pEndTab = pEndNode->StartOfSectionNode()->FindTableNode();
    SwNode* pSttStart = pSttNode;
    while( pSttStart && (!pSttStart->IsStartNode() || pSttStart->IsSectionNode() ||
        pSttStart->IsTableNode() ) )
        pSttStart = pSttStart->StartOfSectionNode();
    SwNode* pEndStart = pEndNode;
    while( pEndStart && (!pEndStart->IsStartNode() || pEndStart->IsSectionNode() ||
        pEndStart->IsTableNode() ) )
        pEndStart = pEndStart->StartOfSectionNode();
    if( pSttTab != pEndTab )
        nError = 1;
    if( !pSttTab && pSttStart != pEndStart )
        nError |= 2;
    if( nError )
        nError += 10;
    return nError != 0;
}
#endif

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

typedef sal_Bool (*Fn_AcceptReject)( SwRedlineTbl& rArr, sal_uInt16& rPos,
                        sal_Bool bCallDelete,
                        const SwPosition* pSttRng,
                        const SwPosition* pEndRng);

sal_Bool lcl_AcceptRedline( SwRedlineTbl& rArr, sal_uInt16& rPos,
                        sal_Bool bCallDelete,
                        const SwPosition* pSttRng = 0,
                        const SwPosition* pEndRng = 0 )
{
    sal_Bool bRet = sal_True;
    SwRedline* pRedl = rArr[ rPos ];
    SwPosition *pRStt = 0, *pREnd = 0;
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
            sal_Bool bCheck = sal_False, bReplace = sal_False;
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
                        SwRedline* pNew = new SwRedline( *pRedl );
                        pNew->SetStart( *pEndRng );
                        rArr.Insert( pNew ); ++rPos;
                    }
                    pRedl->SetEnd( *pSttRng, pREnd );
                    bCheck = sal_True;
                }
                break;

            case POS_OVERLAP_BEFORE:
                pRedl->SetStart( *pEndRng, pRStt );
                bReplace = sal_True;
                break;

            case POS_OVERLAP_BEHIND:
                pRedl->SetEnd( *pSttRng, pREnd );
                bCheck = sal_True;
                break;

            case POS_OUTSIDE:
            case POS_EQUAL:
                rArr.DeleteAndDestroy( rPos-- );
                break;

            default:
                bRet = sal_False;
            }

            if( bReplace || ( bCheck && !pRedl->HasValidRange() ))
            {
                // re-insert
                rArr.Remove( rArr.GetPos( pRedl ));
                rArr.Insert( pRedl );
            }
        }
        break;
    case nsRedlineType_t::REDLINE_DELETE:
        {
            SwDoc& rDoc = *pRedl->GetDoc();
            const SwPosition *pDelStt = 0, *pDelEnd = 0;
            sal_Bool bDelRedl = sal_False;
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
                    bDelRedl = sal_True;
                    if( bCallDelete )
                    {
                        pDelStt = pRedl->Start();
                        pDelEnd = pRedl->End();
                    }
                }
                break;
            default:
                bRet = sal_False;
            }

            if( pDelStt && pDelEnd )
            {
                SwPaM aPam( *pDelStt, *pDelEnd );
                SwCntntNode* pCSttNd = pDelStt->nNode.GetNode().GetCntntNode();
                SwCntntNode* pCEndNd = pDelEnd->nNode.GetNode().GetCntntNode();

                if( bDelRedl )
                    delete pRedl;

                RedlineMode_t eOld = rDoc.GetRedlineMode();
                rDoc.SetRedlineMode_intern( (RedlineMode_t)(eOld & ~(nsRedlineMode_t::REDLINE_ON | nsRedlineMode_t::REDLINE_IGNORE)));

                if( pCSttNd && pCEndNd )
                    rDoc.DeleteAndJoin( aPam );
                else
                {
                    rDoc.DeleteRange( aPam );

                    if( pCSttNd && !pCEndNd )
                    {
                        aPam.GetBound( sal_True ).nContent.Assign( 0, 0 );
                        aPam.GetBound( sal_False ).nContent.Assign( 0, 0 );
                        aPam.DeleteMark();
                        rDoc.DelFullPara( aPam );
                    }
                }
                rDoc.SetRedlineMode_intern( eOld );
            }
            else if( bDelRedl )
                delete pRedl;
        }
        break;

    case nsRedlineType_t::REDLINE_FMTCOLL:
        rArr.DeleteAndDestroy( rPos-- );
        break;

    default:
        bRet = sal_False;
    }
    return bRet;
}

sal_Bool lcl_RejectRedline( SwRedlineTbl& rArr, sal_uInt16& rPos,
                        sal_Bool bCallDelete,
                        const SwPosition* pSttRng = 0,
                        const SwPosition* pEndRng = 0 )
{
    sal_Bool bRet = sal_True;
    SwRedline* pRedl = rArr[ rPos ];
    SwPosition *pRStt = 0, *pREnd = 0;
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
            const SwPosition *pDelStt = 0, *pDelEnd = 0;
            sal_Bool bDelRedl = sal_False;
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
                    bDelRedl = sal_True;
                    if( bCallDelete )
                    {
                        pDelStt = pRedl->Start();
                        pDelEnd = pRedl->End();
                    }
                }
                break;

            default:
                bRet = sal_False;
            }
            if( pDelStt && pDelEnd )
            {
                SwPaM aPam( *pDelStt, *pDelEnd );

                SwCntntNode* pCSttNd = pDelStt->nNode.GetNode().GetCntntNode();
                SwCntntNode* pCEndNd = pDelEnd->nNode.GetNode().GetCntntNode();

                if( bDelRedl )
                    delete pRedl;

                RedlineMode_t eOld = rDoc.GetRedlineMode();
                rDoc.SetRedlineMode_intern( (RedlineMode_t)(eOld & ~(nsRedlineMode_t::REDLINE_ON | nsRedlineMode_t::REDLINE_IGNORE)));

                if( pCSttNd && pCEndNd )
                    rDoc.DeleteAndJoin( aPam );
                else
                {
                    rDoc.DeleteRange( aPam );

                    if( pCSttNd && !pCEndNd )
                    {
                        aPam.GetBound( sal_True ).nContent.Assign( 0, 0 );
                        aPam.GetBound( sal_False ).nContent.Assign( 0, 0 );
                        aPam.DeleteMark();
                        rDoc.DelFullPara( aPam );
                    }
                }
                rDoc.SetRedlineMode_intern( eOld );
            }
            else if( bDelRedl )
                delete pRedl;
        }
        break;
    case nsRedlineType_t::REDLINE_DELETE:
        {
            SwRedline* pNew = 0;
            sal_Bool bCheck = sal_False, bReplace = sal_False;

            switch( eCmp )
            {
            case POS_INSIDE:
                {
                    if( 1 < pRedl->GetStackCount() )
                    {
                        pNew = new SwRedline( *pRedl );
                        pNew->PopData();
                    }
                    if( *pSttRng == *pRStt )
                    {
                        pRedl->SetStart( *pEndRng, pRStt );
                        bReplace = sal_True;
                        if( pNew )
                            pNew->SetEnd( *pEndRng );
                    }
                    else
                    {
                        if( *pEndRng != *pREnd )
                        {
                            // split up
                            SwRedline* pCpy = new SwRedline( *pRedl );
                            pCpy->SetStart( *pEndRng );
                            rArr.Insert( pCpy ); ++rPos;
                            if( pNew )
                                pNew->SetEnd( *pEndRng );
                        }

                        pRedl->SetEnd( *pSttRng, pREnd );
                        bCheck = sal_True;
                        if( pNew )
                            pNew->SetStart( *pSttRng );
                    }
                }
                break;

            case POS_OVERLAP_BEFORE:
                if( 1 < pRedl->GetStackCount() )
                {
                    pNew = new SwRedline( *pRedl );
                    pNew->PopData();
                }
                pRedl->SetStart( *pEndRng, pRStt );
                bReplace = sal_True;
                if( pNew )
                    pNew->SetEnd( *pEndRng );
                break;

            case POS_OVERLAP_BEHIND:
                if( 1 < pRedl->GetStackCount() )
                {
                    pNew = new SwRedline( *pRedl );
                    pNew->PopData();
                }
                pRedl->SetEnd( *pSttRng, pREnd );
                bCheck = sal_True;
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
                bRet = sal_False;
            }

            if( pNew )
            {
                rArr.Insert( pNew ); ++rPos;
            }

            if( bReplace || ( bCheck && !pRedl->HasValidRange() ))
            {
                // re-insert
                rArr.Remove( rArr.GetPos( pRedl ));
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
        bRet = sal_False;
    }
    return bRet;
}


const SwRedline* lcl_FindCurrRedline( const SwPosition& rSttPos,
                                        sal_uInt16& rPos,
                                        sal_Bool bNext = sal_True )
{
    const SwRedline* pFnd = 0;
    const SwRedlineTbl& rArr = rSttPos.nNode.GetNode().GetDoc()->GetRedlineTbl();
    for( ; rPos < rArr.Count() ; ++rPos )
    {
        const SwRedline* pTmp = rArr[ rPos ];
        if( pTmp->HasMark() && pTmp->IsVisible() )
        {
            const SwPosition* pRStt = pTmp->Start(),
                      * pREnd = pRStt == pTmp->GetPoint() ? pTmp->GetMark()
                                                          : pTmp->GetPoint();
            if( bNext ? *pRStt <= rSttPos : *pRStt < rSttPos )
            {
                if( bNext ? *pREnd > rSttPos : *pREnd >= rSttPos )
                {
                    pFnd = pTmp;
                    break;
                }
            }
            else
                break;
        }
    }
    return pFnd;
}

int lcl_AcceptRejectRedl( Fn_AcceptReject fn_AcceptReject,
                            SwRedlineTbl& rArr, sal_Bool bCallDelete,
                            const SwPaM& rPam)
{
    sal_uInt16 n = 0;
    int nCount = 0;

    const SwPosition* pStt = rPam.Start(),
                    * pEnd = pStt == rPam.GetPoint() ? rPam.GetMark()
                                                     : rPam.GetPoint();
    const SwRedline* pFnd = lcl_FindCurrRedline( *pStt, n, sal_True );
    if( pFnd &&     // Is new a part of it?
        ( *pFnd->Start() != *pStt || *pFnd->End() > *pEnd ))
    {
        // Only revoke the partial selection
        if( (*fn_AcceptReject)( rArr, n, bCallDelete, pStt, pEnd ))
            nCount++;
        ++n;
    }

    for( ; n < rArr.Count(); ++n )
    {
        SwRedline* pTmp = rArr[ n ];
        if( pTmp->HasMark() && pTmp->IsVisible() )
        {
            if( *pTmp->End() <= *pEnd )
            {
                if( (*fn_AcceptReject)( rArr, n, bCallDelete, 0, 0 ))
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

void lcl_AdjustRedlineRange( SwPaM& rPam )
{
    // The Selection is only in the ContentSection. If there are Redlines
    // to Non-ContentNodes before or after that, then the Selections
    // expand to them.
    SwPosition* pStt = rPam.Start(),
              * pEnd = pStt == rPam.GetPoint() ? rPam.GetMark()
                                               : rPam.GetPoint();
    SwDoc* pDoc = rPam.GetDoc();
    if( !pStt->nContent.GetIndex() &&
        !pDoc->GetNodes()[ pStt->nNode.GetIndex() - 1 ]->IsCntntNode() )
    {
        const SwRedline* pRedl = pDoc->GetRedline( *pStt, 0 );
        if( pRedl )
        {
            const SwPosition* pRStt = pRedl->Start();
            if( !pRStt->nContent.GetIndex() && pRStt->nNode.GetIndex() ==
                pStt->nNode.GetIndex() - 1 )
                *pStt = *pRStt;
        }
    }
    if( pEnd->nNode.GetNode().IsCntntNode() &&
        !pDoc->GetNodes()[ pEnd->nNode.GetIndex() + 1 ]->IsCntntNode() &&
        pEnd->nContent.GetIndex() == pEnd->nNode.GetNode().GetCntntNode()->Len()    )
    {
        const SwRedline* pRedl = pDoc->GetRedline( *pEnd, 0 );
        if( pRedl )
        {
            const SwPosition* pREnd = pRedl->End();
            if( !pREnd->nContent.GetIndex() && pREnd->nNode.GetIndex() ==
                pEnd->nNode.GetIndex() + 1 )
                *pEnd = *pREnd;
        }
    }
}


sal_Bool SwRedlineTbl::Insert( SwRedlinePtr& p, sal_Bool bIns )
{
    sal_Bool bRet = sal_False;
    if( p->HasValidRange() )
    {
        bRet = _SwRedlineTbl::Insert( p );
        p->CallDisplayFunc();
    }
    else if( bIns )
        bRet = InsertWithValidRanges( p );
    else
    {
        OSL_ENSURE( !this, "Redline: wrong range" );
    }
    return bRet;
}

sal_Bool SwRedlineTbl::Insert( SwRedlinePtr& p, sal_uInt16& rP, sal_Bool bIns )
{
    sal_Bool bRet = sal_False;
    if( p->HasValidRange() )
    {
        bRet = _SwRedlineTbl::Insert( p, rP );
        p->CallDisplayFunc();
    }
    else if( bIns )
        bRet = InsertWithValidRanges( p, &rP );
    else
    {
        OSL_ENSURE( !this, "Redline: wrong range" );
    }
    return bRet;
}

sal_Bool SwRedlineTbl::InsertWithValidRanges( SwRedlinePtr& p, sal_uInt16* pInsPos )
{
    // Create valid "sub-ranges" from the Selection
    sal_Bool bAnyIns = sal_False;
    SwPosition* pStt = p->Start(),
              * pEnd = pStt == p->GetPoint() ? p->GetMark() : p->GetPoint();
    SwPosition aNewStt( *pStt );
    SwNodes& rNds = aNewStt.nNode.GetNodes();
    SwCntntNode* pC;

    if( !aNewStt.nNode.GetNode().IsCntntNode() )
    {
        pC = rNds.GoNext( &aNewStt.nNode );
        if( pC )
            aNewStt.nContent.Assign( pC, 0 );
        else
            aNewStt.nNode = rNds.GetEndOfContent();
    }

    SwRedline* pNew = 0;
    sal_uInt16 nInsPos;

    if( aNewStt < *pEnd )
        do {
            if( !pNew )
                pNew = new SwRedline( p->GetRedlineData(), aNewStt );
            else
            {
                pNew->DeleteMark();
                *pNew->GetPoint() = aNewStt;
            }

            pNew->SetMark();
            GoEndSection( pNew->GetPoint() );
            // i60396: If the redlines starts before a table but the table is the last member
            // of the section, the GoEndSection will end inside the table.
            // This will result in an incorrect redline, so we've to go back
            SwNode* pTab = pNew->GetPoint()->nNode.GetNode().StartOfSectionNode()->FindTableNode();
            // We end in a table when pTab != 0
            if( pTab && !pNew->GetMark()->nNode.GetNode().StartOfSectionNode()->FindTableNode() )
            { // but our Mark was outside the table => Correction
                do
                {
                    // We want to be before the table
                    *pNew->GetPoint() = SwPosition(*pTab);
                    pC = GoPreviousNds( &pNew->GetPoint()->nNode, sal_False ); // here we are.
                    if( pC )
                        pNew->GetPoint()->nContent.Assign( pC, 0 );
                    pTab = pNew->GetPoint()->nNode.GetNode().StartOfSectionNode()->FindTableNode();
                }while( pTab ); // If there is another table we have to repeat our step backwards
            }

            if( *pNew->GetPoint() > *pEnd )
            {
                pC = 0;
                if( aNewStt.nNode != pEnd->nNode )
                    do {
                        SwNode& rCurNd = aNewStt.nNode.GetNode();
                        if( rCurNd.IsStartNode() )
                        {
                            if( rCurNd.EndOfSectionIndex() < pEnd->nNode.GetIndex() )
                                aNewStt.nNode = *rCurNd.EndOfSectionNode();
                            else
                                break;
                        }
                        else if( rCurNd.IsCntntNode() )
                            pC = rCurNd.GetCntntNode();
                        aNewStt.nNode++;
                    } while( aNewStt.nNode.GetIndex() < pEnd->nNode.GetIndex() );

                if( aNewStt.nNode == pEnd->nNode )
                    aNewStt.nContent = pEnd->nContent;
                else if( pC )
                {
                    aNewStt.nNode = *pC;
                    aNewStt.nContent.Assign( pC, pC->Len() );
                }

                if( aNewStt <= *pEnd )
                    *pNew->GetPoint() = aNewStt;
            }
            else
                aNewStt = *pNew->GetPoint();
#if OSL_DEBUG_LEVEL > 0
            CheckPosition( pNew->GetPoint(), pNew->GetMark() );
#endif
            if( *pNew->GetPoint() != *pNew->GetMark() &&
                _SwRedlineTbl::Insert( pNew, nInsPos ) )
            {
                pNew->CallDisplayFunc();
                bAnyIns = sal_True;
                pNew = 0;
                if( pInsPos && *pInsPos < nInsPos )
                    *pInsPos = nInsPos;
            }

            if( aNewStt >= *pEnd ||
                0 == (pC = rNds.GoNext( &aNewStt.nNode )) )
                break;

            aNewStt.nContent.Assign( pC, 0 );

        } while( aNewStt < *pEnd );

    delete pNew;
    delete p, p = 0;
    return bAnyIns;
}

void SwRedlineTbl::Remove( sal_uInt16 nP, sal_uInt16 nL )
{
    SwDoc* pDoc = 0;
    if( !nP && nL && nL == _SwRedlineTbl::Count() )
        pDoc = _SwRedlineTbl::GetObject( 0 )->GetDoc();

    _SwRedlineTbl::Remove( nP, nL );

    ViewShell* pSh;
    if( pDoc && !pDoc->IsInDtor() &&
        0 != ( pSh = pDoc->GetCurrentViewShell()) ) //swmod 071108//swmod 071225
        pSh->InvalidateWindows( SwRect( 0, 0, LONG_MAX, LONG_MAX ) );
}

void SwRedlineTbl::DeleteAndDestroy( sal_uInt16 nP, sal_uInt16 nL )
{
    SwDoc* pDoc = 0;
    if( !nP && nL && nL == _SwRedlineTbl::Count() )
        pDoc = _SwRedlineTbl::GetObject( 0 )->GetDoc();

    _SwRedlineTbl::DeleteAndDestroy( nP, nL );

    ViewShell* pSh;
    if( pDoc && !pDoc->IsInDtor() &&
        0 != ( pSh = pDoc->GetCurrentViewShell() ) )    //swmod 071108//swmod 071225
        pSh->InvalidateWindows( SwRect( 0, 0, LONG_MAX, LONG_MAX ) );
}

// Find the next or preceding Redline with the same seq.no.
// We can limit the search using look ahead.
// 0 or USHRT_MAX searches the whole array.
sal_uInt16 SwRedlineTbl::FindNextOfSeqNo( sal_uInt16 nSttPos, sal_uInt16 nLookahead ) const
{
    return nSttPos + 1 < _SwRedlineTbl::Count()
                ? FindNextSeqNo( _SwRedlineTbl::GetObject( nSttPos )
                                    ->GetSeqNo(), nSttPos+1, nLookahead )
                : USHRT_MAX;
}

sal_uInt16 SwRedlineTbl::FindPrevOfSeqNo( sal_uInt16 nSttPos, sal_uInt16 nLookahead ) const
{
    return nSttPos ? FindPrevSeqNo( _SwRedlineTbl::GetObject(
                                        nSttPos )->GetSeqNo(),
                                    nSttPos-1, nLookahead )
                   : USHRT_MAX;
}

sal_uInt16 SwRedlineTbl::FindNextSeqNo( sal_uInt16 nSeqNo, sal_uInt16 nSttPos,
                                    sal_uInt16 nLookahead ) const
{
    sal_uInt16 nRet = USHRT_MAX, nEnd;
    if( nSeqNo && nSttPos < _SwRedlineTbl::Count() )
    {
        nEnd = _SwRedlineTbl::Count();
        if( nLookahead && USHRT_MAX != nLookahead &&
            nSttPos + nLookahead < _SwRedlineTbl::Count() )
            nEnd = nSttPos + nLookahead;

        for( ; nSttPos < nEnd; ++nSttPos )
            if( nSeqNo == _SwRedlineTbl::GetObject( nSttPos )->GetSeqNo() )
            {
                nRet = nSttPos;
                break;
            }
    }
    return nRet;
}

sal_uInt16 SwRedlineTbl::FindPrevSeqNo( sal_uInt16 nSeqNo, sal_uInt16 nSttPos,
                                    sal_uInt16 nLookahead ) const
{
    sal_uInt16 nRet = USHRT_MAX, nEnd;
    if( nSeqNo && nSttPos < _SwRedlineTbl::Count() )
    {
        nEnd = 0;
        if( nLookahead && USHRT_MAX != nLookahead && nSttPos > nLookahead )
            nEnd = nSttPos - nLookahead;

        ++nSttPos;
        while( nSttPos > nEnd )
            if( nSeqNo == _SwRedlineTbl::GetObject( --nSttPos )->GetSeqNo() )
            {
                nRet = nSttPos;
                break;
            }
    }
    return nRet;
}


SwRedlineExtraData::~SwRedlineExtraData()
{
}

void SwRedlineExtraData::Accept( SwPaM& ) const
{
}

void SwRedlineExtraData::Reject( SwPaM& ) const
{
}

int SwRedlineExtraData::operator == ( const SwRedlineExtraData& ) const
{
    return sal_False;
}


SwRedlineExtraData_FmtColl::SwRedlineExtraData_FmtColl( const String& rColl,
                                                sal_uInt16 nPoolFmtId,
                                                const SfxItemSet* pItemSet )
    : sFmtNm(rColl), pSet(0), nPoolId(nPoolFmtId)
{
    if( pItemSet && pItemSet->Count() )
        pSet = new SfxItemSet( *pItemSet );
}

SwRedlineExtraData_FmtColl::~SwRedlineExtraData_FmtColl()
{
    delete pSet;
}

SwRedlineExtraData* SwRedlineExtraData_FmtColl::CreateNew() const
{
    return new SwRedlineExtraData_FmtColl( sFmtNm, nPoolId, pSet );
}

void SwRedlineExtraData_FmtColl::Reject( SwPaM& rPam ) const
{
    SwDoc* pDoc = rPam.GetDoc();

    // What about Undo? Is it turned off?
    SwTxtFmtColl* pColl = USHRT_MAX == nPoolId
                            ? pDoc->FindTxtFmtCollByName( sFmtNm )
                            : pDoc->GetTxtCollFromPool( nPoolId );
    if( pColl )
        pDoc->SetTxtFmtColl( rPam, pColl, false );

    if( pSet )
    {
        rPam.SetMark();
        SwPosition& rMark = *rPam.GetMark();
        SwTxtNode* pTNd = rMark.nNode.GetNode().GetTxtNode();
        if( pTNd )
        {
            rMark.nContent.Assign( pTNd, pTNd->GetTxt().Len() );

            if( pTNd->HasSwAttrSet() )
            {
                // Only set those that are not there anymore. Others
                // could have changed, but we don't touch these.
                SfxItemSet aTmp( *pSet );
                aTmp.Differentiate( *pTNd->GetpSwAttrSet() );
                pDoc->InsertItemSet( rPam, aTmp, 0 );
            }
            else
            {
                pDoc->InsertItemSet( rPam, *pSet, 0 );
            }
        }
        rPam.DeleteMark();
    }
}

int SwRedlineExtraData_FmtColl::operator == ( const SwRedlineExtraData& r) const
{
    const SwRedlineExtraData_FmtColl& rCmp = (SwRedlineExtraData_FmtColl&)r;
    return sFmtNm == rCmp.sFmtNm && nPoolId == rCmp.nPoolId &&
            ( ( !pSet && !rCmp.pSet ) ||
               ( pSet && rCmp.pSet && *pSet == *rCmp.pSet ) );
}

void SwRedlineExtraData_FmtColl::SetItemSet( const SfxItemSet& rSet )
{
    delete pSet;
    if( rSet.Count() )
        pSet = new SfxItemSet( rSet );
    else
        pSet = 0;
}


SwRedlineExtraData_Format::SwRedlineExtraData_Format( const SfxItemSet& rSet )
{
    SfxItemIter aIter( rSet );
    const SfxPoolItem* pItem = aIter.FirstItem();
    while( sal_True )
    {
        aWhichIds.push_back( pItem->Which() );
        if( aIter.IsAtEnd() )
            break;
        pItem = aIter.NextItem();
    }
}

SwRedlineExtraData_Format::SwRedlineExtraData_Format(
        const SwRedlineExtraData_Format& rCpy )
    : SwRedlineExtraData()
{
    aWhichIds.insert( aWhichIds.begin(), rCpy.aWhichIds.begin(), rCpy.aWhichIds.end() );
}

SwRedlineExtraData_Format::~SwRedlineExtraData_Format()
{
}

SwRedlineExtraData* SwRedlineExtraData_Format::CreateNew() const
{
    return new SwRedlineExtraData_Format( *this );
}

void SwRedlineExtraData_Format::Reject( SwPaM& rPam ) const
{
    SwDoc* pDoc = rPam.GetDoc();

    RedlineMode_t eOld = pDoc->GetRedlineMode();
    pDoc->SetRedlineMode_intern((RedlineMode_t)(eOld & ~(nsRedlineMode_t::REDLINE_ON | nsRedlineMode_t::REDLINE_IGNORE)));

    // Actually we need to reset the Attribute here!
    std::vector<sal_uInt16>::const_iterator it;
    for( it = aWhichIds.begin(); it != aWhichIds.end(); ++it )
    {
        pDoc->InsertPoolItem( rPam, *GetDfltAttr( *it ),
                nsSetAttrMode::SETATTR_DONTEXPAND );
    }

    pDoc->SetRedlineMode_intern( eOld );
}

int SwRedlineExtraData_Format::operator == ( const SwRedlineExtraData& rCmp ) const
{
    int nRet = 1;
    size_t nEnd = aWhichIds.size();
    if( nEnd != ((SwRedlineExtraData_Format&)rCmp).aWhichIds.size() )
        nRet = 0;
    else
        for( size_t n = 0; n < nEnd; ++n )
            if( ((SwRedlineExtraData_Format&)rCmp).aWhichIds[n] != aWhichIds[n])
            {
                nRet = 0;
                break;
            }
    return nRet;
}

SwRedlineData::SwRedlineData( RedlineType_t eT, sal_uInt16 nAut )
    : pNext( 0 ), pExtraData( 0 ),
    aStamp( DateTime::SYSTEM ),
    eType( eT ), nAuthor( nAut ), nSeqNo( 0 )
{
    aStamp.SetSec( 0 );
    aStamp.Set100Sec( 0 );
}

SwRedlineData::SwRedlineData( const SwRedlineData& rCpy, sal_Bool bCpyNext )
    :
    pNext( (bCpyNext && rCpy.pNext) ? new SwRedlineData( *rCpy.pNext ) : 0 ),
    pExtraData( rCpy.pExtraData ? rCpy.pExtraData->CreateNew() : 0 ),
    sComment( rCpy.sComment ), aStamp( rCpy.aStamp ), eType( rCpy.eType ),
    nAuthor( rCpy.nAuthor ), nSeqNo( rCpy.nSeqNo )
{
}

// For sw3io: We now own pNext!
SwRedlineData::SwRedlineData(RedlineType_t eT, sal_uInt16 nAut, const DateTime& rDT,
    const String& rCmnt, SwRedlineData *pNxt, SwRedlineExtraData* pData)
    : pNext(pNxt), pExtraData(pData), sComment(rCmnt), aStamp(rDT),
    eType(eT), nAuthor(nAut), nSeqNo(0)
{
}

SwRedlineData::~SwRedlineData()
{
    delete pExtraData;
    delete pNext;
}

// ExtraData is copied. The Pointer's ownership is thus NOT transferred
// to the Redline Object!
void SwRedlineData::SetExtraData( const SwRedlineExtraData* pData )
{
    delete pExtraData;

    if( pData )
        pExtraData = pData->CreateNew();
    else
        pExtraData = 0;
}

String SwRedlineData::GetDescr() const
{
    String aResult;

    aResult += String(SW_RES(STR_REDLINE_INSERT + GetType()));

    return aResult;
}

SwRedline::SwRedline(RedlineType_t eTyp, const SwPaM& rPam )
    : SwPaM( *rPam.GetMark(), *rPam.GetPoint() ),
    pRedlineData( new SwRedlineData( eTyp, GetDoc()->GetRedlineAuthor() ) ),
    pCntntSect( 0 )
{
    bDelLastPara = bIsLastParaDelete = sal_False;
    bIsVisible = sal_True;
    if( !rPam.HasMark() )
        DeleteMark();
}

SwRedline::SwRedline( const SwRedlineData& rData, const SwPaM& rPam )
    : SwPaM( *rPam.GetMark(), *rPam.GetPoint() ),
    pRedlineData( new SwRedlineData( rData )),
    pCntntSect( 0 )
{
    bDelLastPara = bIsLastParaDelete = sal_False;
    bIsVisible = sal_True;
    if( !rPam.HasMark() )
        DeleteMark();
}

SwRedline::SwRedline( const SwRedlineData& rData, const SwPosition& rPos )
    : SwPaM( rPos ),
    pRedlineData( new SwRedlineData( rData )),
    pCntntSect( 0 )
{
    bDelLastPara = bIsLastParaDelete = sal_False;
    bIsVisible = sal_True;
}

SwRedline::SwRedline( const SwRedline& rCpy )
    : SwPaM( *rCpy.GetMark(), *rCpy.GetPoint() ),
    pRedlineData( new SwRedlineData( *rCpy.pRedlineData )),
    pCntntSect( 0 )
{
    bDelLastPara = bIsLastParaDelete = sal_False;
    bIsVisible = sal_True;
    if( !rCpy.HasMark() )
        DeleteMark();
}

SwRedline::~SwRedline()
{
    if( pCntntSect )
    {
        // delete the ContentSection
        if( !GetDoc()->IsInDtor() )
            GetDoc()->DeleteSection( &pCntntSect->GetNode() );
        delete pCntntSect;
    }
    delete pRedlineData;
}

// Do we have a valid Selection?
sal_Bool SwRedline::HasValidRange() const
{
    const SwNode* pPtNd = &GetPoint()->nNode.GetNode(),
                * pMkNd = &GetMark()->nNode.GetNode();
    if( pPtNd->StartOfSectionNode() == pMkNd->StartOfSectionNode() &&
        !pPtNd->StartOfSectionNode()->IsTableNode() &&
        // invalid if points on the end of content
        // end-of-content only invalid if no content index exists
        ( pPtNd != pMkNd || GetContentIdx() != NULL ||
          pPtNd != &pPtNd->GetNodes().GetEndOfContent() )
        )
        return sal_True;
    return sal_False;
}

void SwRedline::CallDisplayFunc( sal_uInt16 nLoop )
{
    switch( nsRedlineMode_t::REDLINE_SHOW_MASK & GetDoc()->GetRedlineMode() )
    {
    case nsRedlineMode_t::REDLINE_SHOW_INSERT | nsRedlineMode_t::REDLINE_SHOW_DELETE:
        Show( nLoop );
        break;
    case nsRedlineMode_t::REDLINE_SHOW_INSERT:
        Hide( nLoop );
        break;
    case nsRedlineMode_t::REDLINE_SHOW_DELETE:
        ShowOriginal( nLoop );
        break;
    }
}

void SwRedline::Show( sal_uInt16 nLoop )
{
    if( 1 <= nLoop )
    {
        SwDoc* pDoc = GetDoc();
        RedlineMode_t eOld = pDoc->GetRedlineMode();
        pDoc->SetRedlineMode_intern((RedlineMode_t)(eOld | nsRedlineMode_t::REDLINE_IGNORE));
        ::sw::UndoGuard const undoGuard(pDoc->GetIDocumentUndoRedo());

        switch( GetType() )
        {
        case nsRedlineType_t::REDLINE_INSERT:           // Content has been inserted
            bIsVisible = sal_True;
            MoveFromSection();
            break;

        case nsRedlineType_t::REDLINE_DELETE:           // Content has been deleted
            bIsVisible = sal_True;
            MoveFromSection();
            break;

        case nsRedlineType_t::REDLINE_FORMAT:           // Attributes have been applied
        case nsRedlineType_t::REDLINE_TABLE:            // Table structure has been modified
            InvalidateRange();
            break;
        default:
            break;
        }
        pDoc->SetRedlineMode_intern( eOld );
    }
}

void SwRedline::Hide( sal_uInt16 nLoop )
{
    SwDoc* pDoc = GetDoc();
    RedlineMode_t eOld = pDoc->GetRedlineMode();
    pDoc->SetRedlineMode_intern((RedlineMode_t)(eOld | nsRedlineMode_t::REDLINE_IGNORE));
    ::sw::UndoGuard const undoGuard(pDoc->GetIDocumentUndoRedo());

    switch( GetType() )
    {
    case nsRedlineType_t::REDLINE_INSERT:           // Content has been inserted
        bIsVisible = sal_True;
        if( 1 <= nLoop )
            MoveFromSection();
        break;

    case nsRedlineType_t::REDLINE_DELETE:           // Content has been deleted
        bIsVisible = sal_False;
        switch( nLoop )
        {
        case 0: MoveToSection();    break;
        case 1: CopyToSection();    break;
        case 2: DelCopyOfSection(); break;
        }
        break;

    case nsRedlineType_t::REDLINE_FORMAT:           // Attributes have been applied
    case nsRedlineType_t::REDLINE_TABLE:            // Table structure has been modified
        if( 1 <= nLoop )
            InvalidateRange();
        break;
    default:
        break;
    }
    pDoc->SetRedlineMode_intern( eOld );
}

void SwRedline::ShowOriginal( sal_uInt16 nLoop )
{
    SwDoc* pDoc = GetDoc();
    RedlineMode_t eOld = pDoc->GetRedlineMode();
    SwRedlineData* pCur;

    pDoc->SetRedlineMode_intern((RedlineMode_t)(eOld | nsRedlineMode_t::REDLINE_IGNORE));
    ::sw::UndoGuard const undoGuard(pDoc->GetIDocumentUndoRedo());

    // Determine the Type, it's the first on Stack
    for( pCur = pRedlineData; pCur->pNext; )
        pCur = pCur->pNext;

    switch( pCur->eType )
    {
    case nsRedlineType_t::REDLINE_INSERT:           // Content has been inserted
        bIsVisible = sal_False;
        switch( nLoop )
        {
        case 0: MoveToSection();    break;
        case 1: CopyToSection();    break;
        case 2: DelCopyOfSection(); break;
        }
        break;

    case nsRedlineType_t::REDLINE_DELETE:           // Inhalt wurde eingefuegt
        bIsVisible = sal_True;
        if( 1 <= nLoop )
            MoveFromSection();
        break;

    case nsRedlineType_t::REDLINE_FORMAT:           // Attributes have been applied
    case nsRedlineType_t::REDLINE_TABLE:            // Table structure has been modified
        if( 1 <= nLoop )
            InvalidateRange();
        break;
    default:
        break;
    }
    pDoc->SetRedlineMode_intern( eOld );
}


void SwRedline::InvalidateRange()       // trigger the Layout
{
    sal_uLong nSttNd = GetMark()->nNode.GetIndex(),
            nEndNd = GetPoint()->nNode.GetIndex();
    sal_uInt16 nSttCnt = GetMark()->nContent.GetIndex(),
            nEndCnt = GetPoint()->nContent.GetIndex();

    if( nSttNd > nEndNd || ( nSttNd == nEndNd && nSttCnt > nEndCnt ))
    {
        sal_uLong nTmp = nSttNd; nSttNd = nEndNd; nEndNd = nTmp;
        nTmp = nSttCnt; nSttCnt = nEndCnt; nEndCnt = (sal_uInt16)nTmp;
    }

    SwUpdateAttr aHt( 0, 0, RES_FMT_CHG );
    SwNodes& rNds = GetDoc()->GetNodes();
    for( sal_uLong n = nSttNd; n <= nEndNd; ++n )
    {
        SwNode* pNd = rNds[n];
        if( pNd->IsTxtNode() )
        {
            aHt.nStart = n == nSttNd ? nSttCnt : 0;
            aHt.nEnd = n == nEndNd ? nEndCnt : ((SwTxtNode*)pNd)->GetTxt().Len();
            ((SwTxtNode*)pNd)->ModifyNotification( &aHt, &aHt );
        }
    }
}

/*************************************************************************
 * SwRedline::CalcStartEnd()
 * Calculates the start and end position of the intersection rTmp and
 * text node nNdIdx
 *************************************************************************/

void SwRedline::CalcStartEnd( sal_uLong nNdIdx, sal_uInt16& nStart, sal_uInt16& nEnd ) const
{
    const SwPosition *pRStt = Start(), *pREnd = End();
    if( pRStt->nNode < nNdIdx )
    {
        if( pREnd->nNode > nNdIdx )
        {
            nStart = 0;             // Paragraph is completely enclosed
            nEnd = STRING_LEN;
        }
        else
        {
            OSL_ENSURE( pREnd->nNode == nNdIdx,
                "SwRedlineItr::Seek: GetRedlinePos Error" );
            nStart = 0;             // Paragraph is overlapped in the beginning
            nEnd = pREnd->nContent.GetIndex();
        }
    }
    else if( pRStt->nNode == nNdIdx )
    {
        nStart = pRStt->nContent.GetIndex();
        if( pREnd->nNode == nNdIdx )
            nEnd = pREnd->nContent.GetIndex(); // Within the Paragraph
        else
            nEnd = STRING_LEN;      // Paragraph is overlapped in the end
    }
    else
    {
        nStart = STRING_LEN;
        nEnd = STRING_LEN;
    }
}

void SwRedline::MoveToSection()
{
    if( !pCntntSect )
    {
        const SwPosition* pStt = Start(),
                        * pEnd = pStt == GetPoint() ? GetMark() : GetPoint();

        SwDoc* pDoc = GetDoc();
        SwPaM aPam( *pStt, *pEnd );
        SwCntntNode* pCSttNd = pStt->nNode.GetNode().GetCntntNode();
        SwCntntNode* pCEndNd = pEnd->nNode.GetNode().GetCntntNode();

        if( !pCSttNd )
        {
            // In order to not move other Redlines' indices, we set them
            // to the end (is exclusive)
            const SwRedlineTbl& rTbl = pDoc->GetRedlineTbl();
            for( sal_uInt16 n = 0; n < rTbl.Count(); ++n )
            {
                SwRedline* pRedl = rTbl[ n ];
                if( pRedl->GetBound(sal_True) == *pStt )
                    pRedl->GetBound(sal_True) = *pEnd;
                if( pRedl->GetBound(sal_False) == *pStt )
                    pRedl->GetBound(sal_False) = *pEnd;
            }
        }

        SwStartNode* pSttNd;
        SwNodes& rNds = pDoc->GetNodes();
        if( pCSttNd || pCEndNd )
        {
            SwTxtFmtColl* pColl = (pCSttNd && pCSttNd->IsTxtNode() )
                                    ? ((SwTxtNode*)pCSttNd)->GetTxtColl()
                                    : (pCEndNd && pCEndNd->IsTxtNode() )
                                        ? ((SwTxtNode*)pCEndNd)->GetTxtColl()
                                        : pDoc->GetTxtCollFromPool(
                                                RES_POOLCOLL_STANDARD );

            pSttNd = rNds.MakeTextSection( SwNodeIndex( rNds.GetEndOfRedlines() ),
                                            SwNormalStartNode, pColl );
            SwTxtNode* pTxtNd = rNds[ pSttNd->GetIndex() + 1 ]->GetTxtNode();

            SwNodeIndex aNdIdx( *pTxtNd );
            SwPosition aPos( aNdIdx, SwIndex( pTxtNd ));
            if( pCSttNd && pCEndNd )
                pDoc->MoveAndJoin( aPam, aPos, IDocumentContentOperations::DOC_MOVEDEFAULT );
            else
            {
                if( pCSttNd && !pCEndNd )
                    bDelLastPara = sal_True;
                pDoc->MoveRange( aPam, aPos,
                    IDocumentContentOperations::DOC_MOVEDEFAULT );
            }
        }
        else
        {
            pSttNd = rNds.MakeEmptySection( SwNodeIndex( rNds.GetEndOfRedlines() ),
                                            SwNormalStartNode );

            SwPosition aPos( *pSttNd->EndOfSectionNode() );
            pDoc->MoveRange( aPam, aPos,
                IDocumentContentOperations::DOC_MOVEDEFAULT );
        }
        pCntntSect = new SwNodeIndex( *pSttNd );

        if( pStt == GetPoint() )
            Exchange();

        DeleteMark();
    }
    else
        InvalidateRange();
}

void SwRedline::CopyToSection()
{
    if( !pCntntSect )
    {
        const SwPosition* pStt = Start(),
                        * pEnd = pStt == GetPoint() ? GetMark() : GetPoint();

        SwCntntNode* pCSttNd = pStt->nNode.GetNode().GetCntntNode();
        SwCntntNode* pCEndNd = pEnd->nNode.GetNode().GetCntntNode();

        SwStartNode* pSttNd;
        SwDoc* pDoc = GetDoc();
        SwNodes& rNds = pDoc->GetNodes();

        sal_Bool bSaveCopyFlag = pDoc->IsCopyIsMove(),
             bSaveRdlMoveFlg = pDoc->IsRedlineMove();
        pDoc->SetCopyIsMove( sal_True );

        // The IsRedlineMove() flag causes the behaviour of the
        // SwDoc::_CopyFlyInFly method to change, which will eventually be
        // called by the pDoc->Copy line below (through SwDoc::_Copy,
        // SwDoc::CopyWithFlyInFly). This rather obscure bugfix
        // apparently never really worked.
        pDoc->SetRedlineMove( pStt->nContent == 0 );

        if( pCSttNd )
        {
            SwTxtFmtColl* pColl = (pCSttNd && pCSttNd->IsTxtNode() )
                                    ? ((SwTxtNode*)pCSttNd)->GetTxtColl()
                                    : pDoc->GetTxtCollFromPool(
                                                RES_POOLCOLL_STANDARD );

            pSttNd = rNds.MakeTextSection( SwNodeIndex( rNds.GetEndOfRedlines() ),
                                            SwNormalStartNode, pColl );

            SwNodeIndex aNdIdx( *pSttNd, 1 );
            SwTxtNode* pTxtNd = aNdIdx.GetNode().GetTxtNode();
            SwPosition aPos( aNdIdx, SwIndex( pTxtNd ));
            pDoc->CopyRange( *this, aPos, false );

            // Take over the style from the EndNode if needed
            // We don't want this in Doc::Copy
            if( pCEndNd && pCEndNd != pCSttNd )
            {
                SwCntntNode* pDestNd = aPos.nNode.GetNode().GetCntntNode();
                if( pDestNd )
                {
                    if( pDestNd->IsTxtNode() && pCEndNd->IsTxtNode() )
                        ((SwTxtNode*)pCEndNd)->CopyCollFmt(
                                            *(SwTxtNode*)pDestNd );
                    else
                        pDestNd->ChgFmtColl( pCEndNd->GetFmtColl() );
                }
            }
        }
        else
        {
            pSttNd = rNds.MakeEmptySection( SwNodeIndex( rNds.GetEndOfRedlines() ),
                                            SwNormalStartNode );

            if( pCEndNd )
            {
                SwPosition aPos( *pSttNd->EndOfSectionNode() );
                pDoc->CopyRange( *this, aPos, false );
            }
            else
            {
                SwNodeIndex aInsPos( *pSttNd->EndOfSectionNode() );
                SwNodeRange aRg( pStt->nNode, 0, pEnd->nNode, 1 );
                pDoc->CopyWithFlyInFly( aRg, 0, aInsPos );
            }
        }
        pCntntSect = new SwNodeIndex( *pSttNd );

        pDoc->SetCopyIsMove( bSaveCopyFlag );
        pDoc->SetRedlineMove( bSaveRdlMoveFlg );
    }
}

void SwRedline::DelCopyOfSection()
{
    if( pCntntSect )
    {
        const SwPosition* pStt = Start(),
                        * pEnd = pStt == GetPoint() ? GetMark() : GetPoint();

        SwDoc* pDoc = GetDoc();
        SwPaM aPam( *pStt, *pEnd );
        SwCntntNode* pCSttNd = pStt->nNode.GetNode().GetCntntNode();
        SwCntntNode* pCEndNd = pEnd->nNode.GetNode().GetCntntNode();

        if( !pCSttNd )
        {
            // In order to not move other Redlines' indices, we set them
            // to the end (is exclusive)
            const SwRedlineTbl& rTbl = pDoc->GetRedlineTbl();
            for( sal_uInt16 n = 0; n < rTbl.Count(); ++n )
            {
                SwRedline* pRedl = rTbl[ n ];
                if( pRedl->GetBound(sal_True) == *pStt )
                    pRedl->GetBound(sal_True) = *pEnd;
                if( pRedl->GetBound(sal_False) == *pStt )
                    pRedl->GetBound(sal_False) = *pEnd;
            }
        }

        if( pCSttNd && pCEndNd )
        {
            // #i100466# - force a <join next> on <delete and join> operation
            pDoc->DeleteAndJoin( aPam, true );
        }
        else if( pCSttNd || pCEndNd )
        {
            if( pCSttNd && !pCEndNd )
                bDelLastPara = sal_True;
            pDoc->DeleteRange( aPam );

            if( bDelLastPara )
            {
                // To prevent dangling references to the paragraph to
                // be deleted, redline that point into this paragraph should be
                // moved to the new end position. Since redlines in the redline
                // table are sorted and the pEnd position is an endnode (see
                // bDelLastPara condition above), only redlines before the
                // current ones can be affected.
                const SwRedlineTbl& rTbl = pDoc->GetRedlineTbl();
                sal_uInt16 n = rTbl.GetPos( this );
                OSL_ENSURE( n != USHRT_MAX, "How strange. We don't exist!" );
                for( sal_Bool bBreak = sal_False; !bBreak && n > 0; )
                {
                    --n;
                    bBreak = sal_True;
                    if( rTbl[ n ]->GetBound(sal_True) == *aPam.GetPoint() )
                    {
                        rTbl[ n ]->GetBound(sal_True) = *pEnd;
                        bBreak = sal_False;
                    }
                    if( rTbl[ n ]->GetBound(sal_False) == *aPam.GetPoint() )
                    {
                        rTbl[ n ]->GetBound(sal_False) = *pEnd;
                        bBreak = sal_False;
                    }
                }

                SwPosition aEnd( *pEnd );
                *GetPoint() = *pEnd;
                *GetMark() = *pEnd;
                DeleteMark();

                aPam.GetBound( sal_True ).nContent.Assign( 0, 0 );
                aPam.GetBound( sal_False ).nContent.Assign( 0, 0 );
                aPam.DeleteMark();
                pDoc->DelFullPara( aPam );
            }
        }
        else
        {
            pDoc->DeleteRange( aPam );
        }

        if( pStt == GetPoint() )
            Exchange();

        DeleteMark();
    }
}

void SwRedline::MoveFromSection()
{
    if( pCntntSect )
    {
        SwDoc* pDoc = GetDoc();
        const SwRedlineTbl& rTbl = pDoc->GetRedlineTbl();
        SvPtrarr aBeforeArr( 16 ), aBehindArr( 16 );
        sal_uInt16 nMyPos = rTbl.GetPos( this );
        OSL_ENSURE( this, "this is not in the array?" );
        sal_Bool bBreak = sal_False;
        sal_uInt16 n;

        for( n = nMyPos+1; !bBreak && n < rTbl.Count(); ++n )
        {
            bBreak = sal_True;
            if( rTbl[ n ]->GetBound(sal_True) == *GetPoint() )
            {
                void* pTmp = &rTbl[ n ]->GetBound(sal_True);
                aBehindArr.Insert( pTmp, aBehindArr.Count());
                bBreak = sal_False;
            }
            if( rTbl[ n ]->GetBound(sal_False) == *GetPoint() )
            {
                void* pTmp = &rTbl[ n ]->GetBound(sal_False);
                aBehindArr.Insert( pTmp, aBehindArr.Count() );
                bBreak = sal_False;
            }
        }
        for( bBreak = sal_False, n = nMyPos; !bBreak && n ; )
        {
            --n;
            bBreak = sal_True;
            if( rTbl[ n ]->GetBound(sal_True) == *GetPoint() )
            {
                void* pTmp = &rTbl[ n ]->GetBound(sal_True);
                aBeforeArr.Insert( pTmp, aBeforeArr.Count() );
                bBreak = sal_False;
            }
            if( rTbl[ n ]->GetBound(sal_False) == *GetPoint() )
            {
                void* pTmp = &rTbl[ n ]->GetBound(sal_False);
                aBeforeArr.Insert( pTmp, aBeforeArr.Count() );
                bBreak = sal_False;
            }
        }

        const SwNode* pKeptCntntSectNode( &pCntntSect->GetNode() ); // #i95711#
        {
            SwPaM aPam( pCntntSect->GetNode(),
                        *pCntntSect->GetNode().EndOfSectionNode(), 1,
                        ( bDelLastPara ? -2 : -1 ) );
            SwCntntNode* pCNd = aPam.GetCntntNode();
            if( pCNd )
                aPam.GetPoint()->nContent.Assign( pCNd, pCNd->Len() );
            else
                aPam.GetPoint()->nNode++;

            SwFmtColl* pColl = pCNd && pCNd->Len() && aPam.GetPoint()->nNode !=
                                        aPam.GetMark()->nNode
                                ? pCNd->GetFmtColl() : 0;

            SwNodeIndex aNdIdx( GetPoint()->nNode, -1 );
            sal_uInt16 nPos = GetPoint()->nContent.GetIndex();

            SwPosition aPos( *GetPoint() );
            if( bDelLastPara && *aPam.GetPoint() == *aPam.GetMark() )
            {
                aPos.nNode--;

                pDoc->AppendTxtNode( aPos );
            }
            else
            {
                pDoc->MoveRange( aPam, aPos,
                    IDocumentContentOperations::DOC_MOVEALLFLYS );
            }

            SetMark();
            *GetPoint() = aPos;
            GetMark()->nNode = aNdIdx.GetIndex() + 1;
            pCNd = GetMark()->nNode.GetNode().GetCntntNode();
            GetMark()->nContent.Assign( pCNd, nPos );

            if( bDelLastPara )
            {
                GetPoint()->nNode++;
                GetPoint()->nContent.Assign( pCNd = GetCntntNode(), 0 );
                bDelLastPara = sal_False;
            }
            else if( pColl )
                pCNd = GetCntntNode();

            if( pColl && pCNd )
                pCNd->ChgFmtColl( pColl );
        }
        // #i95771#
        // Under certain conditions the previous <SwDoc::Move(..)> has already
        // removed the change tracking section of this <SwRedline> instance from
        // the change tracking nodes area.
        // Thus, check if <pCntntSect> still points to the change tracking section
        // by comparing it with the "indexed" <SwNode> instance copied before
        // perform the intrinsic move.
        // Note: Such condition is e.g. a "delete" change tracking only containing a table.
        if ( &pCntntSect->GetNode() == pKeptCntntSectNode )
        {
            pDoc->DeleteSection( &pCntntSect->GetNode() );
        }
        delete pCntntSect, pCntntSect = 0;

        // adjustment of redline table positions must take start and
        // end into account, not point and mark.
        for( n = 0; n < aBeforeArr.Count(); ++n )
            *(SwPosition*)aBeforeArr[ n ] = *Start();
        for( n = 0; n < aBehindArr.Count(); ++n )
            *(SwPosition*)aBehindArr[ n ] = *End();
    }
    else
        InvalidateRange();
}

// for Undo
void SwRedline::SetContentIdx( const SwNodeIndex* pIdx )
{
    if( pIdx && !pCntntSect )
    {
        pCntntSect = new SwNodeIndex( *pIdx );
        bIsVisible = sal_False;
    }
    else if( !pIdx && pCntntSect )
    {
        delete pCntntSect, pCntntSect = 0;
        bIsVisible = sal_False;
    }
    else
    {
        OSL_FAIL("SwRedline::SetContentIdx: invalid state");
    }
}

sal_Bool SwRedline::CanCombine( const SwRedline& rRedl ) const
{
    return  IsVisible() && rRedl.IsVisible() &&
            pRedlineData->CanCombine( *rRedl.pRedlineData );
}

void SwRedline::PushData( const SwRedline& rRedl, sal_Bool bOwnAsNext )
{
    SwRedlineData* pNew = new SwRedlineData( *rRedl.pRedlineData, sal_False );
    if( bOwnAsNext )
    {
        pNew->pNext = pRedlineData;
        pRedlineData = pNew;
    }
    else
    {
        pNew->pNext = pRedlineData->pNext;
        pRedlineData->pNext = pNew;
    }
}

sal_Bool SwRedline::PopData()
{
    if( !pRedlineData->pNext )
        return sal_False;
    SwRedlineData* pCur = pRedlineData;
    pRedlineData = pCur->pNext;
    pCur->pNext = 0;
    delete pCur;
    return sal_True;
}

sal_uInt16 SwRedline::GetStackCount() const
{
    sal_uInt16 nRet = 1;
    for( SwRedlineData* pCur = pRedlineData; pCur->pNext; ++nRet )
        pCur = pCur->pNext;
    return nRet;
}

sal_uInt16 SwRedline::GetAuthor( sal_uInt16 nPos ) const
{
    return GetRedlineData(nPos).nAuthor;
}

const String& SwRedline::GetAuthorString( sal_uInt16 nPos ) const
{
    return SW_MOD()->GetRedlineAuthor(GetRedlineData(nPos).nAuthor);
}

const DateTime& SwRedline::GetTimeStamp( sal_uInt16 nPos ) const
{
    return GetRedlineData(nPos).aStamp;
}

RedlineType_t SwRedline::GetRealType( sal_uInt16 nPos ) const
{
    return GetRedlineData(nPos).eType;
}

const String& SwRedline::GetComment( sal_uInt16 nPos ) const
{
    return GetRedlineData(nPos).sComment;
}

int SwRedline::operator==( const SwRedline& rCmp ) const
{
    return this == &rCmp;
}

int SwRedline::operator<( const SwRedline& rCmp ) const
{
    sal_Bool nResult = sal_False;

    if (*Start() < *rCmp.Start())
        nResult = sal_True;
    else if (*Start() == *rCmp.Start())
        if (*End() < *rCmp.End())
            nResult = sal_True;

    return nResult;
}

const SwRedlineData & SwRedline::GetRedlineData(sal_uInt16 nPos) const
{
    SwRedlineData * pCur = pRedlineData;

    while (nPos > 0 && NULL != pCur->pNext)
    {
        pCur = pCur->pNext;

        nPos--;
    }

    OSL_ENSURE( 0 == nPos, "Pos is too big" );

    return *pCur;
}

String SwRedline::GetDescr(sal_uInt16 nPos)
{
    String aResult;

    // get description of redline data (e.g.: "insert $1")
    aResult = GetRedlineData(nPos).GetDescr();

    SwPaM * pPaM = NULL;
    bool bDeletePaM = false;

    // if this redline is visible the content is in this PaM
    if (NULL == pCntntSect)
    {
        pPaM = this;
    }
    else // otherwise it is saved in pCntntSect
    {
        SwNodeIndex aTmpIdx( *pCntntSect->GetNode().EndOfSectionNode() );
        pPaM = new SwPaM(*pCntntSect, aTmpIdx );
        bDeletePaM = true;
    }

    // replace $1 in description by description of the redlines text
    String aTmpStr;
    aTmpStr += String(SW_RES(STR_START_QUOTE));
    aTmpStr += ShortenString(pPaM->GetTxt(), nUndoStringLength,
                             String(SW_RES(STR_LDOTS)));
    aTmpStr += String(SW_RES(STR_END_QUOTE));

    SwRewriter aRewriter;
    aRewriter.AddRule(UNDO_ARG1, aTmpStr);

    aResult = aRewriter.Apply(aResult);

    if (bDeletePaM)
        delete pPaM;

    return aResult;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
