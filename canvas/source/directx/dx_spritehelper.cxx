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
#include <canvas/verbosetrace.hxx>
#include <tools/diagnose_ex.h>

#include <rtl/math.hxx>

#include <canvas/canvastools.hxx>

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/tools/canvastools.hxx>
#include <basegfx/numeric/ftools.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolygontriangulator.hxx>
#include <basegfx/polygon/b2dpolygoncutandtouch.hxx>

#include "dx_canvascustomsprite.hxx"
#include "dx_spritehelper.hxx"
#include "dx_impltools.hxx"

using namespace ::com::sun::star;

namespace dxcanvas
{
    SpriteHelper::SpriteHelper() :
        mpSpriteCanvas(),
        mpBitmap(),
        mbTextureDirty( true ),
        mbShowSpriteBounds( false )
    {
    }

    void SpriteHelper::init( const geometry::RealSize2D&     rSpriteSize,
                             const SpriteCanvasRef&          rSpriteCanvas,
                             const IDXRenderModuleSharedPtr& rRenderModule,
                             const DXSurfaceBitmapSharedPtr  rBitmap,
                             bool                            bShowSpriteBounds )
    {
        ENSURE_OR_THROW( rSpriteCanvas.get() &&
                          rRenderModule &&
                          rBitmap,
                          "SpriteHelper::init(): Invalid device, sprite canvas or surface" );

        mpSpriteCanvas     = rSpriteCanvas;
        mpBitmap           = rBitmap;
        mbTextureDirty     = true;
        mbShowSpriteBounds = bShowSpriteBounds;

        
        CanvasCustomSpriteHelper::init( rSpriteSize,
                                        rSpriteCanvas.get() );
    }

    void SpriteHelper::disposing()
    {
        mpBitmap.reset();
        mpSpriteCanvas.clear();

        
        CanvasCustomSpriteHelper::disposing();
    }

    ::basegfx::B2DPolyPolygon SpriteHelper::polyPolygonFromXPolyPolygon2D( uno::Reference< rendering::XPolyPolygon2D >& xPoly ) const
    {
        return tools::polyPolygonFromXPolyPolygon2D( xPoly );
    }

    bool SpriteHelper::needRedraw() const
    {
        if( !mpBitmap ||
            !mpSpriteCanvas.get() )
        {
            return false; 
        }

        if( !isActive() ||
            ::basegfx::fTools::equalZero( getAlpha() ) )
        {
            return false; 
        }

        return true;
    }

    void SpriteHelper::redraw( bool& io_bSurfaceDirty ) const
    {
        if( !mpBitmap ||
            !mpSpriteCanvas.get() )
        {
            return; 
        }

        const ::basegfx::B2DPoint& rPos( getPosPixel() );
        const double               fAlpha( getAlpha() );

        if( isActive() &&
            !::basegfx::fTools::equalZero( fAlpha ) )
        {

            
            
            

            
            VERBOSE_TRACE( "SpriteHelper::redraw(): output pos is (%f, %f)",
                           rPos.getX(),
                           rPos.getY() );

            const double                                       fAlpha( getAlpha() );
            const ::basegfx::B2DVector&                        rSize( getSizePixel() );
            const ::basegfx::B2DHomMatrix&                     rTransform( getTransformation() );
            const uno::Reference< rendering::XPolyPolygon2D >& xClip( getClip() );

            mbTextureDirty   = false;
            io_bSurfaceDirty = false; 

            ::basegfx::B2DPolyPolygon   aClipPath; 
            bool                        bIsClipRectangular( false ); 
                                                                    
                                                                    

            
            

            if( xClip.is() )
            {
                aClipPath = tools::polyPolygonFromXPolyPolygon2D( xClip );

                const sal_Int32 nNumClipPolygons( aClipPath.count() );
                if( nNumClipPolygons )
                {
                    
                    

                    
                    if( nNumClipPolygons == 1 )
                        if( ::basegfx::tools::isRectangle( aClipPath.getB2DPolygon( 0 ) ) )
                            bIsClipRectangular = true;
                }
            }

            const ::basegfx::B2DRectangle aSourceRect( 0.0,
                                                       0.0,
                                                       rSize.getX(),
                                                       rSize.getY() );

            
            if( !aClipPath.count() )
            {
                mpBitmap->draw(fAlpha,rPos,rTransform);
            }
            else if( bIsClipRectangular )
            {
                
                

                ::basegfx::B2DRectangle aClipBounds(
                    ::basegfx::tools::getRange( aClipPath ) );
                aClipBounds.intersect( aSourceRect );

                mpBitmap->draw(fAlpha,rPos,aClipBounds,rTransform);
            }
            else
            {
                
                

                mpBitmap->draw(fAlpha,rPos,aClipPath,rTransform);
            }

            if( mbShowSpriteBounds )
            {
                if( aClipPath.count() )
                {
                    
                }
            }
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
