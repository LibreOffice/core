/*************************************************************************
 *
 *  $RCSfile: TxtCnvtHlp.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: tra $ $Date: 2001-03-20 09:26:01 $
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

int CalcBuffSizeForTextConversion( LPCSTR lpMultiByteString, int nLen = -1 )
{
    return ( MultiByteToWideChar( CP_ACP,
                                0,
                                lpMultiByteString,
                                nLen,
                                NULL,
                                0 ) * sizeof( sal_Unicode ) );
}

//------------------------------------------------------------------
// assuming a '\0' terminated string if no length specified
//------------------------------------------------------------------

int CalcBuffSizeForTextConversion( LPCWSTR lpWideCharString, int nLen = -1 )
{
    return WideCharToMultiByte( CP_ACP,
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
    int reqSize = CalcBuffSizeForTextConversion( lpMultiByteString, lenStr );

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
    int reqSize = CalcBuffSizeForTextConversion( lpWideCharString, lenStr );

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

