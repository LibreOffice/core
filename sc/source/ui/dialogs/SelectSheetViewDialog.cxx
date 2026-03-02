/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <dialogs/SelectSheetViewDialog.hxx>
#include <sal/config.h>
#include <o3tl/safeint.hxx>

#include <viewdata.hxx>
#include <SheetViewManager.hxx>

namespace sc
{
class SheetViewListEntry
{
    OUString maName;
    sc::SheetViewID mnSheetViewID = sc::InvalidSheetViewID;

public:
    SheetViewListEntry(OUString const& rName, sc::SheetViewID nSheetViewID)
        : maName(rName)
        , mnSheetViewID(nSheetViewID)
    {
    }

    OUString const& getName() const { return maName; }
    sc::SheetViewID getSheetViewID() const { return mnSheetViewID; }
};

SelectSheetViewDialog::SelectSheetViewDialog(weld::Window* pParent, ScViewData& rViewData)
    : GenericDialogController(pParent, u"modules/scalc/ui/selectsheetviewdialog.ui"_ustr,
                              u"SelectSheetViewDialog"_ustr)
    , mrViewData(rViewData)
    , m_xEntryTree(m_xBuilder->weld_tree_view(u"entry-list"_ustr))
{
    m_xEntryTree->connect_changed(LINK(this, SelectSheetViewDialog, NameModifyHdl));

    m_xEntryTree->clear();

    m_xEntryTree->freeze();

    OUString sActiveID = u"-1"_ustr;
    m_xEntryTree->append(u"-1"_ustr, SheetViewManager::defaultViewName());

    if (auto pSheetManager = mrViewData.GetCurrentSheetViewManager())
    {
        for (auto const& rSheetView : pSheetManager->iterateValidSheetViews())
        {
            auto& aEntry = m_aEntries.emplace_back(rSheetView.GetName(), rSheetView.getID());
            OUString sID = OUString::number(m_aEntries.size() - 1);
            if (rSheetView.getID() == mrViewData.GetSheetViewID())
                sActiveID = sID;
            m_xEntryTree->append(sID, aEntry.getName());
        }
    }

    m_xEntryTree->thaw();

    m_xEntryTree->select_id(sActiveID);
}

SelectSheetViewDialog::~SelectSheetViewDialog() {}

IMPL_LINK_NOARG(SelectSheetViewDialog, NameModifyHdl, weld::TreeView&, void)
{
    m_nCurrentIndex = m_xEntryTree->get_selected_id().toInt32();
}

sc::SheetViewID SelectSheetViewDialog::getSelectedSheetViewID() const
{
    if (m_nCurrentIndex == sc::DefaultSheetViewID)
        return sc::DefaultSheetViewID;

    if (m_nCurrentIndex < 0 || o3tl::make_unsigned(m_nCurrentIndex) >= m_aEntries.size())
        return sc::InvalidSheetViewID;

    return m_aEntries[m_nCurrentIndex].getSheetViewID();
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
