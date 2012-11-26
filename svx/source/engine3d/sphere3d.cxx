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
// #110094# DrawContact section

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

/*************************************************************************
|*
|* Kugel aus Polygonfacetten nach Laengen und Breitengrad aufbauen
|*
\************************************************************************/

E3dSphereObj::E3dSphereObj(
    SdrModel& rSdrModel,
    const E3dDefaultAttributes& rDefault,
    const basegfx::B3DPoint _aCenter,
    const basegfx::B3DVector a3DSize)
:   E3dCompoundObject(rSdrModel, rDefault)
{
    // Defaults setzen
    SetDefaultAttributes(rDefault);

    // Uebergebene drueberbuegeln
    aCenter = _aCenter;
    aSize = a3DSize;
}

E3dSphereObj::~E3dSphereObj()
{
}

void E3dSphereObj::copyDataFromSdrObject(const SdrObject& rSource)
{
    if(this != &rSource)
    {
        const E3dSphereObj* pSource = dynamic_cast< const E3dSphereObj* >(&rSource);

        if(pSource)
        {
            // call parent
            E3dCompoundObject::copyDataFromSdrObject(rSource);

            // copy local data
            aCenter = pSource->aCenter;
            aSize = pSource->aSize;
        }
        else
        {
            OSL_ENSURE(false, "copyDataFromSdrObject with ObjectType of Source different from Target (!)");
        }
    }
}

SdrObject* E3dSphereObj::CloneSdrObject(SdrModel* pTargetModel) const
{
    E3dSphereObj* pClone = new E3dSphereObj(
        pTargetModel ? *pTargetModel : getSdrModelFromSdrObject(),
        E3dDefaultAttributes());
    OSL_ENSURE(pClone, "CloneSdrObject error (!)");
    pClone->copyDataFromSdrObject(*this);

    return pClone;
}

void E3dSphereObj::SetDefaultAttributes(const E3dDefaultAttributes& rDefault)
{
    // Defaults setzen
    aCenter = rDefault.GetDefaultSphereCenter();
    aSize = rDefault.GetDefaultSphereSize();
}

/*************************************************************************
|*
|* Identifier zurueckgeben
|*
\************************************************************************/

sal_uInt16 E3dSphereObj::GetObjIdentifier() const
{
    return E3D_SPHEREOBJ_ID;
}

/*************************************************************************
|*
|* Wandle das Objekt in ein Gruppenobjekt bestehend aus n Polygonen
|*
\************************************************************************/

SdrObject *E3dSphereObj::DoConvertToPolygonObject(bool /*bBezier*/, bool /*bAddText*/) const
{
    return NULL;
}

/*************************************************************************
|*
|* Leer-Konstruktor
|*
\************************************************************************/

void E3dSphereObj::ReSegment(sal_uInt32 nHSegs, sal_uInt32 nVSegs)
{
    if((nHSegs != GetHorizontalSegments() || nVSegs != GetVerticalSegments()) && (nHSegs != 0 || nVSegs != 0))
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

/*************************************************************************
|*
|* Get the name of the object (singular)
|*
\************************************************************************/

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

/*************************************************************************
|*
|* Get the name of the object (plural)
|*
\************************************************************************/

void E3dSphereObj::TakeObjNamePlural(XubString& rName) const
{
    rName=ImpGetResStr(STR_ObjNamePluralSphere3d);
}

// eof
