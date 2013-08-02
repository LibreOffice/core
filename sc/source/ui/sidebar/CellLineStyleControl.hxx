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
#ifndef SC_SIDEBAR_CELL_LINE_STYLE_CONTROL_HXX
#define SC_SIDEBAR_CELL_LINE_STYLE_CONTROL_HXX

#include <svx/sidebar/PopupControl.hxx>
#include <vcl/button.hxx>
#include <CellLineStyleValueSet.hxx>

namespace sc { namespace sidebar {

class CellAppearancePropertyPanel;

class CellLineStyleControl : public svx::sidebar::PopupControl
{
private:
    CellAppearancePropertyPanel&       mrCellAppearancePropertyPanel;
    PushButton                         maPushButtonMoreOptions;
    CellLineStyleValueSet              maCellLineStyleValueSet;
    OUString                           maStr[CELL_LINE_STYLE_ENTRIES];

    /// bitfield
    bool                               mbVSfocus : 1;

    void Initialize();
    void SetAllNoSel();

    DECL_LINK(VSSelectHdl, void*);
    DECL_LINK(PBClickHdl, PushButton*);

public:
    CellLineStyleControl(Window* pParent, CellAppearancePropertyPanel& rPanel);

    void GetFocus();
    void SetLineStyleSelect(sal_uInt16 out, sal_uInt16 in, sal_uInt16 dis);

    ValueSet& GetValueSet()
    {
        return maCellLineStyleValueSet;
    }

    Control& GetPushButton()
    {
        return maPushButtonMoreOptions;
    }
};

} } // end of namespace svx::sidebar

#endif // SC_SIDEBAR_CELL_LINE_STYLE_CONTROL_HXX

// eof
