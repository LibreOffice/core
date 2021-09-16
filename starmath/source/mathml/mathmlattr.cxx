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

#include <unordered_map>

static sal_Int32 ParseMathMLUnsignedNumber(const OUString& rStr, Fraction& rUN)
{
    auto nLen = rStr.getLength();
    sal_Int32 nDecimalPoint = -1;
    sal_Int32 nIdx;
    sal_Int64 nom = 0;
    sal_Int64 den = 1;
    bool validNomDen = true;
    for (nIdx = 0; nIdx < nLen; nIdx++)
    {
        auto cD = rStr[nIdx];
        if (cD == u'.')
        {
            if (nDecimalPoint >= 0)
                return -1;
            nDecimalPoint = nIdx;
            continue;
        }
        if (cD < u'0' || u'9' < cD)
            break;
        if (validNomDen
            && (o3tl::checked_multiply(nom, sal_Int64(10), nom)
                || o3tl::checked_add(nom, sal_Int64(cD - u'0'), nom)
                || (nDecimalPoint >= 0 && o3tl::checked_multiply(den, sal_Int64(10), den))))
        {
            validNomDen = false;
        }
    }
    if (nIdx == 0 || (nIdx == 1 && nDecimalPoint == 0))
        return -1;

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
        rUN = Fraction(rStr.copy(0, nIdx).toDouble());
    }

    return nIdx;
}

static sal_Int32 ParseMathMLNumber(const OUString& rStr, Fraction& rN)
{
    if (rStr.isEmpty())
        return -1;
    bool bNegative = (rStr[0] == '-');
    sal_Int32 nOffset = bNegative ? 1 : 0;
    auto nIdx = ParseMathMLUnsignedNumber(rStr.copy(nOffset), rN);
    if (nIdx <= 0 || !rN.IsValid())
        return -1;
    if (bNegative)
        rN *= -1;
    return nOffset + nIdx;
}

sal_Int32 ParseMathMLAttributeLengthValue(const OUString& rStr, MathMLAttributeLengthValue& rV)
{
    auto nIdx = ParseMathMLNumber(rStr, rV.aNumber);
    if (nIdx <= 0)
        return -1;
    OUString sRest = rStr.copy(nIdx);
    if (sRest.isEmpty())
    {
        rV.eUnit = MathMLLengthUnit::None;
        return nIdx;
    }
    if (sRest.startsWith("em"))
    {
        rV.eUnit = MathMLLengthUnit::Em;
        return nIdx + 2;
    }
    if (sRest.startsWith("ex"))
    {
        rV.eUnit = MathMLLengthUnit::Ex;
        return nIdx + 2;
    }
    if (sRest.startsWith("px"))
    {
        rV.eUnit = MathMLLengthUnit::Px;
        return nIdx + 2;
    }
    if (sRest.startsWith("in"))
    {
        rV.eUnit = MathMLLengthUnit::In;
        return nIdx + 2;
    }
    if (sRest.startsWith("cm"))
    {
        rV.eUnit = MathMLLengthUnit::Cm;
        return nIdx + 2;
    }
    if (sRest.startsWith("mm"))
    {
        rV.eUnit = MathMLLengthUnit::Mm;
        return nIdx + 2;
    }
    if (sRest.startsWith("pt"))
    {
        rV.eUnit = MathMLLengthUnit::Pt;
        return nIdx + 2;
    }
    if (sRest.startsWith("pc"))
    {
        rV.eUnit = MathMLLengthUnit::Pc;
        return nIdx + 2;
    }
    if (sRest[0] == u'%')
    {
        rV.eUnit = MathMLLengthUnit::Percent;
        return nIdx + 2;
    }
    return nIdx;
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
