/*************************************************************************
 *
 *  $RCSfile: svdhdl.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: aw $ $Date: 2001-02-22 15:09:30 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include "svdhdl.hxx"
#include "svdtouch.hxx"
#include "svdpagv.hxx"
#include "svdetc.hxx"
#include "svdmrkv.hxx"

#ifndef _WINDOW_HXX //autogen
#include <vcl/window.hxx>
#endif

#ifndef _VIRDEV_HXX //autogen
#include <vcl/virdev.hxx>
#endif

#ifndef _POLY_HXX //autogen
#include <vcl/poly.hxx>
#endif

#ifndef _SV_BMPACC_HXX
#include <vcl/bmpacc.hxx>
#endif

#ifndef _B2D_MTRI_HXX
#include <goodies/b2dmtri.hxx>
#endif

#ifndef _B3D_B3DCOLOR_HXX
#include <goodies/b3dcolor.hxx>
#endif

#include "sxekitm.hxx"
#include "sxesitm.hxx"
#include "svdstr.hrc"
#include "svdglob.hxx"

#include "svdmodel.hxx"
#include "gradtrns.hxx"
#include "xflgrit.hxx"
#include "svdundo.hxx"
#include "dialmgr.hxx"
#include "xflftrit.hxx"

////////////////////////////////////////////////////////////////////////////////////////////////////

class SdrHdlBitmapSet
{
    BitmapEx                aRect_7x7[5];
    BitmapEx                aRect_9x9[5];
    BitmapEx                aRect_11x11[5];
    BitmapEx                aRect_13x13[5];
    BitmapEx                aCirc_7x7[5];
    BitmapEx                aCirc_9x9[5];
    BitmapEx                aCirc_11x11[5];
    BitmapEx                aElli_7x9[5];
    BitmapEx                aElli_9x11[5];
    BitmapEx                aElli_9x7[5];
    BitmapEx                aElli_11x9[5];
    BitmapEx                aRectPlus_7x7[5];
    BitmapEx                aRectPlus_9x9[5];
    BitmapEx                aRectPlus_11x11[5];
    BitmapEx                aCrosshair;
    BitmapEx                aGlue;
    BitmapEx                aAnchor;

    void FillBitmapsFromResource(UINT16 nResId);

public:
    SdrHdlBitmapSet(UINT16 nResId);
    ~SdrHdlBitmapSet();

    BitmapEx& GetBitmapEx(BitmapMarkerKind eKindOfMarker, UINT16 nInd=0);
};

////////////////////////////////////////////////////////////////////////////////////////////////////

SdrHdlBitmapSet::SdrHdlBitmapSet(UINT16 nResId)
{
    FillBitmapsFromResource(nResId);
}

SdrHdlBitmapSet::~SdrHdlBitmapSet()
{
}

void SdrHdlBitmapSet::FillBitmapsFromResource(UINT16 nResId)
{
    Color aColWhite(COL_WHITE);
    OutputDevice* pOut = Application::GetDefaultDevice();
    Bitmap aBitmap(ResId(nResId, ImpGetResMgr()));
    BitmapEx aMarkersBitmap(aBitmap, aColWhite);

    for(UINT16 a=0;a<5;a++)
    {
        UINT16 nYPos = a * 11;

        aRect_7x7[a] = aMarkersBitmap; aRect_7x7[a].Crop(Rectangle(Point(0, nYPos), Size(7, 7)));
        aRect_7x7[a] = BitmapEx( aRect_7x7[a].GetBitmap().CreateDisplayBitmap( pOut ),
            aRect_7x7[a].GetMask().CreateDisplayBitmap( pOut ) );

        aRect_9x9[a] = aMarkersBitmap; aRect_9x9[a].Crop(Rectangle(Point(7, nYPos), Size(9, 9)));
        aRect_9x9[a] = BitmapEx( aRect_9x9[a].GetBitmap().CreateDisplayBitmap( pOut ),
            aRect_9x9[a].GetMask().CreateDisplayBitmap( pOut ) );

        aRect_11x11[a] = aMarkersBitmap; aRect_11x11[a].Crop(Rectangle(Point(16, nYPos), Size(11, 11)));
        aRect_11x11[a] = BitmapEx( aRect_11x11[a].GetBitmap().CreateDisplayBitmap( pOut ),
            aRect_11x11[a].GetMask().CreateDisplayBitmap( pOut ) );

        aCirc_7x7[a] = aMarkersBitmap; aCirc_7x7[a].Crop(Rectangle(Point(27, nYPos), Size(7, 7)));
        aCirc_7x7[a] = BitmapEx( aCirc_7x7[a].GetBitmap().CreateDisplayBitmap( pOut ),
            aCirc_7x7[a].GetMask().CreateDisplayBitmap( pOut ) );

        aCirc_9x9[a] = aMarkersBitmap; aCirc_9x9[a].Crop(Rectangle(Point(34, nYPos), Size(9, 9)));
        aCirc_9x9[a] = BitmapEx( aCirc_9x9[a].GetBitmap().CreateDisplayBitmap( pOut ),
            aCirc_9x9[a].GetMask().CreateDisplayBitmap( pOut ) );

        aCirc_11x11[a] = aMarkersBitmap; aCirc_11x11[a].Crop(Rectangle(Point(43, nYPos), Size(11, 11)));
        aCirc_11x11[a] = BitmapEx( aCirc_11x11[a].GetBitmap().CreateDisplayBitmap( pOut ),
            aCirc_11x11[a].GetMask().CreateDisplayBitmap( pOut ) );

        aElli_7x9[a] = aMarkersBitmap; aElli_7x9[a].Crop(Rectangle(Point(54, nYPos), Size(7, 9)));
        aElli_7x9[a] = BitmapEx( aElli_7x9[a].GetBitmap().CreateDisplayBitmap( pOut ),
            aElli_7x9[a].GetMask().CreateDisplayBitmap( pOut ) );

        aElli_9x11[a] = aMarkersBitmap; aElli_9x11[a].Crop(Rectangle(Point(61, nYPos), Size(9, 11)));
        aElli_9x11[a] = BitmapEx( aElli_9x11[a].GetBitmap().CreateDisplayBitmap( pOut ),
            aElli_9x11[a].GetMask().CreateDisplayBitmap( pOut ) );

        aElli_9x7[a] = aMarkersBitmap; aElli_9x7[a].Crop(Rectangle(Point(70, nYPos), Size(9, 7)));
        aElli_9x7[a] = BitmapEx( aElli_9x7[a].GetBitmap().CreateDisplayBitmap( pOut ),
            aElli_9x7[a].GetMask().CreateDisplayBitmap( pOut ) );

        aElli_11x9[a] = aMarkersBitmap; aElli_11x9[a].Crop(Rectangle(Point(79, nYPos), Size(11, 9)));
        aElli_11x9[a] = BitmapEx( aElli_11x9[a].GetBitmap().CreateDisplayBitmap( pOut ),
            aElli_11x9[a].GetMask().CreateDisplayBitmap( pOut ) );

        aRectPlus_7x7[a] = aMarkersBitmap; aRectPlus_7x7[a].Crop(Rectangle(Point(90, nYPos), Size(7, 7)));
        aRectPlus_7x7[a] = BitmapEx( aRectPlus_7x7[a].GetBitmap().CreateDisplayBitmap( pOut ),
            aRectPlus_7x7[a].GetMask().CreateDisplayBitmap( pOut ) );

        aRectPlus_9x9[a] = aMarkersBitmap; aRectPlus_9x9[a].Crop(Rectangle(Point(97, nYPos), Size(9, 9)));
        aRectPlus_9x9[a] = BitmapEx( aRectPlus_9x9[a].GetBitmap().CreateDisplayBitmap( pOut ),
            aRectPlus_9x9[a].GetMask().CreateDisplayBitmap( pOut ) );

        aRectPlus_11x11[a] = aMarkersBitmap; aRectPlus_11x11[a].Crop(Rectangle(Point(106, nYPos), Size(11, 11)));
        aRectPlus_11x11[a] = BitmapEx( aRectPlus_11x11[a].GetBitmap().CreateDisplayBitmap( pOut ),
            aRectPlus_11x11[a].GetMask().CreateDisplayBitmap( pOut ) );
    }

    aRect_13x13[0] = aMarkersBitmap; aRect_13x13[0].Crop(Rectangle(Point(71, 53), Size(13, 13)));
    aRect_13x13[0] = BitmapEx( aRect_13x13[0].GetBitmap().CreateDisplayBitmap( pOut ),
        aRect_13x13[0].GetMask().CreateDisplayBitmap( pOut ) );
    aRect_13x13[1] = aMarkersBitmap; aRect_13x13[1].Crop(Rectangle(Point(85, 53), Size(13, 13)));
    aRect_13x13[1] = BitmapEx( aRect_13x13[1].GetBitmap().CreateDisplayBitmap( pOut ),
        aRect_13x13[1].GetMask().CreateDisplayBitmap( pOut ) );
    aRect_13x13[2] = aMarkersBitmap; aRect_13x13[2].Crop(Rectangle(Point(72, 65), Size(13, 13)));
    aRect_13x13[2] = BitmapEx( aRect_13x13[2].GetBitmap().CreateDisplayBitmap( pOut ),
        aRect_13x13[2].GetMask().CreateDisplayBitmap( pOut ) );
    aRect_13x13[3] = aMarkersBitmap; aRect_13x13[3].Crop(Rectangle(Point(85, 65), Size(13, 13)));
    aRect_13x13[3] = BitmapEx( aRect_13x13[3].GetBitmap().CreateDisplayBitmap( pOut ),
        aRect_13x13[3].GetMask().CreateDisplayBitmap( pOut ) );
    aRect_13x13[4] = aMarkersBitmap; aRect_13x13[4].Crop(Rectangle(Point(98, 65), Size(13, 13)));
    aRect_13x13[4] = BitmapEx( aRect_13x13[4].GetBitmap().CreateDisplayBitmap( pOut ),
        aRect_13x13[4].GetMask().CreateDisplayBitmap( pOut ) );

    aCrosshair = aMarkersBitmap; aCrosshair.Crop(Rectangle(Point(0, 55), Size(15, 15)));
    aCrosshair = BitmapEx( aCrosshair.GetBitmap().CreateDisplayBitmap( pOut ),
        aCrosshair.GetMask().CreateDisplayBitmap( pOut ) );

    aGlue = aMarkersBitmap; aGlue.Crop(Rectangle(Point(15, 61), Size(9, 9)));
    aGlue = BitmapEx( aGlue.GetBitmap().CreateDisplayBitmap( pOut ),
        aGlue.GetMask().CreateDisplayBitmap( pOut ) );

    aAnchor = aMarkersBitmap; aAnchor.Crop(Rectangle(Point(24, 55), Size(24, 23)));
    aAnchor = BitmapEx( aAnchor.GetBitmap().CreateDisplayBitmap( pOut ),
        aAnchor.GetMask().CreateDisplayBitmap( pOut ) );
}

BitmapEx& SdrHdlBitmapSet::GetBitmapEx(BitmapMarkerKind eKindOfMarker, UINT16 nInd)
{
    switch(eKindOfMarker)
    {
        case Rect_7x7: return aRect_7x7[nInd]; break;
        case Rect_9x9: return aRect_9x9[nInd]; break;
        case Rect_11x11: return aRect_11x11[nInd]; break;
        case Rect_13x13: return aRect_13x13[nInd]; break;
        case Circ_7x7: return aCirc_7x7[nInd]; break;
        case Circ_9x9: return aCirc_9x9[nInd]; break;
        case Circ_11x11: return aCirc_11x11[nInd]; break;
        case Elli_7x9: return aElli_7x9[nInd]; break;
        case Elli_9x11: return aElli_9x11[nInd]; break;
        case Elli_9x7: return aElli_9x7[nInd]; break;
        case Elli_11x9: return aElli_11x9[nInd]; break;
        case RectPlus_7x7: return aRectPlus_7x7[nInd]; break;
        case RectPlus_9x9: return aRectPlus_9x9[nInd]; break;
        case RectPlus_11x11: return aRectPlus_11x11[nInd]; break;
        case Crosshair: return aCrosshair; break;
        case Glue: return aGlue; break;
        case Anchor: return aAnchor; break;
        default: DBG_ERROR( "unknown kind of marker" ); return aRect_7x7[nInd]; break;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

SdrHdlBitmapSet* SdrHdl::pSimpleSet = NULL;
SdrHdlBitmapSet* SdrHdl::pModernSet = NULL;

////////////////////////////////////////////////////////////////////////////////////////////////////

SdrHdl::SdrHdl():
    pObj(NULL),
    pPV(NULL),
    pHdlList(NULL),
    nDrehWink(0),
    eKind(HDL_MOVE),
    bSelect(FALSE),
    b1PixMore(FALSE),
    nObjHdlNum(0),
    nPolyNum(0),
    nPPntNum(0),
    bPlusHdl(FALSE),
    nSourceHdlNum(0)
{
    if(!pSimpleSet)
        pSimpleSet = new SdrHdlBitmapSet(SIP_SA_MARKERS);
    DBG_ASSERT(pSimpleSet, "Could not construct SdrHdlBitmapSet()!");

    if(!pModernSet)
        pModernSet = new SdrHdlBitmapSet(SIP_SA_FINE_MARKERS);
    DBG_ASSERT(pModernSet, "Could not construct SdrHdlBitmapSet()!");
}

SdrHdl::SdrHdl(const Point& rPnt, SdrHdlKind eNewKind):
    aPos(rPnt),
    pObj(NULL),
    pPV(NULL),
    pHdlList(NULL),
    nDrehWink(0),
    eKind(eNewKind),
    bSelect(FALSE),
    b1PixMore(FALSE),
    nObjHdlNum(0),
    nPolyNum(0),
    nPPntNum(0),
    bPlusHdl(FALSE),
    nSourceHdlNum(0)
{
    if(!pSimpleSet)
        pSimpleSet = new SdrHdlBitmapSet(SIP_SA_MARKERS);
    DBG_ASSERT(pSimpleSet, "Could not construct SdrHdlBitmapSet()!");

    if(!pModernSet)
        pModernSet = new SdrHdlBitmapSet(SIP_SA_FINE_MARKERS);
    DBG_ASSERT(pModernSet, "Could not construct SdrHdlBitmapSet()!");
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
    aIAOGroup.Delete();
}

void SdrHdl::CreateB2dIAObject()
{
    // first throw away old one
    GetRidOfIAObject();

    if(pHdlList && pHdlList->GetView() && !pHdlList->GetView()->IsMarkHdlHidden())
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
        }

        for(UINT16 a=0;a<pHdlList->GetView()->GetWinCount();a++)
        {
            SdrViewWinRec& rRec = pHdlList->GetView()->GetWinRec(a);
            Point aMoveOutsideOffset(0, 0);

            // add offset if necessary
            if(rRec.pWin && pHdlList->IsMoveOutside())
            {
                Size aOffset = rRec.pWin->PixelToLogic(Size(4, 4));

                if(eKind == HDL_UPLFT || eKind == HDL_UPPER || eKind == HDL_UPRGT)
                    aMoveOutsideOffset.Y() -= aOffset.Width();
                if(eKind == HDL_LWLFT || eKind == HDL_LOWER || eKind == HDL_LWRGT)
                    aMoveOutsideOffset.Y() += aOffset.Height();
                if(eKind == HDL_UPLFT || eKind == HDL_LEFT  || eKind == HDL_LWLFT)
                    aMoveOutsideOffset.X() -= aOffset.Width();
                if(eKind == HDL_UPRGT || eKind == HDL_RIGHT || eKind == HDL_LWRGT)
                    aMoveOutsideOffset.X() += aOffset.Height();
            }

            // Manager may be zero when printing or drawing to VDevs
            if(rRec.pIAOManager)
            {
                B2dIAObject* pNew = CreateMarkerObject(
                    rRec.pIAOManager,
                    aPos + aMoveOutsideOffset,
                    eColIndex,
                    eKindOfMarker);

                if(pNew)
                {
                    // set as B2DIAObject
                    aIAOGroup.InsertIAO(pNew);
                }
            }
        }
    }
}

B2dIAObject* SdrHdl::CreateMarkerObject(B2dIAOManager* pMan, Point aPos, BitmapColorIndex eColIndex, BitmapMarkerKind eKindOfMarker)
{
    // support bigger sizes
    if(pHdlList->GetHdlSize() > 3)
    {
        switch(eKindOfMarker)
        {
            case Rect_7x7: eKindOfMarker = Rect_9x9; break;
            case Rect_9x9: eKindOfMarker = Rect_11x11; break;
            case Rect_11x11: eKindOfMarker = Rect_13x13; break;
            case Circ_7x7: eKindOfMarker = Circ_9x9; break;
            case Circ_9x9: eKindOfMarker = Circ_11x11; break;
            case Elli_7x9: eKindOfMarker = Elli_9x11; break;
            case Elli_9x7: eKindOfMarker = Elli_11x9; break;
            case RectPlus_7x7: eKindOfMarker = RectPlus_9x9; break;
            case RectPlus_9x9: eKindOfMarker = RectPlus_11x11; break;
        }
    }

    BitmapEx& rBmpEx = pHdlList->IsFineHdl()
        ? pModernSet->GetBitmapEx(eKindOfMarker, (UINT16)eColIndex)
        : pSimpleSet->GetBitmapEx(eKindOfMarker, (UINT16)eColIndex);

    if(eKindOfMarker != Anchor)
        return new B2dIAOBitmapExReference(pMan, aPos, &rBmpEx,
            (UINT16)(rBmpEx.GetSizePixel().Width() - 1) >> 1,
            (UINT16)(rBmpEx.GetSizePixel().Height() - 1) >> 1);
    else
        return new B2dIAOBitmapExReference(pMan, aPos, &rBmpEx);
}

BOOL SdrHdl::IsHit(const Point& rPnt, const OutputDevice& rOut) const
{
    if(aIAOGroup.GetIAOCount())
    {
        Point aPixelPos(rOut.LogicToPixel(rPnt));
        return aIAOGroup.IsHit(aPixelPos);
    }
    else
        return FALSE;
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
            }
        } else {
            switch (eKind) {
                case HDL_UPLFT: ePtr=POINTER_SESIZE;  break;
                case HDL_UPPER: ePtr=POINTER_SSIZE;     break;
                case HDL_UPRGT: ePtr=POINTER_NESIZE;  break;
                case HDL_LEFT : ePtr=POINTER_ESIZE;     break;
                case HDL_RIGHT: ePtr=POINTER_ESIZE;     break;
                case HDL_LWLFT: ePtr=POINTER_NESIZE;  break;
                case HDL_LOWER: ePtr=POINTER_SSIZE;     break;
                case HDL_LWRGT: ePtr=POINTER_SESIZE;  break;
                case HDL_POLY : ePtr=POINTER_MOVEPOINT; break;
                case HDL_CIRC : ePtr=POINTER_HAND;      break;
                case HDL_REF1 : ePtr=POINTER_REFHAND;   break;
                case HDL_REF2 : ePtr=POINTER_REFHAND;   break;
                case HDL_BWGT : ePtr=POINTER_MOVEBEZIERWEIGHT; break;
                case HDL_GLUE : ePtr=POINTER_MOVEPOINT; break;
            }
        }
    }
    return Pointer(ePtr);
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

    if(pHdlList && pHdlList->GetView() && !pHdlList->GetView()->IsMarkHdlHidden())
    {
        for(UINT16 a=0;a<pHdlList->GetView()->GetWinCount();a++)
        {
            SdrViewWinRec& rRec = pHdlList->GetView()->GetWinRec(a);

            // Manager may be zero when printing or drawing to VDevs
            if(rRec.pIAOManager)
            {
                Bitmap aBmpCol(CreateColorDropper(aMarkerColor));
                B2dIAObject* pNew = new B2dIAOBitmapObj(rRec.pIAOManager, aPos, aBmpCol,
                    (UINT16)(aBmpCol.GetSizePixel().Width() - 1) >> 1,
                    (UINT16)(aBmpCol.GetSizePixel().Height() - 1) >> 1);
                DBG_ASSERT(pNew, "Got NO new IAO!");
                aIAOGroup.InsertIAO(pNew);
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
    a2ndPos(rRef2),
    bGradient(bGrad),
    pColHdl1(NULL),
    pColHdl2(NULL)
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

    if(pHdlList && pHdlList->GetView() && !pHdlList->GetView()->IsMarkHdlHidden())
    {
        for(UINT16 a=0;a<pHdlList->GetView()->GetWinCount();a++)
        {
            SdrViewWinRec& rRec = pHdlList->GetView()->GetWinRec(a);

            // Manager may be zero when printing or drawing to VDevs
            if(rRec.pIAOManager)
            {
                // striped line in between
                Vector2D aVec(a2ndPos.X() - aPos.X(), a2ndPos.Y() - aPos.Y());
                double fVecLen = aVec.GetLength();
                double fLongPercentArrow = (1.0 - 0.05) * fVecLen;
                double fHalfArrowWidth = (0.05 * 0.5) * fVecLen;
                aVec.Normalize();
                Vector2D aPerpend(-aVec.Y(), aVec.X());
                INT32 nMidX = (INT32)(aPos.X() + aVec.X() * fLongPercentArrow);
                INT32 nMidY = (INT32)(aPos.Y() + aVec.Y() * fLongPercentArrow);
                Point aMidPoint(nMidX, nMidY);
                B2dIAObject* pNew = new B2dIAOLineStriped(rRec.pIAOManager, aPos, aMidPoint, 4);
                DBG_ASSERT(pNew, "Got NO new IAO!");
                pNew->SetBaseColor(IsGradient() ? Color(COL_BLACK) : Color(COL_BLUE));
                aIAOGroup.InsertIAO(pNew);

                // arrowhead
                Point aLeft(aMidPoint.X() + (INT32)(aPerpend.X() * fHalfArrowWidth),
                            aMidPoint.Y() + (INT32)(aPerpend.Y() * fHalfArrowWidth));
                Point aRight(aMidPoint.X() - (INT32)(aPerpend.X() * fHalfArrowWidth),
                            aMidPoint.Y() - (INT32)(aPerpend.Y() * fHalfArrowWidth));
                pNew = new B2dIAOBitmapTriangle(rRec.pIAOManager, aLeft, a2ndPos, aRight, IsGradient() ? Color(COL_BLACK) : Color(COL_BLUE));
                DBG_ASSERT(pNew, "Got NO new IAO!");
                aIAOGroup.InsertIAO(pNew);
            }
        }
    }
}

IMPL_LINK(SdrHdlGradient, ColorChangeHdl, SdrHdl*, pHdl)
{
    if(GetObj())
        FromIAOToItem(GetObj(), TRUE, TRUE);
    return NULL;
}

void SdrHdlGradient::FromIAOToItem(SdrObject* pObj, BOOL bSetItemOnObject, BOOL bUndo)
{
    // from IAO positions and colors to gradient
    const SfxItemSet& rSet = pObj->GetItemSet();

    GradTransformer aGradTransformer;
    GradTransGradient aOldGradTransGradient;
    GradTransGradient aGradTransGradient;
    GradTransVector aGradTransVector;

    String aString;

    aGradTransVector.aPos1 = GetPos();
    aGradTransVector.aPos2 = Get2ndPos();
    if(pColHdl1)
        aGradTransVector.aCol1 = pColHdl1->GetColor();
    if(pColHdl2)
        aGradTransVector.aCol2 = pColHdl2->GetColor();

    if(IsGradient())
        aOldGradTransGradient.aGradient = ((XFillGradientItem&)rSet.Get(XATTR_FILLGRADIENT)).GetValue();
    else
        aOldGradTransGradient.aGradient = ((XFillFloatTransparenceItem&)rSet.Get(XATTR_FILLFLOATTRANSPARENCE)).GetValue();

    // transform vector data to gradient
    aGradTransformer.VecToGrad(aGradTransVector, aGradTransGradient, aOldGradTransGradient, pObj, bMoveSingleHandle, bMoveFirstHandle);

    if(bSetItemOnObject)
    {
        SdrModel* pModel = pObj->GetModel();
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
            pModel->AddUndo(new SdrUndoAttrObj(*pObj));
            pModel->EndUndo();
        }

        pObj->SetItemSetAndBroadcast(aNewSet);
    }

    // back transformation, set values on pIAOHandle
    aGradTransformer.GradToVec(aGradTransGradient, aGradTransVector, pObj);

    SetPos(aGradTransVector.aPos1);
    Set2ndPos(aGradTransVector.aPos2);
    if(pColHdl1)
    {
        pColHdl1->SetPos(aGradTransVector.aPos1);
        pColHdl1->SetColor(aGradTransVector.aCol1);
    }
    if(pColHdl2)
    {
        pColHdl2->SetPos(aGradTransVector.aPos2);
        pColHdl2->SetColor(aGradTransVector.aCol2);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

SdrHdlLine::~SdrHdlLine() {}

void SdrHdlLine::CreateB2dIAObject()
{
    // first throw away old one
    GetRidOfIAObject();

    if(pHdlList && pHdlList->GetView() && !pHdlList->GetView()->IsMarkHdlHidden() && pHdl1 && pHdl2)
    {
        for(UINT16 a=0;a<pHdlList->GetView()->GetWinCount();a++)
        {
            SdrViewWinRec& rRec = pHdlList->GetView()->GetWinRec(a);

            // Manager may be zero when printing or drawing to VDevs
            if(rRec.pIAOManager)
            {
                B2dIAObject* pNew = new B2dIAOLine(
                    rRec.pIAOManager,
                    pHdl1->GetPos(),
                    pHdl2->GetPos());

                if(pNew)
                {
                    // color(?)
                    pNew->SetBaseColor(Color(COL_LIGHTRED));

                    // set as B2DIAObject
                    aIAOGroup.InsertIAO(pNew);
                }
            }
        }
    }
}

BOOL SdrHdlLine::IsHit(const Point& rPnt, const OutputDevice& rOut) const
{
    if(aIAOGroup.GetIAOCount())
    {
        Point aPixelPos(rOut.LogicToPixel(rPnt));
        return aIAOGroup.IsHit(aPixelPos, 2);
    }
    else
        return FALSE;
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
    if(pHdlList && pHdlList->GetView() && !pHdlList->GetView()->IsMarkHdlHidden())
    {
        for(UINT16 a=0;a<pHdlList->GetView()->GetWinCount();a++)
        {
            SdrViewWinRec& rRec = pHdlList->GetView()->GetWinRec(a);

            // Manager may be zero when printing or drawing to VDevs
            if(rRec.pIAOManager)
            {
                B2dIAObject* pNew = new B2dIAOLine(
                    rRec.pIAOManager,
                    pHdl1->GetPos(),
                    aPos);

                if(pNew)
                {
                    // line part is not hittable
                    pNew->SetHittable(FALSE);

                    // color(?)
                    pNew->SetBaseColor(Color(COL_LIGHTBLUE));

                    // set as B2DIAObject
                    aIAOGroup.InsertIAO(pNew);
                }
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

E3dVolumeMarker::E3dVolumeMarker(const XPolyPolygon& rXPP)
{
    aWireframePoly = rXPP;
}

void E3dVolumeMarker::CreateB2dIAObject()
{
    // create lines
    if(pHdlList && pHdlList->GetView() && !pHdlList->GetView()->IsMarkHdlHidden())
    {
        for(UINT16 a=0;a<pHdlList->GetView()->GetWinCount();a++)
        {
            SdrViewWinRec& rRec = pHdlList->GetView()->GetWinRec(a);

            // Manager may be zero when printing or drawing to VDevs
            if(rRec.pIAOManager)
            {
                UINT16 nCnt = aWireframePoly.Count();
                for(UINT16 i = 0; i < nCnt; i++)
                {
                    B2dIAObject* pNew = new B2dIAOLine(
                        rRec.pIAOManager,
                        aWireframePoly[i][0],
                        aWireframePoly[i][1]);

                    if(pNew)
                    {
                        // color(?)
                        pNew->SetBaseColor(Color(COL_BLACK));

                        // set as B2DIAObject
                        aIAOGroup.InsertIAO(pNew);
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

        if(pHdlList && pHdlList->GetView() && !pHdlList->GetView()->IsMarkHdlHidden())
        {
            const SdrEdgeObj* pEdge = (SdrEdgeObj*)pObj;

            if(pEdge->GetConnectedNode(nObjHdlNum == 0) != NULL)
                eColIndex = LightRed;

            if(nPPntNum < 2)
            {
                // Handle with plus sign inside
                eKindOfMarker = Circ_7x7;
            }

            for(UINT16 a=0;a<pHdlList->GetView()->GetWinCount();a++)
            {
                SdrViewWinRec& rRec = pHdlList->GetView()->GetWinRec(a);

                // Manager may be zero when printing or drawing to VDevs
                if(rRec.pIAOManager)
                {
                    B2dIAObject* pNew = CreateMarkerObject(
                        rRec.pIAOManager,
                        aPos,
                        eColIndex,
                        eKindOfMarker);

                    if(pNew)
                    {
                        // set as B2DIAObject
                        aIAOGroup.InsertIAO(pNew);
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
        CreateB2dIAObject();
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

    SdrEdgeKind eKind = ((SdrEdgeKindItem&)(pEdge->GetItem(SDRATTR_EDGEKIND))).GetValue();

    const SdrEdgeInfoRec& rInfo=pEdge->aEdgeInfo;
    if (eKind==SDREDGE_ORTHOLINES || eKind==SDREDGE_BEZIER) {
        return !rInfo.ImpIsHorzLine(eLineCode,*pEdge->pEdgeTrack);
    } else if (eKind==SDREDGE_THREELINES) {
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

    if(pHdlList && pHdlList->GetView() && !pHdlList->GetView()->IsMarkHdlHidden())
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

        for(UINT16 a=0;a<pHdlList->GetView()->GetWinCount();a++)
        {
            SdrViewWinRec& rRec = pHdlList->GetView()->GetWinRec(a);

            // Manager may be zero when printing or drawing to VDevs
            if(rRec.pIAOManager)
            {
                B2dIAObject* pNew = CreateMarkerObject(
                    rRec.pIAOManager,
                    aPos,
                    eColIndex,
                    eKindOfMarker);

                if(pNew)
                {
                    // set as B2DIAObject
                    aIAOGroup.InsertIAO(pNew);
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
        if (eKind2==HDL_REF1 || eKind2==HDL_REF2 || eKind2==HDL_MIRX) n2=5;
        else if (eKind2==HDL_GLUE) n2=2;
        else if (eKind2==HDL_USER) n2=3;
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
                USHORT nNum1=((SdrHdl*)pElem1)->GetObjHdlNum();
                USHORT nNum2=((SdrHdl*)pElem2)->GetObjHdlNum();
                if (nNum1==nNum2) { // #48763#
                    if (eKind1==eKind2) (long)pElem1<(long)pElem2 ? -1 : 1; // Notloesung, um immer die gleiche Sortierung zu haben
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

////////////////////////////////////////////////////////////////////////////////////////////////////

SdrHdlList::SdrHdlList(SdrMarkView* pV)
:   aList(1024,32,32),
    pView(pV)
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

    // immediately remove from display
    if(pView)
        pView->RefreshAllIAOManagers();

    bRotateShear=FALSE;
    bDistortShear=FALSE;
}

void SdrHdlList::Sort()
{
    ImpSdrHdlListSorter aSort(aList);
    aSort.DoSort();
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

SdrHdl* SdrHdlList::HitTest(const Point& rPnt, const OutputDevice& rOut, BOOL bBack, BOOL bNext, SdrHdl* pHdl0) const
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
           if (pHdl->IsHit(rPnt,rOut)) pRet=pHdl;
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

