/*************************************************************************
 *
 *  $RCSfile: displayinfo.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-12 14:29:36 $
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

#ifndef _SDR_CONTACT_DISPLAYINFO_HXX
#define _SDR_CONTACT_DISPLAYINFO_HXX

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#ifndef _SVDSOB_HXX
#include <svdsob.hxx>
#endif

#ifndef INCLUDED_SVTOOLS_COLORCFG_HXX
#include <svtools/colorcfg.hxx>
#endif

#ifndef _SV_REGION_HXX
#include <vcl/region.hxx>
#endif

#include <vector>

//////////////////////////////////////////////////////////////////////////////
// predeclarations

class SdrPageView;
class ExtOutputDevice;
class SdrPaintInfoRec;
class OutputDevice;
class SdrPage;

namespace sdr
{
    namespace contact
    {
        class DisplayInfo;

        // typedef for a vector of DisplayInfos
        typedef ::std::vector< DisplayInfo* > DisplayInfoVector;

        class ViewObjectContact;
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
        class DisplayInfo
        {
        protected:
            // the owner of this DisplayInfo. Set from constructor and not
            // to be changed in any way. May be empty, too.
            SdrPageView*                                    mpPageView;

            // For being able to detect the processed page, allow setting
            // it at DisplayInfo. To allow access from old stuff, register
            // the DisplayInfo at given SdrPageView, too. Init value is 0L.
            SdrPage*                                        mpProcessedPage;

            // To temporarily remember internally the current DispolayInfo
            // which is set at the PaintView. This will be removes ASA CL
            // will use the DrawXXX overlay mechanism (not yet available)
            const DisplayInfo*                              mpLastDisplayInfo;

            // The Layers which shall be processed (visible)
            SetOfByte                                       maProcessLayers;

            // The OutputDevice to work on
            OutputDevice*                                   mpOutputDevice;

            // The redraw area, in logical coordinates of OutputDevice. If Region
            // is empty, everything needs to be redrawn
            Region                                          maRedrawArea;

            // a svtools::ColorConfig entry to have access to all current set
            // colors
            svtools::ColorConfig                            maColorConfig;

            // old stuff, for first compatible tests
            ExtOutputDevice*                                mpExtOutputDevice;
            SdrPaintInfoRec*                                mpPaintInfoRec;

            // The root ViewObjectContact of the current visualisation. Set from
            // ProccessDisplay()
            ViewObjectContact*                              mpRootVOC;

            // remember the original drawmode for entered group visualisation
            sal_uInt32                                      mnOriginalDrawMode;

            // bitfield
            // Internal flag to know when the control layer is painted. Default is
            // sal_False. If set to sal_True, painting of the page, page borders and
            // the rasters will be suppressed. This flag is set
            // internally from the DoProcessDisplay mechanism to avoid double page
            // painting when the control layer needs to be painted as last layer. For
            // generally switching page painting on and off, use the PagePainting flag
            // or the according flags at the view (->Is*Visible()).
            unsigned                                        mbControlLayerPainting : 1;

            // Flag to decide if page will be painted at all. This flag is user-defined
            // and will not be changed from the DoProcessDisplay mechanism. Default is
            // sal_True, thus set to sal_False if PagePainting should be suppressed.
            unsigned                                        mbPagePainting : 1;

            // Flag to remember if EnteredGroupDrawMode is active
            unsigned                                        mbGhostedDrawModeActive : 1;

            // Flag to have more control over object output buffering. This
            // is sal_True by default, but maybe set to sal_False from MasterPage Paints
            // if MasterPage is painted in DrawPage mode.
            unsigned                                        mbBufferingAllowed : 1;

            // Flag if Paint shall be continued. This is initialized with sal_true
            // and used from DoContinuePaint() together with CheckContinuePaint()
            // which may use Application::AnyInput() to interrupt painting.
            unsigned                                        mbContinuePaint : 1;

            // Flag to know if a MasterPage is painted in MasterPage mode.
            // Initialized to sal_False, this shopuld only be changed from
            // the instance which is painting the MasterPage and knows about.
            unsigned                                        mbMasterPagePainting : 1;

            // Flag to set PreRendering to allowed/forbidden. Default is forbidden.
            unsigned                                        mbPreRenderingAllowed : 1;

            // This uses Application::AnyInput() and may change mbContinuePaint
            // to interrupt the paint
            void CheckContinuePaint();

        public:
            // access to SdrPageView
            SdrPageView* GetPageView() const
            {
                return mpPageView;
            }

            // basic constructor.
            DisplayInfo(SdrPageView* pView = 0L);

            // destructor
            virtual ~DisplayInfo();

            // access to ProcessedPage, write is for internal use only.
            // read is used from SdrPageView eventually, to identify the
            // currently painting page. This is needed for things like
            // PageNumber fields (outliner callback). For that purpose all page painting parts
            // should use SetProcessedPage to set the currently rendered page,
            // and also to reset that pointer again.
            void SetProcessedPage(SdrPage* pNew);
            const SdrPage* GetProcessedPage() const;

            // access to ProcessLayers
            void SetProcessLayers(const SetOfByte& rSet);
            const SetOfByte& GetProcessLayers() const;

            // access to ExtendedOutputDevice
            void SetExtendedOutputDevice(ExtOutputDevice* pExtOut);
            ExtOutputDevice* GetExtendedOutputDevice() const;

            // access to PaintInfoRec
            void SetPaintInfoRec(SdrPaintInfoRec* pInfoRec);
            SdrPaintInfoRec* GetPaintInfoRec() const;

            // access to OutputDevice
            void SetOutputDevice(OutputDevice* pOutDev);
            OutputDevice* GetOutputDevice() const;

            // access to RedrawArea
            void SetRedrawArea(const Region& rRegion);
            const Region& GetRedrawArea() const;

            // Is OutDev a printer?
            sal_Bool OutputToPrinter() const;

            // Is OutDev a window?
            sal_Bool OutputToWindow() const;

            // Is OutDev a VirtualDevice?
            sal_Bool OutputToVirtualDevice() const;

            // Is OutDev a recording MetaFile?
            sal_Bool OutputToRecordingMetaFile() const;

            // Access to ControlLayerPainting flag
            void SetControlLayerPainting(sal_Bool bDoPaint);
            sal_Bool GetControlLayerPainting() const;

            // Access to PagePainting flag
            void SetPagePainting(sal_Bool bDoPaint);
            sal_Bool GetPagePainting() const;

            // Access to svtools::ColorConfig
            const svtools::ColorConfig& GetColorConfig() const;

            // Save the original DrawMode from outdev
            void SaveOriginalDrawMode();
            sal_uInt32 GetOriginalDrawMode() const;
            sal_uInt32 GetCurrentDrawMode() const;
            void RestoreOriginalDrawMode();
            void SetGhostedDrawMode();
            sal_Bool IsGhostedDrawModeActive() const;

            // access to buffering allowed flag
            void SetBufferingAllowed(sal_Bool bNew);
            sal_Bool IsBufferingAllowed() const;

            // Check if painting should be continued. If not, return from paint
            // as soon as possible.
            sal_Bool DoContinuePaint();

            // access to master page painting flag
            sal_Bool GetMasterPagePainting() const;
            void SetMasterPagePainting(sal_Bool bNew);

            // access to PreRendering flag
            sal_Bool IsPreRenderingAllowed() const;
            void SetPreRenderingAllowed(sal_Bool bNew);

            // Infos about draft painting. These may get bitfield members later.
            sal_Bool IsDraftText() const;
            sal_Bool IsDraftGraphic() const;
            sal_Bool IsDraftLine() const;
            sal_Bool IsDraftFill() const;
            sal_Bool IsHideDraftGraphic() const;
        };
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

#endif //_SDR_CONTACT_DISPLAYINFO_HXX

// eof
