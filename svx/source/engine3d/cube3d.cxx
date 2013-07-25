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


#include "svx/svdstr.hrc"
#include "svx/svdglob.hxx"
#include <svx/cube3d.hxx>
#include "svx/globl3d.hxx"
#include <basegfx/point/b3dpoint.hxx>
#include <basegfx/polygon/b3dpolygon.hxx>
#include <svx/sdr/contact/viewcontactofe3dcube.hxx>

//////////////////////////////////////////////////////////////////////////////
// DrawContact section

sdr::contact::ViewContact* E3dCubeObj::CreateObjectSpecificViewContact()
{
    return new sdr::contact::ViewContactOfE3dCube(*this);
}

TYPEINIT1(E3dCubeObj, E3dCompoundObject);

E3dCubeObj::E3dCubeObj(E3dDefaultAttributes& rDefault, basegfx::B3DPoint aPos, const basegfx::B3DVector& r3DSize)
:   E3dCompoundObject(rDefault)
{
    // Set Defaults
    SetDefaultAttributes(rDefault);

    aCubePos = aPos; // position centre or left, bottom, back (dependant on bPosIsCenter)
    aCubeSize = r3DSize;
}

E3dCubeObj::E3dCubeObj()
:   E3dCompoundObject()
{
    // Set Defaults
    E3dDefaultAttributes aDefault;
    SetDefaultAttributes(aDefault);
}

void E3dCubeObj::SetDefaultAttributes(E3dDefaultAttributes& rDefault)
{
    aCubePos = rDefault.GetDefaultCubePos();
    aCubeSize = rDefault.GetDefaultCubeSize();
    nSideFlags = rDefault.GetDefaultCubeSideFlags();
    bPosIsCenter = rDefault.GetDefaultCubePosIsCenter();
}

sal_uInt16 E3dCubeObj::GetObjIdentifier() const
{
    return E3D_CUBEOBJ_ID;
}

// Convert the object into a group object consisting of 6 polygons

SdrObject *E3dCubeObj::DoConvertToPolyObj(sal_Bool /*bBezier*/, bool /*bAddText*/) const
{
    return NULL;
}

E3dCubeObj* E3dCubeObj::Clone() const
{
    return CloneHelper< E3dCubeObj >();
}

// Set local parameters with geometry re-creating

void E3dCubeObj::SetCubePos(const basegfx::B3DPoint& rNew)
{
    if(aCubePos != rNew)
    {
        aCubePos = rNew;
        ActionChanged();
    }
}

void E3dCubeObj::SetCubeSize(const basegfx::B3DVector& rNew)
{
    if(aCubeSize != rNew)
    {
        aCubeSize = rNew;
        ActionChanged();
    }
}

void E3dCubeObj::SetPosIsCenter(sal_Bool bNew)
{
    if(bPosIsCenter != bNew)
    {
        bPosIsCenter = bNew;
        ActionChanged();
    }
}

// Get the name of the object (singular)

OUString E3dCubeObj::TakeObjNameSingul() const
{
    OUStringBuffer sName(ImpGetResStr(STR_ObjNameSingulCube3d));

    OUString aName(GetName());
    if (!aName.isEmpty())
    {
        sName.append(' ');
        sName.append('\'');
        sName.append(aName);
        sName.append('\'');
    }
    return sName.makeStringAndClear();
}

// Get the name of the object (plural)

OUString E3dCubeObj::TakeObjNamePlural() const
{
    return ImpGetResStr(STR_ObjNamePluralCube3d);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
