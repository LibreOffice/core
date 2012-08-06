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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_CONTROLPRIMITIVE2D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE2D_CONTROLPRIMITIVE2D_HXX

#include <drawinglayer/drawinglayerdllapi.h>

#include <drawinglayer/primitive2d/baseprimitive2d.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <com/sun/star/awt/XControlModel.hpp>
#include <com/sun/star/awt/XControl.hpp>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        /** ControlPrimitive2D class

            Base class for ControlPrimitive handling. It decoposes to a
            graphical representation (Bitmap data) of the control. This
            representation is limited to a quadratic pixel maximum defined
            in the applicatin settings.
         */
        class DRAWINGLAYER_DLLPUBLIC ControlPrimitive2D : public BufferedDecompositionPrimitive2D
        {
        private:
            /// object's base data
            basegfx::B2DHomMatrix                                                   maTransform;
            com::sun::star::uno::Reference< com::sun::star::awt::XControlModel >    mxControlModel;

            /// the created an cached awt::XControl
            com::sun::star::uno::Reference< com::sun::star::awt::XControl >         mxXControl;

            /// the last used scaling, used from getDecomposition for buffering
            basegfx::B2DVector                                                      maLastViewScaling;

            /** used from getXControl() to create a local awt::XControl which is remembered in mxXControl
                and from thereon always used and returned by getXControl()
             */
            void createXControl();

            /// single local decompositions, used from create2DDecomposition()
            Primitive2DReference createBitmapDecomposition(const geometry::ViewInformation2D& rViewInformation) const;
            Primitive2DReference createPlaceholderDecomposition(const geometry::ViewInformation2D& rViewInformation) const;

        protected:
            /// local decomposition
            virtual Primitive2DSequence create2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const;

        public:
            /// constructor
            ControlPrimitive2D(
                const basegfx::B2DHomMatrix& rTransform,
                const com::sun::star::uno::Reference< com::sun::star::awt::XControlModel >& rxControlModel);

            /** constructor with an additional XControl as parameter to allow to hand it over at incarnation time
                if it exists. This will avoid to create a 2nd one on demand in createXControl()
                and thus double the XControls.
             */
            ControlPrimitive2D(
                const basegfx::B2DHomMatrix& rTransform,
                const com::sun::star::uno::Reference< com::sun::star::awt::XControlModel >& rxControlModel,
                const com::sun::star::uno::Reference< com::sun::star::awt::XControl >& rxXControl);

            /// data read access
            const basegfx::B2DHomMatrix& getTransform() const { return maTransform; }
            const com::sun::star::uno::Reference< com::sun::star::awt::XControlModel >& getControlModel() const { return mxControlModel; }

            /** mxControl access. This will on demand create the awt::XControl using createXControl()
                if it does not exist. It may already have been created or even handed over at
                incarnation
             */
            const com::sun::star::uno::Reference< com::sun::star::awt::XControl >& getXControl() const;

            /// compare operator
            virtual bool operator==(const BasePrimitive2D& rPrimitive) const;

            /// get range
            virtual basegfx::B2DRange getB2DRange(const geometry::ViewInformation2D& rViewInformation) const;

            /// provide unique ID
            DeclPrimitrive2DIDBlock()

            /// Overload standard getDecomposition call to be view-dependent here
            virtual Primitive2DSequence get2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const;
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif // INCLUDED_DRAWINGLAYER_PRIMITIVE2D_CONTROLPRIMITIVE2D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
