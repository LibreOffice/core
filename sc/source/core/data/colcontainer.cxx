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
    SCCOL nLast = static_cast<SCCOL>( aCols.size() ) - 1;
    if ( nColIdx <= nLast )
    {
        if ( aCols[nColIdx] )
            return;
        else
            aCols[nColIdx] = new ScColumn;
    }
    else
    {
        aCols.resize( nColIdx + 1, nullptr );
        aCols[nColIdx] = new ScColumn;
    }
    aCols[nColIdx]->Init( nColIdx, nTab, pDocument );
    // TODO: Apply any full row formatting / document formatting
}

void ScColContainer::DeleteCol( SCCOL nColIdx )
{
    if ( !ColumnExists( nColIdx ) )
        return;

    if ( !pDocument->IsInDtorClear() )
        aCols[nColIdx]->FreeNotes();
    aCols[nColIdx]->PrepareBroadcastersForDestruction();
    delete aCols[nColIdx];
    aCols[nColIdx] = nullptr;

    SCCOL nLast = static_cast<SCCOL>( aCols.size() ) - 1;

    if ( nColIdx == nLast )
    {
        SCCOL nIdx;
        for ( nIdx = nLast - 1; nIdx >= 0; --nIdx )
            if ( aCols[static_cast<size_t>( nIdx )] )
                break;
        if ( nIdx >= 0 )
            aCols.resize( static_cast<size_t>( nIdx ) + 1 );
        else
            aCols.clear();
    }
}

bool ScColContainer::ColumnExists( SCCOL nColIdx ) const
{
    SCCOL nLast = static_cast<SCCOL>( aCols.size() ) - 1;
    if ( nColIdx < 0 || nColIdx > nLast )
        return false;
    return ( aCols[nColIdx] ? true : false );
}

void ScColContainer::Clear()
{
    SCCOL nLast = static_cast<SCCOL>( aCols.size() ) - 1;
    for ( SCCOL nIdx = 0; nIdx <= nLast; ++nIdx )
    {
        size_t nTmp = static_cast<size_t>( nIdx );
        if ( !aCols[nTmp] )
            continue;
        aCols[nTmp]->PrepareBroadcastersForDestruction();
        delete aCols[nTmp];
    }
    aCols.clear();
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
