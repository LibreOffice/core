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

/**
  * This class provides LO with Kahan summation algorithm
  * About this algorithm: https://en.wikipedia.org/wiki/Kahan_summation_algorithm
  * For general purpose software we assume second order error is enough.
  * This class could be made constexpr if needed.
  */

class ksum
{
private:
    double m_fSum;
    double m_fError;
    double m_fErrorError;

public:
    constexpr ksum()
        : m_fSum(0)
        , m_fError(0)
        , m_fErrorError(0)
    {
    }

    constexpr ksum(double x_0)
        : m_fSum(x_0)
        , m_fError(0)
        , m_fErrorError(0)
    {
    }

    constexpr ksum(const ksum& fSum)
        : m_fSum(fSum.m_fSum)
        , m_fError(fSum.m_fError)
        , m_fErrorError(fSum.m_fErrorError)
    {
    }

public:
    /**
      * Adds a value to the sum using Kahan sumation.
      * @param x_i
      */
    void add(double x_i)
    {
        double c;
        double cc;
        double t = m_fSum + x_i;
        if (std::abs(m_fSum) >= std::abs(x_i))
        {
            c = (m_fSum - t) + x_i;
        }
        else
        {
            c = (x_i - t) + m_fSum;
        }
        m_fSum = t;
        t = m_fError + c;
        if (std::abs(m_fError) >= std::abs(c))
        {
            cc = (m_fError - t) + c;
        }
        else
        {
            cc = (c - t) + m_fError;
        }
        m_fError = t;
        m_fErrorError = m_fErrorError + cc;
    }

    /**
      * Adds a value to the sum using Kahan sumation.
      * @param x_i
      */
    inline void add(const ksum& fSum)
    {
        add(fSum.m_fSum);
        add(fSum.m_fError);
        add(fSum.m_fErrorError);
    }

public:
    constexpr bool operator!() const { return 0 != m_fSum || 0 != m_fError || 0 != m_fErrorError; }

    constexpr ksum operator-() const
    {
        ksum fKsum;
        fKsum.m_fSum = -m_fSum;
        fKsum.m_fError = -m_fError;
        fKsum.m_fErrorError = -m_fErrorError;
        return fKsum;
    }

    constexpr void operator=(double fSum)
    {
        m_fSum = fSum;
        m_fError = 0;
        m_fErrorError = 0;
    }

    constexpr void operator=(const ksum& fSum)
    {
        m_fSum = fSum.m_fSum;
        m_fError = fSum.m_fError;
        m_fErrorError = fSum.m_fErrorError;
    }

    inline void operator+=(const ksum& fSum) { add(fSum); }

    inline void operator+=(double fSum) { add(fSum); }

    inline void operator-=(const ksum& fSum) { add(-fSum); }

    inline void operator-=(double fSum) { add(-fSum); }

    inline void operator*=(double fTimes)
    {
        m_fSum *= fTimes;
        m_fError *= fTimes;
        m_fErrorError *= fTimes;
    }

    inline void operator/=(double fDivides)
    {
        m_fSum /= fDivides;
        m_fError /= fDivides;
        m_fErrorError /= fDivides;
    }

    inline ksum operator+(const ksum& fSum) const
    {
        ksum fKsum(*this);
        fKsum.add(fSum);
        return fKsum;
    }

    inline ksum operator+(double fSum) const
    {
        ksum fKsum(*this);
        fKsum.add(fSum);
        return fKsum;
    }

    inline ksum operator-(const ksum& fSum) const
    {
        ksum fKsum(*this);
        fKsum.add(-fSum);
        return fKsum;
    }

    inline ksum operator-(double fSum) const
    {
        ksum fKsum(*this);
        fKsum.add(-fSum);
        return fKsum;
    }

    inline ksum operator*(double fTimes) const
    {
        ksum fKsum(*this);
        fKsum *= (fTimes);
        return fKsum;
    }

    inline ksum operator/(double fDivides) const
    {
        ksum fKsum(*this);
        fKsum /= fDivides;
        return fKsum;
    }

    constexpr bool operator<(const ksum& fSum) const { return get() < fSum.get(); }

    constexpr bool operator<(double fSum) const { return get() < fSum; }

    constexpr bool operator>(const ksum& fSum) const { return get() > fSum.get(); }

    constexpr bool operator>(double fSum) const { return get() > fSum; }

    constexpr bool operator<=(const ksum& fSum) const { return get() <= fSum.get(); }

    constexpr bool operator<=(double fSum) const { return get() <= fSum; }

    constexpr bool operator>=(const ksum& fSum) const { return get() >= fSum.get(); }

    constexpr bool operator>=(double fSum) const { return get() >= fSum; }

    constexpr bool operator==(const ksum& fSum) const
    {
        return fSum.m_fSum == m_fSum && fSum.m_fError == m_fError
               && fSum.m_fErrorError == m_fErrorError;
    }

    constexpr bool operator!=(const ksum& fSum) const
    {
        return fSum.m_fSum != m_fSum || fSum.m_fError != m_fError
               || fSum.m_fErrorError != m_fErrorError;
    }

public:
    /**
  * Returns the final sum.
  * @return final sum
  */
    constexpr double get() const { return m_fSum + m_fError + m_fErrorError; }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
