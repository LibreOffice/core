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

#include <clipparam.hxx>


ScClipParam::ScClipParam() :
    meDirection(Unspecified),
    mbCutMode(false),
    mnSourceDocID(0)
{
}

ScClipParam::ScClipParam(const ScRange& rRange, bool bCutMode) :
    meDirection(Unspecified),
    mbCutMode(bCutMode),
    mnSourceDocID(0)
{
    maRanges.push_back(rRange);
}

bool ScClipParam::isMultiRange() const
{
    return maRanges.size() > 1;
}

SCCOL ScClipParam::getPasteColSize()
{
    if (maRanges.empty())
        return 0;

    switch (meDirection)
    {
        case ScClipParam::Column:
        {
            SCCOL nColSize = 0;
            for ( size_t i = 0, nListSize = maRanges.size(); i < nListSize; ++i )
            {
                const ScRange& rRange = maRanges[ i ];
                nColSize += rRange.aEnd.Col() - rRange.aStart.Col() + 1;
            }
            return nColSize;
        }
        case ScClipParam::Row:
        {
            // We assume that all ranges have identical column size.
            const ScRange& rRange = maRanges.front();
            return rRange.aEnd.Col() - rRange.aStart.Col() + 1;
        }
        case ScClipParam::Unspecified:
        default:
            ;
    }
    return 0;
}

SCROW ScClipParam::getPasteRowSize()
{
    if (maRanges.empty())
        return 0;

    switch (meDirection)
    {
        case ScClipParam::Column:
        {
            // We assume that all ranges have identical row size.
            const ScRange& rRange = maRanges.front();
            return rRange.aEnd.Row() - rRange.aStart.Row() + 1;
        }
        case ScClipParam::Row:
        {
            SCROW nRowSize = 0;
            for ( size_t i = 0, nListSize = maRanges.size(); i < nListSize; ++i )
            {
                const ScRange& rRange = maRanges[ i ];
                nRowSize += rRange.aEnd.Row() - rRange.aStart.Row() + 1;
            }
            return nRowSize;
        }
        case ScClipParam::Unspecified:
        default:
            ;
    }
    return 0;
}

ScRange ScClipParam::getWholeRange() const
{
    return maRanges.Combine();
}

void ScClipParam::transpose()
{
    switch (meDirection)
    {
        case Column:
            meDirection = ScClipParam::Row;
        break;
        case Row:
            meDirection = ScClipParam::Column;
        break;
        case Unspecified:
        default:
            ;
    }

    ScRangeList aNewRanges;
    if (!maRanges.empty())
    {
        const ScRange & rRange1 = maRanges.front();
        SCCOL nColOrigin = rRange1.aStart.Col();
        SCROW nRowOrigin = rRange1.aStart.Row();

        for ( size_t i = 0, n = maRanges.size(); i < n; ++i )
        {
            const ScRange & rRange = maRanges[ i ];
            SCCOL nColDelta = rRange.aStart.Col() - nColOrigin;
            SCROW nRowDelta = rRange.aStart.Row() - nRowOrigin;
            SCCOL nCol1 = 0;
            SCCOL nCol2 = static_cast<SCCOL>(rRange.aEnd.Row() - rRange.aStart.Row());
            SCROW nRow1 = 0;
            SCROW nRow2 = static_cast<SCROW>(rRange.aEnd.Col() - rRange.aStart.Col());
            nCol1 += static_cast<SCCOL>(nRowDelta);
            nCol2 += static_cast<SCCOL>(nRowDelta);
            nRow1 += static_cast<SCROW>(nColDelta);
            nRow2 += static_cast<SCROW>(nColDelta);
            aNewRanges.push_back( ScRange(nCol1, nRow1, rRange.aStart.Tab(), nCol2, nRow2, rRange.aStart.Tab() ) );
        }
    }
    maRanges = aNewRanges;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
