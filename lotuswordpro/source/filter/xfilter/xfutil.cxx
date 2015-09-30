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
#include "xfutil.hxx"
#include "xfparagraph.hxx"
#include "xfcontentcontainer.hxx"
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
        return OUString("none");
        break;
    case enumXFUnderlineSingle:
        return OUString("single");
        break;
    case enumXFUnderlineDouble:
        return OUString("double");
        break;
    case enumXFUnderlineDotted:
        return OUString("dotted");
        break;
    case enumXFUnderlineDash:
        return OUString("dash");
        break;
    case enumXFUnderlineLongDash:
        return OUString("long-dash");
        break;
    case enumXFUnderlineDotDash:
        return OUString("dot-dash");
        break;
    case enumXFUnderlineDotDotDash:
        return OUString("dot-dot-dash");
        break;
    case enumXFUnderlineWave:
        return OUString("wave");
        break;
    case enumXFUnderlineBold:
        return OUString("bold");
        break;
    case enumXFUnderlineBoldDotted:
        return OUString("bold-dotted");
        break;
    case enumXFUnderlineBoldDash:
        return OUString("bold-dash");
        break;
    case enumXFUnderlineBoldLongDash:
        return OUString("bold-long-dash");
        break;
    case enumXFUnderlineBoldDotDash:
        return OUString("bold-dot-dash");
        break;
    case enumXFUnderlineBoldDotDotDash:
        return OUString("bold-dot-dot-dash");
        break;
    case enumXFUnderlineBoldWave:
        return OUString("bold-wave");
        break;
    case enumXFUnderlineDoubleWave:
        return OUString("double-wave");
        break;
    case enumXFUnderlineSmallWave:
        return OUString("samll-wave");
        break;
    }
    return OUString("");
}

OUString   GetReliefName(enumXFRelief type)
{
    switch(type)
    {
        case enumXFReliefEngraved:
            return OUString("engraved");
            break;
        case enumXFReliefEmbossed:
            return OUString("embossed");
            break;
        default:
            break;
    }
    return OUString("");
}

OUString   GetCrossoutName(enumXFCrossout type)
{
    switch(type)
    {
    case enumXFCrossoutSignel:
        return OUString("single-line");
        break;
    case enumXFCrossoutDouble:
        return OUString("double-line");
        break;
    case enumXFCrossoutThick:
        return OUString("thick-line");
        break;
    case enumXFCrossoutSlash:
        return OUString("slash");
        break;
    case enumXFCrossoutX:
        return OUString("X");
        break;
    default:
        break;
    }
    return OUString("");
}

OUString   GetTransformName(enumXFTransform type)
{
    switch(type) {
    case enumXFTransformUpper:
        return OUString("uppercase");
        break;
    case enumXFTransformLower:
        return OUString("lowercase");
        break;
    case enumXFTransformCapitalize:
        return OUString("capitalize");
        break;
    case enumXFTransformSmallCaps:
        return OUString("small-caps");
        break;
    default:
        break;
    }
    return OUString("");
}

OUString   GetEmphasizeName(enumXFEmphasize type)
{
    switch(type) {
    case enumXFEmphasizeDot:
        return OUString("dot");
        break;
    case enumXFEmphasizeCircle:
        return OUString("circle");
        break;
    case enumXFEmphasizeDisc:
        return OUString("disc");
        break;
    case enumXFEmphasizeAccent:
        return OUString("accent");
        break;
    default:
        break;
    }
    return OUString("");
}

OUString   GetTextDirName(enumXFTextDir dir)
{
    switch(dir)
    {
    case enumXFTextDirLR:
        return OUString("lr");
        break;
    case enumXFTextDirLR_TB:
        return OUString("lr-tb");
        break;
    case enumXFTextDirPage:
        return OUString("page");
        break;
    case enumXFTextDirRL:
        return OUString("rl");
        break;
    case enumXFTextDirRL_TB:
        return OUString("rl-tb");
        break;
    case enumXFTextDirTB:
        return OUString("tb");
        break;
    case enumXFTextDirTB_LR:
        return OUString("tb-lr");
        break;
    case enumXFTextDirTB_RL:
        return OUString("tb-rl");
        break;
    default:
        break;
    }
    return OUString("");
}

OUString   GetFrameXPos(enumXFFrameXPos pos)
{
    switch(pos)
    {
    case enumXFFrameXPosLeft:
        return OUString("left");
        break;
    case enumXFFrameXPosRight:
        return OUString("right");
        break;
    case enumXFFrameXPosCenter:
        return OUString("center");
        break;
    case enumXFFrameXPosFromLeft:
        return OUString("from-left");
        break;
    default:
        break;
    }
    return OUString("");
}

OUString   GetFrameXRel(enumXFFrameXRel rel)
{
    switch(rel)
    {
    case enumXFFrameXRelPage:
        return OUString("page");
        break;
    case enumXFFrameXRelPageContent:
        return OUString("page-content");
        break;
    case enumXFFrameXRelPageEndMargin:
        return OUString("page-end-margin");
        break;
    case enumXFFrameXRelPageStartMargin:
        return OUString("page-start-margin");
        break;
    case enumXFFrameXRelPara:
        return OUString("paragraph");
        break;
    case enumXFFrameXRelParaContent:
        return OUString("paragraph-content");
        break;
    case enumXFFrameXRelParaEndMargin:
        return OUString("paragraph-end-margin");
        break;
    case enumXFFrameXRelParaStartMargin:
        return OUString("paragraph-start-margin");
        break;
    case enumXFFrameXRelChar:
        return OUString("char");
        break;
    case enumXFFrameXRelFrame:
        return OUString("frame");
        break;
    case enumXFFrameXRelFrameContent:
        return OUString("frame-content");
        break;
    case enumXFFrameXRelFrameEndMargin:
        return OUString("frame-end-margin");
        break;
    case enumXFFrameXRelFrameStartMargin:
        return OUString("frame-start-margin");
        break;
    default:
        break;
    }
    return OUString("");
}

OUString   GetFrameYPos(enumXFFrameYPos pos)
{
    switch(pos)
    {
    case enumXFFrameYPosTop:
        return OUString("top");
    case enumXFFrameYPosMiddle:
        return OUString("middle");
    case enumXFFrameYPosBottom:
        return OUString("bottom");
    case enumXFFrameYPosFromTop:
        return OUString("from-top");
    case enumXFFrameYPosBelow:
        return OUString("below");
    }
    return OUString("");
}

OUString   GetFrameYRel(enumXFFrameYRel rel)
{
    switch(rel)
    {
    case enumXFFrameYRelBaseLine:
        return OUString("baseline");
    case enumXFFrameYRelChar:
        return OUString("char");
    case enumXFFrameYRelFrame:
        return OUString("frame");
    case enumXFFrameYRelFrameContent:
        return OUString("frame-content");
    case enumXFFrameYRelLine:
        return OUString("line");
    case enumXFFrameYRelPage:
        return OUString("page");
    case enumXFFrameYRelPageContent:
        return OUString("page-content");
    case enumXFFrameYRelPara:
        return OUString("paragraph");
    case enumXFFrameYRelParaContent:
        return OUString("paragraph-content");
    case enumXFFrameYRelText:
        return OUString("text");
    }
    return OUString("");
}

OUString   GetAlignName(enumXFAlignType align)
{
    if( align == enumXFAlignStart )
        return OUString("start");
    else if( align == enumXFAlignCenter )
        return OUString("center");
    else if( align == enumXFAlignEnd )
        return OUString("end");
    else if( align == enumXFAlignJustify )
        return OUString("justify");
    else if( align == enumXFAlignBottom )
        return OUString("bottom");
    else if( align == enumXFAlignTop )
        return OUString("top");
    else if( align == enumXFAlignMiddle )
        return OUString("middle");
    else if( align == enumXFALignMargins )
        return OUString("margins");

    return OUString("");
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
        return OUString("standard");
    case enumXFColorGreyscale:
        return OUString("greyscale");
    case enumXFColorWatermark:
        return OUString("watermark");
    case enumXFColorMono:
        return OUString("mono");
    }
    return OUString("");
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
