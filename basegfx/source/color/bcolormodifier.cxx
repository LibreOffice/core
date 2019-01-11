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

#include <basegfx/color/bcolormodifier.hxx>

namespace basegfx
{
    BColorModifier::~BColorModifier()
    {
    }
} // end of namespace basegfx

namespace basegfx
{
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
} // end of namespace basegfx

namespace basegfx
{
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
} // end of namespace basegfx

namespace basegfx
{
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
} // end of namespace basegfx

namespace basegfx
{
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
} // end of namespace basegfx

namespace basegfx
{
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

        return getBColor() == pCompare->getBColor() && getValue() == pCompare->getValue();
    }

    ::basegfx::BColor BColorModifier_interpolate::getModifiedColor(const ::basegfx::BColor& aSourceColor) const
    {
        return interpolate(maBColor, aSourceColor, mfValue);
    }
} // end of namespace basegfx

namespace basegfx
{
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

        return getValue() == pCompare->getValue();
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
} // end of namespace basegfx

namespace basegfx
{
    BColorModifier_gamma::BColorModifier_gamma(double fValue)
    :   BColorModifier(),
        mfValue(fValue),
        mfInvValue(fValue),
        mbUseIt(!basegfx::fTools::equal(fValue, 1.0) && basegfx::fTools::more(fValue, 0.0) && basegfx::fTools::lessOrEqual(fValue, 10.0))
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
        return getValue() == pCompare->getValue();
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
} // end of namespace basegfx

namespace basegfx
{
    BColorModifier_RGBLuminanceContrast::BColorModifier_RGBLuminanceContrast(double fRed, double fGreen, double fBlue, double fLuminance, double fContrast)
    :   BColorModifier(),
        mfRed(std::clamp(fRed, -1.0, 1.0)),
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
        if(!basegfx::fTools::equalZero(mfRed)
            || !basegfx::fTools::equalZero(mfGreen)
            || !basegfx::fTools::equalZero(mfBlue)
            || !basegfx::fTools::equalZero(mfLuminance)
            || !basegfx::fTools::equalZero(mfContrast))
        {
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
        return getRed() == pCompare->getRed()
            && getGreen() == pCompare->getGreen()
            && getBlue() == pCompare->getBlue()
            && getLuminance() == pCompare->getLuminance()
            && getContrast() == pCompare->getContrast();
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
} // end of namespace basegfx

namespace basegfx
{
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
