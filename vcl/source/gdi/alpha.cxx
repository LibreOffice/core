/*************************************************************************
 *
 *  $RCSfile: alpha.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ka $ $Date: 2000-11-16 13:29:32 $
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

#define _SV_ALPHA_CXX

#ifndef _DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _SV_BMPACC_HXX
#include <bmpacc.hxx>
#endif
#ifndef _SV_COLOR_HXX
#include <color.hxx>
#endif
#ifndef _SV_ALPHA_HXX
#include <alpha.hxx>
#endif

// -------------
// - AlphaMask -
// -------------

AlphaMask::AlphaMask()
{
}

// -----------------------------------------------------------------------------

AlphaMask::AlphaMask( const Bitmap& rBitmap ) :
    Bitmap( rBitmap )
{
    if( !!rBitmap )
        Bitmap::Convert( BMP_CONVERSION_8BIT_GREYS );
}

// -----------------------------------------------------------------------------

AlphaMask::AlphaMask( const AlphaMask& rAlphaMask ) :
    Bitmap( rAlphaMask )
{
}

// -----------------------------------------------------------------------------

AlphaMask::AlphaMask( const Size& rSizePixel, BYTE* pEraseTransparency ) :
    Bitmap( rSizePixel, 8, &Bitmap::GetGreyPalette( 256 ) )
{
    if( pEraseTransparency )
        Bitmap::Erase( Color( *pEraseTransparency, *pEraseTransparency, *pEraseTransparency ) );
}

// -----------------------------------------------------------------------------

AlphaMask::~AlphaMask()
{
}

// -----------------------------------------------------------------------------

AlphaMask& AlphaMask::operator=( const Bitmap& rBitmap )
{
    *(Bitmap*) this = rBitmap;

    if( !!rBitmap )
        Bitmap::Convert( BMP_CONVERSION_8BIT_GREYS );

    return *this;
}

// -----------------------------------------------------------------------------

const Bitmap& AlphaMask::ImplGetBitmap() const
{
    return( (const Bitmap&) *this );
}

// -----------------------------------------------------------------------------

void AlphaMask::ImplSetBitmap( const Bitmap& rBitmap )
{
    *(Bitmap*) this = rBitmap;
}

// -----------------------------------------------------------------------------

BOOL AlphaMask::Crop( const Rectangle& rRectPixel )
{
    return Bitmap::Crop( rRectPixel );
}

// -----------------------------------------------------------------------------

BOOL AlphaMask::Expand( ULONG nDX, ULONG nDY, BYTE* pInitTransparency )
{
    Color aColor;

    if( pInitTransparency )
        aColor = Color( *pInitTransparency, *pInitTransparency, *pInitTransparency );

    return Bitmap::Expand( nDX, nDY, pInitTransparency ? &aColor : NULL );
}

// -----------------------------------------------------------------------------

BOOL AlphaMask::CopyPixel( const Rectangle& rRectDst, const Rectangle& rRectSrc,
                           const AlphaMask* pAlphaSrc )
{
    return Bitmap::CopyPixel( rRectDst, rRectSrc, (Bitmap*) pAlphaSrc );
}

// -----------------------------------------------------------------------------

BOOL AlphaMask::Erase( BYTE cTransparency )
{
    return Bitmap::Erase( Color( cTransparency, cTransparency, cTransparency ) );
}

// -----------------------------------------------------------------------------

BOOL AlphaMask::Invert()
{
    BitmapWriteAccess*  pAcc = AcquireWriteAccess();
    BOOL                bRet = FALSE;

    if( pAcc && pAcc->GetBitCount() == 8 )
    {
        BitmapColor aCol( 0 );
        const long  nWidth = pAcc->Width(), nHeight = pAcc->Height();
        BYTE*       pMap = new BYTE[ 256 ];

        for( long i = 0; i < 256; i++ )
            pMap[ i ] = ~(BYTE) i;

        for( long nY = 0L; nY < nHeight; nY++ )
        {
            for( long nX = 0L; nX < nWidth; nX++ )
            {
                aCol.SetIndex( pMap[ pAcc->GetPixel( nY, nX ).GetIndex() ] );
                pAcc->SetPixel( nY, nX, aCol );
            }
        }

        delete[] pMap;
        bRet = TRUE;
    }

    if( pAcc )
        ReleaseAccess( pAcc );

    return bRet;
}

// -----------------------------------------------------------------------------

BOOL AlphaMask::Mirror( ULONG nMirrorFlags )
{
    return Bitmap::Mirror( nMirrorFlags );
}

// -----------------------------------------------------------------------------

BOOL AlphaMask::Scale( const Size& rNewSize, ULONG nScaleFlag )
{
    BOOL bRet = Bitmap::Scale( rNewSize, nScaleFlag );

    if( bRet && ( nScaleFlag == BMP_SCALE_INTERPOLATE ) )
        Bitmap::Convert( BMP_CONVERSION_8BIT_GREYS );

    return bRet;
}

// -----------------------------------------------------------------------------

BOOL AlphaMask::Scale( const double& rScaleX, const double& rScaleY, ULONG nScaleFlag )
{
    BOOL bRet = Bitmap::Scale( rScaleX, rScaleY, nScaleFlag );

    if( bRet && ( nScaleFlag == BMP_SCALE_INTERPOLATE ) )
        Bitmap::Convert( BMP_CONVERSION_8BIT_GREYS );

    return bRet;
}

// -----------------------------------------------------------------------------

BOOL AlphaMask::Rotate( long nAngle10, BYTE cFillTransparency )
{
    return Bitmap::Rotate( nAngle10, Color( cFillTransparency, cFillTransparency, cFillTransparency ) );
}

// -----------------------------------------------------------------------------

BOOL AlphaMask::Replace( const Bitmap& rMask, BYTE cReplaceTransparency )
{
    BitmapReadAccess*   pMaskAcc = ( (Bitmap&) rMask ).AcquireReadAccess();
    BitmapWriteAccess*  pAcc = AcquireWriteAccess();
    BOOL                bRet = FALSE;

    if( pMaskAcc && pAcc )
    {
        const BitmapColor   aReplace( cReplaceTransparency );
        const long          nWidth = Min( pMaskAcc->Width(), pAcc->Width() );
        const long          nHeight = Min( pMaskAcc->Height(), pAcc->Height() );
        const BitmapColor   aMaskWhite( pMaskAcc->GetBestMatchingColor( Color( COL_WHITE ) ) );

        for( long nY = 0L; nY < nHeight; nY++ )
            for( long nX = 0L; nX < nWidth; nX++ )
                if( pMaskAcc->GetPixel( nY, nX ) == aMaskWhite )
                    pAcc->SetPixel( nY, nX, aReplace );
    }

    ( (Bitmap&) rMask ).ReleaseAccess( pMaskAcc );
    ReleaseAccess( pAcc );

    return bRet;
}

// -----------------------------------------------------------------------------

BOOL AlphaMask::Replace( BYTE cSearchTransparency, BYTE cReplaceTransparency, ULONG nTol )
{
    BitmapWriteAccess*  pAcc = AcquireWriteAccess();
    BOOL                bRet = FALSE;

    DBG_ASSERT( !nTol, "AlphaMask::Replace: nTol not used yet" );

    if( pAcc && pAcc->GetBitCount() == 8 )
    {
        const long nWidth = pAcc->Width(), nHeight = pAcc->Height();

        if( pAcc->GetScanlineFormat() == BMP_FORMAT_8BIT_PAL )
        {
            for( long nY = 0L; nY < nHeight; nY++ )
            {
                Scanline pScan = pAcc->GetScanline( nY );

                for( long nX = 0L; nX < nWidth; nX++, pScan++ )
                {
                    if( *pScan == cSearchTransparency )
                        *pScan = cReplaceTransparency;
                }
            }
        }
        else
        {
            BitmapColor aReplace( cReplaceTransparency );

            for( long nY = 0L; nY < nHeight; nY++ )
            {
                for( long nX = 0L; nX < nWidth; nX++ )
                {
                    if( pAcc->GetPixel( nY, nX ).GetIndex() == cSearchTransparency )
                        pAcc->SetPixel( nY, nX, aReplace );
                }
            }
        }

        bRet = TRUE;
    }

    if( pAcc )
        ReleaseAccess( pAcc );

    return bRet;
}

// -----------------------------------------------------------------------------

BOOL AlphaMask::Replace( BYTE* pSearchTransparencies, BYTE* pReplaceTransparencies,
                         ULONG nColorCount, ULONG* pTols )
{
    Color*  pSearchColors = new Color[ nColorCount ];
    Color*  pReplaceColors = new Color[ nColorCount ];
    BOOL    bRet;

    for( ULONG i = 0; i < nColorCount; i++ )
    {
        const BYTE cSearchTransparency = pSearchTransparencies[ i ];
        const BYTE cReplaceTransparency = pReplaceTransparencies[ i ];

        pSearchColors[ i ] = Color( cSearchTransparency, cSearchTransparency, cSearchTransparency );
        pReplaceColors[ i ] = Color( cReplaceTransparency, cReplaceTransparency, cReplaceTransparency );
    }

    bRet = Bitmap::Replace( pSearchColors, pReplaceColors, nColorCount, pTols ) &&
           Bitmap::Convert( BMP_CONVERSION_8BIT_GREYS );

    delete[] pSearchColors;
    delete[] pReplaceColors;

    return bRet;
}

// -----------------------------------------------------------------------------

void AlphaMask::ReleaseAccess( BitmapReadAccess* pAccess )
{
    if( pAccess )
    {
        Bitmap::ReleaseAccess( pAccess );
        Bitmap::Convert( BMP_CONVERSION_8BIT_GREYS );
    }
}
