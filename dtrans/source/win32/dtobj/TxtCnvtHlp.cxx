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
#include "TxtCnvtHlp.hxx"
#include "DTransHelper.hxx"
#include "../misc/ImplHelper.hxx"

using namespace ::com::sun::star::datatransfer;
using namespace ::com::sun::star::uno;





int CalcBuffSizeForTextConversion( UINT code_page, LPCSTR lpMultiByteString, int nLen = -1 )
{
    return ( MultiByteToWideChar( code_page,
                                0,
                                lpMultiByteString,
                                nLen,
                                NULL,
                                0 ) * sizeof( sal_Unicode ) );
}





int CalcBuffSizeForTextConversion( UINT code_page, LPCWSTR lpWideCharString, int nLen = -1 )
{
    return WideCharToMultiByte( code_page,
                                0,
                                lpWideCharString,
                                nLen,
                                NULL,
                                0,
                                NULL,
                                NULL );
}







int MultiByteToWideCharEx( UINT cp_src,
                           LPCSTR lpMultiByteString,
                           sal_uInt32 lenStr,
                           CStgTransferHelper& refDTransHelper,
                           BOOL bEnsureTrailingZero )
{
    OSL_ASSERT( IsValidCodePage( cp_src ) );
    OSL_ASSERT( NULL != lpMultiByteString );

    
    int reqSize = CalcBuffSizeForTextConversion( cp_src, lpMultiByteString, lenStr );

    if ( bEnsureTrailingZero )
        reqSize += sizeof( sal_Unicode );

    
    refDTransHelper.init( reqSize );

    
    CRawHGlobalPtr ptrHGlob( refDTransHelper );

    
    return MultiByteToWideChar( cp_src,
                                0,
                                lpMultiByteString,
                                lenStr,
                                static_cast< LPWSTR >( ptrHGlob.GetMemPtr( ) ),
                                ptrHGlob.MemSize( ) );
}







int WideCharToMultiByteEx( UINT cp_dest,
                           LPCWSTR lpWideCharString,
                           sal_uInt32 lenStr,
                           CStgTransferHelper& refDTransHelper,
                           BOOL bEnsureTrailingZero )
{
    OSL_ASSERT( IsValidCodePage( cp_dest ) );
    OSL_ASSERT( NULL != lpWideCharString );

    
    int reqSize = CalcBuffSizeForTextConversion( cp_dest, lpWideCharString, lenStr );

    if ( bEnsureTrailingZero )
        reqSize += sizeof( sal_Int8 );

    
    refDTransHelper.init( reqSize );

    
    CRawHGlobalPtr ptrHGlob( refDTransHelper );

    
    return WideCharToMultiByte( cp_dest,
                                0,
                                lpWideCharString,
                                lenStr,
                                static_cast< LPSTR >( ptrHGlob.GetMemPtr( ) ),
                                ptrHGlob.MemSize( ),
                                NULL,
                                NULL );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
