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
    // SvxContourItem -> SfxBoolItem
    // SvxShadowedItem -> SfxBoolItem

    namespace SvxBrush
    {
        void UNLESS_MERGELIBS(EDITENG_DLLPUBLIC) Create(SvxBrushItem& rItem, SvStream& rStrm, sal_uInt16 nItemVersion);
        UNLESS_MERGELIBS(EDITENG_DLLPUBLIC) SvStream& Store(const SvxBrushItem& rItem, SvStream& rStrm, sal_uInt16 nItemVersion);
    }

}

#endif // INCLUDED_EDITENG_LEGACYITEM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
