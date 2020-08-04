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

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/util/TriState.hpp>
#include <memory>

namespace Gdiplus { class Graphics; }

namespace dxcanvas::tools
{
        /** Raw RGBA bitmap data,
            contiguous in memory
        */
        struct RawRGBABitmap
        {
            sal_Int32                           mnWidth;
            sal_Int32                           mnHeight;
            std::vector<sal_uInt8>              maBitmapData;
        };

        bool drawVCLBitmapFromXBitmap( const std::shared_ptr< Gdiplus::Graphics >& rGraphics,
                                       const css::uno::Reference<
                                             css::rendering::XBitmap >&   xBitmap );
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
