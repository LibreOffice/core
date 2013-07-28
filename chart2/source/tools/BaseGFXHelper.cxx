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

#include "BaseGFXHelper.hxx"
#include <com/sun/star/drawing/DoubleSequence.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::drawing;
using namespace ::basegfx;

namespace chart
{
namespace BaseGFXHelper
{

::basegfx::B3DRange getBoundVolume( const drawing::PolyPolygonShape3D& rPolyPoly )
{
    ::basegfx::B3DRange aRet;

    bool bInited = false;
    sal_Int32 nPolyCount = rPolyPoly.SequenceX.getLength();
    for(sal_Int32 nPoly = 0; nPoly < nPolyCount; nPoly++)
    {
        sal_Int32 nPointCount = rPolyPoly.SequenceX[nPoly].getLength();
        for( sal_Int32 nPoint = 0; nPoint < nPointCount; nPoint++)
        {
            if(!bInited)
            {
                aRet = ::basegfx::B3DRange(::basegfx::B3DTuple(
                          rPolyPoly.SequenceX[nPoly][nPoint]
                        , rPolyPoly.SequenceY[nPoly][nPoint]
                        , rPolyPoly.SequenceZ[nPoly][nPoint]));
                bInited = true;
            }
            else
            {
                aRet.expand( ::basegfx::B3DTuple(
                          rPolyPoly.SequenceX[nPoly][nPoint]
                        , rPolyPoly.SequenceY[nPoly][nPoint]
                        , rPolyPoly.SequenceZ[nPoly][nPoint]));
            }
        }
    }

    return aRet;
}

B2IRectangle makeRectangle( const awt::Point& rPos, const awt::Size& rSize )
{
    return B2IRectangle(rPos.X,rPos.Y,rPos.X+rSize.Width,rPos.Y+rSize.Height);
}

awt::Point B2IRectangleToAWTPoint( const ::basegfx::B2IRectangle& rB2IRectangle )
{
    return awt::Point( rB2IRectangle.getMinX(), rB2IRectangle.getMinY() );
}

awt::Size B2IRectangleToAWTSize( const ::basegfx::B2IRectangle& rB2IRectangle )
{
    return awt::Size( static_cast< sal_Int32 >( rB2IRectangle.getWidth()),
                      static_cast< sal_Int32 >( rB2IRectangle.getHeight()));
}

awt::Rectangle B2IRectangleToAWTRectangle(
    const ::basegfx::B2IRectangle& rB2IRectangle )
{
    return awt::Rectangle( rB2IRectangle.getMinX(), rB2IRectangle.getMinY(),
                           static_cast< sal_Int32 >( rB2IRectangle.getWidth()),
                           static_cast< sal_Int32 >( rB2IRectangle.getHeight()));
}

B3DVector Direction3DToB3DVector( const Direction3D& rDirection )
{
    return B3DVector(
          rDirection.DirectionX
        , rDirection.DirectionY
        , rDirection.DirectionZ
        );
}

Direction3D B3DVectorToDirection3D( const B3DVector& rB3DVector )
{
    return Direction3D(
          rB3DVector.getX()
        , rB3DVector.getY()
        , rB3DVector.getZ()
        );
}

B3DVector Position3DToB3DVector( const Position3D& rPosition )
{
    return B3DVector(
          rPosition.PositionX
        , rPosition.PositionY
        , rPosition.PositionZ
        );
}

Position3D B3DVectorToPosition3D( const B3DVector& rB3DVector )
{
    return Position3D(
          rB3DVector.getX()
        , rB3DVector.getY()
        , rB3DVector.getZ()
        );
}

B3DHomMatrix HomogenMatrixToB3DHomMatrix( const HomogenMatrix & rHomogenMatrix )
{
    B3DHomMatrix aResult;

    aResult.set( 0, 0, rHomogenMatrix.Line1.Column1 );
    aResult.set( 0, 1, rHomogenMatrix.Line1.Column2 );
    aResult.set( 0, 2, rHomogenMatrix.Line1.Column3 );
    aResult.set( 0, 3, rHomogenMatrix.Line1.Column4 );

    aResult.set( 1, 0, rHomogenMatrix.Line2.Column1 );
    aResult.set( 1, 1, rHomogenMatrix.Line2.Column2 );
    aResult.set( 1, 2, rHomogenMatrix.Line2.Column3 );
    aResult.set( 1, 3, rHomogenMatrix.Line2.Column4 );

    aResult.set( 2, 0, rHomogenMatrix.Line3.Column1 );
    aResult.set( 2, 1, rHomogenMatrix.Line3.Column2 );
    aResult.set( 2, 2, rHomogenMatrix.Line3.Column3 );
    aResult.set( 2, 3, rHomogenMatrix.Line3.Column4 );

    aResult.set( 3, 0, rHomogenMatrix.Line4.Column1 );
    aResult.set( 3, 1, rHomogenMatrix.Line4.Column2 );
    aResult.set( 3, 2, rHomogenMatrix.Line4.Column3 );
    aResult.set( 3, 3, rHomogenMatrix.Line4.Column4 );

    return aResult;
}

HomogenMatrix B3DHomMatrixToHomogenMatrix( const B3DHomMatrix & rB3DMatrix )
{
    HomogenMatrix aResult;

    aResult.Line1.Column1 = rB3DMatrix.get( 0, 0 );
    aResult.Line1.Column2 = rB3DMatrix.get( 0, 1 );
    aResult.Line1.Column3 = rB3DMatrix.get( 0, 2 );
    aResult.Line1.Column4 = rB3DMatrix.get( 0, 3 );

    aResult.Line2.Column1 = rB3DMatrix.get( 1, 0 );
    aResult.Line2.Column2 = rB3DMatrix.get( 1, 1 );
    aResult.Line2.Column3 = rB3DMatrix.get( 1, 2 );
    aResult.Line2.Column4 = rB3DMatrix.get( 1, 3 );

    aResult.Line3.Column1 = rB3DMatrix.get( 2, 0 );
    aResult.Line3.Column2 = rB3DMatrix.get( 2, 1 );
    aResult.Line3.Column3 = rB3DMatrix.get( 2, 2 );
    aResult.Line3.Column4 = rB3DMatrix.get( 2, 3 );

    aResult.Line4.Column1 = rB3DMatrix.get( 3, 0 );
    aResult.Line4.Column2 = rB3DMatrix.get( 3, 1 );
    aResult.Line4.Column3 = rB3DMatrix.get( 3, 2 );
    aResult.Line4.Column4 = rB3DMatrix.get( 3, 3 );

    return aResult;
}

B3DTuple GetRotationFromMatrix( const B3DHomMatrix & rB3DMatrix )
{
    B3DTuple aScale, aTranslation, aRotation, aShearing;
    rB3DMatrix.decompose( aScale, aTranslation, aRotation, aShearing );
    return aRotation;
}

B3DTuple GetScaleFromMatrix( const B3DHomMatrix & rB3DMatrix )
{
    B3DTuple aScale, aTranslation, aRotation, aShearing;
    rB3DMatrix.decompose( aScale, aTranslation, aRotation, aShearing );
    return aScale;
}

void ReduceToRotationMatrix( ::basegfx::B3DHomMatrix & rB3DMatrix )
{
    B3DTuple aR( GetRotationFromMatrix( rB3DMatrix ) );
    ::basegfx::B3DHomMatrix aRotationMatrix;
    aRotationMatrix.rotate(aR.getX(),aR.getY(),aR.getZ());
    rB3DMatrix = aRotationMatrix;
}

double Deg2Rad( double fDegrees )
{
    return fDegrees * ( F_PI / 180.0 );
}

double Rad2Deg( double fRadians )
{
    return fRadians * ( 180.0 / F_PI );
}

} //  namespace BaseGFXHelper
} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
