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

#include <svx/svdpage.hxx>
#include "svx/globl3d.hxx"
#include <svx/polysc3d.hxx>
#include <svx/cube3d.hxx>
#include <svx/sphere3d.hxx>
#include <svx/extrud3d.hxx>
#include <svx/lathe3d.hxx>
#include <svx/polygn3d.hxx>
#include "svx/objfac3d.hxx"
#include <svx/svdobj.hxx>

static bool bInit = false;

E3dObjFactory::E3dObjFactory()
{
    if ( !bInit )
    {
        SdrObjFactory::InsertMakeObjectHdl(LINK(this, E3dObjFactory, MakeObject));
        bInit = true;
    }
}

E3dObjFactory::~E3dObjFactory()
{
}

// Generate chart internal objects

IMPL_STATIC_LINK(
    E3dObjFactory, MakeObject, SdrObjFactory*, pObjFactory)
{
    if ( pObjFactory->nInventor == E3dInventor )
    {
        switch ( pObjFactory->nIdentifier )
        {
            case E3D_POLYSCENE_ID:
                pObjFactory->pNewObj = new E3dPolyScene();
                break;
            case E3D_POLYGONOBJ_ID  :
                pObjFactory->pNewObj = new E3dPolygonObj();
                break;
            case E3D_CUBEOBJ_ID :
                pObjFactory->pNewObj = new E3dCubeObj();
                break;
            case E3D_SPHEREOBJ_ID:
                // Gets the dummy constructor, as this is only called when
                // loading documents. The actual number of segments is however
                // determined only after loading the members. This will result
                // in that the first sphere will be immediately destroyed,
                // although it was never used.
                pObjFactory->pNewObj = new E3dSphereObj(123);
                break;
            case E3D_EXTRUDEOBJ_ID:
                pObjFactory->pNewObj = new E3dExtrudeObj();
                break;
            case E3D_LATHEOBJ_ID:
                pObjFactory->pNewObj = new E3dLatheObj();
                break;
            case E3D_COMPOUNDOBJ_ID:
                pObjFactory->pNewObj = new E3dCompoundObject();
                break;
        }
    }
    return 0;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
