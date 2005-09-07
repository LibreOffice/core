/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: b3ipoint.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 20:45:02 $
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

#ifndef _BGFX_POINT_B3IPOINT_HXX
#include <basegfx/point/b3ipoint.hxx>
#endif

#ifndef _BGFX_MATRIX_B3DHOMMATRIX_HXX
#include <basegfx/matrix/b3dhommatrix.hxx>
#endif

#include <basegfx/numeric/ftools.hxx>

namespace basegfx
{
    B3IPoint& B3IPoint::operator*=( const ::basegfx::B3DHomMatrix& rMat )
    {
        mnX = fround(rMat.get(0,0)*mnX +
                     rMat.get(0,1)*mnY +
                     rMat.get(0,2)*mnZ +
                     rMat.get(0,3));
        mnY = fround(rMat.get(1,0)*mnX +
                     rMat.get(1,1)*mnY +
                     rMat.get(1,2)*mnZ +
                     rMat.get(1,3));
        mnZ = fround(rMat.get(2,0)*mnX +
                     rMat.get(2,1)*mnY +
                     rMat.get(2,2)*mnZ +
                     rMat.get(2,3));

        return *this;
    }
} // end of namespace basegfx

// eof
