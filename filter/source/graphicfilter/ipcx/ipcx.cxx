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


#include <vcl/graphic.hxx>
#include <vcl/bitmapaccess.hxx>

class FilterConfigItem;

//============================ PCXReader ==================================

class PCXReader {

private:

    SvStream& m_rPCX;               // the PCX file to read

    Bitmap              aBmp;
    sal_uInt8           nVersion;           // PCX-Version
    sal_uInt8           nEncoding;          // compression type
    sal_uLong           nBitsPerPlanePix;   // bits per plane per pixel
    sal_uLong           nPlanes;            // no of planes
    sal_uLong           nBytesPerPlaneLin;  // bytes per plane line
    sal_uInt16          nPaletteInfo;

    sal_uLong           nWidth, nHeight;    // dimension in pixel
    sal_uInt16          nResX, nResY;       // resolution in pixel per inch oder 0,0
    sal_uInt16          nDestBitsPerPixel;  // bits per pixel in destination bitmap 1,4,8 or 24
    sal_uInt8*          pPalette;
    bool                nStatus;            // from now on do not read status from stream ( SJ )


    void                ImplReadBody(BitmapWriteAccess * pAcc);
    void                ImplReadPalette( sal_uLong nCol );
    void                ImplReadHeader();

public:
    explicit PCXReader(SvStream &rStream);
    ~PCXReader();
    bool                ReadPCX(Graphic & rGraphic );
                        // Reads a PCX file from the stream and fills the GDIMetaFile
};

//=================== methods of PCXReader ==============================

PCXReader::PCXReader(SvStream &rStream)
    : m_rPCX(rStream)
    , nVersion(0)
    , nEncoding(0)
    , nBitsPerPlanePix(0)
    , nPlanes(0)
    , nBytesPerPlaneLin(0)
    , nPaletteInfo(0)
    , nWidth(0)
    , nHeight(0)
    , nResX(0)
    , nResY(0)
    , nDestBitsPerPixel(0)
    , nStatus(false)
{
    pPalette = new sal_uInt8[ 768 ];
}

PCXReader::~PCXReader()
{
    delete[] pPalette;
}

bool PCXReader::ReadPCX(Graphic & rGraphic)
{
    if ( m_rPCX.GetError() )
        return false;

    m_rPCX.SetEndian(SvStreamEndian::LITTLE);

    // read header:

    nStatus = true;

    ImplReadHeader();

    // Write BMP header and conditionally (maybe invalid for now) color palette:
    if ( nStatus )
    {
        aBmp = Bitmap( Size( nWidth, nHeight ), nDestBitsPerPixel );
        Bitmap::ScopedWriteAccess pAcc(aBmp);
        if ( pAcc == nullptr )
            return false;

        if ( nDestBitsPerPixel <= 8 )
        {
            sal_uInt16 nColors = 1 << nDestBitsPerPixel;
            sal_uInt8* pPal = pPalette;
            pAcc->SetPaletteEntryCount( nColors );
            for ( sal_uInt16 i = 0; i < nColors; i++, pPal += 3 )
            {
                pAcc->SetPaletteColor( i, BitmapColor ( pPal[ 0 ], pPal[ 1 ], pPal[ 2 ] ) );
            }
        }
        // read bitmap data
        ImplReadBody(pAcc.get());

        // If an extended color palette exists at the end of the file, then read it and
        // and write again in palette:
        if ( nDestBitsPerPixel == 8 && nStatus )
        {
            sal_uInt8* pPal = pPalette;
            m_rPCX.SeekRel(1);
            ImplReadPalette(256);
            pAcc->SetPaletteEntryCount( 256 );
            for ( sal_uInt16 i = 0; i < 256; i++, pPal += 3 )
            {
                pAcc->SetPaletteColor( i, BitmapColor ( pPal[ 0 ], pPal[ 1 ], pPal[ 2 ] ) );
            }
        }
    /*
        // set resolution:
        if (nResX!=0 && nResY!=0) {
            MapMode aMapMode(MAP_INCH,Point(0,0),Fraction(1,nResX),Fraction(1,nResY));
            rBitmap.SetPrefMapMode(aMapMode);
            rBitmap.SetPrefSize(Size(nWidth,nHeight));
        }
    */  if ( nStatus )
        {
            rGraphic = aBmp;
            return true;
        }
    }
    return false;
}

void PCXReader::ImplReadHeader()
{
    sal_uInt8 nbyte(0);
    m_rPCX.ReadUChar( nbyte ).ReadUChar( nVersion ).ReadUChar( nEncoding );
    if ( nbyte!=0x0a || (nVersion != 0 && nVersion != 2 && nVersion != 3 && nVersion != 5) || nEncoding > 1 )
    {
        nStatus = false;
        return;
    }

    nbyte = 0;
    m_rPCX.ReadUChar( nbyte ); nBitsPerPlanePix = (sal_uLong)nbyte;
    sal_uInt16 nMinX(0),nMinY(0),nMaxX(0),nMaxY(0);
    m_rPCX.ReadUInt16( nMinX ).ReadUInt16( nMinY ).ReadUInt16( nMaxX ).ReadUInt16( nMaxY );

    if ((nMinX > nMaxX) || (nMinY > nMaxY))
    {
        nStatus = false;
        return;
    }

    nWidth = nMaxX-nMinX+1;
    nHeight = nMaxY-nMinY+1;

    m_rPCX.ReadUInt16( nResX );
    m_rPCX.ReadUInt16( nResY );
    if ( nResX >= nWidth || nResY >= nHeight || ( nResX != nResY ) )
        nResX = nResY = 0;

    ImplReadPalette( 16 );

    m_rPCX.SeekRel( 1 );
    nbyte = 0;
    m_rPCX.ReadUChar( nbyte );   nPlanes = (sal_uLong)nbyte;
    sal_uInt16 nushort(0);
    m_rPCX.ReadUInt16( nushort ); nBytesPerPlaneLin = (sal_uLong)nushort;
    m_rPCX.ReadUInt16( nPaletteInfo );

    m_rPCX.SeekRel( 58 );

    nDestBitsPerPixel = (sal_uInt16)( nBitsPerPlanePix * nPlanes );
    if (nDestBitsPerPixel == 2 || nDestBitsPerPixel == 3) nDestBitsPerPixel = 4;

    if ( ( nDestBitsPerPixel != 1 && nDestBitsPerPixel != 4 && nDestBitsPerPixel != 8 && nDestBitsPerPixel != 24 )
        || nPlanes > 4 || nBytesPerPlaneLin < ( ( nWidth * nBitsPerPlanePix+7 ) >> 3 ) )
    {
        nStatus = false;
        return;
    }

    // If the bitmap has only 2 colors, the palette is most often invalid and it is always(?)
    // a black and white image:
    if ( nPlanes == 1 && nBitsPerPlanePix == 1 )
    {
        pPalette[ 0 ] = pPalette[ 1 ] = pPalette[ 2 ] = 0x00;
        pPalette[ 3 ] = pPalette[ 4 ] = pPalette[ 5 ] = 0xff;
    }
}

void PCXReader::ImplReadBody(BitmapWriteAccess * pAcc)
{
    sal_uInt8   *pPlane[ 4 ], * pDest;
    sal_uLong   i, nx, ny, np, nCount, nPercent;
    sal_uLong   nLastPercent = 0;
    sal_uInt8   nDat = 0, nCol = 0;

    //sanity check there is enough data before trying allocation
    if (nBytesPerPlaneLin > m_rPCX.remainingSize() / nPlanes)
    {
        nStatus = false;
        return;
    }

    for( np = 0; np < nPlanes; np++ )
        pPlane[ np ] = new sal_uInt8[ nBytesPerPlaneLin ];

    nCount = 0;
    for ( ny = 0; ny < nHeight; ny++ )
    {
        if (!m_rPCX.good())
        {
            nStatus = false;
            break;
        }
        nPercent = ny * 60 / nHeight + 10;
        if ( ny == 0 || nLastPercent + 4 <= nPercent )
        {
            nLastPercent = nPercent;
        }
        for ( np = 0; np < nPlanes; np++)
        {
            if ( nEncoding == 0)
                m_rPCX.Read( static_cast<void *>(pPlane[ np ]), nBytesPerPlaneLin );
            else
            {
                pDest = pPlane[ np ];
                nx = nBytesPerPlaneLin;
                while ( nCount > 0 && nx > 0)
                {
                    *(pDest++) = nDat;
                    nx--;
                    nCount--;
                }
                while (nx > 0 && m_rPCX.good())
                {
                    m_rPCX.ReadUChar( nDat );
                    if ( ( nDat & 0xc0 ) == 0xc0 )
                    {
                        nCount =( (sal_uLong)nDat ) & 0x003f;
                        m_rPCX.ReadUChar( nDat );
                        if ( nCount < nx )
                        {
                            nx -= nCount;
                            while ( nCount > 0)
                            {
                                *(pDest++) = nDat;
                                nCount--;
                            }
                        }
                        else
                        {
                            nCount -= nx;
                            do
                            {
                                *(pDest++) = nDat;
                                nx--;
                            }
                            while ( nx > 0 );
                            break;
                        }
                    }
                    else
                    {
                        *(pDest++) = nDat;
                        nx--;
                    }
                }
            }
        }
        sal_uInt8 *pSource1 = pPlane[ 0 ];
        sal_uInt8 *pSource2 = pPlane[ 1 ];
        sal_uInt8 *pSource3 = pPlane[ 2 ];
        sal_uInt8 *pSource4 = pPlane[ 3 ];
        switch ( nBitsPerPlanePix + ( nPlanes << 8 ) )
        {
            // 2 colors
            case 0x101 :
                for ( i = 0; i < nWidth; i++ )
                {
                    sal_uLong nShift = ( i & 7 ) ^ 7;
                    if ( nShift == 0 )
                        pAcc->SetPixelIndex( ny, i, *(pSource1++) & 1 );
                    else
                        pAcc->SetPixelIndex( ny, i, (*pSource1 >> nShift ) & 1 );
                }
                break;
            // 4 colors
            case 0x102 :
                for ( i = 0; i < nWidth; i++ )
                {
                    switch( i & 3 )
                    {
                        case 0 :
                            nCol = *pSource1 >> 6;
                            break;
                        case 1 :
                            nCol = ( *pSource1 >> 4 ) & 0x03 ;
                            break;
                        case 2 :
                            nCol = ( *pSource1 >> 2 ) & 0x03;
                            break;
                        case 3 :
                            nCol = ( *pSource1++ ) & 0x03;
                            break;
                    }
                    pAcc->SetPixelIndex( ny, i, nCol );
                }
                break;
            // 256 colors
            case 0x108 :
                for ( i = 0; i < nWidth; i++ )
                {
                    pAcc->SetPixelIndex( ny, i, *pSource1++ );
                }
                break;
            // 8 colors
            case 0x301 :
                for ( i = 0; i < nWidth; i++ )
                {
                    sal_uLong nShift = ( i & 7 ) ^ 7;
                    if ( nShift == 0 )
                    {
                        nCol = ( *pSource1++ & 1) + ( ( *pSource2++ << 1 ) & 2 ) + ( ( *pSource3++ << 2 ) & 4 );
                        pAcc->SetPixelIndex( ny, i, nCol );
                    }
                    else
                    {
                        nCol = sal::static_int_cast< sal_uInt8 >(
                            ( ( *pSource1 >> nShift ) & 1)  + ( ( ( *pSource2 >> nShift ) << 1 ) & 2 ) +
                            ( ( ( *pSource3 >> nShift ) << 2 ) & 4 ));
                        pAcc->SetPixelIndex( ny, i, nCol );
                    }
                }
                break;
            // 16 colors
            case 0x401 :
                for ( i = 0; i < nWidth; i++ )
                {
                    sal_uLong nShift = ( i & 7 ) ^ 7;
                    if ( nShift == 0 )
                    {
                        nCol = ( *pSource1++ & 1) + ( ( *pSource2++ << 1 ) & 2 ) + ( ( *pSource3++ << 2 ) & 4 ) +
                            ( ( *pSource4++ << 3 ) & 8 );
                        pAcc->SetPixelIndex( ny, i, nCol );
                    }
                    else
                    {
                        nCol = sal::static_int_cast< sal_uInt8 >(
                            ( ( *pSource1 >> nShift ) & 1)  + ( ( ( *pSource2 >> nShift ) << 1 ) & 2 ) +
                            ( ( ( *pSource3 >> nShift ) << 2 ) & 4 ) + ( ( ( *pSource4 >> nShift ) << 3 ) & 8 ));
                        pAcc->SetPixelIndex( ny, i, nCol );
                    }
                }
                break;
            // 16m colors
            case 0x308 :
                for ( i = 0; i < nWidth; i++ )
                {
                    pAcc->SetPixel( ny, i, Color( *pSource1++, *pSource2++, *pSource3++ ) );

                }
                break;
            default :
                nStatus = false;
                break;
        }
    }
    for ( np = 0; np < nPlanes; np++ )
        delete[] pPlane[ np ];
}

void PCXReader::ImplReadPalette( sal_uLong nCol )
{
    sal_uInt8   r, g, b;
    sal_uInt8*  pPtr = pPalette;
    for ( sal_uLong i = 0; i < nCol; i++ )
    {
        m_rPCX.ReadUChar( r ).ReadUChar( g ).ReadUChar( b );
        *pPtr++ = r;
        *pPtr++ = g;
        *pPtr++ = b;
    }
}

//================== GraphicImport - the exported function ================

extern "C" SAL_DLLPUBLIC_EXPORT bool SAL_CALL
ipxGraphicImport( SvStream & rStream, Graphic & rGraphic, FilterConfigItem* )
{
    PCXReader aPCXReader(rStream);
    bool bRetValue = aPCXReader.ReadPCX(rGraphic);
    if ( !bRetValue )
        rStream.SetError( SVSTREAM_FILEFORMAT_ERROR );
    return bRetValue;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
