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
#ifndef _XMLOFF_TEXTPRMAP_HXX_
#define _XMLOFF_TEXTPRMAP_HXX_

#include <xmloff/contextid.hxx>
#include <xmloff/xmlprmap.hxx>

#define CTF_CHARHEIGHT               (XML_TEXT_CTF_START + 1)
#define CTF_CHARHEIGHT_REL           (XML_TEXT_CTF_START + 2)
#define CTF_PARALEFTMARGIN           (XML_TEXT_CTF_START + 3)
#define CTF_PARALEFTMARGIN_REL       (XML_TEXT_CTF_START + 4)
#define CTF_PARARIGHTMARGIN          (XML_TEXT_CTF_START + 5)
#define CTF_PARARIGHTMARGIN_REL      (XML_TEXT_CTF_START + 6)
#define CTF_PARAFIRSTLINE            (XML_TEXT_CTF_START + 7)
#define CTF_PARAFIRSTLINE_REL        (XML_TEXT_CTF_START + 8)
#define CTF_PARATOPMARGIN            (XML_TEXT_CTF_START + 9)
#define CTF_PARATOPMARGIN_REL       (XML_TEXT_CTF_START + 10)
#define CTF_PARABOTTOMMARGIN        (XML_TEXT_CTF_START + 11)
#define CTF_PARABOTTOMMARGIN_REL    (XML_TEXT_CTF_START + 12)
#define CTF_ALLBORDERWIDTH          (XML_TEXT_CTF_START + 13)
#define CTF_LEFTBORDERWIDTH         (XML_TEXT_CTF_START + 14)
#define CTF_RIGHTBORDERWIDTH        (XML_TEXT_CTF_START + 15)
#define CTF_TOPBORDERWIDTH          (XML_TEXT_CTF_START + 16)
#define CTF_BOTTOMBORDERWIDTH       (XML_TEXT_CTF_START + 17)
#define CTF_ALLBORDERDISTANCE       (XML_TEXT_CTF_START + 18)
#define CTF_LEFTBORDERDISTANCE      (XML_TEXT_CTF_START + 19)
#define CTF_RIGHTBORDERDISTANCE     (XML_TEXT_CTF_START + 20)
#define CTF_TOPBORDERDISTANCE       (XML_TEXT_CTF_START + 21)
#define CTF_BOTTOMBORDERDISTANCE    (XML_TEXT_CTF_START + 22)
#define CTF_ALLBORDER               (XML_TEXT_CTF_START + 23)
#define CTF_LEFTBORDER              (XML_TEXT_CTF_START + 24)
#define CTF_RIGHTBORDER             (XML_TEXT_CTF_START + 25)
#define CTF_TOPBORDER               (XML_TEXT_CTF_START + 26)
#define CTF_BOTTOMBORDER            (XML_TEXT_CTF_START + 27)
#define CTF_DROPCAPWHOLEWORD        (XML_TEXT_CTF_START + 28)
#define CTF_DROPCAPFORMAT           (XML_TEXT_CTF_START + 29)
#define CTF_DROPCAPCHARSTYLE        (XML_TEXT_CTF_START + 30)
#define CTF_TABSTOP                 (XML_TEXT_CTF_START + 31)
#define CTF_NUMBERINGSTYLENAME      (XML_TEXT_CTF_START + 32)
#define CTF_OLDTEXTBACKGROUND       (XML_TEXT_CTF_START + 33)
#define CTF_FONTFAMILYNAME          (XML_TEXT_CTF_START + 34)
#define CTF_FONTSTYLENAME           (XML_TEXT_CTF_START + 35)
#define CTF_FONTFAMILY              (XML_TEXT_CTF_START + 36)
#define CTF_FONTPITCH               (XML_TEXT_CTF_START + 37)
#define CTF_FONTCHARSET             (XML_TEXT_CTF_START + 38)
#define CTF_TEXTWRITINGMODE         (XML_TEXT_CTF_START + 39)
#define CTF_SYNCHEIGHT_MIN          (XML_TEXT_CTF_START + 40)
#define CTF_FRAMEHEIGHT_ABS         (XML_TEXT_CTF_START + 41)
#define CTF_FRAMEHEIGHT_REL         (XML_TEXT_CTF_START + 42)
#define CTF_FRAMEHEIGHT_MIN_ABS     (XML_TEXT_CTF_START + 43)
#define CTF_FRAMEHEIGHT_MIN_REL     (XML_TEXT_CTF_START + 44)
#define CTF_SIZETYPE                (XML_TEXT_CTF_START + 45)
#define CTF_SYNCHEIGHT              (XML_TEXT_CTF_START + 46)
#define CTF_WRAP                    (XML_TEXT_CTF_START + 47)
#define CTF_WRAP_CONTOUR            (XML_TEXT_CTF_START + 48)
#define CTF_WRAP_CONTOUR_MODE       (XML_TEXT_CTF_START + 49)
#define CTF_WRAP_PARAGRAPH_ONLY     (XML_TEXT_CTF_START + 50)
#define CTF_ANCHORTYPE              (XML_TEXT_CTF_START + 51)
#define CTF_ANCHORPAGENUMBER        (XML_TEXT_CTF_START + 52)
#define CTF_TEXTCOLUMNS             (XML_TEXT_CTF_START + 53)
#define CTF_HORIZONTALPOS           (XML_TEXT_CTF_START + 54)
#define CTF_HORIZONTALPOS_MIRRORED  (XML_TEXT_CTF_START + 55)
#define CTF_HORIZONTALREL           (XML_TEXT_CTF_START + 56)
#define CTF_HORIZONTALREL_FRAME     (XML_TEXT_CTF_START + 57)
#define CTF_HORIZONTALMIRROR        (XML_TEXT_CTF_START + 58)
#define CTF_VERTICALPOS             (XML_TEXT_CTF_START + 59)
#define CTF_VERTICALREL             (XML_TEXT_CTF_START + 60)
#define CTF_VERTICALREL_PAGE        (XML_TEXT_CTF_START + 61)
#define CTF_VERTICALREL_FRAME       (XML_TEXT_CTF_START + 62)
#define CTF_VERTICALREL_ASCHAR      (XML_TEXT_CTF_START + 63)
#define CTF_PAGEDESCNAME            (XML_TEXT_CTF_START + 64)
#define CTF_BACKGROUND_URL          (XML_TEXT_CTF_START + 66)
#define CTF_BACKGROUND_POS          (XML_TEXT_CTF_START + 67)
#define CTF_BACKGROUND_FILTER       (XML_TEXT_CTF_START + 68)
#define CTF_CHARHEIGHT_DIFF         (XML_TEXT_CTF_START + 69)
#define CTF_FONTNAME                (XML_TEXT_CTF_START + 70)
#define CTF_FONTNAME_CJK            (XML_TEXT_CTF_START + 71)
#define CTF_FONTFAMILYNAME_CJK      (XML_TEXT_CTF_START + 72)
#define CTF_FONTSTYLENAME_CJK       (XML_TEXT_CTF_START + 73)
#define CTF_FONTFAMILY_CJK          (XML_TEXT_CTF_START + 74)
#define CTF_FONTPITCH_CJK           (XML_TEXT_CTF_START + 75)
#define CTF_FONTCHARSET_CJK         (XML_TEXT_CTF_START + 76)
#define CTF_CHARHEIGHT_CJK          (XML_TEXT_CTF_START + 77)
#define CTF_CHARHEIGHT_REL_CJK      (XML_TEXT_CTF_START + 78)
#define CTF_CHARHEIGHT_DIFF_CJK     (XML_TEXT_CTF_START + 79)
#define CTF_FONTNAME_CTL            (XML_TEXT_CTF_START + 80)
#define CTF_FONTFAMILYNAME_CTL      (XML_TEXT_CTF_START + 81)
#define CTF_FONTSTYLENAME_CTL       (XML_TEXT_CTF_START + 82)
#define CTF_FONTFAMILY_CTL          (XML_TEXT_CTF_START + 83)
#define CTF_FONTPITCH_CTL           (XML_TEXT_CTF_START + 84)
#define CTF_FONTCHARSET_CTL         (XML_TEXT_CTF_START + 85)
#define CTF_CHARHEIGHT_CTL          (XML_TEXT_CTF_START + 86)
#define CTF_CHARHEIGHT_REL_CTL      (XML_TEXT_CTF_START + 87)
#define CTF_CHARHEIGHT_DIFF_CTL     (XML_TEXT_CTF_START + 88)
#define CTF_UNDERLINE               (XML_TEXT_CTF_START + 89)
#define CTF_UNDERLINE_COLOR         (XML_TEXT_CTF_START + 90)
#define CTF_UNDERLINE_HASCOLOR      (XML_TEXT_CTF_START + 91)
#define CTF_SECTION_FOOTNOTE_END            (XML_TEXT_CTF_START +  92)
#define CTF_SECTION_FOOTNOTE_NUM_RESTART    (XML_TEXT_CTF_START +  93)
#define CTF_SECTION_FOOTNOTE_NUM_RESTART_AT (XML_TEXT_CTF_START +  94)
#define CTF_SECTION_FOOTNOTE_NUM_TYPE       (XML_TEXT_CTF_START +  95)
#define CTF_SECTION_FOOTNOTE_NUM_PREFIX     (XML_TEXT_CTF_START +  96)
#define CTF_SECTION_FOOTNOTE_NUM_SUFFIX     (XML_TEXT_CTF_START +  97)
#define CTF_SECTION_FOOTNOTE_NUM_OWN        (XML_TEXT_CTF_START +  98)
#define CTF_SECTION_ENDNOTE_END             (XML_TEXT_CTF_START +  99)
#define CTF_SECTION_ENDNOTE_NUM_RESTART     (XML_TEXT_CTF_START + 100)
#define CTF_SECTION_ENDNOTE_NUM_RESTART_AT  (XML_TEXT_CTF_START + 101)
#define CTF_SECTION_ENDNOTE_NUM_TYPE        (XML_TEXT_CTF_START + 102)
#define CTF_SECTION_ENDNOTE_NUM_PREFIX      (XML_TEXT_CTF_START + 103)
#define CTF_SECTION_ENDNOTE_NUM_SUFFIX      (XML_TEXT_CTF_START + 104)
#define CTF_SECTION_ENDNOTE_NUM_OWN         (XML_TEXT_CTF_START + 105)
#define CTF_PAGE_FOOTNOTE                   (XML_TEXT_CTF_START + 106)
#define CTF_FRAME_DISPLAY_SCROLLBAR         (XML_TEXT_CTF_START + 107)
#define CTF_FRAME_DISPLAY_BORDER            (XML_TEXT_CTF_START + 108)
#define CTF_FRAME_MARGIN_HORI               (XML_TEXT_CTF_START + 109)
#define CTF_FRAME_MARGIN_VERT               (XML_TEXT_CTF_START + 110)
#define CTF_COMBINED_CHARACTERS_FIELD       (XML_TEXT_CTF_START + 111)
#define CTF_OLE_VIS_AREA_TOP                (XML_TEXT_CTF_START + 112)
#define CTF_OLE_VIS_AREA_LEFT               (XML_TEXT_CTF_START + 113)
#define CTF_OLE_VIS_AREA_WIDTH              (XML_TEXT_CTF_START + 114)
#define CTF_OLE_VIS_AREA_HEIGHT             (XML_TEXT_CTF_START + 115)
#define CTF_OLE_DRAW_ASPECT                 (XML_TEXT_CTF_START + 116)
#define CTF_HYPERLINK_URL                   (XML_TEXT_CTF_START + 117)
#define CTF_CHAR_STYLE_NAME                 (XML_TEXT_CTF_START + 118)
#define CTF_VERTICALPOS_ATCHAR              (XML_TEXT_CTF_START + 119)
#define CTF_BACKGROUND_TRANSPARENCY         (XML_TEXT_CTF_START + 120)
#define CTF_BACKGROUND_TRANSPARENT          (XML_TEXT_CTF_START + 121)
#define CTF_KEEP_TOGETHER                   (XML_TEXT_CTF_START + 122)
#define CTF_FRAMEWIDTH_ABS                  (XML_TEXT_CTF_START + 123)
#define CTF_FRAMEWIDTH_MIN_ABS              (XML_TEXT_CTF_START + 124)
#define CTF_FRAMEWIDTH_MIN_REL              (XML_TEXT_CTF_START + 125)
#define CTF_FRAMEWIDTH_REL                  (XML_TEXT_CTF_START + 126)
#define CTF_FRAMEWIDTH_TYPE                 (XML_TEXT_CTF_START + 127)
#define CTF_BORDER_MODEL                    (XML_TEXT_CTF_START + 128)
/* Define CTF ids for positioning properties of
   shapes, because on export to OpenOffice.org file format these have to
   be handled special, if shape isn't anchored as-character. (#i28749#)
*/
#define CTF_SHAPE_HORIZONTALPOS             (XML_TEXT_CTF_START + 129)
#define CTF_SHAPE_HORIZONTALPOS_MIRRORED    (XML_TEXT_CTF_START + 130)
#define CTF_SHAPE_HORIZONTALREL             (XML_TEXT_CTF_START + 131)
#define CTF_SHAPE_HORIZONTALREL_FRAME       (XML_TEXT_CTF_START + 132)
#define CTF_SHAPE_HORIZONTALMIRROR          (XML_TEXT_CTF_START + 133)
#define CTF_SHAPE_VERTICALPOS               (XML_TEXT_CTF_START + 134)
#define CTF_SHAPE_VERTICALPOS_ATCHAR        (XML_TEXT_CTF_START + 135)
#define CTF_SHAPE_VERTICALREL               (XML_TEXT_CTF_START + 136)
#define CTF_SHAPE_VERTICALREL_PAGE          (XML_TEXT_CTF_START + 137)
#define CTF_SHAPE_VERTICALREL_FRAME         (XML_TEXT_CTF_START + 138)
#define CTF_PARA_ADJUSTLAST                 (XML_TEXT_CTF_START + 139)
#define CTF_DEFAULT_OUTLINE_LEVEL           (XML_TEXT_CTF_START + 140)
#define CTF_OLD_FLOW_WITH_TEXT              (XML_TEXT_CTF_START + 141)
#define CTF_ISNUMBERING                     (XML_TEXT_CTF_START + 142)
#define CTF_TEXT_DISPLAY                    (XML_TEXT_CTF_START + 143)
#define CTF_TEXT_CLIP                       (XML_TEXT_CTF_START + 144)
#define CTF_TEXT_CLIP11                     (XML_TEXT_CTF_START + 145)
#define CTF_PARAMARGINALL                   (XML_TEXT_CTF_START + 146)
#define CTF_PARAMARGINALL_REL               (XML_TEXT_CTF_START + 147)
#define CTF_MARGINALL                       (XML_TEXT_CTF_START + 148)
#define CTF_MARGINLEFT                      (XML_TEXT_CTF_START + 149)
#define CTF_MARGINRIGHT                     (XML_TEXT_CTF_START + 150)
#define CTF_MARGINTOP                       (XML_TEXT_CTF_START + 151)
#define CTF_MARGINBOTTOM                    (XML_TEXT_CTF_START + 152)
/* CTF ids for character border to avoid interference between
   paragraph and character attributes */
#define CTF_CHARALLBORDERWIDTH              (XML_TEXT_CTF_START + 153)
#define CTF_CHARLEFTBORDERWIDTH             (XML_TEXT_CTF_START + 154)
#define CTF_CHARRIGHTBORDERWIDTH            (XML_TEXT_CTF_START + 155)
#define CTF_CHARTOPBORDERWIDTH              (XML_TEXT_CTF_START + 156)
#define CTF_CHARBOTTOMBORDERWIDTH           (XML_TEXT_CTF_START + 157)
#define CTF_CHARALLBORDERDISTANCE           (XML_TEXT_CTF_START + 158)
#define CTF_CHARLEFTBORDERDISTANCE          (XML_TEXT_CTF_START + 159)
#define CTF_CHARRIGHTBORDERDISTANCE         (XML_TEXT_CTF_START + 160)
#define CTF_CHARTOPBORDERDISTANCE           (XML_TEXT_CTF_START + 161)
#define CTF_CHARBOTTOMBORDERDISTANCE        (XML_TEXT_CTF_START + 162)
#define CTF_CHARALLBORDER                   (XML_TEXT_CTF_START + 163)
#define CTF_CHARLEFTBORDER                  (XML_TEXT_CTF_START + 164)
#define CTF_CHARRIGHTBORDER                 (XML_TEXT_CTF_START + 165)
#define CTF_CHARTOPBORDER                   (XML_TEXT_CTF_START + 166)
#define CTF_CHARBOTTOMBORDER                (XML_TEXT_CTF_START + 167)


#define TEXT_PROP_MAP_TEXT 0
#define TEXT_PROP_MAP_PARA 1
#define TEXT_PROP_MAP_FRAME 2
#define TEXT_PROP_MAP_AUTO_FRAME 3
#define TEXT_PROP_MAP_SECTION 4
#define TEXT_PROP_MAP_SHAPE 5
#define TEXT_PROP_MAP_RUBY 6
#define TEXT_PROP_MAP_SHAPE_PARA 7
#define TEXT_PROP_MAP_TEXT_ADDITIONAL_DEFAULTS 8
#define TEXT_PROP_MAP_TABLE_DEFAULTS 9
#define TEXT_PROP_MAP_TABLE_ROW_DEFAULTS 10

class XMLOFF_DLLPUBLIC XMLTextPropertySetMapper : public XMLPropertySetMapper
{
public:
    XMLTextPropertySetMapper( sal_uInt16 nType );
    virtual ~XMLTextPropertySetMapper();

    static const XMLPropertyMapEntry* getPropertyMapForType( sal_uInt16 _nType );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
