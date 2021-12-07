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

#include <rangelst.hxx>
#include <comphelper/string.hxx>
#include <sfx2/app.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/viewfrm.hxx>
#include <osl/diagnose.h>

#include <anyrefdg.hxx>
#include <sc.hrc>
#include <inputhdl.hxx>
#include <scmod.hxx>
#include <inputwin.hxx>
#include <tabvwsh.hxx>
#include <docsh.hxx>
#include <rfindlst.hxx>
#include <compiler.hxx>
#include <inputopt.hxx>
#include <rangeutl.hxx>
#include <tokenarray.hxx>
#include <comphelper/lok.hxx>
#include <output.hxx>

#include <memory>

ScFormulaReferenceHelper::ScFormulaReferenceHelper(IAnyRefDialog* _pDlg,SfxBindings* _pBindings)
 : m_pDlg(_pDlg)
 , m_pRefEdit (nullptr)
 , m_pRefBtn (nullptr)
 , m_pDialog(nullptr)
 , m_pBindings(_pBindings)
 , m_nRefTab(0)
 , m_bHighlightRef(false)
{
    ScInputOptions aInputOption=SC_MOD()->GetInputOptions();
    m_bEnableColorRef=aInputOption.GetRangeFinder();
}

ScFormulaReferenceHelper::~ScFormulaReferenceHelper() COVERITY_NOEXCEPT_FALSE
{
    dispose();
}

void ScFormulaReferenceHelper::dispose()
{
    // common cleanup for ScAnyRefDlg and ScFormulaDlg is done here
    HideReference();
    enableInput( true );

    ScInputHandler* pInputHdl = SC_MOD()->GetInputHdl();
    if ( pInputHdl )
        pInputHdl->ResetDelayTimer();   // stop the timer for disabling the input line

    m_pDialog = nullptr;
}

void ScFormulaReferenceHelper::enableInput( bool bEnable )
{
    ScDocShell* pDocShell = static_cast<ScDocShell*>(SfxObjectShell::GetFirst(checkSfxObjectShell<ScDocShell>));
    while( pDocShell )
    {
        SfxViewFrame* pFrame = SfxViewFrame::GetFirst( pDocShell );
        while( pFrame )
        {
            //  enable everything except InPlace, including bean frames
            if ( !pFrame->GetFrame().IsInPlace() )
            {
                SfxViewShell* p = pFrame->GetViewShell();
                ScTabViewShell* pViewSh = dynamic_cast< ScTabViewShell *>( p );
                if(pViewSh!=nullptr)
                {
                    vcl::Window *pWin=pViewSh->GetWindow();
                    if(pWin)
                    {
                        vcl::Window *pParent=pWin->GetParent();
                        if(pParent)
                        {
                            pParent->EnableInput(bEnable);
                            pViewSh->EnableRefInput(bEnable);
                        }
                    }
                }
            }
            pFrame = SfxViewFrame::GetNext( *pFrame, pDocShell );
        }

        pDocShell = static_cast<ScDocShell*>(SfxObjectShell::GetNext(*pDocShell, checkSfxObjectShell<ScDocShell>));
    }
}

void ScFormulaReferenceHelper::ShowSimpleReference(const OUString& rStr)
{
    if (!m_bEnableColorRef)
        return;

    m_bHighlightRef = true;
    ScViewData* pViewData=ScDocShell::GetViewData();
    if ( !pViewData )
        return;

    ScDocument& rDoc = pViewData->GetDocument();
    ScTabViewShell* pTabViewShell=pViewData->GetViewShell();

    ScRangeList aRangeList;

    pTabViewShell->DoneRefMode();
    pTabViewShell->ClearHighlightRanges();

    if( ParseWithNames( aRangeList, rStr, rDoc ) )
    {
        for ( size_t i = 0, nRanges = aRangeList.size(); i < nRanges; ++i )
        {
            ScRange const & rRangeEntry = aRangeList[ i ];
            Color aColName = ScRangeFindList::GetColorName( i );
            pTabViewShell->AddHighlightRange( rRangeEntry, aColName );
        }
    }
}

bool ScFormulaReferenceHelper::ParseWithNames( ScRangeList& rRanges, const OUString& rStr, const ScDocument& rDoc )
{
    rRanges.RemoveAll();

    if (rStr.isEmpty())
        return true;

    ScAddress::Details aDetails(rDoc.GetAddressConvention(), 0, 0);

    bool bError = false;
    sal_Int32 nIdx {0};
    do
    {
        ScRange aRange;
        OUString aRangeStr( rStr.getToken( 0, ';', nIdx ) );

        ScRefFlags nFlags = aRange.ParseAny( aRangeStr, rDoc, aDetails );
        if ( nFlags & ScRefFlags::VALID )
        {
            if ( (nFlags & ScRefFlags::TAB_3D) == ScRefFlags::ZERO )
                aRange.aStart.SetTab( m_nRefTab );
            if ( (nFlags & ScRefFlags::TAB2_3D) == ScRefFlags::ZERO )
                aRange.aEnd.SetTab( aRange.aStart.Tab() );
            rRanges.push_back( aRange );
        }
        else if ( ScRangeUtil::MakeRangeFromName( aRangeStr, rDoc, m_nRefTab, aRange, RUTL_NAMES, aDetails ) )
            rRanges.push_back( aRange );
        else
            bError = true;
    }
    while (nIdx>0);

    return !bError;
}

void ScFormulaReferenceHelper::ShowFormulaReference(const OUString& rStr)
{
    if( !m_bEnableColorRef)
        return;

    m_bHighlightRef=true;
    ScViewData* pViewData=ScDocShell::GetViewData();
    if ( !(pViewData && m_pRefComp) )
        return;

    ScTabViewShell* pTabViewShell=pViewData->GetViewShell();
    SCCOL nCol = pViewData->GetCurX();
    SCROW nRow = pViewData->GetCurY();
    SCTAB nTab = pViewData->GetTabNo();
    ScAddress aPos( nCol, nRow, nTab );

    std::unique_ptr<ScTokenArray> pScTokA(m_pRefComp->CompileString(rStr));

    if (!(pTabViewShell && pScTokA))
        return;

    const ScViewData& rViewData = pTabViewShell->GetViewData();
    ScDocument& rDoc = rViewData.GetDocument();
    pTabViewShell->DoneRefMode();
    pTabViewShell->ClearHighlightRanges();

    formula::FormulaTokenArrayPlainIterator aIter(*pScTokA);
    const formula::FormulaToken* pToken = aIter.GetNextReference();

    sal_uInt16 nIndex=0;

    while(pToken!=nullptr)
    {
        bool bDoubleRef=(pToken->GetType()==formula::svDoubleRef);

        if(pToken->GetType()==formula::svSingleRef || bDoubleRef)
        {
            ScRange aRange;
            if(bDoubleRef)
            {
                ScComplexRefData aRef( *pToken->GetDoubleRef() );
                aRange = aRef.toAbs(rDoc, aPos);
            }
            else
            {
                ScSingleRefData aRef( *pToken->GetSingleRef() );
                aRange.aStart = aRef.toAbs(rDoc, aPos);
                aRange.aEnd = aRange.aStart;
            }
            Color aColName=ScRangeFindList::GetColorName(nIndex++);
            pTabViewShell->AddHighlightRange(aRange, aColName);
        }

        pToken = aIter.GetNextReference();
    }
}

void ScFormulaReferenceHelper::HideReference( bool bDoneRefMode )
{
    ScViewData* pViewData=ScDocShell::GetViewData();

    if( !(pViewData && m_bHighlightRef && m_bEnableColorRef))
        return;

    ScTabViewShell* pTabViewShell=pViewData->GetViewShell();

    if(pTabViewShell!=nullptr)
    {
        //  bDoneRefMode is sal_False when called from before SetReference.
        //  In that case, RefMode was just started and must not be ended now.

        if ( bDoneRefMode )
            pTabViewShell->DoneRefMode();
        pTabViewShell->ClearHighlightRanges();

        if( comphelper::LibreOfficeKit::isActive() )
        {
            // Clear
            std::vector<ReferenceMark> aReferenceMarks;
            ScInputHandler::SendReferenceMarks( pTabViewShell, aReferenceMarks );
        }
    }
    m_bHighlightRef=false;
}

void ScFormulaReferenceHelper::ShowReference(const OUString& rStr)
{
    if( !m_bEnableColorRef )
        return;

    if( rStr.indexOf('(') != -1 ||
        rStr.indexOf('+') != -1 ||
        rStr.indexOf('*') != -1 ||
        rStr.indexOf('-') != -1 ||
        rStr.indexOf('/') != -1 ||
        rStr.indexOf('&') != -1 ||
        rStr.indexOf('<') != -1 ||
        rStr.indexOf('>') != -1 ||
        rStr.indexOf('=') != -1 ||
        rStr.indexOf('^') != -1 )
    {
        ShowFormulaReference(rStr);
    }
    else
    {
        ShowSimpleReference(rStr);
    }
}

void ScFormulaReferenceHelper::ReleaseFocus( formula::RefEdit* pEdit )
{
    if( !m_pRefEdit && pEdit )
    {
        m_pDlg->RefInputStart( pEdit );
    }

    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    if( !pViewShell )
        return;

    pViewShell->ActiveGrabFocus();
    if( !m_pRefEdit )
        return;

    const ScViewData& rViewData = pViewShell->GetViewData();
    ScDocument& rDoc = rViewData.GetDocument();
    ScRangeList aRangeList;
    if( !ParseWithNames( aRangeList, m_pRefEdit->GetText(), rDoc ) )
        return;

    if ( !aRangeList.empty() )
    {
        const ScRange & rRange = aRangeList.front();
        pViewShell->SetTabNo( rRange.aStart.Tab() );
        pViewShell->MoveCursorAbs(  rRange.aStart.Col(),
            rRange.aStart.Row(), SC_FOLLOW_JUMP, false, false );
        pViewShell->MoveCursorAbs( rRange.aEnd.Col(),
            rRange.aEnd.Row(), SC_FOLLOW_JUMP, true, false );
        m_pDlg->SetReference( rRange, rDoc );
    }
}

void ScFormulaReferenceHelper::Init()
{
    ScViewData* pViewData=ScDocShell::GetViewData();    //! use pScViewShell?
    if ( !pViewData )
        return;

    ScDocument& rDoc = pViewData->GetDocument();
    SCCOL nCol = pViewData->GetCurX();
    SCROW nRow = pViewData->GetCurY();
    SCTAB nTab = pViewData->GetTabNo();
    ScAddress aCursorPos( nCol, nRow, nTab );

    m_pRefComp.reset( new ScCompiler( rDoc, aCursorPos, rDoc.GetGrammar()) );
    m_pRefComp->EnableJumpCommandReorder(false);
    m_pRefComp->EnableStopOnError(false);

    m_nRefTab = nTab;
}

IMPL_LINK_NOARG(ScFormulaReferenceHelper, ActivateHdl, weld::Widget&, bool)
{
    if (m_pRefEdit)
        m_pRefEdit->GrabFocus();
    m_pDlg->RefInputDone(true);
    return true;
}

void ScFormulaReferenceHelper::RefInputDone( bool bForced )
{
    if ( !CanInputDone( bForced ) )
        return;

    if (!m_pDialog)
        return;

    // Adjust window title
    m_pDialog->set_title(m_sOldDialogText);

    if (m_pRefEdit)
        m_pRefEdit->SetActivateHdl(Link<weld::Widget&, bool>());

    // set button image
    if (m_pRefBtn)
    {
        m_pRefBtn->SetActivateHdl(Link<weld::Widget&, bool>());
        m_pRefBtn->SetStartImage();
    }

    m_pDialog->undo_collapse();

    m_pRefEdit = nullptr;
    m_pRefBtn = nullptr;
}

void ScFormulaReferenceHelper::RefInputStart( formula::RefEdit* pEdit, formula::RefButton* pButton )
{
    if (m_pRefEdit)
        return;

    m_pRefEdit = pEdit;
    m_pRefBtn  = pButton;

    // Save and adjust window title
    m_sOldDialogText = m_pDialog->get_title();
    if (weld::Label *pLabel = m_pRefEdit->GetLabelWidgetForShrinkMode())
    {
        const OUString sLabel = pLabel->get_label();
        if (!sLabel.isEmpty())
        {
            const OUString sNewDialogText = m_sOldDialogText + ": " + comphelper::string::stripEnd(sLabel, ':');
            m_pDialog->set_title(pLabel->strip_mnemonic(sNewDialogText));
        }
    }

    m_pDialog->collapse(pEdit->GetWidget(), pButton ? pButton->GetWidget() : nullptr);

    // set button image
    if (pButton)
        pButton->SetEndImage();

    m_pRefEdit->SetActivateHdl(LINK(this, ScFormulaReferenceHelper, ActivateHdl));
    if (m_pRefBtn)
        m_pRefBtn->SetActivateHdl(LINK(this, ScFormulaReferenceHelper, ActivateHdl));
}

void ScFormulaReferenceHelper::ToggleCollapsed( formula::RefEdit* pEdit, formula::RefButton* pButton )
{
    if( !pEdit )
        return;

    if( m_pRefEdit == pEdit )                 // is this the active ref edit field?
    {
        m_pRefEdit->GrabFocus();              // before RefInputDone()
        m_pDlg->RefInputDone( true );               // finish ref input
    }
    else
    {
        m_pDlg->RefInputDone( true );               // another active ref edit?
        m_pDlg->RefInputStart( pEdit, pButton );    // start ref input
        // pRefEdit might differ from pEdit after RefInputStart() (i.e. ScFormulaDlg)
        if( m_pRefEdit )
            m_pRefEdit->GrabFocus();
    }
}

void ScFormulaReferenceHelper::DoClose( sal_uInt16 nId )
{
    SfxApplication* pSfxApp = SfxGetpApp();

    SetDispatcherLock( false );         //! here and in dtor ?

    SfxViewFrame* pViewFrm = SfxViewFrame::Current();
    if ( pViewFrm && pViewFrm->HasChildWindow(FID_INPUTLINE_STATUS) )
    {
        //  The input row is disabled with ToolBox::Disable disabled, thus it must be
        //  reenabled with ToolBox::Enable (before the AppWindow is enabled)
        //  for the buttons to be drawn as enabled.
        SfxChildWindow* pChild = pViewFrm->GetChildWindow(FID_INPUTLINE_STATUS);
        if (pChild)
        {
            ScInputWindow* pWin = static_cast<ScInputWindow*>(pChild->GetWindow());
            pWin->Enable();
        }
    }

    // find parent view frame to close dialog
    SfxViewFrame* pMyViewFrm = nullptr;
    if ( m_pBindings )
    {
        SfxDispatcher* pMyDisp = m_pBindings->GetDispatcher();
        if (pMyDisp)
            pMyViewFrm = pMyDisp->GetFrame();
    }
    SC_MOD()->SetRefDialog( nId, false, pMyViewFrm );

    pSfxApp->Broadcast( SfxHint( SfxHintId::ScKillEditView ) );

    ScTabViewShell* pScViewShell = ScTabViewShell::GetActiveViewShell();
    if ( pScViewShell )
        pScViewShell->UpdateInputHandler(true);
}

void ScFormulaReferenceHelper::SetDispatcherLock( bool bLock )
{
    if (!comphelper::LibreOfficeKit::isActive())
    {
        // lock / unlock only the dispatchers of Calc documents
        ScDocShell* pDocShell = static_cast<ScDocShell*>(SfxObjectShell::GetFirst(checkSfxObjectShell<ScDocShell>));
        while (pDocShell)
        {
            SfxViewFrame* pFrame = SfxViewFrame::GetFirst(pDocShell);
            while (pFrame)
            {
                SfxDispatcher* pDisp = pFrame->GetDispatcher();
                if (pDisp)
                    pDisp->Lock(bLock);
                pFrame = SfxViewFrame::GetNext(*pFrame, pDocShell);
            }
            pDocShell = static_cast<ScDocShell*>(SfxObjectShell::GetNext(*pDocShell, checkSfxObjectShell<ScDocShell>));
        }
        return;
        // if a new view is created while the dialog is open,
        // that view's dispatcher is locked when trying to create the dialog
        // for that view (ScTabViewShell::CreateRefDialog)
    }

    //  lock / unlock only the dispatcher of Calc document
    SfxDispatcher* pDisp = nullptr;
    if ( m_pBindings )
    {
        pDisp = m_pBindings->GetDispatcher();
    }
    else if(SfxViewFrame* pViewFrame = SfxViewFrame::Current())
    {
        if (dynamic_cast< ScTabViewShell* >(pViewFrame->GetViewShell()))
            pDisp = pViewFrame->GetDispatcher();
    }

    if (pDisp)
        pDisp->Lock(bLock);
}

void ScFormulaReferenceHelper::ViewShellChanged()
{
    enableInput( false );

    EnableSpreadsheets();
}
void ScFormulaReferenceHelper::EnableSpreadsheets(bool bFlag)
{
    ScDocShell* pDocShell = static_cast<ScDocShell*>(SfxObjectShell::GetFirst(checkSfxObjectShell<ScDocShell>));
    while( pDocShell )
    {
        SfxViewFrame* pFrame = SfxViewFrame::GetFirst( pDocShell );
        while( pFrame )
        {
            //  enable everything except InPlace, including bean frames
            if ( !pFrame->GetFrame().IsInPlace() )
            {
                SfxViewShell* p = pFrame->GetViewShell();
                ScTabViewShell* pViewSh = dynamic_cast< ScTabViewShell *>( p );
                if(pViewSh!=nullptr)
                {
                    vcl::Window *pWin=pViewSh->GetWindow();
                    if(pWin)
                    {
                        vcl::Window *pParent=pWin->GetParent();
                        if(pParent)
                        {
                            pParent->EnableInput(bFlag,false);
                            pViewSh->EnableRefInput(bFlag);
                        }
                    }
                }
            }
            pFrame = SfxViewFrame::GetNext( *pFrame, pDocShell );
        }

        pDocShell = static_cast<ScDocShell*>(SfxObjectShell::GetNext(*pDocShell, checkSfxObjectShell<ScDocShell>));
    }
}

static void lcl_InvalidateWindows()
{
    ScDocShell* pDocShell = static_cast<ScDocShell*>(SfxObjectShell::GetFirst(checkSfxObjectShell<ScDocShell>));
    while( pDocShell )
    {
        SfxViewFrame* pFrame = SfxViewFrame::GetFirst( pDocShell );
        while( pFrame )
        {
            //  enable everything except InPlace, including bean frames
            if ( !pFrame->GetFrame().IsInPlace() )
            {
                SfxViewShell* p = pFrame->GetViewShell();
                ScTabViewShell* pViewSh = dynamic_cast< ScTabViewShell *>( p );
                if(pViewSh!=nullptr)
                {
                    vcl::Window *pWin=pViewSh->GetWindow();
                    if(pWin)
                    {
                        vcl::Window *pParent=pWin->GetParent();
                        if(pParent)
                            pParent->Invalidate();
                    }
                }
            }
            pFrame = SfxViewFrame::GetNext( *pFrame, pDocShell );
        }

        pDocShell = static_cast<ScDocShell*>(SfxObjectShell::GetNext(*pDocShell, checkSfxObjectShell<ScDocShell>));
    }
}

static void lcl_HideAllReferences()
{
    SfxViewShell* pSh = SfxViewShell::GetFirst( true, checkSfxViewShell<ScTabViewShell> );
    while ( pSh )
    {
        static_cast<ScTabViewShell*>(pSh)->ClearHighlightRanges();
        pSh = SfxViewShell::GetNext( *pSh, true, checkSfxViewShell<ScTabViewShell> );
    }
}

ScRefHandler::ScRefHandler(SfxDialogController& rController, SfxBindings* pB, bool bBindRef)
    : m_pController(&rController)
    , m_bInRefMode(false)
    , m_aHelper(this, pB)
    , m_pMyBindings(pB)
{
    m_aHelper.SetDialog(rController.getDialog());

    if( bBindRef ) EnterRefMode();
}

bool ScRefHandler::EnterRefMode()
{
    if( m_bInRefMode ) return false;

    SC_MOD()->InputEnterHandler();

    ScTabViewShell* pScViewShell = nullptr;

    // title has to be from the view that opened the dialog,
    // even if it's not the current view

    SfxObjectShell* pParentDoc = nullptr;
    if ( m_pMyBindings )
    {
        SfxDispatcher* pMyDisp = m_pMyBindings->GetDispatcher();
        if (pMyDisp)
        {
            SfxViewFrame* pMyViewFrm = pMyDisp->GetFrame();
            if (pMyViewFrm)
            {
                pScViewShell = dynamic_cast<ScTabViewShell*>( pMyViewFrm->GetViewShell()  );
                if( pScViewShell )
                    pScViewShell->UpdateInputHandler(true);
                pParentDoc = pMyViewFrm->GetObjectShell();
            }
        }
    }
    if ( !pParentDoc && pScViewShell )                  // use current only if above fails
        pParentDoc = pScViewShell->GetObjectShell();
    if ( pParentDoc )
        m_aDocName = pParentDoc->GetTitle();

    ScInputHandler* pInputHdl = SC_MOD()->GetInputHdl(pScViewShell);

    OSL_ENSURE( pInputHdl, "Missing input handler :-/" );

    if ( pInputHdl )
        pInputHdl->NotifyChange( nullptr );

    ScFormulaReferenceHelper::enableInput( false );

    ScFormulaReferenceHelper::EnableSpreadsheets();

    m_aHelper.Init();

    m_aHelper.SetDispatcherLock( true );

    m_bInRefMode = true;
    return m_bInRefMode;
}

ScRefHandler::~ScRefHandler() COVERITY_NOEXCEPT_FALSE
{
    disposeRefHandler();
}

void ScRefHandler::disposeRefHandler()
{
    m_pController = nullptr;
    LeaveRefMode();
    m_aHelper.dispose();
}

bool ScRefHandler::LeaveRefMode()
{
    if( !m_bInRefMode ) return false;

    lcl_HideAllReferences();

    SetDispatcherLock( false );         //! here and in DoClose ?

    ScTabViewShell* pScViewShell = ScTabViewShell::GetActiveViewShell();
    if( pScViewShell )
        pScViewShell->UpdateInputHandler(true);

    lcl_InvalidateWindows();

    m_bInRefMode = false;
    return true;
}

void ScRefHandler::SwitchToDocument()
{
    ScTabViewShell* pCurrent = ScTabViewShell::GetActiveViewShell();
    if (pCurrent)
    {
        SfxObjectShell* pObjSh = pCurrent->GetObjectShell();
        if ( pObjSh && pObjSh->GetTitle() == m_aDocName )
        {
            //  right document already visible -> nothing to do
            return;
        }
    }

    SfxViewShell* pSh = SfxViewShell::GetFirst( true, checkSfxViewShell<ScTabViewShell> );
    while ( pSh )
    {
        SfxObjectShell* pObjSh = pSh->GetObjectShell();
        if ( pObjSh && pObjSh->GetTitle() == m_aDocName )
        {
            //  switch to first TabViewShell for document
            static_cast<ScTabViewShell*>(pSh)->SetActive();
            return;
        }
        pSh = SfxViewShell::GetNext( *pSh, true, checkSfxViewShell<ScTabViewShell> );
    }
}

bool ScRefHandler::IsDocAllowed(SfxObjectShell* pDocSh) const   // pDocSh may be 0
{
    //  if aDocName isn't initialized, allow
    if ( m_aDocName.isEmpty() )
        return true;

    if ( !pDocSh )
        return false;

    //  default: allow only same document (overridden in function dialog)
    return m_aDocName==pDocSh->GetTitle();
}

bool ScRefHandler::IsRefInputMode() const
{
    return m_pController->getDialog()->get_visible();
}

bool ScRefHandler::DoClose( sal_uInt16 nId )
{
    m_aHelper.DoClose(nId);
    return true;
}

void ScRefHandler::SetDispatcherLock( bool bLock )
{
    m_aHelper.SetDispatcherLock( bLock );
}

void ScRefHandler::ViewShellChanged()
{
    ScFormulaReferenceHelper::ViewShellChanged();
}

void ScRefHandler::AddRefEntry()
{
    // override this for multi-references
}

bool ScRefHandler::IsTableLocked() const
{
    // the default is that the sheet can be switched during while the reference is edited

    return false;
}

//  RefInputStart/Done: Zoom-In (AutoHide) on single field
//  (using button or movement)

void ScRefHandler::RefInputStart( formula::RefEdit* pEdit, formula::RefButton* pButton )
{
    m_aHelper.RefInputStart( pEdit, pButton );
}

void ScRefHandler::ToggleCollapsed( formula::RefEdit* pEdit, formula::RefButton* pButton )
{
    m_aHelper.ToggleCollapsed( pEdit, pButton );
}

bool ScRefHandler::ParseWithNames( ScRangeList& rRanges, const OUString& rStr, const ScDocument& rDoc )
{
    return m_aHelper.ParseWithNames( rRanges, rStr, rDoc );
}

void ScRefHandler::HideReference( bool bDoneRefMode )
{
    m_aHelper.HideReference( bDoneRefMode );
}

void ScRefHandler::ShowReference(const OUString& rStr)
{
    m_aHelper.ShowReference(rStr);
}

void ScRefHandler::ReleaseFocus( formula::RefEdit* pEdit )
{
    m_aHelper.ReleaseFocus( pEdit );
}

void ScRefHandler::RefInputDone( bool bForced )
{
    m_aHelper.RefInputDone( bForced );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
