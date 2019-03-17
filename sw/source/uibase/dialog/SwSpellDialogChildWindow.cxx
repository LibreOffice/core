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

#include <memory>
#include <SwSpellDialogChildWindow.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>
#include <editeng/svxacorr.hxx>
#include <editeng/acorrcfg.hxx>
#include <svx/svxids.hrc>
#include <sfx2/app.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <editeng/unolingu.hxx>
#include <editeng/editeng.hxx>
#include <editeng/editview.hxx>
#include <wrtsh.hxx>
#include <sfx2/printer.hxx>
#include <svx/svdoutl.hxx>
#include <svx/svdview.hxx>
#include <svx/svditer.hxx>
#include <svx/svdogrp.hxx>
#include <unotools/linguprops.hxx>
#include <unotools/lingucfg.hxx>
#include <doc.hxx>
#include <IDocumentDeviceAccess.hxx>
#include <IDocumentDrawModelAccess.hxx>
#include <docsh.hxx>
#include <docary.hxx>
#include <drawdoc.hxx>
#include <frmfmt.hxx>
#include <dcontact.hxx>
#include <edtwin.hxx>
#include <pam.hxx>
#include <drawbase.hxx>
#include <unotextrange.hxx>
#include <strings.hrc>
#include <cmdid.h>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::linguistic2;
using namespace ::com::sun::star::beans;

SFX_IMPL_CHILDWINDOW_WITHID(SwSpellDialogChildWindow, FN_SPELL_GRAMMAR_DIALOG)

struct SpellState
{
    bool                m_bInitialCall;
    bool                m_bLockFocus; // lock the focus notification while a modal dialog is active
    bool                m_bLostFocus;

    // restart and progress information
    bool                m_bBodySpelled;  // body already spelled
    bool                m_bOtherSpelled; // frames, footnotes, headers and footers spelled
    bool                m_bStartedInOther; // started the spelling inside of the _other_ area
    bool                m_bStartedInSelection; // there was an initial text selection
    std::unique_ptr<SwPaM>
                        pOtherCursor; // position where the spelling inside the _other_ area started
    bool                m_bDrawingsSpelled; // all drawings spelled
    Reference<XTextRange> m_xStartRange; // text range that marks the start of spelling
    const SdrObject*    m_pStartDrawing; // draw text object spelling started in
    ESelection          m_aStartDrawingSelection; // draw text start selection
    bool                m_bRestartDrawing; // the first selected drawing object is found again

    // lose/get focus information to decide if spelling can be continued
    ShellMode           m_eSelMode;
    const SwNode*       m_pPointNode;
    const SwNode*       m_pMarkNode;
    sal_Int32           m_nPointPos;
    sal_Int32           m_nMarkPos;
    const SdrOutliner*  m_pOutliner;
    ESelection          m_aESelection;

    // iterating over draw text objects
    std::list<SdrTextObj*> m_aTextObjects;
    bool                m_bTextObjectsCollected;

    SpellState() :
        m_bInitialCall(true),
        m_bLockFocus(false),
        m_bLostFocus(false),
        m_bBodySpelled(false),
        m_bOtherSpelled(false),
        m_bStartedInOther(false),
        m_bStartedInSelection(false),
        m_bDrawingsSpelled(false),
        m_pStartDrawing(nullptr),
        m_bRestartDrawing(false),

        m_eSelMode(ShellMode::Object), // initially invalid
        m_pPointNode(nullptr),
        m_pMarkNode(nullptr),
        m_nPointPos(0),
        m_nMarkPos(0),
        m_pOutliner(nullptr),
        m_bTextObjectsCollected(false)
        {}

    // reset state in ::InvalidateSpellDialog
    void    Reset()
            {   m_bInitialCall = true;
                m_bBodySpelled = m_bOtherSpelled = m_bDrawingsSpelled = false;
                m_xStartRange = nullptr;
                m_pStartDrawing = nullptr;
                m_bRestartDrawing = false;
                m_bTextObjectsCollected = false;
                m_aTextObjects.clear();
                m_bStartedInOther = false;
                pOtherCursor.reset();
            }
};

static void lcl_LeaveDrawText(SwWrtShell& rSh)
{
    if(rSh.GetDrawView())
    {
        rSh.GetDrawView()->SdrEndTextEdit( true );
        Point aPt(LONG_MIN, LONG_MIN);
        // go out of the frame
        rSh.SelectObj(aPt, SW_LEAVE_FRAME);
        rSh.EnterStdMode();
        rSh.GetView().AttrChangedNotify(&rSh);
    }
}

SwSpellDialogChildWindow::SwSpellDialogChildWindow (
            vcl::Window* _pParent,
            sal_uInt16 nId,
            SfxBindings* pBindings,
            SfxChildWinInfo* /*pInfo*/)
    : svx::SpellDialogChildWindow (
        _pParent, nId, pBindings)
    , m_bIsGrammarCheckingOn(false)
    , m_pSpellState(new SpellState)
{
    SvtLinguConfig().GetProperty( UPN_IS_GRAMMAR_INTERACTIVE ) >>= m_bIsGrammarCheckingOn;
}

SwSpellDialogChildWindow::~SwSpellDialogChildWindow ()
{
    SwWrtShell* pWrtShell = GetWrtShell_Impl();
    if(!m_pSpellState->m_bInitialCall && pWrtShell)
        pWrtShell->SpellEnd();
    m_pSpellState.reset();
}

SfxChildWinInfo SwSpellDialogChildWindow::GetInfo() const
{
    SfxChildWinInfo aInfo = svx::SpellDialogChildWindow::GetInfo();
    aInfo.bVisible = false;
    return aInfo;
}

svx::SpellPortions SwSpellDialogChildWindow::GetNextWrongSentence(bool bRecheck)
{
    svx::SpellPortions aRet;
    SwWrtShell* pWrtShell = GetWrtShell_Impl();
    if(pWrtShell)
    {
        if (!bRecheck)
        {
            // first set continuation point for spell/grammar check to the
            // end of the current sentence
            SwEditShell::MoveContinuationPosToEndOfCheckedSentence();
        }

        ShellMode eSelMode = pWrtShell->GetView().GetShellMode();
        bool bDrawText = ShellMode::DrawText == eSelMode;
        bool bNormalText =
            ShellMode::TableText == eSelMode ||
            ShellMode::ListText == eSelMode ||
            ShellMode::TableListText == eSelMode ||
            ShellMode::Text == eSelMode;
        // Writer text outside of the body
        bool bOtherText = false;

        if( m_pSpellState->m_bInitialCall )
        {
            // if no text selection exists the cursor has to be set into the text
            if(!bDrawText && !bNormalText)
            {
                MakeTextSelection_Impl(*pWrtShell, eSelMode);
                // the selection type has to be checked again - both text types are possible
                if(pWrtShell->GetSelectionType() & SelectionType::DrawObjectEditMode)
                    bDrawText = true;
                bNormalText = !bDrawText;
            }
            if(bNormalText)
            {
                // set cursor to the start of the sentence
                if(!pWrtShell->HasSelection())
                    pWrtShell->GoStartSentence();
                else
                {
                    pWrtShell->ExpandToSentenceBorders();
                    m_pSpellState->m_bStartedInSelection = true;
                }
                // determine if the selection is outside of the body text
                bOtherText = !(pWrtShell->GetFrameType(nullptr,true) & FrameTypeFlags::BODY);
                if(bOtherText)
                {
                    m_pSpellState->pOtherCursor.reset( new SwPaM(*pWrtShell->GetCursor()->GetPoint()) );
                    m_pSpellState->m_bStartedInOther = true;
                    pWrtShell->SpellStart( SwDocPositions::OtherStart, SwDocPositions::OtherEnd, SwDocPositions::Curr );
                }
                else
                {
                    // mark the start position only if not at start of doc
                    if(!pWrtShell->IsStartOfDoc())
                    {
                        // Record the position *before* the current cursor, as
                        // the word at the current cursor can possibly be
                        // replaced by a spellcheck correction which invalidates
                        // an XTextRange at this position.
                        SwDoc *pDoc = pWrtShell->GetDoc();
                        auto pStart = pWrtShell->GetCursor()->Start();
                        auto pUnoCursor = pDoc->CreateUnoCursor(*pStart);
                        pUnoCursor->Left( 1 );
                        pStart = pUnoCursor->Start();
                        m_pSpellState->m_xStartRange
                            = SwXTextRange::CreateXTextRange(*pDoc, *pStart, nullptr);
                    }
                    pWrtShell->SpellStart( SwDocPositions::Start, SwDocPositions::End, SwDocPositions::Curr );
                }
            }
            else
            {
                SdrView* pSdrView = pWrtShell->GetDrawView();
                m_pSpellState->m_pStartDrawing = pSdrView->GetMarkedObjectList().GetMark(0)->GetMarkedSdrObj();
                OutlinerView* pOLV = pSdrView->GetTextEditOutlinerView();
                // start checking at the top of the drawing object
                pOLV->SetSelection( ESelection() );
                m_pSpellState->m_aStartDrawingSelection = ESelection();
/*
Note: spelling in a selection only, or starting in a mid of a drawing object requires
further changes elsewhere. (Especially if it should work in sc and sd as well.)
The code below would only be part of the solution.
(Keeping it a as a comment for the time being)
                ESelection aCurSel( pOLV->GetSelection() );
                ESelection aSentenceSel( pOLV->GetEditView().GetEditEngine()->SelectSentence( aCurSel ) );
                if (!aCurSel.HasRange())
                {
                    aSentenceSel.nEndPara = aSentenceSel.nStartPara;
                    aSentenceSel.nEndPos  = aSentenceSel.nStartPos;
                }
                pOLV->SetSelection( aSentenceSel );
                m_pSpellState->m_aStartDrawingSelection = aSentenceSel;
*/
            }

            m_pSpellState->m_bInitialCall = false;
        }
        if( bDrawText )
        {
            // spell inside of the current draw text
            if(!SpellDrawText_Impl(*pWrtShell, aRet))
            {
                if(!FindNextDrawTextError_Impl(*pWrtShell) || !SpellDrawText_Impl(*pWrtShell, aRet))
                {
                    lcl_LeaveDrawText(*pWrtShell);
                    // now the drawings have been spelled
                    m_pSpellState->m_bDrawingsSpelled = true;
                    // the spelling continues at the other content
                    // if there's any that has not been spelled yet
                    if(!m_pSpellState->m_bOtherSpelled && pWrtShell->HasOtherCnt())
                    {
                        pWrtShell->SpellStart(SwDocPositions::OtherStart, SwDocPositions::OtherEnd, SwDocPositions::OtherStart );
                        if(!pWrtShell->SpellSentence(aRet, m_bIsGrammarCheckingOn))
                        {
                            pWrtShell->SpellEnd();
                            m_pSpellState->m_bOtherSpelled = true;
                        }
                    }
                    else
                        m_pSpellState->m_bOtherSpelled = true;
                    // if no result has been found try at the body text - completely
                    if(!m_pSpellState->m_bBodySpelled && aRet.empty())
                    {
                        pWrtShell->SpellStart(SwDocPositions::Start, SwDocPositions::End, SwDocPositions::Start );
                        if(!pWrtShell->SpellSentence(aRet, m_bIsGrammarCheckingOn))
                        {
                            m_pSpellState->m_bBodySpelled = true;
                            pWrtShell->SpellEnd();
                        }
                    }

                }
            }
        }
        else
        {
            // spell inside of the Writer text
            if(!pWrtShell->SpellSentence(aRet, m_bIsGrammarCheckingOn))
            {
                // if there is a selection (within body or header/footer text)
                // then spell/grammar checking should not move outside of it.
                if (!m_pSpellState->m_bStartedInSelection)
                {
                    // find out which text has been spelled body or other
                    bOtherText = !(pWrtShell->GetFrameType(nullptr,true) & FrameTypeFlags::BODY);
                    if(bOtherText && m_pSpellState->m_bStartedInOther && m_pSpellState->pOtherCursor)
                    {
                        m_pSpellState->m_bStartedInOther = false;
                        pWrtShell->SetSelection(*m_pSpellState->pOtherCursor);
                        pWrtShell->SpellEnd();
                        m_pSpellState->pOtherCursor.reset();
                        pWrtShell->SpellStart(SwDocPositions::OtherStart, SwDocPositions::Curr, SwDocPositions::OtherStart );
                        (void)pWrtShell->SpellSentence(aRet, m_bIsGrammarCheckingOn);
                    }
                    if(aRet.empty())
                    {
                        // end spelling
                        pWrtShell->SpellEnd();
                        if(bOtherText)
                        {
                            m_pSpellState->m_bOtherSpelled = true;
                            // has the body been spelled?
                            if(!m_pSpellState->m_bBodySpelled)
                            {
                                pWrtShell->SpellStart(SwDocPositions::Start, SwDocPositions::End, SwDocPositions::Start );
                                if(!pWrtShell->SpellSentence(aRet, m_bIsGrammarCheckingOn))
                                {
                                    m_pSpellState->m_bBodySpelled = true;
                                    pWrtShell->SpellEnd();
                                }
                            }
                        }
                        else
                        {
                             m_pSpellState->m_bBodySpelled = true;
                             if(!m_pSpellState->m_bOtherSpelled && pWrtShell->HasOtherCnt())
                             {
                                pWrtShell->SpellStart(SwDocPositions::OtherStart, SwDocPositions::OtherEnd, SwDocPositions::OtherStart );
                                if(!pWrtShell->SpellSentence(aRet, m_bIsGrammarCheckingOn))
                                {
                                    pWrtShell->SpellEnd();
                                    m_pSpellState->m_bOtherSpelled = true;
                                }
                             }
                             else
                                 m_pSpellState->m_bOtherSpelled = true;
                        }
                    }

                    // search for a draw text object that contains error and spell it
                    if(aRet.empty() &&
                            (m_pSpellState->m_bDrawingsSpelled ||
                            !FindNextDrawTextError_Impl(*pWrtShell) || !SpellDrawText_Impl(*pWrtShell, aRet)))
                    {
                        lcl_LeaveDrawText(*pWrtShell);
                        m_pSpellState->m_bDrawingsSpelled = true;
                    }
                }
            }
        }
        // now only the rest of the body text can be spelled -
        // if the spelling started inside of the body
        bool bCloseMessage = true;
        if(aRet.empty() && !m_pSpellState->m_bStartedInSelection)
        {
            OSL_ENSURE(m_pSpellState->m_bDrawingsSpelled &&
                        m_pSpellState->m_bOtherSpelled && m_pSpellState->m_bBodySpelled,
                        "not all parts of the document are already spelled");
            if(m_pSpellState->m_xStartRange.is())
            {
                LockFocusNotification( true );
                std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(GetWindow()->GetFrameWeld(),
                                                                                        VclMessageType::Question, VclButtonsType::YesNo, SwResId(STR_QUERY_SPELL_CONTINUE)));
                sal_uInt16 nRet = xBox->run();
                if (RET_YES == nRet)
                {
                    SwUnoInternalPaM aPam(*pWrtShell->GetDoc());
                    if (::sw::XTextRangeToSwPaM(aPam,
                                m_pSpellState->m_xStartRange))
                    {
                        pWrtShell->SetSelection(aPam);
                        pWrtShell->SpellStart(SwDocPositions::Start, SwDocPositions::Curr, SwDocPositions::Start);
                        if(!pWrtShell->SpellSentence(aRet, m_bIsGrammarCheckingOn))
                            pWrtShell->SpellEnd();
                    }
                    m_pSpellState->m_xStartRange = nullptr;
                    LockFocusNotification( false );
                    // take care that the now valid selection is stored
                    LoseFocus();
                }
                else
                    bCloseMessage = false; // no closing message if a wrap around has been denied
            }
        }
        bool bNoDictionaryAvailable = pWrtShell->GetDoc()->IsDictionaryMissing();
        if( aRet.empty() && bCloseMessage )
        {
            LockFocusNotification( true );
            OUString sInfo( SwResId( bNoDictionaryAvailable ? STR_DICTIONARY_UNAVAILABLE : STR_SPELLING_COMPLETED ) );
            vcl::Window* pThisWindow = GetWindow();
            // #i84610#
            std::unique_ptr<weld::MessageDialog> xBox(
                Application::CreateMessageDialog( pThisWindow->GetFrameWeld(),
                                                  VclMessageType::Info,
                                                  VclButtonsType::Ok,
                                                  sInfo ) );
            xBox->run();
            LockFocusNotification( false );
            // take care that the now valid selection is stored
            LoseFocus();
            if( pThisWindow )
                pThisWindow->GrabFocus();
        }
    }
    return aRet;
}

void SwSpellDialogChildWindow::ApplyChangedSentence(const svx::SpellPortions& rChanged, bool bRecheck)
{
    SwWrtShell* pWrtShell = GetWrtShell_Impl();
    OSL_ENSURE(!m_pSpellState->m_bInitialCall, "ApplyChangedSentence in initial call or after resume");
    if(pWrtShell && !m_pSpellState->m_bInitialCall)
    {
        ShellMode eSelMode = pWrtShell->GetView().GetShellMode();
        bool bDrawText = ShellMode::DrawText == eSelMode;
        bool bNormalText =
            ShellMode::TableText == eSelMode ||
            ShellMode::ListText == eSelMode ||
            ShellMode::TableListText == eSelMode ||
            ShellMode::Text == eSelMode;

        // evaluate if the same sentence should be rechecked or not.
        // Sentences that got grammar checked should always be rechecked in order
        // to detect possible errors that get introduced with the changes
        bRecheck |= SwEditShell::HasLastSentenceGotGrammarChecked();

        if(bNormalText)
            pWrtShell->ApplyChangedSentence(rChanged, bRecheck);
        else if(bDrawText )
        {
            SdrView* pDrView = pWrtShell->GetDrawView();
            SdrOutliner *pOutliner = pDrView->GetTextEditOutliner();
            pOutliner->ApplyChangedSentence(pDrView->GetTextEditOutlinerView()->GetEditView(), rChanged, bRecheck);
        }
    }
}

void SwSpellDialogChildWindow::AddAutoCorrection(
        const OUString& rOld, const OUString& rNew, LanguageType eLanguage)
{
    SvxAutoCorrect* pACorr = SvxAutoCorrCfg::Get().GetAutoCorrect();
    pACorr->PutText( rOld, rNew, eLanguage );
}

bool SwSpellDialogChildWindow::HasAutoCorrection()
{
    return true;
}

bool SwSpellDialogChildWindow::HasGrammarChecking()
{
    return SvtLinguConfig().HasGrammarChecker();
}

bool SwSpellDialogChildWindow::IsGrammarChecking()
{
    return m_bIsGrammarCheckingOn;
}

void SwSpellDialogChildWindow::SetGrammarChecking(bool bOn)
{
    uno::Any aVal;
    aVal <<= bOn;
    m_bIsGrammarCheckingOn = bOn;
    SvtLinguConfig().SetProperty( UPN_IS_GRAMMAR_INTERACTIVE, aVal );
    // set current spell position to the start of the current sentence to
    // continue with this sentence after grammar checking state has been changed
    SwWrtShell* pWrtShell = GetWrtShell_Impl();
    if(pWrtShell)
    {
        ShellMode eSelMode = pWrtShell->GetView().GetShellMode();
        bool bDrawText = ShellMode::DrawText == eSelMode;
        bool bNormalText =
            ShellMode::TableText == eSelMode ||
            ShellMode::ListText == eSelMode ||
            ShellMode::TableListText == eSelMode ||
            ShellMode::Text == eSelMode;
        if( bNormalText )
            SwEditShell::PutSpellingToSentenceStart();
        else if( bDrawText )
        {
            SdrView*     pSdrView = pWrtShell->GetDrawView();
            SdrOutliner* pOutliner = pSdrView ? pSdrView->GetTextEditOutliner() : nullptr;
            OSL_ENSURE(pOutliner, "No Outliner in SwSpellDialogChildWindow::SetGrammarChecking");
            if(pOutliner)
            {
                pOutliner->PutSpellingToSentenceStart( pSdrView->GetTextEditOutlinerView()->GetEditView() );
            }
        }
    }
}

void SwSpellDialogChildWindow::GetFocus()
{
    if(m_pSpellState->m_bLockFocus)
        return;
    bool bInvalidate = false;
    SwWrtShell* pWrtShell = GetWrtShell_Impl();
    if(pWrtShell && !m_pSpellState->m_bInitialCall)
    {
        ShellMode eSelMode = pWrtShell->GetView().GetShellMode();
        if(eSelMode != m_pSpellState->m_eSelMode)
        {
            // prevent initial invalidation
            if(m_pSpellState->m_bLostFocus)
                bInvalidate = true;
        }
        else
        {
            switch(m_pSpellState->m_eSelMode)
            {
                case ShellMode::Text:
                case ShellMode::ListText:
                case ShellMode::TableText:
                case ShellMode::TableListText:
                {
                    SwPaM* pCursor = pWrtShell->GetCursor();
                    if(m_pSpellState->m_pPointNode != &pCursor->GetNode() ||
                        m_pSpellState->m_pMarkNode != &pCursor->GetNode(false)||
                        m_pSpellState->m_nPointPos != pCursor->GetPoint()->nContent.GetIndex()||
                        m_pSpellState->m_nMarkPos != pCursor->GetMark()->nContent.GetIndex())
                            bInvalidate = true;
                }
                break;
                case ShellMode::DrawText:
                {
                    SdrView*     pSdrView = pWrtShell->GetDrawView();
                    SdrOutliner* pOutliner = pSdrView ? pSdrView->GetTextEditOutliner() : nullptr;
                    if(!pOutliner || m_pSpellState->m_pOutliner != pOutliner)
                        bInvalidate = true;
                    else
                    {
                        OutlinerView* pOLV = pSdrView->GetTextEditOutlinerView();
                        OSL_ENSURE(pOLV, "no OutlinerView in SwSpellDialogChildWindow::GetFocus()");
                        if(!pOLV || m_pSpellState->m_aESelection != pOLV->GetSelection())
                            bInvalidate = true;
                    }
                }
                break;
                default: bInvalidate = true;
            }
        }
    }
    else
    {
        bInvalidate = true;
    }
    if(bInvalidate)
        InvalidateSpellDialog();
}

void SwSpellDialogChildWindow::LoseFocus()
{
    // prevent initial invalidation
    m_pSpellState->m_bLostFocus = true;
    if(m_pSpellState->m_bLockFocus)
        return;
    SwWrtShell* pWrtShell = GetWrtShell_Impl();
    if(pWrtShell)
    {
        m_pSpellState->m_eSelMode = pWrtShell->GetView().GetShellMode();
        m_pSpellState->m_pPointNode = m_pSpellState->m_pMarkNode = nullptr;
        m_pSpellState->m_nPointPos = m_pSpellState->m_nMarkPos = 0;
        m_pSpellState->m_pOutliner = nullptr;

        switch(m_pSpellState->m_eSelMode)
        {
            case ShellMode::Text:
            case ShellMode::ListText:
            case ShellMode::TableText:
            case ShellMode::TableListText:
            {
                // store a node pointer and a pam-position to be able to check on next GetFocus();
                SwPaM* pCursor = pWrtShell->GetCursor();
                m_pSpellState->m_pPointNode = &pCursor->GetNode();
                m_pSpellState->m_pMarkNode = &pCursor->GetNode(false);
                m_pSpellState->m_nPointPos = pCursor->GetPoint()->nContent.GetIndex();
                m_pSpellState->m_nMarkPos = pCursor->GetMark()->nContent.GetIndex();

            }
            break;
            case ShellMode::DrawText:
            {
                SdrView*     pSdrView = pWrtShell->GetDrawView();
                SdrOutliner* pOutliner = pSdrView->GetTextEditOutliner();
                m_pSpellState->m_pOutliner = pOutliner;
                OutlinerView* pOLV = pSdrView->GetTextEditOutlinerView();
                OSL_ENSURE(pOutliner && pOLV, "no Outliner/OutlinerView in SwSpellDialogChildWindow::LoseFocus()");
                if(pOLV)
                {
                    m_pSpellState->m_aESelection = pOLV->GetSelection();
                }
            }
            break;
            default:;// prevent warning
        }
    }
    else
        m_pSpellState->m_eSelMode = ShellMode::Object;
}

void SwSpellDialogChildWindow::InvalidateSpellDialog()
{
    SwWrtShell* pWrtShell = GetWrtShell_Impl();
    if(!m_pSpellState->m_bInitialCall && pWrtShell)
        pWrtShell->SpellEnd(nullptr, false);
    m_pSpellState->Reset();
    svx::SpellDialogChildWindow::InvalidateSpellDialog();
}

SwWrtShell* SwSpellDialogChildWindow::GetWrtShell_Impl()
{
    SfxDispatcher* pDispatch = GetBindings().GetDispatcher();
    SwView* pView = nullptr;
    if(pDispatch)
    {
        sal_uInt16 nShellIdx = 0;
        SfxShell* pShell;
        while(nullptr != (pShell = pDispatch->GetShell(nShellIdx++)))
            if(dynamic_cast< const SwView *>( pShell ) !=  nullptr)
            {
                pView = static_cast<SwView* >(pShell);
                break;
            }
    }
    return pView ? pView->GetWrtShellPtr(): nullptr;
}

// set the cursor into the body text - necessary if any object is selected
// on start of the spelling dialog
void SwSpellDialogChildWindow::MakeTextSelection_Impl(SwWrtShell& rShell, ShellMode eSelMode)
{
    SwView& rView = rShell.GetView();
    switch(eSelMode)
    {
        case ShellMode::Text:
        case ShellMode::ListText:
        case ShellMode::TableText:
        case ShellMode::TableListText:
        case ShellMode::DrawText:
            OSL_FAIL("text already active in SwSpellDialogChildWindow::MakeTextSelection_Impl()");
        break;

        case ShellMode::Frame:
        {
            rShell.UnSelectFrame();
            rShell.LeaveSelFrameMode();
            rView.AttrChangedNotify(&rShell);
        }
        break;

        case ShellMode::Draw:
        case ShellMode::DrawForm:
        case ShellMode::Bezier:
            if(FindNextDrawTextError_Impl(rShell))
            {
                rView.AttrChangedNotify(&rShell);
                break;
            }
            [[fallthrough]]; // to deselect the object
        case ShellMode::Graphic:
        case ShellMode::Object:
        {
            if ( rShell.IsDrawCreate() )
            {
                rView.GetDrawFuncPtr()->BreakCreate();
                rView.AttrChangedNotify(&rShell);
            }
            else if ( rShell.HasSelection() || rView.IsDrawMode() )
            {
                SdrView *pSdrView = rShell.GetDrawView();
                if(pSdrView && pSdrView->AreObjectsMarked() &&
                    pSdrView->GetHdlList().GetFocusHdl())
                {
                    const_cast<SdrHdlList&>(pSdrView->GetHdlList()).ResetFocusHdl();
                }
                else
                {
                    rView.LeaveDrawCreate();
                    Point aPt(LONG_MIN, LONG_MIN);
                    // go out of the frame
                    rShell.SelectObj(aPt, SW_LEAVE_FRAME);
                    SfxBindings& rBind = rView.GetViewFrame()->GetBindings();
                    rBind.Invalidate( SID_ATTR_SIZE );
                    rShell.EnterStdMode();
                    rView.AttrChangedNotify(&rShell);
                }
            }
        }
        break;
        default:; // prevent warning
    }
}

// select the next draw text object that has a spelling error
bool SwSpellDialogChildWindow::FindNextDrawTextError_Impl(SwWrtShell& rSh)
{
    bool bNextDoc = false;
    SdrView* pDrView = rSh.GetDrawView();
    if(!pDrView)
        return bNextDoc;
    SwView& rView = rSh.GetView();
    SwDoc* pDoc = rView.GetDocShell()->GetDoc();
    const SdrMarkList& rMarkList = pDrView->GetMarkedObjectList();
    // start at the current draw object - if there is any selected
    SdrTextObj* pCurrentTextObj = nullptr;
    if ( rMarkList.GetMarkCount() == 1 )
    {
        SdrObject* pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();
        if( auto pSdrTextObj = dynamic_cast<SdrTextObj *>( pObj ) )
            pCurrentTextObj = pSdrTextObj;
    }
    // at first fill the list of drawing objects
    if(!m_pSpellState->m_bTextObjectsCollected )
    {
        m_pSpellState->m_bTextObjectsCollected = true;
        SwDrawContact::GetTextObjectsFromFormat( m_pSpellState->m_aTextObjects, pDoc );
        if(pCurrentTextObj)
        {
            m_pSpellState->m_aTextObjects.remove(pCurrentTextObj);
            m_pSpellState->m_aTextObjects.push_back(pCurrentTextObj);
        }
    }
    if(!m_pSpellState->m_aTextObjects.empty())
    {
        Reference< XSpellChecker1 >  xSpell( GetSpellChecker() );
        while(!bNextDoc && !m_pSpellState->m_aTextObjects.empty())
        {
            std::list<SdrTextObj*>::iterator aStart = m_pSpellState->m_aTextObjects.begin();
            SdrTextObj* pTextObj = *aStart;
            if(m_pSpellState->m_pStartDrawing == pTextObj)
                m_pSpellState->m_bRestartDrawing = true;
            m_pSpellState->m_aTextObjects.erase(aStart);
            OutlinerParaObject* pParaObj = pTextObj->GetOutlinerParaObject();
            if ( pParaObj )
            {
                bool bHasSpellError = false;
                {
                    SdrOutliner aTmpOutliner(pDoc->getIDocumentDrawModelAccess().GetDrawModel()->
                                             GetDrawOutliner().GetEmptyItemSet().GetPool(),
                                                OutlinerMode::TextObject );
                    aTmpOutliner.SetRefDevice( pDoc->getIDocumentDeviceAccess().getPrinter( false ) );
                    MapMode aMapMode (MapUnit::MapTwip);
                    aTmpOutliner.SetRefMapMode(aMapMode);
                    aTmpOutliner.SetPaperSize( pTextObj->GetLogicRect().GetSize() );
                    aTmpOutliner.SetSpeller( xSpell );

                    std::unique_ptr<OutlinerView> pOutlView( new OutlinerView( &aTmpOutliner, &(rView.GetEditWin()) ) );
                    pOutlView->GetOutliner()->SetRefDevice( rSh.getIDocumentDeviceAccess().getPrinter( false ) );
                    aTmpOutliner.InsertView( pOutlView.get() );
                    Size aSize(1,1);
                    tools::Rectangle aRect( Point(), aSize );
                    pOutlView->SetOutputArea( aRect );
                    aTmpOutliner.SetText( *pParaObj );
                    aTmpOutliner.ClearModifyFlag();
                    bHasSpellError = EESpellState::Ok != aTmpOutliner.HasSpellErrors();
                    aTmpOutliner.RemoveView( pOutlView.get() );
                }
                if(bHasSpellError)
                {
                    // now the current one has to be deselected
                    if(pCurrentTextObj)
                        pDrView->SdrEndTextEdit( true );
                    // and the found one should be activated
                    rSh.MakeVisible(pTextObj->GetLogicRect());
                    Point aTmp( 0,0 );
                    rSh.SelectObj( aTmp, 0, pTextObj );
                    SdrPageView* pPV = pDrView->GetSdrPageView();
                    rView.BeginTextEdit( pTextObj, pPV, &rView.GetEditWin(), false, true );
                    rView.AttrChangedNotify(&rSh);
                    bNextDoc = true;
                }
            }
        }
    }
    return bNextDoc;
}

bool SwSpellDialogChildWindow::SpellDrawText_Impl(SwWrtShell& rSh, svx::SpellPortions& rPortions)
{
    bool bRet = false;
    SdrView*     pSdrView = rSh.GetDrawView();
    SdrOutliner* pOutliner = pSdrView ? pSdrView->GetTextEditOutliner() : nullptr;
    OSL_ENSURE(pOutliner, "No Outliner in SwSpellDialogChildWindow::SpellDrawText_Impl");
    if(pOutliner)
    {
        bRet = pOutliner->SpellSentence(pSdrView->GetTextEditOutlinerView()->GetEditView(), rPortions);
        // find out if the current selection is in the first spelled drawing object
        // and behind the initial selection
        if(bRet && m_pSpellState->m_bRestartDrawing)
        {
            OutlinerView* pOLV = pSdrView->GetTextEditOutlinerView();
            ESelection aCurrentSelection = pOLV->GetSelection();
            if(m_pSpellState->m_aStartDrawingSelection.nEndPara < aCurrentSelection.nEndPara ||
               (m_pSpellState->m_aStartDrawingSelection.nEndPara ==  aCurrentSelection.nEndPara &&
                    m_pSpellState->m_aStartDrawingSelection.nEndPos <  aCurrentSelection.nEndPos))
            {
                bRet = false;
                rPortions.clear();
            }
        }
    }
    return bRet;
}

void SwSpellDialogChildWindow::LockFocusNotification(bool bLock)
{
    OSL_ENSURE(m_pSpellState->m_bLockFocus != bLock, "invalid locking - no change of state");
    m_pSpellState->m_bLockFocus = bLock;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
