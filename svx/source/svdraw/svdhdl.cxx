/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


#include <algorithm>
#include <cassert>

#include <svx/svdhdl.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svdetc.hxx>
#include <svx/svdmrkv.hxx>
#include <vcl/window.hxx>
#include <vcl/settings.hxx>
#include <vcl/virdev.hxx>
#include <tools/poly.hxx>
#include <vcl/bmpacc.hxx>

#include <svx/sxekitm.hxx>
#include "svx/svdstr.hrc"
#include "svdglob.hxx"

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
#include <sdr/overlay/overlayline.hxx>
#include <svx/sdr/overlay/overlaytriangle.hxx>
#include <sdr/overlay/overlayhandle.hxx>
#include <sdr/overlay/overlayrectangle.hxx>
#include <svx/sdrpagewindow.hxx>
#include <svx/sdrpaintwindow.hxx>
#include <vcl/svapp.hxx>
#include <svx/sdr/overlay/overlaypolypolygon.hxx>
#include <vcl/lazydelete.hxx>

#include <basegfx/polygon/b2dpolygontools.hxx>
#include <drawinglayer/primitive2d/polypolygonprimitive2d.hxx>
#include <svx/sdr/overlay/overlayprimitive2dsequenceobject.hxx>
#include <drawinglayer/primitive2d/graphicprimitive2d.hxx>
#include <drawinglayer/primitive2d/maskprimitive2d.hxx>
#include <drawinglayer/primitive2d/unifiedtransparenceprimitive2d.hxx>
#include <drawinglayer/primitive2d/polygonprimitive2d.hxx>
#include <memory>


// #i15222#
// Due to the resource problems in Win95/98 with bitmap resources I
// will change this handle bitmap providing class. Old version was splitting
// and preparing all small handle bitmaps in device bitmap format, now this will
// be done on the fly. Thus, there is only one big bitmap in memory. With
// three source bitmaps, this will be 3 system bitmap resources instead of hundreds.
// The price for that needs to be evaluated. Maybe we will need another change here
// if this is too expensive.
class SdrHdlBitmapSet
{
    // the bitmap holding all information
    BitmapEx                    maMarkersBitmap;

    // the cropped Bitmaps for reusage
    ::std::vector< BitmapEx >   maRealMarkers;

    // helpers
    BitmapEx& impGetOrCreateTargetBitmap(sal_uInt16 nIndex, const Rectangle& rRectangle);

public:
    explicit SdrHdlBitmapSet(sal_uInt16 nResId);
    ~SdrHdlBitmapSet();

    const BitmapEx& GetBitmapEx(BitmapMarkerKind eKindOfMarker, sal_uInt16 nInd=0);
};


#define KIND_COUNT          (14)
#define INDEX_COUNT         (6)
#define INDIVIDUAL_COUNT    (5)

SdrHdlBitmapSet::SdrHdlBitmapSet(sal_uInt16 nResId)
    :   maMarkersBitmap(ResId(nResId, *ImpGetResMgr())),
        // 15 kinds (BitmapMarkerKind) use index [0..5] + 5 extra
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

// change getting of bitmap to use the big resource bitmap
const BitmapEx& SdrHdlBitmapSet::GetBitmapEx(BitmapMarkerKind eKindOfMarker, sal_uInt16 nInd)
{
    // fill in size and source position in maMarkersBitmap
    const sal_uInt16 nYPos(nInd * 11);

    switch(eKindOfMarker)
    {
        default:
        {
            OSL_FAIL( "Unknown kind of marker." );
            // no break here, return Rect_9x9 as default
        }
        case Rect_9x9:
        {
            return impGetOrCreateTargetBitmap((1 * INDEX_COUNT) + nInd, Rectangle(Point(7, nYPos), Size(9, 9)));
        }

        case Rect_7x7:
        {
            return impGetOrCreateTargetBitmap((0 * INDEX_COUNT) + nInd, Rectangle(Point(0, nYPos), Size(7, 7)));
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
            return impGetOrCreateTargetBitmap((KIND_COUNT * INDEX_COUNT) + 1, Rectangle(Point(15, 76), Size(9, 9)));
        }

        case Glue_Deselected:
        {
            return impGetOrCreateTargetBitmap((KIND_COUNT * INDEX_COUNT) + 2, Rectangle(Point(15, 67), Size(9, 9)));
        }

        case Anchor: // AnchorTR for SW
        case AnchorTR:
        {
            return impGetOrCreateTargetBitmap((KIND_COUNT * INDEX_COUNT) + 3, Rectangle(Point(24, 67), Size(24, 24)));
        }

        // add AnchorPressed to be able to animate anchor control
        case AnchorPressed:
        case AnchorPressedTR:
        {
            return impGetOrCreateTargetBitmap((KIND_COUNT * INDEX_COUNT) + 4, Rectangle(Point(48, 67), Size(24, 24)));
        }
    }
}



SdrHdl::SdrHdl():
    pObj(nullptr),
    pPV(nullptr),
    pHdlList(nullptr),
    eKind(HDL_MOVE),
    nRotationAngle(0),
    nObjHdlNum(0),
    nPolyNum(0),
    nPPntNum(0),
    nSourceHdlNum(0),
    bSelect(false),
    b1PixMore(false),
    bPlusHdl(false),
    mbMoveOutside(false),
    mbMouseOver(false)
{
}

SdrHdl::SdrHdl(const Point& rPnt, SdrHdlKind eNewKind):
    pObj(nullptr),
    pPV(nullptr),
    pHdlList(nullptr),
    aPos(rPnt),
    eKind(eNewKind),
    nRotationAngle(0),
    nObjHdlNum(0),
    nPolyNum(0),
    nPPntNum(0),
    nSourceHdlNum(0),
    bSelect(false),
    b1PixMore(false),
    bPlusHdl(false),
    mbMoveOutside(false),
    mbMouseOver(false)
{
}

SdrHdl::~SdrHdl()
{
    GetRidOfIAObject();
}

void SdrHdl::Set1PixMore(bool bJa)
{
    if(b1PixMore != bJa)
    {
        b1PixMore = bJa;

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

void SdrHdl::SetRotationAngle(long n)
{
    if(nRotationAngle != n)
    {
        nRotationAngle = n;

        // create new display
        Touch();
    }
}

void SdrHdl::SetPos(const Point& rPnt)
{
    if(aPos != rPnt)
    {
        // remember new position
        aPos = rPnt;

        // create new display
        Touch();
    }
}

void SdrHdl::SetSelected(bool bJa)
{
    if(bSelect != bJa)
    {
        // remember new value
        bSelect = bJa;

        // create new display
        Touch();
    }
}

void SdrHdl::SetHdlList(SdrHdlList* pList)
{
    if(pHdlList != pList)
    {
        // remember list
        pHdlList = pList;

        // now its possible to create graphic representation
        Touch();
    }
}

void SdrHdl::SetObj(SdrObject* pNewObj)
{
    if(pObj != pNewObj)
    {
        // remember new object
        pObj = pNewObj;

        // graphic representation may have changed
        Touch();
    }
}

void SdrHdl::Touch()
{
    // force update of graphic representation
    CreateB2dIAObject();
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

    if(pHdlList && pHdlList->GetView() && !pHdlList->GetView()->areMarkHandlesHidden())
    {
        BitmapColorIndex eColIndex = LightGreen;
        BitmapMarkerKind eKindOfMarker = Rect_7x7;

        bool bRot = pHdlList->IsRotateShear();
        if(pObj)
            eColIndex = (bSelect) ? Cyan : LightCyan;
        if(bRot)
        {
            // red rotation handles
            if(pObj && bSelect)
                eColIndex = Red;
            else
                eColIndex = LightRed;
        }

        switch(eKind)
        {
            case HDL_MOVE:
            {
                eKindOfMarker = (b1PixMore) ? Rect_9x9 : Rect_7x7;
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
                    eKindOfMarker = (b1PixMore) ? Circ_9x9 : Circ_7x7;
                }
                else
                {
                    eKindOfMarker = (b1PixMore) ? Rect_9x9 : Rect_7x7;
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
            case HDL_GLUE_DESELECTED:
            {
                eKindOfMarker = Glue_Deselected;
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
            // top right anchor for SW
            case HDL_ANCHOR_TR:
            {
                eKindOfMarker = AnchorTR;
                break;
            }

            // for SJ and the CustomShapeHandles:
            case HDL_CUSTOMSHAPE1:
            {
                eKindOfMarker = (b1PixMore) ? Customshape_9x9 : Customshape_7x7;
                eColIndex = Yellow;
                break;
            }
            default:
                break;
        }

        SdrMarkView* pView = pHdlList->GetView();
        SdrPageView* pPageView = pView->GetSdrPageView();

        if(pPageView)
        {
            for(sal_uInt32 b(0L); b < pPageView->PageWindowCount(); b++)
            {
                // const SdrPageViewWinRec& rPageViewWinRec = rPageViewWinList[b];
                const SdrPageWindow& rPageWindow = *pPageView->GetPageWindow(b);

                if(rPageWindow.GetPaintWindow().OutputToWindow())
                {
                    Point aMoveOutsideOffset(0, 0);
                    OutputDevice& rOutDev = rPageWindow.GetPaintWindow().GetOutputDevice();

                    // add offset if necessary
                    if(pHdlList->IsMoveOutside() || mbMoveOutside)
                    {
                        Size aOffset = rOutDev.PixelToLogic(Size(4, 4));

                        if(eKind == HDL_UPLFT || eKind == HDL_UPPER || eKind == HDL_UPRGT)
                            aMoveOutsideOffset.Y() -= aOffset.Width();
                        if(eKind == HDL_LWLFT || eKind == HDL_LOWER || eKind == HDL_LWRGT)
                            aMoveOutsideOffset.Y() += aOffset.Height();
                        if(eKind == HDL_UPLFT || eKind == HDL_LEFT  || eKind == HDL_LWLFT)
                            aMoveOutsideOffset.X() -= aOffset.Width();
                        if(eKind == HDL_UPRGT || eKind == HDL_RIGHT || eKind == HDL_LWRGT)
                            aMoveOutsideOffset.X() += aOffset.Height();
                    }

                    rtl::Reference< sdr::overlay::OverlayManager > xManager = rPageWindow.GetOverlayManager();
                    if (xManager.is())
                    {
                        basegfx::B2DPoint aPosition(aPos.X(), aPos.Y());
                        sdr::overlay::OverlayObject* pNewOverlayObject = nullptr;
                        if (getenv ("SVX_DRAW_HANDLES") && (eKindOfMarker == Rect_7x7 || eKindOfMarker == Rect_9x9 || eKindOfMarker == Rect_11x11))
                        {
                            double fSize = 7.0;
                            switch (eKindOfMarker)
                            {
                                case Rect_9x9:
                                    fSize = 9.0;
                                    break;
                                case Rect_11x11:
                                    fSize = 11.0;
                                    break;
                                default:
                                    break;
                            }
                            sal_Int32 nScaleFactor = rOutDev.GetDPIScaleFactor();
                            basegfx::B2DSize aB2DSize(fSize * nScaleFactor, fSize * nScaleFactor);

                            Color aHandleStrokeColor(COL_BLACK);
                            Color aHandleFillColor(COL_LIGHTGREEN);
                            switch (eColIndex)
                            {
                                case Cyan:
                                    aHandleFillColor = Color(COL_CYAN);
                                    break;
                                case LightCyan:
                                    aHandleFillColor = Color(COL_LIGHTCYAN);
                                    break;
                                case Red:
                                    aHandleFillColor = Color(COL_RED);
                                    break;
                                case LightRed:
                                    aHandleFillColor = Color(COL_LIGHTRED);
                                    break;
                                case Yellow:
                                    aHandleFillColor = Color(COL_YELLOW);
                                    break;
                                default:
                                    break;
                            }
                            pNewOverlayObject = new sdr::overlay::OverlayHandle(aPosition, aB2DSize, aHandleStrokeColor, aHandleFillColor);
                        }
                        else
                        {
                            pNewOverlayObject = CreateOverlayObject(
                                                    aPosition, eColIndex, eKindOfMarker,
                                                    rOutDev, aMoveOutsideOffset);
                        }
                        // OVERLAYMANAGER
                        if (pNewOverlayObject)
                        {
                            xManager->add(*pNewOverlayObject);
                            maOverlayGroup.append(*pNewOverlayObject);
                        }
                    }
                }
            }
        }
    }
}

BitmapMarkerKind SdrHdl::GetNextBigger(BitmapMarkerKind eKnd)
{
    BitmapMarkerKind eRetval(eKnd);

    switch(eKnd)
    {
        case Rect_7x7:          eRetval = Rect_9x9;         break;
        case Rect_9x9:          eRetval = Rect_11x11;       break;
        case Rect_11x11:        eRetval = Rect_13x13;       break;

        case Circ_7x7:          eRetval = Circ_9x9;         break;
        case Circ_9x9:          eRetval = Circ_11x11;       break;

        case Customshape_7x7:       eRetval = Customshape_9x9;      break;
        case Customshape_9x9:       eRetval = Customshape_11x11;    break;
        //case Customshape_11x11:   eRetval = ; break;

        case Elli_7x9:          eRetval = Elli_9x11;        break;

        case Elli_9x7:          eRetval = Elli_11x9;        break;

        case RectPlus_7x7:      eRetval = RectPlus_9x9;     break;
        case RectPlus_9x9:      eRetval = RectPlus_11x11;   break;

        // let anchor blink with its pressed state
        case Anchor:            eRetval = AnchorPressed;    break;

        // same for AnchorTR
        case AnchorTR:          eRetval = AnchorPressedTR;  break;
        default:
            break;
    }

    return eRetval;
}

BitmapEx SdrHdl::ImpGetBitmapEx( BitmapMarkerKind eKindOfMarker, sal_uInt16 nInd)
{
    static vcl::DeleteOnDeinit< SdrHdlBitmapSet > aModernSet(new SdrHdlBitmapSet(SIP_SA_MARKERS));
    return aModernSet.get()->GetBitmapEx(eKindOfMarker, nInd);
}

sdr::overlay::OverlayObject* SdrHdl::CreateOverlayObject(
    const basegfx::B2DPoint& rPos,
    BitmapColorIndex eColIndex, BitmapMarkerKind eKindOfMarker, OutputDevice& rOutDev, Point aMoveOutsideOffset)
{
    sdr::overlay::OverlayObject* pRetval = nullptr;

    // support bigger sizes
    bool bForceBiggerSize(false);

    if(pHdlList->GetHdlSize() > 3)
    {
        switch(eKindOfMarker)
        {
            case Anchor:
            case AnchorPressed:
            case AnchorTR:
            case AnchorPressedTR:
            {
                // #i121463# For anchor, do not simply make bigger because of HdlSize,
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

    if(bForceBiggerSize)
    {
        eKindOfMarker = GetNextBigger(eKindOfMarker);
    }

    // This handle has the focus, visualize it
    if(IsFocusHdl() && pHdlList && pHdlList->GetFocusHdl() == this)
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
                case Glue_Deselected:
                    eNextBigger = Glue;
                    break;
                default:
                    break;
            }
        }

        // create animated handle
        BitmapEx aBmpEx1 = ImpGetBitmapEx( eKindOfMarker, (sal_uInt16)eColIndex );
        BitmapEx aBmpEx2 = ImpGetBitmapEx( eNextBigger,   (sal_uInt16)eColIndex );

        // #i53216# Use system cursor blink time. Use the unsigned value.
        const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
        const sal_uInt64 nBlinkTime(rStyleSettings.GetCursorBlinkTime());

        if(eKindOfMarker == Anchor || eKindOfMarker == AnchorPressed)
        {
            // when anchor is used take upper left as reference point inside the handle
            pRetval = new sdr::overlay::OverlayAnimatedBitmapEx(rPos, aBmpEx1, aBmpEx2, nBlinkTime);
        }
        else if(eKindOfMarker == AnchorTR || eKindOfMarker == AnchorPressedTR)
        {
            // AnchorTR for SW, take top right as (0,0)
            pRetval = new sdr::overlay::OverlayAnimatedBitmapEx(rPos, aBmpEx1, aBmpEx2, nBlinkTime,
                (sal_uInt16)(aBmpEx1.GetSizePixel().Width() - 1), 0,
                (sal_uInt16)(aBmpEx2.GetSizePixel().Width() - 1), 0);
        }
        else
        {
            // create centered handle as default
            pRetval = new sdr::overlay::OverlayAnimatedBitmapEx(rPos, aBmpEx1, aBmpEx2, nBlinkTime,
                (sal_uInt16)(aBmpEx1.GetSizePixel().Width() - 1) >> 1,
                (sal_uInt16)(aBmpEx1.GetSizePixel().Height() - 1) >> 1,
                (sal_uInt16)(aBmpEx2.GetSizePixel().Width() - 1) >> 1,
                (sal_uInt16)(aBmpEx2.GetSizePixel().Height() - 1) >> 1);
        }
    }
    else
    {
        // create normal handle: use ImpGetBitmapEx(...) now
        BitmapEx aBmpEx = ImpGetBitmapEx(eKindOfMarker, (sal_uInt16)eColIndex);

        // When the image with handles is not found, the bitmap returned is
        // empty. This is a problem when we use LibreOffice as a library
        // (through LOKit - for example on Android) even when we don't show
        // the handles, because the hit test would always return false.
        //
        // This HACK replaces the empty bitmap with a black 13x13 bitmap handle
        // so that the hit test works for this case.
        if (aBmpEx.IsEmpty())
        {
            aBmpEx = BitmapEx(Bitmap(Size(13, 13), 24));
            aBmpEx.Erase(COL_BLACK);
        }

        // Scale the handle with the DPI scale factor
        sal_Int32 nScaleFactor = rOutDev.GetDPIScaleFactor();
        aBmpEx.Scale(nScaleFactor, nScaleFactor);

        if(eKindOfMarker == Anchor || eKindOfMarker == AnchorPressed)
        {
            // upper left as reference point inside the handle for AnchorPressed, too
            pRetval = new sdr::overlay::OverlayBitmapEx(rPos, aBmpEx);
        }
        else if(eKindOfMarker == AnchorTR || eKindOfMarker == AnchorPressedTR)
        {
            // AnchorTR for SW, take top right as (0,0)
            pRetval = new sdr::overlay::OverlayBitmapEx(rPos, aBmpEx,
                (sal_uInt16)(aBmpEx.GetSizePixel().Width() - 1), 0);
        }
        else
        {
            sal_uInt16 nCenX((sal_uInt16)(aBmpEx.GetSizePixel().Width() - 1L) >> 1);
            sal_uInt16 nCenY((sal_uInt16)(aBmpEx.GetSizePixel().Height() - 1L) >> 1);

            if(aMoveOutsideOffset.X() > 0)
            {
                nCenX = 0;
            }
            else if(aMoveOutsideOffset.X() < 0)
            {
                nCenX = (sal_uInt16)(aBmpEx.GetSizePixel().Width() - 1);
            }

            if(aMoveOutsideOffset.Y() > 0)
            {
                nCenY = 0;
            }
            else if(aMoveOutsideOffset.Y() < 0)
            {
                nCenY = (sal_uInt16)(aBmpEx.GetSizePixel().Height() - 1);
            }

            // create centered handle as default
            pRetval = new sdr::overlay::OverlayBitmapEx(rPos, aBmpEx, nCenX, nCenY);
        }
    }

    return pRetval;
}

bool SdrHdl::IsHdlHit(const Point& rPnt) const
{
    // OVERLAYMANAGER
    basegfx::B2DPoint aPosition(rPnt.X(), rPnt.Y());
    return maOverlayGroup.isHitLogic(aPosition);
}

Pointer SdrHdl::GetPointer() const
{
    PointerStyle ePtr=PointerStyle::Move;
    const bool bSize=eKind>=HDL_UPLFT && eKind<=HDL_LWRGT;
    const bool bRot=pHdlList!=nullptr && pHdlList->IsRotateShear();
    const bool bDis=pHdlList!=nullptr && pHdlList->IsDistortShear();
    if (bSize && pHdlList!=nullptr && (bRot || bDis)) {
        switch (eKind) {
            case HDL_UPLFT: case HDL_UPRGT:
            case HDL_LWLFT: case HDL_LWRGT: ePtr=bRot ? PointerStyle::Rotate : PointerStyle::RefHand; break;
            case HDL_LEFT : case HDL_RIGHT: ePtr=PointerStyle::VShear; break;
            case HDL_UPPER: case HDL_LOWER: ePtr=PointerStyle::HShear; break;
            default:
                break;
        }
    } else {
        // When resizing rotated rectangles, rotate the mouse cursor slightly, too
        if (bSize && nRotationAngle!=0) {
            long nHdlAngle=0;
            switch (eKind) {
                case HDL_LWRGT: nHdlAngle=31500; break;
                case HDL_LOWER: nHdlAngle=27000; break;
                case HDL_LWLFT: nHdlAngle=22500; break;
                case HDL_LEFT : nHdlAngle=18000; break;
                case HDL_UPLFT: nHdlAngle=13500; break;
                case HDL_UPPER: nHdlAngle=9000;  break;
                case HDL_UPRGT: nHdlAngle=4500;  break;
                case HDL_RIGHT: nHdlAngle=0;     break;
                default:
                    break;
            }
            nHdlAngle+=nRotationAngle+2249; // a little bit more (for rounding)
            while (nHdlAngle<0) nHdlAngle+=36000;
            while (nHdlAngle>=36000) nHdlAngle-=36000;
            nHdlAngle/=4500;
            switch ((sal_uInt8)nHdlAngle) {
                case 0: ePtr=PointerStyle::ESize;  break;
                case 1: ePtr=PointerStyle::NESize; break;
                case 2: ePtr=PointerStyle::NSize;  break;
                case 3: ePtr=PointerStyle::NWSize; break;
                case 4: ePtr=PointerStyle::WSize;  break;
                case 5: ePtr=PointerStyle::SWSize; break;
                case 6: ePtr=PointerStyle::SSize;  break;
                case 7: ePtr=PointerStyle::SESize; break;
            } // switch
        } else {
            switch (eKind) {
                case HDL_UPLFT: ePtr=PointerStyle::NWSize;  break;
                case HDL_UPPER: ePtr=PointerStyle::NSize;     break;
                case HDL_UPRGT: ePtr=PointerStyle::NESize;  break;
                case HDL_LEFT : ePtr=PointerStyle::WSize;     break;
                case HDL_RIGHT: ePtr=PointerStyle::ESize;     break;
                case HDL_LWLFT: ePtr=PointerStyle::SWSize;  break;
                case HDL_LOWER: ePtr=PointerStyle::SSize;     break;
                case HDL_LWRGT: ePtr=PointerStyle::SESize;  break;
                case HDL_POLY : ePtr=PointerStyle::MovePoint; break;
                case HDL_CIRC : ePtr=PointerStyle::Hand;      break;
                case HDL_REF1 : ePtr=PointerStyle::RefHand;   break;
                case HDL_REF2 : ePtr=PointerStyle::RefHand;   break;
                case HDL_BWGT : ePtr=PointerStyle::MoveBezierWeight; break;
                case HDL_GLUE : ePtr=PointerStyle::MovePoint; break;
                case HDL_GLUE_DESELECTED : ePtr=PointerStyle::MovePoint; break;
                case HDL_CUSTOMSHAPE1 : ePtr=PointerStyle::Hand; break;
                default:
                    break;
            }
        }
    }
    return Pointer(ePtr);
}

bool SdrHdl::IsFocusHdl() const
{
    switch(eKind)
    {
        case HDL_UPLFT:
        case HDL_UPPER:
        case HDL_UPRGT:
        case HDL_LEFT:
        case HDL_RIGHT:
        case HDL_LWLFT:
        case HDL_LOWER:
        case HDL_LWRGT:
        {
            // if it's an activated TextEdit, it's moved to extended points
            if(pHdlList && pHdlList->IsMoveOutside())
                return false;
            else
                return true;
        }

        case HDL_MOVE:      // handle to move object
        case HDL_POLY:      // selected point of polygon or curve
        case HDL_BWGT:      // weight at a curve
        case HDL_CIRC:      // angle of circle segments, corner radius of rectangles
        case HDL_REF1:      // reference point 1, e. g. center of rotation
        case HDL_REF2:      // reference point 2, e. g. endpoint of reflection axis
        case HDL_GLUE:      // glue point
        case HDL_GLUE_DESELECTED:      // deselected glue point, used to be a little blue cross

        // for SJ and the CustomShapeHandles:
        case HDL_CUSTOMSHAPE1:

        case HDL_USER:
        {
            return true;
        }

        default:
        {
            return false;
        }
    }
}

void SdrHdl::onMouseEnter(const MouseEvent& /*rMEvt*/)
{
}

void SdrHdl::onMouseLeave()
{
}

SdrHdlColor::SdrHdlColor(const Point& rRef, Color aCol, const Size& rSize, bool bLum)
:   SdrHdl(rRef, HDL_COLR),
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

void SdrHdlColor::CreateB2dIAObject()
{
    // first throw away old one
    GetRidOfIAObject();

    if(pHdlList)
    {
        SdrMarkView* pView = pHdlList->GetView();

        if(pView && !pView->areMarkHandlesHidden())
        {
            SdrPageView* pPageView = pView->GetSdrPageView();

            if(pPageView)
            {
                for(sal_uInt32 b(0L); b < pPageView->PageWindowCount(); b++)
                {
                    const SdrPageWindow& rPageWindow = *pPageView->GetPageWindow(b);

                    if(rPageWindow.GetPaintWindow().OutputToWindow())
                    {
                        rtl::Reference< sdr::overlay::OverlayManager > xManager = rPageWindow.GetOverlayManager();
                        if (xManager.is())
                        {
                            Bitmap aBmpCol(CreateColorDropper(aMarkerColor));
                            basegfx::B2DPoint aPosition(aPos.X(), aPos.Y());
                            sdr::overlay::OverlayObject* pNewOverlayObject = new
                                sdr::overlay::OverlayBitmapEx(
                                    aPosition,
                                    BitmapEx(aBmpCol),
                                    (sal_uInt16)(aBmpCol.GetSizePixel().Width() - 1) >> 1,
                                    (sal_uInt16)(aBmpCol.GetSizePixel().Height() - 1) >> 1
                                );

                            // OVERLAYMANAGER
                            xManager->add(*pNewOverlayObject);
                            maOverlayGroup.append(*pNewOverlayObject);
                        }
                    }
                }
            }
        }
    }
}

Bitmap SdrHdlColor::CreateColorDropper(Color aCol)
{
    // get the Bitmap
    Bitmap aRetval(aMarkerSize, 24);
    aRetval.Erase(aCol);

    // get write access
    std::unique_ptr<BitmapWriteAccess> pWrite(aRetval.AcquireWriteAccess());
    DBG_ASSERT(pWrite, "Got NO write access to a new Bitmap!");

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

SdrHdlGradient::SdrHdlGradient(const Point& rRef1, const Point& rRef2, bool bGrad)
    : SdrHdl(rRef1, bGrad ? HDL_GRAD : HDL_TRNS)
    , pColHdl1(nullptr)
    , pColHdl2(nullptr)
    , a2ndPos(rRef2)
    , bGradient(bGrad)
    , bMoveSingleHandle(false)
    , bMoveFirstHandle(false)
{
}

SdrHdlGradient::~SdrHdlGradient()
{
}

void SdrHdlGradient::Set2ndPos(const Point& rPnt)
{
    if(a2ndPos != rPnt)
    {
        // remember new position
        a2ndPos = rPnt;

        // create new display
        Touch();
    }
}

void SdrHdlGradient::CreateB2dIAObject()
{
    // first throw away old one
    GetRidOfIAObject();

    if(pHdlList)
    {
        SdrMarkView* pView = pHdlList->GetView();

        if(pView && !pView->areMarkHandlesHidden())
        {
            SdrPageView* pPageView = pView->GetSdrPageView();

            if(pPageView)
            {
                for(sal_uInt32 b(0L); b < pPageView->PageWindowCount(); b++)
                {
                    const SdrPageWindow& rPageWindow = *pPageView->GetPageWindow(b);

                    if(rPageWindow.GetPaintWindow().OutputToWindow())
                    {
                        rtl::Reference< sdr::overlay::OverlayManager > xManager = rPageWindow.GetOverlayManager();
                        if (xManager.is())
                        {
                            // striped line in between
                            basegfx::B2DVector aVec(a2ndPos.X() - aPos.X(), a2ndPos.Y() - aPos.Y());
                            double fVecLen = aVec.getLength();
                            double fLongPercentArrow = (1.0 - 0.05) * fVecLen;
                            double fHalfArrowWidth = (0.05 * 0.5) * fVecLen;
                            aVec.normalize();
                            basegfx::B2DVector aPerpend(-aVec.getY(), aVec.getX());
                            sal_Int32 nMidX = (sal_Int32)(aPos.X() + aVec.getX() * fLongPercentArrow);
                            sal_Int32 nMidY = (sal_Int32)(aPos.Y() + aVec.getY() * fLongPercentArrow);
                            Point aMidPoint(nMidX, nMidY);

                            basegfx::B2DPoint aPosition(aPos.X(), aPos.Y());
                            basegfx::B2DPoint aMidPos(aMidPoint.X(), aMidPoint.Y());

                            sdr::overlay::OverlayObject* pNewOverlayObject = new
                                sdr::overlay::OverlayLineStriped(
                                    aPosition, aMidPos
                                );
                            DBG_ASSERT(pNewOverlayObject, "Got NO new IAO!");

                            pNewOverlayObject->setBaseColor(IsGradient() ? Color(COL_BLACK) : Color(COL_BLUE));
                            xManager->add(*pNewOverlayObject);
                            maOverlayGroup.append(*pNewOverlayObject);

                            // arrowhead
                            Point aLeft(aMidPoint.X() + (sal_Int32)(aPerpend.getX() * fHalfArrowWidth),
                                        aMidPoint.Y() + (sal_Int32)(aPerpend.getY() * fHalfArrowWidth));
                            Point aRight(aMidPoint.X() - (sal_Int32)(aPerpend.getX() * fHalfArrowWidth),
                                        aMidPoint.Y() - (sal_Int32)(aPerpend.getY() * fHalfArrowWidth));

                            basegfx::B2DPoint aPositionLeft(aLeft.X(), aLeft.Y());
                            basegfx::B2DPoint aPositionRight(aRight.X(), aRight.Y());
                            basegfx::B2DPoint aPosition2(a2ndPos.X(), a2ndPos.Y());

                            pNewOverlayObject = new
                                sdr::overlay::OverlayTriangle(
                                    aPositionLeft,
                                    aPosition2,
                                    aPositionRight,
                                    IsGradient() ? Color(COL_BLACK) : Color(COL_BLUE)
                                );
                            DBG_ASSERT(pNewOverlayObject, "Got NO new IAO!");

                            xManager->add(*pNewOverlayObject);
                            maOverlayGroup.append(*pNewOverlayObject);
                        }
                    }
                }
            }
        }
    }
}

IMPL_LINK_NOARG_TYPED(SdrHdlGradient, ColorChangeHdl, SdrHdlColor*, void)
{
    if(GetObj())
        FromIAOToItem(GetObj(), true, true);
}

void SdrHdlGradient::FromIAOToItem(SdrObject* _pObj, bool bSetItemOnObject, bool bUndo)
{
    // from IAO positions and colors to gradient
    const SfxItemSet& rSet = _pObj->GetMergedItemSet();

    GradTransformer aGradTransformer;
    GradTransGradient aOldGradTransGradient;
    GradTransGradient aGradTransGradient;
    GradTransVector aGradTransVector;

    OUString aString;

    aGradTransVector.maPositionA = basegfx::B2DPoint(GetPos().X(), GetPos().Y());
    aGradTransVector.maPositionB = basegfx::B2DPoint(Get2ndPos().X(), Get2ndPos().Y());
    if(pColHdl1)
        aGradTransVector.aCol1 = pColHdl1->GetColor();
    if(pColHdl2)
        aGradTransVector.aCol2 = pColHdl2->GetColor();

    if(IsGradient())
        aOldGradTransGradient.aGradient = static_cast<const XFillGradientItem&>(rSet.Get(XATTR_FILLGRADIENT)).GetGradientValue();
    else
        aOldGradTransGradient.aGradient = static_cast<const XFillFloatTransparenceItem&>(rSet.Get(XATTR_FILLFLOATTRANSPARENCE)).GetGradientValue();

    // transform vector data to gradient
    GradTransformer::VecToGrad(aGradTransVector, aGradTransGradient, aOldGradTransGradient, _pObj, bMoveSingleHandle, bMoveFirstHandle);

    if(bSetItemOnObject)
    {
        SdrModel* pModel = _pObj->GetModel();
        SfxItemSet aNewSet(pModel->GetItemPool());

        if(IsGradient())
        {
            aString.clear();
            XFillGradientItem aNewGradItem(aString, aGradTransGradient.aGradient);
            aNewSet.Put(aNewGradItem);
        }
        else
        {
            aString.clear();
            XFillFloatTransparenceItem aNewTransItem(aString, aGradTransGradient.aGradient);
            aNewSet.Put(aNewTransItem);
        }

        if(bUndo && pModel->IsUndoEnabled())
        {
            pModel->BegUndo(SVX_RESSTR(IsGradient() ? SIP_XA_FILLGRADIENT : SIP_XA_FILLTRANSPARENCE));
            pModel->AddUndo(pModel->GetSdrUndoFactory().CreateUndoAttrObject(*_pObj));
            pModel->EndUndo();
        }

        pObj->SetMergedItemSetAndBroadcast(aNewSet);
    }

    // back transformation, set values on pIAOHandle
    GradTransformer::GradToVec(aGradTransGradient, aGradTransVector, _pObj);

    SetPos(Point(FRound(aGradTransVector.maPositionA.getX()), FRound(aGradTransVector.maPositionA.getY())));
    Set2ndPos(Point(FRound(aGradTransVector.maPositionB.getX()), FRound(aGradTransVector.maPositionB.getY())));
    if(pColHdl1)
    {
        pColHdl1->SetPos(Point(FRound(aGradTransVector.maPositionA.getX()), FRound(aGradTransVector.maPositionA.getY())));
        pColHdl1->SetColor(aGradTransVector.aCol1);
    }
    if(pColHdl2)
    {
        pColHdl2->SetPos(Point(FRound(aGradTransVector.maPositionB.getX()), FRound(aGradTransVector.maPositionB.getY())));
        pColHdl2->SetColor(aGradTransVector.aCol2);
    }
}



SdrHdlLine::~SdrHdlLine() {}

void SdrHdlLine::CreateB2dIAObject()
{
    // first throw away old one
    GetRidOfIAObject();

    if(pHdlList)
    {
        SdrMarkView* pView = pHdlList->GetView();

        if(pView && !pView->areMarkHandlesHidden() && pHdl1 && pHdl2)
        {
            SdrPageView* pPageView = pView->GetSdrPageView();

            if(pPageView)
            {
                for(sal_uInt32 b(0L); b < pPageView->PageWindowCount(); b++)
                {
                    const SdrPageWindow& rPageWindow = *pPageView->GetPageWindow(b);

                    if(rPageWindow.GetPaintWindow().OutputToWindow())
                    {
                        rtl::Reference< sdr::overlay::OverlayManager > xManager = rPageWindow.GetOverlayManager();
                        if (xManager.is())
                        {
                            basegfx::B2DPoint aPosition1(pHdl1->GetPos().X(), pHdl1->GetPos().Y());
                            basegfx::B2DPoint aPosition2(pHdl2->GetPos().X(), pHdl2->GetPos().Y());

                            sdr::overlay::OverlayObject* pNewOverlayObject = new
                                sdr::overlay::OverlayLineStriped(
                                    aPosition1,
                                    aPosition2
                                );

                            // OVERLAYMANAGER
                            // color(?)
                            pNewOverlayObject->setBaseColor(Color(COL_LIGHTRED));

                            xManager->add(*pNewOverlayObject);
                            maOverlayGroup.append(*pNewOverlayObject);
                        }
                    }
                }
            }
        }
    }
}

Pointer SdrHdlLine::GetPointer() const
{
    return Pointer(PointerStyle::RefHand);
}



SdrHdlBezWgt::~SdrHdlBezWgt() {}

void SdrHdlBezWgt::CreateB2dIAObject()
{
    // call parent
    SdrHdl::CreateB2dIAObject();

    // create lines
    if(pHdlList)
    {
        SdrMarkView* pView = pHdlList->GetView();

        if(pView && !pView->areMarkHandlesHidden())
        {
            SdrPageView* pPageView = pView->GetSdrPageView();

            if(pPageView)
            {
                for(sal_uInt32 b(0L); b < pPageView->PageWindowCount(); b++)
                {
                    const SdrPageWindow& rPageWindow = *pPageView->GetPageWindow(b);

                    if(rPageWindow.GetPaintWindow().OutputToWindow())
                    {
                        rtl::Reference< sdr::overlay::OverlayManager > xManager = rPageWindow.GetOverlayManager();
                        if (xManager.is())
                        {
                            basegfx::B2DPoint aPosition1(pHdl1->GetPos().X(), pHdl1->GetPos().Y());
                            basegfx::B2DPoint aPosition2(aPos.X(), aPos.Y());

                            if(!aPosition1.equal(aPosition2))
                            {
                                sdr::overlay::OverlayObject* pNewOverlayObject = new
                                    sdr::overlay::OverlayLineStriped(
                                        aPosition1,
                                        aPosition2
                                    );
                                // OVERLAYMANAGER
                                // line part is not hittable
                                pNewOverlayObject->setHittable(false);

                                // color(?)
                                pNewOverlayObject->setBaseColor(Color(COL_LIGHTBLUE));

                                xManager->add(*pNewOverlayObject);
                                maOverlayGroup.append(*pNewOverlayObject);
                            }
                        }
                    }
                }
            }
        }
    }
}



E3dVolumeMarker::E3dVolumeMarker(const basegfx::B2DPolyPolygon& rWireframePoly)
{
    aWireframePoly = rWireframePoly;
}

void E3dVolumeMarker::CreateB2dIAObject()
{
    // create lines
    if(pHdlList)
    {
        SdrMarkView* pView = pHdlList->GetView();

        if(pView && !pView->areMarkHandlesHidden())
        {
            SdrPageView* pPageView = pView->GetSdrPageView();

            if(pPageView)
            {
                for(sal_uInt32 b(0L); b < pPageView->PageWindowCount(); b++)
                {
                    const SdrPageWindow& rPageWindow = *pPageView->GetPageWindow(b);

                    if(rPageWindow.GetPaintWindow().OutputToWindow())
                    {
                        rtl::Reference< sdr::overlay::OverlayManager > xManager = rPageWindow.GetOverlayManager();
                        if (xManager.is() && aWireframePoly.count())
                        {
                            sdr::overlay::OverlayObject* pNewOverlayObject = new
                            sdr::overlay::OverlayPolyPolygonStripedAndFilled(
                                aWireframePoly);

                            // OVERLAYMANAGER
                            pNewOverlayObject->setBaseColor(Color(COL_BLACK));

                            xManager->add(*pNewOverlayObject);
                            maOverlayGroup.append(*pNewOverlayObject);
                        }
                    }
                }
            }
        }
    }
}



ImpEdgeHdl::~ImpEdgeHdl()
{
}

void ImpEdgeHdl::CreateB2dIAObject()
{
    if(nObjHdlNum <= 1 && pObj)
    {
        // first throw away old one
        GetRidOfIAObject();

        BitmapColorIndex eColIndex = LightCyan;
        BitmapMarkerKind eKindOfMarker = Rect_7x7;

        if(pHdlList)
        {
            SdrMarkView* pView = pHdlList->GetView();

            if(pView && !pView->areMarkHandlesHidden())
            {
                const SdrEdgeObj* pEdge = static_cast<SdrEdgeObj*>(pObj);

                if(pEdge->GetConnectedNode(nObjHdlNum == 0) != nullptr)
                    eColIndex = LightRed;

                if(nPPntNum < 2)
                {
                    // Handle with plus sign inside
                    eKindOfMarker = Circ_7x7;
                }

                SdrPageView* pPageView = pView->GetSdrPageView();

                if(pPageView)
                {
                    for(sal_uInt32 b(0); b < pPageView->PageWindowCount(); b++)
                    {
                        const SdrPageWindow& rPageWindow = *pPageView->GetPageWindow(b);

                        if(rPageWindow.GetPaintWindow().OutputToWindow())
                        {
                            rtl::Reference< sdr::overlay::OverlayManager > xManager = rPageWindow.GetOverlayManager();
                            if (xManager.is())
                            {
                                basegfx::B2DPoint aPosition(aPos.X(), aPos.Y());
                                OutputDevice& rOutDev = rPageWindow.GetPaintWindow().GetOutputDevice();
                                sdr::overlay::OverlayObject* pNewOverlayObject = CreateOverlayObject(
                                    aPosition,
                                    eColIndex,
                                    eKindOfMarker,
                                    rOutDev);

                                // OVERLAYMANAGER
                                if (pNewOverlayObject)
                                {
                                    xManager->add(*pNewOverlayObject);
                                    maOverlayGroup.append(*pNewOverlayObject);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else
    {
        // call parent
        SdrHdl::CreateB2dIAObject();
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
    SdrEdgeObj* pEdge=dynamic_cast<SdrEdgeObj*>( pObj );
    if (pEdge==nullptr)
        return SdrHdl::GetPointer();
    if (nObjHdlNum<=1)
        return Pointer(PointerStyle::MovePoint);
    if (IsHorzDrag())
        return Pointer(PointerStyle::ESize);
    else
        return Pointer(PointerStyle::SSize);
}

bool ImpEdgeHdl::IsHorzDrag() const
{
    SdrEdgeObj* pEdge=dynamic_cast<SdrEdgeObj*>( pObj );
    if (pEdge==nullptr)
        return false;
    if (nObjHdlNum<=1)
        return false;

    SdrEdgeKind eEdgeKind = static_cast<const SdrEdgeKindItem&>(pEdge->GetObjectItem(SDRATTR_EDGEKIND)).GetValue();

    const SdrEdgeInfoRec& rInfo=pEdge->aEdgeInfo;
    if (eEdgeKind==SDREDGE_ORTHOLINES || eEdgeKind==SDREDGE_BEZIER)
    {
        return !rInfo.ImpIsHorzLine(eLineCode,*pEdge->pEdgeTrack);
    }
    else if (eEdgeKind==SDREDGE_THREELINES)
    {
        long nAngle=nObjHdlNum==2 ? rInfo.nAngle1 : rInfo.nAngle2;
        if (nAngle==0 || nAngle==18000)
            return true;
        else
            return false;
    }
    return false;
}



ImpMeasureHdl::~ImpMeasureHdl()
{
}

void ImpMeasureHdl::CreateB2dIAObject()
{
    // first throw away old one
    GetRidOfIAObject();

    if(pHdlList)
    {
        SdrMarkView* pView = pHdlList->GetView();

        if(pView && !pView->areMarkHandlesHidden())
        {
            BitmapColorIndex eColIndex = LightCyan;
            BitmapMarkerKind eKindOfMarker = Rect_9x9;

            if(nObjHdlNum > 1)
            {
                eKindOfMarker = Rect_7x7;
            }

            if(bSelect)
            {
                eColIndex = Cyan;
            }

            SdrPageView* pPageView = pView->GetSdrPageView();

            if(pPageView)
            {
                for(sal_uInt32 b(0L); b < pPageView->PageWindowCount(); b++)
                {
                    const SdrPageWindow& rPageWindow = *pPageView->GetPageWindow(b);

                    if(rPageWindow.GetPaintWindow().OutputToWindow())
                    {
                        rtl::Reference< sdr::overlay::OverlayManager > xManager = rPageWindow.GetOverlayManager();
                        if (xManager.is())
                        {
                            basegfx::B2DPoint aPosition(aPos.X(), aPos.Y());
                            OutputDevice& rOutDev = rPageWindow.GetPaintWindow().GetOutputDevice();
                            sdr::overlay::OverlayObject* pNewOverlayObject = CreateOverlayObject(
                                aPosition,
                                eColIndex,
                                eKindOfMarker,
                                rOutDev);

                            // OVERLAYMANAGER
                            if (pNewOverlayObject)
                            {
                                xManager->add(*pNewOverlayObject);
                                maOverlayGroup.append(*pNewOverlayObject);
                            }
                        }
                    }
                }
            }
        }
    }
}

Pointer ImpMeasureHdl::GetPointer() const
{
    switch (nObjHdlNum)
    {
        case 0: case 1: return Pointer(PointerStyle::Hand);
        case 2: case 3: return Pointer(PointerStyle::MovePoint);
        case 4: case 5: return SdrHdl::GetPointer(); // will then be rotated appropriately
    } // switch
    return Pointer(PointerStyle::NotAllowed);
}



ImpTextframeHdl::ImpTextframeHdl(const Rectangle& rRect) :
    SdrHdl(rRect.TopLeft(),HDL_MOVE),
    maRect(rRect)
{
}

void ImpTextframeHdl::CreateB2dIAObject()
{
    // first throw away old one
    GetRidOfIAObject();

    if(pHdlList)
    {
        SdrMarkView* pView = pHdlList->GetView();

        if(pView && !pView->areMarkHandlesHidden())
        {
            SdrPageView* pPageView = pView->GetSdrPageView();

            if(pPageView)
            {
                for(sal_uInt32 b(0L); b < pPageView->PageWindowCount(); b++)
                {
                    const SdrPageWindow& rPageWindow = *pPageView->GetPageWindow(b);

                    if(rPageWindow.GetPaintWindow().OutputToWindow())
                    {
                        rtl::Reference< sdr::overlay::OverlayManager > xManager = rPageWindow.GetOverlayManager();
                        if (xManager.is())
                        {
                            const basegfx::B2DPoint aTopLeft(maRect.Left(), maRect.Top());
                            const basegfx::B2DPoint aBottomRight(maRect.Right(), maRect.Bottom());
                            const SvtOptionsDrawinglayer aSvtOptionsDrawinglayer;
                            const Color aHilightColor(aSvtOptionsDrawinglayer.getHilightColor());
                            const double fTransparence(aSvtOptionsDrawinglayer.GetTransparentSelectionPercent() * 0.01);

                            sdr::overlay::OverlayRectangle* pNewOverlayObject = new sdr::overlay::OverlayRectangle(
                                aTopLeft,
                                aBottomRight,
                                aHilightColor,
                                fTransparence,
                                3.0,
                                3.0,
                                nRotationAngle * -F_PI18000,
                                500,
                                true); // allow animation; the Handle is not shown at text edit time

                            // OVERLAYMANAGER
                            pNewOverlayObject->setHittable(false);
                            xManager->add(*pNewOverlayObject);
                            maOverlayGroup.append(*pNewOverlayObject);
                        }
                    }
                }
            }
        }
    }
}



static bool ImpSdrHdlListSorter(SdrHdl* const& lhs, SdrHdl* const& rhs)
{
    SdrHdlKind eKind1=lhs->GetKind();
    SdrHdlKind eKind2=rhs->GetKind();
    // Level 1: first normal handles, then Glue, then User, then Plus handles, then reference point handles
    unsigned n1=1;
    unsigned n2=1;
    if (eKind1!=eKind2)
    {
        if (eKind1==HDL_REF1 || eKind1==HDL_REF2 || eKind1==HDL_MIRX) n1=5;
        else if (eKind1==HDL_GLUE || eKind1==HDL_GLUE_DESELECTED) n1=2;
        else if (eKind1==HDL_USER) n1=3;
        else if (eKind1==HDL_SMARTTAG) n1=0;
        if (eKind2==HDL_REF1 || eKind2==HDL_REF2 || eKind2==HDL_MIRX) n2=5;
        else if (eKind2==HDL_GLUE || eKind2==HDL_GLUE_DESELECTED) n2=2;
        else if (eKind2==HDL_USER) n2=3;
        else if (eKind2==HDL_SMARTTAG) n2=0;
    }
    if (lhs->IsPlusHdl()) n1=4;
    if (rhs->IsPlusHdl()) n2=4;
    if (n1==n2)
    {
        // Level 2: PageView (Pointer)
        SdrPageView* pPV1=lhs->GetPageView();
        SdrPageView* pPV2=rhs->GetPageView();
        if (pPV1==pPV2)
        {
            // Level 3: Position (x+y)
            SdrObject* pObj1=lhs->GetObj();
            SdrObject* pObj2=rhs->GetObj();
            if (pObj1==pObj2)
            {
                sal_uInt32 nNum1=lhs->GetObjHdlNum();
                sal_uInt32 nNum2=rhs->GetObjHdlNum();
                if (nNum1==nNum2)
                {
                    if (eKind1==eKind2)
                        return lhs<rhs; // Hack, to always get to the same sorting
                    return (sal_uInt16)eKind1<(sal_uInt16)eKind2;
                }
                else
                    return nNum1<nNum2;
            }
            else
            {
                return pObj1<pObj2;
            }
        }
        else
        {
            return pPV1<pPV2;
        }
    }
    else
    {
        return n1<n2;
    }
}


// Helper struct for re-sorting handles
struct ImplHdlAndIndex
{
    SdrHdl*                     mpHdl;
    sal_uInt32                  mnIndex;
};

// Helper method for sorting handles taking care of OrdNums, keeping order in
// single objects and re-sorting polygon handles intuitively
extern "C" int SAL_CALL ImplSortHdlFunc( const void* pVoid1, const void* pVoid2 )
{
    const ImplHdlAndIndex* p1 = static_cast<ImplHdlAndIndex const *>(pVoid1);
    const ImplHdlAndIndex* p2 = static_cast<ImplHdlAndIndex const *>(pVoid2);

    if(p1->mpHdl->GetObj() == p2->mpHdl->GetObj())
    {
        if(p1->mpHdl->GetObj() && dynamic_cast<const SdrPathObj*>(p1->mpHdl->GetObj()) != nullptr)
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
            const sal_uInt32 nOrdNum1 = p1->mpHdl->GetObj()->GetOrdNum();
            const sal_uInt32 nOrdNum2 = p2->mpHdl->GetObj()->GetOrdNum();

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



void SdrHdlList::TravelFocusHdl(bool bForward)
{
    // security correction
    if (mnFocusIndex >= GetHdlCount())
        mnFocusIndex = SAL_MAX_SIZE;

    if(!aList.empty())
    {
        // take care of old handle
        const size_t nOldHdlNum(mnFocusIndex);
        SdrHdl* pOld = GetHdl(nOldHdlNum);

        if(pOld)
        {
            // switch off old handle
            mnFocusIndex = SAL_MAX_SIZE;
            pOld->Touch();
        }

        // allocate pointer array for sorted handle list
        std::unique_ptr<ImplHdlAndIndex[]> pHdlAndIndex(new ImplHdlAndIndex[aList.size()]);

        // build sorted handle list
        for( size_t a = 0; a < aList.size(); ++a)
        {
            pHdlAndIndex[a].mpHdl = aList[a];
            pHdlAndIndex[a].mnIndex = a;
        }

        qsort(pHdlAndIndex.get(), aList.size(), sizeof(ImplHdlAndIndex), ImplSortHdlFunc);

        // look for old num in sorted array
        size_t nOldHdl(nOldHdlNum);

        if(nOldHdlNum != SAL_MAX_SIZE)
        {
            for(size_t a = 0; a < aList.size(); ++a)
            {
                if(pHdlAndIndex[a].mpHdl == pOld)
                {
                    nOldHdl = a;
                    break;
                }
            }
        }

        // build new HdlNum
        size_t nNewHdl(nOldHdl);

        // do the focus travel
        if(bForward)
        {
            if(nOldHdl != SAL_MAX_SIZE)
            {
                if(nOldHdl == aList.size() - 1)
                {
                    // end forward run
                    nNewHdl = SAL_MAX_SIZE;
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
            if(nOldHdl == SAL_MAX_SIZE)
            {
                // start backward run at last entry
                nNewHdl = aList.size() - 1;

            }
            else
            {
                if(nOldHdl == 0)
                {
                    // end backward run
                    nNewHdl = SAL_MAX_SIZE;
                }
                else
                {
                    // simply the previous handle
                    nNewHdl--;
                }
            }
        }

        // build new HdlNum
        sal_uIntPtr nNewHdlNum(nNewHdl);

        // look for old num in sorted array
        if(nNewHdl != SAL_MAX_SIZE)
        {
            SdrHdl* pNew = pHdlAndIndex[nNewHdl].mpHdl;

            for(size_t a = 0; a < aList.size(); ++a)
            {
                if(aList[a] == pNew)
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
            SdrHdl* pNew = GetHdl(mnFocusIndex);

            if(pNew)
            {
                pNew->Touch();
            }
        }
    }
}

SdrHdl* SdrHdlList::GetFocusHdl() const
{
    if(mnFocusIndex < GetHdlCount())
        return GetHdl(mnFocusIndex);
    else
        return nullptr;
}

void SdrHdlList::SetFocusHdl(SdrHdl* pNew)
{
    if(pNew)
    {
        SdrHdl* pActual = GetFocusHdl();

        if(!pActual || pActual != pNew)
        {
            const size_t nNewHdlNum = GetHdlNum(pNew);

            if(nNewHdlNum != SAL_MAX_SIZE)
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

    mnFocusIndex = SAL_MAX_SIZE;

    if(pHdl)
    {
        pHdl->Touch();
    }
}



SdrHdlList::SdrHdlList(SdrMarkView* pV)
:   mnFocusIndex(SAL_MAX_SIZE),
    pView(pV),
    aList()
{
    nHdlSize = 3;
    bRotateShear = false;
    bMoveOutside = false;
    bDistortShear = false;
}

SdrHdlList::~SdrHdlList()
{
    Clear();
}

void SdrHdlList::SetHdlSize(sal_uInt16 nSiz)
{
    if(nHdlSize != nSiz)
    {
        // remember new value
        nHdlSize = nSiz;

        // propagate change to IAOs
        for(size_t i=0; i<GetHdlCount(); ++i)
        {
            SdrHdl* pHdl = GetHdl(i);
            pHdl->Touch();
        }
    }
}

void SdrHdlList::SetMoveOutside(bool bOn)
{
    if(bMoveOutside != bOn)
    {
        // remember new value
        bMoveOutside = bOn;

        // propagate change to IAOs
        for(size_t i=0; i<GetHdlCount(); ++i)
        {
            SdrHdl* pHdl = GetHdl(i);
            pHdl->Touch();
        }
    }
}

void SdrHdlList::SetRotateShear(bool bOn)
{
    bRotateShear = bOn;
}

void SdrHdlList::SetDistortShear(bool bOn)
{
    bDistortShear = bOn;
}

SdrHdl* SdrHdlList::RemoveHdl(size_t nNum)
{
    SdrHdl* pRetval = aList[nNum];
    aList.erase(aList.begin() + nNum);

    return pRetval;
}

void SdrHdlList::RemoveAllByKind(SdrHdlKind eKind)
{
    for(std::deque<SdrHdl*>::iterator it = aList.begin(); it != aList.end(); )
    {
        SdrHdl* p = *it;
        if (p->GetKind() == eKind)
        {
            it = aList.erase( it );
            delete p;
        }
        else
            ++it;
    }
}

void SdrHdlList::Clear()
{
    for (size_t i=0; i<GetHdlCount(); ++i)
    {
        SdrHdl* pHdl=GetHdl(i);
        delete pHdl;
    }
    aList.clear();

    bRotateShear=false;
    bDistortShear=false;
}

void SdrHdlList::Sort()
{
    // remember currently focused handle
    SdrHdl* pPrev = GetFocusHdl();

    std::sort( aList.begin(), aList.end(), ImpSdrHdlListSorter );

    // get now and compare
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

size_t SdrHdlList::GetHdlNum(const SdrHdl* pHdl) const
{
    if (pHdl==nullptr)
        return SAL_MAX_SIZE;
    std::deque<SdrHdl*>::const_iterator it = std::find( aList.begin(), aList.end(), pHdl);
    if( it == aList.end() )
        return SAL_MAX_SIZE;
    return it - aList.begin();
}

void SdrHdlList::AddHdl(SdrHdl* pHdl, bool bAtBegin)
{
    if (pHdl!=nullptr)
    {
        if (bAtBegin)
        {
            aList.push_front(pHdl);
        }
        else
        {
            aList.push_back(pHdl);
        }
        pHdl->SetHdlList(this);
    }
}

SdrHdl* SdrHdlList::IsHdlListHit(const Point& rPnt, bool bBack, bool bNext, SdrHdl* pHdl0) const
{
    SdrHdl* pRet=nullptr;
    const size_t nCount=GetHdlCount();
    size_t nNum=bBack ? 0 : nCount;
    while ((bBack ? nNum<nCount : nNum>0) && pRet==nullptr)
    {
        if (!bBack)
            nNum--;
        SdrHdl* pHdl=GetHdl(nNum);
        if (bNext)
        {
            if (pHdl==pHdl0)
                bNext=false;
        }
        else
        {
            if (pHdl->IsHdlHit(rPnt))
                pRet=pHdl;
        }
        if (bBack)
            nNum++;
    }
    return pRet;
}

SdrHdl* SdrHdlList::GetHdl(SdrHdlKind eKind1) const
{
    SdrHdl* pRet=nullptr;
    for (size_t i=0; i<GetHdlCount() && pRet==nullptr; ++i)
    {
        SdrHdl* pHdl=GetHdl(i);
        if (pHdl->GetKind()==eKind1)
            pRet=pHdl;
    }
    return pRet;
}

SdrCropHdl::SdrCropHdl(
    const Point& rPnt,
    SdrHdlKind eNewKind,
    double fShearX,
    double fRotation)
:   SdrHdl(rPnt, eNewKind),
    mfShearX(fShearX),
    mfRotation(fRotation)
{
}



BitmapEx SdrCropHdl::GetHandlesBitmap()
{
    return BitmapEx(ResId(SIP_SA_CROP_MARKERS, *ImpGetResMgr()));
}



BitmapEx SdrCropHdl::GetBitmapForHandle( const BitmapEx& rBitmap, int nSize )
{
    int nPixelSize = 0, nX = 0, nY = 0, nOffset = 0;

    if( nSize <= 3 )
    {
        nPixelSize = 13;
        nOffset = 0;
    }
    else if( nSize <=4 )
    {
        nPixelSize = 17;
        nOffset = 39;
    }
    else
    {
        nPixelSize = 21;
        nOffset = 90;
    }

    switch( eKind )
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

    Rectangle aSourceRect( Point( nX * (nPixelSize) + nOffset,  nY * (nPixelSize)), Size(nPixelSize, nPixelSize) );

    BitmapEx aRetval(rBitmap);
    aRetval.Crop(aSourceRect);
    return aRetval;
}



void SdrCropHdl::CreateB2dIAObject()
{
    // first throw away old one
    GetRidOfIAObject();

    SdrMarkView* pView = pHdlList ? pHdlList->GetView() : nullptr;
    SdrPageView* pPageView = pView ? pView->GetSdrPageView() : nullptr;

    if( pPageView && !pView->areMarkHandlesHidden() )
    {
        const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
        int nHdlSize = pHdlList->GetHdlSize();

        const BitmapEx aHandlesBitmap( GetHandlesBitmap() );
        BitmapEx aBmpEx1( GetBitmapForHandle( aHandlesBitmap, nHdlSize ) );

        for(sal_uInt32 b(0L); b < pPageView->PageWindowCount(); b++)
        {
            const SdrPageWindow& rPageWindow = *pPageView->GetPageWindow(b);

            if(rPageWindow.GetPaintWindow().OutputToWindow())
            {
                rtl::Reference< sdr::overlay::OverlayManager > xManager = rPageWindow.GetOverlayManager();
                if (xManager.is())
                {
                    basegfx::B2DPoint aPosition(aPos.X(), aPos.Y());

                    sdr::overlay::OverlayObject* pOverlayObject = nullptr;

                    // animate focused handles
                    if(IsFocusHdl() && (pHdlList->GetFocusHdl() == this))
                    {
                        if( nHdlSize >= 2 )
                            nHdlSize = 1;

                        BitmapEx aBmpEx2( GetBitmapForHandle( aHandlesBitmap, nHdlSize + 1 ) );

                        const sal_uInt64 nBlinkTime = rStyleSettings.GetCursorBlinkTime();

                        pOverlayObject = new sdr::overlay::OverlayAnimatedBitmapEx(
                            aPosition,
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
                        pOverlayObject = new sdr::overlay::OverlayBitmapEx(
                            aPosition,
                            aBmpEx1,
                            (sal_uInt16)(aBmpEx1.GetSizePixel().Width() - 1) >> 1,
                            (sal_uInt16)(aBmpEx1.GetSizePixel().Height() - 1) >> 1,
                            0.0,
                            mfShearX,
                            mfRotation);
                    }

                    // OVERLAYMANAGER
                    if(pOverlayObject)
                    {
                        xManager->add(*pOverlayObject);
                        maOverlayGroup.append(*pOverlayObject);
                    }
                }
            }
        }
    }
}


// with the correction of crop handling I could get rid of the extra mirroring flag, adapted stuff
// accordingly

SdrCropViewHdl::SdrCropViewHdl(
    const basegfx::B2DHomMatrix& rObjectTransform,
    const Graphic& rGraphic,
    double fCropLeft,
    double fCropTop,
    double fCropRight,
    double fCropBottom)
:   SdrHdl(Point(), HDL_USER),
    maObjectTransform(rObjectTransform),
    maGraphic(rGraphic),
    mfCropLeft(fCropLeft),
    mfCropTop(fCropTop),
    mfCropRight(fCropRight),
    mfCropBottom(fCropBottom)
{
}

namespace {

void translateRotationToMirroring(basegfx::B2DVector & scale, double * rotate) {
    assert(rotate != nullptr);

    // detect 180 degree rotation, this is the same as mirrored in X and Y,
    // thus change to mirroring. Prefer mirroring here. Use the equal call
    // with getSmallValue here, the original which uses rtl::math::approxEqual
    // is too correct here. Maybe this changes with enhanced precision in aw080
    // to the better so that this can be reduced to the more precise call again
    if(basegfx::fTools::equal(fabs(*rotate), F_PI, 0.000000001))
    {
        scale.setX(scale.getX() * -1.0);
        scale.setY(scale.getY() * -1.0);
        *rotate = 0.0;
    }
}

}

void SdrCropViewHdl::CreateB2dIAObject()
{
    GetRidOfIAObject();
    SdrMarkView* pView = pHdlList ? pHdlList->GetView() : nullptr;
    SdrPageView* pPageView = pView ? pView->GetSdrPageView() : nullptr;

    if(!pPageView || pView->areMarkHandlesHidden())
    {
        return;
    }

    // decompose to have current translate and scale
    basegfx::B2DVector aScale, aTranslate;
    double fRotate, fShearX;

    maObjectTransform.decompose(aScale, aTranslate, fRotate, fShearX);

    if(aScale.equalZero())
    {
        return;
    }

    translateRotationToMirroring(aScale, &fRotate);

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

    // prepare graphic primitive (transformed)
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
    drawinglayer::primitive2d::Primitive2DContainer aCombination(2);
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
            drawinglayer::primitive2d::Primitive2DContainer { aMaskedGraphic },
            0.8));

    const drawinglayer::primitive2d::Primitive2DContainer aSequence { aTransparenceMaskedGraphic };

    for(sal_uInt32 b(0L); b < pPageView->PageWindowCount(); b++)
    {
        // const SdrPageViewWinRec& rPageViewWinRec = rPageViewWinList[b];
        const SdrPageWindow& rPageWindow = *(pPageView->GetPageWindow(b));

        if(rPageWindow.GetPaintWindow().OutputToWindow())
        {
            rtl::Reference< sdr::overlay::OverlayManager > xManager = rPageWindow.GetOverlayManager();
            if(xManager.is())
            {
                sdr::overlay::OverlayObject* pNew = new sdr::overlay::OverlayPrimitive2DSequenceObject(aSequence);

                // only informative object, no hit
                pNew->setHittable(false);

                xManager->add(*pNew);
                maOverlayGroup.append(*pNew);
            }
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
