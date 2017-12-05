/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "mathmlattr.hxx"

#include <cassert>
#include <unordered_map>

namespace {

sal_Int32 lcl_GetPowerOf10(sal_Int32 nPower)
{
    assert(nPower > 0);
    sal_Int32 nResult = 1;
    while (nPower--)
        nResult *= 10;
    return nResult;
}

}

sal_Int32 ParseMathMLUnsignedNumber(const OUString &rStr, Fraction *pUN)
{
    assert(pUN);
    auto nLen = rStr.getLength();
    sal_Int32 nDecimalPoint = -1;
    sal_Int32 nIdx;
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
    }
    if (nIdx == 0 || ((nIdx == 1 || nIdx == 11) && nDecimalPoint == 0))
        return -1;
    if (nDecimalPoint == -1)
    {
        assert(nIdx > 0);
        *pUN = Fraction(rStr.copy(0, nIdx).toInt32(), 1);
        return nIdx;
    }
    if (nDecimalPoint == 0)
    {
        assert(nIdx > 1);
        *pUN = Fraction(rStr.copy(1, nIdx-1).toInt32(), lcl_GetPowerOf10(nIdx-1));
        return nIdx;
    }
    assert(0 < nDecimalPoint);
    assert(nDecimalPoint < nIdx);
    *pUN = Fraction(rStr.copy(0, nDecimalPoint).toInt32(), 1);
    if (++nDecimalPoint < nIdx)
        *pUN += Fraction(rStr.copy(nDecimalPoint, nIdx-nDecimalPoint).toInt32(),
                         lcl_GetPowerOf10(nIdx-nDecimalPoint));
    return nIdx;
}

sal_Int32 ParseMathMLNumber(const OUString &rStr, Fraction *pN)
{
    assert(pN);
    if (rStr.isEmpty())
        return -1;
    bool bNegative = (rStr[0] == '-');
    sal_Int32 nOffset = bNegative ? 1 : 0;
    Fraction aF;
    auto nIdx = ParseMathMLUnsignedNumber(rStr.copy(nOffset), &aF);
    if (nIdx <= 0)
        return -1;
    if (bNegative)
        *pN = Fraction(aF.GetNumerator(), aF.GetDenominator());
    else
        *pN = aF;
    return nOffset + nIdx;
}

sal_Int32 ParseMathMLAttributeLengthValue(const OUString &rStr, MathMLAttributeLengthValue *pV)
{
    assert(pV);
    auto nIdx = ParseMathMLNumber(rStr, &pV->aNumber);
    if (nIdx <= 0)
        return -1;
    OUString sRest = rStr.copy(nIdx);
    if (sRest.isEmpty())
    {
        pV->eUnit = MathMLLengthUnit::None;
        return nIdx;
    }
    if (sRest.startsWith("em"))
    {
        pV->eUnit = MathMLLengthUnit::Em;
        return nIdx + 2;
    }
    if (sRest.startsWith("ex"))
    {
        pV->eUnit = MathMLLengthUnit::Ex;
        return nIdx + 2;
    }
    if (sRest.startsWith("px"))
    {
        pV->eUnit = MathMLLengthUnit::Px;
        return nIdx + 2;
    }
    if (sRest.startsWith("in"))
    {
        pV->eUnit = MathMLLengthUnit::In;
        return nIdx + 2;
    }
    if (sRest.startsWith("cm"))
    {
        pV->eUnit = MathMLLengthUnit::Cm;
        return nIdx + 2;
    }
    if (sRest.startsWith("mm"))
    {
        pV->eUnit = MathMLLengthUnit::Mm;
        return nIdx + 2;
    }
    if (sRest.startsWith("pt"))
    {
        pV->eUnit = MathMLLengthUnit::Pt;
        return nIdx + 2;
    }
    if (sRest.startsWith("pc"))
    {
        pV->eUnit = MathMLLengthUnit::Pc;
        return nIdx + 2;
    }
    if (sRest[0] == u'%')
    {
        pV->eUnit = MathMLLengthUnit::Percent;
        return nIdx + 2;
    }
    return nIdx;
}


bool GetMathMLMathvariantValue(const OUString &rStr, MathMLMathvariantValue *pV)
{
    static const std::unordered_map<OUString, MathMLMathvariantValue> aMap{
        {"normal", MathMLMathvariantValue::Normal},
        {"bold", MathMLMathvariantValue::Bold},
        {"italic", MathMLMathvariantValue::Italic},
        {"bold-italic", MathMLMathvariantValue::BoldItalic},
        {"double-struck", MathMLMathvariantValue::DoubleStruck},
        {"bold-fraktur", MathMLMathvariantValue::BoldFraktur},
        {"script", MathMLMathvariantValue::Script},
        {"bold-script", MathMLMathvariantValue::BoldScript},
        {"fraktur", MathMLMathvariantValue::Fraktur},
        {"sans-serif", MathMLMathvariantValue::SansSerif},
        {"bold-sans-serif", MathMLMathvariantValue::BoldSansSerif},
        {"sans-serif-italic", MathMLMathvariantValue::SansSerifItalic},
        {"sans-serif-bold-italic", MathMLMathvariantValue::SansSerifBoldItalic},
        {"monospace", MathMLMathvariantValue::Monospace},
        {"initial", MathMLMathvariantValue::Initial},
        {"tailed", MathMLMathvariantValue::Tailed},
        {"looped", MathMLMathvariantValue::Looped},
        {"stretched", MathMLMathvariantValue::Stretched}
    };

    assert(pV);
    auto it = aMap.find(rStr);
    if (it != aMap.end())
    {
        *pV = it->second;
        return true;
    }
    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
