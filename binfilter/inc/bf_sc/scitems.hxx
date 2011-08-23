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

#ifndef SC_ITEMS_HXX
#define SC_ITEMS_HXX

#ifndef _SVX_SVXIDS_HRC
#include <bf_svx/svxids.hrc>
#endif
namespace binfilter {

//	EditEngine darf nicht ihre eigenen ITEMID's definieren
#define _EEITEMID_HXX

//--------------------------
// Item-IDs fuer UI-MsgPool:
//--------------------------

//!	sollten auch nach unter 1000 verschoben werden!

#define MSGPOOL_START				1100
//--------------------------------------
#define SCITEM_STRING       		1100
#define SCITEM_SEARCHDATA			1101
#define SCITEM_SORTDATA				1102
#define SCITEM_QUERYDATA			1103
#define SCITEM_SUBTDATA     		1104
#define SCITEM_CONSOLIDATEDATA      1105
#define SCITEM_PIVOTDATA		    1106
#define SCITEM_SOLVEDATA            1107
#define SCITEM_USERLIST             1108
#define SCITEM_PRINTWARN            1109
//--------------------------------------
#define MSGPOOL_END                 1109

//-------------------------
// Item-IDs fuer Attribute:
//-------------------------
#define ATTR_STARTINDEX			100		// Anfang Attribute

#define ATTR_PATTERN_START		100		// Anfang Zellattribut-Pattern

#define ATTR_FONT				100		// Anfang Zellattribute
#define ATTR_FONT_HEIGHT 		101
#define ATTR_FONT_WEIGHT		102
#define	ATTR_FONT_POSTURE		103
#define	ATTR_FONT_UNDERLINE		104
#define	ATTR_FONT_CROSSEDOUT	105
#define ATTR_FONT_CONTOUR		106
#define ATTR_FONT_SHADOWED		107
#define ATTR_FONT_COLOR			108
#define ATTR_FONT_LANGUAGE		109
#define ATTR_CJK_FONT			110
#define ATTR_CJK_FONT_HEIGHT 	111
#define ATTR_CJK_FONT_WEIGHT	112
#define	ATTR_CJK_FONT_POSTURE	113
#define ATTR_CJK_FONT_LANGUAGE	114
#define ATTR_CTL_FONT			115
#define ATTR_CTL_FONT_HEIGHT 	116
#define ATTR_CTL_FONT_WEIGHT	117
#define	ATTR_CTL_FONT_POSTURE	118
#define ATTR_CTL_FONT_LANGUAGE	119
#define ATTR_FONT_EMPHASISMARK	120
#define ATTR_USERDEF			121		// not saved in binary files
#define ATTR_FONT_WORDLINE		122
#define ATTR_FONT_RELIEF		123
#define ATTR_HYPHENATE			124
#define ATTR_SCRIPTSPACE		125
#define ATTR_HANGPUNCTUATION	126
#define ATTR_FORBIDDEN_RULES	127
#define	ATTR_HOR_JUSTIFY		128
#define ATTR_INDENT				129
#define	ATTR_VER_JUSTIFY		130
#define ATTR_ORIENTATION		131
#define ATTR_ROTATE_VALUE		132
#define ATTR_ROTATE_MODE		133
#define ATTR_VERTICAL_ASIAN		134
#define ATTR_WRITINGDIR			135
#define ATTR_LINEBREAK			136
#define ATTR_MARGIN				137
#define	ATTR_MERGE				138
#define	ATTR_MERGE_FLAG			139
#define	ATTR_VALUE_FORMAT		140
#define	ATTR_LANGUAGE_FORMAT	141
#define	ATTR_BACKGROUND			142
#define	ATTR_PROTECTION			143
#define	ATTR_BORDER				144
#define ATTR_BORDER_INNER		145		// innen, wg. Vorlagen-EditDialog
#define	ATTR_SHADOW				146
#define ATTR_VALIDDATA			147
#define ATTR_CONDITIONAL		148

#define ATTR_PATTERN_END		148		// Ende Zellattribut-Pattern

#define ATTR_PATTERN			149
                                        // Seitenattribute
#define ATTR_LRSPACE			150		// Editor: PageDesc-TabPage
#define ATTR_ULSPACE 			151
#define ATTR_PAGE				152
#define ATTR_PAGE_PAPERTRAY		153
#define ATTR_PAGE_PAPERBIN		154
#define ATTR_PAGE_SIZE			155
#define ATTR_PAGE_MAXSIZE		156
#define ATTR_PAGE_HORCENTER		157
#define ATTR_PAGE_VERCENTER		158

#define ATTR_PAGE_ON			159		// Editor: Header/Footer-Page
#define ATTR_PAGE_DYNAMIC		160
#define ATTR_PAGE_SHARED		161

#define ATTR_PAGE_NOTES			162		// Editor: Tabelle
#define ATTR_PAGE_GRID			163
#define ATTR_PAGE_HEADERS		164
#define ATTR_PAGE_CHARTS		165
#define ATTR_PAGE_OBJECTS		166
#define ATTR_PAGE_DRAWINGS		167
#define ATTR_PAGE_TOPDOWN		168
#define ATTR_PAGE_SCALE			169
#define ATTR_PAGE_SCALETOPAGES	170
#define ATTR_PAGE_FIRSTPAGENO	171

#define ATTR_PAGE_PRINTAREA		172		// Editor: Druckbereiche
#define ATTR_PAGE_REPEATROW		173
#define ATTR_PAGE_REPEATCOL		174
#define ATTR_PAGE_PRINTTABLES	175

#define ATTR_PAGE_HEADERLEFT	176		// Inhalte der Kopf-/
#define ATTR_PAGE_FOOTERLEFT	177		// Fusszeilen (links)
#define ATTR_PAGE_HEADERRIGHT	178		// Inhalte der Kopf-/
#define ATTR_PAGE_FOOTERRIGHT	179		// Fusszeilen (rechts)
#define ATTR_PAGE_HEADERSET		180		// die Sets dazu...
#define ATTR_PAGE_FOOTERSET		181

#define	ATTR_PAGE_FORMULAS		182
#define	ATTR_PAGE_NULLVALS		183

#define ATTR_ENDINDEX			ATTR_PAGE_NULLVALS		// Ende Pool-Range

//--------------------------------
// Dummy Slot-IDs fuer Dialoge
//--------------------------------
#define SID_SCATTR_PAGE_NOTES			ATTR_PAGE_NOTES
#define SID_SCATTR_PAGE_GRID			ATTR_PAGE_GRID
#define SID_SCATTR_PAGE_HEADERS			ATTR_PAGE_HEADERS
#define SID_SCATTR_PAGE_CHARTS			ATTR_PAGE_CHARTS
#define SID_SCATTR_PAGE_OBJECTS			ATTR_PAGE_OBJECTS
#define SID_SCATTR_PAGE_DRAWINGS		ATTR_PAGE_DRAWINGS
#define SID_SCATTR_PAGE_TOPDOWN			ATTR_PAGE_TOPDOWN
#define SID_SCATTR_PAGE_SCALE			ATTR_PAGE_SCALE
#define SID_SCATTR_PAGE_SCALETOPAGES	ATTR_PAGE_SCALETOPAGES
#define SID_SCATTR_PAGE_FIRSTPAGENO		ATTR_PAGE_FIRSTPAGENO
#define SID_SCATTR_PAGE_PRINTAREA		ATTR_PAGE_PRINTAREA
#define SID_SCATTR_PAGE_REPEATROW		ATTR_PAGE_REPEATROW
#define SID_SCATTR_PAGE_REPEATCOL		ATTR_PAGE_REPEATCOL
#define SID_SCATTR_PAGE_PRINTTABLES		ATTR_PAGE_PRINTTABLES
#define SID_SCATTR_PAGE_HEADERLEFT		ATTR_PAGE_HEADERLEFT
#define SID_SCATTR_PAGE_FOOTERLEFT		ATTR_PAGE_FOOTERLEFT
#define SID_SCATTR_PAGE_HEADERRIGHT		ATTR_PAGE_HEADERRIGHT
#define SID_SCATTR_PAGE_FOOTERRIGHT		ATTR_PAGE_FOOTERRIGHT
#define	SID_SCATTR_PAGE_FORMULAS		ATTR_PAGE_FORMULAS
#define SID_SCATTR_PAGE_NULLVALS	 	ATTR_PAGE_NULLVALS

//--------------------------------
// IDs fuer Svx-Items:
//--------------------------------
#define ITEMID_LINE 			SID_FRAME_LINESTYLE
#define ITEMID_BOX				ATTR_BORDER
#define ITEMID_BOXINFO			ATTR_BORDER_INNER
#define ITEMID_SHADOW			ATTR_SHADOW
#define ITEMID_BRUSH			ATTR_BACKGROUND
#define ITEMID_FONT 			ATTR_FONT
#define ITEMID_FONTHEIGHT		ATTR_FONT_HEIGHT
#define ITEMID_WEIGHT			ATTR_FONT_WEIGHT
#define ITEMID_POSTURE			ATTR_FONT_POSTURE
#define ITEMID_UNDERLINE		ATTR_FONT_UNDERLINE
#define ITEMID_CROSSEDOUT		ATTR_FONT_CROSSEDOUT
#define ITEMID_CONTOUR			ATTR_FONT_CONTOUR
#define ITEMID_SHADOWED 		ATTR_FONT_SHADOWED
#define ITEMID_COLOR			ATTR_FONT_COLOR
#define ITEMID_LANGUAGE 		ATTR_FONT_LANGUAGE
#define ITEMID_EMPHASISMARK		ATTR_FONT_EMPHASISMARK
#define ITEMID_WORDLINEMODE		ATTR_FONT_WORDLINE
#define ITEMID_CHARRELIEF		ATTR_FONT_RELIEF
#define ITEMID_SCRIPTSPACE		ATTR_SCRIPTSPACE
#define ITEMID_HANGINGPUNCTUATION	ATTR_HANGPUNCTUATION
#define ITEMID_FORBIDDENRULE	ATTR_FORBIDDEN_RULES
#define ITEMID_HORJUSTIFY		ATTR_HOR_JUSTIFY
#define ITEMID_VERJUSTIFY		ATTR_VER_JUSTIFY
#define ITEMID_ORIENTATION		ATTR_ORIENTATION
#define ITEMID_FRAMEDIR			ATTR_WRITINGDIR
#define ITEMID_LINEBREAK		ATTR_LINEBREAK
#define ITEMID_MARGIN			ATTR_MARGIN
#define ITEMID_ESCAPEMENT		0
#define ITEMID_CASEMAP			0
#define ITEMID_NUMBERINFO		0
#define ITEMID_FONTLIST 		0
#define ITEMID_SEARCH			SCITEM_SEARCHDATA
#define ITEMID_ADJUST			0
#define ITEMID_LRSPACE			0
#define ITEMID_ULSPACE			0
#define ITEMID_PAPERBIN 		0
#define ITEMID_SIZE 			0
#define ITEMID_PAGE 			0
#define ITEMID_SETITEM			0
#define ITEMID_COLOR_TABLE		SID_COLOR_TABLE
#define ITEMID_GRADIENT_LIST	SID_GRADIENT_LIST
#define ITEMID_HATCH_LIST		SID_HATCH_LIST
#define ITEMID_BITMAP_LIST		SID_BITMAP_LIST
#define ITEMID_COLOR_TABLE		SID_COLOR_TABLE
#define ITEMID_DASH_LIST		SID_DASH_LIST
#define ITEMID_LINEEND_LIST 	SID_LINEEND_LIST
#define ITEMID_AUTHOR			0
#define ITEMID_DATE 			0
#define ITEMID_TEXT 			0

} //namespace binfilter
#endif


