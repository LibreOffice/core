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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_PRIMITIVE2DTOOLS_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE2D_PRIMITIVE2DTOOLS_HXX

#include <drawinglayer/drawinglayerdllapi.h>

#include <drawinglayer/primitive2d/baseprimitive2d.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        /** DiscreteMetricDependentPrimitive2D class

            tooling class for BufferedDecompositionPrimitive2D baseed classes which are view-dependent
            regarding the size of a discrete unit. The implementation of get2DDecomposition
            guards the buffered local decomposition and ensures that a create2DDecomposition
            implementation may use an up-to-date DiscreteUnit accessible using getDiscreteUnit()
         */
        class DRAWINGLAYER_DLLPUBLIC DiscreteMetricDependentPrimitive2D : public BufferedDecompositionPrimitive2D
        {
        private:
            /** the last used fDiscreteUnit definitions for decomposition. Since this
                is checked and updated from get2DDecomposition() it will be current and
                usable in create2DDecomposition()
             */
            double                                  mfDiscreteUnit;

        public:
            /// constructor
            DiscreteMetricDependentPrimitive2D()
            :   BufferedDecompositionPrimitive2D(),
                mfDiscreteUnit(0.0)
            {
            }

            /// data read access
            double getDiscreteUnit() const { return mfDiscreteUnit; }

            /// get local decomposition. Overloaded since this decomposition is view-dependent
            virtual Primitive2DSequence get2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const;
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        /** ViewportDependentPrimitive2D class

            tooling class for BufferedDecompositionPrimitive2D baseed classes which are view-dependent
            regarding the viewport. The implementation of get2DDecomposition
            guards the buffered local decomposition and ensures that a create2DDecomposition
            implementation may use an up-to-date Viewport accessible using getViewport()
         */
        class DRAWINGLAYER_DLLPUBLIC ViewportDependentPrimitive2D : public BufferedDecompositionPrimitive2D
        {
        private:
            /** the last used Viewport definition for decomposition. Since this
                is checked and updated from get2DDecomposition() it will be current and
                usable in create2DDecomposition()
             */
            basegfx::B2DRange                       maViewport;

        public:
            /// constructor
            ViewportDependentPrimitive2D()
            :   BufferedDecompositionPrimitive2D(),
                maViewport()
            {
            }

            /// data read access
            const basegfx::B2DRange& getViewport() const { return maViewport; }

            /// get local decomposition. Overloaded since this decomposition is view-dependent
            virtual Primitive2DSequence get2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const;
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        /** ViewTransformationDependentPrimitive2D class

            tooling class for BufferedDecompositionPrimitive2D based classes which are view-dependent
            regarding the complete Viewtransformation. The implementation of get2DDecomposition
            guards the buffered local decomposition and ensures that a create2DDecomposition
            implementation may use an up-to-date ViewTransformation accessible using getViewTransformation()
         */
        class DRAWINGLAYER_DLLPUBLIC ViewTransformationDependentPrimitive2D : public BufferedDecompositionPrimitive2D
        {
        private:
            /** the last used ViewTransformation definition for decomposition. Since this
                is checked and updated from get2DDecomposition() it will be current and
                usable in create2DDecomposition()
             */
            basegfx::B2DHomMatrix                   maViewTransformation;

        public:
            /// constructor
            ViewTransformationDependentPrimitive2D()
            :   BufferedDecompositionPrimitive2D(),
                maViewTransformation()
            {
            }

            /// data read access
            const basegfx::B2DHomMatrix& getViewTransformation() const { return maViewTransformation; }

            /// get local decomposition. Overloaded since this decomposition is view-dependent
            virtual Primitive2DSequence get2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const;
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        /** ObjectAndViewTransformationDependentPrimitive2D class

            tooling class for BufferedDecompositionPrimitive2D based classes which are view-dependent
            and Object-Transform dependent. The implementation of get2DDecomposition
            guards the buffered local decomposition and ensures that a create2DDecomposition
            implementation may use an up-to-date ViewTransformation accessible using getViewTransformation()
            and an object transformation via getObjectTransformation()
         */
        class DRAWINGLAYER_DLLPUBLIC ObjectAndViewTransformationDependentPrimitive2D : public BufferedDecompositionPrimitive2D
        {
        private:
            /** the last used ViewTransformation and the last ObjectTransformation
                definition for decomposition. Since this is checked and updated from
                get2DDecomposition() it will be current and usable in create2DDecomposition()
             */
            basegfx::B2DHomMatrix                   maViewTransformation;
            basegfx::B2DHomMatrix                   maObjectTransformation;

        public:
            /// constructor
            ObjectAndViewTransformationDependentPrimitive2D()
            :   BufferedDecompositionPrimitive2D(),
                maViewTransformation(),
                maObjectTransformation()
            {
            }

            /// data read access
            const basegfx::B2DHomMatrix& getViewTransformation() const { return maViewTransformation; }
            const basegfx::B2DHomMatrix& getObjectTransformation() const { return maObjectTransformation; }

            /// get local decomposition. Overloaded since this decomposition is view-dependent
            virtual Primitive2DSequence get2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const;
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_PRIMITIVE2D_PRIMITIVE2DTOOLS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
