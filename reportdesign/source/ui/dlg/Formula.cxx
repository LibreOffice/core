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

#include <vcl/svapp.hxx>
#include <vcl/mnemonic.hxx>
#include <vcl/msgbox.hxx>
#include <unotools/charclass.hxx>
#include <unotools/viewoptions.hxx>
#include <formula/formdata.hxx>
#include <formula/funcutl.hxx>
#include <formula/tokenarray.hxx>

#include <boost/scoped_ptr.hpp>

#include "Formula.hxx"
#include "AddField.hxx"
#include "helpids.hrc"

//============================================================================
namespace rptui
{
    using namespace formula;
    using namespace ::com::sun::star;

//  --------------------------------------------------------------------------
//      Initialisierung / gemeinsame Funktionen  fuer Dialog
//  --------------------------------------------------------------------------

FormulaDialog::FormulaDialog(Window* pParent
                             , const uno::Reference<lang::XMultiServiceFactory>& _xServiceFactory
                             , const ::boost::shared_ptr< IFunctionManager >&  _pFunctionMgr
                             , const OUString& _sFormula
                             , const ::com::sun::star::uno::Reference < ::com::sun::star::beans::XPropertySet >& _xRowSet)
    : FormulaModalDialog( pParent, false,false,false,_pFunctionMgr.get(),this)
    ,m_aFunctionManager(_pFunctionMgr)
    ,m_pFormulaData(new FormEditData())
    ,m_pAddField(NULL)
    ,m_xRowSet(_xRowSet)
    ,m_pEdit(NULL)
    ,m_sFormula("=")
    ,m_nStart(0)
    ,m_nEnd(1)
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
// -----------------------------------------------------------------------------
void FormulaDialog::fill()
{
    SetMeText(m_sFormula);
    Update(m_sFormula);
    CheckMatrix(m_sFormula);
    Update();
}

FormulaDialog::~FormulaDialog()
{
    if ( m_pAddField )
    {
        SvtViewOptions aDlgOpt( E_WINDOW, OUString( HID_RPT_FIELD_SEL_WIN ) );
        aDlgOpt.SetWindowState(OStringToOUString(m_pAddField->GetWindowState((WINDOWSTATE_MASK_X | WINDOWSTATE_MASK_Y | WINDOWSTATE_MASK_STATE | WINDOWSTATE_MASK_MINIMIZED)), RTL_TEXTENCODING_ASCII_US));

        boost::scoped_ptr<Window> aTemp2(m_pAddField);
        m_pAddField = NULL;
    }

    StoreFormEditData( m_pFormulaData );
}

//  --------------------------------------------------------------------------
//                          Funktionen fuer rechte Seite
//  --------------------------------------------------------------------------
bool FormulaDialog::calculateValue( const OUString& rStrExp, OUString& rStrResult )
{
    rStrResult = rStrExp;
    return false;
}
void FormulaDialog::doClose(sal_Bool _bOk)
{
    EndDialog(_bOk ? RET_OK : RET_CANCEL);
}
void FormulaDialog::insertEntryToLRUList(const IFunctionDescription*    /*_pDesc*/)
{
}
void FormulaDialog::showReference(const OUString& /*_sFormula*/)
{
}
void FormulaDialog::dispatch(sal_Bool /*_bOK*/,sal_Bool /*_bMartixChecked*/)
{
}
void FormulaDialog::setDispatcherLock( sal_Bool /*bLock*/ )
{
}
void FormulaDialog::setReferenceInput(const FormEditData* /*_pData*/)
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
    return m_pFormulaData;
}
void FormulaDialog::setCurrentFormula(const OUString& _sReplacement)
{
    const xub_StrLen nOldLen = m_nEnd - m_nStart;
    const sal_Int32 nNewLen = _sReplacement.getLength();
    if (nOldLen)
        m_sFormula = m_sFormula.replaceAt( m_nStart, nOldLen, "" );
    if (nNewLen)
        m_sFormula = m_sFormula.replaceAt( m_nStart, 0, _sReplacement );
    m_nEnd = m_nStart + nNewLen;
}
void FormulaDialog::setSelection(xub_StrLen _nStart,xub_StrLen _nEnd)
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
void FormulaDialog::getSelection(xub_StrLen& _nStart,xub_StrLen& _nEnd) const
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
// -----------------------------------------------------------------------------
void FormulaDialog::ShowReference(const OUString& /*_sRef*/)
{
}
// -----------------------------------------------------------------------------
void FormulaDialog::HideReference( sal_Bool /*bDoneRefMode*/)
{
}
// -----------------------------------------------------------------------------
void FormulaDialog::ReleaseFocus( RefEdit* /*pEdit*/, RefButton* /*pButton*/)
{
}
// -----------------------------------------------------------------------------
void FormulaDialog::ToggleCollapsed( RefEdit* _pEdit, RefButton* _pButton)
{
    ::std::pair<RefButton*,RefEdit*> aPair = RefInputStartBefore( _pEdit, _pButton );
    m_pEdit = aPair.second;
    if ( m_pEdit )
        m_pEdit->Hide();
    if ( aPair.first )
        aPair.first->Hide();

    if ( !m_pAddField )
    {
        m_pAddField = new OAddFieldWindow(this,m_xRowSet);
        m_pAddField->SetCreateHdl(LINK( this, FormulaDialog, OnClickHdl ) );
        SvtViewOptions aDlgOpt( E_WINDOW, OUString( HID_RPT_FIELD_SEL_WIN ) );
        if ( aDlgOpt.Exists() )
        {
            m_pAddField->SetWindowState(OUStringToOString(aDlgOpt.GetWindowState().getStr(), RTL_TEXTENCODING_ASCII_US));

        }

        m_pAddField->Update();
    }
    RefInputStartAfter( aPair.second, aPair.first );
    m_pAddField->Show();

}
// -----------------------------------------------------------------------------
IMPL_LINK( FormulaDialog, OnClickHdl, OAddFieldWindow* ,_pAddFieldDlg)
{
    const uno::Sequence< beans::PropertyValue > aArgs = _pAddFieldDlg->getSelectedFieldDescriptors();
    // we use this way to create undo actions
    if ( m_pEdit && aArgs.getLength() == 1)
    {
        uno::Sequence< beans::PropertyValue > aValue;
        aArgs[0].Value >>= aValue;
        ::svx::ODataAccessDescriptor aDescriptor(aValue);
        OUString sName;
        aDescriptor[ ::svx::daColumnName ] >>= sName;
        if ( !sName.isEmpty() )
        {
            sName = OUString("[") + sName + OUString("]");
            m_pEdit->SetText(sName);
        }
    }
    m_pEdit = NULL;
    _pAddFieldDlg->Hide();
    RefInputDoneAfter( sal_True );

    return 0L;
}
// -----------------------------------------------------------------------------
uno::Reference< sheet::XFormulaParser> FormulaDialog::getFormulaParser() const
{
    return m_xParser.get();
}
// -----------------------------------------------------------------------------
uno::Reference< sheet::XFormulaOpCodeMapper> FormulaDialog::getFormulaOpCodeMapper() const
{
    return m_xOpCodeMapper;
}
// -----------------------------------------------------------------------------
table::CellAddress FormulaDialog::getReferencePosition() const
{
    return table::CellAddress();
}
// -----------------------------------------------------------------------------
SAL_WNODEPRECATED_DECLARATIONS_PUSH
::std::auto_ptr<formula::FormulaTokenArray> FormulaDialog::convertToTokenArray(const uno::Sequence< sheet::FormulaToken >& _aTokenList)
{
    ::std::auto_ptr<formula::FormulaTokenArray> pArray(new FormulaTokenArray());
    pArray->Fill(_aTokenList, NULL);
    return pArray;
}

} // rptui

// for mysterious reasons Apple llvm-g++ 4.2.1 needs these explicit
// template instantiations; otherwise linking fails with unresolved symbols
template class ::std::auto_ptr<formula::FormulaTokenArray>;
template std::auto_ptr<formula::FormulaTokenArray>::operator std::auto_ptr_ref<formula::FormulaTokenArray>();
SAL_WNODEPRECATED_DECLARATIONS_POP

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
