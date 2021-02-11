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

#include <unotools/viewoptions.hxx>
#include <formula/formdata.hxx>
#include <formula/funcutl.hxx>
#include <formula/tokenarray.hxx>
#include <formula/FormulaCompiler.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>

#include <memory>

#include <Formula.hxx>
#include <AddField.hxx>
#include <helpids.h>


namespace rptui
{
    using namespace formula;
    using namespace ::com::sun::star;


//      initialization / shared functions for the dialog


FormulaDialog::FormulaDialog(weld::Window* pParent
                             , const uno::Reference<lang::XMultiServiceFactory>& _xServiceFactory
                             , const std::shared_ptr< IFunctionManager >&  _pFunctionMgr
                             , const OUString& _sFormula
                             , const css::uno::Reference < css::beans::XPropertySet >& _xRowSet
                             , svl::SharedStringPool& rStrPool )
    : FormulaModalDialog( pParent, _pFunctionMgr.get(),this)
    ,m_aFunctionManager(_pFunctionMgr)
    ,m_xFormulaData(new FormEditData())
    ,m_xRowSet(_xRowSet)
    ,m_pEdit(nullptr)
    ,m_sFormula("=")
    ,m_nStart(0)
    ,m_nEnd(1)
    ,mrStringPool(rStrPool)
{
    if ( !_sFormula.isEmpty() )
    {
        if ( _sFormula[0] != '=' )
            m_sFormula += _sFormula;
        else
            m_sFormula = _sFormula;
    }
    m_xParser.set(_xServiceFactory->createInstance("org.libreoffice.report.pentaho.SOFormulaParser"),uno::UNO_QUERY);
    if ( m_xParser.is() )
        m_xOpCodeMapper = m_xParser->getFormulaOpCodeMapper();
    fill();
}

void FormulaDialog::notifyChange()
{
}

void FormulaDialog::fill()
{
    SetMeText(m_sFormula);
    Update(m_sFormula);
    CheckMatrix(m_sFormula);
    Update();
}

FormulaDialog::~FormulaDialog()
{
    if ( m_xAddField )
    {
        SvtViewOptions aDlgOpt( EViewType::Window, HID_RPT_FIELD_SEL_WIN );
        aDlgOpt.SetWindowState(OStringToOUString(m_xAddField->getDialog()->get_window_state(WindowStateMask::X | WindowStateMask::Y | WindowStateMask::State | WindowStateMask::Minimized), RTL_TEXTENCODING_ASCII_US));

        if (m_xAddField->getDialog()->get_visible())
            m_xAddField->response(RET_CANCEL);

        m_xAddField.reset();
    }

    StoreFormEditData(m_xFormulaData.get());
    m_pEdit = nullptr;
}

// functions for right side

bool FormulaDialog::calculateValue( const OUString& rStrExp, OUString& rStrResult, bool /*bMatrixFormula*/ )
{
    rStrResult = rStrExp;
    return false;
}

std::shared_ptr<formula::FormulaCompiler> FormulaDialog::getCompiler() const
{
    return nullptr;
}

std::unique_ptr<formula::FormulaCompiler> FormulaDialog::createCompiler( formula::FormulaTokenArray& rArray ) const
{
    return std::unique_ptr<formula::FormulaCompiler>(new FormulaCompiler(rArray));
}

void FormulaDialog::doClose(bool _bOk)
{
    response(_bOk ? RET_OK : RET_CANCEL);
}

void FormulaDialog::insertEntryToLRUList(const IFunctionDescription*    /*_pDesc*/)
{
}
void FormulaDialog::showReference(const OUString& /*_sFormula*/)
{
}
void FormulaDialog::dispatch(bool /*_bOK*/, bool /*_bMatrixChecked*/)
{
}
void FormulaDialog::setDispatcherLock( bool /*bLock*/ )
{
}
void FormulaDialog::deleteFormData()
{
}
void FormulaDialog::clear()
{
}
void FormulaDialog::switchBack()
{
}
FormEditData* FormulaDialog::getFormEditData() const
{
    return m_xFormulaData.get();
}
void FormulaDialog::setCurrentFormula(const OUString& _sReplacement)
{
    const sal_Int32 nOldLen = m_nEnd - m_nStart;
    const sal_Int32 nNewLen = _sReplacement.getLength();
    if (nOldLen)
        m_sFormula = m_sFormula.replaceAt( m_nStart, nOldLen, "" );
    if (nNewLen)
        m_sFormula = m_sFormula.replaceAt( m_nStart, 0, _sReplacement );
    m_nEnd = m_nStart + nNewLen;
}
void FormulaDialog::setSelection(sal_Int32 _nStart, sal_Int32 _nEnd)
{
    if ( _nStart <= _nEnd )
    {
        m_nStart = _nStart;
        m_nEnd = _nEnd;
    }
    else
    {
        m_nEnd = _nStart;
        m_nStart = _nEnd;
    }
}
void FormulaDialog::getSelection(sal_Int32& _nStart, sal_Int32& _nEnd) const
{
    _nStart = m_nStart;
    _nEnd = m_nEnd;
}
OUString FormulaDialog::getCurrentFormula() const
{
    return m_sFormula;
}
IFunctionManager* FormulaDialog::getFunctionManager()
{
    return m_aFunctionManager.get();
}

void FormulaDialog::ShowReference(const OUString& /*_sRef*/)
{
}

void FormulaDialog::HideReference( bool /*bDoneRefMode*/)
{
}

void FormulaDialog::ReleaseFocus( RefEdit* /*pEdit*/)
{
}

void FormulaDialog::ToggleCollapsed( RefEdit* _pEdit, RefButton* _pButton)
{
    ::std::pair<RefButton*,RefEdit*> aPair = RefInputStartBefore( _pEdit, _pButton );
    m_pEdit = aPair.second;
    if ( m_pEdit )
        m_pEdit->GetWidget()->hide();
    if ( aPair.first )
        aPair.first->GetWidget()->hide();

    if (!m_xAddField)
    {
        m_xAddField = std::make_shared<OAddFieldWindow>(m_xDialog.get(), m_xRowSet);
        m_xAddField->SetCreateHdl(LINK( this, FormulaDialog, OnClickHdl ) );
        SvtViewOptions aDlgOpt( EViewType::Window, HID_RPT_FIELD_SEL_WIN );
        if ( aDlgOpt.Exists() )
        {
            m_xAddField->getDialog()->set_window_state(OUStringToOString(aDlgOpt.GetWindowState(), RTL_TEXTENCODING_ASCII_US));

        }

        m_xAddField->Update();
    }
    RefInputStartAfter();

    if (!m_xAddField->getDialog()->get_visible())
        weld::DialogController::runAsync(m_xAddField, [this](sal_Int32 /*nResult*/) { m_xAddField.reset(); });
}

IMPL_LINK( FormulaDialog, OnClickHdl, OAddFieldWindow& ,_rAddFieldDlg, void)
{
    const uno::Sequence< beans::PropertyValue > aArgs = _rAddFieldDlg.getSelectedFieldDescriptors();
    // we use this way to create undo actions
    if ( m_pEdit && aArgs.getLength() == 1)
    {
        uno::Sequence< beans::PropertyValue > aValue;
        aArgs[0].Value >>= aValue;
        svx::ODataAccessDescriptor aDescriptor(aValue);
        OUString sName;
        aDescriptor[ svx::DataAccessDescriptorProperty::ColumnName ] >>= sName;
        if ( !sName.isEmpty() )
        {
            sName = "[" + sName + "]";
            m_pEdit->SetText(sName);
        }
    }
    m_pEdit = nullptr;
    if (_rAddFieldDlg.getDialog()->get_visible())
        _rAddFieldDlg.response(RET_CANCEL);
    RefInputDoneAfter();
}

uno::Reference< sheet::XFormulaParser> FormulaDialog::getFormulaParser() const
{
    return m_xParser;
}

uno::Reference< sheet::XFormulaOpCodeMapper> FormulaDialog::getFormulaOpCodeMapper() const
{
    return m_xOpCodeMapper;
}

table::CellAddress FormulaDialog::getReferencePosition() const
{
    return table::CellAddress();
}

::std::unique_ptr<formula::FormulaTokenArray> FormulaDialog::convertToTokenArray(const uno::Sequence< sheet::FormulaToken >& _aTokenList)
{
    ::std::unique_ptr<formula::FormulaTokenArray> pArray(new FormulaTokenArray());
    pArray->Fill(_aTokenList, mrStringPool, nullptr);
    return pArray;
}

} // rptui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
