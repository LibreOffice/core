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

#ifndef _XPMREAD_HXX
#define _XPMREAD_HXX

#ifndef _BITMAP_HXX
#include <vcl/bitmap.hxx>
#endif

#ifdef _XPMPRIVATE

#define XPMTEMPBUFSIZE      0x00008000
#define XPMSTRINGBUF        0x00008000

#define XPMIDENTIFIER       0x00000001          // mnIdentifier includes on of the six phases
#define XPMDEFINITION       0x00000002          // the XPM format consists of
#define XPMVALUES           0x00000003
#define XPMCOLORS           0x00000004
#define XPMPIXELS           0x00000005
#define XPMEXTENSIONS       0x00000006
#define XPMENDEXT           0x00000007


#define XPMREMARK           0x00000001          // defines used by mnStatus
#define XPMDOUBLE           0x00000002
#define XPMSTRING           0x00000004
#define XPMFINISHED         0x00000008

#define XPMCASESENSITIVE    0x00000001
#define XPMCASENONSENSITIVE 0x00000002

// ---------
// - Enums -
// ---------

enum ReadState
{
    XPMREAD_OK,
    XPMREAD_ERROR,
    XPMREAD_NEED_MORE
};

// -------------
// - XPMReader -
// -------------

class BitmapWriteAccess;
class Graphic;

class XPMReader : public GraphicReader
{
private:

    SvStream&           mrIStm;
    Bitmap              maBmp;
    BitmapWriteAccess*  mpAcc;
    Bitmap              maMaskBmp;
    BitmapWriteAccess*  mpMaskAcc;
    long                mnLastPos;

    sal_uLong               mnWidth;
    sal_uLong               mnHeight;
    sal_uLong               mnColors;
    sal_uLong               mnCpp;                              // characters per pix
    sal_Bool                mbTransparent;
    sal_Bool                mbStatus;
    sal_uLong               mnStatus;
    sal_uLong               mnIdentifier;
    sal_uInt8               mcThisByte;
    sal_uInt8               mcLastByte;
    sal_uLong               mnTempAvail;
    sal_uInt8*              mpTempBuf;
    sal_uInt8*              mpTempPtr;
    sal_uInt8*              mpFastColorTable;
    sal_uInt8*              mpColMap;
    sal_uLong               mnStringSize;
    sal_uInt8*              mpStringBuf;
    sal_uLong               mnParaSize;
    sal_uInt8*              mpPara;

    sal_Bool                ImplGetString( void );
    sal_Bool                ImplGetColor( sal_uLong );
    sal_Bool                ImplGetScanLine( sal_uLong );
    sal_Bool                ImplGetColSub( sal_uInt8* );
    sal_Bool                ImplGetColKey( sal_uInt8 );
    void                ImplGetRGBHex( sal_uInt8*, sal_uLong );
    sal_Bool                ImplGetPara( sal_uLong numb );
    sal_Bool                ImplCompare( sal_uInt8*, sal_uInt8*, sal_uLong, sal_uLong nmode = XPMCASENONSENSITIVE );
    sal_uLong               ImplGetULONG( sal_uLong nPara );

public:
                        XPMReader( SvStream& rStm );
    virtual             ~XPMReader();

    ReadState           ReadXPM( Graphic& rGraphic );
};

#endif // _XPMPRIVATE

// -------------
// - ImportXPM -
// -------------

sal_Bool ImportXPM( SvStream& rStream, Graphic& rGraphic );

#endif // _XPMREAD_HXX
