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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_WRONGSPELLPRIMITIVE2D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE2D_WRONGSPELLPRIMITIVE2D_HXX

#include <drawinglayer/drawinglayerdllapi.h>

#include <drawinglayer/primitive2d/baseprimitive2d.hxx>
#include <basegfx/color/bcolor.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>

//////////////////////////////////////////////////////////////////////////////
// WrongSpellPrimitive2D class

namespace drawinglayer
{
    namespace primitive2d
    {
        /** WrongSpellPrimitive2D class

            This is a helper primitive to hold evtl. WrongSpell visualisations
            in the sequence of primitives. The primitive holds this information
            separated form the TextPortions to where it belongs to tot expand the
            TextSimplePortionPrimitive2D more as needed.

            A renderer who does not want to visualize this (if contained at all)
            can detect and ignore this primitive. If it's decomposition is used,
            it will be visualized as red wavelines.

            The geometric definition defines a line on the X-Axis (no Y-coordinates)
            which will when transformed by Transformation, create the coordinate data.
         */
        class DRAWINGLAYER_DLLPUBLIC WrongSpellPrimitive2D : public BufferedDecompositionPrimitive2D
        {
        private:
            /// geometry definition
            basegfx::B2DHomMatrix                           maTransformation;
            double                                          mfStart;
            double                                          mfStop;

            /// color (usually red)
            basegfx::BColor                                 maColor;

        protected:
            /// create local decomposition
            virtual Primitive2DSequence create2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const;

        public:
            /// constructor
            WrongSpellPrimitive2D(
                const basegfx::B2DHomMatrix& rTransformation,
                double fStart,
                double fStop,
                const basegfx::BColor& rColor);

            /// data read access
            const basegfx::B2DHomMatrix& getTransformation() const { return maTransformation; }
            double getStart() const { return mfStart; }
            double getStop() const { return mfStop; }
            const basegfx::BColor& getColor() const { return maColor; }

            /// compare operator
            virtual bool operator==(const BasePrimitive2D& rPrimitive) const;

            /// provide unique ID
            DeclPrimitrive2DIDBlock()
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_PRIMITIVE2D_WRONGSPELLPRIMITIVE2D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
