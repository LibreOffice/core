/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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


#if defined ( UNX )
void ReadBitmap( SvStream& rIStream, Bitmap& rBmp, sal_uInt16 nDefaultHeight = 0, sal_uLong nOffBits = 0 );
void ReplaceInfoHeader( SvStream& rStm, sal_uInt8* pBuffer );

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
