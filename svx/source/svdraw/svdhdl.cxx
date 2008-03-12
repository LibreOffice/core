/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: svdhdl.cxx,v $
 *
 *  $Revision: 1.31 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 09:51:40 $
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
#include "precompiled_svx.hxx"

#include <svx/svdhdl.hxx>
#include "svdtouch.hxx"
#include <svx/svdpagv.hxx>
#include <svx/svdetc.hxx>
#include <svx/svdmrkv.hxx>

#ifndef _WINDOW_HXX //autogen
#include <vcl/window.hxx>
#endif

#ifndef _VIRDEV_HXX //autogen
#include <vcl/virdev.hxx>
#endif

#ifndef _TL_POLY_HXX
#include <tools/poly.hxx>
#endif

#ifndef _SV_BMPACC_HXX
#include <vcl/bmpacc.hxx>
#endif

#ifndef _B3D_B3DCOLOR_HXX
#include <goodies/b3dcolor.hxx>
#endif

#include <svx/sxekitm.hxx>
#include "svdstr.hrc"
#include "svdglob.hxx"

#include <svx/svdmodel.hxx>
#include "gradtrns.hxx"
#include <svx/xflgrit.hxx>
#include <svx/svdundo.hxx>
#include <svx/dialmgr.hxx>
#include <svx/xflftrit.hxx>

// #105678#
#ifndef _SVDOPATH_HXX
#include <svx/svdopath.hxx>
#endif

#ifndef _BGFX_VECTOR_B2DVECTOR_HXX
#include <basegfx/vector/b2dvector.hxx>
#endif

#ifndef _BGFX_POLYGON_B2DPOLYGON_HXX
#include <basegfx/polygon/b2dpolygon.hxx>
#endif

#ifndef _SDR_OVERLAY_OVERLAYMANAGER_HXX
#include <svx/sdr/overlay/overlaymanager.hxx>
#endif

#ifndef _SDR_OVERLAY_OVERLAYANIMATEDBITMAPEX_HXX
#include <svx/sdr/overlay/overlayanimatedbitmapex.hxx>
#endif

#ifndef _SDR_OVERLAY_OVERLAYBITMAPEX_HXX
#include <svx/sdr/overlay/overlaybitmapex.hxx>
#endif

#ifndef _SDR_OVERLAY_OVERLAYBITMAP_HXX
#include <svx/sdr/overlay/overlaybitmap.hxx>
#endif

#ifndef _SDR_OVERLAY_OVERLAYLINESTRIPED_HXX
#include <svx/sdr/overlay/overlaylinestriped.hxx>
#endif

#ifndef _SDR_OVERLAY_OVERLAYTRIANGLE_HXX
#include <svx/sdr/overlay/overlaytriangle.hxx>
#endif

#ifndef _SDRPAGEWINDOW_HXX
#include <svx/sdrpagewindow.hxx>
#endif

#ifndef _SDRPAINTWINDOW_HXX
#include <sdrpaintwindow.hxx>
#endif

#ifndef _SV_SVAPP_HXX //autogen
#include <vcl/svapp.hxx>
#endif

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
    BitmapEx                maMarkersBitmap;

public:
    SdrHdlBitmapSet(UINT16 nResId);
    ~SdrHdlBitmapSet();

    BitmapEx GetBitmapEx(BitmapMarkerKind eKindOfMarker, UINT16 nInd=0);
};

////////////////////////////////////////////////////////////////////////////////////////////////////

SdrHdlBitmapSet::SdrHdlBitmapSet(UINT16 nResId)
{
    // #101928# change color used for transparent parts to 0x00ff00ff (ImageList standard)
    Color aColTransparent(0x00ff00ff);
    Bitmap aBitmap(ResId(nResId, *ImpGetResMgr()));
    maMarkersBitmap = BitmapEx(aBitmap, aColTransparent);
}

SdrHdlBitmapSet::~SdrHdlBitmapSet()
{
}

// #i15222#
// change getting of bitmap to use the big ressource bitmap
BitmapEx SdrHdlBitmapSet::GetBitmapEx(BitmapMarkerKind eKindOfMarker, UINT16 nInd)
{
    // fill in size and source position in maMarkersBitmap
    const sal_uInt16 nYPos(nInd * 11);
    Rectangle aSourceRect;

    switch(eKindOfMarker)
    {
        default:
        {
            DBG_ERROR( "unknown kind of marker" );
            // no break here, return Rect_7x7 as default
        }
        case Rect_7x7:
        {
            aSourceRect = Rectangle(Point(0, nYPos), Size(7, 7));
            break;
        }

        case Rect_9x9:
        {
            aSourceRect = Rectangle(Point(7, nYPos), Size(9, 9));
            break;
        }

        case Rect_11x11:
        {
            aSourceRect = Rectangle(Point(16, nYPos), Size(11, 11));
            break;
        }

        case Rect_13x13:
        {
            switch(nInd)
            {
                case 0: aSourceRect = Rectangle(Point(72, 66), Size(13, 13)); break;
                case 1: aSourceRect = Rectangle(Point(85, 66), Size(13, 13)); break;
                case 2: aSourceRect = Rectangle(Point(72, 78), Size(13, 13)); break;
                case 3: aSourceRect = Rectangle(Point(85, 78), Size(13, 13)); break;
                case 4: aSourceRect = Rectangle(Point(98, 78), Size(13, 13)); break;
                case 5: aSourceRect = Rectangle(Point(98, 66), Size(13, 13)); break;
            }

            break;
        }

        case Circ_7x7:
        {
            aSourceRect = Rectangle(Point(27, nYPos), Size(7, 7));
            break;
        }

        case Circ_9x9:
        case Customshape1:
        {
            aSourceRect = Rectangle(Point(34, nYPos), Size(9, 9));
            break;
        }

        case Circ_11x11:
        {
            aSourceRect = Rectangle(Point(43, nYPos), Size(11, 11));
            break;
        }

        case Elli_7x9:
        {
            aSourceRect = Rectangle(Point(54, nYPos), Size(7, 9));
            break;
        }

        case Elli_9x11:
        {
            aSourceRect = Rectangle(Point(61, nYPos), Size(9, 11));
            break;
        }

        case Elli_9x7:
        {
            aSourceRect = Rectangle(Point(70, nYPos), Size(9, 7));
            break;
        }

        case Elli_11x9:
        {
            aSourceRect = Rectangle(Point(79, nYPos), Size(11, 9));
            break;
        }

        case RectPlus_7x7:
        {
            aSourceRect = Rectangle(Point(90, nYPos), Size(7, 7));
            break;
        }

        case RectPlus_9x9:
        {
            aSourceRect = Rectangle(Point(97, nYPos), Size(9, 9));
            break;
        }

        case RectPlus_11x11:
        {
            aSourceRect = Rectangle(Point(106, nYPos), Size(11, 11));
            break;
        }

        case Crosshair:
        {
            aSourceRect = Rectangle(Point(0, 68), Size(15, 15));
            break;
        }

        case Glue:
        {
            aSourceRect = Rectangle(Point(15, 74), Size(9, 9));
            break;
        }

        case Anchor:
        // #101688# AnchorTR for SW
        case AnchorTR:
        {
            aSourceRect = Rectangle(Point(24, 68), Size(24, 23));
            break;
        }

        // #98388# add AnchorPressed to be able to aninate anchor control
        case AnchorPressed:
        case AnchorPressedTR:
        {
            aSourceRect = Rectangle(Point(48, 68), Size(24, 23));
            break;
        }
    }

    // construct return bitmap
    BitmapEx aRetval(maMarkersBitmap);
    aRetval.Crop(aSourceRect);

    return aRetval;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

SdrHdlBitmapSet* SdrHdl::pSimpleSet = NULL;
SdrHdlBitmapSet* SdrHdl::pModernSet = NULL;

// #101928#
SdrHdlBitmapSet* SdrHdl::pHighContrastSet = NULL;

////////////////////////////////////////////////////////////////////////////////////////////////////

SdrHdl::SdrHdl():
    pObj(NULL),
    pPV(NULL),
    pHdlList(NULL),
    eKind(HDL_MOVE),
    nDrehWink(0),
    nObjHdlNum(0),
    nPolyNum(0),
    nPPntNum(0),
    nSourceHdlNum(0),
    bSelect(FALSE),
    b1PixMore(FALSE),
    bPlusHdl(FALSE),
    mbMoveOutside(false)
{
    if(!pSimpleSet)
        pSimpleSet = new SdrHdlBitmapSet(SIP_SA_MARKERS);
    DBG_ASSERT(pSimpleSet, "Could not construct SdrHdlBitmapSet()!");

    if(!pModernSet)
        pModernSet = new SdrHdlBitmapSet(SIP_SA_FINE_MARKERS);
    DBG_ASSERT(pModernSet, "Could not construct SdrHdlBitmapSet()!");

    // #101928#
    if(!pHighContrastSet)
        pHighContrastSet = new SdrHdlBitmapSet(SIP_SA_ACCESSIBILITY_MARKERS);
    DBG_ASSERT(pHighContrastSet, "Could not construct SdrHdlBitmapSet()!");
}

SdrHdl::SdrHdl(const Point& rPnt, SdrHdlKind eNewKind):
    pObj(NULL),
    pPV(NULL),
    pHdlList(NULL),
    aPos(rPnt),
    eKind(eNewKind),
    nDrehWink(0),
    nObjHdlNum(0),
    nPolyNum(0),
    nPPntNum(0),
    nSourceHdlNum(0),
    bSelect(FALSE),
    b1PixMore(FALSE),
    bPlusHdl(FALSE),
    mbMoveOutside(false)
{
    if(!pSimpleSet)
        pSimpleSet = new SdrHdlBitmapSet(SIP_SA_MARKERS);
    DBG_ASSERT(pSimpleSet, "Could not construct SdrHdlBitmapSet()!");

    if(!pModernSet)
        pModernSet = new SdrHdlBitmapSet(SIP_SA_FINE_MARKERS);
    DBG_ASSERT(pModernSet, "Could not construct SdrHdlBitmapSet()!");

    // #101928#
    if(!pHighContrastSet)
        pHighContrastSet = new SdrHdlBitmapSet(SIP_SA_ACCESSIBILITY_MARKERS);
    DBG_ASSERT(pHighContrastSet, "Could not construct SdrHdlBitmapSet()!");
}

SdrHdl::~SdrHdl()
{
    GetRidOfIAObject();
}

void SdrHdl::Set1PixMore(BOOL bJa)
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

void SdrHdl::SetDrehWink(long n)
{
    if(nDrehWink != n)
    {
        nDrehWink = n;

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

void SdrHdl::SetSelected(BOOL bJa)
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
        // rememver list
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
    //OLMaIAOGroup.Delete();

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

        BOOL bRot = pHdlList->IsRotateShear();
        if(pObj)
            eColIndex = (bSelect) ? Cyan : LightCyan;
        if(bRot)
        {
            // Drehhandles in Rot
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
                eKindOfMarker = Customshape1;
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

                    // add offset if necessary
                    if(pHdlList->IsMoveOutside() || mbMoveOutside)
                    {
                        OutputDevice& rOutDev = rPageWindow.GetPaintWindow().GetOutputDevice();
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

                    if(rPageWindow.GetOverlayManager())
                    {
                        basegfx::B2DPoint aPosition(aPos.X(), aPos.Y());
                        ::sdr::overlay::OverlayObject* pNewOverlayObject = CreateOverlayObject(
                            aPosition,
                            eColIndex,
                            eKindOfMarker,
                            aMoveOutsideOffset);

                        // OVERLAYMANAGER
                        if(pNewOverlayObject)
                        {
                            rPageWindow.GetOverlayManager()->add(*pNewOverlayObject);
                            maOverlayGroup.append(*pNewOverlayObject);
                        }
                    }
                }
            }
        }
    }
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
BitmapEx SdrHdl::ImpGetBitmapEx(BitmapMarkerKind eKindOfMarker, sal_uInt16 nInd, sal_Bool bFine, sal_Bool bIsHighContrast)
{
    if(bIsHighContrast)
    {
        return pHighContrastSet->GetBitmapEx(eKindOfMarker, nInd);
    }
    else
    {
        if(bFine)
        {
            return pModernSet->GetBitmapEx(eKindOfMarker, nInd);
        }
        else
        {
            return pSimpleSet->GetBitmapEx(eKindOfMarker, nInd);
        }
    }
}

::sdr::overlay::OverlayObject* SdrHdl::CreateOverlayObject(
    const basegfx::B2DPoint& rPos,
    BitmapColorIndex eColIndex, BitmapMarkerKind eKindOfMarker, Point aMoveOutsideOffset)
{
    ::sdr::overlay::OverlayObject* pRetval = 0L;
    sal_Bool bIsFineHdl(pHdlList->IsFineHdl());
    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
    sal_Bool bIsHighContrast(rStyleSettings.GetHighContrastMode());

    // support bigger sizes
    sal_Bool bForceBiggerSize(sal_False);

    if(pHdlList->GetHdlSize() > 3)
    {
        bForceBiggerSize = sal_True;
    }

    // #101928# ...for high contrast, too.
    if(!bForceBiggerSize && bIsHighContrast)
    {
        // #107925#
        // ...but not for anchors, else they will not blink when activated
        if(Anchor != eKindOfMarker && AnchorTR != eKindOfMarker)
        {
            bForceBiggerSize = sal_True;
        }
    }

    if(bForceBiggerSize)
    {
        eKindOfMarker = GetNextBigger(eKindOfMarker);
    }

    // #97016# II This handle has the focus, visualize it
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
                (UINT16)(aBmpEx1.GetSizePixel().Width() - 1), 0,
                (UINT16)(aBmpEx2.GetSizePixel().Width() - 1), 0);
        }
        else
        {
            // create centered handle as default
            pRetval = new ::sdr::overlay::OverlayAnimatedBitmapEx(rPos, aBmpEx1, aBmpEx2, nBlinkTime,
                (UINT16)(aBmpEx1.GetSizePixel().Width() - 1) >> 1,
                (UINT16)(aBmpEx1.GetSizePixel().Height() - 1) >> 1,
                (UINT16)(aBmpEx2.GetSizePixel().Width() - 1) >> 1,
                (UINT16)(aBmpEx2.GetSizePixel().Height() - 1) >> 1);
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
                (UINT16)(aBmpEx.GetSizePixel().Width() - 1), 0);
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
            pRetval = new ::sdr::overlay::OverlayBitmapEx(rPos, aBmpEx, nCenX, nCenY);
        }
    }

    return pRetval;
}

bool SdrHdl::IsHdlHit(const Point& rPnt) const
{
    // OVERLAYMANAGER
    basegfx::B2DPoint aPosition(rPnt.X(), rPnt.Y());
    return maOverlayGroup.isHit(aPosition);
}

Pointer SdrHdl::GetPointer() const
{
    PointerStyle ePtr=POINTER_MOVE;
    BOOL bSize=eKind>=HDL_UPLFT && eKind<=HDL_LWRGT;
    // Fuer Resize von gedrehten Rechtecken die Mauszeiger etwas mitdrehen
    if (bSize && nDrehWink!=0) {
        long nHdlWink=0;
        switch (eKind) {
            case HDL_UPLFT: nHdlWink=13500; break;
            case HDL_UPPER: nHdlWink=9000;  break;
            case HDL_UPRGT: nHdlWink=4500;  break;
            case HDL_LEFT : nHdlWink=0;     break;
            case HDL_RIGHT: nHdlWink=0;     break;
            case HDL_LWLFT: nHdlWink=4500;  break;
            case HDL_LOWER: nHdlWink=9000;  break;
            case HDL_LWRGT: nHdlWink=13500; break;
            default:
                break;
        }
        nHdlWink+=nDrehWink+2249; // und etwas drauf (zum runden)
        while (nHdlWink<0) nHdlWink+=18000;
        while (nHdlWink>=18000) nHdlWink-=18000;
        nHdlWink/=4500;
        switch ((BYTE)nHdlWink) {
            case 0: ePtr=POINTER_ESIZE;    break;
            case 1: ePtr=POINTER_NESIZE; break;
            case 2: ePtr=POINTER_SSIZE;    break;
            case 3: ePtr=POINTER_SESIZE; break;
        } // switch
    }
    if (ePtr==POINTER_MOVE) {
        BOOL bRot=pHdlList!=NULL && pHdlList->IsRotateShear();
        BOOL bDis=pHdlList!=NULL && pHdlList->IsDistortShear();
        if (bSize && pHdlList!=NULL && (bRot || bDis)) {
            switch (eKind) {
                case HDL_UPLFT: case HDL_UPRGT:
                case HDL_LWLFT: case HDL_LWRGT: ePtr=bRot ? POINTER_ROTATE : POINTER_REFHAND; break;
                case HDL_LEFT : case HDL_RIGHT: ePtr=POINTER_VSHEAR; break;
                case HDL_UPPER: case HDL_LOWER: ePtr=POINTER_HSHEAR; break;
                default:
                    break;
            }
        } else {
            switch (eKind) {
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
BOOL SdrHdl::IsFocusHdl() const
{
    switch(eKind)
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
            if(pHdlList && pHdlList->IsMoveOutside())
                return FALSE;
            else
                return TRUE;
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
            return TRUE;
        }

        default:
        {
            return FALSE;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// class SdrHdlColor

SdrHdlColor::SdrHdlColor(const Point& rRef, Color aCol, const Size& rSize, BOOL bLum)
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
                    // const SdrPageViewWinRec& rPageViewWinRec = rPageViewWinList[b];
                    const SdrPageWindow& rPageWindow = *pPageView->GetPageWindow(b);

                    if(rPageWindow.GetPaintWindow().OutputToWindow())
                    {
                        if(rPageWindow.GetOverlayManager())
                        {
                            Bitmap aBmpCol(CreateColorDropper(aMarkerColor));
                            basegfx::B2DPoint aPosition(aPos.X(), aPos.Y());
                            ::sdr::overlay::OverlayObject* pNewOverlayObject = new
                                ::sdr::overlay::OverlayBitmap(
                                    aPosition, aBmpCol,
                                    (UINT16)(aBmpCol.GetSizePixel().Width() - 1) >> 1,
                                    (UINT16)(aBmpCol.GetSizePixel().Height() - 1) >> 1
                                );
                            DBG_ASSERT(pNewOverlayObject, "Got NO new IAO!");

                            // OVERLAYMANAGER
                            if(pNewOverlayObject)
                            {
                                rPageWindow.GetOverlayManager()->add(*pNewOverlayObject);
                                maOverlayGroup.append(*pNewOverlayObject);
                            }
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
    BitmapWriteAccess* pWrite = aRetval.AcquireWriteAccess();
    DBG_ASSERT(pWrite, "Got NO write access to a new Bitmap !!!");

    if(pWrite)
    {
        // draw outer border
        INT32 nWidth = aMarkerSize.Width();
        INT32 nHeight = aMarkerSize.Height();

        pWrite->SetLineColor(Color(COL_LIGHTGRAY));
        pWrite->DrawLine(Point(0, 0), Point(0, nHeight - 1));
        pWrite->DrawLine(Point(1, 0), Point(nWidth - 1, 0));
        pWrite->SetLineColor(Color(COL_GRAY));
        pWrite->DrawLine(Point(1, nHeight - 1), Point(nWidth - 1, nHeight - 1));
        pWrite->DrawLine(Point(nWidth - 1, 1), Point(nWidth - 1, nHeight - 2));

        // draw lighter UpperLeft
        B3dColor aMixCol(aCol);
        B3dColor aFactor(0x40, 0x40, 0x40);
        aMixCol += aFactor;
        pWrite->SetLineColor((Color)aMixCol);
        pWrite->DrawLine(Point(1, 1), Point(1, nHeight - 2));
        pWrite->DrawLine(Point(2, 1), Point(nWidth - 2, 1));

        // draw darker LowerRight
        aMixCol = aCol;
        aMixCol -= aFactor;
        pWrite->SetLineColor((Color)aMixCol);
        pWrite->DrawLine(Point(2, nHeight - 2), Point(nWidth - 2, nHeight - 2));
        pWrite->DrawLine(Point(nWidth - 2, 2), Point(nWidth - 2, nHeight - 3));

        // get rid of write access
        delete pWrite;
    }

    return aRetval;
}

Color SdrHdlColor::GetLuminance(const Color& rCol)
{
    UINT8 aLum = rCol.GetLuminance();
    Color aRetval(aLum, aLum, aLum);
    return aRetval;
}

void SdrHdlColor::CallColorChangeLink()
{
    aColorChangeHdl.Call(this);
}

void SdrHdlColor::SetColor(Color aNew, BOOL bCallLink)
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

SdrHdlGradient::SdrHdlGradient(const Point& rRef1, const Point& rRef2, BOOL bGrad)
:   SdrHdl(rRef1, bGrad ? HDL_GRAD : HDL_TRNS),
    pColHdl1(NULL),
    pColHdl2(NULL),
    a2ndPos(rRef2),
    bGradient(bGrad)
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
                        if(rPageWindow.GetOverlayManager())
                        {
                            // striped line in between
                            basegfx::B2DVector aVec(a2ndPos.X() - aPos.X(), a2ndPos.Y() - aPos.Y());
                            double fVecLen = aVec.getLength();
                            double fLongPercentArrow = (1.0 - 0.05) * fVecLen;
                            double fHalfArrowWidth = (0.05 * 0.5) * fVecLen;
                            aVec.normalize();
                            basegfx::B2DVector aPerpend(-aVec.getY(), aVec.getX());
                            INT32 nMidX = (INT32)(aPos.X() + aVec.getX() * fLongPercentArrow);
                            INT32 nMidY = (INT32)(aPos.Y() + aVec.getY() * fLongPercentArrow);
                            Point aMidPoint(nMidX, nMidY);

                            basegfx::B2DPoint aPosition(aPos.X(), aPos.Y());
                            basegfx::B2DPoint aMidPos(aMidPoint.X(), aMidPoint.Y());

                            ::sdr::overlay::OverlayObject* pNewOverlayObject = new
                                ::sdr::overlay::OverlayLineStriped(
                                    aPosition, aMidPos
                                );
                            DBG_ASSERT(pNewOverlayObject, "Got NO new IAO!");

                            pNewOverlayObject->setBaseColor(IsGradient() ? Color(COL_BLACK) : Color(COL_BLUE));
                            rPageWindow.GetOverlayManager()->add(*pNewOverlayObject);
                            maOverlayGroup.append(*pNewOverlayObject);

                            // arrowhead
                            Point aLeft(aMidPoint.X() + (INT32)(aPerpend.getX() * fHalfArrowWidth),
                                        aMidPoint.Y() + (INT32)(aPerpend.getY() * fHalfArrowWidth));
                            Point aRight(aMidPoint.X() - (INT32)(aPerpend.getX() * fHalfArrowWidth),
                                        aMidPoint.Y() - (INT32)(aPerpend.getY() * fHalfArrowWidth));

                            basegfx::B2DPoint aPositionLeft(aLeft.X(), aLeft.Y());
                            basegfx::B2DPoint aPositionRight(aRight.X(), aRight.Y());
                            basegfx::B2DPoint aPosition2(a2ndPos.X(), a2ndPos.Y());

                            pNewOverlayObject = new
                                ::sdr::overlay::OverlayTriangle(
                                    aPositionLeft, aPosition2, aPositionRight
                                );
                            DBG_ASSERT(pNewOverlayObject, "Got NO new IAO!");

                            pNewOverlayObject->setBaseColor(IsGradient() ? Color(COL_BLACK) : Color(COL_BLUE));
                            rPageWindow.GetOverlayManager()->add(*pNewOverlayObject);
                            maOverlayGroup.append(*pNewOverlayObject);
                        }
                    }
                }
            }
        }
    }
}

IMPL_LINK(SdrHdlGradient, ColorChangeHdl, SdrHdl*, /*pHdl*/)
{
    if(GetObj())
        FromIAOToItem(GetObj(), TRUE, TRUE);
    return 0;
}

void SdrHdlGradient::FromIAOToItem(SdrObject* _pObj, BOOL bSetItemOnObject, BOOL bUndo)
{
    // from IAO positions and colors to gradient
    const SfxItemSet& rSet = _pObj->GetMergedItemSet();

    GradTransformer aGradTransformer;
    GradTransGradient aOldGradTransGradient;
    GradTransGradient aGradTransGradient;
    GradTransVector aGradTransVector;

    String aString;

    aGradTransVector.maPositionA = basegfx::B2DPoint(GetPos().X(), GetPos().Y());
    aGradTransVector.maPositionB = basegfx::B2DPoint(Get2ndPos().X(), Get2ndPos().Y());
    if(pColHdl1)
        aGradTransVector.aCol1 = pColHdl1->GetColor();
    if(pColHdl2)
        aGradTransVector.aCol2 = pColHdl2->GetColor();

    if(IsGradient())
        aOldGradTransGradient.aGradient = ((XFillGradientItem&)rSet.Get(XATTR_FILLGRADIENT)).GetGradientValue();
    else
        aOldGradTransGradient.aGradient = ((XFillFloatTransparenceItem&)rSet.Get(XATTR_FILLFLOATTRANSPARENCE)).GetGradientValue();

    // transform vector data to gradient
    aGradTransformer.VecToGrad(aGradTransVector, aGradTransGradient, aOldGradTransGradient, _pObj, bMoveSingleHandle, bMoveFirstHandle);

    if(bSetItemOnObject)
    {
        SdrModel* pModel = _pObj->GetModel();
        SfxItemSet aNewSet(pModel->GetItemPool());

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

        if(bUndo)
        {
            pModel->BegUndo(SVX_RESSTR(IsGradient() ? SIP_XA_FILLGRADIENT : SIP_XA_FILLTRANSPARENCE));
            pModel->AddUndo(pModel->GetSdrUndoFactory().CreateUndoAttrObject(*_pObj));
            pModel->EndUndo();
        }

        pObj->SetMergedItemSetAndBroadcast(aNewSet);
    }

    // back transformation, set values on pIAOHandle
    aGradTransformer.GradToVec(aGradTransGradient, aGradTransVector, _pObj);

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

////////////////////////////////////////////////////////////////////////////////////////////////////

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
                        if(rPageWindow.GetOverlayManager())
                        {
                            basegfx::B2DPoint aPosition1(pHdl1->GetPos().X(), pHdl1->GetPos().Y());
                            basegfx::B2DPoint aPosition2(pHdl2->GetPos().X(), pHdl2->GetPos().Y());

                            ::sdr::overlay::OverlayObject* pNewOverlayObject = new
                                ::sdr::overlay::OverlayLineStriped(
                                    aPosition1,
                                    aPosition2
                                );
                            DBG_ASSERT(pNewOverlayObject, "Got NO new IAO!");

                            // OVERLAYMANAGER
                            if(pNewOverlayObject)
                            {
                                // color(?)
                                pNewOverlayObject->setBaseColor(Color(COL_LIGHTRED));

                                rPageWindow.GetOverlayManager()->add(*pNewOverlayObject);
                                maOverlayGroup.append(*pNewOverlayObject);
                            }
                        }
                    }
                }
            }
        }
    }
}

bool SdrHdlLine::IsHdlHit(const Point& rPnt) const
{
    // OVERLAYMANAGER
    basegfx::B2DPoint aPosition(rPnt.X(), rPnt.Y());
    return maOverlayGroup.isHit(aPosition);
}

Pointer SdrHdlLine::GetPointer() const
{
    return Pointer(POINTER_REFHAND);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

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
                        if(rPageWindow.GetOverlayManager())
                        {
                            basegfx::B2DPoint aPosition1(pHdl1->GetPos().X(), pHdl1->GetPos().Y());
                            basegfx::B2DPoint aPosition2(aPos.X(), aPos.Y());

                            ::sdr::overlay::OverlayObject* pNewOverlayObject = new
                                ::sdr::overlay::OverlayLineStriped(
                                    aPosition1,
                                    aPosition2
                                );
                            DBG_ASSERT(pNewOverlayObject, "Got NO new IAO!");

                            // OVERLAYMANAGER
                            if(pNewOverlayObject)
                            {
                                // line part is not hittable
                                pNewOverlayObject->setHittable(sal_False);

                                // color(?)
                                pNewOverlayObject->setBaseColor(Color(COL_LIGHTBLUE));

                                rPageWindow.GetOverlayManager()->add(*pNewOverlayObject);
                                maOverlayGroup.append(*pNewOverlayObject);
                            }
                        }
                    }
                }
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

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
                        if(rPageWindow.GetOverlayManager())
                        {
                            const sal_uInt32 nCnt(aWireframePoly.count());

                            for(sal_uInt32 i(0L); i < nCnt; i++)
                            {
                                const basegfx::B2DPolygon aPoly(aWireframePoly.getB2DPolygon(i));
                                const basegfx::B2DPoint aPointA(aPoly.getB2DPoint(0L));
                                const basegfx::B2DPoint aPointB(aPoly.getB2DPoint(1L));

                                ::sdr::overlay::OverlayObject* pNewOverlayObject = new
                                    ::sdr::overlay::OverlayLineStriped(
                                        aPointA,
                                        aPointB
                                    );
                                DBG_ASSERT(pNewOverlayObject, "Got NO new IAO!");

                                // OVERLAYMANAGER
                                if(pNewOverlayObject)
                                {
                                    pNewOverlayObject->setBaseColor(Color(COL_BLACK));

                                    rPageWindow.GetOverlayManager()->add(*pNewOverlayObject);
                                    maOverlayGroup.append(*pNewOverlayObject);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

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
                const SdrEdgeObj* pEdge = (SdrEdgeObj*)pObj;

                if(pEdge->GetConnectedNode(nObjHdlNum == 0) != NULL)
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
                            if(rPageWindow.GetOverlayManager())
                            {
                                basegfx::B2DPoint aPosition(aPos.X(), aPos.Y());

                                ::sdr::overlay::OverlayObject* pNewOverlayObject = CreateOverlayObject(
                                    aPosition,
                                    eColIndex,
                                    eKindOfMarker);

                                // OVERLAYMANAGER
                                if(pNewOverlayObject)
                                {
                                    rPageWindow.GetOverlayManager()->add(*pNewOverlayObject);
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
    SdrEdgeObj* pEdge=PTR_CAST(SdrEdgeObj,pObj);
    if (pEdge==NULL) return SdrHdl::GetPointer();
    if (nObjHdlNum<=1) return Pointer(POINTER_MOVEPOINT); //Pointer(POINTER_DRAW_CONNECT);
    if (IsHorzDrag()) return Pointer(POINTER_ESIZE);
    else return Pointer(POINTER_SSIZE);
}

BOOL ImpEdgeHdl::IsHorzDrag() const
{
    SdrEdgeObj* pEdge=PTR_CAST(SdrEdgeObj,pObj);
    if (pEdge==NULL) return FALSE;
    if (nObjHdlNum<=1) return FALSE;

    SdrEdgeKind eEdgeKind = ((SdrEdgeKindItem&)(pEdge->GetObjectItem(SDRATTR_EDGEKIND))).GetValue();

    const SdrEdgeInfoRec& rInfo=pEdge->aEdgeInfo;
    if (eEdgeKind==SDREDGE_ORTHOLINES || eEdgeKind==SDREDGE_BEZIER) {
        return !rInfo.ImpIsHorzLine(eLineCode,*pEdge->pEdgeTrack);
    } else if (eEdgeKind==SDREDGE_THREELINES) {
        long nWink=nObjHdlNum==2 ? rInfo.nAngle1 : rInfo.nAngle2;
        if (nWink==0 || nWink==18000) return TRUE;
        else return FALSE;
    }
    return FALSE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

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
                        if(rPageWindow.GetOverlayManager())
                        {
                            basegfx::B2DPoint aPosition(aPos.X(), aPos.Y());

                            ::sdr::overlay::OverlayObject* pNewOverlayObject = CreateOverlayObject(
                                aPosition,
                                eColIndex,
                                eKindOfMarker);

                            // OVERLAYMANAGER
                            if(pNewOverlayObject)
                            {
                                rPageWindow.GetOverlayManager()->add(*pNewOverlayObject);
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
    switch (nObjHdlNum) {
        case 0: case 1: return Pointer(POINTER_HAND);
        case 2: case 3: return Pointer(POINTER_MOVEPOINT);
        case 4: case 5: return SdrHdl::GetPointer(); // wird dann entsprechend gedreht
    } // switch
    return Pointer(POINTER_NOTALLOWED);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

class ImpSdrHdlListSorter: public ContainerSorter {
public:
    ImpSdrHdlListSorter(Container& rNewCont): ContainerSorter(rNewCont) {}
    virtual int Compare(const void* pElem1, const void* pElem2) const;
};

int ImpSdrHdlListSorter::Compare(const void* pElem1, const void* pElem2) const
{
    SdrHdlKind eKind1=((SdrHdl*)pElem1)->GetKind();
    SdrHdlKind eKind2=((SdrHdl*)pElem2)->GetKind();
    // Level 1: Erst normale Handles, dann Glue, dann User, dann Plushandles, dann Retpunkt-Handles
    unsigned n1=1;
    unsigned n2=1;
    if (eKind1!=eKind2) {
        if (eKind1==HDL_REF1 || eKind1==HDL_REF2 || eKind1==HDL_MIRX) n1=5;
        else if (eKind1==HDL_GLUE) n1=2;
        else if (eKind1==HDL_USER) n1=3;
        else if (eKind1==HDL_SMARTTAG) n1=0;
        if (eKind2==HDL_REF1 || eKind2==HDL_REF2 || eKind2==HDL_MIRX) n2=5;
        else if (eKind2==HDL_GLUE) n2=2;
        else if (eKind2==HDL_USER) n2=3;
        else if (eKind2==HDL_SMARTTAG) n2=0;
    }
    if (((SdrHdl*)pElem1)->IsPlusHdl()) n1=4;
    if (((SdrHdl*)pElem2)->IsPlusHdl()) n2=4;
    if (n1==n2) {
        // Level 2: PageView (Pointer)
        SdrPageView* pPV1=((SdrHdl*)pElem1)->GetPageView();
        SdrPageView* pPV2=((SdrHdl*)pElem2)->GetPageView();
        if (pPV1==pPV2) {
            // Level 3: Position (x+y)
            SdrObject* pObj1=((SdrHdl*)pElem1)->GetObj();
            SdrObject* pObj2=((SdrHdl*)pElem2)->GetObj();
            if (pObj1==pObj2) {
                sal_uInt32 nNum1=((SdrHdl*)pElem1)->GetObjHdlNum();
                sal_uInt32 nNum2=((SdrHdl*)pElem2)->GetObjHdlNum();
                if (nNum1==nNum2) { // #48763#
                    if (eKind1==eKind2)
                        return (long)pElem1<(long)pElem2 ? -1 : 1; // Notloesung, um immer die gleiche Sortierung zu haben
                    return (USHORT)eKind1<(USHORT)eKind2 ? -1 : 1;
                } else return nNum1<nNum2 ? -1 : 1;
            } else {
                return (long)pObj1<(long)pObj2 ? -1 : 1;
            }
        } else {
            return (long)pPV1<(long)pPV2 ? -1 : 1;
        }
    } else {
        return n1<n2 ? -1 : 1;
    }
}

SdrMarkView* SdrHdlList::GetView() const
{
    return pView;
}

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
        if(p1->mpHdl->GetObj() && p1->mpHdl->GetObj()->ISA(SdrPathObj))
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

////////////////////////////////////////////////////////////////////////////////////////////////////
// #97016# II

void SdrHdlList::TravelFocusHdl(sal_Bool bForward)
{
    // security correction
    if(mnFocusIndex != CONTAINER_ENTRY_NOTFOUND && mnFocusIndex >= GetHdlCount())
        mnFocusIndex = CONTAINER_ENTRY_NOTFOUND;

    if(aList.Count())
    {
        // take care of old handle
        const sal_uInt32 nOldHdlNum(mnFocusIndex);
        SdrHdl* pOld = GetHdl(nOldHdlNum);
        //SDOsal_Bool bRefresh(sal_False);

        if(pOld)
        {
            // switch off old handle
            mnFocusIndex = CONTAINER_ENTRY_NOTFOUND;
            pOld->Touch();
            //SDObRefresh = sal_True;
        }

        // #105678# Alloc pointer array for sorted handle list
        ImplHdlAndIndex* pHdlAndIndex = new ImplHdlAndIndex[aList.Count()];

        // #105678# build sorted handle list
        sal_uInt32 a;
        for( a = 0; a < aList.Count(); a++)
        {
            pHdlAndIndex[a].mpHdl = (SdrHdl*)aList.GetObject(a);
            pHdlAndIndex[a].mnIndex = a;
        }

        // #105678# qsort all entries
        qsort(pHdlAndIndex, aList.Count(), sizeof(ImplHdlAndIndex), ImplSortHdlFunc);

        // #105678# look for old num in sorted array
        sal_uInt32 nOldHdl(nOldHdlNum);

        if(nOldHdlNum != CONTAINER_ENTRY_NOTFOUND)
        {
            for(a = 0; a < aList.Count(); a++)
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
                if(nOldHdl == aList.Count() - 1)
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
                nNewHdl = aList.Count() - 1;

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

            for(a = 0; a < aList.Count(); a++)
            {
                if((SdrHdl*)aList.GetObject(a) == pNew)
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
                //SDObRefresh = sal_True;
            }
        }

        // #105678# free mem again
        delete pHdlAndIndex;
    }
}

SdrHdl* SdrHdlList::GetFocusHdl() const
{
    if(mnFocusIndex != CONTAINER_ENTRY_NOTFOUND && mnFocusIndex < GetHdlCount())
        return GetHdl(mnFocusIndex);
    else
        return 0L;
}

void SdrHdlList::SetFocusHdl(SdrHdl* pNew)
{
    if(pNew)
    {
        SdrHdl* pActual = GetFocusHdl();

        if(!pActual || pActual != pNew)
        {
            sal_uInt32 nNewHdlNum = GetHdlNum(pNew);

            if(nNewHdlNum != CONTAINER_ENTRY_NOTFOUND)
            {
                //SDOsal_Bool bRefresh(sal_False);
                mnFocusIndex = nNewHdlNum;

                if(pActual)
                {
                    pActual->Touch();
                    //SDObRefresh = sal_True;
                }

                if(pNew)
                {
                    pNew->Touch();
                    //SDObRefresh = sal_True;
                }

                //OLMif(bRefresh)
                //OLM{
                //OLM   if(pView)
                //OLM       pView->RefreshAllIAOManagers();
                //OLM}
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

////////////////////////////////////////////////////////////////////////////////////////////////////

SdrHdlList::SdrHdlList(SdrMarkView* pV)
:   mnFocusIndex(CONTAINER_ENTRY_NOTFOUND),
    pView(pV),
    aList(1024,32,32)
{
    nHdlSize = 3;
    bRotateShear = FALSE;
    bMoveOutside = FALSE;
    bDistortShear = FALSE;
    bFineHandles = FALSE;
}

SdrHdlList::~SdrHdlList()
{
    Clear();
}

void SdrHdlList::SetHdlSize(USHORT nSiz)
{
    if(nHdlSize != nSiz)
    {
        // remember new value
        nHdlSize = nSiz;

        // propagate change to IAOs
        for(UINT32 i=0; i<GetHdlCount(); i++)
        {
            SdrHdl* pHdl = GetHdl(i);
            pHdl->Touch();
        }
    }
}

void SdrHdlList::SetMoveOutside(BOOL bOn)
{
    if(bMoveOutside != bOn)
    {
        // remember new value
        bMoveOutside = bOn;

        // propagate change to IAOs
        for(UINT32 i=0; i<GetHdlCount(); i++)
        {
            SdrHdl* pHdl = GetHdl(i);
            pHdl->Touch();
        }
    }
}

void SdrHdlList::SetRotateShear(BOOL bOn)
{
    bRotateShear = bOn;
}

void SdrHdlList::SetDistortShear(BOOL bOn)
{
    bDistortShear = bOn;
}

void SdrHdlList::SetFineHdl(BOOL bOn)
{
    if(bFineHandles != bOn)
    {
        // remember new state
        bFineHandles = bOn;

        // propagate change to IAOs
        for(UINT32 i=0; i<GetHdlCount(); i++)
        {
            SdrHdl* pHdl = GetHdl(i);
            pHdl->Touch();
        }
    }
}

SdrHdl* SdrHdlList::RemoveHdl(ULONG nNum)
{
    SdrHdl* pRetval = (SdrHdl*)aList.Remove(nNum);

    return pRetval;
}

void SdrHdlList::Clear()
{
    for (ULONG i=0; i<GetHdlCount(); i++)
    {
        SdrHdl* pHdl=GetHdl(i);
        delete pHdl;
    }
    aList.Clear();

    bRotateShear=FALSE;
    bDistortShear=FALSE;
}

void SdrHdlList::Sort()
{
    // #97016# II: remember current focused handle
    SdrHdl* pPrev = GetFocusHdl();

    ImpSdrHdlListSorter aSort(aList);
    aSort.DoSort();

    // #97016# II: get now and compare
    SdrHdl* pNow = GetFocusHdl();

    if(pPrev != pNow)
    {
        //SDOsal_Bool bRefresh(sal_False);

        if(pPrev)
        {
            pPrev->Touch();
            //SDObRefresh = sal_True;
        }

        if(pNow)
        {
            pNow->Touch();
            //SDObRefresh = sal_True;
        }
    }
}

ULONG SdrHdlList::GetHdlNum(const SdrHdl* pHdl) const
{
    if (pHdl==NULL) return CONTAINER_ENTRY_NOTFOUND;
    ULONG nPos=aList.GetPos(pHdl);
    return nPos;
}

void SdrHdlList::AddHdl(SdrHdl* pHdl, BOOL bAtBegin)
{
    if (pHdl!=NULL) {
        if (bAtBegin) {
            aList.Insert(pHdl,ULONG(0));
        } else {
            aList.Insert(pHdl,CONTAINER_APPEND);
        }
        pHdl->SetHdlList(this);
    }
}

SdrHdl* SdrHdlList::IsHdlListHit(const Point& rPnt, BOOL bBack, BOOL bNext, SdrHdl* pHdl0) const
{
   SdrHdl* pRet=NULL;
   ULONG nAnz=GetHdlCount();
   ULONG nNum=bBack ? 0 : nAnz;
   while ((bBack ? nNum<nAnz : nNum>0) && pRet==NULL) {
       if (!bBack) nNum--;
       SdrHdl* pHdl=GetHdl(nNum);
       if (bNext) {
           if (pHdl==pHdl0) bNext=FALSE;
       } else {
           if (pHdl->IsHdlHit(rPnt)) pRet=pHdl;
       }
       if (bBack) nNum++;
   }
   return pRet;
}

SdrHdl* SdrHdlList::GetHdl(SdrHdlKind eKind1) const
{
   SdrHdl* pRet=NULL;
   for (ULONG i=0; i<GetHdlCount() && pRet==NULL; i++) {
       SdrHdl* pHdl=GetHdl(i);
       if (pHdl->GetKind()==eKind1) pRet=pHdl;
   }
   return pRet;
}

// --------------------------------------------------------------------
// SdrCropHdl
// --------------------------------------------------------------------

SdrCropHdl::SdrCropHdl(const Point& rPnt, SdrHdlKind eNewKind)
: SdrHdl( rPnt, eNewKind )
{
}

// --------------------------------------------------------------------

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

// --------------------------------------------------------------------

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
        nOffset = 36;
    }
    else
    {
        nPixelSize = 21;
        nOffset = 84;
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

    Rectangle aSourceRect( Point( nX * (nPixelSize-1) + nOffset,  nY * (nPixelSize-1)), Size(nPixelSize, nPixelSize) );

    BitmapEx aRetval(rBitmap);
    aRetval.Crop(aSourceRect);
    return aRetval;
}

// --------------------------------------------------------------------

void SdrCropHdl::CreateB2dIAObject()
{
    // first throw away old one
    GetRidOfIAObject();

    SdrMarkView* pView = pHdlList ? pHdlList->GetView() : 0;
    SdrPageView* pPageView = pView ? pView->GetSdrPageView() : 0;

    if( pPageView && !pView->areMarkHandlesHidden() )
    {
        sal_Bool bIsFineHdl(pHdlList->IsFineHdl());
        const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
        sal_Bool bIsHighContrast(rStyleSettings.GetHighContrastMode());
        int nHdlSize = pHdlList->GetHdlSize();
        if( bIsHighContrast )
            nHdlSize = 4;

        const BitmapEx aHandlesBitmap( GetHandlesBitmap( bIsFineHdl, bIsHighContrast ) );
        BitmapEx aBmpEx1( GetBitmapForHandle( aHandlesBitmap, nHdlSize ) );

        for(sal_uInt32 b(0L); b < pPageView->PageWindowCount(); b++)
        {
            // const SdrPageViewWinRec& rPageViewWinRec = rPageViewWinList[b];
            const SdrPageWindow& rPageWindow = *pPageView->GetPageWindow(b);

            if(rPageWindow.GetPaintWindow().OutputToWindow())
            {
                if(rPageWindow.GetOverlayManager())
                {
                    basegfx::B2DPoint aPosition(aPos.X(), aPos.Y());

                    ::sdr::overlay::OverlayObject* pOverlayObject = 0L;

                    // animate focused handles
                    if(IsFocusHdl() && (pHdlList->GetFocusHdl() == this))
                    {
                        if( nHdlSize >= 2 )
                            nHdlSize = 1;

                        BitmapEx aBmpEx2( GetBitmapForHandle( aHandlesBitmap, nHdlSize + 1 ) );

                        const sal_uInt32 nBlinkTime = sal::static_int_cast<sal_uInt32>(rStyleSettings.GetCursorBlinkTime());

                        pOverlayObject = new ::sdr::overlay::OverlayAnimatedBitmapEx(aPosition, aBmpEx1, aBmpEx2, nBlinkTime,
                            (UINT16)(aBmpEx1.GetSizePixel().Width() - 1) >> 1,
                            (UINT16)(aBmpEx1.GetSizePixel().Height() - 1) >> 1,
                            (UINT16)(aBmpEx2.GetSizePixel().Width() - 1) >> 1,
                            (UINT16)(aBmpEx2.GetSizePixel().Height() - 1) >> 1);
                    }
                    else
                    {
                        // create centered handle as default
                        pOverlayObject = new ::sdr::overlay::OverlayBitmapEx(aPosition, aBmpEx1,
                            (UINT16)(aBmpEx1.GetSizePixel().Width() - 1) >> 1,
                            (UINT16)(aBmpEx1.GetSizePixel().Height() - 1) >> 1);
                    }

                    // OVERLAYMANAGER
                    if(pOverlayObject)
                    {
                        rPageWindow.GetOverlayManager()->add(*pOverlayObject);
                        maOverlayGroup.append(*pOverlayObject);
                    }
                }
            }
        }
    }
}

// --------------------------------------------------------------------
