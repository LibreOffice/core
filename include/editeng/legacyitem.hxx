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
#ifndef INCLUDED_EDITENG_LEGACYITEM_HXX
#define INCLUDED_EDITENG_LEGACYITEM_HXX

#include <config_options.h>
#include <editeng/editengdllapi.h>

//////////////////////////////////////////////////////////////////////////////
// // editeng
//     SvxFontItem                 aFont;
//     SvxFontHeightItem           aHeight;
//     SvxWeightItem               aWeight;
//     SvxPostureItem              aPosture;
//     SvxUnderlineItem            aUnderline;
//     SvxOverlineItem             aOverline;
//     SvxCrossedOutItem           aCrossedOut;
//     SvxContourItem              aContour;
//     SvxShadowedItem             aShadowed;
//     SvxColorItem                aColor;
//     SvxBoxItem                  aBox;
//     SvxLineItem                 aTLBR;
//     SvxBrushItem                aBackground;
//     SvxAdjustItem               aAdjust;
//     SvxHorJustifyItem           aHorJustify;
//     SvxVerJustifyItem           aVerJustify;
//     SvxFrameDirectionItem m_aTextOrientation;
//////////////////////////////////////////////////////////////////////////////

class SvStream;
class SvxFontItem;
class SvxFontHeightItem;
class SvxWeightItem;
class SvxPostureItem;
class SvxTextLineItem;
class SvxCrossedOutItem;
class SvxColorItem;
class SvxBoxItem;
class SvxLineItem;
class SvxBrushItem;
class SvxAdjustItem;
class SvxHorJustifyItem;
class SvxVerJustifyItem;
class SvxFrameDirectionItem;
class SvxFormatBreakItem;
class SvxFormatKeepItem;
class SvxShadowItem;

namespace legacy
{
    namespace SvxFont
    {
        sal_uInt16 UNLESS_MERGELIBS(EDITENG_DLLPUBLIC) GetVersion(sal_uInt16 nFileFormatVersion);
        void UNLESS_MERGELIBS(EDITENG_DLLPUBLIC) Create(SvxFontItem& rItem, SvStream& rStrm, sal_uInt16 nItemVersion);
        UNLESS_MERGELIBS(EDITENG_DLLPUBLIC) SvStream& Store(const SvxFontItem& rItem, SvStream& rStrm, sal_uInt16 nItemVersion);
    }

    namespace SvxFontHeight
    {
        sal_uInt16 UNLESS_MERGELIBS(EDITENG_DLLPUBLIC) GetVersion(sal_uInt16 nFileFormatVersion);
        void UNLESS_MERGELIBS(EDITENG_DLLPUBLIC) Create(SvxFontHeightItem& rItem, SvStream& rStrm, sal_uInt16 nItemVersion);
        UNLESS_MERGELIBS(EDITENG_DLLPUBLIC) SvStream& Store(const SvxFontHeightItem& rItem, SvStream& rStrm, sal_uInt16 nItemVersion);
    }

    namespace SvxWeight
    {
        sal_uInt16 UNLESS_MERGELIBS(EDITENG_DLLPUBLIC) GetVersion(sal_uInt16 nFileFormatVersion);
        void UNLESS_MERGELIBS(EDITENG_DLLPUBLIC) Create(SvxWeightItem& rItem, SvStream& rStrm, sal_uInt16 nItemVersion);
        UNLESS_MERGELIBS(EDITENG_DLLPUBLIC) SvStream& Store(const SvxWeightItem& rItem, SvStream& rStrm, sal_uInt16 nItemVersion);
    }

    namespace SvxPosture
    {
        sal_uInt16 UNLESS_MERGELIBS(EDITENG_DLLPUBLIC) GetVersion(sal_uInt16 nFileFormatVersion);
        void UNLESS_MERGELIBS(EDITENG_DLLPUBLIC) Create(SvxPostureItem& rItem, SvStream& rStrm, sal_uInt16 nItemVersion);
        UNLESS_MERGELIBS(EDITENG_DLLPUBLIC) SvStream& Store(const SvxPostureItem& rItem, SvStream& rStrm, sal_uInt16 nItemVersion);
    }

    namespace SvxTextLine // SvxUnderlineItem, SvxOverlineItem -> SvxTextLineItem
    {
        sal_uInt16 UNLESS_MERGELIBS(EDITENG_DLLPUBLIC) GetVersion(sal_uInt16 nFileFormatVersion);
        void UNLESS_MERGELIBS(EDITENG_DLLPUBLIC) Create(SvxTextLineItem& rItem, SvStream& rStrm, sal_uInt16 nItemVersion);
        UNLESS_MERGELIBS(EDITENG_DLLPUBLIC) SvStream& Store(const SvxTextLineItem& rItem, SvStream& rStrm, sal_uInt16 nItemVersion);
    }

    namespace SvxCrossedOut
    {
        sal_uInt16 UNLESS_MERGELIBS(EDITENG_DLLPUBLIC) GetVersion(sal_uInt16 nFileFormatVersion);
        void UNLESS_MERGELIBS(EDITENG_DLLPUBLIC) Create(SvxCrossedOutItem& rItem, SvStream& rStrm, sal_uInt16 nItemVersion);
        UNLESS_MERGELIBS(EDITENG_DLLPUBLIC) SvStream& Store(const SvxCrossedOutItem& rItem, SvStream& rStrm, sal_uInt16 nItemVersion);
    }

    // SvxContourItem -> SfxBoolItem
    // SvxShadowedItem -> SfxBoolItem

    namespace SvxColor
    {
        sal_uInt16 UNLESS_MERGELIBS(EDITENG_DLLPUBLIC) GetVersion(sal_uInt16 nFileFormatVersion);
        void UNLESS_MERGELIBS(EDITENG_DLLPUBLIC) Create(SvxColorItem& rItem, SvStream& rStrm, sal_uInt16 nItemVersion);
        UNLESS_MERGELIBS(EDITENG_DLLPUBLIC) SvStream& Store(const SvxColorItem& rItem, SvStream& rStrm, sal_uInt16 nItemVersion);
    }

    namespace SvxBox
    {
        sal_uInt16 UNLESS_MERGELIBS(EDITENG_DLLPUBLIC) GetVersion(sal_uInt16 nFileFormatVersion);
        void UNLESS_MERGELIBS(EDITENG_DLLPUBLIC) Create(SvxBoxItem& rItem, SvStream& rStrm, sal_uInt16 nItemVersion);
        UNLESS_MERGELIBS(EDITENG_DLLPUBLIC) SvStream& Store(const SvxBoxItem& rItem, SvStream& rStrm, sal_uInt16 nItemVersion);
    }

    namespace SvxLine
    {
        sal_uInt16 UNLESS_MERGELIBS(EDITENG_DLLPUBLIC) GetVersion(sal_uInt16 nFileFormatVersion);
        void UNLESS_MERGELIBS(EDITENG_DLLPUBLIC) Create(SvxLineItem& rItem, SvStream& rStrm, sal_uInt16 nItemVersion);
        UNLESS_MERGELIBS(EDITENG_DLLPUBLIC) SvStream& Store(const SvxLineItem& rItem, SvStream& rStrm, sal_uInt16 nItemVersion);
    }

    namespace SvxBrush
    {
        sal_uInt16 UNLESS_MERGELIBS(EDITENG_DLLPUBLIC) GetVersion(sal_uInt16 nFileFormatVersion);
        void UNLESS_MERGELIBS(EDITENG_DLLPUBLIC) Create(SvxBrushItem& rItem, SvStream& rStrm, sal_uInt16 nItemVersion);
        UNLESS_MERGELIBS(EDITENG_DLLPUBLIC) SvStream& Store(const SvxBrushItem& rItem, SvStream& rStrm, sal_uInt16 nItemVersion);
    }

    namespace SvxAdjust
    {
        sal_uInt16 UNLESS_MERGELIBS(EDITENG_DLLPUBLIC) GetVersion(sal_uInt16 nFileFormatVersion);
        void UNLESS_MERGELIBS(EDITENG_DLLPUBLIC) Create(SvxAdjustItem& rItem, SvStream& rStrm, sal_uInt16 nItemVersion);
        UNLESS_MERGELIBS(EDITENG_DLLPUBLIC) SvStream& Store(const SvxAdjustItem& rItem, SvStream& rStrm, sal_uInt16 nItemVersion);
    }

    namespace SvxHorJustify
    {
        sal_uInt16 UNLESS_MERGELIBS(EDITENG_DLLPUBLIC) GetVersion(sal_uInt16 nFileFormatVersion);
        void UNLESS_MERGELIBS(EDITENG_DLLPUBLIC) Create(SvxHorJustifyItem& rItem, SvStream& rStrm, sal_uInt16 nItemVersion);
        UNLESS_MERGELIBS(EDITENG_DLLPUBLIC) SvStream& Store(const SvxHorJustifyItem& rItem, SvStream& rStrm, sal_uInt16 nItemVersion);
    }

    namespace SvxVerJustify
    {
        sal_uInt16 UNLESS_MERGELIBS(EDITENG_DLLPUBLIC) GetVersion(sal_uInt16 nFileFormatVersion);
        void UNLESS_MERGELIBS(EDITENG_DLLPUBLIC) Create(SvxVerJustifyItem& rItem, SvStream& rStrm, sal_uInt16 nItemVersion);
        UNLESS_MERGELIBS(EDITENG_DLLPUBLIC) SvStream& Store(const SvxVerJustifyItem& rItem, SvStream& rStrm, sal_uInt16 nItemVersion);
    }

    namespace SvxFrameDirection
    {
        sal_uInt16 EDITENG_DLLPUBLIC GetVersion(sal_uInt16 nFileFormatVersion);
        void EDITENG_DLLPUBLIC Create(SvxFrameDirectionItem& rItem, SvStream& rStrm, sal_uInt16 nItemVersion);
        EDITENG_DLLPUBLIC SvStream& Store(const SvxFrameDirectionItem& rItem, SvStream& rStrm, sal_uInt16 nItemVersion);
    }

    namespace SvxFormatBreak
    {
        sal_uInt16 EDITENG_DLLPUBLIC GetVersion(sal_uInt16 nFileFormatVersion);
        void EDITENG_DLLPUBLIC Create(SvxFormatBreakItem& rItem, SvStream& rStrm, sal_uInt16 nItemVersion);
        EDITENG_DLLPUBLIC SvStream& Store(const SvxFormatBreakItem& rItem, SvStream& rStrm, sal_uInt16 nItemVersion);
    }

    namespace SvxFormatKeep
    {
        sal_uInt16 EDITENG_DLLPUBLIC GetVersion(sal_uInt16 nFileFormatVersion);
        void EDITENG_DLLPUBLIC Create(SvxFormatKeepItem& rItem, SvStream& rStrm, sal_uInt16 nItemVersion);
        EDITENG_DLLPUBLIC SvStream& Store(const SvxFormatKeepItem& rItem, SvStream& rStrm, sal_uInt16 nItemVersion);
    }

    namespace SvxShadow
    {
        sal_uInt16 EDITENG_DLLPUBLIC GetVersion(sal_uInt16 nFileFormatVersion);
        void EDITENG_DLLPUBLIC Create(SvxShadowItem& rItem, SvStream& rStrm, sal_uInt16 nItemVersion);
        EDITENG_DLLPUBLIC SvStream& Store(const SvxShadowItem& rItem, SvStream& rStrm, sal_uInt16 nItemVersion);
    }
}

#endif // INCLUDED_EDITENG_LEGACYITEM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
