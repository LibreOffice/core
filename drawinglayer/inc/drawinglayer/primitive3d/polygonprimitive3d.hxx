/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: polygonprimitive3d.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: aw $ $Date: 2006-08-09 16:38:13 $
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

#ifndef _DRAWINGLAYER_PRIMITIVE3D_POLYGONPRIMITIVE3D_HXX
#define _DRAWINGLAYER_PRIMITIVE3D_POLYGONPRIMITIVE3D_HXX

#ifndef _DRAWINGLAYER_PRIMITIVE3D_PRIMITIVE3D_HXX
#include <drawinglayer/primitive3d/primitive3d.hxx>
#endif

#ifndef _BGFX_COLOR_BCOLOR_HXX
#include <basegfx/color/bcolor.hxx>
#endif

#ifndef _BGFX_POLYGON_B3DPOLYGON_HXX
#include <basegfx/polygon/b3dpolygon.hxx>
#endif

#ifndef _DRAWINGLAYER_ATTRIBUTE_STROKEATTRIBUTE_HXX
#include <drawinglayer/attribute/strokeattribute.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////
// predefines

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive3d
    {
        class polygonHairlinePrimitive3D : public basePrimitive3D
        {
        protected:
            basegfx::B3DPolygon                     maPolygon;
            basegfx::BColor                         maBColor;

        public:
            polygonHairlinePrimitive3D(const basegfx::B3DPolygon& rPolygon, const basegfx::BColor& rBColor);
            virtual ~polygonHairlinePrimitive3D();

            // get data
            const basegfx::B3DPolygon& getB3DPolygon() const { return maPolygon; }
            const basegfx::BColor& getBColor() const { return maBColor; }

            // compare operator
            virtual bool operator==(const basePrimitive3D& rPrimitive) const;

            // id generator
            virtual PrimitiveID getID() const;

            // get 3Drange of primitive. Default implementation uses decomposition
            virtual basegfx::B3DRange get3DRange() const;
        };
    } // end of namespace primitive3d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive3d
    {
        class polygonStrokePrimitive3D : public basePrimitive3D
        {
        protected:
            basegfx::B3DPolygon                     maPolygon;
            attribute::strokeAttribute                  maStrokeAttribute;

            //  create decomposition
            virtual void decompose(primitiveVector3D& rTarget);

        public:
            polygonStrokePrimitive3D(
                const basegfx::B3DPolygon& rPolygon,
                const attribute::strokeAttribute& rStrokeAttribute);
            virtual ~polygonStrokePrimitive3D();

            // get data
            basegfx::B3DPolygon getB3DPolygon() const { return maPolygon; }
            const attribute::strokeAttribute& getStrokeAttribute() const { return maStrokeAttribute; }

            // compare operator
            virtual bool operator==(const basePrimitive3D& rPrimitive) const;

            // id generator
            virtual PrimitiveID getID() const;
        };
    } // end of namespace primitive3d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif // _DRAWINGLAYER_PRIMITIVE_POLYGONPRIMITIVE_HXX

// eof
