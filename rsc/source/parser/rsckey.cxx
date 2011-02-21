/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_rsc.hxx"
/****************** I N C L U D E S **************************************/
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <rscall.h>
#include <rsctools.hxx>
#include <rschash.hxx>
#include <rsckey.hxx>

#if defined(_MSC_VER) && (_MSC_VER >= 1200 )
#define _cdecl __cdecl
#endif

/****************** C o d e **********************************************/
/****************** keyword sort function ********************************/
extern "C" {
#if defined( WNT ) && !defined (ICC)
    int _cdecl KeyCompare( const void * pFirst, const void * pSecond );
#else
    int KeyCompare( const void * pFirst, const void * pSecond );
#endif
}

#if defined( WNT ) && !defined(ICC)
int _cdecl KeyCompare( const void * pFirst, const void * pSecond ){
#else
int KeyCompare( const void * pFirst, const void * pSecond ){
#endif
    if( ((KEY_STRUCT *)pFirst)->nName > ((KEY_STRUCT *)pSecond)->nName )
        return( 1 );
    else if( ((KEY_STRUCT *)pFirst)->nName < ((KEY_STRUCT *)pSecond)->nName )
        return( -1 );
    else
        return( 0 );
}

/*************************************************************************
|*
|*    RscNameTable::RscNameTable()
|*
*************************************************************************/
RscNameTable::RscNameTable() {
    bSort    = TRUE;
    nEntries = 0;
    pTable   = NULL;
};

/*************************************************************************
|*
|*    RscNameTable::~RscNameTable()
|*
*************************************************************************/
RscNameTable::~RscNameTable() {
    if( pTable )
        rtl_freeMemory( pTable );
};


/*************************************************************************
|*
|*    RscNameTable::SetSort()
|*
*************************************************************************/
void RscNameTable::SetSort( BOOL bSorted ){
    bSort = bSorted;
    if( bSort && pTable){
        // Schluesselwort Feld sortieren
        qsort( (void *)pTable, nEntries,
               sizeof( KEY_STRUCT ), KeyCompare );
    };
};

/*************************************************************************
|*
|*    RscNameTable::Put()
|*
*************************************************************************/
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

Atom RscNameTable::Put( Atom nName, sal_uInt32 nTyp )
{
    return( Put( nName, nTyp, (long)nName ) );
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

Atom RscNameTable::Put( const char * pName, sal_uInt32 nTyp, RscTop * pClass )
{
    return( Put( pHS->getID( pName ), nTyp, (long)pClass ) );
};

/*************************************************************************
|*
|*    RscNameTable::Get()
|*
*************************************************************************/
BOOL RscNameTable::Get( Atom nName, KEY_STRUCT * pEle ){
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
        return( TRUE );
    };
    return( FALSE );
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
