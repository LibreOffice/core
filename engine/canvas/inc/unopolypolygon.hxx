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

#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/geometry/RealBezierSegment2D.hpp>
#include <com/sun/star/geometry/RealPoint2D.hpp>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <canvas/canvastoolsdllapi.h>
#include <o3tl/safeint.hxx>
#include <salhelper/simplereferenceobject.hxx>

namespace canvastools
{
    /** Generic interface for poly-polygons in 2D.
     */
    class CANVASTOOLS_DLLPUBLIC UnoPolyPolygon final
        : public salhelper::SimpleReferenceObject
    {
    public:
        UnoPolyPolygon( basegfx::B2DPolyPolygon );

        SAL_DLLPRIVATE basegfx::B2DPolyPolygon getPolyPolygon() const;

    private:
        UnoPolyPolygon(const UnoPolyPolygon&) = delete;
        UnoPolyPolygon& operator=(const UnoPolyPolygon&) = delete;

        basegfx::B2DPolyPolygon               maPolyPoly;
    };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
