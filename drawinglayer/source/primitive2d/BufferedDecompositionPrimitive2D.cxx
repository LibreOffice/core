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
#include <drawinglayer/primitive2d/BufferedDecompositionFlusher.hxx>

namespace drawinglayer::primitive2d
{
bool BufferedDecompositionPrimitive2D::hasBuffered2DDecomposition() const
{
    if (!mbFlushOnTimer)
        return maBuffered2DDecomposition.is();
    else
    {
        std::lock_guard Guard(maCallbackLock);
        return maBuffered2DDecomposition.is();
    }
}

void BufferedDecompositionPrimitive2D::setBuffered2DDecomposition(Primitive2DReference rNew)
{
    if (!mbFlushOnTimer)
    {
        // no flush used, just set
        maBuffered2DDecomposition = std::move(rNew);
    }
    else
    {
        // decomposition changed, touch
        maLastAccess = std::chrono::steady_clock::now();
        BufferedDecompositionFlusher::update(this);

        // tdf#158913 need to secure change when flush/multithreading is in use
        std::lock_guard Guard(maCallbackLock);
        maBuffered2DDecomposition = std::move(rNew);
    }
}

BufferedDecompositionPrimitive2D::BufferedDecompositionPrimitive2D()
    : maBuffered2DDecomposition()
    , maCallbackLock()
    , mbFlushOnTimer(false)
{
}

BufferedDecompositionPrimitive2D::~BufferedDecompositionPrimitive2D() {}

void BufferedDecompositionPrimitive2D::get2DDecomposition(
    Primitive2DDecompositionVisitor& rVisitor,
    const geometry::ViewInformation2D& rViewInformation) const
{
    if (!mbFlushOnTimer)
    {
        // no flush/multithreading is in use, just call
        if (!maBuffered2DDecomposition)
            maBuffered2DDecomposition = create2DDecomposition(rViewInformation);
        rVisitor.visit(maBuffered2DDecomposition);
    }
    else
    {
        // tdf#158913 need to secure 'visit' when flush/multithreading is in use,
        // so that the local non-ref-Counted instance of the decomposition gets not
        // manipulated (e.g. deleted)
        Primitive2DReference xTmp;
        {
            // only hold the lock for long enough to get a valid reference
            std::lock_guard Guard(maCallbackLock);
            maLastAccess = std::chrono::steady_clock::now();
            if (!maBuffered2DDecomposition)
            {
                maBuffered2DDecomposition = create2DDecomposition(rViewInformation);
                BufferedDecompositionFlusher::update(this);
            }
            xTmp = maBuffered2DDecomposition;
        }
        rVisitor.visit(xTmp);
    }
}

} // end of namespace drawinglayer::primitive2d

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
