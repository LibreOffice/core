/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_SCENEPRIMITIVE2D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE2D_SCENEPRIMITIVE2D_HXX

#include <drawinglayer/drawinglayerdllapi.h>

#include <drawinglayer/primitive2d/baseprimitive2d.hxx>
#include <drawinglayer/primitive3d/baseprimitive3d.hxx>
#include <drawinglayer/geometry/viewinformation3d.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <vcl/bitmapex.hxx>
#include <drawinglayer/attribute/sdrsceneattribute3d.hxx>
#include <drawinglayer/attribute/sdrlightingattribute3d.hxx>
#include <drawinglayer/attribute/sdrlightattribute3d.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        /** ScenePrimitive2D class

            This primitive defines a 3D scene as a 2D primitive and is the anchor point
            for a 3D visualisation. The decomposition is view-dependent and will try to
            re-use already rendered 3D content.

            The rendering is done using the default-3D renderer from basegfx which supports
            AntiAliasing.

            The 2D primitive's geometric range is defined completely by the
            ObjectTransformation combined with evtl. 2D shadows from the 3D objects. The
            shadows of 3D objects are 2D polygons, projected with the 3D transformation.

            This is the class a renderer may process directly when he wants to implement
            an own (e.g. system-specific) 3D renderer.
         */
        class DRAWINGLAYER_DLLPUBLIC ScenePrimitive2D : public BufferedDecompositionPrimitive2D
        {
        private:
            /// the 3D geometry definition
            primitive3d::Primitive3DSequence                    mxChildren3D;

            /// 3D scene attribute set
            attribute::SdrSceneAttribute                        maSdrSceneAttribute;

            /// lighting attribute set
            attribute::SdrLightingAttribute                     maSdrLightingAttribute;

            /// object transformation for scene for 2D definition
            basegfx::B2DHomMatrix                               maObjectTransformation;

            /// scene transformation set and object transformation
            geometry::ViewInformation3D                         maViewInformation3D;

            /// the primitiveSequence for on-demand created shadow primitives (see mbShadow3DChecked)
            Primitive2DSequence                                 maShadowPrimitives;

            /// bitfield
            /** flag if given 3D geometry is already cheched for shadow definitions and 2d shadows
                are created in maShadowPrimitives
             */
            unsigned                                            mbShadow3DChecked : 1;

            /// the last used NewDiscreteSize and NewUnitVisiblePart definitions for decomposition
            double                                              mfOldDiscreteSizeX;
            double                                              mfOldDiscreteSizeY;
            basegfx::B2DRange                                   maOldUnitVisiblePart;

            /** the last created BitmapEx, e.g. for fast HitTest. This does not really need
                memory since BitmapEx is internally RefCounted
             */
            BitmapEx                                            maOldRenderedBitmap;

            /// private helpers
            bool impGetShadow3D(const geometry::ViewInformation2D& rViewInformation) const;
            void calculateDiscreteSizes(
                const geometry::ViewInformation2D& rViewInformation,
                basegfx::B2DRange& rDiscreteRange,
                basegfx::B2DRange& rVisibleDiscreteRange,
                basegfx::B2DRange& rUnitVisibleRange) const;

        protected:
            /// local decomposition.
            virtual Primitive2DSequence create2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const;

        public:
            /// public helpers
            /** Geometry extractor. Shadow will be added as in create2DDecomposition, but
                the 3D content is not converted to a bitmap visualisation but to projected 2D gemetry. This
                helper is useful e.g. for Contour extraction or HitTests.
              */
            Primitive2DSequence getGeometry2D() const;
            Primitive2DSequence getShadow2D(const geometry::ViewInformation2D& rViewInformation) const;

            /** Fast HitTest which uses the last buffered BitmapEx from the last
                rendered area if available. The return value describes if the check
                could be done with the current information, so do NOT use o_rResult
                when it returns false. o_rResult will be changed on return true and
                then contains a definitive answer if content of this scene is hit or
                not. On return false, it is normally necessary to use the geometric
                HitTest (see CutFindProcessor usages). The given HitPoint
                has to be in logic coordinates in scene's ObjectCoordinateSystem.
             */
            bool tryToCheckLastVisualisationDirectHit(const basegfx::B2DPoint& rLogicHitPoint, bool& o_rResult) const;

            /// constructor
            ScenePrimitive2D(
                const primitive3d::Primitive3DSequence& rxChildren3D,
                const attribute::SdrSceneAttribute& rSdrSceneAttribute,
                const attribute::SdrLightingAttribute& rSdrLightingAttribute,
                const basegfx::B2DHomMatrix& rObjectTransformation,
                const geometry::ViewInformation3D& rViewInformation3D);

            /// data read access
            const primitive3d::Primitive3DSequence& getChildren3D() const { return mxChildren3D; }
            const attribute::SdrSceneAttribute& getSdrSceneAttribute() const { return maSdrSceneAttribute; }
            const attribute::SdrLightingAttribute& getSdrLightingAttribute() const { return maSdrLightingAttribute; }
            const basegfx::B2DHomMatrix& getObjectTransformation() const { return maObjectTransformation; }
            const geometry::ViewInformation3D& getViewInformation3D() const { return maViewInformation3D; }

            /// compare operator
            virtual bool operator==(const BasePrimitive2D& rPrimitive) const;

            /// get range
            virtual basegfx::B2DRange getB2DRange(const geometry::ViewInformation2D& rViewInformation) const;

            /// provide unique ID
            DeclPrimitrive2DIDBlock()

            /// get local decomposition. Overloaded since this decomposition is view-dependent
            virtual Primitive2DSequence get2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const;
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_PRIMITIVE2D_SCENEPRIMITIVE2D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
