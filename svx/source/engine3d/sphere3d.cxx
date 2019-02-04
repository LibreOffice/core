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
#include <svx/svdmodel.hxx>
#include <svx/svdpage.hxx>
#include <svx/globl3d.hxx>
#include <svx/sphere3d.hxx>

#include <svx/svxids.hrc>
#include <svx/svx3ditems.hxx>
#include <sdr/properties/e3dsphereproperties.hxx>
#include <basegfx/vector/b3dvector.hxx>
#include <basegfx/point/b3dpoint.hxx>
#include <sdr/contact/viewcontactofe3dsphere.hxx>
#include <basegfx/polygon/b3dpolygon.hxx>

// DrawContact section
std::unique_ptr<sdr::contact::ViewContact> E3dSphereObj::CreateObjectSpecificViewContact()
{
    return std::make_unique<sdr::contact::ViewContactOfE3dSphere>(*this);
}

std::unique_ptr<sdr::properties::BaseProperties> E3dSphereObj::CreateObjectSpecificProperties()
{
    return std::make_unique<sdr::properties::E3dSphereProperties>(*this);
}

// Build Sphere from polygon facets in latitude and longitude
E3dSphereObj::E3dSphereObj(
    SdrModel& rSdrModel,
    const E3dDefaultAttributes& rDefault,
    const basegfx::B3DPoint& rCenter,
    const basegfx::B3DVector& r3DSize)
:   E3dCompoundObject(rSdrModel)
{
    // Set defaults
    SetDefaultAttributes(rDefault);

    aCenter = rCenter;
    aSize = r3DSize;
}

E3dSphereObj::E3dSphereObj(SdrModel& rSdrModel)
:   E3dCompoundObject(rSdrModel)
{
    // Set defaults
    const E3dDefaultAttributes aDefault;

    SetDefaultAttributes(aDefault);
}

E3dSphereObj::~E3dSphereObj()
{
}

void E3dSphereObj::SetDefaultAttributes(const E3dDefaultAttributes& rDefault)
{
    // Set defaults
    aCenter = rDefault.GetDefaultSphereCenter();
    aSize = rDefault.GetDefaultSphereSize();
}

sal_uInt16 E3dSphereObj::GetObjIdentifier() const
{
    return E3D_SPHEREOBJ_ID;
}

// Convert the object into a group object consisting of n polygons

SdrObject *E3dSphereObj::DoConvertToPolyObj(bool /*bBezier*/, bool /*bAddText*/) const
{
    return nullptr;
}

E3dSphereObj* E3dSphereObj::CloneSdrObject(SdrModel& rTargetModel) const
{
    return CloneHelper< E3dSphereObj >(rTargetModel);
}

E3dSphereObj& E3dSphereObj::operator=(const E3dSphereObj& rObj)
{
    if( this == &rObj )
        return *this;
    E3dCompoundObject::operator=(rObj);

    aCenter = rObj.aCenter;
    aSize = rObj.aSize;

    return *this;
}

// Set local parameters with geometry re-creating

void E3dSphereObj::SetCenter(const basegfx::B3DPoint& rNew)
{
    if(aCenter != rNew)
    {
        aCenter = rNew;
        ActionChanged();
    }
}

void E3dSphereObj::SetSize(const basegfx::B3DVector& rNew)
{
    if(aSize != rNew)
    {
        aSize = rNew;
        ActionChanged();
    }
}

// Get the name of the object (singular)

OUString E3dSphereObj::TakeObjNameSingul() const
{
    OUStringBuffer sName(SvxResId(STR_ObjNameSingulSphere3d));

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

OUString E3dSphereObj::TakeObjNamePlural() const
{
    return SvxResId(STR_ObjNamePluralSphere3d);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
