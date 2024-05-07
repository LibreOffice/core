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

#include <rtl/ustring.hxx>

// this are the defines for the position of the settings in the
// ViewSettingsSequence

#define SC_VIEWSETTINGS_COUNT               27

#define SC_VIEW_ID                          0
#define SC_TABLE_VIEWSETTINGS               1
#define SC_ACTIVE_TABLE                     2
#define SC_HORIZONTAL_SCROLL_BAR_WIDTH      3
#define SC_ZOOM_TYPE                        4
#define SC_ZOOM_VALUE                       5
#define SC_PAGE_VIEW_ZOOM_VALUE             6
#define SC_PAGE_BREAK_PREVIEW               7
#define SC_SHOWZERO                         8
#define SC_SHOWNOTES                        9
#define SC_SHOWNOTEAUTHOR                   10
#define SC_SHOWFORMULASMARKS                11
#define SC_SHOWGRID                         12
#define SC_GRIDCOLOR                        13
#define SC_SHOWPAGEBR                       14
#define SC_COLROWHDR                        15
#define SC_SHEETTABS                        16
#define SC_OUTLSYMB                         17
#define SC_VALUE_HIGHLIGHTING               18
#define SC_SNAPTORASTER                     19
#define SC_RASTERVIS                        20
#define SC_RASTERRESX                       21
#define SC_RASTERRESY                       22
#define SC_RASTERSUBX                       23
#define SC_RASTERSUBY                       24
#define SC_RASTERSYNC                       25
#define SC_FORMULA_BAR_HEIGHT               26

// this are the defines for the position of the settings in the
// TableViewSettingsSequence

#define SC_TABLE_VIEWSETTINGS_COUNT         16

#define SC_CURSOR_X                         0
#define SC_CURSOR_Y                         1
#define SC_HORIZONTAL_SPLIT_MODE            2
#define SC_VERTICAL_SPLIT_MODE              3
#define SC_HORIZONTAL_SPLIT_POSITION        4
#define SC_VERTICAL_SPLIT_POSITION          5
#define SC_ACTIVE_SPLIT_RANGE               6
#define SC_POSITION_LEFT                    7
#define SC_POSITION_RIGHT                   8
#define SC_POSITION_TOP                     9
#define SC_POSITION_BOTTOM                  10
#define SC_TABLE_ZOOM_TYPE                  11
#define SC_TABLE_ZOOM_VALUE                 12
#define SC_TABLE_PAGE_VIEW_ZOOM_VALUE       13
#define SC_FORMULA_BAR_HEIGHT_VALUE         14
#define SC_TABLE_SHOWGRID                   15

inline constexpr OUString SC_CURSORPOSITIONX = u"CursorPositionX"_ustr;
inline constexpr OUString SC_CURSORPOSITIONY = u"CursorPositionY"_ustr;
inline constexpr OUString SC_HORIZONTALSPLITMODE = u"HorizontalSplitMode"_ustr;
inline constexpr OUString SC_VERTICALSPLITMODE = u"VerticalSplitMode"_ustr;
inline constexpr OUString SC_HORIZONTALSPLITPOSITION = u"HorizontalSplitPosition"_ustr;
inline constexpr OUString SC_VERTICALSPLITPOSITION = u"VerticalSplitPosition"_ustr;
#define SC_HORIZONTALSPLITPOSITION_TWIPS    "HorizontalSplitPositionTwips"
#define SC_VERTICALSPLITPOSITION_TWIPS      "VerticalSplitPositionTwips"
inline constexpr OUString SC_ACTIVESPLITRANGE = u"ActiveSplitRange"_ustr;
inline constexpr OUString SC_POSITIONLEFT = u"PositionLeft"_ustr;
inline constexpr OUString SC_POSITIONRIGHT = u"PositionRight"_ustr;
inline constexpr OUString SC_POSITIONTOP = u"PositionTop"_ustr;
inline constexpr OUString SC_POSITIONBOTTOM = u"PositionBottom"_ustr;
#define SC_TABLESELECTED                    "TableSelected"

inline constexpr OUString SC_TABLES = u"Tables"_ustr;
inline constexpr OUString SC_ACTIVETABLE = u"ActiveTable"_ustr;
inline constexpr OUString SC_HORIZONTALSCROLLBARWIDTH = u"HorizontalScrollbarWidth"_ustr;
#define SC_RELHORIZONTALTABBARWIDTH         "RelativeHorizontalTabbarWidth"
inline constexpr OUString SC_ZOOMTYPE = u"ZoomType"_ustr;
inline constexpr OUString SC_ZOOMVALUE = u"ZoomValue"_ustr;
inline constexpr OUString SC_PAGEVIEWZOOMVALUE = u"PageViewZoomValue"_ustr;
inline constexpr OUString SC_SHOWPAGEBREAKPREVIEW = u"ShowPageBreakPreview"_ustr;
inline constexpr OUString SC_FORMULABARHEIGHT = u"FormulaBarHeight"_ustr;
inline constexpr OUString SC_VIEWID = u"ViewId"_ustr;
#define SC_VIEW                             "view"

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
