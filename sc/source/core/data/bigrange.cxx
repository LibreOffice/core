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
            || nCol == nInt32Min || nCol == nInt32Max) &&
        ((0 <= nRow && nRow <= rDoc.MaxRow())
            || nRow == nInt32Min || nRow == nInt32Max) &&
        ((0 <= nTab && nTab < rDoc.GetTableCount())
            || nTab == nInt32Min || nTab == nInt32Max)
        ;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
