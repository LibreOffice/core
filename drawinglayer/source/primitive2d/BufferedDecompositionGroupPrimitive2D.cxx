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

#include <sal/config.h>

#include <drawinglayer/primitive2d/BufferedDecompositionGroupPrimitive2D.hxx>
#include <drawinglayer/geometry/viewinformation2d.hxx>

namespace
{
class LocalCallbackTimer : public salhelper::Timer
{
protected:
    drawinglayer::primitive2d::BufferedDecompositionGroupPrimitive2D* pCustomer;

public:
    explicit LocalCallbackTimer(
        drawinglayer::primitive2d::BufferedDecompositionGroupPrimitive2D& rCustomer)
        : pCustomer(&rCustomer)
    {
    }

    void clearCallback() { pCustomer = nullptr; }

protected:
    virtual void SAL_CALL onShot() override;
};

void SAL_CALL LocalCallbackTimer::onShot()
{
    if (nullptr != pCustomer)
        flushBufferedDecomposition(*pCustomer);
}
}

namespace drawinglayer::primitive2d
{
void flushBufferedDecomposition(BufferedDecompositionGroupPrimitive2D& rTarget)
{
    rTarget.acquire();
    rTarget.setBuffered2DDecomposition(Primitive2DContainer());
    rTarget.release();
}

bool BufferedDecompositionGroupPrimitive2D::hasBuffered2DDecomposition() const
{
    if (0 != maCallbackSeconds)
    {
        std::lock_guard Guard(maCallbackLock);
        return !maBuffered2DDecomposition.empty();
    }
    else
    {
        return !maBuffered2DDecomposition.empty();
    }
}

void BufferedDecompositionGroupPrimitive2D::setBuffered2DDecomposition(Primitive2DContainer&& rNew)
{
    if (0 == maCallbackSeconds)
    {
        // no flush used, just set
        maBuffered2DDecomposition = std::move(rNew);
        return;
    }

    if (maCallbackTimer.is())
    {
        if (rNew.empty())
        {
            // stop timer
            maCallbackTimer->stop();
        }
        else
        {
            // decomposition changed, touch
            maCallbackTimer->setRemainingTime(salhelper::TTimeValue(maCallbackSeconds, 0));
            if (!maCallbackTimer->isTicking())
                maCallbackTimer->start();
        }
    }
    else if (!rNew.empty())
    {
        // decomposition defined/set/changed, init & start timer
        maCallbackTimer.set(new LocalCallbackTimer(*this));
        maCallbackTimer->setRemainingTime(salhelper::TTimeValue(maCallbackSeconds, 0));
        maCallbackTimer->start();
    }

    // tdf#158913 need to secure change when flush/multithreading is in use
    std::lock_guard Guard(maCallbackLock);
    maBuffered2DDecomposition = std::move(rNew);
}

BufferedDecompositionGroupPrimitive2D::BufferedDecompositionGroupPrimitive2D(
    Primitive2DContainer&& aChildren)
    : GroupPrimitive2D(std::move(aChildren))
    , maCallbackTimer()
    , maCallbackLock()
    , maCallbackSeconds(0)
{
}

BufferedDecompositionGroupPrimitive2D::~BufferedDecompositionGroupPrimitive2D()
{
    if (maCallbackTimer.is())
    {
        // no more decomposition, end callback
        static_cast<LocalCallbackTimer*>(maCallbackTimer.get())->clearCallback();
        maCallbackTimer->stop();
    }
}

void BufferedDecompositionGroupPrimitive2D::get2DDecomposition(
    Primitive2DDecompositionVisitor& rVisitor,
    const geometry::ViewInformation2D& rViewInformation) const
{
    if (!hasBuffered2DDecomposition())
    {
        Primitive2DContainer aNewSequence;
        create2DDecomposition(aNewSequence, rViewInformation);
        const_cast<BufferedDecompositionGroupPrimitive2D*>(this)->setBuffered2DDecomposition(
            std::move(aNewSequence));
    }

    if (0 == maCallbackSeconds)
    {
        // no flush/multithreading is in use, just call
        rVisitor.visit(maBuffered2DDecomposition);
        return;
    }

    // decomposition was used, touch/restart time
    if (maCallbackTimer)
        maCallbackTimer->setRemainingTime(salhelper::TTimeValue(maCallbackSeconds, 0));

    // tdf#158913 need to secure 'visit' when flush/multithreading is in use,
    // so that the local non-ref-Counted instance of the decomposition gets not
    // manipulated (e.g. deleted)
    std::lock_guard Guard(maCallbackLock);
    rVisitor.visit(maBuffered2DDecomposition);
}

} // end of namespace drawinglayer::primitive2d

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
