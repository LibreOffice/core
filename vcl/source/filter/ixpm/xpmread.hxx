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

#ifndef _XPMREAD_HXX
#define _XPMREAD_HXX

#include <vcl/bitmap.hxx>

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

enum ReadState
{
    XPMREAD_OK,
    XPMREAD_ERROR,
    XPMREAD_NEED_MORE
};

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

sal_Bool ImportXPM( SvStream& rStream, Graphic& rGraphic );

#endif // _XPMREAD_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
