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

#include <basegfx/vector/b3dvector.hxx>
#include <basegfx/matrix/b3dhommatrix.hxx>



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
