/*************************************************************************
 *
 *  $RCSfile: anyrefdg.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:45:02 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PCH
#include "ui_pch.hxx"
#endif

#pragma hdrstop

//----------------------------------------------------------------------------

#include "rangelst.hxx"
#include <sfx2/app.hxx>
#include <sfx2/viewsh.hxx>
#include <vcl/wrkwin.hxx>
#include <tools/shl.hxx>
#include <svtools/taskbar.hxx>
#include <sfx2/topfrm.hxx>


#define ANYREFDG_CXX
#include "anyrefdg.hxx"
#undef ANYREFDG_CXX

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

//----------------------------------------------------------------------------

void lcl_EnableInput( BOOL bEnable )
{
    TypeId aType(TYPE(ScDocShell));
    ScDocShell* pDocShell = (ScDocShell*)SfxObjectShell::GetFirst(&aType);
    while( pDocShell )
    {
        SfxViewFrame* pFrame = SfxViewFrame::GetFirst( pDocShell );
        while( pFrame )
        {
            //  #71577# enable everything except InPlace, including bean frames
            if ( !pFrame->ISA(SfxInPlaceFrame) )
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
                            pParent->EnableInput(bEnable,TRUE /* FALSE */);
                            if(TRUE /*bChilds*/)
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

void lcl_InvalidateWindows()
{
//  Application::GetAppWindow()->Invalidate(); //Weils so schoen ist!

    TypeId aType(TYPE(ScDocShell));
    ScDocShell* pDocShell = (ScDocShell*)SfxObjectShell::GetFirst(&aType);
    while( pDocShell )
    {
        SfxViewFrame* pFrame = SfxViewFrame::GetFirst( pDocShell );
        while( pFrame )
        {
            //  #71577# enable everything except InPlace, including bean frames
            if ( !pFrame->ISA(SfxInPlaceFrame) )
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

//============================================================================
//  class ScRefEdit
//----------------------------------------------------------------------------

#define SC_ENABLE_TIME 100

ScRefEdit::ScRefEdit( ScAnyRefDlg* pParent, const ResId& rResId):
            Edit(pParent,rResId)
{
    pAnyRefDlg=pParent;
    aTimer.SetTimeoutHdl(LINK( this, ScRefEdit, UpdateHdl) );
    aTimer.SetTimeout(SC_ENABLE_TIME);
}

ScRefEdit::ScRefEdit( Window *pParent, const ResId& rResId):
            Edit(pParent,rResId)
{
    pAnyRefDlg=NULL;
}

ScRefEdit::~ScRefEdit()
{
    aTimer.SetTimeoutHdl(Link());
    aTimer.Stop();
}

void ScRefEdit::SetRefDialog(ScAnyRefDlg *pDlg)
{
    pAnyRefDlg=pDlg;

    if(pDlg!=NULL)
    {
        aTimer.SetTimeoutHdl(LINK( this, ScRefEdit, UpdateHdl) );
        aTimer.SetTimeout(SC_ENABLE_TIME);
    }
    else
    {
        aTimer.SetTimeoutHdl(Link());
        aTimer.Stop();
    }
}

void ScRefEdit::StartUpdateData()
{
    aTimer.Start();
}

void ScRefEdit::SetText( const XubString& rStr )
{
    Edit::SetText(rStr);
    UpdateHdl(&aTimer);
}

void ScRefEdit::SetRefString( const XubString& rStr )
{
    Edit::SetText(rStr);
}

void ScRefEdit::GetFocus()
{
    Edit::GetFocus();
    StartUpdateData();
}

void ScRefEdit::Modify()
{
    Edit::Modify();
    if(pAnyRefDlg!=NULL)
    {
        pAnyRefDlg->HideReference();
    }
}


void ScRefEdit::LoseFocus()
{
    Edit::LoseFocus();

    if(pAnyRefDlg!=NULL)
    {
        pAnyRefDlg->HideReference();
    }
}

IMPL_LINK( ScRefEdit, UpdateHdl, Timer*, pTi)
{
    if(pAnyRefDlg!=NULL)
    {
        pAnyRefDlg->ShowReference(GetText());
    }

    return 0;
}


//============================================================================
//  class ScRefButton
//----------------------------------------------------------------------------

ScRefButton::ScRefButton( ScAnyRefDlg* pParent, const ResId& rResId, ScRefEdit* pEdit )
    :   ImageButton ( pParent, rResId ),
        aImgRefStart(ScResId( RID_BMP_REFBTN1 )),
        aImgRefDone (ScResId( RID_BMP_REFBTN2 )),
        pDlg        ( pParent ),
        pRefEdit    ( pEdit )
{
    SetImage(aImgRefStart);
}

ScRefButton::ScRefButton( Window *pParent, const ResId& rResId)
    :   ImageButton ( pParent, rResId ),
        aImgRefStart( ScResId( RID_BMP_REFBTN1 )),
        aImgRefDone ( ScResId( RID_BMP_REFBTN2 )),
        pDlg        ( NULL),
        pRefEdit    ( NULL)
{
    SetImage(aImgRefStart);
}

void ScRefButton::SetReferences( ScAnyRefDlg* pRefDlg,ScRefEdit* pEdit )
{
    pDlg    =pRefDlg;
    pRefEdit=pEdit;
}


//----------------------------------------------------------------------------

//  wenn der Button reingedrueckt erscheinen soll,
//  Check(FALSE/TRUE) statt SetImage

void ScRefButton::Click()
{
    if(pDlg!=NULL && pRefEdit!=NULL)
    {
        if (pDlg->pRefEdit == pRefEdit)             // Ref.-Input aktiv auf Edit?
        {
            SetImage(aImgRefStart);                 // Image aendern
            pDlg->RefInputDone(TRUE);               // Ref.-Eingabe beenden
        }
        else
        {
            pDlg->RefInputDone(TRUE);               // sicherheitshalber
            pDlg->RefInputStart(pRefEdit, this);    // Ref.-Eingabe starten
            pRefEdit->GrabFocus();                  // Focus auf Edit
            SetImage(aImgRefDone);                  // Image aendern
        }
    }
}

void ScRefButton::SetStartImage()               // fuer Aufruf von aussen
{
    SetImage(aImgRefStart);
}

void ScRefButton::SetEndImage()
{
    SetImage(aImgRefDone);
}

void ScRefButton::GetFocus()
{
    ImageButton::GetFocus();
    if(pRefEdit!=NULL) pRefEdit->StartUpdateData();
}


void ScRefButton::LoseFocus()
{
    ImageButton::LoseFocus();

    if(pRefEdit!=NULL) pRefEdit->Modify();
}

//----------------------------------------------------------------------------

void lcl_HideAllReferences()
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
//  class ScAnyRefDlg
//----------------------------------------------------------------------------

ScAnyRefDlg::ScAnyRefDlg( SfxBindings* pB, SfxChildWindow* pCW,
                          Window* pParent, USHORT nResId)
    :   SfxModelessDialog ( pB, pCW, pParent, ScResId( nResId ) ),
        pRefEdit( NULL ),
        pAccel( NULL ),
        bAccInserted( FALSE ),
        bHighLightRef( FALSE ),
        bEnableColorRef( FALSE ),
        pRefCell(NULL),
        pRefComp(NULL),
        pActiveWin(NULL)
{
    if(GetHelpId()==0)              //Hack, da im SfxModelessDialog die HelpId
        SetHelpId(GetUniqueId());   //fuer einen ModelessDialog entfernt und
                                    //in eine UniqueId gewandelt wird, machen
                                    //wir das an dieser Stelle rueckgaengig.
    aTimer.SetTimeout(200);
    aTimer.SetTimeoutHdl(LINK( this, ScAnyRefDlg, UpdateFocusHdl));

    SC_MOD()->InputEnterHandler();
    ScTabViewShell* pScViewShell = PTR_CAST(ScTabViewShell, SfxViewShell::Current());
    if ( pScViewShell )
    {
        pScViewShell->UpdateInputHandler(TRUE);

        ScDocShell* pDocSh = pScViewShell->GetViewData()->GetDocShell();
        aDocName = pDocSh->GetTitle();
    }

    ScInputHandler* pInputHdl = SC_MOD()->GetInputHdl();

    DBG_ASSERT( pInputHdl, "Missing input handler :-/" );

    if ( pInputHdl )
        pInputHdl->NotifyChange( NULL );

    lcl_EnableInput( FALSE );

    EnableSpreadsheets();

    ScInputOptions aInputOption=SC_MOD()->GetInputOptions();
    bEnableColorRef=aInputOption.GetRangeFinder();

    ScViewData* pViewData=ScDocShell::GetViewData();
    if ( pViewData )
    {
        ScDocument* pDoc = pViewData->GetDocument();
        USHORT nCol = pViewData->GetCurX();
        USHORT nRow = pViewData->GetCurY();
        USHORT nTab = pViewData->GetTabNo();
        ScAddress aCursorPos( nCol, nRow, nTab );

        String rStrExp;
        pRefCell = new ScFormulaCell( pDoc, aCursorPos, rStrExp );
        pRefComp=new ScCompiler( pDoc, aCursorPos );
        pRefComp->SetCompileForFAP(TRUE);
    }


    SFX_APP()->LockDispatcher( TRUE );
    //@Test
    //SFX_APPWINDOW->Disable(TRUE);   //@BugID 54702
}

//----------------------------------------------------------------------------

ScAnyRefDlg::~ScAnyRefDlg()
{
    HideReference();
    lcl_HideAllReferences();

    lcl_EnableInput( TRUE );
    SetModalInputMode(FALSE);
    SFX_APP()->LockDispatcher( FALSE );         //! hier und in DoClose ?

    delete pRefComp;
    delete pRefCell;

    ScTabViewShell* pScViewShell = PTR_CAST(ScTabViewShell, SfxViewShell::Current());
    if ( pScViewShell )
    {
        pScViewShell->UpdateInputHandler(TRUE);
    }
    if (bAccInserted)
        Application::RemoveAccel( pAccel );
    delete pAccel;

    ScInputHandler* pInputHdl = SC_MOD()->GetInputHdl();

    if ( pInputHdl )                    //@BugID 54702 Da der Timer fuers Disablen
        pInputHdl->ResetDelayTimer();   // noch laufen koennte, Reset ausloesen.

    //SFX_APPWINDOW->Enable(TRUE,TRUE);
//  Application::GetAppWindow()->Invalidate(); //Weils so schoen ist!
    lcl_InvalidateWindows();
}

//----------------------------------------------------------------------------

void ScAnyRefDlg::SwitchToDocument()
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

BOOL ScAnyRefDlg::IsDocAllowed(SfxObjectShell* pDocSh) const    // pDocSh may be 0
{
    //  default: allow only same document (overridden in function dialog)
    String aCmpName;
    if ( pDocSh )
        aCmpName = pDocSh->GetTitle();

    //  if aDocName isn't initialized, allow
    return ( aDocName.Len() == 0 || aDocName == aCmpName );
}

//----------------------------------------------------------------------------

BOOL __EXPORT ScAnyRefDlg::IsRefInputMode() const
{
    return IsVisible(); // nur wer sichtbar ist kann auch Referenzen bekommen
}

//----------------------------------------------------------------------------

BOOL __EXPORT ScAnyRefDlg::DoClose( USHORT nId )
{
    SfxApplication* pSfxApp = SFX_APP();

    pSfxApp->LockDispatcher( FALSE );           //! hier und im dtor ?

    //! remember the view for which the dialog was started
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
    //Application::GetAppWindow()->Enable();

    SC_MOD()->SetRefDialog( nId, FALSE );

    pSfxApp->Broadcast( SfxSimpleHint( FID_KILLEDITVIEW ) );

    ScTabViewShell* pScViewShell = PTR_CAST(ScTabViewShell, SfxViewShell::Current());
    if ( pScViewShell )
        pScViewShell->UpdateInputHandler(TRUE);

    return TRUE;
}

void ScAnyRefDlg::EnableSpreadsheets(BOOL bFlag, BOOL bChilds)
{
    TypeId aType(TYPE(ScDocShell));
    ScDocShell* pDocShell = (ScDocShell*)SfxObjectShell::GetFirst(&aType);
    while( pDocShell )
    {
        SfxViewFrame* pFrame = SfxViewFrame::GetFirst( pDocShell );
        while( pFrame )
        {
            //  #71577# enable everything except InPlace, including bean frames
            if ( !pFrame->ISA(SfxInPlaceFrame) )
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
                            pParent->EnableInput(bFlag,FALSE);
                            if(bChilds)
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

void ScAnyRefDlg::ViewShellChanged(ScTabViewShell* pScViewShell)
{
    lcl_EnableInput( FALSE );

    EnableSpreadsheets();
}

void ScAnyRefDlg::ShowReference( const XubString& rStr )
{
    if(pRefEdit==NULL && bEnableColorRef)
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

void ScAnyRefDlg::ShowSimpleReference( const XubString& rStr )
{
    if(pRefEdit==NULL && bEnableColorRef)
    {
        bHighLightRef=TRUE;
        ScViewData* pViewData=ScDocShell::GetViewData();
        if ( pViewData )
        {
            ScDocument* pDoc=pViewData->GetDocument();
            ScTabViewShell* pTabViewShell=pViewData->GetViewShell();

            ScRangeList aRangeList;

            pTabViewShell->DoneRefMode( FALSE );
            pTabViewShell->ClearHighlightRanges();

            if(aRangeList.Parse(rStr ,pDoc))
            {
                ScRange* pRangeEntry=aRangeList.First();

                USHORT nIndex=0;
                while(pRangeEntry!=NULL)
                {
                    ColorData aColName=ScRangeFindList::GetColorName(nIndex++);
                    pTabViewShell->AddHighlightRange(*pRangeEntry, aColName);

                    pRangeEntry=aRangeList.Next();
                }
            }
        }
    }
}

void ScAnyRefDlg::ShowFormulaReference( const XubString& rStr )
{
    if(pRefEdit==NULL && bEnableColorRef)
    {
        bHighLightRef=TRUE;
        ScViewData* pViewData=ScDocShell::GetViewData();
        if ( pViewData && pRefComp )
        {
            ScDocument* pDoc=pViewData->GetDocument();
            ScTabViewShell* pTabViewShell=pViewData->GetViewShell();
            USHORT nCol = pViewData->GetCurX();
            USHORT nRow = pViewData->GetCurY();
            USHORT nTab = pViewData->GetTabNo();
            ScAddress aPos( nCol, nRow, nTab );

            ScTokenArray* pScTokA=pRefComp->CompileString(rStr);
            //pRefComp->CompileTokenArray();

            if(pTabViewShell!=NULL && pScTokA!=NULL)
            {
                pTabViewShell->DoneRefMode( FALSE );
                pTabViewShell->ClearHighlightRanges();

                pScTokA->Reset();
                ScToken*  pToken=pScTokA->GetNextReference();

                USHORT nIndex=0;

                while(pToken!=NULL)
                {
                    BOOL bDoubleRef=(pToken->GetType()==svDoubleRef);


                    if(pToken->GetType()==svSingleRef || bDoubleRef)
                    {
                        ScRange aRange;
                        ComplRefData aRef( pToken->GetReference() );
                        aRef.Ref1.CalcAbsIfRel( aPos );
                        if(bDoubleRef)
                        {
                            aRef.Ref2.CalcAbsIfRel( aPos );
                            aRange=ScRange( aRef.Ref1.nCol, aRef.Ref1.nRow, aRef.Ref1.nTab,
                                        aRef.Ref2.nCol, aRef.Ref2.nRow, aRef.Ref2.nTab);
                        }
                        else
                        {
                            aRange=ScRange( aRef.Ref1.nCol, aRef.Ref1.nRow, aRef.Ref1.nTab);
                        }
                        ColorData aColName=ScRangeFindList::GetColorName(nIndex++);
                        pTabViewShell->AddHighlightRange(aRange, aColName);
                    }

                    pToken=pScTokA->GetNextReference();
                }
            }
            if(pScTokA!=NULL) delete pScTokA;
        }
    }
}

void ScAnyRefDlg::HideReference()
{
    ScViewData* pViewData=ScDocShell::GetViewData();

    if(pViewData!=NULL  && pRefEdit==NULL &&
            bHighLightRef  && bEnableColorRef)
    {
        ScTabViewShell* pTabViewShell=pViewData->GetViewShell();

        if(pTabViewShell!=NULL)
        {
            pTabViewShell->DoneRefMode( FALSE );
            pTabViewShell->ClearHighlightRanges();
        }
        bHighLightRef=FALSE;
    }
}

//----------------------------------------------------------------------------

void ScAnyRefDlg::AddRefEntry()
{
    //  wenn nicht ueberladen, gibt es keine Mehrfach-Referenzen
}

//----------------------------------------------------------------------------

BOOL __EXPORT ScAnyRefDlg::IsTableLocked() const
{
    // per Default kann bei Referenzeingabe auch die Tabelle umgeschaltet werden

    return FALSE;
}

//----------------------------------------------------------------------------
//
//  RefInputStart/Done: Zoom-In (AutoHide) auf einzelnes Feld
//  (per Button oder Bewegung)
//
//----------------------------------------------------------------------------

void ScAnyRefDlg::RefInputStart( ScRefEdit* pEdit, ScRefButton* pButton )
{
    if (!pRefEdit)
    {
        pRefEdit = pEdit;
        pRefBtn  = pButton;

        // Neuen Fenstertitel basteln
        String sNewDialogText;
        sOldDialogText = GetText();
        sNewDialogText  = sOldDialogText;
        sNewDialogText.AppendAscii(RTL_CONSTASCII_STRINGPARAM( ": " ));

        // Alle Elemente ausser EditCell und Button verstecken
        USHORT nChildren = GetChildCount();
        pHiddenMarks = new BOOL [nChildren];
        for (USHORT i = 0; i < nChildren; i++)
        {
            pHiddenMarks[i] = FALSE;
            Window* pWin = GetChild(i);
#ifdef VCL
            pWin = pWin->GetWindow( WINDOW_CLIENT );
#endif
            if (pWin == (Window*)pRefEdit)
            {
#ifdef VCL
                sNewDialogText += GetChild(i-1)->GetWindow( WINDOW_CLIENT )->GetText();
#else
                sNewDialogText += GetChild(i-1)->GetText();
#endif
            }
            else if (pWin == (Window*)pRefBtn)
                ;   // do nothing
            else if (pWin->IsVisible())
            {
                pHiddenMarks[i] = TRUE;
                pWin->Hide();
            }
        }

        // Alte Daten merken
        aOldDialogSize = GetOutputSizePixel();
        aOldEditPos = pRefEdit->GetPosPixel();
        aOldEditSize = pRefEdit->GetSizePixel();
        if (pRefBtn)
            aOldButtonPos = pRefBtn->GetPosPixel();

        // Edit-Feld verschieben und anpassen
        Size aNewDlgSize(aOldDialogSize.Width(), aOldEditSize.Height());
        Size aNewEditSize(aNewDlgSize);
        if (pRefBtn)
        {
            aNewEditSize.Width() -= pRefBtn->GetSizePixel().Width();
            aNewEditSize.Width() -= aOldButtonPos.X() - (aOldEditPos.X()+aOldEditSize.Width());
        }
        pRefEdit->SetPosSizePixel(Point(0, 0), aNewEditSize);

        // Button verschieben
        if (pRefBtn)
            pRefBtn->SetPosPixel(Point(aOldDialogSize.Width()-pRefBtn->GetSizePixel().Width(), 0));

        // Fenster verkleinern
        SetOutputSizePixel(aNewDlgSize);

        // Fenstertitel anpassen
        sNewDialogText.EraseAllChars('~');
        SetText(sNewDialogText);

        if ( pButton )      // ueber den Button: Enter und Escape abfangen
        {
            if (!pAccel)
            {
                pAccel = new Accelerator;
                pAccel->InsertItem( 1, KeyCode( KEY_RETURN ) );
                pAccel->InsertItem( 2, KeyCode( KEY_ESCAPE ) );
                pAccel->SetSelectHdl( LINK( this, ScAnyRefDlg, AccelSelectHdl ) );
            }
            Application::InsertAccel( pAccel );
            bAccInserted = TRUE;
        }
    }
}

//----------------------------------------------------------------------------

void ScAnyRefDlg::RefInputDone( BOOL bForced )
{
    if (pRefEdit && (bForced || !pRefBtn))
    {
        if (bAccInserted)           // Accelerator wieder abschalten
        {
            Application::RemoveAccel( pAccel );
            bAccInserted = FALSE;
        }

        // Fenstertitel anpassen
        SetText(sOldDialogText);

        // Fenster wieder gross
        SetOutputSizePixel(aOldDialogSize);

        // pEditCell an alte Position
        pRefEdit->SetPosSizePixel(aOldEditPos, aOldEditSize);
        if (pRefBtn)
            pRefBtn->SetPosPixel(aOldButtonPos);

        // Alle anderen: Show();
        USHORT nChildren = GetChildCount();
        for ( USHORT i = 0; i < nChildren; i++ )
            if (pHiddenMarks[i])
            {
#ifdef VCL
                GetChild(i)->GetWindow( WINDOW_CLIENT )->Show();
#else
                GetChild(i)->Show();
#endif
            }
        delete [] pHiddenMarks;

        pRefEdit = NULL;
    }
}

long ScAnyRefDlg::PreNotify( NotifyEvent& rNEvt )
{
    USHORT nSwitch=rNEvt.GetType();
    if(nSwitch==EVENT_GETFOCUS)
    {
        pActiveWin=rNEvt.GetWindow();
    }
    return SfxModelessDialog::PreNotify(rNEvt);
}

void ScAnyRefDlg::StateChanged( StateChangedType nStateChange )
{
    SfxModelessDialog::StateChanged( nStateChange );

    if(nStateChange == STATE_CHANGE_VISIBLE)
    {
        if(IsVisible())
        {
            lcl_EnableInput( FALSE );
            EnableSpreadsheets();
            SFX_APP()->LockDispatcher( TRUE );
            aTimer.Start();
        }
        else
        {
            lcl_EnableInput( TRUE );
            SFX_APP()->LockDispatcher( FALSE );         //! hier und in DoClose ?
        }
    }
}

IMPL_LINK( ScAnyRefDlg, UpdateFocusHdl, Timer*, pTi)
{
    if (pActiveWin)
    {
        pActiveWin->GrabFocus();
    }
    return 0;
}

//----------------------------------------------------------------------------

IMPL_LINK( ScAnyRefDlg, AccelSelectHdl, Accelerator *, pAccel )
{
    if ( !pAccel )
        return 0;

    switch ( pAccel->GetCurKeyCode().GetCode() )
    {
        case KEY_RETURN:
        case KEY_ESCAPE:
            //  #57383# der SimpleRefDlg loescht sich bei RefInputDone,
            //  darum Button umschalten vorher...
            if (pRefBtn)
                pRefBtn->SetStartImage();       // Image aendern
            RefInputDone(TRUE);                 // Ref.-Eingabe beenden
            break;

        default:
        break;
    }
    return TRUE;
}




