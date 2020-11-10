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

#include <sal/log.hxx>
#include <rtl/ustrbuf.hxx>
#include "emfpstringformat.hxx"

namespace emfplushelper
{
    EMFPStringFormat::EMFPStringFormat()
        : header(0)
        , stringFormatFlags(0)
        , language(0)
        , stringAlignment(0)
        , lineAlign(0)
        , digitSubstitution(0)
        , digitLanguage(0)
        , firstTabOffset(0.0)
        , hotkeyPrefix(0)
        , leadingMargin(0.0)
        , trailingMargin(0.0)
        , tracking(1.0)
        , trimming(0)
        , tabStopCount(0)
        , rangeCount(0)
    {
    }

    static OUString StringFormatFlags(sal_uInt32 flag)
    {
        OUStringBuffer sFlags;

        if (flag & StringFormatDirectionRightToLeft)
            sFlags.append("StringFormatDirectionRightToLeft");

        if (flag & StringFormatDirectionRightToLeft)
        {
            if (!sFlags.isEmpty())
                sFlags.append(", ");

            sFlags.append("StringFormatDirectionRightToLeft");
        }

        if (flag & StringFormatNoFitBlackBox)
        {
            if (!sFlags.isEmpty())
                sFlags.append(", ");

            sFlags.append("StringFormatNoFitBlackBox");
        }

        if (flag & StringFormatDisplayFormatControl)
        {
            if (!sFlags.isEmpty())
                sFlags.append(", ");

            sFlags.append("StringFormatDisplayFormatControl");
        }
        if (flag & StringFormatNoFontFallback)
        {
            if (!sFlags.isEmpty())
                sFlags.append(", ");

            sFlags.append("StringFormatNoFontFallback");
        }
        if (flag & StringFormatMeasureTrailingSpaces)
        {
            if (!sFlags.isEmpty())
                sFlags.append(", ");

            sFlags.append("StringFormatMeasureTrailingSpaces");
        }
        if (flag & StringFormatNoWrap)
        {
            if (!sFlags.isEmpty())
                sFlags.append(", ");

            sFlags.append("StringFormatNoWrap");
        }
        if (flag & StringFormatLineLimit)
        {
            if (!sFlags.isEmpty())
                sFlags.append(", ");

            sFlags.append("StringFormatLineLimit");
        }
        if (flag & StringFormatNoClip)
        {
            if (!sFlags.isEmpty())
                sFlags.append(", ");

            sFlags.append("StringFormatNoClip");
        }
        if (flag & StringFormatBypassGDI)
        {
            if (!sFlags.isEmpty())
                sFlags.append(", ");

            sFlags.append("StringFormatBypassGDI");
        }

        return sFlags.makeStringAndClear();
    }

    static OUString StringAlignmentString(sal_uInt32 nAlignment)
    {
        switch(nAlignment)
        {
            case StringAlignment::StringAlignmentNear:
                return "StringAlignmentNear";
            case StringAlignment::StringAlignmentCenter:
                return "StringAlignmentCenter";
            case StringAlignment::StringAlignmentFar:
                return "StringAlignmentFar";
            default:
                assert(false && nAlignment && "invalid string alignment value");
                return "INVALID";
        }
    }

    static OUString DigitSubstitutionString(sal_uInt32 nSubst)
    {
        switch(nSubst)
        {
            case StringDigitSubstitution::StringDigitSubstitutionUser:
                return "StringDigitSubstitutionUser";
            case StringDigitSubstitution::StringDigitSubstitutionNone:
                return "StringDigitSubstitutionNone";
            case StringDigitSubstitution::StringDigitSubstitutionNational:
                return "StringDigitSubstitutionNational";
            case StringDigitSubstitution::StringDigitSubstitutionTraditional:
                return "StringDigitSubstitutionTraditional";
            default:
                assert(false && nSubst && "invalid string digit substitution value");
                return "INVALID";
        }
    }

    static OUString HotkeyPrefixString(sal_uInt32 nHotkey)
    {
        switch(nHotkey)
        {
            case HotkeyPrefix::HotkeyPrefixNone:
                return "HotkeyPrefixNone";
            case HotkeyPrefix::HotkeyPrefixShow:
                return "HotkeyPrefixShow";
            case HotkeyPrefix::HotkeyPrefixHide:
                return "HotkeyPrefixHide";
            default:
                assert(false && nHotkey && "invalid hotkey prefix value");
                return "INVALID";
        }
    }

    static OUString StringTrimmingString(sal_uInt32 nTrimming)
    {
        switch(nTrimming)
        {
            case StringTrimming::StringTrimmingNone:
                return "StringTrimmingNone";
            case StringTrimming::StringTrimmingCharacter:
                return "StringTrimmingCharacter";
            case StringTrimming::StringTrimmingWord:
                return "StringTrimmingWord";
            case StringTrimming::StringTrimmingEllipsisCharacter:
                return "StringTrimmingEllipsisCharacter";
            case StringTrimming::StringTrimmingEllipsisWord:
                return "StringTrimmingEllipsisWord";
            case StringTrimming::StringTrimmingEllipsisPath:
                return "StringTrimmingEllipsisPath";
            default:
                assert(false && nTrimming && "invalid trim value");
                return "INVALID";
        }
    }

    void EMFPStringFormat::Read(SvMemoryStream &s)
    {
        s.ReadUInt32(header).ReadUInt32(stringFormatFlags).ReadUInt32(language);
        s.ReadUInt32(stringAlignment).ReadUInt32(lineAlign).ReadUInt32(digitSubstitution).ReadUInt32(digitLanguage);
        s.ReadFloat(firstTabOffset).ReadInt32(hotkeyPrefix).ReadFloat(leadingMargin).ReadFloat(trailingMargin).ReadFloat(tracking);
        s.ReadInt32(trimming).ReadInt32(tabStopCount).ReadInt32(rangeCount);
        // keep only the last 16 bits of language
        language >>= 16;
        digitLanguage >>= 16;
        SAL_WARN_IF((header >> 12) != 0xdbc01, "drawinglayer", "Invalid header - not 0xdbc01");
        SAL_INFO("drawinglayer", "EMF+\tString format");
        SAL_INFO("drawinglayer", "EMF+\t\tHeader: 0x" << std::hex << (header >> 12));
        SAL_INFO("drawinglayer", "EMF+\t\tVersion: 0x" << (header & 0x1fff) << std::dec);
        SAL_INFO("drawinglayer", "EMF+\t\tStringFormatFlags: " << StringFormatFlags(stringFormatFlags));
        SAL_INFO("drawinglayer", "EMF+\t\tLanguage: sublangid: 0x" << std::hex << (language >> 10) << ", primarylangid: 0x" << (language & 0xF800));
        SAL_INFO("drawinglayer", "EMF+\t\tLineAlign: " << StringAlignmentString(lineAlign));
        SAL_INFO("drawinglayer", "EMF+\t\tDigitSubstitution: " << DigitSubstitutionString(digitSubstitution));
        SAL_INFO("drawinglayer", "EMF+\t\tDigitLanguage: sublangid: 0x" << std::hex << (digitLanguage >> 10) << ", primarylangid: 0x" << (digitLanguage & 0xF800));
        SAL_INFO("drawinglayer", "EMF+\t\tFirstTabOffset: " << firstTabOffset);
        SAL_INFO("drawinglayer", "EMF+\t\tHotkeyPrefix: " << HotkeyPrefixString(hotkeyPrefix));
        SAL_INFO("drawinglayer", "EMF+\t\tLeadingMargin: " << leadingMargin);
        SAL_INFO("drawinglayer", "EMF+\t\tTrailingMargin: " << trailingMargin);
        SAL_INFO("drawinglayer", "EMF+\t\tTracking: " << tracking);
        SAL_INFO("drawinglayer", "EMF+\t\tTrimming: " << StringTrimmingString(trimming));
        SAL_INFO("drawinglayer", "EMF+\t\tTabStopCount: " << tabStopCount);
        SAL_INFO("drawinglayer", "EMF+\t\tRangeCount: " << rangeCount);

        SAL_WARN_IF(stringAlignment != StringAlignment::StringAlignmentNear, "drawinglayer", "EMF+\t TODO EMFPStringFormat:StringAlignment");
        SAL_WARN_IF(lineAlign != StringAlignment::StringAlignmentNear, "drawinglayer", "EMF+\t TODO EMFPStringFormat:lineAlign");
        SAL_WARN_IF(digitSubstitution != StringDigitSubstitution::StringDigitSubstitutionNone,
                        "drawinglayer", "EMF+\t TODO EMFPStringFormat:digitSubstitution");
        SAL_WARN_IF(firstTabOffset != 0.0, "drawinglayer", "EMF+\t TODO EMFPStringFormat:firstTabOffset");
        SAL_WARN_IF(hotkeyPrefix != HotkeyPrefix::HotkeyPrefixNone, "drawinglayer", "EMF+\t TODO EMFPStringFormat:hotkeyPrefix");
        SAL_WARN_IF(leadingMargin != 0.0, "drawinglayer", "EMF+\t TODO EMFPStringFormat:leadingMargin");
        SAL_WARN_IF(trailingMargin != 0.0, "drawinglayer", "EMF+\t TODO EMFPStringFormat:trailingMargin");
        SAL_WARN_IF(tracking != 1.0, "drawinglayer", "EMF+\t TODO EMFPStringFormat:tracking");
        SAL_WARN_IF(trimming != StringTrimming::StringTrimmingNone, "drawinglayer", "EMF+\t TODO EMFPStringFormat:trimming");
        SAL_WARN_IF(tabStopCount, "drawinglayer", "EMF+\t TODO EMFPStringFormat:tabStopCount");
        SAL_WARN_IF(rangeCount, "drawinglayer", "EMF+\t TODO EMFPStringFormat:StringFormatData");
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
