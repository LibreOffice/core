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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_drawinglayer.hxx"

#include <drawinglayer/primitive3d/textureprimitive3d.hxx>
#include <drawinglayer/primitive3d/drawinglayer_primitivetypes3d.hxx>

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
        UnifiedAlphaTexturePrimitive3D::UnifiedAlphaTexturePrimitive3D(
            double fTransparence,
            const Primitive3DSequence& rChildren)
        :   TexturePrimitive3D(rChildren, basegfx::B2DVector(), false, false),
            mfTransparence(fTransparence)
        {
        }

        bool UnifiedAlphaTexturePrimitive3D::operator==(const BasePrimitive3D& rPrimitive) const
        {
            if(TexturePrimitive3D::operator==(rPrimitive))
            {
                const UnifiedAlphaTexturePrimitive3D& rCompare = (UnifiedAlphaTexturePrimitive3D&)rPrimitive;

                return (getTransparence() == rCompare.getTransparence());
            }

            return false;
        }

        Primitive3DSequence UnifiedAlphaTexturePrimitive3D::get3DDecomposition(const geometry::ViewInformation3D& /*rViewInformation*/) const
        {
            if(0.0 == getTransparence())
            {
                // no transparence used, so just use content
                return getChildren();
            }
            else if(getTransparence() > 0.0 && getTransparence() < 1.0)
            {
                // create AlphaTexturePrimitive3D with fixed transparence as replacement
                const basegfx::BColor aGray(getTransparence(), getTransparence(), getTransparence());
                const attribute::FillGradientAttribute aFillGradient(attribute::GRADIENTSTYLE_LINEAR, 0.0, 0.0, 0.0, 0.0, aGray, aGray, 1);
                const Primitive3DReference xRef(new AlphaTexturePrimitive3D(aFillGradient, getChildren(), getTextureSize()));
                return Primitive3DSequence(&xRef, 1L);
            }
            else
            {
                // completely transparent or invalid definition, add nothing
                return Primitive3DSequence();
            }
        }

        // provide unique ID
        ImplPrimitrive3DIDBlock(UnifiedAlphaTexturePrimitive3D, PRIMITIVE3D_ID_UNIFIEDALPHATEXTUREPRIMITIVE3D)

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
        AlphaTexturePrimitive3D::AlphaTexturePrimitive3D(
            const attribute::FillGradientAttribute& rGradient,
            const Primitive3DSequence& rChildren,
            const basegfx::B2DVector& rTextureSize)
        :   GradientTexturePrimitive3D(rGradient, rChildren, rTextureSize, false, false)
        {
        }

        bool AlphaTexturePrimitive3D::operator==(const BasePrimitive3D& rPrimitive) const
        {
            return (GradientTexturePrimitive3D::operator==(rPrimitive));
        }

        // provide unique ID
        ImplPrimitrive3DIDBlock(AlphaTexturePrimitive3D, PRIMITIVE3D_ID_ALPHATEXTUREPRIMITIVE3D)

    } // end of namespace primitive3d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
