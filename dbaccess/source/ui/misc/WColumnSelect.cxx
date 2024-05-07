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

#include <WColumnSelect.hxx>
#include <strings.hrc>
#include <osl/diagnose.h>
#include <WCopyTable.hxx>
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <core_resource.hxx>
#include <com/sun/star/sdb/application/CopyTableOperation.hpp>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sdbc;
using namespace dbaui;

namespace CopyTableOperation = ::com::sun::star::sdb::application::CopyTableOperation;

OUString OWizColumnSelect::GetTitle() const { return DBA_RES(STR_WIZ_COLUMN_SELECT_TITLE); }

OWizardPage::OWizardPage(weld::Container* pPage, OCopyTableWizard* pWizard, const OUString& rUIXMLDescription, const OUString& rID)
    : ::vcl::OWizardPage(pPage, pWizard, rUIXMLDescription, rID)
    , m_pParent(pWizard)
    , m_bFirstTime(true)
{
}

OWizardPage::~OWizardPage()
{
}

// OWizColumnSelect
OWizColumnSelect::OWizColumnSelect(weld::Container* pPage, OCopyTableWizard* pWizard)
    : OWizardPage(pPage, pWizard, u"dbaccess/ui/applycolpage.ui"_ustr, u"ApplyColPage"_ustr)
    , m_xOrgColumnNames(m_xBuilder->weld_tree_view(u"from"_ustr))
    , m_xColumn_RH(m_xBuilder->weld_button(u"colrh"_ustr))
    , m_xColumns_RH(m_xBuilder->weld_button(u"colsrh"_ustr))
    , m_xColumn_LH(m_xBuilder->weld_button(u"collh"_ustr))
    , m_xColumns_LH(m_xBuilder->weld_button(u"colslh"_ustr))
    , m_xNewColumnNames(m_xBuilder->weld_tree_view(u"to"_ustr))
{
    m_xColumn_RH->connect_clicked(LINK(this,OWizColumnSelect,ButtonClickHdl));
    m_xColumn_LH->connect_clicked(LINK(this,OWizColumnSelect,ButtonClickHdl));
    m_xColumns_RH->connect_clicked(LINK(this,OWizColumnSelect,ButtonClickHdl));
    m_xColumns_LH->connect_clicked(LINK(this,OWizColumnSelect,ButtonClickHdl));

    m_xOrgColumnNames->set_selection_mode(SelectionMode::Multiple);
    m_xNewColumnNames->set_selection_mode(SelectionMode::Multiple);

    m_xOrgColumnNames->connect_row_activated(LINK(this,OWizColumnSelect,ListDoubleClickHdl));
    m_xNewColumnNames->connect_row_activated(LINK(this,OWizColumnSelect,ListDoubleClickHdl));
}

OWizColumnSelect::~OWizColumnSelect()
{
    while (m_xNewColumnNames->n_children())
    {
        delete weld::fromId<OFieldDescription*>(m_xNewColumnNames->get_id(0));
        m_xNewColumnNames->remove(0);
    }
}

void OWizColumnSelect::Reset()
{
    // restore original state
    clearListBox(*m_xOrgColumnNames);
    clearListBox(*m_xNewColumnNames);
    m_pParent->m_mNameMapping.clear();

    // insert the source columns in the left listbox
    const ODatabaseExport::TColumnVector& rSrcColumns = m_pParent->getSrcVector();

    for (auto const& column : rSrcColumns)
    {
        OUString sId(weld::toId(column->second));
        m_xOrgColumnNames->append(sId, column->first);
    }

    if (m_xOrgColumnNames->n_children())
        m_xOrgColumnNames->select(0);

    m_bFirstTime = false;
}

void OWizColumnSelect::Activate( )
{
    // if there are no dest columns reset the left side with the original columns
    if(m_pParent->getDestColumns().empty())
        Reset();

    clearListBox(*m_xNewColumnNames);

    const ODatabaseExport::TColumnVector& rDestColumns = m_pParent->getDestVector();

    // tdf#113923, the added columns must exist in the table
    // in the case where:
    // 1: we enabled the creation of a primary key
    // 2: we come back here from the "Back" button of the next page,
    // we want to avoid to list the new column generated in the next page
    const ODatabaseExport::TColumns& rSrcColumns = m_pParent->getSourceColumns();

    for (auto const& column : rDestColumns)
    {
        if (rSrcColumns.find(column->first) != rSrcColumns.end())
        {
            OUString sId(weld::toId(new OFieldDescription(*(column->second))));
            m_xNewColumnNames->append(sId, column->first);
            int nRemove = m_xOrgColumnNames->find_text(column->first);
            if (nRemove != -1)
                m_xOrgColumnNames->remove(nRemove);
        }
    }
    m_pParent->GetOKButton().set_sensitive(m_xNewColumnNames->n_children() != 0);
    m_pParent->EnableNextButton(m_xNewColumnNames->n_children() && m_pParent->getOperation() != CopyTableOperation::AppendData);
    m_xColumns_RH->grab_focus();
}

bool OWizColumnSelect::LeavePage()
{

    m_pParent->clearDestColumns();

    for(sal_Int32 i=0 ; i< m_xNewColumnNames->n_children();++i)
    {
        OFieldDescription* pField = weld::fromId<OFieldDescription*>(m_xNewColumnNames->get_id(i));
        OSL_ENSURE(pField,"The field information can not be null!");
        m_pParent->insertColumn(i,pField);
    }

    clearListBox(*m_xNewColumnNames);

    if  (   m_pParent->GetPressedButton() == OCopyTableWizard::WIZARD_NEXT
        ||  m_pParent->GetPressedButton() == OCopyTableWizard::WIZARD_FINISH
        )
        return !m_pParent->getDestColumns().empty();
    else
        return true;
}

IMPL_LINK(OWizColumnSelect, ButtonClickHdl, weld::Button&, rButton, void)
{
    weld::TreeView *pLeft = nullptr;
    weld::TreeView *pRight = nullptr;
    bool bAll = false;

    if (&rButton == m_xColumn_RH.get())
    {
        pLeft  = m_xOrgColumnNames.get();
        pRight = m_xNewColumnNames.get();
    }
    else if (&rButton == m_xColumn_LH.get())
    {
        pLeft  = m_xNewColumnNames.get();
        pRight = m_xOrgColumnNames.get();
    }
    else if (&rButton == m_xColumns_RH.get())
    {
        pLeft  = m_xOrgColumnNames.get();
        pRight = m_xNewColumnNames.get();
        bAll   = true;
    }
    else if (&rButton == m_xColumns_LH.get())
    {
        pLeft  = m_xNewColumnNames.get();
        pRight = m_xOrgColumnNames.get();
        bAll   = true;
    }

    if (!pLeft || !pRight)
        return;

    Reference< XDatabaseMetaData > xMetaData( m_pParent->m_xDestConnection->getMetaData() );
    OUString sExtraChars = xMetaData->getExtraNameCharacters();
    sal_Int32 nMaxNameLen       = m_pParent->getMaxColumnNameLength();

    ::comphelper::UStringMixEqual aCase(xMetaData->supportsMixedCaseQuotedIdentifiers());
    std::vector< OUString> aRightColumns;
    fillColumns(pRight,aRightColumns);

    if(!bAll)
    {
        auto aRows = pLeft->get_selected_rows();
        std::sort(aRows.begin(), aRows.end());

        for (auto it = aRows.begin(); it != aRows.end(); ++it)
            moveColumn(pRight,pLeft,aRightColumns,pLeft->get_text(*it),sExtraChars,nMaxNameLen,aCase);

        for (auto it = aRows.rbegin(); it != aRows.rend(); ++it)
            pLeft->remove(*it);
    }
    else
    {
        const sal_Int32 nEntries = pLeft->n_children();
        for(sal_Int32 i=0; i < nEntries; ++i)
            moveColumn(pRight,pLeft,aRightColumns,pLeft->get_text(i),sExtraChars,nMaxNameLen,aCase);
        for(sal_Int32 j=pLeft->n_children(); j ; )
            pLeft->remove(--j);
    }

    enableButtons();

    if (m_xOrgColumnNames->n_children())
        m_xOrgColumnNames->select(0);
}

IMPL_LINK( OWizColumnSelect, ListDoubleClickHdl, weld::TreeView&, rListBox, bool )
{
    weld::TreeView *pLeft,*pRight;
    if (&rListBox == m_xOrgColumnNames.get())
    {
        pLeft  = m_xOrgColumnNames.get();
        pRight = m_xNewColumnNames.get();
    }
    else
    {
        pRight = m_xOrgColumnNames.get();
        pLeft  = m_xNewColumnNames.get();
    }

    // If database is able to process PrimaryKeys, set PrimaryKey
    Reference< XDatabaseMetaData >  xMetaData( m_pParent->m_xDestConnection->getMetaData() );
    OUString sExtraChars = xMetaData->getExtraNameCharacters();
    sal_Int32 nMaxNameLen       = m_pParent->getMaxColumnNameLength();

    ::comphelper::UStringMixEqual aCase(xMetaData->supportsMixedCaseQuotedIdentifiers());
    std::vector< OUString> aRightColumns;
    fillColumns(pRight,aRightColumns);

    auto aRows = pLeft->get_selected_rows();
    std::sort(aRows.begin(), aRows.end());

    for (auto it = aRows.begin(); it != aRows.end(); ++it)
        moveColumn(pRight,pLeft,aRightColumns,pLeft->get_text(*it),sExtraChars,nMaxNameLen,aCase);

    for (auto it = aRows.rbegin(); it != aRows.rend(); ++it)
        pLeft->remove(*it);

    enableButtons();

    return true;
}

void OWizColumnSelect::clearListBox(weld::TreeView& rListBox)
{
    rListBox.clear();
}

void OWizColumnSelect::fillColumns(weld::TreeView const * pRight,std::vector< OUString> &_rRightColumns)
{
    const sal_Int32 nCount = pRight->n_children();
    _rRightColumns.reserve(nCount);
    for (sal_Int32 i=0; i < nCount; ++i)
        _rRightColumns.push_back(pRight->get_text(i));
}

void OWizColumnSelect::createNewColumn( weld::TreeView* _pListbox,
                                        OFieldDescription const * _pSrcField,
                                        std::vector< OUString>& _rRightColumns,
                                        const OUString&  _sColumnName,
                                        std::u16string_view  _sExtraChars,
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

    _pListbox->append(weld::toId(pNewField), sConvertedName);
    _rRightColumns.push_back(sConvertedName);

    if ( !bNotConvert )
        m_pParent->showColumnTypeNotSupported(sConvertedName);
}

void OWizColumnSelect::moveColumn(  weld::TreeView* _pRight,
                                    weld::TreeView const * _pLeft,
                                    std::vector< OUString>& _rRightColumns,
                                    const OUString&  _sColumnName,
                                    std::u16string_view  _sExtraChars,
                                    sal_Int32               _nMaxNameLen,
                                    const ::comphelper::UStringMixEqual& _aCase)
{
    if(_pRight == m_xNewColumnNames.get())
    {
        // we copy the column into the new format for the dest
        OFieldDescription* pSrcField = weld::fromId<OFieldDescription*>(_pLeft->get_id(_pLeft->find_text(_sColumnName)));
        createNewColumn(_pRight,pSrcField,_rRightColumns,_sColumnName,_sExtraChars,_nMaxNameLen,_aCase);
    }
    else
    {
        // find the new column in the dest name mapping to obtain the old column
        OCopyTableWizard::TNameMapping::const_iterator aIter = std::find_if(m_pParent->m_mNameMapping.begin(),m_pParent->m_mNameMapping.end(),
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
            ODatabaseExport::TColumnVector::const_iterator aPos = std::find(rSrcVector.begin(), rSrcVector.end(), aSrcIter);
            OSL_ENSURE( aPos != rSrcVector.end(),"Invalid position for the iterator here!");
            ODatabaseExport::TColumnVector::size_type nPos = (aPos - rSrcVector.begin()) - adjustColumnPosition(_pLeft, _sColumnName, (aPos - rSrcVector.begin()), _aCase);

            OUString sId(weld::toId(aSrcIter->second));
            const OUString& rStr = (*aIter).first;
            _pRight->insert(nullptr, nPos, &rStr, &sId, nullptr, nullptr, false, nullptr);
            _rRightColumns.push_back(rStr);
            m_pParent->removeColumnNameFromNameMap(_sColumnName);
        }
    }
}

// Simply returning fields back to their original position is
// not enough. We need to take into account what fields have
// been removed earlier and adjust accordingly. Based on the
// algorithm employed in moveColumn().
sal_Int32 OWizColumnSelect::adjustColumnPosition(weld::TreeView const * _pLeft,
                                                 std::u16string_view  _sColumnName,
                                                 ODatabaseExport::TColumnVector::size_type nCurrentPos,
                                                 const ::comphelper::UStringMixEqual& _aCase)
{
    sal_Int32 nAdjustedPos = 0;

    // if returning all entries to their original position,
    // then there is no need to adjust the positions.
    if (m_xColumns_LH->has_focus())
        return nAdjustedPos;

    const sal_Int32 nCount = _pLeft->n_children();
    OUString sColumnString;
    for(sal_Int32 i=0; i < nCount; ++i)
    {
        sColumnString = _pLeft->get_text(i);
        if(_sColumnName != sColumnString)
        {
            // find the new column in the dest name mapping to obtain the old column
            OCopyTableWizard::TNameMapping::const_iterator aIter = std::find_if(m_pParent->m_mNameMapping.begin(),m_pParent->m_mNameMapping.end(),
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
                ODatabaseExport::TColumnVector::const_iterator aPos = std::find(rSrcVector.begin(), rSrcVector.end(), aSrcIter);
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
    bool bEntries = m_xNewColumnNames->n_children() != 0;
    if (!bEntries)
        m_pParent->m_mNameMapping.clear();

    m_pParent->GetOKButton().set_sensitive(bEntries);
    m_pParent->EnableNextButton(bEntries && m_pParent->getOperation() != CopyTableOperation::AppendData);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
