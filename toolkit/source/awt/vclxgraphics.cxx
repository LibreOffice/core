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


#include <toolkit/awt/vclxgraphics.hxx>
#include <toolkit/awt/vclxdevice.hxx>
#include <toolkit/helper/macros.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <rtl/uuid.h>

#include <vcl/svapp.hxx>
#include <vcl/outdev.hxx>
#include <vcl/gradient.hxx>
#include <tools/debug.hxx>


//  ----------------------------------------------------
//  class VCLXGraphics
//  ----------------------------------------------------

// ::com::sun::star::uno::XInterface
::com::sun::star::uno::Any VCLXGraphics::queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aRet = ::cppu::queryInterface( rType,
                                        (static_cast< ::com::sun::star::awt::XGraphics* >(this)),
                                        (static_cast< ::com::sun::star::lang::XTypeProvider* >(this)),
                                        (static_cast< ::com::sun::star::lang::XUnoTunnel* >(this)) );
    return (aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType ));
}

// ::com::sun::star::lang::XUnoTunnel
IMPL_XUNOTUNNEL( VCLXGraphics )

// ::com::sun::star::lang::XTypeProvider
IMPL_XTYPEPROVIDER_START( VCLXGraphics )
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XGraphics>* ) NULL )
IMPL_XTYPEPROVIDER_END

VCLXGraphics::VCLXGraphics()
{
    mpOutputDevice = NULL;
    mpClipRegion = NULL;
}

VCLXGraphics::~VCLXGraphics()
{
    VCLXGraphicsList_impl* pLst = mpOutputDevice ? mpOutputDevice->GetUnoGraphicsList() : NULL;
    if ( pLst )
    {
        for( VCLXGraphicsList_impl::iterator it = pLst->begin(); it != pLst->end(); ++it )
        {
            if( *it == this ) {
                pLst->erase( it );
                break;
            }
        }
    }

    delete mpClipRegion;
}

void VCLXGraphics::SetOutputDevice( OutputDevice* pOutDev )
{
    mpOutputDevice = pOutDev;
    mxDevice = NULL;
}

void VCLXGraphics::Init( OutputDevice* pOutDev )
{
    DBG_ASSERT( !mpOutputDevice, "VCLXGraphics::Init allready has pOutDev !" );
    mpOutputDevice  = pOutDev;

    maFont          = mpOutputDevice->GetFont();
    maTextColor     = COL_BLACK;
    maTextFillColor = COL_TRANSPARENT;
    maLineColor     = COL_BLACK;
    maFillColor     = COL_WHITE;
    meRasterOp      = ROP_OVERPAINT;
    mpClipRegion    = NULL;

    // Register at OutputDevice
    VCLXGraphicsList_impl* pLst = mpOutputDevice->GetUnoGraphicsList();
    if ( !pLst )
        pLst = mpOutputDevice->CreateUnoGraphicsList();
    pLst->push_back( this );
}

void VCLXGraphics::InitOutputDevice( sal_uInt16 nFlags )
{
    if(mpOutputDevice)
    {
        SolarMutexGuard aVclGuard;

        if ( nFlags & INITOUTDEV_FONT )
        {
            mpOutputDevice->SetFont( maFont );
            mpOutputDevice->SetTextColor( maTextColor );
            mpOutputDevice->SetTextFillColor( maTextFillColor );
        }

        if ( nFlags & INITOUTDEV_COLORS )
        {
            mpOutputDevice->SetLineColor( maLineColor );
            mpOutputDevice->SetFillColor( maFillColor );
        }

        if ( nFlags & INITOUTDEV_RASTEROP )
        {
            mpOutputDevice->SetRasterOp( meRasterOp );
        }

        if ( nFlags & INITOUTDEV_CLIPREGION )
        {
            if( mpClipRegion )
                mpOutputDevice->SetClipRegion( *mpClipRegion );
            else
                mpOutputDevice->SetClipRegion();
        }
    }
}

::com::sun::star::uno::Reference< ::com::sun::star::awt::XDevice > VCLXGraphics::getDevice() throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    if( !mxDevice.is() && mpOutputDevice )
    {
        VCLXDevice* pDev = new VCLXDevice;
        pDev->SetOutputDevice( mpOutputDevice );
        mxDevice = pDev;
    }
    return mxDevice;
}

::com::sun::star::awt::SimpleFontMetric VCLXGraphics::getFontMetric() throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    ::com::sun::star::awt::SimpleFontMetric aM;
    if( mpOutputDevice )
    {
        mpOutputDevice->SetFont( maFont );
        aM = VCLUnoHelper::CreateFontMetric( mpOutputDevice->GetFontMetric() );
    }
    return aM;
}

void VCLXGraphics::setFont( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XFont >& rxFont ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    maFont = VCLUnoHelper::CreateFont( rxFont );
}

void VCLXGraphics::selectFont( const ::com::sun::star::awt::FontDescriptor& rDescription ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    maFont = VCLUnoHelper::CreateFont( rDescription, Font() );
}

void VCLXGraphics::setTextColor( sal_Int32 nColor ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    maTextColor = Color( (sal_uInt32)nColor );
}

void VCLXGraphics::setTextFillColor( sal_Int32 nColor ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    maTextFillColor = Color( (sal_uInt32)nColor );
}

void VCLXGraphics::setLineColor( sal_Int32 nColor ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    maLineColor = Color( (sal_uInt32)nColor );
}

void VCLXGraphics::setFillColor( sal_Int32 nColor ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    maFillColor = Color( (sal_uInt32)nColor );
}

void VCLXGraphics::setRasterOp( ::com::sun::star::awt::RasterOperation eROP ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    meRasterOp = (RasterOp)eROP;
}

void VCLXGraphics::setClipRegion( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XRegion >& rxRegion ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    delete mpClipRegion;
    if ( rxRegion.is() )
        mpClipRegion = new Region( VCLUnoHelper::GetRegion( rxRegion ) );
    else
        mpClipRegion = NULL;
}

void VCLXGraphics::intersectClipRegion( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XRegion >& rxRegion ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    if ( rxRegion.is() )
    {
        Region aRegion( VCLUnoHelper::GetRegion( rxRegion ) );
        if ( !mpClipRegion )
            mpClipRegion = new Region( aRegion );
        else
            mpClipRegion->Intersect( aRegion );
    }
}

void VCLXGraphics::push(  ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;


    if( mpOutputDevice )
        mpOutputDevice->Push();
}

void VCLXGraphics::pop(  ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;


    if( mpOutputDevice )
        mpOutputDevice->Pop();
}

void VCLXGraphics::copy( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XDevice >& rxSource, sal_Int32 nSourceX, sal_Int32 nSourceY, sal_Int32 nSourceWidth, sal_Int32 nSourceHeight, sal_Int32 nDestX, sal_Int32 nDestY, sal_Int32 nDestWidth, sal_Int32 nDestHeight ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    if ( mpOutputDevice )
    {
        VCLXDevice* pFromDev = VCLXDevice::GetImplementation( rxSource );
        DBG_ASSERT( pFromDev, "VCLXGraphics::copy - invalid device" );
        if ( pFromDev )
        {
            InitOutputDevice( INITOUTDEV_CLIPREGION|INITOUTDEV_RASTEROP );
            mpOutputDevice->DrawOutDev( Point( nDestX, nDestY ), Size( nDestWidth, nDestHeight ),
                                    Point( nSourceX, nSourceY ), Size( nSourceWidth, nSourceHeight ), *pFromDev->GetOutputDevice() );
        }
    }
}

void VCLXGraphics::draw( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XDisplayBitmap >& rxBitmapHandle, sal_Int32 nSourceX, sal_Int32 nSourceY, sal_Int32 nSourceWidth, sal_Int32 nSourceHeight, sal_Int32 nDestX, sal_Int32 nDestY, sal_Int32 nDestWidth, sal_Int32 nDestHeight ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    if( mpOutputDevice )
    {
        InitOutputDevice( INITOUTDEV_CLIPREGION|INITOUTDEV_RASTEROP);
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XBitmap > xBitmap( rxBitmapHandle, ::com::sun::star::uno::UNO_QUERY );
        BitmapEx aBmpEx = VCLUnoHelper::GetBitmap( xBitmap );

        Point aPos(nDestX - nSourceX, nDestY - nSourceY);
          Size aSz = aBmpEx.GetSizePixel();

        if(nDestWidth != nSourceWidth)
        {
            float zoomX = (float)nDestWidth / (float)nSourceWidth;
            aSz.Width() = (long) ((float)aSz.Width() * zoomX);
        }

        if(nDestHeight != nSourceHeight)
        {
            float zoomY = (float)nDestHeight / (float)nSourceHeight;
            aSz.Height() = (long) ((float)aSz.Height() * zoomY);
        }

        if(nSourceX || nSourceY || aSz.Width() != nSourceWidth || aSz.Height() != nSourceHeight)
            mpOutputDevice->IntersectClipRegion(Region(Rectangle(nDestX, nDestY, nDestX + nDestWidth - 1, nDestY + nDestHeight - 1)));

        mpOutputDevice->DrawBitmapEx( aPos, aSz, aBmpEx );
    }
}

void VCLXGraphics::drawPixel( sal_Int32 x, sal_Int32 y ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    if( mpOutputDevice )
    {
        InitOutputDevice( INITOUTDEV_CLIPREGION|INITOUTDEV_RASTEROP|INITOUTDEV_COLORS );
        mpOutputDevice->DrawPixel( Point( x, y ) );
    }
}

void VCLXGraphics::drawLine( sal_Int32 x1, sal_Int32 y1, sal_Int32 x2, sal_Int32 y2 ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    if( mpOutputDevice )
    {
        InitOutputDevice( INITOUTDEV_CLIPREGION|INITOUTDEV_RASTEROP|INITOUTDEV_COLORS );
        mpOutputDevice->DrawLine( Point( x1, y1 ), Point( x2, y2 ) );
    }
}

void VCLXGraphics::drawRect( sal_Int32 x, sal_Int32 y, sal_Int32 width, sal_Int32 height ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    if( mpOutputDevice )
    {
        InitOutputDevice( INITOUTDEV_CLIPREGION|INITOUTDEV_RASTEROP|INITOUTDEV_COLORS );
        mpOutputDevice->DrawRect( Rectangle( Point( x, y ), Size( width, height ) ) );
    }
}

void VCLXGraphics::drawRoundedRect( sal_Int32 x, sal_Int32 y, sal_Int32 width, sal_Int32 height, sal_Int32 nHorzRound, sal_Int32 nVertRound ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    if( mpOutputDevice )
    {
        InitOutputDevice( INITOUTDEV_CLIPREGION|INITOUTDEV_RASTEROP|INITOUTDEV_COLORS );
        mpOutputDevice->DrawRect( Rectangle( Point( x, y ), Size( width, height ) ), nHorzRound, nVertRound );
    }
}

void VCLXGraphics::drawPolyLine( const ::com::sun::star::uno::Sequence< sal_Int32 >& DataX, const ::com::sun::star::uno::Sequence< sal_Int32 >& DataY ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    if( mpOutputDevice )
    {
        InitOutputDevice( INITOUTDEV_CLIPREGION|INITOUTDEV_RASTEROP|INITOUTDEV_COLORS );
        mpOutputDevice->DrawPolyLine( VCLUnoHelper::CreatePolygon( DataX, DataY ) );
    }
}

void VCLXGraphics::drawPolygon( const ::com::sun::star::uno::Sequence< sal_Int32 >& DataX, const ::com::sun::star::uno::Sequence< sal_Int32 >& DataY ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    if( mpOutputDevice )
    {
        InitOutputDevice( INITOUTDEV_CLIPREGION|INITOUTDEV_RASTEROP|INITOUTDEV_COLORS );
        mpOutputDevice->DrawPolygon( VCLUnoHelper::CreatePolygon( DataX, DataY ) );
    }
}

void VCLXGraphics::drawPolyPolygon( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< sal_Int32 > >& DataX, const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< sal_Int32 > >& DataY ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    if( mpOutputDevice )
    {
        InitOutputDevice( INITOUTDEV_CLIPREGION|INITOUTDEV_RASTEROP|INITOUTDEV_COLORS );
        sal_uInt16 nPolys = (sal_uInt16) DataX.getLength();
        PolyPolygon aPolyPoly( nPolys );
        for ( sal_uInt16 n = 0; n < nPolys; n++ )
            aPolyPoly[n] = VCLUnoHelper::CreatePolygon( DataX.getConstArray()[n], DataY.getConstArray()[n] );

        mpOutputDevice->DrawPolyPolygon( aPolyPoly );
    }
}

void VCLXGraphics::drawEllipse( sal_Int32 x, sal_Int32 y, sal_Int32 width, sal_Int32 height ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    if( mpOutputDevice )
    {
        InitOutputDevice( INITOUTDEV_CLIPREGION|INITOUTDEV_RASTEROP|INITOUTDEV_COLORS );
        mpOutputDevice->DrawEllipse( Rectangle( Point( x, y ), Size( width, height ) ) );
    }
}

void VCLXGraphics::drawArc( sal_Int32 x, sal_Int32 y, sal_Int32 width, sal_Int32 height, sal_Int32 x1, sal_Int32 y1, sal_Int32 x2, sal_Int32 y2 ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    if( mpOutputDevice )
    {
        InitOutputDevice( INITOUTDEV_CLIPREGION|INITOUTDEV_RASTEROP|INITOUTDEV_COLORS );
        mpOutputDevice->DrawArc( Rectangle( Point( x, y ), Size( width, height ) ), Point( x1, y1 ), Point( x2, y2 ) );
    }
}

void VCLXGraphics::drawPie( sal_Int32 x, sal_Int32 y, sal_Int32 width, sal_Int32 height, sal_Int32 x1, sal_Int32 y1, sal_Int32 x2, sal_Int32 y2 ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    if( mpOutputDevice )
    {
        InitOutputDevice( INITOUTDEV_CLIPREGION|INITOUTDEV_RASTEROP|INITOUTDEV_COLORS );
        mpOutputDevice->DrawPie( Rectangle( Point( x, y ), Size( width, height ) ), Point( x1, y1 ), Point( x2, y2 ) );
    }
}

void VCLXGraphics::drawChord( sal_Int32 x, sal_Int32 y, sal_Int32 width, sal_Int32 height, sal_Int32 x1, sal_Int32 y1, sal_Int32 x2, sal_Int32 y2 ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    if( mpOutputDevice )
    {
        InitOutputDevice( INITOUTDEV_CLIPREGION|INITOUTDEV_RASTEROP|INITOUTDEV_COLORS );
        mpOutputDevice->DrawChord( Rectangle( Point( x, y ), Size( width, height ) ), Point( x1, y1 ), Point( x2, y2 ) );
    }
}

void VCLXGraphics::drawGradient( sal_Int32 x, sal_Int32 y, sal_Int32 width, sal_Int32 height, const ::com::sun::star::awt::Gradient& rGradient ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    if( mpOutputDevice )
    {
        InitOutputDevice( INITOUTDEV_CLIPREGION|INITOUTDEV_RASTEROP|INITOUTDEV_COLORS );
        Gradient aGradient((GradientStyle)rGradient.Style, rGradient.StartColor, rGradient.EndColor);
        aGradient.SetAngle(rGradient.Angle);
        aGradient.SetBorder(rGradient.Border);
        aGradient.SetOfsX(rGradient.XOffset);
        aGradient.SetOfsY(rGradient.YOffset);
        aGradient.SetStartIntensity(rGradient.StartIntensity);
        aGradient.SetEndIntensity(rGradient.EndIntensity);
        aGradient.SetSteps(rGradient.StepCount);
        mpOutputDevice->DrawGradient( Rectangle( Point( x, y ), Size( width, height ) ), aGradient );
    }
}

void VCLXGraphics::drawText( sal_Int32 x, sal_Int32 y, const ::rtl::OUString& rText ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    if( mpOutputDevice )
    {
        InitOutputDevice( INITOUTDEV_CLIPREGION|INITOUTDEV_RASTEROP|INITOUTDEV_COLORS |INITOUTDEV_FONT);
        mpOutputDevice->DrawText( Point( x, y ), rText );
    }
}

void VCLXGraphics::drawTextArray( sal_Int32 x, sal_Int32 y, const ::rtl::OUString& rText, const ::com::sun::star::uno::Sequence< sal_Int32 >& rLongs ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    if( mpOutputDevice )
    {
        InitOutputDevice( INITOUTDEV_CLIPREGION|INITOUTDEV_RASTEROP|INITOUTDEV_COLORS|INITOUTDEV_FONT );
        mpOutputDevice->DrawTextArray( Point( x, y ), rText, rLongs.getConstArray() );
    }
}




/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
