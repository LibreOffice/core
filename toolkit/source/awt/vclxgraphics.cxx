/*************************************************************************
 *
 *  $RCSfile: vclxgraphics.cxx,v $
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

#define SMART_UNO_GENERATION    // Um einige Funktionen in VCL freizuschalten

#include <toolkit/awt/vclxgraphics.hxx>
#include <toolkit/awt/vclxdevice.hxx>
#include <toolkit/helper/macros.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <rtl/memory.h>
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
                                        SAL_STATIC_CAST( ::com::sun::star::awt::XGraphics*, this ),
                                        SAL_STATIC_CAST( ::com::sun::star::lang::XTypeProvider*, this ),
                                        SAL_STATIC_CAST( ::com::sun::star::lang::XUnoTunnel*, this ) );
    return (aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType ));
}

// ::com::sun::star::lang::XUnoTunnel
IMPL_XUNOTUNNEL( VCLXGraphics )

// ::com::sun::star::lang::XTypeProvider
IMPL_XTYPEPROVIDER_START( VCLXGraphics )
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XGraphics>* ) NULL )
IMPL_XTYPEPROVIDER_END

VCLXGraphics::VCLXGraphics() : mrMutex( Application::GetSolarMutex() )
{
    mpOutputDevice = NULL;
    mpClipRegion = NULL;
}

VCLXGraphics::~VCLXGraphics()
{
    List* pLst = mpOutputDevice ? mpOutputDevice->GetUnoGraphicsList() : NULL;
    if ( pLst )
        pLst->Remove( this );

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
    List* pLst = mpOutputDevice->GetUnoGraphicsList();
    if ( !pLst )
        pLst = mpOutputDevice->CreateUnoGraphicsList();
    pLst->Insert( this, LIST_APPEND );
}

void VCLXGraphics::InitOutputDevice( sal_uInt16 nFlags )
{
    if(mpOutputDevice)
    {
        NAMESPACE_VOS(OGuard) aVclGuard( Application::GetSolarMutex()  );

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
    ::vos::OGuard aGuard( GetMutex() );

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
    ::vos::OGuard aGuard( GetMutex() );

    ::com::sun::star::awt::SimpleFontMetric aM;
    if( mpOutputDevice )
    {
        mpOutputDevice->SetFont( maFont );
        FontMetric aFMetric = mpOutputDevice->GetFontMetric();

        aM.Ascent = aFMetric.GetAscent();
        aM.Descent = aFMetric.GetDescent();
        aM.Leading = aFMetric.GetLeading();
        aM.Slant = aFMetric.GetSlant();
        aM.FirstChar = aFMetric.getFirstChar();
        aM.LastChar = aFMetric.getLastChar();
    }
    return aM;
}

void VCLXGraphics::setFont( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XFont >& rxFont ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    maFont = VCLUnoHelper::CreateFont( rxFont );
}

void VCLXGraphics::selectFont( const ::com::sun::star::awt::FontDescriptor& rDescription ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    maFont = VCLUnoHelper::CreateFont( rDescription, Font() );
}

void VCLXGraphics::setTextColor( sal_Int32 nColor ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    maTextColor = Color( (sal_uInt32)nColor );
}

void VCLXGraphics::setTextFillColor( sal_Int32 nColor ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    maTextFillColor = Color( (sal_uInt32)nColor );
}

void VCLXGraphics::setLineColor( sal_Int32 nColor ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    maLineColor = Color( (sal_uInt32)nColor );
}

void VCLXGraphics::setFillColor( sal_Int32 nColor ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    maFillColor = Color( (sal_uInt32)nColor );
}

void VCLXGraphics::setRasterOp( ::com::sun::star::awt::RasterOperation eROP ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    meRasterOp = (RasterOp)eROP;
}

void VCLXGraphics::setClipRegion( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XRegion >& rxRegion ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    delete mpClipRegion;
    if ( rxRegion.is() )
        mpClipRegion = new Region( VCLUnoHelper::GetRegion( rxRegion ) );
    else
        mpClipRegion = NULL;
}

void VCLXGraphics::intersectClipRegion( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XRegion >& rxRegion ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

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
    ::vos::OGuard aGuard( GetMutex() );


    if( mpOutputDevice )
        mpOutputDevice->Push();
}

void VCLXGraphics::pop(  ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );


    if( mpOutputDevice )
        mpOutputDevice->Pop();
}

void VCLXGraphics::copy( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XDevice >& rxSource, sal_Int32 nSourceX, sal_Int32 nSourceY, sal_Int32 nSourceWidth, sal_Int32 nSourceHeight, sal_Int32 nDestX, sal_Int32 nDestY, sal_Int32 nDestWidth, sal_Int32 nDestHeight ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

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
    ::vos::OGuard aGuard( GetMutex() );

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
    ::vos::OGuard aGuard( GetMutex() );

    if( mpOutputDevice )
    {
        InitOutputDevice( INITOUTDEV_CLIPREGION|INITOUTDEV_RASTEROP|INITOUTDEV_COLORS );
        mpOutputDevice->DrawPixel( Point( x, y ) );
    }
}

void VCLXGraphics::drawLine( sal_Int32 x1, sal_Int32 y1, sal_Int32 x2, sal_Int32 y2 ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    if( mpOutputDevice )
    {
        InitOutputDevice( INITOUTDEV_CLIPREGION|INITOUTDEV_RASTEROP|INITOUTDEV_COLORS );
        mpOutputDevice->DrawLine( Point( x1, y1 ), Point( x2, y2 ) );
    }
}

void VCLXGraphics::drawRect( sal_Int32 x, sal_Int32 y, sal_Int32 width, sal_Int32 height ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    if( mpOutputDevice )
    {
        InitOutputDevice( INITOUTDEV_CLIPREGION|INITOUTDEV_RASTEROP|INITOUTDEV_COLORS );
        mpOutputDevice->DrawRect( Rectangle( Point( x, y ), Size( width, height ) ) );
    }
}

void VCLXGraphics::drawRoundedRect( sal_Int32 x, sal_Int32 y, sal_Int32 width, sal_Int32 height, sal_Int32 nHorzRound, sal_Int32 nVertRound ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    if( mpOutputDevice )
    {
        InitOutputDevice( INITOUTDEV_CLIPREGION|INITOUTDEV_RASTEROP|INITOUTDEV_COLORS );
        mpOutputDevice->DrawRect( Rectangle( Point( x, y ), Size( width, height ) ), nHorzRound, nVertRound );
    }
}

void VCLXGraphics::drawPolyLine( const ::com::sun::star::uno::Sequence< sal_Int32 >& DataX, const ::com::sun::star::uno::Sequence< sal_Int32 >& DataY ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    if( mpOutputDevice )
    {
        InitOutputDevice( INITOUTDEV_CLIPREGION|INITOUTDEV_RASTEROP|INITOUTDEV_COLORS );
        mpOutputDevice->DrawPolyLine( VCLUnoHelper::CreatePolygon( DataX, DataY ) );
    }
}

void VCLXGraphics::drawPolygon( const ::com::sun::star::uno::Sequence< sal_Int32 >& DataX, const ::com::sun::star::uno::Sequence< sal_Int32 >& DataY ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    if( mpOutputDevice )
    {
        InitOutputDevice( INITOUTDEV_CLIPREGION|INITOUTDEV_RASTEROP|INITOUTDEV_COLORS );
        mpOutputDevice->DrawPolygon( VCLUnoHelper::CreatePolygon( DataX, DataY ) );
    }
}

void VCLXGraphics::drawPolyPolygon( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< sal_Int32 > >& DataX, const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< sal_Int32 > >& DataY ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    if( mpOutputDevice )
    {
        InitOutputDevice( INITOUTDEV_CLIPREGION|INITOUTDEV_RASTEROP|INITOUTDEV_COLORS );
        sal_Int32 nPolys = (sal_uInt16) DataX.getLength();
        PolyPolygon aPolyPoly( nPolys );
        for ( sal_Int32 n = 0; n < nPolys; n++ )
            aPolyPoly[n] = VCLUnoHelper::CreatePolygon( DataX.getConstArray()[n], DataY.getConstArray()[n] );

        mpOutputDevice->DrawPolyPolygon( aPolyPoly );
    }
}

void VCLXGraphics::drawEllipse( sal_Int32 x, sal_Int32 y, sal_Int32 width, sal_Int32 height ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    if( mpOutputDevice )
    {
        InitOutputDevice( INITOUTDEV_CLIPREGION|INITOUTDEV_RASTEROP|INITOUTDEV_COLORS );
        mpOutputDevice->DrawEllipse( Rectangle( Point( x, y ), Size( width, height ) ) );
    }
}

void VCLXGraphics::drawArc( sal_Int32 x, sal_Int32 y, sal_Int32 width, sal_Int32 height, sal_Int32 x1, sal_Int32 y1, sal_Int32 x2, sal_Int32 y2 ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    if( mpOutputDevice )
    {
        InitOutputDevice( INITOUTDEV_CLIPREGION|INITOUTDEV_RASTEROP|INITOUTDEV_COLORS );
        mpOutputDevice->DrawArc( Rectangle( Point( x, y ), Size( width, height ) ), Point( x1, y1 ), Point( x2, y2 ) );
    }
}

void VCLXGraphics::drawPie( sal_Int32 x, sal_Int32 y, sal_Int32 width, sal_Int32 height, sal_Int32 x1, sal_Int32 y1, sal_Int32 x2, sal_Int32 y2 ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    if( mpOutputDevice )
    {
        InitOutputDevice( INITOUTDEV_CLIPREGION|INITOUTDEV_RASTEROP|INITOUTDEV_COLORS );
        mpOutputDevice->DrawPie( Rectangle( Point( x, y ), Size( width, height ) ), Point( x1, y1 ), Point( x2, y2 ) );
    }
}

void VCLXGraphics::drawChord( sal_Int32 x, sal_Int32 y, sal_Int32 width, sal_Int32 height, sal_Int32 x1, sal_Int32 y1, sal_Int32 x2, sal_Int32 y2 ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    if( mpOutputDevice )
    {
        InitOutputDevice( INITOUTDEV_CLIPREGION|INITOUTDEV_RASTEROP|INITOUTDEV_COLORS );
        mpOutputDevice->DrawChord( Rectangle( Point( x, y ), Size( width, height ) ), Point( x1, y1 ), Point( x2, y2 ) );
    }
}

void VCLXGraphics::drawGradient( sal_Int32 x, sal_Int32 y, sal_Int32 width, sal_Int32 height, const ::com::sun::star::awt::Gradient& rGradient ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

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
    ::vos::OGuard aGuard( GetMutex() );

    if( mpOutputDevice )
    {
        InitOutputDevice( INITOUTDEV_CLIPREGION|INITOUTDEV_RASTEROP|INITOUTDEV_COLORS |INITOUTDEV_FONT);
        mpOutputDevice->DrawText( Point( x, y ), rText );
    }
}

void VCLXGraphics::drawTextArray( sal_Int32 x, sal_Int32 y, const ::rtl::OUString& rText, const ::com::sun::star::uno::Sequence< sal_Int32 >& rLongs ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    if( mpOutputDevice )
    {
        InitOutputDevice( INITOUTDEV_CLIPREGION|INITOUTDEV_RASTEROP|INITOUTDEV_COLORS|INITOUTDEV_FONT );
        mpOutputDevice->DrawTextArray( Point( x, y ), rText, rLongs.getConstArray() );
    }
}




