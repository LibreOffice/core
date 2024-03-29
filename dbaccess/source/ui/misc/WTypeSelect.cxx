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

#include <WTypeSelect.hxx>
#include <bitmaps.hlst>
#include <comphelper/diagnose_ex.hxx>
#include <osl/diagnose.h>
#include <FieldDescriptions.hxx>
#include <WCopyTable.hxx>
#include <strings.hrc>
#include <tools/stream.hxx>
#include <vcl/commandevent.hxx>
#include <vcl/svapp.hxx>
#include <UITools.hxx>
#include <core_resource.hxx>
#include <FieldControls.hxx>

using namespace ::dbaui;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::sdbc;

// OWizTypeSelectControl
OWizTypeSelectControl::OWizTypeSelectControl(weld::Container* pPage, OWizTypeSelect* pParentTabPage)
    : OFieldDescControl(pPage, nullptr)
    , m_pParentTabPage(pParentTabPage)
{
}

OWizTypeSelectControl::~OWizTypeSelectControl()
{
}

void OWizTypeSelectControl::ActivateAggregate( EControlType eType )
{
    switch(eType )
    {
        case tpFormat:
        case tpDefault:
        case tpAutoIncrement:
        case tpAutoIncrementValue:
            break;
        default:
            OFieldDescControl::ActivateAggregate( eType );
    }
}

void OWizTypeSelectControl::DeactivateAggregate( EControlType eType )
{
    switch(eType )
    {
        case tpFormat:
        case tpDefault:
        case tpAutoIncrement:
        case tpAutoIncrementValue:
            break;
        default:
            OFieldDescControl::DeactivateAggregate( eType );
    }
}

void OWizTypeSelectControl::CellModified(sal_Int32 nRow, sal_uInt16 nColId )
{
    OSL_ENSURE(nRow == -1,"nRow must be -1!");

    weld::TreeView* pListBox = m_pParentTabPage->m_xColumnNames->GetWidget();

    OFieldDescription* pCurFieldDescr = getCurrentFieldDescData();

    const sal_Int32 nPos = pListBox->find_text(pCurFieldDescr->GetName());
    pCurFieldDescr = weld::fromId<OFieldDescription*>(pListBox->get_id(nPos));
    OSL_ENSURE( pCurFieldDescr, "OWizTypeSelectControl::CellModified: Columnname/type not found in the listbox!" );
    if ( !pCurFieldDescr )
        return;
    setCurrentFieldDescData( pCurFieldDescr );

    OUString sName = pCurFieldDescr->GetName();
    OUString sNewName;
    const OPropColumnEditCtrl* pColumnName = getColumnCtrl();
    if ( pColumnName )
        sNewName = pColumnName->get_text();

    switch(nColId)
    {
        case FIELD_PROPERTY_COLUMNNAME:
            {
                OCopyTableWizard* pWiz = m_pParentTabPage->m_pParent;
                // first we have to check if this name already exists
                bool bDoubleName = false;
                bool bCase = true;
                if ( getMetaData().is() && !getMetaData()->supportsMixedCaseQuotedIdentifiers() )
                {
                    bCase = false;
                    const sal_Int32 nCount = pListBox->n_children();
                    for (sal_Int32 i=0 ; !bDoubleName && i < nCount ; ++i)
                    {
                        OUString sEntry(pListBox->get_text(i));
                        bDoubleName = sNewName.equalsIgnoreAsciiCase(sEntry);
                    }
                    if ( !bDoubleName && pWiz->shouldCreatePrimaryKey() )
                        bDoubleName = sNewName.equalsIgnoreAsciiCase(pWiz->getPrimaryKeyName());

                }
                else
                    bDoubleName =  ((pListBox->find_text(sNewName) != -1)
                                    || ( pWiz->shouldCreatePrimaryKey()
                                        &&  pWiz->getPrimaryKeyName() == sNewName) );

                if ( bDoubleName )
                {
                    OUString strMessage = DBA_RES(STR_TABLEDESIGN_DUPLICATE_NAME);
                    strMessage = strMessage.replaceFirst("$column$", sNewName);
                    pWiz->showError(strMessage);
                    pCurFieldDescr->SetName(sName);
                    DisplayData(pCurFieldDescr);
                    m_pParentTabPage->setDuplicateName(true);
                    return;
                }

                OUString sOldName = pCurFieldDescr->GetName();
                pCurFieldDescr->SetName(sNewName);
                m_pParentTabPage->setDuplicateName(false);

                // now we change the name

                ::comphelper::UStringMixEqual aCase(bCase);
                for (auto & elem : pWiz->m_mNameMapping)
                {
                    if ( aCase(elem.second,sName) )
                    {
                        elem.second = sNewName;
                        break;
                    }
                }

                pListBox->remove(nPos);
                pListBox->insert_text(nPos, pCurFieldDescr->GetName());
                pListBox->set_id(nPos, weld::toId(pCurFieldDescr));

                pWiz->replaceColumn(nPos,pCurFieldDescr,sOldName);
            }
            break;
    }
    saveCurrentFieldDescData();
}

css::lang::Locale  OWizTypeSelectControl::GetLocale() const
{
    return m_pParentTabPage->m_pParent->GetLocale();
}

Reference< XNumberFormatter > OWizTypeSelectControl::GetFormatter() const
{
    return m_pParentTabPage->m_pParent->GetFormatter();
}

TOTypeInfoSP    OWizTypeSelectControl::getTypeInfo(sal_Int32 _nPos)
{
    return m_pParentTabPage->m_pParent->getDestTypeInfo(_nPos);
}

const OTypeInfoMap* OWizTypeSelectControl::getTypeInfo() const
{
    return &m_pParentTabPage->m_pParent->getDestTypeInfo();
}

css::uno::Reference< css::sdbc::XDatabaseMetaData> OWizTypeSelectControl::getMetaData()
{
    return m_pParentTabPage->m_pParent->m_xDestConnection->getMetaData();
}

css::uno::Reference< css::sdbc::XConnection> OWizTypeSelectControl::getConnection()
{
    return m_pParentTabPage->m_pParent->m_xDestConnection;
}

bool OWizTypeSelectControl::isAutoIncrementValueEnabled() const
{
    return m_pParentTabPage->m_bAutoIncrementEnabled;
}

OUString OWizTypeSelectControl::getAutoIncrementValue() const
{
    return m_pParentTabPage->m_sAutoIncrementValue;
}

OWizTypeSelect::OWizTypeSelect(weld::Container* pPage, OCopyTableWizard* pWizard, SvStream* pStream)
    : OWizardPage(pPage, pWizard, "dbaccess/ui/typeselectpage.ui", "TypeSelect")
    , m_xColumnNames(new OWizTypeSelectList(m_xBuilder->weld_tree_view("columnnames")))
    , m_xControlContainer(m_xBuilder->weld_container("control_container"))
    , m_xTypeControl(new OWizTypeSelectControl(m_xControlContainer.get(), this))
    , m_xAutoType(m_xBuilder->weld_label("autotype"))
    , m_xAutoFt(m_xBuilder->weld_label("autolabel"))
    , m_xAutoEt(m_xBuilder->weld_spin_button("auto"))
    , m_xAutoPb(m_xBuilder->weld_button("autobutton"))
    , m_pParserStream(pStream)
    , m_nDisplayRow(0)
    , m_bAutoIncrementEnabled(false)
    , m_bDuplicateName(false)
{
    m_xColumnNames->connect_changed(LINK(this,OWizTypeSelect,ColumnSelectHdl));

    m_xTypeControl->Init();

    m_xAutoEt->set_text("10");
    m_xAutoEt->set_digits(0);
    m_xAutoPb->connect_clicked(LINK(this,OWizTypeSelect,ButtonClickHdl));
    m_xColumnNames->set_selection_mode(SelectionMode::Multiple);

    try
    {
        m_xColumnNames->SetPKey( m_pParent->supportsPrimaryKey() );
        ::dbaui::fillAutoIncrementValue( m_pParent->m_xDestConnection, m_bAutoIncrementEnabled, m_sAutoIncrementValue );
    }
    catch(const Exception&)
    {
        DBG_UNHANDLED_EXCEPTION("dbaccess");
    }
}

OWizTypeSelect::~OWizTypeSelect()
{
}

OUString OWizTypeSelect::GetTitle() const
{
    return DBA_RES(STR_WIZ_TYPE_SELECT_TITLE);
}

IMPL_LINK_NOARG(OWizTypeSelect, ColumnSelectHdl, weld::TreeView&, void)
{
    OFieldDescription* pField = weld::fromId<OFieldDescription*>(m_xColumnNames->get_selected_id());
    if (pField)
        m_xTypeControl->DisplayData(pField);

    m_xTypeControl->Enable(m_xColumnNames->count_selected_rows() == 1);
}

void OWizTypeSelect::Reset()
{
    // restore original state
    m_xColumnNames->clear();
    sal_Int32 nCount(0), nBreakPos;
    m_pParent->CheckColumns(nBreakPos);

    const ODatabaseExport::TColumnVector& rDestColumns = m_pParent->getDestVector();
    for (auto const& column : rDestColumns)
    {
        OUString sId(weld::toId(column->second));
        m_xColumnNames->append(sId, column->first);
        if (column->second->IsPrimaryKey())
            m_xColumnNames->set_image(nCount, BMP_PRIMARY_KEY);
        ++nCount;
    }
    m_bFirstTime = false;
}

void OWizTypeSelect::Activate( )
{
    bool bOldFirstTime = m_bFirstTime;
    Reset();
    m_bFirstTime = bOldFirstTime;

    m_xColumnNames->select(m_nDisplayRow);
    m_nDisplayRow = 0;
    ColumnSelectHdl(*m_xColumnNames->GetWidget());
}

bool OWizTypeSelect::LeavePage()
{
    bool bDuplicateName = false;
    OFieldDescription* pField = weld::fromId<OFieldDescription*>(m_xColumnNames->get_selected_id());
    if ( pField )
    {
        m_xTypeControl->SaveData(pField);
        bDuplicateName = m_bDuplicateName;
    }
    return !bDuplicateName;
}

void OWizTypeSelect::EnableAuto(bool bEnable)
{
    m_xAutoFt->set_visible(bEnable);
    m_xAutoEt->set_visible(bEnable);
    m_xAutoPb->set_visible(bEnable);
    m_xAutoType->set_visible(bEnable);
}

IMPL_LINK_NOARG(OWizTypeSelect, ButtonClickHdl, weld::Button&, void)
{
    sal_Int32 nBreakPos;
    m_pParent->CheckColumns(nBreakPos);

    // fill column list
    sal_uInt32 nRows = m_xAutoEt->get_text().toInt32();
    if(m_pParserStream)
    {
        sal_uInt64 const nTell = m_pParserStream->Tell(); // might change seek position of stream

        createReaderAndCallParser(nRows);
        m_pParserStream->Seek(nTell);
    }

    Activate();
}

OWizTypeSelectList::OWizTypeSelectList(std::unique_ptr<weld::TreeView> xControl)
    : m_xControl(std::move(xControl))
    , m_bPKey(false)
{
    m_xControl->connect_popup_menu(LINK(this, OWizTypeSelectList, CommandHdl));
}

bool OWizTypeSelectList::IsPrimaryKeyAllowed() const
{
    auto aRows = m_xControl->get_selected_rows();
    std::sort(aRows.begin(), aRows.end());

    const sal_Int32 nCount = aRows.size();

    for( sal_Int32 j = 0; m_bPKey && j < nCount; ++j )
    {
        OFieldDescription* pField = weld::fromId<OFieldDescription*>(m_xControl->get_id(aRows[j]));
        if(!pField || pField->getTypeInfo()->nSearchType == ColumnSearch::NONE)
            return false;
    }
    return true;
}

void OWizTypeSelectList::setPrimaryKey(OFieldDescription* _pFieldDescr, sal_uInt16 _nPos, bool _bSet)
{
    _pFieldDescr->SetPrimaryKey(_bSet);
    if( _bSet )
    {
        m_xControl->set_image(_nPos, BMP_PRIMARY_KEY);
    }
    else if( _pFieldDescr->getTypeInfo()->bNullable )
    {
        _pFieldDescr->SetControlDefault(Any());
        m_xControl->set_image(_nPos, OUString());
    }
}

IMPL_LINK(OWizTypeSelectList, CommandHdl, const CommandEvent&, rCEvt, bool)
{
    if (rCEvt.GetCommand() != CommandEventId::ContextMenu)
        return false;
    if (!IsPrimaryKeyAllowed())
        return false;

    std::unique_ptr<weld::Builder> xBuilder(Application::CreateBuilder(m_xControl.get(), "dbaccess/ui/keymenu.ui"));
    auto xContextMenu = xBuilder->weld_menu("menu");
    // Should primary key checkbox be checked?
    const sal_Int32 nCount = m_xControl->n_children();
    bool bCheckOk = false;
    for(sal_Int32 j = 0 ; j < nCount ; ++j)
    {
        OFieldDescription* pFieldDescr = weld::fromId<OFieldDescription*>(m_xControl->get_id(j));
        // if at least one of the fields is selected but not in the primary key,
        // or is in the primary key but not selected, then don't check the
        // primary key checkbox.
        if( pFieldDescr && pFieldDescr->IsPrimaryKey() != m_xControl->is_selected(j) )
        {
            bCheckOk = false;
            break;
        }
        if (!bCheckOk && m_xControl->is_selected(j))
            bCheckOk = true;
    }

    if (bCheckOk)
        xContextMenu->set_active("primarykey", true);

    OUString sCommand(xContextMenu->popup_at_rect(m_xControl.get(), tools::Rectangle(rCEvt.GetMousePosPixel(), Size(1,1))));
    if (sCommand != "primarykey")
        return true;

    for (sal_Int32 j = 0 ; j < nCount; ++j)
    {
        OFieldDescription* pFieldDescr = weld::fromId<OFieldDescription*>(m_xControl->get_id(j));
        if (pFieldDescr)
        {
            if(!bCheckOk && m_xControl->is_selected(j))
            {
                setPrimaryKey(pFieldDescr,j,true);
            }
            else
            {
                setPrimaryKey(pFieldDescr,j);
            }
        }
    }
    m_aChangeHdl.Call(*m_xControl);

    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
