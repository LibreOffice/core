/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <vcl/InterimItemWindow.hxx>
#include <dbdata.hxx>

/** Box / container for the combobox for showing / selecting the sheet view */
class TableStylesBox final : public InterimItemWindow
{
public:
    explicit TableStylesBox(vcl::Window* pParent);
    virtual void dispose() override;
    virtual ~TableStylesBox() override;

    void Update(const ScDatabaseSettingItem* pItem);

    weld::ComboBox* getWidget() { return m_xCmbStyle.get(); }

private:
    std::unique_ptr<weld::CheckButton> m_xChkHeaderRow;
    std::unique_ptr<weld::CheckButton> m_xChkTotalRow;
    std::unique_ptr<weld::CheckButton> m_xChkFilterButtons;
    std::unique_ptr<weld::CheckButton> m_xChkBandedRows;
    std::unique_ptr<weld::CheckButton> m_xChkBandedColumns;
    std::unique_ptr<weld::CheckButton> m_xChkFirstColumn;
    std::unique_ptr<weld::CheckButton> m_xChkLastColumn;
    std::unique_ptr<weld::ComboBox> m_xCmbStyle;

    DECL_LINK(ToggleHdl, weld::Toggleable&, void);
    DECL_LINK(SelectHdl, weld::ComboBox&, void);
    DECL_LINK(KeyInputHdl, const KeyEvent&, bool);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
