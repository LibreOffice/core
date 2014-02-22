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
#include <canvas/canvastools.hxx>
#include <tools/diagnose_ex.h>

#include <comphelper/scopeguard.hxx>

#include <basegfx/range/b2drectangle.hxx>
#include <basegfx/tools/canvastools.hxx>

#include <boost/cast.hpp>

#include "dx_spritecanvashelper.hxx"
#include "dx_canvascustomsprite.hxx"

#if defined(DX_DEBUG_IMAGES)
# if OSL_DEBUG_LEVEL > 0
#  include <imdebug.h>
#  undef min
#  undef max
# endif
#endif

using namespace ::com::sun::star;

namespace dxcanvas
{
    namespace
    {
        void repaintBackground( const ::basegfx::B2DRange&      rUpdateArea,
                                const ::basegfx::B2IRange&      rOutputArea,
                                const DXSurfaceBitmapSharedPtr& rBackBuffer )
        {
            
            ::basegfx::B2IRange aActualArea( 0, 0,
                                             static_cast<sal_Int32>(rOutputArea.getWidth()),
                                             static_cast<sal_Int32>(rOutputArea.getHeight()) );
            aActualArea.intersect( fround( rUpdateArea ) );

            
            rBackBuffer->draw(aActualArea);
        }

        void spriteRedraw( const ::canvas::Sprite::Reference& rSprite )
        {
            
            
            ::boost::polymorphic_downcast< Sprite* >(
                rSprite.get() )->redraw();
        }

        void spriteRedrawStub( const ::canvas::Sprite::Reference& rSprite )
        {
            if( rSprite.is() )
            {
                
                
                ::boost::polymorphic_downcast< Sprite* >(
                    rSprite.get() )->redraw();
            }
        }

        void spriteRedrawStub2( const ::canvas::SpriteRedrawManager::AreaComponent& rComponent )
        {
            if( rComponent.second.getSprite().is() )
            {
                
                
                ::boost::polymorphic_downcast< Sprite* >(
                    rComponent.second.getSprite().get() )->redraw();
            }
        }
    }

    SpriteCanvasHelper::SpriteCanvasHelper() :
        mpSpriteSurface( NULL ),
        mpRedrawManager( NULL ),
        mpRenderModule(),
        mpSurfaceProxy(),
        mpBackBuffer(),
        maUpdateRect(),
        maScrapRect(),
        mbShowSpriteBounds( false )
    {
#if OSL_DEBUG_LEVEL > 2
        
        mbShowSpriteBounds = true;
#endif
    }

    void SpriteCanvasHelper::init( SpriteCanvas&                                    rParent,
                                   ::canvas::SpriteRedrawManager&                   rManager,
                                   const IDXRenderModuleSharedPtr&                  rRenderModule,
                                   const ::canvas::ISurfaceProxyManagerSharedPtr&   rSurfaceProxy,
                                   const DXSurfaceBitmapSharedPtr&                  rBackBuffer,
                                   const ::basegfx::B2ISize&                        rOutputOffset )
    {
        
        setDevice( rParent );
        setTarget( rBackBuffer, rOutputOffset );

        mpSpriteSurface = &rParent;
        mpRedrawManager = &rManager;
        mpRenderModule  = rRenderModule;
        mpSurfaceProxy  = rSurfaceProxy;
        mpBackBuffer    = rBackBuffer;
    }

    void SpriteCanvasHelper::disposing()
    {
        if(mpRenderModule)
            mpRenderModule->disposing();

        mpBackBuffer.reset();
        mpRenderModule.reset();
        mpRedrawManager = NULL;
        mpSpriteSurface = NULL;

        
        CanvasHelper::disposing();
    }

    uno::Reference< rendering::XAnimatedSprite > SpriteCanvasHelper::createSpriteFromAnimation(
        const uno::Reference< rendering::XAnimation >& /*animation*/ )
    {
        return uno::Reference< rendering::XAnimatedSprite >();
    }

    uno::Reference< rendering::XAnimatedSprite > SpriteCanvasHelper::createSpriteFromBitmaps(
        const uno::Sequence< uno::Reference< rendering::XBitmap > >& /*animationBitmaps*/,
        sal_Int8                                                     /*interpolationMode*/ )
    {
        return uno::Reference< rendering::XAnimatedSprite >();
    }

    uno::Reference< rendering::XCustomSprite > SpriteCanvasHelper::createCustomSprite( const geometry::RealSize2D& spriteSize )
    {
        if( !mpRedrawManager )
            return uno::Reference< rendering::XCustomSprite >(); 

        return uno::Reference< rendering::XCustomSprite >(
            new CanvasCustomSprite( spriteSize,
                                    mpSpriteSurface,
                                    mpRenderModule,
                                    mpSurfaceProxy,
                                    mbShowSpriteBounds ) );
    }

    uno::Reference< rendering::XSprite > SpriteCanvasHelper::createClonedSprite( const uno::Reference< rendering::XSprite >& /*original*/ )
    {
        return uno::Reference< rendering::XSprite >();
    }

    sal_Bool SpriteCanvasHelper::updateScreen( const ::basegfx::B2IRectangle& rCurrArea,
                                               sal_Bool                       bUpdateAll,
                                               bool&                          io_bSurfaceDirty )
    {
        if( !mpRedrawManager ||
            !mpRenderModule ||
            !mpBackBuffer )
        {
            return sal_False; 
        }

#if defined(DX_DEBUG_IMAGES)
# if OSL_DEBUG_LEVEL > 0
        mpBackBuffer->imageDebugger();
# endif
#endif

        
        
        
        maScrapRect = rCurrArea;

        
        maUpdateRect.reset();

        
        

        
        
        
        if( !bUpdateAll && !io_bSurfaceDirty )
        {
            
            

            
            
            mpRedrawManager->forEachSpriteArea( *this );

            
            

            
            mpRenderModule->flip( maUpdateRect,
                                  rCurrArea );
        }
        else
        {
            
            
            const ::basegfx::B2IRectangle aUpdateArea( 0,0,
                                                       static_cast<sal_Int32>(rCurrArea.getWidth()),
                                                       static_cast<sal_Int32>(rCurrArea.getHeight()) );

            
            
            
            

            
            mpBackBuffer->draw(aUpdateArea);

            
            mpRedrawManager->forEachSprite(::std::ptr_fun( &spriteRedraw ) );

            
            

            
            mpRenderModule->flip( aUpdateArea,
                                  rCurrArea );
        }

        
        
        mpRedrawManager->clearChangeRecords();

        io_bSurfaceDirty = false;

        return sal_True;
    }

    void SpriteCanvasHelper::backgroundPaint( const ::basegfx::B2DRange& rUpdateRect )
    {
        ENSURE_OR_THROW( mpRenderModule &&
                          mpBackBuffer,
                          "SpriteCanvasHelper::backgroundPaint(): NULL device pointer " );

        repaintBackground( rUpdateRect,
                           maScrapRect,
                           mpBackBuffer );
    }

    void SpriteCanvasHelper::scrollUpdate( const ::basegfx::B2DRange&                       /*rMoveStart*/,
                                           const ::basegfx::B2DRange&                       rMoveEnd,
                                           const ::canvas::SpriteRedrawManager::UpdateArea& rUpdateArea )
    {
        ENSURE_OR_THROW( mpRenderModule &&
                          mpBackBuffer,
                          "SpriteCanvasHelper::scrollUpdate(): NULL device pointer " );

        
        
        
        
        
        const ::basegfx::B2IRange& rDestRect(
            ::canvas::tools::spritePixelAreaFromB2DRange( rMoveEnd ) );

        
        
        
        
        
        
        
        ::std::for_each( rUpdateArea.maComponentList.begin(),
                         rUpdateArea.maComponentList.end(),
                         ::std::ptr_fun( &spriteRedrawStub2 ) );

        
        
        
        ::std::vector< ::basegfx::B2DRange > aUncoveredAreas;
        ::basegfx::computeSetDifference( aUncoveredAreas,
                                         rUpdateArea.maTotalBounds,
                                         ::basegfx::B2DRange( rDestRect ) );
        ::std::for_each( aUncoveredAreas.begin(),
                         aUncoveredAreas.end(),
                         ::boost::bind( &repaintBackground,
                                        _1,
                                        ::boost::cref(maScrapRect),
                                        ::boost::cref(mpBackBuffer) ) );

        
        ::basegfx::B2IRange aActualArea( 0, 0,
                                         static_cast<sal_Int32>(maScrapRect.getWidth()),
                                         static_cast<sal_Int32>(maScrapRect.getHeight()) );
        aActualArea.intersect( fround( rUpdateArea.maTotalBounds ) );

        
        maUpdateRect.expand( aActualArea );
    }

    void SpriteCanvasHelper::opaqueUpdate( const ::basegfx::B2DRange&                          rTotalArea,
                                           const ::std::vector< ::canvas::Sprite::Reference >& rSortedUpdateSprites )
    {
        ENSURE_OR_THROW( mpRenderModule &&
                          mpBackBuffer,
                          "SpriteCanvasHelper::opaqueUpdate(): NULL device pointer " );

        
        
        
        ::std::for_each( rSortedUpdateSprites.begin(),
                         rSortedUpdateSprites.end(),
                         ::std::ptr_fun( &spriteRedrawStub ) );

        
        ::basegfx::B2IRange aActualArea( 0, 0,
                                         static_cast<sal_Int32>(maScrapRect.getWidth()),
                                         static_cast<sal_Int32>(maScrapRect.getHeight()) );
        aActualArea.intersect( fround( rTotalArea ) );

        
        maUpdateRect.expand( aActualArea );
    }

    void SpriteCanvasHelper::genericUpdate( const ::basegfx::B2DRange&                          rTotalArea,
                                            const ::std::vector< ::canvas::Sprite::Reference >& rSortedUpdateSprites )
    {
        ENSURE_OR_THROW( mpRenderModule &&
                          mpBackBuffer,
                          "SpriteCanvasHelper::genericUpdate(): NULL device pointer " );

        
        

        
        ::basegfx::B2IRange aActualArea( 0, 0,
                                         static_cast<sal_Int32>(maScrapRect.getWidth()),
                                         static_cast<sal_Int32>(maScrapRect.getHeight()) );
        aActualArea.intersect( fround( rTotalArea ) );

        
        mpBackBuffer->draw(aActualArea);

        
        

        ::std::for_each( rSortedUpdateSprites.begin(),
                         rSortedUpdateSprites.end(),
                         ::std::ptr_fun( &spriteRedrawStub ) );

        
        maUpdateRect.expand( aActualArea );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
