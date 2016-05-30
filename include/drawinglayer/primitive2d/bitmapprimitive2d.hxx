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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_BITMAPPRIMITIVE2D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE2D_BITMAPPRIMITIVE2D_HXX

#include <drawinglayer/drawinglayerdllapi.h>

#include <drawinglayer/primitive2d/baseprimitive2d.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/virdev.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>


// BitmapPrimitive2D class

namespace drawinglayer
{
    namespace primitive2d
    {
        class DRAWINGLAYER_DLLPUBLIC BitmapPrimitive2DBase : public BasePrimitive2D
        {
        protected:
            /** the object transformation from unit coordinates, defining
                size, shear, rotate and position
             */
            basegfx::B2DHomMatrix                       maTransform;

        public:
            /// constructor
            BitmapPrimitive2DBase(const basegfx::B2DHomMatrix& rTransform);

            /// data read access
            virtual BitmapEx getBitmapEx() const = 0;
            const basegfx::B2DHomMatrix& getTransform() const { return maTransform; }

            /// compare operator
            virtual bool operator==(const BasePrimitive2D& rPrimitive) const override;

            /// get range
            virtual basegfx::B2DRange getB2DRange(const geometry::ViewInformation2D& rViewInformation) const override;

            /// provide unique ID
            DeclPrimitive2DIDBlock()
        };

        /** BitmapPrimitive2D class

            This class is the central primitive for Bitmap-based primitives.
            It provides RGBA-based bitmaps, currently using a BitmapEx from VCL.
            This may change in the future to any other, maybe more general base
            class providing 24bit RGBA.
         */
        class DRAWINGLAYER_DLLPUBLIC BitmapPrimitive2D : public BitmapPrimitive2DBase
        {
        private:
            /// the RGBA Bitmap-data
            BitmapEx                                    maBitmapEx;

        public:
            /// constructor
            BitmapPrimitive2D(
                const BitmapEx& rBitmapEx,
                const basegfx::B2DHomMatrix& rTransform);

            /// data read access
            virtual BitmapEx getBitmapEx() const override { return maBitmapEx; }
        };

        /** BitmapPrimitive2D class

            This class provides on-demand access to a specific frame of an animated gif
         */
        class DRAWINGLAYER_DLLPUBLIC AnimationFrameBitmapPrimitive2D : public BitmapPrimitive2DBase
        {
        private:
            //SharedPtrs to virtual devices shared among the frames of an animated gif
            VclPtr<VirtualDevice> maVirtualDevice;
            VclPtr<VirtualDevice> maVirtualDeviceMask;
            mutable std::unique_ptr<BitmapEx> mxCachedFrame0BitmapEx;
            mutable std::shared_ptr<sal_Int32> mxLastDrawnIndex;
            ::Animation           maAnimation;
            sal_Int32             mnIndex;
        public:
            /// constructor
            AnimationFrameBitmapPrimitive2D(
                const ::Animation& rAnimation,
                const VclPtr<VirtualDevice> rVirtualDevice,
                const VclPtr<VirtualDevice> rVirtualDeviceMask,
                const basegfx::B2DHomMatrix& rTransform,
                std::shared_ptr<sal_Int32>& rLastDrawnIndex,
                sal_Int32                   nIndex);

            virtual BitmapEx getBitmapEx() const override;
        };

    } // end of namespace primitive2d
} // end of namespace drawinglayer


#endif // INCLUDED_DRAWINGLAYER_PRIMITIVE2D_BITMAPPRIMITIVE2D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
