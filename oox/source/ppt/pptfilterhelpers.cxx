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

#include <com/sun/star/animations/TransitionType.hpp>
#include <com/sun/star/animations/TransitionSubType.hpp>
#include <com/sun/star/animations/ValuePair.hpp>
#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/awt/FontUnderline.hpp>
#include <com/sun/star/awt/FontSlant.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <oox/ppt/pptfilterhelpers.hxx>
#include <rtl/ustrbuf.hxx>
#include <tools/color.hxx>

namespace {
    int lcl_gethex(int nChar)
    {
        if (nChar >= '0' && nChar <= '9')
            return nChar - '0';
        else if (nChar >= 'a' && nChar <= 'f')
            return nChar - 'a' + 10;
        else if (nChar >= 'A' && nChar <= 'F')
            return nChar - 'A' + 10;
        else
            return 0;
    }
}

namespace oox { namespace ppt {

    const ImplAttributeNameConversion *getAttributeConversionList()
    {
        static const ImplAttributeNameConversion aList[] =
        {
            { AnimationAttributeEnum::PPT_X,             "ppt_x",                        "X" },
            { AnimationAttributeEnum::PPT_Y,             "ppt_y",                        "Y" },
            { AnimationAttributeEnum::PPT_W,             "ppt_w",                        "Width" },
            { AnimationAttributeEnum::PPT_H,             "ppt_h",                        "Height" },
            { AnimationAttributeEnum::PPT_C,             "ppt_c",                        "DimColor" },
            { AnimationAttributeEnum::R,                 "r",                            "Rotate" },
            { AnimationAttributeEnum::XSHEAR,            "xshear",                       "SkewX" },
            { AnimationAttributeEnum::FILLCOLOR,         "fillcolor",                    "FillColor" },
            { AnimationAttributeEnum::FILLCOLOR,         "fillColor",                    "FillColor" },
            { AnimationAttributeEnum::FILLTYPE,          "fill.type",                    "FillStyle" },
            { AnimationAttributeEnum::FILLON,            "fill.on",                      "FillOn" },
            { AnimationAttributeEnum::STROKECOLOR,       "stroke.color",                 "LineColor" },
            { AnimationAttributeEnum::STROKEON,          "stroke.on",                    "LineStyle" },
            { AnimationAttributeEnum::STYLECOLOR,        "style.color",                  "CharColor" },
            { AnimationAttributeEnum::STYLEROTATION,     "style.rotation",               "Rotate" },
            { AnimationAttributeEnum::FONTWEIGHT,        "style.fontWeight",             "CharWeight" },
            { AnimationAttributeEnum::STYLEUNDERLINE,    "style.textDecorationUnderline","CharUnderline" },
            { AnimationAttributeEnum::STYLEFONTFAMILY,   "style.fontFamily",             "CharFontName" },
            { AnimationAttributeEnum::STYLEFONTSIZE,     "style.fontSize",               "CharHeight" },
            { AnimationAttributeEnum::STYLEFONTSTYLE,    "style.fontStyle",              "CharPosture" },
            { AnimationAttributeEnum::STYLEVISIBILITY,   "style.visibility",             "Visibility" },
            { AnimationAttributeEnum::STYLEOPACITY,      "style.opacity",                "Opacity" },
            { AnimationAttributeEnum::UNKNOWN, nullptr, nullptr }
        };

        return aList;
    }

    const transition* transition::getList()
    {
        static const transition aList[] =
        {
            { "wipe(up)", css::animations::TransitionType::BARWIPE, css::animations::TransitionSubType::TOPTOBOTTOM, true },
            { "wipe(right)", css::animations::TransitionType::BARWIPE, css::animations::TransitionSubType::LEFTTORIGHT, false },
            { "wipe(left)", css::animations::TransitionType::BARWIPE, css::animations::TransitionSubType::LEFTTORIGHT, true },
            { "wipe(down)", css::animations::TransitionType::BARWIPE, css::animations::TransitionSubType::TOPTOBOTTOM, false },
            { "wheel(1)", css::animations::TransitionType::PINWHEELWIPE, css::animations::TransitionSubType::ONEBLADE, true },
            { "wheel(2)", css::animations::TransitionType::PINWHEELWIPE, css::animations::TransitionSubType::TWOBLADEVERTICAL, true },
            { "wheel(3)", css::animations::TransitionType::PINWHEELWIPE, css::animations::TransitionSubType::THREEBLADE, true },
            { "wheel(4)", css::animations::TransitionType::PINWHEELWIPE, css::animations::TransitionSubType::FOURBLADE, true },
            { "wheel(8)", css::animations::TransitionType::PINWHEELWIPE, css::animations::TransitionSubType::EIGHTBLADE, true },
            { "strips(downLeft)", css::animations::TransitionType::WATERFALLWIPE, css::animations::TransitionSubType::HORIZONTALRIGHT, true },
            { "strips(upLeft)", css::animations::TransitionType::WATERFALLWIPE, css::animations::TransitionSubType::HORIZONTALLEFT, false },
            { "strips(downRight)", css::animations::TransitionType::WATERFALLWIPE, css::animations::TransitionSubType::HORIZONTALLEFT, true },
            { "strips(upRight)", css::animations::TransitionType::WATERFALLWIPE, css::animations::TransitionSubType::HORIZONTALRIGHT, false },
            { "barn(inVertical)", css::animations::TransitionType::BARNDOORWIPE, css::animations::TransitionSubType::VERTICAL, false },
            { "barn(outVertical)", css::animations::TransitionType::BARNDOORWIPE, css::animations::TransitionSubType::VERTICAL, true },
            { "barn(inHorizontal)", css::animations::TransitionType::BARNDOORWIPE, css::animations::TransitionSubType::HORIZONTAL, false },
            { "barn(outHorizontal)", css::animations::TransitionType::BARNDOORWIPE, css::animations::TransitionSubType::HORIZONTAL, true },
            { "randombar(vertical)", css::animations::TransitionType::RANDOMBARWIPE, css::animations::TransitionSubType::VERTICAL, true},
            { "randombar(horizontal)", css::animations::TransitionType::RANDOMBARWIPE, css::animations::TransitionSubType::HORIZONTAL, true },
            { "checkerboard(down)", css::animations::TransitionType::CHECKERBOARDWIPE, css::animations::TransitionSubType::DOWN, true},
            { "checkerboard(across)", css::animations::TransitionType::CHECKERBOARDWIPE, css::animations::TransitionSubType::ACROSS, true },
            { "plus(out)", css::animations::TransitionType::FOURBOXWIPE, css::animations::TransitionSubType::CORNERSIN, false },
            { "plus(in)", css::animations::TransitionType::FOURBOXWIPE, css::animations::TransitionSubType::CORNERSIN, true },
            { "diamond(out)", css::animations::TransitionType::IRISWIPE, css::animations::TransitionSubType::DIAMOND, true },
            { "diamond(in)", css::animations::TransitionType::IRISWIPE, css::animations::TransitionSubType::DIAMOND, false },
            { "circle(out)", css::animations::TransitionType::ELLIPSEWIPE, css::animations::TransitionSubType::HORIZONTAL, true },
            { "circle(in)", css::animations::TransitionType::ELLIPSEWIPE, css::animations::TransitionSubType::HORIZONTAL, false },
            { "box(out)", css::animations::TransitionType::IRISWIPE, css::animations::TransitionSubType::RECTANGLE, true },
            { "box(in)", css::animations::TransitionType::IRISWIPE, css::animations::TransitionSubType::RECTANGLE, false },
            { "wedge", css::animations::TransitionType::FANWIPE, css::animations::TransitionSubType::CENTERTOP, true },
            { "blinds(vertical)", css::animations::TransitionType::BLINDSWIPE, css::animations::TransitionSubType::VERTICAL, true },
            { "blinds(horizontal)", css::animations::TransitionType::BLINDSWIPE, css::animations::TransitionSubType::HORIZONTAL, true },
            { "fade", css::animations::TransitionType::FADE, css::animations::TransitionSubType::CROSSFADE, true },
            { "slide(fromTop)", css::animations::TransitionType::SLIDEWIPE, css::animations::TransitionSubType::FROMTOP, true },
            { "slide(fromRight)", css::animations::TransitionType::SLIDEWIPE, css::animations::TransitionSubType::FROMRIGHT, true },
            { "slide(fromLeft)", css::animations::TransitionType::SLIDEWIPE, css::animations::TransitionSubType::FROMLEFT, true },
            { "slide(fromBottom)", css::animations::TransitionType::SLIDEWIPE, css::animations::TransitionSubType::FROMBOTTOM, true },
            { "dissolve", css::animations::TransitionType::DISSOLVE, css::animations::TransitionSubType::DEFAULT, true },
            { "image", css::animations::TransitionType::DISSOLVE, css::animations::TransitionSubType::DEFAULT, true }, // TODO
            { nullptr, 0, 0, false }
        };

        return aList;
    }

    const transition* transition::find( const OUString& rName )
    {
        const transition* p = transition::getList();

        while( p->mpName )
        {
            if( rName.equalsAscii( p->mpName ) )
                return p;

            p++;
        }

        return nullptr;
    }

    bool convertMeasure(OUString& rString)
    {
        bool bRet = false;

        const sal_Char* pSource[] = { "ppt_x", "ppt_y", "ppt_w", "ppt_h", nullptr };
        const sal_Char* pDest[] = { "x", "y", "width", "height", nullptr };

        /* here we want to substitute all occurrences of
         * [#]ppt_[xyhw] with
         * x,y,height and width respectively
         */
        sal_Int32 nIndex = 0;

        const sal_Char** ps = pSource;
        const sal_Char** pd = pDest;

        while (*ps)
        {
            const OUString aSearch(OUString::createFromAscii(*ps));
            while ((nIndex = rString.indexOf(aSearch, nIndex)) != -1)
            {
                sal_Int32 nLength = aSearch.getLength();
                if (nIndex && (rString[nIndex - 1] == '#'))
                {
                    nIndex--;
                    nLength++;
                }

                const OUString aNew(OUString::createFromAscii(*pd));
                rString = rString.replaceAt(nIndex, nLength, aNew);
                nIndex += aNew.getLength();
                bRet = true;
            }
            ps++;
            pd++;
        }

        return bRet;
    }

    bool convertAnimationValue(AnimationAttributeEnum eAttribute, css::uno::Any& rValue)
    {
        using css::animations::ValuePair;
        bool bRet = false;
        switch (eAttribute)
        {
            case AnimationAttributeEnum::PPT_X:
            case AnimationAttributeEnum::PPT_Y:
            case AnimationAttributeEnum::PPT_W:
            case AnimationAttributeEnum::PPT_H:
            {
                OUString aString;

                if (rValue.getValueType() == cppu::UnoType<ValuePair>::get())
                {
                    ValuePair aValuePair;
                    if (rValue >>= aValuePair)
                    {
                        if (aValuePair.First >>= aString)
                        {
                            if (convertMeasure(aString))
                            {
                                aValuePair.First <<= aString;
                                bRet = true;
                            }
                        }

                        if (aValuePair.Second >>= aString)
                        {
                            if (convertMeasure(aString))
                            {
                                aValuePair.Second <<= aString;
                                bRet = true;
                            }
                        }
                    }
                }
                else if (rValue.getValueType() == cppu::UnoType<OUString>::get())
                {
                    if (rValue >>= aString)
                    {
                        bRet = convertMeasure(aString);

                        if (bRet)
                            rValue <<= aString;
                    }
                }
            }
            break;

            case AnimationAttributeEnum::XSHEAR:
            case AnimationAttributeEnum::R:
            {
                OUString aString;
                if (rValue >>= aString)
                {
                    rValue <<= aString.toDouble();
                    bRet = true;
                }
            }
            break;

            case AnimationAttributeEnum::STYLEROTATION:
            {
                if (rValue.getValueType() == cppu::UnoType<OUString>::get())
                {
                    OUString aString;
                    rValue >>= aString;
                    rValue <<= static_cast<sal_Int16>(aString.toDouble());
                    bRet = true;
                }
                else if (rValue.getValueType() == cppu::UnoType<double>::get())
                {
                    double fValue = 0.0;
                    rValue >>= fValue;
                    rValue <<= static_cast<sal_Int16>(fValue);
                    bRet = true;
                }
            }
            break;

            case AnimationAttributeEnum::FILLCOLOR:
            case AnimationAttributeEnum::STROKECOLOR:
            case AnimationAttributeEnum::STYLECOLOR:
            case AnimationAttributeEnum::PPT_C:
            {
                OUString aString;
                if (rValue >>= aString)
                {
                    if (aString.getLength() >= 7 && aString[0] == '#')
                    {
                        Color aColor;
                        aColor.SetRed(static_cast<sal_uInt8>(lcl_gethex(aString[1]) * 16
                                                             + lcl_gethex(aString[2])));
                        aColor.SetGreen(static_cast<sal_uInt8>(lcl_gethex(aString[3]) * 16
                                                               + lcl_gethex(aString[4])));
                        aColor.SetBlue(static_cast<sal_uInt8>(lcl_gethex(aString[5]) * 16
                                                              + lcl_gethex(aString[6])));
                        rValue <<= aColor;
                        bRet = true;
                    }
                    else if (aString.startsWith("rgb("))
                    {
                        aString = aString.copy(4, aString.getLength() - 5);
                        Color aColor;
                        sal_Int32 index = 0;
                        aColor.SetRed(
                            static_cast<sal_uInt8>(aString.getToken(0, ',', index).toInt32()));
                        aColor.SetGreen(
                            static_cast<sal_uInt8>(aString.getToken(0, ',', index).toInt32()));
                        aColor.SetRed(
                            static_cast<sal_uInt8>(aString.getToken(0, ',', index).toInt32()));
                        rValue <<= aColor;
                        bRet = true;
                    }
                    else if (aString.startsWith("hsl("))
                    {
                        sal_Int32 index = 0;
                        sal_Int32 nA = aString.getToken(0, ',', index).toInt32();
                        sal_Int32 nB = aString.getToken(0, ',', index).toInt32();
                        sal_Int32 nC = aString.getToken(0, ',', index).toInt32();
                        css::uno::Sequence<double> aHSL(3);
                        aHSL[0] = nA * 360.0 / 255.0;
                        aHSL[1] = nB / 255.0;
                        aHSL[2] = nC / 255.0;
                        rValue <<= aHSL;
                        bRet = true;
                    }
                }
            }
            break;
            case AnimationAttributeEnum::FILLON:
            {
                // Slideshow doesn't support FillOn, but we need to convert the value type
                // so it can be written out again.
                OUString aString;
                if (rValue >>= aString)
                {
                    rValue <<= aString == "true";
                    bRet = true;
                }
            }
            break;

            case AnimationAttributeEnum::FILLTYPE:
            {
                OUString aString;
                if (rValue >>= aString)
                {
                    rValue <<= aString == "solid" ? css::drawing::FillStyle_SOLID
                                                  : css::drawing::FillStyle_NONE;
                    bRet = true;
                }
            }
            break;

            case AnimationAttributeEnum::STROKEON:
            {
                OUString aString;
                if (rValue >>= aString)
                {
                    rValue <<= aString == "true" ? css::drawing::LineStyle_SOLID
                                                 : css::drawing::LineStyle_NONE;
                    bRet = true;
                }
            }
            break;

            case AnimationAttributeEnum::FONTWEIGHT:
            {
                OUString aString;
                if (rValue >>= aString)
                {
                    rValue <<= aString == "bold" ? css::awt::FontWeight::BOLD
                                                 : css::awt::FontWeight::NORMAL;
                    bRet = true;
                }
            }
            break;

            case AnimationAttributeEnum::STYLEFONTSTYLE:
            {
                OUString aString;
                if (rValue >>= aString)
                {
                    rValue
                        <<= aString == "italic" ? css::awt::FontSlant_ITALIC : css::awt::FontSlant_NONE;
                    bRet = true;
                }
            }
            break;

            case AnimationAttributeEnum::STYLEUNDERLINE:
            {
                OUString aString;
                if (rValue >>= aString)
                {
                    rValue <<= aString == "true" ? css::awt::FontUnderline::SINGLE
                                                 : css::awt::FontUnderline::NONE;
                    bRet = true;
                }
            }
            break;

            case AnimationAttributeEnum::STYLEOPACITY:
            case AnimationAttributeEnum::STYLEFONTSIZE:
            {
                OUString aString;
                if (rValue >>= aString)
                {
                    rValue <<= static_cast<float>(aString.toDouble());
                    bRet = true;
                }
            }
            break;

            case AnimationAttributeEnum::STYLEVISIBILITY:
            {
                OUString aString;
                if (rValue >>= aString)
                {
                    rValue <<= aString == "visible";
                    bRet = true;
                }
            }
            break;
            default:
                break;
        }

        return bRet;
    }

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
