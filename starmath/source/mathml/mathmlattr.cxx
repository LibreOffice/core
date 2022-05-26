/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <mathmlattr.hxx>

#include <o3tl/safeint.hxx>
#include <o3tl/string_view.hxx>
#include <rtl/math.h>

#include <cstddef>
#include <string_view>
#include <unordered_map>

static std::size_t ParseMathMLUnsignedNumber(std::u16string_view rStr, Fraction& rUN)
{
    auto nLen = rStr.length();
    std::size_t nDecimalPoint = std::u16string_view::npos;
    std::size_t nIdx;
    sal_Int64 nom = 0;
    sal_Int64 den = 1;
    bool validNomDen = true;
    for (nIdx = 0; nIdx < nLen; nIdx++)
    {
        auto cD = rStr[nIdx];
        if (cD == u'.')
        {
            if (nDecimalPoint != std::u16string_view::npos)
                return std::u16string_view::npos;
            nDecimalPoint = nIdx;
            continue;
        }
        if (cD < u'0' || u'9' < cD)
            break;
        if (validNomDen
            && (o3tl::checked_multiply(nom, sal_Int64(10), nom)
                || o3tl::checked_add(nom, sal_Int64(cD - u'0'), nom)
                || nom >= std::numeric_limits<sal_Int32>::max()
                || (nDecimalPoint != std::u16string_view::npos
                    && o3tl::checked_multiply(den, sal_Int64(10), den))))
        {
            validNomDen = false;
        }
    }
    if (nIdx == 0 || (nIdx == 1 && nDecimalPoint == 0))
        return std::u16string_view::npos;

    // If the input "xx.yyy" can be represented with nom = xx*10^n + yyy and den = 10^n in sal_Int64
    // (where n is the length of "yyy"), then use that to create an accurate Fraction (and TODO: we
    // could even ignore trailing "0" characters in "yyy", for a smaller n and thus a greater chance
    // of validNomDen); if not, use the less accurate approach of creating a Fraction from double:
    if (validNomDen)
    {
        rUN = Fraction(nom, den);
    }
    else
    {
        rUN = Fraction(
            rtl_math_uStringToDouble(rStr.data(), rStr.data() + nIdx, '.', 0, nullptr, nullptr));
    }

    return nIdx;
}

static std::size_t ParseMathMLNumber(std::u16string_view rStr, Fraction& rN)
{
    if (rStr.empty())
        return std::u16string_view::npos;
    bool bNegative = (rStr[0] == '-');
    std::size_t nOffset = bNegative ? 1 : 0;
    auto nIdx = ParseMathMLUnsignedNumber(rStr.substr(nOffset), rN);
    if (nIdx == std::u16string_view::npos || !rN.IsValid())
        return std::u16string_view::npos;
    if (bNegative)
        rN *= -1;
    return nOffset + nIdx;
}

bool ParseMathMLAttributeLengthValue(std::u16string_view rStr, MathMLAttributeLengthValue& rV)
{
    auto nIdx = ParseMathMLNumber(rStr, rV.aNumber);
    if (nIdx == std::u16string_view::npos)
        return false;
    std::u16string_view sRest = rStr.substr(nIdx);
    if (sRest.empty())
    {
        rV.eUnit = MathMLLengthUnit::None;
    }
    if (o3tl::starts_with(sRest, u"em"))
    {
        rV.eUnit = MathMLLengthUnit::Em;
    }
    if (o3tl::starts_with(sRest, u"ex"))
    {
        rV.eUnit = MathMLLengthUnit::Ex;
    }
    if (o3tl::starts_with(sRest, u"px"))
    {
        rV.eUnit = MathMLLengthUnit::Px;
    }
    if (o3tl::starts_with(sRest, u"in"))
    {
        rV.eUnit = MathMLLengthUnit::In;
    }
    if (o3tl::starts_with(sRest, u"cm"))
    {
        rV.eUnit = MathMLLengthUnit::Cm;
    }
    if (o3tl::starts_with(sRest, u"mm"))
    {
        rV.eUnit = MathMLLengthUnit::Mm;
    }
    if (o3tl::starts_with(sRest, u"pt"))
    {
        rV.eUnit = MathMLLengthUnit::Pt;
    }
    if (o3tl::starts_with(sRest, u"pc"))
    {
        rV.eUnit = MathMLLengthUnit::Pc;
    }
    if (sRest[0] == u'%')
    {
        rV.eUnit = MathMLLengthUnit::Percent;
    }
    return true;
}

bool GetMathMLMathvariantValue(const OUString& rStr, MathMLMathvariantValue& rV)
{
    static const std::unordered_map<OUString, MathMLMathvariantValue> aMap{
        { "normal", MathMLMathvariantValue::Normal },
        { "bold", MathMLMathvariantValue::Bold },
        { "italic", MathMLMathvariantValue::Italic },
        { "bold-italic", MathMLMathvariantValue::BoldItalic },
        { "double-struck", MathMLMathvariantValue::DoubleStruck },
        { "bold-fraktur", MathMLMathvariantValue::BoldFraktur },
        { "script", MathMLMathvariantValue::Script },
        { "bold-script", MathMLMathvariantValue::BoldScript },
        { "fraktur", MathMLMathvariantValue::Fraktur },
        { "sans-serif", MathMLMathvariantValue::SansSerif },
        { "bold-sans-serif", MathMLMathvariantValue::BoldSansSerif },
        { "sans-serif-italic", MathMLMathvariantValue::SansSerifItalic },
        { "sans-serif-bold-italic", MathMLMathvariantValue::SansSerifBoldItalic },
        { "monospace", MathMLMathvariantValue::Monospace },
        { "initial", MathMLMathvariantValue::Initial },
        { "tailed", MathMLMathvariantValue::Tailed },
        { "looped", MathMLMathvariantValue::Looped },
        { "stretched", MathMLMathvariantValue::Stretched }
    };

    auto it = aMap.find(rStr);
    if (it != aMap.end())
    {
        rV = it->second;
        return true;
    }
    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
