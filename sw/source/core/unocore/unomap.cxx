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

#include <com/sun/star/awt/FontSlant.hpp>
#include <com/sun/star/awt/Point.hpp>
#include <com/sun/star/awt/XBitmap.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/PropertyValues.hpp>
#include <com/sun/star/container/XIndexReplace.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/i18n/XForbiddenCharacters.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/style/GraphicLocation.hpp>
#include <com/sun/star/style/VerticalAlignment.hpp>
#include <com/sun/star/table/BorderLine.hpp>
#include <com/sun/star/text/PageNumberType.hpp>
#include <com/sun/star/text/TableColumnSeparator.hpp>
#include <com/sun/star/text/TextContentAnchorType.hpp>
#include <com/sun/star/text/WrapTextMode.hpp>
#include <com/sun/star/text/XDependentTextField.hpp>
#include <com/sun/star/text/XDocumentIndexMark.hpp>
#include <com/sun/star/text/XTextColumns.hpp>
#include <com/sun/star/text/XTextFrame.hpp>
#include <com/sun/star/text/XTextSection.hpp>
#include <com/sun/star/util/Date.hpp>
#include <com/sun/star/util/DateTime.hpp>
#include <com/sun/star/script/XLibraryContainer.hpp>
#include <com/sun/star/drawing/HomogenMatrix3.hpp>
#include <osl/diagnose.h>
#include <unomap.hxx>
#include <unoprnms.hxx>
#include <unomid.h>
#include <cmdid.h>
#include <unofldmid.h>
#include <editeng/memberids.h>
#include <editeng/unoprnms.hxx>
#include <svl/itemprop.hxx>
#include "unomapproperties.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;

#define COMMON_FLDTYP_PROPERTIES \
                    { UNO_NAME_IS_FIELD_USED,      FIELD_PROP_IS_FIELD_USED,      cppu::UnoType<float>::get(), PropertyAttribute::READONLY, 0},\
                    { UNO_NAME_IS_FIELD_DISPLAYED, FIELD_PROP_IS_FIELD_DISPLAYED, cppu::UnoType<sal_Int16>::get(), PropertyAttribute::READONLY, 0},\

const SfxItemPropertyMapEntry* SwUnoPropertyMapProvider::GetPropertyMapEntries(sal_uInt16 nPropertyId)
{
    OSL_ENSURE(nPropertyId < PROPERTY_MAP_END, "Id ?" );
    if( !m_aMapEntriesArr[ nPropertyId ] )
    {
        switch(nPropertyId)
        {
            case PROPERTY_MAP_TEXT_CURSOR:
            {
                m_aMapEntriesArr[nPropertyId] = GetTextCursorPropertyMap();
            }
            break;
            case PROPERTY_MAP_ACCESSIBILITY_TEXT_ATTRIBUTE:
            {
                m_aMapEntriesArr[nPropertyId] = GetAccessibilityTextAttrPropertyMap();
            }
            break;
            case PROPERTY_MAP_PARAGRAPH:
            {
                m_aMapEntriesArr[nPropertyId] = GetParagraphPropertyMap();
            }
            break;
            case PROPERTY_MAP_PARA_AUTO_STYLE :
            {
                m_aMapEntriesArr[nPropertyId] = GetAutoParaStylePropertyMap();
            }
            break;
            case PROPERTY_MAP_CHAR_STYLE :
            {
                m_aMapEntriesArr[nPropertyId] = GetCharStylePropertyMap();
            }
            break;
            case PROPERTY_MAP_CHAR_AUTO_STYLE :
            {
                m_aMapEntriesArr[nPropertyId] = GetAutoCharStylePropertyMap();
            }
            break;
            case PROPERTY_MAP_RUBY_AUTO_STYLE :
            {
                static SfxItemPropertyMapEntry const aAutoRubyStyleMap [] =
                {
                    { UNO_NAME_RUBY_ADJUST, RES_TXTATR_CJK_RUBY,  cppu::UnoType<sal_Int16>::get(),   PropertyAttribute::MAYBEVOID,          MID_RUBY_ADJUST },
                    { UNO_NAME_RUBY_IS_ABOVE, RES_TXTATR_CJK_RUBY,    cppu::UnoType<bool>::get(),  PropertyAttribute::MAYBEVOID,     MID_RUBY_ABOVE },
                    { UNO_NAME_RUBY_POSITION, RES_TXTATR_CJK_RUBY,    cppu::UnoType<sal_Int16>::get(),  PropertyAttribute::MAYBEVOID,     MID_RUBY_POSITION },
                    { u"", 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aAutoRubyStyleMap;
            }
            break;
            case PROPERTY_MAP_PARA_STYLE :
            {
                m_aMapEntriesArr[nPropertyId] = GetParaStylePropertyMap();
            }
            break;
            case PROPERTY_MAP_CONDITIONAL_PARA_STYLE :
            {
                m_aMapEntriesArr[nPropertyId] = GetConditionalParaStylePropertyMap();
            }
            break;
            case PROPERTY_MAP_FRAME_STYLE:
            {
                m_aMapEntriesArr[nPropertyId] = GetFrameStylePropertyMap();
            }
            break;
            case PROPERTY_MAP_PAGE_STYLE :
            {
                m_aMapEntriesArr[nPropertyId] = GetPageStylePropertyMap();
            }
            break;
            case PROPERTY_MAP_NUM_STYLE  :
            {
                static SfxItemPropertyMapEntry const aNumStyleMap        [] =
                {
                    { UNO_NAME_NUMBERING_RULES, FN_UNO_NUM_RULES, cppu::UnoType<css::container::XIndexReplace>::get(), PROPERTY_NONE, CONVERT_TWIPS},
                    { UNO_NAME_IS_PHYSICAL, FN_UNO_IS_PHYSICAL,     cppu::UnoType<bool>::get(), PropertyAttribute::READONLY, 0},
                    { UNO_NAME_DISPLAY_NAME, FN_UNO_DISPLAY_NAME, cppu::UnoType<OUString>::get(), PropertyAttribute::READONLY, 0},
                    { UNO_NAME_HIDDEN, FN_UNO_HIDDEN,     cppu::UnoType<bool>::get(), PROPERTY_NONE, 0},
                    { UNO_NAME_STYLE_INTEROP_GRAB_BAG, FN_UNO_STYLE_INTEROP_GRAB_BAG, cppu::UnoType< cppu::UnoSequenceType<css::beans::PropertyValue> >::get(), PROPERTY_NONE, 0},
                    { u"", 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aNumStyleMap;
            }
            break;
            case PROPERTY_MAP_TEXT_TABLE :
            {
                m_aMapEntriesArr[nPropertyId] = GetTablePropertyMap();
            }
            break;
            case PROPERTY_MAP_TABLE_CELL :
            {
                static SfxItemPropertyMapEntry const aCellMap_Impl[] =
                {
                    { UNO_NAME_BACK_COLOR, RES_BACKGROUND,    cppu::UnoType<sal_Int32>::get(),           PROPERTY_NONE , MID_BACK_COLOR       },
                    { UNO_NAME_BACK_GRAPHIC_URL, RES_BACKGROUND,      cppu::UnoType<OUString>::get(), PROPERTY_NONE ,MID_GRAPHIC_URL    },
                    { UNO_NAME_BACK_GRAPHIC, RES_BACKGROUND,      cppu::UnoType<graphic::XGraphic>::get(), PROPERTY_NONE, MID_GRAPHIC    },
                    { UNO_NAME_BACK_GRAPHIC_FILTER, RES_BACKGROUND,       cppu::UnoType<OUString>::get(), PROPERTY_NONE ,MID_GRAPHIC_FILTER    },
                    { UNO_NAME_BACK_GRAPHIC_LOCATION, RES_BACKGROUND,         cppu::UnoType<css::style::GraphicLocation>::get(), PROPERTY_NONE ,MID_GRAPHIC_POSITION},
                    { UNO_NAME_BACK_TRANSPARENT, RES_BACKGROUND,  cppu::UnoType<bool>::get(),         PROPERTY_NONE , MID_GRAPHIC_TRANSPARENT      },
                    { UNO_NAME_NUMBER_FORMAT, RES_BOXATR_FORMAT,  cppu::UnoType<sal_Int32>::get(),           PropertyAttribute::MAYBEVOID ,0             },
                    { UNO_NAME_LEFT_BORDER, RES_BOX,    cppu::UnoType<css::table::BorderLine>::get(),    0, LEFT_BORDER  |CONVERT_TWIPS },
                    { UNO_NAME_RIGHT_BORDER, RES_BOX,    cppu::UnoType<css::table::BorderLine>::get(),   0, RIGHT_BORDER |CONVERT_TWIPS },
                    { UNO_NAME_TOP_BORDER, RES_BOX,    cppu::UnoType<css::table::BorderLine>::get(), 0, TOP_BORDER   |CONVERT_TWIPS },
                    { UNO_NAME_BOTTOM_BORDER, RES_BOX,    cppu::UnoType<css::table::BorderLine>::get(),  0, BOTTOM_BORDER|CONVERT_TWIPS },
                    { UNO_NAME_BORDER_DISTANCE, RES_BOX,    cppu::UnoType<sal_Int32>::get(), 0, BORDER_DISTANCE|CONVERT_TWIPS },
                    { UNO_NAME_LEFT_BORDER_DISTANCE, RES_BOX,             cppu::UnoType<sal_Int32>::get(),   0, LEFT_BORDER_DISTANCE  |CONVERT_TWIPS },
                    { UNO_NAME_RIGHT_BORDER_DISTANCE, RES_BOX,                cppu::UnoType<sal_Int32>::get(),   0, RIGHT_BORDER_DISTANCE |CONVERT_TWIPS },
                    { UNO_NAME_TOP_BORDER_DISTANCE, RES_BOX,              cppu::UnoType<sal_Int32>::get(),   0, TOP_BORDER_DISTANCE   |CONVERT_TWIPS },
                    { UNO_NAME_BOTTOM_BORDER_DISTANCE, RES_BOX,               cppu::UnoType<sal_Int32>::get(),   0, BOTTOM_BORDER_DISTANCE|CONVERT_TWIPS },
                    { UNO_NAME_USER_DEFINED_ATTRIBUTES, RES_UNKNOWNATR_CONTAINER, cppu::UnoType<css::container::XNameContainer>::get(), PropertyAttribute::MAYBEVOID, 0 },
                    { UNO_NAME_TEXT_SECTION, FN_UNO_TEXT_SECTION, cppu::UnoType<css::text::XTextSection>::get(),  PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY ,0 },
                    { UNO_NAME_IS_PROTECTED, RES_PROTECT,            cppu::UnoType<bool>::get(), 0, MID_PROTECT_CONTENT},
                    { UNO_NAME_CELL_NAME, FN_UNO_CELL_NAME,            cppu::UnoType<OUString>::get(), PropertyAttribute::READONLY,0},
                    { UNO_NAME_VERT_ORIENT, RES_VERT_ORIENT,      cppu::UnoType<sal_Int16>::get(),           PROPERTY_NONE ,MID_VERTORIENT_ORIENT    },
                    { UNO_NAME_WRITING_MODE, RES_FRAMEDIR, cppu::UnoType<sal_Int16>::get(), PROPERTY_NONE, 0 },
                    { UNO_NAME_ROW_SPAN,     FN_UNO_CELL_ROW_SPAN, cppu::UnoType<sal_Int32>::get(),  0, 0 },
                    { UNO_NAME_CELL_INTEROP_GRAB_BAG, RES_FRMATR_GRABBAG, cppu::UnoType< cppu::UnoSequenceType<css::beans::PropertyValue> >::get(), PROPERTY_NONE, 0 },
                    { UNO_NAME_PARENT_TEXT, FN_UNO_PARENT_TEXT, cppu::UnoType<text::XText>::get(), PropertyAttribute::MAYBEVOID | PropertyAttribute::READONLY, 0 },
                    REDLINE_NODE_PROPERTIES
                    { u"", 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aCellMap_Impl;
            }
            break;
            case PROPERTY_MAP_TABLE_RANGE:
            {
                m_aMapEntriesArr[nPropertyId] = GetRangePropertyMap();
            }
            break;
            case PROPERTY_MAP_SECTION:
            {
                m_aMapEntriesArr[nPropertyId] = GetSectionPropertyMap();
            }
            break;
            case PROPERTY_MAP_TEXT_SEARCH:
            {
                static SfxItemPropertyMapEntry const aSearchPropertyMap_Impl[] =
                {
                    { UNO_NAME_SEARCH_ALL, WID_SEARCH_ALL,        cppu::UnoType<bool>::get()  ,       PROPERTY_NONE,     0},
                    { UNO_NAME_SEARCH_BACKWARDS, WID_BACKWARDS,           cppu::UnoType<bool>::get()  ,       PROPERTY_NONE,     0},
                    { UNO_NAME_SEARCH_CASE_SENSITIVE, WID_CASE_SENSITIVE,     cppu::UnoType<bool>::get()  ,       PROPERTY_NONE,     0},
                    { UNO_NAME_SEARCH_REGULAR_EXPRESSION, WID_REGULAR_EXPRESSION, cppu::UnoType<bool>::get()  ,       PROPERTY_NONE,     0},
                    { UNO_NAME_SEARCH_SIMILARITY, WID_SIMILARITY,         cppu::UnoType<bool>::get()  ,       PROPERTY_NONE,     0},
                    { UNO_NAME_SEARCH_SIMILARITY_ADD, WID_SIMILARITY_ADD,     cppu::UnoType<sal_Int16>::get()  ,     PROPERTY_NONE,     0},
                    { UNO_NAME_SEARCH_SIMILARITY_EXCHANGE, WID_SIMILARITY_EXCHANGE,cppu::UnoType<sal_Int16>::get()  ,    PROPERTY_NONE,     0},
                    { UNO_NAME_SEARCH_SIMILARITY_RELAX, WID_SIMILARITY_RELAX,     cppu::UnoType<bool>::get()  ,       PROPERTY_NONE,     0},
                    { UNO_NAME_SEARCH_SIMILARITY_REMOVE, WID_SIMILARITY_REMOVE,   cppu::UnoType<sal_Int16>::get()  ,     PROPERTY_NONE,     0},
                    { UNO_NAME_SEARCH_STYLES, WID_STYLES,             cppu::UnoType<bool>::get()  ,       PROPERTY_NONE,     0},
                    { UNO_NAME_SEARCH_WORDS, WID_WORDS,               cppu::UnoType<bool>::get()  ,       PROPERTY_NONE,     0},
                    { u"", 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aSearchPropertyMap_Impl;
            }
            break;
            case PROPERTY_MAP_TEXT_FRAME:
            {
                m_aMapEntriesArr[nPropertyId] = GetFramePropertyMap();
            }
            break;
            case PROPERTY_MAP_TEXT_GRAPHIC:
            {
                m_aMapEntriesArr[nPropertyId] = GetGraphicPropertyMap();
            }
            break;
            case PROPERTY_MAP_EMBEDDED_OBJECT:
            {
                m_aMapEntriesArr[nPropertyId] = GetEmbeddedPropertyMap();
            }
            break;
            case PROPERTY_MAP_TEXT_SHAPE:
            {
                static SfxItemPropertyMapEntry const aShapeMap_Impl[] =
                {
                    { UNO_NAME_ANCHOR_PAGE_NO, RES_ANCHOR,            cppu::UnoType<sal_Int16>::get(),           PROPERTY_NONE|PropertyAttribute::MAYBEVOID, MID_ANCHOR_PAGENUM      },
                    { UNO_NAME_ANCHOR_TYPE, RES_ANCHOR,           cppu::UnoType<css::text::TextContentAnchorType>::get(),            PROPERTY_NONE|PropertyAttribute::MAYBEVOID, MID_ANCHOR_ANCHORTYPE},
                    { UNO_NAME_ANCHOR_FRAME, RES_ANCHOR,             cppu::UnoType<css::text::XTextFrame>::get(),    PropertyAttribute::MAYBEVOID, MID_ANCHOR_ANCHORFRAME},
                    { UNO_NAME_HORI_ORIENT, RES_HORI_ORIENT,        cppu::UnoType<sal_Int16>::get(),   PROPERTY_NONE|PropertyAttribute::MAYBEVOID ,MID_HORIORIENT_ORIENT },
                    { UNO_NAME_HORI_ORIENT_POSITION, RES_HORI_ORIENT,     cppu::UnoType<sal_Int32>::get(),           PROPERTY_NONE|PropertyAttribute::MAYBEVOID ,MID_HORIORIENT_POSITION|CONVERT_TWIPS   },
                    { UNO_NAME_HORI_ORIENT_RELATION, RES_HORI_ORIENT,     cppu::UnoType<sal_Int16>::get(),           PROPERTY_NONE|PropertyAttribute::MAYBEVOID ,MID_HORIORIENT_RELATION },
                    { UNO_NAME_LEFT_MARGIN, RES_LR_SPACE,             cppu::UnoType<sal_Int32>::get(), PROPERTY_NONE|PropertyAttribute::MAYBEVOID, MID_L_MARGIN|CONVERT_TWIPS},
                    { UNO_NAME_RIGHT_MARGIN, RES_LR_SPACE,            cppu::UnoType<sal_Int32>::get(), PROPERTY_NONE|PropertyAttribute::MAYBEVOID, MID_R_MARGIN|CONVERT_TWIPS},
                    { UNO_NAME_SURROUND, RES_SURROUND,          cppu::UnoType<css::text::WrapTextMode>::get(),    PROPERTY_NONE|PropertyAttribute::MAYBEVOID, MID_SURROUND_SURROUNDTYPE },
                    { UNO_NAME_TEXT_WRAP, RES_SURROUND,           cppu::UnoType<css::text::WrapTextMode>::get(),             PROPERTY_NONE, MID_SURROUND_SURROUNDTYPE    },
                    { UNO_NAME_SURROUND_ANCHORONLY, RES_SURROUND,             cppu::UnoType<bool>::get(),             PROPERTY_NONE|PropertyAttribute::MAYBEVOID, MID_SURROUND_ANCHORONLY     },
                    { UNO_NAME_SURROUND_CONTOUR, RES_SURROUND,            cppu::UnoType<bool>::get(),             PROPERTY_NONE, MID_SURROUND_CONTOUR         },
                    { UNO_NAME_CONTOUR_OUTSIDE, RES_SURROUND,             cppu::UnoType<bool>::get(),             PROPERTY_NONE, MID_SURROUND_CONTOUROUTSIDE  },
                    { UNO_NAME_TOP_MARGIN, RES_UL_SPACE,          cppu::UnoType<sal_Int32>::get(), PROPERTY_NONE, MID_UP_MARGIN|CONVERT_TWIPS},
                    { UNO_NAME_BOTTOM_MARGIN, RES_UL_SPACE,           cppu::UnoType<sal_Int32>::get(), PROPERTY_NONE, MID_LO_MARGIN|CONVERT_TWIPS},
                    { UNO_NAME_VERT_ORIENT, RES_VERT_ORIENT,      cppu::UnoType<sal_Int16>::get(),           PROPERTY_NONE|PropertyAttribute::MAYBEVOID ,MID_VERTORIENT_ORIENT   },
                    { UNO_NAME_VERT_ORIENT_POSITION, RES_VERT_ORIENT,     cppu::UnoType<sal_Int32>::get(),           PROPERTY_NONE|PropertyAttribute::MAYBEVOID ,MID_VERTORIENT_POSITION|CONVERT_TWIPS   },
                    { UNO_NAME_VERT_ORIENT_RELATION, RES_VERT_ORIENT,     cppu::UnoType<sal_Int16>::get(),           PROPERTY_NONE|PropertyAttribute::MAYBEVOID ,MID_VERTORIENT_RELATION },
                    { UNO_NAME_TEXT_RANGE, FN_TEXT_RANGE,         cppu::UnoType<css::text::XTextRange>::get(),         PROPERTY_NONE, 0},
                    { UNO_NAME_OPAQUE, RES_OPAQUE,             cppu::UnoType<bool>::get(),            PROPERTY_NONE, 0},
                    { UNO_NAME_ANCHOR_POSITION, FN_ANCHOR_POSITION,    cppu::UnoType<css::awt::Point>::get(),    PropertyAttribute::READONLY, 0},
                    // #i26791#
                    { UNO_NAME_IS_FOLLOWING_TEXT_FLOW, RES_FOLLOW_TEXT_FLOW,     cppu::UnoType<bool>::get(), PROPERTY_NONE, MID_FOLLOW_TEXT_FLOW},
                    // #i28701#
                    { UNO_NAME_WRAP_INFLUENCE_ON_POSITION, RES_WRAP_INFLUENCE_ON_OBJPOS, cppu::UnoType<sal_Int8>::get(), PROPERTY_NONE, MID_WRAP_INFLUENCE},
                    { UNO_NAME_ALLOW_OVERLAP, RES_WRAP_INFLUENCE_ON_OBJPOS, cppu::UnoType<bool>::get(), PROPERTY_NONE, MID_ALLOW_OVERLAP},
                    // #i28749#
                    { UNO_NAME_TRANSFORMATION_IN_HORI_L2R,
                                    FN_SHAPE_TRANSFORMATION_IN_HORI_L2R,
                                    cppu::UnoType<css::drawing::HomogenMatrix3>::get(),
                                    PropertyAttribute::READONLY, 0},
                    { UNO_NAME_POSITION_LAYOUT_DIR,
                                    FN_SHAPE_POSITION_LAYOUT_DIR,
                                    cppu::UnoType<sal_Int16>::get(),
                                    PROPERTY_NONE, 0},
                    // #i36248#
                    { UNO_NAME_STARTPOSITION_IN_HORI_L2R,
                                    FN_SHAPE_STARTPOSITION_IN_HORI_L2R,
                                    cppu::UnoType<css::awt::Point>::get(),
                                    PropertyAttribute::READONLY, 0},
                    { UNO_NAME_ENDPOSITION_IN_HORI_L2R,
                                    FN_SHAPE_ENDPOSITION_IN_HORI_L2R,
                                    cppu::UnoType<css::awt::Point>::get(),
                                    PropertyAttribute::READONLY, 0},
                    // #i71182#
                    // missing map entry for property <PageToggle>
                    { UNO_NAME_PAGE_TOGGLE, RES_HORI_ORIENT,      cppu::UnoType<bool>::get(),         PROPERTY_NONE ,MID_HORIORIENT_PAGETOGGLE },
                    { UNO_NAME_RELATIVE_HEIGHT, RES_FRM_SIZE,     cppu::UnoType<sal_Int16>::get()  ,         PROPERTY_NONE, MID_FRMSIZE_REL_HEIGHT },
                    { UNO_NAME_RELATIVE_HEIGHT_RELATION, RES_FRM_SIZE, cppu::UnoType<sal_Int16>::get(),      PROPERTY_NONE, MID_FRMSIZE_REL_HEIGHT_RELATION },
                    { UNO_NAME_RELATIVE_WIDTH, RES_FRM_SIZE,      cppu::UnoType<sal_Int16>::get()  ,         PROPERTY_NONE, MID_FRMSIZE_REL_WIDTH  },
                    { UNO_NAME_RELATIVE_WIDTH_RELATION, RES_FRM_SIZE, cppu::UnoType<sal_Int16>::get(),       PROPERTY_NONE, MID_FRMSIZE_REL_WIDTH_RELATION },
                    { UNO_NAME_TEXT_BOX, FN_TEXT_BOX, cppu::UnoType<bool>::get(), PROPERTY_NONE, MID_TEXT_BOX},
                    { UNO_NAME_TEXT_BOX_CONTENT, FN_TEXT_BOX, cppu::UnoType<text::XTextFrame>::get(), PROPERTY_NONE, MID_TEXT_BOX_CONTENT},
                    { UNO_NAME_CHAIN_NEXT_NAME, RES_CHAIN,                cppu::UnoType<OUString>::get(),            PropertyAttribute::MAYBEVOID ,MID_CHAIN_NEXTNAME},
                    { UNO_NAME_CHAIN_PREV_NAME, RES_CHAIN,                cppu::UnoType<OUString>::get(),            PropertyAttribute::MAYBEVOID ,MID_CHAIN_PREVNAME},
                    { UNO_NAME_CHAIN_NAME,      RES_CHAIN,                cppu::UnoType<OUString>::get(),            PropertyAttribute::MAYBEVOID ,MID_CHAIN_NAME    },
                    { u"", 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aShapeMap_Impl;
            }
            break;
            case PROPERTY_MAP_INDEX_MARK:
            {
                m_aMapEntriesArr[nPropertyId] = GetIndexMarkPropertyMap();
            }
            break;
            case PROPERTY_MAP_CNTIDX_MARK:
            {
                m_aMapEntriesArr[nPropertyId] = GetContentMarkPropertyMap();
            }
            break;
            case PROPERTY_MAP_USER_MARK:
            {
                m_aMapEntriesArr[nPropertyId] = GetUserMarkPropertyMap();
            }
            break;
            case PROPERTY_MAP_INDEX_IDX:
            {
                static SfxItemPropertyMapEntry const aTOXIndexMap_Impl[] =
                {
                    BASE_INDEX_PROPERTIES_
                    { UNO_NAME_CREATE_FROM_CHAPTER, WID_CREATE_FROM_CHAPTER                 ,  cppu::UnoType<bool>::get()  , PROPERTY_NONE,     0},
                    { UNO_NAME_IS_PROTECTED, WID_PROTECTED                           ,  cppu::UnoType<bool>::get()  , PROPERTY_NONE,     0},
                    { UNO_NAME_USE_ALPHABETICAL_SEPARATORS, WID_USE_ALPHABETICAL_SEPARATORS         ,  cppu::UnoType<bool>::get()  , PROPERTY_NONE,     0},
                    { UNO_NAME_USE_KEY_AS_ENTRY, WID_USE_KEY_AS_ENTRY                    ,  cppu::UnoType<bool>::get()  , PROPERTY_NONE,     0},
                    { UNO_NAME_USE_COMBINED_ENTRIES, WID_USE_COMBINED_ENTRIES                ,  cppu::UnoType<bool>::get()  , PROPERTY_NONE,     0},
                    { UNO_NAME_IS_CASE_SENSITIVE, WID_IS_CASE_SENSITIVE                   ,  cppu::UnoType<bool>::get()  , PROPERTY_NONE,     0},
                    { UNO_NAME_USE_P_P, WID_USE_P_P                             ,  cppu::UnoType<bool>::get()  , PROPERTY_NONE,     0},
                    { UNO_NAME_USE_DASH, WID_USE_DASH                            ,  cppu::UnoType<bool>::get()  , PROPERTY_NONE,     0},
                    { UNO_NAME_USE_UPPER_CASE, WID_USE_UPPER_CASE                      ,  cppu::UnoType<bool>::get()  , PROPERTY_NONE,     0},
                    { UNO_NAME_LEVEL_FORMAT, WID_LEVEL_FORMAT                        ,  cppu::UnoType<css::container::XIndexReplace>::get()  , PROPERTY_NONE,   0},
                    { UNO_NAME_MAIN_ENTRY_CHARACTER_STYLE_NAME, WID_MAIN_ENTRY_CHARACTER_STYLE_NAME     ,  cppu::UnoType<OUString>::get()  , PROPERTY_NONE,     0},
                    { UNO_NAME_TEXT_COLUMNS, RES_COL,                cppu::UnoType<css::text::XTextColumns>::get(),    PROPERTY_NONE, MID_COLUMNS},
                    { UNO_NAME_BACK_GRAPHIC_URL, RES_BACKGROUND,      cppu::UnoType<OUString>::get(), PROPERTY_NONE ,MID_GRAPHIC_URL    },
                    { UNO_NAME_BACK_GRAPHIC, RES_BACKGROUND,      cppu::UnoType<graphic::XGraphic>::get(), PROPERTY_NONE, MID_GRAPHIC    },
                    { UNO_NAME_BACK_GRAPHIC_FILTER, RES_BACKGROUND,       cppu::UnoType<OUString>::get(), PROPERTY_NONE ,MID_GRAPHIC_FILTER    },
                    { UNO_NAME_BACK_GRAPHIC_LOCATION, RES_BACKGROUND,         cppu::UnoType<css::style::GraphicLocation>::get(),          PROPERTY_NONE ,MID_GRAPHIC_POSITION},
                    { UNO_NAME_BACK_COLOR, RES_BACKGROUND,            cppu::UnoType<sal_Int32>::get(),           PROPERTY_NONE ,MID_BACK_COLOR        },
                    { UNO_NAME_BACK_TRANSPARENT, RES_BACKGROUND,      cppu::UnoType<bool>::get(),         PROPERTY_NONE ,MID_GRAPHIC_TRANSPARENT       },
                    { UNO_NAME_PARA_STYLEHEADING,                 WID_PARA_HEAD,          cppu::UnoType<OUString>::get()  , 0,     0},
                    { UNO_NAME_PARA_STYLESEPARATOR,           WID_PARA_SEP,           cppu::UnoType<OUString>::get()  , 0,     0},
                    { UNO_NAME_PARA_STYLELEVEL1,              WID_PARA_LEV1,          cppu::UnoType<OUString>::get()  , 0,     0},
                    { UNO_NAME_PARA_STYLELEVEL2,              WID_PARA_LEV2,          cppu::UnoType<OUString>::get()  , 0,     0},
                    { UNO_NAME_PARA_STYLELEVEL3,              WID_PARA_LEV3,          cppu::UnoType<OUString>::get()  , 0,     0},
                    { UNO_NAME_IS_COMMA_SEPARATED, WID_IS_COMMA_SEPARATED, cppu::UnoType<bool>::get(),            PROPERTY_NONE ,0         },
                    { UNO_NAME_DOCUMENT_INDEX_MARKS, WID_INDEX_MARKS, cppu::UnoType< cppu::UnoSequenceType<css::text::XDocumentIndexMark> >::get(),           PropertyAttribute::READONLY ,0       },
                    { UNO_NAME_IS_RELATIVE_TABSTOPS, WID_IS_RELATIVE_TABSTOPS, cppu::UnoType<bool>::get(), PROPERTY_NONE, 0},
                    { UNO_NAME_LOCALE,            WID_IDX_LOCALE,         cppu::UnoType<css::lang::Locale>::get(), PROPERTY_NONE,     0},
                    { UNO_NAME_SORT_ALGORITHM,    WID_IDX_SORT_ALGORITHM,  cppu::UnoType<OUString>::get(), PROPERTY_NONE,     0},
                    { u"", 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aTOXIndexMap_Impl;
            }
            break;
            case PROPERTY_MAP_INDEX_CNTNT:
            {
                static SfxItemPropertyMapEntry const aTOXContentMap_Impl[] =
                {
                    BASE_INDEX_PROPERTIES_
                    { UNO_NAME_LEVEL, WID_LEVEL                               ,  cppu::UnoType<sal_Int16>::get()  , PROPERTY_NONE,     0},
                    { UNO_NAME_CREATE_FROM_MARKS, WID_CREATE_FROM_MARKS                   ,  cppu::UnoType<bool>::get()  , PROPERTY_NONE,     0},
                    { UNO_NAME_HIDE_TAB_LEADER_AND_PAGE_NUMBERS, WID_HIDE_TABLEADER_PAGENUMBERS                   ,  cppu::UnoType<bool>::get()  , PROPERTY_NONE,     0},
                    { UNO_NAME_TAB_IN_TOC, WID_TAB_IN_TOC, cppu::UnoType<bool>::get(), PROPERTY_NONE, 0},
                    { UNO_NAME_TOC_BOOKMARK, WID_TOC_BOOKMARK, cppu::UnoType<OUString>::get(), PROPERTY_NONE, 0},
                    { UNO_NAME_TOC_NEWLINE, WID_TOC_NEWLINE, cppu::UnoType<bool>::get(), PROPERTY_NONE, 0},
                    { UNO_NAME_TOC_PARAGRAPH_OUTLINE_LEVEL, WID_TOC_PARAGRAPH_OUTLINE_LEVEL, cppu::UnoType<bool>::get(), PROPERTY_NONE, 0},
                    { UNO_NAME_CREATE_FROM_OUTLINE, WID_CREATE_FROM_OUTLINE                 ,  cppu::UnoType<bool>::get()  , PROPERTY_NONE,     0},
                    { UNO_NAME_CREATE_FROM_CHAPTER, WID_CREATE_FROM_CHAPTER                 ,  cppu::UnoType<bool>::get()  , PROPERTY_NONE,     0},
                    { UNO_NAME_IS_PROTECTED, WID_PROTECTED                           ,  cppu::UnoType<bool>::get()  , PROPERTY_NONE,     0},
                    { UNO_NAME_LEVEL_FORMAT, WID_LEVEL_FORMAT                        ,  cppu::UnoType<css::container::XIndexReplace>::get()  , PROPERTY_NONE,     0},
                    { UNO_NAME_LEVEL_PARAGRAPH_STYLES, WID_LEVEL_PARAGRAPH_STYLES              ,  cppu::UnoType<css::container::XIndexReplace>::get()  , PropertyAttribute::READONLY,     0},
                    { UNO_NAME_CREATE_FROM_LEVEL_PARAGRAPH_STYLES, WID_CREATE_FROM_PARAGRAPH_STYLES, cppu::UnoType<bool>::get(), PROPERTY_NONE, 0},
                    { UNO_NAME_TEXT_COLUMNS, RES_COL,                cppu::UnoType<css::text::XTextColumns>::get(),    PROPERTY_NONE, MID_COLUMNS},
                    { UNO_NAME_BACK_GRAPHIC_URL, RES_BACKGROUND,      cppu::UnoType<OUString>::get(), PROPERTY_NONE ,MID_GRAPHIC_URL    },
                    { UNO_NAME_BACK_GRAPHIC, RES_BACKGROUND,      cppu::UnoType<graphic::XGraphic>::get(), PROPERTY_NONE, MID_GRAPHIC    },
                    { UNO_NAME_BACK_GRAPHIC_FILTER, RES_BACKGROUND,       cppu::UnoType<OUString>::get(), PROPERTY_NONE ,MID_GRAPHIC_FILTER    },
                    { UNO_NAME_BACK_GRAPHIC_LOCATION, RES_BACKGROUND,         cppu::UnoType<css::style::GraphicLocation>::get(),          PROPERTY_NONE ,MID_GRAPHIC_POSITION},
                    { UNO_NAME_BACK_COLOR, RES_BACKGROUND,            cppu::UnoType<sal_Int32>::get(),           PROPERTY_NONE ,MID_BACK_COLOR        },
                    { UNO_NAME_BACK_TRANSPARENT, RES_BACKGROUND,      cppu::UnoType<bool>::get(),         PROPERTY_NONE ,MID_GRAPHIC_TRANSPARENT       },
                    { UNO_NAME_PARA_STYLEHEADING,     WID_PARA_HEAD,  cppu::UnoType<OUString>::get()  , 0,     0},
                    { UNO_NAME_PARA_STYLELEVEL1,  WID_PARA_LEV1,  cppu::UnoType<OUString>::get()  , 0,     0},
                    { UNO_NAME_PARA_STYLELEVEL2,  WID_PARA_LEV2,  cppu::UnoType<OUString>::get()  , 0,     0},
                    { UNO_NAME_PARA_STYLELEVEL3,  WID_PARA_LEV3,  cppu::UnoType<OUString>::get()  , 0,     0},
                    { UNO_NAME_PARA_STYLELEVEL4,  WID_PARA_LEV4,  cppu::UnoType<OUString>::get()  , 0,     0},
                    { UNO_NAME_PARA_STYLELEVEL5,  WID_PARA_LEV5,  cppu::UnoType<OUString>::get()  , 0,     0},
                    { UNO_NAME_PARA_STYLELEVEL6,  WID_PARA_LEV6,  cppu::UnoType<OUString>::get()  , 0,     0},
                    { UNO_NAME_PARA_STYLELEVEL7,  WID_PARA_LEV7,  cppu::UnoType<OUString>::get()  , 0,     0},
                    { UNO_NAME_PARA_STYLELEVEL8,  WID_PARA_LEV8,  cppu::UnoType<OUString>::get()  , 0,     0},
                    { UNO_NAME_PARA_STYLELEVEL9,  WID_PARA_LEV9,  cppu::UnoType<OUString>::get()  , 0,     0},
                    { UNO_NAME_PARA_STYLELEVEL10,     WID_PARA_LEV10,     cppu::UnoType<OUString>::get()  , 0,     0},
                    { UNO_NAME_IS_RELATIVE_TABSTOPS, WID_IS_RELATIVE_TABSTOPS, cppu::UnoType<bool>::get(), PROPERTY_NONE, 0},
                    { UNO_NAME_DOCUMENT_INDEX_MARKS, WID_INDEX_MARKS, cppu::UnoType< cppu::UnoSequenceType<css::text::XDocumentIndexMark> >::get(),           PropertyAttribute::READONLY ,0       },
                    { u"", 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aTOXContentMap_Impl;
            }
            break;
            case PROPERTY_MAP_INDEX_USER:
            {
                static SfxItemPropertyMapEntry const aTOXUserMap_Impl[] =
                {
                    BASE_INDEX_PROPERTIES_
                    { UNO_NAME_CREATE_FROM_MARKS, WID_CREATE_FROM_MARKS                   ,  cppu::UnoType<bool>::get()  , PROPERTY_NONE,     0},
                    { UNO_NAME_HIDE_TAB_LEADER_AND_PAGE_NUMBERS, WID_HIDE_TABLEADER_PAGENUMBERS                   ,  cppu::UnoType<bool>::get()  , PROPERTY_NONE,     0},
                    { UNO_NAME_TAB_IN_TOC, WID_TAB_IN_TOC, cppu::UnoType<bool>::get(), PROPERTY_NONE, 0},
                    { UNO_NAME_TOC_BOOKMARK, WID_TOC_BOOKMARK, cppu::UnoType<OUString>::get(), PROPERTY_NONE, 0},
                    { UNO_NAME_TOC_NEWLINE, WID_TOC_NEWLINE, cppu::UnoType<bool>::get(), PROPERTY_NONE, 0},
                    { UNO_NAME_TOC_PARAGRAPH_OUTLINE_LEVEL, WID_TOC_PARAGRAPH_OUTLINE_LEVEL, cppu::UnoType<bool>::get(), PROPERTY_NONE, 0},
                    { UNO_NAME_CREATE_FROM_CHAPTER, WID_CREATE_FROM_CHAPTER                 ,  cppu::UnoType<bool>::get()  , PROPERTY_NONE,     0},
                    { UNO_NAME_IS_PROTECTED, WID_PROTECTED                           ,  cppu::UnoType<bool>::get()  , PROPERTY_NONE,     0},
                    { UNO_NAME_USE_LEVEL_FROM_SOURCE, WID_USE_LEVEL_FROM_SOURCE               ,  cppu::UnoType<bool>::get()  , PROPERTY_NONE,     0},
                    { UNO_NAME_LEVEL_FORMAT, WID_LEVEL_FORMAT                        ,  cppu::UnoType<css::container::XIndexReplace>::get()  , PROPERTY_NONE,0},
                    { UNO_NAME_LEVEL_PARAGRAPH_STYLES, WID_LEVEL_PARAGRAPH_STYLES              ,  cppu::UnoType<css::container::XIndexReplace>::get()  , PropertyAttribute::READONLY,0},
                    { UNO_NAME_CREATE_FROM_LEVEL_PARAGRAPH_STYLES, WID_CREATE_FROM_PARAGRAPH_STYLES, cppu::UnoType<bool>::get(), PROPERTY_NONE, 0},
                    { UNO_NAME_CREATE_FROM_TABLES, WID_CREATE_FROM_TABLES                  ,  cppu::UnoType<bool>::get()  , PROPERTY_NONE,     0},
                    { UNO_NAME_CREATE_FROM_TEXT_FRAMES, WID_CREATE_FROM_TEXT_FRAMES             ,  cppu::UnoType<bool>::get()  , PROPERTY_NONE,     0},
                    { UNO_NAME_CREATE_FROM_GRAPHIC_OBJECTS, WID_CREATE_FROM_GRAPHIC_OBJECTS         ,  cppu::UnoType<bool>::get()  , PROPERTY_NONE,     0},
                    { UNO_NAME_CREATE_FROM_EMBEDDED_OBJECTS, WID_CREATE_FROM_EMBEDDED_OBJECTS        ,  cppu::UnoType<bool>::get()  , PROPERTY_NONE,     0},
                    { UNO_NAME_TEXT_COLUMNS, RES_COL,                cppu::UnoType<css::text::XTextColumns>::get(),    PROPERTY_NONE, MID_COLUMNS},
                    { UNO_NAME_BACK_GRAPHIC_URL, RES_BACKGROUND,      cppu::UnoType<OUString>::get(), PROPERTY_NONE ,MID_GRAPHIC_URL    },
                    { UNO_NAME_BACK_GRAPHIC, RES_BACKGROUND,      cppu::UnoType<graphic::XGraphic>::get(), PROPERTY_NONE, MID_GRAPHIC    },
                    { UNO_NAME_BACK_GRAPHIC_FILTER, RES_BACKGROUND,       cppu::UnoType<OUString>::get(), PROPERTY_NONE ,MID_GRAPHIC_FILTER    },
                    { UNO_NAME_BACK_GRAPHIC_LOCATION, RES_BACKGROUND,         cppu::UnoType<css::style::GraphicLocation>::get(),          PROPERTY_NONE ,MID_GRAPHIC_POSITION},
                    { UNO_NAME_BACK_COLOR, RES_BACKGROUND,            cppu::UnoType<sal_Int32>::get(),           PROPERTY_NONE ,MID_BACK_COLOR        },
                    { UNO_NAME_BACK_TRANSPARENT, RES_BACKGROUND,      cppu::UnoType<bool>::get(),         PROPERTY_NONE ,MID_GRAPHIC_TRANSPARENT       },
                    { UNO_NAME_PARA_STYLEHEADING,     WID_PARA_HEAD,  cppu::UnoType<OUString>::get()  , 0,     0},
                    { UNO_NAME_PARA_STYLELEVEL1,  WID_PARA_LEV1,  cppu::UnoType<OUString>::get()  , 0,     0},
                    { UNO_NAME_PARA_STYLELEVEL2,  WID_PARA_LEV2,  cppu::UnoType<OUString>::get()  , 0,     0},
                    { UNO_NAME_PARA_STYLELEVEL3,  WID_PARA_LEV3,  cppu::UnoType<OUString>::get()  , 0,     0},
                    { UNO_NAME_PARA_STYLELEVEL4,  WID_PARA_LEV4,  cppu::UnoType<OUString>::get()  , 0,     0},
                    { UNO_NAME_PARA_STYLELEVEL5,  WID_PARA_LEV5,  cppu::UnoType<OUString>::get()  , 0,     0},
                    { UNO_NAME_PARA_STYLELEVEL6,  WID_PARA_LEV6,  cppu::UnoType<OUString>::get()  , 0,     0},
                    { UNO_NAME_PARA_STYLELEVEL7,  WID_PARA_LEV7,  cppu::UnoType<OUString>::get()  , 0,     0},
                    { UNO_NAME_PARA_STYLELEVEL8,  WID_PARA_LEV8,  cppu::UnoType<OUString>::get()  , 0,     0},
                    { UNO_NAME_PARA_STYLELEVEL9,  WID_PARA_LEV9,  cppu::UnoType<OUString>::get()  , 0,     0},
                    { UNO_NAME_PARA_STYLELEVEL10,     WID_PARA_LEV10,     cppu::UnoType<OUString>::get()  , 0,     0},
                    { UNO_NAME_DOCUMENT_INDEX_MARKS, WID_INDEX_MARKS, cppu::UnoType< cppu::UnoSequenceType<css::text::XDocumentIndexMark> >::get(),           PropertyAttribute::READONLY ,0       },
                    { UNO_NAME_IS_RELATIVE_TABSTOPS, WID_IS_RELATIVE_TABSTOPS, cppu::UnoType<bool>::get(), PROPERTY_NONE, 0},
                    { UNO_NAME_USER_INDEX_NAME, WID_USER_IDX_NAME,    cppu::UnoType<OUString>::get()  ,      PROPERTY_NONE,     0},
                    { u"", 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aTOXUserMap_Impl;
            }
            break;
            case PROPERTY_MAP_INDEX_TABLES:
            {
                static SfxItemPropertyMapEntry const aTOXTablesMap_Impl[] =
                {
                    BASE_INDEX_PROPERTIES_
                    { UNO_NAME_CREATE_FROM_CHAPTER, WID_CREATE_FROM_CHAPTER                 ,  cppu::UnoType<bool>::get()  , PROPERTY_NONE,     0},
                    { UNO_NAME_CREATE_FROM_LABELS, WID_CREATE_FROM_LABELS                  ,  cppu::UnoType<bool>::get()  , PROPERTY_NONE,     0},
                    { UNO_NAME_IS_PROTECTED, WID_PROTECTED                           ,  cppu::UnoType<bool>::get()  , PROPERTY_NONE,     0},
                    { UNO_NAME_LABEL_CATEGORY, WID_LABEL_CATEGORY                      ,  cppu::UnoType<OUString>::get()  , PROPERTY_NONE,     0},
                    { UNO_NAME_LABEL_DISPLAY_TYPE, WID_LABEL_DISPLAY_TYPE                  ,  cppu::UnoType<sal_Int16>::get()  , PROPERTY_NONE,     0},
                    { UNO_NAME_LEVEL_FORMAT, WID_LEVEL_FORMAT                        ,  cppu::UnoType<css::container::XIndexReplace>::get()  , PROPERTY_NONE,0},
                    { UNO_NAME_TEXT_COLUMNS, RES_COL,                cppu::UnoType<css::text::XTextColumns>::get(),    PROPERTY_NONE, MID_COLUMNS},
                    { UNO_NAME_BACK_GRAPHIC_URL, RES_BACKGROUND,      cppu::UnoType<OUString>::get(), PROPERTY_NONE ,MID_GRAPHIC_URL    },
                    { UNO_NAME_BACK_GRAPHIC, RES_BACKGROUND,      cppu::UnoType<graphic::XGraphic>::get(), PROPERTY_NONE, MID_GRAPHIC    },
                    { UNO_NAME_BACK_GRAPHIC_FILTER, RES_BACKGROUND,       cppu::UnoType<OUString>::get(), PROPERTY_NONE ,MID_GRAPHIC_FILTER    },
                    { UNO_NAME_BACK_GRAPHIC_LOCATION, RES_BACKGROUND,         cppu::UnoType<css::style::GraphicLocation>::get(),          PROPERTY_NONE ,MID_GRAPHIC_POSITION},
                    { UNO_NAME_BACK_COLOR, RES_BACKGROUND,            cppu::UnoType<sal_Int32>::get(),           PROPERTY_NONE ,MID_BACK_COLOR        },
                    { UNO_NAME_BACK_TRANSPARENT, RES_BACKGROUND,      cppu::UnoType<bool>::get(),         PROPERTY_NONE ,MID_GRAPHIC_TRANSPARENT       },
                    { UNO_NAME_PARA_STYLEHEADING,     WID_PARA_HEAD,  cppu::UnoType<OUString>::get()  , 0,     0},
                    { UNO_NAME_PARA_STYLELEVEL1,  WID_PARA_LEV1,  cppu::UnoType<OUString>::get()  , 0,     0},
                    { UNO_NAME_IS_RELATIVE_TABSTOPS, WID_IS_RELATIVE_TABSTOPS, cppu::UnoType<bool>::get(), PROPERTY_NONE, 0},
                    { u"", 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aTOXTablesMap_Impl;
            }
            break;
            case PROPERTY_MAP_INDEX_OBJECTS:
            {
                static SfxItemPropertyMapEntry const aTOXObjectsMap_Impl[] =
                {
                    BASE_INDEX_PROPERTIES_
                    { UNO_NAME_CREATE_FROM_CHAPTER, WID_CREATE_FROM_CHAPTER                 ,  cppu::UnoType<bool>::get()  , PROPERTY_NONE,     0},
                    { UNO_NAME_IS_PROTECTED, WID_PROTECTED                           ,  cppu::UnoType<bool>::get()  , PROPERTY_NONE,     0},
                    { UNO_NAME_USE_ALPHABETICAL_SEPARATORS, WID_USE_ALPHABETICAL_SEPARATORS         ,  cppu::UnoType<bool>::get()  , PROPERTY_NONE,     0},
                    { UNO_NAME_LEVEL_FORMAT, WID_LEVEL_FORMAT                        ,  cppu::UnoType<css::container::XIndexReplace>::get()  , PROPERTY_NONE,0},
                    { UNO_NAME_CREATE_FROM_STAR_MATH, WID_CREATE_FROM_STAR_MATH               ,  cppu::UnoType<bool>::get()  , PROPERTY_NONE,     0},
                    { UNO_NAME_CREATE_FROM_STAR_CHART, WID_CREATE_FROM_STAR_CHART              ,  cppu::UnoType<bool>::get()  , PROPERTY_NONE,     0},
                    { UNO_NAME_CREATE_FROM_STAR_CALC, WID_CREATE_FROM_STAR_CALC               ,  cppu::UnoType<bool>::get()  , PROPERTY_NONE,     0},
                    { UNO_NAME_CREATE_FROM_STAR_DRAW, WID_CREATE_FROM_STAR_DRAW               ,  cppu::UnoType<bool>::get()  , PROPERTY_NONE,     0},
                    { UNO_NAME_CREATE_FROM_OTHER_EMBEDDED_OBJECTS, WID_CREATE_FROM_OTHER_EMBEDDED_OBJECTS  ,  cppu::UnoType<bool>::get()  , PROPERTY_NONE,     0},
                    { UNO_NAME_TEXT_COLUMNS, RES_COL,                cppu::UnoType<css::text::XTextColumns>::get(),    PROPERTY_NONE, MID_COLUMNS},
                    { UNO_NAME_BACK_GRAPHIC_URL, RES_BACKGROUND,      cppu::UnoType<OUString>::get(), PROPERTY_NONE ,MID_GRAPHIC_URL    },
                    { UNO_NAME_BACK_GRAPHIC, RES_BACKGROUND,      cppu::UnoType<graphic::XGraphic>::get(), PROPERTY_NONE, MID_GRAPHIC    },
                    { UNO_NAME_BACK_GRAPHIC_FILTER, RES_BACKGROUND,       cppu::UnoType<OUString>::get(), PROPERTY_NONE ,MID_GRAPHIC_FILTER    },
                    { UNO_NAME_BACK_GRAPHIC_LOCATION, RES_BACKGROUND,         cppu::UnoType<css::style::GraphicLocation>::get(),          PROPERTY_NONE ,MID_GRAPHIC_POSITION},
                    { UNO_NAME_BACK_COLOR, RES_BACKGROUND,            cppu::UnoType<sal_Int32>::get(),           PROPERTY_NONE ,MID_BACK_COLOR        },
                    { UNO_NAME_BACK_TRANSPARENT, RES_BACKGROUND,      cppu::UnoType<bool>::get(),         PROPERTY_NONE ,MID_GRAPHIC_TRANSPARENT       },
                    { UNO_NAME_PARA_STYLEHEADING,     WID_PARA_HEAD,  cppu::UnoType<OUString>::get()  , 0,     0},
                    { UNO_NAME_PARA_STYLELEVEL1,  WID_PARA_LEV1,  cppu::UnoType<OUString>::get()  , 0,     0},
                    { UNO_NAME_IS_RELATIVE_TABSTOPS, WID_IS_RELATIVE_TABSTOPS, cppu::UnoType<bool>::get(), PROPERTY_NONE, 0},
                    { u"", 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aTOXObjectsMap_Impl;
            }
            break;
            case PROPERTY_MAP_INDEX_ILLUSTRATIONS:
            {
                static SfxItemPropertyMapEntry const aTOXIllustrationsMap_Impl[] =
                {
                    BASE_INDEX_PROPERTIES_
                    { UNO_NAME_CREATE_FROM_CHAPTER, WID_CREATE_FROM_CHAPTER                 ,  cppu::UnoType<bool>::get()  , PROPERTY_NONE,     0},
                    { UNO_NAME_CREATE_FROM_LABELS, WID_CREATE_FROM_LABELS                  ,  cppu::UnoType<bool>::get()  , PROPERTY_NONE,     0},
                    { UNO_NAME_IS_PROTECTED, WID_PROTECTED                           ,  cppu::UnoType<bool>::get()  , PROPERTY_NONE,     0},
                    { UNO_NAME_USE_ALPHABETICAL_SEPARATORS, WID_USE_ALPHABETICAL_SEPARATORS         ,  cppu::UnoType<bool>::get()  , PROPERTY_NONE,     0},
                    { UNO_NAME_LABEL_CATEGORY, WID_LABEL_CATEGORY                      ,  cppu::UnoType<OUString>::get()  , PROPERTY_NONE,     0},
                    { UNO_NAME_LABEL_DISPLAY_TYPE, WID_LABEL_DISPLAY_TYPE                  ,  cppu::UnoType<sal_Int16>::get()  , PROPERTY_NONE,     0},
                    { UNO_NAME_LEVEL_FORMAT, WID_LEVEL_FORMAT                        ,  cppu::UnoType<css::container::XIndexReplace>::get()  , PROPERTY_NONE,0},
                    { UNO_NAME_TEXT_COLUMNS, RES_COL,                cppu::UnoType<css::text::XTextColumns>::get(),    PROPERTY_NONE, MID_COLUMNS},
                    { UNO_NAME_BACK_GRAPHIC_URL, RES_BACKGROUND,      cppu::UnoType<OUString>::get(), PROPERTY_NONE ,MID_GRAPHIC_URL    },
                    { UNO_NAME_BACK_GRAPHIC, RES_BACKGROUND,      cppu::UnoType<graphic::XGraphic>::get(), PROPERTY_NONE, MID_GRAPHIC    },
                    { UNO_NAME_BACK_GRAPHIC_FILTER, RES_BACKGROUND,       cppu::UnoType<OUString>::get(), PROPERTY_NONE ,MID_GRAPHIC_FILTER    },
                    { UNO_NAME_BACK_GRAPHIC_LOCATION, RES_BACKGROUND,         cppu::UnoType<css::style::GraphicLocation>::get(),          PROPERTY_NONE ,MID_GRAPHIC_POSITION},
                    { UNO_NAME_BACK_COLOR, RES_BACKGROUND,            cppu::UnoType<sal_Int32>::get(),           PROPERTY_NONE ,MID_BACK_COLOR        },
                    { UNO_NAME_BACK_TRANSPARENT, RES_BACKGROUND,      cppu::UnoType<bool>::get(),         PROPERTY_NONE ,MID_GRAPHIC_TRANSPARENT       },
                    { UNO_NAME_PARA_STYLEHEADING,     WID_PARA_HEAD,  cppu::UnoType<OUString>::get()  , 0,     0},
                    { UNO_NAME_PARA_STYLELEVEL1,  WID_PARA_LEV1,  cppu::UnoType<OUString>::get()  , 0,     0},
                    { UNO_NAME_IS_RELATIVE_TABSTOPS, WID_IS_RELATIVE_TABSTOPS, cppu::UnoType<bool>::get(), PROPERTY_NONE, 0},
                    { u"", 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aTOXIllustrationsMap_Impl;
            }
            break;
            case PROPERTY_MAP_TEXT_TABLE_ROW:
            {
                static SfxItemPropertyMapEntry const aTableRowPropertyMap_Impl[] =
                {
                    { UNO_NAME_BACK_COLOR, RES_BACKGROUND, cppu::UnoType<sal_Int32>::get(), PROPERTY_NONE ,MID_BACK_COLOR         },
                    { UNO_NAME_BACK_GRAPHIC_URL, RES_BACKGROUND,      cppu::UnoType<OUString>::get(), PROPERTY_NONE ,MID_GRAPHIC_URL    },
                    { UNO_NAME_BACK_GRAPHIC, RES_BACKGROUND,      cppu::UnoType<graphic::XGraphic>::get(), PROPERTY_NONE, MID_GRAPHIC    },
                    { UNO_NAME_BACK_GRAPHIC_FILTER, RES_BACKGROUND,       cppu::UnoType<OUString>::get(), PROPERTY_NONE ,MID_GRAPHIC_FILTER    },
                    { UNO_NAME_BACK_GRAPHIC_LOCATION, RES_BACKGROUND,         cppu::UnoType<css::style::GraphicLocation>::get(),          PROPERTY_NONE ,MID_GRAPHIC_POSITION},
                    { UNO_NAME_BACK_TRANSPARENT, RES_BACKGROUND,      cppu::UnoType<bool>::get(),         PROPERTY_NONE ,MID_GRAPHIC_TRANSPARENT       },
                    { UNO_NAME_TABLE_COLUMN_SEPARATORS, FN_UNO_TABLE_COLUMN_SEPARATORS,   cppu::UnoType< cppu::UnoSequenceType<css::text::TableColumnSeparator> >::get(),   PropertyAttribute::MAYBEVOID, 0 },
                    { UNO_NAME_HEIGHT, FN_UNO_ROW_HEIGHT,     cppu::UnoType<sal_Int32>::get(),           PROPERTY_NONE ,CONVERT_TWIPS },
                    { UNO_NAME_IS_AUTO_HEIGHT, FN_UNO_ROW_AUTO_HEIGHT,    cppu::UnoType<bool>::get(),         PROPERTY_NONE , 0 },
                    { UNO_NAME_SIZE_TYPE, RES_FRM_SIZE,           cppu::UnoType<sal_Int16>::get()  ,         PROPERTY_NONE,   MID_FRMSIZE_SIZE_TYPE  },
                    { UNO_NAME_WIDTH_TYPE, RES_FRM_SIZE,          cppu::UnoType<sal_Int16>::get()  ,         PROPERTY_NONE,   MID_FRMSIZE_WIDTH_TYPE },
                    { UNO_NAME_IS_SPLIT_ALLOWED, RES_ROW_SPLIT,       cppu::UnoType<bool>::get()  , PropertyAttribute::MAYBEVOID, 0},
                    { UNO_NAME_HAS_TEXT_CHANGES_ONLY, RES_PRINT, cppu::UnoType<bool>::get()  , PropertyAttribute::MAYBEVOID, 0},
                    { UNO_NAME_ROW_INTEROP_GRAB_BAG, RES_FRMATR_GRABBAG, cppu::UnoType< cppu::UnoSequenceType<css::beans::PropertyValue> >::get(), PROPERTY_NONE, 0 },
                    { u"", 0, css::uno::Type(), 0, 0 }
                };

                m_aMapEntriesArr[nPropertyId] = aTableRowPropertyMap_Impl;
            }
            break;
            case PROPERTY_MAP_TEXT_TABLE_CURSOR:
            {
                m_aMapEntriesArr[nPropertyId] = GetTextTableCursorPropertyMap();
            }
            break;
            case PROPERTY_MAP_BOOKMARK:
            {
                m_aMapEntriesArr[nPropertyId] = GetBookmarkPropertyMap();
            }
            break;
            case PROPERTY_MAP_FIELDMARK:
            {
                static SfxItemPropertyMapEntry const aFieldmarkMap_Impl[] =
                {
                    // FIXME: is this supposed to actually exist as UNO property, or is it supposed to be in the "parameters" of the field?
                    { u"Checked", 0, cppu::UnoType<bool>::get(), PROPERTY_NONE,     0},
                    { u"", 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aFieldmarkMap_Impl;
            }
            break;
            case PROPERTY_MAP_PARAGRAPH_EXTENSIONS:
            {
                m_aMapEntriesArr[nPropertyId] = GetParagraphExtensionsPropertyMap();
            }
            break;
            case PROPERTY_MAP_BIBLIOGRAPHY :
            {
                static SfxItemPropertyMapEntry const aBibliographyMap_Impl[] =
                {
                    BASE_INDEX_PROPERTIES_
                    { UNO_NAME_IS_PROTECTED, WID_PROTECTED                           ,  cppu::UnoType<bool>::get()  , PROPERTY_NONE,     0},
                    { UNO_NAME_TEXT_COLUMNS, RES_COL,                cppu::UnoType<css::text::XTextColumns>::get(),    PROPERTY_NONE, MID_COLUMNS},
                    { UNO_NAME_BACK_GRAPHIC_URL, RES_BACKGROUND,      cppu::UnoType<OUString>::get(), PROPERTY_NONE ,MID_GRAPHIC_URL    },
                    { UNO_NAME_BACK_GRAPHIC, RES_BACKGROUND,      cppu::UnoType<graphic::XGraphic>::get(), PROPERTY_NONE, MID_GRAPHIC    },
                    { UNO_NAME_BACK_GRAPHIC_FILTER, RES_BACKGROUND,       cppu::UnoType<OUString>::get(), PROPERTY_NONE ,MID_GRAPHIC_FILTER    },
                    { UNO_NAME_BACK_GRAPHIC_LOCATION, RES_BACKGROUND,         cppu::UnoType<css::style::GraphicLocation>::get(),          PROPERTY_NONE ,MID_GRAPHIC_POSITION},
                    { UNO_NAME_BACK_COLOR, RES_BACKGROUND,            cppu::UnoType<sal_Int32>::get(),           PROPERTY_NONE ,MID_BACK_COLOR        },
                    { UNO_NAME_BACK_TRANSPARENT, RES_BACKGROUND,      cppu::UnoType<bool>::get(),         PROPERTY_NONE ,MID_GRAPHIC_TRANSPARENT       },
                    { UNO_NAME_PARA_STYLEHEADING,     WID_PARA_HEAD,  cppu::UnoType<OUString>::get()  , 0,     0},
                    { UNO_NAME_PARA_STYLELEVEL1,  WID_PARA_LEV1,  cppu::UnoType<OUString>::get()  , 0,     0},
                    { UNO_NAME_LEVEL_FORMAT, WID_LEVEL_FORMAT                        ,  cppu::UnoType<css::container::XIndexReplace>::get()  , PROPERTY_NONE,0},
                    { UNO_NAME_LOCALE,            WID_IDX_LOCALE,         cppu::UnoType<css::lang::Locale>::get(), PROPERTY_NONE,     0},
                    { UNO_NAME_SORT_ALGORITHM,    WID_IDX_SORT_ALGORITHM,  cppu::UnoType<OUString>::get(), PROPERTY_NONE,     0},
                    { u"", 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aBibliographyMap_Impl;
            }
            break;
            case PROPERTY_MAP_TEXT_DOCUMENT:
            {
                static SfxItemPropertyMapEntry const aDocMap_Impl[] =
                {
                    { UNO_NAME_BASIC_LIBRARIES, WID_DOC_BASIC_LIBRARIES,  cppu::UnoType<css::script::XLibraryContainer>::get(), PropertyAttribute::READONLY, 0},
                    { UNO_NAME_CHAR_FONT_NAME, RES_CHRATR_FONT,       cppu::UnoType<OUString>::get(),  PropertyAttribute::MAYBEVOID, MID_FONT_FAMILY_NAME },
                    { UNO_NAME_CHAR_FONT_STYLE_NAME, RES_CHRATR_FONT,     cppu::UnoType<OUString>::get(), PropertyAttribute::MAYBEVOID, MID_FONT_STYLE_NAME },
                    { UNO_NAME_CHAR_FONT_FAMILY, RES_CHRATR_FONT,     cppu::UnoType<sal_Int16>::get(),                   PropertyAttribute::MAYBEVOID, MID_FONT_FAMILY   },
                    { UNO_NAME_CHAR_FONT_CHAR_SET, RES_CHRATR_FONT,       cppu::UnoType<sal_Int16>::get(),   PropertyAttribute::MAYBEVOID, MID_FONT_CHAR_SET },
                    { UNO_NAME_CHAR_FONT_PITCH, RES_CHRATR_FONT,      cppu::UnoType<sal_Int16>::get(),                   PropertyAttribute::MAYBEVOID, MID_FONT_PITCH   },
                    { UNO_NAME_CHAR_FONT_NAME_ASIAN, RES_CHRATR_CJK_FONT,     cppu::UnoType<OUString>::get(),  PropertyAttribute::MAYBEVOID, MID_FONT_FAMILY_NAME },
                    { UNO_NAME_CHAR_FONT_STYLE_NAME_ASIAN, RES_CHRATR_CJK_FONT,   cppu::UnoType<OUString>::get(),    PropertyAttribute::MAYBEVOID, MID_FONT_STYLE_NAME },
                    { UNO_NAME_CHAR_FONT_FAMILY_ASIAN, RES_CHRATR_CJK_FONT,   cppu::UnoType<sal_Int16>::get(),   PropertyAttribute::MAYBEVOID, MID_FONT_FAMILY   },
                    { UNO_NAME_CHAR_FONT_CHAR_SET_ASIAN, RES_CHRATR_CJK_FONT, cppu::UnoType<sal_Int16>::get(),   PropertyAttribute::MAYBEVOID, MID_FONT_CHAR_SET },
                    { UNO_NAME_CHAR_FONT_PITCH_ASIAN, RES_CHRATR_CJK_FONT,    cppu::UnoType<sal_Int16>::get(),   PropertyAttribute::MAYBEVOID, MID_FONT_PITCH   },
                    { UNO_NAME_CHAR_FONT_NAME_COMPLEX, RES_CHRATR_CTL_FONT,   cppu::UnoType<OUString>::get(),    PropertyAttribute::MAYBEVOID, MID_FONT_FAMILY_NAME },
                    { UNO_NAME_CHAR_FONT_STYLE_NAME_COMPLEX, RES_CHRATR_CTL_FONT, cppu::UnoType<OUString>::get(),    PropertyAttribute::MAYBEVOID, MID_FONT_STYLE_NAME },
                    { UNO_NAME_CHAR_FONT_FAMILY_COMPLEX, RES_CHRATR_CTL_FONT, cppu::UnoType<sal_Int16>::get(),   PropertyAttribute::MAYBEVOID, MID_FONT_FAMILY   },
                    { UNO_NAME_CHAR_FONT_CHAR_SET_COMPLEX, RES_CHRATR_CTL_FONT,   cppu::UnoType<sal_Int16>::get(),   PropertyAttribute::MAYBEVOID, MID_FONT_CHAR_SET },
                    { UNO_NAME_CHAR_FONT_PITCH_COMPLEX, RES_CHRATR_CTL_FONT,  cppu::UnoType<sal_Int16>::get(),   PropertyAttribute::MAYBEVOID, MID_FONT_PITCH   },
                    { UNO_NAME_CHAR_LOCALE, RES_CHRATR_LANGUAGE ,   cppu::UnoType<css::lang::Locale>::get(), PropertyAttribute::MAYBEVOID,  MID_LANG_LOCALE },
                    { UNO_NAME_CHARACTER_COUNT, WID_DOC_CHAR_COUNT,           cppu::UnoType<sal_Int32>::get(),   PropertyAttribute::READONLY,   0},
                    { UNO_NAME_DIALOG_LIBRARIES, WID_DOC_DIALOG_LIBRARIES,  cppu::UnoType<css::script::XLibraryContainer>::get(), PropertyAttribute::READONLY, 0},
                    { UNO_NAME_VBA_DOCOBJ, WID_DOC_VBA_DOCOBJ,  cppu::UnoType<OUString>::get(), PropertyAttribute::READONLY, 0},
                    { UNO_NAME_INDEX_AUTO_MARK_FILE_U_R_L, WID_DOC_AUTO_MARK_URL, cppu::UnoType<OUString>::get(),    PROPERTY_NONE,   0},
                    { UNO_NAME_PARAGRAPH_COUNT, WID_DOC_PARA_COUNT,           cppu::UnoType<sal_Int32>::get(),   PropertyAttribute::READONLY,   0},
                    { UNO_NAME_RECORD_CHANGES, WID_DOC_CHANGES_RECORD,        cppu::UnoType<bool>::get(), PROPERTY_NONE,   0},
                    { UNO_NAME_SHOW_CHANGES, WID_DOC_CHANGES_SHOW,        cppu::UnoType<bool>::get(), PROPERTY_NONE,   0},
                    { UNO_NAME_WORD_COUNT, WID_DOC_WORD_COUNT,            cppu::UnoType<sal_Int32>::get(),   PropertyAttribute::READONLY,   0},
                    { UNO_NAME_IS_TEMPLATE, WID_DOC_ISTEMPLATEID,         cppu::UnoType<bool>::get(), PropertyAttribute::READONLY,   0},
                    { UNO_NAME_WORD_SEPARATOR, WID_DOC_WORD_SEPARATOR,        cppu::UnoType<OUString>::get(),    PROPERTY_NONE,   0},
                    { UNO_NAME_HIDE_FIELD_TIPS, WID_DOC_HIDE_TIPS,            cppu::UnoType<bool>::get(), PROPERTY_NONE,   0},
                    { UNO_NAME_REDLINE_DISPLAY_TYPE, WID_DOC_REDLINE_DISPLAY,     cppu::UnoType<sal_Int16>::get(),   PROPERTY_NONE,   0},
                    { UNO_NAME_REDLINE_PROTECTION_KEY, WID_DOC_CHANGES_PASSWORD,      cppu::UnoType< cppu::UnoSequenceType<sal_Int8> >::get(), PROPERTY_NONE, 0 },
                    { UNO_NAME_FORBIDDEN_CHARACTERS, WID_DOC_FORBIDDEN_CHARS,    cppu::UnoType<css::i18n::XForbiddenCharacters>::get(), PROPERTY_NONE,   0},
                    { UNO_NAME_TWO_DIGIT_YEAR, WID_DOC_TWO_DIGIT_YEAR,    cppu::UnoType<sal_Int16>::get(),   PROPERTY_NONE,   0},
                    { UNO_NAME_AUTOMATIC_CONTROL_FOCUS,       WID_DOC_AUTOMATIC_CONTROL_FOCUS,    cppu::UnoType<bool>::get(), PROPERTY_NONE,   0},
                    { UNO_NAME_APPLY_FORM_DESIGN_MODE,        WID_DOC_APPLY_FORM_DESIGN_MODE,     cppu::UnoType<bool>::get(), PROPERTY_NONE,   0},
                    { UNO_NAME_RUNTIME_UID, WID_DOC_RUNTIME_UID,  cppu::UnoType<OUString>::get(), PropertyAttribute::READONLY, 0},
                    { UNO_NAME_LOCK_UPDATES,        WID_DOC_LOCK_UPDATES,     cppu::UnoType<bool>::get(), PROPERTY_NONE,   0},
                    { u"UndocumentedWriterfilterHack", WID_DOC_WRITERFILTER,     cppu::UnoType<bool>::get(), PROPERTY_NONE,   0},
                    { UNO_NAME_HAS_VALID_SIGNATURES,  WID_DOC_HAS_VALID_SIGNATURES, cppu::UnoType<bool>::get(), PropertyAttribute::READONLY,   0},
                    { UNO_NAME_BUILDID, WID_DOC_BUILDID, cppu::UnoType<OUString>::get(), 0, 0},
                    { UNO_NAME_DOC_INTEROP_GRAB_BAG, WID_DOC_INTEROP_GRAB_BAG, cppu::UnoType< cppu::UnoSequenceType<css::beans::PropertyValue> >::get(), PROPERTY_NONE, 0 },
                    { UNO_NAME_DEFAULT_PAGE_MODE,  WID_DOC_DEFAULT_PAGE_MODE,  cppu::UnoType<bool>::get(), PROPERTY_NONE,  0},
                    { u"", 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aDocMap_Impl;
            }
            break;
            case PROPERTY_MAP_LINK_TARGET:
            {
                static SfxItemPropertyMapEntry const aLinkTargetMap_Impl[] =
                {
                    { UNO_LINK_DISPLAY_BITMAP, 0,     cppu::UnoType<css::awt::XBitmap>::get(), PropertyAttribute::READONLY, 0xbf},
                    { UNO_LINK_DISPLAY_NAME, 0,   cppu::UnoType<OUString>::get(), PropertyAttribute::READONLY, 0xbf},
                    { u"", 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aLinkTargetMap_Impl;
            }
            break;
            case PROPERTY_MAP_AUTO_TEXT_GROUP :
            {
                static SfxItemPropertyMapEntry const aAutoTextGroupMap_Impl[] =
                {
                    { UNO_NAME_FILE_PATH, WID_GROUP_PATH,     cppu::UnoType<OUString>::get(), PropertyAttribute::READONLY, 0},
                    { UNO_NAME_TITLE, WID_GROUP_TITLE, cppu::UnoType<OUString>::get(),   PROPERTY_NONE,   0},
                    { u"", 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aAutoTextGroupMap_Impl;
            }
            break;
            case PROPERTY_MAP_TEXTPORTION_EXTENSIONS:
            {
                m_aMapEntriesArr[nPropertyId] = GetTextPortionExtensionPropertyMap();
            }
            break;
            case PROPERTY_MAP_FOOTNOTE:
            {
                m_aMapEntriesArr[nPropertyId] = GetFootnotePropertyMap();
            }
            break;
            case PROPERTY_MAP_REDLINE :
            {
                m_aMapEntriesArr[nPropertyId] = GetRedlinePropertyMap();
            }
            break;
            case PROPERTY_MAP_TEXT_DEFAULT :
            {
                SfxItemPropertyMapEntry* aTextDefaultMap_Impl = GetTextDefaultPropertyMap();
                m_aMapEntriesArr[nPropertyId] = aTextDefaultMap_Impl;
                for( SfxItemPropertyMapEntry * pMap = aTextDefaultMap_Impl;
                     !pMap->aName.isEmpty(); ++pMap )
                {
                    // UNO_NAME_PAGE_DESC_NAME should keep its MAYBEVOID flag
                    if (RES_PAGEDESC != pMap->nWID || MID_PAGEDESC_PAGEDESCNAME != pMap->nMemberId)
                        pMap->nFlags &= ~PropertyAttribute::MAYBEVOID;
                }
            }
            break;
            case PROPERTY_MAP_REDLINE_PORTION :
            {
                m_aMapEntriesArr[nPropertyId] = GetRedlinePortionPropertyMap();
            }
            break;
            case  PROPERTY_MAP_FLDTYP_DATETIME:
            {
                static SfxItemPropertyMapEntry const aDateTimeFieldPropMap[] =
                {
                    { UNO_NAME_ADJUST, FIELD_PROP_SUBTYPE,     cppu::UnoType<sal_Int32>::get(), PROPERTY_NONE,    0},
                    { UNO_NAME_DATE_TIME_VALUE, FIELD_PROP_DATE_TIME,  cppu::UnoType<css::util::DateTime>::get(), PROPERTY_NONE, 0},
                    { UNO_NAME_IS_FIXED,       FIELD_PROP_BOOL1,   cppu::UnoType<bool>::get()  , PROPERTY_NONE,0},
                    { UNO_NAME_IS_DATE,    FIELD_PROP_BOOL2,   cppu::UnoType<bool>::get()  , PROPERTY_NONE,0},
                    { UNO_NAME_NUMBER_FORMAT, FIELD_PROP_FORMAT,   cppu::UnoType<sal_Int32>::get(), PROPERTY_NONE,    0},
                    { UNO_NAME_IS_FIXED_LANGUAGE, FIELD_PROP_BOOL4, cppu::UnoType<bool>::get(), PROPERTY_NONE,    0},
                    COMMON_FLDTYP_PROPERTIES
                    { u"", 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aDateTimeFieldPropMap;
            }
            break;
            case  PROPERTY_MAP_FLDTYP_USER     :
            {
                static SfxItemPropertyMapEntry const aUserFieldPropMap[] =
                {
                    { UNO_NAME_IS_SHOW_FORMULA, FIELD_PROP_BOOL2,  cppu::UnoType<bool>::get(), PROPERTY_NONE,  0},
                    { UNO_NAME_IS_VISIBLE,     FIELD_PROP_BOOL1,   cppu::UnoType<bool>::get(), PROPERTY_NONE,  0},
                    { UNO_NAME_NUMBER_FORMAT,  FIELD_PROP_FORMAT,  cppu::UnoType<sal_Int32>::get(), PROPERTY_NONE,    0},
                    { UNO_NAME_IS_FIXED_LANGUAGE, FIELD_PROP_BOOL4, cppu::UnoType<bool>::get(), PROPERTY_NONE,    0},
                    COMMON_FLDTYP_PROPERTIES
                    { u"", 0, css::uno::Type(), 0, 0 }
                };

                m_aMapEntriesArr[nPropertyId] = aUserFieldPropMap;
            }
            break;
            case  PROPERTY_MAP_FLDTYP_SET_EXP  :
            {
                static SfxItemPropertyMapEntry const aSetExpFieldPropMap     [] =
                {
                    { UNO_NAME_CONTENT,            FIELD_PROP_PAR2,    cppu::UnoType<OUString>::get(),   PROPERTY_NONE, 0},
                    { UNO_NAME_CURRENT_PRESENTATION, FIELD_PROP_PAR4, cppu::UnoType<OUString>::get(),  PROPERTY_NONE, 0},
                    { UNO_NAME_HINT,               FIELD_PROP_PAR3, cppu::UnoType<OUString>::get(),   PROPERTY_NONE, 0},
                    { UNO_NAME_NUMBER_FORMAT,      FIELD_PROP_FORMAT,  cppu::UnoType<sal_Int32>::get(), PROPERTY_NONE,    0},
                    { UNO_NAME_NUMBERING_TYPE,     FIELD_PROP_USHORT2, cppu::UnoType<sal_Int16>::get(), PROPERTY_NONE,    0},
                    { UNO_NAME_IS_INPUT,       FIELD_PROP_BOOL1,   cppu::UnoType<bool>::get(), PROPERTY_NONE, 0},
                    // #i69733# wrong name - UNO_NAME_IS_INPUT expanded to "Input" instead of "IsInput"
                    { UNO_NAME_INPUT,          FIELD_PROP_BOOL1,   cppu::UnoType<bool>::get(), PROPERTY_NONE, 0},
                    { UNO_NAME_IS_SHOW_FORMULA, FIELD_PROP_BOOL3,  cppu::UnoType<bool>::get(), PROPERTY_NONE,  0},
                    { UNO_NAME_IS_VISIBLE,       FIELD_PROP_BOOL2,   cppu::UnoType<bool>::get(),    PROPERTY_NONE, 0},
                    { UNO_NAME_SEQUENCE_VALUE, FIELD_PROP_USHORT1, cppu::UnoType<sal_Int16>::get(),   PROPERTY_NONE,  0},
                    { UNO_NAME_SUB_TYPE,           FIELD_PROP_SUBTYPE, cppu::UnoType<sal_Int16>::get(), PROPERTY_NONE,    0},
                    { UNO_NAME_VALUE,          FIELD_PROP_DOUBLE,  cppu::UnoType<double>::get(),  PROPERTY_NONE,  0},
                    { UNO_NAME_VARIABLE_NAME,  FIELD_PROP_PAR1,    cppu::UnoType<OUString>::get(),   PropertyAttribute::READONLY, 0},
                    { UNO_NAME_IS_FIXED_LANGUAGE, FIELD_PROP_BOOL4, cppu::UnoType<bool>::get(), PROPERTY_NONE,    0},
                    COMMON_FLDTYP_PROPERTIES
                    { u"", 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aSetExpFieldPropMap;
            }
            break;
            case  PROPERTY_MAP_FLDTYP_GET_EXP  :
            {
                static SfxItemPropertyMapEntry const aGetExpFieldPropMap     [] =
                {
                    { UNO_NAME_CONTENT,            FIELD_PROP_PAR1,    cppu::UnoType<OUString>::get(),   PROPERTY_NONE, 0},
                    { UNO_NAME_CURRENT_PRESENTATION, FIELD_PROP_PAR4, cppu::UnoType<OUString>::get(),  PROPERTY_NONE, 0},
                    { UNO_NAME_IS_SHOW_FORMULA, FIELD_PROP_BOOL2,  cppu::UnoType<bool>::get(), PROPERTY_NONE,  0},
                    { UNO_NAME_NUMBER_FORMAT,  FIELD_PROP_FORMAT,  cppu::UnoType<sal_Int32>::get(), PROPERTY_NONE,    0},
                    { UNO_NAME_SUB_TYPE,           FIELD_PROP_SUBTYPE, cppu::UnoType<sal_Int16>::get(), PROPERTY_NONE,    0},
                    { UNO_NAME_VALUE,          FIELD_PROP_DOUBLE,  cppu::UnoType<double>::get(), PropertyAttribute::READONLY, 0},
                    { UNO_NAME_VARIABLE_SUBTYPE,   FIELD_PROP_USHORT1, cppu::UnoType<sal_Int16>::get(), PROPERTY_NONE,    0},
                    { UNO_NAME_IS_FIXED_LANGUAGE, FIELD_PROP_BOOL4, cppu::UnoType<bool>::get(), PROPERTY_NONE,    0},
                    COMMON_FLDTYP_PROPERTIES
                    { u"", 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aGetExpFieldPropMap;
            }
            break;
            case  PROPERTY_MAP_FLDTYP_FILE_NAME:
            {
                static SfxItemPropertyMapEntry const aFileNameFieldPropMap   [] =
                {
                    { UNO_NAME_CURRENT_PRESENTATION, FIELD_PROP_PAR3, cppu::UnoType<OUString>::get(),  PROPERTY_NONE, 0},
                    { UNO_NAME_FILE_FORMAT, FIELD_PROP_FORMAT, cppu::UnoType<sal_Int16>::get(), PROPERTY_NONE,    0},
                    { UNO_NAME_IS_FIXED,   FIELD_PROP_BOOL2, cppu::UnoType<bool>::get(),       PROPERTY_NONE, 0},
                    COMMON_FLDTYP_PROPERTIES
                    { u"", 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aFileNameFieldPropMap;
            }
            break;
            case  PROPERTY_MAP_FLDTYP_PAGE_NUM :
            {
                static SfxItemPropertyMapEntry const aPageNumFieldPropMap        [] =
                {
                    { UNO_NAME_NUMBERING_TYPE,     FIELD_PROP_FORMAT,  cppu::UnoType<sal_Int16>::get(), PROPERTY_NONE,    0},
                    { UNO_NAME_OFFSET,             FIELD_PROP_USHORT1, cppu::UnoType<sal_Int16>::get(),  PROPERTY_NONE,   0},
                    { UNO_NAME_SUB_TYPE,           FIELD_PROP_SUBTYPE, cppu::UnoType<css::text::PageNumberType>::get(), PROPERTY_NONE,  0},
                    { UNO_NAME_USERTEXT,           FIELD_PROP_PAR1,    cppu::UnoType<OUString>::get(), PROPERTY_NONE, 0},
                    COMMON_FLDTYP_PROPERTIES
                    { u"", 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aPageNumFieldPropMap;
            }
            break;
            case  PROPERTY_MAP_FLDTYP_AUTHOR   :
            {
                static SfxItemPropertyMapEntry const aAuthorFieldPropMap     [] =
                {
                    { UNO_NAME_CONTENT,    FIELD_PROP_PAR1, cppu::UnoType<OUString>::get(),   PROPERTY_NONE, 0},
                    { UNO_NAME_CURRENT_PRESENTATION, FIELD_PROP_PAR1, cppu::UnoType<OUString>::get(),  PROPERTY_NONE, 0},
                    { UNO_NAME_IS_FIXED,   FIELD_PROP_BOOL2, cppu::UnoType<bool>::get(),       PROPERTY_NONE, 0},
                    { UNO_NAME_FULL_NAME,FIELD_PROP_BOOL1, cppu::UnoType<bool>::get(),     PROPERTY_NONE, 0},
                    COMMON_FLDTYP_PROPERTIES
                    { u"", 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aAuthorFieldPropMap;
            }
            break;
            case  PROPERTY_MAP_FLDTYP_CHAPTER  :
            {
                static SfxItemPropertyMapEntry const aChapterFieldPropMap        [] =
                {
                    { UNO_NAME_CHAPTER_FORMAT,FIELD_PROP_USHORT1,  cppu::UnoType<sal_Int16>::get(),   PROPERTY_NONE, 0},
                    { UNO_NAME_LEVEL,FIELD_PROP_BYTE1,         cppu::UnoType<sal_Int8>::get(),    PROPERTY_NONE, 0},
                    COMMON_FLDTYP_PROPERTIES
                    { u"", 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aChapterFieldPropMap;
            }
            break;
            case PROPERTY_MAP_FLDTYP_GET_REFERENCE          :
            {
                static SfxItemPropertyMapEntry const aGetRefFieldPropMap     [] =
                {
                    { UNO_NAME_CURRENT_PRESENTATION, FIELD_PROP_PAR3, cppu::UnoType<OUString>::get(),  PROPERTY_NONE, 0},
                    { UNO_NAME_REFERENCE_FIELD_PART,FIELD_PROP_USHORT1, cppu::UnoType<sal_Int16>::get(),  PROPERTY_NONE,  0},
                    { UNO_NAME_REFERENCE_FIELD_SOURCE,FIELD_PROP_USHORT2, cppu::UnoType<sal_Int16>::get(),    PROPERTY_NONE,  0},
                    { UNO_NAME_SEQUENCE_NUMBER,    FIELD_PROP_SHORT1,  cppu::UnoType<sal_Int16>::get(),   PROPERTY_NONE, 0},
                    { UNO_NAME_SOURCE_NAME,        FIELD_PROP_PAR1,    cppu::UnoType<OUString>::get(),   PROPERTY_NONE, 0},
                    { UNO_NAME_REFERENCE_FIELD_LANGUAGE, FIELD_PROP_PAR4, cppu::UnoType<OUString>::get(),  PROPERTY_NONE, 0},
                    COMMON_FLDTYP_PROPERTIES
                    { u"", 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aGetRefFieldPropMap;
            }
            break;
            case PROPERTY_MAP_FLDTYP_CONDITIONED_TEXT      :
            {
                static SfxItemPropertyMapEntry const aConditionedTextFieldPropMap [] =
                {
                    { UNO_NAME_CONDITION,      FIELD_PROP_PAR1, cppu::UnoType<OUString>::get(),   PROPERTY_NONE, 0},
                    { UNO_NAME_FALSE_CONTENT,  FIELD_PROP_PAR3, cppu::UnoType<OUString>::get(),   PROPERTY_NONE, 0},
                    { UNO_NAME_IS_CONDITION_TRUE ,  FIELD_PROP_BOOL1, cppu::UnoType<bool>::get(),   PROPERTY_NONE, 0},
                    { UNO_NAME_TRUE_CONTENT ,  FIELD_PROP_PAR2, cppu::UnoType<OUString>::get(),   PROPERTY_NONE, 0},
                    { UNO_NAME_CURRENT_PRESENTATION, FIELD_PROP_PAR4, cppu::UnoType<OUString>::get(),  PROPERTY_NONE, 0},
                    COMMON_FLDTYP_PROPERTIES
                    { u"", 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aConditionedTextFieldPropMap;
            }
            break;
            case PROPERTY_MAP_FLDTYP_HIDDEN_TEXT :
            {
                static SfxItemPropertyMapEntry const aHiddenTextFieldPropMap  [] =
                {
                    { UNO_NAME_CONDITION,      FIELD_PROP_PAR1, cppu::UnoType<OUString>::get(),   PROPERTY_NONE, 0},
                    { UNO_NAME_CONTENT ,       FIELD_PROP_PAR2, cppu::UnoType<OUString>::get(),   PROPERTY_NONE, 0},
                    { UNO_NAME_IS_HIDDEN ,     FIELD_PROP_BOOL1, cppu::UnoType<bool>::get(),   PROPERTY_NONE, 0},
                    { UNO_NAME_CURRENT_PRESENTATION, FIELD_PROP_PAR4, cppu::UnoType<OUString>::get(),  PROPERTY_NONE, 0},
                    COMMON_FLDTYP_PROPERTIES
                    { u"", 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aHiddenTextFieldPropMap;
            }
            break;
            case PROPERTY_MAP_FLDTYP_ANNOTATION            :
            {
                static SfxItemPropertyMapEntry const aAnnotationFieldPropMap [] =
                {
                    { UNO_NAME_AUTHOR, FIELD_PROP_PAR1,    cppu::UnoType<OUString>::get(),   PROPERTY_NONE, 0},
                    { UNO_NAME_CONTENT,    FIELD_PROP_PAR2,    cppu::UnoType<OUString>::get(),   PROPERTY_NONE, 0},
                    { UNO_NAME_INITIALS,   FIELD_PROP_PAR3,    cppu::UnoType<OUString>::get(),   PROPERTY_NONE, 0},
                    { UNO_NAME_NAME,       FIELD_PROP_PAR4,    cppu::UnoType<OUString>::get(),   PROPERTY_NONE, 0},
                    { UNO_NAME_RESOLVED,       FIELD_PROP_BOOL1,    cppu::UnoType<bool>::get(),   PROPERTY_NONE, 0},
                    { UNO_NAME_DATE_TIME_VALUE,    FIELD_PROP_DATE_TIME,   cppu::UnoType<css::util::DateTime>::get(),    PROPERTY_NONE, 0},
                    { UNO_NAME_DATE,    FIELD_PROP_DATE,   cppu::UnoType<css::util::Date>::get(),    PROPERTY_NONE, 0},
                    { UNO_NAME_TEXT_RANGE, FIELD_PROP_TEXT, cppu::UnoType<css::uno::XInterface>::get(),  PropertyAttribute::READONLY,    0},
                    COMMON_FLDTYP_PROPERTIES
                    { u"", 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aAnnotationFieldPropMap;
            }
            break;
            case PROPERTY_MAP_FLDTYP_INPUT:
            {
                static SfxItemPropertyMapEntry const aInputFieldPropMap      [] =
                {
                    { UNO_NAME_CONTENT,    FIELD_PROP_PAR1, cppu::UnoType<OUString>::get(),   PROPERTY_NONE, 0},
                    { UNO_NAME_HINT,       FIELD_PROP_PAR2, cppu::UnoType<OUString>::get(),   PROPERTY_NONE, 0},
                    { UNO_NAME_HELP,       FIELD_PROP_PAR3, cppu::UnoType<OUString>::get(),   PROPERTY_NONE, 0},
                    { UNO_NAME_TOOLTIP,        FIELD_PROP_PAR4, cppu::UnoType<OUString>::get(),   PROPERTY_NONE, 0},
                    COMMON_FLDTYP_PROPERTIES
                    { u"", 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aInputFieldPropMap;
            }
            break;
            case PROPERTY_MAP_FLDTYP_MACRO                 :
            {
                static SfxItemPropertyMapEntry const aMacroFieldPropMap      [] =
                {
                    { UNO_NAME_HINT, FIELD_PROP_PAR2, cppu::UnoType<OUString>::get(),   PROPERTY_NONE, 0},
                    { UNO_NAME_MACRO_NAME,FIELD_PROP_PAR1, cppu::UnoType<OUString>::get(),   PROPERTY_NONE, 0},
                    { UNO_NAME_MACRO_LIBRARY,FIELD_PROP_PAR3, cppu::UnoType<OUString>::get(),PROPERTY_NONE, 0},
                    { UNO_NAME_SCRIPT_URL,FIELD_PROP_PAR4, cppu::UnoType<OUString>::get(),PROPERTY_NONE, 0},
                    COMMON_FLDTYP_PROPERTIES
                    { u"", 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aMacroFieldPropMap;
            }
            break;
            case PROPERTY_MAP_FLDTYP_DDE                   :
            {
                static SfxItemPropertyMapEntry const aDDEFieldPropMap            [] =
                {
                    COMMON_FLDTYP_PROPERTIES
                    { u"", 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aDDEFieldPropMap;
            }
            break;
            case PROPERTY_MAP_FLDTYP_DROPDOWN :
            {
                static SfxItemPropertyMapEntry const aDropDownMap            [] =
                {
                    { UNO_NAME_ITEMS, FIELD_PROP_STRINGS, cppu::UnoType< cppu::UnoSequenceType<OUString> >::get(), PROPERTY_NONE, 0},
                    { UNO_NAME_SELITEM, FIELD_PROP_PAR1, cppu::UnoType<OUString>::get(), PROPERTY_NONE, 0},
                    { UNO_NAME_NAME, FIELD_PROP_PAR2, cppu::UnoType<OUString>::get(), PROPERTY_NONE, 0},
                    { UNO_NAME_HELP, FIELD_PROP_PAR3, cppu::UnoType<OUString>::get(), PROPERTY_NONE, 0},
                    { UNO_NAME_TOOLTIP, FIELD_PROP_PAR4, cppu::UnoType<OUString>::get(), PROPERTY_NONE, 0},
                    COMMON_FLDTYP_PROPERTIES
                    { u"", 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aDropDownMap;
            }
            break;
            case PROPERTY_MAP_FLDTYP_HIDDEN_PARA           :
            {
                static SfxItemPropertyMapEntry const aHiddenParaFieldPropMap [] =
                {
                    { UNO_NAME_CONDITION,FIELD_PROP_PAR1, cppu::UnoType<OUString>::get(),   PROPERTY_NONE, 0},
                    { UNO_NAME_IS_HIDDEN ,  FIELD_PROP_BOOL1, cppu::UnoType<bool>::get(),   PROPERTY_NONE, 0},
                    COMMON_FLDTYP_PROPERTIES
                    { u"", 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aHiddenParaFieldPropMap;
            }
            break;
            case PROPERTY_MAP_FLDTYP_DOC_INFO              :
            {
                static SfxItemPropertyMapEntry const aDocInfoFieldPropMap        [] =
                {
                    { UNO_NAME_IS_FIXED,       FIELD_PROP_BOOL1,   cppu::UnoType<bool>::get(),     PROPERTY_NONE, 0},
                    { UNO_NAME_INFO_FORMAT,    FIELD_PROP_USHORT2, cppu::UnoType<sal_Int16>::get(), PROPERTY_NONE,    0},
                    { UNO_NAME_INFO_TYPE,  FIELD_PROP_USHORT1, cppu::UnoType<sal_Int16>::get(), PROPERTY_NONE,    0},
                    COMMON_FLDTYP_PROPERTIES
                    { u"", 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aDocInfoFieldPropMap;
            }
            break;
            case PROPERTY_MAP_FLDTYP_TEMPLATE_NAME         :
            {
                static SfxItemPropertyMapEntry const aTmplNameFieldPropMap   [] =
                {
                    { UNO_NAME_FILE_FORMAT, FIELD_PROP_FORMAT, cppu::UnoType<sal_Int16>::get(), PROPERTY_NONE,    0},
                    COMMON_FLDTYP_PROPERTIES
                    { u"", 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aTmplNameFieldPropMap;
            }
            break;
            case PROPERTY_MAP_FLDTYP_USER_EXT              :
            {
                static SfxItemPropertyMapEntry const aUsrExtFieldPropMap     [] =
                {
                    { UNO_NAME_CONTENT,            FIELD_PROP_PAR1,    cppu::UnoType<OUString>::get(),   PROPERTY_NONE, 0},
                    { UNO_NAME_CURRENT_PRESENTATION, FIELD_PROP_PAR1, cppu::UnoType<OUString>::get(),  PROPERTY_NONE, 0},
                    { UNO_NAME_IS_FIXED,           FIELD_PROP_BOOL1,   cppu::UnoType<bool>::get(),     PROPERTY_NONE, 0},
                    { UNO_NAME_USER_DATA_TYPE, FIELD_PROP_USHORT1, cppu::UnoType<sal_Int16>::get(),   PROPERTY_NONE, 0},
                    COMMON_FLDTYP_PROPERTIES
                    { u"", 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId]= aUsrExtFieldPropMap;
            }
            break;
            case PROPERTY_MAP_FLDTYP_REF_PAGE_SET          :
            {
                static SfxItemPropertyMapEntry const aRefPgSetFieldPropMap   [] =
                {
                    { UNO_NAME_OFFSET,     FIELD_PROP_USHORT1, cppu::UnoType<sal_Int16>::get(),   PROPERTY_NONE,  0},
                    { UNO_NAME_ON,     FIELD_PROP_BOOL1,   cppu::UnoType<bool>::get(),     PROPERTY_NONE, 0},
                    COMMON_FLDTYP_PROPERTIES
                    { u"", 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aRefPgSetFieldPropMap;
            }
            break;
            case PROPERTY_MAP_FLDTYP_REF_PAGE_GET          :
            {
                static SfxItemPropertyMapEntry const aRefPgGetFieldPropMap   [] =
                {
                    { UNO_NAME_CURRENT_PRESENTATION, FIELD_PROP_PAR1, cppu::UnoType<OUString>::get(),  PROPERTY_NONE, 0},
                    { UNO_NAME_NUMBERING_TYPE,     FIELD_PROP_USHORT1, cppu::UnoType<sal_Int16>::get(), PROPERTY_NONE,    0},
                    COMMON_FLDTYP_PROPERTIES
                    { u"", 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aRefPgGetFieldPropMap;
            }
            break;
            case PROPERTY_MAP_FLDTYP_JUMP_EDIT             :
            {
                static SfxItemPropertyMapEntry const aJumpEdtFieldPropMap        [] =
                {
                    { UNO_NAME_HINT,               FIELD_PROP_PAR1, cppu::UnoType<OUString>::get(),   PROPERTY_NONE, 0},
                    { UNO_NAME_PLACEHOLDER,        FIELD_PROP_PAR2, cppu::UnoType<OUString>::get(),   PROPERTY_NONE, 0},
                    { UNO_NAME_PLACEHOLDER_TYPE, FIELD_PROP_USHORT1, cppu::UnoType<sal_Int16>::get(),     PROPERTY_NONE,  0},
                    COMMON_FLDTYP_PROPERTIES
                    { u"", 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aJumpEdtFieldPropMap;
            }
            break;
            case PROPERTY_MAP_FLDTYP_SCRIPT                :
            {
                static SfxItemPropertyMapEntry const aScriptFieldPropMap     [] =
                {
                    { UNO_NAME_CONTENT,        FIELD_PROP_PAR2, cppu::UnoType<OUString>::get(),   PROPERTY_NONE, 0},
                    { UNO_NAME_SCRIPT_TYPE,    FIELD_PROP_PAR1, cppu::UnoType<OUString>::get(),   PROPERTY_NONE, 0},
                    { UNO_NAME_URL_CONTENT,    FIELD_PROP_BOOL1, cppu::UnoType<bool>::get(),       PROPERTY_NONE, 0},
                    COMMON_FLDTYP_PROPERTIES
                    { u"", 0, css::uno::Type(), 0, 0 }
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

                    { UNO_NAME_DATA_BASE_NAME , FIELD_PROP_PAR1, cppu::UnoType<OUString>::get(),   PROPERTY_NONE, 0},
                    { UNO_NAME_DATA_TABLE_NAME , FIELD_PROP_PAR2, cppu::UnoType<OUString>::get(),   PROPERTY_NONE, 0},
                    { UNO_NAME_CONDITION   ,     FIELD_PROP_PAR3, cppu::UnoType<OUString>::get(),   PROPERTY_NONE, 0},
                    { UNO_NAME_DATA_BASE_URL ,  FIELD_PROP_PAR1, cppu::UnoType<OUString>::get(),   PROPERTY_NONE, 0},
                    { UNO_NAME_DATA_COMMAND_TYPE, FIELD_PROP_SHORT1, cppu::UnoType<sal_Int32>::get(),   PROPERTY_NONE, 0},
                    COMMON_FLDTYP_PROPERTIES
                    { u"", 0, css::uno::Type(), 0, 0 }
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

                    { UNO_NAME_DATA_BASE_NAME ,  FIELD_PROP_PAR1, cppu::UnoType<OUString>::get(),   PROPERTY_NONE, 0},
                    { UNO_NAME_DATA_TABLE_NAME, FIELD_PROP_PAR2, cppu::UnoType<OUString>::get(),   PROPERTY_NONE, 0},
                    { UNO_NAME_CONDITION,         FIELD_PROP_PAR3, cppu::UnoType<OUString>::get(),   PROPERTY_NONE, 0},
                    { UNO_NAME_DATA_BASE_URL ,   FIELD_PROP_PAR1, cppu::UnoType<OUString>::get(),   PROPERTY_NONE, 0},
                    { UNO_NAME_DATA_COMMAND_TYPE, FIELD_PROP_SHORT1, cppu::UnoType<sal_Int32>::get(),   PROPERTY_NONE, 0},
                    { UNO_NAME_SET_NUMBER, FIELD_PROP_FORMAT, cppu::UnoType<sal_Int32>::get(), PROPERTY_NONE,  0},
                    COMMON_FLDTYP_PROPERTIES
                    { u"", 0, css::uno::Type(), 0, 0 }
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

                    { UNO_NAME_DATA_BASE_NAME , FIELD_PROP_PAR1, cppu::UnoType<OUString>::get(),   PROPERTY_NONE, 0},
                    { UNO_NAME_DATA_TABLE_NAME , FIELD_PROP_PAR2, cppu::UnoType<OUString>::get(),   PROPERTY_NONE, 0},
                    { UNO_NAME_DATA_BASE_URL ,  FIELD_PROP_PAR1, cppu::UnoType<OUString>::get(),   PROPERTY_NONE, 0},
                    { UNO_NAME_DATA_COMMAND_TYPE, FIELD_PROP_SHORT1, cppu::UnoType<sal_Int32>::get(),   PROPERTY_NONE, 0},
                    { UNO_NAME_NUMBERING_TYPE,       FIELD_PROP_USHORT1, cppu::UnoType<sal_Int16>::get(), PROPERTY_NONE,  0},
                    { UNO_NAME_SET_NUMBER, FIELD_PROP_FORMAT, cppu::UnoType<sal_Int32>::get(), PROPERTY_NONE,  0},
                    { UNO_NAME_IS_VISIBLE,       FIELD_PROP_BOOL2,   cppu::UnoType<bool>::get(),    PROPERTY_NONE, 0},
                    COMMON_FLDTYP_PROPERTIES
                    { u"", 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aDBSetNumFieldPropMap;
            }
            break;
            case PROPERTY_MAP_FLDTYP_DATABASE              :
            {
                static SfxItemPropertyMapEntry const aDBFieldPropMap         [] =
                {
                    { UNO_NAME_CONTENT,            FIELD_PROP_PAR1,    cppu::UnoType<OUString>::get(), PROPERTY_NONE, 0},
                    { UNO_NAME_CURRENT_PRESENTATION, FIELD_PROP_PAR1, cppu::UnoType<OUString>::get(),  PROPERTY_NONE, 0},
                    { UNO_NAME_FIELD_CODE,         FIELD_PROP_PAR2, cppu::UnoType<OUString>::get(),  PROPERTY_NONE, 0},
                    { UNO_NAME_IS_DATA_BASE_FORMAT,FIELD_PROP_BOOL1, cppu::UnoType<bool>::get()  , PROPERTY_NONE,0},
                    { UNO_NAME_NUMBER_FORMAT,      FIELD_PROP_FORMAT, cppu::UnoType<sal_Int32>::get(), PROPERTY_NONE, 0},
                    { UNO_NAME_IS_VISIBLE,       FIELD_PROP_BOOL2,   cppu::UnoType<bool>::get(),    PROPERTY_NONE, 0},
                    COMMON_FLDTYP_PROPERTIES
                    { u"", 0, css::uno::Type(), 0, 0 }
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

                    { UNO_NAME_DATA_BASE_NAME , FIELD_PROP_PAR1, cppu::UnoType<OUString>::get(),   PROPERTY_NONE, 0},
                    { UNO_NAME_DATA_TABLE_NAME , FIELD_PROP_PAR2, cppu::UnoType<OUString>::get(),   PROPERTY_NONE, 0},
                    { UNO_NAME_DATA_BASE_URL ,  FIELD_PROP_PAR1, cppu::UnoType<OUString>::get(),   PROPERTY_NONE, 0},
                    { UNO_NAME_DATA_COMMAND_TYPE, FIELD_PROP_SHORT1, cppu::UnoType<sal_Int32>::get(),   PROPERTY_NONE, 0},
                    { UNO_NAME_IS_VISIBLE,       FIELD_PROP_BOOL2,   cppu::UnoType<bool>::get(),    PROPERTY_NONE, 0},
                    COMMON_FLDTYP_PROPERTIES
                    { u"", 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aDBNameFieldPropMap;
            }
            break;
            case PROPERTY_MAP_FLDTYP_DOCSTAT:
            {
                static SfxItemPropertyMapEntry const aDocstatFieldPropMap        [] =
                {
                    { UNO_NAME_NUMBERING_TYPE,     FIELD_PROP_USHORT2, cppu::UnoType<sal_Int16>::get(), PROPERTY_NONE,    0},
                //  {UNO_NAME_STATISTIC_TYPE_ID,FIELD_PROP_USHORT1, cppu::UnoType<sal_Int16>::get(),   PROPERTY_NONE,  0},
                    COMMON_FLDTYP_PROPERTIES
                    { u"", 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aDocstatFieldPropMap;
            }
            break;
            case PROPERTY_MAP_FLDTYP_DOCINFO_AUTHOR:
            {
                static SfxItemPropertyMapEntry const aDocInfoAuthorPropMap           [] =
                {
                    { UNO_NAME_AUTHOR, FIELD_PROP_PAR1,    cppu::UnoType<OUString>::get(), PROPERTY_NONE, 0},
                    { UNO_NAME_CURRENT_PRESENTATION, FIELD_PROP_PAR3, cppu::UnoType<OUString>::get(),  PROPERTY_NONE, 0},
                    { UNO_NAME_IS_FIXED,   FIELD_PROP_BOOL1,   cppu::UnoType<bool>::get()  , PROPERTY_NONE,0},
                    COMMON_FLDTYP_PROPERTIES
                    { u"", 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aDocInfoAuthorPropMap;
            }
            break;
            case PROPERTY_MAP_FLDTYP_DOCINFO_DATE_TIME:
            {
                static SfxItemPropertyMapEntry const aDocInfoDateTimePropMap         [] =
                {
                    { UNO_NAME_CURRENT_PRESENTATION, FIELD_PROP_PAR3, cppu::UnoType<OUString>::get(),  PROPERTY_NONE, 0},
                    { UNO_NAME_DATE_TIME_VALUE,        FIELD_PROP_DOUBLE,  cppu::UnoType<double>::get(), PropertyAttribute::READONLY, 0},
                    { UNO_NAME_IS_DATE,    FIELD_PROP_BOOL2,   cppu::UnoType<bool>::get()  , PROPERTY_NONE,0},
                    { UNO_NAME_NUMBER_FORMAT,FIELD_PROP_FORMAT,    cppu::UnoType<sal_Int32>::get(), PROPERTY_NONE,    0},
                    { UNO_NAME_IS_FIXED,       FIELD_PROP_BOOL1,   cppu::UnoType<bool>::get()  , PROPERTY_NONE,    0},
                    { UNO_NAME_IS_FIXED_LANGUAGE, FIELD_PROP_BOOL4, cppu::UnoType<bool>::get(), PROPERTY_NONE,    0},
                    { u"", 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aDocInfoDateTimePropMap;
            }
            break;
            case PROPERTY_MAP_FLDTYP_DOCINFO_EDIT_TIME     :
            {
                static SfxItemPropertyMapEntry const aDocInfoEditTimePropMap         [] =
                {
                    { UNO_NAME_CURRENT_PRESENTATION, FIELD_PROP_PAR3, cppu::UnoType<OUString>::get(),  PROPERTY_NONE, 0},
                    { UNO_NAME_DATE_TIME_VALUE,        FIELD_PROP_DOUBLE,  cppu::UnoType<double>::get(), PropertyAttribute::READONLY, 0},
                    { UNO_NAME_NUMBER_FORMAT,FIELD_PROP_FORMAT,    cppu::UnoType<sal_Int32>::get(), PROPERTY_NONE,    0},
                    { UNO_NAME_IS_FIXED,       FIELD_PROP_BOOL1,   cppu::UnoType<bool>::get()  , PROPERTY_NONE,    0},
                    { UNO_NAME_IS_FIXED_LANGUAGE, FIELD_PROP_BOOL4, cppu::UnoType<bool>::get(), PROPERTY_NONE,    0},
                    COMMON_FLDTYP_PROPERTIES
                    { u"", 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aDocInfoEditTimePropMap;
            }
            break;
            case PROPERTY_MAP_FLDTYP_DOCINFO_MISC:
            {
                static SfxItemPropertyMapEntry const aDocInfoStringContentPropMap            [] =
                {
                    { UNO_NAME_CONTENT,    FIELD_PROP_PAR1,    cppu::UnoType<OUString>::get(), PROPERTY_NONE, 0},
                    { UNO_NAME_CURRENT_PRESENTATION, FIELD_PROP_PAR3, cppu::UnoType<OUString>::get(),  PROPERTY_NONE, 0},
                    { UNO_NAME_IS_FIXED,   FIELD_PROP_BOOL1,   cppu::UnoType<bool>::get()  , PROPERTY_NONE,0},
                    COMMON_FLDTYP_PROPERTIES
                    { u"", 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aDocInfoStringContentPropMap;
            }
            break;
            case PROPERTY_MAP_FLDTYP_DOCINFO_CUSTOM:
            {
                static SfxItemPropertyMapEntry const aDocInfoCustomPropMap           [] =
                {
                    { UNO_NAME_NAME,   FIELD_PROP_PAR4,    cppu::UnoType<OUString>::get(), PROPERTY_NONE, 0},
                    { UNO_NAME_CURRENT_PRESENTATION, FIELD_PROP_PAR3, cppu::UnoType<OUString>::get(),  PROPERTY_NONE, 0},
                    { UNO_NAME_IS_FIXED,   FIELD_PROP_BOOL1,   cppu::UnoType<bool>::get()  , PROPERTY_NONE,0},
                    { UNO_NAME_NUMBER_FORMAT, FIELD_PROP_FORMAT,   cppu::UnoType<sal_Int32>::get(), PROPERTY_NONE,    0},
                    { UNO_NAME_IS_FIXED_LANGUAGE, FIELD_PROP_BOOL4, cppu::UnoType<bool>::get(), PROPERTY_NONE,    0},
                    COMMON_FLDTYP_PROPERTIES
                    { u"", 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aDocInfoCustomPropMap;
            }
            break;
            case PROPERTY_MAP_FLDTYP_DOCINFO_REVISION          :
            {
                static SfxItemPropertyMapEntry const aDocInfoRevisionPropMap [] =
                {
                    { UNO_NAME_CURRENT_PRESENTATION, FIELD_PROP_PAR3, cppu::UnoType<OUString>::get(),  PROPERTY_NONE, 0},
                    { UNO_NAME_REVISION,   FIELD_PROP_USHORT1, cppu::UnoType<sal_Int16>::get(), PROPERTY_NONE,    0},
                    { UNO_NAME_IS_FIXED,   FIELD_PROP_BOOL1,   cppu::UnoType<bool>::get()  , PROPERTY_NONE,0},
                    COMMON_FLDTYP_PROPERTIES
                    { u"", 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aDocInfoRevisionPropMap;
            }
            break;
            case PROPERTY_MAP_FLDTYP_COMBINED_CHARACTERS:
            {
                static SfxItemPropertyMapEntry const aCombinedCharactersPropMap[] =
                {
                    { UNO_NAME_CONTENT, FIELD_PROP_PAR1, cppu::UnoType<OUString>::get(),  PROPERTY_NONE, 0},
                    COMMON_FLDTYP_PROPERTIES
                    { u"", 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aCombinedCharactersPropMap;
            }
            break;
            case PROPERTY_MAP_FLDTYP_TABLE_FORMULA:
            {
                static SfxItemPropertyMapEntry const aTableFormulaPropMap[] =
                {
                    { UNO_NAME_CURRENT_PRESENTATION, FIELD_PROP_PAR1, cppu::UnoType<OUString>::get(),  PROPERTY_NONE, 0},
                    { UNO_NAME_CONTENT, FIELD_PROP_PAR2, cppu::UnoType<OUString>::get(),  PROPERTY_NONE, 0},
                    { UNO_NAME_IS_SHOW_FORMULA, FIELD_PROP_BOOL1,  cppu::UnoType<bool>::get(), PROPERTY_NONE, 0},
                    { UNO_NAME_NUMBER_FORMAT, FIELD_PROP_FORMAT,   cppu::UnoType<sal_Int32>::get(), PROPERTY_NONE, 0},
                    COMMON_FLDTYP_PROPERTIES
                    { u"", 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aTableFormulaPropMap;
            }
            break;
            case PROPERTY_MAP_FLDTYP_DUMMY_0:
            {
                static SfxItemPropertyMapEntry const aEmptyPropMap           [] =
                {
                    COMMON_FLDTYP_PROPERTIES
                    { u"", 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aEmptyPropMap;
            }
            break;
            case PROPERTY_MAP_FLDMSTR_USER :
            {
                static SfxItemPropertyMapEntry const aUserFieldTypePropMap[] =
                {
                    { UNO_NAME_DEPENDENT_TEXT_FIELDS,  FIELD_PROP_PROP_SEQ,    cppu::UnoType< cppu::UnoSequenceType<css::text::XDependentTextField> >::get(), PropertyAttribute::READONLY, 0},
                    { UNO_NAME_IS_EXPRESSION,      FIELD_PROP_BOOL1,  cppu::UnoType<bool>::get(), PROPERTY_NONE,   0},
                    { UNO_NAME_NAME,               FIELD_PROP_PAR1,  cppu::UnoType<OUString>::get(), PropertyAttribute::MAYBEVOID, 0},
                    { UNO_NAME_VALUE,          FIELD_PROP_DOUBLE,  cppu::UnoType<double>::get(), PROPERTY_NONE,   0},
                    { UNO_NAME_CONTENT,            FIELD_PROP_PAR2,    cppu::UnoType<OUString>::get(), PROPERTY_NONE, 0},
                    { UNO_NAME_INSTANCE_NAME,      FIELD_PROP_PAR3,    cppu::UnoType<OUString>::get(), PropertyAttribute::READONLY, 0},
                    { u"", 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aUserFieldTypePropMap;
            }
            break;
            case PROPERTY_MAP_FLDMSTR_DDE       :
            {
                static SfxItemPropertyMapEntry const aDDEFieldTypePropMap[] =
                {
                    { UNO_NAME_DDE_COMMAND_ELEMENT, FIELD_PROP_PAR2,  cppu::UnoType<OUString>::get(), PROPERTY_NONE, 0},
                    { UNO_NAME_DDE_COMMAND_FILE, FIELD_PROP_PAR4,  cppu::UnoType<OUString>::get(), PROPERTY_NONE, 0},
                    { UNO_NAME_DDE_COMMAND_TYPE, FIELD_PROP_SUBTYPE,   cppu::UnoType<OUString>::get(), PROPERTY_NONE, 0},
                    { UNO_NAME_DEPENDENT_TEXT_FIELDS,  FIELD_PROP_PROP_SEQ,    cppu::UnoType< cppu::UnoSequenceType<css::text::XDependentTextField> >::get(), PropertyAttribute::READONLY, 0},
                    { UNO_NAME_IS_AUTOMATIC_UPDATE, FIELD_PROP_BOOL1,  cppu::UnoType<bool>::get(), PROPERTY_NONE,    0},
                    { UNO_NAME_NAME,               FIELD_PROP_PAR1,  cppu::UnoType<OUString>::get(), PROPERTY_NONE, 0},
                    { UNO_NAME_INSTANCE_NAME,      FIELD_PROP_PAR3,    cppu::UnoType<OUString>::get(), PropertyAttribute::READONLY, 0},
                    { UNO_NAME_CONTENT,            FIELD_PROP_PAR5,    cppu::UnoType<OUString>::get(),   PROPERTY_NONE, 0},
                    { u"", 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aDDEFieldTypePropMap;
            }
            break;
            case PROPERTY_MAP_FLDMSTR_SET_EXP     :
            {
                static SfxItemPropertyMapEntry const aSetExpFieldTypePropMap[] =
                {
                    { UNO_NAME_CHAPTER_NUMBERING_LEVEL,FIELD_PROP_SHORT1,  cppu::UnoType<sal_Int8>::get(), PROPERTY_NONE, 0},
                    { UNO_NAME_DEPENDENT_TEXT_FIELDS,  FIELD_PROP_PROP_SEQ,    cppu::UnoType< cppu::UnoSequenceType<css::text::XDependentTextField> >::get(), PropertyAttribute::READONLY, 0},
                    { UNO_NAME_NAME,               FIELD_PROP_PAR1,  cppu::UnoType<OUString>::get(), PROPERTY_NONE, 0},
                    { UNO_NAME_NUMBERING_SEPARATOR, FIELD_PROP_PAR2,   cppu::UnoType<OUString>::get(), PROPERTY_NONE, 0},
                    { UNO_NAME_SUB_TYPE,           FIELD_PROP_SUBTYPE, cppu::UnoType<sal_Int16>::get(), PROPERTY_NONE,    0},
                    { UNO_NAME_INSTANCE_NAME,      FIELD_PROP_PAR3,    cppu::UnoType<OUString>::get(), PropertyAttribute::READONLY, 0},
                    { u"", 0, css::uno::Type(), 0, 0 }
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

                    { UNO_NAME_DATA_BASE_NAME ,  FIELD_PROP_PAR1, cppu::UnoType<OUString>::get(),   PROPERTY_NONE, 0},
                    { UNO_NAME_NAME,              FIELD_PROP_PAR3,  cppu::UnoType<OUString>::get(), PropertyAttribute::MAYBEVOID, 0},
                    { UNO_NAME_DATA_TABLE_NAME, FIELD_PROP_PAR2, cppu::UnoType<OUString>::get(),   PROPERTY_NONE, 0},
                    { UNO_NAME_DATA_COLUMN_NAME, FIELD_PROP_PAR3, cppu::UnoType<OUString>::get(),   PROPERTY_NONE, 0},
                    { UNO_NAME_INSTANCE_NAME,     FIELD_PROP_PAR4, cppu::UnoType<OUString>::get(), PropertyAttribute::READONLY, 0},
                    { UNO_NAME_DATA_BASE_URL ,   FIELD_PROP_PAR1, cppu::UnoType<OUString>::get(),   PROPERTY_NONE, 0},
                    { UNO_NAME_DATA_COMMAND_TYPE, FIELD_PROP_SHORT1, cppu::UnoType<sal_Int32>::get(),   PROPERTY_NONE, 0},
                    { UNO_NAME_DEPENDENT_TEXT_FIELDS,  FIELD_PROP_PROP_SEQ,    cppu::UnoType< cppu::UnoSequenceType<css::text::XDependentTextField> >::get(), PropertyAttribute::READONLY, 0},
                    { u"", 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aDBFieldTypePropMap;
            }
            break;
            case PROPERTY_MAP_FLDMSTR_DUMMY0      :
            {
                static SfxItemPropertyMapEntry const aStandardFieldMasterMap[] =
                {
                    { UNO_NAME_DEPENDENT_TEXT_FIELDS,  0,  cppu::UnoType< cppu::UnoSequenceType<css::text::XDependentTextField> >::get(), PropertyAttribute::READONLY, 0},
                    { UNO_NAME_NAME,               0,  cppu::UnoType<OUString>::get(), PROPERTY_NONE, 0},
                    { UNO_NAME_INSTANCE_NAME,      0,  cppu::UnoType<OUString>::get(), PropertyAttribute::READONLY, 0},
                    { u"", 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aStandardFieldMasterMap;
            }
            break;
            case PROPERTY_MAP_FLDTYP_BIBLIOGRAPHY:
            {
                static SfxItemPropertyMapEntry const aBibliographyFieldMap[] =
                {
                    { UNO_NAME_FIELDS    , FIELD_PROP_PROP_SEQ, cppu::UnoType< cppu::UnoSequenceType<css::beans::PropertyValue> >::get(),PROPERTY_NONE, 0},
                    COMMON_FLDTYP_PROPERTIES
                    { u"", 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aBibliographyFieldMap;
            }
            break;
            case PROPERTY_MAP_FLDMSTR_BIBLIOGRAPHY:
            {
                static SfxItemPropertyMapEntry const aBibliographyFieldMasterMap[] =
                {
                    { UNO_NAME_BRACKET_BEFORE , FIELD_PROP_PAR1, cppu::UnoType<OUString>::get(),               PROPERTY_NONE, 0},
                    { UNO_NAME_BRACKET_AFTER , FIELD_PROP_PAR2, cppu::UnoType<OUString>::get(),               PROPERTY_NONE, 0},
                    { UNO_NAME_IS_NUMBER_ENTRIES , FIELD_PROP_BOOL1, cppu::UnoType<bool>::get(),                    PROPERTY_NONE, 0},
                    { UNO_NAME_IS_SORT_BY_POSITION , FIELD_PROP_BOOL2, cppu::UnoType<bool>::get(),                    PROPERTY_NONE, 0},
                    { UNO_NAME_LOCALE,            FIELD_PROP_LOCALE,  cppu::UnoType<css::lang::Locale>::get()  , PROPERTY_NONE,     0},
                    { UNO_NAME_SORT_ALGORITHM,    FIELD_PROP_PAR3,  cppu::UnoType<OUString>::get(), PROPERTY_NONE,     0},
                    { UNO_NAME_SORT_KEYS , FIELD_PROP_PROP_SEQ, cppu::UnoType< cppu::UnoSequenceType<css::beans::PropertyValues> >::get(),   PROPERTY_NONE, 0},
                    { UNO_NAME_INSTANCE_NAME,      FIELD_PROP_PAR4,    cppu::UnoType<OUString>::get(), PropertyAttribute::READONLY, 0},
                    { u"", 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aBibliographyFieldMasterMap;
            }
            break;
            case PROPERTY_MAP_TEXT :
            {
                static SfxItemPropertyMapEntry const aTextMap[] =
                {
                    REDLINE_NODE_PROPERTIES
                    { u"", 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aTextMap;
            }
            break;
            case PROPERTY_MAP_MAILMERGE :
            {
                static SfxItemPropertyMapEntry const aMailMergeMap[] =
                {
                    { UNO_NAME_SELECTION,             WID_SELECTION,              cppu::UnoType< cppu::UnoSequenceType<css::uno::Any> >::get(),      PROPERTY_NONE, 0},
                    { UNO_NAME_RESULT_SET,            WID_RESULT_SET,             cppu::UnoType<css::sdbc::XResultSet>::get(), PROPERTY_NONE, 0},
                    { UNO_NAME_CONNECTION,            WID_CONNECTION,             cppu::UnoType<css::sdbc::XConnection>::get(), PROPERTY_NONE, 0},
                    { UNO_NAME_MODEL,                 WID_MODEL,                  cppu::UnoType<css::frame::XModel>::get(),    PropertyAttribute::READONLY, 0},
                    { UNO_NAME_DATA_SOURCE_NAME,      WID_DATA_SOURCE_NAME,       cppu::UnoType<OUString>::get(),    PROPERTY_NONE, 0},
                    { UNO_NAME_DAD_COMMAND,           WID_DATA_COMMAND,           cppu::UnoType<OUString>::get(),    PROPERTY_NONE, 0},
                    { UNO_NAME_FILTER,                WID_FILTER,                 cppu::UnoType<OUString>::get(),    PROPERTY_NONE, 0},
                    { UNO_NAME_DOCUMENT_URL,          WID_DOCUMENT_URL,           cppu::UnoType<OUString>::get(),    PROPERTY_NONE, 0},
                    { UNO_NAME_OUTPUT_URL,            WID_OUTPUT_URL,             cppu::UnoType<OUString>::get(),    PROPERTY_NONE, 0},
                    { UNO_NAME_DAD_COMMAND_TYPE,      WID_DATA_COMMAND_TYPE,      cppu::UnoType<sal_Int32>::get(),       PROPERTY_NONE, 0},
                    { UNO_NAME_OUTPUT_TYPE,           WID_OUTPUT_TYPE,            cppu::UnoType<sal_Int16>::get(),       PROPERTY_NONE, 0},
                    { UNO_NAME_ESCAPE_PROCESSING,     WID_ESCAPE_PROCESSING,      cppu::UnoType<bool>::get(),     PROPERTY_NONE, 0},
                    { UNO_NAME_SINGLE_PRINT_JOBS,     WID_SINGLE_PRINT_JOBS,      cppu::UnoType<bool>::get(),     PROPERTY_NONE, 0},
                    { UNO_NAME_FILE_NAME_FROM_COLUMN, WID_FILE_NAME_FROM_COLUMN,  cppu::UnoType<bool>::get(),     PROPERTY_NONE, 0},
                    { UNO_NAME_FILE_NAME_PREFIX,      WID_FILE_NAME_PREFIX,       cppu::UnoType<OUString>::get(),    PROPERTY_NONE, 0},
                    { UNO_NAME_SUBJECT,               WID_MAIL_SUBJECT,           cppu::UnoType<OUString>::get(),        PROPERTY_NONE, 0},
                    { UNO_NAME_ADDRESS_FROM_COLUMN,   WID_ADDRESS_FROM_COLUMN,    cppu::UnoType<OUString>::get(),        PROPERTY_NONE, 0},
                    { UNO_NAME_SEND_AS_HTML,          WID_SEND_AS_HTML,           cppu::UnoType<bool>::get(),         PROPERTY_NONE, 0},
                    { UNO_NAME_SEND_AS_ATTACHMENT,    WID_SEND_AS_ATTACHMENT,     cppu::UnoType<bool>::get(),         PROPERTY_NONE, 0},
                    { UNO_NAME_MAIL_BODY,             WID_MAIL_BODY,              cppu::UnoType<OUString>::get(),        PROPERTY_NONE, 0},
                    { UNO_NAME_ATTACHMENT_NAME,       WID_ATTACHMENT_NAME,        cppu::UnoType<OUString>::get(),        PROPERTY_NONE, 0},
                    { UNO_NAME_ATTACHMENT_FILTER,     WID_ATTACHMENT_FILTER,      cppu::UnoType<OUString>::get(),        PROPERTY_NONE, 0},
                    { UNO_NAME_PRINT_OPTIONS,         WID_PRINT_OPTIONS,          cppu::UnoType< cppu::UnoSequenceType<css::beans::PropertyValue> >::get(),  PROPERTY_NONE, 0},
                    { UNO_NAME_SAVE_AS_SINGLE_FILE,   WID_SAVE_AS_SINGLE_FILE,    cppu::UnoType<bool>::get(),         PROPERTY_NONE, 0},
                    { UNO_NAME_SAVE_FILTER,           WID_SAVE_FILTER,            cppu::UnoType<OUString>::get(),        PROPERTY_NONE, 0},
                    { UNO_NAME_SAVE_FILTER_OPTIONS,   WID_SAVE_FILTER_OPTIONS,    cppu::UnoType<OUString>::get(),        PROPERTY_NONE, 0},
                    { UNO_NAME_SAVE_FILTER_DATA,      WID_SAVE_FILTER_DATA,       cppu::UnoType< cppu::UnoSequenceType<css::beans::PropertyValue> >::get(),        PROPERTY_NONE, 0},
                    { UNO_NAME_COPIES_TO,             WID_COPIES_TO,              cppu::UnoType< cppu::UnoSequenceType<OUString> >::get(),       PROPERTY_NONE, 0},
                    { UNO_NAME_BLIND_COPIES_TO,       WID_BLIND_COPIES_TO,        cppu::UnoType< cppu::UnoSequenceType<OUString> >::get(),       PROPERTY_NONE, 0},
                    { UNO_NAME_IN_SERVER_PASSWORD,     WID_IN_SERVER_PASSWORD,     cppu::UnoType<OUString>::get(),    PROPERTY_NONE, 0},
                    { UNO_NAME_OUT_SERVER_PASSWORD,    WID_OUT_SERVER_PASSWORD,    cppu::UnoType<OUString>::get(),    PROPERTY_NONE, 0},
                    { u"", 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aMailMergeMap;
            }
            break;
            case PROPERTY_MAP_TEXT_VIEW :
            {
                static SfxItemPropertyMapEntry pTextViewMap[] =
                {
                    { UNO_NAME_PAGE_COUNT,             WID_PAGE_COUNT,             cppu::UnoType<sal_Int32>::get(),   PropertyAttribute::READONLY, 0},
                    { UNO_NAME_LINE_COUNT,             WID_LINE_COUNT,             cppu::UnoType<sal_Int32>::get(),   PropertyAttribute::READONLY, 0},
                    { UNO_NAME_IS_CONSTANT_SPELLCHECK, WID_IS_CONSTANT_SPELLCHECK, cppu::UnoType<bool>::get(), PROPERTY_NONE, 0},
                    { UNO_NAME_IS_HIDE_SPELL_MARKS,    WID_IS_HIDE_SPELL_MARKS,    cppu::UnoType<bool>::get(), PROPERTY_NONE, 0},  // deprecated #i91949
                    { u"", 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = pTextViewMap;
            }
            break;
            case PROPERTY_MAP_CHART2_DATA_SEQUENCE :
            {
                static SfxItemPropertyMapEntry const aChart2DataSequenceMap[] =
                {
                    { UNO_NAME_ROLE, 0, cppu::UnoType<OUString>::get(),   PROPERTY_NONE, 0 },
                    { u"", 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aChart2DataSequenceMap;
            }
            break;
            case PROPERTY_MAP_METAFIELD:
            {
                static SfxItemPropertyMapEntry const aMetaFieldMap[] =
                {
                    { UNO_NAME_NUMBER_FORMAT, 0,
                        cppu::UnoType<sal_Int32>::get(), PROPERTY_NONE, 0 },
                    { UNO_NAME_IS_FIXED_LANGUAGE, 0,
                        cppu::UnoType<bool>::get(), PROPERTY_NONE, 0 },
                    { u"", 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aMetaFieldMap;
            }
            break;
            case PROPERTY_MAP_TABLE_STYLE:
            {
                static SfxItemPropertyMapEntry const aTableStyleMap[] =
                {
                    { UNO_NAME_TABLE_FIRST_ROW_END_COLUMN,   0, cppu::UnoType<OUString>::get(), PROPERTY_NONE, 0 },
                    { UNO_NAME_TABLE_FIRST_ROW_START_COLUMN, 0, cppu::UnoType<OUString>::get(), PROPERTY_NONE, 0 },
                    { UNO_NAME_TABLE_LAST_ROW_END_COLUMN,    0, cppu::UnoType<OUString>::get(), PROPERTY_NONE, 0 },
                    { UNO_NAME_TABLE_LAST_ROW_START_COLUMN,  0, cppu::UnoType<OUString>::get(), PROPERTY_NONE, 0 },
                    { UNO_NAME_DISPLAY_NAME,                 0, cppu::UnoType<OUString>::get(), PropertyAttribute::READONLY, 0 },
                    { u"", 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aTableStyleMap;
            }
            break;
            case PROPERTY_MAP_CELL_STYLE:
            {
                static SfxItemPropertyMapEntry const aCellStyleMap[] =
                {
                    // SvxBrushItem
                    { UNO_NAME_BACK_COLOR,             RES_BACKGROUND,    cppu::UnoType<sal_Int32>::get(),               PROPERTY_NONE,  0                                    },
                    // SvxBoxItem
                    { UNO_NAME_LEFT_BORDER,            RES_BOX,           cppu::UnoType<css::table::BorderLine>::get(),  PROPERTY_NONE,  LEFT_BORDER|CONVERT_TWIPS            },
                    { UNO_NAME_RIGHT_BORDER,           RES_BOX,           cppu::UnoType<css::table::BorderLine>::get(),  PROPERTY_NONE,  RIGHT_BORDER|CONVERT_TWIPS           },
                    { UNO_NAME_TOP_BORDER,             RES_BOX,           cppu::UnoType<css::table::BorderLine>::get(),  PROPERTY_NONE,  TOP_BORDER|CONVERT_TWIPS             },
                    { UNO_NAME_BOTTOM_BORDER,          RES_BOX,           cppu::UnoType<css::table::BorderLine>::get(),  PROPERTY_NONE,  BOTTOM_BORDER|CONVERT_TWIPS          },
                    { UNO_NAME_BORDER_DISTANCE,        RES_BOX,           cppu::UnoType<sal_Int32>::get(),               PROPERTY_NONE,  BORDER_DISTANCE|CONVERT_TWIPS        },
                    { UNO_NAME_LEFT_BORDER_DISTANCE,   RES_BOX,           cppu::UnoType<sal_Int32>::get(),               PROPERTY_NONE,  LEFT_BORDER_DISTANCE  |CONVERT_TWIPS },
                    { UNO_NAME_RIGHT_BORDER_DISTANCE,  RES_BOX,           cppu::UnoType<sal_Int32>::get(),               PROPERTY_NONE,  RIGHT_BORDER_DISTANCE |CONVERT_TWIPS },
                    { UNO_NAME_TOP_BORDER_DISTANCE,    RES_BOX,           cppu::UnoType<sal_Int32>::get(),               PROPERTY_NONE,  TOP_BORDER_DISTANCE   |CONVERT_TWIPS },
                    { UNO_NAME_BOTTOM_BORDER_DISTANCE, RES_BOX,           cppu::UnoType<sal_Int32>::get(),               PROPERTY_NONE,  BOTTOM_BORDER_DISTANCE|CONVERT_TWIPS },
                    // SwFormatVertOrient
                    { UNO_NAME_VERT_ORIENT,            RES_VERT_ORIENT,   cppu::UnoType<sal_Int16>::get(),               PROPERTY_NONE,  MID_VERTORIENT_ORIENT                },
                    // SvxFrameDirectionItem
                    { UNO_NAME_WRITING_MODE,           RES_FRAMEDIR,      cppu::UnoType<sal_Int16>::get(),               PROPERTY_NONE,  0                                    },
                    // SvNumberformat
                    { UNO_NAME_NUMBER_FORMAT,          RES_BOXATR_FORMAT, cppu::UnoType<sal_Int32>::get(),PropertyAttribute::MAYBEVOID,  0                                    },
                    // SvxAdjustItem
                    { UNO_NAME_PARA_ADJUST,            RES_PARATR_ADJUST, cppu::UnoType<sal_Int16>::get(),PropertyAttribute::MAYBEVOID,  MID_PARA_ADJUST                      },
                    // SvxColorItem
                    { UNO_NAME_CHAR_COLOR,             RES_CHRATR_COLOR,  cppu::UnoType<sal_Int32>::get(),               PROPERTY_NONE,  0                                    },
                    // SvxShadowedItem
                    { UNO_NAME_CHAR_SHADOWED,          RES_CHRATR_SHADOWED,    cppu::UnoType<bool>::get(),               PROPERTY_NONE,  0                                    },
                    // SvxContouredItem
                    { UNO_NAME_CHAR_CONTOURED,         RES_CHRATR_CONTOUR,     cppu::UnoType<bool>::get(),               PROPERTY_NONE,  0                                    },
                    // SvxCrossedOutItem
                    { UNO_NAME_CHAR_STRIKEOUT,     RES_CHRATR_CROSSEDOUT, cppu::UnoType<sal_Int16>::get(),PropertyAttribute::MAYBEVOID,  MID_CROSS_OUT                        },
                    // SvxUnderlineItem
                    { UNO_NAME_CHAR_UNDERLINE,      RES_CHRATR_UNDERLINE, cppu::UnoType<sal_Int16>::get(),               PROPERTY_NONE,  MID_TL_STYLE                         },
                    { UNO_NAME_CHAR_UNDERLINE_COLOR, RES_CHRATR_UNDERLINE,cppu::UnoType<sal_Int32>::get(),               PROPERTY_NONE,  MID_TL_COLOR                         },
                    { UNO_NAME_CHAR_UNDERLINE_HAS_COLOR, RES_CHRATR_UNDERLINE, cppu::UnoType<bool>::get(),               PROPERTY_NONE,  MID_TL_HASCOLOR                      },
                    // standard font
                    // SvxFontHeightItem
                    { UNO_NAME_CHAR_HEIGHT,            RES_CHRATR_FONTSIZE,   cppu::UnoType<float>::get(),PropertyAttribute::MAYBEVOID,  MID_FONTHEIGHT|CONVERT_TWIPS         },
                    // SvxWeightItem
                    { UNO_NAME_CHAR_WEIGHT,            RES_CHRATR_WEIGHT,     cppu::UnoType<float>::get(),PropertyAttribute::MAYBEVOID,  MID_WEIGHT                           },
                    // SvxPostureItem
                    { UNO_NAME_CHAR_POSTURE, RES_CHRATR_POSTURE, cppu::UnoType<css::awt::FontSlant>::get(),PropertyAttribute::MAYBEVOID, MID_POSTURE                          },
                    // SvxFontItem
                    { UNO_NAME_CHAR_FONT_NAME,         RES_CHRATR_FONT,   cppu::UnoType<OUString>::get(), PropertyAttribute::MAYBEVOID,  MID_FONT_FAMILY_NAME                 },
                    { UNO_NAME_CHAR_FONT_STYLE_NAME,   RES_CHRATR_FONT,   cppu::UnoType<OUString>::get(), PropertyAttribute::MAYBEVOID,  MID_FONT_STYLE_NAME                  },
                    { UNO_NAME_CHAR_FONT_FAMILY,       RES_CHRATR_FONT,  cppu::UnoType<sal_Int16>::get(), PropertyAttribute::MAYBEVOID,  MID_FONT_FAMILY                      },
                    { UNO_NAME_CHAR_FONT_CHAR_SET,     RES_CHRATR_FONT,  cppu::UnoType<sal_Int16>::get(), PropertyAttribute::MAYBEVOID,  MID_FONT_CHAR_SET                    },
                    { UNO_NAME_CHAR_FONT_PITCH,        RES_CHRATR_FONT,  cppu::UnoType<sal_Int16>::get(), PropertyAttribute::MAYBEVOID,  MID_FONT_PITCH                       },
                    // cjk font
                    { UNO_NAME_CHAR_HEIGHT_ASIAN,             RES_CHRATR_CJK_FONTSIZE,        cppu::UnoType<float>::get(),        PropertyAttribute::MAYBEVOID, MID_FONTHEIGHT|CONVERT_TWIPS },
                    { UNO_NAME_CHAR_WEIGHT_ASIAN,             RES_CHRATR_CJK_WEIGHT,          cppu::UnoType<float>::get(),        PropertyAttribute::MAYBEVOID, MID_WEIGHT                   },
                    { UNO_NAME_CHAR_POSTURE_ASIAN,            RES_CHRATR_CJK_POSTURE,  cppu::UnoType<css::awt::FontSlant>::get(), PropertyAttribute::MAYBEVOID, MID_POSTURE                  },
                    { UNO_NAME_CHAR_FONT_NAME_ASIAN,          RES_CHRATR_CJK_FONT,         cppu::UnoType<OUString>::get(),        PropertyAttribute::MAYBEVOID, MID_FONT_FAMILY_NAME         },
                    { UNO_NAME_CHAR_FONT_STYLE_NAME_ASIAN,    RES_CHRATR_CJK_FONT,         cppu::UnoType<OUString>::get(),        PropertyAttribute::MAYBEVOID, MID_FONT_STYLE_NAME          },
                    { UNO_NAME_CHAR_FONT_FAMILY_ASIAN,        RES_CHRATR_CJK_FONT,        cppu::UnoType<sal_Int16>::get(),        PropertyAttribute::MAYBEVOID, MID_FONT_FAMILY              },
                    { UNO_NAME_CHAR_FONT_CHAR_SET_ASIAN,      RES_CHRATR_CJK_FONT,        cppu::UnoType<sal_Int16>::get(),        PropertyAttribute::MAYBEVOID, MID_FONT_CHAR_SET            },
                    { UNO_NAME_CHAR_FONT_PITCH_ASIAN,         RES_CHRATR_CJK_FONT,        cppu::UnoType<sal_Int16>::get(),        PropertyAttribute::MAYBEVOID, MID_FONT_PITCH               },
                    // ctl font
                    { UNO_NAME_CHAR_HEIGHT_COMPLEX,           RES_CHRATR_CTL_FONTSIZE,        cppu::UnoType<float>::get(),        PropertyAttribute::MAYBEVOID, MID_FONTHEIGHT|CONVERT_TWIPS },
                    { UNO_NAME_CHAR_WEIGHT_COMPLEX,           RES_CHRATR_CTL_WEIGHT,          cppu::UnoType<float>::get(),        PropertyAttribute::MAYBEVOID, MID_WEIGHT                   },
                    { UNO_NAME_CHAR_POSTURE_COMPLEX,          RES_CHRATR_CTL_POSTURE,  cppu::UnoType<css::awt::FontSlant>::get(), PropertyAttribute::MAYBEVOID, MID_POSTURE                  },
                    { UNO_NAME_CHAR_FONT_NAME_COMPLEX,        RES_CHRATR_CTL_FONT,         cppu::UnoType<OUString>::get(),        PropertyAttribute::MAYBEVOID, MID_FONT_FAMILY_NAME         },
                    { UNO_NAME_CHAR_FONT_STYLE_NAME_COMPLEX,  RES_CHRATR_CTL_FONT,         cppu::UnoType<OUString>::get(),        PropertyAttribute::MAYBEVOID, MID_FONT_STYLE_NAME          },
                    { UNO_NAME_CHAR_FONT_FAMILY_COMPLEX,      RES_CHRATR_CTL_FONT,        cppu::UnoType<sal_Int16>::get(),        PropertyAttribute::MAYBEVOID, MID_FONT_FAMILY              },
                    { UNO_NAME_CHAR_FONT_CHAR_SET_COMPLEX,    RES_CHRATR_CTL_FONT,        cppu::UnoType<sal_Int16>::get(),        PropertyAttribute::MAYBEVOID, MID_FONT_CHAR_SET            },
                    { UNO_NAME_CHAR_FONT_PITCH_COMPLEX,       RES_CHRATR_CTL_FONT,        cppu::UnoType<sal_Int16>::get(),        PropertyAttribute::MAYBEVOID, MID_FONT_PITCH               },
                    { u"", 0, css::uno::Type(), 0, 0 }
                };
                m_aMapEntriesArr[nPropertyId] = aCellStyleMap;
            }
            break;

            default:
                OSL_FAIL( "unexpected property map ID" );
        }
    }
    return m_aMapEntriesArr[nPropertyId];
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
