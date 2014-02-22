/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */


#include <SwSpellDialogChildWindow.hxx>
#include <vcl/msgbox.hxx>
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
#include <docsh.hxx>
#include <docary.hxx>
#include <frmfmt.hxx>
#include <dcontact.hxx>
#include <edtwin.hxx>
#include <pam.hxx>
#include <drawbase.hxx>
#include <unotextrange.hxx>
#include <dialog.hrc>
#include <cmdid.h>


using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::linguistic2;
using namespace ::com::sun::star::beans;

SFX_IMPL_CHILDWINDOW_WITHID(SwSpellDialogChildWindow, FN_SPELL_GRAMMAR_DIALOG)


#define SPELL_START_BODY        0   
#define SPELL_START_OTHER       1   
#define SPELL_START_DRAWTEXT    2   

struct SpellState
{
    bool                m_bInitialCall;
    bool                m_bLockFocus; 
    bool                m_bLostFocus;

    
    sal_uInt16              m_SpellStartPosition;
    bool                m_bBodySpelled;  
    bool                m_bOtherSpelled; 
    bool                m_bStartedInOther; 
    bool                m_bStartedInSelection; 
    SwPaM*              pOtherCursor; 
    bool                m_bDrawingsSpelled; 
    Reference<XTextRange> m_xStartRange; 
    const SdrObject*    m_pStartDrawing; 
    ESelection          m_aStartDrawingSelection; 
    bool                m_bRestartDrawing; 

    
    ShellModes          m_eSelMode;
    const SwNode*       m_pPointNode;
    const SwNode*       m_pMarkNode;
    sal_Int32           m_nPointPos;
    sal_Int32           m_nMarkPos;
    const SdrOutliner*  m_pOutliner;
    ESelection          m_aESelection;

    
    std::list<SdrTextObj*> m_aTextObjects;
    bool                m_bTextObjectsCollected;

    SpellState() :
        m_bInitialCall(true),
        m_bLockFocus(false),
        m_bLostFocus(false),
        m_SpellStartPosition(SPELL_START_BODY),
        m_bBodySpelled(false),
        m_bOtherSpelled(false),
        m_bStartedInOther(false),
        m_bStartedInSelection(false),
        pOtherCursor(0),
        m_bDrawingsSpelled(false),
        m_pStartDrawing(0),
        m_bRestartDrawing(false),

        m_eSelMode(SHELL_MODE_OBJECT), 
        m_pPointNode(0),
        m_pMarkNode(0),
        m_nPointPos(0),
        m_nMarkPos(0),
        m_pOutliner(0),
        m_bTextObjectsCollected(false)
        {}

    ~SpellState() {delete pOtherCursor;}

    
    void    Reset()
            {   m_bInitialCall = true;
                m_bBodySpelled = m_bOtherSpelled = m_bDrawingsSpelled = false;
                m_xStartRange = 0;
                m_pStartDrawing = 0;
                m_bRestartDrawing = false;
                m_bTextObjectsCollected = false;
                m_aTextObjects.clear();
                m_bStartedInOther = false;
                delete pOtherCursor;
                pOtherCursor = 0;
            }
};

static void lcl_LeaveDrawText(SwWrtShell& rSh)
{
    if(rSh.GetDrawView())
    {
        rSh.GetDrawView()->SdrEndTextEdit( sal_True );
        Point aPt(LONG_MIN, LONG_MIN);
        
        rSh.SelectObj(aPt, SW_LEAVE_FRAME);
        rSh.EnterStdMode();
        rSh.GetView().AttrChangedNotify(&rSh);
    }
}

SwSpellDialogChildWindow::SwSpellDialogChildWindow (
            Window* _pParent,
            sal_uInt16 nId,
            SfxBindings* pBindings,
            SfxChildWinInfo* pInfo) :
                svx::SpellDialogChildWindow (
                    _pParent, nId, pBindings, pInfo),
                    m_pSpellState(new SpellState)
{
    OUString aPropName(UPN_IS_GRAMMAR_INTERACTIVE);
    SvtLinguConfig().GetProperty( aPropName ) >>= m_bIsGrammarCheckingOn;
}

SwSpellDialogChildWindow::~SwSpellDialogChildWindow ()
{
    SwWrtShell* pWrtShell = GetWrtShell_Impl();
    if(!m_pSpellState->m_bInitialCall && pWrtShell)
        pWrtShell->SpellEnd();
    delete m_pSpellState;
}


SfxChildWinInfo SwSpellDialogChildWindow::GetInfo (void) const
{
    SfxChildWinInfo aInfo = svx::SpellDialogChildWindow::GetInfo();
    aInfo.bVisible = sal_False;
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
            
            
            pWrtShell->MoveContinuationPosToEndOfCheckedSentence();
        }

        ShellModes  eSelMode = pWrtShell->GetView().GetShellMode();
        bool bDrawText = SHELL_MODE_DRAWTEXT == eSelMode;
        bool bNormalText =
            SHELL_MODE_TABLE_TEXT == eSelMode ||
            SHELL_MODE_LIST_TEXT == eSelMode ||
            SHELL_MODE_TABLE_LIST_TEXT == eSelMode ||
            SHELL_MODE_TEXT == eSelMode;
        
        bool bOtherText = false;

        if( m_pSpellState->m_bInitialCall )
        {
            
            if(!bDrawText && !bNormalText)
            {
                if(!MakeTextSelection_Impl(*pWrtShell, eSelMode))
                    return aRet;
                else
                {
                    
                    if(0 != (pWrtShell->GetSelectionType()& nsSelectionType::SEL_DRW_TXT))
                        bDrawText = true;
                    bNormalText = !bDrawText;
                }
            }
            if(bNormalText)
            {
                
                if(!pWrtShell->HasSelection())
                    pWrtShell->GoStartSentence();
                else
                {
                    pWrtShell->ExpandToSentenceBorders();
                    m_pSpellState->m_bStartedInSelection = true;
                }
                
                bOtherText = !(pWrtShell->GetFrmType(0,sal_True) & FRMTYPE_BODY);
                m_pSpellState->m_SpellStartPosition = bOtherText ? SPELL_START_OTHER : SPELL_START_BODY;
                if(bOtherText)
                {
                    m_pSpellState->pOtherCursor = new SwPaM(*pWrtShell->GetCrsr()->GetPoint());
                    m_pSpellState->m_bStartedInOther = true;
                    pWrtShell->SpellStart( DOCPOS_OTHERSTART, DOCPOS_OTHEREND, DOCPOS_CURR );
                }
                else
                {
                    SwPaM* pCrsr = pWrtShell->GetCrsr();
                    
                    if(!pWrtShell->IsStartOfDoc())
                    {
                        m_pSpellState->m_xStartRange =
                            SwXTextRange::CreateXTextRange(
                                *pWrtShell->GetDoc(),
                                *pCrsr->Start(), pCrsr->End());
                    }
                    pWrtShell->SpellStart( DOCPOS_START, DOCPOS_END, DOCPOS_CURR );
                }
            }
            else
            {
                SdrView* pSdrView = pWrtShell->GetDrawView();
                m_pSpellState->m_SpellStartPosition = SPELL_START_DRAWTEXT;
                m_pSpellState->m_pStartDrawing = pSdrView->GetMarkedObjectList().GetMark(0)->GetMarkedSdrObj();
                OutlinerView* pOLV = pSdrView->GetTextEditOutlinerView();
                
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
            
            if(!SpellDrawText_Impl(*pWrtShell, aRet))
            {
                if(!FindNextDrawTextError_Impl(*pWrtShell) || !SpellDrawText_Impl(*pWrtShell, aRet))
                {
                    lcl_LeaveDrawText(*pWrtShell);
                    
                    m_pSpellState->m_bDrawingsSpelled = true;
                    
                    
                    if(!m_pSpellState->m_bOtherSpelled && pWrtShell->HasOtherCnt())
                    {
                        pWrtShell->SpellStart(DOCPOS_OTHERSTART, DOCPOS_OTHEREND, DOCPOS_OTHERSTART );
                        if(!pWrtShell->SpellSentence(aRet, m_bIsGrammarCheckingOn))
                        {
                            pWrtShell->SpellEnd();
                            m_pSpellState->m_bOtherSpelled = true;
                        }
                    }
                    else
                        m_pSpellState->m_bOtherSpelled = true;
                    
                    if(!m_pSpellState->m_bBodySpelled && !aRet.size())
                    {
                        pWrtShell->SpellStart(DOCPOS_START, DOCPOS_END, DOCPOS_START );
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
            
            if(!pWrtShell->SpellSentence(aRet, m_bIsGrammarCheckingOn))
            {
                
                
                if (!m_pSpellState->m_bStartedInSelection)
                {
                    
                    bOtherText = !(pWrtShell->GetFrmType(0,sal_True) & FRMTYPE_BODY);
                    if(bOtherText && m_pSpellState->m_bStartedInOther && m_pSpellState->pOtherCursor)
                    {
                        m_pSpellState->m_bStartedInOther = false;
                        pWrtShell->SetSelection(*m_pSpellState->pOtherCursor);
                        pWrtShell->SpellEnd();
                        delete m_pSpellState->pOtherCursor;
                        m_pSpellState->pOtherCursor = 0;
                        pWrtShell->SpellStart(DOCPOS_OTHERSTART, DOCPOS_CURR, DOCPOS_OTHERSTART );
                        pWrtShell->SpellSentence(aRet, m_bIsGrammarCheckingOn);
                    }
                    if(!aRet.size())
                    {
                        
                        pWrtShell->SpellEnd();
                        if(bOtherText)
                        {
                            m_pSpellState->m_bOtherSpelled = true;
                            
                            if(!m_pSpellState->m_bBodySpelled)
                            {
                                pWrtShell->SpellStart(DOCPOS_START, DOCPOS_END, DOCPOS_START );
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
                                pWrtShell->SpellStart(DOCPOS_OTHERSTART, DOCPOS_OTHEREND, DOCPOS_OTHERSTART );
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

                    
                    if(!aRet.size() &&
                            (m_pSpellState->m_bDrawingsSpelled ||
                            !FindNextDrawTextError_Impl(*pWrtShell) || !SpellDrawText_Impl(*pWrtShell, aRet)))
                    {
                        lcl_LeaveDrawText(*pWrtShell);
                        m_pSpellState->m_bDrawingsSpelled = true;
                    }
                }
            }
        }
        
        
        bool bCloseMessage = true;
        if(!aRet.size() && !m_pSpellState->m_bStartedInSelection)
        {
            OSL_ENSURE(m_pSpellState->m_bDrawingsSpelled &&
                        m_pSpellState->m_bOtherSpelled && m_pSpellState->m_bBodySpelled,
                        "not all parts of the document are already spelled");
            if(m_pSpellState->m_xStartRange.is())
            {
                LockFocusNotification( true );
                sal_uInt16 nRet = QueryBox( GetWindow(),  SW_RES(RID_QB_SPELL_CONTINUE)).Execute();
                if(RET_YES == nRet)
                {
                    SwUnoInternalPaM aPam(*pWrtShell->GetDoc());
                    if (::sw::XTextRangeToSwPaM(aPam,
                                m_pSpellState->m_xStartRange))
                    {
                        pWrtShell->SetSelection(aPam);
                        pWrtShell->SpellStart(DOCPOS_START, DOCPOS_CURR, DOCPOS_START);
                        if(!pWrtShell->SpellSentence(aRet, m_bIsGrammarCheckingOn))
                            pWrtShell->SpellEnd();
                    }
                    m_pSpellState->m_xStartRange = 0;
                    LockFocusNotification( false );
                    
                    LoseFocus();
                }
                else
                    bCloseMessage = false; 
            }
        }
        if(!aRet.size())
        {
            if(bCloseMessage)
            {
                LockFocusNotification( true );
                OUString sInfo(SW_RES(STR_SPELLING_COMPLETED));
                
                Window* pTemp = GetWindow();    
                InfoBox(pTemp, sInfo ).Execute();
                LockFocusNotification( false );
                
                LoseFocus();
            }

            
            GetBindings().GetDispatcher()->Execute(FN_SPELL_GRAMMAR_DIALOG, SFX_CALLMODE_ASYNCHRON);
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
        ShellModes  eSelMode = pWrtShell->GetView().GetShellMode();
        bool bDrawText = SHELL_MODE_DRAWTEXT == eSelMode;
        bool bNormalText =
            SHELL_MODE_TABLE_TEXT == eSelMode ||
            SHELL_MODE_LIST_TEXT == eSelMode ||
            SHELL_MODE_TABLE_LIST_TEXT == eSelMode ||
            SHELL_MODE_TEXT == eSelMode;

        
        
        
        bRecheck |= pWrtShell->HasLastSentenceGotGrammarChecked();

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
    OUString aPropName(UPN_IS_GRAMMAR_INTERACTIVE);
    SvtLinguConfig().SetProperty( aPropName, aVal );
    
    
    SwWrtShell* pWrtShell = GetWrtShell_Impl();
    if(pWrtShell)
    {
        ShellModes  eSelMode = pWrtShell->GetView().GetShellMode();
        bool bDrawText = SHELL_MODE_DRAWTEXT == eSelMode;
        bool bNormalText =
            SHELL_MODE_TABLE_TEXT == eSelMode ||
            SHELL_MODE_LIST_TEXT == eSelMode ||
            SHELL_MODE_TABLE_LIST_TEXT == eSelMode ||
            SHELL_MODE_TEXT == eSelMode;
        if( bNormalText )
            pWrtShell->PutSpellingToSentenceStart();
        else if( bDrawText )
        {
            SdrView*     pSdrView = pWrtShell->GetDrawView();
            SdrOutliner* pOutliner = pSdrView ? pSdrView->GetTextEditOutliner() : 0;
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
        ShellModes  eSelMode = pWrtShell->GetView().GetShellMode();
        if(eSelMode != m_pSpellState->m_eSelMode)
        {
            
            if(m_pSpellState->m_bLostFocus)
                bInvalidate = true;
        }
        else
        {
            switch(m_pSpellState->m_eSelMode)
            {
                case SHELL_MODE_TEXT:
                case SHELL_MODE_LIST_TEXT:
                case SHELL_MODE_TABLE_TEXT:
                case SHELL_MODE_TABLE_LIST_TEXT:
                {
                    SwPaM* pCursor = pWrtShell->GetCrsr();
                    if(m_pSpellState->m_pPointNode != pCursor->GetNode(true) ||
                        m_pSpellState->m_pMarkNode != pCursor->GetNode(false)||
                        m_pSpellState->m_nPointPos != pCursor->GetPoint()->nContent.GetIndex()||
                        m_pSpellState->m_nMarkPos != pCursor->GetMark()->nContent.GetIndex())
                            bInvalidate = true;
                }
                break;
                case SHELL_MODE_DRAWTEXT:
                {
                    SdrView*     pSdrView = pWrtShell->GetDrawView();
                    SdrOutliner* pOutliner = pSdrView ? pSdrView->GetTextEditOutliner() : 0;
                    if(!pOutliner || m_pSpellState->m_pOutliner != pOutliner)
                        bInvalidate = true;
                    else
                    {
                        OutlinerView* pOLV = pSdrView->GetTextEditOutlinerView();
                        OSL_ENSURE(pOLV, "no OutlinerView in SwSpellDialogChildWindow::GetFocus()");
                        if(!pOLV || !m_pSpellState->m_aESelection.IsEqual(pOLV->GetSelection()))
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
    
    m_pSpellState->m_bLostFocus = true;
    if(m_pSpellState->m_bLockFocus)
        return;
    SwWrtShell* pWrtShell = GetWrtShell_Impl();
    if(pWrtShell)
    {
        m_pSpellState->m_eSelMode = pWrtShell->GetView().GetShellMode();
        m_pSpellState->m_pPointNode = m_pSpellState->m_pMarkNode = 0;
        m_pSpellState->m_nPointPos = m_pSpellState->m_nMarkPos = 0;
        m_pSpellState->m_pOutliner = 0;

        switch(m_pSpellState->m_eSelMode)
        {
            case SHELL_MODE_TEXT:
            case SHELL_MODE_LIST_TEXT:
            case SHELL_MODE_TABLE_TEXT:
            case SHELL_MODE_TABLE_LIST_TEXT:
            {
                
                SwPaM* pCursor = pWrtShell->GetCrsr();
                m_pSpellState->m_pPointNode = pCursor->GetNode(true);
                m_pSpellState->m_pMarkNode = pCursor->GetNode(false);
                m_pSpellState->m_nPointPos = pCursor->GetPoint()->nContent.GetIndex();
                m_pSpellState->m_nMarkPos = pCursor->GetMark()->nContent.GetIndex();

            }
            break;
            case SHELL_MODE_DRAWTEXT:
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
            default:;
        }
    }
    else
        m_pSpellState->m_eSelMode = SHELL_MODE_OBJECT;
}

void SwSpellDialogChildWindow::InvalidateSpellDialog()
{
    SwWrtShell* pWrtShell = GetWrtShell_Impl();
    if(!m_pSpellState->m_bInitialCall && pWrtShell)
        pWrtShell->SpellEnd(0, false);
    m_pSpellState->Reset();
    svx::SpellDialogChildWindow::InvalidateSpellDialog();
}

SwWrtShell* SwSpellDialogChildWindow::GetWrtShell_Impl()
{
    SfxDispatcher* pDispatch = GetBindings().GetDispatcher();
    SwView* pView = 0;
    if(pDispatch)
    {
        sal_uInt16 nShellIdx = 0;
        SfxShell* pShell;
        while(0 != (pShell = pDispatch->GetShell(nShellIdx++)))
            if(pShell->ISA(SwView))
            {
                pView = static_cast<SwView* >(pShell);
                break;
            }
    }
    return pView ? pView->GetWrtShellPtr(): 0;
}



bool SwSpellDialogChildWindow::MakeTextSelection_Impl(SwWrtShell& rShell, ShellModes  eSelMode)
{
    SwView& rView = rShell.GetView();
    switch(eSelMode)
    {
        case SHELL_MODE_TEXT:
        case SHELL_MODE_LIST_TEXT:
        case SHELL_MODE_TABLE_TEXT:
        case SHELL_MODE_TABLE_LIST_TEXT:
        case SHELL_MODE_DRAWTEXT:
            OSL_FAIL("text already active in SwSpellDialogChildWindow::MakeTextSelection_Impl()");
        break;

        case SHELL_MODE_FRAME:
        {
            rShell.UnSelectFrm();
            rShell.LeaveSelFrmMode();
            rView.AttrChangedNotify(&rShell);
        }
        break;

        case SHELL_MODE_DRAW:
        case SHELL_MODE_DRAW_CTRL:
        case SHELL_MODE_DRAW_FORM:
        case SHELL_MODE_BEZIER:
            if(FindNextDrawTextError_Impl(rShell))
            {
                rView.AttrChangedNotify(&rShell);
                break;
            }
        
        case SHELL_MODE_GRAPHIC:
        case SHELL_MODE_OBJECT:
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
                    ((SdrHdlList&)pSdrView->GetHdlList()).ResetFocusHdl();
                }
                else
                {
                    rView.LeaveDrawCreate();
                    Point aPt(LONG_MIN, LONG_MIN);
                    
                    rShell.SelectObj(aPt, SW_LEAVE_FRAME);
                    SfxBindings& rBind = rView.GetViewFrame()->GetBindings();
                    rBind.Invalidate( SID_ATTR_SIZE );
                    rShell.EnterStdMode();
                    rView.AttrChangedNotify(&rShell);
                }
            }
        }
        break;
        default:; 
    }
    return true;
}


bool SwSpellDialogChildWindow::FindNextDrawTextError_Impl(SwWrtShell& rSh)
{
    bool bNextDoc = false;
    SdrView* pDrView = rSh.GetDrawView();
    if(!pDrView)
        return bNextDoc;
    SwView& rView = rSh.GetView();
    SwDoc* pDoc = rView.GetDocShell()->GetDoc();
    const SdrMarkList& rMarkList = pDrView->GetMarkedObjectList();
    
    SdrTextObj* pCurrentTextObj = 0;
    if ( rMarkList.GetMarkCount() == 1 )
    {
        SdrObject* pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();
        if( pObj && pObj->ISA(SdrTextObj) )
            pCurrentTextObj = static_cast<SdrTextObj*>(pObj);
    }
    
    if(!m_pSpellState->m_bTextObjectsCollected )
    {
        m_pSpellState->m_bTextObjectsCollected = true;
        std::list<SdrTextObj*> aTextObjs;
        SwDrawContact::GetTextObjectsFromFmt( aTextObjs, pDoc );
        if(pCurrentTextObj)
        {
            m_pSpellState->m_aTextObjects.remove(pCurrentTextObj);
            m_pSpellState->m_aTextObjects.push_back(pCurrentTextObj);
                                }
                            }
    if(m_pSpellState->m_aTextObjects.size())
    {
        Reference< XSpellChecker1 >  xSpell( GetSpellChecker() );
        while(!bNextDoc && m_pSpellState->m_aTextObjects.size())
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
                    SdrOutliner aTmpOutliner(pDoc->GetDrawModel()->
                                             GetDrawOutliner().GetEmptyItemSet().GetPool(),
                                                OUTLINERMODE_TEXTOBJECT );
                    aTmpOutliner.SetRefDevice( pDoc->getPrinter( false ) );
                    MapMode aMapMode (MAP_TWIP);
                    aTmpOutliner.SetRefMapMode(aMapMode);
                    aTmpOutliner.SetPaperSize( pTextObj->GetLogicRect().GetSize() );
                    aTmpOutliner.SetSpeller( xSpell );

                    OutlinerView* pOutlView = new OutlinerView( &aTmpOutliner, &(rView.GetEditWin()) );
                    pOutlView->GetOutliner()->SetRefDevice( rSh.getIDocumentDeviceAccess()->getPrinter( false ) );
                    aTmpOutliner.InsertView( pOutlView );
                    Point aPt;
                    Size aSize(1,1);
                    Rectangle aRect( aPt, aSize );
                    pOutlView->SetOutputArea( aRect );
                    aTmpOutliner.SetText( *pParaObj );
                    aTmpOutliner.ClearModifyFlag();
                    bHasSpellError = EE_SPELL_OK != aTmpOutliner.HasSpellErrors();
                    aTmpOutliner.RemoveView( pOutlView );
                    delete pOutlView;
                }
                if(bHasSpellError)
                {
                    
                    if(pCurrentTextObj)
                        pDrView->SdrEndTextEdit( sal_True );
                    
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

bool SwSpellDialogChildWindow::SpellDrawText_Impl(SwWrtShell& rSh, ::svx::SpellPortions& rPortions)
{
    bool bRet = false;
    SdrView*     pSdrView = rSh.GetDrawView();
    SdrOutliner* pOutliner = pSdrView ? pSdrView->GetTextEditOutliner() : 0;
    OSL_ENSURE(pOutliner, "No Outliner in SwSpellDialogChildWindow::SpellDrawText_Impl");
    if(pOutliner)
    {
        bRet = pOutliner->SpellSentence(pSdrView->GetTextEditOutlinerView()->GetEditView(), rPortions, m_bIsGrammarCheckingOn);
        
        
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
