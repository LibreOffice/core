/*************************************************************************
 *
 *  $RCSfile: jpeg.h,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: sj $ $Date: 2002-08-19 15:00:53 $
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
 *  WITHOUT WARRUNTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRUNTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _JPEG_H
#define _JPEG_H

#if defined( ICC )
#include <stdio.h>
#endif

#if defined (UNX)
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

#endif
