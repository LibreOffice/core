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
#include <formula/token.hxx>
#include "tokenarray.hxx"
#include "sc.hrc"
#include "servuno.hxx"
#include "unonames.hxx"
#include "externalrefmgr.hxx"

#include <com/sun/star/table/CellAddress.hpp>

using namespace formula;
using namespace com::sun::star;

ScDocument* ScFormulaDlg::pDoc = NULL;
ScAddress ScFormulaDlg::aCursorPos;

//      init/ shared functions for dialog

ScFormulaDlg::ScFormulaDlg( SfxBindings* pB, SfxChildWindow* pCW,
                                    vcl::Window* pParent, ScViewData* pViewData,formula::IFunctionManager* _pFunctionMgr )
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
    m_xParser.set(ScServiceProvider::MakeInstance(SC_SERVICE_FORMULAPARS, static_cast<ScDocShell*>(pDoc->GetDocumentShell())),uno::UNO_QUERY);
    uno::Reference< beans::XPropertySet> xSet(m_xParser,uno::UNO_QUERY);
    xSet->setPropertyValue(OUString(SC_UNO_COMPILEFAP),uno::makeAny(sal_True));

    m_xOpCodeMapper.set(ScServiceProvider::MakeInstance(SC_SERVICE_OPCODEMAPPER, static_cast<ScDocShell*>(pDoc->GetDocumentShell())),uno::UNO_QUERY);

    ScInputHandler* pInputHdl = SC_MOD()->GetInputHdl(pScViewShell);

    OSL_ENSURE( pInputHdl, "Missing input handler :-/" );

    if ( pInputHdl )
        pInputHdl->NotifyChange( NULL );

    ScFormulaReferenceHelper::enableInput( false );
    ScFormulaReferenceHelper::EnableSpreadsheets();
    m_aHelper.Init();
    ScFormulaReferenceHelper::SetDispatcherLock( true );

    notifyChange();
    fill();

    ScFormEditData* pData = pScMod->GetFormEditData();
    if (!pData)
    {
        pScMod->SetRefInputHdl(pScMod->GetInputHdl());

        pDoc = pViewData->GetDocument();
        SCCOL nCol = pViewData->GetCurX();
        SCROW nRow = pViewData->GetCurY();
        SCTAB nTab = pViewData->GetTabNo();
        aCursorPos = ScAddress( nCol, nRow, nTab );

        pScMod->InitFormEditData();                             // create new
        pData = pScMod->GetFormEditData();
        pData->SetInputHandler(pScMod->GetInputHdl());
        pData->SetDocShell(pViewData->GetDocShell());

        OSL_ENSURE(pData,"FormEditData ist nicht da");

        formula::FormulaDlgMode eMode = FORMULA_FORMDLG_FORMULA;            // default...

        // edit if formula exists

        OUString aFormula;
        pDoc->GetFormula( nCol, nRow, nTab, aFormula );
        bool bEdit   = ( aFormula.getLength() > 1 );
        bool bMatrix = false;
        if ( bEdit )
        {
            bMatrix = CheckMatrix(aFormula);

            sal_Int32 nFStart = 0;
            sal_Int32 nFEnd   = 0;
            if ( GetFormulaHelper().GetNextFunc( aFormula, false, nFStart, &nFEnd) )
            {
                pScMod->InputReplaceSelection( aFormula );
                pScMod->InputSetSelection( nFStart, nFEnd );
                sal_Int32 PrivStart, PrivEnd;
                pScMod->InputGetSelection( PrivStart, PrivEnd);

                eMode = SetMeText(pScMod->InputGetFormulaStr(),PrivStart, PrivEnd, bMatrix, true, true);
                pData->SetFStart( nFStart );
            }
            else
                bEdit = false;
        }

        if ( !bEdit )
        {
            OUString aNewFormula('=');
            if ( aFormula.startsWith("=") )
                aNewFormula = aFormula;

            pScMod->InputReplaceSelection( aNewFormula );
            pScMod->InputSetSelection( 1, aNewFormula.getLength()+1 );
            sal_Int32 PrivStart, PrivEnd;
            pScMod->InputGetSelection( PrivStart, PrivEnd);
            SetMeText(pScMod->InputGetFormulaStr(),PrivStart, PrivEnd,bMatrix,false,false);

            pData->SetFStart( 1 );      // after "="
        }

        pData->SetMode( (sal_uInt16) eMode );
        OUString rStrExp = GetMeText();

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

void ScFormulaDlg::fill()
{
    ScModule* pScMod = SC_MOD();
    ScFormEditData* pData = pScMod->GetFormEditData();
    notifyChange();
    OUString rStrExp;
    if (pData)
    {
        //  data exists -> restore state (after switch)
        //  don't reinitialise pDoc and aCursorPos
        //pDoc = pViewData->GetDocument();
        if(IsInputHdl(pData->GetInputHandler()))
        {
            pScMod->SetRefInputHdl(pData->GetInputHandler());
        }
        else
        {
            ScTabViewShell* pTabViewShell;
            ScInputHandler* pInputHdl = GetNextInputHandler(pData->GetDocShell(),&pTabViewShell);

            if ( pInputHdl == NULL ) //no more InputHandler for DocShell
            {
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

        OUString aOldFormulaTmp(pScMod->InputGetFormulaStr());
        pScMod->InputSetSelection( 0, aOldFormulaTmp.getLength());

        rStrExp=pData->GetUndoStr();
        pScMod->InputReplaceSelection(rStrExp);

        SetMeText(rStrExp);

        Update();
        // switch back, maybe new Doc has been opened
        pScMod->SetRefInputHdl(NULL);
    }
}

ScFormulaDlg::~ScFormulaDlg()
{
    disposeOnce();
}

void ScFormulaDlg::dispose()
{
    ScModule* pScMod = SC_MOD();
    ScFormEditData* pData = pScMod->GetFormEditData();
    m_aHelper.dispose();

    if (pData) // close dosen't destroy;
    {
        //set back reference input handler
        pScMod->SetRefInputHdl(NULL);
        StoreFormEditData(pData);
    }
    formula::FormulaDlg::dispose();
}

bool ScFormulaDlg::IsInputHdl(ScInputHandler* pHdl)
{
    bool bAlive = false;

    //  belongs InputHandler to a ViewShell?

    TypeId aScType = TYPE(ScTabViewShell);
    SfxViewShell* pSh = SfxViewShell::GetFirst( &aScType );
    while ( pSh && !bAlive )
    {
        if (static_cast<ScTabViewShell*>(pSh)->GetInputHandler() == pHdl)
            bAlive = true;
        pSh = SfxViewShell::GetNext( *pSh, &aScType );
    }

    return bAlive;

}

ScInputHandler* ScFormulaDlg::GetNextInputHandler(ScDocShell* pDocShell, ScTabViewShell** ppViewSh)
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

bool ScFormulaDlg::Close()
{
    DoEnter(false);
    return true;
}

//                          functions for right side

bool ScFormulaDlg::calculateValue( const OUString& rStrExp, OUString& rStrResult )
{
    boost::scoped_ptr<ScSimpleFormulaCalculator> pFCell( new ScSimpleFormulaCalculator( pDoc, aCursorPos, rStrExp ) );
    pFCell->SetLimitString(true);

    // HACK! to avoid neither #REF! from ColRowNames
    // if a name is added as actually range in the overall formula,
    // but is interpreted at the individual representation as single-cell reference
    bool bColRowName = pFCell->HasColRowName();
    if ( bColRowName )
    {
        // ColRowName from RPN-Code?
        if ( pFCell->GetCode()->GetCodeLen() <= 1 )
        {   // ==1: area
            // ==0: would be an area if...
            OUStringBuffer aBraced;
            aBraced.append('(');
            aBraced.append(rStrExp);
            aBraced.append(')');
            pFCell.reset( new ScSimpleFormulaCalculator( pDoc, aCursorPos, aBraced.makeStringAndClear() ) );
            pFCell->SetLimitString(true);
        }
        else
            bColRowName = false;
    }

    sal_uInt16 nErrCode = pFCell->GetErrCode();
    if ( nErrCode == 0 || pFCell->IsMatrix() )
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
            sal_uLong nFormat = aFormatter.GetStandardFormat(
                            pFCell->GetFormatType(), ScGlobal::eLnge);
            aFormatter.GetOutputString( pFCell->GetString().getString(), nFormat,
                                        rStrResult, &pColor );
        }

        ScRange aTestRange;
        if ( bColRowName || (aTestRange.Parse(rStrExp) & SCA_VALID) )
            rStrResult += " ...";
            // area
    }
    else
        rStrResult += ScGlobal::GetErrorString(nErrCode);

    return true;
}

//  virtual methods of ScAnyRefDlg:
void ScFormulaDlg::RefInputStart( formula::RefEdit* pEdit, formula::RefButton* pButton )
{
    pEdit->SetSelection(Selection(0, SELECTION_MAX));
    ::std::pair<formula::RefButton*,formula::RefEdit*> aPair = RefInputStartBefore( pEdit, pButton );
    m_aHelper.RefInputStart( aPair.second, aPair.first);
    RefInputStartAfter( aPair.second, aPair.first );
}

void ScFormulaDlg::RefInputDone( bool bForced )
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
        bool bRefNull = UpdateParaWin(theSel);

        if ( rRef.aStart != rRef.aEnd && bRefNull )
        {
            RefInputStart(GetActiveEdit());
        }

        OUString      aRefStr;
        bool bOtherDoc = ( pRefDoc != pDoc && pRefDoc->GetDocumentShell()->HasName() );
        if ( bOtherDoc )
        {
            //  reference to other document - wie inputhdl.cxx

            OSL_ENSURE(rRef.aStart.Tab()==rRef.aEnd.Tab(), "nStartTab!=nEndTab");

            OUString aTmp(rRef.Format(SCA_VALID|SCA_TAB_3D, pRefDoc));     // immer 3d

            SfxObjectShell* pObjSh = pRefDoc->GetDocumentShell();

            // #i75893# convert escaped URL of the document to something user friendly
//           OUString aFileName = pObjSh->GetMedium()->GetName();
            OUString aFileName = pObjSh->GetMedium()->GetURLObject().GetMainURL( INetURLObject::DECODE_UNAMBIGUOUS );

            aRefStr = "'";
            aRefStr += aFileName;
            aRefStr += "'#";
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

bool ScFormulaDlg::IsRefInputMode() const
{
    const IFunctionDescription* pDesc = getCurrentFunctionDescription();
    bool bRef = (pDesc && (pDesc->getSuppressedArgumentCount() > 0)) && (pDoc!=NULL);
    return bRef;
}

bool ScFormulaDlg::IsDocAllowed(SfxObjectShell* pDocSh) const
{
    //  not allowed: different from this doc, and no name
    //  pDocSh is always a ScDocShell
    if ( pDocSh && &static_cast<ScDocShell*>(pDocSh)->GetDocument() != pDoc && !pDocSh->HasName() )
        return false;

    return true;        // everything else is allowed
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

void ScFormulaDlg::doClose(bool /*_bOk*/)
{
    m_aHelper.DoClose( ScFormulaDlgWrapper::GetChildWindowId() );
}
void ScFormulaDlg::insertEntryToLRUList(const formula::IFunctionDescription*    _pDesc)
{
    const ScFuncDesc* pDesc = dynamic_cast<const ScFuncDesc*>(_pDesc);
    SaveLRUEntry(pDesc);
}
void ScFormulaDlg::showReference(const OUString& _sFormula)
{
    ShowReference(_sFormula);
}
void ScFormulaDlg::ShowReference(const OUString& _sFormula)
{
    m_aHelper.ShowReference(_sFormula);
}
void ScFormulaDlg::HideReference( bool bDoneRefMode )
{
    m_aHelper.HideReference(bDoneRefMode);
}
void ScFormulaDlg::ViewShellChanged()
{
    ScFormulaReferenceHelper::ViewShellChanged();
}
void ScFormulaDlg::AddRefEntry( )
{

}
bool ScFormulaDlg::IsTableLocked( ) const
{
    // default: reference input can also be used to switch the table
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
void ScFormulaDlg::dispatch(bool _bOK, bool _bMatrixChecked)
{
    SfxBoolItem   aRetItem( SID_DLG_RETOK, _bOK );
    SfxBoolItem   aMatItem( SID_DLG_MATRIX, _bMatrixChecked );
    SfxStringItem aStrItem( SCITEM_STRING, getCurrentFormula() );

    // if edit line is empty (caused by document switching) -> string is empty
    // -> don't delete old formula
    if ( aStrItem.GetValue().isEmpty() )
        aRetItem.SetValue( false );     // sal_False = Cancel

    ScFormulaReferenceHelper::SetDispatcherLock( false ); // turn off modal-mode

    clear();

    GetBindings().GetDispatcher()->Execute( SID_INS_FUNCTION,
                              SfxCallMode::ASYNCHRON | SfxCallMode::RECORD,
                              &aRetItem, &aStrItem, &aMatItem, 0L );
}
void ScFormulaDlg::setDispatcherLock( bool bLock )
{
    ScFormulaReferenceHelper::SetDispatcherLock( bLock );
}
void ScFormulaDlg::setReferenceInput(const formula::FormEditData* _pData)
{
    ScModule* pScMod = SC_MOD();
    ScFormEditData& rData = const_cast<ScFormEditData&>(dynamic_cast<const ScFormEditData&>(*_pData));
    pScMod->SetRefInputHdl(rData.GetInputHandler());
}
void ScFormulaDlg::deleteFormData()
{
    ScModule* pScMod = SC_MOD();
    pScMod->ClearFormEditData();        // pData is invalid!
}
void ScFormulaDlg::clear()
{
    pDoc = NULL;

    //restore reference inputhandler
    ScModule* pScMod = SC_MOD();
    pScMod->SetRefInputHdl(NULL);

    // force Enable() of edit line
    ScTabViewShell* pScViewShell = PTR_CAST(ScTabViewShell, SfxViewShell::Current());
    if ( pScViewShell )
        pScViewShell->UpdateInputHandler();
}
void ScFormulaDlg::switchBack()
{
    ScModule* pScMod = SC_MOD();
    // back to the document
    // (foreign doc could be above - #34222#)
    ScInputHandler* pHdl = pScMod->GetInputHdl();
    if ( pHdl )
    {
        pHdl->ViewShellGone(NULL);  // -> get active view
        pHdl->ShowRefFrame();
    }

    // restore current chart (cause mouse-RefInput)
    ScTabViewShell* pScViewShell = PTR_CAST(ScTabViewShell, SfxViewShell::Current());
    if ( pScViewShell )
    {
        ScViewData& rVD=pScViewShell->GetViewData();
        SCTAB nExecTab = aCursorPos.Tab();
        if ( nExecTab != rVD.GetTabNo() )
            pScViewShell->SetTabNo( nExecTab );

        SCROW nRow=aCursorPos.Row();
        SCCOL nCol=aCursorPos.Col();

        if(rVD.GetCurX()!=nCol || rVD.GetCurY()!=nRow)
            pScViewShell->SetCursor(nCol,nRow);
    }
}
formula::FormEditData* ScFormulaDlg::getFormEditData() const
{
    ScModule* pScMod = SC_MOD();
    return pScMod->GetFormEditData();
}
void ScFormulaDlg::setCurrentFormula(const OUString& _sReplacement)
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
        pScMod->InputTurnOffWinEngine();
    }
    pScMod->InputReplaceSelection(_sReplacement);
}
void ScFormulaDlg::setSelection(sal_Int32 _nStart, sal_Int32 _nEnd)
{
    ScModule* pScMod = SC_MOD();
    pScMod->InputSetSelection( _nStart, _nEnd );
}
void ScFormulaDlg::getSelection(sal_Int32& _nStart, sal_Int32& _nEnd) const
{
    ScModule* pScMod = SC_MOD();
    pScMod->InputGetSelection( _nStart, _nEnd );
}
OUString ScFormulaDlg::getCurrentFormula() const
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

::std::unique_ptr<formula::FormulaTokenArray> ScFormulaDlg::convertToTokenArray(const uno::Sequence< sheet::FormulaToken >& _aTokenList)
{
    ::std::unique_ptr<formula::FormulaTokenArray> pArray(new ScTokenArray());
    pArray->Fill(_aTokenList, pDoc->GetSharedStringPool(), pDoc->GetExternalRefManager());
    return pArray;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
