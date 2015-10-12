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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_PATTERNFILLPRIMITIVE2D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE2D_PATTERNFILLPRIMITIVE2D_HXX

#include <drawinglayer/drawinglayerdllapi.h>
#include <drawinglayer/primitive2d/baseprimitive2d.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>



namespace drawinglayer
{
    namespace primitive2d
    {
        /** PatternFillPrimitive2D class

            This primitive allows filling a given tools::PolyPolygon with a pattern
            defined by a sequence of primitives which are mapped to  the unit range.
            The pattern is defined using a reference range which defines a rectangular
            area relative to the tools::PolyPolygon (in unit coordinates) which is virtually
            infinitely repeated.
         */
        class DRAWINGLAYER_DLLPUBLIC PatternFillPrimitive2D : public BufferedDecompositionPrimitive2D
        {
        private:
            const basegfx::B2DPolyPolygon       maMask;
            const Primitive2DSequence           maChildren;
            const basegfx::B2DRange             maReferenceRange;

        protected:
            /// create local decomposition
            virtual Primitive2DSequence create2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const override;

        public:
            /// constructor
            PatternFillPrimitive2D(
                const basegfx::B2DPolyPolygon& rMask,
                const Primitive2DSequence& rChildren,
                const basegfx::B2DRange& rReferenceRange);

            /// data read access
            const basegfx::B2DPolyPolygon& getMask() const { return maMask; }
            const Primitive2DSequence& getChildren() const { return maChildren; }
            const basegfx::B2DRange& getReferenceRange() const { return maReferenceRange; }

            /// compare operator
            virtual bool operator==(const BasePrimitive2D& rPrimitive) const override;

            /// get range
            virtual basegfx::B2DRange getB2DRange(const geometry::ViewInformation2D& rViewInformation) const override;

            /// provide unique ID
            DeclPrimitive2DIDBlock()
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer



#endif //INCLUDED_DRAWINGLAYER_PRIMITIVE2D_PATTERNFILLPRIMITIVE2D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
