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
#include <tools/poly.hxx>
#include <svx/svdpage.hxx>
#include "svx/globl3d.hxx"
#include <svx/lathe3d.hxx>
#include <svx/xpoly.hxx>
#include <svx/svxids.hrc>
#include <svx/svdopath.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svx3ditems.hxx>
#include <svx/sdr/properties/e3dlatheproperties.hxx>
#include <svx/sdr/contact/viewcontactofe3dlathe.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>

//////////////////////////////////////////////////////////////////////////////
// DrawContact section

sdr::contact::ViewContact* E3dLatheObj::CreateObjectSpecificViewContact()
{
    return new sdr::contact::ViewContactOfE3dLathe(*this);
}

//////////////////////////////////////////////////////////////////////////////

sdr::properties::BaseProperties* E3dLatheObj::CreateObjectSpecificProperties()
{
    return new sdr::properties::E3dLatheProperties(*this);
}

//////////////////////////////////////////////////////////////////////////////

TYPEINIT1(E3dLatheObj, E3dCompoundObject);

// Constructor from 3D polygon, scale is the conversion factor for the coordinates

E3dLatheObj::E3dLatheObj(E3dDefaultAttributes& rDefault, const basegfx::B2DPolyPolygon rPoly2D)
:   E3dCompoundObject(rDefault),
    maPolyPoly2D(rPoly2D)
{
    // since the old class PolyPolygon3D did mirror the given PolyPolygons in Y, do the same here
    basegfx::B2DHomMatrix aMirrorY;
    aMirrorY.scale(1.0, -1.0);
    maPolyPoly2D.transform(aMirrorY);

    // Set Defaults
    SetDefaultAttributes(rDefault);

    // Superfluous items removed, in particular to prevent duplicate
    // start and end points
    maPolyPoly2D.removeDoublePoints();

    if(maPolyPoly2D.count())
    {
        const basegfx::B2DPolygon rPoly(maPolyPoly2D.getB2DPolygon(0L));
        sal_uInt32 nSegCnt(rPoly.count());

        if(nSegCnt && !rPoly.isClosed())
        {
            nSegCnt -= 1;
        }

        GetProperties().SetObjectItemDirect(Svx3DVerticalSegmentsItem(nSegCnt));
    }
}

E3dLatheObj::E3dLatheObj()
:    E3dCompoundObject()
{
    // Set Defaults
    E3dDefaultAttributes aDefault;
    SetDefaultAttributes(aDefault);
}

void E3dLatheObj::SetDefaultAttributes(E3dDefaultAttributes& rDefault)
{
    GetProperties().SetObjectItemDirect(Svx3DSmoothNormalsItem(rDefault.GetDefaultLatheSmoothed()));
    GetProperties().SetObjectItemDirect(Svx3DSmoothLidsItem(rDefault.GetDefaultLatheSmoothFrontBack()));
    GetProperties().SetObjectItemDirect(Svx3DCharacterModeItem(rDefault.GetDefaultLatheCharacterMode()));
    GetProperties().SetObjectItemDirect(Svx3DCloseFrontItem(rDefault.GetDefaultLatheCloseFront()));
    GetProperties().SetObjectItemDirect(Svx3DCloseBackItem(rDefault.GetDefaultLatheCloseBack()));
}

sal_uInt16 E3dLatheObj::GetObjIdentifier() const
{
    return E3D_LATHEOBJ_ID;
}

E3dLatheObj* E3dLatheObj::Clone() const
{
    return CloneHelper< E3dLatheObj >();
}

// Convert the object to group object consisting of n polygons

SdrObject *E3dLatheObj::DoConvertToPolyObj(sal_Bool /*bBezier*/, bool /*bAddText*/) const
{
    return NULL;
}

// Set Local parameters set to re-create geometry

void E3dLatheObj::SetPolyPoly2D(const basegfx::B2DPolyPolygon& rNew)
{
    if(maPolyPoly2D != rNew)
    {
        maPolyPoly2D = rNew;
        maPolyPoly2D.removeDoublePoints();

        if(maPolyPoly2D.count())
        {
            const basegfx::B2DPolygon rPoly(maPolyPoly2D.getB2DPolygon(0L));
            sal_uInt32 nSegCnt(rPoly.count());

            if(nSegCnt && !rPoly.isClosed())
            {
                nSegCnt -= 1;
            }

            GetProperties().SetObjectItemDirect(Svx3DVerticalSegmentsItem(nSegCnt));
        }

        ActionChanged();
    }
}

// Get the name of the object (singular)

OUString E3dLatheObj::TakeObjNameSingul() const
{
    OUStringBuffer sName(ImpGetResStr(STR_ObjNameSingulLathe3d));

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

void E3dLatheObj::TakeObjNamePlural(XubString& rName) const
{
    rName=ImpGetResStr(STR_ObjNamePluralLathe3d);
}

bool E3dLatheObj::IsBreakObjPossible()
{
    return true;
}

SdrAttrObj* E3dLatheObj::GetBreakObj()
{
    // create PathObj
    basegfx::B3DPolyPolygon aLathePoly3D(basegfx::tools::createB3DPolyPolygonFromB2DPolyPolygon(maPolyPoly2D));
    basegfx::B2DPolyPolygon aTransPoly(TransformToScreenCoor(aLathePoly3D));
    SdrPathObj* pPathObj = new SdrPathObj(OBJ_PLIN, aTransPoly);

    if(pPathObj)
    {
        // Set Attribute
        SfxItemSet aSet(GetObjectItemSet());

        // Enable lines to guarantee that the object becomes visible
        aSet.Put(XLineStyleItem(XLINE_SOLID));

        pPathObj->SetMergedItemSet(aSet);
    }

    return pPathObj;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
