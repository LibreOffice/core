/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */


#include "svx/svdstr.hrc"
#include "svx/svdglob.hxx"
#include <svx/svdmodel.hxx>
#include <svx/svdpage.hxx>
#include "svx/globl3d.hxx"
#include <svx/sphere3d.hxx>

#include <svx/svxids.hrc>
#include <svx/svx3ditems.hxx>
#include <svx/sdr/properties/e3dsphereproperties.hxx>
#include <basegfx/vector/b3dvector.hxx>
#include <basegfx/point/b3dpoint.hxx>
#include <svx/sdr/contact/viewcontactofe3dsphere.hxx>
#include <basegfx/polygon/b3dpolygon.hxx>




sdr::contact::ViewContact* E3dSphereObj::CreateObjectSpecificViewContact()
{
    return new sdr::contact::ViewContactOfE3dSphere(*this);
}



sdr::properties::BaseProperties* E3dSphereObj::CreateObjectSpecificProperties()
{
    return new sdr::properties::E3dSphereProperties(*this);
}



TYPEINIT1(E3dSphereObj, E3dCompoundObject);




E3dSphereObj::E3dSphereObj(E3dDefaultAttributes& rDefault, const basegfx::B3DPoint& rCenter, const basegfx::B3DVector& r3DSize)
:   E3dCompoundObject(rDefault)
{
    
    SetDefaultAttributes(rDefault);

    aCenter = rCenter;
    aSize = r3DSize;
}








E3dSphereObj::E3dSphereObj(int /*dummy*/)


{
    
    E3dDefaultAttributes aDefault;
    SetDefaultAttributes(aDefault);
}

void E3dSphereObj::SetDefaultAttributes(E3dDefaultAttributes& rDefault)
{
    
    aCenter = rDefault.GetDefaultSphereCenter();
    aSize = rDefault.GetDefaultSphereSize();
}

sal_uInt16 E3dSphereObj::GetObjIdentifier() const
{
    return E3D_SPHEREOBJ_ID;
}



SdrObject *E3dSphereObj::DoConvertToPolyObj(sal_Bool /*bBezier*/, bool /*bAddText*/) const
{
    return NULL;
}

E3dSphereObj* E3dSphereObj::Clone() const
{
    return CloneHelper< E3dSphereObj >();
}



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



OUString E3dSphereObj::TakeObjNameSingul() const
{
    OUStringBuffer sName(ImpGetResStr(STR_ObjNameSingulSphere3d));

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



OUString E3dSphereObj::TakeObjNamePlural() const
{
    return ImpGetResStr(STR_ObjNamePluralSphere3d);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
