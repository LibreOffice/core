/*************************************************************************
 *
 *  $RCSfile: cgm.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:30:14 $
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

#ifndef _COM_SUN_STAR_TASK_XSTATUSINDICATOR_HPP_
#include <com/sun/star/task/XStatusIndicator.hpp>
#endif

#define CGM_BREAK_ACTION    0xffffffff

#include <vcl/virdev.hxx>
#include <vcl/graph.hxx>
#include <tools/stream.hxx>
#include <chart.hxx>
#include <main.hxx>
#include <elements.hxx>
#include <outact.hxx>

using namespace ::com::sun::star;

// ---------------------------------------------------------------

void CGM::ImplCGMInit()
{
    mbIsFinished = mbPicture = mbMetaFile = mbPictureBody = sal_False;

    mnActCount = 0;
    mnOutdx = 28000;
    mnOutdy = 21000;

    mpBuf = NULL;
    mpChart = NULL;
    mpBitmapInUse = NULL;

    pCopyOfE = new CGMElements( *this );
    pElement = new CGMElements( *this );
}

// ---------------------------------------------------------------

CGM::CGM( sal_uInt32 nMode ) :
    mnMode          ( nMode ),
    mpGraphic       ( NULL ),       //
    mpCommentOut    ( NULL ),       //
    mbStatus        ( sal_True ),
    mpOutAct        ( new CGMOutAct( *this ) )
{
    ImplCGMInit();
};

// ---------------------------------------------------------------

#ifdef CGM_EXPORT_IMPRESS

CGM::CGM( sal_uInt32 nMode, uno::Reference< frame::XModel > & rModel )  :
    mnMode                  ( nMode ),
    mpGraphic               ( NULL ),
    mpCommentOut            ( NULL ),
    mbStatus                ( sal_True ),
    mpOutAct                ( new CGMImpressOutAct( *this, rModel ) )
{
//  mpCommentOut = new SvFileStream( "d:\\out.txt", STREAM_WRITE | STREAM_TRUNC );
    mnMode |= CGM_EXPORT_IMPRESS;
    ImplCGMInit();
}
#endif

// ---------------------------------------------------------------

#ifdef CGM_EXPORT_META
CGM::CGM( sal_uInt32 nMode, Graphic& rGraphic ) :
    mnMode          ( nMode ),
    mpGraphic       ( &rGraphic ),
    mpCommentOut    ( NULL ),
    mbStatus        ( sal_True ),
    mpGDIMetaFile   ( new GDIMetaFile ),
    mpOutAct        ( new CGMMetaOutAct( *this ) )
{
    ImplCGMInit();
    mpVirDev = new VirtualDevice();
    mpVirDev->EnableOutput( sal_False );
    mpGDIMetaFile->Record( mpVirDev );
};
#endif

// ---------------------------------------------------------------

void CGM::ImplComment( sal_uInt32 Level, char* Description )
{
    if ( mpCommentOut )
    {
        if ( Level == CGM_DESCRIPTION )
        {
            *mpCommentOut << "                                  " << Description << "\n";
        }
        else
        {
            sal_Int8 nFirst, nSecond, i, nCount = 0;
            if ( mnActCount < 10000 )
                nCount++;
            if ( mnActCount < 1000 )
                nCount++;
            if ( mnActCount < 100 )
                nCount++;
            if ( mnActCount < 10 )
                nCount++;
            for ( i = 0; i <= nCount; i++ )
                *mpCommentOut << " ";
            mpCommentOut->WriteNumber( mnActCount );

            switch( Level & 0xff )
            {
                case CGM_UNKNOWN_LEVEL :
                    *mpCommentOut << " L?";
                break;
                case CGM_UNKNOWN_COMMAND :
                    *mpCommentOut << " UNKNOWN COMMAND";
                break;
                case CGM_GDSF_ONLY :
                    *mpCommentOut << " LI";
                break;
                default:
                    *mpCommentOut << " L";
                    mpCommentOut->WriteNumber( Level & 0xff );
                break;
            }
            *mpCommentOut << " C";
            mpCommentOut->WriteNumber( mnElementClass );
            *mpCommentOut << " ID-0x";
            nFirst = ( mnElementID  > 0x9F ) ? (sal_Int8)( mnElementID >> 4 ) + 'A' - 10: (sal_Int8)( mnElementID >> 4 ) + '0';
            nSecond = ( ( mnElementID & 15 ) > 9 ) ? (sal_Int8)( mnElementID & 15 ) + 'A' - 10 : (sal_Int8)( mnElementID & 15 ) + '0';
            *mpCommentOut << nFirst << nSecond;
             *mpCommentOut << " Size";
            nCount = 1;
            if ( mnElementSize < 1000000 )
                nCount++;
            if ( mnElementSize < 100000 )
                nCount++;
            if ( mnElementSize < 10000 )
                nCount++;
            if ( mnElementSize < 1000 )
                nCount++;
            if ( mnElementSize < 100 )
                nCount++;
            if ( mnElementSize < 10 )
                nCount++;
            for ( i = 0; i < nCount; i++ )
                *mpCommentOut << " ";
            mpCommentOut->WriteNumber( mnElementSize );
            *mpCommentOut << " " << Description << "\n";
        }
    }
}

// ---------------------------------------------------------------

CGM::~CGM()
{

#ifdef CGM_EXPORT_META
    if ( mpGraphic )
    {
        mpGDIMetaFile->Stop();
        mpGDIMetaFile->SetPrefMapMode( MapMode() );
        mpGDIMetaFile->SetPrefSize( Size( mnOutdx, mnOutdy ) );
        delete mpVirDev;
        *mpGraphic = Graphic( *mpGDIMetaFile );
    }
#endif
    sal_Int8* pBuf = (sal_Int8*)maDefRepList.First();
    while( pBuf )
    {
        delete pBuf;
        pBuf = (sal_Int8*)maDefRepList.Next();
    }
    maDefRepList.Clear();
    delete mpBitmapInUse;
    delete mpCommentOut;
    delete mpChart;
    delete mpOutAct;
    delete pCopyOfE;
    delete pElement;
    delete mpBuf;
};

// ---------------------------------------------------------------

sal_uInt32 CGM::GetBackGroundColor()
{
    return ( pElement ) ? pElement->aColorTable[ 0 ] : 0;
}

// ---------------------------------------------------------------

sal_uInt32 CGM::ImplGetUI16( sal_uInt32 nAlign )
{
    sal_uInt8* pSource = mpSource + mnParaSize;
    mnParaSize += 2;
    if ( nAlign && ( mnMode & CGM_IMPORT_IM ) )
    {
        nAlign--;
        mnParaSize += nAlign;
        mnParaSize &=~nAlign;
    }
    if ( mnMode & CGM_BIG_ENDIAN )
        return ( pSource[ 0 ] << 8 ) +  pSource[ 1 ];
    else
        return ( pSource[ 1 ] << 8 ) +  pSource[ 0 ];
};

// ---------------------------------------------------------------

sal_uInt8 CGM::ImplGetByte( sal_uInt32 nSource, sal_uInt32 nPrecision )
{
    return (sal_uInt8)( nSource >> ( ( nPrecision - 1 ) << 3 ) );
};

// ---------------------------------------------------------------

long CGM::ImplGetI( sal_uInt32 nPrecision )
{
    sal_uInt8* pSource = mpSource + mnParaSize;
    mnParaSize += nPrecision;
    switch( nPrecision )
    {
        case 1 :
        {
            return  (char)*pSource;
        }

        case 2 :
        {
            if ( mnMode & CGM_BIG_ENDIAN )
                return (sal_Int16)( ( pSource[ 0 ] << 8 ) | pSource[ 1 ] );
            else
                return (sal_Int16)( ( pSource[ 1 ] << 8 ) | pSource[ 0 ] );
        }

        case 3 :
        {
            if ( mnMode & CGM_BIG_ENDIAN )
                return ( ( pSource[ 0 ] << 24 ) | ( pSource[ 1 ] << 16 ) | pSource[ 2 ] << 8 ) >> 8;
            else
                return ( ( pSource[ 2 ] << 24 ) | ( pSource[ 1 ] << 16 ) | pSource[ 0 ] << 8 ) >> 8;
        }
        case 4:
        {
            if ( mnMode & CGM_BIG_ENDIAN )
                return (sal_Int32)( ( pSource[ 0 ] << 24 ) | ( pSource[ 1 ] << 16 ) | ( pSource[ 2 ] << 8 ) | ( pSource[ 3 ] ) );
            else
                return (sal_Int32)( ( pSource[ 3 ] << 24 ) | ( pSource[ 2 ] << 16 ) | ( pSource[ 1 ] << 8 ) | ( pSource[ 0 ] ) );
        }
        default:
            mbStatus = sal_False;
            return 0;
    }
}

// ---------------------------------------------------------------

sal_uInt32 CGM::ImplGetUI( sal_uInt32 nPrecision )
{
    sal_uInt8* pSource = mpSource + mnParaSize;
    mnParaSize += nPrecision;
    switch( nPrecision )
    {
        case 1 :
            return  (sal_Int8)*pSource;
        case 2 :
        {
            if ( mnMode & CGM_BIG_ENDIAN )
                return (sal_uInt16)( ( pSource[ 0 ] << 8 ) | pSource[ 1 ] );
            else
                return (sal_uInt16)( ( pSource[ 1 ] << 8 ) | pSource[ 0 ] );
        }
        case 3 :
        {
            if ( mnMode & CGM_BIG_ENDIAN )
                return ( pSource[ 0 ] << 16 ) | ( pSource[ 1 ] << 8 ) | pSource[ 2 ];
            else
                return ( pSource[ 2 ] << 16 ) | ( pSource[ 1 ] << 8 ) | pSource[ 0 ];
        }
        case 4:
        {
            if ( mnMode & CGM_BIG_ENDIAN )
                return (sal_uInt32)( ( pSource[ 0 ] << 24 ) | ( pSource[ 1 ] << 16 ) | ( pSource[ 2 ] << 8 ) | ( pSource[ 3 ] ) );
            else
                return (sal_uInt32)( ( pSource[ 3 ] << 24 ) | ( pSource[ 2 ] << 16 ) | ( pSource[ 1 ] << 8 ) | ( pSource[ 0 ] ) );
        }
        default:
            mbStatus = sal_False;
            return 0;
    }
}

// ---------------------------------------------------------------

void CGM::ImplGetSwitch4( sal_uInt8* pSource, sal_uInt8* pDest )
{
    for ( int i = 0; i < 4; i++ )
    {
        pDest[ i ] = pSource[ i ^ 3 ];          // Little Endian <-> Big Endian switch
    }
}

// ---------------------------------------------------------------

void CGM::ImplGetSwitch8( sal_uInt8* pSource, sal_uInt8* pDest )
{
    for ( int i = 0; i < 8; i++ )
    {
        pDest[ i ] = pSource[ i ^ 7 ];          // Little Endian <-> Big Endian switch
    }
}

// ---------------------------------------------------------------

double CGM::ImplGetFloat( RealPrecision eRealPrecision, sal_uInt32 nRealSize )
{
    void*   pPtr;
    sal_uInt8   aBuf[8];
    sal_Bool    bCompatible;
    double  nRetValue;
    double  fDoubleBuf;
    float   fFloatBuf;

#ifdef __BIGENDIAN
    if ( mnMode & CGM_BIG_ENDIAN )
        bCompatible = sal_True;
    else
        bCompatible = sal_False;
#else
    if ( mnMode & CGM_LITTLE_ENDIAN )
        bCompatible = sal_True;
    else
        bCompatible = sal_False;
#endif
    if ( bCompatible )
        pPtr = mpSource + mnParaSize;
    else
    {
        if ( nRealSize == 4 )
            ImplGetSwitch4( mpSource + mnParaSize, &aBuf[0] );
        else
            ImplGetSwitch8( mpSource + mnParaSize, &aBuf[0] );
        pPtr = &aBuf;
    }
    if ( eRealPrecision == RP_FLOAT )
    {
        if ( nRealSize == 4 )
        {
            memcpy( (void*)&fFloatBuf, pPtr, 4 );
            nRetValue = (double)fFloatBuf;
        }
        else
        {
            memcpy( (void*)&fDoubleBuf, pPtr, 8 );
            nRetValue = fDoubleBuf;
        }
    }
    else // ->RP_FIXED
    {
        long    nVal;
        int     nSwitch = ( bCompatible ) ? 0 : 1 ;
        if ( nRealSize == 4 )
        {
            sal_uInt16* pShort = (sal_uInt16*)pPtr;
            nVal = pShort[ nSwitch ];
            nVal <<= 16;
            nVal |= pShort[ nSwitch ^ 1 ];
            nRetValue = (double)nVal;
            nRetValue /= 65536;
        }
        else
        {
            long* pLong = (long*)pPtr;
            nRetValue = (double)abs( pLong[ nSwitch ] );
            nRetValue *= 65536;
            nVal = (sal_uInt32)( pLong[ nSwitch ^ 1 ] );
            nVal >>= 16;
            nRetValue += (double)nVal;
            if ( pLong[ nSwitch ] < 0 )
            {
                nRetValue -= nRetValue;
            }
            nRetValue /= 65536;
        }
    }
    mnParaSize += nRealSize;
    return nRetValue;
}

// ---------------------------------------------------------------

sal_uInt32 CGM::ImplGetPointSize()
{
    if ( pElement->eVDCType == VDC_INTEGER )
        return pElement->nVDCIntegerPrecision << 1;
    else
    {
        if ( mnMode & CGM_IMPORT_IM )
            return pElement->nVDCRealSize * 3;
        else
            return pElement->nVDCRealSize << 1;
    }
}

// ---------------------------------------------------------------

inline double CGM::ImplGetIX()
{
    return ( ( ImplGetI( pElement->nVDCIntegerPrecision ) + mnVDCXadd ) * mnVDCXmul );
}

// ---------------------------------------------------------------

inline double CGM::ImplGetFX()
{
    return ( ( ImplGetFloat( pElement->eVDCRealPrecision, pElement->nVDCRealSize ) + mnVDCXadd ) * mnVDCXmul );
}

// ---------------------------------------------------------------

inline double CGM::ImplGetIY()
{
    return ( ( ImplGetI( pElement->nVDCIntegerPrecision ) + mnVDCYadd ) * mnVDCYmul );
}

// ---------------------------------------------------------------

inline double CGM::ImplGetFY()
{
    return ( ( ImplGetFloat( pElement->eVDCRealPrecision, pElement->nVDCRealSize ) + mnVDCYadd ) * mnVDCYmul );
}

// ---------------------------------------------------------------

void CGM::ImplGetPoint( FloatPoint& rFloatPoint, sal_Bool bMap )
{
    if ( pElement->eVDCType == VDC_INTEGER )
    {
        rFloatPoint.X = ImplGetIX();
        rFloatPoint.Y = ImplGetIY();
    }
    else // ->floating points
    {
        rFloatPoint.X = ImplGetFX();
        rFloatPoint.Y = ImplGetFY();
        if ( mnMode & CGM_IMPORT_IM )
            mnParaSize += 8;
    }
    if ( bMap )
        ImplMapPoint( rFloatPoint );
}

// ---------------------------------------------------------------

void CGM::ImplGetRectangle( FloatRect& rFloatRect, sal_Bool bMap )
{
    if ( pElement->eVDCType == VDC_INTEGER )
    {
        rFloatRect.Left = ImplGetIX();
        rFloatRect.Bottom = ImplGetIY();
        rFloatRect.Right = ImplGetIX();
        rFloatRect.Top = ImplGetIY();
    }
    else // ->floating points
    {
        rFloatRect.Left = ImplGetFX();
        rFloatRect.Bottom = ImplGetFY();
        if ( mnMode & CGM_IMPORT_IM )
            mnParaSize += 8;
        rFloatRect.Right = ImplGetFX();
        rFloatRect.Top = ImplGetFY();
        if ( mnMode & CGM_IMPORT_IM )
            mnParaSize += 8;
    }
    if ( bMap )
    {
        ImplMapX( rFloatRect.Left );
        ImplMapX( rFloatRect.Right );
        ImplMapY( rFloatRect.Top );
        ImplMapY( rFloatRect.Bottom );
        rFloatRect.Justify();
    }
}

// ---------------------------------------------------------------

void CGM::ImplGetRectangleNS( FloatRect& rFloatRect )
{
    if ( pElement->eVDCType == VDC_INTEGER )
    {
        rFloatRect.Left = ImplGetI( pElement->nVDCIntegerPrecision );
        rFloatRect.Bottom = ImplGetI( pElement->nVDCIntegerPrecision );
        rFloatRect.Right = ImplGetI( pElement->nVDCIntegerPrecision );
        rFloatRect.Top = ImplGetI( pElement->nVDCIntegerPrecision );
    }
    else // ->floating points
    {
        rFloatRect.Left = ImplGetFloat( pElement->eVDCRealPrecision, pElement->nVDCRealSize );
        rFloatRect.Bottom = ImplGetFloat( pElement->eVDCRealPrecision, pElement->nVDCRealSize );
        if ( mnMode & CGM_IMPORT_IM )
            mnParaSize += 8;
        rFloatRect.Right = ImplGetFloat( pElement->eVDCRealPrecision, pElement->nVDCRealSize );
        rFloatRect.Top = ImplGetFloat( pElement->eVDCRealPrecision, pElement->nVDCRealSize );
        if ( mnMode & CGM_IMPORT_IM )
            mnParaSize += 8;
    }
}

// ---------------------------------------------------------------

sal_uInt32 CGM::ImplGetBitmapColor( sal_Bool bDirect )
{
    // the background color is always a direct color

    sal_uInt32  nTmp;
    if ( ( pElement->eColorSelectionMode == CSM_DIRECT ) || bDirect )
    {
        sal_uInt32      nColor = ImplGetByte( ImplGetUI( pElement->nColorPrecision ), 1 );
        sal_uInt32      nDiff = pElement->nColorValueExtent[ 3 ] - pElement->nColorValueExtent[ 0 ] + 1;

        if ( !nDiff )
            nDiff++;
        nColor = ( ( nColor - pElement->nColorValueExtent[ 0 ] ) << 8 ) / nDiff;
        nTmp = nColor << 16 & 0xff0000;

        nColor = ImplGetByte( ImplGetUI( pElement->nColorPrecision ), 1 );
        nDiff = pElement->nColorValueExtent[ 4 ] - pElement->nColorValueExtent[ 1 ] + 1;
        if ( !nDiff )
            nDiff++;
        nColor = ( ( nColor - pElement->nColorValueExtent[ 1 ] ) << 8 ) / nDiff;
        nTmp |= nColor << 8 & 0xff00;

        nColor = ImplGetByte( ImplGetUI( pElement->nColorPrecision ), 1 );
        nDiff = pElement->nColorValueExtent[ 5 ] - pElement->nColorValueExtent[ 2 ] + 1;
        if ( !nDiff )
            nDiff++;
        nColor = ( ( nColor - pElement->nColorValueExtent[ 2 ] ) << 8 ) / nDiff;
        nTmp |= (sal_Int8)nColor;
    }
    else
    {
        sal_uInt32 nIndex = ImplGetUI( pElement->nColorIndexPrecision );
        nTmp = pElement->aColorTable[ (sal_Int8)( nIndex ) ] ;
    }
    return nTmp;
}

// ---------------------------------------------------------------

// call this function each time after the mapmode settings has been changed
void CGM::ImplSetMapMode()
{
    int nAngReverse = 1;
    mnVDCdx = pElement->aVDCExtent.Right - pElement->aVDCExtent.Left;

    mnVDCXadd = -pElement->aVDCExtent.Left;
    mnVDCXmul = 1;
    if ( mnVDCdx < 0 )
    {
        nAngReverse ^= 1;
        mnVDCdx = -mnVDCdx;
        mnVDCXmul = -1;
    }

    mnVDCdy = pElement->aVDCExtent.Bottom - pElement->aVDCExtent.Top;
    mnVDCYadd = -pElement->aVDCExtent.Top;
    mnVDCYmul = 1;
    if ( mnVDCdy < 0 )
    {
        nAngReverse ^= 1;
        mnVDCdy = -mnVDCdy;
        mnVDCYmul = -1;
    }
    if ( nAngReverse )
        mbAngReverse = sal_True;
    else
        mbAngReverse = sal_False;

    double fQuo1 = mnVDCdx / mnVDCdy;
    double fQuo2 = mnOutdx / mnOutdy;
    if ( fQuo2 < fQuo1 )
    {
        mnXFraction = mnOutdx / mnVDCdx;
        mnYFraction = mnOutdy * ( fQuo2 / fQuo1 ) / mnVDCdy;
    }
    else
    {
        mnXFraction = mnOutdx * ( fQuo1 / fQuo2 ) / mnVDCdx;
        mnYFraction = mnOutdy / mnVDCdy;
    }
}

// ---------------------------------------------------------------

void CGM::ImplMapDouble( double& nNumb )
{
    if ( pElement->eDeviceViewPortMap == DVPM_FORCED )
    {
        // point is 1mm * ScalingFactor
        switch ( pElement->eDeviceViewPortMode )
        {
            case DVPM_FRACTION :
            {
                nNumb *= ( mnXFraction + mnYFraction ) / 2;
            }
            break;

            case DVPM_METRIC :
            {
//              nNumb *= ( 100 * pElement->nDeviceViewPortScale );
                nNumb *= ( mnXFraction + mnYFraction ) / 2;
                if ( pElement->nDeviceViewPortScale < 0 )
                    nNumb = -nNumb;
            }
            break;

            case DVPM_DEVICE :
            {

            }
            break;

            default:

                break;
        }
    }
    else
    {


    }
}

// ---------------------------------------------------------------

void CGM::ImplMapX( double& nNumb )
{
    if ( pElement->eDeviceViewPortMap == DVPM_FORCED )
    {
        // point is 1mm * ScalingFactor
        switch ( pElement->eDeviceViewPortMode )
        {
            case DVPM_FRACTION :
            {
                nNumb *= mnXFraction;
            }
            break;

            case DVPM_METRIC :
            {
//              nNumb *= ( 100 * pElement->nDeviceViewPortScale );
                nNumb *= mnXFraction;
                if ( pElement->nDeviceViewPortScale < 0 )
                    nNumb = -nNumb;
            }
            break;

            case DVPM_DEVICE :
            {

            }
            break;

            default:

                break;
        }
    }
    else
    {


    }
}


// ---------------------------------------------------------------

void CGM::ImplMapY( double& nNumb )
{
    if ( pElement->eDeviceViewPortMap == DVPM_FORCED )
    {
        // point is 1mm * ScalingFactor
        switch ( pElement->eDeviceViewPortMode )
        {
            case DVPM_FRACTION :
            {
                nNumb *= mnYFraction;
            }
            break;

            case DVPM_METRIC :
            {
//              nNumb *= ( 100 * pElement->nDeviceViewPortScale );
                nNumb *= mnYFraction;
                if ( pElement->nDeviceViewPortScale < 0 )
                    nNumb = -nNumb;
            }
            break;

            case DVPM_DEVICE :
            {

            }
            break;

            default:

                break;
        }
    }
    else
    {


    }
}


// ---------------------------------------------------------------

// convert a point to the current VC mapmode (1/100TH mm)
void CGM::ImplMapPoint( FloatPoint& rFloatPoint )
{
    if ( pElement->eDeviceViewPortMap == DVPM_FORCED )
    {
        // point is 1mm * ScalingFactor
        switch ( pElement->eDeviceViewPortMode )
        {
            case DVPM_FRACTION :
            {
                rFloatPoint.X *= mnXFraction;
                rFloatPoint.Y *= mnYFraction;
            }
            break;

            case DVPM_METRIC :
            {
                rFloatPoint.X *= mnXFraction;
                rFloatPoint.Y *= mnYFraction;
                if ( pElement->nDeviceViewPortScale < 0 )
                {
                    rFloatPoint.X = -rFloatPoint.X;
                    rFloatPoint.Y = -rFloatPoint.Y;
                }
            }
            break;

            case DVPM_DEVICE :
            {

            }
            break;

            default:

                break;
        }
    }
    else
    {


    }
}

// ---------------------------------------------------------------

void CGM::ImplDoClass()
{
#ifdef CGM_USER_BREAKPOINT
#ifdef WNT
    if ( mnActCount == CGM_BREAK_ACTION )
        _asm int 0x3;
#endif
#endif
    switch ( mnElementClass )
    {
        case 0 : ImplDoClass0(); break;
        case 1 : ImplDoClass1(); break;
        case 2 : ImplDoClass2(); break;
        case 3 : ImplDoClass3(); break;
        case 4 :
        {
            ImplDoClass4();
            mnAct4PostReset = 0;
        }
        break;
        case 5 : ImplDoClass5(); break;
        case 6 : ImplDoClass6(); break;
        case 7 : ImplDoClass7(); break;
        case 8 : ImplDoClass8(); break;
        case 9 : ImplDoClass9(); break;
        case 15 :ImplDoClass15(); break;
        default : ComOut( CGM_UNKNOWN_COMMAND, "" ); break;
    }
    mnActCount++;
};

// ---------------------------------------------------------------

void CGM::ImplDefaultReplacement()
{
    sal_uInt8*  pBuf = (sal_uInt8*)maDefRepList.First();
    if ( pBuf )
    {
        sal_uInt32  nElementSize = (sal_uInt32)maDefRepSizeList.First();
        sal_uInt32  nOldEscape = mnEscape;
        sal_uInt32  nOldElementClass = mnElementClass;
        sal_uInt32  nOldElementID = mnElementID;
        sal_uInt32  nOldElementSize = mnElementSize;
        sal_uInt8*  pOldBuf = mpSource;
        while( pBuf )
        {
            sal_uInt32  nCount = 0;
            while ( mbStatus && ( nCount < nElementSize ) )
            {
                mpSource = pBuf + nCount;
                mnParaSize = 0;
                mnEscape = ImplGetUI16();
                mnElementClass = mnEscape >> 12;
                mnElementID = ( mnEscape & 0x0fe0 ) >> 5;
                mnElementSize = mnEscape & 0x1f;
                if ( mnElementSize == 31 )
                {
                    mnElementSize = ImplGetUI16();
                }
                nCount += mnParaSize;
                mnParaSize = 0;
                mpSource = pBuf + nCount;
                if ( (!( mnMode & CGM_NO_PAD_BYTE ) ) && ( mnElementSize & 1 ) )
                    nCount++;
                nCount += mnElementSize;
                if ( ( mnElementClass != 1 ) || ( mnElementID != 0xc ) )    // rekursion hier nicht moeglich!!
                    ImplDoClass();
            }
            nElementSize = (sal_uInt32)maDefRepSizeList.Next();
            pBuf = (sal_uInt8*)maDefRepList.Next();
        }
        mnEscape = nOldEscape;
        mnElementClass = nOldElementClass;
        mnElementID = nOldElementID;
        mnParaSize = mnElementSize = nOldElementSize;
        mpSource = pOldBuf;
    }
}

// ---------------------------------------------------------------

sal_Bool CGM::Write( SvStream& rIStm )
{
    if ( !mpBuf )
        mpBuf = new sal_uInt8[ 0xffff ];

    mnParaSize = 0;
    mpSource = mpBuf;
    rIStm.Read( mpSource, 2 );
    mnEscape = ImplGetUI16();
    mnElementClass = mnEscape >> 12;
    mnElementID = ( mnEscape & 0x0fe0 ) >> 5;
    mnElementSize = mnEscape & 0x1f;

    if ( mnElementSize == 31 )
    {
        rIStm.Read( mpSource + mnParaSize, 2 );
        mnElementSize = ImplGetUI16();
    }
    mnParaSize = 0;
    if ( mnElementSize )
        rIStm.Read( mpSource + mnParaSize, mnElementSize );

    if ( (!( mnMode & CGM_NO_PAD_BYTE ) ) && ( mnElementSize & 1 ) )
        rIStm.SeekRel( 1 );
    ImplDoClass();


#ifdef CGM_USER_BREAKPOINT
#ifdef WNT
    if ( !mbStatus || mnParaSize && ( mnElementSize != mnParaSize ) )
        _asm int 0x3;
#endif
#endif

    return mbStatus;
};

// ---------------------------------------------------------------

sal_Bool CGM::Write( sal_uInt8* pSource )
{
    if ( mnMode & CGM_IMPORT_IM )
    {
        mnElementID = pSource[ 0 ];
        mnElementClass = pSource[ 1 ];
        mnElementSize = *( (long*)pSource + 1  );
        mpSource = pSource + 16;
        mnParaSize = 0;
        ImplDoClass();
    }
    else
    {
        mpSource = pSource;
        mnEscape = ImplGetUI16();
        mnElementClass = mnEscape >> 12;
        mnElementID = ( mnEscape & 0x0fe0 ) >> 5;
        mnElementSize = mnEscape & 0x1f;
        if ( mnElementSize == 31 )
            mnElementSize = ImplGetUI16();
        mpSource += mnParaSize;
        mnParaSize = 0;
        ImplDoClass();
    }

#ifdef CGM_USER_BREAKPOINT
#ifdef WNT
    if ( !mbStatus || mnParaSize && ( mnElementSize != mnParaSize ) )
        _asm int 0x3;
#endif
#endif

    return mbStatus;
};

// ---------------------------------------------------------------

SvStream& operator>>( SvStream& rOStm, CGM& rCGM )
{

    return rOStm;
};

// ---------------------------------------------------------------



//================== GraphicImport - die exportierte Funktion ================

extern "C" sal_uInt32 __LOADONCALLAPI ImportCGM( String& rFileName, uno::Reference< frame::XModel > & rXModel, sal_uInt32 nMode, void* pProgressBar )
{
    sal_Bool bProgressBar = sal_False;

    CGM*    pCGM;                       // retvalue == 0 -> ERROR
    sal_uInt32  nStatus = 0;            //          == 0xffrrggbb -> background color in the lower 24 bits

    if( rXModel.is() )
    {
        pCGM = new CGM( nMode, rXModel );
        if ( pCGM && pCGM->IsValid() )
        {
            if ( nMode & CGM_IMPORT_CGM )
            {
                SvFileStream    aIn( rFileName, STREAM_READ );
                aIn.SetNumberFormatInt( NUMBERFORMAT_INT_BIGENDIAN );
                aIn.Seek( STREAM_SEEK_TO_END );
                sal_uInt32  nInSize = aIn.Tell();
                aIn.Seek( 0 );

#if defined CGM_EXPORT_IMPRESS && TF_ONE51
                uno::Reference< task::XStatusIndicator >  aXStatInd;
                sal_uInt32  nNext = 0;
                sal_uInt32  nAdd = nInSize / 20;
                if ( pProgressBar )
                    aXStatInd = *(uno::Reference< task::XStatusIndicator > *)pProgressBar;
                bProgressBar = aXStatInd.is();
                if ( bProgressBar )
                    aXStatInd->start( rtl::OUString::createFromAscii("CGM Import"), nInSize );
#endif

                while ( pCGM->IsValid() && ( aIn.Tell() < nInSize ) && !pCGM->IsFinished() )
                {

#if defined CGM_EXPORT_IMPRESS && defined TF_ONE51


                    if ( bProgressBar )
                    {
                        sal_uInt32 nCurrentPos = aIn.Tell();
                        if ( nCurrentPos >= nNext )
                        {
                            aXStatInd->setValue( nCurrentPos );
                            nNext = nCurrentPos + nAdd;
                        }
                    }
#endif

                    if ( pCGM->Write( aIn ) == sal_False )
                        break;
                }
                if ( pCGM->IsValid() )
                {
                    nStatus = pCGM->GetBackGroundColor() | 0xff000000;
                }

#if defined CGM_EXPORT_IMPRESS && defined TF_ONE51
    if ( bProgressBar )
        aXStatInd->end();
#endif

            }
        }
        delete pCGM;
    }
    return nStatus;
}
