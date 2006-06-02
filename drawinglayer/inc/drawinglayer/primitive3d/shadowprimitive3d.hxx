/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: shadowprimitive3d.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: aw $ $Date: 2006-06-02 13:57:59 $
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

#ifndef _DRAWINGLAYER_PRIMITIVE3D_SHADOWPRIMITIVE3D_HXX
#define _DRAWINGLAYER_PRIMITIVE3D_SHADOWPRIMITIVE3D_HXX

#ifndef _DRAWINGLAYER_PRIMITIVE_VECTORPRIMITIVE_HXX
#include <drawinglayer/primitive/vectorprimitive.hxx>
#endif

#ifndef _BGFX_MATRIX_B2DHOMMATRIX_HXX
#include <basegfx/matrix/b2dhommatrix.hxx>
#endif

#ifndef _BGFX_COLOR_BCOLOR_HXX
#include <basegfx/color/bcolor.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive
    {
        class shadowPrimitive3D : public vectorPrimitive
        {
        protected:
            ::basegfx::B2DHomMatrix                     maShadowTransform;
            ::basegfx::BColor                           maShadowColor;
            double                                      mfShadowTransparence;

            // bitfield
            unsigned                                    mbShadow3D : 1;

        public:
            shadowPrimitive3D(
                const ::basegfx::B2DHomMatrix& rShadowTransform, const ::basegfx::BColor& rShadowColor,
                double fShadowTransparence, bool bShadow3D, const primitiveVector& rPrimitiveVector);
            virtual ~shadowPrimitive3D();

            // get data
            const ::basegfx::B2DHomMatrix& getShadowTransform() const { return maShadowTransform; }
            const ::basegfx::BColor& getShadowColor() const { return maShadowColor; }
            double getShadowTransparence() const { return mfShadowTransparence; }
            bool getShadow3D() const { return mbShadow3D; }

            // compare operator
            virtual bool operator==(const basePrimitive& rPrimitive) const;

            // id generator
            virtual PrimitiveID getID() const;
        };
    } // end of namespace primitive
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif // _DRAWINGLAYER_PRIMITIVE3D_SHADOWPRIMITIVE3D_HXX

// eof
