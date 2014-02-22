/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */


#include <canvas/debug.hxx>
#include <tools/diagnose_ex.h>
#include <canvas/verbosetrace.hxx>

#include <rtl/math.hxx>

#include <vcl/outdev.hxx>
#include <vcl/bitmap.hxx>
#include <vcl/alpha.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/canvastools.hxx>

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/tools/canvastools.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/polygon/b2dpolygoncutandtouch.hxx>
#include <basegfx/polygon/b2dpolygontriangulator.hxx>
#include <basegfx/polygon/b2dpolygonclipper.hxx>
#include <basegfx/numeric/ftools.hxx>

#include <canvas/canvastools.hxx>

#include "spritehelper.hxx"

using namespace ::com::sun::star;


namespace vclcanvas
{
    SpriteHelper::SpriteHelper() :
        mpBackBuffer(),
        mpBackBufferMask(),
        maContent(),
        mbShowSpriteBounds(false)
    {
    }

    void SpriteHelper::init( const geometry::RealSize2D&               rSpriteSize,
                             const ::canvas::SpriteSurface::Reference& rOwningSpriteCanvas,
                             const BackBufferSharedPtr&                rBackBuffer,
                             const BackBufferSharedPtr&                rBackBufferMask,
                             bool                                      bShowSpriteBounds )
    {
        ENSURE_OR_THROW( rOwningSpriteCanvas.get() && rBackBuffer && rBackBufferMask,
                         "SpriteHelper::init(): Invalid sprite canvas or back buffer" );

        mpBackBuffer        = rBackBuffer;
        mpBackBufferMask    = rBackBufferMask;
        mbShowSpriteBounds  = bShowSpriteBounds;

        init( rSpriteSize, rOwningSpriteCanvas );
    }

    void SpriteHelper::disposing()
    {
        mpBackBuffer.reset();
        mpBackBufferMask.reset();

        
        CanvasCustomSpriteHelper::disposing();
    }

    void SpriteHelper::redraw( OutputDevice&                rTargetSurface,
                               const ::basegfx::B2DPoint&   rPos,
                               bool&                        io_bSurfacesDirty,
                               bool                         bBufferedUpdate ) const
    {
        (void)bBufferedUpdate; 

        if( !mpBackBuffer ||
            !mpBackBufferMask )
        {
            return; 
        }

        
        VERBOSE_TRACE( "SpriteHelper::redraw(): output pos is (%f, %f)",
                       rPos.getX(),
                       rPos.getY() );

        const double fAlpha( getAlpha() );

        if( isActive() &&
            !::basegfx::fTools::equalZero( fAlpha ) )
        {
            const Point                 aEmptyPoint;
            const ::basegfx::B2DVector& rOrigOutputSize( getSizePixel() );

            
            
            
            
            
            ::Size  aOutputSize( ::vcl::unotools::sizeFromB2DSize( rOrigOutputSize ) );
            ::Point aOutPos( ::vcl::unotools::pointFromB2DPoint( rPos ) );


            

            
            
            const bool bNeedBitmapUpdate( io_bSurfacesDirty ||
                                          hasTransformChanged() ||
                                          maContent->IsEmpty() );

            
            
            io_bSurfacesDirty = false;
            transformUpdated();

            if( bNeedBitmapUpdate )
            {
                Bitmap aBmp( mpBackBuffer->getOutDev().GetBitmap( aEmptyPoint,
                                                                  aOutputSize ) );

                if( isContentFullyOpaque() )
                {
                    
                    
                    
                    
                    maContent = BitmapEx( aBmp );
                }
                else
                {
                    
                    
                    Bitmap aMask( mpBackBufferMask->getOutDev().GetBitmap( aEmptyPoint,
                                                                           aOutputSize ) );

                    
                    
#ifndef MACOSX
                    if( aMask.GetBitCount() != 1 )
                    {
                        OSL_FAIL("CanvasCustomSprite::redraw(): Mask bitmap is not "
                                   "monochrome (performance!)");
                        aMask.MakeMono(255);
                    }
#endif

                    
                    
                    
                    maContent = BitmapEx( aBmp, aMask );
                }
            }

            ::basegfx::B2DHomMatrix aTransform( getTransformation() );

            
            
            
            const bool bIdentityTransform( aTransform.isIdentity() );

            
            
            
            aTransform.translate( aOutPos.X(),
                                  aOutPos.Y() );

            if( !bIdentityTransform )
            {
                if( !::basegfx::fTools::equalZero( aTransform.get(0,1) ) ||
                    !::basegfx::fTools::equalZero( aTransform.get(1,0) ) )
                {
                    
                    

                    
                    
                    
                    ::basegfx::B2DRectangle aDestRect;
                    ::canvas::tools::calcTransformedRectBounds( aDestRect,
                                                                ::basegfx::B2DRectangle(0,
                                                                                        0,
                                                                                        rOrigOutputSize.getX(),
                                                                                        rOrigOutputSize.getY()),
                                                                aTransform );

                    aOutPos.X() = ::basegfx::fround( aDestRect.getMinX() );
                    aOutPos.Y() = ::basegfx::fround( aDestRect.getMinY() );

                    

                    
                    if( bNeedBitmapUpdate )
                        maContent = tools::transformBitmap( *maContent,
                                                            aTransform,
                                                            uno::Sequence<double>(),
                                                            tools::MODULATE_NONE );

                    aOutputSize = maContent->GetSizePixel();
                }
                else
                {
                    
                    
                    aOutputSize.setWidth (
                        ::basegfx::fround( rOrigOutputSize.getX() * aTransform.get(0,0) ) );
                    aOutputSize.setHeight(
                        ::basegfx::fround( rOrigOutputSize.getY() * aTransform.get(1,1) ) );

                    aOutPos.X() = ::basegfx::fround( aTransform.get(0,2) );
                    aOutPos.Y() = ::basegfx::fround( aTransform.get(1,2) );
                }
            }

            
            
            if( !!(*maContent) )
            {
                
                
                bool bSpriteRedrawn( false );

                rTargetSurface.Push( PUSH_CLIPREGION );

                
                if( getClip().is() )
                {
                    ::basegfx::B2DPolyPolygon aClipPoly(
                        ::basegfx::unotools::b2DPolyPolygonFromXPolyPolygon2D(
                            getClip() ));

                    if( aClipPoly.count() )
                    {
                        
                        
                        
                        aClipPoly.transform( aTransform );

#if ! defined WNT && ! defined MACOSX
                        
                        

                        
                        
                        
                        
                        
                        
                        
                        bool bAtLeastOnePolygon( false );
                        const sal_Int32 nPolygons( aClipPoly.count() );

                        for( sal_Int32 i=0; i<nPolygons; ++i )
                        {
                            if( !::basegfx::tools::isRectangle(
                                    aClipPoly.getB2DPolygon(i)) )
                            {
                                bAtLeastOnePolygon = true;
                                break;
                            }
                        }
#endif

                        if( mbShowSpriteBounds )
                        {
                            
                            rTargetSurface.SetLineColor( Color( 0,255,0 ) );
                            rTargetSurface.SetFillColor();

                            rTargetSurface.DrawPolyPolygon(PolyPolygon(aClipPoly)); 
                        }

#if ! defined WNT && ! defined MACOSX
                        
                        
                        
                        if( bAtLeastOnePolygon &&
                            bBufferedUpdate &&
                            ::rtl::math::approxEqual(fAlpha, 1.0) &&
                            !maContent->IsTransparent() )
                        {
                            
                            
                            

                            
                            
                            
                            
                            
                            
                            
                            rTargetSurface.Push( PUSH_RASTEROP );
                            rTargetSurface.SetRasterOp( ROP_XOR );
                            rTargetSurface.DrawBitmap( aOutPos,
                                                       aOutputSize,
                                                       maContent->GetBitmap() );

                            rTargetSurface.SetLineColor();
                            rTargetSurface.SetFillColor( COL_BLACK );
                            rTargetSurface.SetRasterOp( ROP_0 );
                            rTargetSurface.DrawPolyPolygon(PolyPolygon(aClipPoly)); 

                            rTargetSurface.SetRasterOp( ROP_XOR );
                            rTargetSurface.DrawBitmap( aOutPos,
                                                       aOutputSize,
                                                       maContent->GetBitmap() );

                            rTargetSurface.Pop();

                            bSpriteRedrawn = true;
                        }
                        else
#endif
                        {
                            Region aClipRegion( aClipPoly );
                            rTargetSurface.SetClipRegion( aClipRegion );
                        }
                    }
                }

                if( !bSpriteRedrawn )
                {
                    if( ::rtl::math::approxEqual(fAlpha, 1.0) )
                    {
                        
                        if( maContent->IsTransparent() )
                            rTargetSurface.DrawBitmapEx( aOutPos, aOutputSize, *maContent );
                        else
                            rTargetSurface.DrawBitmap( aOutPos, aOutputSize, maContent->GetBitmap() );
                    }
                    else
                    {
                        
                        

                        
                        sal_uInt8 nColor( static_cast<sal_uInt8>( ::basegfx::fround( 255.0*(1.0 - fAlpha) + .5) ) );
                        AlphaMask aAlpha( maContent->GetSizePixel(),
                                          &nColor );

                        
                        if( maContent->IsTransparent() )
                            aAlpha.Replace( maContent->GetMask(), 255 );

                        
                        rTargetSurface.DrawBitmapEx( aOutPos, aOutputSize,
                                                     BitmapEx( maContent->GetBitmap(),
                                                               aAlpha ) );
                    }
                }

                rTargetSurface.Pop();

                if( mbShowSpriteBounds )
                {
                    ::PolyPolygon aMarkerPoly(
                        ::canvas::tools::getBoundMarksPolyPolygon(
                            ::basegfx::B2DRectangle(aOutPos.X(),
                                                    aOutPos.Y(),
                                                    aOutPos.X() + aOutputSize.Width()-1,
                                                    aOutPos.Y() + aOutputSize.Height()-1) ) );

                    
                    rTargetSurface.SetLineColor( COL_RED );
                    rTargetSurface.SetFillColor();

                    for( int i=0; i<aMarkerPoly.Count(); ++i )
                    {
                        rTargetSurface.DrawPolyLine( aMarkerPoly.GetObject((sal_uInt16)i) );
                    }

                    
                    Font aVCLFont;
                    aVCLFont.SetHeight( std::min(long(20),aOutputSize.Height()) );
                    aVCLFont.SetColor( COL_RED );

                    rTargetSurface.SetTextAlign(ALIGN_TOP);
                    rTargetSurface.SetTextColor( COL_RED );
                    rTargetSurface.SetFont( aVCLFont );

                    OUString text( ::rtl::math::doubleToUString( getPriority(),
                                                                        rtl_math_StringFormat_F,
                                                                        2,'.',NULL,' ') );

                    rTargetSurface.DrawText( aOutPos+Point(2,2), text );
                    VERBOSE_TRACE( "SpriteHelper::redraw(): sprite %X has prio %f\n",
                                   this, getPriority() );
                }
            }
        }
    }

    ::basegfx::B2DPolyPolygon SpriteHelper::polyPolygonFromXPolyPolygon2D( uno::Reference< rendering::XPolyPolygon2D >& xPoly ) const
    {
        return ::basegfx::unotools::b2DPolyPolygonFromXPolyPolygon2D( xPoly );
    }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
