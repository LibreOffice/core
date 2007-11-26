/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: viewcontactofsdrobj.cxx,v $
 *
 *  $Revision: 1.20 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-26 14:52:28 $
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

#ifndef _SDR_CONTACT_VIEWCONTACTOFSDROBJ_HXX
#include <svx/sdr/contact/viewcontactofsdrobj.hxx>
#endif

#ifndef _SDR_CONTACT_VIEWOBJECTCONTACT_HXX
#include <svx/sdr/contact/viewobjectcontact.hxx>
#endif

#ifndef _SVDOBJ_HXX
#include <svx/svdobj.hxx>
#endif

#ifndef _SDR_CONTACT_DISPLAYINFO_HXX
#include <svx/sdr/contact/displayinfo.hxx>
#endif

#ifndef _SV_OUTDEV_HXX
#include <vcl/outdev.hxx>
#endif

#ifndef _SVDOOLE2_HXX
#include <svx/svdoole2.hxx>
#endif

#ifndef _SVDPAGE_HXX
#include <svx/svdpage.hxx>
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

#ifndef _SDR_CONTACT_OBJECTCONTACTOFPAGEVIEW_HXX
#include <svx/sdr/contact/objectcontactofpageview.hxx>
#endif

#ifndef _SDRPAGEWINDOW_HXX
#include <svx/sdrpagewindow.hxx>
#endif

#ifndef _SDRPAINTWINDOW_HXX
#include <sdrpaintwindow.hxx>
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
                        case SDRTEXTANI_ALTERNATE:
                        case SDRTEXTANI_SLIDE:
                        {
                            // #i38135# Use new implementation of AInfoScrollText
                            pRetval = new sdr::animation::AInfoScrollText(rTextObj);
                            break;
                        }
                        default: break;
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

        // Create a Object-Specific ViewObjectContact, set ViewContact and
        // ObjectContact. Always needs to return something.
        ViewObjectContact& ViewContactOfSdrObj::CreateObjectSpecificViewObjectContact(ObjectContact& rObjectContact)
        {
            // #i42323#
            // The reason for #114233# is gone, so i remove it again
            ViewObjectContact& rRetval = ViewContact::CreateObjectSpecificViewObjectContact(rObjectContact);
            DBG_ASSERT(&rRetval, "ViewContactOfSdrObj::CreateObjectSpecificViewObjectContact() failed (!)");
            return rRetval;
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
            // Before we can test whether the paint rectangle has an
            // intersection with the redraw area we enlarge the paint
            // rectangle by the equivalent in logical coordinates of one
            // pixel.  This takes care of rounding errors in VCL of the
            // redraw area (#i81212#). (The redraw area is the one which
            // should be enlarged but it may consist of more than one
            // rectangle and enlarging the paint rectangle leads to the same
            // result).  This can be considered a hack.  Not because its
            // there but because it is here.  It belongs to VCL where
            // internally the redraw rectangles are transformed into pixels
            // and back again.
            Rectangle aPaintRectangle (GetPaintRectangle());
            ObjectContactOfPageView* pObjectContact
                = dynamic_cast<ObjectContactOfPageView*>(&rAssociatedVOC.GetObjectContact());
            if (pObjectContact != NULL)
            {
                OutputDevice& rDevice = pObjectContact->GetPageWindow().GetPaintWindow().GetOutputDevice();
                Size aOnePixel (rDevice.PixelToLogic(Size(1,1)));
                aPaintRectangle.Left() -= aOnePixel.Width();
                aPaintRectangle.Right() += aOnePixel.Width();
                aPaintRectangle.Top() -= aOnePixel.Height();
                aPaintRectangle.Bottom() += aOnePixel.Height();
            }
            if(!rRedrawArea.IsEmpty() && !rRedrawArea.IsOver(aPaintRectangle))
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
        sal_Bool ViewContactOfSdrObj::PaintObject(DisplayInfo& rDisplayInfo, Rectangle& rPaintRectangle, const ViewObjectContact& /*rAssociatedVOC*/)
        {
            sal_Bool bRetval(sal_False);

            // Paint the object. Just hand over to the old Paint() ATM.
            GetSdrObject().DoPaintObject(
                *rDisplayInfo.GetExtendedOutputDevice(),
                *rDisplayInfo.GetPaintInfoRec());

            rPaintRectangle = GetPaintRectangle();
            bRetval = sal_True;

            return bRetval;
        }

        // Paint this objects GluePoints. This is after PaitObject() was called.
        // This is temporarily as long as GluePoints are no handles yet.
        void ViewContactOfSdrObj::PaintGluePoints(DisplayInfo& rDisplayInfo, const ViewObjectContact& /*rAssociatedVOC*/)
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
            sal_Bool bRetval(sal_False);

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
            DBG_ASSERT(pObj, "ViewContactOfSdrObj::GetViewContact: Corrupt SdrObjList (!)");
            return pObj->GetViewContact();
        }

        ViewContact* ViewContactOfSdrObj::GetParentContact() const
        {
            ViewContact* pRetval = 0L;
            SdrObjList* pObjList = GetSdrObject().GetObjList();

            if(pObjList)
            {
                if(pObjList->ISA(SdrPage))
                {
                    // Is a page
                    pRetval = &(((SdrPage*)pObjList)->GetViewContact());
                }
                else
                {
                    // Is a group?
                    if(pObjList->GetOwnerObj())
                    {
                        pRetval = &(pObjList->GetOwnerObj()->GetViewContact());
                    }
                }
            }

            return pRetval;
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

                        // #i38135# now remember new type
                        meRememberedAnimationKind = rTextObj.GetTextAniKind();
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
                        default: break;
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
