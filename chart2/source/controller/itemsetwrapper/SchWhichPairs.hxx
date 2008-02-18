/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SchWhichPairs.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: rt $ $Date: 2008-02-18 15:56:12 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef CHART_SCHWHICHPAIRS_HXX
#define CHART_SCHWHICHPAIRS_HXX

#ifndef _SVX_SVXIDS_HRC
#include <svx/svxids.hrc>
#endif

#ifndef _XDEF_HXX
#include <svx/xdef.hxx>
#endif
#ifndef _SVDDEF_HXX
#include <svx/svddef.hxx>
#endif
#ifndef _EEITEM_HXX
#include <svx/eeitem.hxx>
#endif

#include "chartview/ChartSfxItemIds.hxx"

namespace
{

const USHORT nTitleWhichPairs[] =
{
    SCHATTR_TEXT_STACKED, SCHATTR_TEXT_STACKED,       //     4          sch/schattr.hxx
    SCHATTR_TEXT_DEGREES,SCHATTR_TEXT_DEGREES,      //    53          sch/schattr.hxx
    XATTR_LINE_FIRST, XATTR_LINE_LAST,              //  1000 -  1016  svx/xdef.hxx
    XATTR_FILL_FIRST, XATTR_FILL_LAST,              //  1018 -  1046  svx/xdef.hxx
    SDRATTR_SHADOW_FIRST, SDRATTR_SHADOW_LAST,      //  1067 -  1078  svx/svddef.hxx
    EE_ITEMS_START, EE_ITEMS_END,                   //  3994 -  4037  svx/eeitem.hxx
    0
};

const USHORT nAxisWhichPairs[] =
{
    XATTR_LINE_FIRST, XATTR_LINE_LAST,                          //  1000 -  1016  svx/xdef.hxx
    EE_ITEMS_START, EE_ITEMS_END,                               //  3994 -  4037  svx/eeitem.hxx
    SID_ATTR_NUMBERFORMAT_VALUE, SID_ATTR_NUMBERFORMAT_VALUE,   // 10585 - 10585  svx/svxids.hrc
    SID_ATTR_NUMBERFORMAT_SOURCE, SID_ATTR_NUMBERFORMAT_SOURCE, // 11432          svx/svxids.hrc
    SCHATTR_AXISTYPE, SCHATTR_AXISTYPE,                         //    39          sch/schattr.hxx
    SCHATTR_TEXT_START, SCHATTR_TEXT_END,                       //     4 -     6  sch/schattr.hxx
    SCHATTR_TEXT_DEGREES,SCHATTR_TEXT_DEGREES,                  //    53          sch/schattr.hxx
    SCHATTR_TEXT_OVERLAP, SCHATTR_TEXT_OVERLAP,                 //    54          sch/schattr.hxx
    SCHATTR_AXIS_START, SCHATTR_AXIS_END,                       //    70 -    95  sch/schattr.hxx
    SCHATTR_TEXTBREAK, SCHATTR_TEXTBREAK,                       // 30587          sch/schattr.hxx
    0
};

const USHORT nAllAxisWhichPairs[] =
{
    XATTR_LINE_FIRST, XATTR_LINE_LAST,              //  1000 -  1016  svx/xdef.hxx
    EE_ITEMS_START, EE_ITEMS_END,                   //  3994 -  4037  svx/eeitem.hxx
    SCHATTR_TEXT_START, SCHATTR_TEXT_END,           //     4 -     6  sch/schattr.hxx
    SCHATTR_TEXT_DEGREES,SCHATTR_TEXT_DEGREES,      //    53          sch/schattr.hxx
    SCHATTR_TEXT_OVERLAP, SCHATTR_TEXT_OVERLAP,     //    54          sch/schattr.hxx
    SCHATTR_AXIS_SHOWDESCR, SCHATTR_AXIS_SHOWDESCR, //    85          sch/schattr.hxx
    SCHATTR_TEXTBREAK, SCHATTR_TEXTBREAK,           // 30587          sch/schattr.hxx
    0
};

const USHORT nGridWhichPairs[] =
{
    XATTR_LINE_FIRST, XATTR_LINE_LAST,              //  1000 -  1016  svx/xdef.hxx
    0
};

const USHORT nChartWhichPairs[] =
{
    SCHATTR_STYLE_START,SCHATTR_STYLE_END,          //    59 -    68  sch/schattr.hxx
    0
};

const USHORT nDiagramAreaWhichPairs[] =
{
    XATTR_LINE_FIRST, XATTR_LINE_LAST,              //  1000 -  1016  svx/xdef.hxx
    XATTR_FILL_FIRST, XATTR_FILL_LAST,              //  1018 -  1046  svx/xdef.hxx
    0
};

const USHORT nAreaAndChartWhichPairs[] =   // pairs for chart AND area
{
    XATTR_LINE_FIRST, XATTR_LINE_LAST,              //  1000 -  1016  svx/xdef.hxx
    XATTR_FILL_FIRST, XATTR_FILL_LAST,              //  1018 -  1046  svx/xdef.hxx
    SCHATTR_STYLE_START,SCHATTR_STYLE_END,          //    59 -    68  sch/schattr.hxx
    0
};

const USHORT nLegendWhichPairs[] =
{
    XATTR_LINE_FIRST, XATTR_LINE_LAST,              //  1000 -  1016  svx/xdef.hxx
    XATTR_FILL_FIRST, XATTR_FILL_LAST,              //  1018 -  1046  svx/xdef.hxx
    SDRATTR_SHADOW_FIRST, SDRATTR_SHADOW_LAST,      //  1067 -  1078  svx/svddef.hxx
    EE_ITEMS_START, EE_ITEMS_END,                   //  3994 -  4037  svx/eeitem.hxx
    SCHATTR_LEGEND_START, SCHATTR_LEGEND_END,       //     3 -     3  sch/schattr.hxx
    0
};

const USHORT nDataLabelWhichPairs[] =
{
    SCHATTR_DATADESCR_START, SCHATTR_DATADESCR_END,
    SID_ATTR_NUMBERFORMAT_VALUE, SID_ATTR_NUMBERFORMAT_INFO,    /* 10585 - 10585  svx/svxids.hrc */ \
    SID_ATTR_NUMBERFORMAT_SOURCE, SID_ATTR_NUMBERFORMAT_SOURCE, /* 11432          svx/svxids.hrc */ \
    SCHATTR_PERCENT_NUMBERFORMAT_VALUE, SCHATTR_PERCENT_NUMBERFORMAT_VALUE,   /*      40          sch/schattr.hxx*/ \
    SCHATTR_PERCENT_NUMBERFORMAT_SOURCE, SCHATTR_PERCENT_NUMBERFORMAT_SOURCE, /*      41          sch/schattr.hxx*/ \
    SCHATTR_DATADESCR_PLACEMENT, SCHATTR_DATADESCR_PLACEMENT,
    SCHATTR_DATADESCR_AVAILABLE_PLACEMENTS, SCHATTR_DATADESCR_AVAILABLE_PLACEMENTS,
    0
};

#define CHART_POINT_WHICHPAIRS  \
    XATTR_LINE_FIRST, XATTR_LINE_LAST,              /*  1000 -  1016  svx/xdef.hxx   */ \
    XATTR_FILL_FIRST, XATTR_FILL_LAST,              /*  1018 -  1046  svx/xdef.hxx   */ \
    EE_ITEMS_START, EE_ITEMS_END,                   /*  3994 -  4037  svx/eeitem.hxx */ \
    SCHATTR_DATADESCR_START, SCHATTR_DATADESCR_END, /*     1 -     2  sch/schattr.hxx*/ \
    SID_ATTR_NUMBERFORMAT_VALUE, SID_ATTR_NUMBERFORMAT_INFO,    /* 10585 - 10585  svx/svxids.hrc */ \
    SID_ATTR_NUMBERFORMAT_SOURCE, SID_ATTR_NUMBERFORMAT_SOURCE, /* 11432          svx/svxids.hrc */ \
    SCHATTR_PERCENT_NUMBERFORMAT_VALUE, SCHATTR_PERCENT_NUMBERFORMAT_VALUE,   /*      40          sch/schattr.hxx*/ \
    SCHATTR_PERCENT_NUMBERFORMAT_SOURCE, SCHATTR_PERCENT_NUMBERFORMAT_SOURCE, /*      41          sch/schattr.hxx*/ \
    SCHATTR_DATADESCR_PLACEMENT, SCHATTR_DATADESCR_PLACEMENT, \
    SCHATTR_DATADESCR_AVAILABLE_PLACEMENTS, SCHATTR_DATADESCR_AVAILABLE_PLACEMENTS, \
    SCHATTR_STYLE_START,SCHATTR_STYLE_END,          /*    59 -    68  sch/schattr.hxx*/ \
    SCHATTR_SYMBOL_BRUSH,SCHATTR_SYMBOL_BRUSH,      /*    94          sch/schattr.hxx*/ \
    SCHATTR_SYMBOL_SIZE,SCHATTR_SYMBOL_SIZE,        /*    97          sch/schattr.hxx*/ \
    SDRATTR_3D_FIRST, SDRATTR_3D_LAST               /*  1244 -  1334  svx/svddef.hxx */

const USHORT nDataPointWhichPairs[] =
{
    CHART_POINT_WHICHPAIRS,
    0
};

#define CHART_SERIES_OPTIONS_WHICHPAIRS \
    SCHATTR_AXIS,SCHATTR_AXIS,                      /*    69          sch/schattr.hxx*/ \
    SCHATTR_BAR_OVERLAP,SCHATTR_BAR_CONNECT,         /*    98 - 100 (incl. SCHATTR_GAPWIDTH) */  \
    SCHATTR_GROUP_BARS_PER_AXIS,SCHATTR_AXIS_FOR_ALL_SERIES, \
    SCHATTR_STARTING_ANGLE,SCHATTR_STARTING_ANGLE, \
    SCHATTR_CLOCKWISE,SCHATTR_CLOCKWISE \

const USHORT nSeriesOptionsWhichPairs[] =
{
    CHART_SERIES_OPTIONS_WHICHPAIRS,
    0
};

const USHORT nRowWhichPairs[] =
{
    CHART_POINT_WHICHPAIRS,
    SCHATTR_STAT_START, SCHATTR_STAT_END,           /*    45 -    52  sch/schattr.hxx*/ \
    CHART_SERIES_OPTIONS_WHICHPAIRS,
    0
};

const USHORT nAreaWhichPairs[] =
{
    XATTR_LINE_FIRST, XATTR_LINE_LAST,              //  1000 -  1016  svx/xdef.hxx
    XATTR_FILL_FIRST, XATTR_FILL_LAST,              //  1000 -  1016  svx/xdef.hxx
    SDRATTR_SHADOW_FIRST, SDRATTR_SHADOW_LAST,      //  1067 -  1078  svx/svddef.hxx
    0
};

const USHORT nTextWhichPairs[] =
{
    EE_ITEMS_START, EE_ITEMS_END,                   //  3994 -  4037  svx/eeitem.hxx
    SCHATTR_TEXT_STACKED, SCHATTR_TEXT_STACKED,       //     4          sch/schattr.hxx
    SCHATTR_TEXT_DEGREES,SCHATTR_TEXT_DEGREES,      //    53          sch/schattr.hxx
    0
};

const USHORT nTextOrientWhichPairs[] =
{
    EE_ITEMS_START, EE_ITEMS_END,                   //  3994 -  4037  svx/eeitem.hxx
    SCHATTR_TEXT_STACKED, SCHATTR_TEXT_STACKED,       //     4          sch/schattr.hxx
    SCHATTR_TEXT_DEGREES,SCHATTR_TEXT_DEGREES,      //    53          sch/schattr.hxx
    0
};

const USHORT nStatWhichPairs[]=
{
    SCHATTR_STAT_START, SCHATTR_STAT_END,           //    45 -    52  sch/schattr.hxx
    SCHATTR_REGRESSION_START, SCHATTR_REGRESSION_END,
    0
};

// for CharacterProperties

const USHORT nCharacterPropertyWhichPairs[] =
{
    EE_ITEMS_START, EE_ITEMS_END,                   //  3994 -  4037  svx/eeitem.hxx
    0
};

const USHORT nLinePropertyWhichPairs[] =
{
    XATTR_LINE_FIRST, XATTR_LINE_LAST,              //  1000 -  1016  svx/xdef.hxx
    0
};

const USHORT nFillPropertyWhichPairs[] =
{
    XATTR_FILL_FIRST, XATTR_FILL_LAST,              //  1000 -  1016  svx/xdef.hxx
    SDRATTR_SHADOW_FIRST, SDRATTR_SHADOW_LAST,      //  1067 -  1078  svx/svddef.hxx
    0
};

const USHORT nLineAndFillPropertyWhichPairs[] =
{
    XATTR_LINE_FIRST, XATTR_LINE_LAST,              //  1000 -  1016  svx/xdef.hxx
    XATTR_FILL_FIRST, XATTR_FILL_LAST,              //  1000 -  1016  svx/xdef.hxx
    SDRATTR_SHADOW_FIRST, SDRATTR_SHADOW_LAST,      //  1067 -  1078  svx/svddef.hxx
    0
};

const USHORT nChartStyleWhichPairs[] =
{
    SCHATTR_DIAGRAM_STYLE,                SCHATTR_DIAGRAM_STYLE,
    SCHATTR_STYLE_SHAPE,                  SCHATTR_STYLE_SHAPE,
    SCHATTR_NUM_OF_LINES_FOR_BAR,         SCHATTR_NUM_OF_LINES_FOR_BAR,
    SCHATTR_SPLINE_ORDER,                 SCHATTR_SPLINE_ORDER,
    SCHATTR_SPLINE_RESOLUTION,            SCHATTR_SPLINE_RESOLUTION,
    0
};

const USHORT nRegressionCurveWhichPairs[] =
{
    SCHATTR_REGRESSION_START, SCHATTR_REGRESSION_END, // 108 -   109
    SCHATTR_STAT_REGRESSTYPE, SCHATTR_STAT_REGRESSTYPE,
    XATTR_LINE_FIRST, XATTR_LINE_LAST,              //  1000 -  1016  svx/xdef.hxx
    0
};

const USHORT nRegEquationWhichPairs[] =
{
    XATTR_LINE_FIRST, XATTR_LINE_LAST,              //  1000 -  1016  svx/xdef.hxx
    XATTR_FILL_FIRST, XATTR_FILL_LAST,              //  1018 -  1046  svx/xdef.hxx
    SDRATTR_SHADOW_FIRST, SDRATTR_SHADOW_LAST,      //  1067 -  1078  svx/svddef.hxx
    EE_ITEMS_START, EE_ITEMS_END,                   //  3994 -  4037  svx/eeitem.hxx
    SID_ATTR_NUMBERFORMAT_VALUE, SID_ATTR_NUMBERFORMAT_VALUE,   // 10585 - 10585  svx/svxids.hrc
    0
};

} //  anonymous namespace

// CHART_SCHWHICHPAIRS_HXX
#endif
