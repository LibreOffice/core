/*************************************************************************
 *
 *  $RCSfile: rsctools.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: mm $ $Date: 2000-09-27 11:26:12 $
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

    $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/rsc/source/tools/rsctools.cxx,v 1.2 2000-09-27 11:26:12 mm Exp $

    Source Code Control System - Update

    $Log: not supported by cvs2svn $
    Revision 1.1.1.1  2000/09/18 16:42:56  hr
    initial import

    Revision 1.32  2000/09/17 12:51:12  willem.vandorp
    OpenOffice header added.

    Revision 1.31  2000/09/07 04:30:57  patrick.luby
    Replaced tempnam() with tmpnam() as tempnam() keeps returning the same file name on Mac OS X.

    Revision 1.30  2000/07/26 17:13:24  willem.vandorp
    Headers/footers replaced

    Revision 1.29  2000/07/11 17:17:46  th
    Unicode

    Revision 1.28  1999/12/08 15:33:51  mh
    tmp file not in actual directory

    Revision 1.27  1999/09/21 17:08:45  mm
    Bigendian correct

**************************************************************************/
/****************** I N C L U D E S **************************************/

// C and C++ Includes.
#include <stdlib.h>
#include <stdio.h>
#if defined ( DOS ) || defined ( WIN ) || defined (WNT )
#include <direct.h>
#endif
#if defined ( OS2 ) && !defined ( GCC )
#include <direct.h>
#endif
#include <string.h>
#include <ctype.h>
#ifdef MAC
#include <cursorctl.h>
#endif

#include <tools/fsys.hxx>

// Include
#ifndef _RSCDEF_HXX
#include <rscdef.hxx>
#endif
#ifndef _RSCTOOLS_HXX
#include <rsctools.hxx>
#endif

#if defined (WIN) || defined (MAC)
#define ONLY_NEW
#endif

/****************** C o d e **********************************************/
/*************************************************************************
|*
|*    rsc_strnicmp()
|*
|*    Beschreibung      Vergleicht zwei Strings Case-Unabhaengig bis zu
|*                      einer bestimmten Laenge
|*    Ersterstellung    MM 13.02.91
|*    Letzte Aenderung  MM 13.02.91
|*
*************************************************************************/
int rsc_strnicmp( const char *string1, const char *string2, size_t count )
{
    size_t i;

    for( i = 0; ( i < count ) && string1[ i ] && string2[ i ] ; i++ )
    {
        if( tolower( string1[ i ] ) < tolower( string2[ i ] ) )
            return( -1 );
        else if( tolower( string1[ i ] ) > tolower( string2[ i ] ) )
            return( 1 );
    }
    if( i == count )
        return( 0 );
    else if( tolower( string1[ i ] ) < tolower( string2[ i ] ) )
        return( -1 );
    else if( tolower( string1[ i ] ) > tolower( string2[ i ] ) )
        return( 1 );
    return( 0 );
}

/*************************************************************************
|*
|*    rsc_strnicmp()
|*
|*    Beschreibung      Vergleicht zwei Strings Case-Unabhaengig
|*    Ersterstellung    MM 13.02.91
|*    Letzte Aenderung  MM 13.02.91
|*
*************************************************************************/
int rsc_stricmp( const char *string1, const char *string2 ){
    int i;

    for( i = 0; string1[ i ] && string2[ i ]; i++ ){
        if( tolower( string1[ i ] ) < tolower( string2[ i ] ) )
            return( -1 );
        else if( tolower( string1[ i ] ) > tolower( string2[ i ] ) )
            return( 1 );
    }
    if( tolower( string1[ i ] ) < tolower( string2[ i ] ) )
        return( -1 );
    else if( tolower( string1[ i ] ) > tolower( string2[ i ] ) )
        return( 1 );
    return( 0 );
}

/*************************************************************************
|*
|*    GetTmpFileName()
|*
|*    Beschreibung      Gibt einen String eines eindeutigen Dateinamens
|*                      zurueck. Der Speicher fuer den String wird mit
|*                      malloc allokiert
|*    Ersterstellung    MM 13.02.91
|*    Letzte Aenderung  MH 13.10.97
|*
*************************************************************************/
ByteString GetTmpFileName()
{
 #ifdef MACOSX
    // Use tmpnam instead of tempnam as tempnam has some bugs in Mac OS X
    return ByteString( tmpnam( NULL ) );
#else
    return ByteString( tempnam( (const char *) P_tmpdir, NULL ) );
#endif
}

/********************************************************************/
/*                                                                  */
/*  Function    :   Append( )                                       */
/*                                                                  */
/*  Parameters  :   psw     - pointer to a preprocessor switch      */
/*                                                                  */
/*  Description :   appends text files                              */
/********************************************************************/
BOOL Append( FILE * fDest, ByteString aTmpFile )
{
#define MAX_BUF 1000
    char    szBuf[ MAX_BUF ];
    short   nItems;
    FILE    *fSource;

    fSource = fopen( aTmpFile.GetBuffer(), "rb" );
    if( !fDest || !fSource ){
        if( fSource )
            fclose( fSource );
        return FALSE;
    }
    else{
        do{ // append
            nItems = fread( szBuf, sizeof( char ), MAX_BUF, fSource );
            fwrite( szBuf, sizeof( char ), nItems, fDest );
        } while( MAX_BUF == nItems );

        fclose( fSource );
    };
    return TRUE;
}

BOOL Append( ByteString aOutputSrs, ByteString aTmpFile )
{
    FILE * fDest   = fopen( aOutputSrs.GetBuffer(), "ab" );

    BOOL bRet = Append( fDest, aTmpFile );

    if( fDest )
        fclose( fDest );

    return bRet;
}

/*************************************************************************
|*
|*    InputFile
|*
|*    Beschreibung      Haengt Extension an, wenn keine da ist
|*    Parameter:        pInput, der Input-Dateiname.
|*                      pExt, die Extension des Ausgabenamens
|*    Ersterstellung    MM 13.02.91
|*    Letzte Aenderung  MM 28.06.91
|*
*************************************************************************/
ByteString InputFile ( char * pInput, char * pExt )
{
    UniString aUniInput( pInput, RTL_TEXTENCODING_ASCII_US );
    DirEntry aFileName( aUniInput );

    if ( 0 == aFileName.GetExtension().Len() )
    {
        UniString aExt( pExt, RTL_TEXTENCODING_ASCII_US );
        aFileName.SetExtension( aExt );
    }

    return ByteString( aFileName.GetFull(), RTL_TEXTENCODING_ASCII_US );
}

/*************************************************************************
|*
|*    OutputFile
|*
|*    Beschreibung      Ersetzt Extension durch eine andere
|*    Parameter:        input, der Input-Dateiname.
|*                      pExt, die Extension des Ausgabenamens
|*    Ersterstellung    MM 13.02.91
|*    Letzte Aenderung  MM 28.06.91
|*
*************************************************************************/
ByteString OutputFile ( ByteString aInput, char * pExt )
{
    UniString   aUniInput( aInput, RTL_TEXTENCODING_ASCII_US );
    DirEntry    aFileName( aUniInput );

    UniString aExt( pExt, RTL_TEXTENCODING_ASCII_US );
    aFileName.SetExtension( aExt );

    return ByteString( aFileName.GetFull(), RTL_TEXTENCODING_ASCII_US );
}

/*************************************************************************
|*
|*    ::ResonseFile()
|*
|*    Beschreibung      Kommandozeile aufbereiten
|*    Ersterstellung    MM 05.09.91
|*    Letzte Aenderung  MM 05.09.91
|*
*************************************************************************/
char * ResponseFile( RscPtrPtr * ppCmd, char ** ppArgv,
                   USHORT nArgc )
{
    FILE    *fFile;
    int     nItems;
    char    szBuffer[4096];       // file buffer
    USHORT  i;

    // Programmname
    ppCmd->Append( ((RscMem *)0)->Assignsw( *ppArgv, 0 ) );
    for( i = 1; i < nArgc; i++ )
    {
        if( '@' == **(ppArgv +i) ){ // wenn @, dann Response-Datei
            if( NULL == (fFile = fopen( (*(ppArgv +i)) +1, "r" )) )
                return( (*(ppArgv +i)) );

            nItems = fread( &szBuffer[ 0 ], 1, sizeof( char ), fFile );
            while( nItems )
            {
                if( !isspace( szBuffer[ 0 ] ) )
                {
                    USHORT n = 0;
                    while( nItems && !isspace( szBuffer[ n ] ) &&
                           n +1 < sizeof( szBuffer )  )
                    {
                        n++;
                        nItems = fread( &szBuffer[ n ], 1,
                                        sizeof( char ), fFile );
                    }
                    szBuffer[ n ] = '\0';
                    ppCmd->Append( ((RscMem *)0)->Assignsw( szBuffer, 0 ) );
                }
                nItems = fread( &szBuffer[ 0 ], 1, sizeof( char ), fFile );
            };

            fclose( fFile );
        }
        else
            ppCmd->Append( ((RscMem *)0)->Assignsw( *(ppArgv +i), 0 ) );
    };
    ppCmd->Append( (void *)0 );
    return( NULL );
}


/*************** R s c M e m ********************************************/

#ifdef DOS
class RscCount {
public:
    long nCount;
    RscCount(){ nCount = 0; }
//  ~RscCount(){ printf( "\nMalloczaehler: %ld\n", nCount ); }
};
RscCount aRscCount;
#endif
/*************************************************************************
|*
|*    RscMem :: Malloc()
|*
|*    Beschreibung      Speicher allokieren
|*    Ersterstellung    MM 13.02.91
|*    Letzte Aenderung  MM 13.02.91
|*
*************************************************************************/
void * RscMem :: Malloc( USHORT nSize ){
    char * pMem;

#ifdef MAC
    SpinCursor( 1 );
#endif
#ifdef DOS
    aRscCount.nCount++;
#endif
#ifdef ONLY_NEW
    pMem = new char[ nSize + sizeof( USHORT ) ];
    *(USHORT *)pMem = nSize;
    pMem += sizeof( USHORT );
#else
    if( NULL == (pMem = (char *)malloc( nSize )) )
        RscExit( 10 );
#endif
    return( pMem );
}

/*************************************************************************
|*
|*    RscMem :: Realloc()
|*
|*    Beschreibung      Speicher zusaetzlich allokieren
|*    Ersterstellung    MM 13.02.91
|*    Letzte Aenderung  MM 13.02.91
|*
*************************************************************************/
void * RscMem :: Realloc( void * pMem, USHORT nSize ){
#ifdef MAC
    SpinCursor( 1 );
#endif
#ifdef ONLY_NEW
    char * pTmp = (char *)pMem;
    USHORT  nMin, nOldSize;

    pMem = Malloc( nSize );
    nOldSize = *(USHORT *)(pTmp - sizeof( USHORT ) );
    nMin = (nSize < nOldSize) ? nSize : nOldSize;
    memcpy( pMem, pTmp, nMin );
    delete (pTmp - sizeof( USHORT ));
#else
#ifdef UNX
    if( NULL == (pMem = realloc( (char*)pMem, nSize )) )
#else
    if( NULL == (pMem = realloc( pMem, nSize )) )
#endif
        RscExit( 10 );
#endif
    return( pMem );
}

/*************************************************************************
|*
|*    RscMem :: Free()
|*
|*    Beschreibung      Speicher freigeben
|*    Ersterstellung    MM 13.02.91
|*    Letzte Aenderung  MM 13.02.91
|*
*************************************************************************/
void RscMem :: Free( void * pMem ){
#ifdef DOS
    aRscCount.nCount--;
#endif
#ifdef ONLY_NEW
    delete ((char *)pMem - sizeof( USHORT ));
#else
#ifdef UNX
    free( (char *)pMem );
#else
    free( pMem );
#endif
#endif
}

/*************************************************************************
|*
|*    RscMem :: Assignsw
|*
|*    Beschreibung      Allokiert speicher fuer String und kopiert ihn
|*    Parameter:        psw, der String.
|*                      nExtraSpace, weiterer Speicher hinter dem String
|*    Ersterstellung    MM 13.02.91
|*    Letzte Aenderung  MM 13.02.91
|*
*************************************************************************/
char * RscMem :: Assignsw( const char *psw, short nExtraSpace )
{
    char * pTmp;

    if( !psw )
        psw = "";
    /* allocate memory    */
    pTmp = (char *)Malloc( strlen( psw ) + 1 + nExtraSpace );
    /* copy string into allocated memory*/
    strcpy( pTmp, psw );
    return( pTmp );
}

/*************** R s c P t r P t r **************************************/
/*************************************************************************
|*
|*    RscPtrPtr :: RscPtrPtr()
|*
|*    Beschreibung      Eine Tabelle mit Zeigern
|*    Ersterstellung    MM 13.02.91
|*    Letzte Aenderung  MM 13.02.91
|*
*************************************************************************/
RscPtrPtr :: RscPtrPtr(){
    nCount = 0;
    pMem = NULL;
}

/*************************************************************************
|*
|*    RscPtrPtr :: ~RscPtrPtr()
|*
|*    Beschreibung      Zerst”rt eine Tabelle mit Zeigern, die Zeiger werde
|*                      ebenfalls freigegebn
|*    Ersterstellung    MM 13.02.91
|*    Letzte Aenderung  MM 13.02.91
|*
*************************************************************************/
RscPtrPtr :: ~RscPtrPtr(){
    Reset();
}

/*************************************************************************
|*
|*    RscPtrPtr :: Reset()
|*
|*    Beschreibung
|*    Ersterstellung    MM 03.05.91
|*    Letzte Aenderung  MM 03.05.91
|*
*************************************************************************/
void RscPtrPtr :: Reset(){
    USHORT i;

    if( pMem ){
        for( i = 0; i < nCount; i++ ){
            if( pMem[ i ] )
               RscMem::Free( pMem[ i ] );
        }
        RscMem::Free( (void *)pMem );
    };
    nCount = 0;
    pMem = NULL;
}

/*************************************************************************
|*
|*    RscPtrPtr :: Append()
|*
|*    Beschreibung      Haengt einen Eintrag an.
|*    Ersterstellung    MM 13.02.91
|*    Letzte Aenderung  MM 13.02.91
|*
*************************************************************************/
USHORT RscPtrPtr :: Append( void * pBuffer ){
    if( !pMem )
        pMem = (void **)RscMem::Malloc( (nCount +1) * sizeof( void * ) );
    else
        pMem = (void **)RscMem::Realloc( (void *)pMem,
                         (USHORT)((nCount +1) * sizeof( void * )
                       ) );
    pMem[ nCount ] = pBuffer;
    return( nCount++ );
}

/*************************************************************************
|*
|*    RscPtrPtr :: GetEntry()
|*
|*    Beschreibung      Liefert einen Eintrag, NULL wenn nicht vorhanden.
|*    Ersterstellung    MM 13.02.91
|*    Letzte Aenderung  MM 13.02.91
|*
*************************************************************************/
void * RscPtrPtr :: GetEntry( USHORT nEntry ){
    if( nEntry < nCount )
        return( pMem[ nEntry ] );
    return( NULL );
}

/****************** R S C W R I T E R C **********************************/
/*************************************************************************
|*
|*    RscWriteRc :: RscWriteRc()
|*
|*    Beschreibung
|*    Ersterstellung    MM 15.04.91
|*    Letzte Aenderung  MM 15.04.91
|*
*************************************************************************/
RscWriteRc::RscWriteRc( RSCBYTEORDER_TYPE nOrder )
{
    short               nSwapTest = 1;
    RSCBYTEORDER_TYPE   nMachineOrder;

    bSwap = FALSE;
    if( nOrder != RSC_SYSTEMENDIAN )
    {
        if( (BYTE)*(BYTE *)&nSwapTest )
            nMachineOrder = RSC_LITTLEENDIAN;
        else
            nMachineOrder = RSC_BIGENDIAN;
        bSwap = nOrder != nMachineOrder;
    }
    nByteOrder = nOrder;
    nLen = 0;
    pMem = NULL;
}

/*************************************************************************
|*
|*    RscWriteRc :: ~RscWriteRc()
|*
|*    Beschreibung
|*    Ersterstellung    MM 15.04.91
|*    Letzte Aenderung  MM 15.04.91
|*
*************************************************************************/
RscWriteRc :: ~RscWriteRc()
{
    if( pMem )
        RscMem::Free( pMem );
}

/*************************************************************************
|*
|*    RscWriteRc :: IncSize()
|*
|*    Beschreibung
|*    Ersterstellung    MM 15.04.91
|*    Letzte Aenderung  MM 15.04.91
|*
*************************************************************************/
USHORT RscWriteRc :: IncSize( USHORT nSize )
{
    nLen += nSize;
    if( pMem )
        pMem = RscMem::Realloc( pMem, nLen );
    return( nLen - nSize );
}

/*************************************************************************
|*
|*    RscWriteRc :: GetPointer()
|*
|*    Beschreibung
|*    Ersterstellung    MM 15.04.91
|*    Letzte Aenderung  MM 15.04.91
|*
*************************************************************************/
char * RscWriteRc :: GetPointer( USHORT nSize )
{
    if( !pMem )
        pMem = (char *)RscMem::Malloc( nLen );
    return( pMem + nSize );
}


/*************************************************************************
|*
|*    RscWriteRc :: Put()
|*
|*    Beschreibung
|*    Ersterstellung    MM 15.04.91
|*    Letzte Aenderung  MM 15.04.91
|*
*************************************************************************/
void RscWriteRc :: Put( USHORT nVal )
{
    USHORT  nOldLen;

    nOldLen = IncSize( sizeof( nVal ) );
    PutAt( nOldLen, nVal );
}

void RscWriteRc :: PutUTF8( char * pStr )
{
    USHORT nStrLen = 0;
    if( pStr )
        nStrLen = strlen( pStr );

    USHORT  n = nStrLen +1;
    if( n % 2 )
        // align to 2
        n++;

    USHORT  nOldLen = IncSize( n );
    memcpy( GetPointer( nOldLen ), pStr, nStrLen );
    // 0 terminated
    pMem[ nOldLen + nStrLen ] = '\0';
}
