/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xmlHelper.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 16:48:44 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dbaccess.hxx"
#ifndef DBA_XMLHELPER_HXX
#include "xmlHelper.hxx"
#endif
#ifndef _XMLOFF_XMLNMSPE_HXX
#include <xmloff/xmlnmspe.hxx>
#endif
#ifndef _XMLOFF_XMLTOKEN_HXX
#include <xmloff/xmltoken.hxx>
#endif
#ifndef _COM_SUN_STAR_AWT_TEXTALIGN_HPP_
#include <com/sun/star/awt/TextAlign.hpp>
#endif
#ifndef DBACCESS_SHARED_XMLSTRINGS_HRC
#include "xmlstrings.hrc"
#endif
#ifndef DBA_XMLENUMS_HXX
#include "xmlEnums.hxx"
#endif
#ifndef _XMLOFF_CONTEXTID_HXX_
#include <xmloff/contextid.hxx>
#endif
#ifndef _XMLOFF_XMLCONSTANTSPROPERTYHANDLER_HXX
#include <xmloff/XMLConstantsPropertyHandler.hxx>
#endif
#ifndef _XMLOFF_XMLEMENT_HXX
#include <xmloff/xmlement.hxx>
#endif
#ifndef _COM_SUN_STAR_AWT_TEXTALIGN_HPP_
#include <com/sun/star/awt/TextAlign.hpp>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

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
#define MAP_CONST( name, prefix, token, type, context )  { name.ascii, name.length, prefix, token, type|XML_TYPE_PROP_TABLE, context }
#define MAP_CONST_ASCII( name, prefix, token, type, context )  { name, sizeof(name)-1,  prefix, token, type|XML_TYPE_PROP_TABLE, context }
#define MAP_END()   { NULL, 0, 0, XML_TOKEN_INVALID, 0 ,0}
// -----------------------------------------------------------------------------
UniReference < XMLPropertySetMapper > OXMLHelper::GetTableStylesPropertySetMapper()
{
    static const XMLPropertyMapEntry s_aTableStylesProperties[] =
    {
        //MAP_CONST( PROPERTY_FONTNAME,         XML_NAMESPACE_STYLE,    XML_FONT_NAME,              XML_TYPE_STRING, 0 ),
        //MAP_CONST( PROPERTY_TEXTCOLOR,            XML_NAMESPACE_FO,       XML_COLOR,                  XML_TYPE_COLOR, 0 ),
        //MAP_CONST( PROPERTY_TEXTLINECOLOR,        XML_NAMESPACE_STYLE,    XML_TEXT_UNDERLINE_COLOR,   XML_TYPE_TEXT_UNDERLINE_COLOR|MID_FLAG_MULTI_PROPERTY, 0 ),

        //MAP_CONST( PROPERTY_TEXTRELIEF,           XML_NAMESPACE_STYLE,    XML_FONT_RELIEF,                XML_TYPE_TEXT_FONT_RELIEF|MID_FLAG_MULTI_PROPERTY, 0 ),
        //MAP_CONST( PROPERTY_TEXTEMPHASIS,     XML_NAMESPACE_STYLE,    XML_TEXT_EMPHASIZE,             XML_TYPE_CONTROL_TEXT_EMPHASIZE, 0 ),
        //MAP_CONST( PROPERTY_FONTCHARWIDTH,        XML_NAMESPACE_STYLE,    XML_FONT_CHAR_WIDTH,            XML_TYPE_NUMBER16, 0 ),
        //MAP_CONST( PROPERTY_FONTCHARSET,      XML_NAMESPACE_STYLE,    XML_FONT_CHARSET,               XML_TYPE_TEXT_FONTENCODING, 0 ),
        //MAP_CONST( PROPERTY_FONTFAMILY,           XML_NAMESPACE_STYLE,    XML_FONT_FAMILY_GENERIC,        XML_TYPE_TEXT_FONTFAMILY, 0 ),
        //MAP_CONST( PROPERTY_FONTHEIGHT,           XML_NAMESPACE_FO,       XML_FONT_SIZE,                  XML_TYPE_MEASURE16, 0 ),
        //MAP_CONST( PROPERTY_FONTKERNING,      XML_NAMESPACE_STYLE,    XML_LETTER_KERNING,             XML_TYPE_BOOL, 0 ),
        //
        //MAP_CONST( PROPERTY_FONTORIENTATION,  XML_NAMESPACE_STYLE,    XML_ROTATION_ANGLE,             XML_TYPE_ROTATION_ANGLE, 0 ),
        //MAP_CONST( PROPERTY_FONTPITCH,            XML_NAMESPACE_STYLE,    XML_FONT_PITCH,                 XML_TYPE_TEXT_FONTPITCH, 0 ),
        //MAP_CONST( PROPERTY_FONTSLANT,            XML_NAMESPACE_FO,       XML_FONT_STYLE,                 XML_TYPE_TEXT_POSTURE, 0 ),
        //MAP_CONST_ASCII( "CharStrikeout",     XML_NAMESPACE_STYLE,    XML_TEXT_LINE_THROUGH_STYLE,    XML_TYPE_TEXT_CROSSEDOUT_STYLE|MID_FLAG_MERGE_PROPERTY, 0),
        //MAP_CONST_ASCII( "CharStrikeout",     XML_NAMESPACE_STYLE,    XML_TEXT_LINE_THROUGH_TYPE,     XML_TYPE_TEXT_CROSSEDOUT_TYPE|MID_FLAG_MERGE_PROPERTY,  0),
        //MAP_CONST_ASCII( "CharStrikeout",     XML_NAMESPACE_STYLE,    XML_TEXT_LINE_THROUGH_WIDTH,    XML_TYPE_TEXT_CROSSEDOUT_WIDTH|MID_FLAG_MERGE_PROPERTY, 0),
        //MAP_CONST_ASCII( "CharStrikeout",     XML_NAMESPACE_STYLE,    XML_TEXT_LINE_THROUGH_TEXT,     XML_TYPE_TEXT_CROSSEDOUT_TEXT|MID_FLAG_MERGE_PROPERTY,  0),
        //MAP_CONST( PROPERTY_FONTSTYLENAME,        XML_NAMESPACE_STYLE,    XML_FONT_STYLE_NAME,            XML_TYPE_STRING, 0 ),
        //MAP_CONST_ASCII( "CharUnderline",     XML_NAMESPACE_STYLE,    XML_TEXT_UNDERLINE_STYLE,       XML_TYPE_TEXT_UNDERLINE_STYLE|MID_FLAG_MERGE_PROPERTY, 0 ),
        //MAP_CONST_ASCII( "CharUnderline",     XML_NAMESPACE_STYLE,    XML_TEXT_UNDERLINE_TYPE,        XML_TYPE_TEXT_UNDERLINE_TYPE|MID_FLAG_MERGE_PROPERTY, 0 ),
        //MAP_CONST_ASCII( "CharUnderline",     XML_NAMESPACE_STYLE,    XML_TEXT_UNDERLINE_WIDTH,       XML_TYPE_TEXT_UNDERLINE_WIDTH|MID_FLAG_MERGE_PROPERTY, 0 ),
        //MAP_CONST_ASCII( "CharUnderlineColor",    XML_NAMESPACE_STYLE,    XML_TEXT_UNDERLINE_COLOR,       XML_TYPE_TEXT_UNDERLINE_COLOR|MID_FLAG_MULTI_PROPERTY, 0    ),
        //MAP_CONST_ASCII( "CharUnderlineHasColor",XML_NAMESPACE_STYLE, XML_TEXT_UNDERLINE_COLOR,       XML_TYPE_TEXT_UNDERLINE_HASCOLOR|MID_FLAG_MERGE_ATTRIBUTE, 0    ),
        //MAP_CONST( PROPERTY_FONTWEIGHT,           XML_NAMESPACE_FO,       XML_FONT_WEIGHT,                XML_TYPE_TEXT_WEIGHT, 0 ),
        //MAP_CONST( PROPERTY_FONTWIDTH,            XML_NAMESPACE_STYLE,    XML_FONT_WIDTH,                 XML_TYPE_FONT_WIDTH, 0 ),
        //MAP_CONST( PROPERTY_FONTWORDLINEMODE, XML_NAMESPACE_STYLE,    XML_TEXT_UNDERLINE_MODE,        XML_TYPE_TEXT_LINE_MODE, 0 ),
        MAP_END()
    };
    UniReference < XMLPropertyHandlerFactory> xFac = new ::xmloff::OControlPropertyHandlerFactory();
    return new XMLPropertySetMapper((XMLPropertyMapEntry*)s_aTableStylesProperties, xFac);
}
// -----------------------------------------------------------------------------
UniReference < XMLPropertySetMapper > OXMLHelper::GetColumnStylesPropertySetMapper()
{
#define MAP_CONST_COLUMN( name, prefix, token, type, context )  { name.ascii, name.length,  prefix, token, type|XML_TYPE_PROP_TABLE_COLUMN, context }
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
#define MAP_CONST_CELL( name, prefix, token, type, context )  { name.ascii, name.length,    prefix, token, type|XML_TYPE_PROP_PARAGRAPH, context }
#define MAP_CONST_TEXT( name, prefix, token, type, context )  { name.ascii, name.length,    prefix, token, type|XML_TYPE_PROP_TEXT, context }
#define MAP_CONST_TEXT_ASCII( name, prefix, token, type, context ) { name, sizeof(name)-1,  prefix, token, type|XML_TYPE_PROP_TEXT, context }
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
#define MAP_CONST_ROW( name, prefix, token, type, context )  { name.ascii, name.length, prefix, token, type|XML_TYPE_PROP_TABLE_ROW, context }
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

