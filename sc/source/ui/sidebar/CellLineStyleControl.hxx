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
#ifndef INCLUDED_SC_SOURCE_UI_SIDEBAR_CELLLINESTYLECONTROL_HXX
#define INCLUDED_SC_SOURCE_UI_SIDEBAR_CELLLINESTYLECONTROL_HXX

#include <svx/sidebar/PopupControl.hxx>
#include <vcl/button.hxx>
#include "CellLineStyleValueSet.hxx"

namespace sc { namespace sidebar {

class CellAppearancePropertyPanel;

class CellLineStyleControl : public svx::sidebar::PopupControl
{
private:
    CellAppearancePropertyPanel&       mrCellAppearancePropertyPanel;
    VclPtr<PushButton>                 maPushButtonMoreOptions;
    VclPtr<CellLineStyleValueSet>      maCellLineStyleValueSet;
    OUString                           maStr[CELL_LINE_STYLE_ENTRIES];

    /// bitfield
    bool                               mbVSfocus : 1;

    void Initialize();
    void SetAllNoSel();

    DECL_LINK_TYPED(VSSelectHdl, ValueSet*, void);
    DECL_LINK_TYPED(PBClickHdl, Button*, void);

public:
    CellLineStyleControl(vcl::Window* pParent, CellAppearancePropertyPanel& rPanel);
    virtual ~CellLineStyleControl();
    virtual void dispose() override;

    void GetFocus() override;
    void SetLineStyleSelect(sal_uInt16 out, sal_uInt16 in, sal_uInt16 dis);
};

} } // end of namespace svx::sidebar

#endif // INCLUDED_SC_SOURCE_UI_SIDEBAR_CELLLINESTYLECONTROL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
