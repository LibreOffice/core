/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: transformprimitive3d.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: aw $ $Date: 2006-06-02 14:12:39 $
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

#ifndef _DRAWINGLAYER_PRIMITIVE3D_TRANSFORMPRIMITIVE3D_HXX
#include <drawinglayer/primitive3d/transformprimitive3d.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive
    {
        transformPrimitive3D::transformPrimitive3D(const ::basegfx::B3DHomMatrix& rTransformation, const primitiveVector& rPrimitiveVector)
        :   vectorPrimitive(rPrimitiveVector),
            maTransformation(rTransformation)
        {
        }

        transformPrimitive3D::~transformPrimitive3D()
        {
        }

        bool transformPrimitive3D::operator==(const basePrimitive& rPrimitive) const
        {
            if(vectorPrimitive::operator==(rPrimitive))
            {
                const transformPrimitive3D& rCompare = static_cast< const transformPrimitive3D& >(rPrimitive);
                return (maTransformation == rCompare.maTransformation);
            }

            return false;
        }

        PrimitiveID transformPrimitive3D::getID() const
        {
            return CreatePrimitiveID('T', 'R', 'N', '3');
        }

        ::basegfx::B3DRange transformPrimitive3D::get3DRange(const ::drawinglayer::geometry::viewInformation& rViewInformation) const
        {
            ::basegfx::B3DRange aRetval(::drawinglayer::primitive::get3DRangeFromVector(maPrimitiveVector, rViewInformation));
            aRetval.transform(maTransformation);
            return aRetval;
        }
    } // end of namespace primitive
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
