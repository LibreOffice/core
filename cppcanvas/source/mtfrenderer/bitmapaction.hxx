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

#include <cppcanvas/canvas.hxx>
#include <action.hxx>

namespace basegfx {
    class B2DPoint;
    class B2DVector;
}
class BitmapEx;

/* Definition of internal::BitmapActionFactory */

namespace cppcanvas::internal
{
        struct OutDevState;

        /** Creates encapsulated converters between GDIMetaFile and
            XCanvas. The Canvas argument is deliberately placed at the
            constructor, to force reconstruction of this object for a
            new canvas. This considerably eases internal state
            handling, since a lot of the internal state (e.g. fonts,
            text layout) is Canvas-dependent.
         */
        namespace BitmapActionFactory
        {
            /// Unscaled bitmap action, only references destination point
            std::shared_ptr<Action> createBitmapAction( const ::Bitmap&,
                                                       const ::basegfx::B2DPoint& rDstPoint,
                                                       const CanvasSharedPtr&,
                                                       const OutDevState& );

            /// Scaled bitmap action, dest point and dest size
            std::shared_ptr<Action> createBitmapAction( const ::Bitmap&,
                                                       const ::basegfx::B2DPoint&  rDstPoint,
                                                       const ::basegfx::B2DVector& rDstSize,
                                                       const CanvasSharedPtr&,
                                                       const OutDevState& );
        }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
