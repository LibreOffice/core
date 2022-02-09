/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <rtl/math.hxx>
#include <cmath>

/**
  * This class provides LO with Kahan summation algorithm
  * About this algorithm: https://en.wikipedia.org/wiki/Kahan_summation_algorithm
  * For general purpose software we assume first order error is enough.
  *
  * Additionally queue and remember the last recent non-zero value and add it
  * similar to approxAdd() when obtaining the final result to further eliminate
  * accuracy errors. (e.g. for the dreaded 0.1 + 0.2 - 0.3 != 0.0)
  */

class KahanSum
{
public:
    constexpr KahanSum() = default;

    constexpr KahanSum(double x_0)
        : m_fSum(x_0)
    {
    }

    constexpr KahanSum(double x_0, double err_0)
        : m_fSum(x_0)
        , m_fError(err_0)
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
        if (x_i == 0.0)
            return;

        if (!m_fMem)
        {
            m_fMem = x_i;
            return;
        }

        double t = m_fSum + m_fMem;
        if (std::abs(m_fSum) >= std::abs(m_fMem))
            m_fError += (m_fSum - t) + m_fMem;
        else
            m_fError += (m_fMem - t) + m_fSum;
        m_fSum = t;
        m_fMem = x_i;
    }

    /**
      * Adds a value to the sum using Kahan summation.
      * @param fSum
      */
    inline void add(const KahanSum& fSum)
    {
        add(fSum.m_fSum);
        add(fSum.m_fError);
        add(fSum.m_fMem);
    }

    /**
      * Substracts a value to the sum using Kahan summation.
      * @param fSum
      */
    inline void subtract(const KahanSum& fSum)
    {
        add(-fSum.m_fSum);
        add(-fSum.m_fError);
        add(-fSum.m_fMem);
    }

public:
    constexpr KahanSum operator-() const
    {
        KahanSum fKahanSum;
        fKahanSum.m_fSum = -m_fSum;
        fKahanSum.m_fError = -m_fError;
        fKahanSum.m_fMem = -m_fMem;
        return fKahanSum;
    }

    constexpr KahanSum& operator=(double fSum)
    {
        m_fSum = fSum;
        m_fError = 0;
        m_fMem = 0;
        return *this;
    }

    constexpr KahanSum& operator=(const KahanSum& fSum) = default;

    inline void operator+=(const KahanSum& fSum) { add(fSum); }

    inline void operator+=(double fSum) { add(fSum); }

    inline void operator-=(const KahanSum& fSum) { subtract(fSum); }

    inline void operator-=(double fSum) { add(-fSum); }

    inline KahanSum operator+(double fSum) const
    {
        KahanSum fNSum(*this);
        fNSum.add(fSum);
        return fNSum;
    }

    inline KahanSum operator+(const KahanSum& fSum) const
    {
        KahanSum fNSum(*this);
        fNSum += fSum;
        return fNSum;
    }

    inline KahanSum operator-(double fSum) const
    {
        KahanSum fNSum(*this);
        fNSum.add(-fSum);
        return fNSum;
    }

    inline KahanSum operator-(const KahanSum& fSum) const
    {
        KahanSum fNSum(*this);
        fNSum -= fSum;
        return fNSum;
    }

    /**
      * In some parts of the code of interpr_.cxx this may be used for
      * product instead of sum. This operator shall be used for that task.
      */
    constexpr void operator*=(double fTimes)
    {
        if (m_fMem)
        {
            m_fSum = get() * fTimes;
            m_fMem = 0.0;
        }
        else
        {
            m_fSum = (m_fSum + m_fError) * fTimes;
        }
        m_fError = 0.0;
    }

    constexpr void operator/=(double fDivides)
    {
        if (m_fMem)
        {
            m_fSum = get() / fDivides;
            m_fMem = 0.0;
        }
        else
        {
            m_fSum = (m_fSum + m_fError) / fDivides;
        }
        m_fError = 0.0;
    }

    inline KahanSum operator*(const KahanSum& fTimes) const { return get() * fTimes.get(); }

    inline KahanSum operator*(double fTimes) const { return get() * fTimes; }

    inline KahanSum operator/(const KahanSum& fDivides) const { return get() / fDivides.get(); }

    inline KahanSum operator/(double fDivides) const { return get() / fDivides; }

    inline bool operator<(const KahanSum& fSum) const { return get() < fSum.get(); }

    inline bool operator<(double fSum) const { return get() < fSum; }

    inline bool operator>(const KahanSum& fSum) const { return get() > fSum.get(); }

    inline bool operator>(double fSum) const { return get() > fSum; }

    inline bool operator<=(const KahanSum& fSum) const { return get() <= fSum.get(); }

    inline bool operator<=(double fSum) const { return get() <= fSum; }

    inline bool operator>=(const KahanSum& fSum) const { return get() >= fSum.get(); }

    inline bool operator>=(double fSum) const { return get() >= fSum; }

    inline bool operator==(const KahanSum& fSum) const { return get() == fSum.get(); }

    inline bool operator!=(const KahanSum& fSum) const { return get() != fSum.get(); }

public:
    /**
      * Returns the final sum.
      * @return final sum
      */
    double get() const
    {
        const double fTotal = m_fSum + m_fError;
        if (!m_fMem)
            return fTotal;

        // Check the same condition as rtl::math::approxAdd() and if true
        // return 0.0, if false use another Kahan summation adding m_fMem.
        if (((m_fMem < 0.0 && fTotal > 0.0) || (fTotal < 0.0 && m_fMem > 0.0))
            && rtl::math::approxEqual(m_fMem, -fTotal))
        {
            /* TODO: should we reset all values to zero here for further
             * summation, or is it better to keep them as they are? */
            return 0.0;
        }

        // The actual argument passed to add() here does not matter as long as
        // it is not 0, m_fMem is not 0 and will be added anyway, see add().
        const_cast<KahanSum*>(this)->add(m_fMem);
        const_cast<KahanSum*>(this)->m_fMem = 0.0;
        return m_fSum + m_fError;
    }

private:
    double m_fSum = 0;
    double m_fError = 0;
    double m_fMem = 0;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
