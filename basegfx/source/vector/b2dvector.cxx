/*************************************************************************
 *
 *  $RCSfile: b2dvector.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: thb $ $Date: 2003-11-12 12:11:24 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
    namespace vector
    {
        B2DVector& B2DVector::normalize()
        {
            double fLen(scalar(*this));

            if(!::basegfx::numeric::fTools::equalZero(fLen))
            {
                const double fOne(1.0);

                if(!::basegfx::numeric::fTools::equal(fOne, fLen))
                {
                    fLen = sqrt(fLen);

                    if(!::basegfx::numeric::fTools::equalZero(fLen))
                    {
                        mfX /= fLen;
                        mfY /= fLen;
                    }
                }
            }

            return *this;
        }

        B2DVector& B2DVector::operator=( const ::basegfx::tuple::B2DTuple& rVec )
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
            return (const B2DVector&) ::basegfx::tuple::B2DTuple::getEmptyTuple();
        }

        B2DVector& B2DVector::operator*=( const matrix::B2DHomMatrix& rMat )
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

            if(!::basegfx::numeric::fTools::equalZero(fLenNow))
            {
                const double fOne(10.0);

                if(!::basegfx::numeric::fTools::equal(fOne, fLenNow))
                {
                    fLen /= sqrt(fLenNow);
                }

                mfX *= fLen;
                mfY *= fLen;
            }

            return *this;
        }

        sal_Bool B2DVector::isNormalized() const
        {
            const double fOne(1.0);
            const double fScalar(scalar(*this));

            return (::basegfx::numeric::fTools::equal(fOne, fScalar));
        }

        sal_Bool areParallel( const B2DVector& rVecA, const B2DVector& rVecB )
        {
            double fVal(rVecA.getX() * rVecB.getY() - rVecA.getY() * rVecB.getX());
            return ::basegfx::numeric::fTools::equalZero(fVal);
        }

        B2DVectorOrientation getOrientation( const B2DVector& rVecA, const B2DVector& rVecB )
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

        B2DVector getPerpendicular( const B2DVector& rNormalizedVec )
        {
            B2DVector aPerpendicular(-rNormalizedVec.getY(), rNormalizedVec.getX());
            return aPerpendicular;
        }

        B2DVector operator*( const matrix::B2DHomMatrix& rMat, const B2DVector& rVec )
        {
            B2DVector aRes( rVec );
            return aRes*=rMat;
        }

        ::basegfx::vector::B2DVectorContinuity getContinuity(const B2DVector& rBackVector, const B2DVector& rForwardVector )
        {
            ::basegfx::vector::B2DVectorContinuity eRetval(::basegfx::vector::CONTINUITY_NONE);

            if(!rBackVector.equalZero() && !rForwardVector.equalZero())
            {
                const B2DVector aInverseForwardVector(-rForwardVector.getX(), -rForwardVector.getY());

                if(rBackVector.equal(aInverseForwardVector))
                {
                    // same direction and same length -> C2
                    eRetval = ::basegfx::vector::CONTINUITY_C2;
                }
                else if(areParallel(rBackVector, aInverseForwardVector))
                {
                    // same direction -> C1
                    eRetval = ::basegfx::vector::CONTINUITY_C1;
                }
            }

            return eRetval;
        }
    } // end of namespace vector
} // end of namespace basegfx

// eof
