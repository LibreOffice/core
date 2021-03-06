/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#pragma once

#include <com/sun/star/awt/XWindow.hpp>
#include <vcl/ctrl.hxx>
#include <vcl/scrbar.hxx>
#include <types.hxx>
#include "hdrcont.hxx"

class ScDocument;

class ScDataTableColView : public ScHeaderControl
{
    ScDocument* mpDoc;
    SCCOL mnCol;

public:
    ScDataTableColView(vcl::Window* pParent, SelectionEngine* pSelectionEngine);

    void SetPos(SCCOLROW nRow);
    void Init(ScDocument* pDoc);

    virtual SCCOLROW GetPos() const override;
    virtual sal_uInt16 GetEntrySize(SCCOLROW nPos) const override;
    virtual OUString GetEntryText(SCCOLROW nPos) const override;
    virtual bool IsLayoutRTL() const override;
    virtual void SetEntrySize(SCCOLROW nPos, sal_uInt16 nWidth) override;
    virtual void HideEntries(SCCOLROW nPos, SCCOLROW nEndPos) override;
};

class ScDataTableRowView : public ScHeaderControl
{
    ScDocument* mpDoc;
    SCROW mnRow;

public:
    ScDataTableRowView(vcl::Window* pParent, SelectionEngine* pSelectionEngine);

    void SetPos(SCCOLROW nRow);
    void Init(ScDocument* pDoc);

    virtual SCCOLROW GetPos() const override;
    virtual sal_uInt16 GetEntrySize(SCCOLROW nPos) const override;
    virtual OUString GetEntryText(SCCOLROW nPos) const override;
    virtual bool IsLayoutRTL() const override;
    virtual void SetEntrySize(SCCOLROW nPos, sal_uInt16 nWidth) override;
    virtual void HideEntries(SCCOLROW nPos, SCCOLROW nEndPos) override;
};

/*
 * A simple UI component that presents a data table.
 *
 * Shares as much code as possible with the normal
 * Calc grid rendering.
 *
 * This class should only depend on ScDocument and not
 * on some of the Calc view shells.
 */
class ScDataTableView : public Control
{
    std::shared_ptr<ScDocument> mpDoc;
    std::unique_ptr<SelectionEngine> mpSelectionEngine;
    VclPtr<ScrollBarBox> mpTopLeft;
    VclPtr<ScDataTableColView> mpColView;
    VclPtr<ScDataTableRowView> mpRowView;
    VclPtr<ScrollBar> mpVScroll;
    VclPtr<ScrollBar> mpHScroll;

    SCROW mnFirstVisibleRow;
    SCCOL mnFirstVisibleCol;

    std::unique_ptr<MouseEvent> mpMouseEvent;

    DECL_LINK(ScrollHdl, ScrollBar*, void);

public:
    ScDataTableView(const css::uno::Reference<css::awt::XWindow>& rParent);

    void Init(std::shared_ptr<ScDocument> pDoc);

    ~ScDataTableView() override;

    virtual void dispose() override;

    virtual void MouseButtonDown(const MouseEvent& rMEvt) override;
    virtual void MouseButtonUp(const MouseEvent& rMEvt) override;
    virtual void Resize() override;
    virtual void Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect) override;
    virtual Size GetOptimalSize() const override;

    void getColRange(SCCOL& rStartCol, SCCOL& rEndCol) const;
    void getRowRange(SCROW& rStartRow, SCROW& rEndRow) const;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
