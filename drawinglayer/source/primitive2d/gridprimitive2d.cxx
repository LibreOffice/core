/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: gridprimitive2d.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: aw $ $Date: 2008-03-05 09:15:42 $
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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_GRIDPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/gridprimitive2d.hxx>
#endif

#ifndef _BGFX_TOOLS_CANVASTOOLS_HXX
#include <basegfx/tools/canvastools.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_POINTRARRAYPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/pointarrayprimitive2d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_MARKERARRAYPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/markerarrayprimitive2d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_GEOMETRY_VIEWINFORMATION2D_HXX
#include <drawinglayer/geometry/viewinformation2d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_PRIMITIVETYPES2D_HXX
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////

using namespace com::sun::star;

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        Primitive2DSequence GridPrimitive2D::createLocalDecomposition(const geometry::ViewInformation2D& rViewInformation) const
        {
            Primitive2DSequence aRetval;

            if(!rViewInformation.getViewport().isEmpty() && getWidth() > 0.0 && getHeight() > 0.0)
            {
                // decompose grid matrix to get logic size
                basegfx::B2DVector aScale, aTranslate;
                double fRotate, fShearX;
                getTransform().decompose(aScale, aTranslate, fRotate, fShearX);

                // create grid matrix which transforms from scaled logic to view
                basegfx::B2DHomMatrix aRST;
                aRST.shearX(fShearX);
                aRST.rotate(fRotate);
                aRST.translate(aTranslate.getX(), aTranslate.getY());
                aRST *= rViewInformation.getViewTransformation();

                // get step widths
                double fStepX(getWidth());
                double fStepY(getHeight());
                const double fMinimalStep(10.0);

                // guarantee a step width of 100.0
                if(basegfx::fTools::less(fStepX, fMinimalStep))
                {
                    fStepX = fMinimalStep;
                }

                if(basegfx::fTools::less(fStepY, fMinimalStep))
                {
                    fStepY = fMinimalStep;
                }

                // get relative distances in view
                double fViewStepX((rViewInformation.getViewTransformation() * basegfx::B2DVector(fStepX, 0.0)).getLength());
                double fViewStepY((rViewInformation.getViewTransformation() * basegfx::B2DVector(0.0, fStepY)).getLength());
                double fSmallStepX(1.0), fViewSmallStepX(1.0), fSmallStepY(1.0), fViewSmallStepY(1.0);
                sal_uInt32 nSmallStepsX(0L), nSmallStepsY(0L);

                // setup subdivisions
                if(getSubdivisionsX())
                {
                    fSmallStepX = fStepX / getSubdivisionsX();
                    fViewSmallStepX = fViewStepX / getSubdivisionsX();
                }

                if(getSubdivisionsY())
                {
                    fSmallStepY = fStepY / getSubdivisionsY();
                    fViewSmallStepY = fViewStepY / getSubdivisionsY();
                }

                // correct step width
                while(fViewStepX < getSmallestViewDistance())
                {
                    fViewStepX *= 2.0;
                    fStepX *= 2.0;
                }

                while(fViewStepY < getSmallestViewDistance())
                {
                    fViewStepY *= 2.0;
                    fStepY *= 2.0;
                }

                // correct small step width
                if(getSubdivisionsX())
                {
                    while(fViewSmallStepX < getSmallestSubdivisionViewDistance())
                    {
                        fViewSmallStepX *= 2.0;
                        fSmallStepX *= 2.0;
                    }

                    nSmallStepsX = (sal_uInt32)(fStepX / fSmallStepX);
                }

                if(getSubdivisionsY())
                {
                    while(fViewSmallStepY < getSmallestSubdivisionViewDistance())
                    {
                        fViewSmallStepY *= 2.0;
                        fSmallStepY *= 2.0;
                    }

                    nSmallStepsY = (sal_uInt32)(fStepY / fSmallStepY);
                }

                // prepare point vectors for point and cross markers
                std::vector< basegfx::B2DPoint > aPositionsPoint;
                std::vector< basegfx::B2DPoint > aPositionsCross;

                for(double fX(0.0); fX < aScale.getX(); fX += fStepX)
                {
                    const bool bXZero(basegfx::fTools::equalZero(fX));

                    for(double fY(0.0); fY < aScale.getY(); fY += fStepY)
                    {
                        const bool bYZero(basegfx::fTools::equalZero(fY));
                        basegfx::B2DPoint aViewPos(aRST * basegfx::B2DPoint(fX, fY));

                        if(rViewInformation.getDiscreteViewport().isInside(aViewPos) && !bXZero && !bYZero)
                        {
                            const basegfx::B2DPoint aLogicPos(rViewInformation.getInverseViewTransformation() * aViewPos);
                            aPositionsCross.push_back(aLogicPos);
                        }

                        if(getSubdivisionsX() && !bYZero)
                        {
                            double fF(fX + fSmallStepX);

                            for(sal_uInt32 a(0L); a < nSmallStepsX && fF < aScale.getX(); a++, fF += fSmallStepX)
                            {
                                aViewPos = aRST * basegfx::B2DPoint(fF, fY);

                                if(rViewInformation.getDiscreteViewport().isInside(aViewPos))
                                {
                                    const basegfx::B2DPoint aLogicPos(rViewInformation.getInverseViewTransformation() * aViewPos);
                                    aPositionsPoint.push_back(aLogicPos);
                                }
                            }
                        }

                        if(getSubdivisionsY() && !bXZero)
                        {
                            double fF(fY + fSmallStepY);

                            for(sal_uInt32 a(0L); a < nSmallStepsY && fF < aScale.getY(); a++, fF += fSmallStepY)
                            {
                                aViewPos = aRST * basegfx::B2DPoint(fX, fF);

                                if(rViewInformation.getDiscreteViewport().isInside(aViewPos))
                                {
                                    const basegfx::B2DPoint aLogicPos(rViewInformation.getInverseViewTransformation() * aViewPos);
                                    aPositionsPoint.push_back(aLogicPos);
                                }
                            }
                        }
                    }
                }

                // prepare return value
                const sal_uInt32 nCountPoint(aPositionsPoint.size());
                const sal_uInt32 nCountCross(aPositionsCross.size());
                const sal_uInt32 nRetvalCount((nCountPoint ? 1 : 0) + (nCountCross ? 1 : 0));
                sal_uInt32 nInsertCounter(0);

                aRetval.realloc(nRetvalCount);

                // add PointArrayPrimitive2D if point markers were added
                if(nCountPoint)
                {
                    aRetval[nInsertCounter++] = Primitive2DReference(new PointArrayPrimitive2D(aPositionsPoint, getBColor()));
                }

                // add MarkerArrayPrimitive2D if cross markers were added
                if(nCountCross)
                {
                    aRetval[nInsertCounter++] = Primitive2DReference(new MarkerArrayPrimitive2D(aPositionsCross, MARKERSTYLE2D_CROSS, getBColor()));
                }
            }

            return aRetval;
        }

        GridPrimitive2D::GridPrimitive2D(
            const basegfx::B2DHomMatrix& rTransform,
            double fWidth,
            double fHeight,
            double fSmallestViewDistance,
            double fSmallestSubdivisionViewDistance,
            sal_uInt32 nSubdivisionsX,
            sal_uInt32 nSubdivisionsY,
            const basegfx::BColor& rBColor)
        :   BasePrimitive2D(),
            maTransform(rTransform),
            mfWidth(fWidth),
            mfHeight(fHeight),
            mfSmallestViewDistance(fSmallestViewDistance),
            mfSmallestSubdivisionViewDistance(fSmallestSubdivisionViewDistance),
            mnSubdivisionsX(nSubdivisionsX),
            mnSubdivisionsY(nSubdivisionsY),
            maBColor(rBColor),
            maLastViewTransformation(),
            maLastViewport()
        {
        }

        bool GridPrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            if(BasePrimitive2D::operator==(rPrimitive))
            {
                const GridPrimitive2D& rCompare = (GridPrimitive2D&)rPrimitive;

                return (getTransform() == rCompare.getTransform()
                    && getWidth() == rCompare.getWidth()
                    && getHeight() == rCompare.getHeight()
                    && getSmallestViewDistance() == rCompare.getSmallestViewDistance()
                    && getSmallestSubdivisionViewDistance() == rCompare.getSmallestSubdivisionViewDistance()
                    && getSubdivisionsX() == rCompare.getSubdivisionsX()
                    && getSubdivisionsY() == rCompare.getSubdivisionsY()
                    && getBColor() == rCompare.getBColor());
            }

            return false;
        }

        basegfx::B2DRange GridPrimitive2D::getB2DRange(const geometry::ViewInformation2D& rViewInformation) const
        {
            // get object's range
            basegfx::B2DRange aUnitRange(0.0, 0.0, 1.0, 1.0);
            aUnitRange.transform(getTransform());

            // intersect with visible part
            aUnitRange.intersect(rViewInformation.getViewport());

            return aUnitRange;
        }

        Primitive2DSequence GridPrimitive2D::get2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const
        {
            ::osl::MutexGuard aGuard( m_aMutex );

            if(getLocalDecomposition().hasElements())
            {
                if(maLastViewport != rViewInformation.getViewport() || maLastViewTransformation != rViewInformation.getViewTransformation())
                {
                    // conditions of last local decomposition have changed, delete
                    const_cast< GridPrimitive2D* >(this)->setLocalDecomposition(Primitive2DSequence());
                }
            }

            if(!getLocalDecomposition().hasElements())
            {
                // remember ViewRange and ViewTransformation
                const_cast< GridPrimitive2D* >(this)->maLastViewTransformation = rViewInformation.getViewTransformation();
                const_cast< GridPrimitive2D* >(this)->maLastViewport = rViewInformation.getViewport();
            }

            // use parent implementation
            return BasePrimitive2D::get2DDecomposition(rViewInformation);
        }

        // provide unique ID
        ImplPrimitrive2DIDBlock(GridPrimitive2D, PRIMITIVE2D_ID_GRIDPRIMITIVE2D)

    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
