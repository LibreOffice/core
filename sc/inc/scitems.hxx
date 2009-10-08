/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: scitems.hxx,v $
 * $Revision: 1.12.144.1 $
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
#define ATTR_FONT_OVERLINE      105
#define ATTR_FONT_CROSSEDOUT    106
#define ATTR_FONT_CONTOUR       107
#define ATTR_FONT_SHADOWED      108
#define ATTR_FONT_COLOR         109
#define ATTR_FONT_LANGUAGE      110
#define ATTR_CJK_FONT           111
#define ATTR_CJK_FONT_HEIGHT    112
#define ATTR_CJK_FONT_WEIGHT    113
#define ATTR_CJK_FONT_POSTURE   114
#define ATTR_CJK_FONT_LANGUAGE  115
#define ATTR_CTL_FONT           116
#define ATTR_CTL_FONT_HEIGHT    117
#define ATTR_CTL_FONT_WEIGHT    118
#define ATTR_CTL_FONT_POSTURE   119
#define ATTR_CTL_FONT_LANGUAGE  120
#define ATTR_FONT_EMPHASISMARK  121
#define ATTR_USERDEF            122     // not saved in binary files
#define ATTR_FONT_WORDLINE      123
#define ATTR_FONT_RELIEF        124
#define ATTR_HYPHENATE          125
#define ATTR_SCRIPTSPACE        126
#define ATTR_HANGPUNCTUATION    127
#define ATTR_FORBIDDEN_RULES    128
#define ATTR_HOR_JUSTIFY        129
#define ATTR_INDENT             130
#define ATTR_VER_JUSTIFY        131
#define ATTR_STACKED            132
#define ATTR_ROTATE_VALUE       133
#define ATTR_ROTATE_MODE        134
#define ATTR_VERTICAL_ASIAN     135
#define ATTR_WRITINGDIR         136
#define ATTR_LINEBREAK          137
#define ATTR_SHRINKTOFIT        138
#define ATTR_BORDER_TLBR        139
#define ATTR_BORDER_BLTR        140
#define ATTR_MARGIN             141
#define ATTR_MERGE              142
#define ATTR_MERGE_FLAG         143
#define ATTR_VALUE_FORMAT       144
#define ATTR_LANGUAGE_FORMAT    145
#define ATTR_BACKGROUND         146
#define ATTR_PROTECTION         147
#define ATTR_BORDER             148
#define ATTR_BORDER_INNER       149     // innen, wg. Vorlagen-EditDialog
#define ATTR_SHADOW             150
#define ATTR_VALIDDATA          151
#define ATTR_CONDITIONAL        152

#define ATTR_PATTERN_END        152     // Ende Zellattribut-Pattern

#define ATTR_PATTERN            153
                                        // Seitenattribute
#define ATTR_LRSPACE            154     // Editor: PageDesc-TabPage
#define ATTR_ULSPACE            155
#define ATTR_PAGE               156
#define ATTR_PAGE_PAPERTRAY     157
#define ATTR_PAGE_PAPERBIN      158
#define ATTR_PAGE_SIZE          159
#define ATTR_PAGE_MAXSIZE       160
#define ATTR_PAGE_HORCENTER     161
#define ATTR_PAGE_VERCENTER     162

#define ATTR_PAGE_ON            163     // Editor: Header/Footer-Page
#define ATTR_PAGE_DYNAMIC       164
#define ATTR_PAGE_SHARED        165

#define ATTR_PAGE_NOTES         166     // Editor: Tabelle
#define ATTR_PAGE_GRID          167
#define ATTR_PAGE_HEADERS       168
#define ATTR_PAGE_CHARTS        169
#define ATTR_PAGE_OBJECTS       170
#define ATTR_PAGE_DRAWINGS      171
#define ATTR_PAGE_TOPDOWN       172
#define ATTR_PAGE_SCALE         173
#define ATTR_PAGE_SCALETOPAGES  174
#define ATTR_PAGE_FIRSTPAGENO   175

#define ATTR_PAGE_PRINTAREA     176     // Editor: Druckbereiche
#define ATTR_PAGE_REPEATROW     177
#define ATTR_PAGE_REPEATCOL     178
#define ATTR_PAGE_PRINTTABLES   179

#define ATTR_PAGE_HEADERLEFT    180     // Inhalte der Kopf-/
#define ATTR_PAGE_FOOTERLEFT    181     // Fusszeilen (links)
#define ATTR_PAGE_HEADERRIGHT   182     // Inhalte der Kopf-/
#define ATTR_PAGE_FOOTERRIGHT   183     // Fusszeilen (rechts)
#define ATTR_PAGE_HEADERSET     184     // die Sets dazu...
#define ATTR_PAGE_FOOTERSET     185

#define ATTR_PAGE_FORMULAS      186
#define ATTR_PAGE_NULLVALS      187

#define ATTR_PAGE_SCALETO       188     // #i8868# scale printout to with/height

#define ATTR_ENDINDEX           ATTR_PAGE_SCALETO        // Ende Pool-Range

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
#define SID_SCATTR_PAGE_SCALETO         ATTR_PAGE_SCALETO

#endif


