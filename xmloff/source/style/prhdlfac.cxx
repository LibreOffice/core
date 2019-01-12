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

#include <xmloff/prhdlfac.hxx>

#include <com/sun/star/drawing/ColorMode.hpp>
#include <com/sun/star/text/HorizontalAdjust.hpp>
#include <com/sun/star/text/WritingMode2.hpp>
#include <com/sun/star/text/VertOrientation.hpp>


#include <sal/log.hxx>
#include <xmloff/xmlement.hxx>
#include <xmloff/xmltypes.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlprhdl.hxx>
#include "xmlbahdl.hxx"
#include <xmloff/NamedBoolPropertyHdl.hxx>
#include <xmloff/XMLConstantsPropertyHandler.hxx>
#include "cdouthdl.hxx"
#include "csmaphdl.hxx"
#include "fonthdl.hxx"
#include "kernihdl.hxx"
#include "postuhdl.hxx"
#include "shadwhdl.hxx"
#include "shdwdhdl.hxx"
#include "undlihdl.hxx"
#include "weighhdl.hxx"
#include "breakhdl.hxx"
#include "adjushdl.hxx"
#include "escphdl.hxx"
#include "chrhghdl.hxx"
#include "chrlohdl.hxx"
#include "lspachdl.hxx"
#include "bordrhdl.hxx"
#include "tabsthdl.hxx"
#include <xmloff/EnumPropertyHdl.hxx>
#include <AttributeContainerHandler.hxx>
#include "durationhdl.hxx"
#include <XMLRectangleMembersHandler.hxx>
#include "DrawAspectHdl.hxx"

#include <map>

using namespace ::com::sun::star;
using namespace ::xmloff::token;

static SvXMLEnumMapEntry<drawing::ColorMode> const aXML_ColorMode_EnumMap[] =
{
    { XML_GREYSCALE,    drawing::ColorMode_GREYS },
    { XML_MONO,         drawing::ColorMode_MONO },
    { XML_WATERMARK,    drawing::ColorMode_WATERMARK },
    { XML_STANDARD,     drawing::ColorMode_STANDARD },
    { XML_TOKEN_INVALID, drawing::ColorMode(0) }
};

static SvXMLEnumMapEntry<text::HorizontalAdjust> const aXML_HorizontalAdjust_Enum[] =
{
    { XML_LEFT,     text::HorizontalAdjust_LEFT },
    { XML_CENTER,   text::HorizontalAdjust_CENTER },
    { XML_RIGHT,    text::HorizontalAdjust_RIGHT },
    { XML_TOKEN_INVALID, text::HorizontalAdjust(0) }
};

// aXML_WritingDirection_Enum is used with and without 'page'
// attribute, so you'll find uses of aXML_WritingDirection_Enum
// directly, as well as &(aXML_WritingDirection_Enum[1])
static SvXMLEnumMapEntry<sal_uInt16> const aXML_WritingDirection_Enum[] =
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

static SvXMLEnumMapEntry<sal_uInt16> const pXML_VertPos_Enum[] =
{
    { XML_FROM_TOP,         text::VertOrientation::NONE       },
    { XML_TOP,              text::VertOrientation::TOP        },
    { XML_TOP,              text::VertOrientation::CHAR_TOP   },  // export only
    { XML_TOP,              text::VertOrientation::LINE_TOP   },  // export only
    { XML_MIDDLE,           text::VertOrientation::CENTER     },
    { XML_MIDDLE,           text::VertOrientation::CHAR_CENTER    },  // export only
    { XML_MIDDLE,           text::VertOrientation::LINE_CENTER    },  // export only
    { XML_BOTTOM,           text::VertOrientation::BOTTOM         },
    { XML_BOTTOM,           text::VertOrientation::CHAR_BOTTOM    },  // export only
    { XML_BOTTOM,           text::VertOrientation::LINE_BOTTOM    },  // export only
    { XML_BELOW,            text::VertOrientation::CHAR_BOTTOM    },  // import only
    { XML_TOKEN_INVALID, 0 }
};

typedef std::map<sal_Int32, const XMLPropertyHandler*> CacheMap;

struct XMLPropertyHandlerFactory::Impl
{
    mutable CacheMap maHandlerCache;
};

XMLPropertyHandlerFactory::XMLPropertyHandlerFactory() :
    mpImpl(new Impl) {}

XMLPropertyHandlerFactory::~XMLPropertyHandlerFactory()
{
    for( auto& rCacheEntry : mpImpl->maHandlerCache )
        delete rCacheEntry.second;
}

// Interface
const XMLPropertyHandler* XMLPropertyHandlerFactory::GetPropertyHandler( sal_Int32 nType ) const
{
    SAL_WARN_IF( (nType & ~(sal_uInt32(MID_FLAG_MASK))) != 0, "xmloff",
                "GetPropertyHandler called with flags in type" );
    return GetBasicHandler( nType );
}

// Helper-methods to create and cache PropertyHandler
const XMLPropertyHandler* XMLPropertyHandlerFactory::GetHdlCache( sal_Int32 nType ) const
{
    const XMLPropertyHandler* pRet = nullptr;

    if( mpImpl->maHandlerCache.find( nType ) != mpImpl->maHandlerCache.end() )
        pRet = mpImpl->maHandlerCache.find( nType )->second;

    return pRet;
}

void XMLPropertyHandlerFactory::PutHdlCache( sal_Int32 nType, const XMLPropertyHandler* pHdl ) const
{
    mpImpl->maHandlerCache[nType] = pHdl;
}

const XMLPropertyHandler* XMLPropertyHandlerFactory::GetBasicHandler( sal_Int32 nType ) const
{
    const XMLPropertyHandler* pPropHdl = GetHdlCache( nType );
    if( pPropHdl )
        return pPropHdl;

    std::unique_ptr<XMLPropertyHandler> pNewPropHdl = CreatePropertyHandler( nType );
    if( pNewPropHdl )
        PutHdlCache( nType, pNewPropHdl.get() );
    return pNewPropHdl.release();
}

std::unique_ptr<XMLPropertyHandler> XMLPropertyHandlerFactory::CreatePropertyHandler( sal_Int32 nType )
{
    std::unique_ptr<XMLPropertyHandler> pPropHdl;

    switch( nType )
    {
        case XML_TYPE_BOOL :
            pPropHdl.reset(new XMLBoolPropHdl);
            break;
        case XML_TYPE_BOOL_FALSE :
            pPropHdl.reset(new XMLBoolFalsePropHdl);
            break;
        case XML_TYPE_MEASURE :
            pPropHdl.reset(new XMLMeasurePropHdl( 4 ));
            break;
        case XML_TYPE_MEASURE8 :
            pPropHdl.reset(new XMLMeasurePropHdl( 1 ));
            break;
        case XML_TYPE_MEASURE16:
            pPropHdl.reset(new XMLMeasurePropHdl( 2 ));
            break;
        case XML_TYPE_PERCENT :
            pPropHdl.reset(new XMLPercentPropHdl( 4 ));
            break;
        case XML_TYPE_PERCENT8 :
            pPropHdl.reset(new XMLPercentPropHdl( 1 ));
            break;
        case XML_TYPE_PERCENT16 :
            pPropHdl.reset(new XMLPercentPropHdl( 2 ));
            break;
        case XML_TYPE_DOUBLE_PERCENT :
            pPropHdl.reset(new XMLDoublePercentPropHdl);
            break;
        case XML_TYPE_NEG_PERCENT :
            pPropHdl.reset(new XMLNegPercentPropHdl( 4 ));
            break;
        case XML_TYPE_NEG_PERCENT8 :
            pPropHdl.reset(new XMLNegPercentPropHdl( 1 ));
            break;
        case XML_TYPE_NEG_PERCENT16 :
            pPropHdl.reset(new XMLNegPercentPropHdl( 2 ));
            break;
        case XML_TYPE_MEASURE_PX :
            pPropHdl.reset(new XMLMeasurePxPropHdl( 4 ));
            break;
        case XML_TYPE_STRING :
            pPropHdl.reset(new XMLStringPropHdl);
            break;
        case XML_TYPE_COLOR :
            pPropHdl.reset(new XMLColorPropHdl);
            break;
        case XML_TYPE_HEX :
            pPropHdl.reset(new XMLHexPropHdl);
            break;
        case XML_TYPE_NUMBER :
            pPropHdl.reset(new XMLNumberPropHdl( 4 ));
            break;
        case XML_TYPE_NUMBER8 :
            pPropHdl.reset(new XMLNumberPropHdl( 1 ));
            break;
        case XML_TYPE_NUMBER16:
            pPropHdl.reset(new XMLNumberPropHdl( 2 ));
            break;
        case XML_TYPE_NUMBER_NONE :
            pPropHdl.reset(new XMLNumberNonePropHdl);
            break;
        case XML_TYPE_NUMBER8_NONE :
            pPropHdl.reset(new XMLNumberNonePropHdl( 1 ));
            break;
        case XML_TYPE_NUMBER16_NONE :
            pPropHdl.reset(new XMLNumberNonePropHdl( 2 ));
            break;
        case XML_TYPE_DOUBLE :
            pPropHdl.reset(new XMLDoublePropHdl);
            break;
        case XML_TYPE_NBOOL :
            pPropHdl.reset(new XMLNBoolPropHdl);
            break;
        case XML_TYPE_COLORTRANSPARENT :
            pPropHdl.reset(new XMLColorTransparentPropHdl);
            break;
        case XML_TYPE_ISTRANSPARENT :
            pPropHdl.reset(new XMLIsTransparentPropHdl);
            break;
        case XML_TYPE_COLORAUTO :
            pPropHdl.reset(new XMLColorAutoPropHdl);
            break;
        case XML_TYPE_ISAUTOCOLOR :
            pPropHdl.reset(new XMLIsAutoColorPropHdl);
            break;
        case XML_TYPE_BUILDIN_CMP_ONLY :
            pPropHdl.reset(new XMLCompareOnlyPropHdl);
            break;

        case XML_TYPE_RECTANGLE_LEFT :
        case XML_TYPE_RECTANGLE_TOP :
        case XML_TYPE_RECTANGLE_WIDTH :
        case XML_TYPE_RECTANGLE_HEIGHT :
            pPropHdl.reset(new XMLRectangleMembersHdl( nType ));
            break;

        case XML_TYPE_TEXT_CROSSEDOUT_TYPE:
            pPropHdl.reset(new XMLCrossedOutTypePropHdl) ;
            break;
        case XML_TYPE_TEXT_CROSSEDOUT_STYLE:
            pPropHdl.reset(new XMLCrossedOutStylePropHdl) ;
            break;
        case XML_TYPE_TEXT_CROSSEDOUT_WIDTH:
            pPropHdl.reset(new XMLCrossedOutWidthPropHdl) ;
            break;
        case XML_TYPE_TEXT_CROSSEDOUT_TEXT:
            pPropHdl.reset(new XMLCrossedOutTextPropHdl) ;
            break;
        case XML_TYPE_TEXT_BOOLCROSSEDOUT:
            pPropHdl.reset(new XMLNamedBoolPropertyHdl(
                GetXMLToken(XML_SOLID),
                GetXMLToken(XML_NONE) ));
            break;
        case XML_TYPE_TEXT_ESCAPEMENT:
            pPropHdl.reset(new XMLEscapementPropHdl);
            break;
        case XML_TYPE_TEXT_ESCAPEMENT_HEIGHT:
            pPropHdl.reset(new XMLEscapementHeightPropHdl);
            break;
        case XML_TYPE_TEXT_CASEMAP:
            pPropHdl.reset(new XMLCaseMapPropHdl);
            break;
        case XML_TYPE_TEXT_CASEMAP_VAR:
            pPropHdl.reset(new XMLCaseMapVariantHdl);
            break;
        case XML_TYPE_TEXT_FONTFAMILYNAME:
            pPropHdl.reset(new XMLFontFamilyNamePropHdl);
            break;
        case XML_TYPE_TEXT_FONTFAMILY:
            pPropHdl.reset(new XMLFontFamilyPropHdl);
            break;
        case XML_TYPE_TEXT_FONTENCODING:
            pPropHdl.reset(new XMLFontEncodingPropHdl);
            break;
        case XML_TYPE_TEXT_FONTPITCH:
            pPropHdl.reset(new XMLFontPitchPropHdl);
            break;
        case XML_TYPE_TEXT_KERNING:
            pPropHdl.reset(new XMLKerningPropHdl);
            break;
        case XML_TYPE_TEXT_POSTURE:
            pPropHdl.reset(new XMLPosturePropHdl);
            break;
        case XML_TYPE_TEXT_SHADOWED:
            pPropHdl.reset(new XMLShadowedPropHdl);
            break;
        case XML_TYPE_TEXT_UNDERLINE_TYPE:
            pPropHdl.reset(new XMLUnderlineTypePropHdl);
            break;
        case XML_TYPE_TEXT_UNDERLINE_STYLE:
            pPropHdl.reset(new XMLUnderlineStylePropHdl);
            break;
        case XML_TYPE_TEXT_UNDERLINE_WIDTH:
            pPropHdl.reset(new XMLUnderlineWidthPropHdl);
            break;
        case XML_TYPE_TEXT_UNDERLINE_COLOR:
            pPropHdl.reset(new XMLColorTransparentPropHdl( XML_FONT_COLOR ));
            break;
        case XML_TYPE_TEXT_UNDERLINE_HASCOLOR:
            pPropHdl.reset(new XMLIsTransparentPropHdl( XML_FONT_COLOR,
                                                     false ));
            break;
        case XML_TYPE_TEXT_OVERLINE_TYPE:
            pPropHdl.reset(new XMLUnderlineTypePropHdl);
            break;
        case XML_TYPE_TEXT_OVERLINE_STYLE:
            pPropHdl.reset(new XMLUnderlineStylePropHdl);
            break;
        case XML_TYPE_TEXT_OVERLINE_WIDTH:
            pPropHdl.reset(new XMLUnderlineWidthPropHdl);
            break;
        case XML_TYPE_TEXT_OVERLINE_COLOR:
            pPropHdl.reset(new XMLColorTransparentPropHdl( XML_FONT_COLOR ));
            break;
        case XML_TYPE_TEXT_OVERLINE_HASCOLOR:
            pPropHdl.reset(new XMLIsTransparentPropHdl( XML_FONT_COLOR,
                                                     false ));
            break;
        case XML_TYPE_TEXT_WEIGHT:
            pPropHdl.reset(new XMLFontWeightPropHdl);
            break;
        case XML_TYPE_TEXT_SPLIT:
            pPropHdl.reset(new XMLNamedBoolPropertyHdl(
                GetXMLToken(XML_AUTO),
                GetXMLToken(XML_ALWAYS) ));
            break;
        case XML_TYPE_TEXT_BREAKBEFORE:
            pPropHdl.reset(new XMLFmtBreakBeforePropHdl);
            break;
        case XML_TYPE_TEXT_BREAKAFTER:
            pPropHdl.reset(new XMLFmtBreakAfterPropHdl);
            break;
        case XML_TYPE_TEXT_SHADOW:
            pPropHdl.reset(new XMLShadowPropHdl);
            break;
        case XML_TYPE_TEXT_ADJUST:
            pPropHdl.reset(new XMLParaAdjustPropHdl);
            break;
        case XML_TYPE_TEXT_ADJUSTLAST:
            pPropHdl.reset(new XMLLastLineAdjustPropHdl);
            break;
        case XML_TYPE_CHAR_HEIGHT:
            pPropHdl.reset(new XMLCharHeightHdl);
            break;
        case XML_TYPE_CHAR_HEIGHT_PROP:
            pPropHdl.reset(new XMLCharHeightPropHdl);
            break;
        case XML_TYPE_CHAR_HEIGHT_DIFF:
            pPropHdl.reset(new XMLCharHeightDiffHdl);
            break;
        case XML_TYPE_CHAR_RFC_LANGUAGE_TAG:
            pPropHdl.reset(new XMLCharRfcLanguageTagHdl);
            break;
        case XML_TYPE_CHAR_LANGUAGE:
            pPropHdl.reset(new XMLCharLanguageHdl);
            break;
        case XML_TYPE_CHAR_SCRIPT:
            pPropHdl.reset(new XMLCharScriptHdl);
            break;
        case XML_TYPE_CHAR_COUNTRY:
            pPropHdl.reset(new XMLCharCountryHdl);
            break;
        case XML_TYPE_LINE_SPACE_FIXED:
            pPropHdl.reset(new XMLLineHeightHdl);
            break;
        case XML_TYPE_LINE_SPACE_MINIMUM:
            pPropHdl.reset(new XMLLineHeightAtLeastHdl);
            break;
        case XML_TYPE_LINE_SPACE_DISTANCE:
            pPropHdl.reset(new XMLLineSpacingHdl);
            break;
        case XML_TYPE_BORDER_WIDTH:
            pPropHdl.reset(new XMLBorderWidthHdl);
            break;
        case XML_TYPE_BORDER:
            pPropHdl.reset(new XMLBorderHdl);
            break;
        case XML_TYPE_TEXT_TABSTOP:
            pPropHdl.reset(new XMLTabStopPropHdl);
            break;
        case XML_TYPE_ATTRIBUTE_CONTAINER:
            pPropHdl.reset(new XMLAttributeContainerHandler);
            break;
        case XML_TYPE_COLOR_MODE:
            pPropHdl.reset(new XMLEnumPropertyHdl(aXML_ColorMode_EnumMap));
            break;
        case XML_TYPE_DURATION16_MS:
            pPropHdl.reset(new XMLDurationMS16PropHdl_Impl);
            break;
        case XML_TYPE_TEXT_HORIZONTAL_ADJUST:
            pPropHdl.reset(new XMLEnumPropertyHdl(aXML_HorizontalAdjust_Enum));
            break;
        case XML_TYPE_TEXT_DRAW_ASPECT:
            pPropHdl.reset(new DrawAspectHdl);
            break;
        case XML_TYPE_TEXT_WRITING_MODE:
            pPropHdl.reset(new XMLConstantsPropertyHandler(
                &(aXML_WritingDirection_Enum[1]),
                XML_LR_TB));
            break;
        case XML_TYPE_TEXT_WRITING_MODE_WITH_DEFAULT:
            pPropHdl.reset(new XMLConstantsPropertyHandler(
                aXML_WritingDirection_Enum,
                XML_PAGE));
            break;
        case XML_TYPE_TEXT_HIDDEN_AS_DISPLAY:
            pPropHdl.reset(new XMLNamedBoolPropertyHdl(
                GetXMLToken(XML_NONE),
                GetXMLToken(XML_TRUE) ));
            break;
        case XML_TYPE_STYLENAME :
            pPropHdl.reset(new XMLStyleNamePropHdl);
            break;
        case XML_TYPE_NUMBER_NO_ZERO:
            pPropHdl.reset(new XMLNumberWithoutZeroPropHdl( 4 ));
            break;
        case XML_TYPE_NUMBER8_NO_ZERO:
            pPropHdl.reset(new XMLNumberWithoutZeroPropHdl( 1 ));
            break;
        case XML_TYPE_NUMBER16_NO_ZERO:
            pPropHdl.reset(new XMLNumberWithoutZeroPropHdl( 2 ));
            break;
        case XML_TYPE_NUMBER16_AUTO:
            pPropHdl.reset(new XMLNumberWithAutoInsteadZeroPropHdl);
            break;
        case XML_TYPE_TEXT_VERTICAL_POS:
            pPropHdl.reset(new XMLConstantsPropertyHandler( pXML_VertPos_Enum, XML_TOKEN_INVALID ));
        break;

    }

    return pPropHdl;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
