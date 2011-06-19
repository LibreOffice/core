/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#include <basegfx/numeric/ftools.hxx>
#include <vcl/outdev.hxx>
#include <vcl/hatch.hxx>
#include <svx/sdr/overlay/overlayobjectcell.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <drawinglayer/primitive2d/polypolygonprimitive2d.hxx>
#include <drawinglayer/primitive2d/unifiedtransparenceprimitive2d.hxx>
#include <drawinglayer/primitive2d/invertprimitive2d.hxx>

using namespace ::basegfx;

// #114409#
namespace sdr
{
    namespace overlay
    {
        OverlayObjectCell::OverlayObjectCell( CellOverlayType eType, const Color& rColor, const RangeVector& rRects )
        :   OverlayObject( rColor ),
            mePaintType( eType ),
            maRectangles( rRects )
        {
            // no AA for selection overlays
            allowAntiAliase(false);
        }

        OverlayObjectCell::~OverlayObjectCell()
        {
        }

        drawinglayer::primitive2d::Primitive2DSequence OverlayObjectCell::createOverlayObjectPrimitive2DSequence()
        {
            drawinglayer::primitive2d::Primitive2DSequence aRetval;
            const sal_uInt32 nCount(maRectangles.size());

            if(nCount)
            {
                const basegfx::BColor aRGBColor(getBaseColor().getBColor());
                aRetval.realloc(nCount);

                // create primitives for all ranges
                for(sal_uInt32 a(0); a < nCount; a++)
                {
                    const basegfx::B2DRange& rRange(maRectangles[a]);
                    const basegfx::B2DPolygon aPolygon(basegfx::tools::createPolygonFromRect(rRange));

                    aRetval[a] = drawinglayer::primitive2d::Primitive2DReference(
                        new drawinglayer::primitive2d::PolyPolygonColorPrimitive2D(
                            basegfx::B2DPolyPolygon(aPolygon),
                            aRGBColor));
                }


                if(mePaintType == CELL_OVERLAY_TRANSPARENT)
                {
                    // embed in 50% transparent paint
                    const drawinglayer::primitive2d::Primitive2DReference aUnifiedTransparence(
                        new drawinglayer::primitive2d::UnifiedTransparencePrimitive2D(
                            aRetval,
                            0.5));

                    aRetval = drawinglayer::primitive2d::Primitive2DSequence(&aUnifiedTransparence, 1);
                }
                else // CELL_OVERLAY_INVERT
                {
                    // embed in invert primitive
                    const drawinglayer::primitive2d::Primitive2DReference aInvert(
                        new drawinglayer::primitive2d::InvertPrimitive2D(
                            aRetval));

                    aRetval = drawinglayer::primitive2d::Primitive2DSequence(&aInvert, 1);
                }
            }

            return aRetval;
        }
    } // end of namespace overlay
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////
// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
