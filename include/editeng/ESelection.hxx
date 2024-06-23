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

struct ESelection
{
    static constexpr sal_Int32 MAX_PARAGRAPH_POSITION = SAL_MAX_INT32;
    static constexpr sal_Int32 MAX_TEXT_POSITION = SAL_MAX_INT32;

    // Select all text
    static ESelection All() { return ESelection(0, 0, MAX_PARAGRAPH_POSITION, MAX_TEXT_POSITION); }

    // Set to "not found" state
    static ESelection NotFound() { return ESelection(MAX_PARAGRAPH_POSITION, MAX_TEXT_POSITION); }
    // Set to no selection
    static ESelection NoSelection()
    {
        return ESelection(MAX_PARAGRAPH_POSITION, MAX_TEXT_POSITION);
    }

    sal_Int32 nStartPara = 0;
    sal_Int32 nStartPos = 0;
    sal_Int32 nEndPara = 0;
    sal_Int32 nEndPos = 0;

    ESelection() = default;

    ESelection(sal_Int32 _nStartPara, sal_Int32 _nStartPos, sal_Int32 _nEndPara, sal_Int32 _nEndPos)
        : nStartPara(_nStartPara)
        , nStartPos(_nStartPos)
        , nEndPara(_nEndPara)
        , nEndPos(_nEndPos)
    {
    }

    ESelection(sal_Int32 nPara, sal_Int32 nPos)
        : nStartPara(nPara)
        , nStartPos(nPos)
        , nEndPara(nPara)
        , nEndPos(nPos)
    {
    }

    void Adjust()
    {
        if (nStartPara > nEndPara || (nStartPara == nEndPara && nStartPos > nEndPos))
        {
            std::swap(nStartPara, nEndPara);
            std::swap(nStartPos, nEndPos);
        }
    }

    bool operator==(const ESelection& rSelection) const
    {
        return nStartPara == rSelection.nStartPara && nStartPos == rSelection.nStartPos
               && nEndPara == rSelection.nEndPara && nEndPos == rSelection.nEndPos;
    }

    bool operator!=(const ESelection& rSelection) const = default;

    bool operator<(const ESelection& rSelection) const
    {
        // The selection must be adjusted.
        // => Only check if end of 'this' < Start of rS
        return nEndPara < rSelection.nStartPara
               || (nEndPara == rSelection.nStartPara && nEndPos < rSelection.nStartPos
                   && operator!=(rSelection));
    }

    bool operator>(const ESelection& rSelection) const
    {
        // The selection must be adjusted.
        // => Only check if end of 'this' < Start of rS
        return nStartPara > rSelection.nEndPara
               || (nStartPara == rSelection.nEndPara && nStartPos > rSelection.nEndPos
                   && operator!=(rSelection));
    }

    bool IsZero() const
    {
        return nStartPara == 0 && nStartPos == 0 && nEndPara == 0 && nEndPos == 0;
    }

    bool HasRange() const { return nStartPara != nEndPara || nStartPos != nEndPos; }
};

template <typename charT, typename traits>
inline std::basic_ostream<charT, traits>& operator<<(std::basic_ostream<charT, traits>& stream,
                                                     ESelection const& rSelection)
{
    return stream << "ESelection(" << rSelection.nStartPara << ',' << rSelection.nStartPos << ","
                  << rSelection.nEndPara << "," << rSelection.nEndPos << ")";
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
