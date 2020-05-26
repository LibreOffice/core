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


#include <colcontainer.hxx>
#include <column.hxx>

ScColContainer::ScColContainer( ScSheetLimits const & rSheetLimits, const size_t nSize )
{
    aCols.resize( nSize );
    for ( size_t nCol = 0; nCol < nSize; ++nCol )
        aCols[nCol].reset( new ScColumn(rSheetLimits) );
}

ScColContainer::~ScColContainer() COVERITY_NOEXCEPT_FALSE
{
    Clear();
}

void ScColContainer::Clear()
{
    SCCOL nSize = size();
    for ( SCCOL nIdx = 0; nIdx < nSize; ++nIdx )
    {
        aCols[nIdx]->PrepareBroadcastersForDestruction();
        aCols[nIdx].reset();
    }
    aCols.clear();
}

void ScColContainer::resize( ScSheetLimits const & rSheetLimits, const size_t aNewColSize )
{
    size_t aOldColSize = aCols.size();
    aCols.resize( aNewColSize );
    for ( size_t nCol = aOldColSize; nCol < aNewColSize; ++nCol )
        aCols[nCol].reset(new ScColumn(rSheetLimits));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
