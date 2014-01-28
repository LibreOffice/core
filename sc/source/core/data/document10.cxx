/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <document.hxx>
#include <clipcontext.hxx>
#include <formulacell.hxx>
#include <clipparam.hxx>
#include <table.hxx>
#include <tokenarray.hxx>
#include <editutil.hxx>

// Add totally brand-new methods to this source file.

void ScDocument::BroadcastCells( const ScRange& rRange, sal_uLong nHint )
{
    ScHint aHint(nHint, ScAddress());
    ScAddress& rPos = aHint.GetAddress();

    for (SCTAB nTab = rRange.aStart.Tab(); nTab <= rRange.aEnd.Tab(); ++nTab)
    {
        rPos.SetTab(nTab);
        for (SCCOL nCol = rRange.aStart.Col(); nCol <= rRange.aEnd.Col(); ++nCol)
        {
            rPos.SetCol(nCol);
            for (SCROW nRow = rRange.aStart.Row(); nRow <= rRange.aEnd.Row(); ++nRow)
            {
                rPos.SetRow(nRow);
                Broadcast(aHint);
            }
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
