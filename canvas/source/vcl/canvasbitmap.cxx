/*************************************************************************
 *
 *  $RCSfile: canvasbitmap.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: thb $ $Date: 2004-03-18 10:38:39 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _SV_BMPACC_HXX
#include <vcl/bmpacc.hxx>
#endif
#ifndef _VCL_CANVASTOOLS_HXX
#include <vcl/canvastools.hxx>
#endif

#include <canvas/canvastools.hxx>

#include "canvasbitmap.hxx"
#include "bitmapcanvas.hxx"
#include "impltools.hxx"

using namespace ::com::sun::star;
using namespace ::drafts::com::sun::star;

namespace vclcanvas
{
    // Currently, the only way to generate an XBitmap is from
    // XGraphicDevice.getCompatibleBitmap(). Therefore, we don't even
    // take a bitmap here, but a VDev directly.
    CanvasBitmap::CanvasBitmap( const ::Size&                   rSize,
                                const OutDevProvider::ImplRef&  rReferenceCanvas ) :
        mpReferenceCanvas( rReferenceCanvas ),
        mpBitmapCanvas( new BitmapCanvas( rSize,
                                          rReferenceCanvas ) )
    {
    }

    CanvasBitmap::CanvasBitmap( const BitmapEx&                 rBitmap,
                                const OutDevProvider::ImplRef&  rReferenceCanvas ) :
        mpReferenceCanvas( rReferenceCanvas ),
        mpBitmapCanvas( new BitmapCanvas( rBitmap.GetSizePixel(),
                                          rReferenceCanvas ) )
    {
        // TODO: Support alpha canvas here
        VirtualDevice& rVDev( getVirDev() );
        rVDev.EnableMapMode( FALSE );
        const Point aEmptyPoint(0,0);
        rVDev.DrawBitmapEx(aEmptyPoint, rBitmap);
    }

    CanvasBitmap::~CanvasBitmap()
    {
    }

    geometry::IntegerSize2D SAL_CALL CanvasBitmap::getSize(  ) throw (uno::RuntimeException)
    {
        tools::LocalGuard aGuard;
        return ::vcl::unotools::integerSize2DFromSize( getVirDev().GetOutputSizePixel() );
    }

    uno::Reference< rendering::XBitmapCanvas > SAL_CALL CanvasBitmap::queryBitmapCanvas(  ) throw (uno::RuntimeException)
    {
        return mpBitmapCanvas.getRef();
    }

    uno::Reference< rendering::XBitmap > SAL_CALL CanvasBitmap::getScaledBitmap( const geometry::RealSize2D& newSize, sal_Bool beFast ) throw (uno::RuntimeException)
    {
        tools::LocalGuard aGuard;

#if 0
        // TODO: To be used when dealing with real bitmaps here
        BitmapEx aRes( *maBitmap );

        aRes.Scale( ::canvas::tools::sizeFromRealSize2D(newSize),
                     beFast ? BMP_SCALE_FAST : BMP_SCALE_INTERPOLATE );

        return uno::Reference< rendering::XBitmap >( new CanvasBitmap( aRes ) );
#else
        // TODO: Support alpha canvas here
        VirtualDevice& rVDev( getVirDev() );
        const Point aEmptyPoint(0,0);
        const Size  aBmpSize( rVDev.GetOutputSizePixel() );

        Bitmap aBitmap( rVDev.GetBitmap(aEmptyPoint, aBmpSize) );

        aBitmap.Scale( ::vcl::unotools::sizeFromRealSize2D(newSize),
                       beFast ? BMP_SCALE_FAST : BMP_SCALE_INTERPOLATE );

        return uno::Reference< rendering::XBitmap >( new CanvasBitmap( aBitmap,
                                                                       mpReferenceCanvas ) );
#endif
    }

    uno::Sequence< sal_Int8 > SAL_CALL CanvasBitmap::getData( const geometry::IntegerRectangle2D& rect ) throw (lang::IndexOutOfBoundsException, rendering::VolatileContentDestroyedException,uno::RuntimeException)
    {
        tools::LocalGuard aGuard;

#if 0
        // TODO: To be used when dealing with real bitmaps here
        Bitmap aBitmap( maBitmap->GetBitmap() );
        Bitmap aAlpha( maBitmap->GetAlpha().GetBitmap() );
#else
        // TODO: Support alpha canvas here
        VirtualDevice& rVDev( getVirDev() );
        const Point aEmptyPoint(0,0);
        const Size  aBmpSize( rVDev.GetOutputSizePixel() );

        Bitmap aBitmap( rVDev.GetBitmap(aEmptyPoint, aBmpSize) );
        Bitmap aAlpha( Bitmap( aBmpSize, 8) );
#endif

        ScopedBitmapReadAccess pReadAccess( aBitmap.AcquireReadAccess(),
                                            aBitmap );
        ScopedBitmapReadAccess pAlphaReadAccess( aAlpha.AcquireReadAccess(),
                                                 aAlpha );

        if( pReadAccess.get() != NULL &&
            pAlphaReadAccess.get() != NULL )
        {
            // TODO: Support more formats.
            const Size aBmpSize( aBitmap.GetSizePixel() );

            // for the time being, always return as BGRA
            uno::Sequence< sal_Int8 > aRes( 4*aBmpSize.Width()*aBmpSize.Height() );

            int nCurrPos(0);
            for( int y=rect.Y1;
                 y<aBmpSize.Height() && y<rect.Y2;
                 ++y )
            {
                for( int x=rect.X1;
                     x<aBmpSize.Width() && x<rect.X2;
                     ++x )
                {
                    aRes[ nCurrPos++ ] = pReadAccess->GetColor( y, x ).GetBlue();
                    aRes[ nCurrPos++ ] = pReadAccess->GetColor( y, x ).GetGreen();
                    aRes[ nCurrPos++ ] = pReadAccess->GetColor( y, x ).GetRed();
                    aRes[ nCurrPos++ ] = pAlphaReadAccess->GetPixel( y, x ).GetIndex();
                }
            }

            return aRes;
        }

        return uno::Sequence< sal_Int8 >();
    }

    void SAL_CALL CanvasBitmap::setData( const uno::Sequence< sal_Int8 >& data, const geometry::IntegerRectangle2D& rect ) throw (lang::IllegalArgumentException, lang::IndexOutOfBoundsException, uno::RuntimeException)
    {
        tools::LocalGuard aGuard;

#if 0
        // TODO: To be used when dealing with real bitmaps here

        // retrieve local copies from the BitmapEx, which are later
        // stored back. Unfortunately, the BitmapEx does not permit
        // in-place modifications, as they are necessary here.
        Bitmap aBitmap( maBitmap->GetBitmap() );
        Bitmap aAlpha( maBitmap->GetAlpha().GetBitmap() );
#else
        // TODO: Support alpha canvas here
        VirtualDevice& rVDev( getVirDev() );
        const Point aEmptyPoint(0,0);
        const Size  aBmpSize( rVDev.GetOutputSizePixel() );

        Bitmap aBitmap( rVDev.GetBitmap(aEmptyPoint, aBmpSize) );
        Bitmap aAlpha( Bitmap( aBmpSize, 8) );
#endif

        bool bCopyBack( false ); // only copy something back, if we
                                 // actually changed pixel

        {
            ScopedBitmapWriteAccess pWriteAccess( aBitmap.AcquireWriteAccess(),
                                                  aBitmap );
            ScopedBitmapWriteAccess pAlphaWriteAccess( aAlpha.AcquireWriteAccess(),
                                                       aAlpha );

            if( pWriteAccess.get() != NULL &&
                pAlphaWriteAccess.get() != NULL )
            {
                // TODO: Support more formats.
                const Size aBmpSize( aBitmap.GetSizePixel() );

                // for the time being, always read as BGRA
                int nCurrPos(0);
                for( int y=rect.Y1;
                     y<aBmpSize.Height() && y<rect.Y2;
                     ++y )
                {
                    for( int x=rect.X1;
                         x<aBmpSize.Width() && x<rect.X2;
                         ++x )
                    {
                        pWriteAccess->SetPixel( y, x, BitmapColor( data[ nCurrPos+2 ],
                                                                   data[ nCurrPos+1 ],
                                                                   data[ nCurrPos ] ) );
                        nCurrPos += 3;

                        pAlphaWriteAccess->SetPixel( y, x, BitmapColor( 255 - data[ nCurrPos++ ] ) );
                    }
                }

                bCopyBack = true;
            }
        }

        // copy back only here, since the BitmapAccessors must be
        // destroyed beforehand
        if( bCopyBack )
        {
#if 0
            // TODO: To be used when dealing with real bitmaps here
            maBitmap = BitmapEx( aBitmap,
                                 AlphaMask( aAlpha ) );
#else
            // TODO: Support alpha canvas here
            VirtualDevice& rVDev( getVirDev() );
            rVDev.EnableMapMode( FALSE );
            const Point aEmptyPoint(0,0);
            rVDev.DrawBitmap(aEmptyPoint, aBitmap);
#endif
        }
    }

    void SAL_CALL CanvasBitmap::setPixel( const uno::Sequence< sal_Int8 >& color, const geometry::IntegerPoint2D& pos ) throw (lang::IllegalArgumentException, lang::IndexOutOfBoundsException, uno::RuntimeException)
    {
        tools::LocalGuard aGuard;

#if 0
        // TODO: To be used when dealing with real bitmaps here
        const Size aBmpSize( maBitmap->GetSizePixel() );
#else
        const Size aBmpSize( getVirDev().GetOutputSizePixel() );
#endif

        OSL_ENSURE( pos.X >= 0 && pos.X < aBmpSize.Width(), "CanvasBitmap::setPixel: X coordinate out of bounds" );
        OSL_ENSURE( pos.Y >= 0 && pos.Y < aBmpSize.Height(), "CanvasBitmap::setPixel: Y coordinate out of bounds" );
        OSL_ENSURE( color.getLength() > 3, "CanvasBitmap::setPixel: not enough color components" );

#if 0
        // TODO: To be used when dealing with real bitmaps here
        // retrieve local copies from the BitmapEx, which are later
        // stored back. Unfortunately, the BitmapEx does not permit
        // in-place modifications, as they are necessary here.
        Bitmap aBitmap( maBitmap->GetBitmap() );
        Bitmap aAlpha( maBitmap->GetAlpha().GetBitmap() );

        bool bCopyBack( false ); // only copy something back, if we
                                 // actually changed a pixel

        {
            ScopedBitmapWriteAccess pWriteAccess( aBitmap.AcquireWriteAccess(),
                                                  aBitmap );
            ScopedBitmapWriteAccess pAlphaWriteAccess( aAlpha.AcquireWriteAccess(),
                                                       aAlpha );

            if( pWriteAccess.get() != NULL &&
                pAlphaWriteAccess.get() != NULL )
            {
                pWriteAccess->SetPixel( pos.Y, pos.X, BitmapColor( color[ 2 ],
                                                                   color[ 1 ],
                                                                   color[ 0 ] ) );
                pAlphaWriteAccess->SetPixel( pos.Y, pos.X, BitmapColor( 255 - color[ 3 ] ) );
            }
        }

        // copy back only here, since the BitmapAccessors must be
        // destroyed beforehand
        if( bCopyBack )
        {
            maBitmap = BitmapEx( aBitmap,
                                 AlphaMask( aAlpha ) );
        }
#else
        VirtualDevice& rVDev( getVirDev() );
        rVDev.EnableMapMode( FALSE );

        uno::Reference< rendering::XCanvas > xCanvas( mpBitmapCanvas.getRef(),
                                                      uno::UNO_QUERY );
        OSL_ENSURE( xCanvas.is(), "CanvasBitmap::setPixel(): Invalid reference canvas" );

        getVirDev().DrawPixel( ::vcl::unotools::pointFromIntegerPoint2D( pos ),
                               ::vcl::unotools::sequenceToColor(
                                   xCanvas->getDevice(),
                                   color ) );
#endif
    }

    uno::Sequence< sal_Int8 > SAL_CALL CanvasBitmap::getPixel( const geometry::IntegerPoint2D& pos ) throw (lang::IndexOutOfBoundsException, rendering::VolatileContentDestroyedException, uno::RuntimeException)
    {
        tools::LocalGuard aGuard;

#if 0
        // TODO: To be used when dealing with real bitmaps here
        const Size aBmpSize( maBitmap->GetSizePixel() );
#else
        const Size aBmpSize( getVirDev().GetOutputSizePixel() );
#endif

        OSL_ENSURE( pos.X >= 0 && pos.X < aBmpSize.Width(), "CanvasBitmap::getPixel: X coordinate out of bounds" );
        OSL_ENSURE( pos.Y >= 0 && pos.Y < aBmpSize.Height(), "CanvasBitmap::getPixel: Y coordinate out of bounds" );

#if 0
        // TODO: To be used when dealing with real bitmaps here
        Bitmap aBitmap( maBitmap->GetBitmap() );
        Bitmap aAlpha( maBitmap->GetAlpha().GetBitmap() );
        ScopedBitmapReadAccess pReadAccess( aBitmap.AcquireReadAccess(),
                                            aBitmap );
        ScopedBitmapReadAccess pAlphaReadAccess( aAlpha.AcquireReadAccess(),
                                                 aAlpha );

        if( pReadAccess.get() != NULL &&
            pAlphaReadAccess.get() != NULL )
        {
            // for the time being, always return as BGRA
            uno::Sequence< sal_Int8 > aRes( 4 );

            const BitmapColor aColor( pReadAccess->GetColor( pos.Y, pos.X ) );
            aRes[ 3 ] = aColor.GetRed();
            aRes[ 2 ] = aColor.GetGreen();
            aRes[ 1 ] = aColor.GetBlue();

            aRes[ 3 ] = pAlphaReadAccess->GetPixel( pos.Y, pos.X ).GetIndex();

            return aRes;
        }

        return uno::Sequence< sal_Int8 >();
#else
        VirtualDevice& rVDev( getVirDev() );
        rVDev.EnableMapMode( FALSE );

        uno::Reference< rendering::XCanvas > xCanvas(mpBitmapCanvas.getRef(),
                                                     uno::UNO_QUERY);

        OSL_ENSURE( xCanvas.is(), "CanvasBitmap::getPixel(): Invalid reference canvas" );

        return ::vcl::unotools::colorToIntSequence(
            xCanvas->getDevice(),
            getVirDev().GetPixel( ::vcl::unotools::pointFromIntegerPoint2D( pos ) ) );
#endif
    }

    uno::Reference< rendering::XBitmapPalette > SAL_CALL CanvasBitmap::getPalette(  ) throw (uno::RuntimeException)
    {
        return uno::Reference< rendering::XBitmapPalette >();
    }

    rendering::IntegerBitmapLayout SAL_CALL CanvasBitmap::getMemoryLayout(  ) throw (uno::RuntimeException)
    {
        // TODO: finish that one
        rendering::IntegerBitmapLayout aLayout;

#if 0
        // TODO: To be used when dealing with real bitmaps here
        const Size aBmpSize( maBitmap->GetSizePixel() );
#else
        const Size aBmpSize( getVirDev().GetOutputSizePixel() );
#endif

        aLayout.ScanLines = aBmpSize.Height();
        aLayout.ScanLineBytes = aBmpSize.Width()*4;
        aLayout.ScanLineStride = aLayout.ScanLineBytes;
        aLayout.Format = 0;
        aLayout.NumComponents = 4;
        aLayout.ComponentMasks = uno::Sequence<sal_Int64>();
        aLayout.Endianness = 0;
        aLayout.IsPseudoColor = false;

        return aLayout;
    }

#define SERVICE_NAME "drafts.com.sun.star.rendering.CanvasBitmap"

    ::rtl::OUString SAL_CALL CanvasBitmap::getImplementationName(  ) throw (uno::RuntimeException)
    {
        return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( CANVASBITMAP_IMPLEMENTATION_NAME ) );
    }

    sal_Bool SAL_CALL CanvasBitmap::supportsService( const ::rtl::OUString& ServiceName ) throw (uno::RuntimeException)
    {
        return ServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM ( SERVICE_NAME ) );
    }

    uno::Sequence< ::rtl::OUString > SAL_CALL CanvasBitmap::getSupportedServiceNames(  ) throw (uno::RuntimeException)
    {
        uno::Sequence< ::rtl::OUString > aRet(1);
        aRet[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( SERVICE_NAME ) );

        return aRet;
    }

    VirtualDevice& CanvasBitmap::getVirDev()
    {
        return mpBitmapCanvas->getVirDev();
    }
}
