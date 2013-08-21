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
#include <stdio.h>
#include "xfutil.hxx"
#include "xfparagraph.hxx"
#include "xfcontentcontainer.hxx"
#include <rtl/ustrbuf.hxx>
#include <sstream>

OUString Int32ToOUString(sal_Int32 num)
{
    /*std::stringstream sstrm;

    sstrm<<(int)num;
    return OUString::createFromAscii(sstrm.str().c_str());
    */
    return OUString::number(num);
}

OUString Int16ToOUString(sal_Int16 num)
{
    /*std::stringstream sstrm;

    sstrm<<(int)num;
    return OUString::createFromAscii(sstrm.str().c_str());
    */
    sal_Int32 nNum = static_cast<sal_Int32>(num);
    return OUString::number(nNum);
}

OUString   FloatToOUString(float num, sal_Int32 /*precision*/)
{
    /*std::stringstream sstrm;
    std::string         strRet;

    sstrm.precision(precision);
    sstrm<<num;
    return OUString::createFromAscii(sstrm.str().c_str());
    */
    return OUString::number(num);
}

OUString   DoubleToOUString(double num, sal_Int32 /*precision*/)
{
    /*std::stringstream sstrm;
    std::string         strRet;

    sstrm.precision(precision);
    sstrm<<num;
    return OUString::createFromAscii(sstrm.str().c_str());
    */
    return OUString::number(num);
}

OUString   DateTimeToOUString(XFDateTime& dt)
{
    OUStringBuffer buf;
    buf.append(dt.nYear);
    buf.append( A2OUSTR("-") );
    buf.append(dt.nMonth);
    buf.append( A2OUSTR("-") );
    buf.append(dt.nDay);
    buf.append( A2OUSTR("T") );
    buf.append(dt.nHour);
    buf.append( A2OUSTR(":") );
    buf.append(dt.nMinute);
    buf.append( A2OUSTR(":") );
    buf.append(dt.nSecond);
    buf.append( A2OUSTR(".") );
    buf.append(dt.nMillSecond);

    return buf.makeStringAndClear();
}

OUString   GetTableColName(sal_Int32 col)
{
    int     remain = 0;
    char    ch;
    std::string strOut;

    if( col <= 26 )
    {
        ch = 'A' + col -1;
        strOut += ch;
        return A2OUSTR(strOut.c_str());
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
    return A2OUSTR(strOut.c_str());
}

//tool functions:
OUString   GetUnderlineName(enumXFUnderline type)
{
    switch(type)
    {
    case enumXFUnderlineNone:
        return A2OUSTR("none");
        break;
    case enumXFUnderlineSingle:
        return A2OUSTR("single");
        break;
    case enumXFUnderlineDouble:
        return A2OUSTR("double");
        break;
    case enumXFUnderlineDotted:
        return A2OUSTR("dotted");
        break;
    case enumXFUnderlineDash:
        return A2OUSTR("dash");
        break;
    case enumXFUnderlineLongDash:
        return A2OUSTR("long-dash");
        break;
    case enumXFUnderlineDotDash:
        return A2OUSTR("dot-dash");
        break;
    case enumXFUnderlineDotDotDash:
        return A2OUSTR("dot-dot-dash");
        break;
    case enumXFUnderlineWave:
        return A2OUSTR("wave");
        break;
    case enumXFUnderlineBold:
        return A2OUSTR("bold");
        break;
    case enumXFUnderlineBoldDotted:
        return A2OUSTR("bold-dotted");
        break;
    case enumXFUnderlineBoldDash:
        return A2OUSTR("bold-dash");
        break;
    case enumXFUnderlineBoldLongDash:
        return A2OUSTR("bold-long-dash");
        break;
    case enumXFUnderlineBoldDotDash:
        return A2OUSTR("bold-dot-dash");
        break;
    case enumXFUnderlineBoldDotDotDash:
        return A2OUSTR("bold-dot-dot-dash");
        break;
    case enumXFUnderlineBoldWave:
        return A2OUSTR("bold-wave");
        break;
    case enumXFUnderlineDoubleWave:
        return A2OUSTR("double-wave");
        break;
    case enumXFUnderlineSmallWave:
        return A2OUSTR("samll-wave");
        break;
    }
    return A2OUSTR("");
}

OUString   GetReliefName(enumXFRelief type)
{
    switch(type)
    {
        case enumXFReliefEngraved:
            return A2OUSTR("engraved");
            break;
        case enumXFReliefEmbossed:
            return A2OUSTR("embossed");
            break;
        default:
            break;
    }
    return A2OUSTR("");
}

OUString   GetCrossoutName(enumXFCrossout type)
{
    switch(type)
    {
    case enumXFCrossoutSignel:
        return A2OUSTR("single-line");
        break;
    case enumXFCrossoutDouble:
        return A2OUSTR("double-line");
        break;
    case enumXFCrossoutThick:
        return A2OUSTR("thick-line");
        break;
    case enumXFCrossoutSlash:
        return A2OUSTR("slash");
        break;
    case enumXFCrossoutX:
        return A2OUSTR("X");
        break;
    default:
        break;
    }
    return A2OUSTR("");
}

OUString   GetTransformName(enumXFTransform type)
{
    switch(type) {
    case enumXFTransformUpper:
        return A2OUSTR("uppercase");
        break;
    case enumXFTransformLower:
        return A2OUSTR("lowercase");
        break;
    case enumXFTransformCapitalize:
        return A2OUSTR("capitalize");
        break;
    case enumXFTransformSmallCaps:
        return A2OUSTR("small-caps");
        break;
    default:
        break;
    }
    return A2OUSTR("");
}

OUString   GetEmphasizeName(enumXFEmphasize type)
{
    switch(type) {
    case enumXFEmphasizeDot:
        return A2OUSTR("dot");
        break;
    case enumXFEmphasizeCircle:
        return A2OUSTR("circle");
        break;
    case enumXFEmphasizeDisc:
        return A2OUSTR("disc");
        break;
    case enumXFEmphasizeAccent:
        return A2OUSTR("accent");
        break;
    default:
        break;
    }
    return A2OUSTR("");
}

OUString   GetTextDirName(enumXFTextDir dir)
{
    switch(dir)
    {
    case enumXFTextDirLR:
        return A2OUSTR("lr");
        break;
    case enumXFTextDirLR_TB:
        return A2OUSTR("lr-tb");
        break;
    case enumXFTextDirPage:
        return A2OUSTR("page");
        break;
    case enumXFTextDirRL:
        return A2OUSTR("rl");
        break;
    case enumXFTextDirRL_TB:
        return A2OUSTR("rl-tb");
        break;
    case enumXFTextDirTB:
        return A2OUSTR("tb");
        break;
    case enumXFTextDirTB_LR:
        return A2OUSTR("tb-lr");
        break;
    case enumXFTextDirTB_RL:
        return A2OUSTR("tb-rl");
        break;
    default:
        break;
    }
    return A2OUSTR("");
}

OUString   GetFrameXPos(enumXFFrameXPos pos)
{
    switch(pos)
    {
    case enumXFFrameXPosLeft:
        return A2OUSTR("left");
        break;
    case enumXFFrameXPosRight:
        return A2OUSTR("right");
        break;
    case enumXFFrameXPosCenter:
        return A2OUSTR("center");
        break;
    case enumXFFrameXPosFromLeft:
        return A2OUSTR("from-left");
        break;
    default:
        break;
    }
    return A2OUSTR("");
}

OUString   GetFrameXRel(enumXFFrameXRel rel)
{
    switch(rel)
    {
    case enumXFFrameXRelPage:
        return A2OUSTR("page");
        break;
    case enumXFFrameXRelPageContent:
        return A2OUSTR("page-content");
        break;
    case enumXFFrameXRelPageEndMargin:
        return A2OUSTR("page-end-margin");
        break;
    case enumXFFrameXRelPageStartMargin:
        return A2OUSTR("page-start-margin");
        break;
    case enumXFFrameXRelPara:
        return A2OUSTR("paragraph");
        break;
    case enumXFFrameXRelParaContent:
        return A2OUSTR("paragraph-content");
        break;
    case enumXFFrameXRelParaEndMargin:
        return A2OUSTR("paragraph-end-margin");
        break;
    case enumXFFrameXRelParaStartMargin:
        return A2OUSTR("paragraph-start-margin");
        break;
    case enumXFFrameXRelChar:
        return A2OUSTR("char");
        break;
    case enumXFFrameXRelFrame:
        return A2OUSTR("frame");
        break;
    case enumXFFrameXRelFrameContent:
        return A2OUSTR("frame-content");
        break;
    case enumXFFrameXRelFrameEndMargin:
        return A2OUSTR("frame-end-margin");
        break;
    case enumXFFrameXRelFrameStartMargin:
        return A2OUSTR("frame-start-margin");
        break;
    default:
        break;
    }
    return A2OUSTR("");
}

OUString   GetFrameYPos(enumXFFrameYPos pos)
{
    switch(pos)
    {
    case enumXFFrameYPosTop:
        return A2OUSTR("top");
    case enumXFFrameYPosMiddle:
        return A2OUSTR("middle");
    case enumXFFrameYPosBottom:
        return A2OUSTR("bottom");
    case enumXFFrameYPosFromTop:
        return A2OUSTR("from-top");
    case enumXFFrameYPosBelow:
        return A2OUSTR("below");
    }
    return A2OUSTR("");
}

OUString   GetFrameYRel(enumXFFrameYRel rel)
{
    switch(rel)
    {
    case enumXFFrameYRelBaseLine:
        return A2OUSTR("baseline");
    case enumXFFrameYRelChar:
        return A2OUSTR("char");
    case enumXFFrameYRelFrame:
        return A2OUSTR("frame");
    case enumXFFrameYRelFrameContent:
        return A2OUSTR("frame-content");
    case enumXFFrameYRelLine:
        return A2OUSTR("line");
    case enumXFFrameYRelPage:
        return A2OUSTR("page");
    case enumXFFrameYRelPageContent:
        return A2OUSTR("page-content");
    case enumXFFrameYRelPara:
        return A2OUSTR("paragraph");
    case enumXFFrameYRelParaContent:
        return A2OUSTR("paragraph-content");
    case enumXFFrameYRelText:
        return A2OUSTR("text");
    }
    return A2OUSTR("");
}

OUString   GetAlignName(enumXFAlignType align)
{
    if( align == enumXFAlignStart )
        return A2OUSTR("start");
    else if( align == enumXFAlignCenter )
        return A2OUSTR("center");
    else if( align == enumXFAlignEnd )
        return A2OUSTR("end");
    else if( align == enumXFAlignJustify )
        return A2OUSTR("justify");
    else if( align == enumXFAlignBottom )
        return A2OUSTR("bottom");
    else if( align == enumXFAlignTop )
        return A2OUSTR("top");
    else if( align == enumXFAlignMiddle )
        return A2OUSTR("middle");
    else if( align == enumXFALignMargins )
        return A2OUSTR("margins");

    return A2OUSTR("");
}

OUString   GetDrawKind(enumXFDrawKind kind)
{
    if( kind == enumXFDrawKindFull )
        return A2OUSTR("full");
    else if( kind == enumXFDrawKindSection )
        return A2OUSTR("section");
    else if( kind == enumXFDrawKindCut )
        return A2OUSTR("cut");
    else    //( kind == enumXFDrawKindArc )
        return A2OUSTR("arc");
}

OUString   GetPageUsageName(enumXFPageUsage usage)
{
    OUString sRet;
    switch(usage)
    {
    case enumXFPageUsageAll:
        sRet = A2OUSTR("all");
        break;
    case enumXFPageUsageLeft:
        sRet = A2OUSTR("left");
        break;
    case enumXFPageUsageRight:
        sRet = A2OUSTR("right");
        break;
    case enumXFPageUsageMirror:
        sRet = A2OUSTR("mirrored");
        break;
    default:
        sRet = A2OUSTR("mirrored");
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
        sRet = A2OUSTR("boolean");
        break;
    case enumXFValueTypeCurrency:
        sRet = A2OUSTR("currency");
        break;
    case enumXFValueTypeDate:
        sRet = A2OUSTR("date");
        break;
    case enumXFValueTypeFloat:
        sRet = A2OUSTR("float");
        break;
    case enumXFValueTypePercentage:
        sRet = A2OUSTR("percentage");
        break;
    case enumXFValueTypeString:
        sRet = A2OUSTR("string");
        break;
    case enumXFValueTypeTime:
        sRet = A2OUSTR("time");
        break;
    default:
        sRet = A2OUSTR("mirrored");
        break;
    }
    return sRet;
}

OUString   GetColorMode(enumXFColorMode mode)
{
    switch(mode)
    {
    case enumXFColorStandard:
        return A2OUSTR("standard");
    case enumXFColorGreyscale:
        return A2OUSTR("greyscale");
    case enumXFColorWatermark:
        return A2OUSTR("watermark");
    case enumXFColorMono:
        return A2OUSTR("mono");
    }
    return A2OUSTR("");
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
