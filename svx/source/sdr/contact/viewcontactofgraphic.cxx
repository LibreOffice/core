/*************************************************************************
 *
 *  $RCSfile: viewcontactofgraphic.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-12 14:41:00 $
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

#ifndef _SDR_CONTACT_VIEWCONTACTOFGRAPHIC_HXX
#include <svx/sdr/contact/viewcontactofgraphic.hxx>
#endif

#ifndef _SVDOGRAF_HXX
#include <svdograf.hxx>
#endif

#ifndef _SDR_ANIMATION_AINFOGRAPHIC_HXX
#include <svx/sdr/animation/ainfographic.hxx>
#endif

#ifndef _SDR_CONTACT_DISPLAYINFO_HXX
#include <svx/sdr/contact/displayinfo.hxx>
#endif

#ifndef _SDR_CONTACT_VIEWOBJECTCONTACT_HXX
#include <svx/sdr/contact/viewobjectcontact.hxx>
#endif

#ifndef _SDR_CONTACT_OBJECTCONTACT_HXX
#include <svx/sdr/contact/objectcontact.hxx>
#endif

#ifndef _SDR_EVENT_EVENTHANDLER_HXX
#include <svx/sdr/event/eventhandler.hxx>
#endif

#include "svdstr.hrc"

#ifndef _SVDGLOB_HXX
#include <svdglob.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace event
    {
        class AsynchGraphicLoadingEvent : public BaseEvent
        {
            // the ViewContactOfGraphic to work with
            sdr::contact::ViewContactOfGraphic&     mrVCOGraphic;

        public:
            // basic constructor.
            AsynchGraphicLoadingEvent(
                EventHandler& rEventHandler, sdr::contact::ViewContactOfGraphic& rVCOGraphic);

            // destructor
            virtual ~AsynchGraphicLoadingEvent();

            // the called method if the event is triggered
            virtual void ExecuteEvent();
        };

        AsynchGraphicLoadingEvent::AsynchGraphicLoadingEvent(
            EventHandler& rEventHandler, sdr::contact::ViewContactOfGraphic& rVCOGraphic)
        :   BaseEvent(rEventHandler),
            mrVCOGraphic(rVCOGraphic)
        {
        }

        AsynchGraphicLoadingEvent::~AsynchGraphicLoadingEvent()
        {
            mrVCOGraphic.ForgetAsynchGraphicLoadingEvent(this);
        }

        void AsynchGraphicLoadingEvent::ExecuteEvent()
        {
            mrVCOGraphic.DoAsynchGraphicLoading();
        }
    } // end of namespace event
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
        // Test graphics state and eventually trigger a SwapIn event or an Asynchronous
        // load event. Return value gives info if SwapIn was triggered.
        sal_Bool ViewContactOfGraphic::ImpPrepareForPaint(
            DisplayInfo& rDisplayInfo, const ViewObjectContact& rAssociatedVOC)
        {
            sal_Bool bRetval(sal_False);

            if(!rDisplayInfo.IsDraftGraphic())
            {
                SdrGrafObj& rGrafObj = GetGrafObject();

                // 2nd part should not be necessary when all graphics creationg instances set
                // the SwapState correctly. It was there for historical reasons since before
                // the SwapState could be set it was simply 'tried' to SwapIn a GRAPHIC_NONE
                if(rGrafObj.IsSwappedOut() /*|| (GRAPHIC_NONE == rGrafObj.GetGraphicType())*/)
                {
                    if(rGrafObj.IsLinkedGraphic())
                    {
                        // update graphic link
                        rGrafObj.ImpUpdateGraphicLink();
                    }
                    else
                    {
                        // SwapIn needs to be done. Decide if it can be done asynchronious.
                        sal_Bool bSwapInAsynchronious(sal_False);
                        ObjectContact& rObjectContact = rAssociatedVOC.GetObjectContact();

                        // only when allowed from configuration
                        if(rObjectContact.IsAsynchronGraphicsLoadingAllowed())
                        {
                            // direct output or vdev output (PageView buffering)
                            if(rDisplayInfo.OutputToWindow() || rDisplayInfo.OutputToVirtualDevice())
                            {
                                // only when no metafile recording
                                if(!rDisplayInfo.OutputToRecordingMetaFile())
                                {
                                    // allow asynchronious loading
                                    bSwapInAsynchronious = sal_True;
                                }
                            }
                        }

                        if(bSwapInAsynchronious)
                        {
                            // maybe it's on the way, then do nothing
                            if(!mpAsynchLoadEvent)
                            {
                                // Trigger asynchronious SwapIn.
                                sdr::event::TimerEventHandler& rEventHandler = rObjectContact.GetEventHandler();

                                mpAsynchLoadEvent = new sdr::event::AsynchGraphicLoadingEvent(
                                    rEventHandler, *this);
                            }
                        }
                        else
                        {
                            // SwapIn direct
                            rGrafObj.ForceSwapIn();
                            bRetval = sal_True;
                        }
                    }
                }
                else
                {
                    // it is not swapped out, somehow it was loaded. In that case, forget
                    // about an existing triggered event
                    if(mpAsynchLoadEvent)
                    {
                        // just delete it, this will remove it from the EventHandler and
                        // will trigger ForgetAsynchGraphicLoadingEvent from the destructor
                        delete mpAsynchLoadEvent;
                    }
                }
            }

            return bRetval;
        }

        // method to create a AnimationInfo. Needs to give a result if
        // SupportsAnimation() is overloaded and returns sal_True.
        sdr::animation::AnimationInfo* ViewContactOfGraphic::CreateAnimationInfo()
        {
            sdr::animation::AnimationInfo* pRetval = 0L;
            SdrGrafObj& rGrafObj = GetGrafObject();

            if(GRAPHIC_BITMAP == rGrafObj.GetGraphicType() && rGrafObj.IsAnimated())
            {
                pRetval = new sdr::animation::AInfoGraphic(GetGrafObject());
            }

            if(!pRetval)
            {
                // something went wrong
                DBG_ERROR("ViewContactOfGraphic::CreateAnimationInfo(): no anim, but SupportsAnimation (!)");
                pRetval = new sdr::animation::AInfoDummy();
            }

            return pRetval;
        }

        // For draft object display
        sal_Bool ViewContactOfGraphic::PaintGraphicDraft(
            DisplayInfo& rDisplayInfo, Rectangle& rPaintRectangle, const ViewObjectContact& rAssociatedVOC)
        {
            sal_Bool bRetval(sal_False);
            SdrGrafObj& rGrafObj = GetGrafObject();

            // paint a shadowed frame in object size, not filled
            Rectangle aUnrotatedRectangle;
            rGrafObj.TakeUnrotatedSnapRect(aUnrotatedRectangle);
            const GeoStat& rGeometric = rGrafObj.GetGeoStat();
            bRetval = PaintShadowedFrame(rDisplayInfo, rPaintRectangle, aUnrotatedRectangle, rGeometric, sal_False);

            // draw a draft bitmap
            Bitmap aDraftBitmap(ResId(BMAP_GrafikEi, ImpGetResMgr()));
            Rectangle aBitmapOutRect;
            bRetval |= PaintDraftBitmap(rDisplayInfo, aBitmapOutRect, aUnrotatedRectangle, rGeometric, aDraftBitmap);
            rPaintRectangle.Union(aBitmapOutRect);

            // Build the text for the draft object
            XubString aDraftText = rGrafObj.GetFileName();

            if(!aDraftText.Len())
            {
                aDraftText = rGrafObj.GetName();

                // if loading add a hint for that
                if(mpAsynchLoadEvent)
                {
                    aDraftText.AppendAscii(" ...");
                }
            }

            if(aDraftText.Len())
            {
                OutputDevice* pOutDev = rDisplayInfo.GetOutputDevice();
                const Size aBitmapSize(pOutDev->PixelToLogic(aDraftBitmap.GetSizePixel()));

                if(aUnrotatedRectangle.GetWidth() > aBitmapSize.Width())
                {
                    // calc a rectangle right from graphic
                    Rectangle aRightRectangle(aUnrotatedRectangle);
                    aRightRectangle.Left() += aBitmapSize.Width();

                    // Paint DraftText
                    Rectangle aTextOutRect;
                    bRetval |= PaintDraftText(rDisplayInfo, aTextOutRect, aRightRectangle, rGeometric, aDraftText, rGrafObj.IsLinkedGraphic());
                    rPaintRectangle.Union(aTextOutRect);
                }
            }

            // If there is text in the object below, render it, too.
            if(rGrafObj.HasText())
            {
                // force only text paint at SdrObject
                rGrafObj.SdrTextObj::DoPaintObject(
                    *rDisplayInfo.GetExtendedOutputDevice(),
                    *rDisplayInfo.GetPaintInfoRec());
                bRetval = sal_True;
                rPaintRectangle.Union(rGrafObj.SdrTextObj::GetCurrentBoundRect());
            }

            return bRetval;
        }

        // For empty Graphic PresObj display
        sal_Bool ViewContactOfGraphic::PaintGraphicPresObj(
            DisplayInfo& rDisplayInfo, Rectangle& rPaintRectangle, const ViewObjectContact& rAssociatedVOC)
        {
            sal_Bool bRetval(sal_False);
            OutputDevice* pOutDev = rDisplayInfo.GetOutputDevice();
            MapMode aDstMapMode(pOutDev->GetMapMode().GetMapUnit());
            rPaintRectangle = GetPaintRectangle();
            Point aPos(rPaintRectangle.Center());
            Size aSize;

            // get size of the graphic in output device units
            SdrGrafObj& rGrafObj = GetGrafObject();
            const MapMode& rGrafMapMode = rGrafObj.GetGrafPrefMapMode();
            const Size& rGrafPrefSize = rGrafObj.GetGrafPrefSize();

            if(MAP_PIXEL == rGrafMapMode.GetMapUnit())
            {
                aSize = pOutDev->PixelToLogic(rGrafPrefSize, aDstMapMode);
            }
            else
            {
                aSize = pOutDev->LogicToLogic(rGrafPrefSize, rGrafMapMode, aDstMapMode);
            }

            // calculate upper left position for painting the graphic
            aPos.X() -= (aSize.Width() >> 1);
            aPos.Y() -= (aSize.Height() >> 1);

            if(aPos.X() >= rPaintRectangle.Left() && aPos.Y() >= rPaintRectangle.Top())
            {
                // centered graphic fits inside object
                if(GRAPHIC_BITMAP == rGrafObj.GetGraphicType())
                {
                    rGrafObj.DrawGraphic(pOutDev, aPos, aSize);
                }
                else
                {
                    const sal_uInt32 nOldDrawMode(pOutDev->GetDrawMode());

                    if(0L != (nOldDrawMode & DRAWMODE_GRAYBITMAP ))
                    {
                        // If GRAYBITMAP is used, render MetaFiles with gray, too
                        sal_uInt32 nNewDrawMode(nOldDrawMode);

                        nNewDrawMode &= ~(DRAWMODE_BLACKLINE | DRAWMODE_BLACKFILL | DRAWMODE_WHITEFILL | DRAWMODE_NOFILL);
                        nNewDrawMode |= DRAWMODE_GRAYLINE | DRAWMODE_GRAYFILL;

                        pOutDev->SetDrawMode(nNewDrawMode);
                    }

                    // paint graphic
                    rGrafObj.DrawGraphic(pOutDev, aPos, aSize);

                    // restore DrawMode
                    pOutDev->SetDrawMode(nOldDrawMode);
                }

                // set parameters
                bRetval = sal_True;
            }
            else
            {
                // paint a shadowed frame in object size, filled
                Rectangle aUnrotatedRectangle;
                rGrafObj.TakeUnrotatedSnapRect(aUnrotatedRectangle);
                const GeoStat& rGeometric = rGrafObj.GetGeoStat();
                bRetval = PaintShadowedFrame(rDisplayInfo, rPaintRectangle, aUnrotatedRectangle, rGeometric, sal_True);
            }

            // If there is text in the object below, render it, too.
            if(rGrafObj.HasText())
            {
                // force only text paint at SdrObject
                rGrafObj.SdrTextObj::DoPaintObject(
                    *rDisplayInfo.GetExtendedOutputDevice(),
                    *rDisplayInfo.GetPaintInfoRec());
                bRetval = sal_True;
                rPaintRectangle.Union(rGrafObj.SdrTextObj::GetCurrentBoundRect());
            }

            return bRetval;
        }

        // Decide if graphic should be painted as draft
        sal_Bool ViewContactOfGraphic::DoPaintGraphicDraft(DisplayInfo& rDisplayInfo) const
        {
            // #115931#
            // Take care for SDRPAINTMODE_DRAFTGRAF
            if(rDisplayInfo.IsDraftGraphic())
            {
                return sal_True;
            }

            SdrGrafObj& rGrafObj = GetGrafObject();

            if(rGrafObj.IsSwappedOut())
            {
                return sal_True;
            }

            if(GRAPHIC_DEFAULT == rGrafObj.GetGraphicType() || GRAPHIC_NONE == rGrafObj.GetGraphicType())
            {
                return sal_True;
            }

            if(rGrafObj.IsEmptyPresObj())
            {
                return sal_True;
            }

            return sal_False;
        }

        void ViewContactOfGraphic::StopGettingViewed()
        {
            // call parent
            ViewContactOfTextObj::StopGettingViewed();

            // If there is an event triggered and no one is visualizing this object,
            // get rid of the event
            if(mpAsynchLoadEvent)
            {
                // just delete it, this will remove it from the EventHandler and
                // will trigger ForgetAsynchGraphicLoadingEvent from the destructor
                delete mpAsynchLoadEvent;
            }
        }

        ViewContactOfGraphic::ViewContactOfGraphic(SdrGrafObj& rGrafObj)
        :   ViewContactOfTextObj(rGrafObj),
            mpAsynchLoadEvent(0L)
        {
        }

        ViewContactOfGraphic::~ViewContactOfGraphic()
        {
            DBG_ASSERT(0L == mpAsynchLoadEvent,
                "ViewContactOfGraphic destructor: mpAsynchLoadEvent is not empty, call PrepareDelete() before deleting (!)");
        }

        // Prepare deletion of this object. Tghis needs to be called always
        // before really deleting this objects. This is necessary since in a c++
        // destructor no virtual function calls are allowed. To avoid this problem,
        // it is required to first call PrepareDelete().
        void ViewContactOfGraphic::PrepareDelete()
        {
            // call parent
            ViewContactOfTextObj::PrepareDelete();

            // evtl. delete the asynch loading event
            if(mpAsynchLoadEvent)
            {
                // just delete it, this will remove it from the EventHandler and
                // will trigger ForgetAsynchGraphicLoadingEvent from the destructor
                delete mpAsynchLoadEvent;
            }
        }

        // When ShouldPaintObject() returns sal_True, the object itself is painted and
        // PaintObject() is called.
        sal_Bool ViewContactOfGraphic::ShouldPaintObject(
            DisplayInfo& rDisplayInfo, const ViewObjectContact& rAssociatedVOC)
        {
            // Test if its a empty presobj, those should not be printed
            if(GetGrafObject().IsEmptyPresObj() && rDisplayInfo.OutputToPrinter())
            {
                return sal_False;
            }

            // return parent
            return ViewContactOfTextObj::ShouldPaintObject(rDisplayInfo, rAssociatedVOC);
        }

        sal_Bool ViewContactOfGraphic::PaintObject(
            DisplayInfo& rDisplayInfo, Rectangle& rPaintRectangle,
            const ViewObjectContact& rAssociatedVOC)
        {
            sal_Bool bRetval(sal_False);

            // Test graphics state and eventually trigger a SwapIn event or an Asynchronous
            // load event. Return value gives info if SwapIn was triggered.
            sal_Bool bSwapInDone = ImpPrepareForPaint(rDisplayInfo, rAssociatedVOC);
            sal_Bool bSwapInExclusiveForPrinting = (bSwapInDone && rDisplayInfo.OutputToPrinter());

            if(DoPaintGraphicDraft(rDisplayInfo))
            {
                if(GetGrafObject().IsEmptyPresObj())
                {
                    // Draw empty Graphic PresObj
                    bRetval = PaintGraphicPresObj(rDisplayInfo, rPaintRectangle, rAssociatedVOC);
                }
                else
                {
                    // Draw as draft
                    bRetval = PaintGraphicDraft(rDisplayInfo, rPaintRectangle, rAssociatedVOC);
                }
            }
            else
            {
                // call parent
                bRetval = ViewContactOfTextObj::PaintObject(rDisplayInfo, rPaintRectangle, rAssociatedVOC);
            }

            // If graphic was only swapped in for printing, swap  out again
            if(bSwapInExclusiveForPrinting)
            {
                GetGrafObject().ForceSwapOut();
            }
            else
            {
                // if was swapped in, animation may be possible now.
                if(bSwapInDone)
                {
                    // check existing animation. This may create or delete an AnimationInfo.
                    CheckAnimationFeatures();

                    if(maVOCList.Count() > 1L)
                    {
                        // more VOCs then only rAssociatedVOC use this object. Tell them about the
                        // change. Since we want no extra-redraw of rAssociatedVOC, i do not call
                        // ActionCanged() here, but invalidate the single VOCs except the current one.
                        for(sal_uInt32 a(0L); a < maVOCList.Count(); a++)
                        {
                            ViewObjectContact* pCandidate = maVOCList.GetObject(a);
                            DBG_ASSERT(pCandidate, "ViewContact::GetViewObjectContact() invalid ViewObjectContactList (!)");

                            if(pCandidate != &rAssociatedVOC)
                            {
                                pCandidate->ActionChanged();
                            }
                        }
                    }
                }
            }

            return bRetval;
        }

        // React on removal of the object of this ViewContact,
        // DrawHierarchy needs to be changed
        void ViewContactOfGraphic::ActionRemoved()
        {
            // call parent
            ViewContactOfTextObj::ActionRemoved();

            // If an event is triggered, get rid of it since object is no longer
            // visible
            if(mpAsynchLoadEvent)
            {
                // just delete it, this will remove it from the EventHandler and
                // will trigger ForgetAsynchGraphicLoadingEvent from the destructor
                delete mpAsynchLoadEvent;
            }
        }

        sal_Bool ViewContactOfGraphic::SupportsAnimation() const
        {
            // #116168# If object is in destruction, force animation support to sal_False
            if(GetGrafObject().IsInDestruction())
            {
                return sal_False;
            }

            // Is animation allowed?
            if(!GetGrafObject().IsGrafAnimationAllowed())
            {
                return sal_False;
            }

            // Is the graphica animated at all?
            if(!GetGrafObject().IsAnimated())
            {
                return sal_False;
            }

            // Is bitmap type?
            if(GRAPHIC_BITMAP != GetGrafObject().GetGraphicType())
            {
                return sal_False;
            }

            // It's a bitmap and it's animated and animation is allowed.
            return sal_True;
        }

        // This is the call from the asynch graphic loading. This may only be called from
        // AsynchGraphicLoadingEvent::ExecuteEvent(). Do load the graphics. The event will
        // be deleted (consumed) and ForgetAsynchGraphicLoadingEvent will be called.
        void ViewContactOfGraphic::DoAsynchGraphicLoading()
        {
            DBG_ASSERT(mpAsynchLoadEvent,
                "ViewContactOfGraphic::DoAsynchGraphicLoading: I did not trigger a event, why am i called (?)");

            // swap it in
            SdrGrafObj& rGrafObj = GetGrafObject();
            rGrafObj.ForceSwapIn();

            // Invalidate paint area and check existing animation (which may have changed).
            ActionChanged();
        }

        // This is the call from the destructor of the asynch graphic loading event.
        // No one else has to call this. It is needed to let this object forget about
        // the event. The parameter allows checking for the correct event.
        void ViewContactOfGraphic::ForgetAsynchGraphicLoadingEvent(sdr::event::AsynchGraphicLoadingEvent* pEvent)
        {
            DBG_ASSERT(mpAsynchLoadEvent,
                "ViewContactOfGraphic::ForgetAsynchGraphicLoadingEvent: I did not trigger a event, why am i called (?)");
            DBG_ASSERT(mpAsynchLoadEvent == pEvent,
                "ViewContactOfGraphic::ForgetAsynchGraphicLoadingEvent: Forced to forget another event then i have scheduled (?)");

            // forget event
            mpAsynchLoadEvent = 0L;
        }
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////
// eof
