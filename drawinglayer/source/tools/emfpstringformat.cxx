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

        // These are extracted from enum in emfpstringformat.hxx
        if (flag & StringFormatDirectionRightToLeft)
            sFlags.append("StringFormatDirectionRightToLeft ");

        if (flag & StringFormatDirectionVertical)
            sFlags.append("StringFormatDirectionVertical ");

        if (flag & StringFormatNoFitBlackBox)
            sFlags.append("StringFormatNoFitBlackBox ");

        if (flag & StringFormatDisplayFormatControl)
            sFlags.append("StringFormatDisplayFormatControl ");

        if (flag & StringFormatNoFontFallback)
            sFlags.append("StringFormatNoFontFallback ");

        if (flag & StringFormatMeasureTrailingSpaces)
            sFlags.append("StringFormatMeasureTrailingSpaces ");

        if (flag & StringFormatNoWrap)
            sFlags.append("StringFormatNoWrap ");

        if (flag & StringFormatLineLimit)
            sFlags.append("StringFormatLineLimit ");

        if (flag & StringFormatNoClip)
            sFlags.append("StringFormatNoClip ");

        if (flag & StringFormatBypassGDI)
            sFlags.append("StringFormatBypassGDI ");

        // There will be 1 extra space in the end. It could be truncated, but
        // as it is for SAL_INFO() only, it would not be important
        return sFlags.makeStringAndClear();
    }

    static OUString StringAlignmentString(sal_uInt32 nAlignment)
    {
        switch(nAlignment)
        {
            case StringAlignment::StringAlignmentNear:
                return u"StringAlignmentNear"_ustr;
            case StringAlignment::StringAlignmentCenter:
                return u"StringAlignmentCenter"_ustr;
            case StringAlignment::StringAlignmentFar:
                return u"StringAlignmentFar"_ustr;
            default:
                assert(false && nAlignment && "invalid string alignment value");
                return u"INVALID"_ustr;
        }
    }

    static OUString DigitSubstitutionString(sal_uInt32 nSubst)
    {
        switch(nSubst)
        {
            case StringDigitSubstitution::StringDigitSubstitutionUser:
                return u"StringDigitSubstitutionUser"_ustr;
            case StringDigitSubstitution::StringDigitSubstitutionNone:
                return u"StringDigitSubstitutionNone"_ustr;
            case StringDigitSubstitution::StringDigitSubstitutionNational:
                return u"StringDigitSubstitutionNational"_ustr;
            case StringDigitSubstitution::StringDigitSubstitutionTraditional:
                return u"StringDigitSubstitutionTraditional"_ustr;
            default:
                assert(false && nSubst && "invalid string digit substitution value");
                return u"INVALID"_ustr;
        }
    }

    static OUString HotkeyPrefixString(sal_uInt32 nHotkey)
    {
        switch(nHotkey)
        {
            case HotkeyPrefix::HotkeyPrefixNone:
                return u"HotkeyPrefixNone"_ustr;
            case HotkeyPrefix::HotkeyPrefixShow:
                return u"HotkeyPrefixShow"_ustr;
            case HotkeyPrefix::HotkeyPrefixHide:
                return u"HotkeyPrefixHide"_ustr;
            default:
                assert(false && nHotkey && "invalid hotkey prefix value");
                return u"INVALID"_ustr;
        }
    }

    static OUString StringTrimmingString(sal_uInt32 nTrimming)
    {
        switch(nTrimming)
        {
            case StringTrimming::StringTrimmingNone:
                return u"StringTrimmingNone"_ustr;
            case StringTrimming::StringTrimmingCharacter:
                return u"StringTrimmingCharacter"_ustr;
            case StringTrimming::StringTrimmingWord:
                return u"StringTrimmingWord"_ustr;
            case StringTrimming::StringTrimmingEllipsisCharacter:
                return u"StringTrimmingEllipsisCharacter"_ustr;
            case StringTrimming::StringTrimmingEllipsisWord:
                return u"StringTrimmingEllipsisWord"_ustr;
            case StringTrimming::StringTrimmingEllipsisPath:
                return u"StringTrimmingEllipsisPath"_ustr;
            default:
                assert(false && nTrimming && "invalid trim value");
                return u"INVALID"_ustr;
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
        SAL_WARN_IF((header >> 12) != 0xdbc01, "drawinglayer.emf", "Invalid header - not 0xdbc01");
        SAL_INFO("drawinglayer.emf", "EMF+\tString format");
        SAL_INFO("drawinglayer.emf", "EMF+\t\tHeader: 0x" << std::hex << (header >> 12));
        SAL_INFO("drawinglayer.emf", "EMF+\t\tVersion: 0x" << (header & 0x1fff) << std::dec);
        SAL_INFO("drawinglayer.emf", "EMF+\t\tStringFormatFlags: " << StringFormatFlags(stringFormatFlags));
        SAL_INFO("drawinglayer.emf", "EMF+\t\tLanguage: sublangid: 0x" << std::hex << (language >> 10) << ", primarylangid: 0x" << (language & 0xF800));
        SAL_INFO("drawinglayer.emf", "EMF+\t\tLineAlign: " << StringAlignmentString(lineAlign));
        SAL_INFO("drawinglayer.emf", "EMF+\t\tDigitSubstitution: " << DigitSubstitutionString(digitSubstitution));
        SAL_INFO("drawinglayer.emf", "EMF+\t\tDigitLanguage: sublangid: 0x" << std::hex << (digitLanguage >> 10) << ", primarylangid: 0x" << (digitLanguage & 0xF800));
        SAL_INFO("drawinglayer.emf", "EMF+\t\tFirstTabOffset: " << firstTabOffset);
        SAL_INFO("drawinglayer.emf", "EMF+\t\tHotkeyPrefix: " << HotkeyPrefixString(hotkeyPrefix));
        SAL_INFO("drawinglayer.emf", "EMF+\t\tLeadingMargin: " << leadingMargin);
        SAL_INFO("drawinglayer.emf", "EMF+\t\tTrailingMargin: " << trailingMargin);
        SAL_INFO("drawinglayer.emf", "EMF+\t\tTracking: " << tracking);
        SAL_INFO("drawinglayer.emf", "EMF+\t\tTrimming: " << StringTrimmingString(trimming));
        SAL_INFO("drawinglayer.emf", "EMF+\t\tTabStopCount: " << tabStopCount);
        SAL_INFO("drawinglayer.emf", "EMF+\t\tRangeCount: " << rangeCount);

        SAL_WARN_IF(digitSubstitution != StringDigitSubstitution::StringDigitSubstitutionNone,
                    "drawinglayer.emf", "EMF+\t TODO EMFPStringFormat:digitSubstitution");
        SAL_WARN_IF(firstTabOffset != 0.0, "drawinglayer.emf", "EMF+\t TODO EMFPStringFormat:firstTabOffset");
        SAL_WARN_IF(hotkeyPrefix != HotkeyPrefix::HotkeyPrefixNone, "drawinglayer.emf", "EMF+\t TODO EMFPStringFormat:hotkeyPrefix");
        SAL_WARN_IF(tracking != 1.0, "drawinglayer.emf", "EMF+\t TODO EMFPStringFormat:tracking");
        SAL_WARN_IF(trimming != StringTrimming::StringTrimmingNone, "drawinglayer.emf", "EMF+\t TODO EMFPStringFormat:trimming");
        SAL_WARN_IF(tabStopCount, "drawinglayer.emf", "EMF+\t TODO EMFPStringFormat:tabStopCount");
        SAL_WARN_IF(rangeCount != 0, "drawinglayer.emf", "EMF+\t TODO EMFPStringFormat:StringFormatData");
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
