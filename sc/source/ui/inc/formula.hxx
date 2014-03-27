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
#include "global.hxx"
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


typedef ScTabViewShell* PtrTabViewShell;


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
    virtual void notifyChange() SAL_OVERRIDE;
    virtual void fill() SAL_OVERRIDE;
    virtual bool calculateValue(const OUString& _sExpression, OUString& _rResult) SAL_OVERRIDE;
    virtual void doClose(bool _bOk) SAL_OVERRIDE;
    virtual void insertEntryToLRUList(const formula::IFunctionDescription*  pDesc) SAL_OVERRIDE;
    virtual void showReference(const OUString& _sFormula) SAL_OVERRIDE;
    virtual void dispatch(bool _bOK, bool _bMatrixChecked) SAL_OVERRIDE;
    virtual void setDispatcherLock( bool bLock ) SAL_OVERRIDE;
    virtual void setReferenceInput(const formula::FormEditData* _pData) SAL_OVERRIDE;
    virtual void deleteFormData() SAL_OVERRIDE;
    virtual void clear() SAL_OVERRIDE;
    virtual void switchBack() SAL_OVERRIDE;
    virtual formula::FormEditData* getFormEditData() const SAL_OVERRIDE;
    virtual void setCurrentFormula(const OUString& _sReplacement) SAL_OVERRIDE;
    virtual void setSelection(sal_Int32 _nStart, sal_Int32 _nEnd) SAL_OVERRIDE;
    virtual void getSelection(sal_Int32& _nStart, sal_Int32& _nEnd) const SAL_OVERRIDE;
    virtual OUString getCurrentFormula() const SAL_OVERRIDE;

    virtual formula::IFunctionManager* getFunctionManager() SAL_OVERRIDE;
    virtual ::std::auto_ptr<formula::FormulaTokenArray> convertToTokenArray(const ::com::sun::star::uno::Sequence< ::com::sun::star::sheet::FormulaToken >& _aTokenList) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XFormulaParser> getFormulaParser() const SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XFormulaOpCodeMapper> getFormulaOpCodeMapper() const SAL_OVERRIDE;
    virtual ::com::sun::star::table::CellAddress getReferencePosition() const SAL_OVERRIDE;

    virtual bool    Close() SAL_OVERRIDE;

    // sc::IAnyRefDialog
    virtual void ShowReference(const OUString& _sRef) SAL_OVERRIDE;
    virtual void HideReference( bool bDoneRefMode = true ) SAL_OVERRIDE;
    virtual void SetReference( const ScRange& rRef, ScDocument* pD ) SAL_OVERRIDE;

    virtual void ReleaseFocus( formula::RefEdit* pEdit, formula::RefButton* pButton = NULL ) SAL_OVERRIDE;
    virtual void ToggleCollapsed( formula::RefEdit* pEdit, formula::RefButton* pButton = NULL ) SAL_OVERRIDE;
    virtual void RefInputDone( bool bForced = false ) SAL_OVERRIDE;
    virtual bool IsTableLocked() const SAL_OVERRIDE;
    virtual bool IsRefInputMode() const SAL_OVERRIDE;

    virtual bool IsDocAllowed( SfxObjectShell* pDocSh ) const SAL_OVERRIDE;
    virtual void AddRefEntry() SAL_OVERRIDE;
    virtual void SetActive() SAL_OVERRIDE;
    virtual void ViewShellChanged() SAL_OVERRIDE;
protected:

    virtual void RefInputStart( formula::RefEdit* pEdit, formula::RefButton* pButton = NULL ) SAL_OVERRIDE;
    void         SaveLRUEntry(const ScFuncDesc* pFuncDesc);

    bool         IsInputHdl(ScInputHandler* pHdl);
    ScInputHandler* GetNextInputHandler(ScDocShell* pDocShell,PtrTabViewShell* ppViewSh);
};



#endif // SC_CRNRDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
