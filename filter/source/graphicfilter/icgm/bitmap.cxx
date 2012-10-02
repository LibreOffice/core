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


#include "main.hxx"

// ---------------------------------------------------------------

CGMBitmap::CGMBitmap( CGM& rCGM ) :
    mpCGM                           ( &rCGM ),
    pCGMBitmapDescriptor            ( new CGMBitmapDescriptor )
{
    ImplGetBitmap( *pCGMBitmapDescriptor );
};

// ---------------------------------------------------------------

CGMBitmap::~CGMBitmap()
{
    delete pCGMBitmapDescriptor;
}

// ---------------------------------------------------------------

void CGMBitmap::ImplGetBitmap( CGMBitmapDescriptor& rDesc )
{
    rDesc.mbStatus = sal_True;

    if ( ImplGetDimensions( rDesc ) && rDesc.mpBuf )
    {
        if ( ( rDesc.mpBitmap = new Bitmap( Size( rDesc.mnX, rDesc.mnY ), (sal_uInt16)rDesc.mnDstBitsPerPixel ) ) != NULL )
        {
            if ( ( rDesc.mpAcc = rDesc.mpBitmap->AcquireWriteAccess() ) != NULL )
            {

                // the picture may either be read from left to right or right to left, from top to bottom ...

                long nxCount = rDesc.mnX + 1;   // +1 because we are using prefix decreasing
                long nyCount = rDesc.mnY + 1;
                long    nx, ny, nxC;

                switch ( rDesc.mnDstBitsPerPixel )
                {
                    case 1 :
                    {
                        if ( rDesc.mnLocalColorPrecision == 1 )
                            ImplSetCurrentPalette( rDesc );
                        else
                        {
                            rDesc.mpAcc->SetPaletteEntryCount( 2 );
                            rDesc.mpAcc->SetPaletteColor( 0, BMCOL( mpCGM->pElement->nBackGroundColor ) );
                            rDesc.mpAcc->SetPaletteColor( 1,
                                ( mpCGM->pElement->nAspectSourceFlags & ASF_FILLINTERIORSTYLE )
                                    ? BMCOL( mpCGM->pElement->pFillBundle->GetColor() )
                                        : BMCOL( mpCGM->pElement->aFillBundle.GetColor() ) ) ;
                        }
                        for ( ny = 0; --nyCount ; ny++, rDesc.mpBuf += rDesc.mnScanSize )
                        {
                            nxC = nxCount;
                            for ( nx = 0; --nxC; nx++ )
                            {   // this is not fast, but a one bit/pixel format is rarely used
                                rDesc.mpAcc->SetPixel( ny, nx, (sal_Int8)( (*( rDesc.mpBuf + ( nx >> 3 ) ) >> ( ( nx & 7 ) ^ 7 ) ) ) & 1 );
                            }
                        }
                    }
                    break;

                    case 2 :
                    {
                        ImplSetCurrentPalette( rDesc );
                        for ( ny = 0; --nyCount; ny++, rDesc.mpBuf += rDesc.mnScanSize )
                        {
                            nxC = nxCount;
                            for ( nx = 0; --nxC; nx++ )
                            {   // this is not fast, but a two bits/pixel format is rarely used
                                rDesc.mpAcc->SetPixel( ny, nx, (sal_Int8)( (*( rDesc.mpBuf + ( nx >> 2 ) ) >> ( ( ( nx & 3 ) ^ 3 ) << 1 ) ) ) & 3 );
                            }
                        }
                    }
                    break;

                    case 4 :
                    {
                        ImplSetCurrentPalette( rDesc );
                        for ( ny = 0; --nyCount; ny++, rDesc.mpBuf += rDesc.mnScanSize )
                        {
                            nxC = nxCount;
                            sal_Int8  nDat;
                            sal_uInt8* pTemp = rDesc.mpBuf;
                            for ( nx = 0; --nxC; nx++ )
                            {
                                nDat = *pTemp++;
                                rDesc.mpAcc->SetPixel( ny, nx, (sal_Int8)( nDat >> 4 ) );
                                if ( --nxC )
                                {
                                    nx ++;
                                    rDesc.mpAcc->SetPixel( ny, nx, (sal_Int8)( nDat & 15 ) );
                                }
                                else
                                    break;
                            }
                        }
                    }
                    break;

                    case 8 :
                    {
                        ImplSetCurrentPalette( rDesc );
                        for ( ny = 0; --nyCount; ny++, rDesc.mpBuf += rDesc.mnScanSize )
                        {
                            sal_uInt8* pTemp = rDesc.mpBuf;
                            nxC = nxCount;
                            for ( nx = 0; --nxC; nx++ )
                            {
                                rDesc.mpAcc->SetPixel( ny, nx, (sal_Int8)( *pTemp++ ) );
                            }
                        }
                    }
                    break;

                    case 24 :
                    {
                        {
                            BitmapColor aBitmapColor;
                            for ( ny = 0; --nyCount; ny++, rDesc.mpBuf += rDesc.mnScanSize )
                            {
                                sal_uInt8* pTemp = rDesc.mpBuf;
                                nxC = nxCount;
                                for ( nx = 0; --nxC; nx++ )
                                {
                                    aBitmapColor.SetRed( (sal_Int8)*pTemp++ );
                                    aBitmapColor.SetGreen( (sal_Int8)*pTemp++ );
                                    aBitmapColor.SetBlue( (sal_Int8)*pTemp++ );
                                    rDesc.mpAcc->SetPixel( ny, nx, aBitmapColor );
                                }
                            }
                        }
                    }
                    break;
                };
                double nX = rDesc.mnR.X - rDesc.mnQ.X;
                double nY = rDesc.mnR.Y - rDesc.mnQ.Y;

                rDesc.mndy = sqrt( nX * nX + nY * nY );

                nX = rDesc.mnR.X - rDesc.mnP.X;
                nY = rDesc.mnR.Y - rDesc.mnP.Y;

                rDesc.mndx = sqrt( nX * nX + nY * nY );

                nX = rDesc.mnR.X - rDesc.mnP.X;
                nY = rDesc.mnR.Y - rDesc.mnP.Y;

                rDesc.mnOrientation = acos( nX / sqrt( nX * nX + nY * nY ) ) * 57.29577951308;
                if ( nY > 0 )
                    rDesc.mnOrientation = 360 - rDesc.mnOrientation;

                nX = rDesc.mnQ.X - rDesc.mnR.X;
                nY = rDesc.mnQ.Y - rDesc.mnR.Y;

                double fAngle = 0.01745329251994 * ( 360 - rDesc.mnOrientation );
                double fSin = sin(fAngle);
                double fCos = cos(fAngle);
                nX = fCos * nX + fSin * nY;
                nY = -( fSin * nX - fCos * nY );

                fAngle = acos( nX / sqrt( nX * nX + nY * nY ) ) * 57.29577951308;
                if ( nY > 0 )
                    fAngle = 360 - fAngle;

                if ( fAngle > 180 )                 // wird das bild nach oben oder unten aufgebaut ?
                {
                    rDesc.mnOrigin = rDesc.mnP;
                }
                else
                {
                    rDesc.mbVMirror = sal_True;
                    rDesc.mnOrigin = rDesc.mnP;
                    rDesc.mnOrigin.X += rDesc.mnQ.X - rDesc.mnR.X;
                    rDesc.mnOrigin.Y += rDesc.mnQ.Y - rDesc.mnR.Y;
                }
            }
            else
                rDesc.mbStatus = sal_False;
        }
        else
            rDesc.mbStatus = sal_False;
    }
    else
        rDesc.mbStatus = sal_False;

    if ( rDesc.mpAcc )
    {
        rDesc.mpBitmap->ReleaseAccess( rDesc.mpAcc );
        rDesc.mpAcc = NULL;
    }
    if ( rDesc.mbStatus == sal_False )
    {
        if ( rDesc.mpBitmap )
        {
            delete rDesc.mpBitmap;
            rDesc.mpBitmap = NULL;
        }
    }
}

// ---------------------------------------------------------------

void CGMBitmap::ImplSetCurrentPalette( CGMBitmapDescriptor& rDesc )
{
    sal_uInt16 nColors = sal::static_int_cast< sal_uInt16 >(
        1 << rDesc.mnDstBitsPerPixel);
    rDesc.mpAcc->SetPaletteEntryCount( nColors );
    for ( sal_uInt16 i = 0; i < nColors; i++ )
    {
        rDesc.mpAcc->SetPaletteColor( i, BMCOL( mpCGM->pElement->aLatestColorTable[ i ] ) );
    }
}

// ---------------------------------------------------------------

sal_Bool CGMBitmap::ImplGetDimensions( CGMBitmapDescriptor& rDesc )
{
    mpCGM->ImplGetPoint( rDesc.mnP );           // parallelogram    p < - > r
    mpCGM->ImplGetPoint( rDesc.mnQ );           //                          |
    mpCGM->ImplGetPoint( rDesc.mnR );           //                          q
    sal_uInt32 nPrecision = mpCGM->pElement->nIntegerPrecision;
    rDesc.mnX = mpCGM->ImplGetUI( nPrecision );
    rDesc.mnY = mpCGM->ImplGetUI( nPrecision );
    rDesc.mnLocalColorPrecision = mpCGM->ImplGetI( nPrecision );
    rDesc.mnScanSize = 0;
    switch( rDesc.mnLocalColorPrecision )
    {
        case long(0x80000001) :                 // monochrome ( bit = 0->backgroundcolor )
        case 0 :                                //              bit = 1->fillcolor
            rDesc.mnDstBitsPerPixel = 1;
            break;
        case 1 :                                // 2 color indexed ( monochrome )
        case -1 :
            rDesc.mnDstBitsPerPixel = 1;
            break;
        case 2 :                                // 4 color indexed
        case -2 :
            rDesc.mnDstBitsPerPixel = 2;
            break;
        case 4 :                                // 16 color indexed
        case -4 :
            rDesc.mnDstBitsPerPixel = 4;
            break;
        case 8 :                                // 256 color indexed
        case -8 :
            rDesc.mnDstBitsPerPixel = 8;
            rDesc.mnScanSize = rDesc.mnX;
            break;
        case 16 :                               // NS
        case -16 :
            rDesc.mbStatus = sal_False;
            break;
        case 24 :                               // 24 bit directColor ( 8 bits each component )
        case -24 :
            rDesc.mnDstBitsPerPixel = 24;
            break;
        case 32 :                               // NS
        case -32 :
            rDesc.mbStatus = sal_False;
            break;

    }
    // mnCompressionMode == 0 : CCOMP_RUNLENGTH
    //                   == 1 : CCOMP_PACKED ( no compression. each row starts on a 4 byte boundary )
    if ( ( rDesc.mnCompressionMode = mpCGM->ImplGetUI16() ) != 1 )
        rDesc.mbStatus = sal_False;

    if ( ( rDesc.mnX || rDesc.mnY ) == 0 )
        rDesc.mbStatus = sal_False;

    sal_uInt32 nHeaderSize = 2 + 3 * nPrecision + 3 * mpCGM->ImplGetPointSize();
    rDesc.mnScanSize = ( ( rDesc.mnX * rDesc.mnDstBitsPerPixel + 7 ) >> 3 );

    sal_uInt32  nScanSize;
    nScanSize = rDesc.mnScanSize;
    if ( ( nScanSize * rDesc.mnY + nHeaderSize ) != mpCGM->mnElementSize )  // try a scansize without dw alignment
    {
        nScanSize = ( rDesc.mnScanSize + 1 ) & ~1;
        if ( ( nScanSize * rDesc.mnY + nHeaderSize ) != mpCGM->mnElementSize )  // then we'll try word alignment
        {
            nScanSize = ( rDesc.mnScanSize + 3 ) & ~3;
            if ( ( nScanSize * rDesc.mnY + nHeaderSize ) != mpCGM->mnElementSize )  // and last we'll try dword alignment
            {
                nScanSize = ( rDesc.mnScanSize + 1 ) & ~1;          // and LAST BUT NOT LEAST we'll try word alignment without aligning the last line
                if ( ( nScanSize * ( rDesc.mnY - 1 ) + rDesc.mnScanSize + nHeaderSize ) != mpCGM->mnElementSize )
                {
                    nScanSize = ( rDesc.mnScanSize + 3 ) & ~3;
                    if ( ( nScanSize * ( rDesc.mnY - 1 ) + rDesc.mnScanSize + nHeaderSize ) != mpCGM->mnElementSize )
                    {
                        mpCGM->mnParaSize = 0;                              // this format is corrupt
                        rDesc.mbStatus = sal_False;
                    }
                }
            }
        }
    }
    rDesc.mnScanSize = nScanSize;
    if ( rDesc.mbStatus )
    {
        rDesc.mpBuf = mpCGM->mpSource + mpCGM->mnParaSize;  // mpBuf now points to the first scanline
        mpCGM->mnParaSize += rDesc.mnScanSize * rDesc.mnY;
    }
    return rDesc.mbStatus;
}

// ---------------------------------------------------------------

void CGMBitmap::ImplInsert( CGMBitmapDescriptor& rSource, CGMBitmapDescriptor& rDest )
{
    if ( ( rSource.mnR.Y == rDest.mnQ.Y ) && ( rSource.mnR.X == rDest.mnQ.X ) )
    {   // Insert on Bottom
        if ( mpCGM->mnVDCYmul == -1 )
            rDest.mnOrigin = rSource.mnOrigin;          // neuer origin
        rDest.mpBitmap->Expand( 0, rSource.mnY );
        rDest.mpBitmap->CopyPixel( Rectangle( Point( 0, rDest.mnY ), Size( rSource.mnX, rSource.mnY ) ),
            Rectangle( Point( 0, 0 ), Size( rSource.mnX, rSource.mnY ) ), rSource.mpBitmap );
        FloatPoint aFloatPoint;
        aFloatPoint.X = rSource.mnQ.X - rSource.mnR.X;
        aFloatPoint.Y = rSource.mnQ.Y - rSource.mnR.Y;
        rDest.mnQ.X += aFloatPoint.X;
        rDest.mnQ.Y += aFloatPoint.Y;
        rDest.mnP = rSource.mnP;
        rDest.mnR = rSource.mnR;
    }
    else
    {   // Insert on Top
        if ( mpCGM->mnVDCYmul == 1 )
            rDest.mnOrigin = rSource.mnOrigin;          // neuer origin
        rDest.mpBitmap->Expand( 0, rSource.mnY );
        rDest.mpBitmap->CopyPixel( Rectangle( Point( 0, rDest.mnY ), Size( rSource.mnX, rSource.mnY ) ),
            Rectangle( Point( 0, 0 ), Size( rSource.mnX, rSource.mnY ) ), rSource.mpBitmap );
        rDest.mnP = rSource.mnP;
        rDest.mnR = rSource.mnR;
    }
    rDest.mnY += rSource.mnY;
    rDest.mndy += rSource.mndy;
};

// ---------------------------------------------------------------

CGMBitmap* CGMBitmap::GetNext()
{
    if ( pCGMBitmapDescriptor->mpBitmap && pCGMBitmapDescriptor->mbStatus )
    {
        CGMBitmap* pCGMTempBitmap = new CGMBitmap( *mpCGM );
        if ( pCGMTempBitmap )
        {
            if ( ( (long)pCGMTempBitmap->pCGMBitmapDescriptor->mnOrientation == (long)pCGMBitmapDescriptor->mnOrientation ) &&
                ( ( ( pCGMTempBitmap->pCGMBitmapDescriptor->mnR.X == pCGMBitmapDescriptor->mnQ.X ) &&
                        ( pCGMTempBitmap->pCGMBitmapDescriptor->mnR.Y == pCGMBitmapDescriptor->mnQ.Y ) ) ||
                ( ( pCGMTempBitmap->pCGMBitmapDescriptor->mnQ.X == pCGMBitmapDescriptor->mnR.X ) &&
                        ( pCGMTempBitmap->pCGMBitmapDescriptor->mnQ.Y == pCGMBitmapDescriptor->mnR.Y ) ) ) )
            {
                ImplInsert( *(pCGMTempBitmap->pCGMBitmapDescriptor), *(pCGMBitmapDescriptor) );
                delete pCGMTempBitmap;
                return NULL;
            }
            else    // we'll replace the pointers and return the old one
            {
                CGMBitmapDescriptor* pTempBD = pCGMBitmapDescriptor;
                pCGMBitmapDescriptor = pCGMTempBitmap->pCGMBitmapDescriptor;
                pCGMTempBitmap->pCGMBitmapDescriptor = pTempBD;
                return pCGMTempBitmap;
            }
        }
        return NULL;
    }
    else
        return NULL;
}

// ---------------------------------------------------------------

CGMBitmapDescriptor* CGMBitmap::GetBitmap()
{
    return pCGMBitmapDescriptor;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
