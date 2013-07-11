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

#include <hintids.hxx>

#include <svx/svxids.hrc>
#include <comphelper/TypeGeneration.hxx>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/text/PageNumberType.hpp>
#include <com/sun/star/script/XLibraryContainer.hpp>
// #i28749#
#include <com/sun/star/drawing/HomogenMatrix3.hpp>
#include <swtypes.hxx>
#include <unomap.hxx>
#include <unoprnms.hxx>
#include <unomid.h>
#include <cmdid.h>
#include <unofldmid.h>
#include <editeng/memberids.hrc>

using namespace ::com::sun::star;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::comphelper;

#define SW_PROP_NMID(id)    ((const char*)id), 0

#ifndef MID_TXT_LMARGIN
#define MID_TXT_LMARGIN 11
#endif

/******************************************************************************
    UI-Maps
******************************************************************************/

SwUnoPropertyMapProvider aSwMapProvider;

SwUnoPropertyMapProvider::SwUnoPropertyMapProvider()
{
    for( sal_uInt16 i = 0; i < PROPERTY_MAP_END; i++ )
    {
        aMapEntriesArr[i] = 0;
        aPropertySetArr[i] = 0;
    }
}

SwUnoPropertyMapProvider::~SwUnoPropertyMapProvider()
{
}

#define _STANDARD_FONT_PROPERTIES \
    { SW_PROP_NMID(UNO_NAME_CHAR_HEIGHT), RES_CHRATR_FONTSIZE  ,  CPPU_E2T(CPPUTYPE_FLOAT),         PropertyAttribute::MAYBEVOID, MID_FONTHEIGHT|CONVERT_TWIPS},      \
    { SW_PROP_NMID(UNO_NAME_CHAR_WEIGHT), RES_CHRATR_WEIGHT    ,  CPPU_E2T(CPPUTYPE_FLOAT),             PropertyAttribute::MAYBEVOID, MID_WEIGHT},                    \
    { SW_PROP_NMID(UNO_NAME_CHAR_FONT_NAME), RES_CHRATR_FONT,       CPPU_E2T(CPPUTYPE_OUSTRING),  PropertyAttribute::MAYBEVOID, MID_FONT_FAMILY_NAME },                  \
    { SW_PROP_NMID(UNO_NAME_CHAR_FONT_STYLE_NAME), RES_CHRATR_FONT,     CPPU_E2T(CPPUTYPE_OUSTRING), PropertyAttribute::MAYBEVOID, MID_FONT_STYLE_NAME },                    \
    { SW_PROP_NMID(UNO_NAME_CHAR_FONT_FAMILY), RES_CHRATR_FONT,     CPPU_E2T(CPPUTYPE_INT16),                   PropertyAttribute::MAYBEVOID, MID_FONT_FAMILY   },    \
    { SW_PROP_NMID(UNO_NAME_CHAR_FONT_CHAR_SET), RES_CHRATR_FONT,       CPPU_E2T(CPPUTYPE_INT16),   PropertyAttribute::MAYBEVOID, MID_FONT_CHAR_SET },                    \
    { SW_PROP_NMID(UNO_NAME_CHAR_FONT_PITCH), RES_CHRATR_FONT,      CPPU_E2T(CPPUTYPE_INT16),                   PropertyAttribute::MAYBEVOID, MID_FONT_PITCH   },     \
    { SW_PROP_NMID(UNO_NAME_CHAR_POSTURE), RES_CHRATR_POSTURE   ,  CPPU_E2T(CPPUTYPE_FONTSLANT),        PropertyAttribute::MAYBEVOID, MID_POSTURE},                   \
    { SW_PROP_NMID(UNO_NAME_RSID), RES_CHRATR_RSID, CPPU_E2T(CPPUTYPE_INT32), PropertyAttribute::MAYBEVOID, 0 }, \
    { SW_PROP_NMID(UNO_NAME_CHAR_LOCALE), RES_CHRATR_LANGUAGE ,   CPPU_E2T(CPPUTYPE_LOCALE)  ,          PropertyAttribute::MAYBEVOID,  MID_LANG_LOCALE },

#define _CJK_FONT_PROPERTIES \
    { SW_PROP_NMID(UNO_NAME_CHAR_HEIGHT_ASIAN), RES_CHRATR_CJK_FONTSIZE  ,  CPPU_E2T(CPPUTYPE_FLOAT),           PropertyAttribute::MAYBEVOID, MID_FONTHEIGHT|CONVERT_TWIPS},   \
    { SW_PROP_NMID(UNO_NAME_CHAR_WEIGHT_ASIAN), RES_CHRATR_CJK_WEIGHT    ,  CPPU_E2T(CPPUTYPE_FLOAT),           PropertyAttribute::MAYBEVOID, MID_WEIGHT},                 \
    { SW_PROP_NMID(UNO_NAME_CHAR_FONT_NAME_ASIAN), RES_CHRATR_CJK_FONT,     CPPU_E2T(CPPUTYPE_OUSTRING),  PropertyAttribute::MAYBEVOID, MID_FONT_FAMILY_NAME },               \
    { SW_PROP_NMID(UNO_NAME_CHAR_FONT_STYLE_NAME_ASIAN), RES_CHRATR_CJK_FONT,       CPPU_E2T(CPPUTYPE_OUSTRING), PropertyAttribute::MAYBEVOID, MID_FONT_STYLE_NAME },                 \
    { SW_PROP_NMID(UNO_NAME_CHAR_FONT_FAMILY_ASIAN), RES_CHRATR_CJK_FONT,       CPPU_E2T(CPPUTYPE_INT16),                   PropertyAttribute::MAYBEVOID, MID_FONT_FAMILY   },    \
    { SW_PROP_NMID(UNO_NAME_CHAR_FONT_CHAR_SET_ASIAN), RES_CHRATR_CJK_FONT,     CPPU_E2T(CPPUTYPE_INT16),   PropertyAttribute::MAYBEVOID, MID_FONT_CHAR_SET },                 \
    { SW_PROP_NMID(UNO_NAME_CHAR_FONT_PITCH_ASIAN), RES_CHRATR_CJK_FONT,        CPPU_E2T(CPPUTYPE_INT16),                   PropertyAttribute::MAYBEVOID, MID_FONT_PITCH   },     \
    { SW_PROP_NMID(UNO_NAME_CHAR_POSTURE_ASIAN), RES_CHRATR_CJK_POSTURE   ,  CPPU_E2T(CPPUTYPE_FONTSLANT),          PropertyAttribute::MAYBEVOID, MID_POSTURE},                \
    { SW_PROP_NMID(UNO_NAME_CHAR_LOCALE_ASIAN), RES_CHRATR_CJK_LANGUAGE ,   CPPU_E2T(CPPUTYPE_LOCALE)  ,        PropertyAttribute::MAYBEVOID,  MID_LANG_LOCALE },

#define _CTL_FONT_PROPERTIES \
    { SW_PROP_NMID(UNO_NAME_CHAR_HEIGHT_COMPLEX), RES_CHRATR_CTL_FONTSIZE  ,  CPPU_E2T(CPPUTYPE_FLOAT),         PropertyAttribute::MAYBEVOID, MID_FONTHEIGHT|CONVERT_TWIPS},\
    { SW_PROP_NMID(UNO_NAME_CHAR_WEIGHT_COMPLEX), RES_CHRATR_CTL_WEIGHT    ,  CPPU_E2T(CPPUTYPE_FLOAT),             PropertyAttribute::MAYBEVOID, MID_WEIGHT},              \
    { SW_PROP_NMID(UNO_NAME_CHAR_FONT_NAME_COMPLEX), RES_CHRATR_CTL_FONT,       CPPU_E2T(CPPUTYPE_OUSTRING),  PropertyAttribute::MAYBEVOID, MID_FONT_FAMILY_NAME },            \
    { SW_PROP_NMID(UNO_NAME_CHAR_FONT_STYLE_NAME_COMPLEX), RES_CHRATR_CTL_FONT,     CPPU_E2T(CPPUTYPE_OUSTRING), PropertyAttribute::MAYBEVOID, MID_FONT_STYLE_NAME },              \
    { SW_PROP_NMID(UNO_NAME_CHAR_FONT_FAMILY_COMPLEX), RES_CHRATR_CTL_FONT,     CPPU_E2T(CPPUTYPE_INT16),                   PropertyAttribute::MAYBEVOID, MID_FONT_FAMILY   },    \
    { SW_PROP_NMID(UNO_NAME_CHAR_FONT_CHAR_SET_COMPLEX), RES_CHRATR_CTL_FONT,       CPPU_E2T(CPPUTYPE_INT16),   PropertyAttribute::MAYBEVOID, MID_FONT_CHAR_SET },              \
    { SW_PROP_NMID(UNO_NAME_CHAR_FONT_PITCH_COMPLEX), RES_CHRATR_CTL_FONT,      CPPU_E2T(CPPUTYPE_INT16),                   PropertyAttribute::MAYBEVOID, MID_FONT_PITCH   },     \
    { SW_PROP_NMID(UNO_NAME_CHAR_POSTURE_COMPLEX), RES_CHRATR_CTL_POSTURE   ,  CPPU_E2T(CPPUTYPE_FONTSLANT),        PropertyAttribute::MAYBEVOID, MID_POSTURE},             \
    { SW_PROP_NMID(UNO_NAME_CHAR_LOCALE_COMPLEX), RES_CHRATR_CTL_LANGUAGE ,   CPPU_E2T(CPPUTYPE_LOCALE)  ,          PropertyAttribute::MAYBEVOID,  MID_LANG_LOCALE },

#define _REDLINE_NODE_PROPERTIES \
    { SW_PROP_NMID(UNO_NAME_START_REDLINE), FN_UNO_REDLINE_NODE_START , CPPU_E2T(CPPUTYPE_PROPERTYVALUE),   PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY,  0xff }, \
    { SW_PROP_NMID(UNO_NAME_END_REDLINE), FN_UNO_REDLINE_NODE_END ,     CPPU_E2T(CPPUTYPE_PROPERTYVALUE),       PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY,  0xff },

#define _REDLINE_PROPERTIES \
    {SW_PROP_NMID(UNO_NAME_REDLINE_AUTHOR), 0, CPPU_E2T(CPPUTYPE_OUSTRING),                     PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY,   0},\
    {SW_PROP_NMID(UNO_NAME_REDLINE_DATE_TIME), 0, CPPU_E2T(CPPUTYPE_DATETIME),                  PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY,   0},\
    {SW_PROP_NMID(UNO_NAME_REDLINE_COMMENT), 0, CPPU_E2T(CPPUTYPE_OUSTRING),                        PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY,   0},\
    {SW_PROP_NMID(UNO_NAME_REDLINE_TYPE), 0, CPPU_E2T(CPPUTYPE_OUSTRING),                       PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY,   0},\
    {SW_PROP_NMID(UNO_NAME_REDLINE_SUCCESSOR_DATA), 0, CPPU_E2T(CPPUTYPE_PROPERTYVALUE),    PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY,   0},\
    {SW_PROP_NMID(UNO_NAME_REDLINE_IDENTIFIER), 0, CPPU_E2T(CPPUTYPE_OUSTRING),                         PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY, 0},\
    {SW_PROP_NMID(UNO_NAME_IS_IN_HEADER_FOOTER), 0, CPPU_E2T(CPPUTYPE_BOOLEAN),                             PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY, 0},\
    {SW_PROP_NMID(UNO_NAME_REDLINE_TEXT), 0, CPPU_E2T(CPPUTYPE_REFTEXT),                    PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY,   0},\
    {SW_PROP_NMID(UNO_NAME_MERGE_LAST_PARA), 0, CPPU_E2T(CPPUTYPE_BOOLEAN),                             PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY, 0},

#define COMMON_CRSR_PARA_PROPERTIES_FN_ONLY \
        { SW_PROP_NMID(UNO_NAME_PARA_STYLE_NAME), FN_UNO_PARA_STYLE,        CPPU_E2T(CPPUTYPE_OUSTRING),                PropertyAttribute::MAYBEVOID,     0},                                                       \
        { SW_PROP_NMID(UNO_NAME_PAGE_STYLE_NAME), FN_UNO_PAGE_STYLE,        CPPU_E2T(CPPUTYPE_OUSTRING),         PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY,   0},                       \
        { SW_PROP_NMID(UNO_NAME_NUMBERING_IS_NUMBER), FN_UNO_IS_NUMBER,     CPPU_E2T(CPPUTYPE_BOOLEAN)  ,       PropertyAttribute::MAYBEVOID,     0},                                                                 \
        { SW_PROP_NMID(UNO_NAME_NUMBERING_LEVEL), FN_UNO_NUM_LEVEL,     CPPU_E2T(CPPUTYPE_INT16),           PropertyAttribute::MAYBEVOID, 0},                                                            \
        { SW_PROP_NMID(UNO_NAME_NUMBERING_RULES), FN_UNO_NUM_RULES,     CPPU_E2T(CPPUTYPE_REFIDXREPL),  PropertyAttribute::MAYBEVOID, CONVERT_TWIPS},                        \
        { SW_PROP_NMID(UNO_NAME_NUMBERING_START_VALUE), FN_UNO_NUM_START_VALUE, CPPU_E2T(CPPUTYPE_INT16),           PropertyAttribute::MAYBEVOID, CONVERT_TWIPS},                                                \
        { SW_PROP_NMID(UNO_NAME_DOCUMENT_INDEX), FN_UNO_DOCUMENT_INDEX, CPPU_E2T(CPPUTYPE_REFDOCINDEX), PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY ,0 },            \
        { SW_PROP_NMID(UNO_NAME_TEXT_TABLE), FN_UNO_TEXT_TABLE,     CPPU_E2T(CPPUTYPE_REFTXTTABLE),     PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY ,0 },               \
        { SW_PROP_NMID(UNO_NAME_CELL), FN_UNO_CELL,         CPPU_E2T(CPPUTYPE_REFCELL),         PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY ,0 },                     \
        { SW_PROP_NMID(UNO_NAME_TEXT_FRAME), FN_UNO_TEXT_FRAME,     CPPU_E2T(CPPUTYPE_REFTEXTFRAME),        PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY ,0 },                     \
        { SW_PROP_NMID(UNO_NAME_TEXT_SECTION), FN_UNO_TEXT_SECTION, CPPU_E2T(CPPUTYPE_REFTEXTSECTION),  PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY ,0 },                    \
        { SW_PROP_NMID(UNO_NAME_PARA_CHAPTER_NUMBERING_LEVEL), FN_UNO_PARA_CHAPTER_NUMBERING_LEVEL,CPPU_E2T(CPPUTYPE_INT8), PROPERTY_NONE, 0},                                                     \
        { SW_PROP_NMID(UNO_NAME_PARA_CONDITIONAL_STYLE_NAME), FN_UNO_PARA_CONDITIONAL_STYLE_NAME, CPPU_E2T(CPPUTYPE_OUSTRING),      PropertyAttribute::READONLY, 0},                                                     \
        { SW_PROP_NMID(UNO_NAME_LIST_ID), FN_UNO_LIST_ID, CPPU_E2T(CPPUTYPE_OUSTRING), PropertyAttribute::MAYBEVOID, 0}, \
        { SW_PROP_NMID(UNO_NAME_PARA_IS_NUMBERING_RESTART), FN_NUMBER_NEWSTART,     CPPU_E2T(CPPUTYPE_BOOLEAN),     PropertyAttribute::MAYBEVOID, 0 }, \
        { SW_PROP_NMID(UNO_NAME_PARA_CONTINUEING_PREVIOUS_SUB_TREE), FN_UNO_PARA_CONT_PREV_SUBTREE, CPPU_E2T(CPPUTYPE_BOOLEAN), PropertyAttribute::READONLY, 0 }, \
        { SW_PROP_NMID(UNO_NAME_PARA_LIST_LABEL_STRING), FN_UNO_PARA_NUM_STRING, CPPU_E2T(CPPUTYPE_OUSTRING), PropertyAttribute::READONLY, 0 }, \
        { SW_PROP_NMID(UNO_NAME_OUTLINE_LEVEL), RES_PARATR_OUTLINELEVEL,        CPPU_E2T(CPPUTYPE_INT16),                PropertyAttribute::MAYBEVOID,     0},

#define COMMON_HYPERLINK_PROPERTIES \
        { SW_PROP_NMID(UNO_NAME_HYPER_LINK_U_R_L), RES_TXTATR_INETFMT,          CPPU_E2T(CPPUTYPE_OUSTRING), PropertyAttribute::MAYBEVOID ,MID_URL_URL},                \
        { SW_PROP_NMID(UNO_NAME_HYPER_LINK_TARGET), RES_TXTATR_INETFMT,         CPPU_E2T(CPPUTYPE_OUSTRING), PropertyAttribute::MAYBEVOID ,MID_URL_TARGET},             \
        { SW_PROP_NMID(UNO_NAME_HYPER_LINK_NAME), RES_TXTATR_INETFMT,           CPPU_E2T(CPPUTYPE_OUSTRING), PropertyAttribute::MAYBEVOID ,MID_URL_HYPERLINKNAME  },    \
        { SW_PROP_NMID(UNO_NAME_UNVISITED_CHAR_STYLE_NAME), RES_TXTATR_INETFMT, CPPU_E2T(CPPUTYPE_OUSTRING), PropertyAttribute::MAYBEVOID ,MID_URL_UNVISITED_FMT   },   \
        { SW_PROP_NMID(UNO_NAME_VISITED_CHAR_STYLE_NAME), RES_TXTATR_INETFMT,   CPPU_E2T(CPPUTYPE_OUSTRING), PropertyAttribute::MAYBEVOID ,MID_URL_VISITED_FMT  },

// same as COMMON_CRSR_PARA_PROPERTIES_WITHOUT_FN_01 but without
// UNO_NAME_BREAK_TYPE and UNO_NAME_PAGE_DESC_NAME which can not be used
// by the SwXTextTableCursor
#define COMMON_CRSR_PARA_PROPERTIES_WITHOUT_FN_01 \
        { SW_PROP_NMID(UNO_NAME_PARRSID), RES_PARATR_RSID, CPPU_E2T(CPPUTYPE_INT32), PropertyAttribute::MAYBEVOID, 0 }, \
        { SW_PROP_NMID(UNO_NAME_PARA_IS_HYPHENATION), RES_PARATR_HYPHENZONE,        CPPU_E2T(CPPUTYPE_BOOLEAN),     PropertyAttribute::MAYBEVOID, MID_IS_HYPHEN         },                                        \
        { SW_PROP_NMID(UNO_NAME_PARA_HYPHENATION_MAX_LEADING_CHARS), RES_PARATR_HYPHENZONE,         CPPU_E2T(CPPUTYPE_INT16),   PropertyAttribute::MAYBEVOID, MID_HYPHEN_MIN_LEAD   },                              \
        { SW_PROP_NMID(UNO_NAME_PARA_HYPHENATION_MAX_TRAILING_CHARS), RES_PARATR_HYPHENZONE,        CPPU_E2T(CPPUTYPE_INT16),   PropertyAttribute::MAYBEVOID, MID_HYPHEN_MIN_TRAIL  },                              \
        { SW_PROP_NMID(UNO_NAME_PARA_HYPHENATION_MAX_HYPHENS), RES_PARATR_HYPHENZONE,       CPPU_E2T(CPPUTYPE_INT16),   PropertyAttribute::MAYBEVOID, MID_HYPHEN_MAX_HYPHENS},                              \
        { SW_PROP_NMID(UNO_NAME_CHAR_AUTO_KERNING), RES_CHRATR_AUTOKERN  ,  CPPU_E2T(CPPUTYPE_BOOLEAN)  ,       PropertyAttribute::MAYBEVOID,     0},                                                                 \
        { SW_PROP_NMID(UNO_NAME_CHAR_BACK_COLOR), RES_CHRATR_BACKGROUND,    CPPU_E2T(CPPUTYPE_INT32),           PropertyAttribute::MAYBEVOID ,MID_BACK_COLOR         },                                           \
        { SW_PROP_NMID(UNO_NAME_PARA_BACK_COLOR), RES_BACKGROUND,       CPPU_E2T(CPPUTYPE_INT32),           PropertyAttribute::MAYBEVOID ,MID_BACK_COLOR         },                                           \
        { SW_PROP_NMID(UNO_NAME_CHAR_CASE_MAP), RES_CHRATR_CASEMAP,     CPPU_E2T(CPPUTYPE_INT16),           PropertyAttribute::MAYBEVOID, 0},                                                            \
        { SW_PROP_NMID(UNO_NAME_CHAR_COLOR), RES_CHRATR_COLOR,      CPPU_E2T(CPPUTYPE_INT32),           PropertyAttribute::MAYBEVOID, 0},                                                           \
        { SW_PROP_NMID(UNO_NAME_CHAR_STRIKEOUT), RES_CHRATR_CROSSEDOUT,  CPPU_E2T(CPPUTYPE_INT16),                  PropertyAttribute::MAYBEVOID, MID_CROSS_OUT},                                              \
        { SW_PROP_NMID(UNO_NAME_CHAR_CROSSED_OUT), RES_CHRATR_CROSSEDOUT,  CPPU_E2T(CPPUTYPE_BOOLEAN)  ,        PropertyAttribute::MAYBEVOID, MID_CROSSED_OUT},                                                       \
        { SW_PROP_NMID(UNO_NAME_CHAR_ESCAPEMENT), RES_CHRATR_ESCAPEMENT,  CPPU_E2T(CPPUTYPE_INT16),             PropertyAttribute::MAYBEVOID, MID_ESC           },                                                   \
        { SW_PROP_NMID(UNO_NAME_CHAR_ESCAPEMENT_HEIGHT), RES_CHRATR_ESCAPEMENT,     CPPU_E2T(CPPUTYPE_INT8)  ,          PropertyAttribute::MAYBEVOID, MID_ESC_HEIGHT},                                               \
        { SW_PROP_NMID(UNO_NAME_CHAR_AUTO_ESCAPEMENT), RES_CHRATR_ESCAPEMENT,  CPPU_E2T(CPPUTYPE_BOOLEAN)  ,            PropertyAttribute::MAYBEVOID, MID_AUTO_ESC  },                                                \
        { SW_PROP_NMID(UNO_NAME_CHAR_FLASH), RES_CHRATR_BLINK   ,   CPPU_E2T(CPPUTYPE_BOOLEAN)  ,       PropertyAttribute::MAYBEVOID,     0},                                                                 \
        { SW_PROP_NMID(UNO_NAME_CHAR_HIDDEN), RES_CHRATR_HIDDEN, CPPU_E2T(CPPUTYPE_BOOLEAN)  ,       PropertyAttribute::MAYBEVOID,     0},\
        { SW_PROP_NMID(UNO_NAME_CHAR_UNDERLINE), RES_CHRATR_UNDERLINE ,  CPPU_E2T(CPPUTYPE_INT16),              PropertyAttribute::MAYBEVOID, MID_TL_STYLE},                                                \
        { SW_PROP_NMID(UNO_NAME_CHAR_UNDERLINE_COLOR), RES_CHRATR_UNDERLINE ,  CPPU_E2T(CPPUTYPE_INT32),            PropertyAttribute::MAYBEVOID, MID_TL_COLOR},                                                \
        { SW_PROP_NMID(UNO_NAME_CHAR_UNDERLINE_HAS_COLOR), RES_CHRATR_UNDERLINE ,  CPPU_E2T(CPPUTYPE_BOOLEAN),              PropertyAttribute::MAYBEVOID, MID_TL_HASCOLOR},                                                \
        { SW_PROP_NMID(UNO_NAME_CHAR_OVERLINE), RES_CHRATR_OVERLINE ,  CPPU_E2T(CPPUTYPE_INT16),              PropertyAttribute::MAYBEVOID, MID_TL_STYLE},                                                \
        { SW_PROP_NMID(UNO_NAME_CHAR_OVERLINE_COLOR), RES_CHRATR_OVERLINE ,  CPPU_E2T(CPPUTYPE_INT32),            PropertyAttribute::MAYBEVOID, MID_TL_COLOR},                                                \
        { SW_PROP_NMID(UNO_NAME_CHAR_OVERLINE_HAS_COLOR), RES_CHRATR_OVERLINE ,  CPPU_E2T(CPPUTYPE_BOOLEAN),              PropertyAttribute::MAYBEVOID, MID_TL_HASCOLOR},                                                \
        { SW_PROP_NMID(UNO_NAME_PARA_GRAPHIC_URL), RES_BACKGROUND,      CPPU_E2T(CPPUTYPE_OUSTRING),        PropertyAttribute::MAYBEVOID ,MID_GRAPHIC_URL    },                                          \
        { SW_PROP_NMID(UNO_NAME_PARA_GRAPHIC_FILTER), RES_BACKGROUND,       CPPU_E2T(CPPUTYPE_OUSTRING),        PropertyAttribute::MAYBEVOID ,MID_GRAPHIC_FILTER    },                                       \
        { SW_PROP_NMID(UNO_NAME_PARA_GRAPHIC_LOCATION), RES_BACKGROUND,         CPPU_E2T(CPPUTYPE_GRAPHICLOC), PropertyAttribute::MAYBEVOID ,MID_GRAPHIC_POSITION},                                \
        { SW_PROP_NMID(UNO_NAME_PARA_LEFT_MARGIN), RES_LR_SPACE,            CPPU_E2T(CPPUTYPE_INT32),           PropertyAttribute::MAYBEVOID, MID_TXT_LMARGIN|CONVERT_TWIPS},                                   \
        { SW_PROP_NMID(UNO_NAME_PARA_RIGHT_MARGIN), RES_LR_SPACE,           CPPU_E2T(CPPUTYPE_INT32),           PropertyAttribute::MAYBEVOID, MID_R_MARGIN|CONVERT_TWIPS},                                  \
        { SW_PROP_NMID(UNO_NAME_PARA_IS_AUTO_FIRST_LINE_INDENT), RES_LR_SPACE,           CPPU_E2T(CPPUTYPE_BOOLEAN),      PropertyAttribute::MAYBEVOID, MID_FIRST_AUTO},                                      \
        { SW_PROP_NMID(UNO_NAME_PARA_FIRST_LINE_INDENT), RES_LR_SPACE,           CPPU_E2T(CPPUTYPE_INT32),      PropertyAttribute::MAYBEVOID, MID_FIRST_LINE_INDENT|CONVERT_TWIPS},                         \
        _STANDARD_FONT_PROPERTIES \
        _CJK_FONT_PROPERTIES \
        _CTL_FONT_PROPERTIES \
        { SW_PROP_NMID(UNO_NAME_CHAR_KERNING), RES_CHRATR_KERNING    ,  CPPU_E2T(CPPUTYPE_INT16)  ,         PropertyAttribute::MAYBEVOID,   CONVERT_TWIPS},                                                           \
        { SW_PROP_NMID(UNO_NAME_CHAR_NO_HYPHENATION), RES_CHRATR_NOHYPHEN   ,   CPPU_E2T(CPPUTYPE_BOOLEAN)  ,       PropertyAttribute::MAYBEVOID,     0},                                                                 \
        { SW_PROP_NMID(UNO_NAME_CHAR_SHADOWED), RES_CHRATR_SHADOWED  ,  CPPU_E2T(CPPUTYPE_BOOLEAN)  ,       PropertyAttribute::MAYBEVOID, 0},                                                                     \
        { SW_PROP_NMID(UNO_NAME_CHAR_CONTOURED), RES_CHRATR_CONTOUR,    CPPU_E2T(CPPUTYPE_BOOLEAN)  ,       PropertyAttribute::MAYBEVOID, 0},                                                                 \
        { SW_PROP_NMID(UNO_NAME_DROP_CAP_FORMAT), RES_PARATR_DROP,        CPPU_E2T(CPPUTYPE_DROPCAPFMT)  , PropertyAttribute::MAYBEVOID, MID_DROPCAP_FORMAT|CONVERT_TWIPS},                        \
        { SW_PROP_NMID(UNO_NAME_DROP_CAP_WHOLE_WORD), RES_PARATR_DROP,        CPPU_E2T(CPPUTYPE_BOOLEAN)  ,         PropertyAttribute::MAYBEVOID, MID_DROPCAP_WHOLE_WORD },                                               \
        { SW_PROP_NMID(UNO_NAME_DROP_CAP_CHAR_STYLE_NAME), RES_PARATR_DROP,        CPPU_E2T(CPPUTYPE_OUSTRING)  ,       PropertyAttribute::MAYBEVOID, MID_DROPCAP_CHAR_STYLE_NAME },                                  \
        { SW_PROP_NMID(UNO_NAME_PARA_KEEP_TOGETHER), RES_KEEP,              CPPU_E2T(CPPUTYPE_BOOLEAN)  ,       PropertyAttribute::MAYBEVOID,     0},                                                                 \
        { SW_PROP_NMID(UNO_NAME_PARA_SPLIT), RES_PARATR_SPLIT,      CPPU_E2T(CPPUTYPE_BOOLEAN)  ,       PropertyAttribute::MAYBEVOID,     0},                                                                 \
        { SW_PROP_NMID(UNO_NAME_PARA_WIDOWS), RES_PARATR_WIDOWS,        CPPU_E2T(CPPUTYPE_INT8),PropertyAttribute::MAYBEVOID,     0},                                                                  \
        { SW_PROP_NMID(UNO_NAME_PARA_ORPHANS), RES_PARATR_ORPHANS,      CPPU_E2T(CPPUTYPE_INT8),PropertyAttribute::MAYBEVOID,     0},                                                                 \
        { SW_PROP_NMID(UNO_NAME_PAGE_NUMBER_OFFSET), RES_PAGEDESC,              CPPU_E2T(CPPUTYPE_INT16),       PropertyAttribute::MAYBEVOID, MID_PAGEDESC_PAGENUMOFFSET},                                   \
        { SW_PROP_NMID(UNO_NAME_PARA_ADJUST), RES_PARATR_ADJUST,      CPPU_E2T(CPPUTYPE_INT16),         PropertyAttribute::MAYBEVOID, MID_PARA_ADJUST},                                              \
        { SW_PROP_NMID(UNO_NAME_PARA_EXPAND_SINGLE_WORD), RES_PARATR_ADJUST,        CPPU_E2T(CPPUTYPE_BOOLEAN)  ,       PropertyAttribute::MAYBEVOID, MID_EXPAND_SINGLE   },                                                  \
        { SW_PROP_NMID(UNO_NAME_PARA_LAST_LINE_ADJUST), RES_PARATR_ADJUST, CPPU_E2T(CPPUTYPE_INT16),                PropertyAttribute::MAYBEVOID, MID_LAST_LINE_ADJUST},                                         \
        { SW_PROP_NMID(UNO_NAME_PARA_LINE_NUMBER_COUNT), RES_LINENUMBER,    CPPU_E2T(CPPUTYPE_BOOLEAN),             PropertyAttribute::MAYBEVOID ,MID_LINENUMBER_COUNT      },                                                \
        { SW_PROP_NMID(UNO_NAME_PARA_LINE_NUMBER_START_VALUE), RES_LINENUMBER, CPPU_E2T(CPPUTYPE_INT32),                    PropertyAttribute::MAYBEVOID ,MID_LINENUMBER_STARTVALUE},                                    \
        { SW_PROP_NMID(UNO_NAME_PARA_LINE_SPACING), RES_PARATR_LINESPACING, CPPU_E2T(CPPUTYPE_LINESPACE),       PropertyAttribute::MAYBEVOID,     CONVERT_TWIPS},                                      \
        { SW_PROP_NMID(UNO_NAME_PARA_REGISTER_MODE_ACTIVE), RES_PARATR_REGISTER,CPPU_E2T(CPPUTYPE_BOOLEAN)  ,           PropertyAttribute::MAYBEVOID, 0},                                                                 \
        { SW_PROP_NMID(UNO_NAME_PARA_TOP_MARGIN), RES_UL_SPACE,             CPPU_E2T(CPPUTYPE_INT32),           PropertyAttribute::MAYBEVOID, MID_UP_MARGIN|CONVERT_TWIPS},                                  \
        { SW_PROP_NMID(UNO_NAME_PARA_BOTTOM_MARGIN), RES_UL_SPACE,          CPPU_E2T(CPPUTYPE_INT32),           PropertyAttribute::MAYBEVOID, MID_LO_MARGIN|CONVERT_TWIPS},                                 \
        { SW_PROP_NMID(UNO_NAME_PARA_CONTEXT_MARGIN), RES_UL_SPACE,          CPPU_E2T(CPPUTYPE_BOOLEAN),           PropertyAttribute::MAYBEVOID, MID_CTX_MARGIN},                                 \
        { SW_PROP_NMID(UNO_NAME_CHAR_BACK_TRANSPARENT), RES_CHRATR_BACKGROUND, CPPU_E2T(CPPUTYPE_BOOLEAN),              PropertyAttribute::MAYBEVOID ,MID_GRAPHIC_TRANSPARENT        },                                                \
        { SW_PROP_NMID(UNO_NAME_PARA_BACK_TRANSPARENT), RES_BACKGROUND, CPPU_E2T(CPPUTYPE_BOOLEAN),                 PropertyAttribute::MAYBEVOID ,MID_GRAPHIC_TRANSPARENT        },                                                \
        { SW_PROP_NMID(UNO_NAME_NUMBERING_STYLE_NAME), RES_PARATR_NUMRULE,  CPPU_E2T(CPPUTYPE_OUSTRING),         PropertyAttribute::MAYBEVOID,   0},                                                           \
        { SW_PROP_NMID(UNO_NAME_CHAR_WORD_MODE), RES_CHRATR_WORDLINEMODE,CPPU_E2T(CPPUTYPE_BOOLEAN)  ,          PropertyAttribute::MAYBEVOID,     0},                                                             \
        { SW_PROP_NMID(UNO_NAME_CHAR_LEFT_BORDER), RES_CHRATR_BOX, CPPU_E2T(CPPUTYPE_BORDERLINE), PropertyAttribute::MAYBEVOID, LEFT_BORDER  |CONVERT_TWIPS },\
        { SW_PROP_NMID(UNO_NAME_CHAR_RIGHT_BORDER), RES_CHRATR_BOX, CPPU_E2T(CPPUTYPE_BORDERLINE), PropertyAttribute::MAYBEVOID, RIGHT_BORDER |CONVERT_TWIPS },\
        { SW_PROP_NMID(UNO_NAME_CHAR_TOP_BORDER), RES_CHRATR_BOX, CPPU_E2T(CPPUTYPE_BORDERLINE), PropertyAttribute::MAYBEVOID, TOP_BORDER   |CONVERT_TWIPS },\
        { SW_PROP_NMID(UNO_NAME_CHAR_BOTTOM_BORDER), RES_CHRATR_BOX, CPPU_E2T(CPPUTYPE_BORDERLINE), PropertyAttribute::MAYBEVOID, BOTTOM_BORDER|CONVERT_TWIPS },\
        { SW_PROP_NMID(UNO_NAME_CHAR_BORDER_DISTANCE), RES_CHRATR_BOX, CPPU_E2T(CPPUTYPE_INT32), PropertyAttribute::MAYBEVOID, BORDER_DISTANCE|CONVERT_TWIPS },\
        { SW_PROP_NMID(UNO_NAME_CHAR_LEFT_BORDER_DISTANCE), RES_CHRATR_BOX, CPPU_E2T(CPPUTYPE_INT32), PropertyAttribute::MAYBEVOID, LEFT_BORDER_DISTANCE  |CONVERT_TWIPS },\
        { SW_PROP_NMID(UNO_NAME_CHAR_RIGHT_BORDER_DISTANCE), RES_CHRATR_BOX, CPPU_E2T(CPPUTYPE_INT32), PropertyAttribute::MAYBEVOID, RIGHT_BORDER_DISTANCE |CONVERT_TWIPS },\
        { SW_PROP_NMID(UNO_NAME_CHAR_TOP_BORDER_DISTANCE), RES_CHRATR_BOX, CPPU_E2T(CPPUTYPE_INT32), PropertyAttribute::MAYBEVOID, TOP_BORDER_DISTANCE   |CONVERT_TWIPS },\
        { SW_PROP_NMID(UNO_NAME_CHAR_BOTTOM_BORDER_DISTANCE), RES_CHRATR_BOX, CPPU_E2T(CPPUTYPE_INT32), PropertyAttribute::MAYBEVOID, BOTTOM_BORDER_DISTANCE|CONVERT_TWIPS },\
        { SW_PROP_NMID(UNO_NAME_LEFT_BORDER), RES_BOX,              CPPU_E2T(CPPUTYPE_BORDERLINE),      PropertyAttribute::MAYBEVOID, LEFT_BORDER  |CONVERT_TWIPS },                            \
        { SW_PROP_NMID(UNO_NAME_RIGHT_BORDER), RES_BOX,             CPPU_E2T(CPPUTYPE_BORDERLINE),      PropertyAttribute::MAYBEVOID, RIGHT_BORDER |CONVERT_TWIPS },                                \
        { SW_PROP_NMID(UNO_NAME_TOP_BORDER), RES_BOX,               CPPU_E2T(CPPUTYPE_BORDERLINE),      PropertyAttribute::MAYBEVOID, TOP_BORDER   |CONVERT_TWIPS },                            \
        { SW_PROP_NMID(UNO_NAME_BOTTOM_BORDER), RES_BOX,                CPPU_E2T(CPPUTYPE_BORDERLINE),      PropertyAttribute::MAYBEVOID, BOTTOM_BORDER|CONVERT_TWIPS },                                \
        { SW_PROP_NMID(UNO_NAME_BORDER_DISTANCE), RES_BOX,              CPPU_E2T(CPPUTYPE_INT32),           PropertyAttribute::MAYBEVOID, BORDER_DISTANCE|CONVERT_TWIPS },                               \
        { SW_PROP_NMID(UNO_NAME_LEFT_BORDER_DISTANCE), RES_BOX,                CPPU_E2T(CPPUTYPE_INT32),        PropertyAttribute::MAYBEVOID, LEFT_BORDER_DISTANCE  |CONVERT_TWIPS },                                                            \
        { SW_PROP_NMID(UNO_NAME_RIGHT_BORDER_DISTANCE), RES_BOX,                CPPU_E2T(CPPUTYPE_INT32),        PropertyAttribute::MAYBEVOID, RIGHT_BORDER_DISTANCE |CONVERT_TWIPS },                                                            \
        { SW_PROP_NMID(UNO_NAME_TOP_BORDER_DISTANCE), RES_BOX,                CPPU_E2T(CPPUTYPE_INT32),    PropertyAttribute::MAYBEVOID, TOP_BORDER_DISTANCE   |CONVERT_TWIPS },                                                        \
        { SW_PROP_NMID(UNO_NAME_BOTTOM_BORDER_DISTANCE), RES_BOX,                CPPU_E2T(CPPUTYPE_INT32),    PropertyAttribute::MAYBEVOID, BOTTOM_BORDER_DISTANCE|CONVERT_TWIPS },                                                        \
        { SW_PROP_NMID(UNO_NAME_PARA_USER_DEFINED_ATTRIBUTES), RES_UNKNOWNATR_CONTAINER, CPPU_E2T(CPPUTYPE_REFNAMECNT), PropertyAttribute::MAYBEVOID, 0 },                                   \
        { SW_PROP_NMID(UNO_NAME_TEXT_USER_DEFINED_ATTRIBUTES), RES_TXTATR_UNKNOWN_CONTAINER, CPPU_E2T(CPPUTYPE_REFNAMECNT), PropertyAttribute::MAYBEVOID, 0 },                               \
        { SW_PROP_NMID(UNO_NAME_PARA_SHADOW_FORMAT), RES_SHADOW,            CPPU_E2T(CPPUTYPE_SHADOWFMT),   PROPERTY_NONE, CONVERT_TWIPS}, \
        { SW_PROP_NMID(UNO_NAME_CHAR_COMBINE_IS_ON), RES_CHRATR_TWO_LINES,          CPPU_E2T(CPPUTYPE_BOOLEAN),     PropertyAttribute::MAYBEVOID, MID_TWOLINES}, \
        { SW_PROP_NMID(UNO_NAME_CHAR_COMBINE_PREFIX), RES_CHRATR_TWO_LINES,             CPPU_E2T(CPPUTYPE_OUSTRING),    PropertyAttribute::MAYBEVOID, MID_START_BRACKET}, \
        { SW_PROP_NMID(UNO_NAME_CHAR_COMBINE_SUFFIX), RES_CHRATR_TWO_LINES,             CPPU_E2T(CPPUTYPE_OUSTRING),    PropertyAttribute::MAYBEVOID, MID_END_BRACKET}, \
        { SW_PROP_NMID(UNO_NAME_CHAR_EMPHASIS), RES_CHRATR_EMPHASIS_MARK,           CPPU_E2T(CPPUTYPE_INT16),   PropertyAttribute::MAYBEVOID, MID_EMPHASIS},\
        { SW_PROP_NMID(UNO_NAME_PARA_IS_HANGING_PUNCTUATION), RES_PARATR_HANGINGPUNCTUATION,  CPPU_E2T(CPPUTYPE_BOOLEAN),    PropertyAttribute::MAYBEVOID ,0  },    \
        { SW_PROP_NMID(UNO_NAME_PARA_IS_CHARACTER_DISTANCE), RES_PARATR_SCRIPTSPACE,         CPPU_E2T(CPPUTYPE_BOOLEAN),    PropertyAttribute::MAYBEVOID ,0  },    \
        { SW_PROP_NMID(UNO_NAME_PARA_IS_FORBIDDEN_RULES), RES_PARATR_FORBIDDEN_RULES,     CPPU_E2T(CPPUTYPE_BOOLEAN),    PropertyAttribute::MAYBEVOID ,0  }, \
        { SW_PROP_NMID(UNO_NAME_PARA_VERT_ALIGNMENT), RES_PARATR_VERTALIGN,CPPU_E2T(CPPUTYPE_INT16),    PropertyAttribute::MAYBEVOID,     0      },  \
        { SW_PROP_NMID(UNO_NAME_CHAR_ROTATION), RES_CHRATR_ROTATE,      CPPU_E2T(CPPUTYPE_INT16),   PropertyAttribute::MAYBEVOID,     MID_ROTATE      },  \
        { SW_PROP_NMID(UNO_NAME_CHAR_ROTATION_IS_FIT_TO_LINE), RES_CHRATR_ROTATE,       CPPU_E2T(CPPUTYPE_BOOLEAN),     PropertyAttribute::MAYBEVOID,         MID_FITTOLINE  },  \
        { SW_PROP_NMID(UNO_NAME_CHAR_SCALE_WIDTH), RES_CHRATR_SCALEW,       CPPU_E2T(CPPUTYPE_INT16),   PropertyAttribute::MAYBEVOID,          0 },  \
        { SW_PROP_NMID(UNO_NAME_RUBY_TEXT), RES_TXTATR_CJK_RUBY,    CPPU_E2T(CPPUTYPE_OUSTRING),    PropertyAttribute::MAYBEVOID,          MID_RUBY_TEXT },  \
        { SW_PROP_NMID(UNO_NAME_RUBY_ADJUST), RES_TXTATR_CJK_RUBY,  CPPU_E2T(CPPUTYPE_INT16),   PropertyAttribute::MAYBEVOID,          MID_RUBY_ADJUST },  \
        { SW_PROP_NMID(UNO_NAME_RUBY_CHAR_STYLE_NAME), RES_TXTATR_CJK_RUBY, CPPU_E2T(CPPUTYPE_OUSTRING),    PropertyAttribute::MAYBEVOID,          MID_RUBY_CHARSTYLE },  \
        { SW_PROP_NMID(UNO_NAME_RUBY_IS_ABOVE), RES_TXTATR_CJK_RUBY,    CPPU_E2T(CPPUTYPE_BOOLEAN),  PropertyAttribute::MAYBEVOID,         MID_RUBY_ABOVE },  \
        { SW_PROP_NMID(UNO_NAME_CHAR_RELIEF), RES_CHRATR_RELIEF,      CPPU_E2T(CPPUTYPE_INT16),    PropertyAttribute::MAYBEVOID,      MID_RELIEF }, \
        { SW_PROP_NMID(UNO_NAME_SNAP_TO_GRID), RES_PARATR_SNAPTOGRID, CPPU_E2T(CPPUTYPE_BOOLEAN), PropertyAttribute::MAYBEVOID, 0 }, \
        { SW_PROP_NMID(UNO_NAME_PARA_IS_CONNECT_BORDER), RES_PARATR_CONNECT_BORDER, CPPU_E2T(CPPUTYPE_BOOLEAN), PropertyAttribute::MAYBEVOID, 0}, \
        { SW_PROP_NMID(UNO_NAME_WRITING_MODE), RES_FRAMEDIR, CPPU_E2T(CPPUTYPE_INT16), PROPERTY_NONE, 0 }, \
        { SW_PROP_NMID(UNO_NAME_CHAR_SHADING_VALUE), RES_CHRATR_BACKGROUND,      CPPU_E2T(CPPUTYPE_INT32),    PROPERTY_NONE,      MID_SHADING_VALUE }, \
        { SW_PROP_NMID(UNO_NAME_PARA_INTEROP_GRAB_BAG), RES_PARATR_GRABBAG, CPPU_E2T(CPPUTYPE_PROPERTYVALUE), PROPERTY_NONE, 0 }, \

#define COMMON_CRSR_PARA_PROPERTIES_WITHOUT_FN \
        COMMON_CRSR_PARA_PROPERTIES_WITHOUT_FN_01 \
        { SW_PROP_NMID(UNO_NAME_BREAK_TYPE), RES_BREAK,                 CPPU_E2T(CPPUTYPE_BREAK),           PropertyAttribute::MAYBEVOID, 0}, \
        { SW_PROP_NMID(UNO_NAME_PAGE_DESC_NAME), RES_PAGEDESC,          CPPU_E2T(CPPUTYPE_OUSTRING),            PropertyAttribute::MAYBEVOID, MID_PAGEDESC_PAGEDESCNAME },


#define TABSTOPS_MAP_ENTRY                { SW_PROP_NMID(UNO_NAME_TABSTOPS), RES_PARATR_TABSTOP,   CPPU_E2T(CPPUTYPE_SEQTABSTOP),   PropertyAttribute::MAYBEVOID, CONVERT_TWIPS},

#define COMMON_CRSR_PARA_PROPERTIES \
        COMMON_CRSR_PARA_PROPERTIES_FN_ONLY \
        COMMON_CRSR_PARA_PROPERTIES_WITHOUT_FN \
        COMMON_HYPERLINK_PROPERTIES \
        { SW_PROP_NMID(UNO_NAME_CHAR_STYLE_NAME), RES_TXTATR_CHARFMT,     CPPU_E2T(CPPUTYPE_OUSTRING),         PropertyAttribute::MAYBEVOID,     0},\
        { SW_PROP_NMID(UNO_NAME_CHAR_STYLE_NAMES), FN_UNO_CHARFMT_SEQUENCE,  CPPU_E2T(CPPUTYPE_OUSTRINGS),     PropertyAttribute::MAYBEVOID,     0},\
        { SW_PROP_NMID(UNO_NAME_CHAR_AUTO_STYLE_NAME), RES_TXTATR_AUTOFMT,     CPPU_E2T(CPPUTYPE_OUSTRING),         PropertyAttribute::MAYBEVOID,     0},\
        { SW_PROP_NMID(UNO_NAME_PARA_AUTO_STYLE_NAME), RES_AUTO_STYLE,     CPPU_E2T(CPPUTYPE_OUSTRING),         PropertyAttribute::MAYBEVOID,     0},

#define COMMON_CRSR_PARA_PROPERTIES_2 \
        COMMON_CRSR_PARA_PROPERTIES_FN_ONLY \
        COMMON_CRSR_PARA_PROPERTIES_WITHOUT_FN

#define  COMPLETE_TEXT_CURSOR_MAP\
        COMMON_CRSR_PARA_PROPERTIES\
        { SW_PROP_NMID(UNO_NAME_DOCUMENT_INDEX_MARK), FN_UNO_DOCUMENT_INDEX_MARK, CPPU_E2T(CPPUTYPE_REFDOCIDXMRK), PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY ,0 },\
        { SW_PROP_NMID(UNO_NAME_TEXT_FIELD), FN_UNO_TEXT_FIELD,      CPPU_E2T(CPPUTYPE_REFTXTFIELD),  PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY ,0 },\
        { SW_PROP_NMID(UNO_NAME_REFERENCE_MARK), FN_UNO_REFERENCE_MARK,  CPPU_E2T(CPPUTYPE_REFTEXTCNTNT), PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY, 0 },\
        { SW_PROP_NMID(UNO_NAME_FOOTNOTE), FN_UNO_FOOTNOTE,        CPPU_E2T(CPPUTYPE_REFFOOTNOTE),  PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY ,0 },\
        { SW_PROP_NMID(UNO_NAME_ENDNOTE), FN_UNO_ENDNOTE,         CPPU_E2T(CPPUTYPE_REFFOOTNOTE),  PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY ,0 },\
        { SW_PROP_NMID(UNO_NAME_HYPER_LINK_EVENTS), RES_TXTATR_INETFMT,     CPPU_E2T(CPPUTYPE_REFNMREPLACE), PropertyAttribute::MAYBEVOID, MID_URL_HYPERLINKEVENTS},\
        { SW_PROP_NMID(UNO_NAME_NESTED_TEXT_CONTENT), FN_UNO_NESTED_TEXT_CONTENT, CPPU_E2T(CPPUTYPE_REFTEXTCNTNT), PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY, 0 },\
        TABSTOPS_MAP_ENTRY



#define _BASE_INDEX_PROPERTIES_\
        { SW_PROP_NMID(UNO_NAME_TITLE), WID_IDX_TITLE,  CPPU_E2T(CPPUTYPE_OUSTRING)  , PROPERTY_NONE,     0},\
        { SW_PROP_NMID(UNO_NAME_NAME),  WID_IDX_NAME,   CPPU_E2T(CPPUTYPE_OUSTRING)  , PROPERTY_NONE,     0},\
        { SW_PROP_NMID(UNO_NAME_CONTENT_SECTION), WID_IDX_CONTENT_SECTION,  CPPU_E2T(CPPUTYPE_REFTEXTSECTION)  , PropertyAttribute::READONLY,     0},\
        { SW_PROP_NMID(UNO_NAME_HEADER_SECTION), WID_IDX_HEADER_SECTION,  CPPU_E2T(CPPUTYPE_REFTEXTSECTION)  , PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY,     0},\

#define ANCHOR_TYPES_PROPERTY    { SW_PROP_NMID(UNO_NAME_ANCHOR_TYPES), FN_UNO_ANCHOR_TYPES,    CPPU_E2T(CPPUTYPE_SEQANCHORTYPE),PropertyAttribute::READONLY, 0xff},

// #i18732# #i28701# #i73249#
#define COMMON_FRAME_PROPERTIES \
    { SW_PROP_NMID(UNO_NAME_ANCHOR_PAGE_NO), RES_ANCHOR,            CPPU_E2T(CPPUTYPE_INT16),           PROPERTY_NONE, MID_ANCHOR_PAGENUM       },              \
    { SW_PROP_NMID(UNO_NAME_ANCHOR_TYPE), RES_ANCHOR,           CPPU_E2T(CPPUTYPE_TXTCNTANCHOR),            PROPERTY_NONE, MID_ANCHOR_ANCHORTYPE},             \
    { SW_PROP_NMID(UNO_NAME_ANCHOR_FRAME), RES_ANCHOR,             CPPU_E2T(CPPUTYPE_REFTEXTFRAME),    PropertyAttribute::MAYBEVOID, MID_ANCHOR_ANCHORFRAME},             \
    ANCHOR_TYPES_PROPERTY\
    { SW_PROP_NMID(UNO_NAME_BACK_COLOR), RES_BACKGROUND,            CPPU_E2T(CPPUTYPE_INT32),           PROPERTY_NONE ,MID_BACK_COLOR        },                      \
    { SW_PROP_NMID(UNO_NAME_BACK_COLOR_R_G_B), RES_BACKGROUND,      CPPU_E2T(CPPUTYPE_INT32), PROPERTY_NONE ,MID_BACK_COLOR_R_G_B},    \
    { SW_PROP_NMID(UNO_NAME_BACK_COLOR_TRANSPARENCY), RES_BACKGROUND,      CPPU_E2T(CPPUTYPE_INT8), PROPERTY_NONE ,MID_BACK_COLOR_TRANSPARENCY},    \
    { SW_PROP_NMID(UNO_NAME_FILL_STYLE), RES_FILL_STYLE,      CPPU_E2T(CPPUTYPE_FILLSTYLE), PROPERTY_NONE ,0}, \
    { SW_PROP_NMID(UNO_NAME_FILL_GRADIENT), RES_FILL_GRADIENT,      CPPU_E2T(CPPUTYPE_GRADIENT), PROPERTY_NONE ,MID_FILLGRADIENT}, \
    { SW_PROP_NMID(UNO_NAME_FILL_GRADIENT_NAME), RES_FILL_GRADIENT, CPPU_E2T(CPPUTYPE_OUSTRING), PROPERTY_NONE ,MID_NAME}, \
    { SW_PROP_NMID(UNO_NAME_CONTENT_PROTECTED), RES_PROTECT,            CPPU_E2T(CPPUTYPE_BOOLEAN),             PROPERTY_NONE, MID_PROTECT_CONTENT  },                          \
    { SW_PROP_NMID(UNO_NAME_FRAME_STYLE_NAME), FN_UNO_FRAME_STYLE_NAME,CPPU_E2T(CPPUTYPE_OUSTRING),         PROPERTY_NONE, 0},                                   \
    { SW_PROP_NMID(UNO_NAME_BACK_GRAPHIC_URL), RES_BACKGROUND,      CPPU_E2T(CPPUTYPE_OUSTRING),        PROPERTY_NONE ,MID_GRAPHIC_URL    },                 \
    { SW_PROP_NMID(UNO_NAME_BACK_GRAPHIC_FILTER), RES_BACKGROUND,       CPPU_E2T(CPPUTYPE_OUSTRING),        PROPERTY_NONE ,MID_GRAPHIC_FILTER    },              \
    { SW_PROP_NMID(UNO_NAME_BACK_GRAPHIC_LOCATION), RES_BACKGROUND,         CPPU_E2T(CPPUTYPE_GRAPHICLOC), PROPERTY_NONE ,MID_GRAPHIC_POSITION},    \
    { SW_PROP_NMID(UNO_NAME_BACK_GRAPHIC_TRANSPARENCY), RES_BACKGROUND,      CPPU_E2T(CPPUTYPE_INT8), PROPERTY_NONE ,MID_GRAPHIC_TRANSPARENCY},    \
    { SW_PROP_NMID(UNO_NAME_LEFT_MARGIN), RES_LR_SPACE,             CPPU_E2T(CPPUTYPE_INT32),           PROPERTY_NONE, MID_L_MARGIN|CONVERT_TWIPS},            \
    { SW_PROP_NMID(UNO_NAME_RIGHT_MARGIN), RES_LR_SPACE,            CPPU_E2T(CPPUTYPE_INT32),           PROPERTY_NONE, MID_R_MARGIN|CONVERT_TWIPS},            \
    { SW_PROP_NMID(UNO_NAME_WIDTH), RES_FRM_SIZE,           CPPU_E2T(CPPUTYPE_INT32)  ,         PROPERTY_NONE, MID_FRMSIZE_WIDTH|CONVERT_TWIPS},\
    { SW_PROP_NMID(UNO_NAME_HEIGHT), RES_FRM_SIZE,          CPPU_E2T(CPPUTYPE_INT32)  ,         PROPERTY_NONE, MID_FRMSIZE_HEIGHT|CONVERT_TWIPS},\
    { SW_PROP_NMID(UNO_NAME_HORI_ORIENT), RES_HORI_ORIENT,      CPPU_E2T(CPPUTYPE_INT16),           PROPERTY_NONE ,MID_HORIORIENT_ORIENT    },             \
    { SW_PROP_NMID(UNO_NAME_HORI_ORIENT_POSITION), RES_HORI_ORIENT,     CPPU_E2T(CPPUTYPE_INT32),           PROPERTY_NONE ,MID_HORIORIENT_POSITION|CONVERT_TWIPS    }, \
    { SW_PROP_NMID(UNO_NAME_HORI_ORIENT_RELATION), RES_HORI_ORIENT,     CPPU_E2T(CPPUTYPE_INT16),           PROPERTY_NONE ,MID_HORIORIENT_RELATION  },               \
    { SW_PROP_NMID(UNO_NAME_HYPER_LINK_U_R_L), RES_URL,                 CPPU_E2T(CPPUTYPE_OUSTRING),        PROPERTY_NONE ,MID_URL_URL},                         \
    { SW_PROP_NMID(UNO_NAME_HYPER_LINK_TARGET), RES_URL,                CPPU_E2T(CPPUTYPE_OUSTRING),        PROPERTY_NONE ,MID_URL_TARGET},                      \
    { SW_PROP_NMID(UNO_NAME_HYPER_LINK_NAME), RES_URL,              CPPU_E2T(CPPUTYPE_OUSTRING),            PROPERTY_NONE ,MID_URL_HYPERLINKNAME     },                 \
    { SW_PROP_NMID(UNO_NAME_OPAQUE), RES_OPAQUE,            CPPU_E2T(CPPUTYPE_BOOLEAN),         PROPERTY_NONE, 0},                                        \
    { SW_PROP_NMID(UNO_NAME_PAGE_TOGGLE), RES_HORI_ORIENT,      CPPU_E2T(CPPUTYPE_BOOLEAN),         PROPERTY_NONE ,MID_HORIORIENT_PAGETOGGLE },               \
    { SW_PROP_NMID(UNO_NAME_POSITION_PROTECTED), RES_PROTECT,           CPPU_E2T(CPPUTYPE_BOOLEAN),             PROPERTY_NONE, MID_PROTECT_POSITION},                      \
    { SW_PROP_NMID(UNO_NAME_PRINT), RES_PRINT,              CPPU_E2T(CPPUTYPE_BOOLEAN),         PROPERTY_NONE, 0},                                        \
    { SW_PROP_NMID(UNO_NAME_RELATIVE_HEIGHT), RES_FRM_SIZE,         CPPU_E2T(CPPUTYPE_INT16)  ,         PROPERTY_NONE,   MID_FRMSIZE_REL_HEIGHT },            \
    { SW_PROP_NMID(UNO_NAME_RELATIVE_WIDTH), RES_FRM_SIZE,          CPPU_E2T(CPPUTYPE_INT16)  ,         PROPERTY_NONE,   MID_FRMSIZE_REL_WIDTH  },         \
    { SW_PROP_NMID(UNO_NAME_SHADOW_FORMAT), RES_SHADOW,             CPPU_E2T(CPPUTYPE_SHADOWFMT),   PROPERTY_NONE, CONVERT_TWIPS},             \
    { SW_PROP_NMID(UNO_NAME_IMAGE_MAP), RES_URL,                    CPPU_E2T(CPPUTYPE_REFIDXCNTNR), PROPERTY_NONE, MID_URL_CLIENTMAP}, \
    { SW_PROP_NMID(UNO_NAME_SERVER_MAP), RES_URL,                CPPU_E2T(CPPUTYPE_BOOLEAN),            PROPERTY_NONE ,MID_URL_SERVERMAP   },                      \
    { SW_PROP_NMID(UNO_NAME_SIZE), RES_FRM_SIZE,            CPPU_E2T(CPPUTYPE_AWTSIZE),             PROPERTY_NONE, MID_FRMSIZE_SIZE|CONVERT_TWIPS},        \
    { SW_PROP_NMID(UNO_NAME_SIZE_PROTECTED), RES_PROTECT,           CPPU_E2T(CPPUTYPE_BOOLEAN),             PROPERTY_NONE, MID_PROTECT_SIZE    },                      \
    { SW_PROP_NMID(UNO_NAME_IS_SYNC_WIDTH_TO_HEIGHT), RES_FRM_SIZE,         CPPU_E2T(CPPUTYPE_BOOLEAN)  ,       PROPERTY_NONE,   MID_FRMSIZE_IS_SYNC_WIDTH_TO_HEIGHT    },  \
    { SW_PROP_NMID(UNO_NAME_IS_SYNC_HEIGHT_TO_WIDTH), RES_FRM_SIZE,         CPPU_E2T(CPPUTYPE_BOOLEAN)  ,       PROPERTY_NONE,   MID_FRMSIZE_IS_SYNC_HEIGHT_TO_WIDTH },     \
    { SW_PROP_NMID(UNO_NAME_TEXT_WRAP), RES_SURROUND,           CPPU_E2T(CPPUTYPE_WRAPTXTMODE),             PROPERTY_NONE, MID_SURROUND_SURROUNDTYPE    },        \
    { SW_PROP_NMID(UNO_NAME_SURROUND), RES_SURROUND,          CPPU_E2T(CPPUTYPE_WRAPTXTMODE),            PROPERTY_NONE, MID_SURROUND_SURROUNDTYPE },        \
    { SW_PROP_NMID(UNO_NAME_SURROUND_ANCHORONLY), RES_SURROUND,             CPPU_E2T(CPPUTYPE_BOOLEAN),             PROPERTY_NONE, MID_SURROUND_ANCHORONLY      },                 \
    { SW_PROP_NMID(UNO_NAME_TOP_MARGIN), RES_UL_SPACE,          CPPU_E2T(CPPUTYPE_INT32), PROPERTY_NONE, MID_UP_MARGIN|CONVERT_TWIPS},          \
    { SW_PROP_NMID(UNO_NAME_BOTTOM_MARGIN), RES_UL_SPACE,           CPPU_E2T(CPPUTYPE_INT32), PROPERTY_NONE, MID_LO_MARGIN|CONVERT_TWIPS},          \
    { SW_PROP_NMID(UNO_NAME_BACK_TRANSPARENT), RES_BACKGROUND,      CPPU_E2T(CPPUTYPE_BOOLEAN),         PROPERTY_NONE ,MID_GRAPHIC_TRANSPARENT       },                    \
    { SW_PROP_NMID(UNO_NAME_VERT_ORIENT), RES_VERT_ORIENT,      CPPU_E2T(CPPUTYPE_INT16),           PROPERTY_NONE ,MID_VERTORIENT_ORIENT    },             \
    { SW_PROP_NMID(UNO_NAME_VERT_ORIENT_POSITION), RES_VERT_ORIENT,     CPPU_E2T(CPPUTYPE_INT32),           PROPERTY_NONE ,MID_VERTORIENT_POSITION|CONVERT_TWIPS    }, \
    { SW_PROP_NMID(UNO_NAME_VERT_ORIENT_RELATION), RES_VERT_ORIENT,     CPPU_E2T(CPPUTYPE_INT16),           PROPERTY_NONE ,MID_VERTORIENT_RELATION  },               \
    { SW_PROP_NMID(UNO_NAME_LEFT_BORDER), RES_BOX,              CPPU_E2T(CPPUTYPE_BORDERLINE),  0, LEFT_BORDER  |CONVERT_TWIPS },             \
    { SW_PROP_NMID(UNO_NAME_RIGHT_BORDER), RES_BOX,             CPPU_E2T(CPPUTYPE_BORDERLINE),  0, RIGHT_BORDER |CONVERT_TWIPS },                 \
    { SW_PROP_NMID(UNO_NAME_TOP_BORDER), RES_BOX,               CPPU_E2T(CPPUTYPE_BORDERLINE),  0, TOP_BORDER   |CONVERT_TWIPS },             \
    { SW_PROP_NMID(UNO_NAME_BOTTOM_BORDER), RES_BOX,                CPPU_E2T(CPPUTYPE_BORDERLINE),  0, BOTTOM_BORDER|CONVERT_TWIPS },                 \
    { SW_PROP_NMID(UNO_NAME_BORDER_DISTANCE), RES_BOX,              CPPU_E2T(CPPUTYPE_INT32),   0, BORDER_DISTANCE|CONVERT_TWIPS },                       \
    { SW_PROP_NMID(UNO_NAME_LEFT_BORDER_DISTANCE), RES_BOX,             CPPU_E2T(CPPUTYPE_INT32),   0, LEFT_BORDER_DISTANCE  |CONVERT_TWIPS },                \
    { SW_PROP_NMID(UNO_NAME_RIGHT_BORDER_DISTANCE), RES_BOX,                CPPU_E2T(CPPUTYPE_INT32),   0, RIGHT_BORDER_DISTANCE |CONVERT_TWIPS },                \
    { SW_PROP_NMID(UNO_NAME_TOP_BORDER_DISTANCE), RES_BOX,              CPPU_E2T(CPPUTYPE_INT32),   0, TOP_BORDER_DISTANCE   |CONVERT_TWIPS },            \
    { SW_PROP_NMID(UNO_NAME_BOTTOM_BORDER_DISTANCE), RES_BOX,               CPPU_E2T(CPPUTYPE_INT32),   0, BOTTOM_BORDER_DISTANCE|CONVERT_TWIPS },            \
    { SW_PROP_NMID(UNO_LINK_DISPLAY_NAME), FN_PARAM_LINK_DISPLAY_NAME,  CPPU_E2T(CPPUTYPE_OUSTRING), PropertyAttribute::READONLY, 0xff},                \
    { SW_PROP_NMID(UNO_NAME_USER_DEFINED_ATTRIBUTES), RES_UNKNOWNATR_CONTAINER, CPPU_E2T(CPPUTYPE_REFNAMECNT), PropertyAttribute::MAYBEVOID, 0 },\
    { SW_PROP_NMID(UNO_NAME_Z_ORDER), FN_UNO_Z_ORDER,           CPPU_E2T(CPPUTYPE_INT32),       PROPERTY_NONE, 0}, \
    { SW_PROP_NMID(UNO_NAME_IS_FOLLOWING_TEXT_FLOW), RES_FOLLOW_TEXT_FLOW,     CPPU_E2T(CPPUTYPE_BOOLEAN), PROPERTY_NONE, 0}, \
    { SW_PROP_NMID(UNO_NAME_WRAP_INFLUENCE_ON_POSITION), RES_WRAP_INFLUENCE_ON_OBJPOS, CPPU_E2T(CPPUTYPE_INT8), PROPERTY_NONE, MID_WRAP_INFLUENCE}, \
    { SW_PROP_NMID(UNO_NAME_TITLE), FN_UNO_TITLE, CPPU_E2T(CPPUTYPE_OUSTRING), PROPERTY_NONE, 0}, \
    { SW_PROP_NMID(UNO_NAME_DESCRIPTION), FN_UNO_DESCRIPTION, CPPU_E2T(CPPUTYPE_OUSTRING), PROPERTY_NONE, 0}, \
    { SW_PROP_NMID(UNO_NAME_LAYOUT_SIZE), WID_LAYOUT_SIZE, CPPU_E2T(CPPUTYPE_AWTSIZE), PropertyAttribute::MAYBEVOID | PropertyAttribute::READONLY, 0 }, \
    { SW_PROP_NMID(UNO_NAME_LINE_STYLE), RES_BOX, CPPU_E2T(CPPUTYPE_LINESTYLE),  0, LINE_STYLE }, \
    { SW_PROP_NMID(UNO_NAME_LINE_WIDTH), RES_BOX, CPPU_E2T(CPPUTYPE_INT32),  0, LINE_WIDTH |CONVERT_TWIPS },



#define COMMON_TEXT_CONTENT_PROPERTIES \
        { SW_PROP_NMID(UNO_NAME_ANCHOR_TYPE), FN_UNO_ANCHOR_TYPE, CPPU_E2T(CPPUTYPE_TXTCNTANCHOR),              PropertyAttribute::READONLY, MID_ANCHOR_ANCHORTYPE},\
        ANCHOR_TYPES_PROPERTY\
        { SW_PROP_NMID(UNO_NAME_TEXT_WRAP), FN_UNO_TEXT_WRAP,   CPPU_E2T(CPPUTYPE_WRAPTXTMODE),                 PropertyAttribute::READONLY, MID_SURROUND_SURROUNDTYPE  },


#define     _PROP_DIFF_FONTHEIGHT \
                    { SW_PROP_NMID(UNO_NAME_CHAR_PROP_HEIGHT), RES_CHRATR_FONTSIZE ,            CPPU_E2T(CPPUTYPE_FLOAT),                                           PROPERTY_NONE , MID_FONTHEIGHT_PROP},\
                    { SW_PROP_NMID(UNO_NAME_CHAR_DIFF_HEIGHT), RES_CHRATR_FONTSIZE ,            CPPU_E2T(CPPUTYPE_INT16),    PROPERTY_NONE , MID_FONTHEIGHT_DIFF|CONVERT_TWIPS},\
                    { SW_PROP_NMID(UNO_NAME_CHAR_PROP_HEIGHT_ASIAN), RES_CHRATR_CJK_FONTSIZE ,          CPPU_E2T(CPPUTYPE_FLOAT),                                           PROPERTY_NONE , MID_FONTHEIGHT_PROP},\
                    { SW_PROP_NMID(UNO_NAME_CHAR_DIFF_HEIGHT_ASIAN), RES_CHRATR_CJK_FONTSIZE ,          CPPU_E2T(CPPUTYPE_INT16),    PROPERTY_NONE , MID_FONTHEIGHT_DIFF|CONVERT_TWIPS},\
                    { SW_PROP_NMID(UNO_NAME_CHAR_PROP_HEIGHT_COMPLEX), RES_CHRATR_CTL_FONTSIZE ,            CPPU_E2T(CPPUTYPE_FLOAT),                                           PROPERTY_NONE , MID_FONTHEIGHT_PROP},\
                    { SW_PROP_NMID(UNO_NAME_CHAR_DIFF_HEIGHT_COMPLEX), RES_CHRATR_CTL_FONTSIZE ,            CPPU_E2T(CPPUTYPE_INT16),    PROPERTY_NONE , MID_FONTHEIGHT_DIFF|CONVERT_TWIPS},


#define COMMON_PARA_STYLE_PROPERTIES \
                    { SW_PROP_NMID(UNO_NAME_BREAK_TYPE), RES_BREAK,                 CPPU_E2T(CPPUTYPE_BREAK),       PROPERTY_NONE, 0},\
                    { SW_PROP_NMID(UNO_NAME_PAGE_DESC_NAME), RES_PAGEDESC,          CPPU_E2T(CPPUTYPE_OUSTRING),            PropertyAttribute::MAYBEVOID, MID_PAGEDESC_PAGEDESCNAME },\
                    { SW_PROP_NMID(UNO_NAME_PAGE_NUMBER_OFFSET), RES_PAGEDESC,              CPPU_E2T(CPPUTYPE_INT16),       PropertyAttribute::MAYBEVOID, MID_PAGEDESC_PAGENUMOFFSET},\
                    { SW_PROP_NMID(UNO_NAME_CHAR_AUTO_KERNING), RES_CHRATR_AUTOKERN  ,  CPPU_E2T(CPPUTYPE_BOOLEAN)  ,       PROPERTY_NONE,     0},\
                    { SW_PROP_NMID(UNO_NAME_CHAR_BACK_TRANSPARENT), RES_CHRATR_BACKGROUND, CPPU_E2T(CPPUTYPE_BOOLEAN),          PROPERTY_NONE ,MID_GRAPHIC_TRANSPARENT       },\
                    { SW_PROP_NMID(UNO_NAME_CHAR_BACK_COLOR), RES_CHRATR_BACKGROUND,    CPPU_E2T(CPPUTYPE_INT32),           PROPERTY_NONE ,MID_BACK_COLOR        },\
                    { SW_PROP_NMID(UNO_NAME_PARA_BACK_COLOR), RES_BACKGROUND,       CPPU_E2T(CPPUTYPE_INT32),           PROPERTY_NONE ,MID_BACK_COLOR        },\
                    { SW_PROP_NMID(UNO_NAME_PARA_BACK_TRANSPARENT), RES_BACKGROUND,         CPPU_E2T(CPPUTYPE_BOOLEAN),         PROPERTY_NONE ,MID_GRAPHIC_TRANSPARENT       },\
                    { SW_PROP_NMID(UNO_NAME_PARA_GRAPHIC_URL), RES_BACKGROUND,      CPPU_E2T(CPPUTYPE_OUSTRING), PROPERTY_NONE ,MID_GRAPHIC_URL    },\
                    { SW_PROP_NMID(UNO_NAME_PARA_GRAPHIC_FILTER), RES_BACKGROUND,       CPPU_E2T(CPPUTYPE_OUSTRING), PROPERTY_NONE ,MID_GRAPHIC_FILTER    },\
                    { SW_PROP_NMID(UNO_NAME_PARA_GRAPHIC_LOCATION), RES_BACKGROUND,         CPPU_E2T(CPPUTYPE_GRAPHICLOC), PROPERTY_NONE ,MID_GRAPHIC_POSITION},\
                    { SW_PROP_NMID(UNO_NAME_CHAR_CASE_MAP), RES_CHRATR_CASEMAP,     CPPU_E2T(CPPUTYPE_INT16),           PROPERTY_NONE, 0},\
                    { SW_PROP_NMID(UNO_NAME_CHAR_COLOR), RES_CHRATR_COLOR,      CPPU_E2T(CPPUTYPE_INT32),           PROPERTY_NONE, 0},\
                    { SW_PROP_NMID(UNO_NAME_CHAR_STRIKEOUT), RES_CHRATR_CROSSEDOUT,  CPPU_E2T(CPPUTYPE_INT16),                  PropertyAttribute::MAYBEVOID, MID_CROSS_OUT},\
                    { SW_PROP_NMID(UNO_NAME_CHAR_CROSSED_OUT), RES_CHRATR_CROSSEDOUT,  CPPU_E2T(CPPUTYPE_BOOLEAN)  ,        PROPERTY_NONE, 0},\
                    { SW_PROP_NMID(UNO_NAME_CHAR_ESCAPEMENT), RES_CHRATR_ESCAPEMENT,  CPPU_E2T(CPPUTYPE_INT16),             PROPERTY_NONE, MID_ESC          },\
                    { SW_PROP_NMID(UNO_NAME_CHAR_ESCAPEMENT_HEIGHT), RES_CHRATR_ESCAPEMENT,     CPPU_E2T(CPPUTYPE_INT8)  ,          PROPERTY_NONE, MID_ESC_HEIGHT},\
                    { SW_PROP_NMID(UNO_NAME_CHAR_FLASH), RES_CHRATR_BLINK   ,   CPPU_E2T(CPPUTYPE_BOOLEAN)  ,       PROPERTY_NONE,     0},\
                    { SW_PROP_NMID(UNO_NAME_CHAR_HIDDEN), RES_CHRATR_HIDDEN, CPPU_E2T(CPPUTYPE_BOOLEAN)  ,       PROPERTY_NONE,     0},\
                    _STANDARD_FONT_PROPERTIES\
                    _CJK_FONT_PROPERTIES\
                    _CTL_FONT_PROPERTIES\
                    { SW_PROP_NMID(UNO_NAME_CHAR_UNDERLINE), RES_CHRATR_UNDERLINE ,  CPPU_E2T(CPPUTYPE_INT16),      PROPERTY_NONE, MID_TL_STYLE},\
                    { SW_PROP_NMID(UNO_NAME_CHAR_UNDERLINE_COLOR), RES_CHRATR_UNDERLINE ,  CPPU_E2T(CPPUTYPE_INT32),            PROPERTY_NONE, MID_TL_COLOR},\
                    { SW_PROP_NMID(UNO_NAME_CHAR_UNDERLINE_HAS_COLOR), RES_CHRATR_UNDERLINE ,  CPPU_E2T(CPPUTYPE_BOOLEAN),              PROPERTY_NONE, MID_TL_HASCOLOR},\
                    { SW_PROP_NMID(UNO_NAME_CHAR_OVERLINE), RES_CHRATR_OVERLINE ,  CPPU_E2T(CPPUTYPE_INT16),      PROPERTY_NONE, MID_TL_STYLE},\
                    { SW_PROP_NMID(UNO_NAME_CHAR_OVERLINE_COLOR), RES_CHRATR_OVERLINE ,  CPPU_E2T(CPPUTYPE_INT32),            PROPERTY_NONE, MID_TL_COLOR},\
                    { SW_PROP_NMID(UNO_NAME_CHAR_OVERLINE_HAS_COLOR), RES_CHRATR_OVERLINE ,  CPPU_E2T(CPPUTYPE_BOOLEAN),              PROPERTY_NONE, MID_TL_HASCOLOR},\
                    { SW_PROP_NMID(UNO_NAME_PARA_LEFT_MARGIN), RES_LR_SPACE,            CPPU_E2T(CPPUTYPE_INT32), PROPERTY_NONE, MID_TXT_LMARGIN|CONVERT_TWIPS},\
                    { SW_PROP_NMID(UNO_NAME_PARA_RIGHT_MARGIN), RES_LR_SPACE,           CPPU_E2T(CPPUTYPE_INT32), PROPERTY_NONE, MID_R_MARGIN|CONVERT_TWIPS},\
                    { SW_PROP_NMID(UNO_NAME_PARA_LEFT_MARGIN_RELATIVE), RES_LR_SPACE,          CPPU_E2T(CPPUTYPE_INT16), PROPERTY_NONE,        MID_L_REL_MARGIN},\
                    { SW_PROP_NMID(UNO_NAME_PARA_RIGHT_MARGIN_RELATIVE), RES_LR_SPACE,         CPPU_E2T(CPPUTYPE_INT16), PROPERTY_NONE,        MID_R_REL_MARGIN},\
                    { SW_PROP_NMID(UNO_NAME_PARA_IS_AUTO_FIRST_LINE_INDENT), RES_LR_SPACE,          CPPU_E2T(CPPUTYPE_BOOLEAN),      PROPERTY_NONE, MID_FIRST_AUTO},\
                    { SW_PROP_NMID(UNO_NAME_PARA_FIRST_LINE_INDENT), RES_LR_SPACE,           CPPU_E2T(CPPUTYPE_INT32), PROPERTY_NONE, MID_FIRST_LINE_INDENT|CONVERT_TWIPS},\
                    { SW_PROP_NMID(UNO_NAME_PARA_FIRST_LINE_INDENT_RELATIVE), RES_LR_SPACE,         CPPU_E2T(CPPUTYPE_INT32), PROPERTY_NONE, MID_FIRST_LINE_REL_INDENT|CONVERT_TWIPS},\
                    { SW_PROP_NMID(UNO_NAME_CHAR_KERNING), RES_CHRATR_KERNING    ,  CPPU_E2T(CPPUTYPE_INT16)  ,         PROPERTY_NONE,  CONVERT_TWIPS},\
                    { SW_PROP_NMID(UNO_NAME_CHAR_NO_HYPHENATION), RES_CHRATR_NOHYPHEN   ,   CPPU_E2T(CPPUTYPE_BOOLEAN)  ,       PROPERTY_NONE,     0},\
                    { SW_PROP_NMID(UNO_NAME_CHAR_SHADOWED), RES_CHRATR_SHADOWED  ,  CPPU_E2T(CPPUTYPE_BOOLEAN)  ,       PROPERTY_NONE, 0},\
                    { SW_PROP_NMID(UNO_NAME_CHAR_CONTOURED), RES_CHRATR_CONTOUR,    CPPU_E2T(CPPUTYPE_BOOLEAN)  ,       PROPERTY_NONE, 0},\
                    { SW_PROP_NMID(UNO_NAME_DROP_CAP_FORMAT), RES_PARATR_DROP,        CPPU_E2T(CPPUTYPE_DROPCAPFMT)  , PROPERTY_NONE, MID_DROPCAP_FORMAT|CONVERT_TWIPS     },\
                    { SW_PROP_NMID(UNO_NAME_DROP_CAP_WHOLE_WORD), RES_PARATR_DROP,        CPPU_E2T(CPPUTYPE_BOOLEAN)  ,         PROPERTY_NONE, MID_DROPCAP_WHOLE_WORD },\
                    { SW_PROP_NMID(UNO_NAME_DROP_CAP_CHAR_STYLE_NAME), RES_PARATR_DROP,        CPPU_E2T(CPPUTYPE_OUSTRING)  ,       PropertyAttribute::MAYBEVOID, MID_DROPCAP_CHAR_STYLE_NAME },\
                    { SW_PROP_NMID(UNO_NAME_PARA_KEEP_TOGETHER), RES_KEEP,              CPPU_E2T(CPPUTYPE_BOOLEAN)  ,       PROPERTY_NONE,     0},\
                    { SW_PROP_NMID(UNO_NAME_PARA_SPLIT), RES_PARATR_SPLIT,      CPPU_E2T(CPPUTYPE_BOOLEAN)  ,       PROPERTY_NONE,     0},\
                    { SW_PROP_NMID(UNO_NAME_PARA_WIDOWS), RES_PARATR_WIDOWS,        CPPU_E2T(CPPUTYPE_INT8),PropertyAttribute::MAYBEVOID,     0},\
                    { SW_PROP_NMID(UNO_NAME_PARA_ORPHANS), RES_PARATR_ORPHANS,      CPPU_E2T(CPPUTYPE_INT8),PropertyAttribute::MAYBEVOID,     0},\
                    { SW_PROP_NMID(UNO_NAME_PARA_EXPAND_SINGLE_WORD), RES_PARATR_ADJUST,      CPPU_E2T(CPPUTYPE_BOOLEAN)  ,         PROPERTY_NONE, MID_EXPAND_SINGLE   },\
                    { SW_PROP_NMID(UNO_NAME_PARA_LAST_LINE_ADJUST), RES_PARATR_ADJUST,      CPPU_E2T(CPPUTYPE_INT16),       PROPERTY_NONE, MID_LAST_LINE_ADJUST},\
                    { SW_PROP_NMID(UNO_NAME_PARA_LINE_NUMBER_COUNT), RES_LINENUMBER,        CPPU_E2T(CPPUTYPE_BOOLEAN),         PROPERTY_NONE ,MID_LINENUMBER_COUNT     },\
                    { SW_PROP_NMID(UNO_NAME_PARA_LINE_NUMBER_START_VALUE), RES_LINENUMBER,      CPPU_E2T(CPPUTYPE_INT32),           PROPERTY_NONE ,MID_LINENUMBER_STARTVALUE},\
                    { SW_PROP_NMID(UNO_NAME_PARA_LINE_SPACING), RES_PARATR_LINESPACING, CPPU_E2T(CPPUTYPE_LINESPACE),PROPERTY_NONE,     CONVERT_TWIPS},\
                    { SW_PROP_NMID(UNO_NAME_PARA_ADJUST), RES_PARATR_ADJUST,      CPPU_E2T(CPPUTYPE_INT16),         PROPERTY_NONE, MID_PARA_ADJUST},\
                    { SW_PROP_NMID(UNO_NAME_PARA_REGISTER_MODE_ACTIVE), RES_PARATR_REGISTER,    CPPU_E2T(CPPUTYPE_BOOLEAN)  ,       PROPERTY_NONE, 0},\
                    { SW_PROP_NMID(UNO_NAME_PARA_TOP_MARGIN), RES_UL_SPACE,             CPPU_E2T(CPPUTYPE_INT32), PROPERTY_NONE, MID_UP_MARGIN|CONVERT_TWIPS},\
                    { SW_PROP_NMID(UNO_NAME_PARA_BOTTOM_MARGIN), RES_UL_SPACE,          CPPU_E2T(CPPUTYPE_INT32), PROPERTY_NONE, MID_LO_MARGIN|CONVERT_TWIPS},\
                    { SW_PROP_NMID(UNO_NAME_PARA_CONTEXT_MARGIN), RES_UL_SPACE,          CPPU_E2T(CPPUTYPE_BOOLEAN), PROPERTY_NONE, MID_CTX_MARGIN},\
                    { SW_PROP_NMID(UNO_NAME_PARA_TOP_MARGIN_RELATIVE), RES_UL_SPACE,        CPPU_E2T(CPPUTYPE_INT16), PROPERTY_NONE, MID_UP_REL_MARGIN},\
                    { SW_PROP_NMID(UNO_NAME_PARA_BOTTOM_MARGIN_RELATIVE), RES_UL_SPACE,         CPPU_E2T(CPPUTYPE_INT16), PROPERTY_NONE, MID_LO_REL_MARGIN},\
                    TABSTOPS_MAP_ENTRY\
                    { SW_PROP_NMID(UNO_NAME_CHAR_WORD_MODE), RES_CHRATR_WORDLINEMODE,CPPU_E2T(CPPUTYPE_BOOLEAN)  ,    PROPERTY_NONE,     0},\
                    { SW_PROP_NMID(UNO_NAME_CHAR_LEFT_BORDER), RES_CHRATR_BOX, CPPU_E2T(CPPUTYPE_BORDERLINE), PROPERTY_NONE, LEFT_BORDER |CONVERT_TWIPS },\
                    { SW_PROP_NMID(UNO_NAME_CHAR_RIGHT_BORDER), RES_CHRATR_BOX, CPPU_E2T(CPPUTYPE_BORDERLINE), PROPERTY_NONE, RIGHT_BORDER |CONVERT_TWIPS },\
                    { SW_PROP_NMID(UNO_NAME_CHAR_TOP_BORDER), RES_CHRATR_BOX, CPPU_E2T(CPPUTYPE_BORDERLINE), PROPERTY_NONE, TOP_BORDER |CONVERT_TWIPS },\
                    { SW_PROP_NMID(UNO_NAME_CHAR_BOTTOM_BORDER), RES_CHRATR_BOX, CPPU_E2T(CPPUTYPE_BORDERLINE), PROPERTY_NONE, BOTTOM_BORDER |CONVERT_TWIPS },\
                    { SW_PROP_NMID(UNO_NAME_CHAR_BORDER_DISTANCE), RES_CHRATR_BOX, CPPU_E2T(CPPUTYPE_INT32), PROPERTY_NONE, BORDER_DISTANCE |CONVERT_TWIPS },\
                    { SW_PROP_NMID(UNO_NAME_CHAR_LEFT_BORDER_DISTANCE), RES_CHRATR_BOX, CPPU_E2T(CPPUTYPE_INT32), PROPERTY_NONE, LEFT_BORDER_DISTANCE |CONVERT_TWIPS },\
                    { SW_PROP_NMID(UNO_NAME_CHAR_RIGHT_BORDER_DISTANCE), RES_CHRATR_BOX, CPPU_E2T(CPPUTYPE_INT32),PROPERTY_NONE, RIGHT_BORDER_DISTANCE |CONVERT_TWIPS },\
                    { SW_PROP_NMID(UNO_NAME_CHAR_TOP_BORDER_DISTANCE), RES_CHRATR_BOX, CPPU_E2T(CPPUTYPE_INT32), PROPERTY_NONE, TOP_BORDER_DISTANCE |CONVERT_TWIPS },\
                    { SW_PROP_NMID(UNO_NAME_CHAR_BOTTOM_BORDER_DISTANCE), RES_CHRATR_BOX, CPPU_E2T(CPPUTYPE_INT32), PROPERTY_NONE, BOTTOM_BORDER_DISTANCE|CONVERT_TWIPS },\
                    { SW_PROP_NMID(UNO_NAME_LEFT_BORDER), RES_BOX,              CPPU_E2T(CPPUTYPE_BORDERLINE),  0, LEFT_BORDER  |CONVERT_TWIPS },\
                    { SW_PROP_NMID(UNO_NAME_RIGHT_BORDER), RES_BOX,             CPPU_E2T(CPPUTYPE_BORDERLINE),  0, RIGHT_BORDER |CONVERT_TWIPS },\
                    { SW_PROP_NMID(UNO_NAME_TOP_BORDER), RES_BOX,               CPPU_E2T(CPPUTYPE_BORDERLINE),  0, TOP_BORDER   |CONVERT_TWIPS },\
                    { SW_PROP_NMID(UNO_NAME_BOTTOM_BORDER), RES_BOX,                CPPU_E2T(CPPUTYPE_BORDERLINE),  0, BOTTOM_BORDER|CONVERT_TWIPS },\
                    { SW_PROP_NMID(UNO_NAME_BORDER_DISTANCE), RES_BOX,              CPPU_E2T(CPPUTYPE_INT32),   0, BORDER_DISTANCE|CONVERT_TWIPS },\
                    { SW_PROP_NMID(UNO_NAME_LEFT_BORDER_DISTANCE), RES_BOX,             CPPU_E2T(CPPUTYPE_INT32),   0, LEFT_BORDER_DISTANCE  |CONVERT_TWIPS },\
                    { SW_PROP_NMID(UNO_NAME_RIGHT_BORDER_DISTANCE), RES_BOX,                CPPU_E2T(CPPUTYPE_INT32),   0, RIGHT_BORDER_DISTANCE |CONVERT_TWIPS },\
                    { SW_PROP_NMID(UNO_NAME_TOP_BORDER_DISTANCE), RES_BOX,              CPPU_E2T(CPPUTYPE_INT32),   0, TOP_BORDER_DISTANCE   |CONVERT_TWIPS },\
                    { SW_PROP_NMID(UNO_NAME_BOTTOM_BORDER_DISTANCE), RES_BOX,               CPPU_E2T(CPPUTYPE_INT32),   0, BOTTOM_BORDER_DISTANCE|CONVERT_TWIPS },\
                    { SW_PROP_NMID(UNO_NAME_PARA_IS_HYPHENATION), RES_PARATR_HYPHENZONE,      CPPU_E2T(CPPUTYPE_BOOLEAN),  PropertyAttribute::MAYBEVOID, MID_IS_HYPHEN         },\
                    { SW_PROP_NMID(UNO_NAME_PARA_HYPHENATION_MAX_LEADING_CHARS), RES_PARATR_HYPHENZONE,         CPPU_E2T(CPPUTYPE_INT16),   PropertyAttribute::MAYBEVOID, MID_HYPHEN_MIN_LEAD   },\
                    { SW_PROP_NMID(UNO_NAME_PARA_HYPHENATION_MAX_TRAILING_CHARS), RES_PARATR_HYPHENZONE,        CPPU_E2T(CPPUTYPE_INT16),   PropertyAttribute::MAYBEVOID, MID_HYPHEN_MIN_TRAIL  },\
                    { SW_PROP_NMID(UNO_NAME_PARA_HYPHENATION_MAX_HYPHENS), RES_PARATR_HYPHENZONE,       CPPU_E2T(CPPUTYPE_INT16),   PropertyAttribute::MAYBEVOID, MID_HYPHEN_MAX_HYPHENS},\
                    { SW_PROP_NMID(UNO_NAME_NUMBERING_STYLE_NAME), RES_PARATR_NUMRULE,  CPPU_E2T(CPPUTYPE_OUSTRING),         PropertyAttribute::MAYBEVOID,   0},\
                    { SW_PROP_NMID(UNO_NAME_PARA_USER_DEFINED_ATTRIBUTES), RES_UNKNOWNATR_CONTAINER, CPPU_E2T(CPPUTYPE_REFNAMECNT), PropertyAttribute::MAYBEVOID, 0 },\
                    { SW_PROP_NMID(UNO_NAME_PARA_SHADOW_FORMAT), RES_SHADOW,    CPPU_E2T(CPPUTYPE_SHADOWFMT),   PROPERTY_NONE, CONVERT_TWIPS},\
                    { SW_PROP_NMID(UNO_NAME_CHAR_COMBINE_IS_ON), RES_CHRATR_TWO_LINES,          CPPU_E2T(CPPUTYPE_BOOLEAN),     PROPERTY_NONE, MID_TWOLINES},\
                    { SW_PROP_NMID(UNO_NAME_CHAR_COMBINE_PREFIX), RES_CHRATR_TWO_LINES,             CPPU_E2T(CPPUTYPE_OUSTRING),    PROPERTY_NONE, MID_START_BRACKET},\
                    { SW_PROP_NMID(UNO_NAME_CHAR_COMBINE_SUFFIX), RES_CHRATR_TWO_LINES,             CPPU_E2T(CPPUTYPE_OUSTRING),    PROPERTY_NONE, MID_END_BRACKET},\
                    { SW_PROP_NMID(UNO_NAME_CHAR_EMPHASIS), RES_CHRATR_EMPHASIS_MARK,           CPPU_E2T(CPPUTYPE_INT16),   PROPERTY_NONE, MID_EMPHASIS},\
                    { SW_PROP_NMID(UNO_NAME_PARA_IS_HANGING_PUNCTUATION), RES_PARATR_HANGINGPUNCTUATION,  CPPU_E2T(CPPUTYPE_BOOLEAN),   PROPERTY_NONE ,0     },\
                    { SW_PROP_NMID(UNO_NAME_PARA_IS_CHARACTER_DISTANCE), RES_PARATR_SCRIPTSPACE,         CPPU_E2T(CPPUTYPE_BOOLEAN),    PROPERTY_NONE ,0     },\
                    { SW_PROP_NMID(UNO_NAME_PARA_IS_FORBIDDEN_RULES), RES_PARATR_FORBIDDEN_RULES,     CPPU_E2T(CPPUTYPE_BOOLEAN),    PROPERTY_NONE ,0    },\
                    { SW_PROP_NMID(UNO_NAME_PARA_VERT_ALIGNMENT), RES_PARATR_VERTALIGN,             CPPU_E2T(CPPUTYPE_INT16),    PROPERTY_NONE , 0  },\
                    { SW_PROP_NMID(UNO_NAME_CHAR_ROTATION), RES_CHRATR_ROTATE,      CPPU_E2T(CPPUTYPE_INT16),   PROPERTY_NONE,    MID_ROTATE      },\
                    { SW_PROP_NMID(UNO_NAME_CHAR_ROTATION_IS_FIT_TO_LINE), RES_CHRATR_ROTATE,       CPPU_E2T(CPPUTYPE_BOOLEAN),     PROPERTY_NONE,        MID_FITTOLINE  },\
                    { SW_PROP_NMID(UNO_NAME_CHAR_SCALE_WIDTH), RES_CHRATR_SCALEW,       CPPU_E2T(CPPUTYPE_INT16),   PROPERTY_NONE,         0 },\
                    { SW_PROP_NMID(UNO_NAME_CHAR_RELIEF), RES_CHRATR_RELIEF,      CPPU_E2T(CPPUTYPE_INT16),    PROPERTY_NONE,      MID_RELIEF },\
                    _PROP_DIFF_FONTHEIGHT\
                    { SW_PROP_NMID(UNO_NAME_FOLLOW_STYLE), FN_UNO_FOLLOW_STYLE,     CPPU_E2T(CPPUTYPE_OUSTRING), PROPERTY_NONE, 0},\
                    { SW_PROP_NMID(UNO_NAME_IS_PHYSICAL), FN_UNO_IS_PHYSICAL,     CPPU_E2T(CPPUTYPE_BOOLEAN), PropertyAttribute::READONLY, 0},\
                    { SW_PROP_NMID(UNO_NAME_IS_AUTO_UPDATE), FN_UNO_IS_AUTO_UPDATE, CPPU_E2T(CPPUTYPE_BOOLEAN), PROPERTY_NONE, 0},\
                    { SW_PROP_NMID(UNO_NAME_DISPLAY_NAME), FN_UNO_DISPLAY_NAME, CPPU_E2T(CPPUTYPE_OUSTRING), PropertyAttribute::READONLY, 0},\
                    { SW_PROP_NMID(UNO_NAME_CATEGORY), FN_UNO_CATEGORY, CPPU_E2T(CPPUTYPE_INT16),           PROPERTY_NONE , 0 },\
                    { SW_PROP_NMID(UNO_NAME_WRITING_MODE), RES_FRAMEDIR, CPPU_E2T(CPPUTYPE_INT16), PROPERTY_NONE, 0 },\
                    { SW_PROP_NMID(UNO_NAME_PARA_IS_CONNECT_BORDER), RES_PARATR_CONNECT_BORDER, CPPU_E2T(CPPUTYPE_BOOLEAN), PropertyAttribute::MAYBEVOID, 0},\
                    { SW_PROP_NMID(UNO_NAME_SNAP_TO_GRID), RES_PARATR_SNAPTOGRID, CPPU_E2T(CPPUTYPE_BOOLEAN), PropertyAttribute::MAYBEVOID, 0 }, \
                    { SW_PROP_NMID(UNO_NAME_OUTLINE_LEVEL), RES_PARATR_OUTLINELEVEL,CPPU_E2T(CPPUTYPE_INT16), PropertyAttribute::MAYBEVOID, 0}, \
                    { SW_PROP_NMID(UNO_NAME_HIDDEN), FN_UNO_HIDDEN,     CPPU_E2T(CPPUTYPE_BOOLEAN), PROPERTY_NONE, 0},


#define COMMON_FLDTYP_PROPERTIES \
                    { SW_PROP_NMID(UNO_NAME_IS_FIELD_USED),      FIELD_PROP_IS_FIELD_USED,      CPPU_E2T(CPPUTYPE_FLOAT), PropertyAttribute::READONLY, 0},\
                    { SW_PROP_NMID(UNO_NAME_IS_FIELD_DISPLAYED), FIELD_PROP_IS_FIELD_DISPLAYED, CPPU_E2T(CPPUTYPE_INT16), PropertyAttribute::READONLY, 0},\


const SfxItemPropertyMapEntry* SwUnoPropertyMapProvider::GetPropertyMapEntries(sal_uInt16 nPropertyId)
{
    OSL_ENSURE(nPropertyId < PROPERTY_MAP_END, "Id ?" );
    if( !aMapEntriesArr[ nPropertyId ] )
    {
        switch(nPropertyId)
        {
            case PROPERTY_MAP_TEXT_CURSOR:
            {
                static SfxItemPropertyMapEntry aCharAndParaMap_Impl[] =
                {
                    COMPLETE_TEXT_CURSOR_MAP
                    {0,0,0,0,0,0}
                };
                aMapEntriesArr[nPropertyId] = aCharAndParaMap_Impl;
            }
            break;
            case PROPERTY_MAP_PARAGRAPH:
            {
                static SfxItemPropertyMapEntry aParagraphMap_Impl[] =
                {
                    COMMON_CRSR_PARA_PROPERTIES_2
                    TABSTOPS_MAP_ENTRY
                    COMMON_TEXT_CONTENT_PROPERTIES
                    { SW_PROP_NMID(UNO_NAME_CHAR_STYLE_NAME), RES_TXTATR_CHARFMT,     CPPU_E2T(CPPUTYPE_OUSTRING),         PropertyAttribute::MAYBEVOID,     0},\
                    { SW_PROP_NMID(UNO_NAME_CHAR_STYLE_NAMES), FN_UNO_CHARFMT_SEQUENCE,  CPPU_E2T(CPPUTYPE_OUSTRINGS),     PropertyAttribute::MAYBEVOID,     0},\
                    {0,0,0,0,0,0}
                };
                aMapEntriesArr[nPropertyId] = aParagraphMap_Impl;
            }
            break;
            case PROPERTY_MAP_PARA_AUTO_STYLE :
            {
                static SfxItemPropertyMapEntry aAutoParaStyleMap [] =
                {
                    { SW_PROP_NMID(UNO_NAME_PARA_STYLE_NAME), RES_FRMATR_STYLE_NAME,        CPPU_E2T(CPPUTYPE_OUSTRING),                PropertyAttribute::MAYBEVOID,     0},                                                       \
                    { SW_PROP_NMID(UNO_NAME_PAGE_STYLE_NAME), FN_UNO_PAGE_STYLE,        CPPU_E2T(CPPUTYPE_OUSTRING),         PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY,   0},                       \
                    { SW_PROP_NMID(UNO_NAME_NUMBERING_IS_NUMBER), FN_UNO_IS_NUMBER,     CPPU_E2T(CPPUTYPE_BOOLEAN)  ,       PropertyAttribute::MAYBEVOID,     0},                                                                 \
                    { SW_PROP_NMID(UNO_NAME_NUMBERING_LEVEL), FN_UNO_NUM_LEVEL,     CPPU_E2T(CPPUTYPE_INT16),           PropertyAttribute::MAYBEVOID, 0},                                                            \
                    { SW_PROP_NMID(UNO_NAME_NUMBERING_START_VALUE), FN_UNO_NUM_START_VALUE, CPPU_E2T(CPPUTYPE_INT16),           PropertyAttribute::MAYBEVOID, CONVERT_TWIPS},                                                \
                    { SW_PROP_NMID(UNO_NAME_DOCUMENT_INDEX), FN_UNO_DOCUMENT_INDEX, CPPU_E2T(CPPUTYPE_REFDOCINDEX), PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY ,0 },            \
                    { SW_PROP_NMID(UNO_NAME_TEXT_TABLE), FN_UNO_TEXT_TABLE,     CPPU_E2T(CPPUTYPE_REFTXTTABLE),     PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY ,0 },               \
                    { SW_PROP_NMID(UNO_NAME_CELL), FN_UNO_CELL,         CPPU_E2T(CPPUTYPE_REFCELL),         PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY ,0 },                     \
                    { SW_PROP_NMID(UNO_NAME_TEXT_FRAME), FN_UNO_TEXT_FRAME,     CPPU_E2T(CPPUTYPE_REFTEXTFRAME),        PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY ,0 },                     \
                    { SW_PROP_NMID(UNO_NAME_TEXT_SECTION), FN_UNO_TEXT_SECTION, CPPU_E2T(CPPUTYPE_REFTEXTSECTION),  PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY ,0 },                    \
                    { SW_PROP_NMID(UNO_NAME_PARA_CHAPTER_NUMBERING_LEVEL), FN_UNO_PARA_CHAPTER_NUMBERING_LEVEL,CPPU_E2T(CPPUTYPE_INT8), PROPERTY_NONE, 0},                                                     \
                    { SW_PROP_NMID(UNO_NAME_PARA_CONDITIONAL_STYLE_NAME), RES_FRMATR_CONDITIONAL_STYLE_NAME,        CPPU_E2T(CPPUTYPE_OUSTRING),         PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY,   0},                       \
                    { SW_PROP_NMID(UNO_NAME_PARA_IS_NUMBERING_RESTART), FN_NUMBER_NEWSTART,     CPPU_E2T(CPPUTYPE_BOOLEAN),     PropertyAttribute::MAYBEVOID, 0 },                       \
                    { SW_PROP_NMID(UNO_NAME_OUTLINE_LEVEL), RES_PARATR_OUTLINELEVEL,        CPPU_E2T(CPPUTYPE_INT16),                PropertyAttribute::MAYBEVOID,     0},
                    COMMON_CRSR_PARA_PROPERTIES_WITHOUT_FN
                    TABSTOPS_MAP_ENTRY
                    COMMON_TEXT_CONTENT_PROPERTIES
                    { SW_PROP_NMID(UNO_NAME_PARA_AUTO_STYLE_NAME), RES_AUTO_STYLE,     CPPU_E2T(CPPUTYPE_OUSTRING),         PropertyAttribute::MAYBEVOID,     0},
                    {0,0,0,0,0,0}
                };
                aMapEntriesArr[nPropertyId] = aAutoParaStyleMap;
            }
            break;
            case PROPERTY_MAP_CHAR_STYLE :
            {
                static SfxItemPropertyMapEntry aCharStyleMap   [] =
                {
                    { SW_PROP_NMID(UNO_NAME_CHAR_AUTO_KERNING), RES_CHRATR_AUTOKERN  ,  CPPU_E2T(CPPUTYPE_BOOLEAN)  ,       PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_CHAR_BACK_TRANSPARENT), RES_CHRATR_BACKGROUND,  CPPU_E2T(CPPUTYPE_BOOLEAN),         PROPERTY_NONE ,MID_GRAPHIC_TRANSPARENT       },
                    { SW_PROP_NMID(UNO_NAME_CHAR_BACK_COLOR), RES_CHRATR_BACKGROUND,    CPPU_E2T(CPPUTYPE_INT32),           PROPERTY_NONE ,MID_BACK_COLOR        },
                    { SW_PROP_NMID(UNO_NAME_CHAR_CASE_MAP), RES_CHRATR_CASEMAP,     CPPU_E2T(CPPUTYPE_INT16),           PROPERTY_NONE, 0},
                    { SW_PROP_NMID(UNO_NAME_CHAR_COLOR), RES_CHRATR_COLOR,      CPPU_E2T(CPPUTYPE_INT32),           PROPERTY_NONE, 0},
                    { SW_PROP_NMID(UNO_NAME_CHAR_STRIKEOUT), RES_CHRATR_CROSSEDOUT,  CPPU_E2T(CPPUTYPE_INT16),                  PropertyAttribute::MAYBEVOID, MID_CROSS_OUT},
                    { SW_PROP_NMID(UNO_NAME_CHAR_CROSSED_OUT), RES_CHRATR_CROSSEDOUT,  CPPU_E2T(CPPUTYPE_BOOLEAN)  ,        PROPERTY_NONE, 0},
                    { SW_PROP_NMID(UNO_NAME_CHAR_ESCAPEMENT), RES_CHRATR_ESCAPEMENT,  CPPU_E2T(CPPUTYPE_INT16),             PROPERTY_NONE, MID_ESC          },
                    { SW_PROP_NMID(UNO_NAME_CHAR_ESCAPEMENT_HEIGHT), RES_CHRATR_ESCAPEMENT,     CPPU_E2T(CPPUTYPE_INT8)  ,          PROPERTY_NONE, MID_ESC_HEIGHT},
                    { SW_PROP_NMID(UNO_NAME_CHAR_FLASH), RES_CHRATR_BLINK   ,   CPPU_E2T(CPPUTYPE_BOOLEAN)  ,       PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_CHAR_HIDDEN), RES_CHRATR_HIDDEN, CPPU_E2T(CPPUTYPE_BOOLEAN)  ,       PROPERTY_NONE,     0},
                    _STANDARD_FONT_PROPERTIES
                    _CJK_FONT_PROPERTIES
                    _CTL_FONT_PROPERTIES
                    { SW_PROP_NMID(UNO_NAME_CHAR_UNDERLINE), RES_CHRATR_UNDERLINE ,  CPPU_E2T(CPPUTYPE_INT16),      PROPERTY_NONE, MID_TL_STYLE},
                    { SW_PROP_NMID(UNO_NAME_CHAR_UNDERLINE_COLOR), RES_CHRATR_UNDERLINE ,  CPPU_E2T(CPPUTYPE_INT32),            PROPERTY_NONE, MID_TL_COLOR},
                    { SW_PROP_NMID(UNO_NAME_CHAR_UNDERLINE_HAS_COLOR), RES_CHRATR_UNDERLINE ,  CPPU_E2T(CPPUTYPE_BOOLEAN),              PROPERTY_NONE, MID_TL_HASCOLOR},
                    { SW_PROP_NMID(UNO_NAME_CHAR_OVERLINE), RES_CHRATR_OVERLINE ,  CPPU_E2T(CPPUTYPE_INT16),    PROPERTY_NONE, MID_TL_STYLE},
                    { SW_PROP_NMID(UNO_NAME_CHAR_OVERLINE_COLOR), RES_CHRATR_OVERLINE ,  CPPU_E2T(CPPUTYPE_INT32),              PROPERTY_NONE, MID_TL_COLOR},
                    { SW_PROP_NMID(UNO_NAME_CHAR_OVERLINE_HAS_COLOR), RES_CHRATR_OVERLINE ,  CPPU_E2T(CPPUTYPE_BOOLEAN),            PROPERTY_NONE, MID_TL_HASCOLOR},
                    { SW_PROP_NMID(UNO_NAME_CHAR_KERNING), RES_CHRATR_KERNING    ,  CPPU_E2T(CPPUTYPE_INT16)  ,         PROPERTY_NONE,  CONVERT_TWIPS},
                    { SW_PROP_NMID(UNO_NAME_CHAR_NO_HYPHENATION), RES_CHRATR_NOHYPHEN   ,   CPPU_E2T(CPPUTYPE_BOOLEAN)  ,       PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_CHAR_SHADOWED), RES_CHRATR_SHADOWED  ,  CPPU_E2T(CPPUTYPE_BOOLEAN)  ,       PROPERTY_NONE, 0},
                    { SW_PROP_NMID(UNO_NAME_CHAR_CONTOURED), RES_CHRATR_CONTOUR,    CPPU_E2T(CPPUTYPE_BOOLEAN)  ,       PROPERTY_NONE, 0},
                    { SW_PROP_NMID(UNO_NAME_CHAR_WORD_MODE), RES_CHRATR_WORDLINEMODE,CPPU_E2T(CPPUTYPE_BOOLEAN)  ,    PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_USER_DEFINED_ATTRIBUTES), RES_UNKNOWNATR_CONTAINER, CPPU_E2T(CPPUTYPE_REFNAMECNT), PropertyAttribute::MAYBEVOID, 0 },
                    { SW_PROP_NMID(UNO_NAME_IS_PHYSICAL), FN_UNO_IS_PHYSICAL,     CPPU_E2T(CPPUTYPE_BOOLEAN), PropertyAttribute::READONLY, 0},
                    { SW_PROP_NMID(UNO_NAME_HIDDEN), FN_UNO_HIDDEN,     CPPU_E2T(CPPUTYPE_BOOLEAN), PROPERTY_NONE, 0},
                    { SW_PROP_NMID(UNO_NAME_DISPLAY_NAME), FN_UNO_DISPLAY_NAME, CPPU_E2T(CPPUTYPE_OUSTRING), PropertyAttribute::READONLY, 0},
                    { SW_PROP_NMID(UNO_NAME_CHAR_COMBINE_IS_ON), RES_CHRATR_TWO_LINES,          CPPU_E2T(CPPUTYPE_BOOLEAN),     PROPERTY_NONE, MID_TWOLINES},
                    { SW_PROP_NMID(UNO_NAME_CHAR_COMBINE_PREFIX), RES_CHRATR_TWO_LINES,             CPPU_E2T(CPPUTYPE_OUSTRING),    PROPERTY_NONE, MID_START_BRACKET},
                    { SW_PROP_NMID(UNO_NAME_CHAR_COMBINE_SUFFIX), RES_CHRATR_TWO_LINES,             CPPU_E2T(CPPUTYPE_OUSTRING),    PROPERTY_NONE, MID_END_BRACKET},
                    { SW_PROP_NMID(UNO_NAME_CHAR_EMPHASIS), RES_CHRATR_EMPHASIS_MARK,           CPPU_E2T(CPPUTYPE_INT16),   PROPERTY_NONE, MID_EMPHASIS},
                    _PROP_DIFF_FONTHEIGHT
                    { SW_PROP_NMID(UNO_NAME_CHAR_ROTATION), RES_CHRATR_ROTATE,      CPPU_E2T(CPPUTYPE_INT16),   PROPERTY_NONE,    MID_ROTATE      },
                    { SW_PROP_NMID(UNO_NAME_CHAR_ROTATION_IS_FIT_TO_LINE), RES_CHRATR_ROTATE,       CPPU_E2T(CPPUTYPE_BOOLEAN),     PROPERTY_NONE,        MID_FITTOLINE  },
                    { SW_PROP_NMID(UNO_NAME_CHAR_SCALE_WIDTH), RES_CHRATR_SCALEW,       CPPU_E2T(CPPUTYPE_INT16),   PROPERTY_NONE,         0 },
                    { SW_PROP_NMID(UNO_NAME_CHAR_RELIEF), RES_CHRATR_RELIEF,      CPPU_E2T(CPPUTYPE_INT16),    PROPERTY_NONE,      MID_RELIEF },
                    { SW_PROP_NMID(UNO_NAME_CHAR_LEFT_BORDER), RES_CHRATR_BOX, CPPU_E2T(CPPUTYPE_BORDERLINE), PROPERTY_NONE, LEFT_BORDER |CONVERT_TWIPS },
                    { SW_PROP_NMID(UNO_NAME_CHAR_RIGHT_BORDER), RES_CHRATR_BOX, CPPU_E2T(CPPUTYPE_BORDERLINE), PROPERTY_NONE, RIGHT_BORDER |CONVERT_TWIPS },
                    { SW_PROP_NMID(UNO_NAME_CHAR_TOP_BORDER), RES_CHRATR_BOX, CPPU_E2T(CPPUTYPE_BORDERLINE), PROPERTY_NONE, TOP_BORDER |CONVERT_TWIPS },
                    { SW_PROP_NMID(UNO_NAME_CHAR_BOTTOM_BORDER), RES_CHRATR_BOX, CPPU_E2T(CPPUTYPE_BORDERLINE), PROPERTY_NONE, BOTTOM_BORDER |CONVERT_TWIPS },
                    { SW_PROP_NMID(UNO_NAME_CHAR_BORDER_DISTANCE), RES_CHRATR_BOX, CPPU_E2T(CPPUTYPE_INT32), PROPERTY_NONE, BORDER_DISTANCE |CONVERT_TWIPS },
                    { SW_PROP_NMID(UNO_NAME_CHAR_LEFT_BORDER_DISTANCE), RES_CHRATR_BOX, CPPU_E2T(CPPUTYPE_INT32), PROPERTY_NONE, LEFT_BORDER_DISTANCE |CONVERT_TWIPS },
                    { SW_PROP_NMID(UNO_NAME_CHAR_RIGHT_BORDER_DISTANCE), RES_CHRATR_BOX, CPPU_E2T(CPPUTYPE_INT32), PROPERTY_NONE, RIGHT_BORDER_DISTANCE |CONVERT_TWIPS },
                    { SW_PROP_NMID(UNO_NAME_CHAR_TOP_BORDER_DISTANCE), RES_CHRATR_BOX, CPPU_E2T(CPPUTYPE_INT32), PROPERTY_NONE, TOP_BORDER_DISTANCE |CONVERT_TWIPS },
                    { SW_PROP_NMID(UNO_NAME_CHAR_BOTTOM_BORDER_DISTANCE), RES_CHRATR_BOX, CPPU_E2T(CPPUTYPE_INT32), PROPERTY_NONE, BOTTOM_BORDER_DISTANCE|CONVERT_TWIPS },
                    {0,0,0,0,0,0}
                };
                aMapEntriesArr[nPropertyId] = aCharStyleMap;
            }
            break;
            case PROPERTY_MAP_CHAR_AUTO_STYLE :
            {
                // same as PROPERTY_MAP_TEXTPORTION_EXTENSIONS
                static SfxItemPropertyMapEntry aAutoCharStyleMap   [] =
                {
                    { SW_PROP_NMID(UNO_NAME_CHAR_AUTO_KERNING), RES_CHRATR_AUTOKERN  ,  CPPU_E2T(CPPUTYPE_BOOLEAN)  ,       PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_CHAR_BACK_TRANSPARENT), RES_CHRATR_BACKGROUND,  CPPU_E2T(CPPUTYPE_BOOLEAN),         PROPERTY_NONE ,MID_GRAPHIC_TRANSPARENT       },
                    { SW_PROP_NMID(UNO_NAME_CHAR_BACK_COLOR), RES_CHRATR_BACKGROUND,    CPPU_E2T(CPPUTYPE_INT32),           PROPERTY_NONE ,MID_BACK_COLOR        },
                    { SW_PROP_NMID(UNO_NAME_CHAR_CASE_MAP), RES_CHRATR_CASEMAP,     CPPU_E2T(CPPUTYPE_INT16),           PROPERTY_NONE, 0},
                    { SW_PROP_NMID(UNO_NAME_CHAR_COLOR), RES_CHRATR_COLOR,      CPPU_E2T(CPPUTYPE_INT32),           PROPERTY_NONE, 0},
                    { SW_PROP_NMID(UNO_NAME_CHAR_STRIKEOUT), RES_CHRATR_CROSSEDOUT,  CPPU_E2T(CPPUTYPE_INT16),                  PropertyAttribute::MAYBEVOID, MID_CROSS_OUT},
                    { SW_PROP_NMID(UNO_NAME_CHAR_CROSSED_OUT), RES_CHRATR_CROSSEDOUT,  CPPU_E2T(CPPUTYPE_BOOLEAN)  ,        PROPERTY_NONE, 0},
                    { SW_PROP_NMID(UNO_NAME_CHAR_ESCAPEMENT), RES_CHRATR_ESCAPEMENT,  CPPU_E2T(CPPUTYPE_INT16),             PROPERTY_NONE, MID_ESC          },
                    { SW_PROP_NMID(UNO_NAME_CHAR_ESCAPEMENT_HEIGHT), RES_CHRATR_ESCAPEMENT,     CPPU_E2T(CPPUTYPE_INT8)  ,          PROPERTY_NONE, MID_ESC_HEIGHT},
                    { SW_PROP_NMID(UNO_NAME_CHAR_FLASH), RES_CHRATR_BLINK   ,   CPPU_E2T(CPPUTYPE_BOOLEAN)  ,       PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_CHAR_HIDDEN), RES_CHRATR_HIDDEN, CPPU_E2T(CPPUTYPE_BOOLEAN)  ,       PROPERTY_NONE,     0},
                    _STANDARD_FONT_PROPERTIES
                    _CJK_FONT_PROPERTIES
                    _CTL_FONT_PROPERTIES
                    { SW_PROP_NMID(UNO_NAME_CHAR_UNDERLINE), RES_CHRATR_UNDERLINE ,  CPPU_E2T(CPPUTYPE_INT16),      PROPERTY_NONE, MID_TL_STYLE},
                    { SW_PROP_NMID(UNO_NAME_CHAR_UNDERLINE_COLOR), RES_CHRATR_UNDERLINE ,  CPPU_E2T(CPPUTYPE_INT32),            PROPERTY_NONE, MID_TL_COLOR},
                    { SW_PROP_NMID(UNO_NAME_CHAR_UNDERLINE_HAS_COLOR), RES_CHRATR_UNDERLINE ,  CPPU_E2T(CPPUTYPE_BOOLEAN),              PROPERTY_NONE, MID_TL_HASCOLOR},
                    { SW_PROP_NMID(UNO_NAME_CHAR_OVERLINE), RES_CHRATR_OVERLINE ,  CPPU_E2T(CPPUTYPE_INT16),      PROPERTY_NONE, MID_TL_STYLE},
                    { SW_PROP_NMID(UNO_NAME_CHAR_OVERLINE_COLOR), RES_CHRATR_OVERLINE ,  CPPU_E2T(CPPUTYPE_INT32),            PROPERTY_NONE, MID_TL_COLOR},
                    { SW_PROP_NMID(UNO_NAME_CHAR_OVERLINE_HAS_COLOR), RES_CHRATR_OVERLINE ,  CPPU_E2T(CPPUTYPE_BOOLEAN),              PROPERTY_NONE, MID_TL_HASCOLOR},
                    { SW_PROP_NMID(UNO_NAME_CHAR_KERNING), RES_CHRATR_KERNING    ,  CPPU_E2T(CPPUTYPE_INT16)  ,         PROPERTY_NONE,  CONVERT_TWIPS},
                    { SW_PROP_NMID(UNO_NAME_CHAR_NO_HYPHENATION), RES_CHRATR_NOHYPHEN   ,   CPPU_E2T(CPPUTYPE_BOOLEAN)  ,       PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_CHAR_SHADOWED), RES_CHRATR_SHADOWED  ,  CPPU_E2T(CPPUTYPE_BOOLEAN)  ,       PROPERTY_NONE, 0},
                    { SW_PROP_NMID(UNO_NAME_CHAR_CONTOURED), RES_CHRATR_CONTOUR,    CPPU_E2T(CPPUTYPE_BOOLEAN)  ,       PROPERTY_NONE, 0},
                    { SW_PROP_NMID(UNO_NAME_CHAR_WORD_MODE), RES_CHRATR_WORDLINEMODE,CPPU_E2T(CPPUTYPE_BOOLEAN)  ,    PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_USER_DEFINED_ATTRIBUTES), RES_UNKNOWNATR_CONTAINER, CPPU_E2T(CPPUTYPE_REFNAMECNT), PropertyAttribute::MAYBEVOID, 0 },
                    { SW_PROP_NMID(UNO_NAME_TEXT_USER_DEFINED_ATTRIBUTES), RES_TXTATR_UNKNOWN_CONTAINER, CPPU_E2T(CPPUTYPE_REFNAMECNT), PropertyAttribute::MAYBEVOID, 0 },
                    { SW_PROP_NMID(UNO_NAME_IS_PHYSICAL), FN_UNO_IS_PHYSICAL,     CPPU_E2T(CPPUTYPE_BOOLEAN), PropertyAttribute::READONLY, 0},
                    { SW_PROP_NMID(UNO_NAME_DISPLAY_NAME), FN_UNO_DISPLAY_NAME, CPPU_E2T(CPPUTYPE_OUSTRING), PropertyAttribute::READONLY, 0},
                    { SW_PROP_NMID(UNO_NAME_CHAR_COMBINE_IS_ON), RES_CHRATR_TWO_LINES,          CPPU_E2T(CPPUTYPE_BOOLEAN),     PROPERTY_NONE, MID_TWOLINES},
                    { SW_PROP_NMID(UNO_NAME_CHAR_COMBINE_PREFIX), RES_CHRATR_TWO_LINES,             CPPU_E2T(CPPUTYPE_OUSTRING),    PROPERTY_NONE, MID_START_BRACKET},
                    { SW_PROP_NMID(UNO_NAME_CHAR_COMBINE_SUFFIX), RES_CHRATR_TWO_LINES,             CPPU_E2T(CPPUTYPE_OUSTRING),    PROPERTY_NONE, MID_END_BRACKET},
                    { SW_PROP_NMID(UNO_NAME_CHAR_EMPHASIS), RES_CHRATR_EMPHASIS_MARK,           CPPU_E2T(CPPUTYPE_INT16),   PROPERTY_NONE, MID_EMPHASIS},
                    { SW_PROP_NMID(UNO_NAME_CHAR_ROTATION), RES_CHRATR_ROTATE,      CPPU_E2T(CPPUTYPE_INT16),   PROPERTY_NONE,    MID_ROTATE      },
                    { SW_PROP_NMID(UNO_NAME_CHAR_ROTATION_IS_FIT_TO_LINE), RES_CHRATR_ROTATE,       CPPU_E2T(CPPUTYPE_BOOLEAN),     PROPERTY_NONE,        MID_FITTOLINE  },
                    { SW_PROP_NMID(UNO_NAME_CHAR_SCALE_WIDTH), RES_CHRATR_SCALEW,       CPPU_E2T(CPPUTYPE_INT16),   PROPERTY_NONE,         0 },
                    { SW_PROP_NMID(UNO_NAME_CHAR_RELIEF), RES_CHRATR_RELIEF,      CPPU_E2T(CPPUTYPE_INT16),    PROPERTY_NONE,      MID_RELIEF },
                    { SW_PROP_NMID(UNO_NAME_CHAR_AUTO_STYLE_NAME), RES_TXTATR_AUTOFMT,     CPPU_E2T(CPPUTYPE_OUSTRING),         PropertyAttribute::MAYBEVOID,     0},
                    { SW_PROP_NMID(UNO_NAME_CHAR_SHADING_VALUE), RES_CHRATR_BACKGROUND,      CPPU_E2T(CPPUTYPE_INT32),    PROPERTY_NONE,      MID_SHADING_VALUE },
                    { SW_PROP_NMID(UNO_NAME_CHAR_LEFT_BORDER), RES_CHRATR_BOX, CPPU_E2T(CPPUTYPE_BORDERLINE), PROPERTY_NONE, LEFT_BORDER |CONVERT_TWIPS },
                    { SW_PROP_NMID(UNO_NAME_CHAR_RIGHT_BORDER), RES_CHRATR_BOX, CPPU_E2T(CPPUTYPE_BORDERLINE), PROPERTY_NONE, RIGHT_BORDER |CONVERT_TWIPS },
                    { SW_PROP_NMID(UNO_NAME_CHAR_TOP_BORDER), RES_CHRATR_BOX, CPPU_E2T(CPPUTYPE_BORDERLINE), PROPERTY_NONE, TOP_BORDER |CONVERT_TWIPS },
                    { SW_PROP_NMID(UNO_NAME_CHAR_BOTTOM_BORDER), RES_CHRATR_BOX, CPPU_E2T(CPPUTYPE_BORDERLINE), PROPERTY_NONE, BOTTOM_BORDER |CONVERT_TWIPS },
                    { SW_PROP_NMID(UNO_NAME_CHAR_BORDER_DISTANCE), RES_CHRATR_BOX, CPPU_E2T(CPPUTYPE_INT32), PROPERTY_NONE, BORDER_DISTANCE |CONVERT_TWIPS },
                    { SW_PROP_NMID(UNO_NAME_CHAR_LEFT_BORDER_DISTANCE), RES_CHRATR_BOX, CPPU_E2T(CPPUTYPE_INT32), PROPERTY_NONE, LEFT_BORDER_DISTANCE |CONVERT_TWIPS },
                    { SW_PROP_NMID(UNO_NAME_CHAR_RIGHT_BORDER_DISTANCE), RES_CHRATR_BOX, CPPU_E2T(CPPUTYPE_INT32), PROPERTY_NONE, RIGHT_BORDER_DISTANCE |CONVERT_TWIPS },
                    { SW_PROP_NMID(UNO_NAME_CHAR_TOP_BORDER_DISTANCE), RES_CHRATR_BOX, CPPU_E2T(CPPUTYPE_INT32), PROPERTY_NONE, TOP_BORDER_DISTANCE |CONVERT_TWIPS },
                    { SW_PROP_NMID(UNO_NAME_CHAR_BOTTOM_BORDER_DISTANCE), RES_CHRATR_BOX, CPPU_E2T(CPPUTYPE_INT32), PROPERTY_NONE, BOTTOM_BORDER_DISTANCE|CONVERT_TWIPS },
                    {0,0,0,0,0,0}
                };
                aMapEntriesArr[nPropertyId] = aAutoCharStyleMap;
            }
            break;
            case PROPERTY_MAP_RUBY_AUTO_STYLE :
            {
                static SfxItemPropertyMapEntry aAutoRubyStyleMap [] =
                {
                    { SW_PROP_NMID(UNO_NAME_RUBY_ADJUST), RES_TXTATR_CJK_RUBY,  CPPU_E2T(CPPUTYPE_INT16),   PropertyAttribute::MAYBEVOID,          MID_RUBY_ADJUST },
                    { SW_PROP_NMID(UNO_NAME_RUBY_IS_ABOVE), RES_TXTATR_CJK_RUBY,    CPPU_E2T(CPPUTYPE_BOOLEAN),  PropertyAttribute::MAYBEVOID,     MID_RUBY_ABOVE },
                    {0,0,0,0,0,0}
                };
                aMapEntriesArr[nPropertyId] = aAutoRubyStyleMap;
            }
            break;
            case PROPERTY_MAP_PARA_STYLE :
            {
                static SfxItemPropertyMapEntry aParaStyleMap [] =
                {
                    COMMON_PARA_STYLE_PROPERTIES
                    {0,0,0,0,0,0}
                };
                aMapEntriesArr[nPropertyId] = aParaStyleMap;
            }
            break;
            case PROPERTY_MAP_CONDITIONAL_PARA_STYLE :
            {
                static SfxItemPropertyMapEntry aParaStyleMap [] =
                {
                    COMMON_PARA_STYLE_PROPERTIES
                    { SW_PROP_NMID(UNO_NAME_PARA_STYLE_CONDITIONS), FN_UNO_PARA_STYLE_CONDITIONS, CPPU_E2T(CPPUTYPE_SEQNAMEDVALUE), PropertyAttribute::MAYBEVOID, 0},
                    {0,0,0,0,0,0}
                };
                aMapEntriesArr[nPropertyId] = aParaStyleMap;
            }
            break;
            case PROPERTY_MAP_FRAME_STYLE:
            {
                static SfxItemPropertyMapEntry aFrameStyleMap   [] =
                {
                    { SW_PROP_NMID(UNO_NAME_ANCHOR_PAGE_NO), RES_ANCHOR,            CPPU_E2T(CPPUTYPE_INT16),           PROPERTY_NONE, MID_ANCHOR_PAGENUM       },
                    { SW_PROP_NMID(UNO_NAME_ANCHOR_TYPE), RES_ANCHOR,           CPPU_E2T(CPPUTYPE_TXTCNTANCHOR),            PROPERTY_NONE, MID_ANCHOR_ANCHORTYPE},
                    { SW_PROP_NMID(UNO_NAME_BACK_COLOR), RES_BACKGROUND,            CPPU_E2T(CPPUTYPE_INT32),           PROPERTY_NONE ,MID_BACK_COLOR        },
                    { SW_PROP_NMID(UNO_NAME_BACK_COLOR_R_G_B), RES_BACKGROUND,      CPPU_E2T(CPPUTYPE_INT32), PROPERTY_NONE ,MID_BACK_COLOR_R_G_B},
                    { SW_PROP_NMID(UNO_NAME_BACK_COLOR_TRANSPARENCY), RES_BACKGROUND,      CPPU_E2T(CPPUTYPE_INT8), PROPERTY_NONE ,MID_BACK_COLOR_TRANSPARENCY},    \
                    { SW_PROP_NMID(UNO_NAME_FILL_STYLE), RES_FILL_STYLE,      CPPU_E2T(CPPUTYPE_FILLSTYLE), PROPERTY_NONE ,0},
                    { SW_PROP_NMID(UNO_NAME_FILL_GRADIENT), RES_FILL_GRADIENT,      CPPU_E2T(CPPUTYPE_GRADIENT), PROPERTY_NONE ,MID_FILLGRADIENT},
                    { SW_PROP_NMID(UNO_NAME_FILL_GRADIENT_NAME), RES_FILL_GRADIENT, CPPU_E2T(CPPUTYPE_OUSTRING), PROPERTY_NONE ,MID_NAME},
                //  { SW_PROP_NMID(UNO_NAME_CHAIN_NEXT_NAME), RES_CHAIN,                CPPU_E2T(CPPUTYPE_OUSTRING),            PROPERTY_NONE ,MID_CHAIN_NEXTNAME},
                //  { SW_PROP_NMID(UNO_NAME_CHAIN_PREV_NAME), RES_CHAIN,                CPPU_E2T(CPPUTYPE_OUSTRING),            PROPERTY_NONE ,MID_CHAIN_PREVNAME},
                /*not impl*/    { SW_PROP_NMID(UNO_NAME_CLIENT_MAP), RES_URL,               CPPU_E2T(CPPUTYPE_BOOLEAN),         PROPERTY_NONE ,MID_URL_CLIENTMAP         },
                    { SW_PROP_NMID(UNO_NAME_CONTENT_PROTECTED), RES_PROTECT,            CPPU_E2T(CPPUTYPE_BOOLEAN),             PROPERTY_NONE, MID_PROTECT_CONTENT   },
                    { SW_PROP_NMID(UNO_NAME_EDIT_IN_READONLY), RES_EDIT_IN_READONLY,    CPPU_E2T(CPPUTYPE_BOOLEAN),         PROPERTY_NONE, 0},
                //  { SW_PROP_NMID(UNO_NAME_GRAPHIC), RES_BACKGROUND,       &,                              PROPERTY_NONE, MID_GRAPHIC
                    { SW_PROP_NMID(UNO_NAME_BACK_GRAPHIC_URL), RES_BACKGROUND,      CPPU_E2T(CPPUTYPE_OUSTRING), PROPERTY_NONE ,MID_GRAPHIC_URL    },
                    { SW_PROP_NMID(UNO_NAME_BACK_GRAPHIC_FILTER), RES_BACKGROUND,       CPPU_E2T(CPPUTYPE_OUSTRING), PROPERTY_NONE ,MID_GRAPHIC_FILTER    },
                    { SW_PROP_NMID(UNO_NAME_BACK_GRAPHIC_LOCATION), RES_BACKGROUND,         CPPU_E2T(CPPUTYPE_GRAPHICLOC),          PROPERTY_NONE ,MID_GRAPHIC_POSITION},
                    // #i50322# - add missing map entry for transparency of graphic background
                    { SW_PROP_NMID(UNO_NAME_BACK_GRAPHIC_TRANSPARENCY), RES_BACKGROUND, CPPU_E2T(CPPUTYPE_INT8), PROPERTY_NONE ,MID_GRAPHIC_TRANSPARENCY},
                    { SW_PROP_NMID(UNO_NAME_LEFT_MARGIN), RES_LR_SPACE,             CPPU_E2T(CPPUTYPE_INT32), PROPERTY_NONE, MID_L_MARGIN|CONVERT_TWIPS},
                    { SW_PROP_NMID(UNO_NAME_RIGHT_MARGIN), RES_LR_SPACE,            CPPU_E2T(CPPUTYPE_INT32), PROPERTY_NONE, MID_R_MARGIN|CONVERT_TWIPS},
                    { SW_PROP_NMID(UNO_NAME_HORI_ORIENT), RES_HORI_ORIENT,      CPPU_E2T(CPPUTYPE_INT16),           PROPERTY_NONE ,MID_HORIORIENT_ORIENT    },
                    { SW_PROP_NMID(UNO_NAME_HORI_ORIENT_POSITION), RES_HORI_ORIENT,     CPPU_E2T(CPPUTYPE_INT32),           PROPERTY_NONE ,MID_HORIORIENT_POSITION|CONVERT_TWIPS    },
                    { SW_PROP_NMID(UNO_NAME_HORI_ORIENT_RELATION), RES_HORI_ORIENT,     CPPU_E2T(CPPUTYPE_INT16),           PROPERTY_NONE ,MID_HORIORIENT_RELATION  },
                    { SW_PROP_NMID(UNO_NAME_HYPER_LINK_U_R_L), RES_URL,                 CPPU_E2T(CPPUTYPE_OUSTRING), PROPERTY_NONE ,MID_URL_URL},
                    { SW_PROP_NMID(UNO_NAME_HYPER_LINK_TARGET), RES_URL,                CPPU_E2T(CPPUTYPE_OUSTRING), PROPERTY_NONE ,MID_URL_TARGET},
                    { SW_PROP_NMID(UNO_NAME_HYPER_LINK_NAME), RES_URL,              CPPU_E2T(CPPUTYPE_OUSTRING),            PROPERTY_NONE ,MID_URL_HYPERLINKNAME     },
                    { SW_PROP_NMID(UNO_NAME_OPAQUE), RES_OPAQUE,            CPPU_E2T(CPPUTYPE_BOOLEAN),         PROPERTY_NONE, 0},
                    { SW_PROP_NMID(UNO_NAME_PAGE_TOGGLE), RES_HORI_ORIENT,      CPPU_E2T(CPPUTYPE_BOOLEAN),         PROPERTY_NONE ,MID_HORIORIENT_PAGETOGGLE },
                    { SW_PROP_NMID(UNO_NAME_POSITION_PROTECTED), RES_PROTECT,           CPPU_E2T(CPPUTYPE_BOOLEAN),             PROPERTY_NONE, MID_PROTECT_POSITION},
                    { SW_PROP_NMID(UNO_NAME_PRINT), RES_PRINT,              CPPU_E2T(CPPUTYPE_BOOLEAN),         PROPERTY_NONE, 0},
                    { SW_PROP_NMID(UNO_NAME_WIDTH), RES_FRM_SIZE,           CPPU_E2T(CPPUTYPE_INT32)  ,         PROPERTY_NONE, MID_FRMSIZE_WIDTH|CONVERT_TWIPS          },
                    { SW_PROP_NMID(UNO_NAME_HEIGHT), RES_FRM_SIZE,          CPPU_E2T(CPPUTYPE_INT32)  ,         PROPERTY_NONE, MID_FRMSIZE_HEIGHT|CONVERT_TWIPS         },
                    { SW_PROP_NMID(UNO_NAME_RELATIVE_HEIGHT), RES_FRM_SIZE,         CPPU_E2T(CPPUTYPE_INT16)  ,         PROPERTY_NONE,   MID_FRMSIZE_REL_HEIGHT },
                    { SW_PROP_NMID(UNO_NAME_RELATIVE_WIDTH), RES_FRM_SIZE,          CPPU_E2T(CPPUTYPE_INT16)  ,         PROPERTY_NONE,   MID_FRMSIZE_REL_WIDTH  },
                    { SW_PROP_NMID(UNO_NAME_SIZE_TYPE), RES_FRM_SIZE,           CPPU_E2T(CPPUTYPE_INT16)  ,         PROPERTY_NONE,   MID_FRMSIZE_SIZE_TYPE  },
                    { SW_PROP_NMID(UNO_NAME_WIDTH_TYPE), RES_FRM_SIZE,          CPPU_E2T(CPPUTYPE_INT16)  ,         PROPERTY_NONE,   MID_FRMSIZE_WIDTH_TYPE },
                    { SW_PROP_NMID(UNO_NAME_SIZE), RES_FRM_SIZE,            CPPU_E2T(CPPUTYPE_AWTSIZE),             PROPERTY_NONE, MID_FRMSIZE_SIZE|CONVERT_TWIPS},
                    { SW_PROP_NMID(UNO_NAME_IS_SYNC_WIDTH_TO_HEIGHT), RES_FRM_SIZE,         CPPU_E2T(CPPUTYPE_BOOLEAN)  ,       PROPERTY_NONE,   MID_FRMSIZE_IS_SYNC_WIDTH_TO_HEIGHT    },
                    { SW_PROP_NMID(UNO_NAME_IS_SYNC_HEIGHT_TO_WIDTH), RES_FRM_SIZE,         CPPU_E2T(CPPUTYPE_BOOLEAN)  ,       PROPERTY_NONE,   MID_FRMSIZE_IS_SYNC_HEIGHT_TO_WIDTH },
                //  { SW_PROP_NMID(UNO_NAME_WIDTH), RES_FRM_SIZE,           CPPU_E2T(CPPUTYPE_INT32)  ,         PROPERTY_NONE, MID_FRMSIZE_WIDTH            },
                    { SW_PROP_NMID(UNO_NAME_SHADOW_FORMAT), RES_SHADOW,             CPPU_E2T(CPPUTYPE_SHADOWFMT),   PROPERTY_NONE, CONVERT_TWIPS},
                    { SW_PROP_NMID(UNO_NAME_SERVER_MAP), RES_URL,               CPPU_E2T(CPPUTYPE_BOOLEAN),         PROPERTY_NONE ,MID_URL_SERVERMAP         },
                    { SW_PROP_NMID(UNO_NAME_SIZE_PROTECTED), RES_PROTECT,           CPPU_E2T(CPPUTYPE_BOOLEAN),             PROPERTY_NONE, MID_PROTECT_SIZE    },
                    //Surround bleibt, weil es mit der 5.1 ausgeliefert wurde, obwohl es mit text::WrapTextMode identisch ist
                    { SW_PROP_NMID(UNO_NAME_SURROUND), RES_SURROUND,          CPPU_E2T(CPPUTYPE_WRAPTXTMODE),    PROPERTY_NONE, MID_SURROUND_SURROUNDTYPE },
                    { SW_PROP_NMID(UNO_NAME_TEXT_WRAP), RES_SURROUND,           CPPU_E2T(CPPUTYPE_WRAPTXTMODE),             PROPERTY_NONE, MID_SURROUND_SURROUNDTYPE    },
                    { SW_PROP_NMID(UNO_NAME_SURROUND_ANCHORONLY), RES_SURROUND,             CPPU_E2T(CPPUTYPE_BOOLEAN),             PROPERTY_NONE, MID_SURROUND_ANCHORONLY      },
                    { SW_PROP_NMID(UNO_NAME_SURROUND_CONTOUR), RES_SURROUND,            CPPU_E2T(CPPUTYPE_BOOLEAN),             PROPERTY_NONE, MID_SURROUND_CONTOUR         },
                    { SW_PROP_NMID(UNO_NAME_CONTOUR_OUTSIDE), RES_SURROUND,             CPPU_E2T(CPPUTYPE_BOOLEAN),             PROPERTY_NONE, MID_SURROUND_CONTOUROUTSIDE  },
                    { SW_PROP_NMID(UNO_NAME_TEXT_COLUMNS), RES_COL,                CPPU_E2T(CPPUTYPE_REFTEXTCOL),    PROPERTY_NONE, MID_COLUMNS},
                    { SW_PROP_NMID(UNO_NAME_TOP_MARGIN), RES_UL_SPACE,          CPPU_E2T(CPPUTYPE_INT32), PROPERTY_NONE, MID_UP_MARGIN|CONVERT_TWIPS},
                    { SW_PROP_NMID(UNO_NAME_BOTTOM_MARGIN), RES_UL_SPACE,           CPPU_E2T(CPPUTYPE_INT32), PROPERTY_NONE, MID_LO_MARGIN|CONVERT_TWIPS},
                    { SW_PROP_NMID(UNO_NAME_BACK_TRANSPARENT), RES_BACKGROUND,      CPPU_E2T(CPPUTYPE_BOOLEAN),         PROPERTY_NONE ,MID_GRAPHIC_TRANSPARENT       },
                    { SW_PROP_NMID(UNO_NAME_VERT_ORIENT), RES_VERT_ORIENT,      CPPU_E2T(CPPUTYPE_INT16),           PROPERTY_NONE ,MID_VERTORIENT_ORIENT    },
                    { SW_PROP_NMID(UNO_NAME_VERT_ORIENT_POSITION), RES_VERT_ORIENT,     CPPU_E2T(CPPUTYPE_INT32),           PROPERTY_NONE ,MID_VERTORIENT_POSITION|CONVERT_TWIPS    },
                    { SW_PROP_NMID(UNO_NAME_VERT_ORIENT_RELATION), RES_VERT_ORIENT,     CPPU_E2T(CPPUTYPE_INT16),           PROPERTY_NONE ,MID_VERTORIENT_RELATION  },
                    { SW_PROP_NMID(UNO_NAME_LEFT_BORDER), RES_BOX,              CPPU_E2T(CPPUTYPE_BORDERLINE),  0, LEFT_BORDER  |CONVERT_TWIPS },
                    { SW_PROP_NMID(UNO_NAME_RIGHT_BORDER), RES_BOX,             CPPU_E2T(CPPUTYPE_BORDERLINE),  0, RIGHT_BORDER |CONVERT_TWIPS },
                    { SW_PROP_NMID(UNO_NAME_TOP_BORDER), RES_BOX,               CPPU_E2T(CPPUTYPE_BORDERLINE),  0, TOP_BORDER   |CONVERT_TWIPS },
                    { SW_PROP_NMID(UNO_NAME_BOTTOM_BORDER), RES_BOX,                CPPU_E2T(CPPUTYPE_BORDERLINE),  0, BOTTOM_BORDER|CONVERT_TWIPS },
                    { SW_PROP_NMID(UNO_NAME_BORDER_DISTANCE), RES_BOX,    CPPU_E2T(CPPUTYPE_INT32), 0, BORDER_DISTANCE|CONVERT_TWIPS },
                    { SW_PROP_NMID(UNO_NAME_LEFT_BORDER_DISTANCE), RES_BOX,             CPPU_E2T(CPPUTYPE_INT32),   0, LEFT_BORDER_DISTANCE  |CONVERT_TWIPS },
                    { SW_PROP_NMID(UNO_NAME_RIGHT_BORDER_DISTANCE), RES_BOX,                CPPU_E2T(CPPUTYPE_INT32),   0, RIGHT_BORDER_DISTANCE |CONVERT_TWIPS },
                    { SW_PROP_NMID(UNO_NAME_TOP_BORDER_DISTANCE), RES_BOX,              CPPU_E2T(CPPUTYPE_INT32),   0, TOP_BORDER_DISTANCE   |CONVERT_TWIPS },
                    { SW_PROP_NMID(UNO_NAME_BOTTOM_BORDER_DISTANCE), RES_BOX,               CPPU_E2T(CPPUTYPE_INT32),   0, BOTTOM_BORDER_DISTANCE|CONVERT_TWIPS },
                    { SW_PROP_NMID(UNO_NAME_USER_DEFINED_ATTRIBUTES), RES_UNKNOWNATR_CONTAINER, CPPU_E2T(CPPUTYPE_REFNAMECNT), PropertyAttribute::MAYBEVOID, 0 },
                    { SW_PROP_NMID(UNO_NAME_IS_PHYSICAL), FN_UNO_IS_PHYSICAL,     CPPU_E2T(CPPUTYPE_BOOLEAN), PropertyAttribute::READONLY, 0},
                    { SW_PROP_NMID(UNO_NAME_IS_AUTO_UPDATE), FN_UNO_IS_AUTO_UPDATE, CPPU_E2T(CPPUTYPE_BOOLEAN), PROPERTY_NONE, 0},
                    { SW_PROP_NMID(UNO_NAME_DISPLAY_NAME), FN_UNO_DISPLAY_NAME, CPPU_E2T(CPPUTYPE_OUSTRING), PropertyAttribute::READONLY, 0},
                    // #i18732#
                    { SW_PROP_NMID(UNO_NAME_IS_FOLLOWING_TEXT_FLOW), RES_FOLLOW_TEXT_FLOW,     CPPU_E2T(CPPUTYPE_BOOLEAN), PROPERTY_NONE, 0},
                    // #i28701#
                    { SW_PROP_NMID(UNO_NAME_WRAP_INFLUENCE_ON_POSITION), RES_WRAP_INFLUENCE_ON_OBJPOS, CPPU_E2T(CPPUTYPE_INT8), PROPERTY_NONE, MID_WRAP_INFLUENCE},
                    { SW_PROP_NMID(UNO_NAME_WRITING_MODE), RES_FRAMEDIR, CPPU_E2T(CPPUTYPE_INT16), PROPERTY_NONE, 0 },
                    { SW_PROP_NMID(UNO_NAME_HIDDEN), FN_UNO_HIDDEN,     CPPU_E2T(CPPUTYPE_BOOLEAN), PROPERTY_NONE, 0},
                    {0,0,0,0,0,0}
                };
                aMapEntriesArr[nPropertyId] = aFrameStyleMap;
            }
            break;
            case PROPERTY_MAP_PAGE_STYLE :
            {
                static SfxItemPropertyMapEntry aPageStyleMap   [] =
                {
                    { SW_PROP_NMID(UNO_NAME_BACK_COLOR), RES_BACKGROUND,            CPPU_E2T(CPPUTYPE_INT32),           PROPERTY_NONE ,MID_BACK_COLOR        },
                //  { SW_PROP_NMID(UNO_NAME_GRAPHIC), RES_BACKGROUND,       &,                              PROPERTY_NONE, MID_GRAPHIC
                    { SW_PROP_NMID(UNO_NAME_BACK_GRAPHIC_URL), RES_BACKGROUND,      CPPU_E2T(CPPUTYPE_OUSTRING), PROPERTY_NONE ,MID_GRAPHIC_URL    },
                    { SW_PROP_NMID(UNO_NAME_BACK_GRAPHIC_FILTER), RES_BACKGROUND,       CPPU_E2T(CPPUTYPE_OUSTRING), PROPERTY_NONE ,MID_GRAPHIC_FILTER    },
                    { SW_PROP_NMID(UNO_NAME_BACK_GRAPHIC_LOCATION), RES_BACKGROUND,         CPPU_E2T(CPPUTYPE_GRAPHICLOC), PROPERTY_NONE ,MID_GRAPHIC_POSITION},
                    { SW_PROP_NMID(UNO_NAME_LEFT_MARGIN), RES_LR_SPACE,             CPPU_E2T(CPPUTYPE_INT32), PROPERTY_NONE, MID_L_MARGIN|CONVERT_TWIPS},
                    { SW_PROP_NMID(UNO_NAME_RIGHT_MARGIN), RES_LR_SPACE,            CPPU_E2T(CPPUTYPE_INT32), PROPERTY_NONE, MID_R_MARGIN|CONVERT_TWIPS},
                    { SW_PROP_NMID(UNO_NAME_BACK_TRANSPARENT), RES_BACKGROUND,      CPPU_E2T(CPPUTYPE_BOOLEAN),         PROPERTY_NONE ,MID_GRAPHIC_TRANSPARENT       },
                    { SW_PROP_NMID(UNO_NAME_LEFT_BORDER), RES_BOX,              CPPU_E2T(CPPUTYPE_BORDERLINE),  0, LEFT_BORDER  |CONVERT_TWIPS },
                    { SW_PROP_NMID(UNO_NAME_RIGHT_BORDER), RES_BOX,             CPPU_E2T(CPPUTYPE_BORDERLINE),  0, RIGHT_BORDER |CONVERT_TWIPS },
                    { SW_PROP_NMID(UNO_NAME_TOP_BORDER), RES_BOX,               CPPU_E2T(CPPUTYPE_BORDERLINE),  0, TOP_BORDER   |CONVERT_TWIPS },
                    { SW_PROP_NMID(UNO_NAME_BOTTOM_BORDER), RES_BOX,                CPPU_E2T(CPPUTYPE_BORDERLINE),  0, BOTTOM_BORDER|CONVERT_TWIPS },
                    { SW_PROP_NMID(UNO_NAME_BORDER_DISTANCE), RES_BOX,    CPPU_E2T(CPPUTYPE_INT32), 0, BORDER_DISTANCE|CONVERT_TWIPS },
                    { SW_PROP_NMID(UNO_NAME_LEFT_BORDER_DISTANCE), RES_BOX,             CPPU_E2T(CPPUTYPE_INT32),   0, LEFT_BORDER_DISTANCE  |CONVERT_TWIPS },
                    { SW_PROP_NMID(UNO_NAME_RIGHT_BORDER_DISTANCE), RES_BOX,                CPPU_E2T(CPPUTYPE_INT32),   0, RIGHT_BORDER_DISTANCE |CONVERT_TWIPS },
                    { SW_PROP_NMID(UNO_NAME_TOP_BORDER_DISTANCE), RES_BOX,              CPPU_E2T(CPPUTYPE_INT32),   0, TOP_BORDER_DISTANCE   |CONVERT_TWIPS },
                    { SW_PROP_NMID(UNO_NAME_BOTTOM_BORDER_DISTANCE), RES_BOX,               CPPU_E2T(CPPUTYPE_INT32),   0, BOTTOM_BORDER_DISTANCE|CONVERT_TWIPS },
                    { SW_PROP_NMID(UNO_NAME_SHADOW_FORMAT), RES_SHADOW,             CPPU_E2T(CPPUTYPE_SHADOWFMT),   PROPERTY_NONE, CONVERT_TWIPS},

                    { SW_PROP_NMID(UNO_NAME_HEADER_BACK_COLOR), FN_UNO_HEADER_BACKGROUND,   CPPU_E2T(CPPUTYPE_INT32),           PROPERTY_NONE ,MID_BACK_COLOR        },
                //  { SW_PROP_NMID(UNO_NAME_HEADER_GRAPHIC), FN_UNO_HEADER_BACKGROUND,  &,                              PROPERTY_NONE, MID_GRAPHIC
                    { SW_PROP_NMID(UNO_NAME_HEADER_GRAPHIC_URL), FN_UNO_HEADER_BACKGROUND,          CPPU_E2T(CPPUTYPE_OUSTRING), PROPERTY_NONE ,MID_GRAPHIC_URL    },
                    { SW_PROP_NMID(UNO_NAME_HEADER_GRAPHIC_FILTER), FN_UNO_HEADER_BACKGROUND,           CPPU_E2T(CPPUTYPE_OUSTRING), PROPERTY_NONE ,MID_GRAPHIC_FILTER    },
                    { SW_PROP_NMID(UNO_NAME_HEADER_GRAPHIC_LOCATION), FN_UNO_HEADER_BACKGROUND,     CPPU_E2T(CPPUTYPE_GRAPHICLOC), PROPERTY_NONE ,MID_GRAPHIC_POSITION},
                    { SW_PROP_NMID(UNO_NAME_HEADER_LEFT_MARGIN), FN_UNO_HEADER_LR_SPACE,    CPPU_E2T(CPPUTYPE_INT32), PROPERTY_NONE, MID_L_MARGIN|CONVERT_TWIPS},
                    { SW_PROP_NMID(UNO_NAME_HEADER_RIGHT_MARGIN), FN_UNO_HEADER_LR_SPACE,   CPPU_E2T(CPPUTYPE_INT32), PROPERTY_NONE, MID_R_MARGIN|CONVERT_TWIPS},
                    { SW_PROP_NMID(UNO_NAME_HEADER_BACK_TRANSPARENT), FN_UNO_HEADER_BACKGROUND,     CPPU_E2T(CPPUTYPE_BOOLEAN),         PROPERTY_NONE ,MID_GRAPHIC_TRANSPARENT       },
                    { SW_PROP_NMID(UNO_NAME_HEADER_LEFT_BORDER), FN_UNO_HEADER_BOX,             CPPU_E2T(CPPUTYPE_BORDERLINE),  0, LEFT_BORDER  |CONVERT_TWIPS },
                    { SW_PROP_NMID(UNO_NAME_HEADER_RIGHT_BORDER), FN_UNO_HEADER_BOX,                CPPU_E2T(CPPUTYPE_BORDERLINE),  0, RIGHT_BORDER |CONVERT_TWIPS },
                    { SW_PROP_NMID(UNO_NAME_HEADER_TOP_BORDER), FN_UNO_HEADER_BOX,              CPPU_E2T(CPPUTYPE_BORDERLINE),  0, TOP_BORDER   |CONVERT_TWIPS },
                    { SW_PROP_NMID(UNO_NAME_HEADER_BOTTOM_BORDER), FN_UNO_HEADER_BOX,               CPPU_E2T(CPPUTYPE_BORDERLINE),  0, BOTTOM_BORDER|CONVERT_TWIPS },
                    { SW_PROP_NMID(UNO_NAME_HEADER_BORDER_DISTANCE), FN_UNO_HEADER_BOX,    CPPU_E2T(CPPUTYPE_INT32),    PropertyAttribute::MAYBEVOID, BORDER_DISTANCE|CONVERT_TWIPS },
                    { SW_PROP_NMID(UNO_NAME_HEADER_LEFT_BORDER_DISTANCE), FN_UNO_HEADER_BOX,                CPPU_E2T(CPPUTYPE_INT32),   0, LEFT_BORDER_DISTANCE  |CONVERT_TWIPS },
                    { SW_PROP_NMID(UNO_NAME_HEADER_RIGHT_BORDER_DISTANCE), FN_UNO_HEADER_BOX,               CPPU_E2T(CPPUTYPE_INT32),   0, RIGHT_BORDER_DISTANCE |CONVERT_TWIPS },
                    { SW_PROP_NMID(UNO_NAME_HEADER_TOP_BORDER_DISTANCE), FN_UNO_HEADER_BOX,             CPPU_E2T(CPPUTYPE_INT32),   0, TOP_BORDER_DISTANCE   |CONVERT_TWIPS },
                    { SW_PROP_NMID(UNO_NAME_HEADER_BOTTOM_BORDER_DISTANCE), FN_UNO_HEADER_BOX,              CPPU_E2T(CPPUTYPE_INT32),   0, BOTTOM_BORDER_DISTANCE|CONVERT_TWIPS },
                    { SW_PROP_NMID(UNO_NAME_HEADER_SHADOW_FORMAT), FN_UNO_HEADER_SHADOW,        CPPU_E2T(CPPUTYPE_SHADOWFMT),   PROPERTY_NONE, CONVERT_TWIPS},
                    { SW_PROP_NMID(UNO_NAME_HEADER_BODY_DISTANCE), FN_UNO_HEADER_BODY_DISTANCE,CPPU_E2T(CPPUTYPE_INT32),            PROPERTY_NONE ,MID_LO_MARGIN|CONVERT_TWIPS       },
                    { SW_PROP_NMID(UNO_NAME_HEADER_IS_DYNAMIC_HEIGHT), FN_UNO_HEADER_IS_DYNAMIC_DISTANCE,CPPU_E2T(CPPUTYPE_BOOLEAN),            PROPERTY_NONE ,0         },
                    { SW_PROP_NMID(UNO_NAME_HEADER_IS_SHARED), FN_UNO_HEADER_SHARE_CONTENT,CPPU_E2T(CPPUTYPE_BOOLEAN),          PROPERTY_NONE ,0         },
                    { SW_PROP_NMID(UNO_NAME_HEADER_HEIGHT), FN_UNO_HEADER_HEIGHT,       CPPU_E2T(CPPUTYPE_INT32),           PROPERTY_NONE ,MID_SIZE_HEIGHT|CONVERT_TWIPS         },
                    { SW_PROP_NMID(UNO_NAME_HEADER_IS_ON), FN_UNO_HEADER_ON,            CPPU_E2T(CPPUTYPE_BOOLEAN),         PROPERTY_NONE ,0         },
                    { SW_PROP_NMID(UNO_NAME_HEADER_DYNAMIC_SPACING), FN_UNO_HEADER_EAT_SPACING,            CPPU_E2T(CPPUTYPE_BOOLEAN),         PropertyAttribute::MAYBEVOID ,0         },

                    { SW_PROP_NMID(UNO_NAME_FIRST_IS_SHARED), FN_UNO_FIRST_SHARE_CONTENT,CPPU_E2T(CPPUTYPE_BOOLEAN), PROPERTY_NONE, 0 },

                    { SW_PROP_NMID(UNO_NAME_FOOTER_BACK_COLOR), FN_UNO_FOOTER_BACKGROUND,   CPPU_E2T(CPPUTYPE_INT32),           PROPERTY_NONE ,MID_BACK_COLOR        },
                //  { SW_PROP_NMID(UNO_NAME_FOOTER_GRAPHIC), FN_UNO_FOOTER_BACKGROUND,      &,                              PROPERTY_NONE, MID_GRAPHIC
                    { SW_PROP_NMID(UNO_NAME_FOOTER_GRAPHIC_URL), FN_UNO_FOOTER_BACKGROUND,      CPPU_E2T(CPPUTYPE_OUSTRING), PROPERTY_NONE ,MID_GRAPHIC_URL    },
                    { SW_PROP_NMID(UNO_NAME_FOOTER_GRAPHIC_FILTER), FN_UNO_FOOTER_BACKGROUND,       CPPU_E2T(CPPUTYPE_OUSTRING), PROPERTY_NONE ,MID_GRAPHIC_FILTER    },
                    { SW_PROP_NMID(UNO_NAME_FOOTER_GRAPHIC_LOCATION), FN_UNO_FOOTER_BACKGROUND,     CPPU_E2T(CPPUTYPE_GRAPHICLOC), PROPERTY_NONE ,MID_GRAPHIC_POSITION},
                    { SW_PROP_NMID(UNO_NAME_FOOTER_LEFT_MARGIN), FN_UNO_FOOTER_LR_SPACE,    CPPU_E2T(CPPUTYPE_INT32), PROPERTY_NONE, MID_L_MARGIN|CONVERT_TWIPS},
                    { SW_PROP_NMID(UNO_NAME_FOOTER_RIGHT_MARGIN), FN_UNO_FOOTER_LR_SPACE,   CPPU_E2T(CPPUTYPE_INT32), PROPERTY_NONE, MID_R_MARGIN|CONVERT_TWIPS},
                    { SW_PROP_NMID(UNO_NAME_FOOTER_BACK_TRANSPARENT), FN_UNO_FOOTER_BACKGROUND,     CPPU_E2T(CPPUTYPE_BOOLEAN),         PROPERTY_NONE ,MID_GRAPHIC_TRANSPARENT       },
                    { SW_PROP_NMID(UNO_NAME_FOOTER_LEFT_BORDER), FN_UNO_FOOTER_BOX,             CPPU_E2T(CPPUTYPE_BORDERLINE),  0, LEFT_BORDER  |CONVERT_TWIPS },
                    { SW_PROP_NMID(UNO_NAME_FOOTER_RIGHT_BORDER), FN_UNO_FOOTER_BOX,                CPPU_E2T(CPPUTYPE_BORDERLINE),  0, RIGHT_BORDER |CONVERT_TWIPS },
                    { SW_PROP_NMID(UNO_NAME_FOOTER_TOP_BORDER), FN_UNO_FOOTER_BOX,              CPPU_E2T(CPPUTYPE_BORDERLINE),  0, TOP_BORDER   |CONVERT_TWIPS },
                    { SW_PROP_NMID(UNO_NAME_FOOTER_BOTTOM_BORDER), FN_UNO_FOOTER_BOX,               CPPU_E2T(CPPUTYPE_BORDERLINE),  0, BOTTOM_BORDER|CONVERT_TWIPS },
                    { SW_PROP_NMID(UNO_NAME_FOOTER_BORDER_DISTANCE), FN_UNO_FOOTER_BOX,    CPPU_E2T(CPPUTYPE_INT32),    PropertyAttribute::MAYBEVOID, BORDER_DISTANCE|CONVERT_TWIPS },
                    { SW_PROP_NMID(UNO_NAME_FOOTER_LEFT_BORDER_DISTANCE), FN_UNO_FOOTER_BOX,                CPPU_E2T(CPPUTYPE_INT32),   0, LEFT_BORDER_DISTANCE  |CONVERT_TWIPS },
                    { SW_PROP_NMID(UNO_NAME_FOOTER_RIGHT_BORDER_DISTANCE), FN_UNO_FOOTER_BOX,               CPPU_E2T(CPPUTYPE_INT32),   0, RIGHT_BORDER_DISTANCE |CONVERT_TWIPS },
                    { SW_PROP_NMID(UNO_NAME_FOOTER_TOP_BORDER_DISTANCE), FN_UNO_FOOTER_BOX,             CPPU_E2T(CPPUTYPE_INT32),   0, TOP_BORDER_DISTANCE   |CONVERT_TWIPS },
                    { SW_PROP_NMID(UNO_NAME_FOOTER_BOTTOM_BORDER_DISTANCE), FN_UNO_FOOTER_BOX,              CPPU_E2T(CPPUTYPE_INT32),   0, BOTTOM_BORDER_DISTANCE|CONVERT_TWIPS },
                    { SW_PROP_NMID(UNO_NAME_FOOTER_SHADOW_FORMAT), FN_UNO_FOOTER_SHADOW,        CPPU_E2T(CPPUTYPE_SHADOWFMT),   PROPERTY_NONE, CONVERT_TWIPS},
                    { SW_PROP_NMID(UNO_NAME_FOOTER_BODY_DISTANCE), FN_UNO_FOOTER_BODY_DISTANCE,CPPU_E2T(CPPUTYPE_INT32),            PROPERTY_NONE ,MID_UP_MARGIN|CONVERT_TWIPS       },
                    { SW_PROP_NMID(UNO_NAME_FOOTER_IS_DYNAMIC_HEIGHT), FN_UNO_FOOTER_IS_DYNAMIC_DISTANCE,CPPU_E2T(CPPUTYPE_BOOLEAN),            PROPERTY_NONE ,0         },
                    { SW_PROP_NMID(UNO_NAME_FOOTER_IS_SHARED), FN_UNO_FOOTER_SHARE_CONTENT,CPPU_E2T(CPPUTYPE_BOOLEAN),          PROPERTY_NONE ,0         },
                    { SW_PROP_NMID(UNO_NAME_FOOTER_HEIGHT), FN_UNO_FOOTER_HEIGHT,       CPPU_E2T(CPPUTYPE_INT32),           PROPERTY_NONE ,MID_SIZE_HEIGHT|CONVERT_TWIPS         },
                    { SW_PROP_NMID(UNO_NAME_FOOTER_IS_ON), FN_UNO_FOOTER_ON,            CPPU_E2T(CPPUTYPE_BOOLEAN),         PROPERTY_NONE ,0         },
                    { SW_PROP_NMID(UNO_NAME_FOOTER_DYNAMIC_SPACING), FN_UNO_FOOTER_EAT_SPACING,            CPPU_E2T(CPPUTYPE_BOOLEAN),         PropertyAttribute::MAYBEVOID ,0         },


                    { SW_PROP_NMID(UNO_NAME_IS_LANDSCAPE), SID_ATTR_PAGE,           CPPU_E2T(CPPUTYPE_BOOLEAN),             PROPERTY_NONE ,MID_PAGE_ORIENTATION   },
                    { SW_PROP_NMID(UNO_NAME_NUMBERING_TYPE), SID_ATTR_PAGE,             CPPU_E2T(CPPUTYPE_INT16),           PROPERTY_NONE , MID_PAGE_NUMTYPE       },
                    { SW_PROP_NMID(UNO_NAME_PAGE_STYLE_LAYOUT), SID_ATTR_PAGE,          CPPU_E2T(CPPUTYPE_PAGESTYLELAY),    PROPERTY_NONE ,MID_PAGE_LAYOUT     },
                    { SW_PROP_NMID(UNO_NAME_PRINTER_PAPER_TRAY), RES_PAPER_BIN,             CPPU_E2T(CPPUTYPE_OUSTRING),            PROPERTY_NONE , 0 },
//                  { SW_PROP_NMID(UNO_NAME_REGISTER_MODE_ACTIVE), SID_SWREGISTER_MODE,     CPPU_E2T(CPPUTYPE_BOOLEAN),             PROPERTY_NONE , 0 },
                    { SW_PROP_NMID(UNO_NAME_REGISTER_PARAGRAPH_STYLE), SID_SWREGISTER_COLLECTION,   CPPU_E2T(CPPUTYPE_OUSTRING),        PROPERTY_NONE , 0 },
                    { SW_PROP_NMID(UNO_NAME_SIZE), SID_ATTR_PAGE_SIZE,  CPPU_E2T(CPPUTYPE_AWTSIZE),             PROPERTY_NONE,   MID_SIZE_SIZE|CONVERT_TWIPS},
                    { SW_PROP_NMID(UNO_NAME_WIDTH), SID_ATTR_PAGE_SIZE,     CPPU_E2T(CPPUTYPE_INT32)  ,         PROPERTY_NONE, MID_SIZE_WIDTH|CONVERT_TWIPS},
                    { SW_PROP_NMID(UNO_NAME_HEIGHT), SID_ATTR_PAGE_SIZE,    CPPU_E2T(CPPUTYPE_INT32)  ,         PROPERTY_NONE, MID_SIZE_HEIGHT|CONVERT_TWIPS            },
                    { SW_PROP_NMID(UNO_NAME_TEXT_COLUMNS), RES_COL,                CPPU_E2T(CPPUTYPE_REFTEXTCOL),    PROPERTY_NONE, MID_COLUMNS},
                    { SW_PROP_NMID(UNO_NAME_TOP_MARGIN), RES_UL_SPACE,          CPPU_E2T(CPPUTYPE_INT32), PROPERTY_NONE, MID_UP_MARGIN|CONVERT_TWIPS},
                    { SW_PROP_NMID(UNO_NAME_BOTTOM_MARGIN), RES_UL_SPACE,           CPPU_E2T(CPPUTYPE_INT32), PROPERTY_NONE, MID_LO_MARGIN|CONVERT_TWIPS},
                    { SW_PROP_NMID(UNO_NAME_HEADER_TEXT), FN_UNO_HEADER,        CPPU_E2T(CPPUTYPE_REFTEXT),         PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY, 0},
                    { SW_PROP_NMID(UNO_NAME_HEADER_TEXT_LEFT), FN_UNO_HEADER_LEFT,     CPPU_E2T(CPPUTYPE_REFTEXT),          PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY, 0},
                    { SW_PROP_NMID(UNO_NAME_HEADER_TEXT_RIGHT), FN_UNO_HEADER_RIGHT,    CPPU_E2T(CPPUTYPE_REFTEXT),         PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY, 0},
                    { SW_PROP_NMID(UNO_NAME_HEADER_TEXT_FIRST), FN_UNO_HEADER_FIRST,    CPPU_E2T(CPPUTYPE_REFTEXT),         PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY, 0},
                    { SW_PROP_NMID(UNO_NAME_FOOTER_TEXT), FN_UNO_FOOTER,        CPPU_E2T(CPPUTYPE_REFTEXT),         PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY, 0},
                    { SW_PROP_NMID(UNO_NAME_FOOTER_TEXT_LEFT), FN_UNO_FOOTER_LEFT,  CPPU_E2T(CPPUTYPE_REFTEXT),         PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY, 0},
                    { SW_PROP_NMID(UNO_NAME_FOOTER_TEXT_RIGHT), FN_UNO_FOOTER_RIGHT,    CPPU_E2T(CPPUTYPE_REFTEXT),         PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY, 0},
                    { SW_PROP_NMID(UNO_NAME_FOOTER_TEXT_FIRST), FN_UNO_FOOTER_FIRST,    CPPU_E2T(CPPUTYPE_REFTEXT),         PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY, 0},
                    { SW_PROP_NMID(UNO_NAME_FOLLOW_STYLE), FN_UNO_FOLLOW_STYLE,     CPPU_E2T(CPPUTYPE_OUSTRING),        PROPERTY_NONE, 0},
                    { SW_PROP_NMID(UNO_NAME_USER_DEFINED_ATTRIBUTES), RES_UNKNOWNATR_CONTAINER, CPPU_E2T(CPPUTYPE_REFNAMECNT), PropertyAttribute::MAYBEVOID, 0 },
                    { SW_PROP_NMID(UNO_NAME_IS_PHYSICAL), FN_UNO_IS_PHYSICAL,     CPPU_E2T(CPPUTYPE_BOOLEAN), PropertyAttribute::READONLY, 0},
                    { SW_PROP_NMID(UNO_NAME_DISPLAY_NAME), FN_UNO_DISPLAY_NAME, CPPU_E2T(CPPUTYPE_OUSTRING), PropertyAttribute::READONLY, 0},
                    { SW_PROP_NMID(UNO_NAME_FOOTNOTE_HEIGHT), FN_PARAM_FTN_INFO,        CPPU_E2T(CPPUTYPE_INT32), PROPERTY_NONE ,       MID_FTN_HEIGHT|CONVERT_TWIPS},
                    { SW_PROP_NMID(UNO_NAME_FOOTNOTE_LINE_WEIGHT), FN_PARAM_FTN_INFO,       CPPU_E2T(CPPUTYPE_INT16),   PROPERTY_NONE ,     MID_LINE_WEIGHT|CONVERT_TWIPS},
                    { SW_PROP_NMID(UNO_NAME_FOOTNOTE_LINE_COLOR), FN_PARAM_FTN_INFO,        CPPU_E2T(CPPUTYPE_INT32),   PROPERTY_NONE ,     MID_LINE_COLOR},
                    { SW_PROP_NMID(UNO_NAME_FOOTNOTE_LINE_STYLE), FN_PARAM_FTN_INFO,        CPPU_E2T(CPPUTYPE_INT8),    PROPERTY_NONE , MID_FTN_LINE_STYLE},
                    { SW_PROP_NMID(UNO_NAME_FOOTNOTE_LINE_RELATIVE_WIDTH), FN_PARAM_FTN_INFO,       CPPU_E2T(CPPUTYPE_INT8),        PROPERTY_NONE ,     MID_LINE_RELWIDTH    },
                    { SW_PROP_NMID(UNO_NAME_FOOTNOTE_LINE_ADJUST), FN_PARAM_FTN_INFO,       CPPU_E2T(CPPUTYPE_INT16),   PROPERTY_NONE ,     MID_LINE_ADJUST     },
                    { SW_PROP_NMID(UNO_NAME_FOOTNOTE_LINE_TEXT_DISTANCE), FN_PARAM_FTN_INFO,        CPPU_E2T(CPPUTYPE_INT32),   PROPERTY_NONE ,     MID_LINE_TEXT_DIST   |CONVERT_TWIPS },
                    { SW_PROP_NMID(UNO_NAME_FOOTNOTE_LINE_DISTANCE), FN_PARAM_FTN_INFO,         CPPU_E2T(CPPUTYPE_INT32),   PROPERTY_NONE ,     MID_LINE_FOOTNOTE_DIST|CONVERT_TWIPS},
                    { SW_PROP_NMID(UNO_NAME_WRITING_MODE), RES_FRAMEDIR, CPPU_E2T(CPPUTYPE_INT16), PROPERTY_NONE, 0 },
                        // writing grid
                    { SW_PROP_NMID(UNO_NAME_GRID_COLOR), RES_TEXTGRID, CPPU_E2T(CPPUTYPE_INT32), PROPERTY_NONE, MID_GRID_COLOR},
                    { SW_PROP_NMID(UNO_NAME_GRID_LINES), RES_TEXTGRID, CPPU_E2T(CPPUTYPE_INT16), PROPERTY_NONE, MID_GRID_LINES},
                    { SW_PROP_NMID(UNO_NAME_GRID_BASE_HEIGHT), RES_TEXTGRID, CPPU_E2T(CPPUTYPE_INT32), PROPERTY_NONE, MID_GRID_BASEHEIGHT|CONVERT_TWIPS},
                    { SW_PROP_NMID(UNO_NAME_GRID_RUBY_HEIGHT), RES_TEXTGRID, CPPU_E2T(CPPUTYPE_INT32), PROPERTY_NONE, MID_GRID_RUBYHEIGHT|CONVERT_TWIPS},
                    { SW_PROP_NMID(UNO_NAME_GRID_MODE), RES_TEXTGRID, CPPU_E2T(CPPUTYPE_INT16), PROPERTY_NONE, MID_GRID_TYPE},
                    { SW_PROP_NMID(UNO_NAME_GRID_RUBY_BELOW), RES_TEXTGRID, CPPU_E2T(CPPUTYPE_BOOLEAN), PROPERTY_NONE, MID_GRID_RUBY_BELOW},
                    { SW_PROP_NMID(UNO_NAME_GRID_PRINT), RES_TEXTGRID, CPPU_E2T(CPPUTYPE_BOOLEAN), PROPERTY_NONE, MID_GRID_PRINT},
                    { SW_PROP_NMID(UNO_NAME_GRID_DISPLAY), RES_TEXTGRID, CPPU_E2T(CPPUTYPE_BOOLEAN), PROPERTY_NONE, MID_GRID_DISPLAY},
                    { SW_PROP_NMID(UNO_NAME_GRID_BASE_WIDTH), RES_TEXTGRID, CPPU_E2T(CPPUTYPE_INT32), PROPERTY_NONE, MID_GRID_BASEWIDTH|CONVERT_TWIPS},
                    { SW_PROP_NMID(UNO_NAME_GRID_SNAP_TO_CHARS), RES_TEXTGRID, CPPU_E2T(CPPUTYPE_BOOLEAN), PROPERTY_NONE, MID_GRID_SNAPTOCHARS},
                    { SW_PROP_NMID(UNO_NAME_GRID_STANDARD_PAGE_MODE), RES_TEXTGRID, CPPU_E2T(CPPUTYPE_BOOLEAN), PROPERTY_NONE, MID_GRID_STANDARD_MODE},
                    { SW_PROP_NMID(UNO_NAME_HIDDEN), FN_UNO_HIDDEN,     CPPU_E2T(CPPUTYPE_BOOLEAN), PROPERTY_NONE, 0},
                    {0,0,0,0,0,0}
                };
                aMapEntriesArr[nPropertyId] = aPageStyleMap;
            }
            break;
            case PROPERTY_MAP_NUM_STYLE  :
            {
                static SfxItemPropertyMapEntry aNumStyleMap        [] =
                {
                    { SW_PROP_NMID(UNO_NAME_NUMBERING_RULES), FN_UNO_NUM_RULES, CPPU_E2T(CPPUTYPE_REFIDXREPL), PROPERTY_NONE, CONVERT_TWIPS},
                    { SW_PROP_NMID(UNO_NAME_IS_PHYSICAL), FN_UNO_IS_PHYSICAL,     CPPU_E2T(CPPUTYPE_BOOLEAN), PropertyAttribute::READONLY, 0},
                    { SW_PROP_NMID(UNO_NAME_DISPLAY_NAME), FN_UNO_DISPLAY_NAME, CPPU_E2T(CPPUTYPE_OUSTRING), PropertyAttribute::READONLY, 0},
                    { SW_PROP_NMID(UNO_NAME_HIDDEN), FN_UNO_HIDDEN,     CPPU_E2T(CPPUTYPE_BOOLEAN), PROPERTY_NONE, 0},
                    {0,0,0,0,0,0}
                };
                aMapEntriesArr[nPropertyId] = aNumStyleMap;
            }
            break;
            case PROPERTY_MAP_TEXT_TABLE :
            {
                static SfxItemPropertyMapEntry aTablePropertyMap_Impl[] =
                {
                    { SW_PROP_NMID(UNO_NAME_BACK_COLOR), RES_BACKGROUND,        CPPU_E2T(CPPUTYPE_INT32),           PROPERTY_NONE,MID_BACK_COLOR         },
                    { SW_PROP_NMID(UNO_NAME_BREAK_TYPE), RES_BREAK,                 CPPU_E2T(CPPUTYPE_BREAK),       PROPERTY_NONE, 0},
                    { SW_PROP_NMID(UNO_NAME_BACK_GRAPHIC_URL), RES_BACKGROUND,      CPPU_E2T(CPPUTYPE_OUSTRING), PROPERTY_NONE ,MID_GRAPHIC_URL    },
                    { SW_PROP_NMID(UNO_NAME_BACK_GRAPHIC_FILTER), RES_BACKGROUND,       CPPU_E2T(CPPUTYPE_OUSTRING), PROPERTY_NONE ,MID_GRAPHIC_FILTER    },
                    { SW_PROP_NMID(UNO_NAME_BACK_GRAPHIC_LOCATION), RES_BACKGROUND,         CPPU_E2T(CPPUTYPE_GRAPHICLOC), PROPERTY_NONE ,MID_GRAPHIC_POSITION},
                    { SW_PROP_NMID(UNO_NAME_LEFT_MARGIN), RES_LR_SPACE,             CPPU_E2T(CPPUTYPE_INT32), PROPERTY_NONE, MID_L_MARGIN|CONVERT_TWIPS},
                    { SW_PROP_NMID(UNO_NAME_RIGHT_MARGIN), RES_LR_SPACE,            CPPU_E2T(CPPUTYPE_INT32), PROPERTY_NONE, MID_R_MARGIN|CONVERT_TWIPS},
                    { SW_PROP_NMID(UNO_NAME_HORI_ORIENT), RES_HORI_ORIENT,      CPPU_E2T(CPPUTYPE_INT16),           PROPERTY_NONE ,MID_HORIORIENT_ORIENT    },
                    { SW_PROP_NMID(UNO_NAME_KEEP_TOGETHER), RES_KEEP,               CPPU_E2T(CPPUTYPE_BOOLEAN)  ,       PROPERTY_NONE, 0},
                    { SW_PROP_NMID(UNO_NAME_SPLIT), RES_LAYOUT_SPLIT,       CPPU_E2T(CPPUTYPE_BOOLEAN)  ,       PROPERTY_NONE, 0},
                    { SW_PROP_NMID(UNO_NAME_PAGE_NUMBER_OFFSET), RES_PAGEDESC,              CPPU_E2T(CPPUTYPE_INT16),       PropertyAttribute::MAYBEVOID, MID_PAGEDESC_PAGENUMOFFSET},
                    { SW_PROP_NMID(UNO_NAME_PAGE_DESC_NAME), RES_PAGEDESC,           CPPU_E2T(CPPUTYPE_OUSTRING),         PropertyAttribute::MAYBEVOID, 0xff},
                    { SW_PROP_NMID(UNO_NAME_RELATIVE_WIDTH), FN_TABLE_RELATIVE_WIDTH,CPPU_E2T(CPPUTYPE_INT16)  ,        PROPERTY_NONE, 0xff },
                    { SW_PROP_NMID(UNO_NAME_REPEAT_HEADLINE), FN_TABLE_HEADLINE_REPEAT,CPPU_E2T(CPPUTYPE_BOOLEAN),      PROPERTY_NONE, 0xff},
                    { SW_PROP_NMID(UNO_NAME_HEADER_ROW_COUNT), FN_TABLE_HEADLINE_COUNT,  CPPU_E2T(CPPUTYPE_INT32),      PROPERTY_NONE, 0xff},
                    { SW_PROP_NMID(UNO_NAME_SHADOW_FORMAT), RES_SHADOW,             CPPU_E2T(CPPUTYPE_SHADOWFMT),   PROPERTY_NONE, 0},
                    { SW_PROP_NMID(UNO_NAME_TOP_MARGIN), RES_UL_SPACE,          CPPU_E2T(CPPUTYPE_INT32), PROPERTY_NONE, MID_UP_MARGIN|CONVERT_TWIPS},
                    { SW_PROP_NMID(UNO_NAME_BOTTOM_MARGIN), RES_UL_SPACE,           CPPU_E2T(CPPUTYPE_INT32), PROPERTY_NONE, MID_LO_MARGIN|CONVERT_TWIPS},
                    { SW_PROP_NMID(UNO_NAME_BACK_TRANSPARENT), RES_BACKGROUND,  CPPU_E2T(CPPUTYPE_BOOLEAN),         PROPERTY_NONE ,MID_GRAPHIC_TRANSPARENT       },
                    { SW_PROP_NMID(UNO_NAME_WIDTH), FN_TABLE_WIDTH,         CPPU_E2T(CPPUTYPE_INT32)  ,         PROPERTY_NONE, 0xff},
                    { SW_PROP_NMID(UNO_NAME_IS_WIDTH_RELATIVE), FN_TABLE_IS_RELATIVE_WIDTH,         CPPU_E2T(CPPUTYPE_BOOLEAN)  ,       PROPERTY_NONE, 0xff},
                    { SW_PROP_NMID(UNO_NAME_CHART_ROW_AS_LABEL), FN_UNO_RANGE_ROW_LABEL,            CPPU_E2T(CPPUTYPE_BOOLEAN),         PROPERTY_NONE,  0},
                    { SW_PROP_NMID(UNO_NAME_CHART_COLUMN_AS_LABEL), FN_UNO_RANGE_COL_LABEL,         CPPU_E2T(CPPUTYPE_BOOLEAN)  ,       PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_TABLE_BORDER), FN_UNO_TABLE_BORDER,         CPPU_E2T(CPPUTYPE_TABLEBORDER), PropertyAttribute::MAYBEVOID, CONVERT_TWIPS },
                    { SW_PROP_NMID(UNO_NAME_TABLE_BORDER2), FN_UNO_TABLE_BORDER2,         CPPU_E2T(CPPUTYPE_TABLEBORDER2), PropertyAttribute::MAYBEVOID, CONVERT_TWIPS },
                    { SW_PROP_NMID(UNO_NAME_TABLE_BORDER_DISTANCES), FN_UNO_TABLE_BORDER_DISTANCES,         CPPU_E2T(CPPUTYPE_TABLEBORDERDISTANCES), PropertyAttribute::MAYBEVOID, CONVERT_TWIPS },
                    { SW_PROP_NMID(UNO_NAME_TABLE_COLUMN_SEPARATORS), FN_UNO_TABLE_COLUMN_SEPARATORS,   CPPU_E2T(CPPUTYPE_TBLCOLSEP),   PropertyAttribute::MAYBEVOID, 0 },
                    { SW_PROP_NMID(UNO_NAME_TABLE_COLUMN_RELATIVE_SUM), FN_UNO_TABLE_COLUMN_RELATIVE_SUM,       CPPU_E2T(CPPUTYPE_INT16),       PropertyAttribute::READONLY, 0 },
                    COMMON_TEXT_CONTENT_PROPERTIES
                    { SW_PROP_NMID(UNO_LINK_DISPLAY_NAME), FN_PARAM_LINK_DISPLAY_NAME,  CPPU_E2T(CPPUTYPE_OUSTRING), PropertyAttribute::READONLY, 0xff},
                    { SW_PROP_NMID(UNO_NAME_USER_DEFINED_ATTRIBUTES), RES_UNKNOWNATR_CONTAINER, CPPU_E2T(CPPUTYPE_REFNAMECNT), PropertyAttribute::MAYBEVOID, 0 },
                    { SW_PROP_NMID(UNO_NAME_TEXT_SECTION), FN_UNO_TEXT_SECTION, CPPU_E2T(CPPUTYPE_REFTEXTSECTION),  PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY ,0 },
                    { SW_PROP_NMID(UNO_NAME_WRITING_MODE), RES_FRAMEDIR, CPPU_E2T(CPPUTYPE_INT16), PROPERTY_NONE, 0 },
                    { SW_PROP_NMID(UNO_NAME_TABLE_NAME),   FN_UNO_TABLE_NAME,   CPPU_E2T(CPPUTYPE_OUSTRING),        PROPERTY_NONE, 0 },
                     { SW_PROP_NMID(UNO_NAME_PAGE_STYLE_NAME), RES_PAGEDESC, CPPU_E2T(CPPUTYPE_OUSTRING),         PROPERTY_NONE, 0},
                    // #i29550#
                    { SW_PROP_NMID(UNO_NAME_COLLAPSING_BORDERS), RES_COLLAPSING_BORDERS, CPPU_E2T(CPPUTYPE_BOOLEAN), PROPERTY_NONE, 0},
                    _REDLINE_NODE_PROPERTIES
                    {0,0,0,0,0,0}
                };

                aMapEntriesArr[nPropertyId] = aTablePropertyMap_Impl;
            }
            break;
            case PROPERTY_MAP_TABLE_CELL :
            {
                static SfxItemPropertyMapEntry aCellMap_Impl[] =
                {
                    { SW_PROP_NMID(UNO_NAME_BACK_COLOR), RES_BACKGROUND,    CPPU_E2T(CPPUTYPE_INT32),           PROPERTY_NONE , MID_BACK_COLOR       },
                    { SW_PROP_NMID(UNO_NAME_BACK_GRAPHIC_URL), RES_BACKGROUND,      CPPU_E2T(CPPUTYPE_OUSTRING), PROPERTY_NONE ,MID_GRAPHIC_URL    },
                    { SW_PROP_NMID(UNO_NAME_BACK_GRAPHIC_FILTER), RES_BACKGROUND,       CPPU_E2T(CPPUTYPE_OUSTRING), PROPERTY_NONE ,MID_GRAPHIC_FILTER    },
                    { SW_PROP_NMID(UNO_NAME_BACK_GRAPHIC_LOCATION), RES_BACKGROUND,         CPPU_E2T(CPPUTYPE_GRAPHICLOC), PROPERTY_NONE ,MID_GRAPHIC_POSITION},
                    { SW_PROP_NMID(UNO_NAME_BACK_TRANSPARENT), RES_BACKGROUND,  CPPU_E2T(CPPUTYPE_BOOLEAN),         PROPERTY_NONE , MID_GRAPHIC_TRANSPARENT      },
                    { SW_PROP_NMID(UNO_NAME_NUMBER_FORMAT), RES_BOXATR_FORMAT,  CPPU_E2T(CPPUTYPE_INT32),           PropertyAttribute::MAYBEVOID ,0             },
                    { SW_PROP_NMID(UNO_NAME_LEFT_BORDER), RES_BOX,    CPPU_E2T(CPPUTYPE_BORDERLINE),    0, LEFT_BORDER  |CONVERT_TWIPS },
                    { SW_PROP_NMID(UNO_NAME_RIGHT_BORDER), RES_BOX,    CPPU_E2T(CPPUTYPE_BORDERLINE),   0, RIGHT_BORDER |CONVERT_TWIPS },
                    { SW_PROP_NMID(UNO_NAME_TOP_BORDER), RES_BOX,    CPPU_E2T(CPPUTYPE_BORDERLINE), 0, TOP_BORDER   |CONVERT_TWIPS },
                    { SW_PROP_NMID(UNO_NAME_BOTTOM_BORDER), RES_BOX,    CPPU_E2T(CPPUTYPE_BORDERLINE),  0, BOTTOM_BORDER|CONVERT_TWIPS },
                    { SW_PROP_NMID(UNO_NAME_BORDER_DISTANCE), RES_BOX,    CPPU_E2T(CPPUTYPE_INT32), 0, BORDER_DISTANCE|CONVERT_TWIPS },
                    { SW_PROP_NMID(UNO_NAME_LEFT_BORDER_DISTANCE), RES_BOX,             CPPU_E2T(CPPUTYPE_INT32),   0, LEFT_BORDER_DISTANCE  |CONVERT_TWIPS },
                    { SW_PROP_NMID(UNO_NAME_RIGHT_BORDER_DISTANCE), RES_BOX,                CPPU_E2T(CPPUTYPE_INT32),   0, RIGHT_BORDER_DISTANCE |CONVERT_TWIPS },
                    { SW_PROP_NMID(UNO_NAME_TOP_BORDER_DISTANCE), RES_BOX,              CPPU_E2T(CPPUTYPE_INT32),   0, TOP_BORDER_DISTANCE   |CONVERT_TWIPS },
                    { SW_PROP_NMID(UNO_NAME_BOTTOM_BORDER_DISTANCE), RES_BOX,               CPPU_E2T(CPPUTYPE_INT32),   0, BOTTOM_BORDER_DISTANCE|CONVERT_TWIPS },
                    { SW_PROP_NMID(UNO_NAME_USER_DEFINED_ATTRIBUTES), RES_UNKNOWNATR_CONTAINER, CPPU_E2T(CPPUTYPE_REFNAMECNT), PropertyAttribute::MAYBEVOID, 0 },
                    { SW_PROP_NMID(UNO_NAME_TEXT_SECTION), FN_UNO_TEXT_SECTION, CPPU_E2T(CPPUTYPE_REFTEXTSECTION),  PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY ,0 },
                    { SW_PROP_NMID(UNO_NAME_IS_PROTECTED), RES_PROTECT,            CPPU_E2T(CPPUTYPE_BOOLEAN), 0, MID_PROTECT_CONTENT},
                    { SW_PROP_NMID(UNO_NAME_CELL_NAME), FN_UNO_CELL_NAME,            CPPU_E2T(CPPUTYPE_OUSTRING), PropertyAttribute::READONLY,0},
                    { SW_PROP_NMID(UNO_NAME_VERT_ORIENT), RES_VERT_ORIENT,      CPPU_E2T(CPPUTYPE_INT16),           PROPERTY_NONE ,MID_VERTORIENT_ORIENT    },
                    { SW_PROP_NMID(UNO_NAME_WRITING_MODE), RES_FRAMEDIR, CPPU_E2T(CPPUTYPE_INT16), PROPERTY_NONE, 0 },
                    { SW_PROP_NMID(UNO_NAME_ROW_SPAN),     FN_UNO_CELL_ROW_SPAN, CPPU_E2T(CPPUTYPE_INT32),  0, 0 },
                    _REDLINE_NODE_PROPERTIES
                    {0,0,0,0,0,0}
                };
                aMapEntriesArr[nPropertyId] = aCellMap_Impl;
            }
            break;
            case PROPERTY_MAP_TABLE_RANGE:
            {
                static SfxItemPropertyMapEntry aRangePropertyMap_Impl[] =
                {
                    COMMON_CRSR_PARA_PROPERTIES_WITHOUT_FN_01
                    TABSTOPS_MAP_ENTRY
                    { SW_PROP_NMID(UNO_NAME_BACK_COLOR), FN_UNO_TABLE_CELL_BACKGROUND,  CPPU_E2T(CPPUTYPE_INT32),   PropertyAttribute::MAYBEVOID, MID_BACK_COLOR  },
                    { SW_PROP_NMID(UNO_NAME_BACK_GRAPHIC_URL), RES_BACKGROUND,      CPPU_E2T(CPPUTYPE_OUSTRING), PropertyAttribute::MAYBEVOID ,MID_GRAPHIC_URL    },
                    { SW_PROP_NMID(UNO_NAME_BACK_GRAPHIC_FILTER), RES_BACKGROUND,       CPPU_E2T(CPPUTYPE_OUSTRING), PropertyAttribute::MAYBEVOID ,MID_GRAPHIC_FILTER    },
                    { SW_PROP_NMID(UNO_NAME_BACK_GRAPHIC_LOCATION), FN_UNO_TABLE_CELL_BACKGROUND,   CPPU_E2T(CPPUTYPE_GRAPHICLOC), PropertyAttribute::MAYBEVOID, MID_GRAPHIC_POSITION},
                    { SW_PROP_NMID(UNO_NAME_BACK_TRANSPARENT), FN_UNO_TABLE_CELL_BACKGROUND,    CPPU_E2T(CPPUTYPE_BOOLEAN), PropertyAttribute::MAYBEVOID, MID_GRAPHIC_TRANSPARENT      },
                    { SW_PROP_NMID(UNO_NAME_NUMBER_FORMAT), RES_BOXATR_FORMAT,         CPPU_E2T(CPPUTYPE_INT32),           PropertyAttribute::MAYBEVOID ,0             },
                    { SW_PROP_NMID(UNO_NAME_VERT_ORIENT), RES_VERT_ORIENT,      CPPU_E2T(CPPUTYPE_INT16),           PROPERTY_NONE ,MID_VERTORIENT_ORIENT    },
                    { SW_PROP_NMID(UNO_NAME_CHART_ROW_AS_LABEL), FN_UNO_RANGE_ROW_LABEL,    CPPU_E2T(CPPUTYPE_BOOLEAN),         PropertyAttribute::MAYBEVOID,  0},
                    { SW_PROP_NMID(UNO_NAME_CHART_COLUMN_AS_LABEL), FN_UNO_RANGE_COL_LABEL, CPPU_E2T(CPPUTYPE_BOOLEAN)  ,       PropertyAttribute::MAYBEVOID,     0},

                    {0,0,0,0,0,0}
                };
                aMapEntriesArr[nPropertyId] = aRangePropertyMap_Impl;
            }
            break;
            case PROPERTY_MAP_SECTION:
            {
                static SfxItemPropertyMapEntry aSectionPropertyMap_Impl[] =
                {
                    { SW_PROP_NMID(UNO_NAME_CONDITION), WID_SECT_CONDITION, CPPU_E2T(CPPUTYPE_OUSTRING)  ,      PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_DDE_COMMAND_FILE), WID_SECT_DDE_TYPE, CPPU_E2T(CPPUTYPE_OUSTRING)  ,        PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_DDE_COMMAND_TYPE), WID_SECT_DDE_FILE, CPPU_E2T(CPPUTYPE_OUSTRING)  ,        PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_DDE_COMMAND_ELEMENT), WID_SECT_DDE_ELEMENT, CPPU_E2T(CPPUTYPE_OUSTRING)  ,      PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_IS_AUTOMATIC_UPDATE), WID_SECT_DDE_AUTOUPDATE, CPPU_E2T(CPPUTYPE_BOOLEAN),              PROPERTY_NONE,  0},
                    { SW_PROP_NMID(UNO_NAME_FILE_LINK), WID_SECT_LINK     , CPPU_E2T(CPPUTYPE_SECTFILELNK),     PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_IS_VISIBLE), WID_SECT_VISIBLE   , CPPU_E2T(CPPUTYPE_BOOLEAN),           PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_IS_PROTECTED), WID_SECT_PROTECTED, CPPU_E2T(CPPUTYPE_BOOLEAN),              PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_EDIT_IN_READONLY), WID_SECT_EDIT_IN_READONLY,    CPPU_E2T(CPPUTYPE_BOOLEAN),         PROPERTY_NONE, 0},
                    { SW_PROP_NMID(UNO_NAME_LINK_REGION), WID_SECT_REGION   , CPPU_E2T(CPPUTYPE_OUSTRING)  ,        PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_TEXT_COLUMNS), RES_COL,                CPPU_E2T(CPPUTYPE_REFTEXTCOL),    PROPERTY_NONE, MID_COLUMNS},
                    { SW_PROP_NMID(UNO_NAME_BACK_GRAPHIC_URL), RES_BACKGROUND,      CPPU_E2T(CPPUTYPE_OUSTRING), PROPERTY_NONE ,MID_GRAPHIC_URL    },
                    { SW_PROP_NMID(UNO_NAME_BACK_GRAPHIC_FILTER), RES_BACKGROUND,       CPPU_E2T(CPPUTYPE_OUSTRING), PROPERTY_NONE ,MID_GRAPHIC_FILTER    },
                    { SW_PROP_NMID(UNO_NAME_BACK_GRAPHIC_LOCATION), RES_BACKGROUND,         CPPU_E2T(CPPUTYPE_GRAPHICLOC),          PROPERTY_NONE ,MID_GRAPHIC_POSITION},
                    { SW_PROP_NMID(UNO_NAME_BACK_COLOR), RES_BACKGROUND,            CPPU_E2T(CPPUTYPE_INT32),           PROPERTY_NONE ,MID_BACK_COLOR        },
                    { SW_PROP_NMID(UNO_NAME_BACK_TRANSPARENT), RES_BACKGROUND,      CPPU_E2T(CPPUTYPE_BOOLEAN),         PROPERTY_NONE ,MID_GRAPHIC_TRANSPARENT       },
                    { SW_PROP_NMID(UNO_LINK_DISPLAY_NAME), FN_PARAM_LINK_DISPLAY_NAME,  CPPU_E2T(CPPUTYPE_OUSTRING), PropertyAttribute::READONLY, 0xff},
                    { SW_PROP_NMID(UNO_NAME_USER_DEFINED_ATTRIBUTES), RES_UNKNOWNATR_CONTAINER, CPPU_E2T(CPPUTYPE_REFNAMECNT), PropertyAttribute::MAYBEVOID, 0 },
                    { SW_PROP_NMID(UNO_NAME_FOOTNOTE_IS_COLLECT_AT_TEXT_END), RES_FTN_AT_TXTEND,        CPPU_E2T(CPPUTYPE_BOOLEAN),                PROPERTY_NONE ,MID_COLLECT                   },
                    { SW_PROP_NMID(UNO_NAME_FOOTNOTE_IS_RESTART_NUMBERING), RES_FTN_AT_TXTEND,      CPPU_E2T(CPPUTYPE_BOOLEAN),                PROPERTY_NONE , MID_RESTART_NUM },
                    { SW_PROP_NMID(UNO_NAME_FOOTNOTE_RESTART_NUMBERING_AT), RES_FTN_AT_TXTEND,      CPPU_E2T(CPPUTYPE_INT16),               PROPERTY_NONE , MID_NUM_START_AT},
                    { SW_PROP_NMID(UNO_NAME_FOOTNOTE_IS_OWN_NUMBERING), RES_FTN_AT_TXTEND,  CPPU_E2T(CPPUTYPE_BOOLEAN),                                                 PROPERTY_NONE ,  MID_OWN_NUM     },
                    { SW_PROP_NMID(UNO_NAME_FOOTNOTE_NUMBERING_TYPE), RES_FTN_AT_TXTEND,        CPPU_E2T(CPPUTYPE_INT16),               PROPERTY_NONE , MID_NUM_TYPE    },
                    { SW_PROP_NMID(UNO_NAME_FOOTNOTE_NUMBERING_PREFIX), RES_FTN_AT_TXTEND,  CPPU_E2T(CPPUTYPE_OUSTRING)  ,      PROPERTY_NONE,      MID_PREFIX      },
                    { SW_PROP_NMID(UNO_NAME_FOOTNOTE_NUMBERING_SUFFIX), RES_FTN_AT_TXTEND,  CPPU_E2T(CPPUTYPE_OUSTRING)  ,      PROPERTY_NONE,      MID_SUFFIX      },
                    { SW_PROP_NMID(UNO_NAME_ENDNOTE_IS_COLLECT_AT_TEXT_END), RES_END_AT_TXTEND,        CPPU_E2T(CPPUTYPE_BOOLEAN),                 PROPERTY_NONE , MID_COLLECT                      },
                    { SW_PROP_NMID(UNO_NAME_ENDNOTE_IS_RESTART_NUMBERING), RES_END_AT_TXTEND,        CPPU_E2T(CPPUTYPE_BOOLEAN),                   PROPERTY_NONE , MID_RESTART_NUM  },
                    { SW_PROP_NMID(UNO_NAME_ENDNOTE_RESTART_NUMBERING_AT), RES_END_AT_TXTEND,     CPPU_E2T(CPPUTYPE_INT16),             PROPERTY_NONE ,  MID_NUM_START_AT },
                    { SW_PROP_NMID(UNO_NAME_ENDNOTE_IS_OWN_NUMBERING), RES_END_AT_TXTEND,     CPPU_E2T(CPPUTYPE_BOOLEAN),                                                   PROPERTY_NONE ,  MID_OWN_NUM      },
                    { SW_PROP_NMID(UNO_NAME_ENDNOTE_NUMBERING_TYPE), RES_END_AT_TXTEND,       CPPU_E2T(CPPUTYPE_INT16),             PROPERTY_NONE ,MID_NUM_TYPE     },
                    { SW_PROP_NMID(UNO_NAME_ENDNOTE_NUMBERING_PREFIX), RES_END_AT_TXTEND,     CPPU_E2T(CPPUTYPE_OUSTRING)  ,        PROPERTY_NONE,   MID_PREFIX       },
                    { SW_PROP_NMID(UNO_NAME_ENDNOTE_NUMBERING_SUFFIX), RES_END_AT_TXTEND,     CPPU_E2T(CPPUTYPE_OUSTRING)  ,        PROPERTY_NONE,   MID_SUFFIX       },
                    { SW_PROP_NMID(UNO_NAME_DOCUMENT_INDEX), WID_SECT_DOCUMENT_INDEX, CPPU_E2T(CPPUTYPE_REFDOCINDEX), PropertyAttribute::READONLY | PropertyAttribute::MAYBEVOID, 0 },
                    { SW_PROP_NMID(UNO_NAME_IS_GLOBAL_DOCUMENT_SECTION), WID_SECT_IS_GLOBAL_DOC_SECTION, CPPU_E2T(CPPUTYPE_BOOLEAN), PropertyAttribute::READONLY, 0 },
                    { SW_PROP_NMID(UNO_NAME_PROTECTION_KEY), WID_SECT_PASSWORD,      CPPU_E2T(CPPUTYPE_SEQINT8), PROPERTY_NONE, 0 },
                    { SW_PROP_NMID(UNO_NAME_DONT_BALANCE_TEXT_COLUMNS), RES_COLUMNBALANCE, CPPU_E2T(CPPUTYPE_BOOLEAN), PROPERTY_NONE, 0 },
                    COMMON_TEXT_CONTENT_PROPERTIES
                    _REDLINE_NODE_PROPERTIES
                    { SW_PROP_NMID(UNO_NAME_IS_CURRENTLY_VISIBLE), WID_SECT_CURRENTLY_VISIBLE, CPPU_E2T(CPPUTYPE_BOOLEAN),          PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_WRITING_MODE), RES_FRAMEDIR, CPPU_E2T(CPPUTYPE_INT16), PROPERTY_NONE, 0 },
                    { SW_PROP_NMID(UNO_NAME_SECT_LEFT_MARGIN), RES_LR_SPACE,            CPPU_E2T(CPPUTYPE_INT32),           PropertyAttribute::MAYBEVOID, MID_L_MARGIN|CONVERT_TWIPS},                                   \
                    { SW_PROP_NMID(UNO_NAME_SECT_RIGHT_MARGIN), RES_LR_SPACE,           CPPU_E2T(CPPUTYPE_INT32),           PropertyAttribute::MAYBEVOID, MID_R_MARGIN|CONVERT_TWIPS},                                  \
                    {0,0,0,0,0,0}
                };
                aMapEntriesArr[nPropertyId] = aSectionPropertyMap_Impl;
            }
            break;
            case PROPERTY_MAP_TEXT_SEARCH:
            {
                static SfxItemPropertyMapEntry aSearchPropertyMap_Impl[] =
                {
                    { SW_PROP_NMID(UNO_NAME_SEARCH_ALL), WID_SEARCH_ALL,        CPPU_E2T(CPPUTYPE_BOOLEAN)  ,       PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_SEARCH_BACKWARDS), WID_BACKWARDS,           CPPU_E2T(CPPUTYPE_BOOLEAN)  ,       PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_SEARCH_CASE_SENSITIVE), WID_CASE_SENSITIVE,     CPPU_E2T(CPPUTYPE_BOOLEAN)  ,       PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_SEARCH_REGULAR_EXPRESSION), WID_REGULAR_EXPRESSION, CPPU_E2T(CPPUTYPE_BOOLEAN)  ,       PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_SEARCH_SIMILARITY), WID_SIMILARITY,         CPPU_E2T(CPPUTYPE_BOOLEAN)  ,       PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_SEARCH_SIMILARITY_ADD), WID_SIMILARITY_ADD,     CPPU_E2T(CPPUTYPE_INT16)  ,     PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_SEARCH_SIMILARITY_EXCHANGE), WID_SIMILARITY_EXCHANGE,CPPU_E2T(CPPUTYPE_INT16)  ,    PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_SEARCH_SIMILARITY_RELAX), WID_SIMILARITY_RELAX,     CPPU_E2T(CPPUTYPE_BOOLEAN)  ,       PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_SEARCH_SIMILARITY_REMOVE), WID_SIMILARITY_REMOVE,   CPPU_E2T(CPPUTYPE_INT16)  ,     PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_SEARCH_STYLES), WID_STYLES,             CPPU_E2T(CPPUTYPE_BOOLEAN)  ,       PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_SEARCH_WORDS), WID_WORDS,               CPPU_E2T(CPPUTYPE_BOOLEAN)  ,       PROPERTY_NONE,     0},
                    {0,0,0,0,0,0}
                };
                aMapEntriesArr[nPropertyId] = aSearchPropertyMap_Impl;
            }
            break;
            case PROPERTY_MAP_TEXT_FRAME:
            {
                static SfxItemPropertyMapEntry aFramePropertyMap_Impl[] =
                {
                    COMMON_FRAME_PROPERTIES
                    _REDLINE_NODE_PROPERTIES
                    { SW_PROP_NMID(UNO_NAME_CHAIN_NEXT_NAME), RES_CHAIN,                CPPU_E2T(CPPUTYPE_OUSTRING),            PropertyAttribute::MAYBEVOID ,MID_CHAIN_NEXTNAME},
                    { SW_PROP_NMID(UNO_NAME_CHAIN_PREV_NAME), RES_CHAIN,                CPPU_E2T(CPPUTYPE_OUSTRING),            PropertyAttribute::MAYBEVOID ,MID_CHAIN_PREVNAME},
                /*not impl*/    { SW_PROP_NMID(UNO_NAME_CLIENT_MAP), RES_URL,               CPPU_E2T(CPPUTYPE_BOOLEAN),         PROPERTY_NONE ,MID_URL_CLIENTMAP         },
                    { SW_PROP_NMID(UNO_NAME_EDIT_IN_READONLY), RES_EDIT_IN_READONLY,    CPPU_E2T(CPPUTYPE_BOOLEAN),         PROPERTY_NONE, 0},
                    { SW_PROP_NMID(UNO_NAME_TEXT_COLUMNS), RES_COL,                CPPU_E2T(CPPUTYPE_REFTEXTCOL),    PROPERTY_NONE, MID_COLUMNS},
                    //next elements are part of the service description
                    { SW_PROP_NMID(UNO_NAME_FRAME_HEIGHT_ABSOLUTE),         RES_FRM_SIZE,           CPPU_E2T(CPPUTYPE_INT32),       PROPERTY_NONE, MID_FRMSIZE_HEIGHT|CONVERT_TWIPS         },
                    { SW_PROP_NMID(UNO_NAME_FRAME_HEIGHT_PERCENT),              RES_FRM_SIZE,           CPPU_E2T(CPPUTYPE_INT8),        PROPERTY_NONE, MID_FRMSIZE_REL_HEIGHT   },
                    { SW_PROP_NMID(UNO_NAME_FRAME_ISAUTOMATIC_HEIGHT),         RES_FRM_SIZE,        CPPU_E2T(CPPUTYPE_BOOLEAN),         PROPERTY_NONE, MID_FRMSIZE_IS_AUTO_HEIGHT   },
                    { SW_PROP_NMID(UNO_NAME_FRAME_WIDTH_ABSOLUTE),          RES_FRM_SIZE,           CPPU_E2T(CPPUTYPE_INT32),       PROPERTY_NONE, MID_FRMSIZE_WIDTH|CONVERT_TWIPS          },
                    { SW_PROP_NMID(UNO_NAME_FRAME_WIDTH_PERCENT),               RES_FRM_SIZE,           CPPU_E2T(CPPUTYPE_INT8),        PROPERTY_NONE, MID_FRMSIZE_REL_WIDTH    },
                    { SW_PROP_NMID(UNO_NAME_SIZE_TYPE), RES_FRM_SIZE,           CPPU_E2T(CPPUTYPE_INT16)  ,         PROPERTY_NONE,   MID_FRMSIZE_SIZE_TYPE  },
                    { SW_PROP_NMID(UNO_NAME_WIDTH_TYPE), RES_FRM_SIZE,          CPPU_E2T(CPPUTYPE_INT16)  ,         PROPERTY_NONE,   MID_FRMSIZE_WIDTH_TYPE },
                    { SW_PROP_NMID(UNO_NAME_WRITING_MODE), RES_FRAMEDIR, CPPU_E2T(CPPUTYPE_INT16), PROPERTY_NONE, 0 },
                    {0,0,0,0,0,0}
                };
                aMapEntriesArr[nPropertyId] = aFramePropertyMap_Impl;
            }
            break;
            case PROPERTY_MAP_TEXT_GRAPHIC:
            {
                static SfxItemPropertyMapEntry aGraphicPropertyMap_Impl[] =
                {
                    COMMON_FRAME_PROPERTIES
                    { SW_PROP_NMID(UNO_NAME_SURROUND_CONTOUR), RES_SURROUND,            CPPU_E2T(CPPUTYPE_BOOLEAN),             PROPERTY_NONE, MID_SURROUND_CONTOUR         },
                    { SW_PROP_NMID(UNO_NAME_CONTOUR_OUTSIDE), RES_SURROUND,             CPPU_E2T(CPPUTYPE_BOOLEAN),             PROPERTY_NONE, MID_SURROUND_CONTOUROUTSIDE  },
                    { SW_PROP_NMID(UNO_NAME_GRAPHIC_CROP), RES_GRFATR_CROPGRF,     CPPU_E2T(CPPUTYPE_GRFCROP),  PROPERTY_NONE, CONVERT_TWIPS },
                    { SW_PROP_NMID(UNO_NAME_HORI_MIRRORED_ON_EVEN_PAGES), RES_GRFATR_MIRRORGRF, CPPU_E2T(CPPUTYPE_BOOLEAN),             PROPERTY_NONE,      MID_MIRROR_HORZ_EVEN_PAGES            },
                    { SW_PROP_NMID(UNO_NAME_HORI_MIRRORED_ON_ODD_PAGES), RES_GRFATR_MIRRORGRF,  CPPU_E2T(CPPUTYPE_BOOLEAN),             PROPERTY_NONE,      MID_MIRROR_HORZ_ODD_PAGES                 },
                    { SW_PROP_NMID(UNO_NAME_VERT_MIRRORED), RES_GRFATR_MIRRORGRF,   CPPU_E2T(CPPUTYPE_BOOLEAN),             PROPERTY_NONE,     MID_MIRROR_VERT            },
                    { SW_PROP_NMID(UNO_NAME_GRAPHIC_URL), FN_UNO_GRAPHIC_U_R_L, CPPU_E2T(CPPUTYPE_OUSTRING), 0, 0 },
                    { SW_PROP_NMID(UNO_NAME_REPLACEMENT_GRAPHIC_URL), FN_UNO_REPLACEMENT_GRAPHIC_U_R_L, CPPU_E2T(CPPUTYPE_OUSTRING), 0, 0 },
                    { SW_PROP_NMID(UNO_NAME_GRAPHIC_FILTER), FN_UNO_GRAPHIC_FILTER,      CPPU_E2T(CPPUTYPE_OUSTRING), 0, 0 },
                    { SW_PROP_NMID(UNO_NAME_GRAPHIC), FN_UNO_GRAPHIC, CPPU_E2T(CPPUTYPE_REFXGRAPHIC), 0, 0 },
                    { SW_PROP_NMID(UNO_NAME_ACTUAL_SIZE), FN_UNO_ACTUAL_SIZE,    CPPU_E2T(CPPUTYPE_AWTSIZE),  PropertyAttribute::READONLY, CONVERT_TWIPS},
                    { SW_PROP_NMID(UNO_NAME_CONTOUR_POLY_POLYGON), FN_PARAM_COUNTOUR_PP, CPPU_E2T(CPPUTYPE_PNTSEQSEQ), PropertyAttribute::MAYBEVOID, 0 },
                    { SW_PROP_NMID(UNO_NAME_IS_PIXEL_CONTOUR), FN_UNO_IS_PIXEL_CONTOUR, CPPU_E2T(CPPUTYPE_BOOLEAN), PROPERTY_NONE, 0 },
                    { SW_PROP_NMID(UNO_NAME_IS_AUTOMATIC_CONTOUR), FN_UNO_IS_AUTOMATIC_CONTOUR , CPPU_E2T(CPPUTYPE_BOOLEAN), PROPERTY_NONE, 0 },
                    { SW_PROP_NMID(UNO_NAME_GRAPHIC_ROTATION), RES_GRFATR_ROTATION,      CPPU_E2T(CPPUTYPE_INT16),  0,   0},
                    { SW_PROP_NMID(UNO_NAME_ADJUST_LUMINANCE), RES_GRFATR_LUMINANCE,     CPPU_E2T(CPPUTYPE_INT16),  0,   0},
                    { SW_PROP_NMID(UNO_NAME_ADJUST_CONTRAST), RES_GRFATR_CONTRAST,   CPPU_E2T(CPPUTYPE_INT16),  0,   0},
                    { SW_PROP_NMID(UNO_NAME_ADJUST_RED), RES_GRFATR_CHANNELR,    CPPU_E2T(CPPUTYPE_INT16),  0,   0},
                    { SW_PROP_NMID(UNO_NAME_ADJUST_GREEN), RES_GRFATR_CHANNELG,      CPPU_E2T(CPPUTYPE_INT16),  0,   0},
                    { SW_PROP_NMID(UNO_NAME_ADJUST_BLUE), RES_GRFATR_CHANNELB,   CPPU_E2T(CPPUTYPE_INT16),  0,   0},
                    { SW_PROP_NMID(UNO_NAME_GAMMA), RES_GRFATR_GAMMA,        CPPU_E2T(CPPUTYPE_DOUBLE),     0,   0},
                    { SW_PROP_NMID(UNO_NAME_GRAPHIC_IS_INVERTED), RES_GRFATR_INVERT,         CPPU_E2T(CPPUTYPE_BOOLEAN),    0,   0},
                    { SW_PROP_NMID(UNO_NAME_TRANSPARENCY), RES_GRFATR_TRANSPARENCY, CPPU_E2T(CPPUTYPE_INT16),   0,   0},
                    { SW_PROP_NMID(UNO_NAME_GRAPHIC_COLOR_MODE), RES_GRFATR_DRAWMODE,    CPPU_E2T(CPPUTYPE_COLORMODE),      0,   0},
                    {0,0,0,0,0,0}
                };
                aMapEntriesArr[nPropertyId] = aGraphicPropertyMap_Impl;
            }
            break;
            case PROPERTY_MAP_EMBEDDED_OBJECT:
            {
                static SfxItemPropertyMapEntry aEmbeddedPropertyMap_Impl[] =
                {
                    COMMON_FRAME_PROPERTIES
                    { SW_PROP_NMID(UNO_NAME_SURROUND_CONTOUR), RES_SURROUND, CPPU_E2T(CPPUTYPE_BOOLEAN), PROPERTY_NONE, MID_SURROUND_CONTOUR },
                    { SW_PROP_NMID(UNO_NAME_CONTOUR_OUTSIDE), RES_SURROUND, CPPU_E2T(CPPUTYPE_BOOLEAN), PROPERTY_NONE, MID_SURROUND_CONTOUROUTSIDE},
                    { SW_PROP_NMID(UNO_NAME_CONTOUR_POLY_POLYGON), FN_PARAM_COUNTOUR_PP, CPPU_E2T(CPPUTYPE_PNTSEQSEQ), PropertyAttribute::MAYBEVOID, 0 },
                    { SW_PROP_NMID(UNO_NAME_IS_PIXEL_CONTOUR), FN_UNO_IS_PIXEL_CONTOUR, CPPU_E2T(CPPUTYPE_BOOLEAN), PROPERTY_NONE, 0 },
                    { SW_PROP_NMID(UNO_NAME_IS_AUTOMATIC_CONTOUR), FN_UNO_IS_AUTOMATIC_CONTOUR , CPPU_E2T(CPPUTYPE_BOOLEAN), PROPERTY_NONE, 0 },
                    { SW_PROP_NMID(UNO_NAME_CLSID),                FN_UNO_CLSID, CPPU_E2T(CPPUTYPE_OUSTRING), PROPERTY_NONE, 0 },
                    { SW_PROP_NMID(UNO_NAME_STREAM_NAME),           FN_UNO_STREAM_NAME, CPPU_E2T(CPPUTYPE_OUSTRING), PROPERTY_NONE, 0 },
                    { SW_PROP_NMID(UNO_NAME_MODEL), FN_UNO_MODEL, CPPU_E2T(CPPUTYPE_REFMODEL), PropertyAttribute::READONLY|PropertyAttribute::MAYBEVOID, 0},
                    { SW_PROP_NMID(UNO_NAME_GRAPHIC_URL), FN_UNO_REPLACEMENT_GRAPHIC_URL, CPPU_E2T(CPPUTYPE_OUSTRING), PropertyAttribute::MAYBEVOID, 0 },
                    { SW_PROP_NMID(UNO_NAME_GRAPHIC), FN_UNO_REPLACEMENT_GRAPHIC, CPPU_E2T(CPPUTYPE_REFXGRAPHIC), PropertyAttribute::MAYBEVOID, 0 },
                    { SW_PROP_NMID(UNO_NAME_COMPONENT),FN_UNO_COMPONENT, CPPU_E2T(CPPUTYPE_REFCOMPONENT), PropertyAttribute::READONLY, 0},
                    { SW_PROP_NMID(UNO_NAME_EMBEDDED_OBJECT),FN_EMBEDDED_OBJECT, CPPU_E2T(CPPUTPYE_REFEMBEDDEDOBJECT), PROPERTY_NONE, 0},
                    {0,0,0,0,0,0}
                };
                aMapEntriesArr[nPropertyId] = aEmbeddedPropertyMap_Impl;
            }
            break;
            case PROPERTY_MAP_TEXT_SHAPE:
            {
                static SfxItemPropertyMapEntry aShapeMap_Impl[] =
                {
                    { SW_PROP_NMID(UNO_NAME_ANCHOR_PAGE_NO), RES_ANCHOR,            CPPU_E2T(CPPUTYPE_INT16),           PROPERTY_NONE|PropertyAttribute::MAYBEVOID, MID_ANCHOR_PAGENUM      },
                    { SW_PROP_NMID(UNO_NAME_ANCHOR_TYPE), RES_ANCHOR,           CPPU_E2T(CPPUTYPE_TXTCNTANCHOR),            PROPERTY_NONE|PropertyAttribute::MAYBEVOID, MID_ANCHOR_ANCHORTYPE},
                    { SW_PROP_NMID(UNO_NAME_ANCHOR_FRAME), RES_ANCHOR,             CPPU_E2T(CPPUTYPE_REFTEXTFRAME),    PropertyAttribute::MAYBEVOID, MID_ANCHOR_ANCHORFRAME},
                    { SW_PROP_NMID(UNO_NAME_HORI_ORIENT), RES_HORI_ORIENT,        CPPU_E2T(CPPUTYPE_INT16),   PROPERTY_NONE|PropertyAttribute::MAYBEVOID ,MID_HORIORIENT_ORIENT },
                    { SW_PROP_NMID(UNO_NAME_HORI_ORIENT_POSITION), RES_HORI_ORIENT,     CPPU_E2T(CPPUTYPE_INT32),           PROPERTY_NONE|PropertyAttribute::MAYBEVOID ,MID_HORIORIENT_POSITION|CONVERT_TWIPS   },
                    { SW_PROP_NMID(UNO_NAME_HORI_ORIENT_RELATION), RES_HORI_ORIENT,     CPPU_E2T(CPPUTYPE_INT16),           PROPERTY_NONE|PropertyAttribute::MAYBEVOID ,MID_HORIORIENT_RELATION },
                    { SW_PROP_NMID(UNO_NAME_LEFT_MARGIN), RES_LR_SPACE,             CPPU_E2T(CPPUTYPE_INT32), PROPERTY_NONE|PropertyAttribute::MAYBEVOID, MID_L_MARGIN|CONVERT_TWIPS},
                    { SW_PROP_NMID(UNO_NAME_RIGHT_MARGIN), RES_LR_SPACE,            CPPU_E2T(CPPUTYPE_INT32), PROPERTY_NONE|PropertyAttribute::MAYBEVOID, MID_R_MARGIN|CONVERT_TWIPS},
                    { SW_PROP_NMID(UNO_NAME_SURROUND), RES_SURROUND,          CPPU_E2T(CPPUTYPE_WRAPTXTMODE),    PROPERTY_NONE|PropertyAttribute::MAYBEVOID, MID_SURROUND_SURROUNDTYPE },
                    { SW_PROP_NMID(UNO_NAME_TEXT_WRAP), RES_SURROUND,           CPPU_E2T(CPPUTYPE_WRAPTXTMODE),             PROPERTY_NONE, MID_SURROUND_SURROUNDTYPE    },
                    { SW_PROP_NMID(UNO_NAME_SURROUND_ANCHORONLY), RES_SURROUND,             CPPU_E2T(CPPUTYPE_BOOLEAN),             PROPERTY_NONE|PropertyAttribute::MAYBEVOID, MID_SURROUND_ANCHORONLY     },
                    { SW_PROP_NMID(UNO_NAME_SURROUND_CONTOUR), RES_SURROUND,            CPPU_E2T(CPPUTYPE_BOOLEAN),             PROPERTY_NONE, MID_SURROUND_CONTOUR         },
                    { SW_PROP_NMID(UNO_NAME_CONTOUR_OUTSIDE), RES_SURROUND,             CPPU_E2T(CPPUTYPE_BOOLEAN),             PROPERTY_NONE, MID_SURROUND_CONTOUROUTSIDE  },
                    { SW_PROP_NMID(UNO_NAME_TOP_MARGIN), RES_UL_SPACE,          CPPU_E2T(CPPUTYPE_INT32), PROPERTY_NONE, MID_UP_MARGIN|CONVERT_TWIPS},
                    { SW_PROP_NMID(UNO_NAME_BOTTOM_MARGIN), RES_UL_SPACE,           CPPU_E2T(CPPUTYPE_INT32), PROPERTY_NONE, MID_LO_MARGIN|CONVERT_TWIPS},
                    { SW_PROP_NMID(UNO_NAME_VERT_ORIENT), RES_VERT_ORIENT,      CPPU_E2T(CPPUTYPE_INT16),           PROPERTY_NONE|PropertyAttribute::MAYBEVOID ,MID_VERTORIENT_ORIENT   },
                    { SW_PROP_NMID(UNO_NAME_VERT_ORIENT_POSITION), RES_VERT_ORIENT,     CPPU_E2T(CPPUTYPE_INT32),           PROPERTY_NONE|PropertyAttribute::MAYBEVOID ,MID_VERTORIENT_POSITION|CONVERT_TWIPS   },
                    { SW_PROP_NMID(UNO_NAME_VERT_ORIENT_RELATION), RES_VERT_ORIENT,     CPPU_E2T(CPPUTYPE_INT16),           PROPERTY_NONE|PropertyAttribute::MAYBEVOID ,MID_VERTORIENT_RELATION },
                    { SW_PROP_NMID(UNO_NAME_TEXT_RANGE), FN_TEXT_RANGE,         CPPU_E2T(CPPUTYPE_REFTXTRANGE),         PROPERTY_NONE, 0},
                    { SW_PROP_NMID(UNO_NAME_OPAQUE), RES_OPAQUE,             CPPU_E2T(CPPUTYPE_BOOLEAN),            PROPERTY_NONE, 0},
                    { SW_PROP_NMID(UNO_NAME_ANCHOR_POSITION), FN_ANCHOR_POSITION,    CPPU_E2T(CPPUTYPE_AWTPOINT),    PropertyAttribute::READONLY, 0},
                    // #i26791#
                    { SW_PROP_NMID(UNO_NAME_IS_FOLLOWING_TEXT_FLOW), RES_FOLLOW_TEXT_FLOW,     CPPU_E2T(CPPUTYPE_BOOLEAN), PROPERTY_NONE, 0},
                    // #i28701#
                    { SW_PROP_NMID(UNO_NAME_WRAP_INFLUENCE_ON_POSITION), RES_WRAP_INFLUENCE_ON_OBJPOS, CPPU_E2T(CPPUTYPE_INT8), PROPERTY_NONE, MID_WRAP_INFLUENCE},
                    // #i28749#
                    { SW_PROP_NMID( UNO_NAME_TRANSFORMATION_IN_HORI_L2R),
                                    FN_SHAPE_TRANSFORMATION_IN_HORI_L2R,
                                    CPPU_E2T(CPPUTYPE_TRANSFORMATIONINHORIL2R),
                                    PropertyAttribute::READONLY, 0},
                    { SW_PROP_NMID( UNO_NAME_POSITION_LAYOUT_DIR),
                                    FN_SHAPE_POSITION_LAYOUT_DIR,
                                    CPPU_E2T(CPPUTYPE_INT16),
                                    PROPERTY_NONE, 0},
                    // #i36248#
                    { SW_PROP_NMID( UNO_NAME_STARTPOSITION_IN_HORI_L2R),
                                    FN_SHAPE_STARTPOSITION_IN_HORI_L2R,
                                    CPPU_E2T(CPPUTYPE_AWTPOINT),
                                    PropertyAttribute::READONLY, 0},
                    { SW_PROP_NMID( UNO_NAME_ENDPOSITION_IN_HORI_L2R),
                                    FN_SHAPE_ENDPOSITION_IN_HORI_L2R,
                                    CPPU_E2T(CPPUTYPE_AWTPOINT),
                                    PropertyAttribute::READONLY, 0},
                    // #i71182#
                    // missing map entry for property <PageToogle>
                    { SW_PROP_NMID(UNO_NAME_PAGE_TOGGLE), RES_HORI_ORIENT,      CPPU_E2T(CPPUTYPE_BOOLEAN),         PROPERTY_NONE ,MID_HORIORIENT_PAGETOGGLE },
                    { SW_PROP_NMID(UNO_NAME_RELATIVE_HEIGHT), RES_FRM_SIZE,     CPPU_E2T(CPPUTYPE_INT16)  ,         PROPERTY_NONE, MID_FRMSIZE_REL_HEIGHT },
                    { SW_PROP_NMID(UNO_NAME_RELATIVE_WIDTH), RES_FRM_SIZE,      CPPU_E2T(CPPUTYPE_INT16)  ,         PROPERTY_NONE, MID_FRMSIZE_REL_WIDTH  },
                    {0,0,0,0,0,0}
                };
                aMapEntriesArr[nPropertyId] = aShapeMap_Impl;
            }
            break;
            case PROPERTY_MAP_INDEX_MARK:
            {
                static SfxItemPropertyMapEntry aIdxMarkMap_Impl[] =
                {
                    { SW_PROP_NMID(UNO_NAME_ALTERNATIVE_TEXT), WID_ALT_TEXT,        CPPU_E2T(CPPUTYPE_OUSTRING)  ,      PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_PRIMARY_KEY), WID_PRIMARY_KEY,  CPPU_E2T(CPPUTYPE_OUSTRING)  ,      PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_SECONDARY_KEY), WID_SECONDARY_KEY,  CPPU_E2T(CPPUTYPE_OUSTRING)  ,      PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_TEXT_READING), WID_TEXT_READING, CPPU_E2T(CPPUTYPE_OUSTRING), PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_PRIMARY_KEY_READING), WID_PRIMARY_KEY_READING, CPPU_E2T(CPPUTYPE_OUSTRING), PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_SECONDARY_KEY_READING), WID_SECONDARY_KEY_READING, CPPU_E2T(CPPUTYPE_OUSTRING), PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_IS_MAIN_ENTRY), WID_MAIN_ENTRY,     CPPU_E2T(CPPUTYPE_BOOLEAN)  ,       PROPERTY_NONE,     0},
                    COMMON_TEXT_CONTENT_PROPERTIES
                    {0,0,0,0,0,0}
                };
                aMapEntriesArr[nPropertyId] = aIdxMarkMap_Impl;
            }
            break;
            case PROPERTY_MAP_CNTIDX_MARK:
            {
                static SfxItemPropertyMapEntry aCntntMarkMap_Impl[] =
                {
                    { SW_PROP_NMID(UNO_NAME_ALTERNATIVE_TEXT), WID_ALT_TEXT,        CPPU_E2T(CPPUTYPE_OUSTRING)  ,      PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_LEVEL), WID_LEVEL        ,  CPPU_E2T(CPPUTYPE_INT16)  ,         PROPERTY_NONE,     0},
                    COMMON_TEXT_CONTENT_PROPERTIES
                    {0,0,0,0,0,0}
                };
                aMapEntriesArr[nPropertyId] = aCntntMarkMap_Impl;
            }
            break;
            case PROPERTY_MAP_USER_MARK:
            {
                static SfxItemPropertyMapEntry aUserMarkMap_Impl[] =
                {
                    { SW_PROP_NMID(UNO_NAME_ALTERNATIVE_TEXT), WID_ALT_TEXT,        CPPU_E2T(CPPUTYPE_OUSTRING)  ,      PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_LEVEL        ),   WID_LEVEL        ,    CPPU_E2T(CPPUTYPE_INT16)  ,         PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_USER_INDEX_NAME), WID_USER_IDX_NAME,    CPPU_E2T(CPPUTYPE_OUSTRING)  ,      PROPERTY_NONE,     0},
                    COMMON_TEXT_CONTENT_PROPERTIES
                    {0,0,0,0,0,0}
                };
                aMapEntriesArr[nPropertyId] = aUserMarkMap_Impl;
            }
            break;
            case PROPERTY_MAP_INDEX_IDX:
            {
                static SfxItemPropertyMapEntry aTOXIndexMap_Impl[] =
                {
                    _BASE_INDEX_PROPERTIES_
                    { SW_PROP_NMID(UNO_NAME_CREATE_FROM_CHAPTER), WID_CREATE_FROM_CHAPTER                 ,  CPPU_E2T(CPPUTYPE_BOOLEAN)  , PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_IS_PROTECTED), WID_PROTECTED                           ,  CPPU_E2T(CPPUTYPE_BOOLEAN)  , PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_USE_ALPHABETICAL_SEPARATORS), WID_USE_ALPHABETICAL_SEPARATORS         ,  CPPU_E2T(CPPUTYPE_BOOLEAN)  , PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_USE_KEY_AS_ENTRY), WID_USE_KEY_AS_ENTRY                    ,  CPPU_E2T(CPPUTYPE_BOOLEAN)  , PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_USE_COMBINED_ENTRIES), WID_USE_COMBINED_ENTRIES                ,  CPPU_E2T(CPPUTYPE_BOOLEAN)  , PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_IS_CASE_SENSITIVE), WID_IS_CASE_SENSITIVE                   ,  CPPU_E2T(CPPUTYPE_BOOLEAN)  , PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_USE_P_P), WID_USE_P_P                             ,  CPPU_E2T(CPPUTYPE_BOOLEAN)  , PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_USE_DASH), WID_USE_DASH                            ,  CPPU_E2T(CPPUTYPE_BOOLEAN)  , PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_USE_UPPER_CASE), WID_USE_UPPER_CASE                      ,  CPPU_E2T(CPPUTYPE_BOOLEAN)  , PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_LEVEL_FORMAT), WID_LEVEL_FORMAT                        ,  CPPU_E2T(CPPUTYPE_REFIDXREPL)  , PROPERTY_NONE,   0},
                    { SW_PROP_NMID(UNO_NAME_MAIN_ENTRY_CHARACTER_STYLE_NAME), WID_MAIN_ENTRY_CHARACTER_STYLE_NAME     ,  CPPU_E2T(CPPUTYPE_OUSTRING)  , PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_TEXT_COLUMNS), RES_COL,                CPPU_E2T(CPPUTYPE_REFTEXTCOL),    PROPERTY_NONE, MID_COLUMNS},
                    { SW_PROP_NMID(UNO_NAME_BACK_GRAPHIC_URL), RES_BACKGROUND,      CPPU_E2T(CPPUTYPE_OUSTRING), PROPERTY_NONE ,MID_GRAPHIC_URL    },
                    { SW_PROP_NMID(UNO_NAME_BACK_GRAPHIC_FILTER), RES_BACKGROUND,       CPPU_E2T(CPPUTYPE_OUSTRING), PROPERTY_NONE ,MID_GRAPHIC_FILTER    },
                    { SW_PROP_NMID(UNO_NAME_BACK_GRAPHIC_LOCATION), RES_BACKGROUND,         CPPU_E2T(CPPUTYPE_GRAPHICLOC),          PROPERTY_NONE ,MID_GRAPHIC_POSITION},
                    { SW_PROP_NMID(UNO_NAME_BACK_COLOR), RES_BACKGROUND,            CPPU_E2T(CPPUTYPE_INT32),           PROPERTY_NONE ,MID_BACK_COLOR        },
                    { SW_PROP_NMID(UNO_NAME_BACK_TRANSPARENT), RES_BACKGROUND,      CPPU_E2T(CPPUTYPE_BOOLEAN),         PROPERTY_NONE ,MID_GRAPHIC_TRANSPARENT       },
                    { SW_PROP_NMID(UNO_NAME_PARA_STYLEHEADING),                 WID_PARA_HEAD,          CPPU_E2T(CPPUTYPE_OUSTRING)  , 0,     0},
                    { SW_PROP_NMID(UNO_NAME_PARA_STYLESEPARATOR),           WID_PARA_SEP,           CPPU_E2T(CPPUTYPE_OUSTRING)  , 0,     0},
                    { SW_PROP_NMID(UNO_NAME_PARA_STYLELEVEL1),              WID_PARA_LEV1,          CPPU_E2T(CPPUTYPE_OUSTRING)  , 0,     0},
                    { SW_PROP_NMID(UNO_NAME_PARA_STYLELEVEL2),              WID_PARA_LEV2,          CPPU_E2T(CPPUTYPE_OUSTRING)  , 0,     0},
                    { SW_PROP_NMID(UNO_NAME_PARA_STYLELEVEL3),              WID_PARA_LEV3,          CPPU_E2T(CPPUTYPE_OUSTRING)  , 0,     0},
                    { SW_PROP_NMID(UNO_NAME_IS_COMMA_SEPARATED), WID_IS_COMMA_SEPARATED, CPPU_E2T(CPPUTYPE_BOOLEAN),            PROPERTY_NONE ,0         },
                    { SW_PROP_NMID(UNO_NAME_DOCUMENT_INDEX_MARKS), WID_INDEX_MARKS, CPPU_E2T(CPPUTYPE_DOCIDXMRK),           PropertyAttribute::READONLY ,0       },
                    { SW_PROP_NMID(UNO_NAME_IS_RELATIVE_TABSTOPS), WID_IS_RELATIVE_TABSTOPS, CPPU_E2T(CPPUTYPE_BOOLEAN), PROPERTY_NONE, 0},
                    { SW_PROP_NMID(UNO_NAME_LOCALE),            WID_IDX_LOCALE,         CPPU_E2T(CPPUTYPE_LOCALE), PROPERTY_NONE,     0},\
                    { SW_PROP_NMID(UNO_NAME_SORT_ALGORITHM),    WID_IDX_SORT_ALGORITHM,  CPPU_E2T(CPPUTYPE_OUSTRING), PROPERTY_NONE,     0},\
                    {0,0,0,0,0,0}
                };
                aMapEntriesArr[nPropertyId] = aTOXIndexMap_Impl;
            }
            break;
            case PROPERTY_MAP_INDEX_CNTNT:
            {
                static SfxItemPropertyMapEntry aTOXContentMap_Impl[] =
                {
                    _BASE_INDEX_PROPERTIES_
                    { SW_PROP_NMID(UNO_NAME_LEVEL), WID_LEVEL                               ,  CPPU_E2T(CPPUTYPE_INT16)  , PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_CREATE_FROM_MARKS), WID_CREATE_FROM_MARKS                   ,  CPPU_E2T(CPPUTYPE_BOOLEAN)  , PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_CREATE_FROM_OUTLINE), WID_CREATE_FROM_OUTLINE                 ,  CPPU_E2T(CPPUTYPE_BOOLEAN)  , PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_CREATE_FROM_CHAPTER), WID_CREATE_FROM_CHAPTER                 ,  CPPU_E2T(CPPUTYPE_BOOLEAN)  , PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_IS_PROTECTED), WID_PROTECTED                           ,  CPPU_E2T(CPPUTYPE_BOOLEAN)  , PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_LEVEL_FORMAT), WID_LEVEL_FORMAT                        ,  CPPU_E2T(CPPUTYPE_REFIDXREPL)  , PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_LEVEL_PARAGRAPH_STYLES), WID_LEVEL_PARAGRAPH_STYLES              ,  CPPU_E2T(CPPUTYPE_REFIDXREPL)  , PropertyAttribute::READONLY,     0},
                    { SW_PROP_NMID(UNO_NAME_CREATE_FROM_LEVEL_PARAGRAPH_STYLES), WID_CREATE_FROM_PARAGRAPH_STYLES, CPPU_E2T(CPPUTYPE_BOOLEAN), PROPERTY_NONE, 0},
                    { SW_PROP_NMID(UNO_NAME_TEXT_COLUMNS), RES_COL,                CPPU_E2T(CPPUTYPE_REFTEXTCOL),    PROPERTY_NONE, MID_COLUMNS},
                    { SW_PROP_NMID(UNO_NAME_BACK_GRAPHIC_URL), RES_BACKGROUND,      CPPU_E2T(CPPUTYPE_OUSTRING), PROPERTY_NONE ,MID_GRAPHIC_URL    },
                    { SW_PROP_NMID(UNO_NAME_BACK_GRAPHIC_FILTER), RES_BACKGROUND,       CPPU_E2T(CPPUTYPE_OUSTRING), PROPERTY_NONE ,MID_GRAPHIC_FILTER    },
                    { SW_PROP_NMID(UNO_NAME_BACK_GRAPHIC_LOCATION), RES_BACKGROUND,         CPPU_E2T(CPPUTYPE_GRAPHICLOC),          PROPERTY_NONE ,MID_GRAPHIC_POSITION},
                    { SW_PROP_NMID(UNO_NAME_BACK_COLOR), RES_BACKGROUND,            CPPU_E2T(CPPUTYPE_INT32),           PROPERTY_NONE ,MID_BACK_COLOR        },
                    { SW_PROP_NMID(UNO_NAME_BACK_TRANSPARENT), RES_BACKGROUND,      CPPU_E2T(CPPUTYPE_BOOLEAN),         PROPERTY_NONE ,MID_GRAPHIC_TRANSPARENT       },
                    { SW_PROP_NMID(UNO_NAME_PARA_STYLEHEADING),     WID_PARA_HEAD,  CPPU_E2T(CPPUTYPE_OUSTRING)  , 0,     0},
                    { SW_PROP_NMID(UNO_NAME_PARA_STYLELEVEL1),  WID_PARA_LEV1,  CPPU_E2T(CPPUTYPE_OUSTRING)  , 0,     0},
                    { SW_PROP_NMID(UNO_NAME_PARA_STYLELEVEL2),  WID_PARA_LEV2,  CPPU_E2T(CPPUTYPE_OUSTRING)  , 0,     0},
                    { SW_PROP_NMID(UNO_NAME_PARA_STYLELEVEL3),  WID_PARA_LEV3,  CPPU_E2T(CPPUTYPE_OUSTRING)  , 0,     0},
                    { SW_PROP_NMID(UNO_NAME_PARA_STYLELEVEL4),  WID_PARA_LEV4,  CPPU_E2T(CPPUTYPE_OUSTRING)  , 0,     0},
                    { SW_PROP_NMID(UNO_NAME_PARA_STYLELEVEL5),  WID_PARA_LEV5,  CPPU_E2T(CPPUTYPE_OUSTRING)  , 0,     0},
                    { SW_PROP_NMID(UNO_NAME_PARA_STYLELEVEL6),  WID_PARA_LEV6,  CPPU_E2T(CPPUTYPE_OUSTRING)  , 0,     0},
                    { SW_PROP_NMID(UNO_NAME_PARA_STYLELEVEL7),  WID_PARA_LEV7,  CPPU_E2T(CPPUTYPE_OUSTRING)  , 0,     0},
                    { SW_PROP_NMID(UNO_NAME_PARA_STYLELEVEL8),  WID_PARA_LEV8,  CPPU_E2T(CPPUTYPE_OUSTRING)  , 0,     0},
                    { SW_PROP_NMID(UNO_NAME_PARA_STYLELEVEL9),  WID_PARA_LEV9,  CPPU_E2T(CPPUTYPE_OUSTRING)  , 0,     0},
                    { SW_PROP_NMID(UNO_NAME_PARA_STYLELEVEL10),     WID_PARA_LEV10,     CPPU_E2T(CPPUTYPE_OUSTRING)  , 0,     0},
                    { SW_PROP_NMID(UNO_NAME_IS_RELATIVE_TABSTOPS), WID_IS_RELATIVE_TABSTOPS, CPPU_E2T(CPPUTYPE_BOOLEAN), PROPERTY_NONE, 0},
                    { SW_PROP_NMID(UNO_NAME_DOCUMENT_INDEX_MARKS), WID_INDEX_MARKS, CPPU_E2T(CPPUTYPE_DOCIDXMRK),           PropertyAttribute::READONLY ,0       },
                    {0,0,0,0,0,0}
                };
                aMapEntriesArr[nPropertyId] = aTOXContentMap_Impl;
            }
            break;
            case PROPERTY_MAP_INDEX_USER:
            {
                static SfxItemPropertyMapEntry aTOXUserMap_Impl[] =
                {
                    _BASE_INDEX_PROPERTIES_
                    { SW_PROP_NMID(UNO_NAME_CREATE_FROM_MARKS), WID_CREATE_FROM_MARKS                   ,  CPPU_E2T(CPPUTYPE_BOOLEAN)  , PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_CREATE_FROM_CHAPTER), WID_CREATE_FROM_CHAPTER                 ,  CPPU_E2T(CPPUTYPE_BOOLEAN)  , PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_IS_PROTECTED), WID_PROTECTED                           ,  CPPU_E2T(CPPUTYPE_BOOLEAN)  , PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_USE_LEVEL_FROM_SOURCE), WID_USE_LEVEL_FROM_SOURCE               ,  CPPU_E2T(CPPUTYPE_BOOLEAN)  , PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_LEVEL_FORMAT), WID_LEVEL_FORMAT                        ,  CPPU_E2T(CPPUTYPE_REFIDXREPL)  , PROPERTY_NONE,0},
                    { SW_PROP_NMID(UNO_NAME_LEVEL_PARAGRAPH_STYLES), WID_LEVEL_PARAGRAPH_STYLES              ,  CPPU_E2T(CPPUTYPE_REFIDXREPL)  , PropertyAttribute::READONLY,0},
                    { SW_PROP_NMID(UNO_NAME_CREATE_FROM_LEVEL_PARAGRAPH_STYLES), WID_CREATE_FROM_PARAGRAPH_STYLES, CPPU_E2T(CPPUTYPE_BOOLEAN), PROPERTY_NONE, 0},
                    { SW_PROP_NMID(UNO_NAME_CREATE_FROM_TABLES), WID_CREATE_FROM_TABLES                  ,  CPPU_E2T(CPPUTYPE_BOOLEAN)  , PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_CREATE_FROM_TEXT_FRAMES), WID_CREATE_FROM_TEXT_FRAMES             ,  CPPU_E2T(CPPUTYPE_BOOLEAN)  , PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_CREATE_FROM_GRAPHIC_OBJECTS), WID_CREATE_FROM_GRAPHIC_OBJECTS         ,  CPPU_E2T(CPPUTYPE_BOOLEAN)  , PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_CREATE_FROM_EMBEDDED_OBJECTS), WID_CREATE_FROM_EMBEDDED_OBJECTS        ,  CPPU_E2T(CPPUTYPE_BOOLEAN)  , PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_TEXT_COLUMNS), RES_COL,                CPPU_E2T(CPPUTYPE_REFTEXTCOL),    PROPERTY_NONE, MID_COLUMNS},
                    { SW_PROP_NMID(UNO_NAME_BACK_GRAPHIC_URL), RES_BACKGROUND,      CPPU_E2T(CPPUTYPE_OUSTRING), PROPERTY_NONE ,MID_GRAPHIC_URL    },
                    { SW_PROP_NMID(UNO_NAME_BACK_GRAPHIC_FILTER), RES_BACKGROUND,       CPPU_E2T(CPPUTYPE_OUSTRING), PROPERTY_NONE ,MID_GRAPHIC_FILTER    },
                    { SW_PROP_NMID(UNO_NAME_BACK_GRAPHIC_LOCATION), RES_BACKGROUND,         CPPU_E2T(CPPUTYPE_GRAPHICLOC),          PROPERTY_NONE ,MID_GRAPHIC_POSITION},
                    { SW_PROP_NMID(UNO_NAME_BACK_COLOR), RES_BACKGROUND,            CPPU_E2T(CPPUTYPE_INT32),           PROPERTY_NONE ,MID_BACK_COLOR        },
                    { SW_PROP_NMID(UNO_NAME_BACK_TRANSPARENT), RES_BACKGROUND,      CPPU_E2T(CPPUTYPE_BOOLEAN),         PROPERTY_NONE ,MID_GRAPHIC_TRANSPARENT       },
                    { SW_PROP_NMID(UNO_NAME_PARA_STYLEHEADING),     WID_PARA_HEAD,  CPPU_E2T(CPPUTYPE_OUSTRING)  , 0,     0},
                    { SW_PROP_NMID(UNO_NAME_PARA_STYLELEVEL1),  WID_PARA_LEV1,  CPPU_E2T(CPPUTYPE_OUSTRING)  , 0,     0},
                    { SW_PROP_NMID(UNO_NAME_PARA_STYLELEVEL2),  WID_PARA_LEV2,  CPPU_E2T(CPPUTYPE_OUSTRING)  , 0,     0},
                    { SW_PROP_NMID(UNO_NAME_PARA_STYLELEVEL3),  WID_PARA_LEV3,  CPPU_E2T(CPPUTYPE_OUSTRING)  , 0,     0},
                    { SW_PROP_NMID(UNO_NAME_PARA_STYLELEVEL4),  WID_PARA_LEV4,  CPPU_E2T(CPPUTYPE_OUSTRING)  , 0,     0},
                    { SW_PROP_NMID(UNO_NAME_PARA_STYLELEVEL5),  WID_PARA_LEV5,  CPPU_E2T(CPPUTYPE_OUSTRING)  , 0,     0},
                    { SW_PROP_NMID(UNO_NAME_PARA_STYLELEVEL6),  WID_PARA_LEV6,  CPPU_E2T(CPPUTYPE_OUSTRING)  , 0,     0},
                    { SW_PROP_NMID(UNO_NAME_PARA_STYLELEVEL7),  WID_PARA_LEV7,  CPPU_E2T(CPPUTYPE_OUSTRING)  , 0,     0},
                    { SW_PROP_NMID(UNO_NAME_PARA_STYLELEVEL8),  WID_PARA_LEV8,  CPPU_E2T(CPPUTYPE_OUSTRING)  , 0,     0},
                    { SW_PROP_NMID(UNO_NAME_PARA_STYLELEVEL9),  WID_PARA_LEV9,  CPPU_E2T(CPPUTYPE_OUSTRING)  , 0,     0},
                    { SW_PROP_NMID(UNO_NAME_PARA_STYLELEVEL10),     WID_PARA_LEV10,     CPPU_E2T(CPPUTYPE_OUSTRING)  , 0,     0},
                    { SW_PROP_NMID(UNO_NAME_DOCUMENT_INDEX_MARKS), WID_INDEX_MARKS, CPPU_E2T(CPPUTYPE_DOCIDXMRK),           PropertyAttribute::READONLY ,0       },
                    { SW_PROP_NMID(UNO_NAME_IS_RELATIVE_TABSTOPS), WID_IS_RELATIVE_TABSTOPS, CPPU_E2T(CPPUTYPE_BOOLEAN), PROPERTY_NONE, 0},
                    { SW_PROP_NMID(UNO_NAME_USER_INDEX_NAME), WID_USER_IDX_NAME,    CPPU_E2T(CPPUTYPE_OUSTRING)  ,      PROPERTY_NONE,     0},
                    {0,0,0,0,0,0}
                };
                aMapEntriesArr[nPropertyId] = aTOXUserMap_Impl;
            }
            break;
            case PROPERTY_MAP_INDEX_TABLES:
            {
                static SfxItemPropertyMapEntry aTOXTablesMap_Impl[] =
                {
                    _BASE_INDEX_PROPERTIES_
                    { SW_PROP_NMID(UNO_NAME_CREATE_FROM_CHAPTER), WID_CREATE_FROM_CHAPTER                 ,  CPPU_E2T(CPPUTYPE_BOOLEAN)  , PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_CREATE_FROM_LABELS), WID_CREATE_FROM_LABELS                  ,  CPPU_E2T(CPPUTYPE_BOOLEAN)  , PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_IS_PROTECTED), WID_PROTECTED                           ,  CPPU_E2T(CPPUTYPE_BOOLEAN)  , PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_LABEL_CATEGORY), WID_LABEL_CATEGORY                      ,  CPPU_E2T(CPPUTYPE_OUSTRING)  , PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_LABEL_DISPLAY_TYPE), WID_LABEL_DISPLAY_TYPE                  ,  CPPU_E2T(CPPUTYPE_INT16)  , PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_LEVEL_FORMAT), WID_LEVEL_FORMAT                        ,  CPPU_E2T(CPPUTYPE_REFIDXREPL)  , PROPERTY_NONE,0},
                    { SW_PROP_NMID(UNO_NAME_TEXT_COLUMNS), RES_COL,                CPPU_E2T(CPPUTYPE_REFTEXTCOL),    PROPERTY_NONE, MID_COLUMNS},
                    { SW_PROP_NMID(UNO_NAME_BACK_GRAPHIC_URL), RES_BACKGROUND,      CPPU_E2T(CPPUTYPE_OUSTRING), PROPERTY_NONE ,MID_GRAPHIC_URL    },
                    { SW_PROP_NMID(UNO_NAME_BACK_GRAPHIC_FILTER), RES_BACKGROUND,       CPPU_E2T(CPPUTYPE_OUSTRING), PROPERTY_NONE ,MID_GRAPHIC_FILTER    },
                    { SW_PROP_NMID(UNO_NAME_BACK_GRAPHIC_LOCATION), RES_BACKGROUND,         CPPU_E2T(CPPUTYPE_GRAPHICLOC),          PROPERTY_NONE ,MID_GRAPHIC_POSITION},
                    { SW_PROP_NMID(UNO_NAME_BACK_COLOR), RES_BACKGROUND,            CPPU_E2T(CPPUTYPE_INT32),           PROPERTY_NONE ,MID_BACK_COLOR        },
                    { SW_PROP_NMID(UNO_NAME_BACK_TRANSPARENT), RES_BACKGROUND,      CPPU_E2T(CPPUTYPE_BOOLEAN),         PROPERTY_NONE ,MID_GRAPHIC_TRANSPARENT       },
                    { SW_PROP_NMID(UNO_NAME_PARA_STYLEHEADING),     WID_PARA_HEAD,  CPPU_E2T(CPPUTYPE_OUSTRING)  , 0,     0},
                    { SW_PROP_NMID(UNO_NAME_PARA_STYLELEVEL1),  WID_PARA_LEV1,  CPPU_E2T(CPPUTYPE_OUSTRING)  , 0,     0},
                    { SW_PROP_NMID(UNO_NAME_IS_RELATIVE_TABSTOPS), WID_IS_RELATIVE_TABSTOPS, CPPU_E2T(CPPUTYPE_BOOLEAN), PROPERTY_NONE, 0},
                    {0,0,0,0,0,0}
                };
                aMapEntriesArr[nPropertyId] = aTOXTablesMap_Impl;
            }
            break;
            case PROPERTY_MAP_INDEX_OBJECTS:
            {
                static SfxItemPropertyMapEntry aTOXObjectsMap_Impl[] =
                {
                    _BASE_INDEX_PROPERTIES_
                    { SW_PROP_NMID(UNO_NAME_CREATE_FROM_CHAPTER), WID_CREATE_FROM_CHAPTER                 ,  CPPU_E2T(CPPUTYPE_BOOLEAN)  , PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_IS_PROTECTED), WID_PROTECTED                           ,  CPPU_E2T(CPPUTYPE_BOOLEAN)  , PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_USE_ALPHABETICAL_SEPARATORS), WID_USE_ALPHABETICAL_SEPARATORS         ,  CPPU_E2T(CPPUTYPE_BOOLEAN)  , PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_LEVEL_FORMAT), WID_LEVEL_FORMAT                        ,  CPPU_E2T(CPPUTYPE_REFIDXREPL)  , PROPERTY_NONE,0},
                    { SW_PROP_NMID(UNO_NAME_CREATE_FROM_STAR_MATH), WID_CREATE_FROM_STAR_MATH               ,  CPPU_E2T(CPPUTYPE_BOOLEAN)  , PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_CREATE_FROM_STAR_CHART), WID_CREATE_FROM_STAR_CHART              ,  CPPU_E2T(CPPUTYPE_BOOLEAN)  , PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_CREATE_FROM_STAR_CALC), WID_CREATE_FROM_STAR_CALC               ,  CPPU_E2T(CPPUTYPE_BOOLEAN)  , PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_CREATE_FROM_STAR_DRAW), WID_CREATE_FROM_STAR_DRAW               ,  CPPU_E2T(CPPUTYPE_BOOLEAN)  , PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_CREATE_FROM_OTHER_EMBEDDED_OBJECTS), WID_CREATE_FROM_OTHER_EMBEDDED_OBJECTS  ,  CPPU_E2T(CPPUTYPE_BOOLEAN)  , PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_TEXT_COLUMNS), RES_COL,                CPPU_E2T(CPPUTYPE_REFTEXTCOL),    PROPERTY_NONE, MID_COLUMNS},
                    { SW_PROP_NMID(UNO_NAME_BACK_GRAPHIC_URL), RES_BACKGROUND,      CPPU_E2T(CPPUTYPE_OUSTRING), PROPERTY_NONE ,MID_GRAPHIC_URL    },
                    { SW_PROP_NMID(UNO_NAME_BACK_GRAPHIC_FILTER), RES_BACKGROUND,       CPPU_E2T(CPPUTYPE_OUSTRING), PROPERTY_NONE ,MID_GRAPHIC_FILTER    },
                    { SW_PROP_NMID(UNO_NAME_BACK_GRAPHIC_LOCATION), RES_BACKGROUND,         CPPU_E2T(CPPUTYPE_GRAPHICLOC),          PROPERTY_NONE ,MID_GRAPHIC_POSITION},
                    { SW_PROP_NMID(UNO_NAME_BACK_COLOR), RES_BACKGROUND,            CPPU_E2T(CPPUTYPE_INT32),           PROPERTY_NONE ,MID_BACK_COLOR        },
                    { SW_PROP_NMID(UNO_NAME_BACK_TRANSPARENT), RES_BACKGROUND,      CPPU_E2T(CPPUTYPE_BOOLEAN),         PROPERTY_NONE ,MID_GRAPHIC_TRANSPARENT       },
                    { SW_PROP_NMID(UNO_NAME_PARA_STYLEHEADING),     WID_PARA_HEAD,  CPPU_E2T(CPPUTYPE_OUSTRING)  , 0,     0},
                    { SW_PROP_NMID(UNO_NAME_PARA_STYLELEVEL1),  WID_PARA_LEV1,  CPPU_E2T(CPPUTYPE_OUSTRING)  , 0,     0},
                    { SW_PROP_NMID(UNO_NAME_IS_RELATIVE_TABSTOPS), WID_IS_RELATIVE_TABSTOPS, CPPU_E2T(CPPUTYPE_BOOLEAN), PROPERTY_NONE, 0},
                    {0,0,0,0,0,0}
                };
                aMapEntriesArr[nPropertyId] = aTOXObjectsMap_Impl;
            }
            break;
            case PROPERTY_MAP_INDEX_ILLUSTRATIONS:
            {
                static SfxItemPropertyMapEntry aTOXIllustrationsMap_Impl[] =
                {
                    _BASE_INDEX_PROPERTIES_
                    { SW_PROP_NMID(UNO_NAME_CREATE_FROM_CHAPTER), WID_CREATE_FROM_CHAPTER                 ,  CPPU_E2T(CPPUTYPE_BOOLEAN)  , PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_CREATE_FROM_LABELS), WID_CREATE_FROM_LABELS                  ,  CPPU_E2T(CPPUTYPE_BOOLEAN)  , PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_IS_PROTECTED), WID_PROTECTED                           ,  CPPU_E2T(CPPUTYPE_BOOLEAN)  , PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_USE_ALPHABETICAL_SEPARATORS), WID_USE_ALPHABETICAL_SEPARATORS         ,  CPPU_E2T(CPPUTYPE_BOOLEAN)  , PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_LABEL_CATEGORY), WID_LABEL_CATEGORY                      ,  CPPU_E2T(CPPUTYPE_OUSTRING)  , PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_LABEL_DISPLAY_TYPE), WID_LABEL_DISPLAY_TYPE                  ,  CPPU_E2T(CPPUTYPE_INT16)  , PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_LEVEL_FORMAT), WID_LEVEL_FORMAT                        ,  CPPU_E2T(CPPUTYPE_REFIDXREPL)  , PROPERTY_NONE,0},
                    { SW_PROP_NMID(UNO_NAME_TEXT_COLUMNS), RES_COL,                CPPU_E2T(CPPUTYPE_REFTEXTCOL),    PROPERTY_NONE, MID_COLUMNS},
                    { SW_PROP_NMID(UNO_NAME_BACK_GRAPHIC_URL), RES_BACKGROUND,      CPPU_E2T(CPPUTYPE_OUSTRING), PROPERTY_NONE ,MID_GRAPHIC_URL    },
                    { SW_PROP_NMID(UNO_NAME_BACK_GRAPHIC_FILTER), RES_BACKGROUND,       CPPU_E2T(CPPUTYPE_OUSTRING), PROPERTY_NONE ,MID_GRAPHIC_FILTER    },
                    { SW_PROP_NMID(UNO_NAME_BACK_GRAPHIC_LOCATION), RES_BACKGROUND,         CPPU_E2T(CPPUTYPE_GRAPHICLOC),          PROPERTY_NONE ,MID_GRAPHIC_POSITION},
                    { SW_PROP_NMID(UNO_NAME_BACK_COLOR), RES_BACKGROUND,            CPPU_E2T(CPPUTYPE_INT32),           PROPERTY_NONE ,MID_BACK_COLOR        },
                    { SW_PROP_NMID(UNO_NAME_BACK_TRANSPARENT), RES_BACKGROUND,      CPPU_E2T(CPPUTYPE_BOOLEAN),         PROPERTY_NONE ,MID_GRAPHIC_TRANSPARENT       },
                    { SW_PROP_NMID(UNO_NAME_PARA_STYLEHEADING),     WID_PARA_HEAD,  CPPU_E2T(CPPUTYPE_OUSTRING)  , 0,     0},
                    { SW_PROP_NMID(UNO_NAME_PARA_STYLELEVEL1),  WID_PARA_LEV1,  CPPU_E2T(CPPUTYPE_OUSTRING)  , 0,     0},
                    { SW_PROP_NMID(UNO_NAME_IS_RELATIVE_TABSTOPS), WID_IS_RELATIVE_TABSTOPS, CPPU_E2T(CPPUTYPE_BOOLEAN), PROPERTY_NONE, 0},
                    {0,0,0,0,0,0}
                };
                aMapEntriesArr[nPropertyId] = aTOXIllustrationsMap_Impl;
            }
            break;
            case PROPERTY_MAP_TEXT_TABLE_ROW:
            {
                static SfxItemPropertyMapEntry aTableRowPropertyMap_Impl[] =
                {
                    { SW_PROP_NMID(UNO_NAME_BACK_COLOR), RES_BACKGROUND, CPPU_E2T(CPPUTYPE_INT32), PROPERTY_NONE ,MID_BACK_COLOR         },
                    { SW_PROP_NMID(UNO_NAME_BACK_GRAPHIC_URL), RES_BACKGROUND,      CPPU_E2T(CPPUTYPE_OUSTRING), PROPERTY_NONE ,MID_GRAPHIC_URL    },
                    { SW_PROP_NMID(UNO_NAME_BACK_GRAPHIC_FILTER), RES_BACKGROUND,       CPPU_E2T(CPPUTYPE_OUSTRING), PROPERTY_NONE ,MID_GRAPHIC_FILTER    },
                    { SW_PROP_NMID(UNO_NAME_BACK_GRAPHIC_LOCATION), RES_BACKGROUND,         CPPU_E2T(CPPUTYPE_GRAPHICLOC),          PROPERTY_NONE ,MID_GRAPHIC_POSITION},
                    { SW_PROP_NMID(UNO_NAME_BACK_TRANSPARENT), RES_BACKGROUND,      CPPU_E2T(CPPUTYPE_BOOLEAN),         PROPERTY_NONE ,MID_GRAPHIC_TRANSPARENT       },
                    { SW_PROP_NMID(UNO_NAME_TABLE_COLUMN_SEPARATORS), FN_UNO_TABLE_COLUMN_SEPARATORS,   CPPU_E2T(CPPUTYPE_TBLCOLSEP),   PropertyAttribute::MAYBEVOID, 0 },
                    { SW_PROP_NMID(UNO_NAME_HEIGHT), FN_UNO_ROW_HEIGHT,     CPPU_E2T(CPPUTYPE_INT32),           PROPERTY_NONE ,CONVERT_TWIPS },
                    { SW_PROP_NMID(UNO_NAME_IS_AUTO_HEIGHT), FN_UNO_ROW_AUTO_HEIGHT,    CPPU_E2T(CPPUTYPE_BOOLEAN),         PROPERTY_NONE , 0 },
                    { SW_PROP_NMID(UNO_NAME_SIZE_TYPE), RES_FRM_SIZE,           CPPU_E2T(CPPUTYPE_INT16)  ,         PROPERTY_NONE,   MID_FRMSIZE_SIZE_TYPE  },
                    { SW_PROP_NMID(UNO_NAME_WIDTH_TYPE), RES_FRM_SIZE,          CPPU_E2T(CPPUTYPE_INT16)  ,         PROPERTY_NONE,   MID_FRMSIZE_WIDTH_TYPE },
                    { SW_PROP_NMID(UNO_NAME_IS_SPLIT_ALLOWED), RES_ROW_SPLIT,       CPPU_E2T(CPPUTYPE_BOOLEAN)  , PropertyAttribute::MAYBEVOID, 0},
                    {0,0,0,0,0,0}
                };

                aMapEntriesArr[nPropertyId] = (SfxItemPropertyMapEntry*)aTableRowPropertyMap_Impl;
            }
            break;
            case PROPERTY_MAP_TEXT_TABLE_CURSOR:
            {
                // das PropertySet entspricht dem Range ohne Chart-Properties
                static SfxItemPropertyMapEntry aTableCursorPropertyMap_Impl [] =
                {
                    COMMON_CRSR_PARA_PROPERTIES_WITHOUT_FN_01
                    TABSTOPS_MAP_ENTRY

                    // attributes from PROPERTY_MAP_TABLE_CELL:
                    { SW_PROP_NMID(UNO_NAME_BACK_COLOR), RES_BACKGROUND,    CPPU_E2T(CPPUTYPE_INT32),           PROPERTY_NONE , MID_BACK_COLOR       },
                    { SW_PROP_NMID(UNO_NAME_BACK_GRAPHIC_URL), RES_BACKGROUND,      CPPU_E2T(CPPUTYPE_OUSTRING), PROPERTY_NONE ,MID_GRAPHIC_URL    },
                    { SW_PROP_NMID(UNO_NAME_BACK_GRAPHIC_FILTER), RES_BACKGROUND,       CPPU_E2T(CPPUTYPE_OUSTRING), PROPERTY_NONE ,MID_GRAPHIC_FILTER    },
                    { SW_PROP_NMID(UNO_NAME_BACK_GRAPHIC_LOCATION), RES_BACKGROUND,         CPPU_E2T(CPPUTYPE_GRAPHICLOC), PROPERTY_NONE ,MID_GRAPHIC_POSITION},
                    { SW_PROP_NMID(UNO_NAME_NUMBER_FORMAT), RES_BOXATR_FORMAT,  CPPU_E2T(CPPUTYPE_INT32),           PropertyAttribute::MAYBEVOID ,0             },
                    { SW_PROP_NMID(UNO_NAME_BACK_TRANSPARENT), RES_BACKGROUND,  CPPU_E2T(CPPUTYPE_BOOLEAN),         PROPERTY_NONE , MID_GRAPHIC_TRANSPARENT      },
                    { SW_PROP_NMID(UNO_NAME_USER_DEFINED_ATTRIBUTES), RES_UNKNOWNATR_CONTAINER, CPPU_E2T(CPPUTYPE_REFNAMECNT), PropertyAttribute::MAYBEVOID, 0 },
                    { SW_PROP_NMID(UNO_NAME_TEXT_SECTION), FN_UNO_TEXT_SECTION, CPPU_E2T(CPPUTYPE_REFTEXTSECTION),  PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY ,0 },
                    { SW_PROP_NMID(UNO_NAME_IS_PROTECTED), RES_PROTECT,            CPPU_E2T(CPPUTYPE_BOOLEAN), 0, MID_PROTECT_CONTENT},
                    { SW_PROP_NMID(UNO_NAME_VERT_ORIENT), RES_VERT_ORIENT,      CPPU_E2T(CPPUTYPE_INT16),           PROPERTY_NONE ,MID_VERTORIENT_ORIENT    },
                    {0,0,0,0,0,0}
                };
                aMapEntriesArr[nPropertyId] = aTableCursorPropertyMap_Impl;
            }
            break;
            case PROPERTY_MAP_BOOKMARK:
            {
                static SfxItemPropertyMapEntry aBookmarkPropertyMap_Impl [] =
                {
                    { SW_PROP_NMID(UNO_LINK_DISPLAY_NAME), FN_PARAM_LINK_DISPLAY_NAME,  CPPU_E2T(CPPUTYPE_OUSTRING), PropertyAttribute::READONLY, 0xff},
                    COMMON_TEXT_CONTENT_PROPERTIES
                    {0,0,0,0,0,0}
                };
                aMapEntriesArr[nPropertyId] = aBookmarkPropertyMap_Impl;
            }
            break;
            case PROPERTY_MAP_PARAGRAPH_EXTENSIONS:
            {
                static SfxItemPropertyMapEntry aParagraphExtensionsMap_Impl[] =
                {
                    COMMON_TEXT_CONTENT_PROPERTIES
                    {0,0,0,0,0,0}
                };

                aMapEntriesArr[nPropertyId] = aParagraphExtensionsMap_Impl;
            }
            break;
            case PROPERTY_MAP_BIBLIOGRAPHY :
            {
                static SfxItemPropertyMapEntry aBibliographyMap_Impl[] =
                {
                    _BASE_INDEX_PROPERTIES_
                    { SW_PROP_NMID(UNO_NAME_IS_PROTECTED), WID_PROTECTED                           ,  CPPU_E2T(CPPUTYPE_BOOLEAN)  , PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_TEXT_COLUMNS), RES_COL,                CPPU_E2T(CPPUTYPE_REFTEXTCOL),    PROPERTY_NONE, MID_COLUMNS},
                    { SW_PROP_NMID(UNO_NAME_BACK_GRAPHIC_URL), RES_BACKGROUND,      CPPU_E2T(CPPUTYPE_OUSTRING), PROPERTY_NONE ,MID_GRAPHIC_URL    },
                    { SW_PROP_NMID(UNO_NAME_BACK_GRAPHIC_FILTER), RES_BACKGROUND,       CPPU_E2T(CPPUTYPE_OUSTRING), PROPERTY_NONE ,MID_GRAPHIC_FILTER    },
                    { SW_PROP_NMID(UNO_NAME_BACK_GRAPHIC_LOCATION), RES_BACKGROUND,         CPPU_E2T(CPPUTYPE_GRAPHICLOC),          PROPERTY_NONE ,MID_GRAPHIC_POSITION},
                    { SW_PROP_NMID(UNO_NAME_BACK_COLOR), RES_BACKGROUND,            CPPU_E2T(CPPUTYPE_INT32),           PROPERTY_NONE ,MID_BACK_COLOR        },
                    { SW_PROP_NMID(UNO_NAME_BACK_TRANSPARENT), RES_BACKGROUND,      CPPU_E2T(CPPUTYPE_BOOLEAN),         PROPERTY_NONE ,MID_GRAPHIC_TRANSPARENT       },
                    { SW_PROP_NMID(UNO_NAME_PARA_STYLEHEADING),     WID_PARA_HEAD,  CPPU_E2T(CPPUTYPE_OUSTRING)  , 0,     0},
                    { SW_PROP_NMID(UNO_NAME_PARA_STYLELEVEL1),  WID_PARA_LEV1,  CPPU_E2T(CPPUTYPE_OUSTRING)  , 0,     0},
                    { SW_PROP_NMID(UNO_NAME_LEVEL_FORMAT), WID_LEVEL_FORMAT                        ,  CPPU_E2T(CPPUTYPE_REFIDXREPL)  , PROPERTY_NONE,0},
                    { SW_PROP_NMID(UNO_NAME_LOCALE),            WID_IDX_LOCALE,         CPPU_E2T(CPPUTYPE_LOCALE), PROPERTY_NONE,     0},\
                    { SW_PROP_NMID(UNO_NAME_SORT_ALGORITHM),    WID_IDX_SORT_ALGORITHM,  CPPU_E2T(CPPUTYPE_OUSTRING), PROPERTY_NONE,     0},\
                    {0,0,0,0,0,0}
                };
                aMapEntriesArr[nPropertyId] = aBibliographyMap_Impl;
            }
            break;
            case PROPERTY_MAP_TEXT_DOCUMENT:
            {
                static SfxItemPropertyMapEntry aDocMap_Impl[] =
                {
                    { SW_PROP_NMID(UNO_NAME_BASIC_LIBRARIES), WID_DOC_BASIC_LIBRARIES,  CPPU_E2T(CPPUTYPE_REFLIBCONTAINER), PropertyAttribute::READONLY, 0},
                    { SW_PROP_NMID(UNO_NAME_CHAR_FONT_NAME), RES_CHRATR_FONT,       CPPU_E2T(CPPUTYPE_OUSTRING),  PropertyAttribute::MAYBEVOID, MID_FONT_FAMILY_NAME },
                    { SW_PROP_NMID(UNO_NAME_CHAR_FONT_STYLE_NAME), RES_CHRATR_FONT,     CPPU_E2T(CPPUTYPE_OUSTRING), PropertyAttribute::MAYBEVOID, MID_FONT_STYLE_NAME },
                    { SW_PROP_NMID(UNO_NAME_CHAR_FONT_FAMILY), RES_CHRATR_FONT,     CPPU_E2T(CPPUTYPE_INT16),                   PropertyAttribute::MAYBEVOID, MID_FONT_FAMILY   },
                    { SW_PROP_NMID(UNO_NAME_CHAR_FONT_CHAR_SET), RES_CHRATR_FONT,       CPPU_E2T(CPPUTYPE_INT16),   PropertyAttribute::MAYBEVOID, MID_FONT_CHAR_SET },
                    { SW_PROP_NMID(UNO_NAME_CHAR_FONT_PITCH), RES_CHRATR_FONT,      CPPU_E2T(CPPUTYPE_INT16),                   PropertyAttribute::MAYBEVOID, MID_FONT_PITCH   },
                    { SW_PROP_NMID(UNO_NAME_CHAR_FONT_NAME_ASIAN), RES_CHRATR_CJK_FONT,     CPPU_E2T(CPPUTYPE_OUSTRING),  PropertyAttribute::MAYBEVOID, MID_FONT_FAMILY_NAME },
                    { SW_PROP_NMID(UNO_NAME_CHAR_FONT_STYLE_NAME_ASIAN), RES_CHRATR_CJK_FONT,   CPPU_E2T(CPPUTYPE_OUSTRING),    PropertyAttribute::MAYBEVOID, MID_FONT_STYLE_NAME },
                    { SW_PROP_NMID(UNO_NAME_CHAR_FONT_FAMILY_ASIAN), RES_CHRATR_CJK_FONT,   CPPU_E2T(CPPUTYPE_INT16),   PropertyAttribute::MAYBEVOID, MID_FONT_FAMILY   },
                    { SW_PROP_NMID(UNO_NAME_CHAR_FONT_CHAR_SET_ASIAN), RES_CHRATR_CJK_FONT, CPPU_E2T(CPPUTYPE_INT16),   PropertyAttribute::MAYBEVOID, MID_FONT_CHAR_SET },
                    { SW_PROP_NMID(UNO_NAME_CHAR_FONT_PITCH_ASIAN), RES_CHRATR_CJK_FONT,    CPPU_E2T(CPPUTYPE_INT16),   PropertyAttribute::MAYBEVOID, MID_FONT_PITCH   },
                    { SW_PROP_NMID(UNO_NAME_CHAR_FONT_NAME_COMPLEX), RES_CHRATR_CTL_FONT,   CPPU_E2T(CPPUTYPE_OUSTRING),    PropertyAttribute::MAYBEVOID, MID_FONT_FAMILY_NAME },
                    { SW_PROP_NMID(UNO_NAME_CHAR_FONT_STYLE_NAME_COMPLEX), RES_CHRATR_CTL_FONT, CPPU_E2T(CPPUTYPE_OUSTRING),    PropertyAttribute::MAYBEVOID, MID_FONT_STYLE_NAME },
                    { SW_PROP_NMID(UNO_NAME_CHAR_FONT_FAMILY_COMPLEX), RES_CHRATR_CTL_FONT, CPPU_E2T(CPPUTYPE_INT16),   PropertyAttribute::MAYBEVOID, MID_FONT_FAMILY   },
                    { SW_PROP_NMID(UNO_NAME_CHAR_FONT_CHAR_SET_COMPLEX), RES_CHRATR_CTL_FONT,   CPPU_E2T(CPPUTYPE_INT16),   PropertyAttribute::MAYBEVOID, MID_FONT_CHAR_SET },
                    { SW_PROP_NMID(UNO_NAME_CHAR_FONT_PITCH_COMPLEX), RES_CHRATR_CTL_FONT,  CPPU_E2T(CPPUTYPE_INT16),   PropertyAttribute::MAYBEVOID, MID_FONT_PITCH   },
                    { SW_PROP_NMID(UNO_NAME_CHAR_LOCALE), RES_CHRATR_LANGUAGE ,   CPPU_E2T(CPPUTYPE_LOCALE), PropertyAttribute::MAYBEVOID,  MID_LANG_LOCALE },
                    { SW_PROP_NMID(UNO_NAME_CHARACTER_COUNT), WID_DOC_CHAR_COUNT,           CPPU_E2T(CPPUTYPE_INT32),   PropertyAttribute::READONLY,   0},
                    { SW_PROP_NMID(UNO_NAME_DIALOG_LIBRARIES), WID_DOC_DIALOG_LIBRARIES,  CPPU_E2T(CPPUTYPE_REFLIBCONTAINER), PropertyAttribute::READONLY, 0},
                    { SW_PROP_NMID(UNO_NAME_VBA_DOCOBJ), WID_DOC_VBA_DOCOBJ,  CPPU_E2T(CPPUTYPE_PROPERTYVALUE), PropertyAttribute::READONLY, 0},
                    { SW_PROP_NMID(UNO_NAME_INDEX_AUTO_MARK_FILE_U_R_L), WID_DOC_AUTO_MARK_URL, CPPU_E2T(CPPUTYPE_OUSTRING),    PROPERTY_NONE,   0},
                    { SW_PROP_NMID(UNO_NAME_PARAGRAPH_COUNT), WID_DOC_PARA_COUNT,           CPPU_E2T(CPPUTYPE_INT32),   PropertyAttribute::READONLY,   0},
                    { SW_PROP_NMID(UNO_NAME_RECORD_CHANGES), WID_DOC_CHANGES_RECORD,        CPPU_E2T(CPPUTYPE_BOOLEAN), PROPERTY_NONE,   0},
                    { SW_PROP_NMID(UNO_NAME_SHOW_CHANGES), WID_DOC_CHANGES_SHOW,        CPPU_E2T(CPPUTYPE_BOOLEAN), PROPERTY_NONE,   0},
                    { SW_PROP_NMID(UNO_NAME_WORD_COUNT), WID_DOC_WORD_COUNT,            CPPU_E2T(CPPUTYPE_INT32),   PropertyAttribute::READONLY,   0},
                    { SW_PROP_NMID(UNO_NAME_IS_TEMPLATE), WID_DOC_ISTEMPLATEID,         CPPU_E2T(CPPUTYPE_BOOLEAN), PropertyAttribute::READONLY,   0},
                    { SW_PROP_NMID(UNO_NAME_WORD_SEPARATOR), WID_DOC_WORD_SEPARATOR,        CPPU_E2T(CPPUTYPE_OUSTRING),    PROPERTY_NONE,   0},
                    { SW_PROP_NMID(UNO_NAME_HIDE_FIELD_TIPS), WID_DOC_HIDE_TIPS,            CPPU_E2T(CPPUTYPE_BOOLEAN), PROPERTY_NONE,   0},
                    { SW_PROP_NMID(UNO_NAME_REDLINE_DISPLAY_TYPE), WID_DOC_REDLINE_DISPLAY,     CPPU_E2T(CPPUTYPE_INT16),   PROPERTY_NONE,   0},
                    { SW_PROP_NMID(UNO_NAME_REDLINE_PROTECTION_KEY), WID_DOC_CHANGES_PASSWORD,      CPPU_E2T(CPPUTYPE_SEQINT8), PROPERTY_NONE, 0 },
                    { SW_PROP_NMID(UNO_NAME_FORBIDDEN_CHARACTERS), WID_DOC_FORBIDDEN_CHARS,    CPPU_E2T(CPPUTYPE_REFFORBCHARS), PROPERTY_NONE,   0},
                    { SW_PROP_NMID(UNO_NAME_TWO_DIGIT_YEAR), WID_DOC_TWO_DIGIT_YEAR,    CPPU_E2T(CPPUTYPE_INT16),   PROPERTY_NONE,   0},
                    { SW_PROP_NMID(UNO_NAME_AUTOMATIC_CONTROL_FOCUS),       WID_DOC_AUTOMATIC_CONTROL_FOCUS,    CPPU_E2T(CPPUTYPE_BOOLEAN), PROPERTY_NONE,   0},
                    { SW_PROP_NMID(UNO_NAME_APPLY_FORM_DESIGN_MODE),        WID_DOC_APPLY_FORM_DESIGN_MODE,     CPPU_E2T(CPPUTYPE_BOOLEAN), PROPERTY_NONE,   0},
                    { SW_PROP_NMID(UNO_NAME_RUNTIME_UID), WID_DOC_RUNTIME_UID,  CPPU_E2T(CPPUTYPE_OUSTRING), PropertyAttribute::READONLY, 0},
                    { SW_PROP_NMID(UNO_NAME_LOCK_UPDATES),        WID_DOC_LOCK_UPDATES,     CPPU_E2T(CPPUTYPE_BOOLEAN), PROPERTY_NONE,   0},
                    { SW_PROP_NMID(UNO_NAME_HAS_VALID_SIGNATURES),  WID_DOC_HAS_VALID_SIGNATURES, CPPU_E2T(CPPUTYPE_BOOLEAN), PropertyAttribute::READONLY,   0},
                    { SW_PROP_NMID(UNO_NAME_BUILDID), WID_DOC_BUILDID, CPPU_E2T(CPPUTYPE_OUSTRING), 0, 0},
                    { SW_PROP_NMID(UNO_NAME_DEFAULT_PAGE_MODE),  WID_DOC_DEFAULT_PAGE_MODE,  CPPU_E2T(CPPUTYPE_BOOLEAN), PROPERTY_NONE,  0},
                    {0,0,0,0,0,0}
                };
                aMapEntriesArr[nPropertyId] = aDocMap_Impl;
            }
            break;
            case PROPERTY_MAP_LINK_TARGET:
            {
                static SfxItemPropertyMapEntry aLinkTargetMap_Impl[] =
                {
                    { SW_PROP_NMID(UNO_LINK_DISPLAY_BITMAP), 0,     CPPU_E2T(CPPUTYPE_REFBITMAP), PropertyAttribute::READONLY, 0xff},
                    { SW_PROP_NMID(UNO_LINK_DISPLAY_NAME), 0,   CPPU_E2T(CPPUTYPE_OUSTRING), PropertyAttribute::READONLY, 0xff},
                    {0,0,0,0,0,0}
                };
                aMapEntriesArr[nPropertyId] = aLinkTargetMap_Impl;
            }
            break;
            case PROPERTY_MAP_AUTO_TEXT_GROUP :
            {
                static SfxItemPropertyMapEntry aAutoTextGroupMap_Impl[] =
                {
                    { SW_PROP_NMID(UNO_NAME_FILE_PATH), WID_GROUP_PATH,     CPPU_E2T(CPPUTYPE_OUSTRING),    PROPERTY_NONE,   PropertyAttribute::READONLY},
                    { SW_PROP_NMID(UNO_NAME_TITLE), WID_GROUP_TITLE, CPPU_E2T(CPPUTYPE_OUSTRING),   PROPERTY_NONE,   0},
                    {0,0,0,0,0,0}
                };
                aMapEntriesArr[nPropertyId] = aAutoTextGroupMap_Impl;
            }
            break;
            case PROPERTY_MAP_TEXTPORTION_EXTENSIONS:
            {
                static SfxItemPropertyMapEntry aTextPortionExtensionMap_Impl[] =
                {
                    COMPLETE_TEXT_CURSOR_MAP
                    {SW_PROP_NMID(UNO_NAME_BOOKMARK), FN_UNO_BOOKMARK, CPPU_E2T(CPPUTYPE_REFTEXTCNTNT),   PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY ,0 },
                    {SW_PROP_NMID(UNO_NAME_CONTROL_CHARACTER), FN_UNO_CONTROL_CHARACTER, CPPU_E2T(CPPUTYPE_INT16),                 PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY, MID_HYPHEN_MIN_LEAD   },
                    {SW_PROP_NMID(UNO_NAME_IS_COLLAPSED), FN_UNO_IS_COLLAPSED, CPPU_E2T(CPPUTYPE_BOOLEAN),                             PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY, 0 },
                    {SW_PROP_NMID(UNO_NAME_IS_START), FN_UNO_IS_START, CPPU_E2T(CPPUTYPE_BOOLEAN),                             PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY, 0 },
                    //_REDLINE_PROPERTIES
                    {SW_PROP_NMID(UNO_NAME_TEXT_PORTION_TYPE), FN_UNO_TEXT_PORTION_TYPE, CPPU_E2T(CPPUTYPE_OUSTRING),                        PropertyAttribute::READONLY, 0},
                    {SW_PROP_NMID(UNO_NAME_META), FN_UNO_META, CPPU_E2T(CPPUTYPE_REFTEXTCNTNT), PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY, 0 },
                    {0,0,0,0,0,0}
                };
                aMapEntriesArr[nPropertyId] = aTextPortionExtensionMap_Impl;
            }
            break;
            case PROPERTY_MAP_FOOTNOTE:
            {
                static SfxItemPropertyMapEntry aFootnoteMap_Impl[] =
                {
                    {SW_PROP_NMID(UNO_NAME_REFERENCE_ID), 0, CPPU_E2T(CPPUTYPE_INT16),PropertyAttribute::READONLY|PropertyAttribute::MAYBEVOID, 0},
                    COMMON_TEXT_CONTENT_PROPERTIES
                    _REDLINE_NODE_PROPERTIES
                    {0,0,0,0,0,0}
                };
                aMapEntriesArr[nPropertyId] = aFootnoteMap_Impl;
            }
            break;
            case PROPERTY_MAP_TEXT_COLUMS :
            {
                static SfxItemPropertyMapEntry aTextColumns_Impl[] =
                {
                    {SW_PROP_NMID(UNO_NAME_IS_AUTOMATIC), WID_TXTCOL_IS_AUTOMATIC, CPPU_E2T(CPPUTYPE_BOOLEAN),PropertyAttribute::READONLY, 0},
                    {SW_PROP_NMID(UNO_NAME_AUTOMATIC_DISTANCE), WID_TXTCOL_AUTO_DISTANCE, CPPU_E2T(CPPUTYPE_INT32),PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_SEPARATOR_LINE_WIDTH), WID_TXTCOL_LINE_WIDTH, CPPU_E2T(CPPUTYPE_INT32),PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_SEPARATOR_LINE_COLOR), WID_TXTCOL_LINE_COLOR, CPPU_E2T(CPPUTYPE_INT32),PROPERTY_NONE,    0},
                    {SW_PROP_NMID(UNO_NAME_SEPARATOR_LINE_RELATIVE_HEIGHT), WID_TXTCOL_LINE_REL_HGT, CPPU_E2T(CPPUTYPE_INT32),PROPERTY_NONE,    0},
                    {SW_PROP_NMID(UNO_NAME_SEPARATOR_LINE_VERTIVAL_ALIGNMENT), WID_TXTCOL_LINE_ALIGN, CPPU_E2T(CPPUTYPE_VERTALIGN),PROPERTY_NONE,   0},
                    {SW_PROP_NMID(UNO_NAME_SEPARATOR_LINE_IS_ON), WID_TXTCOL_LINE_IS_ON, CPPU_E2T(CPPUTYPE_BOOLEAN),PROPERTY_NONE,  0},
                    {SW_PROP_NMID(UNO_NAME_SEPARATOR_LINE_STYLE), WID_TXTCOL_LINE_STYLE, CPPU_E2T(CPPUTYPE_INT8),PROPERTY_NONE, 0},
                    {0,0,0,0,0,0}
                };
                aMapEntriesArr[nPropertyId] = aTextColumns_Impl;
            }
            break;
            case PROPERTY_MAP_REDLINE :
            {
                static SfxItemPropertyMapEntry aRedlineMap_Impl[] =
                {
                    _REDLINE_PROPERTIES
                    _REDLINE_NODE_PROPERTIES
                    {SW_PROP_NMID(UNO_NAME_REDLINE_START), 0, CPPU_E2T(CPPUTYPE_REFINTERFACE),  PropertyAttribute::READONLY,    0},
                    {SW_PROP_NMID(UNO_NAME_REDLINE_END), 0, CPPU_E2T(CPPUTYPE_REFINTERFACE),    PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY,   0},
                    {0,0,0,0,0,0}
                };
                aMapEntriesArr[nPropertyId] = aRedlineMap_Impl;
            }
            break;
            case PROPERTY_MAP_TEXT_DEFAULT :
            {
                static SfxItemPropertyMapEntry aTextDefaultMap_Impl[] =
                {
                    { SW_PROP_NMID(UNO_NAME_TAB_STOP_DISTANCE), RES_PARATR_TABSTOP,     CPPU_E2T(CPPUTYPE_INT32),   PROPERTY_NONE, MID_STD_TAB | CONVERT_TWIPS},
                    COMMON_CRSR_PARA_PROPERTIES_WITHOUT_FN
                    COMMON_HYPERLINK_PROPERTIES
                    { SW_PROP_NMID(UNO_NAME_CHAR_STYLE_NAME), RES_TXTATR_CHARFMT,     CPPU_E2T(CPPUTYPE_OUSTRING),  PropertyAttribute::MAYBEVOID,     0},
                    { SW_PROP_NMID(UNO_NAME_IS_SPLIT_ALLOWED), RES_ROW_SPLIT,       CPPU_E2T(CPPUTYPE_BOOLEAN)  , PropertyAttribute::MAYBEVOID, 0},
                    // #i29550#
                    { SW_PROP_NMID(UNO_NAME_COLLAPSING_BORDERS), RES_COLLAPSING_BORDERS, CPPU_E2T(CPPUTYPE_BOOLEAN), PROPERTY_NONE, 0},

            //text grid enhancement for better CJK support.  2007-04-01
            //just export the default page mode property, other properties are not handled in this version
                    { SW_PROP_NMID(UNO_NAME_GRID_STANDARD_PAGE_MODE), RES_TEXTGRID, CPPU_E2T(CPPUTYPE_BOOLEAN), PROPERTY_NONE, MID_GRID_STANDARD_MODE},
                    {0,0,0,0,0,0}
                };
                aMapEntriesArr[nPropertyId] = aTextDefaultMap_Impl;
                for( SfxItemPropertyMapEntry * pMap = aTextDefaultMap_Impl;
                        pMap->pName; ++pMap )
                {
                    // UNO_NAME_PAGE_DESC_NAME should keep its MAYBEVOID flag
                    if (!(RES_PAGEDESC == pMap->nWID && MID_PAGEDESC_PAGEDESCNAME == pMap->nMemberId))
                        pMap->nFlags &= ~PropertyAttribute::MAYBEVOID;
                }
            }
            break;
            case PROPERTY_MAP_REDLINE_PORTION :
            {
                static SfxItemPropertyMapEntry aRedlinePortionMap_Impl[] =
                {
                    COMPLETE_TEXT_CURSOR_MAP
                    {SW_PROP_NMID(UNO_NAME_BOOKMARK), FN_UNO_BOOKMARK, CPPU_E2T(CPPUTYPE_REFTEXTCNTNT),   PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY ,0 },
                    {SW_PROP_NMID(UNO_NAME_CONTROL_CHARACTER), FN_UNO_CONTROL_CHARACTER, CPPU_E2T(CPPUTYPE_INT16),                 PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY, MID_HYPHEN_MIN_LEAD   },
                    {SW_PROP_NMID(UNO_NAME_IS_COLLAPSED), FN_UNO_IS_COLLAPSED, CPPU_E2T(CPPUTYPE_BOOLEAN),                             PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY, 0 },
                    {SW_PROP_NMID(UNO_NAME_IS_START), FN_UNO_IS_START, CPPU_E2T(CPPUTYPE_BOOLEAN),                             PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY, 0 },
                    _REDLINE_PROPERTIES
                    {SW_PROP_NMID(UNO_NAME_TEXT_PORTION_TYPE), FN_UNO_TEXT_PORTION_TYPE, CPPU_E2T(CPPUTYPE_OUSTRING),                        PropertyAttribute::READONLY, 0},
                    {0,0,0,0,0,0}
                };
                aMapEntriesArr[nPropertyId] = aRedlinePortionMap_Impl;
            }
            break;
            case  PROPERTY_MAP_FLDTYP_DATETIME:
            {
                static SfxItemPropertyMapEntry aDateTimeFieldPropMap[] =
                {
                    {SW_PROP_NMID(UNO_NAME_ADJUST), FIELD_PROP_SUBTYPE,     CPPU_E2T(CPPUTYPE_INT32), PROPERTY_NONE,    0},
                    {SW_PROP_NMID(UNO_NAME_DATE_TIME_VALUE), FIELD_PROP_DATE_TIME,  CPPU_E2T(CPPUTYPE_DATETIME), PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_IS_FIXED),       FIELD_PROP_BOOL1,   CPPU_E2T(CPPUTYPE_BOOLEAN)  , PROPERTY_NONE,0},
                    {SW_PROP_NMID(UNO_NAME_IS_DATE),    FIELD_PROP_BOOL2,   CPPU_E2T(CPPUTYPE_BOOLEAN)  , PROPERTY_NONE,0},
                    {SW_PROP_NMID(UNO_NAME_NUMBER_FORMAT), FIELD_PROP_FORMAT,   CPPU_E2T(CPPUTYPE_INT32), PROPERTY_NONE,    0},
                    {SW_PROP_NMID(UNO_NAME_IS_FIXED_LANGUAGE), FIELD_PROP_BOOL4, CPPU_E2T(CPPUTYPE_BOOLEAN), PROPERTY_NONE,    0},
                    COMMON_FLDTYP_PROPERTIES
                    {0,0,0,0,0,0}
                };
                aMapEntriesArr[nPropertyId] = aDateTimeFieldPropMap;
            }
            break;
            case  PROPERTY_MAP_FLDTYP_USER     :
            {
                static SfxItemPropertyMapEntry aUserFieldPropMap[] =
                {
                    {SW_PROP_NMID(UNO_NAME_IS_SHOW_FORMULA), FIELD_PROP_BOOL2,  CPPU_E2T(CPPUTYPE_BOOLEAN), PROPERTY_NONE,  0},
                    {SW_PROP_NMID(UNO_NAME_IS_VISIBLE),     FIELD_PROP_BOOL1,   CPPU_E2T(CPPUTYPE_BOOLEAN), PROPERTY_NONE,  0},
                    {SW_PROP_NMID(UNO_NAME_NUMBER_FORMAT),  FIELD_PROP_FORMAT,  CPPU_E2T(CPPUTYPE_INT32), PROPERTY_NONE,    0},
                    {SW_PROP_NMID(UNO_NAME_IS_FIXED_LANGUAGE), FIELD_PROP_BOOL4, CPPU_E2T(CPPUTYPE_BOOLEAN), PROPERTY_NONE,    0},
                    COMMON_FLDTYP_PROPERTIES
                    {0,0,0,0,0,0}
                };

                aMapEntriesArr[nPropertyId] = aUserFieldPropMap;
            }
            break;
            case  PROPERTY_MAP_FLDTYP_SET_EXP  :
            {
                static SfxItemPropertyMapEntry aSetExpFieldPropMap     [] =
                {
                    {SW_PROP_NMID(UNO_NAME_CONTENT),            FIELD_PROP_PAR2,    CPPU_E2T(CPPUTYPE_OUSTRING),   PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_CURRENT_PRESENTATION), FIELD_PROP_PAR4, CPPU_E2T(CPPUTYPE_OUSTRING),  PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_HINT),               FIELD_PROP_PAR3, CPPU_E2T(CPPUTYPE_OUSTRING),   PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_NUMBER_FORMAT),      FIELD_PROP_FORMAT,  CPPU_E2T(CPPUTYPE_INT32), PROPERTY_NONE,    0},
                    {SW_PROP_NMID(UNO_NAME_NUMBERING_TYPE),     FIELD_PROP_USHORT2, CPPU_E2T(CPPUTYPE_INT16), PROPERTY_NONE,    0},
                    {SW_PROP_NMID(UNO_NAME_IS_INPUT),       FIELD_PROP_BOOL1,   CPPU_E2T(CPPUTYPE_BOOLEAN), PROPERTY_NONE, 0},
                    // #i69733# wrong name - UNO_NAME_IS_INPUT expanded to "Input" instead of "IsInput"
                    {SW_PROP_NMID(UNO_NAME_INPUT),          FIELD_PROP_BOOL1,   CPPU_E2T(CPPUTYPE_BOOLEAN), PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_IS_SHOW_FORMULA), FIELD_PROP_BOOL3,  CPPU_E2T(CPPUTYPE_BOOLEAN), PROPERTY_NONE,  0},
                    {SW_PROP_NMID(UNO_NAME_IS_VISIBLE),       FIELD_PROP_BOOL2,   CPPU_E2T(CPPUTYPE_BOOLEAN),    PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_SEQUENCE_VALUE), FIELD_PROP_USHORT1, CPPU_E2T(CPPUTYPE_INT16),   PROPERTY_NONE,  0},
                    {SW_PROP_NMID(UNO_NAME_SUB_TYPE),           FIELD_PROP_SUBTYPE, CPPU_E2T(CPPUTYPE_INT16), PROPERTY_NONE,    0},
                    {SW_PROP_NMID(UNO_NAME_VALUE),          FIELD_PROP_DOUBLE,  CPPU_E2T(CPPUTYPE_DOUBLE),  PROPERTY_NONE,  0},
                    {SW_PROP_NMID(UNO_NAME_VARIABLE_NAME),  FIELD_PROP_PAR1,    CPPU_E2T(CPPUTYPE_OUSTRING),   PropertyAttribute::READONLY, 0},
                    {SW_PROP_NMID(UNO_NAME_IS_FIXED_LANGUAGE), FIELD_PROP_BOOL4, CPPU_E2T(CPPUTYPE_BOOLEAN), PROPERTY_NONE,    0},
                    COMMON_FLDTYP_PROPERTIES
                    {0,0,0,0,0,0}
                };
                aMapEntriesArr[nPropertyId] = aSetExpFieldPropMap;
            }
            break;
            case  PROPERTY_MAP_FLDTYP_GET_EXP  :
            {
                static SfxItemPropertyMapEntry aGetExpFieldPropMap     [] =
                {
                    {SW_PROP_NMID(UNO_NAME_CONTENT),            FIELD_PROP_PAR1,    CPPU_E2T(CPPUTYPE_OUSTRING),   PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_CURRENT_PRESENTATION), FIELD_PROP_PAR4, CPPU_E2T(CPPUTYPE_OUSTRING),  PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_IS_SHOW_FORMULA), FIELD_PROP_BOOL2,  CPPU_E2T(CPPUTYPE_BOOLEAN), PROPERTY_NONE,  0},
                    {SW_PROP_NMID(UNO_NAME_NUMBER_FORMAT),  FIELD_PROP_FORMAT,  CPPU_E2T(CPPUTYPE_INT32), PROPERTY_NONE,    0},
                    {SW_PROP_NMID(UNO_NAME_SUB_TYPE),           FIELD_PROP_SUBTYPE, CPPU_E2T(CPPUTYPE_INT16), PROPERTY_NONE,    0},
                    {SW_PROP_NMID(UNO_NAME_VALUE),          FIELD_PROP_DOUBLE,  CPPU_E2T(CPPUTYPE_DOUBLE), PropertyAttribute::READONLY, 0},
                    {SW_PROP_NMID(UNO_NAME_VARIABLE_SUBTYPE),   FIELD_PROP_USHORT1, CPPU_E2T(CPPUTYPE_INT16), PROPERTY_NONE,    0},
                    {SW_PROP_NMID(UNO_NAME_IS_FIXED_LANGUAGE), FIELD_PROP_BOOL4, CPPU_E2T(CPPUTYPE_BOOLEAN), PROPERTY_NONE,    0},
                    COMMON_FLDTYP_PROPERTIES
                    {0,0,0,0,0,0}
                };
                aMapEntriesArr[nPropertyId] = aGetExpFieldPropMap;
            }
            break;
            case  PROPERTY_MAP_FLDTYP_FILE_NAME:
            {
                static SfxItemPropertyMapEntry aFileNameFieldPropMap   [] =
                {
                    {SW_PROP_NMID(UNO_NAME_CURRENT_PRESENTATION), FIELD_PROP_PAR3, CPPU_E2T(CPPUTYPE_OUSTRING),  PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_FILE_FORMAT), FIELD_PROP_FORMAT, CPPU_E2T(CPPUTYPE_INT16), PROPERTY_NONE,    0},
                    {SW_PROP_NMID(UNO_NAME_IS_FIXED),   FIELD_PROP_BOOL2, CPPU_E2T(CPPUTYPE_BOOLEAN),       PROPERTY_NONE, 0},
                    COMMON_FLDTYP_PROPERTIES
                    {0,0,0,0,0,0}
                };
                aMapEntriesArr[nPropertyId] = aFileNameFieldPropMap;
            }
            break;
            case  PROPERTY_MAP_FLDTYP_PAGE_NUM :
            {
                static SfxItemPropertyMapEntry aPageNumFieldPropMap        [] =
                {
                    {SW_PROP_NMID(UNO_NAME_NUMBERING_TYPE),     FIELD_PROP_FORMAT,  CPPU_E2T(CPPUTYPE_INT16), PROPERTY_NONE,    0},
                    {SW_PROP_NMID(UNO_NAME_OFFSET),             FIELD_PROP_USHORT1, CPPU_E2T(CPPUTYPE_INT16),  PROPERTY_NONE,   0},
                    {SW_PROP_NMID(UNO_NAME_SUB_TYPE),           FIELD_PROP_SUBTYPE, CPPU_E2T(CPPUTYPE_PAGENUMTYPE), PROPERTY_NONE,  0},
                    {SW_PROP_NMID(UNO_NAME_USERTEXT),           FIELD_PROP_PAR1,    CPPU_E2T(CPPUTYPE_OUSTRING), PROPERTY_NONE, 0},
                    COMMON_FLDTYP_PROPERTIES
                    {0,0,0,0,0,0}
                };
                aMapEntriesArr[nPropertyId] = aPageNumFieldPropMap;
            }
            break;
            case  PROPERTY_MAP_FLDTYP_AUTHOR   :
            {
                static SfxItemPropertyMapEntry aAuthorFieldPropMap     [] =
                {
                    {SW_PROP_NMID(UNO_NAME_CONTENT),    FIELD_PROP_PAR1, CPPU_E2T(CPPUTYPE_OUSTRING),   PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_CURRENT_PRESENTATION), FIELD_PROP_PAR1, CPPU_E2T(CPPUTYPE_OUSTRING),  PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_IS_FIXED),   FIELD_PROP_BOOL2, CPPU_E2T(CPPUTYPE_BOOLEAN),       PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_FULL_NAME),FIELD_PROP_BOOL1, CPPU_E2T(CPPUTYPE_BOOLEAN),     PROPERTY_NONE, 0},
                    COMMON_FLDTYP_PROPERTIES
                    {0,0,0,0,0,0}
                };
                aMapEntriesArr[nPropertyId] = aAuthorFieldPropMap;
            }
            break;
            case  PROPERTY_MAP_FLDTYP_CHAPTER  :
            {
                static SfxItemPropertyMapEntry aChapterFieldPropMap        [] =
                {
                    {SW_PROP_NMID(UNO_NAME_CHAPTER_FORMAT),FIELD_PROP_USHORT1,  CPPU_E2T(CPPUTYPE_INT16),   PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_LEVEL        ),FIELD_PROP_BYTE1,         CPPU_E2T(CPPUTYPE_INT8),    PROPERTY_NONE, 0},
                    COMMON_FLDTYP_PROPERTIES
                    {0,0,0,0,0,0}
                };
                aMapEntriesArr[nPropertyId] = aChapterFieldPropMap;
            }
            break;
            case PROPERTY_MAP_FLDTYP_GET_REFERENCE          :
            {
                static SfxItemPropertyMapEntry aGetRefFieldPropMap     [] =
                {
                    {SW_PROP_NMID(UNO_NAME_CURRENT_PRESENTATION), FIELD_PROP_PAR3, CPPU_E2T(CPPUTYPE_OUSTRING),  PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_REFERENCE_FIELD_PART),FIELD_PROP_USHORT1, CPPU_E2T(CPPUTYPE_INT16),  PROPERTY_NONE,  0},
                    {SW_PROP_NMID(UNO_NAME_REFERENCE_FIELD_SOURCE),FIELD_PROP_USHORT2, CPPU_E2T(CPPUTYPE_INT16),    PROPERTY_NONE,  0},
                    {SW_PROP_NMID(UNO_NAME_SEQUENCE_NUMBER),    FIELD_PROP_SHORT1,  CPPU_E2T(CPPUTYPE_INT16),   PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_SOURCE_NAME),        FIELD_PROP_PAR1,    CPPU_E2T(CPPUTYPE_OUSTRING),   PROPERTY_NONE, 0},
                    COMMON_FLDTYP_PROPERTIES
                    {0,0,0,0,0,0}
                };
                aMapEntriesArr[nPropertyId] = aGetRefFieldPropMap;
            }
            break;
            case PROPERTY_MAP_FLDTYP_CONDITIONED_TEXT      :
            {
                static SfxItemPropertyMapEntry aConditionedTxtFieldPropMap [] =
                {
                    {SW_PROP_NMID(UNO_NAME_CONDITION),      FIELD_PROP_PAR1, CPPU_E2T(CPPUTYPE_OUSTRING),   PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_FALSE_CONTENT),  FIELD_PROP_PAR3, CPPU_E2T(CPPUTYPE_OUSTRING),   PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_IS_CONDITION_TRUE) ,  FIELD_PROP_BOOL1, CPPU_E2T(CPPUTYPE_BOOLEAN),   PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_TRUE_CONTENT) ,  FIELD_PROP_PAR2, CPPU_E2T(CPPUTYPE_OUSTRING),   PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_CURRENT_PRESENTATION), FIELD_PROP_PAR4, CPPU_E2T(CPPUTYPE_OUSTRING),  PROPERTY_NONE, 0},
                    COMMON_FLDTYP_PROPERTIES
                    {0,0,0,0,0,0}
                };
                aMapEntriesArr[nPropertyId] = aConditionedTxtFieldPropMap;
            }
            break;
            case PROPERTY_MAP_FLDTYP_HIDDEN_TEXT :
            {
                static SfxItemPropertyMapEntry aHiddenTxtFieldPropMap  [] =
                {
                    {SW_PROP_NMID(UNO_NAME_CONDITION),      FIELD_PROP_PAR1, CPPU_E2T(CPPUTYPE_OUSTRING),   PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_CONTENT) ,       FIELD_PROP_PAR2, CPPU_E2T(CPPUTYPE_OUSTRING),   PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_IS_HIDDEN) ,     FIELD_PROP_BOOL1, CPPU_E2T(CPPUTYPE_BOOLEAN),   PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_CURRENT_PRESENTATION), FIELD_PROP_PAR4, CPPU_E2T(CPPUTYPE_OUSTRING),  PROPERTY_NONE, 0},
                    COMMON_FLDTYP_PROPERTIES
                    {0,0,0,0,0,0}
                };
                aMapEntriesArr[nPropertyId] = aHiddenTxtFieldPropMap;
            }
            break;
            case PROPERTY_MAP_FLDTYP_ANNOTATION            :
            {
                static SfxItemPropertyMapEntry aAnnotationFieldPropMap [] =
                {
                    {SW_PROP_NMID(UNO_NAME_AUTHOR), FIELD_PROP_PAR1,    CPPU_E2T(CPPUTYPE_OUSTRING),   PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_CONTENT),    FIELD_PROP_PAR2,    CPPU_E2T(CPPUTYPE_OUSTRING),   PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_INITIALS),   FIELD_PROP_PAR3,    CPPU_E2T(CPPUTYPE_OUSTRING),   PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_NAME),       FIELD_PROP_PAR4,    CPPU_E2T(CPPUTYPE_OUSTRING),   PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_DATE_TIME_VALUE),    FIELD_PROP_DATE_TIME,   CPPU_E2T(CPPUTYPE_DATETIME),    PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_DATE),    FIELD_PROP_DATE,   CPPU_E2T(CPPUTYPE_DATE),    PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_TEXT_RANGE), FIELD_PROP_TEXT, CPPU_E2T(CPPUTYPE_REFINTERFACE),  PropertyAttribute::READONLY,    0},
                    COMMON_FLDTYP_PROPERTIES
                    {0,0,0,0,0,0}
                };
                aMapEntriesArr[nPropertyId] = aAnnotationFieldPropMap;
            }
            break;
            case PROPERTY_MAP_FLDTYP_INPUT:
            {
                static SfxItemPropertyMapEntry aInputFieldPropMap      [] =
                {
                    {SW_PROP_NMID(UNO_NAME_CONTENT),    FIELD_PROP_PAR1, CPPU_E2T(CPPUTYPE_OUSTRING),   PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_HINT),       FIELD_PROP_PAR2, CPPU_E2T(CPPUTYPE_OUSTRING),   PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_HELP),       FIELD_PROP_PAR3, CPPU_E2T(CPPUTYPE_OUSTRING),   PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_TOOLTIP),        FIELD_PROP_PAR4, CPPU_E2T(CPPUTYPE_OUSTRING),   PROPERTY_NONE, 0},
                    COMMON_FLDTYP_PROPERTIES
                    {0,0,0,0,0,0}
                };
                aMapEntriesArr[nPropertyId] = aInputFieldPropMap;
            }
            break;
            case PROPERTY_MAP_FLDTYP_MACRO                 :
            {
                static SfxItemPropertyMapEntry aMacroFieldPropMap      [] =
                {
                    {SW_PROP_NMID(UNO_NAME_HINT), FIELD_PROP_PAR2, CPPU_E2T(CPPUTYPE_OUSTRING),   PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_MACRO_NAME),FIELD_PROP_PAR1, CPPU_E2T(CPPUTYPE_OUSTRING),   PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_MACRO_LIBRARY),FIELD_PROP_PAR3, CPPU_E2T(CPPUTYPE_OUSTRING),PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_SCRIPT_URL),FIELD_PROP_PAR4, CPPU_E2T(CPPUTYPE_OUSTRING),PROPERTY_NONE, 0},
                    COMMON_FLDTYP_PROPERTIES
                    {0,0,0,0,0,0}
                };
                aMapEntriesArr[nPropertyId] = aMacroFieldPropMap;
            }
            break;
            case PROPERTY_MAP_FLDTYP_DDE                   :
            {
                static SfxItemPropertyMapEntry aDDEFieldPropMap            [] =
                {
                    COMMON_FLDTYP_PROPERTIES
                    {0,0,0,0,0,0}
                };
                aMapEntriesArr[nPropertyId] = aDDEFieldPropMap;
            }
            break;
            case PROPERTY_MAP_FLDTYP_DROPDOWN :
            {
                static SfxItemPropertyMapEntry aDropDownMap            [] =
                {
                    {SW_PROP_NMID(UNO_NAME_ITEMS), FIELD_PROP_STRINGS, CPPU_E2T(CPPUTYPE_OUSTRINGS), PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_SELITEM), FIELD_PROP_PAR1, CPPU_E2T(CPPUTYPE_OUSTRING), PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_NAME), FIELD_PROP_PAR2, CPPU_E2T(CPPUTYPE_OUSTRING), PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_HELP), FIELD_PROP_PAR3, CPPU_E2T(CPPUTYPE_OUSTRING), PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_TOOLTIP), FIELD_PROP_PAR4, CPPU_E2T(CPPUTYPE_OUSTRING), PROPERTY_NONE, 0},
                    COMMON_FLDTYP_PROPERTIES
                    {0,0,0,0,0,0}
                };
                aMapEntriesArr[nPropertyId] = aDropDownMap;
            }
            break;
            case PROPERTY_MAP_FLDTYP_HIDDEN_PARA           :
            {
                static SfxItemPropertyMapEntry aHiddenParaFieldPropMap [] =
                {
                    {SW_PROP_NMID(UNO_NAME_CONDITION),FIELD_PROP_PAR1, CPPU_E2T(CPPUTYPE_OUSTRING),   PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_IS_HIDDEN) ,  FIELD_PROP_BOOL1, CPPU_E2T(CPPUTYPE_BOOLEAN),   PROPERTY_NONE, 0},
                    COMMON_FLDTYP_PROPERTIES
                    {0,0,0,0,0,0}
                };
                aMapEntriesArr[nPropertyId] = aHiddenParaFieldPropMap;
            }
            break;
            case PROPERTY_MAP_FLDTYP_DOC_INFO              :
            {
                static SfxItemPropertyMapEntry aDocInfoFieldPropMap        [] =
                {
                    {SW_PROP_NMID(UNO_NAME_IS_FIXED),       FIELD_PROP_BOOL1,   CPPU_E2T(CPPUTYPE_BOOLEAN),     PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_INFO_FORMAT),    FIELD_PROP_USHORT2, CPPU_E2T(CPPUTYPE_INT16), PROPERTY_NONE,    0},
                    {SW_PROP_NMID(UNO_NAME_INFO_TYPE),  FIELD_PROP_USHORT1, CPPU_E2T(CPPUTYPE_INT16), PROPERTY_NONE,    0},
                    COMMON_FLDTYP_PROPERTIES
                    {0,0,0,0,0,0}
                };
                aMapEntriesArr[nPropertyId] = aDocInfoFieldPropMap;
            }
            break;
            case PROPERTY_MAP_FLDTYP_TEMPLATE_NAME         :
            {
                static SfxItemPropertyMapEntry aTmplNameFieldPropMap   [] =
                {
                    {SW_PROP_NMID(UNO_NAME_FILE_FORMAT), FIELD_PROP_FORMAT, CPPU_E2T(CPPUTYPE_INT16), PROPERTY_NONE,    0},
                    COMMON_FLDTYP_PROPERTIES
                    {0,0,0,0,0,0}
                };
                aMapEntriesArr[nPropertyId] = aTmplNameFieldPropMap;
            }
            break;
            case PROPERTY_MAP_FLDTYP_USER_EXT              :
            {
                static SfxItemPropertyMapEntry aUsrExtFieldPropMap     [] =
                {
                    {SW_PROP_NMID(UNO_NAME_CONTENT),            FIELD_PROP_PAR1,    CPPU_E2T(CPPUTYPE_OUSTRING),   PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_CURRENT_PRESENTATION), FIELD_PROP_PAR1, CPPU_E2T(CPPUTYPE_OUSTRING),  PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_IS_FIXED),           FIELD_PROP_BOOL1,   CPPU_E2T(CPPUTYPE_BOOLEAN),     PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_USER_DATA_TYPE), FIELD_PROP_USHORT1, CPPU_E2T(CPPUTYPE_INT16),   PROPERTY_NONE, 0},
                    COMMON_FLDTYP_PROPERTIES
                    {0,0,0,0,0,0}
                };
                aMapEntriesArr[nPropertyId]= aUsrExtFieldPropMap;
            }
            break;
            case PROPERTY_MAP_FLDTYP_REF_PAGE_SET          :
            {
                static SfxItemPropertyMapEntry aRefPgSetFieldPropMap   [] =
                {
                    {SW_PROP_NMID(UNO_NAME_OFFSET),     FIELD_PROP_USHORT1, CPPU_E2T(CPPUTYPE_INT16),   PROPERTY_NONE,  0},
                    {SW_PROP_NMID(UNO_NAME_ON),     FIELD_PROP_BOOL1,   CPPU_E2T(CPPUTYPE_BOOLEAN),     PROPERTY_NONE, 0},
                    COMMON_FLDTYP_PROPERTIES
                    {0,0,0,0,0,0}
                };
                aMapEntriesArr[nPropertyId] = aRefPgSetFieldPropMap;
            }
            break;
            case PROPERTY_MAP_FLDTYP_REF_PAGE_GET          :
            {
                static SfxItemPropertyMapEntry aRefPgGetFieldPropMap   [] =
                {
                    {SW_PROP_NMID(UNO_NAME_CURRENT_PRESENTATION), FIELD_PROP_PAR1, CPPU_E2T(CPPUTYPE_OUSTRING),  PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_NUMBERING_TYPE),     FIELD_PROP_USHORT1, CPPU_E2T(CPPUTYPE_INT16), PROPERTY_NONE,    0},
                    COMMON_FLDTYP_PROPERTIES
                    {0,0,0,0,0,0}
                };
                aMapEntriesArr[nPropertyId] = aRefPgGetFieldPropMap;
            }
            break;
            case PROPERTY_MAP_FLDTYP_JUMP_EDIT             :
            {
                static SfxItemPropertyMapEntry aJumpEdtFieldPropMap        [] =
                {
                    {SW_PROP_NMID(UNO_NAME_HINT),               FIELD_PROP_PAR1, CPPU_E2T(CPPUTYPE_OUSTRING),   PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_PLACEHOLDER),        FIELD_PROP_PAR2, CPPU_E2T(CPPUTYPE_OUSTRING),   PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_PLACEHOLDER_TYPE), FIELD_PROP_USHORT1, CPPU_E2T(CPPUTYPE_INT16),     PROPERTY_NONE,  0},
                    COMMON_FLDTYP_PROPERTIES
                    {0,0,0,0,0,0}
                };
                aMapEntriesArr[nPropertyId] = aJumpEdtFieldPropMap;
            }
            break;
            case PROPERTY_MAP_FLDTYP_SCRIPT                :
            {
                static SfxItemPropertyMapEntry aScriptFieldPropMap     [] =
                {
                    {SW_PROP_NMID(UNO_NAME_CONTENT),        FIELD_PROP_PAR2, CPPU_E2T(CPPUTYPE_OUSTRING),   PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_SCRIPT_TYPE),    FIELD_PROP_PAR1, CPPU_E2T(CPPUTYPE_OUSTRING),   PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_URL_CONTENT),    FIELD_PROP_BOOL1, CPPU_E2T(CPPUTYPE_BOOLEAN),       PROPERTY_NONE, 0},
                    COMMON_FLDTYP_PROPERTIES
                    {0,0,0,0,0,0}
                };
                aMapEntriesArr[nPropertyId] = aScriptFieldPropMap;
            }
            break;
            case PROPERTY_MAP_FLDTYP_DATABASE_NEXT_SET     :
            {
                static SfxItemPropertyMapEntry aDBNextSetFieldPropMap  [] =
                {
                    // Note: DATA_BASE_NAME and DATA_BASE_URL
                    // are mapped to the same nMId, because internally  we only use
                    // them as DataSource and it does not matter which one it is.

                    {SW_PROP_NMID(UNO_NAME_DATA_BASE_NAME ) , FIELD_PROP_PAR1, CPPU_E2T(CPPUTYPE_OUSTRING),   PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_DATA_TABLE_NAME) , FIELD_PROP_PAR2, CPPU_E2T(CPPUTYPE_OUSTRING),   PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_CONDITION)   ,     FIELD_PROP_PAR3, CPPU_E2T(CPPUTYPE_OUSTRING),   PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_DATA_BASE_URL ) ,  FIELD_PROP_PAR1, CPPU_E2T(CPPUTYPE_OUSTRING),   PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_DATA_COMMAND_TYPE), FIELD_PROP_SHORT1, CPPU_E2T(CPPUTYPE_INT32),   PROPERTY_NONE, 0},
                    COMMON_FLDTYP_PROPERTIES
                    {0,0,0,0,0,0}
                };
                aMapEntriesArr[nPropertyId] = aDBNextSetFieldPropMap;
            }
            break;
            case PROPERTY_MAP_FLDTYP_DATABASE_NUM_SET      :
            {
                static SfxItemPropertyMapEntry aDBNumSetFieldPropMap   [] =
                {
                    // Note: DATA_BASE_NAME and DATA_BASE_URL
                    // are mapped to the same nMId, because internally  we only use
                    // them as DataSource and it does not matter which one it is.

                    {SW_PROP_NMID(UNO_NAME_DATA_BASE_NAME ) ,  FIELD_PROP_PAR1, CPPU_E2T(CPPUTYPE_OUSTRING),   PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_DATA_TABLE_NAME  ), FIELD_PROP_PAR2, CPPU_E2T(CPPUTYPE_OUSTRING),   PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_CONDITION),         FIELD_PROP_PAR3, CPPU_E2T(CPPUTYPE_OUSTRING),   PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_DATA_BASE_URL ) ,   FIELD_PROP_PAR1, CPPU_E2T(CPPUTYPE_OUSTRING),   PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_DATA_COMMAND_TYPE), FIELD_PROP_SHORT1, CPPU_E2T(CPPUTYPE_INT32),   PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_SET_NUMBER       ), FIELD_PROP_FORMAT, CPPU_E2T(CPPUTYPE_INT32), PROPERTY_NONE,  0},
                    COMMON_FLDTYP_PROPERTIES
                    {0,0,0,0,0,0}
                };
                aMapEntriesArr[nPropertyId] = aDBNumSetFieldPropMap;
            }
            break;
            case PROPERTY_MAP_FLDTYP_DATABASE_SET_NUM      :
            {
                static SfxItemPropertyMapEntry aDBSetNumFieldPropMap   [] =
                {
                    // Note: DATA_BASE_NAME and DATA_BASE_URL
                    // are mapped to the same nMId, because internally  we only use
                    // them as DataSource and it does not matter which one it is.

                    {SW_PROP_NMID(UNO_NAME_DATA_BASE_NAME ) , FIELD_PROP_PAR1, CPPU_E2T(CPPUTYPE_OUSTRING),   PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_DATA_TABLE_NAME) , FIELD_PROP_PAR2, CPPU_E2T(CPPUTYPE_OUSTRING),   PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_DATA_BASE_URL ) ,  FIELD_PROP_PAR1, CPPU_E2T(CPPUTYPE_OUSTRING),   PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_DATA_COMMAND_TYPE), FIELD_PROP_SHORT1, CPPU_E2T(CPPUTYPE_INT32),   PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_NUMBERING_TYPE),       FIELD_PROP_USHORT1, CPPU_E2T(CPPUTYPE_INT16), PROPERTY_NONE,  0},
                    {SW_PROP_NMID(UNO_NAME_SET_NUMBER       ), FIELD_PROP_FORMAT, CPPU_E2T(CPPUTYPE_INT32), PROPERTY_NONE,  0},
                    {SW_PROP_NMID(UNO_NAME_IS_VISIBLE),       FIELD_PROP_BOOL2,   CPPU_E2T(CPPUTYPE_BOOLEAN),    PROPERTY_NONE, 0},
                    COMMON_FLDTYP_PROPERTIES
                    {0,0,0,0,0,0}
                };
                aMapEntriesArr[nPropertyId] = aDBSetNumFieldPropMap;
            }
            break;
            case PROPERTY_MAP_FLDTYP_DATABASE              :
            {
                static SfxItemPropertyMapEntry aDBFieldPropMap         [] =
                {
                    {SW_PROP_NMID(UNO_NAME_CONTENT),            FIELD_PROP_PAR1,    CPPU_E2T(CPPUTYPE_OUSTRING), PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_CURRENT_PRESENTATION), FIELD_PROP_PAR1, CPPU_E2T(CPPUTYPE_OUSTRING),  PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_FIELD_CODE),         FIELD_PROP_PAR2, CPPU_E2T(CPPUTYPE_OUSTRING),  PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_IS_DATA_BASE_FORMAT),FIELD_PROP_BOOL1, CPPU_E2T(CPPUTYPE_BOOLEAN)  , PROPERTY_NONE,0},
                    {SW_PROP_NMID(UNO_NAME_NUMBER_FORMAT),      FIELD_PROP_FORMAT, CPPU_E2T(CPPUTYPE_INT32), PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_IS_VISIBLE),       FIELD_PROP_BOOL2,   CPPU_E2T(CPPUTYPE_BOOLEAN),    PROPERTY_NONE, 0},
                    COMMON_FLDTYP_PROPERTIES
                    {0,0,0,0,0,0}
                };
                aMapEntriesArr[nPropertyId] = aDBFieldPropMap;
            }
            break;
            case PROPERTY_MAP_FLDTYP_DATABASE_NAME         :
            {
                static SfxItemPropertyMapEntry aDBNameFieldPropMap     [] =
                {
                    // Note: DATA_BASE_NAME and DATA_BASE_URL
                    // are mapped to the same nMId, because internally  we only use
                    // them as DataSource and it does not matter which one it is.

                    {SW_PROP_NMID(UNO_NAME_DATA_BASE_NAME ) , FIELD_PROP_PAR1, CPPU_E2T(CPPUTYPE_OUSTRING),   PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_DATA_TABLE_NAME) , FIELD_PROP_PAR2, CPPU_E2T(CPPUTYPE_OUSTRING),   PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_DATA_BASE_URL ) ,  FIELD_PROP_PAR1, CPPU_E2T(CPPUTYPE_OUSTRING),   PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_DATA_COMMAND_TYPE), FIELD_PROP_SHORT1, CPPU_E2T(CPPUTYPE_INT32),   PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_IS_VISIBLE),       FIELD_PROP_BOOL2,   CPPU_E2T(CPPUTYPE_BOOLEAN),    PROPERTY_NONE, 0},
                    COMMON_FLDTYP_PROPERTIES
                    {0,0,0,0,0,0}
                };
                aMapEntriesArr[nPropertyId] = aDBNameFieldPropMap;
            }
            break;
            case PROPERTY_MAP_FLDTYP_DOCSTAT:
            {
                static SfxItemPropertyMapEntry aDocstatFieldPropMap        [] =
                {
                    {SW_PROP_NMID(UNO_NAME_NUMBERING_TYPE),     FIELD_PROP_USHORT2, CPPU_E2T(CPPUTYPE_INT16), PROPERTY_NONE,    0},
                //  {UNO_NAME_STATISTIC_TYPE_ID,FIELD_PROP_USHORT1, CPPU_E2T(CPPUTYPE_INT16),   PROPERTY_NONE,  0},
                    COMMON_FLDTYP_PROPERTIES
                    {0,0,0,0,0,0}
                };
                aMapEntriesArr[nPropertyId] = aDocstatFieldPropMap;
            }
            break;
            case PROPERTY_MAP_FLDTYP_DOCINFO_AUTHOR:
            {
                static SfxItemPropertyMapEntry aDocInfoAuthorPropMap           [] =
                {
                    {SW_PROP_NMID(UNO_NAME_AUTHOR), FIELD_PROP_PAR1,    CPPU_E2T(CPPUTYPE_OUSTRING), PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_CURRENT_PRESENTATION), FIELD_PROP_PAR3, CPPU_E2T(CPPUTYPE_OUSTRING),  PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_IS_FIXED),   FIELD_PROP_BOOL1,   CPPU_E2T(CPPUTYPE_BOOLEAN)  , PROPERTY_NONE,0},
                    COMMON_FLDTYP_PROPERTIES
                    {0,0,0,0,0,0}
                };
                aMapEntriesArr[nPropertyId] = aDocInfoAuthorPropMap;
            }
            break;
            case PROPERTY_MAP_FLDTYP_DOCINFO_DATE_TIME:
            {
                static SfxItemPropertyMapEntry aDocInfoDateTimePropMap         [] =
                {
                    {SW_PROP_NMID(UNO_NAME_CURRENT_PRESENTATION), FIELD_PROP_PAR3, CPPU_E2T(CPPUTYPE_OUSTRING),  PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_DATE_TIME_VALUE),        FIELD_PROP_DOUBLE,  CPPU_E2T(CPPUTYPE_DOUBLE), PropertyAttribute::READONLY, 0},
                    {SW_PROP_NMID(UNO_NAME_IS_DATE),    FIELD_PROP_BOOL2,   CPPU_E2T(CPPUTYPE_BOOLEAN)  , PROPERTY_NONE,0},
                    {SW_PROP_NMID(UNO_NAME_NUMBER_FORMAT),FIELD_PROP_FORMAT,    CPPU_E2T(CPPUTYPE_INT32), PROPERTY_NONE,    0},
                    {SW_PROP_NMID(UNO_NAME_IS_FIXED),       FIELD_PROP_BOOL1,   CPPU_E2T(CPPUTYPE_BOOLEAN)  , PROPERTY_NONE,    0},
                    {SW_PROP_NMID(UNO_NAME_IS_FIXED_LANGUAGE), FIELD_PROP_BOOL4, CPPU_E2T(CPPUTYPE_BOOLEAN), PROPERTY_NONE,    0},
                    {0,0,0,0,0,0}
                };
                aMapEntriesArr[nPropertyId] = aDocInfoDateTimePropMap;
            }
            break;
            case PROPERTY_MAP_FLDTYP_DOCINFO_EDIT_TIME     :
            {
                static SfxItemPropertyMapEntry aDocInfoEditTimePropMap         [] =
                {
                    {SW_PROP_NMID(UNO_NAME_CURRENT_PRESENTATION), FIELD_PROP_PAR3, CPPU_E2T(CPPUTYPE_OUSTRING),  PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_DATE_TIME_VALUE),        FIELD_PROP_DOUBLE,  CPPU_E2T(CPPUTYPE_DOUBLE), PropertyAttribute::READONLY, 0},
                    {SW_PROP_NMID(UNO_NAME_NUMBER_FORMAT),FIELD_PROP_FORMAT,    CPPU_E2T(CPPUTYPE_INT32), PROPERTY_NONE,    0},
                    {SW_PROP_NMID(UNO_NAME_IS_FIXED),       FIELD_PROP_BOOL1,   CPPU_E2T(CPPUTYPE_BOOLEAN)  , PROPERTY_NONE,    0},
                    {SW_PROP_NMID(UNO_NAME_IS_FIXED_LANGUAGE), FIELD_PROP_BOOL4, CPPU_E2T(CPPUTYPE_BOOLEAN), PROPERTY_NONE,    0},
                    COMMON_FLDTYP_PROPERTIES
                    {0,0,0,0,0,0}
                };
                aMapEntriesArr[nPropertyId] = aDocInfoEditTimePropMap;
            }
            break;
            case PROPERTY_MAP_FLDTYP_DOCINFO_MISC:
            {
                static SfxItemPropertyMapEntry aDocInfoStringContentPropMap            [] =
                {
                    {SW_PROP_NMID(UNO_NAME_CONTENT),    FIELD_PROP_PAR1,    CPPU_E2T(CPPUTYPE_OUSTRING), PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_CURRENT_PRESENTATION), FIELD_PROP_PAR3, CPPU_E2T(CPPUTYPE_OUSTRING),  PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_IS_FIXED),   FIELD_PROP_BOOL1,   CPPU_E2T(CPPUTYPE_BOOLEAN)  , PROPERTY_NONE,0},
                    COMMON_FLDTYP_PROPERTIES
                    {0,0,0,0,0,0}
                };
                aMapEntriesArr[nPropertyId] = aDocInfoStringContentPropMap;
            }
            break;
            case PROPERTY_MAP_FLDTYP_DOCINFO_CUSTOM:
            {
                static SfxItemPropertyMapEntry aDocInfoCustomPropMap           [] =
                {
                    {SW_PROP_NMID(UNO_NAME_NAME),   FIELD_PROP_PAR4,    CPPU_E2T(CPPUTYPE_OUSTRING), PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_CURRENT_PRESENTATION), FIELD_PROP_PAR3, CPPU_E2T(CPPUTYPE_OUSTRING),  PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_IS_FIXED),   FIELD_PROP_BOOL1,   CPPU_E2T(CPPUTYPE_BOOLEAN)  , PROPERTY_NONE,0},
                    {SW_PROP_NMID(UNO_NAME_NUMBER_FORMAT), FIELD_PROP_FORMAT,   CPPU_E2T(CPPUTYPE_INT32), PROPERTY_NONE,    0},
                    {SW_PROP_NMID(UNO_NAME_IS_FIXED_LANGUAGE), FIELD_PROP_BOOL4, CPPU_E2T(CPPUTYPE_BOOLEAN), PROPERTY_NONE,    0},
                    COMMON_FLDTYP_PROPERTIES
                    {0,0,0,0,0,0}
                };
                aMapEntriesArr[nPropertyId] = aDocInfoCustomPropMap;
            }
            break;
            case PROPERTY_MAP_FLDTYP_DOCINFO_REVISION          :
            {
                static SfxItemPropertyMapEntry aDocInfoRevisionPropMap [] =
                {
                    {SW_PROP_NMID(UNO_NAME_CURRENT_PRESENTATION), FIELD_PROP_PAR3, CPPU_E2T(CPPUTYPE_OUSTRING),  PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_REVISION),   FIELD_PROP_USHORT1, CPPU_E2T(CPPUTYPE_INT16), PROPERTY_NONE,    0},
                    {SW_PROP_NMID(UNO_NAME_IS_FIXED),   FIELD_PROP_BOOL1,   CPPU_E2T(CPPUTYPE_BOOLEAN)  , PROPERTY_NONE,0},
                    COMMON_FLDTYP_PROPERTIES
                    {0,0,0,0,0,0}
                };
                aMapEntriesArr[nPropertyId] = aDocInfoRevisionPropMap;
            }
            break;
            case PROPERTY_MAP_FLDTYP_COMBINED_CHARACTERS:
            {
                static SfxItemPropertyMapEntry aCombinedCharactersPropMap[] =
                {
                    {SW_PROP_NMID(UNO_NAME_CONTENT), FIELD_PROP_PAR1, CPPU_E2T(CPPUTYPE_OUSTRING),  PROPERTY_NONE, 0},
                    COMMON_FLDTYP_PROPERTIES
                    {0,0,0,0,0,0}
                };
                aMapEntriesArr[nPropertyId] = aCombinedCharactersPropMap;
            }
            break;
            case PROPERTY_MAP_FLDTYP_TABLE_FORMULA:
            {
                static SfxItemPropertyMapEntry aTableFormulaPropMap[] =
                {
                    {SW_PROP_NMID(UNO_NAME_CURRENT_PRESENTATION), FIELD_PROP_PAR1, CPPU_E2T(CPPUTYPE_OUSTRING),  PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_CONTENT), FIELD_PROP_PAR2, CPPU_E2T(CPPUTYPE_OUSTRING),  PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_IS_SHOW_FORMULA), FIELD_PROP_BOOL1,  CPPU_E2T(CPPUTYPE_BOOLEAN), PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_NUMBER_FORMAT), FIELD_PROP_FORMAT,   CPPU_E2T(CPPUTYPE_INT32), PROPERTY_NONE, 0},
                    COMMON_FLDTYP_PROPERTIES
                    {0,0,0,0,0,0}
                };
                aMapEntriesArr[nPropertyId] = aTableFormulaPropMap;
            }
            break;
            case PROPERTY_MAP_FLDTYP_DUMMY_0:
            {
                static SfxItemPropertyMapEntry aEmptyPropMap           [] =
                {
                    COMMON_FLDTYP_PROPERTIES
                    {0,0,0,0,0,0}
                };
                aMapEntriesArr[nPropertyId] = aEmptyPropMap;
            }
            break;
            case PROPERTY_MAP_FLDMSTR_USER :
            {
                static SfxItemPropertyMapEntry aUserFieldTypePropMap[] =
                {
                    {SW_PROP_NMID(UNO_NAME_DEPENDENT_TEXT_FIELDS),  FIELD_PROP_PROP_SEQ,    CPPU_E2T(CPPUTYPE_SEQDEPTXTFLD), PropertyAttribute::READONLY, 0},
                    {SW_PROP_NMID(UNO_NAME_IS_EXPRESSION),      FIELD_PROP_BOOL1,  CPPU_E2T(CPPUTYPE_BOOLEAN), PROPERTY_NONE,   0},
                    {SW_PROP_NMID(UNO_NAME_NAME),               FIELD_PROP_PAR1,  CPPU_E2T(CPPUTYPE_OUSTRING), PropertyAttribute::MAYBEVOID, 0},
                    {SW_PROP_NMID(UNO_NAME_VALUE),          FIELD_PROP_DOUBLE,  CPPU_E2T(CPPUTYPE_DOUBLE), PROPERTY_NONE,   0},
                    {SW_PROP_NMID(UNO_NAME_CONTENT),            FIELD_PROP_PAR2,    CPPU_E2T(CPPUTYPE_OUSTRING), PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_INSTANCE_NAME),      FIELD_PROP_PAR3,    CPPU_E2T(CPPUTYPE_OUSTRING), PropertyAttribute::READONLY, 0},
                    {0,0,0,0,0,0}
                };
                aMapEntriesArr[nPropertyId] = aUserFieldTypePropMap;
            }
            break;
            case PROPERTY_MAP_FLDMSTR_DDE       :
            {
                static SfxItemPropertyMapEntry aDDEFieldTypePropMap[] =
                {
                    {SW_PROP_NMID(UNO_NAME_DDE_COMMAND_ELEMENT ), FIELD_PROP_PAR2,  CPPU_E2T(CPPUTYPE_OUSTRING), PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_DDE_COMMAND_FILE    ), FIELD_PROP_PAR4,  CPPU_E2T(CPPUTYPE_OUSTRING), PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_DDE_COMMAND_TYPE    ), FIELD_PROP_SUBTYPE,   CPPU_E2T(CPPUTYPE_OUSTRING), PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_DEPENDENT_TEXT_FIELDS),  FIELD_PROP_PROP_SEQ,    CPPU_E2T(CPPUTYPE_SEQDEPTXTFLD), PropertyAttribute::READONLY, 0},
                    {SW_PROP_NMID(UNO_NAME_IS_AUTOMATIC_UPDATE), FIELD_PROP_BOOL1,  CPPU_E2T(CPPUTYPE_BOOLEAN), PROPERTY_NONE,    0},
                    {SW_PROP_NMID(UNO_NAME_NAME),               FIELD_PROP_PAR1,  CPPU_E2T(CPPUTYPE_OUSTRING), PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_INSTANCE_NAME),      FIELD_PROP_PAR3,    CPPU_E2T(CPPUTYPE_OUSTRING), PropertyAttribute::READONLY, 0},
                    {SW_PROP_NMID(UNO_NAME_CONTENT),            FIELD_PROP_PAR5,    CPPU_E2T(CPPUTYPE_OUSTRING),   PROPERTY_NONE, 0},
                    {0,0,0,0,0,0}
                };
                aMapEntriesArr[nPropertyId] = aDDEFieldTypePropMap;
            }
            break;
            case PROPERTY_MAP_FLDMSTR_SET_EXP     :
            {
                static SfxItemPropertyMapEntry aSetExpFieldTypePropMap[] =
                {
                    {SW_PROP_NMID(UNO_NAME_CHAPTER_NUMBERING_LEVEL),FIELD_PROP_SHORT1,  CPPU_E2T(CPPUTYPE_INT8), PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_DEPENDENT_TEXT_FIELDS),  FIELD_PROP_PROP_SEQ,    CPPU_E2T(CPPUTYPE_SEQDEPTXTFLD), PropertyAttribute::READONLY, 0},
                    {SW_PROP_NMID(UNO_NAME_NAME),               FIELD_PROP_PAR1,  CPPU_E2T(CPPUTYPE_OUSTRING), PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_NUMBERING_SEPARATOR), FIELD_PROP_PAR2,   CPPU_E2T(CPPUTYPE_OUSTRING), PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_SUB_TYPE),           FIELD_PROP_SUBTYPE, CPPU_E2T(CPPUTYPE_INT16), PROPERTY_NONE,    0},
                    {SW_PROP_NMID(UNO_NAME_INSTANCE_NAME),      FIELD_PROP_PAR3,    CPPU_E2T(CPPUTYPE_OUSTRING), PropertyAttribute::READONLY, 0},
                    {0,0,0,0,0,0}
                };
                aMapEntriesArr[nPropertyId] = aSetExpFieldTypePropMap;
            }
            break;
            case PROPERTY_MAP_FLDMSTR_DATABASE    :
            {
                static SfxItemPropertyMapEntry aDBFieldTypePropMap         [] =
                {
                    // Note: DATA_BASE_NAME and DATA_BASE_URL
                    // are mapped to the same nMId, because internally  we only use
                    // them as DataSource and it does not matter which one it is.

                    {SW_PROP_NMID(UNO_NAME_DATA_BASE_NAME ) ,  FIELD_PROP_PAR1, CPPU_E2T(CPPUTYPE_OUSTRING),   PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_NAME),              FIELD_PROP_PAR3,  CPPU_E2T(CPPUTYPE_OUSTRING), PropertyAttribute::MAYBEVOID, 0},
                    {SW_PROP_NMID(UNO_NAME_DATA_TABLE_NAME  ), FIELD_PROP_PAR2, CPPU_E2T(CPPUTYPE_OUSTRING),   PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_DATA_COLUMN_NAME ), FIELD_PROP_PAR3, CPPU_E2T(CPPUTYPE_OUSTRING),   PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_INSTANCE_NAME),     FIELD_PROP_PAR4, CPPU_E2T(CPPUTYPE_OUSTRING), PropertyAttribute::READONLY, 0},
                    {SW_PROP_NMID(UNO_NAME_DATA_BASE_URL ) ,   FIELD_PROP_PAR1, CPPU_E2T(CPPUTYPE_OUSTRING),   PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_DATA_COMMAND_TYPE), FIELD_PROP_SHORT1, CPPU_E2T(CPPUTYPE_INT32),   PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_DEPENDENT_TEXT_FIELDS),  FIELD_PROP_PROP_SEQ,    CPPU_E2T(CPPUTYPE_SEQDEPTXTFLD), PropertyAttribute::READONLY, 0},
                    {0,0,0,0,0,0}
                };
                aMapEntriesArr[nPropertyId] = aDBFieldTypePropMap;
            }
            break;
            case PROPERTY_MAP_FLDMSTR_DUMMY0      :
            {
                static SfxItemPropertyMapEntry aStandardFieldMasterMap[] =
                {
                    {SW_PROP_NMID(UNO_NAME_DEPENDENT_TEXT_FIELDS),  0,  CPPU_E2T(CPPUTYPE_SEQDEPTXTFLD), PropertyAttribute::READONLY, 0},
                    {SW_PROP_NMID(UNO_NAME_NAME),               0,  CPPU_E2T(CPPUTYPE_OUSTRING), PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_INSTANCE_NAME),      0,  CPPU_E2T(CPPUTYPE_OUSTRING), PropertyAttribute::READONLY, 0},
                    {0,0,0,0,0,0}
                };
                aMapEntriesArr[nPropertyId] = aStandardFieldMasterMap;
            }
            break;
            case PROPERTY_MAP_FLDTYP_BIBLIOGRAPHY:
            {
                static SfxItemPropertyMapEntry aBibliographyFieldMap[] =
                {
                    {SW_PROP_NMID(UNO_NAME_FIELDS  )    , FIELD_PROP_PROP_SEQ, CPPU_E2T(CPPUTYPE_PROPERTYVALUE),PROPERTY_NONE, 0},
                    COMMON_FLDTYP_PROPERTIES
                    {0,0,0,0,0,0}
                };
                aMapEntriesArr[nPropertyId] = aBibliographyFieldMap;
            }
            break;
            case PROPERTY_MAP_FLDMSTR_BIBLIOGRAPHY:
            {
                static SfxItemPropertyMapEntry aBibliographyFieldMasterMap[] =
                {
                    {SW_PROP_NMID(UNO_NAME_BRACKET_BEFORE     ) , FIELD_PROP_PAR1, CPPU_E2T(CPPUTYPE_OUSTRING),               PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_BRACKET_AFTER      ) , FIELD_PROP_PAR2, CPPU_E2T(CPPUTYPE_OUSTRING),               PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_IS_NUMBER_ENTRIES  ) , FIELD_PROP_BOOL1, CPPU_E2T(CPPUTYPE_BOOLEAN),                    PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_IS_SORT_BY_POSITION) , FIELD_PROP_BOOL2, CPPU_E2T(CPPUTYPE_BOOLEAN),                    PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_LOCALE),            FIELD_PROP_LOCALE,  CPPU_E2T(CPPUTYPE_LOCALE)  , PROPERTY_NONE,     0},
                    {SW_PROP_NMID(UNO_NAME_SORT_ALGORITHM),    FIELD_PROP_PAR3,  CPPU_E2T(CPPUTYPE_OUSTRING), PROPERTY_NONE,     0},
                    {SW_PROP_NMID(UNO_NAME_SORT_KEYS          ) , FIELD_PROP_PROP_SEQ, CPPU_E2T(CPPUTYPE_PROPERTYVALUES),   PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_INSTANCE_NAME),      FIELD_PROP_PAR4,    CPPU_E2T(CPPUTYPE_OUSTRING), PropertyAttribute::READONLY, 0},
                    {0,0,0,0,0,0}
                };
                aMapEntriesArr[nPropertyId] = aBibliographyFieldMasterMap;
            }
            break;
            case PROPERTY_MAP_TEXT :
            {
                static SfxItemPropertyMapEntry aTextMap[] =
                {
                    _REDLINE_NODE_PROPERTIES
                    {0,0,0,0,0,0}
                };
                aMapEntriesArr[nPropertyId] = aTextMap;
            }
            break;
            case PROPERTY_MAP_MAILMERGE :
            {
                static SfxItemPropertyMapEntry aMailMergeMap[] =
                {
                    { SW_PROP_NMID(UNO_NAME_SELECTION),             WID_SELECTION,              CPPU_E2T(CPPUTYPE_SEQANY),      PROPERTY_NONE, 0},
                    { SW_PROP_NMID(UNO_NAME_RESULT_SET),            WID_RESULT_SET,             CPPU_E2T(CPPUTYPE_REFRESULTSET), PROPERTY_NONE, 0},
                    { SW_PROP_NMID(UNO_NAME_CONNECTION),            WID_CONNECTION,             CPPU_E2T(CPPUTYPE_REFCONNECTION), PROPERTY_NONE, 0},
                    { SW_PROP_NMID(UNO_NAME_MODEL),                 WID_MODEL,                  CPPU_E2T(CPPUTYPE_REFMODEL),    PropertyAttribute::READONLY, 0},
                    { SW_PROP_NMID(UNO_NAME_DATA_SOURCE_NAME),      WID_DATA_SOURCE_NAME,       CPPU_E2T(CPPUTYPE_OUSTRING),    PROPERTY_NONE, 0},
                    { SW_PROP_NMID(UNO_NAME_DAD_COMMAND),           WID_DATA_COMMAND,           CPPU_E2T(CPPUTYPE_OUSTRING),    PROPERTY_NONE, 0},
                    { SW_PROP_NMID(UNO_NAME_FILTER),                WID_FILTER,                 CPPU_E2T(CPPUTYPE_OUSTRING),    PROPERTY_NONE, 0},
                    { SW_PROP_NMID(UNO_NAME_DOCUMENT_URL),          WID_DOCUMENT_URL,           CPPU_E2T(CPPUTYPE_OUSTRING),    PROPERTY_NONE, 0},
                    { SW_PROP_NMID(UNO_NAME_OUTPUT_URL),            WID_OUTPUT_URL,             CPPU_E2T(CPPUTYPE_OUSTRING),    PROPERTY_NONE, 0},
                    { SW_PROP_NMID(UNO_NAME_DAD_COMMAND_TYPE),      WID_DATA_COMMAND_TYPE,      CPPU_E2T(CPPUTYPE_INT32),       PROPERTY_NONE, 0},
                    { SW_PROP_NMID(UNO_NAME_OUTPUT_TYPE),           WID_OUTPUT_TYPE,            CPPU_E2T(CPPUTYPE_INT16),       PROPERTY_NONE, 0},
                    { SW_PROP_NMID(UNO_NAME_ESCAPE_PROCESSING),     WID_ESCAPE_PROCESSING,      CPPU_E2T(CPPUTYPE_BOOLEAN),     PROPERTY_NONE, 0},
                    { SW_PROP_NMID(UNO_NAME_SINGLE_PRINT_JOBS),     WID_SINGLE_PRINT_JOBS,      CPPU_E2T(CPPUTYPE_BOOLEAN),     PROPERTY_NONE, 0},
                    { SW_PROP_NMID(UNO_NAME_FILE_NAME_FROM_COLUMN), WID_FILE_NAME_FROM_COLUMN,  CPPU_E2T(CPPUTYPE_BOOLEAN),     PROPERTY_NONE, 0},
                    { SW_PROP_NMID(UNO_NAME_FILE_NAME_PREFIX),      WID_FILE_NAME_PREFIX,       CPPU_E2T(CPPUTYPE_OUSTRING),    PROPERTY_NONE, 0},
                    { SW_PROP_NMID(UNO_NAME_SUBJECT),               WID_MAIL_SUBJECT,           CPPU_E2T(CPPUTYPE_OUSTRING),        PROPERTY_NONE, 0},
                    { SW_PROP_NMID(UNO_NAME_ADDRESS_FROM_COLUMN),   WID_ADDRESS_FROM_COLUMN,    CPPU_E2T(CPPUTYPE_OUSTRING),        PROPERTY_NONE, 0},
                    { SW_PROP_NMID(UNO_NAME_SEND_AS_HTML),          WID_SEND_AS_HTML,           CPPU_E2T(CPPUTYPE_BOOLEAN),         PROPERTY_NONE, 0},
                    { SW_PROP_NMID(UNO_NAME_SEND_AS_ATTACHMENT),    WID_SEND_AS_ATTACHMENT,     CPPU_E2T(CPPUTYPE_BOOLEAN),         PROPERTY_NONE, 0},
                    { SW_PROP_NMID(UNO_NAME_MAIL_BODY),             WID_MAIL_BODY,              CPPU_E2T(CPPUTYPE_OUSTRING),        PROPERTY_NONE, 0},
                    { SW_PROP_NMID(UNO_NAME_ATTACHMENT_NAME),       WID_ATTACHMENT_NAME,        CPPU_E2T(CPPUTYPE_OUSTRING),        PROPERTY_NONE, 0},
                    { SW_PROP_NMID(UNO_NAME_ATTACHMENT_FILTER),     WID_ATTACHMENT_FILTER,      CPPU_E2T(CPPUTYPE_OUSTRING),        PROPERTY_NONE, 0},
                    { SW_PROP_NMID(UNO_NAME_PRINT_OPTIONS),         WID_PRINT_OPTIONS,          CPPU_E2T(CPPUTYPE_PROPERTYVALUE),  PROPERTY_NONE, 0},
                    { SW_PROP_NMID(UNO_NAME_SAVE_AS_SINGLE_FILE),   WID_SAVE_AS_SINGLE_FILE,    CPPU_E2T(CPPUTYPE_BOOLEAN),         PROPERTY_NONE, 0},
                    { SW_PROP_NMID(UNO_NAME_SAVE_FILTER),           WID_SAVE_FILTER,            CPPU_E2T(CPPUTYPE_OUSTRING),        PROPERTY_NONE, 0},
                    { SW_PROP_NMID(UNO_NAME_SAVE_FILTER_OPTIONS),   WID_SAVE_FILTER_OPTIONS,    CPPU_E2T(CPPUTYPE_OUSTRING),        PROPERTY_NONE, 0},
                    { SW_PROP_NMID(UNO_NAME_SAVE_FILTER_DATA),      WID_SAVE_FILTER_DATA,       CPPU_E2T(CPPUTYPE_PROPERTYVALUE),        PROPERTY_NONE, 0},
                    { SW_PROP_NMID(UNO_NAME_COPIES_TO),             WID_COPIES_TO,              CPPU_E2T(CPPUTYPE_OUSTRINGS),       PROPERTY_NONE, 0},
                    { SW_PROP_NMID(UNO_NAME_BLIND_COPIES_TO),       WID_BLIND_COPIES_TO,        CPPU_E2T(CPPUTYPE_OUSTRINGS),       PROPERTY_NONE, 0},
                    { SW_PROP_NMID(UNO_NAME_IN_SERVER_PASSWORD),     WID_IN_SERVER_PASSWORD,     CPPU_E2T(CPPUTYPE_OUSTRING),    PROPERTY_NONE, 0},
                    { SW_PROP_NMID(UNO_NAME_OUT_SERVER_PASSWORD),    WID_OUT_SERVER_PASSWORD,    CPPU_E2T(CPPUTYPE_OUSTRING),    PROPERTY_NONE, 0},
                    {0,0,0,0,0,0}
                };
                aMapEntriesArr[nPropertyId] = aMailMergeMap;
            }
            break;
            case PROPERTY_MAP_TEXT_VIEW :
            {
                static SfxItemPropertyMapEntry pTextViewMap[] =
                {
                    {SW_PROP_NMID(UNO_NAME_PAGE_COUNT),             WID_PAGE_COUNT,             CPPU_E2T(CPPUTYPE_INT32),   PropertyAttribute::READONLY, 0},
                    {SW_PROP_NMID(UNO_NAME_LINE_COUNT),             WID_LINE_COUNT,             CPPU_E2T(CPPUTYPE_INT32),   PropertyAttribute::READONLY, 0},
                    {SW_PROP_NMID(UNO_NAME_IS_CONSTANT_SPELLCHECK), WID_IS_CONSTANT_SPELLCHECK, CPPU_E2T(CPPUTYPE_BOOLEAN), PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_IS_HIDE_SPELL_MARKS),    WID_IS_HIDE_SPELL_MARKS,    CPPU_E2T(CPPUTYPE_BOOLEAN), PROPERTY_NONE, 0},  // deprecated #i91949
                    {0,0,0,0,0,0}
                };
                aMapEntriesArr[nPropertyId] = pTextViewMap;
            }
            break;
            case PROPERTY_MAP_CHART2_DATA_SEQUENCE :
            {
                static SfxItemPropertyMapEntry aChart2DataSequenceMap[] =
                {
                    {SW_PROP_NMID(UNO_NAME_ROLE), 0, CPPU_E2T(CPPUTYPE_OUSTRING),   PROPERTY_NONE, 0 },
                    {0,0,0,0,0,0}
                };
                aMapEntriesArr[nPropertyId] = aChart2DataSequenceMap;
            }
            break;
            case PROPERTY_MAP_METAFIELD:
            {
                static SfxItemPropertyMapEntry aMetaFieldMap[] =
                {
                    { SW_PROP_NMID(UNO_NAME_NUMBER_FORMAT), 0,
                        CPPU_E2T(CPPUTYPE_INT32), PROPERTY_NONE, 0 },
                    { SW_PROP_NMID(UNO_NAME_IS_FIXED_LANGUAGE), 0,
                        CPPU_E2T(CPPUTYPE_BOOLEAN), PROPERTY_NONE, 0 },
                    {0,0,0,0,0,0}
                };
                aMapEntriesArr[nPropertyId] = aMetaFieldMap;
            }
            break;

            default:
                OSL_FAIL( "unexpected property map ID" );
        }
        //fill the character pointers and types into the arrays
        SfxItemPropertyMapEntry* p = aMapEntriesArr[nPropertyId];
        sal_uInt16 i = 0;
        for( ; p->pName; ++p, ++i )
        {
            // set the name
            const SwPropNameLen& rPropNm = GetPropName( (sal_uInt16)(sal_IntPtr)p->pName );
            p->pName = rPropNm.pName;
            p->nNameLen = rPropNm.nNameLen;
            // get the cppu type from the comphelper
            CppuTypes nTyp = (CppuTypes) (sal_IntPtr) p->pType;
            GenerateCppuType( nTyp, p->pType );
            OSL_ENSURE( nTyp != (CppuTypes) (sal_IntPtr) p->pType, "unknown type" );
        }
    }
    return aMapEntriesArr[nPropertyId];
}

const SfxItemPropertySet*  SwUnoPropertyMapProvider::GetPropertySet( sal_uInt16 nPropertyId)
{
    if( !aPropertySetArr[nPropertyId] )
    {
        const SfxItemPropertyMapEntry* pEntries = GetPropertyMapEntries(nPropertyId);
        switch( nPropertyId )
        {
            case PROPERTY_MAP_TEXT_CURSOR:
            {
                static SfxItemPropertySet aPROPERTY_MAP_TEXT_CURSOR(pEntries);
                aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_TEXT_CURSOR;
            }
            break;
            case PROPERTY_MAP_CHAR_STYLE:
            {
                static SfxItemPropertySet aPROPERTY_MAP_CHAR_STYLE(pEntries);
                aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_CHAR_STYLE;
            }
            break;
            case PROPERTY_MAP_PARA_STYLE:
            {
                static SfxItemPropertySet aPROPERTY_MAP_PARA_STYLE(pEntries);
                aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_PARA_STYLE;
            }
            break;
            case PROPERTY_MAP_FRAME_STYLE:
            {
                static SfxItemPropertySet aPROPERTY_MAP_FRAME_STYLE(pEntries);
                aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_FRAME_STYLE;
            }
            break;
            case PROPERTY_MAP_PAGE_STYLE:
            {
                static SfxItemPropertySet aPROPERTY_MAP_PAGE_STYLE(pEntries);
                aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_PAGE_STYLE;
            }
            break;
            case PROPERTY_MAP_NUM_STYLE:
            {
                static SfxItemPropertySet aPROPERTY_MAP_NUM_STYLE(pEntries);
                aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_NUM_STYLE;
            }
            break;
            case PROPERTY_MAP_SECTION:
            {
                static SfxItemPropertySet aPROPERTY_MAP_SECTION(pEntries);
                aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_SECTION;
            }
            break;
            case PROPERTY_MAP_TEXT_TABLE:
            {
                static SfxItemPropertySet aPROPERTY_MAP_TEXT_TABLE(pEntries);
                aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_TEXT_TABLE;
            }
            break;
            case PROPERTY_MAP_TABLE_CELL:
            {
                static SfxItemPropertySet aPROPERTY_MAP_TABLE_CELL(pEntries);
                aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_TABLE_CELL;
            }
            break;
            case PROPERTY_MAP_TABLE_RANGE:
            {
                static SfxItemPropertySet aPROPERTY_MAP_TABLE_RANGE(pEntries);
                aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_TABLE_RANGE;
            }
            break;
            case PROPERTY_MAP_TEXT_SEARCH:
            {
                static SfxItemPropertySet aPROPERTY_MAP_TEXT_SEARCH(pEntries);
                aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_TEXT_SEARCH;
            }
            break;
            case PROPERTY_MAP_TEXT_FRAME:
            {
                static SfxItemPropertySet aPROPERTY_MAP_TEXT_FRAME(pEntries);
                aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_TEXT_FRAME;
            }
            break;
            case PROPERTY_MAP_TEXT_GRAPHIC:
            {
                static SfxItemPropertySet aPROPERTY_MAP_TEXT_GRAPHIC(pEntries);
                aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_TEXT_GRAPHIC;
            }
            break;
            case PROPERTY_MAP_TEXT_SHAPE:
            {
                static SfxItemPropertySet aPROPERTY_MAP_TEXT_SHAPE(pEntries);
                aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_TEXT_SHAPE;
            }
            break;
            case PROPERTY_MAP_INDEX_USER:
            {
                static SfxItemPropertySet aPROPERTY_MAP_INDEX_USER(pEntries);
                aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_INDEX_USER;
            }
            break;
            case PROPERTY_MAP_INDEX_CNTNT:
            {
                static SfxItemPropertySet aPROPERTY_MAP_INDEX_CNTNT(pEntries);
                aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_INDEX_CNTNT;
            }
            break;
            case PROPERTY_MAP_INDEX_IDX:
            {
                static SfxItemPropertySet aPROPERTY_MAP_INDEX_IDX(pEntries);
                aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_INDEX_IDX;
            }
            break;
            case PROPERTY_MAP_USER_MARK:
            {
                static SfxItemPropertySet aPROPERTY_MAP_USER_MARK(pEntries);
                aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_USER_MARK;
            }
            break;
            case PROPERTY_MAP_CNTIDX_MARK:
            {
                static SfxItemPropertySet aPROPERTY_MAP_CNTIDX_MARK(pEntries);
                aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_CNTIDX_MARK;
            }
            break;
            case PROPERTY_MAP_INDEX_MARK:
            {
                static SfxItemPropertySet aPROPERTY_MAP_INDEX_MARK(pEntries);
                aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_INDEX_MARK;
            }
            break;
            case PROPERTY_MAP_TEXT_TABLE_ROW:
            {
                static SfxItemPropertySet aPROPERTY_MAP_TEXT_TABLE_ROW(pEntries);
                aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_TEXT_TABLE_ROW;
            }
            break;
            case PROPERTY_MAP_TEXT_SHAPE_DESCRIPTOR:
            {
                static SfxItemPropertySet aPROPERTY_MAP_TEXT_SHAPE_DESCRIPTOR(pEntries);
                aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_TEXT_SHAPE_DESCRIPTOR;
            }
            break;
            case PROPERTY_MAP_TEXT_TABLE_CURSOR:
            {
                static SfxItemPropertySet aPROPERTY_MAP_TEXT_TABLE_CURSOR(pEntries);
                aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_TEXT_TABLE_CURSOR;
            }
            break;
            case PROPERTY_MAP_BOOKMARK:
            {
                static SfxItemPropertySet aPROPERTY_MAP_BOOKMARK(pEntries);
                aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_BOOKMARK;
            }
            break;
            case PROPERTY_MAP_PARAGRAPH_EXTENSIONS:
            {
                static SfxItemPropertySet aPROPERTY_MAP_PARAGRAPH_EXTENSIONS(pEntries);
                aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_PARAGRAPH_EXTENSIONS;
            }
            break;
            case PROPERTY_MAP_INDEX_ILLUSTRATIONS:
            {
                static SfxItemPropertySet aPROPERTY_MAP_INDEX_ILLUSTRATIONS(pEntries);
                aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_INDEX_ILLUSTRATIONS;
            }
            break;
            case PROPERTY_MAP_INDEX_OBJECTS:
            {
                static SfxItemPropertySet aPROPERTY_MAP_INDEX_OBJECTS(pEntries);
                aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_INDEX_OBJECTS;
            }
            break;
            case PROPERTY_MAP_INDEX_TABLES:
            {
                static SfxItemPropertySet aPROPERTY_MAP_INDEX_TABLES(pEntries);
                aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_INDEX_TABLES;
            }
            break;
            case PROPERTY_MAP_BIBLIOGRAPHY           :
            {
                static SfxItemPropertySet aPROPERTY_MAP_BIBLIOGRAPHY(pEntries);
                aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_BIBLIOGRAPHY;
            }
            break;
            case PROPERTY_MAP_TEXT_DOCUMENT:
            {
                static SfxItemPropertySet aPROPERTY_MAP_TEXT_DOCUMENT(pEntries);
                aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_TEXT_DOCUMENT;
            }
            break;
            case PROPERTY_MAP_LINK_TARGET            :
            {
                static SfxItemPropertySet aPROPERTY_MAP_LINK_TARGET(pEntries);
                aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_LINK_TARGET;
            }
            break;
            case PROPERTY_MAP_AUTO_TEXT_GROUP        :
            {
                static SfxItemPropertySet aPROPERTY_MAP_AUTO_TEXT_GROUP(pEntries);
                aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_AUTO_TEXT_GROUP;
            }
            break;
            case PROPERTY_MAP_TEXTPORTION_EXTENSIONS :
            {
                static SfxItemPropertySet aPROPERTY_MAP_TEXTPORTION_EXTENSIONS(pEntries);
                aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_TEXTPORTION_EXTENSIONS;
            }
            break;
            case PROPERTY_MAP_FOOTNOTE               :
            {
                static SfxItemPropertySet aPROPERTY_MAP_FOOTNOTE(pEntries);
                aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_FOOTNOTE;
            }
            break;
            case PROPERTY_MAP_TEXT_COLUMS            :
            {
                static SfxItemPropertySet aPROPERTY_MAP_TEXT_COLUMS(pEntries);
                aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_TEXT_COLUMS;
            }
            break;
            case PROPERTY_MAP_PARAGRAPH              :
            {
                static SfxItemPropertySet aPROPERTY_MAP_PARAGRAPH(pEntries);
                aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_PARAGRAPH;
            }
            break;
            case PROPERTY_MAP_EMBEDDED_OBJECT        :
            {
                static SfxItemPropertySet aPROPERTY_MAP_EMBEDDED_OBJECT(pEntries);
                aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_EMBEDDED_OBJECT;
            }
            break;
            case PROPERTY_MAP_REDLINE                :
            {
                static SfxItemPropertySet aPROPERTY_MAP_REDLINE(pEntries);
                aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_REDLINE;
            }
            break;
            case PROPERTY_MAP_TEXT_DEFAULT           :
            {
                static SfxItemPropertySet aPROPERTY_MAP_TEXT_DEFAULT(pEntries);
                aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_TEXT_DEFAULT;
            }
            break;
            case PROPERTY_MAP_FLDTYP_DATETIME:
            {
                static SfxItemPropertySet aPROPERTY_MAP_FLDTYP_DATETIME(pEntries);
                aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_FLDTYP_DATETIME;
            }
            break;
            case PROPERTY_MAP_FLDTYP_USER:
            {
                static SfxItemPropertySet aPROPERTY_MAP_FLDTYP_USER(pEntries);
                aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_FLDTYP_USER;
            }
            break;
            case PROPERTY_MAP_FLDTYP_SET_EXP:
            {
                static SfxItemPropertySet aPROPERTY_MAP_FLDTYP_SET_EXP(pEntries);
                aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_FLDTYP_SET_EXP;
            }
            break;
            case PROPERTY_MAP_FLDTYP_GET_EXP:
            {
                static SfxItemPropertySet aPROPERTY_MAP_FLDTYP_GET_EXP(pEntries);
                aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_FLDTYP_GET_EXP;
            }
            break;
            case PROPERTY_MAP_FLDTYP_FILE_NAME:
            {
                static SfxItemPropertySet aPROPERTY_MAP_FLDTYP_FILE_NAME(pEntries);
                aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_FLDTYP_FILE_NAME;
            }
            break;
            case PROPERTY_MAP_FLDTYP_PAGE_NUM:
            {
                static SfxItemPropertySet aPROPERTY_MAP_FLDTYP_PAGE_NUM(pEntries);
                aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_FLDTYP_PAGE_NUM;
            }
            break;
            case PROPERTY_MAP_FLDTYP_AUTHOR:
            {
                static SfxItemPropertySet aPROPERTY_MAP_FLDTYP_AUTHOR(pEntries);
                aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_FLDTYP_AUTHOR;
            }
            break;
            case PROPERTY_MAP_FLDTYP_CHAPTER:
            {
                static SfxItemPropertySet aPROPERTY_MAP_FLDTYP_CHAPTER(pEntries);
                aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_FLDTYP_CHAPTER;
            }
            break;
            case PROPERTY_MAP_FLDTYP_GET_REFERENCE:
            {
                static SfxItemPropertySet aPROPERTY_MAP_FLDTYP_GET_REFERENCE(pEntries);
                aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_FLDTYP_GET_REFERENCE;
            }
            break;
            case PROPERTY_MAP_FLDTYP_CONDITIONED_TEXT:
            {
                static SfxItemPropertySet aPROPERTY_MAP_FLDTYP_CONDITIONED_TEXT(pEntries);
                aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_FLDTYP_CONDITIONED_TEXT;
            }
            break;
            case PROPERTY_MAP_FLDTYP_HIDDEN_TEXT:
            {
                static SfxItemPropertySet aPROPERTY_MAP_FLDTYP_HIDDEN_TEXT(pEntries);
                aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_FLDTYP_HIDDEN_TEXT;
            }
            break;
            case PROPERTY_MAP_FLDTYP_ANNOTATION :
            {
                static SfxItemPropertySet aPROPERTY_MAP_FLDTYP_ANNOTATION(pEntries);
                aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_FLDTYP_ANNOTATION;
            }
            break;
            case PROPERTY_MAP_FLDTYP_INPUT:
            {
                static SfxItemPropertySet aPROPERTY_MAP_FLDTYP_INPUT(pEntries);
                aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_FLDTYP_INPUT;
            }
            break;
            case PROPERTY_MAP_FLDTYP_MACRO:
            {
                static SfxItemPropertySet aPROPERTY_MAP_FLDTYP_MACRO(pEntries);
                aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_FLDTYP_MACRO;
            }
            break;
            case PROPERTY_MAP_FLDTYP_DDE:
            {
                static SfxItemPropertySet aPROPERTY_MAP_FLDTYP_DDE(pEntries);
                aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_FLDTYP_DDE;
            }
            break;
            case PROPERTY_MAP_FLDTYP_HIDDEN_PARA:
            {
                static SfxItemPropertySet aPROPERTY_MAP_FLDTYP_HIDDEN_PARA(pEntries);
                aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_FLDTYP_HIDDEN_PARA;
            }
            break;
            case PROPERTY_MAP_FLDTYP_DOC_INFO :
            {
                static SfxItemPropertySet aPROPERTY_MAP_FLDTYP_DOC_INFO(pEntries);
                aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_FLDTYP_DOC_INFO;
            }
            break;
            case PROPERTY_MAP_FLDTYP_TEMPLATE_NAME:
            {
                static SfxItemPropertySet aPROPERTY_MAP_FLDTYP_TEMPLATE_NAME(pEntries);
                aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_FLDTYP_TEMPLATE_NAME;
            }
            break;
            case PROPERTY_MAP_FLDTYP_USER_EXT :
            {
                static SfxItemPropertySet aPROPERTY_MAP_FLDTYP_USER_EXT(pEntries);
                aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_FLDTYP_USER_EXT;
            }
            break;
            case PROPERTY_MAP_FLDTYP_REF_PAGE_SET:
            {
                static SfxItemPropertySet aPROPERTY_MAP_FLDTYP_REF_PAGE_SET(pEntries);
                aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_FLDTYP_REF_PAGE_SET;
            }
            break;
            case PROPERTY_MAP_FLDTYP_REF_PAGE_GET:
            {
                static SfxItemPropertySet aPROPERTY_MAP_FLDTYP_REF_PAGE_GET(pEntries);
                aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_FLDTYP_REF_PAGE_GET;
            }
            break;
            case PROPERTY_MAP_FLDTYP_JUMP_EDIT:
            {
                static SfxItemPropertySet aPROPERTY_MAP_FLDTYP_JUMP_EDIT(pEntries);
                aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_FLDTYP_JUMP_EDIT;
            }
            break;
            case PROPERTY_MAP_FLDTYP_SCRIPT:
            {
                static SfxItemPropertySet aPROPERTY_MAP_FLDTYP_SCRIPT(pEntries);
                aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_FLDTYP_SCRIPT;
            }
            break;
            case PROPERTY_MAP_FLDTYP_DATABASE_NEXT_SET:
            {
                static SfxItemPropertySet aPROPERTY_MAP_FLDTYP_DATABASE_NEXT_SET(pEntries);
                aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_FLDTYP_DATABASE_NEXT_SET;
            }
            break;
            case PROPERTY_MAP_FLDTYP_DATABASE_NUM_SET:
            {
                static SfxItemPropertySet aPROPERTY_MAP_FLDTYP_DATABASE_NUM_SET(pEntries);
                aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_FLDTYP_DATABASE_NUM_SET;
            }
            break;
            case PROPERTY_MAP_FLDTYP_DATABASE_SET_NUM:
            {
                static SfxItemPropertySet aPROPERTY_MAP_FLDTYP_DATABASE_SET_NUM(pEntries);
                aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_FLDTYP_DATABASE_SET_NUM;
            }
            break;
            case PROPERTY_MAP_FLDTYP_DATABASE:
            {
                static SfxItemPropertySet aPROPERTY_MAP_FLDTYP_DATABASE(pEntries);
                aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_FLDTYP_DATABASE;
            }
            break;
            case PROPERTY_MAP_FLDTYP_DATABASE_NAME:
            {
                static SfxItemPropertySet aPROPERTY_MAP_FLDTYP_DATABASE_NAME(pEntries);
                aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_FLDTYP_DATABASE_NAME;
            }
            break;
            case PROPERTY_MAP_FLDTYP_DOCSTAT:
            {
                static SfxItemPropertySet aPROPERTY_MAP_FLDTYP_DOCSTAT(pEntries);
                aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_FLDTYP_DOCSTAT;
            }
            break;
            case PROPERTY_MAP_FLDTYP_DOCINFO_AUTHOR:
            {
                static SfxItemPropertySet aPROPERTY_MAP_FLDTYP_DOCINFO_AUTHOR(pEntries);
                aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_FLDTYP_DOCINFO_AUTHOR;
            }
            break;
            case PROPERTY_MAP_FLDTYP_DOCINFO_DATE_TIME:
            {
                static SfxItemPropertySet aPROPERTY_MAP_FLDTYP_DOCINFO_DATE_TIME(pEntries);
                aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_FLDTYP_DOCINFO_DATE_TIME;
            }
            break;
            case PROPERTY_MAP_FLDTYP_DOCINFO_CHANGE_DATE_TIME:
            {
                static SfxItemPropertySet aPROPERTY_MAP_FLDTYP_DOCINFO_CHANGE_DATE_TIME(pEntries);
                aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_FLDTYP_DOCINFO_CHANGE_DATE_TIME;
            }
            break;
            case PROPERTY_MAP_FLDTYP_DOCINFO_CREATE_DATE_TIME:
            {
                static SfxItemPropertySet aPROPERTY_MAP_FLDTYP_DOCINFO_CREATE_DATE_TIME(pEntries);
                aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_FLDTYP_DOCINFO_CREATE_DATE_TIME;
            }
            break;
            case PROPERTY_MAP_FLDTYP_DOCINFO_EDIT_TIME:
            {
                static SfxItemPropertySet aPROPERTY_MAP_FLDTYP_DOCINFO_EDIT_TIME(pEntries);
                aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_FLDTYP_DOCINFO_EDIT_TIME;
            }
            break;
            case PROPERTY_MAP_FLDTYP_DOCINFO_MISC :
            {
                static SfxItemPropertySet aPROPERTY_MAP_FLDTYP_DOCINFO_MISC(pEntries);
                aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_FLDTYP_DOCINFO_MISC;
            }
            break;
            case PROPERTY_MAP_FLDTYP_DOCINFO_REVISION:
            {
                static SfxItemPropertySet aPROPERTY_MAP_FLDTYP_DOCINFO_REVISION(pEntries);
                aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_FLDTYP_DOCINFO_REVISION;
            }
            break;
            case PROPERTY_MAP_FLDTYP_COMBINED_CHARACTERS:
            {
                static SfxItemPropertySet aPROPERTY_MAP_FLDTYP_COMBINED_CHARACTERS(pEntries);
                aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_FLDTYP_COMBINED_CHARACTERS;
            }
            break;
            case PROPERTY_MAP_FLDTYP_DUMMY_0:
            {
                static SfxItemPropertySet aPROPERTY_MAP_FLDTYP_DUMMY_0(pEntries);
                aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_FLDTYP_DUMMY_0;
            }
            break;
            case PROPERTY_MAP_FLDTYP_TABLE_FORMULA:
            {
                static SfxItemPropertySet aPROPERTY_MAP_FLDTYP_TABLE_FORMULA(pEntries);
                aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_FLDTYP_TABLE_FORMULA;
            }
            break;
            case PROPERTY_MAP_FLDMSTR_USER:
            {
                static SfxItemPropertySet aPROPERTY_MAP_FLDMSTR_USER(pEntries);
                aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_FLDMSTR_USER;
            }
            break;
            case PROPERTY_MAP_FLDMSTR_DDE:
            {
                static SfxItemPropertySet aPROPERTY_MAP_FLDMSTR_DDE(pEntries);
                aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_FLDMSTR_DDE;
            }
            break;
            case PROPERTY_MAP_FLDMSTR_SET_EXP:
            {
                static SfxItemPropertySet aPROPERTY_MAP_FLDMSTR_SET_EXP(pEntries);
                aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_FLDMSTR_SET_EXP;
            }
            break;
            case PROPERTY_MAP_FLDMSTR_DATABASE:
            {
                static SfxItemPropertySet aPROPERTY_MAP_FLDMSTR_DATABASE(pEntries);
                aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_FLDMSTR_DATABASE;
            }
            break;
            case PROPERTY_MAP_FLDMSTR_DUMMY0:
            {
                static SfxItemPropertySet aPROPERTY_MAP_FLDMSTR_DUMMY0(pEntries);
                aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_FLDMSTR_DUMMY0;
            }
            break;
            case PROPERTY_MAP_FLDTYP_BIBLIOGRAPHY:
            {
                static SfxItemPropertySet aPROPERTY_MAP_FLDTYP_BIBLIOGRAPHY(pEntries);
                aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_FLDTYP_BIBLIOGRAPHY;
            }
            break;
            case PROPERTY_MAP_FLDMSTR_BIBLIOGRAPHY:
            {
                static SfxItemPropertySet aPROPERTY_MAP_FLDMSTR_BIBLIOGRAPHY(pEntries);
                aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_FLDMSTR_BIBLIOGRAPHY;
            }
            break;
            case PROPERTY_MAP_TEXT:
            {
                static SfxItemPropertySet aPROPERTY_MAP_TEXT(pEntries);
                aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_TEXT;
            }
            break;
            case PROPERTY_MAP_REDLINE_PORTION:
            {
                static SfxItemPropertySet aPROPERTY_MAP_REDLINE_PORTION(pEntries);
                aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_REDLINE_PORTION;
            }
            break;
            case PROPERTY_MAP_MAILMERGE:
            {
                static SfxItemPropertySet aPROPERTY_MAP_MAILMERGE(pEntries);
                aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_MAILMERGE;
            }
            break;
            case PROPERTY_MAP_FLDTYP_DROPDOWN:
            {
                static SfxItemPropertySet aPROPERTY_MAP_FLDTYP_DROPDOWN(pEntries);
                aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_FLDTYP_DROPDOWN;
            }
            break;
            case PROPERTY_MAP_CHART2_DATA_SEQUENCE:
            {
                static SfxItemPropertySet aPROPERTY_MAP_CHART2_DATA_SEQUENCE(pEntries);
                aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_CHART2_DATA_SEQUENCE;
            }
            break;
            case PROPERTY_MAP_TEXT_VIEW:
            {
                static SfxItemPropertySet aPROPERTY_MAP_TEXT_VIEW(pEntries);
                aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_TEXT_VIEW;
            }
            break;
            case PROPERTY_MAP_CONDITIONAL_PARA_STYLE:
            {
                static SfxItemPropertySet aPROPERTY_MAP_CONDITIONAL_PARA_STYLE(pEntries);
                aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_CONDITIONAL_PARA_STYLE;
            }
            break;
            case PROPERTY_MAP_CHAR_AUTO_STYLE:
            {
                static SfxItemPropertySet aPROPERTY_MAP_CHAR_AUTO_STYLE(pEntries);
                aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_CHAR_AUTO_STYLE;
            }
            break;
            case PROPERTY_MAP_RUBY_AUTO_STYLE:
            {
                static SfxItemPropertySet aPROPERTY_MAP_RUBY_AUTO_STYLE(pEntries);
                aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_RUBY_AUTO_STYLE;
            }
            break;
            case PROPERTY_MAP_PARA_AUTO_STYLE:
            {
                static SfxItemPropertySet aPROPERTY_MAP_PARA_AUTO_STYLE(pEntries);
                aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_PARA_AUTO_STYLE;
            }
            break;
            case PROPERTY_MAP_FLDTYP_DOCINFO_CUSTOM:
            {
                static SfxItemPropertySet aPROPERTY_MAP_FLDTYP_DOCINFO_CUSTOM(pEntries);
                aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_FLDTYP_DOCINFO_CUSTOM;
            }
            break;
            case PROPERTY_MAP_METAFIELD:
            {
                static SfxItemPropertySet aPROPERTY_MAP_METAFIELD(pEntries);
                aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_METAFIELD;
            }
            break;
        }
    }
    return aPropertySetArr[nPropertyId];
}

sal_Bool SwItemPropertySet::FillItem(SfxItemSet& /*rSet*/, sal_uInt16 /*nWhich*/, sal_Bool /*bGetProperty*/) const
{
    sal_Bool bRet = sal_False;
    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
