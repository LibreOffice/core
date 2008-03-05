/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: helplineprimitive2d.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: aw $ $Date: 2008-03-05 09:15:43 $
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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_HELPLINEPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/helplineprimitive2d.hxx>
#endif

#ifndef _BGFX_POLYGON_B2DPOLYGON_HXX
#include <basegfx/polygon/b2dpolygon.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_POLYGONPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/polygonprimitive2d.hxx>
#endif

#ifndef _BGFX_POLYPOLYGON_B2DPOLYGONCLIPPER_HXX
#include <basegfx/polygon/b2dpolygonclipper.hxx>
#endif

#ifndef _BGFX_TOOLS_CANVASTOOLS_HXX
#include <basegfx/tools/canvastools.hxx>
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
        Primitive2DSequence HelplinePrimitive2D::createLocalDecomposition(const geometry::ViewInformation2D& rViewInformation) const
        {
            std::vector< BasePrimitive2D* > aTempPrimitiveTarget;

            if(!rViewInformation.getViewport().isEmpty() && !getDirection().equalZero())
            {
                // position to view coordinates, DashLen and DashLen in logic
                const basegfx::B2DPoint aViewPosition(rViewInformation.getViewTransformation() * getPosition());
                const double fLogicDashLen((rViewInformation.getInverseViewTransformation() * basegfx::B2DVector(getViewDashLength(), 0.0)).getLength());

                switch(getStyle())
                {
                    default : // HELPLINESTYLE2D_POINT
                    {
                        const double fViewFixValue(15.0);
                        basegfx::B2DVector aNormalizedDirection(getDirection());
                        aNormalizedDirection.normalize();
                        aNormalizedDirection *= fViewFixValue;
                        const basegfx::B2DPoint aStartA(aViewPosition - aNormalizedDirection);
                        const basegfx::B2DPoint aEndA(aViewPosition + aNormalizedDirection);
                        basegfx::B2DPolygon aLineA;
                        aLineA.append(aStartA);
                        aLineA.append(aEndA);
                        aLineA.transform(rViewInformation.getInverseViewTransformation());
                        PolygonMarkerPrimitive2D* pNewA = new PolygonMarkerPrimitive2D(aLineA, getRGBColA(), getRGBColB(), fLogicDashLen);
                        aTempPrimitiveTarget.push_back(pNewA);

                        const basegfx::B2DVector aPerpendicularNormalizedDirection(basegfx::getPerpendicular(aNormalizedDirection));
                        const basegfx::B2DPoint aStartB(aViewPosition - aPerpendicularNormalizedDirection);
                        const basegfx::B2DPoint aEndB(aViewPosition + aPerpendicularNormalizedDirection);
                        basegfx::B2DPolygon aLineB;
                        aLineB.append(aStartB);
                        aLineB.append(aEndB);
                        aLineB.transform(rViewInformation.getInverseViewTransformation());
                        PolygonMarkerPrimitive2D* pNewB = new PolygonMarkerPrimitive2D(aLineB, getRGBColA(), getRGBColB(), fLogicDashLen);
                        aTempPrimitiveTarget.push_back(pNewB);

                        break;
                    }
                    case HELPLINESTYLE2D_LINE :
                    {
                        basegfx::B2DPolygon aLine;

                        if(basegfx::areParallel(getDirection(), basegfx::B2DVector(1.0, 0.0)))
                        {
                            // parallel to X-Axis, get cuts with Y-Axes
                            const double fCutA((rViewInformation.getDiscreteViewport().getMinX() - aViewPosition.getX()) / getDirection().getX());
                            const double fCutB((rViewInformation.getDiscreteViewport().getMaxX() - aViewPosition.getX()) / getDirection().getX());
                            const basegfx::B2DPoint aPosA(aViewPosition + (fCutA * getDirection()));
                            const basegfx::B2DPoint aPosB(aViewPosition + (fCutB * getDirection()));
                            const bool bBothLeft(aPosA.getX() < rViewInformation.getDiscreteViewport().getMinX() && aPosB.getX() < rViewInformation.getDiscreteViewport().getMinX());
                            const bool bBothRight(aPosA.getX() > rViewInformation.getDiscreteViewport().getMaxX() && aPosB.getX() < rViewInformation.getDiscreteViewport().getMaxX());

                            if(!bBothLeft && !bBothRight)
                            {
                                aLine.append(aPosA);
                                aLine.append(aPosB);
                            }
                        }
                        else
                        {
                            // get cuts with X-Axes
                            const double fCutA((rViewInformation.getDiscreteViewport().getMinY() - aViewPosition.getY()) / getDirection().getY());
                            const double fCutB((rViewInformation.getDiscreteViewport().getMaxY() - aViewPosition.getY()) / getDirection().getY());
                            const basegfx::B2DPoint aPosA(aViewPosition + (fCutA * getDirection()));
                            const basegfx::B2DPoint aPosB(aViewPosition + (fCutB * getDirection()));
                            const bool bBothAbove(aPosA.getY() < rViewInformation.getDiscreteViewport().getMinY() && aPosB.getY() < rViewInformation.getDiscreteViewport().getMinY());
                            const bool bBothBelow(aPosA.getY() > rViewInformation.getDiscreteViewport().getMaxY() && aPosB.getY() < rViewInformation.getDiscreteViewport().getMaxY());

                            if(!bBothAbove && !bBothBelow)
                            {
                                aLine.append(aPosA);
                                aLine.append(aPosB);
                            }
                        }

                        if(aLine.count())
                        {
                            // clip against visible area
                            const basegfx::B2DPolyPolygon aResult(basegfx::tools::clipPolygonOnRange(aLine, rViewInformation.getDiscreteViewport(), true, true));

                            for(sal_uInt32 a(0L); a < aResult.count(); a++)
                            {
                                basegfx::B2DPolygon aPart(aResult.getB2DPolygon(a));
                                aPart.transform(rViewInformation.getInverseViewTransformation());
                                PolygonMarkerPrimitive2D* pNew = new PolygonMarkerPrimitive2D(aPart, getRGBColA(), getRGBColB(), fLogicDashLen);
                                aTempPrimitiveTarget.push_back(pNew);
                            }
                        }

                        break;
                    }
                }
            }

            // prepare return value
            Primitive2DSequence aRetval(aTempPrimitiveTarget.size());

            for(sal_uInt32 a(0L); a < aTempPrimitiveTarget.size(); a++)
            {
                const Primitive2DReference xRef(aTempPrimitiveTarget[a]);
                aRetval[a] = xRef;
            }

            return aRetval;
        }

        HelplinePrimitive2D::HelplinePrimitive2D(
            const basegfx::B2DPoint& rPosition,
            const basegfx::B2DVector& rDirection,
            HelplineStyle2D eStyle,
            const basegfx::BColor& rRGBColA,
            const basegfx::BColor& rRGBColB,
            double fViewDashLength)
        :   BasePrimitive2D(),
            maPosition(rPosition),
            maDirection(rDirection),
            meStyle(eStyle),
            maRGBColA(rRGBColA),
            maRGBColB(rRGBColB),
            mfViewDashLength(fViewDashLength),
            maLastViewTransformation(),
            maLastViewport()
        {
        }

        bool HelplinePrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            if(BasePrimitive2D::operator==(rPrimitive))
            {
                const HelplinePrimitive2D& rCompare = (HelplinePrimitive2D&)rPrimitive;

                return (getPosition() == rCompare.getPosition()
                    && getDirection() == rCompare.getDirection()
                    && getStyle() == rCompare.getStyle()
                    && getRGBColA() == rCompare.getRGBColA()
                    && getRGBColB() == rCompare.getRGBColB()
                    && getViewDashLength() == rCompare.getViewDashLength());
            }

            return false;
        }

        Primitive2DSequence HelplinePrimitive2D::get2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const
        {
            ::osl::MutexGuard aGuard( m_aMutex );

            if(getLocalDecomposition().hasElements())
            {
                if(maLastViewport != rViewInformation.getViewport() || maLastViewTransformation != rViewInformation.getViewTransformation())
                {
                    // conditions of last local decomposition have changed, delete
                    const_cast< HelplinePrimitive2D* >(this)->setLocalDecomposition(Primitive2DSequence());
                }
            }

            if(!getLocalDecomposition().hasElements())
            {
                // remember ViewRange and ViewTransformation
                const_cast< HelplinePrimitive2D* >(this)->maLastViewTransformation = rViewInformation.getViewTransformation();
                const_cast< HelplinePrimitive2D* >(this)->maLastViewport = rViewInformation.getViewport();
            }

            // use parent implementation
            return BasePrimitive2D::get2DDecomposition(rViewInformation);
        }

        // provide unique ID
        ImplPrimitrive2DIDBlock(HelplinePrimitive2D, PRIMITIVE2D_ID_HELPLINEPRIMITIVE2D)

    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
