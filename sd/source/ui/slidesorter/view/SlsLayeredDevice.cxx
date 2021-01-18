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

#include "SlsLayeredDevice.hxx"
#include <Window.hxx>

#include <vcl/virdev.hxx>
#include <sal/log.hxx>
#include <o3tl/safeint.hxx>
#include <osl/diagnose.h>

#include <tools/gen.hxx>
#include <tools/fract.hxx>

#include <functional>

namespace sd::slidesorter::view {

namespace {
const sal_Int32 gnMaximumLayerCount = 8;

class LayerInvalidator : public ILayerInvalidator
{
public:
    LayerInvalidator (
        const std::shared_ptr<LayeredDevice>& rpLayeredDevice,
        sd::Window *pTargetWindow,
        const int nLayer)
        : mpLayeredDevice(rpLayeredDevice),
          mpTargetWindow(pTargetWindow),
          mnLayer(nLayer)
    {
    }

    virtual void Invalidate (const ::tools::Rectangle& rInvalidationBox) override
    {
        mpLayeredDevice->Invalidate(rInvalidationBox, mnLayer);
        mpTargetWindow->Invalidate(rInvalidationBox);
    }

private:
    const std::shared_ptr<LayeredDevice> mpLayeredDevice;
    VclPtr<sd::Window> mpTargetWindow;
    const int mnLayer;
};

void DeviceCopy (
    vcl::RenderContext& rTargetDevice,
    vcl::RenderContext const & rSourceDevice,
    const ::tools::Rectangle& rBox)
{
    rTargetDevice.DrawOutDev(
        rBox.TopLeft(),
        rBox.GetSize(),
        rBox.TopLeft(),
        rBox.GetSize(),
        rSourceDevice);
}

void ForAllRectangles (const vcl::Region& rRegion, const std::function<void (const ::tools::Rectangle&)>& aFunction)
{
    OSL_ASSERT(aFunction);
    RectangleVector aRectangles;
    rRegion.GetRegionRectangles(aRectangles);

    if(aRectangles.empty())
    {
        aFunction(::tools::Rectangle());
    }
    else
    {
        for(const auto& rRect : aRectangles)
        {
            aFunction(rRect);
        }

        //Region aMutableRegionCopy (rRegion);
        //RegionHandle aHandle(aMutableRegionCopy.BeginEnumRects());
        //Rectangle aBox;
        //while (aMutableRegionCopy.GetEnumRects(aHandle, aBox))
        //    aFunction(aBox);
        //aMutableRegionCopy.EndEnumRects(aHandle);
    }
}

class Layer
{
public:
    Layer();
    Layer(const Layer&) = delete;
    Layer& operator=(const Layer&) = delete;

    void Initialize (sd::Window *pTargetWindow);
    void InvalidateRectangle (const ::tools::Rectangle& rInvalidationBox);
    void InvalidateRegion (const vcl::Region& rInvalidationRegion);
    void Validate (const MapMode& rMapMode);
    void Repaint (
        OutputDevice& rTargetDevice,
        const ::tools::Rectangle& rRepaintRectangle);
    void Resize (const Size& rSize);
    void AddPainter (const SharedILayerPainter& rpPainter);
    void RemovePainter (const SharedILayerPainter& rpPainter);
    bool HasPainter() const;
    void Dispose();

private:
    ScopedVclPtr<VirtualDevice> mpLayerDevice;
    ::std::vector<SharedILayerPainter> maPainters;
    vcl::Region maInvalidationRegion;

    void ValidateRectangle (const ::tools::Rectangle& rBox);
};
typedef std::shared_ptr<Layer> SharedLayer;

} // end of anonymous namespace

class LayeredDevice::LayerContainer
{
public:
    LayerContainer() : mvLayers() {}

    bool empty() const { return mvLayers.empty(); }

    size_t size() const { return mvLayers.size(); }

    const SharedLayer& back() const { return mvLayers.back(); }

    ::std::vector<SharedLayer>::const_iterator begin() const { return mvLayers.begin(); }
    ::std::vector<SharedLayer>::const_iterator end() const { return mvLayers.end(); }

    void clear() { mvLayers.clear(); }

    void pop_back() { mvLayers.pop_back(); }

    void resize(size_t n) { mvLayers.resize(n); }

    SharedLayer& operator[](size_t i) { return mvLayers[i]; }

private:
    ::std::vector<SharedLayer> mvLayers;
};

//===== LayeredDevice =========================================================

LayeredDevice::LayeredDevice (const VclPtr<sd::Window>& pTargetWindow)
    : mpTargetWindow(pTargetWindow),
      mpLayers(new LayerContainer()),
      mpBackBuffer(VclPtr<VirtualDevice>::Create(*mpTargetWindow->GetOutDev())),
      maSavedMapMode(pTargetWindow->GetMapMode())
{
    mpBackBuffer->SetOutputSizePixel(mpTargetWindow->GetSizePixel());
}

LayeredDevice::~LayeredDevice()
{
}

void LayeredDevice::Invalidate (
    const ::tools::Rectangle& rInvalidationArea,
    const sal_Int32 nLayer)
{
    if (nLayer<0 || o3tl::make_unsigned(nLayer)>=mpLayers->size())
    {
        OSL_ASSERT(nLayer>=0 && o3tl::make_unsigned(nLayer)<mpLayers->size());
        return;
    }

    (*mpLayers)[nLayer]->InvalidateRectangle(rInvalidationArea);
}

void LayeredDevice::InvalidateAllLayers (const ::tools::Rectangle& rInvalidationArea)
{
    for (size_t nLayer=0; nLayer<mpLayers->size(); ++nLayer)
        (*mpLayers)[nLayer]->InvalidateRectangle(rInvalidationArea);
}

void LayeredDevice::InvalidateAllLayers (const vcl::Region& rInvalidationRegion)
{
    for (size_t nLayer=0; nLayer<mpLayers->size(); ++nLayer)
        (*mpLayers)[nLayer]->InvalidateRegion(rInvalidationRegion);
}

void LayeredDevice::RegisterPainter (
    const SharedILayerPainter& rpPainter,
    const sal_Int32 nLayer)
{
    OSL_ASSERT(mpLayers);
    if ( ! rpPainter)
    {
        OSL_ASSERT(rpPainter);
        return;
    }
    if (nLayer<0 || nLayer>=gnMaximumLayerCount)
    {
        OSL_ASSERT(nLayer>=0 && nLayer<gnMaximumLayerCount);
        return;
    }

    // Provide the layers.
    if (o3tl::make_unsigned(nLayer) >= mpLayers->size())
    {
        const sal_Int32 nOldLayerCount (mpLayers->size());
        mpLayers->resize(nLayer+1);

        for (size_t nIndex=nOldLayerCount; nIndex<mpLayers->size(); ++nIndex)
            (*mpLayers)[nIndex] = std::make_shared<Layer>();
    }

    (*mpLayers)[nLayer]->AddPainter(rpPainter);
    if (nLayer == 0)
        (*mpLayers)[nLayer]->Initialize(mpTargetWindow);

    rpPainter->SetLayerInvalidator(
        std::make_shared<LayerInvalidator>(shared_from_this(),mpTargetWindow,nLayer));
}

void LayeredDevice::RemovePainter (
    const SharedILayerPainter& rpPainter,
    const sal_Int32 nLayer)
{
    if ( ! rpPainter)
    {
        OSL_ASSERT(rpPainter);
        return;
    }
    if (nLayer<0 || o3tl::make_unsigned(nLayer)>=mpLayers->size())
    {
        OSL_ASSERT(nLayer>=0 && o3tl::make_unsigned(nLayer)<mpLayers->size());
        return;
    }

    rpPainter->SetLayerInvalidator(SharedILayerInvalidator());

    (*mpLayers)[nLayer]->RemovePainter(rpPainter);

    // Remove top most layers that do not contain any painters.
    while ( ! mpLayers->empty() && ! mpLayers->back()->HasPainter())
        mpLayers->pop_back();
}

void LayeredDevice::Repaint (const vcl::Region& rRepaintRegion)
{
    // Validate the contents of all layers (that have their own devices.)
    for (auto const& it : *mpLayers)
    {
        it->Validate(mpTargetWindow->GetMapMode());
    }

    ForAllRectangles(rRepaintRegion,
            [this] (::tools::Rectangle const& r) { this->RepaintRectangle(r); });
}

void LayeredDevice::RepaintRectangle (const ::tools::Rectangle& rRepaintRectangle)
{
    if (mpLayers->empty())
        return;
    else if (mpLayers->size() == 1)
    {
        // Just copy the main layer into the target device.
        (*mpLayers)[0]->Repaint(*mpTargetWindow->GetOutDev(), rRepaintRectangle);
    }
    else
    {
        // Paint all layers first into the back buffer (to avoid flickering
        // due to synchronous paints) and then copy that into the target
        // device.
        mpBackBuffer->SetMapMode(mpTargetWindow->GetMapMode());
        for (auto const& it : *mpLayers)
        {
            it->Repaint(*mpBackBuffer, rRepaintRectangle);
        }
        DeviceCopy(*mpTargetWindow->GetOutDev(), *mpBackBuffer, rRepaintRectangle);
    }
}

void LayeredDevice::Resize()
{
    const Size aSize (mpTargetWindow->GetSizePixel());
    mpBackBuffer->SetOutputSizePixel(aSize);
    for (auto const& it : *mpLayers)
    {
        it->Resize(aSize);
    }
}

void LayeredDevice::Dispose()
{
    for (auto const& it : *mpLayers)
    {
        it->Dispose();
    }
    mpLayers->clear();
}

bool LayeredDevice::HandleMapModeChange()
{
    const MapMode& rMapMode (mpTargetWindow->GetMapMode());
    if (maSavedMapMode == rMapMode)
        return false;

    const ::tools::Rectangle aLogicWindowBox (
        mpTargetWindow->PixelToLogic(::tools::Rectangle(Point(0,0), mpTargetWindow->GetSizePixel())));
    if (maSavedMapMode.GetScaleX() != rMapMode.GetScaleX()
        || maSavedMapMode.GetScaleY() != rMapMode.GetScaleY()
        || maSavedMapMode.GetMapUnit() != rMapMode.GetMapUnit())
    {
        // When the scale has changed then we have to paint everything.
        InvalidateAllLayers(aLogicWindowBox);
    }
    else if (maSavedMapMode.GetOrigin() != rMapMode.GetOrigin())
    {
        // Window has been scrolled.  Adapt contents of backbuffers and
        // layer devices.
        const Point aDelta (rMapMode.GetOrigin() - maSavedMapMode.GetOrigin());
        mpBackBuffer->CopyArea(
            aLogicWindowBox.TopLeft(),
            mpTargetWindow->PixelToLogic(Point(0,0), maSavedMapMode),
            aLogicWindowBox.GetSize());

        // Invalidate the area(s) that have been exposed.
        const ::tools::Rectangle aWindowBox (Point(0,0), mpTargetWindow->GetSizePixel());
        if (aDelta.Y() < 0)
            InvalidateAllLayers(mpTargetWindow->PixelToLogic(::tools::Rectangle(
                aWindowBox.Left(),
                aWindowBox.Bottom()+aDelta.Y(),
                aWindowBox.Right(),
                aWindowBox.Bottom())));
        else if (aDelta.Y() > 0)
            InvalidateAllLayers(mpTargetWindow->PixelToLogic(::tools::Rectangle(
                aWindowBox.Left(),
                aWindowBox.Top(),
                aWindowBox.Right(),
                aWindowBox.Top()+aDelta.Y())));
        if (aDelta.X() < 0)
            InvalidateAllLayers(mpTargetWindow->PixelToLogic(::tools::Rectangle(
                aWindowBox.Right()+aDelta.X(),
                aWindowBox.Top(),
                aWindowBox.Right(),
                aWindowBox.Bottom())));
        else if (aDelta.X() > 0)
            InvalidateAllLayers(mpTargetWindow->PixelToLogic(::tools::Rectangle(
                aWindowBox.Left(),
                aWindowBox.Top(),
                aWindowBox.Left()+aDelta.X(),
                aWindowBox.Bottom())));
    }
    else
    {
        // Can this happen?  Lets trigger a warning when it does.
        OSL_ASSERT(false);
    }

    maSavedMapMode = rMapMode;

    return true;
}

//===== Layer =================================================================

Layer::Layer()
    : mpLayerDevice(),
      maPainters(),
      maInvalidationRegion()
{
}

void Layer::Initialize (sd::Window *pTargetWindow)
{
#if 0
    (void)pTargetWindow;
#else
    if ( ! mpLayerDevice)
    {
        mpLayerDevice.disposeAndReset(VclPtr<VirtualDevice>::Create(*pTargetWindow->GetOutDev()));
        mpLayerDevice->SetOutputSizePixel(pTargetWindow->GetSizePixel());
    }
#endif
}

void Layer::InvalidateRectangle (const ::tools::Rectangle& rInvalidationBox)
{
    maInvalidationRegion.Union(rInvalidationBox);
}

void Layer::InvalidateRegion (const vcl::Region& rInvalidationRegion)
{
    maInvalidationRegion.Union(rInvalidationRegion);
}

void Layer::Validate (const MapMode& rMapMode)
{
    if (mpLayerDevice && ! maInvalidationRegion.IsEmpty())
    {
        vcl::Region aRegion (maInvalidationRegion);
        maInvalidationRegion.SetEmpty();

        mpLayerDevice->SetMapMode(rMapMode);
        ForAllRectangles(
            aRegion,
            [this] (::tools::Rectangle const& r) { return this->ValidateRectangle(r); });
    }
}

void Layer::ValidateRectangle (const ::tools::Rectangle& rBox)
{
    if ( ! mpLayerDevice)
        return;
    const vcl::Region aSavedClipRegion (mpLayerDevice->GetClipRegion());
    mpLayerDevice->IntersectClipRegion(rBox);

    for (const auto& rxPainter : maPainters)
    {
        rxPainter->Paint(*mpLayerDevice, rBox);
    }

    mpLayerDevice->SetClipRegion(aSavedClipRegion);
}

void Layer::Repaint (
    OutputDevice& rTargetDevice,
    const ::tools::Rectangle& rRepaintRectangle)
{
    if (mpLayerDevice)
    {
        DeviceCopy(rTargetDevice, *mpLayerDevice, rRepaintRectangle);
    }
    else
    {
        for (auto const& it : maPainters)
        {
            it->Paint(rTargetDevice, rRepaintRectangle);
        }
    }
}

void Layer::Resize (const Size& rSize)
{
    if (mpLayerDevice)
    {
        mpLayerDevice->SetOutputSizePixel(rSize);
        maInvalidationRegion = ::tools::Rectangle(Point(0,0), rSize);
    }
}

void Layer::AddPainter (const SharedILayerPainter& rpPainter)
{
    OSL_ASSERT(::std::find(maPainters.begin(), maPainters.end(), rpPainter) == maPainters.end());

    maPainters.push_back(rpPainter);
}

void Layer::RemovePainter (const SharedILayerPainter& rpPainter)
{
    const ::std::vector<SharedILayerPainter>::iterator iPainter (
        ::std::find(maPainters.begin(), maPainters.end(), rpPainter));
    if (iPainter != maPainters.end())
    {
        maPainters.erase(iPainter);
    }
    else
    {
        SAL_WARN("sd", "LayeredDevice::RemovePainter called for painter that is not registered");
    }
}

bool Layer::HasPainter() const
{
    return !maPainters.empty();
}

void Layer::Dispose()
{
    maPainters.clear();
}

} // end of namespace ::sd::slidesorter::view

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
