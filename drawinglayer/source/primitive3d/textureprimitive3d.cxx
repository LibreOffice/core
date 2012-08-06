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

#include <drawinglayer/primitive3d/textureprimitive3d.hxx>
#include <drawinglayer/primitive3d/drawinglayer_primitivetypes3d.hxx>
#include <basegfx/color/bcolor.hxx>

//////////////////////////////////////////////////////////////////////////////

using namespace com::sun::star;

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive3d
    {
        TexturePrimitive3D::TexturePrimitive3D(
            const Primitive3DSequence& rChildren,
            const basegfx::B2DVector& rTextureSize,
            bool bModulate, bool bFilter)
        :   GroupPrimitive3D(rChildren),
            maTextureSize(rTextureSize),
            mbModulate(bModulate),
            mbFilter(bFilter)
        {
        }

        bool TexturePrimitive3D::operator==(const BasePrimitive3D& rPrimitive) const
        {
            if(GroupPrimitive3D::operator==(rPrimitive))
            {
                const TexturePrimitive3D& rCompare = (TexturePrimitive3D&)rPrimitive;

                return (getModulate() == rCompare.getModulate()
                    && getFilter() == rCompare.getFilter());
            }

            return false;
        }
    } // end of namespace primitive3d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive3d
    {
        UnifiedTransparenceTexturePrimitive3D::UnifiedTransparenceTexturePrimitive3D(
            double fTransparence,
            const Primitive3DSequence& rChildren)
        :   TexturePrimitive3D(rChildren, basegfx::B2DVector(), false, false),
            mfTransparence(fTransparence)
        {
        }

        bool UnifiedTransparenceTexturePrimitive3D::operator==(const BasePrimitive3D& rPrimitive) const
        {
            if(TexturePrimitive3D::operator==(rPrimitive))
            {
                const UnifiedTransparenceTexturePrimitive3D& rCompare = (UnifiedTransparenceTexturePrimitive3D&)rPrimitive;

                return (getTransparence() == rCompare.getTransparence());
            }

            return false;
        }

        basegfx::B3DRange UnifiedTransparenceTexturePrimitive3D::getB3DRange(const geometry::ViewInformation3D& rViewInformation) const
        {
            // do not use the fallback to decomposition here since for a correct BoundRect we also
            // need invisible (1.0 == getTransparence()) geometry; these would be deleted in the decomposition
            return getB3DRangeFromPrimitive3DSequence(getChildren(), rViewInformation);
        }

        Primitive3DSequence UnifiedTransparenceTexturePrimitive3D::get3DDecomposition(const geometry::ViewInformation3D& /*rViewInformation*/) const
        {
            if(0.0 == getTransparence())
            {
                // no transparence used, so just use content
                return getChildren();
            }
            else if(getTransparence() > 0.0 && getTransparence() < 1.0)
            {
                // create TransparenceTexturePrimitive3D with fixed transparence as replacement
                const basegfx::BColor aGray(getTransparence(), getTransparence(), getTransparence());
                const attribute::FillGradientAttribute aFillGradient(attribute::GRADIENTSTYLE_LINEAR, 0.0, 0.0, 0.0, 0.0, aGray, aGray, 1);
                const Primitive3DReference xRef(new TransparenceTexturePrimitive3D(aFillGradient, getChildren(), getTextureSize()));
                return Primitive3DSequence(&xRef, 1L);
            }
            else
            {
                // completely transparent or invalid definition, add nothing
                return Primitive3DSequence();
            }
        }

        // provide unique ID
        ImplPrimitrive3DIDBlock(UnifiedTransparenceTexturePrimitive3D, PRIMITIVE3D_ID_UNIFIEDTRANSPARENCETEXTUREPRIMITIVE3D)

    } // end of namespace primitive3d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive3d
    {
        GradientTexturePrimitive3D::GradientTexturePrimitive3D(
            const attribute::FillGradientAttribute& rGradient,
            const Primitive3DSequence& rChildren,
            const basegfx::B2DVector& rTextureSize,
            bool bModulate,
            bool bFilter)
        :   TexturePrimitive3D(rChildren, rTextureSize, bModulate, bFilter),
            maGradient(rGradient)
        {
        }

        bool GradientTexturePrimitive3D::operator==(const BasePrimitive3D& rPrimitive) const
        {
            if(TexturePrimitive3D::operator==(rPrimitive))
            {
                const GradientTexturePrimitive3D& rCompare = (GradientTexturePrimitive3D&)rPrimitive;

                return (getGradient() == rCompare.getGradient());
            }

            return false;
        }

        // provide unique ID
        ImplPrimitrive3DIDBlock(GradientTexturePrimitive3D, PRIMITIVE3D_ID_GRADIENTTEXTUREPRIMITIVE3D)

    } // end of namespace primitive3d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive3d
    {
        BitmapTexturePrimitive3D::BitmapTexturePrimitive3D(
            const attribute::FillBitmapAttribute& rFillBitmapAttribute,
            const Primitive3DSequence& rChildren,
            const basegfx::B2DVector& rTextureSize,
            bool bModulate, bool bFilter)
        :   TexturePrimitive3D(rChildren, rTextureSize, bModulate, bFilter),
            maFillBitmapAttribute(rFillBitmapAttribute)
        {
        }

        bool BitmapTexturePrimitive3D::operator==(const BasePrimitive3D& rPrimitive) const
        {
            if(TexturePrimitive3D::operator==(rPrimitive))
            {
                const BitmapTexturePrimitive3D& rCompare = (BitmapTexturePrimitive3D&)rPrimitive;

                return (getFillBitmapAttribute() == rCompare.getFillBitmapAttribute());
            }

            return false;
        }

        // provide unique ID
        ImplPrimitrive3DIDBlock(BitmapTexturePrimitive3D, PRIMITIVE3D_ID_BITMAPTEXTUREPRIMITIVE3D)

    } // end of namespace primitive3d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive3d
    {
        TransparenceTexturePrimitive3D::TransparenceTexturePrimitive3D(
            const attribute::FillGradientAttribute& rGradient,
            const Primitive3DSequence& rChildren,
            const basegfx::B2DVector& rTextureSize)
        :   GradientTexturePrimitive3D(rGradient, rChildren, rTextureSize, false, false)
        {
        }

        bool TransparenceTexturePrimitive3D::operator==(const BasePrimitive3D& rPrimitive) const
        {
            return (GradientTexturePrimitive3D::operator==(rPrimitive));
        }

        // provide unique ID
        ImplPrimitrive3DIDBlock(TransparenceTexturePrimitive3D, PRIMITIVE3D_ID_TRANSPARENCETEXTUREPRIMITIVE3D)

    } // end of namespace primitive3d
} // end of namespace drawinglayer

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
