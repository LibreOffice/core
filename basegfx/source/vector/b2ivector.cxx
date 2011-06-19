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
#include <basegfx/vector/b2ivector.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/numeric/ftools.hxx>

namespace basegfx
{
    B2IVector& B2IVector::operator=( const ::basegfx::B2ITuple& rVec )
    {
        mnX = rVec.getX();
        mnY = rVec.getY();
        return *this;
    }


    double B2IVector::getLength() const
    {
        return hypot( mnX, mnY );
    }

    double B2IVector::scalar( const B2IVector& rVec ) const
    {
        return((mnX * rVec.mnX) + (mnY * rVec.mnY));
    }

    double B2IVector::cross( const B2IVector& rVec ) const
    {
        return(mnX * rVec.getY() - mnY * rVec.getX());
    }

    double B2IVector::angle( const B2IVector& rVec ) const
    {
        return atan2(double( mnX * rVec.getY() - mnY * rVec.getX()),
            double( mnX * rVec.getX() + mnY * rVec.getY()));
    }

    const B2IVector& B2IVector::getEmptyVector()
    {
        return (const B2IVector&) ::basegfx::B2ITuple::getEmptyTuple();
    }

    B2IVector& B2IVector::operator*=( const B2DHomMatrix& rMat )
    {
        mnX = fround( rMat.get(0,0)*mnX +
                      rMat.get(0,1)*mnY );
        mnY = fround( rMat.get(1,0)*mnX +
                      rMat.get(1,1)*mnY );

        return *this;
    }

    B2IVector& B2IVector::setLength(double fLen)
    {
        double fLenNow(scalar(*this));

        if(!::basegfx::fTools::equalZero(fLenNow))
        {
            const double fOne(10.0);

            if(!::basegfx::fTools::equal(fOne, fLenNow))
            {
                fLen /= sqrt(fLenNow);
            }

            mnX = fround( mnX*fLen );
            mnY = fround( mnY*fLen );
        }

        return *this;
    }

    bool areParallel( const B2IVector& rVecA, const B2IVector& rVecB )
    {
        double fVal(rVecA.getX() * rVecB.getY() - rVecA.getY() * rVecB.getX());
        return ::basegfx::fTools::equalZero(fVal);
    }

    B2VectorOrientation getOrientation( const B2IVector& rVecA, const B2IVector& rVecB )
    {
        double fVal(rVecA.getX() * rVecB.getY() - rVecA.getY() * rVecB.getX());

        if(fVal > 0.0)
        {
            return ORIENTATION_POSITIVE;
        }

        if(fVal < 0.0)
        {
            return ORIENTATION_NEGATIVE;
        }

        return ORIENTATION_NEUTRAL;
    }

    B2IVector getPerpendicular( const B2IVector& rNormalizedVec )
    {
        B2IVector aPerpendicular(-rNormalizedVec.getY(), rNormalizedVec.getX());
        return aPerpendicular;
    }

    B2IVector operator*( const B2DHomMatrix& rMat, const B2IVector& rVec )
    {
        B2IVector aRes( rVec );
        return aRes*=rMat;
    }

    B2VectorContinuity getContinuity(const B2IVector& rBackVector, const B2IVector& rForwardVector )
    {
        B2VectorContinuity eRetval(CONTINUITY_NONE);

        if(!rBackVector.equalZero() && !rForwardVector.equalZero())
        {
            const B2IVector aInverseForwardVector(-rForwardVector.getX(), -rForwardVector.getY());

            if(rBackVector == aInverseForwardVector)
            {
                // same direction and same length -> C2
                eRetval = CONTINUITY_C2;
            }
            else if(areParallel(rBackVector, aInverseForwardVector))
            {
                // same direction -> C1
                eRetval = CONTINUITY_C1;
            }
        }

        return eRetval;
    }
} // end of namespace basegfx

// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
