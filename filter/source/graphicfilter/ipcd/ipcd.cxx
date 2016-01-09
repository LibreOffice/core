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


#include "rtl/alloc.h"
#include <vcl/graph.hxx>
#include <vcl/bmpacc.hxx>
#include <vcl/svapp.hxx>
#include <vcl/fltcall.hxx>
#include <svl/solar.hrc>
#include <vcl/FilterConfigItem.hxx>

//============================ PCDReader ==================================

// these resolutions are contained in a PCD file:
enum PCDResolution {
    PCDRES_BASE16,  //  192 x  128
    PCDRES_BASE4,   //  384 x  256
    PCDRES_BASE,    //  768 x  512
    // the following ones are compressed
    // and CANNOT be read by us
    PCDRES_4BASE,   // 1536 x 1024
    PCDRES_16BASE   // 3072 x 3072
};

class PCDReader {

private:

    bool bStatus;

    sal_uLong               nLastPercent;

    SvStream &m_rPCD;
    BitmapWriteAccess*  mpAcc;

    sal_uInt8               nOrientation;   // orientation of the picture within the PCD file:
                                        // 0 - spire point up
                                        // 1 - spire points to the right
                                        // 2 - spire points down
                                        // 3 - spire points to the left

    PCDResolution       eResolution;    // which resolution we want

    sal_uLong               nWidth;         // width of the PCD picture
    sal_uLong               nHeight;        // height of the PCD picture
    sal_uLong               nImagePos;      // position of the picture within the PCD file

    // temporary lLue-Green-Red-Bitmap
    sal_uLong               nBMPWidth;
    sal_uLong               nBMPHeight;

    void    CheckPCDImagePacFile();
        // checks whether it's a Photo-CD file with 'Image Pac'

    void    ReadOrientation();
        // reads the orientation and sets nOrientation

    void    ReadImage();

public:

    explicit PCDReader(SvStream &rStream)
        : bStatus(false)
        , nLastPercent(0)
        , m_rPCD(rStream)
        , mpAcc(nullptr)
        , nOrientation(0)
        , eResolution(PCDRES_BASE16)
        , nWidth(0)
        , nHeight(0)
        , nImagePos(0)
        , nBMPWidth(0)
        , nBMPHeight(0)
    {
    }

    bool ReadPCD( Graphic & rGraphic, FilterConfigItem* pConfigItem );
};

//=================== Methods of PCDReader ==============================

bool PCDReader::ReadPCD( Graphic & rGraphic, FilterConfigItem* pConfigItem )
{
    Bitmap       aBmp;

    bStatus      = true;
    nLastPercent = 0;

    // is it a PCD file with a picture? ( sets bStatus == sal_False, if that's not the case):
    CheckPCDImagePacFile();

    // read orientation of the picture:
    ReadOrientation();

    // which resolution do we want?:
    eResolution = PCDRES_BASE;
    if ( pConfigItem )
    {
        sal_Int32 nResolution = pConfigItem->ReadInt32( "Resolution", 2 );
        if ( nResolution == 1 )
            eResolution = PCDRES_BASE4;
        else if ( nResolution == 0 )
            eResolution = PCDRES_BASE16;
    }
    // determine size and position (position within the PCD file) of the picture:
    switch (eResolution)
    {
        case PCDRES_BASE16 :
            nWidth = 192;
            nHeight = 128;
            nImagePos = 8192;
            break;

        case PCDRES_BASE4 :
            nWidth = 384;
            nHeight = 256;
            nImagePos = 47104;
            break;

        case PCDRES_BASE :
            nWidth = 768;
            nHeight = 512;
            nImagePos = 196608;
            break;

        default:
            bStatus = false;
    }
    if ( bStatus )
    {
        if ( ( nOrientation & 0x01 ) == 0 )
        {
            nBMPWidth = nWidth;
            nBMPHeight = nHeight;
        }
        else
        {
            nBMPWidth = nHeight;
            nBMPHeight = nWidth;
        }
        aBmp = Bitmap( Size( nBMPWidth, nBMPHeight ), 24 );
        if ( ( mpAcc = aBmp.AcquireWriteAccess() ) == nullptr )
            return false;

        ReadImage();

        Bitmap::ReleaseAccess( mpAcc ), mpAcc = nullptr;
        rGraphic = aBmp;
    }
    return bStatus;
}



void PCDReader::CheckPCDImagePacFile()
{
    char Buf[ 8 ];

    m_rPCD.Seek( 2048 );
    m_rPCD.Read( Buf, 7 );
    Buf[ 7 ] = 0;
    if (OString(Buf) != "PCD_IPI")
        bStatus = false;
}



void PCDReader::ReadOrientation()
{
    if ( !bStatus )
        return;
    m_rPCD.Seek( 194635 );
    m_rPCD.ReadUChar( nOrientation );
    nOrientation &= 0x03;
}



void PCDReader::ReadImage()
{
    sal_uLong  nx,ny,nW2,nH2,nYPair,ndy,nXPair;
    long   nL,nCb,nCr,nRed,nGreen,nBlue;
    sal_uInt8 * pt;
    sal_uInt8 * pL0; // luminance for each pixel of the 1st row of the current pair of rows
    sal_uInt8 * pL1; // luminance for each pixel of the 2nd row of the current pair of rows
    sal_uInt8 * pCb; // blue chrominance for each 2x2 pixel of the current pair of rows
    sal_uInt8 * pCr; // red chrominance fuer je 2x2 pixel of the current pair of rows
    sal_uInt8 * pL0N, * pL1N, * pCbN, * pCrN; // like above, but for the next pair of rows

    if ( !bStatus )
        return;

    nW2=nWidth>>1;
    nH2=nHeight>>1;

    pL0 =static_cast<sal_uInt8*>(rtl_allocateMemory( nWidth ));
    pL1 =static_cast<sal_uInt8*>(rtl_allocateMemory( nWidth ));
    pCb =static_cast<sal_uInt8*>(rtl_allocateMemory( nW2+1 ));
    pCr =static_cast<sal_uInt8*>(rtl_allocateMemory( nW2+1 ));
    pL0N=static_cast<sal_uInt8*>(rtl_allocateMemory( nWidth ));
    pL1N=static_cast<sal_uInt8*>(rtl_allocateMemory( nWidth ));
    pCbN=static_cast<sal_uInt8*>(rtl_allocateMemory( nW2+1 ));
    pCrN=static_cast<sal_uInt8*>(rtl_allocateMemory( nW2+1 ));

    if ( pL0 == nullptr || pL1 == nullptr || pCb == nullptr || pCr == nullptr ||
        pL0N == nullptr || pL1N == nullptr || pCbN == nullptr || pCrN == nullptr)
    {
        rtl_freeMemory(static_cast<void*>(pL0) );
        rtl_freeMemory(static_cast<void*>(pL1) );
        rtl_freeMemory(static_cast<void*>(pCb) );
        rtl_freeMemory(static_cast<void*>(pCr) );
        rtl_freeMemory(static_cast<void*>(pL0N));
        rtl_freeMemory(static_cast<void*>(pL1N));
        rtl_freeMemory(static_cast<void*>(pCbN));
        rtl_freeMemory(static_cast<void*>(pCrN));
        bStatus = false;
        return;
    }

    m_rPCD.Seek( nImagePos );

    // next pair of rows := first pair of rows:
    m_rPCD.Read( pL0N, nWidth );
    m_rPCD.Read( pL1N, nWidth );
    m_rPCD.Read( pCbN, nW2 );
    m_rPCD.Read( pCrN, nW2 );
    pCbN[ nW2 ] = pCbN[ nW2 - 1 ];
    pCrN[ nW2 ] = pCrN[ nW2 - 1 ];

    for ( nYPair = 0; nYPair < nH2; nYPair++ )
    {
        // current pair of rows := next pair of rows:
        pt=pL0; pL0=pL0N; pL0N=pt;
        pt=pL1; pL1=pL1N; pL1N=pt;
        pt=pCb; pCb=pCbN; pCbN=pt;
        pt=pCr; pCr=pCrN; pCrN=pt;

        // get the next pair of rows:
        if ( nYPair < nH2 - 1 )
        {
            m_rPCD.Read( pL0N, nWidth );
            m_rPCD.Read( pL1N, nWidth );
            m_rPCD.Read( pCbN, nW2 );
            m_rPCD.Read( pCrN, nW2 );
            pCbN[nW2]=pCbN[ nW2 - 1 ];
            pCrN[nW2]=pCrN[ nW2 - 1 ];
        }
        else
        {
            for ( nXPair = 0; nXPair < nW2; nXPair++ )
            {
                pCbN[ nXPair ] = pCb[ nXPair ];
                pCrN[ nXPair ] = pCr[ nXPair ];
            }
        }

        // loop through both rows of the pair of rows:
        for ( ndy = 0; ndy < 2; ndy++ )
        {
            ny = ( nYPair << 1 ) + ndy;

            // loop through X:
            for ( nx = 0; nx < nWidth; nx++ )
            {
                // get/calculate nL,nCb,nCr for the pixel nx,ny:
                nXPair = nx >> 1;
                if ( ndy == 0 )
                {
                    nL = (long)pL0[ nx ];
                    if (( nx & 1 ) == 0 )
                    {
                        nCb = (long)pCb[ nXPair ];
                        nCr = (long)pCr[ nXPair ];
                    }
                    else
                    {
                        nCb = ( ( (long)pCb[ nXPair ] ) + ( (long)pCb[ nXPair + 1 ] ) ) >> 1;
                        nCr = ( ( (long)pCr[ nXPair ] ) + ( (long)pCr[ nXPair + 1 ] ) ) >> 1;
                    }
                }
                else {
                    nL = pL1[ nx ];
                    if ( ( nx & 1 ) == 0 )
                    {
                        nCb = ( ( (long)pCb[ nXPair ] ) + ( (long)pCbN[ nXPair ] ) ) >> 1;
                        nCr = ( ( (long)pCr[ nXPair ] ) + ( (long)pCrN[ nXPair ] ) ) >> 1;
                    }
                    else
                    {
                        nCb = ( ( (long)pCb[ nXPair ] ) + ( (long)pCb[ nXPair + 1 ] ) +
                               ( (long)pCbN[ nXPair ] ) + ( (long)pCbN[ nXPair + 1 ] ) ) >> 2;
                        nCr = ( ( (long)pCr[ nXPair ] ) + ( (long)pCr[ nXPair + 1] ) +
                               ( (long)pCrN[ nXPair ] ) + ( (long)pCrN[ nXPair + 1 ] ) ) >> 2;
                    }
                }
                // conversion of nL,nCb,nCr in nRed,nGreen,nBlue:
                nL *= 89024L;
                nCb -= 156;
                nCr -= 137;
                nRed = ( nL + nCr * 119374L + 0x8000 ) >> 16;
                if ( nRed < 0 )
                    nRed = 0;
                if ( nRed > 255)
                    nRed = 255;
                nGreen = ( nL - nCb * 28198L - nCr * 60761L + 0x8000 ) >> 16;
                if ( nGreen < 0 )
                    nGreen = 0;
                if ( nGreen > 255 )
                    nGreen = 255;
                nBlue = ( nL + nCb * 145352L + 0x8000 ) >> 16;
                if ( nBlue < 0 )
                    nBlue = 0;
                if ( nBlue > 255 )
                    nBlue = 255;

                // register color value in pBMPMap:
                if ( nOrientation < 2 )
                {
                    if ( nOrientation == 0 )
                        mpAcc->SetPixel( ny, nx, BitmapColor( (sal_uInt8)nRed, (sal_uInt8)nGreen, (sal_uInt8)nBlue ) );
                    else
                        mpAcc->SetPixel( nWidth - 1 - nx, ny, BitmapColor( (sal_uInt8)nRed, (sal_uInt8)nGreen, (sal_uInt8)nBlue ) );
                }
                else
                {
                    if ( nOrientation == 2 )
                        mpAcc->SetPixel( nHeight - 1 - ny, ( nWidth - 1 - nx ), BitmapColor( (sal_uInt8)nRed, (sal_uInt8)nGreen, (sal_uInt8)nBlue ) );
                    else
                        mpAcc->SetPixel( nx, ( nHeight - 1 - ny ), BitmapColor( (sal_uInt8)nRed, (sal_uInt8)nGreen, (sal_uInt8)nBlue ) );
                }
            }
        }

        if ( m_rPCD.GetError() )
            bStatus = false;
        if ( !bStatus )
            break;
    }
    rtl_freeMemory(static_cast<void*>(pL0) );
    rtl_freeMemory(static_cast<void*>(pL1) );
    rtl_freeMemory(static_cast<void*>(pCb) );
    rtl_freeMemory(static_cast<void*>(pCr) );
    rtl_freeMemory(static_cast<void*>(pL0N));
    rtl_freeMemory(static_cast<void*>(pL1N));
    rtl_freeMemory(static_cast<void*>(pCbN));
    rtl_freeMemory(static_cast<void*>(pCrN));
}

//================== GraphicImport - the exported Function ================

extern "C" SAL_DLLPUBLIC_EXPORT bool SAL_CALL
icdGraphicImport( SvStream & rStream, Graphic & rGraphic, FilterConfigItem* pConfigItem )
{
    PCDReader aPCDReader(rStream);
    return aPCDReader.ReadPCD(rGraphic, pConfigItem);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
