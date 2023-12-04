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
    sal_Int32 nStartPara;
    sal_Int32 nStartPos;
    sal_Int32 nEndPara;
    sal_Int32 nEndPos;

    ESelection()
        : nStartPara(0)
        , nStartPos(0)
        , nEndPara(0)
        , nEndPos(0)
    {
    }

    ESelection(sal_Int32 nStPara, sal_Int32 nStPos, sal_Int32 nEPara, sal_Int32 nEPos)
        : nStartPara(nStPara)
        , nStartPos(nStPos)
        , nEndPara(nEPara)
        , nEndPos(nEPos)
    {
    }

    ESelection(sal_Int32 nPara, sal_Int32 nPos)
        : nStartPara(nPara)
        , nStartPos(nPos)
        , nEndPara(nPara)
        , nEndPos(nPos)
    {
    }

    void Adjust();
    bool operator==(const ESelection& rS) const;
    bool operator!=(const ESelection& rS) const { return !operator==(rS); }
    bool operator<(const ESelection& rS) const;
    bool operator>(const ESelection& rS) const;
    bool IsZero() const;
    bool HasRange() const;
};

template <typename charT, typename traits>
inline std::basic_ostream<charT, traits>& operator<<(std::basic_ostream<charT, traits>& stream,
                                                     ESelection const& sel)
{
    return stream << "ESelection(" << sel.nStartPara << ',' << sel.nStartPos << "," << sel.nEndPara
                  << "," << sel.nEndPos << ")";
}

inline bool ESelection::HasRange() const
{
    return (nStartPara != nEndPara) || (nStartPos != nEndPos);
}

inline bool ESelection::IsZero() const
{
    return ((nStartPara == 0) && (nStartPos == 0) && (nEndPara == 0) && (nEndPos == 0));
}

inline bool ESelection::operator==(const ESelection& rS) const
{
    return ((nStartPara == rS.nStartPara) && (nStartPos == rS.nStartPos)
            && (nEndPara == rS.nEndPara) && (nEndPos == rS.nEndPos));
}

inline bool ESelection::operator<(const ESelection& rS) const
{
    // The selection must be adjusted.
    // => Only check if end of 'this' < Start of rS
    return (nEndPara < rS.nStartPara)
           || ((nEndPara == rS.nStartPara) && (nEndPos < rS.nStartPos) && !operator==(rS));
}

inline bool ESelection::operator>(const ESelection& rS) const
{
    // The selection must be adjusted.
    // => Only check if end of 'this' < Start of rS
    return (nStartPara > rS.nEndPara)
           || ((nStartPara == rS.nEndPara) && (nStartPos > rS.nEndPos) && !operator==(rS));
}

inline void ESelection::Adjust()
{
    bool bSwap = false;
    if (nStartPara > nEndPara)
        bSwap = true;
    else if ((nStartPara == nEndPara) && (nStartPos > nEndPos))
        bSwap = true;

    if (bSwap)
    {
        sal_Int32 nSPar = nStartPara;
        sal_Int32 nSPos = nStartPos;
        nStartPara = nEndPara;
        nStartPos = nEndPos;
        nEndPara = nSPar;
        nEndPos = nSPos;
    }
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
