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

#ifndef INCLUDED_DRAWINGLAYER_PROCESSOR3D_CUTFINDPROCESSOR3D_HXX
#define INCLUDED_DRAWINGLAYER_PROCESSOR3D_CUTFINDPROCESSOR3D_HXX

#include <drawinglayer/processor3d/defaultprocessor3d.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace processor3d
    {
        class CutFindProcessor : public BaseProcessor3D
        {
        private:
            // the start and stop point for the cut vector
            basegfx::B3DPoint                       maFront;
            basegfx::B3DPoint                       maBack;

            // the found cut points
            ::std::vector< basegfx::B3DPoint >      maResult;

            // #i102956# the transformation change from TransformPrimitive3D processings
            // needs to be remembered to be able to transform found cuts to the
            // basic coordinate system the processor starts with
            basegfx::B3DHomMatrix                   maCombinedTransform;

            // bitfield
            bool                                    mbAnyHit : 1;

            // as tooling, the process() implementation takes over API handling and calls this
            // virtual render method when the primitive implementation is BasePrimitive3D-based.
            virtual void processBasePrimitive3D(const primitive3d::BasePrimitive3D& rCandidate);

        public:
            CutFindProcessor(const geometry::ViewInformation3D& rViewInformation,
                const basegfx::B3DPoint& rFront,
                const basegfx::B3DPoint& rBack,
                bool bAnyHit);

            // data access
            const ::std::vector< basegfx::B3DPoint >& getCutPoints() const { return maResult; }
            bool getAnyHit() const { return mbAnyHit; }
        };
    } // end of namespace processor3d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_PROCESSOR3D_CUTFINDPROCESSOR3D_HXX

// eof
