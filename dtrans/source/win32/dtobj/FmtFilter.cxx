/*************************************************************************
 *
 *  $RCSfile: FmtFilter.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: tra $ $Date: 2001-02-27 07:53:46 $
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

#ifndef _FMTFILTER_HXX_
#include "FmtFilter.hxx"
#endif

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

//------------------------------------------------------------------------
// namespace directives
//------------------------------------------------------------------------

using namespace com::sun::star::uno;

//------------------------------------------------------------------------
// implementation
//------------------------------------------------------------------------

#pragma pack(2)
struct METAFILEHEADER
{
    DWORD       key;
    short       hmf;
    SMALL_RECT  bbox;
    WORD        inch;
    DWORD       reserved;
    WORD        checksum;
};
#pragma pack()

//------------------------------------------------------------------------
// convert a windows metafile picture to a openoffice metafile picture
//------------------------------------------------------------------------

Sequence< sal_Int8 > SAL_CALL WinMFPictToOOMFPict( Sequence< sal_Int8 >& aMetaFilePict )
{
    OSL_ASSERT( aMetaFilePict.getLength( ) );

    METAFILEPICT* pMFPict = reinterpret_cast< METAFILEPICT* >( aMetaFilePict.getArray( ) );

    OSL_ASSERT( pMFPict );
    OSL_ASSERT( !IsBadReadPtr( pMFPict, sizeof( METAFILEPICT ) ) );

    sal_uInt32 nCount = GetMetaFileBitsEx( pMFPict->hMF, 0, NULL );
    Sequence< sal_Int8 > mfpictStream( nCount + sizeof( METAFILEHEADER ) );

    METAFILEHEADER* pMFHeader = (METAFILEHEADER*)mfpictStream.getArray( );
    SMALL_RECT aRect = { 0,
                         0,
                         static_cast< short >( pMFPict->xExt ),
                         static_cast< short >( pMFPict->yExt ) };
    USHORT nInch;

    switch( pMFPict->mm )
    {
    case MM_TEXT:
        nInch = 72;
        break;

    case MM_LOMETRIC:
        nInch = 100;
        break;

    case MM_HIMETRIC:
        nInch = 1000;
        break;

    case MM_LOENGLISH:
        nInch = 254;
        break;

    case MM_HIENGLISH:
    case MM_ISOTROPIC:
    case MM_ANISOTROPIC:
        nInch = 2540;
        break;

    case MM_TWIPS:
        nInch = 1440;
        break;

    default:
        nInch = 576;
    }

    pMFHeader->key      = 0x9AC6CDD7L;
    pMFHeader->hmf      = 0;
    pMFHeader->bbox     = aRect;
    pMFHeader->inch     = nInch;
    pMFHeader->reserved = 0;
    pMFHeader->checksum = 0;

    char* pMFBuff = reinterpret_cast< char* >( mfpictStream.getArray( ) );

    nCount = GetMetaFileBitsEx( pMFPict->hMF, nCount, pMFBuff + sizeof( METAFILEHEADER ) );
    OSL_ASSERT( nCount );

    return mfpictStream;
}

//------------------------------------------------------------------------
// convert a openoffice metafile picture to a windows metafile picture
//------------------------------------------------------------------------

HMETAFILE SAL_CALL OOMFPictToWinMFPict( Sequence< sal_Int8 >& aOOMetaFilePict )
{
    OSL_ASSERT( aOOMetaFilePict.getLength( ) );

    return SetMetaFileBitsEx( aOOMetaFilePict.getLength( ) - sizeof( METAFILEHEADER ),
                              reinterpret_cast< BYTE* >( aOOMetaFilePict.getArray( ) ) + sizeof( METAFILEHEADER ) );
}

//------------------------------------------------------------------------
// convert a windows device independent bitmap into a openoffice bitmap
//------------------------------------------------------------------------

Sequence< sal_Int8 > SAL_CALL WinDIBToOOBMP( const Sequence< sal_Int8 >& aWinDIB )
{
    Sequence< sal_Int8 > ooBmpStream( aWinDIB.getLength( ) + sizeof(BITMAPFILEHEADER) );

    const BITMAPINFOHEADER  *pBmpInfoHdr = (const BITMAPINFOHEADER*)aWinDIB.getConstArray();
    BITMAPFILEHEADER        *pBmpFileHdr = (BITMAPFILEHEADER*)ooBmpStream.getArray();
    DWORD                   nOffset      = sizeof( BITMAPFILEHEADER ) + sizeof( BITMAPINFOHEADER );

    rtl_copyMemory( pBmpFileHdr + 1, pBmpInfoHdr, aWinDIB.getLength( ) );

    if( pBmpInfoHdr->biBitCount <= 8 )
        nOffset += ( pBmpInfoHdr->biClrUsed ? pBmpInfoHdr->biClrUsed : ( 1 << pBmpInfoHdr->biBitCount ) ) << 2;

    pBmpFileHdr->bfType      = 'MB';
    pBmpFileHdr->bfSize      = 0; // maybe: nMemSize + sizeof(BITMAPFILEHEADER)
    pBmpFileHdr->bfReserved1 = 0;
    pBmpFileHdr->bfReserved2 = 0;
    pBmpFileHdr->bfOffBits   = nOffset;

    return ooBmpStream;
}

//------------------------------------------------------------------------
// convert a openoffice bitmap into a windows device independent bitmap
//------------------------------------------------------------------------

Sequence< sal_Int8 > SAL_CALL OOBmpToWinDIB( Sequence< sal_Int8 >& aOOBmp )
{
    Sequence< sal_Int8 > winDIBStream( aOOBmp.getLength( ) - sizeof(BITMAPFILEHEADER) );

    rtl_copyMemory( winDIBStream.getArray( ),
                    aOOBmp.getArray( ) + sizeof(BITMAPFILEHEADER),
                    aOOBmp.getLength( ) - sizeof(BITMAPFILEHEADER) );

    return winDIBStream;
}