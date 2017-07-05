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
#if defined(_WIN32)
#include <direct.h>
#endif
#include <string.h>

#include <rscdef.hxx>
#include <rsctools.hxx>

#include <osl/file.h>
#include <rtl/alloc.h>
#include <rtl/character.hxx>
#include <sal/log.hxx>

/* case insensitive compare of two strings up to a given length */
int rsc_strnicmp( const char *string1, const char *string2, size_t count )
{
    return rtl_str_shortenedCompareIgnoreAsciiCase_WithLength(
                string1, strlen (string1), string2, strlen (string2), count);
}

/* case insensitive compare of two strings */
int rsc_stricmp( const char *string1, const char *string2 )
{
    return rtl_str_compareIgnoreAsciiCase( string1, string2 );
}

char* rsc_strdup( const char* pStr )
{
    int nLen = strlen( pStr );
    char* pBuffer = static_cast<char*>(rtl_allocateMemory( nLen+1 ));
    memcpy( pBuffer, pStr, nLen+1 );
    return pBuffer;
}

OString GetTmpFileName()
{
    OUString aTmpURL, aTmpFile;
    osl_createTempFile( nullptr, nullptr, &aTmpURL.pData );
    osl_getSystemPathFromFileURL( aTmpURL.pData, &aTmpFile.pData );
    return OUStringToOString( aTmpFile, RTL_TEXTENCODING_MS_1252 );
}

/* replaces extension of a file name */
OString OutputFile(const OString &rInput, const char * pExt)
{
    sal_Int32 nSepInd = rInput.lastIndexOf('.');

    if( nSepInd != -1 )
    {
        return rInput.copy(0, nSepInd + 1).concat(OString(pExt));
    }

    return rInput.concat(OString(".")).concat(OString(pExt));
}

char * ResponseFile( RscPtrPtr * ppCmd, char ** ppArgv, sal_uInt32 nArgc )
{
    FILE    *fFile;
    int     nItems;
    char    szBuffer[4096];       // file buffer
    sal_uInt32  i;
    bool bInQuotes = false;

    // program name
    ppCmd->Append( rsc_strdup( *ppArgv ) );
    for( i = 1; i < nArgc; i++ )
    {
        if( '@' == **(ppArgv +i) ){ // when @, then response file
            if( nullptr == (fFile = fopen( (*(ppArgv +i)) +1, "r" )) )
                return *(ppArgv +i);
            nItems = fread( &szBuffer[ 0 ], 1, sizeof( char ), fFile );
            while( nItems )
            {
                if( !rtl::isAsciiWhiteSpace(
                        static_cast<unsigned char>(szBuffer[ 0 ]) ) )
                {
                    /*
                     *  #i27914# double ticks '"' now have a duplicate function:
                     *  1. they define a string ( e.g. -DFOO="baz" )
                     *  2. a string can contain spaces, so -DFOO="baz zum" defines one
                     *  argument no two !
                     */
                    unsigned int n = 0;
                    while( nItems &&
                           (!rtl::isAsciiWhiteSpace(
                               static_cast<unsigned char>(szBuffer[ n ]) ) ||
                            bInQuotes) &&
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
            }

            fclose( fFile );
        }
        else
            ppCmd->Append( rsc_strdup( *(ppArgv +i) ) );
    }
    ppCmd->Append( static_cast<void *>(nullptr) );
    return nullptr;
}


RscPtrPtr::RscPtrPtr()
{
    nCount = 0;
    pMem = nullptr;
}

RscPtrPtr::~RscPtrPtr()
{
    Reset();
}

void RscPtrPtr::Reset()
{
    sal_uInt32 i;

    if( pMem )
    {
        for( i = 0; i < nCount; i++ )
        {
            if( pMem[ i ] )
               rtl_freeMemory( pMem[ i ] );
        }
        rtl_freeMemory( static_cast<void *>(pMem) );
    };
    nCount = 0;
    pMem = nullptr;
}

sal_uInt32 RscPtrPtr::Append( void * pBuffer )
{
    if( !pMem )
        pMem = static_cast<void **>(rtl_allocateMemory( (nCount +1) * sizeof( void * ) ));
    else
        pMem = static_cast<void **>(rtl_reallocateMemory( static_cast<void *>(pMem),
                         ((nCount +1) * sizeof( void * )
                       ) ));
    pMem[ nCount ] = pBuffer;
    return nCount++;
}

void * RscPtrPtr::GetEntry( sal_uInt32 nEntry )
{
    if( nEntry < nCount )
        return pMem[ nEntry ];
    return nullptr;
}

RscWriteRc::RscWriteRc( RSCBYTEORDER_TYPE nOrder )
{
    bSwap = false;
    if( nOrder != RSC_SYSTEMENDIAN )
    {
        RSCBYTEORDER_TYPE nMachineOrder;
#if defined OSL_LITENDIAN
        nMachineOrder = RSC_LITTLEENDIAN;
#else
        nMachineOrder = RSC_BIGENDIAN;
#endif
        bSwap = nOrder != nMachineOrder;
    }
    nLen = 0;
    pMem = nullptr;
}

RscWriteRc::~RscWriteRc()
{
    if( pMem )
        rtl_freeMemory( pMem );
}

sal_uInt32 RscWriteRc::IncSize( sal_uInt32 nSize )
{
    sal_uInt32 nOrigPos = nLen;
    nLen += nSize;
    if( pMem )
        pMem = static_cast<char*>(rtl_reallocateMemory( pMem, nLen ));
    if( pMem )
        memset( pMem + nOrigPos, 0, nSize );
    return nOrigPos;
}

char * RscWriteRc::GetPointer( sal_uInt32 nSize )
{
    if( !pMem )
    {
        pMem = static_cast<char *>(rtl_allocateMemory( nLen ));
        memset( pMem, 0, nLen );
    }
    return pMem + nSize;
}


void RscWriteRc::Put( sal_uInt16 nVal )
{
    sal_uInt32  nOldLen;

    nOldLen = IncSize( sizeof( nVal ) );
    PutAt( nOldLen, nVal );
}

void RscWriteRc::PutUTF8( char * pStr )
{
    sal_uInt32 nStrLen = 0;
    if( pStr )
        nStrLen = strlen( pStr );

    sal_uInt32  n = nStrLen +1;
    if( n % 2 )
        // align to 2
        n++;

    sal_uInt32  nOldLen = IncSize( n );
    char * p = GetPointer( nOldLen );
    if (nStrLen != 0)
    {
        memcpy( p, pStr, nStrLen );
    }
    // 0 terminated
    pMem[ nOldLen + nStrLen ] = '\0';
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
