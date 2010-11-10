/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dbaccess.hxx"
#include "xmlHelper.hxx"
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmltoken.hxx>
#include <com/sun/star/awt/TextAlign.hpp>
#include "xmlstrings.hrc"
#include "xmlEnums.hxx"
#include <xmloff/contextid.hxx>
#include <xmloff/XMLConstantsPropertyHandler.hxx>
#include <xmloff/xmlement.hxx>
#include <com/sun/star/awt/TextAlign.hpp>
#include <tools/debug.hxx>

namespace dbaxml
{
    using namespace ::xmloff::token;
    using namespace ::com::sun::star::awt;
DBG_NAME(OPropertyHandlerFactory)

OPropertyHandlerFactory::OPropertyHandlerFactory()
{
    DBG_CTOR(OPropertyHandlerFactory,NULL);

}
// -----------------------------------------------------------------------------
OPropertyHandlerFactory::~OPropertyHandlerFactory()
{

    DBG_DTOR(OPropertyHandlerFactory,NULL);
}
// -----------------------------------------------------------------------------
const XMLPropertyHandler* OPropertyHandlerFactory::GetPropertyHandler(sal_Int32 _nType) const
{
    const XMLPropertyHandler* pHandler = NULL;

    switch (_nType)
    {
        case XML_DB_TYPE_EQUAL:
            if ( !m_pDisplayHandler.get() )
            {
                static SvXMLEnumMapEntry aDisplayMap[] =
                {
                    { XML_VISIBLE,      sal_True },
                    { XML_COLLAPSE,     sal_False },
                    { XML_TOKEN_INVALID, 0 }
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
// -----------------------------------------------------------------------------
#define MAP_CONST( name, prefix, token, type, context )  { name.ascii, name.length, prefix, token, type|XML_TYPE_PROP_TABLE, context, SvtSaveOptions::ODFVER_010 }
#define MAP_CONST_ASCII( name, prefix, token, type, context )  { name, sizeof(name)-1,  prefix, token, type|XML_TYPE_PROP_TABLE, context, SvtSaveOptions::ODFVER_010 }
#define MAP_END() { NULL, 0, 0, XML_TOKEN_INVALID, 0 , 0, SvtSaveOptions::ODFVER_010}
// -----------------------------------------------------------------------------
UniReference < XMLPropertySetMapper > OXMLHelper::GetTableStylesPropertySetMapper()
{
    static const XMLPropertyMapEntry s_aTableStylesProperties[] =
    {
        MAP_END()
    };
    UniReference < XMLPropertyHandlerFactory> xFac = new ::xmloff::OControlPropertyHandlerFactory();
    return new XMLPropertySetMapper((XMLPropertyMapEntry*)s_aTableStylesProperties, xFac);
}
// -----------------------------------------------------------------------------
UniReference < XMLPropertySetMapper > OXMLHelper::GetColumnStylesPropertySetMapper()
{
#define MAP_CONST_COLUMN( name, prefix, token, type, context )  { name.ascii, name.length,  prefix, token, type|XML_TYPE_PROP_TABLE_COLUMN, context, SvtSaveOptions::ODFVER_010 }
    static const XMLPropertyMapEntry s_aColumnStylesProperties[] =
    {
        MAP_CONST_COLUMN( PROPERTY_WIDTH,           XML_NAMESPACE_STYLE,    XML_COLUMN_WIDTH,       XML_TYPE_MEASURE, 0),
        MAP_CONST_COLUMN( PROPERTY_HIDDEN,          XML_NAMESPACE_TABLE,    XML_DISPLAY,            XML_DB_TYPE_EQUAL|MID_FLAG_SPECIAL_ITEM, CTF_DB_ISVISIBLE ),
        MAP_CONST_COLUMN( PROPERTY_NUMBERFORMAT,    XML_NAMESPACE_STYLE,    XML_DATA_STYLE_NAME,    XML_TYPE_NUMBER|MID_FLAG_SPECIAL_ITEM, CTF_DB_NUMBERFORMAT),
        MAP_END()
    };
    UniReference < XMLPropertyHandlerFactory> xFac = new OPropertyHandlerFactory();
    return new XMLPropertySetMapper((XMLPropertyMapEntry*)s_aColumnStylesProperties, xFac);
}
// -----------------------------------------------------------------------------
UniReference < XMLPropertySetMapper > OXMLHelper::GetCellStylesPropertySetMapper()
{
#define MAP_CONST_CELL( name, prefix, token, type, context )  { name.ascii, name.length,    prefix, token, type|XML_TYPE_PROP_PARAGRAPH, context, SvtSaveOptions::ODFVER_010 }
#define MAP_CONST_TEXT( name, prefix, token, type, context )  { name.ascii, name.length,    prefix, token, type|XML_TYPE_PROP_TEXT, context, SvtSaveOptions::ODFVER_010 }
#define MAP_CONST_TEXT_ASCII( name, prefix, token, type, context ) { name, sizeof(name)-1,  prefix, token, type|XML_TYPE_PROP_TEXT, context, SvtSaveOptions::ODFVER_010 }
    static const XMLPropertyMapEntry s_aCellStylesProperties[] =
    {
        MAP_CONST_CELL( PROPERTY_ALIGN,             XML_NAMESPACE_FO,       XML_TEXT_ALIGN,             XML_TYPE_TEXT_ALIGN, CTF_DB_COLUMN_TEXT_ALIGN),
        MAP_CONST_TEXT( PROPERTY_FONTNAME,          XML_NAMESPACE_STYLE,    XML_FONT_NAME,              XML_TYPE_STRING, 0 ),
        MAP_CONST_TEXT( PROPERTY_TEXTCOLOR,         XML_NAMESPACE_FO,       XML_COLOR,                  XML_TYPE_COLOR, 0 ),
        MAP_CONST_TEXT( PROPERTY_TEXTLINECOLOR,     XML_NAMESPACE_STYLE,    XML_TEXT_UNDERLINE_COLOR,   XML_TYPE_TEXT_UNDERLINE_COLOR|MID_FLAG_MULTI_PROPERTY, 0 ),

        MAP_CONST_TEXT( PROPERTY_TEXTRELIEF,        XML_NAMESPACE_STYLE,    XML_FONT_RELIEF,                XML_TYPE_TEXT_FONT_RELIEF|MID_FLAG_MULTI_PROPERTY, 0 ),
        MAP_CONST_TEXT( PROPERTY_TEXTEMPHASIS,      XML_NAMESPACE_STYLE,    XML_TEXT_EMPHASIZE,             XML_TYPE_CONTROL_TEXT_EMPHASIZE, 0 ),
        MAP_CONST_TEXT( PROPERTY_FONTCHARWIDTH,     XML_NAMESPACE_STYLE,    XML_FONT_CHAR_WIDTH,            XML_TYPE_NUMBER16, 0 ),
        MAP_CONST_TEXT( PROPERTY_FONTCHARSET,       XML_NAMESPACE_STYLE,    XML_FONT_CHARSET,               XML_TYPE_TEXT_FONTENCODING, 0 ),
        MAP_CONST_TEXT( PROPERTY_FONTFAMILY,        XML_NAMESPACE_STYLE,    XML_FONT_FAMILY_GENERIC,        XML_TYPE_TEXT_FONTFAMILY, 0 ),
        MAP_CONST_TEXT( PROPERTY_FONTHEIGHT,        XML_NAMESPACE_FO,       XML_FONT_SIZE,                  XML_TYPE_MEASURE16, 0 ),
        MAP_CONST_TEXT( PROPERTY_FONTKERNING,       XML_NAMESPACE_STYLE,    XML_LETTER_KERNING,             XML_TYPE_BOOL, 0 ),

        MAP_CONST_TEXT( PROPERTY_FONTORIENTATION,   XML_NAMESPACE_STYLE,    XML_ROTATION_ANGLE,             XML_TYPE_ROTATION_ANGLE, 0 ),
        MAP_CONST_TEXT( PROPERTY_FONTPITCH,         XML_NAMESPACE_STYLE,    XML_FONT_PITCH,                 XML_TYPE_TEXT_FONTPITCH, 0 ),
        MAP_CONST_TEXT( PROPERTY_FONTSLANT,         XML_NAMESPACE_FO,       XML_FONT_STYLE,                 XML_TYPE_TEXT_POSTURE, 0 ),
        MAP_CONST_TEXT_ASCII( "CharStrikeout",      XML_NAMESPACE_STYLE,    XML_TEXT_LINE_THROUGH_STYLE,    XML_TYPE_TEXT_CROSSEDOUT_STYLE|MID_FLAG_MERGE_PROPERTY, 0),
        MAP_CONST_TEXT_ASCII( "CharStrikeout",      XML_NAMESPACE_STYLE,    XML_TEXT_LINE_THROUGH_TYPE,     XML_TYPE_TEXT_CROSSEDOUT_TYPE|MID_FLAG_MERGE_PROPERTY,  0),
        MAP_CONST_TEXT_ASCII( "CharStrikeout",      XML_NAMESPACE_STYLE,    XML_TEXT_LINE_THROUGH_WIDTH,    XML_TYPE_TEXT_CROSSEDOUT_WIDTH|MID_FLAG_MERGE_PROPERTY, 0),
        MAP_CONST_TEXT_ASCII( "CharStrikeout",      XML_NAMESPACE_STYLE,    XML_TEXT_LINE_THROUGH_TEXT,     XML_TYPE_TEXT_CROSSEDOUT_TEXT|MID_FLAG_MERGE_PROPERTY,  0),
        MAP_CONST_TEXT( PROPERTY_FONTSTYLENAME,     XML_NAMESPACE_STYLE,    XML_FONT_STYLE_NAME,            XML_TYPE_STRING, 0 ),
        MAP_CONST_TEXT_ASCII( "CharUnderline",      XML_NAMESPACE_STYLE,    XML_TEXT_UNDERLINE_STYLE,       XML_TYPE_TEXT_UNDERLINE_STYLE|MID_FLAG_MERGE_PROPERTY, 0 ),
        MAP_CONST_TEXT_ASCII( "CharUnderline",      XML_NAMESPACE_STYLE,    XML_TEXT_UNDERLINE_TYPE,        XML_TYPE_TEXT_UNDERLINE_TYPE|MID_FLAG_MERGE_PROPERTY, 0 ),
        MAP_CONST_TEXT_ASCII( "CharUnderline",      XML_NAMESPACE_STYLE,    XML_TEXT_UNDERLINE_WIDTH,       XML_TYPE_TEXT_UNDERLINE_WIDTH|MID_FLAG_MERGE_PROPERTY, 0 ),
        MAP_CONST_TEXT_ASCII( "CharUnderlineColor", XML_NAMESPACE_STYLE,    XML_TEXT_UNDERLINE_COLOR,       XML_TYPE_TEXT_UNDERLINE_COLOR|MID_FLAG_MULTI_PROPERTY, 0    ),
        MAP_CONST_TEXT_ASCII( "CharUnderlineHasColor",XML_NAMESPACE_STYLE,  XML_TEXT_UNDERLINE_COLOR,       XML_TYPE_TEXT_UNDERLINE_HASCOLOR|MID_FLAG_MERGE_ATTRIBUTE, 0    ),
        MAP_CONST_TEXT( PROPERTY_FONTWEIGHT,        XML_NAMESPACE_FO,       XML_FONT_WEIGHT,                XML_TYPE_TEXT_WEIGHT, 0 ),
        MAP_CONST_TEXT( PROPERTY_FONTWIDTH,         XML_NAMESPACE_STYLE,    XML_FONT_WIDTH,                 XML_TYPE_FONT_WIDTH, 0 ),
        MAP_CONST_TEXT( PROPERTY_FONTWORDLINEMODE,  XML_NAMESPACE_STYLE,    XML_TEXT_UNDERLINE_MODE,        XML_TYPE_TEXT_LINE_MODE|MID_FLAG_MERGE_PROPERTY, 0 ),
        MAP_END()
    };
    UniReference < XMLPropertyHandlerFactory> xFac = new /*OPropertyHandlerFactory*/::xmloff::OControlPropertyHandlerFactory();
    return new XMLPropertySetMapper((XMLPropertyMapEntry*)s_aCellStylesProperties, xFac);
}
// -----------------------------------------------------------------------------
UniReference < XMLPropertySetMapper > OXMLHelper::GetRowStylesPropertySetMapper()
{
#define MAP_CONST_ROW( name, prefix, token, type, context )  { name.ascii, name.length, prefix, token, type|XML_TYPE_PROP_TABLE_ROW, context, SvtSaveOptions::ODFVER_010 }
    static const XMLPropertyMapEntry s_aStylesProperties[] =
    {
        MAP_CONST_ROW( PROPERTY_ROW_HEIGHT,         XML_NAMESPACE_STYLE,    XML_ROW_HEIGHT,             XML_TYPE_MEASURE, 0),
        MAP_END()
    };
    UniReference < XMLPropertyHandlerFactory> xFac = new OPropertyHandlerFactory();
    return new XMLPropertySetMapper((XMLPropertyMapEntry*)s_aStylesProperties, xFac);
}
// -----------------------------------------------------------------------------
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
