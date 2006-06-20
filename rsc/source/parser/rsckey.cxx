/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: rsckey.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 05:46:38 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
/****************** I N C L U D E S **************************************/
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>

#ifndef _RSCALL_H
#include <rscall.h>
#endif
#ifndef _RSCTOOLS_HXX
#include <rsctools.hxx>
#endif
#ifndef _RSCHASH_HXX
#include <rschash.hxx>
#endif
#ifndef _RSCKEY_HXX
#include <rsckey.hxx>
#endif

#if defined(_MSC_VER) && (_MSC_VER >= 1200 )
#define _cdecl __cdecl
#endif

/****************** C o d e **********************************************/
/****************** keyword sort function ********************************/
extern "C" {
#if defined( ZTC ) && defined( PM2 )
    int __CLIB KeyCompare( const void * pFirst, const void * pSecond );
#else
#if defined( WNT ) && !defined( WTC ) && !defined (ICC)
    int _cdecl KeyCompare( const void * pFirst, const void * pSecond );
#else
    int KeyCompare( const void * pFirst, const void * pSecond );
#endif
#endif
}

#if defined( WNT ) && !defined( WTC ) && !defined(ICC)
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
|*    Beschreibung      RES.DOC
|*    Ersterstellung    MM 28.02.91
|*    Letzte Aenderung  MM 28.02.91
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
|*    Beschreibung
|*    Ersterstellung    MM 15.05.91
|*    Letzte Aenderung  MM 15.05.91
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
|*    Beschreibung      RES.DOC
|*    Ersterstellung    MM 28.02.91
|*    Letzte Aenderung  MM 28.02.91
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
|*    Beschreibung      RES.DOC
|*    Ersterstellung    MM 28.02.91
|*    Letzte Aenderung  MM 28.02.91
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
|*    Beschreibung      RES.DOC
|*    Ersterstellung    MM 28.02.91
|*    Letzte Aenderung  MM 28.02.91
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

