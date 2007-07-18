/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fltdefs.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2007-07-18 08:51:26 $
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

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif
#include <vcl/bitmap.hxx>

#include <string.h>

#ifndef _FLTDEFS_HXX
#define _FLTDEFS_HXX


#if defined ( WIN ) || defined ( WNT )

#define RGBQUAD RGBQUADWIN

typedef struct RGBQUAD
{
    BYTE    rgbBlue;
    BYTE    rgbGreen;
    BYTE    rgbRed;
    BYTE    rgbReserved;

            RGBQUAD( const BYTE cRed = 0, const BYTE cGreen = 0, const BYTE cBlue = 0 ) :
                rgbBlue     ( cBlue ),
                rgbGreen    ( cGreen ),
                rgbRed      ( cRed ),
                rgbReserved ( 0 ) {};
} RGBQUAD;


#ifdef WIN
typedef BYTE huge* PDIBBYTE;
#define MEMCPY hmemcpy
#define GLOBALALLOC(nSize) ((PDIBBYTE)GlobalLock(GlobalAlloc(GHND,(nSize))))
#define GLOBALHANDLE(pPointer) ((HGLOBAL)GlobalHandle((*((size_t*)&(pPointer)+1))))
#define GLOBALFREE(pPointer) (GlobalUnlock(GLOBALHANDLE((pPointer))))
#define MEMSET( pDst, cByte, nCount )   \
{                                       \
    PDIBBYTE pTmp = (PDIBBYTE) pDst;    \
    for ( ULONG i = 0; i < nCount; i++ )\
        *pTmp++ = cByte;                \
}

#else

typedef BYTE* PDIBBYTE;
#define MEMCPY memcpy
#define MEMSET memset
#define GLOBALALLOC(nSize) ((PDIBBYTE)GlobalAlloc(GMEM_FIXED,(nSize)))
#define GLOBALFREE(pPointer) (GlobalFree((HGLOBAL)pPointer))
#define GLOBALHANDLE(pPointer) ((HGLOBAL)(pPointer))

#endif
#else

typedef BYTE* PDIBBYTE;
#define MEMCPY memcpy
#define MEMSET memset
#define GLOBALALLOC(nSize) ((PDIBBYTE)new BYTE[(nSize)])
#define GLOBALFREE(pPointer) (delete[] (pPointer))

#endif


#if defined ( OS2 ) || defined ( UNX )
void ReadBitmap( SvStream& rIStream, Bitmap& rBmp, USHORT nDefaultHeight = 0, ULONG nOffBits = 0 );
void ReplaceInfoHeader( SvStream& rStm, BYTE* pBuffer );

#ifdef OS2
#define RGBQUAD             RGBQUADOS2
#define BITMAPFILEHEADER    BITMAPFILEHEADEROS2
#define PBITMAPFILEHEADER   PBITMAPFILEHEADEROS2
#define BITMAPINFOHEADER    BITMAPINFOHEADEROS2
#define PBITMAPINFOHEADER   PBITMAPINFOHEADEROS2
#define BITMAPINFO          BITMAPINFOOS2
#define PBITMAPINFO         PBITMAPINFOOS2
#endif

typedef struct RGBQUAD
{
    BYTE    rgbBlue;
    BYTE    rgbGreen;
    BYTE    rgbRed;
    BYTE    rgbReserved;

            RGBQUAD( const BYTE cRed = 0, const BYTE cGreen = 0, const BYTE cBlue = 0 ) :
                rgbBlue     ( cBlue ),
                rgbGreen    ( cGreen ),
                rgbRed      ( cRed ),
                rgbReserved ( 0 ) {};
} RGBQUAD;

typedef struct BITMAPFILEHEADER
{
    UINT16  bfType;
    UINT32  bfSize;
    UINT16  bfReserved1;
    UINT16  bfReserved2;
    UINT32  bfOffBits;
} BITMAPFILEHEADER;
typedef BITMAPFILEHEADER* PBITMAPFILEHEADER;

typedef struct BITMAPINFOHEADER
{
    UINT32  biSize;
    UINT32  biWidth;
    UINT32  biHeight;
    UINT16  biPlanes;
    UINT16  biBitCount;
    UINT32  biCompression;
    UINT32  biSizeImage;
    UINT32  biXPelsPerMeter;
    UINT32  biYPelsPerMeter;
    UINT32  biClrUsed;
    UINT32  biClrImportant;
} BITMAPINFOHEADER;
typedef BITMAPINFOHEADER* PBITMAPINFOHEADER;

typedef struct BITMAPINFO
{
    BITMAPINFOHEADER    bmiHeader;
    RGBQUAD             bmiColors[1];
} BITMAPINFO;
typedef BITMAPINFO* PBITMAPINFO;

#endif
#endif
