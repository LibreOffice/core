/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: b2dpoint.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 20:43:56 $
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

#ifndef _BGFX_POINT_B2DPOINT_HXX
#include <basegfx/point/b2dpoint.hxx>
#endif

#ifndef _BGFX_MATRIX_B2DHOMMATRIX_HXX
#include <basegfx/matrix/b2dhommatrix.hxx>
#endif

namespace basegfx
{
    B2DPoint& B2DPoint::operator=( const ::basegfx::B2DTuple& rPoint )
    {
        mfX = rPoint.getX();
        mfY = rPoint.getY();
        return *this;
    }

    B2DPoint& B2DPoint::operator*=( const ::basegfx::B2DHomMatrix& rMat )
    {
        const double fTempX( rMat.get(0,0)*mfX +
                            rMat.get(0,1)*mfY +
                            rMat.get(0,2)        );
        const double fTempY( rMat.get(1,0)*mfX +
                            rMat.get(1,1)*mfY +
                            rMat.get(1,2)        );
        mfX = fTempX;
        mfY = fTempY;

        return *this;
    }

    B2DPoint operator*( const ::basegfx::B2DHomMatrix& rMat, const B2DPoint& rPoint )
    {
        B2DPoint aRes( rPoint );
        return aRes *= rMat;
    }
} // end of namespace basegfx

// eof
