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

#include <editsh.hxx>

#include <officecfg/Office/Common.hxx>
#include <osl/diagnose.h>
#include <unotools/configmgr.hxx>
#include <vcl/window.hxx>

#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <IDocumentState.hxx>
#include <pam.hxx>
#include <acorrect.hxx>
#include <swtable.hxx>
#include <ndtxt.hxx>
#include <txtfrm.hxx>
#include <swundo.hxx>
#include <SwRewriter.hxx>
#include <frameformats.hxx>

// masqueraded copy constructor
SwEditShell::SwEditShell( SwEditShell& rEdSH, vcl::Window *pWindow )
    : SwCursorShell( rEdSH, pWindow )
    , m_bNbspRunNext(false)   // TODO: would copying that make sense? only if editing continues
    , m_bDoParagraphSignatureValidation(true)
{
}

SwEditShell::SwEditShell( SwDoc& rDoc, vcl::Window *pWindow, const SwViewOption *pOptions )
    : SwCursorShell( rDoc, pWindow, pOptions )
    , m_bNbspRunNext(false)
    , m_bDoParagraphSignatureValidation(true)
{
    if (!comphelper::IsFuzzing() && 0 < officecfg::Office::Common::Undo::Steps::get())
    {
        GetDoc()->GetIDocumentUndoRedo().DoUndo(true);
    }

    // Restore the paragraph metadata fields and validate signatures.
    RestoreMetadataFieldsAndValidateParagraphSignatures();
}

SwEditShell::~SwEditShell() // USED
{
}

bool SwEditShell::IsModified() const
{
    return GetDoc()->getIDocumentState().IsModified();
}

void SwEditShell::SetModified()
{
    GetDoc()->getIDocumentState().SetModified();
}

void SwEditShell::ResetModified()
{
    GetDoc()->getIDocumentState().ResetModified();
}

void SwEditShell::SetUndoNoResetModified()
{
    GetDoc()->getIDocumentState().SetModified();
    GetDoc()->GetIDocumentUndoRedo().SetUndoNoResetModified();
}

void SwEditShell::CalcLayout()
{
    StartAllAction();
    SwViewShell::CalcLayout();

    for(SwViewShell& rCurrentShell : GetRingContainer())
    {
        if ( rCurrentShell.GetWin() )
            rCurrentShell.GetWin()->Invalidate();
    }

    EndAllAction();
}

/** Get the content type of a shell
 *
 * @todo Is this called for every attribute?
 */
sal_uInt16 SwEditShell::GetCntType() const
{
    sal_uInt16 nRet = 0;
    if( IsTableMode() )
        nRet = CNT_TXT;
    else
        switch( GetCursor()->GetPointNode().GetNodeType() )
        {
        case SwNodeType::Text:   nRet = CNT_TXT; break;
        case SwNodeType::Grf:    nRet = CNT_GRF; break;
        case SwNodeType::Ole:    nRet = CNT_OLE; break;
        default: break;
        }

    OSL_ASSERT( nRet );
    return nRet;
}

bool SwEditShell::HasOtherCnt() const

{
    if ( !GetDoc()->GetSpzFrameFormats()->empty() )
        return true;

    const SwNodes &rNds = GetDoc()->GetNodes();
    const SwNode *pNd;

    pNd = &rNds.GetEndOfInserts();
    if ( SwNodeOffset(1) != (pNd->GetIndex() - pNd->StartOfSectionIndex()) )
        return true;

    pNd = &rNds.GetEndOfAutotext();
    return SwNodeOffset(1) != (pNd->GetIndex() - pNd->StartOfSectionIndex());
}

SwActContext::SwActContext(SwEditShell *pShell)
    : m_rShell(*pShell)
{
    m_rShell.StartAction();
}

SwActContext::~SwActContext() COVERITY_NOEXCEPT_FALSE
{
    m_rShell.EndAction();
}

SwMvContext::SwMvContext(SwEditShell *pShell)
    : m_rShell(*pShell)
{
    m_rShell.SttCursorMove();
}

SwMvContext::~SwMvContext() COVERITY_NOEXCEPT_FALSE
{
    m_rShell.EndCursorMove();
}

SwFrameFormat *SwEditShell::GetTableFormat() // fastest test on a table
{
    const SwTableNode* pTableNd = IsCursorInTable();
    return pTableNd ? static_cast<SwFrameFormat*>(pTableNd->GetTable().GetFrameFormat()) : nullptr;
}

// TODO: Why is this called 3x for a new document?
sal_uInt16 SwEditShell::GetTOXTypeCount(TOXTypes eTyp) const
{
    return mxDoc->GetTOXTypeCount(eTyp);
}

void SwEditShell::InsertTOXType(const SwTOXType& rTyp)
{
    mxDoc->InsertTOXType(rTyp);
}

void SwEditShell::DoUndo( bool bOn )
{ GetDoc()->GetIDocumentUndoRedo().DoUndo( bOn ); }

bool SwEditShell::DoesUndo() const
{ return GetDoc()->GetIDocumentUndoRedo().DoesUndo(); }

void SwEditShell::DoGroupUndo( bool bOn )
{ GetDoc()->GetIDocumentUndoRedo().DoGroupUndo( bOn ); }

bool SwEditShell::DoesGroupUndo() const
{ return GetDoc()->GetIDocumentUndoRedo().DoesGroupUndo(); }

void SwEditShell::DelAllUndoObj()
{
    GetDoc()->GetIDocumentUndoRedo().DelAllUndoObj();
}

// Combine continuous calls of Insert/Delete/Overwrite on characters. Default: sdbcx::Group-Undo.

/** open undo container
 *
 * @return nUndoId ID of the container
 */
SwUndoId SwEditShell::StartUndo( SwUndoId eUndoId,
                                   const SwRewriter *pRewriter )
{ return GetDoc()->GetIDocumentUndoRedo().StartUndo( eUndoId, pRewriter ); }

/** close undo container
 *
 * not used by UI
 *
 * @param eUndoId   ID of the undo container
 * @param pRewriter ?
*/
SwUndoId SwEditShell::EndUndo(SwUndoId eUndoId, const SwRewriter *pRewriter)
{ return GetDoc()->GetIDocumentUndoRedo().EndUndo(eUndoId, pRewriter); }

bool     SwEditShell::GetLastUndoInfo(OUString *const o_pStr,
                                      SwUndoId *const o_pId,
                                      const SwView* pView) const
{
    return GetDoc()->GetIDocumentUndoRedo().GetLastUndoInfo(o_pStr, o_pId, pView);
}

bool SwEditShell::GetFirstRedoInfo(OUString *const o_pStr,
                                   SwUndoId *const o_pId,
                                   const SwView* pView) const
{
    return GetDoc()->GetIDocumentUndoRedo().GetFirstRedoInfo(o_pStr, o_pId, pView);
}

SwUndoId SwEditShell::GetRepeatInfo(OUString *const o_pStr) const
{ return GetDoc()->GetIDocumentUndoRedo().GetRepeatInfo(o_pStr); }

/** Auto correction */
void SwEditShell::AutoCorrect( SvxAutoCorrect& rACorr, bool bInsert,
                                sal_Unicode cChar )
{
    CurrShell aCurr( this );

    StartAllAction();

    SwPaM* pCursor = getShellCursor( true );
    SwTextNode* pTNd = pCursor->GetPointNode().GetTextNode();

    SwAutoCorrDoc aSwAutoCorrDoc( *this, *pCursor, cChar );
    // FIXME: this _must_ be called with reference to the actual node text!
    SwTextFrame const*const pFrame(static_cast<SwTextFrame const*>(pTNd->getLayoutFrame(GetLayout())));
    TextFrameIndex const nPos(pFrame->MapModelToViewPos(*pCursor->GetPoint()));
    // tdf#147414 sw_redlinehide: if cursor moved backward, it may be at the
    // start of a delete redline - but MapViewToModelPos() always returns end
    // of redline and it will be called when AutoCorrect actually inserts
    // something - so first normalize cursor point to end of redline so that
    // point will then be moved forward when something is inserted.
    *pCursor->GetPoint() = pFrame->MapViewToModelPos(nPos);
    // The hope is that the AutoCorrect never deletes nodes, hence never
    // deletes SwTextFrames, hence we can pass in the SwTextFrame::GetText()
    // result and it will be updated via the SwTextFrame::SwClientNotify()
    // on editing operations.
    OUString const& rMergedText(pFrame->GetText());
    rACorr.DoAutoCorrect( aSwAutoCorrDoc,
                    rMergedText, sal_Int32(nPos),
                    cChar, bInsert, m_bNbspRunNext, GetWin() );
    if( cChar )
        SaveTableBoxContent( pCursor->GetPoint() );
    EndAllAction();
}

void SwEditShell::SetNewDoc()
{
    GetDoc()->getIDocumentState().SetNewDoc(true);
}

OUString SwEditShell::GetPrevAutoCorrWord(SvxAutoCorrect& rACorr)
{
    CurrShell aCurr( this );

    OUString sRet;
    SwPaM* pCursor = getShellCursor( true );
    SwTextNode* pTNd = pCursor->GetPointNode().GetTextNode();
    if (pTNd)
    {
        SwAutoCorrDoc aSwAutoCorrDoc( *this, *pCursor, 0 );
        SwTextFrame const*const pFrame(static_cast<SwTextFrame const*>(pTNd->getLayoutFrame(GetLayout())));
        TextFrameIndex const nPos(pFrame->MapModelToViewPos(*pCursor->GetPoint()));
        sRet = rACorr.GetPrevAutoCorrWord(aSwAutoCorrDoc, pFrame->GetText(), sal_Int32(nPos));
    }
    return sRet;
}

std::vector<OUString> SwEditShell::GetChunkForAutoText()
{
    CurrShell aCurr(this);

    std::vector<OUString> aRet;
    SwPaM* pCursor = getShellCursor(true);
    SwTextNode* pTNd = pCursor->GetPointNode().GetTextNode();
    if (pTNd)
    {
        const auto pFrame = static_cast<SwTextFrame const*>(pTNd->getLayoutFrame(GetLayout()));
        TextFrameIndex const nPos(pFrame->MapModelToViewPos(*pCursor->GetPoint()));
        aRet = SvxAutoCorrect::GetChunkForAutoText(pFrame->GetText(), sal_Int32(nPos));
    }
    return aRet;
}

SwAutoCompleteWord& SwEditShell::GetAutoCompleteWords()
{
    return SwDoc::GetAutoCompleteWords();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
