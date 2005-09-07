/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: b3ivector.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 20:55:03 $
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

#ifndef _BGFX_VECTOR_B3IVECTOR_HXX
#include <basegfx/vector/b3ivector.hxx>
#endif

#ifndef _BGFX_MATRIX_B3DHOMMATRIX_HXX
#include <basegfx/matrix/b3dhommatrix.hxx>
#endif

namespace basegfx
{
    B3IVector& B3IVector::operator*=( const B3DHomMatrix& rMat )
    {
        mnX = fround( rMat.get(0,0)*mnX + rMat.get(0,1)*mnY + rMat.get(0,2)*mnZ );
        mnY = fround( rMat.get(1,0)*mnX + rMat.get(1,1)*mnY + rMat.get(1,2)*mnZ );
        mnZ = fround( rMat.get(2,0)*mnX + rMat.get(2,1)*mnY + rMat.get(2,2)*mnZ );

        return *this;
    }

    B3IVector operator*( const B3DHomMatrix& rMat, const B3IVector& rVec )
    {
        B3IVector aRes( rVec );
        return aRes*=rMat;
    }
} // end of namespace basegfx

// eof
