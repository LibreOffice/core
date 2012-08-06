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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
