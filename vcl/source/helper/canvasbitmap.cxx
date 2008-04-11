/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: canvasbitmap.cxx,v $
 * $Revision: 1.10 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"
#include <com/sun/star/rendering/Endianness.hpp>
#include <com/sun/star/rendering/IntegerBitmapFormat.hpp>

#ifndef _VOS_MUTEX_HXX
#include <vos/mutex.hxx>
#endif

#include <vcl/canvasbitmap.hxx>
#include <vcl/canvastools.hxx>
#include <vcl/bmpacc.hxx>
#include <vcl/svapp.hxx>

using namespace vcl::unotools;
using namespace com::sun::star::uno;
using namespace ::com::sun::star::rendering;
using namespace com::sun::star::lang;
using namespace ::com::sun::star::geometry;

VclCanvasBitmap::VclCanvasBitmap( const BitmapEx& rBitmap ) :
    m_pBitmap( new BitmapEx(rBitmap) ),
    m_aLayout(),
    m_bHavePalette( false )
{
    vos::OGuard aGuard( Application::GetSolarMutex() );
    Size aSz = m_pBitmap->GetSizePixel();
    m_aLayout.Palette.clear();
    m_bHavePalette = false;
    if( m_pBitmap->IsTransparent() )
    {
        m_aLayout.ScanLines         = aSz.Height();
        m_aLayout.ScanLineBytes     = aSz.Width()*4;
        m_aLayout.ScanLineStride    = aSz.Width()*4;
        m_aLayout.PlaneStride       = 0;
        m_aLayout.ColorSpace.clear(); // TODO(F2): Provide VCL-wide
                                      // default XGraphicDevice
        m_aLayout.NumComponents     = 4;
        m_aLayout.ComponentMasks.realloc( 4 );
        sal_Int64* pMasks = m_aLayout.ComponentMasks.getArray();
        pMasks[0] = 0xff000000LL;
        pMasks[1] = 0x00ff0000LL;
        pMasks[2] = 0x0000ff00LL;
        pMasks[3] = 0x000000ffLL;
        m_aLayout.Endianness        = Endianness::BIG;
        m_aLayout.Format            = IntegerBitmapFormat::CHUNKY_32BIT;
        m_aLayout.IsMsbFirst        = sal_False;
    }
    else
    {
        Bitmap aBmp = m_pBitmap->GetBitmap();
        BitmapReadAccess* pAcc = aBmp.AcquireReadAccess();

        if( !pAcc )
        {
            // bitmap is dysfunctional, fill out dummy MemLayout
            m_aLayout.ScanLines         = 0;
            m_aLayout.ScanLineBytes     = 0;
            m_aLayout.ScanLineStride    = 0;
            m_aLayout.PlaneStride       = 0;
            m_aLayout.ColorSpace.clear(); // TODO(F2): Provide VCL-wide
                                          // default XGraphicDevice
            m_aLayout.NumComponents     = 0;
            m_aLayout.Endianness        = Endianness::BIG;
            m_aLayout.Format            = IntegerBitmapFormat::CHUNKY_32BIT;
            m_aLayout.IsMsbFirst        = sal_False;
        }
        else
        {
            m_aLayout.ScanLines         = pAcc->Height();
            m_aLayout.ScanLineBytes     =
            m_aLayout.ScanLineStride    = pAcc->GetScanlineSize();
            m_aLayout.PlaneStride       = 0;
            m_aLayout.ColorSpace.clear(); // TODO(F2): Provide VCL-wide
                                          // default XGraphicDevice
            switch( pAcc->GetScanlineFormat() )
            {
                case BMP_FORMAT_1BIT_MSB_PAL:
                    m_aLayout.NumComponents     = 1;
                    m_aLayout.ComponentMasks.realloc( 1 );
                    m_aLayout.ComponentMasks.getArray()[0] = 0x1LL;
                    m_bHavePalette = true;
                    m_aLayout.Endianness        = Endianness::BIG;
                    m_aLayout.Format            = IntegerBitmapFormat::CHUNKY_1BIT;
                    m_aLayout.IsMsbFirst        = sal_True;
                    break;
                case BMP_FORMAT_1BIT_LSB_PAL:
                    m_aLayout.NumComponents     = 1;
                    m_aLayout.ComponentMasks.realloc( 1 );
                    m_aLayout.ComponentMasks.getArray()[0] = 1LL;
                    m_bHavePalette = true;
                    m_aLayout.Endianness        = Endianness::BIG;
                    m_aLayout.Format            = IntegerBitmapFormat::CHUNKY_1BIT;
                    m_aLayout.IsMsbFirst        = sal_False;
                    break;
                case BMP_FORMAT_4BIT_MSN_PAL:
                    m_aLayout.NumComponents     = 1;
                    m_aLayout.ComponentMasks.realloc( 1 );
                    m_aLayout.ComponentMasks.getArray()[0] = 0x0fLL;
                    m_bHavePalette = true;
                    m_aLayout.Endianness        = Endianness::BIG;
                    m_aLayout.Format            = IntegerBitmapFormat::CHUNKY_4BIT;
                    m_aLayout.IsMsbFirst        = sal_True;
                    break;
                case BMP_FORMAT_4BIT_LSN_PAL:
                    m_aLayout.NumComponents     = 1;
                    m_aLayout.ComponentMasks.realloc( 1 );
                    m_aLayout.ComponentMasks.getArray()[0] = 0x0fLL;
                    m_bHavePalette = true;
                    m_aLayout.Endianness        = Endianness::BIG;
                    m_aLayout.Format            = IntegerBitmapFormat::CHUNKY_4BIT;
                    m_aLayout.IsMsbFirst        = sal_False;
                    break;
                case BMP_FORMAT_8BIT_PAL:
                    m_aLayout.NumComponents     = 1;
                    m_aLayout.ComponentMasks.realloc( 1 );
                    m_aLayout.ComponentMasks.getArray()[0] = 0xffLL;
                    m_bHavePalette = true;
                    m_aLayout.Endianness        = Endianness::BIG;
                    m_aLayout.Format            = IntegerBitmapFormat::CHUNKY_8BIT;
                    m_aLayout.IsMsbFirst        = sal_False;
                    break;

                case BMP_FORMAT_8BIT_TC_MASK:
                    m_aLayout.NumComponents     = 3;
                    m_aLayout.ComponentMasks.realloc( 3 );
                    m_aLayout.ComponentMasks.getArray()[0] = pAcc->GetColorMask().GetRedMask();
                    m_aLayout.ComponentMasks.getArray()[1] = pAcc->GetColorMask().GetGreenMask();
                    m_aLayout.ComponentMasks.getArray()[2] = pAcc->GetColorMask().GetBlueMask();
                    m_aLayout.Endianness        = Endianness::BIG;
                    m_aLayout.Format            = IntegerBitmapFormat::CHUNKY_8BIT;
                    m_aLayout.IsMsbFirst        = sal_False;
                    break;
                case BMP_FORMAT_16BIT_TC_MSB_MASK:
                    m_aLayout.NumComponents     = 3;
                    m_aLayout.ComponentMasks.realloc( 3 );
                    m_aLayout.ComponentMasks.getArray()[0] = pAcc->GetColorMask().GetRedMask();
                    m_aLayout.ComponentMasks.getArray()[1] = pAcc->GetColorMask().GetGreenMask();
                    m_aLayout.ComponentMasks.getArray()[2] = pAcc->GetColorMask().GetBlueMask();
                    m_aLayout.Endianness        = Endianness::BIG;
                    m_aLayout.Format            = IntegerBitmapFormat::CHUNKY_16BIT;
                    m_aLayout.IsMsbFirst        = sal_False;
                    break;
                case BMP_FORMAT_16BIT_TC_LSB_MASK:
                    m_aLayout.NumComponents     = 3;
                    m_aLayout.ComponentMasks.realloc( 3 );
                    m_aLayout.ComponentMasks.getArray()[0] = pAcc->GetColorMask().GetRedMask();
                    m_aLayout.ComponentMasks.getArray()[1] = pAcc->GetColorMask().GetGreenMask();
                    m_aLayout.ComponentMasks.getArray()[2] = pAcc->GetColorMask().GetBlueMask();
                    m_aLayout.Endianness        = Endianness::LITTLE;
                    m_aLayout.Format            = IntegerBitmapFormat::CHUNKY_16BIT;
                    m_aLayout.IsMsbFirst        = sal_False;
                    break;
                case BMP_FORMAT_24BIT_TC_BGR:
                    m_aLayout.NumComponents     = 3;
                    m_aLayout.ComponentMasks.realloc( 3 );
                    m_aLayout.ComponentMasks.getArray()[0] = 0x0000ffLL;
                    m_aLayout.ComponentMasks.getArray()[1] = 0x00ff00LL;
                    m_aLayout.ComponentMasks.getArray()[2] = 0xff0000LL;
                    m_aLayout.Endianness        = Endianness::BIG;
                    m_aLayout.Format            = IntegerBitmapFormat::CHUNKY_24BIT;
                    m_aLayout.IsMsbFirst        = sal_False;
                    break;
                case BMP_FORMAT_24BIT_TC_RGB:
                    m_aLayout.NumComponents     = 3;
                    m_aLayout.ComponentMasks.realloc( 3 );
                    m_aLayout.ComponentMasks.getArray()[0] = 0xff0000LL;
                    m_aLayout.ComponentMasks.getArray()[1] = 0x00ff00LL;
                    m_aLayout.ComponentMasks.getArray()[2] = 0x0000ffLL;
                    m_aLayout.Endianness        = Endianness::BIG;
                    m_aLayout.Format            = IntegerBitmapFormat::CHUNKY_24BIT;
                    m_aLayout.IsMsbFirst        = sal_False;
                    break;
                case BMP_FORMAT_24BIT_TC_MASK:
                    m_aLayout.NumComponents     = 3;
                    m_aLayout.ComponentMasks.realloc( 3 );
                    m_aLayout.ComponentMasks.getArray()[0] = pAcc->GetColorMask().GetRedMask();
                    m_aLayout.ComponentMasks.getArray()[1] = pAcc->GetColorMask().GetGreenMask();
                    m_aLayout.ComponentMasks.getArray()[2] = pAcc->GetColorMask().GetBlueMask();
                    m_aLayout.Endianness        = Endianness::LITTLE;
                    m_aLayout.Format            = IntegerBitmapFormat::CHUNKY_24BIT;
                    m_aLayout.IsMsbFirst        = sal_False;
                    break;
                case BMP_FORMAT_32BIT_TC_ABGR:
                    m_aLayout.NumComponents     = 4;
                    m_aLayout.ComponentMasks.realloc( 4 );
                    m_aLayout.ComponentMasks.getArray()[0] = 0x000000ffLL;
                    m_aLayout.ComponentMasks.getArray()[1] = 0x0000ff00LL;
                    m_aLayout.ComponentMasks.getArray()[2] = 0x00ff0000LL;
                    m_aLayout.ComponentMasks.getArray()[3] = 0xff000000LL;
                    m_aLayout.Endianness        = Endianness::BIG;
                    m_aLayout.Format            = IntegerBitmapFormat::CHUNKY_32BIT;
                    m_aLayout.IsMsbFirst        = sal_False;
                    break;
                case BMP_FORMAT_32BIT_TC_ARGB:
                    m_aLayout.NumComponents     = 4;
                    m_aLayout.ComponentMasks.realloc( 4 );
                    m_aLayout.ComponentMasks.getArray()[0] = 0x00ff0000LL;
                    m_aLayout.ComponentMasks.getArray()[1] = 0x0000ff00LL;
                    m_aLayout.ComponentMasks.getArray()[2] = 0x000000ffLL;
                    m_aLayout.ComponentMasks.getArray()[3] = 0xff000000LL;
                    m_aLayout.Endianness        = Endianness::BIG;
                    m_aLayout.Format            = IntegerBitmapFormat::CHUNKY_32BIT;
                    m_aLayout.IsMsbFirst        = sal_False;
                    break;
                case BMP_FORMAT_32BIT_TC_BGRA:
                    m_aLayout.NumComponents     = 4;
                    m_aLayout.ComponentMasks.realloc( 4 );
                    m_aLayout.ComponentMasks.getArray()[0] = 0x0000ff00LL;
                    m_aLayout.ComponentMasks.getArray()[1] = 0x00ff0000LL;
                    m_aLayout.ComponentMasks.getArray()[2] = 0xff000000LL;
                    m_aLayout.ComponentMasks.getArray()[3] = 0x000000ffLL;
                    m_aLayout.Endianness        = Endianness::BIG;
                    m_aLayout.Format            = IntegerBitmapFormat::CHUNKY_32BIT;
                    m_aLayout.IsMsbFirst        = sal_False;
                    break;
                case BMP_FORMAT_32BIT_TC_RGBA:
                    m_aLayout.NumComponents     = 4;
                    m_aLayout.ComponentMasks.realloc( 4 );
                    m_aLayout.ComponentMasks.getArray()[0] = 0xff000000LL;
                    m_aLayout.ComponentMasks.getArray()[1] = 0x00ff0000LL;
                    m_aLayout.ComponentMasks.getArray()[2] = 0x0000ff00LL;
                    m_aLayout.ComponentMasks.getArray()[3] = 0x000000ffLL;
                    m_aLayout.Endianness        = Endianness::BIG;
                    m_aLayout.Format            = IntegerBitmapFormat::CHUNKY_32BIT;
                    m_aLayout.IsMsbFirst        = sal_False;
                    break;
                case BMP_FORMAT_32BIT_TC_MASK:
                    m_aLayout.NumComponents     = 3;
                    m_aLayout.ComponentMasks.realloc( 3 );
                    m_aLayout.ComponentMasks.getArray()[0] = pAcc->GetColorMask().GetRedMask();
                    m_aLayout.ComponentMasks.getArray()[1] = pAcc->GetColorMask().GetGreenMask();
                    m_aLayout.ComponentMasks.getArray()[2] = pAcc->GetColorMask().GetBlueMask();
                    m_aLayout.Endianness        = Endianness::LITTLE;
                    m_aLayout.Format            = IntegerBitmapFormat::CHUNKY_32BIT;
                    m_aLayout.IsMsbFirst        = sal_False;
                    break;
                default:
                    DBG_ERROR( "unsupported bitmap format" );
                    break;
            }
        }

        aBmp.ReleaseAccess( pAcc );
    }
}

VclCanvasBitmap::~VclCanvasBitmap()
{
    vos::OGuard aGuard( Application::GetSolarMutex() );
    delete m_pBitmap;
}

// XBitmap
IntegerSize2D SAL_CALL VclCanvasBitmap::getSize() throw (RuntimeException)
{
    vos::OGuard aGuard( Application::GetSolarMutex() );

    return integerSize2DFromSize( m_pBitmap->GetSizePixel() );
}

::sal_Bool SAL_CALL VclCanvasBitmap::hasAlpha() throw (RuntimeException)
{
    vos::OGuard aGuard( Application::GetSolarMutex() );

    return m_pBitmap->IsTransparent();
}

Reference< XBitmapCanvas > SAL_CALL VclCanvasBitmap::queryBitmapCanvas() throw (RuntimeException)
{
    return Reference< XBitmapCanvas >();
}

Reference< XBitmap > SAL_CALL VclCanvasBitmap::getScaledBitmap( const RealSize2D& newSize, sal_Bool beFast ) throw (RuntimeException)
{
    vos::OGuard aGuard( Application::GetSolarMutex() );

    BitmapEx aNewBmp( *m_pBitmap );
    aNewBmp.Scale( sizeFromRealSize2D( newSize ), beFast ? BMP_SCALE_FAST : BMP_SCALE_INTERPOLATE );
    return Reference<XBitmap>( new VclCanvasBitmap( aNewBmp ) );
}

// XIntegerBitmap
Sequence< sal_Int8 > SAL_CALL VclCanvasBitmap::getData( IntegerBitmapLayout&      bitmapLayout,
                                                        const IntegerRectangle2D& /*rect*/ ) throw (IndexOutOfBoundsException,
                                                                                                VolatileContentDestroyedException,
                                                                                                RuntimeException)
{
    vos::OGuard aGuard( Application::GetSolarMutex() );

    bitmapLayout = getMemoryLayout();

    Bitmap aBmp = m_pBitmap->GetBitmap();
    BitmapReadAccess* pAcc = aBmp.AcquireReadAccess();

    // Invalid/empty bitmap: no data available
    if( !pAcc )
        return Sequence< sal_Int8 >();

    Sequence< sal_Int8 > aRet;
    if( m_pBitmap->IsTransparent() )
    {
        int w = pAcc->Width();
        int h = pAcc->Height();
        aRet.realloc( 4 * w * h );
        sal_Int8* pContent = aRet.getArray();
        for( int y = 0; y < h; y++ )
            for( int x = 0; x < w; x++ )
        {
            BitmapColor aCol = pAcc->GetColor( x, y );
            *pContent++ = aCol.GetRed();
            *pContent++ = aCol.GetGreen();
            *pContent++ = aCol.GetBlue();
            pContent++;
        }
        pContent = aRet.getArray()+3;
        if( m_pBitmap->IsAlpha() )
        {
            AlphaMask aAlpha = m_pBitmap->GetAlpha();
            BitmapReadAccess* pAlphaAcc = aAlpha.AcquireReadAccess();

            // Invalid/empty alpha mask: keep alpha bytes to 0
            if( pAlphaAcc )
            {
                for( int y = 0; y < h; y++ )
                {
                    Scanline pLine = pAlphaAcc->GetScanline( y );
                    for( int x = 0; x < w; x++ )
                    {
                        *pContent = *pLine++;
                        pContent += 4;
                    }
                }
            }
            aAlpha.ReleaseAccess( pAlphaAcc );
        }
        else if( m_pBitmap->GetTransparentType() == TRANSPARENT_BITMAP )
        {
            Bitmap aMask = m_pBitmap->GetMask();
            BitmapReadAccess* pMaskAcc = aMask.AcquireReadAccess();

            // Invalid/empty mask: keep alpha bytes to 0
            if( pMaskAcc )
            {
                BitmapColor aZeroCol = pMaskAcc->GetPaletteColor( 0 );
                sal_uInt8 nIndexZeroAlpha = 0xff;
                if( !aZeroCol.GetRed() && ! aZeroCol.GetGreen() && ! aZeroCol.GetBlue() )
                    nIndexZeroAlpha = 0;
                sal_uInt8 nIndexOneAlpha = ~nIndexZeroAlpha;
                for( int y = 0; y < h; y++ )
                    for( int x = 0; x < w; x++ )
                    {
                        *pContent = pMaskAcc->GetPixel(x,y).GetIndex() ? nIndexOneAlpha : nIndexZeroAlpha;
                        pContent += 4;
                    }
            }
            aMask.ReleaseAccess( pMaskAcc );
        }
        else if( m_pBitmap->GetTransparentType() == TRANSPARENT_COLOR )
        {
            sal_Int8 nR = m_pBitmap->GetTransparentColor().GetRed();
            sal_Int8 nG = m_pBitmap->GetTransparentColor().GetGreen();
            sal_Int8 nB = m_pBitmap->GetTransparentColor().GetBlue();
            for( int y = 0; y < h; y++ )
                for( int x = 0; x < w; x++ )
            {
                *pContent = (pContent[-3] == nR && pContent[-2] == nG && pContent[-1] == nB) ? 0xff : 0;
                pContent += 4;
            }
        }
        else
        {
            aBmp.ReleaseAccess( pAcc );
            DBG_ERROR( "unsupported transparency type" );
            throw RuntimeException();
        }
    }
    else
    {
        int nH = pAcc->Height();
        int nScS = pAcc->GetScanlineSize();
        aRet.realloc( nScS * nH );
        sal_Int8* pContent = aRet.getArray();
        for( int i = 0; i < nH; i++ )
        {
            Scanline pLine = pAcc->GetScanline( i );
            rtl_copyMemory( pContent, pLine, nScS );
            pContent += nScS;
        }
    }
    aBmp.ReleaseAccess( pAcc );
    return aRet;
}

void SAL_CALL VclCanvasBitmap::setData( const Sequence< sal_Int8 >& /*data*/, const IntegerBitmapLayout& /*bitmapLayout*/, const IntegerRectangle2D& /*rect*/ ) throw (IllegalArgumentException, IndexOutOfBoundsException, RuntimeException)
{
    DBG_ERROR( "this XBitmap implementation is readonly" );
    throw IllegalArgumentException();
}

void SAL_CALL VclCanvasBitmap::setPixel( const Sequence< sal_Int8 >& /*color*/, const IntegerBitmapLayout& /*bitmapLayout*/, const IntegerPoint2D& /*pos*/ ) throw (IllegalArgumentException, IndexOutOfBoundsException, RuntimeException)
{
    DBG_ERROR( "this XBitmap implementation is readonly" );
    throw IllegalArgumentException();
}

Sequence< sal_Int8 > SAL_CALL VclCanvasBitmap::getPixel( IntegerBitmapLayout&   bitmapLayout,
                                                         const IntegerPoint2D&  pos ) throw (IndexOutOfBoundsException,
                                                                                             VolatileContentDestroyedException,
                                                                                             RuntimeException)
{
    vos::OGuard aGuard( Application::GetSolarMutex() );

    bitmapLayout = getMemoryLayout();

    Point aPos = pointFromIntegerPoint2D( pos );
    if( aPos.X() < 0 || aPos.Y() < 0 )
        throw IndexOutOfBoundsException();

    Bitmap aBmp = m_pBitmap->GetBitmap();
    BitmapReadAccess* pAcc = aBmp.AcquireReadAccess();

    // Invalid/empty bitmap: no data available
    if( !pAcc )
        return Sequence< sal_Int8 >();

    if( aPos.X() >= pAcc->Width() || aPos.Y() >= pAcc->Height() )
    {
        aBmp.ReleaseAccess( pAcc );
        throw IndexOutOfBoundsException();
    }

    Sequence< sal_Int8 > aRet;
    if( m_pBitmap->IsTransparent() )
    {
        BitmapColor aCol = pAcc->GetColor( aPos.X(), aPos.Y() );
        sal_uInt8 nAlpha = 0;
        if( m_pBitmap->IsAlpha() )
        {
            AlphaMask aMask = m_pBitmap->GetAlpha();
            BitmapReadAccess* pAlphaAcc = aMask.AcquireReadAccess();

            // Invalid/empty alpha mask: keep alpha value to 0
            if( pAlphaAcc )
            {
                BitmapColor aAlphaCol = pAlphaAcc->GetPixel( aPos.X(), aPos.Y() );
                nAlpha = aAlphaCol.GetIndex();
            }
            aMask.ReleaseAccess( pAlphaAcc );
        }
        else if( m_pBitmap->GetTransparentType() == TRANSPARENT_COLOR )
        {
            Color aTranspCol = m_pBitmap->GetTransparentColor();
            if( aCol.GetRed()   == aTranspCol.GetRed() &&
                aCol.GetGreen() == aTranspCol.GetGreen() &&
                aCol.GetBlue()  == aTranspCol.GetBlue() )
                nAlpha = 0xff;
        }
        else if( m_pBitmap->GetTransparentType() == TRANSPARENT_BITMAP )
        {
            Bitmap aMask = m_pBitmap->GetMask();
            BitmapReadAccess* pMaskAcc = aMask.AcquireReadAccess();

            // Invalid/empty mask: keep alpha value to 0
            if( pMaskAcc )
            {
                BitmapColor aMaskColor = pMaskAcc->GetColor( aPos.X(), aPos.Y() );
                if( aMaskColor.GetRed() || aMaskColor.GetGreen() || aMaskColor.GetBlue() )
                    nAlpha = 0xff;
            }
            aMask.ReleaseAccess( pMaskAcc );
        }
        else
        {
            aBmp.ReleaseAccess( pAcc );
            DBG_ERROR( "unsupported transparency type" );
            throw RuntimeException();
        }
        aRet.realloc( 4 );
        sal_Int8* pContent = aRet.getArray();
        *pContent++ = aCol.GetRed();
        *pContent++ = aCol.GetGreen();
        *pContent++ = aCol.GetBlue();
        *pContent = nAlpha;
    }
    else
    {
        if( pAcc->GetBitCount() < 8 )
        {
            aRet.realloc( 1 );
            *aRet.getArray() = pAcc->GetPixel( aPos.X(), aPos.Y() ).GetIndex();
        }
        else
        {
            int nByteSize = pAcc->GetBitCount()/4;
            aRet.realloc( nByteSize );
            Scanline pLine = pAcc->GetScanline( aPos.Y() );
            rtl_copyMemory( aRet.getArray(), pLine + aPos.X() * nByteSize, nByteSize );
        }
    }
    aBmp.ReleaseAccess( pAcc );

    return aRet;
}

Reference< XBitmapPalette > SAL_CALL VclCanvasBitmap::getPalette() throw (RuntimeException)
{
    return m_bHavePalette ?
        Reference< XBitmapPalette >( this ) :
        Reference< XBitmapPalette >();
}

IntegerBitmapLayout SAL_CALL VclCanvasBitmap::getMemoryLayout() throw (RuntimeException)
{
    IntegerBitmapLayout aLayout( m_aLayout );

    // only set bitmap palette on separate copy of IntegerBitmapLayout
    // - if we'd set that on m_aLayout, we'd have a circular reference!
    if( m_bHavePalette )
        aLayout.Palette.set( this );

    return aLayout;
}

sal_Int32 SAL_CALL VclCanvasBitmap::getNumberOfEntries() throw (RuntimeException)
{
    vos::OGuard aGuard( Application::GetSolarMutex() );
    Bitmap aBmp = m_pBitmap->GetBitmap();
    BitmapReadAccess* pAcc = aBmp.AcquireReadAccess();

    if( !pAcc )
        return 0;

    sal_Int32 nEntries = pAcc->HasPalette() ? pAcc->GetPaletteEntryCount() : 0 ;
    aBmp.ReleaseAccess( pAcc );

    return nEntries;
}

Sequence< double > SAL_CALL VclCanvasBitmap::getPaletteIndex( sal_Int32 nIndex ) throw (IndexOutOfBoundsException, RuntimeException)
{
    vos::OGuard aGuard( Application::GetSolarMutex() );

    Sequence< double > aRet( 3 );
    double* pContents = aRet.getArray();
    Bitmap aBmp = m_pBitmap->GetBitmap();
    BitmapReadAccess* pAcc = aBmp.AcquireReadAccess();

    if( !pAcc )
        return Sequence< double >();

    if( nIndex >= 0 && nIndex < pAcc->GetPaletteEntryCount() )
    {
        // range already checked above
        const BitmapColor& rColor = pAcc->GetPaletteColor(
            static_cast<USHORT>(nIndex) );
        *pContents++ = double(rColor.GetRed()) / 255.0;
        *pContents++ = double(rColor.GetGreen()) / 255.0;
        *pContents++ = double(rColor.GetBlue()) / 255.0;
    }
    else
    {
        aBmp.ReleaseAccess( pAcc );
        throw IndexOutOfBoundsException();
    }
    aBmp.ReleaseAccess( pAcc );

    return aRet;
}

sal_Bool SAL_CALL VclCanvasBitmap::setPaletteIndex( const Sequence< double >& /*color*/, sal_Int32 /*nIndex*/ ) throw (IndexOutOfBoundsException, IllegalArgumentException, RuntimeException)
{
    return sal_False; // read only implementation
}

Reference< XColorSpace > SAL_CALL VclCanvasBitmap::getColorSpace(  ) throw (RuntimeException)
{
    return Reference< XColorSpace >(); // TODO(F2): Provide VCL-wide
                                       // default XGraphicDevice
}

sal_Int64 SAL_CALL VclCanvasBitmap::getSomething( const Sequence< sal_Int8 >& aIdentifier ) throw (RuntimeException)
{
    vos::OGuard aGuard( Application::GetSolarMutex() );

    sal_Int64 nRet = 0;
    const Sequence< sal_Int8 >& rTest = getTunnelIdentifier( Id_BitmapEx );
    if( aIdentifier.getLength() == rTest.getLength() &&
        rtl_compareMemory( rTest.getConstArray(), aIdentifier.getConstArray(), rTest.getLength() ) == 0
        )
    {
        nRet = (sal_Int64)sal_IntPtr(m_pBitmap);
    }
    return nRet;
}
