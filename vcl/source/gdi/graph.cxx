/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"
#include <vcl/impgraph.hxx>
#include <vcl/outdev.hxx>
#include <vcl/svapp.hxx>
#include <vcl/graph.hxx>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/graphic/XGraphicProvider.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>

// -----------------------
// - Compression defines -
// -----------------------

#define COMPRESS_OWN                ('S'|('D'<<8UL))
#define COMPRESS_NONE               ( 0UL )
#define RLE_8                       ( 1UL )
#define RLE_4                       ( 2UL )
#define BITFIELDS                   ( 3UL )
#define ZCOMPRESS                   ( COMPRESS_OWN | 0x01000000UL ) /* == 'SD01' (binary) */

using namespace ::com::sun::star;

// -----------------------
// - Default-Drawmethode -
// -----------------------

static void ImplDrawDefault( OutputDevice* pOutDev, const UniString* pText,
                             Font* pFont, const Bitmap* pBitmap, const BitmapEx* pBitmapEx,
                             const Point& rDestPt, const Size& rDestSize )
{
    sal_uInt16      nPixel = (sal_uInt16) pOutDev->PixelToLogic( Size( 1, 1 ) ).Width();
    sal_uInt16      nPixelWidth = nPixel;
    Point       aPoint( rDestPt.X() + nPixelWidth, rDestPt.Y() + nPixelWidth );
    Size        aSize( rDestSize.Width() - ( nPixelWidth << 1 ), rDestSize.Height() - ( nPixelWidth << 1 ) );
    sal_Bool        bFilled = ( pBitmap != NULL || pBitmapEx != NULL || pFont != NULL );
    Rectangle   aBorderRect( aPoint, aSize );

    pOutDev->Push();

    pOutDev->SetFillColor();

    // Auf dem Drucker ein schwarzes Rechteck und auf dem Bildschirm eins mit 3D-Effekt
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

    if ( aSize.Width() > 0 && aSize.Height() > 0 && pFont && pText && pText->Len()
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
                // Die N"aherung ber"ucksichtigt keine Ungenauigkeiten durch
                // Wortumbr"uche
                long nLines = aSize.Height() / nTextHeight;
                long nWidth = aSize.Width() * nLines; // N"aherung!!!

                if ( nTextWidth <= nWidth || aSz.Height() <= nThreshold )
                {
                    sal_uInt16 nStart = 0;
                    sal_uInt16 nLen = 0;

                    while( nStart < pText->Len() && pText->GetChar( nStart ) == ' ' )
                        nStart++;
                    while( nStart+nLen < pText->Len() && pText->GetChar( nStart+nLen ) != ' ' )
                        nLen++;
                    while( nStart < pText->Len() && nLines-- )
                    {
                        sal_uInt16 nNext = nLen;
                        do
                        {
                            while ( nStart+nNext < pText->Len() && pText->GetChar( nStart+nNext ) == ' ' )
                                nNext++;
                            while ( nStart+nNext < pText->Len() && pText->GetChar( nStart+nNext ) != ' ' )
                                nNext++;
                            nTextWidth = pOutDev->GetTextWidth( *pText, nStart, nNext );
                            if ( nTextWidth > aSize.Width() )
                                break;
                            nLen = nNext;
                        }
                        while ( nStart+nNext < pText->Len() );

                        sal_uInt16 n = nLen;
                        nTextWidth = pOutDev->GetTextWidth( *pText, nStart, n );
                        while( nTextWidth > aSize.Width() )
                            nTextWidth = pOutDev->GetTextWidth( *pText, nStart, --n );
                        pOutDev->DrawText( aPoint, *pText, nStart, n );

                        aPoint.Y() += nTextHeight;
                        nStart      = sal::static_int_cast<sal_uInt16>(nStart + nLen);
                        nLen        = nNext-nLen;
                        while( nStart < pText->Len() && pText->GetChar( nStart ) == ' ' )
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

    // Falls die Default-Graphik keinen Inhalt hat,
    // malen wir ein rotes Kreuz
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

// -----------
// - Graphic -
// -----------

TYPEINIT1_AUTOFACTORY( Graphic, SvDataCopyStream );

// ------------------------------------------------------------------------

Graphic::Graphic()
{
    mpImpGraphic = new ImpGraphic;
}

// ------------------------------------------------------------------------

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

// ------------------------------------------------------------------------

Graphic::Graphic( const Bitmap& rBmp )
{
    mpImpGraphic = new ImpGraphic( rBmp );
}

// ------------------------------------------------------------------------

Graphic::Graphic( const BitmapEx& rBmpEx )
{
    mpImpGraphic = new ImpGraphic( rBmpEx );
}

// ------------------------------------------------------------------------

Graphic::Graphic( const Animation& rAnimation )
{
    mpImpGraphic = new ImpGraphic( rAnimation );
}

// ------------------------------------------------------------------------

Graphic::Graphic( const GDIMetaFile& rMtf )
{
    mpImpGraphic = new ImpGraphic( rMtf );
}

// ------------------------------------------------------------------------

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

// ------------------------------------------------------------------------

Graphic::~Graphic()
{
    if( mpImpGraphic->mnRefCount == 1UL )
        delete mpImpGraphic;
    else
        mpImpGraphic->mnRefCount--;
}

// ------------------------------------------------------------------------

void Graphic::ImplTestRefCount()
{
    if( mpImpGraphic->mnRefCount > 1UL )
    {
        mpImpGraphic->mnRefCount--;
        mpImpGraphic = new ImpGraphic( *mpImpGraphic );
    }
}

// ------------------------------------------------------------------------

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

// ------------------------------------------------------------------------

sal_Bool Graphic::operator==( const Graphic& rGraphic ) const
{
    return( *mpImpGraphic == *rGraphic.mpImpGraphic );
}

// ------------------------------------------------------------------------

sal_Bool Graphic::operator!=( const Graphic& rGraphic ) const
{
    return( *mpImpGraphic != *rGraphic.mpImpGraphic );
}

// ------------------------------------------------------------------------

sal_Bool Graphic::operator!() const
{
    return( GRAPHIC_NONE == mpImpGraphic->ImplGetType() );
}

// ------------------------------------------------------------------------

void Graphic::Load( SvStream& rIStm )
{
    rIStm >> *this;
}

// ------------------------------------------------------------------------

void Graphic::Save( SvStream& rOStm )
{
    rOStm << *this;
}

// ------------------------------------------------------------------------

void Graphic::Assign( const SvDataCopyStream& rCopyStream )
{
    *this = (const Graphic& ) rCopyStream;
}

// ------------------------------------------------------------------------

void Graphic::Clear()
{
    ImplTestRefCount();
    mpImpGraphic->ImplClear();
}

// ------------------------------------------------------------------------

GraphicType Graphic::GetType() const
{
    return mpImpGraphic->ImplGetType();
}

// ------------------------------------------------------------------------

void Graphic::SetDefaultType()
{
    ImplTestRefCount();
    mpImpGraphic->ImplSetDefaultType();
}

// ------------------------------------------------------------------------

sal_Bool Graphic::IsSupportedGraphic() const
{
    return mpImpGraphic->ImplIsSupportedGraphic();
}

// ------------------------------------------------------------------------

sal_Bool Graphic::IsTransparent() const
{
    return mpImpGraphic->ImplIsTransparent();
}

// ------------------------------------------------------------------------

sal_Bool Graphic::IsAlpha() const
{
    return mpImpGraphic->ImplIsAlpha();
}

// ------------------------------------------------------------------------

sal_Bool Graphic::IsAnimated() const
{
    return mpImpGraphic->ImplIsAnimated();
}

// ------------------------------------------------------------------------

Bitmap Graphic::GetBitmap(const GraphicConversionParameters& rParameters) const
{
    return mpImpGraphic->ImplGetBitmap(rParameters);
}

// ------------------------------------------------------------------------

BitmapEx Graphic::GetBitmapEx(const GraphicConversionParameters& rParameters) const
{
    return mpImpGraphic->ImplGetBitmapEx(rParameters);
}

// ------------------------------------------------------------------------

Animation Graphic::GetAnimation() const
{
    return mpImpGraphic->ImplGetAnimation();
}

// ------------------------------------------------------------------------

const GDIMetaFile& Graphic::GetGDIMetaFile() const
{
    return mpImpGraphic->ImplGetGDIMetaFile();
}

// ------------------------------------------------------------------------

uno::Reference< graphic::XGraphic > Graphic::GetXGraphic() const
{
    uno::Reference< graphic::XGraphic > xRet;

    if( GetType() != GRAPHIC_NONE )
    {
        uno::Reference < lang::XMultiServiceFactory > xMSF( ::comphelper::getProcessServiceFactory() );

        if( xMSF.is() )
        {
            uno::Reference< graphic::XGraphicProvider > xProv( xMSF->createInstance(
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.graphic.GraphicProvider" ) ) ),
                uno::UNO_QUERY );

            if( xProv.is() )
            {
                uno::Sequence< beans::PropertyValue >   aLoadProps( 1 );
                ::rtl::OUString                         aURL( RTL_CONSTASCII_USTRINGPARAM( "private:memorygraphic/" ) );

                aLoadProps[ 0 ].Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "URL" ) );
                aLoadProps[ 0 ].Value <<= ( aURL += ::rtl::OUString::valueOf( reinterpret_cast< sal_Int64 >( this ) ) );

                xRet = xProv->queryGraphic( aLoadProps );
            }
        }
    }

    return xRet;
}

// ------------------------------------------------------------------------

Size Graphic::GetPrefSize() const
{
    return mpImpGraphic->ImplGetPrefSize();
}

// ------------------------------------------------------------------------

void Graphic::SetPrefSize( const Size& rPrefSize )
{
    ImplTestRefCount();
    mpImpGraphic->ImplSetPrefSize( rPrefSize );
}

// ------------------------------------------------------------------------

MapMode Graphic::GetPrefMapMode() const
{
    return mpImpGraphic->ImplGetPrefMapMode();
}

// ------------------------------------------------------------------------

void Graphic::SetPrefMapMode( const MapMode& rPrefMapMode )
{
    ImplTestRefCount();
    mpImpGraphic->ImplSetPrefMapMode( rPrefMapMode );
}

// ------------------------------------------------------------------

Size Graphic::GetSizePixel( const OutputDevice* pRefDevice ) const
{
    Size aRet;

    if( GRAPHIC_BITMAP == mpImpGraphic->ImplGetType() )
        aRet = mpImpGraphic->ImplGetBitmapEx(GraphicConversionParameters()).GetSizePixel();
    else
        aRet = ( pRefDevice ? pRefDevice : Application::GetDefaultDevice() )->LogicToPixel( GetPrefSize(), GetPrefMapMode() );

    return aRet;
}

// ------------------------------------------------------------------

sal_uLong Graphic::GetSizeBytes() const
{
    return mpImpGraphic->ImplGetSizeBytes();
}

// ------------------------------------------------------------------------

void Graphic::Draw( OutputDevice* pOutDev, const Point& rDestPt ) const
{
    mpImpGraphic->ImplDraw( pOutDev, rDestPt );
}

// ------------------------------------------------------------------------

void Graphic::Draw( OutputDevice* pOutDev,
                    const Point& rDestPt, const Size& rDestSz ) const
{
    if( GRAPHIC_DEFAULT == mpImpGraphic->ImplGetType() )
        ImplDrawDefault( pOutDev, NULL, NULL, NULL, NULL, rDestPt, rDestSz );
    else
        mpImpGraphic->ImplDraw( pOutDev, rDestPt, rDestSz );
}

// ------------------------------------------------------------------------

void Graphic::Draw( OutputDevice* pOutDev, const String& rText,
                    Font& rFont, const Bitmap& rBitmap,
                    const Point& rDestPt, const Size& rDestSz )
{
    ImplDrawDefault( pOutDev, &rText, &rFont, &rBitmap, NULL, rDestPt, rDestSz );
}

// ------------------------------------------------------------------------

void Graphic::DrawEx( OutputDevice* pOutDev, const String& rText,
                    Font& rFont, const BitmapEx& rBitmap,
                    const Point& rDestPt, const Size& rDestSz )
{
    ImplDrawDefault( pOutDev, &rText, &rFont, NULL, &rBitmap, rDestPt, rDestSz );
}

// ------------------------------------------------------------------------

void Graphic::StartAnimation( OutputDevice* pOutDev, const Point& rDestPt, long nExtraData,
                              OutputDevice* pFirstFrameOutDev )
{
    ImplTestRefCount();
    mpImpGraphic->ImplStartAnimation( pOutDev, rDestPt, nExtraData, pFirstFrameOutDev );
}

// ------------------------------------------------------------------------

void Graphic::StartAnimation( OutputDevice* pOutDev, const Point& rDestPt,
                              const Size& rDestSz, long nExtraData,
                              OutputDevice* pFirstFrameOutDev )
{
    ImplTestRefCount();
    mpImpGraphic->ImplStartAnimation( pOutDev, rDestPt, rDestSz, nExtraData, pFirstFrameOutDev );
}

// ------------------------------------------------------------------------

void Graphic::StopAnimation( OutputDevice* pOutDev, long nExtraData )
{
    ImplTestRefCount();
    mpImpGraphic->ImplStopAnimation( pOutDev, nExtraData );
}

// ------------------------------------------------------------------------

void Graphic::SetAnimationNotifyHdl( const Link& rLink )
{
    mpImpGraphic->ImplSetAnimationNotifyHdl( rLink );
}

// ------------------------------------------------------------------------

Link Graphic::GetAnimationNotifyHdl() const
{
    return mpImpGraphic->ImplGetAnimationNotifyHdl();
}

// ------------------------------------------------------------------------

sal_uLong Graphic::GetAnimationLoopCount() const
{
    return mpImpGraphic->ImplGetAnimationLoopCount();
}

// ------------------------------------------------------------------------

void Graphic::ResetAnimationLoopCount()
{
    mpImpGraphic->ImplResetAnimationLoopCount();
}

// ------------------------------------------------------------------------

List* Graphic::GetAnimationInfoList() const
{
    return mpImpGraphic->ImplGetAnimationInfoList();
}

// ------------------------------------------------------------------------

GraphicReader* Graphic::GetContext()
{
    return mpImpGraphic->ImplGetContext();
}

// ------------------------------------------------------------------------

void Graphic::SetContext( GraphicReader* pReader )
{
    mpImpGraphic->ImplSetContext( pReader );
}

// ------------------------------------------------------------------------

sal_uInt16 Graphic::GetGraphicsCompressMode( SvStream& rIStm )
{
    const sal_uLong     nPos = rIStm.Tell();
    const sal_uInt16    nOldFormat = rIStm.GetNumberFormatInt();
    sal_uInt32          nTmp32;
    sal_uInt16          nTmp16;
    sal_uInt16          nCompressMode = COMPRESSMODE_NONE;

    rIStm.SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );

    rIStm >> nTmp32;

    // is it a swapped graphic with a bitmap?
    rIStm.SeekRel( (nTmp32 == (sal_uInt32) GRAPHIC_BITMAP ) ? 40 : -4 );

    // try to read bitmap id
    rIStm >> nTmp16;

    // check id of BitmapFileHeader
    if( 0x4D42 == nTmp16 )
    {
        // seek to compress field of BitmapInfoHeader
        rIStm.SeekRel( 28 );
        rIStm >> nTmp32;

        // Compare with our own compressmode
        if( ZCOMPRESS == nTmp32 )
            nCompressMode = COMPRESSMODE_ZBITMAP;
    }

    rIStm.SetNumberFormatInt( nOldFormat );
    rIStm.Seek( nPos );

    return nCompressMode;
}

// ------------------------------------------------------------------------

void Graphic::SetDocFileName( const String& rName, sal_uLong nFilePos )
{
    mpImpGraphic->ImplSetDocFileName( rName, nFilePos );
}

// ------------------------------------------------------------------------

const String& Graphic::GetDocFileName() const
{
    return mpImpGraphic->ImplGetDocFileName();
}

// ------------------------------------------------------------------------

sal_uLong Graphic::GetDocFilePos() const
{
    return mpImpGraphic->ImplGetDocFilePos();
}

// ------------------------------------------------------------------------

sal_Bool Graphic::ReadEmbedded( SvStream& rIStream, sal_Bool bSwap )
{
    ImplTestRefCount();
    return mpImpGraphic->ImplReadEmbedded( rIStream, bSwap );
}

// ------------------------------------------------------------------------

sal_Bool Graphic::WriteEmbedded( SvStream& rOStream )
{
    ImplTestRefCount();
    return mpImpGraphic->ImplWriteEmbedded( rOStream );
}

// ------------------------------------------------------------------------

sal_Bool Graphic::SwapOut()
{
    ImplTestRefCount();
    return mpImpGraphic->ImplSwapOut();
}

// ------------------------------------------------------------------------

sal_Bool Graphic::SwapOut( SvStream* pOStream )
{
    ImplTestRefCount();
    return mpImpGraphic->ImplSwapOut( pOStream );
}

// ------------------------------------------------------------------------

sal_Bool Graphic::SwapIn()
{
    ImplTestRefCount();
    return mpImpGraphic->ImplSwapIn();
}

// ------------------------------------------------------------------------

sal_Bool Graphic::SwapIn( SvStream* pStrm )
{
    ImplTestRefCount();
    return mpImpGraphic->ImplSwapIn( pStrm );
}

// ------------------------------------------------------------------------

sal_Bool Graphic::IsSwapOut() const
{
    return mpImpGraphic->ImplIsSwapOut();
}

// ------------------------------------------------------------------------

void Graphic::SetLink( const GfxLink& rGfxLink )
{
    ImplTestRefCount();
    mpImpGraphic->ImplSetLink( rGfxLink );
}

// ------------------------------------------------------------------------

GfxLink Graphic::GetLink() const
{
    return mpImpGraphic->ImplGetLink();
}

// ------------------------------------------------------------------------

sal_Bool Graphic::IsLink() const
{
    return mpImpGraphic->ImplIsLink();
}

// ------------------------------------------------------------------------

sal_uLong Graphic::GetChecksum() const
{
    return mpImpGraphic->ImplGetChecksum();
}

// ------------------------------------------------------------------------

sal_Bool Graphic::ExportNative( SvStream& rOStream ) const
{
    return mpImpGraphic->ImplExportNative( rOStream );
}

// ------------------------------------------------------------------------

SvStream& operator>>( SvStream& rIStream, Graphic& rGraphic )
{
    rGraphic.ImplTestRefCount();
    return rIStream >> *rGraphic.mpImpGraphic;
}

// ------------------------------------------------------------------------

SvStream& operator<<( SvStream& rOStream, const Graphic& rGraphic )
{
    return rOStream << *rGraphic.mpImpGraphic;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
