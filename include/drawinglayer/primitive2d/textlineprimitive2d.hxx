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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_TEXTLINEPRIMITIVE2D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE2D_TEXTLINEPRIMITIVE2D_HXX

#include <drawinglayer/drawinglayerdllapi.h>

#include <drawinglayer/primitive2d/baseprimitive2d.hxx>
#include <drawinglayer/primitive2d/textenumsprimitive2d.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/color/bcolor.hxx>



namespace drawinglayer
{
    namespace primitive2d
    {
        class DRAWINGLAYER_DLLPUBLIC TextLinePrimitive2D : public BufferedDecompositionPrimitive2D
        {
        private:
            /// geometric definitions
            basegfx::B2DHomMatrix                       maObjectTransformation;
            double                                      mfWidth;
            double                                      mfOffset;
            double                                      mfHeight;

            /// decoration definitions
            TextLine                                    meTextLine;
            basegfx::BColor                             maLineColor;

        protected:
            /// local decomposition.
            virtual Primitive2DSequence create2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const override;

        public:
            /// constructor
            TextLinePrimitive2D(
                const basegfx::B2DHomMatrix& rObjectTransformation,
                double fWidth,
                double fOffset,
                double fHeight,
                TextLine eTextLine,
                const basegfx::BColor& rLineColor);

            /// data read access
            const basegfx::B2DHomMatrix& getObjectTransformation() const { return maObjectTransformation; }
            double getWidth() const { return mfWidth; }
            double getOffset() const { return mfOffset; }
            double getHeight() const { return mfHeight; }
            TextLine getTextLine() const { return meTextLine; }
            const basegfx::BColor& getLineColor() const { return maLineColor; }

            /// compare operator
            virtual bool operator==( const BasePrimitive2D& rPrimitive ) const override;

            /// provide unique ID
            DeclPrimitive2DIDBlock()
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer



#endif //INCLUDED_DRAWINGLAYER_PRIMITIVE2D_TEXTLINEPRIMITIVE2D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
