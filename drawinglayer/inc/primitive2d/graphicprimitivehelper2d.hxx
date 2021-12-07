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

#pragma once

#include <drawinglayer/primitive2d/baseprimitive2d.hxx>
#include <vcl/GraphicAttributes.hxx>

class Graphic;

namespace drawinglayer::primitive2d
{
        /** Helper method with supports decomposing a Graphic with all
            possible contents to lower level primitives.

            #i121194# Unified to use this helper for FillGraphicPrimitive2D
            and GraphicPrimitive2D at the same time. It is able to handle
            Bitmaps (with the sub-categories animated bitmap, and SVG),
            and Metafiles.
         */
        void create2DDecompositionOfGraphic(
            Primitive2DContainer& rContainer,
            const Graphic& rGraphic,
            const basegfx::B2DHomMatrix& rTransform);

        /** Helper to embed given sequence of primitives to evtl. a stack
            of ModifiedColorPrimitive2D's to get all the needed modifications
            applied.
        */
        Primitive2DContainer create2DColorModifierEmbeddingsAsNeeded(
            Primitive2DContainer&& rChildren,
            GraphicDrawMode aGraphicDrawMode,
            double fLuminance = 0.0,        // [-1.0 .. 1.0]
            double fContrast = 0.0,         // [-1.0 .. 1.0]
            double fRed = 0.0,              // [-1.0 .. 1.0]
            double fGreen = 0.0,            // [-1.0 .. 1.0]
            double fBlue = 0.0,             // [-1.0 .. 1.0]
            double fGamma = 1.0,            // ]0.0 .. 10.0]
            bool bInvert = false);

} // end of namespace drawinglayer::primitive2d

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
