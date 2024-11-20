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

#pragma once

#include <sal/config.h>

#include <editeng/EPaM.hxx>

#include <cassert>
#include <utility>

struct ESelection
{
    // Select all text
    static constexpr ESelection All() { return { 0, 0, EE_PARA_MAX, EE_TEXTPOS_MAX }; }

    // Set to end without selection
    static constexpr ESelection AtEnd() { return { EE_PARA_MAX, EE_TEXTPOS_MAX }; }

    EPaM start;
    EPaM end;

    constexpr ESelection() = default;

    constexpr ESelection(sal_Int32 _nStartPara, sal_Int32 _nStartPos, sal_Int32 _nEndPara,
                         sal_Int32 _nEndPos)
        : start(_nStartPara, _nStartPos)
        , end(_nEndPara, _nEndPos)
    {
    }

    constexpr ESelection(sal_Int32 nPara, sal_Int32 nPos)
        : ESelection(nPara, nPos, nPara, nPos)
    {
    }

    constexpr explicit ESelection(const EPaM& rPos)
        : ESelection(rPos.nPara, rPos.nIndex)
    {
    }

    bool IsAdjusted() const { return !(end < start); }

    void Adjust()
    {
        if (!IsAdjusted())
            std::swap(start, end);
    }

    void CollapseToStart() { end = start; }
    void CollapseToEnd() { start = end; }

    bool operator==(const ESelection&) const = default;

    bool operator<(const ESelection& rSelection) const
    {
        // The selection must be adjusted.
        assert(IsAdjusted() && rSelection.IsAdjusted());
        // => Only check if end of 'this' < Start of rS
        return end < rSelection.start;
    }
    bool operator>(const ESelection& rSelection) const { return rSelection < *this; }

    bool HasRange() const { return start != end; }
};

template <typename charT, typename traits>
inline std::basic_ostream<charT, traits>& operator<<(std::basic_ostream<charT, traits>& stream,
                                                     ESelection const& rSelection)
{
    return stream << "ESelection(" << rSelection.start.nPara << ',' << rSelection.start.nIndex
                  << "," << rSelection.end.nPara << "," << rSelection.end.nIndex << ")";
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
