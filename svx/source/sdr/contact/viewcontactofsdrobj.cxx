/*************************************************************************
 *
 *  $RCSfile: viewcontactofsdrobj.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-12 14:41:49 $
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

#ifndef _SDR_CONTACT_VIEWCONTACTOFSDROBJ_HXX
#include <svx/sdr/contact/viewcontactofsdrobj.hxx>
#endif

#ifndef _SDR_CONTACT_VIEWOBJECTCONTACT_HXX
#include <svx/sdr/contact/viewobjectcontact.hxx>
#endif

#ifndef _SVDOBJ_HXX
#include <svdobj.hxx>
#endif

#ifndef _SDR_CONTACT_DISPLAYINFO_HXX
#include <svx/sdr/contact/displayinfo.hxx>
#endif

#ifndef _SV_OUTDEV_HXX
#include <vcl/outdev.hxx>
#endif

#ifndef _SVDOOLE2_HXX
#include <svdoole2.hxx>
#endif

#ifndef _SVDPAGE_HXX
#include <svdpage.hxx>
#endif

#ifndef _SDR_CONTACT_VOCBITMAPBUFFER_HXX
#include <svx/sdr/contact/vocbitmapbuffer.hxx>
#endif

#ifndef _SDR_ANIMATION_AINFOTEXT_HXX
#include <svx/sdr/animation/ainfotext.hxx>
#endif

#ifndef _SDR_ANIMATION_AINFOSCROLLTEXT_HXX
#include <svx/sdr/animation/ainfoscrolltext.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
        // method to create a AnimationInfo. Needs to give a result if
        // SupportsAnimation() is overloaded and returns sal_True.
        sdr::animation::AnimationInfo* ViewContactOfSdrObj::CreateAnimationInfo()
        {
            sdr::animation::AnimationInfo* pRetval = 0L;

            if(GetSdrObject().ISA(SdrTextObj))
            {
                SdrTextObj& rTextObj = (SdrTextObj&)GetSdrObject();

                if(rTextObj.HasText())
                {
                    // Has animated text, create necessary AnimationInfo and return
                    SdrTextAniKind eAniKind = rTextObj.GetTextAniKind();

                    switch(eAniKind)
                    {
                        case SDRTEXTANI_BLINK:
                        {
                            pRetval = new sdr::animation::AInfoBlinkText(rTextObj);
                            break;
                        }
                        case SDRTEXTANI_SCROLL:
                        {
                            pRetval = new sdr::animation::AInfoScrollText(rTextObj);
                            break;
                        }
                        case SDRTEXTANI_ALTERNATE:
                        {
                            pRetval = new sdr::animation::AInfoScrollText(rTextObj, sal_True);
                            break;
                        }
                        case SDRTEXTANI_SLIDE:
                        {
                            // for simple scroll in just use the same. The parameters will be
                            // defaulted as necessary.
                            pRetval = new sdr::animation::AInfoScrollText(rTextObj, sal_False, sal_True);
                            break;
                        }
                    }
                }
            }

            if(!pRetval)
            {
                // something went wrong
                DBG_ERROR("ViewContactOfSdrObj::CreateAnimationInfo(): no anim, but SupportsAnimation (!)");
                pRetval = new sdr::animation::AInfoDummy();
            }

            return pRetval;
        }

#ifdef DBG_UTIL
static sal_Bool bTestTheBitmapBufferedObjects(sal_False);
#endif // DBG_UTIL

        // Create a Object-Specific ViewObjectContact, set ViewContact and
        // ObjectContact. Always needs to return something.
        ViewObjectContact& ViewContactOfSdrObj::CreateObjectSpecificViewObjectContact(ObjectContact& rObjectContact)
        {
            ViewObjectContact* pRetval = 0L;

            // #114233#
            if(GetSdrObject().ISA(SdrOle2Obj))
            {
                // Always buffer OLE2 objects, this needs to be moved to a
                // ViewContactOfOLE2Obj later
                pRetval = new VOCBitmapBuffer(rObjectContact, *this);
            }
            else
            {
#ifdef DBG_UTIL
                if(bTestTheBitmapBufferedObjects)
                {
                    pRetval = new VOCBitmapBuffer(rObjectContact, *this);
                }
                else
                {
#endif // DBG_UTIL
                    pRetval = new ViewObjectContact(rObjectContact, *this);
#ifdef DBG_UTIL
                }
#endif // DBG_UTIL
            }

            DBG_ASSERT(pRetval, "ViewContactOfSdrObj::CreateObjectSpecificViewObjectContact() failed (!)");
            return *pRetval;
        }

        // method to recalculate the PaintRectangle if the validity flag shows that
        // it is invalid. The flag is set from GetPaintRectangle, thus the implementation
        // only needs to refresh maPaintRectangle itself.
        void ViewContactOfSdrObj::CalcPaintRectangle()
        {
            maPaintRectangle = GetSdrObject().GetCurrentBoundRect();
        }

        // Used from ViewContactOfE3dScene and ViewContactOfGroup when
        // those groupings are empty.
        sal_Bool ViewContactOfSdrObj::PaintReplacementObject(DisplayInfo& rDisplayInfo, Rectangle& rPaintRectangle)
        {
            sal_Bool bRetval(sal_False);

            if(!rDisplayInfo.OutputToPrinter())
            {
                SdrLayerID nObjectLayerId = GetSdrObject().SdrObject::GetLayer();

                if(rDisplayInfo.GetProcessLayers().IsSet(nObjectLayerId))
                {
                    OutputDevice* pOut = rDisplayInfo.GetOutputDevice();
                    rPaintRectangle = GetPaintRectangle();

                    pOut->SetFillColor();
                    pOut->SetLineColor(COL_LIGHTGRAY);
                    pOut->DrawRect(rPaintRectangle);

                    bRetval = sal_True;
                }
            }

            return bRetval;
        }

        // For calc draft object display
        sal_Bool ViewContactOfSdrObj::PaintCalcDraftObject(DisplayInfo& rDisplayInfo, Rectangle& rPaintRectangle)
        {
            OutputDevice* pOut = rDisplayInfo.GetOutputDevice();
            rPaintRectangle = GetPaintRectangle();

            pOut->SetFillColor(COL_LIGHTGRAY);
            pOut->SetLineColor(COL_BLACK);
            pOut->DrawRect(rPaintRectangle);

            return sal_True;
        }

        ViewContactOfSdrObj::ViewContactOfSdrObj(SdrObject& rObj)
        :   ViewContact(),
            mrObject(rObj),
            meRememberedAnimationKind(SDRTEXTANI_NONE)
        {
            // init AnimationKind
            if(GetSdrObject().ISA(SdrTextObj))
            {
                SdrTextObj& rTextObj = (SdrTextObj&)GetSdrObject();
                meRememberedAnimationKind = rTextObj.GetTextAniKind();
            }
        }

        ViewContactOfSdrObj::~ViewContactOfSdrObj()
        {
        }

        // When ShouldPaintObject() returns sal_True, the object itself is painted and
        // PaintObject() is called.
        sal_Bool ViewContactOfSdrObj::ShouldPaintObject(DisplayInfo& rDisplayInfo, const ViewObjectContact& rAssociatedVOC)
        {
            // Test layer visibility
            if(!rDisplayInfo.GetProcessLayers().IsSet(GetSdrObject().GetLayer()))
            {
                return sal_False;
            }

            // Test area visibility
            const Region& rRedrawArea = rDisplayInfo.GetRedrawArea();

            if(!rRedrawArea.IsEmpty() && !rRedrawArea.IsOver(GetPaintRectangle()))
            {
                return sal_False;
            }

            // Test if print output but not printable
            if(rDisplayInfo.OutputToPrinter() && !GetSdrObject().IsPrintable())
            {
                return sal_False;
            }

            // Test calc hide/draft features
            if(!DoPaintForCalc(rDisplayInfo))
            {
                return sal_False;
            }

            // Test for hidden object on MasterPage
            if(rDisplayInfo.GetMasterPagePainting() && GetSdrObject().IsNotVisibleAsMaster())
            {
                return sal_False;
            }

            // ATM just paint it
            return sal_True;
        }

        // Paint this object. This is before evtl. SubObjects get painted. It needs to return
        // sal_True when something was pained and the paint output rectangle in rPaintRectangle.
        sal_Bool ViewContactOfSdrObj::PaintObject(DisplayInfo& rDisplayInfo, Rectangle& rPaintRectangle, const ViewObjectContact& rAssociatedVOC)
        {
            sal_Bool bRetval(sal_False);

            if(DoDraftForCalc(rDisplayInfo))
            {
                bRetval = PaintCalcDraftObject(rDisplayInfo, rPaintRectangle);
            }
            else
            {
                // Paint the object. Just hand over to the old Paint() ATM.
                GetSdrObject().DoPaintObject(
                    *rDisplayInfo.GetExtendedOutputDevice(),
                    *rDisplayInfo.GetPaintInfoRec());

                rPaintRectangle = GetPaintRectangle();
                bRetval = sal_True;
            }

            return bRetval;
        }

        // Paint this objects GluePoints. This is after PaitObject() was called.
        // This is temporarily as long as GluePoints are no handles yet.
        void ViewContactOfSdrObj::PaintGluePoints(DisplayInfo& rDisplayInfo, const ViewObjectContact& rAssociatedVOC)
        {
            const SdrGluePointList* pGluePointList = GetSdrObject().GetGluePointList();

            if(pGluePointList)
            {
                OutputDevice* pOut = rDisplayInfo.GetOutputDevice();
                pGluePointList->DrawAll(*pOut, &GetSdrObject());
            }
        }

        // New methodology to test for the new SC drawing flags (SDRPAINTMODE_SC_)
        sal_Bool ViewContactOfSdrObj::DoPaintForCalc(DisplayInfo& rDisplayInfo) const
        {
            const sal_uInt16 nPaintMode = rDisplayInfo.GetPaintInfoRec()->nPaintMode;

            if(nPaintMode & SDRPAINTMODE_SC_ALL_HIDE)
            {
                if(OBJ_OLE2 == GetSdrObject().GetObjIdentifier())
                {
                    if(((SdrOle2Obj&)GetSdrObject()).IsChart())
                    {
                        // chart
                        if(nPaintMode & SDRPAINTMODE_SC_HIDE_CHART)
                        {
                            return sal_False;
                        }
                    }
                    else
                    {
                        // OLE
                        if(nPaintMode & SDRPAINTMODE_SC_HIDE_OLE)
                        {
                            return sal_False;
                        }
                    }
                }
                else if(OBJ_GRAF == GetSdrObject().GetObjIdentifier())
                {
                    // graphic handled like OLE
                    if(nPaintMode & SDRPAINTMODE_SC_HIDE_OLE)
                    {
                        return sal_False;
                    }
                }
                else
                {
                    // any other draw object
                    if(nPaintMode & SDRPAINTMODE_SC_HIDE_DRAW)
                    {
                        return sal_False;
                    }
                }
            }

            return sal_True;
        }

        // New methodology to test for the new SC drawing flags (SDRPAINTMODE_SC_)
        sal_Bool ViewContactOfSdrObj::DoDraftForCalc(DisplayInfo& rDisplayInfo) const
        {
            const sal_uInt16 nPaintMode = rDisplayInfo.GetPaintInfoRec()->nPaintMode;

            if(nPaintMode & SDRPAINTMODE_SC_ALL_DRAFT)
            {
                if(OBJ_OLE2 == GetSdrObject().GetObjIdentifier())
                {
                    if(((SdrOle2Obj&)GetSdrObject()).IsChart())
                    {
                        // chart
                        if(nPaintMode & SDRPAINTMODE_SC_DRAFT_CHART)
                        {
                            return sal_True;
                        }
                    }
                    else
                    {
                        // OLE
                        if(nPaintMode & SDRPAINTMODE_SC_DRAFT_OLE)
                        {
                            return sal_True;
                        }
                    }
                }
                else if(OBJ_GRAF == GetSdrObject().GetObjIdentifier())
                {
                    // graphic handled like OLE
                    if(nPaintMode & SDRPAINTMODE_SC_DRAFT_OLE)
                    {
                        return sal_True;
                    }
                }
                else
                {
                    // any other draw object
                    if(nPaintMode & SDRPAINTMODE_SC_DRAFT_DRAW)
                    {
                        return sal_True;
                    }
                }
            }

            return sal_False;
        }

        // Paint a shadowed frame in object size. Fill it with a default gray if last parameter is sal_True.
        sal_Bool ViewContactOfSdrObj::PaintShadowedFrame(
            DisplayInfo& rDisplayInfo, Rectangle& rPaintRectangle, const Rectangle& rUnrotatedRectangle,
            const GeoStat& rGeometric, sal_Bool bFilled)
        {
            OutputDevice* pOutDev = rDisplayInfo.GetOutputDevice();
            const Size aPixelSize(pOutDev->PixelToLogic(Size(1, 1)));
            Rectangle aOuterRect(rUnrotatedRectangle);
            Rectangle aInnerRect(rUnrotatedRectangle);
            const Point aTopLeft(rUnrotatedRectangle.TopLeft());
            const StyleSettings& rStyleSettings = pOutDev->GetSettings().GetStyleSettings();

            aInnerRect.Left() += aPixelSize.Width();
            aInnerRect.Top() += aPixelSize.Height();
            aInnerRect.Right() -= aPixelSize.Width();
            aInnerRect.Bottom() -= aPixelSize.Height();

            Polygon aOuterPoly(aOuterRect);
            Polygon aInnerPoly(aInnerRect);

            if(rGeometric.nShearWink)
            {
                ShearPoly( aOuterPoly, aTopLeft, rGeometric.nTan );
                ShearPoly( aInnerPoly, aTopLeft, rGeometric.nTan );
            }

            if(rGeometric.nDrehWink)
            {
                RotatePoly( aOuterPoly, aTopLeft, rGeometric.nSin, rGeometric.nCos );
                RotatePoly( aInnerPoly, aTopLeft, rGeometric.nSin, rGeometric.nCos );
            }

            if(bFilled)
            {
                pOutDev->SetLineColor();
                pOutDev->SetFillColor(COL_LIGHTGRAY);
                pOutDev->DrawPolygon(aInnerPoly);
                pOutDev->SetFillColor();
            }

            pOutDev->SetLineColor(rStyleSettings.GetShadowColor());
            pOutDev->DrawPolygon(aOuterPoly);

            pOutDev->SetLineColor(rStyleSettings.GetLightColor());
            pOutDev->DrawPolygon(aInnerPoly);

            // set parameters
            rPaintRectangle = aOuterPoly.GetBoundRect();

            return sal_True;
        }

        // Paint draft text in object size.
        sal_Bool ViewContactOfSdrObj::PaintDraftText(
            DisplayInfo& rDisplayInfo, Rectangle& rPaintRectangle, const Rectangle& rUnrotatedRectangle,
            const GeoStat& rGeometric, const XubString& rDraftString, sal_Bool bUnderline)
        {
            sal_Bool bRetval;

            if(rDraftString.Len())
            {
                OutputDevice* pOutDev = rDisplayInfo.GetOutputDevice();
                const Size aPixelSize(pOutDev->PixelToLogic(Size(1, 1)));
                const Size aOutSize(rUnrotatedRectangle.GetWidth() - 6 * aPixelSize.Width(), rUnrotatedRectangle.GetHeight() - 6 * aPixelSize.Height());
                const Point aTopLeft(rUnrotatedRectangle.TopLeft());

                if(aOutSize.Width() >= (4 * aPixelSize.Width()) || aOutSize.Height() >= (4 * aPixelSize.Height()))
                {
                    Point aOutPos(rUnrotatedRectangle.Left() + 3 * aPixelSize.Width(), rUnrotatedRectangle.Top() + 3 * aPixelSize.Height());
                    sal_Int32 nMaxOutY(aOutPos.Y() + aOutSize.Height());

                    // remember font and set a new one
                    Font aSavedFont(pOutDev->GetFont());
                    Font aFont(OutputDevice::GetDefaultFont(DEFAULTFONT_SANS_UNICODE, LANGUAGE_SYSTEM, DEFAULTFONT_FLAGS_ONLYONE));

                    aFont.SetColor(COL_LIGHTRED);
                    aFont.SetTransparent(sal_True);
                    aFont.SetOrientation(sal_uInt16(NormAngle360(rGeometric.nDrehWink)/10));

                    if(bUnderline)
                    {
                        aFont.SetUnderline(UNDERLINE_SINGLE);
                    }

                    Size aFontSize( 0, (rGeometric.nDrehWink % 9000 == 0 ? 12 : 14 ) * aPixelSize.Height());

                    if(aFontSize.Height() > aOutSize.Height())
                    {
                        aFontSize.Height() = aOutSize.Height();
                    }

                    aFont.SetSize(aFontSize);
                    pOutDev->SetFont(aFont);

                    // Formatted text output
                    String aOutStr(rDraftString);

                    while(aOutStr.Len() && aOutPos.Y() <= nMaxOutY)
                    {
                        String aStr1(aOutStr);
                        sal_Int32 nTextWidth(pOutDev->GetTextWidth(aStr1));
                        sal_Int32 nTextHeight(pOutDev->GetTextHeight());

                        while(aStr1.Len() && nTextWidth > aOutSize.Width())
                        {
                            aStr1.Erase(aStr1.Len() - 1);
                            nTextWidth = pOutDev->GetTextWidth(aStr1);
                            nTextHeight = pOutDev->GetTextHeight();
                        }

                        Point aPos(aOutPos);
                        aOutPos.Y() += nTextHeight;

                        if(aOutPos.Y() <= nMaxOutY)
                        {
                            if(rGeometric.nShearWink)
                            {
                                ShearPoint(aPos, aTopLeft, rGeometric.nTan);
                            }

                            if(rGeometric.nDrehWink)
                            {
                                RotatePoint(aPos, aTopLeft, rGeometric.nSin, rGeometric.nCos);
                            }

                            pOutDev->DrawText(aPos, aStr1);
                            aOutStr.Erase(0, aStr1.Len());
                            bRetval = sal_True;
                        }
                    }

                    // restore font
                    pOutDev->SetFont(aSavedFont);

                    // calc rectangle
                    if(bRetval)
                    {
                        Polygon aPoly(rUnrotatedRectangle);

                        if(rGeometric.nShearWink)
                        {
                            ShearPoly(aPoly, aTopLeft, rGeometric.nTan);
                        }

                        if(rGeometric.nDrehWink)
                        {
                            RotatePoly(aPoly, aTopLeft, rGeometric.nSin, rGeometric.nCos);
                        }

                        rPaintRectangle = aPoly.GetBoundRect();
                    }
                }
            }

            return bRetval;
        }

        // Paint draft bitmap in object size.
        sal_Bool ViewContactOfSdrObj::PaintDraftBitmap(
            DisplayInfo& rDisplayInfo, Rectangle& rPaintRectangle, const Rectangle& rUnrotatedRectangle,
            const GeoStat& rGeometric, const Bitmap& rBitmap)
        {
            sal_Bool bRetval(sal_False);
            OutputDevice* pOutDev = rDisplayInfo.GetOutputDevice();
            const Point aTopLeft(rUnrotatedRectangle.TopLeft());
            const Size aBitmapSize(pOutDev->PixelToLogic(rBitmap.GetSizePixel()));
            const Size aPixelSize(pOutDev->PixelToLogic(Size(1, 1)));
            const sal_Int32 nDoublePixelSize(aPixelSize.Width() << 1);

            if(aBitmapSize.Width() + nDoublePixelSize <= rUnrotatedRectangle.GetWidth()
                && aBitmapSize.Height() + nDoublePixelSize<= rUnrotatedRectangle.GetHeight())
            {
                Polygon aPoly(rUnrotatedRectangle);
                Point aBitmapPos(rUnrotatedRectangle.Left() + nDoublePixelSize,
                    rUnrotatedRectangle.Top() + nDoublePixelSize);

                if(rGeometric.nShearWink)
                {
                    ShearPoint(aBitmapPos, aTopLeft, rGeometric.nTan);
                    ShearPoly(aPoly, aTopLeft, rGeometric.nTan);
                }

                if(rGeometric.nDrehWink)
                {
                    RotatePoint(aBitmapPos, aTopLeft, rGeometric.nSin, rGeometric.nCos);
                    RotatePoly(aPoly, aTopLeft, rGeometric.nSin, rGeometric.nCos);
                }

                pOutDev->DrawBitmap(aBitmapPos, aBitmapSize, rBitmap);

                // set parameters
                rPaintRectangle = aPoly.GetBoundRect();
                bRetval = sal_True;
            }

            return bRetval;
        }

        // Access to possible sub-hierarchy
        sal_uInt32 ViewContactOfSdrObj::GetObjectCount() const
        {
            if(GetSdrObject().GetSubList())
            {
                return GetSdrObject().GetSubList()->GetObjCount();
            }

            return 0L;
        }

        ViewContact& ViewContactOfSdrObj::GetViewContact(sal_uInt32 nIndex) const
        {
            DBG_ASSERT(GetSdrObject().GetSubList(),
                "ViewContactOfSdrObj::GetViewContact: Access to non-existent Sub-List (!)");
            SdrObject* pObj = GetSdrObject().GetSubList()->GetObj(nIndex);
            DBG_ASSERT(pObj, "ViewContactOfMasterPage::GetViewContact: Corrupt SdrObjList (!)");
            return pObj->GetViewContact();
        }

        sal_Bool ViewContactOfSdrObj::GetParentContacts(ViewContactVector& rVContacts) const
        {
            rVContacts.clear();
            SdrObjList* pObjList = GetSdrObject().GetObjList();

            if(pObjList)
            {
                if(pObjList->ISA(SdrPage))
                {
                    // Is a page
                    ViewContact* pParent = &(((SdrPage*)pObjList)->GetViewContact());
                    rVContacts.push_back(pParent);
                }
                else
                {
                    // Is a group?
                    if(pObjList->GetOwnerObj())
                    {
                        ViewContact* pParent = &(pObjList->GetOwnerObj()->GetViewContact());
                        rVContacts.push_back(pParent);
                    }
                }
            }

            return (0L != rVContacts.size());
        }

        // React on changes of the object of this ViewContact
        void ViewContactOfSdrObj::ActionChanged()
        {
            // look for own changes
            if(HasAnimationInfo() && SupportsAnimation())
            {
                if(GetSdrObject().ISA(SdrTextObj))
                {
                    SdrTextObj& rTextObj = (SdrTextObj&)GetSdrObject();

                    if(rTextObj.GetTextAniKind() != meRememberedAnimationKind)
                    {
                        // need to reset the animation because the type
                        // created by CreateAnimationInfo() may have changed. Do this
                        // by destroying an existing AnimationInfo.
                        DeleteAnimationInfo();
                    }
                }
            }

            // call parent
            ViewContact::ActionChanged();
        }

        // Does this ViewContact support animation?
        sal_Bool ViewContactOfSdrObj::SupportsAnimation() const
        {
            // default is no
            sal_Bool bRetval(sal_False);

            // to prepare supporting a docked text object at each DrawObject,
            // this question for TextObjects is answered at SdrObject level.
            if(GetSdrObject().ISA(SdrTextObj))
            {
                SdrTextObj& rTextObj = (SdrTextObj&)GetSdrObject();

                if( rTextObj.IsTextAnimationAllowed()
                    && rTextObj.HasText())
                {
                    // Has animated text, create necessary AnimationInfo and return
                    SdrTextAniKind eAniKind = rTextObj.GetTextAniKind();

                    switch(eAniKind)
                    {
                        case SDRTEXTANI_BLINK:
                        case SDRTEXTANI_SCROLL:
                        case SDRTEXTANI_ALTERNATE:
                        case SDRTEXTANI_SLIDE:
                        {
                            bRetval = sal_True;
                            break;
                        }
                    }
                }

                // #116168# If object is in destruction, force animation support to sal_False
                if(bRetval && GetSdrObject().IsInDestruction())
                {
                    bRetval = sal_False;
                }
            }

            return bRetval;
        }

        // overload for acessing the SdrObject
        SdrObject* ViewContactOfSdrObj::TryToGetSdrObject() const
        {
            return &GetSdrObject();
        }
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////
// eof
