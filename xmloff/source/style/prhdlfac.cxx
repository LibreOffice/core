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

#include <com/sun/star/drawing/ColorMode.hpp>
#include <com/sun/star/text/HorizontalAdjust.hpp>
#include <com/sun/star/text/WritingMode2.hpp>

#include <tools/debug.hxx>

#include <xmloff/prhdlfac.hxx>
#include <xmloff/xmltypes.hxx>
#include <xmloff/xmltoken.hxx>
#include "xmlbahdl.hxx"
#include <xmloff/NamedBoolPropertyHdl.hxx>
#include <xmloff/XMLConstantsPropertyHandler.hxx>
#include "cdouthdl.hxx"
#include "csmaphdl.hxx"
#include "fonthdl.hxx"
#include "kernihdl.hxx"
#include <postuhdl.hxx>
#include "shadwhdl.hxx"
#include "shdwdhdl.hxx"
#include "undlihdl.hxx"
#include "weighhdl.hxx"
#include "breakhdl.hxx"
#include <adjushdl.hxx>
#include <escphdl.hxx>
#include <chrhghdl.hxx>
#include <chrlohdl.hxx>
#include <lspachdl.hxx>
#include <bordrhdl.hxx>
#include <tabsthdl.hxx>
#include <xmloff/EnumPropertyHdl.hxx>
#include "AttributeContainerHandler.hxx"
#include "durationhdl.hxx"
#include "XMLRectangleMembersHandler.hxx"
#include "DrawAspectHdl.hxx"

using namespace ::com::sun::star;
using namespace ::xmloff::token;

SvXMLEnumMapEntry aXML_ColorMode_EnumMap[] =
{
    { XML_GREYSCALE,    drawing::ColorMode_GREYS },
    { XML_MONO,         drawing::ColorMode_MONO },
    { XML_WATERMARK,    drawing::ColorMode_WATERMARK },
    { XML_STANDARD,     drawing::ColorMode_STANDARD },
    { XML_TOKEN_INVALID, 0 }
};

SvXMLEnumMapEntry const aXML_HorizontalAdjust_Enum[] =
{
    { XML_LEFT,     text::HorizontalAdjust_LEFT },
    { XML_CENTER,   text::HorizontalAdjust_CENTER },
    { XML_RIGHT,    text::HorizontalAdjust_RIGHT },
    { XML_TOKEN_INVALID, 0 }
};

// aXML_WritingDirection_Enum is used with and without 'page'
// attribute, so you'll find uses of aXML_WritingDirection_Enum
// directly, as well as &(aXML_WritingDirection_Enum[1])
SvXMLEnumMapEntry const aXML_WritingDirection_Enum[] =
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

// Dtor
XMLPropertyHandlerFactory::~XMLPropertyHandlerFactory()
{
    for( CacheMap::iterator pPos = maHandlerCache.begin(); pPos != maHandlerCache.end(); ++pPos )
        delete pPos->second;
}

// Interface
const XMLPropertyHandler* XMLPropertyHandlerFactory::GetPropertyHandler( sal_Int32 nType ) const
{
    DBG_ASSERT( (nType & ~((sal_uInt32)MID_FLAG_MASK)) == 0,
                "GetPropertyHandler called with flags in type" );
    return GetBasicHandler( nType );
}

// Helper-methods to create and cache PropertyHandler
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
        case XML_TYPE_BOOL_FALSE :
            pPropHdl = new XMLBoolFalsePropHdl;
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
        case XML_TYPE_DOUBLE_PERCENT :
            pPropHdl = new XMLDoublePercentPropHdl();
            break;
        case XML_TYPE_NEG_PERCENT :
            pPropHdl = new XMLNegPercentPropHdl( 4 );
            break;
        case XML_TYPE_NEG_PERCENT8 :
            pPropHdl = new XMLNegPercentPropHdl( 1 );
            break;
        case XML_TYPE_NEG_PERCENT16 :
            pPropHdl = new XMLNegPercentPropHdl( 2 );
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
        case XML_TYPE_HEX :
            pPropHdl = new XMLHexPropHdl;
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

        case XML_TYPE_TEXT_CROSSEDOUT_TYPE:
            pPropHdl = new XMLCrossedOutTypePropHdl ;
            break;
        case XML_TYPE_TEXT_CROSSEDOUT_STYLE:
            pPropHdl = new XMLCrossedOutStylePropHdl ;
            break;
        case XML_TYPE_TEXT_CROSSEDOUT_WIDTH:
            pPropHdl = new XMLCrossedOutWidthPropHdl ;
            break;
        case XML_TYPE_TEXT_CROSSEDOUT_TEXT:
            pPropHdl = new XMLCrossedOutTextPropHdl ;
            break;
        case XML_TYPE_TEXT_BOOLCROSSEDOUT:
            pPropHdl = new XMLNamedBoolPropertyHdl(
                GetXMLToken(XML_SOLID),
                GetXMLToken(XML_NONE) );
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
        case XML_TYPE_TEXT_UNDERLINE_TYPE:
            pPropHdl = new XMLUnderlineTypePropHdl;
            break;
        case XML_TYPE_TEXT_UNDERLINE_STYLE:
            pPropHdl = new XMLUnderlineStylePropHdl;
            break;
        case XML_TYPE_TEXT_UNDERLINE_WIDTH:
            pPropHdl = new XMLUnderlineWidthPropHdl;
            break;
        case XML_TYPE_TEXT_UNDERLINE_COLOR:
            pPropHdl = new XMLColorTransparentPropHdl( XML_FONT_COLOR );
            break;
        case XML_TYPE_TEXT_UNDERLINE_HASCOLOR:
            pPropHdl = new XMLIsTransparentPropHdl( XML_FONT_COLOR,
                                                     sal_False );
            break;
        case XML_TYPE_TEXT_OVERLINE_TYPE:
            pPropHdl = new XMLUnderlineTypePropHdl;
            break;
        case XML_TYPE_TEXT_OVERLINE_STYLE:
            pPropHdl = new XMLUnderlineStylePropHdl;
            break;
        case XML_TYPE_TEXT_OVERLINE_WIDTH:
            pPropHdl = new XMLUnderlineWidthPropHdl;
            break;
        case XML_TYPE_TEXT_OVERLINE_COLOR:
            pPropHdl = new XMLColorTransparentPropHdl( XML_FONT_COLOR );
            break;
        case XML_TYPE_TEXT_OVERLINE_HASCOLOR:
            pPropHdl = new XMLIsTransparentPropHdl( XML_FONT_COLOR,
                                                     sal_False );
            break;
        case XML_TYPE_TEXT_WEIGHT:
            pPropHdl = new XMLFontWeightPropHdl;
            break;
        case XML_TYPE_TEXT_SPLIT:
            pPropHdl = new XMLNamedBoolPropertyHdl(
                GetXMLToken(XML_AUTO),
                GetXMLToken(XML_ALWAYS) );
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
        case XML_TYPE_CHAR_RFC_LANGUAGE_TAG:
            pPropHdl = new XMLCharRfcLanguageTagHdl;
            break;
        case XML_TYPE_CHAR_LANGUAGE:
            pPropHdl = new XMLCharLanguageHdl;
            break;
        case XML_TYPE_CHAR_SCRIPT:
            pPropHdl = new XMLCharScriptHdl;
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
        case XML_TYPE_TEXT_HIDDEN_AS_DISPLAY:
            pPropHdl = new XMLNamedBoolPropertyHdl(
                GetXMLToken(XML_NONE),
                GetXMLToken(XML_TRUE) );
            break;
        case XML_TYPE_STYLENAME :
            pPropHdl = new XMLStyleNamePropHdl;
            break;
        case XML_TYPE_NUMBER_NO_ZERO:
            pPropHdl = new XMLNumberWithoutZeroPropHdl( 4 );
            break;
        case XML_TYPE_NUMBER8_NO_ZERO:
            pPropHdl = new XMLNumberWithoutZeroPropHdl( 1 );
            break;
        case XML_TYPE_NUMBER16_NO_ZERO:
            pPropHdl = new XMLNumberWithoutZeroPropHdl( 2 );
            break;
        case XML_TYPE_NUMBER16_AUTO:
            pPropHdl = new XMLNumberWithAutoInsteadZeroPropHdl();
            break;
    }

    return pPropHdl;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
