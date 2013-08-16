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

#include <hintids.hxx>
#include <tools/shl.hxx>
#include <svl/itemiter.hxx>
#include <sfx2/app.hxx>
#include <editeng/colritem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/crossedoutitem.hxx>
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
    static void lcl_CheckPosition( const SwPosition* pPos )
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

    static void lcl_CheckPam( const SwPaM* pPam )
    {
        OSL_ENSURE( pPam != NULL, _ERROR_PREFIX "illegal argument" );
        lcl_CheckPosition( pPam->GetPoint() );
        lcl_CheckPosition( pPam->GetMark() );
    }

    // check validity of the redline table. Checks redline bounds, and make
    // sure the redlines are sorted and non-overlapping.
    static void lcl_CheckRedline( const SwDoc* pDoc )
    {
        const SwRedlineTbl& rTbl = pDoc->GetRedlineTbl();

        // verify valid redline positions
        for( sal_uInt16 i = 0; i < rTbl.size(); ++i )
            lcl_CheckPam( rTbl[ i ] );

        for( sal_uInt16 j = 0; j < rTbl.size(); ++j )
        {
            // check for empty redlines
            OSL_ENSURE( ( *(rTbl[j]->GetPoint()) != *(rTbl[j]->GetMark()) ) ||
                        ( rTbl[j]->GetContentIdx() != NULL ),
                        _ERROR_PREFIX "empty redline" );
         }

        // verify proper redline sorting
        for( sal_uInt16 n = 1; n < rTbl.size(); ++n )
        {
            const SwRedline* pPrev = rTbl[ n-1 ];
            const SwRedline* pCurrent = rTbl[ n ];

            // check redline sorting
            SAL_WARN_IF( *pPrev->Start() > *pCurrent->Start(), "sw",
                         _ERROR_PREFIX "not sorted correctly" );

            // check for overlapping redlines
            SAL_WARN_IF( *pPrev->End() > *pCurrent->Start(), "sw",
                         _ERROR_PREFIX "overlapping redlines" );
        }
    }

    #define _CHECK_REDLINE( pDoc ) lcl_CheckRedline( pDoc );

    void sw_DebugRedline( const SwDoc* pDoc )
    {
        static sal_uInt16 nWatch = 0;
        const SwRedlineTbl& rTbl = pDoc->GetRedlineTbl();
        for( sal_uInt16 n = 0; n < rTbl.size(); ++n )
        {
            sal_uInt16 nDummy = 0;
            const SwRedline* pCurrent = rTbl[ n ];
            const SwRedline* pNext = n+1 < (sal_uInt16)rTbl.size() ? rTbl[ n+1 ] : 0;
            if( pCurrent == pNext )
                ++nDummy;
            if( n == nWatch )
                ++nDummy; // Possible debugger breakpoint
        }
    }

#else

    #define _CHECK_REDLINE( pDoc )

#endif

RedlineMode_t SwDoc::GetRedlineMode() const
{
    return meRedlineMode;
}

void SwDoc::SetRedlineMode( RedlineMode_t eMode )
{
    if( meRedlineMode != eMode )
    {
        if( (nsRedlineMode_t::REDLINE_SHOW_MASK & meRedlineMode) != (nsRedlineMode_t::REDLINE_SHOW_MASK & eMode)
            || 0 == (nsRedlineMode_t::REDLINE_SHOW_MASK & eMode) )
        {
            bool bSaveInXMLImportFlag = IsInXMLImport();
            SetInXMLImport( false );
            // and then hide/display everything
            void (SwRedline::*pFnc)( sal_uInt16 ) = 0;

            switch( nsRedlineMode_t::REDLINE_SHOW_MASK & eMode )
            {
            case nsRedlineMode_t::REDLINE_SHOW_INSERT | nsRedlineMode_t::REDLINE_SHOW_DELETE :
                pFnc = &SwRedline::Show;
                break;
            case nsRedlineMode_t::REDLINE_SHOW_INSERT:
                pFnc = &SwRedline::Hide;
                break;
            case nsRedlineMode_t::REDLINE_SHOW_DELETE:
                pFnc = &SwRedline::ShowOriginal;
                break;

            default:
                pFnc = &SwRedline::Hide;
                eMode = (RedlineMode_t)(eMode | nsRedlineMode_t::REDLINE_SHOW_INSERT);
                break;
            }

            _CHECK_REDLINE( this )

            if( pFnc )
                for( sal_uInt16 nLoop = 1; nLoop <= 2; ++nLoop )
                    for( sal_uInt16 i = 0; i < mpRedlineTbl->size(); ++i )
                        ((*mpRedlineTbl)[ i ]->*pFnc)( nLoop );
            _CHECK_REDLINE( this )
            SetInXMLImport( bSaveInXMLImportFlag );
        }
        meRedlineMode = eMode;
        SetModified();
    }
}

bool SwDoc::IsRedlineOn() const
{
    return IDocumentRedlineAccess::IsRedlineOn(meRedlineMode);
}

bool SwDoc::IsIgnoreRedline() const
{
    return (nsRedlineMode_t::REDLINE_IGNORE & meRedlineMode);
}

void SwDoc::SetRedlineMode_intern(RedlineMode_t eMode)
{
    meRedlineMode = eMode;
}

const SwRedlineTbl& SwDoc::GetRedlineTbl() const
{
    return *mpRedlineTbl;
}

bool SwDoc::IsRedlineMove() const
{
    return mbIsRedlineMove;
}

void SwDoc::SetRedlineMove(bool bFlag)
{
    mbIsRedlineMove = bFlag;
}

const uno::Sequence <sal_Int8>& SwDoc::GetRedlinePassword() const
{
    return maRedlinePasswd;
}

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
bool SwDoc::AppendRedline( SwRedline* pNewRedl, bool bCallDelete )
{
    bool bError = true;
    _CHECK_REDLINE( this )

    if( IsRedlineOn() && !IsShowOriginal( meRedlineMode ) &&
        !pNewRedl->GetAuthorString().isEmpty() )
    {
        pNewRedl->InvalidateRange();

        if( mbIsAutoFmtRedline )
        {
            pNewRedl->SetAutoFmtFlag();
            if( mpAutoFmtRedlnComment && mpAutoFmtRedlnComment->Len() )
            {
                pNewRedl->SetComment( *mpAutoFmtRedlnComment );
                pNewRedl->SetSeqNo( mnAutoFmtRedlnCommentNo );
            }
        }

        SwPosition* pStt = pNewRedl->Start(),
                  * pEnd = pStt == pNewRedl->GetPoint() ? pNewRedl->GetMark()
                                                        : pNewRedl->GetPoint();
        {
            SwTxtNode* pTxtNode = pStt->nNode.GetNode().GetTxtNode();
            if( pTxtNode == NULL )
            {
                if( pStt->nContent > 0 )
                {
                    OSL_ENSURE( false, "Redline start: non-text-node with content" );
                    pStt->nContent = 0;
                }
            }
            else
            {
                if( pStt->nContent > pTxtNode->Len() )
                {
                    OSL_ENSURE( false, "Redline start: index behind text" );
                    pStt->nContent = pTxtNode->Len();
                }
            }
            pTxtNode = pEnd->nNode.GetNode().GetTxtNode();
            if( pTxtNode == NULL )
            {
                if( pEnd->nContent > 0 )
                {
                    OSL_ENSURE( false, "Redline end: non-text-node with content" );
                    pEnd->nContent = 0;
                }
            }
            else
            {
                if( pEnd->nContent > pTxtNode->Len() )
                {
                    OSL_ENSURE( false, "Redline end: index behind text" );
                    pEnd->nContent = pTxtNode->Len();
                }
            }
        }
        if( ( *pStt == *pEnd ) &&
            ( pNewRedl->GetContentIdx() == NULL ) )
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

        for( ; pNewRedl && n < mpRedlineTbl->size(); bDec ? n : ++n )
        {
            bDec = false;

            SwRedline* pRedl = (*mpRedlineTbl)[ n ];
            SwPosition* pRStt = pRedl->Start(),
                      * pREnd = pRStt == pRedl->GetPoint() ? pRedl->GetMark()
                                                           : pRedl->GetPoint();

            // #i8518# remove empty redlines while we're at it
            if( ( *pRStt == *pREnd ) &&
                ( pRedl->GetContentIdx() == NULL ) )
            {
                mpRedlineTbl->DeleteAndDestroy(n);
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
                            ( n+1 >= (sal_uInt16)mpRedlineTbl->size() ||
                             ( *(*mpRedlineTbl)[ n+1 ]->Start() >= *pEnd &&
                             *(*mpRedlineTbl)[ n+1 ]->Start() != *pREnd ) ) )
                        {
                            pRedl->SetEnd( *pEnd, pREnd );
                            if( !pRedl->HasValidRange() )
                            {
                                // re-insert
                                mpRedlineTbl->Remove( n );
                                mpRedlineTbl->Insert( pRedl );
                            }

                            bError = false;
                            bDelete = true;
                        }
                        else if( (( POS_BEFORE == eCmpPos &&
                                    IsPrevPos( *pEnd, *pRStt ) ) ||
                                   ( POS_COLLIDE_END == eCmpPos ) ||
                                  ( POS_OVERLAP_BEFORE == eCmpPos ) ) &&
                            pRedl->CanCombine( *pNewRedl ) &&
                            ( !n ||
                             *(*mpRedlineTbl)[ n-1 ]->End() != *pRStt ))
                        {
                            pRedl->SetStart( *pStt, pRStt );
                            // re-insert
                            mpRedlineTbl->Remove( n );
                            mpRedlineTbl->Insert( pRedl );

                            bError = false;
                            bDelete = true;
                        }
                        else if ( POS_OUTSIDE == eCmpPos )
                        {
                            // own insert-over-insert redlines:
                            // just scrap the inside ones
                            mpRedlineTbl->DeleteAndDestroy( n );
                            bDec = true;
                        }
                        else if( POS_OVERLAP_BEHIND == eCmpPos )
                        {
                            *pStt = *pREnd;
                            if( ( *pStt == *pEnd ) &&
                                ( pNewRedl->GetContentIdx() == NULL ) )
                                bDelete = true;
                        }
                        else if( POS_OVERLAP_BEFORE == eCmpPos )
                        {
                            *pEnd = *pRStt;
                            if( ( *pStt == *pEnd ) &&
                                ( pNewRedl->GetContentIdx() == NULL ) )
                                bDelete = true;
                        }
                        else if( POS_INSIDE == eCmpPos || POS_EQUAL == eCmpPos)
                            bDelete = true;

                        if( bDelete )
                        {
                            delete pNewRedl, pNewRedl = 0;
                            bCompress = true;
                        }
                    }
                    else if( POS_INSIDE == eCmpPos )
                    {
                        // split up
                        if( *pEnd != *pREnd )
                        {
                            SwRedline* pCpy = new SwRedline( *pRedl );
                            pCpy->SetStart( *pEnd );
                            mpRedlineTbl->Insert( pCpy );
                        }
                        pRedl->SetEnd( *pStt, pREnd );
                        if( ( *pStt == *pRStt ) &&
                            ( pRedl->GetContentIdx() == NULL ) )
                        {
                            mpRedlineTbl->DeleteAndDestroy( n );
                            bDec = true;
                        }
                        else if( !pRedl->HasValidRange() )
                        {
                            // re-insert
                            mpRedlineTbl->Remove( n );
                            mpRedlineTbl->Insert( pRedl );
                        }
                    }
                    else if ( POS_OUTSIDE == eCmpPos )
                    {
                        // handle overlapping redlines in broken documents

                        // split up the new redline, since it covers the
                        // existing redline. Insert the first part, and
                        // progress with the remainder as usual
                        SwRedline* pSplit = new SwRedline( *pNewRedl );
                        pSplit->SetEnd( *pRStt );
                        pNewRedl->SetStart( *pREnd );
                        mpRedlineTbl->Insert( pSplit );
                        if( *pStt == *pEnd && pNewRedl->GetContentIdx() == NULL )
                        {
                            delete pNewRedl;
                            pNewRedl = 0;
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
                            ( pNewRedl->GetContentIdx() == NULL ) )
                        {
                            delete pNewRedl;
                            pNewRedl = 0;
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
                            SwRedline* pCpy = new SwRedline( *pRedl );
                            pCpy->SetStart( *pEnd );
                            mpRedlineTbl->Insert( pCpy );
                        }
                        pRedl->SetEnd( *pStt, pREnd );
                        if( ( *pStt == *pRStt ) &&
                            ( pRedl->GetContentIdx() == NULL ) )
                        {
                            mpRedlineTbl->DeleteAndDestroy( n );
                            bDec = true;
                        }
                        else if( !pRedl->HasValidRange() )
                        {
                            // re-insert
                            mpRedlineTbl->Remove( n );
                            mpRedlineTbl->Insert( pRedl, n );
                        }
                    }
                    else if ( POS_OUTSIDE == eCmpPos )
                    {
                        // handle overlapping redlines in broken documents

                        // split up the new redline, since it covers the
                        // existing redline. Insert the first part, and
                        // progress with the remainder as usual
                        SwRedline* pSplit = new SwRedline( *pNewRedl );
                        pSplit->SetEnd( *pRStt );
                        pNewRedl->SetStart( *pREnd );
                        mpRedlineTbl->Insert( pSplit );
                        if( *pStt == *pEnd && pNewRedl->GetContentIdx() == NULL )
                        {
                            delete pNewRedl;
                            pNewRedl = 0;
                            bCompress = true;
                        }
                    }
                    else if ( POS_EQUAL == eCmpPos )
                    {
                        // handle identical redlines in broken documents
                        // delete old (delete) redline
                        mpRedlineTbl->DeleteAndDestroy( n );
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
                        mpRedlineTbl->Remove( n );
                        mpRedlineTbl->Insert( pRedl, n );
                        bDec = true;
                        break;

                    case POS_OVERLAP_BEHIND:
                        pRedl->SetEnd( *pStt, pREnd );
                        if( *pStt == *pRStt && pRedl->GetContentIdx() == NULL )
                        {
                            mpRedlineTbl->DeleteAndDestroy( n );
                            bDec = true;
                        }
                        break;

                    case POS_EQUAL:
                    case POS_OUTSIDE:
                        // Overlaps the current one completely or has the
                        // same dimension, delete the old one
                        mpRedlineTbl->DeleteAndDestroy( n );
                        bDec = true;
                        break;

                    case POS_INSIDE:
                        // Overlaps the current one completely,
                        // split or shorten the new one
                        if( *pEnd != *pREnd )
                        {
                            if( *pEnd != *pRStt )
                            {
                                SwRedline* pNew = new SwRedline( *pRedl );
                                pNew->SetStart( *pEnd );
                                pRedl->SetEnd( *pStt, pREnd );
                                if( *pStt == *pRStt && pRedl->GetContentIdx() == NULL )
                                    mpRedlineTbl->DeleteAndDestroy( n );
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
                                SwRedline* pNew = new SwRedline( *pNewRedl );
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
                        delete pNewRedl, pNewRedl = 0;
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
                            mpRedlineTbl->DeleteAndDestroy( n );
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
                                mpRedlineTbl->Insert( pNewRedl );
                                pRedl->Show();
                                mpRedlineTbl->Remove( pNewRedl );
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

                            mpRedlineTbl->DeleteAndDestroy( nToBeDeleted );
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
                            mpRedlineTbl->DeleteAndDestroy( n );
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
                                        pEnd->nNode.GetNode().GetTxtNode(), 0);
                                    DelFullPara( *pNewRedl );
                                }
                                else
                                    DeleteAndJoin( *pNewRedl );

                                bCompress = true;
                            }
                            delete pNewRedl, pNewRedl = 0;
                            break;

                        case POS_OUTSIDE:
                            {
                                mpRedlineTbl->Remove( n );
                                bDec = true;
                                // We insert temporarily so that pNew is
                                // also dealt with when moving the indices.
                                if( bCallDelete )
                                {
                                    mpRedlineTbl->Insert( pNewRedl );
                                    DeleteAndJoin( *pRedl );
                                    if( !mpRedlineTbl->Remove( pNewRedl ) )
                                        pNewRedl = 0;
                                }
                                delete pRedl;
                            }
                            break;

                        case POS_OVERLAP_BEFORE:
                            {
                                SwPaM aPam( *pRStt, *pEnd );

                                if( *pEnd == *pREnd )
                                    mpRedlineTbl->DeleteAndDestroy( n );
                                else
                                {
                                    pRedl->SetStart( *pEnd, pRStt );
                                    // re-insert
                                    mpRedlineTbl->Remove( n );
                                    mpRedlineTbl->Insert( pRedl, n );
                                }

                                if( bCallDelete )
                                {
                                    // We insert temporarily so that pNew is
                                    // also dealt with when moving the indices.
                                    mpRedlineTbl->Insert( pNewRedl );
                                    DeleteAndJoin( aPam );
                                    if( !mpRedlineTbl->Remove( pNewRedl ) )
                                        pNewRedl = 0;
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
                                    mpRedlineTbl->DeleteAndDestroy( n );
                                    bDec = true;
                                }
                                else
                                    pRedl->SetEnd( *pStt, pREnd );

                                if( bCallDelete )
                                {
                                    // We insert temporarily so that pNew is
                                    // also dealt with when moving the indices.
                                    mpRedlineTbl->Insert( pNewRedl );
                                    DeleteAndJoin( aPam );
                                    if( !mpRedlineTbl->Remove( pNewRedl ) )
                                        pNewRedl = 0;
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
                        // only part of it's former range, and pNew will cover
                        // the remainder.
                        SwRedline* pNew = 0;

                        switch( eCmpPos )
                        {
                        case POS_EQUAL:
                            {
                                pRedl->PushData( *pNewRedl );
                                delete pNewRedl, pNewRedl = 0;
                                if( IsHideChanges( meRedlineMode ))
                                    pRedl->Hide();
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
                                        pNewRedl->PushData( *pRedl, sal_False );
                                        pRedl->SetStart( *pEnd, pRStt );
                                        // re-insert
                                        mpRedlineTbl->Remove( n );
                                        mpRedlineTbl->Insert( pRedl, n );
                                        bDec = true;
                                    }
                                }
                                else
                                {
                                    pNewRedl->PushData( *pRedl, sal_False );
                                    if( *pREnd != *pEnd )
                                    {
                                        pNew = new SwRedline( *pRedl );
                                        pNew->SetStart( *pEnd );
                                    }
                                    pRedl->SetEnd( *pStt, pREnd );
                                    if( !pRedl->HasValidRange() )
                                    {
                                        // re-insert
                                        mpRedlineTbl->Remove( n );
                                        mpRedlineTbl->Insert( pRedl, n );
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
                                    pNew = new SwRedline( *pNewRedl );
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
                                        mpRedlineTbl->Insert( pNewRedl );
                                        pRedl->Hide();
                                        mpRedlineTbl->Remove( pNewRedl );
                                    }
                                }
                                else
                                {
                                    pNew = new SwRedline( *pRedl );
                                    pNew->PushData( *pNewRedl );
                                    pNew->SetEnd( *pEnd );
                                    pNewRedl->SetEnd( *pRStt, pEnd );
                                    pRedl->SetStart( *pNew->End(), pRStt ) ;
                                    // re-insert
                                    mpRedlineTbl->Remove( n );
                                    mpRedlineTbl->Insert( pRedl );
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
                                        mpRedlineTbl->Insert( pNewRedl );
                                        pRedl->Hide();
                                        mpRedlineTbl->Remove( pNewRedl );
                                    }
                                }
                                else
                                {
                                    pNew = new SwRedline( *pRedl );
                                    pNew->PushData( *pNewRedl );
                                    pNew->SetStart( *pStt );
                                    pNewRedl->SetStart( *pREnd, pStt );
                                    pRedl->SetEnd( *pNew->Start(), pREnd );
                                    if( !pRedl->HasValidRange() )
                                    {
                                        // re-insert
                                        mpRedlineTbl->Remove( n );
                                        mpRedlineTbl->Insert( pRedl );
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
                            mpRedlineTbl->Insert( pNew );

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
                        mpRedlineTbl->Remove( n );
                        mpRedlineTbl->Insert( pRedl, n );
                        bDec = true;
                        break;

                    case POS_OVERLAP_BEHIND:
                        pRedl->SetEnd( *pStt, pREnd );
                        break;

                    case POS_EQUAL:
                    case POS_OUTSIDE:
                        // Overlaps the current one completely or has the
                        // same dimension, delete the old one
                        mpRedlineTbl->DeleteAndDestroy( n );
                        bDec = true;
                        break;

                    case POS_INSIDE:
                        // Overlaps the current one completely,
                        // split or shorten the new one
                        if( *pEnd != *pREnd )
                        {
                            if( *pEnd != *pRStt )
                            {
                                SwRedline* pNew = new SwRedline( *pRedl );
                                pNew->SetStart( *pEnd );
                                pRedl->SetEnd( *pStt, pREnd );
                                if( ( *pStt == *pRStt ) &&
                                    ( pRedl->GetContentIdx() == NULL ) )
                                    mpRedlineTbl->DeleteAndDestroy( n );
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
                        delete pNewRedl, pNewRedl = 0;
                        break;

                    case POS_OUTSIDE:
                        // Overlaps the current one completely,
                        // split or shorten the new one
                        if( *pEnd != *pREnd )
                        {
                            if( *pEnd != *pRStt )
                            {
                                SwRedline* pNew = new SwRedline( *pNewRedl );
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
                            mpRedlineTbl->DeleteAndDestroy( n );
                            bDec = true;
                        }
                        break;

                    case POS_INSIDE:
                        if( pRedl->IsOwnRedline( *pNewRedl ) &&
                            pRedl->CanCombine( *pNewRedl ))
                            // own one can be ignored completely
                            delete pNewRedl, pNewRedl = 0;

                        else if( *pREnd == *pEnd )
                            // or else only shorten the current one
                            pRedl->SetEnd( *pStt, pREnd );
                        else if( *pRStt == *pStt )
                        {
                            // or else only shorten the current one
                            pRedl->SetStart( *pEnd, pRStt );
                            // re-insert
                            mpRedlineTbl->Remove( n );
                            mpRedlineTbl->Insert( pRedl, n );
                            bDec = true;
                        }
                        else
                        {
                            // If it lies completely within the current one
                            // we need to split it
                            SwRedline* pNew = new SwRedline( *pRedl );
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
                            mpRedlineTbl->DeleteAndDestroy( n );
                            bDec = 0;
                        }
                        else if( POS_OVERLAP_BEHIND == eCmpPos )
                            pNewRedl->SetStart( *pREnd, pStt );
                        else
                            pNewRedl->SetEnd( *pRStt, pEnd );
                        break;

                    case POS_COLLIDE_END:
                        if( pRedl->IsOwnRedline( *pNewRedl ) &&
                            pRedl->CanCombine( *pNewRedl ) && n &&
                            *(*mpRedlineTbl)[ n-1 ]->End() < *pStt )
                        {
                            // If that's the case we can merge it, meaning
                            // the new one covers this well
                            pNewRedl->SetEnd( *pREnd, pEnd );
                            mpRedlineTbl->DeleteAndDestroy( n );
                            bDec = true;
                        }
                        break;
                    case POS_COLLIDE_START:
                        if( pRedl->IsOwnRedline( *pNewRedl ) &&
                            pRedl->CanCombine( *pNewRedl ) &&
                            n+1 < (sal_uInt16)mpRedlineTbl->size() &&
                            *(*mpRedlineTbl)[ n+1 ]->Start() < *pEnd )
                        {
                            // If that's the case we can merge it, meaning
                            // the new one covers this well
                            pNewRedl->SetStart( *pRStt, pStt );
                            mpRedlineTbl->DeleteAndDestroy( n );
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
                ( pNewRedl->GetContentIdx() == NULL ) )
            {   // Do not insert empty redlines
                delete pNewRedl;
                pNewRedl = 0;
            }
            else
                mpRedlineTbl->Insert( pNewRedl );
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
            DeleteAndJoin( *pNewRedl );
            meRedlineMode = eOld;
        }
        delete pNewRedl, pNewRedl = 0;
    }
    _CHECK_REDLINE( this )

    return ( 0 != pNewRedl ) || !bError;
}

void SwDoc::CompressRedlines()
{
    _CHECK_REDLINE( this )

    void (SwRedline::*pFnc)(sal_uInt16) = 0;
    switch( nsRedlineMode_t::REDLINE_SHOW_MASK & meRedlineMode )
    {
    case nsRedlineMode_t::REDLINE_SHOW_INSERT | nsRedlineMode_t::REDLINE_SHOW_DELETE:
        pFnc = &SwRedline::Show;
        break;
    case nsRedlineMode_t::REDLINE_SHOW_INSERT:
        pFnc = &SwRedline::Hide;
        break;
    }

    // Try to merge identical ones
    for( sal_uInt16 n = 1; n < mpRedlineTbl->size(); ++n )
    {
        SwRedline* pPrev = (*mpRedlineTbl)[ n-1 ],
                    * pCur = (*mpRedlineTbl)[ n ];
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
            pPrev->Show();
            pCur->Show();

            pPrev->SetEnd( *pCur->End() );
            mpRedlineTbl->DeleteAndDestroy( n );
            --n;
            if( pFnc )
                (pPrev->*pFnc)(0);
        }
    }
    _CHECK_REDLINE( this )
}

bool SwDoc::SplitRedline( const SwPaM& rRange )
{
    bool bChg = false;
    sal_uInt16 n = 0;
    const SwPosition* pStt = rRange.Start(),
                  * pEnd = pStt == rRange.GetPoint() ? rRange.GetMark()
                                                     : rRange.GetPoint();
    GetRedline( *pStt, &n );
    for( ; n < mpRedlineTbl->size() ; ++n )
    {
        SwRedline* pTmp = (*mpRedlineTbl)[ n ];
        SwPosition* pTStt = pTmp->Start(),
                  * pTEnd = pTStt == pTmp->GetPoint() ? pTmp->GetMark()
                                                      : pTmp->GetPoint();
        if( *pTStt <= *pStt && *pStt <= *pTEnd &&
            *pTStt <= *pEnd && *pEnd <= *pTEnd )
        {
            bChg = true;
            int nn = 0;
            if( *pStt == *pTStt )
                nn += 1;
            if( *pEnd == *pTEnd )
                nn += 2;

            SwRedline* pNew = 0;
            switch( nn )
            {
            case 0:
                pNew = new SwRedline( *pTmp );
                pTmp->SetEnd( *pStt, pTEnd );
                pNew->SetStart( *pEnd );
                break;

            case 1:
                *pTStt = *pEnd;
                break;

            case 2:
                *pTEnd = *pStt;
                break;

            case 3:
                pTmp->InvalidateRange();
                mpRedlineTbl->DeleteAndDestroy( n-- );
                pTmp = 0;
                break;
            }
            if( pTmp && !pTmp->HasValidRange() )
            {
                // re-insert
                mpRedlineTbl->Remove( n );
                mpRedlineTbl->Insert( pTmp, n );
            }
            if( pNew )
                mpRedlineTbl->Insert( pNew, n );
        }
        else if( *pEnd < *pTStt )
            break;
    }
    return bChg;
}

bool SwDoc::DeleteRedline( const SwPaM& rRange, bool bSaveInUndo,
                            sal_uInt16 nDelType )
{
    if( nsRedlineMode_t::REDLINE_IGNOREDELETE_REDLINES & meRedlineMode ||
        !rRange.HasMark() || *rRange.GetMark() == *rRange.GetPoint() )
        return sal_False;

    bool bChg = false;

    if (bSaveInUndo && GetIDocumentUndoRedo().DoesUndo())
    {
        SwUndoRedline* pUndo = new SwUndoRedline( UNDO_REDLINE, rRange );
        if( pUndo->GetRedlSaveCount() )
        {
            GetIDocumentUndoRedo().AppendUndo(pUndo);
        }
        else
            delete pUndo;
    }

    const SwPosition* pStt = rRange.Start(),
                    * pEnd = pStt == rRange.GetPoint() ? rRange.GetMark()
                                                       : rRange.GetPoint();
    sal_uInt16 n = 0;
    GetRedline( *pStt, &n );
    for( ; n < mpRedlineTbl->size() ; ++n )
    {
        SwRedline* pRedl = (*mpRedlineTbl)[ n ];
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
            mpRedlineTbl->DeleteAndDestroy( n-- );
            bChg = true;
            break;

        case POS_OVERLAP_BEFORE:
                pRedl->InvalidateRange();
                pRedl->SetStart( *pEnd, pRStt );
                // re-insert
                mpRedlineTbl->Remove( n );
                mpRedlineTbl->Insert( pRedl );
                --n;
            break;

        case POS_OVERLAP_BEHIND:
                pRedl->InvalidateRange();
                pRedl->SetEnd( *pStt, pREnd );
                if( !pRedl->HasValidRange() )
                {
                    // re-insert
                    mpRedlineTbl->Remove( n );
                    mpRedlineTbl->Insert( pRedl );
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
                    mpRedlineTbl->Remove( n );
                    mpRedlineTbl->Insert( pRedl );
                    --n;
                }
                else
                {
                    SwRedline* pCpy;
                    if( *pREnd != *pEnd )
                    {
                        pCpy = new SwRedline( *pRedl );
                        pCpy->SetStart( *pEnd );
                    }
                    else
                        pCpy = 0;
                    pRedl->SetEnd( *pStt, pREnd );
                    if( !pRedl->HasValidRange() )
                    {
                        // re-insert
                        mpRedlineTbl->Remove( pRedl );
                        mpRedlineTbl->Insert( pRedl );
                        --n;
                    }
                    if( pCpy )
                        mpRedlineTbl->Insert( pCpy );
                }
            }
            break;

        case POS_COLLIDE_END:
        case POS_BEFORE:
            n = mpRedlineTbl->size();
            break;
        default:
            break;
        }
    }

    if( bChg )
        SetModified();

    return bChg;
}

bool SwDoc::DeleteRedline( const SwStartNode& rNode, bool bSaveInUndo,
                            sal_uInt16 nDelType )
{
    SwPaM aTemp(*rNode.EndOfSectionNode(), rNode);
    return DeleteRedline(aTemp, bSaveInUndo, nDelType);
}

sal_uInt16 SwDoc::GetRedlinePos( const SwNode& rNd, sal_uInt16 nType ) const
{
    const sal_uLong nNdIdx = rNd.GetIndex();
    for( sal_uInt16 n = 0; n < mpRedlineTbl->size() ; ++n )
    {
        const SwRedline* pTmp = (*mpRedlineTbl)[ n ];
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
}

const SwRedline* SwDoc::GetRedline( const SwPosition& rPos,
                                    sal_uInt16* pFndPos ) const
{
    sal_uInt16 nO = mpRedlineTbl->size(), nM, nU = 0;
    if( nO > 0 )
    {
        nO--;
        while( nU <= nO )
        {
            nM = nU + ( nO - nU ) / 2;
            const SwRedline* pRedl = (*mpRedlineTbl)[ nM ];
            const SwPosition* pStt = pRedl->Start();
            const SwPosition* pEnd = pStt == pRedl->GetPoint()
                                        ? pRedl->GetMark()
                                        : pRedl->GetPoint();
            if( pEnd == pStt
                    ? *pStt == rPos
                    : ( *pStt <= rPos && rPos < *pEnd ) )
            {
                while( nM && rPos == *(*mpRedlineTbl)[ nM - 1 ]->End() &&
                    rPos == *(*mpRedlineTbl)[ nM - 1 ]->Start() )
                {
                    --nM;
                    pRedl = (*mpRedlineTbl)[ nM ];
                }
                // if there are format and insert changes in the same position
                // show insert change first.
                // since the redlines are sorted by position, only check the redline
                // before and after the current redline
                if( nsRedlineType_t::REDLINE_FORMAT == pRedl->GetType() )
                {
                    if( nM && rPos >= *(*mpRedlineTbl)[ nM - 1 ]->Start() &&
                        rPos <= *(*mpRedlineTbl)[ nM - 1 ]->End() &&
                        ( nsRedlineType_t::REDLINE_INSERT == (*mpRedlineTbl)[ nM - 1 ]->GetType() ) )
                    {
                        --nM;
                        pRedl = (*mpRedlineTbl)[ nM ];
                    }
                    else if( ( nM + 1 ) <= nO && rPos >= *(*mpRedlineTbl)[ nM + 1 ]->Start() &&
                        rPos <= *(*mpRedlineTbl)[ nM + 1 ]->End() &&
                        ( nsRedlineType_t::REDLINE_INSERT == (*mpRedlineTbl)[ nM + 1 ]->GetType() ) )
                    {
                        ++nM;
                        pRedl = (*mpRedlineTbl)[ nM ];
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
                return 0;
            }
            else
                nO = nM - 1;
        }
    }
    if( pFndPos )
        *pFndPos = nU;
    return 0;
}

typedef sal_Bool (*Fn_AcceptReject)( SwRedlineTbl& rArr, sal_uInt16& rPos,
                        sal_Bool bCallDelete,
                        const SwPosition* pSttRng,
                        const SwPosition* pEndRng);

static sal_Bool lcl_AcceptRedline( SwRedlineTbl& rArr, sal_uInt16& rPos,
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
                        SwRedline* pNew = new SwRedline( *pRedl );
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
                bRet = sal_False;
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
            const SwPosition *pDelStt = 0, *pDelEnd = 0;
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

static sal_Bool lcl_RejectRedline( SwRedlineTbl& rArr, sal_uInt16& rPos,
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
            bool bCheck = false, bReplace = false;

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
                        bReplace = true;
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
                        bCheck = true;
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
                bReplace = true;
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
                bRet = sal_False;
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
        bRet = sal_False;
    }
    return bRet;
}

static const SwRedline* lcl_FindCurrRedline( const SwPosition& rSttPos,
                                        sal_uInt16& rPos,
                                        bool bNext = true )
{
    const SwRedline* pFnd = 0;
    const SwRedlineTbl& rArr = rSttPos.nNode.GetNode().GetDoc()->GetRedlineTbl();
    for( ; rPos < rArr.size() ; ++rPos )
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

static int lcl_AcceptRejectRedl( Fn_AcceptReject fn_AcceptReject,
                            SwRedlineTbl& rArr, sal_Bool bCallDelete,
                            const SwPaM& rPam)
{
    sal_uInt16 n = 0;
    int nCount = 0;

    const SwPosition* pStt = rPam.Start(),
                    * pEnd = pStt == rPam.GetPoint() ? rPam.GetMark()
                                                     : rPam.GetPoint();
    const SwRedline* pFnd = lcl_FindCurrRedline( *pStt, n, true );
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

bool SwDoc::AcceptRedline( sal_uInt16 nPos, bool bCallDelete )
{
    sal_Bool bRet = sal_False;

    // Switch to visible in any case
    if( (nsRedlineMode_t::REDLINE_SHOW_INSERT | nsRedlineMode_t::REDLINE_SHOW_DELETE) !=
        (nsRedlineMode_t::REDLINE_SHOW_MASK & meRedlineMode) )
      SetRedlineMode( (RedlineMode_t)(nsRedlineMode_t::REDLINE_SHOW_INSERT | nsRedlineMode_t::REDLINE_SHOW_DELETE | meRedlineMode));

    SwRedline* pTmp = (*mpRedlineTbl)[ nPos ];
    if( pTmp->HasMark() && pTmp->IsVisible() )
    {
        if (GetIDocumentUndoRedo().DoesUndo())
        {
            SwRewriter aRewriter;

            aRewriter.AddRule(UndoArg1, pTmp->GetDescr());
            GetIDocumentUndoRedo().StartUndo(UNDO_ACCEPT_REDLINE, &aRewriter);
        }

        int nLoopCnt = 2;
        sal_uInt16 nSeqNo = pTmp->GetSeqNo();

        do {

            if (GetIDocumentUndoRedo().DoesUndo())
            {
                SwUndo *const pUndo( new SwUndoAcceptRedline(*pTmp) );
                GetIDocumentUndoRedo().AppendUndo(pUndo);
            }

            bRet |= lcl_AcceptRedline( *mpRedlineTbl, nPos, bCallDelete );

            if( nSeqNo )
            {
                if( USHRT_MAX == nPos )
                    nPos = 0;
                sal_uInt16 nFndPos = 2 == nLoopCnt
                                    ? mpRedlineTbl->FindNextSeqNo( nSeqNo, nPos )
                                    : mpRedlineTbl->FindPrevSeqNo( nSeqNo, nPos );
                if( USHRT_MAX != nFndPos || ( 0 != ( --nLoopCnt ) &&
                    USHRT_MAX != ( nFndPos =
                        mpRedlineTbl->FindPrevSeqNo( nSeqNo, nPos ))) )
                    pTmp = (*mpRedlineTbl)[ nPos = nFndPos ];
                else
                    nLoopCnt = 0;
            }
            else
                nLoopCnt = 0;

        } while( nLoopCnt );

        if( bRet )
        {
            CompressRedlines();
            SetModified();
        }

        if (GetIDocumentUndoRedo().DoesUndo())
        {
            GetIDocumentUndoRedo().EndUndo(UNDO_END, 0);
        }
    }
    return bRet;
}

bool SwDoc::AcceptRedline( const SwPaM& rPam, bool bCallDelete )
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

    if (GetIDocumentUndoRedo().DoesUndo())
    {
        GetIDocumentUndoRedo().StartUndo( UNDO_ACCEPT_REDLINE, NULL );
        GetIDocumentUndoRedo().AppendUndo( new SwUndoAcceptRedline( aPam ));
    }

    int nRet = lcl_AcceptRejectRedl( lcl_AcceptRedline, *mpRedlineTbl,
                                     bCallDelete, aPam );
    if( nRet > 0 )
    {
        CompressRedlines();
        SetModified();
    }
    if (GetIDocumentUndoRedo().DoesUndo())
    {
        String aTmpStr;

        {
            SwRewriter aRewriter;
            aRewriter.AddRule(UndoArg1, OUString::number(nRet));
            aTmpStr = aRewriter.Apply(String(SW_RES(STR_N_REDLINES)));
        }

        SwRewriter aRewriter;
        aRewriter.AddRule(UndoArg1, aTmpStr);

        GetIDocumentUndoRedo().EndUndo( UNDO_ACCEPT_REDLINE, &aRewriter );
    }
    return nRet != 0;
}

bool SwDoc::RejectRedline( sal_uInt16 nPos, bool bCallDelete )
{
    sal_Bool bRet = sal_False;

    // Switch to visible in any case
    if( (nsRedlineMode_t::REDLINE_SHOW_INSERT | nsRedlineMode_t::REDLINE_SHOW_DELETE) !=
        (nsRedlineMode_t::REDLINE_SHOW_MASK & meRedlineMode) )
      SetRedlineMode( (RedlineMode_t)(nsRedlineMode_t::REDLINE_SHOW_INSERT | nsRedlineMode_t::REDLINE_SHOW_DELETE | meRedlineMode));

    SwRedline* pTmp = (*mpRedlineTbl)[ nPos ];
    if( pTmp->HasMark() && pTmp->IsVisible() )
    {
        if (GetIDocumentUndoRedo().DoesUndo())
        {
            SwRewriter aRewriter;

            aRewriter.AddRule(UndoArg1, pTmp->GetDescr());
            GetIDocumentUndoRedo().StartUndo(UNDO_REJECT_REDLINE, &aRewriter);
        }

        int nLoopCnt = 2;
        sal_uInt16 nSeqNo = pTmp->GetSeqNo();

        do {

            if (GetIDocumentUndoRedo().DoesUndo())
            {
                SwUndo *const pUndo( new SwUndoRejectRedline( *pTmp ) );
                GetIDocumentUndoRedo().AppendUndo(pUndo);
            }

            bRet |= lcl_RejectRedline( *mpRedlineTbl, nPos, bCallDelete );

            if( nSeqNo )
            {
                if( USHRT_MAX == nPos )
                    nPos = 0;
                sal_uInt16 nFndPos = 2 == nLoopCnt
                                    ? mpRedlineTbl->FindNextSeqNo( nSeqNo, nPos )
                                    : mpRedlineTbl->FindPrevSeqNo( nSeqNo, nPos );
                if( USHRT_MAX != nFndPos || ( 0 != ( --nLoopCnt ) &&
                    USHRT_MAX != ( nFndPos =
                            mpRedlineTbl->FindPrevSeqNo( nSeqNo, nPos ))) )
                    pTmp = (*mpRedlineTbl)[ nPos = nFndPos ];
                else
                    nLoopCnt = 0;
            }
            else
                nLoopCnt = 0;

        } while( nLoopCnt );

        if( bRet )
        {
            CompressRedlines();
            SetModified();
        }

        if (GetIDocumentUndoRedo().DoesUndo())
        {
            GetIDocumentUndoRedo().EndUndo(UNDO_END, 0);
        }
    }
    return bRet;
}

bool SwDoc::RejectRedline( const SwPaM& rPam, bool bCallDelete )
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

    if (GetIDocumentUndoRedo().DoesUndo())
    {
        GetIDocumentUndoRedo().StartUndo( UNDO_REJECT_REDLINE, NULL );
        GetIDocumentUndoRedo().AppendUndo( new SwUndoRejectRedline(aPam) );
    }

    int nRet = lcl_AcceptRejectRedl( lcl_RejectRedline, *mpRedlineTbl,
                                        bCallDelete, aPam );
    if( nRet > 0 )
    {
        CompressRedlines();
        SetModified();
    }
    if (GetIDocumentUndoRedo().DoesUndo())
    {
        String aTmpStr;

        {
            SwRewriter aRewriter;
            aRewriter.AddRule(UndoArg1, OUString::number(nRet));
            aTmpStr = aRewriter.Apply(String(SW_RES(STR_N_REDLINES)));
        }

        SwRewriter aRewriter;
        aRewriter.AddRule(UndoArg1, aTmpStr);

        GetIDocumentUndoRedo().EndUndo( UNDO_REJECT_REDLINE, &aRewriter );
    }

    return nRet != 0;
}

const SwRedline* SwDoc::SelNextRedline( SwPaM& rPam ) const
{
    rPam.DeleteMark();
    rPam.SetMark();

    SwPosition& rSttPos = *rPam.GetPoint();
    SwPosition aSavePos( rSttPos );
    bool bRestart;

    // If the starting positon points to the last valid ContentNode,
    // we take the next Redline in any case.
    sal_uInt16 n = 0;
    const SwRedline* pFnd = lcl_FindCurrRedline( rSttPos, n, true );
    if( pFnd )
    {
        const SwPosition* pEnd = pFnd->End();
        if( !pEnd->nNode.GetNode().IsCntntNode() )
        {
            SwNodeIndex aTmp( pEnd->nNode );
            SwCntntNode* pCNd = GetNodes().GoPrevSection( &aTmp );
            if( !pCNd || ( aTmp == rSttPos.nNode &&
                pCNd->Len() == rSttPos.nContent.GetIndex() ))
                pFnd = 0;
        }
        if( pFnd )
            rSttPos = *pFnd->End();
    }

    do {
        bRestart = false;

        for( ; !pFnd && n < mpRedlineTbl->size(); ++n )
        {
            pFnd = (*mpRedlineTbl)[ n ];
            if( pFnd->HasMark() && pFnd->IsVisible() )
            {
                *rPam.GetMark() = *pFnd->Start();
                rSttPos = *pFnd->End();
                break;
            }
            else
                pFnd = 0;
        }

        if( pFnd )
        {
            // Merge all of the same type and author that are
            // consecutive into one Selection.
            const SwPosition* pPrevEnd = pFnd->End();
            while( ++n < mpRedlineTbl->size() )
            {
                const SwRedline* pTmp = (*mpRedlineTbl)[ n ];
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
            const SwRedline* pSaveFnd = pFnd;

            SwCntntNode* pCNd;
            SwNodeIndex* pIdx = &rPam.GetMark()->nNode;
            if( !pIdx->GetNode().IsCntntNode() &&
                0 != ( pCNd = GetNodes().GoNextSection( pIdx )) )
            {
                if( *pIdx <= rPam.GetPoint()->nNode )
                    rPam.GetMark()->nContent.Assign( pCNd, 0 );
                else
                    pFnd = 0;
            }

            if( pFnd )
            {
                pIdx = &rPam.GetPoint()->nNode;
                if( !pIdx->GetNode().IsCntntNode() &&
                    0 != ( pCNd = GetNodes().GoPrevSection( pIdx )) )
                {
                    if( *pIdx >= rPam.GetMark()->nNode )
                        rPam.GetPoint()->nContent.Assign( pCNd, pCNd->Len() );
                    else
                        pFnd = 0;
                }
            }

            if( !pFnd || *rPam.GetMark() == *rPam.GetPoint() )
            {
                if( n < mpRedlineTbl->size() )
                {
                    bRestart = true;
                    *rPam.GetPoint() = *pSaveFnd->End();
                }
                else
                {
                    rPam.DeleteMark();
                    *rPam.GetPoint() = aSavePos;
                }
                pFnd = 0;
            }
        }
    } while( bRestart );

    return pFnd;
}

const SwRedline* SwDoc::SelPrevRedline( SwPaM& rPam ) const
{
    rPam.DeleteMark();
    rPam.SetMark();

    SwPosition& rSttPos = *rPam.GetPoint();
    SwPosition aSavePos( rSttPos );
    bool bRestart;

    // If the starting positon points to the last valid ContentNode,
    // we take the previous Redline in any case.
    sal_uInt16 n = 0;
    const SwRedline* pFnd = lcl_FindCurrRedline( rSttPos, n, false );
    if( pFnd )
    {
        const SwPosition* pStt = pFnd->Start();
        if( !pStt->nNode.GetNode().IsCntntNode() )
        {
            SwNodeIndex aTmp( pStt->nNode );
            SwCntntNode* pCNd = GetNodes().GoNextSection( &aTmp );
            if( !pCNd || ( aTmp == rSttPos.nNode &&
                !rSttPos.nContent.GetIndex() ))
                pFnd = 0;
        }
        if( pFnd )
            rSttPos = *pFnd->Start();
    }

    do {
        bRestart = false;

        while( !pFnd && 0 < n )
        {
            pFnd = (*mpRedlineTbl)[ --n ];
            if( pFnd->HasMark() && pFnd->IsVisible() )
            {
                *rPam.GetMark() = *pFnd->End();
                rSttPos = *pFnd->Start();
            }
            else
                pFnd = 0;
        }

        if( pFnd )
        {
            // Merge all of the same type and author that are
            // consecutive into one Selection.
            const SwPosition* pNextStt = pFnd->Start();
            while( 0 < n )
            {
                const SwRedline* pTmp = (*mpRedlineTbl)[ --n ];
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
            const SwRedline* pSaveFnd = pFnd;

            SwCntntNode* pCNd;
            SwNodeIndex* pIdx = &rPam.GetMark()->nNode;
            if( !pIdx->GetNode().IsCntntNode() &&
                0 != ( pCNd = GetNodes().GoPrevSection( pIdx )) )
            {
                if( *pIdx >= rPam.GetPoint()->nNode )
                    rPam.GetMark()->nContent.Assign( pCNd, pCNd->Len() );
                else
                    pFnd = 0;
            }

            if( pFnd )
            {
                pIdx = &rPam.GetPoint()->nNode;
                if( !pIdx->GetNode().IsCntntNode() &&
                    0 != ( pCNd = GetNodes().GoNextSection( pIdx )) )
                {
                    if( *pIdx <= rPam.GetMark()->nNode )
                        rPam.GetPoint()->nContent.Assign( pCNd, 0 );
                    else
                        pFnd = 0;
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
                pFnd = 0;
            }
        }
    } while( bRestart );

    return pFnd;
}

// Set comment at the Redline
bool SwDoc::SetRedlineComment( const SwPaM& rPaM, const String& rS )
{
    bool bRet = false;
    const SwPosition* pStt = rPaM.Start(),
                    * pEnd = pStt == rPaM.GetPoint() ? rPaM.GetMark()
                                                     : rPaM.GetPoint();
    sal_uInt16 n = 0;
    if( lcl_FindCurrRedline( *pStt, n, true ) )
    {
        for( ; n < mpRedlineTbl->size(); ++n )
        {
            bRet = true;
            SwRedline* pTmp = (*mpRedlineTbl)[ n ];
            if( pStt != pEnd && *pTmp->Start() > *pEnd )
                break;

            pTmp->SetComment( rS );
            if( *pTmp->End() >= *pEnd )
                break;
        }
    }
    if( bRet )
        SetModified();

    return bRet;
}

// Create a new author if necessary
sal_uInt16 SwDoc::GetRedlineAuthor()
{
    return SW_MOD()->GetRedlineAuthor();
}

/// Insert new author into the Table for the Readers etc.
sal_uInt16 SwDoc::InsertRedlineAuthor( const String& rNew )
{
    return SW_MOD()->InsertRedlineAuthor(rNew);
}

void SwDoc::UpdateRedlineAttr()
{
    const SwRedlineTbl& rTbl = GetRedlineTbl();
    for( sal_uInt16 n = 0; n < rTbl.size(); ++n )
    {
        SwRedline* pRedl = rTbl[ n ];
        if( pRedl->IsVisible() )
            pRedl->InvalidateRange();
    }
}

/// Set comment text for the Redline, which is inserted later on via
/// AppendRedline. Is used by Autoformat.
/// A null pointer resets the mode. The pointer is not copied, so it
/// needs to stay valid!
void SwDoc::SetAutoFmtRedlineComment( const String* pTxt, sal_uInt16 nSeqNo )
{
    mbIsAutoFmtRedline = 0 != pTxt;
    if( pTxt )
    {
        if( !mpAutoFmtRedlnComment )
            mpAutoFmtRedlnComment = new String( *pTxt );
        else
            *mpAutoFmtRedlnComment = *pTxt;
    }
    else
        delete mpAutoFmtRedlnComment, mpAutoFmtRedlnComment = 0;

    mnAutoFmtRedlnCommentNo = nSeqNo;
}

void SwDoc::SetRedlinePassword(
            /*[in]*/const uno::Sequence <sal_Int8>& rNewPassword)
{
    maRedlinePasswd = rNewPassword;
    SetModified();
}

bool SwRedlineTbl::Insert( SwRedline* p, bool bIns )
{
    bool bRet = false;
    if( p->HasValidRange() )
    {
        bRet = insert( p ).second;
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

bool SwRedlineTbl::Insert( SwRedline* p, sal_uInt16& rP, bool bIns )
{
    bool bRet = false;
    if( p->HasValidRange() )
    {
        std::pair<_SwRedlineTbl::const_iterator, bool> rv = insert( p );
        rP = rv.first - begin();
        bRet = rv.second;
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

bool SwRedlineTbl::InsertWithValidRanges( SwRedline* p, sal_uInt16* pInsPos )
{
    // Create valid "sub-ranges" from the Selection
    bool bAnyIns = false;
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
                Insert( pNew, nInsPos ) )
            {
                pNew->CallDisplayFunc();
                bAnyIns = true;
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

bool CompareSwRedlineTbl::operator()(SwRedline* const &lhs, SwRedline* const &rhs) const
{
    return *lhs < *rhs;
}

_SwRedlineTbl::~_SwRedlineTbl()
{
    DeleteAndDestroyAll();
}

sal_uInt16 SwRedlineTbl::GetPos(const SwRedline* p) const
{
    const_iterator it = find(const_cast<SwRedline* const>(p));
    if( it == end() )
        return USHRT_MAX;
    return it - begin();
}

bool SwRedlineTbl::Remove( const SwRedline* p )
{
    sal_uInt16 nPos = GetPos(p);
    if (nPos != USHRT_MAX)
        Remove(nPos);
    return nPos != USHRT_MAX;
}

void SwRedlineTbl::Remove( sal_uInt16 nP )
{
    SwDoc* pDoc = 0;
    if( !nP && 1 == size() )
        pDoc = front()->GetDoc();

    erase( begin() + nP );

    ViewShell* pSh;
    if( pDoc && !pDoc->IsInDtor() &&
        0 != ( pSh = pDoc->GetCurrentViewShell()) ) //swmod 071108//swmod 071225
        pSh->InvalidateWindows( SwRect( 0, 0, LONG_MAX, LONG_MAX ) );
}

void SwRedlineTbl::DeleteAndDestroyAll()
{
    DeleteAndDestroy(0, size());
}

void SwRedlineTbl::DeleteAndDestroy( sal_uInt16 nP, sal_uInt16 nL )
{
    SwDoc* pDoc = 0;
    if( !nP && nL && nL == size() )
        pDoc = front()->GetDoc();

    for( const_iterator it = begin() + nP; it != begin() + nP + nL; ++it )
        delete *it;
    erase( begin() + nP, begin() + nP + nL );

    ViewShell* pSh;
    if( pDoc && !pDoc->IsInDtor() &&
        0 != ( pSh = pDoc->GetCurrentViewShell() ) )    //swmod 071108//swmod 071225
        pSh->InvalidateWindows( SwRect( 0, 0, LONG_MAX, LONG_MAX ) );
}

/// Find the next or preceding Redline with the same seq.no.
/// We can limit the search using look ahead.
/// 0 or USHRT_MAX searches the whole array.
sal_uInt16 SwRedlineTbl::FindNextOfSeqNo( sal_uInt16 nSttPos, sal_uInt16 nLookahead ) const
{
    return nSttPos + 1 < (sal_uInt16)size()
                ? FindNextSeqNo( operator[]( nSttPos )->GetSeqNo(), nSttPos+1, nLookahead )
                : USHRT_MAX;
}

sal_uInt16 SwRedlineTbl::FindPrevOfSeqNo( sal_uInt16 nSttPos, sal_uInt16 nLookahead ) const
{
    return nSttPos ? FindPrevSeqNo( operator[]( nSttPos )->GetSeqNo(), nSttPos-1, nLookahead )
                   : USHRT_MAX;
}

sal_uInt16 SwRedlineTbl::FindNextSeqNo( sal_uInt16 nSeqNo, sal_uInt16 nSttPos,
                                    sal_uInt16 nLookahead ) const
{
    sal_uInt16 nRet = USHRT_MAX, nEnd;
    if( nSeqNo && nSttPos < size() )
    {
        nEnd = size();
        if( nLookahead && USHRT_MAX != nLookahead &&
            static_cast<size_t>(nSttPos + nLookahead) < size() )
            nEnd = nSttPos + nLookahead;

        for( ; nSttPos < nEnd; ++nSttPos )
            if( nSeqNo == operator[]( nSttPos )->GetSeqNo() )
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
    if( nSeqNo && nSttPos < size() )
    {
        nEnd = 0;
        if( nLookahead && USHRT_MAX != nLookahead && nSttPos > nLookahead )
            nEnd = nSttPos - nLookahead;

        ++nSttPos;
        while( nSttPos > nEnd )
            if( nSeqNo == operator[]( --nSttPos )->GetSeqNo() )
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
            rMark.nContent.Assign(pTNd, pTNd->GetTxt().getLength());

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
    while( true )
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
    aStamp.SetNanoSec( 0 );
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

/// ExtraData is copied. The Pointer's ownership is thus NOT transferred
/// to the Redline Object!
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

/// Do we have a valid Selection?
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
            aHt.nEnd = (n == nEndNd)
                ? nEndCnt
                : static_cast<SwTxtNode*>(pNd)->GetTxt().getLength();
            ((SwTxtNode*)pNd)->ModifyNotification( &aHt, &aHt );
        }
    }
}

/** Calculates the start and end position of the intersection rTmp and
    text node nNdIdx */
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
            for( sal_uInt16 n = 0; n < rTbl.size(); ++n )
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
            for( sal_uInt16 n = 0; n < rTbl.size(); ++n )
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
                for( bool bBreak = false; !bBreak && n > 0; )
                {
                    --n;
                    bBreak = true;
                    if( rTbl[ n ]->GetBound(sal_True) == *aPam.GetPoint() )
                    {
                        rTbl[ n ]->GetBound(sal_True) = *pEnd;
                        bBreak = false;
                    }
                    if( rTbl[ n ]->GetBound(sal_False) == *aPam.GetPoint() )
                    {
                        rTbl[ n ]->GetBound(sal_False) = *pEnd;
                        bBreak = false;
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
        std::vector<SwPosition*> aBeforeArr, aBehindArr;
        sal_uInt16 nMyPos = rTbl.GetPos( this );
        OSL_ENSURE( this, "this is not in the array?" );
        bool bBreak = false;
        sal_uInt16 n;

        for( n = nMyPos+1; !bBreak && n < rTbl.size(); ++n )
        {
            bBreak = true;
            if( rTbl[ n ]->GetBound(sal_True) == *GetPoint() )
            {
                aBehindArr.push_back( &rTbl[ n ]->GetBound(sal_True) );
                bBreak = false;
            }
            if( rTbl[ n ]->GetBound(sal_False) == *GetPoint() )
            {
                aBehindArr.push_back( &rTbl[ n ]->GetBound(sal_False) );
                bBreak = false;
            }
        }
        for( bBreak = false, n = nMyPos; !bBreak && n ; )
        {
            --n;
            bBreak = true;
            if( rTbl[ n ]->GetBound(sal_True) == *GetPoint() )
            {
                aBeforeArr.push_back( &rTbl[ n ]->GetBound(sal_True) );
                bBreak = false;
            }
            if( rTbl[ n ]->GetBound(sal_False) == *GetPoint() )
            {
                aBeforeArr.push_back( &rTbl[ n ]->GetBound(sal_False) );
                bBreak = false;
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
        for( n = 0; n < aBeforeArr.size(); ++n )
            *aBeforeArr[ n ] = *Start();
        for( n = 0; n < aBehindArr.size(); ++n )
            *aBehindArr[ n ] = *End();
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

OUString SwRedline::GetAuthorString( sal_uInt16 nPos ) const
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
    aRewriter.AddRule(UndoArg1, aTmpStr);

    aResult = aRewriter.Apply(aResult);

    if (bDeletePaM)
        delete pPaM;

    return aResult;
}

bool SwDoc::IsInRedlines(const SwNode & rNode) const
{
    SwPosition aPos(rNode);
    SwNode & rEndOfRedlines = GetNodes().GetEndOfRedlines();
    SwPaM aPam(SwPosition(*rEndOfRedlines.StartOfSectionNode()),
               SwPosition(rEndOfRedlines));

    return aPam.ContainsPosition(aPos);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
