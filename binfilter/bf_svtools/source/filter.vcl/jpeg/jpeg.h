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

#ifndef _JPEG_H
#define _JPEG_H

#if defined( ICC )
#include <stdio.h>
#endif

#if defined (UNX) || defined(__MINGW32__)
#include <sys/types.h>
#endif

struct JPEGCreateBitmapParam
{
    unsigned long nWidth;
    unsigned long nHeight;
    unsigned long density_unit;
    unsigned long X_density;
    unsigned long Y_density;
    long     bGray;
    
    long     nAlignedWidth;  // these members will be filled by the
    long     bTopDown;      // CreateBitmap method in svtools
};

typedef struct my_error_mgr*	my_error_ptr;
typedef unsigned char			BYTE;
#ifdef WIN
typedef unsigned char _huge*	HPBYTE;
#else
typedef unsigned char*			HPBYTE;
#endif

void*	JPEGMalloc( size_t size );
void	JPEGFree( void *ptr );
long	JPEGCallback( void* pCallbackData, long nPercent );

long	WriteJPEG( void* pJPEGWriter, void* pOStm, long nWidth, long nHeight,
                   long nQualityPercent, void* pCallbackData );
void*	GetScanline( void* pJPEGWriter, long nY );

void	ReadJPEG( void* pJPEGReader, void* pIStm, long* pLines );
void*	CreateBitmap( void* pJPEGReader, void* pJPEGCreateBitmapParam );

/* TODO: when incompatible changes are possible again
   the preview size hint should be redone */
void    SetJpegPreviewSizeHint( int nWidth, int nHeight );

#endif
