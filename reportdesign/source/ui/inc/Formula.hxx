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
    ::com::sun::star::uno::Reference < ::com::sun::star::beans::XPropertySet >          m_xRowSet;
    ::com::sun::star::uno::Reference< ::com::sun::star::report::meta::XFormulaParser>   m_xParser;
    ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XFormulaOpCodeMapper>    m_xOpCodeMapper;
    VclPtr<formula::RefEdit>           m_pEdit;
    OUString                    m_sFormula;
    sal_Int32                   m_nStart;
    sal_Int32                   m_nEnd;

    svl::SharedStringPool& mrStringPool;

    DECL_LINK_TYPED( OnClickHdl, OAddFieldWindow&, void );
public:
    FormulaDialog( vcl::Window* pParent
        , const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>& _xServiceFactory
        , const std::shared_ptr< formula::IFunctionManager >& _pFunctionMgr
        , const OUString& _sFormula
        , const ::com::sun::star::uno::Reference < ::com::sun::star::beans::XPropertySet >& _xRowSet
        , svl::SharedStringPool& rStrPool );

    virtual ~FormulaDialog();
    virtual void dispose() SAL_OVERRIDE;

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
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XFormulaParser> getFormulaParser() const SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XFormulaOpCodeMapper> getFormulaOpCodeMapper() const SAL_OVERRIDE;
    virtual ::com::sun::star::table::CellAddress getReferencePosition() const SAL_OVERRIDE;

    virtual ::std::unique_ptr<formula::FormulaTokenArray> convertToTokenArray(const ::com::sun::star::uno::Sequence< ::com::sun::star::sheet::FormulaToken >& _aTokenList) SAL_OVERRIDE;

    // IControlReferenceHandler
    virtual void ShowReference(const OUString& _sRef) SAL_OVERRIDE;
    virtual void HideReference( bool bDoneRefMode = true ) SAL_OVERRIDE;
    virtual void ReleaseFocus( formula::RefEdit* pEdit, formula::RefButton* pButton = NULL ) SAL_OVERRIDE;
    virtual void ToggleCollapsed( formula::RefEdit* pEdit, formula::RefButton* pButton = NULL ) SAL_OVERRIDE;
};


} // rptui


#endif // INCLUDED_REPORTDESIGN_SOURCE_UI_INC_FORMULA_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
