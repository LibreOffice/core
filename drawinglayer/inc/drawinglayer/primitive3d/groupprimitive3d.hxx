/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: groupprimitive3d.hxx,v $
 *
 *  $Revision: 1.4 $
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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE3D_GROUPPRIMITIVE3D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE3D_GROUPPRIMITIVE3D_HXX

#include <drawinglayer/primitive3d/baseprimitive3d.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive3d
    {
        /** GroupPrimitive3D class

            Baseclass for all grouping 3D primitives

            The description/functionality is identical with the 2D case in groupprimitive2d.hxx,
            please see there for detailed information.

            Current Basic 3D StatePrimitives are:

            - ModifiedColorPrimitive3D (for a stack of color modifications)
            - ShadowPrimitive3D (for 3D objects with shadow; this is a special case
              since the shadow of a 3D primitive is a 2D primitive set)
            - TexturePrimitive3D (with the following variations)
                - GradientTexturePrimitive3D (for 3D gradient fill)
                - BitmapTexturePrimitive3D (for 3D Bitmap fill)
                - AlphaTexturePrimitive3D (for 3D transparence)
                - HatchTexturePrimitive3D (for 3D hatch fill)
            - TransformPrimitive3D (for a transformation stack)
         */
        class GroupPrimitive3D : public BasePrimitive3D
        {
        private:
            /// the children. Declared private since this shall never be changed at all after construction
            Primitive3DSequence                             maChildren;

        public:
            /// constructor
            GroupPrimitive3D(const Primitive3DSequence& rChildren);

            /// data read access
            Primitive3DSequence getChildren() const { return maChildren; }

            /// compare operator
            virtual bool operator==( const BasePrimitive3D& rPrimitive ) const;

            /// local decomposition. Implementation will just return children
            virtual Primitive3DSequence get3DDecomposition(const geometry::ViewInformation3D& rViewInformation) const;

            /// provide unique ID
            DeclPrimitrive3DIDBlock()
        };
    } // end of namespace primitive3d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_PRIMITIVE3D_GROUPPRIMITIVE3D_HXX

//////////////////////////////////////////////////////////////////////////////
// eof
