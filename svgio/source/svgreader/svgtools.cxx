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

#include <svgio/svgreader/svgtools.hxx>
#include <osl/thread.h>
#include <tools/color.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <svgio/svgreader/svgtoken.hxx>
#include <boost/unordered_map.hpp>

//////////////////////////////////////////////////////////////////////////////

namespace svgio
{
    namespace svgreader
    {
#ifdef DBG_UTIL
        void myAssert(const OUString& rMessage)
        {
            OString aMessage2;

            rMessage.convertToString(&aMessage2, osl_getThreadTextEncoding(), RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR|RTL_UNICODETOTEXT_FLAGS_INVALID_ERROR);
            OSL_ENSURE(false, aMessage2.getStr());
        }
#endif

        // common non-token strings
        const OUString commonStrings::aStrUserSpaceOnUse(OUString::createFromAscii("userSpaceOnUse"));
        const OUString commonStrings::aStrObjectBoundingBox(OUString::createFromAscii("objectBoundingBox"));
        const OUString commonStrings::aStrNonzero(OUString::createFromAscii("nonzero"));
        const OUString commonStrings::aStrEvenOdd(OUString::createFromAscii("evenodd"));

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
            if(!isSet() || Align_none == getSvgAlign())
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
                        break;
                    }
                    default:
                    {
                        OSL_ENSURE(false, "Do not use with percentage! ");
                        return 0.0;
                        break;
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
                        break;
                    }
                    case Unit_pt:
                    case Unit_pc:
                    case Unit_cm:
                    case Unit_mm:
                    case Unit_in:
                    case Unit_em:
                    case Unit_ex:
                    {
                        return solveNonPercentage( rInfoProvider);
                        break;
                    }
                    case Unit_percent:
                    {
                        double fRetval(mfNumber * 0.01);
                        const basegfx::B2DRange* pViewPort = rInfoProvider.getCurrentViewPort();

                        if(!pViewPort)
                        {
#ifdef DBG_UTIL
                            myAssert(rtl::OUString::createFromAscii("Design error, this case should have been handled in the caller"));
#endif
                            // no viewPort, assume a normal page size (A4)
                            static basegfx::B2DRange aDinA4Range(
                                0.0,
                                0.0,
                                210.0 * F_SVG_PIXEL_PER_INCH / 2.54,
                                297.0 * F_SVG_PIXEL_PER_INCH / 2.54);

                            pViewPort = &aDinA4Range;
                        }

                        if(pViewPort)
                        {
                            if(xcoordinate == aNumberType)
                            {
                                // it's a x-coordinate, relative to current width (w)
                                fRetval *= pViewPort->getWidth();
                            }
                            else if(ycoordinate == aNumberType)
                            {
                                // it's a y-coordinate, relative to current height (h)
                                fRetval *= pViewPort->getHeight();
                            }
                            else // length
                            {
                                // it's a length, relative to sqrt(w*w + h*h)/sqrt(2)
                                const double fCurrentWidth(pViewPort->getWidth());
                                const double fCurrentHeight(pViewPort->getHeight());
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

        void skip_char(const OUString& rCandidate, const sal_Unicode& rChar, sal_Int32& nPos, const sal_Int32 nLen)
        {
            while(nPos < nLen && rChar == rCandidate[nPos])
            {
                nPos++;
            }
        }

        void skip_char(const OUString& rCandidate, const sal_Unicode& rCharA, const sal_Unicode& rCharB, sal_Int32& nPos, const sal_Int32 nLen)
        {
            while(nPos < nLen && (rCharA == rCandidate[nPos] || rCharB == rCandidate[nPos]))
            {
                nPos++;
            }
        }

        void copySign(const OUString& rCandidate, sal_Int32& nPos, OUStringBuffer& rTarget, const sal_Int32 nLen)
        {
            if(nPos < nLen)
            {
                const sal_Unicode aChar(rCandidate[nPos]);

                if(sal_Unicode('+') == aChar || sal_Unicode('-') == aChar)
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

                bOnNumber = (sal_Unicode('0') <= aChar && sal_Unicode('9') >= aChar) || sal_Unicode('.') == aChar;

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

                bOnHex = (sal_Unicode('0') <= aChar && sal_Unicode('9') >= aChar)
                    || (sal_Unicode('A') <= aChar && sal_Unicode('F') >= aChar)
                    || (sal_Unicode('a') <= aChar && sal_Unicode('f') >= aChar);

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

                bOnChar = (sal_Unicode('a') <= aChar && sal_Unicode('z') >= aChar)
                    || (sal_Unicode('A') <= aChar && sal_Unicode('Z') >= aChar)
                    || sal_Unicode('-') == aChar;

                if(bOnChar)
                {
                    rTarget.append(aChar);
                    nPos++;
                }
            }
        }

        void copyToLimiter(const OUString& rCandidate, const sal_Unicode& rLimiter, sal_Int32& nPos, OUStringBuffer& rTarget, const sal_Int32 nLen)
        {
            while(nPos < nLen && rLimiter != rCandidate[nPos])
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

                    if(sal_Unicode('e') == aChar || sal_Unicode('E') == aChar)
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
                        aNum.makeStringAndClear(), (sal_Unicode)('.'), (sal_Unicode)(','),
                        &eStatus, 0);

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
                        case sal_Unicode('e') :
                        {
                            if(sal_Unicode('m') == aCharB)
                            {
                                // 'em' Relative to current font size
                                aRetval = Unit_em;
                                bTwoCharValid = true;
                            }
                            else if(sal_Unicode('x') == aCharB)
                            {
                                // 'ex' Relative to current font x-height
                                aRetval = Unit_ex;
                                bTwoCharValid = true;
                            }
                            break;
                        }
                        case sal_Unicode('p') :
                        {
                            if(sal_Unicode('x') == aCharB)
                            {
                                // 'px' UserUnit (default)
                                bTwoCharValid = true;
                            }
                            else if(sal_Unicode('t') == aCharB)
                            {
                                // 'pt' == 1.25 px
                                aRetval = Unit_pt;
                                bTwoCharValid = true;
                            }
                            else if(sal_Unicode('c') == aCharB)
                            {
                                // 'pc' == 15 px
                                aRetval = Unit_pc;
                                bTwoCharValid = true;
                            }
                            break;
                        }
                        case sal_Unicode('i') :
                        {
                            if(sal_Unicode('n') == aCharB)
                            {
                                // 'in' == 90 px
                                aRetval = Unit_in;
                                bTwoCharValid = true;
                            }
                            break;
                        }
                        case sal_Unicode('c') :
                        {
                            if(sal_Unicode('m') == aCharB)
                            {
                                // 'cm' == 35.43307 px
                                aRetval = Unit_cm;
                                bTwoCharValid = true;
                            }
                            break;
                        }
                        case sal_Unicode('m') :
                        {
                            if(sal_Unicode('m') == aCharB)
                            {
                                // 'mm' == 3.543307 px
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
                    if(sal_Unicode('%') == aCharA)
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
                skip_char(rCandidate, sal_Unicode(' '), nPos, nLen);
                aNum = SvgNumber(fNum, readUnit(rCandidate, nPos, nLen));

                return true;
            }

            return false;
        }

        bool readAngle(const OUString& rCandidate, sal_Int32& nPos, double& fAngle, const sal_Int32 nLen)
        {
            if(readNumber(rCandidate, nPos, fAngle, nLen))
            {
                skip_char(rCandidate, sal_Unicode(' '), nPos, nLen);

                enum DegreeType
                {
                    deg,
                    grad,
                    rad
                } aType(deg); // degrees is default

                if(nPos < nLen)
                {
                    const sal_Unicode aChar(rCandidate[nPos]);
                    static OUString aStrGrad(OUString::createFromAscii("grad"));
                    static OUString aStrRad(OUString::createFromAscii("rad"));

                    switch(aChar)
                    {
                        case sal_Unicode('g') :
                        case sal_Unicode('G') :
                        {
                            if(rCandidate.matchIgnoreAsciiCase(aStrGrad, nPos))
                            {
                                // angle in grad
                                nPos += aStrGrad.getLength();
                                aType = grad;
                            }
                            break;
                        }
                        case sal_Unicode('r') :
                        case sal_Unicode('R') :
                        {
                            if(rCandidate.matchIgnoreAsciiCase(aStrRad, nPos))
                            {
                                // angle in radians
                                nPos += aStrRad.getLength();
                                aType = rad;
                            }
                            break;
                        }
                    }
                }

                // convert to radians
                if(deg == aType)
                {
                    fAngle *= F_PI / 180.0;
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

        sal_Int32 read_hex(const sal_Unicode& rChar)
        {
            if(rChar >= sal_Unicode('0') && rChar <=sal_Unicode('9'))
            {
                return sal_Int32(rChar - sal_Unicode('0'));
            }
            else if(rChar >= sal_Unicode('A') && rChar <=sal_Unicode('F'))
            {
                return 10 + sal_Int32(rChar - sal_Unicode('A'));
            }
            else if(rChar >= sal_Unicode('a') && rChar <=sal_Unicode('f'))
            {
                return 10 + sal_Int32(rChar - sal_Unicode('a'));
            }
            else
            {
                // error
                return 0;
            }
        }

        bool match_colorKeyword(basegfx::BColor& rColor, const OUString& rName)
        {
            typedef boost::unordered_map< OUString, Color,
                      OUStringHash,
                      ::std::equal_to< OUString >
                      > ColorTokenMapper;
            typedef std::pair< OUString, Color > ColorTokenValueType;
            ColorTokenMapper aColorTokenMapperList;

            if(aColorTokenMapperList.empty())
            {
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("aliceblue"), Color(240, 248, 255)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("antiquewhite"), Color(250, 235, 215)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("aqua"), Color( 0, 255, 255)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("aquamarine"), Color(127, 255, 212)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("azure"), Color(240, 255, 255)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("beige"), Color(245, 245, 220)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("bisque"), Color(255, 228, 196)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("black"), Color( 0, 0, 0)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("blanchedalmond"), Color(255, 235, 205)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("blue"), Color( 0, 0, 255)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("blueviolet"), Color(138, 43, 226)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("brown"), Color(165, 42, 42)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("burlywood"), Color(222, 184, 135)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("cadetblue"), Color( 95, 158, 160)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("chartreuse"), Color(127, 255, 0)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("chocolate"), Color(210, 105, 30)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("coral"), Color(255, 127, 80)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("cornflowerblue"), Color(100, 149, 237)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("cornsilk"), Color(255, 248, 220)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("crimson"), Color(220, 20, 60)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("cyan"), Color( 0, 255, 255)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("darkblue"), Color( 0, 0, 139)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("darkcyan"), Color( 0, 139, 139)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("darkgoldenrod"), Color(184, 134, 11)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("darkgray"), Color(169, 169, 169)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("darkgreen"), Color( 0, 100, 0)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("darkgrey"), Color(169, 169, 169)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("darkkhaki"), Color(189, 183, 107)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("darkmagenta"), Color(139, 0, 139)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("darkolivegreen"), Color( 85, 107, 47)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("darkorange"), Color(255, 140, 0)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("darkorchid"), Color(153, 50, 204)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("darkred"), Color(139, 0, 0)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("darksalmon"), Color(233, 150, 122)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("darkseagreen"), Color(143, 188, 143)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("darkslateblue"), Color( 72, 61, 139)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("darkslategray"), Color( 47, 79, 79)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("darkslategrey"), Color( 47, 79, 79)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("darkturquoise"), Color( 0, 206, 209)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("darkviolet"), Color(148, 0, 211)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("deeppink"), Color(255, 20, 147)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("deepskyblue"), Color( 0, 191, 255)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("dimgray"), Color(105, 105, 105)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("dimgrey"), Color(105, 105, 105)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("dodgerblue"), Color( 30, 144, 255)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("firebrick"), Color(178, 34, 34)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("floralwhite"), Color(255, 250, 240)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("forestgreen"), Color( 34, 139, 34)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("fuchsia"), Color(255, 0, 255)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("gainsboro"), Color(220, 220, 220)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("ghostwhite"), Color(248, 248, 255)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("gold"), Color(255, 215, 0)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("goldenrod"), Color(218, 165, 32)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("gray"), Color(128, 128, 128)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("grey"), Color(128, 128, 128)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("green"), Color(0, 128, 0)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("greenyellow"), Color(173, 255, 47)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("honeydew"), Color(240, 255, 240)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("hotpink"), Color(255, 105, 180)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("indianred"), Color(205, 92, 92)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("indigo"), Color( 75, 0, 130)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("ivory"), Color(255, 255, 240)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("khaki"), Color(240, 230, 140)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("lavender"), Color(230, 230, 250)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("lavenderblush"), Color(255, 240, 245)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("lawngreen"), Color(124, 252, 0)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("lemonchiffon"), Color(255, 250, 205)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("lightblue"), Color(173, 216, 230)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("lightcoral"), Color(240, 128, 128)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("lightcyan"), Color(224, 255, 255)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("lightgoldenrodyellow"), Color(250, 250, 210)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("lightgray"), Color(211, 211, 211)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("lightgreen"), Color(144, 238, 144)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("lightgrey"), Color(211, 211, 211)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("lightpink"), Color(255, 182, 193)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("lightsalmon"), Color(255, 160, 122)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("lightseagreen"), Color( 32, 178, 170)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("lightskyblue"), Color(135, 206, 250)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("lightslategray"), Color(119, 136, 153)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("lightslategrey"), Color(119, 136, 153)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("lightsteelblue"), Color(176, 196, 222)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("lightyellow"), Color(255, 255, 224)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("lime"), Color( 0, 255, 0)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("limegreen"), Color( 50, 205, 50)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("linen"), Color(250, 240, 230)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("magenta"), Color(255, 0, 255)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("maroon"), Color(128, 0, 0)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("mediumaquamarine"), Color(102, 205, 170)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("mediumblue"), Color( 0, 0, 205)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("mediumorchid"), Color(186, 85, 211)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("mediumpurple"), Color(147, 112, 219)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("mediumseagreen"), Color( 60, 179, 113)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("mediumslateblue"), Color(123, 104, 238)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("mediumspringgreen"), Color( 0, 250, 154)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("mediumturquoise"), Color( 72, 209, 204)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("mediumvioletred"), Color(199, 21, 133)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("midnightblue"), Color( 25, 25, 112)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("mintcream"), Color(245, 255, 250)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("mistyrose"), Color(255, 228, 225)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("moccasin"), Color(255, 228, 181)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("navajowhite"), Color(255, 222, 173)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("navy"), Color( 0, 0, 128)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("oldlace"), Color(253, 245, 230)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("olive"), Color(128, 128, 0)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("olivedrab"), Color(107, 142, 35)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("orange"), Color(255, 165, 0)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("orangered"), Color(255, 69, 0)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("orchid"), Color(218, 112, 214)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("palegoldenrod"), Color(238, 232, 170)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("palegreen"), Color(152, 251, 152)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("paleturquoise"), Color(175, 238, 238)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("palevioletred"), Color(219, 112, 147)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("papayawhip"), Color(255, 239, 213)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("peachpuff"), Color(255, 218, 185)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("peru"), Color(205, 133, 63)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("pink"), Color(255, 192, 203)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("plum"), Color(221, 160, 221)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("powderblue"), Color(176, 224, 230)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("purple"), Color(128, 0, 128)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("red"), Color(255, 0, 0)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("rosybrown"), Color(188, 143, 143)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("royalblue"), Color( 65, 105, 225)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("saddlebrown"), Color(139, 69, 19)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("salmon"), Color(250, 128, 114)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("sandybrown"), Color(244, 164, 96)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("seagreen"), Color( 46, 139, 87)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("seashell"), Color(255, 245, 238)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("sienna"), Color(160, 82, 45)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("silver"), Color(192, 192, 192)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("skyblue"), Color(135, 206, 235)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("slateblue"), Color(106, 90, 205)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("slategray"), Color(112, 128, 144)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("slategrey"), Color(112, 128, 144)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("snow"), Color(255, 250, 250)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("springgreen"), Color( 0, 255, 127)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("steelblue"), Color( 70, 130, 180)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("tan"), Color(210, 180, 140)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("teal"), Color( 0, 128, 128)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("thistle"), Color(216, 191, 216)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("tomato"), Color(255, 99, 71)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("turquoise"), Color( 64, 224, 208)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("violet"), Color(238, 130, 238)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("wheat"), Color(245, 222, 179)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("white"), Color(255, 255, 255)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("whitesmoke"), Color(245, 245, 245)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("yellow"), Color(255, 255, 0)));
                aColorTokenMapperList.insert(ColorTokenValueType(OUString::createFromAscii("yellowgreen"), Color(154, 205, 50)));
            }

            const ColorTokenMapper::const_iterator aResult(aColorTokenMapperList.find(rName));

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

        bool read_color(const OUString& rCandidate, basegfx::BColor& rColor)
        {
            const sal_Int32 nLen(rCandidate.getLength());

            if(nLen)
            {
                const sal_Unicode aChar(rCandidate[0]);
                const double fFactor(1.0 / 255.0);

                if(aChar == sal_Unicode('#'))
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
                    static OUString aStrRgb(OUString::createFromAscii("rgb"));

                    if(rCandidate.matchIgnoreAsciiCase(aStrRgb, 0))
                    {
                        // rgb definition
                        sal_Int32 nPos(aStrRgb.getLength());
                        skip_char(rCandidate, sal_Unicode(' '), sal_Unicode('('), nPos, nLen);
                        double fR(0.0);

                        if(readNumber(rCandidate, nPos, fR, nLen))
                        {
                            skip_char(rCandidate, sal_Unicode(' '), nPos, nLen);

                            if(nPos < nLen)
                            {
                                const sal_Unicode aPercentChar(rCandidate[nPos]);
                                const bool bIsPercent(sal_Unicode('%') == aPercentChar);
                                double fG(0.0);

                                if(bIsPercent)
                                {
                                    skip_char(rCandidate, sal_Unicode('%'), nPos, nLen);
                                }

                                skip_char(rCandidate, sal_Unicode(' '), sal_Unicode(','), nPos, nLen);

                                if(readNumber(rCandidate, nPos, fG, nLen))
                                {
                                    double fB(0.0);

                                    if(bIsPercent)
                                    {
                                        skip_char(rCandidate, sal_Unicode('%'), nPos, nLen);
                                    }

                                    skip_char(rCandidate, sal_Unicode(' '), sal_Unicode(','), nPos, nLen);

                                    if(readNumber(rCandidate, nPos, fB, nLen))
                                    {
                                        const double fFac(bIsPercent ? 0.01 : fFactor);

                                        rColor.setRed(fR * fFac);
                                        rColor.setGreen(fG * fFac);
                                        rColor.setBlue(fB * fFac);

                                        if(bIsPercent)
                                        {
                                            skip_char(rCandidate, sal_Unicode('%'), nPos, nLen);
                                        }

                                        skip_char(rCandidate, sal_Unicode(' '), sal_Unicode(')'), nPos, nLen);
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

        basegfx::B2DRange readViewBox(const OUString& rCandidate, InfoProvider& rInfoProvider)
        {
            const sal_Int32 nLen(rCandidate.getLength());

            if(nLen)
            {
                sal_Int32 nPos(0);
                SvgNumber aMinX;
                skip_char(rCandidate, sal_Unicode(' '), sal_Unicode(','), nPos, nLen);

                if(readNumberAndUnit(rCandidate, nPos, aMinX, nLen))
                {
                    SvgNumber aMinY;
                    skip_char(rCandidate, sal_Unicode(' '), sal_Unicode(','), nPos, nLen);

                    if(readNumberAndUnit(rCandidate, nPos, aMinY, nLen))
                    {
                        SvgNumber aWidth;
                        skip_char(rCandidate, sal_Unicode(' '), sal_Unicode(','), nPos, nLen);

                        if(readNumberAndUnit(rCandidate, nPos, aWidth, nLen))
                        {
                            SvgNumber aHeight;
                            skip_char(rCandidate, sal_Unicode(' '), sal_Unicode(','), nPos, nLen);

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

        basegfx::B2DHomMatrix readTransform(const OUString& rCandidate, InfoProvider& rInfoProvider)
        {
            basegfx::B2DHomMatrix aMatrix;
            const sal_Int32 nLen(rCandidate.getLength());

            if(nLen)
            {
                sal_Int32 nPos(0);
                skip_char(rCandidate, sal_Unicode(' '), sal_Unicode(','), nPos, nLen);

                while(nPos < nLen)
                {
                    const sal_Unicode aChar(rCandidate[nPos]);
                    const sal_Int32 nInitPos(nPos);
                    static OUString aStrMatrix(OUString::createFromAscii("matrix"));
                    static OUString aStrTranslate(OUString::createFromAscii("translate"));
                    static OUString aStrScale(OUString::createFromAscii("scale"));
                    static OUString aStrRotate(OUString::createFromAscii("rotate"));
                    static OUString aStrSkewX(OUString::createFromAscii("skewX"));
                    static OUString aStrSkewY(OUString::createFromAscii("skewY"));

                    switch(aChar)
                    {
                        case sal_Unicode('m') :
                        {
                            if(rCandidate.match(aStrMatrix, nPos))
                            {
                                // matrix element
                                nPos += aStrMatrix.getLength();
                                skip_char(rCandidate, sal_Unicode(' '), sal_Unicode('('), nPos, nLen);
                                SvgNumber aVal;
                                basegfx::B2DHomMatrix aNew;

                                if(readNumberAndUnit(rCandidate, nPos, aVal, nLen))
                                {
                                    aNew.set(0, 0, aVal.solve(rInfoProvider)); // Element A
                                    skip_char(rCandidate, sal_Unicode(' '), sal_Unicode(','), nPos, nLen);

                                    if(readNumberAndUnit(rCandidate, nPos, aVal, nLen))
                                    {
                                        aNew.set(1, 0, aVal.solve(rInfoProvider)); // Element B
                                        skip_char(rCandidate, sal_Unicode(' '), sal_Unicode(','), nPos, nLen);

                                        if(readNumberAndUnit(rCandidate, nPos, aVal, nLen))
                                        {
                                            aNew.set(0, 1, aVal.solve(rInfoProvider)); // Element C
                                            skip_char(rCandidate, sal_Unicode(' '), sal_Unicode(','), nPos, nLen);

                                            if(readNumberAndUnit(rCandidate, nPos, aVal, nLen))
                                            {
                                                aNew.set(1, 1, aVal.solve(rInfoProvider)); // Element D
                                                skip_char(rCandidate, sal_Unicode(' '), sal_Unicode(','), nPos, nLen);

                                                if(readNumberAndUnit(rCandidate, nPos, aVal, nLen))
                                                {
                                                    aNew.set(0, 2, aVal.solve(rInfoProvider, xcoordinate)); // Element E
                                                    skip_char(rCandidate, sal_Unicode(' '), sal_Unicode(','), nPos, nLen);

                                                    if(readNumberAndUnit(rCandidate, nPos, aVal, nLen))
                                                    {
                                                        aNew.set(1, 2, aVal.solve(rInfoProvider, ycoordinate)); // Element F
                                                        skip_char(rCandidate, sal_Unicode(' '), sal_Unicode(')'), nPos, nLen);
                                                        skip_char(rCandidate, sal_Unicode(' '), sal_Unicode(','), nPos, nLen);

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
                        case sal_Unicode('t') :
                        {
                            if(rCandidate.match(aStrTranslate, nPos))
                            {
                                // translate element
                                nPos += aStrTranslate.getLength();
                                skip_char(rCandidate, sal_Unicode(' '), sal_Unicode('('), nPos, nLen);
                                SvgNumber aTransX;

                                if(readNumberAndUnit(rCandidate, nPos, aTransX, nLen))
                                {
                                    skip_char(rCandidate, sal_Unicode(' '), sal_Unicode(','), nPos, nLen);
                                    SvgNumber aTransY;
                                    readNumberAndUnit(rCandidate, nPos, aTransY, nLen);
                                    skip_char(rCandidate, sal_Unicode(' '), sal_Unicode(')'), nPos, nLen);
                                    skip_char(rCandidate, sal_Unicode(' '), sal_Unicode(','), nPos, nLen);

                                    aMatrix = aMatrix * basegfx::tools::createTranslateB2DHomMatrix(
                                        aTransX.solve(rInfoProvider, xcoordinate),
                                        aTransY.solve(rInfoProvider, ycoordinate));
                                }
                            }
                            break;
                        }
                        case sal_Unicode('s') :
                        {
                            if(rCandidate.match(aStrScale, nPos))
                            {
                                // scale element
                                nPos += aStrScale.getLength();
                                skip_char(rCandidate, sal_Unicode(' '), sal_Unicode('('), nPos, nLen);
                                SvgNumber aScaleX;

                                if(readNumberAndUnit(rCandidate, nPos, aScaleX, nLen))
                                {
                                    skip_char(rCandidate, sal_Unicode(' '), sal_Unicode(','), nPos, nLen);
                                    SvgNumber aScaleY(aScaleX);
                                    readNumberAndUnit(rCandidate, nPos, aScaleY, nLen);
                                    skip_char(rCandidate, sal_Unicode(' '), sal_Unicode(')'), nPos, nLen);
                                    skip_char(rCandidate, sal_Unicode(' '), sal_Unicode(','), nPos, nLen);

                                    aMatrix = aMatrix * basegfx::tools::createScaleB2DHomMatrix(
                                        aScaleX.solve(rInfoProvider),
                                        aScaleY.solve(rInfoProvider));
                                }
                            }
                            else if(rCandidate.match(aStrSkewX, nPos))
                            {
                                // skewx element
                                nPos += aStrSkewX.getLength();
                                skip_char(rCandidate, sal_Unicode(' '), sal_Unicode('('), nPos, nLen);
                                double fSkewX(0.0);

                                if(readAngle(rCandidate, nPos, fSkewX, nLen))
                                {
                                    skip_char(rCandidate, sal_Unicode(' '), sal_Unicode(')'), nPos, nLen);
                                    skip_char(rCandidate, sal_Unicode(' '), sal_Unicode(','), nPos, nLen);

                                    aMatrix = aMatrix * basegfx::tools::createShearXB2DHomMatrix(tan(fSkewX));
                                }
                            }
                            else if(rCandidate.match(aStrSkewY, nPos))
                            {
                                // skewy element
                                nPos += aStrSkewY.getLength();
                                skip_char(rCandidate, sal_Unicode(' '), sal_Unicode('('), nPos, nLen);
                                double fSkewY(0.0);

                                if(readAngle(rCandidate, nPos, fSkewY, nLen))
                                {
                                    skip_char(rCandidate, sal_Unicode(' '), sal_Unicode(')'), nPos, nLen);
                                    skip_char(rCandidate, sal_Unicode(' '), sal_Unicode(','), nPos, nLen);

                                    aMatrix = aMatrix * basegfx::tools::createShearYB2DHomMatrix(tan(fSkewY));
                                }
                            }
                            break;
                        }
                        case sal_Unicode('r') :
                        {
                            if(rCandidate.match(aStrRotate, nPos))
                            {
                                // rotate element
                                nPos += aStrRotate.getLength();
                                skip_char(rCandidate, sal_Unicode(' '), sal_Unicode('('), nPos, nLen);
                                double fAngle(0.0);

                                if(readAngle(rCandidate, nPos, fAngle, nLen))
                                {
                                    skip_char(rCandidate, sal_Unicode(' '), sal_Unicode(','), nPos, nLen);
                                    SvgNumber aX;
                                    readNumberAndUnit(rCandidate, nPos, aX, nLen);
                                    skip_char(rCandidate, sal_Unicode(' '), sal_Unicode(','), nPos, nLen);
                                    SvgNumber aY;
                                    readNumberAndUnit(rCandidate, nPos, aY, nLen);
                                    skip_char(rCandidate, sal_Unicode(' '), sal_Unicode(')'), nPos, nLen);
                                    skip_char(rCandidate, sal_Unicode(' '), sal_Unicode(','), nPos, nLen);

                                    const double fX(aX.isSet() ? aX.solve(rInfoProvider, xcoordinate) : 0.0);
                                    const double fY(aY.isSet() ? aY.solve(rInfoProvider, ycoordinate) : 0.0);

                                    if(!basegfx::fTools::equalZero(fX) || !basegfx::fTools::equalZero(fY))
                                    {
                                        // rotate around point
                                        aMatrix = aMatrix * basegfx::tools::createRotateAroundPoint(fX, fY, fAngle);
                                    }
                                    else
                                    {
                                        // rotate
                                        aMatrix = aMatrix * basegfx::tools::createRotateB2DHomMatrix(fAngle);
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
            static OUString aStrUrl(OUString::createFromAscii("url"));

            if(rCandidate.match(aStrUrl, 0))
            {
                const sal_Int32 nLen(rCandidate.getLength());
                sal_Int32 nPos(aStrUrl.getLength());

                skip_char(rCandidate, sal_Unicode('('), sal_Unicode('#'), nPos, nLen);
                OUStringBuffer aTokenValue;
                copyToLimiter(rCandidate, sal_Unicode(')'), nPos, aTokenValue, nLen);
                rURL = aTokenValue.makeStringAndClear();

                return true;
            }

            return false;
        }

        bool readSvgPaint(const OUString& rCandidate, SvgPaint& rSvgPaint, OUString& rURL)
        {
            if( !rCandidate.isEmpty() )
            {
                basegfx::BColor aColor;

                if(read_color(rCandidate, aColor))
                {
                    rSvgPaint = SvgPaint(aColor, true, true);
                    return true;
                }
                else
                {
                    static OUString aStrNone(OUString::createFromAscii("none"));
                    static OUString aStrCurrentColor(OUString::createFromAscii("currentColor"));

                    if(rCandidate.match(aStrNone, 0))
                    {
                        rSvgPaint = SvgPaint(aColor, true, false, false);
                        return true;
                    }
                    else if(readLocalUrl(rCandidate, rURL))
                    {
                        /// Url is copied to rURL, but needs to be solved outside this helper
                        return false;
                    }
                    else if(rCandidate.match(aStrCurrentColor, 0))
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
                skip_char(rCandidate, sal_Unicode(' '), sal_Unicode(','), nPos, nLen);

                while(readNumberAndUnit(rCandidate, nPos, aNum, nLen))
                {
                    rSvgNumberVector.push_back(aNum);
                    skip_char(rCandidate, sal_Unicode(' '), sal_Unicode(','), nPos, nLen);
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
                bool bDefer(false);
                bool bMeetOrSlice(true);
                bool bChanged(false);

                while(nPos < nLen)
                {
                    const sal_Int32 nInitPos(nPos);
                    skip_char(rCandidate, sal_Unicode(' '), nPos, nLen);
                    OUStringBuffer aTokenName;
                    copyString(rCandidate, nPos, aTokenName, nLen);

                    if(!aTokenName.isEmpty())
                    {
                        switch(StrToSVGToken(aTokenName.makeStringAndClear()))
                        {
                            case SVGTokenDefer:
                            {
                                bDefer = true;
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
                    return SvgAspectRatio(aSvgAlign, bDefer, bMeetOrSlice);
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
                skip_char(rCandidate, sal_Unicode(' '), sal_Unicode(','), nPos, nLen);

                while(nPos < nLen)
                {
                    copyToLimiter(rCandidate, sal_Unicode(','), nPos, aTokenValue, nLen);
                    skip_char(rCandidate, sal_Unicode(','), sal_Unicode(' '), nPos, nLen);
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
            rXLink = rUrl = rMimeType = rData = OUString();

            if(sal_Unicode('#') == rCandidate[0])
            {
                // local link
                rXLink = rCandidate.copy(1);
            }
            else
            {
                static OUString aStrData(OUString::createFromAscii("data:"));

                if(rCandidate.match(aStrData, 0))
                {
                    // embedded data
                    sal_Int32 nPos(aStrData.getLength());
                    sal_Int32 nLen(rCandidate.getLength());
                    OUStringBuffer aBuffer;

                    // read mime type
                    skip_char(rCandidate, sal_Unicode(' '), nPos, nLen);
                    copyToLimiter(rCandidate, sal_Unicode(';'), nPos, aBuffer, nLen);
                    skip_char(rCandidate, sal_Unicode(' '), sal_Unicode(';'), nPos, nLen);
                    rMimeType = aBuffer.makeStringAndClear();

                    if(!rMimeType.isEmpty() && nPos < nLen)
                    {
                        static OUString aStrImage(OUString::createFromAscii("image"));

                        if(rMimeType.match(aStrImage, 0))
                        {
                            // image data
                            OUString aData(rCandidate.copy(nPos));
                            static OUString aStrBase64(OUString::createFromAscii("base64"));

                            if(aData.match(aStrBase64, 0))
                            {
                                // base64 encoded
                                nPos = aStrBase64.getLength();
                                nLen = aData.getLength();

                                skip_char(aData, sal_Unicode(' '), sal_Unicode(','), nPos, nLen);

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

        OUString convert(const OUString& rCandidate, const sal_Unicode& rPattern, const sal_Unicode& rNew, bool bRemove)
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

                    if(rPattern == aChar)
                    {
                        bChanged = true;

                        if(!bRemove)
                        {
                            aBuffer.append(rNew);
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

        OUString consolidateContiguosSpace(const OUString& rCandidate)
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

            // consolidate contiguos space
            aRetval = consolidateContiguosSpace(aRetval);

            return aRetval;
        }

        OUString whiteSpaceHandlingPreserve(const OUString& rCandidate)
        {
            const sal_Unicode aNewline('\n');
            const sal_Unicode aTab('\t');
            const sal_Unicode aSpace(' ');

            // convert newline to space
            OUString aRetval(convert(rCandidate, aNewline, aSpace, false));

            // convert tab to space
            aRetval = convert(rCandidate, aTab, aSpace, false);

            return rCandidate;
        }

        ::std::vector< double > solveSvgNumberVector(const SvgNumberVector& rInput, const InfoProvider& rInfoProvider, NumberType aNumberType)
        {
            ::std::vector< double > aRetval;

            if(!rInput.empty())
            {
                const double nCount(rInput.size());
                aRetval.reserve(nCount);

                for(sal_uInt32 a(0); a < nCount; a++)
                {
                    aRetval.push_back(rInput[a].solve(rInfoProvider, aNumberType));
                }
            }

            return aRetval;
        }

    } // end of namespace svgreader
} // end of namespace svgio

//////////////////////////////////////////////////////////////////////////////
// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
