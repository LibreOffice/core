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

#ifndef _E3D_SCENEUPDATER_HXX
#define _E3D_SCENEUPDATER_HXX

#include <svx/svxdllapi.h>

//////////////////////////////////////////////////////////////////////////////
// predeclarations

class SdrObject;
class E3dScene;
namespace drawinglayer { namespace geometry {
    class ViewInformation3D;
}}

//////////////////////////////////////////////////////////////////////////////
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

#endif          // _E3D_SCENEUPDATER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
