/*************************************************************************
 *
 *  $RCSfile: viewcontactofmasterpagedescriptor.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2004-10-12 10:07:39 $
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

#ifndef _SDR_CONTACT_VIEWCONTACTOFMASTERPAGEDESCRIPTOR_HXX
#include <svx/sdr/contact/viewcontactofmasterpagedescriptor.hxx>
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

#ifndef _SVDOBJ_HXX
#include <svdobj.hxx>
#endif

#ifndef _SDR_CONTACT_OBJECTCONTACTOFOBJLISTPAINTER_HXX
#include <svx/sdr/contact/objectcontactofobjlistpainter.hxx>
#endif

#ifndef _SDR_CONTACT_VIEWCONTACTPAINTHELPER_HXX
#include <svx/sdr/contact/viewcontactpainthelper.hxx>
#endif

#ifndef _SDR_CONTACT_VOCBITMAPBUFFER_HXX
#include <svx/sdr/contact/vocbitmapbuffer.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////

#define PAPER_SHADOW(SIZE) (SIZE >> 8)

//////////////////////////////////////////////////////////////////////////////
// Local MasterPagePainter

namespace sdr
{
    namespace contact
    {
        class OwnMasterPagePainter : public ObjectContactOfPagePainter
        {
        protected:
            // the descriptor we are working with
            ::sdr::contact::ViewContactOfMasterPageDescriptor&      mrVCOfMasterPageDescriptor;

            // access to MasterPageDescriptor
            sdr::MasterPageDescriptor& GetMasterPageDescriptor() const
            {
                return mrVCOfMasterPageDescriptor.GetMasterPageDescriptor();
            }

        public:
            // basic constructor
            OwnMasterPagePainter(
                ::sdr::contact::ViewContactOfMasterPageDescriptor& rViewContact,
                sal_Bool bBufferingAllowed = sal_False);

            // The destructor. When PrepareDelete() was not called before (see there)
            // warnings will be generated in debug version if there are still contacts
            // existing.
            virtual ~OwnMasterPagePainter();

            // Own paint
            sal_Bool PaintIt(DisplayInfo& rDisplayInfo, Rectangle& rDestinationRectangle);

            // test if visualizing of entered groups is switched on at all. Default
            // implementation returns sal_False.
            virtual sal_Bool DoVisualizeEnteredGroup() const;

            // Own reaction on changes
            virtual void InvalidatePartOfView(const Rectangle& rRectangle) const;
            virtual void ObjectGettingPotentiallyVisible(const ViewObjectContact& rVOC) const;
        };

        //////////////////////////////////////////////////////////////////////////////
        // Implementation
        OwnMasterPagePainter::OwnMasterPagePainter(
            ::sdr::contact::ViewContactOfMasterPageDescriptor& rViewContact,
            sal_Bool bBufferingAllowed)
        :   ObjectContactOfPagePainter(&(rViewContact.GetMasterPageDescriptor().GetUsedPage()), bBufferingAllowed),
            mrVCOfMasterPageDescriptor(rViewContact)
        {
        }

        OwnMasterPagePainter::~OwnMasterPagePainter()
        {
        }

        sal_Bool OwnMasterPagePainter::PaintIt(DisplayInfo& rDisplayInfo, Rectangle& rDestinationRectangle)
        {
            // save old clipping from OutDev
            sal_Bool bRetval(sal_False);
            OutputDevice* pOut = rDisplayInfo.GetOutputDevice();

            if(pOut)
            {
                // save clipping
                sal_Bool bRememberedClipping(pOut->IsClipRegion());
                Region aRememberedClipping;

                if(bRememberedClipping)
                {
                    aRememberedClipping = pOut->GetClipRegion();
                }

                // add clipping against object bounds
                pOut->IntersectClipRegion(mrVCOfMasterPageDescriptor.GetPaintRectangle());

                // remember original layers
                SetOfByte aPreprocessedLayers = rDisplayInfo.GetProcessLayers();
                SetOfByte aRememberedLayers = aPreprocessedLayers;

                // use visibility settings originating from SdrMasterPageDescriptor
                aPreprocessedLayers &= GetMasterPageDescriptor().GetVisibleLayers();

                // set preprocessed layer info
                rDisplayInfo.SetProcessLayers(aPreprocessedLayers);

                // set MasterPagePaint flag
                rDisplayInfo.SetMasterPagePainting(sal_True);

                // set flag in PaintMode that MasterPage is painted
                SdrPaintInfoRec* pInfoRec = rDisplayInfo.GetPaintInfoRec();
                pInfoRec->nPaintMode = pInfoRec->nPaintMode | SDRPAINTMODE_MASTERPAGE;

                // keep draw hierarchy up-to-date
                PreProcessDisplay(rDisplayInfo);

                // do processing
                ProcessDisplay(rDisplayInfo);

                // reset special MasterPageLayers
                rDisplayInfo.SetProcessLayers(aRememberedLayers);

                // clear MasterPagePaint flag
                rDisplayInfo.SetMasterPagePainting(sal_False);

                // clear MasterPage painting flag again
                pInfoRec->nPaintMode = pInfoRec->nPaintMode & (~((sal_uInt16)SDRPAINTMODE_MASTERPAGE));

                // restore remembered clipping
                if(bRememberedClipping)
                {
                    pOut->SetClipRegion(aRememberedClipping);
                }
                else
                {
                    pOut->SetClipRegion();
                }

                bRetval = sal_True;
                rDestinationRectangle = mrVCOfMasterPageDescriptor.GetPaintRectangle();
            }

            return bRetval;
        }

        void OwnMasterPagePainter::InvalidatePartOfView(const Rectangle& rRectangle) const
        {
            // call user change
            mrVCOfMasterPageDescriptor.ActionChanged();
        }

        void OwnMasterPagePainter::ObjectGettingPotentiallyVisible(const ViewObjectContact& rVOC) const
        {
            // call user change
            mrVCOfMasterPageDescriptor.ActionChanged();
        }

        sal_Bool OwnMasterPagePainter::DoVisualizeEnteredGroup() const
        {
            // switch off entered group visualisation for MasterPage content painting
            return sal_False;
        }
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
        // Get the correct BackgroundObject
        SdrObject* ViewContactOfMasterPageDescriptor::GetBackgroundObject() const
        {
            SdrObject* pRetval = 0L;
            const SdrPage& rMasterPage = GetMasterPageDescriptor().GetUsedPage();

            // Here i will rely on old knowledge about the 0'st element of a masterpage
            // being the PageBackgroundObject. This will be removed again when that definition
            // will be changed.
            const sal_uInt32 nMasterPageObjectCount(rMasterPage.GetObjCount());
            DBG_ASSERT(1 <= nMasterPageObjectCount,
                "ViewContactOfMasterPageDescriptor::GetBackgroundObject(): MasterPageBackgroundObject missing (!)");
            pRetval = rMasterPage.GetObj(0L);

            // Test if it's really what we need. There are known problems where
            // the 0th object is not the MasterPageBackgroundObject at all.
            if(!pRetval->IsMasterPageBackgroundObject())
            {
                pRetval = 0L;
            }

            // Get the evtl. existing page background object from the using page and use it
            // preferred to the MasterPageBackgroundObject
            const SdrPage& rOwnerPage = GetMasterPageDescriptor().GetOwnerPage();
            SdrObject* pCandidate = rOwnerPage.GetBackgroundObj();

            if(pCandidate)
            {
                pRetval = pCandidate;
            }

            return pRetval;
        }

        // Get the LayerId of the BackgroundObject
        sal_uInt8 ViewContactOfMasterPageDescriptor::GetBackgroundObjectLayerId() const
        {
            SdrObject* pCandidate = GetBackgroundObject();

            if(pCandidate)
            {
                return pCandidate->GetLayer();
            }

            return 0;
        }

        // Create a Object-Specific ViewObjectContact, set ViewContact and
        // ObjectContact. Always needs to return something. Default is to create
        // a standard ViewObjectContact containing the given ObjectContact and *this
        ViewObjectContact& ViewContactOfMasterPageDescriptor::CreateObjectSpecificViewObjectContact(ObjectContact& rObjectContact)
        {
            ViewObjectContact* pRetval = 0L;

            if(rObjectContact.IsMasterPageBufferingAllowed())
            {
                // buffered
                pRetval = new VOCBitmapBuffer(rObjectContact, *this);
            }
            else
            {
                // standard
                pRetval = &ViewContact::CreateObjectSpecificViewObjectContact(rObjectContact);
            }

            DBG_ASSERT(pRetval, "ViewContactOfMasterPageDescriptor::CreateObjectSpecificViewObjectContact() failed (!)");
            return *pRetval;
        }

        // method to recalculate the PaintRectangle if the validity flag shows that
        // it is invalid. The flag is set from GetPaintRectangle, thus the implementation
        // only needs to refresh maPaintRectangle itself.
        void ViewContactOfMasterPageDescriptor::CalcPaintRectangle()
        {
            // return rectangle of BackgroundObject and clipped MasterPage
            const SdrPage& rOwnerPage = GetMasterPageDescriptor().GetOwnerPage();
            maPaintRectangle = Rectangle(
                rOwnerPage.GetLftBorder(),
                rOwnerPage.GetUppBorder(),
                rOwnerPage.GetWdt() - rOwnerPage.GetRgtBorder(),
                rOwnerPage.GetHgt() - rOwnerPage.GetLwrBorder());
        }

        // basic constructor
        ViewContactOfMasterPageDescriptor::ViewContactOfMasterPageDescriptor(sdr::MasterPageDescriptor& rDescriptor)
        :   ViewContact(),
            mrMasterPageDescriptor(rDescriptor),
            mpMasterPagePainter(0L)
        {
            mpMasterPagePainter = new OwnMasterPagePainter(*this);
        }

        // The destructor.
        ViewContactOfMasterPageDescriptor::~ViewContactOfMasterPageDescriptor()
        {
            delete mpMasterPagePainter;
        }

        // When ShouldPaintObject() returns sal_True, the object itself is painted and
        // PaintObject() is called.
        sal_Bool ViewContactOfMasterPageDescriptor::ShouldPaintObject(DisplayInfo& rDisplayInfo,
            const ViewObjectContact& rAssociatedVOC)
        {
            // Test page painting. Suppress output when control layer is painting.
            if(rDisplayInfo.GetControlLayerPainting())
            {
                return sal_False;
            }

            // Test area visibility
            const Region& rRedrawArea = rDisplayInfo.GetRedrawArea();

            if(!rRedrawArea.IsEmpty() && !rRedrawArea.IsOver(GetPaintRectangle()))
            {
                return sal_False;
            }

            // Test layer visibility. PageBackgroundObject uses the background layer.
            if(!rDisplayInfo.GetProcessLayers().IsSet(GetBackgroundObjectLayerId()))
            {
                return sal_False;
            }

            // test against PaintMode combinations
            const sal_uInt32 nDrawMode(rDisplayInfo.GetCurrentDrawMode());

            if(nDrawMode == (DRAWMODE_GRAYLINE | DRAWMODE_GRAYFILL | DRAWMODE_BLACKTEXT | DRAWMODE_GRAYBITMAP | DRAWMODE_GRAYGRADIENT ))
            {
                return sal_False;
            }

            if(nDrawMode == (DRAWMODE_SETTINGSLINE | DRAWMODE_SETTINGSFILL | DRAWMODE_SETTINGSTEXT | DRAWMODE_SETTINGSGRADIENT ))
            {
                return sal_False;
            }
            return sal_True;
        }

        // #115593# Paint this object. This is before evtl. SubObjects get painted. It needs to return
        // sal_True when something was pained and the paint output rectangle in rPaintRectangle.
        sal_Bool ViewContactOfMasterPageDescriptor::PaintObject(DisplayInfo& rDisplayInfo, Rectangle& rPaintRectangle,
            const ViewObjectContact& rAssociatedVOC)
        {
            sal_Bool bRetval(sal_False);

            // Draw the correct BackgroundObject
            SdrObject* pCandidate = GetBackgroundObject();
            if(pCandidate)
            {
                bRetval = PaintBackgroundObject(*this, *pCandidate, rDisplayInfo, rPaintRectangle, rAssociatedVOC);
            }

            // Draw the MasterPage content
            Rectangle aSecondRectangle;

            if(mpMasterPagePainter->PaintIt(rDisplayInfo, aSecondRectangle))
            {
                bRetval = sal_True;
                rPaintRectangle.Union(aSecondRectangle);
            }

            return bRetval;
        }

        ViewContact* ViewContactOfMasterPageDescriptor::GetParentContact() const
        {
            return &(GetMasterPageDescriptor().GetUsedPage().GetViewContact());
        }
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////
// eof
