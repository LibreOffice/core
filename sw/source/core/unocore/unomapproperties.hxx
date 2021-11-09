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
#ifndef INCLUDED_SW_INC_UNOMAPPROPERTIES_HXX
#define INCLUDED_SW_INC_UNOMAPPROPERTIES_HXX

// These are the unomap common properties used by unomap?.cxx

#ifndef MID_TXT_LMARGIN
#define MID_TXT_LMARGIN 11
#endif

#define STANDARD_FONT_PROPERTIES \
    { UNO_NAME_CHAR_HEIGHT, RES_CHRATR_FONTSIZE  ,  cppu::UnoType<float>::get(),         PropertyAttribute::MAYBEVOID, MID_FONTHEIGHT|CONVERT_TWIPS},      \
    { UNO_NAME_CHAR_WEIGHT, RES_CHRATR_WEIGHT    ,  cppu::UnoType<float>::get(),             PropertyAttribute::MAYBEVOID, MID_WEIGHT},                    \
    { UNO_NAME_CHAR_FONT_NAME, RES_CHRATR_FONT,       cppu::UnoType<OUString>::get(),  PropertyAttribute::MAYBEVOID, MID_FONT_FAMILY_NAME },                  \
    { UNO_NAME_CHAR_FONT_STYLE_NAME, RES_CHRATR_FONT,     cppu::UnoType<OUString>::get(), PropertyAttribute::MAYBEVOID, MID_FONT_STYLE_NAME },                    \
    { UNO_NAME_CHAR_FONT_FAMILY, RES_CHRATR_FONT,     cppu::UnoType<sal_Int16>::get(),                   PropertyAttribute::MAYBEVOID, MID_FONT_FAMILY   },    \
    { UNO_NAME_CHAR_FONT_CHAR_SET, RES_CHRATR_FONT,       cppu::UnoType<sal_Int16>::get(),   PropertyAttribute::MAYBEVOID, MID_FONT_CHAR_SET },                    \
    { UNO_NAME_CHAR_FONT_PITCH, RES_CHRATR_FONT,      cppu::UnoType<sal_Int16>::get(),                   PropertyAttribute::MAYBEVOID, MID_FONT_PITCH   },     \
    { UNO_NAME_CHAR_POSTURE, RES_CHRATR_POSTURE   ,  cppu::UnoType<css::awt::FontSlant>::get(),        PropertyAttribute::MAYBEVOID, MID_POSTURE},                   \
    { UNO_NAME_RSID, RES_CHRATR_RSID, cppu::UnoType<sal_Int32>::get(), PropertyAttribute::MAYBEVOID, 0 }, \
    { UNO_NAME_CHAR_LOCALE,           RES_CHRATR_LANGUAGE,    cppu::UnoType<css::lang::Locale>::get(),          PropertyAttribute::MAYBEVOID,   MID_LANG_LOCALE }, \
    { UNO_NAME_CHAR_INTEROP_GRAB_BAG, RES_CHRATR_GRABBAG,     cppu::UnoType< cppu::UnoSequenceType<css::beans::PropertyValue> >::get(),   PROPERTY_NONE,                  0               }, \

#define CJK_FONT_PROPERTIES \
    { UNO_NAME_CHAR_HEIGHT_ASIAN, RES_CHRATR_CJK_FONTSIZE  ,  cppu::UnoType<float>::get(),           PropertyAttribute::MAYBEVOID, MID_FONTHEIGHT|CONVERT_TWIPS},   \
    { UNO_NAME_CHAR_WEIGHT_ASIAN, RES_CHRATR_CJK_WEIGHT    ,  cppu::UnoType<float>::get(),           PropertyAttribute::MAYBEVOID, MID_WEIGHT},                 \
    { UNO_NAME_CHAR_FONT_NAME_ASIAN, RES_CHRATR_CJK_FONT,     cppu::UnoType<OUString>::get(),  PropertyAttribute::MAYBEVOID, MID_FONT_FAMILY_NAME },               \
    { UNO_NAME_CHAR_FONT_STYLE_NAME_ASIAN, RES_CHRATR_CJK_FONT,       cppu::UnoType<OUString>::get(), PropertyAttribute::MAYBEVOID, MID_FONT_STYLE_NAME },                 \
    { UNO_NAME_CHAR_FONT_FAMILY_ASIAN, RES_CHRATR_CJK_FONT,       cppu::UnoType<sal_Int16>::get(),                   PropertyAttribute::MAYBEVOID, MID_FONT_FAMILY   },    \
    { UNO_NAME_CHAR_FONT_CHAR_SET_ASIAN, RES_CHRATR_CJK_FONT,     cppu::UnoType<sal_Int16>::get(),   PropertyAttribute::MAYBEVOID, MID_FONT_CHAR_SET },                 \
    { UNO_NAME_CHAR_FONT_PITCH_ASIAN, RES_CHRATR_CJK_FONT,        cppu::UnoType<sal_Int16>::get(),                   PropertyAttribute::MAYBEVOID, MID_FONT_PITCH   },     \
    { UNO_NAME_CHAR_POSTURE_ASIAN, RES_CHRATR_CJK_POSTURE   ,  cppu::UnoType<css::awt::FontSlant>::get(),          PropertyAttribute::MAYBEVOID, MID_POSTURE},                \
    { UNO_NAME_CHAR_LOCALE_ASIAN, RES_CHRATR_CJK_LANGUAGE ,   cppu::UnoType<css::lang::Locale>::get()  ,        PropertyAttribute::MAYBEVOID,  MID_LANG_LOCALE },

#define CTL_FONT_PROPERTIES \
    { UNO_NAME_CHAR_HEIGHT_COMPLEX, RES_CHRATR_CTL_FONTSIZE  ,  cppu::UnoType<float>::get(),         PropertyAttribute::MAYBEVOID, MID_FONTHEIGHT|CONVERT_TWIPS},\
    { UNO_NAME_CHAR_WEIGHT_COMPLEX, RES_CHRATR_CTL_WEIGHT    ,  cppu::UnoType<float>::get(),             PropertyAttribute::MAYBEVOID, MID_WEIGHT},              \
    { UNO_NAME_CHAR_FONT_NAME_COMPLEX, RES_CHRATR_CTL_FONT,       cppu::UnoType<OUString>::get(),  PropertyAttribute::MAYBEVOID, MID_FONT_FAMILY_NAME },            \
    { UNO_NAME_CHAR_FONT_STYLE_NAME_COMPLEX, RES_CHRATR_CTL_FONT,     cppu::UnoType<OUString>::get(), PropertyAttribute::MAYBEVOID, MID_FONT_STYLE_NAME },              \
    { UNO_NAME_CHAR_FONT_FAMILY_COMPLEX, RES_CHRATR_CTL_FONT,     cppu::UnoType<sal_Int16>::get(),                   PropertyAttribute::MAYBEVOID, MID_FONT_FAMILY   },    \
    { UNO_NAME_CHAR_FONT_CHAR_SET_COMPLEX, RES_CHRATR_CTL_FONT,       cppu::UnoType<sal_Int16>::get(),   PropertyAttribute::MAYBEVOID, MID_FONT_CHAR_SET },              \
    { UNO_NAME_CHAR_FONT_PITCH_COMPLEX, RES_CHRATR_CTL_FONT,      cppu::UnoType<sal_Int16>::get(),                   PropertyAttribute::MAYBEVOID, MID_FONT_PITCH   },     \
    { UNO_NAME_CHAR_POSTURE_COMPLEX, RES_CHRATR_CTL_POSTURE   ,  cppu::UnoType<css::awt::FontSlant>::get(),        PropertyAttribute::MAYBEVOID, MID_POSTURE},             \
    { UNO_NAME_CHAR_LOCALE_COMPLEX, RES_CHRATR_CTL_LANGUAGE ,   cppu::UnoType<css::lang::Locale>::get()  ,          PropertyAttribute::MAYBEVOID,  MID_LANG_LOCALE },

#define REDLINE_NODE_PROPERTIES \
    { UNO_NAME_START_REDLINE, FN_UNO_REDLINE_NODE_START , cppu::UnoType< cppu::UnoSequenceType<css::beans::PropertyValue> >::get(),   PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY,  0xbf }, \
    { UNO_NAME_END_REDLINE, FN_UNO_REDLINE_NODE_END ,     cppu::UnoType< cppu::UnoSequenceType<css::beans::PropertyValue> >::get(),       PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY,  0xbf },

#define REDLINE_PROPERTIES(readonly) \
    { UNO_NAME_REDLINE_AUTHOR, 0, cppu::UnoType<OUString>::get(),                     PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY,   0},\
    { UNO_NAME_REDLINE_DATE_TIME, 0, cppu::UnoType<css::util::DateTime>::get(),                  PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY,   0},\
    { UNO_NAME_REDLINE_COMMENT, 0, cppu::UnoType<OUString>::get(),                        PropertyAttribute::MAYBEVOID|readonly,   0},\
    { UNO_NAME_REDLINE_DESCRIPTION, 0, cppu::UnoType<OUString>::get(), PropertyAttribute::MAYBEVOID | PropertyAttribute::READONLY, 0}, \
    { UNO_NAME_REDLINE_TYPE, 0, cppu::UnoType<OUString>::get(),                       PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY,   0},\
    { UNO_NAME_REDLINE_SUCCESSOR_DATA, 0, cppu::UnoType< cppu::UnoSequenceType<css::beans::PropertyValue> >::get(),    PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY,   0},\
    { UNO_NAME_REDLINE_IDENTIFIER, 0, cppu::UnoType<OUString>::get(),                         PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY, 0},\
    { UNO_NAME_IS_IN_HEADER_FOOTER, 0, cppu::UnoType<bool>::get(),                             PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY, 0},\
    { UNO_NAME_REDLINE_TEXT, 0, cppu::UnoType<css::text::XText>::get(),                    PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY,   0},\
    { UNO_NAME_MERGE_LAST_PARA, 0, cppu::UnoType<bool>::get(),                             PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY, 0},

#define COMMON_CRSR_PARA_PROPERTIES_FN_ONLY \
        { UNO_NAME_PARA_STYLE_NAME, FN_UNO_PARA_STYLE,        cppu::UnoType<OUString>::get(),                PropertyAttribute::MAYBEVOID,     0},                                                       \
        { UNO_NAME_PAGE_STYLE_NAME, FN_UNO_PAGE_STYLE,        cppu::UnoType<OUString>::get(),         PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY,   0},                       \
        { UNO_NAME_NUMBERING_IS_NUMBER, FN_UNO_IS_NUMBER,     cppu::UnoType<bool>::get()  ,       PropertyAttribute::MAYBEVOID,     0},                                                                 \
        { UNO_NAME_NUMBERING_LEVEL, FN_UNO_NUM_LEVEL,     cppu::UnoType<sal_Int16>::get(),           PropertyAttribute::MAYBEVOID, 0},                                                            \
        { UNO_NAME_NUMBERING_RULES, FN_UNO_NUM_RULES,     cppu::UnoType<css::container::XIndexReplace>::get(),  PropertyAttribute::MAYBEVOID, CONVERT_TWIPS},                        \
        { UNO_NAME_NUMBERING_START_VALUE, FN_UNO_NUM_START_VALUE, cppu::UnoType<sal_Int16>::get(),           PropertyAttribute::MAYBEVOID, CONVERT_TWIPS},                                                \
        { UNO_NAME_DOCUMENT_INDEX, FN_UNO_DOCUMENT_INDEX, cppu::UnoType<css::text::XDocumentIndex>::get(), PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY ,0 },            \
        { UNO_NAME_TEXT_TABLE, FN_UNO_TEXT_TABLE,     cppu::UnoType<css::text::XTextTable>::get(),     PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY ,0 },               \
        { UNO_NAME_CELL, FN_UNO_CELL,         cppu::UnoType<css::table::XCell>::get(),         PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY ,0 },                     \
        { UNO_NAME_TEXT_FRAME, FN_UNO_TEXT_FRAME,     cppu::UnoType<css::text::XTextFrame>::get(),        PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY ,0 },                     \
        { UNO_NAME_TEXT_SECTION, FN_UNO_TEXT_SECTION, cppu::UnoType<css::text::XTextSection>::get(),  PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY ,0 },                    \
        { UNO_NAME_TEXT_PARAGRAPH, FN_UNO_TEXT_PARAGRAPH, cppu::UnoType<css::text::XTextContent>::get(),  PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY ,0 },                    \
        { UNO_NAME_PARA_CHAPTER_NUMBERING_LEVEL, FN_UNO_PARA_CHAPTER_NUMBERING_LEVEL,cppu::UnoType<sal_Int8>::get(), PropertyAttribute::MAYBEVOID, 0},                                                     \
        { UNO_NAME_PARA_CONDITIONAL_STYLE_NAME, FN_UNO_PARA_CONDITIONAL_STYLE_NAME, cppu::UnoType<OUString>::get(),      PropertyAttribute::READONLY, 0},                                                     \
        { UNO_NAME_LIST_ID, FN_UNO_LIST_ID, cppu::UnoType<OUString>::get(), PropertyAttribute::MAYBEVOID, 0}, \
        { UNO_NAME_PARA_IS_NUMBERING_RESTART, FN_NUMBER_NEWSTART,     cppu::UnoType<bool>::get(),     PropertyAttribute::MAYBEVOID, 0 }, \
        { UNO_NAME_PARA_CONTINUEING_PREVIOUS_SUB_TREE, FN_UNO_PARA_CONT_PREV_SUBTREE, cppu::UnoType<bool>::get(), PropertyAttribute::READONLY, 0 }, \
        { UNO_NAME_PARA_LIST_LABEL_STRING, FN_UNO_PARA_NUM_STRING, cppu::UnoType<OUString>::get(), PropertyAttribute::READONLY, 0 }, \
        { UNO_NAME_PARA_LIST_AUTO_FORMAT, FN_UNO_PARA_NUM_AUTO_FORMAT, cppu::UnoType<cppu::UnoSequenceType<css::beans::NamedValue>>::get(), PropertyAttribute::MAYBEVOID, 0 }, \
        { UNO_NAME_OUTLINE_LEVEL, RES_PARATR_OUTLINELEVEL,        cppu::UnoType<sal_Int16>::get(),                PropertyAttribute::MAYBEVOID,     0}, \
        { UNO_NAME_OUTLINE_CONTENT_VISIBLE, RES_PARATR_GRABBAG, cppu::UnoType<bool>::get(), PropertyAttribute::MAYBEVOID, 0 },

#define COMMON_HYPERLINK_PROPERTIES \
        { UNO_NAME_HYPER_LINK_U_R_L, RES_TXTATR_INETFMT,          cppu::UnoType<OUString>::get(), PropertyAttribute::MAYBEVOID ,MID_URL_URL},                \
        { UNO_NAME_HYPER_LINK_TARGET, RES_TXTATR_INETFMT,         cppu::UnoType<OUString>::get(), PropertyAttribute::MAYBEVOID ,MID_URL_TARGET},             \
        { UNO_NAME_HYPER_LINK_NAME, RES_TXTATR_INETFMT,           cppu::UnoType<OUString>::get(), PropertyAttribute::MAYBEVOID ,MID_URL_HYPERLINKNAME  },    \
        { UNO_NAME_UNVISITED_CHAR_STYLE_NAME, RES_TXTATR_INETFMT, cppu::UnoType<OUString>::get(), PropertyAttribute::MAYBEVOID ,MID_URL_UNVISITED_FMT   },   \
        { UNO_NAME_VISITED_CHAR_STYLE_NAME, RES_TXTATR_INETFMT,   cppu::UnoType<OUString>::get(), PropertyAttribute::MAYBEVOID ,MID_URL_VISITED_FMT  },

// same as COMMON_CRSR_PARA_PROPERTIES_WITHOUT_FN but without
// UNO_NAME_BREAK_TYPE and UNO_NAME_PAGE_DESC_NAME which can not be used
// by the SwXTextTableCursor
#define COMMON_CRSR_PARA_PROPERTIES_WITHOUT_FN_01 \
        { UNO_NAME_PARRSID,                             RES_PARATR_RSID,               cppu::UnoType<sal_Int32>::get(),         PropertyAttribute::MAYBEVOID, 0                                      }, \
        { UNO_NAME_PARA_IS_HYPHENATION,                 RES_PARATR_HYPHENZONE,         cppu::UnoType<bool>::get(),       PropertyAttribute::MAYBEVOID, MID_IS_HYPHEN                          }, \
        { UNO_NAME_PARA_HYPHENATION_NO_CAPS,            RES_PARATR_HYPHENZONE,         cppu::UnoType<bool>::get(),       PropertyAttribute::MAYBEVOID, MID_HYPHEN_NO_CAPS                     }, \
        { UNO_NAME_PARA_HYPHENATION_MAX_LEADING_CHARS,  RES_PARATR_HYPHENZONE,         cppu::UnoType<sal_Int16>::get(),         PropertyAttribute::MAYBEVOID, MID_HYPHEN_MIN_LEAD                    }, \
        { UNO_NAME_PARA_HYPHENATION_MAX_TRAILING_CHARS, RES_PARATR_HYPHENZONE,         cppu::UnoType<sal_Int16>::get(),         PropertyAttribute::MAYBEVOID, MID_HYPHEN_MIN_TRAIL                   }, \
        { UNO_NAME_PARA_HYPHENATION_MAX_HYPHENS,        RES_PARATR_HYPHENZONE,         cppu::UnoType<sal_Int16>::get(),         PropertyAttribute::MAYBEVOID, MID_HYPHEN_MAX_HYPHENS                 }, \
        { UNO_NAME_CHAR_AUTO_KERNING,                   RES_CHRATR_AUTOKERN,           cppu::UnoType<bool>::get(),       PropertyAttribute::MAYBEVOID, 0                                      }, \
        { UNO_NAME_CHAR_BACK_COLOR,                     RES_CHRATR_BACKGROUND,         cppu::UnoType<sal_Int32>::get(),         PropertyAttribute::MAYBEVOID, MID_BACK_COLOR                         }, \
        { UNO_NAME_CHAR_HIGHLIGHT,                      RES_CHRATR_HIGHLIGHT,          cppu::UnoType<sal_Int32>::get(), PropertyAttribute::MAYBEVOID, MID_BACK_COLOR                         }, \
        { UNO_NAME_PARA_BACK_COLOR,                     RES_BACKGROUND,                cppu::UnoType<sal_Int32>::get(),         PropertyAttribute::MAYBEVOID, MID_BACK_COLOR                         }, \
        { UNO_NAME_CHAR_CASE_MAP,                       RES_CHRATR_CASEMAP,            cppu::UnoType<sal_Int16>::get(),         PropertyAttribute::MAYBEVOID, 0                                      }, \
        { UNO_NAME_CHAR_COLOR,                          RES_CHRATR_COLOR,              cppu::UnoType<sal_Int32>::get(),         PropertyAttribute::MAYBEVOID, 0                                      }, \
        { UNO_NAME_CHAR_TRANSPARENCE,                   RES_CHRATR_COLOR,              cppu::UnoType<sal_Int16>::get(),         PropertyAttribute::MAYBEVOID, MID_COLOR_ALPHA }, \
        { UNO_NAME_CHAR_STRIKEOUT,                      RES_CHRATR_CROSSEDOUT,         cppu::UnoType<sal_Int16>::get(),         PropertyAttribute::MAYBEVOID, MID_CROSS_OUT                          }, \
        { UNO_NAME_CHAR_CROSSED_OUT,                    RES_CHRATR_CROSSEDOUT,         cppu::UnoType<bool>::get(),       PropertyAttribute::MAYBEVOID, MID_CROSSED_OUT                        }, \
        { UNO_NAME_CHAR_ESCAPEMENT,                     RES_CHRATR_ESCAPEMENT,         cppu::UnoType<sal_Int16>::get(),         PropertyAttribute::MAYBEVOID, MID_ESC                                }, \
        { UNO_NAME_CHAR_ESCAPEMENT_HEIGHT,              RES_CHRATR_ESCAPEMENT,         cppu::UnoType<sal_Int8>::get(),          PropertyAttribute::MAYBEVOID, MID_ESC_HEIGHT                         }, \
        { UNO_NAME_CHAR_AUTO_ESCAPEMENT,                RES_CHRATR_ESCAPEMENT,         cppu::UnoType<bool>::get(),       PropertyAttribute::MAYBEVOID, MID_AUTO_ESC                           }, \
        { UNO_NAME_CHAR_FLASH,                          RES_CHRATR_BLINK,              cppu::UnoType<bool>::get(),       PropertyAttribute::MAYBEVOID, 0                                      }, \
        { UNO_NAME_CHAR_HIDDEN,                         RES_CHRATR_HIDDEN,             cppu::UnoType<bool>::get(),       PropertyAttribute::MAYBEVOID, 0                                      }, \
        { UNO_NAME_CHAR_UNDERLINE,                      RES_CHRATR_UNDERLINE,          cppu::UnoType<sal_Int16>::get(),         PropertyAttribute::MAYBEVOID, MID_TL_STYLE                           }, \
        { UNO_NAME_CHAR_UNDERLINE_COLOR,                RES_CHRATR_UNDERLINE,          cppu::UnoType<sal_Int32>::get(),         PropertyAttribute::MAYBEVOID, MID_TL_COLOR                           }, \
        { UNO_NAME_CHAR_UNDERLINE_HAS_COLOR,            RES_CHRATR_UNDERLINE,          cppu::UnoType<bool>::get(),       PropertyAttribute::MAYBEVOID, MID_TL_HASCOLOR                        }, \
        { UNO_NAME_CHAR_OVERLINE,                       RES_CHRATR_OVERLINE,           cppu::UnoType<sal_Int16>::get(),         PropertyAttribute::MAYBEVOID, MID_TL_STYLE                           }, \
        { UNO_NAME_CHAR_OVERLINE_COLOR,                 RES_CHRATR_OVERLINE,           cppu::UnoType<sal_Int32>::get(),         PropertyAttribute::MAYBEVOID, MID_TL_COLOR                           }, \
        { UNO_NAME_CHAR_OVERLINE_HAS_COLOR,             RES_CHRATR_OVERLINE,           cppu::UnoType<bool>::get(),       PropertyAttribute::MAYBEVOID, MID_TL_HASCOLOR                        }, \
        { UNO_NAME_PARA_GRAPHIC_URL,                    RES_BACKGROUND,                cppu::UnoType<OUString>::get(),      PropertyAttribute::MAYBEVOID, MID_GRAPHIC_URL                        }, \
        { UNO_NAME_PARA_GRAPHIC,                        RES_BACKGROUND,                cppu::UnoType<css::graphic::XGraphic>::get(),      PropertyAttribute::MAYBEVOID, MID_GRAPHIC                        }, \
        { UNO_NAME_PARA_GRAPHIC_FILTER,                 RES_BACKGROUND,                cppu::UnoType<OUString>::get(),      PropertyAttribute::MAYBEVOID, MID_GRAPHIC_FILTER                     }, \
        { UNO_NAME_PARA_GRAPHIC_LOCATION,               RES_BACKGROUND,                cppu::UnoType<css::style::GraphicLocation>::get(),    PropertyAttribute::MAYBEVOID, MID_GRAPHIC_POSITION                   }, \
        { UNO_NAME_PARA_LEFT_MARGIN,                    RES_LR_SPACE,                  cppu::UnoType<sal_Int32>::get(),         PropertyAttribute::MAYBEVOID, MID_TXT_LMARGIN        | CONVERT_TWIPS }, \
        { UNO_NAME_PARA_RIGHT_MARGIN,                   RES_LR_SPACE,                  cppu::UnoType<sal_Int32>::get(),         PropertyAttribute::MAYBEVOID, MID_R_MARGIN           | CONVERT_TWIPS }, \
        { UNO_NAME_PARA_IS_AUTO_FIRST_LINE_INDENT,      RES_LR_SPACE,                  cppu::UnoType<bool>::get(),       PropertyAttribute::MAYBEVOID, MID_FIRST_AUTO                         }, \
        { UNO_NAME_PARA_FIRST_LINE_INDENT,              RES_LR_SPACE,                  cppu::UnoType<sal_Int32>::get(),         PropertyAttribute::MAYBEVOID, MID_FIRST_LINE_INDENT  | CONVERT_TWIPS }, \
        STANDARD_FONT_PROPERTIES \
        CJK_FONT_PROPERTIES \
        CTL_FONT_PROPERTIES \
        { UNO_NAME_CHAR_KERNING,                        RES_CHRATR_KERNING,            cppu::UnoType<sal_Int16>::get(),         PropertyAttribute::MAYBEVOID, CONVERT_TWIPS                          }, \
        { UNO_NAME_CHAR_NO_HYPHENATION,                 RES_CHRATR_NOHYPHEN,           cppu::UnoType<bool>::get(),       PropertyAttribute::MAYBEVOID, 0                                      }, \
        { UNO_NAME_CHAR_SHADOWED,                       RES_CHRATR_SHADOWED,           cppu::UnoType<bool>::get(),       PropertyAttribute::MAYBEVOID, 0                                      }, \
        { UNO_NAME_CHAR_CONTOURED,                      RES_CHRATR_CONTOUR,            cppu::UnoType<bool>::get(),       PropertyAttribute::MAYBEVOID, 0                                      }, \
        { UNO_NAME_DROP_CAP_FORMAT,                     RES_PARATR_DROP,               cppu::UnoType<css::style::DropCapFormat>::get(),    PropertyAttribute::MAYBEVOID, MID_DROPCAP_FORMAT     | CONVERT_TWIPS }, \
        { UNO_NAME_DROP_CAP_WHOLE_WORD,                 RES_PARATR_DROP,               cppu::UnoType<bool>::get(),       PropertyAttribute::MAYBEVOID, MID_DROPCAP_WHOLE_WORD                 }, \
        { UNO_NAME_DROP_CAP_CHAR_STYLE_NAME,            RES_PARATR_DROP,               cppu::UnoType<OUString>::get(),      PropertyAttribute::MAYBEVOID, MID_DROPCAP_CHAR_STYLE_NAME            }, \
        { UNO_NAME_PARA_KEEP_TOGETHER,                  RES_KEEP,                      cppu::UnoType<bool>::get(),       PropertyAttribute::MAYBEVOID, 0                                      }, \
        { UNO_NAME_PARA_SPLIT,                          RES_PARATR_SPLIT,              cppu::UnoType<bool>::get(),       PropertyAttribute::MAYBEVOID, 0                                      }, \
        { UNO_NAME_PARA_WIDOWS,                         RES_PARATR_WIDOWS,             cppu::UnoType<sal_Int8>::get(),          PropertyAttribute::MAYBEVOID, 0                                      }, \
        { UNO_NAME_PARA_ORPHANS,                        RES_PARATR_ORPHANS,            cppu::UnoType<sal_Int8>::get(),          PropertyAttribute::MAYBEVOID, 0                                      }, \
        { UNO_NAME_PAGE_NUMBER_OFFSET,                  RES_PAGEDESC,                  cppu::UnoType<sal_Int16>::get(),         PropertyAttribute::MAYBEVOID, MID_PAGEDESC_PAGENUMOFFSET             }, \
        { UNO_NAME_PARA_ADJUST,                         RES_PARATR_ADJUST,             cppu::UnoType<sal_Int16>::get(),         PropertyAttribute::MAYBEVOID, MID_PARA_ADJUST                        }, \
        { UNO_NAME_PARA_EXPAND_SINGLE_WORD,             RES_PARATR_ADJUST,             cppu::UnoType<bool>::get(),       PropertyAttribute::MAYBEVOID, MID_EXPAND_SINGLE                      }, \
        { UNO_NAME_PARA_LAST_LINE_ADJUST,               RES_PARATR_ADJUST,             cppu::UnoType<sal_Int16>::get(),         PropertyAttribute::MAYBEVOID, MID_LAST_LINE_ADJUST                   }, \
        { UNO_NAME_PARA_LINE_NUMBER_COUNT,              RES_LINENUMBER,                cppu::UnoType<bool>::get(),       PropertyAttribute::MAYBEVOID, MID_LINENUMBER_COUNT                   }, \
        { UNO_NAME_PARA_LINE_NUMBER_START_VALUE,        RES_LINENUMBER,                cppu::UnoType<sal_Int32>::get(),         PropertyAttribute::MAYBEVOID, MID_LINENUMBER_STARTVALUE              }, \
        { UNO_NAME_PARA_LINE_SPACING,                   RES_PARATR_LINESPACING,        cppu::UnoType<css::style::LineSpacing>::get(),     PropertyAttribute::MAYBEVOID, CONVERT_TWIPS                          }, \
        { UNO_NAME_PARA_REGISTER_MODE_ACTIVE,           RES_PARATR_REGISTER,           cppu::UnoType<bool>::get(),       PropertyAttribute::MAYBEVOID, 0                                      }, \
        { UNO_NAME_PARA_TOP_MARGIN,                     RES_UL_SPACE,                  cppu::UnoType<sal_Int32>::get(),         PropertyAttribute::MAYBEVOID, MID_UP_MARGIN          | CONVERT_TWIPS }, \
        { UNO_NAME_PARA_BOTTOM_MARGIN,                  RES_UL_SPACE,                  cppu::UnoType<sal_Int32>::get(),         PropertyAttribute::MAYBEVOID, MID_LO_MARGIN          | CONVERT_TWIPS }, \
        { UNO_NAME_PARA_CONTEXT_MARGIN,                 RES_UL_SPACE,                  cppu::UnoType<bool>::get(),       PropertyAttribute::MAYBEVOID, MID_CTX_MARGIN                         }, \
        { UNO_NAME_CHAR_BACK_TRANSPARENT,               RES_CHRATR_BACKGROUND,         cppu::UnoType<bool>::get(),       PropertyAttribute::MAYBEVOID, MID_GRAPHIC_TRANSPARENT                }, \
        { UNO_NAME_PARA_BACK_TRANSPARENT,               RES_BACKGROUND,                cppu::UnoType<bool>::get(),       PropertyAttribute::MAYBEVOID, MID_GRAPHIC_TRANSPARENT                }, \
        { UNO_NAME_NUMBERING_STYLE_NAME,                RES_PARATR_NUMRULE,            cppu::UnoType<OUString>::get(),      PropertyAttribute::MAYBEVOID, 0                                      }, \
        { UNO_NAME_CHAR_WORD_MODE,                      RES_CHRATR_WORDLINEMODE,       cppu::UnoType<bool>::get(),       PropertyAttribute::MAYBEVOID, 0                                      }, \
        { UNO_NAME_CHAR_LEFT_BORDER,                    RES_CHRATR_BOX,                cppu::UnoType<css::table::BorderLine>::get(),    PropertyAttribute::MAYBEVOID, LEFT_BORDER            | CONVERT_TWIPS }, \
        { UNO_NAME_CHAR_RIGHT_BORDER,                   RES_CHRATR_BOX,                cppu::UnoType<css::table::BorderLine>::get(),    PropertyAttribute::MAYBEVOID, RIGHT_BORDER           | CONVERT_TWIPS }, \
        { UNO_NAME_CHAR_TOP_BORDER,                     RES_CHRATR_BOX,                cppu::UnoType<css::table::BorderLine>::get(),    PropertyAttribute::MAYBEVOID, TOP_BORDER             | CONVERT_TWIPS }, \
        { UNO_NAME_CHAR_BOTTOM_BORDER,                  RES_CHRATR_BOX,                cppu::UnoType<css::table::BorderLine>::get(),    PropertyAttribute::MAYBEVOID, BOTTOM_BORDER          | CONVERT_TWIPS }, \
        { UNO_NAME_CHAR_BORDER_DISTANCE,                RES_CHRATR_BOX,                cppu::UnoType<sal_Int32>::get(),         PropertyAttribute::MAYBEVOID, BORDER_DISTANCE        | CONVERT_TWIPS }, \
        { UNO_NAME_CHAR_LEFT_BORDER_DISTANCE,           RES_CHRATR_BOX,                cppu::UnoType<sal_Int32>::get(),         PropertyAttribute::MAYBEVOID, LEFT_BORDER_DISTANCE   | CONVERT_TWIPS }, \
        { UNO_NAME_CHAR_RIGHT_BORDER_DISTANCE,          RES_CHRATR_BOX,                cppu::UnoType<sal_Int32>::get(),         PropertyAttribute::MAYBEVOID, RIGHT_BORDER_DISTANCE  | CONVERT_TWIPS }, \
        { UNO_NAME_CHAR_TOP_BORDER_DISTANCE,            RES_CHRATR_BOX,                cppu::UnoType<sal_Int32>::get(),         PropertyAttribute::MAYBEVOID, TOP_BORDER_DISTANCE    | CONVERT_TWIPS }, \
        { UNO_NAME_CHAR_BOTTOM_BORDER_DISTANCE,         RES_CHRATR_BOX,                cppu::UnoType<sal_Int32>::get(),         PropertyAttribute::MAYBEVOID, BOTTOM_BORDER_DISTANCE | CONVERT_TWIPS }, \
        { UNO_NAME_CHAR_SHADOW_FORMAT,                  RES_CHRATR_SHADOW,             cppu::UnoType<css::table::ShadowFormat>::get(),     PropertyAttribute::MAYBEVOID, CONVERT_TWIPS                          }, \
        { UNO_NAME_LEFT_BORDER,                         RES_BOX,                       cppu::UnoType<css::table::BorderLine>::get(),    PropertyAttribute::MAYBEVOID, LEFT_BORDER            | CONVERT_TWIPS }, \
        { UNO_NAME_RIGHT_BORDER,                        RES_BOX,                       cppu::UnoType<css::table::BorderLine>::get(),    PropertyAttribute::MAYBEVOID, RIGHT_BORDER           | CONVERT_TWIPS }, \
        { UNO_NAME_TOP_BORDER,                          RES_BOX,                       cppu::UnoType<css::table::BorderLine>::get(),    PropertyAttribute::MAYBEVOID, TOP_BORDER             | CONVERT_TWIPS }, \
        { UNO_NAME_BOTTOM_BORDER,                       RES_BOX,                       cppu::UnoType<css::table::BorderLine>::get(),    PropertyAttribute::MAYBEVOID, BOTTOM_BORDER          | CONVERT_TWIPS }, \
        { UNO_NAME_BORDER_DISTANCE,                     RES_BOX,                       cppu::UnoType<sal_Int32>::get(),         PropertyAttribute::MAYBEVOID, BORDER_DISTANCE        | CONVERT_TWIPS }, \
        { UNO_NAME_LEFT_BORDER_DISTANCE,                RES_BOX,                       cppu::UnoType<sal_Int32>::get(),         PropertyAttribute::MAYBEVOID, LEFT_BORDER_DISTANCE   | CONVERT_TWIPS }, \
        { UNO_NAME_RIGHT_BORDER_DISTANCE,               RES_BOX,                       cppu::UnoType<sal_Int32>::get(),         PropertyAttribute::MAYBEVOID, RIGHT_BORDER_DISTANCE  | CONVERT_TWIPS }, \
        { UNO_NAME_TOP_BORDER_DISTANCE,                 RES_BOX,                       cppu::UnoType<sal_Int32>::get(),         PropertyAttribute::MAYBEVOID, TOP_BORDER_DISTANCE    | CONVERT_TWIPS }, \
        { UNO_NAME_BOTTOM_BORDER_DISTANCE,              RES_BOX,                       cppu::UnoType<sal_Int32>::get(),         PropertyAttribute::MAYBEVOID, BOTTOM_BORDER_DISTANCE | CONVERT_TWIPS }, \
        { UNO_NAME_PARA_USER_DEFINED_ATTRIBUTES,        RES_UNKNOWNATR_CONTAINER,      cppu::UnoType<css::container::XNameContainer>::get(),    PropertyAttribute::MAYBEVOID, 0                                      }, \
        { UNO_NAME_TEXT_USER_DEFINED_ATTRIBUTES,        RES_TXTATR_UNKNOWN_CONTAINER,  cppu::UnoType<css::container::XNameContainer>::get(),    PropertyAttribute::MAYBEVOID, 0                                      }, \
        { UNO_NAME_PARA_SHADOW_FORMAT,                  RES_SHADOW,                    cppu::UnoType<css::table::ShadowFormat>::get(),     PROPERTY_NONE,                CONVERT_TWIPS                          }, \
        { UNO_NAME_CHAR_COMBINE_IS_ON,                  RES_CHRATR_TWO_LINES,          cppu::UnoType<bool>::get(),       PropertyAttribute::MAYBEVOID, MID_TWOLINES                           }, \
        { UNO_NAME_CHAR_COMBINE_PREFIX,                 RES_CHRATR_TWO_LINES,          cppu::UnoType<OUString>::get(),      PropertyAttribute::MAYBEVOID, MID_START_BRACKET                      }, \
        { UNO_NAME_CHAR_COMBINE_SUFFIX,                 RES_CHRATR_TWO_LINES,          cppu::UnoType<OUString>::get(),      PropertyAttribute::MAYBEVOID, MID_END_BRACKET                        }, \
        { UNO_NAME_CHAR_EMPHASIS,                       RES_CHRATR_EMPHASIS_MARK,      cppu::UnoType<sal_Int16>::get(),         PropertyAttribute::MAYBEVOID, MID_EMPHASIS                           }, \
        { UNO_NAME_PARA_IS_HANGING_PUNCTUATION,         RES_PARATR_HANGINGPUNCTUATION, cppu::UnoType<bool>::get(),       PropertyAttribute::MAYBEVOID, 0                                      }, \
        { UNO_NAME_PARA_IS_CHARACTER_DISTANCE,          RES_PARATR_SCRIPTSPACE,        cppu::UnoType<bool>::get(),       PropertyAttribute::MAYBEVOID, 0                                      }, \
        { UNO_NAME_PARA_IS_FORBIDDEN_RULES,             RES_PARATR_FORBIDDEN_RULES,    cppu::UnoType<bool>::get(),       PropertyAttribute::MAYBEVOID, 0                                      }, \
        { UNO_NAME_PARA_VERT_ALIGNMENT,                 RES_PARATR_VERTALIGN,          cppu::UnoType<sal_Int16>::get(),         PropertyAttribute::MAYBEVOID, 0                                      }, \
        { UNO_NAME_CHAR_ROTATION,                       RES_CHRATR_ROTATE,             cppu::UnoType<sal_Int16>::get(),         PropertyAttribute::MAYBEVOID, MID_ROTATE                             }, \
        { UNO_NAME_CHAR_ROTATION_IS_FIT_TO_LINE,        RES_CHRATR_ROTATE,             cppu::UnoType<bool>::get(),       PropertyAttribute::MAYBEVOID, MID_FITTOLINE                          }, \
        { UNO_NAME_CHAR_SCALE_WIDTH,                    RES_CHRATR_SCALEW,             cppu::UnoType<sal_Int16>::get(),         PropertyAttribute::MAYBEVOID, 0                                      }, \
        { UNO_NAME_RUBY_TEXT,                           RES_TXTATR_CJK_RUBY,           cppu::UnoType<OUString>::get(),      PropertyAttribute::MAYBEVOID, MID_RUBY_TEXT                          }, \
        { UNO_NAME_RUBY_ADJUST,                         RES_TXTATR_CJK_RUBY,           cppu::UnoType<sal_Int16>::get(),         PropertyAttribute::MAYBEVOID, MID_RUBY_ADJUST                        }, \
        { UNO_NAME_RUBY_CHAR_STYLE_NAME,                RES_TXTATR_CJK_RUBY,           cppu::UnoType<OUString>::get(),      PropertyAttribute::MAYBEVOID, MID_RUBY_CHARSTYLE                     }, \
        { UNO_NAME_RUBY_POSITION,                       RES_TXTATR_CJK_RUBY,           cppu::UnoType<sal_Int16>::get(),       PropertyAttribute::MAYBEVOID, MID_RUBY_POSITION}, \
        { UNO_NAME_RUBY_IS_ABOVE,                       RES_TXTATR_CJK_RUBY,           cppu::UnoType<bool>::get(),       PropertyAttribute::MAYBEVOID, MID_RUBY_ABOVE                         }, \
        { UNO_NAME_CHAR_RELIEF,                         RES_CHRATR_RELIEF,             cppu::UnoType<sal_Int16>::get(),         PropertyAttribute::MAYBEVOID, MID_RELIEF                             }, \
        { UNO_NAME_SNAP_TO_GRID,                        RES_PARATR_SNAPTOGRID,         cppu::UnoType<bool>::get(),       PropertyAttribute::MAYBEVOID, 0                                      }, \
        { UNO_NAME_PARA_IS_CONNECT_BORDER,              RES_PARATR_CONNECT_BORDER,     cppu::UnoType<bool>::get(),       PropertyAttribute::MAYBEVOID, 0                                      }, \
        { UNO_NAME_WRITING_MODE,                        RES_FRAMEDIR,                  cppu::UnoType<sal_Int16>::get(),         PROPERTY_NONE,                0                                      }, \
        { UNO_NAME_CHAR_SHADING_VALUE,                  RES_CHRATR_BACKGROUND,         cppu::UnoType<sal_Int32>::get(),         PROPERTY_NONE,                MID_SHADING_VALUE                      }, \
        { UNO_NAME_PARA_INTEROP_GRAB_BAG,               RES_PARATR_GRABBAG,            cppu::UnoType< cppu::UnoSequenceType<css::beans::PropertyValue> >::get(), PROPERTY_NONE,                0                                      }, \

#define COMMON_CRSR_PARA_PROPERTIES_WITHOUT_FN \
        COMMON_CRSR_PARA_PROPERTIES_WITHOUT_FN_01 \
        { UNO_NAME_BREAK_TYPE, RES_BREAK,                 cppu::UnoType<css::style::BreakType>::get(),           PropertyAttribute::MAYBEVOID, 0}, \
        { UNO_NAME_PAGE_DESC_NAME, RES_PAGEDESC,          cppu::UnoType<OUString>::get(),            PropertyAttribute::MAYBEVOID, MID_PAGEDESC_PAGEDESCNAME },

#define TABSTOPS_MAP_ENTRY                { UNO_NAME_TABSTOPS, RES_PARATR_TABSTOP,   cppu::UnoType< cppu::UnoSequenceType<css::style::TabStop> >::get(),   PropertyAttribute::MAYBEVOID, CONVERT_TWIPS},

#define COMMON_CRSR_PARA_PROPERTIES \
        COMMON_CRSR_PARA_PROPERTIES_FN_ONLY \
        COMMON_CRSR_PARA_PROPERTIES_WITHOUT_FN \
        COMMON_HYPERLINK_PROPERTIES \
        { UNO_NAME_CHAR_STYLE_NAME, RES_TXTATR_CHARFMT,     cppu::UnoType<OUString>::get(),         PropertyAttribute::MAYBEVOID,     0},\
        { UNO_NAME_CHAR_STYLE_NAMES, FN_UNO_CHARFMT_SEQUENCE,  cppu::UnoType< cppu::UnoSequenceType<OUString> >::get(),     PropertyAttribute::MAYBEVOID,     0}, \
        { UNO_NAME_CHAR_AUTO_STYLE_NAME, RES_TXTATR_AUTOFMT,     cppu::UnoType<OUString>::get(),         PropertyAttribute::MAYBEVOID,     0},\
        { UNO_NAME_PARA_AUTO_STYLE_NAME, RES_AUTO_STYLE,     cppu::UnoType<OUString>::get(),         PropertyAttribute::MAYBEVOID,     0},

#define COMMON_CRSR_PARA_PROPERTIES_2 \
        COMMON_CRSR_PARA_PROPERTIES_FN_ONLY \
        COMMON_CRSR_PARA_PROPERTIES_WITHOUT_FN

#define  COMPLETE_TEXT_CURSOR_MAP\
        COMMON_CRSR_PARA_PROPERTIES\
        { UNO_NAME_DOCUMENT_INDEX_MARK, FN_UNO_DOCUMENT_INDEX_MARK, cppu::UnoType<css::text::XDocumentIndexMark>::get(), PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY ,0 },\
        { UNO_NAME_TEXT_FIELD, FN_UNO_TEXT_FIELD,      cppu::UnoType<css::text::XTextField>::get(),  PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY ,0 },\
        { UNO_NAME_REFERENCE_MARK, FN_UNO_REFERENCE_MARK,  cppu::UnoType<css::text::XTextContent>::get(), PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY, 0 },\
        { UNO_NAME_FOOTNOTE, FN_UNO_FOOTNOTE,        cppu::UnoType<css::text::XFootnote>::get(),  PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY ,0 },\
        { UNO_NAME_ENDNOTE, FN_UNO_ENDNOTE,         cppu::UnoType<css::text::XFootnote>::get(),  PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY ,0 },\
        { UNO_NAME_HYPER_LINK_EVENTS, RES_TXTATR_INETFMT,     cppu::UnoType<css::container::XNameReplace>::get(), PropertyAttribute::MAYBEVOID, MID_URL_HYPERLINKEVENTS},\
        { UNO_NAME_NESTED_TEXT_CONTENT, FN_UNO_NESTED_TEXT_CONTENT, cppu::UnoType<css::text::XTextContent>::get(), PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY, 0 },\
        TABSTOPS_MAP_ENTRY

#define BASE_INDEX_PROPERTIES_\
        { UNO_NAME_TITLE, WID_IDX_TITLE,  cppu::UnoType<OUString>::get()  , PROPERTY_NONE,     0},\
        { UNO_NAME_NAME,  WID_IDX_NAME,   cppu::UnoType<OUString>::get()  , PROPERTY_NONE,     0},\
        { UNO_NAME_CONTENT_SECTION, WID_IDX_CONTENT_SECTION,  cppu::UnoType<css::text::XTextSection>::get()  , PropertyAttribute::READONLY,     0},\
        { UNO_NAME_HEADER_SECTION, WID_IDX_HEADER_SECTION,  cppu::UnoType<css::text::XTextSection>::get()  , PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY,     0},\

#define ANCHOR_TYPES_PROPERTY    { UNO_NAME_ANCHOR_TYPES, FN_UNO_ANCHOR_TYPES,    cppu::UnoType< cppu::UnoSequenceType<css::text::TextContentAnchorType> >::get(),PropertyAttribute::READONLY, 0xbf},

// #i18732# #i28701# #i73249#
// all users of COMMON_FRAME_PROPERTIES add the new XATTR_FILL_FIRST, XATTR_FILL_LAST FillStyle,
// thus it may be possible to remove the RES_BACKGROUND entries from SvxBrushItem completely (this includes
// all using UNO_NAME_BACK_* slots) in the future
#define COMMON_FRAME_PROPERTIES \
    { UNO_NAME_ANCHOR_PAGE_NO, RES_ANCHOR,            cppu::UnoType<sal_Int16>::get(),           PROPERTY_NONE, MID_ANCHOR_PAGENUM       },              \
    { UNO_NAME_ANCHOR_TYPE, RES_ANCHOR,           cppu::UnoType<css::text::TextContentAnchorType>::get(),            PROPERTY_NONE, MID_ANCHOR_ANCHORTYPE},             \
    { UNO_NAME_ANCHOR_FRAME, RES_ANCHOR,             cppu::UnoType<css::text::XTextFrame>::get(),    PropertyAttribute::MAYBEVOID, MID_ANCHOR_ANCHORFRAME},             \
    ANCHOR_TYPES_PROPERTY\
    { UNO_NAME_BACK_COLOR, RES_BACKGROUND,            cppu::UnoType<sal_Int32>::get(),           PROPERTY_NONE ,MID_BACK_COLOR        },                      \
    { UNO_NAME_BACK_COLOR_R_G_B, RES_BACKGROUND,      cppu::UnoType<sal_Int32>::get(), PROPERTY_NONE ,MID_BACK_COLOR_R_G_B},    \
    { UNO_NAME_BACK_COLOR_TRANSPARENCY, RES_BACKGROUND,      cppu::UnoType<sal_Int8>::get(), PROPERTY_NONE ,MID_BACK_COLOR_TRANSPARENCY},    \
    { UNO_NAME_FRAME_INTEROP_GRAB_BAG, RES_FRMATR_GRABBAG, cppu::UnoType< cppu::UnoSequenceType<css::beans::PropertyValue> >::get(), PROPERTY_NONE, 0}, \
    { UNO_NAME_CONTENT_PROTECTED, RES_PROTECT,            cppu::UnoType<bool>::get(),             PROPERTY_NONE, MID_PROTECT_CONTENT  },                          \
    { UNO_NAME_FRAME_STYLE_NAME, FN_UNO_FRAME_STYLE_NAME,cppu::UnoType<OUString>::get(),         PROPERTY_NONE, 0},                                   \
    { UNO_NAME_BACK_GRAPHIC_URL, RES_BACKGROUND,      cppu::UnoType<OUString>::get(),        PROPERTY_NONE ,MID_GRAPHIC_URL    },                 \
    { UNO_NAME_BACK_GRAPHIC, RES_BACKGROUND, cppu::UnoType<css::graphic::XGraphic>::get(), PROPERTY_NONE, MID_GRAPHIC }, \
    { UNO_NAME_BACK_GRAPHIC_FILTER, RES_BACKGROUND,       cppu::UnoType<OUString>::get(),        PROPERTY_NONE ,MID_GRAPHIC_FILTER    },              \
    { UNO_NAME_BACK_GRAPHIC_LOCATION, RES_BACKGROUND,         cppu::UnoType<css::style::GraphicLocation>::get(), PROPERTY_NONE ,MID_GRAPHIC_POSITION}, \
    { UNO_NAME_BACK_GRAPHIC_TRANSPARENCY, RES_BACKGROUND,      cppu::UnoType<sal_Int8>::get(), PROPERTY_NONE ,MID_GRAPHIC_TRANSPARENCY},    \
    { UNO_NAME_LEFT_MARGIN, RES_LR_SPACE,             cppu::UnoType<sal_Int32>::get(),           PROPERTY_NONE, MID_L_MARGIN|CONVERT_TWIPS},            \
    { UNO_NAME_RIGHT_MARGIN, RES_LR_SPACE,            cppu::UnoType<sal_Int32>::get(),           PROPERTY_NONE, MID_R_MARGIN|CONVERT_TWIPS},            \
    { UNO_NAME_WIDTH, RES_FRM_SIZE,           cppu::UnoType<sal_Int32>::get()  ,         PROPERTY_NONE, MID_FRMSIZE_WIDTH|CONVERT_TWIPS},\
    { UNO_NAME_HEIGHT, RES_FRM_SIZE,          cppu::UnoType<sal_Int32>::get()  ,         PROPERTY_NONE, MID_FRMSIZE_HEIGHT|CONVERT_TWIPS},\
    { UNO_NAME_HORI_ORIENT, RES_HORI_ORIENT,      cppu::UnoType<sal_Int16>::get(),           PROPERTY_NONE ,MID_HORIORIENT_ORIENT    },             \
    { UNO_NAME_HORI_ORIENT_POSITION, RES_HORI_ORIENT,     cppu::UnoType<sal_Int32>::get(),           PROPERTY_NONE ,MID_HORIORIENT_POSITION|CONVERT_TWIPS    }, \
    { UNO_NAME_HORI_ORIENT_RELATION, RES_HORI_ORIENT,     cppu::UnoType<sal_Int16>::get(),           PROPERTY_NONE ,MID_HORIORIENT_RELATION  },               \
    { UNO_NAME_HYPER_LINK_U_R_L, RES_URL,                 cppu::UnoType<OUString>::get(),        PROPERTY_NONE ,MID_URL_URL},                         \
    { UNO_NAME_HYPER_LINK_TARGET, RES_URL,                cppu::UnoType<OUString>::get(),        PROPERTY_NONE ,MID_URL_TARGET},                      \
    { UNO_NAME_HYPER_LINK_NAME, RES_URL,              cppu::UnoType<OUString>::get(),            PROPERTY_NONE ,MID_URL_HYPERLINKNAME     },                 \
    { UNO_NAME_OPAQUE, RES_OPAQUE,            cppu::UnoType<bool>::get(),         PROPERTY_NONE, 0},                                        \
    { UNO_NAME_PAGE_TOGGLE, RES_HORI_ORIENT,      cppu::UnoType<bool>::get(),         PROPERTY_NONE ,MID_HORIORIENT_PAGETOGGLE },               \
    { UNO_NAME_POSITION_PROTECTED, RES_PROTECT,           cppu::UnoType<bool>::get(),             PROPERTY_NONE, MID_PROTECT_POSITION},                      \
    { UNO_NAME_PRINT, RES_PRINT,              cppu::UnoType<bool>::get(),         PROPERTY_NONE, 0},                                        \
    { UNO_NAME_RELATIVE_HEIGHT, RES_FRM_SIZE,         cppu::UnoType<sal_Int16>::get()  ,         PROPERTY_NONE,   MID_FRMSIZE_REL_HEIGHT },            \
    { UNO_NAME_RELATIVE_HEIGHT_RELATION, RES_FRM_SIZE, cppu::UnoType<sal_Int16>::get()  ,        PROPERTY_NONE,   MID_FRMSIZE_REL_HEIGHT_RELATION  },         \
    { UNO_NAME_RELATIVE_WIDTH, RES_FRM_SIZE,          cppu::UnoType<sal_Int16>::get()  ,         PROPERTY_NONE,   MID_FRMSIZE_REL_WIDTH  },         \
    { UNO_NAME_RELATIVE_WIDTH_RELATION, RES_FRM_SIZE, cppu::UnoType<sal_Int16>::get()  ,         PROPERTY_NONE,   MID_FRMSIZE_REL_WIDTH_RELATION  },         \
    { UNO_NAME_SHADOW_FORMAT, RES_SHADOW,             cppu::UnoType<css::table::ShadowFormat>::get(),   PROPERTY_NONE, CONVERT_TWIPS},             \
    { UNO_NAME_SHADOW_TRANSPARENCE, RES_SHADOW,       cppu::UnoType<sal_Int16>::get(),       PROPERTY_NONE, MID_SHADOW_TRANSPARENCE},             \
    { UNO_NAME_IMAGE_MAP, RES_URL,                    cppu::UnoType<css::container::XIndexContainer>::get(), PROPERTY_NONE, MID_URL_CLIENTMAP}, \
    { UNO_NAME_SERVER_MAP, RES_URL,                cppu::UnoType<bool>::get(),            PROPERTY_NONE ,MID_URL_SERVERMAP   },                      \
    { UNO_NAME_SIZE, RES_FRM_SIZE,            cppu::UnoType<css::awt::Size>::get(),             PROPERTY_NONE, MID_FRMSIZE_SIZE|CONVERT_TWIPS},        \
    { UNO_NAME_SIZE_PROTECTED, RES_PROTECT,           cppu::UnoType<bool>::get(),             PROPERTY_NONE, MID_PROTECT_SIZE    },                      \
    { UNO_NAME_IS_SYNC_WIDTH_TO_HEIGHT, RES_FRM_SIZE,         cppu::UnoType<bool>::get()  ,       PROPERTY_NONE,   MID_FRMSIZE_IS_SYNC_WIDTH_TO_HEIGHT    },  \
    { UNO_NAME_IS_SYNC_HEIGHT_TO_WIDTH, RES_FRM_SIZE,         cppu::UnoType<bool>::get()  ,       PROPERTY_NONE,   MID_FRMSIZE_IS_SYNC_HEIGHT_TO_WIDTH },     \
    { UNO_NAME_TEXT_WRAP, RES_SURROUND,           cppu::UnoType<css::text::WrapTextMode>::get(),             PROPERTY_NONE, MID_SURROUND_SURROUNDTYPE    },        \
    { UNO_NAME_SURROUND, RES_SURROUND,          cppu::UnoType<css::text::WrapTextMode>::get(),            PROPERTY_NONE, MID_SURROUND_SURROUNDTYPE },        \
    { UNO_NAME_SURROUND_ANCHORONLY, RES_SURROUND,             cppu::UnoType<bool>::get(),             PROPERTY_NONE, MID_SURROUND_ANCHORONLY      },                 \
    { UNO_NAME_TOP_MARGIN, RES_UL_SPACE,          cppu::UnoType<sal_Int32>::get(), PROPERTY_NONE, MID_UP_MARGIN|CONVERT_TWIPS},          \
    { UNO_NAME_BOTTOM_MARGIN, RES_UL_SPACE,           cppu::UnoType<sal_Int32>::get(), PROPERTY_NONE, MID_LO_MARGIN|CONVERT_TWIPS},          \
    { UNO_NAME_BACK_TRANSPARENT, RES_BACKGROUND,      cppu::UnoType<bool>::get(),         PROPERTY_NONE ,MID_GRAPHIC_TRANSPARENT       },                    \
    { UNO_NAME_VERT_ORIENT, RES_VERT_ORIENT,      cppu::UnoType<sal_Int16>::get(),           PROPERTY_NONE ,MID_VERTORIENT_ORIENT    },             \
    { UNO_NAME_VERT_ORIENT_POSITION, RES_VERT_ORIENT,     cppu::UnoType<sal_Int32>::get(),           PROPERTY_NONE ,MID_VERTORIENT_POSITION|CONVERT_TWIPS    }, \
    { UNO_NAME_VERT_ORIENT_RELATION, RES_VERT_ORIENT,     cppu::UnoType<sal_Int16>::get(),           PROPERTY_NONE ,MID_VERTORIENT_RELATION  },               \
    { UNO_NAME_LEFT_BORDER, RES_BOX,              cppu::UnoType<css::table::BorderLine>::get(),  0, LEFT_BORDER  |CONVERT_TWIPS },             \
    { UNO_NAME_RIGHT_BORDER, RES_BOX,             cppu::UnoType<css::table::BorderLine>::get(),  0, RIGHT_BORDER |CONVERT_TWIPS },                 \
    { UNO_NAME_TOP_BORDER, RES_BOX,               cppu::UnoType<css::table::BorderLine>::get(),  0, TOP_BORDER   |CONVERT_TWIPS },             \
    { UNO_NAME_BOTTOM_BORDER, RES_BOX,                cppu::UnoType<css::table::BorderLine>::get(),  0, BOTTOM_BORDER|CONVERT_TWIPS },                 \
    { UNO_NAME_BORDER_DISTANCE, RES_BOX,              cppu::UnoType<sal_Int32>::get(),   0, BORDER_DISTANCE|CONVERT_TWIPS },                       \
    { UNO_NAME_LEFT_BORDER_DISTANCE, RES_BOX,             cppu::UnoType<sal_Int32>::get(),   0, LEFT_BORDER_DISTANCE  |CONVERT_TWIPS },                \
    { UNO_NAME_RIGHT_BORDER_DISTANCE, RES_BOX,                cppu::UnoType<sal_Int32>::get(),   0, RIGHT_BORDER_DISTANCE |CONVERT_TWIPS },                \
    { UNO_NAME_TOP_BORDER_DISTANCE, RES_BOX,              cppu::UnoType<sal_Int32>::get(),   0, TOP_BORDER_DISTANCE   |CONVERT_TWIPS },            \
    { UNO_NAME_BOTTOM_BORDER_DISTANCE, RES_BOX,               cppu::UnoType<sal_Int32>::get(),   0, BOTTOM_BORDER_DISTANCE|CONVERT_TWIPS },            \
    { UNO_LINK_DISPLAY_NAME, FN_PARAM_LINK_DISPLAY_NAME,  cppu::UnoType<OUString>::get(), PropertyAttribute::READONLY, 0xbf}, \
    { UNO_NAME_USER_DEFINED_ATTRIBUTES, RES_UNKNOWNATR_CONTAINER, cppu::UnoType<css::container::XNameContainer>::get(), PropertyAttribute::MAYBEVOID, 0 },\
    { UNO_NAME_Z_ORDER, FN_UNO_Z_ORDER,           cppu::UnoType<sal_Int32>::get(),       PROPERTY_NONE, 0}, \
    { UNO_NAME_IS_FOLLOWING_TEXT_FLOW, RES_FOLLOW_TEXT_FLOW,     cppu::UnoType<bool>::get(), PROPERTY_NONE, MID_FOLLOW_TEXT_FLOW}, \
    { UNO_NAME_PARENT_TEXT, FN_UNO_PARENT_TEXT, cppu::UnoType<text::XText>::get(), PropertyAttribute::MAYBEVOID | PropertyAttribute::READONLY, 0 }, \
    { UNO_NAME_WRAP_INFLUENCE_ON_POSITION, RES_WRAP_INFLUENCE_ON_OBJPOS, cppu::UnoType<sal_Int8>::get(), PROPERTY_NONE, MID_WRAP_INFLUENCE}, \
    { UNO_NAME_ALLOW_OVERLAP, RES_WRAP_INFLUENCE_ON_OBJPOS, cppu::UnoType<bool>::get(), PROPERTY_NONE, MID_ALLOW_OVERLAP}, \
    { UNO_NAME_TITLE, FN_UNO_TITLE, cppu::UnoType<OUString>::get(), PROPERTY_NONE, 0}, \
    { UNO_NAME_DESCRIPTION, FN_UNO_DESCRIPTION, cppu::UnoType<OUString>::get(), PROPERTY_NONE, 0}, \
    { UNO_NAME_LAYOUT_SIZE, WID_LAYOUT_SIZE, cppu::UnoType<css::awt::Size>::get(), PropertyAttribute::MAYBEVOID | PropertyAttribute::READONLY, 0 }, \
    { UNO_NAME_LINE_STYLE, RES_BOX, cppu::UnoType<css::drawing::LineStyle>::get(),  0, LINE_STYLE }, \
    { UNO_NAME_LINE_WIDTH, RES_BOX, cppu::UnoType<sal_Int32>::get(),  0, LINE_WIDTH |CONVERT_TWIPS }, \
    { UNO_NAME_TEXT_VERT_ADJUST, RES_TEXT_VERT_ADJUST, cppu::UnoType<css::drawing::TextVerticalAdjust>::get(), PROPERTY_NONE ,0},

#define COMMON_TEXT_CONTENT_PROPERTIES \
        { UNO_NAME_ANCHOR_TYPE, FN_UNO_ANCHOR_TYPE, cppu::UnoType<css::text::TextContentAnchorType>::get(),              PropertyAttribute::READONLY, MID_ANCHOR_ANCHORTYPE},\
        ANCHOR_TYPES_PROPERTY\
        { UNO_NAME_TEXT_WRAP, FN_UNO_TEXT_WRAP,   cppu::UnoType<css::text::WrapTextMode>::get(),                 PropertyAttribute::READONLY, MID_SURROUND_SURROUNDTYPE  },

#define     PROP_DIFF_FONTHEIGHT \
                    { UNO_NAME_CHAR_PROP_HEIGHT, RES_CHRATR_FONTSIZE ,            cppu::UnoType<float>::get(),                                           PROPERTY_NONE , MID_FONTHEIGHT_PROP},\
                    { UNO_NAME_CHAR_DIFF_HEIGHT, RES_CHRATR_FONTSIZE ,            cppu::UnoType<sal_Int16>::get(),    PROPERTY_NONE , MID_FONTHEIGHT_DIFF|CONVERT_TWIPS},\
                    { UNO_NAME_CHAR_PROP_HEIGHT_ASIAN, RES_CHRATR_CJK_FONTSIZE ,          cppu::UnoType<float>::get(),                                           PROPERTY_NONE , MID_FONTHEIGHT_PROP},\
                    { UNO_NAME_CHAR_DIFF_HEIGHT_ASIAN, RES_CHRATR_CJK_FONTSIZE ,          cppu::UnoType<sal_Int16>::get(),    PROPERTY_NONE , MID_FONTHEIGHT_DIFF|CONVERT_TWIPS},\
                    { UNO_NAME_CHAR_PROP_HEIGHT_COMPLEX, RES_CHRATR_CTL_FONTSIZE ,            cppu::UnoType<float>::get(),                                           PROPERTY_NONE , MID_FONTHEIGHT_PROP},\
                    { UNO_NAME_CHAR_DIFF_HEIGHT_COMPLEX, RES_CHRATR_CTL_FONTSIZE ,            cppu::UnoType<sal_Int16>::get(),    PROPERTY_NONE , MID_FONTHEIGHT_DIFF|CONVERT_TWIPS},

#define COMMON_PARA_STYLE_PROPERTIES \
                    { UNO_NAME_BREAK_TYPE, RES_BREAK,                 cppu::UnoType<css::style::BreakType>::get(),       PROPERTY_NONE, 0},\
                    { UNO_NAME_PAGE_DESC_NAME, RES_PAGEDESC,          cppu::UnoType<OUString>::get(),            PropertyAttribute::MAYBEVOID, MID_PAGEDESC_PAGEDESCNAME },\
                    { UNO_NAME_PAGE_NUMBER_OFFSET, RES_PAGEDESC,              cppu::UnoType<sal_Int16>::get(),       PropertyAttribute::MAYBEVOID, MID_PAGEDESC_PAGENUMOFFSET},\
                    { UNO_NAME_CHAR_AUTO_KERNING, RES_CHRATR_AUTOKERN  ,  cppu::UnoType<bool>::get()  ,       PROPERTY_NONE,     0},\
                    { UNO_NAME_CHAR_BACK_TRANSPARENT, RES_CHRATR_BACKGROUND, cppu::UnoType<bool>::get(),          PROPERTY_NONE ,MID_GRAPHIC_TRANSPARENT       },\
                    { UNO_NAME_CHAR_BACK_COLOR, RES_CHRATR_BACKGROUND,    cppu::UnoType<sal_Int32>::get(),           PROPERTY_NONE ,MID_BACK_COLOR        },\
                    { UNO_NAME_CHAR_HIGHLIGHT, RES_CHRATR_HIGHLIGHT, cppu::UnoType<sal_Int32>::get(), PROPERTY_NONE ,MID_BACK_COLOR },\
                    { UNO_NAME_PARA_BACK_COLOR, RES_BACKGROUND,       cppu::UnoType<sal_Int32>::get(),           PROPERTY_NONE ,MID_BACK_COLOR        },\
                    { UNO_NAME_PARA_BACK_TRANSPARENT, RES_BACKGROUND,         cppu::UnoType<bool>::get(),         PROPERTY_NONE ,MID_GRAPHIC_TRANSPARENT       },\
                    { UNO_NAME_PARA_GRAPHIC_URL, RES_BACKGROUND,      cppu::UnoType<OUString>::get(), PROPERTY_NONE ,MID_GRAPHIC_URL    },\
                    { UNO_NAME_PARA_GRAPHIC, RES_BACKGROUND,      cppu::UnoType<css::graphic::XGraphic>::get(), PROPERTY_NONE ,MID_GRAPHIC    },\
                    { UNO_NAME_PARA_GRAPHIC_FILTER, RES_BACKGROUND,       cppu::UnoType<OUString>::get(), PROPERTY_NONE ,MID_GRAPHIC_FILTER    },\
                    { UNO_NAME_PARA_GRAPHIC_LOCATION, RES_BACKGROUND,         cppu::UnoType<css::style::GraphicLocation>::get(), PROPERTY_NONE ,MID_GRAPHIC_POSITION}, \
                    { UNO_NAME_CHAR_CASE_MAP, RES_CHRATR_CASEMAP,     cppu::UnoType<sal_Int16>::get(),           PROPERTY_NONE, 0},\
                    { UNO_NAME_CHAR_COLOR, RES_CHRATR_COLOR,      cppu::UnoType<sal_Int32>::get(),           PROPERTY_NONE, 0},\
                    { UNO_NAME_CHAR_TRANSPARENCE, RES_CHRATR_COLOR,      cppu::UnoType<sal_Int16>::get(),           PROPERTY_NONE, MID_COLOR_ALPHA},\
                    { UNO_NAME_CHAR_STRIKEOUT, RES_CHRATR_CROSSEDOUT,  cppu::UnoType<sal_Int16>::get(),                  PropertyAttribute::MAYBEVOID, MID_CROSS_OUT},\
                    { UNO_NAME_CHAR_CROSSED_OUT, RES_CHRATR_CROSSEDOUT,  cppu::UnoType<bool>::get()  ,        PROPERTY_NONE, 0},\
                    { UNO_NAME_CHAR_ESCAPEMENT, RES_CHRATR_ESCAPEMENT,  cppu::UnoType<sal_Int16>::get(),             PROPERTY_NONE, MID_ESC          },\
                    { UNO_NAME_CHAR_ESCAPEMENT_HEIGHT, RES_CHRATR_ESCAPEMENT,     cppu::UnoType<sal_Int8>::get()  ,          PROPERTY_NONE, MID_ESC_HEIGHT},\
                    { UNO_NAME_CHAR_FLASH, RES_CHRATR_BLINK   ,   cppu::UnoType<bool>::get()  ,       PROPERTY_NONE,     0},\
                    { UNO_NAME_CHAR_HIDDEN, RES_CHRATR_HIDDEN, cppu::UnoType<bool>::get()  ,       PROPERTY_NONE,     0},\
                    STANDARD_FONT_PROPERTIES\
                    CJK_FONT_PROPERTIES\
                    CTL_FONT_PROPERTIES\
                    { UNO_NAME_CHAR_UNDERLINE, RES_CHRATR_UNDERLINE ,  cppu::UnoType<sal_Int16>::get(),      PROPERTY_NONE, MID_TL_STYLE},\
                    { UNO_NAME_CHAR_UNDERLINE_COLOR, RES_CHRATR_UNDERLINE ,  cppu::UnoType<sal_Int32>::get(),            PROPERTY_NONE, MID_TL_COLOR},\
                    { UNO_NAME_CHAR_UNDERLINE_HAS_COLOR, RES_CHRATR_UNDERLINE ,  cppu::UnoType<bool>::get(),              PROPERTY_NONE, MID_TL_HASCOLOR},\
                    { UNO_NAME_CHAR_OVERLINE, RES_CHRATR_OVERLINE ,  cppu::UnoType<sal_Int16>::get(),      PROPERTY_NONE, MID_TL_STYLE},\
                    { UNO_NAME_CHAR_OVERLINE_COLOR, RES_CHRATR_OVERLINE ,  cppu::UnoType<sal_Int32>::get(),            PROPERTY_NONE, MID_TL_COLOR},\
                    { UNO_NAME_CHAR_OVERLINE_HAS_COLOR, RES_CHRATR_OVERLINE ,  cppu::UnoType<bool>::get(),              PROPERTY_NONE, MID_TL_HASCOLOR},\
                    { UNO_NAME_PARA_LEFT_MARGIN, RES_LR_SPACE,            cppu::UnoType<sal_Int32>::get(), PROPERTY_NONE, MID_TXT_LMARGIN|CONVERT_TWIPS},\
                    { UNO_NAME_PARA_RIGHT_MARGIN, RES_LR_SPACE,           cppu::UnoType<sal_Int32>::get(), PROPERTY_NONE, MID_R_MARGIN|CONVERT_TWIPS},\
                    { UNO_NAME_PARA_LEFT_MARGIN_RELATIVE, RES_LR_SPACE,          cppu::UnoType<sal_Int16>::get(), PROPERTY_NONE,        MID_L_REL_MARGIN},\
                    { UNO_NAME_PARA_RIGHT_MARGIN_RELATIVE, RES_LR_SPACE,         cppu::UnoType<sal_Int16>::get(), PROPERTY_NONE,        MID_R_REL_MARGIN},\
                    { UNO_NAME_PARA_IS_AUTO_FIRST_LINE_INDENT, RES_LR_SPACE,          cppu::UnoType<bool>::get(),      PROPERTY_NONE, MID_FIRST_AUTO},\
                    { UNO_NAME_PARA_FIRST_LINE_INDENT, RES_LR_SPACE,           cppu::UnoType<sal_Int32>::get(), PROPERTY_NONE, MID_FIRST_LINE_INDENT|CONVERT_TWIPS},\
                    { UNO_NAME_PARA_FIRST_LINE_INDENT_RELATIVE, RES_LR_SPACE,         cppu::UnoType<sal_Int32>::get(), PROPERTY_NONE, MID_FIRST_LINE_REL_INDENT|CONVERT_TWIPS},\
                    { UNO_NAME_CHAR_KERNING, RES_CHRATR_KERNING    ,  cppu::UnoType<sal_Int16>::get()  ,         PROPERTY_NONE,  CONVERT_TWIPS},\
                    { UNO_NAME_CHAR_NO_HYPHENATION, RES_CHRATR_NOHYPHEN   ,   cppu::UnoType<bool>::get()  ,       PROPERTY_NONE,     0},\
                    { UNO_NAME_CHAR_SHADOWED, RES_CHRATR_SHADOWED  ,  cppu::UnoType<bool>::get()  ,       PROPERTY_NONE, 0},\
                    { UNO_NAME_CHAR_CONTOURED, RES_CHRATR_CONTOUR,    cppu::UnoType<bool>::get()  ,       PROPERTY_NONE, 0},\
                    { UNO_NAME_DROP_CAP_FORMAT, RES_PARATR_DROP,        cppu::UnoType<css::style::DropCapFormat>::get()  , PROPERTY_NONE, MID_DROPCAP_FORMAT|CONVERT_TWIPS     },\
                    { UNO_NAME_DROP_CAP_WHOLE_WORD, RES_PARATR_DROP,        cppu::UnoType<bool>::get()  ,         PROPERTY_NONE, MID_DROPCAP_WHOLE_WORD },\
                    { UNO_NAME_DROP_CAP_CHAR_STYLE_NAME, RES_PARATR_DROP,        cppu::UnoType<OUString>::get()  ,       PropertyAttribute::MAYBEVOID, MID_DROPCAP_CHAR_STYLE_NAME },\
                    { UNO_NAME_PARA_KEEP_TOGETHER, RES_KEEP,              cppu::UnoType<bool>::get()  ,       PROPERTY_NONE,     0},\
                    { UNO_NAME_PARA_SPLIT, RES_PARATR_SPLIT,      cppu::UnoType<bool>::get()  ,       PROPERTY_NONE,     0},\
                    { UNO_NAME_PARA_WIDOWS, RES_PARATR_WIDOWS,        cppu::UnoType<sal_Int8>::get(),PropertyAttribute::MAYBEVOID,     0},\
                    { UNO_NAME_PARA_ORPHANS, RES_PARATR_ORPHANS,      cppu::UnoType<sal_Int8>::get(),PropertyAttribute::MAYBEVOID,     0},\
                    { UNO_NAME_PARA_EXPAND_SINGLE_WORD, RES_PARATR_ADJUST,      cppu::UnoType<bool>::get()  ,         PROPERTY_NONE, MID_EXPAND_SINGLE   },\
                    { UNO_NAME_PARA_LAST_LINE_ADJUST, RES_PARATR_ADJUST,      cppu::UnoType<sal_Int16>::get(),       PROPERTY_NONE, MID_LAST_LINE_ADJUST},\
                    { UNO_NAME_PARA_LINE_NUMBER_COUNT, RES_LINENUMBER,        cppu::UnoType<bool>::get(),         PROPERTY_NONE ,MID_LINENUMBER_COUNT     },\
                    { UNO_NAME_PARA_LINE_NUMBER_START_VALUE, RES_LINENUMBER,      cppu::UnoType<sal_Int32>::get(),           PROPERTY_NONE ,MID_LINENUMBER_STARTVALUE},\
                    { UNO_NAME_PARA_LINE_SPACING, RES_PARATR_LINESPACING, cppu::UnoType<css::style::LineSpacing>::get(),PROPERTY_NONE,     CONVERT_TWIPS},\
                    { UNO_NAME_PARA_ADJUST, RES_PARATR_ADJUST,      cppu::UnoType<sal_Int16>::get(),         PROPERTY_NONE, MID_PARA_ADJUST},\
                    { UNO_NAME_PARA_REGISTER_MODE_ACTIVE, RES_PARATR_REGISTER,    cppu::UnoType<bool>::get()  ,       PROPERTY_NONE, 0},\
                    { UNO_NAME_PARA_TOP_MARGIN, RES_UL_SPACE,             cppu::UnoType<sal_Int32>::get(), PROPERTY_NONE, MID_UP_MARGIN|CONVERT_TWIPS},\
                    { UNO_NAME_PARA_BOTTOM_MARGIN, RES_UL_SPACE,          cppu::UnoType<sal_Int32>::get(), PROPERTY_NONE, MID_LO_MARGIN|CONVERT_TWIPS},\
                    { UNO_NAME_PARA_CONTEXT_MARGIN, RES_UL_SPACE,          cppu::UnoType<bool>::get(), PROPERTY_NONE, MID_CTX_MARGIN},\
                    { UNO_NAME_PARA_TOP_MARGIN_RELATIVE, RES_UL_SPACE,        cppu::UnoType<sal_Int16>::get(), PROPERTY_NONE, MID_UP_REL_MARGIN},\
                    { UNO_NAME_PARA_BOTTOM_MARGIN_RELATIVE, RES_UL_SPACE,         cppu::UnoType<sal_Int16>::get(), PROPERTY_NONE, MID_LO_REL_MARGIN},\
                    TABSTOPS_MAP_ENTRY\
                    { UNO_NAME_CHAR_WORD_MODE, RES_CHRATR_WORDLINEMODE,cppu::UnoType<bool>::get()  ,    PROPERTY_NONE,     0},\
                    { UNO_NAME_CHAR_SHADING_VALUE, RES_CHRATR_BACKGROUND, cppu::UnoType<sal_Int32>::get(), PROPERTY_NONE, MID_SHADING_VALUE }, \
                    { UNO_NAME_CHAR_LEFT_BORDER, RES_CHRATR_BOX, cppu::UnoType<css::table::BorderLine>::get(), PROPERTY_NONE, LEFT_BORDER |CONVERT_TWIPS },\
                    { UNO_NAME_CHAR_RIGHT_BORDER, RES_CHRATR_BOX, cppu::UnoType<css::table::BorderLine>::get(), PROPERTY_NONE, RIGHT_BORDER |CONVERT_TWIPS },\
                    { UNO_NAME_CHAR_TOP_BORDER, RES_CHRATR_BOX, cppu::UnoType<css::table::BorderLine>::get(), PROPERTY_NONE, TOP_BORDER |CONVERT_TWIPS },\
                    { UNO_NAME_CHAR_BOTTOM_BORDER, RES_CHRATR_BOX, cppu::UnoType<css::table::BorderLine>::get(), PROPERTY_NONE, BOTTOM_BORDER |CONVERT_TWIPS },\
                    { UNO_NAME_CHAR_BORDER_DISTANCE, RES_CHRATR_BOX, cppu::UnoType<sal_Int32>::get(), PROPERTY_NONE, BORDER_DISTANCE |CONVERT_TWIPS },\
                    { UNO_NAME_CHAR_LEFT_BORDER_DISTANCE, RES_CHRATR_BOX, cppu::UnoType<sal_Int32>::get(), PROPERTY_NONE, LEFT_BORDER_DISTANCE |CONVERT_TWIPS },\
                    { UNO_NAME_CHAR_RIGHT_BORDER_DISTANCE, RES_CHRATR_BOX, cppu::UnoType<sal_Int32>::get(),PROPERTY_NONE, RIGHT_BORDER_DISTANCE |CONVERT_TWIPS },\
                    { UNO_NAME_CHAR_TOP_BORDER_DISTANCE, RES_CHRATR_BOX, cppu::UnoType<sal_Int32>::get(), PROPERTY_NONE, TOP_BORDER_DISTANCE |CONVERT_TWIPS },\
                    { UNO_NAME_CHAR_BOTTOM_BORDER_DISTANCE, RES_CHRATR_BOX, cppu::UnoType<sal_Int32>::get(), PROPERTY_NONE, BOTTOM_BORDER_DISTANCE|CONVERT_TWIPS },\
                    { UNO_NAME_CHAR_SHADOW_FORMAT, RES_CHRATR_SHADOW, cppu::UnoType<css::table::ShadowFormat>::get(), PROPERTY_NONE, CONVERT_TWIPS},\
                    { UNO_NAME_LEFT_BORDER, RES_BOX,              cppu::UnoType<css::table::BorderLine>::get(),  0, LEFT_BORDER  |CONVERT_TWIPS },\
                    { UNO_NAME_RIGHT_BORDER, RES_BOX,             cppu::UnoType<css::table::BorderLine>::get(),  0, RIGHT_BORDER |CONVERT_TWIPS },\
                    { UNO_NAME_TOP_BORDER, RES_BOX,               cppu::UnoType<css::table::BorderLine>::get(),  0, TOP_BORDER   |CONVERT_TWIPS },\
                    { UNO_NAME_BOTTOM_BORDER, RES_BOX,                cppu::UnoType<css::table::BorderLine>::get(),  0, BOTTOM_BORDER|CONVERT_TWIPS },\
                    { UNO_NAME_BORDER_DISTANCE, RES_BOX,              cppu::UnoType<sal_Int32>::get(),   0, BORDER_DISTANCE|CONVERT_TWIPS },\
                    { UNO_NAME_LEFT_BORDER_DISTANCE, RES_BOX,             cppu::UnoType<sal_Int32>::get(),   0, LEFT_BORDER_DISTANCE  |CONVERT_TWIPS },\
                    { UNO_NAME_RIGHT_BORDER_DISTANCE, RES_BOX,                cppu::UnoType<sal_Int32>::get(),   0, RIGHT_BORDER_DISTANCE |CONVERT_TWIPS },\
                    { UNO_NAME_TOP_BORDER_DISTANCE, RES_BOX,              cppu::UnoType<sal_Int32>::get(),   0, TOP_BORDER_DISTANCE   |CONVERT_TWIPS },\
                    { UNO_NAME_BOTTOM_BORDER_DISTANCE, RES_BOX,               cppu::UnoType<sal_Int32>::get(),   0, BOTTOM_BORDER_DISTANCE|CONVERT_TWIPS },\
                    { UNO_NAME_PARA_IS_HYPHENATION, RES_PARATR_HYPHENZONE,      cppu::UnoType<bool>::get(),  PropertyAttribute::MAYBEVOID, MID_IS_HYPHEN         },\
                    { UNO_NAME_PARA_HYPHENATION_NO_CAPS, RES_PARATR_HYPHENZONE,      cppu::UnoType<bool>::get(),  PropertyAttribute::MAYBEVOID, MID_HYPHEN_NO_CAPS },\
                    { UNO_NAME_PARA_HYPHENATION_MAX_LEADING_CHARS, RES_PARATR_HYPHENZONE,         cppu::UnoType<sal_Int16>::get(),   PropertyAttribute::MAYBEVOID, MID_HYPHEN_MIN_LEAD   },\
                    { UNO_NAME_PARA_HYPHENATION_MAX_TRAILING_CHARS, RES_PARATR_HYPHENZONE,        cppu::UnoType<sal_Int16>::get(),   PropertyAttribute::MAYBEVOID, MID_HYPHEN_MIN_TRAIL  },\
                    { UNO_NAME_PARA_HYPHENATION_MAX_HYPHENS, RES_PARATR_HYPHENZONE,       cppu::UnoType<sal_Int16>::get(),   PropertyAttribute::MAYBEVOID, MID_HYPHEN_MAX_HYPHENS},\
                    { UNO_NAME_NUMBERING_STYLE_NAME, RES_PARATR_NUMRULE,  cppu::UnoType<OUString>::get(),         PropertyAttribute::MAYBEVOID,   0},\
                    { UNO_NAME_NUMBERING_LEVEL, RES_PARATR_LIST_LEVEL,    cppu::UnoType<sal_Int16>::get(),        PropertyAttribute::MAYBEVOID,   0},\
                    { UNO_NAME_PARA_USER_DEFINED_ATTRIBUTES, RES_UNKNOWNATR_CONTAINER, cppu::UnoType<css::container::XNameContainer>::get(), PropertyAttribute::MAYBEVOID, 0 },\
                    { UNO_NAME_PARA_SHADOW_FORMAT, RES_SHADOW,    cppu::UnoType<css::table::ShadowFormat>::get(),   PROPERTY_NONE, CONVERT_TWIPS},\
                    { UNO_NAME_CHAR_COMBINE_IS_ON, RES_CHRATR_TWO_LINES,          cppu::UnoType<bool>::get(),     PROPERTY_NONE, MID_TWOLINES},\
                    { UNO_NAME_CHAR_COMBINE_PREFIX, RES_CHRATR_TWO_LINES,             cppu::UnoType<OUString>::get(),    PROPERTY_NONE, MID_START_BRACKET},\
                    { UNO_NAME_CHAR_COMBINE_SUFFIX, RES_CHRATR_TWO_LINES,             cppu::UnoType<OUString>::get(),    PROPERTY_NONE, MID_END_BRACKET},\
                    { UNO_NAME_CHAR_EMPHASIS, RES_CHRATR_EMPHASIS_MARK,           cppu::UnoType<sal_Int16>::get(),   PROPERTY_NONE, MID_EMPHASIS},\
                    { UNO_NAME_PARA_IS_HANGING_PUNCTUATION, RES_PARATR_HANGINGPUNCTUATION,  cppu::UnoType<bool>::get(),   PROPERTY_NONE ,0     },\
                    { UNO_NAME_PARA_IS_CHARACTER_DISTANCE, RES_PARATR_SCRIPTSPACE,         cppu::UnoType<bool>::get(),    PROPERTY_NONE ,0     },\
                    { UNO_NAME_PARA_IS_FORBIDDEN_RULES, RES_PARATR_FORBIDDEN_RULES,     cppu::UnoType<bool>::get(),    PROPERTY_NONE ,0    },\
                    { UNO_NAME_PARA_VERT_ALIGNMENT, RES_PARATR_VERTALIGN,             cppu::UnoType<sal_Int16>::get(),    PROPERTY_NONE , 0  },\
                    { UNO_NAME_CHAR_ROTATION, RES_CHRATR_ROTATE,      cppu::UnoType<sal_Int16>::get(),   PROPERTY_NONE,    MID_ROTATE      },\
                    { UNO_NAME_CHAR_ROTATION_IS_FIT_TO_LINE, RES_CHRATR_ROTATE,       cppu::UnoType<bool>::get(),     PROPERTY_NONE,        MID_FITTOLINE  },\
                    { UNO_NAME_CHAR_SCALE_WIDTH, RES_CHRATR_SCALEW,       cppu::UnoType<sal_Int16>::get(),   PROPERTY_NONE,         0 },\
                    { UNO_NAME_CHAR_RELIEF, RES_CHRATR_RELIEF,      cppu::UnoType<sal_Int16>::get(),    PROPERTY_NONE,      MID_RELIEF },\
                    PROP_DIFF_FONTHEIGHT\
                    { UNO_NAME_FOLLOW_STYLE, FN_UNO_FOLLOW_STYLE,     cppu::UnoType<OUString>::get(), PROPERTY_NONE, 0},\
                    { UNO_NAME_LINK_STYLE, FN_UNO_LINK_STYLE,     cppu::UnoType<OUString>::get(), PROPERTY_NONE, 0},\
                    { UNO_NAME_IS_PHYSICAL, FN_UNO_IS_PHYSICAL,     cppu::UnoType<bool>::get(), PropertyAttribute::READONLY, 0},\
                    { UNO_NAME_IS_AUTO_UPDATE, FN_UNO_IS_AUTO_UPDATE, cppu::UnoType<bool>::get(), PROPERTY_NONE, 0},\
                    { UNO_NAME_DISPLAY_NAME, FN_UNO_DISPLAY_NAME, cppu::UnoType<OUString>::get(), PropertyAttribute::READONLY, 0},\
                    { UNO_NAME_CATEGORY, FN_UNO_CATEGORY, cppu::UnoType<sal_Int16>::get(),           PROPERTY_NONE , 0 },\
                    { UNO_NAME_WRITING_MODE, RES_FRAMEDIR, cppu::UnoType<sal_Int16>::get(), PROPERTY_NONE, 0 },\
                    { UNO_NAME_PARA_IS_CONNECT_BORDER, RES_PARATR_CONNECT_BORDER, cppu::UnoType<bool>::get(), PropertyAttribute::MAYBEVOID, 0},\
                    { UNO_NAME_SNAP_TO_GRID, RES_PARATR_SNAPTOGRID, cppu::UnoType<bool>::get(), PropertyAttribute::MAYBEVOID, 0 }, \
                    { UNO_NAME_OUTLINE_LEVEL, RES_PARATR_OUTLINELEVEL,cppu::UnoType<sal_Int16>::get(), PropertyAttribute::MAYBEVOID, 0}, \
                    { UNO_NAME_HIDDEN, FN_UNO_HIDDEN,     cppu::UnoType<bool>::get(), PROPERTY_NONE, 0}, \
                    { UNO_NAME_STYLE_INTEROP_GRAB_BAG, FN_UNO_STYLE_INTEROP_GRAB_BAG, cppu::UnoType< cppu::UnoSequenceType<css::beans::PropertyValue> >::get(), PROPERTY_NONE, 0}, \
                    { UNO_NAME_PARA_INTEROP_GRAB_BAG, RES_PARATR_GRABBAG, cppu::UnoType< cppu::UnoSequenceType<css::beans::PropertyValue> >::get(), PROPERTY_NONE, 0},

#define COMMON_ACCESSIBILITY_TEXT_ATTRIBUTE \
                    { UNO_NAME_CHAR_BACK_COLOR, RES_CHRATR_BACKGROUND,    cppu::UnoType<sal_Int32>::get(),           PROPERTY_NONE ,MID_BACK_COLOR        }, \
                    { UNO_NAME_CHAR_COLOR, RES_CHRATR_COLOR,      cppu::UnoType<sal_Int32>::get(),           PROPERTY_NONE, 0},  \
                    { UNO_NAME_CHAR_TRANSPARENCE, RES_CHRATR_COLOR,      cppu::UnoType<sal_Int16>::get(),           PROPERTY_NONE, MID_COLOR_ALPHA },  \
                    { UNO_NAME_CHAR_CONTOURED, RES_CHRATR_CONTOUR,    cppu::UnoType<bool>::get()  ,       PROPERTY_NONE, 0},  \
                    { UNO_NAME_CHAR_EMPHASIS, RES_CHRATR_EMPHASIS_MARK,           cppu::UnoType<sal_Int16>::get(),   PROPERTY_NONE, MID_EMPHASIS},   \
                    { UNO_NAME_CHAR_ESCAPEMENT, RES_CHRATR_ESCAPEMENT,  cppu::UnoType<sal_Int16>::get(),             PROPERTY_NONE, MID_ESC          },  \
                    { UNO_NAME_CHAR_FONT_NAME, RES_CHRATR_FONT,       cppu::UnoType<OUString>::get(),  PropertyAttribute::MAYBEVOID, MID_FONT_FAMILY_NAME }, \
                    { UNO_NAME_CHAR_HEIGHT, RES_CHRATR_FONTSIZE  ,  cppu::UnoType<float>::get(),         PropertyAttribute::MAYBEVOID, MID_FONTHEIGHT|CONVERT_TWIPS},    \
                    { UNO_NAME_CHAR_POSTURE, RES_CHRATR_POSTURE   ,  cppu::UnoType<css::awt::FontSlant>::get(),        PropertyAttribute::MAYBEVOID, MID_POSTURE}, \
                    { UNO_NAME_CHAR_SHADOWED, RES_CHRATR_SHADOWED  ,  cppu::UnoType<bool>::get()  ,       PROPERTY_NONE, 0},  \
                    { UNO_NAME_CHAR_STRIKEOUT, RES_CHRATR_CROSSEDOUT,  cppu::UnoType<sal_Int16>::get(),                  PropertyAttribute::MAYBEVOID, MID_CROSS_OUT},   \
                    { UNO_NAME_CHAR_UNDERLINE_COLOR, RES_CHRATR_UNDERLINE ,  cppu::UnoType<sal_Int32>::get(),            PropertyAttribute::MAYBEVOID, MID_TL_COLOR},    \
                    { UNO_NAME_CHAR_WEIGHT, RES_CHRATR_WEIGHT    ,  cppu::UnoType<float>::get(),             PropertyAttribute::MAYBEVOID, MID_WEIGHT},  \
                    { UNO_NAME_NUMBERING_LEVEL, RES_PARATR_LIST_LEVEL,cppu::UnoType<sal_Int16>::get(), PropertyAttribute::MAYBEVOID, 0}, \
                    { UNO_NAME_CHAR_UNDERLINE, RES_CHRATR_UNDERLINE ,  cppu::UnoType<sal_Int16>::get(),              PropertyAttribute::MAYBEVOID, MID_TL_STYLE},    \
                    { UNO_NAME_NUMBERING_RULES, RES_PARATR_NUMRULE,cppu::UnoType<sal_Int16>::get(), PropertyAttribute::MAYBEVOID, CONVERT_TWIPS},    \
                    { UNO_NAME_PARA_ADJUST, RES_PARATR_ADJUST,      cppu::UnoType<sal_Int16>::get(),         PropertyAttribute::MAYBEVOID, MID_PARA_ADJUST}, \
                    { UNO_NAME_PARA_BOTTOM_MARGIN, RES_UL_SPACE,          cppu::UnoType<sal_Int32>::get(),           PropertyAttribute::MAYBEVOID, MID_LO_MARGIN|CONVERT_TWIPS}, \
                    { UNO_NAME_PARA_FIRST_LINE_INDENT, RES_LR_SPACE,           cppu::UnoType<sal_Int32>::get(),      PropertyAttribute::MAYBEVOID, MID_FIRST_LINE_INDENT|CONVERT_TWIPS}, \
                    { UNO_NAME_PARA_LEFT_MARGIN, RES_LR_SPACE,            cppu::UnoType<sal_Int32>::get(),           PropertyAttribute::MAYBEVOID, MID_TXT_LMARGIN|CONVERT_TWIPS},   \
                    { UNO_NAME_PARA_LINE_SPACING, RES_PARATR_LINESPACING, cppu::UnoType<css::style::LineSpacing>::get(),       PropertyAttribute::MAYBEVOID,     CONVERT_TWIPS},   \
                    { UNO_NAME_PARA_RIGHT_MARGIN, RES_LR_SPACE,           cppu::UnoType<sal_Int32>::get(),           PropertyAttribute::MAYBEVOID, MID_R_MARGIN|CONVERT_TWIPS},  \
                    { UNO_NAME_TABSTOPS, RES_PARATR_TABSTOP,   cppu::UnoType< cppu::UnoSequenceType<css::style::TabStop> >::get(),   PropertyAttribute::MAYBEVOID, CONVERT_TWIPS}, \

#define FILL_PROPERTIES_SW_BMP \
    { UNO_NAME_SW_FILLBMP_LOGICAL_SIZE,               XATTR_FILLBMP_SIZELOG,      cppu::UnoType<bool>::get(),       0,  0}, \
    { UNO_NAME_SW_FILLBMP_OFFSET_X,                   XATTR_FILLBMP_TILEOFFSETX,  cppu::UnoType<sal_Int32>::get(),   0,  0}, \
    { UNO_NAME_SW_FILLBMP_OFFSET_Y,                   XATTR_FILLBMP_TILEOFFSETY,  cppu::UnoType<sal_Int32>::get(),   0,  0}, \
    { UNO_NAME_SW_FILLBMP_POSITION_OFFSET_X,          XATTR_FILLBMP_POSOFFSETX,   cppu::UnoType<sal_Int32>::get(),   0,  0}, \
    { UNO_NAME_SW_FILLBMP_POSITION_OFFSET_Y,          XATTR_FILLBMP_POSOFFSETY,   cppu::UnoType<sal_Int32>::get(),   0,  0}, \
    { UNO_NAME_SW_FILLBMP_RECTANGLE_POINT,            XATTR_FILLBMP_POS,          cppu::UnoType<css::drawing::RectanglePoint>::get(), 0,  0}, \
    { UNO_NAME_SW_FILLBMP_SIZE_X,                     XATTR_FILLBMP_SIZEX,        cppu::UnoType<sal_Int32>::get(),   0,  0, PropertyMoreFlags::METRIC_ITEM}, \
    { UNO_NAME_SW_FILLBMP_SIZE_Y,                     XATTR_FILLBMP_SIZEY,        cppu::UnoType<sal_Int32>::get(),   0,  0, PropertyMoreFlags::METRIC_ITEM},    \
    { UNO_NAME_SW_FILLBMP_STRETCH,                    XATTR_FILLBMP_STRETCH,      cppu::UnoType<bool>::get(),       0,  0}, \
    { UNO_NAME_SW_FILLBMP_TILE,                       XATTR_FILLBMP_TILE,         cppu::UnoType<bool>::get(),       0,  0},\
    { UNO_NAME_SW_FILLBMP_MODE,                       OWN_ATTR_FILLBMP_MODE,      cppu::UnoType<drawing::BitmapMode>::get(), 0,  0}, \

#define FILL_PROPERTIES_SW_DEFAULTS \
    { UNO_NAME_SW_FILLCOLOR,                          XATTR_FILLCOLOR,            cppu::UnoType<sal_Int32>::get(),   0,  0}, \

#define FILL_PROPERTIES_SW \
    FILL_PROPERTIES_SW_BMP \
    FILL_PROPERTIES_SW_DEFAULTS \
    { UNO_NAME_SW_FILLBACKGROUND,                 XATTR_FILLBACKGROUND,           cppu::UnoType<bool>::get(),        0, 0}, \
    { UNO_NAME_SW_FILLBITMAP,                     XATTR_FILLBITMAP,               cppu::UnoType<css::awt::XBitmap>::get(), 0, MID_BITMAP}, \
    { UNO_NAME_SW_FILLBITMAPURL,                  XATTR_FILLBITMAP,               cppu::UnoType<OUString>::get(),          0, MID_BITMAP }, \
    { UNO_NAME_SW_FILLBITMAPNAME,                 XATTR_FILLBITMAP,               cppu::UnoType<OUString>::get(),    0,  MID_NAME }, \
    { UNO_NAME_SW_FILLGRADIENTSTEPCOUNT,          XATTR_GRADIENTSTEPCOUNT,        cppu::UnoType<sal_Int16>::get(),   0,  0}, \
    { UNO_NAME_SW_FILLGRADIENT,                   XATTR_FILLGRADIENT,             cppu::UnoType<css::awt::Gradient>::get(), 0, MID_FILLGRADIENT}, \
    { UNO_NAME_SW_FILLGRADIENTNAME,               XATTR_FILLGRADIENT,             cppu::UnoType<OUString>::get(),    0, MID_NAME }, \
    { UNO_NAME_SW_FILLHATCH,                      XATTR_FILLHATCH,                cppu::UnoType<css::drawing::Hatch>::get(), 0, MID_FILLHATCH}, \
    { UNO_NAME_SW_FILLHATCHNAME,                  XATTR_FILLHATCH,                cppu::UnoType<OUString>::get(),  0,  MID_NAME }, \
    { UNO_NAME_SW_FILLSTYLE,                      XATTR_FILLSTYLE,                cppu::UnoType<css::drawing::FillStyle>::get(), 0, 0}, \
    { UNO_NAME_SW_FILL_TRANSPARENCE,              XATTR_FILLTRANSPARENCE,         cppu::UnoType<sal_Int16>::get(), 0, 0}, \
    { UNO_NAME_SW_FILLTRANSPARENCEGRADIENT,       XATTR_FILLFLOATTRANSPARENCE,    cppu::UnoType<css::awt::Gradient>::get(), 0,  MID_FILLGRADIENT}, \
    { UNO_NAME_SW_FILLTRANSPARENCEGRADIENTNAME,   XATTR_FILLFLOATTRANSPARENCE,    cppu::UnoType<OUString>::get(),  0,  MID_NAME }, \
    { UNO_NAME_SW_FILLCOLOR_2,                    XATTR_SECONDARYFILLCOLOR,       cppu::UnoType<sal_Int32>::get(), 0,  0}, \

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
