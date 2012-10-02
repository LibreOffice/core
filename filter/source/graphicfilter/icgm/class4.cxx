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


#include <main.hxx>
#include <chart.hxx>
#include <outact.hxx>
#include <math.h>

using namespace ::com::sun::star;

double CGM::ImplGetOrientation( FloatPoint& rCenter, FloatPoint& rPoint )
{
    double fOrientation;

    double nX = rPoint.X - rCenter.X;
    double nY = rPoint.Y - rCenter.Y;

    fOrientation = acos( nX / sqrt( nX * nX + nY * nY ) ) * 57.29577951308;
    if ( nY > 0 )
        fOrientation = 360 - fOrientation;

    return fOrientation;
}

// ---------------------------------------------------------------

void CGM::ImplSwitchStartEndAngle( double& rStartAngle, double& rEndAngle )
{
    double nTemp;
    nTemp = rStartAngle;
    rStartAngle = rEndAngle;
    rEndAngle = nTemp;
}

// ---------------------------------------------------------------

void CGM::ImplGetVector( double* pVector )
{
    if ( pElement->eVDCType == VDC_REAL )
    {
        for ( sal_uInt32 i = 0; i < 4; i++ )
        {
            pVector[ i ] = (double)ImplGetFloat( pElement->eVDCRealPrecision, pElement->nVDCRealSize );
        }
    }
    else
    {
        for ( sal_uInt32 i = 0; i < 4; i++ )
        {
            pVector[ i ] = (double)ImplGetI( pElement->nVDCIntegerPrecision );
        }
    }
    pVector[ 0 ] *= mnVDCXmul;
    pVector[ 2 ] *= mnVDCXmul;
    pVector[ 1 ] *= mnVDCYmul;
    pVector[ 3 ] *= mnVDCYmul;
}

// ---------------------------------------------------------------
sal_Bool CGM::ImplGetEllipse( FloatPoint& rCenter, FloatPoint& rRadius, double& rAngle )
{
    FloatPoint  aPoint1, aPoint2;
    double      fRot1, fRot2;
    ImplGetPoint( rCenter, sal_True );
    ImplGetPoint( aPoint1, sal_True );
    ImplGetPoint( aPoint2, sal_True );
    fRot1 = ImplGetOrientation( rCenter, aPoint1 );
    fRot2 = ImplGetOrientation( rCenter, aPoint2 );
    rAngle = ImplGetOrientation( rCenter, aPoint1 );
    aPoint1.X -= rCenter.X;
    aPoint1.Y -= rCenter.Y;
    rRadius.X = sqrt( aPoint1.X * aPoint1.X + aPoint1.Y * aPoint1.Y );
    aPoint2.X -= rCenter.X;
    aPoint2.Y -= rCenter.Y;
    rRadius.Y = sqrt( aPoint2.X * aPoint2.X + aPoint2.Y * aPoint2.Y );

    if ( fRot1 > fRot2 )
    {
        if ( ( fRot1 - fRot2 ) < 180 )
            return sal_False;
    }
    else
    {
        if ( ( fRot2 - fRot1 ) > 180 )
            return sal_False;
    }
    return sal_True;
}

void CGM::ImplDoClass4()
{
    if ( mbFirstOutPut )
        mpOutAct->FirstOutPut();

    if ( mpBitmapInUse && ( mnElementID != 9 ) )    // vorhandene grafik verarbeiten,
    {                                               // da jetzt nicht bitmap actions anstehen
        CGMBitmapDescriptor* pBmpDesc = mpBitmapInUse->GetBitmap();
        // irgendetwas mit der Bitmap anfangen
        mpOutAct->DrawBitmap( pBmpDesc );
        delete mpBitmapInUse;
        mpBitmapInUse = NULL;
    }

    if ( ( mpChart == NULL ) || mpChart->IsAnnotation() )
    {
        switch ( mnElementID )
        {
            case 0x01 : /*PolyLine*/
            {
                sal_uInt32 nPoints = mnElementSize / ImplGetPointSize();
                Polygon aPolygon( (sal_uInt16)nPoints );
                for ( sal_uInt16 i = 0; i < nPoints; i++)
                {
                    FloatPoint  aFloatPoint;
                    ImplGetPoint( aFloatPoint, sal_True );
                    aPolygon.SetPoint( Point( (long)aFloatPoint.X, (long)aFloatPoint.Y ), i );
                }
                if ( mbFigure )
                    mpOutAct->RegPolyLine( aPolygon );
                else
                    mpOutAct->DrawPolyLine( aPolygon );
            }
            break;

            case 0x02 : /*Disjoint PolyLine*/
            {
                sal_uInt16 nPoints = sal::static_int_cast< sal_uInt16 >(
                    mnElementSize / ImplGetPointSize());
                if ( ! ( nPoints & 1 ) )
                {
                    nPoints >>= 1;
                    FloatPoint  aFloatPoint;
                    if ( mbFigure )
                    {
                        Polygon aPolygon( nPoints );
                        for ( sal_uInt16 i = 0; i < nPoints; i++ )
                        {
                            ImplGetPoint( aFloatPoint, sal_True );
                            aPolygon.SetPoint( Point( (long)aFloatPoint.X, (long)aFloatPoint.Y ), 0 );
                        }
                        mpOutAct->RegPolyLine( aPolygon );
                    }
                    else
                    {
                        mpOutAct->BeginGroup();
                        Polygon aPolygon( (sal_uInt16)2 );
                        for ( sal_uInt16 i = 0; i < nPoints; i++ )
                        {
                            ImplGetPoint( aFloatPoint, sal_True );
                            aPolygon.SetPoint( Point( (long)aFloatPoint.X, (long)aFloatPoint.Y ), 0 );
                            ImplGetPoint( aFloatPoint, sal_True );
                            aPolygon.SetPoint( Point( (long)aFloatPoint.X, (long)aFloatPoint.Y ), 1);
                            mpOutAct->DrawPolyLine( aPolygon );
                        }
                        mpOutAct->EndGroup();
                    }
                }
            }
            break;

            case 0x03 : /*PolyMarker*/ break;
            case 0x04 : /*Text*/
            {
                FloatPoint  aFloatPoint;
                sal_uInt32      nType, nSize;

                if ( mbFigure )
                    mpOutAct->CloseRegion();

                ImplGetPoint ( aFloatPoint, sal_True );
                nType = ImplGetUI16( 4 );
                nSize = ImplGetUI( 1 );
                mpSource[ mnParaSize + nSize ] = 0;

                awt::Size aSize;
                awt::Point aPoint( (long)aFloatPoint.X, (long)aFloatPoint.Y );
                mpOutAct->DrawText( aPoint, aSize,
                                (char*)mpSource + mnParaSize, nSize, (FinalFlag)nType );
                mnParaSize = mnElementSize;
            }
            break;

            case 0x05 : /*Restricted Text*/
            {
                double      dx, dy;
                FloatPoint  aFloatPoint;
                sal_uInt32      nType, nSize;

                if ( mbFigure )
                    mpOutAct->CloseRegion();

                if ( pElement->eVDCType == VDC_REAL )
                {
                    dx = ImplGetFloat( pElement->eVDCRealPrecision, pElement->nVDCRealSize );
                    dy = ImplGetFloat( pElement->eVDCRealPrecision, pElement->nVDCRealSize );
                }
                else
                {
                    dx = (double)ImplGetI( pElement->nVDCIntegerPrecision );
                    dy = (double)ImplGetI( pElement->nVDCIntegerPrecision );
                }
                ImplMapDouble( dx );
                ImplMapDouble( dy );

                ImplGetPoint ( aFloatPoint, sal_True );
                nType = ImplGetUI16( 4 );
                nSize = ImplGetUI( 1 );

                mpSource[ mnParaSize + nSize ] = 0;

                awt::Point aPoint( (long)aFloatPoint.X, (long)aFloatPoint.Y );
                awt::Size aSize((long)dx, (long)dy);
                mpOutAct->DrawText( aPoint, aSize ,
                                (char*)mpSource + mnParaSize, nSize, (FinalFlag)nType );
                mnParaSize = mnElementSize;
            }
            break;

            case 0x06 : /*Append Text*/
            {
                sal_uInt32 nSize;
                sal_uInt32 nType = ImplGetUI16( 4 );

                nSize = ImplGetUI( 1 );
                mpSource[ mnParaSize + nSize ] = 0;

                mpOutAct->AppendText( (char*)mpSource + mnParaSize, nSize, (FinalFlag)nType );
                mnParaSize = mnElementSize;
            }
            break;

            case 0x07 : /*Polygon*/
            {
                if ( mbFigure )
                    mpOutAct->CloseRegion();

                sal_uInt16 nPoints = sal::static_int_cast< sal_uInt16 >(
                    mnElementSize / ImplGetPointSize());
                Polygon aPolygon( nPoints );
                for ( sal_uInt16 i = 0; i < nPoints; i++)
                {
                    FloatPoint  aFloatPoint;
                    ImplGetPoint( aFloatPoint, sal_True );
                    aPolygon.SetPoint( Point ( (long)( aFloatPoint.X ), (long)( aFloatPoint.Y ) ), i );
                }
                mpOutAct->DrawPolygon( aPolygon );
            }
            break;

            case 0x08 : /*Polygon Set*/
            {
                if ( mbFigure )
                    mpOutAct->CloseRegion();

                sal_uInt16      nPoints = 0;
                Point*      pPoints = new Point[ 0x4000 ];

                PolyPolygon aPolyPolygon;
                FloatPoint  aFloatPoint;
                sal_uInt32      nEdgeFlag;
                while ( mnParaSize < mnElementSize )
                {
                    ImplGetPoint( aFloatPoint, sal_True );
                    nEdgeFlag = ImplGetUI16();
                    pPoints[ nPoints++ ] = Point( (long)aFloatPoint.X, (long)aFloatPoint.Y );
                    if ( ( nEdgeFlag & 2 ) || ( mnParaSize == mnElementSize ) )
                    {
                        Polygon aPolygon( nPoints );
                        for ( sal_uInt16 i = 0; i < nPoints; i++ )
                        {
                            aPolygon.SetPoint( pPoints[ i ], i );
                        }
                        aPolyPolygon.Insert( aPolygon, POLYPOLY_APPEND );
                        nPoints = 0;
                    }
                }
                delete[] pPoints;
                mpOutAct->DrawPolyPolygon( aPolyPolygon );
            }
            break;

            case 0x09 : /*Cell Array*/
            {
                if ( mbFigure )
                    mpOutAct->CloseRegion();

                if ( mpBitmapInUse )
                {
                    CGMBitmap* pBmpDesc = mpBitmapInUse->GetNext();
                    if ( pBmpDesc ) // eventuell bekommen wir eine bitmap zurück, die nicht
                    {               // zur vorherigen paßt -> diese müssen wir dann auch löschen
                        mpOutAct->DrawBitmap( pBmpDesc->GetBitmap() );
                        delete pBmpDesc;
                    }
                }
                else
                {
                    mpBitmapInUse = new CGMBitmap( *this );
                }
            }
            break;

            case 0x0a : /*Generalized Drawing Primitive*/
            {
                ImplGetI( pElement->nIntegerPrecision );  //-Wall is this needed
                ImplGetUI( pElement->nIntegerPrecision ); //-Wall is this needed
                mnParaSize = mnElementSize;
            }
            break;

            case 0x0b : /*Rectangle*/
            {
                if ( mbFigure )
                    mpOutAct->CloseRegion();

                FloatRect   aFloatRect;
                ImplGetRectangle( aFloatRect, sal_True );
                mpOutAct->DrawRectangle( aFloatRect );
            }
            break;

            case 0x0c : /*Circle*/
            {
                if ( mbFigure )
                    mpOutAct->CloseRegion();

                double fRotation = 0;
                FloatPoint aCenter, aRadius;
                ImplGetPoint( aCenter, sal_True );
                if ( pElement->eVDCType == VDC_REAL )
                    aRadius.X = ImplGetFloat( pElement->eVDCRealPrecision, pElement->nVDCRealSize );
                else
                    aRadius.X = (double)ImplGetI( pElement->nVDCIntegerPrecision );
                ImplMapDouble( aRadius.X );
                aRadius.Y = aRadius.X;
                mpOutAct->DrawEllipse( aCenter, aRadius, fRotation );
            }
            break;

            case 0x0d : /*Circular Arc 3 Point*/
            {
                FloatPoint aStartingPoint, aIntermediatePoint, aEndingPoint, aCenterPoint;
                ImplGetPoint( aStartingPoint, sal_True );
                ImplGetPoint( aIntermediatePoint, sal_True );
                ImplGetPoint( aEndingPoint, sal_True );

                double fA = aIntermediatePoint.X - aStartingPoint.X;
                double fB = aIntermediatePoint.Y - aStartingPoint.Y;
                double fC = aEndingPoint.X - aStartingPoint.X;
                double fD = aEndingPoint.Y - aStartingPoint.Y;

                double fE = fA * ( aStartingPoint.X + aIntermediatePoint.X ) + fB * ( aStartingPoint.Y + aIntermediatePoint.Y );
                double fF = fC * ( aStartingPoint.X + aEndingPoint.X ) + fD * ( aStartingPoint.Y + aEndingPoint.Y );

                double fG = 2.0 * ( fA * ( aEndingPoint.Y - aIntermediatePoint.Y ) - fB * ( aEndingPoint.X - aIntermediatePoint.X ) );

                aCenterPoint.X = ( fD * fE - fB * fF ) / fG;
                aCenterPoint.Y = ( fA * fF - fC * fE ) / fG;

                if ( fG != 0 )
                {
                    double fStartAngle = ImplGetOrientation( aCenterPoint, aStartingPoint );
                    double fInterAngle = ImplGetOrientation( aCenterPoint, aIntermediatePoint );
                    double fEndAngle = ImplGetOrientation( aCenterPoint, aEndingPoint );

                    int nSwitch = 0;

                    if ( fStartAngle > fEndAngle )
                    {
                        nSwitch ^=1;
                        aIntermediatePoint = aEndingPoint;
                        aEndingPoint = aStartingPoint;
                        aStartingPoint = aIntermediatePoint;
                        fG = fStartAngle;
                        fStartAngle = fEndAngle;
                        fEndAngle = fG;
                    }
                    if ( ! ( fInterAngle > fStartAngle )  && ( fInterAngle < fEndAngle ) )
                    {
                        nSwitch ^=1;
                        aIntermediatePoint = aEndingPoint;
                        aEndingPoint = aStartingPoint;
                        aStartingPoint = aIntermediatePoint;
                        fG = fStartAngle;
                        fStartAngle = fEndAngle;
                        fEndAngle = fG;
                    }
                    double fRadius = sqrt( pow( ( aStartingPoint.X - aCenterPoint.X ), 2 ) + pow( ( aStartingPoint.Y - aCenterPoint.Y ), 2 ) ) ;

                    if ( mbFigure )
                    {
                        Rectangle aBoundingBox( Point( (long)( aCenterPoint.X - fRadius ), long( aCenterPoint.Y - fRadius ) ),
                            Size( ( static_cast< long >( 2 * fRadius ) ), (long)( 2 * fRadius) ) );
                        Polygon aPolygon( aBoundingBox, Point( (long)aStartingPoint.X, (long)aStartingPoint.Y ) ,Point( (long)aEndingPoint.X, (long)aEndingPoint.Y ), POLY_ARC );
                        if ( nSwitch )
                            mpOutAct->RegPolyLine( aPolygon, sal_True );
                        else
                            mpOutAct->RegPolyLine( aPolygon );
                    }
                    else
                    {
                        fG = 0;
                        FloatPoint aRadius;
                        aRadius.X = aRadius.Y = fRadius;
                        mpOutAct->DrawEllipticalArc( aCenterPoint, aRadius, fG, 2, fStartAngle, fEndAngle );
                    }
                }
            }
            break;

            case 0x0e : /*Circular Arc 3 Point Close*/
            {
                if ( mbFigure )
                    mpOutAct->CloseRegion();

                FloatPoint aStartingPoint, aIntermediatePoint, aEndingPoint, aCenterPoint;
                ImplGetPoint( aStartingPoint );
                ImplGetPoint( aIntermediatePoint );
                ImplGetPoint( aEndingPoint );

                double fA = aIntermediatePoint.X - aStartingPoint.X;
                double fB = aIntermediatePoint.Y - aStartingPoint.Y;
                double fC = aEndingPoint.X - aStartingPoint.X;
                double fD = aEndingPoint.Y - aStartingPoint.Y;

                double fE = fA * ( aStartingPoint.X + aIntermediatePoint.X ) + fB * ( aStartingPoint.Y + aIntermediatePoint.Y );
                double fF = fC * ( aStartingPoint.X + aEndingPoint.X ) + fD * ( aStartingPoint.Y + aEndingPoint.Y );

                double fG = 2.0 * ( fA * ( aEndingPoint.Y - aIntermediatePoint.Y ) - fB * ( aEndingPoint.X - aIntermediatePoint.X ) );

                aCenterPoint.X = ( fD * fE - fB * fF ) / fG;
                aCenterPoint.Y = ( fA * fF - fC * fE ) / fG;

                if ( fG != 0 )
                {
                    double fStartAngle = ImplGetOrientation( aCenterPoint, aStartingPoint );
                    double fInterAngle = ImplGetOrientation( aCenterPoint, aIntermediatePoint );
                    double fEndAngle = ImplGetOrientation( aCenterPoint, aEndingPoint );

                    if ( fStartAngle > fEndAngle )
                    {
                        aIntermediatePoint = aEndingPoint;
                        aEndingPoint = aStartingPoint;
                        aStartingPoint = aIntermediatePoint;
                        fG = fStartAngle;
                        fStartAngle = fEndAngle;
                        fEndAngle = fG;
                    }
                    if ( ! ( fInterAngle > fStartAngle )  && ( fInterAngle < fEndAngle ) )
                    {
                        aIntermediatePoint = aEndingPoint;
                        aEndingPoint = aStartingPoint;
                        aStartingPoint = aIntermediatePoint;
                        fG = fStartAngle;
                        fStartAngle = fEndAngle;
                        fEndAngle = fG;
                    }
                    FloatPoint fRadius;
                    fRadius.Y = fRadius.X = sqrt( pow( ( aStartingPoint.X - aCenterPoint.X ), 2 ) + pow( ( aStartingPoint.Y - aCenterPoint.Y ), 2 ) ) ;

                    sal_uInt32 nType = ImplGetUI16();
                    if ( nType == 0 )
                        nType = 0;          // is PIE
                    else
                        nType = 1;          // is CHORD

                    double fOrientation = 0;
                    mpOutAct->DrawEllipticalArc( aCenterPoint, fRadius, fOrientation, nType, fStartAngle, fEndAngle );
                }
            }
            break;

            case 0x0f : /*Circular Arc Centre*/
            {
                double fStartAngle, fEndAngle, vector[ 4 ];
                FloatPoint aCenter, aRadius;

                if ( mbFigure )
                    mpOutAct->CloseRegion();

                ImplGetPoint( aCenter, sal_True );
                ImplGetVector( &vector[ 0 ] );

                if ( pElement->eVDCType == VDC_REAL )
                {
                    aRadius.X = (double)ImplGetFloat( pElement->eVDCRealPrecision, pElement->nVDCRealSize );
                }
                else
                {
                    aRadius.X = (double)ImplGetI( pElement->nVDCIntegerPrecision );
                }

                ImplMapDouble( aRadius.X );
                aRadius.Y = aRadius.X;

                fStartAngle = acos( vector[ 0 ] / sqrt( vector[ 0 ] * vector[ 0 ] + vector[ 1 ] * vector[ 1 ] ) ) * 57.29577951308;
                fEndAngle = acos( vector[ 2 ] / sqrt( vector[ 2 ] * vector[ 2 ] + vector[ 3 ] * vector[ 3 ] ) ) * 57.29577951308;

                if ( vector[ 1 ] > 0 )
                    fStartAngle = 360 - fStartAngle;
                if ( vector[ 3 ] > 0 )
                    fEndAngle = 360 - fEndAngle;

                if ( mbAngReverse )
                    ImplSwitchStartEndAngle( fStartAngle, fEndAngle );

                if ( mbFigure )
                {
                    Rectangle aBoundingBox(
                        Point( (long)( aCenter.X - aRadius.X ), long( aCenter.Y - aRadius.X ) ),
                        Size( static_cast< long >( 2 * aRadius.X ), (long)( 2 * aRadius.X ) ) );
                    Polygon aPolygon( aBoundingBox,
                        Point( (long)vector[ 0 ], (long)vector[ 1 ] ),
                        Point( (long)vector[ 2 ], (long)vector[ 3 ] ), POLY_ARC );
                    mpOutAct->RegPolyLine( aPolygon );
                }
                else
                {
                    double fOrientation = 0;
                    mpOutAct->DrawEllipticalArc( aCenter, aRadius, fOrientation, 2, fStartAngle, fEndAngle );
                }
                mnParaSize = mnElementSize;

            }
            break;

            case 0x10 : /*Circular Arc Centre Close*/
            {
                double fOrientation, fStartAngle, fEndAngle, vector[ 4 ];
                FloatPoint aCenter, aRadius;

                if ( mbFigure )
                    mpOutAct->CloseRegion();

                ImplGetPoint( aCenter, sal_True );
                ImplGetVector( &vector[ 0 ] );
                if ( pElement->eVDCType == VDC_REAL )
                {
                    aRadius.X = (double)ImplGetFloat( pElement->eVDCRealPrecision, pElement->nVDCRealSize );
                }
                else
                {
                    aRadius.X = (double)ImplGetI( pElement->nVDCIntegerPrecision );
                }
                ImplMapDouble( aRadius.X );
                aRadius.Y = aRadius.X;
                fStartAngle = acos( vector[ 0 ] / sqrt( vector[ 0 ] * vector[ 0 ] + vector[ 1 ] * vector[ 1 ] ) ) * 57.29577951308;
                fEndAngle = acos( vector[ 2 ] / sqrt( vector[ 2 ] * vector[ 2 ] + vector[ 3 ] * vector[ 3 ] ) ) * 57.29577951308;

                if ( vector[ 1 ] > 0 )
                    fStartAngle = 360 - fStartAngle;
                if ( vector[ 3 ] > 0 )
                    fEndAngle = 360 - fEndAngle;

                if ( mbAngReverse )
                    ImplSwitchStartEndAngle( fStartAngle, fEndAngle );


                sal_uInt32 nType = ImplGetUI16();
                if ( nType == 0 )
                    nType = 0;          // is PIE
                else
                    nType = 1;          // is CHORD
                fOrientation = 0;

                mpOutAct->DrawEllipticalArc( aCenter, aRadius, fOrientation,
                            nType, fStartAngle, fEndAngle );
                mnParaSize = mnElementSize;
            }
            break;

            case 0x11 : /*Ellipse*/
            {
                double fOrientation;
                FloatPoint aCenter, aRadius;

                if ( mbFigure )
                    mpOutAct->CloseRegion();

                ImplGetEllipse( aCenter, aRadius, fOrientation ) ;
                mpOutAct->DrawEllipse( aCenter, aRadius, fOrientation ) ;
            }
            break;

            case 0x12 : /*Elliptical Arc*/
            {
                if ( mbFigure )
                    mpOutAct->CloseRegion();

                double fOrientation, fStartAngle, fEndAngle, vector[ 4 ];
                FloatPoint aCenter, aRadius;

                if ( mbFigure )
                    mpOutAct->CloseRegion();

                sal_Bool bDirection = ImplGetEllipse( aCenter, aRadius, fOrientation );
                ImplGetVector( &vector[ 0 ] );

                fStartAngle = acos( vector[ 0 ] / sqrt( vector[ 0 ] * vector[ 0 ] + vector[ 1 ] * vector[ 1 ] ) ) * 57.29577951308;
                fEndAngle = acos( vector[ 2 ] / sqrt( vector[ 2 ] * vector[ 2 ] + vector[ 3 ] * vector[ 3 ] ) ) * 57.29577951308;

                if ( vector[ 1 ] > 0 )
                    fStartAngle = 360 - fStartAngle;
                if ( vector[ 3 ] > 0 )
                    fEndAngle = 360 - fEndAngle;

                if ( bDirection )
                    mpOutAct->DrawEllipticalArc( aCenter, aRadius, fOrientation,
                                2, fStartAngle, fEndAngle );
                else
                    mpOutAct->DrawEllipticalArc( aCenter, aRadius, fOrientation,
                                2, fEndAngle, fStartAngle);
            }
            break;

            case 0x13 : /*Elliptical Arc Close*/
            {
                double fOrientation, fStartAngle, fEndAngle, vector[ 4 ];
                FloatPoint aCenter, aRadius;

                if ( mbFigure )
                    mpOutAct->CloseRegion();

                sal_Bool bDirection = ImplGetEllipse( aCenter, aRadius, fOrientation );
                ImplGetVector( &vector[ 0 ] );

                fStartAngle = acos( vector[ 0 ] / sqrt( vector[ 0 ] * vector[ 0 ] + vector[ 1 ] * vector[ 1 ] ) ) * 57.29577951308;
                fEndAngle = acos( vector[ 2 ] / sqrt( vector[ 2 ] * vector[ 2 ] + vector[ 3 ] * vector[ 3 ] ) ) * 57.29577951308;

                if ( vector[ 1 ] > 0 )
                    fStartAngle = 360 - fStartAngle;
                if ( vector[ 3 ] > 0 )
                    fEndAngle = 360 - fEndAngle;

                sal_uInt32 nType = ImplGetUI16();
                if ( nType == 0 )
                    nType = 0;          // is PIE
                else
                    nType = 1;          // is CHORD

                if ( bDirection )
                    mpOutAct->DrawEllipticalArc( aCenter, aRadius, fOrientation,
                                nType, fStartAngle, fEndAngle );
                else
                    mpOutAct->DrawEllipticalArc( aCenter, aRadius, fOrientation,
                                nType, fEndAngle, fStartAngle);
            }
            break;
            case 0x14 : /*Circular Arc Centre Reversed*/
            {
                if ( mbFigure )
                    mpOutAct->CloseRegion();
            }
            break;
            case 0x15 : /*Connection Edge   */                      // NS
            {
//              if ( mbFigure )
//                  mpOutAct->CloseRegion();
            }
            break;
            case 0x16 : /*Hyperbolic Arc    */                      // NS
            {
                if ( mbFigure )
                    mpOutAct->CloseRegion();
            }
            break;
            case 0x17 : /*Parabolic Arc */                      // NS
            {
                if ( mbFigure )
                    mpOutAct->CloseRegion();
            }
            break;
            case 0x18 : /*Non Uniform B-Spline  */              // NS
            {
                if ( mbFigure )
                    mpOutAct->CloseRegion();
            }
            break;
            case 0x19 : /*Non Uniform Rational B-Spline */      // NS
            {
                if ( mbFigure )
                    mpOutAct->CloseRegion();
            }
            break;
            case 0x1a : /*Polybezier*/
            {
                sal_uInt32 nOrder = ImplGetI( pElement->nIntegerPrecision );

                sal_uInt16 nNumberOfPoints = sal::static_int_cast< sal_uInt16 >(( mnElementSize - pElement->nIntegerPrecision ) / ImplGetPointSize());

                Polygon aPolygon( nNumberOfPoints );

                for ( sal_uInt16 i = 0; i < nNumberOfPoints; i++)
                {
                    FloatPoint  aFloatPoint;
                    ImplGetPoint( aFloatPoint, sal_True );
                    aPolygon.SetPoint( Point ( (long)( aFloatPoint.X ), (long)( aFloatPoint.Y ) ), i );
                }
                if ( nOrder & 4 )
                {
                    for ( sal_uInt16 i = 0; i < nNumberOfPoints; i++ )
                    {
                        if ( ( i % 3 ) == 0 )
                            aPolygon.SetFlags( i, POLY_NORMAL );
                        else
                            aPolygon.SetFlags( i, POLY_CONTROL );
                    }
                }
                else
                {
                    for ( sal_uInt16 i = 0; i < nNumberOfPoints; i++ )
                    {
                        switch ( i & 3 )
                        {
                            case 0 :
                            case 3 : aPolygon.SetFlags( i, POLY_NORMAL ); break;
                            default : aPolygon.SetFlags( i, POLY_CONTROL ); break;
                        }
                    }
                }
                if ( mbFigure )
                    mpOutAct->RegPolyLine( aPolygon );
                else
                    mpOutAct->DrawPolybezier( aPolygon );
                mnParaSize = mnElementSize;
            }
            break;

            case 0x1b : /*Polysymbol    */                          // NS
            {
                if ( mbFigure )
                    mpOutAct->CloseRegion();
            }
            break;
            case 0x1c : /*Bitonal Tile  */                      // NS
            {
                if ( mbFigure )
                    mpOutAct->CloseRegion();
            }
            break;
            case 0x1d : /*Tile  */                              // NS
            {
                if ( mbFigure )
                    mpOutAct->CloseRegion();
            }
            break;
            case 0x1e : /*Insert Object*/
            {
                if ( mbFigure )
                    mpOutAct->CloseRegion();
            }
            break;
            case 0xff : /*Polybezier*/
            {
                if ( mbFigure )
                    mpOutAct->CloseRegion();
            }
            break;
            case 0xfe : /*Sharp Polybezier*/
            {
                if ( mbFigure )
                    mpOutAct->CloseRegion();
            }
            break;
            case 0xfd : /*Polyspline*/
            {
                if ( mbFigure )
                    mpOutAct->CloseRegion();
            }
            break;
            case 0xfc : /*Reounded Rectangle*/
            {
                if ( mbFigure )
                    mpOutAct->CloseRegion();
            }
            break;
            case 0xfb : /*Begin Cell Array*/
            {
                if ( mbFigure )
                    mpOutAct->CloseRegion();
            }
            break;
            case 0xfa : /*End Cell Array*/
            {
                if ( mbFigure )
                    mpOutAct->CloseRegion();
            }
            break;
            case 0xf9 : /*Insert File*/
            {
                if ( mbFigure )
                    mpOutAct->CloseRegion();
            }
            break;
            case 0xf8 : /*Block Text*/
            {
                if ( mbFigure )
                    mpOutAct->CloseRegion();
            }
            break;
            case 0xf7 : /*Variable Width Polyline*/
            {
                if ( mbFigure )
                    mpOutAct->CloseRegion();
            }
            break;
            case 0xf6 : /*Elliptical Arc 3 Point*/
            {
                if ( mbFigure )
                    mpOutAct->CloseRegion();
            }
            break;
            case 0xf1 : /*Hyperlink Definition  */break;
            default: break;
        }
    }
    else
        mnParaSize = mnElementSize;
};


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
