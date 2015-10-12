/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_SC_SOURCE_UI_INC_PIVOTLAYOUTTREELIST_HXX
#define INCLUDED_SC_SOURCE_UI_INC_PIVOTLAYOUTTREELIST_HXX

#include "PivotLayoutTreeListBase.hxx"

class ScPivotLayoutTreeList : public ScPivotLayoutTreeListBase
{
public:
    ScPivotLayoutTreeList(vcl::Window* pParent, WinBits nBits);
    virtual ~ScPivotLayoutTreeList();
    virtual bool DoubleClickHdl() override;

    void Setup(ScPivotLayoutDialog* pParent, SvPivotTreeListType eType);
    void FillFields(ScPivotFieldVector& rFieldVector);

protected:
    virtual void InsertEntryForSourceTarget(SvTreeListEntry* pSource, SvTreeListEntry* pTarget) override;
    virtual void InsertEntryForItem(ScItemValue* pItemValue, sal_uLong nPosition) override;

    virtual void KeyInput(const KeyEvent& rKeyEvent) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

#endif
