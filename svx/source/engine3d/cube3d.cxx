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
#include <svx/cube3d.hxx>
#include "svx/globl3d.hxx"
#include <basegfx/point/b3dpoint.hxx>
#include <basegfx/polygon/b3dpolygon.hxx>
#include <svx/sdr/contact/viewcontactofe3dcube.hxx>

//////////////////////////////////////////////////////////////////////////////
// #110094# DrawContact section

sdr::contact::ViewContact* E3dCubeObj::CreateObjectSpecificViewContact()
{
    return new sdr::contact::ViewContactOfE3dCube(*this);
}

/*************************************************************************
|*
|* Konstruktor:                                                 |
|* 3D-Quader erzeugen; aPos: Zentrum oder links, unten, hinten  |__
|*                           (abhaengig von bPosIsCenter)      /
|*
\************************************************************************/

E3dCubeObj::E3dCubeObj(
    SdrModel& rSdrModel,
    const E3dDefaultAttributes& rDefault,
    const basegfx::B3DPoint aPos,
    const basegfx::B3DVector a3DSize)
:   E3dCompoundObject(rSdrModel, rDefault)
{
    // Defaults setzen
    SetDefaultAttributes(rDefault);

    // uebergebene drueberbuegeln
    aCubePos = aPos;
    aCubeSize = a3DSize;
}

//E3dCubeObj::E3dCubeObj()
//: E3dCompoundObject()
//{
//  // Defaults setzen
//  E3dDefaultAttributes aDefault;
//  SetDefaultAttributes(aDefault);
//}

E3dCubeObj::~E3dCubeObj()
{
}

void E3dCubeObj::copyDataFromSdrObject(const SdrObject& rSource)
{
    if(this != &rSource)
    {
        const E3dCubeObj* pSource = dynamic_cast< const E3dCubeObj* >(&rSource);

        if(pSource)
        {
            // call parent
            E3dCompoundObject::copyDataFromSdrObject(rSource);

            // copy local data
            aCubePos = pSource->aCubePos;
            aCubeSize = pSource->aCubeSize;
            bPosIsCenter = pSource->bPosIsCenter;
            nSideFlags = pSource->nSideFlags;
        }
        else
        {
            OSL_ENSURE(false, "copyDataFromSdrObject with ObjectType of Source different from Target (!)");
        }
    }
}

SdrObject* E3dCubeObj::CloneSdrObject(SdrModel* pTargetModel) const
{
    E3dCubeObj* pClone = new E3dCubeObj(
        pTargetModel ? *pTargetModel : getSdrModelFromSdrObject(),
        E3dDefaultAttributes());
    OSL_ENSURE(pClone, "CloneSdrObject error (!)");
    pClone->copyDataFromSdrObject(*this);

    return pClone;
}

void E3dCubeObj::SetDefaultAttributes(const E3dDefaultAttributes& rDefault)
{
    aCubePos = rDefault.GetDefaultCubePos();
    aCubeSize = rDefault.GetDefaultCubeSize();
    nSideFlags = rDefault.GetDefaultCubeSideFlags();
    bPosIsCenter = rDefault.GetDefaultCubePosIsCenter();
}

/*************************************************************************
|*
|* Identifier zurueckgeben
|*
\************************************************************************/

sal_uInt16 E3dCubeObj::GetObjIdentifier() const
{
    return E3D_CUBEOBJ_ID;
}

/*************************************************************************
|*
|* Wandle das Objekt in ein Gruppenobjekt bestehend aus 6 Polygonen
|*
\************************************************************************/

SdrObject* E3dCubeObj::DoConvertToPolygonObject(bool /*bBezier*/, bool /*bAddText*/) const
{
    return 0;
}

/*************************************************************************
|*
|* Lokale Parameter setzen mit Geometrieneuerzeugung
|*
\************************************************************************/

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

void E3dCubeObj::SetSideFlags(sal_uInt16 nNew)
{
    if(nSideFlags != nNew)
    {
        nSideFlags = nNew;
        ActionChanged();
    }
}

/*************************************************************************
|*
|* Get the name of the object (singular)
|*
\************************************************************************/

void E3dCubeObj::TakeObjNameSingul(XubString& rName) const
{
    rName=ImpGetResStr(STR_ObjNameSingulCube3d);

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

void E3dCubeObj::TakeObjNamePlural(XubString& rName) const
{
    rName=ImpGetResStr(STR_ObjNamePluralCube3d);
}

// eof
