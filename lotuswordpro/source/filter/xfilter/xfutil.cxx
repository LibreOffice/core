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

OUString   GetTableColName(sal_Int32 col)
{
    int     remain = 0;
    char    ch;
    std::string strOut;

    if( col <= 26 )
    {
        ch = 'A' + col -1;
        strOut += ch;
        return OUString::createFromAscii(strOut);
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
    return OUString::createFromAscii(strOut);
}

//tool functions:
OUString   GetUnderlineName(enumXFUnderline type)
{
    switch(type)
    {
    case enumXFUnderlineNone:
        return u"none"_ustr;
    case enumXFUnderlineSingle:
        return u"single"_ustr;
    case enumXFUnderlineDouble:
        return u"double"_ustr;
    case enumXFUnderlineDotted:
        return u"dotted"_ustr;
    case enumXFUnderlineDash:
        return u"dash"_ustr;
    case enumXFUnderlineLongDash:
        return u"long-dash"_ustr;
    case enumXFUnderlineDotDash:
        return u"dot-dash"_ustr;
    case enumXFUnderlineDotDotDash:
        return u"dot-dot-dash"_ustr;
    case enumXFUnderlineWave:
        return u"wave"_ustr;
    case enumXFUnderlineBold:
        return u"bold"_ustr;
    case enumXFUnderlineBoldDotted:
        return u"bold-dotted"_ustr;
    case enumXFUnderlineBoldDash:
        return u"bold-dash"_ustr;
    case enumXFUnderlineBoldLongDash:
        return u"bold-long-dash"_ustr;
    case enumXFUnderlineBoldDotDash:
        return u"bold-dot-dash"_ustr;
    case enumXFUnderlineBoldDotDotDash:
        return u"bold-dot-dot-dash"_ustr;
    case enumXFUnderlineBoldWave:
        return u"bold-wave"_ustr;
    case enumXFUnderlineDoubleWave:
        return u"double-wave"_ustr;
    case enumXFUnderlineSmallWave:
        return u"small-wave"_ustr;
    }
    return OUString();
}

OUString   GetCrossoutName(enumXFCrossout type)
{
    switch(type)
    {
    case enumXFCrossoutSignel:
        return u"single-line"_ustr;
    case enumXFCrossoutDouble:
        return u"double-line"_ustr;
    case enumXFCrossoutThick:
        return u"thick-line"_ustr;
    case enumXFCrossoutSlash:
        return u"slash"_ustr;
    case enumXFCrossoutX:
        return u"X"_ustr;
    default:
        break;
    }
    return OUString();
}

OUString   GetTransformName(enumXFTransform type)
{
    switch(type) {
    case enumXFTransformUpper:
        return u"uppercase"_ustr;
    case enumXFTransformLower:
        return u"lowercase"_ustr;
    case enumXFTransformCapitalize:
        return u"capitalize"_ustr;
    case enumXFTransformSmallCaps:
        return u"small-caps"_ustr;
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
        return u"lr"_ustr;
    case enumXFTextDirLR_TB:
        return u"lr-tb"_ustr;
    case enumXFTextDirPage:
        return u"page"_ustr;
    case enumXFTextDirRL:
        return u"rl"_ustr;
    case enumXFTextDirRL_TB:
        return u"rl-tb"_ustr;
    case enumXFTextDirTB:
        return u"tb"_ustr;
    case enumXFTextDirTB_LR:
        return u"tb-lr"_ustr;
    case enumXFTextDirTB_RL:
        return u"tb-rl"_ustr;
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
        return u"left"_ustr;
    case enumXFFrameXPosRight:
        return u"right"_ustr;
    case enumXFFrameXPosCenter:
        return u"center"_ustr;
    case enumXFFrameXPosFromLeft:
        return u"from-left"_ustr;
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
        return u"page"_ustr;
    case enumXFFrameXRelPageContent:
        return u"page-content"_ustr;
    case enumXFFrameXRelPageEndMargin:
        return u"page-end-margin"_ustr;
    case enumXFFrameXRelPageStartMargin:
        return u"page-start-margin"_ustr;
    case enumXFFrameXRelPara:
        return u"paragraph"_ustr;
    case enumXFFrameXRelParaContent:
        return u"paragraph-content"_ustr;
    case enumXFFrameXRelParaEndMargin:
        return u"paragraph-end-margin"_ustr;
    case enumXFFrameXRelParaStartMargin:
        return u"paragraph-start-margin"_ustr;
    case enumXFFrameXRelChar:
        return u"char"_ustr;
    case enumXFFrameXRelFrame:
        return u"frame"_ustr;
    case enumXFFrameXRelFrameContent:
        return u"frame-content"_ustr;
    case enumXFFrameXRelFrameEndMargin:
        return u"frame-end-margin"_ustr;
    case enumXFFrameXRelFrameStartMargin:
        return u"frame-start-margin"_ustr;
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
        return u"top"_ustr;
    case enumXFFrameYPosMiddle:
        return u"middle"_ustr;
    case enumXFFrameYPosBottom:
        return u"bottom"_ustr;
    case enumXFFrameYPosFromTop:
        return u"from-top"_ustr;
    case enumXFFrameYPosBelow:
        return u"below"_ustr;
    }
    return OUString();
}

OUString   GetFrameYRel(enumXFFrameYRel rel)
{
    switch(rel)
    {
    case enumXFFrameYRelBaseLine:
        return u"baseline"_ustr;
    case enumXFFrameYRelChar:
        return u"char"_ustr;
    case enumXFFrameYRelFrame:
        return u"frame"_ustr;
    case enumXFFrameYRelFrameContent:
        return u"frame-content"_ustr;
    case enumXFFrameYRelLine:
        return u"line"_ustr;
    case enumXFFrameYRelPage:
        return u"page"_ustr;
    case enumXFFrameYRelPageContent:
        return u"page-content"_ustr;
    case enumXFFrameYRelPara:
        return u"paragraph"_ustr;
    case enumXFFrameYRelParaContent:
        return u"paragraph-content"_ustr;
    case enumXFFrameYRelText:
        return u"text"_ustr;
    }
    return OUString();
}

OUString   GetAlignName(enumXFAlignType align)
{
    if( align == enumXFAlignStart )
        return u"start"_ustr;
    else if( align == enumXFAlignCenter )
        return u"center"_ustr;
    else if( align == enumXFAlignEnd )
        return u"end"_ustr;
    else if( align == enumXFAlignJustify )
        return u"justify"_ustr;
    else if( align == enumXFAlignBottom )
        return u"bottom"_ustr;
    else if( align == enumXFAlignTop )
        return u"top"_ustr;
    else if( align == enumXFAlignMiddle )
        return u"middle"_ustr;
    else if( align == enumXFALignMargins )
        return u"margins"_ustr;

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
        return u"standard"_ustr;
    case enumXFColorGreyscale:
        return u"greyscale"_ustr;
    case enumXFColorWatermark:
        return u"watermark"_ustr;
    case enumXFColorMono:
        return u"mono"_ustr;
    }
    return OUString();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
