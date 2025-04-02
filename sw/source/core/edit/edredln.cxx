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

#include <IDocumentRedlineAccess.hxx>
#include <IDocumentUndoRedo.hxx>
#include <SwRewriter.hxx>
#include <docary.hxx>
#include <redline.hxx>
#include <doc.hxx>
#include <editsh.hxx>
#include <frmtool.hxx>
#include <docsh.hxx>
#include <swdtflvr.hxx>
#include <strings.hrc>

RedlineFlags SwEditShell::GetRedlineFlags() const
{
    return GetDoc()->getIDocumentRedlineAccess().GetRedlineFlags(this);
}

void SwEditShell::SetRedlineFlags( RedlineFlags eMode, bool bRecordAllViews )
{
    if (SwDocShell* pDocSh = GetDoc()->GetDocShell())
    {
        bool bRecordModeChange = bRecordAllViews != pDocSh->IsChangeRecording(nullptr, bRecordAllViews);
        if( eMode != GetDoc()->getIDocumentRedlineAccess().GetRedlineFlags() || bRecordModeChange )
        {
            CurrShell aCurr( this );
            StartAllAction();
            GetDoc()->getIDocumentRedlineAccess().SetRedlineFlags( eMode, bRecordAllViews, bRecordModeChange );
            EndAllAction();
        }
    }
}

bool SwEditShell::IsRedlineOn() const
{
    return GetDoc()->getIDocumentRedlineAccess().IsRedlineOn();
}

SwRedlineTable::size_type SwEditShell::GetRedlineCount() const
{
    return GetDoc()->getIDocumentRedlineAccess().GetRedlineTable().size();
}

const SwRangeRedline& SwEditShell::GetRedline( SwRedlineTable::size_type nPos ) const
{
    return *GetDoc()->getIDocumentRedlineAccess().GetRedlineTable()[ nPos ];
}

SwRangeRedline& SwEditShell::GetRedline(SwRedlineTable::size_type nPos)
{
    return const_cast<SwRangeRedline&>(const_cast<const SwEditShell*>(this)->GetRedline(nPos));
}

static void lcl_InvalidateAll( SwViewShell* pSh )
{
    for(SwViewShell& rCurrentShell : pSh->GetRingContainer())
    {
        if ( rCurrentShell.GetWin() )
            rCurrentShell.GetWin()->Invalidate();
    }
}

bool SwEditShell::AcceptRedline( SwRedlineTable::size_type nPos )
{
    CurrShell aCurr( this );
    StartAllAction();
    bool bRet = GetDoc()->getIDocumentRedlineAccess().AcceptRedline( nPos, true, true );
    if( !nPos && !::IsExtraData( GetDoc() ) )
        lcl_InvalidateAll( this );
    EndAllAction();
    return bRet;
}

void SwEditShell::ReinstatePaM(const SwRangeRedline& rRedline, SwPaM& rPaM)
{
    if (rRedline.GetType() == RedlineType::Insert)
    {
        DeleteSel(rPaM, /*isArtificialSelection=*/true);
    }
    else if (rRedline.GetType() == RedlineType::Delete)
    {
        // Re-insert after the deletion.
        SwDocShell* pDocShell = GetDoc()->GetDocShell();
        if (!pDocShell)
        {
            return;
        }

        SwWrtShell* pWrtShell = pDocShell->GetWrtShell();
        if (!pWrtShell)
        {
            return;
        }

        // Get rid of table selection or multi-selection if there is one.
        KillPams();
        assert(!GetTableCursor() && "coverity#1645529");
        SwShellCursor* pCursor = getShellCursor(/*bBlock=*/true);
        *pCursor->GetPoint() = *rPaM.End();
        SetMark();
        *pCursor->GetMark() = *rPaM.Start();
        rtl::Reference<SwTransferable> pTransfer(new SwTransferable(*pWrtShell));
        // Copy rich text, but don't strip out text inside delete redlines.
        pTransfer->Copy(/*bIsCut=*/false, /*bDeleteRedlines=*/false);
        ClearMark();
        *pCursor->GetPoint() = *rPaM.End();
        TransferableDataHelper aHelper(pTransfer);
        SwTransferable::Paste(*pWrtShell, aHelper);
    }
}

void SwEditShell::ReinstateRedline(SwRedlineTable::size_type nPos)
{
    CurrShell aCurr(this);
    StartAllAction();

    if (!IsRedlineOn())
    {
        RedlineFlags nMode = GetRedlineFlags();
        SetRedlineFlags(nMode | RedlineFlags::On, /*bRecordAllViews=*/false);
    }

    SwRangeRedline& rRedline = GetRedline(nPos);
    SwPaM aPaM(*rRedline.GetPoint());
    aPaM.SetMark();
    *aPaM.GetMark() = *rRedline.GetMark();

    IDocumentUndoRedo& rIDUR = GetDoc()->GetIDocumentUndoRedo();
    if (rIDUR.DoesUndo())
    {
        SwRewriter aRewriter;
        aRewriter.AddRule(UndoArg1, rRedline.GetDescr());
        rIDUR.StartUndo(SwUndoId::REINSTATE_REDLINE, &aRewriter);
    }
    ReinstatePaM(rRedline, aPaM);
    if (rIDUR.DoesUndo())
    {
        rIDUR.EndUndo(SwUndoId::END, nullptr);
    }

    EndAllAction();
}

bool SwEditShell::RejectRedline( SwRedlineTable::size_type nPos )
{
    CurrShell aCurr( this );
    StartAllAction();
    bool bRet = GetDoc()->getIDocumentRedlineAccess().RejectRedline( nPos, true, true );
    if( !nPos && !::IsExtraData( GetDoc() ) )
        lcl_InvalidateAll( this );
    EndAllAction();
    return bRet;
}

bool SwEditShell::AcceptRedlinesInSelection()
{
    CurrShell aCurr( this );
    StartAllAction();
    // in table selection mode, process the selected boxes in reverse order
    // to allow accepting their text changes and the tracked row deletions
    bool bRet = false;
    if ( IsTableMode() )
    {
        const SwSelBoxes& rBoxes = GetTableCursor()->GetSelectedBoxes();
        std::vector<std::unique_ptr<SwPaM>> vBoxes;
        for(auto pBox : rBoxes)
        {
            if ( !pBox->IsEmpty() )
            {
                const SwStartNode *pSttNd = pBox->GetSttNd();
                SwNode* pEndNode = pSttNd->GetNodes()[pSttNd->EndOfSectionIndex()];
                vBoxes.push_back(std::unique_ptr<SwPaM>(new SwPaM(*pEndNode, 0, *pSttNd, 0)));
            }
        }

        for (size_t i = 0; i < vBoxes.size(); ++i)
            bRet |= GetDoc()->getIDocumentRedlineAccess().AcceptRedline( *vBoxes[vBoxes.size()-i-1], true );
    }
    else
        bRet = GetDoc()->getIDocumentRedlineAccess().AcceptRedline( *GetCursor(), true );
    EndAllAction();
    return bRet;
}

bool SwEditShell::RejectRedlinesInSelection()
{
    CurrShell aCurr( this );
    StartAllAction();
    bool bRet = false;
    // in table selection mode, process the selected boxes in reverse order
    // to allow rejecting their text changes and the tracked row insertions
    if ( IsTableMode() )
    {
        const SwSelBoxes& rBoxes = GetTableCursor()->GetSelectedBoxes();
        std::vector<std::unique_ptr<SwPaM>> vBoxes;
        for(auto pBox : rBoxes)
        {
            if ( !pBox->IsEmpty() )
            {
                const SwStartNode *pSttNd = pBox->GetSttNd();
                SwNode* pEndNode = pSttNd->GetNodes()[pSttNd->EndOfSectionIndex()];
                vBoxes.push_back(std::unique_ptr<SwPaM>(new SwPaM(*pEndNode, 0, *pSttNd, 0)));
            }
        }

        for (size_t i = 0; i < vBoxes.size(); ++i)
            bRet |= GetDoc()->getIDocumentRedlineAccess().RejectRedline( *vBoxes[vBoxes.size()-i-1], true );
    }
    else
        bRet = GetDoc()->getIDocumentRedlineAccess().RejectRedline( *GetCursor(), true );
    EndAllAction();
    return bRet;
}

void SwEditShell::ReinstateRedlinesInSelection()
{
    CurrShell aCurr( this );
    StartAllAction();
    if (!IsRedlineOn())
    {
        RedlineFlags nMode = GetRedlineFlags();
        SetRedlineFlags(nMode | RedlineFlags::On, /*bRecordAllViews=*/false);
    }

    SwPosition aCursorStart(*GetCursor()->Start());
    SwPosition aCursorEnd(*GetCursor()->End());
    SwRedlineTable& rTable = GetDoc()->getIDocumentRedlineAccess().GetRedlineTable();

    // Work on a copy, since reinstate will modify the table, and reinstate of just inserted
    // redlines is not wanted.
    std::vector<SwRangeRedline*> aRedlines(rTable.begin(), rTable.end());

    IDocumentUndoRedo& rIDUR = GetDoc()->GetIDocumentUndoRedo();
    if (rIDUR.DoesUndo())
    {
        rIDUR.StartUndo(SwUndoId::REINSTATE_REDLINE, nullptr);
    }
    int nRedlines = 0;
    for (size_t nIndex = 0; nIndex < aRedlines.size(); ++nIndex)
    {
        const SwRangeRedline& rRedline = *aRedlines[nIndex];
        if (!rRedline.HasMark() || !rRedline.IsVisible())
        {
            continue;
        }

        if (*rRedline.End() < aCursorStart)
        {
            // Ends before the selection, skip to the next redline.
            continue;
        }

        if (*rRedline.Start() > aCursorEnd)
        {
            // Starts after the selection, can stop.
            break;
        }

        // Check if the redline is only partially selected.
        const SwPosition* pStart = rRedline.Start();
        if (*pStart < aCursorStart)
        {
            pStart = &aCursorStart;
        }
        const SwPosition* pEnd = rRedline.End();
        if (*pEnd > aCursorEnd)
        {
            pEnd = &aCursorEnd;
        }

        // Process the (partially) selected redline.
        SwPaM aPaM(*pEnd);
        aPaM.SetMark();
        *aPaM.GetMark() = *pStart;
        ReinstatePaM(rRedline, aPaM);
        ++nRedlines;
    }
    if (rIDUR.DoesUndo())
    {
        OUString aWith;
        {
            SwRewriter aRewriter;
            aRewriter.AddRule(UndoArg1, OUString::number(nRedlines));
            aWith = aRewriter.Apply(SwResId(STR_N_REDLINES));
        }
        SwRewriter aRewriter;
        aRewriter.AddRule(UndoArg1, aWith);
        rIDUR.EndUndo(SwUndoId::REINSTATE_REDLINE, &aRewriter);
    }

    EndAllAction();
}

// Set the comment at the Redline
bool SwEditShell::SetRedlineComment( const OUString& rS )
{
    bool bRet = false;
    for(const SwPaM& rPaM : GetCursor()->GetRingContainer())
    {
        bRet = bRet || GetDoc()->getIDocumentRedlineAccess().SetRedlineComment( rPaM, rS );
    }

    return bRet;
}

const SwRangeRedline* SwEditShell::GetCurrRedline() const
{
    if (const SwRangeRedline* pRed = GetDoc()->getIDocumentRedlineAccess().GetRedline( *GetCursor()->GetPoint(), nullptr ))
        return pRed;
    // check the other side of the selection to handle completely selected changes, where the Point is at the end
    return GetDoc()->getIDocumentRedlineAccess().GetRedline( *GetCursor()->GetMark(), nullptr );
}

void SwEditShell::UpdateRedlineAttr()
{
    if( IDocumentRedlineAccess::IsShowChanges(GetDoc()->getIDocumentRedlineAccess().GetRedlineFlags()) )
    {
        CurrShell aCurr( this );
        StartAllAction();

        GetDoc()->getIDocumentRedlineAccess().UpdateRedlineAttr();

        EndAllAction();
    }
}

/** Search the Redline of the data given
 *
 * @return Returns the Pos of the Array, or SwRedlineTable::npos if not present
 */
SwRedlineTable::size_type SwEditShell::FindRedlineOfData( const SwRedlineData& rData ) const
{
    const SwRedlineTable& rTable = GetDoc()->getIDocumentRedlineAccess().GetRedlineTable();

    for( SwRedlineTable::size_type i = 0, nCnt = rTable.size(); i < nCnt; ++i )
        if( &rTable[ i ]->GetRedlineData() == &rData )
            return i;
    return SwRedlineTable::npos;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
