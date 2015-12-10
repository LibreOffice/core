/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_PRIMITIVETOOLS2D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE2D_PRIMITIVETOOLS2D_HXX

#include <drawinglayer/drawinglayerdllapi.h>

#include <drawinglayer/primitive2d/baseprimitive2d.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>



namespace drawinglayer
{
    namespace primitive2d
    {
        /** DiscreteMetricDependentPrimitive2D class

            tooling class for BufferedDecompositionPrimitive2D based classes which are view-dependent
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

        protected:
            /// helper to update discrete unit
            void updateDiscreteUnit(double fNew)
            {
                mfDiscreteUnit = fNew;
            }

        public:
            /// constructor
            DiscreteMetricDependentPrimitive2D()
            :   BufferedDecompositionPrimitive2D(),
                mfDiscreteUnit(0.0)
            {
            }

            /// data read access
            double getDiscreteUnit() const { return mfDiscreteUnit; }

            /// Override standard getDecomposition to be view-dependent here
            virtual Primitive2DContainer get2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const override;
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer



namespace drawinglayer
{
    namespace primitive2d
    {
        /** ViewportDependentPrimitive2D class

            tooling class for BufferedDecompositionPrimitive2D based classes which are view-dependent
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

            /// Override standard getDecomposition to be view-dependent here
            virtual Primitive2DContainer get2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const override;
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer



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

            /// Override standard getDecomposition to be view-dependent here
            virtual Primitive2DContainer get2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const override;
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer



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

            /// Override standard getDecomposition to be view-dependent here
            virtual Primitive2DContainer get2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const override;
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer



#endif // INCLUDED_DRAWINGLAYER_PRIMITIVE2D_PRIMITIVETOOLS2D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
