/*************************************************************************
 *
 *  $RCSfile: SchWhichPairs.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: bm $ $Date: 2003-10-06 09:58:28 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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

#include "SchSfxItemIds.hxx"
#include "SchSlotIds.hxx"

namespace
{

const USHORT nTitleWhichPairs[] =
{
//  SCHATTR_TEXT_ORIENT, SCHATTR_TEXT_ORIENT,       //     4          sch/schattr.hxx
    SCHATTR_TEXT_STACKED, SCHATTR_TEXT_STACKED,       //     4          sch/schattr.hxx
    SCHATTR_TEXT_DEGREES,SCHATTR_TEXT_DEGREES,      //    53          sch/schattr.hxx
    SCHATTR_USER_DEFINED_ATTR, SCHATTR_USER_DEFINED_ATTR, //     100  sch/schattr.hxx
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
    SCHATTR_USER_DEFINED_ATTR, SCHATTR_USER_DEFINED_ATTR,       //   100          sch/schattr.hxx
    // SID_TEXTBREAK, SID_TEXTBREAK,                                // 30587          sch/app.hrc
    SCHATTR_TEXTBREAK, SCHATTR_TEXTBREAK,                       // 30587          sch/schattr.hxx
    0
};

const USHORT nAllAxisWhichPairs[] =
{
    XATTR_LINE_FIRST, XATTR_LINE_LAST,              //  1000 -  1016  svx/xdef.hxx
    EE_ITEMS_START, EE_ITEMS_END,                   //  3994 -  4037  svx/eeitem.hxx
//  SCHATTR_TEXT_ORIENT, SCHATTR_TEXT_ORIENT,       //    4           sch/schattr.hxx
    SCHATTR_TEXT_START, SCHATTR_TEXT_END,           //     4 -     6  sch/schattr.hxx
    SCHATTR_TEXT_DEGREES,SCHATTR_TEXT_DEGREES,      //    53          sch/schattr.hxx
    SCHATTR_TEXT_OVERLAP, SCHATTR_TEXT_OVERLAP,     //    54          sch/schattr.hxx
    SCHATTR_AXIS_SHOWDESCR, SCHATTR_AXIS_SHOWDESCR, //    85          sch/schattr.hxx
    // SID_TEXTBREAK, SID_TEXTBREAK,                    // 30587          sch/app.hrc
    SCHATTR_TEXTBREAK, SCHATTR_TEXTBREAK,           // 30587          sch/schattr.hxx
    0
};

const USHORT nGridWhichPairs[] =
{
    XATTR_LINE_FIRST, XATTR_LINE_LAST,              //  1000 -  1016  svx/xdef.hxx
    SCHATTR_USER_DEFINED_ATTR, SCHATTR_USER_DEFINED_ATTR, //     100  sch/schattr.hxx
    0
};

const USHORT nChartWhichPairs[] =
{
    SCHATTR_STYLE_START,SCHATTR_STYLE_END,          //    59 -    68  sch/schattr.hxx
    SCHATTR_USER_DEFINED_ATTR, SCHATTR_USER_DEFINED_ATTR, //     100  sch/schattr.hxx
    0
};

const USHORT nDiagramAreaWhichPairs[] =
{
    XATTR_LINE_FIRST, XATTR_LINE_LAST,              //  1000 -  1016  svx/xdef.hxx
    XATTR_FILL_FIRST, XATTR_FILL_LAST,              //  1018 -  1046  svx/xdef.hxx
    SCHATTR_USER_DEFINED_ATTR, SCHATTR_USER_DEFINED_ATTR, //     100  sch/schattr.hxx
    0
};

const USHORT nAreaAndChartWhichPairs[] =   // pairs for chart AND area
{
    XATTR_LINE_FIRST, XATTR_LINE_LAST,              //  1000 -  1016  svx/xdef.hxx
    XATTR_FILL_FIRST, XATTR_FILL_LAST,              //  1018 -  1046  svx/xdef.hxx
    SCHATTR_STYLE_START,SCHATTR_STYLE_END,          //    59 -    68  sch/schattr.hxx
    SCHATTR_USER_DEFINED_ATTR, SCHATTR_USER_DEFINED_ATTR, //     100  sch/schattr.hxx
    0
};

const USHORT nLegendWhichPairs[] =
{
    XATTR_LINE_FIRST, XATTR_LINE_LAST,              //  1000 -  1016  svx/xdef.hxx
    XATTR_FILL_FIRST, XATTR_FILL_LAST,              //  1018 -  1046  svx/xdef.hxx
    SDRATTR_SHADOW_FIRST, SDRATTR_SHADOW_LAST,      //  1067 -  1078  svx/svddef.hxx
    EE_ITEMS_START, EE_ITEMS_END,                   //  3994 -  4037  svx/eeitem.hxx
    SCHATTR_LEGEND_START, SCHATTR_LEGEND_END,       //     3 -     3  sch/schattr.hxx
    SCHATTR_USER_DEFINED_ATTR, SCHATTR_USER_DEFINED_ATTR, //     100  sch/schattr.hxx
    0
};

const USHORT nDataLabelWhichPairs[] =
{
    SCHATTR_DATADESCR_START, SCHATTR_DATADESCR_END,
    0
};

#define CHART_ROW_WHICHPAIRS    \
    XATTR_LINE_FIRST, XATTR_LINE_LAST,              /*  1000 -  1016  svx/xdef.hxx   */ \
    XATTR_FILL_FIRST, XATTR_FILL_LAST,              /*  1018 -  1046  svx/xdef.hxx   */ \
    EE_ITEMS_START, EE_ITEMS_END,                   /*  3994 -  4037  svx/eeitem.hxx */ \
    SCHATTR_DATADESCR_START, SCHATTR_DATADESCR_END, /*     1 -     2  sch/schattr.hxx*/ \
    SCHATTR_DUMMY0, SCHATTR_DUMMY0,                 /*    40          sch/schattr.hxx*/ \
    SCHATTR_DUMMY1, SCHATTR_DUMMY1,                 /*    41          sch/schattr.hxx*/ \
    SCHATTR_STAT_START, SCHATTR_STAT_END,           /*    45 -    52  sch/schattr.hxx*/ \
    SCHATTR_STYLE_START,SCHATTR_STYLE_END,          /*    59 -    68  sch/schattr.hxx*/ \
    SCHATTR_AXIS,SCHATTR_AXIS,                      /*    69          sch/schattr.hxx*/ \
    SCHATTR_SYMBOL_BRUSH,SCHATTR_SYMBOL_BRUSH,      /*    96          sch/schattr.hxx*/ \
    SCHATTR_SYMBOL_SIZE,SCHATTR_USER_DEFINED_ATTR,  /*    99 -   100  sch/schattr.hxx*/ \
    SDRATTR_3D_FIRST, SDRATTR_3D_LAST               /*  1244 -  1334  svx/svddef.hxx */

const USHORT nRowWhichPairs[] =
{
    CHART_ROW_WHICHPAIRS,
    0
};

const USHORT nAreaWhichPairs[] =
{
    XATTR_LINE_FIRST, XATTR_LINE_LAST,              //  1000 -  1016  svx/xdef.hxx
    XATTR_FILL_FIRST, XATTR_FILL_LAST,              //  1000 -  1016  svx/xdef.hxx
    SDRATTR_SHADOW_FIRST, SDRATTR_SHADOW_LAST,      //  1067 -  1078  svx/svddef.hxx
    SCHATTR_USER_DEFINED_ATTR, SCHATTR_USER_DEFINED_ATTR, //     100  sch/schattr.hxx
    0
};

const USHORT nTextWhichPairs[] =
{
    EE_ITEMS_START, EE_ITEMS_END,                   //  3994 -  4037  svx/eeitem.hxx
//  SCHATTR_TEXT_ORIENT, SCHATTR_TEXT_ORIENT,       //     4          sch/schattr.hxx
    SCHATTR_TEXT_STACKED, SCHATTR_TEXT_STACKED,       //     4          sch/schattr.hxx
    SCHATTR_TEXT_DEGREES,SCHATTR_TEXT_DEGREES,      //    53          sch/schattr.hxx
    SCHATTR_USER_DEFINED_ATTR, SCHATTR_USER_DEFINED_ATTR, //     100  sch/schattr.hxx
    0
};

const USHORT nTextOrientWhichPairs[] =
{
    EE_ITEMS_START, EE_ITEMS_END,                   //  3994 -  4037  svx/eeitem.hxx
//  SCHATTR_TEXT_ORIENT, SCHATTR_TEXT_ORIENT,       //     4          sch/schattr.hxx
    SCHATTR_TEXT_STACKED, SCHATTR_TEXT_STACKED,       //     4          sch/schattr.hxx
    SCHATTR_TEXT_DEGREES,SCHATTR_TEXT_DEGREES,      //    53          sch/schattr.hxx
    0
};

const USHORT nStatWhichPairs[]=
{
    SCHATTR_STAT_START, SCHATTR_STAT_END,           //    45 -    52  sch/schattr.hxx
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


} //  anonymous namespace

// CHART_SCHWHICHPAIRS_HXX
#endif
