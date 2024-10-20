/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <tools/duration.hxx>
#include <tools/datetime.hxx>
#include <rtl/math.hxx>
#include <o3tl/safeint.hxx>
#include <cmath>

namespace tools
{
Duration::Duration(const ::DateTime& rStart, const ::DateTime& rEnd)
    : mnDays(static_cast<const Date&>(rEnd) - static_cast<const Date&>(rStart))
{
    SetTimeDiff(rStart, rEnd);
}

Duration::Duration(const Time& rStart, const Time& rEnd)
{
    const sal_uInt16 nStartHour = rStart.GetHour();
    const sal_uInt16 nEndHour = rEnd.GetHour();
    if (nStartHour >= 24 || nEndHour >= 24)
    {
        Time aEnd(rEnd);
        if (nEndHour >= 24)
        {
            mnDays = (nEndHour / 24) * (aEnd.GetTime() < 0 ? -1 : 1);
            aEnd.SetHour(nEndHour % 24);
        }
        Time aStart(rStart);
        if (nStartHour >= 24)
        {
            mnDays -= (nStartHour / 24) * (aStart.GetTime() < 0 ? -1 : 1);
            aStart.SetHour(nStartHour % 24);
        }
        SetTimeDiff(aStart, aEnd);
    }
    else
    {
        SetTimeDiff(rStart, rEnd);
    }
}

Duration::Duration(double fTimeInDays, sal_uInt64 nAccuracyEpsilonNanoseconds)
{
    assert(nAccuracyEpsilonNanoseconds <= Time::nanoSecPerSec - 1);
    double fInt, fFrac;
    if (fTimeInDays < 0.0)
    {
        fInt = ::rtl::math::approxCeil(fTimeInDays);
        fFrac = fInt <= fTimeInDays ? 0.0 : fTimeInDays - fInt;
    }
    else
    {
        fInt = ::rtl::math::approxFloor(fTimeInDays);
        fFrac = fInt >= fTimeInDays ? 0.0 : fTimeInDays - fInt;
    }
    mnDays = static_cast<sal_Int32>(fInt);
    if (fFrac)
    {
        fFrac *= Time::nanoSecPerDay;
        fFrac = ::rtl::math::approxFloor(fFrac);
        sal_Int64 nNS = static_cast<sal_Int64>(fFrac);
        const sal_Int64 nN = nNS % Time::nanoSecPerSec;
        if (nN)
        {
            const sal_uInt64 nA = std::abs(nN);
            if (nA <= nAccuracyEpsilonNanoseconds)
                nNS -= (nNS < 0) ? -nN : nN;
            else if (nA >= Time::nanoSecPerSec - nAccuracyEpsilonNanoseconds)
            {
                const sal_Int64 nD = Time::nanoSecPerSec - nA;
                nNS += (nNS < 0) ? -nD : nD;
                if (std::abs(nNS) >= Time::nanoSecPerDay)
                {
                    mnDays += nNS / Time::nanoSecPerDay;
                    nNS %= Time::nanoSecPerDay;
                }
            }
        }
        maTime.MakeTimeFromNS(nNS);
        assert(mnDays == 0 || maTime.GetTime() == 0 || (mnDays < 0) == (nNS < 0));
    }
}

Duration::Duration(sal_Int32 nDays, const Time& rTime)
    : mnDays(nDays)
{
    assert(nDays == 0 || rTime.GetTime() == 0 || (nDays < 0) == (rTime.GetTime() < 0));
    Normalize(rTime.GetHour(), rTime.GetMin(), rTime.GetSec(), rTime.GetNanoSec(),
              ((nDays < 0) || (rTime.GetTime() < 0)));
}

Duration::Duration(sal_Int32 nDays, sal_uInt32 nHours, sal_uInt32 nMinutes, sal_uInt32 nSeconds,
                   sal_uInt64 nNanoseconds)
    : mnDays(nDays)
{
    Normalize(nHours, nMinutes, nSeconds, nNanoseconds, nDays < 0);
}

Duration::Duration(sal_Int32 nDays, sal_Int64 nTime)
    : maTime(Time::fromEncodedTime(nTime))
    , mnDays(nDays)
{
}

void Duration::Normalize(sal_uInt64 nHours, sal_uInt64 nMinutes, sal_uInt64 nSeconds,
                         sal_uInt64 nNanoseconds, bool bNegative)
{
    if (nNanoseconds >= Time::nanoSecPerSec)
    {
        nSeconds += nNanoseconds / Time::nanoSecPerSec;
        nNanoseconds %= Time::nanoSecPerSec;
    }
    if (nSeconds >= Time::secondPerMinute)
    {
        nMinutes += nSeconds / Time::secondPerMinute;
        nSeconds %= Time::secondPerMinute;
    }
    if (nMinutes >= Time::minutePerHour)
    {
        nHours += nMinutes / Time::minutePerHour;
        nMinutes %= Time::minutePerHour;
    }
    if (nHours >= Time::hourPerDay)
    {
        sal_Int64 nDiff = nHours / Time::hourPerDay;
        nHours %= Time::hourPerDay;
        bool bOverflow = false;
        if (bNegative)
        {
            nDiff = -nDiff;
            bOverflow = (nDiff < SAL_MIN_INT32);
            bOverflow |= o3tl::checked_add(mnDays, static_cast<sal_Int32>(nDiff), mnDays);
            if (bOverflow)
                mnDays = SAL_MIN_INT32;
        }
        else
        {
            bOverflow = (nDiff > SAL_MAX_INT32);
            bOverflow |= o3tl::checked_add(mnDays, static_cast<sal_Int32>(nDiff), mnDays);
            if (bOverflow)
                mnDays = SAL_MAX_INT32;
        }
        assert(!bOverflow);
        if (bOverflow)
        {
            nHours = Time::hourPerDay - 1;
            nMinutes = Time::minutePerHour - 1;
            nSeconds = Time::secondPerMinute - 1;
            nNanoseconds = Time::nanoSecPerSec - 1;
        }
    }
    maTime = Time(nHours, nMinutes, nSeconds, nNanoseconds);
    if (bNegative)
        maTime = -maTime;
    assert(mnDays == 0 || maTime.GetTime() == 0 || (mnDays < 0) == (maTime.GetTime() < 0));
}

void Duration::ApplyTime(sal_Int64 nNS)
{
    if (mnDays > 0 && nNS < 0)
    {
        --mnDays;
        nNS = Time::nanoSecPerDay + nNS;
    }
    else if (mnDays < 0 && nNS > 0)
    {
        ++mnDays;
        nNS = -Time::nanoSecPerDay + nNS;
    }
    maTime.MakeTimeFromNS(nNS);
    assert(mnDays == 0 || maTime.GetTime() == 0 || (mnDays < 0) == (maTime.GetTime() < 0));
}

void Duration::SetTimeDiff(const Time& rStart, const Time& rEnd)
{
    const sal_Int64 nNS = rEnd.GetNSFromTime() - rStart.GetNSFromTime();
    ApplyTime(nNS);
}

Duration Duration::operator-() const
{
    Duration aD(-mnDays, -maTime.GetTime());
    return aD;
}

Duration& Duration::Add(const Duration& rDuration, bool& rbOverflow)
{
    rbOverflow = o3tl::checked_add(mnDays, rDuration.mnDays, mnDays);
    // Duration is always normalized, time values >= 24h don't occur.
    sal_Int64 nNS = maTime.GetNSFromTime() + rDuration.maTime.GetNSFromTime();
    if (nNS < -Time::nanoSecPerDay)
    {
        rbOverflow |= o3tl::checked_sub(mnDays, sal_Int32(1), mnDays);
        nNS += Time::nanoSecPerDay;
    }
    else if (nNS > Time::nanoSecPerDay)
    {
        rbOverflow |= o3tl::checked_add(mnDays, sal_Int32(1), mnDays);
        nNS -= Time::nanoSecPerDay;
    }
    ApplyTime(nNS);
    return *this;
}

Duration Duration::Mult(sal_Int32 nMult, bool& rbOverflow) const
{
    // First try a simple calculation in nanoseconds.
    bool bBadNS = false;
    sal_Int64 nNS;
    sal_Int64 nDays;
    if (o3tl::checked_multiply(static_cast<sal_Int64>(mnDays), static_cast<sal_Int64>(nMult), nDays)
        || o3tl::checked_multiply(nDays, Time::nanoSecPerDay, nDays)
        || o3tl::checked_multiply(maTime.GetNSFromTime(), static_cast<sal_Int64>(nMult), nNS)
        || o3tl::checked_add(nDays, nNS, nNS))
    {
        bBadNS = rbOverflow = true;
    }
    else
    {
        const sal_Int64 nD = nNS / Time::nanoSecPerDay;
        if (nD < SAL_MIN_INT32 || SAL_MAX_INT32 < nD)
            rbOverflow = true;
        else
        {
            rbOverflow = false;
            nNS -= nD * Time::nanoSecPerDay;
            Duration aD(static_cast<sal_Int32>(nD), 0);
            aD.ApplyTime(nNS);
            return aD;
        }
    }
    if (bBadNS)
    {
        // Simple calculation in overall nanoseconds overflowed, try with
        // individual components.
        const sal_uInt64 nMult64 = (nMult < 0) ? -nMult : nMult;
        do
        {
            sal_uInt64 nN;
            if (o3tl::checked_multiply(static_cast<sal_uInt64>(maTime.GetNanoSec()), nMult64, nN))
                break;
            sal_uInt64 nS;
            if (o3tl::checked_multiply(static_cast<sal_uInt64>(maTime.GetSec()), nMult64, nS))
                break;
            sal_uInt64 nM;
            if (o3tl::checked_multiply(static_cast<sal_uInt64>(maTime.GetMin()), nMult64, nM))
                break;
            sal_uInt64 nH;
            if (o3tl::checked_multiply(static_cast<sal_uInt64>(maTime.GetHour()), nMult64, nH))
                break;
            sal_uInt64 nD;
            if (o3tl::checked_multiply(
                    mnDays < 0 ? static_cast<sal_uInt64>(-static_cast<sal_Int64>(mnDays))
                               : static_cast<sal_uInt64>(mnDays),
                    nMult64, nD))
                break;
            if (nN > Time::nanoSecPerSec)
            {
                const sal_uInt64 nC = nN / Time::nanoSecPerSec;
                if (o3tl::checked_add(nS, nC, nS))
                    break;
                nN -= nC * Time::nanoSecPerSec;
            }
            if (nS > Time::secondPerMinute)
            {
                const sal_uInt64 nC = nS / Time::secondPerMinute;
                if (o3tl::checked_add(nM, nC, nM))
                    break;
                nS -= nC * Time::secondPerMinute;
            }
            if (nM > Time::minutePerHour)
            {
                const sal_uInt64 nC = nM / Time::minutePerHour;
                if (o3tl::checked_add(nH, nC, nH))
                    break;
                nM -= nC * Time::minutePerHour;
            }
            if (nH > Time::hourPerDay)
            {
                const sal_uInt64 nC = nH / Time::hourPerDay;
                if (o3tl::checked_add(nD, nC, nD))
                    break;
                nH -= nC * Time::hourPerDay;
            }
            if (IsNegative() ? (static_cast<sal_uInt64>(SAL_MAX_INT32) + 1) < nD
                                   || -static_cast<sal_Int64>(nD) < SAL_MIN_INT32
                             : SAL_MAX_INT32 < nD)
                break;

            rbOverflow = false;
            Time aTime(nH, nM, nS, nN);
            if (IsNegative() == (nMult < 0))
            {
                Duration aD(nD, aTime.GetTime());
                return aD;
            }
            else
            {
                Duration aD(-static_cast<sal_Int64>(nD), -aTime.GetTime());
                return aD;
            }
        } while (false);
    }
    assert(rbOverflow);
    if (IsNegative() == (nMult < 0))
    {
        Duration aD(SAL_MAX_INT32, 0);
        aD.ApplyTime(Time::nanoSecPerDay - 1);
        return aD;
    }
    else
    {
        Duration aD(SAL_MIN_INT32, 0);
        aD.ApplyTime(-(Time::nanoSecPerDay - 1));
        return aD;
    }
}
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
