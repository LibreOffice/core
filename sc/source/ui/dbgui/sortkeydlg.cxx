/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <memory>
#include <sortkeydlg.hxx>
#include <comphelper/lok.hxx>
#include <vcl/svapp.hxx>

#include <scresid.hxx>
#include <strings.hrc>

ScSortKeyItem::ScSortKeyItem(weld::Container* pParent)
    : m_xBuilder(Application::CreateBuilder(pParent, u"modules/scalc/ui/sortkey.ui"_ustr))
    , m_xFrame(m_xBuilder->weld_frame(u"SortKeyFrame"_ustr))
    , m_xLbSort(m_xBuilder->weld_combo_box(u"sortlb"_ustr))
    , m_xBtnUp(m_xBuilder->weld_radio_button(u"up"_ustr))
    , m_xBtnDown(m_xBuilder->weld_radio_button(u"down"_ustr))
    , m_xLabel(m_xBuilder->weld_label(u"lbColRow"_ustr))
    , m_pParent(pParent)
{
    // tdf#136155 let the other elements in the dialog determine the width of the
    // combobox
    m_xLbSort->set_size_request(m_xLbSort->get_approximate_digit_width() * 12, -1);
    // keep the UI static when switching the labels
    const sal_Int32 nChars = std::max( ScResId(SCSTR_COLUMN).getLength(), ScResId(SCSTR_ROW).getLength() ) + 2; // +2 to avoid cut-off labels on kf5/gen
    m_xLabel->set_size_request( m_xLabel->get_approximate_digit_width() * nChars, -1);
}

ScSortKeyItem::~ScSortKeyItem()
{
    m_pParent->move(m_xFrame.get(), nullptr);
}

void ScSortKeyItem::DisableField()
{
    m_xFrame->set_sensitive(false);
}

void ScSortKeyItem::EnableField()
{
    m_xFrame->set_sensitive(true);
}

ScSortKeyWindow::ScSortKeyWindow(weld::Container* pBox)
    : m_pBox(pBox)
{
}

ScSortKeyWindow::~ScSortKeyWindow()
{
}

void ScSortKeyWindow::AddSortKey( sal_uInt16 nItemNumber )
{
    ScSortKeyItem* pSortKeyItem = new ScSortKeyItem(m_pBox);

    // Set Sort key number
    OUString aLine = pSortKeyItem->m_xFrame->get_label() +
                     OUString::number( nItemNumber );
    pSortKeyItem->m_xFrame->set_label(aLine);

    // for ui-testing. Distinguish the sort keys
    if (!comphelper::LibreOfficeKit::isActive())
    {
        if ( m_aSortKeyItems.size() > 0 )
        {
            pSortKeyItem->m_xLbSort->set_buildable_name(
                pSortKeyItem->m_xLbSort->get_buildable_name() + OUString::number(m_aSortKeyItems.size() + 1));
        }
    }

    m_aSortKeyItems.push_back(std::unique_ptr<ScSortKeyItem>(pSortKeyItem));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
