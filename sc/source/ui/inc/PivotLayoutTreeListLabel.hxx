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

class ScPivotLayoutTreeListLabel : public ScPivotLayoutTreeListBase
{
private:
    std::vector<std::unique_ptr<ScItemValue>> maItemValues;
    SCCOL maDataItem;

    DECL_LINK(KeyInputHdl, const KeyEvent&, bool);

public:
    ScPivotLayoutTreeListLabel(std::unique_ptr<weld::TreeView> xControl);
    virtual ~ScPivotLayoutTreeListLabel() override;
    void FillLabelFields(ScDPLabelDataVector& rLabelVector);
    ScItemValue* GetItem(SCCOL nColumn);
    bool IsDataElement(SCCOL nColumn);
    virtual void InsertEntryForSourceTarget(weld::TreeView& rSource, int nTarget) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
