/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <sal/config.h>
#include <o3tl/span.hxx>
#include <cmath>
#include <vector>

class KernArraySpan final
{
private:
    int m_nSubUnitFactor;
    o3tl::span<const sal_Int32> m_DXArray;

public:
    KernArraySpan()
        : m_nSubUnitFactor(1)
    {
    }

    KernArraySpan(o3tl::span<const sal_Int32> DXArray, int nSubUnitFactor = 1)
        : m_nSubUnitFactor(nSubUnitFactor)
        , m_DXArray(DXArray)
    {
    }
    size_t size() const { return m_DXArray.size(); }
    bool empty() const { return m_DXArray.empty(); }
    sal_Int32 operator[](size_t nIndex) const { return get(nIndex); }
    sal_Int32 get(size_t nIndex) const
    {
        return std::round(static_cast<double>(m_DXArray[nIndex]) / m_nSubUnitFactor);
    }

    int get_factor() const { return m_nSubUnitFactor; }
    sal_Int32 get_subunit(size_t nIndex) const { return m_DXArray[nIndex]; }
};

class KernArray final
{
private:
    int m_nSubUnitFactor;
    std::vector<sal_Int32> m_aDXArray;

public:
    KernArray(int nSubUnitFactor = 1)
        : m_nSubUnitFactor(nSubUnitFactor)
    {
    }

    sal_Int32 operator[](size_t nIndex) const { return get(nIndex); }
    sal_Int32 get(size_t nIndex) const
    {
        return std::round(static_cast<double>(m_aDXArray[nIndex]) / m_nSubUnitFactor);
    }

    int get_factor() const { return m_nSubUnitFactor; }
    sal_Int32 get_subunit(size_t nIndex) const { return m_aDXArray[nIndex]; }

    void set_subunit(size_t nIndex, sal_Int32 nValue) { m_aDXArray[nIndex] = nValue; }
    std::vector<sal_Int32>& get_subunit_array() { return m_aDXArray; }

    void adjust(size_t nIndex, sal_Int32 nDiff) { m_aDXArray[nIndex] += nDiff * m_nSubUnitFactor; }
    void set(size_t nIndex, sal_Int32 nValue) { m_aDXArray[nIndex] = nValue * m_nSubUnitFactor; }
    void push_back(sal_Int32 nUnit) { m_aDXArray.push_back(nUnit * m_nSubUnitFactor); }
    sal_Int32 back() const { return m_aDXArray.back() * m_nSubUnitFactor; }
    size_t size() const { return m_aDXArray.size(); }
    bool empty() const { return m_aDXArray.empty(); }
    void clear() { m_aDXArray.clear(); }
    void assign(KernArraySpan other)
    {
        m_nSubUnitFactor = other.get_factor();
        m_aDXArray.clear();
        size_t nLen = other.size();
        m_aDXArray.reserve(nLen);
        for (size_t i = 0; i < nLen; ++i)
            m_aDXArray.push_back(other.get_subunit(i));
    }
    void resize(size_t nSize) { m_aDXArray.resize(nSize); }
    void resize(size_t nSize, sal_Int32 nDefault)
    {
        m_aDXArray.resize(nSize, nDefault * m_nSubUnitFactor);
    }
    void reserve(size_t nCapacity) { m_aDXArray.reserve(nCapacity); }

    bool operator==(const KernArray& rOther) const
    {
        size_t nSize = size();
        if (nSize != rOther.size())
            return false;
        for (size_t i = 0; i < nSize; ++i)
            if (m_aDXArray[i] != rOther.m_aDXArray[i])
                return false;
        return true;
    }

    operator KernArraySpan() const { return KernArraySpan(m_aDXArray, m_nSubUnitFactor); }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
