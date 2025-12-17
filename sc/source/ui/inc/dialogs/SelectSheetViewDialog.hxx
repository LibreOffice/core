/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <vector>
#include <sal/types.h>
#include <vcl/weld/TreeView.hxx>
#include <vcl/weld/weld.hxx>
#include <SheetViewTypes.hxx>

class ScViewData;
class ScDocument;

namespace sc
{
class SheetViewListEntry;

/** Dialog to show all sheet views and allows to select one to be used */
class SelectSheetViewDialog : public weld::GenericDialogController
{
    ScViewData& mrViewData;
    std::unique_ptr<weld::TreeView> m_xEntryTree;
    std::vector<SheetViewListEntry> m_aEntries;
    sal_Int32 m_nCurrentIndex = -1;

    DECL_LINK(NameModifyHdl, weld::TreeView&, void);

public:
    SelectSheetViewDialog(weld::Window* pParent, ScViewData& rViewData);
    virtual ~SelectSheetViewDialog() override;

    sc::SheetViewID getSelectedSheetViewID() const;
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
