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

#ifndef _SCH_CHATTR_HXX
#define _SCH_CHATTR_HXX

// header for SID_OBJ1_START
#ifndef _SFXSIDS_HRC
#include <bf_sfx2/sfxsids.hrc>
#endif
namespace binfilter {

/*--------------------------------------------------

  These Slot-IDs are for internal use only.
  They are not relevant for the file format

--------------------------------------------------*/

//	SID_OBJ1_START == 30000 + 512

#define CHATTR_START					(SID_OBJ1_START + 150)			//	30662

// diagram attributes
#define CHATTR_DIAGRAM_START			CHATTR_START					//	30662
#define CHATTR_DIAGRAM_STYLE			CHATTR_DIAGRAM_START
#define CHATTR_DIAGRAM_GAPWIDTH			(CHATTR_DIAGRAM_START + 1)
#define CHATTR_DIAGRAM_OVERLAP			(CHATTR_DIAGRAM_START + 2)
#define CHATTR_DIAGRAM_END				CHATTR_DIAGRAM_OVERLAP			//	30664

// title attributes
#define CHATTR_TITLE_START				(CHATTR_DIAGRAM_END + 1)		//	30665
#define CHATTR_TITLE_SHOW_MAIN			CHATTR_TITLE_START
#define CHATTR_TITLE_MAIN				(CHATTR_TITLE_START + 1)
#define CHATTR_TITLE_SHOW_SUB			(CHATTR_TITLE_START + 2)
#define CHATTR_TITLE_SUB				(CHATTR_TITLE_START + 3)
#define CHATTR_TITLE_SHOW_X_AXIS		(CHATTR_TITLE_START + 4)
#define CHATTR_TITLE_X_AXIS				(CHATTR_TITLE_START + 5)
#define CHATTR_TITLE_SHOW_Y_AXIS		(CHATTR_TITLE_START + 6)
#define CHATTR_TITLE_Y_AXIS				(CHATTR_TITLE_START + 7)
#define CHATTR_TITLE_SHOW_Z_AXIS		(CHATTR_TITLE_START + 8)
#define CHATTR_TITLE_Z_AXIS				(CHATTR_TITLE_START + 9)
#define CHATTR_TITLE_END				CHATTR_TITLE_Z_AXIS				//	30674

// axis and grid attributes
#define CHATTR_AXISGRID_START			(CHATTR_TITLE_END + 1)			//	30675
#define CHATTR_AXISGRID_SHOW_X_AXIS		CHATTR_AXISGRID_START
#define CHATTR_AXISGRID_SHOW_X_MAIN		(CHATTR_AXISGRID_START + 1)
#define CHATTR_AXISGRID_SHOW_X_HELP		(CHATTR_AXISGRID_START + 2)
#define CHATTR_AXISGRID_SHOW_X_DESCR	(CHATTR_AXISGRID_START + 3)
#define CHATTR_AXISGRID_SHOW_Y_AXIS		(CHATTR_AXISGRID_START + 4)
#define CHATTR_AXISGRID_SHOW_Y_MAIN		(CHATTR_AXISGRID_START + 5)
#define CHATTR_AXISGRID_SHOW_Y_HELP		(CHATTR_AXISGRID_START + 6)
#define CHATTR_AXISGRID_SHOW_Y_DESCR	(CHATTR_AXISGRID_START + 7)
#define CHATTR_AXISGRID_SHOW_Z_AXIS		(CHATTR_AXISGRID_START + 8)
#define CHATTR_AXISGRID_SHOW_Z_MAIN		(CHATTR_AXISGRID_START + 9)
#define CHATTR_AXISGRID_SHOW_Z_HELP		(CHATTR_AXISGRID_START + 10)
#define CHATTR_AXISGRID_SHOW_Z_DESCR	(CHATTR_AXISGRID_START + 11)
#define CHATTR_AXIS_AUTO_ORIGIN			(CHATTR_AXISGRID_START + 12)
#define CHATTR_AXISGRID_END				CHATTR_AXIS_AUTO_ORIGIN			//	30687

// legend attributes
#define CHATTR_LEGEND_START				(CHATTR_AXISGRID_END + 1)		//	30688
#define CHATTR_LEGEND_POS				CHATTR_LEGEND_START
#define CHATTR_LEGEND_END				CHATTR_LEGEND_POS				//	30688

// data attributes
#define CHATTR_DATA_START				(CHATTR_LEGEND_END + 1)			//	30689
#define CHATTR_DATA_SWITCH				CHATTR_DATA_START
#define CHATTR_DATA_END					CHATTR_DATA_SWITCH				//	30689

#define CHATTR_AXISGRID_SHOW_2Y_AXIS	(CHATTR_DATA_END + 1)			//	30690
#define CHATTR_AXISGRID_SHOW_2Y_DESCR	(CHATTR_DATA_END + 2)
#define CHATTR_AXISGRID_SHOW_2X_AXIS	(CHATTR_DATA_END + 3)
#define CHATTR_AXISGRID_SHOW_2X_DESCR	(CHATTR_DATA_END + 4)			//	30693

#define CHATTR_STYLE_SHAPE				(CHATTR_DATA_END + 5)			// #67610#

#define CHATTR_DEFCOLORS				(CHATTR_DATA_END + 6)			// ListItem of default colors

#define CHATTR_REGRESSION_PROPS			(CHATTR_DATA_END + 7)			// #70231#
#define CHATTR_ERROR_PROPS				(CHATTR_DATA_END + 8)
#define CHATTR_AVERAGE_PROPS			(CHATTR_DATA_END + 9)

#define CHATTR_NUM_OF_LINES_FOR_BAR		(CHATTR_DATA_END + 10)

#define CHATTR_PIE_SEGMENT_OFFSET		(CHATTR_DATA_END + 11)

#define CHATTR_ADDR_CATEGORIES			(CHATTR_DATA_END + 12)
#define CHATTR_ADDR_SERIES				(CHATTR_DATA_END + 13)
#define CHATTR_ADDR_MAIN_TITLE			(CHATTR_DATA_END + 14)
#define CHATTR_ADDR_SUB_TITLE			(CHATTR_DATA_END + 15)
#define CHATTR_ADDR_CHART               (CHATTR_DATA_END + 16)
#define CHATTR_TABLE_NUMBER_LIST        (CHATTR_DATA_END + 17)
#define CHATTR_EXPORT_TABLE             (CHATTR_DATA_END + 18)
#define CHATTR_FIRST_COL_LABELS         (CHATTR_DATA_END + 19)
#define CHATTR_FIRST_ROW_LABELS         (CHATTR_DATA_END + 20)			//	30709
#define CHATTR_SPLINE_ORDER             (CHATTR_DATA_END + 21)			//	30710
#define CHATTR_SPLINE_RESOLUTION        (CHATTR_DATA_END + 22)			//	30711
#define CHATTR_ADDITIONAL_SHAPES        (CHATTR_DATA_END + 23)			//	30712

#define CHATTR_HAS_TRANSLATED_COLS      (CHATTR_DATA_END + 24)			//	30713
#define CHATTR_HAS_TRANSLATED_ROWS      (CHATTR_DATA_END + 25)			//	30714
#define CHATTR_TRANSLATED_COLS          (CHATTR_DATA_END + 26)			//	30715
#define CHATTR_TRANSLATED_ROWS          (CHATTR_DATA_END + 27)			//	30716

#define CHATTR_ATTRIBUTED_DATA_POINTS   (CHATTR_DATA_END + 28)          //  30717

#define CHATTR_END						CHATTR_ATTRIBUTED_DATA_POINTS   //  30717

//Diese Items sollten früher oder später in den Pool als SCHATTR_*
//Derzeit werden sie nur für StarOne zum Datenaustauch benutzt,
//speichern ist streng verboten , externe Verwendung
//(nicht innerhalb des Chart) ebenfalls

#define CHATTR_TMP_START				CHATTR_DATA_END + 30			//	30719

#define CHATTR_BARWIDTH					CHATTR_TMP_START
#define CHATTR_BARCONNECT				CHATTR_TMP_START+1

#define CHATTR_TMP_END					CHATTR_BARCONNECT				//	30720

} //namespace binfilter
#endif	// _SCH_CHATTR_HXX

