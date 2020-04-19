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
#include "emfpfont.hxx"

namespace emfplushelper
{
    static OUString FontStyleToString(sal_uInt32 style)
    {
        OUString sStyle;

        if (style & FontStyleBold)
            sStyle = "\n\t\t\tFontStyleBold";

        if (style & FontStyleItalic)
            sStyle = sStyle.concat("\n\t\t\tFontStyleItalic");

        if (style & FontStyleUnderline)
            sStyle = sStyle.concat("\n\t\t\tFontStyleUnderline");

        if (style & FontStyleStrikeout)
            sStyle = sStyle.concat("\n\t\t\tFontStyleStrikeout");

        return sStyle;
    }

    void EMFPFont::Read(SvMemoryStream &s)
    {
        sal_uInt32 header;
        sal_uInt32 reserved;
        sal_uInt32 length;
        s.ReadUInt32(header).ReadFloat(emSize).ReadUInt32(sizeUnit).ReadInt32(fontFlags).ReadUInt32(reserved).ReadUInt32(length);
        SAL_WARN_IF((header >> 12) != 0xdbc01, "drawinglayer", "Invalid header - not 0xdbc01");
        SAL_INFO("drawinglayer", "EMF+\tHeader: 0x" << std::hex << (header >> 12));
        SAL_INFO("drawinglayer", "EMF+\tVersion: 0x" << (header & 0x1fff));
        SAL_INFO("drawinglayer", "EMF+\tSize: " << std::dec << emSize);
        SAL_INFO("drawinglayer", "EMF+\tUnit: " << UnitTypeToString(sizeUnit) << " (0x" << std::hex << sizeUnit << ")" << std::dec);
        SAL_INFO("drawinglayer", "EMF+\tFlags: " << FontStyleToString(fontFlags) << " (0x" << std::hex << fontFlags << ")");
        SAL_INFO("drawinglayer", "EMF+\tReserved: 0x" << reserved << std::dec);
        SAL_INFO("drawinglayer", "EMF+\tLength: " << length);

        if (length <= 0 || length >= 0x4000)
            return;

        rtl_uString *pStr = rtl_uString_alloc(length);
        sal_Unicode *chars = pStr->buffer;

        for (sal_uInt32 i = 0; i < length; ++i)
        {
            s.ReadUtf16(chars[i]);
        }

        family = OUString(pStr, SAL_NO_ACQUIRE);
        SAL_INFO("drawinglayer", "EMF+\tFamily: " << family);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
