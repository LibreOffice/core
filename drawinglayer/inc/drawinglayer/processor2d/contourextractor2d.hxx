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

#ifndef INCLUDED_DRAWINGLAYER_PROCESSOR2D_CONTOUREXTRACTOR2D_HXX
#define INCLUDED_DRAWINGLAYER_PROCESSOR2D_CONTOUREXTRACTOR2D_HXX

#include <drawinglayer/drawinglayerdllapi.h>

#include <drawinglayer/processor2d/baseprocessor2d.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace processor2d
    {
        /** ContourExtractor2D class

            A processor who extracts the contour of the primtives feeded to it
            in the single local PolyPolygon
         */
        class DRAWINGLAYER_DLLPUBLIC ContourExtractor2D : public BaseProcessor2D
        {
        private:
            /// the extracted contour
            std::vector< basegfx::B2DPolyPolygon >  maExtractedContour;

            /// tooling methods
            void processBasePrimitive2D(const primitive2d::BasePrimitive2D& rCandidate);

        public:
            explicit ContourExtractor2D(const geometry::ViewInformation2D& rViewInformation);
            virtual ~ContourExtractor2D();

            const std::vector< basegfx::B2DPolyPolygon >& getExtractedContour() const { return maExtractedContour; }
        };
    } // end of namespace processor2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif // INCLUDED_DRAWINGLAYER_PROCESSOR2D_CONTOUREXTRACTOR2D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
