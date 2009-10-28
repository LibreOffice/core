/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: shadowprimitive2d.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: aw $ $Date: 2008-05-27 14:11:17 $
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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_SHADOWPRIMITIVE2D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE2D_SHADOWPRIMITIVE2D_HXX

#include <drawinglayer/primitive2d/groupprimitive2d.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/color/bcolor.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        /** ShadowPrimitive2D class

            This primitive defines a generic shadow geometry construction
            for 2D objects. It decomposes to a TransformPrimitive2D embedded
            into a ModifiedColorPrimitive2D.

            It's for primtive usage convenience, so that not everyone has
            to implement the generic shadow construction by himself.

            The same geometry as sequence of primitives is used as geometry and
            as shadow. Since these are RefCounted Uno-Api objects, no extra objects
            are needed for the shadow itself; all the local decompositions of the
            original geometry can be reused from the renderer for shadow visualisation.
        */
        class ShadowPrimitive2D : public GroupPrimitive2D
        {
        private:
            /// the shadow transformation, normally just an offset
            basegfx::B2DHomMatrix                   maShadowTransform;

            /// the shadow color to which all geometry is to be forced
            basegfx::BColor                         maShadowColor;

        public:
            /// constructor
            ShadowPrimitive2D(
                const basegfx::B2DHomMatrix& rShadowTransform,
                const basegfx::BColor& rShadowColor,
                const Primitive2DSequence& rChildren);

            /// data read access
            const basegfx::B2DHomMatrix& getShadowTransform() const { return maShadowTransform; }
            const basegfx::BColor& getShadowColor() const { return maShadowColor; }

            /// compare operator
            virtual bool operator==(const BasePrimitive2D& rPrimitive) const;

            /// get range
            virtual basegfx::B2DRange getB2DRange(const geometry::ViewInformation2D& rViewInformation) const;

            ///  create decomposition
            virtual Primitive2DSequence get2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const;

            /// provide unique ID
            DeclPrimitrive2DIDBlock()
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_PRIMITIVE2D_SHADOWPRIMITIVE2D_HXX

//////////////////////////////////////////////////////////////////////////////
// eof
