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

#include <memory>
#include <scitems.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/objsh.hxx>
#include <svl/zforlist.hxx>
#include <svl/stritem.hxx>
#include <sfx2/viewfrm.hxx>
#include <tools/urlobj.hxx>
#include <formula/formulahelper.hxx>
#include <formula/IFunctionDescription.hxx>
#include <formula/errorcodes.hxx>

#include <compiler.hxx>
#include <formula.hxx>
#include <formdata.hxx>
#include <reffact.hxx>
#include <document.hxx>
#include <simpleformulacalc.hxx>
#include <scmod.hxx>
#include <inputhdl.hxx>
#include <tabvwsh.hxx>
#include <docsh.hxx>
#include <funcdesc.hxx>
#include <tokenarray.hxx>
#include <sc.hrc>
#include <servuno.hxx>
#include <unonames.hxx>
#include <externalrefmgr.hxx>

#include <com/sun/star/table/CellAddress.hpp>
#include <com/sun/star/sheet/XFormulaOpCodeMapper.hpp>
#include <com/sun/star/sheet/XFormulaParser.hpp>

using namespace formula;
using namespace com::sun::star;

//      init/ shared functions for dialog

ScFormulaDlg::ScFormulaDlg( SfxBindings* pB, SfxChildWindow* pCW,
                                    vcl::Window* pParent, const ScViewData* pViewData, const formula::IFunctionManager* _pFunctionMgr )
    : formula::FormulaDlg( pB, pCW, pParent, _pFunctionMgr,this)
    , m_aHelper(this,pB)
{
    m_aHelper.SetWindow(this);
    ScModule* pScMod = SC_MOD();
    pScMod->InputEnterHandler();
    ScTabViewShell* pScViewShell = nullptr;

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
                pScViewShell = dynamic_cast<ScTabViewShell*>( pMyViewFrm->GetViewShell()  );
                if( pScViewShell )
                    pScViewShell->UpdateInputHandler(true);
            }
        }
    }

    m_pDoc = pViewData->GetDocument();
    m_xParser.set(ScServiceProvider::MakeInstance(ScServiceProvider::Type::FORMULAPARS,
                                                  static_cast<ScDocShell*>(m_pDoc->GetDocumentShell())),uno::UNO_QUERY);
    uno::Reference< beans::XPropertySet> xSet(m_xParser,uno::UNO_QUERY);
    xSet->setPropertyValue(SC_UNO_COMPILEFAP, uno::makeAny(true));

    m_xOpCodeMapper.set(ScServiceProvider::MakeInstance(ScServiceProvider::Type::OPCODEMAPPER,
                                                        static_cast<ScDocShell*>(m_pDoc->GetDocumentShell())),uno::UNO_QUERY);

    ScInputHandler* pInputHdl = SC_MOD()->GetInputHdl(pScViewShell);

    OSL_ENSURE( pInputHdl, "Missing input handler :-/" );

    if ( pInputHdl )
        pInputHdl->NotifyChange( nullptr );

    ScFormulaReferenceHelper::enableInput( true );
    ScFormulaReferenceHelper::EnableSpreadsheets();
    m_aHelper.Init();
    m_aHelper.SetDispatcherLock( true );

    notifyChange();
    fill();

    ScFormEditData* pData = pScMod->GetFormEditData();
    if (!pData)
    {
        pScMod->SetRefInputHdl(pScMod->GetInputHdl());

        m_pDoc = pViewData->GetDocument();
        SCCOL nCol = pViewData->GetCurX();
        SCROW nRow = pViewData->GetCurY();
        SCTAB nTab = pViewData->GetTabNo();
        m_CursorPos = ScAddress( nCol, nRow, nTab );

        pScMod->InitFormEditData();                             // create new
        pData = pScMod->GetFormEditData();
        pData->SetInputHandler(pScMod->GetInputHdl());
        pData->SetDocShell(pViewData->GetDocShell());

        OSL_ENSURE(pData,"FormEditData not available");

        formula::FormulaDlgMode eMode = FormulaDlgMode::Formula;            // default...

        // edit if formula exists

        OUString aFormula;
        m_pDoc->GetFormula( nCol, nRow, nTab, aFormula );
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

        pData->SetMode( eMode );
        OUString rStrExp = GetMeText();

        Update(rStrExp);
    }

}

void ScFormulaDlg::notifyChange()
{
    ScModule* pScMod = SC_MOD();

    ScInputHandler* pInputHdl = pScMod->GetInputHdl();
    if ( pInputHdl )
        pInputHdl->NotifyChange( nullptr );
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
        //  don't reinitialise m_pDoc and m_CursorPos
        //pDoc = pViewData->GetDocument();
        if(IsInputHdl(pData->GetInputHandler()))
        {
            pScMod->SetRefInputHdl(pData->GetInputHandler());
        }
        else
        {
            ScTabViewShell* pTabViewShell;
            ScInputHandler* pInputHdl = GetNextInputHandler(pData->GetDocShell(),&pTabViewShell);

            if ( pInputHdl == nullptr ) //no more InputHandler for DocShell
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
        pScMod->SetRefInputHdl(nullptr);
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

    if (pData) // close doesn't destroy;
    {
        //set back reference input handler
        pScMod->SetRefInputHdl(nullptr);
        StoreFormEditData(pData);
    }
    formula::FormulaDlg::dispose();
}

bool ScFormulaDlg::IsInputHdl(const ScInputHandler* pHdl)
{
    bool bAlive = false;

    //  belongs InputHandler to a ViewShell?

    SfxViewShell* pSh = SfxViewShell::GetFirst( true, checkSfxViewShell<ScTabViewShell> );
    while ( pSh && !bAlive )
    {
        if (static_cast<ScTabViewShell*>(pSh)->GetInputHandler() == pHdl)
            bAlive = true;
        pSh = SfxViewShell::GetNext( *pSh, true, checkSfxViewShell<ScTabViewShell> );
    }

    return bAlive;

}

ScInputHandler* ScFormulaDlg::GetNextInputHandler(const ScDocShell* pDocShell, ScTabViewShell** ppViewSh)
{
    ScInputHandler* pHdl=nullptr;

    SfxViewFrame* pFrame = SfxViewFrame::GetFirst( pDocShell );
    while( pFrame && pHdl==nullptr)
    {
        SfxViewShell* p = pFrame->GetViewShell();
        ScTabViewShell* pViewSh = dynamic_cast< ScTabViewShell *>( p );
        if(pViewSh!=nullptr)
        {
            pHdl=pViewSh->GetInputHandler();
            if(ppViewSh!=nullptr) *ppViewSh=pViewSh;
        }
        pFrame = SfxViewFrame::GetNext( *pFrame, pDocShell );
    }

    return pHdl;
}

bool ScFormulaDlg::Close()
{
    DoEnter();
    return true;
}

//                          functions for right side

bool ScFormulaDlg::calculateValue( const OUString& rStrExp, OUString& rStrResult, bool bMatrixFormula )
{
    std::unique_ptr<ScSimpleFormulaCalculator> pFCell( new ScSimpleFormulaCalculator(
                m_pDoc, m_CursorPos, rStrExp, bMatrixFormula));
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
            pFCell.reset( new ScSimpleFormulaCalculator(
                        m_pDoc, m_CursorPos, aBraced.makeStringAndClear(), bMatrixFormula));
            pFCell->SetLimitString(true);
        }
        else
            bColRowName = false;
    }

    FormulaError nErrCode = pFCell->GetErrCode();
    if ( nErrCode == FormulaError::NONE || pFCell->IsMatrix() )
    {
        SvNumberFormatter& aFormatter = *(m_pDoc->GetFormatTable());
        Color* pColor;
        if (pFCell->IsMatrix())
        {
            rStrResult = pFCell->GetString().getString();
        }
        else if (pFCell->IsValue())
        {
            double n = pFCell->GetValue();
            sal_uLong nFormat = aFormatter.GetStandardFormat( n, 0,
                            pFCell->GetFormatType(), ScGlobal::eLnge );
            aFormatter.GetOutputString( n, nFormat, rStrResult, &pColor );
        }
        else
        {
            sal_uLong nFormat = aFormatter.GetStandardFormat(
                            pFCell->GetFormatType(), ScGlobal::eLnge);
            aFormatter.GetOutputString( pFCell->GetString().getString(), nFormat,
                                        rStrResult, &pColor );
            // Indicate it's a string, so a number string doesn't look numeric.
            // Escape embedded quotation marks first by doubling them, as
            // usual. Actually the result can be copy-pasted from the result
            // box as literal into a formula expression.
            rStrResult = "\"" + rStrResult.replaceAll( "\"", "\"\"") + "\"";
        }

        ScRange aTestRange;
        if ( bColRowName || (aTestRange.Parse(rStrExp) & ScRefFlags::VALID) )
            rStrResult += " ...";
            // area
    }
    else
        rStrResult += ScGlobal::GetErrorString(nErrCode);

    return true;
}

std::shared_ptr<formula::FormulaCompiler> ScFormulaDlg::getCompiler() const
{
    if (!m_xCompiler)
        m_xCompiler.reset( new ScCompiler( m_pDoc, m_CursorPos, m_pDoc->GetGrammar()));
    return m_xCompiler;
}

std::unique_ptr<formula::FormulaCompiler> ScFormulaDlg::createCompiler( formula::FormulaTokenArray& rArray ) const
{
    ScCompiler* pCompiler = nullptr;
    ScTokenArray* pArr = dynamic_cast<ScTokenArray*>(&rArray);
    assert(pArr);   // violation of contract and not created using convertToTokenArray()?
    if (pArr)
        pCompiler = new ScCompiler( m_pDoc, m_CursorPos, *pArr, m_pDoc->GetGrammar());
    return std::unique_ptr<formula::FormulaCompiler>(pCompiler);
}

//  virtual methods of ScAnyRefDlg:
void ScFormulaDlg::RefInputStart( formula::RefEdit* pEdit, formula::RefButton* pButton )
{
    pEdit->SetSelection(Selection(0, SELECTION_MAX));
    ::std::pair<formula::RefButton*,formula::RefEdit*> aPair = RefInputStartBefore( pEdit, pButton );
    m_aHelper.RefInputStart( aPair.second, aPair.first);
    RefInputStartAfter();
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
        bool bOtherDoc = (pRefDoc != m_pDoc && pRefDoc->GetDocumentShell()->HasName());
        if ( bOtherDoc )
        {
            //  reference to other document - like inputhdl.cxx

            OSL_ENSURE(rRef.aStart.Tab()==rRef.aEnd.Tab(), "nStartTab!=nEndTab");

            // Always 3D and absolute.
            OUString aTmp( rRef.Format( ScRefFlags::VALID | ScRefFlags::TAB_ABS_3D, pRefDoc));

            SfxObjectShell* pObjSh = pRefDoc->GetDocumentShell();

            // #i75893# convert escaped URL of the document to something user friendly
//           OUString aFileName = pObjSh->GetMedium()->GetName();
            OUString aFileName = pObjSh->GetMedium()->GetURLObject().GetMainURL( INetURLObject::DecodeMechanism::Unambiguous );

            aRefStr = "'" + aFileName + "'#" + aTmp;
        }
        else
        {
            // We can't use ScRange::Format here because in R1C1 mode we need
            // to display the reference position relative to the cursor
            // position.
            ScTokenArray aArray;
            ScComplexRefData aRefData;
            aRefData.InitRangeRel(rRef, m_CursorPos);
            bool bSingle = aRefData.Ref1 == aRefData.Ref2;
            if (m_CursorPos.Tab() != rRef.aStart.Tab())
            {
                // pointer-selected => absolute sheet reference
                aRefData.Ref1.SetAbsTab( rRef.aStart.Tab() );
                aRefData.Ref1.SetFlag3D(true);
            }
            if (bSingle)
                aArray.AddSingleReference(aRefData.Ref1);
            else
                aArray.AddDoubleReference(aRefData);
            ScCompiler aComp(m_pDoc, m_CursorPos, aArray, m_pDoc->GetGrammar());
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
    bool bRef = (pDesc && (pDesc->getSuppressedArgumentCount() > 0)) && (m_pDoc != nullptr);
    return bRef;
}

bool ScFormulaDlg::IsDocAllowed(SfxObjectShell* pDocSh) const
{
    //  not allowed: different from this doc, and no name
    //  pDocSh is always a ScDocShell
    return !(pDocSh && &static_cast<ScDocShell*>(pDocSh)->GetDocument() != m_pDoc && !pDocSh->HasName());        // everything else is allowed
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
void ScFormulaDlg::ReleaseFocus( formula::RefEdit* pEdit)
{
    m_aHelper.ReleaseFocus(pEdit);
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

    m_aHelper.SetDispatcherLock( false ); // turn off modal-mode

    clear();

    GetBindings().GetDispatcher()->ExecuteList( SID_INS_FUNCTION,
                              SfxCallMode::ASYNCHRON | SfxCallMode::RECORD,
                              { &aRetItem, &aStrItem, &aMatItem });
}
void ScFormulaDlg::setDispatcherLock( bool bLock )
{
    m_aHelper.SetDispatcherLock( bLock );
}
void ScFormulaDlg::deleteFormData()
{
    ScModule* pScMod = SC_MOD();
    pScMod->ClearFormEditData();        // pData is invalid!
}
void ScFormulaDlg::clear()
{
    m_pDoc = nullptr;

    //restore reference inputhandler
    ScModule* pScMod = SC_MOD();
    pScMod->SetRefInputHdl(nullptr);

    // force Enable() of edit line
    ScTabViewShell* pScViewShell = dynamic_cast<ScTabViewShell*>( SfxViewShell::Current() );
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
        pHdl->ViewShellGone(nullptr);  // -> get active view
        pHdl->ShowRefFrame();
    }

    // restore current chart (cause mouse-RefInput)
    ScTabViewShell* pScViewShell = dynamic_cast<ScTabViewShell*>( SfxViewShell::Current() );
    if ( pScViewShell )
    {
        ScViewData& rVD=pScViewShell->GetViewData();
        SCTAB nExecTab = m_CursorPos.Tab();
        if ( nExecTab != rVD.GetTabNo() )
            pScViewShell->SetTabNo( nExecTab );

        SCROW nRow = m_CursorPos.Row();
        SCCOL nCol = m_CursorPos.Col();

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
    return table::CellAddress(m_CursorPos.Tab(), m_CursorPos.Col(), m_CursorPos.Row());
}

::std::unique_ptr<formula::FormulaTokenArray> ScFormulaDlg::convertToTokenArray(const uno::Sequence< sheet::FormulaToken >& _aTokenList)
{
    ::std::unique_ptr<formula::FormulaTokenArray> pArray(new ScTokenArray());
    pArray->Fill(_aTokenList, m_pDoc->GetSharedStringPool(), m_pDoc->GetExternalRefManager());
    return pArray;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
