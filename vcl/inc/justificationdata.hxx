/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <sal/types.h>
#include <o3tl/typed_flags_set.hxx>

#include <optional>
#include <vector>

enum class ClusterJustificationFlags
{
    NONE = 0x0000,
    KashidaPosition = 0x0001,
};

namespace o3tl
{
template <>
struct typed_flags<ClusterJustificationFlags> : is_typed_flags<ClusterJustificationFlags, 0x0001>
{
};
}

class ClusterJustification
{
public:
    double m_nTotalAdvance = 0.0;
    ClusterJustificationFlags m_nFlags = ClusterJustificationFlags::NONE;
};

class JustificationData
{
private:
    std::vector<ClusterJustification> m_aClusters;
    sal_Int32 m_nBaseIndex = 0;
    sal_Int32 m_nEndIndex = 0;
    bool m_bContainsAdvances = false;
    bool m_bContainsKashidaPositions = false;

    [[nodiscard]] inline std::optional<size_t> GetIndex(sal_Int32 nClusterId) const
    {
        auto nIndex = nClusterId - m_nBaseIndex;
        if (nIndex >= 0 && nIndex < static_cast<sal_Int32>(m_aClusters.size()))
        {
            return static_cast<size_t>(nIndex);
        }

        return std::nullopt;
    }

public:
    JustificationData() = default;
    JustificationData(sal_Int32 nBaseIndex, sal_Int32 nSize)
        : m_nBaseIndex(nBaseIndex)
        , m_nEndIndex(nBaseIndex + nSize)
    {
        m_aClusters.resize(nSize);
    }

    [[nodiscard]] bool empty() const { return m_aClusters.empty(); }

    [[nodiscard]] bool ContainsAdvances() const { return m_bContainsAdvances; }
    [[nodiscard]] bool ContainsKashidaPositions() const { return m_bContainsKashidaPositions; }

    [[nodiscard]] double GetTotalAdvance(sal_Int32 nClusterId) const
    {
        if (nClusterId < m_nBaseIndex || m_aClusters.empty())
        {
            return 0.0;
        }

        if (nClusterId < m_nEndIndex)
        {
            return m_aClusters.at(nClusterId - m_nBaseIndex).m_nTotalAdvance;
        }

        return m_aClusters.back().m_nTotalAdvance;
    }

    [[nodiscard]] std::optional<bool> GetPositionHasKashida(sal_Int32 nClusterId) const
    {
        if (auto nIndex = GetIndex(nClusterId); nIndex.has_value())
        {
            return std::optional<bool>{ m_aClusters.at(*nIndex).m_nFlags
                                        & ClusterJustificationFlags::KashidaPosition };
        }

        return std::nullopt;
    }

    void SetTotalAdvance(sal_Int32 nClusterId, double nValue)
    {
        if (auto nIndex = GetIndex(nClusterId); nIndex.has_value())
        {
            m_aClusters.at(*nIndex).m_nTotalAdvance = nValue;
            m_bContainsAdvances = true;
        }
    }

    void SetKashidaPosition(sal_Int32 nClusterId, bool bValue)
    {
        if (auto nIndex = GetIndex(nClusterId); nIndex.has_value())
        {
            if (bValue)
            {
                m_aClusters.at(*nIndex).m_nFlags |= ClusterJustificationFlags::KashidaPosition;
            }
            else
            {
                m_aClusters.at(*nIndex).m_nFlags &= ~ClusterJustificationFlags::KashidaPosition;
            }

            m_bContainsKashidaPositions = true;
        }
    }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
