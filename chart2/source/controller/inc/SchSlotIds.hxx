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
#ifndef SCH_SLOTIDS_HXX
#define SCH_SLOTIDS_HXX

// header for SID_OBJ1_START
#include <sfx2/sfxsids.hrc>

// SID_OBJ1_START == 30000 + 512
//not used as slot in old chart
//#define SID_COPYFORMAT            SID_OBJ1_START
//not used as slot in old chart
//#define SID_PASTEFORMAT       (SID_OBJ1_START +  1)

//still used as help id
#define SID_DIAGRAM_DATA        (SID_OBJ1_START +  2)
//only used as help id
#define SID_INSERT_TITLE        (SID_OBJ1_START +  3)
//only used as help id
#define SID_INSERT_CHART_LEGEND (SID_OBJ1_START +  4)
//only used as help id
#define SID_INSERT_DESCRIPTION  (SID_OBJ1_START +  5)
//only used as help id
#define SID_INSERT_AXIS         (SID_OBJ1_START +  6)
//some how used as slot in old chart
//#define SID_DATA_ROW_POINT        (SID_OBJ1_START +  7)
//menu parent id and helpid
//MENUCHANGE FORMATMENU #define SID_CHART_TITLE         (SID_OBJ1_START +  8)
//still used as slot id
//MENUCHANGE FORMATMENU #define SID_LEGEND              (SID_OBJ1_START +  9)
//menu parent id and helpid
//MENUCHANGE FORMATMENU #define SID_DIAGRAM_AXIS        (SID_OBJ1_START + 10)
//menu parent id and helpid
//MENUCHANGE FORMATMENU #define SID_DIAGRAM_GRID        (SID_OBJ1_START + 11)

/*
//MENUCHANGE
//still used as slot id
#define SID_DIAGRAM_WALL        (SID_OBJ1_START + 12)
//still used as slot id
#define SID_DIAGRAM_FLOOR       (SID_OBJ1_START + 13)
//still used as slot id
#define SID_DIAGRAM_AREA        (SID_OBJ1_START + 14)
*/

//only used as help id
#define SID_3D_VIEW             (SID_OBJ1_START + 15)
//still used as slot id
#define SID_DIAGRAM_TYPE        (SID_OBJ1_START + 16)
//used as slot in old chart
//#define SID_TOGGLE_TITLE        (SID_OBJ1_START + 17)
//used as slot in old chart
//#define SID_TOGGLE_LEGEND     (SID_OBJ1_START + 18)
//used as slot in old chart
//#define SID_TOGGLE_AXIS_TITLE (SID_OBJ1_START + 19)
//used as slot in old chart
//#define SID_TOGGLE_AXIS_DESCR (SID_OBJ1_START + 20)      // 30532
//used as slot in old chart
//#define SID_TOGGLE_GRID_HORZ  (SID_OBJ1_START + 21)
//used as slot in old chart
//#define SID_TOGGLE_GRID_VERT  (SID_OBJ1_START + 22)
//used as slot in old chart
//#define SID_DATA_IN_ROWS      (SID_OBJ1_START + 23)
//used as slot in old chart
//#define SID_DATA_IN_COLUMNS       (SID_OBJ1_START + 24)
//used as slot in old chart
//#define SID_TOOL_SELECT       (SID_OBJ1_START + 25)
//some how used as slot in old chart
//#define SID_CONTEXT_TYPE      (SID_OBJ1_START + 26)
//used as slot in old chart
//#define SID_NEW_ARRANGEMENT     (SID_OBJ1_START + 27)     //  30539
//only used as help id
#define SID_INSERT_GRIDS        (SID_OBJ1_START + 28)

//not used as slot in old chart
//#define SID_MAINTITLE         (SID_OBJ1_START + 29)
//not used as slot in old chart
//#define SID_SUBTITLE          (SID_OBJ1_START + 30)
//not used as slot in old chart
//#define SID_XAXIS_TITLE         (SID_OBJ1_START + 31)
//not used as slot in old chart
//#define SID_YAXIS_TITLE         (SID_OBJ1_START + 32)
//not used as slot in old chart
//#define SID_ZAXIS_TITLE         (SID_OBJ1_START + 33)

/*
//used as slot in old chart
#ifdef SID_UPDATE
#undef SID_UPDATE
#endif
#define SID_UPDATE              (SID_OBJ1_START + 34)
*/

//not used as slot in old chart
//#define SID_DATA                (SID_OBJ1_START + 35)
//not used as slot in old chart
//#define SID_ADD_COLUMN          (SID_OBJ1_START + 36)
//not used as slot in old chart
//#define SID_ADD_ROW             (SID_OBJ1_START + 37)

//still used as slot id
//MENUCHANGE FORMATMENU #define SID_DIAGRAM_AXIS_X      (SID_OBJ1_START + 40)
//still used as slot id
//MENUCHANGE FORMATMENU #define SID_DIAGRAM_AXIS_Y      (SID_OBJ1_START + 41)
//still used as slot id
//MENUCHANGE FORMATMENU #define SID_DIAGRAM_AXIS_Z      (SID_OBJ1_START + 42)
//still used as slot id
//MENUCHANGE FORMATMENU #define SID_DIAGRAM_AXIS_ALL    (SID_OBJ1_START + 43)
//only used as help id
#define SID_INSERT_STATISTICS   (SID_OBJ1_START + 44)
//still used as slot id
//MENUCHANGE FORMATMENU #define SID_DIAGRAM_TITLE_MAIN  (SID_OBJ1_START + 45)
//still used as slot id
//MENUCHANGE FORMATMENU #define SID_DIAGRAM_TITLE_SUB   (SID_OBJ1_START + 46)
//still used as slot id
//MENUCHANGE FORMATMENU #define SID_DIAGRAM_TITLE_X     (SID_OBJ1_START + 47)
//still used as slot id
//MENUCHANGE FORMATMENU #define SID_DIAGRAM_TITLE_Y     (SID_OBJ1_START + 48)
//still used as slot id
//MENUCHANGE FORMATMENU #define SID_DIAGRAM_TITLE_Z     (SID_OBJ1_START + 49)
//still used as slot id
//MENUCHANGE FORMATMENU #define SID_DIAGRAM_TITLE_ALL   (SID_OBJ1_START + 50)
//still used as slot id
//MENUCHANGE FORMATMENU #define SID_DIAGRAM_GRID_Y_MAIN (SID_OBJ1_START + 51)
//still used as slot id
//MENUCHANGE FORMATMENU #define SID_DIAGRAM_GRID_X_MAIN (SID_OBJ1_START + 52)
//still used as slot id
//MENUCHANGE FORMATMENU #define SID_DIAGRAM_GRID_Z_MAIN (SID_OBJ1_START + 53)
//still used as slot id
//MENUCHANGE FORMATMENU #define SID_DIAGRAM_GRID_ALL    (SID_OBJ1_START + 54)

//used as slot in old chart
//#define SID_HAS_X_DESCR           (SID_OBJ1_START + 55)
//used as slot in old chart
//#define SID_HAS_Y_DESCR           (SID_OBJ1_START + 56)
//used as slot in old chart
//#define SID_HAS_Z_DESCR           (SID_OBJ1_START + 57)

//#define SID_DIAGRAM_OBJECTS     (SID_OBJ1_START + 60)

//not used as slot in old chart
//#define SID_HAS_X_TITLE           (SID_OBJ1_START + 61)
//not used as slot in old chart
//#define SID_HAS_Y_TITLE           (SID_OBJ1_START + 62)
//not used as slot in old chart
//#define SID_HAS_Z_TITLE           (SID_OBJ1_START + 63)
//not used as slot in old chart
//#define SID_HAS_MAIN_TITLE        (SID_OBJ1_START + 64)
//not used as slot in old chart
//#define SID_HAS_SUB_TITLE     (SID_OBJ1_START + 65)

//still used as slot id
//MENUCHANGE FORMATMENU #define SID_DIAGRAM_GRID_Y_HELP (SID_OBJ1_START + 66)
//still used as slot id
//MENUCHANGE FORMATMENU #define SID_DIAGRAM_GRID_X_HELP (SID_OBJ1_START + 67)
//still used as slot id
//MENUCHANGE FORMATMENU #define SID_DIAGRAM_GRID_Z_HELP (SID_OBJ1_START + 68)

//not used as slot in old chart
//#define SID_DIAGRAM_DATA_WIN  (SID_OBJ1_START + 70)

//used as slot in old chart
//#define SID_DIAGRAM_ERROR      (SID_OBJ1_START + 71)
//used as slot in old chart
//#define SID_DIAGRAM_AVERAGEVALUE (SID_OBJ1_START + 72)
//used as slot in old chart
//#define SID_DIAGRAM_REGRESSION   (SID_OBJ1_START + 73)
//used as slot in old chart
//#define SID_SCALE_TEXT           (SID_OBJ1_START + 74)
//not used as slot in old chart
//#define SID_TEXTBREAK            (SID_OBJ1_START + 75)

//not used as slot in old chart
//#define SID_DIAGRAM_DATADESCRIPTION  (SID_OBJ1_START + 76)
//not used as slot in old chart
//#define SID_DIAGRAM_POSLEGEND        (SID_OBJ1_START + 77)
//not used as slot in old chart
//#define SID_DIAGRAM_DEFAULTCOLORS    (SID_OBJ1_START + 78)
//not used as slot in old chart
//#define SID_DIAGRAM_BARWIDTH         (SID_OBJ1_START + 79)
//not used as slot in old chart
//#define SID_DIAGRAM_NUMLINES         (SID_OBJ1_START + 80)

//menu parent id and helpid
//#define SID_ROW_POSITION         (SID_OBJ1_START + 81)
//still used as slot id
//#define SID_ROW_MOREFRONT        (SID_OBJ1_START + 82)
//still used as slot id
//#define SID_ROW_MOREBACK         (SID_OBJ1_START + 83)

//not used as slot in old chart
//#define SID_TITLE_TEXT            (SID_OBJ1_START + 100)
//somehow used as slot in old chart
//#define SID_DIAGRAM_STOCK_LINE  (SID_OBJ1_START + 101)
//used as slot in old chart
//#define SID_DIAGRAM_STOCK_LOSS  (SID_OBJ1_START + 102)
//used as slot in old chart
//#define SID_DIAGRAM_STOCK_PLUS  (SID_OBJ1_START + 103)

//still used as slot id
//MENUCHANGE FORMATMENU #define SID_DIAGRAM_AXIS_A      (SID_OBJ1_START + 104)
//still used as slot id
//MENUCHANGE FORMATMENU #define SID_DIAGRAM_AXIS_B      (SID_OBJ1_START + 105)
//used as slot in old chart
//#define SID_DIAGRAM_AXIS_C      (SID_OBJ1_START + 106)

// Reserved till (SID_OBJ1_START + 110)

#endif
// SCH_SLOTIDS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
