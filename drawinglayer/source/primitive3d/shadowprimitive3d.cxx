/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: shadowprimitive3d.cxx,v $
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

#ifndef _DRAWINGLAYER_PRIMITIVE3D_SHADOWPRIMITIVE3D_HXX
#include <drawinglayer/primitive3d/shadowprimitive3d.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive
    {
        shadowPrimitive3D::shadowPrimitive3D(
            const ::basegfx::B2DHomMatrix& rShadowTransform, const ::basegfx::BColor& rShadowColor,
            double fShadowTransparence, bool bShadow3D, const primitiveList& rPrimitiveList)
        :   listPrimitive(rPrimitiveList),
            maShadowTransform(rShadowTransform),
            maShadowColor(rShadowColor),
            mfShadowTransparence(fShadowTransparence),
            mbShadow3D(bShadow3D)
        {
        }

        shadowPrimitive3D::~shadowPrimitive3D()
        {
        }

        bool shadowPrimitive3D::operator==(const basePrimitive& rPrimitive) const
        {
            if(listPrimitive::operator==(rPrimitive))
            {
                const shadowPrimitive3D& rCompare = (shadowPrimitive3D&)rPrimitive;
                return (maShadowTransform == rCompare.maShadowTransform
                    && maShadowColor == rCompare.maShadowColor
                    && mfShadowTransparence == rCompare.mfShadowTransparence
                    && mbShadow3D == rCompare.mbShadow3D);
            }

            return false;
        }

        basePrimitive* shadowPrimitive3D::createNewClone() const
        {
            return new shadowPrimitive3D(maShadowTransform, maShadowColor, mfShadowTransparence, mbShadow3D, maPrimitiveList);
        }

        PrimitiveID shadowPrimitive3D::getID() const
        {
            return CreatePrimitiveID('S', 'H', 'D', '3');
        }

        void shadowPrimitive3D::transform(const ::basegfx::B2DHomMatrix& rMatrix)
        {
            // call basePrimitive, not listPrimitive parent to not change the SubList
            basePrimitive::transform(rMatrix);

            // add tansform to local transform matrix. Applying to local sub-list is not necessary,
            // this is done at decomposition or left to the renderer to allow evtl. less copying
            maShadowTransform *= rMatrix;
        }
    } // end of namespace primitive
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
