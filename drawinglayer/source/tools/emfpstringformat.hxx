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

#ifndef INCLUDED_DRAWINGLAYER_SOURCE_TOOLS_EMFPSTRINGFORMAT_HXX
#define INCLUDED_DRAWINGLAYER_SOURCE_TOOLS_EMFPSTRINGFORMAT_HXX

#include "emfphelperdata.hxx"

namespace emfplushelper
{
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

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
