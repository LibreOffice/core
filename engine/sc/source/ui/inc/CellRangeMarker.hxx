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

#include <address.hxx>
#include <rtl/string.hxx>
#include <tools/color.hxx>

#include <vector>

class ScViewData;

namespace sc
{
/** One kind of cell marker: its name, the ranges it marks out on the sheet nPart, and how they
    are drawn. A fFillOpacity fills the ranges with the color, otherwise they get a border in it.
    An aHandleCommand puts a drag handle on the corner of each range, and a drag of one sends
    that command with the range it ended on as the CellRange parameter.
 */
struct CellRangeMarkerOptions
{
    OString aName;
    std::vector<ScRange> aCellRanges;
    SCTAB nPart = 0;
    Color aColor = COL_AUTO;
    bool bDashed = false;
    double fFillOpacity = 0.0;
    OString aHandleCommand;
};

/** Send a marker to the client. It replaces the ranges under its name, and none clears the
    kind. The ranges are cell indexes, not pixels.
 */
void notifyCellRangeMarker(const ScViewData& rViewData, const CellRangeMarkerOptions& rOptions);

} // namespace sc

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
