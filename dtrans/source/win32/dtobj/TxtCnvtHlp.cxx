/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: TxtCnvtHlp.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 17:01:38 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dtrans.hxx"

//------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

#ifndef _TXTCNVTHLP_HXX_
#include "TxtCnvtHlp.hxx"
#endif

#ifndef _DTRANSHELPER_HXX_
#include "DTransHelper.hxx"
#endif

#ifndef _IMPLHELPER_HXX_
#include "..\misc\ImplHelper.hxx"
#endif

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

