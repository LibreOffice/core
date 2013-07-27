/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/

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
    Image*                          mpImageListH; //high contrast

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
