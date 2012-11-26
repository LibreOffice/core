/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef _SDRPAINTWINDOW_HXX
#define _SDRPAINTWINDOW_HXX

#ifndef _VIRDEV_HXX //autogen
#include <vcl/virdev.hxx>
#endif
#include "svx/svxdllapi.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
// predeclarations
class SdrPaintView;

namespace sdr
{
    namespace overlay
    {
        class OverlayManager;
    } // end of namespace overlay
} // end of namespace sdr

////////////////////////////////////////////////////////////////////////////////////////////////////

class SdrPreRenderDevice
{
    // The original OutputDevice
    OutputDevice&                                       mrOutputDevice;

    // The VirtualDevice for PreRendering
    VirtualDevice                                       maPreRenderDevice;

public:
    SdrPreRenderDevice(OutputDevice& rOriginal);
    ~SdrPreRenderDevice();

    void PreparePreRenderDevice();
    void OutputPreRenderDevice(const Region& rExpandedRegion);

    OutputDevice& GetOriginalOutputDevice() const { return mrOutputDevice; }
    OutputDevice& GetPreRenderDevice() { return maPreRenderDevice; }
};

////////////////////////////////////////////////////////////////////////////////////////////////////

class SVX_DLLPUBLIC SdrPaintWindow
{
private:
    // the OutputDevice this window represents
    OutputDevice&                                       mrOutputDevice;

    // the SdrPaintView this window belongs to
    SdrPaintView&                                       mrPaintView;

    // the new OverlayManager for the new OverlayObjects. Test add here, will
    // replace the IAOManager as soon as it works.
    ::sdr::overlay::OverlayManager*                     mpOverlayManager;

    // The PreRenderDevice for PreRendering
    SdrPreRenderDevice*                                 mpPreRenderDevice;

    // The RedrawRegion used for rendering
    Region                                              maRedrawRegion;

    // bitfield
    // #i72889# flag if this is only a temporary target for repaint, default is false
    bool                                                mbTemporaryTarget : 1;

    // helpers
    void impCreateOverlayManager();

public:
    SdrPaintWindow(SdrPaintView& rNewPaintView, OutputDevice& rOut);
    ~SdrPaintWindow();

    // data read accesses
    SdrPaintView& GetPaintView() const { return mrPaintView; }
    OutputDevice& GetOutputDevice() const { return mrOutputDevice; }

    // OVERLAYMANAGER
    ::sdr::overlay::OverlayManager* GetOverlayManager() const;
    void DrawOverlay(const Region& rRegion);
    void HideOverlay(const Region& rRegion);

    // calculate visible area and return
    Rectangle GetVisibleArea() const;

    // Is OutDev a printer?
    sal_Bool OutputToPrinter() const { return (OUTDEV_PRINTER == mrOutputDevice.GetOutDevType()); }

    // Is OutDev a window?
    sal_Bool OutputToWindow() const { return (OUTDEV_WINDOW == mrOutputDevice.GetOutDevType()); }

    // Is OutDev a VirtualDevice?
    sal_Bool OutputToVirtualDevice() const { return (OUTDEV_VIRDEV == mrOutputDevice.GetOutDevType()); }

    // Is OutDev a recording MetaFile?
    sal_Bool OutputToRecordingMetaFile() const;

    // prepare PreRendering (evtl.)
    void PreparePreRenderDevice();
    void DestroyPreRenderDevice();
    void OutputPreRenderDevice(const Region& rExpandedRegion);
    SdrPreRenderDevice* GetPreRenderDevice() const { return mpPreRenderDevice; }

    // RedrawRegion
    const Region& GetRedrawRegion() const;
    void SetRedrawRegion(const Region& rNew);

    // #i72889# read/write access to TempoparyTarget
    bool getTemporaryTarget() const { return (bool)mbTemporaryTarget; }
    void setTemporaryTarget(bool bNew) { if(bNew != (bool)mbTemporaryTarget) mbTemporaryTarget = bNew; }

    // #i72889# get target output device, take into account output buffering
    OutputDevice& GetTargetOutputDevice() { if(mpPreRenderDevice) return mpPreRenderDevice->GetPreRenderDevice(); else return mrOutputDevice; }
};

// typedefs for a list of SdrPaintWindows
typedef ::std::vector< SdrPaintWindow* > SdrPaintWindowVector;

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //_SDRPAINTWINDOW_HXX

