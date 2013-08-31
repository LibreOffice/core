/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef SC_ITEMS_HXX
#define SC_ITEMS_HXX

#include <svx/svxids.hrc>

//  EditEngine is not allowed to define it's own ITEMID's
#define _EEITEMID_HXX

// Item-IDs for UI-MsgPool:

//! shall be moved to the below-1000 range!

#define MSGPOOL_START               1100

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

#define MSGPOOL_END                 1109

// Item-IDs for attributes:

#define ATTR_STARTINDEX         100     // begin of attributes

#define ATTR_PATTERN_START      100     // begin of cell-attribute-pattern

#define ATTR_FONT               100     // begin of cell-attributes
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
#define ATTR_HOR_JUSTIFY_METHOD 130
#define ATTR_INDENT             131
#define ATTR_VER_JUSTIFY        132
#define ATTR_VER_JUSTIFY_METHOD 133
#define ATTR_STACKED            134
#define ATTR_ROTATE_VALUE       135
#define ATTR_ROTATE_MODE        136
#define ATTR_VERTICAL_ASIAN     137
#define ATTR_WRITINGDIR         138
#define ATTR_LINEBREAK          139
#define ATTR_SHRINKTOFIT        140
#define ATTR_BORDER_TLBR        141
#define ATTR_BORDER_BLTR        142
#define ATTR_MARGIN             143
#define ATTR_MERGE              144
#define ATTR_MERGE_FLAG         145
#define ATTR_VALUE_FORMAT       146
#define ATTR_LANGUAGE_FORMAT    147
#define ATTR_BACKGROUND         148
#define ATTR_PROTECTION         149
#define ATTR_BORDER             150
#define ATTR_BORDER_INNER       151     // inside, because of template-EditDialog
#define ATTR_SHADOW             152
#define ATTR_VALIDDATA          153
#define ATTR_CONDITIONAL        154
#define ATTR_HYPERLINK          155

#define ATTR_PATTERN_END        155     // end cell-attribute-pattern

#define ATTR_PATTERN            156
                                        // page attributes
#define ATTR_LRSPACE            157     // editor: PageDesc-TabPage
#define ATTR_ULSPACE            158
#define ATTR_PAGE               159
#define ATTR_PAGE_PAPERTRAY     160
#define ATTR_PAGE_PAPERBIN      161
#define ATTR_PAGE_SIZE          162
#define ATTR_PAGE_MAXSIZE       163
#define ATTR_PAGE_HORCENTER     164
#define ATTR_PAGE_VERCENTER     165

#define ATTR_PAGE_ON            166     // editor: header/footer-page
#define ATTR_PAGE_DYNAMIC       167
#define ATTR_PAGE_SHARED        168

#define ATTR_PAGE_NOTES         169     // editor: table
#define ATTR_PAGE_GRID          170
#define ATTR_PAGE_HEADERS       171
#define ATTR_PAGE_CHARTS        172
#define ATTR_PAGE_OBJECTS       173
#define ATTR_PAGE_DRAWINGS      174
#define ATTR_PAGE_TOPDOWN       175
#define ATTR_PAGE_SCALE         176
#define ATTR_PAGE_SCALETOPAGES  177
#define ATTR_PAGE_FIRSTPAGENO   178

#define ATTR_PAGE_PRINTAREA     179     // editor: print areas
#define ATTR_PAGE_REPEATROW     180
#define ATTR_PAGE_REPEATCOL     181
#define ATTR_PAGE_PRINTTABLES   182

#define ATTR_PAGE_HEADERLEFT    183     // contents of header/
#define ATTR_PAGE_FOOTERLEFT    184     // footer (left)
#define ATTR_PAGE_HEADERRIGHT   185     // contents of header/
#define ATTR_PAGE_FOOTERRIGHT   186     // footer (right)
#define ATTR_PAGE_HEADERSET     187     // the corresponding sets
#define ATTR_PAGE_FOOTERSET     188

#define ATTR_PAGE_FORMULAS      189
#define ATTR_PAGE_NULLVALS      190

#define ATTR_PAGE_SCALETO       191     // #i8868# scale printout to width/height

#define ATTR_HIDDEN             192

#define ATTR_ENDINDEX           ATTR_HIDDEN        // end of pool-range

// Dummy Slot-IDs for dialogs

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
