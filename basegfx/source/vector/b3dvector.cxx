/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: b3dvector.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 20:54:45 $
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

#ifndef _BGFX_VECTOR_B3DVECTOR_HXX
#include <basegfx/vector/b3dvector.hxx>
#endif

#ifndef _BGFX_MATRIX_B3DHOMMATRIX_HXX
#include <basegfx/matrix/b3dhommatrix.hxx>
#endif

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

    B3DVector B3DVector::getProjectionOnPlane(const B3DVector& rNormalizedPlane) const
    {
        B3DVector aNew(*this);
        aNew = cross(aNew, rNormalizedPlane);
        aNew = cross(aNew, rNormalizedPlane);

        aNew.mfX = mfX - aNew.mfX;
        aNew.mfY = mfY - aNew.mfY;
        aNew.mfZ = mfZ - aNew.mfZ;

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
} // end of namespace basegfx

// eof
