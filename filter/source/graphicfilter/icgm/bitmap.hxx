/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef CGM_BITMAP_HXX_
#define CGM_BITMAP_HXX_

#include "cgm.hxx"
#include <vcl/bmpacc.hxx>

class CGM;

class CGMBitmapDescriptor
{
    public:
        sal_uInt8*              mpBuf;
        Bitmap*                 mpBitmap;
        BitmapWriteAccess*      mpAcc;
        sal_Bool                mbStatus;
        sal_Bool                mbVMirror;
        sal_Bool                mbHMirror;
        sal_uInt32              mnDstBitsPerPixel;
        sal_uInt32              mnScanSize;         // bytes per line
        FloatPoint              mnP, mnQ, mnR;

        FloatPoint              mnOrigin;
        double                  mndx, mndy;
        double                  mnOrientation;

        sal_uInt32              mnX, mnY;
        long                    mnLocalColorPrecision;
        sal_uInt32              mnCompressionMode;
                                CGMBitmapDescriptor() :
                                mpBuf       ( NULL ),
                                mpBitmap    ( NULL ),
                                mpAcc       ( NULL ),
                                mbStatus    ( sal_False ),
                                mbVMirror   ( sal_False ),
                                mbHMirror   ( sal_False ) { };
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
        CGM*                    mpCGM;
        CGMBitmapDescriptor*    pCGMBitmapDescriptor;
        sal_Bool                ImplGetDimensions( CGMBitmapDescriptor& );
        void                    ImplSetCurrentPalette( CGMBitmapDescriptor& );
        void                    ImplGetBitmap( CGMBitmapDescriptor& );
        void                    ImplInsert( CGMBitmapDescriptor& rSource, CGMBitmapDescriptor& rDest );
    public:
                                CGMBitmap( CGM& rCGM );
                                ~CGMBitmap();
        CGMBitmapDescriptor*    GetBitmap();
        CGMBitmap*              GetNext();
};
#endif

