/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dtrans.hxx"

//------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------
#include <osl/diagnose.h>
#include "TxtCnvtHlp.hxx"
#include "DTransHelper.hxx"
#include "..\misc\ImplHelper.hxx"

using namespace ::com::sun::star::datatransfer;
using namespace ::com::sun::star::uno;

//------------------------------------------------------------------
// assuming a '\0' terminated string if no length specified
//------------------------------------------------------------------

int CalcBuffSizeForTextConversion( UINT code_page, LPCSTR lpMultiByteString, int nLen = -1 )
{
    return ( MultiByteToWideChar( code_page,
                                0,
                                lpMultiByteString,
                                nLen,
                                NULL,
                                0 ) * sizeof( sal_Unicode ) );
}

//------------------------------------------------------------------
// assuming a '\0' terminated string if no length specified
//------------------------------------------------------------------

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

//------------------------------------------------------------------
// converts text in one code page into unicode text
// automatically calculates the necessary buffer size and allocates
// the buffer
//------------------------------------------------------------------

int MultiByteToWideCharEx( UINT cp_src,
                           LPCSTR lpMultiByteString,
                           sal_uInt32 lenStr,
                           CStgTransferHelper& refDTransHelper,
                           BOOL bEnsureTrailingZero )
{
    OSL_ASSERT( IsValidCodePage( cp_src ) );
    OSL_ASSERT( NULL != lpMultiByteString );

    // calculate the required buff size
    int reqSize = CalcBuffSizeForTextConversion( cp_src, lpMultiByteString, lenStr );

    if ( bEnsureTrailingZero )
        reqSize += sizeof( sal_Unicode );

    // initialize the data-transfer helper
    refDTransHelper.init( reqSize );

    // setup a global memory pointer
    CRawHGlobalPtr ptrHGlob( refDTransHelper );

    // do the converssion an return
    return MultiByteToWideChar( cp_src,
                                0,
                                lpMultiByteString,
                                lenStr,
                                static_cast< LPWSTR >( ptrHGlob.GetMemPtr( ) ),
                                ptrHGlob.MemSize( ) );
}

//------------------------------------------------------------------
// converts unicode text into text of the specified code page
// automatically calculates the necessary buffer size and allocates
// the buffer
//------------------------------------------------------------------

int WideCharToMultiByteEx( UINT cp_dest,
                           LPCWSTR lpWideCharString,
                           sal_uInt32 lenStr,
                           CStgTransferHelper& refDTransHelper,
                           BOOL bEnsureTrailingZero )
{
    OSL_ASSERT( IsValidCodePage( cp_dest ) );
    OSL_ASSERT( NULL != lpWideCharString );

    // calculate the required buff size
    int reqSize = CalcBuffSizeForTextConversion( cp_dest, lpWideCharString, lenStr );

    if ( bEnsureTrailingZero )
        reqSize += sizeof( sal_Int8 );

    // initialize the data-transfer helper
    refDTransHelper.init( reqSize );

    // setup a global memory pointer
    CRawHGlobalPtr ptrHGlob( refDTransHelper );

    // do the converssion an return
    return WideCharToMultiByte( cp_dest,
                                0,
                                lpWideCharString,
                                lenStr,
                                static_cast< LPSTR >( ptrHGlob.GetMemPtr( ) ),
                                ptrHGlob.MemSize( ),
                                NULL,
                                NULL );
}

