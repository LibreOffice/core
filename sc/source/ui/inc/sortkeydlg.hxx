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
#include <memory>

#include <vcl/weld.hxx>

struct ScSortKeyItem
{
    std::unique_ptr<weld::Builder> m_xBuilder;

    std::unique_ptr<weld::Frame> m_xFrame;
    std::unique_ptr<weld::ComboBox> m_xLbSort;
    std::unique_ptr<weld::RadioButton> m_xBtnUp;
    std::unique_ptr<weld::RadioButton> m_xBtnDown;
    weld::Container* m_pParent;

    ScSortKeyItem(weld::Container* pParent);
    ~ScSortKeyItem();

    void DisableField();
    void EnableField();
};

typedef std::vector<std::unique_ptr<ScSortKeyItem>> ScSortKeyItems;

class ScSortKeyWindow
{
public:
    ScSortKeyItems m_aSortKeyItems;

private:
    weld::Container* m_pBox;

public:
    ScSortKeyWindow(weld::Container* pBox);
    ~ScSortKeyWindow();

    void AddSortKey(sal_uInt16 nItem);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
