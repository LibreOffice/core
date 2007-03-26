/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: jpeg.h,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: vg $ $Date: 2007-03-26 14:46:20 $
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

#ifndef _JPEG_H
#define _JPEG_H

#if defined( ICC )
#include <stdio.h>
#endif

#if defined (UNX) || defined(__MINGW32__)
#include <sys/types.h>
#endif

#ifdef MAC
#include <stdlib.h>
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

typedef struct my_error_mgr*    my_error_ptr;
typedef unsigned char           BYTE;
#ifdef WIN
typedef unsigned char _huge*    HPBYTE;
#else
typedef unsigned char*          HPBYTE;
#endif

void*   JPEGMalloc( size_t size );
void    JPEGFree( void *ptr );
long    JPEGCallback( void* pCallbackData, long nPercent );

long    WriteJPEG( void* pJPEGWriter, void* pOStm, long nWidth, long nHeight,
                   long nQualityPercent, void* pCallbackData );
void*   GetScanline( void* pJPEGWriter, long nY );

void    ReadJPEG( void* pJPEGReader, void* pIStm, long* pLines );
void*   CreateBitmap( void* pJPEGReader, void* pJPEGCreateBitmapParam );

/* TODO: when incompatible changes are possible again
   the preview size hint should be redone */
void    SetJpegPreviewSizeHint( int nWidth, int nHeight );

#endif
