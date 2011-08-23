/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif
#ifndef __SGI_STL_ALGORITHM
#include <algorithm>
#endif

#ifndef _XMLOFF_FORMS_STRINGS_HXX_
#include "strings.hxx"
#endif
#ifndef _XMLOFF_CONTEXTID_HXX_
#include "contextid.hxx"
#endif

#ifndef _XMLOFF_FORMS_CONTROLPROPERTYMAP_HXX_
#include "controlpropertymap.hxx"
#endif

#include <string.h>
namespace binfilter {

using namespace ::binfilter::xmloff::token;

//.........................................................................
namespace xmloff
{
//.........................................................................

#define MAP_ASCII( name, prefix, token, type, context )  { name, sizeof(name)-1, prefix, token, type, context }
#define MAP_CONST( name, prefix, token, type, context )  { name.ascii, name.length,	prefix, token, type, context }
#define MAP_END()	{ NULL, 0, 0, XML_TOKEN_INVALID, 0 }

    XMLPropertyMapEntry* getControlStylePropertyMap_Access( )
    {
        static XMLPropertyMapEntry aControlStyleProperties[] =
        {
            MAP_CONST( PROPERTY_BACKGROUNDCOLOR, XML_NAMESPACE_FO,		XML_BACKGROUND_COLOR,		XML_TYPE_COLOR, 0 ),
            MAP_CONST( PROPERTY_ALIGN,			XML_NAMESPACE_STYLE,	XML_TEXT_ALIGN, 			XML_TYPE_TEXT_ALIGN, 0 ),
            MAP_CONST( PROPERTY_BORDER,			XML_NAMESPACE_FO,		XML_BORDER, 				XML_TYPE_CONTROL_BORDER, 0 ),
            MAP_ASCII( "FontCharWidth",			XML_NAMESPACE_STYLE,	XML_FONT_CHAR_WIDTH,		XML_TYPE_NUMBER16, 0 ),
            MAP_ASCII( "FontCharset",			XML_NAMESPACE_STYLE,	XML_FONT_CHARSET,			XML_TYPE_TEXT_FONTENCODING, 0 ),
            MAP_ASCII( "FontFamily",				XML_NAMESPACE_STYLE,	XML_FONT_FAMILY_GENERIC,	XML_TYPE_TEXT_FONTFAMILY, 0 ),
            MAP_ASCII( "FontHeight",				XML_NAMESPACE_FO,		XML_FONT_SIZE,				XML_TYPE_CHAR_HEIGHT, 0 ),
            MAP_ASCII( "FontKerning",			XML_NAMESPACE_STYLE,	XML_LETTER_KERNING, 		XML_TYPE_BOOL, 0 ),
            MAP_ASCII( "FontName",				XML_NAMESPACE_STYLE,	XML_FONT_NAME,   			XML_TYPE_STRING, 0 ),
            MAP_ASCII( "FontOrientation",		XML_NAMESPACE_STYLE,	XML_ROTATION_ANGLE, 		XML_TYPE_ROTATION_ANGLE, 0 ),
            MAP_ASCII( "FontPitch",				XML_NAMESPACE_STYLE,	XML_FONT_PITCH, 			XML_TYPE_TEXT_FONTPITCH, 0 ),
            MAP_ASCII( "FontSlant",				XML_NAMESPACE_FO,		XML_FONT_STYLE, 			XML_TYPE_TEXT_POSTURE, 0 ),
            MAP_ASCII( "FontStrikeout",			XML_NAMESPACE_STYLE,	XML_TEXT_CROSSING_OUT,   	XML_TYPE_TEXT_CROSSEDOUT, 0 ),
            MAP_ASCII( "FontStyleName",			XML_NAMESPACE_STYLE,	XML_FONT_STYLE_NAME,		XML_TYPE_STRING, 0 ),
            MAP_ASCII( "FontUnderline",			XML_NAMESPACE_STYLE,	XML_TEXT_UNDERLINE, 		XML_TYPE_TEXT_UNDERLINE, 0 ),
            MAP_ASCII( "FontWeight",				XML_NAMESPACE_FO,		XML_FONT_WEIGHT,			XML_TYPE_TEXT_WEIGHT, 0 ),
            MAP_ASCII( "FontWidth",				XML_NAMESPACE_STYLE,	XML_FONT_WIDTH, 			XML_TYPE_FONT_WIDTH, 0 ),
            MAP_ASCII( "FontWordLineMode",		XML_NAMESPACE_FO,		XML_SCORE_SPACES,			XML_TYPE_NBOOL, 0 ),
            MAP_ASCII( "TextColor",				XML_NAMESPACE_FO,		XML_COLOR,					XML_TYPE_COLOR, 0 ),
            MAP_CONST( PROPERTY_FORMATKEY,		XML_NAMESPACE_STYLE,	XML_DATA_STYLE_NAME,		XML_TYPE_STRING | MID_FLAG_NO_PROPERTY_EXPORT | MID_FLAG_SPECIAL_ITEM, CTF_FORMS_DATA_STYLE ),
            MAP_ASCII( "FontEmphasisMark",		XML_NAMESPACE_STYLE,	XML_TEXT_EMPHASIZE, 		XML_TYPE_CONTROL_TEXT_EMPHASIZE, 0 ),
            MAP_ASCII( "FontRelief",				XML_NAMESPACE_STYLE,	XML_FONT_RELIEF,			XML_TYPE_TEXT_FONT_RELIEF|MID_FLAG_MULTI_PROPERTY, 0 ),
            MAP_ASCII( "TextLineColor",			XML_NAMESPACE_STYLE,	XML_TEXT_UNDERLINE_COLOR,	XML_TYPE_TEXT_UNDERLINE_COLOR|MID_FLAG_MULTI_PROPERTY, 0 ),
            MAP_END()
        };

        return aControlStyleProperties;
    }

    const XMLPropertyMapEntry* getControlStylePropertyMap( )
    {
        return getControlStylePropertyMap_Access();
    }

    //=====================================================================
    //---------------------------------------------------------------------
    struct XMLPropertyMapEntryLess
    {
        sal_Bool operator()(const XMLPropertyMapEntry& _rLeft, const XMLPropertyMapEntry& _rRight)
        {
            return strcmp(_rLeft.msApiName, _rRight.msApiName) < 0;
        }
    };

    //---------------------------------------------------------------------
    void implSortMap(XMLPropertyMapEntry* _pMap)
    {
        // determine the last element
        XMLPropertyMapEntry* pEnd;

        for (pEnd = _pMap; pEnd->msApiName; ++pEnd)
            ;
        ::std::sort(_pMap, pEnd, XMLPropertyMapEntryLess());
    }

    //---------------------------------------------------------------------
    void initializePropertyMaps()
    {
        static sal_Bool bSorted = sal_False;
        if (!bSorted)
        {
            implSortMap(getControlStylePropertyMap_Access());
            bSorted = sal_True;
        }
    }

    //=====================================================================
    //= OFormExportPropertyMapper
    //=====================================================================
    //---------------------------------------------------------------------
    OFormExportPropertyMapper::OFormExportPropertyMapper( const UniReference< XMLPropertySetMapper >& _rMapper )
        :SvXMLExportPropertyMapper( _rMapper )
    {
    }

    //---------------------------------------------------------------------
    void OFormExportPropertyMapper::handleSpecialItem( SvXMLAttributeList& _rAttrList, const XMLPropertyState& _rProperty, const SvXMLUnitConverter& _rUnitConverter,
        const SvXMLNamespaceMap& _rNamespaceMap, const ::std::vector< XMLPropertyState >* _pProperties,
        sal_uInt32 _nIdx ) const
    {
        // ignore the number style of grid columns - this is formatted elsewhere
        if ( CTF_FORMS_DATA_STYLE != getPropertySetMapper()->GetEntryContextId( _rProperty.mnIndex ) )
            SvXMLExportPropertyMapper::handleSpecialItem( _rAttrList, _rProperty, _rUnitConverter, _rNamespaceMap, _pProperties, _nIdx );
    }

//.........................................................................
}	// namespace xmloff
//.........................................................................


}//end of namespace binfilter
