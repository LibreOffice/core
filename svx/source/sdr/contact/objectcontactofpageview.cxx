/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: objectcontactofpageview.cxx,v $
 *
 *  $Revision: 1.16 $
 *
 *  last change: $Author: ihi $ $Date: 2006-11-14 13:30:51 $
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

#ifndef _SDR_CONTACT_OBJECTCONTACTOFPAGEVIEW_HXX
#include <svx/sdr/contact/objectcontactofpageview.hxx>
#endif
#ifndef SVX_SDR_CONTACT_VIEWOBJECTCONTACTOFUNOCONTROL_HXX
#include <svx/sdr/contact/viewobjectcontactofunocontrol.hxx>
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

#ifndef _SDR_EVENT_EVENTHANDLER_HXX
#include <svx/sdr/event/eventhandler.hxx>
#endif

#ifndef _SDRPAGEWINDOW_HXX
#include <sdrpagewindow.hxx>
#endif

#ifndef _SDRPAINTWINDOW_HXX
#include <sdrpaintwindow.hxx>
#endif

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

        // Create and set the ExpandPaintClipRegion. This needs to be done before
        // any of the objects gets really painted because it relies on the invalidated
        // and not painted state of the single objects.
        void ObjectContactOfPageView::ExpandPaintClipRegion(DisplayInfo& rDisplayInfo)
        {
            if(rDisplayInfo.GetPaintInfoRec() && rDisplayInfo.GetOutputDevice())
            {
                OutputDevice* pOut = rDisplayInfo.GetOutputDevice();

                if(pOut && OUTDEV_WINDOW == pOut->GetOutDevType())
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

                    // #i42431#
                    if(false)
                    {
                        // what should be here:
                        if(!(aExpandRegion.IsEmpty() || aExpandRegion.IsNull()))
                        {
                            pWin->ExpandPaintClipRegion(aExpandRegion);
                            aOriginalDrawArea.Union(aExpandRegion);
                        }

                        // #i42431#
                        // copied comments from older fixes for documentation:

                            // #116639#
                            //      aOriginalDrawArea.Union(aExpandRegion.GetBoundRect());
                            // Because of a bug in Region which THB wants to take a look at,
                            // the result of the previous command is WRONG when done in logical
                            // coodinates, the Region gets a line to small (!). Thus, for
                            // a workaround i have to do the union in pixel coordinates (!).

                            //const Region aExpandRegionPixel(pOut->LogicToPixel(aExpandRegion));
                            //Rectangle aExpandPixelRect(aExpandRegionPixel.GetBoundRect());

                            // #116639#, #i29132#
                            // Since e.g. with connectors like in #i29132# still rounding
                            // errors happen when converting to pixels, i choose now to handle
                            // this here in expanding the expand rectangle by one pixel, that
                            // means: device dependent.
                            // The maximum rounding error is potentially 1/2 pixel, so with
                            // one pixel the rect is then potentially up to 1/2 pixel too big.
                            // I see no other way until we have more precise conversions, though.
                            // All involved logic coordinates are correct.

                        // #i42431#
                        // Real problem is: When regions/rectangles get converted from pixel to logic,
                        // it should be a scale which expands single pixel lines to rectanges. That
                        // does not happen. Example: In pixel, a region contains one horizontal region
                        // which describes one pixel line. That region has no height. When scaled You
                        // should get a logical region with the logic rectangle describing the line,
                        // but Yot get just a logical line with no height.
                        // So all pixel sources need to be expanded by on elogical pixel size in width
                        // and height, that's now done below. Since the OriginalDrawArea was created
                        // normally (in all cases?) by converting a rectangle from pixel to logic, it
                        // is always corrected. In the cases it was not derived from pixels, it's safer
                        // to do it. Same argument at the ExpandRegion.
                    }
                    else
                    {
                        const Size aPixelSizeInLogic(pOut->PixelToLogic(Size(1L, 1L)));

                        // correct aOriginalDrawArea
                        {
                            Region aCorrectedDrawArea;
                            Rectangle aCorrectRectangle;
                            RegionHandle aRegionHandle = aOriginalDrawArea.BeginEnumRects();

                            while(aOriginalDrawArea.GetEnumRects(aRegionHandle, aCorrectRectangle))
                            {
                                aCorrectRectangle.Bottom() += aPixelSizeInLogic.Height();
                                aCorrectRectangle.Right() += aPixelSizeInLogic.Height();
                                aCorrectedDrawArea.Union(aCorrectRectangle);
                            }

                            aOriginalDrawArea.EndEnumRects(aRegionHandle);
                            aOriginalDrawArea = aCorrectedDrawArea;
                        }

                        if(!(aExpandRegion.IsEmpty() || aExpandRegion.IsNull()))
                        {
                            // correct expand region
                            Region aCorrectedRegion;
                            Rectangle aCorrectRectangle;
                            RegionHandle aRegionHandle = aExpandRegion.BeginEnumRects();

                            while(aExpandRegion.GetEnumRects(aRegionHandle, aCorrectRectangle))
                            {
                                aCorrectRectangle.Bottom() += aPixelSizeInLogic.Height();
                                aCorrectRectangle.Right() += aPixelSizeInLogic.Height();
                                aCorrectedRegion.Union(aCorrectRectangle);
                            }

                            aExpandRegion.EndEnumRects(aRegionHandle);
                            aExpandRegion = aCorrectedRegion;

                            // use corrected region
                            pWin->ExpandPaintClipRegion(aExpandRegion);
                            aOriginalDrawArea.Union(aExpandRegion);
                        }

                        // Clip the draw area to the visible are of the
                        // output device.
                        aOriginalDrawArea.Intersect(aVisibleLogic);
                    }

                    // restore original DrawArea
                    rDisplayInfo.SetRedrawArea(aOriginalDrawArea);
                }
            }
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

        // #i37394# Non-painted object was changed. Test for potentially
        // getting visible
        void ObjectContactOfPageView::ObjectGettingPotentiallyVisible(const ViewObjectContact& rVOC) const
        {
            // #i42815# Use new test method
            const Rectangle& rOrigObjectRectangle = rVOC.GetViewContact().GetPaintRectangle();

            if(IsAreaVisible(rOrigObjectRectangle))
            {
                // invalidate
                GetPageWindow().Invalidate(rOrigObjectRectangle);
            }

            // call parent
            ObjectContact::ObjectGettingPotentiallyVisible(rVOC);
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
