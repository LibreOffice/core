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

#include <drawinglayer/primitive2d/BufferedDecompositionPrimitive2D.hxx>
#include <drawinglayer/geometry/viewinformation2d.hxx>

namespace
{
class LocalCallbackTimer : public salhelper::Timer
{
protected:
    drawinglayer::primitive2d::BufferedDecompositionPrimitive2D* pCustomer;

public:
    explicit LocalCallbackTimer(
        drawinglayer::primitive2d::BufferedDecompositionPrimitive2D& rCustomer)
        : pCustomer(&rCustomer)
    {
    }

protected:
    virtual void SAL_CALL onShot() override;
};

void SAL_CALL LocalCallbackTimer::onShot() { flushBufferedDecomposition(*pCustomer); }
}

namespace drawinglayer::primitive2d
{
void flushBufferedDecomposition(BufferedDecompositionPrimitive2D& rTarget)
{
    rTarget.setBuffered2DDecomposition(Primitive2DContainer());
}

const Primitive2DContainer& BufferedDecompositionPrimitive2D::getBuffered2DDecomposition() const
{
    if (0 != maCallbackSeconds && maCallbackTimer.is())
    {
        // decomposition was used, touch
        maCallbackTimer->setRemainingTime(salhelper::TTimeValue(maCallbackSeconds, 0));
    }

    return maBuffered2DDecomposition;
}

void BufferedDecompositionPrimitive2D::setBuffered2DDecomposition(Primitive2DContainer&& rNew)
{
    if (0 != maCallbackSeconds)
    {
        if (maCallbackTimer.is())
        {
            if (rNew.empty())
            {
                // no more decomposition, end callback
                maCallbackTimer->stop();
                maCallbackTimer.clear();
            }
            else
            {
                // decomposition changed, touch
                maCallbackTimer->setRemainingTime(salhelper::TTimeValue(maCallbackSeconds, 0));
            }
        }
        else if (!rNew.empty())
        {
            // decomposition changed, start callback
            maCallbackTimer.set(new LocalCallbackTimer(*this));
            maCallbackTimer->setRemainingTime(salhelper::TTimeValue(maCallbackSeconds, 0));
            maCallbackTimer->start();
        }
    }

    maBuffered2DDecomposition = std::move(rNew);
}

BufferedDecompositionPrimitive2D::BufferedDecompositionPrimitive2D()
    : maBuffered2DDecomposition()
    , maCallbackTimer()
    , maCallbackSeconds(0)
    , mnTransparenceForShadow(0)
{
}

BufferedDecompositionPrimitive2D::~BufferedDecompositionPrimitive2D()
{
    if (maCallbackTimer.is())
    {
        // no more decomposition, end callback
        maCallbackTimer->stop();
        maCallbackTimer.clear();
    }
}

void BufferedDecompositionPrimitive2D::get2DDecomposition(
    Primitive2DDecompositionVisitor& rVisitor,
    const geometry::ViewInformation2D& rViewInformation) const
{
    if (getBuffered2DDecomposition().empty())
    {
        Primitive2DContainer aNewSequence;
        create2DDecomposition(aNewSequence, rViewInformation);
        const_cast<BufferedDecompositionPrimitive2D*>(this)->setBuffered2DDecomposition(
            std::move(aNewSequence));
    }

    rVisitor.visit(getBuffered2DDecomposition());
}

} // end of namespace drawinglayer::primitive2d

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
