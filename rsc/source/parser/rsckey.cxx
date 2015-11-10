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

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <rscall.h>
#include <rsctools.hxx>
#include <rschash.hxx>
#include <rsckey.hxx>

#ifdef _MSC_VER
#define _cdecl __cdecl
#endif

extern "C" {
    int SAL_CALL KeyCompare( const void * pFirst, const void * pSecond );
}

int SAL_CALL KeyCompare( const void * pFirst, const void * pSecond )
{
    if( static_cast<KEY_STRUCT const *>(pFirst)->nName > static_cast<KEY_STRUCT const *>(pSecond)->nName )
        return 1;
    else if( static_cast<KEY_STRUCT const *>(pFirst)->nName < static_cast<KEY_STRUCT const *>(pSecond)->nName )
        return -1;
    else
        return 0;
}

RscNameTable::RscNameTable()
{
    bSort    = true;
    nEntries = 0;
    pTable   = nullptr;
};

RscNameTable::~RscNameTable()
{
    if( pTable )
        rtl_freeMemory( pTable );
};


void RscNameTable::SetSort( bool bSorted )
{
    bSort = bSorted;
    if( bSort && pTable)
    {
        // Schluesselwort Feld sortieren
        qsort( static_cast<void *>(pTable), nEntries,
               sizeof( KEY_STRUCT ), KeyCompare );
    }
};

Atom RscNameTable::Put( Atom nName, sal_uInt32 nTyp, sal_IntPtr nValue )
{
    if( pTable )
        pTable = static_cast<KEY_STRUCT *>(
                 rtl_reallocateMemory( static_cast<void *>(pTable),
                 ((nEntries +1) * sizeof( KEY_STRUCT )) ));
    else
        pTable = static_cast<KEY_STRUCT *>(
                 rtl_allocateMemory( ((nEntries +1)
                                 * sizeof( KEY_STRUCT )) ));

    pTable[ nEntries ].nName  = nName;
    pTable[ nEntries ].nTyp   = nTyp;
    pTable[ nEntries ].yylval = nValue;
    nEntries++;
    if( bSort )
        SetSort();

    return nName;
};

Atom RscNameTable::Put( const char * pName, sal_uInt32 nTyp, sal_IntPtr nValue )
{
    return Put( pHS->getID( pName ), nTyp, nValue );
};

Atom RscNameTable::Put( const char * pName, sal_uInt32 nTyp )
{
    Atom  nId;

    nId = pHS->getID( pName );
    return Put( nId, nTyp, (sal_IntPtr)nId );
};

Atom RscNameTable::Put( Atom nName, sal_uInt32 nTyp, RscTop * pClass )
{
    return Put( nName, nTyp, reinterpret_cast<sal_IntPtr>(pClass) );
};

bool RscNameTable::Get( Atom nName, KEY_STRUCT * pEle )
{
    KEY_STRUCT * pKey = nullptr;
    KEY_STRUCT  aSearchName;
    sal_uInt32  i;

    if( bSort )
    {
        // Suche nach dem Schluesselwort
        aSearchName.nName = nName;
        pKey = static_cast<KEY_STRUCT *>(bsearch(
                &aSearchName, pTable,
                nEntries, sizeof( KEY_STRUCT ), KeyCompare ));
    }
    else
    {
        i = 0;
        while( i < nEntries && !pKey )
        {
            if( pTable[ i ].nName == nName )
                pKey = &pTable[ i ];
            i++;
        }
    }

    if( pKey )
    {
        // Schluesselwort gefunden
        *pEle = *pKey;
        return true;
    }
    return false;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
