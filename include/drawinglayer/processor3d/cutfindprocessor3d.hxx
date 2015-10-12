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

#ifndef INCLUDED_DRAWINGLAYER_PROCESSOR3D_CUTFINDPROCESSOR3D_HXX
#define INCLUDED_DRAWINGLAYER_PROCESSOR3D_CUTFINDPROCESSOR3D_HXX

#include <drawinglayer/drawinglayerdllapi.h>

#include <drawinglayer/processor3d/defaultprocessor3d.hxx>



namespace drawinglayer
{
    namespace processor3d
    {
        /** CutFindProcessor class

            This processor extracts all cuts of 3D plane geometries in the feeded primitives
            with the given cut vector, based on the ViewInformation3D given.
         */
        class DRAWINGLAYER_DLLPUBLIC CutFindProcessor : public BaseProcessor3D
        {
        private:
            /// the start and stop point for the cut vector
            basegfx::B3DPoint                       maFront;
            basegfx::B3DPoint                       maBack;

            /// the found cut points
            ::std::vector< basegfx::B3DPoint >      maResult;

            /*  #i102956# the transformation change from TransformPrimitive3D processings
                needs to be remembered to be able to transform found cuts to the
                basic coordinate system the processor starts with
             */
            basegfx::B3DHomMatrix                   maCombinedTransform;

            /// bitfield
            bool                                    mbAnyHit : 1;

            /*  this flag decides if primitives which are invisible will be taken into account for
                HitTesting or not.
             */
            bool                                    mbUseInvisiblePrimitiveContent : 1;

            /*  as tooling, the process() implementation takes over API handling and calls this
                virtual render method when the primitive implementation is BasePrimitive3D-based.
             */
            virtual void processBasePrimitive3D(const primitive3d::BasePrimitive3D& rCandidate) override;

        public:
            CutFindProcessor(const geometry::ViewInformation3D& rViewInformation,
                const basegfx::B3DPoint& rFront,
                const basegfx::B3DPoint& rBack,
                bool bAnyHit);

            /// data read access
            const ::std::vector< basegfx::B3DPoint >& getCutPoints() const { return maResult; }
            bool getAnyHit() const { return mbAnyHit; }
            bool getUseInvisiblePrimitiveContent() const { return mbUseInvisiblePrimitiveContent;}
        };
    } // end of namespace processor3d
} // end of namespace drawinglayer



#endif //INCLUDED_DRAWINGLAYER_PROCESSOR3D_CUTFINDPROCESSOR3D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
