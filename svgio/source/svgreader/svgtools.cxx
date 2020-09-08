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
#include <osl/diagnose.h>
#include <tools/color.hxx>
#include <rtl/math.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <svgtoken.hxx>
#include <unordered_map>

namespace svgio::svgreader
{
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
            if(Align_none == getSvgAlign())
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
                case Align_xMidYMin:
                case Align_xMidYMid:
                case Align_xMidYMax:
                {
                    // centerX
                    const double fFreeSpace(rTarget.getWidth() - fNewWidth);
                    fTransX = fFreeSpace * 0.5;
                    break;
                }
                case Align_xMaxYMin:
                case Align_xMaxYMid:
                case Align_xMaxYMax:
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
                case Align_xMinYMid:
                case Align_xMidYMid:
                case Align_xMaxYMid:
                {
                    // centerY
                    const double fFreeSpace(rTarget.getHeight() - fNewHeight);
                    fTransY = fFreeSpace * 0.5;
                    break;
                }
                case Align_xMinYMax:
                case Align_xMidYMax:
                case Align_xMaxYMax:
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

        double SvgNumber::solveNonPercentage(const InfoProvider& rInfoProvider) const
        {
            if(isSet())
            {
                switch(meUnit)
                {
                    case Unit_em:
                    {
                        return mfNumber * rInfoProvider.getCurrentFontSizeInherited();
                    }
                    case Unit_ex:
                    {
                        return mfNumber * rInfoProvider.getCurrentXHeightInherited() * 0.5;
                    }
                    case Unit_px:
                    {
                        return mfNumber;
                    }
                    case Unit_pt:
                    case Unit_pc:
                    case Unit_cm:
                    case Unit_mm:
                    case Unit_in:
                    {
                        double fRetval(mfNumber);

                        switch(meUnit)
                        {
                            case Unit_pt: fRetval *= F_SVG_PIXEL_PER_INCH / 72.0; break;
                            case Unit_pc: fRetval *= F_SVG_PIXEL_PER_INCH / 6.0; break;
                            case Unit_cm: fRetval *= F_SVG_PIXEL_PER_INCH / 2.54; break;
                            case Unit_mm: fRetval *= 0.1 * F_SVG_PIXEL_PER_INCH / 2.54; break;
                            case Unit_in: fRetval *= F_SVG_PIXEL_PER_INCH; break;
                            default: break;
                        }

                        return fRetval;
                    }
                    case Unit_none:
                    {
                        SAL_WARN("svgio", "Design error, this case should have been handled in the caller");
                        return mfNumber;
                    }
                    default:
                    {
                        OSL_ENSURE(false, "Do not use with percentage! ");
                        return 0.0;
                    }
                }
            }

            /// not set
            OSL_ENSURE(false, "SvgNumber not set (!)");
            return 0.0;
        }

        double SvgNumber::solve(const InfoProvider& rInfoProvider, NumberType aNumberType) const
        {
            if(isSet())
            {
                switch(meUnit)
                {
                    case Unit_px:
                    {
                        return mfNumber;
                    }
                    case Unit_pt:
                    case Unit_pc:
                    case Unit_cm:
                    case Unit_mm:
                    case Unit_in:
                    case Unit_em:
                    case Unit_ex:
                    case Unit_none:
                    {
                        return solveNonPercentage( rInfoProvider);
                    }
                    case Unit_percent:
                    {
                        double fRetval(mfNumber * 0.01);
                        basegfx::B2DRange aViewPort = rInfoProvider.getCurrentViewPort();

                        if ( aViewPort.isEmpty() )
                        {
                            SAL_WARN("svgio", "Design error, this case should have been handled in the caller");
                            // no viewPort, assume a normal page size (A4)
                            aViewPort = basegfx::B2DRange(
                                0.0,
                                0.0,
                                210.0 * F_SVG_PIXEL_PER_INCH / 2.54,
                                297.0 * F_SVG_PIXEL_PER_INCH / 2.54);

                        }

                        if ( !aViewPort.isEmpty() )
                        {
                            if(xcoordinate == aNumberType)
                            {
                                // it's a x-coordinate, relative to current width (w)
                                fRetval *= aViewPort.getWidth();
                            }
                            else if(ycoordinate == aNumberType)
                            {
                                // it's a y-coordinate, relative to current height (h)
                                fRetval *= aViewPort.getHeight();
                            }
                            else // length
                            {
                                // it's a length, relative to sqrt(w*w + h*h)/sqrt(2)
                                const double fCurrentWidth(aViewPort.getWidth());
                                const double fCurrentHeight(aViewPort.getHeight());
                                const double fCurrentLength(
                                    sqrt(fCurrentWidth * fCurrentWidth + fCurrentHeight * fCurrentHeight)/sqrt(2.0));

                                fRetval *= fCurrentLength;
                            }
                        }

                        return fRetval;
                    }
                    default:
                    {
                        break;
                    }
                }
            }

            /// not set
            OSL_ENSURE(false, "SvgNumber not set (!)");
            return 0.0;
        }

        bool SvgNumber::isPositive() const
        {
            return basegfx::fTools::moreOrEqual(mfNumber, 0.0);
        }

        void skip_char(const OUString& rCandidate, sal_Unicode nChar, sal_Int32& nPos, const sal_Int32 nLen)
        {
            while(nPos < nLen && nChar == rCandidate[nPos])
            {
                nPos++;
            }
        }

        void skip_char(const OUString& rCandidate, sal_Unicode nCharA, sal_Unicode nCharB, sal_Int32& nPos, const sal_Int32 nLen)
        {
            while(nPos < nLen && (nCharA == rCandidate[nPos] || nCharB == rCandidate[nPos]))
            {
                nPos++;
            }
        }

        void copySign(const OUString& rCandidate, sal_Int32& nPos, OUStringBuffer& rTarget, const sal_Int32 nLen)
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

        void copyNumber(const OUString& rCandidate, sal_Int32& nPos, OUStringBuffer& rTarget, const sal_Int32 nLen)
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

        void copyHex(const OUString& rCandidate, sal_Int32& nPos, OUStringBuffer& rTarget, const sal_Int32 nLen)
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

        void copyString(const OUString& rCandidate, sal_Int32& nPos, OUStringBuffer& rTarget, const sal_Int32 nLen)
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

        void copyToLimiter(const OUString& rCandidate, sal_Unicode nLimiter, sal_Int32& nPos, OUStringBuffer& rTarget, const sal_Int32 nLen)
        {
            while(nPos < nLen && nLimiter != rCandidate[nPos])
            {
                rTarget.append(rCandidate[nPos]);
                nPos++;
            }
        }

        bool readNumber(const OUString& rCandidate, sal_Int32& nPos, double& fNum, const sal_Int32 nLen)
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
                        aNum.makeStringAndClear(), '.', ',',
                        &eStatus);

                    return eStatus == rtl_math_ConversionStatus_Ok;
                }
            }

            return false;
        }

        SvgUnit readUnit(const OUString& rCandidate, sal_Int32& nPos, const sal_Int32 nLen)
        {
            SvgUnit aRetval(Unit_px);

            if(nPos < nLen)
            {
                const sal_Unicode aCharA(rCandidate[nPos]);

                if(nPos + 1 < nLen)
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
                                aRetval = Unit_em;
                                bTwoCharValid = true;
                            }
                            else if('x' == aCharB)
                            {
                                // 'ex' Relative to current font x-height
                                aRetval = Unit_ex;
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
                                aRetval = Unit_pt;
                                bTwoCharValid = true;
                            }
                            else if('c' == aCharB)
                            {
                                // 'pc' == 16 px
                                aRetval = Unit_pc;
                                bTwoCharValid = true;
                            }
                            break;
                        }
                        case u'i' :
                        {
                            if('n' == aCharB)
                            {
                                // 'in' == 96 px, since CSS 2.1
                                aRetval = Unit_in;
                                bTwoCharValid = true;
                            }
                            break;
                        }
                        case u'c' :
                        {
                            if('m' == aCharB)
                            {
                                // 'cm' == 37.79527559 px
                                aRetval = Unit_cm;
                                bTwoCharValid = true;
                            }
                            break;
                        }
                        case u'm' :
                        {
                            if('m' == aCharB)
                            {
                                // 'mm' == 3.779528 px
                                aRetval = Unit_mm;
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
                else
                {
                    if('%' == aCharA)
                    {
                        // percent used, relative to current
                        nPos++;
                        aRetval = Unit_percent;
                    }
                }
            }

            return aRetval;
        }

        bool readNumberAndUnit(const OUString& rCandidate, sal_Int32& nPos, SvgNumber& aNum, const sal_Int32 nLen)
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

        bool readAngle(const OUString& rCandidate, sal_Int32& nPos, double& fAngle, const sal_Int32 nLen)
        {
            if(readNumber(rCandidate, nPos, fAngle, nLen))
            {
                skip_char(rCandidate, ' ', nPos, nLen);

                enum DegreeType
                {
                    deg,
                    grad,
                    rad
                } aType(deg); // degrees is default

                if(nPos < nLen)
                {
                    const sal_Unicode aChar(rCandidate[nPos]);
                    static const char aStrGrad[] = "grad";
                    static const char aStrRad[] = "rad";

                    switch(aChar)
                    {
                        case u'g' :
                        case u'G' :
                        {
                            if(rCandidate.matchIgnoreAsciiCase(aStrGrad, nPos))
                            {
                                // angle in grad
                                nPos += strlen(aStrGrad);
                                aType = grad;
                            }
                            break;
                        }
                        case u'r' :
                        case u'R' :
                        {
                            if(rCandidate.matchIgnoreAsciiCase(aStrRad, nPos))
                            {
                                // angle in radians
                                nPos += strlen(aStrRad);
                                aType = rad;
                            }
                            break;
                        }
                    }
                }

                // convert to radians
                if(deg == aType)
                {
                    fAngle = basegfx::deg2rad(fAngle);
                }
                else if(grad == aType)
                {
                    // looks like 100 grad is 90 degrees
                    fAngle *= F_PI / 200.0;
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

        bool match_colorKeyword(basegfx::BColor& rColor, const OUString& rName, bool bCaseIndependent)
        {
            typedef std::unordered_map< OUString, Color > ColorTokenMapper;
            typedef std::pair< OUString, Color > ColorTokenValueType;
            static const ColorTokenMapper aColorTokenMapperList {
                { ColorTokenValueType(OUString("aliceblue"), Color(240, 248, 255)) },
                { ColorTokenValueType(OUString("antiquewhite"), Color(250, 235, 215) ) },
                { ColorTokenValueType(OUString("aqua"), Color( 0, 255, 255) ) },
                { ColorTokenValueType(OUString("aquamarine"), Color(127, 255, 212) ) },
                { ColorTokenValueType(OUString("azure"), Color(240, 255, 255) ) },
                { ColorTokenValueType(OUString("beige"), Color(245, 245, 220) ) },
                { ColorTokenValueType(OUString("bisque"), Color(255, 228, 196) ) },
                { ColorTokenValueType(OUString("black"), Color( 0, 0, 0) ) },
                { ColorTokenValueType(OUString("blanchedalmond"), Color(255, 235, 205) ) },
                { ColorTokenValueType(OUString("blue"), Color( 0, 0, 255) ) },
                { ColorTokenValueType(OUString("blueviolet"), Color(138, 43, 226) ) },
                { ColorTokenValueType(OUString("brown"), Color(165, 42, 42) ) },
                { ColorTokenValueType(OUString("burlywood"), Color(222, 184, 135) ) },
                { ColorTokenValueType(OUString("cadetblue"), Color( 95, 158, 160) ) },
                { ColorTokenValueType(OUString("chartreuse"), Color(127, 255, 0) ) },
                { ColorTokenValueType(OUString("chocolate"), Color(210, 105, 30) ) },
                { ColorTokenValueType(OUString("coral"), Color(255, 127, 80) ) },
                { ColorTokenValueType(OUString("cornflowerblue"), Color(100, 149, 237) ) },
                { ColorTokenValueType(OUString("cornsilk"), Color(255, 248, 220) ) },
                { ColorTokenValueType(OUString("crimson"), Color(220, 20, 60) ) },
                { ColorTokenValueType(OUString("cyan"), Color( 0, 255, 255) ) },
                { ColorTokenValueType(OUString("darkblue"), Color( 0, 0, 139) ) },
                { ColorTokenValueType(OUString("darkcyan"), Color( 0, 139, 139) ) },
                { ColorTokenValueType(OUString("darkgoldenrod"), Color(184, 134, 11) ) },
                { ColorTokenValueType(OUString("darkgray"), Color(169, 169, 169) ) },
                { ColorTokenValueType(OUString("darkgreen"), Color( 0, 100, 0) ) },
                { ColorTokenValueType(OUString("darkgrey"), Color(169, 169, 169) ) },
                { ColorTokenValueType(OUString("darkkhaki"), Color(189, 183, 107) ) },
                { ColorTokenValueType(OUString("darkmagenta"), Color(139, 0, 139) ) },
                { ColorTokenValueType(OUString("darkolivegreen"), Color( 85, 107, 47) ) },
                { ColorTokenValueType(OUString("darkorange"), Color(255, 140, 0) ) },
                { ColorTokenValueType(OUString("darkorchid"), Color(153, 50, 204) ) },
                { ColorTokenValueType(OUString("darkred"), Color(139, 0, 0) ) },
                { ColorTokenValueType(OUString("darksalmon"), Color(233, 150, 122) ) },
                { ColorTokenValueType(OUString("darkseagreen"), Color(143, 188, 143) ) },
                { ColorTokenValueType(OUString("darkslateblue"), Color( 72, 61, 139) ) },
                { ColorTokenValueType(OUString("darkslategray"), Color( 47, 79, 79) ) },
                { ColorTokenValueType(OUString("darkslategrey"), Color( 47, 79, 79) ) },
                { ColorTokenValueType(OUString("darkturquoise"), Color( 0, 206, 209) ) },
                { ColorTokenValueType(OUString("darkviolet"), Color(148, 0, 211) ) },
                { ColorTokenValueType(OUString("deeppink"), Color(255, 20, 147) ) },
                { ColorTokenValueType(OUString("deepskyblue"), Color( 0, 191, 255) ) },
                { ColorTokenValueType(OUString("dimgray"), Color(105, 105, 105) ) },
                { ColorTokenValueType(OUString("dimgrey"), Color(105, 105, 105) ) },
                { ColorTokenValueType(OUString("dodgerblue"), Color( 30, 144, 255) ) },
                { ColorTokenValueType(OUString("firebrick"), Color(178, 34, 34) ) },
                { ColorTokenValueType(OUString("floralwhite"), Color(255, 250, 240) ) },
                { ColorTokenValueType(OUString("forestgreen"), Color( 34, 139, 34) ) },
                { ColorTokenValueType(OUString("fuchsia"), Color(255, 0, 255) ) },
                { ColorTokenValueType(OUString("gainsboro"), Color(220, 220, 220) ) },
                { ColorTokenValueType(OUString("ghostwhite"), Color(248, 248, 255) ) },
                { ColorTokenValueType(OUString("gold"), Color(255, 215, 0) ) },
                { ColorTokenValueType(OUString("goldenrod"), Color(218, 165, 32) ) },
                { ColorTokenValueType(OUString("gray"), Color(128, 128, 128) ) },
                { ColorTokenValueType(OUString("grey"), Color(128, 128, 128) ) },
                { ColorTokenValueType(OUString("green"), Color(0, 128, 0) ) },
                { ColorTokenValueType(OUString("greenyellow"), Color(173, 255, 47) ) },
                { ColorTokenValueType(OUString("honeydew"), Color(240, 255, 240) ) },
                { ColorTokenValueType(OUString("hotpink"), Color(255, 105, 180) ) },
                { ColorTokenValueType(OUString("indianred"), Color(205, 92, 92) ) },
                { ColorTokenValueType(OUString("indigo"), Color( 75, 0, 130) ) },
                { ColorTokenValueType(OUString("ivory"), Color(255, 255, 240) ) },
                { ColorTokenValueType(OUString("khaki"), Color(240, 230, 140) ) },
                { ColorTokenValueType(OUString("lavender"), Color(230, 230, 250) ) },
                { ColorTokenValueType(OUString("lavenderblush"), Color(255, 240, 245) ) },
                { ColorTokenValueType(OUString("lawngreen"), Color(124, 252, 0) ) },
                { ColorTokenValueType(OUString("lemonchiffon"), Color(255, 250, 205) ) },
                { ColorTokenValueType(OUString("lightblue"), Color(173, 216, 230) ) },
                { ColorTokenValueType(OUString("lightcoral"), Color(240, 128, 128) ) },
                { ColorTokenValueType(OUString("lightcyan"), Color(224, 255, 255) ) },
                { ColorTokenValueType(OUString("lightgoldenrodyellow"), Color(250, 250, 210) ) },
                { ColorTokenValueType(OUString("lightgray"), Color(211, 211, 211) ) },
                { ColorTokenValueType(OUString("lightgreen"), Color(144, 238, 144) ) },
                { ColorTokenValueType(OUString("lightgrey"), Color(211, 211, 211) ) },
                { ColorTokenValueType(OUString("lightpink"), Color(255, 182, 193) ) },
                { ColorTokenValueType(OUString("lightsalmon"), Color(255, 160, 122) ) },
                { ColorTokenValueType(OUString("lightseagreen"), Color( 32, 178, 170) ) },
                { ColorTokenValueType(OUString("lightskyblue"), Color(135, 206, 250) ) },
                { ColorTokenValueType(OUString("lightslategray"), Color(119, 136, 153) ) },
                { ColorTokenValueType(OUString("lightslategrey"), Color(119, 136, 153) ) },
                { ColorTokenValueType(OUString("lightsteelblue"), Color(176, 196, 222) ) },
                { ColorTokenValueType(OUString("lightyellow"), Color(255, 255, 224) ) },
                { ColorTokenValueType(OUString("lime"), Color( 0, 255, 0) ) },
                { ColorTokenValueType(OUString("limegreen"), Color( 50, 205, 50) ) },
                { ColorTokenValueType(OUString("linen"), Color(250, 240, 230) ) },
                { ColorTokenValueType(OUString("magenta"), Color(255, 0, 255) ) },
                { ColorTokenValueType(OUString("maroon"), Color(128, 0, 0) ) },
                { ColorTokenValueType(OUString("mediumaquamarine"), Color(102, 205, 170) ) },
                { ColorTokenValueType(OUString("mediumblue"), Color( 0, 0, 205) ) },
                { ColorTokenValueType(OUString("mediumorchid"), Color(186, 85, 211) ) },
                { ColorTokenValueType(OUString("mediumpurple"), Color(147, 112, 219) ) },
                { ColorTokenValueType(OUString("mediumseagreen"), Color( 60, 179, 113) ) },
                { ColorTokenValueType(OUString("mediumslateblue"), Color(123, 104, 238) ) },
                { ColorTokenValueType(OUString("mediumspringgreen"), Color( 0, 250, 154) ) },
                { ColorTokenValueType(OUString("mediumturquoise"), Color( 72, 209, 204) ) },
                { ColorTokenValueType(OUString("mediumvioletred"), Color(199, 21, 133) ) },
                { ColorTokenValueType(OUString("midnightblue"), Color( 25, 25, 112) ) },
                { ColorTokenValueType(OUString("mintcream"), Color(245, 255, 250) ) },
                { ColorTokenValueType(OUString("mistyrose"), Color(255, 228, 225) ) },
                { ColorTokenValueType(OUString("moccasin"), Color(255, 228, 181) ) },
                { ColorTokenValueType(OUString("navajowhite"), Color(255, 222, 173) ) },
                { ColorTokenValueType(OUString("navy"), Color( 0, 0, 128) ) },
                { ColorTokenValueType(OUString("oldlace"), Color(253, 245, 230) ) },
                { ColorTokenValueType(OUString("olive"), Color(128, 128, 0) ) },
                { ColorTokenValueType(OUString("olivedrab"), Color(107, 142, 35) ) },
                { ColorTokenValueType(OUString("orange"), Color(255, 165, 0) ) },
                { ColorTokenValueType(OUString("orangered"), Color(255, 69, 0) ) },
                { ColorTokenValueType(OUString("orchid"), Color(218, 112, 214) ) },
                { ColorTokenValueType(OUString("palegoldenrod"), Color(238, 232, 170) ) },
                { ColorTokenValueType(OUString("palegreen"), Color(152, 251, 152) ) },
                { ColorTokenValueType(OUString("paleturquoise"), Color(175, 238, 238) ) },
                { ColorTokenValueType(OUString("palevioletred"), Color(219, 112, 147) ) },
                { ColorTokenValueType(OUString("papayawhip"), Color(255, 239, 213) ) },
                { ColorTokenValueType(OUString("peachpuff"), Color(255, 218, 185) ) },
                { ColorTokenValueType(OUString("peru"), Color(205, 133, 63) ) },
                { ColorTokenValueType(OUString("pink"), Color(255, 192, 203) ) },
                { ColorTokenValueType(OUString("plum"), Color(221, 160, 221) ) },
                { ColorTokenValueType(OUString("powderblue"), Color(176, 224, 230) ) },
                { ColorTokenValueType(OUString("purple"), Color(128, 0, 128) ) },
                { ColorTokenValueType(OUString("red"), Color(255, 0, 0) ) },
                { ColorTokenValueType(OUString("rosybrown"), Color(188, 143, 143) ) },
                { ColorTokenValueType(OUString("royalblue"), Color( 65, 105, 225) ) },
                { ColorTokenValueType(OUString("saddlebrown"), Color(139, 69, 19) ) },
                { ColorTokenValueType(OUString("salmon"), Color(250, 128, 114) ) },
                { ColorTokenValueType(OUString("sandybrown"), Color(244, 164, 96) ) },
                { ColorTokenValueType(OUString("seagreen"), Color( 46, 139, 87) ) },
                { ColorTokenValueType(OUString("seashell"), Color(255, 245, 238) ) },
                { ColorTokenValueType(OUString("sienna"), Color(160, 82, 45) ) },
                { ColorTokenValueType(OUString("silver"), Color(192, 192, 192) ) },
                { ColorTokenValueType(OUString("skyblue"), Color(135, 206, 235) ) },
                { ColorTokenValueType(OUString("slateblue"), Color(106, 90, 205) ) },
                { ColorTokenValueType(OUString("slategray"), Color(112, 128, 144) ) },
                { ColorTokenValueType(OUString("slategrey"), Color(112, 128, 144) ) },
                { ColorTokenValueType(OUString("snow"), Color(255, 250, 250) ) },
                { ColorTokenValueType(OUString("springgreen"), Color( 0, 255, 127) ) },
                { ColorTokenValueType(OUString("steelblue"), Color( 70, 130, 180) ) },
                { ColorTokenValueType(OUString("tan"), Color(210, 180, 140) ) },
                { ColorTokenValueType(OUString("teal"), Color( 0, 128, 128) ) },
                { ColorTokenValueType(OUString("thistle"), Color(216, 191, 216) ) },
                { ColorTokenValueType(OUString("tomato"), Color(255, 99, 71) ) },
                { ColorTokenValueType(OUString("turquoise"), Color( 64, 224, 208) ) },
                { ColorTokenValueType(OUString("violet"), Color(238, 130, 238) ) },
                { ColorTokenValueType(OUString("wheat"), Color(245, 222, 179) ) },
                { ColorTokenValueType(OUString("white"), Color(255, 255, 255) ) },
                { ColorTokenValueType(OUString("whitesmoke"), Color(245, 245, 245) ) },
                { ColorTokenValueType(OUString("yellow"), Color(255, 255, 0) ) },
                { ColorTokenValueType(OUString("yellowgreen"), Color(154, 205, 50) ) },
            };

            ColorTokenMapper::const_iterator aResult(aColorTokenMapperList.find(rName));

            if(bCaseIndependent && aResult == aColorTokenMapperList.end())
            {
                // also try case independent match (e.g. for Css styles)
                aResult = aColorTokenMapperList.find(rName.toAsciiLowerCase());
            }

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

        bool read_color(const OUString& rCandidate, basegfx::BColor& rColor, bool bCaseIndependent, SvgNumber& rOpacity)
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
                        if(match_colorKeyword(rColor, rCandidate, bCaseIndependent))
                        {
                            return true;
                        }
                    }
                }
            }

            return false;
        }

        basegfx::B2DRange readViewBox(const OUString& rCandidate, InfoProvider const & rInfoProvider)
        {
            const sal_Int32 nLen(rCandidate.getLength());

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
                                double fX(aMinX.solve(rInfoProvider, xcoordinate));
                                double fY(aMinY.solve(rInfoProvider, ycoordinate));
                                double fW(aWidth.solve(rInfoProvider,xcoordinate));
                                double fH(aHeight.solve(rInfoProvider,ycoordinate));
                                return basegfx::B2DRange(fX,fY,fX+fW,fY+fH);
                            }
                        }
                    }
                }
            }

            return basegfx::B2DRange();
        }

        basegfx::B2DHomMatrix readTransform(const OUString& rCandidate, InfoProvider const & rInfoProvider)
        {
            basegfx::B2DHomMatrix aMatrix;
            const sal_Int32 nLen(rCandidate.getLength());

            if(nLen)
            {
                sal_Int32 nPos(0);
                skip_char(rCandidate, ' ', ',', nPos, nLen);

                while(nPos < nLen)
                {
                    const sal_Unicode aChar(rCandidate[nPos]);
                    const sal_Int32 nInitPos(nPos);
                    static const char aStrMatrix[] = "matrix";
                    static const char aStrTranslate[] = "translate";
                    static const char aStrScale[] = "scale";
                    static const char aStrRotate[] = "rotate";
                    static const char aStrSkewX[] = "skewX";
                    static const char aStrSkewY[] = "skewY";

                    switch(aChar)
                    {
                        case u'm' :
                        {
                            if(rCandidate.match(aStrMatrix, nPos))
                            {
                                // matrix element
                                nPos += strlen(aStrMatrix);
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
                                                    aNew.set(0, 2, aVal.solve(rInfoProvider, xcoordinate)); // Element E
                                                    skip_char(rCandidate, ' ', ',', nPos, nLen);

                                                    if(readNumberAndUnit(rCandidate, nPos, aVal, nLen))
                                                    {
                                                        aNew.set(1, 2, aVal.solve(rInfoProvider, ycoordinate)); // Element F
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
                            if(rCandidate.match(aStrTranslate, nPos))
                            {
                                // translate element
                                nPos += strlen(aStrTranslate);
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
                                        aTransX.solve(rInfoProvider, xcoordinate),
                                        aTransY.solve(rInfoProvider, ycoordinate));
                                }
                            }
                            break;
                        }
                        case u's' :
                        {
                            if(rCandidate.match(aStrScale, nPos))
                            {
                                // scale element
                                nPos += strlen(aStrScale);
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
                            else if(rCandidate.match(aStrSkewX, nPos))
                            {
                                // skewx element
                                nPos += strlen(aStrSkewX);
                                skip_char(rCandidate, ' ', '(', nPos, nLen);
                                double fSkewX(0.0);

                                if(readAngle(rCandidate, nPos, fSkewX, nLen))
                                {
                                    skip_char(rCandidate, ' ', ')', nPos, nLen);
                                    skip_char(rCandidate, ' ', ',', nPos, nLen);

                                    aMatrix = aMatrix * basegfx::utils::createShearXB2DHomMatrix(tan(fSkewX));
                                }
                            }
                            else if(rCandidate.match(aStrSkewY, nPos))
                            {
                                // skewy element
                                nPos += strlen(aStrSkewY);
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
                            if(rCandidate.match(aStrRotate, nPos))
                            {
                                // rotate element
                                nPos += strlen(aStrRotate);
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

                                    const double fX(aX.isSet() ? aX.solve(rInfoProvider, xcoordinate) : 0.0);
                                    const double fY(aY.isSet() ? aY.solve(rInfoProvider, ycoordinate) : 0.0);

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
                        OSL_ENSURE(false, "Could not interpret on current position (!)");
                        nPos++;
                    }
                }
            }

            return aMatrix;
        }

        bool readSingleNumber(const OUString& rCandidate, SvgNumber& aNum)
        {
            const sal_Int32 nLen(rCandidate.getLength());
            sal_Int32 nPos(0);

            return readNumberAndUnit(rCandidate, nPos, aNum, nLen);
        }

        bool readLocalUrl(const OUString& rCandidate, OUString& rURL)
        {
            static const char aStrUrl[] = "url";

            if(rCandidate.startsWith(aStrUrl))
            {
                const sal_Int32 nLen(rCandidate.getLength());
                sal_Int32 nPos(strlen(aStrUrl));

                skip_char(rCandidate, '(', '#', nPos, nLen);
                OUStringBuffer aTokenValue;
                copyToLimiter(rCandidate, ')', nPos, aTokenValue, nLen);
                rURL = aTokenValue.makeStringAndClear();

                return true;
            }

            return false;
        }

        bool readSvgPaint(const OUString& rCandidate, SvgPaint& rSvgPaint,
            OUString& rURL, bool bCaseIndependent, SvgNumber& rOpacity)
        {
            if( !rCandidate.isEmpty() )
            {
                basegfx::BColor aColor;

                if(read_color(rCandidate, aColor, bCaseIndependent, rOpacity))
                {
                    rSvgPaint = SvgPaint(aColor, true, true);
                    return true;
                }
                else
                {
                    if(rCandidate.startsWith("none"))
                    {
                        rSvgPaint = SvgPaint(aColor, true, false, false);
                        return true;
                    }
                    else if(readLocalUrl(rCandidate, rURL))
                    {
                        /// Url is copied to rURL, but needs to be solved outside this helper
                        return false;
                    }
                    else if(rCandidate.startsWith("currentColor"))
                    {
                        rSvgPaint = SvgPaint(aColor, true, true, true);
                        return true;
                    }
                }
            }

            return false;
        }

        bool readSvgNumberVector(const OUString& rCandidate, SvgNumberVector& rSvgNumberVector)
        {
            const sal_Int32 nLen(rCandidate.getLength());
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

        SvgAspectRatio readSvgAspectRatio(const OUString& rCandidate)
        {
            const sal_Int32 nLen(rCandidate.getLength());

            if(nLen)
            {
                sal_Int32 nPos(0);
                SvgAlign aSvgAlign(Align_xMidYMid);
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
                            case SVGTokenDefer:
                            {
                                bChanged = true;
                                break;
                            }
                            case SVGTokenNone:
                            {
                                aSvgAlign = Align_none;
                                bChanged = true;
                                break;
                            }
                            case SVGTokenXMinYMin:
                            {
                                aSvgAlign = Align_xMinYMin;
                                bChanged = true;
                                break;
                            }
                            case SVGTokenXMidYMin:
                            {
                                aSvgAlign = Align_xMidYMin;
                                bChanged = true;
                                break;
                            }
                            case SVGTokenXMaxYMin:
                            {
                                aSvgAlign = Align_xMaxYMin;
                                bChanged = true;
                                break;
                            }
                            case SVGTokenXMinYMid:
                            {
                                aSvgAlign = Align_xMinYMid;
                                bChanged = true;
                                break;
                            }
                            case SVGTokenXMidYMid:
                            {
                                aSvgAlign = Align_xMidYMid;
                                bChanged = true;
                                break;
                            }
                            case SVGTokenXMaxYMid:
                            {
                                aSvgAlign = Align_xMaxYMid;
                                bChanged = true;
                                break;
                            }
                            case SVGTokenXMinYMax:
                            {
                                aSvgAlign = Align_xMinYMax;
                                bChanged = true;
                                break;
                            }
                            case SVGTokenXMidYMax:
                            {
                                aSvgAlign = Align_xMidYMax;
                                bChanged = true;
                                break;
                            }
                            case SVGTokenXMaxYMax:
                            {
                                aSvgAlign = Align_xMaxYMax;
                                bChanged = true;
                                break;
                            }
                            case SVGTokenMeet:
                            {
                                bMeetOrSlice = true;
                                bChanged = true;
                                break;
                            }
                            case SVGTokenSlice:
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
                        OSL_ENSURE(false, "Could not interpret on current position (!)");
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

        bool readSvgStringVector(const OUString& rCandidate, SvgStringVector& rSvgStringVector)
        {
            rSvgStringVector.clear();
            const sal_Int32 nLen(rCandidate.getLength());

            if(nLen)
            {
                sal_Int32 nPos(0);
                OUStringBuffer aTokenValue;
                skip_char(rCandidate, ' ', ',', nPos, nLen);

                while(nPos < nLen)
                {
                    copyToLimiter(rCandidate, ',', nPos, aTokenValue, nLen);
                    skip_char(rCandidate, ',', ' ', nPos, nLen);
                    const OUString aString = aTokenValue.makeStringAndClear();

                    if(!aString.isEmpty())
                    {
                        rSvgStringVector.push_back(aString);
                    }
                }
            }

            return !rSvgStringVector.empty();
        }

        void readImageLink(const OUString& rCandidate, OUString& rXLink, OUString& rUrl, OUString& rMimeType, OUString& rData)
        {
            rXLink.clear();
            rUrl.clear();
            rMimeType.clear();
            rData.clear();

            if('#' == rCandidate[0])
            {
                // local link
                rXLink = rCandidate.copy(1);
            }
            else
            {
                static const char aStrData[] = "data:";

                if(rCandidate.match(aStrData, 0))
                {
                    // embedded data
                    sal_Int32 nPos(strlen(aStrData));
                    sal_Int32 nLen(rCandidate.getLength());
                    OUStringBuffer aBuffer;

                    // read mime type
                    skip_char(rCandidate, ' ', nPos, nLen);
                    copyToLimiter(rCandidate, ';', nPos, aBuffer, nLen);
                    skip_char(rCandidate, ' ', ';', nPos, nLen);
                    rMimeType = aBuffer.makeStringAndClear();

                    if(!rMimeType.isEmpty() && nPos < nLen)
                    {
                        if(rMimeType.startsWith("image"))
                        {
                            // image data
                            OUString aData(rCandidate.copy(nPos));
                            static const char aStrBase64[] = "base64";

                            if(aData.startsWith(aStrBase64))
                            {
                                // base64 encoded
                                nPos = strlen(aStrBase64);
                                nLen = aData.getLength();

                                skip_char(aData, ' ', ',', nPos, nLen);

                                if(nPos < nLen)
                                {
                                    rData = aData.copy(nPos);
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

        OUString convert(const OUString& rCandidate, sal_Unicode nPattern, sal_Unicode nNew, bool bRemove)
        {
            const sal_Int32 nLen(rCandidate.getLength());

            if(nLen)
            {
                sal_Int32 nPos(0);
                OUStringBuffer aBuffer;
                bool bChanged(false);

                while(nPos < nLen)
                {
                    const sal_Unicode aChar(rCandidate[nPos]);

                    if(nPattern == aChar)
                    {
                        bChanged = true;

                        if(!bRemove)
                        {
                            aBuffer.append(nNew);
                        }
                    }
                    else
                    {
                        aBuffer.append(aChar);
                    }

                    nPos++;
                }

                if(bChanged)
                {
                    return aBuffer.makeStringAndClear();
                }
            }

            return rCandidate;
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

        OUString whiteSpaceHandlingDefault(const OUString& rCandidate)
        {
            const sal_Unicode aNewline('\n');
            const sal_Unicode aTab('\t');
            const sal_Unicode aSpace(' ');

            // remove all newline characters
            OUString aRetval(convert(rCandidate, aNewline, aNewline, true));

            // convert tab to space
            aRetval = convert(aRetval, aTab, aSpace, false);

            // strip of all leading and trailing spaces
            aRetval = aRetval.trim();

            // consolidate contiguous space
            aRetval = consolidateContiguousSpace(aRetval);

            return aRetval;
        }

        OUString whiteSpaceHandlingPreserve(const OUString& rCandidate)
        {
            const sal_Unicode aNewline('\n');
            const sal_Unicode aTab('\t');
            const sal_Unicode aSpace(' ');

            // convert newline to space
            convert(rCandidate, aNewline, aSpace, false);

            // convert tab to space
            convert(rCandidate, aTab, aSpace, false);

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
