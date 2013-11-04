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

#include "scitems.hxx"
#include <sfx2/dispatch.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/objsh.hxx>
#include <svl/zforlist.hxx>
#include <svl/stritem.hxx>
#include <svtools/treelistbox.hxx>
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
#include "simpleformulacalc.hxx"
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
    : formula::FormulaDlg( pB, pCW, pParent, true,true,true, _pFunctionMgr,this)
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
                    pScViewShell->UpdateInputHandler(true);
            }
        }
    }

    if ( pDoc == NULL )
        pDoc = pViewData->GetDocument();
    m_xParser.set(ScServiceProvider::MakeInstance(SC_SERVICE_FORMULAPARS,(ScDocShell*)pDoc->GetDocumentShell()),uno::UNO_QUERY);
    uno::Reference< beans::XPropertySet> xSet(m_xParser,uno::UNO_QUERY);
    xSet->setPropertyValue(OUString(SC_UNO_COMPILEFAP),uno::makeAny(sal_True));

    m_xOpCodeMapper.set(ScServiceProvider::MakeInstance(SC_SERVICE_OPCODEMAPPER,(ScDocShell*)pDoc->GetDocumentShell()),uno::UNO_QUERY);

    ScInputHandler* pInputHdl = SC_MOD()->GetInputHdl(pScViewShell);

    OSL_ENSURE( pInputHdl, "Missing input handler :-/" );

    if ( pInputHdl )
        pInputHdl->NotifyChange( NULL );

    m_aHelper.enableInput( false );
    m_aHelper.EnableSpreadsheets();
    m_aHelper.Init();
    m_aHelper.SetDispatcherLock( sal_True );

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

        OSL_ENSURE(pData,"FormEditData ist nicht da");

        formula::FormulaDlgMode eMode = FORMULA_FORMDLG_FORMULA;            // Default...

        //  Formel vorhanden? Dann editieren

        String aFormula;
        pDoc->GetFormula( nCol, nRow, nTab, aFormula );
        sal_Bool bEdit   = ( aFormula.Len() > 1 );
        sal_Bool bMatrix = false;
        if ( bEdit )
        {
            bMatrix = CheckMatrix(aFormula);

            xub_StrLen nFStart = 0;
            xub_StrLen nFEnd   = 0;
            if ( GetFormulaHelper().GetNextFunc( aFormula, false, nFStart, &nFEnd) )
            {
                pScMod->InputReplaceSelection( aFormula );
                pScMod->InputSetSelection( nFStart, nFEnd );
                xub_StrLen PrivStart, PrivEnd;
                pScMod->InputGetSelection( PrivStart, PrivEnd);

                eMode = SetMeText(pScMod->InputGetFormulaStr(),PrivStart, PrivEnd,bMatrix,sal_True,sal_True);
                pData->SetFStart( nFStart );
            }
            else
                bEdit = false;
        }

        if ( !bEdit )
        {
            OUString aNewFormula('=');
            if ( aFormula.Len() > 0 && aFormula.GetChar(0) == '=' )
                aNewFormula = aFormula;

            pScMod->InputReplaceSelection( aNewFormula );
            pScMod->InputSetSelection( 1, aNewFormula.getLength()+1 );
            xub_StrLen PrivStart, PrivEnd;
            pScMod->InputGetSelection( PrivStart, PrivEnd);
            SetMeText(pScMod->InputGetFormulaStr(),PrivStart, PrivEnd,bMatrix,false,false);

            pData->SetFStart( 1 );      // hinter dem "="
        }

        pData->SetMode( (sal_uInt16) eMode );
        String rStrExp = GetMeText();

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
        StoreFormEditData(pData);
    } // if (pData) // wird nicht ueber Close zerstoert;
}

sal_Bool ScFormulaDlg::IsInputHdl(ScInputHandler* pHdl)
{
    sal_Bool bAlive = false;

    //  gehoert der InputHandler zu irgendeiner ViewShell ?

    TypeId aScType = TYPE(ScTabViewShell);
    SfxViewShell* pSh = SfxViewShell::GetFirst( &aScType );
    while ( pSh && !bAlive )
    {
        if (((ScTabViewShell*)pSh)->GetInputHandler() == pHdl)
            bAlive = sal_True;
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


sal_Bool ScFormulaDlg::Close()
{
    DoEnter(false);
    return sal_True;
}

//  --------------------------------------------------------------------------
//                          Funktionen fuer rechte Seite
//  --------------------------------------------------------------------------
bool ScFormulaDlg::calculateValue( const String& rStrExp, String& rStrResult )
{
    boost::scoped_ptr<ScSimpleFormulaCalculator> pFCell( new ScSimpleFormulaCalculator( pDoc, aCursorPos, rStrExp ) );

    // HACK! um bei ColRowNames kein #REF! zu bekommen,
    // wenn ein Name eigentlich als Bereich in die Gesamt-Formel
    // eingefuegt wird, bei der Einzeldarstellung aber als
    // single-Zellbezug interpretiert wird
    sal_Bool bColRowName = pFCell->HasColRowName();
    if ( bColRowName )
    {
        // ColRowName im RPN-Code?
        if ( pFCell->GetCode()->GetCodeLen() <= 1 )
        {   // ==1: einzelner ist als Parameter immer Bereich
            // ==0: es waere vielleicht einer, wenn..
            OUStringBuffer aBraced;
            aBraced.append('(');
            aBraced.append(rStrExp);
            aBraced.append(')');
            pFCell.reset( new ScSimpleFormulaCalculator( pDoc, aCursorPos, aBraced.makeStringAndClear() ) );
        }
        else
            bColRowName = false;
    }

    sal_uInt16 nErrCode = pFCell->GetErrCode();
    if ( nErrCode == 0 )
    {
        SvNumberFormatter& aFormatter = *(pDoc->GetFormatTable());
        Color* pColor;
        if ( pFCell->IsValue() )
        {
            double n = pFCell->GetValue();
            sal_uLong nFormat = aFormatter.GetStandardFormat( n, 0,
                            pFCell->GetFormatType(), ScGlobal::eLnge );
            aFormatter.GetOutputString( n, nFormat,
                                        rStrResult, &pColor );
        }
        else
        {
            OUString aStr = pFCell->GetString();
            OUString sTempOut(rStrResult);
            sal_uLong nFormat = aFormatter.GetStandardFormat(
                            pFCell->GetFormatType(), ScGlobal::eLnge);
            aFormatter.GetOutputString( aStr, nFormat,
                                        sTempOut, &pColor );
            rStrResult = sTempOut;
        }

        ScRange aTestRange;
        if ( bColRowName || (aTestRange.Parse(rStrExp) & SCA_VALID) )
            rStrResult.AppendAscii(RTL_CONSTASCII_STRINGPARAM( " ..." ));
            // Bereich
    }
    else
        rStrResult += ScGlobal::GetErrorString(nErrCode);

    return true;
}



//  virtuelle Methoden von ScAnyRefDlg:
void ScFormulaDlg::RefInputStart( formula::RefEdit* pEdit, formula::RefButton* pButton )
{
    pEdit->SetSelection(Selection(0, SELECTION_MAX));
    ::std::pair<formula::RefButton*,formula::RefEdit*> aPair = RefInputStartBefore( pEdit, pButton );
    m_aHelper.RefInputStart( aPair.second, aPair.first);
    RefInputStartAfter( aPair.second, aPair.first );
}
void ScFormulaDlg::RefInputDone( sal_Bool bForced )
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
        sal_Bool bRefNull = UpdateParaWin(theSel);

        if ( rRef.aStart != rRef.aEnd && bRefNull )
        {
            RefInputStart(GetActiveEdit());
        }

        String      aRefStr;
        sal_Bool bOtherDoc = ( pRefDoc != pDoc && pRefDoc->GetDocumentShell()->HasName() );
        if ( bOtherDoc )
        {
            //  Referenz auf anderes Dokument - wie inputhdl.cxx

            OSL_ENSURE(rRef.aStart.Tab()==rRef.aEnd.Tab(), "nStartTab!=nEndTab");

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
            OUStringBuffer aBuf;
            aComp.CreateStringFromTokenArray(aBuf);
            aRefStr = aBuf.makeStringAndClear();
        }

        UpdateParaWin(theSel,aRefStr);
    }
}

sal_Bool ScFormulaDlg::IsRefInputMode() const
{
    const IFunctionDescription* pDesc = getCurrentFunctionDescription();
    sal_Bool bRef = (pDesc && (pDesc->getSuppressedArgumentCount() > 0)) && (pDoc!=NULL);
    return bRef;
}

sal_Bool ScFormulaDlg::IsDocAllowed(SfxObjectShell* pDocSh) const
{
    //  not allowed: different from this doc, and no name
    //  pDocSh is always a ScDocShell
    if ( pDocSh && ((ScDocShell*)pDocSh)->GetDocument() != pDoc && !pDocSh->HasName() )
        return false;

    return sal_True;        // everything else is allowed
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

void ScFormulaDlg::doClose(sal_Bool /*_bOk*/)
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
void ScFormulaDlg::HideReference( sal_Bool bDoneRefMode )
{
    m_aHelper.HideReference(bDoneRefMode);
}
void ScFormulaDlg::ViewShellChanged()
{
    m_aHelper.ViewShellChanged();
}
void ScFormulaDlg::AddRefEntry( )
{

}
sal_Bool ScFormulaDlg::IsTableLocked( ) const
{
    // per Default kann bei Referenzeingabe auch die Tabelle umgeschaltet werden
    return false;
}
void ScFormulaDlg::ToggleCollapsed( formula::RefEdit* pEdit, formula::RefButton* pButton)
{
    m_aHelper.ToggleCollapsed(pEdit,pButton);
}
void ScFormulaDlg::ReleaseFocus( formula::RefEdit* pEdit, formula::RefButton* pButton)
{
    m_aHelper.ReleaseFocus(pEdit,pButton);
}
void ScFormulaDlg::dispatch(sal_Bool _bOK,sal_Bool _bMartixChecked)
{
    SfxBoolItem   aRetItem( SID_DLG_RETOK, _bOK );
    SfxBoolItem   aMatItem( SID_DLG_MATRIX, _bMartixChecked );
    SfxStringItem aStrItem( SCITEM_STRING, getCurrentFormula() );

    // Wenn durch Dokument-Umschalterei die Eingabezeile weg war/ist,
    // ist der String leer. Dann nicht die alte Formel loeschen.
    if ( aStrItem.GetValue().isEmpty() )
        aRetItem.SetValue( false );     // sal_False = Cancel

    m_aHelper.SetDispatcherLock( false ); // Modal-Modus ausschalten

    clear();

    GetBindings().GetDispatcher()->Execute( SID_INS_FUNCTION,
                              SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD,
                              &aRetItem, &aStrItem, &aMatItem, 0L );
}
void ScFormulaDlg::setDispatcherLock( sal_Bool bLock )
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
    {
        //fdo#69971 We need the EditEngine Modification handler of the inputbar that we
        //are feeding to be disabled while this dialog is open. Otherwise we end up in
        //a situation where...
        //a) this ScFormulaDlg changes the editengine
        //b) the modify callback gets called
        //c) which also modifies the editengine
        //d) on return from that modify handler the editengine attempts to use
        //   old node pointers which were replaced and removed by c
        //
        //We turn it off in the ctor and back on in the dtor, but if calc has
        //to repaint, e.g. when switching to another window and back, then in
        //ScMultiTextWnd::Paint a new editengine will have been created via
        //GetEditView with its default Modification handler enabled. So ensure
        //its off when we will access it via InputReplaceSelection
        pScMod->InputEnterHandler();
    }
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

SAL_WNODEPRECATED_DECLARATIONS_PUSH
::std::auto_ptr<formula::FormulaTokenArray> ScFormulaDlg::convertToTokenArray(const uno::Sequence< sheet::FormulaToken >& _aTokenList)
{
    ::std::auto_ptr<formula::FormulaTokenArray> pArray(new ScTokenArray());
    pArray->Fill( _aTokenList, pDoc->GetExternalRefManager());
    return pArray;
}
// for mysterious reasons Apple llvm-g++ 4.2.1 needs these explicit
// template instantiations; otherwise linking fails with unresolved symbols
template class ::std::auto_ptr<formula::FormulaTokenArray>;
template std::auto_ptr<formula::FormulaTokenArray>::operator std::auto_ptr_ref<formula::FormulaTokenArray>();
SAL_WNODEPRECATED_DECLARATIONS_POP

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
