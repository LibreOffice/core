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

#ifndef INCLUDED_DRAWINGLAYER_PROCESSOR2D_LINEGEOMETRYEXTRACTOR2D_HXX
#define INCLUDED_DRAWINGLAYER_PROCESSOR2D_LINEGEOMETRYEXTRACTOR2D_HXX

#include <drawinglayer/drawinglayerdllapi.h>

#include <drawinglayer/processor2d/baseprocessor2d.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace processor2d
    {
        /** LineGeometryExtractor2D class

            This processor can extract the line geometry from feeded primpitives. The
            hairlines and the fille geometry from fat lines are separated.
         */
        class DRAWINGLAYER_DLLPUBLIC LineGeometryExtractor2D : public BaseProcessor2D
        {
        private:
            std::vector< basegfx::B2DPolygon >      maExtractedHairlines;
            std::vector< basegfx::B2DPolyPolygon >  maExtractedLineFills;

            /// bitfield
            unsigned                                mbInLineGeometry : 1;

            /// tooling methods
            void processBasePrimitive2D(const primitive2d::BasePrimitive2D& rCandidate);

        public:
            LineGeometryExtractor2D(const geometry::ViewInformation2D& rViewInformation);
            virtual ~LineGeometryExtractor2D();

            const std::vector< basegfx::B2DPolygon >& getExtractedHairlines() const { return maExtractedHairlines; }
            const std::vector< basegfx::B2DPolyPolygon >& getExtractedLineFills() const { return maExtractedLineFills; }
        };
    } // end of namespace processor2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif // INCLUDED_DRAWINGLAYER_PROCESSOR2D_LINEGEOMETRYEXTRACTOR2D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
