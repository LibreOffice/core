/*************************************************************************
 *
 *  $RCSfile: bitmapex.cxx,v $
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

#define _SV_BITMAPEX_CXX

#ifndef _RTL_CRC_H_
#include <rtl/crc.h>
#endif
#ifndef _SV_SALBTYPE_HXX
#include <salbtype.hxx>
#endif
#ifndef _STREAM_HXX
#include <tools/stream.hxx>
#endif
#ifndef _DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _SV_OUTDEV_HXX
#include <outdev.hxx>
#endif
#ifndef _SV_ALPHA_HXX
#include <alpha.hxx>
#endif
#ifndef _SV_BITMAPEX_HXX
#include <bitmapex.hxx>
#endif

// ------------
// - BitmapEx -
// ------------

BitmapEx::BitmapEx() :
        eTransparent( TRANSPARENT_NONE ),
        bAlpha      ( FALSE )
{
}

// ------------------------------------------------------------------

BitmapEx::BitmapEx( const BitmapEx& rBitmapEx ) :
        aBitmap             ( rBitmapEx.aBitmap ),
        aMask               ( rBitmapEx.aMask ),
        aBitmapSize         ( rBitmapEx.aBitmapSize ),
        aTransparentColor   ( rBitmapEx.aTransparentColor ),
        eTransparent        ( rBitmapEx.eTransparent ),
        bAlpha              ( rBitmapEx.bAlpha )
{
}

// ------------------------------------------------------------------

BitmapEx::BitmapEx( const Bitmap& rBmp ) :
        aBitmap     ( rBmp ),
        aBitmapSize ( aBitmap.GetSizePixel() ),
        eTransparent( TRANSPARENT_NONE ),
        bAlpha      ( FALSE )
{
}

// ------------------------------------------------------------------

BitmapEx::BitmapEx( const Bitmap& rBmp, const Bitmap& rMask ) :
        aBitmap         ( rBmp ),
        aMask           ( rMask ),
        aBitmapSize     ( aBitmap.GetSizePixel() ),
        eTransparent    ( !rMask ? TRANSPARENT_NONE : TRANSPARENT_BITMAP ),
        bAlpha          ( FALSE )
{
}

// ------------------------------------------------------------------

BitmapEx::BitmapEx( const Bitmap& rBmp, const AlphaMask& rAlphaMask ) :
        aBitmap         ( rBmp ),
        aMask           ( rAlphaMask.ImplGetBitmap() ),
        aBitmapSize     ( aBitmap.GetSizePixel() ),
        eTransparent    ( !rAlphaMask ? TRANSPARENT_NONE : TRANSPARENT_BITMAP ),
        bAlpha          ( !rAlphaMask ? FALSE : TRUE )
{
}

// ------------------------------------------------------------------

BitmapEx::BitmapEx( const Bitmap& rBmp, const Color& rTransparentColor ) :
        aBitmap             ( rBmp ),
        aBitmapSize         ( aBitmap.GetSizePixel() ),
        aTransparentColor   ( rTransparentColor ),
        eTransparent        ( TRANSPARENT_BITMAP ),
        bAlpha              ( FALSE )
{
    aMask = aBitmap.CreateMask( aTransparentColor );
}

// ------------------------------------------------------------------

BitmapEx::~BitmapEx()
{
}

// ------------------------------------------------------------------

BitmapEx& BitmapEx::operator=( const BitmapEx& rBitmapEx )
{
    if( &rBitmapEx != this )
    {
        aBitmap = rBitmapEx.aBitmap;
        aMask = rBitmapEx.aMask;
        aBitmapSize = rBitmapEx.aBitmapSize;
        aTransparentColor = rBitmapEx.aTransparentColor;
        eTransparent = rBitmapEx.eTransparent;
        bAlpha = rBitmapEx.bAlpha;
    }

    return *this;
}

// ------------------------------------------------------------------

BOOL BitmapEx::operator==( const BitmapEx& rBitmapEx ) const
{
    if( eTransparent != rBitmapEx.eTransparent )
        return FALSE;

    if( aBitmap != rBitmapEx.aBitmap )
        return FALSE;

    if( aBitmapSize != rBitmapEx.aBitmapSize )
        return FALSE;

    if( eTransparent == TRANSPARENT_NONE )
        return TRUE;

    if( eTransparent == TRANSPARENT_COLOR )
        return aTransparentColor == rBitmapEx.aTransparentColor;

    return( ( aMask == rBitmapEx.aMask ) && ( bAlpha == rBitmapEx.bAlpha ) );
}

// ------------------------------------------------------------------

BOOL BitmapEx::IsEqual( const BitmapEx& rBmpEx ) const
{
    return( rBmpEx.eTransparent == eTransparent &&
            rBmpEx.bAlpha == bAlpha &&
            rBmpEx.aBitmap.IsEqual( aBitmap ) &&
            rBmpEx.aMask.IsEqual( aMask ) );
}

// ------------------------------------------------------------------

BOOL BitmapEx::IsEmpty() const
{
    return( aBitmap.IsEmpty() && aMask.IsEmpty() );
}

// ------------------------------------------------------------------

void BitmapEx::SetEmpty()
{
    aBitmap.SetEmpty();
    aMask.SetEmpty();
    eTransparent = TRANSPARENT_NONE;
    bAlpha = FALSE;
}

// ------------------------------------------------------------------

void BitmapEx::Clear()
{
    SetEmpty();
}

// ------------------------------------------------------------------

BOOL BitmapEx::IsTransparent() const
{
    return( eTransparent != TRANSPARENT_NONE );
}

// ------------------------------------------------------------------

BOOL BitmapEx::IsAlpha() const
{
    return( IsTransparent() && bAlpha );
}

// ------------------------------------------------------------------

Bitmap BitmapEx::GetBitmap( const Color* pTransReplaceColor ) const
{
    Bitmap aRetBmp( aBitmap );

    if( pTransReplaceColor && ( eTransparent != TRANSPARENT_NONE ) )
    {
        Bitmap aTempMask;

        if( eTransparent == TRANSPARENT_COLOR )
            aTempMask = aBitmap.CreateMask( aTransparentColor );
        else
            aTempMask = aMask;

        if( !IsAlpha() )
            aRetBmp.Replace( aTempMask, *pTransReplaceColor );
        else
            aRetBmp.Replace( GetAlpha(), *pTransReplaceColor );
    }

    return aRetBmp;
}

// ------------------------------------------------------------------

Bitmap BitmapEx::GetMask() const
{
    Bitmap aRet( aMask );

    if( IsAlpha() )
        aRet.ImplMakeMono( 255 );

    return aRet;
}

// ------------------------------------------------------------------

AlphaMask BitmapEx::GetAlpha() const
{
    AlphaMask aAlpha;

    if( IsAlpha() )
        aAlpha.ImplSetBitmap( aMask );
    else
        aAlpha = aMask;

    return aAlpha;
}

// ------------------------------------------------------------------

ULONG BitmapEx::GetSizeBytes() const
{
    ULONG nSizeBytes = aBitmap.GetSizeBytes();

    if( eTransparent == TRANSPARENT_BITMAP )
        nSizeBytes += aMask.GetSizeBytes();

    return nSizeBytes;
}

// ------------------------------------------------------------------

ULONG BitmapEx::GetChecksum() const
{
    sal_uInt32  nCrc = aBitmap.GetChecksum();
    SVBT32      aBT32;

    LongToSVBT32( (long) eTransparent, aBT32 );
    nCrc = rtl_crc32( nCrc, aBT32, 4 );

    LongToSVBT32( (long) bAlpha, aBT32 );
    nCrc = rtl_crc32( nCrc, aBT32, 4 );

    if( ( TRANSPARENT_BITMAP == eTransparent ) && !aMask.IsEmpty() )
    {
        LongToSVBT32( aMask.GetChecksum(), aBT32 );
        nCrc = rtl_crc32( nCrc, aBT32, 4 );
    }

    return nCrc;
}

// ------------------------------------------------------------------

void BitmapEx::SetSizePixel( const Size& rNewSize )
{
    Scale( rNewSize );
}

// ------------------------------------------------------------------

BOOL BitmapEx::Invert()
{
    BOOL bRet = FALSE;

    if( !!aBitmap )
    {
        bRet = aBitmap.Invert();

        if( bRet && ( eTransparent == TRANSPARENT_COLOR ) )
            aTransparentColor = BitmapColor( aTransparentColor ).Invert();
    }

    return bRet;
}

// ------------------------------------------------------------------

BOOL BitmapEx::Mirror( ULONG nMirrorFlags )
{
    BOOL bRet = FALSE;

    if( !!aBitmap )
    {
        bRet = aBitmap.Mirror( nMirrorFlags );

        if( bRet && ( eTransparent == TRANSPARENT_BITMAP ) && !!aMask )
            aMask.Mirror( nMirrorFlags );
    }

    return bRet;
}

// ------------------------------------------------------------------

BOOL BitmapEx::Scale( const double& rScaleX, const double& rScaleY, ULONG nScaleFlag )
{
    BOOL bRet = FALSE;

    if( !!aBitmap )
    {
        bRet = aBitmap.Scale( rScaleX, rScaleY, nScaleFlag );

        if( bRet && ( eTransparent == TRANSPARENT_BITMAP ) && !!aMask )
            aMask.Scale( rScaleX, rScaleY, BMP_SCALE_FAST );

        aBitmapSize = aBitmap.GetSizePixel();
    }

    return bRet;
}

// ------------------------------------------------------------------------

BOOL BitmapEx::Scale( const Size& rNewSize, ULONG nScaleFlag )
{
    BOOL bRet;

    if( aBitmapSize.Width() && aBitmapSize.Height() )
    {
        bRet = Scale( (double) rNewSize.Width() / aBitmapSize.Width(),
                      (double) rNewSize.Height() / aBitmapSize.Height(),
                      nScaleFlag );
    }
    else
        bRet = TRUE;

    return bRet;
}

// ------------------------------------------------------------------

BOOL BitmapEx::Rotate( long nAngle10, const Color& rFillColor )
{
    BOOL bRet = FALSE;

    if( !!aBitmap )
    {
        const BOOL bTransRotate = ( Color( COL_TRANSPARENT ) == rFillColor );

        if( bTransRotate )
        {
            if( eTransparent == TRANSPARENT_COLOR )
                bRet = aBitmap.Rotate( nAngle10, aTransparentColor );
            else
            {
                bRet = aBitmap.Rotate( nAngle10, COL_BLACK );

                if( eTransparent == TRANSPARENT_NONE )
                {
                    aMask = Bitmap( aBitmapSize, 1 );
                    aMask.Erase( COL_BLACK );
                    eTransparent = TRANSPARENT_BITMAP;
                }

                if( bRet && !!aMask )
                    aMask.Rotate( nAngle10, COL_WHITE );
            }
        }
        else
        {
            bRet = aBitmap.Rotate( nAngle10, rFillColor );

            if( bRet && ( eTransparent == TRANSPARENT_BITMAP ) && !!aMask )
                aMask.Rotate( nAngle10, COL_WHITE );
        }

        aBitmapSize = aBitmap.GetSizePixel();
    }

    return bRet;
}

// ------------------------------------------------------------------

BOOL BitmapEx::Crop( const Rectangle& rRectPixel )
{
    BOOL bRet = FALSE;

    if( !!aBitmap )
    {
        bRet = aBitmap.Crop( rRectPixel );

        if( bRet && ( eTransparent == TRANSPARENT_BITMAP ) && !!aMask )
            aMask.Crop( rRectPixel );

        aBitmapSize = aBitmap.GetSizePixel();
    }

    return bRet;
}

// ------------------------------------------------------------------

BOOL BitmapEx::Convert( BmpConversion eConversion )
{
    return( !!aBitmap ? aBitmap.Convert( eConversion ) : FALSE );
}

// ------------------------------------------------------------------

BOOL BitmapEx::ReduceColors( USHORT nNewColorCount, BmpReduce eReduce )
{
    return( !!aBitmap ? aBitmap.ReduceColors( nNewColorCount, eReduce ) : FALSE );
}

// ------------------------------------------------------------------

BOOL BitmapEx::Expand( ULONG nDX, ULONG nDY, const Color* pInitColor, BOOL bExpandTransparent )
{
    BOOL bRet = FALSE;

    if( !!aBitmap )
    {
        bRet = aBitmap.Expand( nDX, nDY, pInitColor );

        if( bRet && ( eTransparent == TRANSPARENT_BITMAP ) && !!aMask )
        {
            Color aColor( bExpandTransparent ? COL_WHITE : COL_BLACK );
            aMask.Expand( nDX, nDY, &aColor );
        }

        aBitmapSize = aBitmap.GetSizePixel();
    }

    return bRet;
}

// ------------------------------------------------------------------

BOOL BitmapEx::Dither( ULONG nDitherFlags, const BitmapPalette* pDitherPal )
{
    return( !!aBitmap ? aBitmap.Dither( nDitherFlags, pDitherPal ) : FALSE );
}

// ------------------------------------------------------------------

BOOL BitmapEx::Replace( const Color& rSearchColor, const Color& rReplaceColor, ULONG nTol )
{
    return( !!aBitmap ? aBitmap.Replace( rSearchColor, rReplaceColor, nTol ) : FALSE );
}

// ------------------------------------------------------------------

BOOL BitmapEx::Replace( const Color* pSearchColors, const Color* pReplaceColors, ULONG nColorCount, const ULONG* pTols )
{
    return( !!aBitmap ? aBitmap.Replace( pSearchColors, pReplaceColors, nColorCount, (ULONG*) pTols ) : FALSE );
}

// ------------------------------------------------------------------

BOOL BitmapEx::Adjust( short nLuminancePercent, short nContrastPercent,
                       short nChannelRPercent, short nChannelGPercent, short nChannelBPercent,
                       double fGamma, BOOL bInvert )
{
    return( !!aBitmap ? aBitmap.Adjust( nLuminancePercent, nContrastPercent,
                                        nChannelRPercent, nChannelGPercent, nChannelBPercent,
                                        fGamma, bInvert ) : FALSE );
}

// ------------------------------------------------------------------

BOOL BitmapEx::Filter( BmpFilter eFilter, const BmpFilterParam* pFilterParam, const Link* pProgress )
{
    return( !!aBitmap ? aBitmap.Filter( eFilter, pFilterParam, pProgress ) : FALSE );
}

// ------------------------------------------------------------------

void BitmapEx::Draw( OutputDevice* pOutDev, const Point& rDestPt ) const
{
    pOutDev->DrawBitmapEx( rDestPt, *this );
}

// ------------------------------------------------------------------

void BitmapEx::Draw( OutputDevice* pOutDev,
                     const Point& rDestPt, const Size& rDestSize ) const
{
    pOutDev->DrawBitmapEx( rDestPt, rDestSize, *this );
}

// ------------------------------------------------------------------

void BitmapEx::Draw( OutputDevice* pOutDev,
                     const Point& rDestPt, const Size& rDestSize,
                     const Point& rSrcPtPixel, const Size& rSrcSizePixel ) const
{
    pOutDev->DrawBitmapEx( rDestPt, rDestSize, rSrcPtPixel, rSrcSizePixel, *this );
}

// ------------------------------------------------------------------

SvStream& operator<<( SvStream& rOStm, const BitmapEx& rBitmapEx )
{
    rBitmapEx.aBitmap.Write( rOStm );

    rOStm << (UINT32) 0x25091962;
    rOStm << (UINT32) 0xACB20201;
    rOStm << (BYTE) rBitmapEx.eTransparent;

    if( rBitmapEx.eTransparent == TRANSPARENT_BITMAP )
        rBitmapEx.aMask.Write( rOStm );
    else if( rBitmapEx.eTransparent == TRANSPARENT_COLOR )
        rOStm << rBitmapEx.aTransparentColor;

    return rOStm;
}

// ------------------------------------------------------------------

SvStream& operator>>( SvStream& rIStm, BitmapEx& rBitmapEx )
{
    Bitmap aBmp;

    rIStm >> aBmp;

    if( !rIStm.GetError() )
    {
        const ULONG nStmPos = rIStm.Tell();
        UINT32      nMagic1;
        UINT32      nMagic2;

        rIStm >> nMagic1 >> nMagic2;

        if( ( nMagic1 != 0x25091962 ) || ( nMagic2 != 0xACB20201 ) || rIStm.GetError() )
        {
            rIStm.Seek( nStmPos );
            rBitmapEx = aBmp;
        }
        else
        {
            BYTE bTransparent;

            rIStm >> bTransparent;

            if( bTransparent == (BYTE) TRANSPARENT_BITMAP )
            {
                Bitmap aMask;

                rIStm >> aMask;

                if( !!aMask)
                {
                    // do we have an alpha mask?
                    if( ( 8 == aMask.GetBitCount() ) && aMask.HasGreyPalette() )
                    {
                        AlphaMask aAlpha;

                        // create alpha mask quickly (without greyscale conversion)
                        aAlpha.ImplSetBitmap( aMask );
                        rBitmapEx = BitmapEx( aBmp, aAlpha );
                    }
                    else
                        rBitmapEx = BitmapEx( aBmp, aMask );
                }
                else
                    rBitmapEx = aBmp;
            }
            else if( bTransparent == (BYTE) TRANSPARENT_COLOR )
            {
                Color aTransparentColor;

                rIStm >> aTransparentColor;
                rBitmapEx = BitmapEx( aBmp, aTransparentColor );
            }
            else
                rBitmapEx = aBmp;
        }
    }

    return rIStm;
}

// ------------------------------------------------------------------

#ifdef REMOTE_APPSERVER

void BitmapEx::ImplDrawRemote( OutputDevice* pOut,
                               const Point& rSrcPt, const Size& rSrcSz,
                               const Point& rDestPt, const Size& rDestSz ) const
{
    if( !!aBitmap )
    {
        switch( eTransparent )
        {
            case( TRANSPARENT_NONE ):
                aBitmap.ImplDrawRemote( pOut, rSrcPt, rSrcSz, rDestPt, rDestSz );
            break;

            case( TRANSPARENT_BITMAP ):
            {
                if( !!aMask )
                {
                    if( IsAlpha() )
                        aBitmap.ImplDrawRemoteAlpha( pOut, rSrcPt, rSrcSz, rDestPt, rDestSz, GetAlpha() );
                    else
                        aBitmap.ImplDrawRemoteEx( pOut, rSrcPt, rSrcSz, rDestPt, rDestSz, aMask );
                }
                else
                    aBitmap.ImplDrawRemote( pOut, rSrcPt, rSrcSz, rDestPt, rDestSz );
            }
            break;

            default:
            {
                DBG_ERROR( "BitmapEx::ImplDrawRemote???" );
            }
            break;
        }
    }
}

#endif // REMOTE
