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

#include <CellRangeMarker.hxx>

#include <tabvwsh.hxx>
#include <viewdata.hxx>

#include <COKit/COKit.hxx>
#include <comphelper/kit.hxx>
#include <tools/json_writer.hxx>

namespace sc
{
void notifyCellRangeMarker(const ScViewData& rViewData, const CellRangeMarkerOptions& rOptions)
{
    if (!comphelper::COKit::isActive())
        return;

    ScTabViewShell* pViewShell = rViewData.GetViewShell();
    if (!pViewShell)
        return;

    tools::JsonWriter aWriter;
    aWriter.put("commandName", "CellRangeMarker");
    {
        const auto aStateNode = aWriter.startNode("state");
        aWriter.put("name", rOptions.aName);
        aWriter.put("part", static_cast<sal_Int32>(rOptions.nPart));
        if (rOptions.aColor != COL_AUTO)
            aWriter.put("color", rOptions.aColor.AsRGBHexString());
        if (rOptions.bDashed)
            aWriter.put("dashed", true);
        if (rOptions.fFillOpacity > 0.0)
            aWriter.put("fillOpacity", rOptions.fFillOpacity);
        if (!rOptions.aHandleCommand.isEmpty())
            aWriter.put("handleCommand", rOptions.aHandleCommand);
        const auto aRangesArray = aWriter.startArray("cellRanges");
        for (auto const& rCellRange : rOptions.aCellRanges)
        {
            const OUString aCells = OUString::number(rCellRange.aStart.Col()) + ", "
                                    + OUString::number(rCellRange.aStart.Row()) + ", "
                                    + OUString::number(rCellRange.aEnd.Col()) + ", "
                                    + OUString::number(rCellRange.aEnd.Row());
            aWriter.putSimpleValue(aCells);
        }
    }
    pViewShell->viewCallback(COKitCallbackType::STATE_CHANGED, aWriter.finishAndGetAsOString());
}

} // namespace sc

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
