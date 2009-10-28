/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: shadowprimitive3d.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: aw $ $Date: 2008-05-27 14:11:18 $
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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE3D_SHADOWPRIMITIVE3D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE3D_SHADOWPRIMITIVE3D_HXX

#include <drawinglayer/primitive3d/groupprimitive3d.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/color/bcolor.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive3d
    {
        /** ShadowPrimitive3D class

            This 3D grouping primitive is used to define a shadow for
            3d geometry by embedding it. The shadow of 3D objects are
            2D polygons, so the shadow transformation is a 2D transformation.

            If the Shadow3D flag is set, the shadow definition has to be
            combined with the scene and camera definition to create the correct
            projected shadow 2D-Polygons.
         */
        class ShadowPrimitive3D : public GroupPrimitive3D
        {
        protected:
            /// 2D shadow definition
            basegfx::B2DHomMatrix                   maShadowTransform;
            basegfx::BColor                         maShadowColor;
            double                                  mfShadowTransparence;

            /// bitfield
            unsigned                                mbShadow3D : 1;

        public:
            /// constructor
            ShadowPrimitive3D(
                const basegfx::B2DHomMatrix& rShadowTransform,
                const basegfx::BColor& rShadowColor,
                double fShadowTransparence,
                bool bShadow3D,
                const Primitive3DSequence& rChildren);

            /// data read access
            const basegfx::B2DHomMatrix& getShadowTransform() const { return maShadowTransform; }
            const basegfx::BColor& getShadowColor() const { return maShadowColor; }
            double getShadowTransparence() const { return mfShadowTransparence; }
            bool getShadow3D() const { return mbShadow3D; }

            /// compare operator
            virtual bool operator==(const BasePrimitive3D& rPrimitive) const;

            /// provide unique ID
            DeclPrimitrive3DIDBlock()
        };
    } // end of namespace primitive3d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_PRIMITIVE3D_SHADOWPRIMITIVE3D_HXX

//////////////////////////////////////////////////////////////////////////////
// eof
