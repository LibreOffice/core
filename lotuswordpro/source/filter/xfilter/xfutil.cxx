/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
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
 *  The Initial Developer of the Original Code is: IBM Corporation
 *
 *  Copyright: 2008 by IBM Corporation
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
/*************************************************************************
 * @file
 * Util functions for xml filter.
 ************************************************************************/
#include <xfilter/xfutil.hxx>
#include <xfilter/xfparagraph.hxx>
#include <xfilter/xfcontentcontainer.hxx>
#include <rtl/ustrbuf.hxx>
#include <sstream>

OUString   GetTableColName(sal_Int32 col)
{
    int     remain = 0;
    char    ch;
    std::string strOut;

    if( col <= 26 )
    {
        ch = 'A' + col -1;
        strOut += ch;
        return OUString::createFromAscii(strOut.c_str());
    }

    while( col>26 )
    {
        remain = col%26;
        col = col/26;
        ch = 'A' + remain -1;
        strOut += ch;
    }

    ch = 'A' + remain -1;
    strOut += ch;
    return OUString::createFromAscii(strOut.c_str());
}

//tool functions:
OUString   GetUnderlineName(enumXFUnderline type)
{
    switch(type)
    {
    case enumXFUnderlineNone:
        return "none";
        break;
    case enumXFUnderlineSingle:
        return "single";
        break;
    case enumXFUnderlineDouble:
        return "double";
        break;
    case enumXFUnderlineDotted:
        return "dotted";
        break;
    case enumXFUnderlineDash:
        return "dash";
        break;
    case enumXFUnderlineLongDash:
        return "long-dash";
        break;
    case enumXFUnderlineDotDash:
        return "dot-dash";
        break;
    case enumXFUnderlineDotDotDash:
        return "dot-dot-dash";
        break;
    case enumXFUnderlineWave:
        return "wave";
        break;
    case enumXFUnderlineBold:
        return "bold";
        break;
    case enumXFUnderlineBoldDotted:
        return "bold-dotted";
        break;
    case enumXFUnderlineBoldDash:
        return "bold-dash";
        break;
    case enumXFUnderlineBoldLongDash:
        return "bold-long-dash";
        break;
    case enumXFUnderlineBoldDotDash:
        return "bold-dot-dash";
        break;
    case enumXFUnderlineBoldDotDotDash:
        return "bold-dot-dot-dash";
        break;
    case enumXFUnderlineBoldWave:
        return "bold-wave";
        break;
    case enumXFUnderlineDoubleWave:
        return "double-wave";
        break;
    case enumXFUnderlineSmallWave:
        return "small-wave";
        break;
    }
    return OUString();
}

OUString   GetCrossoutName(enumXFCrossout type)
{
    switch(type)
    {
    case enumXFCrossoutSignel:
        return "single-line";
        break;
    case enumXFCrossoutDouble:
        return "double-line";
        break;
    case enumXFCrossoutThick:
        return "thick-line";
        break;
    case enumXFCrossoutSlash:
        return "slash";
        break;
    case enumXFCrossoutX:
        return "X";
        break;
    default:
        break;
    }
    return OUString();
}

OUString   GetTransformName(enumXFTransform type)
{
    switch(type) {
    case enumXFTransformUpper:
        return "uppercase";
        break;
    case enumXFTransformLower:
        return "lowercase";
        break;
    case enumXFTransformCapitalize:
        return "capitalize";
        break;
    case enumXFTransformSmallCaps:
        return "small-caps";
        break;
    default:
        break;
    }
    return OUString();
}

OUString   GetTextDirName(enumXFTextDir dir)
{
    switch(dir)
    {
    case enumXFTextDirLR:
        return "lr";
        break;
    case enumXFTextDirLR_TB:
        return "lr-tb";
        break;
    case enumXFTextDirPage:
        return "page";
        break;
    case enumXFTextDirRL:
        return "rl";
        break;
    case enumXFTextDirRL_TB:
        return "rl-tb";
        break;
    case enumXFTextDirTB:
        return "tb";
        break;
    case enumXFTextDirTB_LR:
        return "tb-lr";
        break;
    case enumXFTextDirTB_RL:
        return "tb-rl";
        break;
    default:
        break;
    }
    return OUString();
}

OUString   GetFrameXPos(enumXFFrameXPos pos)
{
    switch(pos)
    {
    case enumXFFrameXPosLeft:
        return "left";
        break;
    case enumXFFrameXPosRight:
        return "right";
        break;
    case enumXFFrameXPosCenter:
        return "center";
        break;
    case enumXFFrameXPosFromLeft:
        return "from-left";
        break;
    default:
        break;
    }
    return OUString();
}

OUString   GetFrameXRel(enumXFFrameXRel rel)
{
    switch(rel)
    {
    case enumXFFrameXRelPage:
        return "page";
        break;
    case enumXFFrameXRelPageContent:
        return "page-content";
        break;
    case enumXFFrameXRelPageEndMargin:
        return "page-end-margin";
        break;
    case enumXFFrameXRelPageStartMargin:
        return "page-start-margin";
        break;
    case enumXFFrameXRelPara:
        return "paragraph";
        break;
    case enumXFFrameXRelParaContent:
        return "paragraph-content";
        break;
    case enumXFFrameXRelParaEndMargin:
        return "paragraph-end-margin";
        break;
    case enumXFFrameXRelParaStartMargin:
        return "paragraph-start-margin";
        break;
    case enumXFFrameXRelChar:
        return "char";
        break;
    case enumXFFrameXRelFrame:
        return "frame";
        break;
    case enumXFFrameXRelFrameContent:
        return "frame-content";
        break;
    case enumXFFrameXRelFrameEndMargin:
        return "frame-end-margin";
        break;
    case enumXFFrameXRelFrameStartMargin:
        return "frame-start-margin";
        break;
    default:
        break;
    }
    return OUString();
}

OUString   GetFrameYPos(enumXFFrameYPos pos)
{
    switch(pos)
    {
    case enumXFFrameYPosTop:
        return "top";
    case enumXFFrameYPosMiddle:
        return "middle";
    case enumXFFrameYPosBottom:
        return "bottom";
    case enumXFFrameYPosFromTop:
        return "from-top";
    case enumXFFrameYPosBelow:
        return "below";
    }
    return OUString();
}

OUString   GetFrameYRel(enumXFFrameYRel rel)
{
    switch(rel)
    {
    case enumXFFrameYRelBaseLine:
        return "baseline";
    case enumXFFrameYRelChar:
        return "char";
    case enumXFFrameYRelFrame:
        return "frame";
    case enumXFFrameYRelFrameContent:
        return "frame-content";
    case enumXFFrameYRelLine:
        return "line";
    case enumXFFrameYRelPage:
        return "page";
    case enumXFFrameYRelPageContent:
        return "page-content";
    case enumXFFrameYRelPara:
        return "paragraph";
    case enumXFFrameYRelParaContent:
        return "paragraph-content";
    case enumXFFrameYRelText:
        return "text";
    }
    return OUString();
}

OUString   GetAlignName(enumXFAlignType align)
{
    if( align == enumXFAlignStart )
        return "start";
    else if( align == enumXFAlignCenter )
        return "center";
    else if( align == enumXFAlignEnd )
        return "end";
    else if( align == enumXFAlignJustify )
        return "justify";
    else if( align == enumXFAlignBottom )
        return "bottom";
    else if( align == enumXFAlignTop )
        return "top";
    else if( align == enumXFAlignMiddle )
        return "middle";
    else if( align == enumXFALignMargins )
        return "margins";

    return OUString();
}

OUString   GetPageUsageName(enumXFPageUsage usage)
{
    OUString sRet;
    switch(usage)
    {
    case enumXFPageUsageAll:
        sRet = "all";
        break;
    case enumXFPageUsageLeft:
        sRet = "left";
        break;
    case enumXFPageUsageRight:
        sRet = "right";
        break;
    case enumXFPageUsageMirror:
        sRet = "mirrored";
        break;
    default:
        sRet = "mirrored";
        break;
    }
    return sRet;
}

OUString   GetValueType(enumXFValueType type)
{
    OUString sRet;
    switch(type)
    {
    case enumXFValueTypeBoolean:
        sRet = "boolean";
        break;
    case enumXFValueTypeCurrency:
        sRet = "currency";
        break;
    case enumXFValueTypeDate:
        sRet = "date";
        break;
    case enumXFValueTypeFloat:
        sRet = "float";
        break;
    case enumXFValueTypePercentage:
        sRet = "percentage";
        break;
    case enumXFValueTypeString:
        sRet = "string";
        break;
    case enumXFValueTypeTime:
        sRet = "time";
        break;
    default:
        sRet = "mirrored";
        break;
    }
    return sRet;
}

OUString   GetColorMode(enumXFColorMode mode)
{
    switch(mode)
    {
    case enumXFColorStandard:
        return "standard";
    case enumXFColorGreyscale:
        return "greyscale";
    case enumXFColorWatermark:
        return "watermark";
    case enumXFColorMono:
        return "mono";
    }
    return OUString();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
