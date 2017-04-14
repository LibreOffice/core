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

#ifndef INCLUDED_VCL_PTRSTYLE_HXX
#define INCLUDED_VCL_PTRSTYLE_HXX

#include <com/sun/star/awt/SystemPointer.hpp>

enum class PointerStyle
{
    Arrow              = css::awt::SystemPointer::ARROW,
    Null               = css::awt::SystemPointer::INVISIBLE,
    Wait               = css::awt::SystemPointer::WAIT,
    Text               = css::awt::SystemPointer::TEXT,
    Help               = css::awt::SystemPointer::HELP,
    Cross              = css::awt::SystemPointer::CROSS,
    Move               = css::awt::SystemPointer::MOVE,
    NSize              = css::awt::SystemPointer::NSIZE,
    SSize              = css::awt::SystemPointer::SSIZE,
    WSize              = css::awt::SystemPointer::WSIZE,
    ESize              = css::awt::SystemPointer::ESIZE,
    NWSize             = css::awt::SystemPointer::NWSIZE,
    NESize             = css::awt::SystemPointer::NESIZE,
    SWSize             = css::awt::SystemPointer::SWSIZE,
    SESize             = css::awt::SystemPointer::SESIZE,
    WindowNSize        = css::awt::SystemPointer::WINDOW_NSIZE,
    WindowSSize        = css::awt::SystemPointer::WINDOW_SSIZE,
    WindowWSize        = css::awt::SystemPointer::WINDOW_WSIZE,
    WindowESize        = css::awt::SystemPointer::WINDOW_ESIZE,
    WindowNWSize       = css::awt::SystemPointer::WINDOW_NWSIZE,
    WindowNESize       = css::awt::SystemPointer::WINDOW_NESIZE,
    WindowSWSize       = css::awt::SystemPointer::WINDOW_SWSIZE,
    WindowSESize       = css::awt::SystemPointer::WINDOW_SESIZE,
    HSplit             = css::awt::SystemPointer::HSPLIT,
    VSplit             = css::awt::SystemPointer::VSPLIT,
    HSizeBar           = css::awt::SystemPointer::HSIZEBAR,
    VSizeBar           = css::awt::SystemPointer::VSIZEBAR,
    Hand               = css::awt::SystemPointer::HAND,
    RefHand            = css::awt::SystemPointer::REFHAND,
    Pen                = css::awt::SystemPointer::PEN,
    Magnify            = css::awt::SystemPointer::MAGNIFY,
    Fill               = css::awt::SystemPointer::FILL,
    Rotate             = css::awt::SystemPointer::ROTATE,
    HShear             = css::awt::SystemPointer::HSHEAR,
    VShear             = css::awt::SystemPointer::VSHEAR,
    Mirror             = css::awt::SystemPointer::MIRROR,
    Crook              = css::awt::SystemPointer::CROOK,
    Crop               = css::awt::SystemPointer::CROP,
    MovePoint          = css::awt::SystemPointer::MOVEPOINT,
    MoveBezierWeight   = css::awt::SystemPointer::MOVEBEZIERWEIGHT,
    MoveData           = css::awt::SystemPointer::MOVEDATA,
    CopyData           = css::awt::SystemPointer::COPYDATA,
    LinkData           = css::awt::SystemPointer::LINKDATA,
    MoveDataLink       = css::awt::SystemPointer::MOVEDATALINK,
    CopyDataLink       = css::awt::SystemPointer::COPYDATALINK,
    MoveFile           = css::awt::SystemPointer::MOVEFILE,
    CopyFile           = css::awt::SystemPointer::COPYFILE,
    LinkFile           = css::awt::SystemPointer::LINKFILE,
    MoveFileLink       = css::awt::SystemPointer::MOVEFILELINK,
    CopyFileLink       = css::awt::SystemPointer::COPYFILELINK,
    MoveFiles          = css::awt::SystemPointer::MOVEFILES,
    CopyFiles          = css::awt::SystemPointer::COPYFILES,
    NotAllowed         = css::awt::SystemPointer::NOTALLOWED,
    DrawLine           = css::awt::SystemPointer::DRAW_LINE,
    DrawRect           = css::awt::SystemPointer::DRAW_RECT,
    DrawPolygon        = css::awt::SystemPointer::DRAW_POLYGON,
    DrawBezier         = css::awt::SystemPointer::DRAW_BEZIER,
    DrawArc            = css::awt::SystemPointer::DRAW_ARC,
    DrawPie            = css::awt::SystemPointer::DRAW_PIE,
    DrawCircleCut      = css::awt::SystemPointer::DRAW_CIRCLECUT,
    DrawEllipse        = css::awt::SystemPointer::DRAW_ELLIPSE,
    DrawFreehand       = css::awt::SystemPointer::DRAW_FREEHAND,
    DrawConnect        = css::awt::SystemPointer::DRAW_CONNECT,
    DrawText           = css::awt::SystemPointer::DRAW_TEXT,
    DrawCaption        = css::awt::SystemPointer::DRAW_CAPTION,
    Chart              = css::awt::SystemPointer::CHART,
    Detective          = css::awt::SystemPointer::DETECTIVE,
    PivotCol           = css::awt::SystemPointer::PIVOT_COL,
    PivotRow           = css::awt::SystemPointer::PIVOT_ROW,
    PivotField         = css::awt::SystemPointer::PIVOT_FIELD,
    Chain              = css::awt::SystemPointer::CHAIN,
    ChainNotAllowed    = css::awt::SystemPointer::CHAIN_NOTALLOWED,
    AutoScrollN,
    AutoScrollS,
    AutoScrollW,
    AutoScrollE,
    AutoScrollNW,
    AutoScrollNE,
    AutoScrollSW,
    AutoScrollSE,
    AutoScrollNS,
    AutoScrollWE,
    AutoScrollNSWE,
    TextVertical,
    PivotDelete,
    TabSelectS,
    TabSelectE,
    TabSelectSE,
    TabSelectW,
    TabSelectSW,
    HideWhitespace,
    ShowWhitespace,
    LAST               = ShowWhitespace,
};

#endif // INCLUDED_VCL_PTRSTYLE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
