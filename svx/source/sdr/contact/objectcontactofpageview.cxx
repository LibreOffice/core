/*************************************************************************
 *
 *  $RCSfile: objectcontactofpageview.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2003-11-24 16:44:37 $
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

#ifndef _SDR_CONTACT_OBJECTCONTACTOFPAGEVIEW_HXX
#include <svx/sdr/contact/objectcontactofpageview.hxx>
#endif

#ifndef _SVDPAGV_HXX
#include <svdpagv.hxx>
#endif

#ifndef _SVDPAGE_HXX
#include <svdpage.hxx>
#endif

#ifndef _SDR_CONTACT_DISPLAYINFO_HXX
#include <svx/sdr/contact/displayinfo.hxx>
#endif

#ifndef _SVDMODEL_HXX
#include <svdmodel.hxx>
#endif

#ifndef _SDR_CONTACT_VIEWOBJECTCONTACT_HXX
#include <svx/sdr/contact/viewobjectcontact.hxx>
#endif

#ifndef _SVDVIEW_HXX
#include <svdview.hxx>
#endif

#ifndef _SDR_CONTACT_VIEWCONTACT_HXX
#include <svx/sdr/contact/viewcontact.hxx>
#endif

#ifndef _SDR_ANIMATION_OBJECTANIMATOR_HXX
#include <svx/sdr/animation/objectanimator.hxx>
#endif

#ifndef _XOUTX_HXX
#include <xoutx.hxx>
#endif

#ifndef _SDR_EVENT_EVENTHANDLER_HXX
#include <svx/sdr/event/eventhandler.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
        // internal access to SdrPageView
        SdrPageView& ObjectContactOfPageView::GetPageView() const
        {
            return mrPageView;
        }

        // internal access to SdrPage of SdrPageView
        SdrPage* ObjectContactOfPageView::GetSdrPage() const
        {
            return GetPageView().GetPage();
        }

        ObjectContactOfPageView::ObjectContactOfPageView(SdrPageView& rPageView)
        :   ObjectContact(),
            mrPageView(rPageView),
            mpRememberedStartPage(0L)
        {
        }

        // The destructor. When PrepareDelete() was not called before (see there)
        // warnings will be generated in debug version if there are still contacts
        // existing.
        ObjectContactOfPageView::~ObjectContactOfPageView()
        {
        }

        // Update Draw Hierarchy data. Take care of everything that is inside
        // of rDisplayInfo.GetRedrawArea(), that is the necessary part.
        void ObjectContactOfPageView::EnsureValidDrawHierarchy(DisplayInfo& rDisplayInfo)
        {
            // get StartNode
            SdrPage* pStartPage = GetSdrPage();

            // test for start point change
            if(pStartPage != mpRememberedStartPage)
            {
                // Clear whole DrawHierarchy, needs complete rebuild
                ClearDrawHierarchy();

                // remember new StartPoint
                mpRememberedStartPage = pStartPage;

                // build hierarchy for local draw page. This will include the
                // MasterPages as sub-hierarchy of the DrawPage because
                // ViewContactOfSdrPage::GetObjectCount() and ::GetViewContact()
                // support that.
                ViewContact& rViewContact = pStartPage->GetViewContact();

                // Get the ViewObjectContact for this ViewContact and this DrawContact. This creates such an object
                // on demand and adds it to the ViewObjectContactList of the ViewContact.
                ViewObjectContact& rViewObjectContact = rViewContact.GetViewObjectContact(*this);

                // set parent at ViewObjectContact. For pages, there is no parent.
                rViewObjectContact.SetParent(0L);

                // build sub-hierarchy
                if(rViewContact.GetObjectCount())
                {
                    rViewObjectContact.BuildDrawHierarchy(*this, rViewContact);
                }

                // Add to list
                maDrawHierarchy.Append(&rViewObjectContact);

                // set DrawHierarchy valid
                mbDrawHierarchyValid = sal_True;
            }
            else
            {
                // no new StartPoint, is the invalid flag set which means
                // somewhere the sub-hierarchy is invalid?
                if(!IsDrawHierarchyValid())
                {
                    // Yes, check the sub-hierarchies
                    const sal_uInt32 nCount(maDrawHierarchy.Count());

                    for(sal_uInt32 a(0L); a < nCount; a++)
                    {
                        ViewObjectContact& rVOContact = *(maDrawHierarchy.GetObject(a));

                        rVOContact.CheckDrawHierarchy(*this);
                    }

                    // set DrawHierarchy valid
                    mbDrawHierarchyValid = sal_True;
                }
            }
        }

        // Create and set the ExpandPaintClipRegion. This needs to be done before
        // any of the objects gets really painted because it relies on the invalidated
        // and not painted state of the single objects.
        void ObjectContactOfPageView::ExpandPaintClipRegion(DisplayInfo& rDisplayInfo)
        {
            if(rDisplayInfo.GetPaintInfoRec() && rDisplayInfo.GetOutputDevice())
            {
                OutputDevice* pOut = rDisplayInfo.GetOutputDevice();

                if(OUTDEV_WINDOW == pOut->GetOutDevType())
                {
                    Window* pWin = (Window*)pOut;
                    Region aExpandRegion;

                    // For this purpose test against visible area, not against
                    // paint rectangle. Thus, Remember DrawArea and set visible area instead.
                    Region aOriginalDrawArea = rDisplayInfo.GetRedrawArea();
                    Point aEmptyPoint;
                    Rectangle aVisiblePixel(aEmptyPoint, pOut->GetOutputSizePixel());
                    Region aVisibleLogic = Region(pOut->PixelToLogic(aVisiblePixel));
                    rDisplayInfo.SetRedrawArea(aVisibleLogic);

                    for(sal_uInt32 a(0L); a < maDrawHierarchy.Count(); a++)
                    {
                        ViewObjectContact& rVOContact = *(maDrawHierarchy.GetObject(a));

                        // Recursively Build Clip Region for ViewObjectContacts
                        rVOContact.BuildClipRegion(rDisplayInfo, aExpandRegion);
                    }

                    if(!(aExpandRegion.IsEmpty() || aExpandRegion.IsNull()))
                    {
                        // expand the ClipRegion
                        pWin->ExpandPaintClipRegion(aExpandRegion);

                        // also expand the DrawArea, more needs to be redrawn now.
                        aOriginalDrawArea.Union(aExpandRegion.GetBoundRect());
                    }

                    // restore original DrawArea
                    rDisplayInfo.SetRedrawArea(aOriginalDrawArea);
                }
            }
        }

        // The PreRenderer
        void ObjectContactOfPageView::PreRender(DisplayInfo& rDisplayInfo)
        {
            // get and remember OutputDevices
            OutputDevice* pOriginalOutDev = rDisplayInfo.GetOutputDevice();
            ExtOutputDevice* pOriginalExtOutDev = rDisplayInfo.GetExtendedOutputDevice();

            // compare size of maPreRenderDevice with size of visible area
            if(maPreRenderDevice.GetOutputSizePixel() != pOriginalOutDev->GetOutputSizePixel())
            {
                maPreRenderDevice.SetOutputSizePixel(pOriginalOutDev->GetOutputSizePixel());
            }

            // Also compare the MapModes for zoom/scroll changes
            if(maPreRenderDevice.GetMapMode() != pOriginalOutDev->GetMapMode())
            {
                maPreRenderDevice.SetMapMode(pOriginalOutDev->GetMapMode());
            }

            // replace values at rDisplayInfo for rendering to PreRenderDevice
            pOriginalExtOutDev->SetOutDev(&maPreRenderDevice);
            rDisplayInfo.SetOutputDevice(&maPreRenderDevice);

            // paint in PreRenderDevice
            DoProcessDisplay(rDisplayInfo);

            // set back to old OutDev, restore rDisplayInfo
            pOriginalExtOutDev->SetOutDev(pOriginalOutDev);
            rDisplayInfo.SetOutputDevice(pOriginalOutDev);
        }

        // Pre-Process the whole displaying.
        void ObjectContactOfPageView::PreProcessDisplay(DisplayInfo& rDisplayInfo)
        {
            // call parent to ensure a valid draw hierarchy.
            ObjectContact::PreProcessDisplay(rDisplayInfo);

            // also expand the paint clip region in the pre-paint phase
            if(IsDrawHierarchyValid())
            {
                ExpandPaintClipRegion(rDisplayInfo);
            }
        }

        // Process the whole displaying
        void ObjectContactOfPageView::ProcessDisplay(DisplayInfo& rDisplayInfo)
        {
            SdrPage* pStartPage = GetSdrPage();

            if(IsDrawHierarchyValid()
                && pStartPage
                && rDisplayInfo.GetPaintInfoRec()
                && rDisplayInfo.GetOutputDevice()
                && maDrawHierarchy.Count())
            {
                // First look if pre-rendering should be done. If Yes, use it. If no,
                // do a normal ProcessDisplay() using DoProcessDiaply(). The second one
                // is called in any case, maybe for pre-rendering or for direct output.
                if(DoPreRender(rDisplayInfo))
                {
                    // Get OutputDevice
                    OutputDevice* pOut = rDisplayInfo.GetOutputDevice();

                    // update pre-rendered VirtualDevice
                    PreRender(rDisplayInfo);

                    // calculate the to-be-refreshed rectangle
                    Rectangle aPaintRect(rDisplayInfo.GetRedrawArea().GetBoundRect());
                    Rectangle aPaintRectPixel = pOut->LogicToPixel(aPaintRect);

                    // paint using prepared, pre-rendered VirtualDevice
                    sal_Bool bMapModeWasEnabledDest(pOut->IsMapModeEnabled());
                    sal_Bool bMapModeWasEnabledSource(maPreRenderDevice.IsMapModeEnabled());
                    pOut->EnableMapMode(sal_False);
                    maPreRenderDevice.EnableMapMode(sal_False);

                    Size aPaintSizePixel = aPaintRectPixel.GetSize();
                    pOut->DrawOutDev(
                        aPaintRectPixel.TopLeft(), aPaintSizePixel,
                        aPaintRectPixel.TopLeft(), aPaintSizePixel,
                        maPreRenderDevice);

                    pOut->EnableMapMode(bMapModeWasEnabledDest);
                    maPreRenderDevice.EnableMapMode(bMapModeWasEnabledSource);
                }
                else
                {
                    // paint direct
                    DoProcessDisplay(rDisplayInfo);
                }
            }

            // after paint take care of the evtl. scheduled asynchronious commands.
            // Do this by resetting the timer contained there. Thus, after the paint
            // that timer will be triggered and the events will be executed.
            if(HasEventHandler())
            {
                sdr::event::TimerEventHandler& rEventHandler = GetEventHandler();

                if(!rEventHandler.IsEmpty())
                {
                    rEventHandler.Restart();
                }
            }

#ifdef DBG_UTIL
            // Do some test painting for the given rectangles
            if(sal_False && rDisplayInfo.GetPaintInfoRec() && rDisplayInfo.GetOutputDevice())
            {
                OutputDevice* pOut = rDisplayInfo.GetOutputDevice();
                pOut->SetFillColor();
                pOut->SetLineColor(COL_RED);
                pOut->DrawRect(rDisplayInfo.GetRedrawArea().GetBoundRect());

                sal_Bool bWasEnabled(pOut->IsMapModeEnabled());
                pOut->EnableMapMode(sal_False);

                // Test ExpandPaintClipRegion
                Point aEmptyPoint;
                Rectangle aVisiblePixel(aEmptyPoint, pOut->GetOutputSizePixel());
                pOut->Push( PUSH_CLIPREGION );

                if(OUTDEV_WINDOW == pOut->GetOutDevType())
                {
                    Window* pWin = (Window*)pOut;
                    Region aExpandRegion(pOut->PixelToLogic(aVisiblePixel));
                    pWin->ExpandPaintClipRegion(aExpandRegion);
                }

                sal_uInt32 nTime(Time::GetSystemTicks());
                pOut->SetLineColor(Color(sal_uInt8(nTime), sal_uInt8(nTime >> 3), sal_uInt8(nTime >> 6)));
                pOut->DrawRect(aVisiblePixel);

                pOut->Pop();
                pOut->EnableMapMode(bWasEnabled);
            }
#endif DBG_UTIL
        }

        // Decide if to PreRender
        sal_Bool ObjectContactOfPageView::DoPreRender(DisplayInfo& rDisplayInfo) const
        {
            return (rDisplayInfo.IsPreRenderingAllowed()
                && !rDisplayInfo.OutputToPrinter()
                && !rDisplayInfo.OutputToVirtualDevice()
                && !rDisplayInfo.OutputToRecordingMetaFile());
        }

        // Process the whole displaying
        void ObjectContactOfPageView::DoProcessDisplay(DisplayInfo& rDisplayInfo)
        {
            // test if ControlLayer is to be painted
            SdrPage* pStartPage = GetSdrPage();
            const SdrModel& rModel = *(pStartPage->GetModel());
            const SdrLayerAdmin& rLayerAdmin = rModel.GetLayerAdmin();
            const sal_uInt32 nControlLayerId = rLayerAdmin.GetLayerID(rLayerAdmin.GetControlLayerName(), sal_False);
            SetOfByte aProcessLayers = rDisplayInfo.GetProcessLayers();
            sal_Bool bDrawControlLayer(aProcessLayers.IsSet((sal_uInt8)nControlLayerId));
            ViewObjectContact& rDrawPageVOContact = *(maDrawHierarchy.GetObject(0L));

            // visualize entered group when that feature is switched on and it's not
            // a print output
            sal_Bool bVisualizeEnteredGroup(DoVisualizeEnteredGroup() && !rDisplayInfo.OutputToPrinter());

            // Always save original DRawMode, it's used by the renderers
            // when they need to paint into a bitmap.
            rDisplayInfo.SaveOriginalDrawMode();

            // Visualize entered groups: Set to ghosted as default
            // start. Do this only for the DrawPage, not for MasterPages
            if(bVisualizeEnteredGroup)
            {
                rDisplayInfo.SetGhostedDrawMode();
            }

            // Clear ControlLayer
            if(bDrawControlLayer)
            {
                // clear for normal paint
                aProcessLayers.Clear((sal_uInt8)nControlLayerId);
            }

            // Draw DrawPage without controls
            if(!aProcessLayers.IsEmpty())
            {
                // standard paint
                rDisplayInfo.SetProcessLayers(aProcessLayers);
                rDisplayInfo.SetTemporaryPaintPage(sal_True);
                rDrawPageVOContact.PaintObjectHierarchy(rDisplayInfo);
            }

            // Draw DrawPage, Controls only
            if(bDrawControlLayer)
            {
                // paint ControlLayer
                aProcessLayers.ClearAll();
                aProcessLayers.Set((sal_uInt8)nControlLayerId);
                rDisplayInfo.SetProcessLayers(aProcessLayers);
                rDisplayInfo.SetTemporaryPaintPage(sal_False);
                rDrawPageVOContact.PaintObjectHierarchy(rDisplayInfo);
            }

            // Visualize entered groups: Reset to original DrawMode
            if(bVisualizeEnteredGroup)
            {
                rDisplayInfo.RestoreOriginalDrawMode();
            }

            // Test if paint was interrupted. If Yes, we need to invalidate the
            // PageView where the paint region was defined
            if(!rDisplayInfo.DoContinuePaint())
            {
                Rectangle aRect = rDisplayInfo.GetRedrawArea().GetBoundRect();
                GetPageView().InvalidateAllWin(aRect);
            }

            // If a ObjectAnimator exists, execute it. This will only do anything
            // if the ObjectAnimator's timer is not yet running and if there are
            // events scheduled.
            if(HasObjectAnimator())
            {
                GetObjectAnimator().Execute();
            }
        }

        // test if visualizing of entered groups is switched on at all
        sal_Bool ObjectContactOfPageView::DoVisualizeEnteredGroup() const
        {
            SdrView& rView = GetPageView().GetView();
            return rView.DoVisualizeEnteredGroup();
        }

        // Get the active group (the entered group). To get independent
        // from the old object/view classes return values use the new
        // classes.
        ViewContact* ObjectContactOfPageView::GetActiveGroupContact() const
        {
            SdrObjList* pActiveGroupList = GetPageView().GetObjList();
            ViewContact* pRetval = 0L;

            if(pActiveGroupList)
            {
                if(pActiveGroupList->ISA(SdrPage))
                {
                    // It's a Page itself
                    pRetval = &(((SdrPage*)pActiveGroupList)->GetViewContact());
                }
                else if(pActiveGroupList->GetOwnerObj())
                {
                    // Group object
                    return &(pActiveGroupList->GetOwnerObj()->GetViewContact());
                }
            }

            if(!pRetval)
            {
                if(GetSdrPage())
                {
                    // use page of associated SdrPageView
                    pRetval = &(GetSdrPage()->GetViewContact());
                }
            }

            return pRetval;
        }

        // Invalidate given rectangle at the window/output which is represented by
        // this ObjectContact.
        void ObjectContactOfPageView::InvalidatePartOfView(const Rectangle& rRectangle) const
        {
            // invalidate all associated windows.
            GetPageView().InvalidateAllWin(rRectangle);
        }

        // Get info about the need to visualize GluePoints
        sal_Bool ObjectContactOfPageView::AreGluePointsVisible() const
        {
            return GetPageView().GetView().ImpIsGlueVisible();
        }

        // check if text animation is allowed.
        sal_Bool ObjectContactOfPageView::IsTextAnimationAllowed() const
        {
            SdrView& rView = GetPageView().GetView();
            const SvtAccessibilityOptions& rOpt = rView.getAccessibilityOptions();
            return rOpt.GetIsAllowAnimatedText();
        }

        // check if graphic animation is allowed.
        sal_Bool ObjectContactOfPageView::IsGraphicAnimationAllowed() const
        {
            SdrView& rView = GetPageView().GetView();
            const SvtAccessibilityOptions& rOpt = rView.getAccessibilityOptions();
            return rOpt.GetIsAllowAnimatedGraphics();
        }

        // check if asynchronious graphis loading is allowed. Default is sal_False.
        sal_Bool ObjectContactOfPageView::IsAsynchronGraphicsLoadingAllowed() const
        {
            SdrView& rView = GetPageView().GetView();
            return rView.IsSwapAsynchron();
        }
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////
// eof
