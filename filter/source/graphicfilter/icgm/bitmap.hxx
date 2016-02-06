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

#ifndef INCLUDED_FILTER_SOURCE_GRAPHICFILTER_ICGM_BITMAP_HXX
#define INCLUDED_FILTER_SOURCE_GRAPHICFILTER_ICGM_BITMAP_HXX

#include "cgm.hxx"
#include <vcl/bitmapaccess.hxx>

class CGM;

class CGMBitmapDescriptor
{
    public:
        sal_uInt8*              mpBuf;
        Bitmap*                 mpBitmap;
        BitmapWriteAccess*      mpAcc;
        bool                mbStatus;
        bool                mbVMirror;
        bool                mbHMirror;
        sal_uInt32              mnDstBitsPerPixel;
        sal_uInt32              mnScanSize;         // bytes per line
        FloatPoint              mnP, mnQ, mnR;

        FloatPoint              mnOrigin;
        double                  mndx, mndy;
        double                  mnOrientation;

        sal_uInt32              mnX, mnY;
        long                    mnLocalColorPrecision;
        sal_uInt32              mnCompressionMode;

        CGMBitmapDescriptor()
            : mpBuf(nullptr)
            , mpBitmap(nullptr)
            , mpAcc(nullptr)
            , mbStatus(false)
            , mbVMirror(false)
            , mbHMirror(false)
            , mnDstBitsPerPixel(0)
            , mnScanSize(0)
            , mndx(0.0)
            , mndy(0.0)
            , mnOrientation(0.0)
            , mnX(0)
            , mnY(0)
            , mnLocalColorPrecision(0)
            , mnCompressionMode(0)
            { };
        ~CGMBitmapDescriptor()
        {
            if ( mpAcc )
                ::Bitmap::ReleaseAccess( mpAcc );
            delete mpBitmap;
        };
};

class CGMBitmap
{
    CGM*                    mpCGM;
    CGMBitmapDescriptor*    pCGMBitmapDescriptor;
    bool                ImplGetDimensions( CGMBitmapDescriptor& );
    void                    ImplSetCurrentPalette( CGMBitmapDescriptor& );
    void                    ImplGetBitmap( CGMBitmapDescriptor& );
    void                    ImplInsert( CGMBitmapDescriptor& rSource, CGMBitmapDescriptor& rDest );
public:
    explicit CGMBitmap( CGM& rCGM );
    ~CGMBitmap();
    CGMBitmapDescriptor*    GetBitmap() { return pCGMBitmapDescriptor;}
    CGMBitmap*              GetNext();
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
