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

#include <vcl/canvastools.hxx>
#include <vcl/outdev.hxx>
#include <vcl/window.hxx>
#include <vcl/bitmapex.hxx>

#include <basegfx/range/b2drectangle.hxx>
#include <basegfx/tools/canvastools.hxx>

#include <boost/cast.hpp>

#include "spritecanvashelper.hxx"
#include "canvascustomsprite.hxx"

using namespace ::com::sun::star;

#define FPS_BOUNDS Rectangle(0,0,130,90)
#define INFO_COLOR COL_RED

namespace vclcanvas
{
    namespace
    {
        /** Sprite redraw at original position

            Used to repaint the whole canvas (background and all
            sprites)
         */
        void spriteRedraw( OutputDevice&                      rOutDev,
                           const ::canvas::Sprite::Reference& rSprite )
        {
            
            
            ::boost::polymorphic_downcast< Sprite* >(rSprite.get())->redraw(rOutDev,
                                                                            true);
        }

        double calcNumPixel( const ::canvas::Sprite::Reference& rSprite )
        {
            const ::basegfx::B2DSize& rSize(
                ::boost::polymorphic_downcast< Sprite* >(rSprite.get())->getSizePixel() );

            return rSize.getX() * rSize.getY();
        }

        void repaintBackground( OutputDevice&               rOutDev,
                                OutputDevice&               rBackBuffer,
                                const ::basegfx::B2DRange&  rArea )
        {
            const ::Point& rPos( ::vcl::unotools::pointFromB2DPoint( rArea.getMinimum()) );
            const ::Size& rSize( ::vcl::unotools::sizeFromB2DSize( rArea.getRange()) );

            rOutDev.DrawOutDev( rPos, rSize, rPos, rSize, rBackBuffer );
        }

        void opaqueUpdateSpriteArea( const ::canvas::Sprite::Reference& rSprite,
                                     OutputDevice&                      rOutDev,
                                     const ::basegfx::B2IRange&         rArea )
        {
            const Rectangle& rRequestedArea(
                ::vcl::unotools::rectangleFromB2IRectangle( rArea ) );

            
            
            
            rOutDev.EnableMapMode( false );
            rOutDev.SetAntialiasing( ANTIALIASING_ENABLE_B2DDRAW );
            rOutDev.SetClipRegion(Region(rRequestedArea));

            
            
            ::boost::polymorphic_downcast< Sprite* >(
                rSprite.get() )->redraw( rOutDev,
                                         false ); 
                                                  
                                                  
        }

        /** Repaint sprite at original position

            Used for opaque updates, which render directly to the
            front buffer.
         */
        void spriteRedrawStub( OutputDevice&                      rOutDev,
                               const ::canvas::Sprite::Reference& rSprite )
        {
            if( rSprite.is() )
            {
                ::boost::polymorphic_downcast< Sprite* >(
                    rSprite.get() )->redraw( rOutDev,
                                             false );
            }
        }

        /** Repaint sprite at given position

            Used for generic update, which renders into vdev of
            adapted size.
         */
        void spriteRedrawStub2( OutputDevice&                       rOutDev,
                                const ::basegfx::B2DPoint&          rOutPos,
                                const ::canvas::Sprite::Reference&  rSprite )
        {
            if( rSprite.is() )
            {
                Sprite* pSprite = ::boost::polymorphic_downcast< Sprite* >(
                    rSprite.get() );

                
                
                const ::basegfx::B2DPoint& rSpriteScreenPos( pSprite->getPosPixel() );
                const ::basegfx::B2DPoint& rSpriteRenderPos( rSpriteScreenPos - rOutPos );

                pSprite->redraw( rOutDev, rSpriteRenderPos, true );
            }
        }

        /** Repaint sprite at original position

            Used for opaque updates from scrollUpdate(), which render
            directly to the front buffer.
         */
        void spriteRedrawStub3( OutputDevice&                                       rOutDev,
                                const ::canvas::SpriteRedrawManager::AreaComponent& rComponent )
        {
            const ::canvas::Sprite::Reference& rSprite( rComponent.second.getSprite() );

            if( rSprite.is() )
            {
                ::boost::polymorphic_downcast< Sprite* >(
                    rSprite.get() )->redraw( rOutDev,
                                             false );
            }
        }

        void renderInfoText( OutputDevice&          rOutDev,
                             const OUString& rStr,
                             const Point&           rPos )
        {
            Font aVCLFont;
            aVCLFont.SetHeight( 20 );
            aVCLFont.SetColor( Color( INFO_COLOR ) );

            rOutDev.SetTextAlign(ALIGN_TOP);
            rOutDev.SetTextColor( Color( INFO_COLOR ) );
            rOutDev.SetFont( aVCLFont );

            rOutDev.DrawText( rPos, rStr );
        }

    }

    SpriteCanvasHelper::SpriteCanvasHelper() :
        mpRedrawManager( NULL ),
        mpOwningSpriteCanvas( NULL ),
        maVDev(),
        maLastUpdate(),
        mbShowFrameInfo( false ),
        mbShowSpriteBounds( false ),
        mbIsUnsafeScrolling( false )
    {
#if OSL_DEBUG_LEVEL > 2
        
        mbShowSpriteBounds = mbShowFrameInfo = true;
#endif
    }

    void SpriteCanvasHelper::init( const OutDevProviderSharedPtr& rOutDev,
                                   SpriteCanvas&                  rOwningSpriteCanvas,
                                   ::canvas::SpriteRedrawManager& rManager,
                                   bool                           bProtect,
                                   bool                           bHaveAlpha )
    {
        mpOwningSpriteCanvas = &rOwningSpriteCanvas;
        mpRedrawManager = &rManager;

        CanvasHelper::init(rOwningSpriteCanvas,rOutDev,bProtect,bHaveAlpha);
    }

    void SpriteCanvasHelper::disposing()
    {
        mpRedrawManager = NULL;
        mpOwningSpriteCanvas = NULL;

        
        CanvasHelper::disposing();
    }

    uno::Reference< rendering::XAnimatedSprite > SpriteCanvasHelper::createSpriteFromAnimation(
        const uno::Reference< rendering::XAnimation >&  )
    {
        return uno::Reference< rendering::XAnimatedSprite >();
    }

    uno::Reference< rendering::XAnimatedSprite > SpriteCanvasHelper::createSpriteFromBitmaps(
        const uno::Sequence< uno::Reference< rendering::XBitmap > >& ,
        sal_Int8                                                      )
    {
        return uno::Reference< rendering::XAnimatedSprite >();
    }

    uno::Reference< rendering::XCustomSprite > SpriteCanvasHelper::createCustomSprite( const geometry::RealSize2D& spriteSize )
    {
        if( !mpRedrawManager || !mpDevice )
            return uno::Reference< rendering::XCustomSprite >(); 

        return uno::Reference< rendering::XCustomSprite >(
            new CanvasCustomSprite( spriteSize,
                                    *mpDevice,
                                    mpOwningSpriteCanvas,
                                    mpOwningSpriteCanvas->getFrontBuffer(),
                                    mbShowSpriteBounds ) );
    }

    uno::Reference< rendering::XSprite > SpriteCanvasHelper::createClonedSprite( const uno::Reference< rendering::XSprite >&  )
    {
        return uno::Reference< rendering::XSprite >();
    }

    sal_Bool SpriteCanvasHelper::updateScreen( sal_Bool bUpdateAll,
                                               bool&    io_bSurfaceDirty )
    {
        if( !mpRedrawManager ||
            !mpOwningSpriteCanvas ||
            !mpOwningSpriteCanvas->getFrontBuffer() ||
            !mpOwningSpriteCanvas->getBackBuffer() )
        {
            return sal_False; 
        }

        
        flush();

        OutputDevice&       rOutDev( mpOwningSpriteCanvas->getFrontBuffer()->getOutDev() );
        BackBufferSharedPtr pBackBuffer( mpOwningSpriteCanvas->getBackBuffer() );
        OutputDevice&       rBackOutDev( pBackBuffer->getOutDev() );

        
        
        tools::OutDevStateKeeper aStateKeeper( rOutDev );

        const Size  aOutDevSize( rBackOutDev.GetOutputSizePixel() );
        const Point aEmptyPoint(0,0);

        Window* pTargetWindow = NULL;
        if( rOutDev.GetOutDevType() == OUTDEV_WINDOW )
        {
            pTargetWindow = &static_cast<Window&>(rOutDev); 

            
            
            
            
            const Region aFullWindowRegion( Rectangle(aEmptyPoint,
                                                      aOutDevSize) );
            pTargetWindow->ExpandPaintClipRegion(aFullWindowRegion);
        }

        
        
        if( !bUpdateAll && !io_bSurfaceDirty )
        {
            if( mbShowFrameInfo )
            {
                
                
                mpRedrawManager->updateSprite( ::canvas::Sprite::Reference(),
                                               ::basegfx::B2DPoint(),
                                               ::basegfx::B2DRectangle( 0.0, 0.0,
                                                                        FPS_BOUNDS.Right(),
                                                                        FPS_BOUNDS.Bottom() ) );
            }

            
            

            
            
            mpRedrawManager->forEachSpriteArea( *this );
        }
        else
        {
            
            

            maVDev->SetOutputSizePixel( aOutDevSize );
            maVDev->EnableMapMode( false );
            maVDev->DrawOutDev( aEmptyPoint, aOutDevSize,
                                aEmptyPoint, aOutDevSize,
                                rBackOutDev );

            
            
            mpRedrawManager->forEachSprite(
                ::boost::bind(
                    &spriteRedraw,
                    ::boost::ref( maVDev.get() ),
                    _1 ) );

            
            rOutDev.EnableMapMode( false );
            rOutDev.SetAntialiasing( ANTIALIASING_ENABLE_B2DDRAW );
            rOutDev.SetClipRegion();
            rOutDev.DrawOutDev( aEmptyPoint, aOutDevSize,
                                aEmptyPoint, aOutDevSize,
                                *maVDev );
        }

        
        
        mpRedrawManager->clearChangeRecords();

        io_bSurfaceDirty = false;

        if( mbShowFrameInfo )
        {
            renderFrameCounter( rOutDev );
            renderSpriteCount( rOutDev );
            renderMemUsage( rOutDev );
        }

#if OSL_DEBUG_LEVEL > 2
        static ::canvas::tools::ElapsedTime aElapsedTime;

        
        OSL_TRACE( "SpriteCanvasHelper::updateScreen(): flip done at %f",
                   aElapsedTime.getElapsedTime() );
#endif

        
        
        
        
        if( pTargetWindow )
        {
            
            pTargetWindow->Sync();
        }

        return sal_True;
    }

    void SpriteCanvasHelper::backgroundPaint( const ::basegfx::B2DRange& rUpdateRect )
    {
        ENSURE_OR_THROW( mpOwningSpriteCanvas &&
                         mpOwningSpriteCanvas->getBackBuffer() &&
                         mpOwningSpriteCanvas->getFrontBuffer(),
                         "SpriteCanvasHelper::backgroundPaint(): NULL device pointer " );

        OutputDevice&       rOutDev( mpOwningSpriteCanvas->getFrontBuffer()->getOutDev() );
        BackBufferSharedPtr pBackBuffer( mpOwningSpriteCanvas->getBackBuffer() );
        OutputDevice&       rBackOutDev( pBackBuffer->getOutDev() );

        repaintBackground( rOutDev, rBackOutDev, rUpdateRect );
    }

    void SpriteCanvasHelper::scrollUpdate( const ::basegfx::B2DRange&                       rMoveStart,
                                           const ::basegfx::B2DRange&                       rMoveEnd,
                                           const ::canvas::SpriteRedrawManager::UpdateArea& rUpdateArea )
    {
        ENSURE_OR_THROW( mpOwningSpriteCanvas &&
                         mpOwningSpriteCanvas->getBackBuffer() &&
                         mpOwningSpriteCanvas->getFrontBuffer(),
                         "SpriteCanvasHelper::scrollUpdate(): NULL device pointer " );

        OutputDevice&       rOutDev( mpOwningSpriteCanvas->getFrontBuffer()->getOutDev() );
        BackBufferSharedPtr pBackBuffer( mpOwningSpriteCanvas->getBackBuffer() );
        OutputDevice&       rBackOutDev( pBackBuffer->getOutDev() );

        const Size&                rTargetSizePixel( rOutDev.GetOutputSizePixel() );
        const ::basegfx::B2IRange  aOutputBounds( 0,0,
                                                  rTargetSizePixel.Width(),
                                                  rTargetSizePixel.Height() );

        
        
        
        
        
        ::basegfx::B2IRange aSourceRect(
            ::canvas::tools::spritePixelAreaFromB2DRange( rMoveStart ) );
        const ::basegfx::B2IRange& rDestRect(
            ::canvas::tools::spritePixelAreaFromB2DRange( rMoveEnd ) );
        ::basegfx::B2IPoint aDestPos( rDestRect.getMinimum() );

        ::std::vector< ::basegfx::B2IRange > aUnscrollableAreas;

        
        
        
        

        
        
        if( !mbIsUnsafeScrolling ||
            !::canvas::tools::clipScrollArea( aSourceRect,
                                              aDestPos,
                                              aUnscrollableAreas,
                                              aOutputBounds ) )
        {
            
            
            
            

            
            ::std::for_each( rUpdateArea.maComponentList.begin(),
                             rUpdateArea.maComponentList.end(),
                             ::boost::bind(
                                 &spriteRedrawStub3,
                                 ::boost::ref( rOutDev ),
                                 _1 ) );
        }
        else
        {
            
            rOutDev.CopyArea( ::vcl::unotools::pointFromB2IPoint( aDestPos ),
                              ::vcl::unotools::pointFromB2IPoint( aSourceRect.getMinimum() ),
                              
                              ::Size( static_cast<sal_Int32>(aSourceRect.getRange().getX()),
                                      static_cast<sal_Int32>(aSourceRect.getRange().getY()) ) );

            const ::canvas::SpriteRedrawManager::SpriteConnectedRanges::ComponentListType::const_iterator
                aFirst( rUpdateArea.maComponentList.begin() );

            ENSURE_OR_THROW( aFirst->second.getSprite().is(),
                              "VCLCanvas::scrollUpdate(): no sprite" );

            
            
            
            rOutDev.Push( PUSH_CLIPREGION );
            ::std::for_each( aUnscrollableAreas.begin(),
                             aUnscrollableAreas.end(),
                             ::boost::bind( &opaqueUpdateSpriteArea,
                                            ::boost::cref(aFirst->second.getSprite()),
                                            ::boost::ref(rOutDev),
                                            _1 ) );
            rOutDev.Pop();
        }

        
        
        
        ::std::vector< ::basegfx::B2DRange > aUncoveredAreas;
        ::basegfx::computeSetDifference( aUncoveredAreas,
                                         rUpdateArea.maTotalBounds,
                                         ::basegfx::B2DRange( rDestRect ) );
        ::std::for_each( aUncoveredAreas.begin(),
                         aUncoveredAreas.end(),
                         ::boost::bind( &repaintBackground,
                                        ::boost::ref(rOutDev),
                                        ::boost::ref(rBackOutDev),
                                        _1 ) );
    }

    void SpriteCanvasHelper::opaqueUpdate( SAL_UNUSED_PARAMETER const ::basegfx::B2DRange&,
                                           const ::std::vector< ::canvas::Sprite::Reference >& rSortedUpdateSprites )
    {
        ENSURE_OR_THROW( mpOwningSpriteCanvas &&
                         mpOwningSpriteCanvas->getBackBuffer() &&
                         mpOwningSpriteCanvas->getFrontBuffer(),
                         "SpriteCanvasHelper::opaqueUpdate(): NULL device pointer " );

        OutputDevice& rOutDev( mpOwningSpriteCanvas->getFrontBuffer()->getOutDev() );

        
        
        
        
        
        

        
        ::std::for_each( rSortedUpdateSprites.begin(),
                         rSortedUpdateSprites.end(),
                         ::boost::bind(
                             &spriteRedrawStub,
                             ::boost::ref( rOutDev ),
                             _1 ) );
    }

    void SpriteCanvasHelper::genericUpdate( const ::basegfx::B2DRange&                          rRequestedArea,
                                            const ::std::vector< ::canvas::Sprite::Reference >& rSortedUpdateSprites )
    {
        ENSURE_OR_THROW( mpOwningSpriteCanvas &&
                         mpOwningSpriteCanvas->getBackBuffer() &&
                         mpOwningSpriteCanvas->getFrontBuffer(),
                         "SpriteCanvasHelper::genericUpdate(): NULL device pointer " );

        OutputDevice&       rOutDev( mpOwningSpriteCanvas->getFrontBuffer()->getOutDev() );
        BackBufferSharedPtr pBackBuffer( mpOwningSpriteCanvas->getBackBuffer() );
        OutputDevice&       rBackOutDev( pBackBuffer->getOutDev() );

        
        const Size& rTargetSizePixel( rOutDev.GetOutputSizePixel() );

        
        
        
        
        const ::Point aOutputPosition(
            ::std::max( sal_Int32( 0 ),
                        static_cast< sal_Int32 >(rRequestedArea.getMinX()) ),
            ::std::max( sal_Int32( 0 ),
                        static_cast< sal_Int32 >(rRequestedArea.getMinY()) ) );
        
        
        
        
        const ::Size aOutputSize(
            ::std::max( sal_Int32( 0 ),
                        ::std::min( static_cast< sal_Int32 >(rTargetSizePixel.Width() - aOutputPosition.X()),
                                    ::canvas::tools::roundUp( rRequestedArea.getMaxX() - aOutputPosition.X() ))),
            ::std::max( sal_Int32( 0 ),
                        ::std::min( static_cast< sal_Int32 >(rTargetSizePixel.Height() - aOutputPosition.Y()),
                                    ::canvas::tools::roundUp( rRequestedArea.getMaxY() - aOutputPosition.Y() ))));

        
        if( aOutputSize.Width() == 0 &&
            aOutputSize.Height() == 0 )
        {
            return;
        }

        const Point aEmptyPoint(0,0);
        const Size  aCurrOutputSize( maVDev->GetOutputSizePixel() );

        
        
        if( aCurrOutputSize.Width() < aOutputSize.Width() ||
            aCurrOutputSize.Height() < aOutputSize.Height() )
        {
            
            
            
            
            
            maVDev->SetOutputSizePixel( aOutputSize );
        }

        
        maVDev->EnableMapMode( false );
        maVDev->SetAntialiasing( ANTIALIASING_ENABLE_B2DDRAW );
        maVDev->SetClipRegion();
        maVDev->DrawOutDev( aEmptyPoint, aOutputSize,
                            aOutputPosition, aOutputSize,
                            rBackOutDev );

        
        
        ::std::for_each( rSortedUpdateSprites.begin(),
                         rSortedUpdateSprites.end(),
                         ::boost::bind( &spriteRedrawStub2,
                                        ::boost::ref( maVDev.get() ),
                                        ::boost::cref(
                                            ::vcl::unotools::b2DPointFromPoint(aOutputPosition)),
                                        _1 ) );

        
        rOutDev.EnableMapMode( false );
        rOutDev.SetAntialiasing( ANTIALIASING_ENABLE_B2DDRAW );
        rOutDev.DrawOutDev( aOutputPosition, aOutputSize,
                            aEmptyPoint, aOutputSize,
                            *maVDev );
    }

    void SpriteCanvasHelper::renderFrameCounter( OutputDevice& rOutDev )
    {
        const double denominator( maLastUpdate.getElapsedTime() );
        maLastUpdate.reset();

        OUString text( ::rtl::math::doubleToUString( denominator == 0.0 ? 100.0 : 1.0/denominator,
                                                            rtl_math_StringFormat_F,
                                                            2,'.',NULL,' ') );

        
        while( text.getLength() < 6 )
            text = " " + text;

        text += " fps";

        renderInfoText( rOutDev,
                        text,
                        Point(0, 0) );
    }

    namespace
    {
        template< typename T > struct Adder
        {
            typedef void result_type;

            Adder( T& rAdderTarget,
                   T  nIncrement ) :
                mpTarget( &rAdderTarget ),
                mnIncrement( nIncrement )
            {
            }

            void operator()() { *mpTarget += mnIncrement; }
            void operator()( const ::canvas::Sprite::Reference& ) { *mpTarget += mnIncrement; }
            void operator()( T nIncrement ) { *mpTarget += nIncrement; }

            T* mpTarget;
            T  mnIncrement;
        };

        template< typename T> Adder<T> makeAdder( T& rAdderTarget,
                                                  T  nIncrement )
        {
            return Adder<T>(rAdderTarget, nIncrement);
        }
    }

    void SpriteCanvasHelper::renderSpriteCount( OutputDevice& rOutDev )
    {
        if( mpRedrawManager )
        {
            sal_Int32 nCount(0);

            mpRedrawManager->forEachSprite( makeAdder(nCount,sal_Int32(1)) );
            OUString text(
                OUString::number(
                    
                    nCount ) );

            
            while( text.getLength() < 3 )
                text = " " + text;

            text = "Sprites: " + text;

            renderInfoText( rOutDev,
                            text,
                            Point(0, 30) );
        }
    }

    void SpriteCanvasHelper::renderMemUsage( OutputDevice& rOutDev )
    {
        BackBufferSharedPtr pBackBuffer( mpOwningSpriteCanvas->getBackBuffer() );

        if( mpRedrawManager &&
            pBackBuffer )
        {
            double nPixel(0.0);

            
            mpRedrawManager->forEachSprite( ::boost::bind(
                                                makeAdder(nPixel,1.0),
                                                ::boost::bind(
                                                    &calcNumPixel,
                                                    _1 ) ) );

            static const int NUM_VIRDEV(2);
            static const int BYTES_PER_PIXEL(3);

            const Size& rVDevSize( maVDev->GetOutputSizePixel() );
            const Size& rBackBufferSize( pBackBuffer->getOutDev().GetOutputSizePixel() );

            const double nMemUsage( nPixel * NUM_VIRDEV * BYTES_PER_PIXEL +
                                    rVDevSize.Width()*rVDevSize.Height() * BYTES_PER_PIXEL +
                                    rBackBufferSize.Width()*rBackBufferSize.Height() * BYTES_PER_PIXEL );

            OUString text( ::rtl::math::doubleToUString( nMemUsage / 1048576.0,
                                                                rtl_math_StringFormat_F,
                                                                2,'.',NULL,' ') );

            
            while( text.getLength() < 4 )
                text = " " + text;

            text = "Mem: " + text + "MB";

            renderInfoText( rOutDev,
                            text,
                            Point(0, 60) );
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
