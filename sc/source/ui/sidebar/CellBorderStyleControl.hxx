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
#ifndef SC_SIDEBAR_CELL_BORDER_STYLE_CONTROL_HXX
#define SC_SIDEBAR_CELL_BORDER_STYLE_CONTROL_HXX

#include <svx/sidebar/PopupControl.hxx>
#include <vcl/button.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/fixed.hxx>

namespace sc { namespace sidebar {

class CellAppearancePropertyPanel;

class CellBorderStyleControl : public svx::sidebar::PopupControl
{
private:
    CellAppearancePropertyPanel&    mrCellAppearancePropertyPanel;
    ToolBox                         maTBBorder1;
    ToolBox                         maTBBorder2;
    ToolBox                         maTBBorder3;
    FixedLine                       maFL1;
    FixedLine                       maFL2;
    Image*                          mpImageList;

    void Initialize();

    DECL_LINK(TB1SelectHdl, ToolBox *);
    DECL_LINK(TB2SelectHdl, ToolBox *);
    DECL_LINK(TB3SelectHdl, ToolBox *);

public:
    CellBorderStyleControl(Window* pParent, CellAppearancePropertyPanel& rPanel);
    virtual ~CellBorderStyleControl(void);
};

} } // end of namespace svx::sidebar

#endif // SC_SIDEBAR_CELL_BORDER_STYLE_CONTROL_HXX

// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
