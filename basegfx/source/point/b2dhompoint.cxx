/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_basegfx.hxx"
#include <basegfx/point/b2dhompoint.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/numeric/ftools.hxx>

namespace basegfx
{
    bool B2DHomPoint::implIsHomogenized() const
    {
        const double fOne(1.0);
        return ::basegfx::fTools::equal(fOne, mfW);
    }

    void B2DHomPoint::implHomogenize()
    {
        const double fFactor(1.0 / mfW);
        maTuple.setX(maTuple.getX() * fFactor);
        maTuple.setY(maTuple.getY() * fFactor);
        mfW = 1.0;
    }

    void B2DHomPoint::implTestAndHomogenize() const
    {
        if(!implIsHomogenized())
            ((B2DHomPoint*)this)->implHomogenize();
    }

    B2DPoint B2DHomPoint::getB2DPoint() const
    {
        implTestAndHomogenize();
        return B2DPoint(maTuple.getX(), maTuple.getY());
    }

    double B2DHomPoint::getX() const
    {
        implTestAndHomogenize();
        return maTuple.getX();
    }

    double B2DHomPoint::getY() const
    {
        implTestAndHomogenize();
        return maTuple.getY();
    }

    void B2DHomPoint::setX(double fX)
    {
        maTuple.setX(implIsHomogenized() ? fX : fX * mfW );
    }

    void B2DHomPoint::setY(double fY)
    {
        maTuple.setY(implIsHomogenized() ? fY : fY * mfW );
    }

    B2DHomPoint& B2DHomPoint::operator+=( const B2DHomPoint& rPnt )
    {
        maTuple.setX(getX() * rPnt.mfW + rPnt.getX() * mfW);
        maTuple.setY(getY() * rPnt.mfW + rPnt.getY() * mfW);
        mfW = mfW * rPnt.mfW;

        return *this;
    }

    B2DHomPoint& B2DHomPoint::operator-=( const B2DHomPoint& rPnt )
    {
        maTuple.setX(getX() * rPnt.mfW - rPnt.getX() * mfW);
        maTuple.setY(getY() * rPnt.mfW - rPnt.getY() * mfW);
        mfW = mfW * rPnt.mfW;

        return *this;
    }

    B2DHomPoint& B2DHomPoint::operator*=(double t)
    {
        if(!::basegfx::fTools::equalZero(t))
        {
            mfW /= t;
        }

        return *this;
    }

    B2DHomPoint& B2DHomPoint::operator*=( const B2DHomMatrix& rMat )
    {
        const double fTempX( rMat.get(0,0)*maTuple.getX() +
                            rMat.get(0,1)*maTuple.getY() +
                            rMat.get(0,2)*mfW );

        const double fTempY( rMat.get(1,0)*maTuple.getX() +
                            rMat.get(1,1)*maTuple.getY() +
                            rMat.get(1,2)*mfW );

        const double fTempZ( rMat.get(2,0)*maTuple.getX() +
                            rMat.get(2,1)*maTuple.getY() +
                            rMat.get(2,2)*mfW );
        maTuple.setX( fTempX );
        maTuple.setY( fTempY );
        mfW = fTempZ;

        return *this;
    }

    B2DHomPoint& B2DHomPoint::operator/=(double t)
    {
        mfW *= t;
        return *this;
    }

    B2DHomPoint& B2DHomPoint::operator-(void)
    {
        mfW = -mfW;
        return *this;
    }

    bool B2DHomPoint::operator==( const B2DHomPoint& rPnt ) const
    {
        implTestAndHomogenize();
        return (maTuple == rPnt.maTuple);
    }

    bool B2DHomPoint::operator!=( const B2DHomPoint& rPnt ) const
    {
        implTestAndHomogenize();
        return (maTuple != rPnt.maTuple);
    }

    B2DHomPoint& B2DHomPoint::operator=( const B2DHomPoint& rPnt )
    {
        maTuple = rPnt.maTuple;
        mfW = rPnt.mfW;
        return *this;
    }

    B2DHomPoint minimum(const B2DHomPoint& rVecA, const B2DHomPoint& rVecB)
    {
        B2DHomPoint aMin(
            (rVecB.getX() < rVecA.getX()) ? rVecB.getX() : rVecA.getX(),
            (rVecB.getY() < rVecA.getY()) ? rVecB.getY() : rVecA.getY());
        return aMin;
    }

    B2DHomPoint maximum(const B2DHomPoint& rVecA, const B2DHomPoint& rVecB)
    {
        B2DHomPoint aMax(
            (rVecB.getX() > rVecA.getX()) ? rVecB.getX() : rVecA.getX(),
            (rVecB.getY() > rVecA.getY()) ? rVecB.getY() : rVecA.getY());
        return aMax;
    }
    B2DHomPoint absolute(const B2DHomPoint& rVec)
    {
        B2DHomPoint aAbs(
            (0.0 > rVec.getX()) ? -rVec.getX() : rVec.getX(),
            (0.0 > rVec.getY()) ? -rVec.getY() : rVec.getY());
        return aAbs;
    }

    B2DHomPoint interpolate(B2DHomPoint& rOld1, B2DHomPoint& rOld2, double t)
    {
        B2DHomPoint aInt(
            ((rOld2.getX() - rOld1.getX()) * t) + rOld1.getX(),
            ((rOld2.getY() - rOld1.getY()) * t) + rOld1.getY());
        return aInt;
    }

    B2DHomPoint average(B2DHomPoint& rOld1, B2DHomPoint& rOld2)
    {
        B2DHomPoint aAvg(
            (rOld1.getX() + rOld2.getX()) * 0.5,
            (rOld1.getY() + rOld2.getY()) * 0.5);
        return aAvg;
    }

    B2DHomPoint average(B2DHomPoint& rOld1, B2DHomPoint& rOld2, B2DHomPoint& rOld3)
    {
        B2DHomPoint aAvg(
            (rOld1.getX() + rOld2.getX() + rOld3.getX()) * (1.0 / 3.0),
            (rOld1.getY() + rOld2.getY() + rOld3.getY()) * (1.0 / 3.0));
        return aAvg;
    }

    B2DHomPoint operator+(const B2DHomPoint& rVecA, const B2DHomPoint& rVecB)
    {
        B2DHomPoint aSum(rVecA);
        aSum += rVecB;
        return aSum;
    }

    B2DHomPoint operator-(const B2DHomPoint& rVecA, const B2DHomPoint& rVecB)
    {
        B2DHomPoint aSub(rVecA);
        aSub -= rVecB;
        return aSub;
    }

    B2DHomPoint operator*(const B2DHomPoint& rVec, double t)
    {
        B2DHomPoint aNew(rVec);
        aNew *= t;
        return aNew;
    }

    B2DHomPoint operator*(double t, const B2DHomPoint& rVec)
    {
        B2DHomPoint aNew(rVec);
        aNew *= t;
        return aNew;
    }

    B2DHomPoint operator*( const B2DHomMatrix& rMat, const B2DHomPoint& rPoint )
    {
        B2DHomPoint aNew(rPoint);
        return aNew*=rMat;
    }

    B2DHomPoint operator/(const B2DHomPoint& rVec, double t)
    {
        B2DHomPoint aNew(rVec);
        aNew /= t;
        return aNew;
    }

    B2DHomPoint operator/(double t, const B2DHomPoint& rVec)
    {
        B2DHomPoint aNew(rVec);
        aNew /= t;
        return aNew;
    }
} // end of namespace basegfx

// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
