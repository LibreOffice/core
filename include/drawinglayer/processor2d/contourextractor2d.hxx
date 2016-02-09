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

#ifndef INCLUDED_DRAWINGLAYER_PROCESSOR2D_CONTOUREXTRACTOR2D_HXX
#define INCLUDED_DRAWINGLAYER_PROCESSOR2D_CONTOUREXTRACTOR2D_HXX

#include <drawinglayer/drawinglayerdllapi.h>

#include <drawinglayer/processor2d/baseprocessor2d.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>


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
            basegfx::B2DPolyPolygonVector       maExtractedContour;

            /// bitfield
            bool                                mbExtractFillOnly : 1;

            /// tooling methods
            void processBasePrimitive2D(const primitive2d::BasePrimitive2D& rCandidate) override;

        public:
            explicit ContourExtractor2D(
                const geometry::ViewInformation2D& rViewInformation,
                bool bExtractFillOnly);

            virtual ~ContourExtractor2D();

            const basegfx::B2DPolyPolygonVector& getExtractedContour() const { return maExtractedContour; }
        };
    } // end of namespace processor2d
} // end of namespace drawinglayer


#endif // INCLUDED_DRAWINGLAYER_PROCESSOR2D_CONTOUREXTRACTOR2D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
