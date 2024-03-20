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

#include <vector>
#include <sal/types.h>
#include <tools/gen.hxx>

class ParaPortion;

class EditLine
{
public:
    typedef std::vector<sal_Int32> CharPosArrayType;

private:
    CharPosArrayType maPositions;
    std::vector<sal_Bool> maKashidaPositions;
    sal_Int32 mnTextWidth = 0;
    sal_Int32 mnStartPosX = 0;
    sal_Int32 mnNextLinePosXDiff = 0;
    sal_Int32 mnStart = 0; // could be replaced by nStartPortion
    sal_Int32 mnEnd = 0; // could be replaced by nEndPortion
    sal_Int32 mnStartPortion = 0;
    sal_Int32 mnEndPortion = 0;
    sal_uInt16 mnHeight = 0; //  Total height of the line
    sal_uInt16 mnTextHeight = 0; // Pure Text height
    sal_uInt16 mnMaxAscent = 0;
    bool mbHangingPunctuation : 1 = false;
    bool mbInvalid : 1 = true; // for skillful formatting

public:
    EditLine() = default;
    EditLine(const EditLine& rEditLine)
        : mnStart(rEditLine.mnStart)
        , mnEnd(rEditLine.mnEnd)
        , mnStartPortion(rEditLine.mnStartPortion)
        , mnEndPortion(rEditLine.mnEndPortion)
        , mbHangingPunctuation(rEditLine.mbHangingPunctuation)
    {
    }

    bool IsIn(sal_Int32 nIndex) const { return nIndex >= mnStart && nIndex < mnEnd; }

    bool IsIn(sal_Int32 nIndex, bool bIncludeEnd) const
    {
        return nIndex >= mnStart && (bIncludeEnd ? nIndex <= mnEnd : nIndex < mnEnd);
    }

    void SetStart(sal_Int32 nStart) { mnStart = nStart; }
    sal_Int32 GetStart() const { return mnStart; }
    sal_Int32& GetStart() { return mnStart; }

    void SetEnd(sal_Int32 nEnd) { mnEnd = nEnd; }
    sal_Int32 GetEnd() const { return mnEnd; }
    sal_Int32& GetEnd() { return mnEnd; }

    void SetStartPortion(sal_Int32 nStartPortion) { mnStartPortion = nStartPortion; }
    sal_Int32 GetStartPortion() const { return mnStartPortion; }
    sal_Int32& GetStartPortion() { return mnStartPortion; }

    void SetEndPortion(sal_Int32 nEndPortion) { mnEndPortion = nEndPortion; }
    sal_Int32 GetEndPortion() const { return mnEndPortion; }
    sal_Int32& GetEndPortion() { return mnEndPortion; }

    void SetHeight(sal_uInt16 nHeight, sal_uInt16 nTextHeight = 0);
    sal_uInt16 GetHeight() const { return mnHeight; }
    sal_uInt16 GetTxtHeight() const { return mnTextHeight; }

    void SetTextWidth(sal_Int32 nTextWidth) { mnTextWidth = nTextWidth; }
    sal_Int32 GetTextWidth() const { return mnTextWidth; }

    void SetMaxAscent(sal_uInt16 nMaxAscent) { mnMaxAscent = nMaxAscent; }
    sal_uInt16 GetMaxAscent() const { return mnMaxAscent; }

    void SetHangingPunctuation(bool bHangingPunctuation)
    {
        mbHangingPunctuation = bHangingPunctuation;
    }
    bool IsHangingPunctuation() const { return mbHangingPunctuation; }

    sal_Int32 GetLen() const { return mnEnd - mnStart; }

    sal_Int32 GetStartPosX() const { return mnStartPosX; }
    void SetStartPosX(sal_Int32 nStart);
    sal_Int32 GetNextLinePosXDiff() const { return mnNextLinePosXDiff; }
    void SetNextLinePosXDiff(sal_Int32 nDiff) { mnNextLinePosXDiff = nDiff; }
    Size CalcTextSize(ParaPortion& rParaPortion);

    bool IsInvalid() const { return mbInvalid; }
    bool IsValid() const { return !mbInvalid; }
    void SetInvalid() { mbInvalid = true; }
    void SetValid() { mbInvalid = false; }

    bool IsEmpty() const { return mnEnd <= mnStart; }

    CharPosArrayType& GetCharPosArray() { return maPositions; }
    const CharPosArrayType& GetCharPosArray() const { return maPositions; }

    std::vector<sal_Bool>& GetKashidaArray() { return maKashidaPositions; }
    const std::vector<sal_Bool>& GetKashidaArray() const { return maKashidaPositions; }

    EditLine* Clone() const;

    EditLine& operator=(const EditLine& rLine)
    {
        mnEnd = rLine.mnEnd;
        mnStart = rLine.mnStart;
        mnEndPortion = rLine.mnEndPortion;
        mnStartPortion = rLine.mnStartPortion;
        return *this;
    }

    bool operator==(const EditLine& rLine) const
    {
        return mnStart == rLine.mnStart && mnEnd == rLine.mnEnd
               && mnStartPortion == rLine.mnStartPortion && mnEndPortion == rLine.mnEndPortion;
    }
};

template <typename charT, typename traits>
inline std::basic_ostream<charT, traits>& operator<<(std::basic_ostream<charT, traits>& stream,
                                                     EditLine const& rLine)
{
    return stream << "EditLine(" << rLine.GetStart() << ", " << rLine.GetEnd() << ")";
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
