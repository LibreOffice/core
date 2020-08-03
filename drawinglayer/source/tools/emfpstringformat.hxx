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

#include "emfphelperdata.hxx"

namespace emfplushelper
{
    const sal_uInt32 StringFormatDirectionRightToLeft = 0x00000001;
    const sal_uInt32 StringFormatDirectionVertical = 0x00000002;
    const sal_uInt32 StringFormatNoFitBlackBox = 0x00000004;
    const sal_uInt32 StringFormatDisplayFormatControl = 0x00000020;
    const sal_uInt32 StringFormatNoFontFallback = 0x00000400;
    const sal_uInt32 StringFormatMeasureTrailingSpaces = 0x00000800;
    const sal_uInt32 StringFormatNoWrap = 0x00001000;
    const sal_uInt32 StringFormatLineLimit = 0x00002000;
    const sal_uInt32 StringFormatNoClip = 0x00004000;
    const sal_uInt32 StringFormatBypassGDI = 0x80000000;

    enum StringAlignment
    {
        StringAlignmentNear = 0x00000000,
        StringAlignmentCenter = 0x00000001,
        StringAlignmentFar = 0x00000002
    };

    enum StringDigitSubstitution
    {
        StringDigitSubstitutionUser = 0x00000000,
        StringDigitSubstitutionNone = 0x00000001,
        StringDigitSubstitutionNational = 0x00000002,
        StringDigitSubstitutionTraditional = 0x00000003
    };

    enum HotkeyPrefix
    {
        HotkeyPrefixNone = 0x00000000,
        HotkeyPrefixShow = 0x00000001,
        HotkeyPrefixHide = 0x00000002
    };

    enum StringTrimming
    {
        StringTrimmingNone = 0x00000000,
        StringTrimmingCharacter = 0x00000001,
        StringTrimmingWord = 0x00000002,
        StringTrimmingEllipsisCharacter = 0x00000003,
        StringTrimmingEllipsisWord = 0x00000004,
        StringTrimmingEllipsisPath = 0x00000005
    };

    struct EMFPStringFormat : public EMFPObject
    {
        sal_uInt32 header;
        sal_uInt32 stringFormatFlags;
        sal_uInt32 language;
        sal_uInt32 stringAlignment; // Horizontal alignment
        sal_uInt32 lineAlign;       // Vertical alignment
        sal_uInt32 digitSubstitution;
        sal_uInt32 digitLanguage;
        float firstTabOffset;
        sal_Int32 hotkeyPrefix;
        float leadingMargin;        // Length of the space to add to the starting position of a string.
        float trailingMargin;       // Length of the space to leave following a string.
        float tracking;
        sal_Int32 trimming;
        sal_Int32 tabStopCount;
        sal_Int32 rangeCount;

        EMFPStringFormat();
        void Read(SvMemoryStream &s);

        // flags table from MS-EMFPLUS doc
        bool DirectionRightToLeft()  const { return stringFormatFlags & 0x00000001;}
        bool DirectionVertical()     const { return stringFormatFlags & 0x00000002;}
        bool NoFitBlackBox()         const { return stringFormatFlags & 0x00000004;}
        bool DisplayFormatControl()  const { return stringFormatFlags & 0x00000020;}
        bool NoFontFallback()        const { return stringFormatFlags & 0x00000400;}
        bool MeasureTrailingSpaces() const { return stringFormatFlags & 0x00000800;}
        bool NoWrap()                const { return stringFormatFlags & 0x00001000;}
        bool LineLimit()             const { return stringFormatFlags & 0x00002000;}
        bool NoClip()                const { return stringFormatFlags & 0x00004000;}
        bool BypassGDI()             const { return stringFormatFlags & 0x80000000;}
    };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
