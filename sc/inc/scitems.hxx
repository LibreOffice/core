/*************************************************************************
 *
 *  $RCSfile: scitems.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: nn $ $Date: 2000-11-30 18:13:44 $
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

#ifndef SC_ITEMS_HXX
#define SC_ITEMS_HXX

#ifndef _SVX_SVXIDS_HRC
#include <svx/svxids.hrc>
#endif

//  EditEngine darf nicht ihre eigenen ITEMID's definieren
#define _EEITEMID_HXX

//--------------------------
// Item-IDs fuer UI-MsgPool:
//--------------------------

//! sollten auch nach unter 1000 verschoben werden!

#define MSGPOOL_START               1100
//--------------------------------------
#define SCITEM_STRING               1100
#define SCITEM_SEARCHDATA           1101
#define SCITEM_SORTDATA             1102
#define SCITEM_QUERYDATA            1103
#define SCITEM_SUBTDATA             1104
#define SCITEM_CONSOLIDATEDATA      1105
#define SCITEM_PIVOTDATA            1106
#define SCITEM_SOLVEDATA            1107
#define SCITEM_USERLIST             1108
#define SCITEM_PRINTWARN            1109
//--------------------------------------
#define MSGPOOL_END                 1109

//-------------------------
// Item-IDs fuer Attribute:
//-------------------------
#define ATTR_STARTINDEX         100     // Anfang Attribute

#define ATTR_PATTERN_START      100     // Anfang Zellattribut-Pattern

#define ATTR_FONT               100     // Anfang Zellattribute
#define ATTR_FONT_HEIGHT        101
#define ATTR_FONT_WEIGHT        102
#define ATTR_FONT_POSTURE       103
#define ATTR_FONT_UNDERLINE     104
#define ATTR_FONT_CROSSEDOUT    105
#define ATTR_FONT_CONTOUR       106
#define ATTR_FONT_SHADOWED      107
#define ATTR_FONT_COLOR         108
#define ATTR_FONT_LANGUAGE      109
#define ATTR_CJK_FONT           110
#define ATTR_CJK_FONT_HEIGHT    111
#define ATTR_CJK_FONT_WEIGHT    112
#define ATTR_CJK_FONT_POSTURE   113
#define ATTR_CJK_FONT_LANGUAGE  114
#define ATTR_CTL_FONT           115
#define ATTR_CTL_FONT_HEIGHT    116
#define ATTR_CTL_FONT_WEIGHT    117
#define ATTR_CTL_FONT_POSTURE   118
#define ATTR_CTL_FONT_LANGUAGE  119
#define ATTR_FONT_EMPHASISMARK  120
#define ATTR_FONT_TWOLINES      121
#define ATTR_SCRIPTSPACE        122
#define ATTR_HANGPUNCTUATION    123
#define ATTR_FORBIDDEN_RULES    124
#define ATTR_HOR_JUSTIFY        125
#define ATTR_INDENT             126
#define ATTR_VER_JUSTIFY        127
#define ATTR_ORIENTATION        128
#define ATTR_ROTATE_VALUE       129
#define ATTR_ROTATE_MODE        130
#define ATTR_LINEBREAK          131
#define ATTR_MARGIN             132
#define ATTR_MERGE              133
#define ATTR_MERGE_FLAG         134
#define ATTR_VALUE_FORMAT       135
#define ATTR_LANGUAGE_FORMAT    136
#define ATTR_BACKGROUND         137
#define ATTR_PROTECTION         138
#define ATTR_BORDER             139
#define ATTR_BORDER_INNER       140     // innen, wg. Vorlagen-EditDialog
#define ATTR_SHADOW             141
#define ATTR_VALIDDATA          142
#define ATTR_CONDITIONAL        143

#define ATTR_PATTERN_END        143     // Ende Zellattribut-Pattern

#define ATTR_PATTERN            144
                                        // Seitenattribute
#define ATTR_LRSPACE            145     // Editor: PageDesc-TabPage
#define ATTR_ULSPACE            146
#define ATTR_PAGE               147
#define ATTR_PAGE_PAPERTRAY     148
#define ATTR_PAGE_PAPERBIN      149
#define ATTR_PAGE_SIZE          150
#define ATTR_PAGE_MAXSIZE       151
#define ATTR_PAGE_HORCENTER     152
#define ATTR_PAGE_VERCENTER     153

#define ATTR_PAGE_ON            154     // Editor: Header/Footer-Page
#define ATTR_PAGE_DYNAMIC       155
#define ATTR_PAGE_SHARED        156

#define ATTR_PAGE_NOTES         157     // Editor: Tabelle
#define ATTR_PAGE_GRID          158
#define ATTR_PAGE_HEADERS       159
#define ATTR_PAGE_CHARTS        160
#define ATTR_PAGE_OBJECTS       161
#define ATTR_PAGE_DRAWINGS      162
#define ATTR_PAGE_TOPDOWN       163
#define ATTR_PAGE_SCALE         164
#define ATTR_PAGE_SCALETOPAGES  165
#define ATTR_PAGE_FIRSTPAGENO   166

#define ATTR_PAGE_PRINTAREA     167     // Editor: Druckbereiche
#define ATTR_PAGE_REPEATROW     168
#define ATTR_PAGE_REPEATCOL     169
#define ATTR_PAGE_PRINTTABLES   170

#define ATTR_PAGE_HEADERLEFT    171     // Inhalte der Kopf-/
#define ATTR_PAGE_FOOTERLEFT    172     // Fusszeilen (links)
#define ATTR_PAGE_HEADERRIGHT   173     // Inhalte der Kopf-/
#define ATTR_PAGE_FOOTERRIGHT   174     // Fusszeilen (rechts)
#define ATTR_PAGE_HEADERSET     175     // die Sets dazu...
#define ATTR_PAGE_FOOTERSET     176

#define ATTR_PAGE_FORMULAS      177
#define ATTR_PAGE_NULLVALS      178

#define ATTR_ENDINDEX           ATTR_PAGE_NULLVALS      // Ende Pool-Range

//--------------------------------
// Dummy Slot-IDs fuer Dialoge
//--------------------------------
#define SID_SCATTR_PAGE_NOTES           ATTR_PAGE_NOTES
#define SID_SCATTR_PAGE_GRID            ATTR_PAGE_GRID
#define SID_SCATTR_PAGE_HEADERS         ATTR_PAGE_HEADERS
#define SID_SCATTR_PAGE_CHARTS          ATTR_PAGE_CHARTS
#define SID_SCATTR_PAGE_OBJECTS         ATTR_PAGE_OBJECTS
#define SID_SCATTR_PAGE_DRAWINGS        ATTR_PAGE_DRAWINGS
#define SID_SCATTR_PAGE_TOPDOWN         ATTR_PAGE_TOPDOWN
#define SID_SCATTR_PAGE_SCALE           ATTR_PAGE_SCALE
#define SID_SCATTR_PAGE_SCALETOPAGES    ATTR_PAGE_SCALETOPAGES
#define SID_SCATTR_PAGE_FIRSTPAGENO     ATTR_PAGE_FIRSTPAGENO
#define SID_SCATTR_PAGE_PRINTAREA       ATTR_PAGE_PRINTAREA
#define SID_SCATTR_PAGE_REPEATROW       ATTR_PAGE_REPEATROW
#define SID_SCATTR_PAGE_REPEATCOL       ATTR_PAGE_REPEATCOL
#define SID_SCATTR_PAGE_PRINTTABLES     ATTR_PAGE_PRINTTABLES
#define SID_SCATTR_PAGE_HEADERLEFT      ATTR_PAGE_HEADERLEFT
#define SID_SCATTR_PAGE_FOOTERLEFT      ATTR_PAGE_FOOTERLEFT
#define SID_SCATTR_PAGE_HEADERRIGHT     ATTR_PAGE_HEADERRIGHT
#define SID_SCATTR_PAGE_FOOTERRIGHT     ATTR_PAGE_FOOTERRIGHT
#define SID_SCATTR_PAGE_FORMULAS        ATTR_PAGE_FORMULAS
#define SID_SCATTR_PAGE_NULLVALS        ATTR_PAGE_NULLVALS

//--------------------------------
// IDs fuer Svx-Items:
//--------------------------------
#define ITEMID_LINE             SID_FRAME_LINESTYLE
#define ITEMID_BOX              ATTR_BORDER
#define ITEMID_BOXINFO          ATTR_BORDER_INNER
#define ITEMID_SHADOW           ATTR_SHADOW
#define ITEMID_BRUSH            ATTR_BACKGROUND
#define ITEMID_FONT             ATTR_FONT
#define ITEMID_FONTHEIGHT       ATTR_FONT_HEIGHT
#define ITEMID_WEIGHT           ATTR_FONT_WEIGHT
#define ITEMID_POSTURE          ATTR_FONT_POSTURE
#define ITEMID_UNDERLINE        ATTR_FONT_UNDERLINE
#define ITEMID_CROSSEDOUT       ATTR_FONT_CROSSEDOUT
#define ITEMID_CONTOUR          ATTR_FONT_CONTOUR
#define ITEMID_SHADOWED         ATTR_FONT_SHADOWED
#define ITEMID_COLOR            ATTR_FONT_COLOR
#define ITEMID_LANGUAGE         ATTR_FONT_LANGUAGE
#define ITEMID_EMPHASISMARK     ATTR_FONT_EMPHASISMARK
#define ITEMID_TWOLINES         ATTR_FONT_TWOLINES
#define ITEMID_SCRIPTSPACE      ATTR_SCRIPTSPACE
#define ITEMID_HANGINGPUNCTUATION   ATTR_HANGPUNCTUATION
#define ITEMID_FORBIDDENRULE    ATTR_FORBIDDEN_RULES
#define ITEMID_HORJUSTIFY       ATTR_HOR_JUSTIFY
#define ITEMID_VERJUSTIFY       ATTR_VER_JUSTIFY
#define ITEMID_ORIENTATION      ATTR_ORIENTATION
#define ITEMID_LINEBREAK        ATTR_LINEBREAK
#define ITEMID_MARGIN           ATTR_MARGIN
#define ITEMID_ESCAPEMENT       0
#define ITEMID_CASEMAP          0
#define ITEMID_NUMBERINFO       0
#define ITEMID_FONTLIST         0
#define ITEMID_SEARCH           SCITEM_SEARCHDATA
#define ITEMID_ADJUST           0
#define ITEMID_LRSPACE          0
#define ITEMID_ULSPACE          0
#define ITEMID_PAPERBIN         0
#define ITEMID_SIZE             0
#define ITEMID_PAGE             0
#define ITEMID_SETITEM          0
#define ITEMID_COLOR_TABLE      SID_COLOR_TABLE
#define ITEMID_GRADIENT_LIST    SID_GRADIENT_LIST
#define ITEMID_HATCH_LIST       SID_HATCH_LIST
#define ITEMID_BITMAP_LIST      SID_BITMAP_LIST
#define ITEMID_COLOR_TABLE      SID_COLOR_TABLE
#define ITEMID_DASH_LIST        SID_DASH_LIST
#define ITEMID_LINEEND_LIST     SID_LINEEND_LIST
#define ITEMID_AUTHOR           0
#define ITEMID_DATE             0
#define ITEMID_TEXT             0

#endif


