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

#ifndef INCLUDED_HWPFILTER_SOURCE_DRAWDEF_H
#define INCLUDED_HWPFILTER_SOURCE_DRAWDEF_H

#include "hwplib.h"

class HWPPara;

/**
 * Constant for drawing objects
 */
enum objtype
{
    HWPDO_CONTAINER,
    HWPDO_LINE,
    HWPDO_RECT,
    HWPDO_ELLIPSE,
    HWPDO_ARC,
    HWPDO_FREEFORM,
    HWPDO_TEXTBOX,
    HWPDO_CURVE,
    HWPDO_ADVANCED_ELLIPSE,
    HWPDO_ADVANCED_ARC,
    HWPDO_CLOSED_FREEFORM,
    HWPDO_NITEMS
};

#define HWPDO_PAT_TYPE_BITS 0xFF000000

#define HWPDO_PAT_SOLID     0x00000000

#define HWPDO_COLOR_NONE    0x10000000

#define HWPDO_FLAG_DRAW_PIE 0x00000002


#define HWPDO_FLAG_GRADATION    0x00010000
#define HWPDO_FLAG_ROTATION 0x00020000
#define HWPDO_FLAG_BITMAP   0x00040000
#define HWPDO_FLAG_AS_TEXTBOX   0x00080000
#define HWPDO_FLAG_WATERMARK   0x00100000

/**
 * @short Basic properties for drawing objects
 *
 * The order of pstyle, hstyle and tstyle is important.
 * @see LineObject
 */
struct BAREHWPDOProperty
{
    int line_pstyle;
    int line_hstyle;
    int line_tstyle;
    unsigned int line_color;
    hunit line_width;
    unsigned int fill_color;
    uint pattern_type;
    unsigned int pattern_color;
    hunit hmargin;
    hunit vmargin;
    uint flag;
};

/**
 * @short Gradation properties for drawing object
 */
struct GradationProperty
{
    int fromcolor;
    int tocolor;
    int gstyle;
    int angle;
    int center_x;
    int center_y;
    int nstep;
};

/**
 * @short Bitmap properties for drawing object
 */
struct BitmapProperty
{
    ZZPoint offset1;
    ZZPoint offset2;
    char szPatternFile[260 + 1];
    char pictype;
};

/**
 * @short Rotation properties for drawing object
 */
struct RotationProperty
{
    int rot_originx;
    int rot_originy;
    ZZParall parall;
};

/**
 * @short All properties for drawing object
 */
struct HWPDOProperty
{
    int line_pstyle; /* Style of the middle of line */
    int line_hstyle; /* Style of the end of line */
    int line_tstyle; /* Style of the start of line */
    unsigned int line_color;
    hunit line_width;
    unsigned int fill_color;
    uint pattern_type;
    unsigned int pattern_color;
    hunit hmargin;
    hunit vmargin;
    uint flag;

     /* Rotation properties */
    int rot_originx;
    int rot_originy;
    ZZParall parall;

     /* Gradation properties */

    int fromcolor;
    int tocolor;
    int gstyle;
    int angle;
    int center_x;
    int center_y;
    int nstep;

     /* Watermark */
     int luminance;
     int contrast;
     int greyscale;

     /* As TextBox */
    HWPPara *pPara;


    ZZPoint offset1;
    ZZPoint offset2;
    char szPatternFile[260 + 1];
    char pictype;
};

/**
 * @short Line Object
 */
struct HWPDOLine
{
    uint flip;
};

/**
 * @short Polygon or Polyline Object
 */
struct HWPDOFreeForm
{
    int npt;
    ZZPoint *pt;
};

/**
 * @short Textbox Object
 */
struct HWPDOTextBox
{
    HWPPara *h;
};

/**
 * @short Arc Object
 */
struct HWPDOArc
{
    ZZPoint radial[2];
};

/**
 * @short Common header for drawing object
 */
struct HWPDrawingObject
{
/**
 * type of drawing object
 */
    int type;
/**
 * offset from origin of current group
 */
    ZZPoint offset;
/**
 * offset from origin of drawing object
 */
    ZZPoint offset2;
    ZZSize extent;
/**
 * rectangle with consideration of line width
 */
    ZZRect vrect;
    HWPDOProperty property;
    union
    {
        HWPDOLine line_arc;
        HWPDOFreeForm freeform;
        HWPDOTextBox textbox;
/**
 * extended from hwpw96
 */
        HWPDOArc arc;
    }
    u;
    struct HWPDrawingObject *next;
/**
 * This exists for container object
 */
    struct HWPDrawingObject *child;
    int index;
    HWPDrawingObject();
    ~HWPDrawingObject();
};
#endif // INCLUDED_HWPFILTER_SOURCE_DRAWDEF_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
