/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: textureprimitive3d.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: aw $ $Date: 2006-05-12 11:49:09 $
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
    namespace primitive
    {
        texturePrimitive3D::texturePrimitive3D(
            const primitiveList& rPrimitiveList,
            const ::basegfx::B2DVector& rTextureSize,
            bool bModulate, bool bFilter)
        :   listPrimitive(rPrimitiveList),
            maTextureSize(rTextureSize),
            mbModulate(bModulate),
            mbFilter(bFilter)
        {
        }

        texturePrimitive3D::~texturePrimitive3D()
        {
        }

        bool texturePrimitive3D::operator==(const basePrimitive& rPrimitive) const
        {
            if(listPrimitive::operator==(rPrimitive))
            {
                const texturePrimitive3D& rCompare = (texturePrimitive3D&)rPrimitive;
                return (mbModulate == rCompare.mbModulate && mbFilter == rCompare.mbFilter);
            }

            return false;
        }
    } // end of namespace primitive
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive
    {
        void simpleTransparenceTexturePrimitive3D::decompose(primitiveList& rTarget, const ::drawinglayer::geometry::viewInformation& rViewInformation)
        {
            if(0.0 == mfTransparence)
            {
                // no transparence used, so just add the content
                rTarget.append(maPrimitiveList);
            }
            else if(mfTransparence > 0.0 && mfTransparence < 1.0)
            {
                // create transparenceTexturePrimitive3D with fixed transparence as replacement
                const ::basegfx::BColor aGray(mfTransparence, mfTransparence, mfTransparence);
                const fillGradientAttribute aFillGradient(GRADIENTSTYLE_LINEAR, 0.0, 0.0, 0.0, 0.0, aGray, aGray, 1);
                basePrimitive* pNewTransparenceTexturePrimitive3D = new transparenceTexturePrimitive3D(aFillGradient, maPrimitiveList, maTextureSize);
                rTarget.append(referencedPrimitive(*pNewTransparenceTexturePrimitive3D));
            }
            else
            {
                // completely transparent or invalid definition, add nothing
            }
        }

        simpleTransparenceTexturePrimitive3D::simpleTransparenceTexturePrimitive3D(
            double fTransparence,
            const primitiveList& rPrimitiveList)
        :   texturePrimitive3D(rPrimitiveList, ::basegfx::B2DVector(), false, false),
            mfTransparence(fTransparence)
        {
        }

        simpleTransparenceTexturePrimitive3D::~simpleTransparenceTexturePrimitive3D()
        {
        }

        bool simpleTransparenceTexturePrimitive3D::operator==(const basePrimitive& rPrimitive) const
        {
            if(texturePrimitive3D::operator==(rPrimitive))
            {
                const simpleTransparenceTexturePrimitive3D& rCompare = (simpleTransparenceTexturePrimitive3D&)rPrimitive;
                return (mfTransparence == rCompare.mfTransparence);
            }

            return false;
        }

        basePrimitive* simpleTransparenceTexturePrimitive3D::createNewClone() const
        {
            return new simpleTransparenceTexturePrimitive3D(mfTransparence, maPrimitiveList);
        }

        PrimitiveID simpleTransparenceTexturePrimitive3D::getID() const
        {
            return CreatePrimitiveID('S', 'T', 'X', '3');
        }
    } // end of namespace primitive
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive
    {
        void gradientTexturePrimitive3D::decompose(primitiveList& rTarget, const ::drawinglayer::geometry::viewInformation& rViewInformation)
        {
            rTarget.append(maPrimitiveList);
        }

        gradientTexturePrimitive3D::gradientTexturePrimitive3D(
            const fillGradientAttribute& rGradient,
            const primitiveList& rPrimitiveList,
            const ::basegfx::B2DVector& rTextureSize,
            bool bModulate,
            bool bFilter)
        :   texturePrimitive3D(rPrimitiveList, rTextureSize, bModulate, bFilter),
            maGradient(rGradient)
        {
        }

        gradientTexturePrimitive3D::~gradientTexturePrimitive3D()
        {
        }

        bool gradientTexturePrimitive3D::operator==(const basePrimitive& rPrimitive) const
        {
            if(texturePrimitive3D::operator==(rPrimitive))
            {
                const gradientTexturePrimitive3D& rCompare = (gradientTexturePrimitive3D&)rPrimitive;
                return (maGradient == rCompare.maGradient);
            }

            return false;
        }

        basePrimitive* gradientTexturePrimitive3D::createNewClone() const
        {
            return new gradientTexturePrimitive3D(maGradient, maPrimitiveList, maTextureSize, mbModulate, mbFilter);
        }

        PrimitiveID gradientTexturePrimitive3D::getID() const
        {
            return CreatePrimitiveID('G', 'R', 'X', '3');
        }
    } // end of namespace primitive
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive
    {
        void bitmapTexturePrimitive3D::decompose(primitiveList& rTarget, const ::drawinglayer::geometry::viewInformation& rViewInformation)
        {
            rTarget.append(maPrimitiveList);
        }

        bitmapTexturePrimitive3D::bitmapTexturePrimitive3D(
            const fillBitmapAttribute& rBitmap,
            const primitiveList& rPrimitiveList,
            const ::basegfx::B2DVector& rTextureSize,
            bool bModulate, bool bFilter)
        :   texturePrimitive3D(rPrimitiveList, rTextureSize, bModulate, bFilter),
            maBitmap(rBitmap)
        {
        }

        bitmapTexturePrimitive3D::~bitmapTexturePrimitive3D()
        {
        }

        bool bitmapTexturePrimitive3D::operator==(const basePrimitive& rPrimitive) const
        {
            if(texturePrimitive3D::operator==(rPrimitive))
            {
                const bitmapTexturePrimitive3D& rCompare = (bitmapTexturePrimitive3D&)rPrimitive;
                return (maBitmap == rCompare.maBitmap);
            }

            return false;
        }

        basePrimitive* bitmapTexturePrimitive3D::createNewClone() const
        {
            return new bitmapTexturePrimitive3D(maBitmap, maPrimitiveList, maTextureSize, mbModulate, mbFilter);
        }

        PrimitiveID bitmapTexturePrimitive3D::getID() const
        {
            return CreatePrimitiveID('B', 'M', 'X', '3');
        }
    } // end of namespace primitive
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive
    {
        transparenceTexturePrimitive3D::transparenceTexturePrimitive3D(
            const fillGradientAttribute& rGradient,
            const primitiveList& rPrimitiveList,
            const ::basegfx::B2DVector& rTextureSize)
        :   gradientTexturePrimitive3D(rGradient, rPrimitiveList, rTextureSize, false, false)
        {
        }

        transparenceTexturePrimitive3D::~transparenceTexturePrimitive3D()
        {
        }

        bool transparenceTexturePrimitive3D::operator==(const basePrimitive& rPrimitive) const
        {
            return (gradientTexturePrimitive3D::operator==(rPrimitive));
        }

        basePrimitive* transparenceTexturePrimitive3D::createNewClone() const
        {
            return new transparenceTexturePrimitive3D(maGradient, maPrimitiveList, maTextureSize);
        }

        PrimitiveID transparenceTexturePrimitive3D::getID() const
        {
            return CreatePrimitiveID('T', 'R', 'X', '3');
        }
    } // end of namespace primitive
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
