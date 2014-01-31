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

#include "rfindlst.hxx"

// STATIC DATA -----------------------------------------------------------

#define SC_RANGECOLORS  8

static const ColorData aColNames[SC_RANGECOLORS] =
    { COL_LIGHTBLUE, COL_LIGHTRED, COL_LIGHTMAGENTA, COL_GREEN,
        COL_BLUE, COL_RED, COL_MAGENTA, COL_BROWN };

//==================================================================

ScRangeFindList::ScRangeFindList(const OUString& rName) :
    aDocName( rName ),
    bHidden( false )
{
}

ColorData ScRangeFindList::Insert( const ScRangeFindData &rNew )
{
    for(std::vector<ScRangeFindData>::iterator it=maEntries.begin(); it!=maEntries.end(); ++it)
    {
        if(it->aRef == rNew.aRef)
        {
            return it->nColorData;
        }
    }
    ScRangeFindData insertData(rNew);
    insertData.nColorData = aColNames[maEntries.size() % SC_RANGECOLORS];
    maEntries.push_back(insertData);
    return insertData.nColorData;
}

ColorData ScRangeFindList::GetColorName( const size_t nIndex )
{
    return aColNames[nIndex % SC_RANGECOLORS];
}

//==================================================================

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
