/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_SC_SOURCE_UI_INC_PIVOTLAYOUTTREELISTLABEL_HXX
#define INCLUDED_SC_SOURCE_UI_INC_PIVOTLAYOUTTREELISTLABEL_HXX

#include "PivotLayoutTreeListBase.hxx"
#include <vector>
#include <memory>

class ScPivotLayoutTreeListLabel : public ScPivotLayoutTreeListBase
{
private:
    std::vector<std::unique_ptr<ScItemValue> > maItemValues;
    SCCOL maDataItem;

public:
    ScPivotLayoutTreeListLabel(vcl::Window* pParent, WinBits nBits);
    virtual ~ScPivotLayoutTreeListLabel();
    void FillLabelFields(ScDPLabelDataVector& rLabelVector);
    ScItemValue* GetItem(SCCOL nColumn);
    bool IsDataElement(SCCOL nColumn);

protected:
    virtual void InsertEntryForSourceTarget(SvTreeListEntry* pSource, SvTreeListEntry* pTarget) override;
    virtual void KeyInput(const KeyEvent& rKeyEvent) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

#endif
