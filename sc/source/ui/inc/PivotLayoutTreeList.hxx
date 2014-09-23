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
    virtual bool DoubleClickHdl() SAL_OVERRIDE;

    void Setup(ScPivotLayoutDialog* pParent, SvPivotTreeListType eType);
    void FillFields(ScPivotFieldVector& rFieldVector);

protected:
    virtual void InsertEntryForSourceTarget(SvTreeListEntry* pSource, SvTreeListEntry* pTarget) SAL_OVERRIDE;
    virtual void InsertEntryForItem(ScItemValue* pItemValue, sal_uLong nPosition) SAL_OVERRIDE;

    virtual void KeyInput(const KeyEvent& rKeyEvent) SAL_OVERRIDE;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

#endif
