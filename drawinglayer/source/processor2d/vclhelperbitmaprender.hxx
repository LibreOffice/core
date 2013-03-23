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
#if 1

#include <sal/types.h>

//////////////////////////////////////////////////////////////////////////////
// predefines

class OutputDevice;
class BitmapEx;
namespace basegfx { class B2DHomMatrix; }

//////////////////////////////////////////////////////////////////////////////
// support methods for vcl direct gradient renderering

namespace drawinglayer
{
    void RenderBitmapPrimitive2D_GraphicManager(
        OutputDevice& rOutDev,
        const BitmapEx& rBitmapEx,
        const basegfx::B2DHomMatrix& rTransform);

    void RenderBitmapPrimitive2D_BitmapEx(
        OutputDevice& rOutDev,
        const BitmapEx& rBitmapEx,
        const basegfx::B2DHomMatrix& rTransform);

    void RenderBitmapPrimitive2D_self(
        OutputDevice& rOutDev,
        const BitmapEx& rBitmapEx,
        const basegfx::B2DHomMatrix& rTransform);

} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif // INCLUDED_DRAWINGLAYER_PROCESSOR2D_VCLHELPERBITMAPRENDER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
