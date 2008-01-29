/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: viewcontactofmasterpagedescriptor.cxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-29 14:04:52 $
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

#ifndef _SDR_CONTACT_VIEWCONTACTOFMASTERPAGEDESCRIPTOR_HXX
#include <svx/sdr/contact/viewcontactofmasterpagedescriptor.hxx>
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

#ifndef _SVDOBJ_HXX
#include <svx/svdobj.hxx>
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

#ifndef _SDR_CONTACT_VOCOFMASTERPAGEDESCRIPTOR_HXX
#include <svx/sdr/contact/vocofmasterpagedescriptor.hxx>
#endif

#ifndef _SV_TIMER_HXX
#include <vcl/timer.hxx>
#endif

#ifndef _SVDPAGV_HXX
#include <svx/svdpagv.hxx>
#endif

#ifndef _SVDVIEW_HXX
#include <svx/svdview.hxx>
#endif

#ifndef _SDR_CONTACT_VIEWCONTACTOFSDRPAGE_HXX
#include <svx/sdr/contact/viewcontactofsdrpage.hxx>
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

        void OwnMasterPagePainter::InvalidatePartOfView(const Rectangle& /*rRectangle*/) const
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
// MasterPageBuffer

// keep used buffers for 30 cycles (5 minutes max)
#define MASTERPAGE_USAGE_COUNT          (30L)
// cycle is 10 seconds
#define MASTERPAGE_TIMEOUT              (10000L)
// maximum count of buffered MasterPages
#define MASTERPAGE_MAXIMUM_COUNT        (8L)

namespace
{
    class MasterPageBufferEntry
    {
        sal_uInt32                                      mnMPUsageCount;
        Bitmap                                          maBitmap;
        MapMode                                         maMapMode;
        SdrPage*                                        mpMasterPage;
        SdrPage*                                        mpPage;
        SdrObject*                                      mpBackgroundObject;

    public:
        MasterPageBufferEntry(
            const Bitmap& rBitmap,
            const MapMode& rMapMode,
            SdrPage* pMasterPage,
            SdrPage* pPage,
            SdrObject* pBackgroundObect)
        :   mnMPUsageCount(MASTERPAGE_USAGE_COUNT),
            maBitmap(rBitmap),
            maMapMode(rMapMode),
            mpMasterPage(pMasterPage),
            mpPage(pPage),
            mpBackgroundObject(pBackgroundObect)
        {
        }

        const Bitmap& GetBitmap() const { return maBitmap; }
        const MapMode& GetMapMode() const { return maMapMode; }
        SdrPage* GetMasterPage() const { return mpMasterPage; }
        SdrPage* GetPage() const { return mpPage; }
        SdrObject* GetBackgroundObject() const { return mpBackgroundObject; }

        sal_uInt32 GetMPUsageCount() const { return mnMPUsageCount; }
        void SetMPUsageCount(sal_uInt32 nNew) { mnMPUsageCount = nNew; }
    };

    class MasterPageBuffer : public Timer
    {
        ::std::vector< MasterPageBufferEntry >          maEntries;

        bool DecrementUsageCounts();
        void ClearUnusedBufferData();

    public:
        MasterPageBuffer();
        ~MasterPageBuffer();

        // The timer when it is triggered; from class Timer
        virtual void Timeout();

        void OfferMasterPageData(const MasterPageBufferEntry& rEntry);
        Bitmap FindCandidate(
            const SdrPage& rMasterPage, const SdrPage& rPage,
            const MapMode& rMapMode, const SdrObject* pBackgroundObect);
        void ForgetMasterPageData(const SdrPage& rMasterPage, const SdrPage& rPage);
    };

    MasterPageBuffer::MasterPageBuffer()
    {
        SetTimeout(MASTERPAGE_TIMEOUT);
        Stop();
    }

    MasterPageBuffer::~MasterPageBuffer()
    {
        Stop();
    }

    void MasterPageBuffer::Timeout()
    {
        if(DecrementUsageCounts())
        {
            ClearUnusedBufferData();
        }

        if(maEntries.size())
        {
            Start();
        }
    }

    bool MasterPageBuffer::DecrementUsageCounts()
    {
        ::std::vector< MasterPageBufferEntry >::iterator aCandidate = maEntries.begin();
        bool bMemberReacedZero(false);

        // set all timeout entries for this page to 0L
        while(aCandidate != maEntries.end())
        {
            if(aCandidate->GetMPUsageCount())
            {
                aCandidate->SetMPUsageCount(aCandidate->GetMPUsageCount() - 1L);

                if(!aCandidate->GetMPUsageCount())
                {
                    bMemberReacedZero = true;
                }
            }

            aCandidate++;
        }

        return bMemberReacedZero;
    }

    void MasterPageBuffer::OfferMasterPageData(const MasterPageBufferEntry& rEntry)
    {
        ::std::vector< MasterPageBufferEntry >::iterator aCandidate = maEntries.begin();

        // search for existing entry for that MasterPage, Page and BackgroundObject
        while(aCandidate != maEntries.end()
            && (aCandidate->GetMasterPage() != rEntry.GetMasterPage()
                || aCandidate->GetPage() != rEntry.GetPage()
                || aCandidate->GetBackgroundObject() != rEntry.GetBackgroundObject()))
        {
            aCandidate++;
        }

        if(aCandidate == maEntries.end())
        {
            // not found, add new combination of MasterPage, Page and BackgroundObject
            maEntries.push_back(rEntry);

            // reduce member count if getting too big
            if(maEntries.size() > MASTERPAGE_MAXIMUM_COUNT)
            {
                ::std::vector< MasterPageBufferEntry >::iterator aCandidate2 = maEntries.begin();
                ::std::vector< MasterPageBufferEntry >::iterator aSmallest = maEntries.begin();

                // search for existing entry with smallest UsageCount
                while(aCandidate2 != maEntries.end())
                {
                    if(aCandidate2->GetMPUsageCount() < aSmallest->GetMPUsageCount())
                    {
                        aSmallest = aCandidate2;
                    }

                    aCandidate2++;
                }

                if(aSmallest != maEntries.end())
                {
                    aSmallest->SetMPUsageCount(0L);
                    ClearUnusedBufferData();
                }
            }

            // start the timer
            if(maEntries.size())
            {
                Start();
            }
        }
        else
        {
            // found, replace in vector
            *aCandidate = rEntry;
        }
    }

    Bitmap MasterPageBuffer::FindCandidate(
        const SdrPage& rMasterPage, const SdrPage& rPage,
        const MapMode& rMapMode, const SdrObject* pBackgroundObect)
    {
        ::std::vector< MasterPageBufferEntry >::iterator aCandidate = maEntries.begin();

        // search for existing entry for that MasterPage
        while(aCandidate != maEntries.end()
            && (aCandidate->GetMasterPage() != &rMasterPage
                || aCandidate->GetPage() != &rPage
                || aCandidate->GetMapMode() != rMapMode
                || aCandidate->GetBackgroundObject() != pBackgroundObect))
        {
            aCandidate++;
        }

        if(aCandidate != maEntries.end())
        {
            // found
            aCandidate->SetMPUsageCount(MASTERPAGE_USAGE_COUNT);
            return Bitmap(aCandidate->GetBitmap());
        }

        return Bitmap();
    }

    void MasterPageBuffer::ForgetMasterPageData(const SdrPage& rMasterPage, const SdrPage& rPage)
    {
        ::std::vector< MasterPageBufferEntry >::iterator aCandidate = maEntries.begin();
        bool bNeedsClear(false);

        // set all timeout entries for this page to 0L
        while(aCandidate != maEntries.end())
        {
            if(aCandidate->GetMasterPage() == &rMasterPage
                || aCandidate->GetPage() == &rPage)
            {
                aCandidate->SetMPUsageCount(0L);
                bNeedsClear = true;
            }

            aCandidate++;
        }

        if(bNeedsClear)
        {
            ClearUnusedBufferData();

            if(!maEntries.size())
            {
                Stop();
            }
        }
    }

    void MasterPageBuffer::ClearUnusedBufferData()
    {
        ::std::vector< MasterPageBufferEntry > maFilteredEntries;
        ::std::vector< MasterPageBufferEntry >::iterator aCandidate = maEntries.begin();

        // copy entries not for this masterpage to new vector
        while(aCandidate != maEntries.end())
        {
            if(aCandidate->GetMPUsageCount())
            {
                maFilteredEntries.push_back(*aCandidate);
            }

            aCandidate++;
        }

        // copy new vector on old one, this will destroy the old one
        // and the not copied entries
        maEntries = maFilteredEntries;
    }

    // declare the global buffer here
    MasterPageBuffer aMasterPageBuffer;
} // end of anonymous namespace

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
        // Create a Object-Specific ViewObjectContact, set ViewContact and
        // ObjectContact. Always needs to return something. Default is to create
        // a standard ViewObjectContact containing the given ObjectContact and *this
        ViewObjectContact& ViewContactOfMasterPageDescriptor::CreateObjectSpecificViewObjectContact(ObjectContact& rObjectContact)
        {
            ViewObjectContact* pRetval = 0L;

            if(rObjectContact.IsMasterPageBufferingAllowed())
            {
                // buffered, but no alpha-channel
//              pRetval = new VOCBitmapBuffer_old(rObjectContact, *this, false);
//              pRetval = new VOCBitmapBuffer(rObjectContact, *this);
                pRetval = new VOCOfMasterPageDescriptor(rObjectContact, *this);
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
            const ViewObjectContact& /*rAssociatedVOC*/)
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
            SdrObject* pCandidate = GetMasterPageDescriptor().GetBackgroundObject();
            if(pCandidate)
            {
                // #i42075# Test layer visibility. PageBackgroundObject uses the background layer.
                if(GetMasterPageDescriptor().GetVisibleLayers().IsSet(pCandidate->GetLayer()))
                {
                    bRetval = PaintBackgroundObject(*this, *pCandidate, rDisplayInfo, rPaintRectangle, rAssociatedVOC);
                }
            }

            // Draw the MasterPage content
            Rectangle aSecondRectangle;

            // #i31397#
            // In OwnMasterPagePainter, use the same ViewObjectContactRedirector as
            // in the view we are painted in
            ViewObjectContactRedirector* pParentRedirector = rAssociatedVOC.GetObjectContact().GetViewObjectContactRedirector();
            ViewObjectContactRedirector* pLocalRedirector = mpMasterPagePainter->GetViewObjectContactRedirector();

            if(pParentRedirector)
            {
                // set ViewObjectContactRedirector of view we are painted in
                mpMasterPagePainter->SetViewObjectContactRedirector(pParentRedirector);
            }

            if(mpMasterPagePainter->PaintIt(rDisplayInfo, aSecondRectangle))
            {
                bRetval = sal_True;
                rPaintRectangle.Union(aSecondRectangle);

                // #i37869# Paint if no buffering to not paint this infos into the evtl.
                // created buffer. When buffered, it will be painted over the buffer
                // output from VOCOfMasterPageDescriptor::PaintObject(...)
                if(!rAssociatedVOC.GetObjectContact().IsMasterPageBufferingAllowed())
                {
                    PaintBackgroundPageBordersAndGrids(rDisplayInfo, rAssociatedVOC);
                }
            }

            if(pParentRedirector)
            {
                // reset ViewObjectContactRedirector to local one
                mpMasterPagePainter->SetViewObjectContactRedirector(pLocalRedirector);
            }

            return bRetval;
        }

        ViewContact* ViewContactOfMasterPageDescriptor::GetParentContact() const
        {
            return &(GetMasterPageDescriptor().GetUsedPage().GetViewContact());
        }

        // React on changes of the object of this ViewContact
        void ViewContactOfMasterPageDescriptor::ActionChanged()
        {
            // get rid of all MasterPage buffer entries concerning this page
            SdrPage& rMasterPage = GetMasterPageDescriptor().GetUsedPage();
            SdrPage& rPage = GetMasterPageDescriptor().GetOwnerPage();
            aMasterPageBuffer.ForgetMasterPageData(rMasterPage, rPage);

            // call parent
            ViewContact::ActionChanged();
        }

        // Interface method for receiving buffered MasterPage render data from
        // VOCOfMasterPageDescriptor. Called from instances of VOCOfMasterPageDescriptor.
        void ViewContactOfMasterPageDescriptor::OfferBufferedData(const Bitmap& rBitmap, const MapMode& rMapMode)
        {
            // set global data
            SdrPage& rMasterPage = GetMasterPageDescriptor().GetUsedPage();
            SdrPage& rPage = GetMasterPageDescriptor().GetOwnerPage();
            SdrObject* pBackgroundObject = GetMasterPageDescriptor().GetBackgroundObject();
            MasterPageBufferEntry aData(rBitmap, rMapMode, &rMasterPage, &rPage, pBackgroundObject);
            aMasterPageBuffer.OfferMasterPageData(aData);
        }

        // Interface method for VOCOfMasterPageDescriptor to ask for buffered data. If
        // the page is the sane and the MapMode is the same, return the Bitmap.
        Bitmap ViewContactOfMasterPageDescriptor::RequestBufferedData(const MapMode& rMapMode)
        {
            // request global data
            Bitmap aRetval;
            SdrPage& rMasterPage = GetMasterPageDescriptor().GetUsedPage();
            SdrPage& rPage = GetMasterPageDescriptor().GetOwnerPage();
            SdrObject* pBackgroundObject = GetMasterPageDescriptor().GetBackgroundObject();
            aRetval = aMasterPageBuffer.FindCandidate(rMasterPage, rPage, rMapMode, pBackgroundObject);

            return aRetval;
        }

        // #i37869# Support method to paint borders and grids which are overpainted from
        // this MasterPage content to let the MasterPage appear as page background
        void ViewContactOfMasterPageDescriptor::PaintBackgroundPageBordersAndGrids(DisplayInfo& rDisplayInfo, const ViewObjectContact& rAssociatedVOC)
        {
            const SdrPageView* pPageView = rDisplayInfo.GetPageView();
            if(pPageView)
            {
                const SdrView& rView = pPageView->GetView();
                const SdrPage& rOwnerPage= GetMasterPageDescriptor().GetOwnerPage();

                if(rView.IsPageVisible() && rView.IsPageBorderVisible())
                {
                    ViewContactOfSdrPage::DrawPaperBorder(rDisplayInfo, rOwnerPage);
                }

                if(rView.IsBordVisible())
                {
                    ViewContactOfSdrPage::DrawBorder(rView.IsBordVisibleOnlyLeftRight(),rDisplayInfo, rOwnerPage);
                }

                // #i71130# find out if OC is preview renderer
                const bool bPreviewRenderer(rAssociatedVOC.GetObjectContact().IsPreviewRenderer());

                // #i71130# no grid and no helplines for page previews
                if(!bPreviewRenderer)
                {
                    if(rView.IsGridVisible() && !rView.IsGridFront())
                    {
                        ViewContactOfSdrPage::DrawGrid(rDisplayInfo);
                    }

                    if(rView.IsHlplVisible() && !rView.IsHlplFront())
                    {
                        ViewContactOfSdrPage::DrawHelplines(rDisplayInfo);
                    }
                }
            }
        }
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////
// eof
