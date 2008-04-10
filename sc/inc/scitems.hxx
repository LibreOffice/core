/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: scitems.hxx,v $
 * $Revision: 1.12 $
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
#define ATTR_USERDEF            121     // not saved in binary files
#define ATTR_FONT_WORDLINE      122
#define ATTR_FONT_RELIEF        123
#define ATTR_HYPHENATE          124
#define ATTR_SCRIPTSPACE        125
#define ATTR_HANGPUNCTUATION    126
#define ATTR_FORBIDDEN_RULES    127
#define ATTR_HOR_JUSTIFY        128
#define ATTR_INDENT             129
#define ATTR_VER_JUSTIFY        130
#define ATTR_STACKED            131
#define ATTR_ROTATE_VALUE       132
#define ATTR_ROTATE_MODE        133
#define ATTR_VERTICAL_ASIAN     134
#define ATTR_WRITINGDIR         135
#define ATTR_LINEBREAK          136
#define ATTR_SHRINKTOFIT        137
#define ATTR_BORDER_TLBR        138
#define ATTR_BORDER_BLTR        139
#define ATTR_MARGIN             140
#define ATTR_MERGE              141
#define ATTR_MERGE_FLAG         142
#define ATTR_VALUE_FORMAT       143
#define ATTR_LANGUAGE_FORMAT    144
#define ATTR_BACKGROUND         145
#define ATTR_PROTECTION         146
#define ATTR_BORDER             147
#define ATTR_BORDER_INNER       148     // innen, wg. Vorlagen-EditDialog
#define ATTR_SHADOW             149
#define ATTR_VALIDDATA          150
#define ATTR_CONDITIONAL        151

#define ATTR_PATTERN_END        151     // Ende Zellattribut-Pattern

#define ATTR_PATTERN            152
                                        // Seitenattribute
#define ATTR_LRSPACE            153     // Editor: PageDesc-TabPage
#define ATTR_ULSPACE            154
#define ATTR_PAGE               155
#define ATTR_PAGE_PAPERTRAY     156
#define ATTR_PAGE_PAPERBIN      157
#define ATTR_PAGE_SIZE          158
#define ATTR_PAGE_MAXSIZE       159
#define ATTR_PAGE_HORCENTER     160
#define ATTR_PAGE_VERCENTER     161

#define ATTR_PAGE_ON            162     // Editor: Header/Footer-Page
#define ATTR_PAGE_DYNAMIC       163
#define ATTR_PAGE_SHARED        164

#define ATTR_PAGE_NOTES         165     // Editor: Tabelle
#define ATTR_PAGE_GRID          166
#define ATTR_PAGE_HEADERS       167
#define ATTR_PAGE_CHARTS        168
#define ATTR_PAGE_OBJECTS       169
#define ATTR_PAGE_DRAWINGS      170
#define ATTR_PAGE_TOPDOWN       171
#define ATTR_PAGE_SCALE         172
#define ATTR_PAGE_SCALETOPAGES  173
#define ATTR_PAGE_FIRSTPAGENO   174

#define ATTR_PAGE_PRINTAREA     175     // Editor: Druckbereiche
#define ATTR_PAGE_REPEATROW     176
#define ATTR_PAGE_REPEATCOL     177
#define ATTR_PAGE_PRINTTABLES   178

#define ATTR_PAGE_HEADERLEFT    179     // Inhalte der Kopf-/
#define ATTR_PAGE_FOOTERLEFT    180     // Fusszeilen (links)
#define ATTR_PAGE_HEADERRIGHT   181     // Inhalte der Kopf-/
#define ATTR_PAGE_FOOTERRIGHT   182     // Fusszeilen (rechts)
#define ATTR_PAGE_HEADERSET     183     // die Sets dazu...
#define ATTR_PAGE_FOOTERSET     184

#define ATTR_PAGE_FORMULAS      185
#define ATTR_PAGE_NULLVALS      186

#define ATTR_PAGE_SCALETO       187     // #i8868# scale printout to with/height

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


