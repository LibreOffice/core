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
#include <com/sun/star/lang/IllegalArgumentException.hpp>

#include <toolkit/awt/vclxdevice.hxx>
#include <toolkit/awt/vclxfont.hxx>
#include <toolkit/awt/vclxbitmap.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <toolkit/helper/macros.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <cppuhelper/queryinterface.hxx>

#include <rtl/uuid.h>

#include <vcl/svapp.hxx>
#include <vcl/outdev.hxx>
#include <vcl/window.hxx>
#include <vcl/print.hxx>
#include <vcl/virdev.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/font.hxx>
#include <vcl/metric.hxx>

//  class VCLXDevice

VCLXDevice::VCLXDevice()
{
}

VCLXDevice::~VCLXDevice()
{
    //TODO: why was this empty, and everything done in ~VCLXVirtualDevice?
    SolarMutexGuard g;
    mpOutputDevice.reset();
}

// css::uno::XInterface
css::uno::Any VCLXDevice::queryInterface( const css::uno::Type & rType )
{
    css::uno::Any aRet = ::cppu::queryInterface( rType,
                                        static_cast< css::awt::XDevice* >(this),
                                        static_cast< css::lang::XUnoTunnel* >(this),
                                        static_cast< css::lang::XTypeProvider* >(this),
                                        static_cast< css::awt::XUnitConversion* >(this) );
    return (aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType ));
}

// css::lang::XUnoTunnel
IMPL_XUNOTUNNEL( VCLXDevice )

IMPL_IMPLEMENTATION_ID( VCLXDevice )

// css::lang::XTypeProvider
css::uno::Sequence< css::uno::Type > VCLXDevice::getTypes()
{
    static const css::uno::Sequence< css::uno::Type > aTypeList {
        cppu::UnoType<css::lang::XTypeProvider>::get(),
        cppu::UnoType<css::awt::XDevice>::get(),
        cppu::UnoType<css::awt::XUnitConversion>::get()
    };
    return aTypeList;
}


// css::awt::XDevice,
css::uno::Reference< css::awt::XGraphics > VCLXDevice::createGraphics(  )
{
    SolarMutexGuard aGuard;

    css::uno::Reference< css::awt::XGraphics > xRef;

    if ( mpOutputDevice )
        xRef = mpOutputDevice->CreateUnoGraphics();

    return xRef;
}

css::uno::Reference< css::awt::XDevice > VCLXDevice::createDevice( sal_Int32 nWidth, sal_Int32 nHeight )
{
    SolarMutexGuard aGuard;

    css::uno::Reference< css::awt::XDevice >  xRef;
    if ( GetOutputDevice() )
    {
        VCLXVirtualDevice* pVDev = new VCLXVirtualDevice;
        VclPtrInstance<VirtualDevice> pVclVDev( *GetOutputDevice() );
        pVclVDev->SetOutputSizePixel( Size( nWidth, nHeight ) );
        pVDev->SetVirtualDevice( pVclVDev );
        xRef = pVDev;
    }
    return xRef;
}

css::awt::DeviceInfo VCLXDevice::getInfo()
{
    SolarMutexGuard aGuard;

    css::awt::DeviceInfo aInfo;

    if( mpOutputDevice )
    {
        Size aDevSz;
        OutDevType eDevType = mpOutputDevice->GetOutDevType();
        if ( eDevType == OUTDEV_WINDOW )
        {
            aDevSz = static_cast<vcl::Window*>(mpOutputDevice.get())->GetSizePixel();
            static_cast<vcl::Window*>(mpOutputDevice.get())->GetBorder( aInfo.LeftInset, aInfo.TopInset, aInfo.RightInset, aInfo.BottomInset );
        }
        else if ( eDevType == OUTDEV_PRINTER )
        {
            aDevSz = static_cast<Printer*>(mpOutputDevice.get())->GetPaperSizePixel();
            Size aOutSz = mpOutputDevice->GetOutputSizePixel();
            Point aOffset = static_cast<Printer*>(mpOutputDevice.get())->GetPageOffset();
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

        Size aTmpSz = mpOutputDevice->LogicToPixel( Size( 1000, 1000 ), MapMode( MapUnit::MapCM ) );
        aInfo.PixelPerMeterX = aTmpSz.Width()/10;
        aInfo.PixelPerMeterY = aTmpSz.Height()/10;

        aInfo.BitsPerPixel = mpOutputDevice->GetBitCount();

        aInfo.Capabilities = 0;
        if ( mpOutputDevice->GetOutDevType() != OUTDEV_PRINTER )
            aInfo.Capabilities = css::awt::DeviceCapability::RASTEROPERATIONS|css::awt::DeviceCapability::GETBITS;
    }

    return aInfo;
}

css::uno::Sequence< css::awt::FontDescriptor > VCLXDevice::getFontDescriptors(  )
{
    SolarMutexGuard aGuard;

    css::uno::Sequence< css::awt::FontDescriptor> aFonts;
    if( mpOutputDevice )
    {
        int nFonts = mpOutputDevice->GetDevFontCount();
        if ( nFonts )
        {
            aFonts = css::uno::Sequence< css::awt::FontDescriptor>( nFonts );
            css::awt::FontDescriptor* pFonts = aFonts.getArray();
            for ( int n = 0; n < nFonts; n++ )
                pFonts[n] = VCLUnoHelper::CreateFontDescriptor( mpOutputDevice->GetDevFont( n ) );
        }
    }
    return aFonts;
}

css::uno::Reference< css::awt::XFont > VCLXDevice::getFont( const css::awt::FontDescriptor& rDescriptor )
{
    SolarMutexGuard aGuard;

    css::uno::Reference< css::awt::XFont >  xRef;
    if( mpOutputDevice )
    {
        VCLXFont* pMetric = new VCLXFont;
        pMetric->Init( *this, VCLUnoHelper::CreateFont( rDescriptor, mpOutputDevice->GetFont() ) );
        xRef = pMetric;
    }
    return xRef;
}

css::uno::Reference< css::awt::XBitmap > VCLXDevice::createBitmap( sal_Int32 nX, sal_Int32 nY, sal_Int32 nWidth, sal_Int32 nHeight )
{
    SolarMutexGuard aGuard;

    css::uno::Reference< css::awt::XBitmap >  xBmp;
    if( mpOutputDevice )
    {
        BitmapEx aBmp = mpOutputDevice->GetBitmapEx( Point( nX, nY ), Size( nWidth, nHeight ) );

        VCLXBitmap* pBmp = new VCLXBitmap;
        pBmp->SetBitmap( aBmp );
        xBmp = pBmp;
    }
    return xBmp;
}

css::uno::Reference< css::awt::XDisplayBitmap > VCLXDevice::createDisplayBitmap( const css::uno::Reference< css::awt::XBitmap >& rxBitmap )
{
    SolarMutexGuard aGuard;

    BitmapEx aBmp = VCLUnoHelper::GetBitmap( rxBitmap );
    VCLXBitmap* pBmp = new VCLXBitmap;
    pBmp->SetBitmap( aBmp );
    css::uno::Reference< css::awt::XDisplayBitmap >  xDBmp = pBmp;
    return xDBmp;
}

VCLXVirtualDevice::~VCLXVirtualDevice()
{
    SolarMutexGuard aGuard;

    mpOutputDevice.disposeAndClear();
}

// Interface implementation of css::awt::XUnitConversion

css::awt::Point SAL_CALL VCLXDevice::convertPointToLogic( const css::awt::Point& aPoint, ::sal_Int16 TargetUnit )
{
    SolarMutexGuard aGuard;
    if (TargetUnit == css::util::MeasureUnit::PERCENT )
    {
        // percentage not allowed here
        throw css::lang::IllegalArgumentException();
    }

    css::awt::Point aAWTPoint(0,0);
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


css::awt::Point SAL_CALL VCLXDevice::convertPointToPixel( const css::awt::Point& aPoint, ::sal_Int16 SourceUnit )
{
    SolarMutexGuard aGuard;
    if (SourceUnit == css::util::MeasureUnit::PERCENT ||
        SourceUnit == css::util::MeasureUnit::PIXEL )
    {
        // pixel or percentage not allowed here
        throw css::lang::IllegalArgumentException();
    }

    css::awt::Point aAWTPoint(0,0);

    if( mpOutputDevice )
    {
        MapMode aMode(VCLUnoHelper::ConvertToMapModeUnit(SourceUnit));
        ::Point aVCLPoint = VCLUnoHelper::ConvertToVCLPoint(aPoint);
        ::Point aDevPoint = mpOutputDevice->LogicToPixel(aVCLPoint, aMode );
        aAWTPoint = VCLUnoHelper::ConvertToAWTPoint(aDevPoint);
    }

    return aAWTPoint;
}

css::awt::Size SAL_CALL VCLXDevice::convertSizeToLogic( const css::awt::Size& aSize, ::sal_Int16 TargetUnit )
{
    SolarMutexGuard aGuard;
    if (TargetUnit == css::util::MeasureUnit::PERCENT)
    {
        // percentage not allowed here
        throw css::lang::IllegalArgumentException();
    }

    css::awt::Size aAWTSize(0,0);
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

css::awt::Size SAL_CALL VCLXDevice::convertSizeToPixel( const css::awt::Size& aSize, ::sal_Int16 SourceUnit )
{
    SolarMutexGuard aGuard;
    if (SourceUnit == css::util::MeasureUnit::PERCENT ||
        SourceUnit == css::util::MeasureUnit::PIXEL)
    {
        // pixel or percentage not allowed here
        throw css::lang::IllegalArgumentException();
    }

    css::awt::Size aAWTSize(0,0);
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
