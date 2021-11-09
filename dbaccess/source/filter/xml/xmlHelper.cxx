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

#include "xmlHelper.hxx"
#include <xmloff/XMLConstantsPropertyHandler.hxx>
#include <xmloff/xmlement.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmltypes.hxx>
#include <xmloff/maptype.hxx>

#include <com/sun/star/awt/TextAlign.hpp>
#include <strings.hxx>
#include <rtl/ref.hxx>

namespace dbaxml
{
    using namespace ::xmloff::token;
    using namespace ::com::sun::star::awt;

OPropertyHandlerFactory::OPropertyHandlerFactory()
{
}

OPropertyHandlerFactory::~OPropertyHandlerFactory()
{

}

const XMLPropertyHandler* OPropertyHandlerFactory::GetPropertyHandler(sal_Int32 _nType) const
{
    const XMLPropertyHandler* pHandler = nullptr;

    switch (_nType)
    {
        case XML_DB_TYPE_EQUAL:
            if (!m_pDisplayHandler)
            {
                static const SvXMLEnumMapEntry<bool> aDisplayMap[] =
                {
                    { XML_VISIBLE,       true },
                    { XML_COLLAPSE,      false },
                    { XML_TOKEN_INVALID, false }
                };
                m_pDisplayHandler.reset(new XMLConstantsPropertyHandler(aDisplayMap, XML_TOKEN_INVALID ));
            }
            pHandler = m_pDisplayHandler.get();
            break;
    }
    if ( !pHandler )
        pHandler = OControlPropertyHandlerFactory::GetPropertyHandler(_nType);
    return pHandler;
}

rtl::Reference < XMLPropertySetMapper > OXMLHelper::GetTableStylesPropertySetMapper( bool bForExport )
{
    static const XMLPropertyMapEntry s_aTableStylesProperties[] =
    {
        { nullptr, 0, 0, XML_TOKEN_INVALID, 0 , 0, SvtSaveOptions::ODFSVER_010, false}
    };
    rtl::Reference < XMLPropertyHandlerFactory> xFac = new ::xmloff::OControlPropertyHandlerFactory();
    return new XMLPropertySetMapper(s_aTableStylesProperties, xFac, bForExport);
}

rtl::Reference < XMLPropertySetMapper > OXMLHelper::GetColumnStylesPropertySetMapper( bool bForExport )
{
    static const XMLPropertyMapEntry s_aColumnStylesProperties[] =
    {
        { PROPERTY_WIDTH, sizeof(PROPERTY_WIDTH)-1,  XML_NAMESPACE_STYLE, XML_COLUMN_WIDTH, XML_TYPE_MEASURE|XML_TYPE_PROP_TABLE_COLUMN, 0, SvtSaveOptions::ODFSVER_010, false },
        { PROPERTY_HIDDEN, sizeof(PROPERTY_HIDDEN)-1,  XML_NAMESPACE_TABLE, XML_DISPLAY, XML_DB_TYPE_EQUAL|MID_FLAG_SPECIAL_ITEM|XML_TYPE_PROP_TABLE_COLUMN,
          CTF_DB_ISVISIBLE, SvtSaveOptions::ODFSVER_010, false },
        { PROPERTY_NUMBERFORMAT, sizeof(PROPERTY_NUMBERFORMAT)-1,  XML_NAMESPACE_STYLE, XML_DATA_STYLE_NAME, XML_TYPE_NUMBER|MID_FLAG_SPECIAL_ITEM|XML_TYPE_PROP_TABLE_COLUMN,
           CTF_DB_NUMBERFORMAT, SvtSaveOptions::ODFSVER_010, false },
        { nullptr, 0, 0, XML_TOKEN_INVALID, 0 , 0, SvtSaveOptions::ODFSVER_010, false}
    };
    rtl::Reference < XMLPropertyHandlerFactory> xFac = new OPropertyHandlerFactory();
    return new XMLPropertySetMapper(s_aColumnStylesProperties, xFac, bForExport);
}

rtl::Reference < XMLPropertySetMapper > OXMLHelper::GetCellStylesPropertySetMapper( bool bForExport )
{
    static const XMLPropertyMapEntry s_aCellStylesProperties[] =
    {
        { PROPERTY_ALIGN, sizeof(PROPERTY_ALIGN)-1,  XML_NAMESPACE_FO, XML_TEXT_ALIGN, XML_TYPE_TEXT_ALIGN|XML_TYPE_PROP_PARAGRAPH, CTF_DB_COLUMN_TEXT_ALIGN, SvtSaveOptions::ODFSVER_010, false },
        { PROPERTY_FONTNAME, sizeof(PROPERTY_FONTNAME)-1,  XML_NAMESPACE_STYLE, XML_FONT_NAME, XML_TYPE_STRING|XML_TYPE_PROP_TEXT, 0 , SvtSaveOptions::ODFSVER_010, false },
        { PROPERTY_TEXTCOLOR, sizeof(PROPERTY_TEXTCOLOR)-1,  XML_NAMESPACE_FO, XML_COLOR, XML_TYPE_COLOR|XML_TYPE_PROP_TEXT, 0 , SvtSaveOptions::ODFSVER_010, false },
        { PROPERTY_TEXTLINECOLOR, sizeof(PROPERTY_TEXTLINECOLOR)-1,  XML_NAMESPACE_STYLE, XML_TEXT_UNDERLINE_COLOR,
          XML_TYPE_TEXT_UNDERLINE_COLOR|MID_FLAG_MULTI_PROPERTY|XML_TYPE_PROP_TEXT, 0 , SvtSaveOptions::ODFSVER_010, false },
        { PROPERTY_TEXTRELIEF, sizeof(PROPERTY_TEXTRELIEF)-1,  XML_NAMESPACE_STYLE, XML_FONT_RELIEF,
          XML_TYPE_TEXT_FONT_RELIEF|MID_FLAG_MULTI_PROPERTY|XML_TYPE_PROP_TEXT, 0 , SvtSaveOptions::ODFSVER_010, false },
        { PROPERTY_TEXTEMPHASIS, sizeof(PROPERTY_TEXTEMPHASIS)-1,  XML_NAMESPACE_STYLE, XML_TEXT_EMPHASIZE, XML_TYPE_CONTROL_TEXT_EMPHASIZE|XML_TYPE_PROP_TEXT, 0 , SvtSaveOptions::ODFSVER_010, false },
        { PROPERTY_FONTCHARWIDTH, sizeof(PROPERTY_FONTCHARWIDTH)-1,  XML_NAMESPACE_STYLE, XML_FONT_CHAR_WIDTH, XML_TYPE_NUMBER16|XML_TYPE_PROP_TEXT, 0 , SvtSaveOptions::ODFSVER_010, false },
        { PROPERTY_FONTCHARSET, sizeof(PROPERTY_FONTCHARSET)-1,  XML_NAMESPACE_STYLE, XML_FONT_CHARSET, XML_TYPE_TEXT_FONTENCODING|XML_TYPE_PROP_TEXT, 0 , SvtSaveOptions::ODFSVER_010, false },
        { PROPERTY_FONTFAMILY, sizeof(PROPERTY_FONTFAMILY)-1,  XML_NAMESPACE_STYLE, XML_FONT_FAMILY_GENERIC, XML_TYPE_TEXT_FONTFAMILY|XML_TYPE_PROP_TEXT, 0 , SvtSaveOptions::ODFSVER_010, false },
        { PROPERTY_FONTHEIGHT, sizeof(PROPERTY_FONTHEIGHT)-1,  XML_NAMESPACE_FO, XML_FONT_SIZE, XML_TYPE_MEASURE16|XML_TYPE_PROP_TEXT, 0 , SvtSaveOptions::ODFSVER_010, false },
        { PROPERTY_FONTKERNING, sizeof(PROPERTY_FONTKERNING)-1,  XML_NAMESPACE_STYLE, XML_LETTER_KERNING,
           XML_TYPE_BOOL|XML_TYPE_PROP_TEXT, 0 , SvtSaveOptions::ODFSVER_010, false },
        { PROPERTY_FONTORIENTATION, sizeof(PROPERTY_FONTORIENTATION)-1,  XML_NAMESPACE_STYLE, XML_ROTATION_ANGLE, XML_TYPE_ROTATION_ANGLE|XML_TYPE_PROP_TEXT, 0 , SvtSaveOptions::ODFSVER_010, false },
        { PROPERTY_FONTPITCH, sizeof(PROPERTY_FONTPITCH)-1,  XML_NAMESPACE_STYLE, XML_FONT_PITCH, XML_TYPE_TEXT_FONTPITCH|XML_TYPE_PROP_TEXT, 0 , SvtSaveOptions::ODFSVER_010, false },
        { PROPERTY_FONTSLANT, sizeof(PROPERTY_FONTSLANT)-1,  XML_NAMESPACE_FO, XML_FONT_STYLE, XML_TYPE_TEXT_POSTURE|XML_TYPE_PROP_TEXT, 0 , SvtSaveOptions::ODFSVER_010, false },
        { "CharStrikeout", sizeof("CharStrikeout")-1,  XML_NAMESPACE_STYLE, XML_TEXT_LINE_THROUGH_STYLE,
           XML_TYPE_TEXT_CROSSEDOUT_STYLE|MID_FLAG_MERGE_PROPERTY|XML_TYPE_PROP_TEXT, 0, SvtSaveOptions::ODFSVER_010, false },
        { "CharStrikeout", sizeof("CharStrikeout")-1,  XML_NAMESPACE_STYLE, XML_TEXT_LINE_THROUGH_TYPE,
          XML_TYPE_TEXT_CROSSEDOUT_TYPE|MID_FLAG_MERGE_PROPERTY|XML_TYPE_PROP_TEXT, 0, SvtSaveOptions::ODFSVER_010, false },
        { "CharStrikeout", sizeof("CharStrikeout")-1,  XML_NAMESPACE_STYLE, XML_TEXT_LINE_THROUGH_WIDTH,
          XML_TYPE_TEXT_CROSSEDOUT_WIDTH|MID_FLAG_MERGE_PROPERTY|XML_TYPE_PROP_TEXT, 0, SvtSaveOptions::ODFSVER_010, false },
        { "CharStrikeout", sizeof("CharStrikeout")-1,  XML_NAMESPACE_STYLE, XML_TEXT_LINE_THROUGH_TEXT,
          XML_TYPE_TEXT_CROSSEDOUT_TEXT|MID_FLAG_MERGE_PROPERTY|XML_TYPE_PROP_TEXT, 0, SvtSaveOptions::ODFSVER_010, false },
        { PROPERTY_FONTSTYLENAME, sizeof(PROPERTY_FONTSTYLENAME)-1,  XML_NAMESPACE_STYLE, XML_FONT_STYLE_NAME, XML_TYPE_STRING|XML_TYPE_PROP_TEXT, 0 , SvtSaveOptions::ODFSVER_010, false },
        { "CharUnderline", sizeof("CharUnderline")-1,  XML_NAMESPACE_STYLE, XML_TEXT_UNDERLINE_STYLE,
           XML_TYPE_TEXT_UNDERLINE_STYLE|MID_FLAG_MERGE_PROPERTY|XML_TYPE_PROP_TEXT, 0 , SvtSaveOptions::ODFSVER_010, false },
        { "CharUnderline", sizeof("CharUnderline")-1,  XML_NAMESPACE_STYLE, XML_TEXT_UNDERLINE_TYPE,
           XML_TYPE_TEXT_UNDERLINE_TYPE|MID_FLAG_MERGE_PROPERTY|XML_TYPE_PROP_TEXT, 0 , SvtSaveOptions::ODFSVER_010, false },
        { "CharUnderline", sizeof("CharUnderline")-1,  XML_NAMESPACE_STYLE, XML_TEXT_UNDERLINE_WIDTH,
           XML_TYPE_TEXT_UNDERLINE_WIDTH|MID_FLAG_MERGE_PROPERTY|XML_TYPE_PROP_TEXT, 0 , SvtSaveOptions::ODFSVER_010, false },
        { "CharUnderlineColor", sizeof("CharUnderlineColor")-1,  XML_NAMESPACE_STYLE, XML_TEXT_UNDERLINE_COLOR,
          XML_TYPE_TEXT_UNDERLINE_COLOR|MID_FLAG_MULTI_PROPERTY|XML_TYPE_PROP_TEXT, 0    , SvtSaveOptions::ODFSVER_010, false },
        { "CharUnderlineHasColor", sizeof("CharUnderlineHasColor")-1,  XML_NAMESPACE_STYLE, XML_TEXT_UNDERLINE_COLOR,
          XML_TYPE_TEXT_UNDERLINE_HASCOLOR|MID_FLAG_MERGE_ATTRIBUTE|XML_TYPE_PROP_TEXT, 0    , SvtSaveOptions::ODFSVER_010, false },
        { PROPERTY_FONTWEIGHT, sizeof(PROPERTY_FONTWEIGHT)-1,  XML_NAMESPACE_FO, XML_FONT_WEIGHT, XML_TYPE_TEXT_WEIGHT|XML_TYPE_PROP_TEXT, 0 , SvtSaveOptions::ODFSVER_010, false },
        { PROPERTY_FONTWIDTH, sizeof(PROPERTY_FONTWIDTH)-1,  XML_NAMESPACE_STYLE, XML_FONT_WIDTH, XML_TYPE_FONT_WIDTH|XML_TYPE_PROP_TEXT, 0 , SvtSaveOptions::ODFSVER_010, false },
        { PROPERTY_FONTWORDLINEMODE, sizeof(PROPERTY_FONTWORDLINEMODE)-1,  XML_NAMESPACE_STYLE, XML_TEXT_UNDERLINE_MODE,
          XML_TYPE_TEXT_LINE_MODE|MID_FLAG_MERGE_PROPERTY|XML_TYPE_PROP_TEXT, 0 , SvtSaveOptions::ODFSVER_010, false },
        { nullptr, 0, 0, XML_TOKEN_INVALID, 0 , 0, SvtSaveOptions::ODFSVER_010, false}
    };
    rtl::Reference < XMLPropertyHandlerFactory> xFac = new /*OPropertyHandlerFactory*/::xmloff::OControlPropertyHandlerFactory();
    return new XMLPropertySetMapper(s_aCellStylesProperties, xFac, bForExport);
}

rtl::Reference < XMLPropertySetMapper > OXMLHelper::GetRowStylesPropertySetMapper()
{
    static const XMLPropertyMapEntry s_aStylesProperties[] =
    {
        { PROPERTY_ROW_HEIGHT, sizeof(PROPERTY_ROW_HEIGHT)-1, XML_NAMESPACE_STYLE, XML_ROW_HEIGHT, XML_TYPE_MEASURE|XML_TYPE_PROP_TABLE_ROW, 0, SvtSaveOptions::ODFSVER_010, false },
        { nullptr, 0, 0, XML_TOKEN_INVALID, 0 , 0, SvtSaveOptions::ODFSVER_010, false}
    };
    rtl::Reference < XMLPropertyHandlerFactory> xFac = new OPropertyHandlerFactory();
    return new XMLPropertySetMapper(s_aStylesProperties, xFac, true/*bForExport*/);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
