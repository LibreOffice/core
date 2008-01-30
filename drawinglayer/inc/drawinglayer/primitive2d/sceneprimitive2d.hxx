/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sceneprimitive2d.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: aw $ $Date: 2008-01-30 12:25:04 $
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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_SCENEPRIMITIVE2D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE2D_SCENEPRIMITIVE2D_HXX

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_BASEPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/baseprimitive2d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE3D_BASEPRIMITIVE3D_HXX
#include <drawinglayer/primitive3d/baseprimitive3d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_ATTRIBUTE_SDRATTRIBUTE3D_HXX
#include <drawinglayer/attribute/sdrattribute3d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_GEOMETRY_TRANSFORMATION3D_HXX
#include <drawinglayer/geometry/transformation3d.hxx>
#endif

#ifndef _BGFX_MATRIX_B2DHOMMATRIX_HXX
#include <basegfx/matrix/b2dhommatrix.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        class ScenePrimitive2D : public BasePrimitive2D
        {
        private:
            primitive3d::Primitive3DSequence                    mxChildren3D;               // the 3d sub-primitives
            attribute::SdrSceneAttribute                        maSdrSceneAttribute;        // 3d scene attribute set
            attribute::SdrLightingAttribute                     maSdrLightingAttribute;     // lighting attribute set
            basegfx::B2DHomMatrix                               maObjectTransformation;     // object transformation for scene for 2d definition
            geometry::Transformation3D                          maTransformation3D;         // scene transformation set and object transformation

            // the primitiveSequence for on-demand created shadow primitives (see mbShadow3DChecked)
            Primitive2DSequence                                 maShadowPrimitives;

            // the primitiveSequence for on-demand created label primitives (see mbLabelChecked)
            Primitive2DSequence                                 maLabelPrimitives;

            // bitfield

            // flag if given 3D geometry is already cheched for shadow definitions and 2d shadows
            // are created in maShadowPrimitives
            unsigned                                            mbShadow3DChecked : 1;

            // flag if given 3D geometry is already cheched for label definitions and 2d labels
            // are created in maLabelPrimitives
            unsigned                                            mbLabel3DChecked : 1;

            // the last used viewInformation, used from getDecomposition for buffering
            basegfx::B2DHomMatrix                               maLastViewTransformation;
            basegfx::B2DRange                                   maLastViewport;

            // protected helpers
            bool impGetShadow3D(const geometry::ViewInformation2D& rViewInformation) const;
            bool impGetLabel3D(const geometry::ViewInformation2D& rViewInformation) const;

        protected:
            // local decomposition.
            virtual Primitive2DSequence createLocalDecomposition(const geometry::ViewInformation2D& rViewInformation) const;

        public:
            // public helpers
            // Geometry extractor. Shadow and labels will be added as in createLocalDecomposition, but
            // the 3D content is not converted to a bitmap visualisation but to projected 2D gemetry. This
            // helper is useful for Contour extraction.
            Primitive2DSequence getGeometry2D(const geometry::ViewInformation2D& rViewInformation) const;

            // constructor/destructor
            ScenePrimitive2D(
                const primitive3d::Primitive3DSequence& rxChildren3D,
                const attribute::SdrSceneAttribute& rSdrSceneAttribute,
                const attribute::SdrLightingAttribute& rSdrLightingAttribute,
                const basegfx::B2DHomMatrix& rObjectTransformation,
                const geometry::Transformation3D& rTransformation3D);

            // get data
            const primitive3d::Primitive3DSequence& getChildren3D() const { return mxChildren3D; }
            const attribute::SdrSceneAttribute& getSdrSceneAttribute() const { return maSdrSceneAttribute; }
            const attribute::SdrLightingAttribute& getSdrLightingAttribute() const { return maSdrLightingAttribute; }
            const basegfx::B2DHomMatrix& getObjectTransformation() const { return maObjectTransformation; }
            const geometry::Transformation3D& getTransformation3D() const { return maTransformation3D; }

            // compare operator
            virtual bool operator==(const BasePrimitive2D& rPrimitive) const;

            // get range
            virtual basegfx::B2DRange getB2DRange(const geometry::ViewInformation2D& rViewInformation) const;

            // provide unique ID
            DeclPrimitrive2DIDBlock()

            // get local decomposition. Overloaded since this decomposition is view-dependent
            virtual Primitive2DSequence get2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const;
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_PRIMITIVE2D_SCENEPRIMITIVE2D_HXX

//////////////////////////////////////////////////////////////////////////////
// eof
