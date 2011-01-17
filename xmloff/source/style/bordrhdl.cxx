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
#include "precompiled_xmloff.hxx"


#include <bordrhdl.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmluconv.hxx>
#include <rtl/ustrbuf.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/table/BorderLine2.hpp>

#if DEBUG
#include <cstdio>
#endif

using ::rtl::OUString;
using ::rtl::OUStringBuffer;

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
const sal_uInt16 API_LINE_NONE = USHRT_MAX;

#define DEF_LINE_WIDTH_0        1
#define DEF_LINE_WIDTH_1        35
#define DEF_LINE_WIDTH_2        88

#define SVX_XML_BORDER_STYLE_NONE 0
#define SVX_XML_BORDER_STYLE_SOLID 1
#define SVX_XML_BORDER_STYLE_DOUBLE 2
#define SVX_XML_BORDER_STYLE_DASHED 3
#define SVX_XML_BORDER_STYLE_DOTTED 4

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

void lcl_frmitems_setXMLBorderStyle( table::BorderLine2 & rBorderLine, sal_uInt16 nStyle )
{
    sal_Int16 eStyle = -1; // None
    if ( nStyle != API_LINE_NONE )
        eStyle = sal_Int16( nStyle );

    rBorderLine.LineStyle = eStyle;
}


///////////////////////////////////////////////////////////////////////////////
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

    if( !rUnitConverter.convertMeasure( nInWidth, aToken, 0, 500 ) )
        return sal_False;

    if( !aTokenEnum.getNextToken( aToken ) )
        return sal_False;

    if( !rUnitConverter.convertMeasure( nDistance, aToken, 0, 500 ) )
        return sal_False;

    if( !aTokenEnum.getNextToken( aToken ) )
        return sal_False;

    if( !rUnitConverter.convertMeasure( nOutWidth, aToken, 0, 500 ) )
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

    rUnitConverter.convertMeasure( aOut, aBorderLine.InnerLineWidth );
    aOut.append( sal_Unicode( ' ' ) );
    rUnitConverter.convertMeasure( aOut, aBorderLine.LineDistance );
    aOut.append( sal_Unicode( ' ' ) );
    rUnitConverter.convertMeasure( aOut, aBorderLine.OuterLineWidth );

    rStrExpValue = aOut.makeStringAndClear();
    return sal_True;
}

///////////////////////////////////////////////////////////////////////////////
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
    Color aColor;

    sal_Int32 nTemp;
    while( aTokens.getNextToken( aToken ) && aToken.getLength() != 0 )
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
        else if( !bHasColor && rUnitConverter.convertColor( aColor, aToken ) )
        {
            bHasColor = sal_True;
        }
        else if( !bHasWidth &&
                 rUnitConverter.convertMeasure( nTemp, aToken, 0,
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
    if( !bHasStyle || (SVX_XML_BORDER_STYLE_NONE != nStyle && !bHasWidth) )
        return sal_False;

    table::BorderLine2 aBorderLine;
    if(!(rValue >>= aBorderLine))
    {
        aBorderLine.Color = 0;
        aBorderLine.InnerLineWidth = 0;
        aBorderLine.OuterLineWidth = 0;
        aBorderLine.LineDistance   = 0;
    }

    // first of all, delete an empty line
    if( (bHasStyle && SVX_XML_BORDER_STYLE_NONE == nStyle) ||
        (bHasWidth && USHRT_MAX == nNamedWidth && 0 == nWidth) )
    {
        aBorderLine.InnerLineWidth = 0;
        aBorderLine.OuterLineWidth = 0;
        aBorderLine.LineDistance   = 0;
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
        aBorderLine.Color = (sal_Int32)aColor.GetRGBColor();

    rValue <<= aBorderLine;
    return sal_True;
}

sal_Bool XMLBorderHdl::exportXML( OUString& rStrExpValue, const uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const
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
        rUnitConverter.convertMeasure( aOut, nWidth,
               MAP_100TH_MM, MAP_POINT );

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
            case API_LINE_SOLID:
            default:
                break;
        }
        aOut.append( GetXMLToken( eStyleToken ) );

        aOut.append( sal_Unicode( ' ' ) );

        rUnitConverter.convertColor( aOut, aBorderLine.Color );
    }

    rStrExpValue = aOut.makeStringAndClear();

    return sal_True;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
