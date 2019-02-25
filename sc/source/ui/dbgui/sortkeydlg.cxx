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
#include <vcl/svapp.hxx>

ScSortKeyItem::ScSortKeyItem(weld::Container* pParent)
    : m_xBuilder(Application::CreateBuilder(pParent, "modules/scalc/ui/sortkey.ui"))
    , m_xFrame(m_xBuilder->weld_frame("SortKeyFrame", true))
    , m_xLbSort(m_xBuilder->weld_combo_box("sortlb"))
    , m_xBtnUp(m_xBuilder->weld_radio_button("up"))
    , m_xBtnDown(m_xBuilder->weld_radio_button("down"))
{
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

    m_aSortKeyItems.push_back(std::unique_ptr<ScSortKeyItem>(pSortKeyItem));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
