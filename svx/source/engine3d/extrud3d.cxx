/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#include "svx/svdstr.hrc"
#include "svx/svdglob.hxx"
#include <svx/svdpage.hxx>
#include "svx/globl3d.hxx"
#include <svx/extrud3d.hxx>
#include <svx/scene3d.hxx>

#include <svx/svxids.hrc>
#include <svx/xpoly.hxx>
#include <svx/svdopath.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svx3ditems.hxx>
#include <svx/sdr/properties/e3dextrudeproperties.hxx>
#include <svx/sdr/contact/viewcontactofe3dextrude.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b3dpolygontools.hxx>
#include <basegfx/polygon/b3dpolypolygontools.hxx>

//////////////////////////////////////////////////////////////////////////////
// #110094# DrawContact section

sdr::contact::ViewContact* E3dExtrudeObj::CreateObjectSpecificViewContact()
{
    return new sdr::contact::ViewContactOfE3dExtrude(*this);
}

//////////////////////////////////////////////////////////////////////////////

sdr::properties::BaseProperties* E3dExtrudeObj::CreateObjectSpecificProperties()
{
    return new sdr::properties::E3dExtrudeProperties(*this);
}

//////////////////////////////////////////////////////////////////////////////

/*************************************************************************
|*
|* Konstruktor, erzeugt zwei Deckelflaechen-PolyPolygone und (PointCount-1)
|* Seitenflaechen-Rechtecke aus dem uebergebenen PolyPolygon
|*
\************************************************************************/

E3dExtrudeObj::E3dExtrudeObj(
    SdrModel& rSdrModel,
    const E3dDefaultAttributes& rDefault,
    const basegfx::B2DPolyPolygon aPP,
    double fDepth)
:   E3dCompoundObject(rSdrModel, rDefault),
    maExtrudePolygon(aPP)
{
    // since the old class PolyPolygon3D did mirror the given PolyPolygons in Y, do the same here
    basegfx::B2DHomMatrix aMirrorY;
    aMirrorY.scale(1.0, -1.0);
    maExtrudePolygon.transform(aMirrorY);

    // Defaults setzen
    SetDefaultAttributes(rDefault);

    // set extrude depth
    GetProperties().SetObjectItemDirect(SfxUInt32Item(SDRATTR_3DOBJ_DEPTH, (sal_uInt32)(fDepth + 0.5)));
}

E3dExtrudeObj::~E3dExtrudeObj()
{
}

void E3dExtrudeObj::copyDataFromSdrObject(const SdrObject& rSource)
{
    if(this != &rSource)
    {
        const E3dExtrudeObj* pSource = dynamic_cast< const E3dExtrudeObj* >(&rSource);

        if(pSource)
        {
            // call parent
            E3dCompoundObject::copyDataFromSdrObject(rSource);

            // copy local data
            maExtrudePolygon = pSource->maExtrudePolygon;
        }
        else
        {
            OSL_ENSURE(false, "copyDataFromSdrObject with ObjectType of Source different from Target (!)");
        }
    }
}

SdrObject* E3dExtrudeObj::CloneSdrObject(SdrModel* pTargetModel) const
{
    E3dExtrudeObj* pClone = new E3dExtrudeObj(
        pTargetModel ? *pTargetModel : getSdrModelFromSdrObject(),
        E3dDefaultAttributes());
    OSL_ENSURE(pClone, "CloneSdrObject error (!)");
    pClone->copyDataFromSdrObject(*this);

    return pClone;
}

void E3dExtrudeObj::SetDefaultAttributes(const E3dDefaultAttributes& rDefault)
{
    GetProperties().SetObjectItemDirect(Svx3DSmoothNormalsItem(rDefault.GetDefaultExtrudeSmoothed()));
    GetProperties().SetObjectItemDirect(Svx3DSmoothLidsItem(rDefault.GetDefaultExtrudeSmoothFrontBack()));
    GetProperties().SetObjectItemDirect(Svx3DCharacterModeItem(rDefault.GetDefaultExtrudeCharacterMode()));
    GetProperties().SetObjectItemDirect(Svx3DCloseFrontItem(rDefault.GetDefaultExtrudeCloseFront()));
    GetProperties().SetObjectItemDirect(Svx3DCloseBackItem(rDefault.GetDefaultExtrudeCloseBack()));

    // Bei extrudes defaultmaessig StdTexture in X und Y
    GetProperties().SetObjectItemDirect(Svx3DTextureProjectionXItem(1));
    GetProperties().SetObjectItemDirect(Svx3DTextureProjectionYItem(1));
}

/*************************************************************************
|*
|* Identifier zurueckgeben
|*
\************************************************************************/

sal_uInt16 E3dExtrudeObj::GetObjIdentifier() const
{
    return E3D_EXTRUDEOBJ_ID;
}

/*************************************************************************
|*
|* Lokale Parameter setzen mit Geometrieneuerzeugung
|*
\************************************************************************/

void E3dExtrudeObj::SetExtrudePolygon(const basegfx::B2DPolyPolygon &rNew)
{
    if(maExtrudePolygon != rNew)
    {
        maExtrudePolygon = rNew;
        ActionChanged();
    }
}

/*************************************************************************
|*
|* Get the name of the object (singular)
|*
\************************************************************************/

void E3dExtrudeObj::TakeObjNameSingul(XubString& rName) const
{
    rName=ImpGetResStr(STR_ObjNameSingulExtrude3d);

    String aName( GetName() );
    if(aName.Len())
    {
        rName += sal_Unicode(' ');
        rName += sal_Unicode('\'');
        rName += aName;
        rName += sal_Unicode('\'');
    }
}

/*************************************************************************
|*
|* Get the name of the object (plural)
|*
\************************************************************************/

void E3dExtrudeObj::TakeObjNamePlural(XubString& rName) const
{
    rName=ImpGetResStr(STR_ObjNamePluralExtrude3d);
}

/*************************************************************************
|*
|* Aufbrechen
|*
\************************************************************************/

sal_Bool E3dExtrudeObj::IsBreakObjPossible()
{
    return sal_True;
}

SdrAttrObj* E3dExtrudeObj::GetBreakObj()
{
    basegfx::B3DPolyPolygon aFrontSide;
    basegfx::B3DPolyPolygon aBackSide;

    if(maExtrudePolygon.count())
    {
        basegfx::B2DPolyPolygon aTemp(maExtrudePolygon);
        aTemp.removeDoublePoints();
        aTemp = basegfx::tools::correctOrientations(aTemp);
        const basegfx::B2VectorOrientation aOrient = basegfx::tools::getOrientation(aTemp.getB2DPolygon(0L));

        if(basegfx::ORIENTATION_POSITIVE == aOrient)
        {
            aTemp.flip();
        }

        aFrontSide = basegfx::tools::createB3DPolyPolygonFromB2DPolyPolygon(aTemp);
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
                const basegfx::B3DRange aPolyPolyRange(basegfx::tools::getRange(aBackSide));
                const basegfx::B3DPoint aCenter(aPolyPolyRange.getCenter());

                aTransform.translate(-aCenter);
                aTransform.scale(fScaleFactor, fScaleFactor, fScaleFactor);
                aTransform.translate(aCenter);
            }

            // translate by extrude depth
            aTransform.translate(0.0, 0.0, (double)GetExtrudeDepth());

            aBackSide.transform(aTransform);
        }
    }

    if(aBackSide.count())
    {
    // create PathObj
        basegfx::B2DPolyPolygon aPoly = TransformToScreenCoor(aBackSide);
        SdrPathObj* pPathObj = new SdrPathObj(
            getSdrModelFromSdrObject(),
            aPoly);

        if(pPathObj)
        {
            SfxItemSet aSet(GetObjectItemSet());
            aSet.Put(XLineStyleItem(XLINE_SOLID));
            pPathObj->SetMergedItemSet(aSet);
        }

        return pPathObj;
    }

    return 0;
}

// eof
