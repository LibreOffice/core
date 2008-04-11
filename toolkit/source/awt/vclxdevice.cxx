/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: vclxdevice.cxx,v $
 * $Revision: 1.7 $
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
#include "precompiled_toolkit.hxx"
#include <com/sun/star/awt/DeviceCapability.hpp>

#include <toolkit/awt/vclxdevice.hxx>
#include <toolkit/awt/vclxfont.hxx>
#include <toolkit/awt/vclxbitmap.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <toolkit/helper/macros.hxx>
#include <cppuhelper/typeprovider.hxx>

#include <rtl/memory.h>
#include <rtl/uuid.h>

#include <vcl/svapp.hxx>
#include <vcl/outdev.hxx>
#include <vcl/window.hxx>
#include <vcl/print.hxx>
#include <vcl/virdev.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/font.hxx>

//  ----------------------------------------------------
//  class VCLXDevice
//  ----------------------------------------------------
VCLXDevice::VCLXDevice() : mrMutex( Application::GetSolarMutex() )
{
    mpOutputDevice = NULL;
    nFlags = 0;
}

VCLXDevice::~VCLXDevice()
{
// Was thought for #88347#, but didn't help, because the interface will not be released
// But would be a good idea anyway, check after 6.0, it's a little bit dangerous now
//    if( mpOutputDevice && IsCreatedWithToolkit() )
//    {
//        delete mpOutputDevice;
//    }
}

void VCLXDevice::DestroyOutputDevice()
{
    delete mpOutputDevice;
    mpOutputDevice = NULL;
}

void VCLXDevice::SetCreatedWithToolkit( sal_Bool bCreatedWithToolkit )
{
    if ( bCreatedWithToolkit )
        nFlags |= FLAGS_CREATEDWITHTOOLKIT;
    else
        nFlags &= ~FLAGS_CREATEDWITHTOOLKIT;
}

sal_Bool VCLXDevice::IsCreatedWithToolkit() const
{
    return ( nFlags & FLAGS_CREATEDWITHTOOLKIT ) != 0;
}

// ::com::sun::star::uno::XInterface
::com::sun::star::uno::Any VCLXDevice::queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aRet = ::cppu::queryInterface( rType,
                                        SAL_STATIC_CAST( ::com::sun::star::awt::XDevice*, this ),
                                        SAL_STATIC_CAST( ::com::sun::star::lang::XUnoTunnel*, this ),
                                        SAL_STATIC_CAST( ::com::sun::star::lang::XTypeProvider*, this ) );
    return (aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType ));
}

// ::com::sun::star::lang::XUnoTunnel
IMPL_XUNOTUNNEL( VCLXDevice )

// ::com::sun::star::lang::XTypeProvider
IMPL_XTYPEPROVIDER_START( VCLXDevice )
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XDevice>* ) NULL )
IMPL_XTYPEPROVIDER_END


// ::com::sun::star::awt::XDevice,
::com::sun::star::uno::Reference< ::com::sun::star::awt::XGraphics > VCLXDevice::createGraphics(  ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XGraphics > xRef;

    if ( mpOutputDevice )
        xRef = mpOutputDevice->CreateUnoGraphics();

    return xRef;
}

::com::sun::star::uno::Reference< ::com::sun::star::awt::XDevice > VCLXDevice::createDevice( sal_Int32 nWidth, sal_Int32 nHeight ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XDevice >  xRef;
    if ( GetOutputDevice() )
    {
        VCLXVirtualDevice* pVDev = new VCLXVirtualDevice;
        VirtualDevice* pVclVDev = new VirtualDevice( *GetOutputDevice() );
        pVclVDev->SetOutputSizePixel( Size( nWidth, nHeight ) );
        pVDev->SetVirtualDevice( pVclVDev );
        xRef = pVDev;
    }
    return xRef;
}

::com::sun::star::awt::DeviceInfo VCLXDevice::getInfo() throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    ::com::sun::star::awt::DeviceInfo aInfo;

    if( mpOutputDevice )
    {
        Size aDevSz;
        OutDevType eDevType = mpOutputDevice->GetOutDevType();
        if ( eDevType == OUTDEV_WINDOW )
        {
            aDevSz = ((Window*)mpOutputDevice)->GetSizePixel();
            ((Window*)mpOutputDevice)->GetBorder( aInfo.LeftInset, aInfo.TopInset, aInfo.RightInset, aInfo.BottomInset );
        }
        else if ( eDevType == OUTDEV_PRINTER )
        {
            aDevSz = ((Printer*)mpOutputDevice)->GetPaperSizePixel();
            Size aOutSz = mpOutputDevice->GetOutputSizePixel();
            Point aOffset = ((Printer*)mpOutputDevice)->GetPageOffset();
            aInfo.LeftInset = aOffset.X();
            aInfo.TopInset = aOffset.Y();
            aInfo.RightInset = aDevSz.Width() - aOutSz.Width() - aOffset.X();
            aInfo.BottomInset = aDevSz.Height() - aOutSz.Height() - aOffset.Y();
        }
        else // VirtualDevice
        {
            aDevSz = mpOutputDevice->GetOutputSizePixel();
            aInfo.LeftInset = 0;
            aInfo.TopInset = 0;
            aInfo.RightInset = 0;
            aInfo.BottomInset = 0;
        }

        aInfo.Width = aDevSz.Width();
        aInfo.Height = aDevSz.Height();

        Size aTmpSz = mpOutputDevice->LogicToPixel( Size( 1000, 1000 ), MapMode( MAP_CM ) );
        aInfo.PixelPerMeterX = aTmpSz.Width()/10;
        aInfo.PixelPerMeterY = aTmpSz.Height()/10;

        aInfo.BitsPerPixel = mpOutputDevice->GetBitCount();

        aInfo.Capabilities = 0;
        if ( mpOutputDevice->GetOutDevType() != OUTDEV_PRINTER )
            aInfo.Capabilities = ::com::sun::star::awt::DeviceCapability::RASTEROPERATIONS|::com::sun::star::awt::DeviceCapability::GETBITS;
    }

    return aInfo;
}

::com::sun::star::uno::Sequence< ::com::sun::star::awt::FontDescriptor > VCLXDevice::getFontDescriptors(  ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    ::com::sun::star::uno::Sequence< ::com::sun::star::awt::FontDescriptor> aFonts;
    if( mpOutputDevice )
    {
        int nFonts = mpOutputDevice->GetDevFontCount();
        if ( nFonts )
        {
            aFonts = ::com::sun::star::uno::Sequence< ::com::sun::star::awt::FontDescriptor>( nFonts );
            ::com::sun::star::awt::FontDescriptor* pFonts = aFonts.getArray();
            for ( int n = 0; n < nFonts; n++ )
                pFonts[n] = VCLUnoHelper::CreateFontDescriptor( mpOutputDevice->GetDevFont( n ) );
        }
    }
    return aFonts;
}

::com::sun::star::uno::Reference< ::com::sun::star::awt::XFont > VCLXDevice::getFont( const ::com::sun::star::awt::FontDescriptor& rDescriptor ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XFont >  xRef;
    if( mpOutputDevice )
    {
        VCLXFont* pMetric = new VCLXFont;
        pMetric->Init( *this, VCLUnoHelper::CreateFont( rDescriptor, mpOutputDevice->GetFont() ) );
        xRef = pMetric;
    }
    return xRef;
}

::com::sun::star::uno::Reference< ::com::sun::star::awt::XBitmap > VCLXDevice::createBitmap( sal_Int32 nX, sal_Int32 nY, sal_Int32 nWidth, sal_Int32 nHeight ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XBitmap >  xBmp;
    if( mpOutputDevice )
    {
        Bitmap aBmp = mpOutputDevice->GetBitmap( Point( nX, nY ), Size( nWidth, nHeight ) );

        VCLXBitmap* pBmp = new VCLXBitmap;
        pBmp->SetBitmap( BitmapEx( aBmp ) );
        xBmp = pBmp;
    }
    return xBmp;
}

::com::sun::star::uno::Reference< ::com::sun::star::awt::XDisplayBitmap > VCLXDevice::createDisplayBitmap( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XBitmap >& rxBitmap ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    BitmapEx aBmp = VCLUnoHelper::GetBitmap( rxBitmap );
    VCLXBitmap* pBmp = new VCLXBitmap;
    pBmp->SetBitmap( aBmp );
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XDisplayBitmap >  xDBmp = pBmp;
    return xDBmp;
}


VCLXVirtualDevice::~VCLXVirtualDevice()
{
    ::vos::OGuard aGuard( GetMutex() );

    DestroyOutputDevice();
}


