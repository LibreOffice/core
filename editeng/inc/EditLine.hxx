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

class ParaPortion;

class EditLine
{
public:
    typedef std::vector<sal_Int32> CharPosArrayType;

private:
    CharPosArrayType aPositions;
    std::vector<sal_Bool> aKashidaPositions;
    sal_Int32 nTxtWidth;
    sal_Int32 nStartPosX;
    sal_Int32 nStart; // could be replaced by nStartPortion
    sal_Int32 nEnd; // could be replaced by nEndPortion
    sal_Int32 nStartPortion;
    sal_Int32 nEndPortion;
    sal_uInt16 nHeight; //  Total height of the line
    sal_uInt16 nTxtHeight; // Pure Text height
    sal_uInt16 nMaxAscent;
    bool bHangingPunctuation : 1;
    bool bInvalid : 1; // for skillful formatting

public:
    EditLine();
    EditLine(const EditLine&);
    ~EditLine();

    bool IsIn(sal_Int32 nIndex) const { return ((nIndex >= nStart) && (nIndex < nEnd)); }

    bool IsIn(sal_Int32 nIndex, bool bInclEnd) const
    {
        return ((nIndex >= nStart) && (bInclEnd ? (nIndex <= nEnd) : (nIndex < nEnd)));
    }

    void SetStart(sal_Int32 n) { nStart = n; }
    sal_Int32 GetStart() const { return nStart; }
    sal_Int32& GetStart() { return nStart; }

    void SetEnd(sal_Int32 n) { nEnd = n; }
    sal_Int32 GetEnd() const { return nEnd; }
    sal_Int32& GetEnd() { return nEnd; }

    void SetStartPortion(sal_Int32 n) { nStartPortion = n; }
    sal_Int32 GetStartPortion() const { return nStartPortion; }
    sal_Int32& GetStartPortion() { return nStartPortion; }

    void SetEndPortion(sal_Int32 n) { nEndPortion = n; }
    sal_Int32 GetEndPortion() const { return nEndPortion; }
    sal_Int32& GetEndPortion() { return nEndPortion; }

    void SetHeight(sal_uInt16 nH, sal_uInt16 nTxtH = 0);
    sal_uInt16 GetHeight() const { return nHeight; }
    sal_uInt16 GetTxtHeight() const { return nTxtHeight; }

    void SetTextWidth(sal_Int32 n) { nTxtWidth = n; }
    sal_Int32 GetTextWidth() const { return nTxtWidth; }

    void SetMaxAscent(sal_uInt16 n) { nMaxAscent = n; }
    sal_uInt16 GetMaxAscent() const { return nMaxAscent; }

    void SetHangingPunctuation(bool b) { bHangingPunctuation = b; }
    bool IsHangingPunctuation() const { return bHangingPunctuation; }

    sal_Int32 GetLen() const { return nEnd - nStart; }

    sal_Int32 GetStartPosX() const { return nStartPosX; }
    void SetStartPosX(sal_Int32 start);
    Size CalcTextSize(ParaPortion& rParaPortion);

    bool IsInvalid() const { return bInvalid; }
    bool IsValid() const { return !bInvalid; }
    void SetInvalid() { bInvalid = true; }
    void SetValid() { bInvalid = false; }

    bool IsEmpty() const { return nEnd <= nStart; }

    CharPosArrayType& GetCharPosArray() { return aPositions; }
    const CharPosArrayType& GetCharPosArray() const { return aPositions; }

    std::vector<sal_Bool>& GetKashidaArray() { return aKashidaPositions; }
    const std::vector<sal_Bool>& GetKashidaArray() const { return aKashidaPositions; }

    EditLine* Clone() const;

    EditLine& operator=(const EditLine& rLine);
    friend bool operator==(const EditLine& r1, const EditLine& r2);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
