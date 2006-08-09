/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: textureprimitive3d.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: aw $ $Date: 2006-08-09 16:51:16 $
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

#ifndef _DRAWINGLAYER_PRIMITIVE3D_TEXTUREPRIMITIVE3D_HXX
#include <drawinglayer/primitive3d/textureprimitive3d.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive3d
    {
        texturePrimitive3D::texturePrimitive3D(
            const primitiveVector3D& rPrimitiveVector,
            const basegfx::B2DVector& rTextureSize,
            bool bModulate, bool bFilter)
        :   vectorPrimitive3D(rPrimitiveVector),
            maTextureSize(rTextureSize),
            mbModulate(bModulate),
            mbFilter(bFilter)
        {
        }

        texturePrimitive3D::~texturePrimitive3D()
        {
        }

        bool texturePrimitive3D::operator==(const basePrimitive3D& rPrimitive) const
        {
            if(vectorPrimitive3D::operator==(rPrimitive))
            {
                const texturePrimitive3D& rCompare = (texturePrimitive3D&)rPrimitive;
                return (mbModulate == rCompare.mbModulate && mbFilter == rCompare.mbFilter);
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
        void simpleTransparenceTexturePrimitive3D::decompose(primitiveVector3D& rTarget)
        {
            if(0.0 == mfTransparence)
            {
                // no transparence used, so just add the content
                rTarget.insert(rTarget.end(), maPrimitiveVector.begin(), maPrimitiveVector.end());
            }
            else if(mfTransparence > 0.0 && mfTransparence < 1.0)
            {
                // create transparenceTexturePrimitive3D with fixed transparence as replacement
                const basegfx::BColor aGray(mfTransparence, mfTransparence, mfTransparence);
                const attribute::fillGradientAttribute aFillGradient(attribute::GRADIENTSTYLE_LINEAR, 0.0, 0.0, 0.0, 0.0, aGray, aGray, 1);
                basePrimitive3D* pNewTransparenceTexturePrimitive3D = new transparenceTexturePrimitive3D(aFillGradient, maPrimitiveVector, maTextureSize);
                rTarget.push_back(referencedPrimitive3D(*pNewTransparenceTexturePrimitive3D));
            }
            else
            {
                // completely transparent or invalid definition, add nothing
            }
        }

        simpleTransparenceTexturePrimitive3D::simpleTransparenceTexturePrimitive3D(
            double fTransparence,
            const primitiveVector3D& rPrimitiveVector)
        :   texturePrimitive3D(rPrimitiveVector, basegfx::B2DVector(), false, false),
            mfTransparence(fTransparence)
        {
        }

        simpleTransparenceTexturePrimitive3D::~simpleTransparenceTexturePrimitive3D()
        {
        }

        bool simpleTransparenceTexturePrimitive3D::operator==(const basePrimitive3D& rPrimitive) const
        {
            if(texturePrimitive3D::operator==(rPrimitive))
            {
                const simpleTransparenceTexturePrimitive3D& rCompare = (simpleTransparenceTexturePrimitive3D&)rPrimitive;
                return (mfTransparence == rCompare.mfTransparence);
            }

            return false;
        }

        PrimitiveID simpleTransparenceTexturePrimitive3D::getID() const
        {
            return CreatePrimitiveID('S', 'T', 'X', '3');
        }
    } // end of namespace primitive3d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive3d
    {
        void gradientTexturePrimitive3D::decompose(primitiveVector3D& rTarget)
        {
            rTarget.insert(rTarget.end(), maPrimitiveVector.begin(), maPrimitiveVector.end());
        }

        gradientTexturePrimitive3D::gradientTexturePrimitive3D(
            const attribute::fillGradientAttribute& rGradient,
            const primitiveVector3D& rPrimitiveVector,
            const basegfx::B2DVector& rTextureSize,
            bool bModulate,
            bool bFilter)
        :   texturePrimitive3D(rPrimitiveVector, rTextureSize, bModulate, bFilter),
            maGradient(rGradient)
        {
        }

        gradientTexturePrimitive3D::~gradientTexturePrimitive3D()
        {
        }

        bool gradientTexturePrimitive3D::operator==(const basePrimitive3D& rPrimitive) const
        {
            if(texturePrimitive3D::operator==(rPrimitive))
            {
                const gradientTexturePrimitive3D& rCompare = (gradientTexturePrimitive3D&)rPrimitive;
                return (maGradient == rCompare.maGradient);
            }

            return false;
        }

        PrimitiveID gradientTexturePrimitive3D::getID() const
        {
            return CreatePrimitiveID('G', 'R', 'X', '3');
        }
    } // end of namespace primitive3d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive3d
    {
        void bitmapTexturePrimitive3D::decompose(primitiveVector3D& rTarget)
        {
            rTarget.insert(rTarget.end(), maPrimitiveVector.begin(), maPrimitiveVector.end());
        }

        bitmapTexturePrimitive3D::bitmapTexturePrimitive3D(
            const attribute::fillBitmapAttribute& rBitmap,
            const primitiveVector3D& rPrimitiveVector,
            const basegfx::B2DVector& rTextureSize,
            bool bModulate, bool bFilter)
        :   texturePrimitive3D(rPrimitiveVector, rTextureSize, bModulate, bFilter),
            maBitmap(rBitmap)
        {
        }

        bitmapTexturePrimitive3D::~bitmapTexturePrimitive3D()
        {
        }

        bool bitmapTexturePrimitive3D::operator==(const basePrimitive3D& rPrimitive) const
        {
            if(texturePrimitive3D::operator==(rPrimitive))
            {
                const bitmapTexturePrimitive3D& rCompare = (bitmapTexturePrimitive3D&)rPrimitive;
                return (maBitmap == rCompare.maBitmap);
            }

            return false;
        }

        PrimitiveID bitmapTexturePrimitive3D::getID() const
        {
            return CreatePrimitiveID('B', 'M', 'X', '3');
        }
    } // end of namespace primitive3d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive3d
    {
        transparenceTexturePrimitive3D::transparenceTexturePrimitive3D(
            const attribute::fillGradientAttribute& rGradient,
            const primitiveVector3D& rPrimitiveVector,
            const basegfx::B2DVector& rTextureSize)
        :   gradientTexturePrimitive3D(rGradient, rPrimitiveVector, rTextureSize, false, false)
        {
        }

        transparenceTexturePrimitive3D::~transparenceTexturePrimitive3D()
        {
        }

        bool transparenceTexturePrimitive3D::operator==(const basePrimitive3D& rPrimitive) const
        {
            return (gradientTexturePrimitive3D::operator==(rPrimitive));
        }

        PrimitiveID transparenceTexturePrimitive3D::getID() const
        {
            return CreatePrimitiveID('T', 'R', 'X', '3');
        }
    } // end of namespace primitive3d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
