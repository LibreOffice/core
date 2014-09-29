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

#ifndef INCLUDED_SVX_SDRPAINTWINDOW_HXX
#define INCLUDED_SVX_SDRPAINTWINDOW_HXX

#include <rtl/ref.hxx>
#include <vcl/virdev.hxx>
#include <svx/svxdllapi.h>


// predeclarations
class SdrPaintView;

namespace sdr
{
    namespace overlay
    {
        class OverlayManager;
    } // end of namespace overlay
} // end of namespace sdr

#ifdef _MSC_VER // broken msvc template instantiation
#include <svx/sdr/overlay/overlaymanager.hxx>
#endif


/// paint the transparent children of rWin that overlap rPixelRect
/// (for example, transparent form controls like check boxes)
void SVX_DLLPUBLIC
PaintTransparentChildren(vcl::Window & rWindow, Rectangle const& rPixelRect);


class SdrPreRenderDevice
{
    // The original OutputDevice
    OutputDevice&                                       mrOutputDevice;

    // The VirtualDevice for PreRendering
    VirtualDevice                                       maPreRenderDevice;

public:
    explicit SdrPreRenderDevice(OutputDevice& rOriginal);
    ~SdrPreRenderDevice();

    void PreparePreRenderDevice();
    void OutputPreRenderDevice(const vcl::Region& rExpandedRegion);

    OutputDevice& GetOriginalOutputDevice() const { return mrOutputDevice; }
    OutputDevice& GetPreRenderDevice() { return maPreRenderDevice; }
};



class SVX_DLLPUBLIC SdrPaintWindow
{
private:
    // the OutputDevice this window represents
    OutputDevice&                                       mrOutputDevice;

    // the SdrPaintView this window belongs to
    SdrPaintView&                                       mrPaintView;

    // the new OverlayManager for the new OverlayObjects. Test add here, will
    // replace the IAOManager as soon as it works.
    rtl::Reference< ::sdr::overlay::OverlayManager >    mxOverlayManager;

    // The PreRenderDevice for PreRendering
    SdrPreRenderDevice*                                 mpPreRenderDevice;

    // The RedrawRegion used for rendering
    vcl::Region                                              maRedrawRegion;

    // bitfield
    // #i72889# flag if this is only a temporary target for repaint, default is false
    bool                                                mbTemporaryTarget : 1;

    /** Remember whether the mxOverlayManager supports buffering.  Using
        this flags expensive dynamic_casts on mxOverlayManager in order to
        detect this.
    */
    bool mbUseBuffer;

    // helpers
    void impCreateOverlayManager();

public:
    SdrPaintWindow(SdrPaintView& rNewPaintView, OutputDevice& rOut);
    ~SdrPaintWindow();

    // data read accesses
    SdrPaintView& GetPaintView() const { return mrPaintView; }
    OutputDevice& GetOutputDevice() const { return mrOutputDevice; }

    // OVERLAYMANAGER
    rtl::Reference< ::sdr::overlay::OverlayManager > GetOverlayManager() const;

    // #i73602# add flag if buffer shall be used
    void DrawOverlay(const vcl::Region& rRegion);

    // calculate visible area and return
    Rectangle GetVisibleArea() const;

    // Is OutDev a printer?
    bool OutputToPrinter() const { return (OUTDEV_PRINTER == mrOutputDevice.GetOutDevType()); }

    // Is OutDev a window?
    bool OutputToWindow() const { return (OUTDEV_WINDOW == mrOutputDevice.GetOutDevType()); }

    // Is OutDev a VirtualDevice?
    bool OutputToVirtualDevice() const { return (OUTDEV_VIRDEV == mrOutputDevice.GetOutDevType()); }

    // Is OutDev a recording MetaFile?
    bool OutputToRecordingMetaFile() const;

    // prepare PreRendering (evtl.)
    void PreparePreRenderDevice();
    void DestroyPreRenderDevice();
    void OutputPreRenderDevice(const vcl::Region& rExpandedRegion);
    SdrPreRenderDevice* GetPreRenderDevice() const { return mpPreRenderDevice; }

    // RedrawRegion
    const vcl::Region& GetRedrawRegion() const { return maRedrawRegion;}
    void SetRedrawRegion(const vcl::Region& rNew);

    // #i72889# read/write access to TempoparyTarget
    bool getTemporaryTarget() const { return (bool)mbTemporaryTarget; }
    void setTemporaryTarget(bool bNew) { mbTemporaryTarget = bNew; }

    // #i72889# get target output device, take into account output buffering
    OutputDevice& GetTargetOutputDevice() { if(mpPreRenderDevice) return mpPreRenderDevice->GetPreRenderDevice(); else return mrOutputDevice; }
};

// typedefs for a list of SdrPaintWindows
typedef ::std::vector< SdrPaintWindow* > SdrPaintWindowVector;



#endif // INCLUDED_SVX_SDRPAINTWINDOW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
