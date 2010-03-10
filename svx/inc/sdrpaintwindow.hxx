/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
    unsigned                                            mbTemporaryTarget : 1;

    /** Remember whether the mpOverlayManager supports buffering.  Using
        this flags expensive dynamic_casts on mpOverlayManager in order to
        detect this.
    */
    bool mbUseBuffer;

    // helpers
    /** Create mpOverlayManager member on demand.
        @param bUseBuffer
            Specifies whether to use the buffered (OverlayManagerBuffered)
            or the unbuffered (OverlayManager) version of the overlay
            manager.  When this values is different from that of the
            previous call then the overlay manager is replaced by the
            specified one.

            The bUseBuffer flag will typically change its value when text
            editing is started or stopped.
    */
    void impCreateOverlayManager(const bool bUseBuffer);

public:
    SdrPaintWindow(SdrPaintView& rNewPaintView, OutputDevice& rOut);
    ~SdrPaintWindow();

    // data read accesses
    SdrPaintView& GetPaintView() const { return mrPaintView; }
    OutputDevice& GetOutputDevice() const { return mrOutputDevice; }

    // OVERLAYMANAGER
    ::sdr::overlay::OverlayManager* GetOverlayManager() const;
    // #i73602# add flag if buffer shall be used
    void DrawOverlay(const Region& rRegion, bool bUseBuffer);
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

