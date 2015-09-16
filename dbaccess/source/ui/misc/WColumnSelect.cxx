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

#include "WColumnSelect.hxx"
#include "dbu_misc.hrc"
#include <osl/diagnose.h>
#include "WCopyTable.hxx"
#include <com/sun/star/sdbcx/XDataDescriptorFactory.hpp>
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <com/sun/star/sdbcx/XAppend.hpp>
#include "moduledbu.hxx"
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/sdbc/ColumnValue.hpp>
#include <com/sun/star/sdb/application/CopyTableOperation.hpp>
#include "dbustrings.hrc"
#include <functional>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace dbaui;

namespace CopyTableOperation = ::com::sun::star::sdb::application::CopyTableOperation;

OUString OWizColumnSelect::GetTitle() const { return ModuleRes(STR_WIZ_COLUMN_SELECT_TITEL); }

OWizardPage::OWizardPage(vcl::Window* pParent, const OString& rID, const OUString& rUIXMLDescription)
    : TabPage(pParent, rID, rUIXMLDescription)
     ,m_pParent(static_cast<OCopyTableWizard*>(pParent))
     ,m_bFirstTime(true)
{
}

OWizardPage::~OWizardPage()
{
    disposeOnce();
}

void OWizardPage::dispose()
{
    m_pParent.clear();
    TabPage::dispose();
}

// OWizColumnSelect
OWizColumnSelect::OWizColumnSelect( vcl::Window* pParent)
    :OWizardPage( pParent, "ApplyColPage", "dbaccess/ui/applycolpage.ui")
{
    get(m_pOrgColumnNames, "from");
    get(m_pColumn_RH, "colrh");
    get(m_pColumns_RH, "colsrh");
    get(m_pColumn_LH, "collh");
    get(m_pColumns_LH, "colslh");
    get(m_pNewColumnNames, "to");

    Size aSize(approximate_char_width() * 30, GetTextHeight() * 40);
    m_pOrgColumnNames->set_width_request(aSize.Width());
    m_pOrgColumnNames->set_height_request(aSize.Height());
    m_pNewColumnNames->set_width_request(aSize.Width());
    m_pNewColumnNames->set_height_request(aSize.Height());

    m_pColumn_RH->SetClickHdl(LINK(this,OWizColumnSelect,ButtonClickHdl));
    m_pColumn_LH->SetClickHdl(LINK(this,OWizColumnSelect,ButtonClickHdl));
    m_pColumns_RH->SetClickHdl(LINK(this,OWizColumnSelect,ButtonClickHdl));
    m_pColumns_LH->SetClickHdl(LINK(this,OWizColumnSelect,ButtonClickHdl));

    m_pOrgColumnNames->EnableMultiSelection(true);
    m_pNewColumnNames->EnableMultiSelection(true);

    m_pOrgColumnNames->SetDoubleClickHdl(LINK(this,OWizColumnSelect,ListDoubleClickHdl));
    m_pNewColumnNames->SetDoubleClickHdl(LINK(this,OWizColumnSelect,ListDoubleClickHdl));
}

OWizColumnSelect::~OWizColumnSelect()
{
    disposeOnce();
}

void OWizColumnSelect::dispose()
{
    while ( m_pNewColumnNames->GetEntryCount() )
    {
        void* pData = m_pNewColumnNames->GetEntryData(0);
        if ( pData )
            delete static_cast<OFieldDescription*>(pData);

        m_pNewColumnNames->RemoveEntry(0);
    }
    m_pNewColumnNames->Clear();
    m_pOrgColumnNames.clear();
    m_pColumn_RH.clear();
    m_pColumns_RH.clear();
    m_pColumn_LH.clear();
    m_pColumns_LH.clear();
    m_pNewColumnNames.clear();
    OWizardPage::dispose();
}

void OWizColumnSelect::Reset()
{
    // restore original state
    clearListBox(*m_pOrgColumnNames);
    clearListBox(*m_pNewColumnNames);
    m_pParent->m_mNameMapping.clear();

    // insert the source columns in the left listbox
    const ODatabaseExport::TColumnVector& rSrcColumns = m_pParent->getSrcVector();
    ODatabaseExport::TColumnVector::const_iterator aIter = rSrcColumns.begin();
    ODatabaseExport::TColumnVector::const_iterator aEnd = rSrcColumns.end();

    for(;aIter != aEnd;++aIter)
    {
        const sal_Int32 nPos = m_pOrgColumnNames->InsertEntry((*aIter)->first);
        m_pOrgColumnNames->SetEntryData(nPos,(*aIter)->second);
    }

    if(m_pOrgColumnNames->GetEntryCount())
        m_pOrgColumnNames->SelectEntryPos(0);

    m_bFirstTime = false;
}

void OWizColumnSelect::ActivatePage( )
{
    // if there are no dest columns reset the left side with the origibnal columns
    if(m_pParent->getDestColumns().empty())
        Reset();

    clearListBox(*m_pNewColumnNames);

    const ODatabaseExport::TColumnVector& rDestColumns = m_pParent->getDestVector();

    ODatabaseExport::TColumnVector::const_iterator aIter = rDestColumns.begin();
    ODatabaseExport::TColumnVector::const_iterator aEnd = rDestColumns.end();
    for(;aIter != aEnd;++aIter)
    {
        const sal_Int32 nPos = m_pNewColumnNames->InsertEntry((*aIter)->first);
        m_pNewColumnNames->SetEntryData(nPos,new OFieldDescription(*((*aIter)->second)));
        m_pOrgColumnNames->RemoveEntry((*aIter)->first);
    }
    m_pParent->GetOKButton().Enable(m_pNewColumnNames->GetEntryCount() != 0);
    m_pParent->EnableButton(OCopyTableWizard::WIZARD_NEXT,m_pNewColumnNames->GetEntryCount() && m_pParent->getOperation() != CopyTableOperation::AppendData);
    m_pColumns_RH->GrabFocus();
}

bool OWizColumnSelect::LeavePage()
{

    m_pParent->clearDestColumns();

    for(sal_Int32 i=0 ; i< m_pNewColumnNames->GetEntryCount();++i)
    {
        OFieldDescription* pField = static_cast<OFieldDescription*>(m_pNewColumnNames->GetEntryData(i));
        OSL_ENSURE(pField,"The field information can not be null!");
        m_pParent->insertColumn(i,pField);
    }

    clearListBox(*m_pNewColumnNames);

    if  (   m_pParent->GetPressedButton() == OCopyTableWizard::WIZARD_NEXT
        ||  m_pParent->GetPressedButton() == OCopyTableWizard::WIZARD_FINISH
        )
        return m_pParent->getDestColumns().size() != 0;
    else
        return true;
}

IMPL_LINK_TYPED( OWizColumnSelect, ButtonClickHdl, Button *, pButton, void )
{
    ListBox *pLeft = NULL;
    ListBox *pRight = NULL;
    bool bAll = false;

    if (pButton == m_pColumn_RH)
    {
        pLeft  = m_pOrgColumnNames;
        pRight = m_pNewColumnNames;
    }
    else if(pButton == m_pColumn_LH)
    {
        pLeft  = m_pNewColumnNames;
        pRight = m_pOrgColumnNames;
    }
    else if(pButton == m_pColumns_RH)
    {
        pLeft  = m_pOrgColumnNames;
        pRight = m_pNewColumnNames;
        bAll   = true;
    }
    else if(pButton == m_pColumns_LH)
    {
        pLeft  = m_pNewColumnNames;
        pRight = m_pOrgColumnNames;
        bAll   = true;
    }

    if (!pLeft || !pRight)
        return;

    Reference< XDatabaseMetaData > xMetaData( m_pParent->m_xDestConnection->getMetaData() );
    OUString sExtraChars = xMetaData->getExtraNameCharacters();
    sal_Int32 nMaxNameLen       = m_pParent->getMaxColumnNameLength();

    ::comphelper::UStringMixEqual aCase(xMetaData->supportsMixedCaseQuotedIdentifiers());
    ::std::vector< OUString> aRightColumns;
    fillColumns(pRight,aRightColumns);

    if(!bAll)
    {
        for(sal_Int32 i=0; i < pLeft->GetSelectEntryCount(); ++i)
            moveColumn(pRight,pLeft,aRightColumns,pLeft->GetSelectEntry(i),sExtraChars,nMaxNameLen,aCase);

        for(sal_Int32 j=pLeft->GetSelectEntryCount(); j ; --j)
            pLeft->RemoveEntry(pLeft->GetSelectEntry(j-1));
    }
    else
    {
        const sal_Int32 nEntries = pLeft->GetEntryCount();
        for(sal_Int32 i=0; i < nEntries; ++i)
            moveColumn(pRight,pLeft,aRightColumns,pLeft->GetEntry(i),sExtraChars,nMaxNameLen,aCase);
        for(sal_Int32 j=pLeft->GetEntryCount(); j ; )
            pLeft->RemoveEntry(--j);
    }

    enableButtons();

    if(m_pOrgColumnNames->GetEntryCount())
        m_pOrgColumnNames->SelectEntryPos(0);
}

IMPL_LINK_TYPED( OWizColumnSelect, ListDoubleClickHdl, ListBox&, rListBox, void )
{
    ListBox *pLeft,*pRight;
    if(&rListBox == m_pOrgColumnNames)
    {
        pLeft  = m_pOrgColumnNames;
        pRight = m_pNewColumnNames;
    }
    else
    {
        pRight = m_pOrgColumnNames;
        pLeft  = m_pNewColumnNames;
    }

    // If database is able to process PrimaryKeys, set PrimaryKey
    Reference< XDatabaseMetaData >  xMetaData( m_pParent->m_xDestConnection->getMetaData() );
    OUString sExtraChars = xMetaData->getExtraNameCharacters();
    sal_Int32 nMaxNameLen       = m_pParent->getMaxColumnNameLength();

    ::comphelper::UStringMixEqual aCase(xMetaData->supportsMixedCaseQuotedIdentifiers());
    ::std::vector< OUString> aRightColumns;
    fillColumns(pRight,aRightColumns);

    for(sal_Int32 i=0; i < pLeft->GetSelectEntryCount(); ++i)
        moveColumn(pRight,pLeft,aRightColumns,pLeft->GetSelectEntry(i),sExtraChars,nMaxNameLen,aCase);
    for(sal_Int32 j=pLeft->GetSelectEntryCount(); j ; )
        pLeft->RemoveEntry(pLeft->GetSelectEntry(--j));

    enableButtons();
}

void OWizColumnSelect::clearListBox(ListBox& _rListBox)
{
    while(_rListBox.GetEntryCount())
        _rListBox.RemoveEntry(0);
    _rListBox.Clear();
}

void OWizColumnSelect::fillColumns(ListBox* pRight,::std::vector< OUString> &_rRightColumns)
{
    const sal_Int32 nCount = pRight->GetEntryCount();
    _rRightColumns.reserve(nCount);
    for(sal_Int32 i=0; i < nCount; ++i)
        _rRightColumns.push_back(pRight->GetEntry(i));
}

void OWizColumnSelect::createNewColumn( ListBox* _pListbox,
                                        OFieldDescription* _pSrcField,
                                        ::std::vector< OUString>& _rRightColumns,
                                        const OUString&  _sColumnName,
                                        const OUString&  _sExtraChars,
                                        sal_Int32               _nMaxNameLen,
                                        const ::comphelper::UStringMixEqual& _aCase)
{
    OUString sConvertedName = m_pParent->convertColumnName(TMultiListBoxEntryFindFunctor(&_rRightColumns,_aCase),
                                                                _sColumnName,
                                                                _sExtraChars,
                                                                _nMaxNameLen);
    OFieldDescription* pNewField = new OFieldDescription(*_pSrcField);
    pNewField->SetName(sConvertedName);
    bool bNotConvert = true;
    pNewField->SetType(m_pParent->convertType(_pSrcField->getSpecialTypeInfo(),bNotConvert));
    if ( !m_pParent->supportsPrimaryKey() )
        pNewField->SetPrimaryKey(false);

    _pListbox->SetEntryData(_pListbox->InsertEntry(sConvertedName),pNewField);
    _rRightColumns.push_back(sConvertedName);

    if ( !bNotConvert )
        m_pParent->showColumnTypeNotSupported(sConvertedName);
}

void OWizColumnSelect::moveColumn(  ListBox* _pRight,
                                    ListBox* _pLeft,
                                    ::std::vector< OUString>& _rRightColumns,
                                    const OUString&  _sColumnName,
                                    const OUString&  _sExtraChars,
                                    sal_Int32               _nMaxNameLen,
                                    const ::comphelper::UStringMixEqual& _aCase)
{
    if(_pRight == m_pNewColumnNames)
    {
        // we copy the column into the new format for the dest
        OFieldDescription* pSrcField = static_cast<OFieldDescription*>(_pLeft->GetEntryData(_pLeft->GetEntryPos(OUString(_sColumnName))));
        createNewColumn(_pRight,pSrcField,_rRightColumns,_sColumnName,_sExtraChars,_nMaxNameLen,_aCase);
    }
    else
    {
        // find the new column in the dest name mapping to obtain the old column
        OCopyTableWizard::TNameMapping::iterator aIter = ::std::find_if(m_pParent->m_mNameMapping.begin(),m_pParent->m_mNameMapping.end(),
            [&_aCase, &_sColumnName] (const OCopyTableWizard::TNameMapping::value_type& nameMap) {
                return _aCase(nameMap.second, _sColumnName);
            });

        OSL_ENSURE(aIter != m_pParent->m_mNameMapping.end(),"Column must be defined");
        if ( aIter == m_pParent->m_mNameMapping.end() )
            return; // do nothing
        const ODatabaseExport::TColumns& rSrcColumns = m_pParent->getSourceColumns();
        ODatabaseExport::TColumns::const_iterator aSrcIter = rSrcColumns.find((*aIter).first);
        if ( aSrcIter != rSrcColumns.end() )
        {
            // we need also the old position of this column to insert it back on that position again
            const ODatabaseExport::TColumnVector& rSrcVector = m_pParent->getSrcVector();
            ODatabaseExport::TColumnVector::const_iterator aPos = ::std::find(rSrcVector.begin(), rSrcVector.end(), aSrcIter);
            OSL_ENSURE( aPos != rSrcVector.end(),"Invalid position for the iterator here!");
            ODatabaseExport::TColumnVector::size_type nPos = (aPos - rSrcVector.begin()) - adjustColumnPosition(_pLeft, _sColumnName, (aPos - rSrcVector.begin()), _aCase);

            _pRight->SetEntryData( _pRight->InsertEntry( (*aIter).first, sal::static_int_cast< sal_uInt16 >(nPos)),aSrcIter->second );
            _rRightColumns.push_back((*aIter).first);
            m_pParent->removeColumnNameFromNameMap(_sColumnName);
        }
    }
}

// Simply returning fields back to their original position is
// not enough. We need to take into account what fields have
// been removed earlier and adjust accordingly. Based on the
// algorithm employed in moveColumn().
sal_Int32 OWizColumnSelect::adjustColumnPosition( ListBox* _pLeft,
                                               const OUString&   _sColumnName,
                                               ODatabaseExport::TColumnVector::size_type nCurrentPos,
                                               const ::comphelper::UStringMixEqual& _aCase)
{
    sal_Int32 nAdjustedPos = 0;

    // if returning all entries to their original position,
    // then there is no need to adjust the positions.
    if (m_pColumns_LH->HasFocus())
        return nAdjustedPos;

    const sal_Int32 nCount = _pLeft->GetEntryCount();
    OUString sColumnString;
    for(sal_Int32 i=0; i < nCount; ++i)
    {
        sColumnString = _pLeft->GetEntry(i);
        if(_sColumnName != sColumnString)
        {
            // find the new column in the dest name mapping to obtain the old column
            OCopyTableWizard::TNameMapping::iterator aIter = ::std::find_if(m_pParent->m_mNameMapping.begin(),m_pParent->m_mNameMapping.end(),
                [&_aCase, &sColumnString] (const OCopyTableWizard::TNameMapping::value_type& nameMap) {
                    return _aCase(nameMap.second, sColumnString);
                });

            OSL_ENSURE(aIter != m_pParent->m_mNameMapping.end(),"Column must be defined");
            const ODatabaseExport::TColumns& rSrcColumns = m_pParent->getSourceColumns();
            ODatabaseExport::TColumns::const_iterator aSrcIter = rSrcColumns.find((*aIter).first);
            if ( aSrcIter != rSrcColumns.end() )
            {
                // we need also the old position of this column to insert it back on that position again
                const ODatabaseExport::TColumnVector& rSrcVector = m_pParent->getSrcVector();
                ODatabaseExport::TColumnVector::const_iterator aPos = ::std::find(rSrcVector.begin(), rSrcVector.end(), aSrcIter);
                ODatabaseExport::TColumnVector::size_type nPos = aPos - rSrcVector.begin();
                if( nPos < nCurrentPos)
                {
                    nAdjustedPos++;
                }
            }
        }
    }

    return nAdjustedPos;
}

void OWizColumnSelect::enableButtons()
{
    bool bEntries = m_pNewColumnNames->GetEntryCount() != 0;
    if(!bEntries)
        m_pParent->m_mNameMapping.clear();

    m_pParent->GetOKButton().Enable(bEntries);
    m_pParent->EnableButton(OCopyTableWizard::WIZARD_NEXT,bEntries && m_pParent->getOperation() != CopyTableOperation::AppendData);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
