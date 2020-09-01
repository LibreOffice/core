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

#include <vcl/InterimItemWindow.hxx>

class ScGridWindow;

enum class ScFilterBoxMode
{
    DataSelect,
    Scenario
};

class ScFilterListBox final : public InterimItemWindow
{
private:
    std::unique_ptr<weld::TreeView> xTreeView;
    VclPtr<ScGridWindow> pGridWin;
    SCCOL nCol;
    SCROW nRow;
    bool bInit;
    bool bCancelled;
    sal_uLong nSel;
    ScFilterBoxMode eMode;
    ImplSVEvent* nAsyncSelectHdl;

    DECL_LINK(SelectHdl, weld::TreeView&, bool);
    DECL_LINK(KeyInputHdl, const KeyEvent&, bool);
    DECL_LINK(AsyncSelectHdl, void*, void);

public:
    ScFilterListBox(vcl::Window* pParent, ScGridWindow* pGrid, SCCOL nNewCol, SCROW nNewRow,
                    ScFilterBoxMode eNewMode);
    virtual ~ScFilterListBox() override;
    virtual void dispose() override;

    weld::TreeView& get_widget() { return *xTreeView; }

    SCCOL GetCol() const { return nCol; }
    SCROW GetRow() const { return nRow; }
    ScFilterBoxMode GetMode() const { return eMode; }
    void EndInit();
    bool IsInInit() const { return bInit; }
    void SetCancelled() { bCancelled = true; }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
