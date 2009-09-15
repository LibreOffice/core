/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: markerarrayprimitive2d.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: aw $ $Date: 2008-07-21 17:41:18 $
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

#include <drawinglayer/primitive2d/markerarrayprimitive2d.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <drawinglayer/geometry/viewinformation2d.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <drawinglayer/primitive2d/polygonprimitive2d.hxx>
#include <drawinglayer/primitive2d/transformprimitive2d.hxx>
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>
#include <drawinglayer/primitive2d/bitmapprimitive2d.hxx>

//////////////////////////////////////////////////////////////////////////////

using namespace com::sun::star;

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        Primitive2DSequence MarkerArrayPrimitive2D::createLocal2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const
        {
            Primitive2DSequence xRetval;
            const std::vector< basegfx::B2DPoint >& rPositions = getPositions();
            const sal_uInt32 nMarkerCount(rPositions.size());

            if(nMarkerCount && !getMarker().IsEmpty())
            {
                // get pixel size
                Size aBitmapSize(getMarker().GetSizePixel());

                if(aBitmapSize.Width() && aBitmapSize.Height())
                {
                    // get logic half pixel size
                    basegfx::B2DVector aLogicHalfSize(rViewInformation.getInverseObjectToViewTransformation() *
                        basegfx::B2DVector(aBitmapSize.getWidth() - 1.0, aBitmapSize.getHeight() - 1.0));

                    // use half size for expand
                    aLogicHalfSize *= 0.5;

                    // number of primitives is known; realloc accordingly
                    xRetval.realloc(nMarkerCount);

                    for(sal_uInt32 a(0); a < nMarkerCount; a++)
                    {
                        const basegfx::B2DPoint& rPosition(rPositions[a]);
                        const basegfx::B2DRange aRange(rPosition - aLogicHalfSize, rPosition + aLogicHalfSize);
                        basegfx::B2DHomMatrix aTransform;

                        aTransform.set(0, 0, aRange.getWidth());
                        aTransform.set(1, 1, aRange.getHeight());
                        aTransform.set(0, 2, aRange.getMinX());
                        aTransform.set(1, 2, aRange.getMinY());

                        xRetval[a] = Primitive2DReference(new BitmapPrimitive2D(getMarker(), aTransform));
                    }
                }
            }

            return xRetval;
        }

        MarkerArrayPrimitive2D::MarkerArrayPrimitive2D(
            const std::vector< basegfx::B2DPoint >& rPositions,
            const BitmapEx& rMarker)
        :   BufferedDecompositionPrimitive2D(),
            maPositions(rPositions),
            maMarker(rMarker)
        {
        }

        bool MarkerArrayPrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            if(BufferedDecompositionPrimitive2D::operator==(rPrimitive))
            {
                const MarkerArrayPrimitive2D& rCompare = (MarkerArrayPrimitive2D&)rPrimitive;

                return (getPositions() == rCompare.getPositions()
                    && getMarker() == rCompare.getMarker());
            }

            return false;
        }

        basegfx::B2DRange MarkerArrayPrimitive2D::getB2DRange(const geometry::ViewInformation2D& rViewInformation) const
        {
            basegfx::B2DRange aRetval;

            if(getPositions().size())
            {
                // get the basic range from the position vector
                for(std::vector< basegfx::B2DPoint >::const_iterator aIter(getPositions().begin()); aIter != getPositions().end(); aIter++)
                {
                    aRetval.expand(*aIter);
                }

                if(!getMarker().IsEmpty())
                {
                    // get pixel size
                    const Size aBitmapSize(getMarker().GetSizePixel());

                    if(aBitmapSize.Width() && aBitmapSize.Height())
                    {
                        // get logic half size
                        basegfx::B2DVector aLogicHalfSize(rViewInformation.getInverseObjectToViewTransformation() *
                            basegfx::B2DVector(aBitmapSize.getWidth(), aBitmapSize.getHeight()));

                        // use half size for expand
                        aLogicHalfSize *= 0.5;

                        // apply aLogicHalfSize
                        aRetval.expand(aRetval.getMinimum() - aLogicHalfSize);
                        aRetval.expand(aRetval.getMaximum() + aLogicHalfSize);
                    }
                }
            }

            return aRetval;
        }

        // provide unique ID
        ImplPrimitrive2DIDBlock(MarkerArrayPrimitive2D, PRIMITIVE2D_ID_MARKERARRAYPRIMITIVE2D)

    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
