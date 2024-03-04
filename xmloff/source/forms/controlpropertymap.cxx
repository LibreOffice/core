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
#include <xmlprop.hxx>

using namespace ::xmloff::token;

namespace xmloff
{

#define MAP_ASCII( name, prefix, token, type, context )  { name, prefix, token, type|XML_TYPE_PROP_TEXT, context, SvtSaveOptions::ODFSVER_010, false }
#define MAP_CONST( name, prefix, token, type, context )  { name, prefix, token, type|XML_TYPE_PROP_TEXT, context, SvtSaveOptions::ODFSVER_010, false }
#define MAP_CONST_P( name, prefix, token, type, context ){ name, prefix, token, type|XML_TYPE_PROP_PARAGRAPH, context, SvtSaveOptions::ODFSVER_010, false }
#define MAP_END() { nullptr }

    XMLPropertyMapEntry const aControlStyleProperties[] =
    {
            MAP_CONST_P( PROPERTY_ALIGN,        XML_NAMESPACE_FO,       XML_TEXT_ALIGN,             XML_TYPE_TEXT_ALIGN, 0 ),
            MAP_CONST( PROPERTY_BACKGROUNDCOLOR, XML_NAMESPACE_FO,      XML_BACKGROUND_COLOR,       XML_TYPE_COLOR, 0 ),
            MAP_CONST( PROPERTY_BORDER,         XML_NAMESPACE_FO,       XML_BORDER,                 XML_TYPE_CONTROL_BORDER|MID_FLAG_MULTI_PROPERTY|MID_FLAG_MERGE_ATTRIBUTE, 0 ),
            MAP_ASCII( PROP_BorderColor,           XML_NAMESPACE_FO,       XML_BORDER,                 XML_TYPE_CONTROL_BORDER_COLOR|MID_FLAG_MULTI_PROPERTY|MID_FLAG_MERGE_ATTRIBUTE, 0 ),
            MAP_ASCII( PROP_FontCharWidth,         XML_NAMESPACE_STYLE,    XML_FONT_CHAR_WIDTH,        XML_TYPE_NUMBER16, 0 ),
            MAP_ASCII( PROP_FontCharset,           XML_NAMESPACE_STYLE,    XML_FONT_CHARSET,           XML_TYPE_TEXT_FONTENCODING, 0 ),
            MAP_ASCII( PROP_FontEmphasisMark,      XML_NAMESPACE_STYLE,    XML_TEXT_EMPHASIZE,         XML_TYPE_CONTROL_TEXT_EMPHASIZE, 0 ),
            MAP_ASCII( PROP_FontFamily,            XML_NAMESPACE_STYLE,    XML_FONT_FAMILY_GENERIC,    XML_TYPE_TEXT_FONTFAMILY, 0 ),
            MAP_ASCII( PROP_FontHeight,            XML_NAMESPACE_FO,       XML_FONT_SIZE,              XML_TYPE_CHAR_HEIGHT, 0 ),
            MAP_ASCII( PROP_FontKerning,           XML_NAMESPACE_STYLE,    XML_LETTER_KERNING,         XML_TYPE_BOOL, 0 ),
            MAP_ASCII( PROP_FontName,              XML_NAMESPACE_STYLE,    XML_FONT_NAME,              XML_TYPE_STRING, 0 ),
            MAP_ASCII( PROP_FontOrientation,       XML_NAMESPACE_STYLE,    XML_ROTATION_ANGLE,         XML_TYPE_ROTATION_ANGLE, 0 ),
            MAP_ASCII( PROP_FontPitch,             XML_NAMESPACE_STYLE,    XML_FONT_PITCH,             XML_TYPE_TEXT_FONTPITCH, 0 ),
            MAP_ASCII( PROP_FontRelief,            XML_NAMESPACE_STYLE,    XML_FONT_RELIEF,            XML_TYPE_TEXT_FONT_RELIEF|MID_FLAG_MULTI_PROPERTY, 0 ),
            MAP_ASCII( PROP_FontSlant,             XML_NAMESPACE_FO,       XML_FONT_STYLE,             XML_TYPE_TEXT_POSTURE, 0 ),

            MAP_ASCII( PROP_FontStrikeout,         XML_NAMESPACE_STYLE,    XML_TEXT_LINE_THROUGH_STYLE,    XML_TYPE_TEXT_CROSSEDOUT_STYLE|MID_FLAG_MERGE_PROPERTY, 0),
            MAP_ASCII( PROP_FontStrikeout,         XML_NAMESPACE_STYLE,    XML_TEXT_LINE_THROUGH_TYPE,     XML_TYPE_TEXT_CROSSEDOUT_TYPE|MID_FLAG_MERGE_PROPERTY,  0),
            MAP_ASCII( PROP_FontStrikeout,         XML_NAMESPACE_STYLE,    XML_TEXT_LINE_THROUGH_WIDTH,    XML_TYPE_TEXT_CROSSEDOUT_WIDTH|MID_FLAG_MERGE_PROPERTY, 0),
            MAP_ASCII( PROP_FontStrikeout,         XML_NAMESPACE_STYLE,    XML_TEXT_LINE_THROUGH_TEXT,     XML_TYPE_TEXT_CROSSEDOUT_TEXT|MID_FLAG_MERGE_PROPERTY,  0),

            MAP_ASCII( PROP_FontStyleName,         XML_NAMESPACE_STYLE,    XML_FONT_STYLE_NAME,        XML_TYPE_STRING, 0 ),
            MAP_ASCII( PROP_FontUnderline,         XML_NAMESPACE_STYLE,    XML_TEXT_UNDERLINE_STYLE,       XML_TYPE_TEXT_UNDERLINE_STYLE|MID_FLAG_MERGE_PROPERTY, 0 ),
            MAP_ASCII( PROP_FontUnderline,         XML_NAMESPACE_STYLE,    XML_TEXT_UNDERLINE_TYPE,        XML_TYPE_TEXT_UNDERLINE_TYPE|MID_FLAG_MERGE_PROPERTY, 0 ),
            MAP_ASCII( PROP_FontUnderline,         XML_NAMESPACE_STYLE,    XML_TEXT_UNDERLINE_WIDTH,       XML_TYPE_TEXT_UNDERLINE_WIDTH|MID_FLAG_MERGE_PROPERTY, 0 ),
            MAP_ASCII( PROP_FontWeight,            XML_NAMESPACE_FO,       XML_FONT_WEIGHT,            XML_TYPE_TEXT_WEIGHT, 0 ),
            MAP_ASCII( PROP_FontWidth,             XML_NAMESPACE_STYLE,    XML_FONT_WIDTH,             XML_TYPE_FONT_WIDTH, 0 ),
            MAP_ASCII( PROP_FontWordLineMode,      XML_NAMESPACE_FO,       XML_SCORE_SPACES,           XML_TYPE_NBOOL, 0 ),

            MAP_CONST( PROPERTY_FORMATKEY,      XML_NAMESPACE_STYLE,    XML_DATA_STYLE_NAME,        XML_TYPE_STRING | MID_FLAG_NO_PROPERTY_EXPORT | MID_FLAG_SPECIAL_ITEM, CTF_FORMS_DATA_STYLE ),

            MAP_ASCII( PROP_SymbolColor,           XML_NAMESPACE_STYLE,    XML_COLOR,                  XML_TYPE_COLOR, 0 ),
            MAP_ASCII( PROP_TextColor,             XML_NAMESPACE_FO,       XML_COLOR,                  XML_TYPE_COLOR, 0 ),
            MAP_ASCII( PROP_TextLineColor,         XML_NAMESPACE_STYLE,    XML_TEXT_UNDERLINE_COLOR,   XML_TYPE_TEXT_UNDERLINE_COLOR|MID_FLAG_MULTI_PROPERTY, 0 ),
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
            for ( pEnd = aControlStyleProperties; !pEnd->IsEnd(); ++pEnd)
                ;
            assert( ::std::is_sorted(aControlStyleProperties, pEnd,
                                    [](const XMLPropertyMapEntry& _rLeft, const XMLPropertyMapEntry& _rRight)
                                    { return _rLeft.getApiName() < _rRight.getApiName(); }) );
            bSorted = true;
        }
    }

    //= OFormComponentStyleExportMapper
    OFormComponentStyleExportMapper::OFormComponentStyleExportMapper( const rtl::Reference< XMLPropertySetMapper >& _rMapper )
        :SvXMLExportPropertyMapper( _rMapper )
    {
    }

    void OFormComponentStyleExportMapper::handleSpecialItem( comphelper::AttributeList& _rAttrList, const XMLPropertyState& _rProperty, const SvXMLUnitConverter& _rUnitConverter,
        const SvXMLNamespaceMap& _rNamespaceMap, const ::std::vector< XMLPropertyState >* _pProperties,
        sal_uInt32 _nIdx ) const
    {
        // ignore the number style of grid columns - this is formatted elsewhere
        if ( CTF_FORMS_DATA_STYLE != getPropertySetMapper()->GetEntryContextId( _rProperty.mnIndex ) )
            SvXMLExportPropertyMapper::handleSpecialItem( _rAttrList, _rProperty, _rUnitConverter, _rNamespaceMap, _pProperties, _nIdx );
    }

}   // namespace xmloff

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
