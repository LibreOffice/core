/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <tools/time.hxx>

class DateTime;

namespace tools
{
/** Duration in days and time. Can be negative in which case days is 0 and time
    is negative or both days and time are negative.
*/
class SAL_WARN_UNUSED TOOLS_DLLPUBLIC Duration
{
public:
    Duration() {}

    /** Assumes that DateTime are normalized and there are no Time out-of-range
        field values. */
    Duration(const ::DateTime& rStart, const ::DateTime& rEnd);

    /** Time can be a limited duration as well. We don't cater for out-of-range
        minutes and seconds values here though. */
    Duration(const Time& rStart, const Time& rEnd);

    /** Difference in days, like DateTime()-DateTime(). */
    explicit Duration(double fTimeInDays);

    bool IsNegative() const { return mnDays < 0 || maTime.GetTime() < 0; }
    sal_Int32 GetDays() const { return mnDays; }
    const Time& GetTime() const { return maTime; }
    double GetInDays() const { return static_cast<double>(GetDays()) + GetTime().GetTimeInDays(); }

    /** Whether a duration is set. */
    operator bool() const { return maTime.GetTime() != 0 || mnDays != 0; }

    /** Unary minus. */
    Duration operator-() const;

    /** Add a duration to this instance. */
    Duration& Add(const Duration& rDuration, bool& rbOverflow);

    /** Get multiple of duration. */
    Duration Mult(sal_Int32 nMult, bool& rbOverflow) const;

private:
    /** Internal days and Time values. */
    Duration(sal_Int32 nDays, sal_Int64 nTime);

    /** Prerequisite: mnDays is already correctly set and absolute value of
        nanoseconds less than one day. */
    void ApplyTime(sal_Int64 nNS);

    /** Prerequisite: mnDays is already correctly set and Time hour values
        are adjusted. */
    void SetTimeDiff(const Time& rStart, const Time& rEnd);

private:
    Time maTime = Time(0);
    sal_Int32 mnDays = 0;
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
