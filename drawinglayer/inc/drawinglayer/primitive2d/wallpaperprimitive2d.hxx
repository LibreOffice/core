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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_WALLPAPERPRIMITIVE2D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE2D_WALLPAPERPRIMITIVE2D_HXX

#include <drawinglayer/drawinglayerdllapi.h>

#include <drawinglayer/primitive2d/primitivetools2d.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/wall.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        /** WallpaperBitmapPrimitive2D class

            This is a specialized primtive for the Wallpaper definitions included in
            VCL and Metafiles. The extraordinary about the bitmap definition part of
            the Wallpaper is that it uses PIXEL size of the given Bitmap and not
            the logic and/or discrete size derived by PrefMapMode/PrefSize methods.
            To emulate this, a ViewTransformation dependent primitive is needed which
            takes over the correct scaling(s).

            Since a specialized primitive is needed anyways, i opted to also add the
            layouting which is dependent from WallpaperStyle; thus it does not need
            to be handled anywhere else in the future.
         */
        class DRAWINGLAYER_DLLPUBLIC WallpaperBitmapPrimitive2D : public ViewTransformationDependentPrimitive2D
        {
        private:
            basegfx::B2DRange                   maObjectRange;
            BitmapEx                            maBitmapEx;
            WallpaperStyle                      meWallpaperStyle;

        protected:
            /// create local decomposition
            virtual Primitive2DSequence create2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const;

        public:
            /// constructor
            WallpaperBitmapPrimitive2D(
                const basegfx::B2DRange& rObjectRange,
                const BitmapEx& rBitmapEx,
                WallpaperStyle eWallpaperStyle);

            /// data read access
            const basegfx::B2DRange& getLocalObjectRange() const { return maObjectRange; }
            const BitmapEx& getBitmapEx() const { return maBitmapEx ; }
            WallpaperStyle getWallpaperStyle() const { return meWallpaperStyle; }

            /// compare operator
            virtual bool operator==(const BasePrimitive2D& rPrimitive) const;

            /// get B2Drange
            virtual basegfx::B2DRange getB2DRange(const geometry::ViewInformation2D& rViewInformation) const;

            /// provide unique ID
            DeclPrimitrive2DIDBlock()
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif // INCLUDED_DRAWINGLAYER_PRIMITIVE2D_WALLPAPERPRIMITIVE2D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
