/*************************************************************************
 *
 *  $RCSfile: FmtFilter.cxx,v $
 *
 *  $Revision: 1.5 $
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
using rtl::OString;

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
    OSL_ASSERT( aMetaFilePict.getLength( ) == sizeof( METAFILEPICT ) );

    Sequence< sal_Int8 > mfpictStream;
    METAFILEPICT* pMFPict = reinterpret_cast< METAFILEPICT* >( aMetaFilePict.getArray( ) );
    HMETAFILE hMf = pMFPict->hMF;
    sal_uInt32 nCount = GetMetaFileBitsEx( hMf, 0, NULL );

    if ( nCount > 0 )
    {
        mfpictStream.realloc( nCount + sizeof( METAFILEHEADER ) );

        METAFILEHEADER* pMFHeader = reinterpret_cast< METAFILEHEADER* >( mfpictStream.getArray( ) );
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
        OSL_ASSERT( nCount > 0 );
    }

    return mfpictStream;
}

//------------------------------------------------------------------------
// convert a openoffice metafile picture to a windows metafile picture
//------------------------------------------------------------------------

HMETAFILEPICT SAL_CALL OOMFPictToWinMFPict( Sequence< sal_Int8 >& aOOMetaFilePict )
{
    OSL_ASSERT( aOOMetaFilePict.getLength() > 22 );

    HMETAFILEPICT   hPict = NULL;
    HMETAFILE       hMtf = SetMetaFileBitsEx( aOOMetaFilePict.getLength() - 22, (sal_uChar*) aOOMetaFilePict.getConstArray() + 22 );

    if( hMtf )
    {
        METAFILEPICT* pPict = (METAFILEPICT*) GlobalLock( hPict = GlobalAlloc( GHND, sizeof( METAFILEPICT ) ) );

        pPict->mm = 8;
        pPict->xExt = 0;
        pPict->yExt = 0;
        pPict->hMF = hMtf;

        GlobalUnlock( hPict );
    }

    return hPict;
}

//------------------------------------------------------------------------
// convert a windows device independent bitmap into a openoffice bitmap
//------------------------------------------------------------------------

Sequence< sal_Int8 > SAL_CALL WinDIBToOOBMP( const Sequence< sal_Int8 >& aWinDIB )
{
    OSL_ASSERT( aWinDIB.getLength( ) > sizeof( BITMAPINFOHEADER ) );

    Sequence< sal_Int8 > ooBmpStream;

    ooBmpStream.realloc( aWinDIB.getLength( ) + sizeof(BITMAPFILEHEADER) );

    const BITMAPINFOHEADER  *pBmpInfoHdr = (const BITMAPINFOHEADER*)aWinDIB.getConstArray();
    BITMAPFILEHEADER        *pBmpFileHdr = reinterpret_cast< BITMAPFILEHEADER* >( ooBmpStream.getArray() );
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
    OSL_ASSERT( aOOBmp.getLength( ) >
                ( sizeof( BITMAPFILEHEADER ) + sizeof( BITMAPINFOHEADER ) ) );

    Sequence< sal_Int8 > winDIBStream( aOOBmp.getLength( ) - sizeof( BITMAPFILEHEADER ) );

    rtl_copyMemory( winDIBStream.getArray( ),
                    aOOBmp.getArray( )  + sizeof( BITMAPFILEHEADER ),
                    aOOBmp.getLength( ) - sizeof( BITMAPFILEHEADER ) );

    return winDIBStream;
}

//------------------------------------------------------------------------------
// converts the openoffice text/html clipboard format to the HTML Format
// well known under MS Windows
// the MS HTML Format has a header before the real html data
//
// Version:1.0      Version number of the clipboard. Staring is 0.9
// StartHTML:       Byte count from the beginning of the clipboard to the start
//                  of the context, or -1 if no context
// EndHTML:         Byte count from the beginning of the clipboard to the end
//                  of the context, or -1 if no context
// StartFragment:   Byte count from the beginning of the clipboard to the
//                  start of the fragment
// EndFragment:     Byte count from the beginning of the clipboard to the
//                  end of the fragment
// StartSelection:  Byte count from the beginning of the clipboard to the
//                  start of the selection
// EndSelection:    Byte count from the beginning of the clipboard to the
//                  end of the selection
//
// StartSelection and EndSelection are optional
// The fragment should be preceded and followed by the HTML comments
// <!--StartFragment--> and <!--EndFragment--> (no space between !-- and the
// text
//------------------------------------------------------------------------------

Sequence< sal_Int8 > SAL_CALL TextHtmlToHTMLFormat( Sequence< sal_Int8 >& aTextHtml )
{
    OSL_ASSERT( aTextHtml.getLength( ) > 0 );

    // check parameter
    if ( !(aTextHtml.getLength( ) > 0) )
        return Sequence< sal_Int8 >( );

    // we create a buffer with the approximated size of
    // the HTML Format header
    char aHTMLFmtHdr[120];

    rtl_zeroMemory( aHTMLFmtHdr, sizeof( aHTMLFmtHdr ) );

    // fill the buffer with dummy values to calc the
    // exact length
    wsprintf(
        aHTMLFmtHdr,
        "Version:1.0\nStartHTML:%010d\nEndHTML:%010d\nStartFragment:%010d\nEndFragment:%010d\n", 0, 0, 0, 0 );

    sal_uInt32 lHTMLFmtHdr = rtl_str_getLength( aHTMLFmtHdr );

    OString startHtmlTag( "<HTML>" );
    OString endHtmlTag(   "</HTML>" );
    OString startBodyTag( "<BODY>" );
    OString endBodyTag(   "</BODY" );

    OString textHtml(
        reinterpret_cast< const sal_Char* >( aTextHtml.getConstArray( ) ),
        aTextHtml.getLength( ) );

    sal_Int32 nStartHtml  = textHtml.search( startHtmlTag );
    sal_Int32 nEndHtml    = textHtml.search( endHtmlTag );
    sal_Int32 nStartFrgmt = textHtml.search( startBodyTag );
    sal_Int32 nEndFrgmt   = textHtml.search( endBodyTag );

    Sequence< sal_Int8 > aHTMLFmtSequence;

    if ( (nStartHtml > -1) && (nEndHtml > -1) && (nStartFrgmt > -1) && (nEndFrgmt > -1) )
    {
        nStartHtml  = nStartHtml  + lHTMLFmtHdr - 1; // we start one before <HTML> Word 2000 does also so
        nEndHtml    = nEndHtml    + lHTMLFmtHdr + endHtmlTag.getLength( ) + 1; // our SOffice 5.2 wants 2 behind </HTML>?
        nStartFrgmt = nStartFrgmt + startBodyTag.getLength( ) + lHTMLFmtHdr; // after the <BODY> tag
        nEndFrgmt   = nEndFrgmt   + lHTMLFmtHdr;

        // fill the html header
        rtl_zeroMemory( aHTMLFmtHdr, sizeof( aHTMLFmtHdr ) );

        wsprintf(
            aHTMLFmtHdr,
            "Version:1.0\nStartHTML:%010d\nEndHTML:%010d\nStartFragment:%010d\nEndFragment:%010d\n",
            nStartHtml, nEndHtml, nStartFrgmt, nEndFrgmt );

        // we add space for a trailing \0
        aHTMLFmtSequence.realloc( lHTMLFmtHdr + aTextHtml.getLength( ) + 1 );
        rtl_zeroMemory( aHTMLFmtSequence.getArray( ), aHTMLFmtSequence.getLength( ) );

        // copy the HTML Format header
        rtl_copyMemory(
            static_cast< LPVOID >( aHTMLFmtSequence.getArray( ) ),
            static_cast< LPVOID >( aHTMLFmtHdr ), lHTMLFmtHdr );

        // concat the text/html
        rtl_copyMemory(
            static_cast< LPVOID >( aHTMLFmtSequence.getArray( ) + lHTMLFmtHdr ),
            static_cast< LPVOID >( aTextHtml.getArray( ) ),
            aTextHtml.getLength( ) );
    }

    return aHTMLFmtSequence;
}
