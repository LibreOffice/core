/*************************************************************************
 *
 *  $RCSfile: ImplHelper.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: tra $ $Date: 2001-03-02 12:44:41 $
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

#ifndef _IMPLHELPER_HXX_
#include "ImplHelper.hxx"
#endif

#ifndef _RTL_TENCINFO_H
#include <rtl/tencinfo.h>
#endif

#ifndef _RTL_MEMORY_H_
#include <rtl/memory.h>
#endif

#include <windows.h>
#include <systools/win32/user9x.h>

//------------------------------------------------------------------------
// defines
//------------------------------------------------------------------------

#define FORMATETC_EXACT_MATCH    1
#define FORMATETC_PARTIAL_MATCH -1
#define FORMATETC_NO_MATCH       0

//------------------------------------------------------------------------
// namespace directives
//------------------------------------------------------------------------

using ::com::sun::star::datatransfer::DataFlavor;
using ::com::sun::star::uno::Sequence;
using ::rtl::OUString;
using ::rtl::OString;

//------------------------------------------------------------------------
// converts a codepage into its string representation
//------------------------------------------------------------------------

OUString SAL_CALL CodePageToString( sal_Int32 codepage )
{
    OSL_ASSERT( IsValidCodePage( codepage ) );

    sal_Unicode cpStr[6];
    _itow( codepage, cpStr, 10 );
    return OUString( cpStr, wcslen( cpStr ) );
}

//------------------------------------------------------------------------
// convert a mime charset into a windows codepage
//------------------------------------------------------------------------

sal_Int32 SAL_CALL getWinCodePageFromMimeCharset( const OUString& charset )
{
    OString osCharset( charset.getStr( ), charset.getLength( ), CP_ACP );

    rtl_TextEncoding textEnc = rtl_getTextEncodingFromMimeCharset( osCharset.getStr( ) );
    return rtl_getBestPCCodePageFromTextEncoding( textEnc );
}

//------------------------------------------------------------------------
// IsOEMCP
//------------------------------------------------------------------------

sal_Bool SAL_CALL IsOEMCP( UINT codepage )
{
    UINT arrOEMCP[] = { 437, 708, 709,
                        710, 720, 737,
                        775, 850, 852,
                        855, 857, 860,
                        861, 862, 863,
                        864, 865, 866,
                        869, 874, 932,
                        936, 949, 950,
                        1361 };

    for ( sal_Int8 i = 0; i < ( sizeof( arrOEMCP )/sizeof( UINT ) ); ++i )
        if ( arrOEMCP[i] == codepage )
            return sal_True;

    return sal_False;
}

//------------------------------------------------------------------------
// a '==' operator for DataFlavors
// we compare only MimeType and DataType
//------------------------------------------------------------------------

sal_Bool SAL_CALL operator==( const DataFlavor& lhs, const DataFlavor& rhs )
{
#pragma message( "******************************************" )
#pragma message( "**************** fix this ****************" )
#pragma message( "******************************************" )

    return ( ( lhs.MimeType == rhs.MimeType ) &&
             ( lhs.DataType == rhs.DataType ) );
}


//-------------------------------------------------------------------------
// OleStdDeleteTargetDevice()
//
// Purpose:
//
// Parameters:
//
// Return Value:
//    SCODE  -  S_OK if successful
//-------------------------------------------------------------------------

void SAL_CALL DeleteTargetDevice( DVTARGETDEVICE* ptd )
{
    __try
    {
        CoTaskMemFree( ptd );
    }
    __except( EXCEPTION_EXECUTE_HANDLER )
    {
        OSL_ENSURE( sal_False, "Error DeleteTargetDevice" );
    }
}



//-------------------------------------------------------------------------
// OleStdCopyTargetDevice()
//
// Purpose:
//  duplicate a TARGETDEVICE struct. this function allocates memory for
//  the copy. the caller MUST free the allocated copy when done with it
//  using the standard allocator returned from CoGetMalloc.
//  (OleStdFree can be used to free the copy).
//
// Parameters:
//  ptdSrc      pointer to source TARGETDEVICE
//
// Return Value:
//    pointer to allocated copy of ptdSrc
//    if ptdSrc==NULL then retuns NULL is returned.
//    if ptdSrc!=NULL and memory allocation fails, then NULL is returned
//-------------------------------------------------------------------------

DVTARGETDEVICE* SAL_CALL CopyTargetDevice( DVTARGETDEVICE* ptdSrc )
{
    DVTARGETDEVICE* ptdDest;

    __try
    {
        ptdDest = static_cast< DVTARGETDEVICE* >( CoTaskMemAlloc( ptdSrc->tdSize ) );
        rtl_copyMemory( ptdDest, ptdSrc, static_cast< size_t >( ptdSrc->tdSize ) );
    }
    __except( EXCEPTION_EXECUTE_HANDLER )
    {
        ptdDest = NULL;
    }

    return ptdDest;
}


//-------------------------------------------------------------------------
// OleStdCopyFormatEtc()
//
// Purpose:
//  Copies the contents of a FORMATETC structure. this function takes
//  special care to copy correctly copying the pointer to the TARGETDEVICE
//  contained within the source FORMATETC structure.
//  if the source FORMATETC has a non-NULL TARGETDEVICE, then a copy
//  of the TARGETDEVICE will be allocated for the destination of the
//  FORMATETC (petcDest).
//
//  NOTE: the caller MUST free the allocated copy of the TARGETDEVICE
//  within the destination FORMATETC when done with it
//  using the standard allocator returned from CoGetMalloc.
//  (OleStdFree can be used to free the copy).
//
// Parameters:
//  petcDest      pointer to destination FORMATETC
//  petcSrc       pointer to source FORMATETC
//
// Return Value:
//    returns TRUE is copy is successful; retuns FALSE if not successful
//-------------------------------------------------------------------------

sal_Bool SAL_CALL CopyFormatEtc( LPFORMATETC petcDest, LPFORMATETC petcSrc )
{
    sal_Bool bRet = sal_False;

    __try
    {
        petcDest->cfFormat = petcSrc->cfFormat;
        petcDest->ptd      = CopyTargetDevice(petcSrc->ptd);
        petcDest->dwAspect = petcSrc->dwAspect;
        petcDest->lindex   = petcSrc->lindex;
        petcDest->tymed    = petcSrc->tymed;

        bRet = sal_True;
    }
    __except( EXCEPTION_EXECUTE_HANDLER )
    {
        OSL_ENSURE( sal_False, "Error CopyFormatEtc" );
    }

    return bRet;
}

//-------------------------------------------------------------------------
// returns:
//  1 for exact match,
//  0 for no match,
// -1 for partial match (which is defined to mean the left is a subset
//    of the right: fewer aspects, null target device, fewer medium).
//-------------------------------------------------------------------------

sal_Int32 SAL_CALL CompareFormatEtc( const FORMATETC* pFetcLhs, const FORMATETC* pFetcRhs )
{
    sal_Int32 nMatch = FORMATETC_NO_MATCH;

    __try
    {
        if ( ( pFetcLhs->cfFormat != pFetcRhs->cfFormat ) ||
             ( pFetcLhs->lindex    != pFetcRhs->lindex ) ||
             !CompareTargetDevice( pFetcLhs->ptd, pFetcRhs->ptd ) )
        {
            nMatch = FORMATETC_NO_MATCH;
            __leave;
        }

        if ( pFetcLhs->dwAspect == pFetcRhs->dwAspect )
            // same aspects; equal
            ;
        else if ( ( pFetcLhs->dwAspect & ~pFetcRhs->dwAspect ) != 0 )
        {
            // left not subset of aspects of right; not equal
            nMatch = FORMATETC_NO_MATCH;
            __leave;
        }
        else
            // left subset of right
            nMatch = FORMATETC_PARTIAL_MATCH;

        if ( pFetcLhs->tymed == pFetcRhs->tymed )
            // same medium flags; equal
            ;
        else if ( ( pFetcLhs->tymed & ~pFetcRhs->tymed ) != 0 )
        {
            // left not subset of medium flags of right; not equal
            nMatch = FORMATETC_NO_MATCH;
            __leave;
        }
        else
            // left subset of right
            nMatch = FORMATETC_PARTIAL_MATCH;
    }
    __except( EXCEPTION_EXECUTE_HANDLER )
    {
        OSL_ENSURE( sal_False, "Error CompareFormatEtc" );
        nMatch = FORMATETC_NO_MATCH;
    }

    return nMatch;
}


//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------

sal_Bool SAL_CALL CompareTargetDevice( DVTARGETDEVICE* ptdLeft, DVTARGETDEVICE* ptdRight )
{
    sal_Bool bRet = sal_False;

    __try
    {
        if ( ( ptdRight == NULL ) || ( ptdLeft == NULL ) )
        {
            bRet = sal_False;
            __leave;
        }

        if ( ptdLeft == ptdRight )
        {
            // same address of td; must be same (handles NULL case)
            bRet = sal_True;
            __leave;
        }


        if ( ptdLeft->tdSize != ptdRight->tdSize )
        {
            // different sizes, not equal
            bRet = sal_False;
            __leave;
        }

        if ( rtl_compareMemory( ptdLeft, ptdRight, ptdLeft->tdSize ) == 0 )
            bRet = sal_True;
    }
    __except( EXCEPTION_EXECUTE_HANDLER )
    {
        OSL_ENSURE( sal_False, "Error CompareTargetDevice" );
        bRet = sal_False;
    }

    return bRet;
}