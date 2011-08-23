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

#ifndef _COM_SUN_STAR_DRAWING_COLORMODE_HPP_
#include <com/sun/star/drawing/ColorMode.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_HORIZONTALADJUST_HPP_
#include <com/sun/star/text/HorizontalAdjust.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_WRITINGMODE2_HPP_
#include <com/sun/star/text/WritingMode2.hpp>
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _XMLOFF_PROPERTYHANDLERFACTORY_HXX
#include <prhdlfac.hxx>
#endif

#ifndef _XMLOFF_XMLTYPES_HXX
#include "xmltypes.hxx"
#endif


#ifndef _XMLOFF_PROPERTYHANDLER_BASICTYPES_HXX
#include "xmlbahdl.hxx"
#endif

#ifndef _XMLOFF_NAMEDBOOLPROPERTYHANDLER_HXX
#include "NamedBoolPropertyHdl.hxx"
#endif

#ifndef _XMLOFF_XMLCONSTANTSPROPERTYHANDLER_HXX
#include "XMLConstantsPropertyHandler.hxx"
#endif

#ifndef _XMLOFF_PROPERTYHANDLER_CROSSEDOUTTYPES_HXX
#include "cdouthdl.hxx"
#endif
#ifndef _XMLOFF_PROPERTYHANDLER_CASEMAPTYPES_HXX
#include "csmaphdl.hxx"
#endif
#ifndef _XMLOFF_PROPERTYHANDLER_FONTTYPES_HXX
#include "fonthdl.hxx"
#endif
#ifndef _XMLOFF_PROPERTYHANDLER_KERNINGTYPES_HXX
#include "kernihdl.hxx"
#endif
#ifndef _XMLOFF_PROPERTYHANDLER_POSTURETYPES_HXX
#include <postuhdl.hxx>
#endif
#ifndef _XMLOFF_PROPERTYHANDLER_SHADOWTYPES_HXX
#include "shadwhdl.hxx"
#endif
#ifndef _XMLOFF_PROPERTYHANDLER_SHADOWEDTYPES_HXX
#include "shdwdhdl.hxx"
#endif
#ifndef _XMLOFF_PROPERTYHANDLER_UNDERLINETYPES_HXX
#include "undlihdl.hxx"
#endif
#ifndef _XMLOFF_PROPERTYHANDLER_FONTWEIGHTTYPES_HXX
#include "weighhdl.hxx"
#endif
#ifndef _XMLOFF_PROPERTYHANDLER_FMTSPLITTYPES_HXX
#include "splithdl.hxx"
#endif
#ifndef _XMLOFF_PROPERTYHANDLER_BREAKTYPES_HXX
#include "breakhdl.hxx"
#endif
#ifndef _XMLOFF_PROPERTYHANDLER_ADJUSTTYPES_HXX
#include <adjushdl.hxx>
#endif
#ifndef _XMLOFF_PROPERTYHANDLER_ESCAPETYPES_HXX
#include <escphdl.hxx>
#endif
#ifndef _XMLOFF_PROPERTYHANDLER_CHARHEIGHTTYPES_HXX
#include <chrhghdl.hxx>
#endif
#ifndef _XMLOFF_PROPERTYHANDLER_CHARLOCALETYPES_HXX
#include <chrlohdl.hxx>
#endif
#ifndef _XMLOFF_PROPERTYHANDLER_LINESPACETYPES_HXX
#include <lspachdl.hxx>
#endif
#ifndef _XMLOFF_PROPERTYHANDLER_BORDERTYPES_HXX
#include <bordrhdl.hxx>
#endif
#ifndef _XMLOFF_PROPERTYHANDLER_TABSTOPTYPES_HXX
#include <tabsthdl.hxx>
#endif
#ifndef _XMLOFF_ENUMPROPERTYHANDLER_HXX 
#include "EnumPropertyHdl.hxx"
#endif
#ifndef _XMLOFF_ATTRIBUTECONTAINERHANDLER_HXX
#include "AttributeContainerHandler.hxx"
#endif
#ifndef _XMLOFF_PROPERTYHANDLER_DURATIONS_HXX
#include "durationhdl.hxx"
#endif
#ifndef _XMLOFF_XMLRECTANGLEMEMBERSHANDLER_HXX
#include "XMLRectangleMembersHandler.hxx"
#endif
#ifndef _XMLOFF_DRAWASPECTHDL_HXX
#include "DrawAspectHdl.hxx"
#endif
namespace binfilter {

using namespace ::com::sun::star;
using namespace ::binfilter::xmloff::token;

SvXMLEnumMapEntry aXML_ColorMode_EnumMap[] =
{
    { XML_GREYSCALE,    drawing::ColorMode_GREYS },
    { XML_MONO,	        drawing::ColorMode_MONO },
    { XML_WATERMARK,    drawing::ColorMode_WATERMARK },
    { XML_STANDARD,     drawing::ColorMode_STANDARD },
    { XML_TOKEN_INVALID, 0 }
};

SvXMLEnumMapEntry __READONLY_DATA aXML_HorizontalAdjust_Enum[] =
{
    { XML_LEFT,	    text::HorizontalAdjust_LEFT },
    { XML_CENTER,	text::HorizontalAdjust_CENTER },
    { XML_RIGHT,	text::HorizontalAdjust_RIGHT },
    { XML_TOKEN_INVALID, 0 }
};

// aXML_WritingDirection_Enum is used with and without 'page'
// attribute, so you'll find uses of aXML_WritingDirection_Enum
// directly, as well as &(aXML_WritingDirection_Enum[1])
SvXMLEnumMapEntry __READONLY_DATA aXML_WritingDirection_Enum[] =
{
    // aXML_WritingDirection_Enum
    { XML_PAGE,     text::WritingMode2::PAGE },

    // &(aXML_WritingDirection_Enum[1])
    { XML_LR_TB,    text::WritingMode2::LR_TB },
    { XML_RL_TB,    text::WritingMode2::RL_TB },
    { XML_TB_RL,    text::WritingMode2::TB_RL },    
    { XML_TB_LR,    text::WritingMode2::TB_LR },

    // alternative names of the above, as accepted by XSL
    { XML_LR,       text::WritingMode2::LR_TB },
    { XML_RL,       text::WritingMode2::RL_TB },
    { XML_TB,       text::WritingMode2::TB_RL },

    { XML_TOKEN_INVALID, 0 }
};


///////////////////////////////////////////////////////////////////////////
//
// Dtor
//
XMLPropertyHandlerFactory::~XMLPropertyHandlerFactory()
{
    for( CacheMap::iterator pPos = maHandlerCache.begin(); pPos != maHandlerCache.end(); pPos++ )
        delete pPos->second;
}

///////////////////////////////////////////////////////////////////////////
//
// Interface
//
const XMLPropertyHandler* XMLPropertyHandlerFactory::GetPropertyHandler( sal_Int32 nType ) const
{
    DBG_ASSERT( (nType & ~((sal_uInt32)MID_FLAG_MASK)) == 0,
                "GetPropertyHandler called with flags in type" );
    return GetBasicHandler( nType );
}

///////////////////////////////////////////////////////////////////////////
//
// Helper-methods to create and cache PropertyHandler
//
XMLPropertyHandler* XMLPropertyHandlerFactory::GetHdlCache( sal_Int32 nType ) const
{
    XMLPropertyHandler* pRet = NULL;

    if( maHandlerCache.find( nType ) != maHandlerCache.end() )
        pRet = maHandlerCache.find( nType )->second;

    return pRet;
}

void XMLPropertyHandlerFactory::PutHdlCache( sal_Int32 nType, const XMLPropertyHandler* pHdl ) const
{
    // Don't be wondered about the following construct. The  sense is to be able to provide a const-
    // method as class-interface.
    ((XMLPropertyHandlerFactory*)this)->maHandlerCache[ nType ] = (XMLPropertyHandler*)pHdl;
}

const XMLPropertyHandler* XMLPropertyHandlerFactory::GetBasicHandler( sal_Int32 nType ) const
{
    const XMLPropertyHandler* pPropHdl = GetHdlCache( nType );

    if( !pPropHdl )
    {
        pPropHdl = CreatePropertyHandler( nType );

        if( pPropHdl )
            PutHdlCache( nType, pPropHdl );
    }

    return pPropHdl;
}

const XMLPropertyHandler* XMLPropertyHandlerFactory::CreatePropertyHandler( sal_Int32 nType )
{
    XMLPropertyHandler* pPropHdl = NULL;

    switch( nType )
    {
        case XML_TYPE_BOOL :
            pPropHdl = new XMLBoolPropHdl;
            break;
        case XML_TYPE_MEASURE :
            pPropHdl = new XMLMeasurePropHdl( 4 );
            break;
        case XML_TYPE_MEASURE8 :
            pPropHdl = new XMLMeasurePropHdl( 1 );
            break;
        case XML_TYPE_MEASURE16:
            pPropHdl = new XMLMeasurePropHdl( 2 );
            break;
        case XML_TYPE_PERCENT :
            pPropHdl = new XMLPercentPropHdl( 4 );
            break;
        case XML_TYPE_PERCENT8 :
            pPropHdl = new XMLPercentPropHdl( 1 );
            break;
        case XML_TYPE_PERCENT16 :
            pPropHdl = new XMLPercentPropHdl( 2 );
            break;
        case XML_TYPE_MEASURE_PX :
            pPropHdl = new XMLMeasurePxPropHdl( 4 );
            break;
        case XML_TYPE_STRING :
            pPropHdl = new XMLStringPropHdl;
            break;
        case XML_TYPE_COLOR :
            pPropHdl = new XMLColorPropHdl;
            break;
        case XML_TYPE_NUMBER :
            pPropHdl = new XMLNumberPropHdl( 4 );
            break;
        case XML_TYPE_NUMBER8 :
            pPropHdl = new XMLNumberPropHdl( 1 );
            break;
        case XML_TYPE_NUMBER16:
            pPropHdl = new XMLNumberPropHdl( 2 );
            break;
        case XML_TYPE_NUMBER_NONE :
            pPropHdl = new XMLNumberNonePropHdl;
            break;
        case XML_TYPE_NUMBER8_NONE :
            pPropHdl = new XMLNumberNonePropHdl( 1 );
            break;
        case XML_TYPE_NUMBER16_NONE :
            pPropHdl = new XMLNumberNonePropHdl( 2 );
            break;
        case XML_TYPE_DOUBLE :
            pPropHdl = new XMLDoublePropHdl;
            break;
        case XML_TYPE_NBOOL :
            pPropHdl = new XMLNBoolPropHdl;
            break;
        case XML_TYPE_COLORTRANSPARENT :
            pPropHdl = new XMLColorTransparentPropHdl;
            break;
        case XML_TYPE_ISTRANSPARENT :
            pPropHdl = new XMLIsTransparentPropHdl;
            break;
        case XML_TYPE_COLORAUTO :
            pPropHdl = new XMLColorAutoPropHdl;
            break;
        case XML_TYPE_ISAUTOCOLOR :
            pPropHdl = new XMLIsAutoColorPropHdl;
            break;
        case XML_TYPE_BUILDIN_CMP_ONLY :
            pPropHdl = new XMLCompareOnlyPropHdl;
            break;

        case XML_TYPE_RECTANGLE_LEFT :
        case XML_TYPE_RECTANGLE_TOP :
        case XML_TYPE_RECTANGLE_WIDTH :
        case XML_TYPE_RECTANGLE_HEIGHT :
            pPropHdl = new XMLRectangleMembersHdl( nType );
            break;

        case XML_TYPE_TEXT_CROSSEDOUT:
            pPropHdl = new XMLCrossedOutPropHdl ;
            break;
        case XML_TYPE_TEXT_BOOLCROSSEDOUT:
            pPropHdl = new XMLNamedBoolPropertyHdl( 
                GetXMLToken(XML_CROSSEDOUT_SINGLE), 
                GetXMLToken(XML_CROSSEDOUT_NONE)    );
            break;
        case XML_TYPE_TEXT_ESCAPEMENT:
            pPropHdl = new XMLEscapementPropHdl;
            break;
        case XML_TYPE_TEXT_ESCAPEMENT_HEIGHT:
            pPropHdl = new XMLEscapementHeightPropHdl;
            break;
        case XML_TYPE_TEXT_CASEMAP:
            pPropHdl = new XMLCaseMapPropHdl;
            break;
        case XML_TYPE_TEXT_CASEMAP_VAR:
            pPropHdl = new XMLCaseMapVariantHdl;
            break;
        case XML_TYPE_TEXT_FONTFAMILYNAME:
            pPropHdl = new XMLFontFamilyNamePropHdl;
            break;
        case XML_TYPE_TEXT_FONTFAMILY:
            pPropHdl = new XMLFontFamilyPropHdl;
            break;
        case XML_TYPE_TEXT_FONTENCODING:
            pPropHdl = new XMLFontEncodingPropHdl;
            break;
        case XML_TYPE_TEXT_FONTPITCH:
            pPropHdl = new XMLFontPitchPropHdl;
            break;
        case XML_TYPE_TEXT_KERNING:
            pPropHdl = new XMLKerningPropHdl;
            break;
        case XML_TYPE_TEXT_POSTURE:
            pPropHdl = new XMLPosturePropHdl;
            break;
        case XML_TYPE_TEXT_SHADOWED:
            pPropHdl = new XMLShadowedPropHdl;
            break;
        case XML_TYPE_TEXT_UNDERLINE:
            pPropHdl = new XMLUnderlinePropHdl;
            break;
        case XML_TYPE_TEXT_UNDERLINE_COLOR:
            pPropHdl = new XMLColorTransparentPropHdl( XML_FONT_COLOR );
            break;
        case XML_TYPE_TEXT_UNDERLINE_HASCOLOR:
            pPropHdl = new XMLIsTransparentPropHdl( XML_FONT_COLOR,
                                                     sal_False );
            break;
        case XML_TYPE_TEXT_WEIGHT:
            pPropHdl = new XMLFontWeightPropHdl;
            break;
        case XML_TYPE_TEXT_SPLIT:
            pPropHdl = new XMLFmtSplitPropHdl;
            break;
        case XML_TYPE_TEXT_BREAKBEFORE:
            pPropHdl = new XMLFmtBreakBeforePropHdl;
            break;
        case XML_TYPE_TEXT_BREAKAFTER:
            pPropHdl = new XMLFmtBreakAfterPropHdl;
            break;
        case XML_TYPE_TEXT_SHADOW:
            pPropHdl = new XMLShadowPropHdl;
            break;
        case XML_TYPE_TEXT_ADJUST:
            pPropHdl = new XMLParaAdjustPropHdl;
            break;
        case XML_TYPE_TEXT_ADJUSTLAST:
            pPropHdl = new XMLLastLineAdjustPropHdl;
            break;
        case XML_TYPE_CHAR_HEIGHT:
            pPropHdl = new XMLCharHeightHdl;
            break;
        case XML_TYPE_CHAR_HEIGHT_PROP:
            pPropHdl = new XMLCharHeightPropHdl;
            break;
        case XML_TYPE_CHAR_HEIGHT_DIFF:
            pPropHdl = new XMLCharHeightDiffHdl;
            break;
        case XML_TYPE_CHAR_LANGUAGE:
            pPropHdl = new XMLCharLanguageHdl;
            break;
        case XML_TYPE_CHAR_COUNTRY:
            pPropHdl = new XMLCharCountryHdl;
            break;
        case XML_TYPE_LINE_SPACE_FIXED:
            pPropHdl = new XMLLineHeightHdl;
            break;
        case XML_TYPE_LINE_SPACE_MINIMUM:
            pPropHdl = new XMLLineHeightAtLeastHdl;
            break;
        case XML_TYPE_LINE_SPACE_DISTANCE:
            pPropHdl = new XMLLineSpacingHdl;
            break;
        case XML_TYPE_BORDER_WIDTH:
            pPropHdl = new XMLBorderWidthHdl;
            break;
        case XML_TYPE_BORDER:
            pPropHdl = new XMLBorderHdl;
            break;
        case XML_TYPE_TEXT_TABSTOP:
            pPropHdl = new XMLTabStopPropHdl;
            break;
        case XML_TYPE_ATTRIBUTE_CONTAINER:
            pPropHdl = new XMLAttributeContainerHandler;
            break;
        case XML_TYPE_COLOR_MODE:
            pPropHdl = new XMLEnumPropertyHdl( aXML_ColorMode_EnumMap, 
                            ::getCppuType((const drawing::ColorMode*)0) );
            break;
        case XML_TYPE_DURATION16_MS:
            pPropHdl = new XMLDurationMS16PropHdl_Impl;
            break;
        case XML_TYPE_TEXT_HORIZONTAL_ADJUST:
            pPropHdl = new XMLEnumPropertyHdl( 
                aXML_HorizontalAdjust_Enum,
                ::getCppuType((const text::HorizontalAdjust*)0) );
            break;
        case XML_TYPE_TEXT_DRAW_ASPECT:
            pPropHdl = new DrawAspectHdl;
            break;
        case XML_TYPE_TEXT_WRITING_MODE:
            pPropHdl = new XMLConstantsPropertyHandler( 
                &(aXML_WritingDirection_Enum[1]),
                XML_LR_TB);
            break;
        case XML_TYPE_TEXT_WRITING_MODE_WITH_DEFAULT:
            pPropHdl = new XMLConstantsPropertyHandler( 
                aXML_WritingDirection_Enum,
                XML_PAGE);
            break;
    }

    return pPropHdl;
}
}//end of namespace binfilter
