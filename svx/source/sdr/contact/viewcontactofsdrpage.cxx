/*************************************************************************
 *
 *  $RCSfile: viewcontactofsdrpage.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 13:32:49 $
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

#ifndef _SDR_CONTACT_VIEWCONTACTOFSDRPAGE_HXX
#include <svx/sdr/contact/viewcontactofsdrpage.hxx>
#endif

#ifndef _SDR_CONTACT_VIEWOBJECTCONTACT_HXX
#include <svx/sdr/contact/viewobjectcontact.hxx>
#endif

#ifndef _SVDPAGE_HXX
#include <svdpage.hxx>
#endif

#ifndef _SDR_CONTACT_DISPLAYINFO_HXX
#include <svx/sdr/contact/displayinfo.hxx>
#endif

#ifndef _SVDPAGV_HXX
#include <svdpagv.hxx>
#endif

#ifndef _SVDVIEW_HXX
#include <svdview.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////

#define PAPER_SHADOW(SIZE) (SIZE >> 8)

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
        // method to create a AnimationInfo. Needs to give a result if
        // SupportsAnimation() is overloaded and returns sal_True.
        sdr::animation::AnimationInfo* ViewContactOfSdrPage::CreateAnimationInfo()
        {
            // This call can only be an error ATM.
            DBG_ERROR("ViewContactOfSdrPage::CreateAnimationInfo(): Page does not support animation (!)");
            return 0L;
        }

        // Create a Object-Specific ViewObjectContact, set ViewContact and
        // ObjectContact. Always needs to return something.
        ViewObjectContact& ViewContactOfSdrPage::CreateObjectSpecificViewObjectContact(ObjectContact& rObjectContact)
        {
            ViewObjectContact* pRetval = new ViewObjectContact(rObjectContact, *this);
            DBG_ASSERT(pRetval, "ViewContactOfSdrPage::CreateObjectSpecificViewObjectContact() failed (!)");

            return *pRetval;
        }

        // method to recalculate the PaintRectangle if the validity flag shows that
        // it is invalid. The flag is set from GetPaintRectangle, thus the implementation
        // only needs to refresh maPaintRectangle itself.
        void ViewContactOfSdrPage::CalcPaintRectangle()
        {
            // Take own painting area
            maPaintRectangle = Rectangle(
                0L,
                0L,
                GetSdrPage().GetWdt() + PAPER_SHADOW(GetSdrPage().GetWdt()),
                GetSdrPage().GetHgt() + PAPER_SHADOW(GetSdrPage().GetHgt()));

            // Combine with all contained object's rectangles
            maPaintRectangle.Union(GetSdrPage().GetAllObjBoundRect());
        }

        // Because of old correction hacks there may be MasterPages (better:
        // MasterPageDescriptors) set at the page, but actually no MasterPages
        // exist at the model. To correctly handle that cases it is necessary to
        // not only get the MasterPageCount() form the page, but also to correct
        // that number if the model does not have any MasterPages.
        sal_uInt32 ViewContactOfSdrPage::ImpGetCorrectedMasterPageCount() const
        {
            sal_uInt32 nRetval(GetSdrPage().TRG_HasMasterPage() ? 1L : 0L);
            return nRetval;
        }

        ViewContactOfSdrPage::ViewContactOfSdrPage(SdrPage& rPage)
        :   ViewContact(),
            mrPage(rPage)
        {
        }

        ViewContactOfSdrPage::~ViewContactOfSdrPage()
        {
        }

        // When ShouldPaintObject() returns sal_True, the object itself is painted and
        // PaintObject() is called.
        sal_Bool ViewContactOfSdrPage::ShouldPaintObject(DisplayInfo& rDisplayInfo, const ViewObjectContact& rAssociatedVOC)
        {
            // #116481# Test page painting. Suppress output when control layer is painting.
            if(rDisplayInfo.GetControlLayerPainting())
            {
                return sal_False;
            }

            // #i29089#
            // Instead of simply returning sal_True, look for the state
            // of PagePainting which is per default disabled for the applications
            // which do not need let the page be painted (but do it themselves).
            return rDisplayInfo.GetPagePainting();
        }

        // Paint this object. This is before evtl. SubObjects get painted. It needs to return
        // sal_True when something was pained and the paint output rectangle in rPaintRectangle.
        sal_Bool ViewContactOfSdrPage::PaintObject(DisplayInfo& rDisplayInfo, Rectangle& rPaintRectangle, const ViewObjectContact& rAssociatedVOC)
        {
            // #115593#
            // set paint flags and rectangle
            rPaintRectangle = GetPaintRectangle();

            return sal_True;
        }

        // Pre- and Post-Paint this object. Is used e.g. for page background/foreground painting.
        void ViewContactOfSdrPage::PrePaintObject(DisplayInfo& rDisplayInfo, const ViewObjectContact& rAssociatedVOC)
        {
            // Make processed page accessible from SdrPageView via DisplayInfo
            rDisplayInfo.SetProcessedPage(&GetSdrPage());

            // test for page painting
            if(!rDisplayInfo.GetControlLayerPainting() && rDisplayInfo.GetPagePainting())
            {
                // Test for printer output
                if(!rDisplayInfo.OutputToPrinter())
                {
                    if(rDisplayInfo.DoContinuePaint())
                    {
                        const SdrPageView* pPageView = rDisplayInfo.GetPageView();

                        if(pPageView)
                        {
                            // Paint (erase) the background.  That was
                            // formerly done in DrawPaper() but has to be
                            // done even when the paper (the page) is not
                            // painted.
                            OutputDevice* pOut=rDisplayInfo.GetOutputDevice();
                            pOut->SetBackground(Wallpaper(
                                pPageView->GetApplicationBackgroundColor()));
                            pOut->SetLineColor();
                            pOut->Erase();

                            const SdrView& rView = pPageView->GetView();

                            if(rView.IsPageVisible())
                            {
                                DrawPaper(rDisplayInfo);

                                if(rView.IsPageBorderVisible())
                                {
                                    DrawPaperBorder(rDisplayInfo);
                                }
                            }

                            if(rView.IsBordVisible())
                            {
                                DrawBorder(rDisplayInfo);
                            }

                            if(rView.IsGridVisible() && !rView.IsGridFront())
                            {
                                DrawGrid(rDisplayInfo);
                            }

                            if(rView.IsHlplVisible() && !rView.IsHlplFront())
                            {
                                DrawHelplines(rDisplayInfo);
                            }
                        }
                    }
                }
            }
        }

        // Pre- and Post-Paint this object. Is used e.g. for page background/foreground painting.
        void ViewContactOfSdrPage::PostPaintObject(DisplayInfo& rDisplayInfo, const ViewObjectContact& rAssociatedVOC)
        {
            // test for page painting
            if(!rDisplayInfo.GetControlLayerPainting() && rDisplayInfo.GetPagePainting())
            {
                // Test for printer output
                if(!rDisplayInfo.OutputToPrinter())
                {
                    if(rDisplayInfo.DoContinuePaint())
                    {
                        const SdrPageView* pPageView = rDisplayInfo.GetPageView();

                        if(pPageView)
                        {
                            const SdrView& rView = pPageView->GetView();

                            if(rView.IsGridVisible() && rView.IsGridFront())
                            {
                                DrawGrid(rDisplayInfo);
                            }

                            if(rView.IsHlplVisible() && rView.IsHlplFront())
                            {
                                DrawHelplines(rDisplayInfo);
                            }
                        }
                    }
                }
            }

            // Reset processed page at DisplayInfo and DisplayInfo at SdrPageView
            rDisplayInfo.SetProcessedPage(0L);
        }

        void ViewContactOfSdrPage::DrawPaper(DisplayInfo& rDisplayInfo)
        {
            const SdrPageView* pPageView = rDisplayInfo.GetPageView();

            if(pPageView)
            {
                OutputDevice* pOut = rDisplayInfo.GetOutputDevice();
                // No line drawing
                pOut->SetLineColor();

                // Set page color
                if(pPageView->GetApplicationDocumentColor() != COL_AUTO)
                {
                    pOut->SetFillColor(pPageView->GetApplicationDocumentColor());
                }
                else
                {
                    pOut->SetFillColor(rDisplayInfo.GetColorConfig().GetColorValue(svtools::DOCCOLOR).nColor);
                }

                // prepare rectangles in logic coordinates
                Rectangle aPaperRect(
                    0L,
                    0L,
                    GetSdrPage().GetWdt(),
                    GetSdrPage().GetHgt());
                Rectangle aShadowRect(aPaperRect);
                const sal_Bool bOutputToMetaFile(rDisplayInfo.OutputToRecordingMetaFile());
                const sal_Bool bWasEnabled(pOut->IsMapModeEnabled());

                aShadowRect.Move(PAPER_SHADOW(GetSdrPage().GetWdt()), PAPER_SHADOW(GetSdrPage().GetHgt()));

                if(bOutputToMetaFile)
                {
                    // draw page rectangle
                    pOut->DrawRect(aPaperRect);
                }
                else
                {
                    // draw page rectangle in pixel
                    aPaperRect = pOut->LogicToPixel(aPaperRect);
                    pOut->EnableMapMode(sal_False);
                    pOut->DrawRect(aPaperRect);
                    pOut->EnableMapMode(bWasEnabled);
                }

                // set page shadow color
                const Color aShadowColor(rDisplayInfo.GetColorConfig().GetColorValue(svtools::FONTCOLOR).nColor);
                pOut->SetFillColor(aShadowColor);

                if(bOutputToMetaFile)
                {
                    // draw shadow rectangles
                    pOut->DrawRect(Rectangle(
                        aPaperRect.Right(),
                        aShadowRect.Top(),
                        aShadowRect.Right(),
                        aShadowRect.Bottom()));

                    pOut->DrawRect(Rectangle(
                        aShadowRect.Left(),
                        aPaperRect.Bottom(),
                        aPaperRect.Right(),
                        aShadowRect.Bottom()));
                }
                else
                {
                    // draw shadow rectangles in pixels
                    aShadowRect = pOut->LogicToPixel(aShadowRect);
                    pOut->EnableMapMode(sal_False);

                    pOut->DrawRect(Rectangle(
                        aPaperRect.Right() + 1L,
                        aShadowRect.Top(),
                        aShadowRect.Right(),
                        aShadowRect.Bottom()));

                    pOut->DrawRect(Rectangle(
                        aShadowRect.Left(),
                        aPaperRect.Bottom() + 1L,
                        aPaperRect.Right(),
                        aShadowRect.Bottom()));

                    // restore MapMode
                    pOut->EnableMapMode(bWasEnabled);
                }
            }
        }

        void ViewContactOfSdrPage::DrawPaperBorder(DisplayInfo& rDisplayInfo)
        {
            OutputDevice* pOut = rDisplayInfo.GetOutputDevice();

            pOut->SetLineColor(Color(rDisplayInfo.GetColorConfig().GetColorValue(svtools::FONTCOLOR).nColor));
            pOut->SetFillColor();
            pOut->DrawRect(Rectangle(
                0L, 0L,
                GetSdrPage().GetWdt(), GetSdrPage().GetHgt()));
        }

        void ViewContactOfSdrPage::DrawBorder(DisplayInfo& rDisplayInfo)
        {
            if(GetSdrPage().GetLftBorder() || GetSdrPage().GetUppBorder() || GetSdrPage().GetRgtBorder() || GetSdrPage().GetLwrBorder())
            {
                OutputDevice* pOut = rDisplayInfo.GetOutputDevice();
                Color aBorderColor;

                if(Application::GetSettings().GetStyleSettings().GetHighContrastMode())
                {
                    aBorderColor = rDisplayInfo.GetColorConfig().GetColorValue(svtools::FONTCOLOR).nColor;
                }
                else
                {
                    aBorderColor = rDisplayInfo.GetColorConfig().GetColorValue(svtools::DOCBOUNDARIES).nColor;
                }

                pOut->SetLineColor(aBorderColor);
                pOut->SetFillColor();

                Rectangle aRect(Rectangle(
                    0L, 0L,
                    GetSdrPage().GetWdt(), GetSdrPage().GetHgt()));

                aRect.Left() += GetSdrPage().GetLftBorder();
                aRect.Top() += GetSdrPage().GetUppBorder();
                aRect.Right() -= GetSdrPage().GetRgtBorder();
                aRect.Bottom() -= GetSdrPage().GetLwrBorder();

                pOut->DrawRect(aRect);
            }
        }

        void ViewContactOfSdrPage::DrawHelplines(DisplayInfo& rDisplayInfo)
        {
            const SdrPageView* pPageView = rDisplayInfo.GetPageView();

            if(pPageView)
            {
                const SdrHelpLineList& rHelpLineList = pPageView->GetHelpLines();
                OutputDevice* pOut = rDisplayInfo.GetOutputDevice();

                rHelpLineList.DrawAll(*pOut, Point());
            }
        }

        void ViewContactOfSdrPage::DrawGrid(DisplayInfo& rDisplayInfo)
        {
            const SdrPageView* pPageView = rDisplayInfo.GetPageView();

            if(pPageView)
            {
                const SdrView& rView = pPageView->GetView();
                OutputDevice* pOut = rDisplayInfo.GetOutputDevice();

                ((SdrPageView*)pPageView)->DrawGrid(
                    *pOut,
                    rDisplayInfo.GetPaintInfoRec()->aCheckRect,
                    rView.GetGridColor());
            }
        }

        // Access to possible sub-hierarchy
        sal_uInt32 ViewContactOfSdrPage::GetObjectCount() const
        {
            // Handle the MasterPages added to this Page as objects on that page,
            // before the normal draw objects. This leads to a correct draw
            // hierarchy which will handle the MasterPages correctly.
            return ImpGetCorrectedMasterPageCount() + GetSdrPage().GetObjCount();
        }

        ViewContact& ViewContactOfSdrPage::GetViewContact(sal_uInt32 nIndex) const
        {
            const sal_uInt32 nMasterPageCount(ImpGetCorrectedMasterPageCount());

            if(nIndex < nMasterPageCount)
            {
                // return the added MasterPage as sub-hierarchy of the draw page
                SdrPage& rMasterPage = GetSdrPage().TRG_GetMasterPage();
                ViewContact& rViewContactOfMasterPage = rMasterPage.GetViewContact();

                // set visible layers from MasterPage Layer info
                SetOfByte aMasterPageVisibleLayers = GetSdrPage().TRG_GetMasterPageVisibleLayers();
                rViewContactOfMasterPage.SetVisibleLayers(aMasterPageVisibleLayers);

                return rViewContactOfMasterPage;
            }
            else
            {
                // return the (nIndex - nMasterPageCount)'th object
                SdrObject* pObj = GetSdrPage().GetObj(nIndex - nMasterPageCount);
                DBG_ASSERT(pObj, "ViewContactOfMasterPage::GetViewContact: Corrupt SdrObjList (!)");
                return pObj->GetViewContact();
            }
        }

        sal_Bool ViewContactOfSdrPage::GetParentContacts(ViewContactVector& rVContacts) const
        {
            // a page has no parent, it's the top of the hierarchy
            return sal_False;
        }

        // Does this ViewContact support animation?
        sal_Bool ViewContactOfSdrPage::SupportsAnimation() const
        {
            // No.
            return sal_False;
        }

        // overload for acessing the SdrPage
        SdrPage* ViewContactOfSdrPage::TryToGetSdrPage() const
        {
            return &GetSdrPage();
        }
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////
// eof
