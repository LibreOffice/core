/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */


#include "rtl/alloc.h"
#include <vcl/graph.hxx>
#include <vcl/bmpacc.hxx>
#include <vcl/svapp.hxx>
#include <vcl/fltcall.hxx>
#include <svl/solar.hrc>
#include <vcl/FilterConfigItem.hxx>




enum PCDResolution {
    PCDRES_BASE16,  
    PCDRES_BASE4,   
    PCDRES_BASE,    
    
    
    PCDRES_4BASE,   
    PCDRES_16BASE   
};

class PCDReader {

private:

    sal_Bool bStatus;

    sal_uLong               nLastPercent;

    SvStream &m_rPCD;
    BitmapWriteAccess*  mpAcc;

    sal_uInt8               nOrientation;   
                                        
                                        
                                        
                                        

    PCDResolution       eResolution;    

    sal_uLong               nWidth;         
    sal_uLong               nHeight;        
    sal_uLong               nImagePos;      

    
    sal_uLong               nBMPWidth;
    sal_uLong               nBMPHeight;

    void    MayCallback(sal_uLong nPercent);

    void    CheckPCDImagePacFile();
        

    void    ReadOrientation();
        

    void    ReadImage(sal_uLong nMinPercent, sal_uLong nMaxPercent);

public:

    PCDReader(SvStream &rStream)
        : m_rPCD(rStream)
    {
    }
    ~PCDReader() {}

    sal_Bool ReadPCD( Graphic & rGraphic, FilterConfigItem* pConfigItem );
};



sal_Bool PCDReader::ReadPCD( Graphic & rGraphic, FilterConfigItem* pConfigItem )
{
    Bitmap       aBmp;

    bStatus      = sal_True;
    nLastPercent = 0;

    MayCallback( 0 );

    
    CheckPCDImagePacFile();

    
    ReadOrientation();

    
    eResolution = PCDRES_BASE;
    if ( pConfigItem )
    {
        sal_Int32 nResolution = pConfigItem->ReadInt32( "Resolution", 2 );
        if ( nResolution == 1 )
            eResolution = PCDRES_BASE4;
        else if ( nResolution == 0 )
            eResolution = PCDRES_BASE16;
    }
    
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
            bStatus = sal_False;
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
        if ( ( mpAcc = aBmp.AcquireWriteAccess() ) == 0 )
            return sal_False;

        ReadImage( 5 ,65 );

        aBmp.ReleaseAccess( mpAcc ), mpAcc = NULL;
        rGraphic = aBmp;
    }
    return bStatus;
}



void PCDReader::MayCallback(sal_uLong /*nPercent*/)
{
}



void PCDReader::CheckPCDImagePacFile()
{
    char Buf[ 8 ];

    m_rPCD.Seek( 2048 );
    m_rPCD.Read( Buf, 7 );
    Buf[ 7 ] = 0;
    if (OString(Buf) != "PCD_IPI")
        bStatus = sal_False;
}



void PCDReader::ReadOrientation()
{
    if ( bStatus == sal_False )
        return;
    m_rPCD.Seek( 194635 );
    m_rPCD.ReadUChar( nOrientation );
    nOrientation &= 0x03;
}



void PCDReader::ReadImage(sal_uLong nMinPercent, sal_uLong nMaxPercent)
{
    sal_uLong  nx,ny,nW2,nH2,nYPair,ndy,nXPair;
    long   nL,nCb,nCr,nRed,nGreen,nBlue;
    sal_uInt8 * pt;
    sal_uInt8 * pL0; 
    sal_uInt8 * pL1; 
    sal_uInt8 * pCb; 
    sal_uInt8 * pCr; 
    sal_uInt8 * pL0N, * pL1N, * pCbN, * pCrN; 

    if ( bStatus == sal_False )
        return;

    nW2=nWidth>>1;
    nH2=nHeight>>1;

    pL0 =(sal_uInt8*)rtl_allocateMemory( nWidth );
    pL1 =(sal_uInt8*)rtl_allocateMemory( nWidth );
    pCb =(sal_uInt8*)rtl_allocateMemory( nW2+1 );
    pCr =(sal_uInt8*)rtl_allocateMemory( nW2+1 );
    pL0N=(sal_uInt8*)rtl_allocateMemory( nWidth );
    pL1N=(sal_uInt8*)rtl_allocateMemory( nWidth );
    pCbN=(sal_uInt8*)rtl_allocateMemory( nW2+1 );
    pCrN=(sal_uInt8*)rtl_allocateMemory( nW2+1 );

    if ( pL0 == NULL || pL1 == NULL || pCb == NULL || pCr == NULL ||
        pL0N == NULL || pL1N == NULL || pCbN == NULL || pCrN == NULL)
    {
        rtl_freeMemory((void*)pL0 );
        rtl_freeMemory((void*)pL1 );
        rtl_freeMemory((void*)pCb );
        rtl_freeMemory((void*)pCr );
        rtl_freeMemory((void*)pL0N);
        rtl_freeMemory((void*)pL1N);
        rtl_freeMemory((void*)pCbN);
        rtl_freeMemory((void*)pCrN);
        bStatus = sal_False;
        return;
    }

    m_rPCD.Seek( nImagePos );

    
    m_rPCD.Read( pL0N, nWidth );
    m_rPCD.Read( pL1N, nWidth );
    m_rPCD.Read( pCbN, nW2 );
    m_rPCD.Read( pCrN, nW2 );
    pCbN[ nW2 ] = pCbN[ nW2 - 1 ];
    pCrN[ nW2 ] = pCrN[ nW2 - 1 ];

    for ( nYPair = 0; nYPair < nH2; nYPair++ )
    {
        
        pt=pL0; pL0=pL0N; pL0N=pt;
        pt=pL1; pL1=pL1N; pL1N=pt;
        pt=pCb; pCb=pCbN; pCbN=pt;
        pt=pCr; pCr=pCrN; pCrN=pt;

        
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

        
        for ( ndy = 0; ndy < 2; ndy++ )
        {
            ny = ( nYPair << 1 ) + ndy;

            
            for ( nx = 0; nx < nWidth; nx++ )
            {
                
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
            bStatus = sal_False;
        MayCallback( nMinPercent + ( nMaxPercent - nMinPercent ) * nYPair / nH2 );
        if ( bStatus == sal_False )
            break;
    }
    rtl_freeMemory((void*)pL0 );
    rtl_freeMemory((void*)pL1 );
    rtl_freeMemory((void*)pCb );
    rtl_freeMemory((void*)pCr );
    rtl_freeMemory((void*)pL0N);
    rtl_freeMemory((void*)pL1N);
    rtl_freeMemory((void*)pCbN);
    rtl_freeMemory((void*)pCrN);
}






#if defined(DISABLE_DYNLOADING)
#define GraphicImport icdGraphicImport
#endif

extern "C" SAL_DLLPUBLIC_EXPORT sal_Bool SAL_CALL
GraphicImport(SvStream & rStream, Graphic & rGraphic, FilterConfigItem* pConfigItem, sal_Bool)
{
    PCDReader aPCDReader(rStream);
    return aPCDReader.ReadPCD(rGraphic, pConfigItem);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
