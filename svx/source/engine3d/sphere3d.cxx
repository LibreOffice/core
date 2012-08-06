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

//////////////////////////////////////////////////////////////////////////////
// DrawContact section

sdr::contact::ViewContact* E3dSphereObj::CreateObjectSpecificViewContact()
{
    return new sdr::contact::ViewContactOfE3dSphere(*this);
}

//////////////////////////////////////////////////////////////////////////////

sdr::properties::BaseProperties* E3dSphereObj::CreateObjectSpecificProperties()
{
    return new sdr::properties::E3dSphereProperties(*this);
}

//////////////////////////////////////////////////////////////////////////////

TYPEINIT1(E3dSphereObj, E3dCompoundObject);


// Build Sphere from polygon facets in latitude and longitude

E3dSphereObj::E3dSphereObj(E3dDefaultAttributes& rDefault, const basegfx::B3DPoint& rCenter, const basegfx::B3DVector& r3DSize)
:   E3dCompoundObject(rDefault)
{
    // Set defaults
    SetDefaultAttributes(rDefault);

    aCenter = rCenter;
    aSize = r3DSize;
}

// Create Sphere without creating the Polygons within

// This call is from the 3D Object Factory (objfac3d.cxx) and only when loading
// of documents. Here you do not need CreateSphere call, since the real number
// of segments is not even known yet. This was until 10.02.1997 a (small)
// memory leak.

E3dSphereObj::E3dSphereObj(int /*dummy*/)
// the parameters it needs to be able to distinguish which
// constructors of the two is meant. The above is the default.
{
    // Set defaults
    E3dDefaultAttributes aDefault;
    SetDefaultAttributes(aDefault);
}

void E3dSphereObj::SetDefaultAttributes(E3dDefaultAttributes& rDefault)
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

SdrObject *E3dSphereObj::DoConvertToPolyObj(sal_Bool /*bBezier*/) const
{
    return NULL;
}

E3dSphereObj* E3dSphereObj::Clone() const
{
    return CloneHelper< E3dSphereObj >();
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

void E3dSphereObj::TakeObjNameSingul(XubString& rName) const
{
    rName=ImpGetResStr(STR_ObjNameSingulSphere3d);

    String aName( GetName() );
    if(aName.Len())
    {
        rName += sal_Unicode(' ');
        rName += sal_Unicode('\'');
        rName += aName;
        rName += sal_Unicode('\'');
    }
}

// Get the name of the object (plural)

void E3dSphereObj::TakeObjNamePlural(XubString& rName) const
{
    rName=ImpGetResStr(STR_ObjNamePluralSphere3d);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
