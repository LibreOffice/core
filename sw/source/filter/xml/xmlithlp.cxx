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

#include <limits.h>

#include "xmlithlp.hxx"
#include <sax/tools/converter.hxx>
#include <editeng/borderline.hxx>
#include <editeng/brushitem.hxx>

#include <xmloff/xmluconv.hxx>
#include <osl/diagnose.h>

#include <com/sun/star/table/BorderLineStyle.hpp>
#include <com/sun/star/text/HoriOrientation.hpp>
#include <com/sun/star/text/VertOrientation.hpp>

using ::editeng::SvxBorderLine;
using namespace ::xmloff::token;
using namespace ::com::sun::star;

#define SVX_XML_BORDER_WIDTH_THIN 0
#define SVX_XML_BORDER_WIDTH_MIDDLE 1
#define SVX_XML_BORDER_WIDTH_THICK 2

const struct SvXMLEnumMapEntry<sal_uInt16> psXML_BorderStyles[] =
{
    { XML_NONE,         table::BorderLineStyle::NONE },
    { XML_HIDDEN,       table::BorderLineStyle::NONE },
    { XML_SOLID,        table::BorderLineStyle::SOLID },
    { XML_DOUBLE,       table::BorderLineStyle::DOUBLE },
    { XML_DOUBLE_THIN,  table::BorderLineStyle::DOUBLE_THIN },
    { XML_DOTTED,       table::BorderLineStyle::DOTTED },
    { XML_DASHED,       table::BorderLineStyle::DASHED },
    { XML_FINE_DASHED,  table::BorderLineStyle::FINE_DASHED },
    { XML_DASH_DOT,     table::BorderLineStyle::DASH_DOT },
    { XML_DASH_DOT_DOT, table::BorderLineStyle::DASH_DOT_DOT },
    { XML_GROOVE,       table::BorderLineStyle::ENGRAVED },
    { XML_RIDGE,        table::BorderLineStyle::EMBOSSED },
    { XML_INSET,        table::BorderLineStyle::INSET },
    { XML_OUTSET,       table::BorderLineStyle::OUTSET },
    { XML_TOKEN_INVALID, 0 }
};

const struct SvXMLEnumMapEntry<sal_uInt16> psXML_NamedBorderWidths[] =
{
        { XML_THIN,     SVX_XML_BORDER_WIDTH_THIN   },
        { XML_MIDDLE,   SVX_XML_BORDER_WIDTH_MIDDLE     },
        { XML_THICK,    SVX_XML_BORDER_WIDTH_THICK      },
        { XML_TOKEN_INVALID, 0 }
};
// mapping tables to map external xml input to internal box line widths

const sal_uInt16 aBorderWidths[] =
{
        DEF_LINE_WIDTH_0,
        DEF_LINE_WIDTH_5,
        DEF_LINE_WIDTH_1,
};

bool sw_frmitems_parseXMLBorder( std::u16string_view rValue,
                                      const SvXMLUnitConverter& rUnitConverter,
                                      bool& rHasStyle, sal_uInt16& rStyle,
                                      bool& rHasWidth, sal_uInt16& rWidth,
                                      sal_uInt16& rNamedWidth,
                                      bool& rHasColor, Color& rColor )
{
    std::u16string_view aToken;
    SvXMLTokenEnumerator aTokens( rValue );

    rHasStyle = false;
    rHasWidth = false;
    rHasColor = false;

    rStyle = USHRT_MAX;
    rWidth = 0;
    rNamedWidth = USHRT_MAX;

    sal_Int32 nTemp;
    while( aTokens.getNextToken( aToken ) && !aToken.empty() )
    {
        if( !rHasWidth &&
            SvXMLUnitConverter::convertEnum( rNamedWidth, aToken,
                                        psXML_NamedBorderWidths ) )
        {
            rHasWidth = true;
        }
        else if( !rHasStyle &&
                 SvXMLUnitConverter::convertEnum( rStyle, aToken,
                                             psXML_BorderStyles ) )
        {
            rHasStyle = true;
        }
        else if (!rHasColor && ::sax::Converter::convertColor(rColor, aToken))
        {
            rHasColor = true;
        }
        else if( !rHasWidth &&
             rUnitConverter.convertMeasureToCore(nTemp, aToken, 0, USHRT_MAX))
        {
            rWidth = static_cast<sal_uInt16>(nTemp);
            rHasWidth = true;
        }
        else
        {
            // misformed
            return false;
        }
    }

    return rHasStyle || rHasWidth || rHasColor;
}

static void sw_frmitems_setXMLBorderStyle( SvxBorderLine& rLine, sal_uInt16 nStyle )
{
    SvxBorderLineStyle eStyle = SvxBorderLineStyle::NONE;
    if ( nStyle != table::BorderLineStyle::NONE )
        eStyle = SvxBorderLineStyle( nStyle );
    rLine.SetBorderLineStyle(eStyle);
}

bool sw_frmitems_setXMLBorder( std::unique_ptr<SvxBorderLine>& rpLine,
                                    bool bHasStyle, sal_uInt16 nStyle,
                                    bool bHasWidth, sal_uInt16 nWidth,
                                    sal_uInt16 nNamedWidth,
                                    bool bHasColor, const Color& rColor )
{
    // first of all, delete an empty line
    if( (bHasStyle && table::BorderLineStyle::NONE == nStyle) ||
        (bHasWidth && USHRT_MAX == nNamedWidth && 0 == nWidth) )
    {
        bool bRet = nullptr != rpLine;
        rpLine.reset();
        return bRet;
    }

    // if there is no line and no style and no with, there will never be a line
    if( !rpLine && !(bHasStyle && bHasWidth) )
        return false;

    // We now do know that there will be a line
    if( !rpLine )
        rpLine.reset(new SvxBorderLine);

    if( ( bHasWidth &&
          (USHRT_MAX != nNamedWidth || (nWidth != rpLine->GetWidth() ) ) ) ||
        ( bHasStyle &&
          ((table::BorderLineStyle::SOLID == nStyle && rpLine->GetDistance()) ||
            (table::BorderLineStyle::DOUBLE == nStyle && !rpLine->GetDistance())) ) )
   {
       bool bDouble = (bHasWidth && table::BorderLineStyle::DOUBLE == nStyle ) ||
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
                   rpLine->SetBorderLineStyle( SvxBorderLineStyle::DOUBLE );
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

void sw_frmitems_setXMLBorder( std::unique_ptr<SvxBorderLine>& rpLine,
  sal_uInt16 nWidth, sal_uInt16 nOutWidth,
  sal_uInt16 nInWidth, sal_uInt16 nDistance )
{
    if( !rpLine )
        rpLine.reset(new SvxBorderLine);

    if( nWidth > 0 )
        rpLine->SetWidth( nWidth );
    else
        rpLine->GuessLinesWidths(SvxBorderLineStyle::DOUBLE,
                nOutWidth, nInWidth, nDistance);
}

const struct SvXMLEnumMapEntry<SvxGraphicPosition> psXML_BrushRepeat[] =
{
    { XML_REPEAT,        GPOS_TILED  },
    { XML_BACKGROUND_NO_REPEAT,     GPOS_MM     },
    { XML_STRETCH,       GPOS_AREA   },
    { XML_TOKEN_INVALID, SvxGraphicPosition(0) }
};

const struct SvXMLEnumMapEntry<SvxGraphicPosition> psXML_BrushHoriPos[] =
{
    { XML_LEFT,         GPOS_LM },
    { XML_RIGHT,        GPOS_RM },
    { XML_TOKEN_INVALID, SvxGraphicPosition(0) }
};

const struct SvXMLEnumMapEntry<SvxGraphicPosition> psXML_BrushVertPos[] =
{
    { XML_TOP,          GPOS_MT },
    { XML_BOTTOM,       GPOS_MB },
    { XML_TOKEN_INVALID, SvxGraphicPosition(0) }
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

const struct SvXMLEnumMapEntry<sal_uInt16> psXML_BreakType[] =
{
        { XML_AUTO,         0 },
        { XML_COLUMN,       1 },
        { XML_PAGE,         2 },
        { XML_EVEN_PAGE,    2 },
        { XML_ODD_PAGE,     2 },
        { XML_TOKEN_INVALID, 0}
};

const struct SvXMLEnumMapEntry<sal_Int16> aXMLTableAlignMap[] =
{
    { XML_LEFT,             text::HoriOrientation::LEFT           },
    { XML_LEFT,             text::HoriOrientation::LEFT_AND_WIDTH },
    { XML_CENTER,           text::HoriOrientation::CENTER         },
    { XML_RIGHT,            text::HoriOrientation::RIGHT          },
    { XML_MARGINS,          text::HoriOrientation::FULL           },
    { XML_MARGINS,          text::HoriOrientation::NONE           },
    { XML_TOKEN_INVALID, 0 }
};

const struct SvXMLEnumMapEntry<sal_Int16> aXMLTableVAlignMap[] =
{
    { XML_TOP,                  text::VertOrientation::TOP        },
    { XML_MIDDLE,               text::VertOrientation::CENTER     },
    { XML_BOTTOM,               text::VertOrientation::BOTTOM     },
    { XML_TOKEN_INVALID, 0 }
};

const struct SvXMLEnumMapEntry<sal_uInt16> aXML_KeepTogetherType[] =
{
        { XML_ALWAYS,       0 },
        { XML_AUTO,         1 },
        { XML_TOKEN_INVALID, 0}
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
