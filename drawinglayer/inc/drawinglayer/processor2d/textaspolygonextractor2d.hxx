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

#ifndef INCLUDED_DRAWINGLAYER_PROCESSOR2D_TEXTASPOLYGONEXTRACTOR2D_HXX
#define INCLUDED_DRAWINGLAYER_PROCESSOR2D_TEXTASPOLYGONEXTRACTOR2D_HXX

#include <drawinglayer/drawinglayerdllapi.h>

#include <drawinglayer/processor2d/baseprocessor2d.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/color/bcolor.hxx>
#include <basegfx/color/bcolormodifier.hxx>
#include <vector>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace processor2d
    {
        /// helper data structure for returning the result
        struct TextAsPolygonDataNode
        {
        private:
            basegfx::B2DPolyPolygon                 maB2DPolyPolygon;
            basegfx::BColor                         maBColor;
            bool                                    mbIsFilled;

        public:
            TextAsPolygonDataNode(
                const basegfx::B2DPolyPolygon& rB2DPolyPolygon,
                const basegfx::BColor& rBColor,
                bool bIsFilled)
            :   maB2DPolyPolygon(rB2DPolyPolygon),
                maBColor(rBColor),
                mbIsFilled(bIsFilled)
            {
            }

            // data read access
            const basegfx::B2DPolyPolygon& getB2DPolyPolygon() const { return maB2DPolyPolygon; }
            const basegfx::BColor& getBColor() const { return maBColor; }
            bool getIsFilled() const { return mbIsFilled; }
        };

        /// typedef for a vector of that helper data
        typedef ::std::vector< TextAsPolygonDataNode > TextAsPolygonDataNodeVector;

        /** TextAsPolygonExtractor2D class

            This processor extracts text in the feeded primitives to filled polygons
         */
        class DRAWINGLAYER_DLLPUBLIC TextAsPolygonExtractor2D : public BaseProcessor2D
        {
        private:
            // extraction target
            TextAsPolygonDataNodeVector             maTarget;

            // the modifiedColorPrimitive stack
            basegfx::BColorModifierStack            maBColorModifierStack;

            // flag if we are in a decomposed text
            sal_uInt32                              mnInText;

            // tooling methods
            void processBasePrimitive2D(const primitive2d::BasePrimitive2D& rCandidate);

        public:
            explicit TextAsPolygonExtractor2D(const geometry::ViewInformation2D& rViewInformation);
            virtual ~TextAsPolygonExtractor2D();

            // data read access
            const TextAsPolygonDataNodeVector& getTarget() const { return maTarget; }
        };
    } // end of namespace processor2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif // INCLUDED_DRAWINGLAYER_PROCESSOR2D_TEXTASPOLYGONEXTRACTOR2D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
