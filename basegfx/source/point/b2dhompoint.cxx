/*************************************************************************
 *
 *  $RCSfile: b2dhompoint.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: aw $ $Date: 2003-10-28 11:26:13 $
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

#ifndef _BGFX_POINT_B2DHOMPOINT_HXX
#include <basegfx/inc/point/b2dhompoint.hxx>
#endif

#ifndef _BGFX_MATRIX_B2DHOMMATRIX_HXX
#include <basegfx/inc/matrix/b2dhommatrix.hxx>
#endif

#ifndef _BGFX_NUMERIC_FTOOLS_HXX
#include <basegfx/inc/numeric/ftools.hxx>
#endif

namespace basegfx
{
    namespace point
    {
        bool B2DHomPoint::implIsHomogenized() const
        {
            const double fOne(1.0);
            return ::basegfx::numeric::fTools::equal(fOne, mfW);
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
            if(!::basegfx::numeric::fTools::equalZero(t))
            {
                mfW /= t;
            }

            return *this;
        }

        B2DHomPoint& B2DHomPoint::operator*=( const matrix::B2DHomMatrix& rMat )
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

        B2DHomPoint min(const B2DHomPoint& rVecA, const B2DHomPoint& rVecB)
        {
            B2DHomPoint aMin(
                (rVecB.getX() < rVecA.getX()) ? rVecB.getX() : rVecA.getX(),
                (rVecB.getY() < rVecA.getY()) ? rVecB.getY() : rVecA.getY());
            return aMin;
        }

        B2DHomPoint max(const B2DHomPoint& rVecA, const B2DHomPoint& rVecB)
        {
            B2DHomPoint aMax(
                (rVecB.getX() > rVecA.getX()) ? rVecB.getX() : rVecA.getX(),
                (rVecB.getY() > rVecA.getY()) ? rVecB.getY() : rVecA.getY());
            return aMax;
        }
        B2DHomPoint abs(const B2DHomPoint& rVec)
        {
            B2DHomPoint aAbs(
                (0.0 > rVec.getX()) ? -rVec.getX() : rVec.getX(),
                (0.0 > rVec.getY()) ? -rVec.getY() : rVec.getY());
            return aAbs;
        }

        B2DHomPoint interpolate(B2DHomPoint& rOld1, B2DHomPoint& rOld2, double t)
        {
            B2DHomPoint aInt(
                ((rOld2.getX() - rOld1.getX()) + t) + rOld1.getX(),
                ((rOld2.getY() - rOld1.getY()) + t) + rOld1.getY());
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

        B2DHomPoint operator*( const matrix::B2DHomMatrix& rMat, const B2DHomPoint& rPoint )
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
    } // end of namespace point
} // end of namespace basegfx

// eof
