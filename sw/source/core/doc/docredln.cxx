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
#include <boost/property_tree/json_parser.hpp>

#include <osl/diagnose.h>
#include <sal/log.hxx>
#include <tools/datetimeutils.hxx>
#include <hintids.hxx>
#include <svl/itemiter.hxx>
#include <comphelper/lok.hxx>
#include <comphelper/string.hxx>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <unotools/datetime.hxx>
#include <sfx2/viewsh.hxx>
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
#include <UndoCore.hxx>
#include <hints.hxx>
#include <pamtyp.hxx>
#include <poolfmt.hxx>
#include <view.hxx>
#include <viewopt.hxx>
#include <usrpref.hxx>
#include <viewsh.hxx>
#include <viscrs.hxx>
#include <rootfrm.hxx>
#include <strings.hrc>
#include <wrtsh.hxx>
#include <txtfld.hxx>

#include <flowfrm.hxx>

using namespace com::sun::star;

#ifdef DBG_UTIL

    void sw_DebugRedline( const SwDoc* pDoc )
    {
        static SwRedlineTable::size_type nWatch = 0; // loplugin:constvars:ignore
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
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("SwExtraRedlineTable"));
    (void)xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("ptr"), "%p", this);

    for (sal_uInt16 nCurExtraRedlinePos = 0; nCurExtraRedlinePos < GetSize(); ++nCurExtraRedlinePos)
    {
        const SwExtraRedline* pExtraRedline = GetRedline(nCurExtraRedlinePos);
        (void)xmlTextWriterStartElement(pWriter, BAD_CAST("SwExtraRedline"));
        (void)xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("ptr"), "%p", this);
        (void)xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("symbol"), "%s", BAD_CAST(typeid(*pExtraRedline).name()));
        (void)xmlTextWriterEndElement(pWriter);
    }
    (void)xmlTextWriterEndElement(pWriter);
}

#if OSL_DEBUG_LEVEL > 0
static bool CheckPosition( const SwPosition* pStt, const SwPosition* pEnd )
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
    assert(pSttTab == pEndTab);
    if( pSttTab != pEndTab )
        nError = 1;
    assert(pSttTab || pSttStart == pEndStart);
    if( !pSttTab && pSttStart != pEndStart )
        nError |= 2;
    if( nError )
        nError += 10;
    return nError != 0;
}
#endif

bool SwExtraRedlineTable::DeleteAllTableRedlines( SwDoc& rDoc, const SwTable& rTable, bool bSaveInUndo, RedlineType nRedlineTypeToDelete )
{
    bool bChg = false;

    if (bSaveInUndo && rDoc.GetIDocumentUndoRedo().DoesUndo())
    {
        // #TODO - Add 'Undo' support for deleting 'Table Cell' redlines
        /*
        SwUndoRedline* pUndo = new SwUndoRedline( SwUndoId::REDLINE, rRange );
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
                const RedlineType nRedlineType = aRedlineData.GetType();

                // Check if this redline object type should be deleted
                if (RedlineType::Any == nRedlineTypeToDelete || nRedlineTypeToDelete == nRedlineType)
                {

                    DeleteAndDestroy( nCurRedlinePos );
                    bChg = true;
                    continue; // don't increment position after delete
                }
            }
        }
        ++nCurRedlinePos;
    }

    if( bChg )
        rDoc.getIDocumentState().SetModified();

    return bChg;
}

bool SwExtraRedlineTable::DeleteTableRowRedline( SwDoc* pDoc, const SwTableLine& rTableLine, bool bSaveInUndo, RedlineType nRedlineTypeToDelete )
{
    bool bChg = false;

    if (bSaveInUndo && pDoc->GetIDocumentUndoRedo().DoesUndo())
    {
        // #TODO - Add 'Undo' support for deleting 'Table Cell' redlines
        /*
        SwUndoRedline* pUndo = new SwUndoRedline( SwUndoId::REDLINE, rRange );
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
            const RedlineType nRedlineType = aRedlineData.GetType();

            // Check if this redline object type should be deleted
            if( RedlineType::Any != nRedlineTypeToDelete && nRedlineTypeToDelete != nRedlineType )
                continue;

            DeleteAndDestroy( nCurRedlinePos );
            bChg = true;
        }
    }

    if( bChg )
        pDoc->getIDocumentState().SetModified();

    return bChg;
}

bool SwExtraRedlineTable::DeleteTableCellRedline( SwDoc* pDoc, const SwTableBox& rTableBox, bool bSaveInUndo, RedlineType nRedlineTypeToDelete )
{
    bool bChg = false;

    if (bSaveInUndo && pDoc->GetIDocumentUndoRedo().DoesUndo())
    {
        // #TODO - Add 'Undo' support for deleting 'Table Cell' redlines
        /*
        SwUndoRedline* pUndo = new SwUndoRedline( SwUndoId::REDLINE, rRange );
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
            const RedlineType nRedlineType = aRedlineData.GetType();

            // Check if this redline object type should be deleted
            if( RedlineType::Any != nRedlineTypeToDelete && nRedlineTypeToDelete != nRedlineType )
                continue;

            DeleteAndDestroy( nCurRedlinePos );
            bChg = true;
        }
    }

    if( bChg )
        pDoc->getIDocumentState().SetModified();

    return bChg;
}

namespace
{

void lcl_LOKInvalidateFrames(const sw::BroadcastingModify& rMod, const SwRootFrame* pLayout,
        SwFrameType const nFrameType, const Point* pPoint)
{
    SwIterator<SwFrame, sw::BroadcastingModify, sw::IteratorMode::UnwrapMulti> aIter(rMod);

    for (SwFrame* pTmpFrame = aIter.First(); pTmpFrame; pTmpFrame = aIter.Next() )
    {
        if ((pTmpFrame->GetType() & nFrameType) &&
            (!pLayout || pLayout == pTmpFrame->getRootFrame()) &&
            (!pTmpFrame->IsFlowFrame() || !SwFlowFrame::CastFlowFrame( pTmpFrame )->IsFollow()))
        {
            if (pPoint)
            {
                pTmpFrame->InvalidateSize();
            }
        }
    }
}

void lcl_LOKInvalidateStartEndFrames(SwShellCursor& rCursor)
{
    if (!(rCursor.HasMark() &&
        rCursor.GetPoint()->nNode.GetNode().IsContentNode() &&
        rCursor.GetPoint()->nNode.GetNode().GetContentNode()->getLayoutFrame(rCursor.GetShell()->GetLayout()) &&
        (rCursor.GetMark()->nNode == rCursor.GetPoint()->nNode ||
        (rCursor.GetMark()->nNode.GetNode().IsContentNode() &&
         rCursor.GetMark()->nNode.GetNode().GetContentNode()->getLayoutFrame(rCursor.GetShell()->GetLayout())))))
    {
        return;
    }


    SwPosition *pStartPos = rCursor.Start(),
               *pEndPos   = rCursor.End();


    lcl_LOKInvalidateFrames(*(pStartPos->nNode.GetNode().GetContentNode()),
                            rCursor.GetShell()->GetLayout(),
                            FRM_CNTNT, &rCursor.GetSttPos());

    lcl_LOKInvalidateFrames(*(pEndPos->nNode.GetNode().GetContentNode()),
                            rCursor.GetShell()->GetLayout(),
                            FRM_CNTNT, &rCursor.GetEndPos());
}

bool lcl_LOKRedlineNotificationEnabled()
{
    static bool bDisableRedlineComments = getenv("DISABLE_REDLINE") != nullptr;
    if (comphelper::LibreOfficeKit::isActive() && !bDisableRedlineComments)
        return true;

    return false;
}

} // anonymous namespace

/// Emits LOK notification about one addition / removal of a redline item.
void SwRedlineTable::LOKRedlineNotification(RedlineNotification nType, SwRangeRedline* pRedline)
{
    // Disable since usability is very low beyond some small number of changes.
    if (!lcl_LOKRedlineNotificationEnabled())
        return;

    boost::property_tree::ptree aRedline;
    aRedline.put("action", (nType == RedlineNotification::Add ? "Add" :
                            (nType == RedlineNotification::Remove ? "Remove" :
                             (nType == RedlineNotification::Modify ? "Modify" : "???"))));
    aRedline.put("index", pRedline->GetId());
    aRedline.put("author", pRedline->GetAuthorString(1).toUtf8().getStr());
    aRedline.put("type", SwRedlineTypeToOUString(pRedline->GetRedlineData().GetType()).toUtf8().getStr());
    aRedline.put("comment", pRedline->GetRedlineData().GetComment().toUtf8().getStr());
    aRedline.put("description", pRedline->GetDescr().toUtf8().getStr());
    OUString sDateTime = utl::toISO8601(pRedline->GetRedlineData().GetTimeStamp().GetUNODateTime());
    aRedline.put("dateTime", sDateTime.toUtf8().getStr());

    SwPosition* pStartPos = pRedline->Start();
    SwPosition* pEndPos = pRedline->End();
    SwContentNode* pContentNd = pRedline->GetContentNode();
    SwView* pView = dynamic_cast<SwView*>(SfxViewShell::Current());
    if (pView && pContentNd)
    {
        SwShellCursor aCursor(pView->GetWrtShell(), *pStartPos);
        aCursor.SetMark();
        aCursor.GetMark()->nNode = pEndPos->nNode;
        aCursor.GetMark()->nContent = pEndPos->nContent;

        aCursor.FillRects();

        SwRects* pRects(&aCursor);
        std::vector<OString> aRects;
        for(const SwRect& rNextRect : *pRects)
            aRects.push_back(rNextRect.SVRect().toString());

        const OString sRects = comphelper::string::join("; ", aRects);
        aRedline.put("textRange", sRects.getStr());

        lcl_LOKInvalidateStartEndFrames(aCursor);

        // When this notify method is called text invalidation is not done yet
        // Calling FillRects updates the text area so invalidation will not run on the correct rects
        // So we need to do an own invalidation here. It invalidates text frames containing the redlining
        SwDoc& rDoc = pRedline->GetDoc();
        SwViewShell* pSh;
        if( !rDoc.IsInDtor() )
        {
            pSh = rDoc.getIDocumentLayoutAccess().GetCurrentViewShell();
            if( pSh )
                for(SwNodeIndex nIdx = pStartPos->nNode; nIdx <= pEndPos->nNode; ++nIdx)
                {
                    SwContentNode* pContentNode = nIdx.GetNode().GetContentNode();
                    if (pContentNode)
                        pSh->InvalidateWindows(pContentNode->FindLayoutRect());
                }
        }
    }

    boost::property_tree::ptree aTree;
    aTree.add_child("redline", aRedline);
    std::stringstream aStream;
    boost::property_tree::write_json(aStream, aTree);
    std::string aPayload = aStream.str();

    SfxViewShell* pViewShell = SfxViewShell::GetFirst();
    while (pViewShell)
    {
        if (pView && pView->GetDocId() == pViewShell->GetDocId())
            pViewShell->libreOfficeKitViewCallback(nType == RedlineNotification::Modify ? LOK_CALLBACK_REDLINE_TABLE_ENTRY_MODIFIED : LOK_CALLBACK_REDLINE_TABLE_SIZE_CHANGED, aPayload.c_str());
        pViewShell = SfxViewShell::GetNext(*pViewShell);
    }
}

bool SwRedlineTable::Insert(SwRangeRedline*& p)
{
    if( p->HasValidRange() )
    {
        std::pair<vector_type::const_iterator, bool> rv = maVector.insert( p );
        size_type nP = rv.first - begin();
        LOKRedlineNotification(RedlineNotification::Add, p);

        // set IsMoved checking nearby redlines
        isMoved(nP);

        p->CallDisplayFunc(nP);
        if (rv.second)
            CheckOverlapping(rv.first);
        return rv.second;
    }
    return InsertWithValidRanges( p );
}

void SwRedlineTable::CheckOverlapping(vector_type::const_iterator it)
{
    if (m_bHasOverlappingElements)
        return;
    if (maVector.size() <= 1) // a single element cannot be overlapping
        return;
    auto pCurr = *it;
    auto itNext = it + 1;
    if (itNext != maVector.end())
    {
        auto pNext = *itNext;
        if (pCurr->End()->nNode.GetIndex() >= pNext->Start()->nNode.GetIndex())
        {
            m_bHasOverlappingElements = true;
            return;
        }
    }
    if (it != maVector.begin())
    {
        auto pPrev = *(it - 1);
        if (pPrev->End()->nNode.GetIndex() >= pCurr->Start()->nNode.GetIndex())
            m_bHasOverlappingElements = true;
    }
}

bool SwRedlineTable::Insert(SwRangeRedline*& p, size_type& rP)
{
    if( p->HasValidRange() )
    {
        std::pair<vector_type::const_iterator, bool> rv = maVector.insert( p );
        rP = rv.first - begin();
        p->CallDisplayFunc(rP);
        if (rv.second)
            CheckOverlapping(rv.first);
        return rv.second;
    }
    return InsertWithValidRanges( p, &rP );
}

namespace sw {

std::vector<SwRangeRedline*> GetAllValidRanges(std::unique_ptr<SwRangeRedline> p)
{
    std::vector<SwRangeRedline*> ret;
    // Create valid "sub-ranges" from the Selection
    SwPosition* pStt = p->Start(),
              * pEnd = p->End();
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
                pNew->HasValidRange())
            {
                ret.push_back(pNew);
                pNew = nullptr;
            }

            if( aNewStt >= *pEnd )
                break;
            pC = rNds.GoNext( &aNewStt.nNode );
            if( !pC )
                break;

            aNewStt.nContent.Assign( pC, 0 );

        } while( aNewStt < *pEnd );

    delete pNew;
    p.reset();
    return ret;
}

} // namespace sw

bool SwRedlineTable::InsertWithValidRanges(SwRangeRedline*& p, size_type* pInsPos)
{
    bool bAnyIns = false;
    std::vector<SwRangeRedline*> const redlines(
            GetAllValidRanges(std::unique_ptr<SwRangeRedline>(p)));
    for (SwRangeRedline * pRedline : redlines)
    {
        assert(pRedline->HasValidRange());
        size_type nInsPos;
        if (Insert(pRedline, nInsPos))
        {
            pRedline->CallDisplayFunc(nInsPos);
            bAnyIns = true;
            if (pInsPos && *pInsPos < nInsPos)
            {
                *pInsPos = nInsPos;
            }
        }
    }
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

SwRedlineTable::size_type SwRedlineTable::GetPos(const SwRangeRedline* p) const
{
    vector_type::const_iterator it = maVector.find(const_cast<SwRangeRedline*>(p));
    if( it == maVector.end() )
        return npos;
    return it - maVector.begin();
}

void SwRedlineTable::Remove( const SwRangeRedline* p )
{
    const size_type nPos = GetPos(p);
    if (nPos == npos)
        return;
    Remove(nPos);
}

void SwRedlineTable::Remove( size_type nP )
{
    LOKRedlineNotification(RedlineNotification::Remove, maVector[nP]);
    SwDoc* pDoc = nullptr;
    if( !nP && 1 == size() )
        pDoc = &maVector.front()->GetDoc();

    maVector.erase( maVector.begin() + nP );

    if( pDoc && !pDoc->IsInDtor() )
    {
        SwViewShell* pSh = pDoc->getIDocumentLayoutAccess().GetCurrentViewShell();
        if( pSh )
            pSh->InvalidateWindows( SwRect( 0, 0, SAL_MAX_INT32, SAL_MAX_INT32 ) );
    }
}

void SwRedlineTable::DeleteAndDestroyAll()
{
    while (!maVector.empty())
    {
        auto const pRedline = maVector.back();
        maVector.erase_at(maVector.size() - 1);
        LOKRedlineNotification(RedlineNotification::Remove, pRedline);
        delete pRedline;
    }
    m_bHasOverlappingElements = false;
}

void SwRedlineTable::DeleteAndDestroy(size_type const nP)
{
    auto const pRedline = maVector[nP];
    maVector.erase(maVector.begin() + nP);
    LOKRedlineNotification(RedlineNotification::Remove, pRedline);
    delete pRedline;
}

SwRedlineTable::size_type SwRedlineTable::FindNextOfSeqNo( size_type nSttPos ) const
{
    return nSttPos + 1 < size()
                ? FindNextSeqNo( operator[]( nSttPos )->GetSeqNo(), nSttPos+1 )
                : npos;
}

SwRedlineTable::size_type SwRedlineTable::FindPrevOfSeqNo( size_type nSttPos ) const
{
    return nSttPos ? FindPrevSeqNo( operator[]( nSttPos )->GetSeqNo(), nSttPos-1 )
                   : npos;
}

/// Find the next or preceding Redline with the same seq.no.
/// We can limit the search using look ahead (0 searches the whole array).
SwRedlineTable::size_type SwRedlineTable::FindNextSeqNo( sal_uInt16 nSeqNo, size_type nSttPos ) const
{
    auto constexpr nLookahead = 20;
    size_type nRet = npos;
    if( nSeqNo && nSttPos < size() )
    {
        size_type nEnd = size();
        const size_type nTmp = nSttPos + nLookahead;
        if (nTmp < nEnd)
        {
            nEnd = nTmp;
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

SwRedlineTable::size_type SwRedlineTable::FindPrevSeqNo( sal_uInt16 nSeqNo, size_type nSttPos ) const
{
    auto constexpr nLookahead = 20;
    size_type nRet = npos;
    if( nSeqNo && nSttPos < size() )
    {
        size_type nEnd = 0;
        if( nSttPos > nLookahead )
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
                                        size_type& rPos,
                                        bool bNext ) const
{
    const SwRangeRedline* pFnd = nullptr;
    for( ; rPos < maVector.size() ; ++rPos )
    {
        const SwRangeRedline* pTmp = (*this)[ rPos ];
        if( pTmp->HasMark() && pTmp->IsVisible() )
        {
            const SwPosition* pRStt = pTmp->Start(),
                      * pREnd = pTmp->End();
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

bool SwRedlineTable::isMoved( size_type rPos ) const
{
    bool bRet = false;
    auto constexpr nLookahead = 20;
    SwRangeRedline* pRedline = (*this)[ rPos ];

    // set redline type of the searched pair
    RedlineType nPairType = pRedline->GetType();
    if ( RedlineType::Delete == nPairType )
        nPairType = RedlineType::Insert;
    else if ( RedlineType::Insert == nPairType )
        nPairType = RedlineType::Delete;
    else
        // only deleted or inserted text can be moved
        return false;

    bool bDeletePaM = false;
    SwPaM* pPaM;

    // if this redline is visible the content is in this PaM
    if ( nullptr == pRedline->GetContentIdx() )
    {
        pPaM = pRedline;
    }
    else // otherwise it is saved in pContentSect, e.g. during ODT import
    {
        SwNodeIndex aTmpIdx( *pRedline->GetContentIdx()->GetNode().EndOfSectionNode() );
        pPaM = new SwPaM(*pRedline->GetContentIdx(), aTmpIdx );
        bDeletePaM = true;
    }

    const OUString sTrimmed = pPaM->GetText().trim();
    if ( sTrimmed.isEmpty() )
    {
        if ( bDeletePaM )
            delete pPaM;
        return false;
    }

    // search pair around the actual redline
    size_type nEnd = rPos + nLookahead < size()
        ? rPos + nLookahead
        : size();
    rPos = rPos > nLookahead ? rPos - nLookahead : 0;
    for ( ; rPos < nEnd && !bRet ; ++rPos )
    {
        SwRangeRedline* pPair = (*this)[ rPos ];

        // redline must be the requested type and from the same author
        if ( nPairType != pPair->GetType() ||
             pRedline->GetAuthor() != pPair->GetAuthor() )
        {
            continue;
        }

        bool bDeletePairPaM = false;
        SwPaM* pPairPaM;

        // if this redline is visible the content is in this PaM
        if ( nullptr == pPair->GetContentIdx() )
        {
            pPairPaM = pPair;
        }
        else // otherwise it is saved in pContentSect, e.g. during ODT import
        {
            // saved in pContentSect, e.g. during ODT import
            SwNodeIndex aTmpIdx( *pPair->GetContentIdx()->GetNode().EndOfSectionNode() );
            pPairPaM = new SwPaM(*pPair->GetContentIdx(), aTmpIdx );
            bDeletePairPaM = true;
        }

        // pair at tracked moving: same text by trimming trailing white spaces
        if ( abs(pPaM->GetText().getLength() - pPairPaM->GetText().getLength()) <= 2 &&
            sTrimmed == pPairPaM->GetText().trim() )
        {
            pRedline->SetMoved();
            pPair->SetMoved();
            pPair->InvalidateRange(SwRangeRedline::Invalidation::Remove);
            bRet = true;
        }

        if ( bDeletePairPaM )
            delete pPairPaM;
    }

    if ( bDeletePaM )
        delete pPaM;

    return bRet;
}

void SwRedlineTable::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("SwRedlineTable"));
    (void)xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("ptr"), "%p", this);

    for (SwRedlineTable::size_type nCurRedlinePos = 0; nCurRedlinePos < size(); ++nCurRedlinePos)
        operator[](nCurRedlinePos)->dumpAsXml(pWriter);

    (void)xmlTextWriterEndElement(pWriter);
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
                                                const SfxItemSet* pItemSet,
                                                bool bFormatAll )
    : m_sFormatNm(rColl), m_nPoolId(nPoolFormatId), m_bFormatAll(bFormatAll)
{
    if( pItemSet && pItemSet->Count() )
        m_pSet.reset( new SfxItemSet( *pItemSet ) );
}

SwRedlineExtraData_FormatColl::~SwRedlineExtraData_FormatColl()
{
}

SwRedlineExtraData* SwRedlineExtraData_FormatColl::CreateNew() const
{
    return new SwRedlineExtraData_FormatColl( m_sFormatNm, m_nPoolId, m_pSet.get(), m_bFormatAll );
}

void SwRedlineExtraData_FormatColl::Reject( SwPaM& rPam ) const
{
    SwDoc& rDoc = rPam.GetDoc();

    // What about Undo? Is it turned off?
    SwTextFormatColl* pColl = USHRT_MAX == m_nPoolId
                            ? rDoc.FindTextFormatCollByName( m_sFormatNm )
                            : rDoc.getIDocumentStylePoolAccess().GetTextCollFromPool( m_nPoolId );

    RedlineFlags eOld = rDoc.getIDocumentRedlineAccess().GetRedlineFlags();
    rDoc.getIDocumentRedlineAccess().SetRedlineFlags_intern(eOld & ~RedlineFlags(RedlineFlags::On | RedlineFlags::Ignore));

    SwPaM aPam( *rPam.GetMark(), *rPam.GetPoint() );

    const SwPosition* pEnd = rPam.End();

    if ( !m_bFormatAll || pEnd->nContent == 0 )
    {
        // don't reject the format of the next paragraph (that is handled by the next redline)
        if (aPam.GetPoint()->nNode > aPam.GetMark()->nNode)
        {
            aPam.GetPoint()->nNode--;
            SwContentNode* pNode = aPam.GetPoint()->nNode.GetNode().GetContentNode();
            aPam.GetPoint()->nContent.Assign( pNode, pNode->Len() );
        }
        else if (aPam.GetPoint()->nNode < aPam.GetMark()->nNode)
        {
            aPam.GetMark()->nNode--;
            SwContentNode* pNode = aPam.GetMark()->nNode.GetNode().GetContentNode();
            aPam.GetMark()->nContent.Assign( pNode, pNode->Len() );
        }
    }

    if( pColl )
        rDoc.SetTextFormatColl( aPam, pColl, false );

    if( m_pSet )
        rDoc.getIDocumentContentOperations().InsertItemSet( aPam, *m_pSet );

    rDoc.getIDocumentRedlineAccess().SetRedlineFlags_intern( eOld );
}

bool SwRedlineExtraData_FormatColl::operator == ( const SwRedlineExtraData& r) const
{
    const SwRedlineExtraData_FormatColl& rCmp = static_cast<const SwRedlineExtraData_FormatColl&>(r);
    return m_sFormatNm == rCmp.m_sFormatNm && m_nPoolId == rCmp.m_nPoolId &&
            m_bFormatAll == rCmp.m_bFormatAll &&
            ( ( !m_pSet && !rCmp.m_pSet ) ||
               ( m_pSet && rCmp.m_pSet && *m_pSet == *rCmp.m_pSet ) );
}

void SwRedlineExtraData_FormatColl::SetItemSet( const SfxItemSet& rSet )
{
    if( rSet.Count() )
        m_pSet.reset( new SfxItemSet( rSet ) );
    else
        m_pSet.reset();
}

SwRedlineExtraData_Format::SwRedlineExtraData_Format( const SfxItemSet& rSet )
{
    SfxItemIter aIter( rSet );
    for (const SfxPoolItem* pItem = aIter.GetCurItem(); pItem; pItem = aIter.NextItem())
    {
        m_aWhichIds.push_back( pItem->Which() );
    }
}

SwRedlineExtraData_Format::SwRedlineExtraData_Format(
        const SwRedlineExtraData_Format& rCpy )
    : SwRedlineExtraData()
{
    m_aWhichIds.insert( m_aWhichIds.begin(), rCpy.m_aWhichIds.begin(), rCpy.m_aWhichIds.end() );
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
    SwDoc& rDoc = rPam.GetDoc();

    RedlineFlags eOld = rDoc.getIDocumentRedlineAccess().GetRedlineFlags();
    rDoc.getIDocumentRedlineAccess().SetRedlineFlags_intern(eOld & ~RedlineFlags(RedlineFlags::On | RedlineFlags::Ignore));

    // Actually we need to reset the Attribute here!
    for( const auto& rWhichId : m_aWhichIds )
    {
        rDoc.getIDocumentContentOperations().InsertPoolItem( rPam, *GetDfltAttr( rWhichId ),
            SetAttrMode::DONTEXPAND );
    }

    rDoc.getIDocumentRedlineAccess().SetRedlineFlags_intern( eOld );
}

bool SwRedlineExtraData_Format::operator == ( const SwRedlineExtraData& rCmp ) const
{
    const size_t nEnd = m_aWhichIds.size();
    if( nEnd != static_cast<const SwRedlineExtraData_Format&>(rCmp).m_aWhichIds.size() )
        return false;

    for( size_t n = 0; n < nEnd; ++n )
    {
        if( static_cast<const SwRedlineExtraData_Format&>(rCmp).m_aWhichIds[n] != m_aWhichIds[n])
        {
            return false;
        }
    }
    return true;
}

SwRedlineData::SwRedlineData( RedlineType eT, std::size_t nAut )
    : m_pNext( nullptr ), m_pExtraData( nullptr ),
    m_aStamp( DateTime::SYSTEM ),
    m_nAuthor( nAut ), m_eType( eT ), m_nSeqNo( 0 ), m_bAutoFormat(false), m_bMoved(false)
{
    m_aStamp.SetNanoSec( 0 );
}

SwRedlineData::SwRedlineData(
    const SwRedlineData& rCpy,
    bool bCpyNext )
    : m_pNext( ( bCpyNext && rCpy.m_pNext ) ? new SwRedlineData( *rCpy.m_pNext ) : nullptr )
    , m_pExtraData( rCpy.m_pExtraData ? rCpy.m_pExtraData->CreateNew() : nullptr )
    , m_sComment( rCpy.m_sComment )
    , m_aStamp( rCpy.m_aStamp )
    , m_nAuthor( rCpy.m_nAuthor )
    , m_eType( rCpy.m_eType )
    , m_nSeqNo( rCpy.m_nSeqNo )
    , m_bAutoFormat(false)
    , m_bMoved( rCpy.m_bMoved )
{
}

// For sw3io: We now own pNext!
SwRedlineData::SwRedlineData(RedlineType eT, std::size_t nAut, const DateTime& rDT,
    const OUString& rCmnt, SwRedlineData *pNxt)
    : m_pNext(pNxt), m_pExtraData(nullptr), m_sComment(rCmnt), m_aStamp(rDT),
    m_nAuthor(nAut), m_eType(eT), m_nSeqNo(0), m_bAutoFormat(false), m_bMoved(false)
{
}

SwRedlineData::~SwRedlineData()
{
    delete m_pExtraData;
    delete m_pNext;
}

bool SwRedlineData::CanCombine(const SwRedlineData& rCmp) const
{
    DateTime aTime = GetTimeStamp();
    aTime.SetSec(0);
    DateTime aCompareTime = rCmp.GetTimeStamp();
    aCompareTime.SetSec(0);
    return m_nAuthor == rCmp.m_nAuthor &&
            m_eType == rCmp.m_eType &&
            m_sComment == rCmp.m_sComment &&
            aTime == aCompareTime &&
            m_bMoved == rCmp.m_bMoved &&
            (( !m_pNext && !rCmp.m_pNext ) ||
                ( m_pNext && rCmp.m_pNext &&
                m_pNext->CanCombine( *rCmp.m_pNext ))) &&
            (( !m_pExtraData && !rCmp.m_pExtraData ) ||
                ( m_pExtraData && rCmp.m_pExtraData &&
                    *m_pExtraData == *rCmp.m_pExtraData ));
}

/// ExtraData is copied. The Pointer's ownership is thus NOT transferred
/// to the Redline Object!
void SwRedlineData::SetExtraData( const SwRedlineExtraData* pData )
{
    delete m_pExtraData;

    // Check if there is data - and if so - delete it
    if( pData )
        m_pExtraData = pData->CreateNew();
    else
        m_pExtraData = nullptr;
}

const TranslateId STR_REDLINE_ARY[] =
{
    STR_UNDO_REDLINE_INSERT,
    STR_UNDO_REDLINE_DELETE,
    STR_UNDO_REDLINE_FORMAT,
    STR_UNDO_REDLINE_TABLE,
    STR_UNDO_REDLINE_FMTCOLL,
    STR_UNDO_REDLINE_PARAGRAPH_FORMAT,
    STR_UNDO_REDLINE_TABLE_ROW_INSERT,
    STR_UNDO_REDLINE_TABLE_ROW_DELETE,
    STR_UNDO_REDLINE_TABLE_CELL_INSERT,
    STR_UNDO_REDLINE_TABLE_CELL_DELETE
};

OUString SwRedlineData::GetDescr() const
{
    return SwResId(STR_REDLINE_ARY[static_cast<int>(GetType())]);
}

sal_uInt32 SwRangeRedline::s_nLastId = 1;

SwRangeRedline::SwRangeRedline(RedlineType eTyp, const SwPaM& rPam )
    : SwPaM( *rPam.GetMark(), *rPam.GetPoint() ),
    m_pRedlineData( new SwRedlineData( eTyp, GetDoc().getIDocumentRedlineAccess().GetRedlineAuthor() ) ),
    m_pContentSect( nullptr ),
    m_nId( s_nLastId++ )
{
    m_bDelLastPara = false;
    m_bIsVisible = true;
    if( !rPam.HasMark() )
        DeleteMark();
}

SwRangeRedline::SwRangeRedline( const SwRedlineData& rData, const SwPaM& rPam )
    : SwPaM( *rPam.GetMark(), *rPam.GetPoint() ),
    m_pRedlineData( new SwRedlineData( rData )),
    m_pContentSect( nullptr ),
    m_nId( s_nLastId++ )
{
    m_bDelLastPara = false;
    m_bIsVisible = true;
    if( !rPam.HasMark() )
        DeleteMark();
}

SwRangeRedline::SwRangeRedline( const SwRedlineData& rData, const SwPosition& rPos )
    : SwPaM( rPos ),
    m_pRedlineData( new SwRedlineData( rData )),
    m_pContentSect( nullptr ),
    m_nId( s_nLastId++ )
{
    m_bDelLastPara = false;
    m_bIsVisible = true;
}

SwRangeRedline::SwRangeRedline( const SwRangeRedline& rCpy )
    : SwPaM( *rCpy.GetMark(), *rCpy.GetPoint() ),
    m_pRedlineData( new SwRedlineData( *rCpy.m_pRedlineData )),
    m_pContentSect( nullptr ),
    m_nId( s_nLastId++ )
{
    m_bDelLastPara = false;
    m_bIsVisible = true;
    if( !rCpy.HasMark() )
        DeleteMark();
}

SwRangeRedline::~SwRangeRedline()
{
    if( m_pContentSect )
    {
        // delete the ContentSection
        if( !GetDoc().IsInDtor() )
            GetDoc().getIDocumentContentOperations().DeleteSection( &m_pContentSect->GetNode() );
        delete m_pContentSect;
    }
    delete m_pRedlineData;
}

void MaybeNotifyRedlineModification(SwRangeRedline& rRedline, SwDoc& rDoc)
{
    if (!lcl_LOKRedlineNotificationEnabled())
        return;

    const SwRedlineTable& rRedTable = rDoc.getIDocumentRedlineAccess().GetRedlineTable();
    for (SwRedlineTable::size_type i = 0; i < rRedTable.size(); ++i)
    {
        if (rRedTable[i] == &rRedline)
        {
            SwRedlineTable::LOKRedlineNotification(RedlineNotification::Modify, &rRedline);
            break;
        }
    }
}

void SwRangeRedline::MaybeNotifyRedlinePositionModification(tools::Long nTop)
{
    if (!lcl_LOKRedlineNotificationEnabled())
        return;

    if(!m_oLOKLastNodeTop || *m_oLOKLastNodeTop != nTop)
    {
        m_oLOKLastNodeTop = nTop;
        SwRedlineTable::LOKRedlineNotification(RedlineNotification::Modify, this);
    }
}

void SwRangeRedline::SetStart( const SwPosition& rPos, SwPosition* pSttPtr )
{
    if( !pSttPtr ) pSttPtr = Start();
    *pSttPtr = rPos;

    MaybeNotifyRedlineModification(*this, GetDoc());
}

void SwRangeRedline::SetEnd( const SwPosition& rPos, SwPosition* pEndPtr )
{
    if( !pEndPtr ) pEndPtr = End();
    *pEndPtr = rPos;

    MaybeNotifyRedlineModification(*this, GetDoc());
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

void SwRangeRedline::CallDisplayFunc(size_t nMyPos)
{
    RedlineFlags eShow = RedlineFlags::ShowMask & GetDoc().getIDocumentRedlineAccess().GetRedlineFlags();
    if (eShow == (RedlineFlags::ShowInsert | RedlineFlags::ShowDelete))
        Show(0, nMyPos);
    else if (eShow == RedlineFlags::ShowInsert)
        Hide(0, nMyPos);
    else if (eShow == RedlineFlags::ShowDelete)
        ShowOriginal(0, nMyPos);
}

void SwRangeRedline::Show(sal_uInt16 nLoop, size_t nMyPos, bool bForced)
{
    SwDoc& rDoc = GetDoc();

    bool bIsShowChangesInMargin = false;
    if ( !bForced )
    {
        SwViewShell* pSh = rDoc.getIDocumentLayoutAccess().GetCurrentViewShell();
        if (pSh)
            bIsShowChangesInMargin = pSh->GetViewOptions()->IsShowChangesInMargin();
        else
            bIsShowChangesInMargin = SW_MOD()->GetUsrPref(false)->IsShowChangesInMargin();
    }

    if( 1 > nLoop && !bIsShowChangesInMargin )
        return;

    RedlineFlags eOld = rDoc.getIDocumentRedlineAccess().GetRedlineFlags();
    rDoc.getIDocumentRedlineAccess().SetRedlineFlags_intern(eOld | RedlineFlags::Ignore);
    ::sw::UndoGuard const undoGuard(rDoc.GetIDocumentUndoRedo());

    switch( GetType() )
    {
    case RedlineType::Insert:           // Content has been inserted
        m_bIsVisible = true;
        MoveFromSection(nMyPos);
        break;

    case RedlineType::Delete:           // Content has been deleted
        m_bIsVisible = !bIsShowChangesInMargin;

        if (m_bIsVisible)
            MoveFromSection(nMyPos);
        else
        {
            switch( nLoop )
            {
            case 0: MoveToSection();    break;
            case 1: CopyToSection();    break;
            case 2: DelCopyOfSection(nMyPos); break;
            }
        }
        break;

    case RedlineType::Format:           // Attributes have been applied
    case RedlineType::Table:            // Table structure has been modified
        InvalidateRange(Invalidation::Add);
        break;
    default:
        break;
    }
    rDoc.getIDocumentRedlineAccess().SetRedlineFlags_intern( eOld );
}

void SwRangeRedline::Hide(sal_uInt16 nLoop, size_t nMyPos, bool /*bForced*/)
{
    SwDoc& rDoc = GetDoc();
    RedlineFlags eOld = rDoc.getIDocumentRedlineAccess().GetRedlineFlags();
    rDoc.getIDocumentRedlineAccess().SetRedlineFlags_intern(eOld | RedlineFlags::Ignore);
    ::sw::UndoGuard const undoGuard(rDoc.GetIDocumentUndoRedo());

    switch( GetType() )
    {
    case RedlineType::Insert:           // Content has been inserted
        m_bIsVisible = true;
        if( 1 <= nLoop )
            MoveFromSection(nMyPos);
        break;

    case RedlineType::Delete:           // Content has been deleted
        m_bIsVisible = false;
        switch( nLoop )
        {
        case 0: MoveToSection();    break;
        case 1: CopyToSection();    break;
        case 2: DelCopyOfSection(nMyPos); break;
        }
        break;

    case RedlineType::Format:           // Attributes have been applied
    case RedlineType::Table:            // Table structure has been modified
        if( 1 <= nLoop )
            InvalidateRange(Invalidation::Remove);
        break;
    default:
        break;
    }
    rDoc.getIDocumentRedlineAccess().SetRedlineFlags_intern( eOld );
}

void SwRangeRedline::ShowOriginal(sal_uInt16 nLoop, size_t nMyPos, bool /*bForced*/)
{
    SwDoc& rDoc = GetDoc();
    RedlineFlags eOld = rDoc.getIDocumentRedlineAccess().GetRedlineFlags();
    SwRedlineData* pCur;

    rDoc.getIDocumentRedlineAccess().SetRedlineFlags_intern(eOld | RedlineFlags::Ignore);
    ::sw::UndoGuard const undoGuard(rDoc.GetIDocumentUndoRedo());

    // Determine the Type, it's the first on Stack
    for( pCur = m_pRedlineData; pCur->m_pNext; )
        pCur = pCur->m_pNext;

    switch( pCur->m_eType )
    {
    case RedlineType::Insert:           // Content has been inserted
        m_bIsVisible = false;
        switch( nLoop )
        {
        case 0: MoveToSection();    break;
        case 1: CopyToSection();    break;
        case 2: DelCopyOfSection(nMyPos); break;
        }
        break;

    case RedlineType::Delete:           // Content has been deleted
        m_bIsVisible = true;
        if( 1 <= nLoop )
            MoveFromSection(nMyPos);
        break;

    case RedlineType::Format:           // Attributes have been applied
    case RedlineType::Table:            // Table structure has been modified
        if( 1 <= nLoop )
            InvalidateRange(Invalidation::Remove);
        break;
    default:
        break;
    }
    rDoc.getIDocumentRedlineAccess().SetRedlineFlags_intern( eOld );
}

// trigger the Layout
void SwRangeRedline::InvalidateRange(Invalidation const eWhy)
{
    SwNodeOffset nSttNd = Start()->nNode.GetIndex(),
                 nEndNd = End()->nNode.GetIndex();
    sal_Int32 nSttCnt = Start()->nContent.GetIndex();
    sal_Int32 nEndCnt = End()->nContent.GetIndex();

    SwNodes& rNds = GetDoc().GetNodes();
    for (SwNodeOffset n(nSttNd); n <= nEndNd; ++n)
    {
        SwNode* pNode = rNds[n];

        if (pNode && pNode->IsTextNode())
        {
            SwTextNode* pNd = pNode->GetTextNode();

            SwUpdateAttr aHt(
                n == nSttNd ? nSttCnt : 0,
                n == nEndNd ? nEndCnt : pNd->GetText().getLength(),
                RES_FMT_CHG);

            pNd->TriggerNodeUpdate(sw::LegacyModifyHint(&aHt, &aHt));

            // SwUpdateAttr must be handled first, otherwise indexes are off
            if (GetType() == RedlineType::Delete)
            {
                sal_Int32 const nStart(n == nSttNd ? nSttCnt : 0);
                sal_Int32 const nLen((n == nEndNd ? nEndCnt : pNd->GetText().getLength()) - nStart);
                if (eWhy == Invalidation::Add)
                {
                    sw::RedlineDelText const hint(nStart, nLen);
                    pNd->CallSwClientNotify(hint);
                }
                else
                {
                    sw::RedlineUnDelText const hint(nStart, nLen);
                    pNd->CallSwClientNotify(hint);
                }
            }
        }
    }
}

/** Calculates the start and end position of the intersection rTmp and
    text node nNdIdx */
void SwRangeRedline::CalcStartEnd( SwNodeOffset nNdIdx, sal_Int32& rStart, sal_Int32& rEnd ) const
{
    const SwPosition *pRStt = Start(), *pREnd = End();
    if( pRStt->nNode < nNdIdx )
    {
        if( pREnd->nNode > nNdIdx )
        {
            rStart = 0;             // Paragraph is completely enclosed
            rEnd = COMPLETE_STRING;
        }
        else if (pREnd->nNode == nNdIdx)
        {
            rStart = 0;             // Paragraph is overlapped in the beginning
            rEnd = pREnd->nContent.GetIndex();
        }
        else // redline ends before paragraph
        {
            rStart = COMPLETE_STRING;
            rEnd = COMPLETE_STRING;
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

static void lcl_storeAnnotationMarks(SwDoc& rDoc, const SwPosition* pStt, const SwPosition* pEnd)
{
    // tdf#115815 keep original start position of collapsed annotation ranges
    // as temporary bookmarks (removed after file saving and file loading)
    IDocumentMarkAccess& rDMA(*rDoc.getIDocumentMarkAccess());
    for (auto iter = rDMA.getAnnotationMarksBegin();
          iter != rDMA.getAnnotationMarksEnd(); )
    {
        SwPosition const& rStartPos((**iter).GetMarkStart());
        if ( *pStt <= rStartPos && rStartPos < *pEnd )
        {
            IDocumentMarkAccess::const_iterator_t pOldMark =
                    rDMA.findAnnotationBookmark((**iter).GetName());
            if ( pOldMark == rDMA.getBookmarksEnd() )
            {
                // at start of redlines use a 1-character length bookmark range
                // instead of a 0-character length bookmark position to avoid its losing
                sal_Int32 nLen = (*pStt == rStartPos) ? 1 : 0;
                SwPaM aPam( rStartPos.nNode, rStartPos.nContent.GetIndex(),
                                rStartPos.nNode, rStartPos.nContent.GetIndex() + nLen);
                ::sw::mark::IMark* pMark = rDMA.makeAnnotationBookmark(
                    aPam,
                    (**iter).GetName(),
                    IDocumentMarkAccess::MarkType::BOOKMARK, sw::mark::InsertMode::New);
                ::sw::mark::IBookmark* pBookmark = dynamic_cast< ::sw::mark::IBookmark* >(pMark);
                if (pBookmark)
                {
                    pBookmark->SetKeyCode(vcl::KeyCode());
                    pBookmark->SetShortName(OUString());
                }
            }
        }
        ++iter;
    }
}

void SwRangeRedline::MoveToSection()
{
    if( !m_pContentSect )
    {
        const SwPosition* pStt = Start(),
                        * pEnd = End();

        SwDoc& rDoc = GetDoc();
        SwPaM aPam( *pStt, *pEnd );
        SwContentNode* pCSttNd = pStt->nNode.GetNode().GetContentNode();
        SwContentNode* pCEndNd = pEnd->nNode.GetNode().GetContentNode();

        if( !pCSttNd )
        {
            // In order to not move other Redlines' indices, we set them
            // to the end (is exclusive)
            const SwRedlineTable& rTable = rDoc.getIDocumentRedlineAccess().GetRedlineTable();
            for(SwRangeRedline* pRedl : rTable)
            {
                if( pRedl->GetBound() == *pStt )
                    pRedl->GetBound() = *pEnd;
                if( pRedl->GetBound(false) == *pStt )
                    pRedl->GetBound(false) = *pEnd;
            }
        }

        SwStartNode* pSttNd;
        SwNodes& rNds = rDoc.GetNodes();
        if( pCSttNd || pCEndNd )
        {
            SwTextFormatColl* pColl = (pCSttNd && pCSttNd->IsTextNode() )
                                    ? pCSttNd->GetTextNode()->GetTextColl()
                                    : (pCEndNd && pCEndNd->IsTextNode() )
                                        ? pCEndNd->GetTextNode()->GetTextColl()
                                        : rDoc.getIDocumentStylePoolAccess().GetTextCollFromPool(RES_POOLCOLL_STANDARD);

            pSttNd = rNds.MakeTextSection( SwNodeIndex( rNds.GetEndOfRedlines() ),
                                            SwNormalStartNode, pColl );
            SwTextNode* pTextNd = rNds[ pSttNd->GetIndex() + 1 ]->GetTextNode();

            SwNodeIndex aNdIdx( *pTextNd );
            SwPosition aPos( aNdIdx, SwIndex( pTextNd ));
            if( pCSttNd && pCEndNd )
            {
                // tdf#140982 keep annotation ranges in deletions in margin mode
                lcl_storeAnnotationMarks( rDoc, pStt, pEnd );
                rDoc.getIDocumentContentOperations().MoveAndJoin( aPam, aPos );
            }
            else
            {
                if( pCSttNd && !pCEndNd )
                    m_bDelLastPara = true;
                rDoc.getIDocumentContentOperations().MoveRange( aPam, aPos,
                    SwMoveFlags::DEFAULT );
            }
        }
        else
        {
            pSttNd = SwNodes::MakeEmptySection( SwNodeIndex( rNds.GetEndOfRedlines() ) );

            SwPosition aPos( *pSttNd->EndOfSectionNode() );
            rDoc.getIDocumentContentOperations().MoveRange( aPam, aPos,
                SwMoveFlags::DEFAULT );
        }
        m_pContentSect = new SwNodeIndex( *pSttNd );

        if( pStt == GetPoint() )
            Exchange();

        DeleteMark();
    }
    else
        InvalidateRange(Invalidation::Remove);
}

void SwRangeRedline::CopyToSection()
{
    if( m_pContentSect )
        return;

    const SwPosition* pStt = Start(),
                    * pEnd = End();

    SwContentNode* pCSttNd = pStt->nNode.GetNode().GetContentNode();
    SwContentNode* pCEndNd = pEnd->nNode.GetNode().GetContentNode();

    SwStartNode* pSttNd;
    SwDoc& rDoc = GetDoc();
    SwNodes& rNds = rDoc.GetNodes();

    bool bSaveCopyFlag = rDoc.IsCopyIsMove(),
         bSaveRdlMoveFlg = rDoc.getIDocumentRedlineAccess().IsRedlineMove();
    rDoc.SetCopyIsMove( true );

    // The IsRedlineMove() flag causes the behaviour of the
    // DocumentContentOperationsManager::CopyFlyInFlyImpl() method to change,
    // which will eventually be called by the CopyRange() below.
    rDoc.getIDocumentRedlineAccess().SetRedlineMove(true);

    if( pCSttNd )
    {
        SwTextFormatColl* pColl = pCSttNd->IsTextNode()
                                ? pCSttNd->GetTextNode()->GetTextColl()
                                : rDoc.getIDocumentStylePoolAccess().GetTextCollFromPool(RES_POOLCOLL_STANDARD);

        pSttNd = rNds.MakeTextSection( SwNodeIndex( rNds.GetEndOfRedlines() ),
                                        SwNormalStartNode, pColl );

        SwNodeIndex aNdIdx( *pSttNd, 1 );
        SwTextNode* pTextNd = aNdIdx.GetNode().GetTextNode();
        SwPosition aPos( aNdIdx, SwIndex( pTextNd ));

        // tdf#115815 keep original start position of collapsed annotation ranges
        // as temporary bookmarks (removed after file saving and file loading)
        lcl_storeAnnotationMarks( rDoc, pStt, pEnd );
        rDoc.getIDocumentContentOperations().CopyRange(*this, aPos, SwCopyFlags::CheckPosInFly);

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
            rDoc.getIDocumentContentOperations().CopyRange(*this, aPos, SwCopyFlags::CheckPosInFly);
        }
        else
        {
            SwNodeIndex aInsPos( *pSttNd->EndOfSectionNode() );
            SwNodeRange aRg( pStt->nNode, SwNodeOffset(0), pEnd->nNode, SwNodeOffset(1) );
            rDoc.GetDocumentContentOperationsManager().CopyWithFlyInFly(aRg, aInsPos);
        }
    }
    m_pContentSect = new SwNodeIndex( *pSttNd );

    rDoc.SetCopyIsMove( bSaveCopyFlag );
    rDoc.getIDocumentRedlineAccess().SetRedlineMove( bSaveRdlMoveFlg );
}

void SwRangeRedline::DelCopyOfSection(size_t nMyPos)
{
    if( !m_pContentSect )
        return;

    const SwPosition* pStt = Start(),
                    * pEnd = End();

    SwDoc& rDoc = GetDoc();
    SwPaM aPam( *pStt, *pEnd );
    SwContentNode* pCSttNd = pStt->nNode.GetNode().GetContentNode();
    SwContentNode* pCEndNd = pEnd->nNode.GetNode().GetContentNode();

    if( !pCSttNd )
    {
        // In order to not move other Redlines' indices, we set them
        // to the end (is exclusive)
        const SwRedlineTable& rTable = rDoc.getIDocumentRedlineAccess().GetRedlineTable();
        for(SwRangeRedline* pRedl : rTable)
        {
            if( pRedl->GetBound() == *pStt )
                pRedl->GetBound() = *pEnd;
            if( pRedl->GetBound(false) == *pStt )
                pRedl->GetBound(false) = *pEnd;
        }
    }

    if( pCSttNd && pCEndNd )
    {
        // #i100466# - force a <join next> on <delete and join> operation
        // tdf#125319 - rather not?
        rDoc.getIDocumentContentOperations().DeleteAndJoin(aPam/*, true*/);
    }
    else if( pCSttNd || pCEndNd )
    {
        if( pCSttNd && !pCEndNd )
            m_bDelLastPara = true;
        rDoc.getIDocumentContentOperations().DeleteRange( aPam );

        if( m_bDelLastPara )
        {
            // To prevent dangling references to the paragraph to
            // be deleted, redline that point into this paragraph should be
            // moved to the new end position. Since redlines in the redline
            // table are sorted and the pEnd position is an endnode (see
            // bDelLastPara condition above), only redlines before the
            // current ones can be affected.
            const SwRedlineTable& rTable = rDoc.getIDocumentRedlineAccess().GetRedlineTable();
            size_t n = nMyPos;
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

            *GetPoint() = *pEnd;
            *GetMark() = *pEnd;
            DeleteMark();

            aPam.GetBound().nContent.Assign( nullptr, 0 );
            aPam.GetBound( false ).nContent.Assign( nullptr, 0 );
            aPam.DeleteMark();
            rDoc.getIDocumentContentOperations().DelFullPara( aPam );
        }
    }
    else
    {
        rDoc.getIDocumentContentOperations().DeleteRange( aPam );
    }

    if( pStt == GetPoint() )
        Exchange();

    DeleteMark();
}

void SwRangeRedline::MoveFromSection(size_t nMyPos)
{
    if( m_pContentSect )
    {
        SwDoc& rDoc = GetDoc();
        const SwRedlineTable& rTable = rDoc.getIDocumentRedlineAccess().GetRedlineTable();
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

        const SwNode* pKeptContentSectNode( &m_pContentSect->GetNode() ); // #i95711#
        {
            SwPaM aPam( m_pContentSect->GetNode(),
                        *m_pContentSect->GetNode().EndOfSectionNode(), SwNodeOffset(1),
                        SwNodeOffset( m_bDelLastPara ? -2 : -1 ) );
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
            if( m_bDelLastPara && *aPam.GetPoint() == *aPam.GetMark() )
            {
                --aPos.nNode;

                rDoc.getIDocumentContentOperations().AppendTextNode( aPos );
            }
            else
            {
                rDoc.getIDocumentContentOperations().MoveRange( aPam, aPos,
                    SwMoveFlags::ALLFLYS );
            }

            SetMark();
            *GetPoint() = aPos;
            GetMark()->nNode = aNdIdx.GetIndex() + 1;
            pCNd = GetMark()->nNode.GetNode().GetContentNode();
            GetMark()->nContent.Assign( pCNd, nPos );

            if( m_bDelLastPara )
            {
                ++GetPoint()->nNode;
                pCNd = GetContentNode();
                GetPoint()->nContent.Assign( pCNd, 0 );
                m_bDelLastPara = false;
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
        if ( &m_pContentSect->GetNode() == pKeptContentSectNode )
        {
            rDoc.getIDocumentContentOperations().DeleteSection( &m_pContentSect->GetNode() );
        }
        delete m_pContentSect;
        m_pContentSect = nullptr;

        // adjustment of redline table positions must take start and
        // end into account, not point and mark.
        for( auto& pItem : aBeforeArr )
            *pItem = *Start();
        for( auto& pItem : aBehindArr )
            *pItem = *End();
    }
    else
        InvalidateRange(Invalidation::Add);
}

// for Undo
void SwRangeRedline::SetContentIdx( const SwNodeIndex* pIdx )
{
    if( pIdx && !m_pContentSect )
    {
        m_pContentSect = new SwNodeIndex( *pIdx );
        m_bIsVisible = false;
    }
    else if( !pIdx && m_pContentSect )
    {
        delete m_pContentSect;
        m_pContentSect = nullptr;
        m_bIsVisible = false;
    }
    else
    {
        OSL_FAIL("SwRangeRedline::SetContentIdx: invalid state");
    }
}

bool SwRangeRedline::CanCombine( const SwRangeRedline& rRedl ) const
{
    return  IsVisible() && rRedl.IsVisible() &&
            m_pRedlineData->CanCombine( *rRedl.m_pRedlineData );
}

void SwRangeRedline::PushData( const SwRangeRedline& rRedl, bool bOwnAsNext )
{
    SwRedlineData* pNew = new SwRedlineData( *rRedl.m_pRedlineData, false );
    if( bOwnAsNext )
    {
        pNew->m_pNext = m_pRedlineData;
        m_pRedlineData = pNew;
    }
    else
    {
        pNew->m_pNext = m_pRedlineData->m_pNext;
        m_pRedlineData->m_pNext = pNew;
    }
}

bool SwRangeRedline::PopData()
{
    if( !m_pRedlineData->m_pNext )
        return false;
    SwRedlineData* pCur = m_pRedlineData;
    m_pRedlineData = pCur->m_pNext;
    pCur->m_pNext = nullptr;
    delete pCur;
    return true;
}

sal_uInt16 SwRangeRedline::GetStackCount() const
{
    sal_uInt16 nRet = 1;
    for( SwRedlineData* pCur = m_pRedlineData; pCur->m_pNext; pCur = pCur->m_pNext )
        ++nRet;
    return nRet;
}

std::size_t SwRangeRedline::GetAuthor( sal_uInt16 nPos ) const
{
    return GetRedlineData(nPos).m_nAuthor;
}

OUString const & SwRangeRedline::GetAuthorString( sal_uInt16 nPos ) const
{
    return SW_MOD()->GetRedlineAuthor(GetRedlineData(nPos).m_nAuthor);
}

const DateTime& SwRangeRedline::GetTimeStamp( sal_uInt16 nPos ) const
{
    return GetRedlineData(nPos).m_aStamp;
}

RedlineType SwRangeRedline::GetType( sal_uInt16 nPos ) const
{
    return GetRedlineData(nPos).m_eType;
}

const OUString& SwRangeRedline::GetComment( sal_uInt16 nPos ) const
{
    return GetRedlineData(nPos).m_sComment;
}

bool SwRangeRedline::operator<( const SwRangeRedline& rCmp ) const
{
    if (*Start() < *rCmp.Start())
        return true;

    return *Start() == *rCmp.Start() && *End() < *rCmp.End();
}

const SwRedlineData & SwRangeRedline::GetRedlineData(const sal_uInt16 nPos) const
{
    SwRedlineData * pCur = m_pRedlineData;

    sal_uInt16 nP = nPos;

    while (nP > 0 && nullptr != pCur->m_pNext)
    {
        pCur = pCur->m_pNext;

        nP--;
    }

    SAL_WARN_IF( nP != 0, "sw.core", "Pos " << nPos << " is " << nP << " too big");

    return *pCur;
}

OUString SwRangeRedline::GetDescr(bool bSimplified)
{
    // get description of redline data (e.g.: "insert $1")
    OUString aResult = GetRedlineData().GetDescr();

    SwPaM * pPaM = nullptr;
    bool bDeletePaM = false;

    // if this redline is visible the content is in this PaM
    if (nullptr == m_pContentSect)
    {
        pPaM = this;
    }
    else // otherwise it is saved in pContentSect
    {
        SwNodeIndex aTmpIdx( *m_pContentSect->GetNode().EndOfSectionNode() );
        pPaM = new SwPaM(*m_pContentSect, aTmpIdx );
        bDeletePaM = true;
    }

    OUString sDescr = DenoteSpecialCharacters(pPaM->GetText().replace('\n', ' '), /*bQuoted=*/!bSimplified);
    if (const SwTextNode *pTextNode = pPaM->GetNode().GetTextNode())
    {
        if (const SwTextAttr* pTextAttr = pTextNode->GetFieldTextAttrAt(pPaM->GetPoint()->nContent.GetIndex() - 1, true ))
        {
            sDescr = ( bSimplified ? "" : SwResId(STR_START_QUOTE) )
                + pTextAttr->GetFormatField().GetField()->GetFieldName()
                + ( bSimplified ? "" : SwResId(STR_END_QUOTE) );
        }
    }

    // replace $1 in description by description of the redlines text
    const OUString aTmpStr = ShortenString(sDescr, nUndoStringLength, SwResId(STR_LDOTS));

    if (!bSimplified)
    {
        SwRewriter aRewriter;
        aRewriter.AddRule(UndoArg1, aTmpStr);

        aResult = aRewriter.Apply(aResult);
    }
    else
    {
        aResult = aTmpStr;
        // more shortening
        sal_Int32 nPos = aTmpStr.indexOf(SwResId(STR_LDOTS));
        if (nPos > 5)
            aResult = aTmpStr.copy(0, nPos + SwResId(STR_LDOTS).getLength());
    }

    if (bDeletePaM)
        delete pPaM;

    return aResult;
}

void SwRangeRedline::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("SwRangeRedline"));

    (void)xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("ptr"), "%p", this);
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("id"), BAD_CAST(OString::number(GetSeqNo()).getStr()));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("author"), BAD_CAST(SW_MOD()->GetRedlineAuthor(GetAuthor()).toUtf8().getStr()));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("date"), BAD_CAST(DateTimeToOString(GetTimeStamp()).getStr()));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("descr"), BAD_CAST(const_cast<SwRangeRedline*>(this)->GetDescr().toUtf8().getStr()));

    OString sRedlineType;
    switch (GetType())
    {
        case RedlineType::Insert:
            sRedlineType = "REDLINE_INSERT";
            break;
        case RedlineType::Delete:
            sRedlineType = "REDLINE_DELETE";
            break;
        case RedlineType::Format:
            sRedlineType = "REDLINE_FORMAT";
            break;
        default:
            sRedlineType = "UNKNOWN";
            break;
    }
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("type"), BAD_CAST(sRedlineType.getStr()));

    SwPaM::dumpAsXml(pWriter);

    (void)xmlTextWriterEndElement(pWriter);
}

void SwExtraRedlineTable::Insert( SwExtraRedline* p )
{
    m_aExtraRedlines.push_back( p );
    //p->CallDisplayFunc();
}

void SwExtraRedlineTable::DeleteAndDestroy(sal_uInt16 const nPos)
{
    /*
    SwDoc* pDoc = 0;
    if( !nP && nL && nL == size() )
        pDoc = front()->GetDoc();
    */

    delete m_aExtraRedlines[nPos];
    m_aExtraRedlines.erase(m_aExtraRedlines.begin() + nPos);

    /*
    SwViewShell* pSh;
    if( pDoc && !pDoc->IsInDtor() &&
        0 != ( pSh = pDoc->getIDocumentLayoutAccess().GetCurrentViewShell() ) )
        pSh->InvalidateWindows( SwRect( 0, 0, SAL_MAX_INT32, SAL_MAX_INT32 ) );
    */
}

void SwExtraRedlineTable::DeleteAndDestroyAll()
{
    while (!m_aExtraRedlines.empty())
    {
        auto const pRedline = m_aExtraRedlines.back();
        m_aExtraRedlines.pop_back();
        delete pRedline;
    }
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
