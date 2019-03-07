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
#include <vcl/accel.hxx>
#include <vcl/mnemonic.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>

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

#include <memory>

ScFormulaReferenceHelper::ScFormulaReferenceHelper(IAnyRefDialog* _pDlg,SfxBindings* _pBindings)
 : m_pDlg(_pDlg)
 , m_pRefEdit (nullptr)
 , m_pRefBtn (nullptr)
 , m_pWindow(nullptr)
 , m_pBindings(_pBindings)
 , m_nOldBorderWidth (0)
 , m_nRefTab(0)
 , m_nOldEditWidthReq(-1)
 , m_pOldEditParent(nullptr)
 , m_bOldDlgLayoutEnabled(false)
 , m_bOldEditParentLayoutEnabled(false)
 , m_bHighlightRef(false)
 , m_bAccInserted(false)
{
    ScInputOptions aInputOption=SC_MOD()->GetInputOptions();
    m_bEnableColorRef=aInputOption.GetRangeFinder();
}

ScFormulaReferenceHelper::~ScFormulaReferenceHelper()
{
    dispose();
}

void ScFormulaReferenceHelper::dispose()
{
    if (m_bAccInserted)
        Application::RemoveAccel( m_pAccel.get() );
    m_bAccInserted = false;

    // common cleanup for ScAnyRefDlg and ScFormulaDlg is done here
    HideReference();
    enableInput( true );

    ScInputHandler* pInputHdl = SC_MOD()->GetInputHdl();
    if ( pInputHdl )
        pInputHdl->ResetDelayTimer();   // stop the timer for disabling the input line

    m_pAccel.reset();

    m_pOldEditParent.clear();
    m_pWindow.clear();
    m_pRefBtn.clear();
    m_pRefEdit.clear();
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
    if (m_bEnableColorRef)
    {
        m_bHighlightRef = true;
        ScViewData* pViewData=ScDocShell::GetViewData();
        if ( pViewData )
        {
            ScDocument* pDoc=pViewData->GetDocument();
            ScTabViewShell* pTabViewShell=pViewData->GetViewShell();

            ScRangeList aRangeList;

            pTabViewShell->DoneRefMode();
            pTabViewShell->ClearHighlightRanges();

            if( ParseWithNames( aRangeList, rStr, pDoc ) )
            {
                for ( size_t i = 0, nRanges = aRangeList.size(); i < nRanges; ++i )
                {
                    ScRange const & rRangeEntry = aRangeList[ i ];
                    Color aColName = ScRangeFindList::GetColorName( i );
                    pTabViewShell->AddHighlightRange( rRangeEntry, aColName );
               }
            }
        }
    }
}

bool ScFormulaReferenceHelper::ParseWithNames( ScRangeList& rRanges, const OUString& rStr, const ScDocument* pDoc )
{
    rRanges.RemoveAll();

    if (rStr.isEmpty())
        return true;

    ScAddress::Details aDetails(pDoc->GetAddressConvention(), 0, 0);
    ScRangeUtil aRangeUtil;

    bool bError = false;
    sal_Int32 nIdx {0};
    do
    {
        ScRange aRange;
        OUString aRangeStr( rStr.getToken( 0, ';', nIdx ) );

        ScRefFlags nFlags = aRange.ParseAny( aRangeStr, pDoc, aDetails );
        if ( nFlags & ScRefFlags::VALID )
        {
            if ( (nFlags & ScRefFlags::TAB_3D) == ScRefFlags::ZERO )
                aRange.aStart.SetTab( m_nRefTab );
            if ( (nFlags & ScRefFlags::TAB2_3D) == ScRefFlags::ZERO )
                aRange.aEnd.SetTab( aRange.aStart.Tab() );
            rRanges.push_back( aRange );
        }
        else if ( ScRangeUtil::MakeRangeFromName( aRangeStr, pDoc, m_nRefTab, aRange, RUTL_NAMES, aDetails ) )
            rRanges.push_back( aRange );
        else
            bError = true;
    }
    while (nIdx>0);

    return !bError;
}

void ScFormulaReferenceHelper::ShowFormulaReference(const OUString& rStr)
{
    if( m_bEnableColorRef)
    {
        m_bHighlightRef=true;
        ScViewData* pViewData=ScDocShell::GetViewData();
        if ( pViewData && m_pRefComp.get() )
        {
            ScTabViewShell* pTabViewShell=pViewData->GetViewShell();
            SCCOL nCol = pViewData->GetCurX();
            SCROW nRow = pViewData->GetCurY();
            SCTAB nTab = pViewData->GetTabNo();
            ScAddress aPos( nCol, nRow, nTab );

            std::unique_ptr<ScTokenArray> pScTokA(m_pRefComp->CompileString(rStr));

            if (pTabViewShell && pScTokA)
            {
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
                            aRange = aRef.toAbs(aPos);
                        }
                        else
                        {
                            ScSingleRefData aRef( *pToken->GetSingleRef() );
                            aRange.aStart = aRef.toAbs(aPos);
                            aRange.aEnd = aRange.aStart;
                        }
                        Color aColName=ScRangeFindList::GetColorName(nIndex++);
                        pTabViewShell->AddHighlightRange(aRange, aColName);
                    }

                    pToken = aIter.GetNextReference();
                }
            }
        }
    }
}

void ScFormulaReferenceHelper::HideReference( bool bDoneRefMode )
{
    ScViewData* pViewData=ScDocShell::GetViewData();

    if( pViewData && m_bHighlightRef && m_bEnableColorRef)
    {
        ScTabViewShell* pTabViewShell=pViewData->GetViewShell();

        if(pTabViewShell!=nullptr)
        {
            //  bDoneRefMode is sal_False when called from before SetReference.
            //  In that case, RefMode was just started and must not be ended now.

            if ( bDoneRefMode )
                pTabViewShell->DoneRefMode();
            pTabViewShell->ClearHighlightRanges();
        }
        m_bHighlightRef=false;
    }
}

void ScFormulaReferenceHelper::ShowReference(const OUString& rStr)
{
    if( m_bEnableColorRef )
    {
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
}

void ScFormulaReferenceHelper::ReleaseFocus( formula::RefEdit* pEdit )
{
    if( !m_pRefEdit && pEdit )
    {
        m_pDlg->RefInputStart( pEdit );
    }

    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    if( pViewShell )
    {
        pViewShell->ActiveGrabFocus();
        if( m_pRefEdit )
        {
            const ScViewData& rViewData = pViewShell->GetViewData();
            ScDocument* pDoc = rViewData.GetDocument();
            ScRangeList aRangeList;
            if( ParseWithNames( aRangeList, m_pRefEdit->GetText(), pDoc ) )
            {
                if ( !aRangeList.empty() )
                {
                    const ScRange & rRange = aRangeList.front();
                    pViewShell->SetTabNo( rRange.aStart.Tab() );
                    pViewShell->MoveCursorAbs(  rRange.aStart.Col(),
                        rRange.aStart.Row(), SC_FOLLOW_JUMP, false, false );
                    pViewShell->MoveCursorAbs( rRange.aEnd.Col(),
                        rRange.aEnd.Row(), SC_FOLLOW_JUMP, true, false );
                    m_pDlg->SetReference( rRange, pDoc );
                }
            }
        }
    }
}

void ScFormulaReferenceHelper::Init()
{
    ScViewData* pViewData=ScDocShell::GetViewData();    //! use pScViewShell?
    if ( pViewData )
    {
        ScDocument* pDoc = pViewData->GetDocument();
        SCCOL nCol = pViewData->GetCurX();
        SCROW nRow = pViewData->GetCurY();
        SCTAB nTab = pViewData->GetTabNo();
        ScAddress aCursorPos( nCol, nRow, nTab );

        m_pRefComp.reset( new ScCompiler( pDoc, aCursorPos, pDoc->GetGrammar()) );
        m_pRefComp->EnableJumpCommandReorder(false);
        m_pRefComp->EnableStopOnError(false);

        m_nRefTab = nTab;
    }
}

IMPL_LINK( ScFormulaReferenceHelper, AccelSelectHdl, Accelerator&, rSelAccel, void )
{
    switch ( rSelAccel.GetCurKeyCode().GetCode() )
    {
        case KEY_RETURN:
        case KEY_ESCAPE:
            if( m_pRefEdit )
                m_pRefEdit->GrabFocus();
            m_pDlg->RefInputDone( true );
        break;
    }
}

void ScFormulaReferenceHelper::RefInputDone( bool bForced )
{
    if ( CanInputDone( bForced ) )
    {
        if (m_bAccInserted)           // disable accelerator
        {
            Application::RemoveAccel( m_pAccel.get() );
            m_bAccInserted = false;
        }

        //get rid of all this junk when we can
        if (!m_bOldDlgLayoutEnabled)
        {
            m_pWindow->SetOutputSizePixel(m_aOldDialogSize);

            // restore the parent of the edit field
            m_pRefEdit->SetParent(m_pOldEditParent);

            // the window is at the old size again
            m_pWindow->SetOutputSizePixel(m_aOldDialogSize);

            // set button parent
            if( m_pRefBtn )
            {
                m_pRefBtn->SetParent(m_pWindow);
            }
        }

        if (!m_bOldEditParentLayoutEnabled)
        {
            // set pEditCell to old position
            m_pRefEdit->SetPosSizePixel(m_aOldEditPos, m_aOldEditSize);

            // set button position
            if( m_pRefBtn )
            {
                m_pRefBtn->SetPosPixel( m_aOldButtonPos );
            }
        }

        // Adjust window title
        m_pWindow->SetText(m_sOldDialogText);

        // set button image
        if( m_pRefBtn )
            m_pRefBtn->SetStartImage();

        // All others: Show();
        for (VclPtr<vcl::Window> const & pWindow : m_aHiddenWidgets)
        {
            pWindow->Show();
        }
        m_aHiddenWidgets.clear();

        if (m_bOldDlgLayoutEnabled)
        {
            m_pRefEdit->set_width_request(m_nOldEditWidthReq);
            Dialog* pResizeDialog = m_pRefEdit->GetParentDialog();
            pResizeDialog->set_border_width(m_nOldBorderWidth);
            if (vcl::Window *pActionArea = pResizeDialog->get_action_area())
                pActionArea->Show();
            pResizeDialog->setOptimalLayoutSize();
        }

        m_pRefEdit = nullptr;
        m_pRefBtn = nullptr;
    }
}

typedef std::set<VclPtr<vcl::Window> > winset;

namespace
{
    void hideUnless(const vcl::Window *pTop, const winset& rVisibleWidgets,
        std::vector<VclPtr<vcl::Window> > &rWasVisibleWidgets)
    {
        for (vcl::Window* pChild = pTop->GetWindow(GetWindowType::FirstChild); pChild;
            pChild = pChild->GetWindow(GetWindowType::Next))
        {
            if (!pChild->IsVisible())
                continue;
            if (rVisibleWidgets.find(pChild) == rVisibleWidgets.end())
            {
                rWasVisibleWidgets.emplace_back(pChild);
                pChild->Hide();
            }
            else if (isContainerWindow(pChild))
            {
                hideUnless(pChild, rVisibleWidgets, rWasVisibleWidgets);
            }
        }
    }
}

void ScFormulaReferenceHelper::RefInputStart( formula::RefEdit* pEdit, formula::RefButton* pButton )
{
    if (!m_pRefEdit)
    {
        m_pRefEdit = pEdit;
        m_pRefBtn  = pButton;

        m_bOldDlgLayoutEnabled = isLayoutEnabled(m_pWindow);
        m_aOldEditSize = m_pRefEdit->GetSizePixel();
        m_nOldEditWidthReq = m_pRefEdit->get_width_request();
        m_pOldEditParent = m_pRefEdit->GetParent();
        m_bOldEditParentLayoutEnabled = isContainerWindow(m_pOldEditParent);

        //get rid of all the !mbOldDlgLayoutEnabled and
        //mbOldEditParentLayoutEnabled junk when we can after the last user of
        //this is widget-layout-ified
        if (!m_bOldEditParentLayoutEnabled)
        {
            // Save current state
            m_aOldDialogSize = m_pWindow->GetOutputSizePixel();
            m_aOldEditPos = m_pRefEdit->GetPosPixel();
            if (m_pRefBtn)
                m_aOldButtonPos = m_pRefBtn->GetPosPixel();
        }

        if (!m_bOldDlgLayoutEnabled)
        {
            m_pRefEdit->SetParent(m_pWindow);
            if(m_pRefBtn)
                m_pRefBtn->SetParent(m_pWindow);
        }

        // Save and adjust window title
        m_sOldDialogText = m_pWindow->GetText();
        if (vcl::Window *pLabel = m_pRefEdit->GetLabelWidgetForShrinkMode())
        {
            const OUString sLabel = pLabel->GetText();
            if (!sLabel.isEmpty())
            {
                const OUString sNewDialogText = m_sOldDialogText + ": " + comphelper::string::stripEnd(sLabel, ':');
                m_pWindow->SetText( MnemonicGenerator::EraseAllMnemonicChars( sNewDialogText ) );
            }
        }

        Dialog* pResizeDialog = nullptr;

        if (!m_bOldDlgLayoutEnabled)
        {
            for (vcl::Window* pChild = m_pWindow->GetWindow(GetWindowType::FirstChild); pChild;
                pChild = pChild->GetWindow(GetWindowType::Next))
            {
                vcl::Window *pWin = pChild->GetWindow(GetWindowType::Client);
                if (pWin == static_cast<vcl::Window*>(m_pRefEdit) || pWin == static_cast<vcl::Window*>(m_pRefBtn))
                    continue; // do nothing
                else if (pWin->IsVisible())
                {
                    m_aHiddenWidgets.emplace_back(pChild);
                    pChild->Hide();
                }
            }
        }
        else
        {
            //We want just pRefBtn and pRefEdit to be shown
            //mark widgets we want to be visible, starting with pRefEdit
            //and all its direct parents.
            winset aVisibleWidgets;
            pResizeDialog = m_pRefEdit->GetParentDialog();
            vcl::Window *pContentArea = pResizeDialog->get_content_area();
            for (vcl::Window *pCandidate = m_pRefEdit;
                pCandidate && (pCandidate != pContentArea && pCandidate->IsVisible());
                pCandidate = pCandidate->GetWindow(GetWindowType::RealParent))
            {
                aVisibleWidgets.insert(pCandidate);
            }
            //same again with pRefBtn, except stop if there's a
            //shared parent in the existing widgets
            for (vcl::Window *pCandidate = m_pRefBtn;
                pCandidate && (pCandidate != pContentArea && pCandidate->IsVisible());
                pCandidate = pCandidate->GetWindow(GetWindowType::RealParent))
            {
                if (aVisibleWidgets.insert(pCandidate).second)
                    break;
            }

            //hide everything except the aVisibleWidgets
            hideUnless(pContentArea, aVisibleWidgets, m_aHiddenWidgets);
        }

        if (!m_bOldDlgLayoutEnabled)
        {
            // move and adapt edit field
            Size aNewDlgSize(m_aOldDialogSize.Width(), m_aOldEditSize.Height());
            Size aNewEditSize(aNewDlgSize);
            long nOffset = 0;
            if (m_pRefBtn)
            {
                aNewEditSize.AdjustWidth( -(m_pRefBtn->GetSizePixel().Width()) );
                aNewEditSize.AdjustWidth( -(m_aOldButtonPos.X() - (m_aOldEditPos.X()+m_aOldEditSize.Width())) );

                long nHeight = m_pRefBtn->GetSizePixel().Height();
                if ( nHeight > m_aOldEditSize.Height() )
                {
                    aNewDlgSize.setHeight( nHeight );
                    nOffset = (nHeight-m_aOldEditSize.Height()) / 2;
                }
                aNewEditSize.AdjustWidth( -nOffset );
            }
            m_pRefEdit->SetPosSizePixel(Point(nOffset, nOffset), aNewEditSize);

            // set button position
            if( m_pRefBtn )
                m_pRefBtn->SetPosPixel( Point( m_aOldDialogSize.Width() - m_pRefBtn->GetSizePixel().Width(), 0 ) );

            // shrink window
            m_pWindow->SetOutputSizePixel(aNewDlgSize);
        }

        // set button image
        if( m_pRefBtn )
            m_pRefBtn->SetEndImage();

        if (!m_pAccel)
        {
            m_pAccel.reset( new Accelerator );
            m_pAccel->InsertItem( 1, vcl::KeyCode( KEY_RETURN ) );
            m_pAccel->InsertItem( 2, vcl::KeyCode( KEY_ESCAPE ) );
            m_pAccel->SetSelectHdl( LINK( this, ScFormulaReferenceHelper, AccelSelectHdl ) );
        }
        Application::InsertAccel( m_pAccel.get() );
        m_bAccInserted = true;

        if (m_bOldDlgLayoutEnabled)
        {
            m_pRefEdit->set_width_request(m_aOldEditSize.Width());
            m_nOldBorderWidth = pResizeDialog->get_border_width();
            pResizeDialog->set_border_width(0);
            if (vcl::Window *pActionArea = pResizeDialog->get_action_area())
                pActionArea->Hide();
            pResizeDialog->setOptimalLayoutSize();
        }
    }
}

void ScFormulaReferenceHelper::ToggleCollapsed( formula::RefEdit* pEdit, formula::RefButton* pButton )
{
    if( pEdit )
    {
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

//The class of ScAnyRefDlg is rewritten by PengYunQuan for Validity Cell Range Picker
//  class ScRefHandler

ScRefHandler::ScRefHandler( vcl::Window &rWindow, SfxBindings* pB, bool bBindRef ):
        m_rWindow( &rWindow ),
        m_bInRefMode( false ),
        m_aHelper(this,pB),
        m_pMyBindings( pB )
{
    m_aHelper.SetWindow(m_rWindow.get());

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

    return m_bInRefMode = true;
}

ScRefHandler::~ScRefHandler()
{
    disposeRefHandler();
}

void ScRefHandler::disposeRefHandler()
{
    m_rWindow.clear();
    LeaveRefMode();
    m_aHelper.dispose();
}

bool ScRefHandler::LeaveRefMode()
{
    if( !m_bInRefMode ) return false;

    lcl_HideAllReferences();

    if( Dialog *pDlg = dynamic_cast<Dialog*>( m_rWindow.get() ) )
        pDlg->SetModalInputMode(false);
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
    return m_rWindow->IsVisible(); // references can only be input to visible windows
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

bool ScRefHandler::ParseWithNames( ScRangeList& rRanges, const OUString& rStr, const ScDocument* pDoc )
{
    return m_aHelper.ParseWithNames( rRanges, rStr, pDoc );
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
