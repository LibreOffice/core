/*************************************************************************
 *
 *  $RCSfile: impimage.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:05:38 $
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

#define _SV_IMPIMAGE_CXX

#include <string.h>

#ifndef _SV_OUTDEV_HXX
#include <outdev.hxx>
#endif
#ifndef _SV_BITMAPEX_HXX
#include <bitmapex.hxx>
#endif
#ifndef _SV_WINDOW_HXX
#include <window.hxx>
#endif
#ifndef _SV_BMPACC_HXX
#include <bmpacc.hxx>
#endif
#ifndef _SV_VIRDEV_HXX
#include <virdev.hxx>
#endif
#ifndef _SV_IMAGE_H
#include <image.h>
#endif

// -------------
// - FASTIMAGE -
// -------------

#ifndef REMOTE_APPSERVER
#if defined WIN || defined WNT || defined OS2
#undef FASTTRANSPARENT
extern BOOL bFastTransparent;
#else
#undef FASTTRANSPARENT
#endif
#else
#undef FASTTRANSPARENT
#endif

// -----------
// - Defines -
// -----------

#define IPOS( nPos )                ( Point( (nPos) * aSize.Width(), 0L ) )
#define IMPSYSIMAGEITEM_NOTFREE     ( 0x01 )
#define IMPSYSIMAGEITEM_MASK        ( 0x02 )
#define DISA_ALL                    ( 0xffff )
#define PAINT_ALL                   ( 0xffff )

// ----------------
// - ImplImageBmp -
// ----------------

ImplImageBmp::ImplImageBmp() :
    pInfoAry    ( NULL ),
    nCount      ( 0 ),
    nSize       ( 0 )
{
}

// -----------------------------------------------------------------------

ImplImageBmp::~ImplImageBmp()
{
    delete[] pInfoAry;
}

// -----------------------------------------------------------------------

void ImplImageBmp::Create( long nItemWidth, long nItemHeight, USHORT nInitSize )
{
    const Size aTotalSize( nInitSize * nItemWidth, nItemHeight );

    nCount = 0;
    aSize = Size( nItemWidth, nItemHeight );
    nSize = nInitSize;

    aBmp = Bitmap( aTotalSize, 4 );
    aMask = Bitmap( aTotalSize, 1 );

    delete[] pInfoAry;
    pInfoAry = new BYTE[ nSize ];
    memset( pInfoAry, 0, nSize );
    ImplClearCaches();
}

// -----------------------------------------------------------------------

void ImplImageBmp::Create( const Bitmap& rBmp, const Bitmap& rMaskBmp,
                           const Color& rColor, BOOL bColor,
                           long nItemWidth, long nItemHeight, USHORT nInitSize )
{
    BYTE nStyle = IMPSYSIMAGEITEM_NOTFREE;

    ImplClearCaches();

    if ( bColor || !!rMaskBmp )
        nStyle |= IMPSYSIMAGEITEM_MASK;

    aSize = Size( nItemWidth, nItemHeight );
    nCount = 0;
    nSize = nInitSize;

    delete[] pInfoAry;
    pInfoAry = new BYTE[ nSize ];
    memset( pInfoAry, nStyle, nSize );

    aBmp = rBmp;

    if( !!rMaskBmp )
        aMask = rMaskBmp;
    else if( bColor )
        aMask = aBmp.CreateMask( rColor );

#ifdef FASTTRANSPARENT
    if( nStyle & IMPSYSIMAGEITEM_MASK )
        ImplUpdatePaintBmp( DISA_ALL );
#endif
}

// -----------------------------------------------------------------------

void ImplImageBmp::Expand( USHORT nGrowSize )
{
    const ULONG     nDX = nGrowSize * aSize.Width();
    const USHORT    nOldSize = nSize;
    BYTE*           pNewAry = new BYTE[ nSize += nGrowSize ];

    ImplClearCaches();

    aBmp.Expand( nDX, 0UL );
    aMask.Expand( nDX, 0UL );

    if( !!aDisa )
        aDisa.Expand( nDX, 0UL );

    memset( pNewAry, 0, nSize );
    memcpy( pNewAry, pInfoAry, nOldSize );
    delete[] pInfoAry;
    pInfoAry = pNewAry;
}

// -----------------------------------------------------------------------

void ImplImageBmp::Replace( USHORT nPos, USHORT nSrcPos )
{
    const Rectangle aSrcRect( IPOS( nSrcPos ), aSize );
    const Rectangle aDstRect( IPOS( nPos ), aSize );

    ImplClearCaches();

    aBmp.CopyPixel( aDstRect, aSrcRect );

    if ( pInfoAry[ nSrcPos ] & IMPSYSIMAGEITEM_MASK )
    {
        aMask.CopyPixel( aDstRect, aSrcRect );

        if( !!aDisa )
            aDisa.CopyPixel( aDstRect, aSrcRect );
    }

    pInfoAry[ nPos ] = pInfoAry[ nSrcPos ];
}

// -----------------------------------------------------------------------

void ImplImageBmp::Replace( USHORT nPos, const ImplImageBmp& rImageBmp, USHORT nSrcPos )
{
    const Rectangle aSrcRect( IPOS( nSrcPos ), aSize );
    const Rectangle aDstRect( IPOS( nPos ), aSize );

    ImplClearCaches();

    aBmp.CopyPixel( aDstRect, aSrcRect, &rImageBmp.aBmp );

    if ( rImageBmp.pInfoAry[ nSrcPos ] & IMPSYSIMAGEITEM_MASK )
    {
        aMask.CopyPixel( aDstRect, aSrcRect, &rImageBmp.aMask );

        if( !!aDisa )
            aDisa.CopyPixel( aDstRect, aSrcRect, &rImageBmp.aDisa );
    }

    pInfoAry[ nPos ] = rImageBmp.pInfoAry[ nSrcPos ];
}

// -----------------------------------------------------------------------

void ImplImageBmp::Replace( USHORT nPos, const Bitmap& rBmp )
{
    Point aPoint;
    const Rectangle aSrcRect( aPoint, aSize );
    const Rectangle aDstRect( IPOS( nPos ), aSize );

    ImplClearCaches();

    aBmp.CopyPixel( aDstRect, aSrcRect, &rBmp );
    pInfoAry[ nPos ] &= ~IMPSYSIMAGEITEM_MASK;
}

// -----------------------------------------------------------------------

void ImplImageBmp::Replace( USHORT nPos, const Bitmap& rBmp, const Bitmap& rMaskBmp )
{
    Point aPoint;
    const Rectangle aSrcRect( aPoint, aSize );
    const Rectangle aDstRect( IPOS( nPos ), aSize );

    ImplClearCaches();

    aBmp.CopyPixel( aDstRect, aSrcRect, &rBmp );
    aMask.CopyPixel( aDstRect, aSrcRect, &rMaskBmp );

    if( !!aDisa )
        ImplUpdateDisaBmp( nPos );

    pInfoAry[ nPos ] |= IMPSYSIMAGEITEM_MASK;

#ifdef FASTTRANSPARENT
    ImplUpdatePaintBmp( nPos );
#endif
}

// -----------------------------------------------------------------------

void ImplImageBmp::Replace( USHORT nPos, const Bitmap& rBmp, const Color& rColor )
{
    Replace( nPos, rBmp, rBmp.CreateMask( rColor ) );
}

// -----------------------------------------------------------------------

void ImplImageBmp::Merge( USHORT nPos, USHORT nSrcPos )
{
    if ( !( pInfoAry[ nSrcPos ] & IMPSYSIMAGEITEM_MASK ) )
        Replace( nPos, nSrcPos );
    else
    {
        ImplClearCaches();

        const Rectangle     aSrcRect( IPOS( nSrcPos ), aSize );
        const Rectangle     aDstRect( IPOS( nPos ), aSize );
        BitmapWriteAccess*  pBmp = aBmp.AcquireWriteAccess();
        BitmapWriteAccess*  pMsk = aMask.AcquireWriteAccess();

        if ( pBmp && pMsk )
        {
            const BitmapColor   aMskBlack( pMsk->GetBestMatchingColor( Color( COL_BLACK ) ) );
            BitmapColor         aDstCol, aSrcCol;
            long                nDstLeft = aDstRect.Left();
            long                nDstRight = aDstRect.Right();
            long                nDstBottom = aDstRect.Bottom();
            long                nSrcLeft = aSrcRect.Left();
            long                nSrcRight = aSrcRect.Right();
            long                nSrcTop = aSrcRect.Bottom();

            for( long nDstY = aDstRect.Top(), nSrcY = aSrcRect.Top(); nDstY <= nDstBottom; nDstY++, nSrcY++ )
            {
                for( long nDstX = nDstLeft, nSrcX = nSrcLeft; nDstX <= nDstRight; nDstX++, nSrcX++ )
                {
                    aDstCol = pMsk->GetPixel( nDstY, nDstX );
                    aSrcCol = pMsk->GetPixel( nSrcY, nSrcX );

                    if( aMskBlack == aDstCol )
                    {
                        if( aMskBlack == aSrcCol )
                            pBmp->SetPixel( nDstY, nDstX, pBmp->GetPixel( nSrcY, nSrcX ) );
                    }
                    else if( aMskBlack == aSrcCol )
                    {
                        pBmp->SetPixel( nDstY, nDstX, pBmp->GetPixel( nSrcY, nSrcX ) );
                        pMsk->SetPixel( nDstY, nDstX, aMskBlack );
                    }
                }
            }
        }

        aBmp.ReleaseAccess( pBmp );
        aMask.ReleaseAccess( pMsk );

        if( !!aDisa )
            ImplUpdateDisaBmp( nPos );

        pInfoAry[ nPos ] |= IMPSYSIMAGEITEM_MASK;

#ifdef FASTTRANSPARENT
        ImplUpdatePaintBmp( nPos );
#endif
    }
}

// -----------------------------------------------------------------------

Bitmap ImplImageBmp::GetBitmap( USHORT nPosCount, USHORT* pPosAry ) const
{
    Bitmap  aNewBmp( Size( nPosCount * aSize.Width(), aSize.Height() ), aBmp.GetBitCount() );

    for( USHORT i = 0; i < nPosCount; i++ )
    {
        const Rectangle aSrcRect( IPOS( pPosAry[ i ] ), aSize );
        const Rectangle aDstRect( IPOS( i ), aSize );

        aNewBmp.CopyPixel( aDstRect, aSrcRect, &aBmp );
    }

    return aNewBmp;
}

// -----------------------------------------------------------------------

BOOL ImplImageBmp::HasMaskBitmap() const
{
    return( !!aMask );
}

// -----------------------------------------------------------------------

Bitmap ImplImageBmp::GetMaskBitmap( USHORT nPosCount, USHORT* pPosAry ) const
{
    Bitmap  aNewMask( Size( nPosCount * aSize.Width(), aSize.Height() ), aMask.GetBitCount() );

    for( USHORT i = 0; i < nPosCount; i++ )
    {
        const Rectangle aSrcRect( IPOS( pPosAry[ i ] ), aSize );
        const Rectangle aDstRect( IPOS( i ), aSize );

        aNewMask.CopyPixel( aDstRect, aSrcRect, &aMask );
    }

    return aNewMask;
}

// -----------------------------------------------------------------------

BOOL ImplImageBmp::HasMaskColor() const
{
    return FALSE;
}

// -----------------------------------------------------------------------

Color ImplImageBmp::GetMaskColor() const
{
    return Color();
}

// -----------------------------------------------------------------------

void ImplImageBmp::Draw( USHORT nPos, OutputDevice* pOutDev,
                         const Point& rPos, USHORT nStyle,
                         const Size* pSize )
{
    if( pOutDev->IsDeviceOutputNecessary() )
    {
#ifndef REMOTE_APPSERVER

        if( !aBmpDisp && !!aBmp )
            aBmpDisp = aBmp.CreateDisplayBitmap( pOutDev );

        if( !aMaskDisp && !!aMask )
            aMaskDisp = aMask.CreateDisplayBitmap( pOutDev );

#else // REMOTE_APPSERVER

        if( !aBmpDisp && !!aBmp )
            aBmpDisp = aBmp;

        if( !aMaskDisp && !!aMask )
            aMaskDisp = aMask;

        if( !aDisaDisp && !!aDisa )
            aDisaDisp = aDisa;

#endif // REMOTE_APPSERVER

        if( !aBmpEx )
            aBmpEx = BitmapEx( aBmpDisp, aMaskDisp );

        if( pInfoAry[ nPos ] & IMPSYSIMAGEITEM_MASK )
        {
#ifdef FASTTRANSPARENT
            BOOL bTmp = bFastTransparent;
            bFastTransparent = TRUE;
#endif

            Point   aOutPos = pOutDev->LogicToPixel( rPos );
            Size    aOutSize;
            BOOL    bOldMap = pOutDev->mbMap;

            if( pSize )
                aOutSize = pOutDev->LogicToPixel( *pSize );
            else
                aOutSize = aSize;

            pOutDev->mbMap = FALSE;

            if ( nStyle & IMAGE_DRAW_DISABLE )
            {
                Point aOutPos1( aOutPos.X()+1, aOutPos.Y()+1 );
                const Point aPos( IPOS( nPos) );
                const StyleSettings& rSettings = pOutDev->GetSettings().GetStyleSettings();

                if( !aDisa )
                {
                    aDisa = Bitmap( aBmpEx.GetSizePixel(), 1 );
                    ImplUpdateDisaBmp( DISA_ALL );
#ifndef REMOTE_APPSERVER
                    aDisaDisp = aDisa.CreateDisplayBitmap( pOutDev );
#else // REMOTE_APPSERVER
                    aDisaDisp = aDisa;
#endif // REMOTE_APPSERVER
                }

                if( !aDisaDisp && !!aDisa )
                    aDisaDisp = aDisa.CreateDisplayBitmap( pOutDev );

                pOutDev->DrawMask( aOutPos1, aOutSize, aPos, aSize,
                                   aDisaDisp, rSettings.GetLightColor() );
                pOutDev->DrawMask( aOutPos, aOutSize, aPos, aSize,
                                   aDisaDisp, rSettings.GetShadowColor() );
            }
            else
            {
                BOOL bDrawn = FALSE;

                if( nStyle & ( IMAGE_DRAW_HIGHLIGHT | IMAGE_DRAW_DEACTIVE ) )
                {
                    Bitmap              aTmpBmp( aBmp );
                    BitmapWriteAccess*  pAcc;

                    aTmpBmp.Crop( Rectangle( IPOS( nPos ), aSize ) );
                    pAcc = aTmpBmp.AcquireWriteAccess();

                    if( pAcc )
                    {
                        const StyleSettings&    rSettings = pOutDev->GetSettings().GetStyleSettings();
                        Color                   aColor;
                        BitmapColor             aCol;
                        const long              nW = pAcc->Width();
                        const long              nH = pAcc->Height();
                        BYTE*                   pMapR = new BYTE[ 256 ];
                        BYTE*                   pMapG = new BYTE[ 256 ];
                        BYTE*                   pMapB = new BYTE[ 256 ];
                        long                    nX, nY;

                        if( nStyle & IMAGE_DRAW_HIGHLIGHT )
                            aColor = rSettings.GetHighlightColor();
                        else
                            aColor = rSettings.GetDeactiveColor();

                        const BYTE cR = aColor.GetRed();
                        const BYTE cG = aColor.GetGreen();
                        const BYTE cB = aColor.GetBlue();

                        for( nX = 0L; nX < 256L; nX++ )
                        {
                            pMapR[ nX ] = (BYTE) ( ( ( nY = ( nX + cR ) >> 1 ) > 255 ) ? 255 : nY );
                            pMapG[ nX ] = (BYTE) ( ( ( nY = ( nX + cG ) >> 1 ) > 255 ) ? 255 : nY );
                            pMapB[ nX ] = (BYTE) ( ( ( nY = ( nX + cB ) >> 1 ) > 255 ) ? 255 : nY );
                        }

                        if( pAcc->HasPalette() )
                        {
                            for( USHORT i = 0, nCount = pAcc->GetPaletteEntryCount(); i < nCount; i++ )
                            {
                                const BitmapColor& rCol = pAcc->GetPaletteColor( i );
                                aCol.SetRed( pMapR[ rCol.GetRed() ] );
                                aCol.SetGreen( pMapG[ rCol.GetGreen() ] );
                                aCol.SetBlue( pMapB[ rCol.GetBlue() ] );
                                pAcc->SetPaletteColor( i, aCol );
                            }
                        }
                        else if( pAcc->GetScanlineFormat() == BMP_FORMAT_24BIT_TC_BGR )
                        {
                            for( nY = 0L; nY < nH; nY++ )
                            {
                                Scanline pScan = pAcc->GetScanline( nY );

                                for( nX = 0L; nX < nW; nX++ )
                                {
                                    *pScan = pMapB[ *pScan ]; pScan++;
                                    *pScan = pMapG[ *pScan ]; pScan++;
                                    *pScan = pMapR[ *pScan ]; pScan++;
                                }
                            }
                        }
                        else
                        {
                            for( nY = 0L; nY < nH; nY++ )
                            {
                                for( nX = 0L; nX < nW; nX++ )
                                {
                                    aCol = pAcc->GetPixel( nY, nX );
                                    aCol.SetRed( pMapR[ aCol.GetRed() ] );
                                    aCol.SetGreen( pMapG[ aCol.GetGreen() ] );
                                    aCol.SetBlue( pMapB[ aCol.GetBlue() ] );
                                    pAcc->SetPixel( nY, nX, aCol );
                                }
                            }
                        }

                        delete[] pMapR;
                        delete[] pMapG;
                        delete[] pMapB;
                        aTmpBmp.ReleaseAccess( pAcc );

                        Bitmap aTmpMsk( aMask );
                        aTmpMsk.Crop( Rectangle( IPOS( nPos), aSize ) );
                        pOutDev->DrawBitmapEx( aOutPos, BitmapEx( aTmpBmp, aTmpMsk ) );
                        bDrawn = TRUE;
                    }
                }

                if( !bDrawn )
                    pOutDev->DrawBitmapEx( aOutPos, aOutSize, IPOS( nPos), aSize, aBmpEx );
            }

            pOutDev->mbMap = bOldMap;

#ifdef FASTTRANSPARENT
            bFastTransparent = bTmp;
#endif
        }
        else if( pSize )
            pOutDev->DrawBitmap( rPos, *pSize,
                                 IPOS( nPos), aSize, aBmpEx.GetBitmap() );
        else
            pOutDev->DrawBitmap( rPos, pOutDev->PixelToLogic( aSize ),
                                 IPOS( nPos), aSize, aBmpEx.GetBitmap() );
    }
}

// -----------------------------------------------------------------------

void ImplImageBmp::ImplUpdateDisaBmp( USHORT nPos )
{
    BitmapReadAccess*   pAcc = aBmp.AcquireReadAccess();
    BitmapReadAccess*   pMsk = aMask.AcquireReadAccess();
    BitmapWriteAccess*  pDis = aDisa.AcquireWriteAccess();

    if( pAcc && pMsk && pDis )
    {
        const Color         aWhite( COL_WHITE );
        const Color         aBlack( COL_BLACK );
        const BitmapColor   aAccWhite( pAcc->GetBestMatchingColor( aWhite ) );
        const BitmapColor   aMskWhite( pMsk->GetBestMatchingColor( aWhite ) );
        const BitmapColor   aDisWhite( pDis->GetBestMatchingColor( aWhite ) );
        const BitmapColor   aDisBlack( pDis->GetBestMatchingColor( aBlack ) );
        long                nLeft;
        long                nTop;
        long                nRight;
        long                nBottom;

        if( DISA_ALL != nPos )
        {
            const Point aPos( IPOS( nPos ) );

            nLeft = aPos.X();
            nTop = aPos.Y();
            nRight = nLeft + aSize.Width();
            nBottom = nTop + aSize.Height();
        }
        else
        {
            nLeft = nTop = 0L;
            nRight = pDis->Width();
            nBottom = pDis->Height();
        }

        if( pAcc->GetScanlineFormat() == BMP_FORMAT_4BIT_MSN_PAL &&
            pMsk->GetScanlineFormat() == BMP_FORMAT_1BIT_MSB_PAL )
        {
            // optimized version
            const BYTE cAccTest = aAccWhite.GetIndex();
            const BYTE cMskTest = aMskWhite.GetIndex();

            for( long nY = nTop; nY < nBottom; nY++ )
            {
                Scanline pAccScan = pAcc->GetScanline( nY );
                Scanline pMskScan = pMsk->GetScanline( nY );

                for( long nX = nLeft; nX < nRight; nX++ )
                {
                    if( ( cMskTest == ( pMskScan[ nX >> 3 ] & ( 1 << ( 7 - ( nX & 7 ) ) ) ? 1 : 0 ) ) ||
                        ( cAccTest == ( ( pAccScan[ nX >> 1 ] >> ( nX & 1 ? 0 : 4 ) ) & 0x0f ) ) )
                    {
                        pDis->SetPixel( nY, nX, aDisWhite );
                    }
                    else
                        pDis->SetPixel( nY, nX, aDisBlack );
                }
            }
        }
        else if( pAcc->GetScanlineFormat() == BMP_FORMAT_8BIT_PAL &&
                 pMsk->GetScanlineFormat() == BMP_FORMAT_1BIT_MSB_PAL )
        {
            // optimized version
            const BYTE cAccTest = aAccWhite.GetIndex();
            const BYTE cMskTest = aMskWhite.GetIndex();

            for( long nY = nTop; nY < nBottom; nY++ )
            {
                Scanline pAccScan = pAcc->GetScanline( nY );
                Scanline pMskScan = pMsk->GetScanline( nY );

                for( long nX = nLeft; nX < nRight; nX++ )
                {
                    if( ( cMskTest == ( pMskScan[ nX >> 3 ] & ( 1 << ( 7 - ( nX & 7 ) ) ) ? 1 : 0 ) ) ||
                        ( cAccTest == pAccScan[ nX ] ) )
                    {
                        pDis->SetPixel( nY, nX, aDisWhite );
                    }
                    else
                        pDis->SetPixel( nY, nX, aDisBlack );
                }
            }
        }
        else
        {
            for( long nY = nTop; nY < nBottom; nY++ )
            {
                for( long nX = nLeft; nX < nRight; nX++ )
                {
                    if( ( aMskWhite == pMsk->GetPixel( nY, nX ) ) || ( aAccWhite == pAcc->GetPixel( nY, nX ) ) )
                        pDis->SetPixel( nY, nX, aDisWhite );
                    else
                        pDis->SetPixel( nY, nX, aDisBlack );
                }
            }
        }
    }

    aBmp.ReleaseAccess( pAcc );
    aMask.ReleaseAccess( pMsk );
    aDisa.ReleaseAccess( pDis );
}

// -----------------------------------------------------------------------

void ImplImageBmp::ImplUpdatePaintBmp( USHORT nPos )
{
    BitmapWriteAccess*  pBmp = aBmp.AcquireWriteAccess();
    BitmapReadAccess*   pMsk = aMask.AcquireReadAccess();

    if ( pBmp && pMsk )
    {
        const Color         aBlack( COL_BLACK );
        const BitmapColor   aBmpBlack( pBmp->GetBestMatchingColor( aBlack ) );
        const BitmapColor   aMskBlack( pMsk->GetBestMatchingColor( aBlack ) );
        long                nLeft, nTop, nRight, nBottom;

        if( PAINT_ALL != nPos )
        {
            const Point aPos( IPOS( nPos ) );

            nLeft = aPos.X();
            nTop = aPos.Y();
            nRight = nLeft + aSize.Width();
            nBottom = nTop + aSize.Height();
        }
        else
        {
            nLeft = nTop = 0L;
            nRight = pBmp->Width();
            nBottom = pBmp->Height();
        }

        for( long nY = nTop; nY < nBottom; nY++ )
            for( long nX = nLeft; nX < nRight; nX++ )
                if( aMskBlack != pMsk->GetPixel( nY, nX ) )
                    pBmp->SetPixel( nY, nX, aBmpBlack );
    }

    aBmp.ReleaseAccess( pBmp );
    aMask.ReleaseAccess( pMsk );
}

// -----------------------------------------------------------------------

void ImplImageBmp::ImplClearCaches()
{
    aBmpEx.Clear();
    aBmpDisp = aMaskDisp = aDisaDisp = Bitmap();
}
