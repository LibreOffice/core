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

#include <sal/config.h>
#include <algorithm>
#include <float.h>
#include <basegfx/color/bcolormodifier.hxx>
#include <comphelper/random.hxx>

namespace basegfx
{
    BColorModifier::~BColorModifier()
    {
    }

    BColorModifier_gray::~BColorModifier_gray()
    {
    }

    bool BColorModifier_gray::operator==(const BColorModifier& rCompare) const
    {
        return dynamic_cast< const BColorModifier_gray* >(&rCompare) != nullptr;
    }

    ::basegfx::BColor BColorModifier_gray::getModifiedColor(const ::basegfx::BColor& aSourceColor) const
    {
        const double fLuminance(aSourceColor.luminance());

        return ::basegfx::BColor(fLuminance, fLuminance, fLuminance);
    }

    OUString BColorModifier_gray::getModifierName() const
    {
        return u"gray"_ustr;
    }

    BColorModifier_invert::~BColorModifier_invert()
    {
    }

    bool BColorModifier_invert::operator==(const BColorModifier& rCompare) const
    {
        return dynamic_cast< const BColorModifier_invert* >(&rCompare) != nullptr;
    }

    ::basegfx::BColor BColorModifier_invert::getModifiedColor(const ::basegfx::BColor& aSourceColor) const
    {
        return ::basegfx::BColor(1.0 - aSourceColor.getRed(), 1.0 - aSourceColor.getGreen(), 1.0 - aSourceColor.getBlue());
    }

    OUString BColorModifier_invert::getModifierName() const
    {
        return u"invert"_ustr;
    }

    BColorModifier_luminance_to_alpha::~BColorModifier_luminance_to_alpha()
    {
    }

    bool BColorModifier_luminance_to_alpha::operator==(const BColorModifier& rCompare) const
    {
        return dynamic_cast< const BColorModifier_luminance_to_alpha* >(&rCompare) != nullptr;
    }

    ::basegfx::BColor BColorModifier_luminance_to_alpha::getModifiedColor(const ::basegfx::BColor& aSourceColor) const
    {
        const double fAlpha(1.0 - ((aSourceColor.getRed() * 0.2125) + (aSourceColor.getGreen() * 0.7154) + (aSourceColor.getBlue() * 0.0721)));

        return ::basegfx::BColor(fAlpha, fAlpha, fAlpha);
    }

    OUString BColorModifier_luminance_to_alpha::getModifierName() const
    {
        return u"luminance_to_alpha"_ustr;
    }

    BColorModifier_replace::~BColorModifier_replace()
    {
    }

    bool BColorModifier_replace::operator==(const BColorModifier& rCompare) const
    {
        const BColorModifier_replace* pCompare = dynamic_cast< const BColorModifier_replace* >(&rCompare);

        if(!pCompare)
        {
            return false;
        }

        return getBColor() == pCompare->getBColor();
    }

    ::basegfx::BColor BColorModifier_replace::getModifiedColor(const ::basegfx::BColor& /*aSourceColor*/) const
    {
        return maBColor;
    }

    OUString BColorModifier_replace::getModifierName() const
    {
        return u"replace"_ustr;
    }

    BColorModifier_interpolate::~BColorModifier_interpolate()
    {
    }

    bool BColorModifier_interpolate::operator==(const BColorModifier& rCompare) const
    {
        const BColorModifier_interpolate* pCompare = dynamic_cast< const BColorModifier_interpolate* >(&rCompare);

        if(!pCompare)
        {
            return false;
        }

        return maBColor == pCompare->maBColor && mfValue == pCompare->mfValue;
    }

    ::basegfx::BColor BColorModifier_interpolate::getModifiedColor(const ::basegfx::BColor& aSourceColor) const
    {
        return interpolate(maBColor, aSourceColor, mfValue);
    }

    OUString BColorModifier_interpolate::getModifierName() const
    {
        return u"interpolate"_ustr;
    }

    BColorModifier_matrix::~BColorModifier_matrix()
    {
    }

    bool BColorModifier_matrix::operator==(const BColorModifier& rCompare) const
    {
        const BColorModifier_matrix* pCompare = dynamic_cast< const BColorModifier_matrix* >(&rCompare);

        if(!pCompare)
        {
            return false;
        }

        return maVector == pCompare->maVector;
    }

    ::basegfx::BColor BColorModifier_matrix::getModifiedColor(const ::basegfx::BColor& aSourceColor) const
    {
        if (maVector.size() != 20)
            return aSourceColor;

        const double aRed = maVector[0] * aSourceColor.getRed()
            + maVector[1] * aSourceColor.getGreen()
            + maVector[2] * aSourceColor.getBlue()
            + maVector[3] * 1.0
            + maVector[4];
        const double aGreen = maVector[5] * aSourceColor.getRed()
            + maVector[6] * aSourceColor.getGreen()
            + maVector[7] * aSourceColor.getBlue()
            + maVector[8] * 1.0
            + maVector[9];
        const double aBlue = maVector[10] * aSourceColor.getRed()
            + maVector[11] * aSourceColor.getGreen()
            + maVector[12] * aSourceColor.getBlue()
            + maVector[13] * 1.0
            + maVector[14];
        /*TODO: add support for alpha
        const double aAlpha = maVector[15] * aSourceColor.getRed()
            + maVector[16] * aSourceColor.getGreen()
            + maVector[17] * aSourceColor.getBlue()
            + maVector[18] * 1.0
            + maVector[19]);
        */

        return ::basegfx::BColor(
                std::clamp(aRed, 0.0, 1.0),
                std::clamp(aGreen, 0.0, 1.0),
                std::clamp(aBlue, 0.0, 1.0));
    }

    OUString BColorModifier_matrix::getModifierName() const
    {
        return u"matrix"_ustr;
    }

    BColorModifier_saturate::BColorModifier_saturate(double fValue)
    {
        maSatMatrix.set(0, 0, 0.213 + 0.787 * fValue);
        maSatMatrix.set(0, 1, 0.715 - 0.715 * fValue);
        maSatMatrix.set(0, 2, 0.072 - 0.072 * fValue);
        maSatMatrix.set(1, 0, 0.213 - 0.213 * fValue);
        maSatMatrix.set(1, 1, 0.715 + 0.285 * fValue);
        maSatMatrix.set(1, 2, 0.072 - 0.072 * fValue);
        maSatMatrix.set(2, 0, 0.213 - 0.213 * fValue);
        maSatMatrix.set(2, 1, 0.715 - 0.715 * fValue);
        maSatMatrix.set(2, 2, 0.072 + 0.928 * fValue);
    }

    BColorModifier_saturate::~BColorModifier_saturate()
    {
    }

    bool BColorModifier_saturate::operator==(const BColorModifier& rCompare) const
    {
        const BColorModifier_saturate* pCompare = dynamic_cast< const BColorModifier_saturate* >(&rCompare);

        if(!pCompare)
        {
            return false;
        }

        return maSatMatrix == pCompare->maSatMatrix;
    }

    ::basegfx::BColor BColorModifier_saturate::getModifiedColor(const ::basegfx::BColor& aSourceColor) const
    {
        basegfx::B3DHomMatrix aColorMatrix;
        aColorMatrix.set(0, 0, aSourceColor.getRed());
        aColorMatrix.set(1, 0, aSourceColor.getGreen());
        aColorMatrix.set(2, 0, aSourceColor.getBlue());

        aColorMatrix = maSatMatrix * aColorMatrix;
        return ::basegfx::BColor(aColorMatrix.get(0, 0), aColorMatrix.get(1, 0), aColorMatrix.get(2, 0));
    }

    OUString BColorModifier_saturate::getModifierName() const
    {
        return u"saturate"_ustr;
    }

    BColorModifier_hueRotate::BColorModifier_hueRotate(double fRad)
    {
        const double fCos = cos(fRad);
        const double fSin = sin(fRad);

        maHueMatrix.set(0, 0, 0.213 + fCos * 0.787 - fSin * 0.213);
        maHueMatrix.set(0, 1, 0.715 - fCos * 0.715 - fSin * 0.715);
        maHueMatrix.set(0, 2, 0.072 - fCos * 0.072 + fSin * 0.928);
        maHueMatrix.set(1, 0, 0.213 - fCos * 0.213 + fSin * 0.143);
        maHueMatrix.set(1, 1, 0.715 + fCos * 0.285 + fSin * 0.140);
        maHueMatrix.set(1, 2, 0.072 - fCos * 0.072 - fSin * 0.283);
        maHueMatrix.set(2, 0, 0.213 - fCos * 0.213 - fSin * 0.787);
        maHueMatrix.set(2, 1, 0.715 - fCos * 0.715 + fSin * 0.715);
        maHueMatrix.set(2, 2, 0.072 + fCos * 0.928 + fSin * 0.072);
    }

    BColorModifier_hueRotate::~BColorModifier_hueRotate()
    {
    }

    bool BColorModifier_hueRotate::operator==(const BColorModifier& rCompare) const
    {
        const BColorModifier_hueRotate* pCompare = dynamic_cast< const BColorModifier_hueRotate* >(&rCompare);

        if(!pCompare)
        {
            return false;
        }

        return maHueMatrix == pCompare->maHueMatrix;
    }

    ::basegfx::BColor BColorModifier_hueRotate::getModifiedColor(const ::basegfx::BColor& aSourceColor) const
    {
        basegfx::B3DHomMatrix aColorMatrix;
        aColorMatrix.set(0, 0, aSourceColor.getRed());
        aColorMatrix.set(1, 0, aSourceColor.getGreen());
        aColorMatrix.set(2, 0, aSourceColor.getBlue());

        aColorMatrix = maHueMatrix * aColorMatrix;
        return ::basegfx::BColor(
                std::clamp(aColorMatrix.get(0, 0), 0.0, 1.0),
                std::clamp(aColorMatrix.get(1, 0), 0.0, 1.0),
                std::clamp(aColorMatrix.get(2, 0), 0.0, 1.0));
    }

    OUString BColorModifier_hueRotate::getModifierName() const
    {
        return u"hueRotate"_ustr;
    }

    BColorModifier_black_and_white::~BColorModifier_black_and_white()
    {
    }

    bool BColorModifier_black_and_white::operator==(const BColorModifier& rCompare) const
    {
        const BColorModifier_black_and_white* pCompare = dynamic_cast< const BColorModifier_black_and_white* >(&rCompare);

        if(!pCompare)
        {
            return false;
        }

        return mfValue == pCompare->mfValue;
    }

    ::basegfx::BColor BColorModifier_black_and_white::getModifiedColor(const ::basegfx::BColor& aSourceColor) const
    {
        const double fLuminance(aSourceColor.luminance());

        if(fLuminance < mfValue)
        {
            return ::basegfx::BColor::getEmptyBColor();
        }
        else
        {
            return ::basegfx::BColor(1.0, 1.0, 1.0);
        }
    }

    OUString BColorModifier_black_and_white::getModifierName() const
    {
        return u"black_and_white"_ustr;
    }

    BColorModifier_gamma::BColorModifier_gamma(double fValue)
    :   mfValue(fValue),
        mfInvValue(fValue),
        mbUseIt(!basegfx::fTools::equal(fValue, 1.0) && fValue > 0.0 && basegfx::fTools::lessOrEqual(fValue, 10.0))
    {
        if(mbUseIt)
        {
            mfInvValue = 1.0 / mfValue;
        }
    }

    BColorModifier_gamma::~BColorModifier_gamma()
    {
    }

    bool BColorModifier_gamma::operator==(const BColorModifier& rCompare) const
    {
        const BColorModifier_gamma* pCompare = dynamic_cast< const BColorModifier_gamma* >(&rCompare);

        if(!pCompare)
        {
            return false;
        }

        // getValue is sufficient, mfInvValue and mbUseIt are only helper values
        return mfValue == pCompare->mfValue;
    }

    ::basegfx::BColor BColorModifier_gamma::getModifiedColor(const ::basegfx::BColor& aSourceColor) const
    {
        if(mbUseIt)
        {
            ::basegfx::BColor aRetval(
                pow(aSourceColor.getRed(), mfInvValue),
                pow(aSourceColor.getGreen(), mfInvValue),
                pow(aSourceColor.getBlue(), mfInvValue));

            aRetval.clamp();
            return aRetval;
        }
        else
        {
            return aSourceColor;
        }
    }

    OUString BColorModifier_gamma::getModifierName() const
    {
        return u"gamma"_ustr;
    }

    BColorModifier_RGBLuminanceContrast::BColorModifier_RGBLuminanceContrast(double fRed, double fGreen, double fBlue, double fLuminance, double fContrast)
    :   mfRed(std::clamp(fRed, -1.0, 1.0)),
        mfGreen(std::clamp(fGreen, -1.0, 1.0)),
        mfBlue(std::clamp(fBlue, -1.0, 1.0)),
        mfLuminance(std::clamp(fLuminance, -1.0, 1.0)),
        mfContrast(std::clamp(fContrast, -1.0, 1.0)),
        mfContrastOff(1.0),
        mfRedOff(0.0),
        mfGreenOff(0.0),
        mfBlueOff(0.0),
        mbUseIt(false)
    {
        if(basegfx::fTools::equalZero(mfRed)
            && basegfx::fTools::equalZero(mfGreen)
            && basegfx::fTools::equalZero(mfBlue)
            && basegfx::fTools::equalZero(mfLuminance)
            && basegfx::fTools::equalZero(mfContrast))
            return;

        // calculate slope
        if(mfContrast >= 0.0)
        {
            mfContrastOff = 128.0 / (128.0 - (mfContrast * 127.0));
        }
        else
        {
            mfContrastOff = ( 128.0 + (mfContrast * 127.0)) / 128.0;
        }

        // calculate unified contrast offset
        const double fPreparedContrastOff((128.0 - mfContrastOff * 128.0) / 255.0);
        const double fCombinedOffset(mfLuminance + fPreparedContrastOff);

        // set full offsets
        mfRedOff = mfRed + fCombinedOffset;
        mfGreenOff = mfGreen + fCombinedOffset;
        mfBlueOff = mfBlue + fCombinedOffset;

        mbUseIt = true;
    }

    BColorModifier_RGBLuminanceContrast::~BColorModifier_RGBLuminanceContrast()
    {
    }

    bool BColorModifier_RGBLuminanceContrast::operator==(const BColorModifier& rCompare) const
    {
        const BColorModifier_RGBLuminanceContrast* pCompare = dynamic_cast< const BColorModifier_RGBLuminanceContrast* >(&rCompare);

        if(!pCompare)
        {
            return false;
        }

        // no need to compare other values, these are just helpers
        return mfRed == pCompare->mfRed
            && mfGreen == pCompare->mfGreen
            && mfBlue == pCompare->mfBlue
            && mfLuminance == pCompare->mfLuminance
            && mfContrast == pCompare->mfContrast;
    }

    ::basegfx::BColor BColorModifier_RGBLuminanceContrast::getModifiedColor(const ::basegfx::BColor& aSourceColor) const
    {
        if(mbUseIt)
        {
            return basegfx::BColor(
                std::clamp(aSourceColor.getRed() * mfContrastOff + mfRedOff, 0.0, 1.0),
                std::clamp(aSourceColor.getGreen() * mfContrastOff + mfGreenOff, 0.0, 1.0),
                std::clamp(aSourceColor.getBlue() * mfContrastOff + mfBlueOff, 0.0, 1.0));
        }
        else
        {
            return aSourceColor;
        }
    }

    OUString BColorModifier_RGBLuminanceContrast::getModifierName() const
    {
        return u"RGBLuminanceContrast"_ustr;
    }

    BColorModifier_randomize::BColorModifier_randomize(double fRandomPart)
    : mfRandomPart(fRandomPart)
    {
    }

    BColorModifier_randomize::~BColorModifier_randomize()
    {
    }

    // compare operator
    bool BColorModifier_randomize::operator==(const BColorModifier& rCompare) const
    {
        const BColorModifier_randomize* pCompare = dynamic_cast< const BColorModifier_randomize* >(&rCompare);

        if(!pCompare)
        {
            return false;
        }

        return mfRandomPart == pCompare->mfRandomPart;
    }

    // compute modified color
    ::basegfx::BColor BColorModifier_randomize::getModifiedColor(const ::basegfx::BColor& aSourceColor) const
    {
        if(0.0 >= mfRandomPart)
        {
            // no randomizing, use orig color
            return aSourceColor;
        }

        if(1.0 <= mfRandomPart)
        {
            // full randomized color
            return basegfx::BColor(
                    comphelper::rng::uniform_real_distribution(0.0, nextafter(1.0, DBL_MAX)),
                    comphelper::rng::uniform_real_distribution(0.0, nextafter(1.0, DBL_MAX)),
                    comphelper::rng::uniform_real_distribution(0.0, nextafter(1.0, DBL_MAX)));
        }

        // mixed color
        const double fMulA(1.0 - mfRandomPart);
        return basegfx::BColor(
            aSourceColor.getRed() * fMulA +
                comphelper::rng::uniform_real_distribution(0.0, nextafter(mfRandomPart, DBL_MAX)),
            aSourceColor.getGreen() * fMulA +
                comphelper::rng::uniform_real_distribution(0.0, nextafter(mfRandomPart, DBL_MAX)),
            aSourceColor.getBlue() * fMulA +
                comphelper::rng::uniform_real_distribution(0.0, nextafter(mfRandomPart, DBL_MAX)));
    }

    OUString BColorModifier_randomize::getModifierName() const
    {
        return u"randomize"_ustr;
    }

    ::basegfx::BColor BColorModifierStack::getModifiedColor(const ::basegfx::BColor& rSource) const
    {
        if(maBColorModifiers.empty())
        {
            return rSource;
        }

        ::basegfx::BColor aRetval(rSource);

        for(sal_uInt32 a(maBColorModifiers.size()); a;)
        {
            a--;
            aRetval = maBColorModifiers[a]->getModifiedColor(aRetval);
        }

        return aRetval;
    }
} // end of namespace basegfx

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
