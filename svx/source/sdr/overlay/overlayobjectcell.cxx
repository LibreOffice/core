/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
