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
#ifndef INCLUDED_SC_SOURCE_UI_SIDEBAR_CELLBORDERSTYLECONTROL_HXX
#define INCLUDED_SC_SOURCE_UI_SIDEBAR_CELLBORDERSTYLECONTROL_HXX

#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <vcl/floatwin.hxx>
#include <vcl/toolbox.hxx>

class SfxDispatcher;

namespace sc { namespace sidebar {

class CellBorderStylePopup : public FloatingWindow
{
private:
    SfxDispatcher*                  mpDispatcher;
    VclPtr<ToolBox>                 maTBBorder1;
    VclPtr<ToolBox>                 maTBBorder2;
    VclPtr<ToolBox>                 maTBBorder3;

    void Initialize();

    DECL_LINK(TB1SelectHdl, ToolBox *, void);
    DECL_LINK(TB2SelectHdl, ToolBox *, void);
    DECL_LINK(TB3SelectHdl, ToolBox *, void);

public:
    explicit CellBorderStylePopup(SfxDispatcher* pDispatcher);
    virtual ~CellBorderStylePopup() override;
    virtual void dispose() override;
};

} } // end of namespace svx::sidebar

#endif // INCLUDED_SC_SOURCE_UI_SIDEBAR_CELLBORDERSTYLECONTROL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
