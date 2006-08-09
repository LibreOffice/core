/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: polypolygonprimitive3d.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: aw $ $Date: 2006-08-09 16:51:15 $
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

#ifndef _DRAWINGLAYER_PRIMITIVE3D_POLYPOLYGONPRIMITIVE_HXX
#include <drawinglayer/primitive3d/polypolygonprimitive3d.hxx>
#endif

#ifndef _BGFX_POLYPOLYGON_B3DPOLYGONTOOLS_HXX
#include <basegfx/polygon/b3dpolypolygontools.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive3d
    {
        polyPolygonMaterialPrimitive3D::polyPolygonMaterialPrimitive3D(
            const basegfx::B3DPolyPolygon& rPolyPolygon,
            const attribute::materialAttribute3D& rMaterial,
            bool bDoubleSided)
        :   basePrimitive3D(),
            maPolyPolygon(rPolyPolygon),
            maMaterial(rMaterial),
            mbDoubleSided(bDoubleSided)
        {
        }

        polyPolygonMaterialPrimitive3D::~polyPolygonMaterialPrimitive3D()
        {
        }

        bool polyPolygonMaterialPrimitive3D::operator==(const basePrimitive3D& rPrimitive) const
        {
            if(getID() == rPrimitive.getID())
            {
                const polyPolygonMaterialPrimitive3D& rCompare = (polyPolygonMaterialPrimitive3D&)rPrimitive;

                return (maPolyPolygon == rCompare.maPolyPolygon
                    && maMaterial == rCompare.maMaterial
                    && mbDoubleSided == rCompare.mbDoubleSided);
            }

            return false;
        }

        PrimitiveID polyPolygonMaterialPrimitive3D::getID() const
        {
            return CreatePrimitiveID('P', 'O', 'M', '3');
        }

        basegfx::B3DRange polyPolygonMaterialPrimitive3D::get3DRange() const
        {
            return basegfx::tools::getRange(maPolyPolygon);
        }
    } // end of namespace primitive3d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
