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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"



//----------------------------------------------------------------------------

#include "scitems.hxx"
#include <sfx2/dispatch.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/objsh.hxx>
#include <svl/zforlist.hxx>
#include <svl/stritem.hxx>
#include <svtools/svtreebx.hxx>
#include <sfx2/viewfrm.hxx>
#include <vcl/svapp.hxx>
#include <vcl/mnemonic.hxx>
#include <unotools/charclass.hxx>
#include <tools/urlobj.hxx>
#include <formula/formulahelper.hxx>
#include <formula/IFunctionDescription.hxx>

#include "tokenuno.hxx"
#include "formula.hxx"
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
#include "funcdesc.hxx"
#include "formula/token.hxx"
#include "tokenarray.hxx"
#include "sc.hrc"
#include "servuno.hxx"
#include "unonames.hxx"
#include "externalrefmgr.hxx"

#include <com/sun/star/table/CellAddress.hpp>

//============================================================================
using namespace formula;
using namespace com::sun::star;

ScDocument* ScFormulaDlg::pDoc = NULL;
ScAddress ScFormulaDlg::aCursorPos;



//  --------------------------------------------------------------------------
//      Initialisierung / gemeinsame Funktionen  fuer Dialog
//  --------------------------------------------------------------------------

ScFormulaDlg::ScFormulaDlg( SfxBindings* pB, SfxChildWindow* pCW,
                                    Window* pParent, ScViewData* pViewData,formula::IFunctionManager* _pFunctionMgr )
    : formula::FormulaDlg( pB, pCW, pParent, true,true,true,this, _pFunctionMgr,this)
    , m_aHelper(this,pB)
{
    m_aHelper.SetWindow(this);
    ScModule* pScMod = SC_MOD();
    pScMod->InputEnterHandler();
    ScTabViewShell* pScViewShell = NULL;

    // title has to be from the view that opened the dialog,
    // even if it's not the current view

    if ( pB )
    {
        SfxDispatcher* pMyDisp = pB->GetDispatcher();
        if (pMyDisp)
        {
            SfxViewFrame* pMyViewFrm = pMyDisp->GetFrame();
            if (pMyViewFrm)
            {
                pScViewShell = PTR_CAST( ScTabViewShell, pMyViewFrm->GetViewShell() );
                if( pScViewShell )
                    pScViewShell->UpdateInputHandler(TRUE);
            }
        }
    }

    if ( pDoc == NULL )
        pDoc = pViewData->GetDocument();
    m_xParser.set(ScServiceProvider::MakeInstance(SC_SERVICE_FORMULAPARS,(ScDocShell*)pDoc->GetDocumentShell()),uno::UNO_QUERY);
    uno::Reference< beans::XPropertySet> xSet(m_xParser,uno::UNO_QUERY);
    xSet->setPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNO_COMPILEFAP)),uno::makeAny(sal_True));

    m_xOpCodeMapper.set(ScServiceProvider::MakeInstance(SC_SERVICE_OPCODEMAPPER,(ScDocShell*)pDoc->GetDocumentShell()),uno::UNO_QUERY);

    ScInputHandler* pInputHdl = SC_MOD()->GetInputHdl(pScViewShell);

    DBG_ASSERT( pInputHdl, "Missing input handler :-/" );

    if ( pInputHdl )
        pInputHdl->NotifyChange( NULL );

    m_aHelper.enableInput( FALSE );
    m_aHelper.EnableSpreadsheets();
    m_aHelper.Init();
    m_aHelper.SetDispatcherLock( TRUE );

    notifyChange();
    fill();

    ScFormEditData* pData = pScMod->GetFormEditData();
    if (!pData)
    {
        //Nun wird es Zeit den Inputhandler festzulegen
        pScMod->SetRefInputHdl(pScMod->GetInputHdl());

        pDoc = pViewData->GetDocument();
        SCCOL nCol = pViewData->GetCurX();
        SCROW nRow = pViewData->GetCurY();
        SCTAB nTab = pViewData->GetTabNo();
        aCursorPos = ScAddress( nCol, nRow, nTab );

        pScMod->InitFormEditData();                             // neu anlegen
        pData = pScMod->GetFormEditData();
        pData->SetInputHandler(pScMod->GetInputHdl());
        pData->SetDocShell(pViewData->GetDocShell());

        DBG_ASSERT(pData,"FormEditData ist nicht da");

        formula::FormulaDlgMode eMode = FORMULA_FORMDLG_FORMULA;            // Default...

        //  Formel vorhanden? Dann editieren

        String aFormula;
        pDoc->GetFormula( nCol, nRow, nTab, aFormula );
        BOOL bEdit   = ( aFormula.Len() > 1 );
        BOOL bMatrix = FALSE;
        if ( bEdit )
        {
            bMatrix = CheckMatrix(aFormula);

            xub_StrLen nFStart = 0;
            xub_StrLen nFEnd   = 0;
            if ( GetFormulaHelper().GetNextFunc( aFormula, FALSE, nFStart, &nFEnd) )
            {
                pScMod->InputReplaceSelection( aFormula );
                pScMod->InputSetSelection( nFStart, nFEnd );
                xub_StrLen PrivStart, PrivEnd;
                pScMod->InputGetSelection( PrivStart, PrivEnd);

                eMode = SetMeText(pScMod->InputGetFormulaStr(),PrivStart, PrivEnd,bMatrix,TRUE,TRUE);
                pData->SetFStart( nFStart );
            }
            else
                bEdit = FALSE;
        }

        if ( !bEdit )
        {
            String aNewFormula = '=';
            if ( aFormula.Len() > 0 && aFormula.GetChar(0) == '=' )
                aNewFormula=aFormula;

            pScMod->InputReplaceSelection( aNewFormula );
            pScMod->InputSetSelection( 1, aNewFormula.Len()+1 );
            xub_StrLen PrivStart, PrivEnd;
            pScMod->InputGetSelection( PrivStart, PrivEnd);
            SetMeText(pScMod->InputGetFormulaStr(),PrivStart, PrivEnd,bMatrix,FALSE,FALSE);

            pData->SetFStart( 1 );      // hinter dem "="
        }

        pData->SetMode( (USHORT) eMode );
        String rStrExp = GetMeText();

        pCell = new ScFormulaCell( pDoc, aCursorPos, rStrExp );

        Update(rStrExp);
    }

}

void ScFormulaDlg::notifyChange()
{
    ScModule* pScMod = SC_MOD();

    ScInputHandler* pInputHdl = pScMod->GetInputHdl();
    if ( pInputHdl )
        pInputHdl->NotifyChange( NULL );
}
// -----------------------------------------------------------------------------
void ScFormulaDlg::fill()
{
    ScModule* pScMod = SC_MOD();
    ScFormEditData* pData = pScMod->GetFormEditData();
    notifyChange();
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
            ScInputHandler* pInputHdl = GetNextInputHandler(pData->GetDocShell(),&pTabViewShell);

            if ( pInputHdl == NULL ) //DocShell hat keinen InputHandler mehr,
            {                   //hat der Anwender halt Pech gehabt.
                disableOk();
                pInputHdl = pScMod->GetInputHdl();
            }
            else
            {
                pInputHdl->SetRefViewShell(pTabViewShell);
            }
            pScMod->SetRefInputHdl(pInputHdl);
            pData->SetInputHandler(pInputHdl);
        }

        String aOldFormulaTmp(pScMod->InputGetFormulaStr());
        pScMod->InputSetSelection( 0, aOldFormulaTmp.Len());

        rStrExp=pData->GetUndoStr();
        pScMod->InputReplaceSelection(rStrExp);

        SetMeText(rStrExp);

        pCell = new ScFormulaCell( pDoc, aCursorPos, rStrExp );

        Update();
        // Jetzt nochmals zurueckschalten, da evtl. neues Doc geoeffnet wurde!
        pScMod->SetRefInputHdl(NULL);
    }
}

ScFormulaDlg::~ScFormulaDlg()
{
    ScModule* pScMod = SC_MOD();
    ScFormEditData* pData = pScMod->GetFormEditData();

    if (pData) // wird nicht ueber Close zerstoert;
    {
        //Referenz Inputhandler zuruecksetzen
        pScMod->SetRefInputHdl(NULL);
    } // if (pData) // wird nicht ueber Close zerstoert;

    delete pCell;
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

    SfxViewFrame* pFrame = SfxViewFrame::GetFirst( pDocShell );
    while( pFrame && pHdl==NULL)
    {
        SfxViewShell* p = pFrame->GetViewShell();
        ScTabViewShell* pViewSh = PTR_CAST(ScTabViewShell,p);
        if(pViewSh!=NULL)
        {
            pHdl=pViewSh->GetInputHandler();
            if(ppViewSh!=NULL) *ppViewSh=pViewSh;
        }
        pFrame = SfxViewFrame::GetNext( *pFrame, pDocShell );
    }


    return pHdl;
}


BOOL ScFormulaDlg::Close()
{
    DoEnter(FALSE);
    return TRUE;
}

//  --------------------------------------------------------------------------
//                          Funktionen fuer rechte Seite
//  --------------------------------------------------------------------------
bool ScFormulaDlg::calculateValue( const String& rStrExp, String& rStrResult )
{
    BOOL bResult = TRUE;

    ::std::auto_ptr<ScFormulaCell> pFCell( new ScFormulaCell( pDoc, aCursorPos, rStrExp ) );

    // HACK! um bei ColRowNames kein #REF! zu bekommen,
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
            pFCell.reset( new ScFormulaCell( pDoc, aCursorPos, aBraced ) );
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

    if(!isUserMatrix() && pFCell->GetMatrixFlag())
    {
        CheckMatrix();
    }

    return bResult;
}



//  virtuelle Methoden von ScAnyRefDlg:
void ScFormulaDlg::RefInputStart( formula::RefEdit* pEdit, formula::RefButton* pButton )
{
    pEdit->SetSelection(Selection(0, SELECTION_MAX));
    ::std::pair<formula::RefButton*,formula::RefEdit*> aPair = RefInputStartBefore( pEdit, pButton );
    m_aHelper.RefInputStart( aPair.second, aPair.first);
    RefInputStartAfter( aPair.second, aPair.first );
}
void ScFormulaDlg::RefInputDone( BOOL bForced )
{
    m_aHelper.RefInputDone( bForced );
    RefInputDoneAfter( bForced );
}

void ScFormulaDlg::SetReference( const ScRange& rRef, ScDocument* pRefDoc )
{
    const IFunctionDescription* pFunc = getCurrentFunctionDescription();
    if ( pFunc && pFunc->getSuppressedArgumentCount() > 0 )
    {
        Selection theSel;
        BOOL bRefNull = UpdateParaWin(theSel);

        if ( rRef.aStart != rRef.aEnd && bRefNull )
        {
            RefInputStart(GetActiveEdit());
        }

        String      aRefStr;
        BOOL bOtherDoc = ( pRefDoc != pDoc && pRefDoc->GetDocumentShell()->HasName() );
        if ( bOtherDoc )
        {
            //  Referenz auf anderes Dokument - wie inputhdl.cxx

            DBG_ASSERT(rRef.aStart.Tab()==rRef.aEnd.Tab(), "nStartTab!=nEndTab");

            String aTmp;
            rRef.Format( aTmp, SCA_VALID|SCA_TAB_3D, pRefDoc );     // immer 3d

            SfxObjectShell* pObjSh = pRefDoc->GetDocumentShell();

            // #i75893# convert escaped URL of the document to something user friendly
//           String aFileName = pObjSh->GetMedium()->GetName();
            String aFileName = pObjSh->GetMedium()->GetURLObject().GetMainURL( INetURLObject::DECODE_UNAMBIGUOUS );

            aRefStr = '\'';
            aRefStr += aFileName;
            aRefStr.AppendAscii(RTL_CONSTASCII_STRINGPARAM( "'#" ));
            aRefStr += aTmp;
        }
        else
        {
            // We can't use ScRange::Format here because in R1C1 mode we need
            // to display the reference position relative to the cursor
            // position.
            ScTokenArray aArray;
            ScComplexRefData aRefData;
            aRefData.InitRangeRel(rRef, aCursorPos);
            bool bSingle = aRefData.Ref1 == aRefData.Ref2;
            if (aCursorPos.Tab() != rRef.aStart.Tab())
                aRefData.Ref1.SetFlag3D(true);
            if (bSingle)
                aArray.AddSingleReference(aRefData.Ref1);
            else
                aArray.AddDoubleReference(aRefData);
            ScCompiler aComp(pDoc, aCursorPos, aArray);
            aComp.SetGrammar(pDoc->GetGrammar());
            ::rtl::OUStringBuffer aBuf;
            aComp.CreateStringFromTokenArray(aBuf);
            aRefStr = aBuf.makeStringAndClear();
        }

        UpdateParaWin(theSel,aRefStr);
    }
}

BOOL ScFormulaDlg::IsRefInputMode() const
{
    const IFunctionDescription* pDesc = getCurrentFunctionDescription();
    BOOL bRef = (pDesc && (pDesc->getSuppressedArgumentCount() > 0)) && (pDoc!=NULL);
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
    const IFunctionDescription* pFunc = getCurrentFunctionDescription();
    if ( pFunc && pFunc->getSuppressedArgumentCount() > 0 )
    {
        RefInputDone();
        SetEdSelection();
    }
}

void ScFormulaDlg::SaveLRUEntry(const ScFuncDesc* pFuncDescP)
{
    if (pFuncDescP && pFuncDescP->nFIndex!=0)
    {
        ScModule* pScMod = SC_MOD();
        pScMod->InsertEntryToLRUList(pFuncDescP->nFIndex);
    }
}

void ScFormulaDlg::doClose(BOOL /*_bOk*/)
{
    m_aHelper.DoClose( ScFormulaDlgWrapper::GetChildWindowId() );
}
void ScFormulaDlg::insertEntryToLRUList(const formula::IFunctionDescription*    _pDesc)
{
    const ScFuncDesc* pDesc = dynamic_cast<const ScFuncDesc*>(_pDesc);
    SaveLRUEntry(pDesc);
}
void ScFormulaDlg::showReference(const String& _sFormula)
{
    ShowReference(_sFormula);
}
void ScFormulaDlg::ShowReference(const String& _sFormula)
{
    m_aHelper.ShowReference(_sFormula);
}
void ScFormulaDlg::HideReference( BOOL bDoneRefMode )
{
    m_aHelper.HideReference(bDoneRefMode);
}
void ScFormulaDlg::ViewShellChanged( ScTabViewShell* pScViewShell )
{
    m_aHelper.ViewShellChanged( pScViewShell );
}
void ScFormulaDlg::AddRefEntry( )
{

}
BOOL ScFormulaDlg::IsTableLocked( ) const
{
    // per Default kann bei Referenzeingabe auch die Tabelle umgeschaltet werden
    return FALSE;
}
void ScFormulaDlg::ToggleCollapsed( formula::RefEdit* pEdit, formula::RefButton* pButton)
{
    m_aHelper.ToggleCollapsed(pEdit,pButton);
}
void ScFormulaDlg::ReleaseFocus( formula::RefEdit* pEdit, formula::RefButton* pButton)
{
    m_aHelper.ReleaseFocus(pEdit,pButton);
}
void ScFormulaDlg::dispatch(BOOL _bOK,BOOL _bMartixChecked)
{
    SfxBoolItem   aRetItem( SID_DLG_RETOK, _bOK );
    SfxBoolItem   aMatItem( SID_DLG_MATRIX, _bMartixChecked );
    SfxStringItem aStrItem( SCITEM_STRING, getCurrentFormula() );

    // Wenn durch Dokument-Umschalterei die Eingabezeile weg war/ist,
    // ist der String leer. Dann nicht die alte Formel loeschen.
    if ( !aStrItem.GetValue().Len() )
        aRetItem.SetValue( FALSE );     // FALSE = Cancel

    m_aHelper.SetDispatcherLock( FALSE ); // Modal-Modus ausschalten

    clear();

    GetBindings().GetDispatcher()->Execute( SID_INS_FUNCTION,
                              SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD,
                              &aRetItem, &aStrItem, &aMatItem, 0L );
}
void ScFormulaDlg::setDispatcherLock( BOOL bLock )
{
    m_aHelper.SetDispatcherLock( bLock );
}
void ScFormulaDlg::setReferenceInput(const formula::FormEditData* _pData)
{
    ScModule* pScMod = SC_MOD();
    ScFormEditData* pData = const_cast<ScFormEditData*>(dynamic_cast<const ScFormEditData*>(_pData));
    pScMod->SetRefInputHdl(pData->GetInputHandler());
}
void ScFormulaDlg::deleteFormData()
{
    ScModule* pScMod = SC_MOD();
    pScMod->ClearFormEditData();        // pData wird ungueltig!
}
void ScFormulaDlg::clear()
{
    pDoc = NULL;

    //Referenz Inputhandler zuruecksetzen
    ScModule* pScMod = SC_MOD();
    pScMod->SetRefInputHdl(NULL);

    // Enable() der Eingabezeile erzwingen:
    ScTabViewShell* pScViewShell = PTR_CAST(ScTabViewShell, SfxViewShell::Current());
    if ( pScViewShell )
        pScViewShell->UpdateInputHandler();
}
void ScFormulaDlg::switchBack()
{
    ScModule* pScMod = SC_MOD();
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
        SCTAB nExecTab = aCursorPos.Tab();
        if ( nExecTab != pVD->GetTabNo() )
            pScViewShell->SetTabNo( nExecTab );

        SCROW nRow=aCursorPos.Row();
        SCCOL nCol=aCursorPos.Col();

        if(pVD->GetCurX()!=nCol || pVD->GetCurY()!=nRow)
            pScViewShell->SetCursor(nCol,nRow);
    }
}
formula::FormEditData* ScFormulaDlg::getFormEditData() const
{
    ScModule* pScMod = SC_MOD();
    return pScMod->GetFormEditData();
}
void ScFormulaDlg::setCurrentFormula(const String& _sReplacement)
{
    ScModule* pScMod = SC_MOD();
    pScMod->InputReplaceSelection(_sReplacement);
}
void ScFormulaDlg::setSelection(xub_StrLen _nStart,xub_StrLen _nEnd)
{
    ScModule* pScMod = SC_MOD();
    pScMod->InputSetSelection( _nStart, _nEnd );
}
void ScFormulaDlg::getSelection(xub_StrLen& _nStart,xub_StrLen& _nEnd) const
{
    ScModule* pScMod = SC_MOD();
    pScMod->InputGetSelection( _nStart, _nEnd );
}
String ScFormulaDlg::getCurrentFormula() const
{
    ScModule* pScMod = SC_MOD();
    return pScMod->InputGetFormulaStr();
}
formula::IFunctionManager* ScFormulaDlg::getFunctionManager()
{
    return ScGlobal::GetStarCalcFunctionMgr();
}
uno::Reference< sheet::XFormulaParser> ScFormulaDlg::getFormulaParser() const
{
    return m_xParser;
}
uno::Reference< sheet::XFormulaOpCodeMapper> ScFormulaDlg::getFormulaOpCodeMapper() const
{
    return m_xOpCodeMapper;
}

table::CellAddress ScFormulaDlg::getReferencePosition() const
{
    return table::CellAddress(aCursorPos.Tab(),aCursorPos.Col(),aCursorPos.Row());
}

::std::auto_ptr<formula::FormulaTokenArray> ScFormulaDlg::convertToTokenArray(const uno::Sequence< sheet::FormulaToken >& _aTokenList)
{
    ::std::auto_ptr<formula::FormulaTokenArray> pArray(new ScTokenArray());
    pArray->Fill( _aTokenList, pDoc->GetExternalRefManager());
    return pArray;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
