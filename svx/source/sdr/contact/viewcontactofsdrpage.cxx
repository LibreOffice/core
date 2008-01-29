/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: viewcontactofsdrpage.cxx,v $
 *
 *  $Revision: 1.22 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-29 14:05:04 $
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

#ifndef _SDR_CONTACT_VIEWCONTACTOFSDRPAGE_HXX
#include <svx/sdr/contact/viewcontactofsdrpage.hxx>
#endif

#ifndef _SDR_CONTACT_VIEWOBJECTCONTACT_HXX
#include <svx/sdr/contact/viewobjectcontact.hxx>
#endif

#ifndef _SVDPAGE_HXX
#include <svx/svdpage.hxx>
#endif

#ifndef _SDR_CONTACT_DISPLAYINFO_HXX
#include <svx/sdr/contact/displayinfo.hxx>
#endif

#ifndef _SVDPAGV_HXX
#include <svx/svdpagv.hxx>
#endif

#ifndef _SVDVIEW_HXX
#include <svx/svdview.hxx>
#endif

#ifndef _SDR_CONTACT_VIEWCONTACTPAINTHELPER_HXX
#include <svx/sdr/contact/viewcontactpainthelper.hxx>
#endif

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

// #i71130#
#ifndef _SDR_CONTACT_OBJECTCONTACT_HXX
#include <svx/sdr/contact/objectcontact.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////

#define PAPER_SHADOW(SIZE) (SIZE >> 8)

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
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
        sal_Bool ViewContactOfSdrPage::ShouldPaintObject(DisplayInfo& rDisplayInfo, const ViewObjectContact& /*rAssociatedVOC*/)
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
            if(!rDisplayInfo.GetPagePainting())
            {
                return sal_False;
            }

            return sal_True;
        }

        // Paint this object. This is before evtl. SubObjects get painted. It needs to return
        // sal_True when something was pained and the paint output rectangle in rPaintRectangle.
        sal_Bool ViewContactOfSdrPage::PaintObject(DisplayInfo& /*rDisplayInfo*/, Rectangle& rPaintRectangle, const ViewObjectContact& /*rAssociatedVOC*/)
        {
            // #115593#
            // set paint flags and rectangle
            rPaintRectangle = GetPaintRectangle();

            return sal_True;
        }

        // Pre- and Post-Paint this object. Is used e.g. for page background/foreground painting.
        void ViewContactOfSdrPage::PrePaintObject(DisplayInfo& rDisplayInfo, const ViewObjectContact& rAssociatedVOC)
        {
            // test for page painting
            if(!rDisplayInfo.GetMasterPagePainting()
                && !rDisplayInfo.GetControlLayerPainting()
                && rDisplayInfo.GetPagePainting())
            {
                // Make processed page accessible from SdrPageView via DisplayInfo
                rDisplayInfo.SetProcessedPage(&GetSdrPage());

                // Test for printer output
                if(!rDisplayInfo.OutputToPrinter())
                {
                    if(rDisplayInfo.DoContinuePaint())
                    {
                        const SdrPageView* pPageView = rDisplayInfo.GetPageView();

                        if(pPageView)
                        {
                            // #i31599# do not paint page context itself ghosted.
                            const sal_Bool bGhostedWasActive(rDisplayInfo.IsGhostedDrawModeActive());
                            if(bGhostedWasActive)
                            {
                                rDisplayInfo.ClearGhostedDrawMode();
                            }

                            // #i34947#
                            // Initialize background. Dependent of IsPageVisible, use
                            // ApplicationBackgroundColor or ApplicationDocumentColor. Most
                            // old renderers for export (html, pdf, gallery, ...) set the
                            // page to not visible (SetPageVisible(false)). They expect the
                            // given OutputDevice to be initialized with the
                            // ApplicationDocumentColor then.
                            const SdrView& rView = pPageView->GetView();
                            Color aInitColor;

                            if(rView.IsPageVisible())
                            {
                                // #i48367# also react on autocolor here
                                aInitColor = pPageView->GetApplicationBackgroundColor();

                                if(Color(COL_AUTO) == aInitColor)
                                {
                                    aInitColor = Color(rDisplayInfo.GetColorConfig().GetColorValue(svtools::APPBACKGROUND).nColor);
                                }
                            }
                            else
                            {
                                aInitColor = pPageView->GetApplicationDocumentColor();

                                if(Color(COL_AUTO) == aInitColor)
                                {
                                    aInitColor = Color(rDisplayInfo.GetColorConfig().GetColorValue(svtools::DOCCOLOR).nColor);
                                }
                            }

                            // init background with InitColor
                            OutputDevice* pOut = rDisplayInfo.GetOutputDevice();
                            pOut->SetBackground(Wallpaper(aInitColor));
                            pOut->SetLineColor();
                            pOut->Erase();

                            if(rView.IsPageVisible())
                            {
                                DrawPaper(rDisplayInfo, rAssociatedVOC);

                                if(rView.IsPageBorderVisible())
                                {
                                    DrawPaperBorder(rDisplayInfo, GetSdrPage());
                                }
                            }

                            if(rView.IsBordVisible())
                            {
                                DrawBorder(rView.IsBordVisibleOnlyLeftRight(),rDisplayInfo, GetSdrPage());
                            }

                            // #i71130# find out if OC is preview renderer
                            const bool bPreviewRenderer(rAssociatedVOC.GetObjectContact().IsPreviewRenderer());

                            // #i71130# no grid and no helplines for page previews
                            if(!bPreviewRenderer)
                            {
                                if(!bPreviewRenderer && rView.IsGridVisible() && !rView.IsGridFront())
                                {
                                    DrawGrid(rDisplayInfo);
                                }

                                if(!bPreviewRenderer && rView.IsHlplVisible() && !rView.IsHlplFront())
                                {
                                    DrawHelplines(rDisplayInfo);
                                }
                            }

                            // #i31599# restore remembered ghosted setting
                            if(bGhostedWasActive)
                            {
                                rDisplayInfo.SetGhostedDrawMode();
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
            if(!rDisplayInfo.GetMasterPagePainting()
                && !rDisplayInfo.GetControlLayerPainting()
                && rDisplayInfo.GetPagePainting())
            {
                // Test for printer output
                if(!rDisplayInfo.OutputToPrinter())
                {
                    if(rDisplayInfo.DoContinuePaint())
                    {
                        const SdrPageView* pPageView = rDisplayInfo.GetPageView();

                        if(pPageView)
                        {
                            // #i31599# do not paint page context itself ghosted.
                            const sal_Bool bGhostedWasActive(rDisplayInfo.IsGhostedDrawModeActive());
                            if(bGhostedWasActive)
                            {
                                rDisplayInfo.ClearGhostedDrawMode();
                            }

                            const SdrView& rView = pPageView->GetView();

                            // #i71130# find out if OC is preview renderer
                            const bool bPreviewRenderer(rAssociatedVOC.GetObjectContact().IsPreviewRenderer());

                            // #i71130# no grid and no helplines for page previews
                            if(!bPreviewRenderer)
                            {
                                if(rView.IsGridVisible() && rView.IsGridFront())
                                {
                                    DrawGrid(rDisplayInfo);
                                }

                                if(rView.IsHlplVisible() && rView.IsHlplFront())
                                {
                                    DrawHelplines(rDisplayInfo);
                                }
                            }

                            // #i31599# restore remembered ghosted setting
                            if(bGhostedWasActive)
                            {
                                rDisplayInfo.SetGhostedDrawMode();
                            }
                        }
                    }
                }

                // Reset processed page at DisplayInfo and DisplayInfo at SdrPageView
                rDisplayInfo.SetProcessedPage(0L);
            }
        }

        void ViewContactOfSdrPage::DrawPaper(DisplayInfo& rDisplayInfo, const ViewObjectContact& rAssociatedVOC)
        {
            const SdrPageView* pPageView = rDisplayInfo.GetPageView();

            if(pPageView)
            {
                OutputDevice* pOut = rDisplayInfo.GetOutputDevice();
                // No line drawing
                pOut->SetLineColor();

                // prepare rectangle
                const Rectangle aPaperRectLogic(
                    0L,
                    0L,
                    GetSdrPage().GetWdt(),
                    GetSdrPage().GetHgt());

                // prepare ShadowRectangle
                Rectangle aShadowRectLogic(aPaperRectLogic);
                aShadowRectLogic.Move(PAPER_SHADOW(GetSdrPage().GetWdt()), PAPER_SHADOW(GetSdrPage().GetHgt()));

                // get some flags
                const sal_Bool bOutputToMetaFile(rDisplayInfo.OutputToRecordingMetaFile());
                const sal_Bool bWasEnabled(pOut->IsMapModeEnabled());

                // #i34682#
                // look if the MasterPageBackgroundObject needs to be painted, else
                // paint page as normal
                sal_Bool bPaintMasterObject(sal_False);
                sal_Bool bPaintPageBackground(sal_True);
                SdrObject* pMasterPageObjectCandidate = 0L;

                if(GetSdrPage().IsMasterPage())
                {
                    pMasterPageObjectCandidate = GetSdrPage().GetObj(0L);

                    if(pMasterPageObjectCandidate
                        && pMasterPageObjectCandidate->IsMasterPageBackgroundObject()
                        && pMasterPageObjectCandidate->HasFillStyle())
                    {
                        bPaintMasterObject = sal_True;
                    }

                    if(bPaintMasterObject)
                    {
                        // #i51798# when the fill mode guarantees that the full page will
                        // be painted with PaintMasterObject, bPaintPageBackground may be
                        // set to false. This is the case for XFILL_SOLID and XFILL_GRADIENT
                        const SfxItemSet& rSet = pMasterPageObjectCandidate->GetMergedItemSet();
                        const XFillStyle eFillStyle = ((const XFillStyleItem&)(rSet.Get(XATTR_FILLSTYLE))).GetValue();

                        if(XFILL_SOLID == eFillStyle || XFILL_GRADIENT == eFillStyle)
                        {
                            bPaintPageBackground = sal_False;
                        }
                    }
                }

                if(bPaintPageBackground)
                {
                    // Set page color for Paper painting
                    if(pPageView->GetApplicationDocumentColor() != COL_AUTO)
                    {
                        pOut->SetFillColor(pPageView->GetApplicationDocumentColor());
                    }
                    else
                    {
                        pOut->SetFillColor(rDisplayInfo.GetColorConfig().GetColorValue(svtools::DOCCOLOR).nColor);
                    }

                    if(bOutputToMetaFile)
                    {
                        // draw page rectangle
                        pOut->DrawRect(aPaperRectLogic);
                    }
                    else
                    {
                        // draw page rectangle in pixel
                        const Rectangle aPaperRectPixel(pOut->LogicToPixel(aPaperRectLogic));
                        pOut->EnableMapMode(sal_False);
                        pOut->DrawRect(aPaperRectPixel);
                        pOut->EnableMapMode(bWasEnabled);
                    }
                }

                if(bPaintMasterObject)
                {
                    // draw a MasterPage background for a MasterPage in MasterPage View
                    Rectangle aRectangle;
                    PaintBackgroundObject(*this, *pMasterPageObjectCandidate, rDisplayInfo, aRectangle, rAssociatedVOC);
                }

                // set page shadow color
                const Color aShadowColor(rDisplayInfo.GetColorConfig().GetColorValue(svtools::FONTCOLOR).nColor);
                pOut->SetFillColor(aShadowColor);

                if(bOutputToMetaFile)
                {
                    // draw shadow rectangles
                    pOut->DrawRect(Rectangle(
                        aPaperRectLogic.Right(),
                        aShadowRectLogic.Top(),
                        aShadowRectLogic.Right(),
                        aShadowRectLogic.Bottom()));

                    pOut->DrawRect(Rectangle(
                        aShadowRectLogic.Left(),
                        aPaperRectLogic.Bottom(),
                        aPaperRectLogic.Right(),
                        aShadowRectLogic.Bottom()));
                }
                else
                {
                    // draw shadow rectangles in pixels
                    const Rectangle aShadowRectPixel(pOut->LogicToPixel(aShadowRectLogic));
                    const Rectangle aPaperRectPixel(pOut->LogicToPixel(aPaperRectLogic));
                    pOut->EnableMapMode(sal_False);

                    pOut->DrawRect(Rectangle(
                        aPaperRectPixel.Right() + 1L,
                        aShadowRectPixel.Top(),
                        aShadowRectPixel.Right(),
                        aShadowRectPixel.Bottom()));

                    pOut->DrawRect(Rectangle(
                        aShadowRectPixel.Left(),
                        aPaperRectPixel.Bottom() + 1L,
                        aPaperRectPixel.Right(),
                        aShadowRectPixel.Bottom()));

                    // restore MapMode
                    pOut->EnableMapMode(bWasEnabled);
                }
            }
        }

        // #i37869#
        void ViewContactOfSdrPage::DrawPaperBorder(DisplayInfo& rDisplayInfo, const SdrPage& rPage)
        {
            // #i42714#
            if(!rDisplayInfo.OutputToPrinter())
            {
                OutputDevice* pOut = rDisplayInfo.GetOutputDevice();

                pOut->SetLineColor(Color(rDisplayInfo.GetColorConfig().GetColorValue(svtools::FONTCOLOR).nColor));
                pOut->SetFillColor();
                pOut->DrawRect(Rectangle(
                    0L, 0L,
                    rPage.GetWdt(), rPage.GetHgt()));
            }
        }

        // #i37869#
        void ViewContactOfSdrPage::DrawBorder(BOOL _bDrawOnlyLeftRightBorder,DisplayInfo& rDisplayInfo, const SdrPage& rPage)
        {
            // #i42714#
            if(!rDisplayInfo.OutputToPrinter())
            {
                if(rPage.GetLftBorder() || rPage.GetUppBorder() || rPage.GetRgtBorder() || rPage.GetLwrBorder())
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
                        rPage.GetWdt(), rPage.GetHgt()));

                    aRect.Left() += rPage.GetLftBorder();
                    aRect.Top() += rPage.GetUppBorder();
                    aRect.Right() -= rPage.GetRgtBorder();
                    aRect.Bottom() -= rPage.GetLwrBorder();

                    if ( _bDrawOnlyLeftRightBorder )
                    {
                        // oj: draw only left right border for the Sun Report Builder
                        pOut->DrawLine(aRect.TopLeft(),aRect.BottomLeft());
                        pOut->DrawLine(aRect.TopRight(),aRect.BottomRight());
                    }
                    else
                        pOut->DrawRect(aRect);
                }
            }
        }

        void ViewContactOfSdrPage::DrawHelplines(DisplayInfo& rDisplayInfo)
        {
            // #i42714#
            if(!rDisplayInfo.OutputToPrinter())
            {
                const SdrPageView* pPageView = rDisplayInfo.GetPageView();

                if(pPageView)
                {
                    const SdrHelpLineList& rHelpLineList = pPageView->GetHelpLines();
                    OutputDevice* pOut = rDisplayInfo.GetOutputDevice();

                    rHelpLineList.DrawAll(*pOut, Point());
                }
            }
        }

        void ViewContactOfSdrPage::DrawGrid(DisplayInfo& rDisplayInfo)
        {
            // #i42714#
            if(!rDisplayInfo.OutputToPrinter())
            {
                const SdrPageView* pPageView = rDisplayInfo.GetPageView();

                if(pPageView)
                {
                    const SdrView& rView = pPageView->GetView();
                    OutputDevice* pOut = rDisplayInfo.GetOutputDevice();

                    ((SdrPageView*)pPageView)->DrawPageViewGrid(
                        *pOut,
                        rDisplayInfo.GetPaintInfoRec()->aCheckRect,
                        rView.GetGridColor());
                }
            }
        }

        // Access to possible sub-hierarchy
        sal_uInt32 ViewContactOfSdrPage::GetObjectCount() const
        {
            sal_uInt32 nRetval(0L);
            const sal_uInt32 nMasterPageCount((GetSdrPage().TRG_HasMasterPage()) ? 1L : 0L);
            sal_uInt32 nSubObjectCount(GetSdrPage().GetObjCount());

            // correct nSubObjectCount from MasterPageBackgroundObject
            if(nSubObjectCount && GetSdrPage().GetObj(0L)->IsMasterPageBackgroundObject())
            {
                nSubObjectCount--;
            }

            // add MasterPageDescriptor if used
            nRetval += nMasterPageCount;

            // add page sub-objects
            nRetval += nSubObjectCount;

            return nRetval;
        }

        ViewContact& ViewContactOfSdrPage::GetViewContact(sal_uInt32 nIndex) const
        {
            // this is only called if GetObjectCount() returned != 0L, so there is a
            // MasterPageDescriptor. Get it!
            const sal_uInt32 nMasterPageCount((GetSdrPage().TRG_HasMasterPage()) ? 1L : 0L);

            if(nIndex < nMasterPageCount)
            {
                return GetSdrPage().TRG_GetMasterPageDescriptorViewContact();
            }
            else
            {
                // return the (nIndex - nMasterPageCount)'th object
                sal_uInt32 nObjectIndex(nIndex - nMasterPageCount);

                // correct if first object is MasterPageBackgroundObject
                if(GetSdrPage().GetObjCount() && GetSdrPage().GetObj(0L)->IsMasterPageBackgroundObject())
                {
                    nObjectIndex++;
                }

                SdrObject* pObj = GetSdrPage().GetObj(nObjectIndex);
                DBG_ASSERT(pObj, "ViewContactOfMasterPage::GetViewContact: Corrupt SdrObjList (!)");

                return pObj->GetViewContact();
            }
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
