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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"
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

static sal_Bool bInit = sal_False;

E3dObjFactory::E3dObjFactory()
{
    if ( !bInit )
    {
        SdrObjFactory::InsertMakeObjectHdl(LINK(this, E3dObjFactory, MakeObject));
        bInit = sal_True;
    }
}

E3dObjFactory::~E3dObjFactory()
{
}

// Generate chart internal objects

IMPL_LINK( E3dObjFactory, MakeObject, SdrObjFactory*, pObjFactory)
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
