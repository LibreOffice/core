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

#include "bordrhdl.hxx"
#include <sax/tools/converter.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmloff/xmlement.hxx>
#include <rtl/ustrbuf.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/table/BorderLine2.hpp>
#include <com/sun/star/table/BorderLineStyle.hpp>

#include <limits.h>

using namespace ::com::sun::star;
using namespace ::xmloff::token;

#define DEF_LINE_WIDTH_0        1
#define DEF_LINE_WIDTH_1        35
#define DEF_LINE_WIDTH_2        88

#define SVX_XML_BORDER_WIDTH_THIN 0
#define SVX_XML_BORDER_WIDTH_MIDDLE 1
#define SVX_XML_BORDER_WIDTH_THICK 2

SvXMLEnumMapEntry<sal_uInt16> const pXML_BorderStyles[] =
{
    { XML_NONE,          table::BorderLineStyle::NONE   },
    { XML_HIDDEN,        table::BorderLineStyle::NONE   },
    { XML_SOLID,         table::BorderLineStyle::SOLID  },
    { XML_DOUBLE,        table::BorderLineStyle::DOUBLE },
    { XML_DOUBLE_THIN,   table::BorderLineStyle::DOUBLE_THIN },
    { XML_DOTTED,        table::BorderLineStyle::DOTTED },
    { XML_DASHED,        table::BorderLineStyle::DASHED },
    { XML_GROOVE,        table::BorderLineStyle::ENGRAVED },
    { XML_RIDGE,         table::BorderLineStyle::EMBOSSED },
    { XML_INSET,         table::BorderLineStyle::INSET  },
    { XML_OUTSET,        table::BorderLineStyle::OUTSET },
    { XML_FINE_DASHED,   table::BorderLineStyle::FINE_DASHED },
    { XML_DASH_DOT,      table::BorderLineStyle::DASH_DOT },
    { XML_DASH_DOT_DOT,  table::BorderLineStyle::DASH_DOT_DOT },
    { XML_TOKEN_INVALID, 0 }
};

SvXMLEnumMapEntry<sal_uInt16> const pXML_NamedBorderWidths[] =
{
    { XML_THIN,             SVX_XML_BORDER_WIDTH_THIN   },
    { XML_MIDDLE,           SVX_XML_BORDER_WIDTH_MIDDLE },
    { XML_THICK,            SVX_XML_BORDER_WIDTH_THICK  },
    { XML_TOKEN_INVALID,    0 }
};
// mapping tables to map external xml input to internal box line widths

static sal_uInt16 const aBorderWidths[] =
{
    DEF_LINE_WIDTH_0,
    DEF_LINE_WIDTH_1,
    DEF_LINE_WIDTH_2
};

static void lcl_frmitems_setXMLBorderStyle( table::BorderLine2 & rBorderLine, sal_uInt16 nStyle )
{
    sal_Int16 eStyle = -1; // None
    if (nStyle != table::BorderLineStyle::NONE)
        eStyle = sal_Int16( nStyle );

    rBorderLine.LineStyle = eStyle;
}


// class XMLEscapementPropHdl


XMLBorderWidthHdl::~XMLBorderWidthHdl()
{
    // nothing to do
}

bool XMLBorderWidthHdl::importXML( const OUString& rStrImpValue, uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const
{
    SvXMLTokenEnumerator aTokenEnum( rStrImpValue );

    sal_Int32 nInWidth, nDistance, nOutWidth;

    OUString aToken;
    if( !aTokenEnum.getNextToken( aToken ) )
        return false;

    if (!rUnitConverter.convertMeasureToCore( nInWidth, aToken, 0, 500 ))
        return false;

    if( !aTokenEnum.getNextToken( aToken ) )
        return false;

    if (!rUnitConverter.convertMeasureToCore( nDistance, aToken, 0, 500 ))
        return false;

    if( !aTokenEnum.getNextToken( aToken ) )
        return false;

    if (!rUnitConverter.convertMeasureToCore( nOutWidth, aToken, 0, 500 ))
        return false;

    table::BorderLine2 aBorderLine;
    if(!(rValue >>= aBorderLine))
        aBorderLine.Color = 0;

    aBorderLine.InnerLineWidth = sal::static_int_cast< sal_Int16 >(nInWidth);
    aBorderLine.OuterLineWidth = sal::static_int_cast< sal_Int16 >(nOutWidth);
    aBorderLine.LineDistance   = sal::static_int_cast< sal_Int16 >(nDistance);

    rValue <<= aBorderLine;
    return true;
}

bool XMLBorderWidthHdl::exportXML( OUString& rStrExpValue, const uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const
{
    OUStringBuffer aOut;

    table::BorderLine2 aBorderLine;
    if(!(rValue >>= aBorderLine))
        return false;

    bool bDouble = false;
    switch ( aBorderLine.LineStyle )
    {
        case table::BorderLineStyle::DOUBLE:
        case table::BorderLineStyle::DOUBLE_THIN:
        case table::BorderLineStyle::THINTHICK_SMALLGAP:
        case table::BorderLineStyle::THINTHICK_MEDIUMGAP:
        case table::BorderLineStyle::THINTHICK_LARGEGAP:
        case table::BorderLineStyle::THICKTHIN_SMALLGAP:
        case table::BorderLineStyle::THICKTHIN_MEDIUMGAP:
        case table::BorderLineStyle::THICKTHIN_LARGEGAP:
            bDouble = true;
            break;
        default:
            break;
    }

    if( ( aBorderLine.LineDistance == 0 && aBorderLine.InnerLineWidth == 0 ) || !bDouble )
        return false;

    rUnitConverter.convertMeasureToXML( aOut, aBorderLine.InnerLineWidth );
    aOut.append( ' ' );
    rUnitConverter.convertMeasureToXML( aOut, aBorderLine.LineDistance );
    aOut.append( ' ' );
    rUnitConverter.convertMeasureToXML( aOut, aBorderLine.OuterLineWidth );

    rStrExpValue = aOut.makeStringAndClear();
    return true;
}


// class XMLEscapementHeightPropHdl


XMLBorderHdl::~XMLBorderHdl()
{
    // nothing to do
}

bool XMLBorderHdl::importXML( const OUString& rStrImpValue, uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const
{
    OUString aToken;
    SvXMLTokenEnumerator aTokens( rStrImpValue );

    bool bHasStyle = false;
    bool bHasWidth = false;
    bool bHasColor = false;

    sal_uInt16 nStyle = USHRT_MAX;
    sal_uInt16 nWidth = 0;
    sal_uInt16 nNamedWidth = USHRT_MAX;
    sal_Int32 nColor = 0;

    sal_Int32 nTemp;
    while( aTokens.getNextToken( aToken ) && !aToken.isEmpty() )
    {
        if( !bHasWidth &&
            SvXMLUnitConverter::convertEnum( nNamedWidth, aToken,
                                        pXML_NamedBorderWidths ) )
        {
            bHasWidth = true;
        }
        else if( !bHasStyle &&
                 SvXMLUnitConverter::convertEnum( nStyle, aToken,
                                              pXML_BorderStyles ) )
        {
            bHasStyle = true;
        }
        else if (!bHasColor && ::sax::Converter::convertColor(nColor, aToken))
        {
            bHasColor = true;
        }
        else if( !bHasWidth &&
                 rUnitConverter.convertMeasureToCore( nTemp, aToken, 0,
                                                 USHRT_MAX ) )
        {
            nWidth = static_cast<sal_uInt16>(nTemp);
            bHasWidth = true;
        }
        else
        {
            // misformed
            return false;
        }
    }

    // if there is no style or a different style than none but no width,
    // then the declaration is not valid.
    if (!bHasStyle || (table::BorderLineStyle::NONE != nStyle && !bHasWidth))
        return false;

    table::BorderLine2 aBorderLine;
    if(!(rValue >>= aBorderLine))
    {
        aBorderLine.Color = 0;
        aBorderLine.InnerLineWidth = 0;
        aBorderLine.OuterLineWidth = 0;
        aBorderLine.LineDistance   = 0;
        aBorderLine.LineWidth      = 0;
    }

    // first of all, delete an empty line
    if (table::BorderLineStyle::NONE == nStyle ||
        (bHasWidth && USHRT_MAX == nNamedWidth && 0 == nWidth) )
    {
        aBorderLine.InnerLineWidth = 0;
        aBorderLine.OuterLineWidth = 0;
        aBorderLine.LineDistance   = 0;
        aBorderLine.LineWidth = 0;
    }
    else
    {
        if( USHRT_MAX != nNamedWidth )
        {
            aBorderLine.LineWidth = aBorderWidths[nNamedWidth];
        }
        else
        {
            aBorderLine.LineWidth = nWidth;
            lcl_frmitems_setXMLBorderStyle( aBorderLine, nStyle );
        }
    }

    // set color
    if( bHasColor )
    {
        aBorderLine.Color = nColor;
    }

    rValue <<= aBorderLine;
    return true;
}

bool XMLBorderHdl::exportXML( OUString& rStrExpValue, const uno::Any& rValue, const SvXMLUnitConverter& /* rUnitConverter */ ) const
{
    OUStringBuffer aOut;

    table::BorderLine2 aBorderLine;
    if(!(rValue >>= aBorderLine))
        return false;

    sal_Int32 nWidth = aBorderLine.LineWidth;

    if( nWidth == 0 )
    {
        aOut.append( GetXMLToken( XML_NONE ) );
    }
    else
    {
        ::sax::Converter::convertMeasure( aOut, nWidth,
               util::MeasureUnit::MM_100TH, util::MeasureUnit::POINT);

        aOut.append( ' ' );

        XMLTokenEnum eStyleToken = XML_SOLID;
        switch ( aBorderLine.LineStyle )
        {
            case table::BorderLineStyle::DASHED:
                eStyleToken = XML_DASHED;
                break;
            case table::BorderLineStyle::DOTTED:
                eStyleToken = XML_DOTTED;
                break;
            case table::BorderLineStyle::DOUBLE:
            case table::BorderLineStyle::THINTHICK_SMALLGAP:
            case table::BorderLineStyle::THINTHICK_MEDIUMGAP:
            case table::BorderLineStyle::THINTHICK_LARGEGAP:
            case table::BorderLineStyle::THICKTHIN_SMALLGAP:
            case table::BorderLineStyle::THICKTHIN_MEDIUMGAP:
            case table::BorderLineStyle::THICKTHIN_LARGEGAP:
                eStyleToken = XML_DOUBLE;
                break;
            case table::BorderLineStyle::EMBOSSED:
                eStyleToken = XML_RIDGE;
                break;
            case table::BorderLineStyle::ENGRAVED:
                eStyleToken = XML_GROOVE;
                break;
            case table::BorderLineStyle::OUTSET:
                eStyleToken = XML_OUTSET;
                break;
            case table::BorderLineStyle::INSET:
                eStyleToken = XML_INSET;
                break;
            case table::BorderLineStyle::FINE_DASHED:
                eStyleToken = XML_FINE_DASHED;
                break;
            case table::BorderLineStyle::DASH_DOT:
                eStyleToken = XML_DASH_DOT;
                break;
            case table::BorderLineStyle::DASH_DOT_DOT:
                eStyleToken = XML_DASH_DOT_DOT;
                break;
            case table::BorderLineStyle::DOUBLE_THIN:
                eStyleToken = XML_DOUBLE_THIN;
                break;
            case table::BorderLineStyle::SOLID:
            default:
                break;
        }
        aOut.append( GetXMLToken( eStyleToken ) );

        aOut.append( ' ' );

        ::sax::Converter::convertColor( aOut, aBorderLine.Color );
    }

    rStrExpValue = aOut.makeStringAndClear();

    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
