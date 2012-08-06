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

#include <basegfx/vector/b2dvector.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/numeric/ftools.hxx>

namespace basegfx
{
    B2DVector& B2DVector::normalize()
    {
        double fLen(scalar(*this));

        if(fTools::equalZero(fLen))
        {
            mfX = 0.0;
            mfY = 0.0;
        }
        else
        {
            const double fOne(1.0);

            if(!fTools::equal(fOne, fLen))
            {
                fLen = sqrt(fLen);

                if(!fTools::equalZero(fLen))
                {
                    mfX /= fLen;
                    mfY /= fLen;
                }
            }
        }

        return *this;
    }

    B2DVector& B2DVector::operator=( const B2DTuple& rVec )
    {
        mfX = rVec.getX();
        mfY = rVec.getY();
        return *this;
    }


    double B2DVector::getLength() const
    {
        if(fTools::equalZero(mfX))
        {
            return fabs(mfY);
        }
        else if(fTools::equalZero(mfY))
        {
            return fabs(mfX);
        }

        return hypot( mfX, mfY );
    }

    double B2DVector::scalar( const B2DVector& rVec ) const
    {
        return((mfX * rVec.mfX) + (mfY * rVec.mfY));
    }

    double B2DVector::cross( const B2DVector& rVec ) const
    {
        return(mfX * rVec.getY() - mfY * rVec.getX());
    }

    double B2DVector::angle( const B2DVector& rVec ) const
    {
        return atan2(mfX * rVec.getY() - mfY * rVec.getX(),
            mfX * rVec.getX() + mfY * rVec.getY());
    }

    const B2DVector& B2DVector::getEmptyVector()
    {
        return (const B2DVector&) B2DTuple::getEmptyTuple();
    }

    B2DVector& B2DVector::operator*=( const B2DHomMatrix& rMat )
    {
        const double fTempX( rMat.get(0,0)*mfX +
                            rMat.get(0,1)*mfY );
        const double fTempY( rMat.get(1,0)*mfX +
                            rMat.get(1,1)*mfY );
        mfX = fTempX;
        mfY = fTempY;

        return *this;
    }

    B2DVector& B2DVector::setLength(double fLen)
    {
        double fLenNow(scalar(*this));

        if(!fTools::equalZero(fLenNow))
        {
            const double fOne(10.0);

            if(!fTools::equal(fOne, fLenNow))
            {
                fLen /= sqrt(fLenNow);
            }

            mfX *= fLen;
            mfY *= fLen;
        }

        return *this;
    }

    bool areParallel( const B2DVector& rVecA, const B2DVector& rVecB )
    {
        const double fValA(rVecA.getX() * rVecB.getY());
        const double fValB(rVecA.getY() * rVecB.getX());

        return fTools::equal(fValA, fValB);
    }

    B2VectorOrientation getOrientation( const B2DVector& rVecA, const B2DVector& rVecB )
    {
        double fVal(rVecA.getX() * rVecB.getY() - rVecA.getY() * rVecB.getX());

        if(fTools::equalZero(fVal))
        {
            return ORIENTATION_NEUTRAL;
        }

        if(fVal > 0.0)
        {
            return ORIENTATION_POSITIVE;
        }
        else
        {
            return ORIENTATION_NEGATIVE;
        }
    }

    B2DVector getPerpendicular( const B2DVector& rNormalizedVec )
    {
        B2DVector aPerpendicular(-rNormalizedVec.getY(), rNormalizedVec.getX());
        return aPerpendicular;
    }

    B2DVector getNormalizedPerpendicular( const B2DVector& rVec )
    {
        B2DVector aPerpendicular(rVec);
        aPerpendicular.normalize();
        const double aTemp(-aPerpendicular.getY());
        aPerpendicular.setY(aPerpendicular.getX());
        aPerpendicular.setX(aTemp);
        return aPerpendicular;
    }

    B2DVector operator*( const B2DHomMatrix& rMat, const B2DVector& rVec )
    {
        B2DVector aRes( rVec );
        return aRes*=rMat;
    }

    B2VectorContinuity getContinuity(const B2DVector& rBackVector, const B2DVector& rForwardVector )
    {
        if(rBackVector.equalZero() || rForwardVector.equalZero())
        {
            return CONTINUITY_NONE;
        }

        if(fTools::equal(rBackVector.getX(), -rForwardVector.getX()) && fTools::equal(rBackVector.getY(), -rForwardVector.getY()))
        {
            // same direction and same length -> C2
            return CONTINUITY_C2;
        }

        if(areParallel(rBackVector, rForwardVector) && rBackVector.scalar(rForwardVector) < 0.0)
        {
            // parallel and opposite direction -> C1
            return CONTINUITY_C1;
        }

        return CONTINUITY_NONE;
    }
} // end of namespace basegfx

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
