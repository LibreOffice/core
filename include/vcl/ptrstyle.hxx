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

enum class PointerStyle
{
    Arrow,
    Null,
    Wait,
    Text,
    Help,
    Cross,
    Move,
    NSize,
    SSize,
    WSize,
    ESize,
    NWSize,
    NESize,
    SWSize,
    SESize,
    WindowNSize,
    WindowSSize,
    WindowWSize,
    WindowESize,
    WindowNWSize,
    WindowNESize,
    WindowSWSize,
    WindowSESize,
    HSplit,
    VSplit,
    HSizeBar,
    VSizeBar,
    Hand,
    RefHand,
    Magnify,
    Fill,
    Rotate,
    HShear,
    VShear,
    Mirror,
    Crook,
    Crop,
    MovePoint,
    MoveBezierWeight,
    CopyData,
    LinkData,
    MoveDataLink,
    NotAllowed,
    DrawLine,
    DrawRect,
    DrawPolygon,
    DrawBezier,
    DrawArc,
    DrawPie,
    DrawCircleCut,
    DrawEllipse,
    DrawFreehand,
    DrawConnect,
    DrawText,
    DrawCaption,
    Detective,
    PivotCol,
    PivotRow,
    PivotField,
    Chain,
    ChainNotAllowed,
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
