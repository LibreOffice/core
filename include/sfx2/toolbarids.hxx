/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SFX2_TOOLBARIDS_HXX
#define INCLUDED_SFX2_TOOLBARIDS_HXX

#include <sal/types.h>

//these numbers end up in the configuration to identify toolbars
//so don't blindly change their numbers
enum class ToolbarId : sal_uInt32
{
    None = 0,
    FullScreenToolbox = 558,
    EnvToolbox = 560,
    Svx_Extrusion_Bar = 10986,
    Svx_Fontwork_Bar = 10987,
    Basicide_Objectbar = 14850,
    SvxTbx_Form_Navigation = 18001,
    SvxTbx_Form_Filter = 18002,
    SvxTbx_Text_Control_Attributes = 18003,
    SvxTbx_Controls = 18004,
    SvxTbx_FormDesign = 18006,
    Math_Toolbox = 20050,
    Webtools_Toolbox = 20402,
    Webtext_Toolbox = 20403,
    Webframe_Toolbox = 20408,
    Webgraphic_Toolbox = 20410,
    Webole_Toolbox = 20411,
    Draw_Toolbox_Sd = 23011,
    Slide_Toolbox = 23012,
    Draw_Obj_Toolbox = 23013,
    Slide_Obj_Toolbox = 23014,
    Bezier_Toolbox_Sd = 23015,
    Draw_Text_Toolbox_Sd = 23016,
    Outline_Toolbox = 23017,
    Draw_Table_Toolbox = 23018,
    Gluepoints_Toolbox = 23019,
    Draw_Options_Toolbox = 23020,
    Draw_CommonTask_Toolbox = 23021,
    FormLayer_Toolbox = 23022,
    Draw_Viewer_Toolbox = 23023,
    Graphic_Obj_Toolbox = 23027,
    Draw_Graf_Toolbox = 23030,
    Draw_Media_Toolbox = 23031,
    Text_Toolbox_Sw = 23265,
    Table_Toolbox = 23266,
    Frame_Toolbox = 23267,
    Grafik_Toolbox = 23268,
    Draw_Toolbox_Sw = 23269,
    Draw_Text_Toolbox_Sw = 23270,
    Num_Toolbox = 23271,
    Ole_Toolbox = 23272,
    Tools_Toolbox = 23273,
    PView_Toolbox = 23281,
    Bezier_Toolbox_Sw = 23283,
    Module_Toolbox = 23310,
    Media_Toolbox = 23311,
    Objectbar_App = 25000,
    Objectbar_Format = 25001,
    Text_Toolbox_Sc = 25005,
    Objectbar_Preview = 25006,
    Objectbar_Tools = 25035,
    Draw_Objectbar = 25053,
    Graphic_Objectbar = 25054,
    Media_Objectbar = 25060
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
