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
// #110094# DrawContact section

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

/*************************************************************************
|*
|* Konstruktor aus 3D-Polygon, Scale gibt den Umrechnungsfaktor fuer
|* die Koordinaten an
|*
\************************************************************************/

E3dLatheObj::E3dLatheObj(
    SdrModel& rSdrModel,
    const E3dDefaultAttributes& rDefault,
    const basegfx::B2DPolyPolygon aPoly2D)
:   E3dCompoundObject(rSdrModel, rDefault),
    maPolyPoly2D(aPoly2D)
{
    // since the old class PolyPolygon3D did mirror the given PolyPolygons in Y, do the same here
    basegfx::B2DHomMatrix aMirrorY;
    aMirrorY.scale(1.0, -1.0);
    maPolyPoly2D.transform(aMirrorY);

    // Defaults setzen
    SetDefaultAttributes(rDefault);

    // Ueberfluessige Punkte entfernen, insbesondere doppelte
    // Start- und Endpunkte verhindern
    maPolyPoly2D.removeDoublePoints();

    if(maPolyPoly2D.count())
    {
        const basegfx::B2DPolygon rPoly(maPolyPoly2D.getB2DPolygon(0L));
        sal_uInt32 nSegCnt(rPoly.count());

        if(nSegCnt && !rPoly.isClosed())
        {
            nSegCnt -= 1;
        }

        GetProperties().SetObjectItemDirect(SfxUInt32Item(SDRATTR_3DOBJ_VERT_SEGS, nSegCnt));
    }
}

E3dLatheObj::~E3dLatheObj()
{
}

void E3dLatheObj::copyDataFromSdrObject(const SdrObject& rSource)
{
    if(this != &rSource)
    {
        const E3dLatheObj* pSource = dynamic_cast< const E3dLatheObj* >(&rSource);

        if(pSource)
        {
            // call parent
            E3dCompoundObject::copyDataFromSdrObject(rSource);

            // copy local data
            maPolyPoly2D  = pSource->maPolyPoly2D;
        }
        else
        {
            OSL_ENSURE(false, "copyDataFromSdrObject with ObjectType of Source different from Target (!)");
        }
    }
}

SdrObject* E3dLatheObj::CloneSdrObject(SdrModel* pTargetModel) const
{
    E3dLatheObj* pClone = new E3dLatheObj(
        pTargetModel ? *pTargetModel : getSdrModelFromSdrObject(),
        E3dDefaultAttributes());
    OSL_ENSURE(pClone, "CloneSdrObject error (!)");
    pClone->copyDataFromSdrObject(*this);

    return pClone;
}

void E3dLatheObj::SetDefaultAttributes(const E3dDefaultAttributes& rDefault)
{
    GetProperties().SetObjectItemDirect(Svx3DSmoothNormalsItem(rDefault.GetDefaultLatheSmoothed()));
    GetProperties().SetObjectItemDirect(Svx3DSmoothLidsItem(rDefault.GetDefaultLatheSmoothFrontBack()));
    GetProperties().SetObjectItemDirect(Svx3DCharacterModeItem(rDefault.GetDefaultLatheCharacterMode()));
    GetProperties().SetObjectItemDirect(Svx3DCloseFrontItem(rDefault.GetDefaultLatheCloseFront()));
    GetProperties().SetObjectItemDirect(Svx3DCloseBackItem(rDefault.GetDefaultLatheCloseBack()));
}

/*************************************************************************
|*
|* Identifier zurueckgeben
|*
\************************************************************************/

sal_uInt16 E3dLatheObj::GetObjIdentifier() const
{
    return E3D_LATHEOBJ_ID;
}

/*************************************************************************
|*
|* Wandle das Objekt in ein Gruppenobjekt bestehend aus n Polygonen
|*
\************************************************************************/

SdrObject* E3dLatheObj::DoConvertToPolygonObject(bool /*bBezier*/, bool /*bAddText*/) const
{
    return 0;
}

/*************************************************************************
|*
|* Neue Segmentierung (Beschreibung siehe Header-File)
|*
\************************************************************************/

void E3dLatheObj::ReSegment(sal_uInt32 nHSegs, sal_uInt32 nVSegs)
{
    if ((nHSegs != GetHorizontalSegments() || nVSegs != GetVerticalSegments()) &&
        (nHSegs != 0 || nVSegs != 0))
    {
        GetProperties().SetObjectItemDirect(SfxUInt32Item(SDRATTR_3DOBJ_HORZ_SEGS, nHSegs));
        GetProperties().SetObjectItemDirect(SfxUInt32Item(SDRATTR_3DOBJ_VERT_SEGS, nVSegs));

        ActionChanged();
    }
}

/*************************************************************************
|*
|* Lokale Parameter setzen mit Geometrieneuerzeugung
|*
\************************************************************************/

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

            GetProperties().SetObjectItemDirect(SfxUInt32Item(SDRATTR_3DOBJ_VERT_SEGS, nSegCnt));
        }

        ActionChanged();
    }
}

/*************************************************************************
|*
|* Get the name of the object (singular)
|*
\************************************************************************/

void E3dLatheObj::TakeObjNameSingul(XubString& rName) const
{
    rName=ImpGetResStr(STR_ObjNameSingulLathe3d);

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

void E3dLatheObj::TakeObjNamePlural(XubString& rName) const
{
    rName=ImpGetResStr(STR_ObjNamePluralLathe3d);
}

/*************************************************************************
|*
|* Aufbrechen
|*
\************************************************************************/

sal_Bool E3dLatheObj::IsBreakObjPossible()
{
    return sal_True;
}

SdrAttrObj* E3dLatheObj::GetBreakObj()
{
    // create PathObj
    basegfx::B3DPolyPolygon aLathePoly3D(basegfx::tools::createB3DPolyPolygonFromB2DPolyPolygon(maPolyPoly2D));
    basegfx::B2DPolyPolygon aTransPoly(TransformToScreenCoor(aLathePoly3D));
    SdrPathObj* pPathObj = new SdrPathObj(
        getSdrModelFromSdrObject(),
        aTransPoly);

    if(pPathObj)
    {
        // Attribute setzen
        SfxItemSet aSet(GetObjectItemSet());

        // Linien aktivieren, um Objekt garantiert sichtbar zu machen
        aSet.Put(XLineStyleItem(XLINE_SOLID));

        pPathObj->SetMergedItemSet(aSet);
    }

    return pPathObj;
}

// eof
