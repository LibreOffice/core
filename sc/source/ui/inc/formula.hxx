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

#ifndef SC_FORMULA_HXX
#define SC_FORMULA_HXX

#include "anyrefdg.hxx"
#include "global.hxx"       // ScAddress
#include <svtools/stdctrl.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/group.hxx>
#include <svtools/svmedit.hxx>
#include <vcl/tabpage.hxx>

#include "compiler.hxx"

#include "formula/formula.hxx"
#include "IAnyRefDialog.hxx"

class ScViewData;
class ScDocument;
class ScFuncDesc;
class ScInputHandler;
class ScDocShell;
class ScFormulaCell;

//============================================================================
typedef ScTabViewShell* PtrTabViewShell;
//============================================================================

class ScFormulaDlg : public formula::FormulaDlg,
                     public IAnyRefDialog
{
    ScFormulaReferenceHelper m_aHelper;
    ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XFormulaParser>          m_xParser;
    ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XFormulaOpCodeMapper>    m_xOpCodeMapper;

    static ScDocument*  pDoc;
    static ScAddress    aCursorPos;
public:
                    ScFormulaDlg( SfxBindings* pB, SfxChildWindow* pCW,
                        Window* pParent, ScViewData* pViewData ,formula::IFunctionManager* _pFunctionMgr);
                    ~ScFormulaDlg();

    // IFormulaEditorHelper
    virtual void notifyChange();
    virtual void fill();
    virtual bool calculateValue(const String& _sExpression,String& _rResult);
    virtual void doClose(sal_Bool _bOk);
    virtual void insertEntryToLRUList(const formula::IFunctionDescription*  pDesc);
    virtual void showReference(const String& _sFormula);
    virtual void dispatch(sal_Bool _bOK,sal_Bool _bMartixChecked);
    virtual void setDispatcherLock( sal_Bool bLock );
    virtual void setReferenceInput(const formula::FormEditData* _pData);
    virtual void deleteFormData();
    virtual void clear();
    virtual void switchBack();
    virtual formula::FormEditData* getFormEditData() const;
    virtual void setCurrentFormula(const String& _sReplacement);
    virtual void setSelection(xub_StrLen _nStart,xub_StrLen _nEnd);
    virtual void getSelection(xub_StrLen& _nStart,xub_StrLen& _nEnd) const;
    virtual String getCurrentFormula() const;

    virtual formula::IFunctionManager* getFunctionManager();
    virtual ::std::auto_ptr<formula::FormulaTokenArray> convertToTokenArray(const ::com::sun::star::uno::Sequence< ::com::sun::star::sheet::FormulaToken >& _aTokenList);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XFormulaParser> getFormulaParser() const;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XFormulaOpCodeMapper> getFormulaOpCodeMapper() const;
    virtual ::com::sun::star::table::CellAddress getReferencePosition() const;

    virtual sal_Bool    Close();

    // sc::IAnyRefDialog
    virtual void ShowReference(const OUString& _sRef);
    virtual void HideReference( sal_Bool bDoneRefMode = sal_True );
    virtual void SetReference( const ScRange& rRef, ScDocument* pD );

    virtual void ReleaseFocus( formula::RefEdit* pEdit, formula::RefButton* pButton = NULL );
    virtual void ToggleCollapsed( formula::RefEdit* pEdit, formula::RefButton* pButton = NULL );
    virtual void RefInputDone( sal_Bool bForced = false );
    virtual sal_Bool IsTableLocked() const;
    virtual sal_Bool IsRefInputMode() const;

    virtual sal_Bool IsDocAllowed( SfxObjectShell* pDocSh ) const;
    virtual void AddRefEntry();
    virtual void SetActive();
    virtual void ViewShellChanged();
protected:

    virtual void RefInputStart( formula::RefEdit* pEdit, formula::RefButton* pButton = NULL );
    void         SaveLRUEntry(const ScFuncDesc* pFuncDesc);

    sal_Bool        IsInputHdl(ScInputHandler* pHdl);
    ScInputHandler* GetNextInputHandler(ScDocShell* pDocShell,PtrTabViewShell* ppViewSh);
};



#endif // SC_CRNRDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
