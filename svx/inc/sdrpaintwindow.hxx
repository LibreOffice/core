/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sdrpaintwindow.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ihi $ $Date: 2006-11-14 12:43:49 $
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

#ifndef _SDRPAINTWINDOW_HXX
#define _SDRPAINTWINDOW_HXX

#ifndef _VIRDEV_HXX //autogen
#include <vcl/virdev.hxx>
#endif

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

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

    // The RedrawRegion used for rendering, Maybe expanded by ExpandPaintClipRegion
    Region                                              maRedrawRegion;

    // helpers
    // create mpOverlayManager member on demand
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

    // RedrawRegion, maybe expanded by ExpandPaintClipRegion
    const Region& GetRedrawRegion() const;
    void SetRedrawRegion(const Region& rNew);
};

// typedefs for a list of SdrPaintWindows
typedef ::std::vector< SdrPaintWindow* > SdrPaintWindowVector;

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //_SDRPAINTWINDOW_HXX

