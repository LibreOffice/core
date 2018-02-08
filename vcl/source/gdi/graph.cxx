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

#include <tools/fract.hxx>
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
#include <cppuhelper/typeprovider.hxx>

using namespace ::com::sun::star;

namespace
{

void ImplDrawDefault( OutputDevice* pOutDev, const OUString* pText,
                             vcl::Font* pFont, const BitmapEx* pBitmapEx,
                             const Point& rDestPt, const Size& rDestSize )
{
    sal_uInt16  nPixel = static_cast<sal_uInt16>(pOutDev->PixelToLogic( Size( 1, 1 ) ).Width());
    sal_uInt16  nPixelWidth = nPixel;
    Point       aPoint( rDestPt.X() + nPixelWidth, rDestPt.Y() + nPixelWidth );
    Size        aSize( rDestSize.Width() - ( nPixelWidth << 1 ), rDestSize.Height() - ( nPixelWidth << 1 ) );
    bool        bFilled = ( pBitmapEx != nullptr || pFont != nullptr );
    tools::Rectangle   aBorderRect( aPoint, aSize );

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
        && ( pBitmapEx && !!*pBitmapEx ) )
    {
        Size aBitmapSize( pOutDev->PixelToLogic( pBitmapEx->GetSizePixel() ) );

        if( aSize.Height() > aBitmapSize.Height() && aSize.Width() > aBitmapSize.Width() )
        {
            pOutDev->DrawBitmapEx( aPoint, *pBitmapEx );
            aPoint.X() += aBitmapSize.Width() + 2*nPixel;
            aSize.Width() -= aBitmapSize.Width() + 2*nPixel;
        }
    }

    if ( aSize.Width() > 0 && aSize.Height() > 0 && pFont && pText && pText->getLength()
         && pOutDev->IsOutputEnabled() )
    {
        MapMode aMapMode( MapUnit::MapPoint );
        Size    aSz = pOutDev->LogicToLogic( Size( 0, 12 ), &aMapMode, nullptr );
        long    nThreshold = aSz.Height() / 2;
        long    nStep = nThreshold / 3;

        if ( !nStep )
            nStep = aSz.Height() - nThreshold;

        for(;; aSz.Height() -= nStep )
        {
            pFont->SetFontSize( aSz );
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
                    sal_Int32 nStart = 0;
                    sal_Int32 nLen = 0;

                    while( nStart < pText->getLength() && (*pText)[nStart] == ' ' )
                        nStart++;
                    while( nStart+nLen < pText->getLength() && (*pText)[nStart+nLen] != ' ' )
                        nLen++;
                    while( nStart < pText->getLength() && nLines-- )
                    {
                        sal_Int32 nNext = nLen;
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

                        sal_Int32 n = nLen;
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

} // end anonymous namespace

Graphic::Graphic()
    : mxImpGraphic(new ImpGraphic)
{
}

Graphic::Graphic(const Graphic& rGraphic)
{
    if( rGraphic.IsAnimated() )
        mxImpGraphic.reset(new ImpGraphic(*rGraphic.mxImpGraphic));
    else
        mxImpGraphic = rGraphic.mxImpGraphic;
}

Graphic::Graphic(Graphic&& rGraphic)
    : mxImpGraphic(std::move(rGraphic.mxImpGraphic))
{
}

Graphic::Graphic(const Bitmap& rBmp)
    : mxImpGraphic(new ImpGraphic(rBmp))
{
}

Graphic::Graphic(const BitmapEx& rBmpEx)
    : mxImpGraphic(new ImpGraphic(rBmpEx))
{
}

Graphic::Graphic(const VectorGraphicDataPtr& rVectorGraphicDataPtr)
    : mxImpGraphic(new ImpGraphic(rVectorGraphicDataPtr))
{
}

Graphic::Graphic(const Animation& rAnimation)
    : mxImpGraphic(new ImpGraphic(rAnimation))
{
}

Graphic::Graphic(const GDIMetaFile& rMtf)
    : mxImpGraphic(new ImpGraphic(rMtf))
{
}

Graphic::Graphic( const css::uno::Reference< css::graphic::XGraphic >& rxGraphic )
{
    uno::Reference< lang::XUnoTunnel >      xTunnel( rxGraphic, uno::UNO_QUERY );
    const ::Graphic*                        pGraphic = ( xTunnel.is() ?
                                                         reinterpret_cast< ::Graphic* >( xTunnel->getSomething( getUnoTunnelId() ) ) :
                                                          nullptr );

    if( pGraphic )
    {
        if (pGraphic->IsAnimated())
            mxImpGraphic.reset(new ImpGraphic(*pGraphic->mxImpGraphic));
        else
            mxImpGraphic = pGraphic->mxImpGraphic;
    }
    else
        mxImpGraphic.reset(new ImpGraphic);
}

void Graphic::ImplTestRefCount()
{
    if (mxImpGraphic.use_count() > 1)
    {
        mxImpGraphic.reset(new ImpGraphic(*mxImpGraphic));
    }
}

Graphic& Graphic::operator=( const Graphic& rGraphic )
{
    if( &rGraphic != this )
    {
        if( rGraphic.IsAnimated() )
        {
            mxImpGraphic.reset(new ImpGraphic(*rGraphic.mxImpGraphic));
        }
        else
        {
            mxImpGraphic = rGraphic.mxImpGraphic;
        }
    }

    return *this;
}

Graphic& Graphic::operator=(Graphic&& rGraphic)
{
    mxImpGraphic = std::move(rGraphic.mxImpGraphic);
    return *this;
}

bool Graphic::operator==( const Graphic& rGraphic ) const
{
    return (*mxImpGraphic == *rGraphic.mxImpGraphic);
}

bool Graphic::operator!=( const Graphic& rGraphic ) const
{
    return (*mxImpGraphic != *rGraphic.mxImpGraphic);
}

bool Graphic::operator!() const
{
    return (GraphicType::NONE == mxImpGraphic->ImplGetType());
}

void Graphic::Clear()
{
    ImplTestRefCount();
    mxImpGraphic->ImplClear();
}

GraphicType Graphic::GetType() const
{
    return mxImpGraphic->ImplGetType();
}

void Graphic::SetDefaultType()
{
    ImplTestRefCount();
    mxImpGraphic->ImplSetDefaultType();
}

bool Graphic::IsSupportedGraphic() const
{
    return mxImpGraphic->ImplIsSupportedGraphic();
}

bool Graphic::IsTransparent() const
{
    return mxImpGraphic->ImplIsTransparent();
}

bool Graphic::IsAlpha() const
{
    return mxImpGraphic->ImplIsAlpha();
}

bool Graphic::IsAnimated() const
{
    return mxImpGraphic->ImplIsAnimated();
}

bool Graphic::IsEPS() const
{
    return mxImpGraphic->ImplIsEPS();
}

Bitmap Graphic::GetBitmap(const GraphicConversionParameters& rParameters) const
{
    return mxImpGraphic->ImplGetBitmap(rParameters);
}

BitmapEx Graphic::GetBitmapEx(const GraphicConversionParameters& rParameters) const
{
    return mxImpGraphic->ImplGetBitmapEx(rParameters);
}

Animation Graphic::GetAnimation() const
{
    return mxImpGraphic->ImplGetAnimation();
}

const GDIMetaFile& Graphic::GetGDIMetaFile() const
{
    return mxImpGraphic->ImplGetGDIMetaFile();
}

const BitmapEx& Graphic::GetBitmapExRef() const
{
    return mxImpGraphic->ImplGetBitmapExRef();
}

uno::Reference< graphic::XGraphic > Graphic::GetXGraphic() const
{
    uno::Reference< graphic::XGraphic > xRet;

    if( GetType() != GraphicType::NONE )
    {
        uno::Reference < uno::XComponentContext > xContext( ::comphelper::getProcessComponentContext() );
        uno::Reference< graphic::XGraphicProvider > xProv( graphic::GraphicProvider::create( xContext ) );

        uno::Sequence< beans::PropertyValue > aLoadProps( 1 );
        OUString aURL = "private:memorygraphic/" + OUString::number( reinterpret_cast< sal_Int64 >( this ) );

        aLoadProps[ 0 ].Name = "URL";
        aLoadProps[ 0 ].Value <<= aURL;

        xRet = xProv->queryGraphic( aLoadProps );
    }

    return xRet;
}

Size Graphic::GetPrefSize() const
{
    return mxImpGraphic->ImplGetPrefSize();
}

void Graphic::SetPrefSize( const Size& rPrefSize )
{
    ImplTestRefCount();
    mxImpGraphic->ImplSetPrefSize( rPrefSize );
}

MapMode Graphic::GetPrefMapMode() const
{
    return mxImpGraphic->ImplGetPrefMapMode();
}

void Graphic::SetPrefMapMode( const MapMode& rPrefMapMode )
{
    ImplTestRefCount();
    mxImpGraphic->ImplSetPrefMapMode( rPrefMapMode );
}

basegfx::B2DSize Graphic::GetPPI() const
{
    double nGrfDPIx;
    double nGrfDPIy;

    const MapMode aGrfMap(GetPrefMapMode());
    const Size aGrfPixelSize(GetSizePixel());
    const Size aGrfPrefMapModeSize(GetPrefSize());
    if (aGrfMap.GetMapUnit() == MapUnit::MapInch)
    {
        nGrfDPIx = aGrfPixelSize.Width() / ( static_cast<double>(aGrfMap.GetScaleX()) * aGrfPrefMapModeSize.Width() );
        nGrfDPIy = aGrfPixelSize.Height() / ( static_cast<double>(aGrfMap.GetScaleY()) * aGrfPrefMapModeSize.Height() );
    }
    else
    {
        const Size aGrf1000thInchSize = OutputDevice::LogicToLogic(
                aGrfPrefMapModeSize, aGrfMap, MapMode(MapUnit::Map1000thInch));
        nGrfDPIx = 1000.0 * aGrfPixelSize.Width() / aGrf1000thInchSize.Width();
        nGrfDPIy = 1000.0 * aGrfPixelSize.Height() / aGrf1000thInchSize.Height();
    }

    return basegfx::B2DSize(nGrfDPIx, nGrfDPIy);
}

Size Graphic::GetSizePixel( const OutputDevice* pRefDevice ) const
{
    Size aRet;

    if( GraphicType::Bitmap == mxImpGraphic->ImplGetType() )
        aRet = mxImpGraphic->ImplGetBitmapEx(GraphicConversionParameters()).GetSizePixel();
    else
        aRet = ( pRefDevice ? pRefDevice : Application::GetDefaultDevice() )->LogicToPixel( GetPrefSize(), GetPrefMapMode() );

    return aRet;
}

sal_uLong Graphic::GetSizeBytes() const
{
    return mxImpGraphic->ImplGetSizeBytes();
}

void Graphic::Draw( OutputDevice* pOutDev, const Point& rDestPt ) const
{
    mxImpGraphic->ImplDraw( pOutDev, rDestPt );
}

void Graphic::Draw( OutputDevice* pOutDev,
                    const Point& rDestPt, const Size& rDestSz ) const
{
    if( GraphicType::Default == mxImpGraphic->ImplGetType() )
        ImplDrawDefault( pOutDev, nullptr, nullptr, nullptr, rDestPt, rDestSz );
    else
        mxImpGraphic->ImplDraw( pOutDev, rDestPt, rDestSz );
}

void Graphic::DrawEx( OutputDevice* pOutDev, const OUString& rText,
                    vcl::Font& rFont, const BitmapEx& rBitmap,
                    const Point& rDestPt, const Size& rDestSz )
{
    ImplDrawDefault( pOutDev, &rText, &rFont, &rBitmap, rDestPt, rDestSz );
}

void Graphic::StartAnimation( OutputDevice* pOutDev, const Point& rDestPt,
                              const Size& rDestSz, long nExtraData,
                              OutputDevice* pFirstFrameOutDev )
{
    ImplTestRefCount();
    mxImpGraphic->ImplStartAnimation( pOutDev, rDestPt, rDestSz, nExtraData, pFirstFrameOutDev );
}

void Graphic::StopAnimation( OutputDevice* pOutDev, long nExtraData )
{
    ImplTestRefCount();
    mxImpGraphic->ImplStopAnimation( pOutDev, nExtraData );
}

void Graphic::SetAnimationNotifyHdl( const Link<Animation*,void>& rLink )
{
    mxImpGraphic->ImplSetAnimationNotifyHdl( rLink );
}

Link<Animation*,void> Graphic::GetAnimationNotifyHdl() const
{
    return mxImpGraphic->ImplGetAnimationNotifyHdl();
}

sal_uInt32 Graphic::GetAnimationLoopCount() const
{
    return mxImpGraphic->ImplGetAnimationLoopCount();
}

std::shared_ptr<GraphicReader>& Graphic::GetContext()
{
    return mxImpGraphic->ImplGetContext();
}

void Graphic::SetContext( const std::shared_ptr<GraphicReader> &pReader )
{
    mxImpGraphic->ImplSetContext( pReader );
}

void Graphic::SetDummyContext( bool value )
{
    mxImpGraphic->ImplSetDummyContext( value );
}

bool Graphic::IsDummyContext()
{
    return mxImpGraphic->ImplIsDummyContext();
}

bool Graphic::SwapOut()
{
    ImplTestRefCount();
    return mxImpGraphic->ImplSwapOut();
}

void Graphic::SwapOutAsLink()
{
    ImplTestRefCount();
    mxImpGraphic->ImplSwapOutAsLink();
}

bool Graphic::SwapOut( SvStream* pOStream )
{
    ImplTestRefCount();
    return mxImpGraphic->ImplSwapOut( pOStream );
}

bool Graphic::SwapIn()
{
    ImplTestRefCount();
    return mxImpGraphic->ImplSwapIn();
}

bool Graphic::SwapIn( SvStream* pStrm )
{
    ImplTestRefCount();
    return mxImpGraphic->ImplSwapIn( pStrm );
}

bool Graphic::IsSwapOut() const
{
    return mxImpGraphic->ImplIsSwapOut();
}

void Graphic::SetLink( const GfxLink& rGfxLink )
{
    ImplTestRefCount();
    mxImpGraphic->ImplSetLink( rGfxLink );
}

GfxLink Graphic::GetLink() const
{
    return mxImpGraphic->ImplGetLink();
}

bool Graphic::IsLink() const
{
    return mxImpGraphic->ImplIsLink();
}

BitmapChecksum Graphic::GetChecksum() const
{
    return mxImpGraphic->ImplGetChecksum();
}

bool Graphic::ExportNative( SvStream& rOStream ) const
{
    return mxImpGraphic->ImplExportNative( rOStream );
}

void ReadGraphic(SvStream& rIStream, Graphic& rGraphic)
{
    rGraphic.ImplTestRefCount();
    ReadImpGraphic(rIStream, *rGraphic.mxImpGraphic);
}

void WriteGraphic( SvStream& rOStream, const Graphic& rGraphic )
{
    WriteImpGraphic(rOStream, *rGraphic.mxImpGraphic);
}

const VectorGraphicDataPtr& Graphic::getVectorGraphicData() const
{
    return mxImpGraphic->getVectorGraphicData();
}

void Graphic::setPdfData(const uno::Sequence<sal_Int8>& rPdfData)
{
    ImplTestRefCount();
    mxImpGraphic->maPdfData = rPdfData;
}

const uno::Sequence<sal_Int8>& Graphic::getPdfData() const
{
    return mxImpGraphic->maPdfData;
}

OUString Graphic::getOriginURL() const
{
    if (mxImpGraphic)
    {
        return mxImpGraphic->getOriginURL();
    }
    return OUString();
}

void Graphic::setOriginURL(OUString const & rOriginURL)
{
    if (mxImpGraphic)
    {
        mxImpGraphic->setOriginURL(rOriginURL);
    }
}

namespace {

struct Id: public rtl::Static<cppu::OImplementationId, Id> {};

}

css::uno::Sequence<sal_Int8> Graphic::getUnoTunnelId() {
    return Id::get().getImplementationId();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
