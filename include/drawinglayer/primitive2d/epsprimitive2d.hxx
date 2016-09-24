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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_EPSPRIMITIVE2D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE2D_EPSPRIMITIVE2D_HXX

#include <drawinglayer/drawinglayerdllapi.h>

#include <drawinglayer/primitive2d/baseprimitive2d.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <vcl/gfxlink.hxx>
#include <vcl/gdimtf.hxx>

namespace drawinglayer
{
    namespace primitive2d
    {
        /** EpsPrimitive2D class */
        class DRAWINGLAYER_DLLPUBLIC EpsPrimitive2D : public BufferedDecompositionPrimitive2D
        {
        private:
            /// the geometry definition
            basegfx::B2DHomMatrix                       maEpsTransform;

            /// the Eps content definition
            GfxLink                                     maGfxLink;

            /// the replacement content definition
            GDIMetaFile                                 maMetaFile;

        protected:
            /// create local decomposition
            virtual Primitive2DContainer create2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const override;

        public:
            /// constructor
            EpsPrimitive2D(
                const basegfx::B2DHomMatrix& rEpsTransform,
                const GfxLink& rGfxLink,
                const GDIMetaFile& rMetaFile);

            /// data read access
            const basegfx::B2DHomMatrix& getEpsTransform() const { return maEpsTransform; }
            const GfxLink& getGfxLink() const { return maGfxLink; }
            const GDIMetaFile& getMetaFile() const { return maMetaFile; }

            /// compare operator
            virtual bool operator==(const BasePrimitive2D& rPrimitive) const override;

            /// get B2Drange
            virtual basegfx::B2DRange getB2DRange(const geometry::ViewInformation2D& rViewInformation) const override;

            /// provide unique ID
            DeclPrimitive2DIDBlock()
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer

#endif //INCLUDED_DRAWINGLAYER_PRIMITIVE2D_EPSPRIMITIVE2D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
