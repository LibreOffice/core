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

#include <bordrhdl.hxx>
#include <sax/tools/converter.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmluconv.hxx>
#include <rtl/ustrbuf.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/table/BorderLine2.hpp>

using namespace ::com::sun::star;
using namespace ::xmloff::token;

const sal_uInt16 API_LINE_SOLID   = 0;
const sal_uInt16 API_LINE_DOTTED  = 1;
const sal_uInt16 API_LINE_DASHED  = 2;
const sal_uInt16 API_LINE_DOUBLE  = 3;
const sal_uInt16 API_LINE_THINTHICK_SMALLGAP  = 4;
const sal_uInt16 API_LINE_THINTHICK_MEDIUMGAP  = 5;
const sal_uInt16 API_LINE_THINTHICK_LARGEGAP  = 6;
const sal_uInt16 API_LINE_THICKTHIN_SMALLGAP  = 7;
const sal_uInt16 API_LINE_THICKTHIN_MEDIUMGAP  = 8;
const sal_uInt16 API_LINE_THICKTHIN_LARGEGAP  = 9;
const sal_uInt16 API_LINE_EMBOSSED  = 10;
const sal_uInt16 API_LINE_ENGRAVED  = 11;
const sal_uInt16 API_LINE_OUTSET  = 12;
const sal_uInt16 API_LINE_INSET  = 13;
const sal_uInt16 API_LINE_FINE_DASHED  = 14;
const sal_uInt16 API_LINE_NONE = USHRT_MAX;

#define DEF_LINE_WIDTH_0        1
#define DEF_LINE_WIDTH_1        35
#define DEF_LINE_WIDTH_2        88

#define SVX_XML_BORDER_WIDTH_THIN 0
#define SVX_XML_BORDER_WIDTH_MIDDLE 1
#define SVX_XML_BORDER_WIDTH_THICK 2

SvXMLEnumMapEntry pXML_BorderStyles[] =
{
    { XML_NONE,                 API_LINE_NONE   },
    { XML_HIDDEN,               API_LINE_NONE   },
    { XML_SOLID,                API_LINE_SOLID  },
    { XML_DOUBLE,               API_LINE_DOUBLE },
    { XML_DOTTED,               API_LINE_DOTTED },
    { XML_DASHED,               API_LINE_DASHED },
    { XML_GROOVE,               API_LINE_ENGRAVED },
    { XML_RIDGE,                API_LINE_EMBOSSED },
    { XML_INSET,                API_LINE_INSET  },
    { XML_OUTSET,               API_LINE_OUTSET },
    { XML_FINE_DASHED,          API_LINE_FINE_DASHED },
    { XML_TOKEN_INVALID,        0 }
};

SvXMLEnumMapEntry pXML_NamedBorderWidths[] =
{
    { XML_THIN,             SVX_XML_BORDER_WIDTH_THIN   },
    { XML_MIDDLE,           SVX_XML_BORDER_WIDTH_MIDDLE },
    { XML_THICK,            SVX_XML_BORDER_WIDTH_THICK  },
    { XML_TOKEN_INVALID,    0 }
};
// mapping tables to map external xml input to intarnal box line widths

static sal_uInt16 const aBorderWidths[] =
{
    DEF_LINE_WIDTH_0,
    DEF_LINE_WIDTH_1,
    DEF_LINE_WIDTH_2
};

static void lcl_frmitems_setXMLBorderStyle( table::BorderLine2 & rBorderLine, sal_uInt16 nStyle )
{
    sal_Int16 eStyle = -1; // None
    if ( nStyle != API_LINE_NONE )
        eStyle = sal_Int16( nStyle );

    rBorderLine.LineStyle = eStyle;
}

//
// class XMLEscapementPropHdl
//

XMLBorderWidthHdl::~XMLBorderWidthHdl()
{
    // nothing to do
}

sal_Bool XMLBorderWidthHdl::importXML( const OUString& rStrImpValue, uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const
{
    SvXMLTokenEnumerator aTokenEnum( rStrImpValue );

    sal_Int32 nInWidth, nDistance, nOutWidth;

    OUString aToken;
    if( !aTokenEnum.getNextToken( aToken ) )
        return sal_False;

    if (!rUnitConverter.convertMeasureToCore( nInWidth, aToken, 0, 500 ))
        return sal_False;

    if( !aTokenEnum.getNextToken( aToken ) )
        return sal_False;

    if (!rUnitConverter.convertMeasureToCore( nDistance, aToken, 0, 500 ))
        return sal_False;

    if( !aTokenEnum.getNextToken( aToken ) )
        return sal_False;

    if (!rUnitConverter.convertMeasureToCore( nOutWidth, aToken, 0, 500 ))
        return sal_False;

    table::BorderLine2 aBorderLine;
    if(!(rValue >>= aBorderLine))
        aBorderLine.Color = 0;

    aBorderLine.InnerLineWidth = sal::static_int_cast< sal_Int16 >(nInWidth);
    aBorderLine.OuterLineWidth = sal::static_int_cast< sal_Int16 >(nOutWidth);
    aBorderLine.LineDistance   = sal::static_int_cast< sal_Int16 >(nDistance);

    rValue <<= aBorderLine;
    return sal_True;
}

sal_Bool XMLBorderWidthHdl::exportXML( OUString& rStrExpValue, const uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const
{
    OUStringBuffer aOut;

    table::BorderLine2 aBorderLine;
    if(!(rValue >>= aBorderLine))
        return sal_False;

    bool bDouble = false;
    switch ( aBorderLine.LineStyle )
    {
        case API_LINE_DOUBLE:
        case API_LINE_THINTHICK_SMALLGAP:
        case API_LINE_THINTHICK_MEDIUMGAP:
        case API_LINE_THINTHICK_LARGEGAP:
        case API_LINE_THICKTHIN_SMALLGAP:
        case API_LINE_THICKTHIN_MEDIUMGAP:
        case API_LINE_THICKTHIN_LARGEGAP:
            bDouble = true;
            break;
        default:
            break;
    }

    if( ( aBorderLine.LineDistance == 0 && aBorderLine.InnerLineWidth == 0 ) || !bDouble )
        return sal_False;

    rUnitConverter.convertMeasureToXML( aOut, aBorderLine.InnerLineWidth );
    aOut.append( sal_Unicode( ' ' ) );
    rUnitConverter.convertMeasureToXML( aOut, aBorderLine.LineDistance );
    aOut.append( sal_Unicode( ' ' ) );
    rUnitConverter.convertMeasureToXML( aOut, aBorderLine.OuterLineWidth );

    rStrExpValue = aOut.makeStringAndClear();
    return sal_True;
}

//
// class XMLEscapementHeightPropHdl
//

XMLBorderHdl::~XMLBorderHdl()
{
    // nothing to do
}

sal_Bool XMLBorderHdl::importXML( const OUString& rStrImpValue, uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const
{
    OUString aToken;
    SvXMLTokenEnumerator aTokens( rStrImpValue );

    sal_Bool bHasStyle = sal_False;
    sal_Bool bHasWidth = sal_False;
    sal_Bool bHasColor = sal_False;

    sal_uInt16 nStyle = USHRT_MAX;
    sal_uInt16 nWidth = 0;
    sal_uInt16 nNamedWidth = USHRT_MAX;
    sal_Int32 nColor = 0;

    sal_Int32 nTemp;
    while( aTokens.getNextToken( aToken ) && !aToken.isEmpty() )
    {
        if( !bHasWidth &&
            rUnitConverter.convertEnum( nNamedWidth, aToken,
                                        pXML_NamedBorderWidths ) )
        {
            bHasWidth = sal_True;
        }
        else if( !bHasStyle &&
                 rUnitConverter.convertEnum( nStyle, aToken,
                                              pXML_BorderStyles ) )
        {
            bHasStyle = sal_True;
        }
        else if (!bHasColor && ::sax::Converter::convertColor(nColor, aToken))
        {
            bHasColor = sal_True;
        }
        else if( !bHasWidth &&
                 rUnitConverter.convertMeasureToCore( nTemp, aToken, 0,
                                                 USHRT_MAX ) )
        {
            nWidth = (sal_uInt16)nTemp;
            bHasWidth = sal_True;
        }
        else
        {
            // missformed
            return sal_False;
        }
    }

    // if there is no style or a different style than none but no width,
       // then the declaration is not valid.
    if( !bHasStyle || (API_LINE_NONE != nStyle && !bHasWidth) )
        return sal_False;

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
    if( (bHasStyle && API_LINE_NONE == nStyle) ||
        (bHasWidth && USHRT_MAX == nNamedWidth && 0 == nWidth) )
    {
        aBorderLine.InnerLineWidth = 0;
        aBorderLine.OuterLineWidth = 0;
        aBorderLine.LineDistance   = 0;
        aBorderLine.LineWidth = 0;
    }
    else if( bHasWidth )
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
    else
    {
        aBorderLine.LineWidth = 0;
        lcl_frmitems_setXMLBorderStyle( aBorderLine, nStyle );
    }

    // set color
    if( bHasColor )
    {
        aBorderLine.Color = nColor;
    }

    rValue <<= aBorderLine;
    return sal_True;
}

sal_Bool XMLBorderHdl::exportXML( OUString& rStrExpValue, const uno::Any& rValue, const SvXMLUnitConverter& /* rUnitConverter */ ) const
{
    OUStringBuffer aOut;

    table::BorderLine2 aBorderLine;
    if(!(rValue >>= aBorderLine))
        return sal_False;

    sal_Int32 nWidth = aBorderLine.LineWidth;

    if( nWidth == 0 )
    {
        aOut.append( GetXMLToken( XML_NONE ) );
    }
    else
    {
        ::sax::Converter::convertMeasure( aOut, nWidth,
               util::MeasureUnit::MM_100TH, util::MeasureUnit::POINT);

        aOut.append( sal_Unicode( ' ' ) );

        XMLTokenEnum eStyleToken = XML_SOLID;
        switch ( aBorderLine.LineStyle )
        {
            case API_LINE_DASHED:
                eStyleToken = XML_DASHED;
                break;
            case API_LINE_DOTTED:
                eStyleToken = XML_DOTTED;
                break;
            case API_LINE_DOUBLE:
            case API_LINE_THINTHICK_SMALLGAP:
            case API_LINE_THINTHICK_MEDIUMGAP:
            case API_LINE_THINTHICK_LARGEGAP:
            case API_LINE_THICKTHIN_SMALLGAP:
            case API_LINE_THICKTHIN_MEDIUMGAP:
            case API_LINE_THICKTHIN_LARGEGAP:
                eStyleToken = XML_DOUBLE;
                break;
            case API_LINE_EMBOSSED:
                eStyleToken = XML_RIDGE;
                break;
            case API_LINE_ENGRAVED:
                eStyleToken = XML_GROOVE;
                break;
            case API_LINE_OUTSET:
                eStyleToken = XML_OUTSET;
                break;
            case API_LINE_INSET:
                eStyleToken = XML_INSET;
                break;
            case API_LINE_FINE_DASHED:
                eStyleToken = XML_FINE_DASHED;
                break;
            case API_LINE_SOLID:
            default:
                break;
        }
        aOut.append( GetXMLToken( eStyleToken ) );

        aOut.append( sal_Unicode( ' ' ) );

        ::sax::Converter::convertColor( aOut, aBorderLine.Color );
    }

    rStrExpValue = aOut.makeStringAndClear();

    return sal_True;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
