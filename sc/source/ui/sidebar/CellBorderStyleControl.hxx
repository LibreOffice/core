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

class SfxDispatcher;

namespace sc::sidebar
{
class CellBorderStylePopup : public WeldToolbarPopup
{
private:
    MenuOrToolMenuButton maToolButton;
    SfxDispatcher* mpDispatcher;
    std::unique_ptr<weld::Toolbar> mxTBBorder1;
    std::unique_ptr<weld::Toolbar> mxTBBorder2;
    std::unique_ptr<weld::Toolbar> mxTBBorder3;
    std::unique_ptr<weld::Toolbar> mxTBBorder4;

    void Initialize();

    DECL_LINK(TB1SelectHdl, const OString&, void);
    DECL_LINK(TB2and3SelectHdl, const OString&, void);
    DECL_LINK(TB4SelectHdl, const OString&, void);

public:
    CellBorderStylePopup(weld::Toolbar* pParent, const OString& rId, SfxDispatcher* pDispatcher);
    virtual void GrabFocus() override;
    virtual ~CellBorderStylePopup() override;
};

} // end of namespace svx::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
