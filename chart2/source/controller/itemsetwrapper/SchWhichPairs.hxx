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

#include <svx/svxids.hrc>
#include <svx/xdef.hxx>
#include <svx/svddef.hxx>
#include <editeng/eeitem.hxx>

#include <chartview/ChartSfxItemIds.hxx>

const sal_uInt16 nTitleWhichPairs[] =
{
    SCHATTR_TEXT_START, SCHATTR_TEXT_END,
    XATTR_LINE_FIRST, XATTR_LINE_LAST,              //  1000 -  1016  svx/xdef.hxx
    XATTR_FILL_FIRST, XATTR_FILL_LAST,              //  1018 -  1046  svx/xdef.hxx
    SDRATTR_SHADOW_FIRST, SDRATTR_SHADOW_LAST,      //  1067 -  1078  svx/svddef.hxx
    EE_ITEMS_START, EE_ITEMS_END,                             // Characters
    SID_CHAR_DLG_PREVIEW_STRING, SID_CHAR_DLG_PREVIEW_STRING, // Characters
    0
};

const sal_uInt16 nAxisWhichPairs[] =
{
    SCHATTR_TEXT_START, SCHATTR_TEXT_END,
    SCHATTR_AXIS_START, SCHATTR_AXIS_END,
    XATTR_LINE_FIRST, XATTR_LINE_LAST,                          //  1000 -  1016  svx/xdef.hxx
    EE_ITEMS_START, EE_ITEMS_END,                             // Characters
    SID_ATTR_NUMBERFORMAT_VALUE, SID_ATTR_NUMBERFORMAT_VALUE,   // 10585 - 10585  svx/svxids.hrc
    SID_ATTR_NUMBERFORMAT_SOURCE, SID_ATTR_NUMBERFORMAT_SOURCE, // 11432          svx/svxids.hrc
    SID_CHAR_DLG_PREVIEW_STRING, SID_CHAR_DLG_PREVIEW_STRING, // Characters
    0
};

const sal_uInt16 nAllAxisWhichPairs[] =
{
    SCHATTR_TEXT_START, SCHATTR_TEXT_END,
    SCHATTR_AXIS_LABEL_START, SCHATTR_AXIS_LABEL_END,
    XATTR_LINE_FIRST, XATTR_LINE_LAST,
    EE_ITEMS_START, EE_ITEMS_END,                             // Characters
    SID_CHAR_DLG_PREVIEW_STRING, SID_CHAR_DLG_PREVIEW_STRING, // Characters
    0
};

const sal_uInt16 nGridWhichPairs[] =
{
    XATTR_LINE_FIRST, XATTR_LINE_LAST,              //  1000 -  1016  svx/xdef.hxx
    0
};

const sal_uInt16 nLegendWhichPairs[] =
{
    SCHATTR_LEGEND_START, SCHATTR_LEGEND_END,       //     3 -     3  sch/schattr.hxx
    XATTR_LINE_FIRST, XATTR_LINE_LAST,              //  1000 -  1016  svx/xdef.hxx
    XATTR_FILL_FIRST, XATTR_FILL_LAST,              //  1018 -  1046  svx/xdef.hxx
    SDRATTR_SHADOW_FIRST, SDRATTR_SHADOW_LAST,      //  1067 -  1078  svx/svddef.hxx
    EE_ITEMS_START, EE_ITEMS_END,                             // Characters
    SID_CHAR_DLG_PREVIEW_STRING, SID_CHAR_DLG_PREVIEW_STRING, // Characters
    0
};

const sal_uInt16 nDataLabelWhichPairs[] =
{
    SCHATTR_DATADESCR_START, SCHATTR_DATADESCR_END,
    SCHATTR_TEXT_DEGREES,SCHATTR_TEXT_DEGREES,
    EE_PARA_WRITINGDIR,EE_PARA_WRITINGDIR,
    SID_ATTR_NUMBERFORMAT_VALUE, SID_ATTR_NUMBERFORMAT_INFO,    /* 10585 - 10585  svx/svxids.hrc */
    SID_ATTR_NUMBERFORMAT_SOURCE, SID_ATTR_NUMBERFORMAT_SOURCE, /* 11432          svx/svxids.hrc */
    0
};

const sal_uInt16 nDataPointWhichPairs[] =
{
    SCHATTR_DATADESCR_START, SCHATTR_DATADESCR_END, /*     1 -     2  sch/schattr.hxx*/
    SCHATTR_TEXT_DEGREES, SCHATTR_TEXT_DEGREES,
    SCHATTR_STYLE_START,SCHATTR_STYLE_END,          /*    59 -    68  sch/schattr.hxx*/
    SCHATTR_SYMBOL_BRUSH,SCHATTR_SYMBOL_BRUSH,      /*    94          sch/schattr.hxx*/
    SCHATTR_SYMBOL_SIZE,SCHATTR_SYMBOL_SIZE,        /*    97          sch/schattr.hxx*/
    SCHATTR_HIDE_DATA_POINT_LEGEND_ENTRY, SCHATTR_HIDE_DATA_POINT_LEGEND_ENTRY,
    XATTR_LINE_FIRST, XATTR_LINE_LAST,              /*  1000 -  1016  svx/xdef.hxx   */
    XATTR_FILL_FIRST, XATTR_FILL_LAST,              /*  1018 -  1046  svx/xdef.hxx   */
    SDRATTR_3D_FIRST, SDRATTR_3D_LAST,              /*  1244 -  1334  svx/svddef.hxx */
    EE_ITEMS_START, EE_ITEMS_END,                   /*  3994 -  4037  editeng/eeitem.hxx */
    SID_ATTR_NUMBERFORMAT_VALUE, SID_ATTR_NUMBERFORMAT_INFO,    /* 10585 - 10585  svx/svxids.hrc */
    SID_ATTR_NUMBERFORMAT_SOURCE, SID_ATTR_NUMBERFORMAT_SOURCE, /* 11432          svx/svxids.hrc */
    SID_CHAR_DLG_PREVIEW_STRING, SID_CHAR_DLG_PREVIEW_STRING,
    0
};

const sal_uInt16 nTextLabelWhichPairs[] =
{
    SCHATTR_DATADESCR_START, SCHATTR_DATADESCR_END,
    SCHATTR_TEXT_DEGREES, SCHATTR_TEXT_DEGREES,
    SCHATTR_STYLE_SYMBOL, SCHATTR_STYLE_SYMBOL,
    SCHATTR_SYMBOL_BRUSH, SCHATTR_SYMBOL_BRUSH,
    SCHATTR_SYMBOL_SIZE, SCHATTR_SYMBOL_SIZE,
    XATTR_LINESTYLE, XATTR_LINECOLOR,
    XATTR_LINETRANSPARENCE, XATTR_LINETRANSPARENCE,
    EE_ITEMS_START, EE_ITEMS_END,
    SID_ATTR_NUMBERFORMAT_VALUE, SID_ATTR_NUMBERFORMAT_INFO,
    SID_ATTR_NUMBERFORMAT_SOURCE, SID_ATTR_NUMBERFORMAT_SOURCE,
    SID_CHAR_DLG_PREVIEW_STRING, SID_CHAR_DLG_PREVIEW_STRING,
    0
};

const sal_uInt16 nSeriesOptionsWhichPairs[] =
{
    SCHATTR_AXIS,SCHATTR_AXIS,                      /*    69          sch/schattr.hxx*/
    SCHATTR_BAR_OVERLAP,SCHATTR_BAR_CONNECT,         /*    98 - 100 (incl. SCHATTR_GAPWIDTH) */
    SCHATTR_GROUP_BARS_PER_AXIS,SCHATTR_AXIS_FOR_ALL_SERIES,
    0
};

// nDataPointWhichPairs + nSeriesOptionsWhichPairs
const sal_uInt16 nRowWhichPairs[] =
{
    SCHATTR_DATADESCR_START, SCHATTR_DATADESCR_END, /*     1 -     2  sch/schattr.hxx*/
    SCHATTR_TEXT_DEGREES, SCHATTR_TEXT_DEGREES,
    SCHATTR_STYLE_START,SCHATTR_STYLE_END,          /*    59 -    68  sch/schattr.hxx*/
    SCHATTR_AXIS,SCHATTR_AXIS,                      /*    69          sch/schattr.hxx*/
    SCHATTR_SYMBOL_BRUSH,SCHATTR_SYMBOL_BRUSH,      /*    94          sch/schattr.hxx*/
    SCHATTR_SYMBOL_SIZE,SCHATTR_SYMBOL_SIZE,        /*    97          sch/schattr.hxx*/
    SCHATTR_HIDE_DATA_POINT_LEGEND_ENTRY, SCHATTR_HIDE_DATA_POINT_LEGEND_ENTRY,
    SCHATTR_BAR_OVERLAP,SCHATTR_BAR_CONNECT,         /*    98 - 100 (incl. SCHATTR_GAPWIDTH) */
    SCHATTR_GROUP_BARS_PER_AXIS,SCHATTR_AXIS_FOR_ALL_SERIES,
    XATTR_LINE_FIRST, XATTR_LINE_LAST,              /*  1000 -  1016  svx/xdef.hxx   */
    XATTR_FILL_FIRST, XATTR_FILL_LAST,              /*  1018 -  1046  svx/xdef.hxx   */
    SDRATTR_3D_FIRST, SDRATTR_3D_LAST,              /*  1244 -  1334  svx/svddef.hxx */
    EE_ITEMS_START, EE_ITEMS_END,                   /*  3994 -  4037  editeng/eeitem.hxx */
    SID_ATTR_NUMBERFORMAT_VALUE, SID_ATTR_NUMBERFORMAT_INFO,    /* 10585 - 10585  svx/svxids.hrc */
    SID_ATTR_NUMBERFORMAT_SOURCE, SID_ATTR_NUMBERFORMAT_SOURCE, /* 11432          svx/svxids.hrc */
    SID_CHAR_DLG_PREVIEW_STRING, SID_CHAR_DLG_PREVIEW_STRING,
    0
};

const sal_uInt16 nStatWhichPairs[]=
{
    SCHATTR_STAT_START, SCHATTR_STAT_END,           //    45 -    52  sch/schattr.hxx
    SCHATTR_REGRESSION_START, SCHATTR_REGRESSION_END, // 108 -   109
    0
};

const sal_uInt16 nErrorBarWhichPairs[]=
{
    SCHATTR_STAT_START, SCHATTR_STAT_END,           //    45 -    52  sch/schattr.hxx
    XATTR_LINE_FIRST, XATTR_LINE_LAST,              //  1000 -  1016  svx/xdef.hxx
    0
};

// for CharacterProperties

const sal_uInt16 nCharacterPropertyWhichPairs[] =
{
    EE_ITEMS_START, EE_ITEMS_END,                             // Characters
    SID_CHAR_DLG_PREVIEW_STRING, SID_CHAR_DLG_PREVIEW_STRING, // Characters
    0
};

const sal_uInt16 nLinePropertyWhichPairs[] =
{
    XATTR_LINE_FIRST, XATTR_LINE_LAST,              //  1000 -  1016  svx/xdef.hxx
    0
};

const sal_uInt16 nLineAndFillPropertyWhichPairs[] =
{
    XATTR_LINE_FIRST, XATTR_LINE_LAST,              //  1000 -  1016  svx/xdef.hxx
    XATTR_FILL_FIRST, XATTR_FILL_LAST,              //  1000 -  1016  svx/xdef.hxx
    SDRATTR_SHADOW_FIRST, SDRATTR_SHADOW_LAST,      //  1067 -  1078  svx/svddef.hxx
    0
};

const sal_uInt16 nRegressionCurveWhichPairs[] =
{
    SCHATTR_REGRESSION_START, SCHATTR_REGRESSION_END, // 108 -   109
    XATTR_LINE_FIRST, XATTR_LINE_LAST,              //  1000 -  1016  svx/xdef.hxx
    0
};

const sal_uInt16 nRegEquationWhichPairs[] =
{
    XATTR_LINE_FIRST, XATTR_LINE_LAST,              //  1000 -  1016  svx/xdef.hxx
    XATTR_FILL_FIRST, XATTR_FILL_LAST,              //  1018 -  1046  svx/xdef.hxx
    SDRATTR_SHADOW_FIRST, SDRATTR_SHADOW_LAST,      //  1067 -  1078  svx/svddef.hxx
    EE_ITEMS_START, EE_ITEMS_END,                             // Characters
    SID_ATTR_NUMBERFORMAT_VALUE, SID_ATTR_NUMBERFORMAT_VALUE,   // 10585 - 10585  svx/svxids.hrc
    SID_CHAR_DLG_PREVIEW_STRING, SID_CHAR_DLG_PREVIEW_STRING, // Characters
    0
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
