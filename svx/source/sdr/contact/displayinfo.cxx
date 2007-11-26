/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: displayinfo.cxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-26 14:51:31 $
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

#ifndef _SDR_CONTACT_DISPLAYINFO_HXX
#include <svx/sdr/contact/displayinfo.hxx>
#endif

#ifndef _SV_OUTDEV_HXX
#include <vcl/outdev.hxx>
#endif

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

#ifndef _SVDOBJ_HXX
#include <svx/svdobj.hxx>
#endif

#ifndef _SV_GDIMTF_HXX
#include <vcl/gdimtf.hxx>
#endif

#ifndef _SVDPAGV_HXX
#include <svx/svdpagv.hxx>
#endif

#define ALL_GHOSTED_DRAWMODES (DRAWMODE_GHOSTEDLINE|DRAWMODE_GHOSTEDFILL|DRAWMODE_GHOSTEDTEXT|DRAWMODE_GHOSTEDBITMAP|DRAWMODE_GHOSTEDGRADIENT)

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
        // This uses Application::AnyInput() and may change mbContinuePaint
        // to interrupt the paint
        void DisplayInfo::CheckContinuePaint()
        {
            // #111111#
            // INPUT_PAINT and INPUT_TIMER removed again since this leads to
            // problems under Linux and Solaris when painting slow objects
            // (e.g. bitmaps)

            // #114335#
            // INPUT_OTHER removed too, leads to problems with added controls
            // from the form layer.
#ifndef SOLARIS
            if(Application::AnyInput(INPUT_KEYBOARD))
            {
                mbContinuePaint = sal_False;
            }
#endif
        }

        DisplayInfo::DisplayInfo(SdrPageView* pPageView)
        :   mpPageView(pPageView),
            mpProcessedPage(0L),
            mpLastDisplayInfo(0L),
            maProcessLayers(sal_True), // init layer info with all bits set to draw everything on default
            mpOutputDevice(0L),
            mpExtOutputDevice(0L),
            mpPaintInfoRec(0L),
            mpRootVOC(0L),
            mbControlLayerPainting(sal_False),
            mbPagePainting(sal_True),
            mbGhostedDrawModeActive(sal_False),
            mbBufferingAllowed(sal_True),
            mbContinuePaint(sal_True),
            mbMasterPagePainting(sal_False)
        {
        }

        DisplayInfo::~DisplayInfo()
        {
            SetProcessedPage( 0L );
        }

        // access to ProcessedPage, write for internal use only.
        void DisplayInfo::SetProcessedPage(SdrPage* pNew)
        {
            if(pNew != mpProcessedPage)
            {
                mpProcessedPage = pNew;

                if(mpPageView)
                {
                    if( pNew == NULL )
                    {
                        // DisplayInfo needs to be reset at PageView if set since DisplayInfo is no longer valid
                        if(mpPageView && mpPageView->GetCurrentPaintingDisplayInfo())
                        {
                            DBG_ASSERT( mpPageView->GetCurrentPaintingDisplayInfo() == this, "DisplayInfo::~DisplayInfo() : stack error!" );

                            // restore remembered DisplayInfo to build a stack, or delete
                            mpPageView->SetCurrentPaintingDisplayInfo(mpLastDisplayInfo);
                        }
                    }
                    else
                    {
                        // rescue current
                        mpLastDisplayInfo = mpPageView->GetCurrentPaintingDisplayInfo();

                        // set at PageView when a page is set
                        mpPageView->SetCurrentPaintingDisplayInfo(this);
                    }
                }
            }
        }

        const SdrPage* DisplayInfo::GetProcessedPage() const
        {
            return mpProcessedPage;
        }

        // Access to LayerInfos (which layers to proccess)
        void DisplayInfo::SetProcessLayers(const SetOfByte& rSet)
        {
            maProcessLayers = rSet;
        }

        const SetOfByte& DisplayInfo::GetProcessLayers() const
        {
            return maProcessLayers;
        }

        // access to ExtendedOutputDevice
        void DisplayInfo::SetExtendedOutputDevice(XOutputDevice* pExtOut)
        {
            if(mpExtOutputDevice != pExtOut)
            {
                mpExtOutputDevice = pExtOut;
            }
        }

        XOutputDevice* DisplayInfo::GetExtendedOutputDevice() const
        {
            return mpExtOutputDevice;
        }

        // access to PaintInfoRec
        void DisplayInfo::SetPaintInfoRec(SdrPaintInfoRec* pInfoRec)
        {
            if(mpPaintInfoRec != pInfoRec)
            {
                mpPaintInfoRec = pInfoRec;
            }
        }

        SdrPaintInfoRec* DisplayInfo::GetPaintInfoRec() const
        {
            return mpPaintInfoRec;
        }

        // access to OutputDevice
        void DisplayInfo::SetOutputDevice(OutputDevice* pOutDev)
        {
            if(mpOutputDevice != pOutDev)
            {
                mpOutputDevice = pOutDev;
            }
        }

        OutputDevice* DisplayInfo::GetOutputDevice() const
        {
            return mpOutputDevice;
        }

        // access to RedrawArea
        void DisplayInfo::SetRedrawArea(const Region& rRegion)
        {
            maRedrawArea = rRegion;
        }

        const Region& DisplayInfo::GetRedrawArea() const
        {
            return maRedrawArea;
        }

        // Is OutDev a printer?
        sal_Bool DisplayInfo::OutputToPrinter() const
        {
            if(mpOutputDevice && OUTDEV_PRINTER == mpOutputDevice->GetOutDevType())
            {
                return sal_True;
            }

            return sal_False;
        }

        // Is OutDev a window?
        sal_Bool DisplayInfo::OutputToWindow() const
        {
            if(mpOutputDevice && OUTDEV_WINDOW == mpOutputDevice->GetOutDevType())
            {
                return sal_True;
            }

            return sal_False;
        }

        // Is OutDev a VirtualDevice?
        sal_Bool DisplayInfo::OutputToVirtualDevice() const
        {
            if(mpOutputDevice && OUTDEV_VIRDEV == mpOutputDevice->GetOutDevType())
            {
                return sal_True;
            }

            return sal_False;
        }

        // Is OutDev a recording MetaFile?
        sal_Bool DisplayInfo::OutputToRecordingMetaFile() const
        {
            if(mpOutputDevice)
            {
                GDIMetaFile* pMetaFile = mpOutputDevice->GetConnectMetaFile();

                if(pMetaFile)
                {
                    sal_Bool bRecording = pMetaFile->IsRecord() && !pMetaFile->IsPause();
                    return bRecording;
                }
            }

            return sal_False;
        }

        void DisplayInfo::SetControlLayerPainting(sal_Bool bDoPaint)
        {
            if(mbControlLayerPainting != bDoPaint)
            {
                mbControlLayerPainting = bDoPaint;
            }
        }

        sal_Bool DisplayInfo::GetControlLayerPainting() const
        {
            return mbControlLayerPainting;
        }

        void DisplayInfo::SetPagePainting(sal_Bool bDoPaint)
        {
            if(mbPagePainting != bDoPaint)
            {
                mbPagePainting = bDoPaint;
            }
        }

        sal_Bool DisplayInfo::GetPagePainting() const
        {
            return mbPagePainting;
        }

        // Access to svtools::ColorConfig
        const svtools::ColorConfig& DisplayInfo::GetColorConfig() const
        {
            return maColorConfig;
        }

        sal_uInt32 DisplayInfo::GetOriginalDrawMode() const
        {
            // return DrawMode without ghosted stuff
            if(mpOutputDevice)
            {
                return (mpOutputDevice->GetDrawMode() & ~ALL_GHOSTED_DRAWMODES);
            }

            return 0L;
        }

        sal_uInt32 DisplayInfo::GetCurrentDrawMode() const
        {
            if(mpOutputDevice)
            {
                return mpOutputDevice->GetDrawMode();
            }

            return 0L;
        }

        void DisplayInfo::ClearGhostedDrawMode()
        {
            if(mpOutputDevice)
            {
                mpOutputDevice->SetDrawMode(mpOutputDevice->GetDrawMode() & ~ALL_GHOSTED_DRAWMODES);
            }

            mbGhostedDrawModeActive = sal_False;
        }

        void DisplayInfo::SetGhostedDrawMode()
        {
            if(mpOutputDevice)
            {
                mpOutputDevice->SetDrawMode(mpOutputDevice->GetDrawMode() | ALL_GHOSTED_DRAWMODES);
            }

            mbGhostedDrawModeActive = sal_True;
        }

        sal_Bool DisplayInfo::IsGhostedDrawModeActive() const
        {
            return mbGhostedDrawModeActive;
        }

        // access to buffering allowed flag
        void DisplayInfo::SetBufferingAllowed(sal_Bool bNew)
        {
            if(mbBufferingAllowed != bNew)
            {
                mbBufferingAllowed = bNew;
            }
        }

        sal_Bool DisplayInfo::IsBufferingAllowed() const
        {
            return mbBufferingAllowed;
        }

        // Check if painting should be continued. If not, return from paint
        // as soon as possible.
        sal_Bool DisplayInfo::DoContinuePaint()
        {
            if(mbContinuePaint
                && mpOutputDevice
                && OUTDEV_WINDOW == mpOutputDevice->GetOutDevType())
            {
                CheckContinuePaint();
            }

            return mbContinuePaint;
        }

        sal_Bool DisplayInfo::GetMasterPagePainting() const
        {
            return mbMasterPagePainting;
        }

        void DisplayInfo::SetMasterPagePainting(sal_Bool bNew)
        {
            if(mbMasterPagePainting != bNew)
            {
                mbMasterPagePainting = bNew;
            }
        }
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////
// eof
