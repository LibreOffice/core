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

#ifndef _DEFINES_HXX
#define _DEFINES_HXX
namespace binfilter {

#define SYMBOLSIZE 250

#define CHSTACK_NONE      0
#define CHSTACK_MINMAX    1
#define CHSTACK_OVERLAP   2

//CHART_AXIS_A,B,C gibt es nicht! (bzw. =X,,Y,Z)
#define CHART_AXIS_X    0
#define CHART_AXIS_Y    1
#define CHART_AXIS_Z    2

#define        CHOBJID_ANY                        0
#define        CHOBJID_TEXT                       1
#define        CHOBJID_AREA                       2
#define        CHOBJID_LINE                       3
#define        CHOBJID_DIAGRAM_AREA              10
#define        CHOBJID_TITLE_MAIN                11
#define        CHOBJID_TITLE_SUB                 12
#define        CHOBJID_DIAGRAM                   13
#define        CHOBJID_DIAGRAM_WALL              14
#define        CHOBJID_DIAGRAM_FLOOR             15
#define        CHOBJID_DIAGRAM_TITLE_X_AXIS      16
#define        CHOBJID_DIAGRAM_TITLE_Y_AXIS      17
#define        CHOBJID_DIAGRAM_TITLE_Z_AXIS      18
#define        CHOBJID_DIAGRAM_X_AXIS            19
#define        CHOBJID_DIAGRAM_Y_AXIS            20
#define        CHOBJID_DIAGRAM_Z_AXIS            21
#define        CHOBJID_DIAGRAM_X_GRID_MAIN       22
#define        CHOBJID_DIAGRAM_Y_GRID_MAIN       23
#define        CHOBJID_DIAGRAM_Z_GRID_MAIN       24
#define        CHOBJID_DIAGRAM_X_GRID_HELP       25
#define        CHOBJID_DIAGRAM_Y_GRID_HELP       26
#define        CHOBJID_DIAGRAM_Z_GRID_HELP       27
#define        CHOBJID_DIAGRAM_ROWGROUP          28
#define        CHOBJID_DIAGRAM_ROWS              29
#define        CHOBJID_DIAGRAM_ROWSLINE          30
#define        CHOBJID_DIAGRAM_DATA              31
#define        CHOBJID_DIAGRAM_DESCRGROUP        32
#define        CHOBJID_DIAGRAM_DESCR_ROW         33
#define        CHOBJID_LEGEND                    34
#define        CHOBJID_LEGEND_BACK               35
#define        CHOBJID_LEGEND_SYMBOL_ROW         36
#define        CHOBJID_LEGEND_SYMBOL_COL         37
#define        CHOBJID_DIAGRAM_DESCR_COL         38
#define        CHOBJID_DIAGRAM_DESCR_SYMBOL      39

//Na toll, doppelt vergeben, nie gesetzt und fürs undo abgefragt:
//ToDo: Weg damit!!!
#define        CHOBJID_DIAGRAM_X_AXIS_GROUP      38
#define        CHOBJID_DIAGRAM_Y_AXIS_GROUP      39
#define        CHOBJID_DIAGRAM_Z_AXIS_GROUP      40

#define        CHOBJID_DIAGRAM_NET               41
#define        CHOBJID_DIAGRAM_AVERAGEVALUE      42
#define        CHOBJID_DIAGRAM_ERROR             43
#define        CHOBJID_DIAGRAM_REGRESSION        45
#define        CHOBJID_DIAGRAM_STACKEDGROUP      46
#define        CHOBJID_DIAGRAM_STATISTICS_GROUP  48
#define        CHOBJID_DIAGRAM_X_GRID_MAIN_GROUP 49
#define        CHOBJID_DIAGRAM_Y_GRID_MAIN_GROUP 50
#define        CHOBJID_DIAGRAM_Z_GRID_MAIN_GROUP 51
#define        CHOBJID_DIAGRAM_X_GRID_HELP_GROUP 52
#define        CHOBJID_DIAGRAM_Y_GRID_HELP_GROUP 53
#define        CHOBJID_DIAGRAM_Z_GRID_HELP_GROUP 54
#define        CHOBJID_DIAGRAM_SPECIAL_GROUP     55
/*#define        CHOBJID_3D_PIE_BOTTOM             57
#define        CHOBJID_3D_PIE_SIDE               58
#define        CHOBJID_3D_PIE_TOP                59
*/
#define CHOBJID_DIAGRAM_STOCKLINE				 60
#define CHOBJID_DIAGRAM_STOCKRECT				 61
#define CHOBJID_DIAGRAM_STOCKLINE_GROUP			 62
#define CHOBJID_DIAGRAM_STOCKLOSS_GROUP			 63
#define CHOBJID_DIAGRAM_STOCKPLUS_GROUP			 64

#define        CHOBJID_DIAGRAM_A_AXIS            65 //2. Achse (x,y,z)
#define        CHOBJID_DIAGRAM_B_AXIS            66
#define        CHOBJID_DIAGRAM_C_AXIS            67
#define        CHOBJID_DIAGRAM_TITLE_A_AXIS      68 //Titel dazu
#define        CHOBJID_DIAGRAM_TITLE_B_AXIS      69
#define        CHOBJID_DIAGRAM_TITLE_C_AXIS      70

#define LINE_POINT_COUNT 9
#define ROW_COLOR_COUNT 12

#define NEW_DOC      0
#define DOC_LOADED   1

// Ab dieser Anzahl werden fuer die Achsenbeschriftung Zeilen zusammengefasst
#define CONDENSE_COLUMNS_THRESHOLD               35000

#define CHSPECIAL_TRACELINES 8    //#50149#

} //namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
