/*************************************************************************
 *
 *  $RCSfile: vclxdevice.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:02:08 $
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

#ifndef _COM_SUN_STAR_AWT_DEVICECAPABILITY_HPP_
#include <com/sun/star/awt/DeviceCapability.hpp>
#endif

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
#ifdef DEBUG
    nDummy = 0x12345678;
    pDummy = (void*)0x12345678;
#endif
}

VCLXDevice::~VCLXDevice()
{
#ifdef DEBUG
    nDummy = 0xbaadbaad;
    pDummy = (void*)0xbaadbaad;
#endif
}

void VCLXDevice::DestroyOutputDevice()
{
    delete mpOutputDevice;
    mpOutputDevice = NULL;
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
        sal_uInt16 nFonts = mpOutputDevice->GetDevFontCount();
        if ( nFonts )
        {
            aFonts = ::com::sun::star::uno::Sequence< ::com::sun::star::awt::FontDescriptor>( nFonts );
            ::com::sun::star::awt::FontDescriptor* pFonts = aFonts.getArray();
            for ( sal_uInt16 n = 0; n < nFonts; n++ )
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



