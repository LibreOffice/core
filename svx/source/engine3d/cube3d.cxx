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


#include <svx/strings.hrc>
#include <svx/dialmgr.hxx>
#include <svx/cube3d.hxx>
#include <svx/globl3d.hxx>
#include <basegfx/point/b3dpoint.hxx>
#include <basegfx/polygon/b3dpolygon.hxx>
#include <sdr/contact/viewcontactofe3dcube.hxx>


// DrawContact section

std::unique_ptr<sdr::contact::ViewContact> E3dCubeObj::CreateObjectSpecificViewContact()
{
    return std::make_unique<sdr::contact::ViewContactOfE3dCube>(*this);
}


E3dCubeObj::E3dCubeObj(
    SdrModel& rSdrModel,
    const E3dDefaultAttributes& rDefault,
    const basegfx::B3DPoint& aPos,
    const basegfx::B3DVector& r3DSize)
:   E3dCompoundObject(rSdrModel)
{
    // Set Defaults
    SetDefaultAttributes(rDefault);

    // position centre or left, bottom, back (dependent on bPosIsCenter)
    aCubePos = aPos;
    aCubeSize = r3DSize;
}

E3dCubeObj::E3dCubeObj(SdrModel& rSdrModel)
:   E3dCompoundObject(rSdrModel)
{
    // Set Defaults
    const E3dDefaultAttributes aDefault;

    SetDefaultAttributes(aDefault);
}

E3dCubeObj::~E3dCubeObj()
{
}

void E3dCubeObj::SetDefaultAttributes(const E3dDefaultAttributes& rDefault)
{
    aCubePos = rDefault.GetDefaultCubePos();
    aCubeSize = rDefault.GetDefaultCubeSize();
    bPosIsCenter = rDefault.GetDefaultCubePosIsCenter();
}

sal_uInt16 E3dCubeObj::GetObjIdentifier() const
{
    return E3D_CUBEOBJ_ID;
}

// Convert the object into a group object consisting of 6 polygons

SdrObject *E3dCubeObj::DoConvertToPolyObj(bool /*bBezier*/, bool /*bAddText*/) const
{
    return nullptr;
}

E3dCubeObj* E3dCubeObj::CloneSdrObject(SdrModel& rTargetModel) const
{
    return CloneHelper< E3dCubeObj >(rTargetModel);
}

E3dCubeObj& E3dCubeObj::operator=(const E3dCubeObj& rObj)
{
    if( this == &rObj )
        return *this;
    E3dCompoundObject::operator=(rObj);

    aCubePos = rObj.aCubePos;
    aCubeSize = rObj.aCubeSize;
    bPosIsCenter = rObj.bPosIsCenter;

    return *this;
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

void E3dCubeObj::SetPosIsCenter(bool bNew)
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
    OUStringBuffer sName(SvxResId(STR_ObjNameSingulCube3d));

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
    return SvxResId(STR_ObjNamePluralCube3d);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
