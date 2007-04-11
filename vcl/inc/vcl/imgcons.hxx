/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: imgcons.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 17:57:13 $
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

#ifndef _IMGCONS_HXX
#define _IMGCONS_HXX

#ifndef _VCL_DLLAPI_H
#include <vcl/dllapi.h>
#endif

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
