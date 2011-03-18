/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
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

#include <drawinglayer/primitive2d/sdrdecompositiontools2d.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <drawinglayer/primitive2d/polypolygonprimitive2d.hxx>
#include <drawinglayer/primitive2d/hiddengeometryprimitive2d.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        Primitive2DReference createHiddenGeometryPrimitives2D(
            bool bFilled,
            const basegfx::B2DHomMatrix& rMatrix)
        {
            const basegfx::B2DPolygon aUnitOutline(basegfx::tools::createUnitPolygon());

            return createHiddenGeometryPrimitives2D(
                bFilled,
                basegfx::B2DPolyPolygon(aUnitOutline),
                rMatrix);
        }

        Primitive2DReference createHiddenGeometryPrimitives2D(
            bool bFilled,
            const basegfx::B2DPolyPolygon& rPolyPolygon)
        {
            return createHiddenGeometryPrimitives2D(
                bFilled,
                rPolyPolygon,
                basegfx::B2DHomMatrix());
        }

        Primitive2DReference createHiddenGeometryPrimitives2D(
            bool bFilled,
            const basegfx::B2DRange& rRange)
        {
            return createHiddenGeometryPrimitives2D(
                bFilled,
                rRange,
                basegfx::B2DHomMatrix());
        }

        Primitive2DReference createHiddenGeometryPrimitives2D(
            bool bFilled,
            const basegfx::B2DRange& rRange,
            const basegfx::B2DHomMatrix& rMatrix)
        {
            const basegfx::B2DPolyPolygon aOutline(basegfx::tools::createPolygonFromRect(rRange));

            return createHiddenGeometryPrimitives2D(
                bFilled,
                aOutline,
                rMatrix);
        }

        Primitive2DReference createHiddenGeometryPrimitives2D(
            bool bFilled,
            const basegfx::B2DPolyPolygon& rPolyPolygon,
            const basegfx::B2DHomMatrix& rMatrix)
        {
            // create fill or line primitive
            Primitive2DReference xReference;
            basegfx::B2DPolyPolygon aScaledOutline(rPolyPolygon);
            aScaledOutline.transform(rMatrix);

            if(bFilled)
            {
                xReference = new PolyPolygonColorPrimitive2D(
                    basegfx::B2DPolyPolygon(aScaledOutline),
                    basegfx::BColor(0.0, 0.0, 0.0));
            }
            else
            {
                const basegfx::BColor aGrayTone(0xc0 / 255.0, 0xc0 / 255.0, 0xc0 / 255.0);

                xReference = new PolyPolygonHairlinePrimitive2D(
                    aScaledOutline,
                    aGrayTone);
            }

            // create HiddenGeometryPrimitive2D
            return Primitive2DReference(
                new HiddenGeometryPrimitive2D(Primitive2DSequence(&xReference, 1)));
        }
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
