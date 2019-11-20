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
        SAL_INFO("drawinglayer", "EMF+\t string format\nEMF+\theader: 0x" << std::hex << (header >> 12) << " version: 0x" << (header & 0x1fff) << " StringFormatFlags: 0x" << stringFormatFlags << std::dec << " Language: " << language);
        SAL_INFO("drawinglayer", "EMF+\t StringAlignment: " << stringAlignment << " LineAlign: " << lineAlign << " DigitSubstitution: " << digitSubstitution << " DigitLanguage: " << digitLanguage);
        SAL_INFO("drawinglayer", "EMF+\t FirstTabOffset: " << firstTabOffset << " HotkeyPrefix: " << hotkeyPrefix << " LeadingMargin: " << leadingMargin << " TrailingMargin: " << trailingMargin << " Tracking: " << tracking);
        SAL_INFO("drawinglayer", "EMF+\t Trimming: " << trimming << " TabStopCount: " << tabStopCount << " RangeCount: " << rangeCount);

        SAL_WARN_IF(stringAlignment, "drawinglayer", "EMF+\t TODO EMFPStringFormat:StringAlignment");
        SAL_WARN_IF(lineAlign, "drawinglayer", "EMF+\t TODO EMFPStringFormat:lineAlign");
        SAL_WARN_IF(digitSubstitution, "drawinglayer", "EMF+\t TODO EMFPStringFormat:digitSubstitution");
        SAL_WARN_IF(firstTabOffset != 0.0, "drawinglayer", "EMF+\t TODO EMFPStringFormat:firstTabOffset");
        SAL_WARN_IF(hotkeyPrefix, "drawinglayer", "EMF+\t TODO EMFPStringFormat:hotkeyPrefix");
        SAL_WARN_IF(leadingMargin != 0.0, "drawinglayer", "EMF+\t TODO EMFPStringFormat:leadingMargin");
        SAL_WARN_IF(trailingMargin != 0.0, "drawinglayer", "EMF+\t TODO EMFPStringFormat:trailingMargin");
        SAL_WARN_IF(tracking != 1.0, "drawinglayer", "EMF+\t TODO EMFPStringFormat:tracking");
        SAL_WARN_IF(trimming, "drawinglayer", "EMF+\t TODO EMFPStringFormat:trimming");
        SAL_WARN_IF(tabStopCount, "drawinglayer", "EMF+\t TODO EMFPStringFormat:tabStopCount");
        SAL_WARN_IF(rangeCount, "drawinglayer", "EMF+\t TODO EMFPStringFormat:StringFormatData");
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
