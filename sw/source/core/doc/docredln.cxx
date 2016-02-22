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

#include <libxml/xmlwriter.h>
#include <tools/datetimeutils.hxx>
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
        static SwRedlineTable::size_type nWatch = 0;
        const SwRedlineTable& rTable = pDoc->getIDocumentRedlineAccess().GetRedlineTable();
        for( SwRedlineTable::size_type n = 0; n < rTable.size(); ++n )
        {
            SwRedlineTable::size_type nDummy = 0;
            const SwRangeRedline* pCurrent = rTable[ n ];
            const SwRangeRedline* pNext = n+1 < rTable.size() ? rTable[ n+1 ] : nullptr;
            if( pCurrent == pNext )
                ++nDummy;
            if( n == nWatch )
                ++nDummy; // Possible debugger breakpoint
        }
    }

#endif


SwExtraRedlineTable::~SwExtraRedlineTable()
{
    DeleteAndDestroyAll();
}

void SwExtraRedlineTable::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    xmlTextWriterStartElement(pWriter, BAD_CAST("swExtraRedlineTable"));
    xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("ptr"), "%p", this);

    for (sal_uInt16 nCurExtraRedlinePos = 0; nCurExtraRedlinePos < GetSize(); ++nCurExtraRedlinePos)
    {
        const SwExtraRedline* pExtraRedline = GetRedline(nCurExtraRedlinePos);
        xmlTextWriterStartElement(pWriter, BAD_CAST("swExtraRedline"));
        xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("ptr"), "%p", this);
        xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("symbol"), "%s", BAD_CAST(typeid(*pExtraRedline).name()));
        xmlTextWriterEndElement(pWriter);
    }
    xmlTextWriterEndElement(pWriter);
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

bool SwExtraRedlineTable::DeleteAllTableRedlines( SwDoc* pDoc, const SwTable& rTable, bool bSaveInUndo, sal_uInt16 nRedlineTypeToDelete )
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

    for (sal_uInt16 nCurRedlinePos = 0; nCurRedlinePos < GetSize(); )
    {
        SwExtraRedline* pExtraRedline = GetRedline(nCurRedlinePos);
        const SwTableCellRedline* pTableCellRedline = dynamic_cast<const SwTableCellRedline*>(pExtraRedline);
        if (pTableCellRedline)
        {
            const SwTableBox *pRedTabBox = &pTableCellRedline->GetTableBox();
            const SwTable& rRedTable = pRedTabBox->GetSttNd()->FindTableNode()->GetTable();
            if ( &rRedTable == &rTable )
            {
                // Redline for this table
                const SwRedlineData& aRedlineData = pTableCellRedline->GetRedlineData();
                const RedlineType_t nRedlineType = aRedlineData.GetType();

                // Check if this redline object type should be deleted
                if (USHRT_MAX == nRedlineTypeToDelete || nRedlineTypeToDelete == nRedlineType)
                {

                    DeleteAndDestroy( nCurRedlinePos );
                    bChg = true;
                    continue; // don't increment position after delete
                }
            }
        }
        else
        {
            const SwTableRowRedline* pTableRowRedline = dynamic_cast<const SwTableRowRedline*>(pExtraRedline);
            if (pTableRowRedline)
            {
                const SwTableLine *pRedTabLine = &pTableRowRedline->GetTableLine();
                const SwTableBoxes &rRedTabBoxes = pRedTabLine->GetTabBoxes();
                const SwTable& rRedTable = rRedTabBoxes[0]->GetSttNd()->FindTableNode()->GetTable();
                if ( &rRedTable == &rTable )
                {
                    // Redline for this table
                    const SwRedlineData& aRedlineData = pTableRowRedline->GetRedlineData();
                    const RedlineType_t nRedlineType = aRedlineData.GetType();

                    // Check if this redline object type should be deleted
                    if (USHRT_MAX == nRedlineTypeToDelete || nRedlineTypeToDelete == nRedlineType)

                    {
                        DeleteAndDestroy( nCurRedlinePos );
                        bChg = true;
                        continue; // don't increment position after delete
                    }
                }
            }
        }
        ++nCurRedlinePos;
    }

    if( bChg )
        pDoc->getIDocumentState().SetModified();

    return bChg;
}

bool SwExtraRedlineTable::DeleteTableRowRedline( SwDoc* pDoc, const SwTableLine& rTableLine, bool bSaveInUndo, sal_uInt16 nRedlineTypeToDelete )
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
        const SwTableLine *pRedTabLine = pTableRowRedline ? &pTableRowRedline->GetTableLine() : nullptr;
        if ( pRedTabLine == &rTableLine )
        {
            // Redline for this table row
            const SwRedlineData& aRedlineData = pTableRowRedline->GetRedlineData();
            const RedlineType_t nRedlineType = aRedlineData.GetType();

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

bool SwExtraRedlineTable::DeleteTableCellRedline( SwDoc* pDoc, const SwTableBox& rTableBox, bool bSaveInUndo, sal_uInt16 nRedlineTypeToDelete )
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
        const SwTableBox *pRedTabBox = pTableCellRedline ? &pTableCellRedline->GetTableBox() : nullptr;
        if ( pRedTabBox == &rTableBox )
        {
            // Redline for this table cell
            const SwRedlineData& aRedlineData = pTableCellRedline->GetRedlineData();
            const RedlineType_t nRedlineType = aRedlineData.GetType();

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

bool SwRedlineTable::Insert( SwRangeRedline* p, bool bIns )
{
    if( p->HasValidRange() )
    {
        std::pair<vector_type::const_iterator, bool> rv = maVector.insert( p );
        size_t nP = rv.first - begin();
        p->CallDisplayFunc(0, nP);
        return rv.second;
    }
    if( bIns )
        return InsertWithValidRanges( p );

    OSL_ENSURE( false, "Redline: wrong range" );
    return false;
}

bool SwRedlineTable::Insert( SwRangeRedline* p, sal_uInt16& rP, bool bIns )
{
    if( p->HasValidRange() )
    {
        std::pair<vector_type::const_iterator, bool> rv = maVector.insert( p );
        rP = rv.first - begin();
        p->CallDisplayFunc(0, rP);
        return rv.second;
    }
    if( bIns )
        return InsertWithValidRanges( p, &rP );

    OSL_ENSURE( false, "Redline: wrong range" );
    return false;
}

bool SwRedlineTable::InsertWithValidRanges( SwRangeRedline* p, sal_uInt16* pInsPos )
{
    // Create valid "sub-ranges" from the Selection
    bool bAnyIns = false;
    SwPosition* pStt = p->Start(),
              * pEnd = pStt == p->GetPoint() ? p->GetMark() : p->GetPoint();
    SwPosition aNewStt( *pStt );
    SwNodes& rNds = aNewStt.nNode.GetNodes();
    SwContentNode* pC;

    if( !aNewStt.nNode.GetNode().IsContentNode() )
    {
        pC = rNds.GoNext( &aNewStt.nNode );
        if( pC )
            aNewStt.nContent.Assign( pC, 0 );
        else
            aNewStt.nNode = rNds.GetEndOfContent();
    }

    SwRangeRedline* pNew = nullptr;
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
                pC = nullptr;
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
                        else if( rCurNd.IsContentNode() )
                            pC = rCurNd.GetContentNode();
                        ++aNewStt.nNode;
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
                pNew->CallDisplayFunc(0, nInsPos);
                bAnyIns = true;
                pNew = nullptr;
                if( pInsPos && *pInsPos < nInsPos )
                    *pInsPos = nInsPos;
            }

            if( aNewStt >= *pEnd ||
                nullptr == (pC = rNds.GoNext( &aNewStt.nNode )) )
                break;

            aNewStt.nContent.Assign( pC, 0 );

        } while( aNewStt < *pEnd );

    delete pNew;
    delete p;
    p = nullptr;
    return bAnyIns;
}

bool CompareSwRedlineTable::operator()(SwRangeRedline* const &lhs, SwRangeRedline* const &rhs) const
{
    return *lhs < *rhs;
}

SwRedlineTable::~SwRedlineTable()
{
   maVector.DeleteAndDestroyAll();
}

sal_uInt16 SwRedlineTable::GetPos(const SwRangeRedline* p) const
{
    vector_type::const_iterator it = maVector.find(const_cast<SwRangeRedline* const>(p));
    if( it == maVector.end() )
        return USHRT_MAX;
    return it - maVector.begin();
}

bool SwRedlineTable::Remove( const SwRangeRedline* p )
{
    const sal_uInt16 nPos = GetPos(p);
    if (nPos == USHRT_MAX)
        return false;
    Remove(nPos);
    return true;
}

void SwRedlineTable::Remove( sal_uInt16 nP )
{
    SwDoc* pDoc = nullptr;
    if( !nP && 1 == size() )
        pDoc = maVector.front()->GetDoc();

    maVector.erase( maVector.begin() + nP );

    SwViewShell* pSh;
    if( pDoc && !pDoc->IsInDtor() &&
        nullptr != ( pSh = pDoc->getIDocumentLayoutAccess().GetCurrentViewShell()) )
        pSh->InvalidateWindows( SwRect( 0, 0, SAL_MAX_INT32, SAL_MAX_INT32 ) );
}

void SwRedlineTable::DeleteAndDestroyAll()
{
    DeleteAndDestroy(0, size());
}

void SwRedlineTable::DeleteAndDestroy( sal_uInt16 nP, sal_uInt16 nL )
{
    SwDoc* pDoc = nullptr;
    if( !nP && nL && nL == size() )
        pDoc = maVector.front()->GetDoc();

    for( vector_type::const_iterator it = maVector.begin() + nP; it != maVector.begin() + nP + nL; ++it )
        delete *it;
    maVector.erase( maVector.begin() + nP, maVector.begin() + nP + nL );

    SwViewShell* pSh;
    if( pDoc && !pDoc->IsInDtor() &&
        nullptr != ( pSh = pDoc->getIDocumentLayoutAccess().GetCurrentViewShell() ) )
        pSh->InvalidateWindows( SwRect( 0, 0, SAL_MAX_INT32, SAL_MAX_INT32 ) );
}

sal_uInt16 SwRedlineTable::FindNextOfSeqNo( sal_uInt16 nSttPos ) const
{
    return static_cast<size_t>(nSttPos) + 1 < size()
                ? FindNextSeqNo( operator[]( nSttPos )->GetSeqNo(), nSttPos+1 )
                : USHRT_MAX;
}

sal_uInt16 SwRedlineTable::FindPrevOfSeqNo( sal_uInt16 nSttPos ) const
{
    return nSttPos ? FindPrevSeqNo( operator[]( nSttPos )->GetSeqNo(), nSttPos-1 )
                   : USHRT_MAX;
}

/// Find the next or preceding Redline with the same seq.no.
/// We can limit the search using look ahead (0 searches the whole array).
sal_uInt16 SwRedlineTable::FindNextSeqNo( sal_uInt16 nSeqNo, sal_uInt16 nSttPos,
                                    sal_uInt16 nLookahead ) const
{
    sal_uInt16 nRet = USHRT_MAX;
    if( nSeqNo && nSttPos < size() )
    {
        size_t nEnd = size();
        if( nLookahead )
        {
            const size_t nTmp = static_cast<size_t>(nSttPos)+ static_cast<size_t>(nLookahead);
            if (nTmp < nEnd)
            {
                nEnd = nTmp;
            }
        }

        for( ; nSttPos < nEnd; ++nSttPos )
            if( nSeqNo == operator[]( nSttPos )->GetSeqNo() )
            {
                nRet = nSttPos;
                break;
            }
    }
    return nRet;
}

sal_uInt16 SwRedlineTable::FindPrevSeqNo( sal_uInt16 nSeqNo, sal_uInt16 nSttPos,
                                    sal_uInt16 nLookahead ) const
{
    sal_uInt16 nRet = USHRT_MAX;
    if( nSeqNo && nSttPos < size() )
    {
        size_t nEnd = 0;
        if( nLookahead && nSttPos > nLookahead )
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

const SwRangeRedline* SwRedlineTable::FindAtPosition( const SwPosition& rSttPos,
                                        sal_uInt16& rPos,
                                        bool bNext ) const
{
    const SwRangeRedline* pFnd = nullptr;
    for( ; rPos < maVector.size() ; ++rPos )
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

void SwRedlineTable::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    xmlTextWriterStartElement(pWriter, BAD_CAST("swRedlineTable"));
    xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("ptr"), "%p", this);

    for (SwRedlineTable::size_type nCurRedlinePos = 0; nCurRedlinePos < size(); ++nCurRedlinePos)
        operator[](nCurRedlinePos)->dumpAsXml(pWriter);

    xmlTextWriterEndElement(pWriter);
}

SwRedlineExtraData::~SwRedlineExtraData()
{
}

void SwRedlineExtraData::Reject( SwPaM& ) const
{
}

bool SwRedlineExtraData::operator == ( const SwRedlineExtraData& ) const
{
    return false;
}

SwRedlineExtraData_FormatColl::SwRedlineExtraData_FormatColl( const OUString& rColl,
                                                sal_uInt16 nPoolFormatId,
                                                const SfxItemSet* pItemSet )
    : sFormatNm(rColl), pSet(nullptr), nPoolId(nPoolFormatId)
{
    if( pItemSet && pItemSet->Count() )
        pSet = new SfxItemSet( *pItemSet );
}

SwRedlineExtraData_FormatColl::~SwRedlineExtraData_FormatColl()
{
    delete pSet;
}

SwRedlineExtraData* SwRedlineExtraData_FormatColl::CreateNew() const
{
    return new SwRedlineExtraData_FormatColl( sFormatNm, nPoolId, pSet );
}

void SwRedlineExtraData_FormatColl::Reject( SwPaM& rPam ) const
{
    SwDoc* pDoc = rPam.GetDoc();

    // What about Undo? Is it turned off?
    SwTextFormatColl* pColl = USHRT_MAX == nPoolId
                            ? pDoc->FindTextFormatCollByName( sFormatNm )
                            : pDoc->getIDocumentStylePoolAccess().GetTextCollFromPool( nPoolId );
    if( pColl )
        pDoc->SetTextFormatColl( rPam, pColl, false );

    if( pSet )
    {
        rPam.SetMark();
        SwPosition& rMark = *rPam.GetMark();
        SwTextNode* pTNd = rMark.nNode.GetNode().GetTextNode();
        if( pTNd )
        {
            rMark.nContent.Assign(pTNd, pTNd->GetText().getLength());

            if( pTNd->HasSwAttrSet() )
            {
                // Only set those that are not there anymore. Others
                // could have changed, but we don't touch these.
                SfxItemSet aTmp( *pSet );
                aTmp.Differentiate( *pTNd->GetpSwAttrSet() );
                pDoc->getIDocumentContentOperations().InsertItemSet( rPam, aTmp );
            }
            else
            {
                pDoc->getIDocumentContentOperations().InsertItemSet( rPam, *pSet );
            }
        }
        rPam.DeleteMark();
    }
}

bool SwRedlineExtraData_FormatColl::operator == ( const SwRedlineExtraData& r) const
{
    const SwRedlineExtraData_FormatColl& rCmp = static_cast<const SwRedlineExtraData_FormatColl&>(r);
    return sFormatNm == rCmp.sFormatNm && nPoolId == rCmp.nPoolId &&
            ( ( !pSet && !rCmp.pSet ) ||
               ( pSet && rCmp.pSet && *pSet == *rCmp.pSet ) );
}

void SwRedlineExtraData_FormatColl::SetItemSet( const SfxItemSet& rSet )
{
    delete pSet;
    if( rSet.Count() )
        pSet = new SfxItemSet( rSet );
    else
        pSet = nullptr;
}

SwRedlineExtraData_Format::SwRedlineExtraData_Format( const SfxItemSet& rSet )
{
    SfxItemIter aIter( rSet );
    const SfxPoolItem* pItem = aIter.FirstItem();
    while(pItem)
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
            SetAttrMode::DONTEXPAND );
    }

    pDoc->getIDocumentRedlineAccess().SetRedlineMode_intern( eOld );
}

bool SwRedlineExtraData_Format::operator == ( const SwRedlineExtraData& rCmp ) const
{
    const size_t nEnd = aWhichIds.size();
    if( nEnd != static_cast<const SwRedlineExtraData_Format&>(rCmp).aWhichIds.size() )
        return false;

    for( size_t n = 0; n < nEnd; ++n )
    {
        if( static_cast<const SwRedlineExtraData_Format&>(rCmp).aWhichIds[n] != aWhichIds[n])
        {
            return false;
        }
    }
    return true;
}

SwRedlineExtraData_FormattingChanges::SwRedlineExtraData_FormattingChanges( const SfxItemSet* pItemSet )
    : pSet(nullptr)
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
        pSet = nullptr;
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

void SwRedlineExtraData_FormattingChanges::Reject(SwPaM&) const
{
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
    : pNext( nullptr ), pExtraData( nullptr ),
    aStamp( DateTime::SYSTEM ),
    eType( eT ), nAuthor( nAut ), nSeqNo( 0 )
{
    aStamp.SetSec( 0 );
    aStamp.SetNanoSec( 0 );
}

SwRedlineData::SwRedlineData(
    const SwRedlineData& rCpy,
    bool bCpyNext )
    : pNext( ( bCpyNext && rCpy.pNext ) ? new SwRedlineData( *rCpy.pNext ) : nullptr )
    , pExtraData( rCpy.pExtraData ? rCpy.pExtraData->CreateNew() : nullptr )
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
        pExtraData = nullptr;
}

OUString SwRedlineData::GetDescr() const
{
    return SW_RES(STR_REDLINE_INSERT + GetType());
}

SwRangeRedline::SwRangeRedline(RedlineType_t eTyp, const SwPaM& rPam )
    : SwPaM( *rPam.GetMark(), *rPam.GetPoint() ),
    pRedlineData( new SwRedlineData( eTyp, GetDoc()->getIDocumentRedlineAccess().GetRedlineAuthor() ) ),
    pContentSect( nullptr )
{
    bDelLastPara = bIsLastParaDelete = false;
    bIsVisible = true;
    if( !rPam.HasMark() )
        DeleteMark();
}

SwRangeRedline::SwRangeRedline( const SwRedlineData& rData, const SwPaM& rPam )
    : SwPaM( *rPam.GetMark(), *rPam.GetPoint() ),
    pRedlineData( new SwRedlineData( rData )),
    pContentSect( nullptr )
{
    bDelLastPara = bIsLastParaDelete = false;
    bIsVisible = true;
    if( !rPam.HasMark() )
        DeleteMark();
}

SwRangeRedline::SwRangeRedline( const SwRedlineData& rData, const SwPosition& rPos )
    : SwPaM( rPos ),
    pRedlineData( new SwRedlineData( rData )),
    pContentSect( nullptr )
{
    bDelLastPara = bIsLastParaDelete = false;
    bIsVisible = true;
}

SwRangeRedline::SwRangeRedline( const SwRangeRedline& rCpy )
    : SwPaM( *rCpy.GetMark(), *rCpy.GetPoint() ),
    pRedlineData( new SwRedlineData( *rCpy.pRedlineData )),
    pContentSect( nullptr )
{
    bDelLastPara = bIsLastParaDelete = false;
    bIsVisible = true;
    if( !rCpy.HasMark() )
        DeleteMark();
}

SwRangeRedline::~SwRangeRedline()
{
    if( pContentSect )
    {
        // delete the ContentSection
        if( !GetDoc()->IsInDtor() )
            GetDoc()->getIDocumentContentOperations().DeleteSection( &pContentSect->GetNode() );
        delete pContentSect;
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
        ( pPtNd != pMkNd || GetContentIdx() != nullptr ||
          pPtNd != &pPtNd->GetNodes().GetEndOfContent() )
        )
        return true;
    return false;
}

void SwRangeRedline::CallDisplayFunc(sal_uInt16 nLoop, size_t nMyPos)
{
    switch( nsRedlineMode_t::REDLINE_SHOW_MASK & GetDoc()->getIDocumentRedlineAccess().GetRedlineMode() )
    {
    case nsRedlineMode_t::REDLINE_SHOW_INSERT | nsRedlineMode_t::REDLINE_SHOW_DELETE:
        Show(nLoop, nMyPos);
        break;
    case nsRedlineMode_t::REDLINE_SHOW_INSERT:
        Hide(nLoop, nMyPos);
        break;
    case nsRedlineMode_t::REDLINE_SHOW_DELETE:
        ShowOriginal(nLoop, nMyPos);
        break;
    }
}

void SwRangeRedline::Show(sal_uInt16 nLoop, size_t nMyPos)
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
            MoveFromSection(nMyPos);
            break;

        case nsRedlineType_t::REDLINE_DELETE:           // Content has been deleted
            bIsVisible = true;
            MoveFromSection(nMyPos);
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

void SwRangeRedline::Hide(sal_uInt16 nLoop, size_t nMyPos)
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
            MoveFromSection(nMyPos);
        break;

    case nsRedlineType_t::REDLINE_DELETE:           // Content has been deleted
        bIsVisible = false;
        switch( nLoop )
        {
        case 0: MoveToSection();    break;
        case 1: CopyToSection();    break;
        case 2: DelCopyOfSection(nMyPos); break;
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

void SwRangeRedline::ShowOriginal(sal_uInt16 nLoop, size_t nMyPos)
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
        case 2: DelCopyOfSection(nMyPos); break;
        }
        break;

    case nsRedlineType_t::REDLINE_DELETE:           // Inhalt wurde eingefuegt
        bIsVisible = true;
        if( 1 <= nLoop )
            MoveFromSection(nMyPos);
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

        if (pNode && pNode->IsTextNode())
        {
            SwTextNode* pNd = pNode->GetTextNode();
            SwUpdateAttr aHt(
                n == nSttNd ? nSttCnt : 0,
                n == nEndNd ? nEndCnt : pNd->GetText().getLength(),
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
    if( !pContentSect )
    {
        const SwPosition* pStt = Start(),
                        * pEnd = pStt == GetPoint() ? GetMark() : GetPoint();

        SwDoc* pDoc = GetDoc();
        SwPaM aPam( *pStt, *pEnd );
        SwContentNode* pCSttNd = pStt->nNode.GetNode().GetContentNode();
        SwContentNode* pCEndNd = pEnd->nNode.GetNode().GetContentNode();

        if( !pCSttNd )
        {
            // In order to not move other Redlines' indices, we set them
            // to the end (is exclusive)
            const SwRedlineTable& rTable = pDoc->getIDocumentRedlineAccess().GetRedlineTable();
            for( SwRedlineTable::size_type n = 0; n < rTable.size(); ++n )
            {
                SwRangeRedline* pRedl = rTable[ n ];
                if( pRedl->GetBound() == *pStt )
                    pRedl->GetBound() = *pEnd;
                if( pRedl->GetBound(false) == *pStt )
                    pRedl->GetBound(false) = *pEnd;
            }
        }

        SwStartNode* pSttNd;
        SwNodes& rNds = pDoc->GetNodes();
        if( pCSttNd || pCEndNd )
        {
            SwTextFormatColl* pColl = (pCSttNd && pCSttNd->IsTextNode() )
                                    ? pCSttNd->GetTextNode()->GetTextColl()
                                    : (pCEndNd && pCEndNd->IsTextNode() )
                                        ? pCEndNd->GetTextNode()->GetTextColl()
                                        : pDoc->getIDocumentStylePoolAccess().GetTextCollFromPool(RES_POOLCOLL_STANDARD);

            pSttNd = rNds.MakeTextSection( SwNodeIndex( rNds.GetEndOfRedlines() ),
                                            SwNormalStartNode, pColl );
            SwTextNode* pTextNd = rNds[ pSttNd->GetIndex() + 1 ]->GetTextNode();

            SwNodeIndex aNdIdx( *pTextNd );
            SwPosition aPos( aNdIdx, SwIndex( pTextNd ));
            if( pCSttNd && pCEndNd )
                pDoc->getIDocumentContentOperations().MoveAndJoin( aPam, aPos, SwMoveFlags::DEFAULT );
            else
            {
                if( pCSttNd && !pCEndNd )
                    bDelLastPara = true;
                pDoc->getIDocumentContentOperations().MoveRange( aPam, aPos,
                    SwMoveFlags::DEFAULT );
            }
        }
        else
        {
            pSttNd = SwNodes::MakeEmptySection( SwNodeIndex( rNds.GetEndOfRedlines() ) );

            SwPosition aPos( *pSttNd->EndOfSectionNode() );
            pDoc->getIDocumentContentOperations().MoveRange( aPam, aPos,
                SwMoveFlags::DEFAULT );
        }
        pContentSect = new SwNodeIndex( *pSttNd );

        if( pStt == GetPoint() )
            Exchange();

        DeleteMark();
    }
    else
        InvalidateRange();
}

void SwRangeRedline::CopyToSection()
{
    if( !pContentSect )
    {
        const SwPosition* pStt = Start(),
                        * pEnd = pStt == GetPoint() ? GetMark() : GetPoint();

        SwContentNode* pCSttNd = pStt->nNode.GetNode().GetContentNode();
        SwContentNode* pCEndNd = pEnd->nNode.GetNode().GetContentNode();

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
            SwTextFormatColl* pColl = (pCSttNd && pCSttNd->IsTextNode() )
                                    ? pCSttNd->GetTextNode()->GetTextColl()
                                    : pDoc->getIDocumentStylePoolAccess().GetTextCollFromPool(RES_POOLCOLL_STANDARD);

            pSttNd = rNds.MakeTextSection( SwNodeIndex( rNds.GetEndOfRedlines() ),
                                            SwNormalStartNode, pColl );

            SwNodeIndex aNdIdx( *pSttNd, 1 );
            SwTextNode* pTextNd = aNdIdx.GetNode().GetTextNode();
            SwPosition aPos( aNdIdx, SwIndex( pTextNd ));
            pDoc->getIDocumentContentOperations().CopyRange( *this, aPos, /*bCopyAll=*/false, /*bCheckPos=*/true );

            // Take over the style from the EndNode if needed
            // We don't want this in Doc::Copy
            if( pCEndNd && pCEndNd != pCSttNd )
            {
                SwContentNode* pDestNd = aPos.nNode.GetNode().GetContentNode();
                if( pDestNd )
                {
                    if( pDestNd->IsTextNode() && pCEndNd->IsTextNode() )
                        pCEndNd->GetTextNode()->CopyCollFormat(*pDestNd->GetTextNode());
                    else
                        pDestNd->ChgFormatColl( pCEndNd->GetFormatColl() );
                }
            }
        }
        else
        {
            pSttNd = SwNodes::MakeEmptySection( SwNodeIndex( rNds.GetEndOfRedlines() ) );

            if( pCEndNd )
            {
                SwPosition aPos( *pSttNd->EndOfSectionNode() );
                pDoc->getIDocumentContentOperations().CopyRange( *this, aPos, /*bCopyAll=*/false, /*bCheckPos=*/true );
            }
            else
            {
                SwNodeIndex aInsPos( *pSttNd->EndOfSectionNode() );
                SwNodeRange aRg( pStt->nNode, 0, pEnd->nNode, 1 );
                pDoc->GetDocumentContentOperationsManager().CopyWithFlyInFly( aRg, 0, aInsPos );
            }
        }
        pContentSect = new SwNodeIndex( *pSttNd );

        pDoc->SetCopyIsMove( bSaveCopyFlag );
        pDoc->getIDocumentRedlineAccess().SetRedlineMove( bSaveRdlMoveFlg );
    }
}

void SwRangeRedline::DelCopyOfSection(size_t nMyPos)
{
    if( pContentSect )
    {
        const SwPosition* pStt = Start(),
                        * pEnd = pStt == GetPoint() ? GetMark() : GetPoint();

        SwDoc* pDoc = GetDoc();
        SwPaM aPam( *pStt, *pEnd );
        SwContentNode* pCSttNd = pStt->nNode.GetNode().GetContentNode();
        SwContentNode* pCEndNd = pEnd->nNode.GetNode().GetContentNode();

        if( !pCSttNd )
        {
            // In order to not move other Redlines' indices, we set them
            // to the end (is exclusive)
            const SwRedlineTable& rTable = pDoc->getIDocumentRedlineAccess().GetRedlineTable();
            for( SwRedlineTable::size_type n = 0; n < rTable.size(); ++n )
            {
                SwRangeRedline* pRedl = rTable[ n ];
                if( pRedl->GetBound() == *pStt )
                    pRedl->GetBound() = *pEnd;
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
                const SwRedlineTable& rTable = pDoc->getIDocumentRedlineAccess().GetRedlineTable();
                size_t n = nMyPos;
                OSL_ENSURE( n != USHRT_MAX, "How strange. We don't exist!" );
                for( bool bBreak = false; !bBreak && n > 0; )
                {
                    --n;
                    bBreak = true;
                    if( rTable[ n ]->GetBound() == *aPam.GetPoint() )
                    {
                        rTable[ n ]->GetBound() = *pEnd;
                        bBreak = false;
                    }
                    if( rTable[ n ]->GetBound(false) == *aPam.GetPoint() )
                    {
                        rTable[ n ]->GetBound(false) = *pEnd;
                        bBreak = false;
                    }
                }

                SwPosition aEnd( *pEnd );
                *GetPoint() = *pEnd;
                *GetMark() = *pEnd;
                DeleteMark();

                aPam.GetBound().nContent.Assign( nullptr, 0 );
                aPam.GetBound( false ).nContent.Assign( nullptr, 0 );
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

void SwRangeRedline::MoveFromSection(size_t nMyPos)
{
    if( pContentSect )
    {
        SwDoc* pDoc = GetDoc();
        const SwRedlineTable& rTable = pDoc->getIDocumentRedlineAccess().GetRedlineTable();
        std::vector<SwPosition*> aBeforeArr, aBehindArr;
        bool bBreak = false;
        SwRedlineTable::size_type n;

        for( n = nMyPos+1; !bBreak && n < rTable.size(); ++n )
        {
            bBreak = true;
            if( rTable[ n ]->GetBound() == *GetPoint() )
            {
                SwRangeRedline* pRedl = rTable[n];
                aBehindArr.push_back(&pRedl->GetBound());
                bBreak = false;
            }
            if( rTable[ n ]->GetBound(false) == *GetPoint() )
            {
                SwRangeRedline* pRedl = rTable[n];
                aBehindArr.push_back(&pRedl->GetBound(false));
                bBreak = false;
            }
        }
        for( bBreak = false, n = nMyPos; !bBreak && n ; )
        {
            --n;
            bBreak = true;
            if( rTable[ n ]->GetBound() == *GetPoint() )
            {
                SwRangeRedline* pRedl = rTable[n];
                aBeforeArr.push_back(&pRedl->GetBound());
                bBreak = false;
            }
            if( rTable[ n ]->GetBound(false) == *GetPoint() )
            {
                SwRangeRedline* pRedl = rTable[n];
                aBeforeArr.push_back(&pRedl->GetBound(false));
                bBreak = false;
            }
        }

        const SwNode* pKeptContentSectNode( &pContentSect->GetNode() ); // #i95711#
        {
            SwPaM aPam( pContentSect->GetNode(),
                        *pContentSect->GetNode().EndOfSectionNode(), 1,
                        ( bDelLastPara ? -2 : -1 ) );
            SwContentNode* pCNd = aPam.GetContentNode();
            if( pCNd )
                aPam.GetPoint()->nContent.Assign( pCNd, pCNd->Len() );
            else
                ++aPam.GetPoint()->nNode;

            SwFormatColl* pColl = pCNd && pCNd->Len() && aPam.GetPoint()->nNode !=
                                        aPam.GetMark()->nNode
                                ? pCNd->GetFormatColl() : nullptr;

            SwNodeIndex aNdIdx( GetPoint()->nNode, -1 );
            const sal_Int32 nPos = GetPoint()->nContent.GetIndex();

            SwPosition aPos( *GetPoint() );
            if( bDelLastPara && *aPam.GetPoint() == *aPam.GetMark() )
            {
                --aPos.nNode;

                pDoc->getIDocumentContentOperations().AppendTextNode( aPos );
            }
            else
            {
                pDoc->getIDocumentContentOperations().MoveRange( aPam, aPos,
                    SwMoveFlags::ALLFLYS );
            }

            SetMark();
            *GetPoint() = aPos;
            GetMark()->nNode = aNdIdx.GetIndex() + 1;
            pCNd = GetMark()->nNode.GetNode().GetContentNode();
            GetMark()->nContent.Assign( pCNd, nPos );

            if( bDelLastPara )
            {
                ++GetPoint()->nNode;
                GetPoint()->nContent.Assign( pCNd = GetContentNode(), 0 );
                bDelLastPara = false;
            }
            else if( pColl )
                pCNd = GetContentNode();

            if( pColl && pCNd )
                pCNd->ChgFormatColl( pColl );
        }

        // #i95771#
        // Under certain conditions the previous <SwDoc::Move(..)> has already
        // removed the change tracking section of this <SwRangeRedline> instance from
        // the change tracking nodes area.
        // Thus, check if <pContentSect> still points to the change tracking section
        // by comparing it with the "indexed" <SwNode> instance copied before
        // perform the intrinsic move.
        // Note: Such condition is e.g. a "delete" change tracking only containing a table.
        if ( &pContentSect->GetNode() == pKeptContentSectNode )
        {
            pDoc->getIDocumentContentOperations().DeleteSection( &pContentSect->GetNode() );
        }
        delete pContentSect;
        pContentSect = nullptr;

        // adjustment of redline table positions must take start and
        // end into account, not point and mark.
        for( auto& pItem : aBeforeArr )
            *pItem = *Start();
        for( auto& pItem : aBehindArr )
            *pItem = *End();
    }
    else
        InvalidateRange();
}

// for Undo
void SwRangeRedline::SetContentIdx( const SwNodeIndex* pIdx )
{
    if( pIdx && !pContentSect )
    {
        pContentSect = new SwNodeIndex( *pIdx );
        bIsVisible = false;
    }
    else if( !pIdx && pContentSect )
    {
        delete pContentSect;
        pContentSect = nullptr;
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
    pCur->pNext = nullptr;
    delete pCur;
    return true;
}

sal_uInt16 SwRangeRedline::GetStackCount() const
{
    sal_uInt16 nRet = 1;
    for( SwRedlineData* pCur = pRedlineData; pCur->pNext; pCur = pCur->pNext )
        ++nRet;
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
    if (*Start() < *rCmp.Start())
        return true;

    return *Start() == *rCmp.Start() && *End() < *rCmp.End();
}

const SwRedlineData & SwRangeRedline::GetRedlineData(sal_uInt16 nPos) const
{
    SwRedlineData * pCur = pRedlineData;

    while (nPos > 0 && nullptr != pCur->pNext)
    {
        pCur = pCur->pNext;

        nPos--;
    }

    OSL_ENSURE( 0 == nPos, "Pos is too big" );

    return *pCur;
}

OUString SwRangeRedline::GetDescr(sal_uInt16 nPos)
{
    // get description of redline data (e.g.: "insert $1")
    OUString aResult = GetRedlineData(nPos).GetDescr();

    SwPaM * pPaM = nullptr;
    bool bDeletePaM = false;

    // if this redline is visible the content is in this PaM
    if (nullptr == pContentSect)
    {
        pPaM = this;
    }
    else // otherwise it is saved in pContentSect
    {
        SwNodeIndex aTmpIdx( *pContentSect->GetNode().EndOfSectionNode() );
        pPaM = new SwPaM(*pContentSect, aTmpIdx );
        bDeletePaM = true;
    }

    // replace $1 in description by description of the redlines text
    const OUString aTmpStr = SW_RESSTR(STR_START_QUOTE)
        + ShortenString(pPaM->GetText(), nUndoStringLength, SW_RESSTR(STR_LDOTS))
        + SW_RESSTR(STR_END_QUOTE);

    SwRewriter aRewriter;
    aRewriter.AddRule(UndoArg1, aTmpStr);

    aResult = aRewriter.Apply(aResult);

    if (bDeletePaM)
        delete pPaM;

    return aResult;
}

void SwRangeRedline::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    xmlTextWriterStartElement(pWriter, BAD_CAST("swRangeRedline"));

    xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("ptr"), "%p", this);
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("id"), BAD_CAST(OString::number(GetSeqNo()).getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("author"), BAD_CAST(SW_MOD()->GetRedlineAuthor(GetAuthor()).toUtf8().getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("date"), BAD_CAST(DateTimeToOString(GetTimeStamp()).getStr()));

    OString sRedlineType;
    switch (GetType())
    {
        case nsRedlineType_t::REDLINE_INSERT:
            sRedlineType = "REDLINE_INSERT";
            break;
        case nsRedlineType_t::REDLINE_DELETE:
            sRedlineType = "REDLINE_DELETE";
            break;
        case nsRedlineType_t::REDLINE_FORMAT:
            sRedlineType = "REDLINE_FORMAT";
            break;
        default:
            sRedlineType = "UNKNOWN";
            break;
    }
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("type"), BAD_CAST(sRedlineType.getStr()));

    SwPaM::dumpAsXml(pWriter);

    xmlTextWriterEndElement(pWriter);
}

bool SwExtraRedlineTable::Insert( SwExtraRedline* p )
{
    m_aExtraRedlines.push_back( p );
    //p->CallDisplayFunc();
    return true;
}

void SwExtraRedlineTable::DeleteAndDestroy( sal_uInt16 nPos, sal_uInt16 nLen )
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
        pSh->InvalidateWindows( SwRect( 0, 0, SAL_MAX_INT32, SAL_MAX_INT32 ) );
    */
}

void SwExtraRedlineTable::DeleteAndDestroyAll()
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

