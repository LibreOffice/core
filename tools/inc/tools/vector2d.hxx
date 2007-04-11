/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vector2d.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 20:21:28 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _VECTOR2D_HXX
#define _VECTOR2D_HXX

#include <math.h>
#include <tools/gen.hxx>

// ------------
// - Vector2D -
// ------------

class Vector2D
{
private:
    double                      mfX;
    double                      mfY;

public:
    inline Vector2D() : mfX( 0.0 ), mfY( 0.0 ) {}
    inline Vector2D( double fX, double fY ) : mfX( fX ), mfY( fY ) {}
    inline Vector2D( const Vector2D& rVec ) : mfX( rVec.mfX ), mfY( rVec.mfY ) {}
    inline Vector2D( const Pair& rPair ) : mfX( rPair.nA ), mfY( rPair.nB ) {};
    inline ~Vector2D() {}

    inline const double& X() const { return mfX; }
    inline const double& Y() const { return mfY; }
    inline double& X() { return mfX; }
    inline double& Y() { return mfY; }
    inline const double& operator[] (int nPos) const { return (nPos ? mfY : mfX); }
    inline double& operator[] (int nPos) { return (nPos ? mfY : mfX); }

    inline double GetLength() const { return hypot( mfX, mfY ); }
    inline Vector2D& Normalize();

    inline void Min(const Vector2D& rVec) { if(rVec.mfX < mfX) mfX = rVec.mfX; if(rVec.mfY < mfY) mfY = rVec.mfY; }
    inline void Max(const Vector2D& rVec) { if(rVec.mfX > mfX) mfX = rVec.mfX; if(rVec.mfY > mfY) mfY = rVec.mfY; }
    inline void Abs() { if(mfX < 0.0) mfX = -mfX; if(mfY < 0.0) mfY = -mfY; }

    inline void CalcInBetween(Vector2D& rOld1, Vector2D& rOld2, double t)
        { mfX = ((rOld2.mfX - rOld1.mfX) + t) + rOld1.mfX; mfY = ((rOld2.mfY - rOld1.mfY) + t) + rOld1.mfY; }
    inline void CalcMiddle(Vector2D& rOld1, Vector2D& rOld2)
        { mfX = (rOld1.mfX + rOld2.mfX) / 2.0; mfY = (rOld1.mfY + rOld2.mfY) / 2.0; }
    inline void CalcMiddle(Vector2D& rOld1, Vector2D& rOld2, Vector2D& rOld3)
        { mfX = (rOld1.mfX + rOld2.mfX + rOld3.mfX) / 3.0; mfY = (rOld1.mfY + rOld2.mfY + rOld3.mfY) / 3.0; }

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

    inline BOOL         operator==( const Vector2D& rVec ) const { return( mfX == rVec.mfX && mfY == rVec.mfY ); }
    inline BOOL         operator!=( const Vector2D& rVec ) const { return !( *this == rVec ); }

    inline Vector2D&    operator=( const Vector2D& rVec ) { mfX = rVec.mfX, mfY = rVec.mfY; return *this; }
    inline Vector2D&    operator=( const Pair& rPair ) { mfX = rPair.nA, mfY = rPair.nB; return *this; }
    inline Vector2D&    operator-=( const Pair& rPair ) { mfX -= rPair.nA, mfY -= rPair.nB; return *this; }
    inline Vector2D&    operator+=( const Pair& rPair ) { mfX += rPair.nA, mfY += rPair.nB; return *this; }
    inline Vector2D&    operator*=( const Pair& rPair ) { mfX *= rPair.nA, mfY *= rPair.nB; return *this; }
    inline Vector2D&    operator/=( const Pair& rPair ) { mfX /= rPair.nA, mfY /= rPair.nB; return *this; }

    inline BOOL         operator==( const Pair& rPair ) const { return( mfX == rPair.nA && mfY == rPair.nB ); }
    inline BOOL         operator!=( const Pair& rPair ) const { return !( *this == rPair ); }

    inline BOOL         IsPositive( Vector2D& rVec ) const { return( ( mfX * rVec.mfY - mfY * rVec.mfX ) >= 0.0 ); }
    inline BOOL         IsNegative( Vector2D& rVec ) const { return !IsPositive( rVec ); }
};

// -----------------------------------------------------------------------------

inline Vector2D& Vector2D::Normalize()
{
    double fLen = Scalar( *this );

    if( ( fLen != 0.0 ) && ( fLen != 1.0 ) && ( ( fLen = sqrt( fLen ) ) != 0.0 ) )
        mfX /= fLen, mfY /= fLen;

    return *this;
}

#endif //  _SV_VECTOR2D_HXX
