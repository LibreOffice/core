/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <config_feature_desktop.h>

#include <svx/sdr/contact/objectcontactofpageview.hxx>
#include <sdr/contact/viewobjectcontactofunocontrol.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svdpage.hxx>
#include <svx/sdr/contact/displayinfo.hxx>
#include <svx/sdr/contact/viewobjectcontact.hxx>
#include <svx/svdview.hxx>
#include <svx/sdr/contact/viewcontact.hxx>
#include <svx/sdr/animation/objectanimator.hxx>
#include <svx/sdrpagewindow.hxx>
#include <svx/sdrpaintwindow.hxx>
#include <svtools/colorcfg.hxx>
#include <sfx2/viewsh.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <drawinglayer/processor2d/baseprocessor2d.hxx>
#include <drawinglayer/processor2d/processor2dtools.hxx>
#include <osl/diagnose.h>
#include <officecfg/Office/Common.hxx>
#include <svx/unoapi.hxx>
#include <comphelper/configuration.hxx>
#include <vcl/canvastools.hxx>
#include <vcl/pdfextoutdevdata.hxx>
#include <comphelper/lok.hxx>

#include <memory>

using namespace com::sun::star;

namespace sdr::contact
{
        // internal access to SdrPage of SdrPageView
        SdrPage* ObjectContactOfPageView::GetSdrPage() const
        {
            return GetPageWindow().GetPageView().GetPage();
        }

        ObjectContactOfPageView::ObjectContactOfPageView(
                SdrPageWindow& rPageWindow, const char *pDebugName)
            : Idle(pDebugName)
            , mrPageWindow(rPageWindow)
        {
            // init PreviewRenderer flag
            setPreviewRenderer(static_cast<SdrPaintView&>(rPageWindow.GetPageView().GetView()).IsPreviewRenderer());

            // init timer
            SetPriority(TaskPriority::HIGH_IDLE);
            Stop();
        }

        ObjectContactOfPageView::~ObjectContactOfPageView()
        {
            // stop timer
            Stop();
        }

        // LazyInvalidate request. Take action.
        void ObjectContactOfPageView::setLazyInvalidate(ViewObjectContact& /*rVOC*/)
        {
            // do NOT call parent, but remember that something is to do by
            // starting the LazyInvalidateTimer
            Start();
        }

        // call this to support evtl. preparations for repaint
        void ObjectContactOfPageView::PrepareProcessDisplay()
        {
            if(IsActive())
                // there are still non-triggered LazyInvalidate events, trigger these
                Invoke();
        }

        // From baseclass Timer, the timeout call triggered by the LazyInvalidate mechanism
        void ObjectContactOfPageView::Invoke()
        {
            // stop the timer
            Stop();

            // invalidate all LazyInvalidate VOCs new situations
            const sal_uInt32 nVOCCount(getViewObjectContactCount());

            for(sal_uInt32 a(0); a < nVOCCount; a++)
            {
                ViewObjectContact* pCandidate = getViewObjectContact(a);
                pCandidate->triggerLazyInvalidate();
            }
        }

        // Process the whole displaying
        void ObjectContactOfPageView::ProcessDisplay(DisplayInfo& rDisplayInfo)
        {
            const SdrPage* pStartPage = GetSdrPage();

            if(pStartPage && !rDisplayInfo.GetProcessLayers().IsEmpty())
            {
                const ViewContact& rDrawPageVC = pStartPage->GetViewContact();

                if(rDrawPageVC.GetObjectCount())
                {
                    DoProcessDisplay(rDisplayInfo);
                }
            }
        }

        // Process the whole displaying. Only use given DisplayInfo, do not access other
        // OutputDevices then the given ones.
        void ObjectContactOfPageView::DoProcessDisplay(DisplayInfo& rDisplayInfo)
        {
            OutputDevice& rTargetOutDev = GetPageWindow().GetPaintWindow().GetTargetOutputDevice();
            const Size aOutputSizePixel(rTargetOutDev.GetOutputSizePixel());
            if (!isOutputToRecordingMetaFile() // do those have outdev too?
                && (0 == aOutputSizePixel.getWidth() ||
                    0 == aOutputSizePixel.getHeight()))
            {
                return;
            }

            // visualize entered group when that feature is switched on and it's not
            // a print output. #i29129# No ghosted display for printing.
            bool bVisualizeEnteredGroup(DoVisualizeEnteredGroup() && !isOutputToPrinter());

            // Visualize entered groups: Set to ghosted as default
            // start. Do this only for the DrawPage, not for MasterPages
            if(bVisualizeEnteredGroup)
            {
                rDisplayInfo.SetGhostedDrawMode();
            }

            // #114359# save old and set clip region
            OutputDevice* pOutDev = TryToGetOutputDevice();
            OSL_ENSURE(nullptr != pOutDev, "ObjectContactOfPageView without OutDev, someone has overridden TryToGetOutputDevice wrong (!)");
            bool bClipRegionPushed(false);
            const vcl::Region& rRedrawArea(rDisplayInfo.GetRedrawArea());

            // tdf#153102 using the given RedrawArea is needed e.g. for Writer's
            // visual clipping against PageBounds (also for android viewer)
            if(!rRedrawArea.IsEmpty())
            {
                bClipRegionPushed = true;
                pOutDev->Push(vcl::PushFlags::CLIPREGION);
                pOutDev->IntersectClipRegion(rRedrawArea);
            }

            // Get start node and process DrawPage contents
            const ViewObjectContact& rDrawPageVOContact = GetSdrPage()->GetViewContact().GetViewObjectContact(*this);

            // update current ViewInformation2D at the ObjectContact
            const double fCurrentTime(getPrimitiveAnimator().GetTime());
            basegfx::B2DRange aViewRange;

            // create ViewRange
            if(isOutputToRecordingMetaFile())
            {
                if (!rDisplayInfo.GetRedrawArea().IsEmpty())
                {
                    // #i98402# if it's a PDF export, set the ClipRegion as ViewRange. This is
                    // mainly because SW does not use DrawingLayer Page-Oriented and if not doing this,
                    // all existing objects will be collected as primitives and processed.
                    // OD 2009-03-05 #i99876# perform the same also for SW on printing.
                    // fdo#78149 same thing also needed for plain MetaFile
                    //           export, so why not do it always
                    const tools::Rectangle aLogicClipRectangle(rDisplayInfo.GetRedrawArea().GetBoundRect());

                    aViewRange = vcl::unotools::b2DRectangleFromRectangle(aLogicClipRectangle);
                }
            }
            else
            {
                // use visible pixels, but transform to world coordinates
                aViewRange = basegfx::B2DRange(0.0, 0.0, aOutputSizePixel.getWidth(), aOutputSizePixel.getHeight());
                // if a clip region is set, use it
                if(!rDisplayInfo.GetRedrawArea().IsEmpty())
                {
                    // get logic clip range and create discrete one
                    const tools::Rectangle aLogicClipRectangle(rDisplayInfo.GetRedrawArea().GetBoundRect());
                    basegfx::B2DRange aDiscreteClipRange = vcl::unotools::b2DRectangleFromRectangle(aLogicClipRectangle);
                    aDiscreteClipRange.transform(rTargetOutDev.GetViewTransformation());

                    // align the discrete one to discrete boundaries (pixel bounds). Also
                    // expand X and Y max by one due to Rectangle definition source
                    aDiscreteClipRange.expand(basegfx::B2DTuple(
                        floor(aDiscreteClipRange.getMinX()),
                        floor(aDiscreteClipRange.getMinY())));
                    aDiscreteClipRange.expand(basegfx::B2DTuple(
                        1.0 + ceil(aDiscreteClipRange.getMaxX()),
                        1.0 + ceil(aDiscreteClipRange.getMaxY())));

                    // intersect current ViewRange with ClipRange
                    aViewRange.intersect(aDiscreteClipRange);
                }

                // transform to world coordinates
                aViewRange.transform(rTargetOutDev.GetInverseViewTransformation());
            }

            // update local ViewInformation2D
            drawinglayer::geometry::ViewInformation2D aNewViewInformation2D;
            aNewViewInformation2D.setViewTransformation(rTargetOutDev.GetViewTransformation());
            aNewViewInformation2D.setViewport(aViewRange);
            aNewViewInformation2D.setVisualizedPage(GetXDrawPageForSdrPage(GetSdrPage()));
            aNewViewInformation2D.setViewTime(fCurrentTime);
            if (const SfxViewShell* pViewShell = SfxViewShell::Current())
                aNewViewInformation2D.setAutoColor(pViewShell->GetColorConfigColor(svtools::DOCCOLOR));
            if (static_cast<SdrPaintView&>(mrPageWindow.GetPageView().GetView()).IsTextEdit())
                aNewViewInformation2D.setTextEditActive(true);

            if (!isOutputToRecordingMetaFile())
            {
                // this is the EditView repaint, provide that information,
                // but only if we do not export to metafile
                aNewViewInformation2D.setEditViewActive(true);

                // also copy the current DrawModeFlags
                aNewViewInformation2D.setDrawModeFlags(rTargetOutDev.GetDrawMode());
            }

            setViewInformation2D2D(aNewViewInformation2D);

            drawinglayer::primitive2d::Primitive2DContainer xPrimitiveSequence;

#if HAVE_FEATURE_DESKTOP || defined( ANDROID )
            // get whole Primitive2DContainer; this will already make use of updated ViewInformation2D
            // and may use the MapMode from the Target OutDev in the DisplayInfo
            rDrawPageVOContact.getPrimitive2DSequenceHierarchy(rDisplayInfo, xPrimitiveSequence);
#else
            // Hmm, !HAVE_FEATURE_DESKTOP && !ANDROID means iOS,
            // right? But does it makes sense to use a different code
            // path for iOS than for Android; both use tiled rendering
            // etc now.

            // HACK: this only works when we are drawing sdr shapes via
            // drawinglayer; but it can happen that the hierarchy contains
            // more than just the shapes, and then it fails.
            //
            // This is good enough for the tiled rendering for the moment, but
            // we need to come up with the real solution shortly.

            // Only get the expensive hierarchy if we can be sure that the
            // returned sequence won't be empty anyway.
            bool bGetHierarchy = rRedrawArea.IsEmpty();
            if (!bGetHierarchy)
            {
                // Not empty? Then not doing a full redraw, check if
                // getPrimitive2DSequenceHierarchy() is still needed.
                for (const rtl::Reference<SdrObject>& pObject : *GetSdrPage())
                {
                    if (rRedrawArea.Overlaps(pObject->GetCurrentBoundRect()))
                    {
                        bGetHierarchy = true;
                        break;
                    }
                }
            }

            if (bGetHierarchy)
                // get whole Primitive2DContainer; this will already make use of updated ViewInformation2D
                // and may use the MapMode from the Target OutDev in the DisplayInfo
                rDrawPageVOContact.getPrimitive2DSequenceHierarchy(rDisplayInfo, xPrimitiveSequence);
#endif

            // if there is something to show, use a primitive processor to render it. There
            // is a choice between VCL and Canvas processors currently. The decision is made in
            // createProcessor2DFromOutputDevice and takes into account things like the
            // Target is a MetaFile, a VDev or something else. The Canvas renderer is triggered
            // currently using the shown boolean. Canvas is not yet the default.
            if(!xPrimitiveSequence.empty())
            {
                // prepare OutputDevice (historical stuff, maybe soon removed)
                rDisplayInfo.ClearGhostedDrawMode(); // reset, else the VCL-paint with the processor will not do the right thing
                pOutDev->SetLayoutMode(vcl::text::ComplexTextLayoutFlags::Default); // reset, default is no BiDi/RTL
                // create renderer
                std::unique_ptr<drawinglayer::processor2d::BaseProcessor2D> pProcessor2D(
                    drawinglayer::processor2d::createProcessor2DFromOutputDevice(
                        rTargetOutDev, getViewInformation2D()));
                pProcessor2D->process(xPrimitiveSequence);
            }

            // #114359# restore old ClipReghion
            if(bClipRegionPushed)
            {
                pOutDev->Pop();
            }

            // Visualize entered groups: Reset to original DrawMode
            if(bVisualizeEnteredGroup)
            {
                rDisplayInfo.ClearGhostedDrawMode();
            }
        }

        // test if visualizing of entered groups is switched on at all
        bool ObjectContactOfPageView::DoVisualizeEnteredGroup() const
        {
            return true;
        }

        // get active group's (the entered group) ViewContact
        const ViewContact* ObjectContactOfPageView::getActiveViewContact() const
        {
            SdrObjList* pActiveGroupList = GetPageWindow().GetPageView().GetObjList();

            if(pActiveGroupList)
            {
                // tdf#122735
                // Here it is necessary to check for SdrObject 1st, that may
                // return nullptr if it is not a SdrObject/SdrObjGroup.
                // Checking for SrPage OTOH will *always* try to return
                // something useful due to SdrObjGroup::getSdrPageFromSdrObjList
                // using getSdrPageFromSdrObject which will recursively go up the
                // hierarchy to get the SdrPage the SdrObject belongs to, so
                // this will *not* be nullptr for e.g. a SdrObjGroup if the
                // SdrObjGroup is inserted to a SdrPage.
                // NOTE: It is also possible to use dynamic_cast<SdrObjGroup*>
                //       here, but getSdrObjectFromSdrObjList and
                //       getSdrPageFromSdrObjListexist  to not need to do that
                SdrObject* pSdrObject(pActiveGroupList->getSdrObjectFromSdrObjList());

                if(nullptr != pSdrObject)
                {
                    // It is a group object
                    return &(pSdrObject->GetViewContact());
                }
                else
                {
                    SdrPage* pSdrPage(pActiveGroupList->getSdrPageFromSdrObjList());

                    if(nullptr != pSdrPage)
                    {
                        // It's a Page itself
                        return &(pSdrPage->GetViewContact());
                    }
                }
            }
            else if(GetSdrPage())
            {
                // use page of associated SdrPageView
                return &(GetSdrPage()->GetViewContact());
            }

            return nullptr;
        }

        // Invalidate given rectangle at the window/output which is represented by
        // this ObjectContact.
        void ObjectContactOfPageView::InvalidatePartOfView(const basegfx::B2DRange& rRange) const
        {
            // invalidate at associated PageWindow
            GetPageWindow().InvalidatePageWindow(rRange);
        }

        // Get info about the need to visualize GluePoints
        bool ObjectContactOfPageView::AreGluePointsVisible() const
        {
            bool bTiledRendering = comphelper::LibreOfficeKit::isActive();
            return !bTiledRendering && GetPageWindow().GetPageView().GetView().ImpIsGlueVisible();
        }

        // check if text animation is allowed.
        bool ObjectContactOfPageView::IsTextAnimationAllowed() const
        {
            if (comphelper::IsFuzzing())
                return true;
            // tdf#161765: Let the user choose which animation settings to use: OS's / LO's
            // New options: "System"/"No"/"Yes".
            // Do respect OS's animation setting if the user has selected the option "System"
            return MiscSettings::IsAnimatedTextAllowed();
        }

        // check if graphic animation is allowed.
        bool ObjectContactOfPageView::IsGraphicAnimationAllowed() const
        {
            if (comphelper::IsFuzzing())
                return true;
            // tdf#161765: Let the user choose which animation settings to use: OS's / LO's and
            // don't override here LO's animation settings with OS's all-or-nothing animation setting,
            // but do respect OS's animation setting if the user has selected the option "System".
            // New options: "System"/"No"/"Yes"
            return MiscSettings::IsAnimatedGraphicAllowed();
        }

        // print?
        bool ObjectContactOfPageView::isOutputToPrinter() const
        {
            return (OUTDEV_PRINTER == mrPageWindow.GetPaintWindow().GetOutputDevice().GetOutDevType());
        }

        // display page decoration? Default is true
        bool ObjectContactOfPageView::isPageDecorationActive() const
        {
            return GetPageWindow().GetPageView().GetView().IsPageDecorationAllowed();
        }

        // display mster page content (ViewContactOfMasterPage)? Default is true
        bool ObjectContactOfPageView::isMasterPageActive() const
        {
            return GetPageWindow().GetPageView().GetView().IsMasterPageVisualizationAllowed();
        }

        // recording MetaFile?
        bool ObjectContactOfPageView::isOutputToRecordingMetaFile() const
        {
            GDIMetaFile* pMetaFile = mrPageWindow.GetPaintWindow().GetOutputDevice().GetConnectMetaFile();
            return (pMetaFile && pMetaFile->IsRecord() && !pMetaFile->IsPause());
        }

        // pdf export?
        bool ObjectContactOfPageView::isOutputToPDFFile() const
        {
            return OUTDEV_PDF == mrPageWindow.GetPaintWindow().GetOutputDevice().GetOutDevType();
        }

        bool ObjectContactOfPageView::isExportTaggedPDF() const
        {
            if (isOutputToPDFFile())
            {
                vcl::PDFExtOutDevData* pPDFExtOutDevData(dynamic_cast<vcl::PDFExtOutDevData*>(
                    mrPageWindow.GetPaintWindow().GetOutputDevice().GetExtOutDevData()));

                if (nullptr != pPDFExtOutDevData)
                {
                    return pPDFExtOutDevData->GetIsExportTaggedPDF();
                }
            }
            return false;
        }

        ::vcl::PDFExtOutDevData const* ObjectContactOfPageView::GetPDFExtOutDevData() const
        {
            if (!isOutputToPDFFile())
            {
                return nullptr;
            }
            vcl::PDFExtOutDevData* pPDFExtOutDevData(dynamic_cast<vcl::PDFExtOutDevData*>(
                mrPageWindow.GetPaintWindow().GetOutputDevice().GetExtOutDevData()));
            return pPDFExtOutDevData;
        }

        // gray display mode
        bool ObjectContactOfPageView::isDrawModeGray() const
        {
            const DrawModeFlags nDrawMode(mrPageWindow.GetPaintWindow().GetOutputDevice().GetDrawMode());
            return (nDrawMode == (DrawModeFlags::GrayLine|DrawModeFlags::GrayFill|DrawModeFlags::BlackText|DrawModeFlags::GrayBitmap|DrawModeFlags::GrayGradient));
        }

        // high contrast display mode
        bool ObjectContactOfPageView::isDrawModeHighContrast() const
        {
            const DrawModeFlags nDrawMode(mrPageWindow.GetPaintWindow().GetOutputDevice().GetDrawMode());
            return (nDrawMode == (DrawModeFlags::SettingsLine|DrawModeFlags::SettingsFill|DrawModeFlags::SettingsText|DrawModeFlags::SettingsGradient));
        }

        // access to SdrPageView
        SdrPageView* ObjectContactOfPageView::TryToGetSdrPageView() const
        {
            return &(mrPageWindow.GetPageView());
        }


        // access to OutputDevice
        OutputDevice* ObjectContactOfPageView::TryToGetOutputDevice() const
        {
            SdrPreRenderDevice* pPreRenderDevice = mrPageWindow.GetPaintWindow().GetPreRenderDevice();

            if(pPreRenderDevice)
            {
                return &(pPreRenderDevice->GetPreRenderDevice());
            }
            else
            {
                return &(mrPageWindow.GetPaintWindow().GetOutputDevice());
            }
        }

        // set all UNO controls displayed in the view to design/alive mode
        void ObjectContactOfPageView::SetUNOControlsDesignMode( bool _bDesignMode ) const
        {
            const sal_uInt32 nCount(getViewObjectContactCount());

            for(sal_uInt32 a(0); a < nCount; a++)
            {
                const ViewObjectContact* pVOC = getViewObjectContact(a);
                const ViewObjectContactOfUnoControl* pUnoObjectVOC = dynamic_cast< const ViewObjectContactOfUnoControl* >(pVOC);

                if(pUnoObjectVOC)
                {
                    pUnoObjectVOC->setControlDesignMode(_bDesignMode);
                }
            }
        }

} // end of namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
