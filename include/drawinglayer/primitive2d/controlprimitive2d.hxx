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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_CONTROLPRIMITIVE2D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE2D_CONTROLPRIMITIVE2D_HXX

#include <drawinglayer/drawinglayerdllapi.h>

#include <drawinglayer/primitive2d/baseprimitive2d.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <com/sun/star/awt/XControlModel.hpp>
#include <com/sun/star/awt/XControl.hpp>



namespace drawinglayer
{
    namespace primitive2d
    {
        /** ControlPrimitive2D class

            Base class for ControlPrimitive handling. It decoposes to a
            graphical representation (Bitmap data) of the control. This
            representation is limited to a quadratic pixel maximum defined
            in the application settings.
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
            virtual Primitive2DSequence create2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const override;

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
            virtual bool operator==(const BasePrimitive2D& rPrimitive) const override;

            /// get range
            virtual basegfx::B2DRange getB2DRange(const geometry::ViewInformation2D& rViewInformation) const override;

            /// provide unique ID
            DeclPrimitive2DIDBlock()

            /// Override standard getDecomposition to be view-dependent here
            virtual Primitive2DSequence get2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const override;
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer



#endif // INCLUDED_DRAWINGLAYER_PRIMITIVE2D_CONTROLPRIMITIVE2D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
