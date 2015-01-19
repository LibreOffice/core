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
#include <svl/itemiter.hxx>
#include <sfx2/app.hxx>
#include <editeng/colritem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/crossedoutitem.hxx>
#include <swmodule.hxx>
#include <doc.hxx>
#include <docredln.hxx>
#include <IDocumentUndoRedo.hxx>
#include <DocumentContentOperationsManager.hxx>
#include <IDocumentRedlineAccess.hxx>
#include <IDocumentState.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <IDocumentStylePoolAccess.hxx>
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

#ifdef DBG_UTIL

    void sw_DebugRedline( const SwDoc* pDoc )
    {
        static sal_uInt16 nWatch = 0;
        const SwRedlineTbl& rTbl = pDoc->getIDocumentRedlineAccess().GetRedlineTbl();
        for( sal_uInt16 n = 0; n < rTbl.size(); ++n )
        {
            sal_uInt16 nDummy = 0;
            const SwRangeRedline* pCurrent = rTbl[ n ];
            const SwRangeRedline* pNext = n+1 < (sal_uInt16)rTbl.size() ? rTbl[ n+1 ] : 0;
            if( pCurrent == pNext )
                ++nDummy;
            if( n == nWatch )
                ++nDummy; // Possible debugger breakpoint
        }
    }

#endif


SwExtraRedlineTbl::~SwExtraRedlineTbl()
{
    DeleteAndDestroyAll();
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

bool SwExtraRedlineTbl::DeleteAllTableRedlines( SwDoc* pDoc, const SwTable& rTable, bool bSaveInUndo, sal_uInt16 nRedlineTypeToDelete )
{
    if( nsRedlineMode_t::REDLINE_IGNOREDELETE_REDLINES & pDoc->getIDocumentRedlineAccess().GetRedlineMode() )
        return false;

    bool bChg = false;

    if (bSaveInUndo && pDoc->GetIDocumentUndoRedo().DoesUndo())
    {
        // #TODO - Add 'Undo' support for deleting 'Table Cell' redlines
        /*
        SwUndoRedline* pUndo = new SwUndoRedline( UNDO_REDLINE, rRange );
        if( pUndo->GetRedlSaveCount() )
        {
            GetIDocumentUndoRedo().AppendUndo(pUndo);
        }
        else
            delete pUndo;
        */
    }

    for(sal_uInt16 nCurRedlinePos = 0; nCurRedlinePos < GetSize(); ++nCurRedlinePos )
    {
        SwExtraRedline* pExtraRedline = GetRedline(nCurRedlinePos);
        const SwTableCellRedline* pTableCellRedline = dynamic_cast<const SwTableCellRedline*>(pExtraRedline);
        if (pTableCellRedline)
        {
            const SwTableBox *pRedTabBox = &pTableCellRedline->GetTableBox();
            const SwTable& pRedTable = pRedTabBox->GetSttNd()->FindTableNode()->GetTable();
            if ( &pRedTable == &rTable )
            {
                // Redline for this table
                const SwRedlineData& aRedlineData = pTableCellRedline->GetRedlineData();
                sal_uInt16 nRedlineType = aRedlineData.GetType();

                // Check if this redline object type should be deleted
                if( USHRT_MAX != nRedlineTypeToDelete && nRedlineTypeToDelete != nRedlineType )
                    continue;

                DeleteAndDestroy( nCurRedlinePos );
                bChg = true;
            }
        }
        else
        {
            const SwTableRowRedline* pTableRowRedline = dynamic_cast<const SwTableRowRedline*>(pExtraRedline);
            if (pTableRowRedline)
            {
                const SwTableLine *pRedTabLine = &pTableRowRedline->GetTableLine();
                const SwTableBoxes &pRedTabBoxes = pRedTabLine->GetTabBoxes();
                const SwTable& pRedTable = pRedTabBoxes[0]->GetSttNd()->FindTableNode()->GetTable();
                if ( &pRedTable == &rTable )
                {
                    // Redline for this table
                    const SwRedlineData& aRedlineData = pTableRowRedline->GetRedlineData();
                    sal_uInt16 nRedlineType = aRedlineData.GetType();

                    // Check if this redline object type should be deleted
                    if( USHRT_MAX != nRedlineTypeToDelete && nRedlineTypeToDelete != nRedlineType )
                        continue;

                    DeleteAndDestroy( nCurRedlinePos );
                    bChg = true;
                }
            }
        }
    }

    if( bChg )
        pDoc->getIDocumentState().SetModified();

    return bChg;
}

bool SwExtraRedlineTbl::DeleteTableRowRedline( SwDoc* pDoc, const SwTableLine& rTableLine, bool bSaveInUndo, sal_uInt16 nRedlineTypeToDelete )
{
    if( nsRedlineMode_t::REDLINE_IGNOREDELETE_REDLINES & pDoc->getIDocumentRedlineAccess().GetRedlineMode() )
        return false;

    bool bChg = false;

    if (bSaveInUndo && pDoc->GetIDocumentUndoRedo().DoesUndo())
    {
        // #TODO - Add 'Undo' support for deleting 'Table Cell' redlines
        /*
        SwUndoRedline* pUndo = new SwUndoRedline( UNDO_REDLINE, rRange );
        if( pUndo->GetRedlSaveCount() )
        {
            GetIDocumentUndoRedo().AppendUndo(pUndo);
        }
        else
            delete pUndo;
        */
    }

    for(sal_uInt16 nCurRedlinePos = 0; nCurRedlinePos < GetSize(); ++nCurRedlinePos )
    {
        SwExtraRedline* pExtraRedline = GetRedline(nCurRedlinePos);
        const SwTableRowRedline* pTableRowRedline = dynamic_cast<const SwTableRowRedline*>(pExtraRedline);
        const SwTableLine *pRedTabLine = pTableRowRedline ? &pTableRowRedline->GetTableLine() : NULL;
        if ( pRedTabLine == &rTableLine )
        {
            // Redline for this table row
            const SwRedlineData& aRedlineData = pTableRowRedline->GetRedlineData();
            sal_uInt16 nRedlineType = aRedlineData.GetType();

            // Check if this redline object type should be deleted
            if( USHRT_MAX != nRedlineTypeToDelete && nRedlineTypeToDelete != nRedlineType )
                continue;

            DeleteAndDestroy( nCurRedlinePos );
            bChg = true;
        }
    }

    if( bChg )
        pDoc->getIDocumentState().SetModified();

    return bChg;
}

bool SwExtraRedlineTbl::DeleteTableCellRedline( SwDoc* pDoc, const SwTableBox& rTableBox, bool bSaveInUndo, sal_uInt16 nRedlineTypeToDelete )
{
    if( nsRedlineMode_t::REDLINE_IGNOREDELETE_REDLINES & pDoc->getIDocumentRedlineAccess().GetRedlineMode() )
        return false;

    bool bChg = false;

    if (bSaveInUndo && pDoc->GetIDocumentUndoRedo().DoesUndo())
    {
        // #TODO - Add 'Undo' support for deleting 'Table Cell' redlines
        /*
        SwUndoRedline* pUndo = new SwUndoRedline( UNDO_REDLINE, rRange );
        if( pUndo->GetRedlSaveCount() )
        {
            GetIDocumentUndoRedo().AppendUndo(pUndo);
        }
        else
            delete pUndo;
        */
    }

    for(sal_uInt16 nCurRedlinePos = 0; nCurRedlinePos < GetSize(); ++nCurRedlinePos )
    {
        SwExtraRedline* pExtraRedline = GetRedline(nCurRedlinePos);
        const SwTableCellRedline* pTableCellRedline = dynamic_cast<const SwTableCellRedline*>(pExtraRedline);
        const SwTableBox *pRedTabBox = pTableCellRedline ? &pTableCellRedline->GetTableBox() : NULL;
        if ( pRedTabBox == &rTableBox )
        {
            // Redline for this table cell
            const SwRedlineData& aRedlineData = pTableCellRedline->GetRedlineData();
            sal_uInt16 nRedlineType = aRedlineData.GetType();

            // Check if this redline object type should be deleted
            if( USHRT_MAX != nRedlineTypeToDelete && nRedlineTypeToDelete != nRedlineType )
                continue;

            DeleteAndDestroy( nCurRedlinePos );
            bChg = true;
        }
    }

    if( bChg )
        pDoc->getIDocumentState().SetModified();

    return bChg;
}

bool SwRedlineTbl::Insert( SwRangeRedline* p, bool bIns )
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
        OSL_ENSURE( false, "Redline: wrong range" );
    }
    return bRet;
}

bool SwRedlineTbl::Insert( SwRangeRedline* p, sal_uInt16& rP, bool bIns )
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
        OSL_ENSURE( false, "Redline: wrong range" );
    }
    return bRet;
}

bool SwRedlineTbl::InsertWithValidRanges( SwRangeRedline* p, sal_uInt16* pInsPos )
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

    SwRangeRedline* pNew = 0;
    sal_uInt16 nInsPos;

    if( aNewStt < *pEnd )
        do {
            if( !pNew )
                pNew = new SwRangeRedline( p->GetRedlineData(), aNewStt );
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
                    pC = GoPreviousNds( &pNew->GetPoint()->nNode, false ); // here we are.
                    if( pC )
                        pNew->GetPoint()->nContent.Assign( pC, 0 );
                    pTab = pNew->GetPoint()->nNode.GetNode().StartOfSectionNode()->FindTableNode();
                } while( pTab ); // If there is another table we have to repeat our step backwards
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
                pNew->HasValidRange() &&
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

bool CompareSwRedlineTbl::operator()(SwRangeRedline* const &lhs, SwRangeRedline* const &rhs) const
{
    return *lhs < *rhs;
}

_SwRedlineTbl::~_SwRedlineTbl()
{
    DeleteAndDestroyAll();
}

sal_uInt16 SwRedlineTbl::GetPos(const SwRangeRedline* p) const
{
    const_iterator it = find(const_cast<SwRangeRedline* const>(p));
    if( it == end() )
        return USHRT_MAX;
    return it - begin();
}

bool SwRedlineTbl::Remove( const SwRangeRedline* p )
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

    SwViewShell* pSh;
    if( pDoc && !pDoc->IsInDtor() &&
        0 != ( pSh = pDoc->getIDocumentLayoutAccess().GetCurrentViewShell()) )
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

    SwViewShell* pSh;
    if( pDoc && !pDoc->IsInDtor() &&
        0 != ( pSh = pDoc->getIDocumentLayoutAccess().GetCurrentViewShell() ) )
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

const SwRangeRedline* SwRedlineTbl::FindAtPosition( const SwPosition& rSttPos,
                                        sal_uInt16& rPos,
                                        bool bNext ) const
{
    const SwRangeRedline* pFnd = 0;
    for( ; rPos < size() ; ++rPos )
    {
        const SwRangeRedline* pTmp = (*this)[ rPos ];
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


SwRedlineExtraData::~SwRedlineExtraData()
{
}

void SwRedlineExtraData::Accept( SwPaM& ) const
{
}

void SwRedlineExtraData::Reject( SwPaM& ) const
{
}

bool SwRedlineExtraData::operator == ( const SwRedlineExtraData& ) const
{
    return false;
}

SwRedlineExtraData_FmtColl::SwRedlineExtraData_FmtColl( const OUString& rColl,
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
                            : pDoc->getIDocumentStylePoolAccess().GetTxtCollFromPool( nPoolId );
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
                pDoc->getIDocumentContentOperations().InsertItemSet( rPam, aTmp, 0 );
            }
            else
            {
                pDoc->getIDocumentContentOperations().InsertItemSet( rPam, *pSet, 0 );
            }
        }
        rPam.DeleteMark();
    }
}

bool SwRedlineExtraData_FmtColl::operator == ( const SwRedlineExtraData& r) const
{
    const SwRedlineExtraData_FmtColl& rCmp = static_cast<const SwRedlineExtraData_FmtColl&>(r);
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

    RedlineMode_t eOld = pDoc->getIDocumentRedlineAccess().GetRedlineMode();
    pDoc->getIDocumentRedlineAccess().SetRedlineMode_intern((RedlineMode_t)(eOld & ~(nsRedlineMode_t::REDLINE_ON | nsRedlineMode_t::REDLINE_IGNORE)));

    // Actually we need to reset the Attribute here!
    std::vector<sal_uInt16>::const_iterator it;
    for( it = aWhichIds.begin(); it != aWhichIds.end(); ++it )
    {
        pDoc->getIDocumentContentOperations().InsertPoolItem( rPam, *GetDfltAttr( *it ),
            nsSetAttrMode::SETATTR_DONTEXPAND );
    }

    pDoc->getIDocumentRedlineAccess().SetRedlineMode_intern( eOld );
}

bool SwRedlineExtraData_Format::operator == ( const SwRedlineExtraData& rCmp ) const
{
    bool nRet = true;
    size_t nEnd = aWhichIds.size();
    if( nEnd != static_cast<const SwRedlineExtraData_Format&>(rCmp).aWhichIds.size() )
        nRet = false;
    else
        for( size_t n = 0; n < nEnd; ++n )
            if( static_cast<const SwRedlineExtraData_Format&>(rCmp).aWhichIds[n] != aWhichIds[n])
            {
                nRet = false;
                break;
            }
    return nRet;
}

SwRedlineExtraData_FormattingChanges::SwRedlineExtraData_FormattingChanges( const SfxItemSet* pItemSet )
    : pSet(0)
{
    if( pItemSet && pItemSet->Count() )
        pSet = new SfxItemSet( *pItemSet );
}

SwRedlineExtraData_FormattingChanges::SwRedlineExtraData_FormattingChanges( const SwRedlineExtraData_FormattingChanges& rCpy )
    : SwRedlineExtraData()
{
    // Checking pointer pSet before accessing it for Count
    if( rCpy.pSet && rCpy.pSet->Count() )
    {
        pSet = new SfxItemSet( *(rCpy.pSet) );
    }
    else
    {
        pSet = 0;
    }
}

SwRedlineExtraData_FormattingChanges::~SwRedlineExtraData_FormattingChanges()
{
    delete pSet;
}

SwRedlineExtraData* SwRedlineExtraData_FormattingChanges::CreateNew() const
{
    return new SwRedlineExtraData_FormattingChanges( *this );
}

void SwRedlineExtraData_FormattingChanges::Reject( SwPaM& rPam ) const
{
    rPam.GetDoc();  // This is here just to prevent build 'warning'

    // ToDo: Add 'Reject' logic
}

bool SwRedlineExtraData_FormattingChanges::operator == ( const SwRedlineExtraData& rExtraData ) const
{
    const SwRedlineExtraData_FormattingChanges& rCmp = static_cast<const SwRedlineExtraData_FormattingChanges&>(rExtraData);

    if ( !pSet && !rCmp.pSet )
    {
        // Both SfxItemSet are null
        return true;
    }
    else if ( pSet && rCmp.pSet && *pSet == *rCmp.pSet )
    {
        // Both SfxItemSet exist and are equal
        return true;
    }
    return false;
}

SwRedlineData::SwRedlineData( RedlineType_t eT, sal_uInt16 nAut )
    : pNext( 0 ), pExtraData( 0 ),
    aStamp( DateTime::SYSTEM ),
    eType( eT ), nAuthor( nAut ), nSeqNo( 0 )
{
    aStamp.SetSec( 0 );
    aStamp.SetNanoSec( 0 );
}

SwRedlineData::SwRedlineData(
    const SwRedlineData& rCpy,
    bool bCpyNext )
    : pNext( ( bCpyNext && rCpy.pNext ) ? new SwRedlineData( *rCpy.pNext ) : 0 )
    , pExtraData( rCpy.pExtraData ? rCpy.pExtraData->CreateNew() : 0 )
    , sComment( rCpy.sComment )
    , aStamp( rCpy.aStamp )
    , eType( rCpy.eType )
    , nAuthor( rCpy.nAuthor )
    , nSeqNo( rCpy.nSeqNo )
{
}

// For sw3io: We now own pNext!
SwRedlineData::SwRedlineData(RedlineType_t eT, sal_uInt16 nAut, const DateTime& rDT,
    const OUString& rCmnt, SwRedlineData *pNxt, SwRedlineExtraData* pData)
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

    // Check if there is data - and if so - delete it
    if( pData )
        pExtraData = pData->CreateNew();
    else
        pExtraData = 0;
}

OUString SwRedlineData::GetDescr() const
{
    OUString aResult;

    aResult += SW_RES(STR_REDLINE_INSERT + GetType());

    return aResult;
}

SwRangeRedline::SwRangeRedline(RedlineType_t eTyp, const SwPaM& rPam )
    : SwPaM( *rPam.GetMark(), *rPam.GetPoint() ),
    pRedlineData( new SwRedlineData( eTyp, GetDoc()->getIDocumentRedlineAccess().GetRedlineAuthor() ) ),
    pCntntSect( 0 )
{
    bDelLastPara = bIsLastParaDelete = false;
    bIsVisible = true;
    if( !rPam.HasMark() )
        DeleteMark();
}

SwRangeRedline::SwRangeRedline( const SwRedlineData& rData, const SwPaM& rPam )
    : SwPaM( *rPam.GetMark(), *rPam.GetPoint() ),
    pRedlineData( new SwRedlineData( rData )),
    pCntntSect( 0 )
{
    bDelLastPara = bIsLastParaDelete = false;
    bIsVisible = true;
    if( !rPam.HasMark() )
        DeleteMark();
}

SwRangeRedline::SwRangeRedline( const SwRedlineData& rData, const SwPosition& rPos )
    : SwPaM( rPos ),
    pRedlineData( new SwRedlineData( rData )),
    pCntntSect( 0 )
{
    bDelLastPara = bIsLastParaDelete = false;
    bIsVisible = true;
}

SwRangeRedline::SwRangeRedline( const SwRangeRedline& rCpy )
    : SwPaM( *rCpy.GetMark(), *rCpy.GetPoint() ),
    pRedlineData( new SwRedlineData( *rCpy.pRedlineData )),
    pCntntSect( 0 )
{
    bDelLastPara = bIsLastParaDelete = false;
    bIsVisible = true;
    if( !rCpy.HasMark() )
        DeleteMark();
}

SwRangeRedline::~SwRangeRedline()
{
    if( pCntntSect )
    {
        // delete the ContentSection
        if( !GetDoc()->IsInDtor() )
            GetDoc()->getIDocumentContentOperations().DeleteSection( &pCntntSect->GetNode() );
        delete pCntntSect;
    }
    delete pRedlineData;
}

/// Do we have a valid Selection?
bool SwRangeRedline::HasValidRange() const
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
        return true;
    return false;
}

void SwRangeRedline::CallDisplayFunc( sal_uInt16 nLoop )
{
    switch( nsRedlineMode_t::REDLINE_SHOW_MASK & GetDoc()->getIDocumentRedlineAccess().GetRedlineMode() )
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

void SwRangeRedline::Show( sal_uInt16 nLoop )
{
    if( 1 <= nLoop )
    {
        SwDoc* pDoc = GetDoc();
        RedlineMode_t eOld = pDoc->getIDocumentRedlineAccess().GetRedlineMode();
        pDoc->getIDocumentRedlineAccess().SetRedlineMode_intern((RedlineMode_t)(eOld | nsRedlineMode_t::REDLINE_IGNORE));
        ::sw::UndoGuard const undoGuard(pDoc->GetIDocumentUndoRedo());

        switch( GetType() )
        {
        case nsRedlineType_t::REDLINE_INSERT:           // Content has been inserted
            bIsVisible = true;
            MoveFromSection();
            break;

        case nsRedlineType_t::REDLINE_DELETE:           // Content has been deleted
            bIsVisible = true;
            MoveFromSection();
            break;

        case nsRedlineType_t::REDLINE_FORMAT:           // Attributes have been applied
        case nsRedlineType_t::REDLINE_TABLE:            // Table structure has been modified
            InvalidateRange();
            break;
        default:
            break;
        }
        pDoc->getIDocumentRedlineAccess().SetRedlineMode_intern( eOld );
    }
}

void SwRangeRedline::Hide( sal_uInt16 nLoop )
{
    SwDoc* pDoc = GetDoc();
    RedlineMode_t eOld = pDoc->getIDocumentRedlineAccess().GetRedlineMode();
    pDoc->getIDocumentRedlineAccess().SetRedlineMode_intern((RedlineMode_t)(eOld | nsRedlineMode_t::REDLINE_IGNORE));
    ::sw::UndoGuard const undoGuard(pDoc->GetIDocumentUndoRedo());

    switch( GetType() )
    {
    case nsRedlineType_t::REDLINE_INSERT:           // Content has been inserted
        bIsVisible = true;
        if( 1 <= nLoop )
            MoveFromSection();
        break;

    case nsRedlineType_t::REDLINE_DELETE:           // Content has been deleted
        bIsVisible = false;
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
    pDoc->getIDocumentRedlineAccess().SetRedlineMode_intern( eOld );
}

void SwRangeRedline::ShowOriginal( sal_uInt16 nLoop )
{
    SwDoc* pDoc = GetDoc();
    RedlineMode_t eOld = pDoc->getIDocumentRedlineAccess().GetRedlineMode();
    SwRedlineData* pCur;

    pDoc->getIDocumentRedlineAccess().SetRedlineMode_intern((RedlineMode_t)(eOld | nsRedlineMode_t::REDLINE_IGNORE));
    ::sw::UndoGuard const undoGuard(pDoc->GetIDocumentUndoRedo());

    // Determine the Type, it's the first on Stack
    for( pCur = pRedlineData; pCur->pNext; )
        pCur = pCur->pNext;

    switch( pCur->eType )
    {
    case nsRedlineType_t::REDLINE_INSERT:           // Content has been inserted
        bIsVisible = false;
        switch( nLoop )
        {
        case 0: MoveToSection();    break;
        case 1: CopyToSection();    break;
        case 2: DelCopyOfSection(); break;
        }
        break;

    case nsRedlineType_t::REDLINE_DELETE:           // Inhalt wurde eingefuegt
        bIsVisible = true;
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
    pDoc->getIDocumentRedlineAccess().SetRedlineMode_intern( eOld );
}

void SwRangeRedline::InvalidateRange()       // trigger the Layout
{
    sal_uLong nSttNd = GetMark()->nNode.GetIndex(),
            nEndNd = GetPoint()->nNode.GetIndex();
    sal_Int32 nSttCnt = GetMark()->nContent.GetIndex();
    sal_Int32 nEndCnt = GetPoint()->nContent.GetIndex();

    if( nSttNd > nEndNd || ( nSttNd == nEndNd && nSttCnt > nEndCnt ))
    {
        sal_uLong nTmp = nSttNd; nSttNd = nEndNd; nEndNd = nTmp;
        sal_Int32 nTmp2 = nSttCnt; nSttCnt = nEndCnt; nEndCnt = nTmp2;
    }

    SwNodes& rNds = GetDoc()->GetNodes();
    for (sal_uLong n(nSttNd); n <= nEndNd; ++n)
    {
        SwNode* pNode = rNds[n];

        if (pNode && pNode->IsTxtNode())
        {
            SwTxtNode* pNd = pNode->GetTxtNode();
            SwUpdateAttr aHt(
                n == nSttNd ? nSttCnt : 0,
                n == nEndNd ? nEndCnt : pNd->GetTxt().getLength(),
                RES_FMT_CHG);

            pNd->ModifyNotification(&aHt, &aHt);
        }
    }
}

/** Calculates the start and end position of the intersection rTmp and
    text node nNdIdx */
void SwRangeRedline::CalcStartEnd( sal_uLong nNdIdx, sal_Int32& rStart, sal_Int32& rEnd ) const
{
    const SwPosition *pRStt = Start(), *pREnd = End();
    if( pRStt->nNode < nNdIdx )
    {
        if( pREnd->nNode > nNdIdx )
        {
            rStart = 0;             // Paragraph is completely enclosed
            rEnd = COMPLETE_STRING;
        }
        else
        {
            OSL_ENSURE( pREnd->nNode == nNdIdx,
                "SwRedlineItr::Seek: GetRedlinePos Error" );
            rStart = 0;             // Paragraph is overlapped in the beginning
            rEnd = pREnd->nContent.GetIndex();
        }
    }
    else if( pRStt->nNode == nNdIdx )
    {
        rStart = pRStt->nContent.GetIndex();
        if( pREnd->nNode == nNdIdx )
            rEnd = pREnd->nContent.GetIndex(); // Within the Paragraph
        else
            rEnd = COMPLETE_STRING;      // Paragraph is overlapped in the end
    }
    else
    {
        rStart = COMPLETE_STRING;
        rEnd = COMPLETE_STRING;
    }
}

void SwRangeRedline::MoveToSection()
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
            const SwRedlineTbl& rTbl = pDoc->getIDocumentRedlineAccess().GetRedlineTbl();
            for( sal_uInt16 n = 0; n < rTbl.size(); ++n )
            {
                SwRangeRedline* pRedl = rTbl[ n ];
                if( pRedl->GetBound(true) == *pStt )
                    pRedl->GetBound(true) = *pEnd;
                if( pRedl->GetBound(false) == *pStt )
                    pRedl->GetBound(false) = *pEnd;
            }
        }

        SwStartNode* pSttNd;
        SwNodes& rNds = pDoc->GetNodes();
        if( pCSttNd || pCEndNd )
        {
            SwTxtFmtColl* pColl = (pCSttNd && pCSttNd->IsTxtNode() )
                                    ? pCSttNd->GetTxtNode()->GetTxtColl()
                                    : (pCEndNd && pCEndNd->IsTxtNode() )
                                        ? pCEndNd->GetTxtNode()->GetTxtColl()
                                        : pDoc->getIDocumentStylePoolAccess().GetTxtCollFromPool(RES_POOLCOLL_STANDARD);

            pSttNd = rNds.MakeTextSection( SwNodeIndex( rNds.GetEndOfRedlines() ),
                                            SwNormalStartNode, pColl );
            SwTxtNode* pTxtNd = rNds[ pSttNd->GetIndex() + 1 ]->GetTxtNode();

            SwNodeIndex aNdIdx( *pTxtNd );
            SwPosition aPos( aNdIdx, SwIndex( pTxtNd ));
            if( pCSttNd && pCEndNd )
                pDoc->getIDocumentContentOperations().MoveAndJoin( aPam, aPos, IDocumentContentOperations::DOC_MOVEDEFAULT );
            else
            {
                if( pCSttNd && !pCEndNd )
                    bDelLastPara = true;
                pDoc->getIDocumentContentOperations().MoveRange( aPam, aPos,
                    IDocumentContentOperations::DOC_MOVEDEFAULT );
            }
        }
        else
        {
            pSttNd = rNds.MakeEmptySection( SwNodeIndex( rNds.GetEndOfRedlines() ),
                                            SwNormalStartNode );

            SwPosition aPos( *pSttNd->EndOfSectionNode() );
            pDoc->getIDocumentContentOperations().MoveRange( aPam, aPos,
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

void SwRangeRedline::CopyToSection()
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

        bool bSaveCopyFlag = pDoc->IsCopyIsMove(),
             bSaveRdlMoveFlg = pDoc->getIDocumentRedlineAccess().IsRedlineMove();
        pDoc->SetCopyIsMove( true );

        // The IsRedlineMove() flag causes the behaviour of the
        // SwDoc::_CopyFlyInFly method to change, which will eventually be
        // called by the pDoc->Copy line below (through SwDoc::_Copy,
        // SwDoc::CopyWithFlyInFly). This rather obscure bugfix
        // apparently never really worked.
        pDoc->getIDocumentRedlineAccess().SetRedlineMove( pStt->nContent == 0 );

        if( pCSttNd )
        {
            SwTxtFmtColl* pColl = (pCSttNd && pCSttNd->IsTxtNode() )
                                    ? pCSttNd->GetTxtNode()->GetTxtColl()
                                    : pDoc->getIDocumentStylePoolAccess().GetTxtCollFromPool(RES_POOLCOLL_STANDARD);

            pSttNd = rNds.MakeTextSection( SwNodeIndex( rNds.GetEndOfRedlines() ),
                                            SwNormalStartNode, pColl );

            SwNodeIndex aNdIdx( *pSttNd, 1 );
            SwTxtNode* pTxtNd = aNdIdx.GetNode().GetTxtNode();
            SwPosition aPos( aNdIdx, SwIndex( pTxtNd ));
            pDoc->getIDocumentContentOperations().CopyRange( *this, aPos, false );

            // Take over the style from the EndNode if needed
            // We don't want this in Doc::Copy
            if( pCEndNd && pCEndNd != pCSttNd )
            {
                SwCntntNode* pDestNd = aPos.nNode.GetNode().GetCntntNode();
                if( pDestNd )
                {
                    if( pDestNd->IsTxtNode() && pCEndNd->IsTxtNode() )
                        pCEndNd->GetTxtNode()->CopyCollFmt(*pDestNd->GetTxtNode());
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
                pDoc->getIDocumentContentOperations().CopyRange( *this, aPos, false );
            }
            else
            {
                SwNodeIndex aInsPos( *pSttNd->EndOfSectionNode() );
                SwNodeRange aRg( pStt->nNode, 0, pEnd->nNode, 1 );
                pDoc->GetDocumentContentOperationsManager().CopyWithFlyInFly( aRg, 0, aInsPos );
            }
        }
        pCntntSect = new SwNodeIndex( *pSttNd );

        pDoc->SetCopyIsMove( bSaveCopyFlag );
        pDoc->getIDocumentRedlineAccess().SetRedlineMove( bSaveRdlMoveFlg );
    }
}

void SwRangeRedline::DelCopyOfSection()
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
            const SwRedlineTbl& rTbl = pDoc->getIDocumentRedlineAccess().GetRedlineTbl();
            for( sal_uInt16 n = 0; n < rTbl.size(); ++n )
            {
                SwRangeRedline* pRedl = rTbl[ n ];
                if( pRedl->GetBound(true) == *pStt )
                    pRedl->GetBound(true) = *pEnd;
                if( pRedl->GetBound(false) == *pStt )
                    pRedl->GetBound(false) = *pEnd;
            }
        }

        if( pCSttNd && pCEndNd )
        {
            // #i100466# - force a <join next> on <delete and join> operation
            pDoc->getIDocumentContentOperations().DeleteAndJoin( aPam, true );
        }
        else if( pCSttNd || pCEndNd )
        {
            if( pCSttNd && !pCEndNd )
                bDelLastPara = true;
            pDoc->getIDocumentContentOperations().DeleteRange( aPam );

            if( bDelLastPara )
            {
                // To prevent dangling references to the paragraph to
                // be deleted, redline that point into this paragraph should be
                // moved to the new end position. Since redlines in the redline
                // table are sorted and the pEnd position is an endnode (see
                // bDelLastPara condition above), only redlines before the
                // current ones can be affected.
                const SwRedlineTbl& rTbl = pDoc->getIDocumentRedlineAccess().GetRedlineTbl();
                sal_uInt16 n = rTbl.GetPos( this );
                OSL_ENSURE( n != USHRT_MAX, "How strange. We don't exist!" );
                for( bool bBreak = false; !bBreak && n > 0; )
                {
                    --n;
                    bBreak = true;
                    if( rTbl[ n ]->GetBound(true) == *aPam.GetPoint() )
                    {
                        rTbl[ n ]->GetBound(true) = *pEnd;
                        bBreak = false;
                    }
                    if( rTbl[ n ]->GetBound(false) == *aPam.GetPoint() )
                    {
                        rTbl[ n ]->GetBound(false) = *pEnd;
                        bBreak = false;
                    }
                }

                SwPosition aEnd( *pEnd );
                *GetPoint() = *pEnd;
                *GetMark() = *pEnd;
                DeleteMark();

                aPam.GetBound( true ).nContent.Assign( 0, 0 );
                aPam.GetBound( false ).nContent.Assign( 0, 0 );
                aPam.DeleteMark();
                pDoc->getIDocumentContentOperations().DelFullPara( aPam );
            }
        }
        else
        {
            pDoc->getIDocumentContentOperations().DeleteRange( aPam );
        }

        if( pStt == GetPoint() )
            Exchange();

        DeleteMark();
    }
}

void SwRangeRedline::MoveFromSection()
{
    if( pCntntSect )
    {
        SwDoc* pDoc = GetDoc();
        const SwRedlineTbl& rTbl = pDoc->getIDocumentRedlineAccess().GetRedlineTbl();
        std::vector<SwPosition*> aBeforeArr, aBehindArr;
        typedef std::map<sal_uInt16, SwRangeRedline*> IndexAndRange;
        IndexAndRange aIndexAndRangeMap;
        sal_uInt16 nMyPos = rTbl.GetPos( this );
        OSL_ENSURE( this, "this is not in the array?" );
        bool bBreak = false;
        sal_uInt16 n;

        for( n = nMyPos+1; !bBreak && n < rTbl.size(); ++n )
        {
            bBreak = true;
            if( rTbl[ n ]->GetBound(true) == *GetPoint() )
            {
                SwRangeRedline* pRedl = rTbl[n];
                aBehindArr.push_back(&pRedl->GetBound(true));
                aIndexAndRangeMap.insert(std::make_pair(n, pRedl));
                bBreak = false;
            }
            if( rTbl[ n ]->GetBound(false) == *GetPoint() )
            {
                SwRangeRedline* pRedl = rTbl[n];
                aBehindArr.push_back(&pRedl->GetBound(false));
                aIndexAndRangeMap.insert(std::make_pair(n, pRedl));
                bBreak = false;
            }
        }
        for( bBreak = false, n = nMyPos; !bBreak && n ; )
        {
            --n;
            bBreak = true;
            if( rTbl[ n ]->GetBound(true) == *GetPoint() )
            {
                SwRangeRedline* pRedl = rTbl[n];
                aBeforeArr.push_back(&pRedl->GetBound(true));
                aIndexAndRangeMap.insert(std::make_pair(n, pRedl));
                bBreak = false;
            }
            if( rTbl[ n ]->GetBound(false) == *GetPoint() )
            {
                SwRangeRedline* pRedl = rTbl[n];
                aBeforeArr.push_back(&pRedl->GetBound(false));
                aIndexAndRangeMap.insert(std::make_pair(n, pRedl));
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
            const sal_Int32 nPos = GetPoint()->nContent.GetIndex();

            SwPosition aPos( *GetPoint() );
            if( bDelLastPara && *aPam.GetPoint() == *aPam.GetMark() )
            {
                aPos.nNode--;

                pDoc->getIDocumentContentOperations().AppendTxtNode( aPos );
            }
            else
            {
                pDoc->getIDocumentContentOperations().MoveRange( aPam, aPos,
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
                bDelLastPara = false;
            }
            else if( pColl )
                pCNd = GetCntntNode();

            if( pColl && pCNd )
                pCNd->ChgFmtColl( pColl );
        }

        // #i95771#
        // Under certain conditions the previous <SwDoc::Move(..)> has already
        // removed the change tracking section of this <SwRangeRedline> instance from
        // the change tracking nodes area.
        // Thus, check if <pCntntSect> still points to the change tracking section
        // by comparing it with the "indexed" <SwNode> instance copied before
        // perform the intrinsic move.
        // Note: Such condition is e.g. a "delete" change tracking only containing a table.
        if ( &pCntntSect->GetNode() == pKeptCntntSectNode )
        {
            pDoc->getIDocumentContentOperations().DeleteSection( &pCntntSect->GetNode() );
        }
        delete pCntntSect, pCntntSect = 0;

        // adjustment of redline table positions must take start and
        // end into account, not point and mark.
        for( n = 0; n < aBeforeArr.size(); ++n )
            *aBeforeArr[ n ] = *Start();
        for( n = 0; n < aBehindArr.size(); ++n )
            *aBehindArr[ n ] = *End();
        SwRedlineTbl& rResortTbl = const_cast<SwRedlineTbl&>(rTbl);
        for (auto& a : aIndexAndRangeMap)
        {
            // re-insert
            rResortTbl.Remove(a.first);
            rResortTbl.Insert(a.second);
        }
    }
    else
        InvalidateRange();
}

// for Undo
void SwRangeRedline::SetContentIdx( const SwNodeIndex* pIdx )
{
    if( pIdx && !pCntntSect )
    {
        pCntntSect = new SwNodeIndex( *pIdx );
        bIsVisible = false;
    }
    else if( !pIdx && pCntntSect )
    {
        delete pCntntSect, pCntntSect = 0;
        bIsVisible = false;
    }
    else
    {
        OSL_FAIL("SwRangeRedline::SetContentIdx: invalid state");
    }
}

bool SwRangeRedline::CanCombine( const SwRangeRedline& rRedl ) const
{
    return  IsVisible() && rRedl.IsVisible() &&
            pRedlineData->CanCombine( *rRedl.pRedlineData );
}

void SwRangeRedline::PushData( const SwRangeRedline& rRedl, bool bOwnAsNext )
{
    SwRedlineData* pNew = new SwRedlineData( *rRedl.pRedlineData, false );
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

bool SwRangeRedline::PopData()
{
    if( !pRedlineData->pNext )
        return false;
    SwRedlineData* pCur = pRedlineData;
    pRedlineData = pCur->pNext;
    pCur->pNext = 0;
    delete pCur;
    return true;
}

sal_uInt16 SwRangeRedline::GetStackCount() const
{
    sal_uInt16 nRet = 1;
    for( SwRedlineData* pCur = pRedlineData; pCur->pNext; ++nRet )
        pCur = pCur->pNext;
    return nRet;
}

sal_uInt16 SwRangeRedline::GetAuthor( sal_uInt16 nPos ) const
{
    return GetRedlineData(nPos).nAuthor;
}

OUString SwRangeRedline::GetAuthorString( sal_uInt16 nPos ) const
{
    return SW_MOD()->GetRedlineAuthor(GetRedlineData(nPos).nAuthor);
}

const DateTime& SwRangeRedline::GetTimeStamp( sal_uInt16 nPos ) const
{
    return GetRedlineData(nPos).aStamp;
}

RedlineType_t SwRangeRedline::GetRealType( sal_uInt16 nPos ) const
{
    return GetRedlineData(nPos).eType;
}

const OUString& SwRangeRedline::GetComment( sal_uInt16 nPos ) const
{
    return GetRedlineData(nPos).sComment;
}

bool SwRangeRedline::operator==( const SwRangeRedline& rCmp ) const
{
    return this == &rCmp;
}

bool SwRangeRedline::operator<( const SwRangeRedline& rCmp ) const
{
    bool nResult = false;

    if (*Start() < *rCmp.Start())
        nResult = true;
    else if (*Start() == *rCmp.Start())
        if (*End() < *rCmp.End())
            nResult = true;

    return nResult;
}

const SwRedlineData & SwRangeRedline::GetRedlineData(sal_uInt16 nPos) const
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

OUString SwRangeRedline::GetDescr(sal_uInt16 nPos)
{
    OUString aResult;

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
    OUString aTmpStr;
    aTmpStr += SW_RES(STR_START_QUOTE);
    aTmpStr += ShortenString(pPaM->GetTxt(), nUndoStringLength,
                             OUString(SW_RES(STR_LDOTS)));
    aTmpStr += SW_RES(STR_END_QUOTE);

    SwRewriter aRewriter;
    aRewriter.AddRule(UndoArg1, aTmpStr);

    aResult = aRewriter.Apply(aResult);

    if (bDeletePaM)
        delete pPaM;

    return aResult;
}

bool SwExtraRedlineTbl::Insert( SwExtraRedline* p )
{
    m_aExtraRedlines.push_back( p );
    //p->CallDisplayFunc();
    return true;
}

void SwExtraRedlineTbl::DeleteAndDestroy( sal_uInt16 nPos, sal_uInt16 nLen )
{
    /*
    SwDoc* pDoc = 0;
    if( !nP && nL && nL == size() )
        pDoc = front()->GetDoc();
    */

    for( std::vector<SwExtraRedline*>::iterator it = m_aExtraRedlines.begin() + nPos; it != m_aExtraRedlines.begin() + nPos + nLen; ++it )
        delete *it;

    m_aExtraRedlines.erase( m_aExtraRedlines.begin() + nPos, m_aExtraRedlines.begin() + nPos + nLen );

    /*
    SwViewShell* pSh;
    if( pDoc && !pDoc->IsInDtor() &&
        0 != ( pSh = pDoc->getIDocumentLayoutAccess().GetCurrentViewShell() ) )
        pSh->InvalidateWindows( SwRect( 0, 0, LONG_MAX, LONG_MAX ) );
    */
}

void SwExtraRedlineTbl::DeleteAndDestroyAll()
{
    DeleteAndDestroy(0, m_aExtraRedlines.size());
}

SwExtraRedline::~SwExtraRedline()
{
}

SwTableRowRedline::SwTableRowRedline(const SwRedlineData& rData, const SwTableLine& rTableLine)
    : m_aRedlineData(rData)
    , m_rTableLine(rTableLine)
{
}

SwTableRowRedline::~SwTableRowRedline()
{
}

SwTableCellRedline::SwTableCellRedline(const SwRedlineData& rData, const SwTableBox& rTableBox)
    : m_aRedlineData(rData)
    , m_rTableBox(rTableBox)
{
}

SwTableCellRedline::~SwTableCellRedline()
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

