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

#include <ImplLayoutRuns.hxx>
#include <algorithm>
#include <tuple>

void ImplLayoutRuns::AddPos( int nCharPos, bool bRTL )
{
    // check if charpos could extend current run
    if (!maRuns.empty())
    {
        auto& rLastRun = maRuns.back();
        if (bRTL == rLastRun.m_bRTL)
        {
            if (nCharPos + 1 == rLastRun.m_nMinRunPos)
            {
                // extend current run by new charpos
                rLastRun.m_nMinRunPos = nCharPos;
            }

            if (nCharPos == rLastRun.m_nEndRunPos)
            {
                // extend current run by new charpos
                ++rLastRun.m_nEndRunPos;
            }
        }

        // ignore new charpos when it is in current run
        if ((rLastRun.m_nMinRunPos <= nCharPos) && (nCharPos < rLastRun.m_nEndRunPos))
        {
            return;
        }
    }

    // else append a new run consisting of the new charpos
    maRuns.emplace_back(nCharPos, nCharPos + 1, bRTL);
}

void ImplLayoutRuns::AddRun( int nCharPos0, int nCharPos1, bool bRTL )
{
    if( nCharPos0 == nCharPos1 )
        return;

    auto nOrderedCharPos0 = std::min(nCharPos0, nCharPos1);
    auto nOrderedCharPos1 = std::max(nCharPos0, nCharPos1);

    if (!maRuns.empty())
    {
        auto& rLastRun = maRuns.back();
        if ((rLastRun.m_nMinRunPos <= nOrderedCharPos0)
            && (nOrderedCharPos0 <= rLastRun.m_nEndRunPos)
            && (nOrderedCharPos0 < rLastRun.m_nEndRunPos || bRTL == rLastRun.m_bRTL))
        {
            rLastRun.m_nEndRunPos = std::max(rLastRun.m_nEndRunPos, nOrderedCharPos1);
            return;
        }
    }

    // append new run
    maRuns.emplace_back(nOrderedCharPos0, nOrderedCharPos1, bRTL);
}

void ImplLayoutRuns::Normalize()
{
    boost::container::small_vector<Run, 8> aOldRuns;
    std::swap(aOldRuns, maRuns);

    std::sort(aOldRuns.begin(), aOldRuns.end(),
              [](const Run& rA, const Run& rB)
              {
                  return std::tie(rA.m_nMinRunPos, rA.m_nEndRunPos)
                         < std::tie(rB.m_nMinRunPos, rB.m_nEndRunPos);
              });

    for (const auto& rRun : aOldRuns)
    {
        AddRun(rRun.m_nMinRunPos, rRun.m_nEndRunPos, false);
    }
}

void ImplLayoutRuns::ReverseTail(size_t nTailIndex)
{
    std::reverse(maRuns.begin() + nTailIndex, maRuns.end());
}

bool ImplLayoutRuns::PosIsInRun( int nCharPos ) const
{
    if( mnRunIndex >= static_cast<int>(maRuns.size()) )
        return false;

    return maRuns.at(mnRunIndex).Contains(nCharPos);
}

bool ImplLayoutRuns::PosIsInAnyRun( int nCharPos ) const
{
    return std::any_of(maRuns.begin(), maRuns.end(),
                       [nCharPos](const auto& rRun) { return rRun.Contains(nCharPos); });
}

bool ImplLayoutRuns::GetNextPos( int* nCharPos, bool* bRightToLeft )
{
    // negative nCharPos => reset to first run
    if( *nCharPos < 0 )
        mnRunIndex = 0;

    // return false when all runs completed
    if( mnRunIndex >= static_cast<int>(maRuns.size()) )
        return false;

    const auto& rRun = maRuns.at(mnRunIndex);

    if( *nCharPos < 0 )
    {
        // get first valid nCharPos in run
        *nCharPos = rRun.m_nMinRunPos;
    }
    else
    {
        // advance to next nCharPos
        ++(*nCharPos);

        // advance to next run if current run is completed
        if (*nCharPos == rRun.m_nEndRunPos)
        {
            ++mnRunIndex;
            if (mnRunIndex >= static_cast<int>(maRuns.size()))
            {
                return false;
            }

            const auto& rNextRun = maRuns.at(mnRunIndex);
            *nCharPos = rNextRun.m_nMinRunPos;
            *bRightToLeft = rNextRun.m_bRTL;
        }
    }

    return true;
}

bool ImplLayoutRuns::GetRun( int* nMinRunPos, int* nEndRunPos, bool* bRightToLeft ) const
{
    if( mnRunIndex >= static_cast<int>(maRuns.size()) )
        return false;

    const auto& rRun = maRuns.at(mnRunIndex);
    *nMinRunPos = rRun.m_nMinRunPos;
    *nEndRunPos = rRun.m_nEndRunPos;
    *bRightToLeft = rRun.m_bRTL;
    return true;
}

void ImplLayoutRuns::PrepareFallbackRuns(ImplLayoutRuns* paRuns, ImplLayoutRuns* paFallbackRuns)
{
    // Normalize the input fallback runs. This is required for efficient lookup.
    paFallbackRuns->Normalize();

    // Adjust fallback runs to have the same order and limits of the original runs.
    ImplLayoutRuns aNewRuns;
    for (const auto& rRun : *paRuns)
    {
        auto nTailIndex = aNewRuns.size();

        // Search for the first fallback run intersecting this run
        auto it = std::lower_bound(paFallbackRuns->begin(), paFallbackRuns->end(),
                                   rRun.m_nMinRunPos, [](const auto& rCompRun, int nValue)
                                   { return rCompRun.m_nEndRunPos < nValue; });
        for (; it != paFallbackRuns->end(); ++it)
        {
            if (rRun.m_nEndRunPos <= it->m_nMinRunPos)
            {
                break;
            }

            int nSubMin = std::max(rRun.m_nMinRunPos, it->m_nMinRunPos);
            int nSubMax = std::min(rRun.m_nEndRunPos, it->m_nEndRunPos);

            aNewRuns.AddRun(nSubMin, nSubMax, rRun.m_bRTL);
        }

        // RTL subruns must be added in reverse order
        if (rRun.m_bRTL)
        {
            aNewRuns.ReverseTail(nTailIndex);
        }
    }

    *paRuns = aNewRuns;
    paRuns->ResetPos();

    paFallbackRuns->Clear();
    paFallbackRuns->ResetPos();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
