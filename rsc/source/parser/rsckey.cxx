/*************************************************************************
 *
 *  $RCSfile: rsckey.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:42:55 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
/************************************************************************

    Source Code Control System - Header

    $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/rsc/source/parser/rsckey.cxx,v 1.1.1.1 2000-09-18 16:42:55 hr Exp $

    Source Code Control System - Update

    $Log: not supported by cvs2svn $
    Revision 1.13  2000/09/17 12:51:11  willem.vandorp
    OpenOffice header added.

    Revision 1.12  2000/07/26 17:13:21  willem.vandorp
    Headers/footers replaced

    Revision 1.11  1999/10/25 11:48:18  mh
    changes for VC6

    Revision 1.10  1997/08/27 18:17:12  MM
    neue Headerstruktur


      Rev 1.9   27 Aug 1997 18:17:12   MM
   neue Headerstruktur

      Rev 1.8   03 Jan 1997 17:07:16   MH
   Calling Convention

      Rev 1.7   12 Jan 1996 17:23:40   TRI
   WNTWTC Anpassung

      Rev 1.6   21 Nov 1995 19:49:48   TLX
   Neuer Link

      Rev 1.5   27 Jul 1995 16:57:36   MM
   LongEnumRange

      Rev 1.4   16 Feb 1995 19:51:06   MM
   char * -> const char *

**************************************************************************/
/****************** I N C L U D E S **************************************/
#pragma hdrstop

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
        RscMem::Free( pTable );
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
HASHID RscNameTable::Put( HASHID nName, USHORT nTyp, long nValue ){
    if( pTable )
        pTable = (KEY_STRUCT *)
                 RscMem::Realloc( (void *)pTable,
                 (USHORT)((nEntries +1) * sizeof( KEY_STRUCT )) );
    else
        pTable = (KEY_STRUCT *)
                 RscMem::Malloc( (USHORT)((nEntries +1)
                                 * sizeof( KEY_STRUCT )) );
    pTable[ nEntries ].nName  = nName;
    pTable[ nEntries ].nTyp   = nTyp;
    pTable[ nEntries ].yylval = nValue;
    nEntries++;
    if( bSort )
        SetSort();
    return( nName );
};

HASHID RscNameTable::Put( const char * pName, USHORT nTyp, long nValue )
{
    return( Put( pHS->Insert( pName ), nTyp, nValue ) );
};

HASHID RscNameTable::Put( HASHID nName, USHORT nTyp )
{
    return( Put( nName, nTyp, (long)nName ) );
};

HASHID RscNameTable::Put( const char * pName, USHORT nTyp )
{
    HASHID  nId;

    nId = pHS->Insert( pName );
    return( Put( nId, nTyp, (long)nId ) );
};

HASHID RscNameTable::Put( HASHID nName, USHORT nTyp, RscTop * pClass )
{
    return( Put( nName, nTyp, (long)pClass ) );
};

HASHID RscNameTable::Put( const char * pName, USHORT nTyp, RscTop * pClass )
{
    return( Put( pHS->Insert( pName ), nTyp, (long)pClass ) );
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
BOOL RscNameTable::Get( HASHID nName, KEY_STRUCT * pEle ){
    KEY_STRUCT * pKey = NULL;
    KEY_STRUCT  aSearchName;
    USHORT      i;

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

