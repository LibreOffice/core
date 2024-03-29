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

#include <basegfx/vector/b2dvector.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/numeric/ftools.hxx>

namespace basegfx
{
    B2DVector& B2DVector::normalize()
    {
        double fLen(std::hypot(mnX, mnY));

        if(!fTools::equalZero(fLen))
        {
            const double fOne(1.0);

            if(!fTools::equal(fOne, fLen))
            {
                mnX /= fLen;
                mnY /= fLen;
            }
        }
        else
        {
            mnX = 0.0;
            mnY = 0.0;
        }

        return *this;
    }

    double B2DVector::getLength() const
    {
        if(fTools::equalZero(mnX))
        {
            return fabs(mnY);
        }
        else if(fTools::equalZero(mnY))
        {
            return fabs(mnX);
        }

        return hypot( mnX, mnY );
    }

    double B2DVector::angle( const B2DVector& rVec ) const
    {
        return atan2(mnX * rVec.getY() - mnY * rVec.getX(),
            mnX * rVec.getX() + mnY * rVec.getY());
    }

    const B2DVector& B2DVector::getEmptyVector()
    {
        return static_cast<const B2DVector&>( B2DTuple::getEmptyTuple() );
    }

    B2DVector& B2DVector::operator*=( const B2DHomMatrix& rMat )
    {
        const double fTempX( rMat.get(0,0)*mnX +
                            rMat.get(0,1)*mnY );
        const double fTempY( rMat.get(1,0)*mnX +
                            rMat.get(1,1)*mnY );
        mnX = fTempX;
        mnY = fTempY;

        return *this;
    }

    B2DVector& B2DVector::setLength(double fLen)
    {
        double fLenNow(std::hypot(mnX, mnY));

        if(!fTools::equalZero(fLenNow))
        {
            const double fOne(1.0);

            if(!fTools::equal(fOne, fLenNow))
            {
                fLen /= fLenNow;
            }

            mnX *= fLen;
            mnY *= fLen;
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
            return B2VectorOrientation::Neutral;
        }

        if(fVal > 0.0)
        {
            return B2VectorOrientation::Positive;
        }
        else
        {
            return B2VectorOrientation::Negative;
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
        aRes *= rMat;
        return aRes;
    }

    B2VectorContinuity getContinuity(const B2DVector& rBackVector, const B2DVector& rForwardVector )
    {
        if(rBackVector.equalZero() || rForwardVector.equalZero())
        {
            return B2VectorContinuity::NONE;
        }

        if(fTools::equal(rBackVector.getX(), -rForwardVector.getX()) && fTools::equal(rBackVector.getY(), -rForwardVector.getY()))
        {
            // same direction and same length -> C2
            return B2VectorContinuity::C2;
        }

        if(areParallel(rBackVector, rForwardVector) && rBackVector.scalar(rForwardVector) < 0.0)
        {
            // parallel and opposite direction -> C1
            return B2VectorContinuity::C1;
        }

        return B2VectorContinuity::NONE;
    }
} // end of namespace basegfx

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
