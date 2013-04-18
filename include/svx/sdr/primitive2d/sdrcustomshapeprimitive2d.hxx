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

#ifndef INCLUDED_SDR_PRIMITIVE2D_SDCUSTOMSHAPEPRIMITIVE2D_HXX
#define INCLUDED_SDR_PRIMITIVE2D_SDCUSTOMSHAPEPRIMITIVE2D_HXX

#include <drawinglayer/primitive2d/baseprimitive2d.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <svx/sdr/attribute/sdrshadowtextattribute.hxx>

//////////////////////////////////////////////////////////////////////////////
// predefines

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        class SdrCustomShapePrimitive2D : public BufferedDecompositionPrimitive2D
        {
        private:
            attribute::SdrShadowTextAttribute           maSdrSTAttribute;
            Primitive2DSequence                         maSubPrimitives;
            basegfx::B2DHomMatrix                       maTextBox;

            // bitfield
            // defines if SdrTextWordWrapItem was set at SdrObjCustomShape which means
            // that the text needs to be block formatted
            unsigned                                    mbWordWrap : 1;

            // defines that the object contains/is a 3D AutoShape. Needed for
            // making exceptions with shadow generation
            unsigned                                    mb3DShape : 1;

            // #SJ# Allow text clipping against TextBox in special cases (used for SC)
            unsigned                                    mbForceTextClipToTextRange : 1;

        protected:
            // local decomposition.
            virtual Primitive2DSequence create2DDecomposition(const geometry::ViewInformation2D& aViewInformation) const;

        public:
            SdrCustomShapePrimitive2D(
                const attribute::SdrShadowTextAttribute& rSdrSTAttribute,
                const Primitive2DSequence& rSubPrimitives,
                const basegfx::B2DHomMatrix& rTextBox,
                bool bWordWrap,
                bool b3DShape,
                bool bForceTextClipToTextRange);

            // data access
            const attribute::SdrShadowTextAttribute& getSdrSTAttribute() const { return maSdrSTAttribute; }
            const Primitive2DSequence& getSubPrimitives() const { return maSubPrimitives; }
            const basegfx::B2DHomMatrix& getTextBox() const { return maTextBox; }
            bool getWordWrap() const { return mbWordWrap; }
            bool get3DShape() const { return mb3DShape; }
            bool isForceTextClipToTextRange() const { return mbForceTextClipToTextRange; }

            // compare operator
            virtual bool operator==(const BasePrimitive2D& rPrimitive) const;

            // provide unique ID
            DeclPrimitrive2DIDBlock()
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_SDR_PRIMITIVE2D_SDCUSTOMSHAPEPRIMITIVE2D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
