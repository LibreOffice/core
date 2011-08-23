/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
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
 *  The Initial Developer of the Original Code is: IBM Corporation
 *
 *  Copyright: 2008 by IBM Corporation
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef	_LWPSTYLEDEF_H_
#define	_LWPSTYLEDEF_H_

#define	INVALID_ATTR_ID 0
#define	EXTERNAL_ID			0x8000
#define	EXTERNAL_MASK		0x7FFF
#define MAX_DESIGNERBORDERS 18

#define BORDER_SOLID		1
#define	BORDER_SDASH		2
#define	BORDER_LDASH		3
#define	BORDER_WAVY			4
#define	BORDER_THKWAVY		5
#define	BORDER_LSDASH		6
#define	BORDER_LSSDASH		7
#define	BORDER_DOT			8
#define	BORDER_CIRCLE		9
#define	BORDER_DBLWAVY		10
#define	BORDER_DBLTHKWAVY	11
#define	BORDER_STAR			12
#define	BORDER_ROPE			13
#define	BORDER_DECO1		14
#define	BORDER_DECO2		15
#define	BORDER_RAIN			16
#define	BORDER_PIN			17
#define	BORDER_ROSE			18
#define	BORDER_SUNF			19
#define	BORDER_DECO3		20
#define	BORDER_WARNING		21
// SPACE					22
#define BORDER_GIRDER		23

// User defined bitmap borders (200 - 400)
//========================================

// scalable lines
#define BGRP_SOLID				20
#define BGRP_DOUBLE_ONE			21
#define BGRP_DOUBLE_TWO			22
#define BGRP_121				23
#define BGRP_31					24
#define BGRP_13					25
#define BGRP_WAVY				26
#define BGRP_DBLWAVY			27
#define BGRP_THICK_WAVY			28
#define BGRP_THICK_DBLWAVY		29
#define BGRP_SDASH				30
#define BGRP_LDASH				31
#define BGRP_LDASH_SDASH		32
#define BGRP_LDASH_SDASH_SDASH	33
#define BGRP_BUTTONUP			34
#define BGRP_BUTTONDOWN			35
#define BGRP_13SPACE			36
#define BGRP_31SPACE			37
#define BGRP_DIAGONAL			38
#define BGRP_TARO				39
#define BGRP_DOT				40
#define BGRP_CIRCLE				41
#define	BGRP_STAR				42
#define	BGRP_ROPE				43
#define	BGRP_DECO1				44
#define	BGRP_DECO2				45
#define	BGRP_RAIN				46
#define	BGRP_PIN				47
#define	BGRP_ROSE				48
#define	BGRP_SUNF				49
#define	BGRP_DECO3				50
#define	BGRP_WARNING			51
//SPACE 					52
#define BGRP_GIRDER				53

#define UNIT_4_96  ((UNIT)196608L)	//INCHES(4/96)
#define UNIT_13_96 ((UNIT)638976L)	//INCHES(13/96)
#define UNIT_14_96 ((UNIT)688128L)	//INCHES(14/96)
#define UNIT_15_96 ((UNIT)737280L)	//INCHES(15/96)
#define UNIT_17_96 ((UNIT)835584L)	//INCHES(17/96)
#define UNIT_18_96 ((UNIT)884736L)	//INCHES(18/96)
#define UNIT_19_96 ((UNIT)933888L)	//INCHES(19/96)
#define UNIT_20_96 ((UNIT)983040L)	//INCHES(20/96)
#define UNIT_21_96 ((UNIT)1032192L)	//INCHES(21/96)
#define UNIT_22_96 ((UNIT)1081344L)	//INCHES(22/96)
#define UNIT_25_96 ((UNIT)1228800L)	//INCHES(25/96)
#define UNIT_28_96 ((UNIT)1376256L)	//INCHES(28/96)
#define UNIT_34_96 ((UNIT)1671108L)	//INCHES(34/96)
#define UNIT_42_96 ((UNIT)2064384L)	//INCHES(42/96)
#define UNIT_44_96 ((UNIT)2162688L)	//INCHES(44/96)
#define UNIT_48_96 ((UNIT)2359296L)	//INCHES(48/96)
#define UNIT_56_96 ((UNIT)2752512L)	//INCHES(56/96)
#define UNIT_64_96 ((UNIT)3145728L)	//INCHES(64/96)

#define ROPE_AMPLITUDE 		UNIT_18_96
#define ROPE_PERIOD			UNIT_20_96
#define DECO1_AMPLITUDE 	UNIT_17_96
#define DECO1_PERIOD		UNIT_4_96
#define DECO2_AMPLITUDE 	UNIT_18_96
#define DECO2_PERIOD		UNIT_64_96
#define RAIN_AMPLITUDE 		UNIT_22_96
#define RAIN_PERIOD			UNIT_28_96
#define PIN_AMPLITUDE 		UNIT_15_96
#define PIN_PERIOD			UNIT_4_96
#define ROSE_AMPLITUDE 		UNIT_34_96
#define ROSE_PERIOD			UNIT_64_96
#define SUNF_AMPLITUDE 		UNIT_25_96
#define SUNF_PERIOD			UNIT_48_96
#define DECO3_AMPLITUDE 	UNIT_14_96
#define DECO3_PERIOD		UNIT_20_96

#define ROPE_JOIN_W			UNIT_18_96
#define ROPE_JOIN_H 		UNIT_18_96
#define DECO1_JOIN_W 		UNIT_56_96
#define DECO1_JOIN_H 		UNIT_44_96
#define DECO2_JOIN_W 		UNIT_64_96
#define DECO2_JOIN_H 		UNIT_64_96
#define RAIN_JOIN_W 		UNIT_48_96
#define RAIN_JOIN_H 		UNIT_48_96
#define PIN_JOIN_W 			UNIT_42_96
#define PIN_JOIN_H 			UNIT_42_96
#define ROSE_JOIN_W 		UNIT_48_96
#define ROSE_JOIN_H 		UNIT_48_96
#define SUNF_JOIN_W 		UNIT_48_96
#define SUNF_JOIN_H 		UNIT_48_96
#define DECO3_JOIN_W 		UNIT_56_96
#define DECO3_JOIN_H 		UNIT_56_96

// User defined bitmap border groups 200 - 400
//============================================

/* background patterns */
#define BACK_TRANSPARENT	0
#define BACK_FILL			1
#define BACK_PATTERN		2
#define BACK_10PCTDOTS		3
#define BACK_30PCTDOTS		4
#define BACK_50PCTDOTS		5
#define BACK_75PCTDOTS		6
#define BACK_90PCTDOTS		7
#define BACK_90PCTINVDOTS	8
#define BACK_75PCTINVDOTS	9
#define BACK_50PCTINVDOTS	10
#define BACK_30PCTINVDOTS	11
#define BACK_10PCTINVDOTS	12
#define BACK_MARBLES		13
#define BACK_STARRYNIGHT	14
#define BACK_DIAGTHICK		15
#define BACK_DIAGTHIN		16
#define BACK_DIAGDOUBLE		17
#define BACK_DIAGDOUBLENARROW 18
#define BACK_DIAGTRIPLE		19
#define BACK_DIAGSINGLEWIDE	20
#define BACK_DIAGTHICKINV	21
#define BACK_DIAGTHININV	22
#define BACK_DIAGDOUBLEINV	23
#define BACK_DIAGDOUBLENARROWINV 24
#define BACK_DIAGTRIPLEINV		25
#define BACK_DIAGSINGLEWIDEINV	26
#define BACK_CHAINLINKSMALLINV	27
#define BACK_CHAINLINKSMALL		28
#define BACK_CHAINLINKMEDIUM	29
#define BACK_CHAINLINKLARGE		30
#define BACK_HORZSTRIPE1		31
#define BACK_HORZSTRIPE2		32
#define BACK_HORZSTRIPE3		33
#define BACK_HORZSTRIPE4		34
#define BACK_VERTSTRIPE1		35
#define BACK_VERTSTRIPE2		36
#define BACK_VERTSTRIPE3		37
#define BACK_VERTSTRIPE4		38
#define BACK_GRIDSMALL			39
#define BACK_GRIDMEDIUM			40
#define BACK_GRIDLARGE			41
#define BACK_BOXDOT				42
#define BACK_GOLFGREEN			43
#define BACK_DITZ				44
#define BACK_HORZWEAVE			45
#define BACK_APPLEPIE			46
#define BACK_BRICK				47
#define BACK_DIAGBRICK			48
#define BACK_CHECKER1			49
#define BACK_CHECKER2			50
#define BACK_CHECKER3			51
#define BACK_CHECKER4			52
#define BACK_CHECKER5			53
#define BACK_SNOWFLAKE			54
#define BACK_MARGARITA			55
#define BACK_SIDEHARP			56
#define BACK_SCALES				57
#define BACK_TAYLOR				58
#define BACK_BUBBLES			59
#define BACK_ARGYLE				60
#define BACK_INDIAN1			61
#define BACK_INDIAN3			62
#define BACK_PEACHPIE			63
#define BACK_GRADIENT1			64
#define BACK_GRADIENT2			65
#define BACK_GRADIENT3			66
#define BACK_GRADIENT4			67
#define BACK_GRADIENT5			68
#define BACK_GRADIENT6			69
#define BACK_GRADIENT7			70
#define BACK_GRADIENT8			71

#endif //STYLEDEF_H_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
