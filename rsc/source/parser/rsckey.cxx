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
    if( ((KEY_STRUCT *)pFirst)->nName > ((KEY_STRUCT *)pSecond)->nName )
        return( 1 );
    else if( ((KEY_STRUCT *)pFirst)->nName < ((KEY_STRUCT *)pSecond)->nName )
        return( -1 );
    else
        return( 0 );
}

RscNameTable::RscNameTable() {
    bSort    = sal_True;
    nEntries = 0;
    pTable   = NULL;
};

RscNameTable::~RscNameTable() {
    if( pTable )
        rtl_freeMemory( pTable );
};


void RscNameTable::SetSort( sal_Bool bSorted ){
    bSort = bSorted;
    if( bSort && pTable){
        // Schluesselwort Feld sortieren
        qsort( (void *)pTable, nEntries,
               sizeof( KEY_STRUCT ), KeyCompare );
    };
};

Atom RscNameTable::Put( Atom nName, sal_uInt32 nTyp, long nValue ){
    if( pTable )
        pTable = (KEY_STRUCT *)
                 rtl_reallocateMemory( (void *)pTable,
                 ((nEntries +1) * sizeof( KEY_STRUCT )) );
    else
        pTable = (KEY_STRUCT *)
                 rtl_allocateMemory( ((nEntries +1)
                                 * sizeof( KEY_STRUCT )) );
    pTable[ nEntries ].nName  = nName;
    pTable[ nEntries ].nTyp   = nTyp;
    pTable[ nEntries ].yylval = nValue;
    nEntries++;
    if( bSort )
        SetSort();
    return( nName );
};

Atom RscNameTable::Put( const char * pName, sal_uInt32 nTyp, long nValue )
{
    return( Put( pHS->getID( pName ), nTyp, nValue ) );
};

Atom RscNameTable::Put( const char * pName, sal_uInt32 nTyp )
{
    Atom  nId;

    nId = pHS->getID( pName );
    return( Put( nId, nTyp, (long)nId ) );
};

Atom RscNameTable::Put( Atom nName, sal_uInt32 nTyp, RscTop * pClass )
{
    return( Put( nName, nTyp, (long)pClass ) );
};

sal_Bool RscNameTable::Get( Atom nName, KEY_STRUCT * pEle ){
    KEY_STRUCT * pKey = NULL;
    KEY_STRUCT  aSearchName;
    sal_uInt32  i;

    if( bSort ){
        // Suche nach dem Schluesselwort
        aSearchName.nName = nName;
        pKey = (KEY_STRUCT *)bsearch(
#ifdef UNX
                   (const char *) &aSearchName, (char *)pTable,
#else
                   (const void *) &aSearchName, (const void *)pTable,
#endif
                   nEntries, sizeof( KEY_STRUCT ), KeyCompare );
    }
    else{
        i = 0;
        while( i < nEntries && !pKey ){
            if( pTable[ i ].nName == nName )
                pKey = &pTable[ i ];
            i++;
        };
    };

    if( pKey ){ // Schluesselwort gefunden
        *pEle = *pKey;
        return( sal_True );
    };
    return( sal_False );
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
