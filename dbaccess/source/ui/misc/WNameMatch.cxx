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

#include <WNameMatch.hxx>
#include <osl/diagnose.h>
#include <FieldDescriptions.hxx>
#include <WCopyTable.hxx>
#include <core_resource.hxx>
#include <strings.hrc>
#include <bitmaps.hlst>
#include <com/sun/star/sdbc/DataType.hpp>

using namespace ::dbaui;

// OWizColumnSelect
OWizNameMatching::OWizNameMatching(weld::Container* pPage, OCopyTableWizard* pWizard)
    : OWizardPage(pPage, pWizard, u"dbaccess/ui/namematchingpage.ui"_ustr, u"NameMatching"_ustr)
    , m_xTABLE_LEFT(m_xBuilder->weld_label(u"leftlabel"_ustr))
    , m_xTABLE_RIGHT(m_xBuilder->weld_label(u"rightlabel"_ustr))
    , m_xCTRL_LEFT(m_xBuilder->weld_tree_view(u"left"_ustr))
    , m_xCTRL_RIGHT(m_xBuilder->weld_tree_view(u"right"_ustr))
    , m_xColumn_up(m_xBuilder->weld_button(u"up"_ustr))
    , m_xColumn_down(m_xBuilder->weld_button(u"down"_ustr))
    , m_xColumn_up_right(m_xBuilder->weld_button(u"up_right"_ustr))
    , m_xColumn_down_right(m_xBuilder->weld_button(u"down_right"_ustr))
    , m_xAll(m_xBuilder->weld_button(u"all"_ustr))
    , m_xNone(m_xBuilder->weld_button(u"none"_ustr))
{
    OUString aImgUp(BMP_UP);
    OUString aImgDown(BMP_DOWN);
    m_xColumn_up->set_from_icon_name(aImgUp);
    m_xColumn_down->set_from_icon_name(aImgDown);
    m_xColumn_up_right->set_from_icon_name(aImgUp);
    m_xColumn_down_right->set_from_icon_name(aImgDown);

    m_xColumn_up->connect_clicked(LINK(this,OWizNameMatching,ButtonClickHdl));
    m_xColumn_down->connect_clicked(LINK(this,OWizNameMatching,ButtonClickHdl));

    m_xColumn_up_right->connect_clicked(LINK(this,OWizNameMatching,RightButtonClickHdl));
    m_xColumn_down_right->connect_clicked(LINK(this,OWizNameMatching,RightButtonClickHdl));

    m_xAll->connect_clicked(LINK(this,OWizNameMatching,AllNoneClickHdl));
    m_xNone->connect_clicked(LINK(this,OWizNameMatching,AllNoneClickHdl));

    m_xCTRL_LEFT->enable_toggle_buttons(weld::ColumnToggleType::Check);

    m_xCTRL_LEFT->connect_changed(LINK(this,OWizNameMatching,TableListClickHdl));
    m_xCTRL_RIGHT->connect_changed(LINK(this,OWizNameMatching,TableListRightSelectHdl));

    m_sSourceText = m_xTABLE_LEFT->get_label() + "\n";
    m_sDestText   = m_xTABLE_RIGHT->get_label() + "\n";
}

OWizNameMatching::~OWizNameMatching()
{
}

void OWizNameMatching::Reset()
{
    m_bFirstTime = false;
}

void OWizNameMatching::Activate( )
{
    // set source table name
    OUString aName = m_sSourceText + m_pParent->m_sSourceName;

    m_xTABLE_LEFT->set_label(aName);

    // set dest table name
    aName = m_sDestText + m_pParent->m_sName;
    m_xTABLE_RIGHT->set_label(aName);

    FillListBox(*m_xCTRL_LEFT, m_pParent->getSrcVector(), true);
    FillListBox(*m_xCTRL_RIGHT, m_pParent->getDestVector(), false);

    m_xColumn_up->set_sensitive( m_xCTRL_LEFT->n_children() > 1 );
    m_xColumn_down->set_sensitive( m_xCTRL_LEFT->n_children() > 1 );

    m_xColumn_up_right->set_sensitive( m_xCTRL_RIGHT->n_children() > 1 );
    m_xColumn_down_right->set_sensitive( m_xCTRL_RIGHT->n_children() > 1 );

    m_pParent->EnableNextButton(false);
    m_xCTRL_LEFT->grab_focus();
    TableListClickHdl(*m_xCTRL_LEFT);
}

bool OWizNameMatching::LeavePage()
{

    const ODatabaseExport::TColumnVector& rSrcColumns = m_pParent->getSrcVector();

    m_pParent->m_vColumnPositions.clear();
    m_pParent->m_vColumnTypes.clear();
    m_pParent->m_vColumnPositions.resize( rSrcColumns.size(), ODatabaseExport::TPositions::value_type( COLUMN_POSITION_NOT_FOUND, COLUMN_POSITION_NOT_FOUND ) );
    m_pParent->m_vColumnTypes.resize( rSrcColumns.size(), COLUMN_POSITION_NOT_FOUND );

    std::unique_ptr<weld::TreeIter> xLeftEntry = m_xCTRL_LEFT->make_iterator();
    std::unique_ptr<weld::TreeIter> xRightEntry = m_xCTRL_RIGHT->make_iterator();

    sal_Int32 nParamPos = 0;
    bool bLeftEntry = m_xCTRL_LEFT->get_iter_first(*xLeftEntry);
    bool bRightEntry = m_xCTRL_RIGHT->get_iter_first(*xRightEntry);
    while (bLeftEntry && bRightEntry)
    {
        OFieldDescription* pSrcField = weld::fromId<OFieldDescription*>(m_xCTRL_LEFT->get_id(*xLeftEntry));
        OSL_ENSURE(pSrcField,"OWizNameMatching: OColumn can not be null!");

        sal_Int32 nPos = 0;
        for (auto const& column : rSrcColumns)
        {
            if (column->second == pSrcField)
                break;
            ++nPos;
        }

        if (m_xCTRL_LEFT->get_toggle(*xLeftEntry) == TRISTATE_TRUE)
        {
            OFieldDescription* pDestField = weld::fromId<OFieldDescription*>(m_xCTRL_RIGHT->get_id(*xRightEntry));
            OSL_ENSURE(pDestField,"OWizNameMatching: OColumn can not be null!");
            const ODatabaseExport::TColumnVector& rDestColumns          = m_pParent->getDestVector();
            sal_Int32 nPosDest = 1;
            bool bDestColumnFound = false;
            TOTypeInfoSP typeInfoSPFound;
            for (auto const& column : rDestColumns)
            {
                if (column->second == pDestField)
                {
                    bDestColumnFound = true;
                    typeInfoSPFound = column->second->getSpecialTypeInfo();
                    break;
                }
                ++nPosDest;
            }

            OSL_ENSURE((nPos) < static_cast<sal_Int32>(m_pParent->m_vColumnPositions.size()),"m_pParent->m_vColumnPositions: Illegal index for vector");
            m_pParent->m_vColumnPositions[nPos].first = ++nParamPos;
            m_pParent->m_vColumnPositions[nPos].second = nPosDest;

            TOTypeInfoSP pTypeInfo;

            assert(bDestColumnFound);
            if (bDestColumnFound)
            {
                bool bNotConvert = true;
                pTypeInfo = m_pParent->convertType(typeInfoSPFound, bNotConvert);
            }

            sal_Int32 nType = css::sdbc::DataType::VARCHAR;
            if ( pTypeInfo )
                nType = pTypeInfo->nType;
            m_pParent->m_vColumnTypes[nPos] = nType;
        }
        else
        {
            m_pParent->m_vColumnPositions[nPos].first = COLUMN_POSITION_NOT_FOUND;
            m_pParent->m_vColumnPositions[nPos].second = COLUMN_POSITION_NOT_FOUND;
        }

        bLeftEntry = m_xCTRL_LEFT->iter_next(*xLeftEntry);
        bRightEntry = m_xCTRL_RIGHT->iter_next(*xRightEntry);
    }

    return true;
}

OUString OWizNameMatching::GetTitle() const { return DBA_RES(STR_WIZ_NAME_MATCHING_TITLE); }

IMPL_LINK(OWizNameMatching, ButtonClickHdl, weld::Button&, rButton, void)
{
    int nPos = m_xCTRL_LEFT->get_selected_index();
    if (nPos == -1)
        return;

    int nOrigPos = nPos;
    if (&rButton == m_xColumn_up.get() && nPos)
        --nPos;
    else if (&rButton == m_xColumn_down.get() && nPos < m_xCTRL_LEFT->n_children() - 1)
        ++nPos;

    m_xCTRL_LEFT->swap(nOrigPos, nPos);

    m_xCTRL_LEFT->scroll_to_row(nPos);

    TableListClickHdl(*m_xCTRL_LEFT);
}

IMPL_LINK( OWizNameMatching, RightButtonClickHdl, weld::Button&, rButton, void )
{
    int nPos = m_xCTRL_RIGHT->get_selected_index();
    if (nPos == -1)
        return;

    int nOrigPos = nPos;
    if (&rButton == m_xColumn_up_right.get() && nPos)
        --nPos;
    else if (&rButton == m_xColumn_down_right.get() && nPos < m_xCTRL_RIGHT->n_children() - 1)
        ++nPos;

    m_xCTRL_RIGHT->swap(nOrigPos, nPos);

    m_xCTRL_RIGHT->scroll_to_row(nPos);

    TableListRightSelectHdl(*m_xCTRL_RIGHT);
}

namespace
{
    int GetFirstEntryInView(weld::TreeView& rTreeView)
    {
        int nFirstEntryInView = -1;

        rTreeView.visible_foreach([&nFirstEntryInView, &rTreeView](weld::TreeIter& rEntry){
            nFirstEntryInView = rTreeView.get_iter_index_in_parent(rEntry);
            // stop after first entry
            return true;
        });

        return nFirstEntryInView;
    }
}

IMPL_LINK_NOARG(OWizNameMatching, TableListClickHdl, weld::TreeView&, void)
{
    int nPos = m_xCTRL_LEFT->get_selected_index();
    if (nPos == -1)
        return;

    int nOldEntry = m_xCTRL_RIGHT->get_selected_index();
    if (nOldEntry != -1 && nPos != nOldEntry)
    {
        m_xCTRL_RIGHT->unselect(nOldEntry);
        if (nPos < m_xCTRL_RIGHT->n_children())
        {
            int nNewPos = GetFirstEntryInView(*m_xCTRL_LEFT);
            if ( nNewPos - nPos == 1 )
                --nNewPos;
            m_xCTRL_RIGHT->scroll_to_row(nNewPos);
            m_xCTRL_RIGHT->select(nPos);
        }
    }
    else if (nOldEntry == -1)
    {
        if (nPos < m_xCTRL_RIGHT->n_children())
            m_xCTRL_RIGHT->select(nPos);
    }
}

IMPL_LINK_NOARG( OWizNameMatching, TableListRightSelectHdl, weld::TreeView&, void )
{
    int nPos = m_xCTRL_RIGHT->get_selected_index();
    if (nPos == -1)
        return;

    OFieldDescription* pColumn = weld::fromId<OFieldDescription*>(m_xCTRL_RIGHT->get_id(nPos));
    if (pColumn->IsAutoIncrement())
    {
        m_xCTRL_RIGHT->unselect(nPos);
        return;
    }

    int nOldEntry = m_xCTRL_LEFT->get_selected_index();
    if (nOldEntry != -1 && nPos != nOldEntry)
    {
        m_xCTRL_LEFT->unselect(nOldEntry);
        if (nPos < m_xCTRL_LEFT->n_children())
        {
            int nNewPos = GetFirstEntryInView(*m_xCTRL_RIGHT);
            if ( nNewPos - nPos == 1 )
                nNewPos--;
            m_xCTRL_LEFT->scroll_to_row(nNewPos);
            m_xCTRL_LEFT->select(nPos);
        }
    }
    else if (nOldEntry == -1)
    {
        if (nPos < m_xCTRL_LEFT->n_children())
            m_xCTRL_LEFT->select(nPos);
    }
}

IMPL_LINK(OWizNameMatching, AllNoneClickHdl, weld::Button&, rButton, void)
{
    bool bAll = &rButton == m_xAll.get();
    m_xCTRL_LEFT->all_foreach([this, bAll](weld::TreeIter& rEntry){
        m_xCTRL_LEFT->set_toggle(rEntry, bAll ? TRISTATE_TRUE : TRISTATE_FALSE);
        return false;
    });
}

void OWizNameMatching::FillListBox(weld::TreeView& rTreeView, const ODatabaseExport::TColumnVector& rList, bool bCheckButtons)
{
    rTreeView.clear();

    int nRow(0);

    for (auto const& elem : rList)
    {
        rTreeView.append();
        if (bCheckButtons)
        {
            bool bChecked = !elem->second->IsAutoIncrement();
            rTreeView.set_toggle(nRow, bChecked ? TRISTATE_TRUE : TRISTATE_FALSE);
        }
        rTreeView.set_text(nRow, elem->first, 0);
        rTreeView.set_id(nRow, weld::toId(elem->second));
        ++nRow;
    }

    if (rTreeView.n_children())
        rTreeView.select(0);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
