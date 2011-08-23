/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

/* NAME $Id: drawdef.h,v 1.5 2008-04-10 11:58:04 rt Exp $
 *
 * HISTORY
 *        frog - Jan 11, 1999: Created.
 */

#ifndef _DRAWDEF_H_
#define _DRAWDEF_H_

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
#define HWPDO_PAT_STYLE_BITS    0x00FFFFFF

#define HWPDO_PAT_SOLID     0x00000000
#define HWPDO_PAT_WIN_HATCHED   0x01000000
#define HWPDO_PAT_BITMAP    0x02000000

#define HWPDO_COLOR_NONE    0x10000000

#define HWPDO_FLAG_ROUND_CORNER 0x00000001
#define HWPDO_FLAG_DRAW_PIE 0x00000002
#define HWPDO_FLAG_SEMICIRCLE   0x00000004
#define HWPDO_FLAG_SCALE_BITMAP 0x00000008

#define HWPDO_FLAG_LOCAL_BITS   0x0000ffff
#define HWPDO_FLAG_PROPERTY_BITS 0x00ff0000

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
    DWORD line_color;
    hunit line_width;
    DWORD fill_color;
    uint pattern_type;
    DWORD pattern_color;
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
    int line_pstyle; /* 선 중간 모양 */
    int line_hstyle; /* 끝 화살표 모양 */
    int line_tstyle; /* 시작 모양 */
    DWORD line_color;
    hunit line_width;
    DWORD fill_color;
    uint pattern_type;
    DWORD pattern_color;
    hunit hmargin;
    hunit vmargin;
    uint flag;

     /* Lotation properties */
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
    char szPatternFile[MAX_PATH + 1];
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
#endif                                            /* _DRAWDEF_H_ */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
