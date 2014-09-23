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

#include <com/sun/star/awt/DeviceCapability.hpp>

#include <com/sun/star/util/MeasureUnit.hpp>

#include <toolkit/awt/vclxdevice.hxx>
#include <toolkit/awt/vclxfont.hxx>
#include <toolkit/awt/vclxbitmap.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <toolkit/helper/macros.hxx>
#include <cppuhelper/typeprovider.hxx>

#include <rtl/uuid.h>

#include <vcl/svapp.hxx>
#include <vcl/outdev.hxx>
#include <vcl/window.hxx>
#include <vcl/print.hxx>
#include <vcl/virdev.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/font.hxx>


//  class VCLXDevice

VCLXDevice::VCLXDevice()
    : mpOutputDevice(NULL)
    , pDummy(NULL)
    , nFlags(0)
{
}

VCLXDevice::~VCLXDevice()
{
}

void VCLXDevice::DestroyOutputDevice()
{
    delete mpOutputDevice;
    mpOutputDevice = NULL;
}

void VCLXDevice::SetCreatedWithToolkit( bool bCreatedWithToolkit )
{
    if ( bCreatedWithToolkit )
        nFlags |= FLAGS_CREATEDWITHTOOLKIT;
    else
        nFlags &= ~FLAGS_CREATEDWITHTOOLKIT;
}

// ::com::sun::star::uno::XInterface
::com::sun::star::uno::Any VCLXDevice::queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    ::com::sun::star::uno::Any aRet = ::cppu::queryInterface( rType,
                                        (static_cast< ::com::sun::star::awt::XDevice* >(this)),
                                        (static_cast< ::com::sun::star::lang::XUnoTunnel* >(this)),
                                        (static_cast< ::com::sun::star::lang::XTypeProvider* >(this)),
                                        (static_cast< ::com::sun::star::awt::XUnitConversion* >(this)) );
    return (aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType ));
}

// ::com::sun::star::lang::XUnoTunnel
IMPL_XUNOTUNNEL( VCLXDevice )

// ::com::sun::star::lang::XTypeProvider
IMPL_XTYPEPROVIDER_START( VCLXDevice )
    cppu::UnoType<com::sun::star::awt::XDevice>::get(),
    cppu::UnoType<com::sun::star::awt::XUnitConversion>::get()
IMPL_XTYPEPROVIDER_END


// ::com::sun::star::awt::XDevice,
::com::sun::star::uno::Reference< ::com::sun::star::awt::XGraphics > VCLXDevice::createGraphics(  ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XGraphics > xRef;

    if ( mpOutputDevice )
        xRef = mpOutputDevice->CreateUnoGraphics();

    return xRef;
}

::com::sun::star::uno::Reference< ::com::sun::star::awt::XDevice > VCLXDevice::createDevice( sal_Int32 nWidth, sal_Int32 nHeight ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

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

::com::sun::star::awt::DeviceInfo VCLXDevice::getInfo() throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    ::com::sun::star::awt::DeviceInfo aInfo;

    if( mpOutputDevice )
    {
        Size aDevSz;
        OutDevType eDevType = mpOutputDevice->GetOutDevType();
        if ( eDevType == OUTDEV_WINDOW )
        {
            aDevSz = ((vcl::Window*)mpOutputDevice)->GetSizePixel();
            ((vcl::Window*)mpOutputDevice)->GetBorder( aInfo.LeftInset, aInfo.TopInset, aInfo.RightInset, aInfo.BottomInset );
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

::com::sun::star::uno::Sequence< ::com::sun::star::awt::FontDescriptor > VCLXDevice::getFontDescriptors(  ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

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

::com::sun::star::uno::Reference< ::com::sun::star::awt::XFont > VCLXDevice::getFont( const ::com::sun::star::awt::FontDescriptor& rDescriptor ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XFont >  xRef;
    if( mpOutputDevice )
    {
        VCLXFont* pMetric = new VCLXFont;
        pMetric->Init( *this, VCLUnoHelper::CreateFont( rDescriptor, mpOutputDevice->GetFont() ) );
        xRef = pMetric;
    }
    return xRef;
}

::com::sun::star::uno::Reference< ::com::sun::star::awt::XBitmap > VCLXDevice::createBitmap( sal_Int32 nX, sal_Int32 nY, sal_Int32 nWidth, sal_Int32 nHeight ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

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

::com::sun::star::uno::Reference< ::com::sun::star::awt::XDisplayBitmap > VCLXDevice::createDisplayBitmap( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XBitmap >& rxBitmap ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    BitmapEx aBmp = VCLUnoHelper::GetBitmap( rxBitmap );
    VCLXBitmap* pBmp = new VCLXBitmap;
    pBmp->SetBitmap( aBmp );
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XDisplayBitmap >  xDBmp = pBmp;
    return xDBmp;
}


VCLXVirtualDevice::~VCLXVirtualDevice()
{
    SolarMutexGuard aGuard;

    DestroyOutputDevice();
}



// Interface implementation of ::com::sun::star::awt::XUnitConversion


::com::sun::star::awt::Point SAL_CALL VCLXDevice::convertPointToLogic( const ::com::sun::star::awt::Point& aPoint, ::sal_Int16 TargetUnit ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception)
{
    (void)aPoint;
    SolarMutexGuard aGuard;
    if (TargetUnit == com::sun::star::util::MeasureUnit::PERCENT )
    {
        // percentage not allowed here
        throw ::com::sun::star::lang::IllegalArgumentException();
    }

    ::com::sun::star::awt::Point aAWTPoint(0,0);
    // X,Y

    if( mpOutputDevice )
    {
        MapMode aMode(VCLUnoHelper::ConvertToMapModeUnit(TargetUnit));
        ::Point aVCLPoint = VCLUnoHelper::ConvertToVCLPoint(aPoint);
        ::Point aDevPoint = mpOutputDevice->PixelToLogic(aVCLPoint, aMode );
        aAWTPoint = VCLUnoHelper::ConvertToAWTPoint(aDevPoint);
    }

    return aAWTPoint;
}


::com::sun::star::awt::Point SAL_CALL VCLXDevice::convertPointToPixel( const ::com::sun::star::awt::Point& aPoint, ::sal_Int16 SourceUnit ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception)
{
    (void)aPoint;
    SolarMutexGuard aGuard;
    if (SourceUnit == com::sun::star::util::MeasureUnit::PERCENT ||
        SourceUnit == com::sun::star::util::MeasureUnit::PIXEL )
    {
        // pixel or percentage not allowed here
        throw ::com::sun::star::lang::IllegalArgumentException();
    }

    ::com::sun::star::awt::Point aAWTPoint(0,0);

    if( mpOutputDevice )
    {
        MapMode aMode(VCLUnoHelper::ConvertToMapModeUnit(SourceUnit));
        ::Point aVCLPoint = VCLUnoHelper::ConvertToVCLPoint(aPoint);
        ::Point aDevPoint = mpOutputDevice->LogicToPixel(aVCLPoint, aMode );
        aAWTPoint = VCLUnoHelper::ConvertToAWTPoint(aDevPoint);
    }

    return aAWTPoint;
}

::com::sun::star::awt::Size SAL_CALL VCLXDevice::convertSizeToLogic( const ::com::sun::star::awt::Size& aSize, ::sal_Int16 TargetUnit ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception)
{
    (void)aSize;
    SolarMutexGuard aGuard;
    if (TargetUnit == com::sun::star::util::MeasureUnit::PERCENT)
    {
        // percentage not allowed here
        throw ::com::sun::star::lang::IllegalArgumentException();
    }

    ::com::sun::star::awt::Size aAWTSize(0,0);
    // Width, Height


    if( mpOutputDevice )
    {
        MapMode aMode(VCLUnoHelper::ConvertToMapModeUnit(TargetUnit));
        ::Size aVCLSize = VCLUnoHelper::ConvertToVCLSize(aSize);
        ::Size aDevSz = mpOutputDevice->PixelToLogic(aVCLSize, aMode );
        aAWTSize = VCLUnoHelper::ConvertToAWTSize(aDevSz);
    }

    return aAWTSize;
}

::com::sun::star::awt::Size SAL_CALL VCLXDevice::convertSizeToPixel( const ::com::sun::star::awt::Size& aSize, ::sal_Int16 SourceUnit ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception)
{
    (void)aSize;
    SolarMutexGuard aGuard;
    if (SourceUnit == com::sun::star::util::MeasureUnit::PERCENT ||
        SourceUnit == com::sun::star::util::MeasureUnit::PIXEL)
    {
        // pixel or percentage not allowed here
        throw ::com::sun::star::lang::IllegalArgumentException();
    }

    ::com::sun::star::awt::Size aAWTSize(0,0);
    // Width, Height
    if( mpOutputDevice )
    {
        MapMode aMode(VCLUnoHelper::ConvertToMapModeUnit(SourceUnit));
        ::Size aVCLSize = VCLUnoHelper::ConvertToVCLSize(aSize);
        ::Size aDevSz = mpOutputDevice->LogicToPixel(aVCLSize, aMode );
        aAWTSize = VCLUnoHelper::ConvertToAWTSize(aDevSz);
    }

    return aAWTSize;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
