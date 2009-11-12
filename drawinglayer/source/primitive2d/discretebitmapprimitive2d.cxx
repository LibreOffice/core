/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: bitmapprimitive2d.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: aw $ $Date: 2008-05-27 14:11:20 $
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

#include <drawinglayer/primitive2d/discretebitmapprimitive2d.hxx>
#include <drawinglayer/primitive2d/bitmapprimitive2d.hxx>
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        Primitive2DSequence DiscreteBitmapPrimitive2D::create2DDecomposition(const geometry::ViewInformation2D& /*rViewInformation*/) const
        {
            // use getViewTransformation() and getObjectTransformation() from
            // ObjectAndViewTransformationDependentPrimitive2D to create a BitmapPrimitive2D
            // with the correct mapping
            Primitive2DSequence xRetval;

            if(!getBitmapEx().IsEmpty())
            {
                // get discrete size
                const Size& rSizePixel = getBitmapEx().GetSizePixel();
                const basegfx::B2DVector aDiscreteSize(rSizePixel.Width(), rSizePixel.Height());

                // get inverse ViewTransformation
                basegfx::B2DHomMatrix aInverseViewTransformation(getViewTransformation());
                aInverseViewTransformation.invert();

                // get size and position in world coordinates
                const basegfx::B2DVector aWorldSize(aInverseViewTransformation * aDiscreteSize);
                const basegfx::B2DPoint  aWorldTopLeft(getObjectTransformation() * getTopLeft());

                // build object matrix in world coordinates so that the top-left
                // position remains, but eventual transformations (e.g. rotations)
                // in the ObjectToView stack remain and get correctly applied
                basegfx::B2DHomMatrix aObjectTransform;

                aObjectTransform.set(0, 0, aWorldSize.getX());
                aObjectTransform.set(1, 1, aWorldSize.getY());
                aObjectTransform.set(0, 2, aWorldTopLeft.getX());
                aObjectTransform.set(1, 2, aWorldTopLeft.getY());

                // get inverse ObjectTransformation
                basegfx::B2DHomMatrix aInverseObjectTransformation(getObjectTransformation());
                aInverseObjectTransformation.invert();

                // transform to object coordinate system
                aObjectTransform = aInverseObjectTransformation * aObjectTransform;

                // create BitmapPrimitive2D with now object-local coordinate data
                const Primitive2DReference xRef(new BitmapPrimitive2D(getBitmapEx(), aObjectTransform));
                xRetval = Primitive2DSequence(&xRef, 1);
            }

            return xRetval;
        }

        DiscreteBitmapPrimitive2D::DiscreteBitmapPrimitive2D(
            const BitmapEx& rBitmapEx,
            const basegfx::B2DPoint& rTopLeft)
        :   ObjectAndViewTransformationDependentPrimitive2D(),
            maBitmapEx(rBitmapEx),
            maTopLeft(rTopLeft)
        {
        }

        bool DiscreteBitmapPrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            if(ObjectAndViewTransformationDependentPrimitive2D::operator==(rPrimitive))
            {
                const DiscreteBitmapPrimitive2D& rCompare = (DiscreteBitmapPrimitive2D&)rPrimitive;

                return (getBitmapEx() == rCompare.getBitmapEx()
                    && getTopLeft() == rCompare.getTopLeft());
            }

            return false;
        }

        // provide unique ID
        ImplPrimitrive2DIDBlock(DiscreteBitmapPrimitive2D, PRIMITIVE2D_ID_DISCRETEBITMAPPRIMITIVE2D)

    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
