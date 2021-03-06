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

#include <vcl/transfer.hxx>
#include <vcl/weld.hxx>
#include <pivot.hxx>

class ScPivotLayoutDialog;
class ScPivotLayoutTreeListBase;
class ScItemValue;

class ScPivotLayoutTreeDropTarget : public DropTargetHelper
{
private:
    ScPivotLayoutTreeListBase& m_rTreeView;

    virtual sal_Int8 AcceptDrop(const AcceptDropEvent& rEvt) override;
    virtual sal_Int8 ExecuteDrop(const ExecuteDropEvent& rEvt) override;

public:
    ScPivotLayoutTreeDropTarget(ScPivotLayoutTreeListBase& rTreeView);
};

class ScPivotLayoutTreeListBase
{
public:
    enum SvPivotTreeListType
    {
        UNDEFINED,
        LABEL_LIST,
        PAGE_LIST,
        ROW_LIST,
        COLUMN_LIST
    };

protected:
    std::unique_ptr<weld::TreeView> mxControl;
    ScPivotLayoutTreeDropTarget maDropTargetHelper;
    SvPivotTreeListType meType;
    ScPivotLayoutDialog* mpParent;

    DECL_LINK(GetFocusHdl, weld::Widget&, void);
    DECL_LINK(MnemonicActivateHdl, weld::Widget&, bool);
    DECL_LINK(LoseFocusHdl, weld::Widget&, void);

public:
    void Setup(ScPivotLayoutDialog* pParent);

    ScPivotLayoutTreeListBase(std::unique_ptr<weld::TreeView> xControl,
                              SvPivotTreeListType eType = UNDEFINED);
    weld::TreeView& get_widget() { return *mxControl; }
    virtual ~ScPivotLayoutTreeListBase();

    void PushEntriesToPivotFieldVector(ScPivotFieldVector& rVector);

    void RemoveEntryForItem(const ScItemValue* pItemValue);

    virtual void InsertEntryForSourceTarget(weld::TreeView& rSource, int nTarget);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
