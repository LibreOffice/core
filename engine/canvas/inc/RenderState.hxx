/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
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

#include <com/sun/star/geometry/AffineMatrix2D.hpp>
#include <com/sun/star/rendering/ColorComponent.hpp>
#include <com/sun/star/rendering/XPolyPolygon2D.hpp>
#include <cpo/uno/Sequence.hxx>

namespace vclcanvas {

/** This structure contains information passed to each
    XCanvas render operation.<p>

    This structure contains information considered as the render
    state, i.e. the common setup required to render each individual
    XCanvas primitive.<p>
 */
struct RenderState
{
    /** The affine transform associated with this render
        operation.<p>

        This is used to transform coordinates of canvas primitives
        from user space to view space (from which they are
        subsequently transformed to device space by the view
        transform).<p>
     */
    ::com::sun::star::geometry::AffineMatrix2D  AffineTransform;


    /** The clipping area associated with this render operation.<p>

        This clipping is interpreted in the user coordinate system,
        i.e. subject to the render state transform followed by the
        view transform before mapped to device coordinate space.<p>

        Specifying an empty interface denotes no clipping,
        i.e. everything rendered to the canvas will be visible
        (subject to device-dependent constraints, of
        course). Specifying an empty XPolyPolygon2D, i.e. a
        poly-polygon containing zero polygons, or an XPolyPolygon2D
        with any number of empty sub-polygons, denotes the NULL
        clip. That means, nothing rendered to the canvas will be
        visible.<p>
     */
    css::uno::Reference<css::rendering::XPolyPolygon2D> Clip;


    /** The device color associated with this render operation.<p>

        Note that this need not be RGB here, but depends on the active
        device color space.<p>

        @see XGraphicDevice
     */
    cpo::uno::Sequence<css::rendering::ColorComponent> DeviceColor;
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
