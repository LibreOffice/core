/*************************************************************************
 *
 *  $RCSfile: bordrhdl.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:07:04 $
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
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
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


#ifndef _XMLOFF_PROPERTYHANDLER_BORDERTYPES_HXX
#include <bordrhdl.hxx>
#endif

#ifndef _XMLOFF_XMLKYWD_HXX
#include "xmlkywd.hxx"
#endif

#ifndef _XMLOFF_XMLUCONV_HXX
#include "xmluconv.hxx"
#endif

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif

#ifndef _XMLOFF_XMLEMENT_HXX
#include "xmlelement.hxx"
#endif

#ifndef _COM_SUN_STAR_TABLE_BORDERLINE_HPP_
#include <com/sun/star/table/BorderLine.hpp>
#endif

using namespace ::rtl;
using namespace ::com::sun::star;

// copied from svx/boxitem.hxx
#define DEF_LINE_WIDTH_0        1
#define DEF_LINE_WIDTH_1        35
#define DEF_LINE_WIDTH_2        88
#define DEF_LINE_WIDTH_3        141
#define DEF_LINE_WIDTH_4        176

#define DEF_MAX_LINE_WIDHT      DEF_LINE_WIDTH_4
#define DEF_MAX_LINE_DIST       DEF_LINE_WIDTH_2

#define DEF_DOUBLE_LINE0_OUT    DEF_LINE_WIDTH_0
#define DEF_DOUBLE_LINE0_IN     DEF_LINE_WIDTH_0
#define DEF_DOUBLE_LINE0_DIST   DEF_LINE_WIDTH_1

#define DEF_DOUBLE_LINE1_OUT    DEF_LINE_WIDTH_1
#define DEF_DOUBLE_LINE1_IN     DEF_LINE_WIDTH_1
#define DEF_DOUBLE_LINE1_DIST   DEF_LINE_WIDTH_1

#define DEF_DOUBLE_LINE2_OUT    DEF_LINE_WIDTH_2
#define DEF_DOUBLE_LINE2_IN     DEF_LINE_WIDTH_2
#define DEF_DOUBLE_LINE2_DIST   DEF_LINE_WIDTH_2

#define DEF_DOUBLE_LINE3_OUT    DEF_LINE_WIDTH_2
#define DEF_DOUBLE_LINE3_IN     DEF_LINE_WIDTH_1
#define DEF_DOUBLE_LINE3_DIST   DEF_LINE_WIDTH_2

#define DEF_DOUBLE_LINE4_OUT    DEF_LINE_WIDTH_1
#define DEF_DOUBLE_LINE4_IN     DEF_LINE_WIDTH_2
#define DEF_DOUBLE_LINE4_DIST   DEF_LINE_WIDTH_1

#define DEF_DOUBLE_LINE5_OUT    DEF_LINE_WIDTH_3
#define DEF_DOUBLE_LINE5_IN     DEF_LINE_WIDTH_2
#define DEF_DOUBLE_LINE5_DIST   DEF_LINE_WIDTH_2

#define DEF_DOUBLE_LINE6_OUT    DEF_LINE_WIDTH_2
#define DEF_DOUBLE_LINE6_IN     DEF_LINE_WIDTH_3
#define DEF_DOUBLE_LINE6_DIST   DEF_LINE_WIDTH_2

#define DEF_DOUBLE_LINE7_OUT    DEF_LINE_WIDTH_0
#define DEF_DOUBLE_LINE7_IN     DEF_LINE_WIDTH_0
#define DEF_DOUBLE_LINE7_DIST   DEF_LINE_WIDTH_2

#define DEF_DOUBLE_LINE8_OUT    DEF_LINE_WIDTH_1
#define DEF_DOUBLE_LINE8_IN     DEF_LINE_WIDTH_0
#define DEF_DOUBLE_LINE8_DIST   DEF_LINE_WIDTH_2

#define DEF_DOUBLE_LINE9_OUT    DEF_LINE_WIDTH_2
#define DEF_DOUBLE_LINE9_IN     DEF_LINE_WIDTH_0
#define DEF_DOUBLE_LINE9_DIST   DEF_LINE_WIDTH_2

#define DEF_DOUBLE_LINE10_OUT   DEF_LINE_WIDTH_3
#define DEF_DOUBLE_LINE10_IN    DEF_LINE_WIDTH_0
#define DEF_DOUBLE_LINE10_DIST  DEF_LINE_WIDTH_2

// finished copy

#define SVX_XML_BORDER_STYLE_NONE 0
#define SVX_XML_BORDER_STYLE_SOLID 1
#define SVX_XML_BORDER_STYLE_DOUBLE 2

#define SVX_XML_BORDER_WIDTH_THIN 0
#define SVX_XML_BORDER_WIDTH_MIDDLE 1
#define SVX_XML_BORDER_WIDTH_THICK 2

SvXMLEnumMapEntry psXML_BorderStyles[] =
{
    { sXML_none,                SVX_XML_BORDER_STYLE_NONE   },
    { sXML_hidden,              SVX_XML_BORDER_STYLE_NONE   },
    { sXML_solid,               SVX_XML_BORDER_STYLE_SOLID  },
    { sXML_double,              SVX_XML_BORDER_STYLE_DOUBLE },
    { sXML_dotted,              SVX_XML_BORDER_STYLE_SOLID  },
    { sXML_dashed,              SVX_XML_BORDER_STYLE_SOLID  },
    { sXML_groove,              SVX_XML_BORDER_STYLE_SOLID  },
    { sXML_ridge,               SVX_XML_BORDER_STYLE_SOLID  },
    { sXML_inset,               SVX_XML_BORDER_STYLE_SOLID  },
    { sXML_outset,              SVX_XML_BORDER_STYLE_SOLID  },
    { 0,        0 }
};

SvXMLEnumMapEntry psXML_NamedBorderWidths[] =
{
    { sXML_thin,    SVX_XML_BORDER_WIDTH_THIN   },
    { sXML_middle,  SVX_XML_BORDER_WIDTH_MIDDLE },
    { sXML_thick,   SVX_XML_BORDER_WIDTH_THICK  },
    { 0,        0 }
};
// mapping tables to map external xml input to intarnal box line widths

// Ein Eintrag besteht aus vier USHORTs. Der erste ist die Gesamtbreite,
// die anderen sind die 3 Einzelbreiten

#define SBORDER_ENTRY( n ) \
    DEF_LINE_WIDTH_##n, DEF_LINE_WIDTH_##n, 0, 0

#define DBORDER_ENTRY( n ) \
    DEF_DOUBLE_LINE##n##_OUT + DEF_DOUBLE_LINE##n##_IN + \
    DEF_DOUBLE_LINE##n##_DIST, \
    DEF_DOUBLE_LINE##n##_OUT, \
    DEF_DOUBLE_LINE##n##_IN, \
    DEF_DOUBLE_LINE##n##_DIST

#define TDBORDER_ENTRY( n ) \
    DEF_DOUBLE_LINE##n##_OUT, \
    DEF_DOUBLE_LINE##n##_OUT, \
    DEF_DOUBLE_LINE##n##_IN, \
    DEF_DOUBLE_LINE##n##_DIST


static sal_uInt16 __READONLY_DATA aSBorderWidths[] =
{
    SBORDER_ENTRY( 0 ), SBORDER_ENTRY( 1 ), SBORDER_ENTRY( 2 ),
    SBORDER_ENTRY( 3 ), SBORDER_ENTRY( 4 )
};

static sal_uInt16 __READONLY_DATA aDBorderWidths[] =
{
    DBORDER_ENTRY( 0 ),
    DBORDER_ENTRY( 7 ),
    DBORDER_ENTRY( 1 ),
    DBORDER_ENTRY( 8 ),
    DBORDER_ENTRY( 4 ),
    DBORDER_ENTRY( 9 ),
    DBORDER_ENTRY( 3 ),
    DBORDER_ENTRY( 10 ),
    DBORDER_ENTRY( 2 ),
    DBORDER_ENTRY( 6 ),
    DBORDER_ENTRY( 5 )
};

void lcl_frmitems_setXMLBorderWidth( table::BorderLine &rBorderLine,
                                     sal_uInt16 nWidth, sal_Bool bDouble )
{
#ifdef XML_CHECK_UI_CONTSTRAINS
    const sal_uInt16 *aWidths;
    sal_uInt16 nSize;
    if( !bDouble )
    {
        aWidths = aSBorderWidths;
        nSize = sizeof( aSBorderWidths );
    }
    else
    {
        aWidths = aDBorderWidths;
        nSize = sizeof( aDBorderWidths );
    }

    sal_uInt16 i = (nSize / sizeof(sal_uInt16)) - 4;
    while( i>0 &&
           nWidth <= ((aWidths[i] + aWidths[i-4]) / 2)  )
    {
        i -= 4;
    }

    rBorderLine.OuterLineWidth = aWidths[i+1];
    rBorderLine.InnerLineWidth = aWidths[i+2];
    rBorderLine.LineDistance = aWidths[i+3];
#else
    if( bDouble )
    {
        const sal_uInt16 *aWidths = aDBorderWidths;
        sal_uInt16 nSize = sizeof( aDBorderWidths );
        sal_uInt16 i = (nSize / sizeof(sal_uInt16)) - 4;
        while( i>0 &&
               nWidth <= ((aWidths[i] + aWidths[i-4]) / 2)  )
        {
            i -= 4;
        }

        rBorderLine.OuterLineWidth = aWidths[i+1];
        rBorderLine.InnerLineWidth = aWidths[i+2];
        rBorderLine.LineDistance = aWidths[i+3];
    }
    else
    {
        rBorderLine.OuterLineWidth = 0 == nWidth ? DEF_LINE_WIDTH_0 : nWidth;
        rBorderLine.InnerLineWidth = 0;
        rBorderLine.LineDistance = 0;

    }
#endif
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

#ifdef XML_CHECK_UI_CONSTRAINS
    sal_uInt16 nSize = sizeof( aDBorderWidths );
    for( sal_uInt16 i=0; i < nSize; i += 4 )
    {
        if( aDBorderWidths[i+1] == nOutWidth &&
            aDBorderWidths[i+2] == nInWidth &&
            aDBorderWidths[i+3] == nDistance )
            break;
    }

    sal_uInt16 nWidth = i < nSize ? 0 : nOutWidth + nInWidth + nDistance;
#endif

    table::BorderLine aBorderLine;
    if(!(rValue >>= aBorderLine))
        aBorderLine.Color = 0;

    aBorderLine.InnerLineWidth = nInWidth;
    aBorderLine.OuterLineWidth = nOutWidth;
    aBorderLine.LineDistance   = nDistance;

    rValue <<= aBorderLine;
    return sal_True;
}

sal_Bool XMLBorderWidthHdl::exportXML( OUString& rStrExpValue, const uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const
{
    OUStringBuffer aOut;

    table::BorderLine aBorderLine;
    if(!(rValue >>= aBorderLine))
        return sal_False;

    if( aBorderLine.LineDistance == 0 && aBorderLine.InnerLineWidth == 0)
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

    long nTemp;
    while( aTokens.getNextToken( aToken ) && aToken.getLength() != 0 )
    {
        if( !bHasWidth &&
            rUnitConverter.convertEnum( nNamedWidth, aToken,
                                        psXML_NamedBorderWidths ) )
        {
            bHasWidth = sal_True;
        }
        else if( !bHasStyle &&
                 rUnitConverter.convertEnum( nStyle, aToken,
                                              psXML_BorderStyles ) )
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

    // if there is no line and no style and no with, there will never be a line
    if( !(bHasStyle && bHasWidth) )
        return sal_False;

    table::BorderLine aBorderLine;
    if(!(rValue >>= aBorderLine))
    {
        aBorderLine.Color = 0;
        aBorderLine.InnerLineWidth = 0;
        aBorderLine.OuterLineWidth = 0;
        aBorderLine.LineDistance   = 0;
    }

    // first of all, delete an empty line
    sal_Bool bDouble = SVX_XML_BORDER_STYLE_DOUBLE == nStyle;
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
            const sal_uInt16 *aWidths = bDouble ? aDBorderWidths
                                            : aSBorderWidths;
            sal_uInt16 nNWidth = nNamedWidth * 4;
            aBorderLine.OuterLineWidth = aWidths[nNWidth+1];
            aBorderLine.InnerLineWidth = aWidths[nNWidth+2];
            aBorderLine.LineDistance = aWidths[nNWidth+3];
        }
        else
        {
            lcl_frmitems_setXMLBorderWidth( aBorderLine, nWidth, bDouble );
        }
    }
    else
    {
        lcl_frmitems_setXMLBorderWidth( aBorderLine, 0, bDouble );
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

    table::BorderLine aBorderLine;
    if(!(rValue >>= aBorderLine))
        return sal_False;

    sal_Int32 nWidth = aBorderLine.OuterLineWidth;
    const sal_uInt16 nDistance = aBorderLine.LineDistance;
    if( 0 != nDistance )
    {
        nWidth += nDistance;
        nWidth += aBorderLine.InnerLineWidth;
    }

    if( nWidth == 0 )
        return sal_False;

    char *pStyle = (0 == nDistance) ? sXML_solid : sXML_double;

    rUnitConverter.convertMeasure( aOut, nWidth );

    aOut.append( sal_Unicode( ' ' ) );

    aOut.appendAscii( pStyle );

    aOut.append( sal_Unicode( ' ' ) );

    rUnitConverter.convertColor( aOut, aBorderLine.Color );

    rStrExpValue = aOut.makeStringAndClear();
    return sal_True;
}
