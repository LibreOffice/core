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

#include <basegfx/vector/b3dvector.hxx>
#include <basegfx/matrix/b3dhommatrix.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace basegfx
{
    B3DVector& B3DVector::normalize()
    {
        double fLen(scalar(*this));

        if(!::basegfx::fTools::equalZero(fLen))
        {
            const double fOne(1.0);

            if(!::basegfx::fTools::equal(fOne, fLen))
            {
                fLen = sqrt(fLen);

                if(!::basegfx::fTools::equalZero(fLen))
                {
                    mfX /= fLen;
                    mfY /= fLen;
                    mfZ /= fLen;
                }
            }
        }

        return *this;
    }

    B3DVector B3DVector::getPerpendicular(const B3DVector& rNormalizedVec) const
    {
        B3DVector aNew(*this);
        aNew = cross(aNew, rNormalizedVec);
        aNew.normalize();
        return aNew;
    }

    B3DVector& B3DVector::operator*=( const ::basegfx::B3DHomMatrix& rMat )
    {
        const double fTempX( rMat.get(0,0)*mfX + rMat.get(0,1)*mfY + rMat.get(0,2)*mfZ );
        const double fTempY( rMat.get(1,0)*mfX + rMat.get(1,1)*mfY + rMat.get(1,2)*mfZ );
        const double fTempZ( rMat.get(2,0)*mfX + rMat.get(2,1)*mfY + rMat.get(2,2)*mfZ );
        mfX = fTempX;
        mfY = fTempY;
        mfZ = fTempZ;

        return *this;
    }

    B3DVector operator*( const ::basegfx::B3DHomMatrix& rMat, const B3DVector& rVec )
    {
        B3DVector aRes( rVec );
        return aRes*=rMat;
    }

    bool areParallel( const B3DVector& rVecA, const B3DVector& rVecB )
    {
        // i think fastest is to compare relations, need no square or division
        if(!fTools::equal(rVecA.getX() * rVecB.getY(), rVecA.getY() * rVecB.getX()))
            return false;

        if(!fTools::equal(rVecA.getX() * rVecB.getZ(), rVecA.getZ() * rVecB.getX()))
            return false;

        return (fTools::equal(rVecA.getY() * rVecB.getZ(), rVecA.getZ() * rVecB.getY()));
    }

} // end of namespace basegfx

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
