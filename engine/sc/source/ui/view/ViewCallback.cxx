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

#include <ViewCallback.hxx>

#include <CellRangeInventory.hxx>
#include <gridwin.hxx>
#include <viewdata.hxx>

#include <tools/json_writer.hxx>

namespace sc
{
void putCellCursorState(tools::JsonWriter& rWriter, const ScViewData& rViewData)
{
    const SCTAB nTab = rViewData.GetTabNumber();
    const ScGridWindow* pGridWindow = rViewData.GetActiveWin();
    const bool bVisible = pGridWindow && pGridWindow->isKitCellCursorVisible();

    rWriter.put("part", static_cast<sal_Int32>(nTab));
    {
        auto aCursor = rWriter.startNode("cursor");
        {
            auto aCell = rWriter.startArray("cell");
            rWriter.putSimpleValue(rViewData.GetCurX());
            rWriter.putSimpleValue(rViewData.GetCurY());
        }
        rWriter.put("visible", bVisible);
    }
    putMarkedRanges(rWriter, "selection", rViewData.GetMarkData(), nTab);
}

} // namespace sc

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
