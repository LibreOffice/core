/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: polypolygonprimitive3d.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: aw $ $Date: 2008-06-10 09:29:21 $
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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE3D_POLYPOLYGONPRIMITIVE3D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE3D_POLYPOLYGONPRIMITIVE3D_HXX

#include <drawinglayer/primitive3d/baseprimitive3d.hxx>
#include <basegfx/polygon/b3dpolypolygon.hxx>
#include <drawinglayer/attribute/materialattribute3d.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive3d
    {
        class PolyPolygonMaterialPrimitive3D : public BasePrimitive3D
        {
        private:
            basegfx::B3DPolyPolygon                     maPolyPolygon;
            attribute::MaterialAttribute3D              maMaterial;

            // bitfield
            unsigned                                    mbDoubleSided : 1;

        public:
            PolyPolygonMaterialPrimitive3D(
                const basegfx::B3DPolyPolygon& rPolyPolygon,
                const attribute::MaterialAttribute3D& rMaterial,
                bool bDoubleSided);

            // get data
            const basegfx::B3DPolyPolygon& getB3DPolyPolygon() const { return maPolyPolygon; }
            const attribute::MaterialAttribute3D& getMaterial() const { return maMaterial; }
            bool getDoubleSided() const { return mbDoubleSided; }

            // compare operator
            virtual bool operator==(const BasePrimitive3D& rPrimitive) const;

            // get range
            virtual basegfx::B3DRange getB3DRange(const geometry::ViewInformation3D& rViewInformation) const;

            // provide unique ID
            DeclPrimitrive3DIDBlock()
        };
    } // end of namespace primitive3d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_PRIMITIVE3D_POLYPOLYGONPRIMITIVE3D_HXX

//////////////////////////////////////////////////////////////////////////////
// eof
