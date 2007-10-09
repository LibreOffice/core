/*************************************************************************
*
*  OpenOffice.org - a multi-platform office productivity suite
*
*  $RCSfile: salbmp.h,v $
*
*  $Revision: 1.4 $
*
*  last change: $Author: kz $ $Date: 2007-10-09 15:08:33 $
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

#ifndef _SV_SALBMP_H
#define _SV_SALBMP_H

#include "tools/gen.hxx"
#include "vcl/sv.h"
#include "vcl/salbtype.hxx"
#include "saldata.hxx"
#include "vcl/salinst.hxx"
#include "salconst.h"
#include "vcl/salvd.hxx"
#include "salcolorutils.hxx"
#include "salpixmaputils.hxx"
#include "vcl/salbmp.hxx"
#include "salgdi.h"
#include "basebmp/bitmapdevice.hxx"

// --------------
// - SalBitmap  -
// --------------

struct  BitmapBuffer;
class   BitmapColor;
class   BitmapPalette;
class   AquaSalVirtualDevice;
class   AquaSalGraphics;

class AquaSalBitmap : public SalBitmap
{
public:
    CGContextRef                    mxGraphicContext;
    BitmapPalette                   maPalette;
    basebmp::RawMemorySharedArray   maUserBuffer;
    basebmp::RawMemorySharedArray   maContextBuffer;
    sal_uInt16                      mnBits;
    int                             mnWidth;
    int                             mnHeight;
    sal_uInt32                      mnBytesPerRow;

public:
    AquaSalBitmap();
    ~AquaSalBitmap();

public:

    // SalBitmap methods
    bool            Create( const Size& rSize, USHORT nBitCount, const BitmapPalette& rPal );
    bool            Create( const SalBitmap& rSalBmp );
    bool            Create( const SalBitmap& rSalBmp, SalGraphics* pGraphics );
    bool            Create( const SalBitmap& rSalBmp, USHORT nNewBitCount );

    void            Destroy();

    Size            GetSize() const;
    USHORT          GetBitCount() const;

    BitmapBuffer   *AcquireBuffer( bool bReadOnly );
    void            ReleaseBuffer( BitmapBuffer* pBuffer, bool bReadOnly );

    bool            GetSystemData( BitmapSystemData& rData );

    // quartz helper
    bool            CreateContext();
    void            DestroyContext();
    bool            AllocateUserData();

    void            ConvertBitmapData( sal_uInt32 nWidth, sal_uInt32 nHeight,
                                       sal_uInt16 nDestBits, sal_uInt32 nDestBytesPerRow, const BitmapPalette& rDestPalette, sal_uInt8* pDestData,
                                       sal_uInt16 nSrcBits, sal_uInt32 nSrcBytesPerRow, const BitmapPalette& rSrcPalette, sal_uInt8* pSrcData );

    bool            Create( CGContextRef xContext, int nX, int nY, int nWidth, int nHeight, bool bMirrorVert = true );
    bool            Create( int nSrcWidth, int nSrcHeight, int nBits,
                            sal_uInt32 nBytesPerRow, const sal_uInt8* pSrcBuffer,
                            int nSrcX, int nSrcY, int nBmpWidth, int nBmpHeight,
                            bool bMirrorVert = true
                            );
    bool            Create( CGImageRef& xImage );

    CGImageRef      CreateWithMask( const AquaSalBitmap& rMask, sal_uInt32 nX, sal_uInt32 nY, sal_uInt32 nDX, sal_uInt32 nDY );
    CGImageRef      CreateColorMask( int nX, int nY, int nWidth, int nHeight, SalColor nMaskColor ) const;
    CGImageRef      CreateMask( int nX, int nY, int nWidth, int nHeight ) const;
    CGImageRef      CreateCroppedImage( int nX, int nY, int nWidth, int nHeight );
    static CGImageRef CreateCroppedImage( CGContextRef xContext, int nX, int nY, int nWidth, int nHeight );

    // helper for basemp methods (e.g. for XOR rendering)
    basebmp::BitmapDeviceSharedPtr getBitmapDevice() const;
};

#endif // _SV_SALBMP_HXX
