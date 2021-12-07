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
#include <drawinglayer/primitive2d/Tools.hxx>
#include <drawinglayer/geometry/viewinformation2d.hxx>
#include <basegfx/utils/canvastools.hxx>
#include <comphelper/sequence.hxx>

namespace drawinglayer::primitive2d
{
BufferedDecompositionPrimitive2D::BufferedDecompositionPrimitive2D() {}

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
