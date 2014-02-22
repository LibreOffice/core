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
#include <canvas/canvastools.hxx>

#include <rtl/math.hxx>

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/tools/canvastools.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/numeric/ftools.hxx>

#include <canvas/base/canvascustomspritehelper.hxx>

using namespace ::com::sun::star;


namespace canvas
{
    bool CanvasCustomSpriteHelper::updateClipState( const Sprite::Reference& rSprite )
    {
        if( !mxClipPoly.is() )
        {
            
            maCurrClipBounds.reset();
            mbIsCurrClipRectangle = true;
        }
        else
        {
            const sal_Int32 nNumClipPolygons( mxClipPoly->getNumberOfPolygons() );

            
            ::basegfx::B2DPolyPolygon aClipPath(
                polyPolygonFromXPolyPolygon2D( mxClipPoly ) );

            
            aClipPath.transform( maTransform );

            
            
            const ::basegfx::B2DRectangle& rClipBounds(
                ::basegfx::tools::getRange( aClipPath ) );

            const ::basegfx::B2DRectangle aBounds( 0.0, 0.0,
                                                   maSize.getX(),
                                                   maSize.getY() );

            
            
            ::basegfx::B2DRectangle aSpriteRectPixel;
            ::canvas::tools::calcTransformedRectBounds( aSpriteRectPixel,
                                                        aBounds,
                                                        maTransform );

            
            
            ::basegfx::B2DRectangle aClipBoundsA(rClipBounds);
            aClipBoundsA.intersect( aSpriteRectPixel );

            if( nNumClipPolygons != 1 )
            {
                
                
                mbIsCurrClipRectangle = false;
                maCurrClipBounds = aClipBoundsA;
            }
            else
            {
                
                
                const bool bNewClipIsRect(
                    ::basegfx::tools::isRectangle( aClipPath.getB2DPolygon(0) ) );

                
                
                const bool bUseOptimizedUpdate( bNewClipIsRect &&
                                                mbIsCurrClipRectangle );

                const ::basegfx::B2DRectangle aOldBounds( maCurrClipBounds );

                
                maCurrClipBounds = aClipBoundsA;
                mbIsCurrClipRectangle = bNewClipIsRect;

                if( mbActive &&
                    bUseOptimizedUpdate  )
                {
                    
                    
                    typedef ::std::vector< ::basegfx::B2DRectangle > VectorOfRects;
                    VectorOfRects aClipDifferences;

                    
                    
                    ::basegfx::computeSetDifference(aClipDifferences,
                                                    aClipBoundsA,
                                                    aOldBounds);

                    
                    
                    
                    
                    
                    
                    
                    VectorOfRects::const_iterator       aCurr( aClipDifferences.begin() );
                    const VectorOfRects::const_iterator aEnd( aClipDifferences.end() );
                    while( aCurr != aEnd )
                    {
                        mpSpriteCanvas->updateSprite(
                            rSprite,
                            maPosition,
                            ::basegfx::B2DRectangle(
                                maPosition + aCurr->getMinimum(),
                                maPosition + aCurr->getMaximum() ) );
                        ++aCurr;
                    }

                    
                    return true;
                }
            }
        }

        
        return false;
    }

    CanvasCustomSpriteHelper::CanvasCustomSpriteHelper() :
        mpSpriteCanvas(),
        maCurrClipBounds(),
        maPosition(),
        maSize(),
        maTransform(),
        mxClipPoly(),
        mfPriority(0.0),
        mfAlpha(0.0),
        mbActive(false),
        mbIsCurrClipRectangle(true),
        mbIsContentFullyOpaque( false ),
        mbAlphaDirty( true ),
        mbPositionDirty( true ),
        mbTransformDirty( true ),
        mbClipDirty( true ),
        mbPrioDirty( true ),
        mbVisibilityDirty( true )
    {
    }

    void CanvasCustomSpriteHelper::init( const geometry::RealSize2D&        rSpriteSize,
                                         const SpriteSurface::Reference&    rOwningSpriteCanvas )
    {
        ENSURE_OR_THROW( rOwningSpriteCanvas.get(),
                          "CanvasCustomSpriteHelper::init(): Invalid owning sprite canvas" );

        mpSpriteCanvas = rOwningSpriteCanvas;
        maSize.setX( ::std::max( 1.0,
                                 ceil( rSpriteSize.Width ) ) ); 
                                                                 
                                                                 
        maSize.setY( ::std::max( 1.0,
                                 ceil( rSpriteSize.Height ) ) );
    }

    void CanvasCustomSpriteHelper::disposing()
    {
        mpSpriteCanvas.clear();
    }

    void CanvasCustomSpriteHelper::clearingContent( const Sprite::Reference& /*rSprite*/ )
    {
        
        mbIsContentFullyOpaque = false;
    }

    void CanvasCustomSpriteHelper::checkDrawBitmap( const Sprite::Reference&                    rSprite,
                                                    const uno::Reference< rendering::XBitmap >& xBitmap,
                                                    const rendering::ViewState&                 viewState,
                                                    const rendering::RenderState&               renderState )
    {
        
        
        if( !xBitmap->hasAlpha() )
        {
            const geometry::IntegerSize2D& rInputSize(
                xBitmap->getSize() );
            const ::basegfx::B2DSize& rOurSize(
                rSprite->getSizePixel() );

            ::basegfx::B2DHomMatrix aTransform;
            if( tools::isInside(
                    ::basegfx::B2DRectangle( 0.0,0.0,
                                             rOurSize.getX(),
                                             rOurSize.getY() ),
                    ::basegfx::B2DRectangle( 0.0,0.0,
                                             rInputSize.Width,
                                             rInputSize.Height ),
                    ::canvas::tools::mergeViewAndRenderTransform(aTransform,
                                                                 viewState,
                                                                 renderState) ) )
            {
                
                
                mbIsContentFullyOpaque = true;
            }
        }
    }

    void CanvasCustomSpriteHelper::setAlpha( const Sprite::Reference&   rSprite,
                                             double                     alpha )
    {
        if( !mpSpriteCanvas.get() )
            return; 

        if( alpha != mfAlpha )
        {
            mfAlpha = alpha;

            if( mbActive )
            {
                mpSpriteCanvas->updateSprite( rSprite,
                                              maPosition,
                                              getUpdateArea() );
            }

            mbAlphaDirty = true;
        }
    }

    void CanvasCustomSpriteHelper::move( const Sprite::Reference&       rSprite,
                                         const geometry::RealPoint2D&   aNewPos,
                                         const rendering::ViewState&    viewState,
                                         const rendering::RenderState&  renderState )
    {
        if( !mpSpriteCanvas.get() )
            return; 

        ::basegfx::B2DHomMatrix aTransform;
        ::canvas::tools::mergeViewAndRenderTransform(aTransform,
                                                     viewState,
                                                     renderState);

        
        ::basegfx::B2DPoint aPoint(
            ::basegfx::unotools::b2DPointFromRealPoint2D(aNewPos) );
        aPoint *= aTransform;

        if( aPoint != maPosition )
        {
            const ::basegfx::B2DRectangle&  rBounds( getFullSpriteRect() );

            if( mbActive )
            {
                mpSpriteCanvas->moveSprite( rSprite,
                                            rBounds.getMinimum(),
                                            rBounds.getMinimum() - maPosition + aPoint,
                                            rBounds.getRange() );
            }

            maPosition = aPoint;
            mbPositionDirty = true;
        }
    }

    void CanvasCustomSpriteHelper::transform( const Sprite::Reference&          rSprite,
                                              const geometry::AffineMatrix2D&   aTransformation )
    {
        ::basegfx::B2DHomMatrix aMatrix;
        ::basegfx::unotools::homMatrixFromAffineMatrix(aMatrix,
                                                       aTransformation);

        if( maTransform != aMatrix )
        {
            
            const ::basegfx::B2DRectangle& rPrevBounds( getUpdateArea() );

            maTransform = aMatrix;

            if( !updateClipState( rSprite ) &&
                mbActive )
            {
                mpSpriteCanvas->updateSprite( rSprite,
                                              maPosition,
                                              rPrevBounds );
                mpSpriteCanvas->updateSprite( rSprite,
                                              maPosition,
                                              getUpdateArea() );
            }

            mbTransformDirty = true;
        }
    }

    void CanvasCustomSpriteHelper::clip( const Sprite::Reference&                           rSprite,
                                         const uno::Reference< rendering::XPolyPolygon2D >& xClip )
    {
        

        
        const ::basegfx::B2DRectangle& rPrevBounds( getUpdateArea() );

        mxClipPoly = xClip;

        if( !updateClipState( rSprite ) &&
            mbActive )
        {
            mpSpriteCanvas->updateSprite( rSprite,
                                          maPosition,
                                          rPrevBounds );
            mpSpriteCanvas->updateSprite( rSprite,
                                          maPosition,
                                          getUpdateArea() );
        }

        mbClipDirty = true;
    }

    void CanvasCustomSpriteHelper::setPriority( const Sprite::Reference&    rSprite,
                                                double                      nPriority )
    {
        if( !mpSpriteCanvas.get() )
            return; 

        if( nPriority != mfPriority )
        {
            mfPriority = nPriority;

            if( mbActive )
            {
                mpSpriteCanvas->updateSprite( rSprite,
                                              maPosition,
                                              getUpdateArea() );
            }

            mbPrioDirty = true;
        }
    }

    void CanvasCustomSpriteHelper::show( const Sprite::Reference& rSprite )
    {
        if( !mpSpriteCanvas.get() )
            return; 

        if( !mbActive )
        {
            mpSpriteCanvas->showSprite( rSprite );
            mbActive = true;

            
            

            if( mfAlpha != 0.0 )
            {
                mpSpriteCanvas->updateSprite( rSprite,
                                              maPosition,
                                              getUpdateArea() );
            }

            mbVisibilityDirty = true;
        }
    }

    void CanvasCustomSpriteHelper::hide( const Sprite::Reference& rSprite )
    {
        if( !mpSpriteCanvas.get() )
            return; 

        if( mbActive )
        {
            mpSpriteCanvas->hideSprite( rSprite );
            mbActive = false;

            
            

            if( mfAlpha != 0.0 )
            {
                mpSpriteCanvas->updateSprite( rSprite,
                                              maPosition,
                                              getUpdateArea() );
            }

            mbVisibilityDirty = true;
        }
    }

    
    bool CanvasCustomSpriteHelper::isAreaUpdateOpaque( const ::basegfx::B2DRange& rUpdateArea ) const
    {
        if( !mbIsCurrClipRectangle ||
            !mbIsContentFullyOpaque ||
            !::rtl::math::approxEqual(mfAlpha, 1.0) )
        {
            
            
            
            return false;
        }
        else
        {
            
            
            
            
            
            return getUpdateArea().isInside( rUpdateArea );
        }
    }

    ::basegfx::B2DPoint CanvasCustomSpriteHelper::getPosPixel() const
    {
        return maPosition;
    }

    ::basegfx::B2DVector CanvasCustomSpriteHelper::getSizePixel() const
    {
        return maSize;
    }

    ::basegfx::B2DRange CanvasCustomSpriteHelper::getUpdateArea( const ::basegfx::B2DRange& rBounds ) const
    {
        
        ::basegfx::B2DHomMatrix aTransform( maTransform );
        aTransform.translate( maPosition.getX(),
                              maPosition.getY() );

        
        
        ::basegfx::B2DRectangle aTransformedBounds;
        return ::canvas::tools::calcTransformedRectBounds( aTransformedBounds,
                                                           rBounds,
                                                           aTransform );
    }

    ::basegfx::B2DRange CanvasCustomSpriteHelper::getUpdateArea() const
    {
        

        
        
        if( maCurrClipBounds.isEmpty() )
            return getUpdateArea( ::basegfx::B2DRectangle( 0.0, 0.0,
                                                           maSize.getX(),
                                                           maSize.getY() ) );
        else
            return ::basegfx::B2DRectangle(
                maPosition + maCurrClipBounds.getMinimum(),
                maPosition + maCurrClipBounds.getMaximum() );
    }

    double CanvasCustomSpriteHelper::getPriority() const
    {
        return mfPriority;
    }

    ::basegfx::B2DRange CanvasCustomSpriteHelper::getFullSpriteRect() const
    {
        
        return getUpdateArea( ::basegfx::B2DRectangle( 0.0, 0.0,
                                                       maSize.getX(),
                                                       maSize.getY() ) );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
