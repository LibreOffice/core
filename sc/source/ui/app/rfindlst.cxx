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

#include <rfindlst.hxx>
#include <tools/debug.hxx>

#define SC_RANGECOLORS  8

static const Color aColNames[SC_RANGECOLORS] =
    { COL_LIGHTBLUE, COL_LIGHTRED, COL_LIGHTMAGENTA, COL_GREEN,
        COL_BLUE, COL_RED, COL_MAGENTA, COL_BROWN };

ScRangeFindList::ScRangeFindList(const OUString& rName) :
    aDocName( rName ),
    bHidden( false ),
    nIndexColor( 0 )
{
}

Color ScRangeFindList::Insert( const ScRangeFindData &rNew )
{
    auto it = std::find_if(maEntries.begin(), maEntries.end(),
        [&rNew](const ScRangeFindData& rEntry) { return rEntry.aRef == rNew.aRef; });
    ScRangeFindData insertData(rNew);
    insertData.nColor = ( it != maEntries.end() ? it->nColor :
                              ScRangeFindList::GetColorName( maEntries.size() ) );
    maEntries.push_back(insertData);
    nIndexColor = maEntries.size() - 1;
    return insertData.nColor;
}

Color ScRangeFindList::GetColorName( const size_t nIndex )
{
    return aColNames[nIndex % SC_RANGECOLORS];
}

Color ScRangeFindList::FindColor( const ScRange& rRef, const size_t nIndex )
{
    sal_Int32 nOldCntr = 0;
    sal_Int32 nNewCntr = 0;
    Color nOldColor = Color(0);
    Color nNewColor = Color(0);

    DBG_ASSERT( (nIndex < maEntries.size()), "nIndex out of range!" );

    nOldColor = maEntries[nIndex].nColor;
    nNewColor = ScRangeFindList::GetColorName( nIndex );

    std::vector<ScRangeFindData>::iterator it=maEntries.begin();
    for( ;it!=maEntries.end(); ++it)
    {
        if(it->aRef == rRef)
            break;

        if (it->nColor == nOldColor )
            nOldCntr++;

        if (it->nColor == nNewColor )
            nNewCntr++;
    }

    if ( it != maEntries.end() )
        return it->nColor;

    if ( nOldCntr == 1 )
        return nOldColor;

    if ( nNewCntr > 0 )
        return ScRangeFindList::GetColorName( ++nIndexColor );

    return nNewColor;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
