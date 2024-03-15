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

#include <basegfx/numeric/ftools.hxx>
#include <ThreeDHelper.hxx>
#include <Diagram.hxx>
#include <ChartTypeHelper.hxx>
#include <DataSeries.hxx>
#include <DataSeriesHelper.hxx>
#include <defines.hxx>

#include <com/sun/star/drawing/LineStyle.hpp>
#include <comphelper/diagnose_ex.hxx>
#include <tools/helpers.hxx>
#include <rtl/math.hxx>

namespace chart
{
using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;

using ::com::sun::star::uno::Reference;
using ::rtl::math::cos;
using ::rtl::math::sin;
using ::rtl::math::tan;

namespace
{

bool lcl_isRightAngledAxesSetAndSupported( const rtl::Reference< Diagram >& xDiagram )
{
    if( xDiagram.is() )
    {
        bool bRightAngledAxes = false;
        xDiagram->getPropertyValue( "RightAngledAxes") >>= bRightAngledAxes;
        if(bRightAngledAxes)
        {
            if( ChartTypeHelper::isSupportingRightAngledAxes(
                    xDiagram->getChartTypeByIndex( 0 ) ) )
            {
                return true;
            }
        }
    }
    return false;
}

} //end anonymous namespace

drawing::CameraGeometry ThreeDHelper::getDefaultCameraGeometry( bool bPie )
{
    // ViewReferencePoint (Point on the View plane)
    drawing::Position3D vrp(17634.6218373783, 10271.4823817647, 24594.8639082739);
    // ViewPlaneNormal (Normal to the View Plane)
    drawing::Direction3D vpn(0.416199821709347, 0.173649045905254, 0.892537795986984);
    // ViewUpVector (determines the v-axis direction on the view plane as
    // projection of VUP parallel to VPN onto th view pane)
    drawing::Direction3D vup(-0.0733876362771618, 0.984807599917971, -0.157379306090273);

    if( bPie )
    {
        vrp = drawing::Position3D( 0.0, 0.0, 87591.2408759124 );//--> 5 percent perspective
        vpn = drawing::Direction3D( 0.0, 0.0, 1.0 );
        vup = drawing::Direction3D( 0.0, 1.0, 0.0 );
    }

    return drawing::CameraGeometry( vrp, vpn, vup );
}

namespace
{
void lcl_ensureIntervalMinus1To1( double& rSinOrCos )
{
    if (rSinOrCos < -1.0)
       rSinOrCos = -1.0;
    else if (rSinOrCos > 1.0)
        rSinOrCos = 1.0;
}

bool lcl_isSinZero( double fAngleRad )
{
    return ::basegfx::fTools::equalZero( sin(fAngleRad), 0.0000001 );
}
bool lcl_isCosZero( double fAngleRad )
{
    return ::basegfx::fTools::equalZero( cos(fAngleRad), 0.0000001 );
}

}

void ThreeDHelper::convertElevationRotationDegToXYZAngleRad(
    sal_Int32 nElevationDeg, sal_Int32 nRotationDeg,
    double& rfXAngleRad, double& rfYAngleRad, double& rfZAngleRad)
{
    // for a description of the algorithm see issue 72994
    //https://bz.apache.org/ooo/show_bug.cgi?id=72994
    //https://bz.apache.org/ooo/attachment.cgi?id=50608

    nElevationDeg = NormAngle360(nElevationDeg);
    nRotationDeg = NormAngle360(nRotationDeg);

    double& x = rfXAngleRad;
    double& y = rfYAngleRad;
    double& z = rfZAngleRad;

    double E = basegfx::deg2rad(nElevationDeg); //elevation in Rad
    double R = basegfx::deg2rad(nRotationDeg); //rotation in Rad

    if( (nRotationDeg == 0 || nRotationDeg == 180 )
        && ( nElevationDeg == 90 || nElevationDeg == 270 ) )
    {
        //sR==0 && cE==0
        z = 0.0;
        //element 23
        double f23 = cos(R)*sin(E);
        if(f23>0)
            x = M_PI_2;
        else
            x = -M_PI_2;
        y = R;
    }
    else if( ( nRotationDeg == 90 || nRotationDeg == 270 )
        && ( nElevationDeg == 90 || nElevationDeg == 270 ) )
    {
        //cR==0 && cE==0
        z = M_PI_2;
        if( sin(R)>0 )
            x = M_PI_2;
        else
            x = -M_PI_2;

        if( (sin(R)*sin(E))>0 )
            y = 0.0;
        else
            y = M_PI;
    }
    else if( (nRotationDeg == 0 || nRotationDeg == 180 )
        && ( nElevationDeg == 0 || nElevationDeg == 180 ) )
    {
        //sR==0 && sE==0
        z = 0.0;
        y = R;
        x = E;
    }
    else if( ( nRotationDeg == 90 || nRotationDeg == 270 )
        && ( nElevationDeg == 0 || nElevationDeg == 180 ) )
    {
        //cR==0 && sE==0
        z = 0.0;

        if( (sin(R)/cos(E))>0 )
            y = M_PI_2;
        else
            y = -M_PI_2;

        if( (cos(E))>0 )
            x = 0;
        else
            x = M_PI;
    }
    else if ( nElevationDeg == 0 || nElevationDeg == 180 )
    {
        //sR!=0 cR!=0 sE==0
        z = 0.0;
        x = E;
        y = R;
        //use element 13 for sign
        if((cos(x)*sin(y)*sin(R))<0.0)
            y *= -1.0;
    }
    else if ( nElevationDeg == 90 || nElevationDeg == 270 )
    {
        //sR!=0 cR!=0 cE==0
        //element 12 + 22 --> y=0 or M_PI and x=+-M_PI/2
        //-->element 13/23:
        z = atan(sin(R)/(cos(R)*sin(E)));
        //use element 13 for sign for x
        if( (sin(R)*sin(z))>0.0 )
            x = M_PI_2;
        else
            x = -M_PI_2;
        //use element 21 for y
        if( (sin(R)*sin(E)*sin(z))>0.0)
            y = 0.0;
        else
            y = M_PI;
    }
    else if ( nRotationDeg == 0 || nRotationDeg == 180 )
    {
        //sE!=0 cE!=0 sR==0
        z = 0.0;
        x = E;
        y = R;
        double f23 = cos(R)*sin(E);
        if( (f23 * sin(x)) < 0.0 )
            x *= -1.0; //todo ??
    }
    else if (nRotationDeg == 90 || nRotationDeg == 270)
    {
        //sE!=0 cE!=0 cR==0
        //z = +- M_PI/2;
        //x = +- M_PI/2;
        z = M_PI_2;
        x = M_PI_2;
        double sR = sin(R);
        if( sR<0.0 )
            x *= -1.0; //different signs for x and z

        //use element 21:
        double cy = sR*sin(E)/sin(z);
        lcl_ensureIntervalMinus1To1(cy);
        y = acos(cy);

        //use element 22 for sign:
        if( (sin(x)*sin(y)*sin(z)*cos(E))<0.0)
            y *= -1.0;
    }
    else
    {
        z = atan(tan(R) * sin(E));
        if(cos(z)==0.0)
        {
            OSL_FAIL("calculation error in ThreeDHelper::convertElevationRotationDegToXYZAngleRad");
            return;
        }
        double cy = cos(R)/cos(z);
        lcl_ensureIntervalMinus1To1(cy);
        y = acos(cy);

        //element 12 in 23
        double fDenominator = cos(z)*(1.0-pow(sin(y),2));
        if(fDenominator==0.0)
        {
            OSL_FAIL("calculation error in ThreeDHelper::convertElevationRotationDegToXYZAngleRad");
            return;
        }
        double sx = cos(R)*sin(E)/fDenominator;
        lcl_ensureIntervalMinus1To1(sx);
        x = asin( sx );

        //use element 13 for sign:
        double f13a = cos(x)*cos(z)*sin(y);
        double f13b = sin(R)-sx*sin(z);
        if( (f13b*f13a)<0.0 )
        {
            //change x or y
            //use element 22 for further investigations:
            //try
            y *= -1;
            double f22a = cos(x)*cos(z);
            double f22b = cos(E)-(sx*sin(y)*sin(z));
            if( (f22a*f22b)<0.0 )
            {
                y *= -1;
                x=(M_PI-x);
            }
        }
        else
        {
            //change nothing or both
            //use element 22 for further investigations:
            double f22a = cos(x)*cos(z);
            double f22b = cos(E)-(sx*sin(y)*sin(z));
            if( (f22a*f22b)<0.0 )
            {
                y *= -1;
                x=(M_PI-x);
            }
        }
    }
}

void ThreeDHelper::convertXYZAngleRadToElevationRotationDeg(
    sal_Int32& rnElevationDeg, sal_Int32& rnRotationDeg,
    double fXRad, double fYRad, double fZRad)
{
    // for a description of the algorithm see issue 72994
    //https://bz.apache.org/ooo/show_bug.cgi?id=72994
    //https://bz.apache.org/ooo/attachment.cgi?id=50608

    double R = 0.0; //Rotation in Rad
    double E = 0.0; //Elevation in Rad

    double& x = fXRad;
    double& y = fYRad;
    double& z = fZRad;

    double f11 = cos(y)*cos(z);

    if( lcl_isSinZero(y) )
    {
        //siny == 0

        if( lcl_isCosZero(x) )
        {
            //siny == 0 && cosx == 0

            if( lcl_isSinZero(z) )
            {
                //siny == 0 && cosx == 0 && sinz == 0
                //example: x=+-90 y=0oder180 z=0(oder180)

                //element 13+11
                if( f11 > 0 )
                    R = 0.0;
                else
                    R = M_PI;

                //element 23
                double f23 = cos(z)*sin(x) / cos(R);
                if( f23 > 0 )
                    E = M_PI_2;
                else
                    E = -M_PI_2;
            }
            else if( lcl_isCosZero(z) )
            {
                //siny == 0 && cosx == 0 && cosz == 0
                //example: x=+-90 y=0oder180 z=+-90

                double f13 = sin(x)*sin(z);
                //element 13+11
                if( f13 > 0 )
                    R = M_PI_2;
                else
                    R = -M_PI_2;

                //element 21
                double f21 = cos(y)*sin(z) / sin(R);
                if( f21 > 0 )
                    E = M_PI_2;
                else
                    E = -M_PI_2;
            }
            else
            {
                //siny == 0 && cosx == 0 && cosz != 0 && sinz != 0
                //element 11 && 13
                double f13 = sin(x)*sin(z);
                R = atan( f13/f11 );

                if(f11<0)
                    R+=M_PI;

                //element 23
                double f23 = cos(z)*sin(x);
                if( f23/cos(R) > 0 )
                    E = M_PI_2;
                else
                    E = -M_PI_2;
            }
        }
        else if( lcl_isSinZero(x) )
        {
            //sinY==0 sinX==0
            //element 13+11
            if( f11 > 0 )
                R = 0.0;
            else
                R = M_PI;

            double f22 = cos(x)*cos(z);
            if( f22 > 0 )
                E = 0.0;
            else
                E = M_PI;
        }
        else if( lcl_isSinZero(z) )
        {
            //sinY==0 sinZ==0 sinx!=0 cosx!=0
            //element 13+11
            if( f11 > 0 )
                R = 0.0;
            else
                R = M_PI;

            //element 22 && 23
            double f22 = cos(x)*cos(z);
            double f23 = cos(z)*sin(x);
            E = atan( f23/(f22*cos(R)) );
            if( (f22*cos(E))<0 )
                E+=M_PI;
        }
        else if( lcl_isCosZero(z) )
        {
            //sinY == 0 && cosZ == 0 && cosx != 0 && sinx != 0
            double f13 = sin(x)*sin(z);
            //element 13+11
            if( f13 > 0 )
                R = M_PI_2;
            else
                R = -M_PI_2;

            //element 21+22
            double f21 = cos(y)*sin(z);
            if( f21/sin(R) > 0 )
                E = M_PI_2;
            else
                E = -M_PI_2;
        }
        else
        {
            //sinY == 0 && all other !=0
            double f13 = sin(x)*sin(z);
            R = atan( f13/f11 );
            if( (f11*cos(R))<0.0 )
                R+=M_PI;

            double f22 = cos(x)*cos(z);
            if( !lcl_isCosZero(R) )
                E = atan( cos(z)*sin(x) /( f22*cos(R) ) );
            else
                E = atan( cos(y)*sin(z) /( f22*sin(R) ) );
            if( (f22*cos(E))<0 )
                E+=M_PI;
        }
    }
    else if( lcl_isCosZero(y) )
    {
        //cosY==0

        double f13 = sin(x)*sin(z)+cos(x)*cos(z)*sin(y);
        if( f13 >= 0 )
            R = M_PI_2;
        else
            R = -M_PI_2;

        double f22 = cos(x)*cos(z)+sin(x)*sin(y)*sin(z);
        if( f22 >= 0 )
            E = 0.0;
        else
            E = M_PI;
    }
    else if( lcl_isSinZero(x) )
    {
        //cosY!=0 sinY!=0 sinX=0
        if( lcl_isSinZero(z) )
        {
            //cosY!=0 sinY!=0 sinX=0 sinZ=0
            double f13 = cos(x)*cos(z)*sin(y);
            R = atan( f13/f11 );
            //R = asin(f13);
            if( f11<0 )
                R+=M_PI;

            double f22 = cos(x)*cos(z);
            if( f22>0 )
                E = 0.0;
            else
                E = M_PI;
        }
        else if( lcl_isCosZero(z) )
        {
            //cosY!=0 sinY!=0 sinX=0 cosZ=0
            R = x;
            E = y;//or -y
            //use 23 for 'signs'
            double f23 =  -1.0*cos(x)*sin(y)*sin(z);
            if( (f23*cos(R)*sin(E))<0.0 )
            {
                //change R or E
                E = -y;
            }
        }
        else
        {
            //cosY!=0 sinY!=0 sinX=0 sinZ!=0 cosZ!=0
            double f13 = cos(x)*cos(z)*sin(y);
            R = atan( f13/f11 );

            if( f11<0 )
                R+=M_PI;

            double f21 = cos(y)*sin(z);
            double f22 = cos(x)*cos(z);
            E = atan(f21/(f22*sin(R)) );

            if( (f22*cos(E))<0.0 )
                E+=M_PI;
        }
    }
    else if( lcl_isCosZero(x) )
    {
        //cosY!=0 sinY!=0 cosX=0

        if( lcl_isSinZero(z) )
        {
            //cosY!=0 sinY!=0 cosX=0 sinZ=0
            R=0;//13 -> R=0 or M_PI
            if( f11<0.0 )
                R=M_PI;
            E=M_PI_2;//22 -> E=+-M_PI/2
            //use element 11 and 23 for sign
            double f23 = cos(z)*sin(x);
            if( (f11*f23*sin(E))<0.0 )
                E=-M_PI_2;
        }
        else if( lcl_isCosZero(z) )
        {
            //cosY!=0 sinY!=0 cosX=0 cosZ=0
            //element 11 & 13:
            if( (sin(x)*sin(z))>0.0 )
                R=M_PI_2;
            else
                R=-M_PI_2;
            //element 22:
            E=acos( sin(x)*sin(y)*sin(z));
            //use element 21 for sign:
            if( (cos(y)*sin(z)*sin(R)*sin(E))<0.0 )
                E*=-1.0;
        }
        else
        {
            //cosY!=0 sinY!=0 cosX=0 sinZ!=0 cosZ!=0
            //element 13/11
            R = atan( sin(x)*sin(z)/(cos(y)*cos(z)) );
            //use 13 for 'sign'
            if( (sin(x)*sin(z))<0.0 )
                R += M_PI;
            //element 22
            E = acos(sin(x)*sin(y)*sin(z) );
            //use 21 for sign
            if( (cos(y)*sin(z)*sin(R)*sin(E))<0.0 )
                E*=-1.0;
        }
    }
    else if( lcl_isSinZero(z) )
    {
        //cosY!=0 sinY!=0 sinX!=0 cosX!=0 sinZ=0
        //element 11
        R=y;
        //use element 13 for sign
        if( (cos(x)*cos(z)*sin(y)*sin(R))<0.0 )
            R*=-1.0;
        //element 22
        E = acos( cos(x)*cos(z) );
        //use element 23 for sign
        if( (cos(z)*sin(x)*cos(R)*sin(E))<0.0 )
            E*=-1.0;
    }
    else if( lcl_isCosZero(z) )
    {
        //cosY!=0 sinY!=0 sinX!=0 cosX!=0 cosZ=0
        //element 21/23
        R=atan(-cos(y)/(cos(x)*sin(y)));
        //use element 13 for 'sign'
        if( (sin(x)*sin(z)*sin(R))<0.0 )
            R+=M_PI;
        //element 21/22
        E=atan( cos(y)*sin(z)/(sin(R)*sin(x)*sin(y)*sin(z)) );
        //use element 23 for 'sign'
        if( (-cos(x)*sin(y)*sin(z)*cos(R)*sin(E))<0.0 )
            E+=M_PI;
    }
    else
    {
        //cosY!=0 sinY!=0 sinX!=0 cosX!=0 sinZ!=0 cosZ!=0
        //13/11:
        double f13 = sin(x)*sin(z)+cos(x)*cos(z)*sin(y);
        R = atan( f13/ f11 );
        if(f11<0.0)
            R+=M_PI;
        double f22 = cos(x)*cos(z)+sin(x)*sin(y)*sin(z);
        double f23 = cos(x)*sin(y)*sin(z)-cos(z)*sin(x);
        //23/22:
        E = atan( -1.0*f23/(f22*cos(R)) );
        if(f22<0.0)
            E+=M_PI;
    }

    rnElevationDeg = basegfx::fround(basegfx::rad2deg(E));
    rnRotationDeg = basegfx::fround(basegfx::rad2deg(R));
}

double ThreeDHelper::getValueClippedToRange( double fAngle, const double& fPositivLimit )
{
    if( fAngle<-1*fPositivLimit )
        fAngle=-1*fPositivLimit;
    else if( fAngle>fPositivLimit )
        fAngle=fPositivLimit;
    return fAngle;
}

void ThreeDHelper::adaptRadAnglesForRightAngledAxes( double& rfXAngleRad, double& rfYAngleRad )
{
    rfXAngleRad = ThreeDHelper::getValueClippedToRange(rfXAngleRad, basegfx::deg2rad(ThreeDHelper::getXDegreeAngleLimitForRightAngledAxes()) );
    rfYAngleRad = ThreeDHelper::getValueClippedToRange(rfYAngleRad, basegfx::deg2rad(ThreeDHelper::getYDegreeAngleLimitForRightAngledAxes()) );
}


void ThreeDHelper::getCameraDistanceRange( double& rfMinimumDistance, double& rfMaximumDistance )
{
    rfMinimumDistance = 3.0/4.0*FIXED_SIZE_FOR_3D_CHART_VOLUME;//empiric value
    rfMaximumDistance = 20.0*FIXED_SIZE_FOR_3D_CHART_VOLUME;//empiric value
}

void ThreeDHelper::ensureCameraDistanceRange( double& rfCameraDistance )
{
    double fMin, fMax;
    getCameraDistanceRange( fMin, fMax );
    if( rfCameraDistance < fMin )
        rfCameraDistance = fMin;
    if( rfCameraDistance > fMax )
        rfCameraDistance = fMax;
}

double ThreeDHelper::CameraDistanceToPerspective( double fCameraDistance )
{
    double fMin, fMax;
    ThreeDHelper::getCameraDistanceRange( fMin, fMax );
    //fMax <-> 0; fMin <->100
    //a/x + b = y
    double a = 100.0*fMax*fMin/(fMax-fMin);
    double b = -a/fMax;

    double fRet = a/fCameraDistance + b;

    return fRet;
}

double ThreeDHelper::PerspectiveToCameraDistance( double fPerspective )
{
    double fMin, fMax;
    ThreeDHelper::getCameraDistanceRange( fMin, fMax );
    //fMax <-> 0; fMin <->100
    //a/x + b = y
    double a = 100.0*fMax*fMin/(fMax-fMin);
    double b = -a/fMax;

    double fRet = a/(fPerspective - b);

    return fRet;
}

void ThreeDHelper::getRoundedEdgesAndObjectLines(
            const rtl::Reference< Diagram > & xDiagram
            , sal_Int32& rnRoundedEdges, sal_Int32& rnObjectLines )
{
    rnRoundedEdges = -1;
    rnObjectLines = -1;
    try
    {
        bool bDifferentRoundedEdges = false;
        bool bDifferentObjectLines = false;

        drawing::LineStyle aLineStyle( drawing::LineStyle_SOLID );

        std::vector< rtl::Reference< DataSeries > > aSeriesList =
            xDiagram->getDataSeries();
        sal_Int32 nSeriesCount = static_cast<sal_Int32>( aSeriesList.size() );

        OUString aPercentDiagonalPropertyName( "PercentDiagonal" );
        OUString aBorderStylePropertyName( "BorderStyle" );

        for( sal_Int32 nS = 0; nS < nSeriesCount; ++nS )
        {
            rtl::Reference< DataSeries > xSeries( aSeriesList[nS] );
            if(!nS)
            {
                rnRoundedEdges = 0;
                try
                {
                    sal_Int16 nPercentDiagonal = 0;

                    xSeries->getPropertyValue( aPercentDiagonalPropertyName ) >>= nPercentDiagonal;
                    rnRoundedEdges = static_cast< sal_Int32 >( nPercentDiagonal );

                    if( DataSeriesHelper::hasAttributedDataPointDifferentValue( xSeries
                        , aPercentDiagonalPropertyName, uno::Any(nPercentDiagonal) ) )
                        bDifferentRoundedEdges = true;
                }
                catch( const uno::Exception& )
                {
                    TOOLS_WARN_EXCEPTION("chart2", "" );
                    bDifferentRoundedEdges = true;
                }
                try
                {
                    xSeries->getPropertyValue( aBorderStylePropertyName ) >>= aLineStyle;

                    if( DataSeriesHelper::hasAttributedDataPointDifferentValue( xSeries
                        , aBorderStylePropertyName, uno::Any(aLineStyle) ) )
                        bDifferentObjectLines = true;
                }
                catch( const uno::Exception& )
                {
                    TOOLS_WARN_EXCEPTION("chart2", "" );
                    bDifferentObjectLines = true;
                }
            }
            else
            {
                if( !bDifferentRoundedEdges )
                {
                    sal_Int16 nPercentDiagonal = 0;
                    xSeries->getPropertyValue( aPercentDiagonalPropertyName ) >>= nPercentDiagonal;
                    sal_Int32 nCurrentRoundedEdges = static_cast< sal_Int32 >( nPercentDiagonal );
                    if(nCurrentRoundedEdges!=rnRoundedEdges
                        || DataSeriesHelper::hasAttributedDataPointDifferentValue( xSeries
                            , aPercentDiagonalPropertyName, uno::Any( static_cast< sal_Int16 >(rnRoundedEdges) ) ) )
                    {
                        bDifferentRoundedEdges = true;
                    }
                }

                if( !bDifferentObjectLines )
                {
                    drawing::LineStyle aCurrentLineStyle;
                    xSeries->getPropertyValue( aBorderStylePropertyName ) >>= aCurrentLineStyle;
                    if(aCurrentLineStyle!=aLineStyle
                        || DataSeriesHelper::hasAttributedDataPointDifferentValue( xSeries
                            , aBorderStylePropertyName, uno::Any(aLineStyle) ) )
                        bDifferentObjectLines = true;
                }
            }
            if( bDifferentRoundedEdges && bDifferentObjectLines )
                break;
        }

        //set rnObjectLines
        rnObjectLines = 0;
        if( bDifferentObjectLines )
            rnObjectLines = -1;
        else if( aLineStyle == drawing::LineStyle_SOLID )
            rnObjectLines = 1;
    }
    catch( const uno::Exception& )
    {
        TOOLS_WARN_EXCEPTION("chart2", "" );
    }
}

void ThreeDHelper::setRoundedEdgesAndObjectLines(
            const rtl::Reference< Diagram > & xDiagram
            , sal_Int32 nRoundedEdges, sal_Int32 nObjectLines )
{
    if( (nRoundedEdges<0||nRoundedEdges>100) && nObjectLines!=0 && nObjectLines!=1 )
        return;

    drawing::LineStyle aLineStyle( drawing::LineStyle_NONE );
    if(nObjectLines==1)
        aLineStyle = drawing::LineStyle_SOLID;

    uno::Any aALineStyle( aLineStyle);
    uno::Any aARoundedEdges( static_cast< sal_Int16 >( nRoundedEdges ));

    std::vector< rtl::Reference< DataSeries > > aSeriesList =
        xDiagram->getDataSeries();
    for( auto const&  xSeries : aSeriesList)
    {
        if( nRoundedEdges>=0 && nRoundedEdges<=100 )
            DataSeriesHelper::setPropertyAlsoToAllAttributedDataPoints( xSeries, "PercentDiagonal", aARoundedEdges );

        if( nObjectLines==0 || nObjectLines==1 )
            DataSeriesHelper::setPropertyAlsoToAllAttributedDataPoints( xSeries, "BorderStyle", aALineStyle );
    }
}

CuboidPlanePosition ThreeDHelper::getAutomaticCuboidPlanePositionForStandardLeftWall( const rtl::Reference< ::chart::Diagram >& xDiagram )
{
    CuboidPlanePosition eRet(CuboidPlanePosition_Left);

    double fXAngleRad=0.0; double fYAngleRad=0.0; double fZAngleRad=0.0;
    xDiagram->getRotationAngle( fXAngleRad, fYAngleRad, fZAngleRad );
    if( lcl_isRightAngledAxesSetAndSupported( xDiagram ) )
    {
        ThreeDHelper::adaptRadAnglesForRightAngledAxes( fXAngleRad, fYAngleRad );
    }
    if( sin(fYAngleRad)>0.0 )
        eRet = CuboidPlanePosition_Right;
    return eRet;
}

CuboidPlanePosition ThreeDHelper::getAutomaticCuboidPlanePositionForStandardBackWall( const rtl::Reference< Diagram >& xDiagram )
{
    CuboidPlanePosition eRet(CuboidPlanePosition_Back);

    double fXAngleRad=0.0; double fYAngleRad=0.0; double fZAngleRad=0.0;
    xDiagram->getRotationAngle( fXAngleRad, fYAngleRad, fZAngleRad );
    if( lcl_isRightAngledAxesSetAndSupported( xDiagram ) )
    {
        ThreeDHelper::adaptRadAnglesForRightAngledAxes( fXAngleRad, fYAngleRad );
    }
    if( cos(fXAngleRad)*cos(fYAngleRad)<0.0 )
        eRet = CuboidPlanePosition_Front;
    return eRet;
}

CuboidPlanePosition ThreeDHelper::getAutomaticCuboidPlanePositionForStandardBottom( const rtl::Reference< Diagram >& xDiagram )
{
    CuboidPlanePosition eRet(CuboidPlanePosition_Bottom);

    double fXAngleRad=0.0; double fYAngleRad=0.0; double fZAngleRad=0.0;
    xDiagram->getRotationAngle( fXAngleRad, fYAngleRad, fZAngleRad );
    if( lcl_isRightAngledAxesSetAndSupported( xDiagram ) )
    {
        ThreeDHelper::adaptRadAnglesForRightAngledAxes( fXAngleRad, fYAngleRad );
    }
    if( sin(fXAngleRad)*cos(fYAngleRad)<0.0 )
        eRet = CuboidPlanePosition_Top;
    return eRet;
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
