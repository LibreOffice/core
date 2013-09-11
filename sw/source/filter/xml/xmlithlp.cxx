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

#include "xmlithlp.hxx"
#include "hintids.hxx"
#include "unomid.h"
#include <sax/tools/converter.hxx>
#include <svx/unomid.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/shaditem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/formatbreakitem.hxx>
#include <editeng/keepitem.hxx>
#include <editeng/brushitem.hxx>
#include "fmtpdsc.hxx"
#include "fmtornt.hxx"
#include "fmtfsize.hxx"

#include "fmtlsplt.hxx"
#include <xmloff/xmluconv.hxx>

using ::editeng::SvxBorderLine;
using namespace ::xmloff::token;
using namespace ::com::sun::star;


#define API_LINE_NONE 0x7FFF
#define API_LINE_SOLID 0
#define API_LINE_DOTTED 1
#define API_LINE_DASHED 2
#define API_LINE_DOUBLE 3
#define API_LINE_EMBOSSED 10
#define API_LINE_ENGRAVED 11
#define API_LINE_OUTSET 12
#define API_LINE_INSET 13

#define SVX_XML_BORDER_WIDTH_THIN 0
#define SVX_XML_BORDER_WIDTH_MIDDLE 1
#define SVX_XML_BORDER_WIDTH_THICK 2


const struct SvXMLEnumMapEntry psXML_BorderStyles[] =
{
    { XML_NONE,       API_LINE_NONE },
    { XML_HIDDEN,     API_LINE_NONE },
    { XML_SOLID,      API_LINE_SOLID },
    { XML_DOUBLE,     API_LINE_DOUBLE },
    { XML_DOTTED,     API_LINE_DOTTED },
    { XML_DASHED,     API_LINE_DASHED },
    { XML_GROOVE,     API_LINE_ENGRAVED },
    { XML_RIDGE,      API_LINE_EMBOSSED },
    { XML_INSET,      API_LINE_INSET },
    { XML_OUTSET,     API_LINE_OUTSET },
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


const sal_uInt16 aBorderWidths[] =
{
        DEF_LINE_WIDTH_0,
        DEF_LINE_WIDTH_5,
        DEF_LINE_WIDTH_1,
};

bool sw_frmitems_parseXMLBorder( const OUString& rValue,
                                      const SvXMLUnitConverter& rUnitConverter,
                                      bool& rHasStyle, sal_uInt16& rStyle,
                                      bool& rHasWidth, sal_uInt16& rWidth,
                                      sal_uInt16& rNamedWidth,
                                      bool& rHasColor, Color& rColor )
{
    OUString aToken;
    SvXMLTokenEnumerator aTokens( rValue );

    rHasStyle = false;
    rHasWidth = false;
    rHasColor = false;

    rStyle = USHRT_MAX;
    rWidth = 0;
    rNamedWidth = USHRT_MAX;

    sal_Int32 nTemp;
    while( aTokens.getNextToken( aToken ) && !aToken.isEmpty() )
    {
        if( !rHasWidth &&
            rUnitConverter.convertEnum( rNamedWidth, aToken,
                                        psXML_NamedBorderWidths ) )
        {
            rHasWidth = true;
        }
        else if( !rHasStyle &&
                 rUnitConverter.convertEnum( rStyle, aToken,
                                             psXML_BorderStyles ) )
        {
            rHasStyle = true;
        }
        else if (!rHasColor && ::sax::Converter::convertColor(nTemp, aToken))
        {
            rColor.SetColor(nTemp);
            rHasColor = true;
        }
        else if( !rHasWidth &&
             rUnitConverter.convertMeasureToCore(nTemp, aToken, 0, USHRT_MAX))
        {
            rWidth = (sal_uInt16)nTemp;
            rHasWidth = true;
        }
        else
        {
            // missformed
            return false;
        }
    }

    return rHasStyle || rHasWidth || rHasColor;
}

void sw_frmitems_setXMLBorderStyle( SvxBorderLine& rLine, sal_uInt16 nStyle )
{
    ::editeng::SvxBorderStyle eStyle = table::BorderLineStyle::NONE;
    if ( nStyle != API_LINE_NONE )
        eStyle = ::editeng::SvxBorderStyle( nStyle );
    rLine.SetBorderLineStyle(eStyle);
}

bool sw_frmitems_setXMLBorder( SvxBorderLine*& rpLine,
                                    bool bHasStyle, sal_uInt16 nStyle,
                                    bool bHasWidth, sal_uInt16 nWidth,
                                    sal_uInt16 nNamedWidth,
                                    bool bHasColor, const Color& rColor )
{
    // first of all, delete an empty line
    if( (bHasStyle && API_LINE_NONE == nStyle) ||
        (bHasWidth && USHRT_MAX == nNamedWidth && 0 == nWidth) )
    {
        bool bRet = 0 != rpLine;
        if( rpLine )
        {
            delete rpLine;
            rpLine = 0;
        }

        return bRet;
    }

    // if there is no line and no style and no with, there will never be a line
    if( !rpLine && !(bHasStyle && bHasWidth) )
        return false;

    // We now do know that there will be a line
    if( !rpLine )
        rpLine = new SvxBorderLine;


    if( ( bHasWidth &&
          (USHRT_MAX != nNamedWidth || (nWidth != rpLine->GetWidth() ) ) ) ||
        ( bHasStyle &&
          ((API_LINE_SOLID == nStyle && rpLine->GetDistance()) ||
            (API_LINE_DOUBLE == nStyle && !rpLine->GetDistance())) ) )
   {
       bool bDouble = (bHasWidth && API_LINE_DOUBLE == nStyle ) ||
           rpLine->GetDistance();

       // fdo#38542: for double borders, do not override the width
       // set via style:border-line-width{,-left,-right,-top,-bottom}
       if (!bDouble || !rpLine->GetWidth())
       {
           // The width has to be changed
           if (bHasWidth && USHRT_MAX != nNamedWidth)
           {
               if (bDouble)
               {
                   rpLine->SetBorderLineStyle( table::BorderLineStyle::DOUBLE );
               }
               rpLine->SetWidth( aBorderWidths[nNamedWidth] );
           }
           else
           {
               if (!bHasWidth)
                   nWidth = rpLine->GetScaledWidth();

               rpLine->SetWidth( nWidth );
           }
       }
       sw_frmitems_setXMLBorderStyle( *rpLine, nStyle );
   }

    // set color
    if( bHasColor )
        rpLine->SetColor( rColor );

    return true;
}

void sw_frmitems_setXMLBorder( SvxBorderLine*& rpLine,
  sal_uInt16 nWidth, sal_uInt16 nOutWidth,
  sal_uInt16 nInWidth, sal_uInt16 nDistance )
{
    if( !rpLine )
        rpLine = new SvxBorderLine;

    if( nWidth > 0 )
        rpLine->SetWidth( nWidth );
    else
        rpLine->GuessLinesWidths(table::BorderLineStyle::DOUBLE,
                nOutWidth, nInWidth, nDistance);
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

void sw_frmitems_MergeXMLHoriPos( SvxGraphicPosition& ePos,
                                   SvxGraphicPosition eHori )
{
    OSL_ENSURE( GPOS_LM==eHori || GPOS_MM==eHori || GPOS_RM==eHori,
                "sw_frmitems_MergeXMLHoriPos: vertical pos must be middle" );

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

void sw_frmitems_MergeXMLVertPos( SvxGraphicPosition& ePos,
                                     SvxGraphicPosition eVert )
{
    OSL_ENSURE( GPOS_MT==eVert || GPOS_MM==eVert || GPOS_MB==eVert,
                "sw_frmitems_MergeXMLVertPos: horizontal pos must be middle" );

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


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
