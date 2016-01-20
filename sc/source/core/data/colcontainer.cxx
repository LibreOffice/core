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


#include "colcontainer.hxx"
#include "column.hxx"
#include "document.hxx"

ScColContainer::ScColContainer( ScDocument* pDoc ):
    aCols( ScColumnVector() ),
    pDocument( pDoc )
{}

ScColContainer::ScColContainer( ScDocument* pDoc, const size_t nSize )
{
    pDocument = pDoc;
    aCols.resize( nSize );
    for ( size_t nCol = 0; nCol < nSize; ++nCol )
        aCols[nCol] = new ScColumn;
}

ScColContainer::~ScColContainer()
{
    Clear();
}

void ScColContainer::CreateCol( SCCOL nColIdx, SCTAB nTab )
{
    assert( nColIdx >= 0 );
    SCCOL nSize = size();
    if ( nColIdx < nSize )
        return;
    else
    {
        aCols.resize( nColIdx + 1, nullptr );
        for ( SCCOL nNewColIdx = nSize; nNewColIdx <= nColIdx; ++nNewColIdx )
        {
            aCols[nNewColIdx] = new ScColumn;
            aCols[nNewColIdx]->Init( nNewColIdx, nTab, pDocument );
            // TODO: Apply any full row formatting / document formatting
        }
    }
}

void ScColContainer::DeleteLastCols( SCSIZE nCols )
{
    SCCOL nSize = size();
    SCCOL nFirstColToDelete = nSize - nCols;
    if ( !ColumnExists( nFirstColToDelete ) )
        return;

    for ( SCCOL nColToDelete = nFirstColToDelete; nColToDelete < nSize; ++nColToDelete )
    {
        if ( !pDocument->IsInDtorClear() )
            aCols[nColToDelete]->FreeNotes();
        aCols[nColToDelete]->PrepareBroadcastersForDestruction();
        delete aCols[nColToDelete];
        aCols.resize( static_cast<size_t>( nFirstColToDelete ) );
    }
}

bool ScColContainer::ColumnExists( SCCOL nColIdx ) const
{
    if ( nColIdx < 0 || nColIdx >= size() )
        return false;
    return true;
}

void ScColContainer::Clear()
{
    SCCOL nSize = size();
    for ( SCCOL nIdx = 0; nIdx < nSize; ++nIdx )
    {
        aCols[nIdx]->PrepareBroadcastersForDestruction();
        delete aCols[nIdx];
    }
    aCols.clear();
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
