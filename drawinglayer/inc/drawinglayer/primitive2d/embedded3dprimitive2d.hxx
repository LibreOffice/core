/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: embedded3dprimitive2d.hxx,v $
 *
 *  $Revision: 1.5 $
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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_EMBEDDED3DPRIMITIVE2D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE2D_EMBEDDED3DPRIMITIVE2D_HXX

#include <drawinglayer/primitive2d/baseprimitive2d.hxx>
#include <drawinglayer/primitive3d/baseprimitive3d.hxx>
#include <drawinglayer/geometry/viewinformation3d.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>

//////////////////////////////////////////////////////////////////////////////
// BackgroundColorPrimitive2D class

namespace drawinglayer
{
    namespace primitive2d
    {
        class Embedded3DPrimitive2D : public BasePrimitive2D
        {
        private:
            primitive3d::Primitive3DSequence                mxChildren3D;
            basegfx::B2DHomMatrix                           maObjectTransformation;
            geometry::ViewInformation3D                     maViewInformation3D;

        protected:
            // local decomposition.
            virtual Primitive2DSequence createLocalDecomposition(const geometry::ViewInformation2D& rViewInformation) const;

        public:
            Embedded3DPrimitive2D(
                const primitive3d::Primitive3DSequence& rxChildren3D,
                const basegfx::B2DHomMatrix& rObjectTransformation,
                const geometry::ViewInformation3D& rViewInformation3D);

            // get data
            const primitive3d::Primitive3DSequence& getChildren3D() const { return mxChildren3D; }
            const basegfx::B2DHomMatrix& getObjectTransformation() const { return maObjectTransformation; }
            const geometry::ViewInformation3D& getViewInformation3D() const { return maViewInformation3D; }

            // compare operator
            virtual bool operator==(const BasePrimitive2D& rPrimitive) const;

            // get range
            virtual basegfx::B2DRange getB2DRange(const geometry::ViewInformation2D& rViewInformation) const;

            // provide unique ID
            DeclPrimitrive2DIDBlock()
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_PRIMITIVE2D_EMBEDDED3DPRIMITIVE2D_HXX

//////////////////////////////////////////////////////////////////////////////
// eof
