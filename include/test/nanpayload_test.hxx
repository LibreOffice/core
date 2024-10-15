/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <string>

#include <sal/config.h>
#include <rtl/math.hxx>

class NanPayloadTest
{
private:
    bool m_bSupported = { false };
    std::string m_aMessage;

public:
    NanPayloadTest()
    {
        // Test that a quiet NaN payload is propagated and behaves as we
        // expect. Ideally that could be done with a constexpr in
        // sal/rtl/math.cxx to fail already during compile time instead of make
        // check, but..
        // See
        // https://grouper.ieee.org/groups/msc/ANSI_IEEE-Std-754-2019/background/nan-propagation.pdf
        double fVal1 = std::numeric_limits<double>::quiet_NaN();
        reinterpret_cast<sal_math_Double*>(&fVal1)->nan_parts.fraction_lo = 0xbeef;
        const double fVal2 = 0 + fVal1;
        if ((!std::isnan(fVal2))
            || (static_cast<sal_uInt32>(0xbeef)
                != static_cast<sal_uInt32>(
                       reinterpret_cast<const sal_math_Double*>(&fVal2)->nan_parts.fraction_lo)))
        {
            m_aMessage = "Your platform does not support propagation of NaN payloads.";
            return;
        }

        reinterpret_cast<sal_math_Double*>(&fVal1)->nan_parts.fraction_lo = 0xdead;
        const double fVal3 = fVal1 + fVal2;
        // Result is one of the payloaded NaNs but the standard does not
        // specify which.
        if (0xbeef != reinterpret_cast<const sal_math_Double*>(&fVal3)->nan_parts.fraction_lo
            && 0xdead != reinterpret_cast<const sal_math_Double*>(&fVal3)->nan_parts.fraction_lo)
        {
            m_aMessage = "Your platform does not support propagation of two combined NaN payloads.";
            return;
        }

        m_bSupported = true;
    }

    bool getIfSupported() { return m_bSupported; }

    std::string getMessage() { return m_aMessage; }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
