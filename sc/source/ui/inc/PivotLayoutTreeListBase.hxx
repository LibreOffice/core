/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef PIVOTLAYOUTTREELISTBASE_HXX
#define PIVOTLAYOUTTREELISTBASE_HXX

#include <svtools/treelistbox.hxx>

#include <boost/ptr_container/ptr_vector.hpp>
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
private:
    bool mbIsInternalDrag;

protected:
    SvPivotTreeListType meType;
    ScPivotLayoutDialog* mpParent;

public:
    void Setup(ScPivotLayoutDialog* pParent);

    ScPivotLayoutTreeListBase(Window* pParent, WinBits nBits, SvPivotTreeListType eType = UNDEFINED);
    virtual ~ScPivotLayoutTreeListBase();

    virtual sal_Int8 AcceptDrop(const AcceptDropEvent& rEvent) SAL_OVERRIDE;
    virtual bool NotifyAcceptDrop(SvTreeListEntry* pEntry) SAL_OVERRIDE;
    virtual sal_Bool NotifyMoving(SvTreeListEntry* pTarget, SvTreeListEntry* pSource,
                                  SvTreeListEntry*& rpNewParent, sal_uLong& rNewChildPos) SAL_OVERRIDE;
    virtual sal_Bool NotifyCopying(SvTreeListEntry* pTarget, SvTreeListEntry* pSource,
                                   SvTreeListEntry*& rpNewParent, sal_uLong& rNewChildPos) SAL_OVERRIDE;
    virtual DragDropMode NotifyStartDrag(TransferDataContainer& aTransferDataContainer,
                                         SvTreeListEntry* pEntry) SAL_OVERRIDE;
    virtual void DragFinished(sal_Int8 nDropAction) SAL_OVERRIDE;

    virtual void GetFocus() SAL_OVERRIDE;
    virtual void LoseFocus() SAL_OVERRIDE;

    void PushEntriesToPivotFieldVector(ScPivotFieldVector& rVector);

    void RemoveEntryForItem(ScItemValue* pItemValue);

    virtual bool HasEntry(SvTreeListEntry* pEntry);

protected:
    virtual void InsertEntryForSourceTarget(SvTreeListEntry* pSource, SvTreeListEntry* pTarget);

    virtual void InsertEntryForItem(ScItemValue* pItemValue, sal_uLong nPosition);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

#endif
