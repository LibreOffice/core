/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef PIVOTLAYOUTTREELISTLABEL_HXX
#define PIVOTLAYOUTTREELISTLABEL_HXX

#include "PivotLayoutTreeListBase.hxx"
#include <boost/ptr_container/ptr_vector.hpp>

class ScPivotLayoutTreeListLabel : public ScPivotLayoutTreeListBase
{
private:
    boost::ptr_vector<ScItemValue> maItemValues;
    SCCOL maDataItem;

public:
    ScPivotLayoutTreeListLabel(Window* pParent, WinBits nBits);
    virtual ~ScPivotLayoutTreeListLabel();
    void FillLabelFields(ScDPLabelDataVector& rLabelVector);
    ScItemValue* GetItem(SCCOL nColumn);
    bool IsDataElement(SCCOL nColumn);

protected:
    virtual void InsertEntryForSourceTarget(SvTreeListEntry* pSource, SvTreeListEntry* pTarget) SAL_OVERRIDE;
    virtual void KeyInput(const KeyEvent& rKeyEvent) SAL_OVERRIDE;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

#endif
