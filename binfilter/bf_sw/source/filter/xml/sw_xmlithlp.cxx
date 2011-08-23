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

#ifndef _SW_XMLITHLP_HXX
#include "xmlithlp.hxx"
#endif

#ifndef _HINTIDS_HXX
#include "hintids.hxx"
#endif

#ifndef SW_UNOMID_HXX
#include "unomid.h"
#endif

#ifndef _SVX_UNOMID_HXX
#include <bf_svx/unomid.hxx>
#endif

#ifndef _SVX_LRSPITEM_HXX
#include <bf_svx/lrspitem.hxx>
#endif

#ifndef _SVX_ULSPITEM_HXX
#include <bf_svx/ulspitem.hxx>
#endif

#ifndef _SVX_SHADITEM_HXX
#include <bf_svx/shaditem.hxx>
#endif

#ifndef _SVX_BOXITEM_HXX
#include <bf_svx/boxitem.hxx>
#endif

#ifndef _SVX_BRKITEM_HXX
#include <bf_svx/brkitem.hxx>
#endif

#ifndef _SVX_KEEPITEM_HXX
#include <bf_svx/keepitem.hxx>
#endif

#ifndef _SVX_BRSHITEM_HXX
#include <bf_svx/brshitem.hxx>
#endif

#ifndef _FMTPDSC_HXX
#include "fmtpdsc.hxx"
#endif

#ifndef _HORIORNT_HXX
#include <horiornt.hxx>
#endif

#ifndef _FMTORNT_HXX
#include "fmtornt.hxx"
#endif

#ifndef _FMTFSIZE_HXX
#include "fmtfsize.hxx"
#endif

#ifndef _FMTLSPLT_HXX
#include "fmtlsplt.hxx"
#endif

#ifndef _XMLOFF_XMLUCONV_HXX
#include <bf_xmloff/xmluconv.hxx>
#endif

#ifndef _XMLOFF_XMLTOKEN_HXX
#include <bf_xmloff/xmltoken.hxx>
#endif
namespace binfilter {


using namespace ::rtl;
using namespace ::binfilter::xmloff::token;


#define SVX_XML_BORDER_STYLE_NONE 0
#define SVX_XML_BORDER_STYLE_SOLID 1
#define SVX_XML_BORDER_STYLE_DOUBLE 2

#define SVX_XML_BORDER_WIDTH_THIN 0
#define SVX_XML_BORDER_WIDTH_MIDDLE 1
#define SVX_XML_BORDER_WIDTH_THICK 2


const struct SvXMLEnumMapEntry psXML_BorderStyles[] =
{
    { XML_NONE,       SVX_XML_BORDER_STYLE_NONE },
    { XML_HIDDEN,     SVX_XML_BORDER_STYLE_NONE },
    { XML_SOLID,      SVX_XML_BORDER_STYLE_SOLID },
    { XML_DOUBLE,     SVX_XML_BORDER_STYLE_DOUBLE },
    { XML_DOTTED,     SVX_XML_BORDER_STYLE_SOLID },
    { XML_DASHED,     SVX_XML_BORDER_STYLE_SOLID },
    { XML_GROOVE,     SVX_XML_BORDER_STYLE_SOLID },
    { XML_RIDGE,      SVX_XML_BORDER_STYLE_SOLID },
    { XML_INSET,      SVX_XML_BORDER_STYLE_SOLID },
    { XML_OUTSET,     SVX_XML_BORDER_STYLE_SOLID },
    { XML_TOKEN_INVALID, 0 }
};

const struct SvXMLEnumMapEntry psXML_NamedBorderWidths[] =
{
        { XML_THIN,         SVX_XML_BORDER_WIDTH_THIN   },
        { XML_MIDDLE,   SVX_XML_BORDER_WIDTH_MIDDLE     },
        { XML_THICK,    SVX_XML_BORDER_WIDTH_THICK      },
        { XML_TOKEN_INVALID, 0 }
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


const sal_uInt16 aSBorderWidths[] =
{
        SBORDER_ENTRY( 0 ), SBORDER_ENTRY( 1 ), SBORDER_ENTRY( 2 ),
        SBORDER_ENTRY( 3 ), SBORDER_ENTRY( 4 )
};

const sal_uInt16 aDBorderWidths[5*11] =
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

sal_Bool lcl_frmitems_parseXMLBorder( const OUString& rValue,
                                      const SvXMLUnitConverter& rUnitConverter,
                                      sal_Bool& rHasStyle, sal_uInt16& rStyle,
                                      sal_Bool& rHasWidth, sal_uInt16& rWidth,
                                      sal_uInt16& rNamedWidth,
                                      sal_Bool& rHasColor, Color& rColor )
{
    OUString aToken;
    SvXMLTokenEnumerator aTokens( rValue );

    rHasStyle = sal_False;
    rHasWidth = sal_False;
    rHasColor = sal_False;

    rStyle = USHRT_MAX;
    rWidth = 0;
    rNamedWidth = USHRT_MAX;

    sal_Int32 nTemp;
    while( aTokens.getNextToken( aToken ) && aToken.getLength() != 0 )
    {
        if( !rHasWidth &&
            rUnitConverter.convertEnum( rNamedWidth, aToken,
                                        psXML_NamedBorderWidths ) )
        {
            rHasWidth = sal_True;
        }
        else if( !rHasStyle &&
                 rUnitConverter.convertEnum( rStyle, aToken,
                                             psXML_BorderStyles ) )
        {
            rHasStyle = sal_True;
        }
        else if( !rHasColor && rUnitConverter.convertColor( rColor, aToken ) )
        {
            rHasColor = sal_True;
        }
        else if( !rHasWidth &&
                 rUnitConverter.convertMeasure( nTemp, aToken, 0, USHRT_MAX ) )
        {
            rWidth = (sal_uInt16)nTemp;
            rHasWidth = sal_True;
        }
        else
        {
            // missformed
            return sal_False;
        }
    }

    return rHasStyle || rHasWidth || rHasColor;
}

void lcl_frmitems_setXMLBorderWidth( SvxBorderLine& rLine,
                                     sal_uInt16 nOutWidth, sal_uInt16 nInWidth,
                                     sal_uInt16 nDistance )
{
    rLine.SetOutWidth( nOutWidth );
    rLine.SetInWidth( nInWidth );
    rLine.SetDistance( nDistance );
}

void lcl_frmitems_setXMLBorderWidth( SvxBorderLine& rLine,
                                     sal_uInt16 nWidth, sal_Bool bDouble )
{
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
        DBG_ASSERT( aWidths[i] >= aWidths[i-4], "line widths are unordered!" );
        i -= 4;
    }

    rLine.SetOutWidth( aWidths[i+1] );
    rLine.SetInWidth( aWidths[i+2] );
    rLine.SetDistance( aWidths[i+3] );
}

sal_Bool lcl_frmitems_setXMLBorder( SvxBorderLine*& rpLine,
                                    sal_Bool bHasStyle, sal_uInt16 nStyle,
                                    sal_Bool bHasWidth, sal_uInt16 nWidth,
                                    sal_uInt16 nNamedWidth,
                                    sal_Bool bHasColor, const Color& rColor )
{
    // first of all, delete an empty line
    if( (bHasStyle && SVX_XML_BORDER_STYLE_NONE == nStyle) ||
        (bHasWidth && USHRT_MAX == nNamedWidth && 0 == nWidth) )
    {
        sal_Bool bRet = 0 != rpLine;
        if( rpLine )
        {
            delete rpLine;
            rpLine = 0;
        }

        return bRet;
    }

    // if there is no line and no style and no with, there will never be a line
    if( !rpLine && !(bHasStyle && bHasWidth) )
        return sal_False;

    // We now do know that there will be a line
    if( !rpLine )
        rpLine = new SvxBorderLine;


    if( ( bHasWidth &&
          (USHRT_MAX != nNamedWidth || (nWidth != rpLine->GetOutWidth() +
                                        rpLine->GetInWidth() +
                                        rpLine->GetDistance()) ) ) ||
        ( bHasStyle &&
          ((SVX_XML_BORDER_STYLE_SOLID == nStyle && rpLine->GetDistance()) || 
            (SVX_XML_BORDER_STYLE_DOUBLE == nStyle && !rpLine->GetDistance())) ))
   {
       sal_Bool bDouble = (bHasWidth && SVX_XML_BORDER_STYLE_DOUBLE == nStyle ) || 
           rpLine->GetDistance();

       // The width has to be changed
       if( bHasWidth && USHRT_MAX != nNamedWidth )
       {
           const sal_uInt16 *aWidths = bDouble ? aDBorderWidths :aSBorderWidths;

           sal_uInt16 nNWidth = nNamedWidth * 4;
           rpLine->SetOutWidth( aWidths[nNWidth+1] );
           rpLine->SetInWidth( aWidths[nNWidth+2] );
           rpLine->SetDistance( aWidths[nNWidth+3] );
           
       }
       else
       {
           if( !bHasWidth )
               nWidth = rpLine->GetInWidth() + rpLine->GetDistance() +
                   rpLine->GetOutWidth();

           lcl_frmitems_setXMLBorderWidth( *rpLine, nWidth, bDouble );
       }
   }

    // set color
    if( bHasColor )
        rpLine->SetColor( rColor );

    return sal_True;
}

void lcl_frmitems_setXMLBorder( SvxBorderLine*& rpLine,
  sal_uInt16 nWidth, sal_uInt16 nOutWidth,
  sal_uInt16 nInWidth, sal_uInt16 nDistance )
{
    if( !rpLine )
        rpLine = new SvxBorderLine;

    if( nWidth > 0 )
        lcl_frmitems_setXMLBorderWidth( *rpLine, nWidth, sal_True );
    else
        lcl_frmitems_setXMLBorderWidth( *rpLine, nOutWidth, nInWidth,
                                        nDistance );
}

const struct SvXMLEnumMapEntry psXML_BrushRepeat[] =
{
    { XML_BACKGROUND_REPEAT,		GPOS_TILED	},
    { XML_BACKGROUND_NO_REPEAT,     GPOS_MM		},
    { XML_BACKGROUND_STRETCH,		GPOS_AREA	},
    { XML_TOKEN_INVALID, 0 }
};

const struct SvXMLEnumMapEntry psXML_BrushHoriPos[] =
{
    { XML_LEFT,		    GPOS_LM	},
    { XML_RIGHT,		GPOS_RM	},
    { XML_TOKEN_INVALID, 0 }
};

const struct SvXMLEnumMapEntry psXML_BrushVertPos[] =
{
    { XML_TOP,			GPOS_MT	},
    { XML_BOTTOM,		GPOS_MB	},
    { XML_TOKEN_INVALID, 0 }
};

void lcl_frmitems_MergeXMLHoriPos( SvxGraphicPosition& ePos,
                                   SvxGraphicPosition eHori )
{
    DBG_ASSERT( GPOS_LM==eHori || GPOS_MM==eHori || GPOS_RM==eHori,
                "lcl_frmitems_MergeXMLHoriPos: vertical pos must be middle" );

    switch( ePos )
    {
    case GPOS_LT:
    case GPOS_MT:
    case GPOS_RT:
        ePos = GPOS_LM==eHori ? GPOS_LT : (GPOS_MM==eHori ? GPOS_MT : GPOS_RT);
        break;

    case GPOS_LM:
    case GPOS_MM:
    case GPOS_RM:
        ePos = eHori;
        break;

    case GPOS_LB:
    case GPOS_MB:
    case GPOS_RB:
        ePos = GPOS_LM==eHori ? GPOS_LB : (GPOS_MM==eHori ? GPOS_MB : GPOS_RB);
        break;
    }
}

void lcl_frmitems_MergeXMLVertPos( SvxGraphicPosition& ePos,
                                     SvxGraphicPosition eVert )
{
    DBG_ASSERT( GPOS_MT==eVert || GPOS_MM==eVert || GPOS_MB==eVert,
                "lcl_frmitems_MergeXMLVertPos: horizontal pos must be middle" );

    switch( ePos )
    {
    case GPOS_LT:
    case GPOS_LM:
    case GPOS_LB:
        ePos = GPOS_MT==eVert ? GPOS_LT : (GPOS_MM==eVert ? GPOS_LM : GPOS_LB);
        ePos = eVert;
        break;

    case GPOS_MT:
    case GPOS_MM:
    case GPOS_MB:
        ePos = eVert;
        break;

    case GPOS_RT:
    case GPOS_RM:
    case GPOS_RB:
        ePos = GPOS_MT==eVert ? GPOS_RT : (GPOS_MM==eVert ? GPOS_RM : GPOS_RB);
        break;
    }
}


const struct SvXMLEnumMapEntry psXML_BreakType[] =
{
        { XML_AUTO,         0 },
        { XML_COLUMN,       1 },
        { XML_PAGE,         2 },
        { XML_EVEN_PAGE,    2 },
        { XML_ODD_PAGE,     2 },
        { XML_TOKEN_INVALID, 0}
};

const struct  SvXMLEnumMapEntry aXMLTableAlignMap[] =
{
    { XML_LEFT,				HORI_LEFT			},
    { XML_LEFT,				HORI_LEFT_AND_WIDTH	},
    { XML_CENTER, 			HORI_CENTER			},
    { XML_RIGHT, 			HORI_RIGHT			},
    { XML_MARGINS, 			HORI_FULL			},
    { XML_MARGINS,			HORI_NONE			},
    { XML_TOKEN_INVALID, 0 }
};

const struct  SvXMLEnumMapEntry aXMLTableVAlignMap[] =
{
    { XML_TOP,					VERT_TOP		},
    { XML_MIDDLE,				VERT_CENTER		},
    { XML_BOTTOM, 				VERT_BOTTOM		},
    { XML_TOKEN_INVALID, 0 }
};
}
