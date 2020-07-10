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

#ifndef INCLUDED_DRAWINGLAYER_PROCESSOR2D_LINEGEOMETRYEXTRACTOR2D_HXX
#define INCLUDED_DRAWINGLAYER_PROCESSOR2D_LINEGEOMETRYEXTRACTOR2D_HXX

#include <drawinglayer/drawinglayerdllapi.h>

#include <drawinglayer/processor2d/baseprocessor2d.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>


namespace drawinglayer::processor2d
    {
        /** LineGeometryExtractor2D class

            This processor can extract the line geometry from fed primitives. The
            hairlines and the fill geometry from fat lines are separated.
         */
        class DRAWINGLAYER_DLLPUBLIC LineGeometryExtractor2D final : public BaseProcessor2D
        {
        private:
            basegfx::B2DPolygonVector               maExtractedHairlines;
            basegfx::B2DPolyPolygonVector           maExtractedLineFills;

            bool                                    mbInLineGeometry : 1;

            /// tooling methods
            void processBasePrimitive2D(const primitive2d::BasePrimitive2D& rCandidate) override;

        public:
            LineGeometryExtractor2D(const geometry::ViewInformation2D& rViewInformation);
            virtual ~LineGeometryExtractor2D() override;

            const basegfx::B2DPolygonVector& getExtractedHairlines() const { return maExtractedHairlines; }
            const basegfx::B2DPolyPolygonVector& getExtractedLineFills() const { return maExtractedLineFills; }
        };

} // end of namespace drawinglayer::processor2d


#endif // INCLUDED_DRAWINGLAYER_PROCESSOR2D_LINEGEOMETRYEXTRACTOR2D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
