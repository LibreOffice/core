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
#include <editeng/prntitem.hxx>
#include <comphelper/lok.hxx>
#include <comphelper/string.hxx>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <unotools/datetime.hxx>
#include <sfx2/viewsh.hxx>
#include <o3tl/string_view.hxx>
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
#include <algorithm>
#include <limits>
#include <utility>
#include <view.hxx>
#include <viewopt.hxx>
#include <usrpref.hxx>
#include <viewsh.hxx>
#include <viscrs.hxx>
#include <rootfrm.hxx>
#include <strings.hrc>
#include <swtypes.hxx>
#include <wrtsh.hxx>
#include <txtfld.hxx>

#include <flowfrm.hxx>
#include <txtfrm.hxx>

using namespace com::sun::star;

#ifdef DBG_UTIL

    void sw_DebugRedline( const SwDoc* pDoc )
    {
        static SwRedlineTable::size_type nWatch = 0; // loplugin:constvars:ignore
        const SwRedlineTable& rTable = pDoc->getIDocumentRedlineAccess().GetRedlineTable();
        for( SwRedlineTable::size_type n = 0; n < rTable.size(); ++n )
        {
            volatile SwRedlineTable::size_type nDummy = 0;
            const SwRangeRedline* pCurrent = rTable[ n ];
            const SwRangeRedline* pNext = n+1 < rTable.size() ? rTable[ n+1 ] : nullptr;
            if( pCurrent == pNext )
                (void) nDummy;
            if( n == nWatch )
                (void) nDummy; // Possible debugger breakpoint
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
    SwNode* pSttNode = &pStt->GetNode();
    SwNode* pEndNode = &pEnd->GetNode();
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
        if (!pTableRowRedline)
            continue;
        const SwTableLine& rRedTabLine = pTableRowRedline->GetTableLine();
        if ( &rRedTabLine == &rTableLine )
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
        if (!pTableCellRedline)
            continue;
        const SwTableBox& rRedTabBox = pTableCellRedline->GetTableBox();
        if (&rRedTabBox == &rTableBox)
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

                // Also empty the text portion cache, so it gets rebuilt, taking the new redlines
                // into account.
                if (pTmpFrame->IsTextFrame())
                {
                    auto pTextFrame = static_cast<SwTextFrame*>(pTmpFrame);
                    pTextFrame->ClearPara();
                }
            }
        }
    }
}

void lcl_LOKInvalidateStartEndFrames(SwShellCursor& rCursor)
{
    if (!(rCursor.HasMark() &&
        rCursor.GetPoint()->GetNode().IsContentNode() &&
        rCursor.GetPoint()->GetNode().GetContentNode()->getLayoutFrame(rCursor.GetShell()->GetLayout()) &&
        (rCursor.GetMark()->GetNode() == rCursor.GetPoint()->GetNode() ||
        (rCursor.GetMark()->GetNode().IsContentNode() &&
         rCursor.GetMark()->GetNode().GetContentNode()->getLayoutFrame(rCursor.GetShell()->GetLayout())))))
    {
        return;
    }

    auto [pStartPos, pEndPos] = rCursor.StartEnd(); // SwPosition*

    lcl_LOKInvalidateFrames(*(pStartPos->GetNode().GetContentNode()),
                            rCursor.GetShell()->GetLayout(),
                            FRM_CNTNT, &rCursor.GetSttPos());

    lcl_LOKInvalidateFrames(*(pEndPos->GetNode().GetContentNode()),
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

void SwRedlineTable::setMovedIDIfNeeded(sal_uInt32 nMax)
{
    if (nMax > m_nMaxMovedID)
        m_nMaxMovedID = nMax;
}

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

    auto [pStartPos, pEndPos] = pRedline->StartEnd(); // SwPosition*
    SwContentNode* pContentNd = pRedline->GetPointContentNode();
    SwView* pView = dynamic_cast<SwView*>(SfxViewShell::Current());
    if (pView && pContentNd)
    {
        SwShellCursor aCursor(pView->GetWrtShell(), *pStartPos);
        aCursor.SetMark();
        *aCursor.GetMark() = *pEndPos;

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
                for(SwNodeIndex nIdx(pStartPos->GetNode()); nIdx <= pEndPos->GetNode(); ++nIdx)
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
            pViewShell->libreOfficeKitViewCallback(nType == RedlineNotification::Modify ? LOK_CALLBACK_REDLINE_TABLE_ENTRY_MODIFIED : LOK_CALLBACK_REDLINE_TABLE_SIZE_CHANGED, OString(aPayload));
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

        // detect text moving by checking nearby redlines, except during Undo
        // (apply isMoved() during OpenDocument and DOCX import, too, to fix
        // missing text moving handling in ODF and e.g. web version of MSO)
        if ( p->GetDoc().GetIDocumentUndoRedo().DoesUndo() ||
             p->GetDoc().IsInWriterfilterImport() ||
             p->GetDoc().IsInXMLImport() )
        {
            isMoved(nP);
        }

        p->CallDisplayFunc(nP);
        if (rv.second)
        {
            CheckOverlapping(rv.first);
            if (!mpMaxEndPos || (*(*rv.first)->End()) > *mpMaxEndPos->End())
                mpMaxEndPos = *rv.first;
        }
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
        if (pCurr->End()->GetNodeIndex() >= pNext->Start()->GetNodeIndex())
        {
            m_bHasOverlappingElements = true;
            return;
        }
    }
    if (it != maVector.begin())
    {
        auto pPrev = *(it - 1);
        if (pPrev->End()->GetNodeIndex() >= pCurr->Start()->GetNodeIndex())
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
        {
            CheckOverlapping(rv.first);
            if (!mpMaxEndPos || (*(*rv.first)->End()) > *mpMaxEndPos->End())
                mpMaxEndPos = *rv.first;
        }
        return rv.second;
    }
    return InsertWithValidRanges( p, &rP );
}

namespace sw {

std::vector<std::unique_ptr<SwRangeRedline>> GetAllValidRanges(std::unique_ptr<SwRangeRedline> p)
{
    std::vector<std::unique_ptr<SwRangeRedline>> ret;
    // Create valid "sub-ranges" from the Selection
    auto [pStt, pEnd] = p->StartEnd(); // SwPosition*
    SwPosition aNewStt( *pStt );
    SwNodes& rNds = aNewStt.GetNodes();
    SwContentNode* pC;

    if( !aNewStt.GetNode().IsContentNode() )
    {
        pC = SwNodes::GoNext(&aNewStt);
        if( !pC )
            aNewStt.Assign(rNds.GetEndOfContent());
    }


    if( aNewStt >= *pEnd )
        return ret;

    std::unique_ptr<SwRangeRedline> pNew;
    do {
        if( !pNew )
            pNew.reset(new SwRangeRedline( p->GetRedlineData(), aNewStt ));
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
        SwNode* pTab = pNew->GetPoint()->GetNode().StartOfSectionNode()->FindTableNode();
        // We end in a table when pTab != 0
        if( pTab && !pNew->GetMark()->GetNode().StartOfSectionNode()->FindTableNode() )
        { // but our Mark was outside the table => Correction
            do
            {
                // We want to be before the table
                pNew->GetPoint()->Assign(*pTab);
                pC = GoPreviousPos( pNew->GetPoint(), false ); // here we are.
                if( pC )
                    pNew->GetPoint()->SetContent( 0 );
                pTab = pNew->GetPoint()->GetNode().StartOfSectionNode()->FindTableNode();
            } while( pTab ); // If there is another table we have to repeat our step backwards
        }

        // insert dummy character to the empty table rows to keep their changes
        SwNode& rBoxNode = pNew->GetMark()->GetNode();
        if ( rBoxNode.GetDoc().GetIDocumentUndoRedo().DoesUndo() && rBoxNode.GetTableBox() &&
             rBoxNode.GetTableBox()->GetUpper()->IsEmpty() && rBoxNode.GetTextNode() )
        {
            ::sw::UndoGuard const undoGuard(rBoxNode.GetDoc().GetIDocumentUndoRedo());
            rBoxNode.GetTextNode()->InsertDummy();
            pNew->GetMark()->SetContent( 1 );
        }

        if( *pNew->GetPoint() > *pEnd )
        {
            pC = nullptr;
            if( aNewStt.GetNode() != pEnd->GetNode() )
                do {
                    SwNode& rCurNd = aNewStt.GetNode();
                    if( rCurNd.IsStartNode() )
                    {
                        if( rCurNd.EndOfSectionIndex() < pEnd->GetNodeIndex() )
                            aNewStt.Assign( *rCurNd.EndOfSectionNode() );
                        else
                            break;
                    }
                    else if( rCurNd.IsContentNode() )
                        pC = rCurNd.GetContentNode();
                    aNewStt.Adjust(SwNodeOffset(1));
                } while( aNewStt.GetNodeIndex() < pEnd->GetNodeIndex() );

            if( aNewStt.GetNode() == pEnd->GetNode() )
                aNewStt.SetContent(pEnd->GetContentIndex());
            else if( pC )
            {
                aNewStt.Assign(*pC, pC->Len() );
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
            ret.push_back(std::move(pNew));
        }

        if( aNewStt >= *pEnd )
            break;
        pC = SwNodes::GoNext(&aNewStt);
        if( !pC )
            break;
    } while( aNewStt < *pEnd );

    return ret;
}

} // namespace sw

static void lcl_setRowNotTracked(SwNode& rNode)
{
    SwDoc& rDoc = rNode.GetDoc();
    if ( rDoc.GetIDocumentUndoRedo().DoesUndo() && rNode.GetTableBox() )
    {
        SvxPrintItem aSetTracking(RES_PRINT, false);
        SwNodeIndex aInsPos( *(rNode.GetTableBox()->GetSttNd()), 1);
        SwCursor aCursor( SwPosition(aInsPos), nullptr );
        ::sw::UndoGuard const undoGuard(rNode.GetDoc().GetIDocumentUndoRedo());
        rDoc.SetRowNotTracked( aCursor, aSetTracking );
    }
}

bool SwRedlineTable::InsertWithValidRanges(SwRangeRedline*& p, size_type* pInsPos)
{
    bool bAnyIns = false;
    bool bInsert = RedlineType::Insert == p->GetType();
    SwNode* pSttNode = &p->Start()->GetNode();

    std::vector<std::unique_ptr<SwRangeRedline>> redlines(
            GetAllValidRanges(std::unique_ptr<SwRangeRedline>(p)));

    // tdf#147180 set table change tracking in the empty row with text insertion
    if ( bInsert )
        lcl_setRowNotTracked(*pSttNode);

    for (std::unique_ptr<SwRangeRedline> & pRedline : redlines)
    {
        assert(pRedline->HasValidRange());
        size_type nInsPos;
        auto pTmpRedline = pRedline.release();
        if (Insert(pTmpRedline, nInsPos))
        {
            // tdf#147180 set table tracking to the table row
            lcl_setRowNotTracked(pTmpRedline->GetPointNode());

            pTmpRedline->CallDisplayFunc(nInsPos);
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

bool CompareSwRedlineTable::operator()(const SwRangeRedline* lhs, const SwRangeRedline* rhs) const
{
    return *lhs < *rhs;
}

SwRedlineTable::~SwRedlineTable()
{
   maVector.DeleteAndDestroyAll();
}

SwRedlineTable::size_type SwRedlineTable::GetPos(const SwRangeRedline* p) const
{
    vector_type::const_iterator it = maVector.find(p);
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

    if (mpMaxEndPos == maVector[nP])
        mpMaxEndPos = nullptr;
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
    mpMaxEndPos = nullptr;
}

void SwRedlineTable::DeleteAndDestroy(size_type const nP)
{
    auto const pRedline = maVector[nP];
    maVector.erase(maVector.begin() + nP);
    LOKRedlineNotification(RedlineNotification::Remove, pRedline);
    delete pRedline;
    if (pRedline == mpMaxEndPos)
        mpMaxEndPos = nullptr;
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
            auto [pRStt, pREnd] = pTmp->StartEnd(); // SwPosition*
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

namespace
{
bool lcl_CanCombineWithRange(SwRangeRedline* pOrigin, SwRangeRedline* pActual,
                             SwRangeRedline* pOther, bool bReverseDir, bool bCheckChilds)
{
    if (pOrigin->IsVisible() != pOther->IsVisible())
        return false;

    if (bReverseDir)
    {
        if (*(pOther->End()) != *(pActual->Start()))
            return false;
    }
    else
    {
        if (*(pActual->End()) != *(pOther->Start()))
            return false;
    }

    if (!pOrigin->GetRedlineData(0).CanCombineForAcceptReject(pOther->GetRedlineData(0)))
    {
        if (!bCheckChilds || pOther->GetStackCount() <= 1
            || !pOrigin->GetRedlineData(0).CanCombineForAcceptReject(pOther->GetRedlineData(1)))
            return false;
    }
    if (pOther->Start()->GetNode().StartOfSectionNode()
        != pActual->Start()->GetNode().StartOfSectionNode())
        return false;

    return true;
}
}

const SwPosition& SwRedlineTable::GetMaxEndPos() const
{
    assert(!empty() && "cannot call this when the redline table is empty");
    if (mpMaxEndPos)
        return *mpMaxEndPos->End();
    for (const SwRangeRedline* i : maVector)
    {
        if (!mpMaxEndPos || *i->End() > *mpMaxEndPos->End())
            mpMaxEndPos = i;
    }
    assert(mpMaxEndPos);
    return *mpMaxEndPos->End();
}

void SwRedlineTable::getConnectedArea(size_type nPosOrigin, size_type& rPosStart,
                                      size_type& rPosEnd, bool bCheckChilds) const
{
    // Keep the original redline .. else we should memorize which children was checked
    // at the last combined redline.
    SwRangeRedline* pOrigin = (*this)[nPosOrigin];
    rPosStart = nPosOrigin;
    rPosEnd = nPosOrigin;
    SwRangeRedline* pRedline = pOrigin;
    SwRangeRedline* pOther;

    // connection info is already here..only the actual text is missing at import time
    // so no need to check Redline->GetContentIdx() here yet.
    while (rPosStart > 0 && (pOther = (*this)[rPosStart - 1])
           && lcl_CanCombineWithRange(pOrigin, pRedline, pOther, true, bCheckChilds))
    {
        rPosStart--;
        pRedline = pOther;
    }
    pRedline = pOrigin;
    while (rPosEnd + 1 < size() && (pOther = (*this)[rPosEnd + 1])
           && lcl_CanCombineWithRange(pOrigin, pRedline, pOther, false, bCheckChilds))
    {
        rPosEnd++;
        pRedline = pOther;
    }
}

OUString SwRedlineTable::getTextOfArea(size_type rPosStart, size_type rPosEnd) const
{
    // Normally a SwPaM::GetText() would be enough with rPosStart-start and rPosEnd-end
    // But at import time some text is not present there yet
    // we have to collect them 1 by 1

    OUString sRet = "";

    for (size_type nIdx = rPosStart; nIdx <= rPosEnd; ++nIdx)
    {
        SwRangeRedline* pRedline = (*this)[nIdx];
        bool bStartWithNonTextNode = false;

        OUString sNew;
        if (nullptr == pRedline->GetContentIdx())
        {
            sNew = pRedline->GetText();
        }
        else // otherwise it is saved in pContentSect, e.g. during ODT import
        {
            SwPaM aTmpPaM(pRedline->GetContentIdx()->GetNode(),
                              *pRedline->GetContentIdx()->GetNode().EndOfSectionNode());
            if (!aTmpPaM.Start()->nNode.GetNode().GetTextNode())
            {
                bStartWithNonTextNode = true;
            }
            sNew = aTmpPaM.GetText();
        }

        if (bStartWithNonTextNode &&
            sNew[0] == CH_TXTATR_NEWLINE)
        {
            sRet += sNew.subView(1);
        }
        else
            sRet += sNew;
    }

    return sRet;
}

bool SwRedlineTable::isMoved(size_type rPos) const
{
    // If it is already a part of a movement, then don't check it.
    if ((*this)[rPos]->GetMoved() != 0)
        return false;
    // First try with single redline. then try with combined redlines
    if (isMovedImpl(rPos, false))
        return true;
    else
        return isMovedImpl(rPos, true);
}

bool SwRedlineTable::isMovedImpl(size_type rPos, bool bTryCombined) const
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

    OUString sTrimmed;
    SwRedlineTable::size_type nPosStart = rPos;
    SwRedlineTable::size_type nPosEnd = rPos;

    if (bTryCombined)
    {
        getConnectedArea(rPos, nPosStart, nPosEnd, false);
        if (nPosStart != nPosEnd)
            sTrimmed = getTextOfArea(nPosStart, nPosEnd).trim();
    }

    if (sTrimmed.isEmpty())
    {
        // if this redline is visible the content is in this PaM
        if (nullptr == pRedline->GetContentIdx())
        {
            sTrimmed = pRedline->GetText().trim();
        }
        else // otherwise it is saved in pContentSect, e.g. during ODT import
        {
            SwPaM aTmpPaM(pRedline->GetContentIdx()->GetNode(),
                             *pRedline->GetContentIdx()->GetNode().EndOfSectionNode());
            sTrimmed = aTmpPaM.GetText().trim();
        }
    }

    // detection of move needs at least 6 characters with an inner
    // space after stripping white spaces of the redline to skip
    // frequent deleted and inserted articles or other common
    // word parts, e.g. 'the' and 'of a' to detect as text moving
    if (sTrimmed.getLength() < 6 || sTrimmed.indexOf(' ') == -1)
    {
        return false;
    }

    // Todo: lessen the previous condition..:
    // if the source / destination is a whole node change then maybe space is not needed

    // search pair around the actual redline
    size_type nEnd = rPos + nLookahead < size()
        ? rPos + nLookahead
        : size();
    size_type nStart = rPos > nLookahead ? rPos - nLookahead : 0;
    // first, try to compare to single redlines
    // next, try to compare to combined redlines
    for (int nPass = 0; nPass < 2 && !bRet; nPass++)
    {
        for (size_type nPosAct = nStart; nPosAct < nEnd && !bRet; ++nPosAct)
        {
            SwRangeRedline* pPair = (*this)[nPosAct];

            // redline must be the requested type and from the same author
            if (nPairType != pPair->GetType() || pRedline->GetAuthor() != pPair->GetAuthor())
            {
                continue;
            }

            OUString sPairTrimmed = "";
            SwRedlineTable::size_type nPairStart = nPosAct;
            SwRedlineTable::size_type nPairEnd = nPosAct;

            if (nPass == 0)
            {
                // if this redline is visible the content is in this PaM
                if (nullptr == pPair->GetContentIdx())
                {
                    sPairTrimmed = o3tl::trim(pPair->GetText());
                }
                else // otherwise it is saved in pContentSect, e.g. during ODT import
                {
                    // saved in pContentSect, e.g. during ODT import
                    SwPaM aPairPaM(pPair->GetContentIdx()->GetNode(),
                                         *pPair->GetContentIdx()->GetNode().EndOfSectionNode());
                    sPairTrimmed = o3tl::trim(aPairPaM.GetText());
                }
            }
            else
            {
                getConnectedArea(nPosAct, nPairStart, nPairEnd, false);
                if (nPairStart != nPairEnd)
                    sPairTrimmed = getTextOfArea(nPairStart, nPairEnd).trim();
            }

            // pair at tracked moving: same text by trimming trailing white spaces
            if (abs(sTrimmed.getLength() - sPairTrimmed.getLength()) <= 2
                && sTrimmed == sPairTrimmed)
            {
                sal_uInt32 nMID = getNewMovedID();
                if (nPosStart != nPosEnd)
                {
                    for (size_type nIdx = nPosStart; nIdx <= nPosEnd; ++nIdx)
                    {
                        (*this)[nIdx]->SetMoved(nMID);
                        if (nIdx != rPos)
                            (*this)[nIdx]->InvalidateRange(SwRangeRedline::Invalidation::Add);
                    }
                }
                else
                    pRedline->SetMoved(nMID);

                //in (nPass == 0) it will only call once .. as nPairStart == nPairEnd == nPosAct
                for (size_type nIdx = nPairStart; nIdx <= nPairEnd; ++nIdx)
                {
                    (*this)[nIdx]->SetMoved(nMID);
                    (*this)[nIdx]->InvalidateRange(SwRangeRedline::Invalidation::Add);
                }

                bRet = true;
            }

            //we can skip the combined redlines
            if (nPass == 1)
                nPosAct = nPairEnd;
        }
    }

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

SwRedlineExtraData_FormatColl::SwRedlineExtraData_FormatColl( OUString aColl,
                                                sal_uInt16 nPoolFormatId,
                                                const SfxItemSet* pItemSet,
                                                bool bFormatAll )
    : m_sFormatNm(std::move(aColl)), m_nPoolId(nPoolFormatId), m_bFormatAll(bFormatAll)
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

    if ( !m_bFormatAll || pEnd->GetContentIndex() == 0 )
    {
        // don't reject the format of the next paragraph (that is handled by the next redline)
        if (aPam.GetPoint()->GetNode() > aPam.GetMark()->GetNode())
        {
            aPam.GetPoint()->Adjust(SwNodeOffset(-1));
            SwContentNode* pNode = aPam.GetPoint()->GetNode().GetContentNode();
            if ( pNode )
                aPam.GetPoint()->SetContent( pNode->Len() );
            else
                // tdf#147507 set it back to a content node to avoid of crashing
                aPam.GetPoint()->Adjust(SwNodeOffset(+1));
        }
        else if (aPam.GetPoint()->GetNode() < aPam.GetMark()->GetNode())
        {
            aPam.GetMark()->Adjust(SwNodeOffset(-1));
            SwContentNode* pNode = aPam.GetMark()->GetNode().GetContentNode();
            aPam.GetMark()->SetContent( pNode->Len() );
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

SwRedlineData::SwRedlineData( RedlineType eT, std::size_t nAut, sal_uInt32 nMovedID )
    : m_pNext( nullptr ), m_pExtraData( nullptr ),
    m_aStamp( DateTime::SYSTEM ),
    m_nAuthor( nAut ), m_eType( eT ), m_nSeqNo( 0 ), m_bAutoFormat(false), m_nMovedID(nMovedID)
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
    , m_nMovedID( rCpy.m_nMovedID )
{
}

// For sw3io: We now own pNext!
SwRedlineData::SwRedlineData(RedlineType eT, std::size_t nAut, const DateTime& rDT,
    sal_uInt32 nMovedID, OUString aCmnt, SwRedlineData *pNxt)
    : m_pNext(pNxt), m_pExtraData(nullptr), m_sComment(std::move(aCmnt)), m_aStamp(rDT),
    m_nAuthor(nAut), m_eType(eT), m_nSeqNo(0), m_bAutoFormat(false), m_nMovedID(nMovedID)
{
}

SwRedlineData::~SwRedlineData()
{
    delete m_pExtraData;
    delete m_pNext;
}

// Check whether the absolute difference between the two dates is no larger than one minute (can
// give inaccurate results if at least one of the dates is not valid/normalized):
static bool deltaOneMinute(DateTime const & t1, DateTime const & t2) {
    auto const & [min, max] = std::minmax(t1, t2);
    // Avoid overflow of `min + tools::Time(0, 1)` below when min is close to the maximum valid
    // DateTime:
    if (min >= DateTime({31, 12, std::numeric_limits<sal_Int16>::max()}, {23, 59})) {
        return true;
    }
    return max <= min + tools::Time(0, 1);
}

bool SwRedlineData::CanCombine(const SwRedlineData& rCmp) const
{
    return m_nAuthor == rCmp.m_nAuthor &&
            m_eType == rCmp.m_eType &&
            m_sComment == rCmp.m_sComment &&
            deltaOneMinute(GetTimeStamp(), rCmp.GetTimeStamp()) &&
            m_nMovedID == rCmp.m_nMovedID &&
            (( !m_pNext && !rCmp.m_pNext ) ||
                ( m_pNext && rCmp.m_pNext &&
                m_pNext->CanCombine( *rCmp.m_pNext ))) &&
            (( !m_pExtraData && !rCmp.m_pExtraData ) ||
                ( m_pExtraData && rCmp.m_pExtraData &&
                    *m_pExtraData == *rCmp.m_pExtraData ));
}

// Check if we could/should accept/reject the 2 redlineData at the same time.
// No need to check its children equality
bool SwRedlineData::CanCombineForAcceptReject(const SwRedlineData& rCmp) const
{
    return m_nAuthor == rCmp.m_nAuthor &&
            m_eType == rCmp.m_eType &&
            m_sComment == rCmp.m_sComment &&
            deltaOneMinute(GetTimeStamp(), rCmp.GetTimeStamp()) &&
            m_nMovedID == rCmp.m_nMovedID &&
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

void SwRedlineData::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("SwRedlineData"));

    (void)xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("ptr"), "%p", this);
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("id"), BAD_CAST(OString::number(GetSeqNo()).getStr()));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("author"), BAD_CAST(SW_MOD()->GetRedlineAuthor(GetAuthor()).toUtf8().getStr()));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("date"), BAD_CAST(DateTimeToOString(GetTimeStamp()).getStr()));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("descr"), BAD_CAST(GetDescr().toUtf8().getStr()));

    OString sRedlineType;
    switch (GetType())
    {
        case RedlineType::Insert:
            sRedlineType = "REDLINE_INSERT"_ostr;
            break;
        case RedlineType::Delete:
            sRedlineType = "REDLINE_DELETE"_ostr;
            break;
        case RedlineType::Format:
            sRedlineType = "REDLINE_FORMAT"_ostr;
            break;
        default:
            sRedlineType = "UNKNOWN"_ostr;
            break;
    }
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("type"), BAD_CAST(sRedlineType.getStr()));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("moved"), BAD_CAST(OString::number(m_nMovedID).getStr()));

    (void)xmlTextWriterEndElement(pWriter);
}

sal_uInt32 SwRangeRedline::s_nLastId = 1;

namespace
{
void lcl_LOKBroadcastCommentOperation(RedlineType type, const SwPaM& rPam)
{
    if (comphelper::LibreOfficeKit::isActive())
    {
        auto eHintType = RedlineType::Delete == type ? SwFormatFieldHintWhich::REDLINED_DELETION: SwFormatFieldHintWhich::INSERTED;
        const SwTextNode *pTextNode = rPam.GetPointNode().GetTextNode();
        SwTextAttr* pTextAttr = pTextNode ? pTextNode->GetFieldTextAttrAt(rPam.GetPoint()->GetContentIndex() - 1, ::sw::GetTextAttrMode::Default) : nullptr;
        SwTextField *const pTextField(static_txtattr_cast<SwTextField*>(pTextAttr));
        if (pTextField)
            const_cast<SwFormatField&>(pTextField->GetFormatField()).Broadcast(SwFormatFieldHint(&pTextField->GetFormatField(), eHintType));
    }
}
} // anonymous namespace

SwRangeRedline::SwRangeRedline(RedlineType eTyp, const SwPaM& rPam, sal_uInt32 nMovedID )
    : SwPaM( *rPam.GetMark(), *rPam.GetPoint() ), m_pRedlineData(
          new SwRedlineData(eTyp, GetDoc().getIDocumentRedlineAccess().GetRedlineAuthor(), nMovedID ) )
    ,
    m_nId( s_nLastId++ )
{
    GetBound().SetRedline(this);
    GetBound(false).SetRedline(this);

    m_bDelLastPara = false;
    m_bIsVisible = true;
    if( !rPam.HasMark() )
        DeleteMark();

    // set default comment for single annotations added or deleted
    if ( IsAnnotation() )
    {
        SetComment( RedlineType::Delete == eTyp
            ? SwResId(STR_REDLINE_COMMENT_DELETED)
            : SwResId(STR_REDLINE_COMMENT_ADDED) );

        lcl_LOKBroadcastCommentOperation(eTyp, rPam);
    }
}

SwRangeRedline::SwRangeRedline( const SwRedlineData& rData, const SwPaM& rPam )
    : SwPaM( *rPam.GetMark(), *rPam.GetPoint() ),
    m_pRedlineData( new SwRedlineData( rData )),
    m_nId( s_nLastId++ )
{
    GetBound().SetRedline(this);
    GetBound(false).SetRedline(this);

    m_bDelLastPara = false;
    m_bIsVisible = true;
    if( !rPam.HasMark() )
        DeleteMark();

    // set default comment for single annotations added or deleted
    if ( IsAnnotation() )
    {
        SetComment( RedlineType::Delete == rData.m_eType
            ? SwResId(STR_REDLINE_COMMENT_DELETED)
            : SwResId(STR_REDLINE_COMMENT_ADDED) );

        lcl_LOKBroadcastCommentOperation(rData.m_eType, rPam);
    }
}

SwRangeRedline::SwRangeRedline( const SwRedlineData& rData, const SwPosition& rPos )
    : SwPaM( rPos ),
    m_pRedlineData( new SwRedlineData( rData )),
    m_nId( s_nLastId++ )
{
    GetBound().SetRedline(this);
    GetBound(false).SetRedline(this);

    m_bDelLastPara = false;
    m_bIsVisible = true;
}

SwRangeRedline::SwRangeRedline( const SwRangeRedline& rCpy )
    : SwPaM( *rCpy.GetMark(), *rCpy.GetPoint() ),
    m_pRedlineData( new SwRedlineData( *rCpy.m_pRedlineData )),
    m_nId( s_nLastId++ )
{
    GetBound().SetRedline(this);
    GetBound(false).SetRedline(this);

    m_bDelLastPara = false;
    m_bIsVisible = true;
    if( !rCpy.HasMark() )
        DeleteMark();
}

SwRangeRedline::~SwRangeRedline()
{
    if( m_oContentSect )
    {
        // delete the ContentSection
        if( !GetDoc().IsInDtor() )
            GetDoc().getIDocumentContentOperations().DeleteSection( &m_oContentSect->GetNode() );
        m_oContentSect.reset();
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
    const SwNode* pPtNd = &GetPoint()->GetNode(),
                * pMkNd = &GetMark()->GetNode();
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
    auto [pRStt, pREnd] = StartEnd(); // SwPosition*
    SwNodeOffset nSttNd = pRStt->GetNodeIndex(),
                 nEndNd = pREnd->GetNodeIndex();
    sal_Int32 nSttCnt = pRStt->GetContentIndex();
    sal_Int32 nEndCnt = pREnd->GetContentIndex();

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

                if (comphelper::LibreOfficeKit::isActive() && IsAnnotation())
                {
                    auto eHintType = eWhy == Invalidation::Add ? SwFormatFieldHintWhich::INSERTED: SwFormatFieldHintWhich::REMOVED;
                    const SwTextNode *pTextNode = this->GetPointNode().GetTextNode();
                    SwTextAttr* pTextAttr = pTextNode ? pTextNode->GetFieldTextAttrAt(this->GetPoint()->GetContentIndex() - 1, ::sw::GetTextAttrMode::Default) : nullptr;
                    SwTextField *const pTextField(static_txtattr_cast<SwTextField*>(pTextAttr));
                    if (pTextField)
                        const_cast<SwFormatField&>(pTextField->GetFormatField()).Broadcast(SwFormatFieldHint(&pTextField->GetFormatField(), eHintType));
                }
            }
        }
    }
}

/** Calculates the start and end position of the intersection rTmp and
    text node nNdIdx */
void SwRangeRedline::CalcStartEnd( SwNodeOffset nNdIdx, sal_Int32& rStart, sal_Int32& rEnd ) const
{
    auto [pRStt, pREnd] = StartEnd(); // SwPosition*
    if( pRStt->GetNodeIndex() < nNdIdx )
    {
        if( pREnd->GetNodeIndex() > nNdIdx )
        {
            rStart = 0;             // Paragraph is completely enclosed
            rEnd = COMPLETE_STRING;
        }
        else if (pREnd->GetNodeIndex() == nNdIdx)
        {
            rStart = 0;             // Paragraph is overlapped in the beginning
            rEnd = pREnd->GetContentIndex();
        }
        else // redline ends before paragraph
        {
            rStart = COMPLETE_STRING;
            rEnd = COMPLETE_STRING;
        }
    }
    else if( pRStt->GetNodeIndex() == nNdIdx )
    {
        rStart = pRStt->GetContentIndex();
        if( pREnd->GetNodeIndex() == nNdIdx )
            rEnd = pREnd->GetContentIndex(); // Within the Paragraph
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
                SwPaM aPam( rStartPos.GetNode(), rStartPos.GetContentIndex(),
                                rStartPos.GetNode(), rStartPos.GetContentIndex() + nLen);
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
    if( !m_oContentSect )
    {
        auto [pStt, pEnd] = StartEnd(); // SwPosition*

        SwDoc& rDoc = GetDoc();
        SwPaM aPam( *pStt, *pEnd );
        SwContentNode* pCSttNd = pStt->GetNode().GetContentNode();
        SwContentNode* pCEndNd = pEnd->GetNode().GetContentNode();

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

            pSttNd = rNds.MakeTextSection( rNds.GetEndOfRedlines(),
                                            SwNormalStartNode, pColl );
            SwTextNode* pTextNd = rNds[ pSttNd->GetIndex() + 1 ]->GetTextNode();

            SwPosition aPos( *pTextNd );
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
            pSttNd = SwNodes::MakeEmptySection( rNds.GetEndOfRedlines() );

            SwPosition aPos( *pSttNd->EndOfSectionNode() );
            rDoc.getIDocumentContentOperations().MoveRange( aPam, aPos,
                SwMoveFlags::DEFAULT );
        }
        m_oContentSect.emplace( *pSttNd );

        if( pStt == GetPoint() )
            Exchange();

        DeleteMark();
    }
    else
        InvalidateRange(Invalidation::Remove);
}

void SwRangeRedline::CopyToSection()
{
    if( m_oContentSect )
        return;

    auto [pStt, pEnd] = StartEnd(); // SwPosition*

    SwContentNode* pCSttNd = pStt->GetNode().GetContentNode();
    SwContentNode* pCEndNd = pEnd->GetNode().GetContentNode();

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

        pSttNd = rNds.MakeTextSection( rNds.GetEndOfRedlines(),
                                        SwNormalStartNode, pColl );

        SwPosition aPos( *pSttNd, SwNodeOffset(1) );

        // tdf#115815 keep original start position of collapsed annotation ranges
        // as temporary bookmarks (removed after file saving and file loading)
        lcl_storeAnnotationMarks( rDoc, pStt, pEnd );
        rDoc.getIDocumentContentOperations().CopyRange(*this, aPos, SwCopyFlags::CheckPosInFly);

        // Take over the style from the EndNode if needed
        // We don't want this in Doc::Copy
        if( pCEndNd && pCEndNd != pCSttNd )
        {
            SwContentNode* pDestNd = aPos.GetNode().GetContentNode();
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
        pSttNd = SwNodes::MakeEmptySection( rNds.GetEndOfRedlines() );

        if( pCEndNd )
        {
            SwPosition aPos( *pSttNd->EndOfSectionNode() );
            rDoc.getIDocumentContentOperations().CopyRange(*this, aPos, SwCopyFlags::CheckPosInFly);
        }
        else
        {
            SwNodeRange aRg( pStt->GetNode(), SwNodeOffset(0), pEnd->GetNode(), SwNodeOffset(1) );
            rDoc.GetDocumentContentOperationsManager().CopyWithFlyInFly(aRg, *pSttNd->EndOfSectionNode());
        }
    }
    m_oContentSect.emplace( *pSttNd );

    rDoc.SetCopyIsMove( bSaveCopyFlag );
    rDoc.getIDocumentRedlineAccess().SetRedlineMove( bSaveRdlMoveFlg );
}

void SwRangeRedline::DelCopyOfSection(size_t nMyPos)
{
    if( !m_oContentSect )
        return;

    auto [pStt, pEnd] = StartEnd(); // SwPosition*

    SwDoc& rDoc = GetDoc();
    SwPaM aPam( *pStt, *pEnd );
    SwContentNode* pCSttNd = pStt->GetNode().GetContentNode();
    SwContentNode* pCEndNd = pEnd->GetNode().GetContentNode();

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

            aPam.DeleteMark();
            aPam.GetPoint()->SetContent(0);;
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
    if( m_oContentSect )
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

        const SwNode* pKeptContentSectNode( &m_oContentSect->GetNode() ); // #i95711#
        {
            SwPaM aPam( m_oContentSect->GetNode(),
                        *m_oContentSect->GetNode().EndOfSectionNode(), SwNodeOffset(1),
                        SwNodeOffset( m_bDelLastPara ? -2 : -1 ) );
            SwContentNode* pCNd = aPam.GetPointContentNode();
            if( pCNd )
                aPam.GetPoint()->SetContent( pCNd->Len() );
            else
                aPam.GetPoint()->Adjust(SwNodeOffset(+1));

            SwFormatColl* pColl = pCNd && pCNd->Len() && aPam.GetPoint()->GetNode() !=
                                        aPam.GetMark()->GetNode()
                                ? pCNd->GetFormatColl() : nullptr;

            SwNodeIndex aNdIdx( GetPoint()->GetNode(), -1 );
            const sal_Int32 nPos = GetPoint()->GetContentIndex();

            SwPosition aPos( *GetPoint() );
            if( m_bDelLastPara && *aPam.GetPoint() == *aPam.GetMark() )
            {
                aPos.Adjust(SwNodeOffset(-1));

                rDoc.getIDocumentContentOperations().AppendTextNode( aPos );
            }
            else
            {
                rDoc.getIDocumentContentOperations().MoveRange( aPam, aPos,
                    SwMoveFlags::ALLFLYS );
            }

            SetMark();
            *GetPoint() = aPos;
            GetMark()->Assign(aNdIdx.GetIndex() + 1);
            pCNd = GetMark()->GetNode().GetContentNode();
            if( pCNd )
                GetMark()->SetContent( nPos );

            if( m_bDelLastPara )
            {
                GetPoint()->Adjust(SwNodeOffset(+1));
                pCNd = GetPointContentNode();
                m_bDelLastPara = false;
            }
            else if( pColl )
                pCNd = GetPointContentNode();

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
        if ( &m_oContentSect->GetNode() == pKeptContentSectNode )
        {
            rDoc.getIDocumentContentOperations().DeleteSection( &m_oContentSect->GetNode() );
        }
        m_oContentSect.reset();

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
void SwRangeRedline::SetContentIdx( const SwNodeIndex& rIdx )
{
    if( !m_oContentSect )
    {
        m_oContentSect = rIdx;
        m_bIsVisible = false;
    }
    else
    {
        OSL_FAIL("SwRangeRedline::SetContentIdx: invalid state");
    }
}

// for Undo
void SwRangeRedline::ClearContentIdx()
{
    if( m_oContentSect )
    {
        m_oContentSect.reset();
    }
    else
    {
        OSL_FAIL("SwRangeRedline::ClearContentIdx: invalid state");
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

bool SwRangeRedline::PopAllDataAfter(int depth)
{
    assert(depth > 0);
    SwRedlineData* pCur = m_pRedlineData;
    while (depth > 1)
    {
        pCur = pCur->m_pNext;
        if (!pCur)
            return false;
        depth--;
    }

    while (pCur->m_pNext)
    {
        SwRedlineData* pToDelete = pCur->m_pNext;
        pCur->m_pNext = pToDelete->m_pNext;
        delete pToDelete;
    }
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

sal_uInt32 SwRangeRedline::GetMovedID(sal_uInt16 nPos) const
{
    return GetRedlineData(nPos).m_nMovedID;
}

const DateTime& SwRangeRedline::GetTimeStamp(sal_uInt16 nPos) const
{
    return GetRedlineData(nPos).m_aStamp;
}

RedlineType SwRangeRedline::GetType( sal_uInt16 nPos ) const
{
    return GetRedlineData(nPos).m_eType;
}

bool SwRangeRedline::IsAnnotation() const
{
    return GetText().getLength() == 1 && GetText()[0] == CH_TXTATR_INWORD;
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
    if (!m_oContentSect.has_value())
    {
        pPaM = this;
    }
    else // otherwise it is saved in pContentSect
    {
        pPaM = new SwPaM( m_oContentSect->GetNode(), *m_oContentSect->GetNode().EndOfSectionNode() );
        bDeletePaM = true;
    }

    OUString sDescr = DenoteSpecialCharacters(pPaM->GetText().replace('\n', ' '), /*bQuoted=*/!bSimplified);
    if (const SwTextNode *pTextNode = pPaM->GetPointNode().GetTextNode())
    {
        if (const SwTextAttr* pTextAttr = pTextNode->GetFieldTextAttrAt(pPaM->GetPoint()->GetContentIndex() - 1, ::sw::GetTextAttrMode::Default))
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

    const SwRedlineData* pRedlineData = m_pRedlineData;
    while (pRedlineData)
    {
        pRedlineData->dumpAsXml(pWriter);
        pRedlineData = pRedlineData->Next();
    }

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
