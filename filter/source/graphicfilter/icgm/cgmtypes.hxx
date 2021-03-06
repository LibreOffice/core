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

#include <tools/long.hxx>

struct FloatPoint
{
        double          X;
        double          Y;
        FloatPoint():X(0), Y(0){};
};

struct FloatRect
{
        double          Left;
        double          Top;
        double          Right;
        double          Bottom;
        FloatRect():Left(0), Top(0), Right(0), Bottom(0){};
        void Justify()
        {
            double fTemp;
            if ( Left > Right )
            {
                fTemp = Left;
                Left = Right;
                Right = fTemp;
            }
            if ( Top > Bottom )
            {
                fTemp = Top;
                Top = Bottom;
                Bottom = fTemp;
            }
        }
};

struct HatchEntry
{
    int     HatchStyle;
    tools::Long    HatchDistance;
    tools::Long    HatchAngle;
};

#define ASF_LINETYPE            0x00000001UL
#define ASF_LINEWIDTH           0x00000002UL
#define ASF_LINECOLOR           0x00000004UL
#define ASF_MARKERTYPE          0x00000008UL
#define ASF_MARKERSIZE          0x00000010UL
#define ASF_MARKERCOLOR         0x00000020UL    // NS
#define ASF_FILLINTERIORSTYLE   0x00000040UL
#define ASF_HATCHINDEX          0x00000080UL
#define ASF_PATTERNINDEX        0x00000100UL
#define ASF_BITMAPINDEX         0x00000200UL    // NS
#define ASF_FILLCOLOR           0x00000400UL
#define ASF_EDGETYPE            0x00000800UL
#define ASF_EDGEWIDTH           0x00001000UL
#define ASF_EDGECOLOR           0x00002000UL
#define ASF_TEXTFONTINDEX       0x00004000UL
#define ASF_TEXTPRECISION       0x00008000UL
#define ASF_CHARACTEREXPANSION  0x00010000UL
#define ASF_CHARACTERSPACING    0x00020000UL
#define ASF_TEXTCOLOR           0x00040000UL

#define ACT4_GRADIENT_ACTION    0x00000001UL

enum RealPrecision      { RP_FLOAT = 0, RP_FIXED = 1 };

enum ScalingMode        { SM_ABSTRACT = 0, SM_METRIC = 1 };

enum VDCType            { VDC_INTEGER = 0, VDC_REAL = 1 };
enum DeviceViewPortMode { DVPM_FRACTION = 0, DVPM_METRIC = 1, DVPM_DEVICE = 2 };
enum DeviceViewPortMap  { DVPM_NOT_FORCED = 0, DVPM_FORCED = 1 };
enum DeviceViewPortMapH { DVPMH_LEFT = 0, DVPMH_CENTER = 1, CVPMH_RIGHT = 2 };
enum DeviceViewPortMapV { DVPMV_BOTTOM = 0, DVPMV_CENTER = 1, DVPMV_TOP = 2 };

enum ClipIndicator      { CI_OFF = 0, CI_ON = 1 };

enum ColorSelectionMode { CSM_INDEXED = 0, CSM_DIRECT = 1 };
enum ColorModel         { CM_RGB = 0, CM_CMYK = 1 };

enum CharacterCodingA   { CCA_BASIC_7 = 0, CCA_BASIC_8 = 1, CCA_EXT_7 = 2, CCA_EXT_8 = 3 };
enum TextPrecision      { TPR_STRING = 0, TPR_CHARACTER = 1, TPR_STROKE = 2, TPR_UNDEFINED = 0xffff };
enum TextPath           { TPR_RIGHT = 0, TPR_LEFT = 1, TPR_UP = 2, TPR_DOWN = 3 };
enum TextAlignmentH     { TAH_NORMAL = 0, TAH_LEFT = 1, TAH_CENTER = 2, TAH_RIGHT = 3, TAH_CONT = 4 };
enum TextAlignmentV     { TAV_NORMAL = 0, TAV_TOP = 1, TAV_CAP = 2, TAV_HALF = 3, TAV_BASE = 4, TAV_BOTTOM = 5, TAV_CONT = 6 };
enum UnderlineMode      { UM_OFF = 0, UM_LOW = 1, UM_HIGH = 2, UM_STRIKEOUT = 4, UM_OVERSCORE = 8 };
enum FinalFlag          { FF_NOT_FINAL = 0, FF_FINAL = 1 };

enum LineType           { LT_SOLID = 1, LT_DASH = 2, LT_DOT = 3, LT_DASHDOT = 4, LT_DASHDOTDOT = 5,     // Standard
                            LT_NONE = -4, LT_DOTDOTSPACE = -3, LT_LONGDASH = -2, LT_DASHDASHDOT = -1 }; // GDSF Styles
enum SpecMode           { SM_ABSOLUTE = 0, SM_SCALED = 1 };
enum LineCapType        { LCT_BUTT = 0, LCT_ROUND = 1, LCT_SQUARE = 2, LCT_TRIANGLE = 3, LCT_ARROW = 4, LCT_NONE = -1 };
enum LineJoinType       { LJT_MITER = 0, LJT_ROUND = 1, LJT_BEVEL = 2, LJT_NONE = -1 };


enum EdgeType           { ET_SOLID = 1, ET_DASH = 2, ET_DOT = 3, ET_DASHDOT = 4, ET_DASHDOTDOT = 5,     // Standard
                            ET_NONE = -4, ET_DOTDOTSPACE = -3, ET_LONGDASH = -2, ET_DASHDASHDOT = -1 }; // GDSF Styles
enum EdgeVisibility     { EV_OFF = 0, EV_ON = 1 };

enum MarkerType         { MT_POINT = 1, MT_PLUS = 2, MT_STAR = 3, MT_CIRCLE = 4, MT_CROSS = 5 };

enum Transparency       { T_OFF = 0, T_ON = 1 };

enum FillInteriorStyle  { FIS_HOLLOW = 0, FIS_SOLID = 1, FIS_PATTERN = 2, FIS_HATCH = 3, FIS_EMPTY = 4, FIS_GEOPATTERN = 5,
                            FIS_INTERPOLATED = 6, FIS_GRADIENT = 7 };


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
