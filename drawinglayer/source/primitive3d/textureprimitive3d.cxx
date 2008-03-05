/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: textureprimitive3d.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: aw $ $Date: 2008-03-05 09:15:44 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_drawinglayer.hxx"

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE3D_TEXTUREPRIMITIVE3D_HXX
#include <drawinglayer/primitive3d/textureprimitive3d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE3D_PRIMITIVETYPES3D_HXX
#include <drawinglayer/primitive3d/drawinglayer_primitivetypes3d.hxx>
#endif

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
        Primitive3DSequence UnifiedAlphaTexturePrimitive3D::createLocalDecomposition(double /*fTime*/) const
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

        // provide unique ID
        ImplPrimitrive3DIDBlock(UnifiedAlphaTexturePrimitive3D, PRIMITIVE3D_ID_UNIFIEDALPHATEXTUREPRIMITIVE3D)

    } // end of namespace primitive3d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive3d
    {
        Primitive3DSequence GradientTexturePrimitive3D::createLocalDecomposition(double /*fTime*/) const
        {
            return getChildren();
        }

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
        Primitive3DSequence BitmapTexturePrimitive3D::createLocalDecomposition(double /*fTime*/) const
        {
            return getChildren();
        }

        BitmapTexturePrimitive3D::BitmapTexturePrimitive3D(
            const attribute::FillBitmapAttribute& rBitmap,
            const Primitive3DSequence& rChildren,
            const basegfx::B2DVector& rTextureSize,
            bool bModulate, bool bFilter)
        :   TexturePrimitive3D(rChildren, rTextureSize, bModulate, bFilter),
            maBitmap(rBitmap)
        {
        }

        bool BitmapTexturePrimitive3D::operator==(const BasePrimitive3D& rPrimitive) const
        {
            if(TexturePrimitive3D::operator==(rPrimitive))
            {
                const BitmapTexturePrimitive3D& rCompare = (BitmapTexturePrimitive3D&)rPrimitive;

                return (getBitmap() == rCompare.getBitmap());
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
