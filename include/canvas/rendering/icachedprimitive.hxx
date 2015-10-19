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

#ifndef INCLUDED_CANVAS_RENDERING_ICACHEDPRIMITIVE_HXX
#define INCLUDED_CANVAS_RENDERING_ICACHEDPRIMITIVE_HXX

#include <sal/types.h>

#include <memory>

namespace basegfx
{
    class B2IRange;
    class B2IPoint;
}

namespace canvas
{
    /** Objects with this interface are returned from every Bitmap
        render operation.

        These objects can be used to implement the
        rendering::XCachedPrimitive interface, which in turn caches
        render state and objects to facilitate quick redraws.
     */
    struct ICachedPrimitive
    {
        virtual ~ICachedPrimitive() {}

        /** Redraw the primitive with the given view state

            Note that the primitive will <em>always</em> be redrawn on
            the bitmap it was created from.
         */
        virtual sal_Int8 redraw( const css::rendering::ViewState& aState ) const = 0;
    };

    typedef std::shared_ptr< ICachedPrimitive > ICachedPrimitiveSharedPtr;
}

#endif // INCLUDED_CANVAS_RENDERING_ICACHEDPRIMITIVE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
