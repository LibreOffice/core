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

#include "rangelst.hxx"
#include <comphelper/string.hxx>
#include <sfx2/app.hxx>
#include <sfx2/viewsh.hxx>
#include <vcl/wrkwin.hxx>
#include <vcl/mnemonic.hxx>
#include <tools/shl.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>

#include "anyrefdg.hxx"
#include "sc.hrc"
#include "inputhdl.hxx"
#include "scmod.hxx"
#include "scresid.hxx"
#include "inputwin.hxx"
#include "tabvwsh.hxx"
#include "docsh.hxx"
#include "rfindlst.hxx"
#include "compiler.hxx"
#include "formulacell.hxx"
#include "global.hxx"
#include "inputopt.hxx"
#include "rangeutl.hxx"
#include "tokenarray.hxx"

#include <boost/scoped_ptr.hpp>

ScFormulaReferenceHelper::ScFormulaReferenceHelper(IAnyRefDialog* _pDlg,SfxBindings* _pBindings)
 : m_pDlg(_pDlg)
 , pRefEdit (NULL)
 , pRefBtn (NULL)
 , m_pWindow(NULL)
 , m_pBindings(_pBindings)
 , pAccel( NULL )
 , m_nOldBorderWidth (0)
 , nRefTab(0)
 , mnOldEditWidthReq( -1 )
 , mpOldEditParent( NULL )
 , mbOldDlgLayoutEnabled( false )
 , mbOldEditParentLayoutEnabled( false )
 , bHighlightRef( false )
 , bAccInserted( false )
{
    ScInputOptions aInputOption=SC_MOD()->GetInputOptions();
    bEnableColorRef=aInputOption.GetRangeFinder();
}

ScFormulaReferenceHelper::~ScFormulaReferenceHelper()
{
    if (bAccInserted)
        Application::RemoveAccel( pAccel.get() );

    // common cleanup for ScAnyRefDlg and ScFormulaDlg is done here

    HideReference();
    enableInput( true );

    ScInputHandler* pInputHdl = SC_MOD()->GetInputHdl();
    if ( pInputHdl )
        pInputHdl->ResetDelayTimer();   // stop the timer for disabling the input line
}

void ScFormulaReferenceHelper::enableInput( bool bEnable )
{
    TypeId aType(TYPE(ScDocShell));
    ScDocShell* pDocShell = (ScDocShell*)SfxObjectShell::GetFirst(&aType);
    while( pDocShell )
    {
        SfxViewFrame* pFrame = SfxViewFrame::GetFirst( pDocShell );
        while( pFrame )
        {
            //  enable everything except InPlace, including bean frames
            if ( !pFrame->GetFrame().IsInPlace() )
            {
                SfxViewShell* p = pFrame->GetViewShell();
                ScTabViewShell* pViewSh = PTR_CAST(ScTabViewShell,p);
                if(pViewSh!=NULL)
                {
                    Window *pWin=pViewSh->GetWindow();
                    if(pWin)
                    {
                        Window *pParent=pWin->GetParent();
                        if(pParent)
                        {
                            pParent->EnableInput(bEnable,true);
                            if(true)
                                pViewSh->EnableRefInput(bEnable);
                        }
                    }
                }
            }
            pFrame = SfxViewFrame::GetNext( *pFrame, pDocShell );
        }

        pDocShell = (ScDocShell*)SfxObjectShell::GetNext(*pDocShell, &aType);
    }
}

void ScFormulaReferenceHelper::ShowSimpleReference(const OUString& rStr)
{
    if( bEnableColorRef )
    {
        bHighlightRef=true;
        ScViewData* pViewData=ScDocShell::GetViewData();
        if ( pViewData )
        {
            ScDocument* pDoc=pViewData->GetDocument();
            ScTabViewShell* pTabViewShell=pViewData->GetViewShell();

            ScRangeList aRangeList;

            pTabViewShell->DoneRefMode( false );
            pTabViewShell->ClearHighlightRanges();

            if( ParseWithNames( aRangeList, rStr, pDoc ) )
            {
                for ( size_t i = 0, nRanges = aRangeList.size(); i < nRanges; ++i )
                {
                    ScRange* pRangeEntry = aRangeList[ i ];
                    ColorData aColName = ScRangeFindList::GetColorName( i );
                    pTabViewShell->AddHighlightRange( *pRangeEntry, aColName );
               }
            }
        }
    }
}

bool ScFormulaReferenceHelper::ParseWithNames( ScRangeList& rRanges, const OUString& rStr, ScDocument* pDoc )
{
    bool bError = false;
    rRanges.RemoveAll();

    ScAddress::Details aDetails(pDoc->GetAddressConvention(), 0, 0);
    ScRangeUtil aRangeUtil;
    sal_Int32 nTokenCnt = comphelper::string::getTokenCount(rStr, ';');
    for( sal_Int32 nToken = 0; nToken < nTokenCnt; ++nToken )
    {
        ScRange aRange;
        OUString aRangeStr( rStr.getToken( nToken, ';' ) );

        sal_uInt16 nFlags = aRange.ParseAny( aRangeStr, pDoc, aDetails );
        if ( nFlags & SCA_VALID )
        {
            if ( (nFlags & SCA_TAB_3D) == 0 )
                aRange.aStart.SetTab( nRefTab );
            if ( (nFlags & SCA_TAB2_3D) == 0 )
                aRange.aEnd.SetTab( aRange.aStart.Tab() );
            rRanges.Append( aRange );
        }
        else if ( aRangeUtil.MakeRangeFromName( aRangeStr, pDoc, nRefTab, aRange, RUTL_NAMES, aDetails ) )
            rRanges.Append( aRange );
        else
            bError = true;
    }

    return !bError;
}

void ScFormulaReferenceHelper::ShowFormulaReference(const OUString& rStr)
{
    if( bEnableColorRef)
    {
        bHighlightRef=true;
        ScViewData* pViewData=ScDocShell::GetViewData();
        if ( pViewData && pRefComp.get() )
        {
            ScTabViewShell* pTabViewShell=pViewData->GetViewShell();
            SCCOL nCol = pViewData->GetCurX();
            SCROW nRow = pViewData->GetCurY();
            SCTAB nTab = pViewData->GetTabNo();
            ScAddress aPos( nCol, nRow, nTab );

            boost::scoped_ptr<ScTokenArray> pScTokA(pRefComp->CompileString(rStr));

            if (pTabViewShell && pScTokA)
            {
                pTabViewShell->DoneRefMode( false );
                pTabViewShell->ClearHighlightRanges();

                pScTokA->Reset();
                const ScToken* pToken = static_cast<const ScToken*>(pScTokA->GetNextReference());

                sal_uInt16 nIndex=0;

                while(pToken!=NULL)
                {
                    bool bDoubleRef=(pToken->GetType()==formula::svDoubleRef);


                    if(pToken->GetType()==formula::svSingleRef || bDoubleRef)
                    {
                        ScRange aRange;
                        if(bDoubleRef)
                        {
                            ScComplexRefData aRef( pToken->GetDoubleRef() );
                            aRange = aRef.toAbs(aPos);
                        }
                        else
                        {
                            ScSingleRefData aRef( pToken->GetSingleRef() );
                            aRange.aStart = aRef.toAbs(aPos);
                            aRange.aEnd = aRange.aStart;
                        }
                        ColorData aColName=ScRangeFindList::GetColorName(nIndex++);
                        pTabViewShell->AddHighlightRange(aRange, aColName);
                    }

                    pToken = static_cast<const ScToken*>(pScTokA->GetNextReference());
                }
            }
        }
    }
}

void ScFormulaReferenceHelper::HideReference( bool bDoneRefMode )
{
    ScViewData* pViewData=ScDocShell::GetViewData();

    if( pViewData && bHighlightRef && bEnableColorRef)
    {
        ScTabViewShell* pTabViewShell=pViewData->GetViewShell();

        if(pTabViewShell!=NULL)
        {
            //  bDoneRefMode is sal_False when called from before SetReference.
            //  In that case, RefMode was just started and must not be ended now.

            if ( bDoneRefMode )
                pTabViewShell->DoneRefMode( false );
            pTabViewShell->ClearHighlightRanges();
        }
        bHighlightRef=false;
    }
}

void ScFormulaReferenceHelper::ShowReference(const OUString& rStr)
{
    if( bEnableColorRef )
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

void ScFormulaReferenceHelper::ReleaseFocus( formula::RefEdit* pEdit, formula::RefButton* pButton )
{
    if( !pRefEdit && pEdit )
    {
        m_pDlg->RefInputStart( pEdit, pButton );
    }

    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    if( pViewShell )
    {
        pViewShell->ActiveGrabFocus();
        if( pRefEdit )
        {
            const ScViewData* pViewData = pViewShell->GetViewData();
            ScDocument* pDoc = pViewData->GetDocument();
            ScRangeList aRangeList;
            if( ParseWithNames( aRangeList, pRefEdit->GetText(), pDoc ) )
            {
                if ( !aRangeList.empty() )
                {
                    const ScRange* pRange = aRangeList.front();
                    pViewShell->SetTabNo( pRange->aStart.Tab() );
                    pViewShell->MoveCursorAbs(  pRange->aStart.Col(),
                        pRange->aStart.Row(), SC_FOLLOW_JUMP, false, false );
                    pViewShell->MoveCursorAbs( pRange->aEnd.Col(),
                        pRange->aEnd.Row(), SC_FOLLOW_JUMP, true, false );
                    m_pDlg->SetReference( *pRange, pDoc );
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

        pRefComp.reset( new ScCompiler( pDoc, aCursorPos) );
        pRefComp->SetGrammar( pDoc->GetGrammar() );
        pRefComp->SetCompileForFAP(true);

        nRefTab = nTab;
    }
}

IMPL_LINK( ScFormulaReferenceHelper, AccelSelectHdl, Accelerator *, pSelAccel )
{
    if ( !pSelAccel )
        return long(false);

    switch ( pSelAccel->GetCurKeyCode().GetCode() )
    {
        case KEY_RETURN:
        case KEY_ESCAPE:
            if( pRefEdit )
                pRefEdit->GrabFocus();
            m_pDlg->RefInputDone( true );
        break;
    }
    return long(true);
}

typedef std::vector<Window*> winvec;

void ScFormulaReferenceHelper::RefInputDone( bool bForced )
{
    if ( CanInputDone( bForced ) )
    {
        if (bAccInserted)           // Accelerator wieder abschalten
        {
            Application::RemoveAccel( pAccel.get() );
            bAccInserted = false;
        }

        //get rid of all this junk when we can
        if (!mbOldDlgLayoutEnabled)
        {
            m_pWindow->SetOutputSizePixel(aOldDialogSize);

            // restore the parent of the edit field
            pRefEdit->SetParent(mpOldEditParent);

            // Fenster wieder gross
            m_pWindow->SetOutputSizePixel(aOldDialogSize);

            // set button parent
            if( pRefBtn )
            {
                pRefBtn->SetParent(m_pWindow);
            }
        }

        if (!mbOldEditParentLayoutEnabled)
        {
            // pEditCell an alte Position
            pRefEdit->SetPosSizePixel(aOldEditPos, aOldEditSize);

            // set button position
            if( pRefBtn )
            {
                pRefBtn->SetPosPixel( aOldButtonPos );
            }
        }

        // Fenstertitel anpassen
        m_pWindow->SetText(sOldDialogText);

        // set button image
        if( pRefBtn )
            pRefBtn->SetStartImage();

        // Alle anderen: Show();
        for (winvec::iterator aI = m_aHiddenWidgets.begin(); aI != m_aHiddenWidgets.end(); ++aI)
        {
            Window *pWindow = *aI;
            pWindow->Show();
        }
        m_aHiddenWidgets.clear();

        if (mbOldDlgLayoutEnabled)
        {
            pRefEdit->set_width_request(mnOldEditWidthReq);
            Dialog* pResizeDialog = pRefEdit->GetParentDialog();
            pResizeDialog->set_border_width(m_nOldBorderWidth);
            if (Window *pActionArea = pResizeDialog->get_action_area())
                pActionArea->Show();
            pResizeDialog->setOptimalLayoutSize();
        }

        pRefEdit = NULL;
        pRefBtn = NULL;
    }
}

typedef std::set<Window*> winset;

namespace
{
    void hideUnless(Window *pTop, const winset& rVisibleWidgets,
        winvec &rWasVisibleWidgets)
    {
        for (Window* pChild = pTop->GetWindow(WINDOW_FIRSTCHILD); pChild;
            pChild = pChild->GetWindow(WINDOW_NEXT))
        {
            if (!pChild->IsVisible())
                continue;
            if (rVisibleWidgets.find(pChild) == rVisibleWidgets.end())
            {
                rWasVisibleWidgets.push_back(pChild);
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
    if (!pRefEdit)
    {
        pRefEdit = pEdit;
        pRefBtn  = pButton;

        mbOldDlgLayoutEnabled = isLayoutEnabled(m_pWindow);
        aOldEditSize = pRefEdit->GetSizePixel();
        mnOldEditWidthReq = pRefEdit->get_width_request();
        mpOldEditParent = pRefEdit->GetParent();
        mbOldEditParentLayoutEnabled = isContainerWindow(mpOldEditParent);

        //get rid of all the !mbOldDlgLayoutEnabled and
        //mbOldEditParentLayoutEnabled junk when we can after the last user of
        //this is widget-layout-ified
        if (!mbOldEditParentLayoutEnabled)
        {
            // Alte Daten merken
            aOldDialogSize = m_pWindow->GetOutputSizePixel();
            aOldEditPos = pRefEdit->GetPosPixel();
            if (pRefBtn)
                aOldButtonPos = pRefBtn->GetPosPixel();
        }

        if (!mbOldDlgLayoutEnabled)
        {
            pRefEdit->SetParent(m_pWindow);
            if(pRefBtn)
                pRefBtn->SetParent(m_pWindow);
        }

        // Fenstertitel anpassen
        sOldDialogText = m_pWindow->GetText();
        if (Window *pLabel = pRefEdit->GetLabelWidgetForShrinkMode())
        {
            OUString sLabel = pLabel->GetText();
            if (!sLabel.isEmpty())
            {
                OUString sNewDialogText = sOldDialogText;
                sNewDialogText += ": ";
                sNewDialogText += comphelper::string::stripEnd(sLabel, ':');
                m_pWindow->SetText( MnemonicGenerator::EraseAllMnemonicChars( sNewDialogText ) );
            }
        }

        Dialog* pResizeDialog = NULL;

        if (!mbOldDlgLayoutEnabled)
        {
            for (Window* pChild = m_pWindow->GetWindow(WINDOW_FIRSTCHILD); pChild;
                pChild = pChild->GetWindow(WINDOW_NEXT))
            {
                Window *pWin = pChild->GetWindow(WINDOW_CLIENT);
                if (pWin == (Window*)pRefEdit || pWin == (Window*)pRefBtn)
                    continue; // do nothing
                else if (pWin->IsVisible())
                {
                    m_aHiddenWidgets.push_back(pChild);
                    pChild->Hide();
                }
            }
        }
        else
        {
            //We want just pRefBtn and pRefEdit to be shown
            //mark widgets we want to be visible, starting with pRefEdit
            //and all its direct parents.
            winset m_aVisibleWidgets;
            pResizeDialog = pRefEdit->GetParentDialog();
            Window *pContentArea = pResizeDialog->get_content_area();
            for (Window *pCandidate = pRefEdit;
                pCandidate && (pCandidate != pContentArea && pCandidate->IsVisible());
                pCandidate = pCandidate->GetWindow(WINDOW_REALPARENT))
            {
                m_aVisibleWidgets.insert(pCandidate);
            }
            //same again with pRefBtn, except stop if there's a
            //shared parent in the existing widgets
            for (Window *pCandidate = pRefBtn;
                pCandidate && (pCandidate != pContentArea && pCandidate->IsVisible());
                pCandidate = pCandidate->GetWindow(WINDOW_REALPARENT))
            {
                if (m_aVisibleWidgets.insert(pCandidate).second)
                    break;
            }

            //hide everything except the m_aVisibleWidgets
            hideUnless(pContentArea, m_aVisibleWidgets, m_aHiddenWidgets);
        }

        if (!mbOldDlgLayoutEnabled)
        {
            // Edit-Feld verschieben und anpassen
            Size aNewDlgSize(aOldDialogSize.Width(), aOldEditSize.Height());
            Size aNewEditSize(aNewDlgSize);
            long nOffset = 0;
            if (pRefBtn)
            {
                aNewEditSize.Width() -= pRefBtn->GetSizePixel().Width();
                aNewEditSize.Width() -= aOldButtonPos.X() - (aOldEditPos.X()+aOldEditSize.Width());

                long nHeight = pRefBtn->GetSizePixel().Height();
                if ( nHeight > aOldEditSize.Height() )
                {
                    aNewDlgSize.Height() = nHeight;
                    nOffset = (nHeight-aOldEditSize.Height()) / 2;
                }
                aNewEditSize.Width() -= nOffset;
            }
            pRefEdit->SetPosSizePixel(Point(nOffset, nOffset), aNewEditSize);

            // set button position
            if( pRefBtn )
                pRefBtn->SetPosPixel( Point( aOldDialogSize.Width() - pRefBtn->GetSizePixel().Width(), 0 ) );

            // Fenster verkleinern
            m_pWindow->SetOutputSizePixel(aNewDlgSize);
        }

        // set button image
        if( pRefBtn )
            pRefBtn->SetEndImage();

        if (!pAccel.get())
        {
            pAccel.reset( new Accelerator );
            pAccel->InsertItem( 1, KeyCode( KEY_RETURN ) );
            pAccel->InsertItem( 2, KeyCode( KEY_ESCAPE ) );
            pAccel->SetSelectHdl( LINK( this, ScFormulaReferenceHelper, AccelSelectHdl ) );
        }
        Application::InsertAccel( pAccel.get() );
        bAccInserted = true;

        if (mbOldDlgLayoutEnabled)
        {
            pRefEdit->set_width_request(aOldEditSize.Width());
            m_nOldBorderWidth = pResizeDialog->get_border_width();
            pResizeDialog->set_border_width(0);
            if (Window *pActionArea = pResizeDialog->get_action_area())
                pActionArea->Hide();
            pResizeDialog->setOptimalLayoutSize();
        }
    }
}

void ScFormulaReferenceHelper::ToggleCollapsed( formula::RefEdit* pEdit, formula::RefButton* pButton )
{
    if( pEdit )
    {
        if( pRefEdit == pEdit )                 // is this the active ref edit field?
        {
            pRefEdit->GrabFocus();              // before RefInputDone()
            m_pDlg->RefInputDone( true );               // finish ref input
        }
        else
        {
            m_pDlg->RefInputDone( true );               // another active ref edit?
            m_pDlg->RefInputStart( pEdit, pButton );    // start ref input
            // pRefEdit might differ from pEdit after RefInputStart() (i.e. ScFormulaDlg)
            if( pRefEdit )
                pRefEdit->GrabFocus();
        }
    }
}

bool ScFormulaReferenceHelper::DoClose( sal_uInt16 nId )
{
    SfxApplication* pSfxApp = SFX_APP();

    SetDispatcherLock( false );         //! here and in dtor ?

    SfxViewFrame* pViewFrm = SfxViewFrame::Current();
    if ( pViewFrm && pViewFrm->HasChildWindow(FID_INPUTLINE_STATUS) )
    {
        //  Die Eingabezeile wird per ToolBox::Disable disabled, muss darum auch
        //  per ToolBox::Enable wieder aktiviert werden (vor dem Enable des AppWindow),
        //  damit die Buttons auch wieder enabled gezeichnet werden.
        SfxChildWindow* pChild = pViewFrm->GetChildWindow(FID_INPUTLINE_STATUS);
        if (pChild)
        {
            ScInputWindow* pWin = (ScInputWindow*)pChild->GetWindow();
            pWin->Enable();
        }
    }

    // find parent view frame to close dialog
    SfxViewFrame* pMyViewFrm = NULL;
    if ( m_pBindings )
    {
        SfxDispatcher* pMyDisp = m_pBindings->GetDispatcher();
        if (pMyDisp)
            pMyViewFrm = pMyDisp->GetFrame();
    }
    SC_MOD()->SetRefDialog( nId, false, pMyViewFrm );

    pSfxApp->Broadcast( SfxSimpleHint( FID_KILLEDITVIEW ) );

    ScTabViewShell* pScViewShell = ScTabViewShell::GetActiveViewShell();
    if ( pScViewShell )
        pScViewShell->UpdateInputHandler(true);

    return true;
}
void ScFormulaReferenceHelper::SetDispatcherLock( bool bLock )
{
    //  lock / unlock only the dispatchers of Calc documents

    TypeId aType(TYPE(ScDocShell));
    ScDocShell* pDocShell = (ScDocShell*)SfxObjectShell::GetFirst(&aType);
    while( pDocShell )
    {
        SfxViewFrame* pFrame = SfxViewFrame::GetFirst( pDocShell );
        while( pFrame )
        {
            SfxDispatcher* pDisp = pFrame->GetDispatcher();
            if (pDisp)
                pDisp->Lock( bLock );

            pFrame = SfxViewFrame::GetNext( *pFrame, pDocShell );
        }
        pDocShell = (ScDocShell*)SfxObjectShell::GetNext(*pDocShell, &aType);
    }

    //  if a new view is created while the dialog is open,
    //  that view's dispatcher is locked when trying to create the dialog
    //  for that view (ScTabViewShell::CreateRefDialog)
}

void ScFormulaReferenceHelper::ViewShellChanged()
{
    enableInput( false );

    EnableSpreadsheets();
}
void ScFormulaReferenceHelper::EnableSpreadsheets(bool bFlag, bool bChildren)
{
    TypeId aType(TYPE(ScDocShell));
    ScDocShell* pDocShell = (ScDocShell*)SfxObjectShell::GetFirst(&aType);
    while( pDocShell )
    {
        SfxViewFrame* pFrame = SfxViewFrame::GetFirst( pDocShell );
        while( pFrame )
        {
            //  enable everything except InPlace, including bean frames
            if ( !pFrame->GetFrame().IsInPlace() )
            {
                SfxViewShell* p = pFrame->GetViewShell();
                ScTabViewShell* pViewSh = PTR_CAST(ScTabViewShell,p);
                if(pViewSh!=NULL)
                {
                    Window *pWin=pViewSh->GetWindow();
                    if(pWin)
                    {
                        Window *pParent=pWin->GetParent();
                        if(pParent)
                        {
                            pParent->EnableInput(bFlag,false);
                            if(bChildren)
                                pViewSh->EnableRefInput(bFlag);
                        }
                    }
                }
            }
            pFrame = SfxViewFrame::GetNext( *pFrame, pDocShell );
        }

        pDocShell = (ScDocShell*)SfxObjectShell::GetNext(*pDocShell, &aType);
    }
}





static void lcl_InvalidateWindows()
{
    TypeId aType(TYPE(ScDocShell));
    ScDocShell* pDocShell = (ScDocShell*)SfxObjectShell::GetFirst(&aType);
    while( pDocShell )
    {
        SfxViewFrame* pFrame = SfxViewFrame::GetFirst( pDocShell );
        while( pFrame )
        {
            //  enable everything except InPlace, including bean frames
            if ( !pFrame->GetFrame().IsInPlace() )
            {
                SfxViewShell* p = pFrame->GetViewShell();
                ScTabViewShell* pViewSh = PTR_CAST(ScTabViewShell,p);
                if(pViewSh!=NULL)
                {
                    Window *pWin=pViewSh->GetWindow();
                    if(pWin)
                    {
                        Window *pParent=pWin->GetParent();
                        if(pParent)
                            pParent->Invalidate();
                    }
                }
            }
            pFrame = SfxViewFrame::GetNext( *pFrame, pDocShell );
        }

        pDocShell = (ScDocShell*)SfxObjectShell::GetNext(*pDocShell, &aType);
    }
}


static void lcl_HideAllReferences()
{
    TypeId aScType = TYPE(ScTabViewShell);
    SfxViewShell* pSh = SfxViewShell::GetFirst( &aScType );
    while ( pSh )
    {
        ((ScTabViewShell*)pSh)->ClearHighlightRanges();
        pSh = SfxViewShell::GetNext( *pSh, &aScType );
    }
}


//The class of ScAnyRefDlg is rewritten by PengYunQuan for Validity Cell Range Picker
//  class ScRefHandler


ScRefHandler::ScRefHandler( Window &rWindow, SfxBindings* pB, bool bBindRef ):
        m_rWindow( rWindow ),
        m_bInRefMode( false ),
        m_aHelper(this,pB),
        pMyBindings( pB ),
        pActiveWin(NULL)
{
    m_aHelper.SetWindow(&m_rWindow);
    reverseUniqueHelpIdHack(m_rWindow);
    aTimer.SetTimeout(200);
    aTimer.SetTimeoutHdl(LINK( this, ScRefHandler, UpdateFocusHdl));

    if( bBindRef ) EnterRefMode();
}

bool ScRefHandler::EnterRefMode()
{
    if( m_bInRefMode ) return false;

    SC_MOD()->InputEnterHandler();

    ScTabViewShell* pScViewShell = NULL;

    // title has to be from the view that opened the dialog,
    // even if it's not the current view

    SfxObjectShell* pParentDoc = NULL;
    if ( pMyBindings )
    {
        SfxDispatcher* pMyDisp = pMyBindings->GetDispatcher();
        if (pMyDisp)
        {
            SfxViewFrame* pMyViewFrm = pMyDisp->GetFrame();
            if (pMyViewFrm)
            {
                pScViewShell = PTR_CAST( ScTabViewShell, pMyViewFrm->GetViewShell() );
                if( pScViewShell )
                    pScViewShell->UpdateInputHandler(true);
                pParentDoc = pMyViewFrm->GetObjectShell();
            }
        }
    }
    if ( !pParentDoc && pScViewShell )                  // use current only if above fails
        pParentDoc = pScViewShell->GetObjectShell();
    if ( pParentDoc )
        aDocName = pParentDoc->GetTitle();

    ScInputHandler* pInputHdl = SC_MOD()->GetInputHdl(pScViewShell);

    OSL_ENSURE( pInputHdl, "Missing input handler :-/" );

    if ( pInputHdl )
        pInputHdl->NotifyChange( NULL );

    m_aHelper.enableInput( false );

    m_aHelper.EnableSpreadsheets();

    m_aHelper.Init();

    m_aHelper.SetDispatcherLock( true );

    return m_bInRefMode = true;
}



ScRefHandler::~ScRefHandler()
{
    LeaveRefMode();
}

bool ScRefHandler::LeaveRefMode()
{
    if( !m_bInRefMode ) return false;

    lcl_HideAllReferences();

    if( Dialog *pDlg = dynamic_cast<Dialog*>( static_cast<Window*>(*this) ) )
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
        if ( pObjSh && pObjSh->GetTitle() == aDocName )
        {
            //  right document already visible -> nothing to do
            return;
        }
    }

    TypeId aScType = TYPE(ScTabViewShell);
    SfxViewShell* pSh = SfxViewShell::GetFirst( &aScType );
    while ( pSh )
    {
        SfxObjectShell* pObjSh = pSh->GetObjectShell();
        if ( pObjSh && pObjSh->GetTitle() == aDocName )
        {
            //  switch to first TabViewShell for document
            ((ScTabViewShell*)pSh)->SetActive();
            return;
        }
        pSh = SfxViewShell::GetNext( *pSh, &aScType );
    }
}



bool ScRefHandler::IsDocAllowed(SfxObjectShell* pDocSh) const   // pDocSh may be 0
{
    //  default: allow only same document (overridden in function dialog)
    OUString aCmpName;
    if ( pDocSh )
        aCmpName = pDocSh->GetTitle();

    //  if aDocName isn't initialized, allow
    return ( aDocName.isEmpty() || aDocName == aCmpName );
}



bool ScRefHandler::IsRefInputMode() const
{
    return m_rWindow.IsVisible(); // nur wer sichtbar ist kann auch Referenzen bekommen
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
    m_aHelper.ViewShellChanged();
}



void ScRefHandler::AddRefEntry()
{
    //  wenn nicht ueberladen, gibt es keine Mehrfach-Referenzen
}



bool ScRefHandler::IsTableLocked() const
{
    // per Default kann bei Referenzeingabe auch die Tabelle umgeschaltet werden

    return false;
}



//  RefInputStart/Done: Zoom-In (AutoHide) auf einzelnes Feld
//  (per Button oder Bewegung)



void ScRefHandler::RefInputStart( formula::RefEdit* pEdit, formula::RefButton* pButton )
{
    m_aHelper.RefInputStart( pEdit, pButton );
}


void ScRefHandler::ToggleCollapsed( formula::RefEdit* pEdit, formula::RefButton* pButton )
{
    m_aHelper.ToggleCollapsed( pEdit, pButton );
}

void ScRefHandler::preNotify(const NotifyEvent& rNEvt, const bool bBindRef)
{
    if( bBindRef || m_bInRefMode )
    {
        sal_uInt16 nSwitch=rNEvt.GetType();
        if(nSwitch==EVENT_GETFOCUS)
        {
            pActiveWin=rNEvt.GetWindow();
        }
    }
}

void ScRefHandler::stateChanged(const StateChangedType nStateChange, const bool bBindRef)
{
    if( !bBindRef && !m_bInRefMode ) return;

    if(nStateChange == STATE_CHANGE_VISIBLE)
    {
        if(m_rWindow.IsVisible())
        {
            m_aHelper.enableInput( false );
            m_aHelper.EnableSpreadsheets();
            m_aHelper.SetDispatcherLock( true );
            aTimer.Start();
        }
        else
        {
            m_aHelper.enableInput( true );
            m_aHelper.SetDispatcherLock( false );           /*//! here and in DoClose ?*/
        }
    }
}

IMPL_LINK_NOARG(ScRefHandler, UpdateFocusHdl)
{
    if (pActiveWin)
    {
        pActiveWin->GrabFocus();
    }
    return 0;
}

bool ScRefHandler::ParseWithNames( ScRangeList& rRanges, const OUString& rStr, ScDocument* pDoc )
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

void ScRefHandler::ReleaseFocus( formula::RefEdit* pEdit, formula::RefButton* pButton )
{
    m_aHelper.ReleaseFocus( pEdit,pButton );
}

void ScRefHandler::RefInputDone( bool bForced )
{
    m_aHelper.RefInputDone( bForced );
}



ScRefHdlModalImpl::ScRefHdlModalImpl(Window* pParent, const OString& rID,
    const OUString& rUIXMLDescription)
    : ModalDialog( pParent, rID, rUIXMLDescription)
    , ScRefHandler(dynamic_cast<Window&>(*this), NULL, true)
{
}

bool ScRefHdlModalImpl::PreNotify( NotifyEvent& rNEvt )
{
    ScRefHandler::preNotify( rNEvt, true );
    return ModalDialog::PreNotify( rNEvt );
}

void ScRefHdlModalImpl::StateChanged( StateChangedType nStateChange )
{
    ModalDialog::StateChanged( nStateChange );
    ScRefHandler::stateChanged( nStateChange, true );
}

ScAnyRefModalDlg::ScAnyRefModalDlg(Window* pParent, const OString& rID,
    const OUString& rUIXMLDescription)
    : ScRefHdlModalImpl(pParent, rID, rUIXMLDescription)
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
