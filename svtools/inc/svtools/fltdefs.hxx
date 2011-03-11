/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include <tools/solar.h>
#include <vcl/bitmap.hxx>

#include <string.h>

#ifndef _FLTDEFS_HXX
#define _FLTDEFS_HXX


#if defined ( WNT )

#define RGBQUAD RGBQUADWIN

typedef struct RGBQUAD
{
    sal_uInt8   rgbBlue;
    sal_uInt8   rgbGreen;
    sal_uInt8   rgbRed;
    sal_uInt8   rgbReserved;

            RGBQUAD( const sal_uInt8 cRed = 0, const sal_uInt8 cGreen = 0, const sal_uInt8 cBlue = 0 ) :
                rgbBlue     ( cBlue ),
                rgbGreen    ( cGreen ),
                rgbRed      ( cRed ),
                rgbReserved ( 0 ) {};
} RGBQUAD;


typedef sal_uInt8* PDIBBYTE;
#define MEMCPY memcpy
#define MEMSET memset
#define GLOBALALLOC(nSize) ((PDIBBYTE)GlobalAlloc(GMEM_FIXED,(nSize)))
#define GLOBALFREE(pPointer) (GlobalFree((HGLOBAL)pPointer))
#define GLOBALHANDLE(pPointer) ((HGLOBAL)(pPointer))

#else

typedef sal_uInt8* PDIBBYTE;
#define MEMCPY memcpy
#define MEMSET memset
#define GLOBALALLOC(nSize) ((PDIBBYTE)new sal_uInt8[(nSize)])
#define GLOBALFREE(pPointer) (delete[] (pPointer))

#endif


#if defined ( OS2 ) || defined ( UNX )
void ReadBitmap( SvStream& rIStream, Bitmap& rBmp, sal_uInt16 nDefaultHeight = 0, sal_uLong nOffBits = 0 );
void ReplaceInfoHeader( SvStream& rStm, sal_uInt8* pBuffer );

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
    sal_uInt8   rgbBlue;
    sal_uInt8   rgbGreen;
    sal_uInt8   rgbRed;
    sal_uInt8   rgbReserved;

            RGBQUAD( const sal_uInt8 cRed = 0, const sal_uInt8 cGreen = 0, const sal_uInt8 cBlue = 0 ) :
                rgbBlue     ( cBlue ),
                rgbGreen    ( cGreen ),
                rgbRed      ( cRed ),
                rgbReserved ( 0 ) {};
} RGBQUAD;

typedef struct BITMAPFILEHEADER
{
    sal_uInt16  bfType;
    sal_uInt32  bfSize;
    sal_uInt16  bfReserved1;
    sal_uInt16  bfReserved2;
    sal_uInt32  bfOffBits;
} BITMAPFILEHEADER;
typedef BITMAPFILEHEADER* PBITMAPFILEHEADER;

typedef struct BITMAPINFOHEADER
{
    sal_uInt32  biSize;
    sal_uInt32  biWidth;
    sal_uInt32  biHeight;
    sal_uInt16  biPlanes;
    sal_uInt16  biBitCount;
    sal_uInt32  biCompression;
    sal_uInt32  biSizeImage;
    sal_uInt32  biXPelsPerMeter;
    sal_uInt32  biYPelsPerMeter;
    sal_uInt32  biClrUsed;
    sal_uInt32  biClrImportant;
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
