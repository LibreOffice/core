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
#include "precompiled_sw.hxx"

#include <OverlayRanges.hxx>
#include <view.hxx>
#include <svx/sdrpaintwindow.hxx>
#include <svx/svdview.hxx>
#include <svx/sdr/overlay/overlaymanager.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygoncutter.hxx>
#include <drawinglayer/primitive2d/polypolygonprimitive2d.hxx>
#include <drawinglayer/primitive2d/unifiedtransparenceprimitive2d.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace
{
    // combine ranges geometrically to a single, ORed polygon
    basegfx::B2DPolyPolygon impCombineRangesToPolyPolygon(const std::vector< basegfx::B2DRange >& rRanges)
    {
        const sal_uInt32 nCount(rRanges.size());
        basegfx::B2DPolyPolygon aRetval;

        for(sal_uInt32 a(0); a < nCount; a++)
        {
            const basegfx::B2DPolygon aDiscretePolygon(basegfx::tools::createPolygonFromRect(rRanges[a]));

            if(0 == a)
            {
                aRetval.append(aDiscretePolygon);
            }
            else
            {
                aRetval = basegfx::tools::solvePolygonOperationOr(aRetval, basegfx::B2DPolyPolygon(aDiscretePolygon));
            }
        }

        return aRetval;
    }
}

namespace sw
{
    namespace overlay
    {
        drawinglayer::primitive2d::Primitive2DSequence OverlayRanges::createOverlayObjectPrimitive2DSequence()
        {
            const sal_uInt32 nCount(getRanges().size());
            drawinglayer::primitive2d::Primitive2DSequence aRetval;
            aRetval.realloc(nCount);
            for ( sal_uInt32 a = 0; a < nCount; ++a )
            {
                const basegfx::BColor aRGBColor(getBaseColor().getBColor());
                const basegfx::B2DPolygon aPolygon(basegfx::tools::createPolygonFromRect(maRanges[a]));
                aRetval[a] = drawinglayer::primitive2d::Primitive2DReference(
                    new drawinglayer::primitive2d::PolyPolygonColorPrimitive2D(
                    basegfx::B2DPolyPolygon(aPolygon),
                    aRGBColor));
            }
            // embed all rectangles in transparent paint
            const SvtOptionsDrawinglayer aSvtOptionsDrawinglayer;
            const sal_uInt16 nTransparence( aSvtOptionsDrawinglayer.GetTransparentSelectionPercent() );
            const double fTransparence( nTransparence / 100.0 );
            const drawinglayer::primitive2d::Primitive2DReference aUnifiedTransparence(
                new drawinglayer::primitive2d::UnifiedTransparencePrimitive2D(
                aRetval,
                fTransparence));

            if ( mbShowSolidBorder )
            {
                const basegfx::BColor aRGBColor(getBaseColor().getBColor());
                const basegfx::B2DPolyPolygon aPolyPolygon(impCombineRangesToPolyPolygon(getRanges()));
                const drawinglayer::primitive2d::Primitive2DReference aOutline(
                    new drawinglayer::primitive2d::PolyPolygonHairlinePrimitive2D(
                    aPolyPolygon,
                    aRGBColor));

                aRetval.realloc(2);
                aRetval[0] = aUnifiedTransparence;
                aRetval[1] = aOutline;
            }
            else
            {
                aRetval = drawinglayer::primitive2d::Primitive2DSequence(&aUnifiedTransparence, 1);
            }

            return aRetval;
        }

        /*static*/ OverlayRanges* OverlayRanges::CreateOverlayRange(
            SwView& rDocView,
            const Color& rColor,
            const std::vector< basegfx::B2DRange >& rRanges,
            const bool bShowSolidBorder )
        {
            OverlayRanges* pOverlayRanges = NULL;

            SdrView* pView = rDocView.GetDrawView();
            if ( pView != NULL )
            {
                SdrPaintWindow* pCandidate = pView->GetPaintWindow(0);
                sdr::overlay::OverlayManager* pTargetOverlay = pCandidate->GetOverlayManager();

                if ( pTargetOverlay != NULL )
                {
                    pOverlayRanges = new sw::overlay::OverlayRanges( rColor, rRanges, bShowSolidBorder );
                    pTargetOverlay->add( *pOverlayRanges );
                }
            }

            return pOverlayRanges;
        }

        OverlayRanges::OverlayRanges(
            const Color& rColor,
            const std::vector< basegfx::B2DRange >& rRanges,
            const bool bShowSolidBorder )
            : sdr::overlay::OverlayObject( rColor )
            , maRanges( rRanges )
            , mbShowSolidBorder( bShowSolidBorder )
        {
            // no AA for highlight overlays
            allowAntiAliase(false);
        }

        OverlayRanges::~OverlayRanges()
        {
            if( getOverlayManager() )
            {
                getOverlayManager()->remove(*this);
            }
        }

        void OverlayRanges::setRanges(const std::vector< basegfx::B2DRange >& rNew)
        {
            if(rNew != maRanges)
            {
                maRanges = rNew;
                objectChange();
            }
        }


        void OverlayRanges::ShowSolidBorder()
        {
            if ( !mbShowSolidBorder )
            {
                mbShowSolidBorder = true;
                objectChange();
            }
        }

        void OverlayRanges::HideSolidBorder()
        {
            if ( mbShowSolidBorder )
            {
                mbShowSolidBorder = false;
                objectChange();
            }
        }

    } // end of namespace overlay
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////
// eof
