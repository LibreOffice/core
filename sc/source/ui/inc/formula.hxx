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

#ifndef INCLUDED_SC_SOURCE_UI_INC_FORMULA_HXX
#define INCLUDED_SC_SOURCE_UI_INC_FORMULA_HXX

#include "anyrefdg.hxx"
#include "global.hxx"
#include <svtools/stdctrl.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/group.hxx>
#include <svtools/svmedit.hxx>
#include <vcl/tabpage.hxx>

#include "compiler.hxx"

#include <formula/formula.hxx>
#include "IAnyRefDialog.hxx"

class ScViewData;
class ScDocument;
class ScFuncDesc;
class ScInputHandler;
class ScDocShell;
class ScFormulaCell;

class ScFormulaDlg : public formula::FormulaDlg,
                     public IAnyRefDialog
{
    ScFormulaReferenceHelper m_aHelper;
    css::uno::Reference< css::sheet::XFormulaParser>          m_xParser;
    css::uno::Reference< css::sheet::XFormulaOpCodeMapper>    m_xOpCodeMapper;

    static ScDocument*  pDoc;
    static ScAddress    aCursorPos;
public:
                    ScFormulaDlg( SfxBindings* pB, SfxChildWindow* pCW,
                        vcl::Window* pParent, ScViewData* pViewData ,formula::IFunctionManager* _pFunctionMgr);
                    virtual ~ScFormulaDlg();
    virtual void dispose() override;

    // IFormulaEditorHelper
    virtual void notifyChange() override;
    virtual void fill() override;
    virtual bool calculateValue(const OUString& _sExpression, OUString& _rResult) override;
    virtual void doClose(bool _bOk) override;
    virtual void insertEntryToLRUList(const formula::IFunctionDescription*  pDesc) override;
    virtual void showReference(const OUString& _sFormula) override;
    virtual void dispatch(bool _bOK, bool _bMatrixChecked) override;
    virtual void setDispatcherLock( bool bLock ) override;
    virtual void setReferenceInput(const formula::FormEditData* _pData) override;
    virtual void deleteFormData() override;
    virtual void clear() override;
    virtual void switchBack() override;
    virtual formula::FormEditData* getFormEditData() const override;
    virtual void setCurrentFormula(const OUString& _sReplacement) override;
    virtual void setSelection(sal_Int32 _nStart, sal_Int32 _nEnd) override;
    virtual void getSelection(sal_Int32& _nStart, sal_Int32& _nEnd) const override;
    virtual OUString getCurrentFormula() const override;

    virtual formula::IFunctionManager* getFunctionManager() override;
    virtual ::std::unique_ptr<formula::FormulaTokenArray> convertToTokenArray(const css::uno::Sequence< css::sheet::FormulaToken >& _aTokenList) override;
    virtual css::uno::Reference< css::sheet::XFormulaParser> getFormulaParser() const override;
    virtual css::uno::Reference< css::sheet::XFormulaOpCodeMapper> getFormulaOpCodeMapper() const override;
    virtual css::table::CellAddress getReferencePosition() const override;

    virtual bool    Close() override;

    // sc::IAnyRefDialog
    virtual void ShowReference(const OUString& _sRef) override;
    virtual void HideReference( bool bDoneRefMode = true ) override;
    virtual void SetReference( const ScRange& rRef, ScDocument* pD ) override;

    virtual void ReleaseFocus( formula::RefEdit* pEdit, formula::RefButton* pButton = NULL ) override;
    virtual void ToggleCollapsed( formula::RefEdit* pEdit, formula::RefButton* pButton = NULL ) override;
    virtual void RefInputDone( bool bForced = false ) override;
    virtual bool IsTableLocked() const override;
    virtual bool IsRefInputMode() const override;

    virtual bool IsDocAllowed( SfxObjectShell* pDocSh ) const override;
    virtual void AddRefEntry() override;
    virtual void SetActive() override;
    virtual void ViewShellChanged() override;
protected:

    virtual void RefInputStart( formula::RefEdit* pEdit, formula::RefButton* pButton = NULL ) override;
    static void  SaveLRUEntry(const ScFuncDesc* pFuncDesc);

    static bool  IsInputHdl(ScInputHandler* pHdl);
    static ScInputHandler* GetNextInputHandler(ScDocShell* pDocShell, ScTabViewShell** ppViewSh);
};

#endif // INCLUDED_SC_SOURCE_UI_INC_FORMULA_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
