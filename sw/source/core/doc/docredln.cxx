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

#include <sal/log.hxx>
#include <tools/datetimeutils.hxx>
#include <hintids.hxx>
#include <svl/itemiter.hxx>
#include <sfx2/app.hxx>
#include <editeng/colritem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/crossedoutitem.hxx>
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
#include <swundo.hxx>
#include <UndoCore.hxx>
#include <UndoRedline.hxx>
#include <hints.hxx>
#include <pamtyp.hxx>
#include <poolfmt.hxx>
#include <view.hxx>
#include <viewsh.hxx>
#include <viscrs.hxx>
#include <rootfrm.hxx>
#include <strings.hrc>
#include <unoport.hxx>
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
    xmlTextWriterStartElement(pWriter, BAD_CAST("SwExtraRedlineTable"));
    xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("ptr"), "%p", this);

    for (sal_uInt16 nCurExtraRedlinePos = 0; nCurExtraRedlinePos < GetSize(); ++nCurExtraRedlinePos)
    {
        const SwExtraRedline* pExtraRedline = GetRedline(nCurExtraRedlinePos);
        xmlTextWriterStartElement(pWriter, BAD_CAST("SwExtraRedline"));
        xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("ptr"), "%p", this);
        xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("symbol"), "%s", BAD_CAST(typeid(*pExtraRedline).name()));
        xmlTextWriterEndElement(pWriter);
    }
    xmlTextWriterEndElement(pWriter);
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

bool SwExtraRedlineTable::DeleteAllTableRedlines( SwDoc* pDoc, const SwTable& rTable, bool bSaveInUndo, sal_uInt16 nRedlineTypeToDelete )
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

namespace
{

void lcl_LOKInvalidateFrames(const SwModify& rMod, const SwRootFrame* pLayout,
        SwFrameType const nFrameType, const Point* pPoint)
{
    SwIterator<SwFrame, SwModify, sw::IteratorMode::UnwrapMulti> aIter(rMod);

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
               *pEndPos   = rCursor.GetPoint() == pStartPos ? rCursor.GetMark() : rCursor.GetPoint();


    lcl_LOKInvalidateFrames(*(pStartPos->nNode.GetNode().GetContentNode()),
                            rCursor.GetShell()->GetLayout(),
                            FRM_CNTNT, &rCursor.GetSttPos());

    lcl_LOKInvalidateFrames(*(pEndPos->nNode.GetNode().GetContentNode()),
                            rCursor.GetShell()->GetLayout(),
                            FRM_CNTNT, &rCursor.GetEndPos());
}

} // anonymous namespace

/// Emits LOK notification about one addition / removal of a redline item.
void SwRedlineTable::LOKRedlineNotification(RedlineNotification nType, SwRangeRedline* pRedline)
{
    // Disable since usability is very low beyond some small number of changes.
    static bool bDisableRedlineComments = getenv("DISABLE_REDLINE") != nullptr;
    if (!comphelper::LibreOfficeKit::isActive() || bDisableRedlineComments)
        return;

    boost::property_tree::ptree aRedline;
    aRedline.put("action", (nType == RedlineNotification::Add ? "Add" :
                            (nType == RedlineNotification::Remove ? "Remove" :
                             (nType == RedlineNotification::Modify ? "Modify" : "???"))));
    aRedline.put("index", pRedline->GetId());
    aRedline.put("author", pRedline->GetAuthorString(1).toUtf8().getStr());
    aRedline.put("type", nsRedlineType_t::SwRedlineTypeToOUString(pRedline->GetRedlineData().GetType()).toUtf8().getStr());
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
        aCursor.GetMark()->nNode = *pContentNd;
        aCursor.GetMark()->nContent.Assign(pContentNd, pEndPos->nContent.GetIndex());

        aCursor.FillRects();

        SwRects* pRects(&aCursor);
        std::vector<OString> aRects;
        for(SwRect& rNextRect : *pRects)
            aRects.push_back(rNextRect.SVRect().toString());

        const OString sRects = comphelper::string::join("; ", aRects);
        aRedline.put("textRange", sRects.getStr());

        lcl_LOKInvalidateStartEndFrames(aCursor);

        // When this notify method is called text invalidation is not done yet
        // Calling FillRects updates the text area so invalidation will not run on the correct rects
        // So we need to do an own invalidation here. It invalidates text frames containing the redlining
        SwDoc* pDoc = pRedline->GetDoc();
        SwViewShell* pSh;
        if( pDoc && !pDoc->IsInDtor() &&
            nullptr != ( pSh = pDoc->getIDocumentLayoutAccess().GetCurrentViewShell()) )
        {
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
        pViewShell->libreOfficeKitViewCallback(nType == RedlineNotification::Modify ? LOK_CALLBACK_REDLINE_TABLE_ENTRY_MODIFIED : LOK_CALLBACK_REDLINE_TABLE_SIZE_CHANGED, aPayload.c_str());
        pViewShell = SfxViewShell::GetNext(*pViewShell);
    }
}

bool SwRedlineTable::Insert(SwRangeRedlinePtr& p)
{
    if( p->HasValidRange() )
    {
        std::pair<vector_type::const_iterator, bool> rv = maVector.insert( p );
        size_type nP = rv.first - begin();
        LOKRedlineNotification(RedlineNotification::Add, p);
        p->CallDisplayFunc(nP);
        return rv.second;
    }
    return InsertWithValidRanges( p );
}

bool SwRedlineTable::Insert(SwRangeRedlinePtr& p, size_type& rP)
{
    if( p->HasValidRange() )
    {
        std::pair<vector_type::const_iterator, bool> rv = maVector.insert( p );
        rP = rv.first - begin();
        p->CallDisplayFunc(rP);
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

            if( aNewStt >= *pEnd ||
                nullptr == (pC = rNds.GoNext( &aNewStt.nNode )) )
                break;

            aNewStt.nContent.Assign( pC, 0 );

        } while( aNewStt < *pEnd );

    delete pNew;
    p.reset();
    return ret;
}

} // namespace sw

bool SwRedlineTable::InsertWithValidRanges(SwRangeRedlinePtr& p, size_type* pInsPos)
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
        pDoc = maVector.front()->GetDoc();

    maVector.erase( maVector.begin() + nP );

    SwViewShell* pSh;
    if( pDoc && !pDoc->IsInDtor() &&
        nullptr != ( pSh = pDoc->getIDocumentLayoutAccess().GetCurrentViewShell()) )
        pSh->InvalidateWindows( SwRect( 0, 0, SAL_MAX_INT32, SAL_MAX_INT32 ) );
}

void SwRedlineTable::DeleteAndDestroyAll()
{
    while (!maVector.empty())
    {
        auto const pRedline = maVector.back();
        maVector.erase(maVector.size() - 1);
        LOKRedlineNotification(RedlineNotification::Remove, pRedline);
        delete pRedline;
    }
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
    auto const nLookahead = 20;
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
    auto const nLookahead = 20;
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
    xmlTextWriterStartElement(pWriter, BAD_CAST("SwRedlineTable"));
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
    : m_sFormatNm(rColl), m_nPoolId(nPoolFormatId)
{
    if( pItemSet && pItemSet->Count() )
        m_pSet.reset( new SfxItemSet( *pItemSet ) );
}

SwRedlineExtraData_FormatColl::~SwRedlineExtraData_FormatColl()
{
}

SwRedlineExtraData* SwRedlineExtraData_FormatColl::CreateNew() const
{
    return new SwRedlineExtraData_FormatColl( m_sFormatNm, m_nPoolId, m_pSet.get() );
}

void SwRedlineExtraData_FormatColl::Reject( SwPaM& rPam ) const
{
    SwDoc* pDoc = rPam.GetDoc();

    // What about Undo? Is it turned off?
    SwTextFormatColl* pColl = USHRT_MAX == m_nPoolId
                            ? pDoc->FindTextFormatCollByName( m_sFormatNm )
                            : pDoc->getIDocumentStylePoolAccess().GetTextCollFromPool( m_nPoolId );
    if( pColl )
        pDoc->SetTextFormatColl( rPam, pColl, false );

    if( m_pSet )
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
                SfxItemSet aTmp( *m_pSet );
                aTmp.Differentiate( *pTNd->GetpSwAttrSet() );
                pDoc->getIDocumentContentOperations().InsertItemSet( rPam, aTmp );
            }
            else
            {
                pDoc->getIDocumentContentOperations().InsertItemSet( rPam, *m_pSet );
            }
        }
        rPam.DeleteMark();
    }
}

bool SwRedlineExtraData_FormatColl::operator == ( const SwRedlineExtraData& r) const
{
    const SwRedlineExtraData_FormatColl& rCmp = static_cast<const SwRedlineExtraData_FormatColl&>(r);
    return m_sFormatNm == rCmp.m_sFormatNm && m_nPoolId == rCmp.m_nPoolId &&
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
    const SfxPoolItem* pItem = aIter.FirstItem();
    while(pItem)
    {
        m_aWhichIds.push_back( pItem->Which() );
        if( aIter.IsAtEnd() )
            break;
        pItem = aIter.NextItem();
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
    SwDoc* pDoc = rPam.GetDoc();

    RedlineFlags eOld = pDoc->getIDocumentRedlineAccess().GetRedlineFlags();
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags_intern(eOld & ~RedlineFlags(RedlineFlags::On | RedlineFlags::Ignore));

    // Actually we need to reset the Attribute here!
    for( const auto& rWhichId : m_aWhichIds )
    {
        pDoc->getIDocumentContentOperations().InsertPoolItem( rPam, *GetDfltAttr( rWhichId ),
            SetAttrMode::DONTEXPAND );
    }

    pDoc->getIDocumentRedlineAccess().SetRedlineFlags_intern( eOld );
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

SwRedlineExtraData_FormattingChanges::SwRedlineExtraData_FormattingChanges( const SfxItemSet* pItemSet )
{
    if( pItemSet && pItemSet->Count() )
        m_pSet.reset( new SfxItemSet( *pItemSet ) );
}

SwRedlineExtraData_FormattingChanges::SwRedlineExtraData_FormattingChanges( const SwRedlineExtraData_FormattingChanges& rCpy )
    : SwRedlineExtraData()
{
    // Checking pointer pSet before accessing it for Count
    if( rCpy.m_pSet && rCpy.m_pSet->Count() )
    {
        m_pSet.reset( new SfxItemSet( *(rCpy.m_pSet) ) );
    }
    else
    {
        m_pSet.reset();
    }
}

SwRedlineExtraData_FormattingChanges::~SwRedlineExtraData_FormattingChanges()
{
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

    if ( !m_pSet && !rCmp.m_pSet )
    {
        // Both SfxItemSet are null
        return true;
    }
    else if ( m_pSet && rCmp.m_pSet && *m_pSet == *rCmp.m_pSet )
    {
        // Both SfxItemSet exist and are equal
        return true;
    }
    return false;
}

SwRedlineData::SwRedlineData( RedlineType_t eT, std::size_t nAut )
    : m_pNext( nullptr ), m_pExtraData( nullptr ),
    m_aStamp( DateTime::SYSTEM ),
    m_eType( eT ), m_nAuthor( nAut ), m_nSeqNo( 0 )
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
    , m_eType( rCpy.m_eType )
    , m_nAuthor( rCpy.m_nAuthor )
    , m_nSeqNo( rCpy.m_nSeqNo )
{
}

// For sw3io: We now own pNext!
SwRedlineData::SwRedlineData(RedlineType_t eT, std::size_t nAut, const DateTime& rDT,
    const OUString& rCmnt, SwRedlineData *pNxt)
    : m_pNext(pNxt), m_pExtraData(nullptr), m_sComment(rCmnt), m_aStamp(rDT),
    m_eType(eT), m_nAuthor(nAut), m_nSeqNo(0)
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

static const char* STR_REDLINE_ARY[] =
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
    return SwResId(STR_REDLINE_ARY[GetType()]);
}

sal_uInt32 SwRangeRedline::m_nLastId = 1;

SwRangeRedline::SwRangeRedline(RedlineType_t eTyp, const SwPaM& rPam )
    : SwPaM( *rPam.GetMark(), *rPam.GetPoint() ),
    m_pRedlineData( new SwRedlineData( eTyp, GetDoc()->getIDocumentRedlineAccess().GetRedlineAuthor() ) ),
    m_pContentSect( nullptr ),
    m_nId( m_nLastId++ )
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
    m_nId( m_nLastId++ )
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
    m_nId( m_nLastId++ )
{
    m_bDelLastPara = false;
    m_bIsVisible = true;
}

SwRangeRedline::SwRangeRedline( const SwRangeRedline& rCpy )
    : SwPaM( *rCpy.GetMark(), *rCpy.GetPoint() ),
    m_pRedlineData( new SwRedlineData( *rCpy.m_pRedlineData )),
    m_pContentSect( nullptr ),
    m_nId( rCpy.m_nId )
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
        if( !GetDoc()->IsInDtor() )
            GetDoc()->getIDocumentContentOperations().DeleteSection( &m_pContentSect->GetNode() );
        delete m_pContentSect;
    }
    delete m_pRedlineData;
}

void MaybeNotifyRedlineModification(SwRangeRedline* pRedline, SwDoc* pDoc)
{
    if (!comphelper::LibreOfficeKit::isActive())
        return;

    const SwRedlineTable& rRedTable = pDoc->getIDocumentRedlineAccess().GetRedlineTable();
    for (SwRedlineTable::size_type i = 0; i < rRedTable.size(); ++i)
    {
        if (rRedTable[i] == pRedline)
        {
            SwRedlineTable::LOKRedlineNotification(RedlineNotification::Modify, pRedline);
            break;
        }
    }
}

void SwRangeRedline::MaybeNotifyRedlinePositionModification(long nTop)
{
    if (!comphelper::LibreOfficeKit::isActive())
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

    MaybeNotifyRedlineModification(this, GetDoc());
}

void SwRangeRedline::SetEnd( const SwPosition& rPos, SwPosition* pEndPtr )
{
    if( !pEndPtr ) pEndPtr = End();
    *pEndPtr = rPos;

    MaybeNotifyRedlineModification(this, GetDoc());
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
    RedlineFlags eShow = RedlineFlags::ShowMask & GetDoc()->getIDocumentRedlineAccess().GetRedlineFlags();
    if (eShow == (RedlineFlags::ShowInsert | RedlineFlags::ShowDelete))
        Show(0, nMyPos);
    else if (eShow == RedlineFlags::ShowInsert)
        Hide(0, nMyPos);
    else if (eShow == RedlineFlags::ShowDelete)
        ShowOriginal(0, nMyPos);
}

void SwRangeRedline::Show(sal_uInt16 nLoop, size_t nMyPos)
{
    if( 1 <= nLoop )
    {
        SwDoc* pDoc = GetDoc();
        RedlineFlags eOld = pDoc->getIDocumentRedlineAccess().GetRedlineFlags();
        pDoc->getIDocumentRedlineAccess().SetRedlineFlags_intern(eOld | RedlineFlags::Ignore);
        ::sw::UndoGuard const undoGuard(pDoc->GetIDocumentUndoRedo());

        switch( GetType() )
        {
        case nsRedlineType_t::REDLINE_INSERT:           // Content has been inserted
            m_bIsVisible = true;
            MoveFromSection(nMyPos);
            break;

        case nsRedlineType_t::REDLINE_DELETE:           // Content has been deleted
            m_bIsVisible = true;
            MoveFromSection(nMyPos);
            break;

        case nsRedlineType_t::REDLINE_FORMAT:           // Attributes have been applied
        case nsRedlineType_t::REDLINE_TABLE:            // Table structure has been modified
            InvalidateRange(Invalidation::Add);
            break;
        default:
            break;
        }
        pDoc->getIDocumentRedlineAccess().SetRedlineFlags_intern( eOld );
    }
}

void SwRangeRedline::Hide(sal_uInt16 nLoop, size_t nMyPos)
{
    SwDoc* pDoc = GetDoc();
    RedlineFlags eOld = pDoc->getIDocumentRedlineAccess().GetRedlineFlags();
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags_intern(eOld | RedlineFlags::Ignore);
    ::sw::UndoGuard const undoGuard(pDoc->GetIDocumentUndoRedo());

    switch( GetType() )
    {
    case nsRedlineType_t::REDLINE_INSERT:           // Content has been inserted
        m_bIsVisible = true;
        if( 1 <= nLoop )
            MoveFromSection(nMyPos);
        break;

    case nsRedlineType_t::REDLINE_DELETE:           // Content has been deleted
        m_bIsVisible = false;
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
            InvalidateRange(Invalidation::Remove);
        break;
    default:
        break;
    }
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags_intern( eOld );
}

void SwRangeRedline::ShowOriginal(sal_uInt16 nLoop, size_t nMyPos)
{
    SwDoc* pDoc = GetDoc();
    RedlineFlags eOld = pDoc->getIDocumentRedlineAccess().GetRedlineFlags();
    SwRedlineData* pCur;

    pDoc->getIDocumentRedlineAccess().SetRedlineFlags_intern(eOld | RedlineFlags::Ignore);
    ::sw::UndoGuard const undoGuard(pDoc->GetIDocumentUndoRedo());

    // Determine the Type, it's the first on Stack
    for( pCur = m_pRedlineData; pCur->m_pNext; )
        pCur = pCur->m_pNext;

    switch( pCur->m_eType )
    {
    case nsRedlineType_t::REDLINE_INSERT:           // Content has been inserted
        m_bIsVisible = false;
        switch( nLoop )
        {
        case 0: MoveToSection();    break;
        case 1: CopyToSection();    break;
        case 2: DelCopyOfSection(nMyPos); break;
        }
        break;

    case nsRedlineType_t::REDLINE_DELETE:           // Content has been deleted
        m_bIsVisible = true;
        if( 1 <= nLoop )
            MoveFromSection(nMyPos);
        break;

    case nsRedlineType_t::REDLINE_FORMAT:           // Attributes have been applied
    case nsRedlineType_t::REDLINE_TABLE:            // Table structure has been modified
        if( 1 <= nLoop )
            InvalidateRange(Invalidation::Remove);
        break;
    default:
        break;
    }
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags_intern( eOld );
}

// trigger the Layout
void SwRangeRedline::InvalidateRange(Invalidation const eWhy)
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

            // SwUpdateAttr must be handled first, otherwise indexes are off
            if (GetType() == nsRedlineType_t::REDLINE_DELETE)
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

void SwRangeRedline::MoveToSection()
{
    if( !m_pContentSect )
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
            for(SwRangeRedline* pRedl : rTable)
            {
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
                pDoc->getIDocumentContentOperations().MoveAndJoin( aPam, aPos );
            else
            {
                if( pCSttNd && !pCEndNd )
                    m_bDelLastPara = true;
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
    // DocumentContentOperationsManager::CopyFlyInFlyImpl() method to change,
    // which will eventually be called by the CopyRange() below.
    pDoc->getIDocumentRedlineAccess().SetRedlineMove(true);

    if( pCSttNd )
    {
        SwTextFormatColl* pColl = pCSttNd->IsTextNode()
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
    m_pContentSect = new SwNodeIndex( *pSttNd );

    pDoc->SetCopyIsMove( bSaveCopyFlag );
    pDoc->getIDocumentRedlineAccess().SetRedlineMove( bSaveRdlMoveFlg );

}

void SwRangeRedline::DelCopyOfSection(size_t nMyPos)
{
    if( m_pContentSect )
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
            pDoc->getIDocumentContentOperations().DeleteAndJoin( aPam, true );
        }
        else if( pCSttNd || pCEndNd )
        {
            if( pCSttNd && !pCEndNd )
                m_bDelLastPara = true;
            pDoc->getIDocumentContentOperations().DeleteRange( aPam );

            if( m_bDelLastPara )
            {
                // To prevent dangling references to the paragraph to
                // be deleted, redline that point into this paragraph should be
                // moved to the new end position. Since redlines in the redline
                // table are sorted and the pEnd position is an endnode (see
                // bDelLastPara condition above), only redlines before the
                // current ones can be affected.
                const SwRedlineTable& rTable = pDoc->getIDocumentRedlineAccess().GetRedlineTable();
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
    if( m_pContentSect )
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

        const SwNode* pKeptContentSectNode( &m_pContentSect->GetNode() ); // #i95711#
        {
            SwPaM aPam( m_pContentSect->GetNode(),
                        *m_pContentSect->GetNode().EndOfSectionNode(), 1,
                        ( m_bDelLastPara ? -2 : -1 ) );
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
            pDoc->getIDocumentContentOperations().DeleteSection( &m_pContentSect->GetNode() );
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

RedlineType_t SwRangeRedline::GetRealType( sal_uInt16 nPos ) const
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

OUString SwRangeRedline::GetDescr()
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

    OUString sDescr = pPaM->GetText();
    if (const SwTextNode *pTextNode = pPaM->GetNode().GetTextNode())
    {
        if (const SwTextAttr* pTextAttr = pTextNode->GetFieldTextAttrAt(pPaM->GetPoint()->nContent.GetIndex() - 1, true ))
        {
            sDescr = pTextAttr->GetFormatField().GetField()->GetFieldName();
        }
    }

    // replace $1 in description by description of the redlines text
    const OUString aTmpStr = SwResId(STR_START_QUOTE)
        + ShortenString(sDescr, nUndoStringLength, SwResId(STR_LDOTS))
        + SwResId(STR_END_QUOTE);

    SwRewriter aRewriter;
    aRewriter.AddRule(UndoArg1, aTmpStr);

    aResult = aRewriter.Apply(aResult);

    if (bDeletePaM)
        delete pPaM;

    return aResult;
}

void SwRangeRedline::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    xmlTextWriterStartElement(pWriter, BAD_CAST("SwRangeRedline"));

    xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("ptr"), "%p", this);
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("id"), BAD_CAST(OString::number(GetSeqNo()).getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("author"), BAD_CAST(SW_MOD()->GetRedlineAuthor(GetAuthor()).toUtf8().getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("date"), BAD_CAST(DateTimeToOString(GetTimeStamp()).getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("descr"), BAD_CAST(const_cast<SwRangeRedline*>(this)->GetDescr().toUtf8().getStr()));

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
