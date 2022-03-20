/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <SparklineData.hxx>

namespace sc
{
RangeOrientation calculateOrientation(sal_Int32 nOutputSize, ScRange const& rInputRange)
{
    sal_Int32 nRowSize = rInputRange.aEnd.Row() - rInputRange.aStart.Row();
    sal_Int32 nColSize = rInputRange.aEnd.Col() - rInputRange.aStart.Col();

    auto eInputOrientation = RangeOrientation::Unknown;
    if (nOutputSize == nRowSize)
        eInputOrientation = RangeOrientation::Row;
    else if (nOutputSize == nColSize)
        eInputOrientation = RangeOrientation::Col;
    return eInputOrientation;
}

} // end sc

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
