/*************************************************************************
 *
 *  $RCSfile: graph.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:05:37 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#define _SV_GRAPH_CXX

#ifndef _SV_CLIP_HXX
#include <clip.hxx>
#endif
#ifndef _SV_IMPGRAPH_HXX
#include <impgraph.hxx>
#endif
#ifndef _SV_OUTDEV_HXX
#include <outdev.hxx>
#endif
#ifndef _SV_CLIP_HXX
#include <clip.hxx>
#endif
#ifndef _SV_DRAG_HXX
#include <drag.hxx>
#endif
#include <graph.hxx>

// -----------------------
// - Compression defines -
// -----------------------

#define COMPRESS_OWN                ('S'|('D'<<8UL))
#define COMPRESS_NONE               ( 0UL )
#define RLE_8                       ( 1UL )
#define RLE_4                       ( 2UL )
#define BITFIELDS                   ( 3UL )
#define ZCOMPRESS                   ( COMPRESS_OWN | 0x01000000UL ) /* == 'SD01' (binary) */

// -----------------------
// - Default-Drawmethode -
// -----------------------

static void ImplDrawDefault( OutputDevice* pOutDev, const UniString* pText,
                             Font* pFont, const Bitmap* pBitmap,
                             const Point& rDestPt, const Size& rDestSize )
{
    USHORT      nPixel = (USHORT) pOutDev->PixelToLogic( Size( 1, 1 ) ).Width();
    USHORT      nWidth = nPixel;
    Point       aPoint( rDestPt.X() + nWidth, rDestPt.Y() + nWidth );
    Size        aSize( rDestSize.Width() - ( nWidth << 1 ), rDestSize.Height() - ( nWidth << 1 ) );
    BOOL        bFilled = ( pBitmap != NULL || pFont != NULL );
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

    aPoint.X() += nWidth + 2*nPixel;
    aPoint.Y() += nWidth + 2*nPixel;
    aSize.Width() -= 2*nWidth + 4*nPixel;
    aSize.Height() -= 2*nWidth + 4*nPixel;

    if( aSize.Width() > 0 && aSize.Height() > 0 && pBitmap && !!*pBitmap )
    {
        Size aBitmapSize( pOutDev->PixelToLogic(pBitmap->GetSizePixel() ) );

        if( aSize.Height() > aBitmapSize.Height() && aSize.Width() > aBitmapSize.Width() )
        {
            pOutDev->DrawBitmap( aPoint, *pBitmap );
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
                    USHORT nStart = 0;
                    USHORT nLen = 0;

                    while( nStart < pText->Len() && pText->GetChar( nStart ) == ' ' )
                        nStart++;
                    while( nStart+nLen < pText->Len() && pText->GetChar( nStart+nLen ) != ' ' )
                        nLen++;
                    while( nStart < pText->Len() && nLines-- )
                    {
                        USHORT nNext = nLen;
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

                        USHORT n = nLen;
                        nTextWidth = pOutDev->GetTextWidth( *pText, nStart, n );
                        while( nTextWidth > aSize.Width() )
                            nTextWidth = pOutDev->GetTextWidth( *pText, nStart, --n );
                        pOutDev->DrawText( aPoint, *pText, nStart, n );

                        aPoint.Y() += nTextHeight;
                        nStart     += nLen;
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

Graphic::Graphic( const Graphic& rGraphic )
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

BOOL Graphic::operator==( const Graphic& rGraphic ) const
{
    return( *mpImpGraphic == *rGraphic.mpImpGraphic );
}

// ------------------------------------------------------------------------

BOOL Graphic::operator!=( const Graphic& rGraphic ) const
{
    return( *mpImpGraphic != *rGraphic.mpImpGraphic );
}

// ------------------------------------------------------------------------

BOOL Graphic::operator!() const
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

BOOL Graphic::IsSupportedGraphic() const
{
    return mpImpGraphic->ImplIsSupportedGraphic();
}

// ------------------------------------------------------------------------

BOOL Graphic::IsTransparent() const
{
    return mpImpGraphic->ImplIsTransparent();
}

// ------------------------------------------------------------------------

BOOL Graphic::IsAlpha() const
{
    return mpImpGraphic->ImplIsAlpha();
}

// ------------------------------------------------------------------------

BOOL Graphic::IsAnimated() const
{
    return mpImpGraphic->ImplIsAnimated();
}

// ------------------------------------------------------------------------

Bitmap Graphic::GetBitmap() const
{
    return mpImpGraphic->ImplGetBitmap();
}

// ------------------------------------------------------------------------

BitmapEx Graphic::GetBitmapEx() const
{
    return mpImpGraphic->ImplGetBitmapEx();
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

ULONG Graphic::GetSizeBytes() const
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
        ImplDrawDefault( pOutDev, NULL, NULL, NULL, rDestPt, rDestSz );
    else
        mpImpGraphic->ImplDraw( pOutDev, rDestPt, rDestSz );
}

// ------------------------------------------------------------------------

void Graphic::Draw( OutputDevice* pOutDev, const String& rText,
                    Font& rFont, const Bitmap& rBitmap,
                    const Point& rDestPt, const Size& rDestSz )
{
    ImplDrawDefault( pOutDev, &rText, &rFont, &rBitmap, rDestPt, rDestSz );
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

ULONG Graphic::GetAnimationLoopCount() const
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

USHORT Graphic::GetGraphicsCompressMode( SvStream& rIStm )
{
    const ULONG     nPos = rIStm.Tell();
    const USHORT    nOldFormat = rIStm.GetNumberFormatInt();
    UINT32          nTmp32;
    UINT16          nTmp16;
    USHORT          nCompressMode = COMPRESSMODE_NONE;

    rIStm.SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );

    rIStm >> nTmp32;

    // is it a swapped graphic with a bitmap?
    rIStm.SeekRel( (nTmp32 == (UINT32) GRAPHIC_BITMAP ) ? 40 : -4 );

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

void Graphic::SetDocFileName( const String& rName, ULONG nFilePos )
{
    mpImpGraphic->ImplSetDocFileName( rName, nFilePos );
}

// ------------------------------------------------------------------------

const String& Graphic::GetDocFileName() const
{
    return mpImpGraphic->ImplGetDocFileName();
}

// ------------------------------------------------------------------------

ULONG Graphic::GetDocFilePos() const
{
    return mpImpGraphic->ImplGetDocFilePos();
}

// ------------------------------------------------------------------------

BOOL Graphic::ReadEmbedded( SvStream& rIStream, BOOL bSwap )
{
    ImplTestRefCount();
    return mpImpGraphic->ImplReadEmbedded( rIStream, bSwap );
}

// ------------------------------------------------------------------------

BOOL Graphic::WriteEmbedded( SvStream& rOStream )
{
    ImplTestRefCount();
    return mpImpGraphic->ImplWriteEmbedded( rOStream );
}

// ------------------------------------------------------------------------

BOOL Graphic::SwapOut()
{
    ImplTestRefCount();
    return mpImpGraphic->ImplSwapOut();
}

// ------------------------------------------------------------------------

BOOL Graphic::SwapOut( SvStream* pOStream )
{
    ImplTestRefCount();
    return mpImpGraphic->ImplSwapOut( pOStream );
}

// ------------------------------------------------------------------------

BOOL Graphic::SwapIn()
{
    ImplTestRefCount();
    return mpImpGraphic->ImplSwapIn();
}

// ------------------------------------------------------------------------

BOOL Graphic::SwapIn( SvStream* pStrm )
{
    ImplTestRefCount();
    return mpImpGraphic->ImplSwapIn( pStrm );
}

// ------------------------------------------------------------------------

BOOL Graphic::IsSwapOut() const
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

GfxLink Graphic::GetLink()
{
    return mpImpGraphic->ImplGetLink();
}

// ------------------------------------------------------------------------

BOOL Graphic::IsLink() const
{
    return mpImpGraphic->ImplIsLink();
}

// ------------------------------------------------------------------------

ULONG Graphic::GetChecksum() const
{
    return mpImpGraphic->ImplGetChecksum();
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

// ------------------------------------------------------------------------

ULONG Graphic::RegisterClipboardFormatName()
{
     static ULONG nFormat = 0;

     if ( !nFormat )
         nFormat = Clipboard::RegisterFormatName( XubString( RTL_CONSTASCII_USTRINGPARAM( "SVXB (StarView Bitmap/Animation)" ) ) );

     return nFormat;
}

// ------------------------------------------------------------------------

BOOL Graphic::ClipboardHasFormat()
{
    return Clipboard::HasFormat( RegisterClipboardFormatName() )
           || Clipboard::HasFormat( FORMAT_GDIMETAFILE )
           || Clipboard::HasFormat( FORMAT_BITMAP );
}

// ------------------------------------------------------------------------

BOOL Graphic::DragServerHasFormat( USHORT nItem )
{
    return DragServer::HasFormat( nItem, RegisterClipboardFormatName() )
           || DragServer::HasFormat( nItem, FORMAT_GDIMETAFILE )
           || DragServer::HasFormat( nItem, FORMAT_BITMAP );
}

// ------------------------------------------------------------------------

BOOL Graphic::Copy() const
{
    SotDataMemberObjectRef  aDataObject = new SotDataMemberObject;
    SvData*                 pData = new SvData( RegisterClipboardFormatName() );

    pData->SetData( (SvDataCopyStream*) this, TRANSFER_COPY );
    aDataObject->Append( pData );
    VclClipboard::Copy( aDataObject );

    return TRUE;
}

// ------------------------------------------------------------------------

BOOL Graphic::Paste()
{
    const ULONG nFormat = RegisterClipboardFormatName();
    BOOL        bRet = FALSE;

    if( VclClipboard::HasFormat( nFormat ) )
    {
        SotDataObjectRef aDataObject = VclClipboard::Paste();
        SvData           aData( nFormat );

        if( aDataObject.Is() && aDataObject->GetData( &aData ) )
        {
            Graphic* pGraphic = NULL;

            if( aData.GetData( (SvDataCopyStream**) &pGraphic, StaticType(), TRANSFER_MOVE ) )
                *this = *pGraphic;

            delete pGraphic;
            bRet = TRUE;
        }
    }

    return bRet;
}
