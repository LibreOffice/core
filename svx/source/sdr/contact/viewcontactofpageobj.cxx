/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: viewcontactofpageobj.cxx,v $
 * $Revision: 1.15 $
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
#include <svx/sdr/contact/viewcontactofpageobj.hxx>
#include <svx/svdopage.hxx>
#include <svx/sdr/contact/displayinfo.hxx>
#include <vcl/outdev.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdpage.hxx>
#include <svx/sdr/contact/objectcontactofobjlistpainter.hxx>
#include <svx/xoutx.hxx>
#include <svx/sdr/contact/viewobjectcontact.hxx>

//////////////////////////////////////////////////////////////////////////////
// Own PagePainter which takes over initialisation tasks

namespace sdr
{
    namespace contact
    {
        class OCOfPageObjPagePainter : public ObjectContactOfPagePainter
        {
        protected:
            Rectangle                               maDestinationRectangle;
            Point                                   maTranslate;
            Fraction                                maScaleX;
            Fraction                                maScaleY;
            ViewContact&                            mrUserViewContact;

        public:
            // basic constructor
            OCOfPageObjPagePainter(const SdrPage* pPage, ViewContact& rUserViewContact);

            // The destructor. When PrepareDelete() was not called before (see there)
            // warnings will be generated in debug version if there are still contacts
            // existing.
            virtual ~OCOfPageObjPagePainter();

            // Own paint
            sal_Bool PaintIt(DisplayInfo& rDisplayInfo, const Rectangle& rDestinationRectangle);

            // Own reaction on changes
            virtual void InvalidatePartOfView(const Rectangle& rRectangle) const;
        };

        //////////////////////////////////////////////////////////////////////////////
        // Implementation

        OCOfPageObjPagePainter::OCOfPageObjPagePainter(const SdrPage* pPage, ViewContact& rUserViewContact)
        :   ObjectContactOfPagePainter(pPage),
            mrUserViewContact(rUserViewContact)
        {
            // #i71130# Mark this view as preview renderer
            mbIsPreviewRenderer = true;
        }

        OCOfPageObjPagePainter::~OCOfPageObjPagePainter()
        {
        }

        sal_Bool OCOfPageObjPagePainter::PaintIt(DisplayInfo& rDisplayInfo, const Rectangle& rDestinationRectangle)
        {
            // save old clipping from OutDev
            sal_Bool bRetval(sal_False);

            OutputDevice* pOut = rDisplayInfo.GetOutputDevice();
            maDestinationRectangle = rDestinationRectangle;

            if(pOut)
            {
                // remap sizes. Destination size is size of rPaintRectangle,
                // source size is the page size.
                MapMode aOldMapMode = pOut->GetMapMode();

                // calc translation
                maTranslate = Point(maDestinationRectangle.TopLeft() + aOldMapMode.GetOrigin());

                // calc scaling
                maScaleX = Fraction(maDestinationRectangle.GetWidth(), mpStartPage->GetWdt());
                maScaleY = Fraction(maDestinationRectangle.GetHeight(), mpStartPage->GetHgt());

                // save clipping
                sal_Bool bRememberedClipping(pOut->IsClipRegion());
                Region aRememberedClipping;

                if(bRememberedClipping)
                {
                    aRememberedClipping = pOut->GetClipRegion();
                }

                // add clipping against object bounds
                pOut->IntersectClipRegion(maDestinationRectangle);

                // change origin
                MapMode aNewMapMode(pOut->GetMapMode());
                aNewMapMode.SetOrigin(maTranslate);
                pOut->SetMapMode(aNewMapMode);

                // change scaling
                Point aEmptyPoint;
                pOut->SetMapMode(MapMode(MAP_RELATIVE, aEmptyPoint, maScaleX, maScaleY));

                // create copy of DisplayInfo. Do copy PageView to have it available at
                // included page painting.
                DisplayInfo aDisplayInfo(rDisplayInfo.GetPageView());
                aDisplayInfo.SetExtendedOutputDevice(rDisplayInfo.GetExtendedOutputDevice());
                aDisplayInfo.SetPaintInfoRec(rDisplayInfo.GetPaintInfoRec());
                aDisplayInfo.SetOutputDevice(pOut);

                // make MasterPages visible
                aDisplayInfo.SetPagePainting(sal_True); // #i72889#

                // do processing
                ProcessDisplay(aDisplayInfo);

                // restore original MapMode
                pOut->SetMapMode(aOldMapMode);

                // restore remembered clipping
                if(bRememberedClipping)
                {
                    pOut->SetClipRegion(aRememberedClipping);
                }
                else
                {
                    pOut->SetClipRegion();
                }
            }

            return bRetval;
        }

        void OCOfPageObjPagePainter::InvalidatePartOfView(const Rectangle& /*rRectangle*/) const
        {
            // call user change
            mrUserViewContact.ActionChanged();
        }
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
        // Access to referenced page
        const SdrPage* ViewContactOfPageObj::GetReferencedPage() const
        {
            return GetPageObj().GetReferencedPage();
        }

        // method to recalculate the PaintRectangle if the validity flag shows that
        // it is invalid. The flag is set from GetPaintRectangle, thus the implementation
        // only needs to refresh maPaintRectangle itself.
        void ViewContactOfPageObj::CalcPaintRectangle()
        {
            maPaintRectangle = GetPageObj().GetCurrentBoundRect();
        }

        // get rid of evtl. remembered PagePainter
        void ViewContactOfPageObj::GetRidOfPagePainter()
        {
            if(mpPagePainter)
            {
                mpPagePainter->PrepareDelete();
                delete mpPagePainter;
                mpPagePainter = 0L;
            }
        }

        // Prepare a PagePainter for current referenced page. This may
        // refresh, create or delete a PagePainter instance in
        // mpPagePainter
        void ViewContactOfPageObj::PreparePagePainter(const SdrPage* pPage)
        {
            if(pPage)
            {
                if(mpPagePainter)
                {
                    mpPagePainter->SetStartPage(pPage);
                }
                else
                {
                    mpPagePainter = new OCOfPageObjPagePainter(pPage, *this);
                }
            }
            else
            {
                GetRidOfPagePainter();
            }
        }

        Rectangle ViewContactOfPageObj::GetPageRectangle (void)
        {
            CalcPaintRectangle();
            return maPaintRectangle;
        }

        // Paint support methods for page content painting
        sal_Bool ViewContactOfPageObj::PaintPageContents(
            DisplayInfo& rDisplayInfo,
            const Rectangle& rPaintRectangle,
            const ViewObjectContact& /*rAssociatedVOC*/)
        {
            // Prepare page painter
            sal_Bool bRetval(sal_False);
            const SdrPage* pPage = GetReferencedPage();
            PreparePagePainter(pPage);

            if(mpPagePainter)
            {
                bRetval = mpPagePainter->PaintIt(rDisplayInfo, rPaintRectangle);
            }

            return bRetval;
        }

        sal_Bool ViewContactOfPageObj::PaintPageReplacement(
            DisplayInfo& rDisplayInfo,
            const Rectangle& rPaintRectangle,
            const ViewObjectContact& /*rAssociatedVOC*/)
        {
            // If painting recursive, paint a replacement visualization
            OutputDevice* pOut = rDisplayInfo.GetOutputDevice();

            svtools::ColorConfigValue aDocColor(
                rDisplayInfo.GetColorConfig().GetColorValue(svtools::DOCCOLOR));
            svtools::ColorConfigValue aBorderColor(
                rDisplayInfo.GetColorConfig().GetColorValue(svtools::DOCBOUNDARIES));

            pOut->SetFillColor(aDocColor.nColor);
            pOut->SetLineColor(aBorderColor.bIsVisible ? aBorderColor.nColor: aDocColor.nColor);

            pOut->DrawRect(rPaintRectangle);

            return sal_True;
        }

        sal_Bool ViewContactOfPageObj::PaintPageBorder(
            DisplayInfo& rDisplayInfo,
            const Rectangle& rPaintRectangle,
            const ViewObjectContact& /*rAssociatedVOC*/)
        {
            OutputDevice* pOut = rDisplayInfo.GetOutputDevice();
            sal_Bool bRetval(sal_False);
            svtools::ColorConfigValue aFrameColor(
                rDisplayInfo.GetColorConfig().GetColorValue(svtools::OBJECTBOUNDARIES));

            if( aFrameColor.bIsVisible )
            {
                pOut->SetFillColor();
                pOut->SetLineColor(aFrameColor.nColor);
                pOut->DrawRect(rPaintRectangle);

                bRetval = sal_True;
            }

            return bRetval;
        }

        // On StopGettingViewed the PagePainter can be dismissed.
        void ViewContactOfPageObj::StopGettingViewed()
        {
            // call parent
            ViewContactOfSdrObj::StopGettingViewed();

            // dismiss PagePainter
            GetRidOfPagePainter();
        }

        ViewContactOfPageObj::ViewContactOfPageObj(SdrPageObj& rPageObj)
        :   ViewContactOfSdrObj(rPageObj),
            mpPagePainter(0L),
            mbIsPainting(sal_False),
            mbIsInActionChange(sal_False)
        {
        }

        ViewContactOfPageObj::~ViewContactOfPageObj()
        {
            GetRidOfPagePainter();
        }

        // Paint this object. This is before evtl. SubObjects get painted. It needs to return
        // sal_True when something was pained and the paint output rectangle in rPaintRectangle.
        sal_Bool ViewContactOfPageObj::PaintObject(DisplayInfo& rDisplayInfo, Rectangle& rPaintRectangle, const ViewObjectContact& rAssociatedVOC)
        {
            sal_Bool bRetval(sal_False);

            // get page to be displayed
            const SdrPage* pPage = GetReferencedPage();

            // avoid recursive painting
            if(mbIsPainting)
            {
                // Paint a replacement object
                const Rectangle aNewRectangle(GetPageRectangle());
                bRetval |= PaintPageReplacement(rDisplayInfo, aNewRectangle, rAssociatedVOC);
                rPaintRectangle.Union(aNewRectangle);
            }
            else
            {
                // something to paint?
                if(pPage)
                {
                    // set recursion flag, see description in *.hxx
                    mbIsPainting = sal_True;

                    // Paint a replacement object.
                    const Rectangle aNewRectangle(GetPageRectangle());
                    bRetval |= PaintPageContents(rDisplayInfo, aNewRectangle, rAssociatedVOC);
                    rPaintRectangle.Union(aNewRectangle);

                    // reset recursion flag, see description in *.hxx
                    mbIsPainting = sal_False;
                }
            }

            // paint frame, but not when printing and no page available
            if(!(rDisplayInfo.OutputToPrinter() && !pPage))
            {
                // #i54989# fill aNewRectangle before usage
                const Rectangle aNewRectangle(GetPageRectangle());
                bRetval |= PaintPageBorder(rDisplayInfo, aNewRectangle, rAssociatedVOC);
                rPaintRectangle.Union(aNewRectangle);
            }

            return bRetval;
        }

        // #i35972# React on changes of the object of this ViewContact
        void ViewContactOfPageObj::ActionChanged()
        {
            if(!mbIsInActionChange)
            {
                // set recursion flag, see description in *.hxx
                mbIsInActionChange = sal_True;

                // call parent
                ViewContactOfSdrObj::ActionChanged();

                // reset recursion flag, see description in *.hxx
                mbIsInActionChange = sal_False;
            }
        }
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////
// eof
