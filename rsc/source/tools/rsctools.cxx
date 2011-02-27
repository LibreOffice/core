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

#include <tools/fsys.hxx>

// Include
#include <rscdef.hxx>
#include <rsctools.hxx>

#include <osl/file.h>
#include <rtl/alloc.h>
#include <rtl/memory.h>

#if defined (WIN)
#define ONLY_NEW
#endif

using ::rtl::OUString;
using ::rtl::OUStringToOString;

/****************** C o d e **********************************************/
/*************************************************************************
|*
|*    rsc_strnicmp()
|*
|*    Beschreibung      Vergleicht zwei Strings Case-Unabhaengig bis zu
|*                      einer bestimmten Laenge
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

char* rsc_strdup( const char* pStr )
{
    int nLen = strlen( pStr );
    char* pBuffer = (char*)rtl_allocateMemory( nLen+1 );
    rtl_copyMemory( pBuffer, pStr, nLen+1 );
    return pBuffer;
}

/*************************************************************************
|*
|*    GetTmpFileName()
|*
|*    Beschreibung      Gibt einen String eines eindeutigen Dateinamens
|*                      zurueck. Der Speicher fuer den String wird mit
|*                      malloc allokiert
|*
*************************************************************************/
ByteString GetTmpFileName()
{
    OUString aTmpURL, aTmpFile;
    osl_createTempFile( NULL, NULL, &aTmpURL.pData );
    osl_getSystemPathFromFileURL( aTmpURL.pData, &aTmpFile.pData );
    return OUStringToOString( aTmpFile, RTL_TEXTENCODING_MS_1252 );
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
#define MAX_BUF 4096
    FILE *fSource = fopen( aTmpFile.GetBuffer(), "rb" );
    if( !fDest || !fSource )
    {
        if( fSource )
            fclose( fSource );
        return FALSE;
    }
    else
    {
        char szBuf[ MAX_BUF ];
        int nItems;

        do //appemd
        {
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
|*
*************************************************************************/
ByteString InputFile ( const char * pInput, const char * pExt )
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
|*
*************************************************************************/
ByteString OutputFile ( ByteString aInput, const char * pExt )
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
|*
*************************************************************************/
char * ResponseFile( RscPtrPtr * ppCmd, char ** ppArgv, sal_uInt32 nArgc )
{
    FILE    *fFile;
    int     nItems;
    char    szBuffer[4096];       // file buffer
    sal_uInt32  i;
    bool bInQuotes = false;

    // Programmname
    ppCmd->Append( rsc_strdup( *ppArgv ) );
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
                    /*
                     *  #i27914# double ticks '"' now have a duplicate function:
                     *  1. they define a string ( e.g. -DFOO="baz" )
                     *  2. a string can contain spaces, so -DFOO="baz zum" defines one
                     *  argument no two !
                     */
                    unsigned int n = 0;
                    while( nItems && (!isspace( szBuffer[ n ] ) || bInQuotes) &&
                           n +1 < sizeof( szBuffer )  )
                    {
                        n++;
                        nItems = fread( &szBuffer[ n ], 1,
                                        sizeof( char ), fFile );
                        if( szBuffer[n] == '"' )
                            bInQuotes = !bInQuotes;
                    }
                    szBuffer[ n ] = '\0';
                    ppCmd->Append( rsc_strdup( szBuffer ) );
                }
                nItems = fread( &szBuffer[ 0 ], 1, sizeof( char ), fFile );
            };

            fclose( fFile );
        }
        else
            ppCmd->Append( rsc_strdup( *(ppArgv +i) ) );
    };
    ppCmd->Append( (void *)0 );
    return( NULL );
}


/*************** R s c P t r P t r **************************************/
/*************************************************************************
|*
|*    RscPtrPtr :: RscPtrPtr()
|*
|*    Beschreibung      Eine Tabelle mit Zeigern
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
|*    Beschreibung      Zerst�rt eine Tabelle mit Zeigern, die Zeiger werde
|*                      ebenfalls freigegebn
|*
*************************************************************************/
RscPtrPtr :: ~RscPtrPtr(){
    Reset();
}

/*************************************************************************
|*
|*    RscPtrPtr :: Reset()
|*
*************************************************************************/
void RscPtrPtr :: Reset(){
    sal_uInt32 i;

    if( pMem ){
        for( i = 0; i < nCount; i++ ){
            if( pMem[ i ] )
               rtl_freeMemory( pMem[ i ] );
        }
        rtl_freeMemory( (void *)pMem );
    };
    nCount = 0;
    pMem = NULL;
}

/*************************************************************************
|*
|*    RscPtrPtr :: Append()
|*
|*    Beschreibung      Haengt einen Eintrag an.
|*
*************************************************************************/
sal_uInt32 RscPtrPtr :: Append( void * pBuffer ){
    if( !pMem )
        pMem = (void **)rtl_allocateMemory( (nCount +1) * sizeof( void * ) );
    else
        pMem = (void **)rtl_reallocateMemory( (void *)pMem,
                         ((nCount +1) * sizeof( void * )
                       ) );
    pMem[ nCount ] = pBuffer;
    return( nCount++ );
}

/*************************************************************************
|*
|*    RscPtrPtr :: GetEntry()
|*
|*    Beschreibung      Liefert einen Eintrag, NULL wenn nicht vorhanden.
|*
*************************************************************************/
void * RscPtrPtr :: GetEntry( sal_uInt32 nEntry ){
    if( nEntry < nCount )
        return( pMem[ nEntry ] );
    return( NULL );
}

/****************** R S C W R I T E R C **********************************/
/*************************************************************************
|*
|*    RscWriteRc :: RscWriteRc()
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
*************************************************************************/
RscWriteRc :: ~RscWriteRc()
{
    if( pMem )
        rtl_freeMemory( pMem );
}

/*************************************************************************
|*
|*    RscWriteRc :: IncSize()
|*
*************************************************************************/
sal_uInt32 RscWriteRc :: IncSize( sal_uInt32 nSize )
{
    nLen += nSize;
    if( pMem )
        pMem = (char*)rtl_reallocateMemory( pMem, nLen );
    return( nLen - nSize );
}

/*************************************************************************
|*
|*    RscWriteRc :: GetPointer()
|*
*************************************************************************/
char * RscWriteRc :: GetPointer( sal_uInt32 nSize )
{
    if( !pMem )
        pMem = (char *)rtl_allocateMemory( nLen );
    return( pMem + nSize );
}


/*************************************************************************
|*
|*    RscWriteRc :: Put()
|*
*************************************************************************/
void RscWriteRc :: Put( sal_uInt16 nVal )
{
    sal_uInt32  nOldLen;

    nOldLen = IncSize( sizeof( nVal ) );
    PutAt( nOldLen, nVal );
}

void RscWriteRc :: PutUTF8( char * pStr )
{
    sal_uInt32 nStrLen = 0;
    if( pStr )
        nStrLen = strlen( pStr );

    sal_uInt32  n = nStrLen +1;
    if( n % 2 )
        // align to 2
        n++;

    sal_uInt32  nOldLen = IncSize( n );
    rtl_copyMemory( GetPointer( nOldLen ), pStr, nStrLen );
    // 0 terminated
    pMem[ nOldLen + nStrLen ] = '\0';
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
