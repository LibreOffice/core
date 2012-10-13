/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
#include "cell.hxx"
#include "global.hxx"
#include "inputopt.hxx"
#include "rangeutl.hxx"


ScFormulaReferenceHelper::ScFormulaReferenceHelper(IAnyRefDialog* _pDlg,SfxBindings* _pBindings)
 : m_pDlg(_pDlg)
 , pRefEdit (NULL)
 , m_pWindow(NULL)
 , m_pBindings(_pBindings)
 , pAccel( NULL )
 , pHiddenMarks(NULL)
 , nRefTab(0)
 , mpOldEditParent( NULL )
 , bHighLightRef( false )
 , bAccInserted( false )
{
    ScInputOptions aInputOption=SC_MOD()->GetInputOptions();
    bEnableColorRef=aInputOption.GetRangeFinder();
}
// -----------------------------------------------------------------------------
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
// -----------------------------------------------------------------------------
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
                            pParent->EnableInput(bEnable,true /* sal_False */);
                            if(true /*bChildren*/)
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
// -----------------------------------------------------------------------------
void ScFormulaReferenceHelper::ShowSimpleReference( const XubString& rStr )
{
    if( bEnableColorRef )
    {
        bHighLightRef=true;
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
// -----------------------------------------------------------------------------
bool ScFormulaReferenceHelper::ParseWithNames( ScRangeList& rRanges, const String& rStr, ScDocument* pDoc )
{
    bool bError = false;
    rRanges.RemoveAll();

    ScAddress::Details aDetails(pDoc->GetAddressConvention(), 0, 0);
    ScRangeUtil aRangeUtil;
    xub_StrLen nTokenCnt = comphelper::string::getTokenCount(rStr, ';');
    for( xub_StrLen nToken = 0; nToken < nTokenCnt; ++nToken )
    {
        ScRange aRange;
        String aRangeStr( rStr.GetToken( nToken ) );

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
// -----------------------------------------------------------------------------
void ScFormulaReferenceHelper::ShowFormulaReference( const XubString& rStr )
{
    if( /*!pRefEdit &&*/ bEnableColorRef)
    {
        bHighLightRef=true;
        ScViewData* pViewData=ScDocShell::GetViewData();
        if ( pViewData && pRefComp.get() )
        {
            ScTabViewShell* pTabViewShell=pViewData->GetViewShell();
            SCCOL nCol = pViewData->GetCurX();
            SCROW nRow = pViewData->GetCurY();
            SCTAB nTab = pViewData->GetTabNo();
            ScAddress aPos( nCol, nRow, nTab );

            ScTokenArray* pScTokA=pRefComp->CompileString(rStr);
            //pRefComp->CompileTokenArray();

            if(pTabViewShell!=NULL && pScTokA!=NULL)
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
                            aRef.CalcAbsIfRel( aPos );
                            aRange.aStart.Set( aRef.Ref1.nCol, aRef.Ref1.nRow, aRef.Ref1.nTab );
                            aRange.aEnd.Set( aRef.Ref2.nCol, aRef.Ref2.nRow, aRef.Ref2.nTab );
                        }
                        else
                        {
                            ScSingleRefData aRef( pToken->GetSingleRef() );
                            aRef.CalcAbsIfRel( aPos );
                            aRange.aStart.Set( aRef.nCol, aRef.nRow, aRef.nTab );
                            aRange.aEnd = aRange.aStart;
                        }
                        ColorData aColName=ScRangeFindList::GetColorName(nIndex++);
                        pTabViewShell->AddHighlightRange(aRange, aColName);
                    }

                    pToken = static_cast<const ScToken*>(pScTokA->GetNextReference());
                }
            }
            if(pScTokA!=NULL) delete pScTokA;
        }
    }
}
// -----------------------------------------------------------------------------
void ScFormulaReferenceHelper::HideReference( bool bDoneRefMode )
{
    ScViewData* pViewData=ScDocShell::GetViewData();

    if( pViewData && /*!pRefEdit &&*/ bHighLightRef && bEnableColorRef)
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
        bHighLightRef=false;
    }
}
// -----------------------------------------------------------------------------
void ScFormulaReferenceHelper::ShowReference( const XubString& rStr )
{
    if( /*!pRefEdit &&*/ bEnableColorRef )
    {
        if( rStr.Search('(')!=STRING_NOTFOUND ||
            rStr.Search('+')!=STRING_NOTFOUND ||
            rStr.Search('*')!=STRING_NOTFOUND ||
            rStr.Search('-')!=STRING_NOTFOUND ||
            rStr.Search('/')!=STRING_NOTFOUND ||
            rStr.Search('&')!=STRING_NOTFOUND ||
            rStr.Search('<')!=STRING_NOTFOUND ||
            rStr.Search('>')!=STRING_NOTFOUND ||
            rStr.Search('=')!=STRING_NOTFOUND ||
            rStr.Search('^')!=STRING_NOTFOUND)
        {
            ShowFormulaReference(rStr);
        }
        else
        {
            ShowSimpleReference(rStr);
        }
    }
}
// -----------------------------------------------------------------------------
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
// -----------------------------------------------------------------------------
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

        String rStrExp;
        pRefCell.reset( new ScFormulaCell( pDoc, aCursorPos, rStrExp ) );
        pRefComp.reset( new ScCompiler( pDoc, aCursorPos) );
        pRefComp->SetGrammar( pDoc->GetGrammar() );
        pRefComp->SetCompileForFAP(true);

        nRefTab = nTab;
    }
}
// -----------------------------------------------------------------------------
IMPL_LINK( ScFormulaReferenceHelper, AccelSelectHdl, Accelerator *, pSelAccel )
{
    if ( !pSelAccel )
        return 0;

    switch ( pSelAccel->GetCurKeyCode().GetCode() )
    {
        case KEY_RETURN:
        case KEY_ESCAPE:
            if( pRefEdit )
                pRefEdit->GrabFocus();
            m_pDlg->RefInputDone( true );
        break;
    }
    return true;
}
//----------------------------------------------------------------------------
void ScFormulaReferenceHelper::RefInputDone( bool bForced )
{
    if ( CanInputDone( bForced ) )
    {
        if (bAccInserted)           // Accelerator wieder abschalten
        {
            Application::RemoveAccel( pAccel.get() );
            bAccInserted = false;
        }

        // restore the parent of the edit field
        pRefEdit->SetParent(mpOldEditParent);

        // Fenstertitel anpassen
        m_pWindow->SetText(sOldDialogText);

        // Fenster wieder gross
        m_pWindow->SetOutputSizePixel(aOldDialogSize);

        // pEditCell an alte Position
        pRefEdit->SetPosSizePixel(aOldEditPos, aOldEditSize);

        // set button position and image
        if( pRefBtn )
        {
            pRefBtn->SetParent(m_pWindow);
            pRefBtn->SetPosPixel( aOldButtonPos );
            pRefBtn->SetStartImage();
        }

        // Alle anderen: Show();
        sal_uInt16 nChildren = m_pWindow->GetChildCount();
        for ( sal_uInt16 i = 0; i < nChildren; i++ )
            if (pHiddenMarks[i])
            {
                m_pWindow->GetChild(i)->GetWindow( WINDOW_CLIENT )->Show();
            }
        delete [] pHiddenMarks;

        pRefEdit = NULL;
        pRefBtn = NULL;
    }
}
// -----------------------------------------------------------------------------
void ScFormulaReferenceHelper::RefInputStart( formula::RefEdit* pEdit, formula::RefButton* pButton )
{
    if (!pRefEdit)
    {
        pRefEdit = pEdit;
        pRefBtn  = pButton;

        // Neuen Fenstertitel basteln
        String sNewDialogText;
        sOldDialogText = m_pWindow->GetText();
        sNewDialogText  = sOldDialogText;
        sNewDialogText.AppendAscii(RTL_CONSTASCII_STRINGPARAM( ": " ));

        mpOldEditParent = pRefEdit->GetParent();

        // Alte Daten merken
        aOldDialogSize = m_pWindow->GetOutputSizePixel();
        aOldEditPos = pRefEdit->GetPosPixel();
        aOldEditSize = pRefEdit->GetSizePixel();
        if (pRefBtn)
            aOldButtonPos = pRefBtn->GetPosPixel();

        pRefEdit->SetParent(m_pWindow);
        if(pRefBtn)
            pRefBtn->SetParent(m_pWindow);

        // Alle Elemente ausser EditCell und Button verstecken
        sal_uInt16 nChildren = m_pWindow->GetChildCount();
        pHiddenMarks = new bool [nChildren];
        for (sal_uInt16 i = 0; i < nChildren; i++)
        {
            pHiddenMarks[i] = false;
            Window* pWin = m_pWindow->GetChild(i);
            pWin = pWin->GetWindow( WINDOW_CLIENT );
            if (pWin == (Window*)pRefEdit)
            {
                sNewDialogText += m_pWindow->GetChild(i-1)->GetWindow( WINDOW_CLIENT )->GetText();
            }
            else if (pWin == (Window*)pRefBtn)
                ;   // do nothing
            else if (pWin->IsVisible())
            {
                pHiddenMarks[i] = true;
                pWin->Hide();
            }
        }

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

        // set button position and image
        if( pRefBtn )
        {
            pRefBtn->SetPosPixel( Point( aOldDialogSize.Width() - pRefBtn->GetSizePixel().Width(), 0 ) );
            pRefBtn->SetEndImage();
        }

        // Fenster verkleinern
        m_pWindow->SetOutputSizePixel(aNewDlgSize);

        // Fenstertitel anpassen
        m_pWindow->SetText( MnemonicGenerator::EraseAllMnemonicChars( sNewDialogText ) );

        if (!pAccel.get())
        {
            pAccel.reset( new Accelerator );
            pAccel->InsertItem( 1, KeyCode( KEY_RETURN ) );
            pAccel->InsertItem( 2, KeyCode( KEY_ESCAPE ) );
            pAccel->SetSelectHdl( LINK( this, ScFormulaReferenceHelper, AccelSelectHdl ) );
        }
        Application::InsertAccel( pAccel.get() );
        bAccInserted = true;
    }
}
// -----------------------------------------------------------------------------
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
// -----------------------------------------------------------------------------
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
// -----------------------------------------------------------------------------
void ScFormulaReferenceHelper::ViewShellChanged(ScTabViewShell* /* pScViewShell */)
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

//----------------------------------------------------------------------------



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
//----------------------------------------------------------------------------

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

//============================================================================
//The class of ScAnyRefDlg is rewritten by PengYunQuan for Validity Cell Range Picker
//  class ScRefHandler
//----------------------------------------------------------------------------

ScRefHandler::ScRefHandler( Window &rWindow, SfxBindings* pB/*, SfxChildWindow* pCW,
                          Window* pParent, sal_uInt16 nResId*/, bool bBindRef )
    :   //SfxModelessDialog ( pB, pCW, pParent, ScResId( nResId ) ),
        m_rWindow( rWindow ),
        m_bInRefMode( false ),
        m_aHelper(this,pB),
        pMyBindings( pB ),
        pActiveWin(NULL)
{
    m_aHelper.SetWindow(/*this*/&m_rWindow);
    if(m_rWindow.GetHelpId().isEmpty())                //Hack, da im SfxModelessDialog die HelpId
        m_rWindow.SetHelpId(m_rWindow.GetUniqueId());   //fuer einen ModelessDialog entfernt und
                                    //in eine UniqueId gewandelt wird, machen
                                    //wir das an dieser Stelle rueckgaengig.
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
                    pScViewShell->UpdateInputHandler(sal_True);
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

//----------------------------------------------------------------------------

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
        pScViewShell->UpdateInputHandler(sal_True);

    lcl_InvalidateWindows();

    m_bInRefMode = false;
    return true;
}

//----------------------------------------------------------------------------

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

//----------------------------------------------------------------------------

sal_Bool ScRefHandler::IsDocAllowed(SfxObjectShell* pDocSh) const   // pDocSh may be 0
{
    //  default: allow only same document (overridden in function dialog)
    String aCmpName;
    if ( pDocSh )
        aCmpName = pDocSh->GetTitle();

    //  if aDocName isn't initialized, allow
    return ( aDocName.Len() == 0 || aDocName == aCmpName );
}

//----------------------------------------------------------------------------

sal_Bool ScRefHandler::IsRefInputMode() const
{
    return m_rWindow.IsVisible(); // nur wer sichtbar ist kann auch Referenzen bekommen
}

//----------------------------------------------------------------------------

sal_Bool ScRefHandler::DoClose( sal_uInt16 nId )
{
    m_aHelper.DoClose(nId);
    return sal_True;
}

void ScRefHandler::SetDispatcherLock( bool bLock )
{
    m_aHelper.SetDispatcherLock( bLock );
}

//----------------------------------------------------------------------------

void ScRefHandler::ViewShellChanged(ScTabViewShell*  pScViewShell )
{
    m_aHelper.ViewShellChanged(pScViewShell);
}

//----------------------------------------------------------------------------

void ScRefHandler::AddRefEntry()
{
    //  wenn nicht ueberladen, gibt es keine Mehrfach-Referenzen
}

//----------------------------------------------------------------------------

sal_Bool ScRefHandler::IsTableLocked() const
{
    // per Default kann bei Referenzeingabe auch die Tabelle umgeschaltet werden

    return false;
}

//----------------------------------------------------------------------------
//
//  RefInputStart/Done: Zoom-In (AutoHide) auf einzelnes Feld
//  (per Button oder Bewegung)
//
//----------------------------------------------------------------------------

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
            m_aHelper.SetDispatcherLock( sal_True );
            aTimer.Start();
        }
        else
        {
            m_aHelper.enableInput( sal_True );
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
// -----------------------------------------------------------------------------
bool ScRefHandler::ParseWithNames( ScRangeList& rRanges, const String& rStr, ScDocument* pDoc )
{
    return m_aHelper.ParseWithNames( rRanges, rStr, pDoc );
}
// -----------------------------------------------------------------------------
void ScRefHandler::HideReference( sal_Bool bDoneRefMode )
{
    m_aHelper.HideReference( bDoneRefMode );
}
// -----------------------------------------------------------------------------
void ScRefHandler::ShowReference( const XubString& rStr )
{
    m_aHelper.ShowReference( rStr );
}
// -----------------------------------------------------------------------------
void ScRefHandler::ReleaseFocus( formula::RefEdit* pEdit, formula::RefButton* pButton )
{
    m_aHelper.ReleaseFocus( pEdit,pButton );
}
//----------------------------------------------------------------------------
void ScRefHandler::RefInputDone( sal_Bool bForced )
{
    m_aHelper.RefInputDone( bForced );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
