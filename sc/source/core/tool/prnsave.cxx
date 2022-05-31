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

#include <prnsave.hxx>
#include <address.hxx>

#include <osl/diagnose.h>
#include <tools/json_writer.hxx>

//      Data per table

ScPrintSaverTab::ScPrintSaverTab() :
    mbEntireSheet(false)
{
}

ScPrintSaverTab::~ScPrintSaverTab()
{
}

void ScPrintSaverTab::SetAreas( ScRangeVec&& rRanges, bool bEntireSheet )
{
    maPrintRanges = std::move(rRanges);
    mbEntireSheet = bEntireSheet;
}

void ScPrintSaverTab::SetRepeat( std::optional<ScRange> oCol, std::optional<ScRange> oRow )
{
    moRepeatCol = std::move(oCol);
    moRepeatRow = std::move(oRow);
}

bool ScPrintSaverTab::operator==( const ScPrintSaverTab& rCmp ) const
{
    return
        (moRepeatCol == rCmp.moRepeatCol) &&
        (moRepeatRow == rCmp.moRepeatRow) &&
        (mbEntireSheet == rCmp.mbEntireSheet) &&
        (maPrintRanges == rCmp.maPrintRanges);
}

//      Data for the whole document

ScPrintRangeSaver::ScPrintRangeSaver( SCTAB nCount ) :
    nTabCount( nCount )
{
    if (nCount > 0)
        pData.reset( new ScPrintSaverTab[nCount] );
}

ScPrintRangeSaver::~ScPrintRangeSaver()
{
}

ScPrintSaverTab& ScPrintRangeSaver::GetTabData(SCTAB nTab)
{
    OSL_ENSURE(nTab<nTabCount,"ScPrintRangeSaver Tab too big");
    return pData[nTab];
}

const ScPrintSaverTab& ScPrintRangeSaver::GetTabData(SCTAB nTab) const
{
    OSL_ENSURE(nTab<nTabCount,"ScPrintRangeSaver Tab too big");
    return pData[nTab];
}

void ScPrintRangeSaver::GetPrintRangesInfo(tools::JsonWriter& rPrintRanges) const
{
    // Array for sheets in the document.
    auto printRanges = rPrintRanges.startArray("printranges");
    for (SCTAB nTab = 0; nTab < nTabCount; nTab++)
    {
        auto sheetNode = rPrintRanges.startStruct();
        const ScPrintSaverTab& rPsTab = pData[nTab];
        const std::vector<ScRange>& rRangeVec = rPsTab.GetPrintRanges();

        rPrintRanges.put("sheet", static_cast<sal_Int32>(nTab));

        // Array for ranges within each sheet.
        auto sheetRanges = rPrintRanges.startArray("ranges");
        OStringBuffer aRanges;
        sal_Int32 nLast = rRangeVec.size() - 1;
        for (sal_Int32 nIdx = 0; nIdx <= nLast; ++nIdx)
        {
            const ScRange& rRange = rRangeVec[nIdx];
            aRanges.append("[ " +
                OString::number(rRange.aStart.Col()) + ", " +
                OString::number(rRange.aStart.Row()) + ", " +
                OString::number(rRange.aEnd.Col()) + ", " +
                OString::number(rRange.aEnd.Row()) +
                (nLast == nIdx ? std::string_view("]") : std::string_view("], ")));
        }

        rPrintRanges.putRaw(aRanges.getStr());
    }
}

bool ScPrintRangeSaver::operator==( const ScPrintRangeSaver& rCmp ) const
{
    bool bEqual = ( nTabCount == rCmp.nTabCount );
    if (bEqual)
        for (SCTAB i=0; i<nTabCount; i++)
            if (!(pData[i]==rCmp.pData[i]))
            {
                bEqual = false;
                break;
            }
    return bEqual;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
