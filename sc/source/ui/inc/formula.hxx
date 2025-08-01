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

#pragma once

#include <memory>
#include "anyrefdg.hxx"

#include <scmod.hxx>
#include <formula/formula.hxx>
#include "IAnyRefDialog.hxx"

class ScViewData;
class ScDocument;
class ScFuncDesc;
class ScInputHandler;
class ScDocShell;

class ScFormulaDlg final : public formula::FormulaDlg,
                     public IAnyRefDialog
{
    ScFormulaReferenceHelper m_aHelper;
    css::uno::Reference< css::sheet::XFormulaParser>          m_xParser;
    css::uno::Reference< css::sheet::XFormulaOpCodeMapper>    m_xOpCodeMapper;

            ScDocument*                 m_pDoc;
            ScAddress                   m_CursorPos;
            ScTabViewShell*             m_pViewShell;
    mutable std::shared_ptr<ScCompiler> m_xCompiler;

public:
    ScFormulaDlg( SfxBindings* pB, SfxChildWindow* pCW,
        weld::Window* pParent, const ScViewData& rViewData, const formula::IFunctionManager* _pFunctionMgr);
    virtual ~ScFormulaDlg() override;

    // IFormulaEditorHelper
    virtual void notifyChange() override;
    virtual void fill() override;
    virtual bool calculateValue(const OUString& _sExpression, OUString& _rResult, bool bMatrixFormula) override;
    virtual std::shared_ptr<formula::FormulaCompiler> getCompiler() const override;
    virtual std::unique_ptr<formula::FormulaCompiler> createCompiler( formula::FormulaTokenArray& rArray ) const override;
    virtual void doClose(bool _bOk) override;
    virtual void insertEntryToLRUList(const formula::IFunctionDescription*  pDesc) override;
    virtual void insertOrEraseFavouritesListEntry(const formula::IFunctionDescription* pDesc, bool bInsert) override;
    virtual void showReference(const OUString& _sFormula) override;
    virtual void dispatch(bool _bOK, bool _bMatrixChecked) override;
    virtual void setDispatcherLock( bool bLock ) override;
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

    virtual void Close() override;

    // sc::IAnyRefDialog
    virtual void ShowReference(const OUString& _sRef) override;
    virtual void HideReference( bool bDoneRefMode = true ) override;
    virtual void SetReference( const ScRange& rRef, ScDocument& rD ) override;

    virtual void ReleaseFocus( formula::RefEdit* pEdit ) override;
    virtual void ToggleCollapsed( formula::RefEdit* pEdit, formula::RefButton* pButton ) override;
    virtual void RefInputDone( bool bForced = false ) override;
    virtual bool IsTableLocked() const override;
    virtual bool IsRefInputMode() const override;

    virtual bool IsDocAllowed( SfxObjectShell* pDocSh ) const override;
    virtual void AddRefEntry() override;
    virtual void SetActive() override;
    virtual void ViewShellChanged() override;

private:
    void ImplDestroy();
    virtual void RefInputStart( formula::RefEdit* pEdit, formula::RefButton* pButton = nullptr ) override;
    static void  SaveLRUEntry(const ScFuncDesc* pFuncDesc);

    static bool  IsInputHdl(const ScInputHandler* pHdl);
    static ScInputHandler* GetNextInputHandler(const ScDocShell& rDocShell, ScTabViewShell** ppViewSh);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
