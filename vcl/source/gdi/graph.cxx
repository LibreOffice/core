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

#include <vcl/outdev.hxx>
#include <vcl/svapp.hxx>
#include <vcl/graph.hxx>
#include <vcl/metaact.hxx>
#include <impgraph.hxx>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/graphic/GraphicProvider.hpp>
#include <com/sun/star/graphic/XGraphicProvider.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>

using namespace ::com::sun::star;

static void ImplDrawDefault( OutputDevice* pOutDev, const OUString* pText,
                             Font* pFont, const Bitmap* pBitmap, const BitmapEx* pBitmapEx,
                             const Point& rDestPt, const Size& rDestSize )
{
    sal_uInt16      nPixel = (sal_uInt16) pOutDev->PixelToLogic( Size( 1, 1 ) ).Width();
    sal_uInt16      nPixelWidth = nPixel;
    Point       aPoint( rDestPt.X() + nPixelWidth, rDestPt.Y() + nPixelWidth );
    Size        aSize( rDestSize.Width() - ( nPixelWidth << 1 ), rDestSize.Height() - ( nPixelWidth << 1 ) );
    bool        bFilled = ( pBitmap != NULL || pBitmapEx != NULL || pFont != NULL );
    Rectangle   aBorderRect( aPoint, aSize );

    pOutDev->Push();

    pOutDev->SetFillColor();

    // On the printer a black rectangle and on the screen one with 3D effect
    if ( pOutDev->GetOutDevType() == OUTDEV_PRINTER )
        pOutDev->SetLineColor( COL_BLACK );
    else
    {
        aBorderRect.Left() += nPixel;
        aBorderRect.Top() += nPixel;

        pOutDev->SetLineColor( COL_LIGHTGRAY );
        pOutDev->DrawRect( aBorderRect );

        aBorderRect.Left() -= nPixel;
        aBorderRect.Top() -= nPixel;
        aBorderRect.Right() -= nPixel;
        aBorderRect.Bottom() -= nPixel;
        pOutDev->SetLineColor( COL_GRAY );
    }

    pOutDev->DrawRect( aBorderRect );

    aPoint.X() += nPixelWidth + 2*nPixel;
    aPoint.Y() += nPixelWidth + 2*nPixel;
    aSize.Width() -= 2*nPixelWidth + 4*nPixel;
    aSize.Height() -= 2*nPixelWidth + 4*nPixel;

    if( aSize.Width() > 0 && aSize.Height() > 0
        && (  ( pBitmap && !!*pBitmap ) || ( pBitmapEx && !!*pBitmapEx ) ) )
    {
        Size aBitmapSize( pOutDev->PixelToLogic( pBitmap ? pBitmap->GetSizePixel() : pBitmapEx->GetSizePixel() ) );

        if( aSize.Height() > aBitmapSize.Height() && aSize.Width() > aBitmapSize.Width() )
        {
            if ( pBitmap )
                pOutDev->DrawBitmap( aPoint, *pBitmap );
            else
                pOutDev->DrawBitmapEx( aPoint, *pBitmapEx );
            aPoint.X() += aBitmapSize.Width() + 2*nPixel;
            aSize.Width() -= aBitmapSize.Width() + 2*nPixel;
        }
    }

    if ( aSize.Width() > 0 && aSize.Height() > 0 && pFont && pText && pText->getLength()
         && !(!pOutDev->IsOutputEnabled() /*&& pOutDev->GetConnectMetaFile() */) )
    {
        MapMode aMapMode( MAP_POINT );
        Size    aSz = pOutDev->LogicToLogic( Size( 0, 12 ), &aMapMode, NULL );
        long    nThreshold = aSz.Height() / 2;
        long    nStep = nThreshold / 3;

        if ( !nStep )
            nStep = aSz.Height() - nThreshold;

        for(;; aSz.Height() -= nStep )
        {
            pFont->SetSize( aSz );
            pOutDev->SetFont( *pFont );

            long nTextHeight = pOutDev->GetTextHeight();
            long nTextWidth = pOutDev->GetTextWidth( *pText );
            if ( nTextHeight )
            {
                // The approximation does not respect imprecisions caused
                // by word wraps
                long nLines = aSize.Height() / nTextHeight;
                long nWidth = aSize.Width() * nLines; // Approximation!!!

                if ( nTextWidth <= nWidth || aSz.Height() <= nThreshold )
                {
                    sal_uInt16 nStart = 0;
                    sal_uInt16 nLen = 0;

                    while( nStart < pText->getLength() && (*pText)[nStart] == ' ' )
                        nStart++;
                    while( nStart+nLen < pText->getLength() && (*pText)[nStart+nLen] != ' ' )
                        nLen++;
                    while( nStart < pText->getLength() && nLines-- )
                    {
                        sal_uInt16 nNext = nLen;
                        do
                        {
                            while ( nStart+nNext < pText->getLength() && (*pText)[nStart+nNext] == ' ' )
                                nNext++;
                            while ( nStart+nNext < pText->getLength() && (*pText)[nStart+nNext] != ' ' )
                                nNext++;
                            nTextWidth = pOutDev->GetTextWidth( *pText, nStart, nNext );
                            if ( nTextWidth > aSize.Width() )
                                break;
                            nLen = nNext;
                        }
                        while ( nStart+nNext < pText->getLength() );

                        sal_uInt16 n = nLen;
                        nTextWidth = pOutDev->GetTextWidth( *pText, nStart, n );
                        while( nTextWidth > aSize.Width() )
                            nTextWidth = pOutDev->GetTextWidth( *pText, nStart, --n );
                        pOutDev->DrawText( aPoint, *pText, nStart, n );

                        aPoint.Y() += nTextHeight;
                        nStart      = sal::static_int_cast<sal_uInt16>(nStart + nLen);
                        nLen        = nNext-nLen;
                        while( nStart < pText->getLength() && (*pText)[nStart] == ' ' )
                        {
                            nStart++;
                            nLen--;
                        }
                    }
                    break;
                }
            }
            else
                break;
        }
    }

    // If the default graphic does not have content, we draw a red rectangle
    if( !bFilled )
    {
        aBorderRect.Left()++;
        aBorderRect.Top()++;
        aBorderRect.Right()--;
        aBorderRect.Bottom()--;

        pOutDev->SetLineColor( COL_LIGHTRED );
        pOutDev->DrawLine( aBorderRect.TopLeft(), aBorderRect.BottomRight() );
        pOutDev->DrawLine( aBorderRect.TopRight(), aBorderRect.BottomLeft() );
    }

    pOutDev->Pop();
}

TYPEINIT1_AUTOFACTORY( Graphic, SvDataCopyStream );

Graphic::Graphic()
{
    mpImpGraphic = new ImpGraphic;
}

Graphic::Graphic( const Graphic& rGraphic ) :
SvDataCopyStream()
{
    if( rGraphic.IsAnimated() )
        mpImpGraphic = new ImpGraphic( *rGraphic.mpImpGraphic );
    else
    {
        mpImpGraphic = rGraphic.mpImpGraphic;
        mpImpGraphic->mnRefCount++;
    }
}

Graphic::Graphic( const Bitmap& rBmp )
{
    mpImpGraphic = new ImpGraphic( rBmp );
}

Graphic::Graphic( const BitmapEx& rBmpEx )
{
    mpImpGraphic = new ImpGraphic( rBmpEx );
}

Graphic::Graphic(const SvgDataPtr& rSvgDataPtr)
{
    mpImpGraphic = new ImpGraphic(rSvgDataPtr);
}

Graphic::Graphic( const Animation& rAnimation )
{
    mpImpGraphic = new ImpGraphic( rAnimation );
}

Graphic::Graphic( const GDIMetaFile& rMtf )
{
    mpImpGraphic = new ImpGraphic( rMtf );
}

Graphic::Graphic( const ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XGraphic >& rxGraphic )
{
    uno::Reference< lang::XUnoTunnel >      xTunnel( rxGraphic, uno::UNO_QUERY );
    uno::Reference< lang::XTypeProvider >   xProv( rxGraphic, uno::UNO_QUERY );
    const ::Graphic*                        pGraphic = ( ( xTunnel.is() && xProv.is() ) ?
                                                         reinterpret_cast< ::Graphic* >( xTunnel->getSomething( xProv->getImplementationId() ) ) :
                                                          NULL );

    if( pGraphic )
    {
        if( pGraphic->IsAnimated() )
            mpImpGraphic = new ImpGraphic( *pGraphic->mpImpGraphic );
        else
        {
            mpImpGraphic = pGraphic->mpImpGraphic;
            mpImpGraphic->mnRefCount++;
        }
    }
    else
        mpImpGraphic = new ImpGraphic;
}

Graphic::~Graphic()
{
    if( mpImpGraphic->mnRefCount == 1UL )
        delete mpImpGraphic;
    else
        mpImpGraphic->mnRefCount--;
}

void Graphic::ImplTestRefCount()
{
    if( mpImpGraphic->mnRefCount > 1UL )
    {
        mpImpGraphic->mnRefCount--;
        mpImpGraphic = new ImpGraphic( *mpImpGraphic );
    }
}

Graphic& Graphic::operator=( const Graphic& rGraphic )
{
    if( &rGraphic != this )
    {
        if( rGraphic.IsAnimated() )
        {
            if( mpImpGraphic->mnRefCount == 1UL )
                delete mpImpGraphic;
            else
                mpImpGraphic->mnRefCount--;

            mpImpGraphic = new ImpGraphic( *rGraphic.mpImpGraphic );
        }
        else
        {
            rGraphic.mpImpGraphic->mnRefCount++;

            if( mpImpGraphic->mnRefCount == 1UL )
                delete mpImpGraphic;
            else
                mpImpGraphic->mnRefCount--;

            mpImpGraphic = rGraphic.mpImpGraphic;
        }
    }

    return *this;
}

sal_Bool Graphic::operator==( const Graphic& rGraphic ) const
{
    return( *mpImpGraphic == *rGraphic.mpImpGraphic );
}

sal_Bool Graphic::operator!=( const Graphic& rGraphic ) const
{
    return( *mpImpGraphic != *rGraphic.mpImpGraphic );
}

sal_Bool Graphic::operator!() const
{
    return( GRAPHIC_NONE == mpImpGraphic->ImplGetType() );
}

void Graphic::Load( SvStream& rIStm )
{
    rIStm >> *this;
}

void Graphic::Save( SvStream& rOStm )
{
    rOStm << *this;
}

void Graphic::Assign( const SvDataCopyStream& rCopyStream )
{
    *this = (const Graphic& ) rCopyStream;
}

void Graphic::Clear()
{
    ImplTestRefCount();
    mpImpGraphic->ImplClear();
}

GraphicType Graphic::GetType() const
{
    return mpImpGraphic->ImplGetType();
}

void Graphic::SetDefaultType()
{
    ImplTestRefCount();
    mpImpGraphic->ImplSetDefaultType();
}

sal_Bool Graphic::IsSupportedGraphic() const
{
    return mpImpGraphic->ImplIsSupportedGraphic();
}

sal_Bool Graphic::IsTransparent() const
{
    return mpImpGraphic->ImplIsTransparent();
}

sal_Bool Graphic::IsAlpha() const
{
    return mpImpGraphic->ImplIsAlpha();
}

sal_Bool Graphic::IsAnimated() const
{
    return mpImpGraphic->ImplIsAnimated();
}

sal_Bool Graphic::IsEPS() const
{
    return mpImpGraphic->ImplIsEPS();
}

Bitmap Graphic::GetBitmap(const GraphicConversionParameters& rParameters) const
{
    return mpImpGraphic->ImplGetBitmap(rParameters);
}

BitmapEx Graphic::GetBitmapEx(const GraphicConversionParameters& rParameters) const
{
    return mpImpGraphic->ImplGetBitmapEx(rParameters);
}

Animation Graphic::GetAnimation() const
{
    return mpImpGraphic->ImplGetAnimation();
}

const GDIMetaFile& Graphic::GetGDIMetaFile() const
{
    return mpImpGraphic->ImplGetGDIMetaFile();
}

uno::Reference< graphic::XGraphic > Graphic::GetXGraphic() const
{
    uno::Reference< graphic::XGraphic > xRet;

    if( GetType() != GRAPHIC_NONE )
    {
        uno::Reference < uno::XComponentContext > xContext( ::comphelper::getProcessComponentContext() );
        uno::Reference< graphic::XGraphicProvider > xProv( graphic::GraphicProvider::create( xContext ) );

        uno::Sequence< beans::PropertyValue >   aLoadProps( 1 );
        OUString                                aURL( "private:memorygraphic/" );

        aLoadProps[ 0 ].Name = OUString( "URL" );
        aLoadProps[ 0 ].Value <<= ( aURL += OUString::number( reinterpret_cast< sal_Int64 >( this ) ) );

        xRet = xProv->queryGraphic( aLoadProps );
    }

    return xRet;
}

Size Graphic::GetPrefSize() const
{
    return mpImpGraphic->ImplGetPrefSize();
}

void Graphic::SetPrefSize( const Size& rPrefSize )
{
    ImplTestRefCount();
    mpImpGraphic->ImplSetPrefSize( rPrefSize );
}

MapMode Graphic::GetPrefMapMode() const
{
    return mpImpGraphic->ImplGetPrefMapMode();
}

void Graphic::SetPrefMapMode( const MapMode& rPrefMapMode )
{
    ImplTestRefCount();
    mpImpGraphic->ImplSetPrefMapMode( rPrefMapMode );
}

Size Graphic::GetSizePixel( const OutputDevice* pRefDevice ) const
{
    Size aRet;

    if( GRAPHIC_BITMAP == mpImpGraphic->ImplGetType() )
        aRet = mpImpGraphic->ImplGetBitmapEx(GraphicConversionParameters()).GetSizePixel();
    else
        aRet = ( pRefDevice ? pRefDevice : Application::GetDefaultDevice() )->LogicToPixel( GetPrefSize(), GetPrefMapMode() );

    return aRet;
}

sal_uLong Graphic::GetSizeBytes() const
{
    return mpImpGraphic->ImplGetSizeBytes();
}

void Graphic::Draw( OutputDevice* pOutDev, const Point& rDestPt ) const
{
    mpImpGraphic->ImplDraw( pOutDev, rDestPt );
}

void Graphic::Draw( OutputDevice* pOutDev,
                    const Point& rDestPt, const Size& rDestSz ) const
{
    if( GRAPHIC_DEFAULT == mpImpGraphic->ImplGetType() )
        ImplDrawDefault( pOutDev, NULL, NULL, NULL, NULL, rDestPt, rDestSz );
    else
        mpImpGraphic->ImplDraw( pOutDev, rDestPt, rDestSz );
}

void Graphic::DrawEx( OutputDevice* pOutDev, const OUString& rText,
                    Font& rFont, const BitmapEx& rBitmap,
                    const Point& rDestPt, const Size& rDestSz )
{
    ImplDrawDefault( pOutDev, &rText, &rFont, NULL, &rBitmap, rDestPt, rDestSz );
}

void Graphic::StartAnimation( OutputDevice* pOutDev, const Point& rDestPt,
                              const Size& rDestSz, long nExtraData,
                              OutputDevice* pFirstFrameOutDev )
{
    ImplTestRefCount();
    mpImpGraphic->ImplStartAnimation( pOutDev, rDestPt, rDestSz, nExtraData, pFirstFrameOutDev );
}

void Graphic::StopAnimation( OutputDevice* pOutDev, long nExtraData )
{
    ImplTestRefCount();
    mpImpGraphic->ImplStopAnimation( pOutDev, nExtraData );
}

void Graphic::SetAnimationNotifyHdl( const Link& rLink )
{
    mpImpGraphic->ImplSetAnimationNotifyHdl( rLink );
}

Link Graphic::GetAnimationNotifyHdl() const
{
    return mpImpGraphic->ImplGetAnimationNotifyHdl();
}

sal_uLong Graphic::GetAnimationLoopCount() const
{
    return mpImpGraphic->ImplGetAnimationLoopCount();
}

GraphicReader* Graphic::GetContext()
{
    return mpImpGraphic->ImplGetContext();
}

void Graphic::SetContext( GraphicReader* pReader )
{
    mpImpGraphic->ImplSetContext( pReader );
}

void Graphic::SetDocFileName( const OUString& rName, sal_uLong nFilePos )
{
    mpImpGraphic->ImplSetDocFileName( rName, nFilePos );
}

const OUString& Graphic::GetDocFileName() const
{
    return mpImpGraphic->ImplGetDocFileName();
}

sal_uLong Graphic::GetDocFilePos() const
{
    return mpImpGraphic->ImplGetDocFilePos();
}

sal_Bool Graphic::SwapOut()
{
    ImplTestRefCount();
    return mpImpGraphic->ImplSwapOut();
}

sal_Bool Graphic::SwapOut( SvStream* pOStream )
{
    ImplTestRefCount();
    return mpImpGraphic->ImplSwapOut( pOStream );
}

sal_Bool Graphic::SwapIn()
{
    ImplTestRefCount();
    return mpImpGraphic->ImplSwapIn();
}

sal_Bool Graphic::SwapIn( SvStream* pStrm )
{
    ImplTestRefCount();
    return mpImpGraphic->ImplSwapIn( pStrm );
}

sal_Bool Graphic::IsSwapOut() const
{
    return mpImpGraphic->ImplIsSwapOut();
}

void Graphic::SetLink( const GfxLink& rGfxLink )
{
    ImplTestRefCount();
    mpImpGraphic->ImplSetLink( rGfxLink );
}

GfxLink Graphic::GetLink() const
{
    return mpImpGraphic->ImplGetLink();
}

sal_Bool Graphic::IsLink() const
{
    return mpImpGraphic->ImplIsLink();
}

sal_uLong Graphic::GetChecksum() const
{
    return mpImpGraphic->ImplGetChecksum();
}

sal_Bool Graphic::ExportNative( SvStream& rOStream ) const
{
    return mpImpGraphic->ImplExportNative( rOStream );
}

SvStream& operator>>( SvStream& rIStream, Graphic& rGraphic )
{
    rGraphic.ImplTestRefCount();
    return rIStream >> *rGraphic.mpImpGraphic;
}

SvStream& operator<<( SvStream& rOStream, const Graphic& rGraphic )
{
    return rOStream << *rGraphic.mpImpGraphic;
}

const SvgDataPtr& Graphic::getSvgData() const
{
    return mpImpGraphic->getSvgData();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
