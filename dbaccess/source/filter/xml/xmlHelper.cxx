/*************************************************************************
 *
 *  $RCSfile: xmlHelper.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 15:21:40 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the License); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an AS IS basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
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

namespace dbaxml
{
    using namespace ::xmloff::token;
    using namespace ::com::sun::star::awt;

    OPropertyHandlerFactory::OPropertyHandlerFactory()
{
}
// -----------------------------------------------------------------------------
OPropertyHandlerFactory::~OPropertyHandlerFactory()
{
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
#define MAP_CONST( name, prefix, token, type, context )  { name.ascii, name.length, prefix, token, type, context }
#define MAP_CONST_ASCII( name, prefix, token, type, context )  { name, sizeof(name)-1,  prefix, token, type, context }
#define MAP_END()   { NULL, 0, 0, XML_TOKEN_INVALID, 0 }
// -----------------------------------------------------------------------------
UniReference < XMLPropertySetMapper > OXMLHelper::GetTableStylesPropertySetMapper()
{
    static const XMLPropertyMapEntry s_aTableStylesProperties[] =
    {
        MAP_CONST( PROPERTY_FONTNAME,           XML_NAMESPACE_STYLE,    XML_FONT_NAME,              XML_TYPE_STRING, 0 ),
        MAP_CONST( PROPERTY_ROW_HEIGHT,         XML_NAMESPACE_STYLE,    XML_ROW_HEIGHT,             XML_TYPE_MEASURE, CTF_DB_ROWHEIGHT),
        MAP_CONST( PROPERTY_TEXTCOLOR,          XML_NAMESPACE_FO,       XML_COLOR,                  XML_TYPE_COLOR, 0 ),
        MAP_CONST( PROPERTY_TEXTLINECOLOR,      XML_NAMESPACE_STYLE,    XML_TEXT_UNDERLINE_COLOR,   XML_TYPE_TEXT_UNDERLINE_COLOR|MID_FLAG_MULTI_PROPERTY, 0 ),

        MAP_CONST( PROPERTY_TEXTRELIEF,         XML_NAMESPACE_STYLE,    XML_FONT_RELIEF,            XML_TYPE_TEXT_FONT_RELIEF|MID_FLAG_MULTI_PROPERTY, 0 ),
        MAP_CONST( PROPERTY_TEXTEMPHASIS,       XML_NAMESPACE_STYLE,    XML_TEXT_EMPHASIZE,         XML_TYPE_CONTROL_TEXT_EMPHASIZE, 0 ),
        MAP_CONST( PROPERTY_FONTCHARWIDTH,      XML_NAMESPACE_STYLE,    XML_FONT_CHAR_WIDTH,        XML_TYPE_NUMBER16, 0 ),
        MAP_CONST( PROPERTY_FONTCHARSET,        XML_NAMESPACE_STYLE,    XML_FONT_CHARSET,           XML_TYPE_TEXT_FONTENCODING, 0 ),
        MAP_CONST( PROPERTY_FONTFAMILY,         XML_NAMESPACE_STYLE,    XML_FONT_FAMILY_GENERIC,    XML_TYPE_TEXT_FONTFAMILY, 0 ),
        MAP_CONST( PROPERTY_FONTHEIGHT,         XML_NAMESPACE_FO,       XML_FONT_SIZE,              XML_TYPE_MEASURE16, 0 ),
        MAP_CONST( PROPERTY_FONTKERNING,        XML_NAMESPACE_STYLE,    XML_LETTER_KERNING,         XML_TYPE_BOOL, 0 ),

        MAP_CONST( PROPERTY_FONTORIENTATION,    XML_NAMESPACE_STYLE,    XML_ROTATION_ANGLE,         XML_TYPE_ROTATION_ANGLE, 0 ),
        MAP_CONST( PROPERTY_FONTPITCH,          XML_NAMESPACE_STYLE,    XML_FONT_PITCH,             XML_TYPE_TEXT_FONTPITCH, 0 ),
        MAP_CONST( PROPERTY_FONTSLANT,          XML_NAMESPACE_FO,       XML_FONT_STYLE,             XML_TYPE_TEXT_POSTURE, 0 ),
        MAP_CONST_ASCII( "CharStrikeout",               XML_NAMESPACE_STYLE,    XML_TEXT_LINE_THROUGH_STYLE,    XML_TYPE_TEXT_CROSSEDOUT_STYLE|MID_FLAG_MERGE_PROPERTY, 0),
        MAP_CONST_ASCII( "CharStrikeout",               XML_NAMESPACE_STYLE,    XML_TEXT_LINE_THROUGH_TYPE,     XML_TYPE_TEXT_CROSSEDOUT_TYPE|MID_FLAG_MERGE_PROPERTY,  0),
        MAP_CONST_ASCII( "CharStrikeout",               XML_NAMESPACE_STYLE,    XML_TEXT_LINE_THROUGH_WIDTH,    XML_TYPE_TEXT_CROSSEDOUT_WIDTH|MID_FLAG_MERGE_PROPERTY, 0),
        MAP_CONST_ASCII( "CharStrikeout",               XML_NAMESPACE_STYLE,    XML_TEXT_LINE_THROUGH_TEXT,     XML_TYPE_TEXT_CROSSEDOUT_TEXT|MID_FLAG_MERGE_PROPERTY,  0),
        MAP_CONST( PROPERTY_FONTSTYLENAME,      XML_NAMESPACE_STYLE,    XML_FONT_STYLE_NAME,        XML_TYPE_STRING, 0 ),
        MAP_CONST_ASCII( "CharUnderline",               XML_NAMESPACE_STYLE,    XML_TEXT_UNDERLINE_STYLE,       XML_TYPE_TEXT_UNDERLINE_STYLE|MID_FLAG_MERGE_PROPERTY, 0 ),
        MAP_CONST_ASCII( "CharUnderline",               XML_NAMESPACE_STYLE,    XML_TEXT_UNDERLINE_TYPE,        XML_TYPE_TEXT_UNDERLINE_TYPE|MID_FLAG_MERGE_PROPERTY, 0 ),
        MAP_CONST_ASCII( "CharUnderline",               XML_NAMESPACE_STYLE,    XML_TEXT_UNDERLINE_WIDTH,       XML_TYPE_TEXT_UNDERLINE_WIDTH|MID_FLAG_MERGE_PROPERTY, 0 ),
        MAP_CONST_ASCII( "CharUnderlineColor",      XML_NAMESPACE_STYLE,    XML_TEXT_UNDERLINE_COLOR,       XML_TYPE_TEXT_UNDERLINE_COLOR|MID_FLAG_MULTI_PROPERTY, 0    ),
        MAP_CONST_ASCII( "CharUnderlineHasColor",       XML_NAMESPACE_STYLE,    XML_TEXT_UNDERLINE_COLOR,       XML_TYPE_TEXT_UNDERLINE_HASCOLOR|MID_FLAG_MERGE_ATTRIBUTE, 0    ),
        MAP_CONST( PROPERTY_FONTWEIGHT,         XML_NAMESPACE_FO,       XML_FONT_WEIGHT,            XML_TYPE_TEXT_WEIGHT, 0 ),
        MAP_CONST( PROPERTY_FONTWIDTH,          XML_NAMESPACE_STYLE,    XML_FONT_WIDTH,             XML_TYPE_FONT_WIDTH, 0 ),
        MAP_CONST( PROPERTY_FONTWORDLINEMODE,   XML_NAMESPACE_FO,       XML_SCORE_SPACES,           XML_TYPE_NBOOL, 0 ),
        MAP_END()
    };
    UniReference < XMLPropertyHandlerFactory> xFac = new ::xmloff::OControlPropertyHandlerFactory();
    return new XMLPropertySetMapper((XMLPropertyMapEntry*)s_aTableStylesProperties, xFac);
}
// -----------------------------------------------------------------------------
UniReference < XMLPropertySetMapper > OXMLHelper::GetColumnStylesPropertySetMapper()
{
    static const XMLPropertyMapEntry s_aColumnStylesProperties[] =
    {
        MAP_CONST( PROPERTY_WIDTH,          XML_NAMESPACE_STYLE,    XML_COLUMN_WIDTH,       XML_TYPE_MEASURE, 0),
        MAP_CONST( PROPERTY_HIDDEN,         XML_NAMESPACE_TABLE,    XML_DISPLAY,            XML_DB_TYPE_EQUAL|MID_FLAG_SPECIAL_ITEM, CTF_DB_ISVISIBLE ),
        MAP_CONST( PROPERTY_ALIGN,          XML_NAMESPACE_STYLE,    XML_TEXT_ALIGN,         XML_TYPE_TEXT_ALIGN, 0 ),
        MAP_CONST( PROPERTY_NUMBERFORMAT,   XML_NAMESPACE_STYLE,    XML_DATA_STYLE_NAME,    XML_TYPE_NUMBER|MID_FLAG_SPECIAL_ITEM, CTF_DB_NUMBERFORMAT),
        MAP_END()
    };
    UniReference < XMLPropertyHandlerFactory> xFac = new OPropertyHandlerFactory();
    return new XMLPropertySetMapper((XMLPropertyMapEntry*)s_aColumnStylesProperties, xFac);
}
// -----------------------------------------------------------------------------
}

