/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: polypolygonprimitive3d.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: aw $ $Date: 2008-03-05 09:15:44 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_drawinglayer.hxx"

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE3D_POLYPOLYGONPRIMITIVE3D_HXX
#include <drawinglayer/primitive3d/polypolygonprimitive3d.hxx>
#endif

#ifndef _BGFX_POLYPOLYGON_B3DPOLYGONTOOLS_HXX
#include <basegfx/polygon/b3dpolypolygontools.hxx>
#endif

#ifndef _BGFX_TOOLS_CANVASTOOLS_HXX
#include <basegfx/tools/canvastools.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE3D_PRIMITIVETYPES3D_HXX
#include <drawinglayer/primitive3d/drawinglayer_primitivetypes3d.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////

using namespace com::sun::star;

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive3d
    {
        PolyPolygonMaterialPrimitive3D::PolyPolygonMaterialPrimitive3D(
            const basegfx::B3DPolyPolygon& rPolyPolygon,
            const attribute::MaterialAttribute3D& rMaterial,
            bool bDoubleSided)
        :   BasePrimitive3D(),
            maPolyPolygon(rPolyPolygon),
            maMaterial(rMaterial),
            mbDoubleSided(bDoubleSided)
        {
        }

        bool PolyPolygonMaterialPrimitive3D::operator==(const BasePrimitive3D& rPrimitive) const
        {
            if(BasePrimitive3D::operator==(rPrimitive))
            {
                const PolyPolygonMaterialPrimitive3D& rCompare = (PolyPolygonMaterialPrimitive3D&)rPrimitive;

                return (getB3DPolyPolygon() == rCompare.getB3DPolyPolygon()
                    && getMaterial() == rCompare.getMaterial()
                    && getDoubleSided() == rCompare.getDoubleSided());
            }

            return false;
        }

        basegfx::B3DRange PolyPolygonMaterialPrimitive3D::getB3DRange(double /*fTime*/) const
        {
            return basegfx::tools::getRange(getB3DPolyPolygon());
        }

        // provide unique ID
        ImplPrimitrive3DIDBlock(PolyPolygonMaterialPrimitive3D, PRIMITIVE3D_ID_POLYPOLYGONMATERIALPRIMITIVE3D)

    } // end of namespace primitive3d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
