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

#ifndef INCLUDED_SVX_INC_SDR_PRIMITIVE2D_SDRCUSTOMSHAPEPRIMITIVE2D_HXX
#define INCLUDED_SVX_INC_SDR_PRIMITIVE2D_SDRCUSTOMSHAPEPRIMITIVE2D_HXX

#include <drawinglayer/primitive2d/baseprimitive2d.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <sdr/attribute/sdreffectstextattribute.hxx>


// predefines


namespace drawinglayer::primitive2d
    {
        class SdrCustomShapePrimitive2D : public BufferedDecompositionPrimitive2D
        {
        private:
            attribute::SdrEffectsTextAttribute           maSdrSTAttribute;
            Primitive2DContainer                         maSubPrimitives;
            basegfx::B2DHomMatrix                       maTextBox;

            // defines if SdrTextWordWrapItem was set at SdrObjCustomShape which means
            // that the text needs to be block formatted
            bool                                        mbWordWrap : 1;

            // defines that the object contains/is a 3D AutoShape. Needed for
            // making exceptions with shadow generation
            bool                                        mb3DShape : 1;

            basegfx::B2DHomMatrix maTransform;

        protected:
            // local decomposition.
            virtual void create2DDecomposition(Primitive2DContainer& rContainer, const geometry::ViewInformation2D& aViewInformation) const override;

        public:
            SdrCustomShapePrimitive2D(
                const attribute::SdrEffectsTextAttribute& rSdrSTAttribute,
                const Primitive2DContainer& rSubPrimitives,
                const basegfx::B2DHomMatrix& rTextBox,
                bool bWordWrap,
                bool b3DShape,
                const basegfx::B2DHomMatrix& rObjectMatrix);

            // data access
            const attribute::SdrEffectsTextAttribute& getSdrSTAttribute() const { return maSdrSTAttribute; }
            const Primitive2DContainer& getSubPrimitives() const { return maSubPrimitives; }
            const basegfx::B2DHomMatrix& getTextBox() const { return maTextBox; }
            bool getWordWrap() const { return mbWordWrap; }
            bool get3DShape() const { return mb3DShape; }

            // compare operator
            virtual bool operator==(const BasePrimitive2D& rPrimitive) const override;

            // provide unique ID
            virtual sal_uInt32 getPrimitive2DID() const override;
        };
} // end of namespace drawinglayer::primitive2d


#endif // INCLUDED_SVX_INC_SDR_PRIMITIVE2D_SDRCUSTOMSHAPEPRIMITIVE2D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
