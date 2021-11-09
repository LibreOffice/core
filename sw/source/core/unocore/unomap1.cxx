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

#include <svx/unomid.hxx>
#include <com/sun/star/awt/FontSlant.hpp>
#include <com/sun/star/awt/Gradient.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/awt/XBitmap.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/container/XIndexReplace.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
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
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/style/BreakType.hpp>
#include <com/sun/star/style/DropCapFormat.hpp>
#include <com/sun/star/style/GraphicLocation.hpp>
#include <com/sun/star/style/LineSpacing.hpp>
#include <com/sun/star/style/PageStyleLayout.hpp>
#include <com/sun/star/style/TabStop.hpp>
#include <com/sun/star/table/BorderLine.hpp>
#include <com/sun/star/table/ShadowFormat.hpp>
#include <com/sun/star/table/TableBorder.hpp>
#include <com/sun/star/table/TableBorder2.hpp>
#include <com/sun/star/table/TableBorderDistances.hpp>
#include <com/sun/star/table/XCell.hpp>
#include <com/sun/star/text/GraphicCrop.hpp>
#include <com/sun/star/text/SectionFileLink.hpp>
#include <com/sun/star/text/TableColumnSeparator.hpp>
#include <com/sun/star/text/TextContentAnchorType.hpp>
#include <com/sun/star/text/WrapTextMode.hpp>
#include <com/sun/star/text/XDocumentIndex.hpp>
#include <com/sun/star/text/XDocumentIndexMark.hpp>
#include <com/sun/star/text/XFootnote.hpp>
#include <com/sun/star/text/XTextColumns.hpp>
#include <com/sun/star/text/XTextContent.hpp>
#include <com/sun/star/text/XTextField.hpp>
#include <com/sun/star/text/XTextFrame.hpp>
#include <com/sun/star/text/XTextSection.hpp>
#include <com/sun/star/text/XTextTable.hpp>
#include <com/sun/star/util/DateTime.hpp>
#include <unomap.hxx>
#include <unoprnms.hxx>
#include <unomid.h>
#include <cmdid.h>
#include <editeng/memberids.h>
#include <editeng/unoprnms.hxx>
#include <svl/itemprop.hxx>
#include <svx/xdef.hxx>
#include "unomapproperties.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;

SwUnoPropertyMapProvider aSwMapProvider;

SwUnoPropertyMapProvider::SwUnoPropertyMapProvider()
{
    for( sal_uInt16 i = 0; i < PROPERTY_MAP_END; i++ )
    {
        m_aMapEntriesArr[i] = nullptr;
        m_aPropertySetArr[i] = nullptr;
    }
}

SwUnoPropertyMapProvider::~SwUnoPropertyMapProvider()
{
}

const SfxItemPropertyMapEntry*  SwUnoPropertyMapProvider::GetTextCursorPropertyMap()
{
    static SfxItemPropertyMapEntry const aCharAndParaMap_Impl[] =
    {
        COMPLETE_TEXT_CURSOR_MAP
        { u"", 0, css::uno::Type(), 0, 0 }
    };

    return aCharAndParaMap_Impl;
}

const SfxItemPropertyMapEntry*  SwUnoPropertyMapProvider::GetAccessibilityTextAttrPropertyMap()
{
    static SfxItemPropertyMapEntry const aAccessibilityTextAttrMap_Impl[] =
    {
        COMMON_ACCESSIBILITY_TEXT_ATTRIBUTE
        { u"", 0, css::uno::Type(), 0, 0 }
    };

    return aAccessibilityTextAttrMap_Impl;
}

const SfxItemPropertyMapEntry*  SwUnoPropertyMapProvider::GetParagraphPropertyMap()
{
    static SfxItemPropertyMapEntry const aParagraphMap_Impl[] =
    {
        COMMON_CRSR_PARA_PROPERTIES_2
        TABSTOPS_MAP_ENTRY
        COMMON_TEXT_CONTENT_PROPERTIES
        { UNO_NAME_CHAR_STYLE_NAME, RES_TXTATR_CHARFMT,     cppu::UnoType<OUString>::get(),         PropertyAttribute::MAYBEVOID,     0},
        { UNO_NAME_CHAR_STYLE_NAMES, FN_UNO_CHARFMT_SEQUENCE,  cppu::UnoType< cppu::UnoSequenceType<OUString> >::get(),     PropertyAttribute::MAYBEVOID,     0},
        // added FillProperties for SW, same as FILL_PROPERTIES in svx
        // but need own defines in Writer due to later association of strings
        // and uno types (see loop at end of this method and definition of SW_PROP_NMID)
        // This entry is for adding that properties to style import/export
        // Added for paragraph backgrounds, this is for paragraph itself
        FILL_PROPERTIES_SW
        { u"", 0, css::uno::Type(), 0, 0 }
    };

    return aParagraphMap_Impl;
}

const SfxItemPropertyMapEntry*  SwUnoPropertyMapProvider::GetAutoParaStylePropertyMap()
{
    static SfxItemPropertyMapEntry const aAutoParaStyleMap [] =
    {
        { UNO_NAME_PARA_STYLE_NAME, RES_FRMATR_STYLE_NAME,        cppu::UnoType<OUString>::get(),                PropertyAttribute::MAYBEVOID,     0},
        { UNO_NAME_PAGE_STYLE_NAME, FN_UNO_PAGE_STYLE,        cppu::UnoType<OUString>::get(),         PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY,   0},
        { UNO_NAME_NUMBERING_IS_NUMBER, FN_UNO_IS_NUMBER,     cppu::UnoType<bool>::get()  ,       PropertyAttribute::MAYBEVOID,     0},
        { UNO_NAME_NUMBERING_LEVEL, FN_UNO_NUM_LEVEL,     cppu::UnoType<sal_Int16>::get(),           PropertyAttribute::MAYBEVOID, 0},
        { UNO_NAME_NUMBERING_START_VALUE, FN_UNO_NUM_START_VALUE, cppu::UnoType<sal_Int16>::get(),           PropertyAttribute::MAYBEVOID, CONVERT_TWIPS},
        { UNO_NAME_DOCUMENT_INDEX, FN_UNO_DOCUMENT_INDEX, cppu::UnoType<css::text::XDocumentIndex>::get(), PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY ,0 },
        { UNO_NAME_TEXT_TABLE, FN_UNO_TEXT_TABLE,     cppu::UnoType<css::text::XTextTable>::get(),     PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY ,0 },
        { UNO_NAME_CELL, FN_UNO_CELL,         cppu::UnoType<css::table::XCell>::get(),         PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY ,0 },
        { UNO_NAME_TEXT_FRAME, FN_UNO_TEXT_FRAME,     cppu::UnoType<css::text::XTextFrame>::get(),        PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY ,0 },
        { UNO_NAME_TEXT_SECTION, FN_UNO_TEXT_SECTION, cppu::UnoType<css::text::XTextSection>::get(),  PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY ,0 },
        { UNO_NAME_PARA_CHAPTER_NUMBERING_LEVEL, FN_UNO_PARA_CHAPTER_NUMBERING_LEVEL,cppu::UnoType<sal_Int8>::get(), PropertyAttribute::MAYBEVOID, 0},
        { UNO_NAME_PARA_CONDITIONAL_STYLE_NAME, RES_FRMATR_CONDITIONAL_STYLE_NAME,        cppu::UnoType<OUString>::get(),         PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY,   0},
        { UNO_NAME_PARA_IS_NUMBERING_RESTART, FN_NUMBER_NEWSTART,     cppu::UnoType<bool>::get(),     PropertyAttribute::MAYBEVOID, 0 },
        // TODO add RES_PARATR_LIST_AUTOFMT?
        { UNO_NAME_OUTLINE_LEVEL, RES_PARATR_OUTLINELEVEL,        cppu::UnoType<sal_Int16>::get(),                PropertyAttribute::MAYBEVOID,     0},
        { UNO_NAME_OUTLINE_CONTENT_VISIBLE, RES_PARATR_GRABBAG, cppu::UnoType<bool>::get(), PropertyAttribute::MAYBEVOID, 0 },
        COMMON_CRSR_PARA_PROPERTIES_WITHOUT_FN
        TABSTOPS_MAP_ENTRY
        COMMON_TEXT_CONTENT_PROPERTIES
        { UNO_NAME_PARA_AUTO_STYLE_NAME, RES_AUTO_STYLE,     cppu::UnoType<OUString>::get(),         PropertyAttribute::MAYBEVOID,     0},
        // added FillProperties for SW, same as FILL_PROPERTIES in svx
        // but need own defines in Writer due to later association of strings
        // and uno types (see loop at end of this method and definition of SW_PROP_NMID)
        // This entry is for adding that properties to style import/export
        // Added for paragraph backgrounds, this is for Paragraph AutoStyles
        FILL_PROPERTIES_SW
        { u"", 0, css::uno::Type(), 0, 0 }
    };

    return aAutoParaStyleMap;
}

const SfxItemPropertyMapEntry*  SwUnoPropertyMapProvider::GetCharStylePropertyMap()
{
    static SfxItemPropertyMapEntry const aCharStyleMap   [] =
    {
        { UNO_NAME_CHAR_AUTO_KERNING, RES_CHRATR_AUTOKERN  ,  cppu::UnoType<bool>::get()  ,       PROPERTY_NONE,     0},
        { UNO_NAME_CHAR_BACK_TRANSPARENT, RES_CHRATR_BACKGROUND,  cppu::UnoType<bool>::get(),         PROPERTY_NONE ,MID_GRAPHIC_TRANSPARENT       },
        { UNO_NAME_CHAR_BACK_COLOR, RES_CHRATR_BACKGROUND,    cppu::UnoType<sal_Int32>::get(),           PROPERTY_NONE ,MID_BACK_COLOR        },
        { UNO_NAME_CHAR_HIGHLIGHT, RES_CHRATR_HIGHLIGHT, cppu::UnoType<sal_Int32>::get(), PROPERTY_NONE, MID_BACK_COLOR },
        { UNO_NAME_CHAR_CASE_MAP, RES_CHRATR_CASEMAP,     cppu::UnoType<sal_Int16>::get(),           PROPERTY_NONE, 0},
        { UNO_NAME_CHAR_COLOR, RES_CHRATR_COLOR,      cppu::UnoType<sal_Int32>::get(),           PROPERTY_NONE, 0},
        { UNO_NAME_CHAR_TRANSPARENCE, RES_CHRATR_COLOR, cppu::UnoType<sal_Int16>::get(), PROPERTY_NONE, MID_COLOR_ALPHA},
        { UNO_NAME_CHAR_STRIKEOUT, RES_CHRATR_CROSSEDOUT,  cppu::UnoType<sal_Int16>::get(),                  PropertyAttribute::MAYBEVOID, MID_CROSS_OUT},
        { UNO_NAME_CHAR_CROSSED_OUT, RES_CHRATR_CROSSEDOUT,  cppu::UnoType<bool>::get()  ,        PROPERTY_NONE, 0},
        { UNO_NAME_CHAR_ESCAPEMENT, RES_CHRATR_ESCAPEMENT,  cppu::UnoType<sal_Int16>::get(),             PROPERTY_NONE, MID_ESC          },
        { UNO_NAME_CHAR_ESCAPEMENT_HEIGHT, RES_CHRATR_ESCAPEMENT,     cppu::UnoType<sal_Int8>::get()  ,          PROPERTY_NONE, MID_ESC_HEIGHT},
        { UNO_NAME_CHAR_FLASH, RES_CHRATR_BLINK   ,   cppu::UnoType<bool>::get()  ,       PROPERTY_NONE,     0},
        { UNO_NAME_CHAR_HIDDEN, RES_CHRATR_HIDDEN, cppu::UnoType<bool>::get()  ,       PROPERTY_NONE,     0},
        STANDARD_FONT_PROPERTIES
        CJK_FONT_PROPERTIES
        CTL_FONT_PROPERTIES
        { UNO_NAME_CHAR_UNDERLINE, RES_CHRATR_UNDERLINE ,  cppu::UnoType<sal_Int16>::get(),      PROPERTY_NONE, MID_TL_STYLE},
        { UNO_NAME_CHAR_UNDERLINE_COLOR, RES_CHRATR_UNDERLINE ,  cppu::UnoType<sal_Int32>::get(),            PROPERTY_NONE, MID_TL_COLOR},
        { UNO_NAME_CHAR_UNDERLINE_HAS_COLOR, RES_CHRATR_UNDERLINE ,  cppu::UnoType<bool>::get(),              PROPERTY_NONE, MID_TL_HASCOLOR},
        { UNO_NAME_CHAR_OVERLINE, RES_CHRATR_OVERLINE ,  cppu::UnoType<sal_Int16>::get(),    PROPERTY_NONE, MID_TL_STYLE},
        { UNO_NAME_CHAR_OVERLINE_COLOR, RES_CHRATR_OVERLINE ,  cppu::UnoType<sal_Int32>::get(),              PROPERTY_NONE, MID_TL_COLOR},
        { UNO_NAME_CHAR_OVERLINE_HAS_COLOR, RES_CHRATR_OVERLINE ,  cppu::UnoType<bool>::get(),            PROPERTY_NONE, MID_TL_HASCOLOR},
        { UNO_NAME_CHAR_KERNING, RES_CHRATR_KERNING    ,  cppu::UnoType<sal_Int16>::get()  ,         PROPERTY_NONE,  CONVERT_TWIPS},
        { UNO_NAME_CHAR_NO_HYPHENATION, RES_CHRATR_NOHYPHEN   ,   cppu::UnoType<bool>::get()  ,       PROPERTY_NONE,     0},
        { UNO_NAME_CHAR_SHADOWED, RES_CHRATR_SHADOWED  ,  cppu::UnoType<bool>::get()  ,       PROPERTY_NONE, 0},
        { UNO_NAME_CHAR_CONTOURED, RES_CHRATR_CONTOUR,    cppu::UnoType<bool>::get()  ,       PROPERTY_NONE, 0},
        { UNO_NAME_CHAR_WORD_MODE, RES_CHRATR_WORDLINEMODE,cppu::UnoType<bool>::get()  ,    PROPERTY_NONE,     0},
        { UNO_NAME_USER_DEFINED_ATTRIBUTES, RES_UNKNOWNATR_CONTAINER, cppu::UnoType<css::container::XNameContainer>::get(), PropertyAttribute::MAYBEVOID, 0 },
        { UNO_NAME_IS_PHYSICAL, FN_UNO_IS_PHYSICAL,     cppu::UnoType<bool>::get(), PropertyAttribute::READONLY, 0},
        { UNO_NAME_HIDDEN, FN_UNO_HIDDEN,     cppu::UnoType<bool>::get(), PROPERTY_NONE, 0},
        { UNO_NAME_STYLE_INTEROP_GRAB_BAG, FN_UNO_STYLE_INTEROP_GRAB_BAG, cppu::UnoType< cppu::UnoSequenceType<css::beans::PropertyValue> >::get(), PROPERTY_NONE, 0},
        { UNO_NAME_DISPLAY_NAME, FN_UNO_DISPLAY_NAME, cppu::UnoType<OUString>::get(), PropertyAttribute::READONLY, 0},
        { UNO_NAME_CHAR_COMBINE_IS_ON, RES_CHRATR_TWO_LINES,          cppu::UnoType<bool>::get(),     PROPERTY_NONE, MID_TWOLINES},
        { UNO_NAME_CHAR_COMBINE_PREFIX, RES_CHRATR_TWO_LINES,             cppu::UnoType<OUString>::get(),    PROPERTY_NONE, MID_START_BRACKET},
        { UNO_NAME_CHAR_COMBINE_SUFFIX, RES_CHRATR_TWO_LINES,             cppu::UnoType<OUString>::get(),    PROPERTY_NONE, MID_END_BRACKET},
        { UNO_NAME_CHAR_EMPHASIS, RES_CHRATR_EMPHASIS_MARK,           cppu::UnoType<sal_Int16>::get(),   PROPERTY_NONE, MID_EMPHASIS},
        PROP_DIFF_FONTHEIGHT
        { UNO_NAME_CHAR_ROTATION, RES_CHRATR_ROTATE,      cppu::UnoType<sal_Int16>::get(),   PROPERTY_NONE,    MID_ROTATE      },
        { UNO_NAME_CHAR_ROTATION_IS_FIT_TO_LINE, RES_CHRATR_ROTATE,       cppu::UnoType<bool>::get(),     PROPERTY_NONE,        MID_FITTOLINE  },
        { UNO_NAME_CHAR_SCALE_WIDTH, RES_CHRATR_SCALEW,       cppu::UnoType<sal_Int16>::get(),   PROPERTY_NONE,         0 },
        { UNO_NAME_CHAR_RELIEF, RES_CHRATR_RELIEF,      cppu::UnoType<sal_Int16>::get(),    PROPERTY_NONE,      MID_RELIEF },
        { UNO_NAME_CHAR_LEFT_BORDER, RES_CHRATR_BOX, cppu::UnoType<css::table::BorderLine>::get(), PROPERTY_NONE, LEFT_BORDER |CONVERT_TWIPS },
        { UNO_NAME_CHAR_RIGHT_BORDER, RES_CHRATR_BOX, cppu::UnoType<css::table::BorderLine>::get(), PROPERTY_NONE, RIGHT_BORDER |CONVERT_TWIPS },
        { UNO_NAME_CHAR_TOP_BORDER, RES_CHRATR_BOX, cppu::UnoType<css::table::BorderLine>::get(), PROPERTY_NONE, TOP_BORDER |CONVERT_TWIPS },
        { UNO_NAME_CHAR_BOTTOM_BORDER, RES_CHRATR_BOX, cppu::UnoType<css::table::BorderLine>::get(), PROPERTY_NONE, BOTTOM_BORDER |CONVERT_TWIPS },
        { UNO_NAME_CHAR_BORDER_DISTANCE, RES_CHRATR_BOX, cppu::UnoType<sal_Int32>::get(), PROPERTY_NONE, BORDER_DISTANCE |CONVERT_TWIPS },
        { UNO_NAME_CHAR_LEFT_BORDER_DISTANCE, RES_CHRATR_BOX, cppu::UnoType<sal_Int32>::get(), PROPERTY_NONE, LEFT_BORDER_DISTANCE |CONVERT_TWIPS },
        { UNO_NAME_CHAR_RIGHT_BORDER_DISTANCE, RES_CHRATR_BOX, cppu::UnoType<sal_Int32>::get(), PROPERTY_NONE, RIGHT_BORDER_DISTANCE |CONVERT_TWIPS },
        { UNO_NAME_CHAR_TOP_BORDER_DISTANCE, RES_CHRATR_BOX, cppu::UnoType<sal_Int32>::get(), PROPERTY_NONE, TOP_BORDER_DISTANCE |CONVERT_TWIPS },
        { UNO_NAME_CHAR_BOTTOM_BORDER_DISTANCE, RES_CHRATR_BOX, cppu::UnoType<sal_Int32>::get(), PROPERTY_NONE, BOTTOM_BORDER_DISTANCE|CONVERT_TWIPS },
        { UNO_NAME_CHAR_SHADOW_FORMAT, RES_CHRATR_SHADOW, cppu::UnoType<css::table::ShadowFormat>::get(), PROPERTY_NONE, CONVERT_TWIPS},
        { UNO_NAME_LINK_STYLE, FN_UNO_LINK_STYLE, cppu::UnoType<OUString>::get(), PROPERTY_NONE, 0},
        { u"", 0, css::uno::Type(), 0, 0 }
    };

    return aCharStyleMap;
}

const SfxItemPropertyMapEntry*  SwUnoPropertyMapProvider::GetAutoCharStylePropertyMap()
{
    // same as PROPERTY_MAP_TEXTPORTION_EXTENSIONS
    static SfxItemPropertyMapEntry const aAutoCharStyleMap   [] =
    {
        {  UNO_NAME_CHAR_AUTO_KERNING, RES_CHRATR_AUTOKERN  ,  cppu::UnoType<bool>::get()  ,       PROPERTY_NONE,     0},
        {  UNO_NAME_CHAR_BACK_TRANSPARENT, RES_CHRATR_BACKGROUND,  cppu::UnoType<bool>::get(),         PROPERTY_NONE ,MID_GRAPHIC_TRANSPARENT       },
        {  UNO_NAME_CHAR_BACK_COLOR, RES_CHRATR_BACKGROUND,    cppu::UnoType<sal_Int32>::get(),           PROPERTY_NONE ,MID_BACK_COLOR        },
        {  UNO_NAME_CHAR_HIGHLIGHT, RES_CHRATR_HIGHLIGHT, cppu::UnoType<sal_Int32>::get(), PROPERTY_NONE, MID_BACK_COLOR },
        {  UNO_NAME_CHAR_CASE_MAP, RES_CHRATR_CASEMAP,     cppu::UnoType<sal_Int16>::get(),           PROPERTY_NONE, 0},
        { UNO_NAME_CHAR_COLOR, RES_CHRATR_COLOR,      cppu::UnoType<sal_Int32>::get(),           PROPERTY_NONE, 0},
        {  UNO_NAME_CHAR_TRANSPARENCE, RES_CHRATR_COLOR, cppu::UnoType<sal_Int16>::get(), PROPERTY_NONE, MID_COLOR_ALPHA},
        {  UNO_NAME_CHAR_STRIKEOUT, RES_CHRATR_CROSSEDOUT,  cppu::UnoType<sal_Int16>::get(),                  PropertyAttribute::MAYBEVOID, MID_CROSS_OUT},
        {  UNO_NAME_CHAR_CROSSED_OUT, RES_CHRATR_CROSSEDOUT,  cppu::UnoType<bool>::get()  ,        PROPERTY_NONE, 0},
        {  UNO_NAME_CHAR_ESCAPEMENT, RES_CHRATR_ESCAPEMENT,  cppu::UnoType<sal_Int16>::get(),             PROPERTY_NONE, MID_ESC          },
        {  UNO_NAME_CHAR_ESCAPEMENT_HEIGHT, RES_CHRATR_ESCAPEMENT,     cppu::UnoType<sal_Int8>::get()  ,          PROPERTY_NONE, MID_ESC_HEIGHT},
        {  UNO_NAME_CHAR_FLASH, RES_CHRATR_BLINK   ,   cppu::UnoType<bool>::get()  ,       PROPERTY_NONE,     0},
        {  UNO_NAME_CHAR_HIDDEN, RES_CHRATR_HIDDEN, cppu::UnoType<bool>::get()  ,       PROPERTY_NONE,     0},
        STANDARD_FONT_PROPERTIES
        CJK_FONT_PROPERTIES
        CTL_FONT_PROPERTIES
        { UNO_NAME_CHAR_UNDERLINE, RES_CHRATR_UNDERLINE ,  cppu::UnoType<sal_Int16>::get(),      PROPERTY_NONE, MID_TL_STYLE},
        { UNO_NAME_CHAR_UNDERLINE_COLOR, RES_CHRATR_UNDERLINE ,  cppu::UnoType<sal_Int32>::get(),            PROPERTY_NONE, MID_TL_COLOR},
        { UNO_NAME_CHAR_UNDERLINE_HAS_COLOR, RES_CHRATR_UNDERLINE ,  cppu::UnoType<bool>::get(),              PROPERTY_NONE, MID_TL_HASCOLOR},
        { UNO_NAME_CHAR_OVERLINE, RES_CHRATR_OVERLINE ,  cppu::UnoType<sal_Int16>::get(),      PROPERTY_NONE, MID_TL_STYLE},
        { UNO_NAME_CHAR_OVERLINE_COLOR, RES_CHRATR_OVERLINE ,  cppu::UnoType<sal_Int32>::get(),            PROPERTY_NONE, MID_TL_COLOR},
        { UNO_NAME_CHAR_OVERLINE_HAS_COLOR, RES_CHRATR_OVERLINE ,  cppu::UnoType<bool>::get(),              PROPERTY_NONE, MID_TL_HASCOLOR},
        { UNO_NAME_CHAR_KERNING, RES_CHRATR_KERNING    ,  cppu::UnoType<sal_Int16>::get()  ,         PROPERTY_NONE,  CONVERT_TWIPS},
        { UNO_NAME_CHAR_NO_HYPHENATION, RES_CHRATR_NOHYPHEN   ,   cppu::UnoType<bool>::get()  ,       PROPERTY_NONE,     0},
        { UNO_NAME_CHAR_SHADOWED, RES_CHRATR_SHADOWED  ,  cppu::UnoType<bool>::get()  ,       PROPERTY_NONE, 0},
        { UNO_NAME_CHAR_CONTOURED, RES_CHRATR_CONTOUR,    cppu::UnoType<bool>::get()  ,       PROPERTY_NONE, 0},
        { UNO_NAME_CHAR_WORD_MODE, RES_CHRATR_WORDLINEMODE,cppu::UnoType<bool>::get()  ,    PROPERTY_NONE,     0},
        { UNO_NAME_USER_DEFINED_ATTRIBUTES, RES_UNKNOWNATR_CONTAINER, cppu::UnoType<css::container::XNameContainer>::get(), PropertyAttribute::MAYBEVOID, 0 },
        { UNO_NAME_TEXT_USER_DEFINED_ATTRIBUTES, RES_TXTATR_UNKNOWN_CONTAINER, cppu::UnoType<css::container::XNameContainer>::get(), PropertyAttribute::MAYBEVOID, 0 },
        { UNO_NAME_IS_PHYSICAL, FN_UNO_IS_PHYSICAL,     cppu::UnoType<bool>::get(), PropertyAttribute::READONLY, 0},
        { UNO_NAME_DISPLAY_NAME, FN_UNO_DISPLAY_NAME, cppu::UnoType<OUString>::get(), PropertyAttribute::READONLY, 0},
        { UNO_NAME_CHAR_COMBINE_IS_ON, RES_CHRATR_TWO_LINES,          cppu::UnoType<bool>::get(),     PROPERTY_NONE, MID_TWOLINES},
        { UNO_NAME_CHAR_COMBINE_PREFIX, RES_CHRATR_TWO_LINES,             cppu::UnoType<OUString>::get(),    PROPERTY_NONE, MID_START_BRACKET},
        { UNO_NAME_CHAR_COMBINE_SUFFIX, RES_CHRATR_TWO_LINES,             cppu::UnoType<OUString>::get(),    PROPERTY_NONE, MID_END_BRACKET},
        { UNO_NAME_CHAR_EMPHASIS, RES_CHRATR_EMPHASIS_MARK,           cppu::UnoType<sal_Int16>::get(),   PROPERTY_NONE, MID_EMPHASIS},
        { UNO_NAME_CHAR_ROTATION, RES_CHRATR_ROTATE,      cppu::UnoType<sal_Int16>::get(),   PROPERTY_NONE,    MID_ROTATE      },
        { UNO_NAME_CHAR_ROTATION_IS_FIT_TO_LINE, RES_CHRATR_ROTATE,       cppu::UnoType<bool>::get(),     PROPERTY_NONE,        MID_FITTOLINE  },
        { UNO_NAME_CHAR_SCALE_WIDTH, RES_CHRATR_SCALEW,       cppu::UnoType<sal_Int16>::get(),   PROPERTY_NONE,         0 },
        { UNO_NAME_CHAR_RELIEF, RES_CHRATR_RELIEF,      cppu::UnoType<sal_Int16>::get(),    PROPERTY_NONE,      MID_RELIEF },
        { UNO_NAME_CHAR_AUTO_STYLE_NAME, RES_TXTATR_AUTOFMT,     cppu::UnoType<OUString>::get(),         PropertyAttribute::MAYBEVOID,     0},
        { UNO_NAME_CHAR_SHADING_VALUE, RES_CHRATR_BACKGROUND,      cppu::UnoType<sal_Int32>::get(),    PROPERTY_NONE,      MID_SHADING_VALUE },
        { UNO_NAME_CHAR_LEFT_BORDER, RES_CHRATR_BOX, cppu::UnoType<css::table::BorderLine>::get(), PROPERTY_NONE, LEFT_BORDER |CONVERT_TWIPS },
        { UNO_NAME_CHAR_RIGHT_BORDER, RES_CHRATR_BOX, cppu::UnoType<css::table::BorderLine>::get(), PROPERTY_NONE, RIGHT_BORDER |CONVERT_TWIPS },
        { UNO_NAME_CHAR_TOP_BORDER, RES_CHRATR_BOX, cppu::UnoType<css::table::BorderLine>::get(), PROPERTY_NONE, TOP_BORDER |CONVERT_TWIPS },
        { UNO_NAME_CHAR_BOTTOM_BORDER, RES_CHRATR_BOX, cppu::UnoType<css::table::BorderLine>::get(), PROPERTY_NONE, BOTTOM_BORDER |CONVERT_TWIPS },
        { UNO_NAME_CHAR_BORDER_DISTANCE, RES_CHRATR_BOX, cppu::UnoType<sal_Int32>::get(), PROPERTY_NONE, BORDER_DISTANCE |CONVERT_TWIPS },
        { UNO_NAME_CHAR_LEFT_BORDER_DISTANCE, RES_CHRATR_BOX, cppu::UnoType<sal_Int32>::get(), PROPERTY_NONE, LEFT_BORDER_DISTANCE |CONVERT_TWIPS },
        { UNO_NAME_CHAR_RIGHT_BORDER_DISTANCE, RES_CHRATR_BOX, cppu::UnoType<sal_Int32>::get(), PROPERTY_NONE, RIGHT_BORDER_DISTANCE |CONVERT_TWIPS },
        { UNO_NAME_CHAR_TOP_BORDER_DISTANCE, RES_CHRATR_BOX, cppu::UnoType<sal_Int32>::get(), PROPERTY_NONE, TOP_BORDER_DISTANCE |CONVERT_TWIPS },
        { UNO_NAME_CHAR_BOTTOM_BORDER_DISTANCE, RES_CHRATR_BOX, cppu::UnoType<sal_Int32>::get(), PROPERTY_NONE, BOTTOM_BORDER_DISTANCE|CONVERT_TWIPS },
        { UNO_NAME_CHAR_SHADOW_FORMAT, RES_CHRATR_SHADOW, cppu::UnoType<css::table::ShadowFormat>::get(), PROPERTY_NONE, CONVERT_TWIPS},
        { u"", 0, css::uno::Type(), 0, 0 }
    };

    return aAutoCharStyleMap;
}

const SfxItemPropertyMapEntry*  SwUnoPropertyMapProvider::GetParaStylePropertyMap()
{
    static SfxItemPropertyMapEntry const aParaStyleMap [] =
    {
        COMMON_PARA_STYLE_PROPERTIES
        // added FillProperties for SW, same as FILL_PROPERTIES in svx
        // but need own defines in Writer due to later association of strings
        // and uno types (see loop at end of this method and definition of SW_PROP_NMID)
        // This entry is for adding that properties to style import/export
        // Added for paragraph backgrounds, this is for Paragraph Styles
        FILL_PROPERTIES_SW
        { u"", 0, css::uno::Type(), 0, 0 }
    };

    return aParaStyleMap;
}

const SfxItemPropertyMapEntry*  SwUnoPropertyMapProvider::GetConditionalParaStylePropertyMap()
{
    static SfxItemPropertyMapEntry const aParaStyleMap [] =
    {
        COMMON_PARA_STYLE_PROPERTIES
        { UNO_NAME_PARA_STYLE_CONDITIONS, FN_UNO_PARA_STYLE_CONDITIONS, cppu::UnoType< cppu::UnoSequenceType<css::beans::NamedValue> >::get(), PropertyAttribute::MAYBEVOID, 0},

        // added FillProperties for SW, same as FILL_PROPERTIES in svx
        // but need own defines in Writer due to later association of strings
        // and uno types (see loop at end of this method and definition of SW_PROP_NMID)
        // This entry is for adding that properties to style import/export
        // Added for paragraph backgrounds, this is for Paragraph Styles
        FILL_PROPERTIES_SW

        { u"", 0, css::uno::Type(), 0, 0 }
    };

    return aParaStyleMap;
}

const SfxItemPropertyMapEntry*  SwUnoPropertyMapProvider::GetFrameStylePropertyMap()
{
    static SfxItemPropertyMapEntry const aFrameStyleMap   [] =
    {
        { UNO_NAME_ANCHOR_PAGE_NO, RES_ANCHOR,            cppu::UnoType<sal_Int16>::get(),           PROPERTY_NONE, MID_ANCHOR_PAGENUM       },
        { UNO_NAME_ANCHOR_TYPE, RES_ANCHOR,           cppu::UnoType<css::text::TextContentAnchorType>::get(),            PROPERTY_NONE, MID_ANCHOR_ANCHORTYPE},
        { UNO_NAME_BACK_COLOR, RES_BACKGROUND,            cppu::UnoType<sal_Int32>::get(),           PROPERTY_NONE ,MID_BACK_COLOR        },
        { UNO_NAME_BACK_COLOR_R_G_B, RES_BACKGROUND,      cppu::UnoType<sal_Int32>::get(), PROPERTY_NONE ,MID_BACK_COLOR_R_G_B},
        { UNO_NAME_BACK_COLOR_TRANSPARENCY, RES_BACKGROUND,      cppu::UnoType<sal_Int8>::get(), PROPERTY_NONE ,MID_BACK_COLOR_TRANSPARENCY},
        { UNO_NAME_FRAME_INTEROP_GRAB_BAG, RES_FRMATR_GRABBAG, cppu::UnoType< cppu::UnoSequenceType<css::beans::PropertyValue> >::get(), PROPERTY_NONE, 0},
    //  { UNO_NAME_CHAIN_NEXT_NAME, RES_CHAIN,                cppu::UnoType<OUString>::get(),            PROPERTY_NONE ,MID_CHAIN_NEXTNAME},
    //  { UNO_NAME_CHAIN_PREV_NAME, RES_CHAIN,                cppu::UnoType<OUString>::get(),            PROPERTY_NONE ,MID_CHAIN_PREVNAME},
    /*not impl*/    { UNO_NAME_CLIENT_MAP, RES_URL,               cppu::UnoType<bool>::get(),         PROPERTY_NONE ,MID_URL_CLIENTMAP         },
        { UNO_NAME_CONTENT_PROTECTED, RES_PROTECT,            cppu::UnoType<bool>::get(),             PROPERTY_NONE, MID_PROTECT_CONTENT   },
        { UNO_NAME_EDIT_IN_READONLY, RES_EDIT_IN_READONLY,    cppu::UnoType<bool>::get(),         PROPERTY_NONE, 0},
        { UNO_NAME_BACK_GRAPHIC_URL, RES_BACKGROUND,      cppu::UnoType<OUString>::get(), PROPERTY_NONE ,MID_GRAPHIC_URL    },
        { UNO_NAME_BACK_GRAPHIC, RES_BACKGROUND, cppu::UnoType<graphic::XGraphic>::get(), PROPERTY_NONE, MID_GRAPHIC },
        { UNO_NAME_BACK_GRAPHIC_FILTER, RES_BACKGROUND,       cppu::UnoType<OUString>::get(), PROPERTY_NONE ,MID_GRAPHIC_FILTER    },
        { UNO_NAME_BACK_GRAPHIC_LOCATION, RES_BACKGROUND,         cppu::UnoType<css::style::GraphicLocation>::get(),          PROPERTY_NONE ,MID_GRAPHIC_POSITION},
        // #i50322# - add missing map entry for transparency of graphic background
        { UNO_NAME_BACK_GRAPHIC_TRANSPARENCY, RES_BACKGROUND, cppu::UnoType<sal_Int8>::get(), PROPERTY_NONE ,MID_GRAPHIC_TRANSPARENCY},
        { UNO_NAME_LEFT_MARGIN, RES_LR_SPACE,             cppu::UnoType<sal_Int32>::get(), PROPERTY_NONE, MID_L_MARGIN|CONVERT_TWIPS},
        { UNO_NAME_RIGHT_MARGIN, RES_LR_SPACE,            cppu::UnoType<sal_Int32>::get(), PROPERTY_NONE, MID_R_MARGIN|CONVERT_TWIPS},
        { UNO_NAME_HORI_ORIENT, RES_HORI_ORIENT,      cppu::UnoType<sal_Int16>::get(),           PROPERTY_NONE ,MID_HORIORIENT_ORIENT    },
        { UNO_NAME_HORI_ORIENT_POSITION, RES_HORI_ORIENT,     cppu::UnoType<sal_Int32>::get(),           PROPERTY_NONE ,MID_HORIORIENT_POSITION|CONVERT_TWIPS    },
        { UNO_NAME_HORI_ORIENT_RELATION, RES_HORI_ORIENT,     cppu::UnoType<sal_Int16>::get(),           PROPERTY_NONE ,MID_HORIORIENT_RELATION  },
        { UNO_NAME_HYPER_LINK_U_R_L, RES_URL,                 cppu::UnoType<OUString>::get(), PROPERTY_NONE ,MID_URL_URL},
        { UNO_NAME_HYPER_LINK_TARGET, RES_URL,                cppu::UnoType<OUString>::get(), PROPERTY_NONE ,MID_URL_TARGET},
        { UNO_NAME_HYPER_LINK_NAME, RES_URL,              cppu::UnoType<OUString>::get(),            PROPERTY_NONE ,MID_URL_HYPERLINKNAME     },
        { UNO_NAME_OPAQUE, RES_OPAQUE,            cppu::UnoType<bool>::get(),         PROPERTY_NONE, 0},
        { UNO_NAME_PAGE_TOGGLE, RES_HORI_ORIENT,      cppu::UnoType<bool>::get(),         PROPERTY_NONE ,MID_HORIORIENT_PAGETOGGLE },
        { UNO_NAME_POSITION_PROTECTED, RES_PROTECT,           cppu::UnoType<bool>::get(),             PROPERTY_NONE, MID_PROTECT_POSITION},
        { UNO_NAME_PRINT, RES_PRINT,              cppu::UnoType<bool>::get(),         PROPERTY_NONE, 0},
        { UNO_NAME_WIDTH, RES_FRM_SIZE,           cppu::UnoType<sal_Int32>::get()  ,         PROPERTY_NONE, MID_FRMSIZE_WIDTH|CONVERT_TWIPS          },
        { UNO_NAME_HEIGHT, RES_FRM_SIZE,          cppu::UnoType<sal_Int32>::get()  ,         PROPERTY_NONE, MID_FRMSIZE_HEIGHT|CONVERT_TWIPS         },
        { UNO_NAME_RELATIVE_HEIGHT, RES_FRM_SIZE,         cppu::UnoType<sal_Int16>::get()  ,         PROPERTY_NONE,   MID_FRMSIZE_REL_HEIGHT },
        { UNO_NAME_RELATIVE_HEIGHT_RELATION, RES_FRM_SIZE, cppu::UnoType<sal_Int16>::get(),          PROPERTY_NONE,   MID_FRMSIZE_REL_HEIGHT_RELATION  },
        { UNO_NAME_RELATIVE_WIDTH, RES_FRM_SIZE,          cppu::UnoType<sal_Int16>::get()  ,         PROPERTY_NONE,   MID_FRMSIZE_REL_WIDTH  },
        { UNO_NAME_RELATIVE_WIDTH_RELATION, RES_FRM_SIZE,          cppu::UnoType<sal_Int16>::get()  ,         PROPERTY_NONE,   MID_FRMSIZE_REL_WIDTH_RELATION  },
        { UNO_NAME_SIZE_TYPE, RES_FRM_SIZE,           cppu::UnoType<sal_Int16>::get()  ,         PROPERTY_NONE,   MID_FRMSIZE_SIZE_TYPE  },
        { UNO_NAME_WIDTH_TYPE, RES_FRM_SIZE,          cppu::UnoType<sal_Int16>::get()  ,         PROPERTY_NONE,   MID_FRMSIZE_WIDTH_TYPE },
        { UNO_NAME_SIZE, RES_FRM_SIZE,            cppu::UnoType<css::awt::Size>::get(),             PROPERTY_NONE, MID_FRMSIZE_SIZE|CONVERT_TWIPS},
        { UNO_NAME_IS_SYNC_WIDTH_TO_HEIGHT, RES_FRM_SIZE,         cppu::UnoType<bool>::get()  ,       PROPERTY_NONE,   MID_FRMSIZE_IS_SYNC_WIDTH_TO_HEIGHT    },
        { UNO_NAME_IS_SYNC_HEIGHT_TO_WIDTH, RES_FRM_SIZE,         cppu::UnoType<bool>::get()  ,       PROPERTY_NONE,   MID_FRMSIZE_IS_SYNC_HEIGHT_TO_WIDTH },
    //  { UNO_NAME_WIDTH, RES_FRM_SIZE,           cppu::UnoType<sal_Int32>::get()  ,         PROPERTY_NONE, MID_FRMSIZE_WIDTH            },
        { UNO_NAME_SHADOW_FORMAT, RES_SHADOW,             cppu::UnoType<css::table::ShadowFormat>::get(),   PROPERTY_NONE, CONVERT_TWIPS},
        { UNO_NAME_SHADOW_TRANSPARENCE, RES_SHADOW,       cppu::UnoType<sal_Int16>::get(),       PROPERTY_NONE, MID_SHADOW_TRANSPARENCE},
        { UNO_NAME_SERVER_MAP, RES_URL,               cppu::UnoType<bool>::get(),         PROPERTY_NONE ,MID_URL_SERVERMAP         },
        { UNO_NAME_SIZE_PROTECTED, RES_PROTECT,           cppu::UnoType<bool>::get(),             PROPERTY_NONE, MID_PROTECT_SIZE    },
        // We keep Surround, as we delivered it with 5.1, although it's identical to text::WrapTextMode
        { UNO_NAME_SURROUND, RES_SURROUND,          cppu::UnoType<css::text::WrapTextMode>::get(),    PROPERTY_NONE, MID_SURROUND_SURROUNDTYPE },
        { UNO_NAME_TEXT_WRAP, RES_SURROUND,           cppu::UnoType<css::text::WrapTextMode>::get(),             PROPERTY_NONE, MID_SURROUND_SURROUNDTYPE    },
        { UNO_NAME_SURROUND_ANCHORONLY, RES_SURROUND,             cppu::UnoType<bool>::get(),             PROPERTY_NONE, MID_SURROUND_ANCHORONLY      },
        { UNO_NAME_SURROUND_CONTOUR, RES_SURROUND,            cppu::UnoType<bool>::get(),             PROPERTY_NONE, MID_SURROUND_CONTOUR         },
        { UNO_NAME_CONTOUR_OUTSIDE, RES_SURROUND,             cppu::UnoType<bool>::get(),             PROPERTY_NONE, MID_SURROUND_CONTOUROUTSIDE  },
        { UNO_NAME_TEXT_COLUMNS, RES_COL,                cppu::UnoType<css::text::XTextColumns>::get(),    PROPERTY_NONE, MID_COLUMNS},
        { UNO_NAME_TOP_MARGIN, RES_UL_SPACE,          cppu::UnoType<sal_Int32>::get(), PROPERTY_NONE, MID_UP_MARGIN|CONVERT_TWIPS},
        { UNO_NAME_BOTTOM_MARGIN, RES_UL_SPACE,           cppu::UnoType<sal_Int32>::get(), PROPERTY_NONE, MID_LO_MARGIN|CONVERT_TWIPS},
        { UNO_NAME_BACK_TRANSPARENT, RES_BACKGROUND,      cppu::UnoType<bool>::get(),         PROPERTY_NONE ,MID_GRAPHIC_TRANSPARENT       },
        { UNO_NAME_VERT_ORIENT, RES_VERT_ORIENT,      cppu::UnoType<sal_Int16>::get(),           PROPERTY_NONE ,MID_VERTORIENT_ORIENT    },
        { UNO_NAME_VERT_ORIENT_POSITION, RES_VERT_ORIENT,     cppu::UnoType<sal_Int32>::get(),           PROPERTY_NONE ,MID_VERTORIENT_POSITION|CONVERT_TWIPS    },
        { UNO_NAME_VERT_ORIENT_RELATION, RES_VERT_ORIENT,     cppu::UnoType<sal_Int16>::get(),           PROPERTY_NONE ,MID_VERTORIENT_RELATION  },
        { UNO_NAME_LEFT_BORDER, RES_BOX,              cppu::UnoType<css::table::BorderLine>::get(),  0, LEFT_BORDER  |CONVERT_TWIPS },
        { UNO_NAME_RIGHT_BORDER, RES_BOX,             cppu::UnoType<css::table::BorderLine>::get(),  0, RIGHT_BORDER |CONVERT_TWIPS },
        { UNO_NAME_TOP_BORDER, RES_BOX,               cppu::UnoType<css::table::BorderLine>::get(),  0, TOP_BORDER   |CONVERT_TWIPS },
        { UNO_NAME_BOTTOM_BORDER, RES_BOX,                cppu::UnoType<css::table::BorderLine>::get(),  0, BOTTOM_BORDER|CONVERT_TWIPS },
        { UNO_NAME_BORDER_DISTANCE, RES_BOX,    cppu::UnoType<sal_Int32>::get(), 0, BORDER_DISTANCE|CONVERT_TWIPS },
        { UNO_NAME_LEFT_BORDER_DISTANCE, RES_BOX,             cppu::UnoType<sal_Int32>::get(),   0, LEFT_BORDER_DISTANCE  |CONVERT_TWIPS },
        { UNO_NAME_RIGHT_BORDER_DISTANCE, RES_BOX,                cppu::UnoType<sal_Int32>::get(),   0, RIGHT_BORDER_DISTANCE |CONVERT_TWIPS },
        { UNO_NAME_TOP_BORDER_DISTANCE, RES_BOX,              cppu::UnoType<sal_Int32>::get(),   0, TOP_BORDER_DISTANCE   |CONVERT_TWIPS },
        { UNO_NAME_BOTTOM_BORDER_DISTANCE, RES_BOX,               cppu::UnoType<sal_Int32>::get(),   0, BOTTOM_BORDER_DISTANCE|CONVERT_TWIPS },
        { UNO_NAME_USER_DEFINED_ATTRIBUTES, RES_UNKNOWNATR_CONTAINER, cppu::UnoType<css::container::XNameContainer>::get(), PropertyAttribute::MAYBEVOID, 0 },
        { UNO_NAME_IS_PHYSICAL, FN_UNO_IS_PHYSICAL,     cppu::UnoType<bool>::get(), PropertyAttribute::READONLY, 0},
        { UNO_NAME_IS_AUTO_UPDATE, FN_UNO_IS_AUTO_UPDATE, cppu::UnoType<bool>::get(), PROPERTY_NONE, 0},
        { UNO_NAME_DISPLAY_NAME, FN_UNO_DISPLAY_NAME, cppu::UnoType<OUString>::get(), PropertyAttribute::READONLY, 0},
        // #i18732#
        { UNO_NAME_IS_FOLLOWING_TEXT_FLOW, RES_FOLLOW_TEXT_FLOW,     cppu::UnoType<bool>::get(), PROPERTY_NONE, MID_FOLLOW_TEXT_FLOW},
        // #i28701#
        { UNO_NAME_WRAP_INFLUENCE_ON_POSITION, RES_WRAP_INFLUENCE_ON_OBJPOS, cppu::UnoType<sal_Int8>::get(), PROPERTY_NONE, MID_WRAP_INFLUENCE},
        { UNO_NAME_ALLOW_OVERLAP, RES_WRAP_INFLUENCE_ON_OBJPOS, cppu::UnoType<bool>::get(), PROPERTY_NONE, MID_ALLOW_OVERLAP},
        { UNO_NAME_WRITING_MODE, RES_FRAMEDIR, cppu::UnoType<sal_Int16>::get(), PROPERTY_NONE, 0 },
        { UNO_NAME_HIDDEN, FN_UNO_HIDDEN,     cppu::UnoType<bool>::get(), PROPERTY_NONE, 0},
        { UNO_NAME_TEXT_VERT_ADJUST, RES_TEXT_VERT_ADJUST, cppu::UnoType<css::drawing::TextVerticalAdjust>::get(), PROPERTY_NONE ,0},

        // added FillProperties for SW, same as FILL_PROPERTIES in svx
        // but need own defines in Writer due to later association of strings
        // and uno types (see loop at end of this method and definition of SW_PROP_NMID)
        // This entry is for adding that properties to style import/export
        FILL_PROPERTIES_SW

        { u"", 0, css::uno::Type(), 0, 0 }
    };

    return aFrameStyleMap;
}

const SfxItemPropertyMapEntry*  SwUnoPropertyMapProvider::GetPageStylePropertyMap()
{
    static SfxItemPropertyMapEntry const aPageStyleMap   [] =
    {
        { UNO_NAME_BACK_COLOR, RES_BACKGROUND,            cppu::UnoType<sal_Int32>::get(),           PROPERTY_NONE ,MID_BACK_COLOR        },
        { UNO_NAME_BACK_GRAPHIC_URL, RES_BACKGROUND,      cppu::UnoType<OUString>::get(), PROPERTY_NONE ,MID_GRAPHIC_URL    },
        { UNO_NAME_BACK_GRAPHIC, RES_BACKGROUND, cppu::UnoType<graphic::XGraphic>::get(), PROPERTY_NONE, MID_GRAPHIC },
        { UNO_NAME_BACK_GRAPHIC_FILTER, RES_BACKGROUND,       cppu::UnoType<OUString>::get(), PROPERTY_NONE ,MID_GRAPHIC_FILTER    },
        { UNO_NAME_BACK_GRAPHIC_LOCATION, RES_BACKGROUND,         cppu::UnoType<css::style::GraphicLocation>::get(), PROPERTY_NONE ,MID_GRAPHIC_POSITION},
        { UNO_NAME_LEFT_MARGIN, RES_LR_SPACE,             cppu::UnoType<sal_Int32>::get(), PROPERTY_NONE, MID_L_MARGIN|CONVERT_TWIPS},
        { UNO_NAME_RIGHT_MARGIN, RES_LR_SPACE,            cppu::UnoType<sal_Int32>::get(), PROPERTY_NONE, MID_R_MARGIN|CONVERT_TWIPS},
        { UNO_NAME_GUTTER_MARGIN, RES_LR_SPACE, cppu::UnoType<sal_Int32>::get(), PROPERTY_NONE, MID_GUTTER_MARGIN | CONVERT_TWIPS},
        { UNO_NAME_BACK_TRANSPARENT, RES_BACKGROUND,      cppu::UnoType<bool>::get(),         PROPERTY_NONE ,MID_GRAPHIC_TRANSPARENT       },
        { UNO_NAME_LEFT_BORDER, RES_BOX,              cppu::UnoType<css::table::BorderLine>::get(),  0, LEFT_BORDER  |CONVERT_TWIPS },
        { UNO_NAME_RIGHT_BORDER, RES_BOX,             cppu::UnoType<css::table::BorderLine>::get(),  0, RIGHT_BORDER |CONVERT_TWIPS },
        { UNO_NAME_TOP_BORDER, RES_BOX,               cppu::UnoType<css::table::BorderLine>::get(),  0, TOP_BORDER   |CONVERT_TWIPS },
        { UNO_NAME_BOTTOM_BORDER, RES_BOX,                cppu::UnoType<css::table::BorderLine>::get(),  0, BOTTOM_BORDER|CONVERT_TWIPS },
        { UNO_NAME_BORDER_DISTANCE, RES_BOX,    cppu::UnoType<sal_Int32>::get(), 0, BORDER_DISTANCE|CONVERT_TWIPS },
        { UNO_NAME_LEFT_BORDER_DISTANCE, RES_BOX,             cppu::UnoType<sal_Int32>::get(),   0, LEFT_BORDER_DISTANCE  |CONVERT_TWIPS },
        { UNO_NAME_RIGHT_BORDER_DISTANCE, RES_BOX,                cppu::UnoType<sal_Int32>::get(),   0, RIGHT_BORDER_DISTANCE |CONVERT_TWIPS },
        { UNO_NAME_TOP_BORDER_DISTANCE, RES_BOX,              cppu::UnoType<sal_Int32>::get(),   0, TOP_BORDER_DISTANCE   |CONVERT_TWIPS },
        { UNO_NAME_BOTTOM_BORDER_DISTANCE, RES_BOX,               cppu::UnoType<sal_Int32>::get(),   0, BOTTOM_BORDER_DISTANCE|CONVERT_TWIPS },
        { UNO_NAME_SHADOW_FORMAT, RES_SHADOW,             cppu::UnoType<css::table::ShadowFormat>::get(),   PROPERTY_NONE, CONVERT_TWIPS},
        { UNO_NAME_SHADOW_TRANSPARENCE, RES_SHADOW,       cppu::UnoType<sal_Int16>::get(),       PROPERTY_NONE, MID_SHADOW_TRANSPARENCE},

        //UUU use real WhichIDs for Header, no longer use extra-defined WhichIDs which make handling harder as needed.
        // The implementation will decide if these are part of Header/Footer or PageStyle depending on the SlotName,
        // more precisely on the first characters. Thus it is necessary that these are 'Header' for the Header slots
        { UNO_NAME_HEADER_BACK_COLOR, RES_BACKGROUND,   cppu::UnoType<sal_Int32>::get(),           PROPERTY_NONE ,MID_BACK_COLOR        },
        { UNO_NAME_HEADER_GRAPHIC_URL, RES_BACKGROUND,          cppu::UnoType<OUString>::get(), PROPERTY_NONE ,MID_GRAPHIC_URL    },
        { UNO_NAME_HEADER_GRAPHIC, RES_BACKGROUND, cppu::UnoType<graphic::XGraphic>::get(), PROPERTY_NONE, MID_GRAPHIC },
        { UNO_NAME_HEADER_GRAPHIC_FILTER, RES_BACKGROUND,           cppu::UnoType<OUString>::get(), PROPERTY_NONE ,MID_GRAPHIC_FILTER    },
        { UNO_NAME_HEADER_GRAPHIC_LOCATION, RES_BACKGROUND,     cppu::UnoType<css::style::GraphicLocation>::get(), PROPERTY_NONE ,MID_GRAPHIC_POSITION},
        { UNO_NAME_HEADER_LEFT_MARGIN, RES_LR_SPACE,    cppu::UnoType<sal_Int32>::get(), PROPERTY_NONE, MID_L_MARGIN|CONVERT_TWIPS},
        { UNO_NAME_HEADER_RIGHT_MARGIN, RES_LR_SPACE,   cppu::UnoType<sal_Int32>::get(), PROPERTY_NONE, MID_R_MARGIN|CONVERT_TWIPS},
        { UNO_NAME_HEADER_BACK_TRANSPARENT, RES_BACKGROUND,     cppu::UnoType<bool>::get(),         PROPERTY_NONE ,MID_GRAPHIC_TRANSPARENT       },
        { UNO_NAME_HEADER_LEFT_BORDER, RES_BOX,             cppu::UnoType<css::table::BorderLine>::get(),  0, LEFT_BORDER  |CONVERT_TWIPS },
        { UNO_NAME_HEADER_RIGHT_BORDER, RES_BOX,                cppu::UnoType<css::table::BorderLine>::get(),  0, RIGHT_BORDER |CONVERT_TWIPS },
        { UNO_NAME_HEADER_TOP_BORDER, RES_BOX,              cppu::UnoType<css::table::BorderLine>::get(),  0, TOP_BORDER   |CONVERT_TWIPS },
        { UNO_NAME_HEADER_BOTTOM_BORDER, RES_BOX,               cppu::UnoType<css::table::BorderLine>::get(),  0, BOTTOM_BORDER|CONVERT_TWIPS },
        { UNO_NAME_HEADER_BORDER_DISTANCE, RES_BOX,    cppu::UnoType<sal_Int32>::get(),    PropertyAttribute::MAYBEVOID, BORDER_DISTANCE|CONVERT_TWIPS },
        { UNO_NAME_HEADER_LEFT_BORDER_DISTANCE, RES_BOX,                cppu::UnoType<sal_Int32>::get(),   0, LEFT_BORDER_DISTANCE  |CONVERT_TWIPS },
        { UNO_NAME_HEADER_RIGHT_BORDER_DISTANCE, RES_BOX,               cppu::UnoType<sal_Int32>::get(),   0, RIGHT_BORDER_DISTANCE |CONVERT_TWIPS },
        { UNO_NAME_HEADER_TOP_BORDER_DISTANCE, RES_BOX,             cppu::UnoType<sal_Int32>::get(),   0, TOP_BORDER_DISTANCE   |CONVERT_TWIPS },
        { UNO_NAME_HEADER_BOTTOM_BORDER_DISTANCE, RES_BOX,              cppu::UnoType<sal_Int32>::get(),   0, BOTTOM_BORDER_DISTANCE|CONVERT_TWIPS },
        { UNO_NAME_HEADER_SHADOW_FORMAT, RES_SHADOW,        cppu::UnoType<css::table::ShadowFormat>::get(),   PROPERTY_NONE, CONVERT_TWIPS},
        { UNO_NAME_HEADER_BODY_DISTANCE, RES_UL_SPACE,    cppu::UnoType<sal_Int32>::get(),            PROPERTY_NONE ,MID_LO_MARGIN|CONVERT_TWIPS       },
        { UNO_NAME_HEADER_IS_DYNAMIC_HEIGHT, SID_ATTR_PAGE_DYNAMIC,   cppu::UnoType<bool>::get(),            PROPERTY_NONE ,0         },
        { UNO_NAME_HEADER_IS_SHARED, SID_ATTR_PAGE_SHARED,    cppu::UnoType<bool>::get(),          PROPERTY_NONE ,0         },
        { UNO_NAME_HEADER_HEIGHT, SID_ATTR_PAGE_SIZE,       cppu::UnoType<sal_Int32>::get(),           PROPERTY_NONE ,MID_SIZE_HEIGHT|CONVERT_TWIPS         },
        { UNO_NAME_HEADER_IS_ON, SID_ATTR_PAGE_ON,            cppu::UnoType<bool>::get(),         PROPERTY_NONE ,0         },
        { UNO_NAME_HEADER_DYNAMIC_SPACING, RES_HEADER_FOOTER_EAT_SPACING,            cppu::UnoType<bool>::get(),         PropertyAttribute::MAYBEVOID ,0         },


        { UNO_NAME_FIRST_IS_SHARED, SID_ATTR_PAGE_SHARED_FIRST,   cppu::UnoType<bool>::get(), PROPERTY_NONE, 0 },

        //UUU use real WhichIDs for Footer, see Header (above) for more infos
        { UNO_NAME_FOOTER_BACK_COLOR, RES_BACKGROUND,   cppu::UnoType<sal_Int32>::get(),           PROPERTY_NONE ,MID_BACK_COLOR        },
        { UNO_NAME_FOOTER_GRAPHIC_URL, RES_BACKGROUND,      cppu::UnoType<OUString>::get(), PROPERTY_NONE ,MID_GRAPHIC_URL    },
        { UNO_NAME_FOOTER_GRAPHIC, RES_BACKGROUND, cppu::UnoType<graphic::XGraphic>::get(), PROPERTY_NONE, MID_GRAPHIC },
        { UNO_NAME_FOOTER_GRAPHIC_FILTER, RES_BACKGROUND,       cppu::UnoType<OUString>::get(), PROPERTY_NONE ,MID_GRAPHIC_FILTER    },
        { UNO_NAME_FOOTER_GRAPHIC_LOCATION, RES_BACKGROUND,     cppu::UnoType<css::style::GraphicLocation>::get(), PROPERTY_NONE ,MID_GRAPHIC_POSITION},
        { UNO_NAME_FOOTER_LEFT_MARGIN, RES_LR_SPACE,    cppu::UnoType<sal_Int32>::get(), PROPERTY_NONE, MID_L_MARGIN|CONVERT_TWIPS},
        { UNO_NAME_FOOTER_RIGHT_MARGIN, RES_LR_SPACE,   cppu::UnoType<sal_Int32>::get(), PROPERTY_NONE, MID_R_MARGIN|CONVERT_TWIPS},
        { UNO_NAME_FOOTER_BACK_TRANSPARENT, RES_BACKGROUND,     cppu::UnoType<bool>::get(),         PROPERTY_NONE ,MID_GRAPHIC_TRANSPARENT       },
        { UNO_NAME_FOOTER_LEFT_BORDER, RES_BOX,             cppu::UnoType<css::table::BorderLine>::get(),  0, LEFT_BORDER  |CONVERT_TWIPS },
        { UNO_NAME_FOOTER_RIGHT_BORDER, RES_BOX,                cppu::UnoType<css::table::BorderLine>::get(),  0, RIGHT_BORDER |CONVERT_TWIPS },
        { UNO_NAME_FOOTER_TOP_BORDER, RES_BOX,              cppu::UnoType<css::table::BorderLine>::get(),  0, TOP_BORDER   |CONVERT_TWIPS },
        { UNO_NAME_FOOTER_BOTTOM_BORDER, RES_BOX,               cppu::UnoType<css::table::BorderLine>::get(),  0, BOTTOM_BORDER|CONVERT_TWIPS },
        { UNO_NAME_FOOTER_BORDER_DISTANCE, RES_BOX,    cppu::UnoType<sal_Int32>::get(),    PropertyAttribute::MAYBEVOID, BORDER_DISTANCE|CONVERT_TWIPS },
        { UNO_NAME_FOOTER_LEFT_BORDER_DISTANCE, RES_BOX,                cppu::UnoType<sal_Int32>::get(),   0, LEFT_BORDER_DISTANCE  |CONVERT_TWIPS },
        { UNO_NAME_FOOTER_RIGHT_BORDER_DISTANCE, RES_BOX,               cppu::UnoType<sal_Int32>::get(),   0, RIGHT_BORDER_DISTANCE |CONVERT_TWIPS },
        { UNO_NAME_FOOTER_TOP_BORDER_DISTANCE, RES_BOX,             cppu::UnoType<sal_Int32>::get(),   0, TOP_BORDER_DISTANCE   |CONVERT_TWIPS },
        { UNO_NAME_FOOTER_BOTTOM_BORDER_DISTANCE, RES_BOX,              cppu::UnoType<sal_Int32>::get(),   0, BOTTOM_BORDER_DISTANCE|CONVERT_TWIPS },
        { UNO_NAME_FOOTER_SHADOW_FORMAT, RES_SHADOW,        cppu::UnoType<css::table::ShadowFormat>::get(),   PROPERTY_NONE, CONVERT_TWIPS},
        { UNO_NAME_FOOTER_BODY_DISTANCE, RES_UL_SPACE,    cppu::UnoType<sal_Int32>::get(),            PROPERTY_NONE ,MID_UP_MARGIN|CONVERT_TWIPS       },
        { UNO_NAME_FOOTER_IS_DYNAMIC_HEIGHT, SID_ATTR_PAGE_DYNAMIC,   cppu::UnoType<bool>::get(),            PROPERTY_NONE ,0         },
        { UNO_NAME_FOOTER_IS_SHARED, SID_ATTR_PAGE_SHARED,    cppu::UnoType<bool>::get(),          PROPERTY_NONE ,0         },
        { UNO_NAME_FOOTER_HEIGHT, SID_ATTR_PAGE_SIZE,       cppu::UnoType<sal_Int32>::get(),           PROPERTY_NONE ,MID_SIZE_HEIGHT|CONVERT_TWIPS         },
        { UNO_NAME_FOOTER_IS_ON, SID_ATTR_PAGE_ON,            cppu::UnoType<bool>::get(),         PROPERTY_NONE ,0         },
        { UNO_NAME_FOOTER_DYNAMIC_SPACING, RES_HEADER_FOOTER_EAT_SPACING,            cppu::UnoType<bool>::get(),         PropertyAttribute::MAYBEVOID ,0         },

        { UNO_NAME_IS_LANDSCAPE, SID_ATTR_PAGE,           cppu::UnoType<bool>::get(),             PROPERTY_NONE ,MID_PAGE_ORIENTATION   },
        { UNO_NAME_NUMBERING_TYPE, SID_ATTR_PAGE,             cppu::UnoType<sal_Int16>::get(),           PROPERTY_NONE , MID_PAGE_NUMTYPE       },
        { UNO_NAME_PAGE_STYLE_LAYOUT, SID_ATTR_PAGE,          cppu::UnoType<css::style::PageStyleLayout>::get(),    PROPERTY_NONE ,MID_PAGE_LAYOUT     },
        { UNO_NAME_PRINTER_PAPER_TRAY, RES_PAPER_BIN,             cppu::UnoType<OUString>::get(),            PROPERTY_NONE , 0 },
//                  { UNO_NAME_REGISTER_MODE_ACTIVE, SID_SWREGISTER_MODE,     cppu::UnoType<bool>::get(),             PROPERTY_NONE , 0 },
        { UNO_NAME_REGISTER_PARAGRAPH_STYLE, SID_SWREGISTER_COLLECTION,   cppu::UnoType<OUString>::get(),        PROPERTY_NONE , 0 },
        { UNO_NAME_SIZE, SID_ATTR_PAGE_SIZE,  cppu::UnoType<css::awt::Size>::get(),             PROPERTY_NONE,   MID_SIZE_SIZE|CONVERT_TWIPS},
        { UNO_NAME_WIDTH, SID_ATTR_PAGE_SIZE,     cppu::UnoType<sal_Int32>::get()  ,         PROPERTY_NONE, MID_SIZE_WIDTH|CONVERT_TWIPS},
        { UNO_NAME_HEIGHT, SID_ATTR_PAGE_SIZE,    cppu::UnoType<sal_Int32>::get()  ,         PROPERTY_NONE, MID_SIZE_HEIGHT|CONVERT_TWIPS            },
        { UNO_NAME_TEXT_VERT_ADJUST, RES_TEXT_VERT_ADJUST, cppu::UnoType<css::drawing::TextVerticalAdjust>::get(), PROPERTY_NONE, 0 },
        { UNO_NAME_TEXT_COLUMNS, RES_COL,                cppu::UnoType<css::text::XTextColumns>::get(),    PROPERTY_NONE, MID_COLUMNS},
        { UNO_NAME_TOP_MARGIN, RES_UL_SPACE,          cppu::UnoType<sal_Int32>::get(), PROPERTY_NONE, MID_UP_MARGIN|CONVERT_TWIPS},
        { UNO_NAME_BOTTOM_MARGIN, RES_UL_SPACE,           cppu::UnoType<sal_Int32>::get(), PROPERTY_NONE, MID_LO_MARGIN|CONVERT_TWIPS},
        { UNO_NAME_HEADER_TEXT, FN_UNO_HEADER,        cppu::UnoType<css::text::XText>::get(),         PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY, 0},
        { UNO_NAME_HEADER_TEXT_LEFT, FN_UNO_HEADER_LEFT,     cppu::UnoType<css::text::XText>::get(),          PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY, 0},
        { UNO_NAME_HEADER_TEXT_RIGHT, FN_UNO_HEADER_RIGHT,    cppu::UnoType<css::text::XText>::get(),         PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY, 0},
        { UNO_NAME_HEADER_TEXT_FIRST, FN_UNO_HEADER_FIRST,    cppu::UnoType<css::text::XText>::get(),         PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY, 0},
        { UNO_NAME_FOOTER_TEXT, FN_UNO_FOOTER,        cppu::UnoType<css::text::XText>::get(),         PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY, 0},
        { UNO_NAME_FOOTER_TEXT_LEFT, FN_UNO_FOOTER_LEFT,  cppu::UnoType<css::text::XText>::get(),         PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY, 0},
        { UNO_NAME_FOOTER_TEXT_RIGHT, FN_UNO_FOOTER_RIGHT,    cppu::UnoType<css::text::XText>::get(),         PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY, 0},
        { UNO_NAME_FOOTER_TEXT_FIRST, FN_UNO_FOOTER_FIRST,    cppu::UnoType<css::text::XText>::get(),         PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY, 0},
        { UNO_NAME_FOLLOW_STYLE, FN_UNO_FOLLOW_STYLE,     cppu::UnoType<OUString>::get(),        PROPERTY_NONE, 0},
        { UNO_NAME_USER_DEFINED_ATTRIBUTES, RES_UNKNOWNATR_CONTAINER, cppu::UnoType<css::container::XNameContainer>::get(), PropertyAttribute::MAYBEVOID, 0 },
        { UNO_NAME_IS_PHYSICAL, FN_UNO_IS_PHYSICAL,     cppu::UnoType<bool>::get(), PropertyAttribute::READONLY, 0},
        { UNO_NAME_DISPLAY_NAME, FN_UNO_DISPLAY_NAME, cppu::UnoType<OUString>::get(), PropertyAttribute::READONLY, 0},
        { UNO_NAME_FOOTNOTE_HEIGHT, FN_PARAM_FTN_INFO,        cppu::UnoType<sal_Int32>::get(), PROPERTY_NONE ,       MID_FTN_HEIGHT|CONVERT_TWIPS},
        { UNO_NAME_FOOTNOTE_LINE_WEIGHT, FN_PARAM_FTN_INFO,       cppu::UnoType<sal_Int16>::get(),   PROPERTY_NONE ,     MID_LINE_WEIGHT|CONVERT_TWIPS},
        { UNO_NAME_FOOTNOTE_LINE_COLOR, FN_PARAM_FTN_INFO,        cppu::UnoType<sal_Int32>::get(),   PROPERTY_NONE ,     MID_LINE_COLOR},
        { UNO_NAME_FOOTNOTE_LINE_STYLE, FN_PARAM_FTN_INFO,        cppu::UnoType<sal_Int8>::get(),    PROPERTY_NONE , MID_FTN_LINE_STYLE},
        { UNO_NAME_FOOTNOTE_LINE_RELATIVE_WIDTH, FN_PARAM_FTN_INFO,       cppu::UnoType<sal_Int8>::get(),        PROPERTY_NONE ,     MID_LINE_RELWIDTH    },
        { UNO_NAME_FOOTNOTE_LINE_ADJUST, FN_PARAM_FTN_INFO,       cppu::UnoType<sal_Int16>::get(),   PROPERTY_NONE ,     MID_LINE_ADJUST     },
        { UNO_NAME_FOOTNOTE_LINE_TEXT_DISTANCE, FN_PARAM_FTN_INFO,        cppu::UnoType<sal_Int32>::get(),   PROPERTY_NONE ,     MID_LINE_TEXT_DIST   |CONVERT_TWIPS },
        { UNO_NAME_FOOTNOTE_LINE_DISTANCE, FN_PARAM_FTN_INFO,         cppu::UnoType<sal_Int32>::get(),   PROPERTY_NONE ,     MID_LINE_FOOTNOTE_DIST|CONVERT_TWIPS},
        { UNO_NAME_WRITING_MODE, RES_FRAMEDIR, cppu::UnoType<sal_Int16>::get(), PROPERTY_NONE, 0 },
            // writing grid
        { UNO_NAME_GRID_COLOR, RES_TEXTGRID, cppu::UnoType<sal_Int32>::get(), PROPERTY_NONE, MID_GRID_COLOR},
        { UNO_NAME_GRID_LINES, RES_TEXTGRID, cppu::UnoType<sal_Int16>::get(), PROPERTY_NONE, MID_GRID_LINES},
        { UNO_NAME_GRID_BASE_HEIGHT, RES_TEXTGRID, cppu::UnoType<sal_Int32>::get(), PROPERTY_NONE, MID_GRID_BASEHEIGHT|CONVERT_TWIPS},
        { UNO_NAME_GRID_RUBY_HEIGHT, RES_TEXTGRID, cppu::UnoType<sal_Int32>::get(), PROPERTY_NONE, MID_GRID_RUBYHEIGHT|CONVERT_TWIPS},
        { UNO_NAME_GRID_MODE, RES_TEXTGRID, cppu::UnoType<sal_Int16>::get(), PROPERTY_NONE, MID_GRID_TYPE},
        { UNO_NAME_GRID_RUBY_BELOW, RES_TEXTGRID, cppu::UnoType<bool>::get(), PROPERTY_NONE, MID_GRID_RUBY_BELOW},
        { UNO_NAME_GRID_PRINT, RES_TEXTGRID, cppu::UnoType<bool>::get(), PROPERTY_NONE, MID_GRID_PRINT},
        { UNO_NAME_GRID_DISPLAY, RES_TEXTGRID, cppu::UnoType<bool>::get(), PROPERTY_NONE, MID_GRID_DISPLAY},
        { UNO_NAME_GRID_BASE_WIDTH, RES_TEXTGRID, cppu::UnoType<sal_Int32>::get(), PROPERTY_NONE, MID_GRID_BASEWIDTH|CONVERT_TWIPS},
        { UNO_NAME_GRID_SNAP_TO_CHARS, RES_TEXTGRID, cppu::UnoType<bool>::get(), PROPERTY_NONE, MID_GRID_SNAPTOCHARS},
        { UNO_NAME_GRID_STANDARD_PAGE_MODE, RES_TEXTGRID, cppu::UnoType<bool>::get(), PROPERTY_NONE, MID_GRID_STANDARD_MODE},
        { UNO_NAME_HIDDEN, FN_UNO_HIDDEN,     cppu::UnoType<bool>::get(), PROPERTY_NONE, 0},

        // added FillProperties for SW, same as FILL_PROPERTIES in svx
        // but need own defines in Writer due to later association of strings
        // and uno types (see loop at end of this method and definition of SW_PROP_NMID)
        // This entry is for adding that properties to style import/export
        FILL_PROPERTIES_SW
        { u"BackgroundFullSize", RES_BACKGROUND_FULL_SIZE,     cppu::UnoType<bool>::get(), PROPERTY_NONE, 0 },
        { u"RtlGutter", RES_RTL_GUTTER, cppu::UnoType<bool>::get(), PROPERTY_NONE, 0 },

        // Added DrawingLayer FillStyle Properties for Header. These need an own unique name,
        // but reuse the same WhichIDs as the regular fill. The implementation will decide to which
        // group of fill properties it belongs based on the start of the name (was already done in
        // the implementation partially), thus all SlotNames *have* to start with 'Header'
        { UNO_NAME_HEADER_FILLBMP_LOGICAL_SIZE,           XATTR_FILLBMP_SIZELOG,          cppu::UnoType<bool>::get() ,        0,  0},
        { UNO_NAME_HEADER_FILLBMP_OFFSET_X,               XATTR_FILLBMP_TILEOFFSETX,      cppu::UnoType<sal_Int32>::get() ,          0,  0},
        { UNO_NAME_HEADER_FILLBMP_OFFSET_Y,               XATTR_FILLBMP_TILEOFFSETY,      cppu::UnoType<sal_Int32>::get() ,          0,  0},
        { UNO_NAME_HEADER_FILLBMP_POSITION_OFFSET_X,      XATTR_FILLBMP_POSOFFSETX,       cppu::UnoType<sal_Int32>::get() ,          0,  0},
        { UNO_NAME_HEADER_FILLBMP_POSITION_OFFSET_Y,      XATTR_FILLBMP_POSOFFSETY,       cppu::UnoType<sal_Int32>::get() ,          0,  0},
        { UNO_NAME_HEADER_FILLBMP_RECTANGLE_POINT,        XATTR_FILLBMP_POS,              cppu::UnoType<css::drawing::RectanglePoint>::get() , 0,  0},
        { UNO_NAME_HEADER_FILLBMP_SIZE_X,                 XATTR_FILLBMP_SIZEX,            cppu::UnoType<sal_Int32>::get() ,          0,  0, PropertyMoreFlags::METRIC_ITEM},
        { UNO_NAME_HEADER_FILLBMP_SIZE_Y,                 XATTR_FILLBMP_SIZEY,            cppu::UnoType<sal_Int32>::get() ,          0,  0, PropertyMoreFlags::METRIC_ITEM},
        { UNO_NAME_HEADER_FILLBMP_STRETCH,                XATTR_FILLBMP_STRETCH,          cppu::UnoType<bool>::get() ,        0,  0},
        { UNO_NAME_HEADER_FILLBMP_TILE,                   XATTR_FILLBMP_TILE,             cppu::UnoType<bool>::get() ,        0,  0},
        { UNO_NAME_HEADER_FILLBMP_MODE,                   OWN_ATTR_FILLBMP_MODE,          cppu::UnoType<css::drawing::BitmapMode>::get(),      0,  0},
        { UNO_NAME_HEADER_FILLCOLOR,                      XATTR_FILLCOLOR,                cppu::UnoType<sal_Int32>::get(),           0,  0},
        { UNO_NAME_HEADER_FILLBACKGROUND,                 XATTR_FILLBACKGROUND,           cppu::UnoType<bool>::get(),         0,  0},
        { UNO_NAME_HEADER_FILLBITMAP,                     XATTR_FILLBITMAP,               cppu::UnoType<css::awt::XBitmap>::get(),       0,  MID_BITMAP},
        { UNO_NAME_HEADER_FILLBITMAPNAME,                 XATTR_FILLBITMAP,               cppu::UnoType<OUString>::get(),        0,  MID_NAME },
        { UNO_NAME_HEADER_FILLGRADIENTSTEPCOUNT,          XATTR_GRADIENTSTEPCOUNT,        cppu::UnoType<sal_Int16>::get(),           0,  0},
        { UNO_NAME_HEADER_FILLGRADIENT,                   XATTR_FILLGRADIENT,             cppu::UnoType<css::awt::Gradient>::get(),        0,  MID_FILLGRADIENT},
        { UNO_NAME_HEADER_FILLGRADIENTNAME,               XATTR_FILLGRADIENT,             cppu::UnoType<OUString>::get(),        0,  MID_NAME },
        { UNO_NAME_HEADER_FILLHATCH,                      XATTR_FILLHATCH,                cppu::UnoType<css::drawing::Hatch>::get(),           0,  MID_FILLHATCH},
        { UNO_NAME_HEADER_FILLHATCHNAME,                  XATTR_FILLHATCH,                cppu::UnoType<OUString>::get(),        0,  MID_NAME },
        { UNO_NAME_HEADER_FILLSTYLE,                      XATTR_FILLSTYLE,                cppu::UnoType<css::drawing::FillStyle>::get(),       0,  0},
        { UNO_NAME_HEADER_FILL_TRANSPARENCE,              XATTR_FILLTRANSPARENCE,         cppu::UnoType<sal_Int16>::get(),           0,  0},
        { UNO_NAME_HEADER_FILLTRANSPARENCEGRADIENT,       XATTR_FILLFLOATTRANSPARENCE,    cppu::UnoType<css::awt::Gradient>::get(),        0,  MID_FILLGRADIENT},
        { UNO_NAME_HEADER_FILLTRANSPARENCEGRADIENTNAME,   XATTR_FILLFLOATTRANSPARENCE,    cppu::UnoType<OUString>::get(),        0,  MID_NAME },
        { UNO_NAME_HEADER_FILLCOLOR_2,                    XATTR_SECONDARYFILLCOLOR,       cppu::UnoType<sal_Int32>::get(),           0,  0},

        // Added DrawingLayer FillStyle Properties for Footer, similar as for Header (see there)
        { UNO_NAME_FOOTER_FILLBMP_LOGICAL_SIZE,           XATTR_FILLBMP_SIZELOG,          cppu::UnoType<bool>::get() ,        0,  0},
        { UNO_NAME_FOOTER_FILLBMP_OFFSET_X,               XATTR_FILLBMP_TILEOFFSETX,      cppu::UnoType<sal_Int32>::get() ,          0,  0},
        { UNO_NAME_FOOTER_FILLBMP_OFFSET_Y,               XATTR_FILLBMP_TILEOFFSETY,      cppu::UnoType<sal_Int32>::get() ,          0,  0},
        { UNO_NAME_FOOTER_FILLBMP_POSITION_OFFSET_X,      XATTR_FILLBMP_POSOFFSETX,       cppu::UnoType<sal_Int32>::get() ,          0,  0},
        { UNO_NAME_FOOTER_FILLBMP_POSITION_OFFSET_Y,      XATTR_FILLBMP_POSOFFSETY,       cppu::UnoType<sal_Int32>::get() ,          0,  0},
        { UNO_NAME_FOOTER_FILLBMP_RECTANGLE_POINT,        XATTR_FILLBMP_POS,              cppu::UnoType<css::drawing::RectanglePoint>::get() , 0,  0},
        { UNO_NAME_FOOTER_FILLBMP_SIZE_X,                 XATTR_FILLBMP_SIZEX,            cppu::UnoType<sal_Int32>::get() ,          0,  0, PropertyMoreFlags::METRIC_ITEM},
        { UNO_NAME_FOOTER_FILLBMP_SIZE_Y,                 XATTR_FILLBMP_SIZEY,            cppu::UnoType<sal_Int32>::get() ,          0,  0, PropertyMoreFlags::METRIC_ITEM},
        { UNO_NAME_FOOTER_FILLBMP_STRETCH,                XATTR_FILLBMP_STRETCH,          cppu::UnoType<bool>::get() ,        0,  0},
        { UNO_NAME_FOOTER_FILLBMP_TILE,                   XATTR_FILLBMP_TILE,             cppu::UnoType<bool>::get() ,        0,  0},
        { UNO_NAME_FOOTER_FILLBMP_MODE,                   OWN_ATTR_FILLBMP_MODE,          cppu::UnoType<css::drawing::BitmapMode>::get(),      0,  0},
        { UNO_NAME_FOOTER_FILLCOLOR,                      XATTR_FILLCOLOR,                cppu::UnoType<sal_Int32>::get(),           0,  0},
        { UNO_NAME_FOOTER_FILLBACKGROUND,                 XATTR_FILLBACKGROUND,           cppu::UnoType<bool>::get(),         0,  0},
        { UNO_NAME_FOOTER_FILLBITMAP,                     XATTR_FILLBITMAP,               cppu::UnoType<css::awt::XBitmap>::get(),       0,  MID_BITMAP},
        { UNO_NAME_FOOTER_FILLBITMAPNAME,                 XATTR_FILLBITMAP,               cppu::UnoType<OUString>::get(),        0,  MID_NAME },
        { UNO_NAME_FOOTER_FILLGRADIENTSTEPCOUNT,          XATTR_GRADIENTSTEPCOUNT,        cppu::UnoType<sal_Int16>::get(),           0,  0},
        { UNO_NAME_FOOTER_FILLGRADIENT,                   XATTR_FILLGRADIENT,             cppu::UnoType<css::awt::Gradient>::get(),        0,  MID_FILLGRADIENT},
        { UNO_NAME_FOOTER_FILLGRADIENTNAME,               XATTR_FILLGRADIENT,             cppu::UnoType<OUString>::get(),        0,  MID_NAME },
        { UNO_NAME_FOOTER_FILLHATCH,                      XATTR_FILLHATCH,                cppu::UnoType<css::drawing::Hatch>::get(),           0,  MID_FILLHATCH},
        { UNO_NAME_FOOTER_FILLHATCHNAME,                  XATTR_FILLHATCH,                cppu::UnoType<OUString>::get(),        0,  MID_NAME },
        { UNO_NAME_FOOTER_FILLSTYLE,                      XATTR_FILLSTYLE,                cppu::UnoType<css::drawing::FillStyle>::get(),       0,  0},
        { UNO_NAME_FOOTER_FILL_TRANSPARENCE,              XATTR_FILLTRANSPARENCE,         cppu::UnoType<sal_Int16>::get(),           0,  0},
        { UNO_NAME_FOOTER_FILLTRANSPARENCEGRADIENT,       XATTR_FILLFLOATTRANSPARENCE,    cppu::UnoType<css::awt::Gradient>::get(),        0,  MID_FILLGRADIENT},
        { UNO_NAME_FOOTER_FILLTRANSPARENCEGRADIENTNAME,   XATTR_FILLFLOATTRANSPARENCE,    cppu::UnoType<OUString>::get(),        0,  MID_NAME },
        { UNO_NAME_FOOTER_FILLCOLOR_2,                    XATTR_SECONDARYFILLCOLOR,       cppu::UnoType<sal_Int32>::get(),           0,  0},

        { u"", 0, css::uno::Type(), 0, 0 }
    };

    return aPageStyleMap;
}

const SfxItemPropertyMapEntry*  SwUnoPropertyMapProvider::GetTablePropertyMap()
{
    static SfxItemPropertyMapEntry const aTablePropertyMap_Impl[] =
    {
        { UNO_NAME_BACK_COLOR, RES_BACKGROUND,        cppu::UnoType<sal_Int32>::get(),           PROPERTY_NONE,MID_BACK_COLOR         },
        { UNO_NAME_BREAK_TYPE, RES_BREAK,                 cppu::UnoType<css::style::BreakType>::get(),       PROPERTY_NONE, 0},
        { UNO_NAME_BACK_GRAPHIC_URL, RES_BACKGROUND,      cppu::UnoType<OUString>::get(), PROPERTY_NONE ,MID_GRAPHIC_URL    },
        { UNO_NAME_BACK_GRAPHIC, RES_BACKGROUND, cppu::UnoType<graphic::XGraphic>::get(), PROPERTY_NONE, MID_GRAPHIC },
        { UNO_NAME_BACK_GRAPHIC_FILTER, RES_BACKGROUND,       cppu::UnoType<OUString>::get(), PROPERTY_NONE ,MID_GRAPHIC_FILTER    },
        { UNO_NAME_BACK_GRAPHIC_LOCATION, RES_BACKGROUND,         cppu::UnoType<css::style::GraphicLocation>::get(), PROPERTY_NONE ,MID_GRAPHIC_POSITION},
        { UNO_NAME_LEFT_MARGIN, RES_LR_SPACE,             cppu::UnoType<sal_Int32>::get(), PROPERTY_NONE, MID_L_MARGIN|CONVERT_TWIPS},
        { UNO_NAME_RIGHT_MARGIN, RES_LR_SPACE,            cppu::UnoType<sal_Int32>::get(), PROPERTY_NONE, MID_R_MARGIN|CONVERT_TWIPS},
        { UNO_NAME_HORI_ORIENT, RES_HORI_ORIENT,      cppu::UnoType<sal_Int16>::get(),           PROPERTY_NONE ,MID_HORIORIENT_ORIENT    },
        { UNO_NAME_KEEP_TOGETHER, RES_KEEP,               cppu::UnoType<bool>::get()  ,       PROPERTY_NONE, 0},
        { UNO_NAME_SPLIT, RES_LAYOUT_SPLIT,       cppu::UnoType<bool>::get()  ,       PROPERTY_NONE, 0},
        { UNO_NAME_PAGE_NUMBER_OFFSET, RES_PAGEDESC,              cppu::UnoType<sal_Int16>::get(),       PropertyAttribute::MAYBEVOID, MID_PAGEDESC_PAGENUMOFFSET},
        { UNO_NAME_PAGE_DESC_NAME, RES_PAGEDESC,           cppu::UnoType<OUString>::get(),         PropertyAttribute::MAYBEVOID, 0xbf},
        { UNO_NAME_RELATIVE_WIDTH, FN_TABLE_RELATIVE_WIDTH,cppu::UnoType<sal_Int16>::get()  ,        PROPERTY_NONE, 0xbf },
        { UNO_NAME_REPEAT_HEADLINE, FN_TABLE_HEADLINE_REPEAT,cppu::UnoType<bool>::get(),      PROPERTY_NONE, 0xbf},
        { UNO_NAME_HEADER_ROW_COUNT, FN_TABLE_HEADLINE_COUNT,  cppu::UnoType<sal_Int32>::get(),      PROPERTY_NONE, 0xbf},
        { UNO_NAME_SHADOW_FORMAT, RES_SHADOW,             cppu::UnoType<css::table::ShadowFormat>::get(),   PROPERTY_NONE, 0},
        { UNO_NAME_SHADOW_TRANSPARENCE, RES_SHADOW,       cppu::UnoType<sal_Int16>::get(),       PROPERTY_NONE, MID_SHADOW_TRANSPARENCE},
        { UNO_NAME_TOP_MARGIN, RES_UL_SPACE,          cppu::UnoType<sal_Int32>::get(), PROPERTY_NONE, MID_UP_MARGIN|CONVERT_TWIPS},
        { UNO_NAME_BOTTOM_MARGIN, RES_UL_SPACE,           cppu::UnoType<sal_Int32>::get(), PROPERTY_NONE, MID_LO_MARGIN|CONVERT_TWIPS},
        { UNO_NAME_BACK_TRANSPARENT, RES_BACKGROUND,  cppu::UnoType<bool>::get(),         PROPERTY_NONE ,MID_GRAPHIC_TRANSPARENT       },
        { UNO_NAME_WIDTH, FN_TABLE_WIDTH,         cppu::UnoType<sal_Int32>::get()  ,         PROPERTY_NONE, 0xbf},
        { UNO_NAME_IS_WIDTH_RELATIVE, FN_TABLE_IS_RELATIVE_WIDTH,         cppu::UnoType<bool>::get()  ,       PROPERTY_NONE, 0xbf},
        { UNO_NAME_CHART_ROW_AS_LABEL, FN_UNO_RANGE_ROW_LABEL,            cppu::UnoType<bool>::get(),         PROPERTY_NONE,  0},
        { UNO_NAME_CHART_COLUMN_AS_LABEL, FN_UNO_RANGE_COL_LABEL,         cppu::UnoType<bool>::get()  ,       PROPERTY_NONE,     0},
        { UNO_NAME_TABLE_BORDER, FN_UNO_TABLE_BORDER,         cppu::UnoType<css::table::TableBorder>::get(), PropertyAttribute::MAYBEVOID, CONVERT_TWIPS },
        { UNO_NAME_TABLE_BORDER2, FN_UNO_TABLE_BORDER2,         cppu::UnoType<css::table::TableBorder2>::get(), PropertyAttribute::MAYBEVOID, CONVERT_TWIPS },
        { UNO_NAME_TABLE_BORDER_DISTANCES, FN_UNO_TABLE_BORDER_DISTANCES,         cppu::UnoType<css::table::TableBorderDistances>::get(), PropertyAttribute::MAYBEVOID, CONVERT_TWIPS },
        { UNO_NAME_TABLE_COLUMN_SEPARATORS, FN_UNO_TABLE_COLUMN_SEPARATORS,   cppu::UnoType< cppu::UnoSequenceType<css::text::TableColumnSeparator> >::get(),   PropertyAttribute::MAYBEVOID, 0 },
        { UNO_NAME_TABLE_COLUMN_RELATIVE_SUM, FN_UNO_TABLE_COLUMN_RELATIVE_SUM,       cppu::UnoType<sal_Int16>::get(),       PropertyAttribute::READONLY, 0 },
        COMMON_TEXT_CONTENT_PROPERTIES
        { UNO_LINK_DISPLAY_NAME, FN_PARAM_LINK_DISPLAY_NAME,  cppu::UnoType<OUString>::get(), PropertyAttribute::READONLY, 0xbf},
        { UNO_NAME_USER_DEFINED_ATTRIBUTES, RES_UNKNOWNATR_CONTAINER, cppu::UnoType<css::container::XNameContainer>::get(), PropertyAttribute::MAYBEVOID, 0 },
        { UNO_NAME_TEXT_SECTION, FN_UNO_TEXT_SECTION, cppu::UnoType<css::text::XTextSection>::get(),  PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY ,0 },
        { UNO_NAME_WRITING_MODE, RES_FRAMEDIR, cppu::UnoType<sal_Int16>::get(), PROPERTY_NONE, 0 },
        { UNO_NAME_TABLE_NAME,   FN_UNO_TABLE_NAME,   cppu::UnoType<OUString>::get(),        PROPERTY_NONE, 0 },
         { UNO_NAME_PAGE_STYLE_NAME, RES_PAGEDESC, cppu::UnoType<OUString>::get(),         PROPERTY_NONE, 0},
        { UNO_NAME_TABLE_TEMPLATE_NAME,   FN_UNO_TABLE_TEMPLATE_NAME,   cppu::UnoType<OUString>::get(),        PROPERTY_NONE, 0 },
        // #i29550#
        { UNO_NAME_COLLAPSING_BORDERS, RES_COLLAPSING_BORDERS, cppu::UnoType<bool>::get(), PROPERTY_NONE, 0},
        REDLINE_NODE_PROPERTIES
        { UNO_NAME_TABLE_INTEROP_GRAB_BAG, RES_FRMATR_GRABBAG, cppu::UnoType< cppu::UnoSequenceType<css::beans::PropertyValue> >::get(), PROPERTY_NONE, 0 },
        { u"", 0, css::uno::Type(), 0, 0 }
    };

    return aTablePropertyMap_Impl;
}

const SfxItemPropertyMapEntry*  SwUnoPropertyMapProvider::GetRangePropertyMap()
{
    static SfxItemPropertyMapEntry const aRangePropertyMap_Impl[] =
    {
        COMMON_CRSR_PARA_PROPERTIES_WITHOUT_FN_01
        TABSTOPS_MAP_ENTRY
        { UNO_NAME_BACK_COLOR, FN_UNO_TABLE_CELL_BACKGROUND,  cppu::UnoType<sal_Int32>::get(),   PropertyAttribute::MAYBEVOID, MID_BACK_COLOR  },
        { UNO_NAME_BACK_GRAPHIC_URL, RES_BACKGROUND,      cppu::UnoType<OUString>::get(), PropertyAttribute::MAYBEVOID ,MID_GRAPHIC_URL    },
        { UNO_NAME_BACK_GRAPHIC, RES_BACKGROUND, cppu::UnoType<graphic::XGraphic>::get(), PROPERTY_NONE, MID_GRAPHIC },
        { UNO_NAME_BACK_GRAPHIC_FILTER, RES_BACKGROUND,       cppu::UnoType<OUString>::get(), PropertyAttribute::MAYBEVOID ,MID_GRAPHIC_FILTER    },
        { UNO_NAME_BACK_GRAPHIC_LOCATION, FN_UNO_TABLE_CELL_BACKGROUND,   cppu::UnoType<css::style::GraphicLocation>::get(), PropertyAttribute::MAYBEVOID, MID_GRAPHIC_POSITION},
        { UNO_NAME_BACK_TRANSPARENT, FN_UNO_TABLE_CELL_BACKGROUND,    cppu::UnoType<bool>::get(), PropertyAttribute::MAYBEVOID, MID_GRAPHIC_TRANSPARENT      },
        { UNO_NAME_NUMBER_FORMAT, RES_BOXATR_FORMAT,         cppu::UnoType<sal_Int32>::get(),           PropertyAttribute::MAYBEVOID ,0             },
        { UNO_NAME_VERT_ORIENT, RES_VERT_ORIENT,      cppu::UnoType<sal_Int16>::get(),           PROPERTY_NONE ,MID_VERTORIENT_ORIENT    },
        { UNO_NAME_CHART_ROW_AS_LABEL, FN_UNO_RANGE_ROW_LABEL,    cppu::UnoType<bool>::get(),         PropertyAttribute::MAYBEVOID,  0},
        { UNO_NAME_CHART_COLUMN_AS_LABEL, FN_UNO_RANGE_COL_LABEL, cppu::UnoType<bool>::get()  ,       PropertyAttribute::MAYBEVOID,     0},
        { u"", 0, css::uno::Type(), 0, 0 }
    };

    return aRangePropertyMap_Impl;
}

const SfxItemPropertyMapEntry*  SwUnoPropertyMapProvider::GetSectionPropertyMap()
{
    static SfxItemPropertyMapEntry const aSectionPropertyMap_Impl[] =
    {
        { UNO_NAME_CONDITION, WID_SECT_CONDITION, cppu::UnoType<OUString>::get()  ,      PROPERTY_NONE,     0},
        { UNO_NAME_DDE_COMMAND_FILE, WID_SECT_DDE_TYPE, cppu::UnoType<OUString>::get()  ,        PROPERTY_NONE,     0},
        { UNO_NAME_DDE_COMMAND_TYPE, WID_SECT_DDE_FILE, cppu::UnoType<OUString>::get()  ,        PROPERTY_NONE,     0},
        { UNO_NAME_DDE_COMMAND_ELEMENT, WID_SECT_DDE_ELEMENT, cppu::UnoType<OUString>::get()  ,      PROPERTY_NONE,     0},
        { UNO_NAME_IS_AUTOMATIC_UPDATE, WID_SECT_DDE_AUTOUPDATE, cppu::UnoType<bool>::get(),              PROPERTY_NONE,  0},
        { UNO_NAME_FILE_LINK, WID_SECT_LINK     , cppu::UnoType<css::text::SectionFileLink>::get(),     PROPERTY_NONE,     0},
        { UNO_NAME_IS_VISIBLE, WID_SECT_VISIBLE   , cppu::UnoType<bool>::get(),           PROPERTY_NONE,     0},
        { UNO_NAME_IS_PROTECTED, WID_SECT_PROTECTED, cppu::UnoType<bool>::get(),              PROPERTY_NONE,     0},
        { UNO_NAME_EDIT_IN_READONLY, WID_SECT_EDIT_IN_READONLY,    cppu::UnoType<bool>::get(),         PROPERTY_NONE, 0},
        { UNO_NAME_LINK_REGION, WID_SECT_REGION   , cppu::UnoType<OUString>::get()  ,        PROPERTY_NONE,     0},
        { UNO_NAME_TEXT_COLUMNS, RES_COL,                cppu::UnoType<css::text::XTextColumns>::get(),    PROPERTY_NONE, MID_COLUMNS},
        { UNO_NAME_BACK_GRAPHIC_URL, RES_BACKGROUND,      cppu::UnoType<OUString>::get(), PROPERTY_NONE ,MID_GRAPHIC_URL    },
        { UNO_NAME_BACK_GRAPHIC, RES_BACKGROUND, cppu::UnoType<graphic::XGraphic>::get(), PROPERTY_NONE, MID_GRAPHIC },
        { UNO_NAME_BACK_GRAPHIC_FILTER, RES_BACKGROUND,       cppu::UnoType<OUString>::get(), PROPERTY_NONE ,MID_GRAPHIC_FILTER    },
        { UNO_NAME_BACK_GRAPHIC_LOCATION, RES_BACKGROUND,         cppu::UnoType<css::style::GraphicLocation>::get(),          PROPERTY_NONE ,MID_GRAPHIC_POSITION},
        { UNO_NAME_BACK_COLOR, RES_BACKGROUND,            cppu::UnoType<sal_Int32>::get(),           PROPERTY_NONE ,MID_BACK_COLOR        },
        { UNO_NAME_BACK_TRANSPARENT, RES_BACKGROUND,      cppu::UnoType<bool>::get(),         PROPERTY_NONE ,MID_GRAPHIC_TRANSPARENT       },
        { UNO_LINK_DISPLAY_NAME, FN_PARAM_LINK_DISPLAY_NAME,  cppu::UnoType<OUString>::get(), PropertyAttribute::READONLY, 0xbf},
        { UNO_NAME_USER_DEFINED_ATTRIBUTES, RES_UNKNOWNATR_CONTAINER, cppu::UnoType<css::container::XNameContainer>::get(), PropertyAttribute::MAYBEVOID, 0 },
        { UNO_NAME_FOOTNOTE_IS_COLLECT_AT_TEXT_END, RES_FTN_AT_TXTEND,        cppu::UnoType<bool>::get(),                PROPERTY_NONE ,MID_COLLECT                   },
        { UNO_NAME_FOOTNOTE_IS_RESTART_NUMBERING, RES_FTN_AT_TXTEND,      cppu::UnoType<bool>::get(),                PROPERTY_NONE , MID_RESTART_NUM },
        { UNO_NAME_FOOTNOTE_RESTART_NUMBERING_AT, RES_FTN_AT_TXTEND,      cppu::UnoType<sal_Int16>::get(),               PROPERTY_NONE , MID_NUM_START_AT},
        { UNO_NAME_FOOTNOTE_IS_OWN_NUMBERING, RES_FTN_AT_TXTEND,  cppu::UnoType<bool>::get(),                                                 PROPERTY_NONE ,  MID_OWN_NUM     },
        { UNO_NAME_FOOTNOTE_NUMBERING_TYPE, RES_FTN_AT_TXTEND,        cppu::UnoType<sal_Int16>::get(),               PROPERTY_NONE , MID_NUM_TYPE    },
        { UNO_NAME_FOOTNOTE_NUMBERING_PREFIX, RES_FTN_AT_TXTEND,  cppu::UnoType<OUString>::get()  ,      PROPERTY_NONE,      MID_PREFIX      },
        { UNO_NAME_FOOTNOTE_NUMBERING_SUFFIX, RES_FTN_AT_TXTEND,  cppu::UnoType<OUString>::get()  ,      PROPERTY_NONE,      MID_SUFFIX      },
        { UNO_NAME_ENDNOTE_IS_COLLECT_AT_TEXT_END, RES_END_AT_TXTEND,        cppu::UnoType<bool>::get(),                 PROPERTY_NONE , MID_COLLECT                      },
        { UNO_NAME_ENDNOTE_IS_RESTART_NUMBERING, RES_END_AT_TXTEND,        cppu::UnoType<bool>::get(),                   PROPERTY_NONE , MID_RESTART_NUM  },
        { UNO_NAME_ENDNOTE_RESTART_NUMBERING_AT, RES_END_AT_TXTEND,     cppu::UnoType<sal_Int16>::get(),             PROPERTY_NONE ,  MID_NUM_START_AT },
        { UNO_NAME_ENDNOTE_IS_OWN_NUMBERING, RES_END_AT_TXTEND,     cppu::UnoType<bool>::get(),                                                   PROPERTY_NONE ,  MID_OWN_NUM      },
        { UNO_NAME_ENDNOTE_NUMBERING_TYPE, RES_END_AT_TXTEND,       cppu::UnoType<sal_Int16>::get(),             PROPERTY_NONE ,MID_NUM_TYPE     },
        { UNO_NAME_ENDNOTE_NUMBERING_PREFIX, RES_END_AT_TXTEND,     cppu::UnoType<OUString>::get()  ,        PROPERTY_NONE,   MID_PREFIX       },
        { UNO_NAME_ENDNOTE_NUMBERING_SUFFIX, RES_END_AT_TXTEND,     cppu::UnoType<OUString>::get()  ,        PROPERTY_NONE,   MID_SUFFIX       },
        { UNO_NAME_DOCUMENT_INDEX, WID_SECT_DOCUMENT_INDEX, cppu::UnoType<css::text::XDocumentIndex>::get(), PropertyAttribute::READONLY | PropertyAttribute::MAYBEVOID, 0 },
        { UNO_NAME_IS_GLOBAL_DOCUMENT_SECTION, WID_SECT_IS_GLOBAL_DOC_SECTION, cppu::UnoType<bool>::get(), PropertyAttribute::READONLY, 0 },
        { UNO_NAME_PROTECTION_KEY, WID_SECT_PASSWORD,      cppu::UnoType< cppu::UnoSequenceType<sal_Int8> >::get(), PROPERTY_NONE, 0 },
        { UNO_NAME_DONT_BALANCE_TEXT_COLUMNS, RES_COLUMNBALANCE, cppu::UnoType<bool>::get(), PROPERTY_NONE, 0 },
        COMMON_TEXT_CONTENT_PROPERTIES
        REDLINE_NODE_PROPERTIES
        { UNO_NAME_IS_CURRENTLY_VISIBLE, WID_SECT_CURRENTLY_VISIBLE, cppu::UnoType<bool>::get(),          PROPERTY_NONE,     0},
        { UNO_NAME_WRITING_MODE, RES_FRAMEDIR, cppu::UnoType<sal_Int16>::get(), PROPERTY_NONE, 0 },
        { UNO_NAME_SECT_LEFT_MARGIN, RES_LR_SPACE,            cppu::UnoType<sal_Int32>::get(),           PropertyAttribute::MAYBEVOID, MID_L_MARGIN|CONVERT_TWIPS},
        { UNO_NAME_SECT_RIGHT_MARGIN, RES_LR_SPACE,           cppu::UnoType<sal_Int32>::get(),           PropertyAttribute::MAYBEVOID, MID_R_MARGIN|CONVERT_TWIPS},
        { u"", 0, css::uno::Type(), 0, 0 }
    };

    return aSectionPropertyMap_Impl;
}

const SfxItemPropertyMapEntry*  SwUnoPropertyMapProvider::GetFramePropertyMap()
{
    static SfxItemPropertyMapEntry const aFramePropertyMap_Impl[] =
    {   //
        // TODO: We should consider completely removing SvxBrushItem() stuff
        // add support for XATTR_FILL_FIRST, XATTR_FILL_LAST
        // COMMON_FRAME_PROPERTIES currently hosts the RES_BACKGROUND entries from SvxBrushItem
        COMMON_FRAME_PROPERTIES
        REDLINE_NODE_PROPERTIES
        { UNO_NAME_CHAIN_NEXT_NAME, RES_CHAIN,                cppu::UnoType<OUString>::get(),            PropertyAttribute::MAYBEVOID ,MID_CHAIN_NEXTNAME},
        { UNO_NAME_CHAIN_PREV_NAME, RES_CHAIN,                cppu::UnoType<OUString>::get(),            PropertyAttribute::MAYBEVOID ,MID_CHAIN_PREVNAME},
    /*not impl*/    { UNO_NAME_CLIENT_MAP, RES_URL,               cppu::UnoType<bool>::get(),         PROPERTY_NONE ,MID_URL_CLIENTMAP         },
        { UNO_NAME_EDIT_IN_READONLY, RES_EDIT_IN_READONLY,    cppu::UnoType<bool>::get(),         PROPERTY_NONE, 0},
        { UNO_NAME_TEXT_COLUMNS, RES_COL,                cppu::UnoType<css::text::XTextColumns>::get(),    PROPERTY_NONE, MID_COLUMNS},
        //next elements are part of the service description
        { UNO_NAME_FRAME_HEIGHT_ABSOLUTE,         RES_FRM_SIZE,           cppu::UnoType<sal_Int32>::get(),       PROPERTY_NONE, MID_FRMSIZE_HEIGHT|CONVERT_TWIPS         },
        { UNO_NAME_FRAME_HEIGHT_PERCENT,              RES_FRM_SIZE,           cppu::UnoType<sal_Int8>::get(),        PROPERTY_NONE, MID_FRMSIZE_REL_HEIGHT   },
        { UNO_NAME_FRAME_ISAUTOMATIC_HEIGHT,         RES_FRM_SIZE,        cppu::UnoType<bool>::get(),         PROPERTY_NONE, MID_FRMSIZE_IS_AUTO_HEIGHT   },
        { UNO_NAME_FRAME_WIDTH_ABSOLUTE,          RES_FRM_SIZE,           cppu::UnoType<sal_Int32>::get(),       PROPERTY_NONE, MID_FRMSIZE_WIDTH|CONVERT_TWIPS          },
        { UNO_NAME_FRAME_WIDTH_PERCENT,               RES_FRM_SIZE,           cppu::UnoType<sal_Int8>::get(),        PROPERTY_NONE, MID_FRMSIZE_REL_WIDTH    },
        { UNO_NAME_SIZE_TYPE, RES_FRM_SIZE,           cppu::UnoType<sal_Int16>::get()  ,         PROPERTY_NONE,   MID_FRMSIZE_SIZE_TYPE  },
        { UNO_NAME_WIDTH_TYPE, RES_FRM_SIZE,          cppu::UnoType<sal_Int16>::get()  ,         PROPERTY_NONE,   MID_FRMSIZE_WIDTH_TYPE },
        { UNO_NAME_WRITING_MODE, RES_FRAMEDIR, cppu::UnoType<sal_Int16>::get(), PROPERTY_NONE, 0 },

        // added FillProperties for SW, same as FILL_PROPERTIES in svx
        // but need own defines in Writer due to later association of strings
        // and uno types (see loop at end of this method and definition of SW_PROP_NMID)
        // This entry is for adding that properties to FlyFrame import/export
        FILL_PROPERTIES_SW

        { u"", 0, css::uno::Type(), 0, 0 }
    };

    return aFramePropertyMap_Impl;
}

const SfxItemPropertyMapEntry*  SwUnoPropertyMapProvider::GetGraphicPropertyMap()
{
    static SfxItemPropertyMapEntry const aGraphicPropertyMap_Impl[] =
    {
        // TODO: We should consider completely removing SvxBrushItem() stuff
        // add support for XATTR_FILL_FIRST, XATTR_FILL_LAST
        // COMMON_FRAME_PROPERTIES currently hosts the RES_BACKGROUND entries from SvxBrushItem
        COMMON_FRAME_PROPERTIES
        { UNO_NAME_SURROUND_CONTOUR, RES_SURROUND,            cppu::UnoType<bool>::get(),             PROPERTY_NONE, MID_SURROUND_CONTOUR         },
        { UNO_NAME_CONTOUR_OUTSIDE, RES_SURROUND,             cppu::UnoType<bool>::get(),             PROPERTY_NONE, MID_SURROUND_CONTOUROUTSIDE  },
        { UNO_NAME_GRAPHIC_CROP, RES_GRFATR_CROPGRF,     cppu::UnoType<css::text::GraphicCrop>::get(),  PROPERTY_NONE, CONVERT_TWIPS },
        { UNO_NAME_HORI_MIRRORED_ON_EVEN_PAGES, RES_GRFATR_MIRRORGRF, cppu::UnoType<bool>::get(),             PROPERTY_NONE,      MID_MIRROR_HORZ_EVEN_PAGES            },
        { UNO_NAME_HORI_MIRRORED_ON_ODD_PAGES, RES_GRFATR_MIRRORGRF,  cppu::UnoType<bool>::get(),             PROPERTY_NONE,      MID_MIRROR_HORZ_ODD_PAGES                 },
        { UNO_NAME_VERT_MIRRORED, RES_GRFATR_MIRRORGRF,   cppu::UnoType<bool>::get(),             PROPERTY_NONE,     MID_MIRROR_VERT            },
        { UNO_NAME_REPLACEMENT_GRAPHIC, FN_UNO_REPLACEMENT_GRAPHIC, cppu::UnoType<css::graphic::XGraphic>::get(), 0, 0 },
        { UNO_NAME_GRAPHIC_FILTER, FN_UNO_GRAPHIC_FILTER,      cppu::UnoType<OUString>::get(), 0, 0 },
        { UNO_NAME_GRAPHIC, FN_UNO_GRAPHIC, cppu::UnoType<css::graphic::XGraphic>::get(), 0, 0 },
        { UNO_NAME_GRAPHIC_URL, FN_UNO_GRAPHIC_URL, cppu::UnoType<css::uno::Any>::get(), 0, 0 },
        { UNO_NAME_TRANSFORMED_GRAPHIC, FN_UNO_TRANSFORMED_GRAPHIC, cppu::UnoType<css::graphic::XGraphic>::get(), 0, 0 },
        { UNO_NAME_ACTUAL_SIZE, FN_UNO_ACTUAL_SIZE,    cppu::UnoType<css::awt::Size>::get(),  PropertyAttribute::READONLY, CONVERT_TWIPS},
        { UNO_NAME_CONTOUR_POLY_POLYGON, FN_PARAM_CONTOUR_PP, cppu::UnoType<css::drawing::PointSequenceSequence>::get(), PropertyAttribute::MAYBEVOID, 0 },
        { UNO_NAME_IS_PIXEL_CONTOUR, FN_UNO_IS_PIXEL_CONTOUR, cppu::UnoType<bool>::get(), PROPERTY_NONE, 0 },
        { UNO_NAME_IS_AUTOMATIC_CONTOUR, FN_UNO_IS_AUTOMATIC_CONTOUR , cppu::UnoType<bool>::get(), PROPERTY_NONE, 0 },
        { UNO_NAME_GRAPHIC_ROTATION, RES_GRFATR_ROTATION,      cppu::UnoType<sal_Int16>::get(),  0,   0},
        { UNO_NAME_ADJUST_LUMINANCE, RES_GRFATR_LUMINANCE,     cppu::UnoType<sal_Int16>::get(),  0,   0},
        { UNO_NAME_ADJUST_CONTRAST, RES_GRFATR_CONTRAST,   cppu::UnoType<sal_Int16>::get(),  0,   0},
        { UNO_NAME_ADJUST_RED, RES_GRFATR_CHANNELR,    cppu::UnoType<sal_Int16>::get(),  0,   0},
        { UNO_NAME_ADJUST_GREEN, RES_GRFATR_CHANNELG,      cppu::UnoType<sal_Int16>::get(),  0,   0},
        { UNO_NAME_ADJUST_BLUE, RES_GRFATR_CHANNELB,   cppu::UnoType<sal_Int16>::get(),  0,   0},
        { UNO_NAME_GAMMA, RES_GRFATR_GAMMA,        cppu::UnoType<double>::get(),     0,   0},
        { UNO_NAME_GRAPHIC_IS_INVERTED, RES_GRFATR_INVERT,         cppu::UnoType<bool>::get(),    0,   0},
        { UNO_NAME_TRANSPARENCY, RES_GRFATR_TRANSPARENCY, cppu::UnoType<sal_Int16>::get(),   0,   0},
        { UNO_NAME_GRAPHIC_COLOR_MODE, RES_GRFATR_DRAWMODE,    cppu::UnoType<css::drawing::ColorMode>::get(),      0,   0},

        // added FillProperties for SW, same as FILL_PROPERTIES in svx
        // but need own defines in Writer due to later association of strings
        // and uno types (see loop at end of this method and definition of SW_PROP_NMID)
        // This entry is for adding that properties to Writer GraphicObject import/export
        FILL_PROPERTIES_SW

        { u"", 0, css::uno::Type(), 0, 0 }
    };

    return aGraphicPropertyMap_Impl;
}

const SfxItemPropertyMapEntry*  SwUnoPropertyMapProvider::GetEmbeddedPropertyMap()
{
    static SfxItemPropertyMapEntry const aEmbeddedPropertyMap_Impl[] =
    {   //
        // TODO: We should consider completely removing SvxBrushItem() stuff
        // add support for XATTR_FILL_FIRST, XATTR_FILL_LAST
        // COMMON_FRAME_PROPERTIES currently hosts the RES_BACKGROUND entries from SvxBrushItem
        COMMON_FRAME_PROPERTIES
        { UNO_NAME_SURROUND_CONTOUR, RES_SURROUND, cppu::UnoType<bool>::get(), PROPERTY_NONE, MID_SURROUND_CONTOUR },
        { UNO_NAME_CONTOUR_OUTSIDE, RES_SURROUND, cppu::UnoType<bool>::get(), PROPERTY_NONE, MID_SURROUND_CONTOUROUTSIDE},
        { UNO_NAME_CONTOUR_POLY_POLYGON, FN_PARAM_CONTOUR_PP, cppu::UnoType<css::drawing::PointSequenceSequence>::get(), PropertyAttribute::MAYBEVOID, 0 },
        { UNO_NAME_IS_PIXEL_CONTOUR, FN_UNO_IS_PIXEL_CONTOUR, cppu::UnoType<bool>::get(), PROPERTY_NONE, 0 },
        { UNO_NAME_IS_AUTOMATIC_CONTOUR, FN_UNO_IS_AUTOMATIC_CONTOUR , cppu::UnoType<bool>::get(), PROPERTY_NONE, 0 },
        { UNO_NAME_CLSID,                FN_UNO_CLSID, cppu::UnoType<OUString>::get(), PROPERTY_NONE, 0 },
        { UNO_NAME_STREAM_NAME,           FN_UNO_STREAM_NAME, cppu::UnoType<OUString>::get(), PROPERTY_NONE, 0 },
        { UNO_NAME_MODEL, FN_UNO_MODEL, cppu::UnoType<css::frame::XModel>::get(), PropertyAttribute::READONLY|PropertyAttribute::MAYBEVOID, 0},
        { UNO_NAME_GRAPHIC_URL, FN_UNO_REPLACEMENT_GRAPHIC_URL, cppu::UnoType<OUString>::get(), PropertyAttribute::MAYBEVOID, 0 },
        { UNO_NAME_GRAPHIC, FN_UNO_REPLACEMENT_GRAPHIC, cppu::UnoType<css::graphic::XGraphic>::get(), PropertyAttribute::MAYBEVOID, 0 },
        { UNO_NAME_COMPONENT,FN_UNO_COMPONENT, cppu::UnoType<css::lang::XComponent>::get(), PropertyAttribute::READONLY, 0},
        { UNO_NAME_EMBEDDED_OBJECT,FN_EMBEDDED_OBJECT, cppu::UnoType<css::embed::XEmbeddedObject>::get(), PROPERTY_NONE, 0},
        { UNO_NAME_DRAW_ASPECT,FN_UNO_DRAW_ASPECT, cppu::UnoType<OUString>::get(), PROPERTY_NONE, 0 },
        { UNO_NAME_VISIBLE_AREA_WIDTH,FN_UNO_VISIBLE_AREA_WIDTH, cppu::UnoType<OUString>::get(), PROPERTY_NONE, 0 },
        { UNO_NAME_VISIBLE_AREA_HEIGHT,FN_UNO_VISIBLE_AREA_HEIGHT, cppu::UnoType<OUString>::get(), PROPERTY_NONE, 0 },
        // added FillProperties for SW, same as FILL_PROPERTIES in svx
        // but need own defines in Writer due to later association of strings
        // and uno types (see loop at end of this method and definition of SW_PROP_NMID)
        // This entry is for adding that properties to OLE/EmbeddedObject import/export
        FILL_PROPERTIES_SW

        { u"", 0, css::uno::Type(), 0, 0 }
    };

    return aEmbeddedPropertyMap_Impl;
}

const SfxItemPropertyMapEntry*  SwUnoPropertyMapProvider::GetIndexMarkPropertyMap()
{
    static SfxItemPropertyMapEntry const aIdxMarkMap_Impl[] =
    {
        { UNO_NAME_ALTERNATIVE_TEXT, WID_ALT_TEXT,        cppu::UnoType<OUString>::get()  ,      PROPERTY_NONE,     0},
        { UNO_NAME_PRIMARY_KEY, WID_PRIMARY_KEY,  cppu::UnoType<OUString>::get()  ,      PROPERTY_NONE,     0},
        { UNO_NAME_SECONDARY_KEY, WID_SECONDARY_KEY,  cppu::UnoType<OUString>::get()  ,      PROPERTY_NONE,     0},
        { UNO_NAME_TEXT_READING, WID_TEXT_READING, cppu::UnoType<OUString>::get(), PROPERTY_NONE,     0},
        { UNO_NAME_PRIMARY_KEY_READING, WID_PRIMARY_KEY_READING, cppu::UnoType<OUString>::get(), PROPERTY_NONE,     0},
        { UNO_NAME_SECONDARY_KEY_READING, WID_SECONDARY_KEY_READING, cppu::UnoType<OUString>::get(), PROPERTY_NONE,     0},
        { UNO_NAME_IS_MAIN_ENTRY, WID_MAIN_ENTRY,     cppu::UnoType<bool>::get()  ,       PROPERTY_NONE,     0},
        COMMON_TEXT_CONTENT_PROPERTIES
        { u"", 0, css::uno::Type(), 0, 0 }
    };

    return aIdxMarkMap_Impl;
}

const SfxItemPropertyMapEntry*  SwUnoPropertyMapProvider::GetContentMarkPropertyMap()
{
    static SfxItemPropertyMapEntry const aContentMarkMap_Impl[] =
    {
        { UNO_NAME_ALTERNATIVE_TEXT, WID_ALT_TEXT,        cppu::UnoType<OUString>::get()  ,      PROPERTY_NONE,     0},
        { UNO_NAME_LEVEL, WID_LEVEL        ,  cppu::UnoType<sal_Int16>::get()  ,         PROPERTY_NONE,     0},
        COMMON_TEXT_CONTENT_PROPERTIES
        { u"", 0, css::uno::Type(), 0, 0 }
    };

    return aContentMarkMap_Impl;
}

const SfxItemPropertyMapEntry*  SwUnoPropertyMapProvider::GetUserMarkPropertyMap()
{
    static SfxItemPropertyMapEntry const aUserMarkMap_Impl[] =
    {
        { UNO_NAME_ALTERNATIVE_TEXT, WID_ALT_TEXT,        cppu::UnoType<OUString>::get()  ,      PROPERTY_NONE,     0},
        { UNO_NAME_LEVEL,   WID_LEVEL        ,    cppu::UnoType<sal_Int16>::get()  ,         PROPERTY_NONE,     0},
        { UNO_NAME_USER_INDEX_NAME, WID_USER_IDX_NAME,    cppu::UnoType<OUString>::get()  ,      PROPERTY_NONE,     0},
        COMMON_TEXT_CONTENT_PROPERTIES
        { u"", 0, css::uno::Type(), 0, 0 }
    };

    return aUserMarkMap_Impl;
}

const SfxItemPropertyMapEntry*  SwUnoPropertyMapProvider::GetTextTableCursorPropertyMap()
{
    // The PropertySet corresponds to the Range without Chart properties
    static SfxItemPropertyMapEntry const aTableCursorPropertyMap_Impl [] =
    {
        COMMON_CRSR_PARA_PROPERTIES_WITHOUT_FN_01
        TABSTOPS_MAP_ENTRY

        // attributes from PROPERTY_MAP_TABLE_CELL:
        { UNO_NAME_BACK_COLOR, FN_UNO_TABLE_CELL_BACKGROUND,    cppu::UnoType<sal_Int32>::get(),           PROPERTY_NONE , MID_BACK_COLOR       },
        { UNO_NAME_BACK_GRAPHIC_URL, RES_BACKGROUND,      cppu::UnoType<OUString>::get(), PROPERTY_NONE ,MID_GRAPHIC_URL    },
        { UNO_NAME_BACK_GRAPHIC, RES_BACKGROUND, cppu::UnoType<graphic::XGraphic>::get(), PROPERTY_NONE, MID_GRAPHIC },
        { UNO_NAME_BACK_GRAPHIC_FILTER, RES_BACKGROUND,       cppu::UnoType<OUString>::get(), PROPERTY_NONE ,MID_GRAPHIC_FILTER    },
        { UNO_NAME_BACK_GRAPHIC_LOCATION, FN_UNO_TABLE_CELL_BACKGROUND,         cppu::UnoType<css::style::GraphicLocation>::get(), PROPERTY_NONE ,MID_GRAPHIC_POSITION},
        { UNO_NAME_NUMBER_FORMAT, RES_BOXATR_FORMAT,  cppu::UnoType<sal_Int32>::get(),           PropertyAttribute::MAYBEVOID ,0             },
        { UNO_NAME_BACK_TRANSPARENT, FN_UNO_TABLE_CELL_BACKGROUND,  cppu::UnoType<bool>::get(),         PROPERTY_NONE , MID_GRAPHIC_TRANSPARENT      },
        { UNO_NAME_USER_DEFINED_ATTRIBUTES, RES_UNKNOWNATR_CONTAINER, cppu::UnoType<css::container::XNameContainer>::get(), PropertyAttribute::MAYBEVOID, 0 },
        { UNO_NAME_TEXT_SECTION, FN_UNO_TEXT_SECTION, cppu::UnoType<css::text::XTextSection>::get(),  PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY ,0 },
        { UNO_NAME_IS_PROTECTED, RES_PROTECT,            cppu::UnoType<bool>::get(), 0, MID_PROTECT_CONTENT},
        { UNO_NAME_VERT_ORIENT, RES_VERT_ORIENT,      cppu::UnoType<sal_Int16>::get(),           PROPERTY_NONE ,MID_VERTORIENT_ORIENT    },
        { u"", 0, css::uno::Type(), 0, 0 }
    };

    return aTableCursorPropertyMap_Impl;
}

const SfxItemPropertyMapEntry*  SwUnoPropertyMapProvider::GetBookmarkPropertyMap()
{
    static SfxItemPropertyMapEntry const aBookmarkPropertyMap_Impl [] =
    {
        { UNO_LINK_DISPLAY_NAME, FN_PARAM_LINK_DISPLAY_NAME,  cppu::UnoType<OUString>::get(), PropertyAttribute::READONLY, 0xbf},
        COMMON_TEXT_CONTENT_PROPERTIES
        { UNO_NAME_BOOKMARK_HIDDEN, FN_BOOKMARK_HIDDEN,  cppu::UnoType<bool>::get(), PROPERTY_NONE, 0 },
        { UNO_NAME_BOOKMARK_CONDITION, FN_BOOKMARK_CONDITION,  cppu::UnoType<OUString>::get(), PROPERTY_NONE, 0 },
        { u"", 0, css::uno::Type(), 0, 0 }
    };

    return aBookmarkPropertyMap_Impl;
}

const SfxItemPropertyMapEntry*  SwUnoPropertyMapProvider::GetParagraphExtensionsPropertyMap()
{
    static SfxItemPropertyMapEntry const aParagraphExtensionsMap_Impl[] =
    {
        COMMON_TEXT_CONTENT_PROPERTIES
        { u"", 0, css::uno::Type(), 0, 0 }
    };

    return aParagraphExtensionsMap_Impl;
}

const SfxItemPropertyMapEntry*  SwUnoPropertyMapProvider::GetTextPortionExtensionPropertyMap()
{
    static SfxItemPropertyMapEntry const aTextPortionExtensionMap_Impl[] =
    {
        COMPLETE_TEXT_CURSOR_MAP
        { UNO_NAME_BOOKMARK, FN_UNO_BOOKMARK, cppu::UnoType<css::text::XTextContent>::get(),   PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY ,0 },
        { UNO_NAME_CONTROL_CHARACTER, FN_UNO_CONTROL_CHARACTER, cppu::UnoType<sal_Int16>::get(),                 PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY, MID_HYPHEN_MIN_LEAD   },
        { UNO_NAME_IS_COLLAPSED, FN_UNO_IS_COLLAPSED, cppu::UnoType<bool>::get(),                             PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY, 0 },
        { UNO_NAME_IS_START, FN_UNO_IS_START, cppu::UnoType<bool>::get(),                             PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY, 0 },
        //REDLINE_PROPERTIES
        { UNO_NAME_TEXT_PORTION_TYPE, FN_UNO_TEXT_PORTION_TYPE, cppu::UnoType<OUString>::get(),                        PropertyAttribute::READONLY, 0},
        { UNO_NAME_META, FN_UNO_META, cppu::UnoType<css::text::XTextContent>::get(), PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY, 0 },
        { u"", 0, css::uno::Type(), 0, 0 }
    };

    return aTextPortionExtensionMap_Impl;
}

const SfxItemPropertyMapEntry*  SwUnoPropertyMapProvider::GetFootnotePropertyMap()
{
    static SfxItemPropertyMapEntry const aFootnoteMap_Impl[] =
    {
        { UNO_NAME_REFERENCE_ID, 0, cppu::UnoType<sal_Int16>::get(),PropertyAttribute::READONLY|PropertyAttribute::MAYBEVOID, 0},
        COMMON_TEXT_CONTENT_PROPERTIES
        REDLINE_NODE_PROPERTIES
        { u"", 0, css::uno::Type(), 0, 0 }
    };

    return aFootnoteMap_Impl;
}

const SfxItemPropertyMapEntry*  SwUnoPropertyMapProvider::GetRedlinePropertyMap()
{
    static SfxItemPropertyMapEntry const aRedlineMap_Impl[] =
    {
        REDLINE_PROPERTIES(PropertyAttribute::READONLY)
        REDLINE_NODE_PROPERTIES
        { UNO_NAME_REDLINE_START, 0, cppu::UnoType<css::uno::XInterface>::get(),  PropertyAttribute::READONLY,    0},
        { UNO_NAME_REDLINE_END, 0, cppu::UnoType<css::uno::XInterface>::get(),    PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY,   0},
        { u"", 0, css::uno::Type(), 0, 0 }
    };

    return aRedlineMap_Impl;
}

SfxItemPropertyMapEntry*  SwUnoPropertyMapProvider::GetTextDefaultPropertyMap()
{
    static SfxItemPropertyMapEntry aTextDefaultMap_Impl[] =
    {
        { UNO_NAME_TAB_STOP_DISTANCE, RES_PARATR_TABSTOP,     cppu::UnoType<sal_Int32>::get(),   PROPERTY_NONE, MID_STD_TAB | CONVERT_TWIPS},
        COMMON_CRSR_PARA_PROPERTIES_WITHOUT_FN
        COMMON_HYPERLINK_PROPERTIES
        { UNO_NAME_CHAR_STYLE_NAME, RES_TXTATR_CHARFMT,     cppu::UnoType<OUString>::get(),  PropertyAttribute::MAYBEVOID,     0},
        { UNO_NAME_IS_SPLIT_ALLOWED, RES_ROW_SPLIT,       cppu::UnoType<bool>::get()  , PropertyAttribute::MAYBEVOID, 0},
        // #i29550#
        { UNO_NAME_COLLAPSING_BORDERS, RES_COLLAPSING_BORDERS, cppu::UnoType<bool>::get(), PROPERTY_NONE, 0},

        //text grid enhancement for better CJK support.  2007-04-01
        //just export the default page mode property, other properties are not handled in this version
        { UNO_NAME_GRID_STANDARD_PAGE_MODE, RES_TEXTGRID, cppu::UnoType<bool>::get(), PROPERTY_NONE, MID_GRID_STANDARD_MODE},
        { u"", 0, css::uno::Type(), 0, 0 }
    };

    return aTextDefaultMap_Impl;
}

const SfxItemPropertyMapEntry*  SwUnoPropertyMapProvider::GetRedlinePortionPropertyMap()
{
    static SfxItemPropertyMapEntry const aRedlinePortionMap_Impl[] =
    {
        COMPLETE_TEXT_CURSOR_MAP
        { UNO_NAME_BOOKMARK, FN_UNO_BOOKMARK, cppu::UnoType<css::text::XTextContent>::get(),   PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY ,0 },
        { UNO_NAME_CONTROL_CHARACTER, FN_UNO_CONTROL_CHARACTER, cppu::UnoType<sal_Int16>::get(),                 PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY, MID_HYPHEN_MIN_LEAD   },
        { UNO_NAME_IS_COLLAPSED, FN_UNO_IS_COLLAPSED, cppu::UnoType<bool>::get(),                             PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY, 0 },
        { UNO_NAME_IS_START, FN_UNO_IS_START, cppu::UnoType<bool>::get(),                             PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY, 0 },
        REDLINE_PROPERTIES(0)
        { UNO_NAME_TEXT_PORTION_TYPE, FN_UNO_TEXT_PORTION_TYPE, cppu::UnoType<OUString>::get(),                        PropertyAttribute::READONLY, 0},
        { u"", 0, css::uno::Type(), 0, 0 }
    };

    return aRedlinePortionMap_Impl;
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
            case PROPERTY_MAP_FIELDMARK:
            {
                static SfxItemPropertySet aPROPERTY_MAP_FIELDMARK(pEntries);
                m_aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_FIELDMARK;
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
            case PROPERTY_MAP_TABLE_STYLE:
            {
                static SfxItemPropertySet aPROPERTY_MAP_TABLE_STYLE(pEntries);
                m_aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_TABLE_STYLE;
            }
            break;
            case PROPERTY_MAP_CELL_STYLE:
            {
                static SfxItemPropertySet aPROPERTY_MAP_CELL_STYLE(pEntries);
                m_aPropertySetArr[nPropertyId] = &aPROPERTY_MAP_CELL_STYLE;
            }
            break;
        }
    }
    return m_aPropertySetArr[nPropertyId];
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
