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

#include <o3tl/float_int_conversion.hxx>
#include <o3tl/safeint.hxx>

#include <memory>

using namespace ::com::sun::star;

double CGM::ImplGetOrientation( FloatPoint const & rCenter, FloatPoint const & rPoint )
{
    double nX = rPoint.X - rCenter.X;
    double nY = rPoint.Y - rCenter.Y;

    double fSqrt = std::hypot(nX, nY);
    double fOrientation = fSqrt != 0.0 ? basegfx::rad2deg(acos(nX / fSqrt)) : 0.0;
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
    rRadius.X = std::hypot(aPoint1.X, aPoint1.Y);
    aPoint2.X -= rCenter.X;
    aPoint2.Y -= rCenter.Y;
    rRadius.Y = std::hypot(aPoint2.X, aPoint2.Y);

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
                    aPolygon.SetPoint( Point( static_cast<tools::Long>(aFloatPoint.X), static_cast<tools::Long>(aFloatPoint.Y) ), i );
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
                            aPolygon.SetPoint( Point( static_cast<tools::Long>(aFloatPoint.X), static_cast<tools::Long>(aFloatPoint.Y) ), 0 );
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
                            aPolygon.SetPoint( Point( static_cast<tools::Long>(aFloatPoint.X), static_cast<tools::Long>(aFloatPoint.Y) ), 0 );
                            ImplGetPoint( aFloatPoint, true );
                            aPolygon.SetPoint( Point( static_cast<tools::Long>(aFloatPoint.X), static_cast<tools::Long>(aFloatPoint.Y) ), 1);
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
                    throw css::uno::Exception(u"attempt to read past end of input"_ustr, nullptr);

                OUString aStr(reinterpret_cast<char*>(mpSource) + mnParaSize, nSize, RTL_TEXTENCODING_ASCII_US);

                awt::Size aSize;
                awt::Point aPoint( static_cast<tools::Long>(aFloatPoint.X), static_cast<tools::Long>(aFloatPoint.Y) );
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
                    throw css::uno::Exception(u"attempt to read past end of input"_ustr, nullptr);

                OUString aStr(reinterpret_cast<char*>(mpSource) + mnParaSize, nSize, RTL_TEXTENCODING_ASCII_US);

                awt::Point aPoint( static_cast<tools::Long>(aFloatPoint.X), static_cast<tools::Long>(aFloatPoint.Y) );
                awt::Size aSize(static_cast<tools::Long>(dx), static_cast<tools::Long>(dy));
                mpOutAct->DrawText(aPoint, aSize , aStr, static_cast<FinalFlag>(nType));
                mnParaSize = mnElementSize;
            }
            break;

            case 0x06 : /*Append Text*/
            {
                (void)ImplGetUI16(); // nType
                sal_uInt32 nSize = ImplGetUI( 1 );

                if (o3tl::make_unsigned(mpEndValidSource - (mpSource + mnParaSize)) <= nSize)
                    throw css::uno::Exception(u"attempt to read past end of input"_ustr, nullptr);

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
                    aPolygon.SetPoint( Point ( static_cast<tools::Long>( aFloatPoint.X ), static_cast<tools::Long>( aFloatPoint.Y ) ), i );
                }
                mpOutAct->DrawPolygon( aPolygon );
            }
            break;

            case 0x08 : /*Polygon Set*/
            {
                if ( mbFigure )
                    mpOutAct->CloseRegion();

                std::vector<Point> aPoints;
                tools::PolyPolygon aPolyPolygon;
                FloatPoint  aFloatPoint;

                while ( mnParaSize < mnElementSize )
                {
                    ImplGetPoint( aFloatPoint, true );
                    sal_uInt32 nEdgeFlag = ImplGetUI16();
                    aPoints.push_back(Point(static_cast<tools::Long>(aFloatPoint.X), static_cast<tools::Long>(aFloatPoint.Y)));
                    if ( ( nEdgeFlag & 2 ) || ( mnParaSize == mnElementSize ) )
                    {
                        aPolyPolygon.Insert(tools::Polygon(aPoints.size(), aPoints.data()));
                        aPoints.clear();
                    }
                }
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

                bool bUseless = fG == 0;

                FloatPoint aCenterPoint;
                if (!bUseless)
                {
                    aCenterPoint.X = ( fD * fE - fB * fF ) / fG;
                    aCenterPoint.Y = ( fA * fF - fC * fE ) / fG;
                    bUseless = useless(aCenterPoint.X) || useless(aCenterPoint.Y);
                }

                if (!bUseless)
                    bUseless = useless(aStartingPoint.X) || useless(aStartingPoint.Y);

                if (!bUseless)
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
                    double fRadius = std::hypot(aStartingPoint.X - aCenterPoint.X, aStartingPoint.Y - aCenterPoint.Y);

                    if ( mbFigure )
                    {
                        double fLeft = aCenterPoint.X - fRadius;
                        double fTop = aCenterPoint.Y - fRadius;
                        double fRight = fLeft + (2 * fRadius);
                        double fBottom = fTop + (2 * fRadius);
                        bUseless = useless(fLeft) || useless(fTop) || useless(2 * fRadius) || useless(fRight) || useless(fBottom);
                        if (!bUseless)
                        {
                            double fCenterCalc = fLeft + fRight;
                            bUseless = !o3tl::convertsToAtLeast(fCenterCalc, std::numeric_limits<tools::Long>::min()) ||
                                       !o3tl::convertsToAtMost(fCenterCalc, std::numeric_limits<tools::Long>::max());
                        }
                        if (!bUseless)
                        {
                            double fCenterCalc = fTop + fBottom;
                            bUseless = !o3tl::convertsToAtLeast(fCenterCalc, std::numeric_limits<tools::Long>::min()) ||
                                       !o3tl::convertsToAtMost(fCenterCalc, std::numeric_limits<tools::Long>::max());
                        }
                        if (!bUseless)
                        {
                            tools::Rectangle aBoundingBox(Point(fLeft, fTop), Size(2 * fRadius, 2 * fRadius));
                            tools::Polygon aPolygon( aBoundingBox, Point( static_cast<tools::Long>(aStartingPoint.X), static_cast<tools::Long>(aStartingPoint.Y) ) ,Point( static_cast<tools::Long>(aEndingPoint.X), static_cast<tools::Long>(aEndingPoint.Y) ), PolyStyle::Arc );
                            if ( nSwitch )
                                mpOutAct->RegPolyLine( aPolygon, true );
                            else
                                mpOutAct->RegPolyLine( aPolygon );
                        }
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
                    fRadius.Y = fRadius.X = std::hypot(aStartingPoint.X - aCenterPoint.X, aStartingPoint.Y - aCenterPoint.Y);

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

                bool bUseless = useless(vector[0]) || useless(vector[1]) || useless(vector[2]) || useless(vector[3]);
                if (!bUseless)
                {
                    const double fStartSqrt = std::hypot(vector[0], vector[1]);
                    fStartAngle = fStartSqrt != 0.0 ? basegfx::rad2deg(acos(vector[0] / fStartSqrt)) : 0.0;
                    const double fEndSqrt = std::hypot(vector[2], vector[3]);
                    fEndAngle = fEndSqrt != 0.0 ? basegfx::rad2deg(acos(vector[ 2 ] / fEndSqrt)) : 0.0;

                    if ( vector[ 1 ] > 0 )
                        fStartAngle = 360 - fStartAngle;
                    if ( vector[ 3 ] > 0 )
                        fEndAngle = 360 - fEndAngle;

                    if ( mbAngReverse )
                        ImplSwitchStartEndAngle( fStartAngle, fEndAngle );

                    if ( mbFigure )
                    {
                        double fLeft = aCenter.X - aRadius.X;
                        double fTop = aCenter.Y - aRadius.X;
                        double fRight = fLeft + (2 * aRadius.X);
                        double fBottom = fTop + (2 * aRadius.X);
                        bUseless = useless(fLeft) || useless(fTop) || useless(2 * aRadius.X) || useless(fRight) || useless(fBottom);
                        if (!bUseless)
                        {
                            double fCenterCalc = fLeft + fRight;
                            bUseless = !o3tl::convertsToAtLeast(fCenterCalc, std::numeric_limits<tools::Long>::min()) ||
                                       !o3tl::convertsToAtMost(fCenterCalc, std::numeric_limits<tools::Long>::max());
                        }
                        if (!bUseless)
                        {
                            double fCenterCalc = fTop + fBottom;
                            bUseless = !o3tl::convertsToAtLeast(fCenterCalc, std::numeric_limits<tools::Long>::min()) ||
                                       !o3tl::convertsToAtMost(fCenterCalc, std::numeric_limits<tools::Long>::max());
                        }
                        if (!bUseless)
                        {
                            tools::Rectangle aBoundingBox(Point(fLeft, fTop), Size(2 * aRadius.X, 2 * aRadius.X));
                            tools::Polygon aPolygon( aBoundingBox,
                                Point( static_cast<tools::Long>(vector[ 0 ]), static_cast<tools::Long>(vector[ 1 ]) ),
                                Point( static_cast<tools::Long>(vector[ 2 ]), static_cast<tools::Long>(vector[ 3 ]) ), PolyStyle::Arc );
                            mpOutAct->RegPolyLine( aPolygon );
                        }
                    }
                    else
                    {
                        double fOrientation = 0;
                        mpOutAct->DrawEllipticalArc( aCenter, aRadius, fOrientation, 2, fStartAngle, fEndAngle );
                    }
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

                sal_uInt32 nType = ImplGetUI16();

                bool bUseless = useless(vector[0]) || useless(vector[1]) || useless(vector[2]) || useless(vector[3]);
                if (!bUseless)
                {
                    const double fStartSqrt = std::hypot(vector[0], vector[1]);
                    double fStartAngle = fStartSqrt ? basegfx::rad2deg(acos(vector[0] / fStartSqrt)) : 0.0;
                    const double fEndSqrt = std::hypot(vector[2], vector[3]);
                    double fEndAngle = fEndSqrt ? basegfx::rad2deg(acos(vector[2] / fEndSqrt)) : 0.0;

                    if ( vector[ 1 ] > 0 )
                        fStartAngle = 360 - fStartAngle;
                    if ( vector[ 3 ] > 0 )
                        fEndAngle = 360 - fEndAngle;

                    if ( mbAngReverse )
                        ImplSwitchStartEndAngle( fStartAngle, fEndAngle );

                    if ( nType == 0 )
                        nType = 0;          // is PIE
                    else
                        nType = 1;          // is CHORD
                    fOrientation = 0;

                    mpOutAct->DrawEllipticalArc( aCenter, aRadius, fOrientation,
                                nType, fStartAngle, fEndAngle );
                }

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

                bool bUseless = useless(vector[0]) || useless(vector[1]) || useless(vector[2]) || useless(vector[3]);
                if (!bUseless)
                {
                    double fStartSqrt = std::hypot(vector[0], vector[1]);
                    fStartAngle = fStartSqrt ? basegfx::rad2deg(acos(vector[0] / fStartSqrt)) : 0.0;
                    double fEndSqrt = std::hypot(vector[2], vector[3]);
                    fEndAngle = fEndSqrt ? basegfx::rad2deg(acos(vector[2] / fEndSqrt)) : 0.0;

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

                sal_uInt32 nType = ImplGetUI16();

                bool bUseless = useless(vector[0]) || useless(vector[1]) || useless(vector[2]) || useless(vector[3]);
                if (!bUseless)
                {
                    double fStartSqrt = std::hypot(vector[0], vector[1]);
                    fStartAngle = fStartSqrt ? basegfx::rad2deg(acos(vector[0] / fStartSqrt)) : 0.0;
                    double fEndSqrt = std::hypot(vector[2], vector[3]);
                    fEndAngle = fEndSqrt ? basegfx::rad2deg(acos(vector[2] / fEndSqrt)) : 0.0;

                    if ( vector[ 1 ] > 0 )
                        fStartAngle = 360 - fStartAngle;
                    if ( vector[ 3 ] > 0 )
                        fEndAngle = 360 - fEndAngle;

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
                    aPolygon.SetPoint( Point ( static_cast<tools::Long>( aFloatPoint.X ), static_cast<tools::Long>( aFloatPoint.Y ) ), i );
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
