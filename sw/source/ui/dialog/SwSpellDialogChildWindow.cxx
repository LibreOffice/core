/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SwSpellDialogChildWindow.cxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: vg $ $Date: 2008-03-18 16:02:48 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"
#ifndef SW_SPELL_DIALOG_CHILD_WINDOW_HXX
#include <SwSpellDialogChildWindow.hxx>
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#ifndef _MySVXACORR_HXX //autogen
#include <svx/svxacorr.hxx>
#endif
#ifndef _SVXACCFG_HXX
#include <svx/acorrcfg.hxx>
#endif
#ifndef _SVX_SVXIDS_HRC
#include <svx/svxids.hrc>
#endif
#ifndef _SFXAPP_HXX
#include <sfx2/app.hxx>
#endif
#ifndef _SFX_BINDINGS_HXX
#include <sfx2/bindings.hxx>
#endif
#ifndef _SFXDISPATCH_HXX
#include <sfx2/dispatch.hxx>
#endif
#ifndef _UNO_LINGU_HXX
#include <svx/unolingu.hxx>
#endif
#ifndef _WRTSH_HXX
#include <wrtsh.hxx>
#endif
#ifndef _SFX_PRINTER_HXX
#include <sfx2/printer.hxx>
#endif
#ifndef _SVDOUTL_HXX
#include <svx/svdoutl.hxx>
#endif
#ifndef _SVDVIEW_HXX
#include <svx/svdview.hxx>
#endif
#ifndef _SVDITER_HXX
#include <svx/svditer.hxx>
#endif
#ifndef _SVDOGRP_HXX
#include <svx/svdogrp.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _DOCSH_HXX
#include <docsh.hxx>
#endif
#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif
#ifndef _FRMFMT_HXX
#include <frmfmt.hxx>
#endif
#ifndef _DCONTACT_HXX
#include <dcontact.hxx>
#endif
#ifndef _EDTWIN_HXX
#include <edtwin.hxx>
#endif
#ifndef _PAM_HXX
#include <pam.hxx>
#endif
#ifndef _SW_DRAWBASE_HXX
#include <drawbase.hxx>
#endif
#ifndef _UNOOBJ_HXX
#include <unoobj.hxx>
#endif
#ifndef _DIALOG_HXX
#include <dialog.hrc>
#endif
#ifndef _BOOKMRK_HXX
#include <bookmrk.hxx>
#endif


using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::linguistic2;
using namespace ::com::sun::star::beans;

SFX_IMPL_CHILDWINDOW(SwSpellDialogChildWindow, SID_SPELL_DIALOG)


#define SPELL_START_BODY        0   // body text area
#define SPELL_START_OTHER       1   // frame, footnote, header, footer
#define SPELL_START_DRAWTEXT    2   // started in a draw text object

struct SpellState
{
    bool                m_bInitialCall;
    bool                m_bLockFocus; //lock the focus notification while a modal dialog is active
    bool                m_bLostFocus;

    //restart and progress information
    USHORT              m_SpellStartPosition;
    bool                m_bBodySpelled;  //body already spelled
    bool                m_bOtherSpelled; //frames, footnotes, headers and footers spelled
    bool                m_bStartedInOther; //started the spelling insided of the _other_ area
    bool                m_bStartedInSelection; // there was an initial text selection
    SwPaM*              pOtherCursor; // position where the spelling inside the _other_ area started
    bool                m_bDrawingsSpelled; //all drawings spelled
    Reference<XTextRange> m_xStartRange; //text range that marks the start of spelling
    const SdrObject*    m_pStartDrawing; //draw text object spelling started in
    ESelection          m_aStartDrawingSelection; //draw text start selection
    bool                m_bRestartDrawing; // the first selected drawing object is found again

    //lose/get focus information to decide if spelling can be continued
    ShellModes          m_eSelMode;
    const SwNode*       m_pPointNode;
    const SwNode*       m_pMarkNode;
    xub_StrLen          m_nPointPos;
    xub_StrLen          m_nMarkPos;
    const SdrOutliner*  m_pOutliner;
    ESelection          m_aESelection;

    //iterating over draw text objects
    std::vector<SdrTextObj*> m_aTextObjects;
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

        m_eSelMode(SHELL_MODE_OBJECT), //initially invalid
        m_pPointNode(0),
        m_pMarkNode(0),
        m_nPointPos(0),
        m_nMarkPos(0),
        m_pOutliner(0),
        m_bTextObjectsCollected(false)
        {}

    ~SpellState() {delete pOtherCursor;}

    // reset state in ::InvalidateSpellDialog
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
/*-- 30.10.2003 14:33:26---------------------------------------------------

  -----------------------------------------------------------------------*/
void lcl_LeaveDrawText(SwWrtShell& rSh)
{
    if(rSh.GetDrawView())
    {
        rSh.GetDrawView()->SdrEndTextEdit( TRUE );
        Point aPt(LONG_MIN, LONG_MIN);
        //go out of the frame
        rSh.SelectObj(aPt, SW_LEAVE_FRAME);
        rSh.EnterStdMode();
        rSh.GetView().AttrChangedNotify(&rSh);
    }
}
/*-- 09.09.2003 10:39:22---------------------------------------------------

  -----------------------------------------------------------------------*/
SwSpellDialogChildWindow::SwSpellDialogChildWindow (
            Window* _pParent,
            USHORT nId,
            SfxBindings* pBindings,
            SfxChildWinInfo* pInfo) :
                svx::SpellDialogChildWindow (
                    _pParent, nId, pBindings, pInfo),
                    m_pSpellState(new SpellState)
{
}
/*-- 09.09.2003 10:39:22---------------------------------------------------

  -----------------------------------------------------------------------*/
SwSpellDialogChildWindow::~SwSpellDialogChildWindow ()
{
    SwWrtShell* pWrtShell = GetWrtShell_Impl();
    if(!m_pSpellState->m_bInitialCall && pWrtShell)
        pWrtShell->SpellEnd();
    delete m_pSpellState;
}

/*-- 09.09.2003 12:40:07---------------------------------------------------

  -----------------------------------------------------------------------*/
SfxChildWinInfo SwSpellDialogChildWindow::GetInfo (void) const
{
    SfxChildWinInfo aInfo = svx::SpellDialogChildWindow::GetInfo();
    aInfo.bVisible = FALSE;
    return aInfo;
}

/*-- 09.09.2003 10:39:40---------------------------------------------------

  -----------------------------------------------------------------------*/
svx::SpellPortions SwSpellDialogChildWindow::GetNextWrongSentence (void)
{
    svx::SpellPortions aRet;
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
        //Writer text outside of the body
        bool bOtherText = false;

        if( m_pSpellState->m_bInitialCall )
        {
            //if no text selection exists the cursor has to be set into the text
            if(!bDrawText && !bNormalText)
            {
                if(!MakeTextSelection_Impl(*pWrtShell, eSelMode))
                    return aRet;
                else
                {
                    // the selection type has to be checked again - both text types are possible
                    if(0 != (pWrtShell->GetSelectionType()& nsSelectionType::SEL_DRW_TXT))
                        bDrawText = true;
                    bNormalText = !bDrawText;
                }
            }
            if(bNormalText)
            {
                //set cursor to the start of the sentence
                if(!pWrtShell->HasSelection())
                    pWrtShell->GoStartSentence();
                else
                    m_pSpellState->m_bStartedInSelection = true;
                //determine if the selection is outside of the body text
                bOtherText = !(pWrtShell->GetFrmType(0,sal_True) & FRMTYPE_BODY);
                m_pSpellState->m_SpellStartPosition = bOtherText ? SPELL_START_OTHER : SPELL_START_BODY;
                if(bOtherText)
                {
                    m_pSpellState->pOtherCursor = new SwPaM(*pWrtShell->GetCrsr()->GetPoint());
                    m_pSpellState->m_bStartedInOther = true;
                    pWrtShell->SpellStart( DOCPOS_OTHERSTART, DOCPOS_OTHEREND, DOCPOS_CURR, FALSE );
                }
                else
                {
                    SwPaM* pCrsr = pWrtShell->GetCrsr();
                    //mark the start position only if not at start of doc
                    if(!pWrtShell->IsStartOfDoc())
                    {
                        m_pSpellState->m_xStartRange = SwXTextRange::CreateTextRangeFromPosition(
                                pWrtShell->GetDoc(), *pCrsr->Start(), pCrsr->End());
                    }
                    pWrtShell->SpellStart( DOCPOS_START, DOCPOS_END, DOCPOS_CURR, FALSE );
                }
            }
            else
            {
                SdrView* pSdrView = pWrtShell->GetDrawView();
                m_pSpellState->m_SpellStartPosition = SPELL_START_DRAWTEXT;
                m_pSpellState->m_pStartDrawing = pSdrView->GetMarkedObjectList().GetMark(0)->GetMarkedSdrObj();
                OutlinerView* pOLV = pSdrView->GetTextEditOutlinerView();
                m_pSpellState->m_aStartDrawingSelection = pOLV->GetSelection();
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
                    //now the drawings have been spelled
                    m_pSpellState->m_bDrawingsSpelled = true;
                    //the spelling continues at the other content
                    //if there's any that has not been spelled yet
                    if(!m_pSpellState->m_bOtherSpelled && pWrtShell->HasOtherCnt())
                    {
                        pWrtShell->SpellStart(DOCPOS_OTHERSTART, DOCPOS_OTHEREND, DOCPOS_OTHERSTART, FALSE );
                        if(!pWrtShell->SpellSentence(aRet))
                        {
                            pWrtShell->SpellEnd();
                            m_pSpellState->m_bOtherSpelled = true;
                        }
                    }
                    else
                        m_pSpellState->m_bOtherSpelled = true;
                    //if no result has been found try at the body text - completely
                    if(!m_pSpellState->m_bBodySpelled && !aRet.size())
                    {
                        pWrtShell->SpellStart(DOCPOS_START, DOCPOS_END, DOCPOS_START, FALSE );
                        if(!pWrtShell->SpellSentence(aRet))
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
            //spell inside of the Writer text
            if(!pWrtShell->SpellSentence(aRet))
            {
                //find out which text has been spelled body or other
                bOtherText = !(pWrtShell->GetFrmType(0,sal_True) & FRMTYPE_BODY);
                if(bOtherText && m_pSpellState->m_bStartedInOther && m_pSpellState->pOtherCursor)
                {
                    m_pSpellState->m_bStartedInOther = false;
                    pWrtShell->SpellEnd();
                    pWrtShell->SetSelection(*m_pSpellState->pOtherCursor);
                    delete m_pSpellState->pOtherCursor;
                    m_pSpellState->pOtherCursor = 0;
                    pWrtShell->SpellStart(DOCPOS_OTHERSTART, DOCPOS_CURR, DOCPOS_OTHERSTART, FALSE );
                    pWrtShell->SpellSentence(aRet);
                }
                if(!aRet.size())
                {
                    //end spelling
                    pWrtShell->SpellEnd();
                    if(bOtherText)
                    {
                        m_pSpellState->m_bOtherSpelled = true;
                        //has the body been spelled?
                        if(!m_pSpellState->m_bBodySpelled)
                        {
                            pWrtShell->SpellStart(DOCPOS_START, DOCPOS_END, DOCPOS_START, FALSE );
                            if(!pWrtShell->SpellSentence(aRet))
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
                            pWrtShell->SpellStart(DOCPOS_OTHERSTART, DOCPOS_OTHEREND, DOCPOS_OTHERSTART, FALSE );
                            if(!pWrtShell->SpellSentence(aRet))
                            {
                                pWrtShell->SpellEnd();
                                m_pSpellState->m_bOtherSpelled = true;
                            }
                         }
                         else
                             m_pSpellState->m_bOtherSpelled = true;
                    }
                }

                //search for a draw text object that contains error and spell it
                if(!aRet.size() &&
                        (m_pSpellState->m_bDrawingsSpelled ||
                        !FindNextDrawTextError_Impl(*pWrtShell) || !SpellDrawText_Impl(*pWrtShell, aRet)))
                {
                    lcl_LeaveDrawText(*pWrtShell);
                    m_pSpellState->m_bDrawingsSpelled = true;
                }
            }
        }
        // now only the rest of the body text can be spelled -
        // if the spelling started inside of the body
        //
        bool bCloseMessage = true;
        if(!aRet.size() && !m_pSpellState->m_bStartedInSelection)
        {
            DBG_ASSERT(m_pSpellState->m_bDrawingsSpelled &&
                        m_pSpellState->m_bOtherSpelled && m_pSpellState->m_bBodySpelled,
                        "not all parts of the document are already spelled")
            if(m_pSpellState->m_xStartRange.is())
            {
                LockFocusNotification( true );
                USHORT nRet = QueryBox( GetWindow(),  SW_RES(RID_QB_SPELL_CONTINUE)).Execute();
                if(RET_YES == nRet)
                {
                    SwUnoInternalPaM aPam(*pWrtShell->GetDoc());
                    if(SwXTextRange::XTextRangeToSwPaM(aPam, m_pSpellState->m_xStartRange))
                    {
                        pWrtShell->SetSelection(aPam);
                        pWrtShell->SpellStart(DOCPOS_START, DOCPOS_CURR, DOCPOS_START);
                        if(!pWrtShell->SpellSentence(aRet))
                            pWrtShell->SpellEnd();
                    }
                    m_pSpellState->m_xStartRange = 0;
                    LockFocusNotification( false );
                    //take care that the now valid selection is stored
                    LoseFocus();
                }
                else
                    bCloseMessage = false; //no closing message if a wrap around has been denied
            }
        }
        if(!aRet.size())
        {
            if(bCloseMessage)
            {
                LockFocusNotification( true );
                String sInfo(SW_RES(STR_SPELLING_COMPLETED));
                //#i84610#
                Window* pTemp = GetWindow();    // temporary needed for g++ 3.3.5
                InfoBox(pTemp, sInfo ).Execute();
                LockFocusNotification( false );
                //take care that the now valid selection is stored
                LoseFocus();
            }

            //close the spelling dialog
            GetBindings().GetDispatcher()->Execute(SID_SPELL_DIALOG, SFX_CALLMODE_ASYNCHRON);
        }
    }
    return aRet;

}
/*-- 09.09.2003 10:39:40---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwSpellDialogChildWindow::ApplyChangedSentence(const svx::SpellPortions& rChanged)
{
    SwWrtShell* pWrtShell = GetWrtShell_Impl();
    DBG_ASSERT(!m_pSpellState->m_bInitialCall, "ApplyChangedSentence in initial call or after resume")
    if(pWrtShell && !m_pSpellState->m_bInitialCall)
    {
        ShellModes  eSelMode = pWrtShell->GetView().GetShellMode();
        bool bDrawText = SHELL_MODE_DRAWTEXT == eSelMode;
        bool bNormalText =
            SHELL_MODE_TABLE_TEXT == eSelMode ||
            SHELL_MODE_LIST_TEXT == eSelMode ||
            SHELL_MODE_TABLE_LIST_TEXT == eSelMode ||
            SHELL_MODE_TEXT == eSelMode;
        if(bNormalText)
            pWrtShell->ApplyChangedSentence(rChanged);
        else if(bDrawText )
        {
            SdrView* pDrView = pWrtShell->GetDrawView();
            SdrOutliner *pOutliner = pDrView->GetTextEditOutliner();
            pOutliner->ApplyChangedSentence(pDrView->GetTextEditOutlinerView()->GetEditView(), rChanged);
        }
    }
}
/*-- 21.10.2003 09:33:57---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwSpellDialogChildWindow::AddAutoCorrection(
        const String& rOld, const String& rNew, LanguageType eLanguage)
{
    SvxAutoCorrect* pACorr = SvxAutoCorrCfg::Get()->GetAutoCorrect();
    pACorr->PutText( rOld, rNew, eLanguage );
}
/*-- 21.10.2003 09:33:59---------------------------------------------------

  -----------------------------------------------------------------------*/
bool SwSpellDialogChildWindow::HasAutoCorrection()
{
    return true;
}
/*-- 28.10.2003 08:41:09---------------------------------------------------

  -----------------------------------------------------------------------*/
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
            //prevent initial invalidation
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
                    if(m_pSpellState->m_pPointNode != pCursor->GetNode(TRUE) ||
                        m_pSpellState->m_pMarkNode != pCursor->GetNode(FALSE)||
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
                        DBG_ASSERT(pOLV, "no OutlinerView in SwSpellDialogChildWindow::GetFocus()")
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
/*-- 28.10.2003 08:41:09---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwSpellDialogChildWindow::LoseFocus()
{
    //prevent initial invalidation
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
                //store a node pointer and a pam-position to be able to check on next GetFocus();
                SwPaM* pCursor = pWrtShell->GetCrsr();
                m_pSpellState->m_pPointNode = pCursor->GetNode(TRUE);
                m_pSpellState->m_pMarkNode = pCursor->GetNode(FALSE);
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
                DBG_ASSERT(pOutliner && pOLV, "no Outliner/OutlinerView in SwSpellDialogChildWindow::LoseFocus()")
                if(pOLV)
                {
                    m_pSpellState->m_aESelection = pOLV->GetSelection();
                }
            }
            break;
            default:;//prevent warning
        }
    }
    else
        m_pSpellState->m_eSelMode = SHELL_MODE_OBJECT;
}
/*-- 18.09.2003 12:50:18---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwSpellDialogChildWindow::InvalidateSpellDialog()
{
    SwWrtShell* pWrtShell = GetWrtShell_Impl();
    if(!m_pSpellState->m_bInitialCall && pWrtShell)
        pWrtShell->SpellEnd(0, false);
    m_pSpellState->Reset();
    svx::SpellDialogChildWindow::InvalidateSpellDialog();
}

/*-- 18.09.2003 12:54:59---------------------------------------------------

  -----------------------------------------------------------------------*/
SwWrtShell* SwSpellDialogChildWindow::GetWrtShell_Impl()
{
    SfxDispatcher* pDispatch = GetBindings().GetDispatcher();
    SwView* pView = 0;
    if(pDispatch)
    {
        USHORT nShellIdx = 0;
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

/*-- 13.10.2003 15:19:04---------------------------------------------------
    set the cursor into the body text - necessary if any object is selected
    on start of the spelling dialog
  -----------------------------------------------------------------------*/
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
            DBG_ERROR("text already active in SwSpellDialogChildWindow::MakeTextSelection_Impl()")
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
        //otherwise no break to deselect the object
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
                    //go out of the frame
                    rShell.SelectObj(aPt, SW_LEAVE_FRAME);
                    SfxBindings& rBind = rView.GetViewFrame()->GetBindings();
                    rBind.Invalidate( SID_ATTR_SIZE );
                    rShell.EnterStdMode();
                    rView.AttrChangedNotify(&rShell);
                }
            }
        }
        break;
        default:; //prevent warning
    }
    return true;
}
/*-- 13.10.2003 15:20:09---------------------------------------------------
    select the next draw text object that has a spelling error
  -----------------------------------------------------------------------*/
bool SwSpellDialogChildWindow::FindNextDrawTextError_Impl(SwWrtShell& rSh)
{
    bool bNextDoc = false;
    SdrView* pDrView = rSh.GetDrawView();
    if(!pDrView)
        return bNextDoc;
    SwView& rView = rSh.GetView();
    SwDoc* pDoc = rView.GetDocShell()->GetDoc();
    const SdrMarkList& rMarkList = pDrView->GetMarkedObjectList();
    //start at the current draw object - if there is any selected
    SdrTextObj* pCurrentTextObj = 0;
    if ( rMarkList.GetMarkCount() == 1 )
    {
        SdrObject* pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();
        if( pObj && pObj->ISA(SdrTextObj) )
            pCurrentTextObj = static_cast<SdrTextObj*>(pObj);
    }
    //at first fill the vector of drawing objects
    if(!m_pSpellState->m_bTextObjectsCollected )
    {
        m_pSpellState->m_bTextObjectsCollected = true;
        sal_uInt16 n = 0;
        //iterate in the 'normal' array of objects
        while( n < pDoc->GetSpzFrmFmts()->Count() )
        {
            SwFrmFmt* pFly = (*pDoc->GetSpzFrmFmts())[ n ];
            if( pFly->IsA( TYPE(SwDrawFrmFmt) ) )
            {
                SwClientIter aIter( (SwFmt&) *pFly );
                if( aIter.First( TYPE(SwDrawContact) ) )
                {
                    SdrObject* pSdrO = ((SwDrawContact*)aIter())->GetMaster();
                    if ( pSdrO )
                    {
                        if ( pSdrO->IsA( TYPE(SdrObjGroup) ) )
                        {
                            SdrObjListIter aListIter( *pSdrO, IM_DEEPNOGROUPS );
                            //iterate inside of a grouped object
                            while( aListIter.IsMore() )
                            {
                                SdrObject* pSdrOElement = aListIter.Next();
                                if( pSdrOElement && pSdrOElement->IsA( TYPE(SdrTextObj) ) &&
                                    static_cast<SdrTextObj*>( pSdrOElement)->HasText() &&
                                    pCurrentTextObj != pSdrOElement)
                                {
                                    m_pSpellState->m_aTextObjects.push_back((SdrTextObj*) pSdrOElement);
                                }
                            }
                        }
                        else if( pSdrO->IsA( TYPE(SdrTextObj) ) &&
                                static_cast<SdrTextObj*>( pSdrO )->HasText() &&
                                    pCurrentTextObj != pSdrO)
                        {
                            m_pSpellState->m_aTextObjects.push_back((SdrTextObj*) pSdrO);
                        }
                    }
                }
            }
            ++n;
        }
        if(pCurrentTextObj)
            m_pSpellState->m_aTextObjects.push_back(pCurrentTextObj);
    }
    if(m_pSpellState->m_aTextObjects.size())
    {
        Reference< XSpellChecker1 >  xSpell( GetSpellChecker() );
        while(!bNextDoc && m_pSpellState->m_aTextObjects.size())
        {
            std::vector<SdrTextObj*>::iterator aStart = m_pSpellState->m_aTextObjects.begin();
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
                    //now the current one has to be deselected
                    if(pCurrentTextObj)
                        pDrView->SdrEndTextEdit( TRUE );
                    //and the found one should be activated
                    rSh.MakeVisible(pTextObj->GetLogicRect());
                    Point aTmp( 0,0 );
                    rSh.SelectObj( aTmp, 0, pTextObj );
                    SdrPageView* pPV = pDrView->GetSdrPageView();
                    rView.BeginTextEdit( pTextObj, pPV, &rView.GetEditWin(), FALSE );
                    rView.AttrChangedNotify(&rSh);
                    bNextDoc = true;
                }
            }
        }
    }
    return bNextDoc;
}

/*-- 13.10.2003 15:24:27---------------------------------------------------

  -----------------------------------------------------------------------*/
bool SwSpellDialogChildWindow::SpellDrawText_Impl(SwWrtShell& rSh, ::svx::SpellPortions& rPortions)
{
    bool bRet = false;
    SdrView*     pSdrView = rSh.GetDrawView();
    SdrOutliner* pOutliner = pSdrView ? pSdrView->GetTextEditOutliner() : 0;
    DBG_ASSERT(pOutliner, "No Outliner in SwSpellDialogChildWindow::SpellDrawText_Impl")
    if(pOutliner)
    {
        bRet = pOutliner->SpellSentence(pSdrView->GetTextEditOutlinerView()->GetEditView(), rPortions);
        //find out if the current selection is in the first spelled drawing object
        //and behind the initial selection
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
/*-- 30.10.2003 14:54:59---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwSpellDialogChildWindow::LockFocusNotification(bool bLock)
{
    DBG_ASSERT(m_pSpellState->m_bLockFocus != bLock, "invalid locking - no change of state")
    m_pSpellState->m_bLockFocus = bLock;
}


