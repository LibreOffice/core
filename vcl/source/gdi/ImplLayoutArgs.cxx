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

#include <ImplLayoutArgs.hxx>

#include <unicode/ubidi.h>
#include <unicode/uchar.h>

#include <algorithm>
#include <memory>

//#include <impglyphitem.hxx>

ImplLayoutArgs::ImplLayoutArgs(const OUString& rStr, int nMinCharPos, int nEndCharPos,
                               SalLayoutFlags nFlags, const LanguageTag& rLanguageTag,
                               vcl::TextLayoutCache const* const pLayoutCache)
    : maLanguageTag(rLanguageTag)
    , mnFlags(nFlags)
    , mrStr(rStr)
    , mnMinCharPos(nMinCharPos)
    , mnEndCharPos(nEndCharPos)
    , m_pTextLayoutCache(pLayoutCache)
    , mpDXArray(nullptr)
    , mnLayoutWidth(0)
    , mnOrientation(0)
{
    if (mnFlags & SalLayoutFlags::BiDiStrong)
    {
        // handle strong BiDi mode

        // do not bother to BiDi analyze strong LTR/RTL
        // TODO: can we assume these strings do not have unicode control chars?
        //       if not remove the control characters from the runs
        bool bRTL(mnFlags & SalLayoutFlags::BiDiRtl);
        AddRun(mnMinCharPos, mnEndCharPos, bRTL);
    }
    else
    {
        // handle weak BiDi mode
        UBiDiLevel nLevel = (mnFlags & SalLayoutFlags::BiDiRtl) ? 1 : 0;

        // prepare substring for BiDi analysis
        // TODO: reuse allocated pParaBidi
        UErrorCode rcI18n = U_ZERO_ERROR;
        const int nLength = mrStr.getLength();
        UBiDi* pParaBidi = ubidi_openSized(nLength, 0, &rcI18n);
        if (!pParaBidi)
            return;
        ubidi_setPara(pParaBidi, reinterpret_cast<const UChar*>(mrStr.getStr()), nLength, nLevel,
                      nullptr, &rcI18n);

        UBiDi* pLineBidi = pParaBidi;
        int nSubLength = mnEndCharPos - mnMinCharPos;
        if (nSubLength != nLength)
        {
            pLineBidi = ubidi_openSized(nSubLength, 0, &rcI18n);
            ubidi_setLine(pParaBidi, mnMinCharPos, mnEndCharPos, pLineBidi, &rcI18n);
        }

        // run BiDi algorithm
        const int nRunCount = ubidi_countRuns(pLineBidi, &rcI18n);
        //maRuns.resize( 2 * nRunCount );
        for (int i = 0; i < nRunCount; ++i)
        {
            int32_t nMinPos, nRunLength;
            const UBiDiDirection nDir = ubidi_getVisualRun(pLineBidi, i, &nMinPos, &nRunLength);
            const int nPos0 = nMinPos + mnMinCharPos;
            const int nPos1 = nPos0 + nRunLength;

            const bool bRTL = (nDir == UBIDI_RTL);
            AddRun(nPos0, nPos1, bRTL);
        }

        // cleanup BiDi engine
        if (pLineBidi != pParaBidi)
            ubidi_close(pLineBidi);
        ubidi_close(pParaBidi);
    }

    // prepare calls to GetNextPos/GetNextRun
    maRuns.ResetPos();
}

void ImplLayoutArgs::SetLayoutWidth(DeviceCoordinate nWidth) { mnLayoutWidth = nWidth; }

void ImplLayoutArgs::SetDXArray(DeviceCoordinate const* pDXArray) { mpDXArray = pDXArray; }

void ImplLayoutArgs::SetOrientation(Degree10 nOrientation) { mnOrientation = nOrientation; }

void ImplLayoutArgs::ResetPos() { maRuns.ResetPos(); }

bool ImplLayoutArgs::GetNextPos(int* nCharPos, bool* bRTL)
{
    return maRuns.GetNextPos(nCharPos, bRTL);
}

void ImplLayoutArgs::NeedFallback(int nMinRunPos, int nEndRunPos, bool bRTL)
{
    maFallbackRuns.AddRun(nMinRunPos, nEndRunPos, bRTL);
}

bool ImplLayoutArgs::NeedFallback() const { return !maFallbackRuns.IsEmpty(); }

static bool IsControlChar(sal_UCS4 cChar)
{
    // C0 control characters
    if ((0x0001 <= cChar) && (cChar <= 0x001F))
        return true;
    // formatting characters
    if ((0x200E <= cChar) && (cChar <= 0x200F))
        return true;
    if ((0x2028 <= cChar) && (cChar <= 0x202E))
        return true;
    // deprecated formatting characters
    if ((0x206A <= cChar) && (cChar <= 0x206F))
        return true;
    if (0x2060 == cChar)
        return true;
    // byte order markers and invalid unicode
    if ((cChar == 0xFEFF) || (cChar == 0xFFFE) || (cChar == 0xFFFF))
        return true;
    return false;
}

// add a run after splitting it up to get rid of control chars
void ImplLayoutArgs::AddRun(int nCharPos0, int nCharPos1, bool bRTL)
{
    SAL_WARN_IF(nCharPos0 > nCharPos1, "vcl", "ImplLayoutArgs::AddRun() nCharPos0>=nCharPos1");

    // remove control characters from runs by splitting them up
    if (!bRTL)
    {
        for (int i = nCharPos0; i < nCharPos1; ++i)
            if (IsControlChar(mrStr[i]))
            {
                // add run until control char
                maRuns.AddRun(nCharPos0, i, bRTL);
                nCharPos0 = i + 1;
            }
    }
    else
    {
        for (int i = nCharPos1; --i >= nCharPos0;)
            if (IsControlChar(mrStr[i]))
            {
                // add run until control char
                maRuns.AddRun(i + 1, nCharPos1, bRTL);
                nCharPos1 = i;
            }
    }

    // add remainder of run
    maRuns.AddRun(nCharPos0, nCharPos1, bRTL);
}

bool ImplLayoutArgs::PrepareFallback()
{
    // short circuit if no fallback is needed
    if (maFallbackRuns.IsEmpty())
    {
        maRuns.Clear();
        return false;
    }

    // convert the fallback requests to layout requests
    bool bRTL;
    int nMin, nEnd;

    // get the individual fallback requests
    std::vector<int> aPosVector;
    aPosVector.reserve(mrStr.getLength());
    maFallbackRuns.ResetPos();
    for (; maFallbackRuns.GetRun(&nMin, &nEnd, &bRTL); maFallbackRuns.NextRun())
        for (int i = nMin; i < nEnd; ++i)
            aPosVector.push_back(i);
    maFallbackRuns.Clear();

    // sort the individual fallback requests
    std::sort(aPosVector.begin(), aPosVector.end());

    // adjust fallback runs to have the same order and limits of the original runs
    ImplLayoutRuns aNewRuns;
    maRuns.ResetPos();
    for (; maRuns.GetRun(&nMin, &nEnd, &bRTL); maRuns.NextRun())
    {
        if (!bRTL)
        {
            auto it = std::lower_bound(aPosVector.begin(), aPosVector.end(), nMin);
            for (; (it != aPosVector.end()) && (*it < nEnd); ++it)
                aNewRuns.AddPos(*it, bRTL);
        }
        else
        {
            auto it = std::upper_bound(aPosVector.begin(), aPosVector.end(), nEnd);
            while ((it != aPosVector.begin()) && (*--it >= nMin))
                aNewRuns.AddPos(*it, bRTL);
        }
    }

    maRuns = aNewRuns; // TODO: use vector<>::swap()
    maRuns.ResetPos();
    return true;
}

bool ImplLayoutArgs::GetNextRun(int* nMinRunPos, int* nEndRunPos, bool* bRTL)
{
    bool bValid = maRuns.GetRun(nMinRunPos, nEndRunPos, bRTL);
    maRuns.NextRun();
    return bValid;
}

std::ostream& operator<<(std::ostream& s, ImplLayoutArgs const& rArgs)
{
#ifndef SAL_LOG_INFO
    (void)rArgs;
#else
    s << "ImplLayoutArgs{";

    s << "Flags=";
    if (rArgs.mnFlags == SalLayoutFlags::NONE)
        s << 0;
    else
    {
        bool need_or = false;
        s << "{";
#define TEST(x)                                                                                    \
    if (rArgs.mnFlags & SalLayoutFlags::x)                                                         \
    {                                                                                              \
        if (need_or)                                                                               \
            s << "|";                                                                              \
        s << #x;                                                                                   \
        need_or = true;                                                                            \
    }
        TEST(BiDiRtl);
        TEST(BiDiStrong);
        TEST(RightAlign);
        TEST(DisableKerning);
        TEST(KerningAsian);
        TEST(Vertical);
        TEST(KashidaJustification);
        TEST(ForFallback);
#undef TEST
        s << "}";
    }

    const int nLength = rArgs.mrStr.getLength();

    s << ",Length=" << nLength;
    s << ",MinCharPos=" << rArgs.mnMinCharPos;
    s << ",EndCharPos=" << rArgs.mnEndCharPos;

    s << ",Str=\"";
    int lim = nLength;
    if (lim > 10)
        lim = 7;
    for (int i = 0; i < lim; i++)
    {
        if (rArgs.mrStr[i] == '\n')
            s << "\\n";
        else if (rArgs.mrStr[i] < ' ' || (rArgs.mrStr[i] >= 0x7F && rArgs.mrStr[i] <= 0xFF))
            s << "\\0x" << std::hex << std::setw(2) << std::setfill('0')
              << static_cast<int>(rArgs.mrStr[i]) << std::setfill(' ') << std::setw(1) << std::dec;
        else if (rArgs.mrStr[i] < 0x7F)
            s << static_cast<char>(rArgs.mrStr[i]);
        else
            s << "\\u" << std::hex << std::setw(4) << std::setfill('0')
              << static_cast<int>(rArgs.mrStr[i]) << std::setfill(' ') << std::setw(1) << std::dec;
    }
    if (nLength > lim)
        s << "...";
    s << "\"";

    s << ",DXArray=";
    if (rArgs.mpDXArray)
    {
        s << "[";
        int count = rArgs.mnEndCharPos - rArgs.mnMinCharPos;
        lim = count;
        if (lim > 10)
            lim = 7;
        for (int i = 0; i < lim; i++)
        {
            s << rArgs.mpDXArray[i];
            if (i < lim - 1)
                s << ",";
        }
        if (count > lim)
        {
            if (count > lim + 1)
                s << "...";
            s << rArgs.mpDXArray[count - 1];
        }
        s << "]";
    }
    else
        s << "NULL";

    s << ",LayoutWidth=" << rArgs.mnLayoutWidth;

    s << "}";

#endif
    return s;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
