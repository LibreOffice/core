/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <bigrange.hxx>
#include <document.hxx>

bool ScBigAddress::IsValid( const ScDocument& rDoc ) const
{   // min/max interval bounds define whole col/row/tab
    return
        ((0 <= nCol && nCol <= rDoc.MaxCol())
            || nCol == ScBigRange::nRangeMin || nCol == ScBigRange::nRangeMax) &&
        ((0 <= nRow && nRow <= rDoc.MaxRow())
            || nRow == ScBigRange::nRangeMin || nRow == ScBigRange::nRangeMax) &&
        ((0 <= nTab && nTab < rDoc.GetTableCount())
            || nTab == ScBigRange::nRangeMin || nTab == ScBigRange::nRangeMax)
        ;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
