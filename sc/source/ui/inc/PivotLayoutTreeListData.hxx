/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef PIVOTLAYOUTTREELISTDATA_HXX
#define PIVOTLAYOUTTREELISTDATA_HXX

#include "PivotLayoutTreeListBase.hxx"
#include <boost/ptr_container/ptr_vector.hpp>

class ScPivotLayoutTreeListData : public ScPivotLayoutTreeListBase
{
private:
    boost::ptr_vector<ScItemValue> maDataItemValues;

public:
    ScPivotLayoutTreeListData(Window* pParent, WinBits nBits);
    virtual ~ScPivotLayoutTreeListData();
    virtual bool DoubleClickHdl() SAL_OVERRIDE;

    void FillDataField(ScPivotFieldVector& rDataFields);
    void PushDataFieldNames(std::vector<ScDPName>& rDataFieldNames);

protected:
    virtual void InsertEntryForSourceTarget(SvTreeListEntry* pSource, SvTreeListEntry* pTarget) SAL_OVERRIDE;
    virtual void InsertEntryForItem(ScItemValue* pItemValue, sal_uLong nPosition) SAL_OVERRIDE;

    void AdjustDuplicateCount(ScItemValue* pInputItemValue);

    virtual void KeyInput(const KeyEvent& rKeyEvent) SAL_OVERRIDE;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

#endif
