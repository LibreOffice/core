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

TYPEINIT1(E3dSphereObj, E3dCompoundObject);

/*************************************************************************
|*
|* Kugel aus Polygonfacetten nach Laengen und Breitengrad aufbauen
|*
\************************************************************************/

E3dSphereObj::E3dSphereObj(E3dDefaultAttributes& rDefault, const basegfx::B3DPoint& rCenter, const basegfx::B3DVector& r3DSize)
:   E3dCompoundObject(rDefault)
{
    // Defaults setzen
    SetDefaultAttributes(rDefault);

    // Uebergebene drueberbuegeln
    aCenter = rCenter;
    aSize = r3DSize;
}

E3dSphereObj::E3dSphereObj()
:   E3dCompoundObject()
{
    // Defaults setzen
    E3dDefaultAttributes aDefault;
    SetDefaultAttributes(aDefault);
}

/*************************************************************************
|*
|* Kugel erzeugen ohne die Polygone darin zu erzeugen
|*
\************************************************************************/

// FG: Dieser Aufruf erfolgt von der 3D-Object Factory (objfac3d.cxx) und zwar ausschliesslich beim
//     laden von Dokumenten. Hier braucht man keinen CreateSphere-Aufruf, denn die wirkliche
//     Anzahl Segmente ist ja noch nicht bekannt. Dies war bis zum 10.2.97 ein (kleines)
//     Speicherleck.
E3dSphereObj::E3dSphereObj(int /*dummy*/) // den Parameter braucht es um unterscheiden zu koennen, welcher
{                                     // der beiden Konstruktoren gemeint ist. Der obige halt per Default
    // Defaults setzen
    E3dDefaultAttributes aDefault;
    SetDefaultAttributes(aDefault);
}

void E3dSphereObj::SetDefaultAttributes(E3dDefaultAttributes& rDefault)
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

SdrObject *E3dSphereObj::DoConvertToPolyObj(sal_Bool /*bBezier*/, bool /*bAddText*/) const
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
        GetProperties().SetObjectItemDirect(Svx3DHorizontalSegmentsItem(nHSegs));
        GetProperties().SetObjectItemDirect(Svx3DVerticalSegmentsItem(nVSegs));

        ActionChanged();
    }
}

/*************************************************************************
|*
|* Zuweisungsoperator
|*
\************************************************************************/

void E3dSphereObj::operator=(const SdrObject& rObj)
{
    // erstmal alle Childs kopieren
    E3dCompoundObject::operator=(rObj);

    // weitere Parameter kopieren
    const E3dSphereObj& r3DObj = (const E3dSphereObj&) rObj;

    aCenter       = r3DObj.aCenter;
    aSize         = r3DObj.aSize;
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
