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
#include <svx/svdpage.hxx>
#include <svx/globl3d.hxx>
#include <svx/extrud3d.hxx>
#include <svx/scene3d.hxx>

#include <svx/svxids.hrc>
#include <svx/xpoly.hxx>
#include <svx/svdopath.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svx3ditems.hxx>
#include <sdr/properties/e3dextrudeproperties.hxx>
#include <sdr/contact/viewcontactofe3dextrude.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b3dpolygontools.hxx>
#include <basegfx/polygon/b3dpolypolygontools.hxx>


// DrawContact section
std::unique_ptr<sdr::contact::ViewContact> E3dExtrudeObj::CreateObjectSpecificViewContact()
{
    return std::make_unique<sdr::contact::ViewContactOfE3dExtrude>(*this);
}

std::unique_ptr<sdr::properties::BaseProperties> E3dExtrudeObj::CreateObjectSpecificProperties()
{
    return std::make_unique<sdr::properties::E3dExtrudeProperties>(*this);
}

// Constructor creates a two cover surface tools::PolyPolygon and (point-count 1) side
// surfaces rectangles from the passed PolyPolygon
E3dExtrudeObj::E3dExtrudeObj(
    SdrModel& rSdrModel,
    const E3dDefaultAttributes& rDefault,
    const basegfx::B2DPolyPolygon& rPP,
    double fDepth)
:   E3dCompoundObject(rSdrModel),
    maExtrudePolygon(rPP)
{
    // since the old class PolyPolygon3D did mirror the given PolyPolygons in Y, do the same here
    basegfx::B2DHomMatrix aMirrorY;
    aMirrorY.scale(1.0, -1.0);
    maExtrudePolygon.transform(aMirrorY);

    // Set Defaults
    SetDefaultAttributes(rDefault);

    // set extrude depth
    GetProperties().SetObjectItemDirect(makeSvx3DDepthItem(static_cast<sal_uInt32>(fDepth + 0.5)));
}

E3dExtrudeObj::E3dExtrudeObj(SdrModel& rSdrModel)
:   E3dCompoundObject(rSdrModel)
{
    // Set Defaults
    const E3dDefaultAttributes aDefault;

    SetDefaultAttributes(aDefault);
}

E3dExtrudeObj::~E3dExtrudeObj()
{
}

void E3dExtrudeObj::SetDefaultAttributes(const E3dDefaultAttributes& rDefault)
{
    GetProperties().SetObjectItemDirect(Svx3DSmoothNormalsItem(rDefault.GetDefaultExtrudeSmoothed()));
    GetProperties().SetObjectItemDirect(Svx3DSmoothLidsItem(rDefault.GetDefaultExtrudeSmoothFrontBack()));
    GetProperties().SetObjectItemDirect(Svx3DCharacterModeItem(rDefault.GetDefaultExtrudeCharacterMode()));
    GetProperties().SetObjectItemDirect(Svx3DCloseFrontItem(rDefault.GetDefaultExtrudeCloseFront()));
    GetProperties().SetObjectItemDirect(Svx3DCloseBackItem(rDefault.GetDefaultExtrudeCloseBack()));

    // For extrudes use StdTexture in X and Y by default
    GetProperties().SetObjectItemDirect(Svx3DTextureProjectionXItem(1));
    GetProperties().SetObjectItemDirect(Svx3DTextureProjectionYItem(1));
}

sal_uInt16 E3dExtrudeObj::GetObjIdentifier() const
{
    return E3D_EXTRUDEOBJ_ID;
}

E3dExtrudeObj* E3dExtrudeObj::CloneSdrObject(SdrModel& rTargetModel) const
{
    return CloneHelper< E3dExtrudeObj >(rTargetModel);
}

E3dExtrudeObj& E3dExtrudeObj::operator=(const E3dExtrudeObj& rObj)
{
    if( this == &rObj )
        return *this;
    E3dCompoundObject::operator=(rObj);

    maExtrudePolygon = rObj.maExtrudePolygon;

    return *this;
}

// Set local parameters with geometry re-creating

void E3dExtrudeObj::SetExtrudePolygon(const basegfx::B2DPolyPolygon &rNew)
{
    if(maExtrudePolygon != rNew)
    {
        maExtrudePolygon = rNew;
        ActionChanged();
    }
}

// Get the name of the object (singular)

OUString E3dExtrudeObj::TakeObjNameSingul() const
{
    OUStringBuffer sName(SvxResId(STR_ObjNameSingulExtrude3d));

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

OUString E3dExtrudeObj::TakeObjNamePlural() const
{
    return SvxResId(STR_ObjNamePluralExtrude3d);
}

bool E3dExtrudeObj::IsBreakObjPossible()
{
    return true;
}

std::unique_ptr<SdrAttrObj,SdrObjectFreeOp> E3dExtrudeObj::GetBreakObj()
{
    basegfx::B3DPolyPolygon aFrontSide;
    basegfx::B3DPolyPolygon aBackSide;

    if(maExtrudePolygon.count())
    {
        basegfx::B2DPolyPolygon aTemp(maExtrudePolygon);
        aTemp.removeDoublePoints();
        aTemp = basegfx::utils::correctOrientations(aTemp);
        const basegfx::B2VectorOrientation aOrient = basegfx::utils::getOrientation(aTemp.getB2DPolygon(0));

        if(basegfx::B2VectorOrientation::Positive == aOrient)
        {
            aTemp.flip();
        }

        aFrontSide = basegfx::utils::createB3DPolyPolygonFromB2DPolyPolygon(aTemp);
    }

    if(aFrontSide.count())
    {
        aBackSide = aFrontSide;

        if(GetExtrudeDepth())
        {
            basegfx::B3DHomMatrix aTransform;

            if(100 != GetPercentBackScale())
            {
                // scale polygon from center
                const double fScaleFactor(GetPercentBackScale() / 100.0);
                const basegfx::B3DRange aPolyPolyRange(basegfx::utils::getRange(aBackSide));
                const basegfx::B3DPoint aCenter(aPolyPolyRange.getCenter());

                aTransform.translate(-aCenter.getX(), -aCenter.getY(), -aCenter.getZ());
                aTransform.scale(fScaleFactor, fScaleFactor, fScaleFactor);
                aTransform.translate(aCenter.getX(), aCenter.getY(), aCenter.getZ());
            }

            // translate by extrude depth
            aTransform.translate(0.0, 0.0, static_cast<double>(GetExtrudeDepth()));

            aBackSide.transform(aTransform);
        }
    }

    if(aBackSide.count())
    {
    // create PathObj
        basegfx::B2DPolyPolygon aPoly = TransformToScreenCoor(aBackSide);
        std::unique_ptr<SdrPathObj,SdrObjectFreeOp> pPathObj(new SdrPathObj(getSdrModelFromSdrObject(), OBJ_PLIN, aPoly));

        SfxItemSet aSet(GetObjectItemSet());
        aSet.Put(XLineStyleItem(css::drawing::LineStyle_SOLID));
        pPathObj->SetMergedItemSet(aSet);

        return std::unique_ptr<SdrAttrObj,SdrObjectFreeOp>(pPathObj.release());
    }

    return nullptr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
