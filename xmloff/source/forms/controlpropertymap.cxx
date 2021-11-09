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

#include <rtl/ref.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/maptype.hxx>
#include <xmloff/xmltypes.hxx>
#include <algorithm>
#include "strings.hxx"
#include <xmloff/contextid.hxx>
#include "controlpropertymap.hxx"

#include <string.h>

using namespace ::xmloff::token;

namespace xmloff
{

#define MAP_ASCII( name, prefix, token, type, context )  { name, prefix, token, type|XML_TYPE_PROP_TEXT, context, SvtSaveOptions::ODFSVER_010, false }
#define MAP_CONST( name, prefix, token, type, context )  { name, prefix, token, type|XML_TYPE_PROP_TEXT, context, SvtSaveOptions::ODFSVER_010, false }
#define MAP_CONST_P( name, prefix, token, type, context ){ name, prefix, token, type|XML_TYPE_PROP_PARAGRAPH, context, SvtSaveOptions::ODFSVER_010, false }
#define MAP_END()   { nullptr, 0, XML_TOKEN_INVALID, 0, 0, SvtSaveOptions::ODFSVER_010, false }

constexpr OUStringLiteral BORDER_COLOR = u"BorderColor";
constexpr OUStringLiteral FONT_CHAR_WIDTH = u"FontCharWidth";
constexpr OUStringLiteral FONT_CHAR_SET = u"FontCharset";
constexpr OUStringLiteral FONT_EMPHASIS_MARK = u"FontEmphasisMark";
constexpr OUStringLiteral FONT_FAMILY = u"FontFamily";
constexpr OUStringLiteral FONT_HEIGHT = u"FontHeight";
constexpr OUStringLiteral FONT_KERNING = u"FontKerning";
constexpr OUStringLiteral FONT_NAME = u"FontName";
constexpr OUStringLiteral FONT_ORIENTATION = u"FontOrientation";
constexpr OUStringLiteral FONT_PITCH = u"FontPitch";
constexpr OUStringLiteral FONT_RELIEF = u"FontRelief";
constexpr OUStringLiteral FONT_SLANT = u"FontSlant";
constexpr OUStringLiteral FONT_STRIKEOUT = u"FontStrikeout";
constexpr OUStringLiteral FONT_STYLE_NAME = u"FontStyleName";
constexpr OUStringLiteral FONT_UNDERLINE = u"FontUnderline";
constexpr OUStringLiteral FONT_WEIGHT = u"FontWeight";
constexpr OUStringLiteral FONT_WIDTH = u"FontWidth";
constexpr OUStringLiteral FONT_WORD_LINE_MODE = u"FontWordLineMode";
constexpr OUStringLiteral SYMBOL_COLOR = u"SymbolColor";
constexpr OUStringLiteral TEXT_COLOR = u"TextColor";
constexpr OUStringLiteral TEXT_LINE_COLOR = u"TextLineColor";

    XMLPropertyMapEntry const aControlStyleProperties[] =
    {
            MAP_CONST_P( PROPERTY_ALIGN,        XML_NAMESPACE_FO,       XML_TEXT_ALIGN,             XML_TYPE_TEXT_ALIGN, 0 ),
            MAP_CONST( PROPERTY_BACKGROUNDCOLOR, XML_NAMESPACE_FO,      XML_BACKGROUND_COLOR,       XML_TYPE_COLOR, 0 ),
            MAP_CONST( PROPERTY_BORDER,         XML_NAMESPACE_FO,       XML_BORDER,                 XML_TYPE_CONTROL_BORDER|MID_FLAG_MULTI_PROPERTY|MID_FLAG_MERGE_ATTRIBUTE, 0 ),
            MAP_ASCII( BORDER_COLOR,            XML_NAMESPACE_FO,       XML_BORDER,                 XML_TYPE_CONTROL_BORDER_COLOR|MID_FLAG_MULTI_PROPERTY|MID_FLAG_MERGE_ATTRIBUTE, 0 ),
            MAP_ASCII( FONT_CHAR_WIDTH,         XML_NAMESPACE_STYLE,    XML_FONT_CHAR_WIDTH,        XML_TYPE_NUMBER16, 0 ),
            MAP_ASCII( FONT_CHAR_SET,           XML_NAMESPACE_STYLE,    XML_FONT_CHARSET,           XML_TYPE_TEXT_FONTENCODING, 0 ),
            MAP_ASCII( FONT_EMPHASIS_MARK,      XML_NAMESPACE_STYLE,    XML_TEXT_EMPHASIZE,         XML_TYPE_CONTROL_TEXT_EMPHASIZE, 0 ),
            MAP_ASCII( FONT_FAMILY,            XML_NAMESPACE_STYLE,    XML_FONT_FAMILY_GENERIC,    XML_TYPE_TEXT_FONTFAMILY, 0 ),
            MAP_ASCII( FONT_HEIGHT,            XML_NAMESPACE_FO,       XML_FONT_SIZE,              XML_TYPE_CHAR_HEIGHT, 0 ),
            MAP_ASCII( FONT_KERNING,           XML_NAMESPACE_STYLE,    XML_LETTER_KERNING,         XML_TYPE_BOOL, 0 ),
            MAP_ASCII( FONT_NAME,              XML_NAMESPACE_STYLE,    XML_FONT_NAME,              XML_TYPE_STRING, 0 ),
            MAP_ASCII( FONT_ORIENTATION,       XML_NAMESPACE_STYLE,    XML_ROTATION_ANGLE,         XML_TYPE_ROTATION_ANGLE, 0 ),
            MAP_ASCII( FONT_PITCH,             XML_NAMESPACE_STYLE,    XML_FONT_PITCH,             XML_TYPE_TEXT_FONTPITCH, 0 ),
            MAP_ASCII( FONT_RELIEF,            XML_NAMESPACE_STYLE,    XML_FONT_RELIEF,            XML_TYPE_TEXT_FONT_RELIEF|MID_FLAG_MULTI_PROPERTY, 0 ),
            MAP_ASCII( FONT_SLANT,             XML_NAMESPACE_FO,       XML_FONT_STYLE,             XML_TYPE_TEXT_POSTURE, 0 ),

            MAP_ASCII( FONT_STRIKEOUT,         XML_NAMESPACE_STYLE,    XML_TEXT_LINE_THROUGH_STYLE,    XML_TYPE_TEXT_CROSSEDOUT_STYLE|MID_FLAG_MERGE_PROPERTY, 0),
            MAP_ASCII( FONT_STRIKEOUT,         XML_NAMESPACE_STYLE,    XML_TEXT_LINE_THROUGH_TYPE,     XML_TYPE_TEXT_CROSSEDOUT_TYPE|MID_FLAG_MERGE_PROPERTY,  0),
            MAP_ASCII( FONT_STRIKEOUT,         XML_NAMESPACE_STYLE,    XML_TEXT_LINE_THROUGH_WIDTH,    XML_TYPE_TEXT_CROSSEDOUT_WIDTH|MID_FLAG_MERGE_PROPERTY, 0),
            MAP_ASCII( FONT_STRIKEOUT,         XML_NAMESPACE_STYLE,    XML_TEXT_LINE_THROUGH_TEXT,     XML_TYPE_TEXT_CROSSEDOUT_TEXT|MID_FLAG_MERGE_PROPERTY,  0),

            MAP_ASCII( FONT_STYLE_NAME,         XML_NAMESPACE_STYLE,    XML_FONT_STYLE_NAME,        XML_TYPE_STRING, 0 ),
            MAP_ASCII( FONT_UNDERLINE,         XML_NAMESPACE_STYLE,    XML_TEXT_UNDERLINE_STYLE,       XML_TYPE_TEXT_UNDERLINE_STYLE|MID_FLAG_MERGE_PROPERTY, 0 ),
            MAP_ASCII( FONT_UNDERLINE,         XML_NAMESPACE_STYLE,    XML_TEXT_UNDERLINE_TYPE,        XML_TYPE_TEXT_UNDERLINE_TYPE|MID_FLAG_MERGE_PROPERTY, 0 ),
            MAP_ASCII( FONT_UNDERLINE,         XML_NAMESPACE_STYLE,    XML_TEXT_UNDERLINE_WIDTH,       XML_TYPE_TEXT_UNDERLINE_WIDTH|MID_FLAG_MERGE_PROPERTY, 0 ),
            MAP_ASCII( FONT_WEIGHT,            XML_NAMESPACE_FO,       XML_FONT_WEIGHT,            XML_TYPE_TEXT_WEIGHT, 0 ),
            MAP_ASCII( FONT_WIDTH,             XML_NAMESPACE_STYLE,    XML_FONT_WIDTH,             XML_TYPE_FONT_WIDTH, 0 ),
            MAP_ASCII( FONT_WORD_LINE_MODE,      XML_NAMESPACE_FO,       XML_SCORE_SPACES,           XML_TYPE_NBOOL, 0 ),

            MAP_CONST( PROPERTY_FORMATKEY,      XML_NAMESPACE_STYLE,    XML_DATA_STYLE_NAME,        XML_TYPE_STRING | MID_FLAG_NO_PROPERTY_EXPORT | MID_FLAG_SPECIAL_ITEM, CTF_FORMS_DATA_STYLE ),

            MAP_ASCII( SYMBOL_COLOR,           XML_NAMESPACE_STYLE,    XML_COLOR,                  XML_TYPE_COLOR, 0 ),
            MAP_ASCII( TEXT_COLOR,             XML_NAMESPACE_FO,       XML_COLOR,                  XML_TYPE_COLOR, 0 ),
            MAP_ASCII( TEXT_LINE_COLOR,         XML_NAMESPACE_STYLE,    XML_TEXT_UNDERLINE_COLOR,   XML_TYPE_TEXT_UNDERLINE_COLOR|MID_FLAG_MULTI_PROPERTY, 0 ),
            MAP_END()
    };

    const XMLPropertyMapEntry* getControlStylePropertyMap( )
    {
        return aControlStyleProperties;
    }

    void initializePropertyMaps()
    {
        static bool bSorted = false;
        if (!bSorted)
        {
            XMLPropertyMapEntry const * pEnd;
            // determine the last element
            for ( pEnd = aControlStyleProperties; !static_cast<const OUString &>(pEnd->msApiName).isEmpty(); ++pEnd)
                ;
            assert( ::std::is_sorted(aControlStyleProperties, pEnd,
                                    [](const XMLPropertyMapEntry& _rLeft, const XMLPropertyMapEntry& _rRight)
                                    { return static_cast<const OUString &>(_rLeft.msApiName) < static_cast<const OUString &>(_rRight.msApiName); }) );
            bSorted = true;
        }
    }

    //= OFormComponentStyleExportMapper
    OFormComponentStyleExportMapper::OFormComponentStyleExportMapper( const rtl::Reference< XMLPropertySetMapper >& _rMapper )
        :SvXMLExportPropertyMapper( _rMapper )
    {
    }

    void OFormComponentStyleExportMapper::handleSpecialItem( SvXMLAttributeList& _rAttrList, const XMLPropertyState& _rProperty, const SvXMLUnitConverter& _rUnitConverter,
        const SvXMLNamespaceMap& _rNamespaceMap, const ::std::vector< XMLPropertyState >* _pProperties,
        sal_uInt32 _nIdx ) const
    {
        // ignore the number style of grid columns - this is formatted elsewhere
        if ( CTF_FORMS_DATA_STYLE != getPropertySetMapper()->GetEntryContextId( _rProperty.mnIndex ) )
            SvXMLExportPropertyMapper::handleSpecialItem( _rAttrList, _rProperty, _rUnitConverter, _rNamespaceMap, _pProperties, _nIdx );
    }

}   // namespace xmloff

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
