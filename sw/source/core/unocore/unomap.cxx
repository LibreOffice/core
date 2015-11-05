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
#include <com/sun/star/awt/FontSlant.hpp>
#include <com/sun/star/awt/Gradient.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/awt/XBitmap.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/PropertyValues.hpp>
#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/container/XIndexReplace.hpp>
#include <com/sun/star/drawing/BitmapMode.hpp>
#include <com/sun/star/drawing/ColorMode.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/Hatch.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/drawing/PointSequenceSequence.hpp>
#include <com/sun/star/drawing/RectanglePoint.hpp>
#include <com/sun/star/drawing/TextVerticalAdjust.hpp>
#include <com/sun/star/embed/XEmbeddedObject.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/i18n/XForbiddenCharacters.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/style/BreakType.hpp>
#include <com/sun/star/style/DropCapFormat.hpp>
#include <com/sun/star/style/GraphicLocation.hpp>
#include <com/sun/star/style/LineSpacing.hpp>
#include <com/sun/star/style/PageStyleLayout.hpp>
#include <com/sun/star/style/TabStop.hpp>
#include <com/sun/star/style/VerticalAlignment.hpp>
#include <com/sun/star/table/BorderLine.hpp>
#include <com/sun/star/table/ShadowFormat.hpp>
#include <com/sun/star/table/TableBorder.hpp>
#include <com/sun/star/table/TableBorder2.hpp>
#include <com/sun/star/table/TableBorderDistances.hpp>
#include <com/sun/star/table/XCell.hpp>
#include <com/sun/star/text/GraphicCrop.hpp>
#include <com/sun/star/text/PageNumberType.hpp>
#include <com/sun/star/text/SectionFileLink.hpp>
#include <com/sun/star/text/TableColumnSeparator.hpp>
#include <com/sun/star/text/TextContentAnchorType.hpp>
#include <com/sun/star/text/WrapTextMode.hpp>
#include <com/sun/star/text/XDependentTextField.hpp>
#include <com/sun/star/text/XDocumentIndex.hpp>
#include <com/sun/star/text/XDocumentIndexMark.hpp>
#include <com/sun/star/text/XFootnote.hpp>
#include <com/sun/star/text/XTextColumns.hpp>
#include <com/sun/star/text/XTextContent.hpp>
#include <com/sun/star/text/XTextField.hpp>
#include <com/sun/star/text/XTextFrame.hpp>
#include <com/sun/star/text/XTextSection.hpp>
#include <com/sun/star/text/XTextTable.hpp>
#include <com/sun/star/util/Date.hpp>
#include <com/sun/star/util/DateTime.hpp>
#include <com/sun/star/script/XLibraryContainer.hpp>
#include <com/sun/star/drawing/HomogenMatrix3.hpp>
#include <osl/diagnose.h>
#include <swtypes.hxx>
#include <unomap.hxx>
#include <unoprnms.hxx>
#include <unomid.h>
#include <cmdid.h>
#include <unofldmid.h>
#include <editeng/memberids.hrc>
#include <editeng/unoipset.hxx>
#include <editeng/unoprnms.hxx>
#include <svx/xdef.hxx>

using namespace ::com::sun::star;
using namespace css::lang;
using namespace css::uno;
using namespace css::beans;

#ifndef MID_TXT_LMARGIN
#define MID_TXT_LMARGIN 11
#endif

SwUnoPropertyMapProvider aSwMapProvider;

SwUnoPropertyMapProvider::SwUnoPropertyMapProvider()
{
    for( sal_uInt16 i = 0; i < PROPERTY_MAP_END; i++ )
    {
        m_aMapEntriesArr[i] = 0;
        m_aPropertySetArr[i] = 0;
    }
}

SwUnoPropertyMapProvider::~SwUnoPropertyMapProvider()
{
}

#define _STANDARD_FONT_PROPERTIES \
    { OUString(UNO_NAME_CHAR_HEIGHT), RES_CHRATR_FONTSIZE  ,  cppu::UnoType<float>::get(),         PropertyAttribute::MAYBEVOID, MID_FONTHEIGHT|CONVERT_TWIPS},      \
    { OUString(UNO_NAME_CHAR_WEIGHT), RES_CHRATR_WEIGHT    ,  cppu::UnoType<float>::get(),             PropertyAttribute::MAYBEVOID, MID_WEIGHT},                    \
    { OUString(UNO_NAME_CHAR_FONT_NAME), RES_CHRATR_FONT,       cppu::UnoType<OUString>::get(),  PropertyAttribute::MAYBEVOID, MID_FONT_FAMILY_NAME },                  \
    { OUString(UNO_NAME_CHAR_FONT_STYLE_NAME), RES_CHRATR_FONT,     cppu::UnoType<OUString>::get(), PropertyAttribute::MAYBEVOID, MID_FONT_STYLE_NAME },                    \
    { OUString(UNO_NAME_CHAR_FONT_FAMILY), RES_CHRATR_FONT,     cppu::UnoType<sal_Int16>::get(),                   PropertyAttribute::MAYBEVOID, MID_FONT_FAMILY   },    \
    { OUString(UNO_NAME_CHAR_FONT_CHAR_SET), RES_CHRATR_FONT,       cppu::UnoType<sal_Int16>::get(),   PropertyAttribute::MAYBEVOID, MID_FONT_CHAR_SET },                    \
    { OUString(UNO_NAME_CHAR_FONT_PITCH), RES_CHRATR_FONT,      cppu::UnoType<sal_Int16>::get(),                   PropertyAttribute::MAYBEVOID, MID_FONT_PITCH   },     \
    { OUString(UNO_NAME_CHAR_POSTURE), RES_CHRATR_POSTURE   ,  cppu::UnoType<css::awt::FontSlant>::get(),        PropertyAttribute::MAYBEVOID, MID_POSTURE},                   \
    { OUString(UNO_NAME_RSID), RES_CHRATR_RSID, cppu::UnoType<sal_Int32>::get(), PropertyAttribute::MAYBEVOID, 0 }, \
    { OUString(UNO_NAME_CHAR_LOCALE),           RES_CHRATR_LANGUAGE,    cppu::UnoType<css::lang::Locale>::get(),          PropertyAttribute::MAYBEVOID,   MID_LANG_LOCALE }, \
    { OUString(UNO_NAME_CHAR_INTEROP_GRAB_BAG), RES_CHRATR_GRABBAG,     cppu::UnoType< cppu::UnoSequenceType<css::beans::PropertyValue> >::get(),   PROPERTY_NONE,                  0               }, \

#define _CJK_FONT_PROPERTIES \
    { OUString(UNO_NAME_CHAR_HEIGHT_ASIAN), RES_CHRATR_CJK_FONTSIZE  ,  cppu::UnoType<float>::get(),           PropertyAttribute::MAYBEVOID, MID_FONTHEIGHT|CONVERT_TWIPS},   \
    { OUString(UNO_NAME_CHAR_WEIGHT_ASIAN), RES_CHRATR_CJK_WEIGHT    ,  cppu::UnoType<float>::get(),           PropertyAttribute::MAYBEVOID, MID_WEIGHT},                 \
    { OUString(UNO_NAME_CHAR_FONT_NAME_ASIAN), RES_CHRATR_CJK_FONT,     cppu::UnoType<OUString>::get(),  PropertyAttribute::MAYBEVOID, MID_FONT_FAMILY_NAME },               \
    { OUString(UNO_NAME_CHAR_FONT_STYLE_NAME_ASIAN), RES_CHRATR_CJK_FONT,       cppu::UnoType<OUString>::get(), PropertyAttribute::MAYBEVOID, MID_FONT_STYLE_NAME },                 \
    { OUString(UNO_NAME_CHAR_FONT_FAMILY_ASIAN), RES_CHRATR_CJK_FONT,       cppu::UnoType<sal_Int16>::get(),                   PropertyAttribute::MAYBEVOID, MID_FONT_FAMILY   },    \
    { OUString(UNO_NAME_CHAR_FONT_CHAR_SET_ASIAN), RES_CHRATR_CJK_FONT,     cppu::UnoType<sal_Int16>::get(),   PropertyAttribute::MAYBEVOID, MID_FONT_CHAR_SET },                 \
    { OUString(UNO_NAME_CHAR_FONT_PITCH_ASIAN), RES_CHRATR_CJK_FONT,        cppu::UnoType<sal_Int16>::get(),                   PropertyAttribute::MAYBEVOID, MID_FONT_PITCH   },     \
    { OUString(UNO_NAME_CHAR_POSTURE_ASIAN), RES_CHRATR_CJK_POSTURE   ,  cppu::UnoType<css::awt::FontSlant>::get(),          PropertyAttribute::MAYBEVOID, MID_POSTURE},                \
    { OUString(UNO_NAME_CHAR_LOCALE_ASIAN), RES_CHRATR_CJK_LANGUAGE ,   cppu::UnoType<css::lang::Locale>::get()  ,        PropertyAttribute::MAYBEVOID,  MID_LANG_LOCALE },

#define _CTL_FONT_PROPERTIES \
    { OUString(UNO_NAME_CHAR_HEIGHT_COMPLEX), RES_CHRATR_CTL_FONTSIZE  ,  cppu::UnoType<float>::get(),         PropertyAttribute::MAYBEVOID, MID_FONTHEIGHT|CONVERT_TWIPS},\
    { OUString(UNO_NAME_CHAR_WEIGHT_COMPLEX), RES_CHRATR_CTL_WEIGHT    ,  cppu::UnoType<float>::get(),             PropertyAttribute::MAYBEVOID, MID_WEIGHT},              \
    { OUString(UNO_NAME_CHAR_FONT_NAME_COMPLEX), RES_CHRATR_CTL_FONT,       cppu::UnoType<OUString>::get(),  PropertyAttribute::MAYBEVOID, MID_FONT_FAMILY_NAME },            \
    { OUString(UNO_NAME_CHAR_FONT_STYLE_NAME_COMPLEX), RES_CHRATR_CTL_FONT,     cppu::UnoType<OUString>::get(), PropertyAttribute::MAYBEVOID, MID_FONT_STYLE_NAME },              \
    { OUString(UNO_NAME_CHAR_FONT_FAMILY_COMPLEX), RES_CHRATR_CTL_FONT,     cppu::UnoType<sal_Int16>::get(),                   PropertyAttribute::MAYBEVOID, MID_FONT_FAMILY   },    \
    { OUString(UNO_NAME_CHAR_FONT_CHAR_SET_COMPLEX), RES_CHRATR_CTL_FONT,       cppu::UnoType<sal_Int16>::get(),   PropertyAttribute::MAYBEVOID, MID_FONT_CHAR_SET },              \
    { OUString(UNO_NAME_CHAR_FONT_PITCH_COMPLEX), RES_CHRATR_CTL_FONT,      cppu::UnoType<sal_Int16>::get(),                   PropertyAttribute::MAYBEVOID, MID_FONT_PITCH   },     \
    { OUString(UNO_NAME_CHAR_POSTURE_COMPLEX), RES_CHRATR_CTL_POSTURE   ,  cppu::UnoType<css::awt::FontSlant>::get(),        PropertyAttribute::MAYBEVOID, MID_POSTURE},             \
    { OUString(UNO_NAME_CHAR_LOCALE_COMPLEX), RES_CHRATR_CTL_LANGUAGE ,   cppu::UnoType<css::lang::Locale>::get()  ,          PropertyAttribute::MAYBEVOID,  MID_LANG_LOCALE },

#define _REDLINE_NODE_PROPERTIES \
    { OUString(UNO_NAME_START_REDLINE), FN_UNO_REDLINE_NODE_START , cppu::UnoType< cppu::UnoSequenceType<css::beans::PropertyValue> >::get(),   PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY,  0xff }, \
    { OUString(UNO_NAME_END_REDLINE), FN_UNO_REDLINE_NODE_END ,     cppu::UnoType< cppu::UnoSequenceType<css::beans::PropertyValue> >::get(),       PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY,  0xff },

#define _REDLINE_PROPERTIES \
    {OUString(UNO_NAME_REDLINE_AUTHOR), 0, cppu::UnoType<OUString>::get(),                     PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY,   0},\
    {OUString(UNO_NAME_REDLINE_DATE_TIME), 0, cppu::UnoType<css::util::DateTime>::get(),                  PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY,   0},\
    {OUString(UNO_NAME_REDLINE_COMMENT), 0, cppu::UnoType<OUString>::get(),                        PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY,   0},\
    {OUString(UNO_NAME_REDLINE_TYPE), 0, cppu::UnoType<OUString>::get(),                       PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY,   0},\
    {OUString(UNO_NAME_REDLINE_SUCCESSOR_DATA), 0, cppu::UnoType< cppu::UnoSequenceType<css::beans::PropertyValue> >::get(),    PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY,   0},\
    {OUString(UNO_NAME_REDLINE_IDENTIFIER), 0, cppu::UnoType<OUString>::get(),                         PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY, 0},\
    {OUString(UNO_NAME_IS_IN_HEADER_FOOTER), 0, cppu::UnoType<bool>::get(),                             PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY, 0},\
    {OUString(UNO_NAME_REDLINE_TEXT), 0, cppu::UnoType<css::text::XText>::get(),                    PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY,   0},\
    {OUString(UNO_NAME_MERGE_LAST_PARA), 0, cppu::UnoType<bool>::get(),                             PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY, 0},

#define COMMON_CRSR_PARA_PROPERTIES_FN_ONLY \
        { OUString(UNO_NAME_PARA_STYLE_NAME), FN_UNO_PARA_STYLE,        cppu::UnoType<OUString>::get(),                PropertyAttribute::MAYBEVOID,     0},                                                       \
        { OUString(UNO_NAME_PAGE_STYLE_NAME), FN_UNO_PAGE_STYLE,        cppu::UnoType<OUString>::get(),         PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY,   0},                       \
        { OUString(UNO_NAME_NUMBERING_IS_NUMBER), FN_UNO_IS_NUMBER,     cppu::UnoType<bool>::get()  ,       PropertyAttribute::MAYBEVOID,     0},                                                                 \
        { OUString(UNO_NAME_NUMBERING_LEVEL), FN_UNO_NUM_LEVEL,     cppu::UnoType<sal_Int16>::get(),           PropertyAttribute::MAYBEVOID, 0},                                                            \
        { OUString(UNO_NAME_NUMBERING_RULES), FN_UNO_NUM_RULES,     cppu::UnoType<css::container::XIndexReplace>::get(),  PropertyAttribute::MAYBEVOID, CONVERT_TWIPS},                        \
        { OUString(UNO_NAME_NUMBERING_START_VALUE), FN_UNO_NUM_START_VALUE, cppu::UnoType<sal_Int16>::get(),           PropertyAttribute::MAYBEVOID, CONVERT_TWIPS},                                                \
        { OUString(UNO_NAME_DOCUMENT_INDEX), FN_UNO_DOCUMENT_INDEX, cppu::UnoType<css::text::XDocumentIndex>::get(), PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY ,0 },            \
        { OUString(UNO_NAME_TEXT_TABLE), FN_UNO_TEXT_TABLE,     cppu::UnoType<css::text::XTextTable>::get(),     PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY ,0 },               \
        { OUString(UNO_NAME_CELL), FN_UNO_CELL,         cppu::UnoType<css::table::XCell>::get(),         PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY ,0 },                     \
        { OUString(UNO_NAME_TEXT_FRAME), FN_UNO_TEXT_FRAME,     cppu::UnoType<css::text::XTextFrame>::get(),        PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY ,0 },                     \
        { OUString(UNO_NAME_TEXT_SECTION), FN_UNO_TEXT_SECTION, cppu::UnoType<css::text::XTextSection>::get(),  PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY ,0 },                    \
        { OUString(UNO_NAME_PARA_CHAPTER_NUMBERING_LEVEL), FN_UNO_PARA_CHAPTER_NUMBERING_LEVEL,cppu::UnoType<sal_Int8>::get(), PROPERTY_NONE, 0},                                                     \
        { OUString(UNO_NAME_PARA_CONDITIONAL_STYLE_NAME), FN_UNO_PARA_CONDITIONAL_STYLE_NAME, cppu::UnoType<OUString>::get(),      PropertyAttribute::READONLY, 0},                                                     \
        { OUString(UNO_NAME_LIST_ID), FN_UNO_LIST_ID, cppu::UnoType<OUString>::get(), PropertyAttribute::MAYBEVOID, 0}, \
        { OUString(UNO_NAME_PARA_IS_NUMBERING_RESTART), FN_NUMBER_NEWSTART,     cppu::UnoType<bool>::get(),     PropertyAttribute::MAYBEVOID, 0 }, \
        { OUString(UNO_NAME_PARA_CONTINUEING_PREVIOUS_SUB_TREE), FN_UNO_PARA_CONT_PREV_SUBTREE, cppu::UnoType<bool>::get(), PropertyAttribute::READONLY, 0 }, \
        { OUString(UNO_NAME_PARA_LIST_LABEL_STRING), FN_UNO_PARA_NUM_STRING, cppu::UnoType<OUString>::get(), PropertyAttribute::READONLY, 0 }, \
        { OUString(UNO_NAME_OUTLINE_LEVEL), RES_PARATR_OUTLINELEVEL,        cppu::UnoType<sal_Int16>::get(),                PropertyAttribute::MAYBEVOID,     0},

#define COMMON_HYPERLINK_PROPERTIES \
        { OUString(UNO_NAME_HYPER_LINK_U_R_L), RES_TXTATR_INETFMT,          cppu::UnoType<OUString>::get(), PropertyAttribute::MAYBEVOID ,MID_URL_URL},                \
        { OUString(UNO_NAME_HYPER_LINK_TARGET), RES_TXTATR_INETFMT,         cppu::UnoType<OUString>::get(), PropertyAttribute::MAYBEVOID ,MID_URL_TARGET},             \
        { OUString(UNO_NAME_HYPER_LINK_NAME), RES_TXTATR_INETFMT,           cppu::UnoType<OUString>::get(), PropertyAttribute::MAYBEVOID ,MID_URL_HYPERLINKNAME  },    \
        { OUString(UNO_NAME_UNVISITED_CHAR_STYLE_NAME), RES_TXTATR_INETFMT, cppu::UnoType<OUString>::get(), PropertyAttribute::MAYBEVOID ,MID_URL_UNVISITED_FMT   },   \
        { OUString(UNO_NAME_VISITED_CHAR_STYLE_NAME), RES_TXTATR_INETFMT,   cppu::UnoType<OUString>::get(), PropertyAttribute::MAYBEVOID ,MID_URL_VISITED_FMT  },

// same as COMMON_CRSR_PARA_PROPERTIES_WITHOUT_FN_01 but without
// OUString(UNO_NAME_BREAK_TYPE) and OUString(UNO_NAME_PAGE_DESC_NAME) which can not be used
// by the SwXTextTableCursor
#define COMMON_CRSR_PARA_PROPERTIES_WITHOUT_FN_01 \
        { OUString(UNO_NAME_PARRSID),                             RES_PARATR_RSID,               cppu::UnoType<sal_Int32>::get(),         PropertyAttribute::MAYBEVOID, 0                                      }, \
        { OUString(UNO_NAME_PARA_IS_HYPHENATION),                 RES_PARATR_HYPHENZONE,         cppu::UnoType<bool>::get(),       PropertyAttribute::MAYBEVOID, MID_IS_HYPHEN                          }, \
        { OUString(UNO_NAME_PARA_HYPHENATION_MAX_LEADING_CHARS),  RES_PARATR_HYPHENZONE,         cppu::UnoType<sal_Int16>::get(),         PropertyAttribute::MAYBEVOID, MID_HYPHEN_MIN_LEAD                    }, \
        { OUString(UNO_NAME_PARA_HYPHENATION_MAX_TRAILING_CHARS), RES_PARATR_HYPHENZONE,         cppu::UnoType<sal_Int16>::get(),         PropertyAttribute::MAYBEVOID, MID_HYPHEN_MIN_TRAIL                   }, \
        { OUString(UNO_NAME_PARA_HYPHENATION_MAX_HYPHENS),        RES_PARATR_HYPHENZONE,         cppu::UnoType<sal_Int16>::get(),         PropertyAttribute::MAYBEVOID, MID_HYPHEN_MAX_HYPHENS                 }, \
        { OUString(UNO_NAME_CHAR_AUTO_KERNING),                   RES_CHRATR_AUTOKERN,           cppu::UnoType<bool>::get(),       PropertyAttribute::MAYBEVOID, 0                                      }, \
        { OUString(UNO_NAME_CHAR_BACK_COLOR),                     RES_CHRATR_BACKGROUND,         cppu::UnoType<sal_Int32>::get(),         PropertyAttribute::MAYBEVOID, MID_BACK_COLOR                         }, \
        { OUString(UNO_NAME_CHAR_HIGHLIGHT),                      RES_CHRATR_HIGHLIGHT,          cppu::UnoType<sal_Int32>::get(), PropertyAttribute::MAYBEVOID, MID_BACK_COLOR                         }, \
        { OUString(UNO_NAME_PARA_BACK_COLOR),                     RES_BACKGROUND,                cppu::UnoType<sal_Int32>::get(),         PropertyAttribute::MAYBEVOID, MID_BACK_COLOR                         }, \
        { OUString(UNO_NAME_CHAR_CASE_MAP),                       RES_CHRATR_CASEMAP,            cppu::UnoType<sal_Int16>::get(),         PropertyAttribute::MAYBEVOID, 0                                      }, \
        { OUString(UNO_NAME_CHAR_COLOR),                          RES_CHRATR_COLOR,              cppu::UnoType<sal_Int32>::get(),         PropertyAttribute::MAYBEVOID, 0                                      }, \
        { OUString(UNO_NAME_CHAR_STRIKEOUT),                      RES_CHRATR_CROSSEDOUT,         cppu::UnoType<sal_Int16>::get(),         PropertyAttribute::MAYBEVOID, MID_CROSS_OUT                          }, \
        { OUString(UNO_NAME_CHAR_CROSSED_OUT),                    RES_CHRATR_CROSSEDOUT,         cppu::UnoType<bool>::get(),       PropertyAttribute::MAYBEVOID, MID_CROSSED_OUT                        }, \
        { OUString(UNO_NAME_CHAR_ESCAPEMENT),                     RES_CHRATR_ESCAPEMENT,         cppu::UnoType<sal_Int16>::get(),         PropertyAttribute::MAYBEVOID, MID_ESC                                }, \
        { OUString(UNO_NAME_CHAR_ESCAPEMENT_HEIGHT),              RES_CHRATR_ESCAPEMENT,         cppu::UnoType<sal_Int8>::get(),          PropertyAttribute::MAYBEVOID, MID_ESC_HEIGHT                         }, \
        { OUString(UNO_NAME_CHAR_AUTO_ESCAPEMENT),                RES_CHRATR_ESCAPEMENT,         cppu::UnoType<bool>::get(),       PropertyAttribute::MAYBEVOID, MID_AUTO_ESC                           }, \
        { OUString(UNO_NAME_CHAR_FLASH),                          RES_CHRATR_BLINK,              cppu::UnoType<bool>::get(),       PropertyAttribute::MAYBEVOID, 0                                      }, \
        { OUString(UNO_NAME_CHAR_HIDDEN),                         RES_CHRATR_HIDDEN,             cppu::UnoType<bool>::get(),       PropertyAttribute::MAYBEVOID, 0                                      }, \
        { OUString(UNO_NAME_CHAR_UNDERLINE),                      RES_CHRATR_UNDERLINE,          cppu::UnoType<sal_Int16>::get(),         PropertyAttribute::MAYBEVOID, MID_TL_STYLE                           }, \
        { OUString(UNO_NAME_CHAR_UNDERLINE_COLOR),                RES_CHRATR_UNDERLINE,          cppu::UnoType<sal_Int32>::get(),         PropertyAttribute::MAYBEVOID, MID_TL_COLOR                           }, \
        { OUString(UNO_NAME_CHAR_UNDERLINE_HAS_COLOR),            RES_CHRATR_UNDERLINE,          cppu::UnoType<bool>::get(),       PropertyAttribute::MAYBEVOID, MID_TL_HASCOLOR                        }, \
        { OUString(UNO_NAME_CHAR_OVERLINE),                       RES_CHRATR_OVERLINE,           cppu::UnoType<sal_Int16>::get(),         PropertyAttribute::MAYBEVOID, MID_TL_STYLE                           }, \
        { OUString(UNO_NAME_CHAR_OVERLINE_COLOR),                 RES_CHRATR_OVERLINE,           cppu::UnoType<sal_Int32>::get(),         PropertyAttribute::MAYBEVOID, MID_TL_COLOR                           }, \
        { OUString(UNO_NAME_CHAR_OVERLINE_HAS_COLOR),             RES_CHRATR_OVERLINE,           cppu::UnoType<bool>::get(),       PropertyAttribute::MAYBEVOID, MID_TL_HASCOLOR                        }, \
        { OUString(UNO_NAME_PARA_GRAPHIC_URL),                    RES_BACKGROUND,                cppu::UnoType<OUString>::get(),      PropertyAttribute::MAYBEVOID, MID_GRAPHIC_URL                        }, \
        { OUString(UNO_NAME_PARA_GRAPHIC_FILTER),                 RES_BACKGROUND,                cppu::UnoType<OUString>::get(),      PropertyAttribute::MAYBEVOID, MID_GRAPHIC_FILTER                     }, \
        { OUString(UNO_NAME_PARA_GRAPHIC_LOCATION),               RES_BACKGROUND,                cppu::UnoType<css::style::GraphicLocation>::get(),    PropertyAttribute::MAYBEVOID, MID_GRAPHIC_POSITION                   }, \
        { OUString(UNO_NAME_PARA_LEFT_MARGIN),                    RES_LR_SPACE,                  cppu::UnoType<sal_Int32>::get(),         PropertyAttribute::MAYBEVOID, MID_TXT_LMARGIN        | CONVERT_TWIPS }, \
        { OUString(UNO_NAME_PARA_RIGHT_MARGIN),                   RES_LR_SPACE,                  cppu::UnoType<sal_Int32>::get(),         PropertyAttribute::MAYBEVOID, MID_R_MARGIN           | CONVERT_TWIPS }, \
        { OUString(UNO_NAME_PARA_IS_AUTO_FIRST_LINE_INDENT),      RES_LR_SPACE,                  cppu::UnoType<bool>::get(),       PropertyAttribute::MAYBEVOID, MID_FIRST_AUTO                         }, \
        { OUString(UNO_NAME_PARA_FIRST_LINE_INDENT),              RES_LR_SPACE,                  cppu::UnoType<sal_Int32>::get(),         PropertyAttribute::MAYBEVOID, MID_FIRST_LINE_INDENT  | CONVERT_TWIPS }, \
        _STANDARD_FONT_PROPERTIES \
        _CJK_FONT_PROPERTIES \
        _CTL_FONT_PROPERTIES \
        { OUString(UNO_NAME_CHAR_KERNING),                        RES_CHRATR_KERNING,            cppu::UnoType<sal_Int16>::get(),         PropertyAttribute::MAYBEVOID, CONVERT_TWIPS                          }, \
        { OUString(UNO_NAME_CHAR_NO_HYPHENATION),                 RES_CHRATR_NOHYPHEN,           cppu::UnoType<bool>::get(),       PropertyAttribute::MAYBEVOID, 0                                      }, \
        { OUString(UNO_NAME_CHAR_SHADOWED),                       RES_CHRATR_SHADOWED,           cppu::UnoType<bool>::get(),       PropertyAttribute::MAYBEVOID, 0                                      }, \
        { OUString(UNO_NAME_CHAR_CONTOURED),                      RES_CHRATR_CONTOUR,            cppu::UnoType<bool>::get(),       PropertyAttribute::MAYBEVOID, 0                                      }, \
        { OUString(UNO_NAME_DROP_CAP_FORMAT),                     RES_PARATR_DROP,               cppu::UnoType<css::style::DropCapFormat>::get(),    PropertyAttribute::MAYBEVOID, MID_DROPCAP_FORMAT     | CONVERT_TWIPS }, \
        { OUString(UNO_NAME_DROP_CAP_WHOLE_WORD),                 RES_PARATR_DROP,               cppu::UnoType<bool>::get(),       PropertyAttribute::MAYBEVOID, MID_DROPCAP_WHOLE_WORD                 }, \
        { OUString(UNO_NAME_DROP_CAP_CHAR_STYLE_NAME),            RES_PARATR_DROP,               cppu::UnoType<OUString>::get(),      PropertyAttribute::MAYBEVOID, MID_DROPCAP_CHAR_STYLE_NAME            }, \
        { OUString(UNO_NAME_PARA_KEEP_TOGETHER),                  RES_KEEP,                      cppu::UnoType<bool>::get(),       PropertyAttribute::MAYBEVOID, 0                                      }, \
        { OUString(UNO_NAME_PARA_SPLIT),                          RES_PARATR_SPLIT,              cppu::UnoType<bool>::get(),       PropertyAttribute::MAYBEVOID, 0                                      }, \
        { OUString(UNO_NAME_PARA_WIDOWS),                         RES_PARATR_WIDOWS,             cppu::UnoType<sal_Int8>::get(),          PropertyAttribute::MAYBEVOID, 0                                      }, \
        { OUString(UNO_NAME_PARA_ORPHANS),                        RES_PARATR_ORPHANS,            cppu::UnoType<sal_Int8>::get(),          PropertyAttribute::MAYBEVOID, 0                                      }, \
        { OUString(UNO_NAME_PAGE_NUMBER_OFFSET),                  RES_PAGEDESC,                  cppu::UnoType<sal_Int16>::get(),         PropertyAttribute::MAYBEVOID, MID_PAGEDESC_PAGENUMOFFSET             }, \
        { OUString(UNO_NAME_PARA_ADJUST),                         RES_PARATR_ADJUST,             cppu::UnoType<sal_Int16>::get(),         PropertyAttribute::MAYBEVOID, MID_PARA_ADJUST                        }, \
        { OUString(UNO_NAME_PARA_EXPAND_SINGLE_WORD),             RES_PARATR_ADJUST,             cppu::UnoType<bool>::get(),       PropertyAttribute::MAYBEVOID, MID_EXPAND_SINGLE                      }, \
        { OUString(UNO_NAME_PARA_LAST_LINE_ADJUST),               RES_PARATR_ADJUST,             cppu::UnoType<sal_Int16>::get(),         PropertyAttribute::MAYBEVOID, MID_LAST_LINE_ADJUST                   }, \
        { OUString(UNO_NAME_PARA_LINE_NUMBER_COUNT),              RES_LINENUMBER,                cppu::UnoType<bool>::get(),       PropertyAttribute::MAYBEVOID, MID_LINENUMBER_COUNT                   }, \
        { OUString(UNO_NAME_PARA_LINE_NUMBER_START_VALUE),        RES_LINENUMBER,                cppu::UnoType<sal_Int32>::get(),         PropertyAttribute::MAYBEVOID, MID_LINENUMBER_STARTVALUE              }, \
        { OUString(UNO_NAME_PARA_LINE_SPACING),                   RES_PARATR_LINESPACING,        cppu::UnoType<css::style::LineSpacing>::get(),     PropertyAttribute::MAYBEVOID, CONVERT_TWIPS                          }, \
        { OUString(UNO_NAME_PARA_REGISTER_MODE_ACTIVE),           RES_PARATR_REGISTER,           cppu::UnoType<bool>::get(),       PropertyAttribute::MAYBEVOID, 0                                      }, \
        { OUString(UNO_NAME_PARA_TOP_MARGIN),                     RES_UL_SPACE,                  cppu::UnoType<sal_Int32>::get(),         PropertyAttribute::MAYBEVOID, MID_UP_MARGIN          | CONVERT_TWIPS }, \
        { OUString(UNO_NAME_PARA_BOTTOM_MARGIN),                  RES_UL_SPACE,                  cppu::UnoType<sal_Int32>::get(),         PropertyAttribute::MAYBEVOID, MID_LO_MARGIN          | CONVERT_TWIPS }, \
        { OUString(UNO_NAME_PARA_CONTEXT_MARGIN),                 RES_UL_SPACE,                  cppu::UnoType<bool>::get(),       PropertyAttribute::MAYBEVOID, MID_CTX_MARGIN                         }, \
        { OUString(UNO_NAME_CHAR_BACK_TRANSPARENT),               RES_CHRATR_BACKGROUND,         cppu::UnoType<bool>::get(),       PropertyAttribute::MAYBEVOID, MID_GRAPHIC_TRANSPARENT                }, \
        { OUString(UNO_NAME_PARA_BACK_TRANSPARENT),               RES_BACKGROUND,                cppu::UnoType<bool>::get(),       PropertyAttribute::MAYBEVOID, MID_GRAPHIC_TRANSPARENT                }, \
        { OUString(UNO_NAME_NUMBERING_STYLE_NAME),                RES_PARATR_NUMRULE,            cppu::UnoType<OUString>::get(),      PropertyAttribute::MAYBEVOID, 0                                      }, \
        { OUString(UNO_NAME_CHAR_WORD_MODE),                      RES_CHRATR_WORDLINEMODE,       cppu::UnoType<bool>::get(),       PropertyAttribute::MAYBEVOID, 0                                      }, \
        { OUString(UNO_NAME_CHAR_LEFT_BORDER),                    RES_CHRATR_BOX,                cppu::UnoType<css::table::BorderLine>::get(),    PropertyAttribute::MAYBEVOID, LEFT_BORDER            | CONVERT_TWIPS }, \
        { OUString(UNO_NAME_CHAR_RIGHT_BORDER),                   RES_CHRATR_BOX,                cppu::UnoType<css::table::BorderLine>::get(),    PropertyAttribute::MAYBEVOID, RIGHT_BORDER           | CONVERT_TWIPS }, \
        { OUString(UNO_NAME_CHAR_TOP_BORDER),                     RES_CHRATR_BOX,                cppu::UnoType<css::table::BorderLine>::get(),    PropertyAttribute::MAYBEVOID, TOP_BORDER             | CONVERT_TWIPS }, \
        { OUString(UNO_NAME_CHAR_BOTTOM_BORDER),                  RES_CHRATR_BOX,                cppu::UnoType<css::table::BorderLine>::get(),    PropertyAttribute::MAYBEVOID, BOTTOM_BORDER          | CONVERT_TWIPS }, \
        { OUString(UNO_NAME_CHAR_BORDER_DISTANCE),                RES_CHRATR_BOX,                cppu::UnoType<sal_Int32>::get(),         PropertyAttribute::MAYBEVOID, BORDER_DISTANCE        | CONVERT_TWIPS }, \
        { OUString(UNO_NAME_CHAR_LEFT_BORDER_DISTANCE),           RES_CHRATR_BOX,                cppu::UnoType<sal_Int32>::get(),         PropertyAttribute::MAYBEVOID, LEFT_BORDER_DISTANCE   | CONVERT_TWIPS }, \
        { OUString(UNO_NAME_CHAR_RIGHT_BORDER_DISTANCE),          RES_CHRATR_BOX,                cppu::UnoType<sal_Int32>::get(),         PropertyAttribute::MAYBEVOID, RIGHT_BORDER_DISTANCE  | CONVERT_TWIPS }, \
        { OUString(UNO_NAME_CHAR_TOP_BORDER_DISTANCE),            RES_CHRATR_BOX,                cppu::UnoType<sal_Int32>::get(),         PropertyAttribute::MAYBEVOID, TOP_BORDER_DISTANCE    | CONVERT_TWIPS }, \
        { OUString(UNO_NAME_CHAR_BOTTOM_BORDER_DISTANCE),         RES_CHRATR_BOX,                cppu::UnoType<sal_Int32>::get(),         PropertyAttribute::MAYBEVOID, BOTTOM_BORDER_DISTANCE | CONVERT_TWIPS }, \
        { OUString(UNO_NAME_CHAR_SHADOW_FORMAT),                  RES_CHRATR_SHADOW,             cppu::UnoType<css::table::ShadowFormat>::get(),     PropertyAttribute::MAYBEVOID, CONVERT_TWIPS                          }, \
        { OUString(UNO_NAME_LEFT_BORDER),                         RES_BOX,                       cppu::UnoType<css::table::BorderLine>::get(),    PropertyAttribute::MAYBEVOID, LEFT_BORDER            | CONVERT_TWIPS }, \
        { OUString(UNO_NAME_RIGHT_BORDER),                        RES_BOX,                       cppu::UnoType<css::table::BorderLine>::get(),    PropertyAttribute::MAYBEVOID, RIGHT_BORDER           | CONVERT_TWIPS }, \
        { OUString(UNO_NAME_TOP_BORDER),                          RES_BOX,                       cppu::UnoType<css::table::BorderLine>::get(),    PropertyAttribute::MAYBEVOID, TOP_BORDER             | CONVERT_TWIPS }, \
        { OUString(UNO_NAME_BOTTOM_BORDER),                       RES_BOX,                       cppu::UnoType<css::table::BorderLine>::get(),    PropertyAttribute::MAYBEVOID, BOTTOM_BORDER          | CONVERT_TWIPS }, \
        { OUString(UNO_NAME_BORDER_DISTANCE),                     RES_BOX,                       cppu::UnoType<sal_Int32>::get(),         PropertyAttribute::MAYBEVOID, BORDER_DISTANCE        | CONVERT_TWIPS }, \
        { OUString(UNO_NAME_LEFT_BORDER_DISTANCE),                RES_BOX,                       cppu::UnoType<sal_Int32>::get(),         PropertyAttribute::MAYBEVOID, LEFT_BORDER_DISTANCE   | CONVERT_TWIPS }, \
        { OUString(UNO_NAME_RIGHT_BORDER_DISTANCE),               RES_BOX,                       cppu::UnoType<sal_Int32>::get(),         PropertyAttribute::MAYBEVOID, RIGHT_BORDER_DISTANCE  | CONVERT_TWIPS }, \
        { OUString(UNO_NAME_TOP_BORDER_DISTANCE),                 RES_BOX,                       cppu::UnoType<sal_Int32>::get(),         PropertyAttribute::MAYBEVOID, TOP_BORDER_DISTANCE    | CONVERT_TWIPS }, \
        { OUString(UNO_NAME_BOTTOM_BORDER_DISTANCE),              RES_BOX,                       cppu::UnoType<sal_Int32>::get(),         PropertyAttribute::MAYBEVOID, BOTTOM_BORDER_DISTANCE | CONVERT_TWIPS }, \
        { OUString(UNO_NAME_PARA_USER_DEFINED_ATTRIBUTES),        RES_UNKNOWNATR_CONTAINER,      cppu::UnoType<css::container::XNameContainer>::get(),    PropertyAttribute::MAYBEVOID, 0                                      }, \
        { OUString(UNO_NAME_TEXT_USER_DEFINED_ATTRIBUTES),        RES_TXTATR_UNKNOWN_CONTAINER,  cppu::UnoType<css::container::XNameContainer>::get(),    PropertyAttribute::MAYBEVOID, 0                                      }, \
        { OUString(UNO_NAME_PARA_SHADOW_FORMAT),                  RES_SHADOW,                    cppu::UnoType<css::table::ShadowFormat>::get(),     PROPERTY_NONE,                CONVERT_TWIPS                          }, \
        { OUString(UNO_NAME_CHAR_COMBINE_IS_ON),                  RES_CHRATR_TWO_LINES,          cppu::UnoType<bool>::get(),       PropertyAttribute::MAYBEVOID, MID_TWOLINES                           }, \
        { OUString(UNO_NAME_CHAR_COMBINE_PREFIX),                 RES_CHRATR_TWO_LINES,          cppu::UnoType<OUString>::get(),      PropertyAttribute::MAYBEVOID, MID_START_BRACKET                      }, \
        { OUString(UNO_NAME_CHAR_COMBINE_SUFFIX),                 RES_CHRATR_TWO_LINES,          cppu::UnoType<OUString>::get(),      PropertyAttribute::MAYBEVOID, MID_END_BRACKET                        }, \
        { OUString(UNO_NAME_CHAR_EMPHASIS),                       RES_CHRATR_EMPHASIS_MARK,      cppu::UnoType<sal_Int16>::get(),         PropertyAttribute::MAYBEVOID, MID_EMPHASIS                           }, \
        { OUString(UNO_NAME_PARA_IS_HANGING_PUNCTUATION),         RES_PARATR_HANGINGPUNCTUATION, cppu::UnoType<bool>::get(),       PropertyAttribute::MAYBEVOID, 0                                      }, \
        { OUString(UNO_NAME_PARA_IS_CHARACTER_DISTANCE),          RES_PARATR_SCRIPTSPACE,        cppu::UnoType<bool>::get(),       PropertyAttribute::MAYBEVOID, 0                                      }, \
        { OUString(UNO_NAME_PARA_IS_FORBIDDEN_RULES),             RES_PARATR_FORBIDDEN_RULES,    cppu::UnoType<bool>::get(),       PropertyAttribute::MAYBEVOID, 0                                      }, \
        { OUString(UNO_NAME_PARA_VERT_ALIGNMENT),                 RES_PARATR_VERTALIGN,          cppu::UnoType<sal_Int16>::get(),         PropertyAttribute::MAYBEVOID, 0                                      }, \
        { OUString(UNO_NAME_CHAR_ROTATION),                       RES_CHRATR_ROTATE,             cppu::UnoType<sal_Int16>::get(),         PropertyAttribute::MAYBEVOID, MID_ROTATE                             }, \
        { OUString(UNO_NAME_CHAR_ROTATION_IS_FIT_TO_LINE),        RES_CHRATR_ROTATE,             cppu::UnoType<bool>::get(),       PropertyAttribute::MAYBEVOID, MID_FITTOLINE                          }, \
        { OUString(UNO_NAME_CHAR_SCALE_WIDTH),                    RES_CHRATR_SCALEW,             cppu::UnoType<sal_Int16>::get(),         PropertyAttribute::MAYBEVOID, 0                                      }, \
        { OUString(UNO_NAME_RUBY_TEXT),                           RES_TXTATR_CJK_RUBY,           cppu::UnoType<OUString>::get(),      PropertyAttribute::MAYBEVOID, MID_RUBY_TEXT                          }, \
        { OUString(UNO_NAME_RUBY_ADJUST),                         RES_TXTATR_CJK_RUBY,           cppu::UnoType<sal_Int16>::get(),         PropertyAttribute::MAYBEVOID, MID_RUBY_ADJUST                        }, \
        { OUString(UNO_NAME_RUBY_CHAR_STYLE_NAME),                RES_TXTATR_CJK_RUBY,           cppu::UnoType<OUString>::get(),      PropertyAttribute::MAYBEVOID, MID_RUBY_CHARSTYLE                     }, \
        { OUString(UNO_NAME_RUBY_IS_ABOVE),                       RES_TXTATR_CJK_RUBY,           cppu::UnoType<bool>::get(),       PropertyAttribute::MAYBEVOID, MID_RUBY_ABOVE                         }, \
        { OUString(UNO_NAME_CHAR_RELIEF),                         RES_CHRATR_RELIEF,             cppu::UnoType<sal_Int16>::get(),         PropertyAttribute::MAYBEVOID, MID_RELIEF                             }, \
        { OUString(UNO_NAME_SNAP_TO_GRID),                        RES_PARATR_SNAPTOGRID,         cppu::UnoType<bool>::get(),       PropertyAttribute::MAYBEVOID, 0                                      }, \
        { OUString(UNO_NAME_PARA_IS_CONNECT_BORDER),              RES_PARATR_CONNECT_BORDER,     cppu::UnoType<bool>::get(),       PropertyAttribute::MAYBEVOID, 0                                      }, \
        { OUString(UNO_NAME_WRITING_MODE),                        RES_FRAMEDIR,                  cppu::UnoType<sal_Int16>::get(),         PROPERTY_NONE,                0                                      }, \
        { OUString(UNO_NAME_CHAR_SHADING_VALUE),                  RES_CHRATR_BACKGROUND,         cppu::UnoType<sal_Int32>::get(),         PROPERTY_NONE,                MID_SHADING_VALUE                      }, \
        { OUString(UNO_NAME_PARA_INTEROP_GRAB_BAG),               RES_PARATR_GRABBAG,            cppu::UnoType< cppu::UnoSequenceType<css::beans::PropertyValue> >::get(), PROPERTY_NONE,                0                                      }, \

#define COMMON_CRSR_PARA_PROPERTIES_WITHOUT_FN \
        COMMON_CRSR_PARA_PROPERTIES_WITHOUT_FN_01 \
        { OUString(UNO_NAME_BREAK_TYPE), RES_BREAK,                 cppu::UnoType<css::style::BreakType>::get(),           PropertyAttribute::MAYBEVOID, 0}, \
        { OUString(UNO_NAME_PAGE_DESC_NAME), RES_PAGEDESC,          cppu::UnoType<OUString>::get(),            PropertyAttribute::MAYBEVOID, MID_PAGEDESC_PAGEDESCNAME },

#define TABSTOPS_MAP_ENTRY                { OUString(UNO_NAME_TABSTOPS), RES_PARATR_TABSTOP,   cppu::UnoType< cppu::UnoSequenceType<css::style::TabStop> >::get(),   PropertyAttribute::MAYBEVOID, CONVERT_TWIPS},

#define COMMON_CRSR_PARA_PROPERTIES \
        COMMON_CRSR_PARA_PROPERTIES_FN_ONLY \
        COMMON_CRSR_PARA_PROPERTIES_WITHOUT_FN \
        COMMON_HYPERLINK_PROPERTIES \
        { OUString(UNO_NAME_CHAR_STYLE_NAME), RES_TXTATR_CHARFMT,     cppu::UnoType<OUString>::get(),         PropertyAttribute::MAYBEVOID,     0},\
        { OUString(UNO_NAME_CHAR_STYLE_NAMES), FN_UNO_CHARFMT_SEQUENCE,  cppu::UnoType< cppu::UnoSequenceType<OUString> >::get(),     PropertyAttribute::MAYBEVOID,     0}, \
        { OUString(UNO_NAME_CHAR_AUTO_STYLE_NAME), RES_TXTATR_AUTOFMT,     cppu::UnoType<OUString>::get(),         PropertyAttribute::MAYBEVOID,     0},\
        { OUString(UNO_NAME_PARA_AUTO_STYLE_NAME), RES_AUTO_STYLE,     cppu::UnoType<OUString>::get(),         PropertyAttribute::MAYBEVOID,     0},

#define COMMON_CRSR_PARA_PROPERTIES_2 \
        COMMON_CRSR_PARA_PROPERTIES_FN_ONLY \
        COMMON_CRSR_PARA_PROPERTIES_WITHOUT_FN

#define  COMPLETE_TEXT_CURSOR_MAP\
        COMMON_CRSR_PARA_PROPERTIES\
        { OUString(UNO_NAME_DOCUMENT_INDEX_MARK), FN_UNO_DOCUMENT_INDEX_MARK, cppu::UnoType<css::text::XDocumentIndexMark>::get(), PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY ,0 },\
        { OUString(UNO_NAME_TEXT_FIELD), FN_UNO_TEXT_FIELD,      cppu::UnoType<css::text::XTextField>::get(),  PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY ,0 },\
        { OUString(UNO_NAME_REFERENCE_MARK), FN_UNO_REFERENCE_MARK,  cppu::UnoType<css::text::XTextContent>::get(), PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY, 0 },\
        { OUString(UNO_NAME_FOOTNOTE), FN_UNO_FOOTNOTE,        cppu::UnoType<css::text::XFootnote>::get(),  PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY ,0 },\
        { OUString(UNO_NAME_ENDNOTE), FN_UNO_ENDNOTE,         cppu::UnoType<css::text::XFootnote>::get(),  PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY ,0 },\
        { OUString(UNO_NAME_HYPER_LINK_EVENTS), RES_TXTATR_INETFMT,     cppu::UnoType<css::container::XNameReplace>::get(), PropertyAttribute::MAYBEVOID, MID_URL_HYPERLINKEVENTS},\
        { OUString(UNO_NAME_NESTED_TEXT_CONTENT), FN_UNO_NESTED_TEXT_CONTENT, cppu::UnoType<css::text::XTextContent>::get(), PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY, 0 },\
        TABSTOPS_MAP_ENTRY

#define _BASE_INDEX_PROPERTIES_\
        { OUString(UNO_NAME_TITLE), WID_IDX_TITLE,  cppu::UnoType<OUString>::get()  , PROPERTY_NONE,     0},\
        { OUString(UNO_NAME_NAME),  WID_IDX_NAME,   cppu::UnoType<OUString>::get()  , PROPERTY_NONE,     0},\
        { OUString(UNO_NAME_CONTENT_SECTION), WID_IDX_CONTENT_SECTION,  cppu::UnoType<css::text::XTextSection>::get()  , PropertyAttribute::READONLY,     0},\
        { OUString(UNO_NAME_HEADER_SECTION), WID_IDX_HEADER_SECTION,  cppu::UnoType<css::text::XTextSection>::get()  , PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY,     0},\

#define ANCHOR_TYPES_PROPERTY    { OUString(UNO_NAME_ANCHOR_TYPES), FN_UNO_ANCHOR_TYPES,    cppu::UnoType< cppu::UnoSequenceType<css::text::TextContentAnchorType> >::get(),PropertyAttribute::READONLY, 0xff},

// #i18732# #i28701# #i73249#
//UUUU all users of COMMON_FRAME_PROPERTIES add the new XATTR_FILL_FIRST, XATTR_FILL_LAST FillStyle,
// thus it may be possible to remove the RES_BACKGROUND entries from SvxBrushItem completely (this includes
// all using UNO_NAME_BACK_* slots) in the future
#define COMMON_FRAME_PROPERTIES \
    { OUString(UNO_NAME_ANCHOR_PAGE_NO), RES_ANCHOR,            cppu::UnoType<sal_Int16>::get(),           PROPERTY_NONE, MID_ANCHOR_PAGENUM       },              \
    { OUString(UNO_NAME_ANCHOR_TYPE), RES_ANCHOR,           cppu::UnoType<css::text::TextContentAnchorType>::get(),            PROPERTY_NONE, MID_ANCHOR_ANCHORTYPE},             \
    { OUString(UNO_NAME_ANCHOR_FRAME), RES_ANCHOR,             cppu::UnoType<css::text::XTextFrame>::get(),    PropertyAttribute::MAYBEVOID, MID_ANCHOR_ANCHORFRAME},             \
    ANCHOR_TYPES_PROPERTY\
    { OUString(UNO_NAME_BACK_COLOR), RES_BACKGROUND,            cppu::UnoType<sal_Int32>::get(),           PROPERTY_NONE ,MID_BACK_COLOR        },                      \
    { OUString(UNO_NAME_BACK_COLOR_R_G_B), RES_BACKGROUND,      cppu::UnoType<sal_Int32>::get(), PROPERTY_NONE ,MID_BACK_COLOR_R_G_B},    \
    { OUString(UNO_NAME_BACK_COLOR_TRANSPARENCY), RES_BACKGROUND,      cppu::UnoType<sal_Int8>::get(), PROPERTY_NONE ,MID_BACK_COLOR_TRANSPARENCY},    \
    { OUString(UNO_NAME_FRAME_INTEROP_GRAB_BAG), RES_FRMATR_GRABBAG, cppu::UnoType< cppu::UnoSequenceType<css::beans::PropertyValue> >::get(), PROPERTY_NONE, 0}, \
    { OUString(UNO_NAME_CONTENT_PROTECTED), RES_PROTECT,            cppu::UnoType<bool>::get(),             PROPERTY_NONE, MID_PROTECT_CONTENT  },                          \
    { OUString(UNO_NAME_FRAME_STYLE_NAME), FN_UNO_FRAME_STYLE_NAME,cppu::UnoType<OUString>::get(),         PROPERTY_NONE, 0},                                   \
    { OUString(UNO_NAME_BACK_GRAPHIC_URL), RES_BACKGROUND,      cppu::UnoType<OUString>::get(),        PROPERTY_NONE ,MID_GRAPHIC_URL    },                 \
    { OUString(UNO_NAME_BACK_GRAPHIC_FILTER), RES_BACKGROUND,       cppu::UnoType<OUString>::get(),        PROPERTY_NONE ,MID_GRAPHIC_FILTER    },              \
    { OUString(UNO_NAME_BACK_GRAPHIC_LOCATION), RES_BACKGROUND,         cppu::UnoType<css::style::GraphicLocation>::get(), PROPERTY_NONE ,MID_GRAPHIC_POSITION}, \
    { OUString(UNO_NAME_BACK_GRAPHIC_TRANSPARENCY), RES_BACKGROUND,      cppu::UnoType<sal_Int8>::get(), PROPERTY_NONE ,MID_GRAPHIC_TRANSPARENCY},    \
    { OUString(UNO_NAME_LEFT_MARGIN), RES_LR_SPACE,             cppu::UnoType<sal_Int32>::get(),           PROPERTY_NONE, MID_L_MARGIN|CONVERT_TWIPS},            \
    { OUString(UNO_NAME_RIGHT_MARGIN), RES_LR_SPACE,            cppu::UnoType<sal_Int32>::get(),           PROPERTY_NONE, MID_R_MARGIN|CONVERT_TWIPS},            \
    { OUString(UNO_NAME_WIDTH), RES_FRM_SIZE,           cppu::UnoType<sal_Int32>::get()  ,         PROPERTY_NONE, MID_FRMSIZE_WIDTH|CONVERT_TWIPS},\
    { OUString(UNO_NAME_HEIGHT), RES_FRM_SIZE,          cppu::UnoType<sal_Int32>::get()  ,         PROPERTY_NONE, MID_FRMSIZE_HEIGHT|CONVERT_TWIPS},\
    { OUString(UNO_NAME_HORI_ORIENT), RES_HORI_ORIENT,      cppu::UnoType<sal_Int16>::get(),           PROPERTY_NONE ,MID_HORIORIENT_ORIENT    },             \
    { OUString(UNO_NAME_HORI_ORIENT_POSITION), RES_HORI_ORIENT,     cppu::UnoType<sal_Int32>::get(),           PROPERTY_NONE ,MID_HORIORIENT_POSITION|CONVERT_TWIPS    }, \
    { OUString(UNO_NAME_HORI_ORIENT_RELATION), RES_HORI_ORIENT,     cppu::UnoType<sal_Int16>::get(),           PROPERTY_NONE ,MID_HORIORIENT_RELATION  },               \
    { OUString(UNO_NAME_HYPER_LINK_U_R_L), RES_URL,                 cppu::UnoType<OUString>::get(),        PROPERTY_NONE ,MID_URL_URL},                         \
    { OUString(UNO_NAME_HYPER_LINK_TARGET), RES_URL,                cppu::UnoType<OUString>::get(),        PROPERTY_NONE ,MID_URL_TARGET},                      \
    { OUString(UNO_NAME_HYPER_LINK_NAME), RES_URL,              cppu::UnoType<OUString>::get(),            PROPERTY_NONE ,MID_URL_HYPERLINKNAME     },                 \
    { OUString(UNO_NAME_OPAQUE), RES_OPAQUE,            cppu::UnoType<bool>::get(),         PROPERTY_NONE, 0},                                        \
    { OUString(UNO_NAME_PAGE_TOGGLE), RES_HORI_ORIENT,      cppu::UnoType<bool>::get(),         PROPERTY_NONE ,MID_HORIORIENT_PAGETOGGLE },               \
    { OUString(UNO_NAME_POSITION_PROTECTED), RES_PROTECT,           cppu::UnoType<bool>::get(),             PROPERTY_NONE, MID_PROTECT_POSITION},                      \
    { OUString(UNO_NAME_PRINT), RES_PRINT,              cppu::UnoType<bool>::get(),         PROPERTY_NONE, 0},                                        \
    { OUString(UNO_NAME_RELATIVE_HEIGHT), RES_FRM_SIZE,         cppu::UnoType<sal_Int16>::get()  ,         PROPERTY_NONE,   MID_FRMSIZE_REL_HEIGHT },            \
    { OUString(UNO_NAME_RELATIVE_HEIGHT_RELATION), RES_FRM_SIZE, cppu::UnoType<sal_Int16>::get()  ,        PROPERTY_NONE,   MID_FRMSIZE_REL_HEIGHT_RELATION  },         \
    { OUString(UNO_NAME_RELATIVE_WIDTH), RES_FRM_SIZE,          cppu::UnoType<sal_Int16>::get()  ,         PROPERTY_NONE,   MID_FRMSIZE_REL_WIDTH  },         \
    { OUString(UNO_NAME_RELATIVE_WIDTH_RELATION), RES_FRM_SIZE, cppu::UnoType<sal_Int16>::get()  ,         PROPERTY_NONE,   MID_FRMSIZE_REL_WIDTH_RELATION  },         \
    { OUString(UNO_NAME_SHADOW_FORMAT), RES_SHADOW,             cppu::UnoType<css::table::ShadowFormat>::get(),   PROPERTY_NONE, CONVERT_TWIPS},             \
    { OUString(UNO_NAME_SHADOW_TRANSPARENCE), RES_SHADOW,       cppu::UnoType<sal_Int16>::get(),       PROPERTY_NONE, MID_SHADOW_TRANSPARENCE},             \
    { OUString(UNO_NAME_IMAGE_MAP), RES_URL,                    cppu::UnoType<css::container::XIndexContainer>::get(), PROPERTY_NONE, MID_URL_CLIENTMAP}, \
    { OUString(UNO_NAME_SERVER_MAP), RES_URL,                cppu::UnoType<bool>::get(),            PROPERTY_NONE ,MID_URL_SERVERMAP   },                      \
    { OUString(UNO_NAME_SIZE), RES_FRM_SIZE,            cppu::UnoType<css::awt::Size>::get(),             PROPERTY_NONE, MID_FRMSIZE_SIZE|CONVERT_TWIPS},        \
    { OUString(UNO_NAME_SIZE_PROTECTED), RES_PROTECT,           cppu::UnoType<bool>::get(),             PROPERTY_NONE, MID_PROTECT_SIZE    },                      \
    { OUString(UNO_NAME_IS_SYNC_WIDTH_TO_HEIGHT), RES_FRM_SIZE,         cppu::UnoType<bool>::get()  ,       PROPERTY_NONE,   MID_FRMSIZE_IS_SYNC_WIDTH_TO_HEIGHT    },  \
    { OUString(UNO_NAME_IS_SYNC_HEIGHT_TO_WIDTH), RES_FRM_SIZE,         cppu::UnoType<bool>::get()  ,       PROPERTY_NONE,   MID_FRMSIZE_IS_SYNC_HEIGHT_TO_WIDTH },     \
    { OUString(UNO_NAME_TEXT_WRAP), RES_SURROUND,           cppu::UnoType<css::text::WrapTextMode>::get(),             PROPERTY_NONE, MID_SURROUND_SURROUNDTYPE    },        \
    { OUString(UNO_NAME_SURROUND), RES_SURROUND,          cppu::UnoType<css::text::WrapTextMode>::get(),            PROPERTY_NONE, MID_SURROUND_SURROUNDTYPE },        \
    { OUString(UNO_NAME_SURROUND_ANCHORONLY), RES_SURROUND,             cppu::UnoType<bool>::get(),             PROPERTY_NONE, MID_SURROUND_ANCHORONLY      },                 \
    { OUString(UNO_NAME_TOP_MARGIN), RES_UL_SPACE,          cppu::UnoType<sal_Int32>::get(), PROPERTY_NONE, MID_UP_MARGIN|CONVERT_TWIPS},          \
    { OUString(UNO_NAME_BOTTOM_MARGIN), RES_UL_SPACE,           cppu::UnoType<sal_Int32>::get(), PROPERTY_NONE, MID_LO_MARGIN|CONVERT_TWIPS},          \
    { OUString(UNO_NAME_BACK_TRANSPARENT), RES_BACKGROUND,      cppu::UnoType<bool>::get(),         PROPERTY_NONE ,MID_GRAPHIC_TRANSPARENT       },                    \
    { OUString(UNO_NAME_VERT_ORIENT), RES_VERT_ORIENT,      cppu::UnoType<sal_Int16>::get(),           PROPERTY_NONE ,MID_VERTORIENT_ORIENT    },             \
    { OUString(UNO_NAME_VERT_ORIENT_POSITION), RES_VERT_ORIENT,     cppu::UnoType<sal_Int32>::get(),           PROPERTY_NONE ,MID_VERTORIENT_POSITION|CONVERT_TWIPS    }, \
    { OUString(UNO_NAME_VERT_ORIENT_RELATION), RES_VERT_ORIENT,     cppu::UnoType<sal_Int16>::get(),           PROPERTY_NONE ,MID_VERTORIENT_RELATION  },               \
    { OUString(UNO_NAME_LEFT_BORDER), RES_BOX,              cppu::UnoType<css::table::BorderLine>::get(),  0, LEFT_BORDER  |CONVERT_TWIPS },             \
    { OUString(UNO_NAME_RIGHT_BORDER), RES_BOX,             cppu::UnoType<css::table::BorderLine>::get(),  0, RIGHT_BORDER |CONVERT_TWIPS },                 \
    { OUString(UNO_NAME_TOP_BORDER), RES_BOX,               cppu::UnoType<css::table::BorderLine>::get(),  0, TOP_BORDER   |CONVERT_TWIPS },             \
    { OUString(UNO_NAME_BOTTOM_BORDER), RES_BOX,                cppu::UnoType<css::table::BorderLine>::get(),  0, BOTTOM_BORDER|CONVERT_TWIPS },                 \
    { OUString(UNO_NAME_BORDER_DISTANCE), RES_BOX,              cppu::UnoType<sal_Int32>::get(),   0, BORDER_DISTANCE|CONVERT_TWIPS },                       \
    { OUString(UNO_NAME_LEFT_BORDER_DISTANCE), RES_BOX,             cppu::UnoType<sal_Int32>::get(),   0, LEFT_BORDER_DISTANCE  |CONVERT_TWIPS },                \
    { OUString(UNO_NAME_RIGHT_BORDER_DISTANCE), RES_BOX,                cppu::UnoType<sal_Int32>::get(),   0, RIGHT_BORDER_DISTANCE |CONVERT_TWIPS },                \
    { OUString(UNO_NAME_TOP_BORDER_DISTANCE), RES_BOX,              cppu::UnoType<sal_Int32>::get(),   0, TOP_BORDER_DISTANCE   |CONVERT_TWIPS },            \
    { OUString(UNO_NAME_BOTTOM_BORDER_DISTANCE), RES_BOX,               cppu::UnoType<sal_Int32>::get(),   0, BOTTOM_BORDER_DISTANCE|CONVERT_TWIPS },            \
    { OUString(UNO_LINK_DISPLAY_NAME), FN_PARAM_LINK_DISPLAY_NAME,  cppu::UnoType<OUString>::get(), PropertyAttribute::READONLY, 0xff}, \
    { OUString(UNO_NAME_USER_DEFINED_ATTRIBUTES), RES_UNKNOWNATR_CONTAINER, cppu::UnoType<css::container::XNameContainer>::get(), PropertyAttribute::MAYBEVOID, 0 },\
    { OUString(UNO_NAME_Z_ORDER), FN_UNO_Z_ORDER,           cppu::UnoType<sal_Int32>::get(),       PROPERTY_NONE, 0}, \
    { OUString(UNO_NAME_IS_FOLLOWING_TEXT_FLOW), RES_FOLLOW_TEXT_FLOW,     cppu::UnoType<bool>::get(), PROPERTY_NONE, 0}, \
    { OUString(UNO_NAME_WRAP_INFLUENCE_ON_POSITION), RES_WRAP_INFLUENCE_ON_OBJPOS, cppu::UnoType<sal_Int8>::get(), PROPERTY_NONE, MID_WRAP_INFLUENCE}, \
    { OUString(UNO_NAME_TITLE), FN_UNO_TITLE, cppu::UnoType<OUString>::get(), PROPERTY_NONE, 0}, \
    { OUString(UNO_NAME_DESCRIPTION), FN_UNO_DESCRIPTION, cppu::UnoType<OUString>::get(), PROPERTY_NONE, 0}, \
    { OUString(UNO_NAME_LAYOUT_SIZE), WID_LAYOUT_SIZE, cppu::UnoType<css::awt::Size>::get(), PropertyAttribute::MAYBEVOID | PropertyAttribute::READONLY, 0 }, \
    { OUString(UNO_NAME_LINE_STYLE), RES_BOX, cppu::UnoType<css::drawing::LineStyle>::get(),  0, LINE_STYLE }, \
    { OUString(UNO_NAME_LINE_WIDTH), RES_BOX, cppu::UnoType<sal_Int32>::get(),  0, LINE_WIDTH |CONVERT_TWIPS }, \
    { OUString(UNO_NAME_TEXT_VERT_ADJUST), RES_TEXT_VERT_ADJUST, cppu::UnoType<css::drawing::TextVerticalAdjust>::get(), PROPERTY_NONE ,0},

#define COMMON_TEXT_CONTENT_PROPERTIES \
        { OUString(UNO_NAME_ANCHOR_TYPE), FN_UNO_ANCHOR_TYPE, cppu::UnoType<css::text::TextContentAnchorType>::get(),              PropertyAttribute::READONLY, MID_ANCHOR_ANCHORTYPE},\
        ANCHOR_TYPES_PROPERTY\
        { OUString(UNO_NAME_TEXT_WRAP), FN_UNO_TEXT_WRAP,   cppu::UnoType<css::text::WrapTextMode>::get(),                 PropertyAttribute::READONLY, MID_SURROUND_SURROUNDTYPE  },

#define     _PROP_DIFF_FONTHEIGHT \
                    { OUString(UNO_NAME_CHAR_PROP_HEIGHT), RES_CHRATR_FONTSIZE ,            cppu::UnoType<float>::get(),                                           PROPERTY_NONE , MID_FONTHEIGHT_PROP},\
                    { OUString(UNO_NAME_CHAR_DIFF_HEIGHT), RES_CHRATR_FONTSIZE ,            cppu::UnoType<sal_Int16>::get(),    PROPERTY_NONE , MID_FONTHEIGHT_DIFF|CONVERT_TWIPS},\
                    { OUString(UNO_NAME_CHAR_PROP_HEIGHT_ASIAN), RES_CHRATR_CJK_FONTSIZE ,          cppu::UnoType<float>::get(),                                           PROPERTY_NONE , MID_FONTHEIGHT_PROP},\
                    { OUString(UNO_NAME_CHAR_DIFF_HEIGHT_ASIAN), RES_CHRATR_CJK_FONTSIZE ,          cppu::UnoType<sal_Int16>::get(),    PROPERTY_NONE , MID_FONTHEIGHT_DIFF|CONVERT_TWIPS},\
                    { OUString(UNO_NAME_CHAR_PROP_HEIGHT_COMPLEX), RES_CHRATR_CTL_FONTSIZE ,            cppu::UnoType<float>::get(),                                           PROPERTY_NONE , MID_FONTHEIGHT_PROP},\
                    { OUString(UNO_NAME_CHAR_DIFF_HEIGHT_COMPLEX), RES_CHRATR_CTL_FONTSIZE ,            cppu::UnoType<sal_Int16>::get(),    PROPERTY_NONE , MID_FONTHEIGHT_DIFF|CONVERT_TWIPS},

#define COMMON_PARA_STYLE_PROPERTIES \
                    { OUString(UNO_NAME_BREAK_TYPE), RES_BREAK,                 cppu::UnoType<css::style::BreakType>::get(),       PROPERTY_NONE, 0},\
                    { OUString(UNO_NAME_PAGE_DESC_NAME), RES_PAGEDESC,          cppu::UnoType<OUString>::get(),            PropertyAttribute::MAYBEVOID, MID_PAGEDESC_PAGEDESCNAME },\
                    { OUString(UNO_NAME_PAGE_NUMBER_OFFSET), RES_PAGEDESC,              cppu::UnoType<sal_Int16>::get(),       PropertyAttribute::MAYBEVOID, MID_PAGEDESC_PAGENUMOFFSET},\
                    { OUString(UNO_NAME_CHAR_AUTO_KERNING), RES_CHRATR_AUTOKERN  ,  cppu::UnoType<bool>::get()  ,       PROPERTY_NONE,     0},\
                    { OUString(UNO_NAME_CHAR_BACK_TRANSPARENT), RES_CHRATR_BACKGROUND, cppu::UnoType<bool>::get(),          PROPERTY_NONE ,MID_GRAPHIC_TRANSPARENT       },\
                    { OUString(UNO_NAME_CHAR_BACK_COLOR), RES_CHRATR_BACKGROUND,    cppu::UnoType<sal_Int32>::get(),           PROPERTY_NONE ,MID_BACK_COLOR        },\
                    { OUString(UNO_NAME_CHAR_HIGHLIGHT), RES_CHRATR_HIGHLIGHT, cppu::UnoType<sal_Int32>::get(), PROPERTY_NONE ,MID_BACK_COLOR },\
                    { OUString(UNO_NAME_PARA_BACK_COLOR), RES_BACKGROUND,       cppu::UnoType<sal_Int32>::get(),           PROPERTY_NONE ,MID_BACK_COLOR        },\
                    { OUString(UNO_NAME_PARA_BACK_TRANSPARENT), RES_BACKGROUND,         cppu::UnoType<bool>::get(),         PROPERTY_NONE ,MID_GRAPHIC_TRANSPARENT       },\
                    { OUString(UNO_NAME_PARA_GRAPHIC_URL), RES_BACKGROUND,      cppu::UnoType<OUString>::get(), PROPERTY_NONE ,MID_GRAPHIC_URL    },\
                    { OUString(UNO_NAME_PARA_GRAPHIC_FILTER), RES_BACKGROUND,       cppu::UnoType<OUString>::get(), PROPERTY_NONE ,MID_GRAPHIC_FILTER    },\
                    { OUString(UNO_NAME_PARA_GRAPHIC_LOCATION), RES_BACKGROUND,         cppu::UnoType<css::style::GraphicLocation>::get(), PROPERTY_NONE ,MID_GRAPHIC_POSITION}, \
                    { OUString(UNO_NAME_CHAR_CASE_MAP), RES_CHRATR_CASEMAP,     cppu::UnoType<sal_Int16>::get(),           PROPERTY_NONE, 0},\
                    { OUString(UNO_NAME_CHAR_COLOR), RES_CHRATR_COLOR,      cppu::UnoType<sal_Int32>::get(),           PROPERTY_NONE, 0},\
                    { OUString(UNO_NAME_CHAR_STRIKEOUT), RES_CHRATR_CROSSEDOUT,  cppu::UnoType<sal_Int16>::get(),                  PropertyAttribute::MAYBEVOID, MID_CROSS_OUT},\
                    { OUString(UNO_NAME_CHAR_CROSSED_OUT), RES_CHRATR_CROSSEDOUT,  cppu::UnoType<bool>::get()  ,        PROPERTY_NONE, 0},\
                    { OUString(UNO_NAME_CHAR_ESCAPEMENT), RES_CHRATR_ESCAPEMENT,  cppu::UnoType<sal_Int16>::get(),             PROPERTY_NONE, MID_ESC          },\
                    { OUString(UNO_NAME_CHAR_ESCAPEMENT_HEIGHT), RES_CHRATR_ESCAPEMENT,     cppu::UnoType<sal_Int8>::get()  ,          PROPERTY_NONE, MID_ESC_HEIGHT},\
                    { OUString(UNO_NAME_CHAR_FLASH), RES_CHRATR_BLINK   ,   cppu::UnoType<bool>::get()  ,       PROPERTY_NONE,     0},\
                    { OUString(UNO_NAME_CHAR_HIDDEN), RES_CHRATR_HIDDEN, cppu::UnoType<bool>::get()  ,       PROPERTY_NONE,     0},\
                    _STANDARD_FONT_PROPERTIES\
                    _CJK_FONT_PROPERTIES\
                    _CTL_FONT_PROPERTIES\
                    { OUString(UNO_NAME_CHAR_UNDERLINE), RES_CHRATR_UNDERLINE ,  cppu::UnoType<sal_Int16>::get(),      PROPERTY_NONE, MID_TL_STYLE},\
                    { OUString(UNO_NAME_CHAR_UNDERLINE_COLOR), RES_CHRATR_UNDERLINE ,  cppu::UnoType<sal_Int32>::get(),            PROPERTY_NONE, MID_TL_COLOR},\
                    { OUString(UNO_NAME_CHAR_UNDERLINE_HAS_COLOR), RES_CHRATR_UNDERLINE ,  cppu::UnoType<bool>::get(),              PROPERTY_NONE, MID_TL_HASCOLOR},\
                    { OUString(UNO_NAME_CHAR_OVERLINE), RES_CHRATR_OVERLINE ,  cppu::UnoType<sal_Int16>::get(),      PROPERTY_NONE, MID_TL_STYLE},\
                    { OUString(UNO_NAME_CHAR_OVERLINE_COLOR), RES_CHRATR_OVERLINE ,  cppu::UnoType<sal_Int32>::get(),            PROPERTY_NONE, MID_TL_COLOR},\
                    { OUString(UNO_NAME_CHAR_OVERLINE_HAS_COLOR), RES_CHRATR_OVERLINE ,  cppu::UnoType<bool>::get(),              PROPERTY_NONE, MID_TL_HASCOLOR},\
                    { OUString(UNO_NAME_PARA_LEFT_MARGIN), RES_LR_SPACE,            cppu::UnoType<sal_Int32>::get(), PROPERTY_NONE, MID_TXT_LMARGIN|CONVERT_TWIPS},\
                    { OUString(UNO_NAME_PARA_RIGHT_MARGIN), RES_LR_SPACE,           cppu::UnoType<sal_Int32>::get(), PROPERTY_NONE, MID_R_MARGIN|CONVERT_TWIPS},\
                    { OUString(UNO_NAME_PARA_LEFT_MARGIN_RELATIVE), RES_LR_SPACE,          cppu::UnoType<sal_Int16>::get(), PROPERTY_NONE,        MID_L_REL_MARGIN},\
                    { OUString(UNO_NAME_PARA_RIGHT_MARGIN_RELATIVE), RES_LR_SPACE,         cppu::UnoType<sal_Int16>::get(), PROPERTY_NONE,        MID_R_REL_MARGIN},\
                    { OUString(UNO_NAME_PARA_IS_AUTO_FIRST_LINE_INDENT), RES_LR_SPACE,          cppu::UnoType<bool>::get(),      PROPERTY_NONE, MID_FIRST_AUTO},\
                    { OUString(UNO_NAME_PARA_FIRST_LINE_INDENT), RES_LR_SPACE,           cppu::UnoType<sal_Int32>::get(), PROPERTY_NONE, MID_FIRST_LINE_INDENT|CONVERT_TWIPS},\
                    { OUString(UNO_NAME_PARA_FIRST_LINE_INDENT_RELATIVE), RES_LR_SPACE,         cppu::UnoType<sal_Int32>::get(), PROPERTY_NONE, MID_FIRST_LINE_REL_INDENT|CONVERT_TWIPS},\
                    { OUString(UNO_NAME_CHAR_KERNING), RES_CHRATR_KERNING    ,  cppu::UnoType<sal_Int16>::get()  ,         PROPERTY_NONE,  CONVERT_TWIPS},\
                    { OUString(UNO_NAME_CHAR_NO_HYPHENATION), RES_CHRATR_NOHYPHEN   ,   cppu::UnoType<bool>::get()  ,       PROPERTY_NONE,     0},\
                    { OUString(UNO_NAME_CHAR_SHADOWED), RES_CHRATR_SHADOWED  ,  cppu::UnoType<bool>::get()  ,       PROPERTY_NONE, 0},\
                    { OUString(UNO_NAME_CHAR_CONTOURED), RES_CHRATR_CONTOUR,    cppu::UnoType<bool>::get()  ,       PROPERTY_NONE, 0},\
                    { OUString(UNO_NAME_DROP_CAP_FORMAT), RES_PARATR_DROP,        cppu::UnoType<css::style::DropCapFormat>::get()  , PROPERTY_NONE, MID_DROPCAP_FORMAT|CONVERT_TWIPS     },\
                    { OUString(UNO_NAME_DROP_CAP_WHOLE_WORD), RES_PARATR_DROP,        cppu::UnoType<bool>::get()  ,         PROPERTY_NONE, MID_DROPCAP_WHOLE_WORD },\
                    { OUString(UNO_NAME_DROP_CAP_CHAR_STYLE_NAME), RES_PARATR_DROP,        cppu::UnoType<OUString>::get()  ,       PropertyAttribute::MAYBEVOID, MID_DROPCAP_CHAR_STYLE_NAME },\
                    { OUString(UNO_NAME_PARA_KEEP_TOGETHER), RES_KEEP,              cppu::UnoType<bool>::get()  ,       PROPERTY_NONE,     0},\
                    { OUString(UNO_NAME_PARA_SPLIT), RES_PARATR_SPLIT,      cppu::UnoType<bool>::get()  ,       PROPERTY_NONE,     0},\
                    { OUString(UNO_NAME_PARA_WIDOWS), RES_PARATR_WIDOWS,        cppu::UnoType<sal_Int8>::get(),PropertyAttribute::MAYBEVOID,     0},\
                    { OUString(UNO_NAME_PARA_ORPHANS), RES_PARATR_ORPHANS,      cppu::UnoType<sal_Int8>::get(),PropertyAttribute::MAYBEVOID,     0},\
                    { OUString(UNO_NAME_PARA_EXPAND_SINGLE_WORD), RES_PARATR_ADJUST,      cppu::UnoType<bool>::get()  ,         PROPERTY_NONE, MID_EXPAND_SINGLE   },\
                    { OUString(UNO_NAME_PARA_LAST_LINE_ADJUST), RES_PARATR_ADJUST,      cppu::UnoType<sal_Int16>::get(),       PROPERTY_NONE, MID_LAST_LINE_ADJUST},\
                    { OUString(UNO_NAME_PARA_LINE_NUMBER_COUNT), RES_LINENUMBER,        cppu::UnoType<bool>::get(),         PROPERTY_NONE ,MID_LINENUMBER_COUNT     },\
                    { OUString(UNO_NAME_PARA_LINE_NUMBER_START_VALUE), RES_LINENUMBER,      cppu::UnoType<sal_Int32>::get(),           PROPERTY_NONE ,MID_LINENUMBER_STARTVALUE},\
                    { OUString(UNO_NAME_PARA_LINE_SPACING), RES_PARATR_LINESPACING, cppu::UnoType<css::style::LineSpacing>::get(),PROPERTY_NONE,     CONVERT_TWIPS},\
                    { OUString(UNO_NAME_PARA_ADJUST), RES_PARATR_ADJUST,      cppu::UnoType<sal_Int16>::get(),         PROPERTY_NONE, MID_PARA_ADJUST},\
                    { OUString(UNO_NAME_PARA_REGISTER_MODE_ACTIVE), RES_PARATR_REGISTER,    cppu::UnoType<bool>::get()  ,       PROPERTY_NONE, 0},\
                    { OUString(UNO_NAME_PARA_TOP_MARGIN), RES_UL_SPACE,             cppu::UnoType<sal_Int32>::get(), PROPERTY_NONE, MID_UP_MARGIN|CONVERT_TWIPS},\
                    { OUString(UNO_NAME_PARA_BOTTOM_MARGIN), RES_UL_SPACE,          cppu::UnoType<sal_Int32>::get(), PROPERTY_NONE, MID_LO_MARGIN|CONVERT_TWIPS},\
                    { OUString(UNO_NAME_PARA_CONTEXT_MARGIN), RES_UL_SPACE,          cppu::UnoType<bool>::get(), PROPERTY_NONE, MID_CTX_MARGIN},\
                    { OUString(UNO_NAME_PARA_TOP_MARGIN_RELATIVE), RES_UL_SPACE,        cppu::UnoType<sal_Int16>::get(), PROPERTY_NONE, MID_UP_REL_MARGIN},\
                    { OUString(UNO_NAME_PARA_BOTTOM_MARGIN_RELATIVE), RES_UL_SPACE,         cppu::UnoType<sal_Int16>::get(), PROPERTY_NONE, MID_LO_REL_MARGIN},\
                    TABSTOPS_MAP_ENTRY\
                    { OUString(UNO_NAME_CHAR_WORD_MODE), RES_CHRATR_WORDLINEMODE,cppu::UnoType<bool>::get()  ,    PROPERTY_NONE,     0},\
                    { OUString(UNO_NAME_CHAR_LEFT_BORDER), RES_CHRATR_BOX, cppu::UnoType<css::table::BorderLine>::get(), PROPERTY_NONE, LEFT_BORDER |CONVERT_TWIPS },\
                    { OUString(UNO_NAME_CHAR_RIGHT_BORDER), RES_CHRATR_BOX, cppu::UnoType<css::table::BorderLine>::get(), PROPERTY_NONE, RIGHT_BORDER |CONVERT_TWIPS },\
                    { OUString(UNO_NAME_CHAR_TOP_BORDER), RES_CHRATR_BOX, cppu::UnoType<css::table::BorderLine>::get(), PROPERTY_NONE, TOP_BORDER |CONVERT_TWIPS },\
                    { OUString(UNO_NAME_CHAR_BOTTOM_BORDER), RES_CHRATR_BOX, cppu::UnoType<css::table::BorderLine>::get(), PROPERTY_NONE, BOTTOM_BORDER |CONVERT_TWIPS },\
                    { OUString(UNO_NAME_CHAR_BORDER_DISTANCE), RES_CHRATR_BOX, cppu::UnoType<sal_Int32>::get(), PROPERTY_NONE, BORDER_DISTANCE |CONVERT_TWIPS },\
                    { OUString(UNO_NAME_CHAR_LEFT_BORDER_DISTANCE), RES_CHRATR_BOX, cppu::UnoType<sal_Int32>::get(), PROPERTY_NONE, LEFT_BORDER_DISTANCE |CONVERT_TWIPS },\
                    { OUString(UNO_NAME_CHAR_RIGHT_BORDER_DISTANCE), RES_CHRATR_BOX, cppu::UnoType<sal_Int32>::get(),PROPERTY_NONE, RIGHT_BORDER_DISTANCE |CONVERT_TWIPS },\
                    { OUString(UNO_NAME_CHAR_TOP_BORDER_DISTANCE), RES_CHRATR_BOX, cppu::UnoType<sal_Int32>::get(), PROPERTY_NONE, TOP_BORDER_DISTANCE |CONVERT_TWIPS },\
                    { OUString(UNO_NAME_CHAR_BOTTOM_BORDER_DISTANCE), RES_CHRATR_BOX, cppu::UnoType<sal_Int32>::get(), PROPERTY_NONE, BOTTOM_BORDER_DISTANCE|CONVERT_TWIPS },\
                    { OUString(UNO_NAME_CHAR_SHADOW_FORMAT), RES_CHRATR_SHADOW, cppu::UnoType<css::table::ShadowFormat>::get(), PROPERTY_NONE, CONVERT_TWIPS},\
                    { OUString(UNO_NAME_LEFT_BORDER), RES_BOX,              cppu::UnoType<css::table::BorderLine>::get(),  0, LEFT_BORDER  |CONVERT_TWIPS },\
                    { OUString(UNO_NAME_RIGHT_BORDER), RES_BOX,             cppu::UnoType<css::table::BorderLine>::get(),  0, RIGHT_BORDER |CONVERT_TWIPS },\
                    { OUString(UNO_NAME_TOP_BORDER), RES_BOX,               cppu::UnoType<css::table::BorderLine>::get(),  0, TOP_BORDER   |CONVERT_TWIPS },\
                    { OUString(UNO_NAME_BOTTOM_BORDER), RES_BOX,                cppu::UnoType<css::table::BorderLine>::get(),  0, BOTTOM_BORDER|CONVERT_TWIPS },\
                    { OUString(UNO_NAME_BORDER_DISTANCE), RES_BOX,              cppu::UnoType<sal_Int32>::get(),   0, BORDER_DISTANCE|CONVERT_TWIPS },\
                    { OUString(UNO_NAME_LEFT_BORDER_DISTANCE), RES_BOX,             cppu::UnoType<sal_Int32>::get(),   0, LEFT_BORDER_DISTANCE  |CONVERT_TWIPS },\
                    { OUString(UNO_NAME_RIGHT_BORDER_DISTANCE), RES_BOX,                cppu::UnoType<sal_Int32>::get(),   0, RIGHT_BORDER_DISTANCE |CONVERT_TWIPS },\
                    { OUString(UNO_NAME_TOP_BORDER_DISTANCE), RES_BOX,              cppu::UnoType<sal_Int32>::get(),   0, TOP_BORDER_DISTANCE   |CONVERT_TWIPS },\
                    { OUString(UNO_NAME_BOTTOM_BORDER_DISTANCE), RES_BOX,               cppu::UnoType<sal_Int32>::get(),   0, BOTTOM_BORDER_DISTANCE|CONVERT_TWIPS },\
                    { OUString(UNO_NAME_PARA_IS_HYPHENATION), RES_PARATR_HYPHENZONE,      cppu::UnoType<bool>::get(),  PropertyAttribute::MAYBEVOID, MID_IS_HYPHEN         },\
                    { OUString(UNO_NAME_PARA_HYPHENATION_MAX_LEADING_CHARS), RES_PARATR_HYPHENZONE,         cppu::UnoType<sal_Int16>::get(),   PropertyAttribute::MAYBEVOID, MID_HYPHEN_MIN_LEAD   },\
                    { OUString(UNO_NAME_PARA_HYPHENATION_MAX_TRAILING_CHARS), RES_PARATR_HYPHENZONE,        cppu::UnoType<sal_Int16>::get(),   PropertyAttribute::MAYBEVOID, MID_HYPHEN_MIN_TRAIL  },\
                    { OUString(UNO_NAME_PARA_HYPHENATION_MAX_HYPHENS), RES_PARATR_HYPHENZONE,       cppu::UnoType<sal_Int16>::get(),   PropertyAttribute::MAYBEVOID, MID_HYPHEN_MAX_HYPHENS},\
                    { OUString(UNO_NAME_NUMBERING_STYLE_NAME), RES_PARATR_NUMRULE,  cppu::UnoType<OUString>::get(),         PropertyAttribute::MAYBEVOID,   0},\
                    { OUString(UNO_NAME_PARA_USER_DEFINED_ATTRIBUTES), RES_UNKNOWNATR_CONTAINER, cppu::UnoType<css::container::XNameContainer>::get(), PropertyAttribute::MAYBEVOID, 0 },\
                    { OUString(UNO_NAME_PARA_SHADOW_FORMAT), RES_SHADOW,    cppu::UnoType<css::table::ShadowFormat>::get(),   PROPERTY_NONE, CONVERT_TWIPS},\
                    { OUString(UNO_NAME_CHAR_COMBINE_IS_ON), RES_CHRATR_TWO_LINES,          cppu::UnoType<bool>::get(),     PROPERTY_NONE, MID_TWOLINES},\
                    { OUString(UNO_NAME_CHAR_COMBINE_PREFIX), RES_CHRATR_TWO_LINES,             cppu::UnoType<OUString>::get(),    PROPERTY_NONE, MID_START_BRACKET},\
                    { OUString(UNO_NAME_CHAR_COMBINE_SUFFIX), RES_CHRATR_TWO_LINES,             cppu::UnoType<OUString>::get(),    PROPERTY_NONE, MID_END_BRACKET},\
                    { OUString(UNO_NAME_CHAR_EMPHASIS), RES_CHRATR_EMPHASIS_MARK,           cppu::UnoType<sal_Int16>::get(),   PROPERTY_NONE, MID_EMPHASIS},\
                    { OUString(UNO_NAME_PARA_IS_HANGING_PUNCTUATION), RES_PARATR_HANGINGPUNCTUATION,  cppu::UnoType<bool>::get(),   PROPERTY_NONE ,0     },\
                    { OUString(UNO_NAME_PARA_IS_CHARACTER_DISTANCE), RES_PARATR_SCRIPTSPACE,         cppu::UnoType<bool>::get(),    PROPERTY_NONE ,0     },\
                    { OUString(UNO_NAME_PARA_IS_FORBIDDEN_RULES), RES_PARATR_FORBIDDEN_RULES,     cppu::UnoType<bool>::get(),    PROPERTY_NONE ,0    },\
                    { OUString(UNO_NAME_PARA_VERT_ALIGNMENT), RES_PARATR_VERTALIGN,             cppu::UnoType<sal_Int16>::get(),    PROPERTY_NONE , 0  },\
                    { OUString(UNO_NAME_CHAR_ROTATION), RES_CHRATR_ROTATE,      cppu::UnoType<sal_Int16>::get(),   PROPERTY_NONE,    MID_ROTATE      },\
                    { OUString(UNO_NAME_CHAR_ROTATION_IS_FIT_TO_LINE), RES_CHRATR_ROTATE,       cppu::UnoType<bool>::get(),     PROPERTY_NONE,        MID_FITTOLINE  },\
                    { OUString(UNO_NAME_CHAR_SCALE_WIDTH), RES_CHRATR_SCALEW,       cppu::UnoType<sal_Int16>::get(),   PROPERTY_NONE,         0 },\
                    { OUString(UNO_NAME_CHAR_RELIEF), RES_CHRATR_RELIEF,      cppu::UnoType<sal_Int16>::get(),    PROPERTY_NONE,      MID_RELIEF },\
                    _PROP_DIFF_FONTHEIGHT\
                    { OUString(UNO_NAME_FOLLOW_STYLE), FN_UNO_FOLLOW_STYLE,     cppu::UnoType<OUString>::get(), PROPERTY_NONE, 0},\
                    { OUString(UNO_NAME_IS_PHYSICAL), FN_UNO_IS_PHYSICAL,     cppu::UnoType<bool>::get(), PropertyAttribute::READONLY, 0},\
                    { OUString(UNO_NAME_IS_AUTO_UPDATE), FN_UNO_IS_AUTO_UPDATE, cppu::UnoType<bool>::get(), PROPERTY_NONE, 0},\
                    { OUString(UNO_NAME_DISPLAY_NAME), FN_UNO_DISPLAY_NAME, cppu::UnoType<OUString>::get(), PropertyAttribute::READONLY, 0},\
                    { OUString(UNO_NAME_CATEGORY), FN_UNO_CATEGORY, cppu::UnoType<sal_Int16>::get(),           PROPERTY_NONE , 0 },\
                    { OUString(UNO_NAME_WRITING_MODE), RES_FRAMEDIR, cppu::UnoType<sal_Int16>::get(), PROPERTY_NONE, 0 },\
                    { OUString(UNO_NAME_PARA_IS_CONNECT_BORDER), RES_PARATR_CONNECT_BORDER, cppu::UnoType<bool>::get(), PropertyAttribute::MAYBEVOID, 0},\
                    { OUString(UNO_NAME_SNAP_TO_GRID), RES_PARATR_SNAPTOGRID, cppu::UnoType<bool>::get(), PropertyAttribute::MAYBEVOID, 0 }, \
                    { OUString(UNO_NAME_OUTLINE_LEVEL), RES_PARATR_OUTLINELEVEL,cppu::UnoType<sal_Int16>::get(), PropertyAttribute::MAYBEVOID, 0}, \
                    { OUString(UNO_NAME_HIDDEN), FN_UNO_HIDDEN,     cppu::UnoType<bool>::get(), PROPERTY_NONE, 0}, \
                    { OUString(UNO_NAME_STYLE_INTEROP_GRAB_BAG), FN_UNO_STYLE_INTEROP_GRAB_BAG, cppu::UnoType< cppu::UnoSequenceType<css::beans::PropertyValue> >::get(), PROPERTY_NONE, 0}, \
                    { OUString(UNO_NAME_PARA_INTEROP_GRAB_BAG), RES_PARATR_GRABBAG, cppu::UnoType< cppu::UnoSequenceType<css::beans::PropertyValue> >::get(), PROPERTY_NONE, 0},

#define COMMON_FLDTYP_PROPERTIES \
                    { OUString(UNO_NAME_IS_FIELD_USED),      FIELD_PROP_IS_FIELD_USED,      cppu::UnoType<float>::get(), PropertyAttribute::READONLY, 0},\
                    { OUString(UNO_NAME_IS_FIELD_DISPLAYED), FIELD_PROP_IS_FIELD_DISPLAYED, cppu::UnoType<sal_Int16>::get(), PropertyAttribute::READONLY, 0},\

#define COMMON_ACCESSIBILITY_TEXT_ATTRIBUTE \
                    { OUString(UNO_NAME_CHAR_BACK_COLOR), RES_CHRATR_BACKGROUND,    cppu::UnoType<sal_Int32>::get(),           PROPERTY_NONE ,MID_BACK_COLOR        }, \
                    { OUString(UNO_NAME_CHAR_COLOR), RES_CHRATR_COLOR,      cppu::UnoType<sal_Int32>::get(),           PROPERTY_NONE, 0},  \
                    { OUString(UNO_NAME_CHAR_CONTOURED), RES_CHRATR_CONTOUR,    cppu::UnoType<bool>::get()  ,       PROPERTY_NONE, 0},  \
                    { OUString(UNO_NAME_CHAR_EMPHASIS), RES_CHRATR_EMPHASIS_MARK,           cppu::UnoType<sal_Int16>::get(),   PROPERTY_NONE, MID_EMPHASIS},   \
                    { OUString(UNO_NAME_CHAR_ESCAPEMENT), RES_CHRATR_ESCAPEMENT,  cppu::UnoType<sal_Int16>::get(),             PROPERTY_NONE, MID_ESC          },  \
                    { OUString(UNO_NAME_CHAR_FONT_NAME), RES_CHRATR_FONT,       cppu::UnoType<OUString>::get(),  PropertyAttribute::MAYBEVOID, MID_FONT_FAMILY_NAME }, \
                    { OUString(UNO_NAME_CHAR_HEIGHT), RES_CHRATR_FONTSIZE  ,  cppu::UnoType<float>::get(),         PropertyAttribute::MAYBEVOID, MID_FONTHEIGHT|CONVERT_TWIPS},    \
                    { OUString(UNO_NAME_CHAR_POSTURE), RES_CHRATR_POSTURE   ,  cppu::UnoType<css::awt::FontSlant>::get(),        PropertyAttribute::MAYBEVOID, MID_POSTURE}, \
                    { OUString(UNO_NAME_CHAR_SHADOWED), RES_CHRATR_SHADOWED  ,  cppu::UnoType<bool>::get()  ,       PROPERTY_NONE, 0},  \
                    { OUString(UNO_NAME_CHAR_STRIKEOUT), RES_CHRATR_CROSSEDOUT,  cppu::UnoType<sal_Int16>::get(),                  PropertyAttribute::MAYBEVOID, MID_CROSS_OUT},   \
                    { OUString(UNO_NAME_CHAR_UNDERLINE_COLOR), RES_CHRATR_UNDERLINE ,  cppu::UnoType<sal_Int32>::get(),            PropertyAttribute::MAYBEVOID, MID_TL_COLOR},    \
                    { OUString(UNO_NAME_CHAR_WEIGHT), RES_CHRATR_WEIGHT    ,  cppu::UnoType<float>::get(),             PropertyAttribute::MAYBEVOID, MID_WEIGHT},  \
                    { OUString(UNO_NAME_NUMBERING_LEVEL), RES_PARATR_LIST_LEVEL,cppu::UnoType<sal_Int16>::get(), PropertyAttribute::MAYBEVOID, 0}, \
                    { OUString(UNO_NAME_CHAR_UNDERLINE), RES_CHRATR_UNDERLINE ,  cppu::UnoType<sal_Int16>::get(),              PropertyAttribute::MAYBEVOID, MID_TL_STYLE},    \
                    { OUString(UNO_NAME_NUMBERING_RULES), RES_PARATR_NUMRULE,cppu::UnoType<sal_Int16>::get(), PropertyAttribute::MAYBEVOID, CONVERT_TWIPS},    \
                    { OUString(UNO_NAME_PARA_ADJUST), RES_PARATR_ADJUST,      cppu::UnoType<sal_Int16>::get(),         PropertyAttribute::MAYBEVOID, MID_PARA_ADJUST}, \
                    { OUString(UNO_NAME_PARA_BOTTOM_MARGIN), RES_UL_SPACE,          cppu::UnoType<sal_Int32>::get(),           PropertyAttribute::MAYBEVOID, MID_LO_MARGIN|CONVERT_TWIPS}, \
                    { OUString(UNO_NAME_PARA_FIRST_LINE_INDENT), RES_LR_SPACE,           cppu::UnoType<sal_Int32>::get(),      PropertyAttribute::MAYBEVOID, MID_FIRST_LINE_INDENT|CONVERT_TWIPS}, \
                    { OUString(UNO_NAME_PARA_LEFT_MARGIN), RES_LR_SPACE,            cppu::UnoType<sal_Int32>::get(),           PropertyAttribute::MAYBEVOID, MID_TXT_LMARGIN|CONVERT_TWIPS},   \
                    { OUString(UNO_NAME_PARA_LINE_SPACING), RES_PARATR_LINESPACING, cppu::UnoType<css::style::LineSpacing>::get(),       PropertyAttribute::MAYBEVOID,     CONVERT_TWIPS},   \
                    { OUString(UNO_NAME_PARA_RIGHT_MARGIN), RES_LR_SPACE,           cppu::UnoType<sal_Int32>::get(),           PropertyAttribute::MAYBEVOID, MID_R_MARGIN|CONVERT_TWIPS},  \
                    { OUString(UNO_NAME_TABSTOPS), RES_PARATR_TABSTOP,   cppu::UnoType< cppu::UnoSequenceType<css::style::TabStop> >::get(),   PropertyAttribute::MAYBEVOID, CONVERT_TWIPS}, \

//UUUU
#define FILL_PROPERTIES_SW_BMP \
    { OUString(UNO_NAME_SW_FILLBMP_LOGICAL_SIZE),               XATTR_FILLBMP_SIZELOG,      cppu::UnoType<bool>::get(),       0,  0}, \
    { OUString(UNO_NAME_SW_FILLBMP_OFFSET_X),                   XATTR_FILLBMP_TILEOFFSETX,  cppu::UnoType<sal_Int32>::get(),   0,  0}, \
    { OUString(UNO_NAME_SW_FILLBMP_OFFSET_Y),                   XATTR_FILLBMP_TILEOFFSETY,  cppu::UnoType<sal_Int32>::get(),   0,  0}, \
    { OUString(UNO_NAME_SW_FILLBMP_POSITION_OFFSET_X),          XATTR_FILLBMP_POSOFFSETX,   cppu::UnoType<sal_Int32>::get(),   0,  0}, \
    { OUString(UNO_NAME_SW_FILLBMP_POSITION_OFFSET_Y),          XATTR_FILLBMP_POSOFFSETY,   cppu::UnoType<sal_Int32>::get(),   0,  0}, \
    { OUString(UNO_NAME_SW_FILLBMP_RECTANGLE_POINT),            XATTR_FILLBMP_POS,          cppu::UnoType<css::drawing::RectanglePoint>::get(), 0,  0}, \
    { OUString(UNO_NAME_SW_FILLBMP_SIZE_X),                     XATTR_FILLBMP_SIZEX,        cppu::UnoType<sal_Int32>::get(),   0,  SFX_METRIC_ITEM}, \
    { OUString(UNO_NAME_SW_FILLBMP_SIZE_Y),                     XATTR_FILLBMP_SIZEY,        cppu::UnoType<sal_Int32>::get(),   0,  SFX_METRIC_ITEM},    \
    { OUString(UNO_NAME_SW_FILLBMP_STRETCH),                    XATTR_FILLBMP_STRETCH,      cppu::UnoType<bool>::get(),       0,  0}, \
    { OUString(UNO_NAME_SW_FILLBMP_TILE),                       XATTR_FILLBMP_TILE,         cppu::UnoType<bool>::get(),       0,  0},\
    { OUString(UNO_NAME_SW_FILLBMP_MODE),                       OWN_ATTR_FILLBMP_MODE,      cppu::UnoType<drawing::BitmapMode>::get(), 0,  0}, \

//UUUU
#define FILL_PROPERTIES_SW_DEFAULTS \
    { OUString(UNO_NAME_SW_FILLCOLOR),                          XATTR_FILLCOLOR,            cppu::UnoType<sal_Int32>::get(),   0,  0}, \

//UUUU
#define FILL_PROPERTIES_SW \
    FILL_PROPERTIES_SW_BMP \
    FILL_PROPERTIES_SW_DEFAULTS \
    { OUString(UNO_NAME_SW_FILLBACKGROUND),                 XATTR_FILLBACKGROUND,           cppu::UnoType<bool>::get(),        0, 0}, \
    { OUString(UNO_NAME_SW_FILLBITMAP),                     XATTR_FILLBITMAP,               cppu::UnoType<css::awt::XBitmap>::get(), 0, MID_BITMAP}, \
    { OUString(UNO_NAME_SW_FILLBITMAPNAME),                 XATTR_FILLBITMAP,               cppu::UnoType<OUString>::get(),    0,  MID_NAME }, \
    { OUString(UNO_NAME_SW_FILLBITMAPURL),                  XATTR_FILLBITMAP,               cppu::UnoType<OUString>::get(),    0,  MID_GRAFURL }, \
    { OUString(UNO_NAME_SW_FILLGRADIENTSTEPCOUNT),          XATTR_GRADIENTSTEPCOUNT,        cppu::UnoType<sal_Int16>::get(),   0,  0}, \
    { OUString(UNO_NAME_SW_FILLGRADIENT),                   XATTR_FILLGRADIENT,             cppu::UnoType<css::awt::Gradient>::get(), 0, MID_FILLGRADIENT}, \
    { OUString(UNO_NAME_SW_FILLGRADIENTNAME),               XATTR_FILLGRADIENT,             cppu::UnoType<OUString>::get(),    0, MID_NAME }, \
    { OUString(UNO_NAME_SW_FILLHATCH),                      XATTR_FILLHATCH,                cppu::UnoType<css::drawing::Hatch>::get(), 0, MID_FILLHATCH}, \
    { OUString(UNO_NAME_SW_FILLHATCHNAME),                  XATTR_FILLHATCH,                cppu::UnoType<OUString>::get(),  0,  MID_NAME }, \
    { OUString(UNO_NAME_SW_FILLSTYLE),                      XATTR_FILLSTYLE,                cppu::UnoType<css::drawing::FillStyle>::get(), 0, 0}, \
    { OUString(UNO_NAME_SW_FILL_TRANSPARENCE),              XATTR_FILLTRANSPARENCE,         cppu::UnoType<sal_Int16>::get(), 0, 0}, \
    { OUString(UNO_NAME_SW_FILLTRANSPARENCEGRADIENT),       XATTR_FILLFLOATTRANSPARENCE,    cppu::UnoType<css::awt::Gradient>::get(), 0,  MID_FILLGRADIENT}, \
    { OUString(UNO_NAME_SW_FILLTRANSPARENCEGRADIENTNAME),   XATTR_FILLFLOATTRANSPARENCE,    cppu::UnoType<OUString>::get(),  0,  MID_NAME }, \
    { OUString(UNO_NAME_SW_FILLCOLOR_2),                    XATTR_SECONDARYFILLCOLOR,       cppu::UnoType<sal_Int32>::get(), 0,  0}, \

const SfxItemPropertyMapEntry* SwUnoPropertyMapProvider::GetPropertyMapEntries(sal_uInt16 nPropertyId)
{
    OSL_ENSURE(nPropertyId < PROPERTY_MAP_END, "Id ?" );
    if( !m_aMapEntriesArr[ nPropertyId ] )
    {
        switch(nPropertyId)
        {
            case PROPERTY_MAP_TEXT_CURSOR:
            {
                static SfxItemPropertyMapEntry const aCharAndParaMap_Impl[] =
                {
                    COMPLETE_TEXT_CURSOR_MAP
                    { OUString(), 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aCharAndParaMap_Impl;
            }
            break;
            case PROPERTY_MAP_ACCESSIBILITY_TEXT_ATTRIBUTE:
            {
                static SfxItemPropertyMapEntry const aAccessibilityTextAttrMap_Impl[] =
                {
                    COMMON_ACCESSIBILITY_TEXT_ATTRIBUTE
                    { OUString(), 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aAccessibilityTextAttrMap_Impl;
            }
            break;
            case PROPERTY_MAP_PARAGRAPH:
            {
                static SfxItemPropertyMapEntry const aParagraphMap_Impl[] =
                {
                    COMMON_CRSR_PARA_PROPERTIES_2
                    TABSTOPS_MAP_ENTRY
                    COMMON_TEXT_CONTENT_PROPERTIES
                    { OUString(UNO_NAME_CHAR_STYLE_NAME), RES_TXTATR_CHARFMT,     cppu::UnoType<OUString>::get(),         PropertyAttribute::MAYBEVOID,     0},
                    { OUString(UNO_NAME_CHAR_STYLE_NAMES), FN_UNO_CHARFMT_SEQUENCE,  cppu::UnoType< cppu::UnoSequenceType<OUString> >::get(),     PropertyAttribute::MAYBEVOID,     0},
                    //UUUU added FillProperties for SW, same as FILL_PROPERTIES in svx
                    // but need own defines in Writer due to later association of strings
                    // and uno types (see loop at end of this method and definition of SW_PROP_NMID)
                    // This entry is for adding that properties to style import/export
                    //UUUU Added for paragraph backgrounds, this is for paragraph itself
                    FILL_PROPERTIES_SW
                    { OUString(), 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aParagraphMap_Impl;
            }
            break;
            case PROPERTY_MAP_PARA_AUTO_STYLE :
            {
                static SfxItemPropertyMapEntry const aAutoParaStyleMap [] =
                {
                    { OUString(UNO_NAME_PARA_STYLE_NAME), RES_FRMATR_STYLE_NAME,        cppu::UnoType<OUString>::get(),                PropertyAttribute::MAYBEVOID,     0},
                    { OUString(UNO_NAME_PAGE_STYLE_NAME), FN_UNO_PAGE_STYLE,        cppu::UnoType<OUString>::get(),         PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY,   0},
                    { OUString(UNO_NAME_NUMBERING_IS_NUMBER), FN_UNO_IS_NUMBER,     cppu::UnoType<bool>::get()  ,       PropertyAttribute::MAYBEVOID,     0},
                    { OUString(UNO_NAME_NUMBERING_LEVEL), FN_UNO_NUM_LEVEL,     cppu::UnoType<sal_Int16>::get(),           PropertyAttribute::MAYBEVOID, 0},
                    { OUString(UNO_NAME_NUMBERING_START_VALUE), FN_UNO_NUM_START_VALUE, cppu::UnoType<sal_Int16>::get(),           PropertyAttribute::MAYBEVOID, CONVERT_TWIPS},
                    { OUString(UNO_NAME_DOCUMENT_INDEX), FN_UNO_DOCUMENT_INDEX, cppu::UnoType<css::text::XDocumentIndex>::get(), PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY ,0 },
                    { OUString(UNO_NAME_TEXT_TABLE), FN_UNO_TEXT_TABLE,     cppu::UnoType<css::text::XTextTable>::get(),     PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY ,0 },
                    { OUString(UNO_NAME_CELL), FN_UNO_CELL,         cppu::UnoType<css::table::XCell>::get(),         PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY ,0 },
                    { OUString(UNO_NAME_TEXT_FRAME), FN_UNO_TEXT_FRAME,     cppu::UnoType<css::text::XTextFrame>::get(),        PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY ,0 },
                    { OUString(UNO_NAME_TEXT_SECTION), FN_UNO_TEXT_SECTION, cppu::UnoType<css::text::XTextSection>::get(),  PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY ,0 },
                    { OUString(UNO_NAME_PARA_CHAPTER_NUMBERING_LEVEL), FN_UNO_PARA_CHAPTER_NUMBERING_LEVEL,cppu::UnoType<sal_Int8>::get(), PROPERTY_NONE, 0},
                    { OUString(UNO_NAME_PARA_CONDITIONAL_STYLE_NAME), RES_FRMATR_CONDITIONAL_STYLE_NAME,        cppu::UnoType<OUString>::get(),         PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY,   0},
                    { OUString(UNO_NAME_PARA_IS_NUMBERING_RESTART), FN_NUMBER_NEWSTART,     cppu::UnoType<bool>::get(),     PropertyAttribute::MAYBEVOID, 0 },
                    { OUString(UNO_NAME_OUTLINE_LEVEL), RES_PARATR_OUTLINELEVEL,        cppu::UnoType<sal_Int16>::get(),                PropertyAttribute::MAYBEVOID,     0},
                    COMMON_CRSR_PARA_PROPERTIES_WITHOUT_FN
                    TABSTOPS_MAP_ENTRY
                    COMMON_TEXT_CONTENT_PROPERTIES
                    { OUString(UNO_NAME_PARA_AUTO_STYLE_NAME), RES_AUTO_STYLE,     cppu::UnoType<OUString>::get(),         PropertyAttribute::MAYBEVOID,     0},
                    //UUUU added FillProperties for SW, same as FILL_PROPERTIES in svx
                    // but need own defines in Writer due to later association of strings
                    // and uno types (see loop at end of this method and definition of SW_PROP_NMID)
                    // This entry is for adding that properties to style import/export
                    //UUUU Added for paragraph backgrounds, this is for Paragraph AutoStyles
                    FILL_PROPERTIES_SW
                    { OUString(), 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aAutoParaStyleMap;
            }
            break;
            case PROPERTY_MAP_CHAR_STYLE :
            {
                static SfxItemPropertyMapEntry const aCharStyleMap   [] =
                {
                    { OUString(UNO_NAME_CHAR_AUTO_KERNING), RES_CHRATR_AUTOKERN  ,  cppu::UnoType<bool>::get()  ,       PROPERTY_NONE,     0},
                    { OUString(UNO_NAME_CHAR_BACK_TRANSPARENT), RES_CHRATR_BACKGROUND,  cppu::UnoType<bool>::get(),         PROPERTY_NONE ,MID_GRAPHIC_TRANSPARENT       },
                    { OUString(UNO_NAME_CHAR_BACK_COLOR), RES_CHRATR_BACKGROUND,    cppu::UnoType<sal_Int32>::get(),           PROPERTY_NONE ,MID_BACK_COLOR        },
                    { OUString(UNO_NAME_CHAR_HIGHLIGHT), RES_CHRATR_HIGHLIGHT, cppu::UnoType<sal_Int32>::get(), PROPERTY_NONE, MID_BACK_COLOR },
                    { OUString(UNO_NAME_CHAR_CASE_MAP), RES_CHRATR_CASEMAP,     cppu::UnoType<sal_Int16>::get(),           PROPERTY_NONE, 0},
                    { OUString(UNO_NAME_CHAR_COLOR), RES_CHRATR_COLOR,      cppu::UnoType<sal_Int32>::get(),           PROPERTY_NONE, 0},
                    { OUString(UNO_NAME_CHAR_STRIKEOUT), RES_CHRATR_CROSSEDOUT,  cppu::UnoType<sal_Int16>::get(),                  PropertyAttribute::MAYBEVOID, MID_CROSS_OUT},
                    { OUString(UNO_NAME_CHAR_CROSSED_OUT), RES_CHRATR_CROSSEDOUT,  cppu::UnoType<bool>::get()  ,        PROPERTY_NONE, 0},
                    { OUString(UNO_NAME_CHAR_ESCAPEMENT), RES_CHRATR_ESCAPEMENT,  cppu::UnoType<sal_Int16>::get(),             PROPERTY_NONE, MID_ESC          },
                    { OUString(UNO_NAME_CHAR_ESCAPEMENT_HEIGHT), RES_CHRATR_ESCAPEMENT,     cppu::UnoType<sal_Int8>::get()  ,          PROPERTY_NONE, MID_ESC_HEIGHT},
                    { OUString(UNO_NAME_CHAR_FLASH), RES_CHRATR_BLINK   ,   cppu::UnoType<bool>::get()  ,       PROPERTY_NONE,     0},
                    { OUString(UNO_NAME_CHAR_HIDDEN), RES_CHRATR_HIDDEN, cppu::UnoType<bool>::get()  ,       PROPERTY_NONE,     0},
                    _STANDARD_FONT_PROPERTIES
                    _CJK_FONT_PROPERTIES
                    _CTL_FONT_PROPERTIES
                    { OUString(UNO_NAME_CHAR_UNDERLINE), RES_CHRATR_UNDERLINE ,  cppu::UnoType<sal_Int16>::get(),      PROPERTY_NONE, MID_TL_STYLE},
                    { OUString(UNO_NAME_CHAR_UNDERLINE_COLOR), RES_CHRATR_UNDERLINE ,  cppu::UnoType<sal_Int32>::get(),            PROPERTY_NONE, MID_TL_COLOR},
                    { OUString(UNO_NAME_CHAR_UNDERLINE_HAS_COLOR), RES_CHRATR_UNDERLINE ,  cppu::UnoType<bool>::get(),              PROPERTY_NONE, MID_TL_HASCOLOR},
                    { OUString(UNO_NAME_CHAR_OVERLINE), RES_CHRATR_OVERLINE ,  cppu::UnoType<sal_Int16>::get(),    PROPERTY_NONE, MID_TL_STYLE},
                    { OUString(UNO_NAME_CHAR_OVERLINE_COLOR), RES_CHRATR_OVERLINE ,  cppu::UnoType<sal_Int32>::get(),              PROPERTY_NONE, MID_TL_COLOR},
                    { OUString(UNO_NAME_CHAR_OVERLINE_HAS_COLOR), RES_CHRATR_OVERLINE ,  cppu::UnoType<bool>::get(),            PROPERTY_NONE, MID_TL_HASCOLOR},
                    { OUString(UNO_NAME_CHAR_KERNING), RES_CHRATR_KERNING    ,  cppu::UnoType<sal_Int16>::get()  ,         PROPERTY_NONE,  CONVERT_TWIPS},
                    { OUString(UNO_NAME_CHAR_NO_HYPHENATION), RES_CHRATR_NOHYPHEN   ,   cppu::UnoType<bool>::get()  ,       PROPERTY_NONE,     0},
                    { OUString(UNO_NAME_CHAR_SHADOWED), RES_CHRATR_SHADOWED  ,  cppu::UnoType<bool>::get()  ,       PROPERTY_NONE, 0},
                    { OUString(UNO_NAME_CHAR_CONTOURED), RES_CHRATR_CONTOUR,    cppu::UnoType<bool>::get()  ,       PROPERTY_NONE, 0},
                    { OUString(UNO_NAME_CHAR_WORD_MODE), RES_CHRATR_WORDLINEMODE,cppu::UnoType<bool>::get()  ,    PROPERTY_NONE,     0},
                    { OUString(UNO_NAME_USER_DEFINED_ATTRIBUTES), RES_UNKNOWNATR_CONTAINER, cppu::UnoType<css::container::XNameContainer>::get(), PropertyAttribute::MAYBEVOID, 0 },
                    { OUString(UNO_NAME_IS_PHYSICAL), FN_UNO_IS_PHYSICAL,     cppu::UnoType<bool>::get(), PropertyAttribute::READONLY, 0},
                    { OUString(UNO_NAME_HIDDEN), FN_UNO_HIDDEN,     cppu::UnoType<bool>::get(), PROPERTY_NONE, 0},
                    { OUString(UNO_NAME_STYLE_INTEROP_GRAB_BAG), FN_UNO_STYLE_INTEROP_GRAB_BAG, cppu::UnoType< cppu::UnoSequenceType<css::beans::PropertyValue> >::get(), PROPERTY_NONE, 0},
                    { OUString(UNO_NAME_DISPLAY_NAME), FN_UNO_DISPLAY_NAME, cppu::UnoType<OUString>::get(), PropertyAttribute::READONLY, 0},
                    { OUString(UNO_NAME_CHAR_COMBINE_IS_ON), RES_CHRATR_TWO_LINES,          cppu::UnoType<bool>::get(),     PROPERTY_NONE, MID_TWOLINES},
                    { OUString(UNO_NAME_CHAR_COMBINE_PREFIX), RES_CHRATR_TWO_LINES,             cppu::UnoType<OUString>::get(),    PROPERTY_NONE, MID_START_BRACKET},
                    { OUString(UNO_NAME_CHAR_COMBINE_SUFFIX), RES_CHRATR_TWO_LINES,             cppu::UnoType<OUString>::get(),    PROPERTY_NONE, MID_END_BRACKET},
                    { OUString(UNO_NAME_CHAR_EMPHASIS), RES_CHRATR_EMPHASIS_MARK,           cppu::UnoType<sal_Int16>::get(),   PROPERTY_NONE, MID_EMPHASIS},
                    _PROP_DIFF_FONTHEIGHT
                    { OUString(UNO_NAME_CHAR_ROTATION), RES_CHRATR_ROTATE,      cppu::UnoType<sal_Int16>::get(),   PROPERTY_NONE,    MID_ROTATE      },
                    { OUString(UNO_NAME_CHAR_ROTATION_IS_FIT_TO_LINE), RES_CHRATR_ROTATE,       cppu::UnoType<bool>::get(),     PROPERTY_NONE,        MID_FITTOLINE  },
                    { OUString(UNO_NAME_CHAR_SCALE_WIDTH), RES_CHRATR_SCALEW,       cppu::UnoType<sal_Int16>::get(),   PROPERTY_NONE,         0 },
                    { OUString(UNO_NAME_CHAR_RELIEF), RES_CHRATR_RELIEF,      cppu::UnoType<sal_Int16>::get(),    PROPERTY_NONE,      MID_RELIEF },
                    { OUString(UNO_NAME_CHAR_LEFT_BORDER), RES_CHRATR_BOX, cppu::UnoType<css::table::BorderLine>::get(), PROPERTY_NONE, LEFT_BORDER |CONVERT_TWIPS },
                    { OUString(UNO_NAME_CHAR_RIGHT_BORDER), RES_CHRATR_BOX, cppu::UnoType<css::table::BorderLine>::get(), PROPERTY_NONE, RIGHT_BORDER |CONVERT_TWIPS },
                    { OUString(UNO_NAME_CHAR_TOP_BORDER), RES_CHRATR_BOX, cppu::UnoType<css::table::BorderLine>::get(), PROPERTY_NONE, TOP_BORDER |CONVERT_TWIPS },
                    { OUString(UNO_NAME_CHAR_BOTTOM_BORDER), RES_CHRATR_BOX, cppu::UnoType<css::table::BorderLine>::get(), PROPERTY_NONE, BOTTOM_BORDER |CONVERT_TWIPS },
                    { OUString(UNO_NAME_CHAR_BORDER_DISTANCE), RES_CHRATR_BOX, cppu::UnoType<sal_Int32>::get(), PROPERTY_NONE, BORDER_DISTANCE |CONVERT_TWIPS },
                    { OUString(UNO_NAME_CHAR_LEFT_BORDER_DISTANCE), RES_CHRATR_BOX, cppu::UnoType<sal_Int32>::get(), PROPERTY_NONE, LEFT_BORDER_DISTANCE |CONVERT_TWIPS },
                    { OUString(UNO_NAME_CHAR_RIGHT_BORDER_DISTANCE), RES_CHRATR_BOX, cppu::UnoType<sal_Int32>::get(), PROPERTY_NONE, RIGHT_BORDER_DISTANCE |CONVERT_TWIPS },
                    { OUString(UNO_NAME_CHAR_TOP_BORDER_DISTANCE), RES_CHRATR_BOX, cppu::UnoType<sal_Int32>::get(), PROPERTY_NONE, TOP_BORDER_DISTANCE |CONVERT_TWIPS },
                    { OUString(UNO_NAME_CHAR_BOTTOM_BORDER_DISTANCE), RES_CHRATR_BOX, cppu::UnoType<sal_Int32>::get(), PROPERTY_NONE, BOTTOM_BORDER_DISTANCE|CONVERT_TWIPS },
                    { OUString(UNO_NAME_CHAR_SHADOW_FORMAT), RES_CHRATR_SHADOW, cppu::UnoType<css::table::ShadowFormat>::get(), PROPERTY_NONE, CONVERT_TWIPS},
                    { OUString(), 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aCharStyleMap;
            }
            break;
            case PROPERTY_MAP_CHAR_AUTO_STYLE :
            {
                // same as PROPERTY_MAP_TEXTPORTION_EXTENSIONS
                static SfxItemPropertyMapEntry const aAutoCharStyleMap   [] =
                {
                    { OUString(UNO_NAME_CHAR_AUTO_KERNING), RES_CHRATR_AUTOKERN  ,  cppu::UnoType<bool>::get()  ,       PROPERTY_NONE,     0},
                    { OUString(UNO_NAME_CHAR_BACK_TRANSPARENT), RES_CHRATR_BACKGROUND,  cppu::UnoType<bool>::get(),         PROPERTY_NONE ,MID_GRAPHIC_TRANSPARENT       },
                    { OUString(UNO_NAME_CHAR_BACK_COLOR), RES_CHRATR_BACKGROUND,    cppu::UnoType<sal_Int32>::get(),           PROPERTY_NONE ,MID_BACK_COLOR        },
                    { OUString(UNO_NAME_CHAR_HIGHLIGHT), RES_CHRATR_HIGHLIGHT, cppu::UnoType<sal_Int32>::get(), PROPERTY_NONE, MID_BACK_COLOR },
                    { OUString(UNO_NAME_CHAR_CASE_MAP), RES_CHRATR_CASEMAP,     cppu::UnoType<sal_Int16>::get(),           PROPERTY_NONE, 0},
                    { OUString(UNO_NAME_CHAR_COLOR), RES_CHRATR_COLOR,      cppu::UnoType<sal_Int32>::get(),           PROPERTY_NONE, 0},
                    { OUString(UNO_NAME_CHAR_STRIKEOUT), RES_CHRATR_CROSSEDOUT,  cppu::UnoType<sal_Int16>::get(),                  PropertyAttribute::MAYBEVOID, MID_CROSS_OUT},
                    { OUString(UNO_NAME_CHAR_CROSSED_OUT), RES_CHRATR_CROSSEDOUT,  cppu::UnoType<bool>::get()  ,        PROPERTY_NONE, 0},
                    { OUString(UNO_NAME_CHAR_ESCAPEMENT), RES_CHRATR_ESCAPEMENT,  cppu::UnoType<sal_Int16>::get(),             PROPERTY_NONE, MID_ESC          },
                    { OUString(UNO_NAME_CHAR_ESCAPEMENT_HEIGHT), RES_CHRATR_ESCAPEMENT,     cppu::UnoType<sal_Int8>::get()  ,          PROPERTY_NONE, MID_ESC_HEIGHT},
                    { OUString(UNO_NAME_CHAR_FLASH), RES_CHRATR_BLINK   ,   cppu::UnoType<bool>::get()  ,       PROPERTY_NONE,     0},
                    { OUString(UNO_NAME_CHAR_HIDDEN), RES_CHRATR_HIDDEN, cppu::UnoType<bool>::get()  ,       PROPERTY_NONE,     0},
                    _STANDARD_FONT_PROPERTIES
                    _CJK_FONT_PROPERTIES
                    _CTL_FONT_PROPERTIES
                    { OUString(UNO_NAME_CHAR_UNDERLINE), RES_CHRATR_UNDERLINE ,  cppu::UnoType<sal_Int16>::get(),      PROPERTY_NONE, MID_TL_STYLE},
                    { OUString(UNO_NAME_CHAR_UNDERLINE_COLOR), RES_CHRATR_UNDERLINE ,  cppu::UnoType<sal_Int32>::get(),            PROPERTY_NONE, MID_TL_COLOR},
                    { OUString(UNO_NAME_CHAR_UNDERLINE_HAS_COLOR), RES_CHRATR_UNDERLINE ,  cppu::UnoType<bool>::get(),              PROPERTY_NONE, MID_TL_HASCOLOR},
                    { OUString(UNO_NAME_CHAR_OVERLINE), RES_CHRATR_OVERLINE ,  cppu::UnoType<sal_Int16>::get(),      PROPERTY_NONE, MID_TL_STYLE},
                    { OUString(UNO_NAME_CHAR_OVERLINE_COLOR), RES_CHRATR_OVERLINE ,  cppu::UnoType<sal_Int32>::get(),            PROPERTY_NONE, MID_TL_COLOR},
                    { OUString(UNO_NAME_CHAR_OVERLINE_HAS_COLOR), RES_CHRATR_OVERLINE ,  cppu::UnoType<bool>::get(),              PROPERTY_NONE, MID_TL_HASCOLOR},
                    { OUString(UNO_NAME_CHAR_KERNING), RES_CHRATR_KERNING    ,  cppu::UnoType<sal_Int16>::get()  ,         PROPERTY_NONE,  CONVERT_TWIPS},
                    { OUString(UNO_NAME_CHAR_NO_HYPHENATION), RES_CHRATR_NOHYPHEN   ,   cppu::UnoType<bool>::get()  ,       PROPERTY_NONE,     0},
                    { OUString(UNO_NAME_CHAR_SHADOWED), RES_CHRATR_SHADOWED  ,  cppu::UnoType<bool>::get()  ,       PROPERTY_NONE, 0},
                    { OUString(UNO_NAME_CHAR_CONTOURED), RES_CHRATR_CONTOUR,    cppu::UnoType<bool>::get()  ,       PROPERTY_NONE, 0},
                    { OUString(UNO_NAME_CHAR_WORD_MODE), RES_CHRATR_WORDLINEMODE,cppu::UnoType<bool>::get()  ,    PROPERTY_NONE,     0},
                    { OUString(UNO_NAME_USER_DEFINED_ATTRIBUTES), RES_UNKNOWNATR_CONTAINER, cppu::UnoType<css::container::XNameContainer>::get(), PropertyAttribute::MAYBEVOID, 0 },
                    { OUString(UNO_NAME_TEXT_USER_DEFINED_ATTRIBUTES), RES_TXTATR_UNKNOWN_CONTAINER, cppu::UnoType<css::container::XNameContainer>::get(), PropertyAttribute::MAYBEVOID, 0 },
                    { OUString(UNO_NAME_IS_PHYSICAL), FN_UNO_IS_PHYSICAL,     cppu::UnoType<bool>::get(), PropertyAttribute::READONLY, 0},
                    { OUString(UNO_NAME_DISPLAY_NAME), FN_UNO_DISPLAY_NAME, cppu::UnoType<OUString>::get(), PropertyAttribute::READONLY, 0},
                    { OUString(UNO_NAME_CHAR_COMBINE_IS_ON), RES_CHRATR_TWO_LINES,          cppu::UnoType<bool>::get(),     PROPERTY_NONE, MID_TWOLINES},
                    { OUString(UNO_NAME_CHAR_COMBINE_PREFIX), RES_CHRATR_TWO_LINES,             cppu::UnoType<OUString>::get(),    PROPERTY_NONE, MID_START_BRACKET},
                    { OUString(UNO_NAME_CHAR_COMBINE_SUFFIX), RES_CHRATR_TWO_LINES,             cppu::UnoType<OUString>::get(),    PROPERTY_NONE, MID_END_BRACKET},
                    { OUString(UNO_NAME_CHAR_EMPHASIS), RES_CHRATR_EMPHASIS_MARK,           cppu::UnoType<sal_Int16>::get(),   PROPERTY_NONE, MID_EMPHASIS},
                    { OUString(UNO_NAME_CHAR_ROTATION), RES_CHRATR_ROTATE,      cppu::UnoType<sal_Int16>::get(),   PROPERTY_NONE,    MID_ROTATE      },
                    { OUString(UNO_NAME_CHAR_ROTATION_IS_FIT_TO_LINE), RES_CHRATR_ROTATE,       cppu::UnoType<bool>::get(),     PROPERTY_NONE,        MID_FITTOLINE  },
                    { OUString(UNO_NAME_CHAR_SCALE_WIDTH), RES_CHRATR_SCALEW,       cppu::UnoType<sal_Int16>::get(),   PROPERTY_NONE,         0 },
                    { OUString(UNO_NAME_CHAR_RELIEF), RES_CHRATR_RELIEF,      cppu::UnoType<sal_Int16>::get(),    PROPERTY_NONE,      MID_RELIEF },
                    { OUString(UNO_NAME_CHAR_AUTO_STYLE_NAME), RES_TXTATR_AUTOFMT,     cppu::UnoType<OUString>::get(),         PropertyAttribute::MAYBEVOID,     0},
                    { OUString(UNO_NAME_CHAR_SHADING_VALUE), RES_CHRATR_BACKGROUND,      cppu::UnoType<sal_Int32>::get(),    PROPERTY_NONE,      MID_SHADING_VALUE },
                    { OUString(UNO_NAME_CHAR_LEFT_BORDER), RES_CHRATR_BOX, cppu::UnoType<css::table::BorderLine>::get(), PROPERTY_NONE, LEFT_BORDER |CONVERT_TWIPS },
                    { OUString(UNO_NAME_CHAR_RIGHT_BORDER), RES_CHRATR_BOX, cppu::UnoType<css::table::BorderLine>::get(), PROPERTY_NONE, RIGHT_BORDER |CONVERT_TWIPS },
                    { OUString(UNO_NAME_CHAR_TOP_BORDER), RES_CHRATR_BOX, cppu::UnoType<css::table::BorderLine>::get(), PROPERTY_NONE, TOP_BORDER |CONVERT_TWIPS },
                    { OUString(UNO_NAME_CHAR_BOTTOM_BORDER), RES_CHRATR_BOX, cppu::UnoType<css::table::BorderLine>::get(), PROPERTY_NONE, BOTTOM_BORDER |CONVERT_TWIPS },
                    { OUString(UNO_NAME_CHAR_BORDER_DISTANCE), RES_CHRATR_BOX, cppu::UnoType<sal_Int32>::get(), PROPERTY_NONE, BORDER_DISTANCE |CONVERT_TWIPS },
                    { OUString(UNO_NAME_CHAR_LEFT_BORDER_DISTANCE), RES_CHRATR_BOX, cppu::UnoType<sal_Int32>::get(), PROPERTY_NONE, LEFT_BORDER_DISTANCE |CONVERT_TWIPS },
                    { OUString(UNO_NAME_CHAR_RIGHT_BORDER_DISTANCE), RES_CHRATR_BOX, cppu::UnoType<sal_Int32>::get(), PROPERTY_NONE, RIGHT_BORDER_DISTANCE |CONVERT_TWIPS },
                    { OUString(UNO_NAME_CHAR_TOP_BORDER_DISTANCE), RES_CHRATR_BOX, cppu::UnoType<sal_Int32>::get(), PROPERTY_NONE, TOP_BORDER_DISTANCE |CONVERT_TWIPS },
                    { OUString(UNO_NAME_CHAR_BOTTOM_BORDER_DISTANCE), RES_CHRATR_BOX, cppu::UnoType<sal_Int32>::get(), PROPERTY_NONE, BOTTOM_BORDER_DISTANCE|CONVERT_TWIPS },
                    { OUString(UNO_NAME_CHAR_SHADOW_FORMAT), RES_CHRATR_SHADOW, cppu::UnoType<css::table::ShadowFormat>::get(), PROPERTY_NONE, CONVERT_TWIPS},
                    { OUString(), 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aAutoCharStyleMap;
            }
            break;
            case PROPERTY_MAP_RUBY_AUTO_STYLE :
            {
                static SfxItemPropertyMapEntry const aAutoRubyStyleMap [] =
                {
                    { OUString(UNO_NAME_RUBY_ADJUST), RES_TXTATR_CJK_RUBY,  cppu::UnoType<sal_Int16>::get(),   PropertyAttribute::MAYBEVOID,          MID_RUBY_ADJUST },
                    { OUString(UNO_NAME_RUBY_IS_ABOVE), RES_TXTATR_CJK_RUBY,    cppu::UnoType<bool>::get(),  PropertyAttribute::MAYBEVOID,     MID_RUBY_ABOVE },
                    { OUString(), 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aAutoRubyStyleMap;
            }
            break;
            case PROPERTY_MAP_PARA_STYLE :
            {
                static SfxItemPropertyMapEntry const aParaStyleMap [] =
                {
                    COMMON_PARA_STYLE_PROPERTIES
                    //UUUU added FillProperties for SW, same as FILL_PROPERTIES in svx
                    // but need own defines in Writer due to later association of strings
                    // and uno types (see loop at end of this method and definition of SW_PROP_NMID)
                    // This entry is for adding that properties to style import/export
                    //UUUU Added for paragraph backgrounds, this is for Paragraph Styles
                    FILL_PROPERTIES_SW
                    { OUString(), 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aParaStyleMap;
            }
            break;
            case PROPERTY_MAP_CONDITIONAL_PARA_STYLE :
            {
                static SfxItemPropertyMapEntry const aParaStyleMap [] =
                {
                    COMMON_PARA_STYLE_PROPERTIES
                    { OUString(UNO_NAME_PARA_STYLE_CONDITIONS), FN_UNO_PARA_STYLE_CONDITIONS, cppu::UnoType< cppu::UnoSequenceType<css::beans::NamedValue> >::get(), PropertyAttribute::MAYBEVOID, 0},

                    //UUUU added FillProperties for SW, same as FILL_PROPERTIES in svx
                    // but need own defines in Writer due to later association of strings
                    // and uno types (see loop at end of this method and definition of SW_PROP_NMID)
                    // This entry is for adding that properties to style import/export
                    //UUUU Added for paragraph backgrounds, this is for Paragraph Styles
                    FILL_PROPERTIES_SW

                    { OUString(), 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aParaStyleMap;
            }
            break;
            case PROPERTY_MAP_FRAME_STYLE:
            {
                static SfxItemPropertyMapEntry const aFrameStyleMap   [] =
                {
                    { OUString(UNO_NAME_ANCHOR_PAGE_NO), RES_ANCHOR,            cppu::UnoType<sal_Int16>::get(),           PROPERTY_NONE, MID_ANCHOR_PAGENUM       },
                    { OUString(UNO_NAME_ANCHOR_TYPE), RES_ANCHOR,           cppu::UnoType<css::text::TextContentAnchorType>::get(),            PROPERTY_NONE, MID_ANCHOR_ANCHORTYPE},
                    { OUString(UNO_NAME_BACK_COLOR), RES_BACKGROUND,            cppu::UnoType<sal_Int32>::get(),           PROPERTY_NONE ,MID_BACK_COLOR        },
                    { OUString(UNO_NAME_BACK_COLOR_R_G_B), RES_BACKGROUND,      cppu::UnoType<sal_Int32>::get(), PROPERTY_NONE ,MID_BACK_COLOR_R_G_B},
                    { OUString(UNO_NAME_BACK_COLOR_TRANSPARENCY), RES_BACKGROUND,      cppu::UnoType<sal_Int8>::get(), PROPERTY_NONE ,MID_BACK_COLOR_TRANSPARENCY},
                    { OUString(UNO_NAME_FRAME_INTEROP_GRAB_BAG), RES_FRMATR_GRABBAG, cppu::UnoType< cppu::UnoSequenceType<css::beans::PropertyValue> >::get(), PROPERTY_NONE, 0},
                //  { OUString(UNO_NAME_CHAIN_NEXT_NAME), RES_CHAIN,                cppu::UnoType<OUString>::get(),            PROPERTY_NONE ,MID_CHAIN_NEXTNAME},
                //  { OUString(UNO_NAME_CHAIN_PREV_NAME), RES_CHAIN,                cppu::UnoType<OUString>::get(),            PROPERTY_NONE ,MID_CHAIN_PREVNAME},
                /*not impl*/    { OUString(UNO_NAME_CLIENT_MAP), RES_URL,               cppu::UnoType<bool>::get(),         PROPERTY_NONE ,MID_URL_CLIENTMAP         },
                    { OUString(UNO_NAME_CONTENT_PROTECTED), RES_PROTECT,            cppu::UnoType<bool>::get(),             PROPERTY_NONE, MID_PROTECT_CONTENT   },
                    { OUString(UNO_NAME_EDIT_IN_READONLY), RES_EDIT_IN_READONLY,    cppu::UnoType<bool>::get(),         PROPERTY_NONE, 0},
                //  { OUString(UNO_NAME_GRAPHIC), RES_BACKGROUND,       &,                              PROPERTY_NONE, MID_GRAPHIC
                    { OUString(UNO_NAME_BACK_GRAPHIC_URL), RES_BACKGROUND,      cppu::UnoType<OUString>::get(), PROPERTY_NONE ,MID_GRAPHIC_URL    },
                    { OUString(UNO_NAME_BACK_GRAPHIC_FILTER), RES_BACKGROUND,       cppu::UnoType<OUString>::get(), PROPERTY_NONE ,MID_GRAPHIC_FILTER    },
                    { OUString(UNO_NAME_BACK_GRAPHIC_LOCATION), RES_BACKGROUND,         cppu::UnoType<css::style::GraphicLocation>::get(),          PROPERTY_NONE ,MID_GRAPHIC_POSITION},
                    // #i50322# - add missing map entry for transparency of graphic background
                    { OUString(UNO_NAME_BACK_GRAPHIC_TRANSPARENCY), RES_BACKGROUND, cppu::UnoType<sal_Int8>::get(), PROPERTY_NONE ,MID_GRAPHIC_TRANSPARENCY},
                    { OUString(UNO_NAME_LEFT_MARGIN), RES_LR_SPACE,             cppu::UnoType<sal_Int32>::get(), PROPERTY_NONE, MID_L_MARGIN|CONVERT_TWIPS},
                    { OUString(UNO_NAME_RIGHT_MARGIN), RES_LR_SPACE,            cppu::UnoType<sal_Int32>::get(), PROPERTY_NONE, MID_R_MARGIN|CONVERT_TWIPS},
                    { OUString(UNO_NAME_HORI_ORIENT), RES_HORI_ORIENT,      cppu::UnoType<sal_Int16>::get(),           PROPERTY_NONE ,MID_HORIORIENT_ORIENT    },
                    { OUString(UNO_NAME_HORI_ORIENT_POSITION), RES_HORI_ORIENT,     cppu::UnoType<sal_Int32>::get(),           PROPERTY_NONE ,MID_HORIORIENT_POSITION|CONVERT_TWIPS    },
                    { OUString(UNO_NAME_HORI_ORIENT_RELATION), RES_HORI_ORIENT,     cppu::UnoType<sal_Int16>::get(),           PROPERTY_NONE ,MID_HORIORIENT_RELATION  },
                    { OUString(UNO_NAME_HYPER_LINK_U_R_L), RES_URL,                 cppu::UnoType<OUString>::get(), PROPERTY_NONE ,MID_URL_URL},
                    { OUString(UNO_NAME_HYPER_LINK_TARGET), RES_URL,                cppu::UnoType<OUString>::get(), PROPERTY_NONE ,MID_URL_TARGET},
                    { OUString(UNO_NAME_HYPER_LINK_NAME), RES_URL,              cppu::UnoType<OUString>::get(),            PROPERTY_NONE ,MID_URL_HYPERLINKNAME     },
                    { OUString(UNO_NAME_OPAQUE), RES_OPAQUE,            cppu::UnoType<bool>::get(),         PROPERTY_NONE, 0},
                    { OUString(UNO_NAME_PAGE_TOGGLE), RES_HORI_ORIENT,      cppu::UnoType<bool>::get(),         PROPERTY_NONE ,MID_HORIORIENT_PAGETOGGLE },
                    { OUString(UNO_NAME_POSITION_PROTECTED), RES_PROTECT,           cppu::UnoType<bool>::get(),             PROPERTY_NONE, MID_PROTECT_POSITION},
                    { OUString(UNO_NAME_PRINT), RES_PRINT,              cppu::UnoType<bool>::get(),         PROPERTY_NONE, 0},
                    { OUString(UNO_NAME_WIDTH), RES_FRM_SIZE,           cppu::UnoType<sal_Int32>::get()  ,         PROPERTY_NONE, MID_FRMSIZE_WIDTH|CONVERT_TWIPS          },
                    { OUString(UNO_NAME_HEIGHT), RES_FRM_SIZE,          cppu::UnoType<sal_Int32>::get()  ,         PROPERTY_NONE, MID_FRMSIZE_HEIGHT|CONVERT_TWIPS         },
                    { OUString(UNO_NAME_RELATIVE_HEIGHT), RES_FRM_SIZE,         cppu::UnoType<sal_Int16>::get()  ,         PROPERTY_NONE,   MID_FRMSIZE_REL_HEIGHT },
                    { OUString(UNO_NAME_RELATIVE_HEIGHT_RELATION), RES_FRM_SIZE, cppu::UnoType<sal_Int16>::get(),          PROPERTY_NONE,   MID_FRMSIZE_REL_HEIGHT_RELATION  },
                    { OUString(UNO_NAME_RELATIVE_WIDTH), RES_FRM_SIZE,          cppu::UnoType<sal_Int16>::get()  ,         PROPERTY_NONE,   MID_FRMSIZE_REL_WIDTH  },
                    { OUString(UNO_NAME_RELATIVE_WIDTH_RELATION), RES_FRM_SIZE,          cppu::UnoType<sal_Int16>::get()  ,         PROPERTY_NONE,   MID_FRMSIZE_REL_WIDTH_RELATION  },
                    { OUString(UNO_NAME_SIZE_TYPE), RES_FRM_SIZE,           cppu::UnoType<sal_Int16>::get()  ,         PROPERTY_NONE,   MID_FRMSIZE_SIZE_TYPE  },
                    { OUString(UNO_NAME_WIDTH_TYPE), RES_FRM_SIZE,          cppu::UnoType<sal_Int16>::get()  ,         PROPERTY_NONE,   MID_FRMSIZE_WIDTH_TYPE },
                    { OUString(UNO_NAME_SIZE), RES_FRM_SIZE,            cppu::UnoType<css::awt::Size>::get(),             PROPERTY_NONE, MID_FRMSIZE_SIZE|CONVERT_TWIPS},
                    { OUString(UNO_NAME_IS_SYNC_WIDTH_TO_HEIGHT), RES_FRM_SIZE,         cppu::UnoType<bool>::get()  ,       PROPERTY_NONE,   MID_FRMSIZE_IS_SYNC_WIDTH_TO_HEIGHT    },
                    { OUString(UNO_NAME_IS_SYNC_HEIGHT_TO_WIDTH), RES_FRM_SIZE,         cppu::UnoType<bool>::get()  ,       PROPERTY_NONE,   MID_FRMSIZE_IS_SYNC_HEIGHT_TO_WIDTH },
                //  { OUString(UNO_NAME_WIDTH), RES_FRM_SIZE,           cppu::UnoType<sal_Int32>::get()  ,         PROPERTY_NONE, MID_FRMSIZE_WIDTH            },
                    { OUString(UNO_NAME_SHADOW_FORMAT), RES_SHADOW,             cppu::UnoType<css::table::ShadowFormat>::get(),   PROPERTY_NONE, CONVERT_TWIPS},
                    { OUString(UNO_NAME_SHADOW_TRANSPARENCE), RES_SHADOW,       cppu::UnoType<sal_Int16>::get(),       PROPERTY_NONE, MID_SHADOW_TRANSPARENCE},
                    { OUString(UNO_NAME_SERVER_MAP), RES_URL,               cppu::UnoType<bool>::get(),         PROPERTY_NONE ,MID_URL_SERVERMAP         },
                    { OUString(UNO_NAME_SIZE_PROTECTED), RES_PROTECT,           cppu::UnoType<bool>::get(),             PROPERTY_NONE, MID_PROTECT_SIZE    },
                    // We keep Surround, as we delivered it with 5.1, although it's identical to text::WrapTextMode
                    { OUString(UNO_NAME_SURROUND), RES_SURROUND,          cppu::UnoType<css::text::WrapTextMode>::get(),    PROPERTY_NONE, MID_SURROUND_SURROUNDTYPE },
                    { OUString(UNO_NAME_TEXT_WRAP), RES_SURROUND,           cppu::UnoType<css::text::WrapTextMode>::get(),             PROPERTY_NONE, MID_SURROUND_SURROUNDTYPE    },
                    { OUString(UNO_NAME_SURROUND_ANCHORONLY), RES_SURROUND,             cppu::UnoType<bool>::get(),             PROPERTY_NONE, MID_SURROUND_ANCHORONLY      },
                    { OUString(UNO_NAME_SURROUND_CONTOUR), RES_SURROUND,            cppu::UnoType<bool>::get(),             PROPERTY_NONE, MID_SURROUND_CONTOUR         },
                    { OUString(UNO_NAME_CONTOUR_OUTSIDE), RES_SURROUND,             cppu::UnoType<bool>::get(),             PROPERTY_NONE, MID_SURROUND_CONTOUROUTSIDE  },
                    { OUString(UNO_NAME_TEXT_COLUMNS), RES_COL,                cppu::UnoType<css::text::XTextColumns>::get(),    PROPERTY_NONE, MID_COLUMNS},
                    { OUString(UNO_NAME_TOP_MARGIN), RES_UL_SPACE,          cppu::UnoType<sal_Int32>::get(), PROPERTY_NONE, MID_UP_MARGIN|CONVERT_TWIPS},
                    { OUString(UNO_NAME_BOTTOM_MARGIN), RES_UL_SPACE,           cppu::UnoType<sal_Int32>::get(), PROPERTY_NONE, MID_LO_MARGIN|CONVERT_TWIPS},
                    { OUString(UNO_NAME_BACK_TRANSPARENT), RES_BACKGROUND,      cppu::UnoType<bool>::get(),         PROPERTY_NONE ,MID_GRAPHIC_TRANSPARENT       },
                    { OUString(UNO_NAME_VERT_ORIENT), RES_VERT_ORIENT,      cppu::UnoType<sal_Int16>::get(),           PROPERTY_NONE ,MID_VERTORIENT_ORIENT    },
                    { OUString(UNO_NAME_VERT_ORIENT_POSITION), RES_VERT_ORIENT,     cppu::UnoType<sal_Int32>::get(),           PROPERTY_NONE ,MID_VERTORIENT_POSITION|CONVERT_TWIPS    },
                    { OUString(UNO_NAME_VERT_ORIENT_RELATION), RES_VERT_ORIENT,     cppu::UnoType<sal_Int16>::get(),           PROPERTY_NONE ,MID_VERTORIENT_RELATION  },
                    { OUString(UNO_NAME_LEFT_BORDER), RES_BOX,              cppu::UnoType<css::table::BorderLine>::get(),  0, LEFT_BORDER  |CONVERT_TWIPS },
                    { OUString(UNO_NAME_RIGHT_BORDER), RES_BOX,             cppu::UnoType<css::table::BorderLine>::get(),  0, RIGHT_BORDER |CONVERT_TWIPS },
                    { OUString(UNO_NAME_TOP_BORDER), RES_BOX,               cppu::UnoType<css::table::BorderLine>::get(),  0, TOP_BORDER   |CONVERT_TWIPS },
                    { OUString(UNO_NAME_BOTTOM_BORDER), RES_BOX,                cppu::UnoType<css::table::BorderLine>::get(),  0, BOTTOM_BORDER|CONVERT_TWIPS },
                    { OUString(UNO_NAME_BORDER_DISTANCE), RES_BOX,    cppu::UnoType<sal_Int32>::get(), 0, BORDER_DISTANCE|CONVERT_TWIPS },
                    { OUString(UNO_NAME_LEFT_BORDER_DISTANCE), RES_BOX,             cppu::UnoType<sal_Int32>::get(),   0, LEFT_BORDER_DISTANCE  |CONVERT_TWIPS },
                    { OUString(UNO_NAME_RIGHT_BORDER_DISTANCE), RES_BOX,                cppu::UnoType<sal_Int32>::get(),   0, RIGHT_BORDER_DISTANCE |CONVERT_TWIPS },
                    { OUString(UNO_NAME_TOP_BORDER_DISTANCE), RES_BOX,              cppu::UnoType<sal_Int32>::get(),   0, TOP_BORDER_DISTANCE   |CONVERT_TWIPS },
                    { OUString(UNO_NAME_BOTTOM_BORDER_DISTANCE), RES_BOX,               cppu::UnoType<sal_Int32>::get(),   0, BOTTOM_BORDER_DISTANCE|CONVERT_TWIPS },
                    { OUString(UNO_NAME_USER_DEFINED_ATTRIBUTES), RES_UNKNOWNATR_CONTAINER, cppu::UnoType<css::container::XNameContainer>::get(), PropertyAttribute::MAYBEVOID, 0 },
                    { OUString(UNO_NAME_IS_PHYSICAL), FN_UNO_IS_PHYSICAL,     cppu::UnoType<bool>::get(), PropertyAttribute::READONLY, 0},
                    { OUString(UNO_NAME_IS_AUTO_UPDATE), FN_UNO_IS_AUTO_UPDATE, cppu::UnoType<bool>::get(), PROPERTY_NONE, 0},
                    { OUString(UNO_NAME_DISPLAY_NAME), FN_UNO_DISPLAY_NAME, cppu::UnoType<OUString>::get(), PropertyAttribute::READONLY, 0},
                    // #i18732#
                    { OUString(UNO_NAME_IS_FOLLOWING_TEXT_FLOW), RES_FOLLOW_TEXT_FLOW,     cppu::UnoType<bool>::get(), PROPERTY_NONE, 0},
                    // #i28701#
                    { OUString(UNO_NAME_WRAP_INFLUENCE_ON_POSITION), RES_WRAP_INFLUENCE_ON_OBJPOS, cppu::UnoType<sal_Int8>::get(), PROPERTY_NONE, MID_WRAP_INFLUENCE},
                    { OUString(UNO_NAME_WRITING_MODE), RES_FRAMEDIR, cppu::UnoType<sal_Int16>::get(), PROPERTY_NONE, 0 },
                    { OUString(UNO_NAME_HIDDEN), FN_UNO_HIDDEN,     cppu::UnoType<bool>::get(), PROPERTY_NONE, 0},
                    { OUString(UNO_NAME_TEXT_VERT_ADJUST), RES_TEXT_VERT_ADJUST, cppu::UnoType<css::drawing::TextVerticalAdjust>::get(), PROPERTY_NONE ,0},

                    //UUUU added FillProperties for SW, same as FILL_PROPERTIES in svx
                    // but need own defines in Writer due to later association of strings
                    // and uno types (see loop at end of this method and definition of SW_PROP_NMID)
                    // This entry is for adding that properties to style import/export
                    FILL_PROPERTIES_SW

                    { OUString(), 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aFrameStyleMap;
            }
            break;
            case PROPERTY_MAP_PAGE_STYLE :
            {
                static SfxItemPropertyMapEntry const aPageStyleMap   [] =
                {
                    { OUString(UNO_NAME_BACK_COLOR), RES_BACKGROUND,            cppu::UnoType<sal_Int32>::get(),           PROPERTY_NONE ,MID_BACK_COLOR        },
                //  { OUString(UNO_NAME_GRAPHIC), RES_BACKGROUND,       &,                              PROPERTY_NONE, MID_GRAPHIC
                    { OUString(UNO_NAME_BACK_GRAPHIC_URL), RES_BACKGROUND,      cppu::UnoType<OUString>::get(), PROPERTY_NONE ,MID_GRAPHIC_URL    },
                    { OUString(UNO_NAME_BACK_GRAPHIC_FILTER), RES_BACKGROUND,       cppu::UnoType<OUString>::get(), PROPERTY_NONE ,MID_GRAPHIC_FILTER    },
                    { OUString(UNO_NAME_BACK_GRAPHIC_LOCATION), RES_BACKGROUND,         cppu::UnoType<css::style::GraphicLocation>::get(), PROPERTY_NONE ,MID_GRAPHIC_POSITION},
                    { OUString(UNO_NAME_LEFT_MARGIN), RES_LR_SPACE,             cppu::UnoType<sal_Int32>::get(), PROPERTY_NONE, MID_L_MARGIN|CONVERT_TWIPS},
                    { OUString(UNO_NAME_RIGHT_MARGIN), RES_LR_SPACE,            cppu::UnoType<sal_Int32>::get(), PROPERTY_NONE, MID_R_MARGIN|CONVERT_TWIPS},
                    { OUString(UNO_NAME_BACK_TRANSPARENT), RES_BACKGROUND,      cppu::UnoType<bool>::get(),         PROPERTY_NONE ,MID_GRAPHIC_TRANSPARENT       },
                    { OUString(UNO_NAME_LEFT_BORDER), RES_BOX,              cppu::UnoType<css::table::BorderLine>::get(),  0, LEFT_BORDER  |CONVERT_TWIPS },
                    { OUString(UNO_NAME_RIGHT_BORDER), RES_BOX,             cppu::UnoType<css::table::BorderLine>::get(),  0, RIGHT_BORDER |CONVERT_TWIPS },
                    { OUString(UNO_NAME_TOP_BORDER), RES_BOX,               cppu::UnoType<css::table::BorderLine>::get(),  0, TOP_BORDER   |CONVERT_TWIPS },
                    { OUString(UNO_NAME_BOTTOM_BORDER), RES_BOX,                cppu::UnoType<css::table::BorderLine>::get(),  0, BOTTOM_BORDER|CONVERT_TWIPS },
                    { OUString(UNO_NAME_BORDER_DISTANCE), RES_BOX,    cppu::UnoType<sal_Int32>::get(), 0, BORDER_DISTANCE|CONVERT_TWIPS },
                    { OUString(UNO_NAME_LEFT_BORDER_DISTANCE), RES_BOX,             cppu::UnoType<sal_Int32>::get(),   0, LEFT_BORDER_DISTANCE  |CONVERT_TWIPS },
                    { OUString(UNO_NAME_RIGHT_BORDER_DISTANCE), RES_BOX,                cppu::UnoType<sal_Int32>::get(),   0, RIGHT_BORDER_DISTANCE |CONVERT_TWIPS },
                    { OUString(UNO_NAME_TOP_BORDER_DISTANCE), RES_BOX,              cppu::UnoType<sal_Int32>::get(),   0, TOP_BORDER_DISTANCE   |CONVERT_TWIPS },
                    { OUString(UNO_NAME_BOTTOM_BORDER_DISTANCE), RES_BOX,               cppu::UnoType<sal_Int32>::get(),   0, BOTTOM_BORDER_DISTANCE|CONVERT_TWIPS },
                    { OUString(UNO_NAME_SHADOW_FORMAT), RES_SHADOW,             cppu::UnoType<css::table::ShadowFormat>::get(),   PROPERTY_NONE, CONVERT_TWIPS},
                    { OUString(UNO_NAME_SHADOW_TRANSPARENCE), RES_SHADOW,       cppu::UnoType<sal_Int16>::get(),       PROPERTY_NONE, MID_SHADOW_TRANSPARENCE},

                    //UUU use real WhichIDs for Header, no longer use extra-defined WhichIDs which make handling harder as needed.
                    // The implementation will decide if these are part of Header/Footer or PageStyle depending on the SlotName,
                    // more precisely on the first characters. Thus it is necessary that these are 'Header' for the Header slots
                    { OUString(UNO_NAME_HEADER_BACK_COLOR), RES_BACKGROUND,   cppu::UnoType<sal_Int32>::get(),           PROPERTY_NONE ,MID_BACK_COLOR        },
                //  { OUString(UNO_NAME_HEADER_GRAPHIC), RES_BACKGROUND,  &,                              PROPERTY_NONE, MID_GRAPHIC
                    { OUString(UNO_NAME_HEADER_GRAPHIC_URL), RES_BACKGROUND,          cppu::UnoType<OUString>::get(), PROPERTY_NONE ,MID_GRAPHIC_URL    },
                    { OUString(UNO_NAME_HEADER_GRAPHIC_FILTER), RES_BACKGROUND,           cppu::UnoType<OUString>::get(), PROPERTY_NONE ,MID_GRAPHIC_FILTER    },
                    { OUString(UNO_NAME_HEADER_GRAPHIC_LOCATION), RES_BACKGROUND,     cppu::UnoType<css::style::GraphicLocation>::get(), PROPERTY_NONE ,MID_GRAPHIC_POSITION},
                    { OUString(UNO_NAME_HEADER_LEFT_MARGIN), RES_LR_SPACE,    cppu::UnoType<sal_Int32>::get(), PROPERTY_NONE, MID_L_MARGIN|CONVERT_TWIPS},
                    { OUString(UNO_NAME_HEADER_RIGHT_MARGIN), RES_LR_SPACE,   cppu::UnoType<sal_Int32>::get(), PROPERTY_NONE, MID_R_MARGIN|CONVERT_TWIPS},
                    { OUString(UNO_NAME_HEADER_BACK_TRANSPARENT), RES_BACKGROUND,     cppu::UnoType<bool>::get(),         PROPERTY_NONE ,MID_GRAPHIC_TRANSPARENT       },
                    { OUString(UNO_NAME_HEADER_LEFT_BORDER), RES_BOX,             cppu::UnoType<css::table::BorderLine>::get(),  0, LEFT_BORDER  |CONVERT_TWIPS },
                    { OUString(UNO_NAME_HEADER_RIGHT_BORDER), RES_BOX,                cppu::UnoType<css::table::BorderLine>::get(),  0, RIGHT_BORDER |CONVERT_TWIPS },
                    { OUString(UNO_NAME_HEADER_TOP_BORDER), RES_BOX,              cppu::UnoType<css::table::BorderLine>::get(),  0, TOP_BORDER   |CONVERT_TWIPS },
                    { OUString(UNO_NAME_HEADER_BOTTOM_BORDER), RES_BOX,               cppu::UnoType<css::table::BorderLine>::get(),  0, BOTTOM_BORDER|CONVERT_TWIPS },
                    { OUString(UNO_NAME_HEADER_BORDER_DISTANCE), RES_BOX,    cppu::UnoType<sal_Int32>::get(),    PropertyAttribute::MAYBEVOID, BORDER_DISTANCE|CONVERT_TWIPS },
                    { OUString(UNO_NAME_HEADER_LEFT_BORDER_DISTANCE), RES_BOX,                cppu::UnoType<sal_Int32>::get(),   0, LEFT_BORDER_DISTANCE  |CONVERT_TWIPS },
                    { OUString(UNO_NAME_HEADER_RIGHT_BORDER_DISTANCE), RES_BOX,               cppu::UnoType<sal_Int32>::get(),   0, RIGHT_BORDER_DISTANCE |CONVERT_TWIPS },
                    { OUString(UNO_NAME_HEADER_TOP_BORDER_DISTANCE), RES_BOX,             cppu::UnoType<sal_Int32>::get(),   0, TOP_BORDER_DISTANCE   |CONVERT_TWIPS },
                    { OUString(UNO_NAME_HEADER_BOTTOM_BORDER_DISTANCE), RES_BOX,              cppu::UnoType<sal_Int32>::get(),   0, BOTTOM_BORDER_DISTANCE|CONVERT_TWIPS },
                    { OUString(UNO_NAME_HEADER_SHADOW_FORMAT), RES_SHADOW,        cppu::UnoType<css::table::ShadowFormat>::get(),   PROPERTY_NONE, CONVERT_TWIPS},
                    { OUString(UNO_NAME_HEADER_BODY_DISTANCE), RES_UL_SPACE,    cppu::UnoType<sal_Int32>::get(),            PROPERTY_NONE ,MID_LO_MARGIN|CONVERT_TWIPS       },
                    { OUString(UNO_NAME_HEADER_IS_DYNAMIC_HEIGHT), SID_ATTR_PAGE_DYNAMIC,   cppu::UnoType<bool>::get(),            PROPERTY_NONE ,0         },
                    { OUString(UNO_NAME_HEADER_IS_SHARED), SID_ATTR_PAGE_SHARED,    cppu::UnoType<bool>::get(),          PROPERTY_NONE ,0         },
                    { OUString(UNO_NAME_HEADER_HEIGHT), SID_ATTR_PAGE_SIZE,       cppu::UnoType<sal_Int32>::get(),           PROPERTY_NONE ,MID_SIZE_HEIGHT|CONVERT_TWIPS         },
                    { OUString(UNO_NAME_HEADER_IS_ON), SID_ATTR_PAGE_ON,            cppu::UnoType<bool>::get(),         PROPERTY_NONE ,0         },
                    { OUString(UNO_NAME_HEADER_DYNAMIC_SPACING), RES_HEADER_FOOTER_EAT_SPACING,            cppu::UnoType<bool>::get(),         PropertyAttribute::MAYBEVOID ,0         },


                    { OUString(UNO_NAME_FIRST_IS_SHARED), SID_ATTR_PAGE_SHARED_FIRST,   cppu::UnoType<bool>::get(), PROPERTY_NONE, 0 },

                    //UUU use real WhichIDs for Footer, see Header (above) for more infos
                    { OUString(UNO_NAME_FOOTER_BACK_COLOR), RES_BACKGROUND,   cppu::UnoType<sal_Int32>::get(),           PROPERTY_NONE ,MID_BACK_COLOR        },
                //  { OUString(UNO_NAME_FOOTER_GRAPHIC), RES_BACKGROUND,      &,                              PROPERTY_NONE, MID_GRAPHIC
                    { OUString(UNO_NAME_FOOTER_GRAPHIC_URL), RES_BACKGROUND,      cppu::UnoType<OUString>::get(), PROPERTY_NONE ,MID_GRAPHIC_URL    },
                    { OUString(UNO_NAME_FOOTER_GRAPHIC_FILTER), RES_BACKGROUND,       cppu::UnoType<OUString>::get(), PROPERTY_NONE ,MID_GRAPHIC_FILTER    },
                    { OUString(UNO_NAME_FOOTER_GRAPHIC_LOCATION), RES_BACKGROUND,     cppu::UnoType<css::style::GraphicLocation>::get(), PROPERTY_NONE ,MID_GRAPHIC_POSITION},
                    { OUString(UNO_NAME_FOOTER_LEFT_MARGIN), RES_LR_SPACE,    cppu::UnoType<sal_Int32>::get(), PROPERTY_NONE, MID_L_MARGIN|CONVERT_TWIPS},
                    { OUString(UNO_NAME_FOOTER_RIGHT_MARGIN), RES_LR_SPACE,   cppu::UnoType<sal_Int32>::get(), PROPERTY_NONE, MID_R_MARGIN|CONVERT_TWIPS},
                    { OUString(UNO_NAME_FOOTER_BACK_TRANSPARENT), RES_BACKGROUND,     cppu::UnoType<bool>::get(),         PROPERTY_NONE ,MID_GRAPHIC_TRANSPARENT       },
                    { OUString(UNO_NAME_FOOTER_LEFT_BORDER), RES_BOX,             cppu::UnoType<css::table::BorderLine>::get(),  0, LEFT_BORDER  |CONVERT_TWIPS },
                    { OUString(UNO_NAME_FOOTER_RIGHT_BORDER), RES_BOX,                cppu::UnoType<css::table::BorderLine>::get(),  0, RIGHT_BORDER |CONVERT_TWIPS },
                    { OUString(UNO_NAME_FOOTER_TOP_BORDER), RES_BOX,              cppu::UnoType<css::table::BorderLine>::get(),  0, TOP_BORDER   |CONVERT_TWIPS },
                    { OUString(UNO_NAME_FOOTER_BOTTOM_BORDER), RES_BOX,               cppu::UnoType<css::table::BorderLine>::get(),  0, BOTTOM_BORDER|CONVERT_TWIPS },
                    { OUString(UNO_NAME_FOOTER_BORDER_DISTANCE), RES_BOX,    cppu::UnoType<sal_Int32>::get(),    PropertyAttribute::MAYBEVOID, BORDER_DISTANCE|CONVERT_TWIPS },
                    { OUString(UNO_NAME_FOOTER_LEFT_BORDER_DISTANCE), RES_BOX,                cppu::UnoType<sal_Int32>::get(),   0, LEFT_BORDER_DISTANCE  |CONVERT_TWIPS },
                    { OUString(UNO_NAME_FOOTER_RIGHT_BORDER_DISTANCE), RES_BOX,               cppu::UnoType<sal_Int32>::get(),   0, RIGHT_BORDER_DISTANCE |CONVERT_TWIPS },
                    { OUString(UNO_NAME_FOOTER_TOP_BORDER_DISTANCE), RES_BOX,             cppu::UnoType<sal_Int32>::get(),   0, TOP_BORDER_DISTANCE   |CONVERT_TWIPS },
                    { OUString(UNO_NAME_FOOTER_BOTTOM_BORDER_DISTANCE), RES_BOX,              cppu::UnoType<sal_Int32>::get(),   0, BOTTOM_BORDER_DISTANCE|CONVERT_TWIPS },
                    { OUString(UNO_NAME_FOOTER_SHADOW_FORMAT), RES_SHADOW,        cppu::UnoType<css::table::ShadowFormat>::get(),   PROPERTY_NONE, CONVERT_TWIPS},
                    { OUString(UNO_NAME_FOOTER_BODY_DISTANCE), RES_UL_SPACE,    cppu::UnoType<sal_Int32>::get(),            PROPERTY_NONE ,MID_UP_MARGIN|CONVERT_TWIPS       },
                    { OUString(UNO_NAME_FOOTER_IS_DYNAMIC_HEIGHT), SID_ATTR_PAGE_DYNAMIC,   cppu::UnoType<bool>::get(),            PROPERTY_NONE ,0         },
                    { OUString(UNO_NAME_FOOTER_IS_SHARED), SID_ATTR_PAGE_SHARED,    cppu::UnoType<bool>::get(),          PROPERTY_NONE ,0         },
                    { OUString(UNO_NAME_FOOTER_HEIGHT), SID_ATTR_PAGE_SIZE,       cppu::UnoType<sal_Int32>::get(),           PROPERTY_NONE ,MID_SIZE_HEIGHT|CONVERT_TWIPS         },
                    { OUString(UNO_NAME_FOOTER_IS_ON), SID_ATTR_PAGE_ON,            cppu::UnoType<bool>::get(),         PROPERTY_NONE ,0         },
                    { OUString(UNO_NAME_FOOTER_DYNAMIC_SPACING), RES_HEADER_FOOTER_EAT_SPACING,            cppu::UnoType<bool>::get(),         PropertyAttribute::MAYBEVOID ,0         },

                    { OUString(UNO_NAME_IS_LANDSCAPE), SID_ATTR_PAGE,           cppu::UnoType<bool>::get(),             PROPERTY_NONE ,MID_PAGE_ORIENTATION   },
                    { OUString(UNO_NAME_NUMBERING_TYPE), SID_ATTR_PAGE,             cppu::UnoType<sal_Int16>::get(),           PROPERTY_NONE , MID_PAGE_NUMTYPE       },
                    { OUString(UNO_NAME_PAGE_STYLE_LAYOUT), SID_ATTR_PAGE,          cppu::UnoType<css::style::PageStyleLayout>::get(),    PROPERTY_NONE ,MID_PAGE_LAYOUT     },
                    { OUString(UNO_NAME_PRINTER_PAPER_TRAY), RES_PAPER_BIN,             cppu::UnoType<OUString>::get(),            PROPERTY_NONE , 0 },
//                  { OUString(UNO_NAME_REGISTER_MODE_ACTIVE), SID_SWREGISTER_MODE,     cppu::UnoType<bool>::get(),             PROPERTY_NONE , 0 },
                    { OUString(UNO_NAME_REGISTER_PARAGRAPH_STYLE), SID_SWREGISTER_COLLECTION,   cppu::UnoType<OUString>::get(),        PROPERTY_NONE , 0 },
                    { OUString(UNO_NAME_SIZE), SID_ATTR_PAGE_SIZE,  cppu::UnoType<css::awt::Size>::get(),             PROPERTY_NONE,   MID_SIZE_SIZE|CONVERT_TWIPS},
                    { OUString(UNO_NAME_WIDTH), SID_ATTR_PAGE_SIZE,     cppu::UnoType<sal_Int32>::get()  ,         PROPERTY_NONE, MID_SIZE_WIDTH|CONVERT_TWIPS},
                    { OUString(UNO_NAME_HEIGHT), SID_ATTR_PAGE_SIZE,    cppu::UnoType<sal_Int32>::get()  ,         PROPERTY_NONE, MID_SIZE_HEIGHT|CONVERT_TWIPS            },
                    { OUString(UNO_NAME_TEXT_VERT_ADJUST), RES_TEXT_VERT_ADJUST, cppu::UnoType<css::drawing::TextVerticalAdjust>::get(), PROPERTY_NONE, 0 },
                    { OUString(UNO_NAME_TEXT_COLUMNS), RES_COL,                cppu::UnoType<css::text::XTextColumns>::get(),    PROPERTY_NONE, MID_COLUMNS},
                    { OUString(UNO_NAME_TOP_MARGIN), RES_UL_SPACE,          cppu::UnoType<sal_Int32>::get(), PROPERTY_NONE, MID_UP_MARGIN|CONVERT_TWIPS},
                    { OUString(UNO_NAME_BOTTOM_MARGIN), RES_UL_SPACE,           cppu::UnoType<sal_Int32>::get(), PROPERTY_NONE, MID_LO_MARGIN|CONVERT_TWIPS},
                    { OUString(UNO_NAME_HEADER_TEXT), FN_UNO_HEADER,        cppu::UnoType<css::text::XText>::get(),         PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY, 0},
                    { OUString(UNO_NAME_HEADER_TEXT_LEFT), FN_UNO_HEADER_LEFT,     cppu::UnoType<css::text::XText>::get(),          PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY, 0},
                    { OUString(UNO_NAME_HEADER_TEXT_RIGHT), FN_UNO_HEADER_RIGHT,    cppu::UnoType<css::text::XText>::get(),         PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY, 0},
                    { OUString(UNO_NAME_HEADER_TEXT_FIRST), FN_UNO_HEADER_FIRST,    cppu::UnoType<css::text::XText>::get(),         PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY, 0},
                    { OUString(UNO_NAME_FOOTER_TEXT), FN_UNO_FOOTER,        cppu::UnoType<css::text::XText>::get(),         PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY, 0},
                    { OUString(UNO_NAME_FOOTER_TEXT_LEFT), FN_UNO_FOOTER_LEFT,  cppu::UnoType<css::text::XText>::get(),         PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY, 0},
                    { OUString(UNO_NAME_FOOTER_TEXT_RIGHT), FN_UNO_FOOTER_RIGHT,    cppu::UnoType<css::text::XText>::get(),         PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY, 0},
                    { OUString(UNO_NAME_FOOTER_TEXT_FIRST), FN_UNO_FOOTER_FIRST,    cppu::UnoType<css::text::XText>::get(),         PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY, 0},
                    { OUString(UNO_NAME_FOLLOW_STYLE), FN_UNO_FOLLOW_STYLE,     cppu::UnoType<OUString>::get(),        PROPERTY_NONE, 0},
                    { OUString(UNO_NAME_USER_DEFINED_ATTRIBUTES), RES_UNKNOWNATR_CONTAINER, cppu::UnoType<css::container::XNameContainer>::get(), PropertyAttribute::MAYBEVOID, 0 },
                    { OUString(UNO_NAME_IS_PHYSICAL), FN_UNO_IS_PHYSICAL,     cppu::UnoType<bool>::get(), PropertyAttribute::READONLY, 0},
                    { OUString(UNO_NAME_DISPLAY_NAME), FN_UNO_DISPLAY_NAME, cppu::UnoType<OUString>::get(), PropertyAttribute::READONLY, 0},
                    { OUString(UNO_NAME_FOOTNOTE_HEIGHT), FN_PARAM_FTN_INFO,        cppu::UnoType<sal_Int32>::get(), PROPERTY_NONE ,       MID_FTN_HEIGHT|CONVERT_TWIPS},
                    { OUString(UNO_NAME_FOOTNOTE_LINE_WEIGHT), FN_PARAM_FTN_INFO,       cppu::UnoType<sal_Int16>::get(),   PROPERTY_NONE ,     MID_LINE_WEIGHT|CONVERT_TWIPS},
                    { OUString(UNO_NAME_FOOTNOTE_LINE_COLOR), FN_PARAM_FTN_INFO,        cppu::UnoType<sal_Int32>::get(),   PROPERTY_NONE ,     MID_LINE_COLOR},
                    { OUString(UNO_NAME_FOOTNOTE_LINE_STYLE), FN_PARAM_FTN_INFO,        cppu::UnoType<sal_Int8>::get(),    PROPERTY_NONE , MID_FTN_LINE_STYLE},
                    { OUString(UNO_NAME_FOOTNOTE_LINE_RELATIVE_WIDTH), FN_PARAM_FTN_INFO,       cppu::UnoType<sal_Int8>::get(),        PROPERTY_NONE ,     MID_LINE_RELWIDTH    },
                    { OUString(UNO_NAME_FOOTNOTE_LINE_ADJUST), FN_PARAM_FTN_INFO,       cppu::UnoType<sal_Int16>::get(),   PROPERTY_NONE ,     MID_LINE_ADJUST     },
                    { OUString(UNO_NAME_FOOTNOTE_LINE_TEXT_DISTANCE), FN_PARAM_FTN_INFO,        cppu::UnoType<sal_Int32>::get(),   PROPERTY_NONE ,     MID_LINE_TEXT_DIST   |CONVERT_TWIPS },
                    { OUString(UNO_NAME_FOOTNOTE_LINE_DISTANCE), FN_PARAM_FTN_INFO,         cppu::UnoType<sal_Int32>::get(),   PROPERTY_NONE ,     MID_LINE_FOOTNOTE_DIST|CONVERT_TWIPS},
                    { OUString(UNO_NAME_WRITING_MODE), RES_FRAMEDIR, cppu::UnoType<sal_Int16>::get(), PROPERTY_NONE, 0 },
                        // writing grid
                    { OUString(UNO_NAME_GRID_COLOR), RES_TEXTGRID, cppu::UnoType<sal_Int32>::get(), PROPERTY_NONE, MID_GRID_COLOR},
                    { OUString(UNO_NAME_GRID_LINES), RES_TEXTGRID, cppu::UnoType<sal_Int16>::get(), PROPERTY_NONE, MID_GRID_LINES},
                    { OUString(UNO_NAME_GRID_BASE_HEIGHT), RES_TEXTGRID, cppu::UnoType<sal_Int32>::get(), PROPERTY_NONE, MID_GRID_BASEHEIGHT|CONVERT_TWIPS},
                    { OUString(UNO_NAME_GRID_RUBY_HEIGHT), RES_TEXTGRID, cppu::UnoType<sal_Int32>::get(), PROPERTY_NONE, MID_GRID_RUBYHEIGHT|CONVERT_TWIPS},
                    { OUString(UNO_NAME_GRID_MODE), RES_TEXTGRID, cppu::UnoType<sal_Int16>::get(), PROPERTY_NONE, MID_GRID_TYPE},
                    { OUString(UNO_NAME_GRID_RUBY_BELOW), RES_TEXTGRID, cppu::UnoType<bool>::get(), PROPERTY_NONE, MID_GRID_RUBY_BELOW},
                    { OUString(UNO_NAME_GRID_PRINT), RES_TEXTGRID, cppu::UnoType<bool>::get(), PROPERTY_NONE, MID_GRID_PRINT},
                    { OUString(UNO_NAME_GRID_DISPLAY), RES_TEXTGRID, cppu::UnoType<bool>::get(), PROPERTY_NONE, MID_GRID_DISPLAY},
                    { OUString(UNO_NAME_GRID_BASE_WIDTH), RES_TEXTGRID, cppu::UnoType<sal_Int32>::get(), PROPERTY_NONE, MID_GRID_BASEWIDTH|CONVERT_TWIPS},
                    { OUString(UNO_NAME_GRID_SNAP_TO_CHARS), RES_TEXTGRID, cppu::UnoType<bool>::get(), PROPERTY_NONE, MID_GRID_SNAPTOCHARS},
                    { OUString(UNO_NAME_GRID_STANDARD_PAGE_MODE), RES_TEXTGRID, cppu::UnoType<bool>::get(), PROPERTY_NONE, MID_GRID_STANDARD_MODE},
                    { OUString(UNO_NAME_HIDDEN), FN_UNO_HIDDEN,     cppu::UnoType<bool>::get(), PROPERTY_NONE, 0},

                    //UUUU added FillProperties for SW, same as FILL_PROPERTIES in svx
                    // but need own defines in Writer due to later association of strings
                    // and uno types (see loop at end of this method and definition of SW_PROP_NMID)
                    // This entry is for adding that properties to style import/export
                    FILL_PROPERTIES_SW

                    //UUUU Added DrawingLayer FillStyle Properties for Header. These need an own unique name,
                    // but reuse the same WhichIDs as the regular fill. The implementation will decide to which
                    // group of fill properties it belongs based on the start of the name (was already done in
                    // the implementation partially), thus all SlotNames *have* to start with 'Header'
                    { OUString(UNO_NAME_HEADER_FILLBMP_LOGICAL_SIZE),           XATTR_FILLBMP_SIZELOG,          cppu::UnoType<bool>::get() ,        0,  0},
                    { OUString(UNO_NAME_HEADER_FILLBMP_OFFSET_X),               XATTR_FILLBMP_TILEOFFSETX,      cppu::UnoType<sal_Int32>::get() ,          0,  0},
                    { OUString(UNO_NAME_HEADER_FILLBMP_OFFSET_Y),               XATTR_FILLBMP_TILEOFFSETY,      cppu::UnoType<sal_Int32>::get() ,          0,  0},
                    { OUString(UNO_NAME_HEADER_FILLBMP_POSITION_OFFSET_X),      XATTR_FILLBMP_POSOFFSETX,       cppu::UnoType<sal_Int32>::get() ,          0,  0},
                    { OUString(UNO_NAME_HEADER_FILLBMP_POSITION_OFFSET_Y),      XATTR_FILLBMP_POSOFFSETY,       cppu::UnoType<sal_Int32>::get() ,          0,  0},
                    { OUString(UNO_NAME_HEADER_FILLBMP_RECTANGLE_POINT),        XATTR_FILLBMP_POS,              cppu::UnoType<css::drawing::RectanglePoint>::get() , 0,  0},
                    { OUString(UNO_NAME_HEADER_FILLBMP_SIZE_X),                 XATTR_FILLBMP_SIZEX,            cppu::UnoType<sal_Int32>::get() ,          0,  SFX_METRIC_ITEM},
                    { OUString(UNO_NAME_HEADER_FILLBMP_SIZE_Y),                 XATTR_FILLBMP_SIZEY,            cppu::UnoType<sal_Int32>::get() ,          0,  SFX_METRIC_ITEM},
                    { OUString(UNO_NAME_HEADER_FILLBMP_STRETCH),                XATTR_FILLBMP_STRETCH,          cppu::UnoType<bool>::get() ,        0,  0},
                    { OUString(UNO_NAME_HEADER_FILLBMP_TILE),                   XATTR_FILLBMP_TILE,             cppu::UnoType<bool>::get() ,        0,  0},
                    { OUString(UNO_NAME_HEADER_FILLBMP_MODE),                   OWN_ATTR_FILLBMP_MODE,          cppu::UnoType<css::drawing::BitmapMode>::get(),      0,  0},
                    { OUString(UNO_NAME_HEADER_FILLCOLOR),                      XATTR_FILLCOLOR,                cppu::UnoType<sal_Int32>::get(),           0,  0},
                    { OUString(UNO_NAME_HEADER_FILLBACKGROUND),                 XATTR_FILLBACKGROUND,           cppu::UnoType<bool>::get(),         0,  0},
                    { OUString(UNO_NAME_HEADER_FILLBITMAP),                     XATTR_FILLBITMAP,               cppu::UnoType<css::awt::XBitmap>::get(),       0,  MID_BITMAP},
                    { OUString(UNO_NAME_HEADER_FILLBITMAPNAME),                 XATTR_FILLBITMAP,               cppu::UnoType<OUString>::get(),        0,  MID_NAME },
                    { OUString(UNO_NAME_HEADER_FILLBITMAPURL),                  XATTR_FILLBITMAP,               cppu::UnoType<OUString>::get(),        0,  MID_GRAFURL },
                    { OUString(UNO_NAME_HEADER_FILLGRADIENTSTEPCOUNT),          XATTR_GRADIENTSTEPCOUNT,        cppu::UnoType<sal_Int16>::get(),           0,  0},
                    { OUString(UNO_NAME_HEADER_FILLGRADIENT),                   XATTR_FILLGRADIENT,             cppu::UnoType<css::awt::Gradient>::get(),        0,  MID_FILLGRADIENT},
                    { OUString(UNO_NAME_HEADER_FILLGRADIENTNAME),               XATTR_FILLGRADIENT,             cppu::UnoType<OUString>::get(),        0,  MID_NAME },
                    { OUString(UNO_NAME_HEADER_FILLHATCH),                      XATTR_FILLHATCH,                cppu::UnoType<css::drawing::Hatch>::get(),           0,  MID_FILLHATCH},
                    { OUString(UNO_NAME_HEADER_FILLHATCHNAME),                  XATTR_FILLHATCH,                cppu::UnoType<OUString>::get(),        0,  MID_NAME },
                    { OUString(UNO_NAME_HEADER_FILLSTYLE),                      XATTR_FILLSTYLE,                cppu::UnoType<css::drawing::FillStyle>::get(),       0,  0},
                    { OUString(UNO_NAME_HEADER_FILL_TRANSPARENCE),              XATTR_FILLTRANSPARENCE,         cppu::UnoType<sal_Int16>::get(),           0,  0},
                    { OUString(UNO_NAME_HEADER_FILLTRANSPARENCEGRADIENT),       XATTR_FILLFLOATTRANSPARENCE,    cppu::UnoType<css::awt::Gradient>::get(),        0,  MID_FILLGRADIENT},
                    { OUString(UNO_NAME_HEADER_FILLTRANSPARENCEGRADIENTNAME),   XATTR_FILLFLOATTRANSPARENCE,    cppu::UnoType<OUString>::get(),        0,  MID_NAME },
                    { OUString(UNO_NAME_HEADER_FILLCOLOR_2),                    XATTR_SECONDARYFILLCOLOR,       cppu::UnoType<sal_Int32>::get(),           0,  0},

                    //UUUU Added DrawingLayer FillStyle Properties for Footer, similar as for Header (see there)
                    { OUString(UNO_NAME_FOOTER_FILLBMP_LOGICAL_SIZE),           XATTR_FILLBMP_SIZELOG,          cppu::UnoType<bool>::get() ,        0,  0},
                    { OUString(UNO_NAME_FOOTER_FILLBMP_OFFSET_X),               XATTR_FILLBMP_TILEOFFSETX,      cppu::UnoType<sal_Int32>::get() ,          0,  0},
                    { OUString(UNO_NAME_FOOTER_FILLBMP_OFFSET_Y),               XATTR_FILLBMP_TILEOFFSETY,      cppu::UnoType<sal_Int32>::get() ,          0,  0},
                    { OUString(UNO_NAME_FOOTER_FILLBMP_POSITION_OFFSET_X),      XATTR_FILLBMP_POSOFFSETX,       cppu::UnoType<sal_Int32>::get() ,          0,  0},
                    { OUString(UNO_NAME_FOOTER_FILLBMP_POSITION_OFFSET_Y),      XATTR_FILLBMP_POSOFFSETY,       cppu::UnoType<sal_Int32>::get() ,          0,  0},
                    { OUString(UNO_NAME_FOOTER_FILLBMP_RECTANGLE_POINT),        XATTR_FILLBMP_POS,              cppu::UnoType<css::drawing::RectanglePoint>::get() , 0,  0},
                    { OUString(UNO_NAME_FOOTER_FILLBMP_SIZE_X),                 XATTR_FILLBMP_SIZEX,            cppu::UnoType<sal_Int32>::get() ,          0,  SFX_METRIC_ITEM},
                    { OUString(UNO_NAME_FOOTER_FILLBMP_SIZE_Y),                 XATTR_FILLBMP_SIZEY,            cppu::UnoType<sal_Int32>::get() ,          0,  SFX_METRIC_ITEM},
                    { OUString(UNO_NAME_FOOTER_FILLBMP_STRETCH),                XATTR_FILLBMP_STRETCH,          cppu::UnoType<bool>::get() ,        0,  0},
                    { OUString(UNO_NAME_FOOTER_FILLBMP_TILE),                   XATTR_FILLBMP_TILE,             cppu::UnoType<bool>::get() ,        0,  0},
                    { OUString(UNO_NAME_FOOTER_FILLBMP_MODE),                   OWN_ATTR_FILLBMP_MODE,          cppu::UnoType<css::drawing::BitmapMode>::get(),      0,  0},
                    { OUString(UNO_NAME_FOOTER_FILLCOLOR),                      XATTR_FILLCOLOR,                cppu::UnoType<sal_Int32>::get(),           0,  0},
                    { OUString(UNO_NAME_FOOTER_FILLBACKGROUND),                 XATTR_FILLBACKGROUND,           cppu::UnoType<bool>::get(),         0,  0},
                    { OUString(UNO_NAME_FOOTER_FILLBITMAP),                     XATTR_FILLBITMAP,               cppu::UnoType<css::awt::XBitmap>::get(),       0,  MID_BITMAP},
                    { OUString(UNO_NAME_FOOTER_FILLBITMAPNAME),                 XATTR_FILLBITMAP,               cppu::UnoType<OUString>::get(),        0,  MID_NAME },
                    { OUString(UNO_NAME_FOOTER_FILLBITMAPURL),                  XATTR_FILLBITMAP,               cppu::UnoType<OUString>::get(),        0,  MID_GRAFURL },
                    { OUString(UNO_NAME_FOOTER_FILLGRADIENTSTEPCOUNT),          XATTR_GRADIENTSTEPCOUNT,        cppu::UnoType<sal_Int16>::get(),           0,  0},
                    { OUString(UNO_NAME_FOOTER_FILLGRADIENT),                   XATTR_FILLGRADIENT,             cppu::UnoType<css::awt::Gradient>::get(),        0,  MID_FILLGRADIENT},
                    { OUString(UNO_NAME_FOOTER_FILLGRADIENTNAME),               XATTR_FILLGRADIENT,             cppu::UnoType<OUString>::get(),        0,  MID_NAME },
                    { OUString(UNO_NAME_FOOTER_FILLHATCH),                      XATTR_FILLHATCH,                cppu::UnoType<css::drawing::Hatch>::get(),           0,  MID_FILLHATCH},
                    { OUString(UNO_NAME_FOOTER_FILLHATCHNAME),                  XATTR_FILLHATCH,                cppu::UnoType<OUString>::get(),        0,  MID_NAME },
                    { OUString(UNO_NAME_FOOTER_FILLSTYLE),                      XATTR_FILLSTYLE,                cppu::UnoType<css::drawing::FillStyle>::get(),       0,  0},
                    { OUString(UNO_NAME_FOOTER_FILL_TRANSPARENCE),              XATTR_FILLTRANSPARENCE,         cppu::UnoType<sal_Int16>::get(),           0,  0},
                    { OUString(UNO_NAME_FOOTER_FILLTRANSPARENCEGRADIENT),       XATTR_FILLFLOATTRANSPARENCE,    cppu::UnoType<css::awt::Gradient>::get(),        0,  MID_FILLGRADIENT},
                    { OUString(UNO_NAME_FOOTER_FILLTRANSPARENCEGRADIENTNAME),   XATTR_FILLFLOATTRANSPARENCE,    cppu::UnoType<OUString>::get(),        0,  MID_NAME },
                    { OUString(UNO_NAME_FOOTER_FILLCOLOR_2),                    XATTR_SECONDARYFILLCOLOR,       cppu::UnoType<sal_Int32>::get(),           0,  0},

                    { OUString(), 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aPageStyleMap;
            }
            break;
            case PROPERTY_MAP_NUM_STYLE  :
            {
                static SfxItemPropertyMapEntry const aNumStyleMap        [] =
                {
                    { OUString(UNO_NAME_NUMBERING_RULES), FN_UNO_NUM_RULES, cppu::UnoType<css::container::XIndexReplace>::get(), PROPERTY_NONE, CONVERT_TWIPS},
                    { OUString(UNO_NAME_IS_PHYSICAL), FN_UNO_IS_PHYSICAL,     cppu::UnoType<bool>::get(), PropertyAttribute::READONLY, 0},
                    { OUString(UNO_NAME_DISPLAY_NAME), FN_UNO_DISPLAY_NAME, cppu::UnoType<OUString>::get(), PropertyAttribute::READONLY, 0},
                    { OUString(UNO_NAME_HIDDEN), FN_UNO_HIDDEN,     cppu::UnoType<bool>::get(), PROPERTY_NONE, 0},
                    { OUString(UNO_NAME_STYLE_INTEROP_GRAB_BAG), FN_UNO_STYLE_INTEROP_GRAB_BAG, cppu::UnoType< cppu::UnoSequenceType<css::beans::PropertyValue> >::get(), PROPERTY_NONE, 0},
                    { OUString(), 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aNumStyleMap;
            }
            break;
            case PROPERTY_MAP_TEXT_TABLE :
            {
                static SfxItemPropertyMapEntry const aTablePropertyMap_Impl[] =
                {
                    { OUString(UNO_NAME_BACK_COLOR), RES_BACKGROUND,        cppu::UnoType<sal_Int32>::get(),           PROPERTY_NONE,MID_BACK_COLOR         },
                    { OUString(UNO_NAME_BREAK_TYPE), RES_BREAK,                 cppu::UnoType<css::style::BreakType>::get(),       PROPERTY_NONE, 0},
                    { OUString(UNO_NAME_BACK_GRAPHIC_URL), RES_BACKGROUND,      cppu::UnoType<OUString>::get(), PROPERTY_NONE ,MID_GRAPHIC_URL    },
                    { OUString(UNO_NAME_BACK_GRAPHIC_FILTER), RES_BACKGROUND,       cppu::UnoType<OUString>::get(), PROPERTY_NONE ,MID_GRAPHIC_FILTER    },
                    { OUString(UNO_NAME_BACK_GRAPHIC_LOCATION), RES_BACKGROUND,         cppu::UnoType<css::style::GraphicLocation>::get(), PROPERTY_NONE ,MID_GRAPHIC_POSITION},
                    { OUString(UNO_NAME_LEFT_MARGIN), RES_LR_SPACE,             cppu::UnoType<sal_Int32>::get(), PROPERTY_NONE, MID_L_MARGIN|CONVERT_TWIPS},
                    { OUString(UNO_NAME_RIGHT_MARGIN), RES_LR_SPACE,            cppu::UnoType<sal_Int32>::get(), PROPERTY_NONE, MID_R_MARGIN|CONVERT_TWIPS},
                    { OUString(UNO_NAME_HORI_ORIENT), RES_HORI_ORIENT,      cppu::UnoType<sal_Int16>::get(),           PROPERTY_NONE ,MID_HORIORIENT_ORIENT    },
                    { OUString(UNO_NAME_KEEP_TOGETHER), RES_KEEP,               cppu::UnoType<bool>::get()  ,       PROPERTY_NONE, 0},
                    { OUString(UNO_NAME_SPLIT), RES_LAYOUT_SPLIT,       cppu::UnoType<bool>::get()  ,       PROPERTY_NONE, 0},
                    { OUString(UNO_NAME_PAGE_NUMBER_OFFSET), RES_PAGEDESC,              cppu::UnoType<sal_Int16>::get(),       PropertyAttribute::MAYBEVOID, MID_PAGEDESC_PAGENUMOFFSET},
                    { OUString(UNO_NAME_PAGE_DESC_NAME), RES_PAGEDESC,           cppu::UnoType<OUString>::get(),         PropertyAttribute::MAYBEVOID, 0xff},
                    { OUString(UNO_NAME_RELATIVE_WIDTH), FN_TABLE_RELATIVE_WIDTH,cppu::UnoType<sal_Int16>::get()  ,        PROPERTY_NONE, 0xff },
                    { OUString(UNO_NAME_REPEAT_HEADLINE), FN_TABLE_HEADLINE_REPEAT,cppu::UnoType<bool>::get(),      PROPERTY_NONE, 0xff},
                    { OUString(UNO_NAME_HEADER_ROW_COUNT), FN_TABLE_HEADLINE_COUNT,  cppu::UnoType<sal_Int32>::get(),      PROPERTY_NONE, 0xff},
                    { OUString(UNO_NAME_SHADOW_FORMAT), RES_SHADOW,             cppu::UnoType<css::table::ShadowFormat>::get(),   PROPERTY_NONE, 0},
                    { OUString(UNO_NAME_SHADOW_TRANSPARENCE), RES_SHADOW,       cppu::UnoType<sal_Int16>::get(),       PROPERTY_NONE, MID_SHADOW_TRANSPARENCE},
                    { OUString(UNO_NAME_TOP_MARGIN), RES_UL_SPACE,          cppu::UnoType<sal_Int32>::get(), PROPERTY_NONE, MID_UP_MARGIN|CONVERT_TWIPS},
                    { OUString(UNO_NAME_BOTTOM_MARGIN), RES_UL_SPACE,           cppu::UnoType<sal_Int32>::get(), PROPERTY_NONE, MID_LO_MARGIN|CONVERT_TWIPS},
                    { OUString(UNO_NAME_BACK_TRANSPARENT), RES_BACKGROUND,  cppu::UnoType<bool>::get(),         PROPERTY_NONE ,MID_GRAPHIC_TRANSPARENT       },
                    { OUString(UNO_NAME_WIDTH), FN_TABLE_WIDTH,         cppu::UnoType<sal_Int32>::get()  ,         PROPERTY_NONE, 0xff},
                    { OUString(UNO_NAME_IS_WIDTH_RELATIVE), FN_TABLE_IS_RELATIVE_WIDTH,         cppu::UnoType<bool>::get()  ,       PROPERTY_NONE, 0xff},
                    { OUString(UNO_NAME_CHART_ROW_AS_LABEL), FN_UNO_RANGE_ROW_LABEL,            cppu::UnoType<bool>::get(),         PROPERTY_NONE,  0},
                    { OUString(UNO_NAME_CHART_COLUMN_AS_LABEL), FN_UNO_RANGE_COL_LABEL,         cppu::UnoType<bool>::get()  ,       PROPERTY_NONE,     0},
                    { OUString(UNO_NAME_TABLE_BORDER), FN_UNO_TABLE_BORDER,         cppu::UnoType<css::table::TableBorder>::get(), PropertyAttribute::MAYBEVOID, CONVERT_TWIPS },
                    { OUString(UNO_NAME_TABLE_BORDER2), FN_UNO_TABLE_BORDER2,         cppu::UnoType<css::table::TableBorder2>::get(), PropertyAttribute::MAYBEVOID, CONVERT_TWIPS },
                    { OUString(UNO_NAME_TABLE_BORDER_DISTANCES), FN_UNO_TABLE_BORDER_DISTANCES,         cppu::UnoType<css::table::TableBorderDistances>::get(), PropertyAttribute::MAYBEVOID, CONVERT_TWIPS },
                    { OUString(UNO_NAME_TABLE_COLUMN_SEPARATORS), FN_UNO_TABLE_COLUMN_SEPARATORS,   cppu::UnoType< cppu::UnoSequenceType<css::text::TableColumnSeparator> >::get(),   PropertyAttribute::MAYBEVOID, 0 },
                    { OUString(UNO_NAME_TABLE_COLUMN_RELATIVE_SUM), FN_UNO_TABLE_COLUMN_RELATIVE_SUM,       cppu::UnoType<sal_Int16>::get(),       PropertyAttribute::READONLY, 0 },
                    COMMON_TEXT_CONTENT_PROPERTIES
                    { OUString(UNO_LINK_DISPLAY_NAME), FN_PARAM_LINK_DISPLAY_NAME,  cppu::UnoType<OUString>::get(), PropertyAttribute::READONLY, 0xff},
                    { OUString(UNO_NAME_USER_DEFINED_ATTRIBUTES), RES_UNKNOWNATR_CONTAINER, cppu::UnoType<css::container::XNameContainer>::get(), PropertyAttribute::MAYBEVOID, 0 },
                    { OUString(UNO_NAME_TEXT_SECTION), FN_UNO_TEXT_SECTION, cppu::UnoType<css::text::XTextSection>::get(),  PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY ,0 },
                    { OUString(UNO_NAME_WRITING_MODE), RES_FRAMEDIR, cppu::UnoType<sal_Int16>::get(), PROPERTY_NONE, 0 },
                    { OUString(UNO_NAME_TABLE_NAME),   FN_UNO_TABLE_NAME,   cppu::UnoType<OUString>::get(),        PROPERTY_NONE, 0 },
                     { OUString(UNO_NAME_PAGE_STYLE_NAME), RES_PAGEDESC, cppu::UnoType<OUString>::get(),         PROPERTY_NONE, 0},
                    // #i29550#
                    { OUString(UNO_NAME_COLLAPSING_BORDERS), RES_COLLAPSING_BORDERS, cppu::UnoType<bool>::get(), PROPERTY_NONE, 0},
                    _REDLINE_NODE_PROPERTIES
                    { OUString(UNO_NAME_TABLE_INTEROP_GRAB_BAG), RES_FRMATR_GRABBAG, cppu::UnoType< cppu::UnoSequenceType<css::beans::PropertyValue> >::get(), PROPERTY_NONE, 0 },
                    { OUString(), 0, css::uno::Type(), 0, 0 }
                };

                m_aMapEntriesArr[nPropertyId] = aTablePropertyMap_Impl;
            }
            break;
            case PROPERTY_MAP_TABLE_CELL :
            {
                static SfxItemPropertyMapEntry const aCellMap_Impl[] =
                {
                    { OUString(UNO_NAME_BACK_COLOR), RES_BACKGROUND,    cppu::UnoType<sal_Int32>::get(),           PROPERTY_NONE , MID_BACK_COLOR       },
                    { OUString(UNO_NAME_BACK_GRAPHIC_URL), RES_BACKGROUND,      cppu::UnoType<OUString>::get(), PROPERTY_NONE ,MID_GRAPHIC_URL    },
                    { OUString(UNO_NAME_BACK_GRAPHIC_FILTER), RES_BACKGROUND,       cppu::UnoType<OUString>::get(), PROPERTY_NONE ,MID_GRAPHIC_FILTER    },
                    { OUString(UNO_NAME_BACK_GRAPHIC_LOCATION), RES_BACKGROUND,         cppu::UnoType<css::style::GraphicLocation>::get(), PROPERTY_NONE ,MID_GRAPHIC_POSITION},
                    { OUString(UNO_NAME_BACK_TRANSPARENT), RES_BACKGROUND,  cppu::UnoType<bool>::get(),         PROPERTY_NONE , MID_GRAPHIC_TRANSPARENT      },
                    { OUString(UNO_NAME_NUMBER_FORMAT), RES_BOXATR_FORMAT,  cppu::UnoType<sal_Int32>::get(),           PropertyAttribute::MAYBEVOID ,0             },
                    { OUString(UNO_NAME_LEFT_BORDER), RES_BOX,    cppu::UnoType<css::table::BorderLine>::get(),    0, LEFT_BORDER  |CONVERT_TWIPS },
                    { OUString(UNO_NAME_RIGHT_BORDER), RES_BOX,    cppu::UnoType<css::table::BorderLine>::get(),   0, RIGHT_BORDER |CONVERT_TWIPS },
                    { OUString(UNO_NAME_TOP_BORDER), RES_BOX,    cppu::UnoType<css::table::BorderLine>::get(), 0, TOP_BORDER   |CONVERT_TWIPS },
                    { OUString(UNO_NAME_BOTTOM_BORDER), RES_BOX,    cppu::UnoType<css::table::BorderLine>::get(),  0, BOTTOM_BORDER|CONVERT_TWIPS },
                    { OUString(UNO_NAME_BORDER_DISTANCE), RES_BOX,    cppu::UnoType<sal_Int32>::get(), 0, BORDER_DISTANCE|CONVERT_TWIPS },
                    { OUString(UNO_NAME_LEFT_BORDER_DISTANCE), RES_BOX,             cppu::UnoType<sal_Int32>::get(),   0, LEFT_BORDER_DISTANCE  |CONVERT_TWIPS },
                    { OUString(UNO_NAME_RIGHT_BORDER_DISTANCE), RES_BOX,                cppu::UnoType<sal_Int32>::get(),   0, RIGHT_BORDER_DISTANCE |CONVERT_TWIPS },
                    { OUString(UNO_NAME_TOP_BORDER_DISTANCE), RES_BOX,              cppu::UnoType<sal_Int32>::get(),   0, TOP_BORDER_DISTANCE   |CONVERT_TWIPS },
                    { OUString(UNO_NAME_BOTTOM_BORDER_DISTANCE), RES_BOX,               cppu::UnoType<sal_Int32>::get(),   0, BOTTOM_BORDER_DISTANCE|CONVERT_TWIPS },
                    { OUString(UNO_NAME_USER_DEFINED_ATTRIBUTES), RES_UNKNOWNATR_CONTAINER, cppu::UnoType<css::container::XNameContainer>::get(), PropertyAttribute::MAYBEVOID, 0 },
                    { OUString(UNO_NAME_TEXT_SECTION), FN_UNO_TEXT_SECTION, cppu::UnoType<css::text::XTextSection>::get(),  PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY ,0 },
                    { OUString(UNO_NAME_IS_PROTECTED), RES_PROTECT,            cppu::UnoType<bool>::get(), 0, MID_PROTECT_CONTENT},
                    { OUString(UNO_NAME_CELL_NAME), FN_UNO_CELL_NAME,            cppu::UnoType<OUString>::get(), PropertyAttribute::READONLY,0},
                    { OUString(UNO_NAME_VERT_ORIENT), RES_VERT_ORIENT,      cppu::UnoType<sal_Int16>::get(),           PROPERTY_NONE ,MID_VERTORIENT_ORIENT    },
                    { OUString(UNO_NAME_WRITING_MODE), RES_FRAMEDIR, cppu::UnoType<sal_Int16>::get(), PROPERTY_NONE, 0 },
                    { OUString(UNO_NAME_ROW_SPAN),     FN_UNO_CELL_ROW_SPAN, cppu::UnoType<sal_Int32>::get(),  0, 0 },
                    { OUString(UNO_NAME_CELL_INTEROP_GRAB_BAG), RES_FRMATR_GRABBAG, cppu::UnoType< cppu::UnoSequenceType<css::beans::PropertyValue> >::get(), PROPERTY_NONE, 0 },
                    _REDLINE_NODE_PROPERTIES
                    { OUString(), 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aCellMap_Impl;
            }
            break;
            case PROPERTY_MAP_TABLE_RANGE:
            {
                static SfxItemPropertyMapEntry const aRangePropertyMap_Impl[] =
                {
                    COMMON_CRSR_PARA_PROPERTIES_WITHOUT_FN_01
                    TABSTOPS_MAP_ENTRY
                    { OUString(UNO_NAME_BACK_COLOR), FN_UNO_TABLE_CELL_BACKGROUND,  cppu::UnoType<sal_Int32>::get(),   PropertyAttribute::MAYBEVOID, MID_BACK_COLOR  },
                    { OUString(UNO_NAME_BACK_GRAPHIC_URL), RES_BACKGROUND,      cppu::UnoType<OUString>::get(), PropertyAttribute::MAYBEVOID ,MID_GRAPHIC_URL    },
                    { OUString(UNO_NAME_BACK_GRAPHIC_FILTER), RES_BACKGROUND,       cppu::UnoType<OUString>::get(), PropertyAttribute::MAYBEVOID ,MID_GRAPHIC_FILTER    },
                    { OUString(UNO_NAME_BACK_GRAPHIC_LOCATION), FN_UNO_TABLE_CELL_BACKGROUND,   cppu::UnoType<css::style::GraphicLocation>::get(), PropertyAttribute::MAYBEVOID, MID_GRAPHIC_POSITION},
                    { OUString(UNO_NAME_BACK_TRANSPARENT), FN_UNO_TABLE_CELL_BACKGROUND,    cppu::UnoType<bool>::get(), PropertyAttribute::MAYBEVOID, MID_GRAPHIC_TRANSPARENT      },
                    { OUString(UNO_NAME_NUMBER_FORMAT), RES_BOXATR_FORMAT,         cppu::UnoType<sal_Int32>::get(),           PropertyAttribute::MAYBEVOID ,0             },
                    { OUString(UNO_NAME_VERT_ORIENT), RES_VERT_ORIENT,      cppu::UnoType<sal_Int16>::get(),           PROPERTY_NONE ,MID_VERTORIENT_ORIENT    },
                    { OUString(UNO_NAME_CHART_ROW_AS_LABEL), FN_UNO_RANGE_ROW_LABEL,    cppu::UnoType<bool>::get(),         PropertyAttribute::MAYBEVOID,  0},
                    { OUString(UNO_NAME_CHART_COLUMN_AS_LABEL), FN_UNO_RANGE_COL_LABEL, cppu::UnoType<bool>::get()  ,       PropertyAttribute::MAYBEVOID,     0},
                    { OUString(), 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aRangePropertyMap_Impl;
            }
            break;
            case PROPERTY_MAP_SECTION:
            {
                static SfxItemPropertyMapEntry const aSectionPropertyMap_Impl[] =
                {
                    { OUString(UNO_NAME_CONDITION), WID_SECT_CONDITION, cppu::UnoType<OUString>::get()  ,      PROPERTY_NONE,     0},
                    { OUString(UNO_NAME_DDE_COMMAND_FILE), WID_SECT_DDE_TYPE, cppu::UnoType<OUString>::get()  ,        PROPERTY_NONE,     0},
                    { OUString(UNO_NAME_DDE_COMMAND_TYPE), WID_SECT_DDE_FILE, cppu::UnoType<OUString>::get()  ,        PROPERTY_NONE,     0},
                    { OUString(UNO_NAME_DDE_COMMAND_ELEMENT), WID_SECT_DDE_ELEMENT, cppu::UnoType<OUString>::get()  ,      PROPERTY_NONE,     0},
                    { OUString(UNO_NAME_IS_AUTOMATIC_UPDATE), WID_SECT_DDE_AUTOUPDATE, cppu::UnoType<bool>::get(),              PROPERTY_NONE,  0},
                    { OUString(UNO_NAME_FILE_LINK), WID_SECT_LINK     , cppu::UnoType<css::text::SectionFileLink>::get(),     PROPERTY_NONE,     0},
                    { OUString(UNO_NAME_IS_VISIBLE), WID_SECT_VISIBLE   , cppu::UnoType<bool>::get(),           PROPERTY_NONE,     0},
                    { OUString(UNO_NAME_IS_PROTECTED), WID_SECT_PROTECTED, cppu::UnoType<bool>::get(),              PROPERTY_NONE,     0},
                    { OUString(UNO_NAME_EDIT_IN_READONLY), WID_SECT_EDIT_IN_READONLY,    cppu::UnoType<bool>::get(),         PROPERTY_NONE, 0},
                    { OUString(UNO_NAME_LINK_REGION), WID_SECT_REGION   , cppu::UnoType<OUString>::get()  ,        PROPERTY_NONE,     0},
                    { OUString(UNO_NAME_TEXT_COLUMNS), RES_COL,                cppu::UnoType<css::text::XTextColumns>::get(),    PROPERTY_NONE, MID_COLUMNS},
                    { OUString(UNO_NAME_BACK_GRAPHIC_URL), RES_BACKGROUND,      cppu::UnoType<OUString>::get(), PROPERTY_NONE ,MID_GRAPHIC_URL    },
                    { OUString(UNO_NAME_BACK_GRAPHIC_FILTER), RES_BACKGROUND,       cppu::UnoType<OUString>::get(), PROPERTY_NONE ,MID_GRAPHIC_FILTER    },
                    { OUString(UNO_NAME_BACK_GRAPHIC_LOCATION), RES_BACKGROUND,         cppu::UnoType<css::style::GraphicLocation>::get(),          PROPERTY_NONE ,MID_GRAPHIC_POSITION},
                    { OUString(UNO_NAME_BACK_COLOR), RES_BACKGROUND,            cppu::UnoType<sal_Int32>::get(),           PROPERTY_NONE ,MID_BACK_COLOR        },
                    { OUString(UNO_NAME_BACK_TRANSPARENT), RES_BACKGROUND,      cppu::UnoType<bool>::get(),         PROPERTY_NONE ,MID_GRAPHIC_TRANSPARENT       },
                    { OUString(UNO_LINK_DISPLAY_NAME), FN_PARAM_LINK_DISPLAY_NAME,  cppu::UnoType<OUString>::get(), PropertyAttribute::READONLY, 0xff},
                    { OUString(UNO_NAME_USER_DEFINED_ATTRIBUTES), RES_UNKNOWNATR_CONTAINER, cppu::UnoType<css::container::XNameContainer>::get(), PropertyAttribute::MAYBEVOID, 0 },
                    { OUString(UNO_NAME_FOOTNOTE_IS_COLLECT_AT_TEXT_END), RES_FTN_AT_TXTEND,        cppu::UnoType<bool>::get(),                PROPERTY_NONE ,MID_COLLECT                   },
                    { OUString(UNO_NAME_FOOTNOTE_IS_RESTART_NUMBERING), RES_FTN_AT_TXTEND,      cppu::UnoType<bool>::get(),                PROPERTY_NONE , MID_RESTART_NUM },
                    { OUString(UNO_NAME_FOOTNOTE_RESTART_NUMBERING_AT), RES_FTN_AT_TXTEND,      cppu::UnoType<sal_Int16>::get(),               PROPERTY_NONE , MID_NUM_START_AT},
                    { OUString(UNO_NAME_FOOTNOTE_IS_OWN_NUMBERING), RES_FTN_AT_TXTEND,  cppu::UnoType<bool>::get(),                                                 PROPERTY_NONE ,  MID_OWN_NUM     },
                    { OUString(UNO_NAME_FOOTNOTE_NUMBERING_TYPE), RES_FTN_AT_TXTEND,        cppu::UnoType<sal_Int16>::get(),               PROPERTY_NONE , MID_NUM_TYPE    },
                    { OUString(UNO_NAME_FOOTNOTE_NUMBERING_PREFIX), RES_FTN_AT_TXTEND,  cppu::UnoType<OUString>::get()  ,      PROPERTY_NONE,      MID_PREFIX      },
                    { OUString(UNO_NAME_FOOTNOTE_NUMBERING_SUFFIX), RES_FTN_AT_TXTEND,  cppu::UnoType<OUString>::get()  ,      PROPERTY_NONE,      MID_SUFFIX      },
                    { OUString(UNO_NAME_ENDNOTE_IS_COLLECT_AT_TEXT_END), RES_END_AT_TXTEND,        cppu::UnoType<bool>::get(),                 PROPERTY_NONE , MID_COLLECT                      },
                    { OUString(UNO_NAME_ENDNOTE_IS_RESTART_NUMBERING), RES_END_AT_TXTEND,        cppu::UnoType<bool>::get(),                   PROPERTY_NONE , MID_RESTART_NUM  },
                    { OUString(UNO_NAME_ENDNOTE_RESTART_NUMBERING_AT), RES_END_AT_TXTEND,     cppu::UnoType<sal_Int16>::get(),             PROPERTY_NONE ,  MID_NUM_START_AT },
                    { OUString(UNO_NAME_ENDNOTE_IS_OWN_NUMBERING), RES_END_AT_TXTEND,     cppu::UnoType<bool>::get(),                                                   PROPERTY_NONE ,  MID_OWN_NUM      },
                    { OUString(UNO_NAME_ENDNOTE_NUMBERING_TYPE), RES_END_AT_TXTEND,       cppu::UnoType<sal_Int16>::get(),             PROPERTY_NONE ,MID_NUM_TYPE     },
                    { OUString(UNO_NAME_ENDNOTE_NUMBERING_PREFIX), RES_END_AT_TXTEND,     cppu::UnoType<OUString>::get()  ,        PROPERTY_NONE,   MID_PREFIX       },
                    { OUString(UNO_NAME_ENDNOTE_NUMBERING_SUFFIX), RES_END_AT_TXTEND,     cppu::UnoType<OUString>::get()  ,        PROPERTY_NONE,   MID_SUFFIX       },
                    { OUString(UNO_NAME_DOCUMENT_INDEX), WID_SECT_DOCUMENT_INDEX, cppu::UnoType<css::text::XDocumentIndex>::get(), PropertyAttribute::READONLY | PropertyAttribute::MAYBEVOID, 0 },
                    { OUString(UNO_NAME_IS_GLOBAL_DOCUMENT_SECTION), WID_SECT_IS_GLOBAL_DOC_SECTION, cppu::UnoType<bool>::get(), PropertyAttribute::READONLY, 0 },
                    { OUString(UNO_NAME_PROTECTION_KEY), WID_SECT_PASSWORD,      cppu::UnoType< cppu::UnoSequenceType<sal_Int8> >::get(), PROPERTY_NONE, 0 },
                    { OUString(UNO_NAME_DONT_BALANCE_TEXT_COLUMNS), RES_COLUMNBALANCE, cppu::UnoType<bool>::get(), PROPERTY_NONE, 0 },
                    COMMON_TEXT_CONTENT_PROPERTIES
                    _REDLINE_NODE_PROPERTIES
                    { OUString(UNO_NAME_IS_CURRENTLY_VISIBLE), WID_SECT_CURRENTLY_VISIBLE, cppu::UnoType<bool>::get(),          PROPERTY_NONE,     0},
                    { OUString(UNO_NAME_WRITING_MODE), RES_FRAMEDIR, cppu::UnoType<sal_Int16>::get(), PROPERTY_NONE, 0 },
                    { OUString(UNO_NAME_SECT_LEFT_MARGIN), RES_LR_SPACE,            cppu::UnoType<sal_Int32>::get(),           PropertyAttribute::MAYBEVOID, MID_L_MARGIN|CONVERT_TWIPS},
                    { OUString(UNO_NAME_SECT_RIGHT_MARGIN), RES_LR_SPACE,           cppu::UnoType<sal_Int32>::get(),           PropertyAttribute::MAYBEVOID, MID_R_MARGIN|CONVERT_TWIPS},
                    { OUString(), 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aSectionPropertyMap_Impl;
            }
            break;
            case PROPERTY_MAP_TEXT_SEARCH:
            {
                static SfxItemPropertyMapEntry const aSearchPropertyMap_Impl[] =
                {
                    { OUString(UNO_NAME_SEARCH_ALL), WID_SEARCH_ALL,        cppu::UnoType<bool>::get()  ,       PROPERTY_NONE,     0},
                    { OUString(UNO_NAME_SEARCH_BACKWARDS), WID_BACKWARDS,           cppu::UnoType<bool>::get()  ,       PROPERTY_NONE,     0},
                    { OUString(UNO_NAME_SEARCH_CASE_SENSITIVE), WID_CASE_SENSITIVE,     cppu::UnoType<bool>::get()  ,       PROPERTY_NONE,     0},
                    { OUString(UNO_NAME_SEARCH_REGULAR_EXPRESSION), WID_REGULAR_EXPRESSION, cppu::UnoType<bool>::get()  ,       PROPERTY_NONE,     0},
                    { OUString(UNO_NAME_SEARCH_SIMILARITY), WID_SIMILARITY,         cppu::UnoType<bool>::get()  ,       PROPERTY_NONE,     0},
                    { OUString(UNO_NAME_SEARCH_SIMILARITY_ADD), WID_SIMILARITY_ADD,     cppu::UnoType<sal_Int16>::get()  ,     PROPERTY_NONE,     0},
                    { OUString(UNO_NAME_SEARCH_SIMILARITY_EXCHANGE), WID_SIMILARITY_EXCHANGE,cppu::UnoType<sal_Int16>::get()  ,    PROPERTY_NONE,     0},
                    { OUString(UNO_NAME_SEARCH_SIMILARITY_RELAX), WID_SIMILARITY_RELAX,     cppu::UnoType<bool>::get()  ,       PROPERTY_NONE,     0},
                    { OUString(UNO_NAME_SEARCH_SIMILARITY_REMOVE), WID_SIMILARITY_REMOVE,   cppu::UnoType<sal_Int16>::get()  ,     PROPERTY_NONE,     0},
                    { OUString(UNO_NAME_SEARCH_STYLES), WID_STYLES,             cppu::UnoType<bool>::get()  ,       PROPERTY_NONE,     0},
                    { OUString(UNO_NAME_SEARCH_WORDS), WID_WORDS,               cppu::UnoType<bool>::get()  ,       PROPERTY_NONE,     0},
                    { OUString(), 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aSearchPropertyMap_Impl;
            }
            break;
            case PROPERTY_MAP_TEXT_FRAME:
            {
                static SfxItemPropertyMapEntry const aFramePropertyMap_Impl[] =
                {   //UUUU
                    // TODO: We should consider completely removing SvxBrushItem() stuff
                    // add support for XATTR_FILL_FIRST, XATTR_FILL_LAST
                    // COMMON_FRAME_PROPERTIES currently hosts the RES_BACKGROUND entries from SvxBrushItem
                    COMMON_FRAME_PROPERTIES
                    _REDLINE_NODE_PROPERTIES
                    { OUString(UNO_NAME_CHAIN_NEXT_NAME), RES_CHAIN,                cppu::UnoType<OUString>::get(),            PropertyAttribute::MAYBEVOID ,MID_CHAIN_NEXTNAME},
                    { OUString(UNO_NAME_CHAIN_PREV_NAME), RES_CHAIN,                cppu::UnoType<OUString>::get(),            PropertyAttribute::MAYBEVOID ,MID_CHAIN_PREVNAME},
                /*not impl*/    { OUString(UNO_NAME_CLIENT_MAP), RES_URL,               cppu::UnoType<bool>::get(),         PROPERTY_NONE ,MID_URL_CLIENTMAP         },
                    { OUString(UNO_NAME_EDIT_IN_READONLY), RES_EDIT_IN_READONLY,    cppu::UnoType<bool>::get(),         PROPERTY_NONE, 0},
                    { OUString(UNO_NAME_TEXT_COLUMNS), RES_COL,                cppu::UnoType<css::text::XTextColumns>::get(),    PROPERTY_NONE, MID_COLUMNS},
                    //next elements are part of the service description
                    { OUString(UNO_NAME_FRAME_HEIGHT_ABSOLUTE),         RES_FRM_SIZE,           cppu::UnoType<sal_Int32>::get(),       PROPERTY_NONE, MID_FRMSIZE_HEIGHT|CONVERT_TWIPS         },
                    { OUString(UNO_NAME_FRAME_HEIGHT_PERCENT),              RES_FRM_SIZE,           cppu::UnoType<sal_Int8>::get(),        PROPERTY_NONE, MID_FRMSIZE_REL_HEIGHT   },
                    { OUString(UNO_NAME_FRAME_ISAUTOMATIC_HEIGHT),         RES_FRM_SIZE,        cppu::UnoType<bool>::get(),         PROPERTY_NONE, MID_FRMSIZE_IS_AUTO_HEIGHT   },
                    { OUString(UNO_NAME_FRAME_WIDTH_ABSOLUTE),          RES_FRM_SIZE,           cppu::UnoType<sal_Int32>::get(),       PROPERTY_NONE, MID_FRMSIZE_WIDTH|CONVERT_TWIPS          },
                    { OUString(UNO_NAME_FRAME_WIDTH_PERCENT),               RES_FRM_SIZE,           cppu::UnoType<sal_Int8>::get(),        PROPERTY_NONE, MID_FRMSIZE_REL_WIDTH    },
                    { OUString(UNO_NAME_SIZE_TYPE), RES_FRM_SIZE,           cppu::UnoType<sal_Int16>::get()  ,         PROPERTY_NONE,   MID_FRMSIZE_SIZE_TYPE  },
                    { OUString(UNO_NAME_WIDTH_TYPE), RES_FRM_SIZE,          cppu::UnoType<sal_Int16>::get()  ,         PROPERTY_NONE,   MID_FRMSIZE_WIDTH_TYPE },
                    { OUString(UNO_NAME_WRITING_MODE), RES_FRAMEDIR, cppu::UnoType<sal_Int16>::get(), PROPERTY_NONE, 0 },

                    //UUUU added FillProperties for SW, same as FILL_PROPERTIES in svx
                    // but need own defines in Writer due to later association of strings
                    // and uno types (see loop at end of this method and definition of SW_PROP_NMID)
                    // This entry is for adding that properties to FlyFrame import/export
                    FILL_PROPERTIES_SW

                    { OUString(), 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aFramePropertyMap_Impl;
            }
            break;
            case PROPERTY_MAP_TEXT_GRAPHIC:
            {
                static SfxItemPropertyMapEntry const aGraphicPropertyMap_Impl[] =
                {   //UUUU
                    // TODO: We should consider completely removing SvxBrushItem() stuff
                    // add support for XATTR_FILL_FIRST, XATTR_FILL_LAST
                    // COMMON_FRAME_PROPERTIES currently hosts the RES_BACKGROUND entries from SvxBrushItem
                    COMMON_FRAME_PROPERTIES
                    { OUString(UNO_NAME_SURROUND_CONTOUR), RES_SURROUND,            cppu::UnoType<bool>::get(),             PROPERTY_NONE, MID_SURROUND_CONTOUR         },
                    { OUString(UNO_NAME_CONTOUR_OUTSIDE), RES_SURROUND,             cppu::UnoType<bool>::get(),             PROPERTY_NONE, MID_SURROUND_CONTOUROUTSIDE  },
                    { OUString(UNO_NAME_GRAPHIC_CROP), RES_GRFATR_CROPGRF,     cppu::UnoType<css::text::GraphicCrop>::get(),  PROPERTY_NONE, CONVERT_TWIPS },
                    { OUString(UNO_NAME_HORI_MIRRORED_ON_EVEN_PAGES), RES_GRFATR_MIRRORGRF, cppu::UnoType<bool>::get(),             PROPERTY_NONE,      MID_MIRROR_HORZ_EVEN_PAGES            },
                    { OUString(UNO_NAME_HORI_MIRRORED_ON_ODD_PAGES), RES_GRFATR_MIRRORGRF,  cppu::UnoType<bool>::get(),             PROPERTY_NONE,      MID_MIRROR_HORZ_ODD_PAGES                 },
                    { OUString(UNO_NAME_VERT_MIRRORED), RES_GRFATR_MIRRORGRF,   cppu::UnoType<bool>::get(),             PROPERTY_NONE,     MID_MIRROR_VERT            },
                    { OUString(UNO_NAME_GRAPHIC_URL), FN_UNO_GRAPHIC_U_R_L, cppu::UnoType<OUString>::get(), 0, 0 },
                    { OUString(UNO_NAME_REPLACEMENT_GRAPHIC_URL), FN_UNO_REPLACEMENT_GRAPHIC_U_R_L, cppu::UnoType<OUString>::get(), 0, 0 },
                    { OUString(UNO_NAME_GRAPHIC_FILTER), FN_UNO_GRAPHIC_FILTER,      cppu::UnoType<OUString>::get(), 0, 0 },
                    { OUString(UNO_NAME_GRAPHIC), FN_UNO_GRAPHIC, cppu::UnoType<css::graphic::XGraphic>::get(), 0, 0 },
                    { OUString(UNO_NAME_ACTUAL_SIZE), FN_UNO_ACTUAL_SIZE,    cppu::UnoType<css::awt::Size>::get(),  PropertyAttribute::READONLY, CONVERT_TWIPS},
                    { OUString(UNO_NAME_CONTOUR_POLY_POLYGON), FN_PARAM_COUNTOUR_PP, cppu::UnoType<css::drawing::PointSequenceSequence>::get(), PropertyAttribute::MAYBEVOID, 0 },
                    { OUString(UNO_NAME_IS_PIXEL_CONTOUR), FN_UNO_IS_PIXEL_CONTOUR, cppu::UnoType<bool>::get(), PROPERTY_NONE, 0 },
                    { OUString(UNO_NAME_IS_AUTOMATIC_CONTOUR), FN_UNO_IS_AUTOMATIC_CONTOUR , cppu::UnoType<bool>::get(), PROPERTY_NONE, 0 },
                    { OUString(UNO_NAME_GRAPHIC_ROTATION), RES_GRFATR_ROTATION,      cppu::UnoType<sal_Int16>::get(),  0,   0},
                    { OUString(UNO_NAME_ADJUST_LUMINANCE), RES_GRFATR_LUMINANCE,     cppu::UnoType<sal_Int16>::get(),  0,   0},
                    { OUString(UNO_NAME_ADJUST_CONTRAST), RES_GRFATR_CONTRAST,   cppu::UnoType<sal_Int16>::get(),  0,   0},
                    { OUString(UNO_NAME_ADJUST_RED), RES_GRFATR_CHANNELR,    cppu::UnoType<sal_Int16>::get(),  0,   0},
                    { OUString(UNO_NAME_ADJUST_GREEN), RES_GRFATR_CHANNELG,      cppu::UnoType<sal_Int16>::get(),  0,   0},
                    { OUString(UNO_NAME_ADJUST_BLUE), RES_GRFATR_CHANNELB,   cppu::UnoType<sal_Int16>::get(),  0,   0},
                    { OUString(UNO_NAME_GAMMA), RES_GRFATR_GAMMA,        cppu::UnoType<double>::get(),     0,   0},
                    { OUString(UNO_NAME_GRAPHIC_IS_INVERTED), RES_GRFATR_INVERT,         cppu::UnoType<bool>::get(),    0,   0},
                    { OUString(UNO_NAME_TRANSPARENCY), RES_GRFATR_TRANSPARENCY, cppu::UnoType<sal_Int16>::get(),   0,   0},
                    { OUString(UNO_NAME_GRAPHIC_COLOR_MODE), RES_GRFATR_DRAWMODE,    cppu::UnoType<css::drawing::ColorMode>::get(),      0,   0},

                    //UUUU added FillProperties for SW, same as FILL_PROPERTIES in svx
                    // but need own defines in Writer due to later association of strings
                    // and uno types (see loop at end of this method and definition of SW_PROP_NMID)
                    // This entry is for adding that properties to Writer GraphicObject import/export
                    FILL_PROPERTIES_SW

                    { OUString(), 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aGraphicPropertyMap_Impl;
            }
            break;
            case PROPERTY_MAP_EMBEDDED_OBJECT:
            {
                static SfxItemPropertyMapEntry const aEmbeddedPropertyMap_Impl[] =
                {   //UUUU
                    // TODO: We should consider completely removing SvxBrushItem() stuff
                    // add support for XATTR_FILL_FIRST, XATTR_FILL_LAST
                    // COMMON_FRAME_PROPERTIES currently hosts the RES_BACKGROUND entries from SvxBrushItem
                    COMMON_FRAME_PROPERTIES
                    { OUString(UNO_NAME_SURROUND_CONTOUR), RES_SURROUND, cppu::UnoType<bool>::get(), PROPERTY_NONE, MID_SURROUND_CONTOUR },
                    { OUString(UNO_NAME_CONTOUR_OUTSIDE), RES_SURROUND, cppu::UnoType<bool>::get(), PROPERTY_NONE, MID_SURROUND_CONTOUROUTSIDE},
                    { OUString(UNO_NAME_CONTOUR_POLY_POLYGON), FN_PARAM_COUNTOUR_PP, cppu::UnoType<css::drawing::PointSequenceSequence>::get(), PropertyAttribute::MAYBEVOID, 0 },
                    { OUString(UNO_NAME_IS_PIXEL_CONTOUR), FN_UNO_IS_PIXEL_CONTOUR, cppu::UnoType<bool>::get(), PROPERTY_NONE, 0 },
                    { OUString(UNO_NAME_IS_AUTOMATIC_CONTOUR), FN_UNO_IS_AUTOMATIC_CONTOUR , cppu::UnoType<bool>::get(), PROPERTY_NONE, 0 },
                    { OUString(UNO_NAME_CLSID),                FN_UNO_CLSID, cppu::UnoType<OUString>::get(), PROPERTY_NONE, 0 },
                    { OUString(UNO_NAME_STREAM_NAME),           FN_UNO_STREAM_NAME, cppu::UnoType<OUString>::get(), PROPERTY_NONE, 0 },
                    { OUString(UNO_NAME_MODEL), FN_UNO_MODEL, cppu::UnoType<css::frame::XModel>::get(), PropertyAttribute::READONLY|PropertyAttribute::MAYBEVOID, 0},
                    { OUString(UNO_NAME_GRAPHIC_URL), FN_UNO_REPLACEMENT_GRAPHIC_URL, cppu::UnoType<OUString>::get(), PropertyAttribute::MAYBEVOID, 0 },
                    { OUString(UNO_NAME_GRAPHIC), FN_UNO_REPLACEMENT_GRAPHIC, cppu::UnoType<css::graphic::XGraphic>::get(), PropertyAttribute::MAYBEVOID, 0 },
                    { OUString(UNO_NAME_COMPONENT),FN_UNO_COMPONENT, cppu::UnoType<css::lang::XComponent>::get(), PropertyAttribute::READONLY, 0},
                    { OUString(UNO_NAME_EMBEDDED_OBJECT),FN_EMBEDDED_OBJECT, cppu::UnoType<css::embed::XEmbeddedObject>::get(), PROPERTY_NONE, 0},
                    //UUUU added FillProperties for SW, same as FILL_PROPERTIES in svx
                    // but need own defines in Writer due to later association of strings
                    // and uno types (see loop at end of this method and definition of SW_PROP_NMID)
                    // This entry is for adding that properties to OLE/EmbeddedObject import/export
                    FILL_PROPERTIES_SW

                    { OUString(), 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aEmbeddedPropertyMap_Impl;
            }
            break;
            case PROPERTY_MAP_TEXT_SHAPE:
            {
                static SfxItemPropertyMapEntry const aShapeMap_Impl[] =
                {
                    { OUString(UNO_NAME_ANCHOR_PAGE_NO), RES_ANCHOR,            cppu::UnoType<sal_Int16>::get(),           PROPERTY_NONE|PropertyAttribute::MAYBEVOID, MID_ANCHOR_PAGENUM      },
                    { OUString(UNO_NAME_ANCHOR_TYPE), RES_ANCHOR,           cppu::UnoType<css::text::TextContentAnchorType>::get(),            PROPERTY_NONE|PropertyAttribute::MAYBEVOID, MID_ANCHOR_ANCHORTYPE},
                    { OUString(UNO_NAME_ANCHOR_FRAME), RES_ANCHOR,             cppu::UnoType<css::text::XTextFrame>::get(),    PropertyAttribute::MAYBEVOID, MID_ANCHOR_ANCHORFRAME},
                    { OUString(UNO_NAME_HORI_ORIENT), RES_HORI_ORIENT,        cppu::UnoType<sal_Int16>::get(),   PROPERTY_NONE|PropertyAttribute::MAYBEVOID ,MID_HORIORIENT_ORIENT },
                    { OUString(UNO_NAME_HORI_ORIENT_POSITION), RES_HORI_ORIENT,     cppu::UnoType<sal_Int32>::get(),           PROPERTY_NONE|PropertyAttribute::MAYBEVOID ,MID_HORIORIENT_POSITION|CONVERT_TWIPS   },
                    { OUString(UNO_NAME_HORI_ORIENT_RELATION), RES_HORI_ORIENT,     cppu::UnoType<sal_Int16>::get(),           PROPERTY_NONE|PropertyAttribute::MAYBEVOID ,MID_HORIORIENT_RELATION },
                    { OUString(UNO_NAME_LEFT_MARGIN), RES_LR_SPACE,             cppu::UnoType<sal_Int32>::get(), PROPERTY_NONE|PropertyAttribute::MAYBEVOID, MID_L_MARGIN|CONVERT_TWIPS},
                    { OUString(UNO_NAME_RIGHT_MARGIN), RES_LR_SPACE,            cppu::UnoType<sal_Int32>::get(), PROPERTY_NONE|PropertyAttribute::MAYBEVOID, MID_R_MARGIN|CONVERT_TWIPS},
                    { OUString(UNO_NAME_SURROUND), RES_SURROUND,          cppu::UnoType<css::text::WrapTextMode>::get(),    PROPERTY_NONE|PropertyAttribute::MAYBEVOID, MID_SURROUND_SURROUNDTYPE },
                    { OUString(UNO_NAME_TEXT_WRAP), RES_SURROUND,           cppu::UnoType<css::text::WrapTextMode>::get(),             PROPERTY_NONE, MID_SURROUND_SURROUNDTYPE    },
                    { OUString(UNO_NAME_SURROUND_ANCHORONLY), RES_SURROUND,             cppu::UnoType<bool>::get(),             PROPERTY_NONE|PropertyAttribute::MAYBEVOID, MID_SURROUND_ANCHORONLY     },
                    { OUString(UNO_NAME_SURROUND_CONTOUR), RES_SURROUND,            cppu::UnoType<bool>::get(),             PROPERTY_NONE, MID_SURROUND_CONTOUR         },
                    { OUString(UNO_NAME_CONTOUR_OUTSIDE), RES_SURROUND,             cppu::UnoType<bool>::get(),             PROPERTY_NONE, MID_SURROUND_CONTOUROUTSIDE  },
                    { OUString(UNO_NAME_TOP_MARGIN), RES_UL_SPACE,          cppu::UnoType<sal_Int32>::get(), PROPERTY_NONE, MID_UP_MARGIN|CONVERT_TWIPS},
                    { OUString(UNO_NAME_BOTTOM_MARGIN), RES_UL_SPACE,           cppu::UnoType<sal_Int32>::get(), PROPERTY_NONE, MID_LO_MARGIN|CONVERT_TWIPS},
                    { OUString(UNO_NAME_VERT_ORIENT), RES_VERT_ORIENT,      cppu::UnoType<sal_Int16>::get(),           PROPERTY_NONE|PropertyAttribute::MAYBEVOID ,MID_VERTORIENT_ORIENT   },
                    { OUString(UNO_NAME_VERT_ORIENT_POSITION), RES_VERT_ORIENT,     cppu::UnoType<sal_Int32>::get(),           PROPERTY_NONE|PropertyAttribute::MAYBEVOID ,MID_VERTORIENT_POSITION|CONVERT_TWIPS   },
                    { OUString(UNO_NAME_VERT_ORIENT_RELATION), RES_VERT_ORIENT,     cppu::UnoType<sal_Int16>::get(),           PROPERTY_NONE|PropertyAttribute::MAYBEVOID ,MID_VERTORIENT_RELATION },
                    { OUString(UNO_NAME_TEXT_RANGE), FN_TEXT_RANGE,         cppu::UnoType<css::text::XTextRange>::get(),         PROPERTY_NONE, 0},
                    { OUString(UNO_NAME_OPAQUE), RES_OPAQUE,             cppu::UnoType<bool>::get(),            PROPERTY_NONE, 0},
                    { OUString(UNO_NAME_ANCHOR_POSITION), FN_ANCHOR_POSITION,    cppu::UnoType<css::awt::Point>::get(),    PropertyAttribute::READONLY, 0},
                    // #i26791#
                    { OUString(UNO_NAME_IS_FOLLOWING_TEXT_FLOW), RES_FOLLOW_TEXT_FLOW,     cppu::UnoType<bool>::get(), PROPERTY_NONE, 0},
                    // #i28701#
                    { OUString(UNO_NAME_WRAP_INFLUENCE_ON_POSITION), RES_WRAP_INFLUENCE_ON_OBJPOS, cppu::UnoType<sal_Int8>::get(), PROPERTY_NONE, MID_WRAP_INFLUENCE},
                    // #i28749#
                    { OUString(UNO_NAME_TRANSFORMATION_IN_HORI_L2R),
                                    FN_SHAPE_TRANSFORMATION_IN_HORI_L2R,
                                    cppu::UnoType<css::drawing::HomogenMatrix3>::get(),
                                    PropertyAttribute::READONLY, 0},
                    { OUString(UNO_NAME_POSITION_LAYOUT_DIR),
                                    FN_SHAPE_POSITION_LAYOUT_DIR,
                                    cppu::UnoType<sal_Int16>::get(),
                                    PROPERTY_NONE, 0},
                    // #i36248#
                    { OUString(UNO_NAME_STARTPOSITION_IN_HORI_L2R),
                                    FN_SHAPE_STARTPOSITION_IN_HORI_L2R,
                                    cppu::UnoType<css::awt::Point>::get(),
                                    PropertyAttribute::READONLY, 0},
                    { OUString(UNO_NAME_ENDPOSITION_IN_HORI_L2R),
                                    FN_SHAPE_ENDPOSITION_IN_HORI_L2R,
                                    cppu::UnoType<css::awt::Point>::get(),
                                    PropertyAttribute::READONLY, 0},
                    // #i71182#
                    // missing map entry for property <PageToogle>
                    { OUString(UNO_NAME_PAGE_TOGGLE), RES_HORI_ORIENT,      cppu::UnoType<bool>::get(),         PROPERTY_NONE ,MID_HORIORIENT_PAGETOGGLE },
                    { OUString(UNO_NAME_RELATIVE_HEIGHT), RES_FRM_SIZE,     cppu::UnoType<sal_Int16>::get()  ,         PROPERTY_NONE, MID_FRMSIZE_REL_HEIGHT },
                    { OUString(UNO_NAME_RELATIVE_HEIGHT_RELATION), RES_FRM_SIZE, cppu::UnoType<sal_Int16>::get(),      PROPERTY_NONE, MID_FRMSIZE_REL_HEIGHT_RELATION },
                    { OUString(UNO_NAME_RELATIVE_WIDTH), RES_FRM_SIZE,      cppu::UnoType<sal_Int16>::get()  ,         PROPERTY_NONE, MID_FRMSIZE_REL_WIDTH  },
                    { OUString(UNO_NAME_RELATIVE_WIDTH_RELATION), RES_FRM_SIZE, cppu::UnoType<sal_Int16>::get(),       PROPERTY_NONE, MID_FRMSIZE_REL_WIDTH_RELATION },
                    { OUString(UNO_NAME_TEXT_BOX), FN_TEXT_BOX, cppu::UnoType<bool>::get(), PROPERTY_NONE, 0},
                    { OUString(UNO_NAME_CHAIN_NEXT_NAME), RES_CHAIN,                cppu::UnoType<OUString>::get(),            PropertyAttribute::MAYBEVOID ,MID_CHAIN_NEXTNAME},
                    { OUString(UNO_NAME_CHAIN_PREV_NAME), RES_CHAIN,                cppu::UnoType<OUString>::get(),            PropertyAttribute::MAYBEVOID ,MID_CHAIN_PREVNAME},
                    { OUString(UNO_NAME_CHAIN_NAME),      RES_CHAIN,                cppu::UnoType<OUString>::get(),            PropertyAttribute::MAYBEVOID ,MID_CHAIN_NAME    },
                    { OUString(), 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aShapeMap_Impl;
            }
            break;
            case PROPERTY_MAP_INDEX_MARK:
            {
                static SfxItemPropertyMapEntry const aIdxMarkMap_Impl[] =
                {
                    { OUString(UNO_NAME_ALTERNATIVE_TEXT), WID_ALT_TEXT,        cppu::UnoType<OUString>::get()  ,      PROPERTY_NONE,     0},
                    { OUString(UNO_NAME_PRIMARY_KEY), WID_PRIMARY_KEY,  cppu::UnoType<OUString>::get()  ,      PROPERTY_NONE,     0},
                    { OUString(UNO_NAME_SECONDARY_KEY), WID_SECONDARY_KEY,  cppu::UnoType<OUString>::get()  ,      PROPERTY_NONE,     0},
                    { OUString(UNO_NAME_TEXT_READING), WID_TEXT_READING, cppu::UnoType<OUString>::get(), PROPERTY_NONE,     0},
                    { OUString(UNO_NAME_PRIMARY_KEY_READING), WID_PRIMARY_KEY_READING, cppu::UnoType<OUString>::get(), PROPERTY_NONE,     0},
                    { OUString(UNO_NAME_SECONDARY_KEY_READING), WID_SECONDARY_KEY_READING, cppu::UnoType<OUString>::get(), PROPERTY_NONE,     0},
                    { OUString(UNO_NAME_IS_MAIN_ENTRY), WID_MAIN_ENTRY,     cppu::UnoType<bool>::get()  ,       PROPERTY_NONE,     0},
                    COMMON_TEXT_CONTENT_PROPERTIES
                    { OUString(), 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aIdxMarkMap_Impl;
            }
            break;
            case PROPERTY_MAP_CNTIDX_MARK:
            {
                static SfxItemPropertyMapEntry const aContentMarkMap_Impl[] =
                {
                    { OUString(UNO_NAME_ALTERNATIVE_TEXT), WID_ALT_TEXT,        cppu::UnoType<OUString>::get()  ,      PROPERTY_NONE,     0},
                    { OUString(UNO_NAME_LEVEL), WID_LEVEL        ,  cppu::UnoType<sal_Int16>::get()  ,         PROPERTY_NONE,     0},
                    COMMON_TEXT_CONTENT_PROPERTIES
                    { OUString(), 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aContentMarkMap_Impl;
            }
            break;
            case PROPERTY_MAP_USER_MARK:
            {
                static SfxItemPropertyMapEntry const aUserMarkMap_Impl[] =
                {
                    { OUString(UNO_NAME_ALTERNATIVE_TEXT), WID_ALT_TEXT,        cppu::UnoType<OUString>::get()  ,      PROPERTY_NONE,     0},
                    { OUString(UNO_NAME_LEVEL),   WID_LEVEL        ,    cppu::UnoType<sal_Int16>::get()  ,         PROPERTY_NONE,     0},
                    { OUString(UNO_NAME_USER_INDEX_NAME), WID_USER_IDX_NAME,    cppu::UnoType<OUString>::get()  ,      PROPERTY_NONE,     0},
                    COMMON_TEXT_CONTENT_PROPERTIES
                    { OUString(), 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aUserMarkMap_Impl;
            }
            break;
            case PROPERTY_MAP_INDEX_IDX:
            {
                static SfxItemPropertyMapEntry const aTOXIndexMap_Impl[] =
                {
                    _BASE_INDEX_PROPERTIES_
                    { OUString(UNO_NAME_CREATE_FROM_CHAPTER), WID_CREATE_FROM_CHAPTER                 ,  cppu::UnoType<bool>::get()  , PROPERTY_NONE,     0},
                    { OUString(UNO_NAME_IS_PROTECTED), WID_PROTECTED                           ,  cppu::UnoType<bool>::get()  , PROPERTY_NONE,     0},
                    { OUString(UNO_NAME_USE_ALPHABETICAL_SEPARATORS), WID_USE_ALPHABETICAL_SEPARATORS         ,  cppu::UnoType<bool>::get()  , PROPERTY_NONE,     0},
                    { OUString(UNO_NAME_USE_KEY_AS_ENTRY), WID_USE_KEY_AS_ENTRY                    ,  cppu::UnoType<bool>::get()  , PROPERTY_NONE,     0},
                    { OUString(UNO_NAME_USE_COMBINED_ENTRIES), WID_USE_COMBINED_ENTRIES                ,  cppu::UnoType<bool>::get()  , PROPERTY_NONE,     0},
                    { OUString(UNO_NAME_IS_CASE_SENSITIVE), WID_IS_CASE_SENSITIVE                   ,  cppu::UnoType<bool>::get()  , PROPERTY_NONE,     0},
                    { OUString(UNO_NAME_USE_P_P), WID_USE_P_P                             ,  cppu::UnoType<bool>::get()  , PROPERTY_NONE,     0},
                    { OUString(UNO_NAME_USE_DASH), WID_USE_DASH                            ,  cppu::UnoType<bool>::get()  , PROPERTY_NONE,     0},
                    { OUString(UNO_NAME_USE_UPPER_CASE), WID_USE_UPPER_CASE                      ,  cppu::UnoType<bool>::get()  , PROPERTY_NONE,     0},
                    { OUString(UNO_NAME_LEVEL_FORMAT), WID_LEVEL_FORMAT                        ,  cppu::UnoType<css::container::XIndexReplace>::get()  , PROPERTY_NONE,   0},
                    { OUString(UNO_NAME_MAIN_ENTRY_CHARACTER_STYLE_NAME), WID_MAIN_ENTRY_CHARACTER_STYLE_NAME     ,  cppu::UnoType<OUString>::get()  , PROPERTY_NONE,     0},
                    { OUString(UNO_NAME_TEXT_COLUMNS), RES_COL,                cppu::UnoType<css::text::XTextColumns>::get(),    PROPERTY_NONE, MID_COLUMNS},
                    { OUString(UNO_NAME_BACK_GRAPHIC_URL), RES_BACKGROUND,      cppu::UnoType<OUString>::get(), PROPERTY_NONE ,MID_GRAPHIC_URL    },
                    { OUString(UNO_NAME_BACK_GRAPHIC_FILTER), RES_BACKGROUND,       cppu::UnoType<OUString>::get(), PROPERTY_NONE ,MID_GRAPHIC_FILTER    },
                    { OUString(UNO_NAME_BACK_GRAPHIC_LOCATION), RES_BACKGROUND,         cppu::UnoType<css::style::GraphicLocation>::get(),          PROPERTY_NONE ,MID_GRAPHIC_POSITION},
                    { OUString(UNO_NAME_BACK_COLOR), RES_BACKGROUND,            cppu::UnoType<sal_Int32>::get(),           PROPERTY_NONE ,MID_BACK_COLOR        },
                    { OUString(UNO_NAME_BACK_TRANSPARENT), RES_BACKGROUND,      cppu::UnoType<bool>::get(),         PROPERTY_NONE ,MID_GRAPHIC_TRANSPARENT       },
                    { OUString(UNO_NAME_PARA_STYLEHEADING),                 WID_PARA_HEAD,          cppu::UnoType<OUString>::get()  , 0,     0},
                    { OUString(UNO_NAME_PARA_STYLESEPARATOR),           WID_PARA_SEP,           cppu::UnoType<OUString>::get()  , 0,     0},
                    { OUString(UNO_NAME_PARA_STYLELEVEL1),              WID_PARA_LEV1,          cppu::UnoType<OUString>::get()  , 0,     0},
                    { OUString(UNO_NAME_PARA_STYLELEVEL2),              WID_PARA_LEV2,          cppu::UnoType<OUString>::get()  , 0,     0},
                    { OUString(UNO_NAME_PARA_STYLELEVEL3),              WID_PARA_LEV3,          cppu::UnoType<OUString>::get()  , 0,     0},
                    { OUString(UNO_NAME_IS_COMMA_SEPARATED), WID_IS_COMMA_SEPARATED, cppu::UnoType<bool>::get(),            PROPERTY_NONE ,0         },
                    { OUString(UNO_NAME_DOCUMENT_INDEX_MARKS), WID_INDEX_MARKS, cppu::UnoType< cppu::UnoSequenceType<css::text::XDocumentIndexMark> >::get(),           PropertyAttribute::READONLY ,0       },
                    { OUString(UNO_NAME_IS_RELATIVE_TABSTOPS), WID_IS_RELATIVE_TABSTOPS, cppu::UnoType<bool>::get(), PROPERTY_NONE, 0},
                    { OUString(UNO_NAME_LOCALE),            WID_IDX_LOCALE,         cppu::UnoType<css::lang::Locale>::get(), PROPERTY_NONE,     0},
                    { OUString(UNO_NAME_SORT_ALGORITHM),    WID_IDX_SORT_ALGORITHM,  cppu::UnoType<OUString>::get(), PROPERTY_NONE,     0},
                    { OUString(UNO_NAME_INDEX_ENTRY_TYPE), WID_INDEX_ENTRY_TYPE, cppu::UnoType<OUString>::get(), PROPERTY_NONE, 0},
                    { OUString(), 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aTOXIndexMap_Impl;
            }
            break;
            case PROPERTY_MAP_INDEX_CNTNT:
            {
                static SfxItemPropertyMapEntry const aTOXContentMap_Impl[] =
                {
                    _BASE_INDEX_PROPERTIES_
                    { OUString(UNO_NAME_LEVEL), WID_LEVEL                               ,  cppu::UnoType<sal_Int16>::get()  , PROPERTY_NONE,     0},
                    { OUString(UNO_NAME_CREATE_FROM_MARKS), WID_CREATE_FROM_MARKS                   ,  cppu::UnoType<bool>::get()  , PROPERTY_NONE,     0},
                    { OUString(UNO_NAME_HIDE_TAB_LEADER_AND_PAGE_NUMBERS), WID_HIDE_TABLEADER_PAGENUMBERS                   ,  cppu::UnoType<bool>::get()  , PROPERTY_NONE,     0},
                    { OUString(UNO_NAME_TAB_IN_TOC), WID_TAB_IN_TOC, cppu::UnoType<bool>::get(), PROPERTY_NONE, 0},
                    { OUString(UNO_NAME_TOC_BOOKMARK), WID_TOC_BOOKMARK, cppu::UnoType<OUString>::get(), PROPERTY_NONE, 0},
                    { OUString(UNO_NAME_TOC_NEWLINE), WID_TOC_NEWLINE, cppu::UnoType<bool>::get(), PROPERTY_NONE, 0},
                    { OUString(UNO_NAME_TOC_PARAGRAPH_OUTLINE_LEVEL), WID_TOC_PARAGRAPH_OUTLINE_LEVEL, cppu::UnoType<bool>::get(), PROPERTY_NONE, 0},
                    { OUString(UNO_NAME_CREATE_FROM_OUTLINE), WID_CREATE_FROM_OUTLINE                 ,  cppu::UnoType<bool>::get()  , PROPERTY_NONE,     0},
                    { OUString(UNO_NAME_CREATE_FROM_CHAPTER), WID_CREATE_FROM_CHAPTER                 ,  cppu::UnoType<bool>::get()  , PROPERTY_NONE,     0},
                    { OUString(UNO_NAME_IS_PROTECTED), WID_PROTECTED                           ,  cppu::UnoType<bool>::get()  , PROPERTY_NONE,     0},
                    { OUString(UNO_NAME_LEVEL_FORMAT), WID_LEVEL_FORMAT                        ,  cppu::UnoType<css::container::XIndexReplace>::get()  , PROPERTY_NONE,     0},
                    { OUString(UNO_NAME_LEVEL_PARAGRAPH_STYLES), WID_LEVEL_PARAGRAPH_STYLES              ,  cppu::UnoType<css::container::XIndexReplace>::get()  , PropertyAttribute::READONLY,     0},
                    { OUString(UNO_NAME_CREATE_FROM_LEVEL_PARAGRAPH_STYLES), WID_CREATE_FROM_PARAGRAPH_STYLES, cppu::UnoType<bool>::get(), PROPERTY_NONE, 0},
                    { OUString(UNO_NAME_TEXT_COLUMNS), RES_COL,                cppu::UnoType<css::text::XTextColumns>::get(),    PROPERTY_NONE, MID_COLUMNS},
                    { OUString(UNO_NAME_BACK_GRAPHIC_URL), RES_BACKGROUND,      cppu::UnoType<OUString>::get(), PROPERTY_NONE ,MID_GRAPHIC_URL    },
                    { OUString(UNO_NAME_BACK_GRAPHIC_FILTER), RES_BACKGROUND,       cppu::UnoType<OUString>::get(), PROPERTY_NONE ,MID_GRAPHIC_FILTER    },
                    { OUString(UNO_NAME_BACK_GRAPHIC_LOCATION), RES_BACKGROUND,         cppu::UnoType<css::style::GraphicLocation>::get(),          PROPERTY_NONE ,MID_GRAPHIC_POSITION},
                    { OUString(UNO_NAME_BACK_COLOR), RES_BACKGROUND,            cppu::UnoType<sal_Int32>::get(),           PROPERTY_NONE ,MID_BACK_COLOR        },
                    { OUString(UNO_NAME_BACK_TRANSPARENT), RES_BACKGROUND,      cppu::UnoType<bool>::get(),         PROPERTY_NONE ,MID_GRAPHIC_TRANSPARENT       },
                    { OUString(UNO_NAME_PARA_STYLEHEADING),     WID_PARA_HEAD,  cppu::UnoType<OUString>::get()  , 0,     0},
                    { OUString(UNO_NAME_PARA_STYLELEVEL1),  WID_PARA_LEV1,  cppu::UnoType<OUString>::get()  , 0,     0},
                    { OUString(UNO_NAME_PARA_STYLELEVEL2),  WID_PARA_LEV2,  cppu::UnoType<OUString>::get()  , 0,     0},
                    { OUString(UNO_NAME_PARA_STYLELEVEL3),  WID_PARA_LEV3,  cppu::UnoType<OUString>::get()  , 0,     0},
                    { OUString(UNO_NAME_PARA_STYLELEVEL4),  WID_PARA_LEV4,  cppu::UnoType<OUString>::get()  , 0,     0},
                    { OUString(UNO_NAME_PARA_STYLELEVEL5),  WID_PARA_LEV5,  cppu::UnoType<OUString>::get()  , 0,     0},
                    { OUString(UNO_NAME_PARA_STYLELEVEL6),  WID_PARA_LEV6,  cppu::UnoType<OUString>::get()  , 0,     0},
                    { OUString(UNO_NAME_PARA_STYLELEVEL7),  WID_PARA_LEV7,  cppu::UnoType<OUString>::get()  , 0,     0},
                    { OUString(UNO_NAME_PARA_STYLELEVEL8),  WID_PARA_LEV8,  cppu::UnoType<OUString>::get()  , 0,     0},
                    { OUString(UNO_NAME_PARA_STYLELEVEL9),  WID_PARA_LEV9,  cppu::UnoType<OUString>::get()  , 0,     0},
                    { OUString(UNO_NAME_PARA_STYLELEVEL10),     WID_PARA_LEV10,     cppu::UnoType<OUString>::get()  , 0,     0},
                    { OUString(UNO_NAME_IS_RELATIVE_TABSTOPS), WID_IS_RELATIVE_TABSTOPS, cppu::UnoType<bool>::get(), PROPERTY_NONE, 0},
                    { OUString(UNO_NAME_DOCUMENT_INDEX_MARKS), WID_INDEX_MARKS, cppu::UnoType< cppu::UnoSequenceType<css::text::XDocumentIndexMark> >::get(),           PropertyAttribute::READONLY ,0       },
                    { OUString(), 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aTOXContentMap_Impl;
            }
            break;
            case PROPERTY_MAP_INDEX_USER:
            {
                static SfxItemPropertyMapEntry const aTOXUserMap_Impl[] =
                {
                    _BASE_INDEX_PROPERTIES_
                    { OUString(UNO_NAME_CREATE_FROM_MARKS), WID_CREATE_FROM_MARKS                   ,  cppu::UnoType<bool>::get()  , PROPERTY_NONE,     0},
                    { OUString(UNO_NAME_HIDE_TAB_LEADER_AND_PAGE_NUMBERS), WID_HIDE_TABLEADER_PAGENUMBERS                   ,  cppu::UnoType<bool>::get()  , PROPERTY_NONE,     0},
                    { OUString(UNO_NAME_TAB_IN_TOC), WID_TAB_IN_TOC, cppu::UnoType<bool>::get(), PROPERTY_NONE, 0},
                    { OUString(UNO_NAME_TOC_BOOKMARK), WID_TOC_BOOKMARK, cppu::UnoType<OUString>::get(), PROPERTY_NONE, 0},
                    { OUString(UNO_NAME_TOC_NEWLINE), WID_TOC_NEWLINE, cppu::UnoType<bool>::get(), PROPERTY_NONE, 0},
                    { OUString(UNO_NAME_TOC_PARAGRAPH_OUTLINE_LEVEL), WID_TOC_PARAGRAPH_OUTLINE_LEVEL, cppu::UnoType<bool>::get(), PROPERTY_NONE, 0},
                    { OUString(UNO_NAME_CREATE_FROM_CHAPTER), WID_CREATE_FROM_CHAPTER                 ,  cppu::UnoType<bool>::get()  , PROPERTY_NONE,     0},
                    { OUString(UNO_NAME_IS_PROTECTED), WID_PROTECTED                           ,  cppu::UnoType<bool>::get()  , PROPERTY_NONE,     0},
                    { OUString(UNO_NAME_USE_LEVEL_FROM_SOURCE), WID_USE_LEVEL_FROM_SOURCE               ,  cppu::UnoType<bool>::get()  , PROPERTY_NONE,     0},
                    { OUString(UNO_NAME_LEVEL_FORMAT), WID_LEVEL_FORMAT                        ,  cppu::UnoType<css::container::XIndexReplace>::get()  , PROPERTY_NONE,0},
                    { OUString(UNO_NAME_LEVEL_PARAGRAPH_STYLES), WID_LEVEL_PARAGRAPH_STYLES              ,  cppu::UnoType<css::container::XIndexReplace>::get()  , PropertyAttribute::READONLY,0},
                    { OUString(UNO_NAME_CREATE_FROM_LEVEL_PARAGRAPH_STYLES), WID_CREATE_FROM_PARAGRAPH_STYLES, cppu::UnoType<bool>::get(), PROPERTY_NONE, 0},
                    { OUString(UNO_NAME_CREATE_FROM_TABLES), WID_CREATE_FROM_TABLES                  ,  cppu::UnoType<bool>::get()  , PROPERTY_NONE,     0},
                    { OUString(UNO_NAME_CREATE_FROM_TEXT_FRAMES), WID_CREATE_FROM_TEXT_FRAMES             ,  cppu::UnoType<bool>::get()  , PROPERTY_NONE,     0},
                    { OUString(UNO_NAME_CREATE_FROM_GRAPHIC_OBJECTS), WID_CREATE_FROM_GRAPHIC_OBJECTS         ,  cppu::UnoType<bool>::get()  , PROPERTY_NONE,     0},
                    { OUString(UNO_NAME_CREATE_FROM_EMBEDDED_OBJECTS), WID_CREATE_FROM_EMBEDDED_OBJECTS        ,  cppu::UnoType<bool>::get()  , PROPERTY_NONE,     0},
                    { OUString(UNO_NAME_TEXT_COLUMNS), RES_COL,                cppu::UnoType<css::text::XTextColumns>::get(),    PROPERTY_NONE, MID_COLUMNS},
                    { OUString(UNO_NAME_BACK_GRAPHIC_URL), RES_BACKGROUND,      cppu::UnoType<OUString>::get(), PROPERTY_NONE ,MID_GRAPHIC_URL    },
                    { OUString(UNO_NAME_BACK_GRAPHIC_FILTER), RES_BACKGROUND,       cppu::UnoType<OUString>::get(), PROPERTY_NONE ,MID_GRAPHIC_FILTER    },
                    { OUString(UNO_NAME_BACK_GRAPHIC_LOCATION), RES_BACKGROUND,         cppu::UnoType<css::style::GraphicLocation>::get(),          PROPERTY_NONE ,MID_GRAPHIC_POSITION},
                    { OUString(UNO_NAME_BACK_COLOR), RES_BACKGROUND,            cppu::UnoType<sal_Int32>::get(),           PROPERTY_NONE ,MID_BACK_COLOR        },
                    { OUString(UNO_NAME_BACK_TRANSPARENT), RES_BACKGROUND,      cppu::UnoType<bool>::get(),         PROPERTY_NONE ,MID_GRAPHIC_TRANSPARENT       },
                    { OUString(UNO_NAME_PARA_STYLEHEADING),     WID_PARA_HEAD,  cppu::UnoType<OUString>::get()  , 0,     0},
                    { OUString(UNO_NAME_PARA_STYLELEVEL1),  WID_PARA_LEV1,  cppu::UnoType<OUString>::get()  , 0,     0},
                    { OUString(UNO_NAME_PARA_STYLELEVEL2),  WID_PARA_LEV2,  cppu::UnoType<OUString>::get()  , 0,     0},
                    { OUString(UNO_NAME_PARA_STYLELEVEL3),  WID_PARA_LEV3,  cppu::UnoType<OUString>::get()  , 0,     0},
                    { OUString(UNO_NAME_PARA_STYLELEVEL4),  WID_PARA_LEV4,  cppu::UnoType<OUString>::get()  , 0,     0},
                    { OUString(UNO_NAME_PARA_STYLELEVEL5),  WID_PARA_LEV5,  cppu::UnoType<OUString>::get()  , 0,     0},
                    { OUString(UNO_NAME_PARA_STYLELEVEL6),  WID_PARA_LEV6,  cppu::UnoType<OUString>::get()  , 0,     0},
                    { OUString(UNO_NAME_PARA_STYLELEVEL7),  WID_PARA_LEV7,  cppu::UnoType<OUString>::get()  , 0,     0},
                    { OUString(UNO_NAME_PARA_STYLELEVEL8),  WID_PARA_LEV8,  cppu::UnoType<OUString>::get()  , 0,     0},
                    { OUString(UNO_NAME_PARA_STYLELEVEL9),  WID_PARA_LEV9,  cppu::UnoType<OUString>::get()  , 0,     0},
                    { OUString(UNO_NAME_PARA_STYLELEVEL10),     WID_PARA_LEV10,     cppu::UnoType<OUString>::get()  , 0,     0},
                    { OUString(UNO_NAME_DOCUMENT_INDEX_MARKS), WID_INDEX_MARKS, cppu::UnoType< cppu::UnoSequenceType<css::text::XDocumentIndexMark> >::get(),           PropertyAttribute::READONLY ,0       },
                    { OUString(UNO_NAME_IS_RELATIVE_TABSTOPS), WID_IS_RELATIVE_TABSTOPS, cppu::UnoType<bool>::get(), PROPERTY_NONE, 0},
                    { OUString(UNO_NAME_USER_INDEX_NAME), WID_USER_IDX_NAME,    cppu::UnoType<OUString>::get()  ,      PROPERTY_NONE,     0},
                    { OUString(), 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aTOXUserMap_Impl;
            }
            break;
            case PROPERTY_MAP_INDEX_TABLES:
            {
                static SfxItemPropertyMapEntry const aTOXTablesMap_Impl[] =
                {
                    _BASE_INDEX_PROPERTIES_
                    { OUString(UNO_NAME_CREATE_FROM_CHAPTER), WID_CREATE_FROM_CHAPTER                 ,  cppu::UnoType<bool>::get()  , PROPERTY_NONE,     0},
                    { OUString(UNO_NAME_CREATE_FROM_LABELS), WID_CREATE_FROM_LABELS                  ,  cppu::UnoType<bool>::get()  , PROPERTY_NONE,     0},
                    { OUString(UNO_NAME_IS_PROTECTED), WID_PROTECTED                           ,  cppu::UnoType<bool>::get()  , PROPERTY_NONE,     0},
                    { OUString(UNO_NAME_LABEL_CATEGORY), WID_LABEL_CATEGORY                      ,  cppu::UnoType<OUString>::get()  , PROPERTY_NONE,     0},
                    { OUString(UNO_NAME_LABEL_DISPLAY_TYPE), WID_LABEL_DISPLAY_TYPE                  ,  cppu::UnoType<sal_Int16>::get()  , PROPERTY_NONE,     0},
                    { OUString(UNO_NAME_LEVEL_FORMAT), WID_LEVEL_FORMAT                        ,  cppu::UnoType<css::container::XIndexReplace>::get()  , PROPERTY_NONE,0},
                    { OUString(UNO_NAME_TEXT_COLUMNS), RES_COL,                cppu::UnoType<css::text::XTextColumns>::get(),    PROPERTY_NONE, MID_COLUMNS},
                    { OUString(UNO_NAME_BACK_GRAPHIC_URL), RES_BACKGROUND,      cppu::UnoType<OUString>::get(), PROPERTY_NONE ,MID_GRAPHIC_URL    },
                    { OUString(UNO_NAME_BACK_GRAPHIC_FILTER), RES_BACKGROUND,       cppu::UnoType<OUString>::get(), PROPERTY_NONE ,MID_GRAPHIC_FILTER    },
                    { OUString(UNO_NAME_BACK_GRAPHIC_LOCATION), RES_BACKGROUND,         cppu::UnoType<css::style::GraphicLocation>::get(),          PROPERTY_NONE ,MID_GRAPHIC_POSITION},
                    { OUString(UNO_NAME_BACK_COLOR), RES_BACKGROUND,            cppu::UnoType<sal_Int32>::get(),           PROPERTY_NONE ,MID_BACK_COLOR        },
                    { OUString(UNO_NAME_BACK_TRANSPARENT), RES_BACKGROUND,      cppu::UnoType<bool>::get(),         PROPERTY_NONE ,MID_GRAPHIC_TRANSPARENT       },
                    { OUString(UNO_NAME_PARA_STYLEHEADING),     WID_PARA_HEAD,  cppu::UnoType<OUString>::get()  , 0,     0},
                    { OUString(UNO_NAME_PARA_STYLELEVEL1),  WID_PARA_LEV1,  cppu::UnoType<OUString>::get()  , 0,     0},
                    { OUString(UNO_NAME_IS_RELATIVE_TABSTOPS), WID_IS_RELATIVE_TABSTOPS, cppu::UnoType<bool>::get(), PROPERTY_NONE, 0},
                    { OUString(), 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aTOXTablesMap_Impl;
            }
            break;
            case PROPERTY_MAP_INDEX_OBJECTS:
            {
                static SfxItemPropertyMapEntry const aTOXObjectsMap_Impl[] =
                {
                    _BASE_INDEX_PROPERTIES_
                    { OUString(UNO_NAME_CREATE_FROM_CHAPTER), WID_CREATE_FROM_CHAPTER                 ,  cppu::UnoType<bool>::get()  , PROPERTY_NONE,     0},
                    { OUString(UNO_NAME_IS_PROTECTED), WID_PROTECTED                           ,  cppu::UnoType<bool>::get()  , PROPERTY_NONE,     0},
                    { OUString(UNO_NAME_USE_ALPHABETICAL_SEPARATORS), WID_USE_ALPHABETICAL_SEPARATORS         ,  cppu::UnoType<bool>::get()  , PROPERTY_NONE,     0},
                    { OUString(UNO_NAME_LEVEL_FORMAT), WID_LEVEL_FORMAT                        ,  cppu::UnoType<css::container::XIndexReplace>::get()  , PROPERTY_NONE,0},
                    { OUString(UNO_NAME_CREATE_FROM_STAR_MATH), WID_CREATE_FROM_STAR_MATH               ,  cppu::UnoType<bool>::get()  , PROPERTY_NONE,     0},
                    { OUString(UNO_NAME_CREATE_FROM_STAR_CHART), WID_CREATE_FROM_STAR_CHART              ,  cppu::UnoType<bool>::get()  , PROPERTY_NONE,     0},
                    { OUString(UNO_NAME_CREATE_FROM_STAR_CALC), WID_CREATE_FROM_STAR_CALC               ,  cppu::UnoType<bool>::get()  , PROPERTY_NONE,     0},
                    { OUString(UNO_NAME_CREATE_FROM_STAR_DRAW), WID_CREATE_FROM_STAR_DRAW               ,  cppu::UnoType<bool>::get()  , PROPERTY_NONE,     0},
                    { OUString(UNO_NAME_CREATE_FROM_OTHER_EMBEDDED_OBJECTS), WID_CREATE_FROM_OTHER_EMBEDDED_OBJECTS  ,  cppu::UnoType<bool>::get()  , PROPERTY_NONE,     0},
                    { OUString(UNO_NAME_TEXT_COLUMNS), RES_COL,                cppu::UnoType<css::text::XTextColumns>::get(),    PROPERTY_NONE, MID_COLUMNS},
                    { OUString(UNO_NAME_BACK_GRAPHIC_URL), RES_BACKGROUND,      cppu::UnoType<OUString>::get(), PROPERTY_NONE ,MID_GRAPHIC_URL    },
                    { OUString(UNO_NAME_BACK_GRAPHIC_FILTER), RES_BACKGROUND,       cppu::UnoType<OUString>::get(), PROPERTY_NONE ,MID_GRAPHIC_FILTER    },
                    { OUString(UNO_NAME_BACK_GRAPHIC_LOCATION), RES_BACKGROUND,         cppu::UnoType<css::style::GraphicLocation>::get(),          PROPERTY_NONE ,MID_GRAPHIC_POSITION},
                    { OUString(UNO_NAME_BACK_COLOR), RES_BACKGROUND,            cppu::UnoType<sal_Int32>::get(),           PROPERTY_NONE ,MID_BACK_COLOR        },
                    { OUString(UNO_NAME_BACK_TRANSPARENT), RES_BACKGROUND,      cppu::UnoType<bool>::get(),         PROPERTY_NONE ,MID_GRAPHIC_TRANSPARENT       },
                    { OUString(UNO_NAME_PARA_STYLEHEADING),     WID_PARA_HEAD,  cppu::UnoType<OUString>::get()  , 0,     0},
                    { OUString(UNO_NAME_PARA_STYLELEVEL1),  WID_PARA_LEV1,  cppu::UnoType<OUString>::get()  , 0,     0},
                    { OUString(UNO_NAME_IS_RELATIVE_TABSTOPS), WID_IS_RELATIVE_TABSTOPS, cppu::UnoType<bool>::get(), PROPERTY_NONE, 0},
                    { OUString(), 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aTOXObjectsMap_Impl;
            }
            break;
            case PROPERTY_MAP_INDEX_ILLUSTRATIONS:
            {
                static SfxItemPropertyMapEntry const aTOXIllustrationsMap_Impl[] =
                {
                    _BASE_INDEX_PROPERTIES_
                    { OUString(UNO_NAME_CREATE_FROM_CHAPTER), WID_CREATE_FROM_CHAPTER                 ,  cppu::UnoType<bool>::get()  , PROPERTY_NONE,     0},
                    { OUString(UNO_NAME_CREATE_FROM_LABELS), WID_CREATE_FROM_LABELS                  ,  cppu::UnoType<bool>::get()  , PROPERTY_NONE,     0},
                    { OUString(UNO_NAME_IS_PROTECTED), WID_PROTECTED                           ,  cppu::UnoType<bool>::get()  , PROPERTY_NONE,     0},
                    { OUString(UNO_NAME_USE_ALPHABETICAL_SEPARATORS), WID_USE_ALPHABETICAL_SEPARATORS         ,  cppu::UnoType<bool>::get()  , PROPERTY_NONE,     0},
                    { OUString(UNO_NAME_LABEL_CATEGORY), WID_LABEL_CATEGORY                      ,  cppu::UnoType<OUString>::get()  , PROPERTY_NONE,     0},
                    { OUString(UNO_NAME_LABEL_DISPLAY_TYPE), WID_LABEL_DISPLAY_TYPE                  ,  cppu::UnoType<sal_Int16>::get()  , PROPERTY_NONE,     0},
                    { OUString(UNO_NAME_LEVEL_FORMAT), WID_LEVEL_FORMAT                        ,  cppu::UnoType<css::container::XIndexReplace>::get()  , PROPERTY_NONE,0},
                    { OUString(UNO_NAME_TEXT_COLUMNS), RES_COL,                cppu::UnoType<css::text::XTextColumns>::get(),    PROPERTY_NONE, MID_COLUMNS},
                    { OUString(UNO_NAME_BACK_GRAPHIC_URL), RES_BACKGROUND,      cppu::UnoType<OUString>::get(), PROPERTY_NONE ,MID_GRAPHIC_URL    },
                    { OUString(UNO_NAME_BACK_GRAPHIC_FILTER), RES_BACKGROUND,       cppu::UnoType<OUString>::get(), PROPERTY_NONE ,MID_GRAPHIC_FILTER    },
                    { OUString(UNO_NAME_BACK_GRAPHIC_LOCATION), RES_BACKGROUND,         cppu::UnoType<css::style::GraphicLocation>::get(),          PROPERTY_NONE ,MID_GRAPHIC_POSITION},
                    { OUString(UNO_NAME_BACK_COLOR), RES_BACKGROUND,            cppu::UnoType<sal_Int32>::get(),           PROPERTY_NONE ,MID_BACK_COLOR        },
                    { OUString(UNO_NAME_BACK_TRANSPARENT), RES_BACKGROUND,      cppu::UnoType<bool>::get(),         PROPERTY_NONE ,MID_GRAPHIC_TRANSPARENT       },
                    { OUString(UNO_NAME_PARA_STYLEHEADING),     WID_PARA_HEAD,  cppu::UnoType<OUString>::get()  , 0,     0},
                    { OUString(UNO_NAME_PARA_STYLELEVEL1),  WID_PARA_LEV1,  cppu::UnoType<OUString>::get()  , 0,     0},
                    { OUString(UNO_NAME_IS_RELATIVE_TABSTOPS), WID_IS_RELATIVE_TABSTOPS, cppu::UnoType<bool>::get(), PROPERTY_NONE, 0},
                    { OUString(), 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aTOXIllustrationsMap_Impl;
            }
            break;
            case PROPERTY_MAP_TEXT_TABLE_ROW:
            {
                static SfxItemPropertyMapEntry const aTableRowPropertyMap_Impl[] =
                {
                    { OUString(UNO_NAME_BACK_COLOR), RES_BACKGROUND, cppu::UnoType<sal_Int32>::get(), PROPERTY_NONE ,MID_BACK_COLOR         },
                    { OUString(UNO_NAME_BACK_GRAPHIC_URL), RES_BACKGROUND,      cppu::UnoType<OUString>::get(), PROPERTY_NONE ,MID_GRAPHIC_URL    },
                    { OUString(UNO_NAME_BACK_GRAPHIC_FILTER), RES_BACKGROUND,       cppu::UnoType<OUString>::get(), PROPERTY_NONE ,MID_GRAPHIC_FILTER    },
                    { OUString(UNO_NAME_BACK_GRAPHIC_LOCATION), RES_BACKGROUND,         cppu::UnoType<css::style::GraphicLocation>::get(),          PROPERTY_NONE ,MID_GRAPHIC_POSITION},
                    { OUString(UNO_NAME_BACK_TRANSPARENT), RES_BACKGROUND,      cppu::UnoType<bool>::get(),         PROPERTY_NONE ,MID_GRAPHIC_TRANSPARENT       },
                    { OUString(UNO_NAME_TABLE_COLUMN_SEPARATORS), FN_UNO_TABLE_COLUMN_SEPARATORS,   cppu::UnoType< cppu::UnoSequenceType<css::text::TableColumnSeparator> >::get(),   PropertyAttribute::MAYBEVOID, 0 },
                    { OUString(UNO_NAME_HEIGHT), FN_UNO_ROW_HEIGHT,     cppu::UnoType<sal_Int32>::get(),           PROPERTY_NONE ,CONVERT_TWIPS },
                    { OUString(UNO_NAME_IS_AUTO_HEIGHT), FN_UNO_ROW_AUTO_HEIGHT,    cppu::UnoType<bool>::get(),         PROPERTY_NONE , 0 },
                    { OUString(UNO_NAME_SIZE_TYPE), RES_FRM_SIZE,           cppu::UnoType<sal_Int16>::get()  ,         PROPERTY_NONE,   MID_FRMSIZE_SIZE_TYPE  },
                    { OUString(UNO_NAME_WIDTH_TYPE), RES_FRM_SIZE,          cppu::UnoType<sal_Int16>::get()  ,         PROPERTY_NONE,   MID_FRMSIZE_WIDTH_TYPE },
                    { OUString(UNO_NAME_IS_SPLIT_ALLOWED), RES_ROW_SPLIT,       cppu::UnoType<bool>::get()  , PropertyAttribute::MAYBEVOID, 0},
                    { OUString(UNO_NAME_ROW_INTEROP_GRAB_BAG), RES_FRMATR_GRABBAG, cppu::UnoType< cppu::UnoSequenceType<css::beans::PropertyValue> >::get(), PROPERTY_NONE, 0 },
                    { OUString(), 0, css::uno::Type(), 0, 0 }
                };

                m_aMapEntriesArr[nPropertyId] = aTableRowPropertyMap_Impl;
            }
            break;
            case PROPERTY_MAP_TEXT_TABLE_CURSOR:
            {
                // The PropertySet corresponds to the Range without Chart properties
                static SfxItemPropertyMapEntry const aTableCursorPropertyMap_Impl [] =
                {
                    COMMON_CRSR_PARA_PROPERTIES_WITHOUT_FN_01
                    TABSTOPS_MAP_ENTRY

                    // attributes from PROPERTY_MAP_TABLE_CELL:
                    { OUString(UNO_NAME_BACK_COLOR), RES_BACKGROUND,    cppu::UnoType<sal_Int32>::get(),           PROPERTY_NONE , MID_BACK_COLOR       },
                    { OUString(UNO_NAME_BACK_GRAPHIC_URL), RES_BACKGROUND,      cppu::UnoType<OUString>::get(), PROPERTY_NONE ,MID_GRAPHIC_URL    },
                    { OUString(UNO_NAME_BACK_GRAPHIC_FILTER), RES_BACKGROUND,       cppu::UnoType<OUString>::get(), PROPERTY_NONE ,MID_GRAPHIC_FILTER    },
                    { OUString(UNO_NAME_BACK_GRAPHIC_LOCATION), RES_BACKGROUND,         cppu::UnoType<css::style::GraphicLocation>::get(), PROPERTY_NONE ,MID_GRAPHIC_POSITION},
                    { OUString(UNO_NAME_NUMBER_FORMAT), RES_BOXATR_FORMAT,  cppu::UnoType<sal_Int32>::get(),           PropertyAttribute::MAYBEVOID ,0             },
                    { OUString(UNO_NAME_BACK_TRANSPARENT), RES_BACKGROUND,  cppu::UnoType<bool>::get(),         PROPERTY_NONE , MID_GRAPHIC_TRANSPARENT      },
                    { OUString(UNO_NAME_USER_DEFINED_ATTRIBUTES), RES_UNKNOWNATR_CONTAINER, cppu::UnoType<css::container::XNameContainer>::get(), PropertyAttribute::MAYBEVOID, 0 },
                    { OUString(UNO_NAME_TEXT_SECTION), FN_UNO_TEXT_SECTION, cppu::UnoType<css::text::XTextSection>::get(),  PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY ,0 },
                    { OUString(UNO_NAME_IS_PROTECTED), RES_PROTECT,            cppu::UnoType<bool>::get(), 0, MID_PROTECT_CONTENT},
                    { OUString(UNO_NAME_VERT_ORIENT), RES_VERT_ORIENT,      cppu::UnoType<sal_Int16>::get(),           PROPERTY_NONE ,MID_VERTORIENT_ORIENT    },
                    { OUString(), 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aTableCursorPropertyMap_Impl;
            }
            break;
            case PROPERTY_MAP_BOOKMARK:
            {
                static SfxItemPropertyMapEntry const aBookmarkPropertyMap_Impl [] =
                {
                    { OUString(UNO_LINK_DISPLAY_NAME), FN_PARAM_LINK_DISPLAY_NAME,  cppu::UnoType<OUString>::get(), PropertyAttribute::READONLY, 0xff},
                    COMMON_TEXT_CONTENT_PROPERTIES
                    { OUString(), 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aBookmarkPropertyMap_Impl;
            }
            break;
            case PROPERTY_MAP_PARAGRAPH_EXTENSIONS:
            {
                static SfxItemPropertyMapEntry const aParagraphExtensionsMap_Impl[] =
                {
                    COMMON_TEXT_CONTENT_PROPERTIES
                    { OUString(), 0, css::uno::Type(), 0, 0 }
                };

                m_aMapEntriesArr[nPropertyId] = aParagraphExtensionsMap_Impl;
            }
            break;
            case PROPERTY_MAP_BIBLIOGRAPHY :
            {
                static SfxItemPropertyMapEntry const aBibliographyMap_Impl[] =
                {
                    _BASE_INDEX_PROPERTIES_
                    { OUString(UNO_NAME_IS_PROTECTED), WID_PROTECTED                           ,  cppu::UnoType<bool>::get()  , PROPERTY_NONE,     0},
                    { OUString(UNO_NAME_TEXT_COLUMNS), RES_COL,                cppu::UnoType<css::text::XTextColumns>::get(),    PROPERTY_NONE, MID_COLUMNS},
                    { OUString(UNO_NAME_BACK_GRAPHIC_URL), RES_BACKGROUND,      cppu::UnoType<OUString>::get(), PROPERTY_NONE ,MID_GRAPHIC_URL    },
                    { OUString(UNO_NAME_BACK_GRAPHIC_FILTER), RES_BACKGROUND,       cppu::UnoType<OUString>::get(), PROPERTY_NONE ,MID_GRAPHIC_FILTER    },
                    { OUString(UNO_NAME_BACK_GRAPHIC_LOCATION), RES_BACKGROUND,         cppu::UnoType<css::style::GraphicLocation>::get(),          PROPERTY_NONE ,MID_GRAPHIC_POSITION},
                    { OUString(UNO_NAME_BACK_COLOR), RES_BACKGROUND,            cppu::UnoType<sal_Int32>::get(),           PROPERTY_NONE ,MID_BACK_COLOR        },
                    { OUString(UNO_NAME_BACK_TRANSPARENT), RES_BACKGROUND,      cppu::UnoType<bool>::get(),         PROPERTY_NONE ,MID_GRAPHIC_TRANSPARENT       },
                    { OUString(UNO_NAME_PARA_STYLEHEADING),     WID_PARA_HEAD,  cppu::UnoType<OUString>::get()  , 0,     0},
                    { OUString(UNO_NAME_PARA_STYLELEVEL1),  WID_PARA_LEV1,  cppu::UnoType<OUString>::get()  , 0,     0},
                    { OUString(UNO_NAME_LEVEL_FORMAT), WID_LEVEL_FORMAT                        ,  cppu::UnoType<css::container::XIndexReplace>::get()  , PROPERTY_NONE,0},
                    { OUString(UNO_NAME_LOCALE),            WID_IDX_LOCALE,         cppu::UnoType<css::lang::Locale>::get(), PROPERTY_NONE,     0},
                    { OUString(UNO_NAME_SORT_ALGORITHM),    WID_IDX_SORT_ALGORITHM,  cppu::UnoType<OUString>::get(), PROPERTY_NONE,     0},
                    { OUString(), 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aBibliographyMap_Impl;
            }
            break;
            case PROPERTY_MAP_TEXT_DOCUMENT:
            {
                static SfxItemPropertyMapEntry const aDocMap_Impl[] =
                {
                    { OUString(UNO_NAME_BASIC_LIBRARIES), WID_DOC_BASIC_LIBRARIES,  cppu::UnoType<css::script::XLibraryContainer>::get(), PropertyAttribute::READONLY, 0},
                    { OUString(UNO_NAME_CHAR_FONT_NAME), RES_CHRATR_FONT,       cppu::UnoType<OUString>::get(),  PropertyAttribute::MAYBEVOID, MID_FONT_FAMILY_NAME },
                    { OUString(UNO_NAME_CHAR_FONT_STYLE_NAME), RES_CHRATR_FONT,     cppu::UnoType<OUString>::get(), PropertyAttribute::MAYBEVOID, MID_FONT_STYLE_NAME },
                    { OUString(UNO_NAME_CHAR_FONT_FAMILY), RES_CHRATR_FONT,     cppu::UnoType<sal_Int16>::get(),                   PropertyAttribute::MAYBEVOID, MID_FONT_FAMILY   },
                    { OUString(UNO_NAME_CHAR_FONT_CHAR_SET), RES_CHRATR_FONT,       cppu::UnoType<sal_Int16>::get(),   PropertyAttribute::MAYBEVOID, MID_FONT_CHAR_SET },
                    { OUString(UNO_NAME_CHAR_FONT_PITCH), RES_CHRATR_FONT,      cppu::UnoType<sal_Int16>::get(),                   PropertyAttribute::MAYBEVOID, MID_FONT_PITCH   },
                    { OUString(UNO_NAME_CHAR_FONT_NAME_ASIAN), RES_CHRATR_CJK_FONT,     cppu::UnoType<OUString>::get(),  PropertyAttribute::MAYBEVOID, MID_FONT_FAMILY_NAME },
                    { OUString(UNO_NAME_CHAR_FONT_STYLE_NAME_ASIAN), RES_CHRATR_CJK_FONT,   cppu::UnoType<OUString>::get(),    PropertyAttribute::MAYBEVOID, MID_FONT_STYLE_NAME },
                    { OUString(UNO_NAME_CHAR_FONT_FAMILY_ASIAN), RES_CHRATR_CJK_FONT,   cppu::UnoType<sal_Int16>::get(),   PropertyAttribute::MAYBEVOID, MID_FONT_FAMILY   },
                    { OUString(UNO_NAME_CHAR_FONT_CHAR_SET_ASIAN), RES_CHRATR_CJK_FONT, cppu::UnoType<sal_Int16>::get(),   PropertyAttribute::MAYBEVOID, MID_FONT_CHAR_SET },
                    { OUString(UNO_NAME_CHAR_FONT_PITCH_ASIAN), RES_CHRATR_CJK_FONT,    cppu::UnoType<sal_Int16>::get(),   PropertyAttribute::MAYBEVOID, MID_FONT_PITCH   },
                    { OUString(UNO_NAME_CHAR_FONT_NAME_COMPLEX), RES_CHRATR_CTL_FONT,   cppu::UnoType<OUString>::get(),    PropertyAttribute::MAYBEVOID, MID_FONT_FAMILY_NAME },
                    { OUString(UNO_NAME_CHAR_FONT_STYLE_NAME_COMPLEX), RES_CHRATR_CTL_FONT, cppu::UnoType<OUString>::get(),    PropertyAttribute::MAYBEVOID, MID_FONT_STYLE_NAME },
                    { OUString(UNO_NAME_CHAR_FONT_FAMILY_COMPLEX), RES_CHRATR_CTL_FONT, cppu::UnoType<sal_Int16>::get(),   PropertyAttribute::MAYBEVOID, MID_FONT_FAMILY   },
                    { OUString(UNO_NAME_CHAR_FONT_CHAR_SET_COMPLEX), RES_CHRATR_CTL_FONT,   cppu::UnoType<sal_Int16>::get(),   PropertyAttribute::MAYBEVOID, MID_FONT_CHAR_SET },
                    { OUString(UNO_NAME_CHAR_FONT_PITCH_COMPLEX), RES_CHRATR_CTL_FONT,  cppu::UnoType<sal_Int16>::get(),   PropertyAttribute::MAYBEVOID, MID_FONT_PITCH   },
                    { OUString(UNO_NAME_CHAR_LOCALE), RES_CHRATR_LANGUAGE ,   cppu::UnoType<css::lang::Locale>::get(), PropertyAttribute::MAYBEVOID,  MID_LANG_LOCALE },
                    { OUString(UNO_NAME_CHARACTER_COUNT), WID_DOC_CHAR_COUNT,           cppu::UnoType<sal_Int32>::get(),   PropertyAttribute::READONLY,   0},
                    { OUString(UNO_NAME_DIALOG_LIBRARIES), WID_DOC_DIALOG_LIBRARIES,  cppu::UnoType<css::script::XLibraryContainer>::get(), PropertyAttribute::READONLY, 0},
                    { OUString(UNO_NAME_VBA_DOCOBJ), WID_DOC_VBA_DOCOBJ,  cppu::UnoType<OUString>::get(), PropertyAttribute::READONLY, 0},
                    { OUString(UNO_NAME_INDEX_AUTO_MARK_FILE_U_R_L), WID_DOC_AUTO_MARK_URL, cppu::UnoType<OUString>::get(),    PROPERTY_NONE,   0},
                    { OUString(UNO_NAME_PARAGRAPH_COUNT), WID_DOC_PARA_COUNT,           cppu::UnoType<sal_Int32>::get(),   PropertyAttribute::READONLY,   0},
                    { OUString(UNO_NAME_RECORD_CHANGES), WID_DOC_CHANGES_RECORD,        cppu::UnoType<bool>::get(), PROPERTY_NONE,   0},
                    { OUString(UNO_NAME_SHOW_CHANGES), WID_DOC_CHANGES_SHOW,        cppu::UnoType<bool>::get(), PROPERTY_NONE,   0},
                    { OUString(UNO_NAME_WORD_COUNT), WID_DOC_WORD_COUNT,            cppu::UnoType<sal_Int32>::get(),   PropertyAttribute::READONLY,   0},
                    { OUString(UNO_NAME_IS_TEMPLATE), WID_DOC_ISTEMPLATEID,         cppu::UnoType<bool>::get(), PropertyAttribute::READONLY,   0},
                    { OUString(UNO_NAME_WORD_SEPARATOR), WID_DOC_WORD_SEPARATOR,        cppu::UnoType<OUString>::get(),    PROPERTY_NONE,   0},
                    { OUString(UNO_NAME_HIDE_FIELD_TIPS), WID_DOC_HIDE_TIPS,            cppu::UnoType<bool>::get(), PROPERTY_NONE,   0},
                    { OUString(UNO_NAME_REDLINE_DISPLAY_TYPE), WID_DOC_REDLINE_DISPLAY,     cppu::UnoType<sal_Int16>::get(),   PROPERTY_NONE,   0},
                    { OUString(UNO_NAME_REDLINE_PROTECTION_KEY), WID_DOC_CHANGES_PASSWORD,      cppu::UnoType< cppu::UnoSequenceType<sal_Int8> >::get(), PROPERTY_NONE, 0 },
                    { OUString(UNO_NAME_FORBIDDEN_CHARACTERS), WID_DOC_FORBIDDEN_CHARS,    cppu::UnoType<css::i18n::XForbiddenCharacters>::get(), PROPERTY_NONE,   0},
                    { OUString(UNO_NAME_TWO_DIGIT_YEAR), WID_DOC_TWO_DIGIT_YEAR,    cppu::UnoType<sal_Int16>::get(),   PROPERTY_NONE,   0},
                    { OUString(UNO_NAME_AUTOMATIC_CONTROL_FOCUS),       WID_DOC_AUTOMATIC_CONTROL_FOCUS,    cppu::UnoType<bool>::get(), PROPERTY_NONE,   0},
                    { OUString(UNO_NAME_APPLY_FORM_DESIGN_MODE),        WID_DOC_APPLY_FORM_DESIGN_MODE,     cppu::UnoType<bool>::get(), PROPERTY_NONE,   0},
                    { OUString(UNO_NAME_RUNTIME_UID), WID_DOC_RUNTIME_UID,  cppu::UnoType<OUString>::get(), PropertyAttribute::READONLY, 0},
                    { OUString(UNO_NAME_LOCK_UPDATES),        WID_DOC_LOCK_UPDATES,     cppu::UnoType<bool>::get(), PROPERTY_NONE,   0},
                    { OUString(UNO_NAME_HAS_VALID_SIGNATURES),  WID_DOC_HAS_VALID_SIGNATURES, cppu::UnoType<bool>::get(), PropertyAttribute::READONLY,   0},
                    { OUString(UNO_NAME_BUILDID), WID_DOC_BUILDID, cppu::UnoType<OUString>::get(), 0, 0},
                    { OUString(UNO_NAME_DOC_INTEROP_GRAB_BAG), WID_DOC_INTEROP_GRAB_BAG, cppu::UnoType< cppu::UnoSequenceType<css::beans::PropertyValue> >::get(), PROPERTY_NONE, 0 },
                    { OUString(UNO_NAME_DEFAULT_PAGE_MODE),  WID_DOC_DEFAULT_PAGE_MODE,  cppu::UnoType<bool>::get(), PROPERTY_NONE,  0},
                    { OUString(), 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aDocMap_Impl;
            }
            break;
            case PROPERTY_MAP_LINK_TARGET:
            {
                static SfxItemPropertyMapEntry const aLinkTargetMap_Impl[] =
                {
                    { OUString(UNO_LINK_DISPLAY_BITMAP), 0,     cppu::UnoType<css::awt::XBitmap>::get(), PropertyAttribute::READONLY, 0xff},
                    { OUString(UNO_LINK_DISPLAY_NAME), 0,   cppu::UnoType<OUString>::get(), PropertyAttribute::READONLY, 0xff},
                    { OUString(), 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aLinkTargetMap_Impl;
            }
            break;
            case PROPERTY_MAP_AUTO_TEXT_GROUP :
            {
                static SfxItemPropertyMapEntry const aAutoTextGroupMap_Impl[] =
                {
                    { OUString(UNO_NAME_FILE_PATH), WID_GROUP_PATH,     cppu::UnoType<OUString>::get(),    PROPERTY_NONE,   PropertyAttribute::READONLY},
                    { OUString(UNO_NAME_TITLE), WID_GROUP_TITLE, cppu::UnoType<OUString>::get(),   PROPERTY_NONE,   0},
                    { OUString(), 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aAutoTextGroupMap_Impl;
            }
            break;
            case PROPERTY_MAP_TEXTPORTION_EXTENSIONS:
            {
                static SfxItemPropertyMapEntry const aTextPortionExtensionMap_Impl[] =
                {
                    COMPLETE_TEXT_CURSOR_MAP
                    {OUString(UNO_NAME_BOOKMARK), FN_UNO_BOOKMARK, cppu::UnoType<css::text::XTextContent>::get(),   PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY ,0 },
                    {OUString(UNO_NAME_CONTROL_CHARACTER), FN_UNO_CONTROL_CHARACTER, cppu::UnoType<sal_Int16>::get(),                 PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY, MID_HYPHEN_MIN_LEAD   },
                    {OUString(UNO_NAME_IS_COLLAPSED), FN_UNO_IS_COLLAPSED, cppu::UnoType<bool>::get(),                             PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY, 0 },
                    {OUString(UNO_NAME_IS_START), FN_UNO_IS_START, cppu::UnoType<bool>::get(),                             PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY, 0 },
                    //_REDLINE_PROPERTIES
                    {OUString(UNO_NAME_TEXT_PORTION_TYPE), FN_UNO_TEXT_PORTION_TYPE, cppu::UnoType<OUString>::get(),                        PropertyAttribute::READONLY, 0},
                    {OUString(UNO_NAME_META), FN_UNO_META, cppu::UnoType<css::text::XTextContent>::get(), PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY, 0 },
                    { OUString(), 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aTextPortionExtensionMap_Impl;
            }
            break;
            case PROPERTY_MAP_FOOTNOTE:
            {
                static SfxItemPropertyMapEntry const aFootnoteMap_Impl[] =
                {
                    {OUString(UNO_NAME_REFERENCE_ID), 0, cppu::UnoType<sal_Int16>::get(),PropertyAttribute::READONLY|PropertyAttribute::MAYBEVOID, 0},
                    COMMON_TEXT_CONTENT_PROPERTIES
                    _REDLINE_NODE_PROPERTIES
                    { OUString(), 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aFootnoteMap_Impl;
            }
            break;
            case PROPERTY_MAP_TEXT_COLUMS :
            {
                static SfxItemPropertyMapEntry const aTextColumns_Impl[] =
                {
                    {OUString(UNO_NAME_IS_AUTOMATIC), WID_TXTCOL_IS_AUTOMATIC, cppu::UnoType<bool>::get(),PropertyAttribute::READONLY, 0},
                    {OUString(UNO_NAME_AUTOMATIC_DISTANCE), WID_TXTCOL_AUTO_DISTANCE, cppu::UnoType<sal_Int32>::get(),PROPERTY_NONE, 0},
                    {OUString(UNO_NAME_SEPARATOR_LINE_WIDTH), WID_TXTCOL_LINE_WIDTH, cppu::UnoType<sal_Int32>::get(),PROPERTY_NONE, 0},
                    {OUString(UNO_NAME_SEPARATOR_LINE_COLOR), WID_TXTCOL_LINE_COLOR, cppu::UnoType<sal_Int32>::get(),PROPERTY_NONE,    0},
                    {OUString(UNO_NAME_SEPARATOR_LINE_RELATIVE_HEIGHT), WID_TXTCOL_LINE_REL_HGT, cppu::UnoType<sal_Int32>::get(),PROPERTY_NONE,    0},
                    {OUString(UNO_NAME_SEPARATOR_LINE_VERTIVAL_ALIGNMENT), WID_TXTCOL_LINE_ALIGN, cppu::UnoType<css::style::VerticalAlignment>::get(),PROPERTY_NONE,   0},
                    {OUString(UNO_NAME_SEPARATOR_LINE_IS_ON), WID_TXTCOL_LINE_IS_ON, cppu::UnoType<bool>::get(),PROPERTY_NONE,  0},
                    {OUString(UNO_NAME_SEPARATOR_LINE_STYLE), WID_TXTCOL_LINE_STYLE, cppu::UnoType<sal_Int8>::get(),PROPERTY_NONE, 0},
                    { OUString(), 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aTextColumns_Impl;
            }
            break;
            case PROPERTY_MAP_REDLINE :
            {
                static SfxItemPropertyMapEntry const aRedlineMap_Impl[] =
                {
                    _REDLINE_PROPERTIES
                    _REDLINE_NODE_PROPERTIES
                    {OUString(UNO_NAME_REDLINE_START), 0, cppu::UnoType<css::uno::XInterface>::get(),  PropertyAttribute::READONLY,    0},
                    {OUString(UNO_NAME_REDLINE_END), 0, cppu::UnoType<css::uno::XInterface>::get(),    PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY,   0},
                    { OUString(), 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aRedlineMap_Impl;
            }
            break;
            case PROPERTY_MAP_TEXT_DEFAULT :
            {
                static SfxItemPropertyMapEntry aTextDefaultMap_Impl[] =
                {
                    { OUString(UNO_NAME_TAB_STOP_DISTANCE), RES_PARATR_TABSTOP,     cppu::UnoType<sal_Int32>::get(),   PROPERTY_NONE, MID_STD_TAB | CONVERT_TWIPS},
                    COMMON_CRSR_PARA_PROPERTIES_WITHOUT_FN
                    COMMON_HYPERLINK_PROPERTIES
                    { OUString(UNO_NAME_CHAR_STYLE_NAME), RES_TXTATR_CHARFMT,     cppu::UnoType<OUString>::get(),  PropertyAttribute::MAYBEVOID,     0},
                    { OUString(UNO_NAME_IS_SPLIT_ALLOWED), RES_ROW_SPLIT,       cppu::UnoType<bool>::get()  , PropertyAttribute::MAYBEVOID, 0},
                    // #i29550#
                    { OUString(UNO_NAME_COLLAPSING_BORDERS), RES_COLLAPSING_BORDERS, cppu::UnoType<bool>::get(), PROPERTY_NONE, 0},

            //text grid enhancement for better CJK support.  2007-04-01
            //just export the default page mode property, other properties are not handled in this version
                    { OUString(UNO_NAME_GRID_STANDARD_PAGE_MODE), RES_TEXTGRID, cppu::UnoType<bool>::get(), PROPERTY_NONE, MID_GRID_STANDARD_MODE},
                    { OUString(), 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aTextDefaultMap_Impl;
                for( SfxItemPropertyMapEntry * pMap = aTextDefaultMap_Impl;
                     !pMap->aName.isEmpty(); ++pMap )
                {
                    // OUString(UNO_NAME_PAGE_DESC_NAME) should keep its MAYBEVOID flag
                    if (!(RES_PAGEDESC == pMap->nWID && MID_PAGEDESC_PAGEDESCNAME == pMap->nMemberId))
                        pMap->nFlags &= ~PropertyAttribute::MAYBEVOID;
                }
            }
            break;
            case PROPERTY_MAP_REDLINE_PORTION :
            {
                static SfxItemPropertyMapEntry const aRedlinePortionMap_Impl[] =
                {
                    COMPLETE_TEXT_CURSOR_MAP
                    {OUString(UNO_NAME_BOOKMARK), FN_UNO_BOOKMARK, cppu::UnoType<css::text::XTextContent>::get(),   PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY ,0 },
                    {OUString(UNO_NAME_CONTROL_CHARACTER), FN_UNO_CONTROL_CHARACTER, cppu::UnoType<sal_Int16>::get(),                 PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY, MID_HYPHEN_MIN_LEAD   },
                    {OUString(UNO_NAME_IS_COLLAPSED), FN_UNO_IS_COLLAPSED, cppu::UnoType<bool>::get(),                             PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY, 0 },
                    {OUString(UNO_NAME_IS_START), FN_UNO_IS_START, cppu::UnoType<bool>::get(),                             PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY, 0 },
                    _REDLINE_PROPERTIES
                    {OUString(UNO_NAME_TEXT_PORTION_TYPE), FN_UNO_TEXT_PORTION_TYPE, cppu::UnoType<OUString>::get(),                        PropertyAttribute::READONLY, 0},
                    { OUString(), 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aRedlinePortionMap_Impl;
            }
            break;
            case  PROPERTY_MAP_FLDTYP_DATETIME:
            {
                static SfxItemPropertyMapEntry const aDateTimeFieldPropMap[] =
                {
                    {OUString(UNO_NAME_ADJUST), FIELD_PROP_SUBTYPE,     cppu::UnoType<sal_Int32>::get(), PROPERTY_NONE,    0},
                    {OUString(UNO_NAME_DATE_TIME_VALUE), FIELD_PROP_DATE_TIME,  cppu::UnoType<css::util::DateTime>::get(), PROPERTY_NONE, 0},
                    {OUString(UNO_NAME_IS_FIXED),       FIELD_PROP_BOOL1,   cppu::UnoType<bool>::get()  , PROPERTY_NONE,0},
                    {OUString(UNO_NAME_IS_DATE),    FIELD_PROP_BOOL2,   cppu::UnoType<bool>::get()  , PROPERTY_NONE,0},
                    {OUString(UNO_NAME_NUMBER_FORMAT), FIELD_PROP_FORMAT,   cppu::UnoType<sal_Int32>::get(), PROPERTY_NONE,    0},
                    {OUString(UNO_NAME_IS_FIXED_LANGUAGE), FIELD_PROP_BOOL4, cppu::UnoType<bool>::get(), PROPERTY_NONE,    0},
                    COMMON_FLDTYP_PROPERTIES
                    { OUString(), 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aDateTimeFieldPropMap;
            }
            break;
            case  PROPERTY_MAP_FLDTYP_USER     :
            {
                static SfxItemPropertyMapEntry const aUserFieldPropMap[] =
                {
                    {OUString(UNO_NAME_IS_SHOW_FORMULA), FIELD_PROP_BOOL2,  cppu::UnoType<bool>::get(), PROPERTY_NONE,  0},
                    {OUString(UNO_NAME_IS_VISIBLE),     FIELD_PROP_BOOL1,   cppu::UnoType<bool>::get(), PROPERTY_NONE,  0},
                    {OUString(UNO_NAME_NUMBER_FORMAT),  FIELD_PROP_FORMAT,  cppu::UnoType<sal_Int32>::get(), PROPERTY_NONE,    0},
                    {OUString(UNO_NAME_IS_FIXED_LANGUAGE), FIELD_PROP_BOOL4, cppu::UnoType<bool>::get(), PROPERTY_NONE,    0},
                    COMMON_FLDTYP_PROPERTIES
                    { OUString(), 0, css::uno::Type(), 0, 0 }
                };

                m_aMapEntriesArr[nPropertyId] = aUserFieldPropMap;
            }
            break;
            case  PROPERTY_MAP_FLDTYP_SET_EXP  :
            {
                static SfxItemPropertyMapEntry const aSetExpFieldPropMap     [] =
                {
                    {OUString(UNO_NAME_CONTENT),            FIELD_PROP_PAR2,    cppu::UnoType<OUString>::get(),   PROPERTY_NONE, 0},
                    {OUString(UNO_NAME_CURRENT_PRESENTATION), FIELD_PROP_PAR4, cppu::UnoType<OUString>::get(),  PROPERTY_NONE, 0},
                    {OUString(UNO_NAME_HINT),               FIELD_PROP_PAR3, cppu::UnoType<OUString>::get(),   PROPERTY_NONE, 0},
                    {OUString(UNO_NAME_NUMBER_FORMAT),      FIELD_PROP_FORMAT,  cppu::UnoType<sal_Int32>::get(), PROPERTY_NONE,    0},
                    {OUString(UNO_NAME_NUMBERING_TYPE),     FIELD_PROP_USHORT2, cppu::UnoType<sal_Int16>::get(), PROPERTY_NONE,    0},
                    {OUString(UNO_NAME_IS_INPUT),       FIELD_PROP_BOOL1,   cppu::UnoType<bool>::get(), PROPERTY_NONE, 0},
                    // #i69733# wrong name - OUString(UNO_NAME_IS_INPUT) expanded to "Input" instead of "IsInput"
                    {OUString(UNO_NAME_INPUT),          FIELD_PROP_BOOL1,   cppu::UnoType<bool>::get(), PROPERTY_NONE, 0},
                    {OUString(UNO_NAME_IS_SHOW_FORMULA), FIELD_PROP_BOOL3,  cppu::UnoType<bool>::get(), PROPERTY_NONE,  0},
                    {OUString(UNO_NAME_IS_VISIBLE),       FIELD_PROP_BOOL2,   cppu::UnoType<bool>::get(),    PROPERTY_NONE, 0},
                    {OUString(UNO_NAME_SEQUENCE_VALUE), FIELD_PROP_USHORT1, cppu::UnoType<sal_Int16>::get(),   PROPERTY_NONE,  0},
                    {OUString(UNO_NAME_SUB_TYPE),           FIELD_PROP_SUBTYPE, cppu::UnoType<sal_Int16>::get(), PROPERTY_NONE,    0},
                    {OUString(UNO_NAME_VALUE),          FIELD_PROP_DOUBLE,  cppu::UnoType<double>::get(),  PROPERTY_NONE,  0},
                    {OUString(UNO_NAME_VARIABLE_NAME),  FIELD_PROP_PAR1,    cppu::UnoType<OUString>::get(),   PropertyAttribute::READONLY, 0},
                    {OUString(UNO_NAME_IS_FIXED_LANGUAGE), FIELD_PROP_BOOL4, cppu::UnoType<bool>::get(), PROPERTY_NONE,    0},
                    COMMON_FLDTYP_PROPERTIES
                    { OUString(), 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aSetExpFieldPropMap;
            }
            break;
            case  PROPERTY_MAP_FLDTYP_GET_EXP  :
            {
                static SfxItemPropertyMapEntry const aGetExpFieldPropMap     [] =
                {
                    {OUString(UNO_NAME_CONTENT),            FIELD_PROP_PAR1,    cppu::UnoType<OUString>::get(),   PROPERTY_NONE, 0},
                    {OUString(UNO_NAME_CURRENT_PRESENTATION), FIELD_PROP_PAR4, cppu::UnoType<OUString>::get(),  PROPERTY_NONE, 0},
                    {OUString(UNO_NAME_IS_SHOW_FORMULA), FIELD_PROP_BOOL2,  cppu::UnoType<bool>::get(), PROPERTY_NONE,  0},
                    {OUString(UNO_NAME_NUMBER_FORMAT),  FIELD_PROP_FORMAT,  cppu::UnoType<sal_Int32>::get(), PROPERTY_NONE,    0},
                    {OUString(UNO_NAME_SUB_TYPE),           FIELD_PROP_SUBTYPE, cppu::UnoType<sal_Int16>::get(), PROPERTY_NONE,    0},
                    {OUString(UNO_NAME_VALUE),          FIELD_PROP_DOUBLE,  cppu::UnoType<double>::get(), PropertyAttribute::READONLY, 0},
                    {OUString(UNO_NAME_VARIABLE_SUBTYPE),   FIELD_PROP_USHORT1, cppu::UnoType<sal_Int16>::get(), PROPERTY_NONE,    0},
                    {OUString(UNO_NAME_IS_FIXED_LANGUAGE), FIELD_PROP_BOOL4, cppu::UnoType<bool>::get(), PROPERTY_NONE,    0},
                    COMMON_FLDTYP_PROPERTIES
                    { OUString(), 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aGetExpFieldPropMap;
            }
            break;
            case  PROPERTY_MAP_FLDTYP_FILE_NAME:
            {
                static SfxItemPropertyMapEntry const aFileNameFieldPropMap   [] =
                {
                    {OUString(UNO_NAME_CURRENT_PRESENTATION), FIELD_PROP_PAR3, cppu::UnoType<OUString>::get(),  PROPERTY_NONE, 0},
                    {OUString(UNO_NAME_FILE_FORMAT), FIELD_PROP_FORMAT, cppu::UnoType<sal_Int16>::get(), PROPERTY_NONE,    0},
                    {OUString(UNO_NAME_IS_FIXED),   FIELD_PROP_BOOL2, cppu::UnoType<bool>::get(),       PROPERTY_NONE, 0},
                    COMMON_FLDTYP_PROPERTIES
                    { OUString(), 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aFileNameFieldPropMap;
            }
            break;
            case  PROPERTY_MAP_FLDTYP_PAGE_NUM :
            {
                static SfxItemPropertyMapEntry const aPageNumFieldPropMap        [] =
                {
                    {OUString(UNO_NAME_NUMBERING_TYPE),     FIELD_PROP_FORMAT,  cppu::UnoType<sal_Int16>::get(), PROPERTY_NONE,    0},
                    {OUString(UNO_NAME_OFFSET),             FIELD_PROP_USHORT1, cppu::UnoType<sal_Int16>::get(),  PROPERTY_NONE,   0},
                    {OUString(UNO_NAME_SUB_TYPE),           FIELD_PROP_SUBTYPE, cppu::UnoType<css::text::PageNumberType>::get(), PROPERTY_NONE,  0},
                    {OUString(UNO_NAME_USERTEXT),           FIELD_PROP_PAR1,    cppu::UnoType<OUString>::get(), PROPERTY_NONE, 0},
                    COMMON_FLDTYP_PROPERTIES
                    { OUString(), 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aPageNumFieldPropMap;
            }
            break;
            case  PROPERTY_MAP_FLDTYP_AUTHOR   :
            {
                static SfxItemPropertyMapEntry const aAuthorFieldPropMap     [] =
                {
                    {OUString(UNO_NAME_CONTENT),    FIELD_PROP_PAR1, cppu::UnoType<OUString>::get(),   PROPERTY_NONE, 0},
                    {OUString(UNO_NAME_CURRENT_PRESENTATION), FIELD_PROP_PAR1, cppu::UnoType<OUString>::get(),  PROPERTY_NONE, 0},
                    {OUString(UNO_NAME_IS_FIXED),   FIELD_PROP_BOOL2, cppu::UnoType<bool>::get(),       PROPERTY_NONE, 0},
                    {OUString(UNO_NAME_FULL_NAME),FIELD_PROP_BOOL1, cppu::UnoType<bool>::get(),     PROPERTY_NONE, 0},
                    COMMON_FLDTYP_PROPERTIES
                    { OUString(), 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aAuthorFieldPropMap;
            }
            break;
            case  PROPERTY_MAP_FLDTYP_CHAPTER  :
            {
                static SfxItemPropertyMapEntry const aChapterFieldPropMap        [] =
                {
                    {OUString(UNO_NAME_CHAPTER_FORMAT),FIELD_PROP_USHORT1,  cppu::UnoType<sal_Int16>::get(),   PROPERTY_NONE, 0},
                    {OUString(UNO_NAME_LEVEL),FIELD_PROP_BYTE1,         cppu::UnoType<sal_Int8>::get(),    PROPERTY_NONE, 0},
                    COMMON_FLDTYP_PROPERTIES
                    { OUString(), 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aChapterFieldPropMap;
            }
            break;
            case PROPERTY_MAP_FLDTYP_GET_REFERENCE          :
            {
                static SfxItemPropertyMapEntry const aGetRefFieldPropMap     [] =
                {
                    {OUString(UNO_NAME_CURRENT_PRESENTATION), FIELD_PROP_PAR3, cppu::UnoType<OUString>::get(),  PROPERTY_NONE, 0},
                    {OUString(UNO_NAME_REFERENCE_FIELD_PART),FIELD_PROP_USHORT1, cppu::UnoType<sal_Int16>::get(),  PROPERTY_NONE,  0},
                    {OUString(UNO_NAME_REFERENCE_FIELD_SOURCE),FIELD_PROP_USHORT2, cppu::UnoType<sal_Int16>::get(),    PROPERTY_NONE,  0},
                    {OUString(UNO_NAME_SEQUENCE_NUMBER),    FIELD_PROP_SHORT1,  cppu::UnoType<sal_Int16>::get(),   PROPERTY_NONE, 0},
                    {OUString(UNO_NAME_SOURCE_NAME),        FIELD_PROP_PAR1,    cppu::UnoType<OUString>::get(),   PROPERTY_NONE, 0},
                    COMMON_FLDTYP_PROPERTIES
                    { OUString(), 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aGetRefFieldPropMap;
            }
            break;
            case PROPERTY_MAP_FLDTYP_CONDITIONED_TEXT      :
            {
                static SfxItemPropertyMapEntry const aConditionedTextFieldPropMap [] =
                {
                    {OUString(UNO_NAME_CONDITION),      FIELD_PROP_PAR1, cppu::UnoType<OUString>::get(),   PROPERTY_NONE, 0},
                    {OUString(UNO_NAME_FALSE_CONTENT),  FIELD_PROP_PAR3, cppu::UnoType<OUString>::get(),   PROPERTY_NONE, 0},
                    {OUString(UNO_NAME_IS_CONDITION_TRUE) ,  FIELD_PROP_BOOL1, cppu::UnoType<bool>::get(),   PROPERTY_NONE, 0},
                    {OUString(UNO_NAME_TRUE_CONTENT) ,  FIELD_PROP_PAR2, cppu::UnoType<OUString>::get(),   PROPERTY_NONE, 0},
                    {OUString(UNO_NAME_CURRENT_PRESENTATION), FIELD_PROP_PAR4, cppu::UnoType<OUString>::get(),  PROPERTY_NONE, 0},
                    COMMON_FLDTYP_PROPERTIES
                    { OUString(), 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aConditionedTextFieldPropMap;
            }
            break;
            case PROPERTY_MAP_FLDTYP_HIDDEN_TEXT :
            {
                static SfxItemPropertyMapEntry const aHiddenTextFieldPropMap  [] =
                {
                    {OUString(UNO_NAME_CONDITION),      FIELD_PROP_PAR1, cppu::UnoType<OUString>::get(),   PROPERTY_NONE, 0},
                    {OUString(UNO_NAME_CONTENT) ,       FIELD_PROP_PAR2, cppu::UnoType<OUString>::get(),   PROPERTY_NONE, 0},
                    {OUString(UNO_NAME_IS_HIDDEN) ,     FIELD_PROP_BOOL1, cppu::UnoType<bool>::get(),   PROPERTY_NONE, 0},
                    {OUString(UNO_NAME_CURRENT_PRESENTATION), FIELD_PROP_PAR4, cppu::UnoType<OUString>::get(),  PROPERTY_NONE, 0},
                    COMMON_FLDTYP_PROPERTIES
                    { OUString(), 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aHiddenTextFieldPropMap;
            }
            break;
            case PROPERTY_MAP_FLDTYP_ANNOTATION            :
            {
                static SfxItemPropertyMapEntry const aAnnotationFieldPropMap [] =
                {
                    {OUString(UNO_NAME_AUTHOR), FIELD_PROP_PAR1,    cppu::UnoType<OUString>::get(),   PROPERTY_NONE, 0},
                    {OUString(UNO_NAME_CONTENT),    FIELD_PROP_PAR2,    cppu::UnoType<OUString>::get(),   PROPERTY_NONE, 0},
                    {OUString(UNO_NAME_INITIALS),   FIELD_PROP_PAR3,    cppu::UnoType<OUString>::get(),   PROPERTY_NONE, 0},
                    {OUString(UNO_NAME_NAME),       FIELD_PROP_PAR4,    cppu::UnoType<OUString>::get(),   PROPERTY_NONE, 0},
                    {OUString(UNO_NAME_DATE_TIME_VALUE),    FIELD_PROP_DATE_TIME,   cppu::UnoType<css::util::DateTime>::get(),    PROPERTY_NONE, 0},
                    {OUString(UNO_NAME_DATE),    FIELD_PROP_DATE,   cppu::UnoType<css::util::Date>::get(),    PROPERTY_NONE, 0},
                    {OUString(UNO_NAME_TEXT_RANGE), FIELD_PROP_TEXT, cppu::UnoType<css::uno::XInterface>::get(),  PropertyAttribute::READONLY,    0},
                    COMMON_FLDTYP_PROPERTIES
                    { OUString(), 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aAnnotationFieldPropMap;
            }
            break;
            case PROPERTY_MAP_FLDTYP_INPUT:
            {
                static SfxItemPropertyMapEntry const aInputFieldPropMap      [] =
                {
                    {OUString(UNO_NAME_CONTENT),    FIELD_PROP_PAR1, cppu::UnoType<OUString>::get(),   PROPERTY_NONE, 0},
                    {OUString(UNO_NAME_HINT),       FIELD_PROP_PAR2, cppu::UnoType<OUString>::get(),   PROPERTY_NONE, 0},
                    {OUString(UNO_NAME_HELP),       FIELD_PROP_PAR3, cppu::UnoType<OUString>::get(),   PROPERTY_NONE, 0},
                    {OUString(UNO_NAME_TOOLTIP),        FIELD_PROP_PAR4, cppu::UnoType<OUString>::get(),   PROPERTY_NONE, 0},
                    COMMON_FLDTYP_PROPERTIES
                    { OUString(), 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aInputFieldPropMap;
            }
            break;
            case PROPERTY_MAP_FLDTYP_MACRO                 :
            {
                static SfxItemPropertyMapEntry const aMacroFieldPropMap      [] =
                {
                    {OUString(UNO_NAME_HINT), FIELD_PROP_PAR2, cppu::UnoType<OUString>::get(),   PROPERTY_NONE, 0},
                    {OUString(UNO_NAME_MACRO_NAME),FIELD_PROP_PAR1, cppu::UnoType<OUString>::get(),   PROPERTY_NONE, 0},
                    {OUString(UNO_NAME_MACRO_LIBRARY),FIELD_PROP_PAR3, cppu::UnoType<OUString>::get(),PROPERTY_NONE, 0},
                    {OUString(UNO_NAME_SCRIPT_URL),FIELD_PROP_PAR4, cppu::UnoType<OUString>::get(),PROPERTY_NONE, 0},
                    COMMON_FLDTYP_PROPERTIES
                    { OUString(), 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aMacroFieldPropMap;
            }
            break;
            case PROPERTY_MAP_FLDTYP_DDE                   :
            {
                static SfxItemPropertyMapEntry const aDDEFieldPropMap            [] =
                {
                    COMMON_FLDTYP_PROPERTIES
                    { OUString(), 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aDDEFieldPropMap;
            }
            break;
            case PROPERTY_MAP_FLDTYP_DROPDOWN :
            {
                static SfxItemPropertyMapEntry const aDropDownMap            [] =
                {
                    {OUString(UNO_NAME_ITEMS), FIELD_PROP_STRINGS, cppu::UnoType< cppu::UnoSequenceType<OUString> >::get(), PROPERTY_NONE, 0},
                    {OUString(UNO_NAME_SELITEM), FIELD_PROP_PAR1, cppu::UnoType<OUString>::get(), PROPERTY_NONE, 0},
                    {OUString(UNO_NAME_NAME), FIELD_PROP_PAR2, cppu::UnoType<OUString>::get(), PROPERTY_NONE, 0},
                    {OUString(UNO_NAME_HELP), FIELD_PROP_PAR3, cppu::UnoType<OUString>::get(), PROPERTY_NONE, 0},
                    {OUString(UNO_NAME_TOOLTIP), FIELD_PROP_PAR4, cppu::UnoType<OUString>::get(), PROPERTY_NONE, 0},
                    COMMON_FLDTYP_PROPERTIES
                    { OUString(), 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aDropDownMap;
            }
            break;
            case PROPERTY_MAP_FLDTYP_HIDDEN_PARA           :
            {
                static SfxItemPropertyMapEntry const aHiddenParaFieldPropMap [] =
                {
                    {OUString(UNO_NAME_CONDITION),FIELD_PROP_PAR1, cppu::UnoType<OUString>::get(),   PROPERTY_NONE, 0},
                    {OUString(UNO_NAME_IS_HIDDEN) ,  FIELD_PROP_BOOL1, cppu::UnoType<bool>::get(),   PROPERTY_NONE, 0},
                    COMMON_FLDTYP_PROPERTIES
                    { OUString(), 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aHiddenParaFieldPropMap;
            }
            break;
            case PROPERTY_MAP_FLDTYP_DOC_INFO              :
            {
                static SfxItemPropertyMapEntry const aDocInfoFieldPropMap        [] =
                {
                    {OUString(UNO_NAME_IS_FIXED),       FIELD_PROP_BOOL1,   cppu::UnoType<bool>::get(),     PROPERTY_NONE, 0},
                    {OUString(UNO_NAME_INFO_FORMAT),    FIELD_PROP_USHORT2, cppu::UnoType<sal_Int16>::get(), PROPERTY_NONE,    0},
                    {OUString(UNO_NAME_INFO_TYPE),  FIELD_PROP_USHORT1, cppu::UnoType<sal_Int16>::get(), PROPERTY_NONE,    0},
                    COMMON_FLDTYP_PROPERTIES
                    { OUString(), 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aDocInfoFieldPropMap;
            }
            break;
            case PROPERTY_MAP_FLDTYP_TEMPLATE_NAME         :
            {
                static SfxItemPropertyMapEntry const aTmplNameFieldPropMap   [] =
                {
                    {OUString(UNO_NAME_FILE_FORMAT), FIELD_PROP_FORMAT, cppu::UnoType<sal_Int16>::get(), PROPERTY_NONE,    0},
                    COMMON_FLDTYP_PROPERTIES
                    { OUString(), 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aTmplNameFieldPropMap;
            }
            break;
            case PROPERTY_MAP_FLDTYP_USER_EXT              :
            {
                static SfxItemPropertyMapEntry const aUsrExtFieldPropMap     [] =
                {
                    {OUString(UNO_NAME_CONTENT),            FIELD_PROP_PAR1,    cppu::UnoType<OUString>::get(),   PROPERTY_NONE, 0},
                    {OUString(UNO_NAME_CURRENT_PRESENTATION), FIELD_PROP_PAR1, cppu::UnoType<OUString>::get(),  PROPERTY_NONE, 0},
                    {OUString(UNO_NAME_IS_FIXED),           FIELD_PROP_BOOL1,   cppu::UnoType<bool>::get(),     PROPERTY_NONE, 0},
                    {OUString(UNO_NAME_USER_DATA_TYPE), FIELD_PROP_USHORT1, cppu::UnoType<sal_Int16>::get(),   PROPERTY_NONE, 0},
                    COMMON_FLDTYP_PROPERTIES
                    { OUString(), 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId]= aUsrExtFieldPropMap;
            }
            break;
            case PROPERTY_MAP_FLDTYP_REF_PAGE_SET          :
            {
                static SfxItemPropertyMapEntry const aRefPgSetFieldPropMap   [] =
                {
                    {OUString(UNO_NAME_OFFSET),     FIELD_PROP_USHORT1, cppu::UnoType<sal_Int16>::get(),   PROPERTY_NONE,  0},
                    {OUString(UNO_NAME_ON),     FIELD_PROP_BOOL1,   cppu::UnoType<bool>::get(),     PROPERTY_NONE, 0},
                    COMMON_FLDTYP_PROPERTIES
                    { OUString(), 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aRefPgSetFieldPropMap;
            }
            break;
            case PROPERTY_MAP_FLDTYP_REF_PAGE_GET          :
            {
                static SfxItemPropertyMapEntry const aRefPgGetFieldPropMap   [] =
                {
                    {OUString(UNO_NAME_CURRENT_PRESENTATION), FIELD_PROP_PAR1, cppu::UnoType<OUString>::get(),  PROPERTY_NONE, 0},
                    {OUString(UNO_NAME_NUMBERING_TYPE),     FIELD_PROP_USHORT1, cppu::UnoType<sal_Int16>::get(), PROPERTY_NONE,    0},
                    COMMON_FLDTYP_PROPERTIES
                    { OUString(), 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aRefPgGetFieldPropMap;
            }
            break;
            case PROPERTY_MAP_FLDTYP_JUMP_EDIT             :
            {
                static SfxItemPropertyMapEntry const aJumpEdtFieldPropMap        [] =
                {
                    {OUString(UNO_NAME_HINT),               FIELD_PROP_PAR1, cppu::UnoType<OUString>::get(),   PROPERTY_NONE, 0},
                    {OUString(UNO_NAME_PLACEHOLDER),        FIELD_PROP_PAR2, cppu::UnoType<OUString>::get(),   PROPERTY_NONE, 0},
                    {OUString(UNO_NAME_PLACEHOLDER_TYPE), FIELD_PROP_USHORT1, cppu::UnoType<sal_Int16>::get(),     PROPERTY_NONE,  0},
                    COMMON_FLDTYP_PROPERTIES
                    { OUString(), 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aJumpEdtFieldPropMap;
            }
            break;
            case PROPERTY_MAP_FLDTYP_SCRIPT                :
            {
                static SfxItemPropertyMapEntry const aScriptFieldPropMap     [] =
                {
                    {OUString(UNO_NAME_CONTENT),        FIELD_PROP_PAR2, cppu::UnoType<OUString>::get(),   PROPERTY_NONE, 0},
                    {OUString(UNO_NAME_SCRIPT_TYPE),    FIELD_PROP_PAR1, cppu::UnoType<OUString>::get(),   PROPERTY_NONE, 0},
                    {OUString(UNO_NAME_URL_CONTENT),    FIELD_PROP_BOOL1, cppu::UnoType<bool>::get(),       PROPERTY_NONE, 0},
                    COMMON_FLDTYP_PROPERTIES
                    { OUString(), 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aScriptFieldPropMap;
            }
            break;
            case PROPERTY_MAP_FLDTYP_DATABASE_NEXT_SET     :
            {
                static SfxItemPropertyMapEntry const aDBNextSetFieldPropMap  [] =
                {
                    // Note: DATA_BASE_NAME and DATA_BASE_URL
                    // are mapped to the same nMId, because internally  we only use
                    // them as DataSource and it does not matter which one it is.

                    {OUString(UNO_NAME_DATA_BASE_NAME) , FIELD_PROP_PAR1, cppu::UnoType<OUString>::get(),   PROPERTY_NONE, 0},
                    {OUString(UNO_NAME_DATA_TABLE_NAME) , FIELD_PROP_PAR2, cppu::UnoType<OUString>::get(),   PROPERTY_NONE, 0},
                    {OUString(UNO_NAME_CONDITION)   ,     FIELD_PROP_PAR3, cppu::UnoType<OUString>::get(),   PROPERTY_NONE, 0},
                    {OUString(UNO_NAME_DATA_BASE_URL) ,  FIELD_PROP_PAR1, cppu::UnoType<OUString>::get(),   PROPERTY_NONE, 0},
                    {OUString(UNO_NAME_DATA_COMMAND_TYPE), FIELD_PROP_SHORT1, cppu::UnoType<sal_Int32>::get(),   PROPERTY_NONE, 0},
                    COMMON_FLDTYP_PROPERTIES
                    { OUString(), 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aDBNextSetFieldPropMap;
            }
            break;
            case PROPERTY_MAP_FLDTYP_DATABASE_NUM_SET      :
            {
                static SfxItemPropertyMapEntry const aDBNumSetFieldPropMap   [] =
                {
                    // Note: DATA_BASE_NAME and DATA_BASE_URL
                    // are mapped to the same nMId, because internally  we only use
                    // them as DataSource and it does not matter which one it is.

                    {OUString(UNO_NAME_DATA_BASE_NAME) ,  FIELD_PROP_PAR1, cppu::UnoType<OUString>::get(),   PROPERTY_NONE, 0},
                    {OUString(UNO_NAME_DATA_TABLE_NAME), FIELD_PROP_PAR2, cppu::UnoType<OUString>::get(),   PROPERTY_NONE, 0},
                    {OUString(UNO_NAME_CONDITION),         FIELD_PROP_PAR3, cppu::UnoType<OUString>::get(),   PROPERTY_NONE, 0},
                    {OUString(UNO_NAME_DATA_BASE_URL) ,   FIELD_PROP_PAR1, cppu::UnoType<OUString>::get(),   PROPERTY_NONE, 0},
                    {OUString(UNO_NAME_DATA_COMMAND_TYPE), FIELD_PROP_SHORT1, cppu::UnoType<sal_Int32>::get(),   PROPERTY_NONE, 0},
                    {OUString(UNO_NAME_SET_NUMBER), FIELD_PROP_FORMAT, cppu::UnoType<sal_Int32>::get(), PROPERTY_NONE,  0},
                    COMMON_FLDTYP_PROPERTIES
                    { OUString(), 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aDBNumSetFieldPropMap;
            }
            break;
            case PROPERTY_MAP_FLDTYP_DATABASE_SET_NUM      :
            {
                static SfxItemPropertyMapEntry const aDBSetNumFieldPropMap   [] =
                {
                    // Note: DATA_BASE_NAME and DATA_BASE_URL
                    // are mapped to the same nMId, because internally  we only use
                    // them as DataSource and it does not matter which one it is.

                    {OUString(UNO_NAME_DATA_BASE_NAME) , FIELD_PROP_PAR1, cppu::UnoType<OUString>::get(),   PROPERTY_NONE, 0},
                    {OUString(UNO_NAME_DATA_TABLE_NAME) , FIELD_PROP_PAR2, cppu::UnoType<OUString>::get(),   PROPERTY_NONE, 0},
                    {OUString(UNO_NAME_DATA_BASE_URL) ,  FIELD_PROP_PAR1, cppu::UnoType<OUString>::get(),   PROPERTY_NONE, 0},
                    {OUString(UNO_NAME_DATA_COMMAND_TYPE), FIELD_PROP_SHORT1, cppu::UnoType<sal_Int32>::get(),   PROPERTY_NONE, 0},
                    {OUString(UNO_NAME_NUMBERING_TYPE),       FIELD_PROP_USHORT1, cppu::UnoType<sal_Int16>::get(), PROPERTY_NONE,  0},
                    {OUString(UNO_NAME_SET_NUMBER), FIELD_PROP_FORMAT, cppu::UnoType<sal_Int32>::get(), PROPERTY_NONE,  0},
                    {OUString(UNO_NAME_IS_VISIBLE),       FIELD_PROP_BOOL2,   cppu::UnoType<bool>::get(),    PROPERTY_NONE, 0},
                    COMMON_FLDTYP_PROPERTIES
                    { OUString(), 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aDBSetNumFieldPropMap;
            }
            break;
            case PROPERTY_MAP_FLDTYP_DATABASE              :
            {
                static SfxItemPropertyMapEntry const aDBFieldPropMap         [] =
                {
                    {OUString(UNO_NAME_CONTENT),            FIELD_PROP_PAR1,    cppu::UnoType<OUString>::get(), PROPERTY_NONE, 0},
                    {OUString(UNO_NAME_CURRENT_PRESENTATION), FIELD_PROP_PAR1, cppu::UnoType<OUString>::get(),  PROPERTY_NONE, 0},
                    {OUString(UNO_NAME_FIELD_CODE),         FIELD_PROP_PAR2, cppu::UnoType<OUString>::get(),  PROPERTY_NONE, 0},
                    {OUString(UNO_NAME_IS_DATA_BASE_FORMAT),FIELD_PROP_BOOL1, cppu::UnoType<bool>::get()  , PROPERTY_NONE,0},
                    {OUString(UNO_NAME_NUMBER_FORMAT),      FIELD_PROP_FORMAT, cppu::UnoType<sal_Int32>::get(), PROPERTY_NONE, 0},
                    {OUString(UNO_NAME_IS_VISIBLE),       FIELD_PROP_BOOL2,   cppu::UnoType<bool>::get(),    PROPERTY_NONE, 0},
                    COMMON_FLDTYP_PROPERTIES
                    { OUString(), 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aDBFieldPropMap;
            }
            break;
            case PROPERTY_MAP_FLDTYP_DATABASE_NAME         :
            {
                static SfxItemPropertyMapEntry const aDBNameFieldPropMap     [] =
                {
                    // Note: DATA_BASE_NAME and DATA_BASE_URL
                    // are mapped to the same nMId, because internally  we only use
                    // them as DataSource and it does not matter which one it is.

                    {OUString(UNO_NAME_DATA_BASE_NAME) , FIELD_PROP_PAR1, cppu::UnoType<OUString>::get(),   PROPERTY_NONE, 0},
                    {OUString(UNO_NAME_DATA_TABLE_NAME) , FIELD_PROP_PAR2, cppu::UnoType<OUString>::get(),   PROPERTY_NONE, 0},
                    {OUString(UNO_NAME_DATA_BASE_URL) ,  FIELD_PROP_PAR1, cppu::UnoType<OUString>::get(),   PROPERTY_NONE, 0},
                    {OUString(UNO_NAME_DATA_COMMAND_TYPE), FIELD_PROP_SHORT1, cppu::UnoType<sal_Int32>::get(),   PROPERTY_NONE, 0},
                    {OUString(UNO_NAME_IS_VISIBLE),       FIELD_PROP_BOOL2,   cppu::UnoType<bool>::get(),    PROPERTY_NONE, 0},
                    COMMON_FLDTYP_PROPERTIES
                    { OUString(), 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aDBNameFieldPropMap;
            }
            break;
            case PROPERTY_MAP_FLDTYP_DOCSTAT:
            {
                static SfxItemPropertyMapEntry const aDocstatFieldPropMap        [] =
                {
                    {OUString(UNO_NAME_NUMBERING_TYPE),     FIELD_PROP_USHORT2, cppu::UnoType<sal_Int16>::get(), PROPERTY_NONE,    0},
                //  {OUString(UNO_NAME_STATISTIC_TYPE_ID),FIELD_PROP_USHORT1, cppu::UnoType<sal_Int16>::get(),   PROPERTY_NONE,  0},
                    COMMON_FLDTYP_PROPERTIES
                    { OUString(), 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aDocstatFieldPropMap;
            }
            break;
            case PROPERTY_MAP_FLDTYP_DOCINFO_AUTHOR:
            {
                static SfxItemPropertyMapEntry const aDocInfoAuthorPropMap           [] =
                {
                    {OUString(UNO_NAME_AUTHOR), FIELD_PROP_PAR1,    cppu::UnoType<OUString>::get(), PROPERTY_NONE, 0},
                    {OUString(UNO_NAME_CURRENT_PRESENTATION), FIELD_PROP_PAR3, cppu::UnoType<OUString>::get(),  PROPERTY_NONE, 0},
                    {OUString(UNO_NAME_IS_FIXED),   FIELD_PROP_BOOL1,   cppu::UnoType<bool>::get()  , PROPERTY_NONE,0},
                    COMMON_FLDTYP_PROPERTIES
                    { OUString(), 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aDocInfoAuthorPropMap;
            }
            break;
            case PROPERTY_MAP_FLDTYP_DOCINFO_DATE_TIME:
            {
                static SfxItemPropertyMapEntry const aDocInfoDateTimePropMap         [] =
                {
                    {OUString(UNO_NAME_CURRENT_PRESENTATION), FIELD_PROP_PAR3, cppu::UnoType<OUString>::get(),  PROPERTY_NONE, 0},
                    {OUString(UNO_NAME_DATE_TIME_VALUE),        FIELD_PROP_DOUBLE,  cppu::UnoType<double>::get(), PropertyAttribute::READONLY, 0},
                    {OUString(UNO_NAME_IS_DATE),    FIELD_PROP_BOOL2,   cppu::UnoType<bool>::get()  , PROPERTY_NONE,0},
                    {OUString(UNO_NAME_NUMBER_FORMAT),FIELD_PROP_FORMAT,    cppu::UnoType<sal_Int32>::get(), PROPERTY_NONE,    0},
                    {OUString(UNO_NAME_IS_FIXED),       FIELD_PROP_BOOL1,   cppu::UnoType<bool>::get()  , PROPERTY_NONE,    0},
                    {OUString(UNO_NAME_IS_FIXED_LANGUAGE), FIELD_PROP_BOOL4, cppu::UnoType<bool>::get(), PROPERTY_NONE,    0},
                    { OUString(), 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aDocInfoDateTimePropMap;
            }
            break;
            case PROPERTY_MAP_FLDTYP_DOCINFO_EDIT_TIME     :
            {
                static SfxItemPropertyMapEntry const aDocInfoEditTimePropMap         [] =
                {
                    {OUString(UNO_NAME_CURRENT_PRESENTATION), FIELD_PROP_PAR3, cppu::UnoType<OUString>::get(),  PROPERTY_NONE, 0},
                    {OUString(UNO_NAME_DATE_TIME_VALUE),        FIELD_PROP_DOUBLE,  cppu::UnoType<double>::get(), PropertyAttribute::READONLY, 0},
                    {OUString(UNO_NAME_NUMBER_FORMAT),FIELD_PROP_FORMAT,    cppu::UnoType<sal_Int32>::get(), PROPERTY_NONE,    0},
                    {OUString(UNO_NAME_IS_FIXED),       FIELD_PROP_BOOL1,   cppu::UnoType<bool>::get()  , PROPERTY_NONE,    0},
                    {OUString(UNO_NAME_IS_FIXED_LANGUAGE), FIELD_PROP_BOOL4, cppu::UnoType<bool>::get(), PROPERTY_NONE,    0},
                    COMMON_FLDTYP_PROPERTIES
                    { OUString(), 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aDocInfoEditTimePropMap;
            }
            break;
            case PROPERTY_MAP_FLDTYP_DOCINFO_MISC:
            {
                static SfxItemPropertyMapEntry const aDocInfoStringContentPropMap            [] =
                {
                    {OUString(UNO_NAME_CONTENT),    FIELD_PROP_PAR1,    cppu::UnoType<OUString>::get(), PROPERTY_NONE, 0},
                    {OUString(UNO_NAME_CURRENT_PRESENTATION), FIELD_PROP_PAR3, cppu::UnoType<OUString>::get(),  PROPERTY_NONE, 0},
                    {OUString(UNO_NAME_IS_FIXED),   FIELD_PROP_BOOL1,   cppu::UnoType<bool>::get()  , PROPERTY_NONE,0},
                    COMMON_FLDTYP_PROPERTIES
                    { OUString(), 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aDocInfoStringContentPropMap;
            }
            break;
            case PROPERTY_MAP_FLDTYP_DOCINFO_CUSTOM:
            {
                static SfxItemPropertyMapEntry const aDocInfoCustomPropMap           [] =
                {
                    {OUString(UNO_NAME_NAME),   FIELD_PROP_PAR4,    cppu::UnoType<OUString>::get(), PROPERTY_NONE, 0},
                    {OUString(UNO_NAME_CURRENT_PRESENTATION), FIELD_PROP_PAR3, cppu::UnoType<OUString>::get(),  PROPERTY_NONE, 0},
                    {OUString(UNO_NAME_IS_FIXED),   FIELD_PROP_BOOL1,   cppu::UnoType<bool>::get()  , PROPERTY_NONE,0},
                    {OUString(UNO_NAME_NUMBER_FORMAT), FIELD_PROP_FORMAT,   cppu::UnoType<sal_Int32>::get(), PROPERTY_NONE,    0},
                    {OUString(UNO_NAME_IS_FIXED_LANGUAGE), FIELD_PROP_BOOL4, cppu::UnoType<bool>::get(), PROPERTY_NONE,    0},
                    COMMON_FLDTYP_PROPERTIES
                    { OUString(), 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aDocInfoCustomPropMap;
            }
            break;
            case PROPERTY_MAP_FLDTYP_DOCINFO_REVISION          :
            {
                static SfxItemPropertyMapEntry const aDocInfoRevisionPropMap [] =
                {
                    {OUString(UNO_NAME_CURRENT_PRESENTATION), FIELD_PROP_PAR3, cppu::UnoType<OUString>::get(),  PROPERTY_NONE, 0},
                    {OUString(UNO_NAME_REVISION),   FIELD_PROP_USHORT1, cppu::UnoType<sal_Int16>::get(), PROPERTY_NONE,    0},
                    {OUString(UNO_NAME_IS_FIXED),   FIELD_PROP_BOOL1,   cppu::UnoType<bool>::get()  , PROPERTY_NONE,0},
                    COMMON_FLDTYP_PROPERTIES
                    { OUString(), 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aDocInfoRevisionPropMap;
            }
            break;
            case PROPERTY_MAP_FLDTYP_COMBINED_CHARACTERS:
            {
                static SfxItemPropertyMapEntry const aCombinedCharactersPropMap[] =
                {
                    {OUString(UNO_NAME_CONTENT), FIELD_PROP_PAR1, cppu::UnoType<OUString>::get(),  PROPERTY_NONE, 0},
                    COMMON_FLDTYP_PROPERTIES
                    { OUString(), 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aCombinedCharactersPropMap;
            }
            break;
            case PROPERTY_MAP_FLDTYP_TABLE_FORMULA:
            {
                static SfxItemPropertyMapEntry const aTableFormulaPropMap[] =
                {
                    {OUString(UNO_NAME_CURRENT_PRESENTATION), FIELD_PROP_PAR1, cppu::UnoType<OUString>::get(),  PROPERTY_NONE, 0},
                    {OUString(UNO_NAME_CONTENT), FIELD_PROP_PAR2, cppu::UnoType<OUString>::get(),  PROPERTY_NONE, 0},
                    {OUString(UNO_NAME_IS_SHOW_FORMULA), FIELD_PROP_BOOL1,  cppu::UnoType<bool>::get(), PROPERTY_NONE, 0},
                    {OUString(UNO_NAME_NUMBER_FORMAT), FIELD_PROP_FORMAT,   cppu::UnoType<sal_Int32>::get(), PROPERTY_NONE, 0},
                    COMMON_FLDTYP_PROPERTIES
                    { OUString(), 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aTableFormulaPropMap;
            }
            break;
            case PROPERTY_MAP_FLDTYP_DUMMY_0:
            {
                static SfxItemPropertyMapEntry const aEmptyPropMap           [] =
                {
                    COMMON_FLDTYP_PROPERTIES
                    { OUString(), 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aEmptyPropMap;
            }
            break;
            case PROPERTY_MAP_FLDMSTR_USER :
            {
                static SfxItemPropertyMapEntry const aUserFieldTypePropMap[] =
                {
                    {OUString(UNO_NAME_DEPENDENT_TEXT_FIELDS),  FIELD_PROP_PROP_SEQ,    cppu::UnoType< cppu::UnoSequenceType<css::text::XDependentTextField> >::get(), PropertyAttribute::READONLY, 0},
                    {OUString(UNO_NAME_IS_EXPRESSION),      FIELD_PROP_BOOL1,  cppu::UnoType<bool>::get(), PROPERTY_NONE,   0},
                    {OUString(UNO_NAME_NAME),               FIELD_PROP_PAR1,  cppu::UnoType<OUString>::get(), PropertyAttribute::MAYBEVOID, 0},
                    {OUString(UNO_NAME_VALUE),          FIELD_PROP_DOUBLE,  cppu::UnoType<double>::get(), PROPERTY_NONE,   0},
                    {OUString(UNO_NAME_CONTENT),            FIELD_PROP_PAR2,    cppu::UnoType<OUString>::get(), PROPERTY_NONE, 0},
                    {OUString(UNO_NAME_INSTANCE_NAME),      FIELD_PROP_PAR3,    cppu::UnoType<OUString>::get(), PropertyAttribute::READONLY, 0},
                    { OUString(), 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aUserFieldTypePropMap;
            }
            break;
            case PROPERTY_MAP_FLDMSTR_DDE       :
            {
                static SfxItemPropertyMapEntry const aDDEFieldTypePropMap[] =
                {
                    {OUString(UNO_NAME_DDE_COMMAND_ELEMENT), FIELD_PROP_PAR2,  cppu::UnoType<OUString>::get(), PROPERTY_NONE, 0},
                    {OUString(UNO_NAME_DDE_COMMAND_FILE), FIELD_PROP_PAR4,  cppu::UnoType<OUString>::get(), PROPERTY_NONE, 0},
                    {OUString(UNO_NAME_DDE_COMMAND_TYPE), FIELD_PROP_SUBTYPE,   cppu::UnoType<OUString>::get(), PROPERTY_NONE, 0},
                    {OUString(UNO_NAME_DEPENDENT_TEXT_FIELDS),  FIELD_PROP_PROP_SEQ,    cppu::UnoType< cppu::UnoSequenceType<css::text::XDependentTextField> >::get(), PropertyAttribute::READONLY, 0},
                    {OUString(UNO_NAME_IS_AUTOMATIC_UPDATE), FIELD_PROP_BOOL1,  cppu::UnoType<bool>::get(), PROPERTY_NONE,    0},
                    {OUString(UNO_NAME_NAME),               FIELD_PROP_PAR1,  cppu::UnoType<OUString>::get(), PROPERTY_NONE, 0},
                    {OUString(UNO_NAME_INSTANCE_NAME),      FIELD_PROP_PAR3,    cppu::UnoType<OUString>::get(), PropertyAttribute::READONLY, 0},
                    {OUString(UNO_NAME_CONTENT),            FIELD_PROP_PAR5,    cppu::UnoType<OUString>::get(),   PROPERTY_NONE, 0},
                    { OUString(), 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aDDEFieldTypePropMap;
            }
            break;
            case PROPERTY_MAP_FLDMSTR_SET_EXP     :
            {
                static SfxItemPropertyMapEntry const aSetExpFieldTypePropMap[] =
                {
                    {OUString(UNO_NAME_CHAPTER_NUMBERING_LEVEL),FIELD_PROP_SHORT1,  cppu::UnoType<sal_Int8>::get(), PROPERTY_NONE, 0},
                    {OUString(UNO_NAME_DEPENDENT_TEXT_FIELDS),  FIELD_PROP_PROP_SEQ,    cppu::UnoType< cppu::UnoSequenceType<css::text::XDependentTextField> >::get(), PropertyAttribute::READONLY, 0},
                    {OUString(UNO_NAME_NAME),               FIELD_PROP_PAR1,  cppu::UnoType<OUString>::get(), PROPERTY_NONE, 0},
                    {OUString(UNO_NAME_NUMBERING_SEPARATOR), FIELD_PROP_PAR2,   cppu::UnoType<OUString>::get(), PROPERTY_NONE, 0},
                    {OUString(UNO_NAME_SUB_TYPE),           FIELD_PROP_SUBTYPE, cppu::UnoType<sal_Int16>::get(), PROPERTY_NONE,    0},
                    {OUString(UNO_NAME_INSTANCE_NAME),      FIELD_PROP_PAR3,    cppu::UnoType<OUString>::get(), PropertyAttribute::READONLY, 0},
                    { OUString(), 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aSetExpFieldTypePropMap;
            }
            break;
            case PROPERTY_MAP_FLDMSTR_DATABASE    :
            {
                static SfxItemPropertyMapEntry const aDBFieldTypePropMap         [] =
                {
                    // Note: DATA_BASE_NAME and DATA_BASE_URL
                    // are mapped to the same nMId, because internally  we only use
                    // them as DataSource and it does not matter which one it is.

                    {OUString(UNO_NAME_DATA_BASE_NAME) ,  FIELD_PROP_PAR1, cppu::UnoType<OUString>::get(),   PROPERTY_NONE, 0},
                    {OUString(UNO_NAME_NAME),              FIELD_PROP_PAR3,  cppu::UnoType<OUString>::get(), PropertyAttribute::MAYBEVOID, 0},
                    {OUString(UNO_NAME_DATA_TABLE_NAME), FIELD_PROP_PAR2, cppu::UnoType<OUString>::get(),   PROPERTY_NONE, 0},
                    {OUString(UNO_NAME_DATA_COLUMN_NAME), FIELD_PROP_PAR3, cppu::UnoType<OUString>::get(),   PROPERTY_NONE, 0},
                    {OUString(UNO_NAME_INSTANCE_NAME),     FIELD_PROP_PAR4, cppu::UnoType<OUString>::get(), PropertyAttribute::READONLY, 0},
                    {OUString(UNO_NAME_DATA_BASE_URL) ,   FIELD_PROP_PAR1, cppu::UnoType<OUString>::get(),   PROPERTY_NONE, 0},
                    {OUString(UNO_NAME_DATA_COMMAND_TYPE), FIELD_PROP_SHORT1, cppu::UnoType<sal_Int32>::get(),   PROPERTY_NONE, 0},
                    {OUString(UNO_NAME_DEPENDENT_TEXT_FIELDS),  FIELD_PROP_PROP_SEQ,    cppu::UnoType< cppu::UnoSequenceType<css::text::XDependentTextField> >::get(), PropertyAttribute::READONLY, 0},
                    { OUString(), 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aDBFieldTypePropMap;
            }
            break;
            case PROPERTY_MAP_FLDMSTR_DUMMY0      :
            {
                static SfxItemPropertyMapEntry const aStandardFieldMasterMap[] =
                {
                    {OUString(UNO_NAME_DEPENDENT_TEXT_FIELDS),  0,  cppu::UnoType< cppu::UnoSequenceType<css::text::XDependentTextField> >::get(), PropertyAttribute::READONLY, 0},
                    {OUString(UNO_NAME_NAME),               0,  cppu::UnoType<OUString>::get(), PROPERTY_NONE, 0},
                    {OUString(UNO_NAME_INSTANCE_NAME),      0,  cppu::UnoType<OUString>::get(), PropertyAttribute::READONLY, 0},
                    { OUString(), 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aStandardFieldMasterMap;
            }
            break;
            case PROPERTY_MAP_FLDTYP_BIBLIOGRAPHY:
            {
                static SfxItemPropertyMapEntry const aBibliographyFieldMap[] =
                {
                    {OUString(UNO_NAME_FIELDS)    , FIELD_PROP_PROP_SEQ, cppu::UnoType< cppu::UnoSequenceType<css::beans::PropertyValue> >::get(),PROPERTY_NONE, 0},
                    COMMON_FLDTYP_PROPERTIES
                    { OUString(), 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aBibliographyFieldMap;
            }
            break;
            case PROPERTY_MAP_FLDMSTR_BIBLIOGRAPHY:
            {
                static SfxItemPropertyMapEntry const aBibliographyFieldMasterMap[] =
                {
                    {OUString(UNO_NAME_BRACKET_BEFORE) , FIELD_PROP_PAR1, cppu::UnoType<OUString>::get(),               PROPERTY_NONE, 0},
                    {OUString(UNO_NAME_BRACKET_AFTER) , FIELD_PROP_PAR2, cppu::UnoType<OUString>::get(),               PROPERTY_NONE, 0},
                    {OUString(UNO_NAME_IS_NUMBER_ENTRIES) , FIELD_PROP_BOOL1, cppu::UnoType<bool>::get(),                    PROPERTY_NONE, 0},
                    {OUString(UNO_NAME_IS_SORT_BY_POSITION) , FIELD_PROP_BOOL2, cppu::UnoType<bool>::get(),                    PROPERTY_NONE, 0},
                    {OUString(UNO_NAME_LOCALE),            FIELD_PROP_LOCALE,  cppu::UnoType<css::lang::Locale>::get()  , PROPERTY_NONE,     0},
                    {OUString(UNO_NAME_SORT_ALGORITHM),    FIELD_PROP_PAR3,  cppu::UnoType<OUString>::get(), PROPERTY_NONE,     0},
                    {OUString(UNO_NAME_SORT_KEYS) , FIELD_PROP_PROP_SEQ, cppu::UnoType< cppu::UnoSequenceType<css::beans::PropertyValues> >::get(),   PROPERTY_NONE, 0},
                    {OUString(UNO_NAME_INSTANCE_NAME),      FIELD_PROP_PAR4,    cppu::UnoType<OUString>::get(), PropertyAttribute::READONLY, 0},
                    { OUString(), 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aBibliographyFieldMasterMap;
            }
            break;
            case PROPERTY_MAP_TEXT :
            {
                static SfxItemPropertyMapEntry const aTextMap[] =
                {
                    _REDLINE_NODE_PROPERTIES
                    { OUString(), 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aTextMap;
            }
            break;
            case PROPERTY_MAP_MAILMERGE :
            {
                static SfxItemPropertyMapEntry const aMailMergeMap[] =
                {
                    { OUString(UNO_NAME_SELECTION),             WID_SELECTION,              cppu::UnoType< cppu::UnoSequenceType<css::uno::Any> >::get(),      PROPERTY_NONE, 0},
                    { OUString(UNO_NAME_RESULT_SET),            WID_RESULT_SET,             cppu::UnoType<css::sdbc::XResultSet>::get(), PROPERTY_NONE, 0},
                    { OUString(UNO_NAME_CONNECTION),            WID_CONNECTION,             cppu::UnoType<css::sdbc::XConnection>::get(), PROPERTY_NONE, 0},
                    { OUString(UNO_NAME_MODEL),                 WID_MODEL,                  cppu::UnoType<css::frame::XModel>::get(),    PropertyAttribute::READONLY, 0},
                    { OUString(UNO_NAME_DATA_SOURCE_NAME),      WID_DATA_SOURCE_NAME,       cppu::UnoType<OUString>::get(),    PROPERTY_NONE, 0},
                    { OUString(UNO_NAME_DAD_COMMAND),           WID_DATA_COMMAND,           cppu::UnoType<OUString>::get(),    PROPERTY_NONE, 0},
                    { OUString(UNO_NAME_FILTER),                WID_FILTER,                 cppu::UnoType<OUString>::get(),    PROPERTY_NONE, 0},
                    { OUString(UNO_NAME_DOCUMENT_URL),          WID_DOCUMENT_URL,           cppu::UnoType<OUString>::get(),    PROPERTY_NONE, 0},
                    { OUString(UNO_NAME_OUTPUT_URL),            WID_OUTPUT_URL,             cppu::UnoType<OUString>::get(),    PROPERTY_NONE, 0},
                    { OUString(UNO_NAME_DAD_COMMAND_TYPE),      WID_DATA_COMMAND_TYPE,      cppu::UnoType<sal_Int32>::get(),       PROPERTY_NONE, 0},
                    { OUString(UNO_NAME_OUTPUT_TYPE),           WID_OUTPUT_TYPE,            cppu::UnoType<sal_Int16>::get(),       PROPERTY_NONE, 0},
                    { OUString(UNO_NAME_ESCAPE_PROCESSING),     WID_ESCAPE_PROCESSING,      cppu::UnoType<bool>::get(),     PROPERTY_NONE, 0},
                    { OUString(UNO_NAME_SINGLE_PRINT_JOBS),     WID_SINGLE_PRINT_JOBS,      cppu::UnoType<bool>::get(),     PROPERTY_NONE, 0},
                    { OUString(UNO_NAME_FILE_NAME_FROM_COLUMN), WID_FILE_NAME_FROM_COLUMN,  cppu::UnoType<bool>::get(),     PROPERTY_NONE, 0},
                    { OUString(UNO_NAME_FILE_NAME_PREFIX),      WID_FILE_NAME_PREFIX,       cppu::UnoType<OUString>::get(),    PROPERTY_NONE, 0},
                    { OUString(UNO_NAME_SUBJECT),               WID_MAIL_SUBJECT,           cppu::UnoType<OUString>::get(),        PROPERTY_NONE, 0},
                    { OUString(UNO_NAME_ADDRESS_FROM_COLUMN),   WID_ADDRESS_FROM_COLUMN,    cppu::UnoType<OUString>::get(),        PROPERTY_NONE, 0},
                    { OUString(UNO_NAME_SEND_AS_HTML),          WID_SEND_AS_HTML,           cppu::UnoType<bool>::get(),         PROPERTY_NONE, 0},
                    { OUString(UNO_NAME_SEND_AS_ATTACHMENT),    WID_SEND_AS_ATTACHMENT,     cppu::UnoType<bool>::get(),         PROPERTY_NONE, 0},
                    { OUString(UNO_NAME_MAIL_BODY),             WID_MAIL_BODY,              cppu::UnoType<OUString>::get(),        PROPERTY_NONE, 0},
                    { OUString(UNO_NAME_ATTACHMENT_NAME),       WID_ATTACHMENT_NAME,        cppu::UnoType<OUString>::get(),        PROPERTY_NONE, 0},
                    { OUString(UNO_NAME_ATTACHMENT_FILTER),     WID_ATTACHMENT_FILTER,      cppu::UnoType<OUString>::get(),        PROPERTY_NONE, 0},
                    { OUString(UNO_NAME_PRINT_OPTIONS),         WID_PRINT_OPTIONS,          cppu::UnoType< cppu::UnoSequenceType<css::beans::PropertyValue> >::get(),  PROPERTY_NONE, 0},
                    { OUString(UNO_NAME_SAVE_AS_SINGLE_FILE),   WID_SAVE_AS_SINGLE_FILE,    cppu::UnoType<bool>::get(),         PROPERTY_NONE, 0},
                    { OUString(UNO_NAME_SAVE_FILTER),           WID_SAVE_FILTER,            cppu::UnoType<OUString>::get(),        PROPERTY_NONE, 0},
                    { OUString(UNO_NAME_SAVE_FILTER_OPTIONS),   WID_SAVE_FILTER_OPTIONS,    cppu::UnoType<OUString>::get(),        PROPERTY_NONE, 0},
                    { OUString(UNO_NAME_SAVE_FILTER_DATA),      WID_SAVE_FILTER_DATA,       cppu::UnoType< cppu::UnoSequenceType<css::beans::PropertyValue> >::get(),        PROPERTY_NONE, 0},
                    { OUString(UNO_NAME_COPIES_TO),             WID_COPIES_TO,              cppu::UnoType< cppu::UnoSequenceType<OUString> >::get(),       PROPERTY_NONE, 0},
                    { OUString(UNO_NAME_BLIND_COPIES_TO),       WID_BLIND_COPIES_TO,        cppu::UnoType< cppu::UnoSequenceType<OUString> >::get(),       PROPERTY_NONE, 0},
                    { OUString(UNO_NAME_IN_SERVER_PASSWORD),     WID_IN_SERVER_PASSWORD,     cppu::UnoType<OUString>::get(),    PROPERTY_NONE, 0},
                    { OUString(UNO_NAME_OUT_SERVER_PASSWORD),    WID_OUT_SERVER_PASSWORD,    cppu::UnoType<OUString>::get(),    PROPERTY_NONE, 0},
                    { OUString(), 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aMailMergeMap;
            }
            break;
            case PROPERTY_MAP_TEXT_VIEW :
            {
                static SfxItemPropertyMapEntry pTextViewMap[] =
                {
                    {OUString(UNO_NAME_PAGE_COUNT),             WID_PAGE_COUNT,             cppu::UnoType<sal_Int32>::get(),   PropertyAttribute::READONLY, 0},
                    {OUString(UNO_NAME_LINE_COUNT),             WID_LINE_COUNT,             cppu::UnoType<sal_Int32>::get(),   PropertyAttribute::READONLY, 0},
                    {OUString(UNO_NAME_IS_CONSTANT_SPELLCHECK), WID_IS_CONSTANT_SPELLCHECK, cppu::UnoType<bool>::get(), PROPERTY_NONE, 0},
                    {OUString(UNO_NAME_IS_HIDE_SPELL_MARKS),    WID_IS_HIDE_SPELL_MARKS,    cppu::UnoType<bool>::get(), PROPERTY_NONE, 0},  // deprecated #i91949
                    { OUString(), 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = pTextViewMap;
            }
            break;
            case PROPERTY_MAP_CHART2_DATA_SEQUENCE :
            {
                static SfxItemPropertyMapEntry const aChart2DataSequenceMap[] =
                {
                    {OUString(UNO_NAME_ROLE), 0, cppu::UnoType<OUString>::get(),   PROPERTY_NONE, 0 },
                    { OUString(), 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aChart2DataSequenceMap;
            }
            break;
            case PROPERTY_MAP_METAFIELD:
            {
                static SfxItemPropertyMapEntry const aMetaFieldMap[] =
                {
                    { OUString(UNO_NAME_NUMBER_FORMAT), 0,
                        cppu::UnoType<sal_Int32>::get(), PROPERTY_NONE, 0 },
                    { OUString(UNO_NAME_IS_FIXED_LANGUAGE), 0,
                        cppu::UnoType<bool>::get(), PROPERTY_NONE, 0 },
                    { OUString(), 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aMetaFieldMap;
            }
            break;

            default:
                OSL_FAIL( "unexpected property map ID" );
        }
    }
    return m_aMapEntriesArr[nPropertyId];
}

const SfxItemPropertySet*  SwUnoPropertyMapProvider::GetPropertySet( sal_uInt16 nPropertyId)
{
    if( !m_aPropertySetArr[nPropertyId] )
    {
        const SfxItemPropertyMapEntry* pEntries = GetPropertyMapEntries(nPropertyId);
        switch( nPropertyId )
        {
            case PROPERTY_MAP_TEXT_CURSOR:
            {
                static SfxItemPropertySet aPROPERTY_MAP_TEXT_CURSOR(pEntries);
                m_aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_TEXT_CURSOR;
            }
            break;
            case PROPERTY_MAP_CHAR_STYLE:
            {
                static SfxItemPropertySet aPROPERTY_MAP_CHAR_STYLE(pEntries);
                m_aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_CHAR_STYLE;
            }
            break;
            case PROPERTY_MAP_PARA_STYLE:
            {
                static SfxItemPropertySet aPROPERTY_MAP_PARA_STYLE(pEntries);
                m_aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_PARA_STYLE;
            }
            break;
            case PROPERTY_MAP_FRAME_STYLE:
            {
                static SfxItemPropertySet aPROPERTY_MAP_FRAME_STYLE(pEntries);
                m_aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_FRAME_STYLE;
            }
            break;
            case PROPERTY_MAP_PAGE_STYLE:
            {
                static SfxItemPropertySet aPROPERTY_MAP_PAGE_STYLE(pEntries);
                m_aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_PAGE_STYLE;
            }
            break;
            case PROPERTY_MAP_NUM_STYLE:
            {
                static SfxItemPropertySet aPROPERTY_MAP_NUM_STYLE(pEntries);
                m_aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_NUM_STYLE;
            }
            break;
            case PROPERTY_MAP_SECTION:
            {
                static SfxItemPropertySet aPROPERTY_MAP_SECTION(pEntries);
                m_aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_SECTION;
            }
            break;
            case PROPERTY_MAP_TEXT_TABLE:
            {
                static SfxItemPropertySet aPROPERTY_MAP_TEXT_TABLE(pEntries);
                m_aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_TEXT_TABLE;
            }
            break;
            case PROPERTY_MAP_TABLE_CELL:
            {
                static SfxItemPropertySet aPROPERTY_MAP_TABLE_CELL(pEntries);
                m_aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_TABLE_CELL;
            }
            break;
            case PROPERTY_MAP_TABLE_RANGE:
            {
                static SfxItemPropertySet aPROPERTY_MAP_TABLE_RANGE(pEntries);
                m_aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_TABLE_RANGE;
            }
            break;
            case PROPERTY_MAP_TEXT_SEARCH:
            {
                static SfxItemPropertySet aPROPERTY_MAP_TEXT_SEARCH(pEntries);
                m_aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_TEXT_SEARCH;
            }
            break;
            case PROPERTY_MAP_TEXT_FRAME:
            {
                static SfxItemPropertySet aPROPERTY_MAP_TEXT_FRAME(pEntries);
                m_aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_TEXT_FRAME;
            }
            break;
            case PROPERTY_MAP_TEXT_GRAPHIC:
            {
                static SfxItemPropertySet aPROPERTY_MAP_TEXT_GRAPHIC(pEntries);
                m_aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_TEXT_GRAPHIC;
            }
            break;
            case PROPERTY_MAP_TEXT_SHAPE:
            {
                static SfxItemPropertySet aPROPERTY_MAP_TEXT_SHAPE(pEntries);
                m_aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_TEXT_SHAPE;
            }
            break;
            case PROPERTY_MAP_INDEX_USER:
            {
                static SfxItemPropertySet aPROPERTY_MAP_INDEX_USER(pEntries);
                m_aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_INDEX_USER;
            }
            break;
            case PROPERTY_MAP_INDEX_CNTNT:
            {
                static SfxItemPropertySet aPROPERTY_MAP_INDEX_CNTNT(pEntries);
                m_aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_INDEX_CNTNT;
            }
            break;
            case PROPERTY_MAP_INDEX_IDX:
            {
                static SfxItemPropertySet aPROPERTY_MAP_INDEX_IDX(pEntries);
                m_aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_INDEX_IDX;
            }
            break;
            case PROPERTY_MAP_USER_MARK:
            {
                static SfxItemPropertySet aPROPERTY_MAP_USER_MARK(pEntries);
                m_aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_USER_MARK;
            }
            break;
            case PROPERTY_MAP_CNTIDX_MARK:
            {
                static SfxItemPropertySet aPROPERTY_MAP_CNTIDX_MARK(pEntries);
                m_aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_CNTIDX_MARK;
            }
            break;
            case PROPERTY_MAP_INDEX_MARK:
            {
                static SfxItemPropertySet aPROPERTY_MAP_INDEX_MARK(pEntries);
                m_aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_INDEX_MARK;
            }
            break;
            case PROPERTY_MAP_TEXT_TABLE_ROW:
            {
                static SfxItemPropertySet aPROPERTY_MAP_TEXT_TABLE_ROW(pEntries);
                m_aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_TEXT_TABLE_ROW;
            }
            break;
            case PROPERTY_MAP_TEXT_SHAPE_DESCRIPTOR:
            {
                static SfxItemPropertySet aPROPERTY_MAP_TEXT_SHAPE_DESCRIPTOR(pEntries);
                m_aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_TEXT_SHAPE_DESCRIPTOR;
            }
            break;
            case PROPERTY_MAP_TEXT_TABLE_CURSOR:
            {
                static SfxItemPropertySet aPROPERTY_MAP_TEXT_TABLE_CURSOR(pEntries);
                m_aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_TEXT_TABLE_CURSOR;
            }
            break;
            case PROPERTY_MAP_BOOKMARK:
            {
                static SfxItemPropertySet aPROPERTY_MAP_BOOKMARK(pEntries);
                m_aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_BOOKMARK;
            }
            break;
            case PROPERTY_MAP_PARAGRAPH_EXTENSIONS:
            {
                static SfxItemPropertySet aPROPERTY_MAP_PARAGRAPH_EXTENSIONS(pEntries);
                m_aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_PARAGRAPH_EXTENSIONS;
            }
            break;
            case PROPERTY_MAP_INDEX_ILLUSTRATIONS:
            {
                static SfxItemPropertySet aPROPERTY_MAP_INDEX_ILLUSTRATIONS(pEntries);
                m_aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_INDEX_ILLUSTRATIONS;
            }
            break;
            case PROPERTY_MAP_INDEX_OBJECTS:
            {
                static SfxItemPropertySet aPROPERTY_MAP_INDEX_OBJECTS(pEntries);
                m_aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_INDEX_OBJECTS;
            }
            break;
            case PROPERTY_MAP_INDEX_TABLES:
            {
                static SfxItemPropertySet aPROPERTY_MAP_INDEX_TABLES(pEntries);
                m_aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_INDEX_TABLES;
            }
            break;
            case PROPERTY_MAP_BIBLIOGRAPHY           :
            {
                static SfxItemPropertySet aPROPERTY_MAP_BIBLIOGRAPHY(pEntries);
                m_aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_BIBLIOGRAPHY;
            }
            break;
            case PROPERTY_MAP_TEXT_DOCUMENT:
            {
                static SfxItemPropertySet aPROPERTY_MAP_TEXT_DOCUMENT(pEntries);
                m_aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_TEXT_DOCUMENT;
            }
            break;
            case PROPERTY_MAP_LINK_TARGET            :
            {
                static SfxItemPropertySet aPROPERTY_MAP_LINK_TARGET(pEntries);
                m_aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_LINK_TARGET;
            }
            break;
            case PROPERTY_MAP_AUTO_TEXT_GROUP        :
            {
                static SfxItemPropertySet aPROPERTY_MAP_AUTO_TEXT_GROUP(pEntries);
                m_aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_AUTO_TEXT_GROUP;
            }
            break;
            case PROPERTY_MAP_TEXTPORTION_EXTENSIONS :
            {
                static SfxItemPropertySet aPROPERTY_MAP_TEXTPORTION_EXTENSIONS(pEntries);
                m_aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_TEXTPORTION_EXTENSIONS;
            }
            break;
            case PROPERTY_MAP_FOOTNOTE               :
            {
                static SfxItemPropertySet aPROPERTY_MAP_FOOTNOTE(pEntries);
                m_aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_FOOTNOTE;
            }
            break;
            case PROPERTY_MAP_TEXT_COLUMS            :
            {
                static SfxItemPropertySet aPROPERTY_MAP_TEXT_COLUMS(pEntries);
                m_aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_TEXT_COLUMS;
            }
            break;
            case PROPERTY_MAP_PARAGRAPH              :
            {
                static SfxItemPropertySet aPROPERTY_MAP_PARAGRAPH(pEntries);
                m_aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_PARAGRAPH;
            }
            break;
            case PROPERTY_MAP_EMBEDDED_OBJECT        :
            {
                static SfxItemPropertySet aPROPERTY_MAP_EMBEDDED_OBJECT(pEntries);
                m_aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_EMBEDDED_OBJECT;
            }
            break;
            case PROPERTY_MAP_REDLINE                :
            {
                static SfxItemPropertySet aPROPERTY_MAP_REDLINE(pEntries);
                m_aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_REDLINE;
            }
            break;
            case PROPERTY_MAP_TEXT_DEFAULT           :
            {
                static SfxItemPropertySet aPROPERTY_MAP_TEXT_DEFAULT(pEntries);
                m_aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_TEXT_DEFAULT;
            }
            break;
            case PROPERTY_MAP_FLDTYP_DATETIME:
            {
                static SfxItemPropertySet aPROPERTY_MAP_FLDTYP_DATETIME(pEntries);
                m_aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_FLDTYP_DATETIME;
            }
            break;
            case PROPERTY_MAP_FLDTYP_USER:
            {
                static SfxItemPropertySet aPROPERTY_MAP_FLDTYP_USER(pEntries);
                m_aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_FLDTYP_USER;
            }
            break;
            case PROPERTY_MAP_FLDTYP_SET_EXP:
            {
                static SfxItemPropertySet aPROPERTY_MAP_FLDTYP_SET_EXP(pEntries);
                m_aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_FLDTYP_SET_EXP;
            }
            break;
            case PROPERTY_MAP_FLDTYP_GET_EXP:
            {
                static SfxItemPropertySet aPROPERTY_MAP_FLDTYP_GET_EXP(pEntries);
                m_aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_FLDTYP_GET_EXP;
            }
            break;
            case PROPERTY_MAP_FLDTYP_FILE_NAME:
            {
                static SfxItemPropertySet aPROPERTY_MAP_FLDTYP_FILE_NAME(pEntries);
                m_aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_FLDTYP_FILE_NAME;
            }
            break;
            case PROPERTY_MAP_FLDTYP_PAGE_NUM:
            {
                static SfxItemPropertySet aPROPERTY_MAP_FLDTYP_PAGE_NUM(pEntries);
                m_aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_FLDTYP_PAGE_NUM;
            }
            break;
            case PROPERTY_MAP_FLDTYP_AUTHOR:
            {
                static SfxItemPropertySet aPROPERTY_MAP_FLDTYP_AUTHOR(pEntries);
                m_aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_FLDTYP_AUTHOR;
            }
            break;
            case PROPERTY_MAP_FLDTYP_CHAPTER:
            {
                static SfxItemPropertySet aPROPERTY_MAP_FLDTYP_CHAPTER(pEntries);
                m_aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_FLDTYP_CHAPTER;
            }
            break;
            case PROPERTY_MAP_FLDTYP_GET_REFERENCE:
            {
                static SfxItemPropertySet aPROPERTY_MAP_FLDTYP_GET_REFERENCE(pEntries);
                m_aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_FLDTYP_GET_REFERENCE;
            }
            break;
            case PROPERTY_MAP_FLDTYP_CONDITIONED_TEXT:
            {
                static SfxItemPropertySet aPROPERTY_MAP_FLDTYP_CONDITIONED_TEXT(pEntries);
                m_aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_FLDTYP_CONDITIONED_TEXT;
            }
            break;
            case PROPERTY_MAP_FLDTYP_HIDDEN_TEXT:
            {
                static SfxItemPropertySet aPROPERTY_MAP_FLDTYP_HIDDEN_TEXT(pEntries);
                m_aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_FLDTYP_HIDDEN_TEXT;
            }
            break;
            case PROPERTY_MAP_FLDTYP_ANNOTATION :
            {
                static SfxItemPropertySet aPROPERTY_MAP_FLDTYP_ANNOTATION(pEntries);
                m_aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_FLDTYP_ANNOTATION;
            }
            break;
            case PROPERTY_MAP_FLDTYP_INPUT:
            {
                static SfxItemPropertySet aPROPERTY_MAP_FLDTYP_INPUT(pEntries);
                m_aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_FLDTYP_INPUT;
            }
            break;
            case PROPERTY_MAP_FLDTYP_MACRO:
            {
                static SfxItemPropertySet aPROPERTY_MAP_FLDTYP_MACRO(pEntries);
                m_aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_FLDTYP_MACRO;
            }
            break;
            case PROPERTY_MAP_FLDTYP_DDE:
            {
                static SfxItemPropertySet aPROPERTY_MAP_FLDTYP_DDE(pEntries);
                m_aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_FLDTYP_DDE;
            }
            break;
            case PROPERTY_MAP_FLDTYP_HIDDEN_PARA:
            {
                static SfxItemPropertySet aPROPERTY_MAP_FLDTYP_HIDDEN_PARA(pEntries);
                m_aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_FLDTYP_HIDDEN_PARA;
            }
            break;
            case PROPERTY_MAP_FLDTYP_DOC_INFO :
            {
                static SfxItemPropertySet aPROPERTY_MAP_FLDTYP_DOC_INFO(pEntries);
                m_aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_FLDTYP_DOC_INFO;
            }
            break;
            case PROPERTY_MAP_FLDTYP_TEMPLATE_NAME:
            {
                static SfxItemPropertySet aPROPERTY_MAP_FLDTYP_TEMPLATE_NAME(pEntries);
                m_aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_FLDTYP_TEMPLATE_NAME;
            }
            break;
            case PROPERTY_MAP_FLDTYP_USER_EXT :
            {
                static SfxItemPropertySet aPROPERTY_MAP_FLDTYP_USER_EXT(pEntries);
                m_aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_FLDTYP_USER_EXT;
            }
            break;
            case PROPERTY_MAP_FLDTYP_REF_PAGE_SET:
            {
                static SfxItemPropertySet aPROPERTY_MAP_FLDTYP_REF_PAGE_SET(pEntries);
                m_aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_FLDTYP_REF_PAGE_SET;
            }
            break;
            case PROPERTY_MAP_FLDTYP_REF_PAGE_GET:
            {
                static SfxItemPropertySet aPROPERTY_MAP_FLDTYP_REF_PAGE_GET(pEntries);
                m_aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_FLDTYP_REF_PAGE_GET;
            }
            break;
            case PROPERTY_MAP_FLDTYP_JUMP_EDIT:
            {
                static SfxItemPropertySet aPROPERTY_MAP_FLDTYP_JUMP_EDIT(pEntries);
                m_aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_FLDTYP_JUMP_EDIT;
            }
            break;
            case PROPERTY_MAP_FLDTYP_SCRIPT:
            {
                static SfxItemPropertySet aPROPERTY_MAP_FLDTYP_SCRIPT(pEntries);
                m_aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_FLDTYP_SCRIPT;
            }
            break;
            case PROPERTY_MAP_FLDTYP_DATABASE_NEXT_SET:
            {
                static SfxItemPropertySet aPROPERTY_MAP_FLDTYP_DATABASE_NEXT_SET(pEntries);
                m_aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_FLDTYP_DATABASE_NEXT_SET;
            }
            break;
            case PROPERTY_MAP_FLDTYP_DATABASE_NUM_SET:
            {
                static SfxItemPropertySet aPROPERTY_MAP_FLDTYP_DATABASE_NUM_SET(pEntries);
                m_aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_FLDTYP_DATABASE_NUM_SET;
            }
            break;
            case PROPERTY_MAP_FLDTYP_DATABASE_SET_NUM:
            {
                static SfxItemPropertySet aPROPERTY_MAP_FLDTYP_DATABASE_SET_NUM(pEntries);
                m_aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_FLDTYP_DATABASE_SET_NUM;
            }
            break;
            case PROPERTY_MAP_FLDTYP_DATABASE:
            {
                static SfxItemPropertySet aPROPERTY_MAP_FLDTYP_DATABASE(pEntries);
                m_aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_FLDTYP_DATABASE;
            }
            break;
            case PROPERTY_MAP_FLDTYP_DATABASE_NAME:
            {
                static SfxItemPropertySet aPROPERTY_MAP_FLDTYP_DATABASE_NAME(pEntries);
                m_aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_FLDTYP_DATABASE_NAME;
            }
            break;
            case PROPERTY_MAP_FLDTYP_DOCSTAT:
            {
                static SfxItemPropertySet aPROPERTY_MAP_FLDTYP_DOCSTAT(pEntries);
                m_aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_FLDTYP_DOCSTAT;
            }
            break;
            case PROPERTY_MAP_FLDTYP_DOCINFO_AUTHOR:
            {
                static SfxItemPropertySet aPROPERTY_MAP_FLDTYP_DOCINFO_AUTHOR(pEntries);
                m_aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_FLDTYP_DOCINFO_AUTHOR;
            }
            break;
            case PROPERTY_MAP_FLDTYP_DOCINFO_DATE_TIME:
            {
                static SfxItemPropertySet aPROPERTY_MAP_FLDTYP_DOCINFO_DATE_TIME(pEntries);
                m_aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_FLDTYP_DOCINFO_DATE_TIME;
            }
            break;
            case PROPERTY_MAP_FLDTYP_DOCINFO_CHANGE_DATE_TIME:
            {
                static SfxItemPropertySet aPROPERTY_MAP_FLDTYP_DOCINFO_CHANGE_DATE_TIME(pEntries);
                m_aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_FLDTYP_DOCINFO_CHANGE_DATE_TIME;
            }
            break;
            case PROPERTY_MAP_FLDTYP_DOCINFO_CREATE_DATE_TIME:
            {
                static SfxItemPropertySet aPROPERTY_MAP_FLDTYP_DOCINFO_CREATE_DATE_TIME(pEntries);
                m_aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_FLDTYP_DOCINFO_CREATE_DATE_TIME;
            }
            break;
            case PROPERTY_MAP_FLDTYP_DOCINFO_EDIT_TIME:
            {
                static SfxItemPropertySet aPROPERTY_MAP_FLDTYP_DOCINFO_EDIT_TIME(pEntries);
                m_aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_FLDTYP_DOCINFO_EDIT_TIME;
            }
            break;
            case PROPERTY_MAP_FLDTYP_DOCINFO_MISC :
            {
                static SfxItemPropertySet aPROPERTY_MAP_FLDTYP_DOCINFO_MISC(pEntries);
                m_aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_FLDTYP_DOCINFO_MISC;
            }
            break;
            case PROPERTY_MAP_FLDTYP_DOCINFO_REVISION:
            {
                static SfxItemPropertySet aPROPERTY_MAP_FLDTYP_DOCINFO_REVISION(pEntries);
                m_aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_FLDTYP_DOCINFO_REVISION;
            }
            break;
            case PROPERTY_MAP_FLDTYP_COMBINED_CHARACTERS:
            {
                static SfxItemPropertySet aPROPERTY_MAP_FLDTYP_COMBINED_CHARACTERS(pEntries);
                m_aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_FLDTYP_COMBINED_CHARACTERS;
            }
            break;
            case PROPERTY_MAP_FLDTYP_DUMMY_0:
            {
                static SfxItemPropertySet aPROPERTY_MAP_FLDTYP_DUMMY_0(pEntries);
                m_aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_FLDTYP_DUMMY_0;
            }
            break;
            case PROPERTY_MAP_FLDTYP_TABLE_FORMULA:
            {
                static SfxItemPropertySet aPROPERTY_MAP_FLDTYP_TABLE_FORMULA(pEntries);
                m_aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_FLDTYP_TABLE_FORMULA;
            }
            break;
            case PROPERTY_MAP_FLDMSTR_USER:
            {
                static SfxItemPropertySet aPROPERTY_MAP_FLDMSTR_USER(pEntries);
                m_aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_FLDMSTR_USER;
            }
            break;
            case PROPERTY_MAP_FLDMSTR_DDE:
            {
                static SfxItemPropertySet aPROPERTY_MAP_FLDMSTR_DDE(pEntries);
                m_aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_FLDMSTR_DDE;
            }
            break;
            case PROPERTY_MAP_FLDMSTR_SET_EXP:
            {
                static SfxItemPropertySet aPROPERTY_MAP_FLDMSTR_SET_EXP(pEntries);
                m_aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_FLDMSTR_SET_EXP;
            }
            break;
            case PROPERTY_MAP_FLDMSTR_DATABASE:
            {
                static SfxItemPropertySet aPROPERTY_MAP_FLDMSTR_DATABASE(pEntries);
                m_aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_FLDMSTR_DATABASE;
            }
            break;
            case PROPERTY_MAP_FLDMSTR_DUMMY0:
            {
                static SfxItemPropertySet aPROPERTY_MAP_FLDMSTR_DUMMY0(pEntries);
                m_aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_FLDMSTR_DUMMY0;
            }
            break;
            case PROPERTY_MAP_FLDTYP_BIBLIOGRAPHY:
            {
                static SfxItemPropertySet aPROPERTY_MAP_FLDTYP_BIBLIOGRAPHY(pEntries);
                m_aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_FLDTYP_BIBLIOGRAPHY;
            }
            break;
            case PROPERTY_MAP_FLDMSTR_BIBLIOGRAPHY:
            {
                static SfxItemPropertySet aPROPERTY_MAP_FLDMSTR_BIBLIOGRAPHY(pEntries);
                m_aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_FLDMSTR_BIBLIOGRAPHY;
            }
            break;
            case PROPERTY_MAP_TEXT:
            {
                static SfxItemPropertySet aPROPERTY_MAP_TEXT(pEntries);
                m_aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_TEXT;
            }
            break;
            case PROPERTY_MAP_REDLINE_PORTION:
            {
                static SfxItemPropertySet aPROPERTY_MAP_REDLINE_PORTION(pEntries);
                m_aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_REDLINE_PORTION;
            }
            break;
            case PROPERTY_MAP_MAILMERGE:
            {
                static SfxItemPropertySet aPROPERTY_MAP_MAILMERGE(pEntries);
                m_aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_MAILMERGE;
            }
            break;
            case PROPERTY_MAP_FLDTYP_DROPDOWN:
            {
                static SfxItemPropertySet aPROPERTY_MAP_FLDTYP_DROPDOWN(pEntries);
                m_aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_FLDTYP_DROPDOWN;
            }
            break;
            case PROPERTY_MAP_CHART2_DATA_SEQUENCE:
            {
                static SfxItemPropertySet aPROPERTY_MAP_CHART2_DATA_SEQUENCE(pEntries);
                m_aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_CHART2_DATA_SEQUENCE;
            }
            break;
            case PROPERTY_MAP_TEXT_VIEW:
            {
                static SfxItemPropertySet aPROPERTY_MAP_TEXT_VIEW(pEntries);
                m_aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_TEXT_VIEW;
            }
            break;
            case PROPERTY_MAP_CONDITIONAL_PARA_STYLE:
            {
                static SfxItemPropertySet aPROPERTY_MAP_CONDITIONAL_PARA_STYLE(pEntries);
                m_aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_CONDITIONAL_PARA_STYLE;
            }
            break;
            case PROPERTY_MAP_CHAR_AUTO_STYLE:
            {
                static SfxItemPropertySet aPROPERTY_MAP_CHAR_AUTO_STYLE(pEntries);
                m_aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_CHAR_AUTO_STYLE;
            }
            break;
            case PROPERTY_MAP_RUBY_AUTO_STYLE:
            {
                static SfxItemPropertySet aPROPERTY_MAP_RUBY_AUTO_STYLE(pEntries);
                m_aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_RUBY_AUTO_STYLE;
            }
            break;
            case PROPERTY_MAP_PARA_AUTO_STYLE:
            {
                static SfxItemPropertySet aPROPERTY_MAP_PARA_AUTO_STYLE(pEntries);
                m_aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_PARA_AUTO_STYLE;
            }
            break;
            case PROPERTY_MAP_FLDTYP_DOCINFO_CUSTOM:
            {
                static SfxItemPropertySet aPROPERTY_MAP_FLDTYP_DOCINFO_CUSTOM(pEntries);
                m_aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_FLDTYP_DOCINFO_CUSTOM;
            }
            break;
            case PROPERTY_MAP_METAFIELD:
            {
                static SfxItemPropertySet aPROPERTY_MAP_METAFIELD(pEntries);
                m_aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_METAFIELD;
            }
            break;
        }
    }
    return m_aPropertySetArr[nPropertyId];
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
