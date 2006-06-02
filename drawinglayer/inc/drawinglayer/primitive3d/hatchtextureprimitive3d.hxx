/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: hatchtextureprimitive3d.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: aw $ $Date: 2006-06-02 13:57:58 $
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

#ifndef _DRAWINGLAYER_PRIMITIVE3D_HATCHTEXTUREPRIMITIVE3D_HXX
#define _DRAWINGLAYER_PRIMITIVE3D_HATCHTEXTUREPRIMITIVE3D_HXX

#ifndef _DRAWINGLAYER_PRIMITIVE3D_TEXTUREPRIMITIVE3D_HXX
#include <drawinglayer/primitive3d/textureprimitive3d.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive
    {
        class hatchTexturePrimitive3D : public texturePrimitive3D
        {
        protected:
            fillHatchAttribute                          maHatch;

            //  create decomposition
            virtual void decompose(primitiveVector& rTarget, const ::drawinglayer::geometry::viewInformation& rViewInformation);

            // helpers
            void impCreateDecomposition(const primitiveVector& rSource, primitiveVector& rDest);

        public:
            hatchTexturePrimitive3D(
                const fillHatchAttribute& rHatch,
                const primitiveVector& rPrimitiveVector,
                const ::basegfx::B2DVector& rTextureSize,
                bool bModulate,
                bool bFilter);
            virtual ~hatchTexturePrimitive3D();

            // get data
            const fillHatchAttribute& getHatch() const { return maHatch; }

            // compare operator
            virtual bool operator==(const basePrimitive& rPrimitive) const;

            // id generator
            virtual PrimitiveID getID() const;
        };
    } // end of namespace primitive
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif // _DRAWINGLAYER_PRIMITIVE3D_HATCHTEXTUREPRIMITIVE3D_HXX

// eof
