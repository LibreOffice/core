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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE3D_SDREXTRUDELATHETOOLS3D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE3D_SDREXTRUDELATHETOOLS3D_HXX

#include <drawinglayer/drawinglayerdllapi.h>

#include <basegfx/polygon/b3dpolypolygon.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <vector>


// predefines

namespace drawinglayer { namespace geometry {
    class ViewInformation3D;
}}



namespace drawinglayer
{
    namespace primitive3d
    {
        /** SliceType3D definition */
        enum SliceType3D
        {
            SLICETYPE3D_REGULAR,        // normal geometry Slice3D
            SLICETYPE3D_FRONTCAP,       // front cap
            SLICETYPE3D_BACKCAP         // back cap
        };

        /// class to hold one Slice3D
        class DRAWINGLAYER_DLLPUBLIC Slice3D
        {
        protected:
            basegfx::B3DPolyPolygon                 maPolyPolygon;
            SliceType3D                             maSliceType;

        public:
            Slice3D(
                const basegfx::B2DPolyPolygon& rPolyPolygon,
                const basegfx::B3DHomMatrix& aTransform,
                SliceType3D aSliceType = SLICETYPE3D_REGULAR)
            :   maPolyPolygon(basegfx::tools::createB3DPolyPolygonFromB2DPolyPolygon(rPolyPolygon)),
                maSliceType(aSliceType)
            {
                maPolyPolygon.transform(aTransform);
            }

            explicit Slice3D(
                const basegfx::B3DPolyPolygon& rPolyPolygon,
                SliceType3D aSliceType = SLICETYPE3D_REGULAR)
            :   maPolyPolygon(rPolyPolygon),
                maSliceType(aSliceType)
            {
            }

            // data access
            const basegfx::B3DPolyPolygon& getB3DPolyPolygon() const { return maPolyPolygon; }
            SliceType3D getSliceType() const { return maSliceType; }
        };

        /// typedef for a group of Slice3Ds
        typedef ::std::vector< Slice3D > Slice3DVector;

        /// helpers for creation
        void DRAWINGLAYER_DLLPUBLIC createLatheSlices(
            Slice3DVector& rSliceVector,
            const basegfx::B2DPolyPolygon& rSource,
            double fBackScale,
            double fDiagonal,
            double fRotation,
            sal_uInt32 nSteps,
            bool bCharacterMode,
            bool bCloseFront,
            bool bCloseBack);

        void DRAWINGLAYER_DLLPUBLIC createExtrudeSlices(
            Slice3DVector& rSliceVector,
            const basegfx::B2DPolyPolygon& rSource,
            double fBackScale,
            double fDiagonal,
            double fDepth,
            bool bCharacterMode,
            bool bCloseFront,
            bool bCloseBack);

        /// helpers for geometry extraction
        basegfx::B3DPolyPolygon DRAWINGLAYER_DLLPUBLIC extractHorizontalLinesFromSlice(const Slice3DVector& rSliceVector, bool bCloseHorLines);
        basegfx::B3DPolyPolygon DRAWINGLAYER_DLLPUBLIC extractVerticalLinesFromSlice(const Slice3DVector& rSliceVector);

        void DRAWINGLAYER_DLLPUBLIC extractPlanesFromSlice(
            ::std::vector< basegfx::B3DPolyPolygon >& rFill,
            const Slice3DVector& rSliceVector,
            bool bCreateNormals,
            bool bSmoothHorizontalNormals,
            bool bSmoothNormals,
            bool bSmoothLids,
            bool bClosed,
            double fSmoothNormalsMix,
            double fSmoothLidsMix,
            bool bCreateTextureCoordinates,
            const basegfx::B2DHomMatrix& rTexTransform);

        void DRAWINGLAYER_DLLPUBLIC createReducedOutlines(
            const geometry::ViewInformation3D& rViewInformation,
            const basegfx::B3DHomMatrix& rObjectTransform,
            const basegfx::B3DPolygon& rLoopA,
            const basegfx::B3DPolygon& rLoopB,
            basegfx::B3DPolyPolygon& rTarget);

    } // end of namespace overlay
} // end of namespace drawinglayer



#endif //_DRAWINGLAYER_PRIMITIVE3D_SDREXTRUDELATHETOOLS3D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
