/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

/**
  * This class provides LO with Kahan summation algorithm
  * About this algorithm: https://en.wikipedia.org/wiki/Kahan_summation_algorithm
  * For general purpose software we assume first order error is enough.
  * This class could be made constexpr if needed.
  */

class KahanSum
{
public:
    constexpr KahanSum() = default;

    constexpr KahanSum(double x_0)
        : m_fSum(x_0)
    {
    }

    constexpr KahanSum(const KahanSum& fSum) = default;

public:
    /**
      * Adds a value to the sum using Kahan summation.
      * @param x_i
      */
    void add(double x_i)
    {
        double t = m_fSum + x_i;
        if (std::abs(m_fSum) >= std::abs(x_i))
            m_fError += (m_fSum - t) + x_i;
        else
            m_fError += (x_i - t) + m_fSum;
        m_fSum = t;
    }

    /**
      * Adds a value to the sum using Kahan summation.
      * @param fSum
      */
    inline void add(const KahanSum& fSum)
    {
        add(fSum.m_fSum);
        add(fSum.m_fError);
    }

public:
    constexpr KahanSum operator-() const
    {
        KahanSum fKahanSum;
        fKahanSum.m_fSum = -m_fSum;
        fKahanSum.m_fError = -m_fError;
        return fKahanSum;
    }

    constexpr KahanSum& operator=(double fSum)
    {
        m_fSum = fSum;
        m_fError = 0;
        return *this;
    }

    constexpr KahanSum& operator=(const KahanSum& fSum) = default;

    inline void operator+=(const KahanSum& fSum) { add(fSum); }

    inline void operator+=(double fSum) { add(fSum); }

    inline void operator-=(const KahanSum& fSum) { add(-fSum); }

    inline void operator-=(double fSum) { add(-fSum); }

    /**
      * In some parts of the code of interpr_.cxx this may be used for
      * product instead of sum. This operator shall be used for that task.
      */
    constexpr void operator*=(double fTimes)
    {
        m_fSum *= fTimes;
        m_fError *= fTimes;
    }

    constexpr void operator/=(double fDivides)
    {
        m_fSum /= fDivides;
        m_fError /= fDivides;
    }

    constexpr bool operator<(const KahanSum& fSum) const { return get() < fSum.get(); }

    constexpr bool operator<(double fSum) const { return get() < fSum; }

    constexpr bool operator>(const KahanSum& fSum) const { return get() > fSum.get(); }

    constexpr bool operator>(double fSum) const { return get() > fSum; }

    constexpr bool operator<=(const KahanSum& fSum) const { return get() <= fSum.get(); }

    constexpr bool operator<=(double fSum) const { return get() <= fSum; }

    constexpr bool operator>=(const KahanSum& fSum) const { return get() >= fSum.get(); }

    constexpr bool operator>=(double fSum) const { return get() >= fSum; }

    constexpr bool operator==(const KahanSum& fSum) const
    {
        return fSum.m_fSum == m_fSum && fSum.m_fError == m_fError;
    }

    constexpr bool operator!=(const KahanSum& fSum) const
    {
        return fSum.m_fSum != m_fSum || fSum.m_fError != m_fError;
    }

public:
    /**
      * Returns the final sum.
      * @return final sum
      */
    constexpr double get() const { return m_fSum + m_fError; }

private:
    double m_fSum = 0;
    double m_fError = 0;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
