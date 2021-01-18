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
#include <impanmvw.hxx>

#include <vcl/virdev.hxx>
#include <vcl/window.hxx>
#include <tools/helpers.hxx>

#include <window.h>

ImplAnimView::ImplAnimView( Animation* pParent, OutputDevice* pOut,
                            const Point& rPt, const Size& rSz,
                            sal_uLong nExtraData,
                            OutputDevice* pFirstFrameOutDev ) :
        mpParent        ( pParent ),
        mpRenderContext ( pFirstFrameOutDev ? pFirstFrameOutDev : pOut ),
        mnExtraData     ( nExtraData ),
        maPt            ( rPt ),
        maSz            ( rSz ),
        maSzPix         ( mpRenderContext->LogicToPixel( maSz ) ),
        maClip          ( mpRenderContext->GetClipRegion() ),
        mpBackground    ( VclPtr<VirtualDevice>::Create() ),
        mpRestore       ( VclPtr<VirtualDevice>::Create() ),
        meLastDisposal  ( Disposal::Back ),
        mbIsPaused      ( false ),
        mbIsMarked      ( false ),
        mbIsMirroredHorizontally         ( maSz.Width() < 0 ),
        mbIsMirroredVertically         ( maSz.Height() < 0 )
{
    Animation::ImplIncAnimCount();

    // Mirrored horizontally?
    if( mbIsMirroredHorizontally )
    {
        maDispPt.setX( maPt.X() + maSz.Width() + 1 );
        maDispSz.setWidth( -maSz.Width() );
        maSzPix.setWidth( -maSzPix.Width() );
    }
    else
    {
        maDispPt.setX( maPt.X() );
        maDispSz.setWidth( maSz.Width() );
    }

    // Mirrored vertically?
    if( mbIsMirroredVertically )
    {
        maDispPt.setY( maPt.Y() + maSz.Height() + 1 );
        maDispSz.setHeight( -maSz.Height() );
        maSzPix.setHeight( -maSzPix.Height() );
    }
    else
    {
        maDispPt.setY( maPt.Y() );
        maDispSz.setHeight( maSz.Height() );
    }

    // save background
    mpBackground->SetOutputSizePixel( maSzPix );
    mpRenderContext->SaveBackground(*mpBackground, maDispPt, maDispSz, maSzPix);

    // Initialize drawing to actual position
    drawToPos( mpParent->ImplGetCurPos() );

    // If first frame OutputDevice is set, update variables now for real OutputDevice
    if( pFirstFrameOutDev )
    {
        mpRenderContext = pOut;
        maClip = mpRenderContext->GetClipRegion();
    }
}

ImplAnimView::~ImplAnimView()
{
    mpBackground.disposeAndClear();
    mpRestore.disposeAndClear();

    Animation::ImplDecAnimCount();
}

bool ImplAnimView::matches(const OutputDevice* pOut, tools::Long nExtraData) const
{
    return (!pOut || pOut == mpRenderContext) && (nExtraData == 0 || nExtraData == mnExtraData);
}

void ImplAnimView::getPosSize( const AnimationBitmap& rAnimationBitmap, Point& rPosPix, Size& rSizePix )
{
    const Size& rAnmSize = mpParent->GetDisplaySizePixel();
    Point       aPt2( rAnimationBitmap.maPositionPixel.X() + rAnimationBitmap.maSizePixel.Width() - 1,
                      rAnimationBitmap.maPositionPixel.Y() + rAnimationBitmap.maSizePixel.Height() - 1 );
    double      fFactX, fFactY;

    // calculate x scaling
    if( rAnmSize.Width() > 1 )
        fFactX = static_cast<double>( maSzPix.Width() - 1 ) / ( rAnmSize.Width() - 1 );
    else
        fFactX = 1.0;

    // calculate y scaling
    if( rAnmSize.Height() > 1 )
        fFactY = static_cast<double>( maSzPix.Height() - 1 ) / ( rAnmSize.Height() - 1 );
    else
        fFactY = 1.0;

    rPosPix.setX( FRound( rAnimationBitmap.maPositionPixel.X() * fFactX ) );
    rPosPix.setY( FRound( rAnimationBitmap.maPositionPixel.Y() * fFactY ) );

    aPt2.setX( FRound( aPt2.X() * fFactX ) );
    aPt2.setY( FRound( aPt2.Y() * fFactY ) );

    rSizePix.setWidth( aPt2.X() - rPosPix.X() + 1 );
    rSizePix.setHeight( aPt2.Y() - rPosPix.Y() + 1 );

    // Mirrored horizontally?
    if( mbIsMirroredHorizontally )
        rPosPix.setX( maSzPix.Width() - 1 - aPt2.X() );

    // Mirrored vertically?
    if( mbIsMirroredVertically )
        rPosPix.setY( maSzPix.Height() - 1 - aPt2.Y() );
}

void ImplAnimView::drawToPos( sal_uLong nPos )
{
    VclPtr<vcl::RenderContext> pRenderContext = mpRenderContext;

    std::unique_ptr<vcl::PaintBufferGuard> pGuard;
    if (mpRenderContext->GetOutDevType() == OUTDEV_WINDOW)
    {
        vcl::Window* pWindow = static_cast<vcl::WindowOutputDevice*>(mpRenderContext.get())->GetOwnerWindow();
        pGuard.reset(new vcl::PaintBufferGuard(pWindow->ImplGetWindowImpl()->mpFrameData, pWindow));
        pRenderContext = pGuard->GetRenderContext();
    }

    ScopedVclPtrInstance<VirtualDevice> aVDev;
    std::unique_ptr<vcl::Region> xOldClip(!maClip.IsNull() ? new vcl::Region( pRenderContext->GetClipRegion() ) : nullptr);

    aVDev->SetOutputSizePixel( maSzPix, false );
    nPos = std::min( nPos, static_cast<sal_uLong>(mpParent->Count()) - 1 );

    for( sal_uLong i = 0; i <= nPos; i++ )
        draw( i, aVDev.get() );

    if (xOldClip)
        pRenderContext->SetClipRegion( maClip );

    pRenderContext->DrawOutDev( maDispPt, maDispSz, Point(), maSzPix, *aVDev );
    if (pGuard)
        pGuard->SetPaintRect(tools::Rectangle(maDispPt, maDispSz));

    if (xOldClip)
        pRenderContext->SetClipRegion(*xOldClip);
}

void ImplAnimView::draw( sal_uLong nPos, VirtualDevice* pVDev )
{
    VclPtr<vcl::RenderContext> pRenderContext = mpRenderContext;

    std::unique_ptr<vcl::PaintBufferGuard> pGuard;
    if (!pVDev && mpRenderContext->GetOutDevType() == OUTDEV_WINDOW)
    {
        vcl::Window* pWindow = static_cast<vcl::WindowOutputDevice*>(mpRenderContext.get())->GetOwnerWindow();
        pGuard.reset(new vcl::PaintBufferGuard(pWindow->ImplGetWindowImpl()->mpFrameData, pWindow));
        pRenderContext = pGuard->GetRenderContext();
    }

    tools::Rectangle aOutRect( pRenderContext->PixelToLogic( Point() ), pRenderContext->GetOutputSize() );

    // check, if output lies out of display
    if( aOutRect.Intersection( tools::Rectangle( maDispPt, maDispSz ) ).IsEmpty() )
        setMarked( true );
    else if( !mbIsPaused )
    {
        VclPtr<VirtualDevice>   pDev;
        Point                   aPosPix;
        Point                   aBmpPosPix;
        Size                    aSizePix;
        Size                    aBmpSizePix;
        const sal_uLong             nLastPos = mpParent->Count() - 1;
        mnActPos = std::min( nPos, nLastPos );
        const AnimationBitmap&  rAnimationBitmap = mpParent->Get( static_cast<sal_uInt16>( mnActPos ) );

        getPosSize( rAnimationBitmap, aPosPix, aSizePix );

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
            pDev->SetOutputSizePixel( maSzPix, false );
            pDev->DrawOutDev( Point(), maSzPix, maDispPt, maDispSz, *pRenderContext );
        }
        else
            pDev = pVDev;

        // restore background after each run
        if( !nPos )
        {
            meLastDisposal = Disposal::Back;
            maRestPt = Point();
            maRestSz = maSzPix;
        }

        // restore
        if( ( Disposal::Not != meLastDisposal ) && maRestSz.Width() && maRestSz.Height() )
        {
            if( Disposal::Back == meLastDisposal )
                pDev->DrawOutDev( maRestPt, maRestSz, maRestPt, maRestSz, *mpBackground );
            else
                pDev->DrawOutDev( maRestPt, maRestSz, Point(), maRestSz, *mpRestore );
        }

        meLastDisposal = rAnimationBitmap.meDisposal;
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

        pDev->DrawBitmapEx( aBmpPosPix, aBmpSizePix, rAnimationBitmap.maBitmapEx );

        if( !pVDev )
        {
            std::unique_ptr<vcl::Region> xOldClip(!maClip.IsNull() ? new vcl::Region( pRenderContext->GetClipRegion() ) : nullptr);

            if (xOldClip)
                pRenderContext->SetClipRegion( maClip );

            pRenderContext->DrawOutDev( maDispPt, maDispSz, Point(), maSzPix, *pDev );
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

void ImplAnimView::repaint()
{
    const bool bOldPause = mbIsPaused;

    mpRenderContext->SaveBackground(*mpBackground, maDispPt, maDispSz, maSzPix);

    mbIsPaused = false;
    drawToPos( mnActPos );
    mbIsPaused = bOldPause;
}

AInfo* ImplAnimView::createAInfo() const
{
    AInfo* pAInfo = new AInfo;

    pAInfo->aStartOrg = maPt;
    pAInfo->aStartSize = maSz;
    pAInfo->pOutDev = mpRenderContext;
    pAInfo->pViewData = const_cast<ImplAnimView *>(this);
    pAInfo->nExtraData = mnExtraData;
    pAInfo->bPause = mbIsPaused;

    return pAInfo;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
