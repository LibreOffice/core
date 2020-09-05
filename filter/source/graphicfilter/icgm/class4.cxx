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


#include "bitmap.hxx"
#include "cgm.hxx"
#include "chart.hxx"
#include "elements.hxx"
#include "outact.hxx"

#include <o3tl/safeint.hxx>

#include <math.h>
#include <memory>

using namespace ::com::sun::star;

double CGM::ImplGetOrientation( FloatPoint const & rCenter, FloatPoint const & rPoint )
{
    double nX = rPoint.X - rCenter.X;
    double nY = rPoint.Y - rCenter.Y;

    double fSqrt = sqrt(nX * nX + nY * nY);
    double fOrientation = fSqrt != 0.0 ? (acos(nX / fSqrt) * 57.29577951308) : 0.0;
    if (nY > 0)
        fOrientation = 360 - fOrientation;

    return fOrientation;
}


void CGM::ImplSwitchStartEndAngle( double& rStartAngle, double& rEndAngle )
{
    double nTemp;
    nTemp = rStartAngle;
    rStartAngle = rEndAngle;
    rEndAngle = nTemp;
}


void CGM::ImplGetVector( double* pVector )
{
    if ( pElement->eVDCType == VDC_REAL )
    {
        for ( sal_uInt32 i = 0; i < 4; i++ )
        {
            pVector[ i ] = ImplGetFloat( pElement->eVDCRealPrecision, pElement->nVDCRealSize );
        }
    }
    else
    {
        for ( sal_uInt32 i = 0; i < 4; i++ )
        {
            pVector[ i ] = static_cast<double>(ImplGetI( pElement->nVDCIntegerPrecision ));
        }
    }
    pVector[ 0 ] *= mnVDCXmul;
    pVector[ 2 ] *= mnVDCXmul;
    pVector[ 1 ] *= mnVDCYmul;
    pVector[ 3 ] *= mnVDCYmul;
}


bool CGM::ImplGetEllipse( FloatPoint& rCenter, FloatPoint& rRadius, double& rAngle )
{
    FloatPoint  aPoint1, aPoint2;
    double      fRot1, fRot2;
    ImplGetPoint( rCenter, true );
    ImplGetPoint( aPoint1, true );
    ImplGetPoint( aPoint2, true );
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
            return false;
    }
    else
    {
        if ( ( fRot2 - fRot1 ) > 180 )
            return false;
    }
    return true;
}

void CGM::ImplDoClass4()
{
    if ( mbFirstOutPut )
        mpOutAct->FirstOutPut();

    if ( mpBitmapInUse && ( mnElementID != 9 ) )    // process existed graphic
    {                                               // because there are now no pending bitmap actions
        CGMBitmapDescriptor* pBmpDesc = mpBitmapInUse->GetBitmap();
        // do anything with the bitmap
        mpOutAct->DrawBitmap( pBmpDesc );
        mpBitmapInUse.reset();
    }

    if ( ( mpChart == nullptr ) || mpChart->IsAnnotation() )
    {
        switch ( mnElementID )
        {
            case 0x01 : /*PolyLine*/
            {
                sal_uInt32 nPoints = mnElementSize / ImplGetPointSize();
                tools::Polygon aPolygon( static_cast<sal_uInt16>(nPoints) );
                for ( sal_uInt32 i = 0; i < nPoints; i++)
                {
                    FloatPoint  aFloatPoint;
                    ImplGetPoint( aFloatPoint, true );
                    aPolygon.SetPoint( Point( static_cast<long>(aFloatPoint.X), static_cast<long>(aFloatPoint.Y) ), i );
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
                        tools::Polygon aPolygon( nPoints );
                        for ( sal_uInt16 i = 0; i < nPoints; i++ )
                        {
                            ImplGetPoint( aFloatPoint, true );
                            aPolygon.SetPoint( Point( static_cast<long>(aFloatPoint.X), static_cast<long>(aFloatPoint.Y) ), 0 );
                        }
                        mpOutAct->RegPolyLine( aPolygon );
                    }
                    else
                    {
                        mpOutAct->BeginGroup();
                        tools::Polygon aPolygon( sal_uInt16(2) );
                        for ( sal_uInt16 i = 0; i < nPoints; i++ )
                        {
                            ImplGetPoint( aFloatPoint, true );
                            aPolygon.SetPoint( Point( static_cast<long>(aFloatPoint.X), static_cast<long>(aFloatPoint.Y) ), 0 );
                            ImplGetPoint( aFloatPoint, true );
                            aPolygon.SetPoint( Point( static_cast<long>(aFloatPoint.X), static_cast<long>(aFloatPoint.Y) ), 1);
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

                if ( mbFigure )
                    mpOutAct->CloseRegion();

                ImplGetPoint ( aFloatPoint, true );
                sal_uInt32 nType = ImplGetUI16();
                sal_uInt32 nSize = ImplGetUI( 1 );

                if (o3tl::make_unsigned(mpEndValidSource - (mpSource + mnParaSize)) < nSize)
                    throw css::uno::Exception("attempt to read past end of input", nullptr);

                OUString aStr(reinterpret_cast<char*>(mpSource) + mnParaSize, nSize, RTL_TEXTENCODING_ASCII_US);

                awt::Size aSize;
                awt::Point aPoint( static_cast<long>(aFloatPoint.X), static_cast<long>(aFloatPoint.Y) );
                mpOutAct->DrawText(aPoint, aSize, aStr, static_cast<FinalFlag>(nType));
                mnParaSize = mnElementSize;
            }
            break;

            case 0x05 : /*Restricted Text*/
            {
                double      dx, dy;
                FloatPoint  aFloatPoint;

                if ( mbFigure )
                    mpOutAct->CloseRegion();

                if ( pElement->eVDCType == VDC_REAL )
                {
                    dx = ImplGetFloat( pElement->eVDCRealPrecision, pElement->nVDCRealSize );
                    dy = ImplGetFloat( pElement->eVDCRealPrecision, pElement->nVDCRealSize );
                }
                else
                {
                    dx = static_cast<double>(ImplGetI( pElement->nVDCIntegerPrecision ));
                    dy = static_cast<double>(ImplGetI( pElement->nVDCIntegerPrecision ));
                }
                ImplMapDouble( dx );
                ImplMapDouble( dy );

                ImplGetPoint ( aFloatPoint, true );
                sal_uInt32 nType = ImplGetUI16();
                sal_uInt32 nSize = ImplGetUI(1);

                if (o3tl::make_unsigned(mpEndValidSource - (mpSource + mnParaSize)) < nSize)
                    throw css::uno::Exception("attempt to read past end of input", nullptr);

                OUString aStr(reinterpret_cast<char*>(mpSource) + mnParaSize, nSize, RTL_TEXTENCODING_ASCII_US);

                awt::Point aPoint( static_cast<long>(aFloatPoint.X), static_cast<long>(aFloatPoint.Y) );
                awt::Size aSize(static_cast<long>(dx), static_cast<long>(dy));
                mpOutAct->DrawText(aPoint, aSize , aStr, static_cast<FinalFlag>(nType));
                mnParaSize = mnElementSize;
            }
            break;

            case 0x06 : /*Append Text*/
            {
                (void)ImplGetUI16(); // nType
                sal_uInt32 nSize = ImplGetUI( 1 );

                if (o3tl::make_unsigned(mpEndValidSource - (mpSource + mnParaSize)) <= nSize)
                    throw css::uno::Exception("attempt to read past end of input", nullptr);

                mpSource[ mnParaSize + nSize ] = 0;

                mpOutAct->AppendText( reinterpret_cast<char*>(mpSource) + mnParaSize );
                mnParaSize = mnElementSize;
            }
            break;

            case 0x07 : /*Polygon*/
            {
                if ( mbFigure )
                    mpOutAct->CloseRegion();

                sal_uInt16 nPoints = sal::static_int_cast< sal_uInt16 >(
                    mnElementSize / ImplGetPointSize());
                tools::Polygon aPolygon( nPoints );
                for ( sal_uInt16 i = 0; i < nPoints; i++)
                {
                    FloatPoint  aFloatPoint;
                    ImplGetPoint( aFloatPoint, true );
                    aPolygon.SetPoint( Point ( static_cast<long>( aFloatPoint.X ), static_cast<long>( aFloatPoint.Y ) ), i );
                }
                mpOutAct->DrawPolygon( aPolygon );
            }
            break;

            case 0x08 : /*Polygon Set*/
            {
                if ( mbFigure )
                    mpOutAct->CloseRegion();

                sal_uInt16      nPoints = 0;
                std::unique_ptr<Point[]> pPoints(new Point[ 0x4000 ]);

                tools::PolyPolygon aPolyPolygon;
                FloatPoint  aFloatPoint;
                sal_uInt32      nEdgeFlag;
                while ( mnParaSize < mnElementSize )
                {
                    ImplGetPoint( aFloatPoint, true );
                    nEdgeFlag = ImplGetUI16();
                    pPoints[ nPoints++ ] = Point( static_cast<long>(aFloatPoint.X), static_cast<long>(aFloatPoint.Y) );
                    if ( ( nEdgeFlag & 2 ) || ( mnParaSize == mnElementSize ) )
                    {
                        tools::Polygon aPolygon( nPoints );
                        for ( sal_uInt16 i = 0; i < nPoints; i++ )
                        {
                            aPolygon.SetPoint( pPoints[ i ], i );
                        }
                        aPolyPolygon.Insert( aPolygon );
                        nPoints = 0;
                    }
                }
                pPoints.reset();
                mpOutAct->DrawPolyPolygon( aPolyPolygon );
            }
            break;

            case 0x09 : /*Cell Array*/
            {
                if ( mbFigure )
                    mpOutAct->CloseRegion();

                if ( mpBitmapInUse )
                {
                    std::unique_ptr<CGMBitmap> xBmpDesc(mpBitmapInUse->GetNext());
                    if (xBmpDesc) // we possibly get a bitmap back which does not fit to
                    {               // to the previous -> we need to delete this one too
                        mpOutAct->DrawBitmap(xBmpDesc->GetBitmap());
                    }
                }
                else
                {
                    mpBitmapInUse.reset( new CGMBitmap( *this ) );
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
                ImplGetRectangle( aFloatRect, true );
                mpOutAct->DrawRectangle( aFloatRect );
            }
            break;

            case 0x0c : /*Circle*/
            {
                if ( mbFigure )
                    mpOutAct->CloseRegion();

                double fRotation = 0;
                FloatPoint aCenter, aRadius;
                ImplGetPoint( aCenter, true );
                if ( pElement->eVDCType == VDC_REAL )
                    aRadius.X = ImplGetFloat( pElement->eVDCRealPrecision, pElement->nVDCRealSize );
                else
                    aRadius.X = static_cast<double>(ImplGetI( pElement->nVDCIntegerPrecision ));
                ImplMapDouble( aRadius.X );
                aRadius.Y = aRadius.X;
                mpOutAct->DrawEllipse( aCenter, aRadius, fRotation );
            }
            break;

            case 0x0d : /*Circular Arc 3 Point*/
            {
                FloatPoint aStartingPoint, aIntermediatePoint, aEndingPoint;
                ImplGetPoint( aStartingPoint, true );
                ImplGetPoint( aIntermediatePoint, true );
                ImplGetPoint( aEndingPoint, true );

                double fA = aIntermediatePoint.X - aStartingPoint.X;
                double fB = aIntermediatePoint.Y - aStartingPoint.Y;
                double fC = aEndingPoint.X - aStartingPoint.X;
                double fD = aEndingPoint.Y - aStartingPoint.Y;

                double fE = fA * ( aStartingPoint.X + aIntermediatePoint.X ) + fB * ( aStartingPoint.Y + aIntermediatePoint.Y );
                double fF = fC * ( aStartingPoint.X + aEndingPoint.X ) + fD * ( aStartingPoint.Y + aEndingPoint.Y );

                double fG = 2.0 * ( fA * ( aEndingPoint.Y - aIntermediatePoint.Y ) - fB * ( aEndingPoint.X - aIntermediatePoint.X ) );

                if ( fG != 0 )
                {
                    FloatPoint aCenterPoint;
                    aCenterPoint.X = ( fD * fE - fB * fF ) / fG;
                    aCenterPoint.Y = ( fA * fF - fC * fE ) / fG;
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
                        tools::Rectangle aBoundingBox(aCenterPoint.X - fRadius, aCenterPoint.Y - fRadius);
                        aBoundingBox.SaturatingSetSize(Size(2 * fRadius, 2 * fRadius));
                        tools::Polygon aPolygon( aBoundingBox, Point( static_cast<long>(aStartingPoint.X), static_cast<long>(aStartingPoint.Y) ) ,Point( static_cast<long>(aEndingPoint.X), static_cast<long>(aEndingPoint.Y) ), PolyStyle::Arc );
                        if ( nSwitch )
                            mpOutAct->RegPolyLine( aPolygon, true );
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

                FloatPoint aStartingPoint, aIntermediatePoint, aEndingPoint;
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

                if ( fG != 0 )
                {
                    FloatPoint aCenterPoint;
                    aCenterPoint.X = ( fD * fE - fB * fF ) / fG;
                    aCenterPoint.Y = ( fA * fF - fC * fE ) / fG;
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

                ImplGetPoint( aCenter, true );
                ImplGetVector( &vector[ 0 ] );

                if ( pElement->eVDCType == VDC_REAL )
                {
                    aRadius.X = ImplGetFloat( pElement->eVDCRealPrecision, pElement->nVDCRealSize );
                }
                else
                {
                    aRadius.X = static_cast<double>(ImplGetI( pElement->nVDCIntegerPrecision ));
                }

                ImplMapDouble( aRadius.X );
                aRadius.Y = aRadius.X;

                const double fStartSqrt = sqrt(vector[0] * vector[ 0 ] + vector[1] * vector[1]);
                fStartAngle = fStartSqrt != 0.0 ? (acos(vector[0] / fStartSqrt) * 57.29577951308) : 0.0;
                const double fEndSqrt = sqrt(vector[2] * vector[ 2 ] + vector[3] * vector[3]);
                fEndAngle = fEndSqrt != 0.0 ? (acos(vector[ 2 ] / fEndSqrt) * 57.29577951308) : 0.0;

                if ( vector[ 1 ] > 0 )
                    fStartAngle = 360 - fStartAngle;
                if ( vector[ 3 ] > 0 )
                    fEndAngle = 360 - fEndAngle;

                if ( mbAngReverse )
                    ImplSwitchStartEndAngle( fStartAngle, fEndAngle );

                if ( mbFigure )
                {
                    tools::Rectangle aBoundingBox(aCenter.X - aRadius.X, aCenter.Y - aRadius.X);
                    aBoundingBox.SaturatingSetSize(Size(2 * aRadius.X, 2 * aRadius.X));
                    tools::Polygon aPolygon( aBoundingBox,
                        Point( static_cast<long>(vector[ 0 ]), static_cast<long>(vector[ 1 ]) ),
                        Point( static_cast<long>(vector[ 2 ]), static_cast<long>(vector[ 3 ]) ), PolyStyle::Arc );
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
                double fOrientation, vector[ 4 ];
                FloatPoint aCenter, aRadius;

                if ( mbFigure )
                    mpOutAct->CloseRegion();

                ImplGetPoint( aCenter, true );
                ImplGetVector( &vector[ 0 ] );
                if ( pElement->eVDCType == VDC_REAL )
                {
                    aRadius.X = ImplGetFloat( pElement->eVDCRealPrecision, pElement->nVDCRealSize );
                }
                else
                {
                    aRadius.X = static_cast<double>(ImplGetI( pElement->nVDCIntegerPrecision ));
                }
                ImplMapDouble( aRadius.X );
                aRadius.Y = aRadius.X;
                const double fStartSqrt = sqrt(vector[0] * vector[0] + vector[1] * vector[1]);
                double fStartAngle = fStartSqrt ? (acos(vector[0] / fStartSqrt) * 57.29577951308) : 0.0;
                const double fEndSqrt = sqrt(vector[2] * vector[2] + vector[3] * vector[3]);
                double fEndAngle = fEndSqrt ? acos(vector[2] / fEndSqrt) * 57.29577951308 : 0.0;

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

                bool bDirection = ImplGetEllipse( aCenter, aRadius, fOrientation );
                ImplGetVector( &vector[ 0 ] );

                double fStartSqrt = sqrt(vector[0] * vector[0] + vector[1] * vector[1]);
                fStartAngle = fStartSqrt ? (acos(vector[0] / fStartSqrt) * 57.29577951308) : 0.0;
                double fEndSqrt = sqrt(vector[2] * vector[2] + vector[3] * vector[3]);
                fEndAngle = fEndSqrt ? (acos(vector[2] / fEndSqrt) * 57.29577951308) : 0.0;

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

                bool bDirection = ImplGetEllipse( aCenter, aRadius, fOrientation );
                ImplGetVector( &vector[ 0 ] );

                double fStartSqrt = sqrt(vector[0] * vector[0] + vector[1] * vector[1]);
                fStartAngle = fStartSqrt ? (acos(vector[0] / fStartSqrt) * 57.29577951308) : 0.0;
                double fEndSqrt = sqrt(vector[2] * vector[2] + vector[3] * vector[3]);
                fEndAngle = fEndSqrt ? (acos(vector[2] / fEndSqrt) * 57.29577951308) : 0.0;

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

                tools::Polygon aPolygon( nNumberOfPoints );

                for ( sal_uInt16 i = 0; i < nNumberOfPoints; i++)
                {
                    FloatPoint  aFloatPoint;
                    ImplGetPoint( aFloatPoint, true );
                    aPolygon.SetPoint( Point ( static_cast<long>( aFloatPoint.X ), static_cast<long>( aFloatPoint.Y ) ), i );
                }
                if ( nOrder & 4 )
                {
                    for ( sal_uInt16 i = 0; i < nNumberOfPoints; i++ )
                    {
                        if ( ( i % 3 ) == 0 )
                            aPolygon.SetFlags( i, PolyFlags::Normal );
                        else
                            aPolygon.SetFlags( i, PolyFlags::Control );
                    }
                }
                else
                {
                    for ( sal_uInt16 i = 0; i < nNumberOfPoints; i++ )
                    {
                        switch ( i & 3 )
                        {
                            case 0 :
                            case 3 : aPolygon.SetFlags( i, PolyFlags::Normal ); break;
                            default : aPolygon.SetFlags( i, PolyFlags::Control ); break;
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
            case 0xfc : /*Rounded Rectangle*/
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
