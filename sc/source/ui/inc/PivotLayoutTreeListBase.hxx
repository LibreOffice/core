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

#include <vcl/weld.hxx>

#include <pivot.hxx>

class ScPivotLayoutDialog;
class ScItemValue;

class ScPivotLayoutTreeListBase
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
    std::unique_ptr<weld::TreeView> mxControl;
    SvPivotTreeListType meType;
    ScPivotLayoutDialog* mpParent;

    DECL_LINK(GetFocusHdl, weld::Widget&, void);
    DECL_LINK(LoseFocusHdl, weld::Widget&, void);

public:
    void Setup(ScPivotLayoutDialog* pParent);

    ScPivotLayoutTreeListBase(std::unique_ptr<weld::TreeView> xControl, SvPivotTreeListType eType = UNDEFINED);
    virtual ~ScPivotLayoutTreeListBase();

#if 0
    virtual bool NotifyAcceptDrop(SvTreeListEntry* pEntry) override;
    virtual TriState NotifyMoving(SvTreeListEntry* pTarget, SvTreeListEntry* pSource,
                                  SvTreeListEntry*& rpNewParent, sal_uLong& rNewChildPos) override;
    virtual TriState NotifyCopying(SvTreeListEntry* pTarget, SvTreeListEntry* pSource,
                                   SvTreeListEntry*& rpNewParent, sal_uLong& rNewChildPos) override;
    virtual DragDropMode NotifyStartDrag(TransferDataContainer& aTransferDataContainer,
                                         SvTreeListEntry* pEntry) override;
    virtual void DragFinished(sal_Int8 nDropAction) override;
#endif

    void PushEntriesToPivotFieldVector(ScPivotFieldVector& rVector);

    void RemoveEntryForItem(const ScItemValue* pItemValue);

    bool HasEntry(const weld::TreeIter& rEntry);

protected:
//TODO    virtual void InsertEntryForSourceTarget(SvTreeListEntry* pSource, SvTreeListEntry* pTarget);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

#endif
