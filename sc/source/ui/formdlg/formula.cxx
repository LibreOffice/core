/*************************************************************************
 *
 *  $RCSfile: formula.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:44:57 $
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

#include "scitems.hxx"
#include <sfx2/dispatch.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/objsh.hxx>
#include <svtools/zforlist.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/topfrm.hxx>
#include <vcl/svapp.hxx>
#include <unotools/charclass.hxx>

#ifndef _SFXSTRITEM_HXX //autogen
#include <svtools/stritem.hxx>
#endif

#include "formula.hxx"
#include "formdlgs.hrc"
#include "formdata.hxx"
#include "globstr.hrc"
#include "scresid.hxx"
#include "reffact.hxx"
#include "document.hxx"
#include "cell.hxx"
#include "scmod.hxx"
#include "inputhdl.hxx"
#include "tabvwsh.hxx"
#include "appoptio.hxx"
#include "docsh.hxx"

//============================================================================

ScDocument* ScFormulaDlg::pDoc = NULL;
ScAddress ScFormulaDlg::aCursorPos;



inline void ShowHide( Window& rWin, BOOL bShow )
{
    if (bShow)
        rWin.Show();
    else
        rWin.Hide();
}

//  --------------------------------------------------------------------------
//      Initialisierung / gemeinsame Funktionen  fuer Dialog
//  --------------------------------------------------------------------------

ScFormulaDlg::ScFormulaDlg( SfxBindings* pB, SfxChildWindow* pCW,
                                    Window* pParent, ScViewData* pViewData ) :
        ScAnyRefDlg     ( pB, pCW, pParent, RID_SCDLG_FORMULA ),
        //
        aTabCtrl        ( this, ScResId( TC_FUNCTION ) ),
        aGbEdit         ( this, ScResId( GB_EDIT ) ),
        aScParaWin      ( this, aGbEdit.GetPosPixel()),
        aFtHeadLine     ( this, ScResId( FT_HEADLINE ) ),
        aFtFuncName     ( this, ScResId( FT_FUNCNAME ) ),
        aFtFuncDesc     ( this, ScResId( FT_FUNCDESC ) ),
        //
        aFtEditName     ( this, ScResId( FT_EDITNAME ) ),
        aFtResult       ( this, ScResId( FT_RESULT ) ),
        aWndResult      ( this, ScResId( WND_RESULT ) ),

        aFtFormula      ( this, ScResId( FT_FORMULA ) ),
        aMEFormula      ( this, ScResId( ED_FORMULA ) ),
        //
        aBtnHelp        ( this, ScResId( BTN_HELP ) ),
        aBtnCancel      ( this, ScResId( BTN_CANCEL ) ),
        aBtnBackward    ( this, ScResId( BTN_BACKWARD ) ),
        aBtnForward     ( this, ScResId( BTN_FORWARD ) ),
        aBtnMatrix      ( this, ScResId( BTN_MATRIX ) ),
        aBtnEnd         ( this, ScResId( BTN_END ) ),
        aEdRef          ( this, ScResId( ED_REF) ),
        aRefBtn         ( this, ScResId( RB_REF),&aEdRef ),
        aFtFormResult   ( this, ScResId( FT_FORMULA_RESULT)),
        aWndFormResult  ( this, ScResId( WND_FORMULA_RESULT)),
        //
        aTitle1         ( ScResId( STR_TITLE1 ) ),      // lokale Resource
        aTitle2         ( ScResId( STR_TITLE2 ) ),      // lokale Resource
        aTxtEnd         ( ScResId( SCSTR_END ) ),       // globale Resource
        aTxtOk          ( aBtnEnd.GetText() ),
        //
        nActivWinId     (0),
        bIsShutDown     (FALSE),
        nEdFocus        (0),
        nArgs           (0),
        bUserMatrixFlag (FALSE),
        pArgArr         (NULL),
        pFuncDesc       (NULL),
        pScTokA         (NULL),
        pTheRefEdit     (NULL),
        pMEdit          (NULL)
{
    FreeResource();
    SetText(aTitle1);

    aEdRef.Hide();
    aRefBtn.Hide();

    pMEdit=aMEFormula.GetEdit();
    bEditFlag=FALSE;
    bStructUpdate=TRUE;
    Point aPos=aGbEdit.GetPosPixel();
    aScParaWin.SetPosPixel(aPos);
    aScParaWin.SetArgModifiedHdl(LINK( this, ScFormulaDlg, ModifyHdl ) );
    aScParaWin.SetFxHdl(LINK( this, ScFormulaDlg, FxHdl ) );

    pScFuncPage= new    ScFuncPage( &aTabCtrl);
    pScStructPage= new  ScStructPage( &aTabCtrl);
    pScFuncPage->Hide();
    pScStructPage->Hide();
    aTabCtrl.SetTabPage( TP_FUNCTION, pScFuncPage);
    aTabCtrl.SetTabPage( TP_STRUCT, pScStructPage);

    nOldHelp = GetHelpId();             // HelpId aus Resource immer fuer "Seite 1"
    nOldUnique = GetUniqueId();

    aBtnMatrix.SetClickHdl(LINK( this, ScFormulaDlg, MatrixHdl ) );
    aBtnCancel  .SetClickHdl( LINK( this, ScFormulaDlg, BtnHdl ) );
    aBtnEnd     .SetClickHdl( LINK( this, ScFormulaDlg, BtnHdl ) );
    aBtnForward .SetClickHdl( LINK( this, ScFormulaDlg, BtnHdl ) );
    aBtnBackward.SetClickHdl( LINK( this, ScFormulaDlg, BtnHdl ) );

    pScFuncPage->SetDoubleClickHdl( LINK( this, ScFormulaDlg, DblClkHdl ) );
    pScFuncPage->SetSelectHdl( LINK( this, ScFormulaDlg, FuncSelHdl) );
    pScStructPage->SetSelectionHdl( LINK( this, ScFormulaDlg, StructSelHdl ) );
    pMEdit->SetModifyHdl( LINK( this, ScFormulaDlg, FormulaHdl ) );
    aMEFormula  .SetSelChangedHdl( LINK( this, ScFormulaDlg, FormulaCursorHdl ) );

    aFntLight = aFtFormula.GetFont();
    aFntLight.SetTransparent( TRUE );
    aFntBold = aFntLight;
    aFntBold.SetWeight( WEIGHT_BOLD );

    aScParaWin.SetArgumentFonts(aFntBold,aFntLight);

    aFntBold.SetColor(Color(COL_BLUE));
    aFntLight.SetColor(Color(COL_BLUE));

    aFtHeadLine.SetFont(aFntBold);
    aFtFuncName.SetFont(aFntLight);
    aFtFuncDesc.SetFont(aFntLight);

    ScModule* pScMod = SC_MOD();

    ScInputHandler* pInputHdl = pScMod->GetInputHdl();
    if ( pInputHdl )
        pInputHdl->NotifyChange( NULL );


    ScFormEditData* pData = pScMod->GetFormEditData();
    String rStrExp;
    if (pData)
    {
        //  Daten schon vorhanden -> Zustand wiederherstellen (nach Umschalten)
        //  pDoc und aCursorPos nicht neu initialisieren
        //pDoc = pViewData->GetDocument();
        if(IsInputHdl(pData->GetInputHandler()))
        {
            pScMod->SetRefInputHdl(pData->GetInputHandler());
        }
        else
        {
            PtrTabViewShell pTabViewShell;
            pInputHdl=GetNextInputHandler(pData->GetDocShell(),&pTabViewShell);

            if(pInputHdl==NULL) //DocShell hat keinen InputHandler mehr,
            {                   //hat der Anwender halt Pech gehabt.
                aBtnEnd.Disable();
                pInputHdl=pScMod->GetInputHdl();
            }
            else
            {
                pInputHdl->SetRefViewShell(pTabViewShell);
            }
            pScMod->SetRefInputHdl(pInputHdl);
            pData->SetInputHandler(pInputHdl);
        }

        String aOldFormula(pScMod->InputGetFormulaStr());
        pScMod->InputSetSelection( 0, aOldFormula.Len());

        rStrExp=pData->GetUndoStr();
        pScMod->InputReplaceSelection(rStrExp);
        pMEdit->SetText(rStrExp);
        xub_StrLen nPos=pData->GetFStart();
        pMEdit->SetSelection( pData->GetSelection());

        pCell = new ScFormulaCell( pDoc, aCursorPos, rStrExp );
        pComp=new ScCompiler( pDoc, aCursorPos );
        pComp->SetCompileForFAP(TRUE);
        UpdateTokenArray(pMEdit->GetText());
        FormulaCursorHdl(&aMEFormula);
        CalcStruct(rStrExp);
        if(pData->GetMode()==SC_FORMDLG_FORMULA)
            aTabCtrl.SetCurPageId(TP_FUNCTION);
        else
            aTabCtrl.SetCurPageId(TP_STRUCT);
        aBtnMatrix.Check(pData->GetMatrixFlag());
        aTimer.SetTimeout(200);
        aTimer.SetTimeoutHdl(LINK( this, ScFormulaDlg, UpdateFocusHdl));
        aTimer.Start();

        // Jetzt nochmals zurueckschalten, da evtl. neues Doc geoeffnet wurde!
        pScMod->SetRefInputHdl(NULL);
    }
    else
    {
        //Nun wird es Zeit den Inputhandler festzulegen
        pScMod->SetRefInputHdl(pScMod->GetInputHdl());

        pDoc = pViewData->GetDocument();
        USHORT nCol = pViewData->GetCurX();
        USHORT nRow = pViewData->GetCurY();
        USHORT nTab = pViewData->GetTabNo();
        aCursorPos = ScAddress( nCol, nRow, nTab );

        pScMod->InitFormEditData();                             // neu anlegen
        ScFormEditData* pData = pScMod->GetFormEditData();
        pData->SetInputHandler(pScMod->GetInputHdl());
        pData->SetDocShell(pViewData->GetDocShell());

        DBG_ASSERT(pData,"FormEditData ist nicht da");

        ScFormulaDlgMode eMode = SC_FORMDLG_FORMULA;            // Default...

        //  Formel vorhanden? Dann editieren

        String aFormula;
        pDoc->GetFormula( nCol, nRow, nTab, aFormula );
        BOOL bEdit   = ( aFormula.Len() > 1 );
        BOOL bMatrix = FALSE;
        if ( bEdit )
        {
            pMEdit->GrabFocus();
            xub_StrLen nLen = aFormula.Len();
            bMatrix =  nLen > 3                     // Matrix-Formel ?
                    && aFormula.GetChar(0) == '{'
                    && aFormula.GetChar(1) == '='
                    && aFormula.GetChar(nLen-1) == '}';
            if ( bMatrix )
            {
                aFormula.Erase( 0, 1 );
                aFormula.Erase( aFormula.Len()-1, 1);
                aBtnMatrix.Check( bMatrix );
                aBtnMatrix.Disable();
            }

            // #40892# auch Formeln mit Fehlern koennen editiert werden
            // (ob ueberhaupt gueltige Funktionen enthalten sind, wird bei
            // ScFormulaUtil::GetNextFunc getestet)
#if 0
            //  Test auf Fehler (ohne Interpretieren, also nur Compiler-Fehler)
            ScFormulaCell aCell( pDoc, aCursorPos, aFormula);
            BOOL bAutoCalc = pDoc->GetAutoCalc();
            pDoc->SetAutoCalc( FALSE );             // Interpretieren fuer GetErrCode abstellen
            USHORT  nErrCode = aCell.GetErrCode();
            pDoc->SetAutoCalc( bAutoCalc );
            bEdit = ( nErrCode == 0 );
#endif
        }

        if ( bEdit )
        {
            aTabCtrl.SetCurPageId(TP_STRUCT);

            xub_StrLen nFStart = 0;
            xub_StrLen nFEnd   = 0;
            if ( ScFormulaUtil::GetNextFunc( aFormula, FALSE, nFStart, &nFEnd) )
            {
                pScMod->InputReplaceSelection( aFormula );
                pScMod->InputSetSelection( nFStart, nFEnd );
                if(!bEditFlag)
                    pMEdit->SetText(pScMod->InputGetFormulaStr());
                xub_StrLen PrivStart, PrivEnd;
                pScMod->InputGetSelection( PrivStart, PrivEnd);
                pMEdit->SetSelection( Selection(PrivStart, PrivEnd));
                pMEdit->Invalidate();
                HighlightFunctionParas(pMEdit->GetSelected());
                eMode = SC_FORMDLG_EDIT;

                pData->SetFStart(nFStart );
                aBtnMatrix.Check( bMatrix );
            }
            else
                bEdit = FALSE;
        }

        if ( !bEdit )
        {
            String aNewFormula = '=';
            if(aFormula.Len()>0)
            {
                if ( aFormula.GetChar(0) == '=' )
                    aNewFormula=aFormula;
            }
            pScMod->InputReplaceSelection( aNewFormula );
            pScMod->InputSetSelection( 1, aNewFormula.Len()+1 );
            if(!bEditFlag)
                pMEdit->SetText(pScMod->InputGetFormulaStr());
            xub_StrLen PrivStart, PrivEnd;
            pScMod->InputGetSelection( PrivStart, PrivEnd);
            if(!bEditFlag)
                pMEdit->SetSelection( Selection(PrivStart, PrivEnd));

            pData->SetFStart( 1 );      // hinter dem "="
        }

        pData->SetMode( (USHORT) eMode );
        rStrExp=pMEdit->GetText();
        pCell = new ScFormulaCell( pDoc, aCursorPos, rStrExp );
        pComp=new ScCompiler( pDoc, aCursorPos );
        pComp->SetCompileForFAP(TRUE);
        CalcStruct(rStrExp);
        FillDialog();
        //aBtnForward.Enable(TRUE); //@New
        FuncSelHdl(NULL);

    }

//? Application::GetAppWindow()->Invalidate();
}

__EXPORT ScFormulaDlg::~ScFormulaDlg()
{
    ScModule* pScMod = SC_MOD();
    ScFormEditData* pData = pScMod->GetFormEditData();

    if (pData) // wird nicht ueber Close zerstoert;
    {
        //Referenz Inputhandler zuruecksetzen

        pScMod->SetRefInputHdl(NULL);

        if(aTimer.IsActive())
        {
            aTimer.SetTimeoutHdl(Link());
            aTimer.Stop();
        }
        bIsShutDown=TRUE;// Setzen, damit PreNotify keinen GetFocus speichert.

        pData->SetFStart((xub_StrLen)pMEdit->GetSelection().Min());
        pData->SetSelection(pMEdit->GetSelection());

        if(aTabCtrl.GetCurPageId()==TP_FUNCTION)
            pData->SetMode( (USHORT) SC_FORMDLG_FORMULA );
        else
            pData->SetMode( (USHORT) SC_FORMDLG_EDIT );
        pData->SetUndoStr(pMEdit->GetText());
        pData->SetMatrixFlag(aBtnMatrix.IsChecked());
    }

    aTabCtrl.RemovePage(TP_FUNCTION);
    aTabCtrl.RemovePage(TP_STRUCT);

    delete pComp;
    delete pCell;
    delete pScStructPage;
    delete pScFuncPage;
    DeleteArgs();
}

BOOL ScFormulaDlg::IsInputHdl(ScInputHandler* pHdl)
{
    BOOL bAlive = FALSE;

    //  gehoert der InputHandler zu irgendeiner ViewShell ?

    TypeId aScType = TYPE(ScTabViewShell);
    SfxViewShell* pSh = SfxViewShell::GetFirst( &aScType );
    while ( pSh && !bAlive )
    {
        if (((ScTabViewShell*)pSh)->GetInputHandler() == pHdl)
            bAlive = TRUE;
        pSh = SfxViewShell::GetNext( *pSh, &aScType );
    }

    return bAlive;

}

ScInputHandler* ScFormulaDlg::GetNextInputHandler(ScDocShell* pDocShell,PtrTabViewShell* ppViewSh)
{
    ScInputHandler* pHdl=NULL;

    SfxViewFrame* pFrame = SfxViewFrame::GetFirst( pDocShell, TYPE(SfxTopViewFrame) );
    while( pFrame && pHdl==NULL)
    {
        SfxViewShell* p = pFrame->GetViewShell();
        ScTabViewShell* pViewSh = PTR_CAST(ScTabViewShell,p);
        if(pViewSh!=NULL)
        {
            Window *pWin=pViewSh->GetWindow();
            pHdl=pViewSh->GetInputHandler();
            if(ppViewSh!=NULL) *ppViewSh=pViewSh;
        }
        pFrame = SfxViewFrame::GetNext(*pFrame,pDocShell, TYPE(SfxTopViewFrame) );
    }


    return pHdl;
}



void ScFormulaDlg::FillDialog(BOOL nFlag)
{
    if(nFlag) FillControls();
    FillListboxes();

    String aStrResult;

    ScModule* pScMod = SC_MOD();
    if ( CalcValue(pScMod->InputGetFormulaStr(), aStrResult ) )
        aWndFormResult.SetValue( aStrResult );
    else
    {
        aStrResult.Erase();
        aWndFormResult.SetValue( aStrResult );
    }

}

void ScFormulaDlg::FillListboxes()
{
    //  Umschalten zwischen den "Seiten"

    ScModule* pScMod = SC_MOD();
    ScFormEditData* pData = pScMod->GetFormEditData();

    String aNewTitle;
    //  1. Seite: Funktion auswaehlen

    if(pFuncDesc)
    {
        if(pScFuncPage->GetCategory()!=pFuncDesc->nCategory+1)
                pScFuncPage->SetCategory(pFuncDesc->nCategory+1);

        USHORT nPos=pScFuncPage->GetFuncPos(pFuncDesc);

        pScFuncPage->SetFunction(nPos);
    }
    else if (pData)
    {
        pScFuncPage->SetCategory(pData->GetCatSel() );
        pScFuncPage->SetFunction( pData->GetFuncSel() );
    }
    FuncSelHdl(NULL);

    //  ResizeArgArr jetzt schon in UpdateFunctionDesc

    //pScFuncPage->GetFunctionPtr()->GrabFocus();

    SFX_APP()->LockDispatcher( TRUE ); // Modal-Modus einschalten

    /*
    aBtnBackward.Enable(FALSE);
    aBtnForward.Enable(TRUE);
    */
    aNewTitle = aTitle1;

    //  HelpId fuer 1. Seite ist die aus der Resource
    SetHelpId( nOldHelp );
    SetUniqueId( nOldUnique );

}
void ScFormulaDlg::FillControls()
{
    //  Umschalten zwischen den "Seiten"

    ScModule* pScMod = SC_MOD();
    ScFormEditData* pData = pScMod->GetFormEditData();
    if (!pData) return;

    String aNewTitle;
    //  2. Seite oder Editieren: ausgewaehlte Funktion anzeigen

    xub_StrLen nFStart     = pData->GetFStart();
    String aFormula        = pScMod->InputGetFormulaStr();
    xub_StrLen nNextFStart = nFStart;
    xub_StrLen nNextFEnd   = 0;

    aFormula.AppendAscii(RTL_CONSTASCII_STRINGPARAM( " )" ));
    DeleteArgs();
    ScFuncDesc*     pOldFuncDesc=pFuncDesc;
    BOOL            bTestFlag=FALSE;

    if ( ScFormulaUtil::GetNextFunc( aFormula, FALSE,
                                     nNextFStart, &nNextFEnd, &pFuncDesc, &pArgArr ) )
    {
        bTestFlag=(pOldFuncDesc!=pFuncDesc);
        if(bTestFlag)
        {
            aFtHeadLine.Hide();
            aFtFuncName.Hide();
            aFtFuncDesc.Hide();
            aScParaWin.SetFunctionDesc(pFuncDesc);
            if(pFuncDesc->pFuncName)
                aFtEditName.SetText( *(pFuncDesc->pFuncName) );
            else
                aFtEditName.SetText( EMPTY_STRING);

        }

        xub_StrLen nOldStart, nOldEnd;
        pScMod->InputGetSelection( nOldStart, nOldEnd );
        if ( nOldStart != nNextFStart || nOldEnd != nNextFEnd )
        {
            pScMod->InputSetSelection( nNextFStart, nNextFEnd );
        }
        aFuncSel.Min()=nNextFStart;
        aFuncSel.Max()=nNextFEnd;

        if(!bEditFlag)
            pMEdit->SetText(pScMod->InputGetFormulaStr());
        xub_StrLen PrivStart, PrivEnd;
        pScMod->InputGetSelection( PrivStart, PrivEnd);
        if(!bEditFlag)
            pMEdit->SetSelection( Selection(PrivStart, PrivEnd));

        nArgs = pFuncDesc->nArgCount;
        USHORT nOffset = pData->GetOffset();
        nEdFocus = pData->GetEdFocus();

        //  Verkettung der Edit's fuer Focus-Kontrolle

        if(bTestFlag) aScParaWin.SetArgCount(nArgs,nOffset);
        USHORT nActiv=0;
        xub_StrLen nArgPos=ScFormulaUtil::GetArgStart( aFormula, nFStart, 0 );
        xub_StrLen nEditPos=(xub_StrLen) pMEdit->GetSelection().Min();
        BOOL    bFlag=FALSE;

        for(USHORT i=0;i<nArgs;i++)
        {
            xub_StrLen nLength=(pArgArr[i])->Len()+1;
            aScParaWin.SetArgument(i,*(pArgArr[i]));
            if(nArgPos<=nEditPos && nEditPos<nArgPos+nLength)
            {
                nActiv=i;
                bFlag=TRUE;
            }
            nArgPos+=nLength;
        }
        aScParaWin.UpdateParas();

        if(bFlag)
        {
            aScParaWin.SetActiveLine(nActiv);
        }

        //aScParaWin.SetEdFocus( nEdFocus );
        UpdateValues();
    }
    else
    {
        aFtEditName.SetText(EMPTY_STRING);
    }
        //  Test, ob vorne/hinten noch mehr Funktionen sind

    xub_StrLen nTempStart = ScFormulaUtil::GetArgStart( aFormula, nFStart, 0 );
    BOOL bNext = ScFormulaUtil::GetNextFunc( aFormula, FALSE, nTempStart );
    nTempStart=(xub_StrLen)pMEdit->GetSelection().Min();
    pData->SetFStart(nTempStart);
    BOOL bPrev = ScFormulaUtil::GetNextFunc( aFormula, TRUE, nTempStart );
    aBtnBackward.Enable(bPrev);
    aBtnForward.Enable(bNext);
}

void ScFormulaDlg::ClearAllParas()
{
    DeleteArgs();
    pFuncDesc=NULL;
    aScParaWin.ClearAll();
    aWndResult.SetValue(EMPTY_STRING);
    aFtEditName.SetText(EMPTY_STRING);
    FuncSelHdl(NULL);

    if(pScFuncPage->IsVisible())
    {
        aBtnForward.Enable(TRUE); //@new
        aFtHeadLine.Show();
        aFtFuncName.Show();
        aFtFuncDesc.Show();
        aFtHeadLine.ToTop();
        aFtFuncName.ToTop();
        aFtFuncDesc.ToTop();
    }
}

void ScFormulaDlg::DeleteArgs()
{
    if ( pArgArr )
    {
        for ( USHORT i=0; i<nArgs; i++ )
            delete pArgArr[i];
        delete [] pArgArr;
    }

    pArgArr = NULL;
    nArgs = 0;
}

BOOL __EXPORT ScFormulaDlg::Close()
{
    DoEnter(FALSE);
    return TRUE;
}

void ScFormulaDlg::DoEnter(BOOL bOk)
{
    //  Eingabe ins Dokument uebernehmen oder abbrechen

    ScModule* pScMod = SC_MOD();

    if ( bOk)
    {
        //  ggf. Dummy-Argumente entfernen
        String  aInputFormula=pScMod->InputGetFormulaStr();
        String  aString=RepairFormula(pMEdit->GetText());
        pScMod->InputSetSelection(0, aInputFormula.Len());
        pScMod->InputReplaceSelection(aString);
    }

    // auf das Dokument zurueckschalten
    // (noetig, weil ein fremdes oben sein kann - #34222#)
    ScInputHandler* pHdl = pScMod->GetInputHdl();
    if ( pHdl )
    {
        pHdl->ViewShellGone(NULL);  // -> aktive View neu holen
        pHdl->ShowRefFrame();
    }

    // aktuelle Tabelle ggF. restaurieren (wg. Maus-RefInput)
    ScTabViewShell* pScViewShell = PTR_CAST(ScTabViewShell, SfxViewShell::Current());
    if ( pScViewShell )
    {
        ScViewData* pVD=pScViewShell->GetViewData();
        USHORT nExecTab = aCursorPos.Tab();
        if ( nExecTab != pVD->GetTabNo() )
            pScViewShell->SetTabNo( nExecTab );

        USHORT nRow=aCursorPos.Row();
        USHORT nCol=aCursorPos.Col();

        if(pVD->GetCurX()!=nRow || pVD->GetCurY()!=nCol)
            pScViewShell->SetCursor(nCol,nRow);
    }

    SfxBoolItem   aRetItem( SID_DLG_RETOK, bOk );
    SfxBoolItem   aMatItem( SID_DLG_MATRIX, aBtnMatrix.IsChecked() );
    SfxStringItem aStrItem( SCITEM_STRING, pScMod->InputGetFormulaStr() );

    // Wenn durch Dokument-Umschalterei die Eingabezeile weg war/ist,
    // ist der String leer. Dann nicht die alte Formel loeschen.
    if ( !aStrItem.GetValue().Len() )
        aRetItem.SetValue( FALSE );     // FALSE = Cancel

    SFX_APP()->LockDispatcher( FALSE ); // Modal-Modus ausschalten

    pDoc = NULL;

    //Referenz Inputhandler zuruecksetzen

    pScMod->SetRefInputHdl(NULL);


    // Enable() der Eingabezeile erzwingen:
    if ( pScViewShell )
        pScViewShell->UpdateInputHandler();
    SFX_DISPATCHER().Execute( SID_INS_FUNCTION,
                              SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD,
                              &aRetItem, &aStrItem, &aMatItem, 0L );

    //  Daten loeschen
    pScMod->ClearFormEditData();        // pData wird ungueltig!

    /*

    ScInputHandler* pInputHdl = pScMod->GetInputHdl();
    if ( pInputHdl )
        pInputHdl->NotifyChange( NULL );

    */
    //  Dialog schliessen
    DoClose( ScFormulaDlgWrapper::GetChildWindowId() );
}

IMPL_LINK( ScFormulaDlg, BtnHdl, PushButton*, pBtn )
{
    if ( pBtn == &aBtnCancel )
    {
        DoEnter(FALSE);                 // schliesst den Dialog
    }
    else if ( pBtn == &aBtnEnd )
    {
        DoEnter(TRUE);                  // schliesst den Dialog
    }
    else if ( pBtn == &aBtnForward )
    {
        //@pMEdit->GrabFocus();         // Damit die Selektion auch angezeigt wird.
        ScFuncDesc* pDesc =pScFuncPage->GetFuncDesc(
                                pScFuncPage->GetFunction() );

        if(pDesc==pFuncDesc || !pScFuncPage->IsVisible())
            EditNextFunc( TRUE );
        else
        {
            DblClkHdl(pScFuncPage);    //new
            aBtnForward.Enable(FALSE); //new
        }
        //@EditNextFunc( TRUE );
    }
    else if ( pBtn == &aBtnBackward )
    {
        bEditFlag=FALSE;
        aBtnForward.Enable(TRUE);
        EditNextFunc( FALSE );
        aMEFormula.Invalidate();
        aMEFormula.Update();
    }
    //...

    return 0;
}


//  --------------------------------------------------------------------------
//                          Funktionen fuer 1. Seite
//  --------------------------------------------------------------------------

void ScFormulaDlg::ResizeArgArr( ScFuncDesc* pNewFunc )
{
    if ( pFuncDesc != pNewFunc )
    {
        DeleteArgs();

        if ( pNewFunc )
        {
            nArgs = pNewFunc->nArgCount;
            if ( nArgs > 0 )
            {
                pArgArr = new String*[nArgs];
                for ( USHORT i=0; i<nArgs; i++ )
                    pArgArr[i] = new String;
            }
        }

        pFuncDesc = pNewFunc;
    }
}

void ScFormulaDlg::UpdateFunctionDesc()
{
    ScModule* pScMod = SC_MOD();

    ScFormEditData* pData = pScMod->GetFormEditData();
    if (!pData) return;
    USHORT nCat = pScFuncPage->GetCategory();
    if ( nCat == LISTBOX_ENTRY_NOTFOUND ) nCat = 0;
    pData->SetCatSel( nCat );
    USHORT nFunc = pScFuncPage->GetFunction();
    if ( nFunc == LISTBOX_ENTRY_NOTFOUND ) nFunc = 0;
    pData->SetFuncSel( nFunc );

    if (   (pScFuncPage->GetFunctionEntryCount() > 0)
        && (pScFuncPage->GetFunction() != LISTBOX_ENTRY_NOTFOUND) )
    {
        ScFuncDesc* pDesc =pScFuncPage->GetFuncDesc(
                                pScFuncPage->GetFunction() );
        if (pDesc)
        {
            String aSig = pDesc->GetSignature();

            aFtFuncName.SetText( aSig );
            if(pDesc->pFuncDesc)
                aFtFuncDesc.SetText( *(pDesc->pFuncDesc) );
            else
                aFtFuncDesc.SetText(EMPTY_STRING );
            ResizeArgArr( pDesc );

            if ( pArgArr && pArgArr[0] && pArgArr[0]->Len() )       // noch Argumente da?
                aSig = pDesc->GetFormulaString( pArgArr );          // fuer Eingabezeile
            //@ pScMod->InputReplaceSelection( aSig );
        }
    }
    else
    {
        aFtFuncName.SetText( EMPTY_STRING );
        aFtFuncDesc.SetText( EMPTY_STRING );

        //ResizeArgArr( NULL );
        pScMod->InputReplaceSelection( EMPTY_STRING );
    }
}

// Handler fuer Listboxen

IMPL_LINK( ScFormulaDlg, DblClkHdl, ScFuncPage*, pLb )
{
    ScModule* pScMod = SC_MOD();

    USHORT nCat = pScFuncPage->GetCategory();
    USHORT nFunc = pScFuncPage->GetFunction();

    //  ex-UpdateLRUList
    ScFuncDesc* pDesc = pScFuncPage->GetFuncDesc(nFunc);
    if (pDesc && pDesc->nFIndex!=0)
        pScMod->InsertEntryToLRUList(pDesc->nFIndex);

    String aFuncName=pScFuncPage->GetSelFunctionName();
    aFuncName.AppendAscii(RTL_CONSTASCII_STRINGPARAM( "()" ));
    pScMod->InputReplaceSelection(aFuncName);
    pMEdit->ReplaceSelected(aFuncName);

    Selection aSel=pMEdit->GetSelection();
    aSel.Max()=aSel.Max()-1;
    pMEdit->SetSelection(aSel);

    FormulaHdl(pMEdit);

    aSel.Min()=aSel.Max();
    pMEdit->SetSelection(aSel);

    if(nArgs==0)
    {
        BtnHdl(&aBtnBackward);
    }

    aScParaWin.SetEdFocus(0);
    aBtnForward.Enable(FALSE); //@New

    return 0;
}

//  --------------------------------------------------------------------------
//                          Funktionen fuer rechte Seite
//  --------------------------------------------------------------------------

void ScFormulaDlg::EditThisFunc(xub_StrLen nFStart)
{
    ScModule* pScMod = SC_MOD();
    ScFormEditData* pData = pScMod->GetFormEditData();
    if (!pData) return;

    String aFormula = pScMod->InputGetFormulaStr();

    if(nFStart==NOT_FOUND)
    {
        nFStart = pData->GetFStart();
    }
    else
    {
        pData->SetFStart(nFStart);
    }

    xub_StrLen nNextFStart  = nFStart;
    xub_StrLen nNextFEnd    = 0;

    BOOL bFound;

    //@bFound = ScFormulaUtil::GetNextFunc( aFormula, FALSE, nNextFStart, &nNextFEnd, &pFuncDesc );

    bFound = ScFormulaUtil::GetNextFunc( aFormula, FALSE, nNextFStart, &nNextFEnd);
    if ( bFound )
    {
        xub_StrLen nFEnd;

        // Selektion merken und neue setzen
        pScMod->InputGetSelection( nFStart, nFEnd );
        pScMod->InputSetSelection( nNextFStart, nNextFEnd );
        if(!bEditFlag)
            pMEdit->SetText(pScMod->InputGetFormulaStr());

        xub_StrLen PrivStart, PrivEnd;
        pScMod->InputGetSelection( PrivStart, PrivEnd);
        if(!bEditFlag)
            pMEdit->SetSelection( Selection(PrivStart, PrivEnd));

        pData->SetFStart( nNextFStart );
        pData->SetOffset( 0 );
        pData->SetEdFocus( 0 );

        HighlightFunctionParas(aFormula.Copy(PrivStart, PrivEnd-PrivStart));
        FillDialog();
    }
    else
    {
        ClearAllParas();
        /*
        aScParaWin.ClearAll();
        aWndResult.SetValue(EMPTY_STRING);
        aFtEditName.SetText(EMPTY_STRING);
        */
    }
}

void ScFormulaDlg::EditNextFunc( BOOL bForward, xub_StrLen nFStart )
{
    ScModule* pScMod = SC_MOD();
    ScFormEditData* pData = pScMod->GetFormEditData();
    if (!pData) return;

    String aFormula = pScMod->InputGetFormulaStr();

    if(nFStart==NOT_FOUND)
    {
        nFStart = pData->GetFStart();
    }
    else
    {
        pData->SetFStart(nFStart);
    }

    xub_StrLen nNextFStart  = 0;
    xub_StrLen nNextFEnd    = 0;

    BOOL bFound;
    if ( bForward )
    {
        nNextFStart = ScFormulaUtil::GetArgStart( aFormula, nFStart, 0 );
        //@bFound = ScFormulaUtil::GetNextFunc( aFormula, FALSE, nNextFStart, &nNextFEnd, &pFuncDesc );
        bFound = ScFormulaUtil::GetNextFunc( aFormula, FALSE, nNextFStart, &nNextFEnd);
    }
    else
    {
        nNextFStart = nFStart;
        //@bFound = ScFormulaUtil::GetNextFunc( aFormula, TRUE, nNextFStart, &nNextFEnd, &pFuncDesc );
        bFound = ScFormulaUtil::GetNextFunc( aFormula, TRUE, nNextFStart, &nNextFEnd);
    }

    if ( bFound )
    {
        xub_StrLen nFEnd;

        // Selektion merken und neue setzen
        pScMod->InputGetSelection( nFStart, nFEnd );
        pScMod->InputSetSelection( nNextFStart, nNextFEnd );
        if(!bEditFlag)
            pMEdit->SetText(pScMod->InputGetFormulaStr());

        xub_StrLen PrivStart, PrivEnd;
        pScMod->InputGetSelection( PrivStart, PrivEnd);
        if(!bEditFlag)
            pMEdit->SetSelection( Selection(PrivStart, PrivEnd));

        pData->SetFStart( nNextFStart );
        pData->SetOffset( 0 );
        pData->SetEdFocus( 0 );

        FillDialog();
    }
}

void ScFormulaDlg::EditFuncParas(xub_StrLen nEditPos)
{
    if(pFuncDesc!=NULL)
    {
        ScModule* pScMod = SC_MOD();
        ScFormEditData* pData = pScMod->GetFormEditData();
        if (!pData) return;

        String aFormula = pScMod->InputGetFormulaStr();
        aFormula +=')';
        xub_StrLen nFStart = pData->GetFStart();

        DeleteArgs();

        nArgs = pFuncDesc->nArgCount;

        xub_StrLen nArgPos=ScFormulaUtil::GetArgStart( aFormula, nFStart, 0 );
        pArgArr = ScFormulaUtil::GetArgStrings( aFormula, nFStart, pFuncDesc->nArgCount );

        USHORT nActiv=aScParaWin.GetSliderPos();
        BOOL    bFlag=FALSE;
        for(USHORT i=0;i<nArgs;i++)
        {
            xub_StrLen nLength=(pArgArr[i])->Len();
            aScParaWin.SetArgument(i,*(pArgArr[i]));
            if(nArgPos<=nEditPos && nEditPos<nArgPos+nLength)
            {
                nActiv=i;
                bFlag=TRUE;
            }
            nArgPos+=nLength+1;
        }

        if(bFlag)
        {
            aScParaWin.SetSliderPos(nActiv);
        }

        aScParaWin.UpdateParas();
        UpdateValues();
    }

}


IMPL_LINK( ScFormulaDlg, ScrollHdl, ScParaWin*, pBar )
{
    USHORT i = 0;

    ScModule* pScMod = SC_MOD();
    ScFormEditData* pData = pScMod->GetFormEditData();
    if (!pData) return 0;
    USHORT nOffset = aScParaWin.GetSliderPos();
    pData->SetOffset( nOffset );

    aScParaWin.UpdateParas();

    UpdateValues();

    return 0;
}

BOOL ScFormulaDlg::CalcValue( const String& rStrExp, String& rStrResult )
{
    BOOL bResult = TRUE;

    if ( rStrExp.Len() > 0 )
    {
        // nur, wenn keine Tastatureingabe mehr anliegt, den Wert berechnen:

        if ( !Application::AnyInput( INPUT_KEYBOARD ) )
        {
            ScFormulaCell* pFCell = new ScFormulaCell( pDoc, aCursorPos, rStrExp );

            ScCompiler* pCompi=new ScCompiler( pDoc, aCursorPos, *(pFCell->GetCode()));

            // #35521# HACK! um bei ColRowNames kein #REF! zu bekommen,
            // wenn ein Name eigentlich als Bereich in die Gesamt-Formel
            // eingefuegt wird, bei der Einzeldarstellung aber als
            // single-Zellbezug interpretiert wird
            BOOL bColRowName = pCell->HasColRowName();
            if ( bColRowName )
            {
                // ColRowName im RPN-Code?
                if ( pCell->GetCode()->GetCodeLen() <= 1 )
                {   // ==1: einzelner ist als Parameter immer Bereich
                    // ==0: es waere vielleicht einer, wenn..
                    String aBraced( '(' );
                    aBraced += rStrExp;
                    aBraced += ')';
                    delete pFCell;
                    pFCell = new ScFormulaCell( pDoc, aCursorPos, aBraced );
                }
                else
                    bColRowName = FALSE;
            }

            USHORT nErrCode = pFCell->GetErrCode();
            if ( nErrCode == 0 )
            {
                SvNumberFormatter& aFormatter = *(pDoc->GetFormatTable());
                Color* pColor;
                if ( pFCell->IsValue() )
                {
                    double n = pFCell->GetValue();
                    ULONG nFormat = aFormatter.GetStandardFormat( n, 0,
                                    pFCell->GetFormatType(), ScGlobal::eLnge );
                    aFormatter.GetOutputString( n, nFormat,
                                                rStrResult, &pColor );
                }
                else
                {
                    String aStr;

                    pFCell->GetString( aStr );
                    ULONG nFormat = aFormatter.GetStandardFormat(
                                    pFCell->GetFormatType(), ScGlobal::eLnge);
                    aFormatter.GetOutputString( aStr, nFormat,
                                                rStrResult, &pColor );
                }

                ScRange aTestRange;
                if ( bColRowName || (aTestRange.Parse(rStrExp) & SCA_VALID) )
                    rStrResult.AppendAscii(RTL_CONSTASCII_STRINGPARAM( " ..." ));
                    // Bereich
            }
            else
                rStrResult += ScGlobal::GetErrorString(nErrCode);

            if(!bUserMatrixFlag && pFCell->GetMatrixFlag())
            {
                aBtnMatrix.Check();
            }

            delete pFCell;
            delete pCompi;
        }
        else
            bResult = FALSE;
    }

    return bResult;
}

void ScFormulaDlg::UpdateValues()
{
    ScModule* pScMod = SC_MOD();
    String aStrResult;

    if ( CalcValue( pFuncDesc->GetFormulaString( pArgArr ), aStrResult ) )
        aWndResult.SetValue( aStrResult );

    aStrResult.Erase();
    if ( CalcValue(pScMod->InputGetFormulaStr(), aStrResult ) )
        aWndFormResult.SetValue( aStrResult );
    else
    {
        aStrResult.Erase();
        aWndFormResult.SetValue( aStrResult );
    }
    CalcStruct(pMEdit->GetText());
}

void ScFormulaDlg::SaveArg( USHORT nEd )
{
    if (nEd<nArgs)
    {
        USHORT i;
        for(i=0;i<=nEd;i++)
        {
            if(pArgArr[i]->Len()==0)
                *(pArgArr[i]) = ' ';
        }
        if(aScParaWin.GetArgument(nEd).Len()!=0)
            *(pArgArr[nEd]) = aScParaWin.GetArgument(nEd);

        USHORT nClearPos=nEd+1;
        for(i=nEd+1;i<nArgs;i++)
        {
            if(aScParaWin.GetArgument(i).Len()!=0)
            {
                nClearPos=i+1;
            }
        }

        for(i=nClearPos;i<nArgs;i++)
        {
            *(pArgArr[i]) = EMPTY_STRING;
        }
    }
}

IMPL_LINK( ScFormulaDlg, FxHdl, ScParaWin*, pPtr )
{
    if(pPtr==&aScParaWin)
    {
        aBtnForward.Enable(TRUE); //@ Damit eine neue Fkt eingegeben werden kann.
        aTabCtrl.SetCurPageId(TP_FUNCTION);
        ScModule* pScMod = SC_MOD();
        String aUndoStr = pScMod->InputGetFormulaStr();     // bevor unten ein ";" eingefuegt wird
        ScFormEditData* pData = pScMod->GetFormEditData();
        if (!pData) return 0;

        BOOL bEmpty = FALSE;
        USHORT nArgNo = aScParaWin.GetActiveLine();
        nEdFocus=nArgNo;

        SaveArg(nArgNo);
        UpdateSelection();

        xub_StrLen nFormulaStrPos = pData->GetFStart();

        String aFormula = pScMod->InputGetFormulaStr();
        xub_StrLen n1 = ScFormulaUtil::GetArgStart( aFormula, nFormulaStrPos, nEdFocus+pData->GetOffset() );
        xub_StrLen n2 = ScFormulaUtil::GetFunctionEnd( aFormula, n1 );



        pData->SetEdFocus( nEdFocus );
        pData->SaveValues();
        pData->SetMode( (USHORT) SC_FORMDLG_FORMULA );
        pData->SetFStart( n1 );
        pData->SetUndoStr( aUndoStr );
        ClearAllParas();

        FillDialog(FALSE);
        pScFuncPage->SetFocus(); //Da Parawin nicht mehr sichtbar
    }
    return 0;
}

IMPL_LINK( ScFormulaDlg, ModifyHdl, ScParaWin*, pPtr )
{
    if(pPtr==&aScParaWin)
    {
        SaveArg(aScParaWin.GetActiveLine());
        UpdateValues();

        UpdateSelection();
        CalcStruct(pMEdit->GetText());
    }
    return 0;
}

IMPL_LINK( ScFormulaDlg, FormulaHdl, MultiLineEdit*, pEd )
{
    ScModule* pScMod = SC_MOD();
    ScFormEditData* pData = pScMod->GetFormEditData();
    if (!pData) return 0;
    xub_StrLen nFStart = pData->GetFStart();

    bEditFlag=TRUE;
    String      aInputFormula=pScMod->InputGetFormulaStr();
    String      aString=pMEdit->GetText();

    Selection   aSel =pMEdit->GetSelection();
    xub_StrLen nTest=0;

    if(aString.Len()==0) //falls alles geloescht wurde
    {
        aString +='=';
        pMEdit->SetText(aString);
        aSel .Min()=1;
        aSel .Max()=1;
        pMEdit->SetSelection(aSel);
    }
    else if(aString.GetChar(nTest)!='=') //falls ersetzt wurde;
    {
        aString.Insert( (sal_Unicode)'=', 0 );
        pMEdit->SetText(aString);
        aSel .Min()+=1;
        aSel .Max()+=1;
        pMEdit->SetSelection(aSel);
    }


    pScMod->InputSetSelection(0, aInputFormula.Len());
    pScMod->InputReplaceSelection(aString);
    pScMod->InputSetSelection((xub_StrLen)aSel.Min(),(xub_StrLen)aSel.Max());

    xub_StrLen nPos=(xub_StrLen)aSel.Min()-1;

    String aStrResult;

    if ( CalcValue(pScMod->InputGetFormulaStr(), aStrResult ) )
        aWndFormResult.SetValue( aStrResult );
    else
    {
        aStrResult.Erase();
        aWndFormResult.SetValue( aStrResult );
    }
    CalcStruct(aString);

    nPos=GetFunctionPos(nPos);

    if(nPos<aSel.Min()-1)
    {
        xub_StrLen nPos1=aString.Search('(',nPos);
        EditNextFunc( FALSE, nPos1);
    }
    else
    {
        ClearAllParas();
        /*
        aScParaWin.ClearAll();
        aWndResult.SetValue(EMPTY_STRING);
        aFtEditName.SetText(EMPTY_STRING);
        */
    }

    pScMod->InputSetSelection((xub_StrLen)aSel.Min(),(xub_StrLen)aSel.Max());
    bEditFlag=FALSE;
    return 0;
}

IMPL_LINK( ScFormulaDlg, FormulaCursorHdl, ScEditBox*, pEd )
{
    ScModule* pScMod = SC_MOD();
    ScFormEditData* pData = pScMod->GetFormEditData();
    if (!pData) return 0;
    xub_StrLen nFStart = pData->GetFStart();

    bEditFlag=TRUE;

    String      aInputFormula=pScMod->InputGetFormulaStr();
    String      aString=pMEdit->GetText();

    Selection   aSel =pMEdit->GetSelection();
    pScMod->InputSetSelection((xub_StrLen)aSel.Min(),(xub_StrLen)aSel.Max());


    if(aSel.Min()==0)
    {
        aSel.Min()=1;
        pMEdit->SetSelection(aSel);
    }

    if(aSel.Min()!=aString.Len())
    {

        xub_StrLen nPos=(xub_StrLen)aSel.Min();

        nFStart=GetFunctionPos(nPos-1);

        if(nFStart<nPos)
        {
            xub_StrLen nPos1=ScFormulaUtil::GetFunctionEnd(aString,nFStart);

            if(nPos1>nPos || nPos1==STRING_NOTFOUND)
            {
                EditThisFunc(nFStart);
            }
            else
            {
                xub_StrLen n=nPos;
                short nCount=1;
                while(n>0)
                {
                   if(aString.GetChar(n)==')')
                       nCount++;
                   else if(aString.GetChar(n)=='(')
                       nCount--;
                   if(nCount==0) break;
                   n--;
                }
                if(nCount==0)
                {
                    nFStart=ScFormulaUtil::GetFunctionStart(aString,n,TRUE);
                    EditThisFunc(nFStart);
                }
                else
                {
                    ClearAllParas();
                }
            }
        }
        else
        {
            ClearAllParas();
        }
    }
    pScMod->InputSetSelection((xub_StrLen)aSel.Min(),(xub_StrLen)aSel.Max());

    bEditFlag=FALSE;
    return 0;
}

void ScFormulaDlg::UpdateSelection()
{
    ScModule* pScMod = SC_MOD();

    pScMod->InputSetSelection((xub_StrLen)aFuncSel.Min(),(xub_StrLen)aFuncSel.Max());

    pScMod->InputReplaceSelection( pFuncDesc->GetFormulaString( pArgArr ) );
    pMEdit->SetText(pScMod->InputGetFormulaStr());
    xub_StrLen PrivStart, PrivEnd;
    pScMod->InputGetSelection( PrivStart, PrivEnd);
    aFuncSel.Min()=PrivStart;
    aFuncSel.Max()=PrivEnd;

    nArgs = pFuncDesc->nArgCount;

    String aFormula=pMEdit->GetText();
    xub_StrLen nArgPos=ScFormulaUtil::GetArgStart( aFormula,PrivStart,0);

    USHORT nPos=aScParaWin.GetActiveLine();

    BOOL    bFlag=FALSE;
    for(USHORT i=0;i<nPos;i++)
    {
        xub_StrLen nTmpLength=(pArgArr[i])->Len();
        nArgPos+=nTmpLength+1;
    }
    xub_StrLen nLength=(pArgArr[nPos])->Len();

    Selection aSel(nArgPos,nArgPos+nLength);
    pScMod->InputSetSelection(nArgPos,nArgPos+nLength);
    pMEdit->SetSelection(aSel);
}

//  virtuelle Methoden von ScAnyRefDlg:
void ScFormulaDlg::RefInputStart( ScRefEdit* pEdit, ScRefButton* pButton)
{
    aEdRef.Show();
    pTheRefEdit=pEdit;
    pTheRefButton=pButton;
    if(pTheRefEdit!=NULL)
    {
        aEdRef.SetRefString(pTheRefEdit->GetText());
        aEdRef.SetSelection(pTheRefEdit->GetSelection());
        aEdRef.SetHelpId(pTheRefEdit->GetHelpId());
        aEdRef.SetUniqueId(pTheRefEdit->GetUniqueId());

        if(pButton!=NULL)
        {
            aRefBtn.Show();
            ScAnyRefDlg::RefInputStart(&aEdRef,&aRefBtn);
            aRefBtn.SetEndImage();
        }
        else
        {
            aRefBtn.Hide();
            ScAnyRefDlg::RefInputStart(&aEdRef);
        }

        String aStr=aTitle2;
        aStr+=String(' ');
        aStr+=aFtEditName.GetText();

        if(aScParaWin.GetActiveLine()>0)
        {
            aStr.AppendAscii(RTL_CONSTASCII_STRINGPARAM( "(...; " ));
        }
        else
        {
            aStr.AppendAscii(RTL_CONSTASCII_STRINGPARAM( "( " ));
        }

        aStr += aScParaWin.GetActiveArgName();
        if(nArgs>1)
            aStr.AppendAscii(RTL_CONSTASCII_STRINGPARAM( ";...)" ));
        else
            aStr += ')';
        SetText(aStr);
    }
    else
    {
        if(pButton!=NULL)
        {
            aRefBtn.Show();
            ScAnyRefDlg::RefInputStart(&aEdRef,&aRefBtn);
        }
        else
        {
            aRefBtn.Hide();
            ScAnyRefDlg::RefInputStart(&aEdRef);
        }
    }
}

void ScFormulaDlg::RefInputDone( BOOL bForced)
{
    ScAnyRefDlg::RefInputDone(bForced);
    if(bForced || !aRefBtn.IsVisible())
    {
        aEdRef.Hide();
        aRefBtn.Hide();
        if(pTheRefEdit!=NULL)
        {
            pTheRefEdit->SetRefString(aEdRef.GetText());
            pTheRefEdit->GrabFocus();

            if(pTheRefButton!=NULL)
                pTheRefButton->SetStartImage();

            aRefBtn.SetStartImage();
            USHORT nPrivActiv=aScParaWin.GetActiveLine();
            aScParaWin.SetArgument(nPrivActiv,aEdRef.GetText());
            ModifyHdl(&aScParaWin);
            pTheRefEdit=NULL;
        }
        SetText(aTitle1);
    }
}

void ScFormulaDlg::SetReference( const ScRange& rRef, ScDocument* pRefDoc )
{
    if (nArgs > 0 )
    {
        aScParaWin.SetRefMode(TRUE);

        Edit*       pEd = aScParaWin.GetActiveEdit();
        Selection   theSel;
        String      aStrEd;
        if(pEd!=NULL && pTheRefEdit==NULL)
        {
            theSel=pEd->GetSelection();
            aStrEd=pEd->GetText();
            aEdRef.SetRefString(aStrEd);
            aEdRef.SetSelection( theSel );
        }
        else
        {
            theSel=aEdRef.GetSelection();
            aStrEd=aEdRef.GetText();
        }
        String      aRefStr;

        if ( rRef.aStart != rRef.aEnd && pTheRefEdit==NULL)
        {
            RefInputStart(aScParaWin.GetActiveEdit());
        }

        BOOL bOtherDoc = ( pRefDoc != pDoc && pRefDoc->GetDocumentShell()->HasName() );
        if ( bOtherDoc )
        {
            //  Referenz auf anderes Dokument - wie inputhdl.cxx

            DBG_ASSERT(rRef.aStart.Tab()==rRef.aEnd.Tab(), "nStartTab!=nEndTab");

            String aTmp;
            rRef.Format( aTmp, SCA_VALID|SCA_TAB_3D, pRefDoc );     // immer 3d

            SfxObjectShell* pObjSh = pRefDoc->GetDocumentShell();
            String aFileName = pObjSh->GetMedium()->GetName();

            aRefStr = '\'';
            aRefStr += aFileName;
            aRefStr.AppendAscii(RTL_CONSTASCII_STRINGPARAM( "'#" ));
            aRefStr += aTmp;
        }
        else
        {
            USHORT nFmt = ( rRef.aStart.Tab() == aCursorPos.Tab() )
                                ? SCA_VALID
                                : SCA_VALID | SCA_TAB_3D;
            rRef.Format( aRefStr, nFmt, pRefDoc );
        }

        aEdRef.ReplaceSelected( aRefStr );
        theSel.Max() = theSel.Min() + aRefStr.Len();
        aEdRef.SetSelection( theSel );

        //-------------------------------------
        // Manuelles Update der Ergebnisfelder:
        //-------------------------------------
        USHORT nPrivActiv=aScParaWin.GetActiveLine();
        aScParaWin.SetArgument(nPrivActiv,aEdRef.GetText());
        aScParaWin.UpdateParas();

        if(pEd!=NULL) pEd->SetSelection( theSel );

        aScParaWin.SetRefMode(FALSE);
    }
}

BOOL ScFormulaDlg::IsRefInputMode() const
{
    BOOL bRef = (nArgs > 0)&& (pDoc!=NULL);
    return bRef;
}

BOOL ScFormulaDlg::IsDocAllowed(SfxObjectShell* pDocSh) const
{
    //  not allowed: different from this doc, and no name
    //  pDocSh is always a ScDocShell
    if ( pDocSh && ((ScDocShell*)pDocSh)->GetDocument() != pDoc && !pDocSh->HasName() )
        return FALSE;

    return TRUE;        // everything else is allowed
}

void ScFormulaDlg::SetActive()
{
    if(nArgs > 0)
    {
        RefInputDone();
        Edit*   pEd = aScParaWin.GetActiveEdit();
        if(pEd!=NULL)
        {
            Selection theSel=aEdRef.GetSelection();
            //  Edit may have the focus -> call ModifyHdl in addition
            //  to what's happening in GetFocus
            pEd->GetModifyHdl().Call(pEd);
            pEd->GrabFocus();
            pEd->SetSelection(theSel);
        }
    }
}

void ScFormulaDlg::MakeTree(SvLBoxEntry* pParent,ScToken* pScToken,long Count,
                            ScTokenArray* pScTokA,ScCompiler*   pComp)
{
    if(pScToken!=NULL && Count>0)
    {
        String aResult;
        long nParas=pScToken->GetParamCount();
        OpCode eOp = pScToken->GetOpCode();

        if(nParas>0)
        {
            pComp->CreateStringFromToken( aResult,pScToken);

            SvLBoxEntry* pEntry;

            String aTest=pScStructPage->GetEntryText(pParent);

            if(aTest==aResult &&
                (eOp==ocAdd || eOp==ocMul ||
                 eOp==ocAmpersand))
            {
                pEntry=pParent;
            }
            else
            {
                if(eOp==ocBad)
                {
                    pEntry=pScStructPage->InsertEntry(aResult,pParent,STRUCT_ERROR,0,pScToken);
                }
                else
                {
                    pEntry=pScStructPage->InsertEntry(aResult,pParent,STRUCT_FOLDER,0,pScToken);
                }
            }

            MakeTree(pEntry,pScTokA->PrevRPN(),nParas,pScTokA,pComp);
            --Count;
            pScTokA->NextRPN();
            MakeTree(pParent,pScTokA->PrevRPN(),Count,pScTokA,pComp);
        }
        else
        {
            pComp->CreateStringFromToken( aResult,pScToken);

            if(eOp==ocBad)
            {
                pScStructPage->InsertEntry(aResult,pParent,STRUCT_ERROR,0,pScToken);
            }
            else
            {
                pScStructPage->InsertEntry(aResult,pParent,STRUCT_END,0,pScToken);
            }
            --Count;
            MakeTree(pParent,pScTokA->PrevRPN(),Count,pScTokA,pComp);
        }
    }
}


void ScFormulaDlg::UpdateTokenArray( const String& rStrExp)
{
    if(pScTokA!=NULL) delete pScTokA;
    pScTokA=pComp->CompileString(rStrExp);
    pComp->CompileTokenArray();
}

xub_StrLen ScFormulaDlg::GetFunctionPos(xub_StrLen nPos)
{
    xub_StrLen nTokPos=1;
    xub_StrLen nOldTokPos=1;
    xub_StrLen nFuncPos=STRING_NOTFOUND;    //@ Testweise
    xub_StrLen nPrevFuncPos=1;
    short  nBracketCount=0;
    BOOL   bFlag=FALSE;
    String aFormString=pMEdit->GetText();
    xub_StrLen nTheEnd=aFormString.Len();
    ScGlobal::pCharClass->toUpper( aFormString );
    if(pScTokA!=NULL)
    {
        ScToken*  pToken=pScTokA->First();
        while(pToken!=NULL)
        {
            String aString;
            OpCode eOp = pToken->GetOpCode();
            pComp->CreateStringFromToken( aString,pToken);
            ScToken*  pNextToken=pScTokA->Next();

            if(!bUserMatrixFlag && pToken->IsMatrixFunction())
            {
                aBtnMatrix.Check();
            }

            if(eOp==ocPush || eOp==ocSpaces)
            {
                xub_StrLen n1=aFormString.Search(';',nTokPos);
                xub_StrLen n2=aFormString.Search(')',nTokPos);
                xub_StrLen nXXX=nTokPos;
                if(n1<n2)
                {
                    nTokPos=n1;
                }
                else
                {
                    nTokPos=n2;
                }
                if(pNextToken!=NULL)
                {
                    String a2String;
                    pComp->CreateStringFromToken( a2String,pNextToken);
                    xub_StrLen n3=aFormString.Search(a2String,nXXX);

                    if(n3<nTokPos)
                        nTokPos=n3;
                }
            }
            else
            {
                nTokPos+=aString.Len();
            }

            if(eOp==ocOpen)
            {
                nBracketCount++;
                bFlag=TRUE;
            }
            else if(eOp==ocClose)
            {
                nBracketCount--;
                bFlag=FALSE;
                nFuncPos=nPrevFuncPos;
            }

            if((pToken->IsFunction()|| ocArcTan2<=eOp) && ocSpaces!=eOp)
            {
                nPrevFuncPos=nFuncPos;
                nFuncPos=nOldTokPos;
            }

            if(nOldTokPos<=nPos && nPos<nTokPos)
            {
                if(!(pToken->IsFunction()|| ocArcTan2<=eOp))
                {
                    if(nBracketCount<1)
                    {
                        nFuncPos=pMEdit->GetText().Len();
                    }
                    else if(!bFlag)
                    {
                        nFuncPos=nPrevFuncPos;
                    }
                }
                break;
            }

            pToken=pNextToken;
            nOldTokPos=nTokPos;
        }
    }

    return nFuncPos;
}

BOOL ScFormulaDlg::CalcStruct( const String& rStrExp)
{
    BOOL bResult = TRUE;
    xub_StrLen nLength=rStrExp.Len();

    if ( rStrExp.Len() > 0 && aOldFormula!=rStrExp && bStructUpdate)
    {
        // nur, wenn keine Tastatureingabe mehr anliegt, den Wert berechnen:

        if ( !Application::AnyInput( INPUT_KEYBOARD ) )
        {
            pScStructPage->ClearStruct();

            String aString=rStrExp;
            if(rStrExp.GetChar(nLength-1)=='(')
            {
                aString.Erase((xub_StrLen)(nLength-1));
            }

            aString.EraseAllChars('\n');
            String aStrResult;

            if ( CalcValue(aString, aStrResult ) )
                    aWndFormResult.SetValue( aStrResult );

            UpdateTokenArray(aString);

            ScToken*  pScToken=pScTokA->LastRPN();

            if(pScToken!=NULL)
            {
                MakeTree(NULL,pScToken,1,pScTokA,pComp);
            }
            aOldFormula=rStrExp;
            if(rStrExp.GetChar(nLength-1)=='(')
                UpdateTokenArray(rStrExp);
        }
        else
            bResult = FALSE;
    }
    return bResult;
}

IMPL_LINK( ScFormulaDlg, StructSelHdl, ScStructPage*, pStruP )
{
    bStructUpdate=FALSE;
    if(pScStructPage->IsVisible())  aBtnForward.Enable(FALSE); //@New

    if(pScStructPage==pStruP)
    {
        ScToken*  pSelToken=pScStructPage->GetSelectedToken();
        xub_StrLen nTokPos=1;

        if(pScTokA!=NULL)
        {
            ScToken*  pToken=pScTokA->First();

            while(pToken!=NULL)
            {
                String aString;
                if(pToken==pSelToken) break;
                pComp->CreateStringFromToken( aString,pToken);
                nTokPos+=aString.Len();
                pToken=pScTokA->Next();
            }
            EditThisFunc(nTokPos);
        }

        if(pSelToken!=NULL)
        {
            String aStr;
            pComp->CreateStringFromToken( aStr,pSelToken);
            String aEntryTxt=pScStructPage->GetSelectedEntryText();

            if(aEntryTxt!=aStr)
                ShowReference(aEntryTxt);
        }

    }
    bStructUpdate=TRUE;
    return 0;
}

ULONG ScFormulaDlg::FindFocusWin(Window *pWin)
{
    ULONG nUniqueId=0;
    if(pWin->HasFocus())
    {
        nUniqueId=pWin->GetUniqueId();
        if(nUniqueId==0)
        {
            Window* pParent=pWin->GetParent();
            while(pParent!=NULL)
            {
                nUniqueId=pParent->GetUniqueId();

                if(nUniqueId!=0) break;

                pParent=pParent->GetParent();
            }
        }
    }
    else
    {
        USHORT nCount=pWin->GetChildCount();

        for(USHORT i=0;i<nCount;i++)
        {
            Window* pChild=pWin->GetChild(i);
            nUniqueId=FindFocusWin(pChild);
            if(nUniqueId>0) break;
        }
    }
    return nUniqueId;
}

void ScFormulaDlg::SetFocusWin(Window *pWin,ULONG nUniqueId)
{
    if(pWin->GetUniqueId()==nUniqueId)
    {
        pWin->GrabFocus();
    }
    else
    {
        USHORT nCount=pWin->GetChildCount();

        for(USHORT i=0;i<nCount;i++)
        {
            Window* pChild=pWin->GetChild(i);
            SetFocusWin(pChild,nUniqueId);
        }
    }
}

IMPL_LINK( ScFormulaDlg, MatrixHdl, CheckBox *, pCb)
{
    bUserMatrixFlag=TRUE;
    return 0;
}

IMPL_LINK( ScFormulaDlg, FuncSelHdl, ScFuncPage*, pLb )
{
    USHORT nCat = pScFuncPage->GetCategory();
    if ( nCat == LISTBOX_ENTRY_NOTFOUND ) nCat = 0;
    USHORT nFunc = pScFuncPage->GetFunction();
    if ( nFunc == LISTBOX_ENTRY_NOTFOUND ) nFunc = 0;

    if (   (pScFuncPage->GetFunctionEntryCount() > 0)
        && (pScFuncPage->GetFunction() != LISTBOX_ENTRY_NOTFOUND) )
    {
        ScFuncDesc* pDesc =pScFuncPage->GetFuncDesc(
                                pScFuncPage->GetFunction() );

        if(pDesc!=pFuncDesc) aBtnForward.Enable(TRUE); //new

        if (pDesc)
        {
            String aSig = pDesc->GetSignature();
            if(pDesc->pFuncName)
                aFtHeadLine.SetText( *(pDesc->pFuncName) );
            else
                aFtHeadLine.SetText( EMPTY_STRING);
            aFtFuncName.SetText( aSig );
            if(pDesc->pFuncDesc)
                aFtFuncDesc.SetText( *(pDesc->pFuncDesc) );
            else
                aFtFuncDesc.SetText( EMPTY_STRING);
        }
    }
    else
    {
        aFtHeadLine.SetText( EMPTY_STRING );
        aFtFuncName.SetText( EMPTY_STRING );
        aFtFuncDesc.SetText( EMPTY_STRING );
    }
    return 0;
}

IMPL_LINK( ScFormulaDlg, UpdateFocusHdl, Timer*, pTi)
{
    ScModule* pScMod = SC_MOD();
    ScFormEditData* pData = pScMod->GetFormEditData();

    if (pData) // wird nicht ueber Close zerstoert;
    {
        pScMod->SetRefInputHdl(pData->GetInputHandler());
        ULONG nUniqueId=pData->GetUniqueId();
        SetFocusWin((Window *)this,nUniqueId);
    }
    return 0;
}

long ScFormulaDlg::PreNotify( NotifyEvent& rNEvt )
{
    USHORT nSwitch=rNEvt.GetType();
    if(nSwitch==EVENT_GETFOCUS && !bIsShutDown)
    {
        Window* pWin=rNEvt.GetWindow();
        if(pWin!=NULL)
        {
            nActivWinId=pWin->GetUniqueId();
            if(nActivWinId==0)
            {
                Window* pParent=pWin->GetParent();
                while(pParent!=NULL)
                {
                    nActivWinId=pParent->GetUniqueId();

                    if(nActivWinId!=0) break;

                    pParent=pParent->GetParent();
                }
            }
            if(nActivWinId!=0)
            {
                ScModule* pScMod = SC_MOD();
                ScFormEditData* pData = pScMod->GetFormEditData();

                if (pData && !aTimer.IsActive()) // wird nicht ueber Close zerstoert;
                {
                    pData->SetUniqueId(nActivWinId);
                }
            }
        }
    }
    return ScAnyRefDlg::PreNotify(rNEvt);
}

String ScFormulaDlg::RepairFormula(const String& aFormula)
{
    String aResult('=');
    String aString2;
    BOOL   bSep=FALSE;

    ScFunctionMgr* pFuncMgr = ScGlobal::GetStarCalcFunctionMgr();

    UpdateTokenArray(aFormula);

    if(pScTokA!=NULL)
    {
        ScToken*  pToken=pScTokA->First();
        while(pToken!=NULL)
        {
            String aString;
            OpCode eOp = pToken->GetOpCode();
            pComp->CreateStringFromToken( aString,pToken);
            ScToken*  pNextToken=pScTokA->Next();

            if(eOp==ocSep)
            {
                bSep=TRUE;
                //aResult+=aString2;
                aString2+=aString;
            }
            else if(eOp==ocClose &&bSep)
            {
                aString2.Erase();
                bSep=FALSE;
                aResult+=aString;
            }
            else if(eOp!=ocSpaces)
            {
                if(bSep)
                {
                    aResult+=aString2;
                    aString2.Erase();
                    bSep=FALSE;
                }
                aResult+=aString;
            }

            ScFuncDesc* pDesc= pFuncMgr->Get(aString);

            SaveLRUEntry(pDesc);    //! is this necessary?? (EnterData updates the list)

            pToken=pNextToken;
        }

    }

    return aResult;
}

void ScFormulaDlg::HighlightFunctionParas(const String& aFormula)
{
    ShowReference(aFormula);
}

void ScFormulaDlg::SaveLRUEntry(ScFuncDesc* pFuncDesc)
{
    if (pFuncDesc && pFuncDesc->nFIndex!=0)
    {
        ScModule* pScMod = SC_MOD();
        pScMod->InsertEntryToLRUList(pFuncDesc->nFIndex);
    }
}



