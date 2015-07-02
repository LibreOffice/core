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

#ifndef INCLUDED_TOOLS_VECTOR2D_HXX
#define INCLUDED_TOOLS_VECTOR2D_HXX

#include <math.h>
#include <tools/gen.hxx>

class Vector2D
{
private:
    double              mfX;
    double              mfY;

public:
    inline Vector2D() : mfX( 0.0 ), mfY( 0.0 ) {}
    inline Vector2D( double fX, double fY ) : mfX( fX ), mfY( fY ) {}
    inline Vector2D( const Vector2D& rVec ) : mfX( rVec.mfX ), mfY( rVec.mfY ) {}
    inline Vector2D( const Pair& rPair ) : mfX( rPair.A() ), mfY( rPair.B() ) {};
    inline ~Vector2D() {}

    inline const double& operator[] (int nPos) const { return (nPos ? mfY : mfX); }
    inline double& operator[] (int nPos) { return (nPos ? mfY : mfX); }

    inline double GetLength() const { return hypot( mfX, mfY ); }
    inline Vector2D& Normalize();

    inline Vector2D&    operator+=( const Vector2D& rVec ) { mfX += rVec.mfX, mfY += rVec.mfY; return *this; }
    inline Vector2D&    operator-=( const Vector2D& rVec ) { mfX -= rVec.mfX, mfY -= rVec.mfY; return *this; }
    inline Vector2D     operator+(const Vector2D& rVec) const { Vector2D aSum(*this); aSum += rVec; return aSum; }
    inline Vector2D     operator-(const Vector2D& rVec) const { Vector2D aSub(*this); aSub -= rVec; return aSub; }
    inline Vector2D     operator-(void) const { return Vector2D(-mfX, -mfY); }

    inline double       Scalar( const Vector2D& rVec ) const { return( mfX * rVec.mfX + mfY * rVec.mfY ); }

    inline Vector2D&    operator/=( const Vector2D& rVec ) { mfX /= rVec.mfX, mfY /= rVec.mfY; return *this; }
    inline Vector2D&    operator*=( const Vector2D& rVec ) { mfX *= rVec.mfX, mfY *= rVec.mfY; return *this; }
    inline Vector2D     operator/(const Vector2D& rVec) const { Vector2D aDiv(*this); aDiv /= rVec; return aDiv; }
    inline Vector2D     operator*(const Vector2D& rVec) const { Vector2D aMul(*this); aMul *= rVec; return aMul; }

    inline Vector2D&    operator*=(double t) { mfX *= t; mfY *= t; return *this; }
    inline Vector2D     operator*(double t) const { Vector2D aNew(*this); aNew *= t; return aNew; }
    inline Vector2D&    operator/=(double t) { mfX /= t; mfY /= t; return *this; }
    inline Vector2D     operator/(double t) const { Vector2D aNew(*this); aNew /= t; return aNew; }

    inline bool         operator==( const Vector2D& rVec ) const { return( mfX == rVec.mfX && mfY == rVec.mfY ); }
    inline bool         operator!=( const Vector2D& rVec ) const { return !( *this == rVec ); }

    inline Vector2D&    operator=( const Vector2D& rVec ) { mfX = rVec.mfX, mfY = rVec.mfY; return *this; }
    inline Vector2D&    operator=( const Pair& rPair ) { mfX = rPair.A(), mfY = rPair.B(); return *this; }
    inline Vector2D&    operator-=( const Pair& rPair ) { mfX -= rPair.A(), mfY -= rPair.B(); return *this; }
    inline Vector2D&    operator+=( const Pair& rPair ) { mfX += rPair.A(), mfY += rPair.B(); return *this; }
    inline Vector2D&    operator*=( const Pair& rPair ) { mfX *= rPair.A(), mfY *= rPair.B(); return *this; }
    inline Vector2D&    operator/=( const Pair& rPair ) { mfX /= rPair.A(), mfY /= rPair.B(); return *this; }

    inline bool         operator==( const Pair& rPair ) const { return( mfX == rPair.A() && mfY == rPair.B() ); }
    inline bool         operator!=( const Pair& rPair ) const { return !( *this == rPair ); }

    inline bool         IsPositive( Vector2D& rVec ) const { return( ( mfX * rVec.mfY - mfY * rVec.mfX ) >= 0.0 ); }
    inline bool         IsNegative( Vector2D& rVec ) const { return !IsPositive( rVec ); }
};

inline Vector2D& Vector2D::Normalize()
{
    double fLen = Scalar( *this );

    if( ( fLen != 0.0 ) && ( fLen != 1.0 ) && ( ( fLen = sqrt( fLen ) ) != 0.0 ) )
        mfX /= fLen, mfY /= fLen;

    return *this;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
