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

#include <com/sun/star/lang/IllegalArgumentException.hpp>

#include <basegfx/range/b2drange.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/utils/canvastools.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <canvastools.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <unopolypolygon.hxx>
#include <utility>

using namespace ::com::sun::star;

namespace canvastools
{
    UnoPolyPolygon::UnoPolyPolygon( basegfx::B2DPolyPolygon aPolyPoly ) :
        maPolyPoly(std::move( aPolyPoly ))
    {
    }

    basegfx::B2DPolyPolygon UnoPolyPolygon::getPolyPolygon() const
    {
        return maPolyPoly;
    }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
