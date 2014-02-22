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


#include <vcl/bmpacc.hxx>
#include <vcl/graph.hxx>
#include "rgbtable.hxx"
#define _XPMPRIVATE
#include "xpmread.hxx"





XPMReader::XPMReader( SvStream& rStm ) :
            mrIStm          ( rStm ),
            mpAcc           ( NULL ),
            mpMaskAcc       ( NULL ),
            mnLastPos       ( rStm.Tell() ),
            mnWidth         ( 0 ),
            mnHeight        ( 0 ),
            mnColors        ( 0 ),
            mnCpp           ( 0 ),
            mbTransparent   ( false ),
            mbStatus        ( true ),
            mnStatus        ( 0 ),
            mnIdentifier    ( XPMIDENTIFIER ),
            mcThisByte      ( 0 ),
            mnTempAvail     ( 0 ),
            mpFastColorTable( NULL ),
            mpColMap        ( NULL )
{

}



XPMReader::~XPMReader()
{
    if( mpAcc )
        maBmp.ReleaseAccess( mpAcc );
}



ReadState XPMReader::ReadXPM( Graphic& rGraphic )
{
    ReadState   eReadState;
    sal_uInt8       cDummy;

    
    mrIStm.Seek( STREAM_SEEK_TO_END );
    mrIStm.ReadUChar( cDummy );

    
    
    if ( mrIStm.GetError() != ERRCODE_IO_PENDING )
    {
        mrIStm.Seek( mnLastPos );
        mbStatus = true;

        if ( mbStatus )
        {
            mpStringBuf = new sal_uInt8 [ XPMSTRINGBUF ];
            mpTempBuf = new sal_uInt8 [ XPMTEMPBUFSIZE ];

            if ( ( mbStatus = ImplGetString() ) )
            {
                mnIdentifier = XPMVALUES;           
                mnWidth = ImplGetULONG( 0 );
                mnHeight = ImplGetULONG( 1 );
                mnColors = ImplGetULONG( 2 );
                mnCpp = ImplGetULONG( 3 );
            }
            if ( mnColors > ( SAL_MAX_UINT32 / ( 4 + mnCpp ) ) )
                mbStatus = false;
            if ( ( mnWidth * mnCpp ) >= XPMSTRINGBUF )
                mbStatus = false;
            if ( mbStatus && mnWidth && mnHeight && mnColors && mnCpp )
            {
                mnIdentifier = XPMCOLORS;

                
                
                
                
                mpColMap = new sal_uInt8[ mnColors * ( 4 + mnCpp ) ];
                if ( mpColMap )
                {
                    for ( sal_uLong i = 0; i < mnColors; i++ )
                    {
                        if ( !ImplGetColor( i ) )
                        {
                            mbStatus = false;
                            break;
                        }
                    }
                }
                else
                    mbStatus = false;

                if ( mbStatus )
                {
                    
                    sal_uInt16  nBits = 1;
                    if ( mnColors > 256 )
                        nBits = 24;
                    else if ( mnColors > 16 )
                        nBits = 8;
                    else if ( mnColors > 2 )
                        nBits = 4;
                    else
                        nBits = 1;

                    maBmp = Bitmap( Size( mnWidth, mnHeight ), nBits );
                    mpAcc = maBmp.AcquireWriteAccess();

                    
                    if ( mbTransparent )
                    {
                        maMaskBmp = Bitmap( Size( mnWidth, mnHeight ), 1 );
                        if ( ( mpMaskAcc = maMaskBmp.AcquireWriteAccess() ) == NULL )
                            mbStatus = false;
                    }
                    if( mpAcc && mbStatus )
                    {
                        sal_uLong   i;
                        if ( mnColors <=256 )   
                        {                       

                            sal_uInt8*  pPtr = &mpColMap[mnCpp];

                            for ( i = 0; i < mnColors; i++ )
                            {
                                mpAcc->SetPaletteColor( (sal_uInt8)i, Color( pPtr[1], pPtr[2], pPtr[3] ) );
                                pPtr += ( mnCpp + 4 );
                            }
                            
                            if ( mnCpp == 2 )   
                            {
                                mpFastColorTable = new sal_uInt8[ 256 * 256 ];
                                for ( pPtr = mpColMap, i = 0; i < mnColors; i++, pPtr += mnCpp + 4 )
                                {
                                    sal_uLong   j =  pPtr[ 0 ] << 8;
                                            j += pPtr[ 1 ];
                                    mpFastColorTable[ j ] = (sal_uInt8)i;
                                }
                            }
                        }
                        
                        mnIdentifier = XPMPIXELS;
                        for ( i = 0; i < mnHeight; i++ )
                        {
                            if ( !ImplGetScanLine( i ) )
                            {
                                mbStatus = false;
                                break;
                            }
                        }
                        mnIdentifier = XPMEXTENSIONS;
                    }
                }
            }

            delete[] mpFastColorTable;
            delete[] mpColMap;
            delete[] mpStringBuf;
            delete[] mpTempBuf;

        }
        if( mbStatus )
        {
            if ( mpMaskAcc )
            {
                maMaskBmp.ReleaseAccess ( mpMaskAcc), mpMaskAcc = NULL;
                maBmp.ReleaseAccess( mpAcc ), mpAcc = NULL;
                rGraphic = Graphic( BitmapEx( maBmp, maMaskBmp ) );
            }
            else
            {
                maBmp.ReleaseAccess( mpAcc ), mpAcc = NULL;
                rGraphic = maBmp;
            }
            eReadState = XPMREAD_OK;
        }
        else
        {
            if ( mpMaskAcc ) maMaskBmp.ReleaseAccess ( mpMaskAcc), mpMaskAcc = NULL;
            if ( mpAcc ) maBmp.ReleaseAccess( mpAcc ), mpAcc = NULL;
            eReadState = XPMREAD_ERROR;
        }
    }
    else
    {
        mrIStm.ResetError();
        eReadState = XPMREAD_NEED_MORE;
    }
    return eReadState;
}





bool XPMReader::ImplGetColor( sal_uLong nNumb )
{
    sal_uInt8*  pString = mpStringBuf;
    sal_uInt8*  pPtr =  ( mpColMap + nNumb * ( 4 + mnCpp ) );
    bool    bStatus = ImplGetString();

    if ( bStatus )
    {
        for ( sal_uLong i = 0; i < mnCpp; i++ )
            *pPtr++ = *pString++;
        bStatus = ImplGetColSub ( pPtr );
    }
    return bStatus;
}





bool XPMReader::ImplGetScanLine( sal_uLong nY )
{
    bool    bStatus = ImplGetString();
    sal_uInt8*  pString = mpStringBuf;
    sal_uInt8*  pColor;
    BitmapColor     aWhite;
    BitmapColor     aBlack;

    if ( bStatus )
    {
        if ( mpMaskAcc )
        {
            aWhite = mpMaskAcc->GetBestMatchingColor( Color( COL_WHITE ) );
            aBlack = mpMaskAcc->GetBestMatchingColor( Color( COL_BLACK ) );
        }
        if ( mnStringSize != ( mnWidth * mnCpp ))
            bStatus = false;
        else
        {
            sal_uLong i, j;
            if ( mpFastColorTable )
            {
                for ( i = 0; i < mnWidth; i++ )
                {
                    j = (*pString++) << 8;
                    j += *pString++;
                    sal_uInt8 k = (sal_uInt8)mpFastColorTable[ j ];
                    mpAcc->SetPixel( nY, i, BitmapColor( (sal_uInt8)k ) );

                    if ( mpMaskAcc )
                        mpMaskAcc->SetPixel( nY, i,
                            ( mpColMap[ k * (mnCpp + 4) + mnCpp] ) ? aWhite : aBlack );
                }
            }
            else for ( i = 0; i < mnWidth; i++ )
            {
                pColor = mpColMap;
                for ( j = 0; j < mnColors; j++ )
                {
                    if ( ImplCompare( pString, pColor, mnCpp, XPMCASESENSITIVE ) )
                    {
                        if ( mnColors > 256 )
                            mpAcc->SetPixel( nY, i, Color ( pColor[3], pColor[4], pColor[5] ) );
                        else
                            mpAcc->SetPixel( nY, i, BitmapColor( (sal_uInt8) j ) );

                        if ( mpMaskAcc )
                            mpMaskAcc->SetPixel( nY, i, (
                                pColor[ mnCpp ] ) ? aWhite : aBlack );

                        break;
                    }
                    pColor += ( mnCpp + 4 );
                }
                pString += mnCpp;
            }

        }
    }
    return bStatus;
}






bool XPMReader::ImplGetColSub( sal_uInt8* pDest )
{
    unsigned char cTransparent[] = "None";

    bool bColStatus = false;

    if ( ImplGetColKey( 'c' ) || ImplGetColKey( 'm' ) || ImplGetColKey( 'g' ) )
    {
        
        if ( *mpPara == '#' )
        {
                *pDest++ = 0;
                bColStatus = true;
                switch ( mnParaSize )
                {
                    case 25 :
                        ImplGetRGBHex ( pDest, 6 );
                        break;
                    case 13 :
                        ImplGetRGBHex ( pDest, 2 );
                        break;
                    case  7 :
                        ImplGetRGBHex ( pDest, 0 );
                        break;
                    default:
                        bColStatus = false;
                        break;
                }
        }
        
        else if ( ImplCompare( &cTransparent[0], mpPara, 4 ))
        {
            *pDest++ = 0xff;
            bColStatus = true;
            mbTransparent = true;
        }
        
        else if ( mnParaSize > 2 )  
        {
            sal_uLong i = 0;
            while ( true )
            {
                if ( pRGBTable[ i ].name == NULL )
                    break;
                if ( pRGBTable[ i ].name[ mnParaSize ] == 0 )
                {
                    if ( ImplCompare ( (unsigned char*)pRGBTable[ i ].name,
                            mpPara, mnParaSize, XPMCASENONSENSITIVE ) )
                    {
                        bColStatus = true;
                        *pDest++ = 0;
                        *pDest++ = pRGBTable[ i ].red;
                        *pDest++ = pRGBTable[ i ].green;
                        *pDest++ = pRGBTable[ i ].blue;
                    }
                }
                i++;
            }
        }
    }
    return bColStatus;
}





bool XPMReader::ImplGetColKey( sal_uInt8 nKey )
{
    sal_uInt8 nTemp, nPrev = ' ';

    mpPara = mpStringBuf + mnCpp + 1;
    mnParaSize = 0;

    while ( *mpPara != 0 )
    {
        if ( *mpPara == nKey )
        {
            nTemp = *( mpPara + 1 );
            if ( nTemp == ' ' || nTemp == 0x09 )
            {
                if ( nPrev == ' ' || nPrev == 0x09 )
                    break;
            }
        }
        nPrev = *mpPara;
        mpPara++;
    }
    if ( *mpPara )
    {
        mpPara++;
        while ( (*mpPara == ' ') || (*mpPara == 0x09) )
        {
            mpPara++;
        }
        if ( *mpPara != 0 )
        {
            while ( *(mpPara+mnParaSize) != ' ' && *(mpPara+mnParaSize) != 0x09 &&
                        *(mpPara+mnParaSize) != 0 )
            {
                mnParaSize++;
            }
        }
    }
    return ( mnParaSize ) ? true : false;
}










void XPMReader::ImplGetRGBHex( sal_uInt8* pDest,sal_uLong  nAdd )
{
    sal_uInt8*  pPtr = mpPara+1;
    sal_uInt8   nHex, nTemp;

    for ( sal_uLong i = 0; i < 3; i++ )
    {
        nHex = (*pPtr++) - '0';
        if ( nHex > 9 )
            nHex = ((nHex - 'A' + '0') & 7) + 10;

        nTemp = (*pPtr++) - '0';
        if ( nTemp > 9 )
            nTemp = ((nTemp - 'A' + '0') & 7) + 10;
        nHex = ( nHex << 4 ) + nTemp;

        pPtr += nAdd;
        *pDest++ = (sal_uInt8)nHex;
    }
}




sal_uLong XPMReader::ImplGetULONG( sal_uLong nPara )
{
    if ( ImplGetPara ( nPara ) )
    {
        sal_uLong nRetValue = 0;
        sal_uInt8* pPtr = mpPara;

        if ( ( mnParaSize > 6 ) || ( mnParaSize == 0 ) ) return 0;
        for ( sal_uLong i = 0; i < mnParaSize; i++ )
        {
            sal_uInt8 j = (*pPtr++) - 48;
            if ( j > 9 ) return 0;              
            nRetValue*=10;
            nRetValue+=j;
        }
        return nRetValue;
    }
    else return 0;
}



bool XPMReader::ImplCompare( sal_uInt8* pSource, sal_uInt8* pDest, sal_uLong nSize, sal_uLong nMode )
{
    bool bRet = true;

    if ( nMode == XPMCASENONSENSITIVE )
    {
        for ( sal_uLong i = 0; i < nSize; i++ )
        {
            if ( ( pSource[i]&~0x20 ) != ( pDest[i]&~0x20 ) )
            {
                bRet = false;
                break;
            }
        }
    }
    else
    {
        for ( sal_uLong i = 0; i < nSize; i++ )
        {
            if ( pSource[i] != pDest[i] )
            {
                bRet = false;
                break;
            }
        }
    }
    return bRet;
}







bool XPMReader::ImplGetPara ( sal_uLong nNumb )
{
    sal_uInt8   nByte;
    sal_uLong   pSize = 0;
    sal_uInt8*  pPtr = mpStringBuf;
    sal_uLong   nCount = 0;

    if ( ( *pPtr != ' ' ) && ( *pPtr != 0x09 ) )
    {
        mpPara = pPtr;
        mnParaSize = 0;
        nCount = 0;
    }
    else
    {
        mpPara = NULL;
        nCount = 0xffffffff;
    }

    while ( pSize < mnStringSize )
    {
        nByte = *pPtr;

        if ( mpPara )
        {
            if ( ( nByte == ' ' ) || ( nByte == 0x09 ) )
            {
                if ( nCount == nNumb )
                    break;
                else
                    mpPara = NULL;
            }
            else
                mnParaSize++;
        }
        else
        {
            if ( ( nByte != ' ' ) && ( nByte != 0x09 ) )
            {
                mpPara = pPtr;
                mnParaSize = 1;
                nCount++;
            }
        }
        pSize++;
        pPtr++;
    }
    return ( ( nCount == nNumb ) && ( mpPara ) );
}






bool XPMReader::ImplGetString( void )
{
    sal_uInt8       sID[] = "/* XPM */";
    sal_uInt8*      pString = mpStringBuf;

    mnStringSize = 0;
    mpStringBuf[0] = 0;

    while( mbStatus && ( mnStatus != XPMFINISHED ) )
    {
        if ( mnTempAvail == 0 )
        {
            mnTempAvail = mrIStm.Read( mpTempBuf, XPMTEMPBUFSIZE );
            if ( mnTempAvail == 0 )
                break;

            mpTempPtr = mpTempBuf;

            if ( mnIdentifier == XPMIDENTIFIER )
            {
                if ( mnTempAvail <= 50 )
                {
                    mbStatus = false;   
                    break;
                }
                for ( int i = 0; i < 9; i++ )   
                    if ( *mpTempPtr++ != sID[i] )
                    {
                        mbStatus = false;
                        break;
                    }
                mnTempAvail-=9;
                mnIdentifier++;
            }
        }
        mcLastByte = mcThisByte;
        mcThisByte = *mpTempPtr++;
        mnTempAvail--;

        if ( mnStatus & XPMDOUBLE )
        {
            if ( mcThisByte == 0x0a )
                mnStatus &=~XPMDOUBLE;
            continue;
        }
        if ( mnStatus & XPMREMARK )
        {
            if ( ( mcThisByte == '/' )  && ( mcLastByte == '*' ) )
                mnStatus &=~XPMREMARK;
            continue;
        }
        if ( mnStatus & XPMSTRING )             
        {
            if ( mcThisByte == '"' )
            {
                mnStatus &=~XPMSTRING;          
                break;
            }
            if ( mnStringSize >= ( XPMSTRINGBUF - 1 ) )
            {
                mbStatus = false;
                break;
            }
            *pString++ = mcThisByte;
            pString[0] = 0;
            mnStringSize++;
            continue;
        }
        else
        {                                           
            switch ( mcThisByte )
            {
                case '*' :
                    if ( mcLastByte == '/' ) mnStatus |= XPMREMARK;
                    break;
                case '/' :
                    if ( mcLastByte == '/' ) mnStatus |= XPMDOUBLE;
                    break;
                case '"' : mnStatus |= XPMSTRING;
                    break;
                case '{' :
                    if ( mnIdentifier == XPMDEFINITION )
                        mnIdentifier++;
                    break;
                case '}' :
                    if ( mnIdentifier == XPMENDEXT )
                        mnStatus = XPMFINISHED;
                    break;
            }
        }
    }
    return mbStatus;
}





bool ImportXPM( SvStream& rStm, Graphic& rGraphic )
{
    XPMReader*  pXPMReader = (XPMReader*) rGraphic.GetContext();
    ReadState   eReadState;
    bool        bRet = true;

    if( !pXPMReader )
        pXPMReader = new XPMReader( rStm );

    rGraphic.SetContext( NULL );
    eReadState = pXPMReader->ReadXPM( rGraphic );

    if( eReadState == XPMREAD_ERROR )
    {
        bRet = false;
        delete pXPMReader;
    }
    else if( eReadState == XPMREAD_OK )
        delete pXPMReader;
    else
        rGraphic.SetContext( pXPMReader );

    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
