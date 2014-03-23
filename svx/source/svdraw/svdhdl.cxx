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

#include <svx/svdhdl.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svdetc.hxx>
#include <svx/svdmrkv.hxx>
#include <vcl/window.hxx>
#include <vcl/virdev.hxx>
#include <tools/poly.hxx>
#include <vcl/bmpacc.hxx>
#include <svx/sxekitm.hxx>
#include <svx/svdstr.hrc>
#include <svx/svdglob.hxx>
#include <svx/svdmodel.hxx>
#include "gradtrns.hxx"
#include <svx/xflgrit.hxx>
#include <svx/svdundo.hxx>
#include <svx/dialmgr.hxx>
#include <svx/xflftrit.hxx>
#include <svx/svdopath.hxx>
#include <basegfx/vector/b2dvector.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <svx/sdr/overlay/overlaymanager.hxx>
#include <svx/sdr/overlay/overlayanimatedbitmapex.hxx>
#include <svx/sdr/overlay/overlaybitmapex.hxx>
#include <svx/sdr/overlay/overlayline.hxx>
#include <svx/sdr/overlay/overlaytriangle.hxx>
#include <svx/sdr/overlay/overlayrectangle.hxx>
#include <svx/sdrpagewindow.hxx>
#include <svx/sdrpaintwindow.hxx>
#include <vcl/svapp.hxx>
#include <svx/sdr/overlay/overlaypolypolygon.hxx>
#include <vcl/lazydelete.hxx>
#include <svx/svdlegacy.hxx>
#include <algorithm>

#include <basegfx/polygon/b2dpolygontools.hxx>
#include <drawinglayer/primitive2d/polypolygonprimitive2d.hxx>
#include <svx/sdr/overlay/overlayprimitive2dsequenceobject.hxx>
#include <drawinglayer/primitive2d/graphicprimitive2d.hxx>
#include <drawinglayer/primitive2d/maskprimitive2d.hxx>
#include <drawinglayer/primitive2d/unifiedtransparenceprimitive2d.hxx>
#include <drawinglayer/primitive2d/polygonprimitive2d.hxx>

////////////////////////////////////////////////////////////////////////////////////////////////////
// #i15222#
// Due to the ressource problems in Win95/98 with bitmap ressources i
// will change this handle bitmap provinging class. Old version was splitting
// and preparing all small handle bitmaps in device bitmap format, now this will
// be done on the fly. Thus, tehre is only the one big bitmap remembered. With
// three source bitmaps, this will be 3 system bitmap ressources instead of hundreds.
// The price for that needs to be evaluated. Maybe we will need another change here
// if this is too expensive.
class SdrHdlBitmapSet
{
    // the bitmap holding all infos
    BitmapEx                    maMarkersBitmap;

    // the cropped Bitmaps for reusage
    ::std::vector< BitmapEx >   maRealMarkers;

    // elpers
    BitmapEx& impGetOrCreateTargetBitmap(sal_uInt16 nIndex, const Rectangle& rRectangle);

public:
    SdrHdlBitmapSet(sal_uInt16 nResId);
    ~SdrHdlBitmapSet();

    const BitmapEx& GetBitmapEx(BitmapMarkerKind eKindOfMarker, sal_uInt16 nInd=0);
};

////////////////////////////////////////////////////////////////////////////////////////////////////

#define KIND_COUNT          (14)
#define INDEX_COUNT         (6)
#define INDIVIDUAL_COUNT    (4)

SdrHdlBitmapSet::SdrHdlBitmapSet(sal_uInt16 nResId)
:   maMarkersBitmap(ResId(nResId, *ImpGetResMgr())), // just use ressource with alpha channel
    // 14 kinds (BitmapMarkerKind) use index [0..5], 4 extra
    maRealMarkers((KIND_COUNT * INDEX_COUNT) + INDIVIDUAL_COUNT)
{
}

SdrHdlBitmapSet::~SdrHdlBitmapSet()
{
}

BitmapEx& SdrHdlBitmapSet::impGetOrCreateTargetBitmap(sal_uInt16 nIndex, const Rectangle& rRectangle)
{
    BitmapEx& rTargetBitmap = maRealMarkers[nIndex];

    if(rTargetBitmap.IsEmpty())
    {
        rTargetBitmap = maMarkersBitmap;
        rTargetBitmap.Crop(rRectangle);
    }

    return rTargetBitmap;
}

// change getting of bitmap to use the big ressource bitmap
const BitmapEx& SdrHdlBitmapSet::GetBitmapEx(BitmapMarkerKind eKindOfMarker, sal_uInt16 nInd)
{
    // fill in size and source position in maMarkersBitmap
    const sal_uInt16 nYPos(nInd * 11);

    switch(eKindOfMarker)
    {
        default:
        {
            DBG_ERROR( "unknown kind of marker" );
            // no break here, return Rect_7x7 as default
        }
        case Rect_7x7:
        {
            return impGetOrCreateTargetBitmap((0 * INDEX_COUNT) + nInd, Rectangle(Point(0, nYPos), Size(7, 7)));
        }

        case Rect_9x9:
        {
            return impGetOrCreateTargetBitmap((1 * INDEX_COUNT) + nInd, Rectangle(Point(7, nYPos), Size(9, 9)));
        }

        case Rect_11x11:
        {
            return impGetOrCreateTargetBitmap((2 * INDEX_COUNT) + nInd, Rectangle(Point(16, nYPos), Size(11, 11)));
        }

        case Rect_13x13:
        {
            const sal_uInt16 nIndex((3 * INDEX_COUNT) + nInd);

            switch(nInd)
            {
                case 0:
                {
                    return impGetOrCreateTargetBitmap(nIndex, Rectangle(Point(72, 66), Size(13, 13)));
                }
                case 1:
                {
                    return impGetOrCreateTargetBitmap(nIndex, Rectangle(Point(85, 66), Size(13, 13)));
                }
                case 2:
                {
                    return impGetOrCreateTargetBitmap(nIndex, Rectangle(Point(72, 79), Size(13, 13)));
                }
                case 3:
                {
                    return impGetOrCreateTargetBitmap(nIndex, Rectangle(Point(85, 79), Size(13, 13)));
                }
                case 4:
                {
                    return impGetOrCreateTargetBitmap(nIndex, Rectangle(Point(98, 79), Size(13, 13)));
                }
                default: // case 5:
                {
                    return impGetOrCreateTargetBitmap(nIndex, Rectangle(Point(98, 66), Size(13, 13)));
                }
            }
        }

        case Circ_7x7:
        case Customshape_7x7:
        {
            return impGetOrCreateTargetBitmap((4 * INDEX_COUNT) + nInd, Rectangle(Point(27, nYPos), Size(7, 7)));
        }

        case Circ_9x9:
        case Customshape_9x9:
        {
            return impGetOrCreateTargetBitmap((5 * INDEX_COUNT) + nInd, Rectangle(Point(34, nYPos), Size(9, 9)));
        }

        case Circ_11x11:
        case Customshape_11x11:
        {
            return impGetOrCreateTargetBitmap((6 * INDEX_COUNT) + nInd, Rectangle(Point(43, nYPos), Size(11, 11)));
        }

        case Elli_7x9:
        {
            return impGetOrCreateTargetBitmap((7 * INDEX_COUNT) + nInd, Rectangle(Point(54, nYPos), Size(7, 9)));
        }

        case Elli_9x11:
        {
            return impGetOrCreateTargetBitmap((8 * INDEX_COUNT) + nInd, Rectangle(Point(61, nYPos), Size(9, 11)));
        }

        case Elli_9x7:
        {
            return impGetOrCreateTargetBitmap((9 * INDEX_COUNT) + nInd, Rectangle(Point(70, nYPos), Size(9, 7)));
        }

        case Elli_11x9:
        {
            return impGetOrCreateTargetBitmap((10 * INDEX_COUNT) + nInd, Rectangle(Point(79, nYPos), Size(11, 9)));
        }

        case RectPlus_7x7:
        {
            return impGetOrCreateTargetBitmap((11 * INDEX_COUNT) + nInd, Rectangle(Point(90, nYPos), Size(7, 7)));
        }

        case RectPlus_9x9:
        {
            return impGetOrCreateTargetBitmap((12 * INDEX_COUNT) + nInd, Rectangle(Point(97, nYPos), Size(9, 9)));
        }

        case RectPlus_11x11:
        {
            return impGetOrCreateTargetBitmap((13 * INDEX_COUNT) + nInd, Rectangle(Point(106, nYPos), Size(11, 11)));
        }

        case Crosshair:
        {
            return impGetOrCreateTargetBitmap((KIND_COUNT * INDEX_COUNT) + 0, Rectangle(Point(0, 68), Size(15, 15)));
        }

        case Glue:
        {
            return impGetOrCreateTargetBitmap((KIND_COUNT * INDEX_COUNT) + 1, Rectangle(Point(15, 74), Size(9, 9)));
        }

        case Anchor: // #101688# AnchorTR for SW
        case AnchorTR:
        {
            return impGetOrCreateTargetBitmap((KIND_COUNT * INDEX_COUNT) + 2, Rectangle(Point(24, 68), Size(24, 24)));
        }

        // #98388# add AnchorPressed to be able to aninate anchor control
        case AnchorPressed:
        case AnchorPressedTR:
        {
            return impGetOrCreateTargetBitmap((KIND_COUNT * INDEX_COUNT) + 3, Rectangle(Point(48, 68), Size(24, 24)));
        }
    }

    // cannot happen since all pathes return something; return Rect_7x7 as default (see switch)
    return maRealMarkers[0];
}

////////////////////////////////////////////////////////////////////////////////////////////////////

SdrHdlBitmapSet& getSimpleSet()
{
    static vcl::DeleteOnDeinit< SdrHdlBitmapSet > aSimpleSet(new SdrHdlBitmapSet(SIP_SA_MARKERS));
    return *aSimpleSet.get();
}

SdrHdlBitmapSet& getModernSet()
{
    static vcl::DeleteOnDeinit< SdrHdlBitmapSet > aModernSet(new SdrHdlBitmapSet(SIP_SA_FINE_MARKERS));
    return *aModernSet.get();
}

SdrHdlBitmapSet& getHighContrastSet()
{
    static vcl::DeleteOnDeinit< SdrHdlBitmapSet > aHighContrastSet(new SdrHdlBitmapSet(SIP_SA_ACCESSIBILITY_MARKERS));
    return *aHighContrastSet.get();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

SdrHdl::SdrHdl(
    SdrHdlList& rHdlList,
    const SdrObject* pSdrHdlObject,
    SdrHdlKind eNewKind,
    const basegfx::B2DPoint& rPosition,
    bool bIsFrameHandle)
:   boost::noncopyable(),
    mrHdlList(rHdlList),
    mpSdrHdlObject(pSdrHdlObject),
    meKind(eNewKind),
    maPosition(rPosition),
    maOverlayGroup(),
    mnObjHdlNum(0),
    mnPolyNum(0),
    mnPPntNum(0),
    mnSourceHdlNum(0),
    mbSelect(false),
    mb1PixMore(false),
    mbPlusHdl(false),
    mbIsFrameHandle(bIsFrameHandle),
    mbMoveOutside(false),
    mbMouseOver(false)
{
    // add to owning list
    mrHdlList.maList.push_back(this);
}

SdrHdl::~SdrHdl()
{
    GetRidOfIAObject();
    OSL_ENSURE(!mrHdlList.maList.size(), "SdrHdl deleted from someone else than SdrHdlList (!)");
}

void SdrHdl::Set1PixMore(bool bJa)
{
    if(mb1PixMore != bJa)
    {
        mb1PixMore = bJa;

        // create new display
        Touch();
    }
}

void SdrHdl::SetMoveOutside( bool bMoveOutside )
{
    if(mbMoveOutside != bMoveOutside)
    {
        mbMoveOutside = bMoveOutside;

        // create new display
        Touch();
    }
}

const basegfx::B2DPoint& SdrHdl::getPosition() const
{
    return maPosition;
}

void SdrHdl::setPosition(const basegfx::B2DPoint& rNew)
{
    if(maPosition != rNew)
    {
        // remember new position
        maPosition = rNew;

        // create new display
        Touch();
    }
}

void SdrHdl::SetSelected(bool bJa)
{
    if(mbSelect != bJa)
    {
        // remember new value
        mbSelect = bJa;

        // create new display
        Touch();
    }
}

void SdrHdl::Touch()
{
    // force update of graphic representation, but only when not empty. When
    // it is empty, the finishing call to CreateVisualizations() is not yet
    // done but can be expected. If not empty, attributes are changed outside
    // handle creation (SdrMarkView::RecreateAllMarkHandles()), e.g. blinking or MouseOver
    if(!getOverlayObjectList().isEmpty())
    {
        CreateB2dIAObject();
    }
}

void SdrHdl::GetRidOfIAObject()
{
    // OVERLAYMANAGER
    maOverlayGroup.clear();
}

void SdrHdl::CreateB2dIAObject()
{
    // first throw away old one
    GetRidOfIAObject();

    SdrPageView* pPageView = mrHdlList.GetViewFromSdrHdlList().GetSdrPageView();

    if(pPageView)
    {
        for(sal_uInt32 a(0); a < pPageView->PageWindowCount(); a++)
        {
            const SdrPageWindow& rPageWindow = *pPageView->GetPageWindow(a);

            if(rPageWindow.GetPaintWindow().OutputToWindow())
            {
                ::sdr::overlay::OverlayManager* pOverlayManager = rPageWindow.GetOverlayManager();

                if(pOverlayManager)
                {
                    CreateB2dIAObject(*pOverlayManager);
                }
            }
        }
    }
}

void SdrHdl::CreateB2dIAObject(::sdr::overlay::OverlayManager& rOverlayManager)
{
    BitmapColorIndex eColIndex = LightGreen;
    BitmapMarkerKind eKindOfMarker = Rect_7x7;
    bool bRot = mrHdlList.IsRotateShear();

    if(!IsFrameHandle())
    {
        eColIndex = (mbSelect) ? Cyan : LightCyan;
    }
    else if(bRot)
    {
        // Drehhandles in Rot
        eColIndex = (mbSelect) ? Red : LightRed;
    }

    switch(meKind)
    {
        case HDL_MOVE:
        {
            eKindOfMarker = (mb1PixMore) ? Rect_9x9 : Rect_7x7;
            break;
        }
        case HDL_UPLFT:
        case HDL_UPRGT:
        case HDL_LWLFT:
        case HDL_LWRGT:
        {
            // corner handles
            if(bRot)
            {
                eKindOfMarker = Circ_7x7;
            }
            else
            {
                eKindOfMarker = Rect_7x7;
            }
            break;
        }
        case HDL_UPPER:
        case HDL_LOWER:
        {
            // Upper/Lower handles
            if(bRot)
            {
                eKindOfMarker = Elli_9x7;
            }
            else
            {
                eKindOfMarker = Rect_7x7;
            }
            break;
        }
        case HDL_LEFT:
        case HDL_RIGHT:
        {
            // Left/Right handles
            if(bRot)
            {
                eKindOfMarker = Elli_7x9;
            }
            else
            {
                eKindOfMarker = Rect_7x7;
            }
            break;
        }
        case HDL_POLY:
        {
            if(bRot)
            {
                eKindOfMarker = (mb1PixMore) ? Circ_9x9 : Circ_7x7;
            }
            else
            {
                eKindOfMarker = (mb1PixMore) ? Rect_9x9 : Rect_7x7;
                break;
            }
            break;
        }
        case HDL_BWGT: // weight at poly
        {
            eKindOfMarker = Circ_7x7;
            break;
        }
        case HDL_CIRC:
        {
            eKindOfMarker = Rect_11x11;
            break;
        }
        case HDL_REF1:
        case HDL_REF2:
        {
            eKindOfMarker = Crosshair;
            break;
        }
        case HDL_GLUE:
        {
            eKindOfMarker = Glue;
            break;
        }
        case HDL_ANCHOR:
        {
            eKindOfMarker = Anchor;
            break;
        }
        case HDL_USER:
        {
            break;
        }
        // #101688# top right anchor for SW
        case HDL_ANCHOR_TR:
        {
            eKindOfMarker = AnchorTR;
            break;
        }

        // for SJ and the CustomShapeHandles:
        case HDL_CUSTOMSHAPE1:
        {
            eKindOfMarker = (mb1PixMore) ? Customshape_9x9 : Customshape_7x7;
            eColIndex = Yellow;
            break;
        }
        default:
            break;
    }

    sal_Int16 nMoveOutsideX(0);
    sal_Int16 nMoveOutsideY(0);

    // add offset if necessary
    if(mrHdlList.IsMoveOutside() || mbMoveOutside)
    {
        const basegfx::B2DVector aOffset(rOverlayManager.getOutputDevice().GetInverseViewTransformation() * basegfx::B2DVector(4.0, 4.0));

        if(meKind == HDL_UPLFT || meKind == HDL_UPPER || meKind == HDL_UPRGT)
        {
            nMoveOutsideY = -1;
        }

        if(meKind == HDL_LWLFT || meKind == HDL_LOWER || meKind == HDL_LWRGT)
        {
            nMoveOutsideY = 1;
        }

        if(meKind == HDL_UPLFT || meKind == HDL_LEFT  || meKind == HDL_LWLFT)
        {
            nMoveOutsideX = -1;
        }

        if(meKind == HDL_UPRGT || meKind == HDL_RIGHT || meKind == HDL_LWRGT)
        {
            nMoveOutsideX = 1;
        }
    }

    ::sdr::overlay::OverlayObject* pNewOverlayObject = CreateOverlayObject(
        maPosition,
        eColIndex,
        eKindOfMarker,
        nMoveOutsideX,
        nMoveOutsideY);

    rOverlayManager.add(*pNewOverlayObject);
    maOverlayGroup.append(*pNewOverlayObject);
}

BitmapMarkerKind SdrHdl::GetNextBigger(BitmapMarkerKind eKnd) const
{
    BitmapMarkerKind eRetval(eKnd);

    switch(eKnd)
    {
        case Rect_7x7:          eRetval = Rect_9x9;         break;
        case Rect_9x9:          eRetval = Rect_11x11;       break;
        case Rect_11x11:        eRetval = Rect_13x13;       break;
        //case Rect_13x13:      eRetval = ; break;

        case Circ_7x7:          eRetval = Circ_9x9;         break;
        case Circ_9x9:          eRetval = Circ_11x11;       break;
        //case Circ_11x11:      eRetval = ; break;

        case Customshape_7x7:       eRetval = Customshape_9x9;      break;
        case Customshape_9x9:       eRetval = Customshape_11x11;    break;
        //case Customshape_11x11:   eRetval = ; break;

        case Elli_7x9:          eRetval = Elli_9x11;        break;
        //case Elli_9x11:           eRetval = ; break;

        case Elli_9x7:          eRetval = Elli_11x9;        break;
        //case Elli_11x9:           eRetval = ; break;

        case RectPlus_7x7:      eRetval = RectPlus_9x9;     break;
        case RectPlus_9x9:      eRetval = RectPlus_11x11;   break;
        //case RectPlus_11x11:  eRetval = ; break;

        //case Crosshair:           eRetval = ; break;
        //case Glue:                eRetval = ; break;

        // #98388# let anchor blink with it's pressed state
        case Anchor:            eRetval = AnchorPressed;    break;

        // #101688# same for AnchorTR
        case AnchorTR:          eRetval = AnchorPressedTR;  break;
        default:
            break;
    }

    return eRetval;
}

// #101928#
BitmapEx SdrHdl::ImpGetBitmapEx(BitmapMarkerKind eKindOfMarker, sal_uInt16 nInd, bool bFine, bool bIsHighContrast)
{
    if(bIsHighContrast)
    {
        return getHighContrastSet().GetBitmapEx(eKindOfMarker, nInd);
    }
    else
    {
        if(bFine)
        {
            return getModernSet().GetBitmapEx(eKindOfMarker, nInd);
        }
        else
        {
            return getSimpleSet().GetBitmapEx(eKindOfMarker, nInd);
        }
    }
}

::sdr::overlay::OverlayObject* SdrHdl::CreateOverlayObject(
    const basegfx::B2DPoint& rPos,
    BitmapColorIndex eColIndex,
    BitmapMarkerKind eKindOfMarker,
    sal_Int16 nMoveOutsideX,
    sal_Int16 nMoveOutsideY)
{
    ::sdr::overlay::OverlayObject* pRetval = 0L;
    bool bIsFineHdl(mrHdlList.IsFineHdl());
    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
    bool bIsHighContrast(rStyleSettings.GetHighContrastMode());

    // support bigger sizes
    bool bForceBiggerSize(false);

    if(mrHdlList.GetHdlSize() > 3)
    {
        switch(eKindOfMarker)
        {
            case Anchor:
            case AnchorPressed:
            case AnchorTR:
            case AnchorPressedTR:
            {
                // #121463# For anchor, do not simply make bigger because of HdlSize,
                // do it dependent of IsSelected() which Writer can set in drag mode
                if(IsSelected())
                {
                    bForceBiggerSize = true;
                }
                break;
            }
            default:
            {
                bForceBiggerSize = true;
                break;
            }
        }
    }

    // #101928# ...for high contrast, too.
    if(!bForceBiggerSize && bIsHighContrast)
    {
        // #107925#
        // ...but not for anchors, else they will not blink when activated
        if(Anchor != eKindOfMarker && AnchorTR != eKindOfMarker)
        {
            bForceBiggerSize = true;
        }
    }

    if(bForceBiggerSize)
    {
        eKindOfMarker = GetNextBigger(eKindOfMarker);
    }

    // #97016# II This handle has the focus, visualize it
    if(IsFocusHdl() && mrHdlList.GetFocusHdl() == this)
    {
        // create animated handle
        BitmapMarkerKind eNextBigger = GetNextBigger(eKindOfMarker);

        if(eNextBigger == eKindOfMarker)
        {
            // this may happen for the not supported getting-bigger types.
            // Choose an alternative here
            switch(eKindOfMarker)
            {
                case Rect_13x13:        eNextBigger = Rect_11x11;   break;
                case Circ_11x11:        eNextBigger = Elli_11x9;    break;
                case Elli_9x11:         eNextBigger = Elli_11x9;    break;
                case Elli_11x9:         eNextBigger = Elli_9x11;    break;
                case RectPlus_11x11:    eNextBigger = Rect_13x13;   break;

                case Crosshair:
                    eNextBigger = Glue;
                    break;

                case Glue:
                    eNextBigger = Crosshair;
                    break;
                default:
                    break;
            }
        }

        // create animated hdl
        // #101928# use ImpGetBitmapEx(...) now
        BitmapEx aBmpEx1 = ImpGetBitmapEx(eKindOfMarker, (sal_uInt16)eColIndex, bIsFineHdl, bIsHighContrast);
        BitmapEx aBmpEx2 = ImpGetBitmapEx(eNextBigger, (sal_uInt16)eColIndex, bIsFineHdl, bIsHighContrast);

        // #i53216# Use system cursor blink time. Use the unsigned value.
        const sal_uInt32 nBlinkTime((sal_uInt32)Application::GetSettings().GetStyleSettings().GetCursorBlinkTime());

        if(eKindOfMarker == Anchor || eKindOfMarker == AnchorPressed)
        {
            // #98388# when anchor is used take upper left as reference point inside the handle
            pRetval = new ::sdr::overlay::OverlayAnimatedBitmapEx(rPos, aBmpEx1, aBmpEx2, nBlinkTime);
        }
        else if(eKindOfMarker == AnchorTR || eKindOfMarker == AnchorPressedTR)
        {
            // #101688# AnchorTR for SW, take top right as (0,0)
            pRetval = new ::sdr::overlay::OverlayAnimatedBitmapEx(rPos, aBmpEx1, aBmpEx2, nBlinkTime,
                (sal_uInt16)(aBmpEx1.GetSizePixel().Width() - 1), 0,
                (sal_uInt16)(aBmpEx2.GetSizePixel().Width() - 1), 0);
        }
        else
        {
            // create centered handle as default
            pRetval = new ::sdr::overlay::OverlayAnimatedBitmapEx(rPos, aBmpEx1, aBmpEx2, nBlinkTime,
                (sal_uInt16)(aBmpEx1.GetSizePixel().Width() - 1) >> 1,
                (sal_uInt16)(aBmpEx1.GetSizePixel().Height() - 1) >> 1,
                (sal_uInt16)(aBmpEx2.GetSizePixel().Width() - 1) >> 1,
                (sal_uInt16)(aBmpEx2.GetSizePixel().Height() - 1) >> 1);
        }
    }
    else
    {
        // create normal handle
        // #101928# use ImpGetBitmapEx(...) now
        BitmapEx aBmpEx = ImpGetBitmapEx(eKindOfMarker, (sal_uInt16)eColIndex, bIsFineHdl, bIsHighContrast);

        if(eKindOfMarker == Anchor || eKindOfMarker == AnchorPressed)
        {
            // #98388# upper left as reference point inside the handle for AnchorPressed, too
            pRetval = new ::sdr::overlay::OverlayBitmapEx(rPos, aBmpEx);
        }
        else if(eKindOfMarker == AnchorTR || eKindOfMarker == AnchorPressedTR)
        {
            // #101688# AnchorTR for SW, take top right as (0,0)
            pRetval = new ::sdr::overlay::OverlayBitmapEx(rPos, aBmpEx,
                (sal_uInt16)(aBmpEx.GetSizePixel().Width() - 1), 0);
        }
        else
        {
            sal_uInt16 nCenX((sal_uInt16)(aBmpEx.GetSizePixel().Width() - 1L) >> 1);
            sal_uInt16 nCenY((sal_uInt16)(aBmpEx.GetSizePixel().Height() - 1L) >> 1);

            if(nMoveOutsideX > 0)
            {
                nCenX = 0;
            }
            else if(nMoveOutsideX < 0)
            {
                nCenX = (sal_uInt16)(aBmpEx.GetSizePixel().Width() - 1);
            }

            if(nMoveOutsideY > 0)
            {
                nCenY = 0;
            }
            else if(nMoveOutsideY < 0)
            {
                nCenY = (sal_uInt16)(aBmpEx.GetSizePixel().Height() - 1);
            }

            // create centered handle as default
            pRetval = new ::sdr::overlay::OverlayBitmapEx(rPos, aBmpEx, nCenX, nCenY);
        }
    }

    return pRetval;
}

bool SdrHdl::IsHdlHit(const basegfx::B2DPoint& rPosition) const
{
    // OVERLAYMANAGER
    return maOverlayGroup.isHitLogic(rPosition);
}

Pointer SdrHdl::GetPointer() const
{
    PointerStyle ePtr=POINTER_MOVE;
    const bool bSize=meKind>=HDL_UPLFT && meKind<=HDL_LWRGT;
    const bool bRot=mrHdlList.IsRotateShear();
    const bool bDis=mrHdlList.IsDistortShear();
    if (bSize && (bRot || bDis)) {
        switch (meKind) {
            case HDL_UPLFT: case HDL_UPRGT:
            case HDL_LWLFT: case HDL_LWRGT: ePtr=bRot ? POINTER_ROTATE : POINTER_REFHAND; break;
            case HDL_LEFT : case HDL_RIGHT: ePtr=POINTER_VSHEAR; break;
            case HDL_UPPER: case HDL_LOWER: ePtr=POINTER_HSHEAR; break;
            default:
                break;
        }
    }
    else
    {
        // Fuer Resize von gedrehten Rechtecken die Mauszeiger etwas mitdrehen
        bool bDone(false);

        if(bSize && mpSdrHdlObject)
        {
            const sal_Int32 aOldRot(sdr::legacy::GetRotateAngle(*mpSdrHdlObject));

            if(aOldRot)
            {
                sal_Int32 nHdlWink(0);

                switch (meKind)
                {
                    case HDL_LWRGT: nHdlWink=31500; break;
                    case HDL_LOWER: nHdlWink=27000; break;
                    case HDL_LWLFT: nHdlWink=22500; break;
                    case HDL_LEFT : nHdlWink=18000; break;
                    case HDL_UPLFT: nHdlWink=13500; break;
                    case HDL_UPPER: nHdlWink=9000;  break;
                    case HDL_UPRGT: nHdlWink=4500;  break;
                    case HDL_RIGHT: nHdlWink=0;     break;
                    default:
                        break;
                }

                nHdlWink += aOldRot + 2249;

                while(nHdlWink < 0)
                {
                    nHdlWink += 36000;
                }

                while(nHdlWink >= 36000)
                {
                    nHdlWink -= 36000;
                }

                nHdlWink/=4500;

                switch ((sal_uInt8)nHdlWink)
                {
                    case 0: ePtr=POINTER_ESIZE;  break;
                    case 1: ePtr=POINTER_NESIZE; break;
                    case 2: ePtr=POINTER_NSIZE;  break;
                    case 3: ePtr=POINTER_NWSIZE; break;
                    case 4: ePtr=POINTER_WSIZE;  break;
                    case 5: ePtr=POINTER_SWSIZE; break;
                    case 6: ePtr=POINTER_SSIZE;  break;
                    case 7: ePtr=POINTER_SESIZE; break;
                }

                bDone = true;
            }
        }

        if(!bDone)
        {
            switch (meKind)
            {
                case HDL_UPLFT: ePtr=POINTER_NWSIZE;  break;
                case HDL_UPPER: ePtr=POINTER_NSIZE;     break;
                case HDL_UPRGT: ePtr=POINTER_NESIZE;  break;
                case HDL_LEFT : ePtr=POINTER_WSIZE;     break;
                case HDL_RIGHT: ePtr=POINTER_ESIZE;     break;
                case HDL_LWLFT: ePtr=POINTER_SWSIZE;  break;
                case HDL_LOWER: ePtr=POINTER_SSIZE;     break;
                case HDL_LWRGT: ePtr=POINTER_SESIZE;  break;
                case HDL_POLY : ePtr=POINTER_MOVEPOINT; break;
                case HDL_CIRC : ePtr=POINTER_HAND;      break;
                case HDL_REF1 : ePtr=POINTER_REFHAND;   break;
                case HDL_REF2 : ePtr=POINTER_REFHAND;   break;
                case HDL_BWGT : ePtr=POINTER_MOVEBEZIERWEIGHT; break;
                case HDL_GLUE : ePtr=POINTER_MOVEPOINT; break;
                case HDL_CUSTOMSHAPE1 : ePtr=POINTER_HAND; break;
                default:
                    break;
            }
        }
    }
    return Pointer(ePtr);
}

// #97016# II
bool SdrHdl::IsFocusHdl() const
{
    switch(meKind)
    {
        case HDL_UPLFT:     // Oben links
        case HDL_UPPER:     // Oben
        case HDL_UPRGT:     // Oben rechts
        case HDL_LEFT:      // Links
        case HDL_RIGHT:     // Rechts
        case HDL_LWLFT:     // Unten links
        case HDL_LOWER:     // Unten
        case HDL_LWRGT:     // Unten rechts
        {
            // if it's a activated TextEdit, it's moved to extended points
            if(mrHdlList.IsMoveOutside())
                return sal_False;
            else
                return sal_True;
        }

        case HDL_MOVE:      // Handle zum Verschieben des Objekts
        case HDL_POLY:      // Punktselektion an Polygon oder Bezierkurve
        case HDL_BWGT:      // Gewicht an einer Bezierkurve
        case HDL_CIRC:      // Winkel an Kreissegmenten, Eckenradius am Rect
        case HDL_REF1:      // Referenzpunkt 1, z.B. Rotationsmitte
        case HDL_REF2:      // Referenzpunkt 2, z.B. Endpunkt der Spiegelachse
        //case HDL_MIRX:        // Die Spiegelachse selbst
        case HDL_GLUE:      // GluePoint

        // #98388# do NOT activate here, let SW implement their own SdrHdl and
        // overload IsFocusHdl() there to make the anchor accessible
        //case HDL_ANCHOR:      // anchor symbol (SD, SW)
        // #101688# same for AnchorTR
        //case HDL_ANCHOR_TR:   // anchor symbol (SD, SW)

        //case HDL_TRNS:        // interactive transparence
        //case HDL_GRAD:        // interactive gradient
        //case HDL_COLR:        // interactive color

        // for SJ and the CustomShapeHandles:
        case HDL_CUSTOMSHAPE1:

        case HDL_USER:
        {
            return sal_True;
        }

        default:
        {
            return sal_False;
        }
    }
}

void SdrHdl::onMouseEnter(const MouseEvent& /*rMEvt*/)
{
}

void SdrHdl::onMouseLeave()
{
}

bool SdrHdl::isMouseOver() const
{
    return mbMouseOver;
}

void SdrHdl::setMouseOver(bool bNew)
{
    if(mbMouseOver != bNew)
    {
        // remember new value
        mbMouseOver = bNew;

        // create new display
        Touch();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// #105678# Help struct for re-sorting handles

struct ImplHdlAndIndex
{
    SdrHdl*                     mpHdl;
    sal_uInt32                  mnIndex;
};

// #105678# Help method for sorting handles taking care of OrdNums, keeping order in
// single objects and re-sorting polygon handles intuitively
extern "C" int __LOADONCALLAPI ImplSortHdlFunc( const void* pVoid1, const void* pVoid2 )
{
    const ImplHdlAndIndex* p1 = (ImplHdlAndIndex*)pVoid1;
    const ImplHdlAndIndex* p2 = (ImplHdlAndIndex*)pVoid2;

    if(p1->mpHdl->GetObj() == p2->mpHdl->GetObj())
    {
        const SdrPathObj* pSdrPathObj = dynamic_cast< const SdrPathObj* >(p1->mpHdl->GetObj());

        if(pSdrPathObj)
        {
            // same object and a path object
            if((p1->mpHdl->GetKind() == HDL_POLY || p1->mpHdl->GetKind() == HDL_BWGT)
                && (p2->mpHdl->GetKind() == HDL_POLY || p2->mpHdl->GetKind() == HDL_BWGT))
            {
                // both handles are point or control handles
                if(p1->mpHdl->GetPolyNum() == p2->mpHdl->GetPolyNum())
                {
                    if(p1->mpHdl->GetPointNum() < p2->mpHdl->GetPointNum())
                    {
                        return -1;
                    }
                    else
                    {
                        return 1;
                    }
                }
                else if(p1->mpHdl->GetPolyNum() < p2->mpHdl->GetPolyNum())
                {
                    return -1;
                }
                else
                {
                    return 1;
                }
            }
        }
    }
    else
    {
        if(!p1->mpHdl->GetObj())
        {
            return -1;
        }
        else if(!p2->mpHdl->GetObj())
        {
            return 1;
        }
        else
        {
            // different objects, use OrdNum for sort
            const sal_uInt32 nOrdNum1 = p1->mpHdl->GetObj()->GetNavigationPosition();
            const sal_uInt32 nOrdNum2 = p2->mpHdl->GetObj()->GetNavigationPosition();

            if(nOrdNum1 < nOrdNum2)
            {
                return -1;
            }
            else
            {
                return 1;
            }
        }
    }

    // fallback to indices
    if(p1->mnIndex < p2->mnIndex)
    {
        return -1;
    }
    else
    {
        return 1;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

SdrHdlList::SdrHdlList(SdrMarkView& rV)
:   boost::noncopyable(),
    mnFocusIndex(CONTAINER_ENTRY_NOTFOUND),
    mrView(rV),
    maList(),
    mnHdlSize(3),
    mbRotateShear(false),
    mbDistortShear(false),
    mbMoveOutside(false),
    mbFineHandles(false)
{
}

SdrHdlList::~SdrHdlList()
{
    Clear();
}

void SdrHdlList::CreateVisualizations()
{
    for(sal_uInt32 a(0); a < maList.size(); a++)
    {
        maList[a]->CreateB2dIAObject();
    }
}

SdrHdl* SdrHdlList::GetHdlByIndex(sal_uInt32 nNum) const
{
    if(nNum < maList.size())
    {
        return *(maList.begin() + nNum);
    }
    else
    {
        return 0;
    }
}

void SdrHdlList::TravelFocusHdl(bool bForward)
{
    // security correction
    if(CONTAINER_ENTRY_NOTFOUND != mnFocusIndex && mnFocusIndex >= GetHdlCount())
    {
        mnFocusIndex = CONTAINER_ENTRY_NOTFOUND;
    }

    if(GetHdlCount())
    {
        // take care of old handle
        const sal_uInt32 nOldHdlNum(mnFocusIndex);
        SdrHdl* pOld = CONTAINER_ENTRY_NOTFOUND != nOldHdlNum ? GetHdlByIndex(nOldHdlNum) : 0;

        if(pOld)
        {
            // switch off old handle
            mnFocusIndex = CONTAINER_ENTRY_NOTFOUND;
            pOld->Touch();
        }

        // #105678# Alloc pointer array for sorted handle list
        ImplHdlAndIndex* pHdlAndIndex = new ImplHdlAndIndex[GetHdlCount()];

        // #105678# build sorted handle list
        sal_uInt32 a(0);

        for(a = 0; a < GetHdlCount(); a++)
        {
            pHdlAndIndex[a].mpHdl = GetHdlByIndex(a);
            pHdlAndIndex[a].mnIndex = a;
        }

        // #105678# qsort all entries
        qsort(pHdlAndIndex, GetHdlCount(), sizeof(ImplHdlAndIndex), ImplSortHdlFunc);

        // #105678# look for old num in sorted array
        sal_uInt32 nOldHdl(nOldHdlNum);

        if(nOldHdlNum != CONTAINER_ENTRY_NOTFOUND)
        {
            for(a = 0; a < GetHdlCount(); a++)
            {
                if(pHdlAndIndex[a].mpHdl == pOld)
                {
                    nOldHdl = a;
                    break;
                }
            }
        }

        // #105678# build new HdlNum
        sal_uInt32 nNewHdl(nOldHdl);

        // #105678# do the focus travel
        if(bForward)
        {
            if(nOldHdl != CONTAINER_ENTRY_NOTFOUND)
            {
                if(nOldHdl == GetHdlCount() - 1)
                {
                    // end forward run
                    nNewHdl = CONTAINER_ENTRY_NOTFOUND;
                }
                else
                {
                    // simply the next handle
                    nNewHdl++;
                }
            }
            else
            {
                // start forward run at first entry
                nNewHdl = 0;
            }
        }
        else
        {
            if(nOldHdl == CONTAINER_ENTRY_NOTFOUND)
            {
                // start backward run at last entry
                nNewHdl = GetHdlCount() - 1;

            }
            else
            {
                if(nOldHdl == 0)
                {
                    // end backward run
                    nNewHdl = CONTAINER_ENTRY_NOTFOUND;
                }
                else
                {
                    // simply the previous handle
                    nNewHdl--;
                }
            }
        }

        // #105678# build new HdlNum
        sal_uInt32 nNewHdlNum(nNewHdl);

        // look for old num in sorted array
        if(nNewHdl != CONTAINER_ENTRY_NOTFOUND)
        {
            SdrHdl* pNew = pHdlAndIndex[nNewHdl].mpHdl;

            for(a = 0; a < GetHdlCount(); a++)
            {
                if(GetHdlByIndex(a) == pNew)
                {
                    nNewHdlNum = a;
                    break;
                }
            }
        }

        // take care of next handle
        if(nOldHdlNum != nNewHdlNum)
        {
            mnFocusIndex = nNewHdlNum;
            SdrHdl* pNew = GetHdlByIndex(mnFocusIndex);

            if(pNew)
            {
                pNew->Touch();
            }
        }

        // #105678# free mem again
        delete [] pHdlAndIndex;
    }
}

SdrHdl* SdrHdlList::GetFocusHdl() const
{
    if(CONTAINER_ENTRY_NOTFOUND != mnFocusIndex  && mnFocusIndex < GetHdlCount())
    {
        return GetHdlByIndex(mnFocusIndex);
    }
    else
    {
        return 0;
    }
}

void SdrHdlList::SetFocusHdl(SdrHdl* pNew)
{
    if(pNew)
    {
        SdrHdl* pActual = GetFocusHdl();

        if(!pActual || pActual != pNew)
        {
            const sal_uInt32 nNewHdlNum(GetHdlNum(pNew));

            if(CONTAINER_ENTRY_NOTFOUND != nNewHdlNum)
            {
                mnFocusIndex = nNewHdlNum;

                if(pActual)
                {
                    pActual->Touch();
                }

                if(pNew)
                {
                    pNew->Touch();
                }
            }
        }
    }
}

void SdrHdlList::ResetFocusHdl()
{
    SdrHdl* pHdl = GetFocusHdl();

    mnFocusIndex = CONTAINER_ENTRY_NOTFOUND;

    if(pHdl)
    {
        pHdl->Touch();
    }
}

void SdrHdlList::SetHdlSize(sal_uInt16 nSiz)
{
    if(GetHdlSize() != nSiz)
    {
        // remember new value
        mnHdlSize = nSiz;

        // propagate change to IAOs
        for(sal_uInt32 i(0); i < GetHdlCount(); i++)
        {
            SdrHdl* pHdl = GetHdlByIndex(i);
            pHdl->Touch();
        }
    }
}

void SdrHdlList::SetMoveOutside(bool bOn)
{
    if(mbMoveOutside != bOn)
    {
        // remember new value
        mbMoveOutside = bOn;

        // propagate change to IAOs
        for(sal_uInt32 i(0); i < GetHdlCount(); i++)
        {
            SdrHdl* pHdl = GetHdlByIndex(i);
            pHdl->Touch();
        }
    }
}

void SdrHdlList::SetFineHdl(bool bOn)
{
    if(mbFineHandles != bOn)
    {
        // remember new state
        mbFineHandles = bOn;

        // propagate change to IAOs
        for(sal_uInt32 i(0); i < GetHdlCount(); i++)
                    {
            SdrHdl* pHdl = GetHdlByIndex(i);
            pHdl->Touch();
        }
    }
}

void SdrHdlList::Clear()
{
    // keep a copy ad clear early to avoid that the delete calls
    // for the SdrHdl have to iterate over the list to remove themselves
    const SdrHdlContainerType aCopy(maList);
    maList.clear();

    for(sal_uInt32 i(0); i < aCopy.size(); i++)
    {
        delete aCopy[i];
    }

    mbRotateShear = false;
    mbDistortShear = false;
}

namespace
{
    struct SdrHdlComparator
    {
        bool operator()(const SdrHdl* pA, const SdrHdl* pB)
        {
            OSL_ENSURE(pA && pB, "SdrHdlComparator: empty pointer (!)");
            const SdrHdlKind eKind1(pA->GetKind());
            const SdrHdlKind eKind2(pB->GetKind());

            // Level 1: Erst normale Handles, dann Glue, dann User, dann Plushandles, dann Retpunkt-Handles
            unsigned n1(1);
            unsigned n2(1);

            if(eKind1 != eKind2)
            {
                if(HDL_REF1 == eKind1 || HDL_REF2 == eKind1 || HDL_MIRX == eKind1)
                {
                    n1 = 5;
                }
                else if(HDL_GLUE == eKind1)
                {
                    n1 = 2;
                }
                else if(HDL_USER == eKind1)
                {
                    n1 = 3;
                }
                else if(HDL_SMARTTAG == eKind1)
                {
                    n1 = 0;
                }

                if(HDL_REF1 == eKind2 || HDL_REF2 == eKind2 || HDL_MIRX == eKind2)
                {
                    n2 = 5;
                }
                else if(HDL_GLUE == eKind2)
                {
                    n2 = 2;
                }
                else if(HDL_USER == eKind2)
                {
                    n2 = 3;
                }
                else if(HDL_SMARTTAG == eKind2)
                {
                    n2 = 0;
                }
            }

            if(pA->IsPlusHdl())
            {
                n1 = 4;
            }

            if(pB->IsPlusHdl())
            {
                n2 = 4;
            }

            if(n1 == n2)
            {
                // Level 2: Position (x+y)
                const SdrObject* pObj1 = pA->GetObj();
                const SdrObject* pObj2 = pB->GetObj();

                if(pObj1 == pObj2)
                {
                    const sal_uInt32 nNum1(pA->GetObjHdlNum());
                    const sal_uInt32 nNum2(pB->GetObjHdlNum());

                    if(nNum1 == nNum2)
                    {
                        // #48763#
                        if(eKind1 == eKind2)
                        {
                            return pA < pB; // Notloesung, um immer die gleiche Sortierung zu haben
                        }
                        else
                        {
                            return (sal_uInt16)eKind1 < (sal_uInt16)eKind2;
                        }
                    }
                    else
                    {
                        return nNum1 < nNum2;
                    }
                }
                else
                {
                    return pObj1 < pObj2;
                }
            }
            else
            {
                return n1 < n2;
            }

            // should never happen and is even unreachable; just leave it here for security
            return pA->getPosition().getX() < pB->getPosition().getX();
        }
    };
}

void SdrHdlList::Sort()
{
    // #97016# II: remember current focused handle
    SdrHdl* pPrev = GetFocusHdl();

    ::std::sort(maList.begin(), maList.end(), SdrHdlComparator());

    // #97016# II: get now and compare
    SdrHdl* pNow = GetFocusHdl();

    if(pPrev != pNow)
    {
        if(pPrev)
        {
            pPrev->Touch();
        }

        if(pNow)
        {
            pNow->Touch();
        }
    }
}

sal_uInt32 SdrHdlList::GetHdlNum(const SdrHdl* pHdl) const
{
    if(!pHdl)
    {
        return CONTAINER_ENTRY_NOTFOUND;
    }

    sal_uInt32 a(0);

    for(SdrHdlContainerType::const_iterator aCandidate(maList.begin());
        aCandidate != maList.end(); a++, aCandidate++)
    {
        if(*aCandidate == pHdl)
        {
            return a;
        }
    }

    return CONTAINER_ENTRY_NOTFOUND;
}

SdrHdl* SdrHdlList::IsHdlListHit(const basegfx::B2DPoint& rPosition) const
{
    SdrHdl* pRet = 0;
    const sal_uInt32 nAnz(GetHdlCount());
    sal_uInt32 nNum(nAnz);

    while(nNum && !pRet)
    {
        nNum--;
        SdrHdl* pHdl = GetHdlByIndex(nNum);

        if(pHdl->IsHdlHit(rPosition))
        {
            pRet = pHdl;
        }
    }

    return pRet;
}

SdrHdl* SdrHdlList::GetHdlByKind(SdrHdlKind eKind1) const
{
   for(sal_uInt32 i(0); i < GetHdlCount(); i++)
    {
       SdrHdl* pHdl = GetHdlByIndex(i);

       if(pHdl->GetKind() == eKind1)
        {
           return pHdl;
        }
    }

   return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// class SdrHdlColor

SdrHdlColor::SdrHdlColor(
    SdrHdlList& rHdlList,
    const SdrObject& rSdrHdlObject,
    const basegfx::B2DPoint& rRef,
    Color aCol,
    const Size& rSize,
    bool bLum)
:   SdrHdl(rHdlList, &rSdrHdlObject, HDL_COLR, rRef),
    aMarkerSize(rSize),
    bUseLuminance(bLum)
{
    if(IsUseLuminance())
        aCol = GetLuminance(aCol);

    // remember color
    aMarkerColor = aCol;
}

SdrHdlColor::~SdrHdlColor()
{
}

void SdrHdlColor::CreateB2dIAObject(::sdr::overlay::OverlayManager& rOverlayManager)
{
    const Bitmap aBmpCol(CreateColorDropper(aMarkerColor));
    ::sdr::overlay::OverlayObject* pNewOverlayObject = new
        ::sdr::overlay::OverlayBitmapEx(
            maPosition,
            BitmapEx(aBmpCol),
            (sal_uInt16)(aBmpCol.GetSizePixel().Width() - 1) >> 1,
            (sal_uInt16)(aBmpCol.GetSizePixel().Height() - 1) >> 1
        );

    rOverlayManager.add(*pNewOverlayObject);
    maOverlayGroup.append(*pNewOverlayObject);
}

Bitmap SdrHdlColor::CreateColorDropper(Color aCol)
{
    // get the Bitmap
    Bitmap aRetval(aMarkerSize, 24);
    aRetval.Erase(aCol);

    // get write access
    BitmapWriteAccess* pWrite = aRetval.AcquireWriteAccess();
    DBG_ASSERT(pWrite, "Got NO write access to a new Bitmap !!!");

    if(pWrite)
    {
        // draw outer border
        sal_Int32 nWidth = aMarkerSize.Width();
        sal_Int32 nHeight = aMarkerSize.Height();

        pWrite->SetLineColor(Color(COL_LIGHTGRAY));
        pWrite->DrawLine(Point(0, 0), Point(0, nHeight - 1));
        pWrite->DrawLine(Point(1, 0), Point(nWidth - 1, 0));
        pWrite->SetLineColor(Color(COL_GRAY));
        pWrite->DrawLine(Point(1, nHeight - 1), Point(nWidth - 1, nHeight - 1));
        pWrite->DrawLine(Point(nWidth - 1, 1), Point(nWidth - 1, nHeight - 2));

        // draw lighter UpperLeft
        const Color aLightColor(
            (sal_uInt8)(::std::min((sal_Int16)((sal_Int16)aCol.GetRed() + (sal_Int16)0x0040), (sal_Int16)0x00ff)),
            (sal_uInt8)(::std::min((sal_Int16)((sal_Int16)aCol.GetGreen() + (sal_Int16)0x0040), (sal_Int16)0x00ff)),
            (sal_uInt8)(::std::min((sal_Int16)((sal_Int16)aCol.GetBlue() + (sal_Int16)0x0040), (sal_Int16)0x00ff)));
        pWrite->SetLineColor(aLightColor);
        pWrite->DrawLine(Point(1, 1), Point(1, nHeight - 2));
        pWrite->DrawLine(Point(2, 1), Point(nWidth - 2, 1));

        // draw darker LowerRight
        const Color aDarkColor(
            (sal_uInt8)(::std::max((sal_Int16)((sal_Int16)aCol.GetRed() - (sal_Int16)0x0040), (sal_Int16)0x0000)),
            (sal_uInt8)(::std::max((sal_Int16)((sal_Int16)aCol.GetGreen() - (sal_Int16)0x0040), (sal_Int16)0x0000)),
            (sal_uInt8)(::std::max((sal_Int16)((sal_Int16)aCol.GetBlue() - (sal_Int16)0x0040), (sal_Int16)0x0000)));
        pWrite->SetLineColor(aDarkColor);
        pWrite->DrawLine(Point(2, nHeight - 2), Point(nWidth - 2, nHeight - 2));
        pWrite->DrawLine(Point(nWidth - 2, 2), Point(nWidth - 2, nHeight - 3));

        // get rid of write access
        delete pWrite;
    }

    return aRetval;
}

Color SdrHdlColor::GetLuminance(const Color& rCol)
{
    sal_uInt8 aLum = rCol.GetLuminance();
    Color aRetval(aLum, aLum, aLum);
    return aRetval;
}

void SdrHdlColor::CallColorChangeLink()
{
    aColorChangeHdl.Call(this);
}

void SdrHdlColor::SetColor(Color aNew, bool bCallLink)
{
    if(IsUseLuminance())
        aNew = GetLuminance(aNew);

    if(aMarkerColor != aNew)
    {
        // remember new color
        aMarkerColor = aNew;

        // create new display
        Touch();

        // tell about change
        if(bCallLink)
            CallColorChangeLink();
    }
}

void SdrHdlColor::SetSize(const Size& rNew)
{
    if(rNew != aMarkerSize)
    {
        // remember new size
        aMarkerSize = rNew;

        // create new display
        Touch();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// class SdrHdlGradient

SdrHdlGradient::SdrHdlGradient(
    SdrHdlList& rHdlList,
    const SdrObject& rSdrHdlObject,
    SdrHdlColor& rSdrHdlColor1,
    SdrHdlColor& rSdrHdlColor2,
    bool bGrad)
:   SdrHdl(rHdlList, &rSdrHdlObject, bGrad ? HDL_GRAD : HDL_TRNS, rSdrHdlColor1.getPosition()),
    mrColHdl1(rSdrHdlColor1),
    mrColHdl2(rSdrHdlColor2),
    bGradient(bGrad)
{
}

SdrHdlGradient::~SdrHdlGradient()
{
}

void SdrHdlGradient::CreateB2dIAObject(::sdr::overlay::OverlayManager& rOverlayManager)
{
    // striped line in between
    basegfx::B2DVector aVec(mrColHdl2.getPosition() - mrColHdl1.getPosition());
    double fVecLen = aVec.getLength();
    double fLongPercentArrow = (1.0 - 0.05) * fVecLen;
    double fHalfArrowWidth = (0.05 * 0.5) * fVecLen;

    aVec.normalize();

    const basegfx::B2DVector aPerpend(-aVec.getY(), aVec.getX());
    const basegfx::B2DPoint aMidPos(mrColHdl1.getPosition() + (aVec * fLongPercentArrow));

    ::sdr::overlay::OverlayObject* pNewOverlayObject = new
        ::sdr::overlay::OverlayLineStriped(
            mrColHdl1.getPosition(),
            aMidPos);

    pNewOverlayObject->setBaseColor(IsGradient() ? Color(COL_BLACK) : Color(COL_BLUE));
    rOverlayManager.add(*pNewOverlayObject);
    maOverlayGroup.append(*pNewOverlayObject);

    // arrowhead
    const basegfx::B2DPoint aPositionLeft(aMidPos + (aPerpend * fHalfArrowWidth));
    const basegfx::B2DPoint aPositionRight(aMidPos- (aPerpend * fHalfArrowWidth));

    pNewOverlayObject = new
        ::sdr::overlay::OverlayTriangle(
            aPositionLeft,
            mrColHdl2.getPosition(),
            aPositionRight,
            IsGradient() ? Color(COL_BLACK) : Color(COL_BLUE));

    rOverlayManager.add(*pNewOverlayObject);
    maOverlayGroup.append(*pNewOverlayObject);
}

IMPL_LINK(SdrHdlGradient, ColorChangeHdl, SdrHdl*, /*pHdl*/)
{
    FromIAOToItem(true, true);

    return 0;
}

const basegfx::B2DPoint& SdrHdlGradient::getPosition() const
{
    return mrColHdl1.getPosition();
}

void SdrHdlGradient::setPosition(const basegfx::B2DPoint& rNew)
{
    // call parent
    SdrHdl::setPosition(rNew);

    if(rNew != mrColHdl1.getPosition())
    {
        // remember new position
        mrColHdl1.setPosition(rNew);

        // create new display
        Touch();
    }
}

const basegfx::B2DPoint& SdrHdlGradient::get2ndPosition() const
{
    return mrColHdl2.getPosition();
}

void SdrHdlGradient::set2ndPosition(const basegfx::B2DPoint& rNew)
{
    if(rNew != mrColHdl2.getPosition())
    {
        // remember new position
        mrColHdl2.setPosition(rNew);

        // create new display
        Touch();
    }
}

void SdrHdlGradient::FromIAOToItem(bool bSetItemOnObject, bool bUndo)
{
    SdrObject* pTarget = const_cast< SdrObject* >(GetObj());

    if(pTarget)
    {
        // from IAO positions and colors to gradient
        const SfxItemSet& rSet = pTarget->GetMergedItemSet();
        GradTransformer aGradTransformer;
        GradTransGradient aOldGradTransGradient;
        GradTransGradient aGradTransGradient;
        GradTransVector aGradTransVector;
        String aString;

        aGradTransVector.maPositionA = mrColHdl1.getPosition();
        aGradTransVector.maPositionB = mrColHdl2.getPosition();
        aGradTransVector.aCol1 = mrColHdl1.GetColor();
        aGradTransVector.aCol2 = mrColHdl2.GetColor();

        if(IsGradient())
        {
            aOldGradTransGradient.aGradient = ((XFillGradientItem&)rSet.Get(XATTR_FILLGRADIENT)).GetGradientValue();
        }
        else
        {
            aOldGradTransGradient.aGradient = ((XFillFloatTransparenceItem&)rSet.Get(XATTR_FILLFLOATTRANSPARENCE)).GetGradientValue();
        }

        // transform vector data to gradient
        aGradTransformer.VecToGrad(aGradTransVector, aGradTransGradient, aOldGradTransGradient, pTarget, bMoveSingleHandle, bMoveFirstHandle);

        if(bSetItemOnObject)
        {
            SfxItemSet aNewSet(pTarget->GetObjectItemPool());

            if(IsGradient())
            {
                aString = String();
                XFillGradientItem aNewGradItem(aString, aGradTransGradient.aGradient);
                aNewSet.Put(aNewGradItem);
            }
            else
            {
                aString = String();
                XFillFloatTransparenceItem aNewTransItem(aString, aGradTransGradient.aGradient);
                aNewSet.Put(aNewTransItem);
            }

            SdrModel& rSdrModel = pTarget->getSdrModelFromSdrObject();

            if(bUndo && rSdrModel.IsUndoEnabled())
            {
                rSdrModel.BegUndo(SVX_RESSTR(IsGradient() ? SIP_XA_FILLGRADIENT : SIP_XA_FILLTRANSPARENCE));
                rSdrModel.AddUndo(rSdrModel.GetSdrUndoFactory().CreateUndoAttrObject(*pTarget));
                rSdrModel.EndUndo();
            }

            pTarget->SetMergedItemSetAndBroadcast(aNewSet);
        }

        // back transformation, set values on pIAOHandle
        aGradTransformer.GradToVec(aGradTransGradient, aGradTransVector, pTarget);

        setPosition(aGradTransVector.maPositionA);
        mrColHdl1.setPosition(aGradTransVector.maPositionA);
        mrColHdl2.setPosition(aGradTransVector.maPositionB);
        mrColHdl1.SetColor(aGradTransVector.aCol1);
        mrColHdl2.SetColor(aGradTransVector.aCol2);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

SdrHdlLine::~SdrHdlLine()
{
}

void SdrHdlLine::CreateB2dIAObject(::sdr::overlay::OverlayManager& rOverlayManager)
{
    if(pHdl1 && pHdl2)
    {
        ::sdr::overlay::OverlayObject* pNewOverlayObject = new
            ::sdr::overlay::OverlayLineStriped(
                pHdl1->getPosition(),
                pHdl2->getPosition());

        pNewOverlayObject->setBaseColor(Color(COL_LIGHTRED));
        rOverlayManager.add(*pNewOverlayObject);
        maOverlayGroup.append(*pNewOverlayObject);
    }
}

Pointer SdrHdlLine::GetPointer() const
{
    return Pointer(POINTER_REFHAND);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

SdrHdlBezWgt::~SdrHdlBezWgt()
{
}

void SdrHdlBezWgt::CreateB2dIAObject(::sdr::overlay::OverlayManager& rOverlayManager)
{
    // call parent, create control point handle
    SdrHdl::CreateB2dIAObject(rOverlayManager);

    // create striped line part
    if(pHdl1 && !pHdl1->getPosition().equal(maPosition))
    {
        ::sdr::overlay::OverlayObject* pNewOverlayObject = new
            ::sdr::overlay::OverlayLineStriped(
                pHdl1->getPosition(),
                maPosition
            );

        // line part is not hittable
        pNewOverlayObject->setHittable(false);

        // color(?)
        pNewOverlayObject->setBaseColor(Color(COL_LIGHTBLUE));

        rOverlayManager.add(*pNewOverlayObject);
        maOverlayGroup.append(*pNewOverlayObject);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

E3dVolumeMarker::E3dVolumeMarker(
    SdrHdlList& rHdlList,
    const SdrObject* pSdrHdlObject,
    const basegfx::B2DPolyPolygon& rWireframePoly)
:   SdrHdl(rHdlList, pSdrHdlObject),
    aWireframePoly(rWireframePoly)
{
}

E3dVolumeMarker::~E3dVolumeMarker()
{
}

void E3dVolumeMarker::CreateB2dIAObject(::sdr::overlay::OverlayManager& rOverlayManager)
{
    ::sdr::overlay::OverlayObject* pNewOverlayObject = new
        ::sdr::overlay::OverlayPolyPolygonStripedAndFilled(aWireframePoly);

    pNewOverlayObject->setBaseColor(Color(COL_BLACK));
    rOverlayManager.add(*pNewOverlayObject);
    maOverlayGroup.append(*pNewOverlayObject);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

ImpEdgeHdl::~ImpEdgeHdl()
{
}

void ImpEdgeHdl::CreateB2dIAObject(::sdr::overlay::OverlayManager& rOverlayManager)
{
    const SdrEdgeObj* pEdge = dynamic_cast< const SdrEdgeObj* >(mpSdrHdlObject);

    if(pEdge && mnObjHdlNum <= 1)
    {
        BitmapColorIndex eColIndex = LightCyan;
        BitmapMarkerKind eKindOfMarker = Rect_7x7;

        if(pEdge->GetSdrObjectConnection(mnObjHdlNum == 0))
        {
            eColIndex = LightRed;
        }

        if(mnPPntNum < 2)
        {
            // Handle with plus sign inside
            eKindOfMarker = Circ_7x7;
        }

        ::sdr::overlay::OverlayObject* pNewOverlayObject = CreateOverlayObject(
            maPosition,
            eColIndex,
            eKindOfMarker,
            0, 0);

        rOverlayManager.add(*pNewOverlayObject);
        maOverlayGroup.append(*pNewOverlayObject);
    }
    else
    {
        // call parent
        SdrHdl::CreateB2dIAObject(rOverlayManager);
    }
}

void ImpEdgeHdl::SetLineCode(SdrEdgeLineCode eCode)
{
    if(eLineCode != eCode)
    {
        // remember new value
        eLineCode = eCode;

        // create new display
        Touch();
    }
}

Pointer ImpEdgeHdl::GetPointer() const
{
    if(!mpSdrHdlObject)
    {
        return SdrHdl::GetPointer();
    }

    const SdrEdgeObj* pSdrEdgeObj = dynamic_cast< const SdrEdgeObj* >(mpSdrHdlObject);

    if(!pSdrEdgeObj)
    {
        return SdrHdl::GetPointer();
    }

    if(mnObjHdlNum <= 1)
    {
        return Pointer(POINTER_MOVEPOINT); //Pointer(POINTER_DRAW_CONNECT);
    }

    if(IsHorzDrag())
    {
        return Pointer(POINTER_ESIZE);
    }
    else
    {
        return Pointer(POINTER_SSIZE);
    }
}

bool ImpEdgeHdl::IsHorzDrag() const
{
    const SdrEdgeObj* pEdge = dynamic_cast< const SdrEdgeObj* >(mpSdrHdlObject);

    if(pEdge)
    {
        if(mnObjHdlNum <= 1)
        {
            return false;
        }

        return pEdge->checkHorizontalDrag(eLineCode, 2 == mnObjHdlNum);
    }

    return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

ImpMeasureHdl::~ImpMeasureHdl()
{
}

void ImpMeasureHdl::CreateB2dIAObject(::sdr::overlay::OverlayManager& rOverlayManager)
{
    BitmapColorIndex eColIndex = LightCyan;
    BitmapMarkerKind eKindOfMarker = Rect_9x9;

    if(mnObjHdlNum > 1)
    {
        eKindOfMarker = Rect_7x7;
    }

    if(mbSelect)
    {
        eColIndex = Cyan;
    }

    ::sdr::overlay::OverlayObject* pNewOverlayObject = CreateOverlayObject(
        maPosition,
        eColIndex,
        eKindOfMarker,
        0, 0);

    rOverlayManager.add(*pNewOverlayObject);
    maOverlayGroup.append(*pNewOverlayObject);
}

Pointer ImpMeasureHdl::GetPointer() const
{
    switch (mnObjHdlNum)
    {
        case 0: case 1: return Pointer(POINTER_HAND);
        case 2: case 3: return Pointer(POINTER_MOVEPOINT);
        case 4: case 5: return SdrHdl::GetPointer(); // wird dann entsprechend gedreht
    } // switch
    return Pointer(POINTER_NOTALLOWED);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

ImpTextframeHdl::ImpTextframeHdl(
    SdrHdlList& rHdlList,
    const SdrObject& rSdrHdlObject,
    const basegfx::B2DHomMatrix& rTransformation)
:   SdrHdl(rHdlList, &rSdrHdlObject),
    maTransformation(rTransformation)
{
    // set member aPos, not sure if this is needed
    setPosition(maTransformation * basegfx::B2DPoint(0.0, 0.0));
}

ImpTextframeHdl::~ImpTextframeHdl()
{
}

void ImpTextframeHdl::CreateB2dIAObject(::sdr::overlay::OverlayManager& rOverlayManager)
{
    const SvtOptionsDrawinglayer aSvtOptionsDrawinglayer;
    const Color aHilightColor(aSvtOptionsDrawinglayer.getHilightColor());
    const double fTransparence(aSvtOptionsDrawinglayer.GetTransparentSelectionPercent() * 0.01);
    ::sdr::overlay::OverlayRectangle* pNewOverlayObject = new ::sdr::overlay::OverlayRectangle(
        maTransformation,
        aHilightColor,
        fTransparence,
        3.0,
        3.0,
        500,
        true); // allow animation; the Handle is not shown at text edit time

    pNewOverlayObject->setHittable(false);
    rOverlayManager.add(*pNewOverlayObject);
    maOverlayGroup.append(*pNewOverlayObject);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// SdrCropHdl

SdrCropHdl::SdrCropHdl(
    SdrHdlList& rHdlList,
    const SdrObject& rSdrHdlObject,
    SdrHdlKind eNewKind,
    const basegfx::B2DPoint& rPnt,
    double fShearX,
    double fRotation)
:   SdrHdl( rHdlList, &rSdrHdlObject, eNewKind, rPnt),
    mfShearX(fShearX),
    mfRotation(fRotation)
{
}

SdrCropHdl::~SdrCropHdl()
{
}

BitmapEx SdrCropHdl::GetHandlesBitmap( bool bIsFineHdl, bool bIsHighContrast )
{
    if( bIsHighContrast )
    {
        static BitmapEx* pHighContrastBitmap = 0;
        if( pHighContrastBitmap == 0 )
            pHighContrastBitmap = new BitmapEx(ResId(SIP_SA_ACCESSIBILITY_CROP_MARKERS, *ImpGetResMgr()));
        return *pHighContrastBitmap;
    }
    else if( bIsFineHdl )
    {
        static BitmapEx* pModernBitmap = 0;
        if( pModernBitmap == 0 )
            pModernBitmap = new BitmapEx(ResId(SIP_SA_CROP_FINE_MARKERS, *ImpGetResMgr()));
        return *pModernBitmap;
    }
    else
    {
        static BitmapEx* pSimpleBitmap = 0;
        if( pSimpleBitmap == 0 )
            pSimpleBitmap = new BitmapEx(ResId(SIP_SA_CROP_MARKERS, *ImpGetResMgr()));
        return *pSimpleBitmap;
    }
}

BitmapEx SdrCropHdl::GetBitmapForHandle( const BitmapEx& rBitmap, sal_uInt16 nSize )
{
    sal_uInt16 nPixelSize = 0, nX = 0, nY = 0, nOffset = 0;

    if( nSize <= 3 )
    {
        nPixelSize = 13;
        nOffset = 0;
    }
    else if( nSize <=4 )
    {
        nPixelSize = 17;
        nOffset = 36;
    }
    else
    {
        nPixelSize = 21;
        nOffset = 84;
    }

    switch( meKind )
    {
        case HDL_UPLFT: nX = 0; nY = 0; break;
        case HDL_UPPER: nX = 1; nY = 0; break;
        case HDL_UPRGT: nX = 2; nY = 0; break;
        case HDL_LEFT:  nX = 0; nY = 1; break;
        case HDL_RIGHT: nX = 2; nY = 1; break;
        case HDL_LWLFT: nX = 0; nY = 2; break;
        case HDL_LOWER: nX = 1; nY = 2; break;
        case HDL_LWRGT: nX = 2; nY = 2; break;
        default: break;
    }

    Rectangle aSourceRect( Point( nX * (nPixelSize-1) + nOffset,  nY * (nPixelSize-1)), Size(nPixelSize, nPixelSize) );

    BitmapEx aRetval(rBitmap);
    aRetval.Crop(aSourceRect);
    return aRetval;
}

void SdrCropHdl::CreateB2dIAObject(::sdr::overlay::OverlayManager& rOverlayManager)
{
    bool bIsFineHdl(mrHdlList.IsFineHdl());
    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
    bool bIsHighContrast(rStyleSettings.GetHighContrastMode());
    sal_uInt16 nHdlSize = mrHdlList.GetHdlSize();

    if( bIsHighContrast )
        nHdlSize = 4;

    const BitmapEx aHandlesBitmap( GetHandlesBitmap( bIsFineHdl, bIsHighContrast ) );
    BitmapEx aBmpEx1( GetBitmapForHandle( aHandlesBitmap, nHdlSize ) );
    ::sdr::overlay::OverlayObject* pOverlayObject = 0L;

    // animate focused handles
    if(IsFocusHdl() && (mrHdlList.GetFocusHdl() == this))
    {
        if( nHdlSize >= 2 )
            nHdlSize = 1;

        BitmapEx aBmpEx2( GetBitmapForHandle( aHandlesBitmap, nHdlSize + 1 ) );

        const sal_uInt32 nBlinkTime = sal::static_int_cast<sal_uInt32>(rStyleSettings.GetCursorBlinkTime());

        pOverlayObject = new ::sdr::overlay::OverlayAnimatedBitmapEx(
            maPosition,
            aBmpEx1,
            aBmpEx2,
            nBlinkTime,
            (sal_uInt16)(aBmpEx1.GetSizePixel().Width() - 1) >> 1,
            (sal_uInt16)(aBmpEx1.GetSizePixel().Height() - 1) >> 1,
            (sal_uInt16)(aBmpEx2.GetSizePixel().Width() - 1) >> 1,
            (sal_uInt16)(aBmpEx2.GetSizePixel().Height() - 1) >> 1,
            mfShearX,
            mfRotation);
    }
    else
    {
        // create centered handle as default
        pOverlayObject = new ::sdr::overlay::OverlayBitmapEx(
            maPosition,
            aBmpEx1,
            (sal_uInt16)(aBmpEx1.GetSizePixel().Width() - 1) >> 1,
            (sal_uInt16)(aBmpEx1.GetSizePixel().Height() - 1) >> 1,
            0.0,
            mfShearX,
            mfRotation);
    }

    rOverlayManager.add(*pOverlayObject);
    maOverlayGroup.append(*pOverlayObject);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// with the correction of crop handling I could get rid of the extra mirroring flag, adapted stuff
// accordingly

SdrCropViewHdl::SdrCropViewHdl(
    SdrHdlList& rHdlList,
    const SdrObject& rSdrHdlObject,
    const basegfx::B2DHomMatrix& rObjectTransform,
    const Graphic& rGraphic,
    double fCropLeft,
    double fCropTop,
    double fCropRight,
    double fCropBottom)
:   SdrHdl(rHdlList, &rSdrHdlObject, HDL_USER),
    maObjectTransform(rObjectTransform),
    maGraphic(rGraphic),
    mfCropLeft(fCropLeft),
    mfCropTop(fCropTop),
    mfCropRight(fCropRight),
    mfCropBottom(fCropBottom)
{
}

void SdrCropViewHdl::CreateB2dIAObject(::sdr::overlay::OverlayManager& rOverlayManager)
{
    GetRidOfIAObject();

    // decompose to have current translate and scale
    basegfx::B2DVector aScale, aTranslate;
    double fRotate, fShearX;

    maObjectTransform.decompose(aScale, aTranslate, fRotate, fShearX);

    if(aScale.equalZero())
    {
        return;
    }

    // detect 180 degree rotation, this is the same as mirrored in X and Y,
    // thus change to mirroring. Prefer mirroring here. Use the equal call
    // with getSmallValue here, the original which uses rtl::math::approxEqual
    // is too correct here. Maybe this changes with enhanced precision in aw080
    // to the better so that this can be reduced to the more precise call again
    if(basegfx::fTools::equal(fabs(fRotate), F_PI, 0.000000001))
    {
        aScale.setX(aScale.getX() * -1.0);
        aScale.setY(aScale.getY() * -1.0);
        fRotate = 0.0;
    }

    // remember mirroring, reset at Scale and adapt crop values for usage;
    // mirroring can stay in the object transformation, so do not have to
    // cope with it here (except later for the CroppedImage transformation,
    // see below)
    const bool bMirroredX(aScale.getX() < 0.0);
    const bool bMirroredY(aScale.getY() < 0.0);
    double fCropLeft(mfCropLeft);
    double fCropTop(mfCropTop);
    double fCropRight(mfCropRight);
    double fCropBottom(mfCropBottom);

    if(bMirroredX)
    {
        aScale.setX(-aScale.getX());
    }

    if(bMirroredY)
    {
        aScale.setY(-aScale.getY());
    }

    // create target translate and scale
    const basegfx::B2DVector aTargetScale(
        aScale.getX() + fCropRight + fCropLeft,
        aScale.getY() + fCropBottom + fCropTop);
    const basegfx::B2DVector aTargetTranslate(
        aTranslate.getX() - fCropLeft,
        aTranslate.getY() - fCropTop);

    // create ranges to make comparisons
    const basegfx::B2DRange aCurrentForCompare(
        aTranslate.getX(), aTranslate.getY(),
        aTranslate.getX() + aScale.getX(), aTranslate.getY() + aScale.getY());
    basegfx::B2DRange aCropped(
        aTargetTranslate.getX(), aTargetTranslate.getY(),
        aTargetTranslate.getX() + aTargetScale.getX(), aTargetTranslate.getY() + aTargetScale.getY());

    if(aCropped.isEmpty())
    {
        // nothing to return since cropped content is completely empty
        return;
    }

    if(aCurrentForCompare.equal(aCropped))
    {
        // no crop at all
        return;
    }

    // back-transform to have values in unit coordinates
    basegfx::B2DHomMatrix aBackToUnit;
    aBackToUnit.translate(-aTranslate.getX(), -aTranslate.getY());
    aBackToUnit.scale(
        basegfx::fTools::equalZero(aScale.getX()) ? 1.0 : 1.0 / aScale.getX(),
        basegfx::fTools::equalZero(aScale.getY()) ? 1.0 : 1.0 / aScale.getY());

    // transform cropped back to unit coordinates
    aCropped.transform(aBackToUnit);

    // prepare crop PolyPolygon
    basegfx::B2DPolygon aGraphicOutlinePolygon(
        basegfx::tools::createPolygonFromRect(
            aCropped));
    basegfx::B2DPolyPolygon aCropPolyPolygon(aGraphicOutlinePolygon);

    // current range is unit range
    basegfx::B2DRange aOverlap(0.0, 0.0, 1.0, 1.0);

    aOverlap.intersect(aCropped);

    if(!aOverlap.isEmpty())
    {
        aCropPolyPolygon.append(
            basegfx::tools::createPolygonFromRect(
                aOverlap));
    }

    // transform to object coordinates to prepare for clip
    aCropPolyPolygon.transform(maObjectTransform);
    aGraphicOutlinePolygon.transform(maObjectTransform);

    // create cropped transformation
    basegfx::B2DHomMatrix aCroppedTransform;

    aCroppedTransform.scale(
        aCropped.getWidth(),
        aCropped.getHeight());
    aCroppedTransform.translate(
        aCropped.getMinX(),
        aCropped.getMinY());
    aCroppedTransform = maObjectTransform * aCroppedTransform;

    // prepare graphic primitive (tranformed)
    const drawinglayer::primitive2d::Primitive2DReference aGraphic(
        new drawinglayer::primitive2d::GraphicPrimitive2D(
            aCroppedTransform,
            maGraphic));

    // prepare outline polygon for whole graphic
    const SvtOptionsDrawinglayer aSvtOptionsDrawinglayer;
    const basegfx::BColor aHilightColor(aSvtOptionsDrawinglayer.getHilightColor().getBColor());
    const drawinglayer::primitive2d::Primitive2DReference aGraphicOutline(
        new drawinglayer::primitive2d::PolygonHairlinePrimitive2D(
        aGraphicOutlinePolygon,
        aHilightColor));

    // combine these
    drawinglayer::primitive2d::Primitive2DSequence aCombination(2);
    aCombination[0] = aGraphic;
    aCombination[1] = aGraphicOutline;

    // embed to MaskPrimitive2D
    const drawinglayer::primitive2d::Primitive2DReference aMaskedGraphic(
        new drawinglayer::primitive2d::MaskPrimitive2D(
            aCropPolyPolygon,
            aCombination));

    // embed to UnifiedTransparencePrimitive2D
    const drawinglayer::primitive2d::Primitive2DReference aTransparenceMaskedGraphic(
        new drawinglayer::primitive2d::UnifiedTransparencePrimitive2D(
            drawinglayer::primitive2d::Primitive2DSequence(&aMaskedGraphic, 1),
            0.8));

    const drawinglayer::primitive2d::Primitive2DSequence aSequence(&aTransparenceMaskedGraphic, 1);
    ::sdr::overlay::OverlayObject* pOverlayObject = new sdr::overlay::OverlayPrimitive2DSequenceObject(aSequence);
    DBG_ASSERT(pOverlayObject, "Got NO new IAO!");

    if(pOverlayObject)
    {
        // only informative object, no hit
        pOverlayObject->setHittable(false);

        rOverlayManager.add(*pOverlayObject);
        maOverlayGroup.append(*pOverlayObject);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// eof
