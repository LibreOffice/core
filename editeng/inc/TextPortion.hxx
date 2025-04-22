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

#include "editattr.hxx"
#include "edtspell.hxx"
#include "eerdll2.hxx"
#include <editeng/svxfont.hxx>
#include <editeng/EPaM.hxx>
#include <svl/itemset.hxx>
#include <svl/style.hxx>
#include <svl/itempool.hxx>
#include <svl/languageoptions.hxx>
#include <tools/lineend.hxx>
#include <o3tl/typed_flags_set.hxx>

#include <vector>

enum class PortionKind
{
    TEXT = 0,
    TAB = 1,
    LINEBREAK = 2,
    FIELD = 3,
    HYPHENATOR = 4
};

enum class AsianCompressionFlags
{
    Normal = 0x00,
    Kana = 0x01,
    PunctuationLeft = 0x02,
    PunctuationRight = 0x04,
};
namespace o3tl
{
template <> struct typed_flags<AsianCompressionFlags> : is_typed_flags<AsianCompressionFlags, 0x07>
{
};
}

struct ExtraPortionInfo
{
    ExtraPortionInfo() {}

    tools::Long nOrgWidth = 0;
    tools::Long nWidthFullCompression = 0;

    tools::Long nPortionOffsetX = 0;

    sal_uInt16 nMaxCompression100thPercent = 0;

    AsianCompressionFlags nAsianCompressionTypes = AsianCompressionFlags::Normal;
    bool bFirstCharIsRightPunktuation = false;
    bool bCompressed = false;

    std::unique_ptr<double[]> pOrgDXArray;
    std::vector<sal_Int32> lineBreaksList;

    void SaveOrgDXArray(const double* pDXArray, sal_Int32 nLen)
    {
        if (pDXArray)
        {
            pOrgDXArray.reset(new double[nLen]);
            memcpy(pOrgDXArray.get(), pDXArray, nLen * sizeof(double));
        }
        else
            pOrgDXArray.reset();
    }
};

class TextPortion
{
private:
    std::unique_ptr<ExtraPortionInfo> xExtraInfos;
    sal_Int32 nLen;
    Size aOutSz = Size(-1, -1);
    PortionKind nKind = PortionKind::TEXT;
    sal_uInt8 nRightToLeftLevel = 0;
    sal_Unicode nExtraValue = 0;

public:
    TextPortion(sal_Int32 nL)
        : nLen(nL)
    {
    }

    TextPortion(const TextPortion& r)
        : nLen(r.nLen)
        , aOutSz(r.aOutSz)
        , nKind(r.nKind)
        , nRightToLeftLevel(r.nRightToLeftLevel)
        , nExtraValue(r.nExtraValue)
    {
    }

    sal_Int32 GetLen() const { return nLen; }
    void SetLen(sal_Int32 nL) { nLen = nL; }

    void setWidth(tools::Long nWidth) { aOutSz.setWidth(nWidth); }

    void setHeight(tools::Long nHeight) { aOutSz.setHeight(nHeight); }

    void adjustSize(tools::Long nDeltaX, tools::Long nDeltaY)
    {
        if (nDeltaX != 0)
            aOutSz.AdjustWidth(nDeltaX);
        if (nDeltaY != 0)
            aOutSz.AdjustHeight(nDeltaY);
    }

    void SetSize(const Size& rSize) { aOutSz = rSize; }

    const Size& GetSize() const { return aOutSz; }

    void SetKind(PortionKind n) { nKind = n; }
    PortionKind GetKind() const { return nKind; }

    void SetRightToLeftLevel(sal_uInt8 n) { nRightToLeftLevel = n; }
    sal_uInt8 GetRightToLeftLevel() const { return nRightToLeftLevel; }
    bool IsRightToLeft() const { return (nRightToLeftLevel & 1); }

    sal_Unicode GetExtraValue() const { return nExtraValue; }
    void SetExtraValue(sal_Unicode n) { nExtraValue = n; }

    ExtraPortionInfo* GetExtraInfos() const { return xExtraInfos.get(); }
    void SetExtraInfos(ExtraPortionInfo* p) { xExtraInfos.reset(p); }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
