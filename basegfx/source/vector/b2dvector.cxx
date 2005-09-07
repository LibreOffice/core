/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: b2dvector.cxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 20:54:06 $
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

#ifndef _BGFX_VECTOR_B2DVECTOR_HXX
#include <basegfx/vector/b2dvector.hxx>
#endif

#ifndef _BGFX_MATRIX_B2DHOMMATRIX_HXX
#include <basegfx/matrix/b2dhommatrix.hxx>
#endif

#ifndef _BGFX_NUMERIC_FTOOLS_HXX
#include <basegfx/numeric/ftools.hxx>
#endif

namespace basegfx
{
    B2DVector& B2DVector::normalize()
    {
        double fLen(scalar(*this));

        if(::basegfx::fTools::equalZero(fLen))
        {
            mfX = 0.0;
            mfY = 0.0;
        }
        else
        {
            const double fOne(1.0);

            if(!::basegfx::fTools::equal(fOne, fLen))
            {
                fLen = sqrt(fLen);

                if(!::basegfx::fTools::equalZero(fLen))
                {
                    mfX /= fLen;
                    mfY /= fLen;
                }
            }
        }

        return *this;
    }

    B2DVector& B2DVector::operator=( const ::basegfx::B2DTuple& rVec )
    {
        mfX = rVec.getX();
        mfY = rVec.getY();
        return *this;
    }


    double B2DVector::getLength() const
    {
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
        return (const B2DVector&) ::basegfx::B2DTuple::getEmptyTuple();
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

        if(!::basegfx::fTools::equalZero(fLenNow))
        {
            const double fOne(10.0);

            if(!::basegfx::fTools::equal(fOne, fLenNow))
            {
                fLen /= sqrt(fLenNow);
            }

            mfX *= fLen;
            mfY *= fLen;
        }

        return *this;
    }

    bool B2DVector::isNormalized() const
    {
        const double fOne(1.0);
        const double fScalar(scalar(*this));

        return (::basegfx::fTools::equal(fOne, fScalar));
    }

    bool areParallel( const B2DVector& rVecA, const B2DVector& rVecB )
    {
        double fVal(rVecA.getX() * rVecB.getY() - rVecA.getY() * rVecB.getX());
        return ::basegfx::fTools::equalZero(fVal);
    }

    B2VectorOrientation getOrientation( const B2DVector& rVecA, const B2DVector& rVecB )
    {
        double fVal(rVecA.getX() * rVecB.getY() - rVecA.getY() * rVecB.getX());

        if(::basegfx::fTools::equalZero(fVal))
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

    B2DVector operator*( const ::basegfx::B2DHomMatrix& rMat, const B2DVector& rVec )
    {
        B2DVector aRes( rVec );
        return aRes*=rMat;
    }

    B2VectorContinuity getContinuity(const B2DVector& rBackVector, const B2DVector& rForwardVector )
    {
        B2VectorContinuity eRetval(::basegfx::CONTINUITY_NONE);

        if(!rBackVector.equalZero() && !rForwardVector.equalZero())
        {
            const B2DVector aInverseForwardVector(-rForwardVector.getX(), -rForwardVector.getY());

            if(rBackVector.equal(aInverseForwardVector))
            {
                // same direction and same length -> C2
                eRetval = ::basegfx::CONTINUITY_C2;
            }
            else if(areParallel(rBackVector, aInverseForwardVector))
            {
                // same direction -> C1
                eRetval = ::basegfx::CONTINUITY_C1;
            }
        }

        return eRetval;
    }
} // end of namespace basegfx

// eof
