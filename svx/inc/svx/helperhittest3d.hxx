/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef INCLUDED_SVX_HELPERHITTEST_HXX
#define INCLUDED_SVX_HELPERHITTEST_HXX

#include "svx/svxdllapi.h"
#include <sal/types.h>
#include <vector>

//////////////////////////////////////////////////////////////////////////////
// predefines

namespace basegfx {
    class B2DPoint;
}

namespace drawinglayer { namespace geometry {
    class ViewInformation3D;
}}

class E3dCompoundObject;
class E3dScene;

//////////////////////////////////////////////////////////////////////////////

/** support for getting a ViewInformation3D for a given CompoudObject3D
    with correct ObjectTransformation filled out

    @param o_rViewInformation3D
    The ViewInformation3D created which will be overwritten

    @param rCandidate
    The E3dCompoundObject for which the ViewInformation3D shall be
    prepared

    @return
    A pointer to the found and used root scene. This is also a hint
    if the operation succeeded or not, since when object has no root
    scene (is not inserted to a model), an empty ViewInformation3D
    will be used
*/
E3dScene* fillViewInformation3DForCompoundObject(
    drawinglayer::geometry::ViewInformation3D& o_rViewInformation3D,
    const E3dCompoundObject& rCandidate);

/** support for getting all from a 2d position hit objects in a 3d scene
    in a depth sorted array

    @param rPoint
    2D Point in view coordinates

    @param rScene
    The 3D Scene for HitTest

    @param o_rResult
    Output parameter which contains all hit 3D objects inside rScene. This
    vector will be changed in any case. If it's empty, no hit exists. If it's
    not empty, the first object is the object closest to the viewer

*/
SVX_DLLPUBLIC void getAllHit3DObjectsSortedFrontToBack(
    const basegfx::B2DPoint& rPoint,
    const E3dScene& rScene,
    ::std::vector< const E3dCompoundObject* >& o_rResult);

/** support for checking if the single given 3d object is hit at position

    @param rPoint
    2D Point in view coordinates

    @param rCandidate
    The 3D Object which needs checking

    @return
    true if hit, false if not
*/
bool checkHitSingle3DObject(
    const basegfx::B2DPoint& rPoint,
    const E3dCompoundObject& rCandidate);

//////////////////////////////////////////////////////////////////////////////

#endif // INCLUDED_SVX_HELPERHITTEST_HXX

// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
