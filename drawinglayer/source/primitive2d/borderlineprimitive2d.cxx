/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: borderlineprimitive2d.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: aw $ $Date: 2008-04-04 06:00:23 $
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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_BORDERLINEPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/borderlineprimitive2d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_PRIMITIVETYPES2D_HXX
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>
#endif

#ifndef _BGFX_POLYGON_B2DPOLYGON_HXX
#include <basegfx/polygon/b2dpolygon.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_POLYGONPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/polygonprimitive2d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_POLYPOLYGONPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/polypolygonprimitive2d.hxx>
#endif

#include <numeric>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        Primitive2DSequence BorderLinePrimitive2D::createLocalDecomposition(const geometry::ViewInformation2D& /*rViewInformation*/) const
        {
            Primitive2DSequence xRetval;
            const sal_uInt32 nCount(getMultiLineArray().size());

            if(nCount)
            {
                if(!getStart().equal(getEnd()))
                {
                    if(1 == nCount && basegfx::fTools::equal(1.0, getMultiLineArray()[0]))
                    {
                        // a single, centered hairline
                        basegfx::B2DPolygon aLine;

                        aLine.append(getStart());
                        aLine.append(getEnd());

                        xRetval.realloc(1);
                        xRetval[0] = Primitive2DReference(new PolygonHairlinePrimitive2D(aLine, getRGBColor()));
                    }
                    else
                    {
                        const double fLineWidth(::std::accumulate(getMultiLineArray().begin(), getMultiLineArray().end(), 0.0));
                        const double fHalfLW(fLineWidth / 2.0);
                        basegfx::B2DVector aVector(getEnd() - getStart());
                        aVector.normalize();
                        const basegfx::B2DVector aPerpendicular(basegfx::getNormalizedPerpendicular(aVector));
                        sal_uInt32 a(0);
                        ::std::vector< BasePrimitive2D* > aTargetVector;
                        double fStart(0.0);

                        for(a = 0; a < nCount; a++)
                        {
                            const double fDist(getMultiLineArray()[a]);

                            if(!(a % 2))
                            {
                                if(basegfx::fTools::equal(1.0, fDist))
                                {
                                    // hairline
                                    const double fOffset(fStart - fHalfLW);
                                    double fHorOffsetStart(0.0);
                                    double fHorOffsetEnd(0.0);
                                    basegfx::B2DPolygon aLine;

                                    if(MULTIEDGESTYLE_NONE != getStartStyle() || MULTIEDGESTYLE_NONE != getEndStyle())
                                    {
                                        if(MULTIEDGESTYLE_SHEARIN == getStartStyle())
                                        {
                                            fHorOffsetStart = -fOffset;
                                        }
                                        else if(MULTIEDGESTYLE_SHEAROUT == getStartStyle())
                                        {
                                            fHorOffsetStart = fOffset;
                                        }
                                        else if(MULTIEDGESTYLE_TIP == getStartStyle())
                                        {
                                            fHorOffsetStart = fabs(fOffset);
                                        }

                                        if(MULTIEDGESTYLE_SHEARIN == getEndStyle())
                                        {
                                            fHorOffsetEnd = -fOffset;
                                        }
                                        else if(MULTIEDGESTYLE_SHEAROUT == getEndStyle())
                                        {
                                            fHorOffsetEnd = fOffset;
                                        }
                                        else if(MULTIEDGESTYLE_TIP == getEndStyle())
                                        {
                                            fHorOffsetEnd = -fabs(fOffset);
                                        }
                                    }

                                    aLine.append(getStart() + (aPerpendicular * fOffset) + (aVector * fHorOffsetStart));
                                    aLine.append(getEnd() + (aPerpendicular * fOffset) + (aVector * fHorOffsetEnd));

                                    aTargetVector.push_back(new PolygonHairlinePrimitive2D(aLine, getRGBColor()));
                                }
                                else
                                {
                                    // fat line
                                    const double fOffsetA(fStart - fHalfLW);
                                    const double fOffsetB(fOffsetA + fDist);
                                    double fHorOffsetStartA(0.0);
                                    double fHorOffsetStartB(0.0);
                                    double fHorOffsetEndA(0.0);
                                    double fHorOffsetEndB(0.0);
                                    basegfx::B2DPolygon aFill;

                                    if(MULTIEDGESTYLE_NONE != getStartStyle() || MULTIEDGESTYLE_NONE != getEndStyle())
                                    {
                                        if(MULTIEDGESTYLE_SHEARIN == getStartStyle())
                                        {
                                            fHorOffsetStartA = -fOffsetA;
                                            fHorOffsetStartB = -fOffsetB;
                                        }
                                        else if(MULTIEDGESTYLE_SHEAROUT == getStartStyle())
                                        {
                                            fHorOffsetStartA = fOffsetA;
                                            fHorOffsetStartB = fOffsetB;
                                        }
                                        else if(MULTIEDGESTYLE_TIP == getStartStyle())
                                        {
                                            fHorOffsetStartA = fabs(fOffsetA);
                                            fHorOffsetStartB = fabs(fOffsetB);
                                        }

                                        if(MULTIEDGESTYLE_SHEARIN == getEndStyle())
                                        {
                                            fHorOffsetEndA = -fOffsetA;
                                            fHorOffsetEndB = -fOffsetB;
                                        }
                                        else if(MULTIEDGESTYLE_SHEAROUT == getEndStyle())
                                        {
                                            fHorOffsetEndA = fOffsetA;
                                            fHorOffsetEndB = fOffsetB;
                                        }
                                        else if(MULTIEDGESTYLE_TIP == getEndStyle())
                                        {
                                            fHorOffsetEndA = -fabs(fOffsetA);
                                            fHorOffsetEndB = -fabs(fOffsetB);
                                        }
                                    }

                                    aFill.append(getStart() + (aPerpendicular * fOffsetA) + (aVector * fHorOffsetStartA));
                                    aFill.append(getEnd() + (aPerpendicular * fOffsetA) + (aVector * fHorOffsetEndA));

                                    if(MULTIEDGESTYLE_TIP == getEndStyle() && fOffsetA < 0.0 && fOffsetB > 0.0)
                                    {
                                        // the polygon outer side travels over the center; insert end point as tip point
                                        aFill.append(getEnd());
                                    }

                                    aFill.append(getEnd() + (aPerpendicular * fOffsetB) + (aVector * fHorOffsetEndB));
                                    aFill.append(getStart() + (aPerpendicular * fOffsetB) + (aVector * fHorOffsetStartB));

                                    if(MULTIEDGESTYLE_TIP == getStartStyle() && fOffsetB > 0.0 && fOffsetA < 0.0)
                                    {
                                        // the polygon outer side travels over the center; insert start point as tip point
                                        aFill.append(getStart());
                                    }

                                    aFill.setClosed(true);
                                    aTargetVector.push_back(new PolyPolygonColorPrimitive2D(basegfx::B2DPolyPolygon(aFill), getRGBColor()));
                                }
                            }

                            fStart += fDist;
                        }

                        xRetval.realloc(aTargetVector.size());

                        for(a = 0; a < aTargetVector.size(); a++)
                        {
                            xRetval[a] = Primitive2DReference(aTargetVector[a]);
                        }
                    }
                }
            }

            return xRetval;
        }

        BorderLinePrimitive2D::BorderLinePrimitive2D(
            const basegfx::B2DPoint& rStart,
            const basegfx::B2DPoint& rEnd,
            const ::std::vector< double >& rMultiLineArray,
            MultiEdgeStyle2D aStartStyle,
            MultiEdgeStyle2D aEndStyle,
            const basegfx::BColor& rRGBColor)
        :   BasePrimitive2D(),
            maStart(rStart),
            maEnd(rEnd),
            maMultiLineArray(rMultiLineArray),
            maStartStyle(aStartStyle),
            maEndStyle(aEndStyle),
            maRGBColor(rRGBColor)
        {
        }

        bool BorderLinePrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            if(BasePrimitive2D::operator==(rPrimitive))
            {
                const BorderLinePrimitive2D& rCompare = (BorderLinePrimitive2D&)rPrimitive;

                return (getStart() == rCompare.getStart()
                    && getEnd() == rCompare.getEnd()
                    && getMultiLineArray() == rCompare.getMultiLineArray()
                    && getStartStyle() == rCompare.getStartStyle()
                    && getEndStyle() == rCompare.getEndStyle()
                    && getRGBColor() == rCompare.getRGBColor());
            }

            return false;
        }

        // provide unique ID
        ImplPrimitrive2DIDBlock(BorderLinePrimitive2D, PRIMITIVE2D_ID_BORDERLINEPRIMITIVE2D)

    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
