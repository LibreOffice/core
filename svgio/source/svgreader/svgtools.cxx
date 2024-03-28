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

#include <svgtools.hxx>
#include <sal/log.hxx>
#include <tools/color.hxx>
#include <rtl/math.hxx>
#include <o3tl/string_view.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <basegfx/matrix/b3dhommatrix.hxx>
#include <svgtoken.hxx>
#include <frozen/bits/defines.h>
#include <frozen/bits/elsa_std.h>
#include <frozen/unordered_map.h>

namespace svgio::svgreader
{
        constexpr auto aColorTokenMapperList = frozen::make_unordered_map<std::u16string_view, Color>(
        {
            { u"aliceblue", Color(240, 248, 255) },
            { u"antiquewhite", Color(250, 235, 215) },
            { u"aqua", Color( 0, 255, 255) },
            { u"aquamarine", Color(127, 255, 212) },
            { u"azure", Color(240, 255, 255) },
            { u"beige", Color(245, 245, 220) },
            { u"bisque", Color(255, 228, 196) },
            { u"black", Color( 0, 0, 0) },
            { u"blanchedalmond", Color(255, 235, 205) },
            { u"blue", Color( 0, 0, 255) },
            { u"blueviolet", Color(138, 43, 226) },
            { u"brown", Color(165, 42, 42) },
            { u"burlywood", Color(222, 184, 135) },
            { u"cadetblue", Color( 95, 158, 160) },
            { u"chartreuse", Color(127, 255, 0) },
            { u"chocolate", Color(210, 105, 30) },
            { u"coral", Color(255, 127, 80) },
            { u"cornflowerblue", Color(100, 149, 237) },
            { u"cornsilk", Color(255, 248, 220) },
            { u"crimson", Color(220, 20, 60) },
            { u"cyan", Color( 0, 255, 255) },
            { u"darkblue", Color( 0, 0, 139) },
            { u"darkcyan", Color( 0, 139, 139) },
            { u"darkgoldenrod", Color(184, 134, 11) },
            { u"darkgray", Color(169, 169, 169) },
            { u"darkgreen", Color( 0, 100, 0) },
            { u"darkgrey", Color(169, 169, 169) },
            { u"darkkhaki", Color(189, 183, 107) },
            { u"darkmagenta", Color(139, 0, 139) },
            { u"darkolivegreen", Color( 85, 107, 47) },
            { u"darkorange", Color(255, 140, 0) },
            { u"darkorchid", Color(153, 50, 204) },
            { u"darkred", Color(139, 0, 0) },
            { u"darksalmon", Color(233, 150, 122) },
            { u"darkseagreen", Color(143, 188, 143) },
            { u"darkslateblue", Color( 72, 61, 139) },
            { u"darkslategray", Color( 47, 79, 79) },
            { u"darkslategrey", Color( 47, 79, 79) },
            { u"darkturquoise", Color( 0, 206, 209) },
            { u"darkviolet", Color(148, 0, 211) },
            { u"deeppink", Color(255, 20, 147) },
            { u"deepskyblue", Color( 0, 191, 255) },
            { u"dimgray", Color(105, 105, 105) },
            { u"dimgrey", Color(105, 105, 105) },
            { u"dodgerblue", Color( 30, 144, 255) },
            { u"firebrick", Color(178, 34, 34) },
            { u"floralwhite", Color(255, 250, 240) },
            { u"forestgreen", Color( 34, 139, 34) },
            { u"fuchsia", Color(255, 0, 255) },
            { u"gainsboro", Color(220, 220, 220) },
            { u"ghostwhite", Color(248, 248, 255) },
            { u"gold", Color(255, 215, 0) },
            { u"goldenrod", Color(218, 165, 32) },
            { u"gray", Color(128, 128, 128) },
            { u"grey", Color(128, 128, 128) },
            { u"green", Color(0, 128, 0) },
            { u"greenyellow", Color(173, 255, 47) },
            { u"honeydew", Color(240, 255, 240) },
            { u"hotpink", Color(255, 105, 180) },
            { u"indianred", Color(205, 92, 92) },
            { u"indigo", Color( 75, 0, 130) },
            { u"ivory", Color(255, 255, 240) },
            { u"khaki", Color(240, 230, 140) },
            { u"lavender", Color(230, 230, 250) },
            { u"lavenderblush", Color(255, 240, 245) },
            { u"lawngreen", Color(124, 252, 0) },
            { u"lemonchiffon", Color(255, 250, 205) },
            { u"lightblue", Color(173, 216, 230) },
            { u"lightcoral", Color(240, 128, 128) },
            { u"lightcyan", Color(224, 255, 255) },
            { u"lightgoldenrodyellow", Color(250, 250, 210) },
            { u"lightgray", Color(211, 211, 211) },
            { u"lightgreen", Color(144, 238, 144) },
            { u"lightgrey", Color(211, 211, 211) },
            { u"lightpink", Color(255, 182, 193) },
            { u"lightsalmon", Color(255, 160, 122) },
            { u"lightseagreen", Color( 32, 178, 170) },
            { u"lightskyblue", Color(135, 206, 250) },
            { u"lightslategray", Color(119, 136, 153) },
            { u"lightslategrey", Color(119, 136, 153) },
            { u"lightsteelblue", Color(176, 196, 222) },
            { u"lightyellow", Color(255, 255, 224) },
            { u"lime", Color( 0, 255, 0) },
            { u"limegreen", Color( 50, 205, 50) },
            { u"linen", Color(250, 240, 230) },
            { u"magenta", Color(255, 0, 255) },
            { u"maroon", Color(128, 0, 0) },
            { u"mediumaquamarine", Color(102, 205, 170) },
            { u"mediumblue", Color( 0, 0, 205) },
            { u"mediumorchid", Color(186, 85, 211) },
            { u"mediumpurple", Color(147, 112, 219) },
            { u"mediumseagreen", Color( 60, 179, 113) },
            { u"mediumslateblue", Color(123, 104, 238) },
            { u"mediumspringgreen", Color( 0, 250, 154) },
            { u"mediumturquoise", Color( 72, 209, 204) },
            { u"mediumvioletred", Color(199, 21, 133) },
            { u"midnightblue", Color( 25, 25, 112) },
            { u"mintcream", Color(245, 255, 250) },
            { u"mistyrose", Color(255, 228, 225) },
            { u"moccasin", Color(255, 228, 181) },
            { u"navajowhite", Color(255, 222, 173) },
            { u"navy", Color( 0, 0, 128) },
            { u"oldlace", Color(253, 245, 230) },
            { u"olive", Color(128, 128, 0) },
            { u"olivedrab", Color(107, 142, 35) },
            { u"orange", Color(255, 165, 0) },
            { u"orangered", Color(255, 69, 0) },
            { u"orchid", Color(218, 112, 214) },
            { u"palegoldenrod", Color(238, 232, 170) },
            { u"palegreen", Color(152, 251, 152) },
            { u"paleturquoise", Color(175, 238, 238) },
            { u"palevioletred", Color(219, 112, 147) },
            { u"papayawhip", Color(255, 239, 213) },
            { u"peachpuff", Color(255, 218, 185) },
            { u"peru", Color(205, 133, 63) },
            { u"pink", Color(255, 192, 203) },
            { u"plum", Color(221, 160, 221) },
            { u"powderblue", Color(176, 224, 230) },
            { u"purple", Color(128, 0, 128) },
            { u"red", Color(255, 0, 0) },
            { u"rosybrown", Color(188, 143, 143) },
            { u"royalblue", Color( 65, 105, 225) },
            { u"saddlebrown", Color(139, 69, 19) },
            { u"salmon", Color(250, 128, 114) },
            { u"sandybrown", Color(244, 164, 96) },
            { u"seagreen", Color( 46, 139, 87) },
            { u"seashell", Color(255, 245, 238) },
            { u"sienna", Color(160, 82, 45) },
            { u"silver", Color(192, 192, 192) },
            { u"skyblue", Color(135, 206, 235) },
            { u"slateblue", Color(106, 90, 205) },
            { u"slategray", Color(112, 128, 144) },
            { u"slategrey", Color(112, 128, 144) },
            { u"snow", Color(255, 250, 250) },
            { u"springgreen", Color( 0, 255, 127) },
            { u"steelblue", Color( 70, 130, 180) },
            { u"tan", Color(210, 180, 140) },
            { u"teal", Color( 0, 128, 128) },
            { u"thistle", Color(216, 191, 216) },
            { u"tomato", Color(255, 99, 71) },
            { u"turquoise", Color( 64, 224, 208) },
            { u"violet", Color(238, 130, 238) },
            { u"wheat", Color(245, 222, 179) },
            { u"white", Color(255, 255, 255) },
            { u"whitesmoke", Color(245, 245, 245) },
            { u"yellow", Color(255, 255, 0) },
            { u"yellowgreen", Color(154, 205, 50) }
        });

        basegfx::B2DHomMatrix SvgAspectRatio::createLinearMapping(const basegfx::B2DRange& rTarget, const basegfx::B2DRange& rSource)
        {
            basegfx::B2DHomMatrix aRetval;
            const double fSWidth(rSource.getWidth());
            const double fSHeight(rSource.getHeight());
            const bool bNoSWidth(basegfx::fTools::equalZero(fSWidth));
            const bool bNoSHeight(basegfx::fTools::equalZero(fSHeight));

            // transform from source state to unit range
            aRetval.translate(-rSource.getMinX(), -rSource.getMinY());
            aRetval.scale(
                (bNoSWidth ? 1.0 : 1.0 / fSWidth) * rTarget.getWidth(),
                (bNoSHeight ? 1.0 : 1.0 / fSHeight) * rTarget.getHeight());

            // transform from unit rage to target range
            aRetval.translate(rTarget.getMinX(), rTarget.getMinY());

            return aRetval;
        }

        basegfx::B2DHomMatrix SvgAspectRatio::createMapping(const basegfx::B2DRange& rTarget, const basegfx::B2DRange& rSource) const
        {
            // removed !isSet() from below. Due to correct defaults in the constructor an instance
            // of this class is perfectly useful without being set by any importer
            if(SvgAlign::none == getSvgAlign())
            {
                // create linear mapping (default)
                return createLinearMapping(rTarget, rSource);
            }

            basegfx::B2DHomMatrix aRetval;

            const double fSWidth(rSource.getWidth());
            const double fSHeight(rSource.getHeight());
            const bool bNoSWidth(basegfx::fTools::equalZero(fSWidth));
            const bool bNoSHeight(basegfx::fTools::equalZero(fSHeight));
            const double fScaleX((bNoSWidth ? 1.0 : 1.0 / fSWidth) * rTarget.getWidth());
            const double fScaleY((bNoSHeight ? 1.0 : 1.0 / fSHeight) * rTarget.getHeight());
            const double fScale(isMeetOrSlice() ? std::min(fScaleX, fScaleY) : std::max(fScaleX, fScaleY));

            // remove source translation, apply scale
            aRetval.translate(-rSource.getMinX(), -rSource.getMinY());
            aRetval.scale(fScale, fScale);

            // evaluate horizontal alignment
            const double fNewWidth(fSWidth * fScale);
            double fTransX(0.0);

            switch(getSvgAlign())
            {
                case SvgAlign::xMidYMin:
                case SvgAlign::xMidYMid:
                case SvgAlign::xMidYMax:
                {
                    // centerX
                    const double fFreeSpace(rTarget.getWidth() - fNewWidth);
                    fTransX = fFreeSpace * 0.5;
                    break;
                }
                case SvgAlign::xMaxYMin:
                case SvgAlign::xMaxYMid:
                case SvgAlign::xMaxYMax:
                {
                    // Right align
                    const double fFreeSpace(rTarget.getWidth() - fNewWidth);
                    fTransX = fFreeSpace;
                    break;
                }
                default: break;
            }

            // evaluate vertical alignment
            const double fNewHeight(fSHeight * fScale);
            double fTransY(0.0);

            switch(getSvgAlign())
            {
                case SvgAlign::xMinYMid:
                case SvgAlign::xMidYMid:
                case SvgAlign::xMaxYMid:
                {
                    // centerY
                    const double fFreeSpace(rTarget.getHeight() - fNewHeight);
                    fTransY = fFreeSpace * 0.5;
                    break;
                }
                case SvgAlign::xMinYMax:
                case SvgAlign::xMidYMax:
                case SvgAlign::xMaxYMax:
                {
                    // Bottom align
                    const double fFreeSpace(rTarget.getHeight() - fNewHeight);
                    fTransY = fFreeSpace;
                    break;
                }
                default: break;
            }

            // add target translation
            aRetval.translate(
                rTarget.getMinX() + fTransX,
                rTarget.getMinY() + fTransY);

            return aRetval;
        }

        void skip_char(std::u16string_view rCandidate, sal_Unicode nChar, sal_Int32& nPos, const sal_Int32 nLen)
        {
            while(nPos < nLen && nChar == rCandidate[nPos])
            {
                nPos++;
            }
        }

        void skip_char(std::u16string_view rCandidate, sal_Unicode nCharA, sal_Unicode nCharB, sal_Int32& nPos, const sal_Int32 nLen)
        {
            while(nPos < nLen && (nCharA == rCandidate[nPos] || nCharB == rCandidate[nPos]))
            {
                nPos++;
            }
        }

        void copySign(std::u16string_view rCandidate, sal_Int32& nPos, OUStringBuffer& rTarget, const sal_Int32 nLen)
        {
            if(nPos < nLen)
            {
                const sal_Unicode aChar(rCandidate[nPos]);

                if('+' == aChar || '-' == aChar)
                {
                    rTarget.append(aChar);
                    nPos++;
                }
            }
        }

        void copyNumber(std::u16string_view rCandidate, sal_Int32& nPos, OUStringBuffer& rTarget, const sal_Int32 nLen)
        {
            bool bOnNumber(true);

            while(bOnNumber && nPos < nLen)
            {
                const sal_Unicode aChar(rCandidate[nPos]);

                bOnNumber = ('0' <= aChar && '9' >= aChar) || '.' == aChar;

                if(bOnNumber)
                {
                    rTarget.append(aChar);
                    nPos++;
                }
            }
        }

        void copyHex(std::u16string_view rCandidate, sal_Int32& nPos, OUStringBuffer& rTarget, const sal_Int32 nLen)
        {
            bool bOnHex(true);

            while(bOnHex && nPos < nLen)
            {
                const sal_Unicode aChar(rCandidate[nPos]);

                bOnHex = ('0' <= aChar && '9' >= aChar)
                    || ('A' <= aChar && 'F' >= aChar)
                    || ('a' <= aChar && 'f' >= aChar);

                if(bOnHex)
                {
                    rTarget.append(aChar);
                    nPos++;
                }
            }
        }

        void copyString(std::u16string_view rCandidate, sal_Int32& nPos, OUStringBuffer& rTarget, const sal_Int32 nLen)
        {
            bool bOnChar(true);

            while(bOnChar && nPos < nLen)
            {
                const sal_Unicode aChar(rCandidate[nPos]);

                bOnChar = ('a' <= aChar && 'z' >= aChar)
                    || ('A' <= aChar && 'Z' >= aChar)
                    || '-' == aChar;

                if(bOnChar)
                {
                    rTarget.append(aChar);
                    nPos++;
                }
            }
        }

        void copyToLimiter(std::u16string_view rCandidate, sal_Unicode nLimiter, sal_Int32& nPos, OUStringBuffer& rTarget, const sal_Int32 nLen)
        {
            while(nPos < nLen && nLimiter != rCandidate[nPos])
            {
                rTarget.append(rCandidate[nPos]);
                nPos++;
            }
        }

        bool readNumber(std::u16string_view rCandidate, sal_Int32& nPos, double& fNum, const sal_Int32 nLen)
        {
            if(nPos < nLen)
            {
                OUStringBuffer aNum;

                copySign(rCandidate, nPos, aNum, nLen);
                copyNumber(rCandidate, nPos, aNum, nLen);

                if(nPos < nLen)
                {
                    const sal_Unicode aChar(rCandidate[nPos]);

                    if('e' == aChar || 'E' == aChar)
                    {
                        // try to read exponential number, but be careful. I had
                        // a case where dx="2em" was used, thus the 'e' was consumed
                        // by error. First try if there are numbers after the 'e',
                        // safe current state
                        nPos++;
                        const OUStringBuffer aNum2(aNum);
                        const sal_Int32 nPosAfterE(nPos);

                        aNum.append(aChar);
                        copySign(rCandidate, nPos, aNum, nLen);
                        copyNumber(rCandidate, nPos, aNum, nLen);

                        if(nPosAfterE == nPos)
                        {
                            // no number after 'e', go back. Do not
                            // return false, it's still a valid integer number
                            aNum = aNum2;
                            nPos--;
                        }
                    }
                }

                if(!aNum.isEmpty())
                {
                    rtl_math_ConversionStatus eStatus;

                    fNum = rtl::math::stringToDouble(
                        aNum, '.', ',',
                        &eStatus);

                    return eStatus == rtl_math_ConversionStatus_Ok;
                }
            }

            return false;
        }

        SvgUnit readUnit(std::u16string_view rCandidate, sal_Int32& nPos, const sal_Int32 nLen)
        {
            SvgUnit aRetval(SvgUnit::px);

            if(nPos < nLen)
            {
                const sal_Unicode aCharA(rCandidate[nPos]);

                if('%' == aCharA)
                {
                    // percent used, relative to current
                    nPos++;
                    aRetval = SvgUnit::percent;
                }
                else if(nPos + 1 < nLen)
                {
                    const sal_Unicode aCharB(rCandidate[nPos + 1]);
                    bool bTwoCharValid(false);

                    switch(aCharA)
                    {
                        case u'e' :
                        {
                            if('m' == aCharB)
                            {
                                // 'em' Relative to current font size
                                aRetval = SvgUnit::em;
                                bTwoCharValid = true;
                            }
                            else if('x' == aCharB)
                            {
                                // 'ex' Relative to current font x-height
                                aRetval = SvgUnit::ex;
                                bTwoCharValid = true;
                            }
                            break;
                        }
                        case u'p' :
                        {
                            if('x' == aCharB)
                            {
                                // 'px' UserUnit (default)
                                bTwoCharValid = true;
                            }
                            else if('t' == aCharB)
                            {
                                // 'pt' == 4/3 px
                                aRetval = SvgUnit::pt;
                                bTwoCharValid = true;
                            }
                            else if('c' == aCharB)
                            {
                                // 'pc' == 16 px
                                aRetval = SvgUnit::pc;
                                bTwoCharValid = true;
                            }
                            break;
                        }
                        case u'i' :
                        {
                            if('n' == aCharB)
                            {
                                // 'in' == 96 px, since CSS 2.1
                                aRetval = SvgUnit::in;
                                bTwoCharValid = true;
                            }
                            break;
                        }
                        case u'c' :
                        {
                            if('m' == aCharB)
                            {
                                // 'cm' == 37.79527559 px
                                aRetval = SvgUnit::cm;
                                bTwoCharValid = true;
                            }
                            break;
                        }
                        case u'm' :
                        {
                            if('m' == aCharB)
                            {
                                // 'mm' == 3.779528 px
                                aRetval = SvgUnit::mm;
                                bTwoCharValid = true;
                            }
                            break;
                        }
                    }

                    if(bTwoCharValid)
                    {
                        nPos += 2;
                    }
                }
            }

            return aRetval;
        }

        bool readNumberAndUnit(std::u16string_view rCandidate, sal_Int32& nPos, SvgNumber& aNum, const sal_Int32 nLen)
        {
            double fNum(0.0);

            if(readNumber(rCandidate, nPos, fNum, nLen))
            {
                skip_char(rCandidate, ' ', nPos, nLen);
                aNum = SvgNumber(fNum, readUnit(rCandidate, nPos, nLen));

                return true;
            }

            return false;
        }

        bool readAngle(std::u16string_view rCandidate, sal_Int32& nPos, double& fAngle, const sal_Int32 nLen)
        {
            if(readNumber(rCandidate, nPos, fAngle, nLen))
            {
                skip_char(rCandidate, ' ', nPos, nLen);

                enum class DegreeType
                {
                    deg,
                    grad,
                    rad
                } aType(DegreeType::deg); // degrees is default

                if(nPos < nLen)
                {
                    const sal_Unicode aChar(rCandidate[nPos]);
                    static constexpr std::u16string_view aStrGrad = u"grad";
                    static constexpr std::u16string_view aStrRad = u"rad";

                    switch(aChar)
                    {
                        case u'g' :
                        case u'G' :
                        {
                            if(o3tl::matchIgnoreAsciiCase(rCandidate, aStrGrad, nPos))
                            {
                                // angle in grad
                                nPos += aStrGrad.size();
                                aType = DegreeType::grad;
                            }
                            break;
                        }
                        case u'r' :
                        case u'R' :
                        {
                            if(o3tl::matchIgnoreAsciiCase(rCandidate, aStrRad, nPos))
                            {
                                // angle in radians
                                nPos += aStrRad.size();
                                aType = DegreeType::rad;
                            }
                            break;
                        }
                    }
                }

                // convert to radians
                if (DegreeType::deg == aType)
                {
                    fAngle = basegfx::deg2rad(fAngle);
                }
                else if (DegreeType::grad == aType)
                {
                    // looks like 100 grad is 90 degrees
                    fAngle *= M_PI / 200.0;
                }

                return true;
            }

            return false;
        }

        sal_Int32 read_hex(sal_Unicode nChar)
        {
            if(nChar >= '0' && nChar <= '9')
            {
                return nChar - u'0';
            }
            else if(nChar >= 'A' && nChar <= 'F')
            {
                return 10 + sal_Int32(nChar - u'A');
            }
            else if(nChar >= 'a' && nChar <= 'f')
            {
                return 10 + sal_Int32(nChar - u'a');
            }
            else
            {
                // error
                return 0;
            }
        }

        bool match_colorKeyword(basegfx::BColor& rColor, const OUString& rName)
        {
            auto const aResult = aColorTokenMapperList.find(rName.toAsciiLowerCase().trim());

            if(aResult == aColorTokenMapperList.end())
            {
                return false;
            }
            else
            {
                rColor = aResult->second.getBColor();
                return true;
            }
        }

        bool read_color(const OUString& rCandidate, basegfx::BColor& rColor, SvgNumber& rOpacity)
        {
            const sal_Int32 nLen(rCandidate.getLength());

            if(nLen)
            {
                const sal_Unicode aChar(rCandidate[0]);
                const double fFactor(1.0 / 255.0);

                if(aChar == '#')
                {
                    // hex definition
                    OUStringBuffer aNum;
                    sal_Int32 nPos(1);

                    copyHex(rCandidate, nPos, aNum, nLen);
                    const sal_Int32 nLength(aNum.getLength());

                    if(3 == nLength)
                    {
                        const sal_Int32 nR(read_hex(aNum[0]));
                        const sal_Int32 nG(read_hex(aNum[1]));
                        const sal_Int32 nB(read_hex(aNum[2]));

                        rColor.setRed((nR | (nR << 4)) * fFactor);
                        rColor.setGreen((nG | (nG << 4)) * fFactor);
                        rColor.setBlue((nB | (nB << 4)) * fFactor);

                        return true;
                    }
                    else if(6 == nLength)
                    {
                        const sal_Int32 nR1(read_hex(aNum[0]));
                        const sal_Int32 nR2(read_hex(aNum[1]));
                        const sal_Int32 nG1(read_hex(aNum[2]));
                        const sal_Int32 nG2(read_hex(aNum[3]));
                        const sal_Int32 nB1(read_hex(aNum[4]));
                        const sal_Int32 nB2(read_hex(aNum[5]));

                        rColor.setRed((nR2 | (nR1 << 4)) * fFactor);
                        rColor.setGreen((nG2 | (nG1 << 4)) * fFactor);
                        rColor.setBlue((nB2 | (nB1 << 4)) * fFactor);

                        return true;
                    }
                }
                else
                {
                    static const char aStrRgb[] = "rgb";

                    if(rCandidate.matchIgnoreAsciiCase(aStrRgb, 0))
                    {
                        // rgb/rgba definition
                        sal_Int32 nPos(strlen(aStrRgb));
                        bool bIsRGBA = false;

                        if('a' == rCandidate[nPos])
                        {
                            //Delete the 'a' from 'rbga'
                            skip_char(rCandidate, 'a', nPos, nPos + 1);
                            bIsRGBA = true;
                        }

                        skip_char(rCandidate, ' ', '(', nPos, nLen);
                        double fR(0.0);

                        if(readNumber(rCandidate, nPos, fR, nLen))
                        {
                            skip_char(rCandidate, ' ', nPos, nLen);

                            if(nPos < nLen)
                            {
                                const sal_Unicode aPercentChar(rCandidate[nPos]);
                                const bool bIsPercent('%' == aPercentChar);
                                double fG(0.0);

                                if(bIsPercent)
                                {
                                    skip_char(rCandidate, '%', nPos, nLen);
                                }

                                skip_char(rCandidate, ' ', ',', nPos, nLen);

                                if(readNumber(rCandidate, nPos, fG, nLen))
                                {
                                    double fB(0.0);

                                    if(bIsPercent)
                                    {
                                        skip_char(rCandidate, '%', nPos, nLen);
                                    }

                                    skip_char(rCandidate, ' ', ',', nPos, nLen);

                                    if(readNumber(rCandidate, nPos, fB, nLen))
                                    {
                                        double fA(1.0);

                                        if(bIsPercent)
                                        {
                                            skip_char(rCandidate, '%', nPos, nLen);
                                        }

                                        skip_char(rCandidate, ' ', ',', nPos, nLen);

                                        if(readNumber(rCandidate, nPos, fA, nLen))
                                        {
                                            if(bIsRGBA)
                                            {
                                                const double fFac(bIsPercent ? 0.01 : 1);
                                                rOpacity = SvgNumber(fA * fFac);

                                                if(bIsPercent)
                                                {
                                                    skip_char(rCandidate, '%', nPos, nLen);
                                                }
                                            }
                                            else
                                            {
                                                return false;
                                            }
                                        }

                                        const double fFac(bIsPercent ? 0.01 : fFactor);

                                        rColor.setRed(fR * fFac);
                                        rColor.setGreen(fG * fFac);
                                        rColor.setBlue(fB * fFac);

                                        skip_char(rCandidate, ' ', ')', nPos, nLen);
                                        return true;
                                    }
                                }
                            }
                        }
                    }
                    else
                    {
                        // color keyword
                        if(match_colorKeyword(rColor, rCandidate))
                        {
                            return true;
                        }
                    }
                }
            }

            return false;
        }

        basegfx::B2DRange readViewBox(std::u16string_view rCandidate, InfoProvider const & rInfoProvider)
        {
            const sal_Int32 nLen(rCandidate.size());

            if(nLen)
            {
                sal_Int32 nPos(0);
                SvgNumber aMinX;
                skip_char(rCandidate, ' ', ',', nPos, nLen);

                if(readNumberAndUnit(rCandidate, nPos, aMinX, nLen))
                {
                    SvgNumber aMinY;
                    skip_char(rCandidate, ' ', ',', nPos, nLen);

                    if(readNumberAndUnit(rCandidate, nPos, aMinY, nLen))
                    {
                        SvgNumber aWidth;
                        skip_char(rCandidate, ' ', ',', nPos, nLen);

                        if(readNumberAndUnit(rCandidate, nPos, aWidth, nLen))
                        {
                            SvgNumber aHeight;
                            skip_char(rCandidate, ' ', ',', nPos, nLen);

                            if(readNumberAndUnit(rCandidate, nPos, aHeight, nLen))
                            {
                                double fX(aMinX.solve(rInfoProvider, NumberType::xcoordinate));
                                double fY(aMinY.solve(rInfoProvider, NumberType::ycoordinate));
                                double fW(aWidth.solve(rInfoProvider, NumberType::xcoordinate));
                                double fH(aHeight.solve(rInfoProvider, NumberType::ycoordinate));
                                return basegfx::B2DRange(fX,fY,fX+fW,fY+fH);
                            }
                        }
                    }
                }
            }

            return basegfx::B2DRange();
        }

        std::vector<double> readFilterMatrix(std::u16string_view rCandidate, InfoProvider const & rInfoProvider)
        {
            std::vector<double> aVector;
            const sal_Int32 nLen(rCandidate.size());

            sal_Int32 nPos(0);
            skip_char(rCandidate, ' ', ',', nPos, nLen);

            SvgNumber aVal;

            while (nPos < nLen)
            {
                if(readNumberAndUnit(rCandidate, nPos, aVal, nLen))
                {
                    aVector.push_back(aVal.solve(rInfoProvider));
                    skip_char(rCandidate, ' ', ',', nPos, nLen);
                }
            }

            return aVector;
        }

        basegfx::B2DHomMatrix readTransform(std::u16string_view rCandidate, InfoProvider const & rInfoProvider)
        {
            basegfx::B2DHomMatrix aMatrix;
            const sal_Int32 nLen(rCandidate.size());

            if(nLen)
            {
                sal_Int32 nPos(0);
                skip_char(rCandidate, ' ', ',', nPos, nLen);

                while(nPos < nLen)
                {
                    const sal_Unicode aChar(rCandidate[nPos]);
                    const sal_Int32 nInitPos(nPos);
                    static constexpr std::u16string_view aStrMatrix = u"matrix";
                    static constexpr std::u16string_view aStrTranslate = u"translate";
                    static constexpr std::u16string_view aStrScale = u"scale";
                    static constexpr std::u16string_view aStrRotate = u"rotate";
                    static constexpr std::u16string_view aStrSkewX = u"skewX";
                    static constexpr std::u16string_view aStrSkewY = u"skewY";

                    switch(aChar)
                    {
                        case u'm' :
                        {
                            if(o3tl::matchIgnoreAsciiCase(rCandidate, aStrMatrix, nPos))
                            {
                                // matrix element
                                nPos += aStrMatrix.size();
                                skip_char(rCandidate, ' ', '(', nPos, nLen);
                                SvgNumber aVal;
                                basegfx::B2DHomMatrix aNew;

                                if(readNumberAndUnit(rCandidate, nPos, aVal, nLen))
                                {
                                    aNew.set(0, 0, aVal.solve(rInfoProvider)); // Element A
                                    skip_char(rCandidate, ' ', ',', nPos, nLen);

                                    if(readNumberAndUnit(rCandidate, nPos, aVal, nLen))
                                    {
                                        aNew.set(1, 0, aVal.solve(rInfoProvider)); // Element B
                                        skip_char(rCandidate, ' ', ',', nPos, nLen);

                                        if(readNumberAndUnit(rCandidate, nPos, aVal, nLen))
                                        {
                                            aNew.set(0, 1, aVal.solve(rInfoProvider)); // Element C
                                            skip_char(rCandidate, ' ', ',', nPos, nLen);

                                            if(readNumberAndUnit(rCandidate, nPos, aVal, nLen))
                                            {
                                                aNew.set(1, 1, aVal.solve(rInfoProvider)); // Element D
                                                skip_char(rCandidate, ' ', ',', nPos, nLen);

                                                if(readNumberAndUnit(rCandidate, nPos, aVal, nLen))
                                                {
                                                    aNew.set(0, 2, aVal.solve(rInfoProvider, NumberType::xcoordinate)); // Element E
                                                    skip_char(rCandidate, ' ', ',', nPos, nLen);

                                                    if(readNumberAndUnit(rCandidate, nPos, aVal, nLen))
                                                    {
                                                        aNew.set(1, 2, aVal.solve(rInfoProvider, NumberType::ycoordinate)); // Element F
                                                        skip_char(rCandidate, ' ', ')', nPos, nLen);
                                                        skip_char(rCandidate, ' ', ',', nPos, nLen);

                                                        // caution: String is evaluated from left to right, but matrix multiplication
                                                        // in SVG is right to left, so put the new transformation before the current
                                                        // one by multiplicating from the right side
                                                        aMatrix = aMatrix * aNew;
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                            break;
                        }
                        case u't' :
                        {
                            if(o3tl::matchIgnoreAsciiCase(rCandidate, aStrTranslate, nPos))
                            {
                                // translate element
                                nPos += aStrTranslate.size();
                                skip_char(rCandidate, ' ', '(', nPos, nLen);
                                SvgNumber aTransX;

                                if(readNumberAndUnit(rCandidate, nPos, aTransX, nLen))
                                {
                                    skip_char(rCandidate, ' ', ',', nPos, nLen);
                                    SvgNumber aTransY;
                                    readNumberAndUnit(rCandidate, nPos, aTransY, nLen);
                                    skip_char(rCandidate, ' ', ')', nPos, nLen);
                                    skip_char(rCandidate, ' ', ',', nPos, nLen);

                                    aMatrix = aMatrix * basegfx::utils::createTranslateB2DHomMatrix(
                                        aTransX.solve(rInfoProvider, NumberType::xcoordinate),
                                        aTransY.solve(rInfoProvider, NumberType::ycoordinate));
                                }
                            }
                            break;
                        }
                        case u's' :
                        {
                            if(o3tl::matchIgnoreAsciiCase(rCandidate, aStrScale, nPos))
                            {
                                // scale element
                                nPos += aStrScale.size();
                                skip_char(rCandidate, ' ', '(', nPos, nLen);
                                SvgNumber aScaleX;

                                if(readNumberAndUnit(rCandidate, nPos, aScaleX, nLen))
                                {
                                    skip_char(rCandidate, ' ', ',', nPos, nLen);
                                    SvgNumber aScaleY(aScaleX);
                                    readNumberAndUnit(rCandidate, nPos, aScaleY, nLen);
                                    skip_char(rCandidate, ' ', ')', nPos, nLen);
                                    skip_char(rCandidate, ' ', ',', nPos, nLen);

                                    aMatrix = aMatrix * basegfx::utils::createScaleB2DHomMatrix(
                                        aScaleX.solve(rInfoProvider),
                                        aScaleY.solve(rInfoProvider));
                                }
                            }
                            else if(o3tl::matchIgnoreAsciiCase(rCandidate, aStrSkewX, nPos))
                            {
                                // skewx element
                                nPos += aStrSkewX.size();
                                skip_char(rCandidate, ' ', '(', nPos, nLen);
                                double fSkewX(0.0);

                                if(readAngle(rCandidate, nPos, fSkewX, nLen))
                                {
                                    skip_char(rCandidate, ' ', ')', nPos, nLen);
                                    skip_char(rCandidate, ' ', ',', nPos, nLen);

                                    aMatrix = aMatrix * basegfx::utils::createShearXB2DHomMatrix(tan(fSkewX));
                                }
                            }
                            else if(o3tl::matchIgnoreAsciiCase(rCandidate, aStrSkewY, nPos))
                            {
                                // skewy element
                                nPos += aStrSkewY.size();
                                skip_char(rCandidate, ' ', '(', nPos, nLen);
                                double fSkewY(0.0);

                                if(readAngle(rCandidate, nPos, fSkewY, nLen))
                                {
                                    skip_char(rCandidate, ' ', ')', nPos, nLen);
                                    skip_char(rCandidate, ' ', ',', nPos, nLen);

                                    aMatrix = aMatrix * basegfx::utils::createShearYB2DHomMatrix(tan(fSkewY));
                                }
                            }
                            break;
                        }
                        case u'r' :
                        {
                            if(o3tl::matchIgnoreAsciiCase(rCandidate, aStrRotate, nPos))
                            {
                                // rotate element
                                nPos += aStrRotate.size();
                                skip_char(rCandidate, ' ', '(', nPos, nLen);
                                double fAngle(0.0);

                                if(readAngle(rCandidate, nPos, fAngle, nLen))
                                {
                                    skip_char(rCandidate, ' ', ',', nPos, nLen);
                                    SvgNumber aX;
                                    readNumberAndUnit(rCandidate, nPos, aX, nLen);
                                    skip_char(rCandidate, ' ', ',', nPos, nLen);
                                    SvgNumber aY;
                                    readNumberAndUnit(rCandidate, nPos, aY, nLen);
                                    skip_char(rCandidate, ' ', ')', nPos, nLen);
                                    skip_char(rCandidate, ' ', ',', nPos, nLen);

                                    const double fX(aX.isSet() ? aX.solve(rInfoProvider, NumberType::xcoordinate) : 0.0);
                                    const double fY(aY.isSet() ? aY.solve(rInfoProvider, NumberType::ycoordinate) : 0.0);

                                    if(!basegfx::fTools::equalZero(fX) || !basegfx::fTools::equalZero(fY))
                                    {
                                        // rotate around point
                                        aMatrix = aMatrix * basegfx::utils::createRotateAroundPoint(fX, fY, fAngle);
                                    }
                                    else
                                    {
                                        // rotate
                                        aMatrix = aMatrix * basegfx::utils::createRotateB2DHomMatrix(fAngle);
                                    }
                                }
                            }
                            break;
                        }
                    }

                    if(nInitPos == nPos)
                    {
                        SAL_WARN("svgio", "Could not interpret on current position (!)");
                        nPos++;
                    }
                }
            }

            return aMatrix;
        }

        bool readSingleNumber(std::u16string_view rCandidate, SvgNumber& aNum)
        {
            const sal_Int32 nLen(rCandidate.size());
            sal_Int32 nPos(0);

            return readNumberAndUnit(rCandidate, nPos, aNum, nLen);
        }

        bool readLocalLink(std::u16string_view rCandidate, OUString& rURL)
        {
            sal_Int32 nPos(0);
            const sal_Int32 nLen(rCandidate.size());

            skip_char(rCandidate, ' ', nPos, nLen);

            if (nLen && nPos < nLen && '#' == rCandidate[nPos])
            {
                ++nPos;
                rURL = rCandidate.substr(nPos);

                return true;
            }

            return false;
        }

        bool readLocalUrl(const OUString& rCandidate, OUString& rURL)
        {
            static const char aStrUrl[] = "url(";

            if(rCandidate.startsWithIgnoreAsciiCase(aStrUrl))
            {
                const sal_Int32 nLen(rCandidate.getLength());
                sal_Int32 nPos(strlen(aStrUrl));
                sal_Unicode aLimiter(')');

                skip_char(rCandidate, ' ', nPos, nLen);

                if('"' == rCandidate[nPos])
                {
                    aLimiter = '"';
                    ++nPos;
                }
                else if('\'' == rCandidate[nPos])
                {
                    aLimiter = '\'';
                    ++nPos;
                }

                skip_char(rCandidate, ' ', nPos, nLen);
                skip_char(rCandidate, '#', nPos, nPos + 1);
                OUStringBuffer aTokenValue;

                copyToLimiter(rCandidate, aLimiter, nPos, aTokenValue, nLen);

                rURL = aTokenValue.makeStringAndClear();

                return true;
            }

            return false;
        }

        bool readSvgPaint(const OUString& rCandidate, SvgPaint& rSvgPaint,
            OUString& rURL, SvgNumber& rOpacity)
        {
            if( !rCandidate.isEmpty() )
            {
                basegfx::BColor aColor;

                if(read_color(rCandidate, aColor, rOpacity))
                {
                    rSvgPaint = SvgPaint(aColor, true, true);
                    return true;
                }
                else
                {
                    if(o3tl::equalsIgnoreAsciiCase(o3tl::trim(rCandidate), u"none"))
                    {
                        rSvgPaint = SvgPaint(aColor, true, false, false);
                        return true;
                    }
                    else if(readLocalUrl(rCandidate, rURL))
                    {
                        /// Url is copied to rURL, but needs to be solved outside this helper
                        return false;
                    }
                    else if(o3tl::equalsIgnoreAsciiCase(o3tl::trim(rCandidate), u"currentColor"))
                    {
                        rSvgPaint = SvgPaint(aColor, true, true, true);
                        return true;
                    }
                }
            }

            return false;
        }

        bool readSvgNumberVector(std::u16string_view rCandidate, SvgNumberVector& rSvgNumberVector)
        {
            const sal_Int32 nLen(rCandidate.size());
            rSvgNumberVector.clear();

            if(nLen)
            {
                sal_Int32 nPos(0);
                SvgNumber aNum;
                skip_char(rCandidate, ' ', ',', nPos, nLen);

                while(readNumberAndUnit(rCandidate, nPos, aNum, nLen))
                {
                    rSvgNumberVector.push_back(aNum);
                    skip_char(rCandidate, ' ', ',', nPos, nLen);
                }

                return !rSvgNumberVector.empty();
            }

            return false;
        }

        SvgAspectRatio readSvgAspectRatio(std::u16string_view rCandidate)
        {
            const sal_Int32 nLen(rCandidate.size());

            if(nLen)
            {
                sal_Int32 nPos(0);
                SvgAlign aSvgAlign(SvgAlign::xMidYMid);
                bool bMeetOrSlice(true);
                bool bChanged(false);

                while(nPos < nLen)
                {
                    const sal_Int32 nInitPos(nPos);
                    skip_char(rCandidate, ' ', nPos, nLen);
                    OUStringBuffer aTokenName;
                    copyString(rCandidate, nPos, aTokenName, nLen);

                    if(!aTokenName.isEmpty())
                    {
                        switch(StrToSVGToken(aTokenName.makeStringAndClear(), false))
                        {
                            case SVGToken::Defer:
                            {
                                bChanged = true;
                                break;
                            }
                            case SVGToken::None:
                            {
                                aSvgAlign = SvgAlign::none;
                                bChanged = true;
                                break;
                            }
                            case SVGToken::XMinYMin:
                            {
                                aSvgAlign = SvgAlign::xMinYMin;
                                bChanged = true;
                                break;
                            }
                            case SVGToken::XMidYMin:
                            {
                                aSvgAlign = SvgAlign::xMidYMin;
                                bChanged = true;
                                break;
                            }
                            case SVGToken::XMaxYMin:
                            {
                                aSvgAlign = SvgAlign::xMaxYMin;
                                bChanged = true;
                                break;
                            }
                            case SVGToken::XMinYMid:
                            {
                                aSvgAlign = SvgAlign::xMinYMid;
                                bChanged = true;
                                break;
                            }
                            case SVGToken::XMidYMid:
                            {
                                aSvgAlign = SvgAlign::xMidYMid;
                                bChanged = true;
                                break;
                            }
                            case SVGToken::XMaxYMid:
                            {
                                aSvgAlign = SvgAlign::xMaxYMid;
                                bChanged = true;
                                break;
                            }
                            case SVGToken::XMinYMax:
                            {
                                aSvgAlign = SvgAlign::xMinYMax;
                                bChanged = true;
                                break;
                            }
                            case SVGToken::XMidYMax:
                            {
                                aSvgAlign = SvgAlign::xMidYMax;
                                bChanged = true;
                                break;
                            }
                            case SVGToken::XMaxYMax:
                            {
                                aSvgAlign = SvgAlign::xMaxYMax;
                                bChanged = true;
                                break;
                            }
                            case SVGToken::Meet:
                            {
                                bMeetOrSlice = true;
                                bChanged = true;
                                break;
                            }
                            case SVGToken::Slice:
                            {
                                bMeetOrSlice = false;
                                bChanged = true;
                                break;
                            }
                            default:
                            {
                                break;
                            }
                        }
                    }

                    if(nInitPos == nPos)
                    {
                        SAL_WARN("svgio", "Could not interpret on current position (!)");
                        nPos++;
                    }
                }

                if(bChanged)
                {
                    return SvgAspectRatio(aSvgAlign, bMeetOrSlice);
                }
            }

            return SvgAspectRatio();
        }

        bool readSvgStringVector(std::u16string_view rCandidate, SvgStringVector& rSvgStringVector, sal_Unicode nSeparator)
        {
            rSvgStringVector.clear();
            const sal_Int32 nLen(rCandidate.size());

            if(nLen)
            {
                sal_Int32 nPos(0);
                OUStringBuffer aTokenValue;
                skip_char(rCandidate, ' ', ',', nPos, nLen);

                while(nPos < nLen)
                {
                    copyToLimiter(rCandidate, nSeparator, nPos, aTokenValue, nLen);
                    skip_char(rCandidate, nSeparator, nPos, nLen);
                    const OUString aString = aTokenValue.makeStringAndClear();

                    if(!aString.isEmpty())
                    {
                        rSvgStringVector.push_back(aString);
                    }
                }
            }

            return !rSvgStringVector.empty();
        }

        void readImageLink(const OUString& rCandidate, OUString& rXLink, OUString& rUrl, OUString& rData)
        {
            rXLink.clear();
            rUrl.clear();
            rData.clear();

            if(!readLocalLink(rCandidate, rXLink))
            {
                static const char aStrData[] = "data:";

                if(rCandidate.matchIgnoreAsciiCase(aStrData, 0))
                {
                    // embedded data
                    sal_Int32 nPos(strlen(aStrData));
                    sal_Int32 nLen(rCandidate.getLength());
                    OUStringBuffer aBuffer;

                    // read mime type
                    skip_char(rCandidate, ' ', nPos, nLen);
                    copyToLimiter(rCandidate, ';', nPos, aBuffer, nLen);
                    skip_char(rCandidate, ' ', ';', nPos, nLen);
                    const OUString aMimeType = aBuffer.makeStringAndClear();

                    if(!aMimeType.isEmpty() && nPos < nLen)
                    {
                        if(aMimeType.startsWith("image"))
                        {
                            // image data
                            std::u16string_view aData(rCandidate.subView(nPos));
                            static constexpr std::u16string_view aStrBase64 = u"base64";

                            if(o3tl::starts_with(aData, aStrBase64))
                            {
                                // base64 encoded
                                nPos = aStrBase64.size();
                                nLen = aData.size();

                                skip_char(aData, ' ', ',', nPos, nLen);

                                if(nPos < nLen)
                                {
                                    rData = aData.substr(nPos);
                                }
                            }
                        }
                    }
                }
                else
                {
                    // Url (path and filename)
                    rUrl = rCandidate;
                }
            }
        }

        // #i125325#
        OUString removeBlockComments(const OUString& rCandidate)
        {
            const sal_Int32 nLen(rCandidate.getLength());

            if(nLen)
            {
                sal_Int32 nPos(0);
                OUStringBuffer aBuffer;
                bool bChanged(false);
                sal_Int32 nInsideComment(0);
                const sal_Unicode aCommentSlash('/');
                const sal_Unicode aCommentStar('*');

                while(nPos < nLen)
                {
                    const sal_Unicode aChar(rCandidate[nPos]);
                    const bool bStart(aCommentSlash == aChar && nPos + 1 < nLen && aCommentStar == rCandidate[nPos + 1]);
                    const bool bEnd(aCommentStar == aChar && nPos + 1 < nLen && aCommentSlash == rCandidate[nPos + 1]);

                    if(bStart)
                    {
                        nPos += 2;
                        nInsideComment++;
                        bChanged = true;
                    }
                    else if(bEnd)
                    {
                        nPos += 2;
                        nInsideComment--;
                    }
                    else
                    {
                        if(!nInsideComment)
                        {
                            aBuffer.append(aChar);
                        }

                        nPos++;
                    }
                }

                if(bChanged)
                {
                    return aBuffer.makeStringAndClear();
                }
            }

            return rCandidate;
        }

        OUString consolidateContiguousSpace(const OUString& rCandidate)
        {
            const sal_Int32 nLen(rCandidate.getLength());

            if(nLen)
            {
                sal_Int32 nPos(0);
                OUStringBuffer aBuffer;
                bool bInsideSpace(false);
                const sal_Unicode aSpace(' ');

                while(nPos < nLen)
                {
                    const sal_Unicode aChar(rCandidate[nPos]);

                    if(aSpace == aChar)
                    {
                        bInsideSpace = true;
                    }
                    else
                    {
                        if(bInsideSpace)
                        {
                            bInsideSpace = false;
                            aBuffer.append(aSpace);
                        }

                        aBuffer.append(aChar);
                    }

                    nPos++;
                }

                if(bInsideSpace)
                {
                    aBuffer.append(aSpace);
                }

                if(aBuffer.getLength() != nLen)
                {
                    return aBuffer.makeStringAndClear();
                }
            }

            return rCandidate;
        }

        ::std::vector< double > solveSvgNumberVector(const SvgNumberVector& rInput, const InfoProvider& rInfoProvider)
        {
            ::std::vector< double > aRetval;

            if(!rInput.empty())
            {
                const double nCount(rInput.size());
                aRetval.reserve(nCount);

                for(sal_uInt32 a(0); a < nCount; a++)
                {
                    aRetval.push_back(rInput[a].solve(rInfoProvider));
                }
            }

            return aRetval;
        }

} // end of namespace svgio::svgreader

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
