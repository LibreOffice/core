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

#include <memory>
#include "PivotLayoutTreeListBase.hxx"
#include <scabstdlg.hxx>

class ScPivotLayoutTreeList : public ScPivotLayoutTreeListBase
{
private:
    std::vector<std::unique_ptr<ScItemValue>> maItemValues;
    std::vector<ScDPName> maDataFieldNames;

    VclPtr<AbstractScDPSubtotalDlg> mpSubtotalDlg;

    DECL_LINK(KeyInputHdl, const KeyEvent&, bool);
    DECL_LINK(DoubleClickHdl, weld::TreeView&, bool);

public:
    ScPivotLayoutTreeList(std::unique_ptr<weld::TreeView> xControl);
    virtual ~ScPivotLayoutTreeList() override;

    void Setup(ScPivotLayoutDialog* pParent, SvPivotTreeListType eType);
    void FillFields(ScPivotFieldVector& rFieldVector);

    virtual void InsertEntryForSourceTarget(weld::TreeView& rSource, int nTarget) override;

protected:
    void InsertEntryForItem(const ScItemValue* pItemValue, int nPosition);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
