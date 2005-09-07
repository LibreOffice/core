/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: b2ivector.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 20:54:29 $
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

#ifndef _BGFX_VECTOR_B2IVECTOR_HXX
#include <basegfx/vector/b2ivector.hxx>
#endif

#ifndef _BGFX_MATRIX_B2DHOMMATRIX_HXX
#include <basegfx/matrix/b2dhommatrix.hxx>
#endif

#ifndef _BGFX_NUMERIC_FTOOLS_HXX
#include <basegfx/numeric/ftools.hxx>
#endif

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
