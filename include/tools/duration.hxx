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

    constexpr static sal_uInt64 kAccuracyEpsilonNanoseconds = 300;
    constexpr static sal_uInt64 kAccuracyEpsilonNanosecondsMicroseconds = 999;

    /** Difference in days, like DateTime()-DateTime().

        Can also be used to round a date+time value to, for example, microseconds.

        @param  nAccuracyEpsilonNanoseconds
                Round for example by 1 nanosecond if it's just 1 off to a
                second,  i.e. 0999999999 or 0000000001. This can be loosened if
                necessary. For example, if fTimeInDays is a date+time in
                "today's" range with a significant seconds resolution, an
                accuracy epsilon (=unsharpness) of ~300 is required. Hence default.
                Must be 0 <= nAccuracyEpsilonNanoseconds <= Time::nanoSecPerSec - 1.
     */
    explicit Duration(double fTimeInDays,
                      sal_uInt64 nAccuracyEpsilonNanoseconds = kAccuracyEpsilonNanoseconds);

    /** Time can be a limited duration as well and can have out-of-range
        values, it will be normalized. Sign of both days and Time must be equal
        unless one is 0. */
    Duration(sal_Int32 nDays, const Time& rTime);

    /** Individual time values can be out-of-range, all will be normalized.
        Additionally, the resulting time overall hour value is not restricted
        to sal_uInt16 like it is with Time, as values >=24 flow over into days.
        For a negative duration only a negative nDays can be given, thus a
        negative duration of less than one day is not possible. */
    Duration(sal_Int32 nDays, sal_uInt32 nHours, sal_uInt32 nMinutes, sal_uInt32 nSeconds,
             sal_uInt64 nNanoseconds);

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

    /** Prerequisite: mnDays is already set. */
    void Normalize(sal_uInt64 nHours, sal_uInt64 nMinutes, sal_uInt64 nSeconds,
                   sal_uInt64 nNanoseconds, bool bNegative);

    /** Prerequisite: mnDays is already correctly set and absolute value of
        nanoseconds less than one day. */
    void ApplyTime(sal_Int64 nNS);

    /** Prerequisite: mnDays is already correctly set and Time hour values
        are adjusted. */
    void SetTimeDiff(const Time& rStart, const Time& rEnd);

private:
    Time maTime = Time(Time::EMPTY);
    sal_Int32 mnDays = 0;
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
