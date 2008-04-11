/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: objectcontactofpageview.cxx,v $
 * $Revision: 1.19 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"
#include <svx/sdr/contact/objectcontactofpageview.hxx>
#include <svx/sdr/contact/viewobjectcontactofunocontrol.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svdpage.hxx>
#include <svx/sdr/contact/displayinfo.hxx>
#include <svx/sdr/contact/viewobjectcontact.hxx>
#include <svx/svdview.hxx>
#include <svx/sdr/contact/viewcontact.hxx>
#include <svx/sdr/animation/objectanimator.hxx>
#include <svx/sdr/event/eventhandler.hxx>
#include <svx/sdrpagewindow.hxx>
#include <sdrpaintwindow.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
        // internal access to SdrPage of SdrPageView
        SdrPage* ObjectContactOfPageView::GetSdrPage() const
        {
            return GetPageWindow().GetPageView().GetPage();
        }

        ObjectContactOfPageView::ObjectContactOfPageView(SdrPageWindow& rPageWindow)
        :   ObjectContact(),
            mrPageWindow(rPageWindow),
            mpRememberedStartPage(0L)
        {
            mbIsPreviewRenderer = ((SdrPaintView&)rPageWindow.GetPageView().GetView()).IsPreviewRenderer();
        }

        // The destructor. When PrepareDelete() was not called before (see there)
        // warnings will be generated in debug version if there are still contacts
        // existing.
        ObjectContactOfPageView::~ObjectContactOfPageView()
        {
        }

        // A ViewObjectContact was deleted and shall be forgotten.
        // #i29181# Overload to clear selection at associated view
        void ObjectContactOfPageView::RemoveViewObjectContact(ViewObjectContact& rVOContact)
        {
            // call parent
            ObjectContact::RemoveViewObjectContact(rVOContact);
        }

        // Update Draw Hierarchy data. Take care of everything that is inside
        // of rDisplayInfo.GetRedrawArea(), that is the necessary part.
        void ObjectContactOfPageView::EnsureValidDrawHierarchy(DisplayInfo& /*rDisplayInfo*/)
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

        // Process the whole displaying
        void ObjectContactOfPageView::ProcessDisplay(DisplayInfo& rDisplayInfo)
        {
            if(!IsDrawHierarchyValid())
            {
                // The default implementation ensures a valid draw hierarchy.
                EnsureValidDrawHierarchy(rDisplayInfo);
            }

            const SdrPage* pStartPage = GetSdrPage();

            if(IsDrawHierarchyValid()
                && pStartPage
                && rDisplayInfo.GetPaintInfoRec()
                && rDisplayInfo.GetOutputDevice()
                && maDrawHierarchy.Count()
                && !rDisplayInfo.GetProcessLayers().IsEmpty())
            {
                // paint direct
                DoProcessDisplay(rDisplayInfo);
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
        }

        // Process the whole displaying. Only use given DsiplayInfo, do not access other
        // OutputDevices then the given ones.
        void ObjectContactOfPageView::DoProcessDisplay(DisplayInfo& rDisplayInfo)
        {
            // visualize entered group when that feature is switched on and it's not
            // a print output
            sal_Bool bVisualizeEnteredGroup(DoVisualizeEnteredGroup() && !rDisplayInfo.OutputToPrinter());

            // Visualize entered groups: Set to ghosted as default
            // start. Do this only for the DrawPage, not for MasterPages
            if(bVisualizeEnteredGroup)
            {
                rDisplayInfo.SetGhostedDrawMode();
            }

            // #114359# save old and set clip region
            OutputDevice& rOutDev = *rDisplayInfo.GetOutputDevice();
            sal_Bool bClipRegionPushed(sal_False);
            const Region& rRedrawArea(rDisplayInfo.GetRedrawArea());

            if(!rRedrawArea.IsEmpty())
            {
                bClipRegionPushed = sal_True;
                rOutDev.Push(PUSH_CLIPREGION);
                rOutDev.IntersectClipRegion(rRedrawArea);
            }

            // Draw DrawPage
            ViewObjectContact& rDrawPageVOContact = *(maDrawHierarchy.GetObject(0L));
            rDrawPageVOContact.PaintObjectHierarchy(rDisplayInfo);

            // #114359# restore old ClipReghion
            if(bClipRegionPushed)
            {
                rOutDev.Pop();
            }

            // Visualize entered groups: Reset to original DrawMode
            if(bVisualizeEnteredGroup)
            {
                rDisplayInfo.ClearGhostedDrawMode();
            }

            // Test if paint was interrupted. If Yes, we need to invalidate the
            // PageView where the paint region was defined
            if(!rDisplayInfo.DoContinuePaint())
            {
                Rectangle aRect = rDisplayInfo.GetRedrawArea().GetBoundRect();
                GetPageWindow().Invalidate(aRect);
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
            SdrView& rView = GetPageWindow().GetPageView().GetView();
            return rView.DoVisualizeEnteredGroup();
        }

        // Get the active group (the entered group). To get independent
        // from the old object/view classes return values use the new
        // classes.
        ViewContact* ObjectContactOfPageView::GetActiveGroupContact() const
        {
            SdrObjList* pActiveGroupList = GetPageWindow().GetPageView().GetObjList();
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
            GetPageWindow().Invalidate(rRectangle);
        }

        // #i42815#
        // Get info if given Rectangle is visible in this view
        sal_Bool ObjectContactOfPageView::IsAreaVisible(const Rectangle& rRectangle) const
        {
            OutputDevice& rOutDev = GetPageWindow().GetPaintWindow().GetOutputDevice();
            const Point aEmptyPoint;
            const Rectangle aVisiblePixel(aEmptyPoint, rOutDev.GetOutputSizePixel());
            const Rectangle aTestPixel(rOutDev.LogicToPixel(rRectangle));

            // compare with the visible rectangle
            if(!aTestPixel.IsOver(aVisiblePixel))
            {
                return sal_False;
            }

            // call parent
            return ObjectContact::IsAreaVisible(rRectangle);
        }

        // Get info about the need to visualize GluePoints
        sal_Bool ObjectContactOfPageView::AreGluePointsVisible() const
        {
            return GetPageWindow().GetPageView().GetView().ImpIsGlueVisible();
        }

        // check if text animation is allowed.
        sal_Bool ObjectContactOfPageView::IsTextAnimationAllowed() const
        {
            SdrView& rView = GetPageWindow().GetPageView().GetView();
            const SvtAccessibilityOptions& rOpt = rView.getAccessibilityOptions();
            return rOpt.GetIsAllowAnimatedText();
        }

        // check if graphic animation is allowed.
        sal_Bool ObjectContactOfPageView::IsGraphicAnimationAllowed() const
        {
            SdrView& rView = GetPageWindow().GetPageView().GetView();
            const SvtAccessibilityOptions& rOpt = rView.getAccessibilityOptions();
            return rOpt.GetIsAllowAnimatedGraphics();
        }

        // check if asynchronious graphis loading is allowed. Default is sal_False.
        sal_Bool ObjectContactOfPageView::IsAsynchronGraphicsLoadingAllowed() const
        {
            SdrView& rView = GetPageWindow().GetPageView().GetView();
            return rView.IsSwapAsynchron();
        }

        // check if buffering of MasterPages is allowed. Default is sal_False.
        sal_Bool ObjectContactOfPageView::IsMasterPageBufferingAllowed() const
        {
            SdrView& rView = GetPageWindow().GetPageView().GetView();
            return rView.IsMasterPagePaintCaching();
        }

        // set all UNO controls displayed in the view to design/alive mode
        void ObjectContactOfPageView::SetUNOControlsDesignMode( bool _bDesignMode ) const
        {
            sal_uInt32 nVOCCount = maVOCList.Count();
            for ( sal_uInt32 voc=0; voc<nVOCCount; ++voc )
            {
                const ViewObjectContact* pVOC = maVOCList.GetObject( voc );
                const ViewObjectContactOfUnoControl* pUnoObjectVOC = dynamic_cast< const ViewObjectContactOfUnoControl* >( pVOC );
                if ( !pUnoObjectVOC )
                    continue;
                pUnoObjectVOC->setControlDesignMode( _bDesignMode );
            }
        }
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////
// eof
