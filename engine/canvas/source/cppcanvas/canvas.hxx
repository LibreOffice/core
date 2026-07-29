/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <sal/types.h>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/rendering/ViewState.hpp>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <memory>
#include <optional>

namespace basegfx
{
    class B2DHomMatrix;
}

namespace vclcanvas
{
    class  XCanvas;
}

namespace com::sun::star::uno { template <class interface_type> class Reference; }

/* Definition of BitmapCanvas */

namespace cppcanvas
{
    class Canvas
    {
    public:
        /** Extra pixel used when canvas anti-aliases.

            Enlarge the bounding box of drawing primitives by this
            amount in both dimensions, and on both sides of the
            bounds, to account for extra pixel touched outside the
            actual primitive bounding box, when the canvas
            performs anti-aliasing.
         */
        static constexpr auto ANTIALIASING_EXTRA_SIZE=2;

        explicit Canvas( css::uno::Reference< vclcanvas::XCanvas > xCanvas );
        ~Canvas();

        Canvas(Canvas const &) = default;
        Canvas(Canvas &&) = default;
        Canvas & operator =(Canvas const &) = delete; // due to const mxCanvas
        Canvas & operator =(Canvas &&) = delete; // due to const mxCanvas

        void setTransformation( const ::basegfx::B2DHomMatrix& rMatrix );

        const css::uno::Reference< vclcanvas::XCanvas > & getUNOCanvas() const { return mxCanvas; }

        const css::rendering::ViewState & getViewState() const { return maViewState; }

    private:
        mutable css::rendering::ViewState                    maViewState;
        const css::uno::Reference< vclcanvas::XCanvas > mxCanvas;
    };

    typedef std::shared_ptr< Canvas > CanvasSharedPtr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
