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

#ifndef _SCHATTR_HXX
#define _SCHATTR_HXX

#include <bf_sfx2/sfxdefs.hxx>

#include "chattr.hxx"
namespace binfilter {

#define SCHATTR_START						1

#define SCHATTR_DATADESCR_START	   			SCHATTR_START
#define SCHATTR_DATADESCR_DESCR	   			SCHATTR_DATADESCR_START
#define SCHATTR_DATADESCR_SHOW_SYM 			(SCHATTR_DATADESCR_START + 1) //2
#define SCHATTR_DATADESCR_END	   			SCHATTR_DATADESCR_SHOW_SYM

#define SCHATTR_LEGEND_START	   			(SCHATTR_DATADESCR_END + 1)   //3
#define SCHATTR_LEGEND_POS		   			SCHATTR_LEGEND_START
#define SCHATTR_LEGEND_END		 			SCHATTR_LEGEND_POS

#define SCHATTR_TEXT_START					(SCHATTR_LEGEND_END + 1)      //4
#define SCHATTR_TEXT_ORIENT					SCHATTR_TEXT_START
#define SCHATTR_TEXT_ORDER					(SCHATTR_TEXT_START + 1)      //5
#define SCHATTR_TEXT_END					SCHATTR_TEXT_ORDER

#define SCHATTR_Y_AXIS_START				(SCHATTR_TEXT_END + 1)        //6
#define SCHATTR_Y_AXIS_AUTO_MIN				SCHATTR_Y_AXIS_START
#define SCHATTR_Y_AXIS_MIN					(SCHATTR_Y_AXIS_START + 1)	//7
#define SCHATTR_Y_AXIS_AUTO_MAX	  			(SCHATTR_Y_AXIS_START + 2)	//8
#define SCHATTR_Y_AXIS_MAX					(SCHATTR_Y_AXIS_START + 3)	//9
#define SCHATTR_Y_AXIS_AUTO_STEP_MAIN		(SCHATTR_Y_AXIS_START + 4)	//10
#define SCHATTR_Y_AXIS_STEP_MAIN			(SCHATTR_Y_AXIS_START + 5)	//11
#define SCHATTR_Y_AXIS_AUTO_STEP_HELP		(SCHATTR_Y_AXIS_START + 6)	//12
#define SCHATTR_Y_AXIS_STEP_HELP			(SCHATTR_Y_AXIS_START + 7)	//13
#define SCHATTR_Y_AXIS_LOGARITHM			(SCHATTR_Y_AXIS_START + 8)	//14
#define SCHATTR_Y_AXIS_AUTO_ORIGIN			(SCHATTR_Y_AXIS_START + 9)	//15
#define SCHATTR_Y_AXIS_ORIGIN				(SCHATTR_Y_AXIS_START + 10)	//16
#define SCHATTR_Y_AXIS_END					SCHATTR_Y_AXIS_ORIGIN

#define SCHATTR_X_AXIS_START				(SCHATTR_Y_AXIS_END + 1)	//17
#define SCHATTR_X_AXIS_AUTO_MIN				SCHATTR_X_AXIS_START		//17
#define SCHATTR_X_AXIS_MIN					(SCHATTR_X_AXIS_START + 1)	//18
#define SCHATTR_X_AXIS_AUTO_MAX	  			(SCHATTR_X_AXIS_START + 2)	//19
#define SCHATTR_X_AXIS_MAX					(SCHATTR_X_AXIS_START + 3)	//20
#define SCHATTR_X_AXIS_AUTO_STEP_MAIN		(SCHATTR_X_AXIS_START + 4)	//21
#define SCHATTR_X_AXIS_STEP_MAIN			(SCHATTR_X_AXIS_START + 5)	//22
#define SCHATTR_X_AXIS_AUTO_STEP_HELP		(SCHATTR_X_AXIS_START + 6)	//23
#define SCHATTR_X_AXIS_STEP_HELP			(SCHATTR_X_AXIS_START + 7)	//24
#define SCHATTR_X_AXIS_LOGARITHM			(SCHATTR_X_AXIS_START + 8)	//25
#define SCHATTR_X_AXIS_AUTO_ORIGIN			(SCHATTR_X_AXIS_START + 9)	//26
#define SCHATTR_X_AXIS_ORIGIN				(SCHATTR_X_AXIS_START + 10)	//27
#define SCHATTR_X_AXIS_END					SCHATTR_X_AXIS_ORIGIN

#define SCHATTR_Z_AXIS_START				(SCHATTR_X_AXIS_END + 1)    //28
#define SCHATTR_Z_AXIS_AUTO_MIN				SCHATTR_Z_AXIS_START		//28
#define SCHATTR_Z_AXIS_MIN					(SCHATTR_Z_AXIS_START + 1)	//29
#define SCHATTR_Z_AXIS_AUTO_MAX	  			(SCHATTR_Z_AXIS_START + 2)  //30
#define SCHATTR_Z_AXIS_MAX					(SCHATTR_Z_AXIS_START + 3)	//31
#define SCHATTR_Z_AXIS_AUTO_STEP_MAIN		(SCHATTR_Z_AXIS_START + 4)	//32
#define SCHATTR_Z_AXIS_STEP_MAIN			(SCHATTR_Z_AXIS_START + 5)	//33
#define SCHATTR_Z_AXIS_AUTO_STEP_HELP		(SCHATTR_Z_AXIS_START + 6)	//34
#define SCHATTR_Z_AXIS_STEP_HELP			(SCHATTR_Z_AXIS_START + 7)	//35
#define SCHATTR_Z_AXIS_LOGARITHM			(SCHATTR_Z_AXIS_START + 8)	//36
#define SCHATTR_Z_AXIS_AUTO_ORIGIN			(SCHATTR_Z_AXIS_START + 9)	//37
#define SCHATTR_Z_AXIS_ORIGIN				(SCHATTR_Z_AXIS_START + 10)	//38
#define SCHATTR_Z_AXIS_END					SCHATTR_Z_AXIS_ORIGIN

// miscellaneous

#define SCHATTR_AXISTYPE				    (SCHATTR_Z_AXIS_END + 1) //39
#define SCHATTR_DUMMY0  				    (SCHATTR_Z_AXIS_END + 2) //40
#define SCHATTR_DUMMY1  				    (SCHATTR_Z_AXIS_END + 3) //41
#define SCHATTR_DUMMY2  				    (SCHATTR_Z_AXIS_END + 4) //42
#define SCHATTR_DUMMY3  				    (SCHATTR_Z_AXIS_END + 5) //43
#define SCHATTR_DUMMY_END  				    (SCHATTR_Z_AXIS_END + 6) //44

#define SCHATTR_STAT_START                  (SCHATTR_DUMMY_END + 1)  //45
#define SCHATTR_STAT_AVERAGE                SCHATTR_STAT_START
#define SCHATTR_STAT_KIND_ERROR             (SCHATTR_STAT_START + 1) //46
#define SCHATTR_STAT_PERCENT                (SCHATTR_STAT_START + 2) //47
#define SCHATTR_STAT_BIGERROR               (SCHATTR_STAT_START + 3) //48
#define SCHATTR_STAT_CONSTPLUS              (SCHATTR_STAT_START + 4) //49
#define SCHATTR_STAT_CONSTMINUS             (SCHATTR_STAT_START + 5) //50
#define SCHATTR_STAT_REGRESSTYPE            (SCHATTR_STAT_START + 6) //51
#define SCHATTR_STAT_INDICATE               (SCHATTR_STAT_START + 7) //52
#define SCHATTR_STAT_END                    SCHATTR_STAT_INDICATE

#define SCHATTR_TEXT2_START                 ( SCHATTR_STAT_END + 1) //53
#define SCHATTR_TEXT_DEGREES                ( SCHATTR_TEXT2_START   )
#define SCHATTR_TEXT_OVERLAP                (SCHATTR_TEXT2_START +1) //54
#define SCHATTR_TEXT_DUMMY0				    (SCHATTR_TEXT2_START + 2)//55
#define SCHATTR_TEXT_DUMMY1				    (SCHATTR_TEXT2_START + 3)//56
#define SCHATTR_TEXT_DUMMY2				    (SCHATTR_TEXT2_START + 4)//57
#define SCHATTR_TEXT_DUMMY3				    (SCHATTR_TEXT2_START + 5)//58
#define SCHATTR_TEXT2_END                   ( SCHATTR_TEXT_DUMMY3 )

// --------------------------------------------------------
// these attributes are for replacement of enum eChartStyle
// --------------------------------------------------------

#define SCHATTR_STYLE_START             ( SCHATTR_TEXT2_END +1 )  //59

// for whole chart
#define SCHATTR_STYLE_DEEP              ( SCHATTR_STYLE_START     )//59
#define SCHATTR_STYLE_3D                ( SCHATTR_STYLE_START + 1 )//60
#define SCHATTR_STYLE_VERTICAL          ( SCHATTR_STYLE_START + 2 )//61

// also for series
#define SCHATTR_STYLE_BASETYPE          ( SCHATTR_STYLE_START + 3 )//62 Line,Area,...,Pie
#define SCHATTR_STYLE_LINES             ( SCHATTR_STYLE_START + 4 )//63 draw line
#define SCHATTR_STYLE_PERCENT           ( SCHATTR_STYLE_START + 5 )//64
#define SCHATTR_STYLE_STACKED           ( SCHATTR_STYLE_START + 6 )//65
#define SCHATTR_STYLE_SPLINES           ( SCHATTR_STYLE_START + 7 )//66

// also for data point
#define SCHATTR_STYLE_SYMBOL            ( SCHATTR_STYLE_START + 8 )//67
#define SCHATTR_STYLE_SHAPE				( SCHATTR_STYLE_START + 9 )//68
#define SCHATTR_STYLE_END               ( SCHATTR_STYLE_SHAPE )

// --------------------------------------------------------------------------

#define SCHATTR_AXIS					(SCHATTR_STYLE_END + 1)//69	see chtmodel.hxx defines CHART_AXIS_PRIMARY_X, etc.
//Ummappen:
#define SCHATTR_AXIS_START				(SCHATTR_AXIS + 1)     //70
#define SCHATTR_AXIS_AUTO_MIN			 SCHATTR_AXIS_START
#define SCHATTR_AXIS_MIN				(SCHATTR_AXIS_START + 1)//71
#define SCHATTR_AXIS_AUTO_MAX	  		(SCHATTR_AXIS_START + 2)//72
#define SCHATTR_AXIS_MAX				(SCHATTR_AXIS_START + 3)//73
#define SCHATTR_AXIS_AUTO_STEP_MAIN		(SCHATTR_AXIS_START + 4)//74
#define SCHATTR_AXIS_STEP_MAIN			(SCHATTR_AXIS_START + 5)//75
#define SCHATTR_AXIS_AUTO_STEP_HELP		(SCHATTR_AXIS_START + 6)//76
#define SCHATTR_AXIS_STEP_HELP			(SCHATTR_AXIS_START + 7)//77
#define SCHATTR_AXIS_LOGARITHM			(SCHATTR_AXIS_START + 8)//78
#define SCHATTR_AXIS_AUTO_ORIGIN		(SCHATTR_AXIS_START + 9)//79
#define SCHATTR_AXIS_ORIGIN				(SCHATTR_AXIS_START + 10)//80
//Neu:
#define SCHATTR_AXIS_TICKS				(SCHATTR_AXIS_START + 11)//81
#define SCHATTR_AXIS_NUMFMT				(SCHATTR_AXIS_START + 12)//82
#define SCHATTR_AXIS_NUMFMTPERCENT		(SCHATTR_AXIS_START + 13)//83
#define SCHATTR_AXIS_SHOWAXIS			(SCHATTR_AXIS_START + 14)//84
#define SCHATTR_AXIS_SHOWDESCR			(SCHATTR_AXIS_START + 15)//85
#define SCHATTR_AXIS_SHOWMAINGRID		(SCHATTR_AXIS_START + 16)//86
#define SCHATTR_AXIS_SHOWHELPGRID		(SCHATTR_AXIS_START + 17)//87
#define SCHATTR_AXIS_TOPDOWN			(SCHATTR_AXIS_START + 18)//88
#define SCHATTR_AXIS_HELPTICKS			(SCHATTR_AXIS_START + 19)//89

#define SCHATTR_AXIS_DUMMY0				(SCHATTR_AXIS_START + 20)//90
#define SCHATTR_AXIS_DUMMY1				(SCHATTR_AXIS_START + 21)//91
#define SCHATTR_AXIS_DUMMY2				(SCHATTR_AXIS_START + 22)//92
#define SCHATTR_AXIS_DUMMY3				(SCHATTR_AXIS_START + 23)//93
#define SCHATTR_BAR_OVERLAP				(SCHATTR_AXIS_START + 24)//94
#define SCHATTR_BAR_GAPWIDTH			(SCHATTR_AXIS_START + 25)//95
#define SCHATTR_AXIS_END				 SCHATTR_BAR_GAPWIDTH

#define SCHATTR_SYMBOL_BRUSH			(SCHATTR_AXIS_END + 1) //96
#define SCHATTR_STOCK_VOLUME			(SCHATTR_AXIS_END + 2) //97
#define SCHATTR_STOCK_UPDOWN			(SCHATTR_AXIS_END + 3) //98
#define SCHATTR_SYMBOL_SIZE				(SCHATTR_AXIS_END + 4) //99

// non persistent items (binary format)
#define SCHATTR_NONPERSISTENT_START     (SCHATTR_SYMBOL_SIZE + 1)    //100
#define SCHATTR_USER_DEFINED_ATTR        SCHATTR_NONPERSISTENT_START //100
#define SCHATTR_NONPERSISTENT_END        SCHATTR_USER_DEFINED_ATTR

#define SCHATTR_END						 SCHATTR_NONPERSISTENT_END

} //namespace binfilter
#endif // _SCHATTR_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
