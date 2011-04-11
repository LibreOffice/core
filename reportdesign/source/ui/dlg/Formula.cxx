/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_reportdesign.hxx"


//----------------------------------------------------------------------------

#include <vcl/svapp.hxx>
#include <vcl/mnemonic.hxx>
#include <vcl/msgbox.hxx>
#include <unotools/charclass.hxx>
#include <unotools/viewoptions.hxx>
#include <tools/urlobj.hxx>
#include <formula/formdata.hxx>
#include <formula/funcutl.hxx>
#include <formula/tokenarray.hxx>

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
                             , const ::rtl::OUString& _sFormula
                             , const ::com::sun::star::uno::Reference < ::com::sun::star::beans::XPropertySet >& _xRowSet)
    : FormulaModalDialog( pParent, false,false,false,this,_pFunctionMgr.get(),this)
    ,m_aFunctionManager(_pFunctionMgr)
    ,m_pFormulaData(new FormEditData())
    ,m_pAddField(NULL)
    ,m_xRowSet(_xRowSet)
    ,m_pEdit(NULL)
    ,m_sFormula(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("=")))
    ,m_nStart(0)
    ,m_nEnd(1)
{
    if ( _sFormula.getLength() > 0 )
    {
        if ( _sFormula.getStr()[0] != '=' )
            m_sFormula += String(_sFormula);
        else
            m_sFormula = _sFormula;
    }
    m_xParser.set(_xServiceFactory->createInstance(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.report.pentaho.SOFormulaParser"))),uno::UNO_QUERY);
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
        SvtViewOptions aDlgOpt( E_WINDOW, String::CreateFromAscii( HID_RPT_FIELD_SEL_WIN ) );
        aDlgOpt.SetWindowState( ::rtl::OUString::createFromAscii( m_pAddField->GetWindowState((WINDOWSTATE_MASK_X | WINDOWSTATE_MASK_Y | WINDOWSTATE_MASK_STATE | WINDOWSTATE_MASK_MINIMIZED)).GetBuffer() ) );

        ::std::auto_ptr<Window> aTemp2(m_pAddField);
        m_pAddField = NULL;
    }
}

//  --------------------------------------------------------------------------
//                          Funktionen fuer rechte Seite
//  --------------------------------------------------------------------------
bool FormulaDialog::calculateValue( const String& rStrExp, String& rStrResult )
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
void FormulaDialog::showReference(const String& /*_sFormula*/)
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
void FormulaDialog::setCurrentFormula(const String& _sReplacement)
{
    const xub_StrLen nOldLen = m_nEnd - m_nStart;
    const xub_StrLen nNewLen = _sReplacement.Len();
    if (nOldLen)
        m_sFormula.Erase( m_nStart, nOldLen );
    if (nNewLen)
        m_sFormula.Insert( _sReplacement, m_nStart );
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
String FormulaDialog::getCurrentFormula() const
{
    return m_sFormula;
}
IFunctionManager* FormulaDialog::getFunctionManager()
{
    return m_aFunctionManager.get();
}
// -----------------------------------------------------------------------------
void FormulaDialog::ShowReference(const String& /*_sRef*/)
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
        SvtViewOptions aDlgOpt( E_WINDOW, String::CreateFromAscii( HID_RPT_FIELD_SEL_WIN ) );
        if ( aDlgOpt.Exists() )
        {
            m_pAddField->SetWindowState( ByteString( aDlgOpt.GetWindowState().getStr(), RTL_TEXTENCODING_ASCII_US ) );

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
        ::rtl::OUString sName;
        aDescriptor[ ::svx::daColumnName ] >>= sName;
        if ( sName.getLength() )
        {
            sName = ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("[")) + sName + ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("]"));
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
::std::auto_ptr<formula::FormulaTokenArray> FormulaDialog::convertToTokenArray(const uno::Sequence< sheet::FormulaToken >& _aTokenList)
{
    ::std::auto_ptr<formula::FormulaTokenArray> pArray(new FormulaTokenArray());
    pArray->Fill(_aTokenList, NULL);
    return pArray;
}
// =============================================================================
} // rptui
// =============================================================================

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
