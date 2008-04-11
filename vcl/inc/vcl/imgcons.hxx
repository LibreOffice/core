/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: imgcons.hxx,v $
 * $Revision: 1.3 $
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

#ifndef _IMGCONS_HXX
#define _IMGCONS_HXX

#include <vcl/dllapi.h>

#include <vcl/bitmap.hxx>

// -----------------
// - ImageConsumer -
// -----------------

class ImageProducer;
class ImplColorMapper;
class BitmapEx;
class Image;

#define IMAGEERROR          1
#define SINGLEFRAMEDONE     2
#define STATICIMAGEDONE     3
#define IMAGEABORTED        4

class VCL_DLLPUBLIC ImageConsumer
{
private:

    Bitmap              maBitmap;
    Bitmap              maMask;
    Rectangle           maChangedRect;
    Size                maSize;
    ImplColorMapper*    mpMapper;
    Color*              mpPal;
    Link                maChgLink;
    Link                maDoneLink;
    sal_uInt32          mnFormat;
    sal_uInt32          mnStatus;
    BOOL                mbTrans;

protected:

    virtual void        DataChanged();

public:

                        ImageConsumer();
    virtual             ~ImageConsumer();

    BOOL                GetData( BitmapEx& rBmpEx ) const;
    BOOL                GetData( Image& rImage ) const;
    const Rectangle&    GetChangedRect() const { return maChangedRect; }
    sal_uInt32          GetStatus() const;

    void                SetDataChangedLink( const Link& rLink ) { maChgLink = rLink; }
    const Link&         GetDataChangedLink() const { return maChgLink; }

    void                SetDoneLink( const Link& rLink ) { maDoneLink = rLink; }
    const Link&         GetDoneLink() const { return maDoneLink; }

public:

    virtual void        Init( sal_uInt32 nWidth, sal_uInt32 nHeight );

    virtual void        SetColorModel( USHORT nBitCount,
                                       sal_uInt32 nPalEntries, const sal_uInt32* pRGBAPal,
                                       sal_uInt32 nRMask, sal_uInt32 nGMask, sal_uInt32 nBMask, sal_uInt32 nAMask  );

    virtual void        SetPixelsByBytes( sal_uInt32 nConsX, sal_uInt32 nConsY,
                                          sal_uInt32 nConsWidth, sal_uInt32 nConsHeight,
                                          const BYTE* pProducerData, sal_uInt32 nOffset, sal_uInt32 nScanSize );

    virtual void        SetPixelsByLongs( sal_uInt32 nConsX, sal_uInt32 nConsY,
                                          sal_uInt32 nConsWidth, sal_uInt32 nConsHeight,
                                          const sal_uInt32* pProducerData, sal_uInt32 nOffset, sal_uInt32 nScanSize );

    virtual void        Completed( sal_uInt32 nStatus );
//  virtual void        Completed( sal_uInt32 nStatus, ImageProducer& rProducer );
};

#endif // _IMGCONS_HXX
