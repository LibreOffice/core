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
#include <svx/svdmrkv.hxx>
#include <vcl/settings.hxx>
#include <vcl/virdev.hxx>
#include <vcl/ptrstyle.hxx>

#include <svx/sxekitm.hxx>
#include <svx/strings.hrc>
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
#include <sdr/overlay/overlaytriangle.hxx>
#include <sdr/overlay/overlayhandle.hxx>
#include <sdr/overlay/overlayrectangle.hxx>
#include <svx/sdrpagewindow.hxx>
#include <svx/sdrpaintwindow.hxx>
#include <vcl/svapp.hxx>
#include <svx/sdr/overlay/overlaypolypolygon.hxx>
#include <vcl/lazydelete.hxx>
#include <vcl/BitmapTools.hxx>
#include <svx/sdr/contact/objectcontact.hxx>
#include <svx/sdr/contact/viewcontact.hxx>
#include <osl/diagnose.h>

#include <basegfx/polygon/b2dpolygontools.hxx>
#include <svx/sdr/overlay/overlayprimitive2dsequenceobject.hxx>
#include <drawinglayer/primitive2d/graphicprimitive2d.hxx>
#include <drawinglayer/primitive2d/maskprimitive2d.hxx>
#include <drawinglayer/primitive2d/unifiedtransparenceprimitive2d.hxx>
#include <drawinglayer/primitive2d/polygonprimitive2d.hxx>
#include <svtools/optionsdrawinglayer.hxx>
#include <memory>
#include <bitmaps.hlst>

namespace {

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
    BitmapEx& impGetOrCreateTargetBitmap(sal_uInt16 nIndex, const tools::Rectangle& rRectangle);

public:
    explicit SdrHdlBitmapSet();

    const BitmapEx& GetBitmapEx(BitmapMarkerKind eKindOfMarker, sal_uInt16 nInd);
};

}

#define KIND_COUNT          (14)
#define INDEX_COUNT         (6)
#define INDIVIDUAL_COUNT    (5)

SdrHdlBitmapSet::SdrHdlBitmapSet()
    :   maMarkersBitmap(SIP_SA_MARKERS),
        // 15 kinds (BitmapMarkerKind) use index [0..5] + 5 extra
        maRealMarkers((KIND_COUNT * INDEX_COUNT) + INDIVIDUAL_COUNT)
{
}

BitmapEx& SdrHdlBitmapSet::impGetOrCreateTargetBitmap(sal_uInt16 nIndex, const tools::Rectangle& rRectangle)
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
            [[fallthrough]]; // return Rect_9x9 as default
        }
        case BitmapMarkerKind::Rect_9x9:
        {
            return impGetOrCreateTargetBitmap((1 * INDEX_COUNT) + nInd, tools::Rectangle(Point(7, nYPos), Size(9, 9)));
        }

        case BitmapMarkerKind::Rect_7x7:
        {
            return impGetOrCreateTargetBitmap((0 * INDEX_COUNT) + nInd, tools::Rectangle(Point(0, nYPos), Size(7, 7)));
        }

        case BitmapMarkerKind::Rect_11x11:
        {
            return impGetOrCreateTargetBitmap((2 * INDEX_COUNT) + nInd, tools::Rectangle(Point(16, nYPos), Size(11, 11)));
        }

        case BitmapMarkerKind::Rect_13x13:
        {
            const sal_uInt16 nIndex((3 * INDEX_COUNT) + nInd);

            switch(nInd)
            {
                case 0:
                {
                    return impGetOrCreateTargetBitmap(nIndex, tools::Rectangle(Point(72, 66), Size(13, 13)));
                }
                case 1:
                {
                    return impGetOrCreateTargetBitmap(nIndex, tools::Rectangle(Point(85, 66), Size(13, 13)));
                }
                case 2:
                {
                    return impGetOrCreateTargetBitmap(nIndex, tools::Rectangle(Point(72, 79), Size(13, 13)));
                }
                case 3:
                {
                    return impGetOrCreateTargetBitmap(nIndex, tools::Rectangle(Point(85, 79), Size(13, 13)));
                }
                case 4:
                {
                    return impGetOrCreateTargetBitmap(nIndex, tools::Rectangle(Point(98, 79), Size(13, 13)));
                }
                default: // case 5:
                {
                    return impGetOrCreateTargetBitmap(nIndex, tools::Rectangle(Point(98, 66), Size(13, 13)));
                }
            }
        }

        case BitmapMarkerKind::Circ_7x7:
        case BitmapMarkerKind::Customshape_7x7:
        {
            return impGetOrCreateTargetBitmap((4 * INDEX_COUNT) + nInd, tools::Rectangle(Point(27, nYPos), Size(7, 7)));
        }

        case BitmapMarkerKind::Circ_9x9:
        case BitmapMarkerKind::Customshape_9x9:
        {
            return impGetOrCreateTargetBitmap((5 * INDEX_COUNT) + nInd, tools::Rectangle(Point(34, nYPos), Size(9, 9)));
        }

        case BitmapMarkerKind::Circ_11x11:
        case BitmapMarkerKind::Customshape_11x11:
        {
            return impGetOrCreateTargetBitmap((6 * INDEX_COUNT) + nInd, tools::Rectangle(Point(43, nYPos), Size(11, 11)));
        }

        case BitmapMarkerKind::Elli_7x9:
        {
            return impGetOrCreateTargetBitmap((7 * INDEX_COUNT) + nInd, tools::Rectangle(Point(54, nYPos), Size(7, 9)));
        }

        case BitmapMarkerKind::Elli_9x11:
        {
            return impGetOrCreateTargetBitmap((8 * INDEX_COUNT) + nInd, tools::Rectangle(Point(61, nYPos), Size(9, 11)));
        }

        case BitmapMarkerKind::Elli_9x7:
        {
            return impGetOrCreateTargetBitmap((9 * INDEX_COUNT) + nInd, tools::Rectangle(Point(70, nYPos), Size(9, 7)));
        }

        case BitmapMarkerKind::Elli_11x9:
        {
            return impGetOrCreateTargetBitmap((10 * INDEX_COUNT) + nInd, tools::Rectangle(Point(79, nYPos), Size(11, 9)));
        }

        case BitmapMarkerKind::RectPlus_7x7:
        {
            return impGetOrCreateTargetBitmap((11 * INDEX_COUNT) + nInd, tools::Rectangle(Point(90, nYPos), Size(7, 7)));
        }

        case BitmapMarkerKind::RectPlus_9x9:
        {
            return impGetOrCreateTargetBitmap((12 * INDEX_COUNT) + nInd, tools::Rectangle(Point(97, nYPos), Size(9, 9)));
        }

        case BitmapMarkerKind::RectPlus_11x11:
        {
            return impGetOrCreateTargetBitmap((13 * INDEX_COUNT) + nInd, tools::Rectangle(Point(106, nYPos), Size(11, 11)));
        }

        case BitmapMarkerKind::Crosshair:
        {
            return impGetOrCreateTargetBitmap((KIND_COUNT * INDEX_COUNT) + 0, tools::Rectangle(Point(0, 68), Size(15, 15)));
        }

        case BitmapMarkerKind::Glue:
        {
            return impGetOrCreateTargetBitmap((KIND_COUNT * INDEX_COUNT) + 1, tools::Rectangle(Point(15, 76), Size(9, 9)));
        }

        case BitmapMarkerKind::Glue_Deselected:
        {
            return impGetOrCreateTargetBitmap((KIND_COUNT * INDEX_COUNT) + 2, tools::Rectangle(Point(15, 67), Size(9, 9)));
        }

        case BitmapMarkerKind::Anchor: // AnchorTR for SW
        case BitmapMarkerKind::AnchorTR:
        {
            return impGetOrCreateTargetBitmap((KIND_COUNT * INDEX_COUNT) + 3, tools::Rectangle(Point(24, 67), Size(24, 24)));
        }

        // add AnchorPressed to be able to animate anchor control
        case BitmapMarkerKind::AnchorPressed:
        case BitmapMarkerKind::AnchorPressedTR:
        {
            return impGetOrCreateTargetBitmap((KIND_COUNT * INDEX_COUNT) + 4, tools::Rectangle(Point(48, 67), Size(24, 24)));
        }
    }
}


SdrHdl::SdrHdl():
    pObj(nullptr),
    pPV(nullptr),
    pHdlList(nullptr),
    eKind(SdrHdlKind::Move),
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

void SdrHdl::SetRotationAngle(Degree100 n)
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

        // now it's possible to create graphic representation
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

    if(!pHdlList || !pHdlList->GetView() || pHdlList->GetView()->areMarkHandlesHidden())
        return;

    BitmapColorIndex eColIndex = BitmapColorIndex::LightGreen;
    BitmapMarkerKind eKindOfMarker = BitmapMarkerKind::Rect_7x7;

    bool bRot = pHdlList->IsRotateShear();
    if(pObj)
        eColIndex = bSelect ? BitmapColorIndex::Cyan : BitmapColorIndex::LightCyan;
    if(bRot)
    {
        // red rotation handles
        if(pObj && bSelect)
            eColIndex = BitmapColorIndex::Red;
        else
            eColIndex = BitmapColorIndex::LightRed;
    }

    switch(eKind)
    {
        case SdrHdlKind::Move:
        {
            eKindOfMarker = b1PixMore ? BitmapMarkerKind::Rect_9x9 : BitmapMarkerKind::Rect_7x7;
            break;
        }
        case SdrHdlKind::UpperLeft:
        case SdrHdlKind::UpperRight:
        case SdrHdlKind::LowerLeft:
        case SdrHdlKind::LowerRight:
        {
            // corner handles
            if(bRot)
            {
                eKindOfMarker = BitmapMarkerKind::Circ_7x7;
            }
            else
            {
                eKindOfMarker = BitmapMarkerKind::Rect_7x7;
            }
            break;
        }
        case SdrHdlKind::Upper:
        case SdrHdlKind::Lower:
        {
            // Upper/Lower handles
            if(bRot)
            {
                eKindOfMarker = BitmapMarkerKind::Elli_9x7;
            }
            else
            {
                eKindOfMarker = BitmapMarkerKind::Rect_7x7;
            }
            break;
        }
        case SdrHdlKind::Left:
        case SdrHdlKind::Right:
        {
            // Left/Right handles
            if(bRot)
            {
                eKindOfMarker = BitmapMarkerKind::Elli_7x9;
            }
            else
            {
                eKindOfMarker = BitmapMarkerKind::Rect_7x7;
            }
            break;
        }
        case SdrHdlKind::Poly:
        {
            if(bRot)
            {
                eKindOfMarker = b1PixMore ? BitmapMarkerKind::Circ_9x9 : BitmapMarkerKind::Circ_7x7;
            }
            else
            {
                eKindOfMarker = b1PixMore ? BitmapMarkerKind::Rect_9x9 : BitmapMarkerKind::Rect_7x7;
            }
            break;
        }
        case SdrHdlKind::BezierWeight: // weight at poly
        {
            eKindOfMarker = BitmapMarkerKind::Circ_7x7;
            break;
        }
        case SdrHdlKind::Circle:
        {
            eKindOfMarker = BitmapMarkerKind::Rect_11x11;
            break;
        }
        case SdrHdlKind::Ref1:
        case SdrHdlKind::Ref2:
        {
            eKindOfMarker = BitmapMarkerKind::Crosshair;
            break;
        }
        case SdrHdlKind::Glue:
        {
            eKindOfMarker = BitmapMarkerKind::Glue;
            break;
        }
        case SdrHdlKind::Anchor:
        {
            eKindOfMarker = BitmapMarkerKind::Anchor;
            break;
        }
        case SdrHdlKind::User:
        {
            break;
        }
        // top right anchor for SW
        case SdrHdlKind::Anchor_TR:
        {
            eKindOfMarker = BitmapMarkerKind::AnchorTR;
            break;
        }

        // for SJ and the CustomShapeHandles:
        case SdrHdlKind::CustomShape1:
        {
            eKindOfMarker = b1PixMore ? BitmapMarkerKind::Customshape_9x9 : BitmapMarkerKind::Customshape_7x7;
            eColIndex = BitmapColorIndex::Yellow;
            break;
        }
        default:
            break;
    }

    SdrMarkView* pView = pHdlList->GetView();
    SdrPageView* pPageView = pView->GetSdrPageView();

    if(!pPageView)
        return;

    for(sal_uInt32 b(0); b < pPageView->PageWindowCount(); b++)
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

                if(eKind == SdrHdlKind::UpperLeft || eKind == SdrHdlKind::Upper || eKind == SdrHdlKind::UpperRight)
                    aMoveOutsideOffset.AdjustY( -(aOffset.Width()) );
                if(eKind == SdrHdlKind::LowerLeft || eKind == SdrHdlKind::Lower || eKind == SdrHdlKind::LowerRight)
                    aMoveOutsideOffset.AdjustY(aOffset.Height() );
                if(eKind == SdrHdlKind::UpperLeft || eKind == SdrHdlKind::Left  || eKind == SdrHdlKind::LowerLeft)
                    aMoveOutsideOffset.AdjustX( -(aOffset.Width()) );
                if(eKind == SdrHdlKind::UpperRight || eKind == SdrHdlKind::Right || eKind == SdrHdlKind::LowerRight)
                    aMoveOutsideOffset.AdjustX(aOffset.Height() );
            }

            const rtl::Reference< sdr::overlay::OverlayManager >& xManager = rPageWindow.GetOverlayManager();
            if (xManager.is())
            {
                basegfx::B2DPoint aPosition(aPos.X(), aPos.Y());
                std::unique_ptr<sdr::overlay::OverlayObject> pNewOverlayObject;
                if (getenv ("SVX_DRAW_HANDLES") && (eKindOfMarker == BitmapMarkerKind::Rect_7x7 || eKindOfMarker == BitmapMarkerKind::Rect_9x9 || eKindOfMarker == BitmapMarkerKind::Rect_11x11))
                {
                    double fSize = 7.0;
                    switch (eKindOfMarker)
                    {
                        case BitmapMarkerKind::Rect_9x9:
                            fSize = 9.0;
                            break;
                        case BitmapMarkerKind::Rect_11x11:
                            fSize = 11.0;
                            break;
                        default:
                            break;
                    }
                    float fScalingFactor = rOutDev.GetDPIScaleFactor();
                    basegfx::B2DSize aB2DSize(fSize * fScalingFactor, fSize * fScalingFactor);

                    Color aHandleFillColor(COL_LIGHTGREEN);
                    switch (eColIndex)
                    {
                        case BitmapColorIndex::Cyan:
                            aHandleFillColor = COL_CYAN;
                            break;
                        case BitmapColorIndex::LightCyan:
                            aHandleFillColor = COL_LIGHTCYAN;
                            break;
                        case BitmapColorIndex::Red:
                            aHandleFillColor = COL_RED;
                            break;
                        case BitmapColorIndex::LightRed:
                            aHandleFillColor = COL_LIGHTRED;
                            break;
                        case BitmapColorIndex::Yellow:
                            aHandleFillColor = COL_YELLOW;
                            break;
                        default:
                            break;
                    }
                    pNewOverlayObject.reset(new sdr::overlay::OverlayHandle(aPosition, aB2DSize, /*HandleStrokeColor*/COL_BLACK, aHandleFillColor));
                }
                else
                {
                    pNewOverlayObject = CreateOverlayObject(
                                            aPosition, eColIndex, eKindOfMarker,
                                            aMoveOutsideOffset);
                }

                // OVERLAYMANAGER
                insertNewlyCreatedOverlayObjectForSdrHdl(
                    std::move(pNewOverlayObject),
                    rPageWindow.GetObjectContact(),
                    *xManager);
            }
        }
    }
}

BitmapMarkerKind SdrHdl::GetNextBigger(BitmapMarkerKind eKnd)
{
    BitmapMarkerKind eRetval(eKnd);

    switch(eKnd)
    {
        case BitmapMarkerKind::Rect_7x7:          eRetval = BitmapMarkerKind::Rect_9x9;         break;
        case BitmapMarkerKind::Rect_9x9:          eRetval = BitmapMarkerKind::Rect_11x11;       break;
        case BitmapMarkerKind::Rect_11x11:        eRetval = BitmapMarkerKind::Rect_13x13;       break;

        case BitmapMarkerKind::Circ_7x7:          eRetval = BitmapMarkerKind::Circ_9x9;         break;
        case BitmapMarkerKind::Circ_9x9:          eRetval = BitmapMarkerKind::Circ_11x11;       break;

        case BitmapMarkerKind::Customshape_7x7:       eRetval = BitmapMarkerKind::Customshape_9x9;      break;
        case BitmapMarkerKind::Customshape_9x9:       eRetval = BitmapMarkerKind::Customshape_11x11;    break;
        //case BitmapMarkerKind::Customshape_11x11:   eRetval = ; break;

        case BitmapMarkerKind::Elli_7x9:          eRetval = BitmapMarkerKind::Elli_9x11;        break;

        case BitmapMarkerKind::Elli_9x7:          eRetval = BitmapMarkerKind::Elli_11x9;        break;

        case BitmapMarkerKind::RectPlus_7x7:      eRetval = BitmapMarkerKind::RectPlus_9x9;     break;
        case BitmapMarkerKind::RectPlus_9x9:      eRetval = BitmapMarkerKind::RectPlus_11x11;   break;

        // let anchor blink with its pressed state
        case BitmapMarkerKind::Anchor:            eRetval = BitmapMarkerKind::AnchorPressed;    break;

        // same for AnchorTR
        case BitmapMarkerKind::AnchorTR:          eRetval = BitmapMarkerKind::AnchorPressedTR;  break;
        default:
            break;
    }

    return eRetval;
}

namespace
{

OUString appendMarkerName(BitmapMarkerKind eKindOfMarker)
{
    switch(eKindOfMarker)
    {
        case BitmapMarkerKind::Rect_7x7:
            return "rect7";
        case BitmapMarkerKind::Rect_9x9:
            return "rect9";
        case BitmapMarkerKind::Rect_11x11:
            return "rect11";
        case BitmapMarkerKind::Rect_13x13:
            return "rect13";
        case BitmapMarkerKind::Circ_7x7:
        case BitmapMarkerKind::Customshape_7x7:
            return "circ7";
        case BitmapMarkerKind::Circ_9x9:
        case BitmapMarkerKind::Customshape_9x9:
            return "circ9";
        case BitmapMarkerKind::Circ_11x11:
        case BitmapMarkerKind::Customshape_11x11:
            return "circ11";
        case BitmapMarkerKind::Elli_7x9:
            return "elli7x9";
        case BitmapMarkerKind::Elli_9x11:
            return "elli9x11";
        case BitmapMarkerKind::Elli_9x7:
            return "elli9x7";
        case BitmapMarkerKind::Elli_11x9:
            return "elli11x9";
        case BitmapMarkerKind::RectPlus_7x7:
            return "rectplus7";
        case BitmapMarkerKind::RectPlus_9x9:
            return "rectplus9";
        case BitmapMarkerKind::RectPlus_11x11:
            return "rectplus11";
        case BitmapMarkerKind::Crosshair:
            return "cross";
        case BitmapMarkerKind::Anchor:
        case BitmapMarkerKind::AnchorTR:
            return "anchor";
        case BitmapMarkerKind::AnchorPressed:
        case BitmapMarkerKind::AnchorPressedTR:
            return "anchor-pressed";
        case BitmapMarkerKind::Glue:
            return "glue-selected";
        case BitmapMarkerKind::Glue_Deselected:
            return "glue-unselected";
        default:
            break;
    }
    return OUString();
}

OUString appendMarkerColor(BitmapColorIndex eIndex)
{
    switch(eIndex)
    {
        case BitmapColorIndex::LightGreen:
            return "1";
        case BitmapColorIndex::Cyan:
            return "2";
        case BitmapColorIndex::LightCyan:
            return "3";
        case BitmapColorIndex::Red:
            return "4";
        case BitmapColorIndex::LightRed:
            return "5";
        case BitmapColorIndex::Yellow:
            return "6";
        default:
            break;
    }
    return OUString();
}

BitmapEx ImpGetBitmapEx(BitmapMarkerKind eKindOfMarker, BitmapColorIndex eIndex)
{
    // use this code path only when we use HiDPI (for now)
    if (Application::GetDefaultDevice()->GetDPIScalePercentage() > 100)
    {
        OUString sMarkerName = appendMarkerName(eKindOfMarker);
        if (!sMarkerName.isEmpty())
        {
            OUString sMarkerPrefix("svx/res/marker-");
            BitmapEx aBitmapEx;

            if (eKindOfMarker == BitmapMarkerKind::Crosshair
             || eKindOfMarker == BitmapMarkerKind::Anchor
             || eKindOfMarker == BitmapMarkerKind::AnchorTR
             || eKindOfMarker == BitmapMarkerKind::AnchorPressed
             || eKindOfMarker == BitmapMarkerKind::AnchorPressedTR
             || eKindOfMarker == BitmapMarkerKind::Glue
             || eKindOfMarker == BitmapMarkerKind::Glue_Deselected)
            {
                aBitmapEx = vcl::bitmap::loadFromName(sMarkerPrefix + sMarkerName + ".png");
            }
            else
            {
                aBitmapEx = vcl::bitmap::loadFromName(sMarkerPrefix + sMarkerName + "-" + appendMarkerColor(eIndex) + ".png");
            }

            if (!aBitmapEx.IsEmpty())
                return aBitmapEx;
        }
    }

    // if we can't load the marker...

    static vcl::DeleteOnDeinit< SdrHdlBitmapSet > aModernSet {};
    return aModernSet.get()->GetBitmapEx(eKindOfMarker, sal_uInt16(eIndex));
}

} // end anonymous namespace

std::unique_ptr<sdr::overlay::OverlayObject> SdrHdl::CreateOverlayObject(
    const basegfx::B2DPoint& rPos,
    BitmapColorIndex eColIndex, BitmapMarkerKind eKindOfMarker, Point aMoveOutsideOffset)
{
    std::unique_ptr<sdr::overlay::OverlayObject> pRetval;

    // support bigger sizes
    bool bForceBiggerSize(false);

    if (pHdlList && pHdlList->GetHdlSize() > 3)
    {
        switch(eKindOfMarker)
        {
            case BitmapMarkerKind::Anchor:
            case BitmapMarkerKind::AnchorPressed:
            case BitmapMarkerKind::AnchorTR:
            case BitmapMarkerKind::AnchorPressedTR:
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
                case BitmapMarkerKind::Rect_13x13:        eNextBigger = BitmapMarkerKind::Rect_11x11;   break;
                case BitmapMarkerKind::Circ_11x11:        eNextBigger = BitmapMarkerKind::Elli_11x9;    break;
                case BitmapMarkerKind::Elli_9x11:         eNextBigger = BitmapMarkerKind::Elli_11x9;    break;
                case BitmapMarkerKind::Elli_11x9:         eNextBigger = BitmapMarkerKind::Elli_9x11;    break;
                case BitmapMarkerKind::RectPlus_11x11:    eNextBigger = BitmapMarkerKind::Rect_13x13;   break;

                case BitmapMarkerKind::Crosshair:
                    eNextBigger = BitmapMarkerKind::Glue;
                    break;

                case BitmapMarkerKind::Glue:
                    eNextBigger = BitmapMarkerKind::Crosshair;
                    break;
                case BitmapMarkerKind::Glue_Deselected:
                    eNextBigger = BitmapMarkerKind::Glue;
                    break;
                default:
                    break;
            }
        }

        // create animated handle
        BitmapEx aBmpEx1 = ImpGetBitmapEx(eKindOfMarker, eColIndex);
        BitmapEx aBmpEx2 = ImpGetBitmapEx(eNextBigger,   eColIndex);

        // #i53216# Use system cursor blink time. Use the unsigned value.
        const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
        const sal_uInt64 nBlinkTime(rStyleSettings.GetCursorBlinkTime());

        if(eKindOfMarker == BitmapMarkerKind::Anchor || eKindOfMarker == BitmapMarkerKind::AnchorPressed)
        {
            // when anchor is used take upper left as reference point inside the handle
            pRetval.reset(new sdr::overlay::OverlayAnimatedBitmapEx(rPos, aBmpEx1, aBmpEx2, nBlinkTime));
        }
        else if(eKindOfMarker == BitmapMarkerKind::AnchorTR || eKindOfMarker == BitmapMarkerKind::AnchorPressedTR)
        {
            // AnchorTR for SW, take top right as (0,0)
            pRetval.reset(new sdr::overlay::OverlayAnimatedBitmapEx(rPos, aBmpEx1, aBmpEx2, nBlinkTime,
                static_cast<sal_uInt16>(aBmpEx1.GetSizePixel().Width() - 1), 0,
                static_cast<sal_uInt16>(aBmpEx2.GetSizePixel().Width() - 1), 0));
        }
        else
        {
            // create centered handle as default
            pRetval.reset(new sdr::overlay::OverlayAnimatedBitmapEx(rPos, aBmpEx1, aBmpEx2, nBlinkTime,
                static_cast<sal_uInt16>(aBmpEx1.GetSizePixel().Width() - 1) >> 1,
                static_cast<sal_uInt16>(aBmpEx1.GetSizePixel().Height() - 1) >> 1,
                static_cast<sal_uInt16>(aBmpEx2.GetSizePixel().Width() - 1) >> 1,
                static_cast<sal_uInt16>(aBmpEx2.GetSizePixel().Height() - 1) >> 1));
        }
    }
    else
    {
        // create normal handle: use ImpGetBitmapEx(...) now
        BitmapEx aBmpEx = ImpGetBitmapEx(eKindOfMarker, eColIndex);

        // When the image with handles is not found, the bitmap returned is
        // empty. This is a problem when we use LibreOffice as a library
        // (through LOKit - for example on Android) even when we don't show
        // the handles, because the hit test would always return false.
        //
        // This HACK replaces the empty bitmap with a black 13x13 bitmap handle
        // so that the hit test works for this case.
        if (aBmpEx.IsEmpty())
        {
            aBmpEx = BitmapEx(Size(13, 13), vcl::PixelFormat::N24_BPP);
            aBmpEx.Erase(COL_BLACK);
        }

        if(eKindOfMarker == BitmapMarkerKind::Anchor || eKindOfMarker == BitmapMarkerKind::AnchorPressed)
        {
            // upper left as reference point inside the handle for AnchorPressed, too
            pRetval.reset(new sdr::overlay::OverlayBitmapEx(rPos, aBmpEx));
        }
        else if(eKindOfMarker == BitmapMarkerKind::AnchorTR || eKindOfMarker == BitmapMarkerKind::AnchorPressedTR)
        {
            // AnchorTR for SW, take top right as (0,0)
            pRetval.reset(new sdr::overlay::OverlayBitmapEx(rPos, aBmpEx,
                static_cast<sal_uInt16>(aBmpEx.GetSizePixel().Width() - 1), 0));
        }
        else
        {
            sal_uInt16 nCenX(static_cast<sal_uInt16>(aBmpEx.GetSizePixel().Width() - 1) >> 1);
            sal_uInt16 nCenY(static_cast<sal_uInt16>(aBmpEx.GetSizePixel().Height() - 1) >> 1);

            if(aMoveOutsideOffset.X() > 0)
            {
                nCenX = 0;
            }
            else if(aMoveOutsideOffset.X() < 0)
            {
                nCenX = static_cast<sal_uInt16>(aBmpEx.GetSizePixel().Width() - 1);
            }

            if(aMoveOutsideOffset.Y() > 0)
            {
                nCenY = 0;
            }
            else if(aMoveOutsideOffset.Y() < 0)
            {
                nCenY = static_cast<sal_uInt16>(aBmpEx.GetSizePixel().Height() - 1);
            }

            // create centered handle as default
            pRetval.reset(new sdr::overlay::OverlayBitmapEx(rPos, aBmpEx, nCenX, nCenY));
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

PointerStyle SdrHdl::GetPointer() const
{
    PointerStyle ePtr=PointerStyle::Move;
    const bool bSize=eKind>=SdrHdlKind::UpperLeft && eKind<=SdrHdlKind::LowerRight;
    const bool bRot=pHdlList!=nullptr && pHdlList->IsRotateShear();
    const bool bDis=pHdlList!=nullptr && pHdlList->IsDistortShear();
    if (bSize && pHdlList!=nullptr && (bRot || bDis)) {
        switch (eKind) {
            case SdrHdlKind::UpperLeft: case SdrHdlKind::UpperRight:
            case SdrHdlKind::LowerLeft: case SdrHdlKind::LowerRight: ePtr=bRot ? PointerStyle::Rotate : PointerStyle::RefHand; break;
            case SdrHdlKind::Left : case SdrHdlKind::Right: ePtr=PointerStyle::VShear; break;
            case SdrHdlKind::Upper: case SdrHdlKind::Lower: ePtr=PointerStyle::HShear; break;
            default:
                break;
        }
    } else {
        // When resizing rotated rectangles, rotate the mouse cursor slightly, too
        if (bSize && nRotationAngle!=0_deg100) {
            Degree100 nHdlAngle(0);
            switch (eKind) {
                case SdrHdlKind::LowerRight: nHdlAngle=31500_deg100; break;
                case SdrHdlKind::Lower: nHdlAngle=27000_deg100; break;
                case SdrHdlKind::LowerLeft: nHdlAngle=22500_deg100; break;
                case SdrHdlKind::Left : nHdlAngle=18000_deg100; break;
                case SdrHdlKind::UpperLeft: nHdlAngle=13500_deg100; break;
                case SdrHdlKind::Upper: nHdlAngle=9000_deg100;  break;
                case SdrHdlKind::UpperRight: nHdlAngle=4500_deg100;  break;
                case SdrHdlKind::Right: nHdlAngle=0_deg100;     break;
                default:
                    break;
            }
            // a little bit more (for rounding)
            nHdlAngle = NormAngle36000(nHdlAngle + nRotationAngle + 2249_deg100);
            nHdlAngle/=4500_deg100;
            switch (static_cast<sal_uInt8>(nHdlAngle.get())) {
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
                case SdrHdlKind::UpperLeft: ePtr=PointerStyle::NWSize;  break;
                case SdrHdlKind::Upper: ePtr=PointerStyle::NSize;     break;
                case SdrHdlKind::UpperRight: ePtr=PointerStyle::NESize;  break;
                case SdrHdlKind::Left : ePtr=PointerStyle::WSize;     break;
                case SdrHdlKind::Right: ePtr=PointerStyle::ESize;     break;
                case SdrHdlKind::LowerLeft: ePtr=PointerStyle::SWSize;  break;
                case SdrHdlKind::Lower: ePtr=PointerStyle::SSize;     break;
                case SdrHdlKind::LowerRight: ePtr=PointerStyle::SESize;  break;
                case SdrHdlKind::Poly : ePtr=PointerStyle::MovePoint; break;
                case SdrHdlKind::Circle : ePtr=PointerStyle::Hand;      break;
                case SdrHdlKind::Ref1 : ePtr=PointerStyle::RefHand;   break;
                case SdrHdlKind::Ref2 : ePtr=PointerStyle::RefHand;   break;
                case SdrHdlKind::BezierWeight : ePtr=PointerStyle::MoveBezierWeight; break;
                case SdrHdlKind::Glue : ePtr=PointerStyle::MovePoint; break;
                case SdrHdlKind::CustomShape1 : ePtr=PointerStyle::Hand; break;
                default:
                    break;
            }
        }
    }
    return ePtr;
}

bool SdrHdl::IsFocusHdl() const
{
    switch(eKind)
    {
        case SdrHdlKind::UpperLeft:
        case SdrHdlKind::Upper:
        case SdrHdlKind::UpperRight:
        case SdrHdlKind::Left:
        case SdrHdlKind::Right:
        case SdrHdlKind::LowerLeft:
        case SdrHdlKind::Lower:
        case SdrHdlKind::LowerRight:
        {
            // if it's an activated TextEdit, it's moved to extended points
            return !pHdlList || !pHdlList->IsMoveOutside();
        }

        case SdrHdlKind::Move:      // handle to move object
        case SdrHdlKind::Poly:      // selected point of polygon or curve
        case SdrHdlKind::BezierWeight:      // weight at a curve
        case SdrHdlKind::Circle:      // angle of circle segments, corner radius of rectangles
        case SdrHdlKind::Ref1:      // reference point 1, e. g. center of rotation
        case SdrHdlKind::Ref2:      // reference point 2, e. g. endpoint of reflection axis
        case SdrHdlKind::Glue:      // gluepoint

        // for SJ and the CustomShapeHandles:
        case SdrHdlKind::CustomShape1:

        case SdrHdlKind::User:
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

void SdrHdl::onHelpRequest()
{
}

void SdrHdl::onMouseLeave()
{
}

BitmapEx SdrHdl::createGluePointBitmap()
{
    return ImpGetBitmapEx(BitmapMarkerKind::Glue_Deselected, BitmapColorIndex::LightGreen);
}

void SdrHdl::insertNewlyCreatedOverlayObjectForSdrHdl(
    std::unique_ptr<sdr::overlay::OverlayObject> pOverlayObject,
    const sdr::contact::ObjectContact& rObjectContact,
    sdr::overlay::OverlayManager& rOverlayManager)
{
    // check if we have an OverlayObject
    if(!pOverlayObject)
    {
        return;
    }

    // Add GridOffset for non-linear ViewToDevice transformation (calc)
    if(nullptr != GetObj() && rObjectContact.supportsGridOffsets())
    {
        basegfx::B2DVector aOffset(0.0, 0.0);
        const sdr::contact::ViewObjectContact& rVOC(GetObj()->GetViewContact().GetViewObjectContact(
            const_cast<sdr::contact::ObjectContact&>(rObjectContact)));

        rObjectContact.calculateGridOffsetForViewOjectContact(aOffset, rVOC);

        if(!aOffset.equalZero())
        {
            pOverlayObject->setOffset(aOffset);
        }
    }

    // add to OverlayManager
    rOverlayManager.add(*pOverlayObject);

    // add to local OverlayObjectList - ownership change (!)
    maOverlayGroup.append(std::move(pOverlayObject));
}

SdrHdlColor::SdrHdlColor(const Point& rRef, Color aCol, const Size& rSize, bool bLum)
:   SdrHdl(rRef, SdrHdlKind::Color),
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

    if(!pHdlList)
        return;

    SdrMarkView* pView = pHdlList->GetView();

    if(!pView || pView->areMarkHandlesHidden())
        return;

    SdrPageView* pPageView = pView->GetSdrPageView();

    if(!pPageView)
        return;

    for(sal_uInt32 b(0); b < pPageView->PageWindowCount(); b++)
    {
        const SdrPageWindow& rPageWindow = *pPageView->GetPageWindow(b);

        if(rPageWindow.GetPaintWindow().OutputToWindow())
        {
            const rtl::Reference< sdr::overlay::OverlayManager >& xManager = rPageWindow.GetOverlayManager();
            if (xManager.is())
            {
                BitmapEx aBmpCol(CreateColorDropper(aMarkerColor));
                basegfx::B2DPoint aPosition(aPos.X(), aPos.Y());
                std::unique_ptr<sdr::overlay::OverlayObject> pNewOverlayObject(new
                    sdr::overlay::OverlayBitmapEx(
                        aPosition,
                        aBmpCol,
                        static_cast<sal_uInt16>(aBmpCol.GetSizePixel().Width() - 1) >> 1,
                        static_cast<sal_uInt16>(aBmpCol.GetSizePixel().Height() - 1) >> 1
                    ));

                // OVERLAYMANAGER
                insertNewlyCreatedOverlayObjectForSdrHdl(
                    std::move(pNewOverlayObject),
                    rPageWindow.GetObjectContact(),
                    *xManager);
            }
        }
    }
}

BitmapEx SdrHdlColor::CreateColorDropper(Color aCol)
{
    // get the Bitmap
    VclPtr<VirtualDevice> pWrite(VclPtr<VirtualDevice>::Create());
    pWrite->SetOutputSizePixel(aMarkerSize);
    pWrite->SetBackground(aCol);
    pWrite->Erase();

    // draw outer border
    sal_Int32 nWidth = aMarkerSize.Width();
    sal_Int32 nHeight = aMarkerSize.Height();

    pWrite->SetLineColor(COL_LIGHTGRAY);
    pWrite->DrawLine(Point(0, 0), Point(0, nHeight - 1));
    pWrite->DrawLine(Point(1, 0), Point(nWidth - 1, 0));
    pWrite->SetLineColor(COL_GRAY);
    pWrite->DrawLine(Point(1, nHeight - 1), Point(nWidth - 1, nHeight - 1));
    pWrite->DrawLine(Point(nWidth - 1, 1), Point(nWidth - 1, nHeight - 2));

    // draw lighter UpperLeft
    const Color aLightColor(
        static_cast<sal_uInt8>(::std::min(static_cast<sal_Int16>(static_cast<sal_Int16>(aCol.GetRed()) + sal_Int16(0x0040)), sal_Int16(0x00ff))),
        static_cast<sal_uInt8>(::std::min(static_cast<sal_Int16>(static_cast<sal_Int16>(aCol.GetGreen()) + sal_Int16(0x0040)), sal_Int16(0x00ff))),
        static_cast<sal_uInt8>(::std::min(static_cast<sal_Int16>(static_cast<sal_Int16>(aCol.GetBlue()) + sal_Int16(0x0040)), sal_Int16(0x00ff))));
    pWrite->SetLineColor(aLightColor);
    pWrite->DrawLine(Point(1, 1), Point(1, nHeight - 2));
    pWrite->DrawLine(Point(2, 1), Point(nWidth - 2, 1));

    // draw darker LowerRight
    const Color aDarkColor(
        static_cast<sal_uInt8>(::std::max(static_cast<sal_Int16>(static_cast<sal_Int16>(aCol.GetRed()) - sal_Int16(0x0040)), sal_Int16(0x0000))),
        static_cast<sal_uInt8>(::std::max(static_cast<sal_Int16>(static_cast<sal_Int16>(aCol.GetGreen()) - sal_Int16(0x0040)), sal_Int16(0x0000))),
        static_cast<sal_uInt8>(::std::max(static_cast<sal_Int16>(static_cast<sal_Int16>(aCol.GetBlue()) - sal_Int16(0x0040)), sal_Int16(0x0000))));
    pWrite->SetLineColor(aDarkColor);
    pWrite->DrawLine(Point(2, nHeight - 2), Point(nWidth - 2, nHeight - 2));
    pWrite->DrawLine(Point(nWidth - 2, 2), Point(nWidth - 2, nHeight - 3));

    return pWrite->GetBitmapEx(Point(0,0), aMarkerSize);
}

Color SdrHdlColor::GetLuminance(const Color& rCol)
{
    sal_uInt8 aLum = rCol.GetLuminance();
    Color aRetval(aLum, aLum, aLum);
    return aRetval;
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
            aColorChangeHdl.Call(this);
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
    : SdrHdl(rRef1, bGrad ? SdrHdlKind::Gradient : SdrHdlKind::Transparence)
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

    if(!pHdlList)
        return;

    SdrMarkView* pView = pHdlList->GetView();

    if(!pView || pView->areMarkHandlesHidden())
        return;

    SdrPageView* pPageView = pView->GetSdrPageView();

    if(!pPageView)
        return;

    for(sal_uInt32 b(0); b < pPageView->PageWindowCount(); b++)
    {
        const SdrPageWindow& rPageWindow = *pPageView->GetPageWindow(b);

        if(rPageWindow.GetPaintWindow().OutputToWindow())
        {
            const rtl::Reference< sdr::overlay::OverlayManager >& xManager = rPageWindow.GetOverlayManager();
            if (xManager.is())
            {
                // striped line in between
                basegfx::B2DVector aVec(a2ndPos.X() - aPos.X(), a2ndPos.Y() - aPos.Y());
                double fVecLen = aVec.getLength();
                double fLongPercentArrow = (1.0 - 0.05) * fVecLen;
                double fHalfArrowWidth = (0.05 * 0.5) * fVecLen;
                aVec.normalize();
                basegfx::B2DVector aPerpend(-aVec.getY(), aVec.getX());
                sal_Int32 nMidX = static_cast<sal_Int32>(aPos.X() + aVec.getX() * fLongPercentArrow);
                sal_Int32 nMidY = static_cast<sal_Int32>(aPos.Y() + aVec.getY() * fLongPercentArrow);
                Point aMidPoint(nMidX, nMidY);

                basegfx::B2DPoint aPosition(aPos.X(), aPos.Y());
                basegfx::B2DPoint aMidPos(aMidPoint.X(), aMidPoint.Y());

                std::unique_ptr<sdr::overlay::OverlayObject> pNewOverlayObject(new
                    sdr::overlay::OverlayLineStriped(
                        aPosition, aMidPos
                    ));

                pNewOverlayObject->setBaseColor(IsGradient() ? COL_BLACK : COL_BLUE);

                // OVERLAYMANAGER
                insertNewlyCreatedOverlayObjectForSdrHdl(
                    std::move(pNewOverlayObject),
                    rPageWindow.GetObjectContact(),
                    *xManager);

                // arrowhead
                Point aLeft(aMidPoint.X() + static_cast<sal_Int32>(aPerpend.getX() * fHalfArrowWidth),
                            aMidPoint.Y() + static_cast<sal_Int32>(aPerpend.getY() * fHalfArrowWidth));
                Point aRight(aMidPoint.X() - static_cast<sal_Int32>(aPerpend.getX() * fHalfArrowWidth),
                            aMidPoint.Y() - static_cast<sal_Int32>(aPerpend.getY() * fHalfArrowWidth));

                basegfx::B2DPoint aPositionLeft(aLeft.X(), aLeft.Y());
                basegfx::B2DPoint aPositionRight(aRight.X(), aRight.Y());
                basegfx::B2DPoint aPosition2(a2ndPos.X(), a2ndPos.Y());

                pNewOverlayObject.reset(new
                    sdr::overlay::OverlayTriangle(
                        aPositionLeft,
                        aPosition2,
                        aPositionRight,
                        IsGradient() ? COL_BLACK : COL_BLUE
                    ));

                // OVERLAYMANAGER
                insertNewlyCreatedOverlayObjectForSdrHdl(
                    std::move(pNewOverlayObject),
                    rPageWindow.GetObjectContact(),
                    *xManager);
            }
        }
    }
}

IMPL_LINK_NOARG(SdrHdlGradient, ColorChangeHdl, SdrHdlColor*, void)
{
    if(GetObj())
        FromIAOToItem(GetObj(), true, true);
}

void SdrHdlGradient::FromIAOToItem(SdrObject* _pObj, bool bSetItemOnObject, bool bUndo)
{
    // from IAO positions and colors to gradient
    const SfxItemSet& rSet = _pObj->GetMergedItemSet();

    GradTransGradient aOldGradTransGradient;
    GradTransGradient aGradTransGradient;
    GradTransVector aGradTransVector;

    aGradTransVector.maPositionA = basegfx::B2DPoint(GetPos().X(), GetPos().Y());
    aGradTransVector.maPositionB = basegfx::B2DPoint(Get2ndPos().X(), Get2ndPos().Y());
    if(pColHdl1)
        aGradTransVector.aCol1 = pColHdl1->GetColor();
    if(pColHdl2)
        aGradTransVector.aCol2 = pColHdl2->GetColor();

    if(IsGradient())
        aOldGradTransGradient.aGradient = rSet.Get(XATTR_FILLGRADIENT).GetGradientValue();
    else
        aOldGradTransGradient.aGradient = rSet.Get(XATTR_FILLFLOATTRANSPARENCE).GetGradientValue();

    // transform vector data to gradient
    GradTransformer::VecToGrad(aGradTransVector, aGradTransGradient, aOldGradTransGradient, _pObj, bMoveSingleHandle, bMoveFirstHandle);

    if(bSetItemOnObject)
    {
        SdrModel& rModel(_pObj->getSdrModelFromSdrObject());
        SfxItemSet aNewSet(rModel.GetItemPool());
        const OUString aString;

        if(IsGradient())
        {
            XFillGradientItem aNewGradItem(aString, aGradTransGradient.aGradient);
            aNewSet.Put(aNewGradItem);
        }
        else
        {
            XFillFloatTransparenceItem aNewTransItem(aString, aGradTransGradient.aGradient);
            aNewSet.Put(aNewTransItem);
        }

        if(bUndo && rModel.IsUndoEnabled())
        {
            rModel.BegUndo(SvxResId(IsGradient() ? SIP_XA_FILLGRADIENT : SIP_XA_FILLTRANSPARENCE));
            rModel.AddUndo(rModel.GetSdrUndoFactory().CreateUndoAttrObject(*_pObj));
            rModel.EndUndo();
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

    if(!pHdlList)
        return;

    SdrMarkView* pView = pHdlList->GetView();

    if(!(pView && !pView->areMarkHandlesHidden() && pHdl1 && pHdl2))
        return;

    SdrPageView* pPageView = pView->GetSdrPageView();

    if(!pPageView)
        return;

    for(sal_uInt32 b(0); b < pPageView->PageWindowCount(); b++)
    {
        const SdrPageWindow& rPageWindow = *pPageView->GetPageWindow(b);

        if(rPageWindow.GetPaintWindow().OutputToWindow())
        {
            const rtl::Reference< sdr::overlay::OverlayManager >& xManager = rPageWindow.GetOverlayManager();
            if (xManager.is())
            {
                basegfx::B2DPoint aPosition1(pHdl1->GetPos().X(), pHdl1->GetPos().Y());
                basegfx::B2DPoint aPosition2(pHdl2->GetPos().X(), pHdl2->GetPos().Y());

                std::unique_ptr<sdr::overlay::OverlayObject> pNewOverlayObject(new
                    sdr::overlay::OverlayLineStriped(
                        aPosition1,
                        aPosition2
                    ));

                // color(?)
                pNewOverlayObject->setBaseColor(COL_LIGHTRED);

                // OVERLAYMANAGER
                insertNewlyCreatedOverlayObjectForSdrHdl(
                    std::move(pNewOverlayObject),
                    rPageWindow.GetObjectContact(),
                    *xManager);
            }
        }
    }
}

PointerStyle SdrHdlLine::GetPointer() const
{
    return PointerStyle::RefHand;
}


SdrHdlBezWgt::~SdrHdlBezWgt() {}

void SdrHdlBezWgt::CreateB2dIAObject()
{
    // call parent
    SdrHdl::CreateB2dIAObject();

    // create lines
    if(!pHdlList)
        return;

    SdrMarkView* pView = pHdlList->GetView();

    if(!pView || pView->areMarkHandlesHidden())
        return;

    SdrPageView* pPageView = pView->GetSdrPageView();

    if(!pPageView)
        return;

    for(sal_uInt32 b(0); b < pPageView->PageWindowCount(); b++)
    {
        const SdrPageWindow& rPageWindow = *pPageView->GetPageWindow(b);

        if(rPageWindow.GetPaintWindow().OutputToWindow())
        {
            const rtl::Reference< sdr::overlay::OverlayManager >& xManager = rPageWindow.GetOverlayManager();
            if (xManager.is())
            {
                basegfx::B2DPoint aPosition1(pHdl1->GetPos().X(), pHdl1->GetPos().Y());
                basegfx::B2DPoint aPosition2(aPos.X(), aPos.Y());

                if(!aPosition1.equal(aPosition2))
                {
                    std::unique_ptr<sdr::overlay::OverlayObject> pNewOverlayObject(new
                        sdr::overlay::OverlayLineStriped(
                            aPosition1,
                            aPosition2
                        ));

                    // line part is not hittable
                    pNewOverlayObject->setHittable(false);

                    // color(?)
                    pNewOverlayObject->setBaseColor(COL_LIGHTBLUE);

                    // OVERLAYMANAGER
                    insertNewlyCreatedOverlayObjectForSdrHdl(
                        std::move(pNewOverlayObject),
                        rPageWindow.GetObjectContact(),
                        *xManager);
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
    if(!pHdlList)
        return;

    SdrMarkView* pView = pHdlList->GetView();

    if(!pView || pView->areMarkHandlesHidden())
        return;

    SdrPageView* pPageView = pView->GetSdrPageView();

    if(!pPageView)
        return;

    for(sal_uInt32 b(0); b < pPageView->PageWindowCount(); b++)
    {
        const SdrPageWindow& rPageWindow = *pPageView->GetPageWindow(b);

        if(rPageWindow.GetPaintWindow().OutputToWindow())
        {
            const rtl::Reference< sdr::overlay::OverlayManager >& xManager = rPageWindow.GetOverlayManager();
            if (xManager.is() && aWireframePoly.count())
            {
                std::unique_ptr<sdr::overlay::OverlayObject> pNewOverlayObject(new
                    sdr::overlay::OverlayPolyPolygonStripedAndFilled(
                        aWireframePoly));

                pNewOverlayObject->setBaseColor(COL_BLACK);

                // OVERLAYMANAGER
                insertNewlyCreatedOverlayObjectForSdrHdl(
                    std::move(pNewOverlayObject),
                    rPageWindow.GetObjectContact(),
                    *xManager);
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

        BitmapColorIndex eColIndex = BitmapColorIndex::LightCyan;
        BitmapMarkerKind eKindOfMarker = BitmapMarkerKind::Rect_7x7;

        if(pHdlList)
        {
            SdrMarkView* pView = pHdlList->GetView();

            if(pView && !pView->areMarkHandlesHidden())
            {
                const SdrEdgeObj* pEdge = static_cast<SdrEdgeObj*>(pObj);

                if(pEdge->GetConnectedNode(nObjHdlNum == 0) != nullptr)
                    eColIndex = BitmapColorIndex::LightRed;

                if(nPPntNum < 2)
                {
                    // Handle with plus sign inside
                    eKindOfMarker = BitmapMarkerKind::Circ_7x7;
                }

                SdrPageView* pPageView = pView->GetSdrPageView();

                if(pPageView)
                {
                    for(sal_uInt32 b(0); b < pPageView->PageWindowCount(); b++)
                    {
                        const SdrPageWindow& rPageWindow = *pPageView->GetPageWindow(b);

                        if(rPageWindow.GetPaintWindow().OutputToWindow())
                        {
                            const rtl::Reference< sdr::overlay::OverlayManager >& xManager = rPageWindow.GetOverlayManager();
                            if (xManager.is())
                            {
                                basegfx::B2DPoint aPosition(aPos.X(), aPos.Y());
                                std::unique_ptr<sdr::overlay::OverlayObject> pNewOverlayObject(CreateOverlayObject(
                                    aPosition,
                                    eColIndex,
                                    eKindOfMarker));

                                // OVERLAYMANAGER
                                insertNewlyCreatedOverlayObjectForSdrHdl(
                                    std::move(pNewOverlayObject),
                                    rPageWindow.GetObjectContact(),
                                    *xManager);
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

PointerStyle ImpEdgeHdl::GetPointer() const
{
    SdrEdgeObj* pEdge=dynamic_cast<SdrEdgeObj*>( pObj );
    if (pEdge==nullptr)
        return SdrHdl::GetPointer();
    if (nObjHdlNum<=1)
        return PointerStyle::MovePoint;
    if (IsHorzDrag())
        return PointerStyle::ESize;
    else
        return PointerStyle::SSize;
}

bool ImpEdgeHdl::IsHorzDrag() const
{
    SdrEdgeObj* pEdge=dynamic_cast<SdrEdgeObj*>( pObj );
    if (pEdge==nullptr)
        return false;
    if (nObjHdlNum<=1)
        return false;

    SdrEdgeKind eEdgeKind = pEdge->GetObjectItem(SDRATTR_EDGEKIND).GetValue();

    const SdrEdgeInfoRec& rInfo=pEdge->aEdgeInfo;
    if (eEdgeKind==SdrEdgeKind::OrthoLines || eEdgeKind==SdrEdgeKind::Bezier)
    {
        return !rInfo.ImpIsHorzLine(eLineCode,*pEdge->pEdgeTrack);
    }
    else if (eEdgeKind==SdrEdgeKind::ThreeLines)
    {
        tools::Long nAngle=nObjHdlNum==2 ? rInfo.nAngle1 : rInfo.nAngle2;
        return nAngle==0 || nAngle==18000;
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

    if(!pHdlList)
        return;

    SdrMarkView* pView = pHdlList->GetView();

    if(!pView || pView->areMarkHandlesHidden())
        return;

    BitmapColorIndex eColIndex = BitmapColorIndex::LightCyan;
    BitmapMarkerKind eKindOfMarker = BitmapMarkerKind::Rect_9x9;

    if(nObjHdlNum > 1)
    {
        eKindOfMarker = BitmapMarkerKind::Rect_7x7;
    }

    if(bSelect)
    {
        eColIndex = BitmapColorIndex::Cyan;
    }

    SdrPageView* pPageView = pView->GetSdrPageView();

    if(!pPageView)
        return;

    for(sal_uInt32 b(0); b < pPageView->PageWindowCount(); b++)
    {
        const SdrPageWindow& rPageWindow = *pPageView->GetPageWindow(b);

        if(rPageWindow.GetPaintWindow().OutputToWindow())
        {
            const rtl::Reference< sdr::overlay::OverlayManager >& xManager = rPageWindow.GetOverlayManager();
            if (xManager.is())
            {
                basegfx::B2DPoint aPosition(aPos.X(), aPos.Y());
                std::unique_ptr<sdr::overlay::OverlayObject> pNewOverlayObject(CreateOverlayObject(
                    aPosition,
                    eColIndex,
                    eKindOfMarker));

                // OVERLAYMANAGER
                insertNewlyCreatedOverlayObjectForSdrHdl(
                    std::move(pNewOverlayObject),
                    rPageWindow.GetObjectContact(),
                    *xManager);
            }
        }
    }
}

PointerStyle ImpMeasureHdl::GetPointer() const
{
    switch (nObjHdlNum)
    {
        case 0: case 1: return PointerStyle::Hand;
        case 2: case 3: return PointerStyle::MovePoint;
        case 4: case 5: return SdrHdl::GetPointer(); // will then be rotated appropriately
    } // switch
    return PointerStyle::NotAllowed;
}


ImpTextframeHdl::ImpTextframeHdl(const tools::Rectangle& rRect) :
    SdrHdl(rRect.TopLeft(),SdrHdlKind::Move),
    maRect(rRect)
{
}

void ImpTextframeHdl::CreateB2dIAObject()
{
    // first throw away old one
    GetRidOfIAObject();

    if(!pHdlList)
        return;

    SdrMarkView* pView = pHdlList->GetView();

    if(!pView || pView->areMarkHandlesHidden())
        return;

    SdrPageView* pPageView = pView->GetSdrPageView();

    if(!pPageView)
        return;

    for(sal_uInt32 b(0); b < pPageView->PageWindowCount(); b++)
    {
        const SdrPageWindow& rPageWindow = *pPageView->GetPageWindow(b);

        if(rPageWindow.GetPaintWindow().OutputToWindow())
        {
            const rtl::Reference< sdr::overlay::OverlayManager >& xManager = rPageWindow.GetOverlayManager();
            if (xManager.is())
            {
                const basegfx::B2DPoint aTopLeft(maRect.Left(), maRect.Top());
                const basegfx::B2DPoint aBottomRight(maRect.Right(), maRect.Bottom());
                const Color aHilightColor(SvtOptionsDrawinglayer::getHilightColor());
                const double fTransparence(SvtOptionsDrawinglayer::GetTransparentSelectionPercent() * 0.01);

                std::unique_ptr<sdr::overlay::OverlayRectangle> pNewOverlayObject(new sdr::overlay::OverlayRectangle(
                    aTopLeft,
                    aBottomRight,
                    aHilightColor,
                    fTransparence,
                    3.0,
                    3.0,
                    -toRadians(nRotationAngle),
                    true)); // allow animation; the Handle is not shown at text edit time

                pNewOverlayObject->setHittable(false);

                // OVERLAYMANAGER
                insertNewlyCreatedOverlayObjectForSdrHdl(
                    std::move(pNewOverlayObject),
                    rPageWindow.GetObjectContact(),
                    *xManager);
            }
        }
    }
}


static bool ImpSdrHdlListSorter(std::unique_ptr<SdrHdl> const& lhs, std::unique_ptr<SdrHdl> const& rhs)
{
    SdrHdlKind eKind1=lhs->GetKind();
    SdrHdlKind eKind2=rhs->GetKind();
    // Level 1: first normal handles, then Glue, then User, then Plus handles, then reference point handles
    unsigned n1=1;
    unsigned n2=1;
    if (eKind1!=eKind2)
    {
        if (eKind1==SdrHdlKind::Ref1 || eKind1==SdrHdlKind::Ref2 || eKind1==SdrHdlKind::MirrorAxis) n1=5;
        else if (eKind1==SdrHdlKind::Glue) n1=2;
        else if (eKind1==SdrHdlKind::User) n1=3;
        else if (eKind1==SdrHdlKind::SmartTag) n1=0;
        if (eKind2==SdrHdlKind::Ref1 || eKind2==SdrHdlKind::Ref2 || eKind2==SdrHdlKind::MirrorAxis) n2=5;
        else if (eKind2==SdrHdlKind::Glue) n2=2;
        else if (eKind2==SdrHdlKind::User) n2=3;
        else if (eKind2==SdrHdlKind::SmartTag) n2=0;
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
                    return static_cast<sal_uInt16>(eKind1)<static_cast<sal_uInt16>(eKind2);
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

namespace {

// Helper struct for re-sorting handles
struct ImplHdlAndIndex
{
    SdrHdl*                     mpHdl;
    sal_uInt32                  mnIndex;
};

}

extern "C" {

// Helper method for sorting handles taking care of OrdNums, keeping order in
// single objects and re-sorting polygon handles intuitively
static int ImplSortHdlFunc( const void* pVoid1, const void* pVoid2 )
{
    const ImplHdlAndIndex* p1 = static_cast<ImplHdlAndIndex const *>(pVoid1);
    const ImplHdlAndIndex* p2 = static_cast<ImplHdlAndIndex const *>(pVoid2);

    if(p1->mpHdl->GetObj() == p2->mpHdl->GetObj())
    {
        if(p1->mpHdl->GetObj() && dynamic_cast<const SdrPathObj*>(p1->mpHdl->GetObj()) != nullptr)
        {
            // same object and a path object
            if((p1->mpHdl->GetKind() == SdrHdlKind::Poly || p1->mpHdl->GetKind() == SdrHdlKind::BezierWeight)
                && (p2->mpHdl->GetKind() == SdrHdlKind::Poly || p2->mpHdl->GetKind() == SdrHdlKind::BezierWeight))
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

}

void SdrHdlList::TravelFocusHdl(bool bForward)
{
    // security correction
    if (mnFocusIndex >= GetHdlCount())
        mnFocusIndex = SAL_MAX_SIZE;

    if(maList.empty())
        return;

    // take care of old handle
    const size_t nOldHdlNum(mnFocusIndex);
    SdrHdl* pOld = nullptr;
    if (nOldHdlNum < GetHdlCount())
        pOld = GetHdl(nOldHdlNum);

    if(pOld)
    {
        // switch off old handle
        mnFocusIndex = SAL_MAX_SIZE;
        pOld->Touch();
    }

    // allocate pointer array for sorted handle list
    std::unique_ptr<ImplHdlAndIndex[]> pHdlAndIndex(new ImplHdlAndIndex[maList.size()]);

    // build sorted handle list
    for( size_t a = 0; a < maList.size(); ++a)
    {
        pHdlAndIndex[a].mpHdl = maList[a].get();
        pHdlAndIndex[a].mnIndex = a;
    }

    qsort(pHdlAndIndex.get(), maList.size(), sizeof(ImplHdlAndIndex), ImplSortHdlFunc);

    // look for old num in sorted array
    size_t nOldHdl(nOldHdlNum);

    if(nOldHdlNum != SAL_MAX_SIZE)
    {
        for(size_t a = 0; a < maList.size(); ++a)
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
            if(nOldHdl == maList.size() - 1)
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
            nNewHdl = maList.size() - 1;

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

        for(size_t a = 0; a < maList.size(); ++a)
        {
            if(maList[a].get() == pNew)
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
        if (mnFocusIndex < GetHdlCount())
        {
            SdrHdl* pNew = GetHdl(mnFocusIndex);
            pNew->Touch();
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
    if(!pNew)
        return;

    SdrHdl* pActual = GetFocusHdl();

    if(pActual && pActual == pNew)
        return;

    const size_t nNewHdlNum = GetHdlNum(pNew);

    if(nNewHdlNum != SAL_MAX_SIZE)
    {
        mnFocusIndex = nNewHdlNum;

        if(pActual)
        {
            pActual->Touch();
        }

        pNew->Touch();
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
    pView(pV)
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

std::unique_ptr<SdrHdl> SdrHdlList::RemoveHdl(size_t nNum)
{
    std::unique_ptr<SdrHdl> pRetval = std::move(maList[nNum]);
    maList.erase(maList.begin() + nNum);

    return pRetval;
}

void SdrHdlList::RemoveAllByKind(SdrHdlKind eKind)
{
    maList.erase(std::remove_if(maList.begin(), maList.end(),
        [&eKind](std::unique_ptr<SdrHdl>& rItem) { return rItem->GetKind() == eKind; }),
        maList.end());
}

void SdrHdlList::Clear()
{
    maList.clear();

    bRotateShear=false;
    bDistortShear=false;
}

void SdrHdlList::Sort()
{
    // remember currently focused handle
    SdrHdl* pPrev = GetFocusHdl();

    std::sort( maList.begin(), maList.end(), ImpSdrHdlListSorter );

    // get now and compare
    SdrHdl* pNow = GetFocusHdl();

    if(pPrev == pNow)
        return;

    if(pPrev)
    {
        pPrev->Touch();
    }

    if(pNow)
    {
        pNow->Touch();
    }
}

size_t SdrHdlList::GetHdlNum(const SdrHdl* pHdl) const
{
    if (pHdl==nullptr)
        return SAL_MAX_SIZE;
    auto it = std::find_if( maList.begin(), maList.end(),
        [&](const std::unique_ptr<SdrHdl> & p) { return p.get() == pHdl; });
    assert(it != maList.end());
    if( it == maList.end() )
        return SAL_MAX_SIZE;
    return it - maList.begin();
}

void SdrHdlList::AddHdl(std::unique_ptr<SdrHdl> pHdl)
{
    assert(pHdl);
    pHdl->SetHdlList(this);
    maList.push_back(std::move(pHdl));
}

SdrHdl* SdrHdlList::IsHdlListHit(const Point& rPnt) const
{
    SdrHdl* pRet=nullptr;
    const size_t nCount=GetHdlCount();
    size_t nNum=nCount;
    while (nNum>0 && pRet==nullptr)
    {
        nNum--;
        SdrHdl* pHdl=GetHdl(nNum);
        if (pHdl->IsHdlHit(rPnt))
            pRet=pHdl;
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

void SdrHdlList::MoveTo(SdrHdlList& rOther)
{
    for (auto & pHdl : maList)
        pHdl->SetHdlList(&rOther);
    rOther.maList.insert(rOther.maList.end(),
        std::make_move_iterator(maList.begin()), std::make_move_iterator(maList.end()));
    maList.clear();
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
        case SdrHdlKind::UpperLeft: nX = 0; nY = 0; break;
        case SdrHdlKind::Upper: nX = 1; nY = 0; break;
        case SdrHdlKind::UpperRight: nX = 2; nY = 0; break;
        case SdrHdlKind::Left:  nX = 0; nY = 1; break;
        case SdrHdlKind::Right: nX = 2; nY = 1; break;
        case SdrHdlKind::LowerLeft: nX = 0; nY = 2; break;
        case SdrHdlKind::Lower: nX = 1; nY = 2; break;
        case SdrHdlKind::LowerRight: nX = 2; nY = 2; break;
        default: break;
    }

    tools::Rectangle aSourceRect( Point( nX * nPixelSize + nOffset,  nY * nPixelSize), Size(nPixelSize, nPixelSize) );

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

    if( !pPageView || pView->areMarkHandlesHidden() )
        return;

    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
    int nHdlSize = pHdlList->GetHdlSize();

    const BitmapEx aHandlesBitmap(SIP_SA_CROP_MARKERS);
    BitmapEx aBmpEx1( GetBitmapForHandle( aHandlesBitmap, nHdlSize ) );

    for(sal_uInt32 b(0); b < pPageView->PageWindowCount(); b++)
    {
        const SdrPageWindow& rPageWindow = *pPageView->GetPageWindow(b);

        if(rPageWindow.GetPaintWindow().OutputToWindow())
        {
            const rtl::Reference< sdr::overlay::OverlayManager >& xManager = rPageWindow.GetOverlayManager();
            if (xManager.is())
            {
                basegfx::B2DPoint aPosition(aPos.X(), aPos.Y());

                std::unique_ptr<sdr::overlay::OverlayObject> pOverlayObject;

                // animate focused handles
                if(IsFocusHdl() && (pHdlList->GetFocusHdl() == this))
                {
                    if( nHdlSize >= 2 )
                        nHdlSize = 1;

                    BitmapEx aBmpEx2( GetBitmapForHandle( aHandlesBitmap, nHdlSize + 1 ) );

                    const sal_uInt64 nBlinkTime = rStyleSettings.GetCursorBlinkTime();

                    pOverlayObject.reset(new sdr::overlay::OverlayAnimatedBitmapEx(
                        aPosition,
                        aBmpEx1,
                        aBmpEx2,
                        nBlinkTime,
                        static_cast<sal_uInt16>(aBmpEx1.GetSizePixel().Width() - 1) >> 1,
                        static_cast<sal_uInt16>(aBmpEx1.GetSizePixel().Height() - 1) >> 1,
                        static_cast<sal_uInt16>(aBmpEx2.GetSizePixel().Width() - 1) >> 1,
                        static_cast<sal_uInt16>(aBmpEx2.GetSizePixel().Height() - 1) >> 1,
                        mfShearX,
                        mfRotation));
                }
                else
                {
                    // create centered handle as default
                    pOverlayObject.reset(new sdr::overlay::OverlayBitmapEx(
                        aPosition,
                        aBmpEx1,
                        static_cast<sal_uInt16>(aBmpEx1.GetSizePixel().Width() - 1) >> 1,
                        static_cast<sal_uInt16>(aBmpEx1.GetSizePixel().Height() - 1) >> 1,
                        0.0,
                        mfShearX,
                        mfRotation));
                }

                // OVERLAYMANAGER
                insertNewlyCreatedOverlayObjectForSdrHdl(
                    std::move(pOverlayObject),
                    rPageWindow.GetObjectContact(),
                    *xManager);
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
:   SdrHdl(Point(), SdrHdlKind::User),
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
    if(basegfx::fTools::equal(fabs(*rotate), M_PI, 0.000000001))
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
        basegfx::utils::createPolygonFromRect(
            aCropped));
    basegfx::B2DPolyPolygon aCropPolyPolygon(aGraphicOutlinePolygon);

    // current range is unit range
    basegfx::B2DRange aOverlap(0.0, 0.0, 1.0, 1.0);

    aOverlap.intersect(aCropped);

    if(!aOverlap.isEmpty())
    {
        aCropPolyPolygon.append(
            basegfx::utils::createPolygonFromRect(
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
    const basegfx::BColor aHilightColor(SvtOptionsDrawinglayer::getHilightColor().getBColor());
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
            std::move(aCombination)));

    // embed to UnifiedTransparencePrimitive2D
    const drawinglayer::primitive2d::Primitive2DReference aTransparenceMaskedGraphic(
        new drawinglayer::primitive2d::UnifiedTransparencePrimitive2D(
            drawinglayer::primitive2d::Primitive2DContainer { aMaskedGraphic },
            0.8));

    const drawinglayer::primitive2d::Primitive2DContainer aSequence { aTransparenceMaskedGraphic };

    for(sal_uInt32 b(0); b < pPageView->PageWindowCount(); b++)
    {
        // const SdrPageViewWinRec& rPageViewWinRec = rPageViewWinList[b];
        const SdrPageWindow& rPageWindow = *(pPageView->GetPageWindow(b));

        if(rPageWindow.GetPaintWindow().OutputToWindow())
        {
            const rtl::Reference< sdr::overlay::OverlayManager >& xManager = rPageWindow.GetOverlayManager();
            if(xManager.is())
            {
                std::unique_ptr<sdr::overlay::OverlayObject> pNew(new sdr::overlay::OverlayPrimitive2DSequenceObject(drawinglayer::primitive2d::Primitive2DContainer(aSequence)));

                // only informative object, no hit
                pNew->setHittable(false);

                // OVERLAYMANAGER
                insertNewlyCreatedOverlayObjectForSdrHdl(
                    std::move(pNew),
                    rPageWindow.GetObjectContact(),
                    *xManager);
            }
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
