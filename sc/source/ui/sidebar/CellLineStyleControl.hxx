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

#include <svtools/toolbarmenu.hxx>
#include <svx/colorwindow.hxx>
#include "CellLineStyleValueSet.hxx"

class SfxDispatcher;

namespace sc::sidebar
{
class CellLineStylePopup : public WeldToolbarPopup
{
private:
    MenuOrToolMenuButton maToolButton;
    SfxDispatcher* mpDispatcher;
    std::unique_ptr<CellLineStyleValueSet> mxCellLineStyleValueSet;
    std::unique_ptr<weld::CustomWeld> mxCellLineStyleValueSetWin;
    std::unique_ptr<weld::Button> mxPushButtonMoreOptions;
    OUString maStr[CELL_LINE_STYLE_ENTRIES];

    void Initialize();
    void SetAllNoSel();

    DECL_LINK(VSSelectHdl, ValueSet*, void);
    DECL_LINK(PBClickHdl, weld::Button&, void);

public:
    CellLineStylePopup(weld::Toolbar* pParent, const OString& rId, SfxDispatcher* pDispatcher);
    void SetLineStyleSelect(sal_uInt16 out, sal_uInt16 in, sal_uInt16 dis);
    virtual void GrabFocus() override;
    virtual ~CellLineStylePopup() override;
};

} // end of namespace svx::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
