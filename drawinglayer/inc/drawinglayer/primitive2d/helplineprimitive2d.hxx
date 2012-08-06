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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_HELPLINEPRIMITIVE2D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE2D_HELPLINEPRIMITIVE2D_HXX

#include <drawinglayer/drawinglayerdllapi.h>

#include <drawinglayer/primitive2d/baseprimitive2d.hxx>
#include <basegfx/color/bcolor.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>

//////////////////////////////////////////////////////////////////////////////
// HelplinePrimitive2D class

namespace drawinglayer
{
    namespace primitive2d
    {
        /** HelplineStyle2D definition

            The available styles of Helplines
        */
        enum HelplineStyle2D
        {
            HELPLINESTYLE2D_POINT,
            HELPLINESTYLE2D_LINE
        };

        /** HelplinePrimitive2D class

            This primitive provides a view-dependent helpline definition. The Helpline
            is defined by a line equation (Point and vector) and a style. When the style
            is a line, dependent from Viewport the visible part of that Helpline is
            constructed. For Point, a cross is constructed. This primitive is highly
            view-dependent.

            The visualisation uses the two given colors to create a dashed line with
            the given dash length.
         */
        class DRAWINGLAYER_DLLPUBLIC HelplinePrimitive2D : public BufferedDecompositionPrimitive2D
        {
        private:
            /// Helpline geometry definition
            basegfx::B2DPoint                               maPosition;
            basegfx::B2DVector                              maDirection;
            HelplineStyle2D                                 meStyle;

            /// Helpline style definition
            basegfx::BColor                                 maRGBColA;
            basegfx::BColor                                 maRGBColB;
            double                                          mfDiscreteDashLength;

            /** the last used object to view transformtion and the last Viewport,
                used from getDecomposition for decide buffering
             */
            basegfx::B2DHomMatrix                           maLastObjectToViewTransformation;
            basegfx::B2DRange                               maLastViewport;

        protected:
            /// create local decomposition
            virtual Primitive2DSequence create2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const;

        public:
            /// constructor
            HelplinePrimitive2D(
                const basegfx::B2DPoint& rPosition,
                const basegfx::B2DVector& rDirection,
                HelplineStyle2D eStyle,
                const basegfx::BColor& rRGBColA,
                const basegfx::BColor& aRGBColB,
                double fDiscreteDashLength);

            /// data read access
            const basegfx::B2DPoint& getPosition() const { return maPosition; }
            const basegfx::B2DVector& getDirection() const { return maDirection; }
            HelplineStyle2D getStyle() const { return meStyle; }
            const basegfx::BColor& getRGBColA() const { return maRGBColA; }
            const basegfx::BColor& getRGBColB() const { return maRGBColB; }
            double getDiscreteDashLength() const { return mfDiscreteDashLength; }

            /// compare operator
            virtual bool operator==(const BasePrimitive2D& rPrimitive) const;

            /// provide unique ID
            DeclPrimitrive2DIDBlock()

            /// Overload standard getDecomposition call to be view-dependent here
            virtual Primitive2DSequence get2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const;
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_PRIMITIVE2D_HELPLINEPRIMITIVE2D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
