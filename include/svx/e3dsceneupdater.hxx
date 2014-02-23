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

#ifndef INCLUDED_SVX_E3DSCENEUPDATER_HXX
#define INCLUDED_SVX_E3DSCENEUPDATER_HXX

#include <svx/svxdllapi.h>


// predeclarations

class SdrObject;
class E3dScene;
namespace drawinglayer { namespace geometry {
    class ViewInformation3D;
}}


// This class is a helper to encapsulate 3D object changes which shall change the
// scene's 2D geometry. E.g. when moving one 3D object in a scene, the visualisation
// would not change since the scene's 2D attributes are defined by it's 2D object
// transformation and the changed content would be projected to the same 2D bounds
// as before. To reflect the fact that the 2D positions may 'change' for the user's
// imagination, it is necessary to calculate a new 2D object transformation of the scene
// (the SnapRect) using the old 3D transformation stack and the eventually changed
// 3D content and tu use it. This is only wanted if changes to the scene's
// content are intended to change the scene's 2D geometry attributes

class SVX_DLLPUBLIC E3DModifySceneSnapRectUpdater
{
    // the scene which may be changed. This gets set to the outmost scene
    // of the to-be-changed 3D object when the scene has a 3d transformation
    // stack at construction time. In all other cases it's set to zero and
    // no action needs to be taken
    E3dScene*                                   mpScene;

    // the 3d transformation stack at the time of construction, valid when
    // mpScene is not zero
    drawinglayer::geometry::ViewInformation3D*  mpViewInformation3D;

public:
    // the constructor evaluates and sets the members at construction time
    E3DModifySceneSnapRectUpdater(const SdrObject* pObject);

    // the destructor will take action if mpScene is not zero and modify the
    // 2D geomeztry of the target scene
    ~E3DModifySceneSnapRectUpdater();
};

#endif // INCLUDED_SVX_E3DSCENEUPDATER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
