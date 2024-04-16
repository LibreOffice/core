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

#include <memory>
#include <animate/AnimationRenderer.hxx>

#include <vcl/virdev.hxx>
#include <vcl/window.hxx>
#include <tools/helpers.hxx>

#include <window.h>

AnimationRenderer::AnimationRenderer( Animation* pParent, OutputDevice* pOut,
                            const Point& rPt, const Size& rSz,
                            sal_uLong nRendererId,
                            OutputDevice* pFirstFrameOutDev ) :
        mpParent        ( pParent ),
        mpRenderContext ( pFirstFrameOutDev ? pFirstFrameOutDev : pOut ),
        mnRendererId    ( nRendererId ),
        maOriginPt      ( rPt ),
        maLogicalSize   ( rSz ),
        maSizePx        ( mpRenderContext->LogicToPixel( maLogicalSize ) ),
        maClip          ( mpRenderContext->GetClipRegion() ),
        mpBackground    ( VclPtr<VirtualDevice>::Create() ),
        mpRestore       ( VclPtr<VirtualDevice>::Create() ),
        mnActIndex      ( 0 ),
        meLastDisposal  ( Disposal::Back ),
        mbIsPaused      ( false ),
        mbIsMarked      ( false ),
        mbIsMirroredHorizontally( maLogicalSize.Width() < 0 ),
        mbIsMirroredVertically( maLogicalSize.Height() < 0 )
{
    Animation::ImplIncAnimCount();

    // Mirrored horizontally?
    if( mbIsMirroredHorizontally )
    {
        maDispPt.setX( maOriginPt.X() + maLogicalSize.Width() + 1 );
        maDispSz.setWidth( -maLogicalSize.Width() );
        maSizePx.setWidth( -maSizePx.Width() );
    }
    else
    {
        maDispPt.setX( maOriginPt.X() );
        maDispSz.setWidth( maLogicalSize.Width() );
    }

    // Mirrored vertically?
    if( mbIsMirroredVertically )
    {
        maDispPt.setY( maOriginPt.Y() + maLogicalSize.Height() + 1 );
        maDispSz.setHeight( -maLogicalSize.Height() );
        maSizePx.setHeight( -maSizePx.Height() );
    }
    else
    {
        maDispPt.setY( maOriginPt.Y() );
        maDispSz.setHeight( maLogicalSize.Height() );
    }

    // save background
    mpBackground->SetOutputSizePixel( maSizePx );
    mpRenderContext->SaveBackground(*mpBackground, maDispPt, maDispSz, maSizePx);

    // Initialize drawing to actual position
    drawToIndex( mpParent->ImplGetCurPos() );

    // If first frame OutputDevice is set, update variables now for real OutputDevice
    if( pFirstFrameOutDev )
    {
        mpRenderContext = pOut;
        maClip = mpRenderContext->GetClipRegion();
    }
}

AnimationRenderer::~AnimationRenderer()
{
    mpBackground.disposeAndClear();
    mpRestore.disposeAndClear();

    Animation::ImplDecAnimCount();
}

bool AnimationRenderer::matches(const OutputDevice* pOut, tools::Long nRendererId) const
{
    return (!pOut || pOut == mpRenderContext) && (nRendererId == 0 || nRendererId == mnRendererId);
}

void AnimationRenderer::getPosSize( const AnimationFrame& rAnimationFrame, Point& rPosPix, Size& rSizePix )
{
    const Size& rAnmSize = mpParent->GetDisplaySizePixel();
    Point       aPt2( rAnimationFrame.maPositionPixel.X() + rAnimationFrame.maSizePixel.Width() - 1,
                      rAnimationFrame.maPositionPixel.Y() + rAnimationFrame.maSizePixel.Height() - 1 );
    double      fFactX, fFactY;

    // calculate x scaling
    if( rAnmSize.Width() > 1 )
        fFactX = static_cast<double>( maSizePx.Width() - 1 ) / ( rAnmSize.Width() - 1 );
    else
        fFactX = 1.0;

    // calculate y scaling
    if( rAnmSize.Height() > 1 )
        fFactY = static_cast<double>( maSizePx.Height() - 1 ) / ( rAnmSize.Height() - 1 );
    else
        fFactY = 1.0;

    rPosPix.setX(basegfx::fround<tools::Long>(rAnimationFrame.maPositionPixel.X() * fFactX));
    rPosPix.setY(basegfx::fround<tools::Long>(rAnimationFrame.maPositionPixel.Y() * fFactY));

    aPt2.setX(basegfx::fround<tools::Long>(aPt2.X() * fFactX));
    aPt2.setY(basegfx::fround<tools::Long>(aPt2.Y() * fFactY));

    rSizePix.setWidth( aPt2.X() - rPosPix.X() + 1 );
    rSizePix.setHeight( aPt2.Y() - rPosPix.Y() + 1 );

    // Mirrored horizontally?
    if( mbIsMirroredHorizontally )
        rPosPix.setX( maSizePx.Width() - 1 - aPt2.X() );

    // Mirrored vertically?
    if( mbIsMirroredVertically )
        rPosPix.setY( maSizePx.Height() - 1 - aPt2.Y() );
}

void AnimationRenderer::drawToIndex( sal_uLong nIndex )
{
    VclPtr<vcl::RenderContext> pRenderContext = mpRenderContext;

    vcl::PaintBufferGuardPtr pGuard;
    if (mpRenderContext->GetOutDevType() == OUTDEV_WINDOW)
    {
        vcl::Window* pWindow = static_cast<vcl::WindowOutputDevice*>(mpRenderContext.get())->GetOwnerWindow();
        pGuard.reset(new vcl::PaintBufferGuard(pWindow->ImplGetWindowImpl()->mpFrameData, pWindow));
        pRenderContext = pGuard->GetRenderContext();
    }

    ScopedVclPtrInstance<VirtualDevice> aVDev;
    std::optional<vcl::Region> xOldClip;
    if (!maClip.IsNull())
        xOldClip = pRenderContext->GetClipRegion();

    aVDev->SetOutputSizePixel( maSizePx, false );
    nIndex = std::min( nIndex, static_cast<sal_uLong>(mpParent->Count()) - 1 );

    for( sal_uLong i = 0; i <= nIndex; i++ )
        draw( i, aVDev.get() );

    if (xOldClip)
        pRenderContext->SetClipRegion( maClip );

    pRenderContext->DrawOutDev( maDispPt, maDispSz, Point(), maSizePx, *aVDev );
    if (pGuard)
        pGuard->SetPaintRect(tools::Rectangle(maDispPt, maDispSz));

    if (xOldClip)
        pRenderContext->SetClipRegion(*xOldClip);
}

void AnimationRenderer::draw( sal_uLong nIndex, VirtualDevice* pVDev )
{
    VclPtr<vcl::RenderContext> pRenderContext = mpRenderContext;

    vcl::PaintBufferGuardPtr pGuard;
    if (!pVDev && mpRenderContext->GetOutDevType() == OUTDEV_WINDOW)
    {
        vcl::Window* pWindow = static_cast<vcl::WindowOutputDevice*>(mpRenderContext.get())->GetOwnerWindow();
        pGuard.reset(new vcl::PaintBufferGuard(pWindow->ImplGetWindowImpl()->mpFrameData, pWindow));
        pRenderContext = pGuard->GetRenderContext();
    }

    tools::Rectangle aOutRect( pRenderContext->PixelToLogic( Point() ), pRenderContext->GetOutputSize() );

    // check, if output lies out of display
    if( aOutRect.Intersection( tools::Rectangle( maDispPt, maDispSz ) ).IsEmpty() )
    {
        setMarked( true );
    }
    else if( !mbIsPaused )
    {
        VclPtr<VirtualDevice>   pDev;
        Point                   aPosPix;
        Point                   aBmpPosPix;
        Size                    aSizePix;
        Size                    aBmpSizePix;
        const sal_uLong             nLastPos = mpParent->Count() - 1;
        mnActIndex = std::min( nIndex, nLastPos );
        const AnimationFrame&  rAnimationFrame = mpParent->Get( static_cast<sal_uInt16>( mnActIndex ) );

        getPosSize( rAnimationFrame, aPosPix, aSizePix );

        // Mirrored horizontally?
        if( mbIsMirroredHorizontally )
        {
            aBmpPosPix.setX( aPosPix.X() + aSizePix.Width() - 1 );
            aBmpSizePix.setWidth( -aSizePix.Width() );
        }
        else
        {
            aBmpPosPix.setX( aPosPix.X() );
            aBmpSizePix.setWidth( aSizePix.Width() );
        }

        // Mirrored vertically?
        if( mbIsMirroredVertically )
        {
            aBmpPosPix.setY( aPosPix.Y() + aSizePix.Height() - 1 );
            aBmpSizePix.setHeight( -aSizePix.Height() );
        }
        else
        {
            aBmpPosPix.setY( aPosPix.Y() );
            aBmpSizePix.setHeight( aSizePix.Height() );
        }

        // get output device
        if( !pVDev )
        {
            pDev = VclPtr<VirtualDevice>::Create();
            pDev->SetOutputSizePixel( maSizePx, false );
            pDev->DrawOutDev( Point(), maSizePx, maDispPt, maDispSz, *pRenderContext );
        }
        else
            pDev = pVDev;

        // restore background after each run
        if( !nIndex )
        {
            meLastDisposal = Disposal::Back;
            maRestPt = Point();
            maRestSz = maSizePx;
        }

        // restore
        if( ( Disposal::Not != meLastDisposal ) && maRestSz.Width() && maRestSz.Height() )
        {
            if( Disposal::Back == meLastDisposal )
                pDev->DrawOutDev( maRestPt, maRestSz, maRestPt, maRestSz, *mpBackground );
            else
                pDev->DrawOutDev( maRestPt, maRestSz, Point(), maRestSz, *mpRestore );
        }

        meLastDisposal = rAnimationFrame.meDisposal;
        maRestPt = aPosPix;
        maRestSz = aSizePix;

        // What do we need to restore the next time?
        // Put it into a bitmap if needed, else delete
        // SaveBitmap to conserve memory
        if( ( meLastDisposal == Disposal::Back ) || ( meLastDisposal == Disposal::Not ) )
            mpRestore->SetOutputSizePixel( Size( 1, 1 ), false );
        else
        {
            mpRestore->SetOutputSizePixel( maRestSz, false );
            mpRestore->DrawOutDev( Point(), maRestSz, aPosPix, aSizePix, *pDev );
        }

        pDev->DrawBitmapEx( aBmpPosPix, aBmpSizePix, rAnimationFrame.maBitmapEx );

        if( !pVDev )
        {
            std::optional<vcl::Region> xOldClip;
            if (!maClip.IsNull())
                xOldClip = pRenderContext->GetClipRegion();

            if (xOldClip)
                pRenderContext->SetClipRegion( maClip );

            pRenderContext->DrawOutDev( maDispPt, maDispSz, Point(), maSizePx, *pDev );
            if (pGuard)
                pGuard->SetPaintRect(tools::Rectangle(maDispPt, maDispSz));

            if( xOldClip)
            {
                pRenderContext->SetClipRegion(*xOldClip);
                xOldClip.reset();
            }

            pDev.disposeAndClear();
            pRenderContext->Flush();
        }
    }
}

void AnimationRenderer::repaint()
{
    const bool bOldPause = mbIsPaused;

    mpRenderContext->SaveBackground(*mpBackground, maDispPt, maDispSz, maSizePx);

    mbIsPaused = false;
    drawToIndex( mnActIndex );
    mbIsPaused = bOldPause;
}

AnimationData* AnimationRenderer::createAnimationData() const
{
    AnimationData* pDataItem = new AnimationData;

    pDataItem->maOriginStartPt = maOriginPt;
    pDataItem->maStartSize = maLogicalSize;
    pDataItem->mpRenderContext = mpRenderContext;
    pDataItem->mpRendererData = const_cast<AnimationRenderer *>(this);
    pDataItem->mnRendererId = mnRendererId;
    pDataItem->mbIsPaused = mbIsPaused;

    return pDataItem;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
