/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xmlithlp.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 12:41:26 $
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
#include "precompiled_sw.hxx"
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
#include <svx/unomid.hxx>
#endif

#ifndef _SVX_LRSPITEM_HXX
#include <svx/lrspitem.hxx>
#endif

#ifndef _SVX_ULSPITEM_HXX
#include <svx/ulspitem.hxx>
#endif

#ifndef _SVX_SHADITEM_HXX
#include <svx/shaditem.hxx>
#endif

#ifndef _SVX_BOXITEM_HXX
#include <svx/boxitem.hxx>
#endif

#ifndef _SVX_BRKITEM_HXX
#include <svx/brkitem.hxx>
#endif

#ifndef _SVX_KEEPITEM_HXX
#include <svx/keepitem.hxx>
#endif

#ifndef _SVX_BRSHITEM_HXX
#include <svx/brshitem.hxx>
#endif

#ifndef _FMTPDSC_HXX
#include "fmtpdsc.hxx"
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
#include <xmloff/xmluconv.hxx>
#endif

using ::rtl::OUString;
using namespace ::xmloff::token;
using namespace ::com::sun::star;


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
        SBORDER_ENTRY( 0 ), SBORDER_ENTRY( 5 ), SBORDER_ENTRY( 1 ),
        SBORDER_ENTRY( 2 ), SBORDER_ENTRY( 3 ), SBORDER_ENTRY( 4 )
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
    { XML_BACKGROUND_REPEAT,        GPOS_TILED  },
    { XML_BACKGROUND_NO_REPEAT,     GPOS_MM     },
    { XML_BACKGROUND_STRETCH,       GPOS_AREA   },
    { XML_TOKEN_INVALID, 0 }
};

const struct SvXMLEnumMapEntry psXML_BrushHoriPos[] =
{
    { XML_LEFT,         GPOS_LM },
    { XML_RIGHT,        GPOS_RM },
    { XML_TOKEN_INVALID, 0 }
};

const struct SvXMLEnumMapEntry psXML_BrushVertPos[] =
{
    { XML_TOP,          GPOS_MT },
    { XML_BOTTOM,       GPOS_MB },
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
    default:
        ;
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
    default:
        ;
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
    { XML_LEFT,             text::HoriOrientation::LEFT           },
    { XML_LEFT,             text::HoriOrientation::LEFT_AND_WIDTH },
    { XML_CENTER,           text::HoriOrientation::CENTER         },
    { XML_RIGHT,            text::HoriOrientation::RIGHT          },
    { XML_MARGINS,          text::HoriOrientation::FULL           },
    { XML_MARGINS,          text::HoriOrientation::NONE           },
    { XML_TOKEN_INVALID, 0 }
};

const struct  SvXMLEnumMapEntry aXMLTableVAlignMap[] =
{
    { XML_TOP,                  text::VertOrientation::TOP        },
    { XML_MIDDLE,               text::VertOrientation::CENTER     },
    { XML_BOTTOM,               text::VertOrientation::BOTTOM     },
    { XML_TOKEN_INVALID, 0 }
};

const struct SvXMLEnumMapEntry aXML_KeepTogetherType[] =
{
        { XML_ALWAYS,       0 },
        { XML_AUTO,         1 },
        { XML_TOKEN_INVALID, 0}
};


