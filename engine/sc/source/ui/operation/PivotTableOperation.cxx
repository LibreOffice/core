/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <operation/PivotTableOperation.hxx>

#include <docsh.hxx>
#include <dpobject.hxx>
#include <dpshttab.hxx>

namespace sc
{
PivotTableOperation::PivotTableOperation(OperationType eType, ScDocShell& rDocShell, bool bRecord,
                                         bool bApi)
    : Operation(eType, bRecord, bApi)
    , mrDocShell(rDocShell)
{
}

void PivotTableOperation::convertDPObjectRanges(ScDPObject& rDPObject)
{
    rDPObject.SetOutRange(convertRange(rDPObject.GetOutRange()));

    const ScSheetSourceDesc* pSheetDesc = rDPObject.GetSheetDesc();
    if (pSheetDesc && !pSheetDesc->HasRangeName())
    {
        ScSheetSourceDesc aDesc(*pSheetDesc);
        aDesc.SetSourceRange(convertRange(aDesc.GetSourceRange()));
        rDPObject.SetSheetDesc(aDesc);
    }
}

ScDPObject* PivotTableOperation::findDefaultViewDPObject(ScDPObject const& rSheetViewObject)
{
    SCTAB nSheetViewTab = rSheetViewObject.GetOutRange().aStart.Tab();
    SCTAB nDefaultTab = convertTab(nSheetViewTab);
    if (nSheetViewTab == nDefaultTab)
        return const_cast<ScDPObject*>(&rSheetViewObject);
    ScDPCollection& rDPs = *mrDocShell.GetDocument().GetDPCollection();
    for (size_t i = 0; i < rDPs.GetCount(); ++i)
    {
        auto& rDPObject = rDPs[i];
        if (rDPObject.GetName() == rSheetViewObject.GetName()
            && rDPObject.GetOutRange().aStart.Tab() == nDefaultTab)
        {
            return &rDPObject;
        }
    }
    return nullptr;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
