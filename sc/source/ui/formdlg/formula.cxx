/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
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


using namespace formula;
using namespace com::sun::star;

ScDocument* ScFormulaDlg::pDoc = NULL;
ScAddress ScFormulaDlg::aCursorPos;







ScFormulaDlg::ScFormulaDlg( SfxBindings* pB, SfxChildWindow* pCW,
                                    Window* pParent, ScViewData* pViewData,formula::IFunctionManager* _pFunctionMgr )
    : formula::FormulaDlg( pB, pCW, pParent, true,true,true, _pFunctionMgr,this)
    , m_aHelper(this,pB)
{
    m_aHelper.SetWindow(this);
    ScModule* pScMod = SC_MOD();
    pScMod->InputEnterHandler();
    ScTabViewShell* pScViewShell = NULL;

    
    

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
    m_aHelper.SetDispatcherLock( true );

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

        pScMod->InitFormEditData();                             
        pData = pScMod->GetFormEditData();
        pData->SetInputHandler(pScMod->GetInputHdl());
        pData->SetDocShell(pViewData->GetDocShell());

        OSL_ENSURE(pData,"FormEditData ist nicht da");

        formula::FormulaDlgMode eMode = FORMULA_FORMDLG_FORMULA;            

        

        OUString aFormula;
        pDoc->GetFormula( nCol, nRow, nTab, aFormula );
        sal_Bool bEdit   = ( aFormula.getLength() > 1 );
        sal_Bool bMatrix = false;
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

            pData->SetFStart( 1 );      
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
        
        
        
        if(IsInputHdl(pData->GetInputHandler()))
        {
            pScMod->SetRefInputHdl(pData->GetInputHandler());
        }
        else
        {
            PtrTabViewShell pTabViewShell;
            ScInputHandler* pInputHdl = GetNextInputHandler(pData->GetDocShell(),&pTabViewShell);

            if ( pInputHdl == NULL ) 
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
        
        pScMod->SetRefInputHdl(NULL);
    }
}

ScFormulaDlg::~ScFormulaDlg()
{
    ScModule* pScMod = SC_MOD();
    ScFormEditData* pData = pScMod->GetFormEditData();

    if (pData) 
    {
        
        pScMod->SetRefInputHdl(NULL);
        StoreFormEditData(pData);
    } 
}

bool ScFormulaDlg::IsInputHdl(ScInputHandler* pHdl)
{
    bool bAlive = false;

    

    TypeId aScType = TYPE(ScTabViewShell);
    SfxViewShell* pSh = SfxViewShell::GetFirst( &aScType );
    while ( pSh && !bAlive )
    {
        if (((ScTabViewShell*)pSh)->GetInputHandler() == pHdl)
            bAlive = true;
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


bool ScFormulaDlg::Close()
{
    DoEnter(false);
    return true;
}




bool ScFormulaDlg::calculateValue( const OUString& rStrExp, OUString& rStrResult )
{
    boost::scoped_ptr<ScSimpleFormulaCalculator> pFCell( new ScSimpleFormulaCalculator( pDoc, aCursorPos, rStrExp ) );

    
    
    
    
    sal_Bool bColRowName = pFCell->HasColRowName();
    if ( bColRowName )
    {
        
        if ( pFCell->GetCode()->GetCodeLen() <= 1 )
        {   
            
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
            OUString sTempOut(rStrResult);
            sal_uLong nFormat = aFormatter.GetStandardFormat( n, 0,
                            pFCell->GetFormatType(), ScGlobal::eLnge );

            aFormatter.GetOutputString( n, nFormat,
                                        sTempOut, &pColor );
            rStrResult = sTempOut;
        }
        else
        {
            OUString aStr = pFCell->GetString().getString();
            OUString sTempOut(rStrResult);
            sal_uLong nFormat = aFormatter.GetStandardFormat(
                            pFCell->GetFormatType(), ScGlobal::eLnge);
            aFormatter.GetOutputString( aStr, nFormat,
                                        sTempOut, &pColor );
            rStrResult = sTempOut;
        }

        ScRange aTestRange;
        if ( bColRowName || (aTestRange.Parse(rStrExp) & SCA_VALID) )
            rStrResult += " ...";
            
    }
    else
        rStrResult += ScGlobal::GetErrorString(nErrCode);

    return true;
}




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
        sal_Bool bRefNull = UpdateParaWin(theSel);

        if ( rRef.aStart != rRef.aEnd && bRefNull )
        {
            RefInputStart(GetActiveEdit());
        }

        OUString      aRefStr;
        sal_Bool bOtherDoc = ( pRefDoc != pDoc && pRefDoc->GetDocumentShell()->HasName() );
        if ( bOtherDoc )
        {
            

            OSL_ENSURE(rRef.aStart.Tab()==rRef.aEnd.Tab(), "nStartTab!=nEndTab");

            OUString aTmp(rRef.Format(SCA_VALID|SCA_TAB_3D, pRefDoc));     

            SfxObjectShell* pObjSh = pRefDoc->GetDocumentShell();

            

            OUString aFileName = pObjSh->GetMedium()->GetURLObject().GetMainURL( INetURLObject::DECODE_UNAMBIGUOUS );

            aRefStr = "'";
            aRefStr += aFileName;
            aRefStr += "'#";
            aRefStr += aTmp;
        }
        else
        {
            
            
            
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
    
    
    if ( pDocSh && ((ScDocShell*)pDocSh)->GetDocument() != pDoc && !pDocSh->HasName() )
        return false;

    return true;        
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
    m_aHelper.ViewShellChanged();
}
void ScFormulaDlg::AddRefEntry( )
{

}
bool ScFormulaDlg::IsTableLocked( ) const
{
    
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

    
    
    if ( aStrItem.GetValue().isEmpty() )
        aRetItem.SetValue( false );     

    m_aHelper.SetDispatcherLock( false ); 

    clear();

    GetBindings().GetDispatcher()->Execute( SID_INS_FUNCTION,
                              SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD,
                              &aRetItem, &aStrItem, &aMatItem, 0L );
}
void ScFormulaDlg::setDispatcherLock( bool bLock )
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
    pScMod->ClearFormEditData();        
}
void ScFormulaDlg::clear()
{
    pDoc = NULL;

    
    ScModule* pScMod = SC_MOD();
    pScMod->SetRefInputHdl(NULL);

    
    ScTabViewShell* pScViewShell = PTR_CAST(ScTabViewShell, SfxViewShell::Current());
    if ( pScViewShell )
        pScViewShell->UpdateInputHandler();
}
void ScFormulaDlg::switchBack()
{
    ScModule* pScMod = SC_MOD();
    
    
    ScInputHandler* pHdl = pScMod->GetInputHdl();
    if ( pHdl )
    {
        pHdl->ViewShellGone(NULL);  
        pHdl->ShowRefFrame();
    }

    
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
void ScFormulaDlg::setCurrentFormula(const OUString& _sReplacement)
{
    ScModule* pScMod = SC_MOD();
    {
        
        
        
        
        
        
        
        
        //
        
        
        
        
        
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

SAL_WNODEPRECATED_DECLARATIONS_PUSH
::std::auto_ptr<formula::FormulaTokenArray> ScFormulaDlg::convertToTokenArray(const uno::Sequence< sheet::FormulaToken >& _aTokenList)
{
    ::std::auto_ptr<formula::FormulaTokenArray> pArray(new ScTokenArray());
    pArray->Fill( _aTokenList, pDoc->GetExternalRefManager());
    return pArray;
}


template class ::std::auto_ptr<formula::FormulaTokenArray>;
template std::auto_ptr<formula::FormulaTokenArray>::operator std::auto_ptr_ref<formula::FormulaTokenArray>();
SAL_WNODEPRECATED_DECLARATIONS_POP

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
