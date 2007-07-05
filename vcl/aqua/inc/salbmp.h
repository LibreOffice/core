/*************************************************************************
*
*  OpenOffice.org - a multi-platform office productivity suite
*
*  $RCSfile: salbmp.h,v $
*
*  $Revision: 1.2 $
*
*  last change: $Author: rt $ $Date: 2007-07-05 08:10:43 $
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

#ifndef _GEN_HXX
#include <tools/gen.hxx>
#endif

#ifndef _SV_SV_H
#include <sv.h>
#endif

#ifndef _SV_SALBTYPE_HXX
#include <salbtype.hxx>
#endif

#ifndef _SV_SALDATA_HXX
#include <saldata.hxx>
#endif

#ifndef _SV_SALINST_HXX
#include <salinst.hxx>
#endif

#ifndef _SV_SALCONST_H
#include <salconst.h>
#endif

#ifndef _SV_SALVD_HXX
#include <salvd.hxx>
#endif

#ifndef _SV_SALCOLORUTILS_HXX
#include <salcolorutils.hxx>
#endif

#ifndef _SV_SALPIXMAPUTILS_HXX
#include <salpixmaputils.hxx>
#endif

#ifndef _SV_SALBMP_HXX
#include <salbmp.hxx>
#endif

#ifndef _SV_SALGDI_H
#include <salgdi.h>
#endif

#ifndef INCLUDED_BASEBMP_BITMAPDEVICE_HXX
#include <basebmp/bitmapdevice.hxx>
#endif

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
    sal_uInt32                      mnWidth;
    sal_uInt32                      mnHeight;
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
    bool            Create( sal_uInt32 nWidth, sal_uInt32 nHeight, sal_uInt16 nBits, sal_uInt32 nBytesPerRow, sal_uInt8* pBuffer,
                            sal_uInt32 nX, sal_uInt32 nY, sal_uInt32 nDX, sal_uInt32 nDY,
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
