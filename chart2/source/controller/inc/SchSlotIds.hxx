/*************************************************************************
 *
 *  $RCSfile: SchSlotIds.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: bm $ $Date: 2003-10-06 09:58:27 $
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
#ifndef SCH_SLOTIDS_HXX
#define SCH_SLOTIDS_HXX

// header for SID_OBJ1_START
#ifndef _SFXSIDS_HRC
#include <sfx2/sfxsids.hrc>
#endif

// SID_OBJ1_START == 30000 + 512
#define SID_COPYFORMAT          SID_OBJ1_START

#define SID_PASTEFORMAT         (SID_OBJ1_START +  1)
#define SID_DIAGRAM_DATA        (SID_OBJ1_START +  2)
#define SID_INSERT_TITLE        (SID_OBJ1_START +  3)
#define SID_INSERT_CHART_LEGEND (SID_OBJ1_START +  4)
#define SID_INSERT_DESCRIPTION  (SID_OBJ1_START +  5)
#define SID_INSERT_AXIS         (SID_OBJ1_START +  6)
#define SID_DATA_ROW_POINT      (SID_OBJ1_START +  7)
#define SID_CHART_TITLE         (SID_OBJ1_START +  8)
#define SID_LEGEND              (SID_OBJ1_START +  9)
#define SID_DIAGRAM_AXIS        (SID_OBJ1_START + 10)
#define SID_DIAGRAM_GRID        (SID_OBJ1_START + 11)
#define SID_DIAGRAM_WALL        (SID_OBJ1_START + 12)
#define SID_DIAGRAM_FLOOR       (SID_OBJ1_START + 13)
#define SID_DIAGRAM_AREA        (SID_OBJ1_START + 14)
#define SID_3D_VIEW             (SID_OBJ1_START + 15)
#define SID_DIAGRAM_TYPE        (SID_OBJ1_START + 16)
#define SID_TOGGLE_TITLE        (SID_OBJ1_START + 17)
#define SID_TOGGLE_LEGEND       (SID_OBJ1_START + 18)
#define SID_TOGGLE_AXIS_TITLE   (SID_OBJ1_START + 19)
#define SID_TOGGLE_AXIS_DESCR   (SID_OBJ1_START + 20)      // 30532
#define SID_TOGGLE_GRID_HORZ    (SID_OBJ1_START + 21)
#define SID_TOGGLE_GRID_VERT    (SID_OBJ1_START + 22)
#define SID_DATA_IN_ROWS        (SID_OBJ1_START + 23)
#define SID_DATA_IN_COLUMNS     (SID_OBJ1_START + 24)
#define SID_TOOL_SELECT         (SID_OBJ1_START + 25)
#define SID_CONTEXT_TYPE        (SID_OBJ1_START + 26)
#define SID_NEW_ARRANGEMENT     (SID_OBJ1_START + 27)     //  30539
#define SID_INSERT_GRIDS        (SID_OBJ1_START + 28)

#define SID_MAINTITLE           (SID_OBJ1_START + 29)
#define SID_SUBTITLE            (SID_OBJ1_START + 30)
#define SID_XAXIS_TITLE         (SID_OBJ1_START + 31)
#define SID_YAXIS_TITLE         (SID_OBJ1_START + 32)
#define SID_ZAXIS_TITLE         (SID_OBJ1_START + 33)

#ifdef SID_UPDATE
#undef SID_UPDATE
#endif
#define SID_UPDATE              (SID_OBJ1_START + 34)

// DataCollection
#define SID_DATA                (SID_OBJ1_START + 35)
#define SID_ADD_COLUMN          (SID_OBJ1_START + 36)
#define SID_ADD_ROW             (SID_OBJ1_START + 37)

#define SID_DIAGRAM_AXIS_X      (SID_OBJ1_START + 40)
#define SID_DIAGRAM_AXIS_Y      (SID_OBJ1_START + 41)
#define SID_DIAGRAM_AXIS_Z      (SID_OBJ1_START + 42)
#define SID_DIAGRAM_AXIS_ALL    (SID_OBJ1_START + 43)
#define SID_INSERT_STATISTICS   (SID_OBJ1_START + 44)
#define SID_DIAGRAM_TITLE_MAIN  (SID_OBJ1_START + 45)
#define SID_DIAGRAM_TITLE_SUB   (SID_OBJ1_START + 46)
#define SID_DIAGRAM_TITLE_X     (SID_OBJ1_START + 47)
#define SID_DIAGRAM_TITLE_Y     (SID_OBJ1_START + 48)
#define SID_DIAGRAM_TITLE_Z     (SID_OBJ1_START + 49)
#define SID_DIAGRAM_TITLE_ALL   (SID_OBJ1_START + 50)
#define SID_DIAGRAM_GRID_Y_MAIN (SID_OBJ1_START + 51)
#define SID_DIAGRAM_GRID_X_MAIN (SID_OBJ1_START + 52)
#define SID_DIAGRAM_GRID_Z_MAIN (SID_OBJ1_START + 53)
#define SID_DIAGRAM_GRID_ALL    (SID_OBJ1_START + 54)

#define SID_HAS_X_DESCR         (SID_OBJ1_START + 55)
#define SID_HAS_Y_DESCR         (SID_OBJ1_START + 56)
#define SID_HAS_Z_DESCR         (SID_OBJ1_START + 57)

#define SID_DIAGRAM_OBJECTS     (SID_OBJ1_START + 60)

#define SID_HAS_X_TITLE         (SID_OBJ1_START + 61)
#define SID_HAS_Y_TITLE         (SID_OBJ1_START + 62)
#define SID_HAS_Z_TITLE         (SID_OBJ1_START + 63)
#define SID_HAS_MAIN_TITLE      (SID_OBJ1_START + 64)
#define SID_HAS_SUB_TITLE       (SID_OBJ1_START + 65)

#define SID_DIAGRAM_GRID_Y_HELP (SID_OBJ1_START + 66)
#define SID_DIAGRAM_GRID_X_HELP (SID_OBJ1_START + 67)
#define SID_DIAGRAM_GRID_Z_HELP (SID_OBJ1_START + 68)

#define SID_DIAGRAM_DATA_WIN    (SID_OBJ1_START + 70)

#define SID_DIAGRAM_ERROR        (SID_OBJ1_START + 71)
#define SID_DIAGRAM_AVERAGEVALUE (SID_OBJ1_START + 72)
#define SID_DIAGRAM_REGRESSION   (SID_OBJ1_START + 73)
#define SID_SCALE_TEXT           (SID_OBJ1_START + 74)
#define SID_TEXTBREAK            (SID_OBJ1_START + 75)

#define SID_DIAGRAM_DATADESCRIPTION  (SID_OBJ1_START + 76)
#define SID_DIAGRAM_POSLEGEND        (SID_OBJ1_START + 77)
#define SID_DIAGRAM_DEFAULTCOLORS    (SID_OBJ1_START + 78)
#define SID_DIAGRAM_BARWIDTH         (SID_OBJ1_START + 79)
#define SID_DIAGRAM_NUMLINES         (SID_OBJ1_START + 80)


#define SID_ROW_POSITION         (SID_OBJ1_START + 81)
#define SID_ROW_MOREFRONT        (SID_OBJ1_START + 82)
#define SID_ROW_MOREBACK         (SID_OBJ1_START + 83)

// Title-Object
#define SID_TITLE_TEXT          (SID_OBJ1_START + 100)

#define SID_DIAGRAM_STOCK_LINE  (SID_OBJ1_START + 101)
#define SID_DIAGRAM_STOCK_LOSS  (SID_OBJ1_START + 102)
#define SID_DIAGRAM_STOCK_PLUS  (SID_OBJ1_START + 103)


#define SID_DIAGRAM_AXIS_A      (SID_OBJ1_START + 104)
#define SID_DIAGRAM_AXIS_B      (SID_OBJ1_START + 105)
#define SID_DIAGRAM_AXIS_C      (SID_OBJ1_START + 106)

// Reserved till (SID_OBJ1_START + 110)

#endif
// SCH_SLOTIDS_HXX
