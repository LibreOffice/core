/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <osl/diagnose.h>
#include "ImplHelper.hxx"
#include <rtl/tencinfo.h>
#include <string.h>
#include <memory>

#if defined _MSC_VER
#pragma warning(push,1)
#endif
#include <windows.h>
#if defined _MSC_VER
#pragma warning(pop)
#endif

#include <vector>

#if defined ( __MINGW32__ )
#include <sehandler.hxx>
#endif

#define FORMATETC_EXACT_MATCH    1
#define FORMATETC_PARTIAL_MATCH -1
#define FORMATETC_NO_MATCH       0











sal_uInt32 SAL_CALL getWinCPFromMimeCharset( const OUString& charset )
{
    sal_uInt32 winCP = GetACP( );

    if ( charset.getLength( ) )
    {
        OString osCharset(
            charset.getStr( ), charset.getLength( ), RTL_TEXTENCODING_ASCII_US );

        rtl_TextEncoding txtEnc =
            rtl_getTextEncodingFromMimeCharset( osCharset.getStr( ) );

        sal_uIntPtr winChrs = rtl_getBestWindowsCharsetFromTextEncoding( txtEnc );

        CHARSETINFO chrsInf;
        sal_Bool bRet = TranslateCharsetInfo( (DWORD*)winChrs, &chrsInf, TCI_SRCCHARSET ) ?
                        sal_True : sal_False;

        
        
        
        if ( bRet )
            winCP = chrsInf.ciACP;
    }

    return winCP;
}






OUString SAL_CALL getWinCPFromLocaleId( LCID lcid, LCTYPE lctype )
{
    OSL_ASSERT( IsValidLocale( lcid, LCID_SUPPORTED ) );

    
    OUString winCP;

    
    if ( LOCALE_IDEFAULTCODEPAGE == lctype )
    {
        winCP = OUString::number( static_cast<sal_Int32>(GetOEMCP( )) );
    }
    else if ( LOCALE_IDEFAULTANSICODEPAGE == lctype )
    {
        winCP = OUString::number( static_cast<sal_Int32>(GetACP( )) );
    }
    else
        OSL_ASSERT( sal_False );

    
    
    char buff[6];
    sal_Int32 nResult = GetLocaleInfoA(
        lcid, lctype | LOCALE_USE_CP_ACP, buff, sizeof( buff ) );

    OSL_ASSERT( nResult );

    if ( nResult )
    {
        sal_Int32 len = MultiByteToWideChar(
            CP_ACP, 0, buff, -1, NULL, 0 );

        OSL_ASSERT( len > 0 );

        std::vector< sal_Unicode > lpwchBuff(len);

        len = MultiByteToWideChar(
            CP_ACP, 0, buff, -1, reinterpret_cast<LPWSTR>(&lpwchBuff[0]), len );

        winCP = OUString( &lpwchBuff[0], (len - 1) );
    }

    return winCP;
}







OUString SAL_CALL getMimeCharsetFromWinCP( sal_uInt32 cp, const OUString& aPrefix )
{
    return aPrefix + cptostr( cp );
}







OUString SAL_CALL getMimeCharsetFromLocaleId( LCID lcid, LCTYPE lctype, const OUString& aPrefix  )
{
    OUString charset = getWinCPFromLocaleId( lcid, lctype );
    return aPrefix + charset;
}





sal_Bool SAL_CALL IsOEMCP( sal_uInt32 codepage )
{
    OSL_ASSERT( IsValidCodePage( codepage ) );

    sal_uInt32 arrOEMCP[] = { 437, 708, 709, 710, 720, 737,
                              775, 850, 852, 855, 857, 860,
                              861, 862, 863, 864, 865, 866,
                              869, 874, 932, 936, 949, 950, 1361 };

    for ( size_t i = 0; i < SAL_N_ELEMENTS( arrOEMCP ); ++i )
        if ( arrOEMCP[i] == codepage )
            return sal_True;

    return sal_False;
}





OUString SAL_CALL cptostr( sal_uInt32 codepage )
{
    OSL_ASSERT( IsValidCodePage( codepage ) );

    return OUString::number( static_cast<sal_Int64>( codepage ) );
}



//

//

//




void SAL_CALL DeleteTargetDevice( DVTARGETDEVICE* ptd )
{
#if defined ( __MINGW32__ ) && !defined ( _WIN64 )
    jmp_buf jmpbuf;
    __SEHandler han;
    if (__builtin_setjmp(jmpbuf) == 0)
    {
        han.Set(jmpbuf, NULL, (__SEHandler::PF)EXCEPTION_EXECUTE_HANDLER);
#else
    __try
    {
#endif
        CoTaskMemFree( ptd );
    }
#if defined ( __MINGW32__ ) && !defined ( _WIN64 )
    else
#else
    __except( EXCEPTION_EXECUTE_HANDLER )
#endif
    {
        OSL_FAIL( "Error DeleteTargetDevice" );
    }
#if defined ( __MINGW32__ ) && !defined ( _WIN64 )
    han.Reset();
#endif
}





//





//


//






DVTARGETDEVICE* SAL_CALL CopyTargetDevice( DVTARGETDEVICE* ptdSrc )
{
    DVTARGETDEVICE* ptdDest = NULL;

#if defined ( __MINGW32__ ) && !defined ( _WIN64 )
    jmp_buf jmpbuf;
    __SEHandler han;
    if (__builtin_setjmp(jmpbuf) == 0)
    {
        han.Set(jmpbuf, NULL, (__SEHandler::PF)EXCEPTION_EXECUTE_HANDLER);
#else
    __try
    {
#endif
        if ( NULL != ptdSrc )
        {
            ptdDest = static_cast< DVTARGETDEVICE* >( CoTaskMemAlloc( ptdSrc->tdSize ) );
            memcpy( ptdDest, ptdSrc, static_cast< size_t >( ptdSrc->tdSize ) );
        }
    }
#if defined ( __MINGW32__ ) && !defined ( _WIN64 )
    han.Reset();
#else
    __except( EXCEPTION_EXECUTE_HANDLER )
    {
    }
#endif

    return ptdDest;
}




//







//




//



//






sal_Bool SAL_CALL CopyFormatEtc( LPFORMATETC petcDest, LPFORMATETC petcSrc )
{
    sal_Bool bRet = sal_False;

#if defined ( __MINGW32__ ) && !defined ( _WIN64 )
    jmp_buf jmpbuf;
    __SEHandler han;
    if (__builtin_setjmp(jmpbuf) == 0)
    {
        han.Set(jmpbuf, NULL, (__SEHandler::PF)EXCEPTION_EXECUTE_HANDLER);
#else
    __try
    {
#endif
        if ( petcDest != petcSrc )
        {

        petcDest->cfFormat = petcSrc->cfFormat;

        petcDest->ptd      = NULL;
        if ( NULL != petcSrc->ptd )
            petcDest->ptd  = CopyTargetDevice(petcSrc->ptd);

        petcDest->dwAspect = petcSrc->dwAspect;
        petcDest->lindex   = petcSrc->lindex;
        petcDest->tymed    = petcSrc->tymed;

        bRet = sal_True;
        }
    }
#if defined ( __MINGW32__ ) && !defined ( _WIN64 )
    else
#else
    __except( EXCEPTION_EXECUTE_HANDLER )
#endif
    {
        OSL_FAIL( "Error CopyFormatEtc" );
    }
#if defined ( __MINGW32__ ) && !defined ( _WIN64 )
    han.Reset();
#endif

    return bRet;
}









sal_Int32 SAL_CALL CompareFormatEtc( const FORMATETC* pFetcLhs, const FORMATETC* pFetcRhs )
{
    sal_Int32 nMatch = FORMATETC_EXACT_MATCH;

#if defined ( __MINGW32__ ) && !defined ( _WIN64 )
    jmp_buf jmpbuf;
    __SEHandler han;
    if (__builtin_setjmp(jmpbuf) == 0)
    {
        han.Set(jmpbuf, NULL, (__SEHandler::PF)EXCEPTION_EXECUTE_HANDLER);
#else
    __try
    {
#endif
        if ( pFetcLhs != pFetcRhs )
        {
            if ( ( pFetcLhs->cfFormat != pFetcRhs->cfFormat ) ||
                ( pFetcLhs->lindex   != pFetcRhs->lindex ) ||
                !CompareTargetDevice( pFetcLhs->ptd, pFetcRhs->ptd ) )
            {
                nMatch = FORMATETC_NO_MATCH;
            }

            else if ( pFetcLhs->dwAspect == pFetcRhs->dwAspect )
                
                ;
            else if ( ( pFetcLhs->dwAspect & ~pFetcRhs->dwAspect ) != 0 )
            {
                
                nMatch = FORMATETC_NO_MATCH;
            }
            else
                
                nMatch = FORMATETC_PARTIAL_MATCH;

            if ( nMatch == FORMATETC_EXACT_MATCH || nMatch == FORMATETC_PARTIAL_MATCH )
            {
                if ( pFetcLhs->tymed == pFetcRhs->tymed )
                    
                    ;
                else if ( ( pFetcLhs->tymed & ~pFetcRhs->tymed ) != 0 )
                {
                    
                    nMatch = FORMATETC_NO_MATCH;
                }
                else
                    
                    nMatch = FORMATETC_PARTIAL_MATCH;
            }
        }
    }
#if defined ( __MINGW32__ ) && !defined ( _WIN64 )
    else
#else
    __except( EXCEPTION_EXECUTE_HANDLER )
#endif
    {
        OSL_FAIL( "Error CompareFormatEtc" );
        nMatch = FORMATETC_NO_MATCH;
    }
#if defined ( __MINGW32__ ) && !defined ( _WIN64 )
    han.Reset();
#endif

    return nMatch;
}



//


sal_Bool SAL_CALL CompareTargetDevice( DVTARGETDEVICE* ptdLeft, DVTARGETDEVICE* ptdRight )
{
    sal_Bool bRet = sal_False;

#if defined ( __MINGW32__ ) && !defined ( _WIN64 )
    jmp_buf jmpbuf;
    __SEHandler han;
    if (__builtin_setjmp(jmpbuf) == 0)
    {
        han.Set(jmpbuf, NULL, (__SEHandler::PF)EXCEPTION_EXECUTE_HANDLER);
#else
    __try
    {
#endif
        if ( ptdLeft == ptdRight )
        {
            
            bRet = sal_True;
        }

        
        else if ( ( NULL != ptdRight ) && ( NULL != ptdLeft ) )

        if ( ptdLeft->tdSize == ptdRight->tdSize )

        if ( memcmp( ptdLeft, ptdRight, ptdLeft->tdSize ) == 0 )
            bRet = sal_True;
    }
#if defined ( __MINGW32__ ) && !defined ( _WIN64 )
    else
#else
    __except( EXCEPTION_EXECUTE_HANDLER )
#endif
    {
        OSL_FAIL( "Error CompareTargetDevice" );
        bRet = sal_False;
    }
#if defined ( __MINGW32__ ) && !defined ( _WIN64 )
    han.Reset();
#endif

    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
