/*************************************************************************
 *
 *  $RCSfile: rschash.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:42:56 $
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

    $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/rsc/source/tools/rschash.cxx,v 1.1.1.1 2000-09-18 16:42:56 hr Exp $

    Source Code Control System - Update

    $Log: not supported by cvs2svn $
    Revision 1.7  2000/09/17 12:51:12  willem.vandorp
    OpenOffice header added.

    Revision 1.6  2000/07/26 17:13:24  willem.vandorp
    Headers/footers replaced

    Revision 1.5  1997/08/27 18:17:52  MM
    neue Headerstruktur


      Rev 1.4   27 Aug 1997 18:17:52   MM
   neue Headerstruktur

      Rev 1.3   21 Nov 1995 19:50:48   TLX
   Neuer Link

      Rev 1.2   25 Aug 1993 15:31:02   mm
   Fehler und Warnings beseitigt

      Rev 1.1   03 Jun 1993 15:45:48   mm
   Fehler in Test_HashId verbessert

      Rev 1.0   10 Aug 1992 07:18:56   MM
   Initial revision.

      Rev 1.6   23 Mar 1992 15:25:54   MM

      Rev 1.5   26 Nov 1991 08:42:22   MM
   Methode Test mit char* und void* Argument

      Rev 1.4   11 Sep 1991 12:40:52   MM

      Rev 1.3   31 Jul 1991 15:37:10   MM

      Rev 1.2   24 Jul 1991 10:10:52   MM
   Ausgabe auskommentiert

      Rev 1.1   18 Jul 1991 12:55:24   MM
   Fehler mit (BOOL) beseitigt

      Rev 1.0   14 Jun 1991 16:20:56   MM
   Initial revision.

      Rev 1.2   22 Mar 1991 11:41:44   MM

      Rev 1.1   25 Feb 1991 13:47:12   MM

      Rev 1.0   08 Feb 1991 13:12:06   MM
   Initial revision.

**************************************************************************/
/****************** I N C L U D E S **************************************/
// C and C++ Includes.
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

// Programmabh„ngige Includes.
#ifndef _RSCHASH_HXX
#include <rschash.hxx>
#endif

/****************** C O D E **********************************************/
/*************************************************************************
|*
|*    HashTabel::HashTabel()
|*
|*    Beschreibung      HASHTAB.DOC
|*    Ersterstellung    MM 20.03.90
|*    Letzte Aenderung  MM 27.06.90
|*
*************************************************************************/
HashTabel :: HashTabel( USHORT nMaxEntries ){
    nMax = nMaxEntries;     // set max entries
    nFill = 0;              // no entries
    lTry = 0;
    lAsk = 0;
}

/*************************************************************************
|*
|*    HashTabel::~HashTabel()
|*
|*    Beschreibung
|*    Ersterstellung    MM 17.07.91
|*    Letzte Aenderung  MM 17.07.91
|*
*************************************************************************/
HashTabel :: ~HashTabel(){
#ifdef DOS
/*
    printf( "Maximum: %d, Fuellung: %d\n", nMax, nFill );
    printf( "Anfragen: %ld, Versuche: %ld", lAsk, lTry );
    if( lTry != 0 )
        printf( ", V/E = %ld\n", lTry / lAsk );
*/
#endif
}

/*************************************************************************
|*
|*    HashTabel::Test_Insert()
|*
|*    Beschreibung      HASHTAB.DOC
|*    Ersterstellung    MM 20.03.90
|*    Letzte Aenderung  MM 27.06.90
|*
*************************************************************************/
HASHID HashTabel::Test_Insert( const void * pElement, BOOL bInsert )
{
    USHORT    nHash;
    USHORT    nIndex;
    USHORT    nLoop;

    lAsk++;
    lTry++;

    nHash =  HashFunc( pElement );
    nIndex = nHash % nMax;

    nLoop = 0;                                      // divide to range
    while( (nMax != nLoop) && IsEntry( nIndex ) )
    {                     // is place occupied
        if( EQUAL == Compare( pElement, nIndex ) )  // is element in tabel
            return( nIndex );                       // place of Element
        nLoop++;
        lTry++;
        nIndex = (USHORT)(nIndex + nHash + 7) % nMax;
    }

    if( bInsert )
    {
        if( nMax == nLoop )                         // is tabel full
            RscExit( 11 );
        nFill++;
        return( nIndex );                           // return free place
    }
    return( HASH_NONAME );
}

/*************************************************************************
|*
|*    HashTabel::Test()
|*
|*    Beschreibung      HASHTAB.DOC
|*    Ersterstellung    MM 15.05.91
|*    Letzte Aenderung  MM 15.05.91
|*
*************************************************************************/
HASHID HashTabel::Test( const void * pElement ){
    return( Test_Insert( pElement, FALSE ) );
}

/*************************************************************************
|*
|*    HashTabel::Insert()
|*
|*    Beschreibung      HASHTAB.DOC
|*    Ersterstellung    MM 20.03.90
|*    Letzte Aenderung  MM 27.06.90
|*
*************************************************************************/
HASHID HashTabel::Insert( const void * pElement ){
// return free place in Tabel or the place, if Element is in the tabel
    return( Test_Insert( pElement, TRUE ) );
}

/*************************************************************************
|*
|*    HashString::HashString()
|*
|*    Beschreibung      HASHTAB.DOC
|*    Ersterstellung    MM 20.03.90
|*    Letzte Aenderung  MM 27.06.90
|*
*************************************************************************/
HashString::HashString( USHORT nMaxEntries ) : HashTabel( nMaxEntries ){
    if( (long)nMaxEntries * sizeof( char * ) >= 0x10000 ){
        // can't allocate more then 64k - 1 Bytes
        RscExit( 12 );
    }
    // allocate

    ppStr = (char **)RscMem::Malloc( nMaxEntries * sizeof( char * ) );

    memset( ppStr, 0, nMaxEntries * sizeof( char * ) );
    paSC = new StringCon( (USHORT)40000 );
}

/*************************************************************************
|*
|*    ~HashString::HashString()
|*
|*    Beschreibung      HASHTAB.DOC
|*    Ersterstellung    MM 02.06.91
|*    Letzte Aenderung  MM 02.06.91
|*
*************************************************************************/
HashString::~HashString(){
    delete paSC;
    RscMem::Free( (void *)ppStr );
}

/*************************************************************************
|*
|*    HashString::HashFunc()
|*
|*    Beschreibung      HASHTAB.DOC
|*    Ersterstellung    MM 20.03.90
|*    Letzte Aenderung  MM 27.06.90
|*
*************************************************************************/
USHORT HashString :: HashFunc( const void * pElement ){
    const char    *pStr;
    USHORT        nHash = 0;  // hash value

    pStr = (const char *)pElement;
    while( *pStr ){
        nHash ^= toupper( *pStr ) - 'A';
        if( *++pStr ){
            nHash ^= (toupper( *pStr ) - 'A') << 4;
            if( *++pStr )
                nHash ^= (toupper( *pStr ) - 'A') << 8;
        }
    }
    return( nHash );
}

/*************************************************************************
|*
|*    HashString::IsEntry()
|*
|*    Beschreibung      HASHTAB.DOC
|*    Ersterstellung    MM 20.03.90
|*    Letzte Aenderung  MM 27.06.90
|*
*************************************************************************/
BOOL HashString :: IsEntry( HASHID nIndex ){
// return TRUE if place is occupied
// return FALSE if place is FREE
    // ppStr[ nIndex ] == pointer to stringtabel
    return( NULL != ppStr[ nIndex ] );
}

/*************************************************************************
|*
|*    HashString::Insert()
|*
|*    Beschreibung      HASHTAB.DOC
|*    Ersterstellung    MM 20.03.90
|*    Letzte Aenderung  MM 27.06.90
|*
*************************************************************************/
HASHID HashString :: Insert( const char * pElement ){
    HASHID  nIndex;

    nIndex = HashTabel::Insert( (const void *)pElement );
    if( !IsEntry( nIndex ) )// is place not occupied ?
        // put string in the string tabel
        ppStr[ nIndex ] =  paSC->Put( pElement );
    return( nIndex );
}

/*************************************************************************
|*
|*    HashString::Test()
|*
|*    Beschreibung
|*    Ersterstellung    MM 05.11.91
|*    Letzte Aenderung  MM 05.11.91
|*
*************************************************************************/
HASHID HashString :: Test( const char * pElement ){
    return HashTabel::Test( (const void *)pElement );
}

/*************************************************************************
|*
|*    HashString::Get()
|*
|*    Beschreibung      HASHTAB.DOC
|*    Ersterstellung    MM 20.03.90
|*    Letzte Aenderung  MM 27.06.90
|*
*************************************************************************/
char * HashString :: Get( HASHID nIndex ){
// return pointer to string
    if( nIndex != HASH_NONAME )
        return( ppStr[ nIndex ] );
    else
        return( NULL );
}

/*************************************************************************
|*
|*    HashString::Get()
|*
|*    Beschreibung      HASHTAB.DOC
|*    Ersterstellung    MM 20.03.90
|*    Letzte Aenderung  MM 27.06.90
|*
*************************************************************************/
COMPARE HashString :: Compare( const void * pElement, HASHID nIndex ){
    short   nCmp;   // Vergleichsresultat

    nCmp = rsc_stricmp( (const char *)pElement, ppStr[ nIndex ] );

    if( 0 < nCmp )
        return( GREATER );
    else if( 0 == nCmp )
        return( EQUAL );
    else
        return( LESS );
}

/*************************************************************************
|*
|*    StringCon::StringCon()
|*
|*    Beschreibung      HASHTAB.DOC
|*    Ersterstellung    MM 20.03.90
|*    Letzte Aenderung  MM 27.06.90
|*
*************************************************************************/
StringCon::StringCon( USHORT nMaxEntries ){
    // allocate character field
    pField = (char * )RscMem::Malloc( nMaxEntries );

    nMax = nMaxEntries;             // set maximum of characters
    nFill = 0;                      // no character in tabel
}

/*************************************************************************
|*
|*    StringCon::~StringCon()
|*
|*    Beschreibung      HASHTAB.DOC
|*    Ersterstellung    MM 02.06.91
|*    Letzte Aenderung  MM 02.06.91
|*
*************************************************************************/
StringCon::~StringCon(){
    // free character field
    RscMem::Free( pField );
}

/*************************************************************************
|*
|*    StringCon::Put()
|*
|*    Beschreibung      HASHTAB.DOC
|*    Ersterstellung    MM 20.03.90
|*    Letzte Aenderung  MM 27.06.90
|*
*************************************************************************/
char * StringCon :: Put( const char * pStr )
{
// put string into the tabel
    char * pReturn = pField + nFill;    // set return value

    while( nMax > (USHORT)(nFill +1) && *pStr )   // strcpy in tabel
        pField[ nFill++ ] = *pStr++;
    if( nMax == nFill +1 ){             // buffer overflow ?
        RscExit( 13 );
    }
    else
        pField[ nFill++ ] = '\0';       // terminate with zero.

    return( pReturn );                  // return pointer to string
}
