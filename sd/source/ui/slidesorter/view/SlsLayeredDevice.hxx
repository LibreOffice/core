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

#ifndef SD_SLIDESORTER_VIEW_LAYERED_DEVICE_HXX
#define SD_SLIDESORTER_VIEW_LAYERED_DEVICE_HXX

#include "view/SlsILayerPainter.hxx"
#include "SlideSorter.hxx"

#include <tools/gen.hxx>
#include <vcl/region.hxx>
#include <vcl/virdev.hxx>

#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <vector>

class Window;

namespace sd { namespace slidesorter { namespace view {

/** A simple wrapper around an OutputDevice that provides support for
    independent layers and buffering.
    Each layer may contain any number of painters.
*/
class LayeredDevice
    : public ::boost::enable_shared_from_this<LayeredDevice>

{
public:
    LayeredDevice (const SharedSdWindow& rpTargetWindow);
    ~LayeredDevice (void);

    void Invalidate (
        const Rectangle& rInvalidationBox,
        const sal_Int32 nLayer);
    void InvalidateAllLayers (
        const Rectangle& rInvalidationBox);
    void InvalidateAllLayers (
        const Region& rInvalidationRegion);

    void RegisterPainter (
        const SharedILayerPainter& rPainter,
        const sal_Int32 nLayer);

    void RemovePainter (
        const SharedILayerPainter& rPainter,
        const sal_Int32 nLayer);

    bool HasPainter (const sal_Int32 nLayer);

    bool HandleMapModeChange (void);
    void Repaint (const Region& rRepaintRegion);

    void Resize (void);

    void Dispose (void);

private:
    SharedSdWindow mpTargetWindow;
    class LayerContainer;
    ::boost::scoped_ptr<LayerContainer> mpLayers;
    ::boost::scoped_ptr<VirtualDevice> mpBackBuffer;
    MapMode maSavedMapMode;

    void RepaintRectangle (const Rectangle& rRepaintRectangle);
};



} } } // end of namespace ::sd::slidesorter::view

#endif
