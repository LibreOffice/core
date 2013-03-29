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

typedef struct my_error_mgr*    my_error_ptr;
typedef unsigned char*          HPBYTE;

void*   JPEGMalloc( size_t size );
void    JPEGFree( void *ptr );
long    JPEGCallback( void* pCallbackData, long nPercent );

long    WriteJPEG( void* pJPEGWriter, void* pOStm, long nWidth, long nHeight, long bGreyScale,
                   long nQualityPercent, long aChromaSubsampling, void* pCallbackData );
void*   GetScanline( void* pJPEGWriter, long nY );

void    ReadJPEG( void* pJPEGReader, void* pIStm, long* pLines );
void*   CreateBitmapFromJPEGReader( void* pJPEGReader, void* pJPEGCreateBitmapParam );

/* TODO: when incompatible changes are possible again
   the preview size hint should be redone */
void    SetJpegPreviewSizeHint( int nWidth, int nHeight );

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
