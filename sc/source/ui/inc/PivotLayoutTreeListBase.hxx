/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_SC_SOURCE_UI_INC_PIVOTLAYOUTTREELISTBASE_HXX
#define INCLUDED_SC_SOURCE_UI_INC_PIVOTLAYOUTTREELISTBASE_HXX

#include <svtools/treelistbox.hxx>

#include <vcl/builder.hxx>

#include "pivot.hxx"

class ScPivotLayoutDialog;
class ScItemValue;

class ScPivotLayoutTreeListBase : public SvTreeListBox
{
public:
    enum SvPivotTreeListType
    {
        UNDEFINED,
        LABEL_LIST,
        PAGE_LIST,
        ROW_LIST,
        COLUMN_LIST,
        DATA_LIST
    };

protected:
    SvPivotTreeListType meType;
    VclPtr<ScPivotLayoutDialog> mpParent;

public:
    void Setup(ScPivotLayoutDialog* pParent);

    ScPivotLayoutTreeListBase(vcl::Window* pParent, WinBits nBits, SvPivotTreeListType eType = UNDEFINED);
    virtual ~ScPivotLayoutTreeListBase();
    virtual void dispose() override;

    virtual sal_Int8 AcceptDrop(const AcceptDropEvent& rEvent) override;
    virtual bool NotifyAcceptDrop(SvTreeListEntry* pEntry) override;
    virtual TriState NotifyMoving(SvTreeListEntry* pTarget, SvTreeListEntry* pSource,
                                  SvTreeListEntry*& rpNewParent, sal_uLong& rNewChildPos) override;
    virtual TriState NotifyCopying(SvTreeListEntry* pTarget, SvTreeListEntry* pSource,
                                   SvTreeListEntry*& rpNewParent, sal_uLong& rNewChildPos) override;
    virtual DragDropMode NotifyStartDrag(TransferDataContainer& aTransferDataContainer,
                                         SvTreeListEntry* pEntry) override;
    virtual void DragFinished(sal_Int8 nDropAction) override;

    void PushEntriesToPivotFieldVector(ScPivotFieldVector& rVector);

    void RemoveEntryForItem(ScItemValue* pItemValue);

    bool HasEntry(SvTreeListEntry* pEntry);

protected:
    virtual void InsertEntryForSourceTarget(SvTreeListEntry* pSource, SvTreeListEntry* pTarget);

    virtual void InsertEntryForItem(ScItemValue* pItemValue, sal_uLong nPosition);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

#endif
