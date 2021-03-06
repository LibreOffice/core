/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#pragma once

#include "PivotLayoutTreeListBase.hxx"
#include <vector>
#include <memory>

class ScPivotLayoutTreeListData final : public ScPivotLayoutTreeListBase
{
private:
    DECL_LINK(KeyInputHdl, const KeyEvent&, bool);
    DECL_LINK(DoubleClickHdl, weld::TreeView&, bool);

public:
    ScPivotLayoutTreeListData(std::unique_ptr<weld::TreeView> xControl);
    virtual ~ScPivotLayoutTreeListData() override;

    void FillDataField(ScPivotFieldVector& rDataFields);
    void PushDataFieldNames(std::vector<ScDPName>& rDataFieldNames);
    virtual void InsertEntryForSourceTarget(weld::TreeView& rSource, int nTarget) override;

private:
    void InsertEntryForItem(ScItemValue* pItemValue, int nPosition);

    void AdjustDuplicateCount(ScItemValue* pInputItemValue);

    std::vector<std::unique_ptr<ScItemValue>> maDataItemValues;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
