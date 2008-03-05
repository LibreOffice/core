/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vclhelpergradient.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: aw $ $Date: 2008-03-05 09:15:45 $
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

#ifndef INCLUDED_DRAWINGLAYER_PROCESSOR2D_VCLHELPERGRADIENT_HXX
#include <vclhelpergradient.hxx>
#endif

#ifndef _BGFX_RANGE_B2DRANGE_HXX
#include <basegfx/range/b2drange.hxx>
#endif

#ifndef _SV_OUTDEV_HXX
#include <vcl/outdev.hxx>
#endif

#ifndef _BGFX_POLYGON_B2DPOLYGON_HXX
#include <basegfx/polygon/b2dpolygon.hxx>
#endif

#ifndef _BGFX_POLYPOLYGON_B2DPOLYGONTOOLS_HXX
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#endif

#ifndef _BGFX_POLYGON_B2DPOLYGONTOOLS_HXX
#include <basegfx/polygon/b2dpolygontools.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_TEXTURE_TEXTURE_HXX
#include <drawinglayer/texture/texture.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////
// support methods for vcl direct gradient renderering

namespace drawinglayer
{
    namespace
    {
        sal_uInt32 impCalcGradientSteps(OutputDevice& rOutDev, sal_uInt32 nSteps, const basegfx::B2DRange& rRange, sal_uInt32 nMaxDist)
        {
            if(nSteps == 0L)
            {
                const Size aSize(rOutDev.LogicToPixel(Size(basegfx::fround(rRange.getWidth()), basegfx::fround(rRange.getHeight()))));
                nSteps = (aSize.getWidth() + aSize.getHeight()) >> 3L;
            }

            if(nSteps < 2L)
            {
                nSteps = 2L;
            }

            if(nSteps > nMaxDist)
            {
                nSteps = nMaxDist;
            }

            return nSteps;
        }

        void impDrawGradientToOutDevSimple(
            OutputDevice& rOutDev,
            const basegfx::B2DPolyPolygon& rTargetForm,
            const ::std::vector< basegfx::B2DHomMatrix >& rMatrices,
            const ::std::vector< basegfx::BColor >& rColors,
            const basegfx::B2DPolygon& rUnitPolygon)
        {
            rOutDev.SetLineColor();

            for(sal_uInt32 a(0L); a < rColors.size(); a++)
            {
                // set correct color
                const basegfx::BColor aFillColor(rColors[a]);
                rOutDev.SetFillColor(Color(aFillColor));

                if(a)
                {
                    if(a - 1L < rMatrices.size())
                    {
                        basegfx::B2DPolygon aNewPoly(rUnitPolygon);
                        aNewPoly.transform(rMatrices[a - 1L]);
                        rOutDev.DrawPolygon(aNewPoly);
                    }
                }
                else
                {
                    rOutDev.DrawPolyPolygon(rTargetForm);
                }
            }
        }

        void impDrawGradientToOutDevComplex(
            OutputDevice& rOutDev,
            const basegfx::B2DPolyPolygon& rTargetForm,
            const ::std::vector< basegfx::B2DHomMatrix >& rMatrices,
            const ::std::vector< basegfx::BColor >& rColors,
            const basegfx::B2DPolygon& rUnitPolygon)
        {
            PolyPolygon aVclTargetForm(rTargetForm);
            ::std::vector< Polygon > aVclPolygons;
            sal_uInt32 a;

            // remember and set to XOR
            rOutDev.SetLineColor();
            rOutDev.Push(PUSH_RASTEROP);
            rOutDev.SetRasterOp(ROP_XOR);

            // draw gradient PolyPolygons
            for(a = 0L; a < rMatrices.size(); a++)
            {
                // create polygon and remember
                basegfx::B2DPolygon aNewPoly(rUnitPolygon);
                aNewPoly.transform(rMatrices[a]);
                aVclPolygons.push_back(Polygon(aNewPoly));

                // set correct color
                if(rColors.size() > a)
                {
                    const basegfx::BColor aFillColor(rColors[a]);
                    rOutDev.SetFillColor(Color(aFillColor));
                }

                // create vcl PolyPolygon and draw it
                if(a)
                {
                    PolyPolygon aVclPolyPoly(aVclPolygons[a - 1L]);
                    aVclPolyPoly.Insert(aVclPolygons[a]);
                    rOutDev.DrawPolyPolygon(aVclPolyPoly);
                }
                else
                {
                    PolyPolygon aVclPolyPoly(aVclTargetForm);
                    aVclPolyPoly.Insert(aVclPolygons[0L]);
                    rOutDev.DrawPolyPolygon(aVclPolyPoly);
                }
            }

            // draw last poly in last color
            if(rColors.size())
            {
                const basegfx::BColor aFillColor(rColors[rColors.size() - 1L]);
                rOutDev.SetFillColor(Color(aFillColor));
                rOutDev.DrawPolygon(aVclPolygons[aVclPolygons.size() - 1L]);
            }

            // draw object form in black and go back to XOR
            rOutDev.SetFillColor(COL_BLACK);
            rOutDev.SetRasterOp(ROP_0);
            rOutDev.DrawPolyPolygon(aVclTargetForm);
            rOutDev.SetRasterOp(ROP_XOR);

            // draw gradient PolyPolygons again
            for(a = 0L; a < rMatrices.size(); a++)
            {
                // set correct color
                if(rColors.size() > a)
                {
                    const basegfx::BColor aFillColor(rColors[a]);
                    rOutDev.SetFillColor(Color(aFillColor));
                }

                // create vcl PolyPolygon and draw it
                if(a)
                {
                    PolyPolygon aVclPolyPoly(aVclPolygons[a - 1L]);
                    aVclPolyPoly.Insert(aVclPolygons[a]);
                    rOutDev.DrawPolyPolygon(aVclPolyPoly);
                }
                else
                {
                    PolyPolygon aVclPolyPoly(aVclTargetForm);
                    aVclPolyPoly.Insert(aVclPolygons[0L]);
                    rOutDev.DrawPolyPolygon(aVclPolyPoly);
                }
            }

            // draw last poly in last color
            if(rColors.size())
            {
                const basegfx::BColor aFillColor(rColors[rColors.size() - 1L]);
                rOutDev.SetFillColor(Color(aFillColor));
                rOutDev.DrawPolygon(aVclPolygons[aVclPolygons.size() - 1L]);
            }

            // reset drawmode
            rOutDev.Pop();
        }
    } // end of anonymous namespace
} // end of namespace drawinglayer

namespace drawinglayer
{
    void impDrawGradientToOutDev(
        OutputDevice& rOutDev,
        const basegfx::B2DPolyPolygon& rTargetForm,
        attribute::GradientStyle eGradientStyle,
        sal_uInt32 nSteps,
        const basegfx::BColor& rStart,
        const basegfx::BColor& rEnd,
        double fBorder, double fAngle, double fOffsetX, double fOffsetY, bool bSimple)
    {
        const basegfx::B2DRange aOutlineRange(basegfx::tools::getRange(rTargetForm));
        ::std::vector< basegfx::B2DHomMatrix > aMatrices;
        ::std::vector< basegfx::BColor > aColors;
        basegfx::B2DPolygon aUnitPolygon;

        if(attribute::GRADIENTSTYLE_RADIAL == eGradientStyle || attribute::GRADIENTSTYLE_ELLIPTICAL == eGradientStyle)
        {
            const basegfx::B2DPoint aCircleCenter(0.5, 0.5);
            aUnitPolygon = basegfx::tools::createPolygonFromEllipse(aCircleCenter, 0.5, 0.5);
        }
        else
        {
            aUnitPolygon = basegfx::tools::createPolygonFromRect(basegfx::B2DRange(0.0, 0.0, 1.0, 1.0));
        }

        // make sure steps is not too high/low
        nSteps = impCalcGradientSteps(rOutDev, nSteps, aOutlineRange, sal_uInt32((rStart.getMaximumDistance(rEnd) * 127.5) + 0.5));

        // create geometries
        switch(eGradientStyle)
        {
            case attribute::GRADIENTSTYLE_LINEAR:
            {
                texture::GeoTexSvxGradientLinear aGradient(aOutlineRange, rStart, rEnd, nSteps, fBorder, fAngle);
                aGradient.appendTransformations(aMatrices);
                aGradient.appendColors(aColors);
                break;
            }
            case attribute::GRADIENTSTYLE_AXIAL:
            {
                texture::GeoTexSvxGradientAxial aGradient(aOutlineRange, rStart, rEnd, nSteps, fBorder, fAngle);
                aGradient.appendTransformations(aMatrices);
                aGradient.appendColors(aColors);
                break;
            }
            case attribute::GRADIENTSTYLE_RADIAL:
            {
                texture::GeoTexSvxGradientRadial aGradient(aOutlineRange, rStart, rEnd, nSteps, fBorder, fOffsetX, fOffsetY);
                aGradient.appendTransformations(aMatrices);
                aGradient.appendColors(aColors);
                break;
            }
            case attribute::GRADIENTSTYLE_ELLIPTICAL:
            {
                texture::GeoTexSvxGradientElliptical aGradient(aOutlineRange, rStart, rEnd, nSteps, fBorder, fOffsetX, fOffsetX, fAngle);
                aGradient.appendTransformations(aMatrices);
                aGradient.appendColors(aColors);
                break;
            }
            case attribute::GRADIENTSTYLE_SQUARE:
            {
                texture::GeoTexSvxGradientSquare aGradient(aOutlineRange, rStart, rEnd, nSteps, fBorder, fOffsetX, fOffsetX, fAngle);
                aGradient.appendTransformations(aMatrices);
                aGradient.appendColors(aColors);
                break;
            }
            case attribute::GRADIENTSTYLE_RECT:
            {
                texture::GeoTexSvxGradientRect aGradient(aOutlineRange, rStart, rEnd, nSteps, fBorder, fOffsetX, fOffsetX, fAngle);
                aGradient.appendTransformations(aMatrices);
                aGradient.appendColors(aColors);
                break;
            }
        }

        // paint them with mask using the XOR method
        if(aMatrices.size())
        {
            if(bSimple)
            {
                impDrawGradientToOutDevSimple(rOutDev, rTargetForm, aMatrices, aColors, aUnitPolygon);
            }
            else
            {
                impDrawGradientToOutDevComplex(rOutDev, rTargetForm, aMatrices, aColors, aUnitPolygon);
            }
        }
    }
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
