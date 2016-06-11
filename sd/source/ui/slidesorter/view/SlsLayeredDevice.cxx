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

#include <vcl/window.hxx>
#include <vcl/virdev.hxx>

#include <tools/gen.hxx>
#include <tools/fract.hxx>

#include <boost/bind.hpp>
#include <boost/noncopyable.hpp>

#include <functional>

namespace sd { namespace slidesorter { namespace view {

namespace {
static const sal_Int32 gnMaximumLayerCount = 8;

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

    virtual ~LayerInvalidator ( )
    {
    }

    virtual void Invalidate (const Rectangle& rInvalidationBox) override
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
    vcl::RenderContext& rSourceDevice,
    const Rectangle& rBox)
{
    rTargetDevice.DrawOutDev(
        rBox.TopLeft(),
        rBox.GetSize(),
        rBox.TopLeft(),
        rBox.GetSize(),
        rSourceDevice);
}

void ForAllRectangles (const vcl::Region& rRegion, ::std::function<void (const Rectangle&)> aFunction)
{
    OSL_ASSERT(aFunction);
    RectangleVector aRectangles;
    rRegion.GetRegionRectangles(aRectangles);

    if(0 == aRectangles.size())
    {
        aFunction(Rectangle());
    }
    else
    {
        for(RectangleVector::const_iterator aRectIter(aRectangles.begin()); aRectIter != aRectangles.end(); ++aRectIter)
        {
            aFunction(*aRectIter);
        }

        //Region aMutableRegionCopy (rRegion);
        //RegionHandle aHandle(aMutableRegionCopy.BeginEnumRects());
        //Rectangle aBox;
        //while (aMutableRegionCopy.GetEnumRects(aHandle, aBox))
        //    aFunction(aBox);
        //aMutableRegionCopy.EndEnumRects(aHandle);
    }
}

class Layer : private ::boost::noncopyable
{
public:
    Layer();
    ~Layer();

    void Initialize (sd::Window *pTargetWindow);
    void InvalidateRectangle (const Rectangle& rInvalidationBox);
    void InvalidateRegion (const vcl::Region& rInvalidationRegion);
    void Validate (const MapMode& rMapMode);
    void Repaint (
        OutputDevice& rTargetDevice,
        const Rectangle& rRepaintRectangle);
    void Resize (const Size& rSize);
    void AddPainter (const SharedILayerPainter& rpPainter);
    void RemovePainter (const SharedILayerPainter& rpPainter);
    bool HasPainter() const;
    void Dispose();

private:
    ScopedVclPtr<VirtualDevice> mpLayerDevice;
    ::std::vector<SharedILayerPainter> maPainters;
    vcl::Region maInvalidationRegion;

    void ValidateRectangle (const Rectangle& rBox);
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

    const ::std::vector<SharedLayer>::const_iterator begin() const { return mvLayers.begin(); }
    const ::std::vector<SharedLayer>::const_iterator end() const { return mvLayers.end(); }

    void clear() { mvLayers.clear(); }

    void pop_back() { mvLayers.pop_back(); }

    void resize(size_t n) { mvLayers.resize(n); }

    SharedLayer& operator[](size_t i) { return mvLayers[i]; }

private:
    ::std::vector<SharedLayer> mvLayers;
};

//===== LayeredDevice =========================================================

LayeredDevice::LayeredDevice (VclPtr<sd::Window> pTargetWindow)
    : mpTargetWindow(pTargetWindow),
      mpLayers(new LayerContainer()),
      mpBackBuffer(VclPtr<VirtualDevice>::Create(*mpTargetWindow)),
      maSavedMapMode(pTargetWindow->GetMapMode())
{
    mpBackBuffer->SetOutputSizePixel(mpTargetWindow->GetSizePixel());
}

LayeredDevice::~LayeredDevice()
{
}

void LayeredDevice::Invalidate (
    const Rectangle& rInvalidationArea,
    const sal_Int32 nLayer)
{
    if (nLayer<0 || size_t(nLayer)>=mpLayers->size())
    {
        OSL_ASSERT(nLayer>=0 && size_t(nLayer)<mpLayers->size());
        return;
    }

    (*mpLayers)[nLayer]->InvalidateRectangle(rInvalidationArea);
}

void LayeredDevice::InvalidateAllLayers (const Rectangle& rInvalidationArea)
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
    if (sal_uInt32(nLayer) >= mpLayers->size())
    {
        const sal_Int32 nOldLayerCount (mpLayers->size());
        mpLayers->resize(nLayer+1);

        for (size_t nIndex=nOldLayerCount; nIndex<mpLayers->size(); ++nIndex)
            (*mpLayers)[nIndex].reset(new Layer());
    }

    (*mpLayers)[nLayer]->AddPainter(rpPainter);
    if (nLayer == 0)
        (*mpLayers)[nLayer]->Initialize(mpTargetWindow);

    rpPainter->SetLayerInvalidator(
        SharedILayerInvalidator(new LayerInvalidator(shared_from_this(),mpTargetWindow,nLayer)));
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
    if (nLayer<0 || size_t(nLayer)>=mpLayers->size())
    {
        OSL_ASSERT(nLayer>=0 && size_t(nLayer)<mpLayers->size());
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
    ::std::for_each(
        mpLayers->begin(),
        mpLayers->end(),
        ::boost::bind(&Layer::Validate, _1, mpTargetWindow->GetMapMode()));

    ForAllRectangles(rRepaintRegion, ::boost::bind(&LayeredDevice::RepaintRectangle, this, _1));
}

void LayeredDevice::RepaintRectangle (const Rectangle& rRepaintRectangle)
{
    if (mpLayers->empty())
        return;
    else if (mpLayers->size() == 1)
    {
        // Just copy the main layer into the target device.
        (*mpLayers)[0]->Repaint(*mpTargetWindow, rRepaintRectangle);
    }
    else
    {
        // Paint all layers first into the back buffer (to avoid flickering
        // due to synchronous paints) and then copy that into the target
        // device.
        mpBackBuffer->SetMapMode(mpTargetWindow->GetMapMode());
        ::std::for_each(
            mpLayers->begin(),
            mpLayers->end(),
            ::boost::bind(&Layer::Repaint, _1, ::boost::ref(*mpBackBuffer), rRepaintRectangle));

        DeviceCopy(*mpTargetWindow, *mpBackBuffer, rRepaintRectangle);
    }
}

void LayeredDevice::Resize()
{
    const Size aSize (mpTargetWindow->GetSizePixel());
    mpBackBuffer->SetOutputSizePixel(aSize);
    ::std::for_each(mpLayers->begin(), mpLayers->end(), ::boost::bind(&Layer::Resize, _1, aSize));
}

void LayeredDevice::Dispose()
{
    ::std::for_each(mpLayers->begin(), mpLayers->end(), ::boost::bind(&Layer::Dispose, _1));
    mpLayers->clear();
}

bool LayeredDevice::HandleMapModeChange()
{
    const MapMode& rMapMode (mpTargetWindow->GetMapMode());
    if (maSavedMapMode == rMapMode)
        return false;

    const Rectangle aLogicWindowBox (
        mpTargetWindow->PixelToLogic(Rectangle(Point(0,0), mpTargetWindow->GetSizePixel())));
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
        const Rectangle aWindowBox (Point(0,0), mpTargetWindow->GetSizePixel());
        if (aDelta.Y() < 0)
            InvalidateAllLayers(mpTargetWindow->PixelToLogic(Rectangle(
                aWindowBox.Left(),
                aWindowBox.Bottom()+aDelta.Y(),
                aWindowBox.Right(),
                aWindowBox.Bottom())));
        else if (aDelta.Y() > 0)
            InvalidateAllLayers(mpTargetWindow->PixelToLogic(Rectangle(
                aWindowBox.Left(),
                aWindowBox.Top(),
                aWindowBox.Right(),
                aWindowBox.Top()+aDelta.Y())));
        if (aDelta.X() < 0)
            InvalidateAllLayers(mpTargetWindow->PixelToLogic(Rectangle(
                aWindowBox.Right()+aDelta.X(),
                aWindowBox.Top(),
                aWindowBox.Right(),
                aWindowBox.Bottom())));
        else if (aDelta.X() > 0)
            InvalidateAllLayers(mpTargetWindow->PixelToLogic(Rectangle(
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

Layer::~Layer()
{
}

void Layer::Initialize (sd::Window *pTargetWindow)
{
#if 0
    (void)pTargetWindow;
#else
    if ( ! mpLayerDevice)
    {
        mpLayerDevice.disposeAndReset(VclPtr<VirtualDevice>::Create(*pTargetWindow));
        mpLayerDevice->SetOutputSizePixel(pTargetWindow->GetSizePixel());
    }
#endif
}

void Layer::InvalidateRectangle (const Rectangle& rInvalidationBox)
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
            ::boost::bind(&Layer::ValidateRectangle, this, _1));
    }
}

void Layer::ValidateRectangle (const Rectangle& rBox)
{
    if ( ! mpLayerDevice)
        return;
    const vcl::Region aSavedClipRegion (mpLayerDevice->GetClipRegion());
    mpLayerDevice->IntersectClipRegion(rBox);

    for (::std::vector<SharedILayerPainter>::const_iterator
             iPainter(maPainters.begin()),
             iEnd(maPainters.end());
         iPainter!=iEnd;
         ++iPainter)
    {
        (*iPainter)->Paint(*mpLayerDevice, rBox);
    }

    mpLayerDevice->SetClipRegion(aSavedClipRegion);
}

void Layer::Repaint (
    OutputDevice& rTargetDevice,
    const Rectangle& rRepaintRectangle)
{
    if (mpLayerDevice)
    {
        DeviceCopy(rTargetDevice, *mpLayerDevice, rRepaintRectangle);
    }
    else
    {
        ::std::for_each(
            maPainters.begin(),
            maPainters.end(),
            ::boost::bind(&ILayerPainter::Paint,
                _1,
                ::boost::ref(rTargetDevice),
                rRepaintRectangle));
    }
}

void Layer::Resize (const Size& rSize)
{
    if (mpLayerDevice)
    {
        mpLayerDevice->SetOutputSizePixel(rSize);
        maInvalidationRegion = Rectangle(Point(0,0), rSize);
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

} } } // end of namespace ::sd::slidesorter::view

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
