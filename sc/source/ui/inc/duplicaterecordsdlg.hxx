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

#pragma once

#include "viewdata.hxx"
#include <vcl/weld.hxx>

using namespace css;

struct DuplicatesResponse
{
    std::vector<int> vEntries;
    bool bRemove; // false ==> Select
    bool bIncludesHeaders;
    bool bDuplicateRows; // false ==> DuplicateColumns
    DuplicatesResponse()
        : bRemove(false)
        , bIncludesHeaders(false)
        , bDuplicateRows(false)
    {
    }
};

class ScDuplicateRecordsDlg : public weld::GenericDialogController
{
public:
    ScDuplicateRecordsDlg() = delete;
    explicit ScDuplicateRecordsDlg(weld::Window* pParent,
                                   css::uno::Sequence<uno::Sequence<uno::Any>>& rData,
                                   ScViewData& rViewData, ScRange& aRange);
    virtual ~ScDuplicateRecordsDlg() override;

    const DuplicatesResponse& GetDialogData() { return maResponse; };

private:
    void Init();
    void SetDialogData(bool bToggle);
    void SetDialogLabels();
    std::unique_ptr<weld::CheckButton> m_xIncludesHeaders;
    std::unique_ptr<weld::RadioButton> m_xRadioRow;
    std::unique_ptr<weld::RadioButton> m_xRadioColumn;
    std::unique_ptr<weld::RadioButton> m_xRadioSelect;
    std::unique_ptr<weld::RadioButton> m_xRadioRemove;
    std::unique_ptr<weld::TreeView> m_xCheckList;
    std::unique_ptr<weld::CheckButton> m_xAllChkBtn;

    std::unique_ptr<weld::Button> m_xOkBtn;

    uno::Sequence<uno::Sequence<uno::Any>>& mrCellData;
    ScRange& mrRange;
    ScViewData& mrViewData;
    DuplicatesResponse maResponse;

    void InsertEntry(const OUString& rTxt, bool bToggle);

    DECL_LINK(OrientationHdl, weld::Toggleable&, void);
    DECL_LINK(HeaderCkbHdl, weld::Toggleable&, void);
    DECL_LINK(OkHdl, weld::Button&, void);
    DECL_LINK(AllCheckBtnHdl, weld::Toggleable&, void);
    DECL_LINK(RecordsChkHdl, const weld::TreeView::iter_col&, void);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
