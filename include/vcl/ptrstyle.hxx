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
    Arrow              = 0,
    Null               = 1,
    Wait               = 2,
    Text               = 3,
    Help               = 4,
    Cross              = 5,
    Move               = 6,
    NSize              = 7,
    SSize              = 8,
    WSize              = 9,
    ESize              = 10,
    NWSize             = 11,
    NESize             = 12,
    SWSize             = 13,
    SESize             = 14,
    WindowNSize        = 15,
    WindowSSize        = 16,
    WindowWSize        = 17,
    WindowESize        = 18,
    WindowNWSize       = 19,
    WindowNESize       = 20,
    WindowSWSize       = 21,
    WindowSESize       = 22,
    HSplit             = 23,
    VSplit             = 24,
    HSizeBar           = 25,
    VSizeBar           = 26,
    Hand               = 27,
    RefHand            = 28,
    Pen                = 29,
    Magnify            = 30,
    Fill               = 31,
    Rotate             = 32,
    HShear             = 33,
    VShear             = 34,
    Mirror             = 35,
    Crook              = 36,
    Crop               = 37,
    MovePoint          = 38,
    MoveBezierWeight   = 39,
    MoveData           = 40,
    CopyData           = 41,
    LinkData           = 42,
    MoveDataLink       = 43,
    CopyDataLink       = 44,
    MoveFile           = 45,
    CopyFile           = 46,
    LinkFile           = 47,
    MoveFileLink       = 48,
    CopyFileLink       = 49,
    MoveFiles          = 50,
    CopyFiles          = 51,
    NotAllowed         = 52,
    DrawLine           = 53,
    DrawRect           = 54,
    DrawPolygon        = 55,
    DrawBezier         = 56,
    DrawArc            = 57,
    DrawPie            = 58,
    DrawCircleCut      = 59,
    DrawEllipse        = 60,
    DrawFreehand       = 61,
    DrawConnect        = 62,
    DrawText           = 63,
    DrawCaption        = 64,
    Chart              = 65,
    Detective          = 66,
    PivotCol           = 67,
    PivotRow           = 68,
    PivotField         = 69,
    Chain              = 70,
    ChainNotAllowed    = 71,
    TimeEventMove      = 72,
    TimeEventSize      = 73,
    AutoScrollN        = 74,
    AutoScrollS        = 75,
    AutoScrollW        = 76,
    AutoScrollE        = 77,
    AutoScrollNW       = 78,
    AutoScrollNE       = 79,
    AutoScrollSW       = 80,
    AutoScrollSE       = 81,
    AutoScrollNS       = 82,
    AutoScrollWE       = 83,
    AutoScrollNSWE     = 84,
    Airbrush           = 85,
    TextVertical       = 86,
    PivotDelete        = 87,
    TabSelectS         = 88,
    TabSelectE         = 89,
    TabSelectSE        = 90,
    TabSelectW         = 91,
    TabSelectSW        = 92,
    Paintbrush         = 93,
    HideWhitespace     = 94,
    ShowWhitespace     = 95,
    LAST               = ShowWhitespace,
};

#endif // INCLUDED_VCL_PTRSTYLE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
