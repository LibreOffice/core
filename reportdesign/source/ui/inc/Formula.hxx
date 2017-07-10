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

#ifndef INCLUDED_REPORTDESIGN_SOURCE_UI_INC_FORMULA_HXX
#define INCLUDED_REPORTDESIGN_SOURCE_UI_INC_FORMULA_HXX

#include <formula/formula.hxx>
#include <formula/IControlReferenceHandler.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/report/meta/XFormulaParser.hpp>
#include <memory>

namespace com { namespace sun { namespace star { namespace lang {
    class XMultiServiceFactory;
} } } }

namespace svl {

class SharedStringPool;

}

namespace rptui
{

class OAddFieldWindow;


class FormulaDialog : public formula::FormulaModalDialog,
                      public formula::IControlReferenceHandler
{
    std::shared_ptr< formula::IFunctionManager > m_aFunctionManager;
    formula::FormEditData*             m_pFormulaData;
    VclPtr<OAddFieldWindow>            m_pAddField;
    css::uno::Reference < css::beans::XPropertySet >          m_xRowSet;
    css::uno::Reference< css::report::meta::XFormulaParser>   m_xParser;
    css::uno::Reference< css::sheet::XFormulaOpCodeMapper>    m_xOpCodeMapper;
    VclPtr<formula::RefEdit>           m_pEdit;
    OUString                           m_sFormula;
    sal_Int32                          m_nStart;
    sal_Int32                          m_nEnd;

    svl::SharedStringPool&             mrStringPool;

    DECL_LINK( OnClickHdl, OAddFieldWindow&, void );
public:
    FormulaDialog( vcl::Window* pParent
        , const css::uno::Reference< css::lang::XMultiServiceFactory>& _xServiceFactory
        , const std::shared_ptr< formula::IFunctionManager >& _pFunctionMgr
        , const OUString& _sFormula
        , const css::uno::Reference < css::beans::XPropertySet >& _xRowSet
        , svl::SharedStringPool& rStrPool );

    virtual ~FormulaDialog() override;
    virtual void dispose() override;

    // IFormulaEditorHelper
    virtual void notifyChange() override;
    virtual void fill() override;
    virtual bool calculateValue(const OUString& _sExpression, OUString& _rResult, bool bMatrixFormula) override;
    virtual std::shared_ptr<formula::FormulaCompiler> getCompiler() const;
    virtual void doClose(bool _bOk) override;
    virtual void insertEntryToLRUList(const formula::IFunctionDescription*  pDesc) override;
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
    virtual css::uno::Reference< css::sheet::XFormulaParser> getFormulaParser() const override;
    virtual css::uno::Reference< css::sheet::XFormulaOpCodeMapper> getFormulaOpCodeMapper() const override;
    virtual css::table::CellAddress getReferencePosition() const override;

    virtual ::std::unique_ptr<formula::FormulaTokenArray> convertToTokenArray(const css::uno::Sequence< css::sheet::FormulaToken >& _aTokenList) override;

    // IControlReferenceHandler
    virtual void ShowReference(const OUString& _sRef) override;
    virtual void HideReference( bool bDoneRefMode = true ) override;
    virtual void ReleaseFocus( formula::RefEdit* pEdit ) override;
    virtual void ToggleCollapsed( formula::RefEdit* pEdit, formula::RefButton* pButton ) override;
};


} // rptui


#endif // INCLUDED_REPORTDESIGN_SOURCE_UI_INC_FORMULA_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
