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

#include <editeng/boxitem.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/contouritem.hxx>
#include <editeng/crossedoutitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/justifyitem.hxx>
#include <editeng/lineitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/shdditem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/wghtitem.hxx>
#include <svl/typedwhich.hxx>
#include <svx/algitem.hxx>

class SvxFontItem;
class SvxFontHeightItem;
class SvxWeightItem;
class SvxPostureItem;
class SvxUnderlineItem;
class SvxBrushItem;
class SvxVerJustifyItem;
class SvxColorItem;
class SvxBoxItem;
class SvxLineItem;

inline constexpr sal_uInt16 SVX_TABLE_START(3000);

inline constexpr TypedWhichId<SvxFontItem> SVX_TABLE_FONT(SVX_TABLE_START + 1);
inline constexpr TypedWhichId<SvxFontHeightItem> SVX_TABLE_FONT_HEIGHT(SVX_TABLE_START + 2);
inline constexpr TypedWhichId<SvxWeightItem> SVX_TABLE_FONT_WEIGHT(SVX_TABLE_START + 3);
inline constexpr TypedWhichId<SvxPostureItem> SVX_TABLE_FONT_POSTURE(SVX_TABLE_START + 4);
inline constexpr TypedWhichId<SvxFontItem> SVX_TABLE_CJK_FONT(SVX_TABLE_START + 5);
inline constexpr TypedWhichId<SvxFontHeightItem> SVX_TABLE_CJK_FONT_HEIGHT(SVX_TABLE_START + 6);
inline constexpr TypedWhichId<SvxWeightItem> SVX_TABLE_CJK_FONT_WEIGHT(SVX_TABLE_START + 7);
inline constexpr TypedWhichId<SvxPostureItem> SVX_TABLE_CJK_FONT_POSTURE(SVX_TABLE_START + 8);
inline constexpr TypedWhichId<SvxFontItem> SVX_TABLE_CTL_FONT(SVX_TABLE_START + 9);
inline constexpr TypedWhichId<SvxFontHeightItem> SVX_TABLE_CTL_FONT_HEIGHT(SVX_TABLE_START + 10);
inline constexpr TypedWhichId<SvxWeightItem> SVX_TABLE_CTL_FONT_WEIGHT(SVX_TABLE_START + 11);
inline constexpr TypedWhichId<SvxPostureItem> SVX_TABLE_CTL_FONT_POSTURE(SVX_TABLE_START + 12);
inline constexpr TypedWhichId<SvxUnderlineItem> SVX_TABLE_FONT_UNDERLINE(SVX_TABLE_START + 13);
inline constexpr TypedWhichId<SvxColorItem> SVX_TABLE_FONT_COLOR(SVX_TABLE_START + 14);
inline constexpr TypedWhichId<SvxBrushItem> SVX_TABLE_BACKGROUND(SVX_TABLE_START + 15);
inline constexpr TypedWhichId<SvxHorJustifyItem> SVX_TABLE_HOR_JUSTIFY(SVX_TABLE_START + 16);
inline constexpr TypedWhichId<SvxVerJustifyItem> SVX_TABLE_VER_JUSTIFY(SVX_TABLE_START + 17);
inline constexpr TypedWhichId<SvxBoxItem> SVX_TABLE_BOX(SVX_TABLE_START + 18);
inline constexpr TypedWhichId<SvxLineItem> SVX_TABLE_PARA_ADJUST(SVX_TABLE_START + 19);
inline constexpr TypedWhichId<SvxMarginItem> SVX_TABLE_MARGIN(SVX_TABLE_START + 20);

inline constexpr sal_uInt16 SVX_TABLE_LAST(SVX_TABLE_PARA_ADJUST);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
