/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <o3tl/unit_conversion.hxx>
#include <sfx2/sfxsids.hrc>
#include <svl/intitem.hxx>
#include <svl/itemset.hxx>
#include <tools/gen.hxx>

#include <cstdlib>
#include <optional>

namespace svx
{
/** The rectangle that an object dragged out in the client is to be created in, in 1/100 mm. The
    request gives it in twips on the page as X, Y, Width and Height. The area of the page does not
    limit it: a shape may lie beside a slide, and a right-to-left sheet lies left of nought.
 */
inline std::optional<tools::Rectangle> getRequestedRectangle(const SfxItemSet* pArgs)
{
    if (!pArgs)
        return std::nullopt;

    const SfxInt32Item* pXItem = pArgs->GetItem<SfxInt32Item>(FN_PARAM_2, false);
    const SfxInt32Item* pYItem = pArgs->GetItem<SfxInt32Item>(FN_PARAM_3, false);
    const SfxInt32Item* pWidthItem = pArgs->GetItem<SfxInt32Item>(FN_PARAM_4, false);
    const SfxInt32Item* pHeightItem = pArgs->GetItem<SfxInt32Item>(FN_PARAM_5, false);
    if (!pXItem || !pYItem || !pWidthItem || !pHeightItem)
        return std::nullopt;

    constexpr sal_Int32 nMostTwips = 10000000;
    const sal_Int32 nX(pXItem->GetValue());
    const sal_Int32 nY(pYItem->GetValue());
    const sal_Int32 nWidth(pWidthItem->GetValue());
    const sal_Int32 nHeight(pHeightItem->GetValue());
    if (nWidth <= 0 || nHeight <= 0 || std::abs(nX) > nMostTwips || std::abs(nY) > nMostTwips
        || nWidth > nMostTwips || nHeight > nMostTwips)
        return std::nullopt;

    auto toMm100 = [](sal_Int32 nTwips) {
        return static_cast<tools::Long>(
            o3tl::convert(nTwips, o3tl::Length::twip, o3tl::Length::mm100));
    };
    return tools::Rectangle(Point(toMm100(nX), toMm100(nY)),
                            Size(toMm100(nWidth), toMm100(nHeight)));
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
