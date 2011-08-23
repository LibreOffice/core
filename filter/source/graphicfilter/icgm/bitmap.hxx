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

#ifndef CGM_BITMAP_HXX_
#define CGM_BITMAP_HXX_

#include "cgm.hxx"
#include <vcl/bmpacc.hxx>

class CGM;

class CGMBitmapDescriptor
{
    public:
        sal_uInt8*				mpBuf;
        Bitmap*					mpBitmap;
        BitmapWriteAccess*		mpAcc;
        sal_Bool				mbStatus;
        sal_Bool				mbVMirror;
        sal_Bool				mbHMirror;
        sal_uInt32				mnDstBitsPerPixel;
        sal_uInt32				mnScanSize;			// bytes per line
        FloatPoint				mnP, mnQ, mnR;

        FloatPoint				mnOrigin;
        double					mndx, mndy;
        double					mnOrientation;

        sal_uInt32				mnX, mnY;
        long					mnLocalColorPrecision;
        sal_uInt32				mnCompressionMode;
                                CGMBitmapDescriptor() :
                                mpBuf		( NULL ),
                                mpBitmap	( NULL ),
                                mpAcc		( NULL ),
                                mbStatus	( sal_False ),
                                mbVMirror	( sal_False ),
                                mbHMirror	( sal_False ) { };
                                ~CGMBitmapDescriptor()
                                {
                                    if ( mpAcc )
                                        mpBitmap->ReleaseAccess( mpAcc );
                                    if ( mpBitmap )
                                        delete mpBitmap;
                                };
};

class CGMBitmap
{
        CGM*					mpCGM;
        CGMBitmapDescriptor*	pCGMBitmapDescriptor;
        sal_Bool				ImplGetDimensions( CGMBitmapDescriptor& );
        void					ImplSetCurrentPalette( CGMBitmapDescriptor& );
        void					ImplGetBitmap( CGMBitmapDescriptor& );
        void					ImplInsert( CGMBitmapDescriptor& rSource, CGMBitmapDescriptor& rDest );
    public:
                                CGMBitmap( CGM& rCGM );
                                ~CGMBitmap();
        CGMBitmapDescriptor*	GetBitmap();
        CGMBitmap*				GetNext();
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
