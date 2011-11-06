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
