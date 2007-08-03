/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: salgdiutils.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: hr $ $Date: 2007-08-03 14:02:31 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"

#ifndef _SV_SALGDI_H
#include <salgdi.h>
#endif

#include <basebmp/scanlineformats.hxx>
#include <basebmp/color.hxx>
#include <basegfx/range/b2drectangle.hxx>
#include <basegfx/range/b2irange.hxx>
#include <basegfx/vector/b2ivector.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <boost/bind.hpp>

#include <vcl/svapp.hxx>

#include "saldata.hxx"

// ----------------------------------------------------------------------

void AquaSalGraphics::SetWindowGraphics( CarbonViewRef rView, CarbonWindowRef rWindow, bool bScreenCompatible )
{
    mrView      = rView;
    mrWindow    = rWindow;
    mbScreen    = bScreenCompatible;

    mbWindow    = true;
    mbPrinter   = false;
    mbVirDev    = false;
}

void AquaSalGraphics::SetPrinterGraphics( CGContextRef xContext, long nDPIX, long nDPIY )
{
    mrView      = 0;
    mbScreen    = false;

    mbWindow    = false;
    mbPrinter   = true;
    mbVirDev    = false;

    mrContext   = xContext;
    mnDPIX      = nDPIX;
    mnDPIY      = nDPIY;

    if( mrContext )
    {
        CGContextSetFillColorSpace( mrContext, GetSalData()->mxRGBSpace );
        CGContextSetStrokeColorSpace( mrContext, GetSalData()->mxRGBSpace );
        CGContextSaveGState( mrContext );
        SetState();
    }
}

static void nil_free( void* )
{
}

void AquaSalGraphics::SetVirDevGraphics( CGContextRef xContext, bool bScreenCompatible )
{
    mrView      = 0;
    mbScreen    = bScreenCompatible;

    mbWindow    = false;
    mbPrinter   = false;
    mbVirDev    = true;

    mrContext   = xContext;
    sal_uInt8* pMemory = NULL;
    if( mrContext )
    {
        pMemory = reinterpret_cast<sal_uInt8*>(CGBitmapContextGetData( mrContext ));
        CGContextSetFillColorSpace( mrContext, GetSalData()->mxRGBSpace );
        CGContextSetStrokeColorSpace( mrContext, GetSalData()->mxRGBSpace );
        CGContextSaveGState( mrContext );
        SetState();
    }
    maContextMemory.reset( pMemory, boost::bind( nil_free, _1 ) );
}

// ----------------------------------------------------------------------

void AquaSalGraphics::SetState()
{
    CGContextRestoreGState( mrContext );
    CGContextSaveGState( mrContext );
    // set up clipping area
    if( mrClippingPath )
    {
        CGContextBeginPath( mrContext );                // discard any existing path
        CGContextAddPath( mrContext, mrClippingPath );  // set the current path to the clipping path
        CGContextClip( mrContext );                     // use it for clipping
    }

    // set RGB colorspace and line and fill colors
    CGContextSetFillColor( mrContext, mpFillColor );
    CGContextSetStrokeColor( mrContext, mpLineColor );
    if( mbXORMode && (mbWindow || mbVirDev) )
    {
        CGContextSetBlendMode(mrContext, kCGBlendModeDifference);
        int nWidth  = CGBitmapContextGetWidth(mrContext);
        int nHeight = CGBitmapContextGetHeight(mrContext);
        if( ! maXORDevice )
        {
            maXORDevice = basebmp::createBitmapDevice( basegfx::B2IVector( nWidth, nHeight ),
                                                       mbWindow,
                                                       basebmp::Format::THIRTYTWO_BIT_TC_MASK_ARGB,
                                                       maContextMemory,
                                                       basebmp::PaletteMemorySharedVector() );
            if( mrClippingPath && maClippingRects.size() == 1 )
            {
                /*
                * optimization: the case with only one clip rectangle is quite common
                * in this case it is much cheaper to constrain the XOR device to a smaller
                * area than to clip every pixel with the mask
                */
                maXORClipMask.reset();
                CGRect aBounds( maClippingRects.front() );
                basegfx::B2IRange aRect( aBounds.origin.x, aBounds.origin.y,
                                         aBounds.origin.x+aBounds.size.width,
                                         aBounds.origin.y+aBounds.size.height );
                maXORDevice = basebmp::subsetBitmapDevice( maXORDevice, aRect );
            }
        }
        if( mrClippingPath )
        {
            if( ! maXORClipMask && maClippingRects.size() > 1 )
            {
                maXORClipMask = basebmp::createBitmapDevice( basegfx::B2IVector( nWidth, nHeight ),
                                                             mbWindow,
                                                             basebmp::Format::ONE_BIT_MSB_GREY );
                maXORClipMask->clear( basebmp::Color(0xffffffff) );
                for( std::vector<CGRect>::const_iterator it = maClippingRects.begin(); it != maClippingRects.end(); ++it )
                {
                    basegfx::B2DRectangle aRect( it->origin.x, it->origin.y,
                                                 it->origin.x+it->size.width,
                                                 it->origin.y+it->size.height );
                    maXORClipMask->fillPolyPolygon( basegfx::B2DPolyPolygon( basegfx::tools::createPolygonFromRect( aRect ) ),
                                                    basebmp::Color( 0 ),
                                                    basebmp::DrawMode_PAINT
                                                    );
                }
            }
        }
        else
            maXORClipMask.reset();
    }

    CGContextSetShouldAntialias( mrContext, false );

}

// ----------------------------------------------------------------------

bool AquaSalGraphics::CheckContext()
{
    if( mrWindow != NULL )
    {
        Rect windowBounds;
        GetWindowPortBounds( mrWindow, &windowBounds );
        const unsigned int nWidth = windowBounds.right - windowBounds.left;
        const unsigned int nHeight = windowBounds.bottom - windowBounds.top;

        CGContextRef rReleaseContext = 0;
        unsigned int nReleaseContextWidth = 0;
        unsigned int nReleaseContextHeight = 0;

        boost::shared_array<sal_uInt8> aOldMem;
        if( mrContext )
        {
            nReleaseContextWidth = CGBitmapContextGetWidth(mrContext);
            nReleaseContextHeight = CGBitmapContextGetHeight(mrContext);
            // check if window size changed and we need to create a new bitmap context
            if( (nReleaseContextWidth != nWidth)  || (nReleaseContextHeight != nHeight) )
            {
                rReleaseContext = mrContext;
                mrContext = 0;
                aOldMem = maContextMemory;
                maXORDevice.reset();
                maXORClipMask.reset();
            }
        }

        if( !mrContext )
        {
            maContextMemory.reset( reinterpret_cast<sal_uInt8*>( rtl_allocateMemory( nWidth * 4 * nHeight ) ),
                                   boost::bind( rtl_freeMemory, _1 ) );
            if( maContextMemory )
            {
                mrContext = CGBitmapContextCreate( maContextMemory.get(), nWidth, nHeight, 8, nWidth * 4, GetSalData()->mxRGBSpace, kCGImageAlphaNoneSkipFirst );

                if( mrContext )
                {
                    // copy bitmap data to new context
                    if( rReleaseContext )
                    {
                        CGRect aBounds;
                        aBounds.origin.x = aBounds.origin.y = 0;
                        aBounds.size.width = nReleaseContextWidth;
                        aBounds.size.height = nReleaseContextHeight;
                        CGImageRef xImage = CGBitmapContextCreateImage( rReleaseContext );
                        CGContextDrawImage( mrContext, aBounds, xImage );
                        CGImageRelease(xImage);
                    }

                    CGContextTranslateCTM( mrContext, 0, nHeight );
                    CGContextScaleCTM( mrContext, 1.0, -1.0 );
                    CGContextSetFillColorSpace( mrContext, GetSalData()->mxRGBSpace );
                    CGContextSetStrokeColorSpace( mrContext, GetSalData()->mxRGBSpace );
                    CGContextSaveGState( mrContext );
                    SetState();
                }
                else
                {
                    maContextMemory.reset(); // free memory again
                }
            }
        }

        if( rReleaseContext ) // released memory runs out of scope and is then freed
            CFRelease( rReleaseContext );
    }
    if( mrContext )
    {
        if( mbXORMode )
        {
            if( ! maXORDevice )
                SetState();
        }
        return true;
    }
    else
    {
        AquaLog("<<<WARNING>>> AquaSalGraphics::CheckContext() FAILED!!!!\n" );
        return false;
    }
}


void AquaSalGraphics::RefreshRect(float lX, float lY, float lWidth, float lHeight)
{
    if( ! mbWindow ) // view only on Window graphics
        return;

    AquaLog("-->%s refresh %d - %d - %d - %d\n", __func__, static_cast<int>(lX), static_cast<int>(lY), static_cast<int>(lWidth), static_cast<int>(lHeight));

    // Refresh windows rect content
    HIRect aHIRect;
    aHIRect.origin.x = static_cast<int>(lX);
    aHIRect.origin.y = static_cast<int>(lY);
    aHIRect.size.width = static_cast<int>(lWidth);
    aHIRect.size.height = static_cast<int>(lHeight);
    OSStatus retVal = HIViewSetNeedsDisplayInRect(mrView,&aHIRect,true);
    if (retVal)
        AquaLog( "FIXME: HIViewSetNeedsDisplayInRect returned %d (mrView is %p)\n", (int) retVal, mrView);

    Rect aRect;
    aRect.left   = (short)lX;
    aRect.top    = (short)lY;
    aRect.right  = (short)(lX + lWidth );
    aRect.bottom = (short)(lY + lHeight );
    InvalWindowRect(mrWindow, &aRect);
}

void AquaSalGraphics::Flush()
{
    if( mbWindow )
    {
        UpdateWindow();
    }
}

CGPoint* AquaSalGraphics::makeCGptArray(ULONG nPoints, const SalPoint*  pPtAry)
{
    AquaLog("-->%s\n",__func__);
    CGPoint *CGpoints = new (CGPoint[nPoints]);
    if ( CGpoints )
      {
        for(ULONG i=0;i<nPoints;i++)
          {
            CGpoints[i].x = (float)(pPtAry[i].mnX);
            CGpoints[i].y = (float)(pPtAry[i].mnY);
          }
      }
    return CGpoints;
}


// -----------------------------------------------------------------------

void AquaSalGraphics::UpdateWindow()
{
    if( mrContext != NULL && mrWindow != NULL )
    {
        SetPortWindowPort(mrWindow);
        CGContextRef xWindowContext = 0;
        if( noErr == QDBeginCGContext (GetWindowPort (mrWindow), &xWindowContext))
        {
            Rect windowBounds;
            GetWindowPortBounds( mrWindow, &windowBounds);
            CGImageRef xImage = CGBitmapContextCreateImage( mrContext );
            CGContextDrawImage(xWindowContext, CGRectMake(windowBounds.left, windowBounds.top, windowBounds.right - windowBounds.left, windowBounds.bottom - windowBounds.top ), xImage);
            CGImageRelease(xImage);
            CGContextFlush( xWindowContext );
            QDEndCGContext (GetWindowPort(mrWindow), &xWindowContext);
        }
    }
}

// -----------------------------------------------------------------------

